/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"

NCCL_API(ncclResult_t, ncclReduce, const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t datatype, ncclRedOp_t op, int root, ncclComm_t comm, cudaStream_t stream);
/* 中文导读：
 * Reduce 汇总各 rank 的输入，但最终结果只要求在 root 的接收缓冲有效。
 * 数据归约由设备原语执行；API 层的 root 语义与 P2P 把同一字段用作 peer 的语义不同。
 */
ncclResult_t ncclReduce(const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t datatype, ncclRedOp_t op, int root, ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：Reduce 只有 root 需要最终输出
 * 所有 rank 各提供 count 元素；归约结果仅保证放到 root 的 recvbuff。
 * 示例：两个 rank 输入 [1,2] 和 [10,20]，root 得到 [11,22]，其他 rank 无需完整输出。
 * 其主机入队流程与 AllReduce 共用，但 pattern 是 PipelineTo，Proxy 仅准备必要方向的边。
 * 设备 reduce.h 从 root 后继启动 send，沿环归约到 root 后结束，不再做 AllGather。
 * 选择 root 改变流水线的起点/终点，不改变数据类型和归约操作的数学含义。
 */
  struct ncclInfo info = { ncclFuncReduce, "Reduce",
    sendbuff, recvbuff, count, datatype, op, root, comm, stream, /* Args */
    REDUCE_CHUNKSTEPS, REDUCE_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
