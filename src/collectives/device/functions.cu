/*************************************************************************
 * Copyright (c) 2015-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "common.h"

/* 中文逐项-变量 V0442：ncclShmemData
 * ncclShmemData（__shared__；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
__shared__ ncclShmemData ncclShmem;

/* 中文逐项-宏 M0027：NCCL_FUNC5
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNC5；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNC5(func, algo, devredop, type, nullify) \
  MACRO_IF(nullify, nullptr, NCCL_FUNC_NAME(func, algo, LL,     devredop, type)), \
  MACRO_IF(nullify, nullptr, NCCL_FUNC_NAME(func, algo, LL128,  devredop, type)), \
  MACRO_IF(nullify, nullptr, NCCL_FUNC_NAME(func, algo, SIMPLE, devredop, type))

/* 中文逐项-宏 M0028：NCCL_FUNC4
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNC4；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNC4(func, devredop, type, nullify) \
  NCCL_FUNC5(func, TREE,    devredop, type, nullify), \
  NCCL_FUNC5(func, RING,    devredop, type, nullify), \
  NCCL_FUNC5(func, COLLNET, devredop, type, nullify)

#if defined(__CUDA_BF16_TYPES_EXIST__)
// Must be consistent with ncclDataType_t
/* 中文逐项-宏 M0029：NCCL_FUNCS3A
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3A；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS3A(func, devredop, nullForFloat) \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, uint8_t, 0), \
  NCCL_FUNC4(func, devredop, int32_t, 0), \
  NCCL_FUNC4(func, devredop, uint32_t, 0), \
  NCCL_FUNC4(func, devredop, int64_t, 0), \
  NCCL_FUNC4(func, devredop, uint64_t, 0), \
  NCCL_FUNC4(func, devredop, half, nullForFloat), \
  NCCL_FUNC4(func, devredop, float, nullForFloat), \
  NCCL_FUNC4(func, devredop, double, nullForFloat), \
  NCCL_FUNC4(func, devredop, __nv_bfloat16, nullForFloat)
/* 中文逐项-宏 M0030：NCCL_FUNCS3B
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3B；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS3B(func, devredop) \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0)
#else
// Must be consistent with ncclDataType_t
/* 中文逐项-宏 M0031：NCCL_FUNCS3A
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3A；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS3A(func, devredop, nullForFloat) \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, uint8_t, 0), \
  NCCL_FUNC4(func, devredop, int32_t, 0), \
  NCCL_FUNC4(func, devredop, uint32_t, 0), \
  NCCL_FUNC4(func, devredop, int64_t, 0), \
  NCCL_FUNC4(func, devredop, uint64_t, 0), \
  NCCL_FUNC4(func, devredop, half, nullForFloat), \
  NCCL_FUNC4(func, devredop, float, nullForFloat), \
  NCCL_FUNC4(func, devredop, double, nullForFloat)
/* 中文逐项-宏 M0032：NCCL_FUNCS3B
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3B；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS3B(func, devredop) \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0), \
  NCCL_FUNC4(func, devredop, int8_t, 0)
#endif

// Must be consistent with ncclRedOp_t
/* 中文逐项-宏 M0033：NCCL_FUNCS3A
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3A；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS2A(func) \
  NCCL_FUNCS3A(func, Sum,        /*nullForFloat=*/0), \
  NCCL_FUNCS3A(func, Prod,       /*nullForFloat=*/0), \
  NCCL_FUNCS3A(func, Max,        /*nullForFloat=*/0), \
  NCCL_FUNCS3A(func, Min,        /*nullForFloat=*/0), \
  NCCL_FUNCS3A(func, PreMulSum,  /*nullForFloat=*/0), \
  NCCL_FUNCS3A(func, SumPostDiv, /*nullForFloat=*/1)

/* 中文逐项-宏 M0034：NCCL_FUNCS2B
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS2B；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS2B(func) \
  NCCL_FUNCS3B(func, Sum), \
  NCCL_FUNCS3B(func, Sum), \
  NCCL_FUNCS3B(func, Sum), \
  NCCL_FUNCS3B(func, Sum), \
  NCCL_FUNCS3B(func, Sum), \
  NCCL_FUNCS3B(func, Sum)

// Must be consistent with the ncclFuncSet enum
/* 中文精读：设备函数表怎样被填满
 * 宏按 collective/归约/数据类型/算法/协议展开函数地址，排列与 FUNC_INDEX 严格一致。
 * Broadcast/AllGather 仅复制，以 Int8/Sum 的实现填充相应槽；没有真的做跨 rank 求和。
 * 并非所有声明组合都具备实际可用算法，调优模型须屏蔽不可用的组合。
 * __CUDA_ARCH__ 条件只在设备编译阶段初始化函数指针，避免主机编译器误取设备函数地址。
 */
__device__ /* 中文逐项-变量 V0443：ncclFuncs
 * ncclFuncs（ncclKern_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclFuncs = { // Don't try to initialize the host shadow copy of this device-side global // variable. There
 * is no host pointer to a device-side function, which // confuses clang. This will be fixed in the next clang
 * release. #if __CUDA_ARCH__ …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */ncclKern_t ncclFuncs[1+ncclNumTypes+NCCL_NUM_FUNCTIONS*ncclNumDevRedOps*ncclNumTypes*NCCL_NUM_ALGORITHMS*NCCL_NUM_PROTOCOLS] = {
// Don't try to initialize the host shadow copy of this device-side global
// variable. There is no host pointer to a device-side function, which
// confuses clang. This will be fixed in the next clang release.
#if __CUDA_ARCH__
  NCCL_FUNC_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, int8_t),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, uint8_t),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, int32_t),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, uint32_t),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, int64_t),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, uint64_t),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, half),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, float),
  NCCL_ONERANK_REDUCE_NAME(PreMulSum, double),
  #if defined(__CUDA_BF16_TYPES_EXIST__)
    NCCL_ONERANK_REDUCE_NAME(PreMulSum, __nv_bfloat16),
  #endif
  NCCL_FUNCS2B(Broadcast),
  NCCL_FUNCS2A(Reduce),
  NCCL_FUNCS2B(AllGather),
  NCCL_FUNCS2A(ReduceScatter),
  NCCL_FUNCS2A(AllReduce)
#endif
};

// Workaround for https://reviews.llvm.org/D55580
__device__ /* 中文内部逻辑 F0128：ncclWorkaroundClangD55580
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__functions_cu.md#f0128
 */
/* 中文逐项-函数 F0128：ncclWorkaroundClangD55580
 * 用途与位置：保留空设备符号作为特定 clang 设备链接问题的变通，不承担通信逻辑。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 */void ncclWorkaroundClangD55580() {}
