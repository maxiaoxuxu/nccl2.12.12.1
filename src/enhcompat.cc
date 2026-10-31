/*************************************************************************
 * Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

/* Define weak symbols used to allow libnccl_static.a to work with older libcudart_static.a */

/* 中文逐项-类型 T0132：cudaError_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum cudaError_t { /* 中文逐项-枚举项 X0019：cudaErrorStubLibrary
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：34
 */cudaErrorStubLibrary = 34 };

extern "C" {

/* 中文逐项-接口 X0020：cudaStreamGetCaptureInfo_v2
 * 接口职责：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：cudaError_t cudaStreamGetCaptureInfo_v2(...) __attribute__((visibility("hidden")))
 * __attribute((weak));
 */
cudaError_t cudaStreamGetCaptureInfo_v2(...)         __attribute__((visibility("hidden"))) __attribute((weak));
/* 中文逐项-函数 F0384：cudaStreamGetCaptureInfo_v2
 * 用途与位置：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclGetCudaGraph。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return cudaErrorStubLibrary;。CHECK类宏还可能提前返回/跳转。
 */
cudaError_t cudaStreamGetCaptureInfo_v2(...)         { return cudaErrorStubLibrary; }

/* 中文逐项-接口 X0021：cudaUserObjectCreate
 * 接口职责：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：cudaError_t cudaUserObjectCreate(...) __attribute__((visibility("hidden")))
 * __attribute((weak));
 */
cudaError_t cudaUserObjectCreate(...)                __attribute__((visibility("hidden"))) __attribute((weak));
/* 中文逐项-函数 F0385：cudaUserObjectCreate
 * 用途与位置：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclCudaGraphHostSetup。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return cudaErrorStubLibrary;。CHECK类宏还可能提前返回/跳转。
 */
cudaError_t cudaUserObjectCreate(...)                { return cudaErrorStubLibrary; }

/* 中文逐项-接口 X0022：cudaGraphRetainUserObject
 * 接口职责：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：cudaError_t cudaGraphRetainUserObject(...) __attribute__((visibility("hidden")))
 * __attribute((weak));
 */
cudaError_t cudaGraphRetainUserObject(...)           __attribute__((visibility("hidden"))) __attribute((weak));
/* 中文逐项-函数 F0386：cudaGraphRetainUserObject
 * 用途与位置：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclCudaGraphHostSetup。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return cudaErrorStubLibrary;。CHECK类宏还可能提前返回/跳转。
 */
cudaError_t cudaGraphRetainUserObject(...)           { return cudaErrorStubLibrary; }

/* 中文逐项-接口 X0023：cudaStreamUpdateCaptureDependencies
 * 接口职责：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：cudaError_t cudaStreamUpdateCaptureDependencies(...)
 * __attribute__((visibility("hidden"))) __attribute((weak));
 */
cudaError_t cudaStreamUpdateCaptureDependencies(...) __attribute__((visibility("hidden"))) __attribute((weak));
/* 中文逐项-函数 F0387：cudaStreamUpdateCaptureDependencies
 * 用途与位置：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclCudaGraphHostSetup。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return cudaErrorStubLibrary;。CHECK类宏还可能提前返回/跳转。
 */
cudaError_t cudaStreamUpdateCaptureDependencies(...) { return cudaErrorStubLibrary; }

/* 中文逐项-接口 X0024：cudaGetDriverEntryPoint
 * 接口职责：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：cudaError_t cudaGetDriverEntryPoint(...) __attribute__((visibility("hidden")))
 * __attribute((weak));
 */
cudaError_t cudaGetDriverEntryPoint(...)             __attribute__((visibility("hidden"))) __attribute((weak));
/* 中文逐项-函数 F0388：cudaGetDriverEntryPoint
 * 用途与位置：提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::commAlloc。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return cudaErrorStubLibrary;。CHECK类宏还可能提前返回/跳转。
 */
cudaError_t cudaGetDriverEntryPoint(...)             { return cudaErrorStubLibrary; }

}
