/*************************************************************************
 * Copyright (c) 2017-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_PARAM_H_
/* 中文逐项-常量 X1044：NCCL_PARAM_H_
 * 编译期定义：NCCL_PARAM_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PARAM_H_

#include <stdint.h>

/* 中文逐项-变量 V3054：userHomeDir
 * userHomeDir（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
const char* userHomeDir();
/* 中文逐项-接口 X1045：setEnvFile
 * 接口职责：读取 KEY=VALUE 配置行并只填入尚未存在的环境键，保留启动环境的优先级。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void setEnvFile(const char* fileName);
 */
void setEnvFile(const char* fileName);
/* 中文逐项-接口 X1046：initEnv
 * 接口职责：按用户配置、系统配置顺序加载配置文件，已有键不会被后读文件覆盖。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void initEnv();
 */
void initEnv();

/* 中文逐项-接口 X1047：ncclLoadParam
 * 接口职责：在互斥锁内首次读取/解析数值参数并写原子缓存，后续 getter 直接复用缓存值。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void ncclLoadParam(char const* env, int64_t deftVal, int64_t uninitialized,
 * int64_t* cache);
 */
void ncclLoadParam(char const* env, int64_t deftVal, int64_t uninitialized, int64_t* cache);

/* 中文精读：NCCL_ 环境参数怎样变成 C++ 函数
 * NCCL_PARAM(IbQpsPerConn,"IB_QPS_PER_CONNECTION",1) 这类宏会生成带静态缓存的 getter。
 * 首次调用读取环境值，之后用原子读返回缓存，减少热路径 getenv/解析开销。
 * 因此在进程运行中改环境变量不保证该参数立即生效，通常应在启动 NCCL 前设置。
 * 这是本版实现约定，具体参数最终用在哪里仍须跟踪调用点。
 */
/* 中文逐项-宏 M0122：NCCL_PARAM
 * 宏职责：生成带进程静态缓存的数值参数getter，首次读取NCCL_前缀环境变量并按默认值解析，之后原子读缓存。
 * 原始宏名 NCCL_PARAM；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_PARAM(name, env, deftVal) \
  int64_t ncclParam##name() { \
    constexpr int64_t uninitialized = INT64_MIN; \
    static_assert(deftVal != uninitialized, "default value cannot be the uninitialized value."); \
    static int64_t cache = uninitialized; \
    if (__builtin_expect(__atomic_load_n(&cache, __ATOMIC_RELAXED) == uninitialized, false)) { \
      ncclLoadParam("NCCL_" env, deftVal, uninitialized, &cache); \
    } \
    return cache; \
  }

#endif
