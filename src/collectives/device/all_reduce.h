/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "primitives.h"

namespace {
  template<typename T, typename RedOp, typename Proto>
/* 中文导读：
 * Ring AllReduce 的设备端主循环。每个 channel/block 负责输入的一部分，沿 prev/next 传递分块。
 * 先 send 和 recvReduceSend 形成 ReduceScatter；最终归约块经 directRecvReduceCopySend 衔接到 AllGather。
 * 这里的 direct 仍受连接能力约束，不能据函数名判断为 GPU 直接提交 RDMA WQE。
 */
  __device__ __forceinline__ void runRing(ncclWorkElem *args) {
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclRing *ring = &ncclShmem.channel.ring;
    int ringIx = ring->index;
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLREDUCE_CHUNKSTEPS : 1));
    const int nranks = ncclShmem.comm.nRanks;
    const ssize_t loopSize = nChannels*nranks*chunkSize;
    const ssize_t size = args->count;

    int minChunkSize;
    if (Proto::Id == NCCL_PROTO_LL)
      minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T));
    if (Proto::Id == NCCL_PROTO_LL128) {
      // We should not need the final /2 but it makes performance much, much smoother. Might be a bug somewhere.
      minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2;
    }

    Primitives<T, RedOp, FanSymmetric<1>, 1, Proto, 0> prims
      (tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);

    for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks*minChunkSize)*minChunkSize);
      realChunkSize = int(realChunkSize);

/* 中文精读：AllReduce 的地址公式与数值示例
 * 所有偏移以 T 元素计。Simple 使用 channel 连续布局：
 * offset=gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize。
 * 假设 4 rank、2 channel、nthreads=96、共 1024 个 float，且 chunkSize 至少为 128 元素。
 * 对齐粒度=(96-32)*8/4=128 元素，realChunkSize=ceil(1024/8)=128。
 * channel0 的 4 块覆盖 [0,512)，channel1 的 4 块覆盖 [512,1024)，块长都是 128 元素。
 * LL/LL128 用 (chunk*nChannels+bid)*realChunkSize 交错布局；不能把 Simple 公式套到所有协议。
 * 注意 realChunkSize 是对齐后的容量，不保证每个块都有这么多有效元素。
 */
      auto calcOffset = [&]__device__(int chunk)->ssize_t {
        if (Proto::Id == NCCL_PROTO_SIMPLE)
          return gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize;
        else
          return gridOffset + (chunk*nChannels + bid)*realChunkSize;
      };
      auto modRanks = [&]__device__(int r)->int {
        return r - (r >= nranks ? nranks : 0);
      };

      ssize_t offset;
      int nelem;
      int chunk;

/* 中文导读：
 * 本轮先发送第一个块，再逐块把上游数据与本地输入归约后继续传递。
 * 各 rank 处理不同块，形成流水线；不是每一轮都发送整个输入数组。
 */
      // step 0: push data to next GPU
      chunk = modRanks(ringIx + nranks-1);
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.send(offset, nelem);

      // k-2 steps: reduce and copy to next GPU
      for (int j=2; j<nranks; ++j) {
        chunk = modRanks(ringIx + nranks-j);
        offset = calcOffset(chunk);
        nelem = min(realChunkSize, size-offset);
        prims.recvReduceSend(offset, nelem);
      }

      // step k-1: reduce this buffer and data, which will produce the final
      // result that we store in this data and push to the next GPU
/* 中文精读：归约分散与结果收集在这一调用衔接
 * 最初发送 (ringIx-1) 块，再依次处理前面的块；收到本 rank 的 ringIx 块时，
 * 它已经包含其余 rank 的部分结果，加上本地输入后才成为完整归约结果。
 * directRecvReduceCopySend 同时把最终值写本地 output 并发下游，启动结果收集阶段。
 * 对于 4 rank/单 channel，rank0 依次 send 块3，reduceSend 块2、块1，最终归约块0；
 * 再 copySend 块3、块2，最后 recv 块1。实际环顺序改变时，ringIx 的对应用户 rank 也改变。
 */
      chunk = ringIx + 0;
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.directRecvReduceCopySend(offset, offset, offset, nelem, /*postOp=*/true);

/* 中文导读：
 * 从这里进入结果传播阶段：继续转发已经完成归约的块，不再对这些块重复做全局求和。
 * 最后 directRecv 收齐本 rank 缺少的最后一个块，整段完成 ReduceScatter+AllGather 语义。
 */
      // k-2 steps: copy to next GPU
      for (int j=1; j<nranks-1; ++j) {
        chunk = modRanks(ringIx + nranks-j);
        offset = calcOffset(chunk);
        nelem = min(realChunkSize, size-offset);
        prims.directRecvCopySend(offset, offset, nelem);
      }

      // Make final copy from buffer to dest.
      chunk = modRanks(ringIx + 1);
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.directRecv(offset, nelem);
    }
  }

  template<typename T, typename RedOp, typename Proto>
/* 中文导读：
 * Tree 的先向上归约、再向下广播实现；根/叶/中间节点使用不同原语。
 * root 把孩子与自身输入归约成结果，之后沿反向边传播。所需邻居由初始化时的 tree.up/down 给出。
 */
  __device__ __forceinline__ void runTreeUpDown(ncclWorkElem *args) {
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclTree *tree = &ncclShmem.channel.tree;
    ssize_t chunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? args->lastChunkSize
                   /* LL & LL128 */  : Proto::calcBytePerStep()/sizeof(T));
    const ssize_t minChunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads-2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T))
                   /* LL & LL128 */  : nthreads*(Proto::calcBytePerGrain()/sizeof(T)));
    const ssize_t loopSize = int(nChannels*chunkSize);
    const ssize_t size = args->count;

    if (loopSize > size)
      chunkSize = divUp((int)size, int(nChannels*minChunkSize))*int(minChunkSize);

    { // Reduce : max number of recv is 3, max number of send is 1 (binary tree + local)
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, /*Direct=*/0, Proto, 0> prims
        (tid, nthreads, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg);
      if (tree->up == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceCopy(offset, offset, nelem, /*postOp=*/true);
        }
      }
      else if (tree->down[0] == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
      else {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceSend(offset, nelem);
        }
      }
    }

    { // Broadcast : max number of recv is 1, max number of send is 3 (binary tree + local)
      Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0> prims
        (tid, nthreads, &tree->up, tree->down, args->sendbuff, args->recvbuff, args->redOpArg);
      if (tree->up == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directSendFromOutput(offset, offset, nelem);
        }
      }
      else if (tree->down[0] == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directRecv(offset, nelem);
        }
      }
      else {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directRecvCopySend(offset, offset, nelem);
        }
      }
    }
  }

  template<typename T, typename RedOp, typename Proto>
/* 中文导读：
 * 在同一 block 中分配不同线程组推进向上归约与向下广播，以改善流水重叠。
 * 这是设备端线程执行拆分；不要与 graph.h 中决定跨节点流量端点的 SPLIT_TREE 搜索模式混为一谈。
 */
  __device__ __forceinline__ void runTreeSplit(ncclWorkElem *args) {
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclTree *tree = &ncclShmem.channel.tree;
    ssize_t chunkSize = int(
      Proto::Id != NCCL_PROTO_LL ? args->lastChunkSize
                                 : Proto::calcBytePerStep()/sizeof(T));
    const ssize_t minChunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads - 2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T)) :
      Proto::Id == NCCL_PROTO_LL     ? nthreads*(Proto::calcBytePerGrain()/sizeof(T))
                   /* LL128 */       : nthreads*(Proto::calcBytePerGrain()/sizeof(T))/8);
    const ssize_t loopSize = int(nChannels*chunkSize);
    const ssize_t size = args->count;

/* 中文精读：Tree 分线程组的流水线
 * 非根节点把一组线程用于向上归约，另一组线程用于向下广播；二者使用不同 group/barrier。
 * Simple 大致均分后对归约组作调整，LL/LL128 约 70% 线程给更重的多输入归约。
 * 根用融合原语同时完成收集、归约、保存、向下分发；叶子没有有效 child，原语自动简化。
 * 线程组分割是设备执行优化，拓扑 SPLIT_TREE 则是选择跨机端点，两者不能混作一个开关。
 */
    int nthreadsSplit;
    if (Proto::Id == NCCL_PROTO_SIMPLE) {
      nthreadsSplit = nthreads/2;
      if (nthreadsSplit >= 256) nthreadsSplit += 64;
    } else { // LL & LL128
      // Receiving from up to 3 sources is more compute intensive than sending
      // to 3 dests. Use 70% for reduce and 30% for bcast.
      nthreadsSplit = (nthreads*7/(10*WARP_SIZE))*WARP_SIZE;
    }

    if (loopSize > size)
      chunkSize = divUp((int)size, nChannels*int(minChunkSize))*int(minChunkSize);

    if (tree->up == -1) {
      // Reduce and broadcast. Max number of recv is 3, max number of send is 3
      Primitives<T, RedOp, FanSymmetric<NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid, nthreads, tree->down, tree->down, args->sendbuff, args->recvbuff, args->redOpArg);
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        ssize_t offset = gridOffset + bid*int(chunkSize);
        int nelem = min(chunkSize, size-offset);
        prims.directRecvReduceCopySend(offset, offset, offset, nelem, /*doPost=*/true);
      }
    }
    else if (tid < nthreadsSplit) {
      /* Reduce up. Max number of recv is 3, max number of send is 1 (binary tree + local).
       * Why Direct=1????
       * Answer: Because despite not performing any direct operations, the ctor
       * must assume Direct so that it can exchange direct pointers with remote ctors
       * that are Direct, otherwise it hangs. A cleaner solution would be to seperate
       * into DirectRecv and DirectSend capabilities, this ctor would have both=0,
       * but the ctor above for tree roots would be DirectRecv=0 DirectSend=1.
       */
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, /*Direct=*/1, Proto, 0>
        prims(tid, nthreadsSplit, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg, 0*Proto::MaxGroupWidth);
      if (tree->down[0] == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
      else {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceSend(offset, nelem);
        }
      }
    }
    else {
      // Broadcast down. Max number of recv is 1, max number of send is 3 (binary tree + local)
      Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid-nthreadsSplit, nthreads-nthreadsSplit, &tree->up, tree->down, args->sendbuff, args->recvbuff, args->redOpArg, 1*Proto::MaxGroupWidth);
      if (tree->down[0] == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directRecv(offset, nelem);
        }
      }
      else {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directRecvCopySend(offset, offset, nelem);
        }
      }
    }
  }
}

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    using Proto = ProtoSimple<ALLREDUCE_CHUNKSTEPS/ALLREDUCE_SLICESTEPS, ALLREDUCE_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    #if CUDART_VERSION >= 11020 && CUDART_VERSION < 11040 && __CUDA_ARCH__ >= 800
      runTreeUpDown<T, RedOp, ProtoSimple<1, 1>>(args);
    #else
      runTreeSplit<T, RedOp, ProtoSimple<1, 1>>(args);
    #endif
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_COLLNET, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
/* 中文精读：CollNet 把一次 AllReduce 分成四组协作流水
 * Scatter 把本机输入分配给多个 head；Reduce 组在 head 聚合机内贡献并发往网络。
 * Broadcast 组把网络归约结果分发到机内参与者；Gather 组收齐各个 head 负责的片段。
 * loopSize=nChannels*nHeads*chunkSize，因此除 channel 分片外，还多了一层 head 分片。
 * 网络端点 tree->out 对应特殊 peer，Proxy 调插件 Iallreduce；GPU 不直接在交换机上执行归约。
 * 本版此特化只支持 Simple，启用需要 CollNet 插件与对应硬件/数据类型支持。
 */
    static constexpr int COLLNET_COPY_THREADS = 96;
    const int tid = threadIdx.x;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    struct ncclDirect* tree = &ncclShmem.channel.collTree;
    const ssize_t chunkSize = int(args->lastChunkSize);
    const ssize_t size = args->count;
    const ssize_t loopSize = nChannels*tree->nHeads*chunkSize;

    const int hasUp = (tree->up[0] >= 0) ? 1 : 0;
    const int hasDn = (tree->down[0] >= 0) ? 1 : 0;
    const int nThreadsScatter = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 3*COLLNET_COPY_THREADS : 0);
    const int nThreadsGather  =             ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 2*COLLNET_COPY_THREADS : 0);
    const int nThreadsBcast   = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 0 : 2*COLLNET_COPY_THREADS);
    const int nThreadsReduce = args->header.nWarps*WARP_SIZE - nThreadsScatter - nThreadsGather - nThreadsBcast;
    const int tidStartBcast = nThreadsGather;
    const int tidStartScatter = tidStartBcast + nThreadsBcast;
    const int tidStartReduce = tidStartScatter + nThreadsScatter;

    using Proto = ProtoSimple<1, 1>;

    if (tid >= tidStartScatter && tid < tidStartReduce && hasUp) {
      // Scatter
      int group = (2*Proto::MaxGroupWidth) | (1<<16);
      Primitives<T, RedOp, FanAsymmetric<0, NCCL_MAX_DIRECT_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid-tidStartScatter, nThreadsScatter, NULL, tree->up, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        ssize_t offset = gridOffset + bid*tree->nHeads*chunkSize;
        int nelem = min(tree->nHeads*chunkSize, size-offset);
        if (args->regUsed) {
          prims.directScatter(offset, nelem, chunkSize, tree->headRank, tree->shift);
        } else {
          prims.scatter(offset, nelem, chunkSize, tree->headRank, tree->shift);
        }
      }
    } else if (tid >= tidStartReduce && tree->out != -1) {
      int group = (3*Proto::MaxGroupWidth) | (1<<16);
      if (hasDn) {
        // Reduce, send to network
        Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 1>, /*Direct=*/1, Proto, 0>
          prims(tid-tidStartReduce, nThreadsReduce, tree->down, &tree->out, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
          int nelem = min(chunkSize, size-offset);
          if (args->regUsed) {
            prims.directRecvReduceSend(offset, offset, nelem);
          } else {
            prims.recvReduceSend(offset, nelem);
          }
        }
      } else {
        // Directly send to network
        Primitives<T, RedOp, FanAsymmetric<0, 1>, /*Direct=*/0, Proto, 0>
          prims(tid-tidStartReduce, nThreadsReduce, nullptr, &tree->out, args->sendbuff, args->recvbuff, args->redOpArg, group);
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
    } else if (tid < tidStartBcast && hasUp) {
      // Gather
      int group = (0*Proto::MaxGroupWidth) | (0<<16);
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 0>, /*Direct=*/1, Proto, 0>
        prims(tid, nThreadsGather, tree->up, NULL, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        ssize_t offset = gridOffset + bid*tree->nHeads*chunkSize;
        int nelem = min(tree->nHeads*chunkSize, size-offset);
        prims.directGather(offset, nelem, chunkSize, tree->headRank, tree->shift);
      }
    } else if (tid >= tidStartBcast && tid < tidStartScatter && tree->out != -1) {
      int group = (1*Proto::MaxGroupWidth) | (0<<16);
      if (hasDn) {
        // Recv from network, broadcast
        Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DIRECT_ARITY>, /*Direct=*/1, Proto, 0>
          prims(tid-tidStartBcast, nThreadsBcast, &tree->out, tree->down, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
          int nelem = min(chunkSize, size-offset);
          prims.recvCopyDirectSend(offset, offset, nelem, /*postOp=*/true);
        }
      } else {
        // Recv from network (no post thread needed)
        Primitives<T, RedOp, FanAsymmetric<1, 0>, /*Direct=*/0, Proto, 0>
          prims(tid-tidStartBcast, nThreadsBcast, &tree->out, nullptr, args->sendbuff, args->recvbuff, args->redOpArg, group);
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
          int nelem = min(chunkSize, size-offset);
          prims.recv(offset, nelem, /*postOp=*/true);
        }
      }
    }
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL>(args);
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    runTreeSplit<T, RedOp, ProtoLL>(args);
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL128>(args);
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL128> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    runTreeSplit<T, RedOp, ProtoLL128>(args);
  }
};
