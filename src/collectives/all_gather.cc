/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"

NCCL_API(ncclResult_t, ncclAllGather, const void* sendbuff, void* recvbuff, size_t sendcount,
    ncclDataType_t datatype, ncclComm_t comm, cudaStream_t stream);
/* 中文导读：
 * AllGather 的 count 是每个 rank 的输入贡献元素数，输出需能容纳 nRanks 份贡献。
 * API 包装后进入统一调度，rank 排序与设备 Ring 顺序的转换在设备算法中处理。
 */
ncclResult_t ncclAllGather(const void* sendbuff, void* recvbuff, size_t sendcount,
    ncclDataType_t datatype, ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：AllGather 的调用与尺寸契约
 * sendcount 是本 rank 贡献元素数，输出需容纳 nRanks*sendcount，按用户 rank 顺序排列。
 * 示例：rank0=[1,2]，rank1=[3,4]，两边输出都是 [1,2,3,4]，不进行求和。
 * EnqueueCheck 的 ArgsCheck 会把复制型任务归一为 Int8，再走共同的选型/排队/发射流程。
 * 设备 all_gather.h 的 Ring 每步收/存/转发别人的贡献，地址公式是 rank*size+chunkOffset。
 * 原地形式要求 sendbuff 指向 recvbuff 中属于自己的那一段，不是任意两个指针相同即可。
 */
  struct ncclInfo info = { ncclFuncAllGather, "AllGather",
    sendbuff, recvbuff, sendcount, datatype, ncclSum, 0, comm, stream, /* Args */
    ALLGATHER_CHUNKSTEPS, ALLGATHER_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
