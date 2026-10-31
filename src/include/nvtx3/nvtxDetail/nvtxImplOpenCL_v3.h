/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#ifndef NVTX_IMPL_GUARD_OPENCL
#error Never include this file directly -- it is automatically included by nvToolsExtCuda.h (except when NVTX_NO_IMPL is defined).
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (NVTX_API * nvtxNameClDeviceA_impl_fntype)(cl_device_id device, const char* name);
typedef void (NVTX_API * nvtxNameClDeviceW_impl_fntype)(cl_device_id device, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClContextA_impl_fntype)(cl_context context, const char* name);
typedef void (NVTX_API * nvtxNameClContextW_impl_fntype)(cl_context context, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClCommandQueueA_impl_fntype)(cl_command_queue command_queue, const char* name);
typedef void (NVTX_API * nvtxNameClCommandQueueW_impl_fntype)(cl_command_queue command_queue, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClMemObjectA_impl_fntype)(cl_mem memobj, const char* name);
typedef void (NVTX_API * nvtxNameClMemObjectW_impl_fntype)(cl_mem memobj, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClSamplerA_impl_fntype)(cl_sampler sampler, const char* name);
typedef void (NVTX_API * nvtxNameClSamplerW_impl_fntype)(cl_sampler sampler, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClProgramA_impl_fntype)(cl_program program, const char* name);
typedef void (NVTX_API * nvtxNameClProgramW_impl_fntype)(cl_program program, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClEventA_impl_fntype)(cl_event evnt, const char* name);
typedef void (NVTX_API * nvtxNameClEventW_impl_fntype)(cl_event evnt, const wchar_t* name);

NVTX_DECLSPEC /* 中文逐项-函数 F0722：nvtxNameClDeviceA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 device（cl_device_id device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClDeviceA(cl_device_id device, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2944：local
 * local（nvtxNameClDeviceA_impl_fntype；函数 nvtxNameClDeviceA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClDeviceA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClDeviceA_impl_fntype local = (nvtxNameClDeviceA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr;
    if(local!=0)
        (*local)(device, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0723：nvtxNameClDeviceW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 device（cl_device_id device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClDeviceW(cl_device_id device, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2945：local
 * local（nvtxNameClDeviceW_impl_fntype；函数 nvtxNameClDeviceW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClDeviceW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClDeviceW_impl_fntype local = (nvtxNameClDeviceW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr;
    if(local!=0)
        (*local)(device, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0724：nvtxNameClContextA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 context（cl_context context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClContextA(cl_context context, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2946：local
 * local（nvtxNameClContextA_impl_fntype；函数 nvtxNameClContextA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClContextA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClContextA_impl_fntype local = (nvtxNameClContextA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr;
    if(local!=0)
        (*local)(context, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0725：nvtxNameClContextW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 context（cl_context context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClContextW(cl_context context, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2947：local
 * local（nvtxNameClContextW_impl_fntype；函数 nvtxNameClContextW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClContextW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClContextW_impl_fntype local = (nvtxNameClContextW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr;
    if(local!=0)
        (*local)(context, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0726：nvtxNameClCommandQueueA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 command_queue（cl_command_queue command_queue）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClCommandQueueA(cl_command_queue command_queue, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2948：local
 * local（nvtxNameClCommandQueueA_impl_fntype；函数 nvtxNameClCommandQueueA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClCommandQueueA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClCommandQueueA_impl_fntype local = (nvtxNameClCommandQueueA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr;
    if(local!=0)
        (*local)(command_queue, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0727：nvtxNameClCommandQueueW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 command_queue（cl_command_queue command_queue）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClCommandQueueW(cl_command_queue command_queue, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2949：local
 * local（nvtxNameClCommandQueueW_impl_fntype；函数 nvtxNameClCommandQueueW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClCommandQueueW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClCommandQueueW_impl_fntype local = (nvtxNameClCommandQueueW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr;
    if(local!=0)
        (*local)(command_queue, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0728：nvtxNameClMemObjectA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 memobj（cl_mem memobj）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClMemObjectA(cl_mem memobj, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2950：local
 * local（nvtxNameClMemObjectA_impl_fntype；函数 nvtxNameClMemObjectA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClMemObjectA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClMemObjectA_impl_fntype local = (nvtxNameClMemObjectA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr;
    if(local!=0)
        (*local)(memobj, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0729：nvtxNameClMemObjectW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 memobj（cl_mem memobj）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClMemObjectW(cl_mem memobj, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2951：local
 * local（nvtxNameClMemObjectW_impl_fntype；函数 nvtxNameClMemObjectW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClMemObjectW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClMemObjectW_impl_fntype local = (nvtxNameClMemObjectW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr;
    if(local!=0)
        (*local)(memobj, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0730：nvtxNameClSamplerA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 sampler（cl_sampler sampler）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClSamplerA(cl_sampler sampler, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2952：local
 * local（nvtxNameClSamplerA_impl_fntype；函数 nvtxNameClSamplerA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClSamplerA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClSamplerA_impl_fntype local = (nvtxNameClSamplerA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr;
    if(local!=0)
        (*local)(sampler, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0731：nvtxNameClSamplerW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 sampler（cl_sampler sampler）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClSamplerW(cl_sampler sampler, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2953：local
 * local（nvtxNameClSamplerW_impl_fntype；函数 nvtxNameClSamplerW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClSamplerW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClSamplerW_impl_fntype local = (nvtxNameClSamplerW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr;
    if(local!=0)
        (*local)(sampler, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0732：nvtxNameClProgramA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 program（cl_program program）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClProgramA(cl_program program, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2954：local
 * local（nvtxNameClProgramA_impl_fntype；函数 nvtxNameClProgramA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClProgramA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClProgramA_impl_fntype local = (nvtxNameClProgramA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr;
    if(local!=0)
        (*local)(program, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0733：nvtxNameClProgramW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 program（cl_program program）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClProgramW(cl_program program, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2955：local
 * local（nvtxNameClProgramW_impl_fntype；函数 nvtxNameClProgramW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClProgramW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClProgramW_impl_fntype local = (nvtxNameClProgramW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr;
    if(local!=0)
        (*local)(program, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0734：nvtxNameClEventA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 evnt（cl_event evnt）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClEventA(cl_event evnt, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2956：local
 * local（nvtxNameClEventA_impl_fntype；函数 nvtxNameClEventA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClEventA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClEventA_impl_fntype local = (nvtxNameClEventA_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr;
    if(local!=0)
        (*local)(evnt, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文逐项-函数 F0735：nvtxNameClEventW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 evnt（cl_event evnt）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameClEventW(cl_event evnt, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2957：local
 * local（nvtxNameClEventW_impl_fntype；函数 nvtxNameClEventW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxNameClEventW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameClEventW_impl_fntype local = (nvtxNameClEventW_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr;
    if(local!=0)
        (*local)(evnt, name);
#endif /*NVTX_DISABLE*/
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
