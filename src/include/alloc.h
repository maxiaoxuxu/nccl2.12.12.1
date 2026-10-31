/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_ALLOC_H_
/* 中文逐项-常量 X0122：NCCL_ALLOC_H_
 * 编译期定义：NCCL_ALLOC_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_ALLOC_H_

#include "nccl.h"
#include "checks.h"
#include "align.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* 中文内部逻辑 F0603：ncclCudaHostCallocDebug
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：CUDACHECK(cudaHostAlloc(ptr, nelem*sizeof(T), cudaHostAllocMapped));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 2. 调用 / 计算 / 写入：memset(*ptr, 0, nelem*sizeof(T));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 3. 调用 / 计算 / 写入：INFO(NCCL_ALLOC, "%s:%d Cuda Host Alloc Size %ld pointer %p", filefunc, line, nelem*sizeof(T), *ptr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__alloc_h.md#f0603
 */
/* 中文逐项-函数 F0603：ncclCudaHostCallocDebug
 * 用途与位置：分配 GPU 可映射的固定主机页并清零，记录文件/行号用于分配诊断。
 * 参数 ptr（T** ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 nelem（size_t nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 filefunc（const char *filefunc）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 line（int line）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；cudaHostAlloc；memset。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
/* 中文精读：三类常见内存的区别
 * ncclCalloc 是普通 CPU 堆内存；ncclCudaHostCalloc 用 cudaHostAllocMapped 分配 GPU 可访问主机内存；
 * ncclCudaCalloc 用 cudaMalloc 分配显存，并在专用 stream 上清零后等待完成。
 * 模板参数 nelem 是元素数，内部乘 sizeof(T)，看调用时不要把元素数和字节数重复相乘。
 * GPU 能访问主机内存不代表这块内存位于 SM 共享内存，更不代表访问成本等同于显存。
 */
static ncclResult_t ncclCudaHostCallocDebug(T** ptr, size_t nelem, const char *filefunc, int line) {
  CUDACHECK(cudaHostAlloc(ptr, nelem*sizeof(T), cudaHostAllocMapped));
  memset(*ptr, 0, nelem*sizeof(T));
  INFO(NCCL_ALLOC, "%s:%d Cuda Host Alloc Size %ld pointer %p", filefunc, line, nelem*sizeof(T), *ptr);
  return ncclSuccess;
}
/* 中文逐项-宏 M0051：ncclCudaHostCalloc
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 ncclCudaHostCalloc；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define ncclCudaHostCalloc(...) ncclCudaHostCallocDebug(__VA_ARGS__, __FILE__, __LINE__)

/* 中文内部逻辑 F0604：ncclCudaHostFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：CUDACHECK(cudaFreeHost(ptr));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__alloc_h.md#f0604
 */
/* 中文逐项-函数 F0604：ncclCudaHostFree
 * 用途与位置：回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/channel.cc::freeChannel；src/transport/coll_net.cc::sharedBuffersDestroy；src/transport/coll_net.cc::sendProxyFree；src/transport/coll_net.cc::recvProxyFree；src/transport/net.cc::sharedBuffersDestroy；src/transport/net.cc::sendProxyFree；src/transport/net.cc::recvProxyFree。
 * 本体直接出现的调用/包装：CUDACHECK；cudaFreeHost。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static inline ncclResult_t ncclCudaHostFree(void* ptr) {
  CUDACHECK(cudaFreeHost(ptr));
  return ncclSuccess;
}

/* 中文内部逻辑 F0605：ncclCallocDebug
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：void* p = malloc(nelem*sizeof(T));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(p == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("Failed to malloc %ld bytes", nelem*sizeof(T));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：memset(p, 0, nelem*sizeof(T));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 7. 调用 / 计算 / 写入：*ptr = (T*)p;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__alloc_h.md#f0605
 */
/* 中文逐项-函数 F0605：ncclCallocDebug
 * 用途与位置：分配并清零普通 CPU 堆对象，模板 nelem 乘 sizeof(T) 才是实际字节数。
 * 参数 ptr（T** ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 nelem（size_t nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 filefunc（const char *filefunc）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 line（int line）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：malloc；memset。
 * 需要对照的边界/推进条件：p == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
static ncclResult_t ncclCallocDebug(T** ptr, size_t nelem, const char *filefunc, int line) {
/* 中文逐项-变量 V1970：p
 * p（void；函数 ncclCallocDebug 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = malloc(nelem*sizeof(T))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  void* p = malloc(nelem*sizeof(T));
  if (p == NULL) {
    WARN("Failed to malloc %ld bytes", nelem*sizeof(T));
    return ncclSystemError;
  }
  //INFO(NCCL_ALLOC, "%s:%d malloc Size %ld pointer %p", filefunc, line, nelem*sizeof(T), p);
  memset(p, 0, nelem*sizeof(T));
  *ptr = (T*)p;
  return ncclSuccess;
}
/* 中文逐项-宏 M0052：ncclCalloc
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 ncclCalloc；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define ncclCalloc(...) ncclCallocDebug(__VA_ARGS__, __FILE__, __LINE__)

/* 中文内部逻辑 F0606：ncclRealloc
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(nelem < oldNelem)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 条件判断：(nelem == oldNelem)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 准备局部状态：T* oldp = *ptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 准备局部状态：T* p = (T*)malloc(nelem*sizeof(T));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 条件判断：(p == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 调用 / 计算 / 写入：WARN("Failed to malloc %ld bytes", nelem*sizeof(T));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     12. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 13. 调用 / 计算 / 写入：memcpy(p, oldp, oldNelem*sizeof(T));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 14. 调用 / 计算 / 写入：free(oldp);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 15. 调用 / 计算 / 写入：memset(p+oldNelem, 0, (nelem-oldNelem)*sizeof(T));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 16. 调用 / 计算 / 写入：*ptr = (T*)p;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：INFO(NCCL_ALLOC, "Mem Realloc old size %ld, new size %ld pointer %p", oldNelem*sizeof(T), nelem*sizeof(T), *ptr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__alloc_h.md#f0606
 */
/* 中文逐项-函数 F0606：ncclRealloc
 * 用途与位置：分配更大且尾部清零的 CPU 数组，复制旧元素后释放旧内存，不支持缩小。
 * 参数 ptr（T** ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 oldNelem（size_t oldNelem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 nelem（size_t nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoGetPxnRanks；src/proxy.cc::ncclProxyNewConnection；src/transport/net_ib.cc::ncclIbRegMr。
 * 本体直接出现的调用/包装：free；malloc；memcpy；memset。
 * 需要对照的边界/推进条件：nelem < oldNelem；nelem == oldNelem；p == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;；return ncclSystemError;。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
static ncclResult_t ncclRealloc(T** ptr, size_t oldNelem, size_t nelem) {
  if (nelem < oldNelem) return ncclInternalError;
  if (nelem == oldNelem) return ncclSuccess;

/* 中文逐项-变量 V1971：oldp
 * oldp（T；函数 ncclRealloc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 oldp = *ptr；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  T* oldp = *ptr;
/* 中文逐项-变量 V1972：p
 * p（T；函数 ncclRealloc 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = (T*)malloc(nelem*sizeof(T))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  T* p = (T*)malloc(nelem*sizeof(T));
  if (p == NULL) {
    WARN("Failed to malloc %ld bytes", nelem*sizeof(T));
    return ncclSystemError;
  }
  memcpy(p, oldp, oldNelem*sizeof(T));
  free(oldp);
  memset(p+oldNelem, 0, (nelem-oldNelem)*sizeof(T));
  *ptr = (T*)p;
  INFO(NCCL_ALLOC, "Mem Realloc old size %ld, new size %ld pointer %p", oldNelem*sizeof(T), nelem*sizeof(T), *ptr);
  return ncclSuccess;
}

/* 中文内部逻辑 F0607：ncclCudaCallocDebug
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：cudaStream_t stream;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：CUDACHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：CUDACHECK(cudaMalloc(ptr, nelem*sizeof(T)));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 调用 / 计算 / 写入：CUDACHECK(cudaMemsetAsync(*ptr, 0, nelem*sizeof(T), stream));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 调用 / 计算 / 写入：CUDACHECK(cudaStreamSynchronize(stream));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 6. 调用 / 计算 / 写入：CUDACHECK(cudaStreamDestroy(stream));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 7. 调用 / 计算 / 写入：INFO(NCCL_ALLOC, "%s:%d Cuda Alloc Size %ld pointer %p", filefunc, line, nelem*sizeof(T), *ptr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__alloc_h.md#f0607
 */
/* 中文逐项-函数 F0607：ncclCudaCallocDebug
 * 用途与位置：分配显存并在专用非阻塞 stream 清零后等待完成，避免默认流依赖影响预连接/图路径。
 * 参数 ptr（T** ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 nelem（size_t nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 filefunc（const char *filefunc）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 line（int line）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；cudaMalloc；cudaMemsetAsync；cudaStreamCreateWithFlags；cudaStreamDestroy；cudaStreamSynchronize。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
static ncclResult_t ncclCudaCallocDebug(T** ptr, size_t nelem, const char *filefunc, int line) {
  // Need async stream for P2P pre-connect + CUDA Graph
/* 中文逐项-变量 V1973：stream
 * stream（cudaStream_t；函数 ncclCudaCallocDebug 内）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cudaStream_t stream;
  CUDACHECK(cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking));
  CUDACHECK(cudaMalloc(ptr, nelem*sizeof(T)));
  CUDACHECK(cudaMemsetAsync(*ptr, 0, nelem*sizeof(T), stream));
  CUDACHECK(cudaStreamSynchronize(stream));
  CUDACHECK(cudaStreamDestroy(stream));
  INFO(NCCL_ALLOC, "%s:%d Cuda Alloc Size %ld pointer %p", filefunc, line, nelem*sizeof(T), *ptr);
  return ncclSuccess;
}
/* 中文逐项-宏 M0053：ncclCudaCalloc
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 ncclCudaCalloc；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define ncclCudaCalloc(...) ncclCudaCallocDebug(__VA_ARGS__, __FILE__, __LINE__)

/* 中文内部逻辑 F0608：ncclCudaMemcpy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：CUDACHECK(cudaMemcpy(dst, src, nelem*sizeof(T), cudaMemcpyDefault));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__alloc_h.md#f0608
 */
/* 中文逐项-函数 F0608：ncclCudaMemcpy
 * 用途与位置：按模板元素数量调用 cudaMemcpyDefault，实际字节数乘 sizeof(T)。
 * 参数 dst（T* dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 src（T* src）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 nelem（size_t nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::devCommSetup。
 * 本体直接出现的调用/包装：CUDACHECK；cudaMemcpy。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
static ncclResult_t ncclCudaMemcpy(T* dst, T* src, size_t nelem) {
  CUDACHECK(cudaMemcpy(dst, src, nelem*sizeof(T), cudaMemcpyDefault));
  return ncclSuccess;
}

// Allocate memory to be potentially ibv_reg_mr'd. This needs to be
// allocated on separate pages as those pages will be marked DONTFORK
// and if they are shared, that could cause a crash in a child process
/* 中文精读：为何为可能注册的缓冲单独按页分配
 * verbs 的 fork 支持会对注册页施加 DONTFORK 等处理；若与普通小对象共用页，可能影响子进程。
 * 这里按系统页尺寸对齐并独占所需页，随后具体后端才调用 reg_mr。
 * 分配内存与创建 MR 是两个步骤，拥有一块缓冲不自动消耗一个 MR 注册项。
 * 页对齐也与后端 MR 缓存键相关，但不意味着不同 PD/NIC 上的注册可任意共享。
 */
/* 中文内部逻辑 F0609：ncclIbMallocDebug
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：size_t page_size = sysconf(_SC_PAGESIZE);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：void* p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int size_aligned = ROUNDUP(size, page_size);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int ret = posix_memalign(&p, page_size, size_aligned);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(ret != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 调用 / 计算 / 写入：memset(p, 0, size);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 调用 / 计算 / 写入：*ptr = p;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：INFO(NCCL_ALLOC, "%s:%d Ib Alloc Size %ld pointer %p", filefunc, line, size, *ptr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__alloc_h.md#f0609
 */
/* 中文逐项-函数 F0609：ncclIbMallocDebug
 * 用途与位置：按系统页对齐为将来 verbs 注册准备独立页缓冲，避免 fork 页标记影响同页其他堆对象。
 * 参数 ptr（void** ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 filefunc（const char *filefunc）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 line（int line）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ROUNDUP；memset；posix_memalign；sysconf。
 * 需要对照的边界/推进条件：ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclIbMallocDebug(void** ptr, size_t size, const char *filefunc, int line) {
/* 中文逐项-变量 V1974：page_size
 * page_size（size_t；函数 ncclIbMallocDebug 内）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 * 初值/来源 page_size = sysconf(_SC_PAGESIZE)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  size_t page_size = sysconf(_SC_PAGESIZE);
/* 中文逐项-变量 V1975：p
 * p（void；函数 ncclIbMallocDebug 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：int ret = posix_memalign(&p, page_size, size_aligned);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  void* p;
/* 中文逐项-变量 V1976：size_aligned
 * size_aligned（int；函数 ncclIbMallocDebug 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 size_aligned = ROUNDUP(size, page_size)；向上按指定粒度对齐，满足缓冲/向量/页或协议推进约束。
 */
  int size_aligned = ROUNDUP(size, page_size);
/* 中文逐项-变量 V1977：ret
 * ret（int；函数 ncclIbMallocDebug 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = posix_memalign(&p, page_size, size_aligned)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int ret = posix_memalign(&p, page_size, size_aligned);
  if (ret != 0) return ncclSystemError;
  memset(p, 0, size);
  *ptr = p;
  INFO(NCCL_ALLOC, "%s:%d Ib Alloc Size %ld pointer %p", filefunc, line, size, *ptr);
  return ncclSuccess;
}
/* 中文逐项-宏 M0054：ncclIbMalloc
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 ncclIbMalloc；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define ncclIbMalloc(...) ncclIbMallocDebug(__VA_ARGS__, __FILE__, __LINE__)

#endif
