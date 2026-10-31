/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#ifndef NVTX_IMPL_GUARD_CUDA
#error Never include this file directly -- it is automatically included by nvToolsExtCuda.h (except when NVTX_NO_IMPL is defined).
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (NVTX_API * nvtxNameCuDeviceA_impl_fntype)(CUdevice device, const char* name);
typedef void (NVTX_API * nvtxNameCuDeviceW_impl_fntype)(CUdevice device, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCuContextA_impl_fntype)(CUcontext context, const char* name);
typedef void (NVTX_API * nvtxNameCuContextW_impl_fntype)(CUcontext context, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCuStreamA_impl_fntype)(CUstream stream, const char* name);
typedef void (NVTX_API * nvtxNameCuStreamW_impl_fntype)(CUstream stream, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCuEventA_impl_fntype)(CUevent event, const char* name);
typedef void (NVTX_API * nvtxNameCuEventW_impl_fntype)(CUevent event, const wchar_t* name);

NVTX_DECLSPEC /* 中文逐项-函数 F0714：nvtxNameCuDeviceA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 device（CUdevice device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCuDeviceA(CUdevice device, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2936：local
 * local（nvtxNameCuDeviceA_impl_fntype；函数 nvtxNameCuDeviceA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCuDeviceA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCuDeviceA_impl_fntype local = (nvtxNameCuDeviceA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr;
    if(local!=0)
        (*local)(device, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0715：nvtxNameCuDeviceW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 device（CUdevice device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCuDeviceW(CUdevice device, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2937：local
 * local（nvtxNameCuDeviceW_impl_fntype；函数 nvtxNameCuDeviceW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCuDeviceW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCuDeviceW_impl_fntype local = (nvtxNameCuDeviceW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr;
    if(local!=0)
        (*local)(device, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0716：nvtxNameCuContextA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 context（CUcontext context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCuContextA(CUcontext context, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2938：local
 * local（nvtxNameCuContextA_impl_fntype；函数 nvtxNameCuContextA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCuContextA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCuContextA_impl_fntype local = (nvtxNameCuContextA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr;
    if(local!=0)
        (*local)(context, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0717：nvtxNameCuContextW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 context（CUcontext context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCuContextW(CUcontext context, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2939：local
 * local（nvtxNameCuContextW_impl_fntype；函数 nvtxNameCuContextW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCuContextW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCuContextW_impl_fntype local = (nvtxNameCuContextW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr;
    if(local!=0)
        (*local)(context, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0718：nvtxNameCuStreamA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 stream（CUstream stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCuStreamA(CUstream stream, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2940：local
 * local（nvtxNameCuStreamA_impl_fntype；函数 nvtxNameCuStreamA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCuStreamA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCuStreamA_impl_fntype local = (nvtxNameCuStreamA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr;
    if(local!=0)
        (*local)(stream, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0719：nvtxNameCuStreamW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 stream（CUstream stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCuStreamW(CUstream stream, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2941：local
 * local（nvtxNameCuStreamW_impl_fntype；函数 nvtxNameCuStreamW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCuStreamW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCuStreamW_impl_fntype local = (nvtxNameCuStreamW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr;
    if(local!=0)
        (*local)(stream, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0720：nvtxNameCuEventA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 event（CUevent event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCuEventA(CUevent event, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2942：local
 * local（nvtxNameCuEventA_impl_fntype；函数 nvtxNameCuEventA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCuEventA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCuEventA_impl_fntype local = (nvtxNameCuEventA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr;
    if(local!=0)
        (*local)(event, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0721：nvtxNameCuEventW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 event（CUevent event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCuEventW(CUevent event, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2943：local
 * local（nvtxNameCuEventW_impl_fntype；函数 nvtxNameCuEventW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameCuEventW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCuEventW_impl_fntype local = (nvtxNameCuEventW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr;
    if(local!=0)
        (*local)(event, name);
#endif /*NVTX_DISABLE*/
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

