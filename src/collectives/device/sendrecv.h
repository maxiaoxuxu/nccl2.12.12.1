/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "primitives.h"

template<typename T, typename RedOp>
/* 中文导读：
 * P2P 的设备端分派：把 work 中的多个发送/接收元素分给线程组，使用 Simple 协议和连接索引 1。
 * 对 self 的匹配通信可转为本地复制；对其他 peer 通过连接能力选择直接访问或缓冲区路径。
 */
struct RunWork<ncclFuncSendRecv, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ void runSend(const int tid, const int nthreads, const int group, struct ncclWorkElemP2p* args) {
    if (args->peer == ncclShmem.comm.rank) {
      struct ncclWorkElemP2p* recvArgs = args-1;
      if (args->buff != recvArgs->buff) {
        ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 0>(tid, nthreads, nullptr, false, 1, (const T**)&args->buff, 1, (T**)&recvArgs->buff, args->count);
      }
    } else {
      using Proto = ProtoSimple<1, 1>;
      ssize_t const count = args->count;
      int const chunkSize = args->chunkSize/sizeof(T);
      int const peer = args->peer;
      Primitives<T, RedOp, FanAsymmetric<0, 1>, 1, Proto, 1> prims
        (tid, nthreads, nullptr, &peer, args->buff, nullptr, /*redOpArg(ignored)=*/0, group);
/* 中文精读：P2P 的第二层切块：在选定 channel 内推进
 * Group 已把原消息切成分配给 channel 的块，此处对当前 args->count 再按 chunkSize 循环。
 * 每轮 directSend 内还有 Simple 的 wait/copy/post，CPU Proxy 根据同一尺寸推进网络。
 * do-while 使 count=0 也执行一次协议同步；它并非遗漏了空数据检查。
 * 这条路径只复制数据，不做集合归约；All-to-All 可由上层为所有 peer 组合这样的 Send/Recv。
 */
      ssize_t offset = 0;
      do {
        int nelem = min(chunkSize, count-offset);
        prims.directSend(offset, offset, nelem);
        offset += nelem;
      } while(offset < count);
    }
  }

  __device__ __forceinline__ void runRecv(const int tid, const int nthreads, const int group, struct ncclWorkElemP2p* args) {
    if (args->peer != ncclShmem.comm.rank) {
      using Proto = ProtoSimple<1, 1>;
      ssize_t const count = args->count;
      int const chunkSize = args->chunkSize/sizeof(T);
      int const peer = args->peer;
      Primitives<T, RedOp, FanAsymmetric<1, 0>, 1, Proto, 1> prims
        (tid, nthreads, &peer, nullptr, nullptr, args->buff, /*redOpArg(ignored)=*/0, group);
      ssize_t offset = 0;
      do {
        int nelem = min(chunkSize, count-offset);
        prims.directRecv(offset, nelem);
        offset += nelem;
      } while(offset < count);
    }
  }

  __device__ __forceinline__ void run(ncclWork *work) {
    struct ncclWorkElemP2p* args = work->p2pElems;
    int ngroups = args->ngroups;
    int tid = threadIdx.x;
    int wid = tid / WARP_SIZE;
    // This has to work even for groups of 2.5 warps (which is 8 groups, and means 3
    // warps for send, 2 warps for recv).
    // warpStarts were rounded thanks to int division, but for group number we need to round the other way around
    // So we mirror wid then mirror again the group.
    #define NWARPS (NCCL_MAX_NTHREADS/WARP_SIZE)
    int group = ngroups-1- (NWARPS-1-wid) * ngroups / NWARPS;
    args += group;
    if (args->header.type == ncclWorkTypeUnused) return;

    tid -= args->warpStart * WARP_SIZE;
    int nthreads = args->nWarps * WARP_SIZE;
/* 中文精读：线程组编码与连接索引
 * work 里收发按偶/奇槽分组，先算当前 warp 属于哪个元素，再减 warpStart 得到组内 tid。
 * 第 16 位是传给 Simple 构造函数的连接索引编码，使 P2P 找 peers[peer].send/recv[1]。
 * 这个位不是 channelId，也不是 GPU rank；低位仍用于组共享内存和 barrier 分配。
 * self send 在 runSend 中直接复制到前一个匹配 recv 元素的缓冲，runRecv 的 self 分支不重复搬运。
 */
    group |= 1<<16; // Used to select connIndex 1
    if (tid >= nthreads || args->peer == -1) return;
    if ((group%2) == 0) {
      runRecv(tid, nthreads, group, args);
    } else {
      runSend(tid, nthreads, group, args);
    }
  }
};
