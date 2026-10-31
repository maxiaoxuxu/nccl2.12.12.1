/*************************************************************************
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

/* 中文逐项-接口 X0028：ncclBuildRings
 * 接口职责：跟随 next 构建每 channel 环顺序，验证 nranks 步回到起点且没有遗漏参与 rank。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclBuildRings(int nrings, int* rings, int rank, int nranks, int*
 * prev, int* next);
 */
ncclResult_t ncclBuildRings(int nrings, int* rings, int rank, int nranks, int* prev, int* next);
