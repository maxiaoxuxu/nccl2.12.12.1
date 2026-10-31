/*************************************************************************
 * Copyright (c) 2017-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_COLLECTIVES_H_
/* 中文逐项-常量 X0148：NCCL_COLLECTIVES_H_
 * 编译期定义：NCCL_COLLECTIVES_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_COLLECTIVES_H_

/* 中文逐项-类型 T0162：ncclDevRedOp_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclDevRedOp_t {
/* 中文逐项-枚举项 X0149：ncclDevSum
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclDevSum, /* 中文逐项-枚举项 X0150：ncclDevProd
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclDevProd, /* 中文逐项-枚举项 X0151：ncclDevMax
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclDevMax, /* 中文逐项-枚举项 X0152：ncclDevMin
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclDevMin,
/* 中文逐项-枚举项 X0153：ncclDevPreMulSum
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclDevPreMulSum, /* 中文逐项-枚举项 X0154：ncclDevSumPostDiv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclDevSumPostDiv,
/* 中文逐项-枚举项 X0155：ncclNumDevRedOps
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclNumDevRedOps
};
/* 中文逐项-类型 T0163：ncclDevRedOpFull
 * 类型职责：用户归约句柄槽及设备可执行的归约操作/标量表示，避免把主机句柄直接交给设备模板。
 * 声明内可见成员（含嵌套成员）：op、scalarArgIsPtr、scalarArg。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclDevRedOpFull {
/* 中文逐项-变量 V1986：op
 * op（ncclDevRedOp_t；成员 ncclDevRedOpFull）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 */
  ncclDevRedOp_t op;
/* 中文逐项-变量 V1987：scalarArgIsPtr
 * scalarArgIsPtr（bool；成员 ncclDevRedOpFull）：该标量是否保存设备地址；设备执行前须解引用才能用于预处理。
 */
  bool scalarArgIsPtr;
/* 中文逐项-变量 V1988：scalarArg
 * scalarArg（uint64_t；成员 ncclDevRedOpFull）：归约标量的位模式容器或设备指针，需结合对应 IsPtr 标志理解。
 */
  uint64_t scalarArg;
};

/* 中文精读：主机和设备必须一致的函数表下标
 * 0 留给 SendRecv，后面先放单 rank 归约函数，再按操作/归约/类型/算法/协议展开。
 * FUNC_INDEX 把多维索引线性化，enqueue 的 ncclKerns 与 device/functions.cu 的 ncclFuncs
 * 必须遵守同一排列；否则会把正确数据交给错误算法或错误类型的设备函数。
 * 模板实例在构建时生成，这不是运行时根据字符串编译 CUDA 源码。
 */
/* 中文逐项-常量 X0156：FUNC_INDEX_P2P
 * 编译期定义：FUNC_INDEX_P2P = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define FUNC_INDEX_P2P 0
/* 中文逐项-宏 M0067：FUNC_INDEX
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 FUNC_INDEX；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define FUNC_INDEX(func, devredop, ncclType, al, pr) (1+ncclNumTypes+(((((func)*ncclNumDevRedOps + (devredop))*ncclNumTypes) + (ncclType))*NCCL_NUM_ALGORITHMS+(al))*NCCL_NUM_PROTOCOLS+(pr))

/* 中文逐项-宏 M0068：NCCL_FUNC_NAME
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNC_NAME；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNC_NAME(func, algo, proto, devredop, type) \
  ncclFunction_##func##_##algo##_##proto##_##devredop##_##type

/* 中文逐项-宏 M0069：NCCL_ONERANK_REDUCE_NAME
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCL_ONERANK_REDUCE_NAME；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_ONERANK_REDUCE_NAME(devredop, type) \
  ncclFunction_OneRankReduce_##devredop##_##type

/* 中文逐项-宏 M0070：NCCL_KERN_NAME
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_KERN_NAME；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_KERN_NAME(func, algo, proto, devredop, type) \
  ncclKernel_##func##_##algo##_##proto##_##devredop##_##type

/* 中文逐项-宏 M0071：NCCL_IMPL_NAME
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCL_IMPL_NAME；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_IMPL_NAME(func, algo, proto) \
  nccl##func##algo##proto

/* Declare all collective operations */
/* 中文逐项-宏 M0072：DECL5
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 DECL5；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define DECL5(func, algo, proto, devredop, type) \
  extern __device__ void NCCL_FUNC_NAME(func, algo, proto, devredop, type)(); \
  extern __global__ void NCCL_KERN_NAME(func, algo, proto, devredop, type)(struct ncclDevComm* comm, struct ncclWorkElem c); \

/* 中文逐项-宏 M0073：CONCAT
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 CONCAT；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define CONCAT(a,b) a##b
/* 中文逐项-宏 M0074：MACRO_IF
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 MACRO_IF；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define MACRO_IF(cond, t, f) CONCAT(MACRO_IF_, cond)(t, f)
/* 中文逐项-宏 M0075：MACRO_IF_0
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 MACRO_IF_0；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define MACRO_IF_0(t, f) f
/* 中文逐项-宏 M0076：MACRO_IF_1
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 MACRO_IF_1；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define MACRO_IF_1(t, f) t

/* 中文逐项-宏 M0077：DECL5
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 DECL5；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define DECL4(func, algo, devredop, type, undef) \
  MACRO_IF(undef, /*undefined*/, DECL5(func, algo, SIMPLE, devredop, type)) \
  MACRO_IF(undef, /*undefined*/, DECL5(func, algo, LL,     devredop, type)) \
  MACRO_IF(undef, /*undefined*/, DECL5(func, algo, LL128,  devredop, type))

/* 中文逐项-宏 M0078：DECL3
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 DECL3；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define DECL3(func, devredop, type, undef) \
  DECL4(func, RING,    devredop, type, undef) \
  DECL4(func, TREE,    devredop, type, undef) \
  DECL4(func, COLLNET, devredop, type, undef)

#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-常量 X0157：undefForFloat
 * 编译期定义：undefForFloat = )。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define DECL2(func, devredop, undefForFloat) \
  DECL3(func, devredop, int8_t, /*undef=*/0) \
  DECL3(func, devredop, uint8_t, /*undef=*/0) \
  DECL3(func, devredop, int32_t, /*undef=*/0) \
  DECL3(func, devredop, uint32_t, /*undef=*/0) \
  DECL3(func, devredop, int64_t, /*undef=*/0) \
  DECL3(func, devredop, uint64_t, /*undef=*/0) \
  DECL3(func, devredop, half, /*undef=*/undefForFloat) \
  DECL3(func, devredop, float, /*undef=*/undefForFloat) \
  DECL3(func, devredop, double, /*undef=*/undefForFloat) \
  DECL3(func, devredop, __nv_bfloat16, /*undef=*/undefForFloat)
#else
/* 中文逐项-宏 M0079：DECL3
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 DECL3；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define DECL2(func, devredop, undefForFloat) \
  DECL3(func, devredop, int8_t, /*undef=*/0) \
  DECL3(func, devredop, uint8_t, /*undef=*/0) \
  DECL3(func, devredop, int32_t, /*undef=*/0) \
  DECL3(func, devredop, uint32_t, /*undef=*/0) \
  DECL3(func, devredop, int64_t, /*undef=*/0) \
  DECL3(func, devredop, uint64_t, /*undef=*/0) \
  DECL3(func, devredop, half, /*undef=*/undefForFloat) \
  DECL3(func, devredop, float, /*undef=*/undefForFloat) \
  DECL3(func, devredop, double, /*undef=*/undefForFloat)
#endif

/* 中文逐项-宏 M0080：DECL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 DECL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define DECL(func) \
  DECL2(func, Sum, /*undefForFloat=*/0) \
  DECL2(func, Prod, /*undefForFloat=*/0) \
  DECL2(func, Min, /*undefForFloat=*/0) \
  DECL2(func, Max, /*undefForFloat=*/0) \
  DECL2(func, PreMulSum, /*undefForFloat=*/0) \
  DECL2(func, SumPostDiv, /*undefForFloat=*/1)

DECL2(Broadcast, Sum, /*undefForFloat=*/0)
DECL(Reduce)
DECL2(AllGather, Sum, /*undefForFloat=*/0)
DECL(ReduceScatter)
DECL(AllReduce)
DECL5(SendRecv, RING, SIMPLE, Sum, int8_t)

extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, int8_t)();
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, uint8_t)();
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, int32_t)();
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, uint32_t)();
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, int64_t)();
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, uint64_t)();
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, half)();
#if defined(__CUDA_BF16_TYPES_EXIST__)
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, __nv_bfloat16)();
#endif
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, float)();
extern __device__ void NCCL_ONERANK_REDUCE_NAME(PreMulSum, double)();

// CHUNKSIZE must be a multiple of SLICESIZE
/* 中文逐项-常量 X0158：ALLREDUCE_SLICESTEPS
 * 编译期定义：ALLREDUCE_SLICESTEPS = (NCCL_STEPS/4)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ALLREDUCE_SLICESTEPS (NCCL_STEPS/4)
/* 中文逐项-常量 X0159：ALLREDUCE_CHUNKSTEPS
 * 编译期定义：ALLREDUCE_CHUNKSTEPS = (NCCL_STEPS/2)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ALLREDUCE_CHUNKSTEPS (NCCL_STEPS/2)
/* 中文逐项-常量 X0160：ALLGATHER_SLICESTEPS
 * 编译期定义：ALLGATHER_SLICESTEPS = (NCCL_STEPS/4)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ALLGATHER_SLICESTEPS (NCCL_STEPS/4)
/* 中文逐项-常量 X0161：ALLGATHER_CHUNKSTEPS
 * 编译期定义：ALLGATHER_CHUNKSTEPS = (NCCL_STEPS/2)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ALLGATHER_CHUNKSTEPS (NCCL_STEPS/2)
/* 中文逐项-常量 X0162：REDUCESCATTER_SLICESTEPS
 * 编译期定义：REDUCESCATTER_SLICESTEPS = (NCCL_STEPS/4)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define REDUCESCATTER_SLICESTEPS (NCCL_STEPS/4)
/* 中文逐项-常量 X0163：REDUCESCATTER_CHUNKSTEPS
 * 编译期定义：REDUCESCATTER_CHUNKSTEPS = (NCCL_STEPS/2)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define REDUCESCATTER_CHUNKSTEPS (NCCL_STEPS/2)
/* 中文逐项-常量 X0164：BROADCAST_SLICESTEPS
 * 编译期定义：BROADCAST_SLICESTEPS = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define BROADCAST_SLICESTEPS 1
/* 中文逐项-常量 X0165：BROADCAST_CHUNKSTEPS
 * 编译期定义：BROADCAST_CHUNKSTEPS = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define BROADCAST_CHUNKSTEPS 1
/* 中文逐项-常量 X0166：REDUCE_SLICESTEPS
 * 编译期定义：REDUCE_SLICESTEPS = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define REDUCE_SLICESTEPS 1
/* 中文逐项-常量 X0167：REDUCE_CHUNKSTEPS
 * 编译期定义：REDUCE_CHUNKSTEPS = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define REDUCE_CHUNKSTEPS 1
/* 中文逐项-常量 X0168：SENDRECV_SLICEFACTOR
 * 编译期定义：SENDRECV_SLICEFACTOR = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define SENDRECV_SLICEFACTOR 4
/* 中文逐项-常量 X0169：NCCL_MAX_SLICE_PER_CHUNK
 * 编译期定义：NCCL_MAX_SLICE_PER_CHUNK = 2 // max value for CHUNKSTEPS/SLICESTEPS, must accord with above。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MAX_SLICE_PER_CHUNK 2  // max value for CHUNKSTEPS/SLICESTEPS, must accord with above

#endif
