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
/* 中文逐项-函数 F0379：ncclSend
 * 用途与位置：把消息按 peer 加入 P2P 排队，通过嵌套 Group 支持批量匹配发射；不在此直接执行网络发送。
 * 参数 sendbuff（const void* sendbuff）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 count（size_t count）：公开API输入的datatype元素数；这里尚未归一化成字节，实际字节长度=count*ncclTypeSize(datatype)。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclEnqueueCheck；ncclGroupEnd；ncclGroupStart。
 * 源码返回表达式：return ret;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSend(const void* sendbuff, size_t count, ncclDataType_t datatype, int peer,
    ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文逐项-变量 V0854：info
 * info（struct ncclInfo；函数 ncclSend 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncSend, "Send", NULL, (void*)sendbuff, count, datatype, ncclSum, peer, comm, stream, / * Args
 * * / 1, 1 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncSend, "Send",
    NULL, (void*)sendbuff, count, datatype, ncclSum, peer, comm, stream, /* Args */
    1, 1 };
/* 中文逐项-变量 V0855：ret
 * ret（ncclResult_t；函数 ncclSend 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：ret = ncclEnqueueCheck(&info);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
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
/* 中文逐项-函数 F0380：ncclRecv
 * 用途与位置：把匹配 peer 的接收加入 P2P 排队，实际缓冲搬运在设备原语与必要的 Proxy 中完成。
 * 参数 recvbuff（void* recvbuff）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 count（size_t count）：公开API输入的datatype元素数；这里尚未归一化成字节，实际字节长度=count*ncclTypeSize(datatype)。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclEnqueueCheck；ncclGroupEnd；ncclGroupStart。
 * 源码返回表达式：return ret;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclRecv(void* recvbuff, size_t count, ncclDataType_t datatype, int peer,
    ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文逐项-变量 V0856：info
 * info（struct ncclInfo；函数 ncclRecv 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncRecv, "Recv", NULL, recvbuff, count, datatype, ncclSum, peer, comm, stream, / * Args * / 1,
 * 1 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncRecv, "Recv",
    NULL, recvbuff, count, datatype, ncclSum, peer, comm, stream, /* Args */
    1, 1 };
/* 中文逐项-变量 V0857：ret
 * ret（ncclResult_t；函数 ncclRecv 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：ret = ncclEnqueueCheck(&info);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t ret;
  NCCLCHECK(ncclGroupStart());
  ret = ncclEnqueueCheck(&info);
  NCCLCHECK(ncclGroupEnd());
  return ret;
}
