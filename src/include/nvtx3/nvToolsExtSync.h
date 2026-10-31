/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#include "nvToolsExt.h"

#ifndef NVTOOLSEXT_SYNC_V3
/* 中文逐项-常量 X0855：NVTOOLSEXT_SYNC_V3
 * 编译期定义：NVTOOLSEXT_SYNC_V3 = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTOOLSEXT_SYNC_V3

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* \cond SHOW_HIDDEN 
* \version \NVTX_VERSION_2
*/
/* 中文逐项-常量 X0856：NVTX_SYNCUSER_ATTRIB_STRUCT_SIZE
 * 编译期定义：NVTX_SYNCUSER_ATTRIB_STRUCT_SIZE = ( (uint16_t)( sizeof(nvtxSyncUserAttributes_v0) ) )。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_SYNCUSER_ATTRIB_STRUCT_SIZE ( (uint16_t)( sizeof(nvtxSyncUserAttributes_v0) ) )
/** \endcond */


/** 
* \page PAGE_SYNCHRONIZATION Synchronization
*
* This section covers a subset of the API that allow users to track additional
* synchronization details of their application.   Naming OS synchronization primitives 
* may allow users to better understand the data collected by traced synchronization 
* APIs.  Additionally, a user defined synchronization object can allow the users to
* to tell the tools when the user is building their own synchronization system
* that do not rely on the OS to provide behaviors and instead use techniques like
* atomic operations and spinlocks.  
*
* See module \ref SYNCHRONIZATION for details.
*
* \par Example:
* \code
* class MyMutex
* {
*     volatile long bLocked;
*     nvtxSyncUser_t hSync;
* public:
*     MyMutex(const char* name, nvtxDomainHandle_t d){
*          bLocked = 0;
*
*          nvtxSyncUserAttributes_t attribs = { 0 };
*          attribs.version = NVTX_VERSION;
*          attribs.size = NVTX_SYNCUSER_ATTRIB_STRUCT_SIZE;
*          attribs.messageType = NVTX_MESSAGE_TYPE_ASCII;
*          attribs.message.ascii = name;
*          hSync = nvtxDomainSyncUserCreate(d, &attribs);
*     }
*
*     ~MyMutex() {
*          nvtxDomainSyncUserDestroy(hSync);
*     }
*
*     bool Lock() {
*          nvtxDomainSyncUserAcquireStart(hSync);
*          bool acquired = __sync_bool_compare_and_swap(&bLocked, 0, 1);//atomic compiler intrinsic 

*          if (acquired) {
*              nvtxDomainSyncUserAcquireSuccess(hSync);
*          }
*          else {
*              nvtxDomainSyncUserAcquireFailed(hSync);
*          }
*          return acquired;
*     }

*     void Unlock() {
*          nvtxDomainSyncUserReleasing(hSync);
*          bLocked = false;
*     }
* };
* \endcode
* 
* \version \NVTX_VERSION_2
*/

/*  ------------------------------------------------------------------------- */
/* \cond SHOW_HIDDEN 
* \brief Used to build a non-colliding value for resource types separated class
* \version \NVTX_VERSION_2
*/
/* 中文逐项-常量 X0857：NVTX_RESOURCE_CLASS_SYNC_OS
 * 编译期定义：NVTX_RESOURCE_CLASS_SYNC_OS = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_RESOURCE_CLASS_SYNC_OS 2 /**< Synchronization objects that are OS specific. */
/* 中文逐项-常量 X0858：NVTX_RESOURCE_CLASS_SYNC_PTHREAD
 * 编译期定义：NVTX_RESOURCE_CLASS_SYNC_PTHREAD = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_RESOURCE_CLASS_SYNC_PTHREAD 3 /**< Synchronization objects that are from the POSIX Threads API (pthread)*/
/** \endcond */


/*  ------------------------------------------------------------------------- */
/** \defgroup SYNCHRONIZATION Synchronization
* See page \ref PAGE_SYNCHRONIZATION.
* @{
*/

/** \brief Resource type values for OSs with POSIX Thread API support
 */
typedef /* 中文逐项-类型 T0331：nvtxResourceSyncPosixThreadType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxResourceSyncPosixThreadType_t
{
/* 中文逐项-枚举项 X0859：NVTX_RESOURCE_TYPE_SYNC_PTHREAD_MUTEX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 1)
 */
    NVTX_RESOURCE_TYPE_SYNC_PTHREAD_MUTEX = NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 1), /* pthread_mutex_t  */
/* 中文逐项-枚举项 X0860：NVTX_RESOURCE_TYPE_SYNC_PTHREAD_CONDITION
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 2)
 */
    NVTX_RESOURCE_TYPE_SYNC_PTHREAD_CONDITION = NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 2), /* pthread_cond_t  */
/* 中文逐项-枚举项 X0861：NVTX_RESOURCE_TYPE_SYNC_PTHREAD_RWLOCK
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 3)
 */
    NVTX_RESOURCE_TYPE_SYNC_PTHREAD_RWLOCK = NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 3), /* pthread_rwlock_t  */
/* 中文逐项-枚举项 X0862：NVTX_RESOURCE_TYPE_SYNC_PTHREAD_BARRIER
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 4)
 */
    NVTX_RESOURCE_TYPE_SYNC_PTHREAD_BARRIER = NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 4), /* pthread_barrier_t  */
/* 中文逐项-枚举项 X0863：NVTX_RESOURCE_TYPE_SYNC_PTHREAD_SPINLOCK
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 5)
 */
    NVTX_RESOURCE_TYPE_SYNC_PTHREAD_SPINLOCK = NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 5), /* pthread_spinlock_t  */
/* 中文逐项-枚举项 X0864：NVTX_RESOURCE_TYPE_SYNC_PTHREAD_ONCE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 6)
 */
    NVTX_RESOURCE_TYPE_SYNC_PTHREAD_ONCE = NVTX_RESOURCE_MAKE_TYPE(SYNC_PTHREAD, 6) /* pthread_once_t  */
} nvtxResourceSyncPosixThreadType_t;

/** \brief Resource type values for Windows OSs
*/
typedef /* 中文逐项-类型 T0332：nvtxResourceSyncWindowsType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxResourceSyncWindowsType_t
{
/* 中文逐项-枚举项 X0865：NVTX_RESOURCE_TYPE_SYNC_WINDOWS_MUTEX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 1)
 */
    NVTX_RESOURCE_TYPE_SYNC_WINDOWS_MUTEX = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 1),
/* 中文逐项-枚举项 X0866：NVTX_RESOURCE_TYPE_SYNC_WINDOWS_SEMAPHORE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 2)
 */
    NVTX_RESOURCE_TYPE_SYNC_WINDOWS_SEMAPHORE = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 2),
/* 中文逐项-枚举项 X0867：NVTX_RESOURCE_TYPE_SYNC_WINDOWS_EVENT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 3)
 */
    NVTX_RESOURCE_TYPE_SYNC_WINDOWS_EVENT = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 3),
/* 中文逐项-枚举项 X0868：NVTX_RESOURCE_TYPE_SYNC_WINDOWS_CRITICAL_SECTION
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 4)
 */
    NVTX_RESOURCE_TYPE_SYNC_WINDOWS_CRITICAL_SECTION = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 4),
/* 中文逐项-枚举项 X0869：NVTX_RESOURCE_TYPE_SYNC_WINDOWS_SRWLOCK
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 5)
 */
    NVTX_RESOURCE_TYPE_SYNC_WINDOWS_SRWLOCK = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 5)
} nvtxResourceSyncWindowsType_t;

/** \brief Resource type values for Linux and Linux derived OSs such as Android
* \sa
* ::nvtxResourceSyncPosixThreadType_t
*/
typedef /* 中文逐项-类型 T0333：nvtxResourceSyncLinuxType_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvtxResourceSyncLinuxType_t
{
/* 中文逐项-枚举项 X0870：NVTX_RESOURCE_TYPE_SYNC_LINUX_MUTEX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 1)
 */
    NVTX_RESOURCE_TYPE_SYNC_LINUX_MUTEX = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 1),
/* 中文逐项-枚举项 X0871：NVTX_RESOURCE_TYPE_SYNC_LINUX_FUTEX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 2)
 */
    NVTX_RESOURCE_TYPE_SYNC_LINUX_FUTEX = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 2),
/* 中文逐项-枚举项 X0872：NVTX_RESOURCE_TYPE_SYNC_LINUX_SEMAPHORE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 3)
 */
    NVTX_RESOURCE_TYPE_SYNC_LINUX_SEMAPHORE = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 3),
/* 中文逐项-枚举项 X0873：NVTX_RESOURCE_TYPE_SYNC_LINUX_COMPLETION
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 4)
 */
    NVTX_RESOURCE_TYPE_SYNC_LINUX_COMPLETION = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 4),
/* 中文逐项-枚举项 X0874：NVTX_RESOURCE_TYPE_SYNC_LINUX_SPINLOCK
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 5)
 */
    NVTX_RESOURCE_TYPE_SYNC_LINUX_SPINLOCK = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 5),
/* 中文逐项-枚举项 X0875：NVTX_RESOURCE_TYPE_SYNC_LINUX_SEQLOCK
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 6)
 */
    NVTX_RESOURCE_TYPE_SYNC_LINUX_SEQLOCK = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 6),
/* 中文逐项-枚举项 X0876：NVTX_RESOURCE_TYPE_SYNC_LINUX_RCU
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 7)
 */
    NVTX_RESOURCE_TYPE_SYNC_LINUX_RCU = NVTX_RESOURCE_MAKE_TYPE(SYNC_OS, 7)
} nvtxResourceSyncLinuxType_t;

/** \brief Resource type values for Android come from Linux.
* \sa
* ::nvtxResourceSyncLinuxType_t
* ::nvtxResourceSyncPosixThreadType_t
*/
typedef enum nvtxResourceSyncLinuxType_t nvtxResourceSyncAndroidType_t;

/** \brief User Defined Synchronization Object Handle .
* \anchor SYNCUSER_HANDLE_STRUCTURE
*
* This structure is opaque to the user and is used as a handle to reference
* a user defined syncrhonization object.  The tools will return a pointer through the API for the application
* to hold on it's behalf to reference the string in the future.
*
*/
typedef struct nvtxSyncUser* nvtxSyncUser_t;

/** \brief User Defined Synchronization Object Attributes Structure.
* \anchor USERDEF_SYNC_ATTRIBUTES_STRUCTURE
*
* This structure is used to describe the attributes of a user defined synchronization 
* object.  The layout of the structure is defined by a specific version of the tools 
* extension library and can change between different versions of the Tools Extension
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
* nvtxSyncUserAttributes_t attribs = {0};
* attribs.version = NVTX_VERSION;
* attribs.size = NVTX_SYNCUSER_ATTRIB_STRUCT_SIZE;
* \endcode
*
* \par Method 2: Initializing nvtxSyncUserAttributes_t for a specific version
* \code
* nvtxSyncUserAttributes_t attribs = {0};
* attribs.version = 1;
* attribs.size = (uint16_t)(sizeof(nvtxSyncUserAttributes_t));
* \endcode
*
* If the caller uses Method 1 it is critical that the entire binary
* layout of the structure be configured to 0 so that all fields
* are initialized to the default value.
*
* The caller should either use both NVTX_VERSION and
* NVTX_SYNCUSER_ATTRIB_STRUCT_SIZE (Method 1) or use explicit values
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
* nvtxSyncUserAttributes_t attribs = {0};
* attribs.version = NVTX_VERSION;
* attribs.size = NVTX_SYNCUSER_ATTRIB_STRUCT_SIZE;
*
* // Configure the Attributes
* attribs.messageType = NVTX_MESSAGE_TYPE_ASCII;
* attribs.message.ascii = "Example";
* \endcode
*
* \sa
* ::nvtxDomainSyncUserCreate
*/
typedef /* 中文逐项-类型 T0334：nvtxSyncUserAttributes_v0
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：version、size、messageType、message。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct nvtxSyncUserAttributes_v0
{
    /**
    * \brief Version flag of the structure.
    *
    * Needs to be set to NVTX_VERSION to indicate the version of NVTX APIs
    * supported in this header file. This can optionally be overridden to
    * another version of the tools extension library.
    */
/* 中文逐项-变量 V2819：version
 * version（uint16_t；成员 nvtxSyncUserAttributes_v0）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 */
    uint16_t version;

    /**
    * \brief Size of the structure.
    *
    * Needs to be set to the size in bytes of the event attribute
    * structure used to specify the event.
    */
/* 中文逐项-变量 V2820：size
 * size（uint16_t；成员 nvtxSyncUserAttributes_v0）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
    uint16_t size;

    /** \brief Message type specified in this attribute structure.
    *
    * Defines the message format of the attribute structure's \ref nvtxSyncUserAttributes_v0::message
    * "message" field.
    *
    * Default Value is NVTX_MESSAGE_UNKNOWN
    */
/* 中文逐项-变量 V2821：messageType
 * messageType（int32_t；成员 nvtxSyncUserAttributes_v0）：NVTX颜色/消息/载荷的union类型标签，读取值前先看对应标签。
 */
    int32_t messageType;            /* nvtxMessageType_t */

    /** \brief Message assigned to this attribute structure.
    *
    * The text message that is attached to an event.
    */
/* 中文逐项-变量 V2822：message
 * message（nvtxMessageValue_t；成员 nvtxSyncUserAttributes_v0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
    nvtxMessageValue_t message;

} nvtxSyncUserAttributes_v0;

typedef struct nvtxSyncUserAttributes_v0 nvtxSyncUserAttributes_t;

/* ------------------------------------------------------------------------- */
/** \brief Create a user defined synchronization object 
* This is used to track non-OS synchronization working with spinlocks and atomics
*
* \param domain - Domain to own the resource
* \param attribs - A structure to assign multiple attributes to the object.
*
* \return A handle that represents the newly created user defined synchronization object.
*
* \sa
* ::nvtxDomainSyncUserCreate
* ::nvtxDomainSyncUserDestroy
* ::nvtxDomainSyncUserAcquireStart
* ::nvtxDomainSyncUserAcquireFailed
* ::nvtxDomainSyncUserAcquireSuccess
* ::nvtxDomainSyncUserReleasing
*
* \version \NVTX_VERSION_2
*/
NVTX_DECLSPEC /* 中文逐项-接口 X0877：nvtxDomainSyncUserCreate
 * 接口职责：创建带属性的用户同步标记对象供性能工具追踪。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：nvtxSyncUser_t NVTX_API nvtxDomainSyncUserCreate(nvtxDomainHandle_t domain, const
 * nvtxSyncUserAttributes_t* attribs);
 */nvtxSyncUser_t NVTX_API nvtxDomainSyncUserCreate(nvtxDomainHandle_t domain, const nvtxSyncUserAttributes_t* attribs);

/* ------------------------------------------------------------------------- */
/** \brief Destroy a user defined synchronization object
* This is used to track non-OS synchronization working with spinlocks and atomics
*
* \param handle - A handle to the object to operate on.
*
* \sa
* ::nvtxDomainSyncUserCreate
* ::nvtxDomainSyncUserDestroy
* ::nvtxDomainSyncUserAcquireStart
* ::nvtxDomainSyncUserAcquireFailed
* ::nvtxDomainSyncUserAcquireSuccess
* ::nvtxDomainSyncUserReleasing
*
* \version \NVTX_VERSION_2
*/
NVTX_DECLSPEC /* 中文逐项-接口 X0878：nvtxDomainSyncUserDestroy
 * 接口职责：结束用户同步标记对象的工具生命周期。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainSyncUserDestroy(nvtxSyncUser_t handle);
 */void NVTX_API nvtxDomainSyncUserDestroy(nvtxSyncUser_t handle);

/* ------------------------------------------------------------------------- */
/** \brief Signal to tools that an attempt to acquire a user defined synchronization object
*
* \param handle - A handle to the object to operate on.
*
* \sa
* ::nvtxDomainSyncUserCreate
* ::nvtxDomainSyncUserDestroy
* ::nvtxDomainSyncUserAcquireStart
* ::nvtxDomainSyncUserAcquireFailed
* ::nvtxDomainSyncUserAcquireSuccess
* ::nvtxDomainSyncUserReleasing
*
* \version \NVTX_VERSION_2
*/
NVTX_DECLSPEC /* 中文逐项-接口 X0879：nvtxDomainSyncUserAcquireStart
 * 接口职责：报告用户同步对象的一次获取尝试开始。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainSyncUserAcquireStart(nvtxSyncUser_t handle);
 */void NVTX_API nvtxDomainSyncUserAcquireStart(nvtxSyncUser_t handle);

/* ------------------------------------------------------------------------- */
/** \brief Signal to tools of failure in acquiring a user defined synchronization object
* This should be called after \ref nvtxDomainSyncUserAcquireStart
* 
* \param handle - A handle to the object to operate on.
*
* \sa
* ::nvtxDomainSyncUserCreate
* ::nvtxDomainSyncUserDestroy
* ::nvtxDomainSyncUserAcquireStart
* ::nvtxDomainSyncUserAcquireFailed
* ::nvtxDomainSyncUserAcquireSuccess
* ::nvtxDomainSyncUserReleasing
*
* \version \NVTX_VERSION_2
*/NVTX_DECLSPEC /* 中文逐项-接口 X0880：nvtxDomainSyncUserAcquireFailed
 * 接口职责：报告用户同步对象获取失败。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainSyncUserAcquireFailed(nvtxSyncUser_t handle);
 */void NVTX_API nvtxDomainSyncUserAcquireFailed(nvtxSyncUser_t handle);

/* ------------------------------------------------------------------------- */
/** \brief Signal to tools of success in acquiring a user defined synchronization object
* This should be called after \ref nvtxDomainSyncUserAcquireStart.
*
* \param handle - A handle to the object to operate on.
*
* \sa
* ::nvtxDomainSyncUserCreate
* ::nvtxDomainSyncUserDestroy
* ::nvtxDomainSyncUserAcquireStart
* ::nvtxDomainSyncUserAcquireFailed
* ::nvtxDomainSyncUserAcquireSuccess
* ::nvtxDomainSyncUserReleasing
*
* \version \NVTX_VERSION_2
*/NVTX_DECLSPEC /* 中文逐项-接口 X0881：nvtxDomainSyncUserAcquireSuccess
 * 接口职责：报告用户同步对象获取成功。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainSyncUserAcquireSuccess(nvtxSyncUser_t handle);
 */void NVTX_API nvtxDomainSyncUserAcquireSuccess(nvtxSyncUser_t handle);

/* ------------------------------------------------------------------------- */
/** \brief Signal to tools of releasing a reservation on user defined synchronization object
* This should be called after \ref nvtxDomainSyncUserAcquireSuccess.
*
* \param handle - A handle to the object to operate on.
*
* \sa
* ::nvtxDomainSyncUserCreate
* ::nvtxDomainSyncUserDestroy
* ::nvtxDomainSyncUserAcquireStart
* ::nvtxDomainSyncUserAcquireFailed
* ::nvtxDomainSyncUserAcquireSuccess
* ::nvtxDomainSyncUserReleasing
*
* \version \NVTX_VERSION_2
*/
NVTX_DECLSPEC /* 中文逐项-接口 X0882：nvtxDomainSyncUserReleasing
 * 接口职责：报告用户同步对象即将释放。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void NVTX_API nvtxDomainSyncUserReleasing(nvtxSyncUser_t handle);
 */void NVTX_API nvtxDomainSyncUserReleasing(nvtxSyncUser_t handle);


/** @} */ /*END defgroup*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifndef NVTX_NO_IMPL
/* 中文逐项-常量 X0883：NVTX_IMPL_GUARD_SYNC
 * 编译期定义：NVTX_IMPL_GUARD_SYNC = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_IMPL_GUARD_SYNC /* Ensure other headers cannot included directly */
#include "nvtxDetail/nvtxImplSync_v3.h"
#undef NVTX_IMPL_GUARD_SYNC
#endif /*NVTX_NO_IMPL*/

#endif /* NVTOOLSEXT_SYNC_V3 */
