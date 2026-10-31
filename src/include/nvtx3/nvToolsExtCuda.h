/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#include "nvToolsExt.h"

#include "cuda.h"

#ifndef NVTOOLSEXT_CUDA_V3
/* 中文逐项-常量 X0776：NVTOOLSEXT_CUDA_V3
 * 编译期定义：NVTOOLSEXT_CUDA_V3 = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTOOLSEXT_CUDA_V3

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ========================================================================= */
/** \name Functions for CUDA Resource Naming
*/
/** \addtogroup RESOURCE_NAMING
 * \section RESOURCE_NAMING_CUDA CUDA Resource Naming
 *
 * This section covers the API functions that allow to annotate CUDA resources
 * with user-provided names.
 *
 * @{
 */

/*  ------------------------------------------------------------------------- */
/* \cond SHOW_HIDDEN 
* \brief Used to build a non-colliding value for resource types separated class
* \version \NVTX_VERSION_2
*/
/* 中文逐项-常量 X0777：NVTX_RESOURCE_CLASS_CUDA
 * 编译期定义：NVTX_RESOURCE_CLASS_CUDA = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_RESOURCE_CLASS_CUDA  4
/** \endcond */

/*  ------------------------------------------------------------------------- */
/** \brief Resource types for CUDA
*/
typedef /* 中文逐项-类型 T0328：nvtxResourceCUDAType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxResourceCUDAType_t
{
/* 中文逐项-枚举项 X0778：NVTX_RESOURCE_TYPE_CUDA_DEVICE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(CUDA, 1)
 */
    NVTX_RESOURCE_TYPE_CUDA_DEVICE = NVTX_RESOURCE_MAKE_TYPE(CUDA, 1), /* CUdevice */
/* 中文逐项-枚举项 X0779：NVTX_RESOURCE_TYPE_CUDA_CONTEXT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(CUDA, 2)
 */
    NVTX_RESOURCE_TYPE_CUDA_CONTEXT = NVTX_RESOURCE_MAKE_TYPE(CUDA, 2), /* CUcontext */
/* 中文逐项-枚举项 X0780：NVTX_RESOURCE_TYPE_CUDA_STREAM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(CUDA, 3)
 */
    NVTX_RESOURCE_TYPE_CUDA_STREAM = NVTX_RESOURCE_MAKE_TYPE(CUDA, 3), /* CUstream */
/* 中文逐项-枚举项 X0781：NVTX_RESOURCE_TYPE_CUDA_EVENT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(CUDA, 4)
 */
    NVTX_RESOURCE_TYPE_CUDA_EVENT = NVTX_RESOURCE_MAKE_TYPE(CUDA, 4), /* CUevent */
} nvtxResourceCUDAType_t;


/* ------------------------------------------------------------------------- */
/** \brief Annotates a CUDA device.
 *
 * Allows the user to associate a CUDA device with a user-provided name.
 *
 * \param device - The handle of the CUDA device to name.
 * \param name   - The name of the CUDA device.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0782：nvtxNameCuDeviceA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCuDeviceA(CUdevice device, const char* name);
 */void NVTX_API nvtxNameCuDeviceA(CUdevice device, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0783：nvtxNameCuDeviceW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCuDeviceW(CUdevice device, const wchar_t* name);
 */void NVTX_API nvtxNameCuDeviceW(CUdevice device, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates a CUDA context.
 *
 * Allows the user to associate a CUDA context with a user-provided name.
 *
 * \param context - The handle of the CUDA context to name.
 * \param name    - The name of the CUDA context.
 *
 * \par Example:
 * \code
 * CUresult status = cuCtxCreate( &cuContext, 0, cuDevice );
 * if ( CUDA_SUCCESS != status )
 *     goto Error;
 * nvtxNameCuContext(cuContext, "CTX_NAME");
 * \endcode
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0784：nvtxNameCuContextA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCuContextA(CUcontext context, const char* name);
 */void NVTX_API nvtxNameCuContextA(CUcontext context, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0785：nvtxNameCuContextW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCuContextW(CUcontext context, const wchar_t* name);
 */void NVTX_API nvtxNameCuContextW(CUcontext context, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates a CUDA stream.
 *
 * Allows the user to associate a CUDA stream with a user-provided name.
 *
 * \param stream - The handle of the CUDA stream to name.
 * \param name   - The name of the CUDA stream.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0786：nvtxNameCuStreamA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCuStreamA(CUstream stream, const char* name);
 */void NVTX_API nvtxNameCuStreamA(CUstream stream, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0787：nvtxNameCuStreamW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCuStreamW(CUstream stream, const wchar_t* name);
 */void NVTX_API nvtxNameCuStreamW(CUstream stream, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates a CUDA event.
 *
 * Allows the user to associate a CUDA event with a user-provided name.
 *
 * \param event - The handle of the CUDA event to name.
 * \param name  - The name of the CUDA event.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0788：nvtxNameCuEventA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCuEventA(CUevent event, const char* name);
 */void NVTX_API nvtxNameCuEventA(CUevent event, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0789：nvtxNameCuEventW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCuEventW(CUevent event, const wchar_t* name);
 */void NVTX_API nvtxNameCuEventW(CUevent event, const wchar_t* name);
/** @} */

/** @} */ /* END RESOURCE_NAMING */

/* ========================================================================= */
#ifdef UNICODE
/* 中文逐项-常量 X0790：nvtxNameCuDevice
 * 编译期定义：nvtxNameCuDevice = nvtxNameCuDeviceW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCuDevice   nvtxNameCuDeviceW
/* 中文逐项-常量 X0791：nvtxNameCuContext
 * 编译期定义：nvtxNameCuContext = nvtxNameCuContextW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCuContext  nvtxNameCuContextW
/* 中文逐项-常量 X0792：nvtxNameCuStream
 * 编译期定义：nvtxNameCuStream = nvtxNameCuStreamW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCuStream   nvtxNameCuStreamW
/* 中文逐项-常量 X0793：nvtxNameCuEvent
 * 编译期定义：nvtxNameCuEvent = nvtxNameCuEventW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCuEvent    nvtxNameCuEventW
#else
/* 中文逐项-常量 X0794：nvtxNameCuDevice
 * 编译期定义：nvtxNameCuDevice = nvtxNameCuDeviceA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCuDevice   nvtxNameCuDeviceA
/* 中文逐项-常量 X0795：nvtxNameCuContext
 * 编译期定义：nvtxNameCuContext = nvtxNameCuContextA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCuContext  nvtxNameCuContextA
/* 中文逐项-常量 X0796：nvtxNameCuStream
 * 编译期定义：nvtxNameCuStream = nvtxNameCuStreamA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCuStream   nvtxNameCuStreamA
/* 中文逐项-常量 X0797：nvtxNameCuEvent
 * 编译期定义：nvtxNameCuEvent = nvtxNameCuEventA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCuEvent    nvtxNameCuEventA
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifndef NVTX_NO_IMPL
/* 中文逐项-常量 X0798：NVTX_IMPL_GUARD_CUDA
 * 编译期定义：NVTX_IMPL_GUARD_CUDA = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_IMPL_GUARD_CUDA /* Ensure other headers cannot included directly */
#include "nvtxDetail/nvtxImplCuda_v3.h"
#undef NVTX_IMPL_GUARD_CUDA
#endif /*NVTX_NO_IMPL*/

#endif /* NVTOOLSEXT_CUDA_V3 */
