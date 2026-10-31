/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"

NCCL_API(ncclResult_t, ncclBroadcast, const void* sendbuff, void* recvbuff, size_t count, ncclDataType_t datatype, int root,
    ncclComm_t comm, cudaStream_t stream);
/* 中文导读：
 * Broadcast 将 root 的输入传播到各 rank，公开入口使用统一 ncclInfo 描述。
 * 实际沿环流水或树向下执行，取决于该版本可用实现与调度结果；它不进行跨 rank 求和。
 */
ncclResult_t ncclBroadcast(const void* sendbuff, void* recvbuff, size_t count, ncclDataType_t datatype, int root,
    ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：Broadcast 的根与普通参与者
 * 所有 rank 都必须参与同一调用序列；只有 root 的 sendbuff 是广播输入。
 * 每个 rank 的 recvbuff 得到相同 count 元素，不增加 nRanks 倍的输出长度。
 * ArgsCheck 归一到 Int8，随后共用 enqueue 提交流程；设备 broadcast.h 按 root 切断环回边。
 * root copySend 或 send，中间 recvCopySend，root 前驱 recv；数据可以按 slice 流水传播。
 * 兼容 API ncclBcast 只把同一 buff 作为输入输出转调这里。
 */
  struct ncclInfo info = { ncclFuncBroadcast, "Broadcast",
    sendbuff, recvbuff, count, datatype, ncclSum, root, comm, stream, /* Args */
    BROADCAST_CHUNKSTEPS, BROADCAST_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
/* Deprecated original "in place" function, similar to MPI */
NCCL_API(ncclResult_t, ncclBcast, void* buff, size_t count, ncclDataType_t datatype, int root,
    ncclComm_t comm, cudaStream_t stream);
ncclResult_t ncclBcast(void* buff, size_t count, ncclDataType_t datatype, int root,
    ncclComm_t comm, cudaStream_t stream) {
  return ncclBroadcast(buff, buff, count, datatype, root, comm, stream);
}

