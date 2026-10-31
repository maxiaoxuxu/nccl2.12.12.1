/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"

NCCL_API(ncclResult_t, ncclAllReduce, const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t datatype, ncclRedOp_t op, ncclComm* comm, cudaStream_t stream);
/* 中文导读：
 * 公开 API 只将用户参数封装为 ncclInfo，指定 AllReduce 的 chunk/slice 参数，再进入统一排队入口。
 * 真正的数据归约在设备端 all_reduce.h 与协议原语中完成，跨机网络推进由 Proxy/NET 后端配合。
 */
ncclResult_t ncclAllReduce(const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t datatype, ncclRedOp_t op, ncclComm* comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：从这一 API 开始跟完整调用链
 * 每个 rank 输入 count 个元素，输出同样 count 个归约结果；可同地址原地操作。
 * 示例：rank0=[1,2]，rank1=[10,20]，Sum 后两边输出均为 [11,22]。
 * 此函数只构造 ncclInfo；EnqueueCheck -> SetupCollKernel -> computeColl 选算法/协议及分片。
 * HostSetup 同时装 GPU FIFO 和发布 Proxy 工作，再 LaunchKernel；设备进入 all_reduce.h。
 * Ring/Tree/CollNet 在设备端调用组合原语，最后 ReduceOrCopyMulti 完成数值计算与存储。
 */
  struct ncclInfo info = { ncclFuncAllReduce, "AllReduce",
    sendbuff, recvbuff, count, datatype, op, 0, comm, stream, /* Args */
    ALLREDUCE_CHUNKSTEPS, ALLREDUCE_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
