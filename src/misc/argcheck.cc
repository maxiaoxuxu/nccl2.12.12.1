/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "argcheck.h"
#include "comm.h"

static ncclResult_t CudaPtrCheck(const void* pointer, struct ncclComm* comm, const char* ptrname, const char* opname) {
  cudaPointerAttributes attr;
  cudaError_t err = cudaPointerGetAttributes(&attr, pointer);
  if (err != cudaSuccess || attr.devicePointer == NULL) {
    WARN("%s : %s %p is not a valid pointer", opname, ptrname, pointer);
    return ncclInvalidArgument;
  }
#if CUDART_VERSION >= 10000
  if (attr.type == cudaMemoryTypeDevice && attr.device != comm->cudaDev) {
#else
  if (attr.memoryType == cudaMemoryTypeDevice && attr.device != comm->cudaDev) {
#endif
    WARN("%s : %s allocated on device %d mismatchs with NCCL device %d", opname, ptrname, attr.device, comm->cudaDev);
    return ncclInvalidArgument;
  }
  return ncclSuccess;
}

ncclResult_t PtrCheck(void* ptr, const char* opname, const char* ptrname) {
  if (ptr == NULL) {
    WARN("%s : %s argument is NULL", opname, ptrname);
    return ncclInvalidArgument;
  }
  return ncclSuccess;
}

/* 中文导读：
 * 除参数检查外还统一计量：先计算 count*datatypeSize，将 AllGather/Broadcast 转成字节类型。
 * AllGather/ReduceScatter 的 nBytes 再乘 nRanks，因为公开 count 是单 rank 的贡献/输出规模。
 * 因此 nBytes、内部 count 与公开 API count 不能跨不同 collective 直接套同一解释。
 */
ncclResult_t ArgsCheck(struct ncclInfo* info) {
  // First, the easy ones
  if (info->root < 0 || info->root >= info->comm->nRanks) {
    WARN("%s : invalid root %d (root should be in the 0..%d range)", info->opName, info->root, info->comm->nRanks);
    return ncclInvalidArgument;
  }
  if (info->datatype < 0 || info->datatype >= ncclNumTypes) {
    WARN("%s : invalid type %d", info->opName, info->datatype);
    return ncclInvalidArgument;
  }
/* 中文精读：API 参数归一化，分片公式从这里开始
 * 输入 count 的单位是用户 datatype 元素；先乘类型大小得到 nBytes。
 * AllGather/Broadcast 只复制，不需要按数值类型运算，因此 count 换成字节且 datatype 换成 Int8。
 * AllGather 的 sendcount、ReduceScatter 的 recvcount 都是每 rank 的片段长度，
 * 用于性能模型和 Proxy 循环的 nBytes 再乘 nRanks；设备 args->count 仍表示单段长度。
 * 例：4 rank、每 rank 8 个 float，AG 的原始 count=8，归一后 count=32 字节、nBytes=128；
 * RS 的 count=8 个 float、nBytes=128 字节。二者设备下标单位因此不同。
 */
  // Type is OK, compute nbytes. Convert Allgather/Broadcast/P2P calls to chars.
  info->nBytes = info->count * ncclTypeSize(info->datatype);
  if (info->coll == ncclFuncAllGather || info->coll == ncclFuncBroadcast) {
    info->count = info->nBytes;
    info->datatype = ncclInt8;
  }
  if (info->coll == ncclFuncAllGather || info->coll == ncclFuncReduceScatter) info->nBytes *= info->comm->nRanks; // count is per rank

  if (info->op < 0 || ncclMaxRedOp < info->op) {
    WARN("%s : invalid reduction operation %d", info->opName, info->op);
    return ncclInvalidArgument;
  }
  int opIx = int(ncclUserRedOpMangle(info->comm, info->op)) - int(ncclNumOps);
  if (ncclNumOps <= info->op &&
      (info->comm->userRedOpCapacity <= opIx || info->comm->userRedOps[opIx].freeNext != -1)) {
    WARN("%s : reduction operation %d unknown to this communicator", info->opName, info->op);
    return ncclInvalidArgument;
  }

  if (info->comm->checkPointers) {
    if ((info->coll == ncclFuncSend || info->coll == ncclFuncRecv)) {
      if (info->count >0)
        NCCLCHECK(CudaPtrCheck(info->recvbuff, info->comm, "buff", info->opName));
    } else {
      // Check CUDA device pointers
      if (info->coll != ncclFuncBroadcast || info->comm->rank == info->root) {
        NCCLCHECK(CudaPtrCheck(info->sendbuff, info->comm, "sendbuff", info->opName));
      }
      if (info->coll != ncclFuncReduce || info->comm->rank == info->root) {
        NCCLCHECK(CudaPtrCheck(info->recvbuff, info->comm, "recvbuff", info->opName));
      }
    }
  }
  return ncclSuccess;
}
