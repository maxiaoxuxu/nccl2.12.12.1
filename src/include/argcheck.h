/*************************************************************************
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_ARGCHECK_H_
/* 中文逐项-常量 X0123：NCCL_ARGCHECK_H_
 * 编译期定义：NCCL_ARGCHECK_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_ARGCHECK_H_

#include "core.h"
#include "info.h"

/* 中文逐项-接口 X0124：PtrCheck
 * 接口职责：检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t PtrCheck(void* ptr, const char* opname, const char* ptrname);
 */
ncclResult_t PtrCheck(void* ptr, const char* opname, const char* ptrname);
/* 中文逐项-接口 X0125：ArgsCheck
 * 接口职责：验证 root/peer、类型、归约句柄及可选缓冲指针，并归一化复制型操作尺寸和总字节数。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ArgsCheck(struct ncclInfo* info);
 */
ncclResult_t ArgsCheck(struct ncclInfo* info);

#endif
