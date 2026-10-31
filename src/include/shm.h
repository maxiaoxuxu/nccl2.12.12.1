/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_SHM_H_
/* 中文逐项-常量 X1093：NCCL_SHM_H_
 * 编译期定义：NCCL_SHM_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SHM_H_

#include "nccl.h"

/* 中文逐项-接口 X1094：ncclShmOpen
 * 接口职责：在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclShmOpen(char* shmPath, const int shmSize, void** shmPtr, void**
 * devShmPtr, int create);
 */
ncclResult_t ncclShmOpen(char* shmPath, const int shmSize, void** shmPtr, void** devShmPtr, int create);
/* 中文逐项-接口 X1095：ncclShmUnlink
 * 接口职责：删除共享内存文件名，已打开的映射仍由各进程按生命周期单独解除。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclShmUnlink(const char* shmname);
 */
ncclResult_t ncclShmUnlink(const char* shmname);
/* 中文逐项-接口 X1096：ncclShmClose
 * 接口职责：先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclShmClose(void* shmPtr, void* devShmPtr, const int shmSize);
 */
ncclResult_t ncclShmClose(void* shmPtr, void* devShmPtr, const int shmSize);
#endif
