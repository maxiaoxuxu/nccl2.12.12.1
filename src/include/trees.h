/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_TREES_H_
/* 中文逐项-常量 X1138：NCCL_TREES_H_
 * 编译期定义：NCCL_TREES_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TREES_H_

/* 中文逐项-接口 X1139：ncclGetBtree
 * 接口职责：根据 rank 最低置位构造二叉树父/子编号，并处理非二次幂规模的越界孩子。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclGetBtree(int nranks, int rank, int* u0, int* d1, int* d0, int*
 * parentChildType);
 */
ncclResult_t ncclGetBtree(int nranks, int rank, int* u0, int* d1, int* d0, int* parentChildType);
/* 中文逐项-接口 X1140：ncclGetDtree
 * 接口职责：在第一棵二叉树基础上用镜像或平移构造互补树，分摊中间节点负载。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclGetDtree(int nranks, int rank, int* u0, int* d0_0, int* d0_1, int*
 * parentChildType0, int* u1, int* d1_0, int* d1_1, int* parentChildType1);
 */
ncclResult_t ncclGetDtree(int nranks, int rank, int* u0, int* d0_0, int* d0_1, int* parentChildType0, int* u1, int* d1_0, int* d1_1, int* parentChildType1);

#endif
