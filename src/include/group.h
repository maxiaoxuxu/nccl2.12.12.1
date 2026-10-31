/*************************************************************************
 * Copyright (c) 2015-2017, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_GROUP_H_
/* 中文逐项-常量 X0327：NCCL_GROUP_H_
 * 编译期定义：NCCL_GROUP_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_GROUP_H_

#include "nccl.h"
#include "comm.h"

/* 中文逐项-接口 X0328：ncclAsyncMode
 * 接口职责：查询本线程是否处于 Group 嵌套内部，决定 API 立即准备还是延迟到最外层 GroupEnd。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：bool ncclAsyncMode();
 */
bool ncclAsyncMode();
/* 中文逐项-接口 X0329：ncclAsyncErrCheck
 * 接口职责：把组内调用错误记录到线程局部 Group 状态，供统一清理和返回。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclAsyncErrCheck(ncclResult_t ret);
 */
ncclResult_t ncclAsyncErrCheck(ncclResult_t ret);

typedef ncclResult_t(*ncclInitFunc_t)(ncclComm_t* newcomm, int ndev, ncclUniqueId commId, int myrank, int cudaDev);

/* 中文逐项-接口 X0330：ncclAsyncInit
 * 接口职责：把初始化函数、rank、设备和 uniqueId 复制到本线程 Group 任务数组。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclAsyncInit(ncclInitFunc_t func, ncclComm_t* newcomm, int ndev,
 * ncclUniqueId commId, int myrank, int cudaDev);
 */
ncclResult_t ncclAsyncInit(ncclInitFunc_t func, ncclComm_t* newcomm, int ndev, ncclUniqueId commId, int myrank, int cudaDev);

typedef ncclResult_t(*ncclCollFunc_t)(const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t type, ncclRedOp_t op, int root, ncclComm_t comm, cudaStream_t stream);

/* 中文逐项-接口 X0331：ncclAsyncColl
 * 接口职责：把 communicator 加入本批 Group 提交集合，重复遇到相同 comm 时避免多次登记。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclAsyncColl(ncclComm_t comm);
 */
ncclResult_t ncclAsyncColl(ncclComm_t comm);
#endif
