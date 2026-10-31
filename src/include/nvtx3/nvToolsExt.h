/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

/** \file nvToolsExt.h
 */

/* ========================================================================= */
/** \mainpage
 * \tableofcontents
 * \section INTRODUCTION Introduction
 *
 * The NVIDIA Tools Extension library is a set of functions that a
 * developer can use to provide additional information to tools.
 * The additional information is used by the tool to improve
 * analysis and visualization of data.
 *
 * The library introduces close to zero overhead if no tool is
 * attached to the application.  The overhead when a tool is
 * attached is specific to the tool.
 *
 * \section INITIALIZATION_SECTION Initialization
 *
 * Typically the tool's library that plugs into NVTX is indirectly 
 * loaded via enviromental properties that are platform specific. 
 * For some platform or special cases, the user may be required 
 * to instead explicity initialize instead though.   This can also
 * be helpful to control when the API loads a tool's library instead
 * of what would typically be the first function call to emit info.
 * For these rare case, see \ref INITIALIZATION for additional information.
 *
 * \section MARKERS_AND_RANGES Markers and Ranges
 *
 * Markers and ranges are used to describe events at a specific time (markers)
 * or over a time span (ranges) during the execution of the application
 * respectively. 
 *
 * \subsection MARKERS Markers
 * 
 * Markers denote specific moments in time.
 * 
 * 
 * See \ref DOMAINS and \ref EVENT_ATTRIBUTES for additional information on
 * how to specify the domain.
 * 
 * \subsection THREAD_RANGES Thread Ranges
 *
 * Thread ranges denote nested time ranges. Nesting is maintained per thread
 * per domain and does not require any additional correlation mechanism. The
 * duration of a thread range is defined by the corresponding pair of
 * nvtxRangePush* to nvtxRangePop API calls.
 *
 * See \ref DOMAINS and \ref EVENT_ATTRIBUTES for additional information on
 * how to specify the domain.
 *
 * \subsection PROCESS_RANGES Process Ranges
 *
 * Process ranges denote a time span that can expose arbitrary concurrency, as 
 * opposed to thread ranges that only support nesting. In addition the range
 * start event can happen on a different thread than the end marker. For the 
 * correlation of a start/end pair an unique correlation ID is used that is
 * returned from the start API call and needs to be passed into the end API
 * call.
 *
 * \subsection EVENT_ATTRIBUTES Event Attributes
 *
 * \ref MARKERS_AND_RANGES can be annotated with various attributes to provide
 * additional information for an event or to guide the tool's visualization of
 * the data. Each of the attributes is optional and if left unused the
 * attributes fall back to a default value. The attributes include:
 * - color
 * - category
 *
 * To specify any attribute other than the text message, the \ref
 * EVENT_ATTRIBUTE_STRUCTURE "Event Attribute Structure" must be used.
 *
 * \section DOMAINS Domains
 *
 * Domains enable developers to scope annotations. By default all events and
 * annotations are in the default domain. Additional domains can be registered.
 * This allows developers to scope markers, ranges, and resources names to
 * avoid conflicts.
 *
 * The function ::nvtxDomainCreateA or ::nvtxDomainCreateW is used to create
 * a named domain.
 * 
 * Each domain maintains its own
 * - categories
 * - thread range stacks
 * - registered strings
 *
 * The function ::nvtxDomainDestroy marks the end of the domain. Destroying 
 * a domain unregisters and destroys all objects associated with it such as 
 * registered strings, resource objects, named categories, and started ranges. 
 *
 * \section RESOURCE_NAMING Resource Naming
 *
 * This section covers calls that allow to annotate objects with user-provided
 * names in order to allow for a better analysis of complex trace data. All of
 * the functions take the handle or the ID of the object to name and the name.
 * The functions can be called multiple times during the execution of an
 * application, however, in that case it is implementation dependent which
 * name will be reported by the tool.
 * 
 * \subsection CATEGORY_NAMING Category Naming
 *
 * Some function in this library support associating an integer category 
 * to enable filtering and sorting.  The category naming functions allow 
 * the application to associate a user friendly name with the integer 
 * category.  Support for domains have been added in NVTX_VERSION_2 to 
 * avoid collisions when domains are developed independantly. 
 *
 * \subsection RESOURCE_OBJECTS Resource Objects
 *
 * Resource objects are a generic mechanism for attaching data to an application 
 * resource.  The identifier field makes the association to a pointer or handle, 
 * while the type field helps provide deeper understanding of the identifier as 
 * well as enabling differentiation in cases where handles generated by different
 * APIs may collide.  The resource object may also have an associated message to
 * associate with the application resource, enabling further annotation of this 
 * object and how it is used.
 * 
 * The resource object was introduced in NVTX_VERSION_2 to supersede existing naming
 * functions and allow the application resource identified by those functions to be
 * associated to a domain.  The other naming functions are still supported for backward
 * compatibility but will be associated only to the default domain.
 *
 * \subsection RESOURCE_NAMING_OS Resource Naming
 * 
 * Some operating system resources creation APIs do not support providing a user friendly 
 * name, such as some OS thread creation APIs.  This API support resource naming though 
 * both through resource objects and functions following the pattern 
 * nvtxName[RESOURCE_TYPE][A|W](identifier, name).  Resource objects introduced in NVTX_VERSION 2 
 * supersede the other functions with a a more general method of assigning names to OS resources,
 * along with associating them to domains too.  The older nvtxName* functions are only associated 
 * with the default domain.
 * \section EXTENSIONS Optional Extensions
 * Optional extensions will either appear within the existing sections the extend or appear 
 * in the "Related Pages" when they introduce new concepts.
 */

 /**
 * Tools Extension API version
 */
#if defined(NVTX_VERSION) && NVTX_VERSION < 3
#error "Trying to #include NVTX version 3 in a source file where an older NVTX version has already been included.  If you are not directly using NVTX (the NVIDIA Tools Extension library), you are getting this error because libraries you are using have included different versions of NVTX.  Suggested solutions are: (1) reorder #includes so the newest NVTX version is included first, (2) avoid using the conflicting libraries in the same .c/.cpp file, or (3) update the library using the older NVTX version to use the newer version instead."
#endif

/* Header guard */
#if !defined(NVTX_VERSION)
/* 中文逐项-常量 X0694：NVTX_VERSION
 * 编译期定义：NVTX_VERSION = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_VERSION 3

#if defined(_MSC_VER)
/* 中文逐项-常量 X0695：__stdcall
 * 编译期定义：__stdcall = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_API __stdcall
/* 中文逐项-常量 X0696：NVTX_INLINE_STATIC
 * 编译期定义：NVTX_INLINE_STATIC = __inline static。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_INLINE_STATIC __inline static
#else /*defined(__GNUC__)*/
#define NVTX_API
/* 中文逐项-常量 X0697：NVTX_INLINE_STATIC
 * 编译期定义：NVTX_INLINE_STATIC = inline static。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_INLINE_STATIC inline static
#endif /* Platform */

#if defined(NVTX_NO_IMPL)
/* When omitting implementation, avoid declaring functions inline */
/* without definitions, since this causes compiler warnings. */
#define NVTX_DECLSPEC
#elif defined(NVTX_EXPORT_API)
/* Allow overriding definition of NVTX_DECLSPEC when exporting API. */
/* Default is empty, meaning non-inline with external linkage. */
#if !defined(NVTX_DECLSPEC)
#define NVTX_DECLSPEC
#endif
#else
/* Normal NVTX usage defines the NVTX API inline with static */
/* (internal) linkage. */
/* 中文逐项-常量 X0698：NVTX_INLINE_STATIC
 * 编译期定义：NVTX_INLINE_STATIC = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_DECLSPEC NVTX_INLINE_STATIC
#endif

#include "nvtxDetail/nvtxLinkOnce.h"

/* 中文逐项-宏 M0095：NVTX_VERSIONED_IDENTIFIER_L3
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_VERSIONED_IDENTIFIER_L3；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_VERSIONED_IDENTIFIER_L3(NAME, VERSION) NAME##_v##VERSION
/* 中文逐项-宏 M0096：NVTX_VERSIONED_IDENTIFIER_L2
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_VERSIONED_IDENTIFIER_L2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_VERSIONED_IDENTIFIER_L2(NAME, VERSION) NVTX_VERSIONED_IDENTIFIER_L3(NAME, VERSION)
/* 中文逐项-宏 M0097：NVTX_VERSIONED_IDENTIFIER
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_VERSIONED_IDENTIFIER；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_VERSIONED_IDENTIFIER(NAME) NVTX_VERSIONED_IDENTIFIER_L2(NAME, NVTX_VERSION)

/**
 * The nvToolsExt library depends on stdint.h.  If the build tool chain in use
 * does not include stdint.h then define NVTX_STDINT_TYPES_ALREADY_DEFINED
 * and define the following types:
 * <ul>
 *   <li>uint8_t
 *   <li>int8_t
 *   <li>uint16_t
 *   <li>int16_t
 *   <li>uint32_t
 *   <li>int32_t
 *   <li>uint64_t
 *   <li>int64_t
 *   <li>uintptr_t
 *   <li>intptr_t
 * </ul>
 * #define NVTX_STDINT_TYPES_ALREADY_DEFINED if you are using your own header file.
 */
#ifndef NVTX_STDINT_TYPES_ALREADY_DEFINED
#include <stdint.h>
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** 
* Result Codes
*/

/* 中文逐项-常量 X0699：NVTX_SUCCESS
 * 编译期定义：NVTX_SUCCESS = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_SUCCESS 0
/* 中文逐项-常量 X0700：NVTX_FAIL
 * 编译期定义：NVTX_FAIL = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_FAIL 1
/* 中文逐项-常量 X0701：NVTX_ERR_INIT_LOAD_PROPERTY
 * 编译期定义：NVTX_ERR_INIT_LOAD_PROPERTY = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_ERR_INIT_LOAD_PROPERTY 2
/* 中文逐项-常量 X0702：NVTX_ERR_INIT_ACCESS_LIBRARY
 * 编译期定义：NVTX_ERR_INIT_ACCESS_LIBRARY = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_ERR_INIT_ACCESS_LIBRARY 3
/* 中文逐项-常量 X0703：NVTX_ERR_INIT_LOAD_LIBRARY
 * 编译期定义：NVTX_ERR_INIT_LOAD_LIBRARY = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_ERR_INIT_LOAD_LIBRARY 4
/* 中文逐项-常量 X0704：NVTX_ERR_INIT_MISSING_LIBRARY_ENTRY_POINT
 * 编译期定义：NVTX_ERR_INIT_MISSING_LIBRARY_ENTRY_POINT = 5。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_ERR_INIT_MISSING_LIBRARY_ENTRY_POINT 5
/* 中文逐项-常量 X0705：NVTX_ERR_INIT_FAILED_LIBRARY_ENTRY_POINT
 * 编译期定义：NVTX_ERR_INIT_FAILED_LIBRARY_ENTRY_POINT = 6。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_ERR_INIT_FAILED_LIBRARY_ENTRY_POINT 6
/* 中文逐项-常量 X0706：NVTX_ERR_NO_INJECTION_LIBRARY_AVAILABLE
 * 编译期定义：NVTX_ERR_NO_INJECTION_LIBRARY_AVAILABLE = 7。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_ERR_NO_INJECTION_LIBRARY_AVAILABLE 7

/**
 * Size of the nvtxEventAttributes_t structure.
 */
/* 中文逐项-常量 X0707：NVTX_EVENT_ATTRIB_STRUCT_SIZE
 * 编译期定义：NVTX_EVENT_ATTRIB_STRUCT_SIZE = ( (uint16_t)( sizeof(nvtxEventAttributes_t) ) )。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_EVENT_ATTRIB_STRUCT_SIZE ( (uint16_t)( sizeof(nvtxEventAttributes_t) ) )

/* 中文逐项-常量 X0708：NVTX_NO_PUSH_POP_TRACKING
 * 编译期定义：NVTX_NO_PUSH_POP_TRACKING = ((int)-2)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_NO_PUSH_POP_TRACKING ((int)-2)

typedef uint64_t nvtxRangeId_t;

/* Forward declaration of opaque domain registration structure */
struct nvtxDomainRegistration_st;
typedef struct nvtxDomainRegistration_st nvtxDomainRegistration;

/* \brief Domain Handle Structure.
* \anchor DOMAIN_HANDLE_STRUCTURE
*
* This structure is opaque to the user and is used as a handle to reference
* a domain.  This type is returned from tools when using the NVTX API to
* create a domain.
*
*/
typedef nvtxDomainRegistration* nvtxDomainHandle_t;

/* Forward declaration of opaque string registration structure */
struct nvtxStringRegistration_st;
typedef struct nvtxStringRegistration_st nvtxStringRegistration;

/* \brief Registered String Handle Structure.
* \anchor REGISTERED_STRING_HANDLE_STRUCTURE
*
* This structure is opaque to the user and is used as a handle to reference
* a registered string.  This type is returned from tools when using the NVTX
* API to create a registered string.
*
*/
typedef nvtxStringRegistration* nvtxStringHandle_t;

/* ========================================================================= */
/** \defgroup GENERAL General
 * @{
 */

/** ---------------------------------------------------------------------------
 * Color Types
 * ------------------------------------------------------------------------- */
typedef /* 中文逐项-类型 T0319：nvtxColorType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxColorType_t
{
/* 中文逐项-枚举项 X0709：NVTX_COLOR_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_COLOR_UNKNOWN  = 0,                 /**< Color attribute is unused. */
/* 中文逐项-枚举项 X0710：NVTX_COLOR_ARGB
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_COLOR_ARGB     = 1                  /**< An ARGB color is provided. */
} nvtxColorType_t;

/** ---------------------------------------------------------------------------
 * Message Types
 * ------------------------------------------------------------------------- */
typedef /* 中文逐项-类型 T0320：nvtxMessageType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxMessageType_t
{
/* 中文逐项-枚举项 X0711：NVTX_MESSAGE_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_MESSAGE_UNKNOWN          = 0,    /**< Message payload is unused. */
/* 中文逐项-枚举项 X0712：NVTX_MESSAGE_TYPE_ASCII
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_MESSAGE_TYPE_ASCII       = 1,    /**< A character sequence is used as payload. */
/* 中文逐项-枚举项 X0713：NVTX_MESSAGE_TYPE_UNICODE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_MESSAGE_TYPE_UNICODE     = 2,     /**< A wide character sequence is used as payload. */
    /* NVTX_VERSION_2 */
/* 中文逐项-枚举项 X0714：NVTX_MESSAGE_TYPE_REGISTERED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_MESSAGE_TYPE_REGISTERED  = 3,    /**< A unique string handle that was registered
                                                with \ref nvtxDomainRegisterStringA() or 
                                                \ref nvtxDomainRegisterStringW(). */
} nvtxMessageType_t;

typedef /* 中文逐项-类型 T0321：nvtxMessageValue_t
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：ascii、unicode、registered。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */union nvtxMessageValue_t
{
/* 中文逐项-变量 V2792：ascii
 * ascii（char；成员 nvtxMessageValue_t）：消息union中的ASCII/Unicode文本地址或已注册字符串句柄，只使用type指定的视图。
 */
    const char* ascii;
/* 中文逐项-变量 V2793：unicode
 * unicode（wchar_t；成员 nvtxMessageValue_t）：消息union中的ASCII/Unicode文本地址或已注册字符串句柄，只使用type指定的视图。
 */
    const wchar_t* unicode;
    /* NVTX_VERSION_2 */
/* 中文逐项-变量 V2794：registered
 * registered（nvtxStringHandle_t；成员 nvtxMessageValue_t）：消息union中的ASCII/Unicode文本地址或已注册字符串句柄，只使用type指定的视图。
 */
    nvtxStringHandle_t registered;
} nvtxMessageValue_t;


/** @} */ /*END defgroup*/
/* ------------------------------------------------------------------------- */
/** \brief Force initialization (optional)
*
* Force NVTX library to initialize.  The first call to any NVTX API function
* will automatically initialize the entire API.  This can make the first call
* much slower than subsequent calls.  In applications where the first call to
* NVTX may be in a performance-critical section, calling nvtxInitialize before
* any performance-critical sections will ensure NVTX initialization occurs at
* an acceptable time.  Since nvtxInitialize takes no parameters and has no
* expected behavior besides initialization, it is convenient to add a call to
* nvtxInitialize in NVTX-instrumented applications that need to force earlier
* initialization without changing any other code.  For example, if an app's
* first NVTX call is nvtxDomainCreate, and it is difficult to move that call
* earlier because the domain handle must be stored in an object only created
* at that point, adding a call to nvtxInitialize at the top of main() will
* ensure the later call to nvtxDomainCreate is as fast as possible.
*
* \version \NVTX_VERSION_3
*
* \param reserved - must be zero or NULL.
*
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0715：nvtxInitialize
 * 接口职责：显式触发NVTX工具初始化，通常API首次调用也能惰性触发；不创建NCCL通信资源。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxInitialize(const void* reserved);
 */void NVTX_API nvtxInitialize(const void* reserved);
/** @} */


/** @} */ /*END defgroup*/

/* ========================================================================= */
/** \defgroup EVENT_ATTRIBUTES Event Attributes
* @{
*/

/** ---------------------------------------------------------------------------
* Payload Types
* ------------------------------------------------------------------------- */
typedef /* 中文逐项-类型 T0322：nvtxPayloadType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxPayloadType_t
{
/* 中文逐项-枚举项 X0716：NVTX_PAYLOAD_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_PAYLOAD_UNKNOWN = 0,   /**< Color payload is unused. */
/* 中文逐项-枚举项 X0717：NVTX_PAYLOAD_TYPE_UNSIGNED_INT64
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVTX_PAYLOAD_TYPE_UNSIGNED_INT64 = 1,   /**< A 64 bit unsigned integer value is used as payload. */
/* 中文逐项-枚举项 X0718：NVTX_PAYLOAD_TYPE_INT64
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVTX_PAYLOAD_TYPE_INT64 = 2,   /**< A 64 bit signed integer value is used as payload. */
/* 中文逐项-枚举项 X0719：NVTX_PAYLOAD_TYPE_DOUBLE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVTX_PAYLOAD_TYPE_DOUBLE = 3,   /**< A 64 bit floating point value is used as payload. */
    /* NVTX_VERSION_2 */
/* 中文逐项-枚举项 X0720：NVTX_PAYLOAD_TYPE_UNSIGNED_INT32
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVTX_PAYLOAD_TYPE_UNSIGNED_INT32 = 4,   /**< A 32 bit floating point value is used as payload. */
/* 中文逐项-枚举项 X0721：NVTX_PAYLOAD_TYPE_INT32
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVTX_PAYLOAD_TYPE_INT32 = 5,   /**< A 32 bit floating point value is used as payload. */
/* 中文逐项-枚举项 X0722：NVTX_PAYLOAD_TYPE_FLOAT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVTX_PAYLOAD_TYPE_FLOAT = 6    /**< A 32 bit floating point value is used as payload. */
} nvtxPayloadType_t;

/** \brief Event Attribute Structure.
 * \anchor EVENT_ATTRIBUTE_STRUCTURE
 *
 * This structure is used to describe the attributes of an event. The layout of
 * the structure is defined by a specific version of the tools extension
 * library and can change between different versions of the Tools Extension
 * library.
 *
 * \par Initializing the Attributes
 *
 * The caller should always perform the following three tasks when using
 * attributes:
 * <ul>
 *    <li>Zero the structure
 *    <li>Set the version field
 *    <li>Set the size field
 * </ul>
 *
 * Zeroing the structure sets all the event attributes types and values
 * to the default value.
 *
 * The version and size field are used by the Tools Extension
 * implementation to handle multiple versions of the attributes structure.
 *
 * It is recommended that the caller use one of the following to methods
 * to initialize the event attributes structure:
 *
 * \par Method 1: Initializing nvtxEventAttributes for future compatibility
 * \code
 * nvtxEventAttributes_t eventAttrib = {0};
 * eventAttrib.version = NVTX_VERSION;
 * eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
 * \endcode
 *
 * \par Method 2: Initializing nvtxEventAttributes for a specific version
 * \code
 * nvtxEventAttributes_t eventAttrib = {0};
 * eventAttrib.version = 1;
 * eventAttrib.size = (uint16_t)(sizeof(nvtxEventAttributes_v1));
 * \endcode
 *
 * If the caller uses Method 1 it is critical that the entire binary
 * layout of the structure be configured to 0 so that all fields
 * are initialized to the default value.
 *
 * The caller should either use both NVTX_VERSION and
 * NVTX_EVENT_ATTRIB_STRUCT_SIZE (Method 1) or use explicit values
 * and a versioned type (Method 2).  Using a mix of the two methods
 * will likely cause either source level incompatibility or binary
 * incompatibility in the future.
 *
 * \par Settings Attribute Types and Values
 *
 *
 * \par Example:
 * \code
 * // Initialize
 * nvtxEventAttributes_t eventAttrib = {0};
 * eventAttrib.version = NVTX_VERSION;
 * eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
 *
 * // Configure the Attributes
 * eventAttrib.colorType = NVTX_COLOR_ARGB;
 * eventAttrib.color = 0xFF880000;
 * eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
 * eventAttrib.message.ascii = "Example";
 * \endcode
 *
 * In the example the caller does not have to set the value of
 * \ref ::nvtxEventAttributes_v2::category or
 * \ref ::nvtxEventAttributes_v2::payload as these fields were set to
 * the default value by {0}.
 * \sa
 * ::nvtxDomainMarkEx
 * ::nvtxDomainRangeStartEx
 * ::nvtxDomainRangePushEx
 */
typedef /* 中文逐项-类型 T0323：nvtxEventAttributes_v2
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：version、size、category、colorType、color、payloadType、reserved0、payload、ullValue、llValue、dValue、uiValue、iValue、fValue、messageType、message。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct nvtxEventAttributes_v2
{
    /**
    * \brief Version flag of the structure.
    *
    * Needs to be set to NVTX_VERSION to indicate the version of NVTX APIs
    * supported in this header file. This can optionally be overridden to
    * another version of the tools extension library.
    */
/* 中文逐项-变量 V2795：version
 * version（uint16_t；成员 nvtxEventAttributes_v2）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 */
    uint16_t version;

    /**
    * \brief Size of the structure.
    *
    * Needs to be set to the size in bytes of the event attribute
    * structure used to specify the event.
    */
/* 中文逐项-变量 V2796：size
 * size（uint16_t；成员 nvtxEventAttributes_v2）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
    uint16_t size;

    /**
     * \brief ID of the category the event is assigned to.
     *
     * A category is a user-controlled ID that can be used to group
     * events.  The tool may use category IDs to improve filtering or
     * enable grouping of events in the same category. The functions
     * \ref ::nvtxNameCategoryA or \ref ::nvtxNameCategoryW can be used
     * to name a category.
     *
     * Default Value is 0
     */
/* 中文逐项-变量 V2797：category
 * category（uint32_t；成员 nvtxEventAttributes_v2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
    uint32_t category;

    /** \brief Color type specified in this attribute structure.
     *
     * Defines the color format of the attribute structure's \ref COLOR_FIELD
     * "color" field.
     *
     * Default Value is NVTX_COLOR_UNKNOWN
     */
/* 中文逐项-变量 V2798：colorType
 * colorType（int32_t；成员 nvtxEventAttributes_v2）：NVTX颜色/消息/载荷的union类型标签，读取值前先看对应标签。
 */
    int32_t colorType;              /* nvtxColorType_t */

    /** \brief Color assigned to this event. \anchor COLOR_FIELD
     *
     * The color that the tool should use to visualize the event.
     */
/* 中文逐项-变量 V2799：color
 * color（uint32_t；成员 nvtxEventAttributes_v2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
    uint32_t color;

    /**
     * \brief Payload type specified in this attribute structure.
     *
     * Defines the payload format of the attribute structure's \ref PAYLOAD_FIELD
     * "payload" field.
     *
     * Default Value is NVTX_PAYLOAD_UNKNOWN
     */
/* 中文逐项-变量 V2800：payloadType
 * payloadType（int32_t；成员 nvtxEventAttributes_v2）：NVTX颜色/消息/载荷的union类型标签，读取值前先看对应标签。
 */
    int32_t payloadType;            /* nvtxPayloadType_t */

/* 中文逐项-变量 V2801：reserved0
 * reserved0（int32_t；成员 nvtxEventAttributes_v2）：ABI/硬件位域保留空间，保留大小和对齐，不能擅自解释为有效配置。
 */
    int32_t reserved0;

    /**
     * \brief Payload assigned to this event. \anchor PAYLOAD_FIELD
     *
     * A numerical value that can be used to annotate an event. The tool could
     * use the payload data to reconstruct graphs and diagrams.
     */
/* 中文逐项-变量 V2802：payload
 * payload（union payload_t { uint64_t ullValue; int64_t llValue; double dValue; / * NVTX_VERSION_2 * / uint32_t
 * uiValue; …（完整表达式见紧邻源码）；成员 nvtxEventAttributes_v2）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 */
/* 中文逐项-类型 T0324：payload_t
 * 类型职责：本文件的union类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：payload、ullValue、llValue、dValue、uiValue、iValue、fValue。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union payload_t
    {
/* 中文逐项-变量 V2803：ullValue
 * ullValue（uint64_t；成员 nvtxEventAttributes_v2::payload_t）：NVTX事件payload的双精度、浮点、整型、64位或指针视图，读取前核对payloadType。
 */
        uint64_t ullValue;
/* 中文逐项-变量 V2804：llValue
 * llValue（int64_t；成员 nvtxEventAttributes_v2::payload_t）：NVTX事件payload的双精度、浮点、整型、64位或指针视图，读取前核对payloadType。
 */
        int64_t llValue;
/* 中文逐项-变量 V2805：dValue
 * dValue（double；成员 nvtxEventAttributes_v2::payload_t）：NVTX事件payload的双精度、浮点、整型、64位或指针视图，读取前核对payloadType。
 */
        double dValue;
        /* NVTX_VERSION_2 */
/* 中文逐项-变量 V2806：uiValue
 * uiValue（uint32_t；成员 nvtxEventAttributes_v2::payload_t）：NVTX事件payload的双精度、浮点、整型、64位或指针视图，读取前核对payloadType。
 */
        uint32_t uiValue;
/* 中文逐项-变量 V2807：iValue
 * iValue（int32_t；成员 nvtxEventAttributes_v2::payload_t）：NVTX事件payload的双精度、浮点、整型、64位或指针视图，读取前核对payloadType。
 */
        int32_t iValue;
/* 中文逐项-变量 V2808：fValue
 * fValue（float；成员 nvtxEventAttributes_v2::payload_t）：NVTX事件payload的双精度、浮点、整型、64位或指针视图，读取前核对payloadType。
 */
        float fValue;
    } payload;

    /** \brief Message type specified in this attribute structure.
     *
     * Defines the message format of the attribute structure's \ref MESSAGE_FIELD
     * "message" field.
     *
     * Default Value is NVTX_MESSAGE_UNKNOWN
     */
/* 中文逐项-变量 V2809：messageType
 * messageType（int32_t；成员 nvtxEventAttributes_v2）：NVTX颜色/消息/载荷的union类型标签，读取值前先看对应标签。
 */
    int32_t messageType;            /* nvtxMessageType_t */

    /** \brief Message assigned to this attribute structure. \anchor MESSAGE_FIELD
     *
     * The text message that is attached to an event.
     */
/* 中文逐项-变量 V2810：message
 * message（nvtxMessageValue_t；成员 nvtxEventAttributes_v2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
    nvtxMessageValue_t message;

} nvtxEventAttributes_v2;

typedef struct nvtxEventAttributes_v2 nvtxEventAttributes_t;

/** @} */ /*END defgroup*/
/* ========================================================================= */
/** \defgroup MARKERS_AND_RANGES Markers and Ranges
 *
 * See \ref MARKERS_AND_RANGES for more details
 *
 * @{
 */

/** \name Marker */

/* ------------------------------------------------------------------------- */
/** \brief Marks an instantaneous event in the application.
*
* A marker can contain a text message or specify additional information
* using the event attributes structure.  These attributes include a text
* message, color, category, and a payload. Each of the attributes is optional
* and can only be sent out using the \ref nvtxDomainMarkEx function.
*
* nvtxDomainMarkEx(NULL, event) is equivalent to calling
* nvtxMarkEx(event).
*
* \param domain    - The domain of scoping the category.
* \param eventAttrib - The event attribute structure defining the marker's
* attribute types and attribute values.
*
* \sa
* ::nvtxMarkEx
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0723：nvtxDomainMarkEx
 * 接口职责：向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainMarkEx(nvtxDomainHandle_t domain, const
 * nvtxEventAttributes_t* eventAttrib);
 */void NVTX_API nvtxDomainMarkEx(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Marks an instantaneous event in the application.
 *
 * A marker can contain a text message or specify additional information
 * using the event attributes structure.  These attributes include a text
 * message, color, category, and a payload. Each of the attributes is optional
 * and can only be sent out using the \ref nvtxMarkEx function.
 * If \ref nvtxMarkA or \ref nvtxMarkW are used to specify the marker
 * or if an attribute is unspecified then a default value will be used.
 *
 * \param eventAttrib - The event attribute structure defining the marker's
 * attribute types and attribute values.
 *
 * \par Example:
 * \code
 * // zero the structure
 * nvtxEventAttributes_t eventAttrib = {0};
 * // set the version and the size information
 * eventAttrib.version = NVTX_VERSION;
 * eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
 * // configure the attributes.  0 is the default for all attributes.
 * eventAttrib.colorType = NVTX_COLOR_ARGB;
 * eventAttrib.color = 0xFF880000;
 * eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
 * eventAttrib.message.ascii = "Example nvtxMarkEx";
 * nvtxMarkEx(&eventAttrib);
 * \endcode
 *
 * \sa
 * ::nvtxDomainMarkEx
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0724：nvtxMarkEx
 * 接口职责：向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxMarkEx(const nvtxEventAttributes_t* eventAttrib);
 */void NVTX_API nvtxMarkEx(const nvtxEventAttributes_t* eventAttrib);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Marks an instantaneous event in the application.
 *
 * A marker created using \ref nvtxMarkA or \ref nvtxMarkW contains only a
 * text message.
 *
 * \param message     - The message associated to this marker event.
 *
 * \par Example:
 * \code
 * nvtxMarkA("Example nvtxMarkA");
 * nvtxMarkW(L"Example nvtxMarkW");
 * \endcode
 *
 * \sa
 * ::nvtxDomainMarkEx
 * ::nvtxMarkEx
 *
 * \version \NVTX_VERSION_0
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0725：nvtxMarkA
 * 接口职责：向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxMarkA(const char* message);
 */void NVTX_API nvtxMarkA(const char* message);
NVTX_DECLSPEC /* 中文逐项-接口 X0726：nvtxMarkW
 * 接口职责：向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxMarkW(const wchar_t* message);
 */void NVTX_API nvtxMarkW(const wchar_t* message);
/** @} */


/** \name Process Ranges */

/* ------------------------------------------------------------------------- */
/** \brief Starts a process range in a domain.
*
* \param domain    - The domain of scoping the category.
* \param eventAttrib - The event attribute structure defining the range's
* attribute types and attribute values.
*
* \return The unique ID used to correlate a pair of Start and End events.
*
* \remarks Ranges defined by Start/End can overlap.
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("my domain");
* nvtxEventAttributes_t eventAttrib = {0};
* eventAttrib.version = NVTX_VERSION;
* eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
* eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
* eventAttrib.message.ascii = "my range";
* nvtxRangeId_t rangeId = nvtxDomainRangeStartEx(&eventAttrib);
* // ...
* nvtxDomainRangeEnd(rangeId);
* \endcode
*
* \sa
* ::nvtxDomainRangeEnd
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0727：nvtxDomainRangeStartEx
 * 接口职责：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxRangeId_t NVTX_API nvtxDomainRangeStartEx(nvtxDomainHandle_t domain, const
 * nvtxEventAttributes_t* eventAttrib);
 */nvtxRangeId_t NVTX_API nvtxDomainRangeStartEx(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Starts a process range.
 *
 * \param eventAttrib - The event attribute structure defining the range's
 * attribute types and attribute values.
 *
 * \return The unique ID used to correlate a pair of Start and End events.
 *
 * \remarks Ranges defined by Start/End can overlap.
 *
 * \par Example:
 * \code
 * nvtxEventAttributes_t eventAttrib = {0};
 * eventAttrib.version = NVTX_VERSION;
 * eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
 * eventAttrib.category = 3;
 * eventAttrib.colorType = NVTX_COLOR_ARGB;
 * eventAttrib.color = 0xFF0088FF;
 * eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
 * eventAttrib.message.ascii = "Example Range";
 * nvtxRangeId_t rangeId = nvtxRangeStartEx(&eventAttrib);
 * // ...
 * nvtxRangeEnd(rangeId);
 * \endcode
 *
 * \sa
 * ::nvtxRangeEnd
 * ::nvtxDomainRangeStartEx
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0728：nvtxRangeStartEx
 * 接口职责：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxRangeId_t NVTX_API nvtxRangeStartEx(const nvtxEventAttributes_t* eventAttrib);
 */nvtxRangeId_t NVTX_API nvtxRangeStartEx(const nvtxEventAttributes_t* eventAttrib);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Starts a process range.
 *
 * \param message     - The event message associated to this range event.
 *
 * \return The unique ID used to correlate a pair of Start and End events.
 *
 * \remarks Ranges defined by Start/End can overlap.
 *
 * \par Example:
 * \code
 * nvtxRangeId_t r1 = nvtxRangeStartA("Range 1");
 * nvtxRangeId_t r2 = nvtxRangeStartW(L"Range 2");
 * nvtxRangeEnd(r1);
 * nvtxRangeEnd(r2);
 * \endcode
 *
 * \sa
 * ::nvtxRangeEnd
 * ::nvtxRangeStartEx
 * ::nvtxDomainRangeStartEx
 *
 * \version \NVTX_VERSION_0
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0729：nvtxRangeStartA
 * 接口职责：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxRangeId_t NVTX_API nvtxRangeStartA(const char* message);
 */nvtxRangeId_t NVTX_API nvtxRangeStartA(const char* message);
NVTX_DECLSPEC /* 中文逐项-接口 X0730：nvtxRangeStartW
 * 接口职责：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxRangeId_t NVTX_API nvtxRangeStartW(const wchar_t* message);
 */nvtxRangeId_t NVTX_API nvtxRangeStartW(const wchar_t* message);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Ends a process range.
*
* \param domain - The domain 
* \param id - The correlation ID returned from a nvtxRangeStart call.
*
* \remarks This function is offered completeness but is an alias for ::nvtxRangeEnd. 
* It does not need a domain param since that is associated iwth the range ID at ::nvtxDomainRangeStartEx
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("my domain");
* nvtxEventAttributes_t eventAttrib = {0};
* eventAttrib.version = NVTX_VERSION;
* eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
* eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
* eventAttrib.message.ascii = "my range";
* nvtxRangeId_t rangeId = nvtxDomainRangeStartEx(&eventAttrib);
* // ...
* nvtxDomainRangeEnd(rangeId);
* \endcode
*
* \sa
* ::nvtxDomainRangeStartEx
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0731：nvtxDomainRangeEnd
 * 接口职责：按id结束显式NVTX时间范围。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainRangeEnd(nvtxDomainHandle_t domain, nvtxRangeId_t id);
 */void NVTX_API nvtxDomainRangeEnd(nvtxDomainHandle_t domain, nvtxRangeId_t id);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Ends a process range.
 *
 * \param id - The correlation ID returned from an nvtxRangeStart call.
 *
 * \sa
 * ::nvtxDomainRangeStartEx
 * ::nvtxRangeStartEx
 * ::nvtxRangeStartA
 * ::nvtxRangeStartW
 *
 * \version \NVTX_VERSION_0
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0732：nvtxRangeEnd
 * 接口职责：按id结束显式NVTX时间范围。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxRangeEnd(nvtxRangeId_t id);
 */void NVTX_API nvtxRangeEnd(nvtxRangeId_t id);
/** @} */

/** \name Thread Ranges */

/* ------------------------------------------------------------------------- */
/** \brief Starts a nested thread range.
*
* \param domain    - The domain of scoping.
* \param eventAttrib - The event attribute structure defining the range's
* attribute types and attribute values.
*
* \return The 0 based level of range being started. This value is scoped to the domain.
* If an error occurs, a negative value is returned.
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("example domain");
* nvtxEventAttributes_t eventAttrib = {0};
* eventAttrib.version = NVTX_VERSION;
* eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
* eventAttrib.colorType = NVTX_COLOR_ARGB;
* eventAttrib.color = 0xFFFF0000;
* eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
* eventAttrib.message.ascii = "Level 0";
* nvtxDomainRangePushEx(domain, &eventAttrib);
*
* // Re-use eventAttrib
* eventAttrib.messageType = NVTX_MESSAGE_TYPE_UNICODE;
* eventAttrib.message.unicode = L"Level 1";
* nvtxDomainRangePushEx(domain, &eventAttrib);
*
* nvtxDomainRangePop(domain); //level 1
* nvtxDomainRangePop(domain); //level 0
* \endcode
*
* \sa
* ::nvtxDomainRangePop
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0733：nvtxDomainRangePushEx
 * 接口职责：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int NVTX_API nvtxDomainRangePushEx(nvtxDomainHandle_t domain, const
 * nvtxEventAttributes_t* eventAttrib);
 */int NVTX_API nvtxDomainRangePushEx(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Starts a nested thread range.
 *
 * \param eventAttrib - The event attribute structure defining the range's
 * attribute types and attribute values.
 *
 * \return The 0 based level of range being started. This level is per domain.
 * If an error occurs a negative value is returned.
 *
 * \par Example:
 * \code
 * nvtxEventAttributes_t eventAttrib = {0};
 * eventAttrib.version = NVTX_VERSION;
 * eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
 * eventAttrib.colorType = NVTX_COLOR_ARGB;
 * eventAttrib.color = 0xFFFF0000;
 * eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
 * eventAttrib.message.ascii = "Level 0";
 * nvtxRangePushEx(&eventAttrib);
 *
 * // Re-use eventAttrib
 * eventAttrib.messageType = NVTX_MESSAGE_TYPE_UNICODE;
 * eventAttrib.message.unicode = L"Level 1";
 * nvtxRangePushEx(&eventAttrib);
 *
 * nvtxRangePop();
 * nvtxRangePop();
 * \endcode
 *
 * \sa
 * ::nvtxDomainRangePushEx
 * ::nvtxRangePop
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0734：nvtxRangePushEx
 * 接口职责：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int NVTX_API nvtxRangePushEx(const nvtxEventAttributes_t* eventAttrib);
 */int NVTX_API nvtxRangePushEx(const nvtxEventAttributes_t* eventAttrib);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Starts a nested thread range.
 *
 * \param message     - The event message associated to this range event.
 *
 * \return The 0 based level of range being started.  If an error occurs a
 * negative value is returned.
 *
 * \par Example:
 * \code
 * nvtxRangePushA("Level 0");
 * nvtxRangePushW(L"Level 1");
 * nvtxRangePop();
 * nvtxRangePop();
 * \endcode
 *
 * \sa
 * ::nvtxDomainRangePushEx
 * ::nvtxRangePop
 *
 * \version \NVTX_VERSION_0
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0735：nvtxRangePushA
 * 接口职责：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int NVTX_API nvtxRangePushA(const char* message);
 */int NVTX_API nvtxRangePushA(const char* message);
NVTX_DECLSPEC /* 中文逐项-接口 X0736：nvtxRangePushW
 * 接口职责：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int NVTX_API nvtxRangePushW(const wchar_t* message);
 */int NVTX_API nvtxRangePushW(const wchar_t* message);
/** @} */


/* ------------------------------------------------------------------------- */
/** \brief Ends a nested thread range.
*
* \return The level of the range being ended. If an error occurs a negative
* value is returned on the current thread.
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreate("example library");
* nvtxDomainRangePushA(domain, "Level 0");
* nvtxDomainRangePushW(domain, L"Level 1");
* nvtxDomainRangePop(domain);
* nvtxDomainRangePop(domain);
* \endcode
*
* \sa
* ::nvtxRangePushEx
* ::nvtxRangePushA
* ::nvtxRangePushW
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0737：nvtxDomainRangePop
 * 接口职责：从当前线程NVTX范围栈pop结束最近区间。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int NVTX_API nvtxDomainRangePop(nvtxDomainHandle_t domain);
 */int NVTX_API nvtxDomainRangePop(nvtxDomainHandle_t domain);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Ends a nested thread range.
 *
 * \return The level of the range being ended. If an error occurs a negative
 * value is returned on the current thread.
 *
 * \par Example:
 * \code
 * nvtxRangePushA("Level 0");
 * nvtxRangePushW(L"Level 1");
 * nvtxRangePop();
 * nvtxRangePop();
 * \endcode
 *
 * \sa
 * ::nvtxRangePushEx
 * ::nvtxRangePushA
 * ::nvtxRangePushW
 *
 * \version \NVTX_VERSION_0
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0738：nvtxRangePop
 * 接口职责：从当前线程NVTX范围栈pop结束最近区间。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int NVTX_API nvtxRangePop(void);
 */int NVTX_API nvtxRangePop(void);
/** @} */


/** @} */ /*END defgroup*/
/* ========================================================================= */
/** \defgroup RESOURCE_NAMING Resource Naming
 *
 * See \ref RESOURCE_NAMING for more details
 *
 * @{
 */


/*  ------------------------------------------------------------------------- */
/** \name Functions for Generic Resource Naming*/
/*  ------------------------------------------------------------------------- */

/*  ------------------------------------------------------------------------- */
/** \cond SHOW_HIDDEN
* \brief Resource typing helpers.  
*
* Classes are used to make it easy to create a series of resource types 
* per API without collisions 
*/
/* 中文逐项-宏 M0098：NVTX_RESOURCE_MAKE_TYPE
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_RESOURCE_MAKE_TYPE；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_RESOURCE_MAKE_TYPE(CLASS, INDEX) ((((uint32_t)(NVTX_RESOURCE_CLASS_ ## CLASS))<<16)|((uint32_t)(INDEX)))
/* 中文逐项-常量 X0739：NVTX_RESOURCE_CLASS_GENERIC
 * 编译期定义：NVTX_RESOURCE_CLASS_GENERIC = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_RESOURCE_CLASS_GENERIC 1
/** \endcond */

/* ------------------------------------------------------------------------- */
/** \brief Generic resource type for when a resource class is not available.
*
* \sa
* ::nvtxDomainResourceCreate
*
* \version \NVTX_VERSION_2
*/
typedef /* 中文逐项-类型 T0325：nvtxResourceGenericType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxResourceGenericType_t
{
/* 中文逐项-枚举项 X0740：NVTX_RESOURCE_TYPE_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVTX_RESOURCE_TYPE_UNKNOWN = 0,
/* 中文逐项-枚举项 X0741：NVTX_RESOURCE_TYPE_GENERIC_POINTER
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(GENERIC, 1)
 */
    NVTX_RESOURCE_TYPE_GENERIC_POINTER = NVTX_RESOURCE_MAKE_TYPE(GENERIC, 1), /**< Generic pointer assumed to have no collisions with other pointers. */
/* 中文逐项-枚举项 X0742：NVTX_RESOURCE_TYPE_GENERIC_HANDLE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(GENERIC, 2)
 */
    NVTX_RESOURCE_TYPE_GENERIC_HANDLE = NVTX_RESOURCE_MAKE_TYPE(GENERIC, 2), /**< Generic handle assumed to have no collisions with other handles. */
/* 中文逐项-枚举项 X0743：NVTX_RESOURCE_TYPE_GENERIC_THREAD_NATIVE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(GENERIC, 3)
 */
    NVTX_RESOURCE_TYPE_GENERIC_THREAD_NATIVE = NVTX_RESOURCE_MAKE_TYPE(GENERIC, 3), /**< OS native thread identifier. */
/* 中文逐项-枚举项 X0744：NVTX_RESOURCE_TYPE_GENERIC_THREAD_POSIX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(GENERIC, 4)
 */
    NVTX_RESOURCE_TYPE_GENERIC_THREAD_POSIX = NVTX_RESOURCE_MAKE_TYPE(GENERIC, 4) /**< POSIX pthread identifier. */
} nvtxResourceGenericType_t;



/** \brief Resource Attribute Structure.
* \anchor RESOURCE_ATTRIBUTE_STRUCTURE
*
* This structure is used to describe the attributes of a resource. The layout of
* the structure is defined by a specific version of the tools extension
* library and can change between different versions of the Tools Extension
* library.
*
* \par Initializing the Attributes
*
* The caller should always perform the following three tasks when using
* attributes:
* <ul>
*    <li>Zero the structure
*    <li>Set the version field
*    <li>Set the size field
* </ul>
*
* Zeroing the structure sets all the resource attributes types and values
* to the default value.
*
* The version and size field are used by the Tools Extension
* implementation to handle multiple versions of the attributes structure.
*
* It is recommended that the caller use one of the following to methods
* to initialize the event attributes structure:
*
* \par Method 1: Initializing nvtxEventAttributes for future compatibility
* \code
* nvtxResourceAttributes_t attribs = {0};
* attribs.version = NVTX_VERSION;
* attribs.size = NVTX_RESOURCE_ATTRIB_STRUCT_SIZE;
* \endcode
*
* \par Method 2: Initializing nvtxEventAttributes for a specific version
* \code
* nvtxResourceAttributes_v0 attribs = {0};
* attribs.version = 2;
* attribs.size = (uint16_t)(sizeof(nvtxResourceAttributes_v0));
* \endcode
*
* If the caller uses Method 1 it is critical that the entire binary
* layout of the structure be configured to 0 so that all fields
* are initialized to the default value.
*
* The caller should either use both NVTX_VERSION and
* NVTX_RESOURCE_ATTRIB_STRUCT_SIZE (Method 1) or use explicit values
* and a versioned type (Method 2).  Using a mix of the two methods
* will likely cause either source level incompatibility or binary
* incompatibility in the future.
*
* \par Settings Attribute Types and Values
*
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("example domain");
*
* // Initialize
* nvtxResourceAttributes_t attribs = {0};
* attribs.version = NVTX_VERSION;
* attribs.size = NVTX_RESOURCE_ATTRIB_STRUCT_SIZE;
*
* // Configure the Attributes
* attribs.identifierType = NVTX_RESOURCE_TYPE_GENERIC_POINTER;
* attribs.identifier.pValue = (const void*)pMutex;
* attribs.messageType = NVTX_MESSAGE_TYPE_ASCII;
* attribs.message.ascii = "Single thread access to database.";
*
* nvtxResourceHandle_t handle = nvtxDomainResourceCreate(domain, attribs);
* \endcode
*
* \sa
* ::nvtxDomainResourceCreate
*/
typedef /* 中文逐项-类型 T0326：nvtxResourceAttributes_v0
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：version、size、identifierType、identifier、pValue、ullValue、messageType、message。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct nvtxResourceAttributes_v0
{
    /**
    * \brief Version flag of the structure.
    *
    * Needs to be set to NVTX_VERSION to indicate the version of NVTX APIs
    * supported in this header file. This can optionally be overridden to
    * another version of the tools extension library.
    */
/* 中文逐项-变量 V2811：version
 * version（uint16_t；成员 nvtxResourceAttributes_v0）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 */
    uint16_t version;

    /**
    * \brief Size of the structure.
    *
    * Needs to be set to the size in bytes of this attribute
    * structure.
    */
/* 中文逐项-变量 V2812：size
 * size（uint16_t；成员 nvtxResourceAttributes_v0）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
    uint16_t size;

    /**
    * \brief Identifier type specifies how to interpret the identifier field
    *
    * Defines the identifier format of the attribute structure's \ref RESOURCE_IDENTIFIER_FIELD
    * "identifier" field.
    *
    * Default Value is NVTX_RESOURCE_TYPE_UNKNOWN
    */
/* 中文逐项-变量 V2813：identifierType
 * identifierType（int32_t；成员 nvtxResourceAttributes_v0）：NVTX资源标识union及类型，描述可被工具命名/关联的操作系统或CUDA资源。
 */
    int32_t identifierType;            /* values from enums following the pattern nvtxResource[name]Type_t */

    /**
    * \brief Identifier for the resource. 
    * \anchor RESOURCE_IDENTIFIER_FIELD
    *
    * An identifier may be a pointer or a handle to an OS or middleware API object.
    * The resource type will assist in avoiding collisions where handles values may collide.
    */
/* 中文逐项-变量 V2814：identifier
 * identifier（union identifier_t { const void* pValue; uint64_t ullValue; }；成员
 * nvtxResourceAttributes_v0）：NVTX资源标识union及类型，描述可被工具命名/关联的操作系统或CUDA资源。
 */
/* 中文逐项-类型 T0327：identifier_t
 * 类型职责：本文件的union类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：identifier、pValue、ullValue。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union identifier_t
    {
/* 中文逐项-变量 V2815：pValue
 * pValue（void；成员 nvtxResourceAttributes_v0::identifier_t）：NVTX事件payload的双精度、浮点、整型、64位或指针视图，读取前核对payloadType。
 */
        const void* pValue;
/* 中文逐项-变量 V2816：ullValue
 * ullValue（uint64_t；成员 nvtxResourceAttributes_v0::identifier_t）：NVTX事件payload的双精度、浮点、整型、64位或指针视图，读取前核对payloadType。
 */
        uint64_t ullValue;
    } identifier;

    /** \brief Message type specified in this attribute structure.
    *
    * Defines the message format of the attribute structure's \ref RESOURCE_MESSAGE_FIELD
    * "message" field.
    *
    * Default Value is NVTX_MESSAGE_UNKNOWN
    */
/* 中文逐项-变量 V2817：messageType
 * messageType（int32_t；成员 nvtxResourceAttributes_v0）：NVTX颜色/消息/载荷的union类型标签，读取值前先看对应标签。
 */
    int32_t messageType;            /* nvtxMessageType_t */

    /** \brief Message assigned to this attribute structure. \anchor RESOURCE_MESSAGE_FIELD
    *
    * The text message that is attached to a resource.
    */
/* 中文逐项-变量 V2818：message
 * message（nvtxMessageValue_t；成员 nvtxResourceAttributes_v0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
    nvtxMessageValue_t message;

} nvtxResourceAttributes_v0;

typedef struct nvtxResourceAttributes_v0 nvtxResourceAttributes_t;

/* \cond SHOW_HIDDEN 
* \version \NVTX_VERSION_2
*/
/* 中文逐项-常量 X0745：NVTX_RESOURCE_ATTRIB_STRUCT_SIZE
 * 编译期定义：NVTX_RESOURCE_ATTRIB_STRUCT_SIZE = ( (uint16_t)( sizeof(nvtxResourceAttributes_v0) ) )。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_RESOURCE_ATTRIB_STRUCT_SIZE ( (uint16_t)( sizeof(nvtxResourceAttributes_v0) ) )
typedef struct nvtxResourceHandle* nvtxResourceHandle_t;
/** \endcond */



/* ------------------------------------------------------------------------- */
/** \brief Create a resource object to track and associate data with OS and middleware objects
*
* Allows users to associate an API handle or pointer with a user-provided name.
* 
*
* \param domain - Domain to own the resource object
* \param attribs - Attributes to be associated with the resource
*
* \return A handle that represents the newly created resource object.
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("example domain");
* nvtxResourceAttributes_t attribs = {0};
* attribs.version = NVTX_VERSION;
* attribs.size = NVTX_RESOURCE_ATTRIB_STRUCT_SIZE;
* attribs.identifierType = NVTX_RESOURCE_TYPE_GENERIC_POINTER;
* attribs.identifier.pValue = (const void*)pMutex;
* attribs.messageType = NVTX_MESSAGE_TYPE_ASCII;
* attribs.message.ascii = "Single thread access to database.";
* nvtxResourceHandle_t handle = nvtxDomainResourceCreate(domain, attribs);
* \endcode
*
* \sa
* ::nvtxResourceAttributes_t
* ::nvtxDomainResourceDestroy
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0746：nvtxDomainResourceCreate
 * 接口职责：在NVTX域中登记一个可被工具识别的资源属性对象。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxResourceHandle_t NVTX_API nvtxDomainResourceCreate(nvtxDomainHandle_t domain,
 * nvtxResourceAttributes_t* attribs);
 */nvtxResourceHandle_t NVTX_API nvtxDomainResourceCreate(nvtxDomainHandle_t domain, nvtxResourceAttributes_t* attribs);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Destroy a resource object to track and associate data with OS and middleware objects
*
* Allows users to associate an API handle or pointer with a user-provided name.
*
* \param resource - Handle to the resource in which to operate.
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("example domain");
* nvtxResourceAttributes_t attribs = {0};
* attribs.version = NVTX_VERSION;
* attribs.size = NVTX_RESOURCE_ATTRIB_STRUCT_SIZE;
* attribs.identifierType = NVTX_RESOURCE_TYPE_GENERIC_POINTER;
* attribs.identifier.pValue = (const void*)pMutex;
* attribs.messageType = NVTX_MESSAGE_TYPE_ASCII;
* attribs.message.ascii = "Single thread access to database.";
* nvtxResourceHandle_t handle = nvtxDomainResourceCreate(domain, attribs);
* nvtxDomainResourceDestroy(handle);
* \endcode
*
* \sa
* ::nvtxDomainResourceCreate
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0747：nvtxDomainResourceDestroy
 * 接口职责：结束域内资源标记的生命周期。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainResourceDestroy(nvtxResourceHandle_t resource);
 */void NVTX_API nvtxDomainResourceDestroy(nvtxResourceHandle_t resource);
/** @} */


/** \name Functions for NVTX Category Naming*/

/* ------------------------------------------------------------------------- */
/**
* \brief Annotate an NVTX category used within a domain.
*
* Categories are used to group sets of events. Each category is identified
* through a unique ID and that ID is passed into any of the marker/range
* events to assign that event to a specific category. The nvtxDomainNameCategory
* function calls allow the user to assign a name to a category ID that is
* specific to the domain.
*
* nvtxDomainNameCategory(NULL, category, name) is equivalent to calling
* nvtxNameCategory(category, name).
*
* \param domain    - The domain of scoping the category.
* \param category  - The category ID to name.
* \param name      - The name of the category.
*
* \remarks The category names are tracked per domain.
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("example");
* nvtxDomainNameCategoryA(domain, 1, "Memory Allocation");
* nvtxDomainNameCategoryW(domain, 2, L"Memory Transfer");
* \endcode
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0748：nvtxDomainNameCategoryA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainNameCategoryA(nvtxDomainHandle_t domain, uint32_t category,
 * const char* name);
 */void NVTX_API nvtxDomainNameCategoryA(nvtxDomainHandle_t domain, uint32_t category, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0749：nvtxDomainNameCategoryW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainNameCategoryW(nvtxDomainHandle_t domain, uint32_t category,
 * const wchar_t* name);
 */void NVTX_API nvtxDomainNameCategoryW(nvtxDomainHandle_t domain, uint32_t category, const wchar_t* name);
/** @} */

/** \brief Annotate an NVTX category.
 *
 * Categories are used to group sets of events. Each category is identified
 * through a unique ID and that ID is passed into any of the marker/range
 * events to assign that event to a specific category. The nvtxNameCategory
 * function calls allow the user to assign a name to a category ID.
 *
 * \param category - The category ID to name.
 * \param name     - The name of the category.
 *
 * \remarks The category names are tracked per process.
 *
 * \par Example:
 * \code
 * nvtxNameCategory(1, "Memory Allocation");
 * nvtxNameCategory(2, "Memory Transfer");
 * nvtxNameCategory(3, "Memory Object Lifetime");
 * \endcode
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0750：nvtxNameCategoryA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCategoryA(uint32_t category, const char* name);
 */void NVTX_API nvtxNameCategoryA(uint32_t category, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0751：nvtxNameCategoryW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameCategoryW(uint32_t category, const wchar_t* name);
 */void NVTX_API nvtxNameCategoryW(uint32_t category, const wchar_t* name);
/** @} */

/** \name Functions for OS Threads Naming*/

/* ------------------------------------------------------------------------- */
/** \brief Annotate an OS thread.
 *
 * Allows the user to name an active thread of the current process. If an
 * invalid thread ID is provided or a thread ID from a different process is
 * used the behavior of the tool is implementation dependent.
 *
 * Tools expect thread ID to be a number that uniquely identifies the thread
 * at the time of the call. Note that a thread's ID can be reused after
 * it is destroyed. Tools may choose how to handle aliasing of thread IDs.
 *
 * POSIX pthread_t type returned by pthread_self() may not comply with these
 * expectations. Please use OS-specific thread ID instead of pthread_t.
 *
 * The thread name is associated to the default domain.  To support domains 
 * use resource objects via ::nvtxDomainResourceCreate.
 *
 * \param threadId - The ID of the thread to name.
 * \param name     - The name of the thread.
 *
 * \par Examples:
 * MS Windows:
 * \code
 * #include <windows.h>
 * nvtxNameOsThread(GetCurrentThreadId(), "Current thread");
 * nvtxNameOsThread(GetThreadId(SomeThreadHandle), "Other thread");
 * \endcode
 *
 * Android:
 * \code
 * #include <unistd.h>
 * nvtxNameOsThreadA(gettid(), "Current thread");
 * nvtxNameOsThreadA(getpid(), "Main thread");
 * \endcode
 *
 * Linux:
 * \code
 * #include <sys/syscall.h>
 * nvtxNameOsThreadA(syscall(SYS_gettid), "Current thread");
 * \endcode
 * \code
 * #include <unistd.h>
 * nvtxNameOsThreadA(getpid(), "Main thread");
 * \endcode
 *
 * OS X:
 * \code
 * #include <sys/syscall.h>
 * nvtxNameOsThreadA(syscall(SYS_thread_selfid), "Current thread");
 * \endcode
 * \code
 * #include <pthread.h>
 * __uint64_t id;
 * pthread_threadid_np(pthread_self(), &id);
 * nvtxNameOsThreadA(id, "Current thread");
 * pthread_threadid_np(somePThreadId, &id);
 * nvtxNameOsThreadA(id, "Other thread");
 * \endcode
 *
 * \version \NVTX_VERSION_1
 * @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0752：nvtxNameOsThreadA
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameOsThreadA(uint32_t threadId, const char* name);
 */void NVTX_API nvtxNameOsThreadA(uint32_t threadId, const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0753：nvtxNameOsThreadW
 * 接口职责：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxNameOsThreadW(uint32_t threadId, const wchar_t* name);
 */void NVTX_API nvtxNameOsThreadW(uint32_t threadId, const wchar_t* name);
/** @} */


/** @} */ /*END defgroup*/
/* ========================================================================= */
/** \defgroup STRING_REGISTRATION String Registration
*
* Registered strings are intended to increase performance by lowering instrumentation
* overhead.  String may be registered once and the handle may be passed in place of
* a string where an the APIs may allow.
*
* See \ref STRING_REGISTRATION for more details
*
* @{
*/

/* ------------------------------------------------------------------------- */
/** \brief Register a string.

* Registers an immutable string with NVTX. Once registered the pointer used
* to register the domain name can be used in nvtxEventAttributes_t
* \ref MESSAGE_FIELD. This allows NVTX implementation to skip copying the
* contents of the message on each event invocation.
*
* String registration is an optimization. It is recommended to use string
* registration if the string will be passed to an event many times.
*
* String are not unregistered, except that by unregistering the entire domain
*
* \param domain  - Domain handle. If NULL then the global domain is used.
* \param string    - A unique pointer to a sequence of characters.
*
* \return A handle representing the registered string.
*
* \par Example:
* \code
* nvtxDomainCreateA("com.nvidia.nvtx.example");
* nvtxStringHandle_t message = nvtxDomainRegisterStringA(domain, "registered string");
* nvtxEventAttributes_t eventAttrib = {0};
* eventAttrib.version = NVTX_VERSION;
* eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
* eventAttrib.messageType = NVTX_MESSAGE_TYPE_REGISTERED;
* eventAttrib.message.registered = message;
* \endcode
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0754：nvtxDomainRegisterStringA
 * 接口职责：注册域内可复用字符串并返回句柄，降低重复消息处理开销。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxStringHandle_t NVTX_API nvtxDomainRegisterStringA(nvtxDomainHandle_t domain,
 * const char* string);
 */nvtxStringHandle_t NVTX_API nvtxDomainRegisterStringA(nvtxDomainHandle_t domain, const char* string);
NVTX_DECLSPEC /* 中文逐项-接口 X0755：nvtxDomainRegisterStringW
 * 接口职责：注册域内可复用字符串并返回句柄，降低重复消息处理开销。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxStringHandle_t NVTX_API nvtxDomainRegisterStringW(nvtxDomainHandle_t domain,
 * const wchar_t* string);
 */nvtxStringHandle_t NVTX_API nvtxDomainRegisterStringW(nvtxDomainHandle_t domain, const wchar_t* string);
/** @} */

/** @} */ /*END defgroup*/
/* ========================================================================= */
/** \defgroup DOMAINS Domains
*
* Domains are used to group events to a developer defined scope. Middleware
* vendors may also scope their own events to avoid collisions with the
* the application developer's events, so that the application developer may
* inspect both parts and easily differentiate or filter them.  By default
* all events are scoped to a global domain where NULL is provided or when
* using APIs provided b versions of NVTX below v2
*
* Domains are intended to be typically long lived objects with the intention
* of logically separating events of large modules from each other such as
* middleware libraries from each other and the main application.
*
* See \ref DOMAINS for more details
*
* @{
*/

/* ------------------------------------------------------------------------- */
/** \brief Register a NVTX domain.
*
* Domains are used to scope annotations. All NVTX_VERSION_0 and NVTX_VERSION_1
* annotations are scoped to the global domain. The function nvtxDomainCreate
* creates a new named domain.
*
* Each domain maintains its own nvtxRangePush and nvtxRangePop stack.
*
* \param name - A unique string representing the domain.
*
* \return A handle representing the domain.
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("com.nvidia.nvtx.example");
*
* nvtxMarkA("nvtxMarkA to global domain");
*
* nvtxEventAttributes_t eventAttrib1 = {0};
* eventAttrib1.version = NVTX_VERSION;
* eventAttrib1.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
* eventAttrib1.message.ascii = "nvtxDomainMarkEx to global domain";
* nvtxDomainMarkEx(NULL, &eventAttrib1);
*
* nvtxEventAttributes_t eventAttrib2 = {0};
* eventAttrib2.version = NVTX_VERSION;
* eventAttrib2.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
* eventAttrib2.message.ascii = "nvtxDomainMarkEx to com.nvidia.nvtx.example";
* nvtxDomainMarkEx(domain, &eventAttrib2);
* nvtxDomainDestroy(domain);
* \endcode
*
* \sa
* ::nvtxDomainDestroy
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0756：nvtxDomainCreateA
 * 接口职责：创建带名字的NVTX标记域，隔离不同组件的事件/类别。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxDomainHandle_t NVTX_API nvtxDomainCreateA(const char* name);
 */nvtxDomainHandle_t NVTX_API nvtxDomainCreateA(const char* name);
NVTX_DECLSPEC /* 中文逐项-接口 X0757：nvtxDomainCreateW
 * 接口职责：创建带名字的NVTX标记域，隔离不同组件的事件/类别。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxDomainHandle_t NVTX_API nvtxDomainCreateW(const wchar_t* name);
 */nvtxDomainHandle_t NVTX_API nvtxDomainCreateW(const wchar_t* name);
/** @} */

/* ------------------------------------------------------------------------- */
/** \brief Unregister a NVTX domain.
*
* Unregisters the domain handle and frees all domain specific resources.
*
* \param domain    - the domain handle
*
* \par Example:
* \code
* nvtxDomainHandle_t domain = nvtxDomainCreateA("com.nvidia.nvtx.example");
* nvtxDomainDestroy(domain);
* \endcode
*
* \sa
* ::nvtxDomainCreateA
* ::nvtxDomainCreateW
*
* \version \NVTX_VERSION_2
* @{ */
NVTX_DECLSPEC /* 中文逐项-接口 X0758：nvtxDomainDestroy
 * 接口职责：释放NVTX域句柄及工具相关引用。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainDestroy(nvtxDomainHandle_t domain);
 */void NVTX_API nvtxDomainDestroy(nvtxDomainHandle_t domain);
/** @} */


/** @} */ /*END defgroup*/
/* ========================================================================= */
/** \cond SHOW_HIDDEN */

#ifdef UNICODE
/* 中文逐项-常量 X0759：nvtxMark
 * 编译期定义：nvtxMark = nvtxMarkW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxMark            nvtxMarkW
/* 中文逐项-常量 X0760：nvtxRangeStart
 * 编译期定义：nvtxRangeStart = nvtxRangeStartW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxRangeStart      nvtxRangeStartW
/* 中文逐项-常量 X0761：nvtxRangePush
 * 编译期定义：nvtxRangePush = nvtxRangePushW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxRangePush       nvtxRangePushW
/* 中文逐项-常量 X0762：nvtxNameCategory
 * 编译期定义：nvtxNameCategory = nvtxNameCategoryW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxNameCategory    nvtxNameCategoryW
/* 中文逐项-常量 X0763：nvtxNameOsThread
 * 编译期定义：nvtxNameOsThread = nvtxNameOsThreadW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxNameOsThread    nvtxNameOsThreadW
    /* NVTX_VERSION_2 */
/* 中文逐项-常量 X0764：nvtxDomainCreate
 * 编译期定义：nvtxDomainCreate = nvtxDomainCreateW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxDomainCreate         nvtxDomainCreateW
/* 中文逐项-常量 X0765：nvtxDomainRegisterString
 * 编译期定义：nvtxDomainRegisterString = nvtxDomainRegisterStringW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxDomainRegisterString nvtxDomainRegisterStringW
/* 中文逐项-常量 X0766：nvtxDomainNameCategory
 * 编译期定义：nvtxDomainNameCategory = nvtxDomainNameCategoryW。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxDomainNameCategory   nvtxDomainNameCategoryW
#else
/* 中文逐项-常量 X0767：nvtxMark
 * 编译期定义：nvtxMark = nvtxMarkA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxMark            nvtxMarkA
/* 中文逐项-常量 X0768：nvtxRangeStart
 * 编译期定义：nvtxRangeStart = nvtxRangeStartA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxRangeStart      nvtxRangeStartA
/* 中文逐项-常量 X0769：nvtxRangePush
 * 编译期定义：nvtxRangePush = nvtxRangePushA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxRangePush       nvtxRangePushA
/* 中文逐项-常量 X0770：nvtxNameCategory
 * 编译期定义：nvtxNameCategory = nvtxNameCategoryA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxNameCategory    nvtxNameCategoryA
/* 中文逐项-常量 X0771：nvtxNameOsThread
 * 编译期定义：nvtxNameOsThread = nvtxNameOsThreadA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxNameOsThread    nvtxNameOsThreadA
    /* NVTX_VERSION_2 */
/* 中文逐项-常量 X0772：nvtxDomainCreate
 * 编译期定义：nvtxDomainCreate = nvtxDomainCreateA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxDomainCreate         nvtxDomainCreateA
/* 中文逐项-常量 X0773：nvtxDomainRegisterString
 * 编译期定义：nvtxDomainRegisterString = nvtxDomainRegisterStringA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxDomainRegisterString nvtxDomainRegisterStringA
/* 中文逐项-常量 X0774：nvtxDomainNameCategory
 * 编译期定义：nvtxDomainNameCategory = nvtxDomainNameCategoryA。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define nvtxDomainNameCategory   nvtxDomainNameCategoryA
#endif

/** \endcond */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

/* 中文逐项-常量 X0775：NVTX_IMPL_GUARD
 * 编译期定义：NVTX_IMPL_GUARD = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_IMPL_GUARD /* Ensure other headers cannot included directly */

#include "nvtxDetail/nvtxTypes.h"

#ifndef NVTX_NO_IMPL
#include "nvtxDetail/nvtxImpl.h"
#endif /*NVTX_NO_IMPL*/

#undef NVTX_IMPL_GUARD

#endif /* !defined(NVTX_VERSION) */
