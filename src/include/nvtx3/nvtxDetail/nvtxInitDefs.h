/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#ifndef NVTX_IMPL_GUARD
#error Never include this file directly -- it is automatically included by nvToolsExt.h (except when NVTX_NO_IMPL is defined).
#endif

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0744：nvtxMarkEx_impl_init
 * 用途与位置：向性能工具记录一个瞬时标记；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxMarkEx。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxMarkEx_impl_init)(const nvtxEventAttributes_t* eventAttrib){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxMarkEx(eventAttrib);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0745：nvtxMarkA_impl_init
 * 用途与位置：向性能工具记录一个瞬时标记；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const char* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxMarkA。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxMarkA_impl_init)(const char* message){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxMarkA(message);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0746：nvtxMarkW_impl_init
 * 用途与位置：向性能工具记录一个瞬时标记；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const wchar_t* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxMarkW。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxMarkW_impl_init)(const wchar_t* message){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxMarkW(message);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0747：nvtxRangeStartEx_impl_init
 * 用途与位置：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxRangeStartEx。
 * 源码返回表达式：return nvtxRangeStartEx(eventAttrib);。CHECK类宏还可能提前返回/跳转。
 */nvtxRangeId_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartEx_impl_init)(const nvtxEventAttributes_t* eventAttrib){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangeStartEx(eventAttrib);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0748：nvtxRangeStartA_impl_init
 * 用途与位置：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 message（const char* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxRangeStartA。
 * 源码返回表达式：return nvtxRangeStartA(message);。CHECK类宏还可能提前返回/跳转。
 */nvtxRangeId_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartA_impl_init)(const char* message){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangeStartA(message);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0749：nvtxRangeStartW_impl_init
 * 用途与位置：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 message（const wchar_t* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxRangeStartW。
 * 源码返回表达式：return nvtxRangeStartW(message);。CHECK类宏还可能提前返回/跳转。
 */nvtxRangeId_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartW_impl_init)(const wchar_t* message){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangeStartW(message);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0750：nvtxRangeEnd_impl_init
 * 用途与位置：按id结束显式NVTX时间范围；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 id（nvtxRangeId_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxRangeEnd。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxRangeEnd_impl_init)(nvtxRangeId_t id){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxRangeEnd(id);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0751：nvtxRangePushEx_impl_init
 * 用途与位置：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxRangePushEx。
 * 源码返回表达式：return nvtxRangePushEx(eventAttrib);。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxRangePushEx_impl_init)(const nvtxEventAttributes_t* eventAttrib){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangePushEx(eventAttrib);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0752：nvtxRangePushA_impl_init
 * 用途与位置：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const char* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxRangePushA。
 * 源码返回表达式：return nvtxRangePushA(message);。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxRangePushA_impl_init)(const char* message){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangePushA(message);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0753：nvtxRangePushW_impl_init
 * 用途与位置：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const wchar_t* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxRangePushW。
 * 源码返回表达式：return nvtxRangePushW(message);。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxRangePushW_impl_init)(const wchar_t* message){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangePushW(message);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0754：nvtxRangePop_impl_init
 * 用途与位置：从当前线程NVTX范围栈pop结束最近区间；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxRangePop。
 * 源码返回表达式：return nvtxRangePop();。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxRangePop_impl_init)(void){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangePop();
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0755：nvtxNameCategoryA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 category（uint32_t category）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxNameCategoryA。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryA_impl_init)(uint32_t category, const char* name){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxNameCategoryA(category, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0756：nvtxNameCategoryW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 category（uint32_t category）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxNameCategoryW。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryW_impl_init)(uint32_t category, const wchar_t* name){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxNameCategoryW(category, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0757：nvtxNameOsThreadA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 threadId（uint32_t threadId）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxNameOsThreadA。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadA_impl_init)(uint32_t threadId, const char* name){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxNameOsThreadA(threadId, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0758：nvtxNameOsThreadW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 threadId（uint32_t threadId）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxNameOsThreadW。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadW_impl_init)(uint32_t threadId, const wchar_t* name){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxNameOsThreadW(threadId, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0759：nvtxDomainMarkEx_impl_init
 * 用途与位置：向性能工具记录一个瞬时标记；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainMarkEx。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainMarkEx_impl_init)(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainMarkEx(domain, eventAttrib);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0760：nvtxDomainRangeStartEx_impl_init
 * 用途与位置：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainRangeStartEx。
 * 源码返回表达式：return nvtxDomainRangeStartEx(domain, eventAttrib);。CHECK类宏还可能提前返回/跳转。
 */nvtxRangeId_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeStartEx_impl_init)(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRangeStartEx(domain, eventAttrib);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0761：nvtxDomainRangeEnd_impl_init
 * 用途与位置：按id结束显式NVTX时间范围；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 id（nvtxRangeId_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainRangeEnd。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeEnd_impl_init)(nvtxDomainHandle_t domain, nvtxRangeId_t id){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainRangeEnd(domain, id);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0762：nvtxDomainRangePushEx_impl_init
 * 用途与位置：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainRangePushEx。
 * 源码返回表达式：return nvtxDomainRangePushEx(domain, eventAttrib);。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePushEx_impl_init)(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRangePushEx(domain, eventAttrib);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0763：nvtxDomainRangePop_impl_init
 * 用途与位置：从当前线程NVTX范围栈pop结束最近区间；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainRangePop。
 * 源码返回表达式：return nvtxDomainRangePop(domain);。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePop_impl_init)(nvtxDomainHandle_t domain){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRangePop(domain);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0764：nvtxDomainResourceCreate_impl_init
 * 用途与位置：在NVTX域中登记一个可被工具识别的资源属性对象；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 attribs（nvtxResourceAttributes_t* attribs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainResourceCreate。
 * 源码返回表达式：return nvtxDomainResourceCreate(domain, attribs);。CHECK类宏还可能提前返回/跳转。
 */nvtxResourceHandle_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceCreate_impl_init)(nvtxDomainHandle_t domain, nvtxResourceAttributes_t* attribs){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainResourceCreate(domain, attribs);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0765：nvtxDomainResourceDestroy_impl_init
 * 用途与位置：结束域内资源标记的生命周期；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 resource（nvtxResourceHandle_t resource）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainResourceDestroy。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceDestroy_impl_init)(nvtxResourceHandle_t resource){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainResourceDestroy(resource);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0766：nvtxDomainNameCategoryA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 category（uint32_t category）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainNameCategoryA。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryA_impl_init)(nvtxDomainHandle_t domain, uint32_t category, const char* name){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainNameCategoryA(domain, category, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0767：nvtxDomainNameCategoryW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 category（uint32_t category）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainNameCategoryW。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryW_impl_init)(nvtxDomainHandle_t domain, uint32_t category, const wchar_t* name){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainNameCategoryW(domain, category, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0768：nvtxDomainRegisterStringA_impl_init
 * 用途与位置：注册域内可复用字符串并返回句柄，降低重复消息处理开销；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 string（const char* string）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainRegisterStringA。
 * 源码返回表达式：return nvtxDomainRegisterStringA(domain, string);。CHECK类宏还可能提前返回/跳转。
 */nvtxStringHandle_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringA_impl_init)(nvtxDomainHandle_t domain, const char* string){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRegisterStringA(domain, string);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0769：nvtxDomainRegisterStringW_impl_init
 * 用途与位置：注册域内可复用字符串并返回句柄，降低重复消息处理开销；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 string（const wchar_t* string）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainRegisterStringW。
 * 源码返回表达式：return nvtxDomainRegisterStringW(domain, string);。CHECK类宏还可能提前返回/跳转。
 */nvtxStringHandle_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringW_impl_init)(nvtxDomainHandle_t domain, const wchar_t* string){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRegisterStringW(domain, string);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0770：nvtxDomainCreateA_impl_init
 * 用途与位置：创建带名字的NVTX标记域，隔离不同组件的事件/类别；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const char* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainCreateA。
 * 源码返回表达式：return nvtxDomainCreateA(message);。CHECK类宏还可能提前返回/跳转。
 */nvtxDomainHandle_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateA_impl_init)(const char* message){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainCreateA(message);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0771：nvtxDomainCreateW_impl_init
 * 用途与位置：创建带名字的NVTX标记域，隔离不同组件的事件/类别；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const wchar_t* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainCreateW。
 * 源码返回表达式：return nvtxDomainCreateW(message);。CHECK类宏还可能提前返回/跳转。
 */nvtxDomainHandle_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateW_impl_init)(const wchar_t* message){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainCreateW(message);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0772：nvtxDomainDestroy_impl_init
 * 用途与位置：释放NVTX域句柄及工具相关引用；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxDomainDestroy。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainDestroy_impl_init)(nvtxDomainHandle_t domain){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainDestroy(domain);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0773：nvtxInitialize_impl_init
 * 用途与位置：触发或协商NVTX性能工具接口的一次性初始化；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 reserved（const void* reserved）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：nvtxInitialize。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxInitialize_impl_init)(const void* reserved){
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxInitialize(reserved);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0774：nvtxNameCuDeviceA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 device（nvtx_CUdevice device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceA_impl_init)(nvtx_CUdevice device, const char* name){
/* 中文逐项-变量 V2985：local
 * local（nvtxNameCuDeviceA_fakeimpl_fntype；函数 nvtxNameCuDeviceA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCuDeviceA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr;
    if (local)
        local(device, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0775：nvtxNameCuDeviceW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 device（nvtx_CUdevice device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceW_impl_init)(nvtx_CUdevice device, const wchar_t* name){
/* 中文逐项-变量 V2986：local
 * local（nvtxNameCuDeviceW_fakeimpl_fntype；函数 nvtxNameCuDeviceW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCuDeviceW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr;
    if (local)
        local(device, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0776：nvtxNameCuContextA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 context（nvtx_CUcontext context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextA_impl_init)(nvtx_CUcontext context, const char* name){
/* 中文逐项-变量 V2987：local
 * local（nvtxNameCuContextA_fakeimpl_fntype；函数 nvtxNameCuContextA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCuContextA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr;
    if (local)
        local(context, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0777：nvtxNameCuContextW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 context（nvtx_CUcontext context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextW_impl_init)(nvtx_CUcontext context, const wchar_t* name){
/* 中文逐项-变量 V2988：local
 * local（nvtxNameCuContextW_fakeimpl_fntype；函数 nvtxNameCuContextW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCuContextW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr;
    if (local)
        local(context, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0778：nvtxNameCuStreamA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 stream（nvtx_CUstream stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamA_impl_init)(nvtx_CUstream stream, const char* name){
/* 中文逐项-变量 V2989：local
 * local（nvtxNameCuStreamA_fakeimpl_fntype；函数 nvtxNameCuStreamA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCuStreamA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr;
    if (local)
        local(stream, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0779：nvtxNameCuStreamW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 stream（nvtx_CUstream stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamW_impl_init)(nvtx_CUstream stream, const wchar_t* name){
/* 中文逐项-变量 V2990：local
 * local（nvtxNameCuStreamW_fakeimpl_fntype；函数 nvtxNameCuStreamW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCuStreamW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr;
    if (local)
        local(stream, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0780：nvtxNameCuEventA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 event（nvtx_CUevent event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventA_impl_init)(nvtx_CUevent event, const char* name){
/* 中文逐项-变量 V2991：local
 * local（nvtxNameCuEventA_fakeimpl_fntype；函数 nvtxNameCuEventA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCuEventA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr;
    if (local)
        local(event, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0781：nvtxNameCuEventW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 event（nvtx_CUevent event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventW_impl_init)(nvtx_CUevent event, const wchar_t* name){
/* 中文逐项-变量 V2992：local
 * local（nvtxNameCuEventW_fakeimpl_fntype；函数 nvtxNameCuEventW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCuEventW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr;
    if (local)
        local(event, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0782：nvtxNameCudaDeviceA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 device（int device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceA_impl_init)(int device, const char* name){
/* 中文逐项-变量 V2993：local
 * local（nvtxNameCudaDeviceA_impl_fntype；函数 nvtxNameCudaDeviceA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCudaDeviceA_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr;
    if (local)
        local(device, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0783：nvtxNameCudaDeviceW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 device（int device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceW_impl_init)(int device, const wchar_t* name){
/* 中文逐项-变量 V2994：local
 * local（nvtxNameCudaDeviceW_impl_fntype；函数 nvtxNameCudaDeviceW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCudaDeviceW_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr;
    if (local)
        local(device, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0784：nvtxNameCudaStreamA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 stream（nvtx_cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamA_impl_init)(nvtx_cudaStream_t stream, const char* name){
/* 中文逐项-变量 V2995：local
 * local（nvtxNameCudaStreamA_fakeimpl_fntype；函数 nvtxNameCudaStreamA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCudaStreamA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr;
    if (local)
        local(stream, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0785：nvtxNameCudaStreamW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 stream（nvtx_cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamW_impl_init)(nvtx_cudaStream_t stream, const wchar_t* name){
/* 中文逐项-变量 V2996：local
 * local（nvtxNameCudaStreamW_fakeimpl_fntype；函数 nvtxNameCudaStreamW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCudaStreamW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr;
    if (local)
        local(stream, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0786：nvtxNameCudaEventA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 event（nvtx_cudaEvent_t event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventA_impl_init)(nvtx_cudaEvent_t event, const char* name){
/* 中文逐项-变量 V2997：local
 * local（nvtxNameCudaEventA_fakeimpl_fntype；函数 nvtxNameCudaEventA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCudaEventA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr;
    if (local)
        local(event, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0787：nvtxNameCudaEventW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 event（nvtx_cudaEvent_t event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventW_impl_init)(nvtx_cudaEvent_t event, const wchar_t* name){
/* 中文逐项-变量 V2998：local
 * local（nvtxNameCudaEventW_fakeimpl_fntype；函数 nvtxNameCudaEventW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameCudaEventW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr;
    if (local)
        local(event, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0788：nvtxNameClDeviceA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 device（nvtx_cl_device_id device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceA_impl_init)(nvtx_cl_device_id device, const char* name){
/* 中文逐项-变量 V2999：local
 * local（nvtxNameClDeviceA_fakeimpl_fntype；函数 nvtxNameClDeviceA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClDeviceA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr;
    if (local)
        local(device, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0789：nvtxNameClDeviceW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 device（nvtx_cl_device_id device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceW_impl_init)(nvtx_cl_device_id device, const wchar_t* name){
/* 中文逐项-变量 V3000：local
 * local（nvtxNameClDeviceW_fakeimpl_fntype；函数 nvtxNameClDeviceW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClDeviceW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr;
    if (local)
        local(device, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0790：nvtxNameClContextA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 context（nvtx_cl_context context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextA_impl_init)(nvtx_cl_context context, const char* name){
/* 中文逐项-变量 V3001：local
 * local（nvtxNameClContextA_fakeimpl_fntype；函数 nvtxNameClContextA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClContextA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr;
    if (local)
        local(context, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0791：nvtxNameClContextW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 context（nvtx_cl_context context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextW_impl_init)(nvtx_cl_context context, const wchar_t* name){
/* 中文逐项-变量 V3002：local
 * local（nvtxNameClContextW_fakeimpl_fntype；函数 nvtxNameClContextW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClContextW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr;
    if (local)
        local(context, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0792：nvtxNameClCommandQueueA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 command_queue（nvtx_cl_command_queue command_queue）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueA_impl_init)(nvtx_cl_command_queue command_queue, const char* name){
/* 中文逐项-变量 V3003：local
 * local（nvtxNameClCommandQueueA_fakeimpl_fntype；函数 nvtxNameClCommandQueueA_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClCommandQueueA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr;
    if (local)
        local(command_queue, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0793：nvtxNameClCommandQueueW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 command_queue（nvtx_cl_command_queue command_queue）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueW_impl_init)(nvtx_cl_command_queue command_queue, const wchar_t* name){
/* 中文逐项-变量 V3004：local
 * local（nvtxNameClCommandQueueW_fakeimpl_fntype；函数 nvtxNameClCommandQueueW_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClCommandQueueW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr;
    if (local)
        local(command_queue, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0794：nvtxNameClMemObjectA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 memobj（nvtx_cl_mem memobj）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectA_impl_init)(nvtx_cl_mem memobj, const char* name){
/* 中文逐项-变量 V3005：local
 * local（nvtxNameClMemObjectA_fakeimpl_fntype；函数 nvtxNameClMemObjectA_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClMemObjectA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr;
    if (local)
        local(memobj, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0795：nvtxNameClMemObjectW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 memobj（nvtx_cl_mem memobj）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectW_impl_init)(nvtx_cl_mem memobj, const wchar_t* name){
/* 中文逐项-变量 V3006：local
 * local（nvtxNameClMemObjectW_fakeimpl_fntype；函数 nvtxNameClMemObjectW_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClMemObjectW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr;
    if (local)
        local(memobj, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0796：nvtxNameClSamplerA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 sampler（nvtx_cl_sampler sampler）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerA_impl_init)(nvtx_cl_sampler sampler, const char* name){
/* 中文逐项-变量 V3007：local
 * local（nvtxNameClSamplerA_fakeimpl_fntype；函数 nvtxNameClSamplerA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClSamplerA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr;
    if (local)
        local(sampler, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0797：nvtxNameClSamplerW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 sampler（nvtx_cl_sampler sampler）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerW_impl_init)(nvtx_cl_sampler sampler, const wchar_t* name){
/* 中文逐项-变量 V3008：local
 * local（nvtxNameClSamplerW_fakeimpl_fntype；函数 nvtxNameClSamplerW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClSamplerW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr;
    if (local)
        local(sampler, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0798：nvtxNameClProgramA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 program（nvtx_cl_program program）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramA_impl_init)(nvtx_cl_program program, const char* name){
/* 中文逐项-变量 V3009：local
 * local（nvtxNameClProgramA_fakeimpl_fntype；函数 nvtxNameClProgramA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClProgramA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr;
    if (local)
        local(program, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0799：nvtxNameClProgramW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 program（nvtx_cl_program program）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramW_impl_init)(nvtx_cl_program program, const wchar_t* name){
/* 中文逐项-变量 V3010：local
 * local（nvtxNameClProgramW_fakeimpl_fntype；函数 nvtxNameClProgramW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClProgramW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr;
    if (local)
        local(program, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0800：nvtxNameClEventA_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 evnt（nvtx_cl_event evnt）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventA_impl_init)(nvtx_cl_event evnt, const char* name){
/* 中文逐项-变量 V3011：local
 * local（nvtxNameClEventA_fakeimpl_fntype；函数 nvtxNameClEventA_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClEventA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr;
    if (local)
        local(evnt, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0801：nvtxNameClEventW_impl_init
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA
 * stream同步，也不负责NCCL通信进度。
 * 参数 evnt（nvtx_cl_event evnt）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventW_impl_init)(nvtx_cl_event evnt, const wchar_t* name){
/* 中文逐项-变量 V3012：local
 * local（nvtxNameClEventW_fakeimpl_fntype；函数 nvtxNameClEventW_impl_init 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxNameClEventW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr;
    if (local)
        local(evnt, name);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0802：nvtxDomainSyncUserCreate_impl_init
 * 用途与位置：创建带属性的用户同步标记对象供性能工具追踪；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 attribs（const nvtxSyncUserAttributes_t* attribs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return local(domain, attribs);；return (nvtxSyncUser_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxSyncUser_t NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserCreate_impl_init)(nvtxDomainHandle_t domain, const nvtxSyncUserAttributes_t* attribs){
/* 中文逐项-变量 V3013：local
 * local（nvtxDomainSyncUserCreate_impl_fntype；函数 nvtxDomainSyncUserCreate_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxDomainSyncUserCreate_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr;
    if (local) {
        return local(domain, attribs);
    }
    return (nvtxSyncUser_t)0;
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0803：nvtxDomainSyncUserDestroy_impl_init
 * 用途与位置：结束用户同步标记对象的工具生命周期；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserDestroy_impl_init)(nvtxSyncUser_t handle){
/* 中文逐项-变量 V3014：local
 * local（nvtxDomainSyncUserDestroy_impl_fntype；函数 nvtxDomainSyncUserDestroy_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxDomainSyncUserDestroy_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr;
    if (local)
        local(handle);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0804：nvtxDomainSyncUserAcquireStart_impl_init
 * 用途与位置：报告用户同步对象的一次获取尝试开始；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireStart_impl_init)(nvtxSyncUser_t handle){
/* 中文逐项-变量 V3015：local
 * local（nvtxDomainSyncUserAcquireStart_impl_fntype；函数 nvtxDomainSyncUserAcquireStart_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxDomainSyncUserAcquireStart_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr;
    if (local)
        local(handle);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0805：nvtxDomainSyncUserAcquireFailed_impl_init
 * 用途与位置：报告用户同步对象获取失败；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireFailed_impl_init)(nvtxSyncUser_t handle){
/* 中文逐项-变量 V3016：local
 * local（nvtxDomainSyncUserAcquireFailed_impl_fntype；函数 nvtxDomainSyncUserAcquireFailed_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxDomainSyncUserAcquireFailed_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr;
    if (local)
        local(handle);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0806：nvtxDomainSyncUserAcquireSuccess_impl_init
 * 用途与位置：报告用户同步对象获取成功；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireSuccess_impl_init)(nvtxSyncUser_t handle){
/* 中文逐项-变量 V3017：local
 * local（nvtxDomainSyncUserAcquireSuccess_impl_fntype；函数 nvtxDomainSyncUserAcquireSuccess_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxDomainSyncUserAcquireSuccess_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr;
    if (local)
        local(handle);
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0807：nvtxDomainSyncUserReleasing_impl_init
 * 用途与位置：报告用户同步对象即将释放；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：local。
 * 需要对照的边界/推进条件：local。各条件所属分支见紧邻函数体。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserReleasing_impl_init)(nvtxSyncUser_t handle){
/* 中文逐项-变量 V3018：local
 * local（nvtxDomainSyncUserReleasing_impl_fntype；函数 nvtxDomainSyncUserReleasing_impl_init
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    nvtxDomainSyncUserReleasing_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr;
    if (local)
        local(handle);
}

NVTX_LINKONCE_FWDDECL_FUNCTION void NVTX_VERSIONED_IDENTIFIER(nvtxSetInitFunctionsToNoops)(int forceAllToNoops);
NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0808：nvtxSetInitFunctionsToNoops
 * 用途与位置：把尚未被工具实现的初始化跳板替换为空回调，避免每次标记重复初始化或调用空指针。
 * 参数 forceAllToNoops（int forceAllToNoops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr ==
 * NVTX_VERSIONED_IDENTIFIER(nvtxMarkEx_impl_init) ||
 * forceAllToNoops；NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr ==
 * NVTX_VERSIONED_IDENTIFIER(nvtxMarkA_impl_init) ||
 * forceAllToNoops；NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr ==
 * NVTX_VERSIONED_IDENTIFIER(nvtxMarkW_impl_init) ||
 * forceAllToNoops；NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr ==
 * NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartEx_impl_init) || forceAllToNoops。各条件所属分支见紧邻函数体。
 */void NVTX_VERSIONED_IDENTIFIER(nvtxSetInitFunctionsToNoops)(int forceAllToNoops)
{
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeEnd_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePop_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainMarkEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeStartEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeEnd_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePushEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePop_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceCreate_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceDestroy_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainDestroy_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxInitialize_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserCreate_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserDestroy_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireStart_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireFailed_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireSuccess_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserReleasing_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr = NULL;
}
