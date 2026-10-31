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
 * Ring ReduceScatter：沿环逐块接收、合并本地输入并转发，最终只保留属于本 rank 的归约分块。
 * 与完整 AllReduce 的区别是没有再传播所有结果的 AllGather 阶段。
 */
  __device__ __forceinline__ void runRing(ncclWorkElem *args) {
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclRing *ring = &ncclShmem.channel.ring;
    int const *ringRanks = ring->devUserRanks;
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? REDUCESCATTER_CHUNKSTEPS : 1));
    // We should not need the final /2 but it makes performance much, much smoother. Might be a bug somewhere.
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2);
    const int nranks = ncclShmem.comm.nRanks;
    const ssize_t loopSize = nChannels*chunkSize;
    const ssize_t size = args->count;

    Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>
      prims(tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);

    for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else if (Proto::Id == NCCL_PROTO_LL)
        realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;
      else if (Proto::Id == NCCL_PROTO_LL128)
        realChunkSize = min(divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128, chunkSize);
      realChunkSize = int(realChunkSize);

      ssize_t chunkOffset = gridOffset + bid*int(realChunkSize);

      /////////////// begin ReduceScatter steps ///////////////
      ssize_t offset;
      int nelem = min(realChunkSize, size-chunkOffset);
      int rankDest;

      // step 0: push data to next GPU
/* 中文精读：ReduceScatter 的输入输出下标
 * size 是 recvcount，即每个 rank 最终保留的 T 元素数，输入总长度 nranks*size。
 * 本 channel 处理所有 rank 段中相同的 chunkOffset；offset=用户rank*size+chunkOffset。
 * 先发最后一段，逐步接收归约并转发，最后把属于自己的完整结果写到 output[chunkOffset]。
 * 例：4 rank、recvcount=8 float，每人输入 32 float，rank2 最终得到全局求和后的 [16,24) 段，
 * 但放在自己的 8 元素输出缓冲 [0,8) 中。
 */
      rankDest = ringRanks[nranks-1];
      offset = chunkOffset + rankDest * size;
      prims.send(offset, nelem);

      // k-2 steps: reduce and copy to next GPU
      for (int j=2; j<nranks; ++j) {
        rankDest = ringRanks[nranks-j];
        offset = chunkOffset + rankDest * size;
        prims.recvReduceSend(offset, nelem);
      }

      // step k-1: reduce this buffer and data, which will produce the final result
      rankDest = ringRanks[0];
      offset = chunkOffset + rankDest * size;
      prims.recvReduceCopy(offset, chunkOffset, nelem, /*postOp=*/true);
    }
  }
}

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    using Proto = ProtoSimple<REDUCESCATTER_CHUNKSTEPS/REDUCESCATTER_SLICESTEPS, REDUCESCATTER_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL>(args);
  }
};

template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128> {
  __device__ __forceinline__ void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL128>(args);
  }
};
