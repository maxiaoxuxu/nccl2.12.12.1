/*************************************************************************
 * Copyright (c) 2021, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_GDRWRAP_H_
/* 中文逐项-常量 X0268：NCCL_GDRWRAP_H_
 * 编译期定义：NCCL_GDRWRAP_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_GDRWRAP_H_

#include "nccl.h"
#include <stdint.h> // for standard [u]intX_t types
#include <stdio.h>
#include <stdlib.h>

// These can be used if the GDR library isn't thread safe
#include <pthread.h>
/* 中文逐项-变量 V2245：gdrLock
 * gdrLock（pthread_mutex_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：#define GDRLOCK() pthread_mutex_lock(&gdrLock)。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern pthread_mutex_t gdrLock;
/* 中文逐项-宏 M0089：GDRLOCK
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 GDRLOCK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define GDRLOCK() pthread_mutex_lock(&gdrLock)
/* 中文逐项-宏 M0090：GDRUNLOCK
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 GDRUNLOCK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define GDRUNLOCK() pthread_mutex_unlock(&gdrLock)
/* 中文逐项-宏 M0091：GDRLOCKCALL
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 GDRLOCKCALL；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define GDRLOCKCALL(cmd, ret) do {                      \
    GDRLOCK();                                          \
    ret = cmd;                                          \
    GDRUNLOCK();                                        \
} while(false)

/* 中文逐项-常量 X0269：e
 * 编译期定义：e = = cmd; \ if( e != 0 ) { \ WARN("GDRCOPY failure %d", e); \ return ncclSystemError; \ } \ } while(false)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define GDRCHECK(cmd) do {                              \
    int e;                                              \
    /* GDRLOCKCALL(cmd, e); */                          \
    e = cmd;                                            \
    if( e != 0 ) {                                      \
      WARN("GDRCOPY failure %d", e);                    \
      return ncclSystemError;                           \
    }                                                   \
} while(false)

// This is required as the GDR memory is mapped WC
#if !defined(__NVCC__)
#if defined(__PPC__)
/* 中文内部逻辑 F0635：wc_store_fence
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：asm volatile("sync") ;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0635
 */
/* 中文逐项-函数 F0635：wc_store_fence
 * 用途与位置：按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyProgress；src/transport/coll_net.cc::recvProxyProgress；src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
static inline void wc_store_fence(void) { asm volatile("sync") ; }
#elif defined(__x86_64__)
#include <immintrin.h>
/* 中文内部逻辑 F0636：wc_store_fence
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：_mm_sfence();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0636
 */
/* 中文逐项-函数 F0636：wc_store_fence
 * 用途与位置：按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyProgress；src/transport/coll_net.cc::recvProxyProgress；src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：_mm_sfence。
 */
static inline void wc_store_fence(void) { _mm_sfence(); }
#elif defined(__aarch64__)
#ifdef __cplusplus
#include <atomic>
/* 中文内部逻辑 F0637：wc_store_fence
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：std::atomic_thread_fence(std::memory_order_release);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0637
 */
/* 中文逐项-函数 F0637：wc_store_fence
 * 用途与位置：按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyProgress；src/transport/coll_net.cc::recvProxyProgress；src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：std::atomic_thread_fence。
 */
static inline void wc_store_fence(void) { std::atomic_thread_fence(std::memory_order_release); }
#else
#include <stdatomic.h>
/* 中文内部逻辑 F0638：wc_store_fence
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：atomic_thread_fence(memory_order_release);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0638
 */
/* 中文逐项-函数 F0638：wc_store_fence
 * 用途与位置：按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyProgress；src/transport/coll_net.cc::recvProxyProgress；src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：atomic_thread_fence。
 */
static inline void wc_store_fence(void) { atomic_thread_fence(memory_order_release); }
#endif
#endif
#endif

//#define GDR_DIRECT 1
#ifdef GDR_DIRECT
// Call the GDR API library code directly rather than via
// dlopen() wrappers
#include <gdrapi.h>

/* 中文内部逻辑 F0639：wrap_gdr_symbols
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0639
 */
/* 中文逐项-函数 F0639：wrap_gdr_symbols
 * 用途与位置：解析 libgdrapi 动态符号；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_symbols(void) { return ncclSuccess; }
/* 中文内部逻辑 F0640：wrap_gdr_open
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：gdr_t g = gdr_open();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return g;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0640
 */
/* 中文逐项-函数 F0640：wrap_gdr_open
 * 用途与位置：打开 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：gdr_open。
 * 源码返回表达式：return g;。CHECK类宏还可能提前返回/跳转。
 */
static gdr_t wrap_gdr_open(void) { /* 中文逐项-变量 V2246：g
 * g（gdr_t；函数 wrap_gdr_open 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = gdr_open()；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */gdr_t g = gdr_open(); return g; }
/* 中文内部逻辑 F0641：wrap_gdr_close
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：GDRCHECK(gdr_close(g));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0641
 */
/* 中文逐项-函数 F0641：wrap_gdr_close
 * 用途与位置：关闭 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：GDRCHECK；gdr_close。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_close(gdr_t g) { GDRCHECK(gdr_close(g)); return ncclSuccess; }
/* 中文内部逻辑 F0642：wrap_gdr_pin_buffer
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：GDRCHECK(gdr_pin_buffer(g, addr, size, p2p_token, va_space, handle));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0642
 */
/* 中文逐项-函数 F0642：wrap_gdr_pin_buffer
 * 用途与位置：固定 GPU 显存以供映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 addr（unsigned long addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 p2p_token（uint64_t p2p_token）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 va_space（uint32_t va_space）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 handle（gdr_mh_t *handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaCalloc。
 * 本体直接出现的调用/包装：GDRCHECK；gdr_pin_buffer。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_pin_buffer(gdr_t g, unsigned long addr, size_t size, uint64_t p2p_token, uint32_t va_space, gdr_mh_t *handle) {
  GDRCHECK(gdr_pin_buffer(g, addr, size, p2p_token, va_space, handle));
  return ncclSuccess;
}
/* 中文内部逻辑 F0643：wrap_gdr_unpin_buffer
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：GDRCHECK(gdr_unpin_buffer(g, handle));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0643
 */
/* 中文逐项-函数 F0643：wrap_gdr_unpin_buffer
 * 用途与位置：解除 GPU 显存固定；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaFree。
 * 本体直接出现的调用/包装：GDRCHECK；gdr_unpin_buffer。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_unpin_buffer(gdr_t g, gdr_mh_t handle) {
  GDRCHECK(gdr_unpin_buffer(g, handle));
  return ncclSuccess;
}
/* 中文内部逻辑 F0644：wrap_gdr_get_info
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：GDRCHECK(gdr_get_info(g, handle, info));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0644
 */
/* 中文逐项-函数 F0644：wrap_gdr_get_info
 * 用途与位置：读取 GPU 显存映射属性及页偏移；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 info（gdr_info_t *info）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaCalloc。
 * 本体直接出现的调用/包装：GDRCHECK；gdr_get_info。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_get_info(gdr_t g, gdr_mh_t handle, gdr_info_t *info) {
  GDRCHECK(gdr_get_info(g, handle, info));
  return ncclSuccess;
}
/* 中文内部逻辑 F0645：wrap_gdr_map
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：GDRCHECK(gdr_map(gdr_t g, gdr_mh_t handle, void **va, size_t size));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0645
 */
/* 中文逐项-函数 F0645：wrap_gdr_map
 * 用途与位置：把已固定 GPU 内存映射到 CPU 地址空间；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 va（void **va）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaCalloc。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_map(gdr_t g, gdr_mh_t handle, void **va, size_t size) {
/* 中文逐项-变量 V2247：gdr_map
 * gdr_map（GDRCHECK；函数 wrap_gdr_map 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  GDRCHECK(gdr_map(gdr_t g, gdr_mh_t handle, void **va, size_t size));
  return ncclSuccess;
}
/* 中文内部逻辑 F0646：wrap_gdr_unmap
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：GDRCHECK(gdr_unmap(gdr_t g, gdr_mh_t handle, void **va, size_t size));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0646
 */
/* 中文逐项-函数 F0646：wrap_gdr_unmap
 * 用途与位置：解除 CPU 映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 va（void *va）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaFree。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_unmap(gdr_t g, gdr_mh_t handle, void *va, size_t size) {
/* 中文逐项-变量 V2248：gdr_unmap
 * gdr_unmap（GDRCHECK；函数 wrap_gdr_unmap 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  GDRCHECK(gdr_unmap(gdr_t g, gdr_mh_t handle, void **va, size_t size));
  return ncclSuccess;
}
/* 中文内部逻辑 F0647：wrap_gdr_runtime_get_version
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：gdr_runtime_get_version(major, minor);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0647
 */
/* 中文逐项-函数 F0647：wrap_gdr_runtime_get_version
 * 用途与位置：查询 GDRCopy 用户库版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 major（int *major）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 minor（int *minor）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：gdr_runtime_get_version。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static void wrap_gdr_runtime_get_version(int *major, int *minor) {
  gdr_runtime_get_version(major, minor);
  return ncclSuccess;
}
/* 中文内部逻辑 F0648：wrap_gdr_driver_get_version
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：gdr_driver_get_version(g, major, minor);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0648
 */
/* 中文逐项-函数 F0648：wrap_gdr_driver_get_version
 * 用途与位置：查询 GDRCopy 驱动版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 major（int *major）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 minor（int *minor）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：gdr_driver_get_version。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static void wrap_gdr_driver_get_version(gdr_t g, int *major, int *minor) {
  gdr_driver_get_version(g, major, minor);
  return ncclSuccess;
}
/* 中文内部逻辑 F0649：wrap_gdr_copy_to_mapping
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：GDRCHECK(gdr_copy_to_mapping(handle, map_d_ptr, h_ptr, size));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0649
 */
/* 中文逐项-函数 F0649：wrap_gdr_copy_to_mapping
 * 用途与位置：向 CPU 可见的 GPU 映射复制数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 map_d_ptr（void *map_d_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 h_ptr（const void *h_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaCopy。
 * 本体直接出现的调用/包装：GDRCHECK；gdr_copy_to_mapping。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_copy_to_mapping(gdr_mh_t handle, void *map_d_ptr, const void *h_ptr, size_t size) {
  GDRCHECK(gdr_copy_to_mapping(handle, map_d_ptr, h_ptr, size));
  return ncclSuccess;
}
/* 中文内部逻辑 F0650：wrap_gdr_copy_from_mapping
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：GDRCHECK(gdr_copy_from_mapping(handle, h_ptr, map_d_ptr, size));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0650
 */
/* 中文逐项-函数 F0650：wrap_gdr_copy_from_mapping
 * 用途与位置：从 CPU 可见的 GPU 映射读回数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 h_ptr（void *h_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 map_d_ptr（const void *map_d_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GDRCHECK；gdr_copy_from_mapping。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t wrap_gdr_copy_from_mapping(gdr_mh_t handle, void *h_ptr, const void *map_d_ptr, size_t size) {
  GDRCHECK(gdr_copy_from_mapping(handle, h_ptr, map_d_ptr, size));
  return ncclSuccess;
}

#else
// Dynamically handle dependency the GDR API library

/* Extracted from gdrapi.h (v2.1 Nov 2020) */

/* 中文逐项-常量 X0270：GPU_PAGE_SHIFT
 * 编译期定义：GPU_PAGE_SHIFT = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define GPU_PAGE_SHIFT   16
/* 中文逐项-常量 X0271：GPU_PAGE_SIZE
 * 编译期定义：GPU_PAGE_SIZE = (1UL << GPU_PAGE_SHIFT)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define GPU_PAGE_SIZE    (1UL << GPU_PAGE_SHIFT)
/* 中文逐项-常量 X0272：GPU_PAGE_OFFSET
 * 编译期定义：GPU_PAGE_OFFSET = (GPU_PAGE_SIZE-1)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define GPU_PAGE_OFFSET  (GPU_PAGE_SIZE-1)
/* 中文逐项-常量 X0273：GPU_PAGE_MASK
 * 编译期定义：GPU_PAGE_MASK = (~GPU_PAGE_OFFSET)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define GPU_PAGE_MASK    (~GPU_PAGE_OFFSET)

struct gdr;
typedef struct gdr *gdr_t;

typedef /* 中文逐项-类型 T0204：gdr_mh_s
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：h。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct gdr_mh_s {
/* 中文逐项-变量 V2249：h
 * h（unsigned long；成员 gdr_mh_s）：GDRCopy固定缓冲句柄的内部表示，必须用匹配的unpin接口回收。
 */
  unsigned long h;
} gdr_mh_t;

/* 中文逐项-类型 T0205：gdr_info
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：va、mapped_size、page_size、tm_cycles、cycles_per_ms、mapped、wc_mapping。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct gdr_info {
/* 中文逐项-变量 V2250：va
 * va（uint64_t；成员 gdr_info）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 */
    uint64_t va;
/* 中文逐项-变量 V2251：mapped_size
 * mapped_size（uint64_t；成员 gdr_info）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 */
    uint64_t mapped_size;
/* 中文逐项-变量 V2252：page_size
 * page_size（uint32_t；成员 gdr_info）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 */
    uint32_t page_size;
/* 中文逐项-变量 V2253：tm_cycles
 * tm_cycles（uint64_t；成员 gdr_info）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 */
    uint64_t tm_cycles;
/* 中文逐项-变量 V2254：cycles_per_ms
 * cycles_per_ms（uint32_t；成员 gdr_info）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 */
    uint32_t cycles_per_ms;
/* 中文逐项-变量 V2255：mapped
 * mapped（unsigned；成员 gdr_info）：GDRCopy映射是否存在以及是否采用write-combining属性的标志。
 */
    unsigned mapped:1;
/* 中文逐项-变量 V2256：wc_mapping
 * wc_mapping（unsigned；成员 gdr_info）：GDRCopy映射是否存在以及是否采用write-combining属性的标志。
 */
    unsigned wc_mapping:1;
};
typedef struct gdr_info gdr_info_t;

/* End of gdrapi.h */

/* 中文逐项-接口 X0274：wrap_gdr_symbols
 * 接口职责：解析 libgdrapi 动态符号；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_symbols(void);
 */
ncclResult_t wrap_gdr_symbols(void);

/* 中文逐项-接口 X0275：wrap_gdr_open
 * 接口职责：打开 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：gdr_t wrap_gdr_open(void);
 */
gdr_t wrap_gdr_open(void);
/* 中文逐项-接口 X0276：wrap_gdr_close
 * 接口职责：关闭 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_close(gdr_t g);
 */
ncclResult_t wrap_gdr_close(gdr_t g);
/* 中文逐项-接口 X0277：wrap_gdr_pin_buffer
 * 接口职责：固定 GPU 显存以供映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_pin_buffer(gdr_t g, unsigned long addr, size_t size, uint64_t
 * p2p_token, uint32_t va_space, gdr_mh_t *handle);
 */
ncclResult_t wrap_gdr_pin_buffer(gdr_t g, unsigned long addr, size_t size, uint64_t p2p_token, uint32_t va_space, gdr_mh_t *handle);
/* 中文逐项-接口 X0278：wrap_gdr_unpin_buffer
 * 接口职责：解除 GPU 显存固定；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_unpin_buffer(gdr_t g, gdr_mh_t handle);
 */
ncclResult_t wrap_gdr_unpin_buffer(gdr_t g, gdr_mh_t handle);
/* 中文逐项-接口 X0279：wrap_gdr_get_info
 * 接口职责：读取 GPU 显存映射属性及页偏移；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_get_info(gdr_t g, gdr_mh_t handle, gdr_info_t *info);
 */
ncclResult_t wrap_gdr_get_info(gdr_t g, gdr_mh_t handle, gdr_info_t *info);
/* 中文逐项-接口 X0280：wrap_gdr_map
 * 接口职责：把已固定 GPU 内存映射到 CPU 地址空间；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_map(gdr_t g, gdr_mh_t handle, void **va, size_t size);
 */
ncclResult_t wrap_gdr_map(gdr_t g, gdr_mh_t handle, void **va, size_t size);
/* 中文逐项-接口 X0281：wrap_gdr_unmap
 * 接口职责：解除 CPU 映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_unmap(gdr_t g, gdr_mh_t handle, void *va, size_t size);
 */
ncclResult_t wrap_gdr_unmap(gdr_t g, gdr_mh_t handle, void *va, size_t size);
/* 中文逐项-接口 X0282：wrap_gdr_runtime_get_version
 * 接口职责：查询 GDRCopy 用户库版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_runtime_get_version(int *major, int *minor);
 */
ncclResult_t wrap_gdr_runtime_get_version(int *major, int *minor);
/* 中文逐项-接口 X0283：wrap_gdr_driver_get_version
 * 接口职责：查询 GDRCopy 驱动版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_driver_get_version(gdr_t g, int *major, int *minor);
 */
ncclResult_t wrap_gdr_driver_get_version(gdr_t g, int *major, int *minor);
/* 中文逐项-接口 X0284：wrap_gdr_copy_to_mapping
 * 接口职责：向 CPU 可见的 GPU 映射复制数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_copy_to_mapping(gdr_mh_t handle, void *map_d_ptr, const void
 * *h_ptr, size_t size);
 */
ncclResult_t wrap_gdr_copy_to_mapping(gdr_mh_t handle, void *map_d_ptr, const void *h_ptr, size_t size);
/* 中文逐项-接口 X0285：wrap_gdr_copy_from_mapping
 * 接口职责：从 CPU 可见的 GPU 映射读回数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_gdr_copy_from_mapping(gdr_mh_t handle, void *h_ptr, const void
 * *map_d_ptr, size_t size);
 */
ncclResult_t wrap_gdr_copy_from_mapping(gdr_mh_t handle, void *h_ptr, const void *map_d_ptr, size_t size);

#endif // GDR_DIRECT

// Global GDR driver handle
/* 中文逐项-变量 V2257：ncclGdrCopy
 * ncclGdrCopy（gdr_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_gdr_pin_buffer(ncclGdrCopy, alignedAddr, mapSize, 0, 0,
 * &mh));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern gdr_t ncclGdrCopy;

#include "alloc.h"

typedef /* 中文逐项-类型 T0206：gdr_mem_desc
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：gdrDevMem、gdrMap、gdrOffset、gdrMapSize、gdrMh。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct gdr_mem_desc {
/* 中文逐项-变量 V2258：gdrDevMem
 * gdrDevMem（void；成员 gdr_mem_desc）：GDRCopy的CUDA分配、CPU映射、页内偏移与实际映射容量，释放时区分原始映射基址。
 */
  void *gdrDevMem;
/* 中文逐项-变量 V2259：gdrMap
 * gdrMap（void；成员 gdr_mem_desc）：GDRCopy的CUDA分配、CPU映射、页内偏移与实际映射容量，释放时区分原始映射基址。
 */
  void *gdrMap;
/* 中文逐项-变量 V2260：gdrOffset
 * gdrOffset（size_t；成员 gdr_mem_desc）：GDRCopy的CUDA分配、CPU映射、页内偏移与实际映射容量，释放时区分原始映射基址。
 */
  size_t gdrOffset;
/* 中文逐项-变量 V2261：gdrMapSize
 * gdrMapSize（size_t；成员 gdr_mem_desc）：GDRCopy的CUDA分配、CPU映射、页内偏移与实际映射容量，释放时区分原始映射基址。
 */
  size_t gdrMapSize;
/* 中文逐项-变量 V2262：gdrMh
 * gdrMh（gdr_mh_t；成员 gdr_mem_desc）：GDRCopy固定缓冲句柄的内部表示，必须用匹配的unpin接口回收。
 */
  gdr_mh_t gdrMh;
} gdr_mem_desc_t;

/* 中文内部逻辑 F0651：ncclGdrInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int libMajor, libMinor, drvMajor, drvMinor;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：gdr_t handle = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(wrap_gdr_symbols() == ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：handle = wrap_gdr_open();；打开 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 *     6. 条件判断：(handle != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       7. 真分支：；仅条件为真执行以下子项。
 *         8. 准备局部状态：ncclResult_t res;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         9. 调用 / 计算 / 写入：NCCLCHECKGOTO(wrap_gdr_runtime_get_version(&libMajor, &libMinor), res, error);；失败时保存错误并跳到清理标签，成功才继续；查询 GDRCopy 用户库版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 *         10. 调用 / 计算 / 写入：NCCLCHECKGOTO(wrap_gdr_driver_get_version(handle, &drvMajor, &drvMinor), res, error);；失败时保存错误并跳到清理标签，成功才继续；查询 GDRCopy 驱动版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 *         11. 条件判断：(libMajor < 2 || (libMajor == 2 && libMinor < 1) || drvMajor < 2 || (drvMajor == 2 && drvMinor < 1))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           12. 真分支：；仅条件为真执行以下子项。
 *             13. 控制转移：goto error;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *           14. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             15. 调用 / 计算 / 写入：INFO(NCCL_INIT, "GDRCOPY enabled library %d.%d driver %d.%d", libMajor, libMinor, drvMajor, drvMinor);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 16. 函数返回：return handle;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 17. 跳转标签：error；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   18. 条件判断：(handle != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     19. 真分支：；仅条件为真执行以下子项。
 *       20. 调用 / 计算 / 写入：(void) wrap_gdr_close(handle);；关闭 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 21. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0651
 */
/* 中文逐项-函数 F0651：ncclGdrInit
 * 用途与位置：动态初始化 GDRCopy 并取得映射句柄，失败时由调用者决定是否使用普通回退路径。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initGdrCopy。
 * 本体直接出现的调用/包装：NCCLCHECKGOTO；wrap_gdr_close；wrap_gdr_driver_get_version；wrap_gdr_open；wrap_gdr_runtime_get_version；wrap_gdr_symbols。
 * 需要对照的边界/推进条件：wrap_gdr_symbols() == ncclSuccess；handle != NULL；libMajor < 2 || (libMajor == 2 && libMinor < 1) ||
 * drvMajor < 2 || (drvMajor == 2 && drvMinor < 1；handle != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return handle;；return NULL;。CHECK类宏还可能提前返回/跳转。
 */
static gdr_t ncclGdrInit() {
/* 中文逐项-变量 V2263：libMajor、libMinor、drvMajor、drvMinor
 * libMajor（int；函数 ncclGdrInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_gdr_runtime_get_version(&libMajor, &libMinor), res,
 * error);。若由输出参数填入，必须先检查填充调用成功再读值。
 * libMinor（int；函数 ncclGdrInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_gdr_runtime_get_version(&libMajor, &libMinor), res,
 * error);。若由输出参数填入，必须先检查填充调用成功再读值。
 * drvMajor（int；函数 ncclGdrInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_gdr_driver_get_version(handle, &drvMajor, &drvMinor), res,
 * error);。若由输出参数填入，必须先检查填充调用成功再读值。
 * drvMinor（int；函数 ncclGdrInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_gdr_driver_get_version(handle, &drvMajor, &drvMinor), res,
 * error);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int libMajor, libMinor, drvMajor, drvMinor;
/* 中文逐项-变量 V2264：handle
 * handle（gdr_t；函数 ncclGdrInit 内）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 初值/来源 handle = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  gdr_t handle = NULL;
  // Dynamically load the GDRAPI library symbols
  if (wrap_gdr_symbols() == ncclSuccess) {
    handle = wrap_gdr_open();

    if (handle != NULL) {
/* 中文逐项-变量 V2265：res
 * res（ncclResult_t；函数 ncclGdrInit 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_gdr_runtime_get_version(&libMajor, &libMinor), res,
 * error);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ncclResult_t res;

      // Query the version of libgdrapi
      NCCLCHECKGOTO(wrap_gdr_runtime_get_version(&libMajor, &libMinor), res, error);

      // Query the version of gdrdrv driver
      NCCLCHECKGOTO(wrap_gdr_driver_get_version(handle, &drvMajor, &drvMinor), res, error);

      // Only support GDRAPI 2.1 and later
      if (libMajor < 2 || (libMajor == 2 && libMinor < 1) || drvMajor < 2 || (drvMajor == 2 && drvMinor < 1)) {
        goto error;
      }
      else
        INFO(NCCL_INIT, "GDRCOPY enabled library %d.%d driver %d.%d", libMajor, libMinor, drvMajor, drvMinor);
    }
  }
  return handle;
error:
  if (handle != NULL) (void) wrap_gdr_close(handle);
  return NULL;
}

/* 中文内部逻辑 F0652：ncclGdrCudaCalloc
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：gdr_info_t info;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：size_t mapSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：gdr_mh_t mh;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：char *devMem;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：void *gdrMap;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：mapSize = sizeof(T)*nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：ALIGN_SIZE(mapSize, GPU_PAGE_SIZE);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaCalloc(&devMem, mapSize+GPU_PAGE_SIZE-1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 9. 准备局部状态：uint64_t alignedAddr = (((uint64_t) devMem) + GPU_PAGE_OFFSET) & GPU_PAGE_MASK;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 准备局部状态：size_t align = alignedAddr - (uint64_t)devMem;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 调用 / 计算 / 写入：NCCLCHECK(wrap_gdr_pin_buffer(ncclGdrCopy, alignedAddr, mapSize, 0, 0, &mh));；失败时可能提前return，成功才继续；固定 GPU 显存以供映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(wrap_gdr_map(ncclGdrCopy, mh, &gdrMap, mapSize));；失败时可能提前return，成功才继续；把已固定 GPU 内存映射到 CPU 地址空间；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 13. 调用 / 计算 / 写入：NCCLCHECK(wrap_gdr_get_info(ncclGdrCopy, mh, &info));；失败时可能提前return，成功才继续；读取 GPU 显存映射属性及页偏移；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 14. 准备局部状态：ssize_t off = info.va - alignedAddr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 准备局部状态：gdr_mem_desc_t* md;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 16. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&md, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 17. 调用 / 计算 / 写入：md->gdrDevMem = devMem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 调用 / 计算 / 写入：md->gdrMap = gdrMap;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 调用 / 计算 / 写入：md->gdrMapSize = mapSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 调用 / 计算 / 写入：md->gdrOffset = off+align;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 21. 调用 / 计算 / 写入：md->gdrMh = mh;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 22. 调用 / 计算 / 写入：*gdrHandle = md;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 23. 调用 / 计算 / 写入：*ptr = (T *)((char *)gdrMap+off);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 24. 条件判断：(devPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   25. 真分支：；仅条件为真执行以下子项。
 *     26. 调用 / 计算 / 写入：*devPtr = (T *)(devMem+off+align);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 27. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "GDRCOPY : allocated devMem %p gdrMap %p offset %lx mh %lx mapSize %zi at %p", md->gdrDevMem, md->gdrMap, md->gdrOffset, md->gdrMh.h, md->gdrMapSize, *ptr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 28. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0652
 */
/* 中文逐项-函数 F0652：ncclGdrCudaCalloc
 * 用途与位置：分配显存并建立 GDRCopy 的 CPU 映射，返回 CUDA 地址、CPU 可写映射与释放所需描述符。
 * 参数 ptr（T** ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 devPtr（T** devPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 nelem（size_t nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 gdrHandle（void** gdrHandle）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/channel.cc::initChannel；src/transport/coll_net.cc::sendProxyConnect；src/transport/coll_net.cc::recvProxyConnect；src/transport/net.cc::sendProxyConnect；src/transport/net.cc::recvProxyConnect。
 * 本体直接出现的调用/包装：ALIGN_SIZE；NCCLCHECK；ncclCalloc；ncclCudaCalloc；wrap_gdr_get_info；wrap_gdr_map；wrap_gdr_pin_buffer。
 * 需要对照的边界/推进条件：devPtr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
static ncclResult_t ncclGdrCudaCalloc(T** ptr, T** devPtr, size_t nelem, void** gdrHandle) {
/* 中文逐项-变量 V2266：info
 * info（gdr_info_t；函数 ncclGdrCudaCalloc 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_gdr_get_info(ncclGdrCopy, mh, &info));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  gdr_info_t info;
/* 中文逐项-变量 V2267：mapSize
 * mapSize（size_t；函数 ncclGdrCudaCalloc 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 此处未显式给初值；后续首个文本使用点：mapSize = sizeof(T)*nelem;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  size_t mapSize;
/* 中文逐项-变量 V2268：mh
 * mh（gdr_mh_t；函数 ncclGdrCudaCalloc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_gdr_pin_buffer(ncclGdrCopy, alignedAddr, mapSize, 0, 0,
 * &mh));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  gdr_mh_t mh;
/* 中文逐项-变量 V2269：devMem
 * devMem（char；函数 ncclGdrCudaCalloc 内）：当前方向拥有的主机或设备分配，地址空间由字段名及类型共同确认。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCudaCalloc(&devMem, mapSize+GPU_PAGE_SIZE-1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char *devMem;
/* 中文逐项-变量 V2270：gdrMap
 * gdrMap（void；函数 ncclGdrCudaCalloc 内）：GDRCopy的CUDA分配、CPU映射、页内偏移与实际映射容量，释放时区分原始映射基址。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_gdr_map(ncclGdrCopy, mh, &gdrMap, mapSize));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  void *gdrMap;

  mapSize = sizeof(T)*nelem;

  // GDRCOPY Pinned buffer has to be a minimum of a GPU_PAGE_SIZE
  ALIGN_SIZE(mapSize, GPU_PAGE_SIZE);
  // GDRCOPY Pinned buffer has to be GPU_PAGE_SIZE aligned too
  NCCLCHECK(ncclCudaCalloc(&devMem, mapSize+GPU_PAGE_SIZE-1));
/* 中文逐项-变量 V2271：alignedAddr
 * alignedAddr（uint64_t；函数 ncclGdrCudaCalloc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 alignedAddr = (((uint64_t) devMem) + GPU_PAGE_OFFSET) & GPU_PAGE_MASK；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  uint64_t alignedAddr = (((uint64_t) devMem) + GPU_PAGE_OFFSET) & GPU_PAGE_MASK;
/* 中文逐项-变量 V2272：align
 * align（size_t；函数 ncclGdrCudaCalloc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 align = alignedAddr - (uint64_t)devMem；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  size_t align = alignedAddr - (uint64_t)devMem;

  //TRACE(NCCL_INIT, "GDRCOPY: Pin buffer 0x%lx (%p) align %zi size %zi", alignedAddr, devMem, align, mapSize);
  NCCLCHECK(wrap_gdr_pin_buffer(ncclGdrCopy, alignedAddr, mapSize, 0, 0, &mh));

  NCCLCHECK(wrap_gdr_map(ncclGdrCopy, mh, &gdrMap, mapSize));
  //TRACE(NCCL_INIT, "GDRCOPY : mapped %p (0x%lx) at %p", devMem, alignedAddr, gdrMap);

  NCCLCHECK(wrap_gdr_get_info(ncclGdrCopy, mh, &info));

  // Will offset ever be non zero ?
/* 中文逐项-变量 V2273：off
 * off（ssize_t；函数 ncclGdrCudaCalloc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 off = info.va - alignedAddr；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ssize_t off = info.va - alignedAddr;

/* 中文逐项-变量 V2274：md
 * md（gdr_mem_desc_t；函数 ncclGdrCudaCalloc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&md, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  gdr_mem_desc_t* md;
  NCCLCHECK(ncclCalloc(&md, 1));
  md->gdrDevMem = devMem;
  md->gdrMap = gdrMap;
  md->gdrMapSize = mapSize;
  md->gdrOffset = off+align;
  md->gdrMh = mh;
  *gdrHandle = md;

  *ptr = (T *)((char *)gdrMap+off);
  if (devPtr) *devPtr = (T *)(devMem+off+align);

  TRACE(NCCL_INIT, "GDRCOPY : allocated devMem %p gdrMap %p offset %lx mh %lx mapSize %zi at %p",
       md->gdrDevMem, md->gdrMap, md->gdrOffset, md->gdrMh.h, md->gdrMapSize, *ptr);

  return ncclSuccess;
}

/* 中文内部逻辑 F0653：ncclGdrCudaCopy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：gdr_mem_desc_t *md = (gdr_mem_desc_t*)gdrHandle;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(wrap_gdr_copy_to_mapping(md->gdrMh, dst, src, nelem*sizeof(T)));；失败时可能提前return，成功才继续；向 CPU 可见的 GPU 映射复制数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0653
 */
/* 中文逐项-函数 F0653：ncclGdrCudaCopy
 * 用途与位置：通过 GDRCopy 映射把指定对象写入显存，用于低延迟提交小量控制/工作数据。
 * 参数 gdrHandle（void *gdrHandle）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 dst（T* dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 src（T* src）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 nelem（size_t nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::setupLaunch。
 * 本体直接出现的调用/包装：NCCLCHECK；wrap_gdr_copy_to_mapping。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
static ncclResult_t ncclGdrCudaCopy(void *gdrHandle, T* dst, T* src, size_t nelem) {
/* 中文逐项-变量 V2275：md
 * md（gdr_mem_desc_t；函数 ncclGdrCudaCopy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 md = (gdr_mem_desc_t*)gdrHandle；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  gdr_mem_desc_t *md = (gdr_mem_desc_t*)gdrHandle;
  NCCLCHECK(wrap_gdr_copy_to_mapping(md->gdrMh, dst, src, nelem*sizeof(T)));
  return ncclSuccess;
}

/* 中文内部逻辑 F0654：ncclGdrCudaFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：gdr_mem_desc_t *md = (gdr_mem_desc_t*)gdrHandle;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(wrap_gdr_unmap(ncclGdrCopy, md->gdrMh, md->gdrMap, md->gdrMapSize));；失败时可能提前return，成功才继续；解除 CPU 映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(wrap_gdr_unpin_buffer(ncclGdrCopy, md->gdrMh));；失败时可能提前return，成功才继续；解除 GPU 显存固定；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 4. 调用 / 计算 / 写入：CUDACHECK(cudaFree(md->gdrDevMem));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 调用 / 计算 / 写入：free(md);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__gdrwrap_h.md#f0654
 */
/* 中文逐项-函数 F0654：ncclGdrCudaFree
 * 用途与位置：按映射、pin、显存分配的依赖顺序撤销 GDRCopy 资源。
 * 参数 gdrHandle（void* gdrHandle）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/channel.cc::freeChannel；src/transport/coll_net.cc::sendProxyFree；src/transport/coll_net.cc::recvProxyFree；src/transport/net.cc::sendProxyFree；src/transport/net.cc::recvProxyFree。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaFree；free；wrap_gdr_unmap；wrap_gdr_unpin_buffer。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclGdrCudaFree(void* gdrHandle) {
/* 中文逐项-变量 V2276：md
 * md（gdr_mem_desc_t；函数 ncclGdrCudaFree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 md = (gdr_mem_desc_t*)gdrHandle；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  gdr_mem_desc_t *md = (gdr_mem_desc_t*)gdrHandle;
  NCCLCHECK(wrap_gdr_unmap(ncclGdrCopy, md->gdrMh, md->gdrMap, md->gdrMapSize));
  NCCLCHECK(wrap_gdr_unpin_buffer(ncclGdrCopy, md->gdrMh));
  CUDACHECK(cudaFree(md->gdrDevMem));
  free(md);

  return ncclSuccess;
}

#endif // End include guard
