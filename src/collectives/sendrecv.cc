/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"
#include "argcheck.h" // Need some checks here since we access comm

NCCL_API(ncclResult_t, ncclSend, const void* sendbuff, size_t count, ncclDataType_t datatype, int peer,
    ncclComm_t comm, cudaStream_t stream);
/* 中文导读：
 * 发送 API 使用 ncclInfo 的 recvbuff 字段承载本地发送指针，root 字段承载 peer。
 * 内部用一层 group 包住排队，使 Send/Recv 进入统一 P2P 调度；外层应用 group 可以将多对通信一起提交。
 */
/* 中文精读：Send/Recv 与 All-to-All 的关系
 * 本版有公开 Send/Recv，但没有公开 ncclAllToAll/ncclGather/ncclScatter 函数。
 * 这些模式可由上层用 Group 组合多个 Send/Recv；资源取决于实际 peer 和映射路径。
 * Send 必须与目标 rank 同序的 Recv 匹配，应用保证长度/类型契约；API 没有任意用户 tag。
 * 调用内部 GroupStart/End 支持单条提交，应用外层 Group 则能让多条收发一起排队发射。
 * 调用链：SaveP2p -> GroupEnd 分块与预建链 -> SetupP2pKernel -> FIFO -> device/sendrecv.h。
 */
ncclResult_t ncclSend(const void* sendbuff, size_t count, ncclDataType_t datatype, int peer,
    ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  struct ncclInfo info = { ncclFuncSend, "Send",
    NULL, (void*)sendbuff, count, datatype, ncclSum, peer, comm, stream, /* Args */
    1, 1 };
  ncclResult_t ret;
  NCCLCHECK(ncclGroupStart());
  ret = ncclEnqueueCheck(&info);
  NCCLCHECK(ncclGroupEnd());
  return ret;
}

NCCL_API(ncclResult_t, ncclRecv, void* recvbuff, size_t count, ncclDataType_t datatype, int peer,
    ncclComm_t comm, cudaStream_t stream);
/* 中文导读：
 * 接收侧按 peer 和 count 记录任务，与对端 Send 匹配；count 的单位仍是 datatype 元素。
 * 在本版本中，全互联交换可由应用按 peer 组合 Send/Recv；调度与资源使用取决于实际出现的 peer 集合。
 */
ncclResult_t ncclRecv(void* recvbuff, size_t count, ncclDataType_t datatype, int peer,
    ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  struct ncclInfo info = { ncclFuncRecv, "Recv",
    NULL, recvbuff, count, datatype, ncclSum, peer, comm, stream, /* Args */
    1, 1 };
  ncclResult_t ret;
  NCCLCHECK(ncclGroupStart());
  ret = ncclEnqueueCheck(&info);
  NCCLCHECK(ncclGroupEnd());
  return ret;
}
