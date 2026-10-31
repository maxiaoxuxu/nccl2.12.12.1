/*************************************************************************
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_NVTX_H_
/* 中文逐项-常量 X0693：NCCL_NVTX_H_
 * 编译期定义：NCCL_NVTX_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NVTX_H_

#include "nvtx3.hpp"

/* 中文逐项-类型 T0318：nccl_domain
 * 类型职责：NVTX性能标记域的静态名字，让外部性能工具识别NCCL主机API范围。
 * 声明内可见成员（含嵌套成员）：name。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct nccl_domain{/* 中文逐项-变量 V2791：name
 * name（char；成员 nccl_domain）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */static constexpr char const* name{"NCCL"};};

#endif
