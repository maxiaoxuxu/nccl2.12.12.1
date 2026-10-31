/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#ifndef NVTX_IMPL_GUARD_SYNC
#error Never include this file directly -- it is automatically included by nvToolsExtCuda.h (except when NVTX_NO_IMPL is defined).
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef nvtxSyncUser_t (NVTX_API * nvtxDomainSyncUserCreate_impl_fntype)(nvtxDomainHandle_t domain, const nvtxSyncUserAttributes_t* attribs);
typedef void (NVTX_API * nvtxDomainSyncUserDestroy_impl_fntype)(nvtxSyncUser_t handle);
typedef void (NVTX_API * nvtxDomainSyncUserAcquireStart_impl_fntype)(nvtxSyncUser_t handle);
typedef void (NVTX_API * nvtxDomainSyncUserAcquireFailed_impl_fntype)(nvtxSyncUser_t handle);
typedef void (NVTX_API * nvtxDomainSyncUserAcquireSuccess_impl_fntype)(nvtxSyncUser_t handle);
typedef void (NVTX_API * nvtxDomainSyncUserReleasing_impl_fntype)(nvtxSyncUser_t handle);

NVTX_DECLSPEC /* 中文内部逻辑 F0736：nvtxDomainSyncUserCreate
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainSyncUserCreate_impl_fntype local = (nvtxDomainSyncUserCreate_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(domain, attribs);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxSyncUser_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplSync_v3_h.md#f0736
 */
/* 中文逐项-函数 F0736：nvtxDomainSyncUserCreate
 * 用途与位置：创建带属性的用户同步标记对象供性能工具追踪。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 attribs（const nvtxSyncUserAttributes_t* attribs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(domain, attribs);；return (nvtxSyncUser_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxSyncUser_t NVTX_API nvtxDomainSyncUserCreate(nvtxDomainHandle_t domain, const nvtxSyncUserAttributes_t* attribs)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2958：local
 * local（nvtxDomainSyncUserCreate_impl_fntype；函数 nvtxDomainSyncUserCreate 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxDomainSyncUserCreate_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainSyncUserCreate_impl_fntype local = (nvtxDomainSyncUserCreate_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr;
    if(local!=0)
        return (*local)(domain, attribs);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxSyncUser_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0737：nvtxDomainSyncUserDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainSyncUserDestroy_impl_fntype local = (nvtxDomainSyncUserDestroy_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(handle);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplSync_v3_h.md#f0737
 */
/* 中文逐项-函数 F0737：nvtxDomainSyncUserDestroy
 * 用途与位置：结束用户同步标记对象的工具生命周期。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainSyncUserDestroy(nvtxSyncUser_t handle)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2959：local
 * local（nvtxDomainSyncUserDestroy_impl_fntype；函数 nvtxDomainSyncUserDestroy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxDomainSyncUserDestroy_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainSyncUserDestroy_impl_fntype local = (nvtxDomainSyncUserDestroy_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr;
    if(local!=0)
        (*local)(handle);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0738：nvtxDomainSyncUserAcquireStart
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainSyncUserAcquireStart_impl_fntype local = (nvtxDomainSyncUserAcquireStart_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(handle);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplSync_v3_h.md#f0738
 */
/* 中文逐项-函数 F0738：nvtxDomainSyncUserAcquireStart
 * 用途与位置：报告用户同步对象的一次获取尝试开始。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainSyncUserAcquireStart(nvtxSyncUser_t handle)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2960：local
 * local（nvtxDomainSyncUserAcquireStart_impl_fntype；函数 nvtxDomainSyncUserAcquireStart
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxDomainSyncUserAcquireStart_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainSyncUserAcquireStart_impl_fntype local = (nvtxDomainSyncUserAcquireStart_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr;
    if(local!=0)
        (*local)(handle);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0739：nvtxDomainSyncUserAcquireFailed
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainSyncUserAcquireFailed_impl_fntype local = (nvtxDomainSyncUserAcquireFailed_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(handle);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplSync_v3_h.md#f0739
 */
/* 中文逐项-函数 F0739：nvtxDomainSyncUserAcquireFailed
 * 用途与位置：报告用户同步对象获取失败。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainSyncUserAcquireFailed(nvtxSyncUser_t handle)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2961：local
 * local（nvtxDomainSyncUserAcquireFailed_impl_fntype；函数 nvtxDomainSyncUserAcquireFailed
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxDomainSyncUserAcquireFailed_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainSyncUserAcquireFailed_impl_fntype local = (nvtxDomainSyncUserAcquireFailed_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr;
    if(local!=0)
        (*local)(handle);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0740：nvtxDomainSyncUserAcquireSuccess
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainSyncUserAcquireSuccess_impl_fntype local = (nvtxDomainSyncUserAcquireSuccess_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(handle);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplSync_v3_h.md#f0740
 */
/* 中文逐项-函数 F0740：nvtxDomainSyncUserAcquireSuccess
 * 用途与位置：报告用户同步对象获取成功。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainSyncUserAcquireSuccess(nvtxSyncUser_t handle)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2962：local
 * local（nvtxDomainSyncUserAcquireSuccess_impl_fntype；函数 nvtxDomainSyncUserAcquireSuccess
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxDomainSyncUserAcquireSuccess_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainSyncUserAcquireSuccess_impl_fntype local = (nvtxDomainSyncUserAcquireSuccess_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr;
    if(local!=0)
        (*local)(handle);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0741：nvtxDomainSyncUserReleasing
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainSyncUserReleasing_impl_fntype local = (nvtxDomainSyncUserReleasing_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(handle);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplSync_v3_h.md#f0741
 */
/* 中文逐项-函数 F0741：nvtxDomainSyncUserReleasing
 * 用途与位置：报告用户同步对象即将释放。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 handle（nvtxSyncUser_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainSyncUserReleasing(nvtxSyncUser_t handle)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2963：local
 * local（nvtxDomainSyncUserReleasing_impl_fntype；函数 nvtxDomainSyncUserReleasing
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * (nvtxDomainSyncUserReleasing_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainSyncUserReleasing_impl_fntype local = (nvtxDomainSyncUserReleasing_impl_fntype)NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr;
    if(local!=0)
        (*local)(handle);
#endif /*NVTX_DISABLE*/
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
