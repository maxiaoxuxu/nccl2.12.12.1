/*************************************************************************
 * Copyright (c) 2017-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_PARAM_H_
#define NCCL_PARAM_H_

#include <stdint.h>

const char* userHomeDir();
void setEnvFile(const char* fileName);
void initEnv();

void ncclLoadParam(char const* env, int64_t deftVal, int64_t uninitialized, int64_t* cache);

/* 中文精读：NCCL_ 环境参数怎样变成 C++ 函数
 * NCCL_PARAM(IbQpsPerConn,"IB_QPS_PER_CONNECTION",1) 这类宏会生成带静态缓存的 getter。
 * 首次调用读取环境值，之后用原子读返回缓存，减少热路径 getenv/解析开销。
 * 因此在进程运行中改环境变量不保证该参数立即生效，通常应在启动 NCCL 前设置。
 * 这是本版实现约定，具体参数最终用在哪里仍须跟踪调用点。
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
