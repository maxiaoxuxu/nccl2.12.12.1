/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#ifndef NVTX_IMPL_GUARD_CUDART
#error Never include this file directly -- it is automatically included by nvToolsExtCudaRt.h (except when NVTX_NO_IMPL is defined).
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (NVTX_API * nvtxNameCudaDeviceA_impl_fntype)(int device, const char* name);
typedef void (NVTX_API * nvtxNameCudaDeviceW_impl_fntype)(int device, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCudaStreamA_impl_fntype)(cudaStream_t stream, const char* name);
typedef void (NVTX_API * nvtxNameCudaStreamW_impl_fntype)(cudaStream_t stream, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCudaEventA_impl_fntype)(cudaEvent_t event, const char* name);
typedef void (NVTX_API * nvtxNameCudaEventW_impl_fntype)(cudaEvent_t event, const wchar_t* name);

NVTX_DECLSPEC /* 中文逐项-函数 F0708：nvtxNameCudaDeviceA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 device（int device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCudaDeviceA(int device, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2930：local
 * local（nvtxNameCudaDeviceA_impl_fntype；函数 nvtxNameCudaDeviceA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCudaDeviceA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCudaDeviceA_impl_fntype local = (nvtxNameCudaDeviceA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr;
    if(local!=0)
        (*local)(device, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0709：nvtxNameCudaDeviceW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 device（int device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCudaDeviceW(int device, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2931：local
 * local（nvtxNameCudaDeviceW_impl_fntype；函数 nvtxNameCudaDeviceW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCudaDeviceW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCudaDeviceW_impl_fntype local = (nvtxNameCudaDeviceW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr;
    if(local!=0)
        (*local)(device, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0710：nvtxNameCudaStreamA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCudaStreamA(cudaStream_t stream, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2932：local
 * local（nvtxNameCudaStreamA_impl_fntype；函数 nvtxNameCudaStreamA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCudaStreamA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCudaStreamA_impl_fntype local = (nvtxNameCudaStreamA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr;
    if(local!=0)
        (*local)(stream, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0711：nvtxNameCudaStreamW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCudaStreamW(cudaStream_t stream, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2933：local
 * local（nvtxNameCudaStreamW_impl_fntype；函数 nvtxNameCudaStreamW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCudaStreamW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCudaStreamW_impl_fntype local = (nvtxNameCudaStreamW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr;
    if(local!=0)
        (*local)(stream, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0712：nvtxNameCudaEventA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 event（cudaEvent_t event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCudaEventA(cudaEvent_t event, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2934：local
 * local（nvtxNameCudaEventA_impl_fntype；函数 nvtxNameCudaEventA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCudaEventA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCudaEventA_impl_fntype local = (nvtxNameCudaEventA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr;
    if(local!=0)
        (*local)(event, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0713：nvtxNameCudaEventW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 event（cudaEvent_t event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCudaEventW(cudaEvent_t event, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2935：local
 * local（nvtxNameCudaEventW_impl_fntype；函数 nvtxNameCudaEventW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCudaEventW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCudaEventW_impl_fntype local = (nvtxNameCudaEventW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr;
    if(local!=0)
        (*local)(event, name);
#endif /*NVTX_DISABLE*/
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

