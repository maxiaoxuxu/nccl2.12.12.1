/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "reduce.h"
#include "common.h"
#include "collectives.h"

/* 中文精读：这个短 .cu 文件是模板实例化入口
 * 真正算法在同名 .h 中，common.h 定义 IMPL_COLL 宏，此处为归约操作生成设备函数/必要 kernel。
 * gen_rules.sh 用不同 NCCL_OP/NCCL_TYPE 组合多次编译此文件，覆盖支持的数据类型与归约函数。
 * 设备函数最后由 functions.cu 排入表，主机 enqueue.cc 按 FUNC_INDEX 选择入口和 work 函数索引。
 * 所以不能只因 .cu 只有几行就认为通信算法在外部库里；请继续进入同名头文件。
 */
IMPL_COLL_R(Reduce);
