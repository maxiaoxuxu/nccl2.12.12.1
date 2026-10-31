/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_BOOTSTRAP_H_
/* 中文逐项-常量 X0126：NCCL_BOOTSTRAP_H_
 * 编译期定义：NCCL_BOOTSTRAP_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_BOOTSTRAP_H_

#include "nccl.h"
#include "comm.h"

/* 中文逐项-接口 X0127：bootstrapNetInit
 * 接口职责：选择初始化控制面使用的本机网络接口和地址；其选择不等于数据面必然使用 TCP。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapNetInit();
 */
ncclResult_t bootstrapNetInit();
/* 中文逐项-接口 X0128：bootstrapCreateRoot
 * 接口职责：创建 bootstrap root 监听对象和后台线程，为后续 rank 会合提供入口。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapCreateRoot(ncclUniqueId* commId, bool idFromEnv);
 */
ncclResult_t bootstrapCreateRoot(ncclUniqueId* commId, bool idFromEnv);
/* 中文逐项-接口 X0129：bootstrapGetUniqueId
 * 接口职责：生成/读取引导会合标识供应用分发，必要时建立 root 服务。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapGetUniqueId(ncclUniqueId* out);
 */
ncclResult_t bootstrapGetUniqueId(ncclUniqueId* out);
/* 中文逐项-接口 X0130：bootstrapInit
 * 接口职责：让当前 rank 加入控制网络并交换 peer/proxy 地址，为拓扑发现和传输建链准备控制通道。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapInit(ncclUniqueId* id, struct ncclComm* comm);
 */
ncclResult_t bootstrapInit(ncclUniqueId* id, struct ncclComm* comm);
/* 中文逐项-接口 X0131：bootstrapAllGather
 * 接口职责：沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapAllGather(void* commState, void* allData, int size);
 */
ncclResult_t bootstrapAllGather(void* commState, void* allData, int size);
/* 中文逐项-接口 X0132：bootstrapSend
 * 接口职责：向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapSend(void* commState, int peer, int tag, void* data, int
 * size);
 */
ncclResult_t bootstrapSend(void* commState, int peer, int tag, void* data, int size);
/* 中文逐项-接口 X0133：bootstrapRecv
 * 接口职责：优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapRecv(void* commState, int peer, int tag, void* data, int
 * size);
 */
ncclResult_t bootstrapRecv(void* commState, int peer, int tag, void* data, int size);
/* 中文逐项-接口 X0134：bootstrapBarrier
 * 接口职责：使用倍增距离的控制消息交换，使所选 rank 子集合完成初始化会合。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapBarrier(void* commState, int *ranks, int rank, int nranks,
 * int tag);
 */
ncclResult_t bootstrapBarrier(void* commState, int *ranks, int rank, int nranks, int tag);
/* 中文逐项-接口 X0135：bootstrapIntraNodeAllGather
 * 接口职责：只在所选节点内 rank 集合交换固定大小控制信息，按局部索引映射到全局 rank。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapIntraNodeAllGather(void* commState, int *ranks, int rank, int
 * nranks, void* allData, int size);
 */
ncclResult_t bootstrapIntraNodeAllGather(void* commState, int *ranks, int rank, int nranks, void* allData, int size);
/* 中文逐项-接口 X0136：bootstrapClose
 * 接口职责：在正常生命周期结束时关闭 bootstrap socket 并回收地址表、暂存项和状态。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapClose(void* commState);
 */
ncclResult_t bootstrapClose(void* commState);
/* 中文逐项-接口 X0137：bootstrapAbort
 * 接口职责：异常退出时关闭控制网络并回收状态，配合中止流程打破初始化等待。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t bootstrapAbort(void* commState);
 */
ncclResult_t bootstrapAbort(void* commState);
#endif
