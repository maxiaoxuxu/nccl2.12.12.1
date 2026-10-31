/*************************************************************************
 * Copyright (c) 2017-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "argcheck.h"
#include "coll_net.h"
#include "gdrwrap.h"
#include "bootstrap.h"
#include "channel.h"

#include <cstring> // std::memcpy

// Only generate inline kernels for LL
/* 中文逐项-宏 M0036：NCCL_FUNC5
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNC5；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNC5(func, algo, devredop, dtype) \
  (void*)NCCL_KERN_NAME(func, algo, LL, devredop, dtype), \
  (void*)NCCL_KERN_NAME(func, algo, LL, devredop, dtype), \
  (void*)NCCL_KERN_NAME(func, algo, LL, devredop, dtype)

/* 中文逐项-宏 M0037：NCCL_FUNC4
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNC4；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNC4(func, devredop, type) \
  (void*)NCCL_FUNC5(func, TREE,    devredop, type), \
  (void*)NCCL_FUNC5(func, RING,    devredop, type), \
  (void*)NCCL_FUNC5(func, COLLNET, devredop, type)

#if defined(__CUDA_BF16_TYPES_EXIST__)
// Must be consistent with ncclDataType_t
/* 中文逐项-宏 M0038：NCCL_FUNCS3A
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3A；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS3A(func, devredop) \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, uint8_t), \
  (void*)NCCL_FUNC4(func, devredop, int32_t), \
  (void*)NCCL_FUNC4(func, devredop, uint32_t), \
  (void*)NCCL_FUNC4(func, devredop, int64_t), \
  (void*)NCCL_FUNC4(func, devredop, uint64_t), \
  (void*)NCCL_FUNC4(func, devredop, half), \
  (void*)NCCL_FUNC4(func, devredop, float), \
  (void*)NCCL_FUNC4(func, devredop, double), \
  (void*)NCCL_FUNC4(func, devredop, __nv_bfloat16)
/* 中文逐项-宏 M0039：NCCL_FUNCS3B
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3B；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS3B(func, devredop) \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t)
#else
// Must be consistent with ncclDataType_t
/* 中文逐项-宏 M0040：NCCL_FUNCS3A
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3A；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS3A(func, devredop) \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, uint8_t), \
  (void*)NCCL_FUNC4(func, devredop, int32_t), \
  (void*)NCCL_FUNC4(func, devredop, uint32_t), \
  (void*)NCCL_FUNC4(func, devredop, int64_t), \
  (void*)NCCL_FUNC4(func, devredop, uint64_t), \
  (void*)NCCL_FUNC4(func, devredop, half), \
  (void*)NCCL_FUNC4(func, devredop, float), \
  (void*)NCCL_FUNC4(func, devredop, double)
/* 中文逐项-宏 M0041：NCCL_FUNCS3B
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS3B；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS3B(func, devredop) \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t), \
  (void*)NCCL_FUNC4(func, devredop, int8_t)
#endif

// Must be consistent with ncclDevRedOp_t -- but we only generate kernel for sums.
/* 中文逐项-宏 M0042：NCCL_FUNCS2A
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS2A；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS2A(func) \
  NCCL_FUNCS3A(func, Sum), /*Sum*/ \
  NCCL_FUNCS3A(func, Sum), /*Prod*/ \
  NCCL_FUNCS3A(func, Sum), /*Max*/ \
  NCCL_FUNCS3A(func, Sum), /*Min*/ \
  NCCL_FUNCS3A(func, Sum), /*PreMulSum*/ \
  NCCL_FUNCS3A(func, Sum)  /*SumPostDiv*/
/* 中文逐项-宏 M0043：NCCL_FUNCS2B
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 NCCL_FUNCS2B；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_FUNCS2B(func) \
  NCCL_FUNCS3B(func, Sum), /*Sum*/ \
  NCCL_FUNCS3B(func, Sum), /*Prod*/ \
  NCCL_FUNCS3B(func, Sum), /*Max*/ \
  NCCL_FUNCS3B(func, Sum), /*Min*/ \
  NCCL_FUNCS3B(func, Sum), /*PreMulSum*/ \
  NCCL_FUNCS3B(func, Sum)  /*SumPostDiv*/

// Must be consistent with the ncclFuncSet enum
/* 中文逐项-变量 V0887：ncclKerns
 * ncclKerns（void；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclKerns = { (void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t), // We don't bake special kernels
 * for the one-rank reductions / *int8* /(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t), / *uint8*
 * /(void*)NCCL_KERN_NAME(Send …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static void* const ncclKerns[1+ncclNumTypes+NCCL_NUM_FUNCTIONS*ncclNumDevRedOps*ncclNumTypes*NCCL_NUM_ALGORITHMS*NCCL_NUM_PROTOCOLS] = {
  (void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  // We don't bake special kernels for the one-rank reductions
  /*int8*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  /*uint8*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  /*int32*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  /*uint32*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  /*int64*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  /*uint64*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  /*half*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  /*float*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  /*double*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  #if defined(__CUDA_BF16_TYPES_EXIST__)
    /*bfloat16*/(void*)NCCL_KERN_NAME(SendRecv, RING, SIMPLE, Sum, int8_t),
  #endif
  NCCL_FUNCS2B(Broadcast),
  NCCL_FUNCS2A(Reduce),
  NCCL_FUNCS2B(AllGather),
  NCCL_FUNCS2A(ReduceScatter),
  NCCL_FUNCS2A(AllReduce)
};

// Determine the maximum kernel stack size of all CUDA kernels
/* 中文逐项-函数 F0389：ncclKernMaxLocalSize
 * 用途与位置：遍历入口 kernel 属性以求最大局部内存使用，供初始化设备栈/资源配置参考。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclInit。
 * 本体直接出现的调用/包装：CUDACHECKGOTO；cudaFuncGetAttributes。
 * 需要对照的边界/推进条件：i < numNcclKerns；attr.localSizeBytes > max。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (res != ncclSuccess) ? 0 : max;。CHECK类宏还可能提前返回/跳转。
 */
size_t ncclKernMaxLocalSize() {
/* 中文逐项-变量 V0888：res
 * res（ncclResult_t；文件/静态存储作用域）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  ncclResult_t res = ncclSuccess;
/* 中文逐项-变量 V0889：numNcclKerns
 * numNcclKerns（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 numNcclKerns = sizeof(ncclKerns)/sizeof(ncclKerns[0])；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int numNcclKerns = sizeof(ncclKerns)/sizeof(ncclKerns[0]);
/* 中文逐项-变量 V0890：attr
 * attr（cudaFuncAttributes；文件/静态存储作用域）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 初值/来源 attr = {0}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  cudaFuncAttributes attr = {0};
/* 中文逐项-变量 V0891：max
 * max（size_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 max = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  size_t max = 0;
  for (/* 中文逐项-变量 V0892：i
 * i（int；文件/静态存储作用域）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < numNcclKerns；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < numNcclKerns; i++) {
    CUDACHECKGOTO(cudaFuncGetAttributes(&attr, ncclKerns[i]), res, error);
    if (attr.localSizeBytes > max) max = attr.localSizeBytes;
  }

error:
  return (res != ncclSuccess) ? 0 : max;
}

// Set shared memory carveout for the nccl kernels
/* 中文逐项-函数 F0390：ncclKernSetSharedMemoryCarveout
 * 用途与位置：给受支持的 CUDA kernel 入口设置共享内存划分偏好，影响 SM 资源分配而不改算法语义。
 * 参数 carveOut（int carveOut）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclInit。
 * 本体直接出现的调用/包装：CUDACHECKGOTO；cudaFuncSetAttribute。
 * 需要对照的边界/推进条件：i < numNcclKerns。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return res;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclKernSetSharedMemoryCarveout(int carveOut) {
/* 中文逐项-变量 V0893：res
 * res（ncclResult_t；函数 ncclKernSetSharedMemoryCarveout 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  ncclResult_t res = ncclSuccess;
/* 中文逐项-变量 V0894：numNcclKerns
 * numNcclKerns（int；函数 ncclKernSetSharedMemoryCarveout 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 numNcclKerns = sizeof(ncclKerns)/sizeof(ncclKerns[0])；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int numNcclKerns = sizeof(ncclKerns)/sizeof(ncclKerns[0]);
  for (/* 中文逐项-变量 V0895：i
 * i（int；文件/静态存储作用域）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < numNcclKerns；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < numNcclKerns; i++) {
    CUDACHECKGOTO(cudaFuncSetAttribute(ncclKerns[i], cudaFuncAttributePreferredSharedMemoryCarveout, carveOut), res, error);
  }

error:
  return res;
}


/*****************************************************************************/
/*       Launch system : synchronization and CUDA kernel launch              */
/*****************************************************************************/

/* 中文逐项-函数 F0391：ncclLaunchCooperativeKernelMultiDevice
 * 用途与位置：优先用受支持的多设备协作发射，否则逐 GPU 切换设备后调用 cudaLaunchKernel。
 * 参数 paramsList（struct cudaLaunchParams *paramsList）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 cudaDevs（int* cudaDevs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 numDevices（int numDevices）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 cgMode（int cgMode）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclLaunchBarrier。
 * 本体直接出现的调用/包装：CUDACHECK；cudaGetDevice；cudaLaunchCooperativeKernelMultiDevice；cudaLaunchKernel；cudaSetDevice。
 * 需要对照的边界/推进条件：cgMode & 0x01；i < numDevices。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclLaunchCooperativeKernelMultiDevice(struct cudaLaunchParams *paramsList, int* cudaDevs, int numDevices, int cgMode) {
#if CUDART_VERSION >= 9000
  if (cgMode & 0x01) {
    CUDACHECK(cudaLaunchCooperativeKernelMultiDevice(paramsList, numDevices,
            // These flags are to reduce the latency of using this API
            cudaCooperativeLaunchMultiDeviceNoPreSync|cudaCooperativeLaunchMultiDeviceNoPostSync));
    return ncclSuccess;
  }
#endif
/* 中文逐项-变量 V0896：savedDev
 * savedDev（int；函数 ncclLaunchCooperativeKernelMultiDevice 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaGetDevice(&savedDev));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int savedDev;
  CUDACHECK(cudaGetDevice(&savedDev));
  for (/* 中文逐项-变量 V0897：i
 * i（int；函数 ncclLaunchCooperativeKernelMultiDevice 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < numDevices；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < numDevices; i++) {
/* 中文逐项-变量 V0898：params
 * params（struct cudaLaunchParams；函数 ncclLaunchCooperativeKernelMultiDevice 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含
 * grid、block、stream 和参数地址。
 * 初值/来源 params = paramsList+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct cudaLaunchParams* params = paramsList+i;
    CUDACHECK(cudaSetDevice(cudaDevs[i]));
    CUDACHECK(cudaLaunchKernel(params->func, params->gridDim, params->blockDim, params->args, params->sharedMem, params->stream));
  }
  CUDACHECK(cudaSetDevice(savedDev));
  return ncclSuccess;
}

/* 中文精读：工作 FIFO 的生产者与槽位复用
 * channel->workFifoTail 是 CPU 单调递增的写入序号，取模 NCCL_MAX_OPS 才是数组下标。
 * GPU 把一条 work 复制到共享内存后，将主机槽位 header.type 清成 Unused；CPU 等待这个标记再覆盖。
 * 所以这里等待的是“描述符已被取走”，并非“这条通信的数据传输已经结束”。
 * workCount 限制本批排队数；它与持久累加的 tail、GPU 的 index 各有不同生命周期。
 */
/* 中文逐项-函数 F0392：getNextOp
 * 用途与位置：等待 channel 工作 FIFO 某槽被 GPU 取走并标成 Unused，清零后分配给本批下一条 work。
 * 参数 channel（struct ncclChannel* channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 work（struct ncclWork** work）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 * 参数 base（struct ncclWorkElem* base）：本操作在长期连接 step 上的对齐起点，本任务 posted/done 等相对计数要加 base 才是绝对进度。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::setupLaunch；src/enqueue.cc::ncclEnqueueP2pKernel；src/enqueue.cc::ncclEnqueueCollKernel。
 * 本体直接出现的调用/包装：memcpy；memset；sched_yield。
 * 需要对照的边界/推进条件：channel->workCount == NCCL_MAX_OPS；typePtr[0] != ncclWorkTypeUnused；base；work。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getNextOp(struct ncclChannel* channel, struct ncclWork** work, struct ncclWorkElem* base) {
  if (channel->workCount == NCCL_MAX_OPS) {
    WARN("Too many aggregated operations on channel %d (%d max)", channel->id, NCCL_MAX_OPS);
    return ncclInvalidUsage;
  }
/* 中文逐项-变量 V0899：opIndex
 * opIndex（int；函数 getNextOp 内）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 初值/来源 opIndex = channel->workFifoTail%NCCL_MAX_OPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
  int opIndex = channel->workFifoTail%NCCL_MAX_OPS;
/* 中文逐项-变量 V0900：w
 * w（struct ncclWork；函数 getNextOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 w = channel->workFifo+opIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclWork* w = channel->workFifo+opIndex;
/* 中文逐项-变量 V0901：typePtr
 * typePtr（uint8_t；函数 getNextOp 内）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 初值/来源 typePtr = (volatile uint8_t*)&w->header.type；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  volatile uint8_t* typePtr = (volatile uint8_t*)&w->header.type;
  while (typePtr[0] != ncclWorkTypeUnused) sched_yield();
  memset(w, 0, sizeof(struct ncclWork));
  // Initialize with work elem if provided
  if (base) memcpy(w->elems, base, sizeof(struct ncclWorkElem));
  channel->workFifoTail++;
  channel->workCount++;
  if (work) *work = w;
  return ncclSuccess;
}

// Finalize channel work FIFO states before launch
// Called during dynamic enqueue
/* 中文精读：为什么空 channel 也要放一个任务
 * gridDim.x 覆盖编号 0 到最高使用 channel；中间有空洞时也会产生对应 block。
 * 给空洞填 nWarps=0 的 P2P work，并给每条通道本批最后一项设 isLast，保证空 block 能正确退出。
 * 普通提交可以临时缩小 grid，CUDA Graph 的 grid 在捕获时就已固定，因此两条分支不能混用。
 * channel 0 单条 collective 可通过 kernel 参数内联传入，省一次工作 FIFO 读取。
 */
/* 中文逐项-函数 F0393：setupLaunch
 * 用途与位置：确定本批 block/channel 范围，给空通道补 no-op，标记各通道 isLast，并准备可选 GDRCopy FIFO。
 * 参数 eqInfo（struct ncclQueueInfo* eqInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 usingCudaGraph（int usingCudaGraph）：当前准备阶段是否在 CUDA Graph 捕获模式，队列生命周期与普通提交不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueHostSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；eqInfo->elemList->count；getNextOp；ncclGdrCudaCopy；std::max。
 * 需要对照的边界/推进条件：!usingCudaGraph；c<nChannels；comm->channels[c].workCount；c<eqInfo->maxChannels。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t setupLaunch(struct ncclQueueInfo* eqInfo, int usingCudaGraph) {
/* 中文逐项-变量 V0902：comm
 * comm（ncclComm_t；函数 setupLaunch 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = eqInfo->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclComm_t comm = eqInfo->comm;
  // Do not use comm->myParams in this function unless in non-graph mode
  // In graph mode, enqueue is async to capture, myParams can have been changed
/* 中文逐项-变量 V0903：params
 * params（struct cudaLaunchParams；函数 setupLaunch 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 * 初值/来源 params = comm->myParams；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct cudaLaunchParams* params = comm->myParams;

  // Only launch blocks where we have work to do.
  // This is not supported when we are in cudaGraph mode.
  // Because in cudaGraph mode the launch param needs to be determined
  // at capture time instead of launch time.
  if (!usingCudaGraph) {
/* 中文逐项-变量 V0904：nChannels
 * nChannels（int；函数 setupLaunch 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = std::max(comm->nChannels, comm->p2pnChannels)；保留下限或较大候选，避免并行度/尺寸低于需要的最小值。
 */
    int nChannels = std::max(comm->nChannels, comm->p2pnChannels);
    for (/* 中文逐项-变量 V0905：c
 * c（int；函数 setupLaunch 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<nChannels; c++) {
      if (comm->channels[c].workCount) params->gridDim.x = c+1;
    }
    eqInfo->maxChannels = params->gridDim.x;
  }

  // Set isLast = 1 for the last operation and add a no-op on empty channels (p2p case).
  for (/* 中文逐项-变量 V0906：c
 * c（int；函数 setupLaunch 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<eqInfo->maxChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<eqInfo->maxChannels; c++) {
/* 中文逐项-变量 V0907：channel
 * channel（struct ncclChannel；函数 setupLaunch 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+c;
    if (channel->workCount == 0) {
/* 中文逐项-变量 V0908：w
 * w（struct ncclWork；函数 setupLaunch 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getNextOp(channel, &w, NULL));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      struct ncclWork* w;
      NCCLCHECK(getNextOp(channel, &w, NULL));
      w->header.funcIndex = FUNC_INDEX_P2P;
      w->header.type = ncclWorkTypeP2p;
      w->header.nWarps = 0;
    }
    channel->workFifo[(channel->workFifoTail-1)%NCCL_MAX_OPS].header.isLast = 1;

    if (c == 0) {
      // As we inline the first coll directly, we can free it immediately.
      // Except P2P or aggregation or registration cases
/* 中文逐项-变量 V0909：work
 * work（struct ncclWork；函数 setupLaunch 内）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 * 初值/来源 work =
 * channel->workFifo+((channel->workFifoTail-channel->workCount)%NCCL_MAX_OPS)；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
      struct ncclWork* work = channel->workFifo+((channel->workFifoTail-channel->workCount)%NCCL_MAX_OPS);
      if (work->header.type == ncclWorkTypeColl && eqInfo->elemList->count() == 1)
        work->header.type = ncclWorkTypeUnused;
    }

    if (channel->gdrMemDesc) {
      // GDRCOPY support
/* 中文逐项-变量 V0910：first
 * first（uint64_t；函数 setupLaunch 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 first = (channel->workFifoTail-channel->workCount)%NCCL_MAX_OPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
      uint64_t first = (channel->workFifoTail-channel->workCount)%NCCL_MAX_OPS;
/* 中文逐项-变量 V0911：nelems
 * nelems（uint64_t；函数 setupLaunch 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nelems = channel->workCount；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      uint64_t nelems = channel->workCount;
      TRACE(NCCL_INIT, "GDRCOPY : copy workFifo %p to %p first %ld nelems %zi",
            channel->workFifo, channel->workFifoGdr, first, nelems);

      for (/* 中文逐项-变量 V0912：i
 * i（int；函数 setupLaunch 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < nelems；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < nelems; i++) {
/* 中文逐项-变量 V0913：elem
 * elem（int；函数 setupLaunch 内）：注册work扩展或队列容器内嵌的普通工作元素，扩展信息在它后面保存。
 * 初值/来源 elem = (first+i) % NCCL_MAX_OPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int elem = (first+i) % NCCL_MAX_OPS;
        // Copy Host workFifo to CUDA workFifo via the GDRCOPY mapping
        NCCLCHECK(ncclGdrCudaCopy(channel->gdrMemDesc, channel->workFifoGdr+elem, channel->workFifo+elem, 1));
      }
    }
  }

  return ncclSuccess;
}

/* 中文逐项-函数 F0394：ncclCpuBarrierIn
 * 用途与位置：进入同进程多 GPU 发射的相位屏障，用原子计数选出最后到达者统一发射。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 isLast（int* isLast）：本通道当前批次的最后一条任务标记，GPU 执行完此记录即可退出本次 kernel。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclLaunchBarrier。
 * 本体直接出现的调用/包装：__sync_bool_compare_and_swap。
 * 需要对照的边界/推进条件：done == false；val >= comm->intraRanks；val+1 == comm->intraRanks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCpuBarrierIn(struct ncclComm* comm, int* isLast) {
/* 中文逐项-变量 V0914：ptr
 * ptr（int；函数 ncclCpuBarrierIn 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  volatile int* ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase);
/* 中文逐项-变量 V0915：val
 * val（int；函数 ncclCpuBarrierIn 内）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 初值/来源 val = *ptr；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int val = *ptr;
/* 中文逐项-变量 V0916：done
 * done（bool；函数 ncclCpuBarrierIn 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 初值/来源 done = false；布尔初态为false，后续条件决定是否切换。
 */
  bool done = false;
  while (done == false) {
    if (val >= comm->intraRanks) {
      WARN("Trying to launch too many work elements, max is %d", NCCL_MAX_OPS);
      return ncclInvalidUsage;
    }
    if (val+1 == comm->intraRanks) {
      // Reset the barrier.
      comm->intraBarrier[comm->intraPhase^1] = 0;
      *isLast = 1;
      return ncclSuccess;
    }
    done = __sync_bool_compare_and_swap(ptr, val, val+1);
    val++;
  }
  *isLast = 0;
  return ncclSuccess;
}

/* 中文逐项-函数 F0395：ncclCpuBarrierLast
 * 用途与位置：最后到达者发布本相位已完成信号，使其他发射线程可离开屏障。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclLaunchBarrier。
 * 本体直接出现的调用/包装：__sync_bool_compare_and_swap。
 * 需要对照的边界/推进条件：__sync_bool_compare_and_swap(ptr, val, val+1) != true。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCpuBarrierLast(struct ncclComm* comm) {
/* 中文逐项-变量 V0917：ptr
 * ptr（int；函数 ncclCpuBarrierLast 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  volatile int* ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase);
/* 中文逐项-变量 V0918：val
 * val（int；函数 ncclCpuBarrierLast 内）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 初值/来源 val = *ptr；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int val = *ptr;
  if (__sync_bool_compare_and_swap(ptr, val, val+1) != true) {
    WARN("Trying to launch too many work elements, max is %d", NCCL_MAX_OPS);
    return ncclInternalError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0396：ncclCpuBarrierOut
 * 用途与位置：等待本地发射相位完成并翻转相位，保证下一轮发射不会误用上轮计数。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclLaunchKernel。
 * 本体直接出现的调用/包装：pthread_yield。
 * 需要对照的边界/推进条件：*ptr < comm->intraRanks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCpuBarrierOut(struct ncclComm* comm) {
/* 中文逐项-变量 V0919：ptr
 * ptr（int；函数 ncclCpuBarrierOut 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  volatile int* ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase);
  while (*ptr < comm->intraRanks) pthread_yield();
  comm->intraPhase ^= 1;
  return ncclSuccess;
}

// Check dependency wrt outside streams or previous launches
// Launch kernel in GROUP mode
/* 中文精读：stream 依赖与本地 GPU 发射屏障
 * 用户 stream 上先前的计算必须完成，通信 kernel 才能读取它的输入。
 * 若切到 groupStream，就先在 userStream record event，再在 groupStream wait event 建立依赖。
 * GROUP 模式还用 intraBarrier 协调同进程的多个 GPU；最后到达的线程可统一启动它们。
 * 这个 CPU 屏障不是跨机器的集体通信屏障，也不等于等待本次 AllReduce 完成。
 */
/* 中文逐项-函数 F0397：ncclLaunchBarrier
 * 用途与位置：建立用户 stream 与内部 stream 依赖，并在 GROUP 发射模式协调同进程 GPU 的启动。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaEventRecord；cudaStreamWaitEvent；ncclCpuBarrierIn；ncclCpuBarrierLast；ncclLaunchCooperativeKernelMultiDevice。
 * 需要对照的边界/推进条件：params->gridDim.x == 0；comm->launchMode == ncclComm::GROUP && (comm->groupCudaStream ||
 * comm->userStream == cudaStreamDefault || comm->userStream == cudaStreamLegacy || …（完整表达式见紧邻源码）；comm->userStream !=
 * params->stream && !comm->usingCudaGraph；comm->launchMode == ncclComm::GROUP。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclLaunchBarrier(struct ncclComm* comm) {
/* 中文逐项-变量 V0920：params
 * params（struct cudaLaunchParams；函数 ncclLaunchBarrier 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 * 初值/来源 params = comm->myParams；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct cudaLaunchParams* params = comm->myParams;
  if (params->gridDim.x == 0) return ncclSuccess;

  // Use internal NCCL stream for CGMD/GROUP launch if required or if the user stream is NULL
  if (comm->launchMode == ncclComm::GROUP &&
      (comm->groupCudaStream ||
       comm->userStream == cudaStreamDefault ||
       comm->userStream == cudaStreamLegacy ||
       comm->userStream == cudaStreamPerThread)) {
    // Enqueue event in user stream
    CUDACHECK(cudaEventRecord(comm->intDoneEvent, comm->userStream));
    // Create dependency between user stream and internal NCCL stream
    CUDACHECK(cudaStreamWaitEvent(comm->groupStream, comm->intDoneEvent, 0));
    params->stream = comm->groupStream;
  } else {
    if (comm->userStream != params->stream && !comm->usingCudaGraph) {
      // Stream changed from last call, create dependency against last NCCL kernel launch
      CUDACHECK(cudaStreamWaitEvent(comm->userStream, comm->doneEvent, 0));
    }
    params->stream = comm->userStream;
  }

  if (comm->launchMode == ncclComm::GROUP) {
/* 中文逐项-变量 V0921：isLast
 * isLast（int；函数 ncclLaunchBarrier 内）：本通道当前批次的最后一条任务标记，GPU 执行完此记录即可退出本次 kernel。
 * 初值/来源 isLast = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int isLast = 0;
    NCCLCHECK(ncclCpuBarrierIn(comm, &isLast));
    if (isLast) {
      // I'm the last. Launch all operations.
      NCCLCHECK(ncclLaunchCooperativeKernelMultiDevice(comm->intraParams, comm->intraCudaDevs, comm->intraRanks, *comm->intraCGMode));
      NCCLCHECK(ncclCpuBarrierLast(comm));
    }
  }
  return ncclSuccess;
}

// Launch kernel in PARALLEL mode
/* 中文导读：
 * 执行 kernel 启动与相关的本地发射协调。常规路径的通信执行与用户 CUDA stream 有序。
 * 主机 API 返回表示工作提交进度，不等于远端归约或 GPU/NIC 传输已经全部结束。
 */
/* 中文逐项-函数 F0398：ncclLaunchKernel
 * 用途与位置：PARALLEL 模式真正调用 CUDA 发射，GROUP 模式等待已由最后线程执行的统一发射完成。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaLaunchKernel；ncclCpuBarrierOut。
 * 需要对照的边界/推进条件：params->gridDim.x == 0；comm->rank == 0 && *comm->intraCGMode & 0x10；comm->launchMode ==
 * ncclComm::GROUP。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclLaunchKernel(ncclComm_t comm) {
/* 中文逐项-变量 V0922：params
 * params（struct cudaLaunchParams；函数 ncclLaunchKernel 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 * 初值/来源 params = comm->myParams；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct cudaLaunchParams *params = comm->myParams;
  if (params->gridDim.x == 0) return ncclSuccess;

  // We can't print the CG mode before the first barrier happened.
  if (comm->rank == 0 && *comm->intraCGMode & 0x10) {
    *comm->intraCGMode ^= 0x10;
    INFO(NCCL_INIT,"Launch mode %s%s%s",
        comm->launchMode == ncclComm::GROUP ? "Group" : "Parallel",
        *comm->intraCGMode ? "/CGMD" : "",
        (comm->launchMode == ncclComm::GROUP && comm->groupCudaStream) ? "/Stream" : "");
  }

  if (comm->launchMode == ncclComm::GROUP) {
    NCCLCHECK(ncclCpuBarrierOut(comm));
  } else {
    CUDACHECK(cudaLaunchKernel(params->func, params->gridDim, params->blockDim, params->args, params->sharedMem, params->stream));
  }

  return ncclSuccess;
}

// Launch network proxy
/* 中文精读：实际调用顺序，以代码调用点为准
 * 下面保留的英文注释提到 kernel launched，但普通路径实际是：
 * ncclEnqueueCheck / ncclGroupEnd -> ncclEnqueueHostSetup<0> -> 本函数 -> ncclProxyStart，
 * 然后才进入 ncclLaunchBarrier / ncclLaunchKernel。CUDA Graph 则用图依赖安排 HostSetup。
 * Start 发布任务并唤醒已有线程，不是每次创建线程，也不是 GPU 给 CPU 发中断。
 * Proxy 可以先挂接收或准备槽位，发送仍须等 GPU 的 sizes/tail/flag 说明数据已就绪。
 */
/* 中文逐项-函数 F0399：ncclLaunchProxy
 * 用途与位置：发布 HostSetup 已保存的 Proxy 任务并重置本批通道统计；普通调用点在实际 kernel 发射之前。
 * 参数 eqInfo（struct ncclQueueInfo* eqInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueHostSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclProxyStart。
 * 需要对照的边界/推进条件：eqInfo->maxChannels == 0；r<eqInfo->maxChannels。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclLaunchProxy(struct ncclQueueInfo* eqInfo) {
  // Start the network proxies as soon as the kernel has been launched. We can't
  // perform any CUDA call between the two or having a cudaFree between the CUDA
  // launch and the ncclProxyStart call could cause a deadlock.
  // Also, starting the proxies after the CUDA launch seems to be better for
  // performance (latency).
/* 中文逐项-变量 V0923：comm
 * comm（ncclComm_t；函数 ncclLaunchProxy 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = eqInfo->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclComm_t comm = eqInfo->comm;
  if (eqInfo->maxChannels == 0) return ncclSuccess;

  for (/* 中文逐项-变量 V0924：r
 * r（int；函数 ncclLaunchProxy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<eqInfo->maxChannels；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<eqInfo->maxChannels; r++) {
/* 中文逐项-变量 V0925：channel
 * channel（struct ncclChannel；函数 ncclLaunchProxy 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+r；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+r;
    channel->workCount = 0;
    channel->totalSize = 0;
  }
  comm->lastChannel = 0;
  NCCLCHECK(ncclProxyStart(comm));
  return ncclSuccess;
}

// Record done event for current launch
/* 中文逐项-函数 F0400：ncclRecordEvents
 * 用途与位置：在执行 stream 记录完成事件并建立必要的用户 stream 依赖；record 本身不代表事件已经完成。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：CUDACHECK；cudaEventRecord；cudaStreamWaitEvent。
 * 需要对照的边界/推进条件：!comm->usingCudaGraph；comm->launchMode == ncclComm::GROUP && (comm->groupCudaStream ||
 * comm->userStream == cudaStreamDefault || comm->userStream == cudaStreamLegacy || …（完整表达式见紧邻源码）。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclRecordEvents(ncclComm_t comm) {
/* 中文逐项-变量 V0926：params
 * params（struct cudaLaunchParams；函数 ncclRecordEvents 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 * 初值/来源 params = comm->myParams；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct cudaLaunchParams *params = comm->myParams;

  // Enqueue event after NCCL kernel (only in non-graph mode)
  if (!comm->usingCudaGraph) CUDACHECK(cudaEventRecord(comm->doneEvent, params->stream));
  // Use internal NCCL stream for CGMD/GROUP launch if required or if the user stream is NULL
  if (comm->launchMode == ncclComm::GROUP &&
      (comm->groupCudaStream ||
       comm->userStream == cudaStreamDefault ||
       comm->userStream == cudaStreamLegacy ||
       comm->userStream == cudaStreamPerThread)) {
    CUDACHECK(cudaEventRecord(comm->intDoneEvent, params->stream));
    // Create dependency between NCCL internal stream and user stream
    CUDACHECK(cudaStreamWaitEvent(comm->userStream, comm->intDoneEvent, 0));
  }
  return ncclSuccess;
}

// Reset parameter space for launch
/* 中文逐项-函数 F0401：ncclLaunchReset
 * 用途与位置：清理本批发射参数和入队计数，为后续提交复用；图路径按持久 QueueInfo 生命周期处理。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCreateQueueInfo；ncclResetQueueInfo。
 * 需要对照的边界/推进条件：comm->usingCudaGraph；comm->launchMode == ncclComm::GROUP_GRAPH。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclLaunchReset(ncclComm_t comm) {
  comm->userStreamSet = false;

  // We are finishing capture of the current launch
  // But we need to keep the current enqueue info for CUDA graph
  // Thus we need to creating a new enqueue info for the next run
  if (comm->usingCudaGraph) {
    NCCLCHECK(ncclCreateQueueInfo(&comm->enqueueInfo, comm));
  } else {
    // If not in CUDA graph mode, we reuse the same info space
    NCCLCHECK(ncclResetQueueInfo(comm->enqueueInfo));
  }

  // After capturing an op in graph mode or launching the op in non-graph mode
  // we can reset myParams for use in next op
/* 中文逐项-变量 V0927：params
 * params（struct cudaLaunchParams；函数 ncclLaunchReset 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 * 初值/来源 params = comm->myParams；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct cudaLaunchParams *params = comm->myParams;
  params->gridDim.x = params->blockDim.x = 0;
  params->func = NULL;

  // Reset launch mode to GROUP if changed
  if (comm->launchMode == ncclComm::GROUP_GRAPH) comm->launchMode = ncclComm::GROUP;
  comm->usingCudaGraph = 0;

  return ncclSuccess;
}

/*****************************************************************************/
/* Enqueueing system : computation of kernel and proxy operations parameters */
/*****************************************************************************/

/* 中文逐项-函数 F0402：getCollNetSupport
 * 用途与位置：检查所选数据类型/归约是否被 CollNet 插件支持，以屏蔽不能正确执行的候选。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 参数 collNetTypeSupport（int* collNetTypeSupport）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::computeColl；src/enqueue.cc::ncclSetupAsyncKernels。
 * 本体直接出现的调用/包装：NCCLCHECK；collNetReduceSupport。
 * 需要对照的边界/推进条件：info->comm->collNetSupport > 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static inline ncclResult_t getCollNetSupport(struct ncclInfo* info, int* collNetTypeSupport) {
  if (info->comm->collNetSupport > 0) {
    // Translate ncclAvg and PreMulSum
/* 中文逐项-变量 V0928：netOp
 * netOp（ncclRedOp_t；函数 getCollNetSupport 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 netOp = info->op == ncclAvg || info->op >= ncclNumOps ? ncclSum : info->op；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    ncclRedOp_t netOp = info->op == ncclAvg || info->op >= ncclNumOps ? ncclSum : info->op;
    NCCLCHECK(collNetReduceSupport(info->datatype, netOp, collNetTypeSupport));
  } else {
    *collNetTypeSupport = 0;
  }
  return ncclSuccess;
}

// numPipeOps: number of pipelined ops. Can be greater than 1 in aggregation mode. Used to adjust latency.
/* 中文导读：
 * 枚举可用算法/协议，取 ncclTopoGetAlgoTime 的最小非负预测时间。
 * 再按消息大小缩减本次操作的 channel/线程数量：comm 的配置通道数与此次实际使用数是不同概念。
 */
/* 中文逐项-函数 F0403：getAlgoInfo
 * 用途与位置：遍历可用算法协议的时间模型，再按消息尺寸缩减本次 channel 和线程数，平衡延迟与带宽。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 参数 collNetTypeSupport（int collNetTypeSupport）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 numPipeOps（int numPipeOps）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::computeColl；src/enqueue.cc::ncclSetupAsyncKernels。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoGetAlgoTime。
 * 需要对照的边界/推进条件：comm->nRanks == 1；a<nAlgos；a == NCCL_ALGO_COLLNET && collNetTypeSupport !=
 * 1；p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getAlgoInfo(struct ncclInfo* info, int collNetTypeSupport, int numPipeOps) {
/* 中文逐项-变量 V0929：comm
 * comm（struct ncclComm；函数 getAlgoInfo 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = info->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclComm* comm = info->comm;
  if (comm->nRanks == 1) {
    info->algorithm = NCCL_ALGO_RING;
    info->protocol = NCCL_PROTO_SIMPLE;
  }
  else {
/* 中文逐项-变量 V0930：minTime
 * minTime（float；函数 getAlgoInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minTime = 3600000000.0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    float minTime = 3600000000.0; // Hopefully no operation will take an hour to complete.
    // Find algorithm / protocol.
    info->algorithm = -1;
    info->protocol = -1;
/* 中文逐项-变量 V0931：nAlgos
 * nAlgos（int；函数 getAlgoInfo 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nAlgos = NCCL_NUM_ALGORITHMS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int nAlgos = NCCL_NUM_ALGORITHMS;
    for (/* 中文逐项-变量 V0932：a
 * a（int；函数 getAlgoInfo 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<nAlgos；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<nAlgos; a++) {
      if (a == NCCL_ALGO_COLLNET && collNetTypeSupport != 1) continue;
      for (/* 中文逐项-变量 V0933：p
 * p（int；函数 getAlgoInfo 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
/* 中文逐项-变量 V0934：time
 * time（float；函数 getAlgoInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetAlgoTime(info, a, p, numPipeOps, &time));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        float time;
        NCCLCHECK(ncclTopoGetAlgoTime(info, a, p, numPipeOps, &time));
        if (time >= 0 && time < minTime) {
          info->algorithm = a;
          info->protocol = p;
          minTime = time;
        }
      }
    }
    if (info->algorithm == -1 || info->protocol == -1) {
      WARN("Error : no algorithm/protocol available");
      return ncclInternalError;
    }
    //if (comm->rank == 0) INFO(NCCL_TUNING, "%ld Bytes -> Algo %d proto %d time %f", info->nBytes, info->algorithm, info->protocol, minTime);
    TRACE(NCCL_COLL, "%ld Bytes -> Algo %d proto %d time %f", info->nBytes, info->algorithm, info->protocol, minTime);
  }

/* 中文逐项-变量 V0935：nc
 * nc（int；函数 getAlgoInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nc = (info->nChannels > 0) ? info->nChannels : comm->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nc = (info->nChannels > 0) ? info->nChannels : comm->nChannels;
/* 中文逐项-变量 V0936：nt
 * nt（int；函数 getAlgoInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nt = comm->maxThreads[info->algorithm][info->protocol]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nt = comm->maxThreads[info->algorithm][info->protocol];
/* 中文逐项-变量 V0937：threadThreshold
 * threadThreshold（int；函数 getAlgoInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 threadThreshold = comm->threadThresholds[info->algorithm][info->protocol]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int threadThreshold = comm->threadThresholds[info->algorithm][info->protocol];
  if (info->algorithm == NCCL_ALGO_COLLNET) {
    // CollNet channel tuning
/* 中文逐项-变量 V0938：ncSwitch
 * ncSwitch（int；函数 getAlgoInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncSwitch = 16；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int ncSwitch = 16;
/* 中文逐项-变量 V0939：flag
 * flag（bool；函数 getAlgoInfo 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = true；布尔初态为true，后续条件决定是否切换。
 */
    bool flag = true;
    while (ncSwitch >= 1 && flag) {
      while ((flag = info->nBytes < nc*nt*info->comm->channels[0].collTree.nHeads*threadThreshold) && nc > ncSwitch) {
        if (nc == ncSwitch+ncSwitch/2) threadThreshold /= 2;
        nc--;
      }
      ncSwitch /= 2;
    }
  } else {
    // Ring/Tree channel tuning
    while (info->nBytes < nc*nt*threadThreshold) {
      if (nc >= 2) nc--;
      else if ((nt % 128) == 0) nt/=2;
      else break;
    }
  }
  if (info->protocol == NCCL_PROTO_SIMPLE) {
    nt += WARP_SIZE; // Extra warp for sync
    // More threads or sync warps needed due to split thread model
    if (info->algorithm == NCCL_ALGO_TREE) nt += 3*WARP_SIZE;
    if (info->algorithm == NCCL_ALGO_COLLNET) nt += 3*WARP_SIZE;
  }
  info->nChannels = nc;
  info->nThreads = nt;
  return ncclSuccess;
}

/* 中文精读：API 语义如何变为通信邻居和方向
 * algorithm 决定 Ring/Tree/CollNet，pattern 再决定沿哪些方向推进。
 * Broadcast 是从 root 出发的流水线，Reduce 是到 root 结束的流水线；不必把数据绕满一圈。
 * AllGather/ReduceScatter 走一遍环；AllReduce 的 RingTwice 对应归约分散与结果收集。
 * pattern 同时交给 ProxySaveColl，确保 CPU 网络工作与 GPU 原语使用相同的邻居。
 */
/* 中文逐项-函数 F0404：getPatternInfo
 * 用途与位置：由 API 和算法映射实际通信方向模式，使 GPU 与 Proxy 选择一致的拓扑边。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::computeColl。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getPatternInfo(struct ncclInfo* info) {
  switch (info->coll) {
    case ncclFuncBroadcast:
      info->pattern = info->algorithm == NCCL_ALGO_TREE ? ncclPatternTreeDown : ncclPatternPipelineFrom; break;
    case ncclFuncReduce:
      info->pattern = info->algorithm == NCCL_ALGO_TREE ? ncclPatternTreeUp : ncclPatternPipelineTo; break;
    case ncclFuncReduceScatter:
    case ncclFuncAllGather:
      info->pattern = ncclPatternRing; break;
    case ncclFuncAllReduce:
      info->pattern = info->algorithm == NCCL_ALGO_COLLNET ? ncclPatternCollTreeUpDown : info->algorithm == NCCL_ALGO_TREE ? ncclPatternTreeUpDown : ncclPatternRingTwice; break;
    default:
      WARN("Unknown pattern for collective %d algorithm %d", info->coll, info->algorithm);
      return ncclInternalError;
  }
  return ncclSuccess;
}

/* 中文精读：算法轮次与 FIFO step 不能混为一谈
 * Ring 的一轮包含 nRanks 个逻辑数据块，跨每条环边传 nRanks-1 次；RingTwice 是两倍。
 * Tree/Pipeline 的一轮按一个块计，具体树中间节点的收发由各自连接推进。
 * nstepsPerLoop 是算法步数；还要乘 chunkSteps，才能得到 Proxy 所用的 FIFO 进度量。
 * 因此 nsteps 不是 RDMA 包数、WQE 数或 QP 数，NIC 还可能继续拆包和分 QP。
 */
/* 中文逐项-函数 F0405：getLoopInfo
 * 用途与位置：给通信模式设置每轮块数和算法步数，再由 computeColl 转成实际协议进度。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::computeColl。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getLoopInfo(struct ncclInfo* info) {
  switch (info->pattern) {
    case ncclPatternTreeUp:
    case ncclPatternTreeDown:
    case ncclPatternTreeUpDown:
    case ncclPatternPipelineFrom:
    case ncclPatternPipelineTo:
      info->nstepsPerLoop = info-> nchunksPerLoop = 1; break;
    case ncclPatternCollTreeUpDown:
      info->nstepsPerLoop = 1; info->nchunksPerLoop = info->comm->channels[0].collTree.nHeads; break;
    case ncclPatternRing:
      info->nstepsPerLoop = info->comm->nRanks-1; info->nchunksPerLoop = info->comm->nRanks; break;
    case ncclPatternRingTwice:
      info->nstepsPerLoop = 2*(info->comm->nRanks-1); info->nchunksPerLoop = info->comm->nRanks; break;
    default:
      WARN("Unknown pattern %d", info->pattern);
      return ncclInternalError;
  }
  return ncclSuccess;
}

/* 中文导读：
 * 把 ncclInfo 同时编译成 GPU work 与 ProxyOp，确保两侧对协议、分片和步数的理解一致。
 * Ring AllReduce 的每轮通信步数为 2*(nRanks-1)，缓冲区每步大小为 buffSizes[protocol]/NCCL_STEPS。
 * chunk 是算法分块，slice 是原语推进单位，step 是循环缓冲区的进度单位，三者不能随意互换。
 */
/* 中文逐项-函数 F0406：computeColl
 * 用途与位置：把一项 API 任务编译成 GPU work 与 ProxyOp，计算协议 chunk/slice、尾块和总推进步数。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 参数 work（struct ncclWorkElem* work）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 * 参数 proxyOp（struct ncclProxyOp* proxyOp）：交给 CPU Proxy 的紧凑计划，与 GPU work 的协议和步数必须一致。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclSetupCollKernel。
 * 本体直接出现的调用/包装：ALIGN_SIZE；DIVUP；FUNC_INDEX；NCCLCHECK；getAlgoInfo；getCollNetSupport；getLoopInfo；getPatternInfo；int；log2i；ncclTypeSize。
 * 需要对照的边界/推进条件：info->nChannels > 0 && info->nThreads > 0；info->comm->nRanks == 1；info->algorithm == NCCL_ALGO_TREE &&
 * info->protocol == NCCL_PROTO_SIMPLE；info->pattern == ncclPatternTreeUpDown。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t computeColl(struct ncclInfo* info /* input */, struct ncclWorkElem* work, struct ncclProxyOp* proxyOp /* output */) {
/* 中文逐项-变量 V0940：collNetTypeSupport
 * collNetTypeSupport（int；函数 computeColl 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNetTypeSupport = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int collNetTypeSupport = 0;
  // Check whether algo and proto have been preset (as in aggregation case)
  // If so, skip the calculation
  if (info->nChannels > 0 && info->nThreads > 0) goto comp_next;
  NCCLCHECK(getCollNetSupport(info, &collNetTypeSupport));
  NCCLCHECK(getAlgoInfo(info, collNetTypeSupport, 1));

comp_next:
  // Set nstepsPerLoop and nchunksPerLoop
  NCCLCHECK(getPatternInfo(info));
  NCCLCHECK(getLoopInfo(info));

  work->header.type = ncclWorkTypeColl;
  work->sendbuff = info->sendbuff;
  work->recvbuff = info->recvbuff;
  work->root = info->root;
  work->count = info->count;
  work->nChannels = info->nChannels;
  work->header.nWarps = info->nThreads / WARP_SIZE;
  work->redOpArg = info->opFull.scalarArg;
  work->redOpArgIsPtr = info->opFull.scalarArgIsPtr;

  if (info->comm->nRanks == 1) {
    // one-rank reduce index
    work->header.funcIndex = 1 + int(info->datatype);
    return ncclSuccess;
  }

  work->header.funcIndex = FUNC_INDEX(info->coll, info->opFull.op, info->datatype, info->algorithm, info->protocol);

/* 中文精读：分片尺寸的第一层：协议环形缓冲区
 * 这里 stepSize/chunkSize 都是字节。NCCL_STEPS=8，把协议缓冲分成 8 个可循环使用的 step。
 * 只有 Simple+Ring 使用 API 给定的 chunkSteps/sliceSteps；其余组合在这里按 1 step 处理。
 * chunkSize=stepSize*chunkSteps；一个算法块可含多个 slice，每个 slice 占 sliceSteps 个 step。
 * GPU 侧会再除 sizeof(T) 转换成元素数。不能把主机的字节尺寸直接代进设备下标。
 * 采用有限槽位和分片，是为了让 GPU 计算、网络传输、下一块处理形成流水，同时限制临时内存。
 */
/* 中文逐项-变量 V0941：stepSize
 * stepSize（int；函数 computeColl 内）：一 FIFO step 的容量；设备 T 指针路径以元素计，主机缓冲路径以字节计。
 * 初值/来源 stepSize = info->comm->buffSizes[info->protocol]/NCCL_STEPS；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int stepSize   = info->comm->buffSizes[info->protocol]/NCCL_STEPS;
/* 中文逐项-变量 V0942：chunkSteps
 * chunkSteps（int；函数 computeColl 内）：一个算法块/原语片段占用的 FIFO step 数，用来对齐 CPU Proxy 与 GPU 进度。
 * 初值/来源 chunkSteps = (info->protocol == NCCL_PROTO_SIMPLE && info->algorithm == NCCL_ALGO_RING) ? info->chunkSteps :
 * 1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int chunkSteps = (info->protocol == NCCL_PROTO_SIMPLE && info->algorithm == NCCL_ALGO_RING) ? info->chunkSteps : 1;
/* 中文逐项-变量 V0943：sliceSteps
 * sliceSteps（int；函数 computeColl 内）：一个算法块/原语片段占用的 FIFO step 数，用来对齐 CPU Proxy 与 GPU 进度。
 * 初值/来源 sliceSteps = (info->protocol == NCCL_PROTO_SIMPLE && info->algorithm == NCCL_ALGO_RING) ? info->sliceSteps :
 * 1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int sliceSteps = (info->protocol == NCCL_PROTO_SIMPLE && info->algorithm == NCCL_ALGO_RING) ? info->sliceSteps : 1;
/* 中文逐项-变量 V0944：chunkSize
 * chunkSize（int；函数 computeColl 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = stepSize*chunkSteps；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int chunkSize  = stepSize*chunkSteps;

  // Compute lastChunkSize
  if (info->algorithm == NCCL_ALGO_TREE && info->protocol == NCCL_PROTO_SIMPLE) {
    if (info->pattern == ncclPatternTreeUpDown) {
      // Optimize chunkSize / nSteps
      while (info->nBytes / (info->nChannels*chunkSize) < info->comm->channels[0].tree.depth*8 && chunkSize > 131072) chunkSize /= 2;
      while (info->nBytes / (info->nChannels*chunkSize) < info->comm->channels[0].tree.depth*4 && chunkSize > 65536) chunkSize /= 2;
      while (info->nBytes / (info->nChannels*chunkSize) < info->comm->channels[0].tree.depth && chunkSize > 32768) chunkSize /= 2;
    }
    // Use lastChunkSize as chunkSize
    work->lastChunkSize = chunkSize / ncclTypeSize(info->datatype);
  } else if (info->algorithm == NCCL_ALGO_COLLNET && info->protocol == NCCL_PROTO_SIMPLE) {
    // Optimize chunkSize / nSteps
    while (info->nBytes / (info->nChannels*info->comm->channels[0].collTree.nHeads*chunkSize) < info->comm->channels[0].collTree.depth*64 && chunkSize > 131072) chunkSize /= 2;
    while (info->nBytes / (info->nChannels*info->comm->channels[0].collTree.nHeads*chunkSize) < info->comm->channels[0].collTree.depth*8 && chunkSize > 65536) chunkSize /= 2;
    while (info->nBytes / (info->nChannels*info->comm->channels[0].collTree.nHeads*chunkSize) < info->comm->channels[0].collTree.depth*8 && chunkSize > 32768) chunkSize /= 2;
    // Use lastChunkSize as chunkSize
    work->lastChunkSize = chunkSize / ncclTypeSize(info->datatype);
    // Set direct direction for broadcast-gather (read or write)
    work->direct = (info->nBytes / info->nChannels <= 1024*1024) ? NCCL_DIRECT_WRITE : NCCL_DIRECT_READ;
  } else if (info->protocol == NCCL_PROTO_LL) {
/* 中文逐项-变量 V0945：sliceSize
 * sliceSize（ssize_t；函数 computeColl 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 sliceSize = stepSize*sizeof(uint64_t)/sizeof(union ncclLLFifoLine)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t sliceSize = stepSize*sizeof(uint64_t)/sizeof(union ncclLLFifoLine);
/* 中文逐项-变量 V0946：loopSize
 * loopSize（ssize_t；函数 computeColl 内）：一轮全部 channel（以及 Ring ranks/CollNet heads）覆盖的逻辑数据范围。
 * 初值/来源 loopSize = info->nChannels*info->nchunksPerLoop*(ssize_t)sliceSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t loopSize = info->nChannels*info->nchunksPerLoop*(ssize_t)sliceSize;
    work->lastChunkSize = DIVUP((info->nBytes-(info->nBytes/loopSize)*loopSize), info->nChannels*info->nchunksPerLoop);
    ALIGN_SIZE(work->lastChunkSize, info->nThreads*sizeof(uint64_t));
    work->lastChunkSize /= ncclTypeSize(info->datatype);
  } else if (info->algorithm == NCCL_ALGO_TREE && info->protocol == NCCL_PROTO_LL128) {
/* 中文逐项-变量 V0947：nNodes
 * nNodes（int；函数 computeColl 内）：当前 communicator 跨越的节点数量；节点编号不等于 GPU rank。
 * 初值/来源 nNodes = info->comm->nNodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int nNodes = info->comm->nNodes;
/* 中文逐项-变量 V0948：ppn
 * ppn（float；函数 computeColl 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ppn = info->comm->nRanks / (float)nNodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    float ppn = info->comm->nRanks / (float)nNodes;
/* 中文逐项-变量 V0949：nstepsLL128
 * nstepsLL128（float；函数 computeColl 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nstepsLL128 = 1+log2i(nNodes) + 0.1*ppn；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    float nstepsLL128 = 1+log2i(nNodes) + 0.1*ppn;
    while (info->nBytes / (info->nChannels*chunkSize) < nstepsLL128*64/ppn && chunkSize > 131072) chunkSize /= 2;
    while (info->nBytes / (info->nChannels*chunkSize) < nstepsLL128*16/ppn && chunkSize > 32768) chunkSize /= 2;
    // Use lastChunkSize as chunkSize
    work->lastChunkSize = chunkSize*NCCL_LL128_DATAELEMS/(NCCL_LL128_LINEELEMS*ncclTypeSize(info->datatype));
  }

/* 中文精读：让 GPU 与 Proxy 对同一分片达成一致
 * LL 的 16 字节 FIFO 行只有 8 字节有效数据；LL128 每 128 字节中有 120 字节有效数据。
 * 所以用于覆盖用户数据的 chunkEffectiveSize 要扣掉 flag 开销，而网络缓冲仍按物理字节分配。
 * nLoops=ceil(nBytes/(nChannels*nchunksPerLoop*chunkEffectiveSize))，
 * nsteps=nstepsPerLoop*nLoops*chunkSteps。尾块不足也要保持两侧推进步数匹配。
 * 这个公式是本版本调度器给 Proxy 的预算，不表示每个 step 必然对应一个独立 verbs 请求。
 */
  // Compute nSteps for proxies
/* 中文逐项-变量 V0950：chunkEffectiveSize
 * chunkEffectiveSize（int；函数 computeColl 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 chunkEffectiveSize = chunkSize；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int chunkEffectiveSize = chunkSize;
  if (info->protocol == NCCL_PROTO_LL) chunkEffectiveSize /= 2;
  if (info->protocol == NCCL_PROTO_LL128) chunkEffectiveSize = (chunkSize / NCCL_LL128_LINEELEMS) * NCCL_LL128_DATAELEMS;
  //if (info->comm->rank == 0) printf("Coll %d, size %ld -> %dx%d, chunkSize %d (algo %d proto%d)\n", info->coll, info->nBytes, info->nChannels, info->nThreads, chunkSize, info->algorithm, info->protocol);
/* 中文逐项-变量 V0951：nLoops
 * nLoops（int；函数 computeColl 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nLoops = (int)(DIVUP(info->nBytes,
 * (((size_t)(info->nChannels))*info->nchunksPerLoop*chunkEffectiveSize)))；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nLoops = (int)(DIVUP(info->nBytes, (((size_t)(info->nChannels))*info->nchunksPerLoop*chunkEffectiveSize)));
  proxyOp->nsteps = info->nstepsPerLoop * nLoops * chunkSteps;
  proxyOp->sliceSteps = sliceSteps;
  proxyOp->chunkSteps = chunkSteps;
  proxyOp->chunkSize = chunkSize;
  proxyOp->protocol = info->protocol;
  proxyOp->dtype = info->datatype;
  proxyOp->redOp = info->algorithm != NCCL_ALGO_COLLNET ? ncclNumOps : // Only set redOp when using CollNet
                     info->opFull.op==ncclDevPreMulSum || info->opFull.op==ncclDevSumPostDiv ? ncclSum : // Network sees avg as sum
                     info->op;
  proxyOp->pattern = info->pattern;
  proxyOp->root = info->root;
  // This is used by P2P to reduce the receive buffer size. We don't use it in collectives
  // because some protocols need to transmit more than the total size, plus they sometimes
  // round up
  proxyOp->nbytes = stepSize*proxyOp->sliceSteps;

  TRACE(NCCL_COLL,"opCount %lx slicesteps %d spl %d cpl %d nbytes %zi -> protocol %d nchannels %d nthreads %d, nloops %d nsteps %d chunksize %d comm %p",
      proxyOp->opCount, sliceSteps, info->nstepsPerLoop, info->nchunksPerLoop, info->nBytes, info->protocol, info->nChannels, info->nThreads,
      nLoops, proxyOp->nsteps, chunkSize, info->comm);
  return ncclSuccess;
}

/* 中文逐项-函数 F0407：checkSetStream
 * 用途与位置：保存本批使用的 CUDA stream，并检查 Group 内同一 communicator 的 stream 使用契约。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：info->comm->userStreamSet == false；info->stream != info->comm->userStream。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t checkSetStream(struct ncclInfo* info) {
 if (info->comm->userStreamSet == false) {
    info->comm->userStream = info->stream;
    info->comm->userStreamSet = true;
  } else if (info->stream != info->comm->userStream) {
    WARN("Error : mixing different streams within a group call is not supported.");
    return ncclInvalidUsage;
  }
  return ncclSuccess;
}

// Handle structure for user buffer registration (IPC) exchange
/* 中文逐项-类型 T0133：ncclBuffRegHandle
 * 类型职责：CUDA Graph用户缓冲注册优化的IPC导出/导入地址信息，区别于普通verbs MR。
 * 声明内可见成员（含嵌套成员）：sendBuffIpc、recvBuffIpc、sendBuffOffset、recvBuffOffset。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclBuffRegHandle {
/* 中文逐项-变量 V0952：sendBuffIpc
 * sendBuffIpc（cudaIpcMemHandle_t；成员 ncclBuffRegHandle）：可跨进程交换的CUDA内存导出句柄，与导入后本进程实际地址不同。
 */
  cudaIpcMemHandle_t sendBuffIpc;
/* 中文逐项-变量 V0953：recvBuffIpc
 * recvBuffIpc（cudaIpcMemHandle_t；成员 ncclBuffRegHandle）：可跨进程交换的CUDA内存导出句柄，与导入后本进程实际地址不同。
 */
  cudaIpcMemHandle_t recvBuffIpc;
/* 中文逐项-变量 V0954：sendBuffOffset
 * sendBuffOffset（ssize_t；成员 ncclBuffRegHandle）：用户输入/输出相对CUDA分配基址的字节偏移，IPC导入基址后需加回该偏移。
 */
  ssize_t sendBuffOffset;
/* 中文逐项-变量 V0955：recvBuffOffset
 * recvBuffOffset（ssize_t；成员 ncclBuffRegHandle）：用户输入/输出相对CUDA分配基址的字节偏移，IPC导入基址后需加回该偏移。
 */
  ssize_t recvBuffOffset;
};

// Register input and output buffers
// Exchange with ranks on the same host
/* 中文逐项-函数 F0408：ncclRegBuffAndExchange
 * 用途与位置：图注册优化下导出/导入用户缓冲 IPC 信息并交换地址，让 CollNet 可直接使用注册缓冲。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 参数 regInfo（struct ncclBuffRegInfo* regInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclSetupCollKernel。
 * 本体直接出现的调用/包装：CUDACHECK；CUDACHECKGOTO；NCCLCHECK；bootstrapIntraNodeAllGather；comm->pfnCuMemGetAddressRange；cudaIpcGetMemHandle；cudaIpcOpenMemHandle。
 * 需要对照的边界/推进条件：comm->localRanks == 1；comm->pfnCuMemGetAddressRange == NULL；i<comm->localRanks；i ==
 * comm->localRank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclRegBuffAndExchange(struct ncclInfo* info, struct ncclBuffRegInfo* regInfo) {
/* 中文逐项-变量 V0956：comm
 * comm（ncclComm_t；函数 ncclRegBuffAndExchange 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = info->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclComm_t comm = info->comm;
  if (comm->localRanks == 1) return ncclSuccess;
  if (comm->pfnCuMemGetAddressRange == NULL) return ncclSuccess;  // CUDA toolkit or driver version too old

/* 中文逐项-变量 V0957：ret
 * ret（ncclResult_t；函数 ncclRegBuffAndExchange 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  ncclResult_t ret = ncclSuccess;
/* 中文逐项-变量 V0958：regHandles
 * regHandles（struct ncclBuffRegHandle；函数 ncclRegBuffAndExchange 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECKGOTO(cudaIpcGetMemHandle(&regHandles[comm->localRank].sendBuffIpc,
 * (void*)info->sendbuff), ret, reg_fallback);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclBuffRegHandle regHandles[NCCL_MAX_LOCAL_RANKS];
  // Get IPC handles
  // Note: the handle only corresponds to the base address of the allocation
  CUDACHECKGOTO(cudaIpcGetMemHandle(&regHandles[comm->localRank].sendBuffIpc, (void*)info->sendbuff), ret, reg_fallback);
  CUDACHECKGOTO(cudaIpcGetMemHandle(&regHandles[comm->localRank].recvBuffIpc, (void*)info->recvbuff), ret, reg_fallback);
  // Get offset of user buffer within allocation
/* 中文逐项-变量 V0959：baseAddr
 * baseAddr（void；函数 ncclRegBuffAndExchange 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(comm->pfnCuMemGetAddressRange(&baseAddr, &size,
 * (void*)info->sendbuff));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  void* baseAddr;
/* 中文逐项-变量 V0960：size
 * size（size_t；函数 ncclRegBuffAndExchange 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(comm->pfnCuMemGetAddressRange(&baseAddr, &size,
 * (void*)info->sendbuff));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  size_t size;
  // Get base address
  CUDACHECK(comm->pfnCuMemGetAddressRange(&baseAddr, &size, (void*)info->sendbuff));
  regHandles[comm->localRank].sendBuffOffset = (char*)info->sendbuff - (char*)baseAddr;
  CUDACHECK(comm->pfnCuMemGetAddressRange(&baseAddr, &size, (void*)info->recvbuff));
  regHandles[comm->localRank].recvBuffOffset = (char*)info->recvbuff - (char*)baseAddr;
  TRACE(NCCL_COLL, "Base %p size %lu offset %ld", baseAddr, size, regHandles[comm->localRank].recvBuffOffset);

  // Exchange handles within node
  NCCLCHECK(bootstrapIntraNodeAllGather(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, regHandles, sizeof(struct ncclBuffRegHandle)));
  // Open handles at local process
  for (/* 中文逐项-变量 V0961：i
 * i（int；函数 ncclRegBuffAndExchange 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->localRanks; i++) {
    // Skip myself
    if (i == comm->localRank) {
      regInfo->sendbuffsBase[i] = regInfo->recvbuffsBase[i] = NULL;
      continue;
    }
    // Get base address of mapping
    CUDACHECK(cudaIpcOpenMemHandle(regInfo->sendbuffsBase+i, regHandles[i].sendBuffIpc, cudaIpcMemLazyEnablePeerAccess));
    CUDACHECK(cudaIpcOpenMemHandle(regInfo->recvbuffsBase+i, regHandles[i].recvBuffIpc, cudaIpcMemLazyEnablePeerAccess));
    // Get real buffer address by adding offset in the mapping
    regInfo->sendbuffs[i] = (char*)regInfo->sendbuffsBase[i] + regHandles[i].sendBuffOffset;
    regInfo->recvbuffs[i] = (char*)regInfo->recvbuffsBase[i] + regHandles[i].recvBuffOffset;
  }
  // Marks the operation as being buffer registered
  regInfo->nBuffs = comm->localRanks;
  TRACE(NCCL_COLL, "Rank %d exchanged %d buffers", comm->rank, regInfo->nBuffs);
  return ncclSuccess;

reg_fallback:
  // If we cannot register specific buffer types, we just bypass this stage, and continue without failing
  (void)ret;
  WARN("Unable to register user buffers");
  return ncclSuccess;
}

// Compute enqueue element, save it in list
// Compute CUDA launch parameters
// Capture time code in view of CUDA graph
/* 中文导读：
 * 准备集合通信的排队描述：计算算法、协议、工作元素和 Proxy 工作，而不是在 API 包装层做数据归约。
 * 后续 HostSetup 将描述放进 channel FIFO，Launch 阶段再提交 GPU kernel。
 */
/* 中文逐项-函数 F0409：ncclSetupCollKernel
 * 用途与位置：创建 collective 排队模板、计算 GPU/Proxy 参数、更新 grid/block 配置并选择入口 kernel。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclSetupAsyncKernels；src/enqueue.cc::ncclEnqueueCheck。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；comm->enqueueInfo->elemList->getNewElem；computeColl；cudaMemcpyAsync；memcpy；ncclRegBuffAndExchange；std::max<unsigned>；std::min<unsigned>。
 * 需要对照的边界/推进条件：comm->nRanks == 1 && // User-defined reduction ops may need alter the data even for unitary reductions
 * info->op < ncclNumOps；info->sendbuff != info->recvbuff；params->func == NULL；work->header.type ==
 * ncclWorkTypeColl。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclSetupCollKernel(struct ncclInfo* info) {
/* 中文逐项-变量 V0962：comm
 * comm（ncclComm_t；函数 ncclSetupCollKernel 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = info->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclComm_t comm = info->comm;
  if (comm->nRanks == 1 &&
      // User-defined reduction ops may need alter the data even for unitary reductions
      info->op < ncclNumOps) {
    if (info->sendbuff != info->recvbuff)
      CUDACHECK(cudaMemcpyAsync(info->recvbuff, info->sendbuff, info->nBytes, cudaMemcpyDeviceToDevice, info->stream));
    return ncclSuccess;
  }

  // Compute cuda kernel arg and proxy arg templates
/* 中文逐项-变量 V0963：eqElem
 * eqElem（struct ncclQueueElem；函数 ncclSetupCollKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(comm->enqueueInfo->elemList->getNewElem(&eqElem));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclQueueElem* eqElem;
  NCCLCHECK(comm->enqueueInfo->elemList->getNewElem(&eqElem));
/* 中文逐项-变量 V0964：work
 * work（struct ncclWork；函数 ncclSetupCollKernel 内）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 * 初值/来源 work = &eqElem->work；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclWork* work = &eqElem->work;
  NCCLCHECK(computeColl(info, work->elems, &eqElem->proxyOp));

  // Determine grid size
/* 中文逐项-变量 V0965：params
 * params（struct cudaLaunchParams；函数 ncclSetupCollKernel 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 * 初值/来源 params = comm->myParams；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct cudaLaunchParams* params = comm->myParams;
  params->gridDim.x += info->nChannels;
  params->gridDim.x = std::min<unsigned>(params->gridDim.x, comm->nChannels);
  params->blockDim.x = std::max<unsigned>(params->blockDim.x, info->nThreads);
  comm->enqueueInfo->maxChannels = params->gridDim.x;  // params may be varied by a second graph hence we need to capture it here

  // Inline the first kernel
  if (params->func == NULL) {
    params->func = ncclKerns[work->header.funcIndex];
    if (work->header.type == ncclWorkTypeColl) {
      // Copy the first operation to the inline argument. Type may be set later to
      // ncclWorkTypeUnused if we have more than one coll element.
      memcpy(&comm->args, work->elems, sizeof(struct ncclWorkElem));
      comm->args.bid = 0;    // Only inline for channel 0
      comm->args.header.isLast = 1; // I am so far the last element
    }
  }

  // Register and exchange input and output buffers
  if (comm->usingCudaGraph &&                   // only in CUDA graph mode
      comm->graphRegister == 1 &&               // when registration is enabled
      info->algorithm == NCCL_ALGO_COLLNET &&   // limited to CollNet for now
      comm->intraHighestTransportType == TRANSPORT_P2P && // only when all ranks can p2p each other
      comm->intraRanks == 1) {                  // only in multi-process mode
    NCCLCHECK(ncclRegBuffAndExchange(info, &eqElem->buffRegInfo));
    comm->enqueueInfo->nRegBuffs += eqElem->buffRegInfo.nBuffs;
    work->header.type = ncclWorkTypeRegColl;
    // Disable inline argument because we need kernel to copy the entire ncclWork from workFifo
    // because the registered addresses are in ncclWorkElemReg
    comm->args.header.type = ncclWorkTypeUnused;
  }

  return ncclSuccess;
}

// Find the channel with the least enqueued work (counted in bytes)
/* 中文精读：Group 聚合怎样选择物理 channel
 * 聚合可按已排入字节数找最短队列，也可按 lastChannel 轮转，尽量让多个 block 负载接近。
 * 分配后的 channelId 是 comm->channels 数组下标；work->bid 是本次操作内的分片编号。
 * 例如某操作占用物理 channel 3、4，bid 仍是 0、1；GPU 用 bid 计算输入偏移，
 * 用 blockIdx.x 找实际 channel 的拓扑与连接，两种编号不能互相替换。
 */
/* 中文逐项-函数 F0410：findShortestChannel
 * 用途与位置：getNextChannel 的最短队列策略：按已排队字节数选负载较小的逻辑 channel。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::getNextChannel。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：c<comm->nChannels；channel->totalSize < minSize。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return minC;。CHECK类宏还可能提前返回/跳转。
 */
static inline int findShortestChannel(ncclComm_t comm) {
/* 中文逐项-变量 V0966：minSize
 * minSize（size_t；函数 findShortestChannel 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 minSize = SIZE_MAX；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  size_t minSize = SIZE_MAX;
/* 中文逐项-变量 V0967：minC
 * minC（int；函数 findShortestChannel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minC = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int minC = 0;
  for (/* 中文逐项-变量 V0968：c
 * c（int；函数 findShortestChannel 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->nChannels; c++) {
/* 中文逐项-变量 V0969：channel
 * channel（struct ncclChannel；函数 findShortestChannel 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+c;
    if (channel->totalSize < minSize) {
      minSize = channel->totalSize;
      minC = c;
    }
  }
  return minC;
}

// Get next channel based on shortest-queue mode or round-robin mode
/* 中文逐项-函数 F0411：getNextChannel
 * 用途与位置：按轮转或最短队列策略为当前 collective 分片选择实际 channelId。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 aggMode（int aggMode）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCollKernel。
 * 本体直接出现的调用/包装：findShortestChannel。
 * 需要对照的边界/推进条件：aggMode && comm->asyncAllocMode == ncclComm::SHORTEST_QUEUE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return nextChannel;。CHECK类宏还可能提前返回/跳转。
 */
static inline int getNextChannel(ncclComm_t comm, int aggMode) {
/* 中文逐项-变量 V0970：nextChannel
 * nextChannel（int；函数 getNextChannel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nextChannel = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int nextChannel = 0;
  if (aggMode && comm->asyncAllocMode == ncclComm::SHORTEST_QUEUE) {
    nextChannel = findShortestChannel(comm);
  } else {
    nextChannel = comm->lastChannel % comm->nChannels;
    comm->lastChannel++;
  }
  return nextChannel;
}

// Setup aggregated kernels
// Op info has been previously saved in comm->asyncOps
/* 中文精读：多个 API 如何打包进一次 kernel 发射
 * 组内 API 先保存在 asyncOps，最外层 GroupEnd 到这里统一估计每项所需 channel。
 * channelSize 是每通道目标工作量；消息太小会缩小它，让现有通道得到更多有效工作。
 * 同操作/归约/类型的 homogeneous 批次可按总字节数统一选算法和协议，减少重复调度与小消息延迟。
 * 异构批次各自计算。随后把兼容元素装进 work，GPU 在同一个 block 内顺序解释执行，
 * 并非把用户数据先合并成一个新的连续大数组。
 */
/* 中文逐项-函数 F0412：ncclSetupAsyncKernels
 * 用途与位置：把 Group 保存的一组 collective 转成发射任务，对同构批次可统一选型并聚合工作元素。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：DIVUP；NCCLCHECK；getAlgoInfo；getCollNetSupport；ncclSetupCollKernel；std::max；std::min。
 * 需要对照的边界/推进条件：comm->asyncOpCount == 0；comm->asyncOpCount == 1；comm->channelSize > 0；comm->collNetSupport &&
 * comm->asyncOps[0].coll == ncclFuncAllReduce。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSetupAsyncKernels(ncclComm_t comm) {
  if (comm->asyncOpCount == 0) {
    return ncclSuccess;
  } else if (comm->asyncOpCount == 1) {
    // No aggregation
/* 中文逐项-变量 V0971：info
 * info（struct ncclInfo；函数 ncclSetupAsyncKernels 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = comm->asyncOps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclInfo* info = comm->asyncOps;
    info->nChannels = 0;
    NCCLCHECK(ncclSetupCollKernel(info));
  } else {
    // Aggregation
    // Determine a per-channel chunk size used to divide an operation into multiple channels
/* 中文逐项-变量 V0972：channelSize
 * channelSize（size_t；函数 ncclSetupAsyncKernels 内）：聚合时每通道目标规模、轮转游标和最短队列/轮转策略。
 * 此处未显式给初值；后续首个文本使用点：if (comm->channelSize > 0) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    size_t channelSize;
    if (comm->channelSize > 0) {
      // Set by user
      channelSize = comm->channelSize;
    } else if (comm->collNetSupport && comm->asyncOps[0].coll == ncclFuncAllReduce) {
      // CollNet specific size (tuned based on experiments)
      channelSize = 256 * 1024;
    } else {
      // Latency increases as scale increases
      // We would thus want to increase the chunk size to compensate for the lost efficiency
      channelSize = NCCL_AGG_CHANNEL_SIZE * std::min(16, comm->nRanks);
    }
    // Reduce the per-channel size if we cannot fully utilize the channels
    while (comm->asyncTotalSize < channelSize * comm->nChannels && channelSize > NCCL_MIN_CHANNEL_SIZE) channelSize /= 2;
    // Check whether the ops have same reduce and data types (and hence can be packed in same ncclWork)
/* 中文逐项-变量 V0973：channelUsed
 * channelUsed（int；函数 ncclSetupAsyncKernels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channelUsed = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int channelUsed = 0;
/* 中文逐项-变量 V0974：homogeneous
 * homogeneous（int；函数 ncclSetupAsyncKernels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 homogeneous = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int homogeneous = 1;
/* 中文逐项-变量 V0975：allCollNetSupport
 * allCollNetSupport（int；函数 ncclSetupAsyncKernels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 allCollNetSupport = comm->collNetSupport；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int allCollNetSupport = comm->collNetSupport;
    for (/* 中文逐项-变量 V0976：c
 * c（int；函数 ncclSetupAsyncKernels 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c < comm->asyncOpCount；循环体与增量表达式决定每轮如何推进。
 */int c = 0; c < comm->asyncOpCount; c++) {
/* 中文逐项-变量 V0977：info
 * info（struct ncclInfo；函数 ncclSetupAsyncKernels 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = comm->asyncOps+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclInfo* info = comm->asyncOps+c;
      info->nChannels = std::min(std::max(1, (int)DIVUP(info->nBytes, channelSize)), comm->nChannels); // assign number of channels
      channelUsed += info->nChannels;
      // We can use fast path if all collectives are the same
      homogeneous &= info->coll == comm->asyncOps[0].coll &&
                     info->opFull.op == comm->asyncOps[0].opFull.op &&
                     info->datatype == comm->asyncOps[0].datatype;
      if (allCollNetSupport > 0) NCCLCHECK(getCollNetSupport(info, &allCollNetSupport));
    }
    // Compute algo, proto, nthreads for the entire kernel
    // Prepare a synthetic op info to calculate the collective algo
/* 中文逐项-变量 V0978：total
 * total（struct ncclInfo；函数 ncclSetupAsyncKernels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：total.comm = comm;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclInfo total;
    total.comm = comm;
    total.coll = comm->asyncOps[0].coll;
    total.nBytes = comm->asyncTotalSize;
    total.nChannels = std::min(channelUsed, comm->nChannels);
/* 中文逐项-变量 V0979：perChannelOps
 * perChannelOps（int；函数 ncclSetupAsyncKernels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 perChannelOps = DIVUP(channelUsed, total.nChannels)；向上取整分配，确保不能整除时也覆盖最后不足一块的数据。
 */
    int perChannelOps = DIVUP(channelUsed, total.nChannels);
    if (homogeneous) NCCLCHECK(getAlgoInfo(&total, allCollNetSupport, perChannelOps));
    // Set for each op
    for (/* 中文逐项-变量 V0980：c
 * c（int；函数 ncclSetupAsyncKernels 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c < comm->asyncOpCount；循环体与增量表达式决定每轮如何推进。
 */int c = 0; c < comm->asyncOpCount; c++) {
/* 中文逐项-变量 V0981：info
 * info（struct ncclInfo；函数 ncclSetupAsyncKernels 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = comm->asyncOps+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclInfo* info = comm->asyncOps+c;
      if (homogeneous) {
        // Set fields to skip the individual computeColl in ncclSetupCollKernel
        info->algorithm = total.algorithm;
        info->protocol = total.protocol;
        info->nThreads = total.nThreads;
      }
      NCCLCHECK(ncclSetupCollKernel(info));
    }
    comm->args.header.type = ncclWorkTypeUnused;  // disable inline argument
  }
  // Reset counters
  comm->asyncOpCount = 0;
  comm->asyncTotalSize = 0;
  return ncclSuccess;
}

// Store aggregated operations info
/* 中文逐项-函数 F0413：ncclSaveAsyncColl
 * 用途与位置：复制当前 collective 的主机中间描述到可延迟提交列表，保留 GroupEnd 计算所需参数。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck。
 * 本体直接出现的调用/包装：memcpy。
 * 需要对照的边界/推进条件：comm->asyncOpCount >= NCCL_MAX_OPS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclSaveAsyncColl(struct ncclInfo* info) {
/* 中文逐项-变量 V0982：comm
 * comm（ncclComm_t；函数 ncclSaveAsyncColl 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = info->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclComm_t comm = info->comm;
  if (comm->asyncOpCount >= NCCL_MAX_OPS) {
    WARN("Too many async operations in progress, max is %d", NCCL_MAX_OPS);
    return ncclInvalidUsage;
  }
  memcpy(comm->asyncOps+comm->asyncOpCount, info, sizeof(struct ncclInfo));
  comm->asyncOpCount++;
  comm->asyncTotalSize += info->nBytes;
  return ncclSuccess;
}

// Save p2p operations in comm->p2pSends and p2pRecvs. Operations will be posted to channels
// during ncclGroupEnd()
/* 中文导读：
 * 按 peer 保存 Send/Recv，并为尚未连接的 channel/peer 的索引 1 设置建链标记。
 * 这解释了为什么 communicator 初始化完成后，首次访问新 peer 仍可能增加连接及 QP/MR。
 */
/* 中文逐项-函数 F0414：ncclSaveP2p
 * 用途与位置：按 peer 保存 P2P 消息和字节数，并标记尚未建立的 channel/方向/索引1连接需求。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclChannelComputeBase；ncclChannelComputeFromBase；ncclSaveP2pInfo；ncclTypeSize。
 * 需要对照的边界/推进条件：info->coll == ncclFuncSend；peer !=
 * comm->rank；c<comm->p2pnChannelsPerPeer；comm->channels[channelId].peers[peer].send[1].connected == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclSaveP2p(struct ncclInfo* info) {
/* 中文逐项-变量 V0983：comm
 * comm（struct ncclComm；函数 ncclSaveP2p 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = info->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclComm* comm = info->comm;
/* 中文逐项-变量 V0984：peer
 * peer（int；函数 ncclSaveP2p 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = info->root；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int peer = info->root;
/* 中文逐项-变量 V0985：nBytes
 * nBytes（ssize_t；函数 ncclSaveP2p 内）：当前消息或片段的字节规模；info.nBytes 对 AG/RS 还计入 nRanks 段总规模。
 * 初值/来源 nBytes = info->count*ncclTypeSize(info->datatype)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ssize_t nBytes = info->count*ncclTypeSize(info->datatype);
/* 中文逐项-变量 V0986：channelBaseId
 * channelBaseId（int；函数 ncclSaveP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclChannelComputeBase(comm, peer, info->coll,
 * &channelBaseId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int channelBaseId;
  NCCLCHECK(ncclChannelComputeBase(comm, peer, info->coll, &channelBaseId));
  if (info->coll == ncclFuncSend) {
    if (peer != comm->rank) {
      // Mark channels that need pre-connect
      for (/* 中文逐项-变量 V0987：c
 * c（int；函数 ncclSaveP2p 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->p2pnChannelsPerPeer；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->p2pnChannelsPerPeer; c++) {
/* 中文逐项-变量 V0988：channelId
 * channelId（int；函数 ncclSaveP2p 内）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclChannelComputeFromBase(comm, channelBaseId, c,
 * &channelId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int channelId;
        NCCLCHECK(ncclChannelComputeFromBase(comm, channelBaseId, c, &channelId));
        if (comm->channels[channelId].peers[peer].send[1].connected == 0) { // P2P uses only 1 connector
          comm->connectSend[peer] |= (1<<channelId);
          comm->connect = 1;
        }
      }
    }
    NCCLCHECK(ncclSaveP2pInfo(comm->p2pSends[info->root], info->recvbuff, nBytes));
    comm->p2pSendCount++;
  } else {
    if (peer != comm->rank) {
      // Mark channels that need pre-connect
      for (/* 中文逐项-变量 V0989：c
 * c（int；函数 ncclSaveP2p 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->p2pnChannelsPerPeer；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->p2pnChannelsPerPeer; c++) {
/* 中文逐项-变量 V0990：channelId
 * channelId（int；函数 ncclSaveP2p 内）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclChannelComputeFromBase(comm, channelBaseId, c,
 * &channelId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int channelId;
        NCCLCHECK(ncclChannelComputeFromBase(comm, channelBaseId, c, &channelId));
        if (comm->channels[channelId].peers[peer].recv[1].connected == 0) { // P2P uses only 1 connector
          comm->connectRecv[peer] |= (1<<channelId);
          comm->connect = 1;
        }
      }
    }
    NCCLCHECK(ncclSaveP2pInfo(comm->p2pRecvs[info->root], info->recvbuff, nBytes));
    comm->p2pRecvCount++;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0415：getSegment
 * 用途与位置：在已有 work 里寻找可放入当前类型/peer/方向任务的槽位，避免破坏固定收发分组布局。
 * 参数 type（enum ncclWorkElemType type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 subType（enum ncclWorkElemSubType subType）：P2P 元素发送/接收方向类别，影响装填时的偶数接收/奇数发送布局。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 work（struct ncclWork* work）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueP2pKernel；src/enqueue.cc::ncclEnqueueCollKernel。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：work->header.type && (work->header.type != type；type ==
 * ncclWorkTypeP2p；s<NCCL_MAX_WORK_ELEMENTS_P2P；work->p2pElems[s].peer == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return -1;；return s;。CHECK类宏还可能提前返回/跳转。
 */
static int getSegment(enum ncclWorkElemType type, enum ncclWorkElemSubType subType, int peer, struct ncclWork* work) {
  if (work->header.type && (work->header.type != type)) return -1;

  if (type == ncclWorkTypeP2p) {  // P2P
/* 中文逐项-变量 V0991：start
 * start（int；函数 getSegment 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 start = subType == ncclWorkSubTypeRecv ? 0 : 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int start = subType == ncclWorkSubTypeRecv ? 0 : 1;
    for (/* 中文逐项-变量 V0992：s
 * s（int；函数 getSegment 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = start；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：s<NCCL_MAX_WORK_ELEMENTS_P2P；循环体与增量表达式决定每轮如何推进。
 */int s=start; s<NCCL_MAX_WORK_ELEMENTS_P2P; s+=2) {
      if (work->p2pElems[s].peer == -1) return s;
      // Do not aggregate multiple sends to the same peer (or receives from the same peer)
      if (work->p2pElems[s].peer == peer) return -1;
    }
  } else if (type == ncclWorkTypeRegColl) { // CollNet
    for (/* 中文逐项-变量 V0993：s
 * s（int；函数 getSegment 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<NCCL_MAX_WORK_ELEMENTS_REG；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<NCCL_MAX_WORK_ELEMENTS_REG; s++) {
      if (work->regElems[s].elem.header.type == ncclWorkTypeUnused) return s;
    }
  } else if (type == ncclWorkTypeColl) {  // Ring or Tree
    for (/* 中文逐项-变量 V0994：s
 * s（int；函数 getSegment 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<NCCL_MAX_WORK_ELEMENTS；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<NCCL_MAX_WORK_ELEMENTS; s++) {
      if (work->elems[s].header.type == ncclWorkTypeUnused) return s;
    }
  }
  return -1;
}

// Compute kernel arguments for P2P ops
/* 中文精读：P2P 工作描述的单位和方向
 * Group 调度已把消息换成 ncclInt8，所以此处 count 表示字节，chunkSize 也表示字节。
 * Send/Recv 共用 ncclWorkElemP2p：subType 区分方向，peer 是对端，buff 是当前块起点。
 * header.nWarps 是整个 P2P work 的线程预算；enqueueSegOp 还会为各个元素分配 warpStart/nWarps。
 * P2P 描述符与 collective 描述符布局不同，不能用 collective 的 count/元素类型规则解释它。
 */
/* 中文逐项-函数 F0416：computeP2pWorkElem
 * 用途与位置：把单个已分块 P2P 任务转换成设备元素，记录字节长度、peer、方向与协议 chunkSize。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 参数 elem（struct ncclWorkElemP2p* elem）：注册work扩展或队列容器内嵌的普通工作元素，扩展信息在它后面保存。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclSetupP2pKernel。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t computeP2pWorkElem(struct ncclInfo* info /* input */, struct ncclWorkElemP2p* elem /* output */) {
  elem->header.type = ncclWorkTypeP2p;
  elem->header.funcIndex = FUNC_INDEX_P2P;
  elem->header.nWarps = NCCL_MAX_NTHREADS/WARP_SIZE;
  elem->buff = info->recvbuff;
  elem->subType = info->coll == ncclFuncSend ? ncclWorkSubTypeSend : ncclWorkSubTypeRecv;
  elem->count = info->count;
  elem->chunkSize = info->chunkSize;
  elem->peer = info->root;
  return ncclSuccess;
}

// Equeue work elements into segment of ncclWork
// Supporting both collectives (aggregated or not) and P2P
/* 中文精读：一条 work 内怎样分配 P2P 线程组
 * P2P 把多项收发装在固定大小的 work 中；ngroups 向上取 2 的幂以便均分线程。
 * 每项记录自己起始 warp 和 warp 数；偶数槽是接收，奇数槽是发送，设备端按同样规则分派。
 * 发送可多配一个同步 warp，让数据复制与 threadfence 的开销重叠。
 * collective 分支则复制完整 workElem；不同 funcIndex/线程数的任务不能任意装进同一条 work。
 */
/* 中文逐项-函数 F0417：enqueueSegOp
 * 用途与位置：把元素装进固定大小 work，对 P2P 计算各收发组 warp 范围，对 collective 复制对应描述。
 * 参数 type（enum ncclWorkElemType type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 elem（struct ncclWork* elem）：注册work扩展或队列容器内嵌的普通工作元素，扩展信息在它后面保存。
 * 参数 work（struct ncclWork* work）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 * 参数 s（int s）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 regInfo（struct ncclBuffRegInfo* regInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 channel（struct ncclChannel* channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueP2pKernel；src/enqueue.cc::ncclEnqueueCollKernel。
 * 本体直接出现的调用/包装：memcpy。
 * 需要对照的边界/推进条件：type == ncclWorkTypeP2p；i<NCCL_MAX_WORK_ELEMENTS_P2P；work->p2pElems[i].header.type；ngroups <
 * nelems。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t enqueueSegOp(enum ncclWorkElemType type, struct ncclWork* elem /* input */, struct ncclWork* work, int s,
    struct ncclBuffRegInfo* regInfo, struct ncclChannel* channel, struct ncclComm* comm) {

  if (type == ncclWorkTypeP2p) {
    memcpy(work->p2pElems+s, elem, sizeof(struct ncclWorkElemP2p));
/* 中文逐项-变量 V0995：nelems
 * nelems（int；函数 enqueueSegOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nelems = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int nelems = 0;
    for (/* 中文逐项-变量 V0996：i
 * i（int；函数 enqueueSegOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_MAX_WORK_ELEMENTS_P2P；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_MAX_WORK_ELEMENTS_P2P; i++) {
      if (work->p2pElems[i].header.type) nelems = i+1;
    }

/* 中文逐项-变量 V0997：ngroups
 * ngroups（int；函数 enqueueSegOp 内）：同一 P2P work 内同时划分的收发线程组数，向上按二次幂组织。
 * 初值/来源 ngroups = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int ngroups = 1;
    while (ngroups < nelems) ngroups *= 2;
/* 中文逐项-变量 V0998：nWarps
 * nWarps（int；函数 enqueueSegOp 内）：本工作元素分到的起始 warp 和 warp 数，决定哪些线程执行本项。
 * 初值/来源 nWarps = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int nWarps = 1;
    while (nWarps*ngroups <= elem->header.nWarps/2) nWarps *= 2;

    for (/* 中文逐项-变量 V0999：i
 * i（int；函数 enqueueSegOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ngroups；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ngroups; i++) {
      work->p2pElems[i].ngroups = ngroups;
      work->p2pElems[i].warpStart =
        i*(NCCL_MAX_NTHREADS/WARP_SIZE)/ngroups;
/* 中文逐项-变量 V1000：extraWarp
 * extraWarp（int；函数 enqueueSegOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 extraWarp = nWarps >= 2 ? i%2 : 0；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      int extraWarp = nWarps >= 2 ? i%2 : 0;
      work->p2pElems[i].nWarps = nWarps + extraWarp;
    }
    return ncclSuccess;
  }

  memcpy(work->elems+s, elem, sizeof(struct ncclWorkElem));

  if (regInfo->nBuffs == 0) return ncclSuccess;

  // Copy registered buffer addresses into ncclWork
/* 中文逐项-变量 V1001：regElem
 * regElem（struct ncclWorkElemReg；函数 enqueueSegOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 regElem = (struct ncclWorkElemReg*)(work->elems+s)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclWorkElemReg* regElem = (struct ncclWorkElemReg*)(work->elems+s);
  // For CollNet
  for (/* 中文逐项-变量 V1002：i
 * i（int；函数 enqueueSegOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_MAX_DIRECT_ARITY；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_MAX_DIRECT_ARITY; i++) {
/* 中文逐项-变量 V1003：peer
 * peer（int；函数 enqueueSegOp 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = channel->collTree.down[i]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int peer = channel->collTree.down[i];
    if (peer == -1) break;
    // Get intra-node slot
/* 中文逐项-变量 V1004：j
 * j（int；函数 enqueueSegOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = comm->rankToLocalRank[peer]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int j = comm->rankToLocalRank[peer];
    if (j < 0) {
      WARN("Invalid intra-node rank %d for peer %d", j, peer);
      return ncclInternalError;
    }
    // Input buffer of leaf peer
    regElem->dnInputs[i] = regInfo->sendbuffs[j];
    // Output buffer of leaf peer
    regElem->dnOutputs[i] = regInfo->recvbuffs[j];
  }
  for (/* 中文逐项-变量 V1005：i
 * i（int；函数 enqueueSegOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_MAX_DIRECT_ARITY；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_MAX_DIRECT_ARITY; i++) {
/* 中文逐项-变量 V1006：peer
 * peer（int；函数 enqueueSegOp 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = channel->collTree.up[i]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int peer = channel->collTree.up[i];
    if (peer == -1) break;
/* 中文逐项-变量 V1007：j
 * j（int；函数 enqueueSegOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = comm->rankToLocalRank[peer]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int j = comm->rankToLocalRank[peer];
    if (j < 0) {
      WARN("Invalid intra-node rank %d for peer %d", j, peer);
      return ncclInternalError;
    }
    // Output buffer of root peer
    regElem->upOutputs[i] = regInfo->recvbuffs[j];
  }
  work->elems[s].regUsed = 1;
  return ncclSuccess;
}

// Enqueue P2P op
/* 中文精读：P2P 从临时描述到 channel FIFO
 * 此时 channelId 已由 peer/块号映射选好，不再走集合通信的轮转选择。
 * 优先找同类型且有空槽的 work 来合并收发；装不下再通过 getNextOp 申请 FIFO 槽位。
 * 同时保存该 P2P 块的 Proxy 操作。GPU work 与 ProxyOp 是同一传输的两份执行计划，
 * 一个驱动 GPU 拷贝/同步，一个驱动 CPU 网络推进，不是重复发送两次数据。
 */
/* 中文逐项-函数 F0418：ncclEnqueueP2pKernel
 * 用途与位置：把已指定 channel 的 P2P 元素和 ProxyOp 装入实际工作 FIFO，可与兼容任务合并。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 eqElem（struct ncclQueueElem* eqElem）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueHostSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；enqueueSegOp；getNextOp；getSegment；ncclProxySaveP2p。
 * 需要对照的边界/推进条件：channel->workCount；segment == -1；i<NCCL_MAX_WORK_ELEMENTS_P2P。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclEnqueueP2pKernel(struct ncclComm* comm, struct ncclQueueElem* eqElem) {
/* 中文逐项-变量 V1008：workElem
 * workElem（struct ncclWorkElemP2p；函数 ncclEnqueueP2pKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 workElem = eqElem->work.p2pElems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclWorkElemP2p* workElem = eqElem->work.p2pElems;
/* 中文逐项-变量 V1009：proxyOp
 * proxyOp（struct ncclProxyOp；函数 ncclEnqueueP2pKernel 内）：交给 CPU Proxy 的紧凑计划，与 GPU work 的协议和步数必须一致。
 * 初值/来源 proxyOp = &eqElem->proxyOp；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyOp* proxyOp = &eqElem->proxyOp;

  // Try to reuse last p2p operation if not full yet
/* 中文逐项-变量 V1010：channel
 * channel（struct ncclChannel；函数 ncclEnqueueP2pKernel 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+proxyOp->channelId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclChannel* channel = comm->channels+proxyOp->channelId;
/* 中文逐项-变量 V1011：opIndex
 * opIndex（int；函数 ncclEnqueueP2pKernel 内）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 初值/来源 opIndex = (channel->workFifoTail-1+NCCL_MAX_OPS)%NCCL_MAX_OPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
  int opIndex = (channel->workFifoTail-1+NCCL_MAX_OPS)%NCCL_MAX_OPS;
/* 中文逐项-变量 V1012：w
 * w（struct ncclWork；函数 ncclEnqueueP2pKernel 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 w = channel->workFifo+opIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclWork* w = channel->workFifo+opIndex;
/* 中文逐项-变量 V1013：segment
 * segment（int；函数 ncclEnqueueP2pKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 segment = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int segment = -1;
  if (channel->workCount) {
    // Try to pack more segments into a single operation
    segment = getSegment(ncclWorkTypeP2p, workElem->subType, workElem->peer, w);
  }
  if (segment == -1) {
    NCCLCHECK(getNextOp(channel, &w, NULL));
    segment = workElem->subType == ncclWorkSubTypeRecv ? 0 : 1;
    // Initialize work as P2P, set peer=-1 to designate the p2p elem is not used.
    w->header.type = ncclWorkTypeP2p;
    for (/* 中文逐项-变量 V1014：i
 * i（int；函数 ncclEnqueueP2pKernel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_MAX_WORK_ELEMENTS_P2P；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_MAX_WORK_ELEMENTS_P2P; i++) w->p2pElems[i].peer = -1;
  }
  //printf("%s to %d -> Channel %d OpCount %ld Segment %d\n", workElem->subType == ncclWorkSubTypeRecv ? "Recv" : "Send", proxyOp->root, channel->id, channel->workFifoTail-1, segment);

  // store work element into FIFO
  NCCLCHECK(ncclProxySaveP2p(comm, proxyOp));
  NCCLCHECK(enqueueSegOp(ncclWorkTypeP2p, &eqElem->work, w, segment, &eqElem->buffRegInfo, channel, comm));
  return ncclSuccess;
}

// Setup P2P op
/* 中文逐项-函数 F0419：ncclSetupP2pKernel
 * 用途与位置：为 P2P 块生成设备/Proxy 计划并更新发射所需的线程和 channel 范围。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/group.cc::scheduleSend；src/group.cc::scheduleRecv。
 * 本体直接出现的调用/包装：NCCLCHECK；comm->enqueueInfo->elemList->getNewElem；computeP2pWorkElem；ncclProxyComputeP2p；std::max<unsigned>。
 * 需要对照的边界/推进条件：params->func == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSetupP2pKernel(struct ncclInfo* info) {
/* 中文逐项-变量 V1015：comm
 * comm（ncclComm；函数 ncclSetupP2pKernel 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = info->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclComm* comm = info->comm;
  // Compute cuda kernel arg and proxy arg templates
/* 中文逐项-变量 V1016：eqElem
 * eqElem（struct ncclQueueElem；函数 ncclSetupP2pKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(comm->enqueueInfo->elemList->getNewElem(&eqElem));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclQueueElem* eqElem;
  NCCLCHECK(comm->enqueueInfo->elemList->getNewElem(&eqElem));
  // The proxy code will set and tune the send/recv chunk size, make sure to run it first.
  NCCLCHECK(ncclProxyComputeP2p(info, &eqElem->proxyOp));
  NCCLCHECK(computeP2pWorkElem(info, eqElem->work.p2pElems));
  // Compute grid size
/* 中文逐项-变量 V1017：channelId
 * channelId（int；函数 ncclSetupP2pKernel 内）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 初值/来源 channelId = info->channelId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int channelId = info->channelId;
/* 中文逐项-变量 V1018：params
 * params（struct cudaLaunchParams；函数 ncclSetupP2pKernel 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 * 初值/来源 params = comm->myParams；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct cudaLaunchParams* params = comm->myParams;
  params->gridDim.x = std::max<unsigned>(params->gridDim.x, channelId+1);
  params->blockDim.x = std::max<unsigned>(params->blockDim.x, eqElem->work.header.nWarps*WARP_SIZE);
  comm->enqueueInfo->maxChannels = params->gridDim.x;  // params may be varied by a second graph hence we need to capture it here

  // Record the first kernel to launch
  // Just for CUDA kernel to know this is a P2P operation
  // The CUDA kernel does not use the inlined first work element as fastpath argument
  if (params->func == NULL) {
    params->func = ncclKerns[eqElem->work.header.funcIndex];
    comm->args.header.type = ncclWorkTypeUnused;
  }
  return ncclSuccess;
}

// Dynamic enqueue function for collective kernels
// Supports both aggregated and non-aggregated modes
/* 中文导读：
 * 把一项 collective 的工作与 Proxy 需求分配到选中的 channel；聚合时可以装入已有 work 记录。
 * 每个 work 的 bid 是此次操作中的分片编号，不能把它始终当成独立的硬件网口编号。
 */
/* 中文逐项-函数 F0420：ncclEnqueueCollKernel
 * 用途与位置：把 collective 模板展开为多个 bid，选择 channel、保存对应 Proxy 工作并装填 FIFO。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 eqElem（struct ncclQueueElem* eqElem）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 aggMode（int aggMode）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueHostSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；enqueueSegOp；getNextChannel；getNextOp；getSegment；ncclProxySaveColl；ncclTypeSize。
 * 需要对照的边界/推进条件：bid<nChannels；proxyOp->nsteps；aggMode && channel->workCount；w->header.funcIndex ==
 * work->header.funcIndex && w->header.nWarps == work->header.nWarps。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclEnqueueCollKernel(struct ncclComm* comm, struct ncclQueueElem* eqElem, int aggMode) {
/* 中文逐项-变量 V1019：work
 * work（struct ncclWork；函数 ncclEnqueueCollKernel 内）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 * 初值/来源 work = &eqElem->work；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclWork* work = &eqElem->work;
/* 中文逐项-变量 V1020：elem
 * elem（struct ncclWorkElem；函数 ncclEnqueueCollKernel 内）：注册work扩展或队列容器内嵌的普通工作元素，扩展信息在它后面保存。
 * 初值/来源 elem = work->elems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclWorkElem* elem = work->elems;
/* 中文逐项-变量 V1021：proxyOp
 * proxyOp（struct ncclProxyOp；函数 ncclEnqueueCollKernel 内）：交给 CPU Proxy 的紧凑计划，与 GPU work 的协议和步数必须一致。
 * 初值/来源 proxyOp = &eqElem->proxyOp；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyOp* proxyOp = &eqElem->proxyOp;

/* 中文逐项-变量 V1022：nChannels
 * nChannels（int；函数 ncclEnqueueCollKernel 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = elem->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nChannels = elem->nChannels;
/* 中文逐项-变量 V1023：channelSize
 * channelSize（size_t；函数 ncclEnqueueCollKernel 内）：聚合时每通道目标规模、轮转游标和最短队列/轮转策略。
 * 初值/来源 channelSize = elem->count*ncclTypeSize(proxyOp->dtype)/elem->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  size_t channelSize = elem->count*ncclTypeSize(proxyOp->dtype)/elem->nChannels;
/* 中文逐项-变量 V1024：workElemType
 * workElemType（enum ncclWorkElemType；函数 ncclEnqueueCollKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 workElemType = proxyOp->redOp == ncclNumOps ? ncclWorkTypeColl :
 * ncclWorkTypeRegColl；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  enum ncclWorkElemType workElemType = proxyOp->redOp == ncclNumOps ? ncclWorkTypeColl : ncclWorkTypeRegColl;  // redOp is only set when using CollNet

  for (/* 中文逐项-变量 V1025：bid
 * bid（int；函数 ncclEnqueueCollKernel 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：bid<nChannels；循环体与增量表达式决定每轮如何推进。
 */int bid=0; bid<nChannels; bid++) {
/* 中文逐项-变量 V1026：channelId
 * channelId（int；函数 ncclEnqueueCollKernel 内）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 初值/来源 channelId = getNextChannel(comm, aggMode)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int channelId = getNextChannel(comm, aggMode);
/* 中文逐项-变量 V1027：channel
 * channel（struct ncclChannel；函数 ncclEnqueueCollKernel 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+channelId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+channelId;

    // Proxy
    proxyOp->channelId = channelId;
    proxyOp->opCount = comm->collOpCount;
    if (proxyOp->nsteps) NCCLCHECK(ncclProxySaveColl(comm, proxyOp, comm->nRanks));

    elem->bid = bid % nChannels;
/* 中文逐项-变量 V1028：w
 * w（struct ncclWork；函数 ncclEnqueueCollKernel 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 w = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    struct ncclWork* w = NULL;
/* 中文逐项-变量 V1029：segment
 * segment（int；函数 ncclEnqueueCollKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 segment = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
    int segment = -1;
    if (aggMode && channel->workCount) {
      // Try to pack more segments into a single operation
/* 中文逐项-变量 V1030：opIndex
 * opIndex（int；函数 ncclEnqueueCollKernel 内）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 初值/来源 opIndex = (channel->workFifoTail-1+NCCL_MAX_OPS)%NCCL_MAX_OPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
      int opIndex = (channel->workFifoTail-1+NCCL_MAX_OPS)%NCCL_MAX_OPS;
      w = channel->workFifo+opIndex;
      // All elems in work must have same (funcIndex,nThreads),
      // see "src/collectives/device/common.h"
      if (w->header.funcIndex == work->header.funcIndex &&
          w->header.nWarps == work->header.nWarps) {
        segment = getSegment(workElemType, ncclWorkSubTypeUnused, 0, w);
      }
    }
    if (segment == -1) {
      NCCLCHECK(getNextOp(channel, &w, NULL));
      segment = 0;
    }

    // store work element into FIFO
    NCCLCHECK(enqueueSegOp(workElemType, work, w, segment, &eqElem->buffRegInfo, channel, comm));
    channel->totalSize += channelSize;
  }
  comm->collOpCount++;
  return ncclSuccess;
}

// Host setup node for CUDA Graph
// Performs the enqueue job
/* 中文逐项-函数 F0421：ncclEnqueueHostSetup
 * 用途与位置：遍历提交批次并执行真正的 FIFO 装填、launch 整理和 Proxy 发布；图 replay 也复用此入口。
 * 参数 arg（void* arg）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：NCCLCHECKGOTO；eqInfo->elemList->begin；eqInfo->elemList->count；eqInfo->elemList->getNext；ncclEnqueueCollKernel；ncclEnqueueP2pKernel；ncclGetErrorString；ncclLaunchProxy；setupLaunch。
 * 需要对照的边界/推进条件：eqElem != NULL；eqElem->work.header.funcIndex == FUNC_INDEX_P2P；ret != ncclSuccess。各条件所属分支见紧邻函数体。
 */
template<int USING_CUDA_GRAPH>
/* 中文导读：
 * 主机侧真正装填任务：遍历排队元素，将 collective/P2P 写入工作 FIFO，完成 launch 配置并发布 Proxy 工作。
 * CUDA Graph 捕获/重放也复用这个桥接点，但元数据的生命周期由相应 QueueInfo/Graph 分支维护。
 */
void CUDART_CB ncclEnqueueHostSetup(void* arg) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文逐项-变量 V1031：ret
 * ret（ncclResult_t；函数 ncclEnqueueHostSetup 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(ncclEnqueueP2pKernel(comm, eqElem), ret, cb_end);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t ret;
  // All work for current launch has been captured in Queue Info
/* 中文逐项-变量 V1032：eqInfo
 * eqInfo（struct ncclQueueInfo；函数 ncclEnqueueHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 eqInfo = (struct ncclQueueInfo*)arg；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclQueueInfo* eqInfo = (struct ncclQueueInfo*)arg;
/* 中文逐项-变量 V1033：comm
 * comm（ncclComm_t；函数 ncclEnqueueHostSetup 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = eqInfo->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclComm_t comm = eqInfo->comm;
/* 中文逐项-变量 V1034：aggMode
 * aggMode（int；函数 ncclEnqueueHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 aggMode = eqInfo->elemList->count() > 1 ? 1 : 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int aggMode = eqInfo->elemList->count() > 1 ? 1 : 0;

  // Iterate through the element list
/* 中文逐项-变量 V1035：eqElem
 * eqElem（struct ncclQueueElem；函数 ncclEnqueueHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 eqElem = eqInfo->elemList->begin()；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclQueueElem* eqElem = eqInfo->elemList->begin();
  while (eqElem != NULL) {
    if (eqElem->work.header.funcIndex == FUNC_INDEX_P2P) {
      NCCLCHECKGOTO(ncclEnqueueP2pKernel(comm, eqElem), ret, cb_end);
    } else {
      NCCLCHECKGOTO(ncclEnqueueCollKernel(comm, eqElem, aggMode), ret, cb_end);
    }
    eqElem = eqInfo->elemList->getNext();
  }

  NCCLCHECKGOTO(setupLaunch(eqInfo, USING_CUDA_GRAPH), ret, cb_end);
  NCCLCHECKGOTO(ncclLaunchProxy(eqInfo), ret, cb_end);

cb_end:
  if (ret != ncclSuccess) {
    WARN("Failure in host setup : %s", ncclGetErrorString(ret));
  }
  eqInfo->ret = ret;
}

template void CUDART_CB ncclEnqueueHostSetup<0>(void*);
template void CUDART_CB ncclEnqueueHostSetup<1>(void*);

// CUDA Graph helper thread
// for de-registering user buffers
/* 中文逐项-函数 F0422：graphHelperFunc
 * 用途与位置：在允许调用 CUDA 的独立线程中关闭图注册缓冲的 IPC 映射，避开图销毁回调的限制。
 * 参数 args（void *args）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECKIGNORE；cudaIpcCloseMemHandle；cudaSetDevice；pthread_cond_wait；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：res == NULL；1；res->ipcHead != ipcTailMark；res->ipcBases[res->ipcHead] != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
void* graphHelperFunc(void *args) {
/* 中文逐项-变量 V1036：res
 * res（struct ncclGraphHelperResources；函数 graphHelperFunc 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = (struct ncclGraphHelperResources*)args；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclGraphHelperResources* res = (struct ncclGraphHelperResources*)args;
  if (res == NULL) {
    WARN("CUDA Graph helper resource is null");
    return NULL;
  }
/* 中文逐项-变量 V1037：dev
 * dev（int；函数 graphHelperFunc 内）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 初值/来源 dev = res->comm->cudaDev；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int dev = res->comm->cudaDev;
  CUDACHECKIGNORE(cudaSetDevice(dev));
  INFO(NCCL_COLL, "CUDA Graph helper thread created for device %d", dev);

/* 中文逐项-变量 V1038：state
 * state（enum helperThreadState；函数 graphHelperFunc 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &res->threadState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  volatile enum helperThreadState* state = &res->threadState;
/* 中文逐项-变量 V1039：ipcTail
 * ipcTail（int；函数 graphHelperFunc 内）：图销毁后待关闭 IPC 基址的有界队列及读写游标。
 * 初值/来源 ipcTail = &res->ipcTail；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  volatile int* ipcTail = &res->ipcTail;
  while (1) {
    // Last IPC entry enqueue so far
/* 中文逐项-变量 V1040：ipcTailMark
 * ipcTailMark（int；函数 graphHelperFunc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ipcTailMark = *ipcTail；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int ipcTailMark = *ipcTail;
/* 中文逐项-变量 V1041：ipcCount
 * ipcCount（int；函数 graphHelperFunc 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 ipcCount = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int ipcCount = 0;
    // Close IPC till the last entry
    while (res->ipcHead != ipcTailMark) {
      if (res->ipcBases[res->ipcHead] != NULL)
        CUDACHECKIGNORE(cudaIpcCloseMemHandle(res->ipcBases[res->ipcHead]));
      res->ipcBases[res->ipcHead] = NULL;
      res->ipcHead = (res->ipcHead+1)%NCCL_IPC_POOL_SIZE;
      ipcCount++;
    }
    TRACE(NCCL_COLL, "CUDA Graph helper thread closed %d IPC handles", ipcCount);
    pthread_mutex_lock(&res->threadLock);
    // Check for exit signal
    while (res->ipcHead == *ipcTail && *state != ThreadStop) {
      pthread_cond_wait(&res->threadCond, &res->threadLock);
    }
    pthread_mutex_unlock(&res->threadLock);
    if (*state == ThreadStop) {
      INFO(NCCL_COLL, "CUDA Graph helper thread for device %d returning", dev);
      return NULL;
    }
  }
}

// Check if we are in CUDA Graph capture mode
/* 中文精读：捕获与执行是两个时间点
 * 调用 API 时可能正在捕获 CUDA Graph；这时要保留 QueueInfo，供将来图执行时填写工作 FIFO。
 * 普通模式可以立即使用/回收排队状态；图模式不能引用随后会被覆盖的 comm->myParams 临时内容。
 * 本函数检查 stream capture 状态和图标识，后续 HostSetup 节点在每次 replay 时重新发布任务。
 * CUDA Graph 优化的是提交组织方式，算法分片和协议流控仍然存在。
 */
/* 中文逐项-函数 F0423：ncclGetCudaGraph
 * 用途与位置：检查当前 stream 是否捕获、图标识和驱动能力，维护图模式队列及生命周期状态。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（cudaGraph_t* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：CUDACHECK；CUDACHECKGOTO；cudaStreamGetCaptureInfo_v2；cudaStreamIsCapturing；ncclSetThreadName；pthread_cond_init；pthread_create；pthread_mutex_init。
 * 需要对照的边界/推进条件：comm->driverVersion < 11030；captureStatus == cudaStreamCaptureStatusActive；cudaGraphId !=
 * comm->lastCudaGraphId；comm->launchMode == ncclComm::GROUP。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInvalidUsage;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclGetCudaGraph(ncclComm_t comm, cudaGraph_t* graph) {
  comm->usingCudaGraph = 0;
  // Feature requires CUDA 11.3/R465 or above
#if CUDART_VERSION >= 11030
/* 中文逐项-变量 V1042：captureStatus
 * captureStatus（cudaStreamCaptureStatus；函数 ncclGetCudaGraph 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECKGOTO(cudaStreamGetCaptureInfo_v2(comm->userStream, &captureStatus, &cudaGraphId, graph,
 * NULL, NULL), ret, enh_compat_end);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cudaStreamCaptureStatus captureStatus;
/* 中文逐项-变量 V1043：cudaGraphId
 * cudaGraphId（unsigned long long；函数 ncclGetCudaGraph 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECKGOTO(cudaStreamGetCaptureInfo_v2(comm->userStream, &captureStatus, &cudaGraphId, graph,
 * NULL, NULL), ret, enh_compat_end);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  unsigned long long cudaGraphId;
/* 中文逐项-变量 V1044：ret
 * ret（ncclResult_t；函数 ncclGetCudaGraph 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  ncclResult_t ret = ncclSuccess;
  if (comm->driverVersion < 11030) {
    // Runtime driver version older than compiler version
    // Enhanced compat fallback
    goto enh_compat_end;
  }
  // Get CUDA Graph handle
  CUDACHECKGOTO(cudaStreamGetCaptureInfo_v2(comm->userStream, &captureStatus, &cudaGraphId, graph, NULL, NULL), ret, enh_compat_end);
  if (captureStatus == cudaStreamCaptureStatusActive) {
    if (cudaGraphId != comm->lastCudaGraphId) {
      INFO(NCCL_COLL, "stream is being captured by a new graph, id %llu", cudaGraphId);
      // We are in a new graph, hence need to forget the last setup node so that
      // the first setup node in the new graph will not have a dependency
      comm->lastCudaGraphId = cudaGraphId;
      comm->lastSetupNode = NULL;
    }
    if (comm->launchMode == ncclComm::GROUP) comm->launchMode = ncclComm::GROUP_GRAPH;
    comm->usingCudaGraph = 1;

    // Create helper thread that closes IPC handles during graph destruction
    // Only create this thread when buffer registration is enabled
    if ((!comm->graphHelperThread) && comm->graphRegister == 1 && comm->disableGraphHelper == 0) {
      pthread_mutex_init(&comm->graphHelperResources->threadLock, NULL);
      // Init signaling method between Graph destroy function and helper thread
      pthread_cond_init(&comm->graphHelperResources->threadCond, NULL);
      // Set state
      comm->graphHelperResources->threadState = ThreadStart;
      // Create thread
      pthread_create(&comm->graphHelperThread, NULL, graphHelperFunc, comm->graphHelperResources);
      // Name thread
      ncclSetThreadName(comm->graphHelperThread, "NCCL GrHelper%2d", comm->cudaDev);
    }
  }
  return ncclSuccess;

enh_compat_end: // Enhanced compat fallback
  (void)ret;
  CUDACHECK(cudaStreamIsCapturing(comm->userStream, &captureStatus));
  if (captureStatus != cudaStreamCaptureStatusNone) {
    WARN("The installed CUDA driver is older than the minimum version (R465) required for NCCL's CUDA Graphs support");
    return ncclInvalidUsage;
  }
  // If we are not in capture mode, we can ignore the driver being lower
#endif
  return ncclSuccess;
}

// Create host setup node in CUDA Graph
/* 中文逐项-函数 F0424：ncclCudaGraphHostSetup
 * 用途与位置：给 CUDA Graph 添加/关联 HostSetup 节点及持有对象，确保 replay 时重新发布 GPU/Proxy 工作。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（cudaGraph_t graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：CUDACHECK；cudaGraphAddHostNode；cudaGraphRetainUserObject；cudaStreamUpdateCaptureDependencies；cudaUserObjectCreate。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCudaGraphHostSetup(ncclComm_t comm, cudaGraph_t graph) {
#if CUDART_VERSION >= 11030
/* 中文逐项-变量 V1045：eqInfo
 * eqInfo（struct ncclQueueInfo；函数 ncclCudaGraphHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 eqInfo = comm->enqueueInfo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclQueueInfo* eqInfo = comm->enqueueInfo;
  // Create a CUDA object to wrap around the argument space
  // which CUDA graph would manage lifetime of
/* 中文逐项-变量 V1046：object
 * object（cudaUserObject_t；函数 ncclCudaGraphHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaUserObjectCreate(&object, eqInfo, ncclDestroyQueueInfo, 1 ,
 * cudaUserObjectNoDestructorSync));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cudaUserObject_t object;
  CUDACHECK(cudaUserObjectCreate(&object, eqInfo, ncclDestroyQueueInfo, 1/*initialRefcount*/, cudaUserObjectNoDestructorSync));
  // Hand over ownership to CUDA Graph
  CUDACHECK(cudaGraphRetainUserObject(graph, object, 1, cudaGraphUserObjectMove));

/* 中文逐项-变量 V1047：fn
 * fn（cudaHostFn_t；函数 ncclCudaGraphHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 fn = ncclEnqueueHostSetup<1>；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  cudaHostFn_t fn = ncclEnqueueHostSetup<1>;
  // Add a CPU node to the graph
/* 中文逐项-变量 V1048：setupNode
 * setupNode（cudaGraphNode_t；函数 ncclCudaGraphHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaGraphAddHostNode(&setupNode, graph, &comm->lastSetupNode, numDependencies,
 * &setupNodeParams));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cudaGraphNode_t setupNode;
  // Function + parameter space for that function (i.e. enqueue info)
/* 中文逐项-变量 V1049：setupNodeParams
 * setupNodeParams（cudaHostNodeParams；函数 ncclCudaGraphHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 setupNodeParams = {fn, eqInfo}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  cudaHostNodeParams setupNodeParams = {fn, eqInfo};
/* 中文逐项-变量 V1050：numDependencies
 * numDependencies（int；函数 ncclCudaGraphHostSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 numDependencies = comm->lastSetupNode == NULL ? 0 : 1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int numDependencies = comm->lastSetupNode == NULL ? 0 : 1;
  CUDACHECK(cudaGraphAddHostNode(&setupNode, graph, &comm->lastSetupNode, numDependencies, &setupNodeParams));
  // Create dependency from last setup node in the same graph
  CUDACHECK(cudaStreamUpdateCaptureDependencies(comm->userStream, &setupNode, 1, cudaStreamAddCaptureDependencies));
  comm->lastSetupNode = setupNode;
  return ncclSuccess;
#else
  WARN("NCCL does not support this CUDA version for CUDA graph feature");
  return ncclInternalError;
#endif
}

/* 中文精读：用户归约句柄如何变成设备操作
 * 公开 ncclRedOp_t 与设备模板枚举不是同一份编码；这里解析内建操作或 communicator 内的用户句柄。
 * 浮点 Avg 可以转成 PreMulSum，让每个输入先乘 1/nRanks；整数平均需在最终结果做 SumPostDiv。
 * scalarArg 可能保存标量的位模式，也可能是设备指针，scalarArgIsPtr 决定 kernel 是否解引用。
 * 组内入队时就复制这些信息，避免用户在 GroupEnd 前销毁操作句柄导致悬空引用。
 */
/* 中文逐项-函数 F0425：hostToDevRedOp
 * 用途与位置：把公开归约句柄转换成设备操作和标量位模式，处理 Avg、用户 PreMulSum 与句柄有效性。
 * 参数 opFull（ncclDevRedOpFull *opFull）：解析公开句柄后的设备归约操作、标量位模式和是否为设备指针的完整信息。
 * 参数 op（ncclRedOp_t op）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 comm（ncclComm *comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck。
 * 本体直接出现的调用/包装：__float2bfloat16；__float2half；float；int；ncclUserRedOpMangle。
 * 需要对照的边界/推进条件：datatype != user->datatype。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t hostToDevRedOp(
    ncclDevRedOpFull *opFull, ncclRedOp_t op, ncclDataType_t datatype, ncclComm *comm
  ) {
/* 中文逐项-类型 T0134：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：i8、u8、i32、u32、i64、u64、f16、bf16、f32、f64、ptr。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V1051：i8
 * i8（int8_t；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
    int8_t i8;
/* 中文逐项-变量 V1052：u8
 * u8（uint8_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    uint8_t u8;
/* 中文逐项-变量 V1053：i32
 * i32（int32_t；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
    int32_t i32;
/* 中文逐项-变量 V1054：u32
 * u32（uint32_t；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
    uint32_t u32;
/* 中文逐项-变量 V1055：i64
 * i64（int64_t；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
    int64_t i64;
/* 中文逐项-变量 V1056：u64
 * u64（uint64_t；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
    uint64_t u64;
/* 中文逐项-变量 V1057：f16
 * f16（half；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
    half f16;
    #if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-变量 V1058：bf16
 * bf16（__nv_bfloat16；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
      __nv_bfloat16 bf16;
    #endif
/* 中文逐项-变量 V1059：f32
 * f32（float；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
    float f32;
/* 中文逐项-变量 V1060：f64
 * f64（double；成员 <匿名>）：同一归约标量位模式的不同数据类型视图，按datatype选择，不是把所有字段独立使用。
 */
    double f64;
/* 中文逐项-变量 V1061：ptr
 * ptr（void；成员 <匿名>）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 */
    void *ptr;
  };
  u64 = 0;
  opFull->scalarArgIsPtr = false;
  switch (int(op)) {
  case ncclSum:  opFull->op = ncclDevSum;  break;
  case ncclProd: opFull->op = ncclDevProd; break;
  case ncclMax:  opFull->op = ncclDevMax;  break;
  case ncclMin:  opFull->op = ncclDevMin;  break;
  case ncclAvg:
    switch ((int)datatype) {
    case ncclInt8:  case ncclInt32:  case ncclInt64:
    case ncclUint8: case ncclUint32: case ncclUint64:
      opFull->op = ncclDevSumPostDiv;
      u64 = comm->nRanks;
      break;
    case ncclFloat16:
      opFull->op = ncclDevPreMulSum;
      f16 = __float2half(float(1.0/comm->nRanks)); // __double2half not supported pre CUDA 11.x
      break;
    #if defined(__CUDA_BF16_TYPES_EXIST__)
    case ncclBfloat16:
      opFull->op = ncclDevPreMulSum;
      bf16 = __float2bfloat16(float(1.0/comm->nRanks));
      break;
    #endif
    case ncclFloat32:
      opFull->op = ncclDevPreMulSum;
      f32 = float(1.0/comm->nRanks);
      break;
    case ncclFloat64:
      opFull->op = ncclDevPreMulSum;
      f64 = 1.0/comm->nRanks;
      break;
    }
    opFull->scalarArgIsPtr = false;
    opFull->scalarArg = u64;
    break;
  default: // user created
/* 中文逐项-变量 V1062：ix
 * ix（int；函数 hostToDevRedOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ix = int(ncclUserRedOpMangle(comm, op)) - int(ncclNumOps)；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
    int ix = int(ncclUserRedOpMangle(comm, op)) - int(ncclNumOps);
/* 中文逐项-变量 V1063：user
 * user（ncclUserRedOp；函数 hostToDevRedOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 user = &comm->userRedOps[ix]；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    ncclUserRedOp *user = &comm->userRedOps[ix];
    if (datatype != user->datatype) {
      WARN("Data type supplied to user-created ncclRedOp_t does not match type "
           "given to reduction operation");
      return ncclInvalidArgument;
    }
    *opFull = user->opFull;
    break;
  }
  return ncclSuccess;
}

/* 中文导读：
 * 所有主要数据通信 API 的统一入口：检查参数、解析归约操作并保存 stream 信息。
 * 在 group 中先保存任务，最外层 GroupEnd 再调度；否则直接准备 collective、HostSetup、Launch 和事件。
 * 这里 isAsync 主要指 group 调度状态，不要与后来版本的 nonblocking communicator 配置混同。
 */
/* 中文逐项-函数 F0426：ncclEnqueueCheck
 * 用途与位置：通信 API 的统一检查与分流入口，组内保存任务，普通路径准备队列并发布/发射执行。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/all_gather.cc::ncclAllGather；src/collectives/all_reduce.cc::ncclAllReduce；src/collectives/broadcast.cc::ncclBroadcast；src/collectives/reduce.cc::ncclReduce；src/collectives/reduce_scatter.cc::ncclReduceScatter；src/collectives/sendrecv.cc::ncclSend；src/collectives/sendrecv.cc::ncclRecv。
 * 本体直接出现的调用/包装：ArgsCheck；CUDACHECK；CUDACHECKGOTO；NCCLCHECK；NCCLCHECKGOTO；PtrCheck；checkSetStream；cudaGetDevice；cudaSetDevice；hostToDevRedOp；ncclAsyncColl；ncclAsyncErrCheck；ncclAsyncMode；ncclCudaGraphHostSetup；ncclEnqueueHostSetup<0>；ncclGetCudaGraph；ncclLaunchBarrier；ncclLaunchKernel；更多调用保留在函数体。
 * 需要对照的边界/推进条件：isAsync && info->comm->checkPointers；isAsync；info->coll == ncclFuncSend || info->coll ==
 * ncclFuncRecv；comm->usingCudaGraph。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ret;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclEnqueueCheck(struct ncclInfo* info) {
/* 中文逐项-变量 V1064：ret
 * ret（ncclResult_t；函数 ncclEnqueueCheck 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  ncclResult_t ret = ncclSuccess;
/* 中文逐项-变量 V1065：isAsync
 * isAsync（bool；函数 ncclEnqueueCheck 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 isAsync = ncclAsyncMode()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
  bool isAsync = ncclAsyncMode();
/* 中文逐项-变量 V1066：savedDev
 * savedDev（int；函数 ncclEnqueueCheck 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 savedDev = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int savedDev = -1;
  // Check arguments
  NCCLCHECK(PtrCheck(info->comm, info->opName, "comm"));
  if (isAsync && info->comm->checkPointers) {
    CUDACHECKGOTO(cudaGetDevice(&savedDev), ret, end);
    CUDACHECKGOTO(cudaSetDevice(info->comm->cudaDev), ret, end);
  }
  NCCLCHECKGOTO(ArgsCheck(info), ret, end);

  // Copy reduction op state from op handle into info struct here since the
  // op handle may be destroyed before ncclGroupEnd().
  NCCLCHECKGOTO(hostToDevRedOp(&info->opFull, info->op, info->datatype, info->comm), ret, end);

  // Launch asynchronously if needed
  if (isAsync) {
    // Always register comm even in case of error to make sure ncclGroupEnd
    // cleans it up.
    NCCLCHECKGOTO(ncclAsyncColl(info->comm), ret, end);
    NCCLCHECKGOTO(checkSetStream(info), ret, end);

    INFO(NCCL_COLL,"%s: opCount %lx sendbuff %p recvbuff %p count %zi datatype %d op %d root %d comm %p [nranks=%d] stream %p",
        info->opName, info->comm->opCount, info->sendbuff, info->recvbuff, info->count,
        info->datatype, info->op, info->root, info->comm, info->comm->nRanks, info->stream);

    if (info->coll == ncclFuncSend || info->coll == ncclFuncRecv) { //p2p stored separately
      NCCLCHECKGOTO(ncclSaveP2p(info), ret, end);
    } else {
      NCCLCHECKGOTO(ncclSaveAsyncColl(info), ret, end);
    }
  } else {
    NCCLCHECKGOTO(checkSetStream(info), ret, end);

    INFO(NCCL_COLL,"%s: opCount %lx sendbuff %p recvbuff %p count %zi datatype %d op %d root %d comm %p [nranks=%d] stream %p",
        info->opName, info->comm->opCount, info->sendbuff, info->recvbuff, info->count,
        info->datatype, info->op, info->root, info->comm, info->comm->nRanks, info->stream);

    // Check whether we are in cuda graph mode
/* 中文逐项-变量 V1067：graph
 * graph（cudaGraph_t；函数 ncclEnqueueCheck 内）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(ncclGetCudaGraph(comm, &graph), ret, end);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    cudaGraph_t graph;
/* 中文逐项-变量 V1068：comm
 * comm（ncclComm_t；函数 ncclEnqueueCheck 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = info->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    ncclComm_t comm = info->comm;
    NCCLCHECKGOTO(ncclGetCudaGraph(comm, &graph), ret, end);

    // Common part between graph mode and non-graph mode
    NCCLCHECKGOTO(ncclSetupCollKernel(info), ret, end);

    // Host setup
    if (comm->usingCudaGraph) {
      NCCLCHECKGOTO(ncclCudaGraphHostSetup(comm, graph), ret, end);
    } else {
      ncclEnqueueHostSetup<0>(comm->enqueueInfo);
      NCCLCHECKGOTO(comm->enqueueInfo->ret, ret, end);
    }

    // Common part between graph mode and non-graph mode
    NCCLCHECKGOTO(ncclLaunchBarrier(comm), ret, end);
    NCCLCHECKGOTO(ncclLaunchKernel(comm), ret, end);
    NCCLCHECKGOTO(ncclRecordEvents(comm), ret, end);
    NCCLCHECKGOTO(ncclLaunchReset(comm), ret, end);
  }
end:
  if (isAsync && savedDev != -1) CUDACHECK(cudaSetDevice(savedDev));
  if (isAsync) ncclAsyncErrCheck(ret);
  return ret;
}

NCCL_API(ncclResult_t, ncclRedOpCreatePreMulSum, ncclRedOp_t *op, void *scalar, ncclDataType_t datatype, ncclScalarResidence_t residence, ncclComm_t comm);
/* 中文精读：自定义归约的边界
 * 本 API 创建的是“先乘指定标量再求和”的操作，不接受任意用户 C/CUDA 回调函数。
 * 用户句柄分配在此 communicator 的 userRedOps 表中，datatype 和 scalar residence 决定保存方式。
 * Destroy 释放的是句柄槽位；已经在 ncclEnqueueCheck 中复制的 opFull 有自己的任务语义。
 * 设备实现继续通过预编译模板执行，避免运行时编译任意归约函数。
 */
/* 中文逐项-函数 F0427：ncclRedOpCreatePreMulSum
 * 用途与位置：在 communicator 中分配并保存先乘标量再求和的操作句柄，按 residence 保存值或设备指针。
 * 参数 op（ncclRedOp_t *op）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 scalar（void *scalar）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 residence（ncclScalarResidence_t residence）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：int；ncclRedOp_t；ncclTypeSize；ncclUserRedOpMangle；reinterpret_cast<uint64_t>；std::memcpy。
 * 需要对照的边界/推进条件：comm->userRedOpFreeHead == comm->userRedOpCapacity；cap < 4；ix < cap；residence ==
 * ncclScalarHostImmediate。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclRedOpCreatePreMulSum(ncclRedOp_t *op, void *scalar, ncclDataType_t datatype, ncclScalarResidence_t residence, ncclComm_t comm) {
  if (comm->userRedOpFreeHead == comm->userRedOpCapacity) {
    // double capacity and resize
/* 中文逐项-变量 V1069：cap
 * cap（int；函数 ncclRedOpCreatePreMulSum 内）：max QP容量描述及发送/接收队列WQE条目上限，不等于已提交请求数。
 * 初值/来源 cap = 2*comm->userRedOpCapacity；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int cap = 2*comm->userRedOpCapacity;
    if (cap < 4) cap = 4;
/* 中文逐项-变量 V1070：ops
 * ops（ncclUserRedOp；函数 ncclRedOpCreatePreMulSum 内）：所处类型的provider函数表、代理待办数组或任务链，按声明类型读取，不能跨类型强转解释。
 * 初值/来源 ops = new ncclUserRedOp[cap]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    ncclUserRedOp *ops = new ncclUserRedOp[cap];
    std::memcpy(ops, comm->userRedOps, comm->userRedOpCapacity*sizeof(ncclUserRedOp));
    for(/* 中文逐项-变量 V1071：ix
 * ix（int；函数 ncclRedOpCreatePreMulSum 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ix = comm->userRedOpCapacity；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：ix < cap；循环体与增量表达式决定每轮如何推进。
 */int ix=comm->userRedOpCapacity; ix < cap; ix++)
      ops[ix].freeNext = ix + 1;
    delete[] comm->userRedOps;
    comm->userRedOps = ops;
    comm->userRedOpCapacity = cap;
  }
  // pop from free list
/* 中文逐项-变量 V1072：ix
 * ix（int；函数 ncclRedOpCreatePreMulSum 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ix = comm->userRedOpFreeHead；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ix = comm->userRedOpFreeHead;
/* 中文逐项-变量 V1073：user
 * user（ncclUserRedOp；函数 ncclRedOpCreatePreMulSum 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 user = &comm->userRedOps[ix]；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  ncclUserRedOp *user = &comm->userRedOps[ix];
  comm->userRedOpFreeHead = user->freeNext;

  user->freeNext = -1; // allocated
  user->datatype = datatype;
  user->opFull.op = ncclDevPreMulSum;
  if (residence == ncclScalarHostImmediate) {
    user->opFull.scalarArgIsPtr = false;
    std::memcpy(&user->opFull.scalarArg, scalar, ncclTypeSize(datatype));
  } else {
    user->opFull.scalarArgIsPtr = true;
    user->opFull.scalarArg = reinterpret_cast<uint64_t>(scalar);
  }
  *op = ncclRedOp_t(int(ncclNumOps) + ix);
  *op = ncclUserRedOpMangle(comm, *op);
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclRedOpDestroy, ncclRedOp_t op, ncclComm_t comm);
/* 中文逐项-函数 F0428：ncclRedOpDestroy
 * 用途与位置：验证并归还用户归约句柄槽，不会销毁已在入队时复制到任务中的操作状态。
 * 参数 op（ncclRedOp_t op）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：int；ncclUserRedOpMangle。
 * 需要对照的边界/推进条件：0 <= int(op) && int(op) < int(ncclNumOps；int(op) < 0 || int(ncclMaxRedOp) <
 * int(op；comm->userRedOpCapacity <= ix || comm->userRedOps[ix].freeNext != -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclRedOpDestroy(ncclRedOp_t op, ncclComm_t comm) {
  if (0 <= int(op) && int(op) < int(ncclNumOps)) {
    WARN("ncclRedOpDestroy : operator is a NCCL builtin.");
    return ncclInvalidArgument;
  }
  if (int(op) < 0 || int(ncclMaxRedOp) < int(op)) {
    WARN("ncclRedOpDestroy :  operator is garbage.");
    return ncclInvalidArgument;
  }
/* 中文逐项-变量 V1074：ix
 * ix（int；函数 ncclRedOpDestroy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ix = int(ncclUserRedOpMangle(comm, op)) - int(ncclNumOps)；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
  int ix = int(ncclUserRedOpMangle(comm, op)) - int(ncclNumOps);
  if (comm->userRedOpCapacity <= ix || comm->userRedOps[ix].freeNext != -1) {
    WARN("ncclRedOpDestroy : operator unknown to this communicator.");
    return ncclInvalidArgument;
  }
  // push to free list
  comm->userRedOps[ix].freeNext = comm->userRedOpFreeHead;
  comm->userRedOpFreeHead = ix;
  return ncclSuccess;
}
