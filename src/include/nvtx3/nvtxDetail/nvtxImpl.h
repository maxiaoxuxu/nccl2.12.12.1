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

/* ---- Include required platform headers ---- */

#if defined(_WIN32) 

#include <Windows.h>

#else
#include <unistd.h>

#if defined(__ANDROID__)
#include <android/api-level.h> 
#endif

#if defined(__linux__) || defined(__CYGWIN__)
#include <sched.h>
#endif

#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <wchar.h>

#endif

/* ---- Define macros used in this file ---- */

/* 中文逐项-常量 X0884：NVTX_INIT_STATE_FRESH
 * 编译期定义：NVTX_INIT_STATE_FRESH = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_INIT_STATE_FRESH 0
/* 中文逐项-常量 X0885：NVTX_INIT_STATE_STARTED
 * 编译期定义：NVTX_INIT_STATE_STARTED = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_INIT_STATE_STARTED 1
/* 中文逐项-常量 X0886：NVTX_INIT_STATE_COMPLETE
 * 编译期定义：NVTX_INIT_STATE_COMPLETE = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_INIT_STATE_COMPLETE 2

#ifdef NVTX_DEBUG_PRINT
#ifdef __ANDROID__
#include <android/log.h>
/* 中文逐项-宏 M0099：NVTX_ERR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_ERR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_ERR(...) __android_log_print(ANDROID_LOG_ERROR, "NVTOOLSEXT", __VA_ARGS__);
/* 中文逐项-宏 M0100：NVTX_INFO
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_INFO；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_INFO(...) __android_log_print(ANDROID_LOG_INFO, "NVTOOLSEXT", __VA_ARGS__);
#else
#include <stdio.h>
/* 中文逐项-宏 M0101：NVTX_ERR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_ERR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_ERR(...) fprintf(stderr, "NVTX_ERROR: " __VA_ARGS__)
/* 中文逐项-宏 M0102：NVTX_INFO
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_INFO；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_INFO(...) fprintf(stderr, "NVTX_INFO: " __VA_ARGS__)
#endif
#else /* !defined(NVTX_DEBUG_PRINT) */
/* 中文逐项-宏 M0103：NVTX_ERR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_ERR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_ERR(...)
/* 中文逐项-宏 M0104：NVTX_INFO
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_INFO；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_INFO(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __GNUC__
#pragma GCC visibility push(hidden)
#endif

/* ---- Forward declare all functions referenced in globals ---- */

NVTX_LINKONCE_FWDDECL_FUNCTION void NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)(void);
NVTX_LINKONCE_FWDDECL_FUNCTION int NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxEtiGetModuleFunctionTable)(
    NvtxCallbackModule module,
    NvtxFunctionTable* out_table,
    unsigned int* out_size);
NVTX_LINKONCE_FWDDECL_FUNCTION void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxEtiSetInjectionNvtxVersion)(
    uint32_t version);
NVTX_LINKONCE_FWDDECL_FUNCTION /* 中文逐项-变量 V2823：NVTX_VERSIONED_IDENTIFIER
 * NVTX_VERSIONED_IDENTIFIER（void；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NVTX_LINKONCE_DEFINE_GLOBAL nvtxGlobals_t NVTX_VERSIONED_IDENTIFIER(nvtxGlobals)
 * =。若由输出参数填入，必须先检查填充调用成功再读值。
 */const void* NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxGetExportTable)(
    uint32_t exportTableId);

#include "nvtxInitDecls.h"

/* ---- Define all globals ---- */

typedef /* 中文逐项-类型 T0335：nvtxGlobals_t
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：initState、etblCallbacks、etblVersionInfo、nvtxMarkEx_impl_fnptr、nvtxMarkA_impl_fnptr、nvtxMarkW_impl_fnptr、nvtxRangeStartEx_impl_fnptr、nvtxRangeStartA_impl_fnptr、nvtxRangeStartW_impl_fnptr、nvtxRangeEnd_impl_fnptr、nvtxRangePushEx_impl_fnptr、nvtxRangePushA_impl_fnptr、nvtxRangePushW_impl_fnptr、nvtxRangePop_impl_fnptr、nvtxNameCategoryA_impl_fnptr、nvtxNameCategoryW_impl_fnptr、nvtxNameOsThreadA_impl_fnptr、nvtxNameOsThreadW_impl_fnptr、nvtxNameCuDeviceA_impl_fnptr、nvtxNameCuDeviceW_impl_fnptr、nvtxNameCuContextA_impl_fnptr、nvtxNameCuContextW_impl_fnptr、nvtxNameCuStreamA_impl_fnptr、nvtxNameCuStreamW_impl_fnptr、nvtxNameCuEventA_impl_fnptr、nvtxNameCuEventW_impl_fnptr、nvtxNameClDeviceA_impl_fnptr、nvtxNameClDeviceW_impl_fnptr、nvtxNameClContextA_impl_fnptr、nvtxNameClContextW_impl_fnptr、nvtxNameClCommandQueueA_impl_fnptr、nvtxNameClCommandQueueW_impl_fnptr、nvtxNameClMemObjectA_impl_fnptr、nvtxNameClMemObjectW_impl_fnptr、nvtxNameClSamplerA_impl_fnptr、nvtxNameClSamplerW_impl_fnptr、nvtxNameClProgramA_impl_fnptr、nvtxNameClProgramW_impl_fnptr、nvtxNameClEventA_impl_fnptr、nvtxNameClEventW_impl_fnptr、nvtxNameCudaDeviceA_impl_fnptr、nvtxNameCudaDeviceW_impl_fnptr、nvtxNameCudaStreamA_impl_fnptr、nvtxNameCudaStreamW_impl_fnptr、nvtxNameCudaEventA_impl_fnptr、nvtxNameCudaEventW_impl_fnptr、nvtxDomainMarkEx_impl_fnptr、nvtxDomainRangeStartEx_impl_fnptr、nvtxDomainRangeEnd_impl_fnptr、nvtxDomainRangePushEx_impl_fnptr、nvtxDomainRangePop_impl_fnptr、nvtxDomainResourceCreate_impl_fnptr、nvtxDomainResourceDestroy_impl_fnptr、nvtxDomainNameCategoryA_impl_fnptr、nvtxDomainNameCategoryW_impl_fnptr、nvtxDomainRegisterStringA_impl_fnptr、nvtxDomainRegisterStringW_impl_fnptr、nvtxDomainCreateA_impl_fnptr、nvtxDomainCreateW_impl_fnptr、nvtxDomainDestroy_impl_fnptr、nvtxInitialize_impl_fnptr、nvtxDomainSyncUserCreate_impl_fnptr、nvtxDomainSyncUserDestroy_impl_fnptr、nvtxDomainSyncUserAcquireStart_impl_fnptr、nvtxDomainSyncUserAcquireFailed_impl_fnptr、nvtxDomainSyncUserAcquireSuccess_impl_fnptr、nvtxDomainSyncUserReleasing_impl_fnptr、functionTable_CORE、functionTable_CUDA、functionTable_OPENCL、functionTable_CUDART、functionTable_CORE2、functionTable_SYNC。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct nvtxGlobals_t
{
/* 中文逐项-变量 V2824：initState
 * initState（unsigned int；成员 nvtxGlobals_t）：NVTX工具注入的一次性初始化状态，避免多线程重复初始化或看到未完成回调表。
 */
    volatile unsigned int initState;
/* 中文逐项-变量 V2825：etblCallbacks
 * etblCallbacks（NvtxExportTableCallbacks；成员 nvtxGlobals_t）：NVTX对注入工具暴露的回调登记/版本信息导出表。
 */
    NvtxExportTableCallbacks etblCallbacks;
/* 中文逐项-变量 V2826：etblVersionInfo
 * etblVersionInfo（NvtxExportTableVersionInfo；成员 nvtxGlobals_t）：NVTX对注入工具暴露的回调登记/版本信息导出表。
 */
    NvtxExportTableVersionInfo etblVersionInfo;

    /* Implementation function pointers */
/* 中文逐项-变量 V2827：nvtxMarkEx_impl_fnptr
 * nvtxMarkEx_impl_fnptr（nvtxMarkEx_impl_fntype；成员 nvtxGlobals_t）：该NVTX API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxMarkEx_impl_fntype nvtxMarkEx_impl_fnptr;
/* 中文逐项-变量 V2828：nvtxMarkA_impl_fnptr
 * nvtxMarkA_impl_fnptr（nvtxMarkA_impl_fntype；成员 nvtxGlobals_t）：该NVTX API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxMarkA_impl_fntype nvtxMarkA_impl_fnptr;
/* 中文逐项-变量 V2829：nvtxMarkW_impl_fnptr
 * nvtxMarkW_impl_fnptr（nvtxMarkW_impl_fntype；成员 nvtxGlobals_t）：该NVTX API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxMarkW_impl_fntype nvtxMarkW_impl_fnptr;
/* 中文逐项-变量 V2830：nvtxRangeStartEx_impl_fnptr
 * nvtxRangeStartEx_impl_fnptr（nvtxRangeStartEx_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxRangeStartEx_impl_fntype nvtxRangeStartEx_impl_fnptr;
/* 中文逐项-变量 V2831：nvtxRangeStartA_impl_fnptr
 * nvtxRangeStartA_impl_fnptr（nvtxRangeStartA_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxRangeStartA_impl_fntype nvtxRangeStartA_impl_fnptr;
/* 中文逐项-变量 V2832：nvtxRangeStartW_impl_fnptr
 * nvtxRangeStartW_impl_fnptr（nvtxRangeStartW_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxRangeStartW_impl_fntype nvtxRangeStartW_impl_fnptr;
/* 中文逐项-变量 V2833：nvtxRangeEnd_impl_fnptr
 * nvtxRangeEnd_impl_fnptr（nvtxRangeEnd_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxRangeEnd_impl_fntype nvtxRangeEnd_impl_fnptr;
/* 中文逐项-变量 V2834：nvtxRangePushEx_impl_fnptr
 * nvtxRangePushEx_impl_fnptr（nvtxRangePushEx_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxRangePushEx_impl_fntype nvtxRangePushEx_impl_fnptr;
/* 中文逐项-变量 V2835：nvtxRangePushA_impl_fnptr
 * nvtxRangePushA_impl_fnptr（nvtxRangePushA_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxRangePushA_impl_fntype nvtxRangePushA_impl_fnptr;
/* 中文逐项-变量 V2836：nvtxRangePushW_impl_fnptr
 * nvtxRangePushW_impl_fnptr（nvtxRangePushW_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxRangePushW_impl_fntype nvtxRangePushW_impl_fnptr;
/* 中文逐项-变量 V2837：nvtxRangePop_impl_fnptr
 * nvtxRangePop_impl_fnptr（nvtxRangePop_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxRangePop_impl_fntype nvtxRangePop_impl_fnptr;
/* 中文逐项-变量 V2838：nvtxNameCategoryA_impl_fnptr
 * nvtxNameCategoryA_impl_fnptr（nvtxNameCategoryA_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCategoryA_impl_fntype nvtxNameCategoryA_impl_fnptr;
/* 中文逐项-变量 V2839：nvtxNameCategoryW_impl_fnptr
 * nvtxNameCategoryW_impl_fnptr（nvtxNameCategoryW_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCategoryW_impl_fntype nvtxNameCategoryW_impl_fnptr;
/* 中文逐项-变量 V2840：nvtxNameOsThreadA_impl_fnptr
 * nvtxNameOsThreadA_impl_fnptr（nvtxNameOsThreadA_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameOsThreadA_impl_fntype nvtxNameOsThreadA_impl_fnptr;
/* 中文逐项-变量 V2841：nvtxNameOsThreadW_impl_fnptr
 * nvtxNameOsThreadW_impl_fnptr（nvtxNameOsThreadW_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameOsThreadW_impl_fntype nvtxNameOsThreadW_impl_fnptr;

/* 中文逐项-变量 V2842：nvtxNameCuDeviceA_impl_fnptr
 * nvtxNameCuDeviceA_impl_fnptr（nvtxNameCuDeviceA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCuDeviceA_fakeimpl_fntype nvtxNameCuDeviceA_impl_fnptr;
/* 中文逐项-变量 V2843：nvtxNameCuDeviceW_impl_fnptr
 * nvtxNameCuDeviceW_impl_fnptr（nvtxNameCuDeviceW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCuDeviceW_fakeimpl_fntype nvtxNameCuDeviceW_impl_fnptr;
/* 中文逐项-变量 V2844：nvtxNameCuContextA_impl_fnptr
 * nvtxNameCuContextA_impl_fnptr（nvtxNameCuContextA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCuContextA_fakeimpl_fntype nvtxNameCuContextA_impl_fnptr;
/* 中文逐项-变量 V2845：nvtxNameCuContextW_impl_fnptr
 * nvtxNameCuContextW_impl_fnptr（nvtxNameCuContextW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCuContextW_fakeimpl_fntype nvtxNameCuContextW_impl_fnptr;
/* 中文逐项-变量 V2846：nvtxNameCuStreamA_impl_fnptr
 * nvtxNameCuStreamA_impl_fnptr（nvtxNameCuStreamA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCuStreamA_fakeimpl_fntype nvtxNameCuStreamA_impl_fnptr;
/* 中文逐项-变量 V2847：nvtxNameCuStreamW_impl_fnptr
 * nvtxNameCuStreamW_impl_fnptr（nvtxNameCuStreamW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCuStreamW_fakeimpl_fntype nvtxNameCuStreamW_impl_fnptr;
/* 中文逐项-变量 V2848：nvtxNameCuEventA_impl_fnptr
 * nvtxNameCuEventA_impl_fnptr（nvtxNameCuEventA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCuEventA_fakeimpl_fntype nvtxNameCuEventA_impl_fnptr;
/* 中文逐项-变量 V2849：nvtxNameCuEventW_impl_fnptr
 * nvtxNameCuEventW_impl_fnptr（nvtxNameCuEventW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCuEventW_fakeimpl_fntype nvtxNameCuEventW_impl_fnptr;

/* 中文逐项-变量 V2850：nvtxNameClDeviceA_impl_fnptr
 * nvtxNameClDeviceA_impl_fnptr（nvtxNameClDeviceA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClDeviceA_fakeimpl_fntype nvtxNameClDeviceA_impl_fnptr;
/* 中文逐项-变量 V2851：nvtxNameClDeviceW_impl_fnptr
 * nvtxNameClDeviceW_impl_fnptr（nvtxNameClDeviceW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClDeviceW_fakeimpl_fntype nvtxNameClDeviceW_impl_fnptr;
/* 中文逐项-变量 V2852：nvtxNameClContextA_impl_fnptr
 * nvtxNameClContextA_impl_fnptr（nvtxNameClContextA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClContextA_fakeimpl_fntype nvtxNameClContextA_impl_fnptr;
/* 中文逐项-变量 V2853：nvtxNameClContextW_impl_fnptr
 * nvtxNameClContextW_impl_fnptr（nvtxNameClContextW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClContextW_fakeimpl_fntype nvtxNameClContextW_impl_fnptr;
/* 中文逐项-变量 V2854：nvtxNameClCommandQueueA_impl_fnptr
 * nvtxNameClCommandQueueA_impl_fnptr（nvtxNameClCommandQueueA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClCommandQueueA_fakeimpl_fntype nvtxNameClCommandQueueA_impl_fnptr;
/* 中文逐项-变量 V2855：nvtxNameClCommandQueueW_impl_fnptr
 * nvtxNameClCommandQueueW_impl_fnptr（nvtxNameClCommandQueueW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClCommandQueueW_fakeimpl_fntype nvtxNameClCommandQueueW_impl_fnptr;
/* 中文逐项-变量 V2856：nvtxNameClMemObjectA_impl_fnptr
 * nvtxNameClMemObjectA_impl_fnptr（nvtxNameClMemObjectA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClMemObjectA_fakeimpl_fntype nvtxNameClMemObjectA_impl_fnptr;
/* 中文逐项-变量 V2857：nvtxNameClMemObjectW_impl_fnptr
 * nvtxNameClMemObjectW_impl_fnptr（nvtxNameClMemObjectW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClMemObjectW_fakeimpl_fntype nvtxNameClMemObjectW_impl_fnptr;
/* 中文逐项-变量 V2858：nvtxNameClSamplerA_impl_fnptr
 * nvtxNameClSamplerA_impl_fnptr（nvtxNameClSamplerA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClSamplerA_fakeimpl_fntype nvtxNameClSamplerA_impl_fnptr;
/* 中文逐项-变量 V2859：nvtxNameClSamplerW_impl_fnptr
 * nvtxNameClSamplerW_impl_fnptr（nvtxNameClSamplerW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClSamplerW_fakeimpl_fntype nvtxNameClSamplerW_impl_fnptr;
/* 中文逐项-变量 V2860：nvtxNameClProgramA_impl_fnptr
 * nvtxNameClProgramA_impl_fnptr（nvtxNameClProgramA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClProgramA_fakeimpl_fntype nvtxNameClProgramA_impl_fnptr;
/* 中文逐项-变量 V2861：nvtxNameClProgramW_impl_fnptr
 * nvtxNameClProgramW_impl_fnptr（nvtxNameClProgramW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClProgramW_fakeimpl_fntype nvtxNameClProgramW_impl_fnptr;
/* 中文逐项-变量 V2862：nvtxNameClEventA_impl_fnptr
 * nvtxNameClEventA_impl_fnptr（nvtxNameClEventA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClEventA_fakeimpl_fntype nvtxNameClEventA_impl_fnptr;
/* 中文逐项-变量 V2863：nvtxNameClEventW_impl_fnptr
 * nvtxNameClEventW_impl_fnptr（nvtxNameClEventW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameClEventW_fakeimpl_fntype nvtxNameClEventW_impl_fnptr;

/* 中文逐项-变量 V2864：nvtxNameCudaDeviceA_impl_fnptr
 * nvtxNameCudaDeviceA_impl_fnptr（nvtxNameCudaDeviceA_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCudaDeviceA_impl_fntype nvtxNameCudaDeviceA_impl_fnptr;
/* 中文逐项-变量 V2865：nvtxNameCudaDeviceW_impl_fnptr
 * nvtxNameCudaDeviceW_impl_fnptr（nvtxNameCudaDeviceW_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCudaDeviceW_impl_fntype nvtxNameCudaDeviceW_impl_fnptr;
/* 中文逐项-变量 V2866：nvtxNameCudaStreamA_impl_fnptr
 * nvtxNameCudaStreamA_impl_fnptr（nvtxNameCudaStreamA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCudaStreamA_fakeimpl_fntype nvtxNameCudaStreamA_impl_fnptr;
/* 中文逐项-变量 V2867：nvtxNameCudaStreamW_impl_fnptr
 * nvtxNameCudaStreamW_impl_fnptr（nvtxNameCudaStreamW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCudaStreamW_fakeimpl_fntype nvtxNameCudaStreamW_impl_fnptr;
/* 中文逐项-变量 V2868：nvtxNameCudaEventA_impl_fnptr
 * nvtxNameCudaEventA_impl_fnptr（nvtxNameCudaEventA_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCudaEventA_fakeimpl_fntype nvtxNameCudaEventA_impl_fnptr;
/* 中文逐项-变量 V2869：nvtxNameCudaEventW_impl_fnptr
 * nvtxNameCudaEventW_impl_fnptr（nvtxNameCudaEventW_fakeimpl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxNameCudaEventW_fakeimpl_fntype nvtxNameCudaEventW_impl_fnptr;

/* 中文逐项-变量 V2870：nvtxDomainMarkEx_impl_fnptr
 * nvtxDomainMarkEx_impl_fnptr（nvtxDomainMarkEx_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainMarkEx_impl_fntype nvtxDomainMarkEx_impl_fnptr;
/* 中文逐项-变量 V2871：nvtxDomainRangeStartEx_impl_fnptr
 * nvtxDomainRangeStartEx_impl_fnptr（nvtxDomainRangeStartEx_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainRangeStartEx_impl_fntype nvtxDomainRangeStartEx_impl_fnptr;
/* 中文逐项-变量 V2872：nvtxDomainRangeEnd_impl_fnptr
 * nvtxDomainRangeEnd_impl_fnptr（nvtxDomainRangeEnd_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainRangeEnd_impl_fntype nvtxDomainRangeEnd_impl_fnptr;
/* 中文逐项-变量 V2873：nvtxDomainRangePushEx_impl_fnptr
 * nvtxDomainRangePushEx_impl_fnptr（nvtxDomainRangePushEx_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainRangePushEx_impl_fntype nvtxDomainRangePushEx_impl_fnptr;
/* 中文逐项-变量 V2874：nvtxDomainRangePop_impl_fnptr
 * nvtxDomainRangePop_impl_fnptr（nvtxDomainRangePop_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainRangePop_impl_fntype nvtxDomainRangePop_impl_fnptr;
/* 中文逐项-变量 V2875：nvtxDomainResourceCreate_impl_fnptr
 * nvtxDomainResourceCreate_impl_fnptr（nvtxDomainResourceCreate_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainResourceCreate_impl_fntype nvtxDomainResourceCreate_impl_fnptr;
/* 中文逐项-变量 V2876：nvtxDomainResourceDestroy_impl_fnptr
 * nvtxDomainResourceDestroy_impl_fnptr（nvtxDomainResourceDestroy_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainResourceDestroy_impl_fntype nvtxDomainResourceDestroy_impl_fnptr;
/* 中文逐项-变量 V2877：nvtxDomainNameCategoryA_impl_fnptr
 * nvtxDomainNameCategoryA_impl_fnptr（nvtxDomainNameCategoryA_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainNameCategoryA_impl_fntype nvtxDomainNameCategoryA_impl_fnptr;
/* 中文逐项-变量 V2878：nvtxDomainNameCategoryW_impl_fnptr
 * nvtxDomainNameCategoryW_impl_fnptr（nvtxDomainNameCategoryW_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainNameCategoryW_impl_fntype nvtxDomainNameCategoryW_impl_fnptr;
/* 中文逐项-变量 V2879：nvtxDomainRegisterStringA_impl_fnptr
 * nvtxDomainRegisterStringA_impl_fnptr（nvtxDomainRegisterStringA_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainRegisterStringA_impl_fntype nvtxDomainRegisterStringA_impl_fnptr;
/* 中文逐项-变量 V2880：nvtxDomainRegisterStringW_impl_fnptr
 * nvtxDomainRegisterStringW_impl_fnptr（nvtxDomainRegisterStringW_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainRegisterStringW_impl_fntype nvtxDomainRegisterStringW_impl_fnptr;
/* 中文逐项-变量 V2881：nvtxDomainCreateA_impl_fnptr
 * nvtxDomainCreateA_impl_fnptr（nvtxDomainCreateA_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainCreateA_impl_fntype nvtxDomainCreateA_impl_fnptr;
/* 中文逐项-变量 V2882：nvtxDomainCreateW_impl_fnptr
 * nvtxDomainCreateW_impl_fnptr（nvtxDomainCreateW_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainCreateW_impl_fntype nvtxDomainCreateW_impl_fnptr;
/* 中文逐项-变量 V2883：nvtxDomainDestroy_impl_fnptr
 * nvtxDomainDestroy_impl_fnptr（nvtxDomainDestroy_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainDestroy_impl_fntype nvtxDomainDestroy_impl_fnptr;
/* 中文逐项-变量 V2884：nvtxInitialize_impl_fnptr
 * nvtxInitialize_impl_fnptr（nvtxInitialize_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxInitialize_impl_fntype nvtxInitialize_impl_fnptr;

/* 中文逐项-变量 V2885：nvtxDomainSyncUserCreate_impl_fnptr
 * nvtxDomainSyncUserCreate_impl_fnptr（nvtxDomainSyncUserCreate_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainSyncUserCreate_impl_fntype nvtxDomainSyncUserCreate_impl_fnptr;
/* 中文逐项-变量 V2886：nvtxDomainSyncUserDestroy_impl_fnptr
 * nvtxDomainSyncUserDestroy_impl_fnptr（nvtxDomainSyncUserDestroy_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainSyncUserDestroy_impl_fntype nvtxDomainSyncUserDestroy_impl_fnptr;
/* 中文逐项-变量 V2887：nvtxDomainSyncUserAcquireStart_impl_fnptr
 * nvtxDomainSyncUserAcquireStart_impl_fnptr（nvtxDomainSyncUserAcquireStart_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainSyncUserAcquireStart_impl_fntype nvtxDomainSyncUserAcquireStart_impl_fnptr;
/* 中文逐项-变量 V2888：nvtxDomainSyncUserAcquireFailed_impl_fnptr
 * nvtxDomainSyncUserAcquireFailed_impl_fnptr（nvtxDomainSyncUserAcquireFailed_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainSyncUserAcquireFailed_impl_fntype nvtxDomainSyncUserAcquireFailed_impl_fnptr;
/* 中文逐项-变量 V2889：nvtxDomainSyncUserAcquireSuccess_impl_fnptr
 * nvtxDomainSyncUserAcquireSuccess_impl_fnptr（nvtxDomainSyncUserAcquireSuccess_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainSyncUserAcquireSuccess_impl_fntype nvtxDomainSyncUserAcquireSuccess_impl_fnptr;
/* 中文逐项-变量 V2890：nvtxDomainSyncUserReleasing_impl_fnptr
 * nvtxDomainSyncUserReleasing_impl_fnptr（nvtxDomainSyncUserReleasing_impl_fntype；成员 nvtxGlobals_t）：该NVTX
 * API当前实现函数指针；初始化跳板被工具真实回调或no-op替换，调用前检查是否为空。
 */
    nvtxDomainSyncUserReleasing_impl_fntype nvtxDomainSyncUserReleasing_impl_fnptr;

    /* Tables of function pointers -- Extra null added to the end to ensure
    *  a crash instead of silent corruption if a tool reads off the end. */
/* 中文逐项-变量 V2891：functionTable_CORE
 * functionTable_CORE（NvtxFunctionPointer；成员 nvtxGlobals_t）：对应NVTX模块的回调函数表，工具注入后API从这里分派。
 */
    NvtxFunctionPointer* functionTable_CORE  [NVTX_CBID_CORE_SIZE   + 1];
/* 中文逐项-变量 V2892：functionTable_CUDA
 * functionTable_CUDA（NvtxFunctionPointer；成员 nvtxGlobals_t）：对应NVTX模块的回调函数表，工具注入后API从这里分派。
 */
    NvtxFunctionPointer* functionTable_CUDA  [NVTX_CBID_CUDA_SIZE   + 1];
/* 中文逐项-变量 V2893：functionTable_OPENCL
 * functionTable_OPENCL（NvtxFunctionPointer；成员 nvtxGlobals_t）：对应NVTX模块的回调函数表，工具注入后API从这里分派。
 */
    NvtxFunctionPointer* functionTable_OPENCL[NVTX_CBID_OPENCL_SIZE + 1];
/* 中文逐项-变量 V2894：functionTable_CUDART
 * functionTable_CUDART（NvtxFunctionPointer；成员 nvtxGlobals_t）：对应NVTX模块的回调函数表，工具注入后API从这里分派。
 */
    NvtxFunctionPointer* functionTable_CUDART[NVTX_CBID_CUDART_SIZE + 1];
/* 中文逐项-变量 V2895：functionTable_CORE2
 * functionTable_CORE2（NvtxFunctionPointer；成员 nvtxGlobals_t）：对应NVTX模块的回调函数表，工具注入后API从这里分派。
 */
    NvtxFunctionPointer* functionTable_CORE2 [NVTX_CBID_CORE2_SIZE  + 1];
/* 中文逐项-变量 V2896：functionTable_SYNC
 * functionTable_SYNC（NvtxFunctionPointer；成员 nvtxGlobals_t）：对应NVTX模块的回调函数表，工具注入后API从这里分派。
 */
    NvtxFunctionPointer* functionTable_SYNC  [NVTX_CBID_SYNC_SIZE   + 1];
} nvtxGlobals_t;

/* 中文逐项-变量 V2897：nvtxGlobals_t NVTX_VERSIONED_IDENTIFIER
 * nvtxGlobals_t
 * NVTX_VERSIONED_IDENTIFIER（NVTX_LINKONCE_DEFINE_GLOBAL；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvtxGlobals_t NVTX_VERSIONED_IDENTIFIER = { NVTX_INIT_STATE_FRESH, { sizeof(NvtxExportTableCallbacks),
 * NVTX_VERSIONED_IDENTIFIER(nvtxEtiGetModuleFunctionTable) }, { sizeof(NvtxExportTableVersionInfo), NVTX_VERSION, 0,
 * NVTX_VERSIONED_IDENTIFIER(nvtxEtiSetInjectionNvtxVersi
 * …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
NVTX_LINKONCE_DEFINE_GLOBAL nvtxGlobals_t NVTX_VERSIONED_IDENTIFIER(nvtxGlobals) =
{
    NVTX_INIT_STATE_FRESH,

    {
        sizeof(NvtxExportTableCallbacks),
        NVTX_VERSIONED_IDENTIFIER(nvtxEtiGetModuleFunctionTable)
    },
    {
        sizeof(NvtxExportTableVersionInfo),
        NVTX_VERSION,
        0,
        NVTX_VERSIONED_IDENTIFIER(nvtxEtiSetInjectionNvtxVersion)
    },

    /* Implementation function pointers */
    NVTX_VERSIONED_IDENTIFIER(nvtxMarkEx_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxMarkA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxMarkW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartEx_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxRangeEnd_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxRangePushEx_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxRangePushA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxRangePushW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxRangePop_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadW_impl_init),

    NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventW_impl_init),

    NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventW_impl_init),

    NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventW_impl_init),

    NVTX_VERSIONED_IDENTIFIER(nvtxDomainMarkEx_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeStartEx_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeEnd_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePushEx_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePop_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceCreate_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceDestroy_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateA_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateW_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainDestroy_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxInitialize_impl_init),

    NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserCreate_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserDestroy_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireStart_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireFailed_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireSuccess_impl_init),
    NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserReleasing_impl_init),

    /* Tables of function pointers */
    {
        0,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr,
        0
    },
    {
        0,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr,
        0
    },
    {
        0,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr,
        0
    },
    {
        0,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr,
        0
    },
    {
        0,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr,
        0
    },
    {
        0,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr,
        (NvtxFunctionPointer*)&NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr,
        0
    }
};

/* ---- Define static inline implementations of core API functions ---- */

#include "nvtxImplCore.h"

/* ---- Define implementations of export table functions ---- */

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0675：nvtxEtiGetModuleFunctionTable
 * 用途与位置：按模块与接口版本导出NVTX回调函数表，让注入工具为对应API登记实现。
 * 参数 module（NvtxCallbackModule module）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 out_table（NvtxFunctionTable* out_table）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 out_size（unsigned int* out_size）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：out_size；out_table。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return 0;；return 1;。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxEtiGetModuleFunctionTable)(
    NvtxCallbackModule module,
    NvtxFunctionTable* out_table,
    unsigned int* out_size)
{
/* 中文逐项-变量 V2898：bytes
 * bytes（unsigned int；函数 nvtxEtiGetModuleFunctionTable 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bytes = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    unsigned int bytes = 0;
/* 中文逐项-变量 V2899：table
 * table（NvtxFunctionTable；函数 nvtxEtiGetModuleFunctionTable 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 table = (NvtxFunctionTable)0；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    NvtxFunctionTable table = (NvtxFunctionTable)0;

    switch (module)
    {
    case NVTX_CB_MODULE_CORE:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE);
        break;
    case NVTX_CB_MODULE_CUDA:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDA;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDA);
        break;
    case NVTX_CB_MODULE_OPENCL:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_OPENCL;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_OPENCL);
        break;
    case NVTX_CB_MODULE_CUDART:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDART;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDART);
        break;
    case NVTX_CB_MODULE_CORE2:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE2;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE2);
        break;
    case NVTX_CB_MODULE_SYNC:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_SYNC;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_SYNC);
        break;
    default: return 0;
    }

    if (out_size)
        *out_size = (bytes / (unsigned int)sizeof(NvtxFunctionPointer*)) - 1;

    if (out_table)
        *out_table = table;

    return 1;
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0676：nvtxGetExportTable
 * 用途与位置：根据导出表id提供版本/回调导出入口，未知id不返回任意布局对象。
 * 参数 exportTableId（uint32_t exportTableId）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).etblCallbacks;；return
 * &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).etblVersionInfo;；return 0;。CHECK类宏还可能提前返回/跳转。
 */const void* NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxGetExportTable)(uint32_t exportTableId)
{
    switch (exportTableId)
    {
    case NVTX_ETID_CALLBACKS:       return &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).etblCallbacks;
    case NVTX_ETID_VERSIONINFO:     return &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).etblVersionInfo;
    default:                        return 0;
    }
}

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文逐项-函数 F0677：nvtxEtiSetInjectionNvtxVersion
 * 用途与位置：记录注入工具宣告的NVTX版本信息，配合回调表协商。
 * 参数 version（uint32_t version）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */void NVTX_API NVTX_VERSIONED_IDENTIFIER(nvtxEtiSetInjectionNvtxVersion)(uint32_t version)
{
    /* Reserved for custom implementations to resolve problems with tools */
    (void)version;
}

/* ---- Define implementations of init versions of all API functions ---- */

#include "nvtxInitDefs.h"

/* ---- Define implementations of initialization functions ---- */

#include "nvtxInit.h"

#ifdef __GNUC__
#pragma GCC visibility pop
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
