/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"

NCCL_API(ncclResult_t, ncclReduceScatter, const void* sendbuff, void* recvbuff, size_t recvcount,
    ncclDataType_t datatype, ncclRedOp_t op, ncclComm* comm, cudaStream_t stream);
/* 中文导读：
 * ReduceScatter 的 recvcount 是本 rank 最终输出的元素数，输入覆盖所有 rank 的分块。
 * 统一调度器计算通信字节数时会处理该语义，不能把 recvcount 直接当作整个输入大小。
 */
ncclResult_t ncclReduceScatter(const void* sendbuff, void* recvbuff, size_t recvcount,
    ncclDataType_t datatype, ncclRedOp_t op, ncclComm* comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：ReduceScatter 的调用与尺寸契约
 * recvcount 是本 rank 的输出元素数，输入需容纳 nRanks*recvcount，所有 rank 输入长度相同。
 * 示例：2 rank、recvcount=2，输入 [1,2,3,4] 和 [10,20,30,40]，
 * rank0 输出 [11,22]，rank1 输出 [33,44]，即完整求和结果按 rank 分段。
 * EnqueueCheck/computeColl 生成 Ring 工作，设备 reduce_scatter.h 做 send/reduceSend/reduceCopy。
 * 最终归约写到自己短输出的 chunkOffset，而输入读取对应用户 rank 的完整段偏移。
 */
  struct ncclInfo info = { ncclFuncReduceScatter, "ReduceScatter",
    sendbuff, recvbuff, recvcount, datatype, op, 0, comm, stream, /* Args */
    REDUCESCATTER_CHUNKSTEPS, REDUCESCATTER_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
