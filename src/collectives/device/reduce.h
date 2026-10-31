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
  __device__ __forceinline__ void runRing(ncclWorkElem *args) {
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclRing *ring = &ncclShmem.channel.ring;
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? REDUCE_CHUNKSTEPS : 1));
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T)));
    const int nranks = ncclShmem.comm.nRanks;
    const ssize_t loopSize = nChannels*chunkSize;
    const ssize_t size = args->count;
    const int rank = ncclShmem.comm.rank;
    const int prevRank = ring->devUserRanks[nranks-1];
    const int root = args->root;

    Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>
      prims(tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);

    auto calcChunkSize = [&]__device__(ssize_t gridOffset)->int {
      int realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else if (Proto::Id == NCCL_PROTO_LL)
        realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;
      else if (Proto::Id == NCCL_PROTO_LL128)
        realChunkSize = min(divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128, chunkSize);
      return realChunkSize;
    };

/* 中文精读：Reduce 的流水线起点在 root 的后继
 * root 的后继没有上游需要合并的部分结果，因此先 send 本地输入。
 * 中间节点 recvReduceSend，将收到的部分结果加本地输入后向下游继续发。
 * root 最后 recvReduceCopy，把其余 rank 的归约结果与自己的输入合并，只有它需要最终输出。
 * 这里按 T 元素分片，count 是每 rank 输入长度；每一块都遵循同一条以 root 为终点的路径。
 * 与 AllReduce 不同，完成后没有再把完整结果广播给所有 rank 的阶段。
 */
    if (prevRank == root) {
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        int realChunkSize = calcChunkSize(gridOffset);
        ssize_t offset = gridOffset + bid*realChunkSize;
        int nelem = min(realChunkSize, size-offset);
        prims.send(offset, nelem);
      }
    }
    else if (rank == root) {
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        int realChunkSize = calcChunkSize(gridOffset);
        ssize_t offset = gridOffset + bid*realChunkSize;
        int nelem = min(realChunkSize, size-offset);
        prims.recvReduceCopy(offset, offset, nelem, /*postOp=*/true);
      }
    }
    else {
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        int realChunkSize = calcChunkSize(gridOffset);
        ssize_t offset = gridOffset + bid*realChunkSize;
        int nelem = min(realChunkSize, size-offset);
        prims.recvReduceSend(offset, nelem);
      }
    }
  }
}

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    using Proto = ProtoSimple<REDUCE_CHUNKSTEPS/REDUCE_SLICESTEPS, REDUCE_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL>(args);
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL128>(args);
  }
};
