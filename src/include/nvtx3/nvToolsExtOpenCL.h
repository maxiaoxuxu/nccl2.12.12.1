/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#include "nvToolsExt.h"

#include <CL/cl.h>

#ifndef NVTOOLSEXT_OPENCL_V3
/* 中文逐项-常量 X0817：NVTOOLSEXT_OPENCL_V3
 * 编译期定义：NVTOOLSEXT_OPENCL_V3 = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTOOLSEXT_OPENCL_V3

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ========================================================================= */
/** \name Functions for OpenCL Resource Naming
 */
/** \addtogroup RESOURCE_NAMING
 * \section RESOURCE_NAMING_OPENCL OpenCL Resource Naming
 *
 * This section covers the API functions that allow to annotate OpenCL resources
 * with user-provided names.
 *
 * @{
 */

/*  ------------------------------------------------------------------------- */
/* \cond SHOW_HIDDEN 
* \brief Used to build a non-colliding value for resource types separated class
* \version \NVTX_VERSION_2
*/
/* 中文逐项-常量 X0818：NVTX_RESOURCE_CLASS_OPENCL
 * 编译期定义：NVTX_RESOURCE_CLASS_OPENCL = 6。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_RESOURCE_CLASS_OPENCL 6 
/** \endcond */

/*  ------------------------------------------------------------------------- */
/** \brief Resource types for OpenCL
*/
typedef /* 中文逐项-类型 T0330：nvtxResourceOpenCLType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxResourceOpenCLType_t
{
/* 中文逐项-枚举项 X0819：NVTX_RESOURCE_TYPE_OPENCL_DEVICE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(OPENCL, 1)
 */
    NVTX_RESOURCE_TYPE_OPENCL_DEVICE = NVTX_RESOURCE_MAKE_TYPE(OPENCL, 1),
/* 中文逐项-枚举项 X0820：NVTX_RESOURCE_TYPE_OPENCL_CONTEXT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(OPENCL, 2)
 */
    NVTX_RESOURCE_TYPE_OPENCL_CONTEXT = NVTX_RESOURCE_MAKE_TYPE(OPENCL, 2),
/* 中文逐项-枚举项 X0821：NVTX_RESOURCE_TYPE_OPENCL_COMMANDQUEUE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(OPENCL, 3)
 */
    NVTX_RESOURCE_TYPE_OPENCL_COMMANDQUEUE = NVTX_RESOURCE_MAKE_TYPE(OPENCL, 3),
/* 中文逐项-枚举项 X0822：NVTX_RESOURCE_TYPE_OPENCL_MEMOBJECT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(OPENCL, 4)
 */
    NVTX_RESOURCE_TYPE_OPENCL_MEMOBJECT = NVTX_RESOURCE_MAKE_TYPE(OPENCL, 4),
/* 中文逐项-枚举项 X0823：NVTX_RESOURCE_TYPE_OPENCL_SAMPLER
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(OPENCL, 5)
 */
    NVTX_RESOURCE_TYPE_OPENCL_SAMPLER = NVTX_RESOURCE_MAKE_TYPE(OPENCL, 5),
/* 中文逐项-枚举项 X0824：NVTX_RESOURCE_TYPE_OPENCL_PROGRAM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(OPENCL, 6)
 */
    NVTX_RESOURCE_TYPE_OPENCL_PROGRAM = NVTX_RESOURCE_MAKE_TYPE(OPENCL, 6),
/* 中文逐项-枚举项 X0825：NVTX_RESOURCE_TYPE_OPENCL_EVENT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(OPENCL, 7)
 */
    NVTX_RESOURCE_TYPE_OPENCL_EVENT = NVTX_RESOURCE_MAKE_TYPE(OPENCL, 7),
} nvtxResourceOpenCLType_t;


/* ------------------------------------------------------------------------- */
/** \brief Annotates an OpenCL device.
 *
 * Allows to associate an OpenCL device with a user-provided name.
 *
 * \param device - The handle of the OpenCL device to name.
 * \param name   - The name of the OpenCL device.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0826：nvtxNameClDeviceA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClDeviceA(cl_device_id device, const char* name);
 */void NVTX_API nvtxNameClDeviceA(cl_device_id device, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0827：nvtxNameClDeviceW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClDeviceW(cl_device_id device, const wchar_t* name);
 */void NVTX_API nvtxNameClDeviceW(cl_device_id device, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates an OpenCL context.
 *
 * Allows to associate an OpenCL context with a user-provided name.
 *
 * \param context - The handle of the OpenCL context to name.
 * \param name    - The name of the OpenCL context.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0828：nvtxNameClContextA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClContextA(cl_context context, const char* name);
 */void NVTX_API nvtxNameClContextA(cl_context context, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0829：nvtxNameClContextW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClContextW(cl_context context, const wchar_t* name);
 */void NVTX_API nvtxNameClContextW(cl_context context, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates an OpenCL command queue.
 *
 * Allows to associate an OpenCL command queue with a user-provided name.
 *
 * \param command_queue - The handle of the OpenCL command queue to name.
 * \param name          - The name of the OpenCL command queue.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0830：nvtxNameClCommandQueueA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClCommandQueueA(cl_command_queue command_queue, const char*
 * name);
 */void NVTX_API nvtxNameClCommandQueueA(cl_command_queue command_queue, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0831：nvtxNameClCommandQueueW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClCommandQueueW(cl_command_queue command_queue, const
 * wchar_t* name);
 */void NVTX_API nvtxNameClCommandQueueW(cl_command_queue command_queue, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates an OpenCL memory object.
 *
 * Allows to associate an OpenCL memory object with a user-provided name.
 *
 * \param memobj - The handle of the OpenCL memory object to name.
 * \param name   - The name of the OpenCL memory object.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0832：nvtxNameClMemObjectA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClMemObjectA(cl_mem memobj, const char* name);
 */void NVTX_API nvtxNameClMemObjectA(cl_mem memobj, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0833：nvtxNameClMemObjectW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClMemObjectW(cl_mem memobj, const wchar_t* name);
 */void NVTX_API nvtxNameClMemObjectW(cl_mem memobj, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates an OpenCL sampler.
 *
 * Allows to associate an OpenCL sampler with a user-provided name.
 *
 * \param sampler - The handle of the OpenCL sampler to name.
 * \param name    - The name of the OpenCL sampler.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0834：nvtxNameClSamplerA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClSamplerA(cl_sampler sampler, const char* name);
 */void NVTX_API nvtxNameClSamplerA(cl_sampler sampler, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0835：nvtxNameClSamplerW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClSamplerW(cl_sampler sampler, const wchar_t* name);
 */void NVTX_API nvtxNameClSamplerW(cl_sampler sampler, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates an OpenCL program.
 *
 * Allows to associate an OpenCL program with a user-provided name.
 *
 * \param program - The handle of the OpenCL program to name.
 * \param name    - The name of the OpenCL program.
 *
 * \code
 * cpProgram = clCreateProgramWithSource(cxGPUContext, 1,
 *     (const char **) &cSourceCL, &program_length, &ciErrNum);
 * shrCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
 * nvtxNameClProgram(cpProgram, L"PROGRAM_NAME");
 * \endcode
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0836：nvtxNameClProgramA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClProgramA(cl_program program, const char* name);
 */void NVTX_API nvtxNameClProgramA(cl_program program, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0837：nvtxNameClProgramW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClProgramW(cl_program program, const wchar_t* name);
 */void NVTX_API nvtxNameClProgramW(cl_program program, const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Annotates an OpenCL event.
 *
 * Allows to associate an OpenCL event with a user-provided name.
 *
 * \param evnt - The handle of the OpenCL event to name.
 * \param name - The name of the OpenCL event.
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0838：nvtxNameClEventA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClEventA(cl_event evnt, const char* name);
 */void NVTX_API nvtxNameClEventA(cl_event evnt, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0839：nvtxNameClEventW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameClEventW(cl_event evnt, const wchar_t* name);
 */void NVTX_API nvtxNameClEventW(cl_event evnt, const wchar_t* name);
/** @} */

/** @} */ /* END RESOURCE_NAMING */

/* ========================================================================= */
#ifdef UNICODE
/* 中文逐项-常量 X0840：nvtxNameClDevice
 * 编译期定义：nvtxNameClDevice = nvtxNameClDeviceW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClDevice        nvtxNameClDeviceW
/* 中文逐项-常量 X0841：nvtxNameClContext
 * 编译期定义：nvtxNameClContext = nvtxNameClContextW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClContext       nvtxNameClContextW
/* 中文逐项-常量 X0842：nvtxNameClCommandQueue
 * 编译期定义：nvtxNameClCommandQueue = nvtxNameClCommandQueueW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClCommandQueue  nvtxNameClCommandQueueW
/* 中文逐项-常量 X0843：nvtxNameClMemObject
 * 编译期定义：nvtxNameClMemObject = nvtxNameClMemObjectW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClMemObject     nvtxNameClMemObjectW
/* 中文逐项-常量 X0844：nvtxNameClSampler
 * 编译期定义：nvtxNameClSampler = nvtxNameClSamplerW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClSampler       nvtxNameClSamplerW
/* 中文逐项-常量 X0845：nvtxNameClProgram
 * 编译期定义：nvtxNameClProgram = nvtxNameClProgramW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClProgram       nvtxNameClProgramW
/* 中文逐项-常量 X0846：nvtxNameClEvent
 * 编译期定义：nvtxNameClEvent = nvtxNameClEventW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClEvent         nvtxNameClEventW
#else
/* 中文逐项-常量 X0847：nvtxNameClDevice
 * 编译期定义：nvtxNameClDevice = nvtxNameClDeviceA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClDevice        nvtxNameClDeviceA
/* 中文逐项-常量 X0848：nvtxNameClContext
 * 编译期定义：nvtxNameClContext = nvtxNameClContextA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClContext       nvtxNameClContextA
/* 中文逐项-常量 X0849：nvtxNameClCommandQueue
 * 编译期定义：nvtxNameClCommandQueue = nvtxNameClCommandQueueA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClCommandQueue  nvtxNameClCommandQueueA
/* 中文逐项-常量 X0850：nvtxNameClMemObject
 * 编译期定义：nvtxNameClMemObject = nvtxNameClMemObjectA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClMemObject     nvtxNameClMemObjectA
/* 中文逐项-常量 X0851：nvtxNameClSampler
 * 编译期定义：nvtxNameClSampler = nvtxNameClSamplerA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClSampler       nvtxNameClSamplerA
/* 中文逐项-常量 X0852：nvtxNameClProgram
 * 编译期定义：nvtxNameClProgram = nvtxNameClProgramA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClProgram       nvtxNameClProgramA
/* 中文逐项-常量 X0853：nvtxNameClEvent
 * 编译期定义：nvtxNameClEvent = nvtxNameClEventA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define nvtxNameClEvent         nvtxNameClEventA
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifndef NVTX_NO_IMPL
/* 中文逐项-常量 X0854：NVTX_IMPL_GUARD_OPENCL
 * 编译期定义：NVTX_IMPL_GUARD_OPENCL = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_IMPL_GUARD_OPENCL /* Ensure other headers cannot included directly */
#include "nvtxDetail/nvtxImplOpenCL_v3.h"
#undef NVTX_IMPL_GUARD_OPENCL
#endif /*NVTX_NO_IMPL*/

#endif /* NVTOOLSEXT_OPENCL_V3 */
