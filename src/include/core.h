/*************************************************************************
 * Copyright (c) 2015-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_CORE_H_
/* 中文逐项-常量 X0185：NCCL_CORE_H_
 * 编译期定义：NCCL_CORE_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_CORE_H_

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <algorithm> // For std::min/std::max
#include "nccl.h"

/* 中文精读：公开符号与实现函数的关系
 * NCCL_API 宏为导出的 C API 设置可见性，extern C 避免 C++ 名字改编。
 * PROFAPI 模式还提供 pnccl 前缀别名/弱符号，供性能分析工具拦截常规 API。
 * 这些是主机链接层机制，不是 GPU kernel 入口；kernel 由 collectives 的设备宏单独生成。
 * ncclTypeSize 则是 count 元素数与物理字节数转换的统一小工具。
 */
#ifdef PROFAPI
/* 中文逐项-宏 M0081：NCCL_API
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCL_API；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_API(ret, func, args...)        \
    __attribute__ ((visibility("default"))) \
    __attribute__ ((alias(#func)))          \
    ret p##func (args);                     \
    extern "C"                              \
    __attribute__ ((visibility("default"))) \
    __attribute__ ((weak))                  \
    ret func(args)
#else
/* 中文逐项-宏 M0082：NCCL_API
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCL_API；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_API(ret, func, args...)        \
    extern "C"                              \
    __attribute__ ((visibility("default"))) \
    ret func(args)
#endif // end PROFAPI

/* 中文逐项-函数 F0628：ncclTypeSize
 * 用途与位置：把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。
 * 参数 type（ncclDataType_t type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::computeColl；src/enqueue.cc::ncclSaveP2p；src/enqueue.cc::ncclEnqueueCollKernel；src/enqueue.cc::ncclRedOpCreatePreMulSum；src/misc/argcheck.cc::ArgsCheck；src/transport/coll_net.cc::sendProxyProgress。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return 1;；return 2;；return 4;；return 8;；return -1;。CHECK类宏还可能提前返回/跳转。
 */
static __inline__ int ncclTypeSize(ncclDataType_t type) {
  switch (type) {
    case ncclInt8:
    case ncclUint8:
      return 1;
    case ncclFloat16:
#if defined(__CUDA_BF16_TYPES_EXIST__)
    case ncclBfloat16:
#endif
      return 2;
    case ncclInt32:
    case ncclUint32:
    case ncclFloat32:
      return 4;
    case ncclInt64:
    case ncclUint64:
    case ncclFloat64:
      return 8;
    default:
      return -1;
  }
}

#include "debug.h"
#include "checks.h"
#include "alloc.h"
#include "utils.h"
#include "param.h"
#include "nvtx.h"

#endif // end include guard
