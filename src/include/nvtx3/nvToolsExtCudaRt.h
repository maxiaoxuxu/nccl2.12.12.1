/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#include "nvToolsExt.h"

#include "cuda.h"
#include "driver_types.h"

#ifndef NVTOOLSEXT_CUDART_V3
/* 中文逐项-常量 X0799：NVTOOLSEXT_CUDART_V3
 * 编译期定义：NVTOOLSEXT_CUDART_V3 = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTOOLSEXT_CUDART_V3

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ========================================================================= */
/** \name Functions for CUDA Resource Naming
*/
/** \addtogroup RESOURCE_NAMING
 * \section RESOURCE_NAMING_CUDART CUDA Runtime Resource Naming
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
/* 中文逐项-常量 X0800：NVTX_RESOURCE_CLASS_CUDART
 * 编译期定义：NVTX_RESOURCE_CLASS_CUDART = 5。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_RESOURCE_CLASS_CUDART 5
/** \endcond */

/*  ------------------------------------------------------------------------- */
/** \brief Resource types for CUDART
*/
typedef /* 中文逐项-类型 T0329：nvtxResourceCUDARTType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxResourceCUDARTType_t
{
/* 中文逐项-枚举项 X0801：NVTX_RESOURCE_TYPE_CUDART_DEVICE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(CUDART, 0)
 */
    NVTX_RESOURCE_TYPE_CUDART_DEVICE = NVTX_RESOURCE_MAKE_TYPE(CUDART, 0), /* int device */
/* 中文逐项-枚举项 X0802：NVTX_RESOURCE_TYPE_CUDART_STREAM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(CUDART, 1)
 */
    NVTX_RESOURCE_TYPE_CUDART_STREAM = NVTX_RESOURCE_MAKE_TYPE(CUDART, 1), /* cudaStream_t */
/* 中文逐项-枚举项 X0803：NVTX_RESOURCE_TYPE_CUDART_EVENT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(CUDART, 2)
 */
    NVTX_RESOURCE_TYPE_CUDART_EVENT = NVTX_RESOURCE_MAKE_TYPE(CUDART, 2), /* cudaEvent_t */
} nvtxResourceCUDARTType_t;


/* ------------------------------------------------------------------------- */
/** \brief Annotates a CUDA device.
 *
 * Allows the user to associate a CUDA device with a user-provided name.
 *
 * \param device - The id of the CUDA device to name.
 * \param name   - The name of the CUDA device.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0804：nvtxNameCudaDeviceA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCudaDeviceA(int device, const char* name);
 */void NVTX_API nvtxNameCudaDeviceA(int device, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0805：nvtxNameCudaDeviceW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCudaDeviceW(int device, const wchar_t* name);
 */void NVTX_API nvtxNameCudaDeviceW(int device, const wchar_t* name);
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
NVTX_DECLSPEC /* 中文逐项-接口 X0806：nvtxNameCudaStreamA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCudaStreamA(cudaStream_t stream, const char* name);
 */void NVTX_API nvtxNameCudaStreamA(cudaStream_t stream, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0807：nvtxNameCudaStreamW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCudaStreamW(cudaStream_t stream, const wchar_t* name);
 */void NVTX_API nvtxNameCudaStreamW(cudaStream_t stream, const wchar_t* name);
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
NVTX_DECLSPEC /* 中文逐项-接口 X0808：nvtxNameCudaEventA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCudaEventA(cudaEvent_t event, const char* name);
 */void NVTX_API nvtxNameCudaEventA(cudaEvent_t event, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0809：nvtxNameCudaEventW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCudaEventW(cudaEvent_t event, const wchar_t* name);
 */void NVTX_API nvtxNameCudaEventW(cudaEvent_t event, const wchar_t* name);
/** @} */

/** @} */ /* END RESOURCE_NAMING */

/* ========================================================================= */
#ifdef UNICODE
/* 中文逐项-常量 X0810：nvtxNameCudaDevice
 * 编译期定义：nvtxNameCudaDevice = nvtxNameCudaDeviceW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCudaDevice nvtxNameCudaDeviceW
/* 中文逐项-常量 X0811：nvtxNameCudaStream
 * 编译期定义：nvtxNameCudaStream = nvtxNameCudaStreamW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCudaStream nvtxNameCudaStreamW
/* 中文逐项-常量 X0812：nvtxNameCudaEvent
 * 编译期定义：nvtxNameCudaEvent = nvtxNameCudaEventW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCudaEvent  nvtxNameCudaEventW
#else
/* 中文逐项-常量 X0813：nvtxNameCudaDevice
 * 编译期定义：nvtxNameCudaDevice = nvtxNameCudaDeviceA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCudaDevice nvtxNameCudaDeviceA
/* 中文逐项-常量 X0814：nvtxNameCudaStream
 * 编译期定义：nvtxNameCudaStream = nvtxNameCudaStreamA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCudaStream nvtxNameCudaStreamA
/* 中文逐项-常量 X0815：nvtxNameCudaEvent
 * 编译期定义：nvtxNameCudaEvent = nvtxNameCudaEventA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameCudaEvent  nvtxNameCudaEventA
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifndef NVTX_NO_IMPL
/* 中文逐项-常量 X0816：NVTX_IMPL_GUARD_CUDART
 * 编译期定义：NVTX_IMPL_GUARD_CUDART = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_IMPL_GUARD_CUDART /* Ensure other headers cannot included directly */
#include "nvtxDetail/nvtxImplCudaRt_v3.h"
#undef NVTX_IMPL_GUARD_CUDART
#endif /*NVTX_NO_IMPL*/

#endif /* NVTOOLSEXT_CUDART_V3 */
