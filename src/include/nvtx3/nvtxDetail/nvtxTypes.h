/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

/* This header defines types which are used by the internal implementation
*  of NVTX and callback subscribers.  API clients do not use these types,
*  so they are defined here instead of in nvToolsExt.h to clarify they are
*  not part of the NVTX client API. */

#ifndef NVTX_IMPL_GUARD
#error Never include this file directly -- it is automatically included by nvToolsExt.h.
#endif

/* ------ Dependency-free types binary-compatible with real types ------- */

/* In order to avoid having the NVTX core API headers depend on non-NVTX
*  headers like cuda.h, NVTX defines binary-compatible types to use for
*  safely making the initialization versions of all NVTX functions without
*  needing to have definitions for the real types. */

typedef int   nvtx_CUdevice;
typedef void* nvtx_CUcontext;
typedef void* nvtx_CUstream;
typedef void* nvtx_CUevent;

typedef void* nvtx_cudaStream_t;
typedef void* nvtx_cudaEvent_t;

typedef void* nvtx_cl_platform_id;
typedef void* nvtx_cl_device_id;
typedef void* nvtx_cl_context;
typedef void* nvtx_cl_command_queue;
typedef void* nvtx_cl_mem;
typedef void* nvtx_cl_program;
typedef void* nvtx_cl_kernel;
typedef void* nvtx_cl_event;
typedef void* nvtx_cl_sampler;

typedef struct nvtxSyncUser* nvtxSyncUser_t;
struct nvtxSyncUserAttributes_v0;
typedef struct nvtxSyncUserAttributes_v0 nvtxSyncUserAttributes_t;

/* --------- Types for function pointers (with fake API types) ---------- */

typedef void (NVTX_API * nvtxMarkEx_impl_fntype)(const nvtxEventAttributes_t* eventAttrib);
typedef void (NVTX_API * nvtxMarkA_impl_fntype)(const char* message);
typedef void (NVTX_API * nvtxMarkW_impl_fntype)(const wchar_t* message);
typedef nvtxRangeId_t (NVTX_API * nvtxRangeStartEx_impl_fntype)(const nvtxEventAttributes_t* eventAttrib);
typedef nvtxRangeId_t (NVTX_API * nvtxRangeStartA_impl_fntype)(const char* message);
typedef nvtxRangeId_t (NVTX_API * nvtxRangeStartW_impl_fntype)(const wchar_t* message);
typedef void (NVTX_API * nvtxRangeEnd_impl_fntype)(nvtxRangeId_t id);
typedef int (NVTX_API * nvtxRangePushEx_impl_fntype)(const nvtxEventAttributes_t* eventAttrib);
typedef int (NVTX_API * nvtxRangePushA_impl_fntype)(const char* message);
typedef int (NVTX_API * nvtxRangePushW_impl_fntype)(const wchar_t* message);
typedef int (NVTX_API * nvtxRangePop_impl_fntype)(void);
typedef void (NVTX_API * nvtxNameCategoryA_impl_fntype)(uint32_t category, const char* name);
typedef void (NVTX_API * nvtxNameCategoryW_impl_fntype)(uint32_t category, const wchar_t* name);
typedef void (NVTX_API * nvtxNameOsThreadA_impl_fntype)(uint32_t threadId, const char* name);
typedef void (NVTX_API * nvtxNameOsThreadW_impl_fntype)(uint32_t threadId, const wchar_t* name);

/* Real impl types are defined in nvtxImplCuda_v3.h, where CUDA headers are included */
typedef void (NVTX_API * nvtxNameCuDeviceA_fakeimpl_fntype)(nvtx_CUdevice device, const char* name);
typedef void (NVTX_API * nvtxNameCuDeviceW_fakeimpl_fntype)(nvtx_CUdevice device, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCuContextA_fakeimpl_fntype)(nvtx_CUcontext context, const char* name);
typedef void (NVTX_API * nvtxNameCuContextW_fakeimpl_fntype)(nvtx_CUcontext context, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCuStreamA_fakeimpl_fntype)(nvtx_CUstream stream, const char* name);
typedef void (NVTX_API * nvtxNameCuStreamW_fakeimpl_fntype)(nvtx_CUstream stream, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCuEventA_fakeimpl_fntype)(nvtx_CUevent event, const char* name);
typedef void (NVTX_API * nvtxNameCuEventW_fakeimpl_fntype)(nvtx_CUevent event, const wchar_t* name);

/* Real impl types are defined in nvtxImplOpenCL_v3.h, where OPENCL headers are included */
typedef void (NVTX_API * nvtxNameClDeviceA_fakeimpl_fntype)(nvtx_cl_device_id device, const char* name);
typedef void (NVTX_API * nvtxNameClDeviceW_fakeimpl_fntype)(nvtx_cl_device_id device, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClContextA_fakeimpl_fntype)(nvtx_cl_context context, const char* name);
typedef void (NVTX_API * nvtxNameClContextW_fakeimpl_fntype)(nvtx_cl_context context, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClCommandQueueA_fakeimpl_fntype)(nvtx_cl_command_queue command_queue, const char* name);
typedef void (NVTX_API * nvtxNameClCommandQueueW_fakeimpl_fntype)(nvtx_cl_command_queue command_queue, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClMemObjectA_fakeimpl_fntype)(nvtx_cl_mem memobj, const char* name);
typedef void (NVTX_API * nvtxNameClMemObjectW_fakeimpl_fntype)(nvtx_cl_mem memobj, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClSamplerA_fakeimpl_fntype)(nvtx_cl_sampler sampler, const char* name);
typedef void (NVTX_API * nvtxNameClSamplerW_fakeimpl_fntype)(nvtx_cl_sampler sampler, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClProgramA_fakeimpl_fntype)(nvtx_cl_program program, const char* name);
typedef void (NVTX_API * nvtxNameClProgramW_fakeimpl_fntype)(nvtx_cl_program program, const wchar_t* name);
typedef void (NVTX_API * nvtxNameClEventA_fakeimpl_fntype)(nvtx_cl_event evnt, const char* name);
typedef void (NVTX_API * nvtxNameClEventW_fakeimpl_fntype)(nvtx_cl_event evnt, const wchar_t* name);

/* Real impl types are defined in nvtxImplCudaRt_v3.h, where CUDART headers are included */
typedef void (NVTX_API * nvtxNameCudaDeviceA_impl_fntype)(int device, const char* name);
typedef void (NVTX_API * nvtxNameCudaDeviceW_impl_fntype)(int device, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCudaStreamA_fakeimpl_fntype)(nvtx_cudaStream_t stream, const char* name);
typedef void (NVTX_API * nvtxNameCudaStreamW_fakeimpl_fntype)(nvtx_cudaStream_t stream, const wchar_t* name);
typedef void (NVTX_API * nvtxNameCudaEventA_fakeimpl_fntype)(nvtx_cudaEvent_t event, const char* name);
typedef void (NVTX_API * nvtxNameCudaEventW_fakeimpl_fntype)(nvtx_cudaEvent_t event, const wchar_t* name);

typedef void (NVTX_API * nvtxDomainMarkEx_impl_fntype)(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib);
typedef nvtxRangeId_t (NVTX_API * nvtxDomainRangeStartEx_impl_fntype)(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib);
typedef void (NVTX_API * nvtxDomainRangeEnd_impl_fntype)(nvtxDomainHandle_t domain, nvtxRangeId_t id);
typedef int (NVTX_API * nvtxDomainRangePushEx_impl_fntype)(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib);
typedef int (NVTX_API * nvtxDomainRangePop_impl_fntype)(nvtxDomainHandle_t domain);
typedef nvtxResourceHandle_t (NVTX_API * nvtxDomainResourceCreate_impl_fntype)(nvtxDomainHandle_t domain, nvtxResourceAttributes_t* attribs);
typedef void (NVTX_API * nvtxDomainResourceDestroy_impl_fntype)(nvtxResourceHandle_t resource);
typedef void (NVTX_API * nvtxDomainNameCategoryA_impl_fntype)(nvtxDomainHandle_t domain, uint32_t category, const char* name);
typedef void (NVTX_API * nvtxDomainNameCategoryW_impl_fntype)(nvtxDomainHandle_t domain, uint32_t category, const wchar_t* name);
typedef nvtxStringHandle_t (NVTX_API * nvtxDomainRegisterStringA_impl_fntype)(nvtxDomainHandle_t domain, const char* string);
typedef nvtxStringHandle_t (NVTX_API * nvtxDomainRegisterStringW_impl_fntype)(nvtxDomainHandle_t domain, const wchar_t* string);
typedef nvtxDomainHandle_t (NVTX_API * nvtxDomainCreateA_impl_fntype)(const char* message);
typedef nvtxDomainHandle_t (NVTX_API * nvtxDomainCreateW_impl_fntype)(const wchar_t* message);
typedef void (NVTX_API * nvtxDomainDestroy_impl_fntype)(nvtxDomainHandle_t domain);
typedef void (NVTX_API * nvtxInitialize_impl_fntype)(const void* reserved);

typedef nvtxSyncUser_t (NVTX_API * nvtxDomainSyncUserCreate_impl_fntype)(nvtxDomainHandle_t domain, const nvtxSyncUserAttributes_t* attribs);
typedef void (NVTX_API * nvtxDomainSyncUserDestroy_impl_fntype)(nvtxSyncUser_t handle);
typedef void (NVTX_API * nvtxDomainSyncUserAcquireStart_impl_fntype)(nvtxSyncUser_t handle);
typedef void (NVTX_API * nvtxDomainSyncUserAcquireFailed_impl_fntype)(nvtxSyncUser_t handle);
typedef void (NVTX_API * nvtxDomainSyncUserAcquireSuccess_impl_fntype)(nvtxSyncUser_t handle);
typedef void (NVTX_API * nvtxDomainSyncUserReleasing_impl_fntype)(nvtxSyncUser_t handle);

/* ---------------- Types for callback subscription --------------------- */

typedef const void *(NVTX_API * NvtxGetExportTableFunc_t)(uint32_t exportTableId);
typedef int (NVTX_API * NvtxInitializeInjectionNvtxFunc_t)(NvtxGetExportTableFunc_t exportTable);

typedef /* 中文逐项-类型 T0336：NvtxCallbackModule
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum NvtxCallbackModule
{
/* 中文逐项-枚举项 X0932：NVTX_CB_MODULE_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_CB_MODULE_INVALID                 = 0,
/* 中文逐项-枚举项 X0933：NVTX_CB_MODULE_CORE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_CB_MODULE_CORE                    = 1,
/* 中文逐项-枚举项 X0934：NVTX_CB_MODULE_CUDA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_CB_MODULE_CUDA                    = 2,
/* 中文逐项-枚举项 X0935：NVTX_CB_MODULE_OPENCL
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_CB_MODULE_OPENCL                  = 3,
/* 中文逐项-枚举项 X0936：NVTX_CB_MODULE_CUDART
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVTX_CB_MODULE_CUDART                  = 4,
/* 中文逐项-枚举项 X0937：NVTX_CB_MODULE_CORE2
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVTX_CB_MODULE_CORE2                   = 5,
/* 中文逐项-枚举项 X0938：NVTX_CB_MODULE_SYNC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVTX_CB_MODULE_SYNC                    = 6,
    /* --- New constants must only be added directly above this line --- */
/* 中文逐项-枚举项 X0939：NVTX_CB_MODULE_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVTX_CB_MODULE_SIZE,
/* 中文逐项-枚举项 X0940：NVTX_CB_MODULE_FORCE_INT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x7fffffff
 */
    NVTX_CB_MODULE_FORCE_INT               = 0x7fffffff
} NvtxCallbackModule;

typedef /* 中文逐项-类型 T0337：NvtxCallbackIdCore
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum NvtxCallbackIdCore
{
/* 中文逐项-枚举项 X0941：NVTX_CBID_CORE_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_CBID_CORE_INVALID                 =  0,
/* 中文逐项-枚举项 X0942：NVTX_CBID_CORE_MarkEx
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_CBID_CORE_MarkEx                  =  1,
/* 中文逐项-枚举项 X0943：NVTX_CBID_CORE_MarkA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_CBID_CORE_MarkA                   =  2,
/* 中文逐项-枚举项 X0944：NVTX_CBID_CORE_MarkW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_CBID_CORE_MarkW                   =  3,
/* 中文逐项-枚举项 X0945：NVTX_CBID_CORE_RangeStartEx
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVTX_CBID_CORE_RangeStartEx            =  4,
/* 中文逐项-枚举项 X0946：NVTX_CBID_CORE_RangeStartA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVTX_CBID_CORE_RangeStartA             =  5,
/* 中文逐项-枚举项 X0947：NVTX_CBID_CORE_RangeStartW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVTX_CBID_CORE_RangeStartW             =  6,
/* 中文逐项-枚举项 X0948：NVTX_CBID_CORE_RangeEnd
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：7
 */
    NVTX_CBID_CORE_RangeEnd                =  7,
/* 中文逐项-枚举项 X0949：NVTX_CBID_CORE_RangePushEx
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
    NVTX_CBID_CORE_RangePushEx             =  8,
/* 中文逐项-枚举项 X0950：NVTX_CBID_CORE_RangePushA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：9
 */
    NVTX_CBID_CORE_RangePushA              =  9,
/* 中文逐项-枚举项 X0951：NVTX_CBID_CORE_RangePushW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：10
 */
    NVTX_CBID_CORE_RangePushW              = 10,
/* 中文逐项-枚举项 X0952：NVTX_CBID_CORE_RangePop
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：11
 */
    NVTX_CBID_CORE_RangePop                = 11,
/* 中文逐项-枚举项 X0953：NVTX_CBID_CORE_NameCategoryA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：12
 */
    NVTX_CBID_CORE_NameCategoryA           = 12,
/* 中文逐项-枚举项 X0954：NVTX_CBID_CORE_NameCategoryW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：13
 */
    NVTX_CBID_CORE_NameCategoryW           = 13,
/* 中文逐项-枚举项 X0955：NVTX_CBID_CORE_NameOsThreadA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：14
 */
    NVTX_CBID_CORE_NameOsThreadA           = 14,
/* 中文逐项-枚举项 X0956：NVTX_CBID_CORE_NameOsThreadW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：15
 */
    NVTX_CBID_CORE_NameOsThreadW           = 15,
    /* --- New constants must only be added directly above this line --- */
/* 中文逐项-枚举项 X0957：NVTX_CBID_CORE_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVTX_CBID_CORE_SIZE,
/* 中文逐项-枚举项 X0958：NVTX_CBID_CORE_FORCE_INT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x7fffffff
 */
    NVTX_CBID_CORE_FORCE_INT = 0x7fffffff
} NvtxCallbackIdCore;

typedef /* 中文逐项-类型 T0338：NvtxCallbackIdCore2
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum NvtxCallbackIdCore2
{
/* 中文逐项-枚举项 X0959：NVTX_CBID_CORE2_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_CBID_CORE2_INVALID                 = 0,
/* 中文逐项-枚举项 X0960：NVTX_CBID_CORE2_DomainMarkEx
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_CBID_CORE2_DomainMarkEx            = 1,
/* 中文逐项-枚举项 X0961：NVTX_CBID_CORE2_DomainRangeStartEx
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_CBID_CORE2_DomainRangeStartEx      = 2,
/* 中文逐项-枚举项 X0962：NVTX_CBID_CORE2_DomainRangeEnd
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_CBID_CORE2_DomainRangeEnd          = 3,
/* 中文逐项-枚举项 X0963：NVTX_CBID_CORE2_DomainRangePushEx
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVTX_CBID_CORE2_DomainRangePushEx       = 4,
/* 中文逐项-枚举项 X0964：NVTX_CBID_CORE2_DomainRangePop
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVTX_CBID_CORE2_DomainRangePop          = 5,
/* 中文逐项-枚举项 X0965：NVTX_CBID_CORE2_DomainResourceCreate
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVTX_CBID_CORE2_DomainResourceCreate    = 6,
/* 中文逐项-枚举项 X0966：NVTX_CBID_CORE2_DomainResourceDestroy
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：7
 */
    NVTX_CBID_CORE2_DomainResourceDestroy   = 7,
/* 中文逐项-枚举项 X0967：NVTX_CBID_CORE2_DomainNameCategoryA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
    NVTX_CBID_CORE2_DomainNameCategoryA     = 8,
/* 中文逐项-枚举项 X0968：NVTX_CBID_CORE2_DomainNameCategoryW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：9
 */
    NVTX_CBID_CORE2_DomainNameCategoryW     = 9,
/* 中文逐项-枚举项 X0969：NVTX_CBID_CORE2_DomainRegisterStringA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：10
 */
    NVTX_CBID_CORE2_DomainRegisterStringA   = 10,
/* 中文逐项-枚举项 X0970：NVTX_CBID_CORE2_DomainRegisterStringW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：11
 */
    NVTX_CBID_CORE2_DomainRegisterStringW   = 11,
/* 中文逐项-枚举项 X0971：NVTX_CBID_CORE2_DomainCreateA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：12
 */
    NVTX_CBID_CORE2_DomainCreateA           = 12,
/* 中文逐项-枚举项 X0972：NVTX_CBID_CORE2_DomainCreateW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：13
 */
    NVTX_CBID_CORE2_DomainCreateW           = 13,
/* 中文逐项-枚举项 X0973：NVTX_CBID_CORE2_DomainDestroy
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：14
 */
    NVTX_CBID_CORE2_DomainDestroy           = 14,
/* 中文逐项-枚举项 X0974：NVTX_CBID_CORE2_Initialize
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：15
 */
    NVTX_CBID_CORE2_Initialize              = 15,
    /* --- New constants must only be added directly above this line --- */
/* 中文逐项-枚举项 X0975：NVTX_CBID_CORE2_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVTX_CBID_CORE2_SIZE,
/* 中文逐项-枚举项 X0976：NVTX_CBID_CORE2_FORCE_INT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x7fffffff
 */
    NVTX_CBID_CORE2_FORCE_INT               = 0x7fffffff
} NvtxCallbackIdCore2;

typedef /* 中文逐项-类型 T0339：NvtxCallbackIdCuda
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum NvtxCallbackIdCuda
{
/* 中文逐项-枚举项 X0977：NVTX_CBID_CUDA_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_CBID_CUDA_INVALID                 =  0,
/* 中文逐项-枚举项 X0978：NVTX_CBID_CUDA_NameCuDeviceA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_CBID_CUDA_NameCuDeviceA           =  1,
/* 中文逐项-枚举项 X0979：NVTX_CBID_CUDA_NameCuDeviceW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_CBID_CUDA_NameCuDeviceW           =  2,
/* 中文逐项-枚举项 X0980：NVTX_CBID_CUDA_NameCuContextA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_CBID_CUDA_NameCuContextA          =  3,
/* 中文逐项-枚举项 X0981：NVTX_CBID_CUDA_NameCuContextW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVTX_CBID_CUDA_NameCuContextW          =  4,
/* 中文逐项-枚举项 X0982：NVTX_CBID_CUDA_NameCuStreamA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVTX_CBID_CUDA_NameCuStreamA           =  5,
/* 中文逐项-枚举项 X0983：NVTX_CBID_CUDA_NameCuStreamW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVTX_CBID_CUDA_NameCuStreamW           =  6,
/* 中文逐项-枚举项 X0984：NVTX_CBID_CUDA_NameCuEventA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：7
 */
    NVTX_CBID_CUDA_NameCuEventA            =  7,
/* 中文逐项-枚举项 X0985：NVTX_CBID_CUDA_NameCuEventW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
    NVTX_CBID_CUDA_NameCuEventW            =  8,
    /* --- New constants must only be added directly above this line --- */
/* 中文逐项-枚举项 X0986：NVTX_CBID_CUDA_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVTX_CBID_CUDA_SIZE,
/* 中文逐项-枚举项 X0987：NVTX_CBID_CUDA_FORCE_INT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x7fffffff
 */
    NVTX_CBID_CUDA_FORCE_INT               = 0x7fffffff
} NvtxCallbackIdCuda;

typedef /* 中文逐项-类型 T0340：NvtxCallbackIdCudaRt
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum NvtxCallbackIdCudaRt
{
/* 中文逐项-枚举项 X0988：NVTX_CBID_CUDART_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_CBID_CUDART_INVALID               =  0,
/* 中文逐项-枚举项 X0989：NVTX_CBID_CUDART_NameCudaDeviceA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_CBID_CUDART_NameCudaDeviceA       =  1,
/* 中文逐项-枚举项 X0990：NVTX_CBID_CUDART_NameCudaDeviceW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_CBID_CUDART_NameCudaDeviceW       =  2,
/* 中文逐项-枚举项 X0991：NVTX_CBID_CUDART_NameCudaStreamA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_CBID_CUDART_NameCudaStreamA       =  3,
/* 中文逐项-枚举项 X0992：NVTX_CBID_CUDART_NameCudaStreamW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVTX_CBID_CUDART_NameCudaStreamW       =  4,
/* 中文逐项-枚举项 X0993：NVTX_CBID_CUDART_NameCudaEventA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVTX_CBID_CUDART_NameCudaEventA        =  5,
/* 中文逐项-枚举项 X0994：NVTX_CBID_CUDART_NameCudaEventW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVTX_CBID_CUDART_NameCudaEventW        =  6,
    /* --- New constants must only be added directly above this line --- */
/* 中文逐项-枚举项 X0995：NVTX_CBID_CUDART_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVTX_CBID_CUDART_SIZE,
/* 中文逐项-枚举项 X0996：NVTX_CBID_CUDART_FORCE_INT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x7fffffff
 */
    NVTX_CBID_CUDART_FORCE_INT             = 0x7fffffff
} NvtxCallbackIdCudaRt;

typedef /* 中文逐项-类型 T0341：NvtxCallbackIdOpenCL
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum NvtxCallbackIdOpenCL
{
/* 中文逐项-枚举项 X0997：NVTX_CBID_OPENCL_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_CBID_OPENCL_INVALID               =  0,
/* 中文逐项-枚举项 X0998：NVTX_CBID_OPENCL_NameClDeviceA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_CBID_OPENCL_NameClDeviceA         =  1,
/* 中文逐项-枚举项 X0999：NVTX_CBID_OPENCL_NameClDeviceW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_CBID_OPENCL_NameClDeviceW         =  2,
/* 中文逐项-枚举项 X1000：NVTX_CBID_OPENCL_NameClContextA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_CBID_OPENCL_NameClContextA        =  3,
/* 中文逐项-枚举项 X1001：NVTX_CBID_OPENCL_NameClContextW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVTX_CBID_OPENCL_NameClContextW        =  4,
/* 中文逐项-枚举项 X1002：NVTX_CBID_OPENCL_NameClCommandQueueA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVTX_CBID_OPENCL_NameClCommandQueueA   =  5,
/* 中文逐项-枚举项 X1003：NVTX_CBID_OPENCL_NameClCommandQueueW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVTX_CBID_OPENCL_NameClCommandQueueW   =  6,
/* 中文逐项-枚举项 X1004：NVTX_CBID_OPENCL_NameClMemObjectA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：7
 */
    NVTX_CBID_OPENCL_NameClMemObjectA      =  7,
/* 中文逐项-枚举项 X1005：NVTX_CBID_OPENCL_NameClMemObjectW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
    NVTX_CBID_OPENCL_NameClMemObjectW      =  8,
/* 中文逐项-枚举项 X1006：NVTX_CBID_OPENCL_NameClSamplerA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：9
 */
    NVTX_CBID_OPENCL_NameClSamplerA        =  9,
/* 中文逐项-枚举项 X1007：NVTX_CBID_OPENCL_NameClSamplerW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：10
 */
    NVTX_CBID_OPENCL_NameClSamplerW        = 10,
/* 中文逐项-枚举项 X1008：NVTX_CBID_OPENCL_NameClProgramA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：11
 */
    NVTX_CBID_OPENCL_NameClProgramA        = 11,
/* 中文逐项-枚举项 X1009：NVTX_CBID_OPENCL_NameClProgramW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：12
 */
    NVTX_CBID_OPENCL_NameClProgramW        = 12,
/* 中文逐项-枚举项 X1010：NVTX_CBID_OPENCL_NameClEventA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：13
 */
    NVTX_CBID_OPENCL_NameClEventA          = 13,
/* 中文逐项-枚举项 X1011：NVTX_CBID_OPENCL_NameClEventW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：14
 */
    NVTX_CBID_OPENCL_NameClEventW          = 14,
    /* --- New constants must only be added directly above this line --- */
/* 中文逐项-枚举项 X1012：NVTX_CBID_OPENCL_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVTX_CBID_OPENCL_SIZE,
/* 中文逐项-枚举项 X1013：NVTX_CBID_OPENCL_FORCE_INT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x7fffffff
 */
    NVTX_CBID_OPENCL_FORCE_INT             = 0x7fffffff
} NvtxCallbackIdOpenCL;

typedef /* 中文逐项-类型 T0342：NvtxCallbackIdSync
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum NvtxCallbackIdSync
{
/* 中文逐项-枚举项 X1014：NVTX_CBID_SYNC_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_CBID_SYNC_INVALID                      = 0,
/* 中文逐项-枚举项 X1015：NVTX_CBID_SYNC_DomainSyncUserCreate
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_CBID_SYNC_DomainSyncUserCreate         = 1,
/* 中文逐项-枚举项 X1016：NVTX_CBID_SYNC_DomainSyncUserDestroy
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_CBID_SYNC_DomainSyncUserDestroy        = 2,
/* 中文逐项-枚举项 X1017：NVTX_CBID_SYNC_DomainSyncUserAcquireStart
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_CBID_SYNC_DomainSyncUserAcquireStart   = 3,
/* 中文逐项-枚举项 X1018：NVTX_CBID_SYNC_DomainSyncUserAcquireFailed
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVTX_CBID_SYNC_DomainSyncUserAcquireFailed  = 4,
/* 中文逐项-枚举项 X1019：NVTX_CBID_SYNC_DomainSyncUserAcquireSuccess
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVTX_CBID_SYNC_DomainSyncUserAcquireSuccess = 5,
/* 中文逐项-枚举项 X1020：NVTX_CBID_SYNC_DomainSyncUserReleasing
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVTX_CBID_SYNC_DomainSyncUserReleasing      = 6,
    /* --- New constants must only be added directly above this line --- */
/* 中文逐项-枚举项 X1021：NVTX_CBID_SYNC_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVTX_CBID_SYNC_SIZE,
/* 中文逐项-枚举项 X1022：NVTX_CBID_SYNC_FORCE_INT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x7fffffff
 */
    NVTX_CBID_SYNC_FORCE_INT                    = 0x7fffffff
} NvtxCallbackIdSync;

/* IDs for NVTX Export Tables */
typedef /* 中文逐项-类型 T0343：NvtxExportTableID
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum NvtxExportTableID
{
/* 中文逐项-枚举项 X1023：NVTX_ETID_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_ETID_INVALID                      = 0,
/* 中文逐项-枚举项 X1024：NVTX_ETID_CALLBACKS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_ETID_CALLBACKS                    = 1,
/* 中文逐项-枚举项 X1025：NVTX_ETID_RESERVED0
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_ETID_RESERVED0                    = 2,
/* 中文逐项-枚举项 X1026：NVTX_ETID_VERSIONINFO
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_ETID_VERSIONINFO                  = 3,
    /* --- New constants must only be added directly above this line --- */
/* 中文逐项-枚举项 X1027：NVTX_ETID_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVTX_ETID_SIZE,
/* 中文逐项-枚举项 X1028：NVTX_ETID_FORCE_INT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x7fffffff
 */
    NVTX_ETID_FORCE_INT                    = 0x7fffffff
} NvtxExportTableID;

typedef void (* NvtxFunctionPointer)(void); /* generic uncallable function pointer, must be casted to appropriate function type */
typedef NvtxFunctionPointer** NvtxFunctionTable; /* double pointer because array(1) of pointers(2) to function pointers */

typedef /* 中文逐项-类型 T0344：NvtxExportTableCallbacks
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：struct_size。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct NvtxExportTableCallbacks
{
/* 中文逐项-变量 V3019：struct_size
 * struct_size（size_t；成员 NvtxExportTableCallbacks）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 */
    size_t struct_size;

    /* returns an array of pointer to function pointers*/
    int (NVTX_API *GetModuleFunctionTable)(
        NvtxCallbackModule module,
        NvtxFunctionTable* out_table,
        unsigned int* out_size);
} NvtxExportTableCallbacks;

typedef /* 中文逐项-类型 T0345：NvtxExportTableVersionInfo
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：struct_size、version、reserved0。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct NvtxExportTableVersionInfo
{
    /* sizeof(NvtxExportTableVersionInfo) */
/* 中文逐项-变量 V3020：struct_size
 * struct_size（size_t；成员 NvtxExportTableVersionInfo）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 */
    size_t struct_size;

    /* The API version comes from the NVTX library linked to the app.  The
    * injection library is can use this info to make some assumptions */
/* 中文逐项-变量 V3021：version
 * version（uint32_t；成员 NvtxExportTableVersionInfo）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 */
    uint32_t version;

    /* Reserved for alignment, do not use */
/* 中文逐项-变量 V3022：reserved0
 * reserved0（uint32_t；成员 NvtxExportTableVersionInfo）：ABI/硬件位域保留空间，保留大小和对齐，不能擅自解释为有效配置。
 */
    uint32_t reserved0;

    /* This must be set by tools when attaching to provide applications
    *  the ability to, in emergency situations, detect problematic tools
    *  versions and modify the NVTX source to prevent attaching anything
    *  that causes trouble in the app.  Currently, this value is ignored. */
    void (NVTX_API *SetInjectionNvtxVersion)(
        uint32_t version);
} NvtxExportTableVersionInfo;







