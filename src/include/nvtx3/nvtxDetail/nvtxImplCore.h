/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

NVTX_DECLSPEC /* 中文内部逻辑 F0678：nvtxMarkEx
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxMarkEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(eventAttrib);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0678
 */
/* 中文逐项-函数 F0678：nvtxMarkEx
 * 用途与位置：向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxMarkEx_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxMarkEx(const nvtxEventAttributes_t* eventAttrib)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2900：local
 * local（nvtxMarkEx_impl_fntype；函数 nvtxMarkEx 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxMarkEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr;
    if(local!=0)
        (*local)(eventAttrib);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0679：nvtxMarkA
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxMarkA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(message);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0679
 */
/* 中文逐项-函数 F0679：nvtxMarkA
 * 用途与位置：向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const char* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxMarkA_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxMarkA(const char* message)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2901：local
 * local（nvtxMarkA_impl_fntype；函数 nvtxMarkA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxMarkA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr;
    if(local!=0)
        (*local)(message);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0680：nvtxMarkW
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxMarkW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(message);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0680
 */
/* 中文逐项-函数 F0680：nvtxMarkW
 * 用途与位置：向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const wchar_t* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxMarkW_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxMarkW(const wchar_t* message)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2902：local
 * local（nvtxMarkW_impl_fntype；函数 nvtxMarkW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxMarkW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr;
    if(local!=0)
        (*local)(message);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0681：nvtxRangeStartEx
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxRangeStartEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(eventAttrib);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxRangeId_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0681
 */
/* 中文逐项-函数 F0681：nvtxRangeStartEx
 * 用途与位置：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxRangeStartEx_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(eventAttrib);；return (nvtxRangeId_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxRangeId_t NVTX_API nvtxRangeStartEx(const nvtxEventAttributes_t* eventAttrib)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2903：local
 * local（nvtxRangeStartEx_impl_fntype；函数 nvtxRangeStartEx 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxRangeStartEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr;
    if(local!=0)
        return (*local)(eventAttrib);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxRangeId_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0682：nvtxRangeStartA
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxRangeStartA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(message);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxRangeId_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0682
 */
/* 中文逐项-函数 F0682：nvtxRangeStartA
 * 用途与位置：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const char* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxRangeStartA_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(message);；return (nvtxRangeId_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxRangeId_t NVTX_API nvtxRangeStartA(const char* message)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2904：local
 * local（nvtxRangeStartA_impl_fntype；函数 nvtxRangeStartA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxRangeStartA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr;
    if(local!=0)
        return (*local)(message);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxRangeId_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0683：nvtxRangeStartW
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxRangeStartW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(message);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxRangeId_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0683
 */
/* 中文逐项-函数 F0683：nvtxRangeStartW
 * 用途与位置：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const wchar_t* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxRangeStartW_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(message);；return (nvtxRangeId_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxRangeId_t NVTX_API nvtxRangeStartW(const wchar_t* message)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2905：local
 * local（nvtxRangeStartW_impl_fntype；函数 nvtxRangeStartW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxRangeStartW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr;
    if(local!=0)
        return (*local)(message);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxRangeId_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0684：nvtxRangeEnd
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxRangeEnd_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(id);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0684
 */
/* 中文逐项-函数 F0684：nvtxRangeEnd
 * 用途与位置：按id结束显式NVTX时间范围。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 id（nvtxRangeId_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxRangeEnd_impl_init；src/include/nvtx3.hpp::end_range。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxRangeEnd(nvtxRangeId_t id)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2906：local
 * local（nvtxRangeEnd_impl_fntype；函数 nvtxRangeEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxRangeEnd_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr;
    if(local!=0)
        (*local)(id);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0685：nvtxRangePushEx
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxRangePushEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(eventAttrib);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (int)NVTX_NO_PUSH_POP_TRACKING;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0685
 */
/* 中文逐项-函数 F0685：nvtxRangePushEx
 * 用途与位置：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxRangePushEx_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(eventAttrib);；return (int)NVTX_NO_PUSH_POP_TRACKING;。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API nvtxRangePushEx(const nvtxEventAttributes_t* eventAttrib)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2907：local
 * local（nvtxRangePushEx_impl_fntype；函数 nvtxRangePushEx 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxRangePushEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr;
    if(local!=0)
        return (*local)(eventAttrib);
    else
#endif  /*NVTX_DISABLE*/
        return (int)NVTX_NO_PUSH_POP_TRACKING;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0686：nvtxRangePushA
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxRangePushA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(message);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (int)NVTX_NO_PUSH_POP_TRACKING;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0686
 */
/* 中文逐项-函数 F0686：nvtxRangePushA
 * 用途与位置：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const char* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxRangePushA_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(message);；return (int)NVTX_NO_PUSH_POP_TRACKING;。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API nvtxRangePushA(const char* message)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2908：local
 * local（nvtxRangePushA_impl_fntype；函数 nvtxRangePushA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxRangePushA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr;
    if(local!=0)
        return (*local)(message);
    else
#endif  /*NVTX_DISABLE*/
        return (int)NVTX_NO_PUSH_POP_TRACKING;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0687：nvtxRangePushW
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxRangePushW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(message);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (int)NVTX_NO_PUSH_POP_TRACKING;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0687
 */
/* 中文逐项-函数 F0687：nvtxRangePushW
 * 用途与位置：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const wchar_t* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxRangePushW_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(message);；return (int)NVTX_NO_PUSH_POP_TRACKING;。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API nvtxRangePushW(const wchar_t* message)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2909：local
 * local（nvtxRangePushW_impl_fntype；函数 nvtxRangePushW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxRangePushW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr;
    if(local!=0)
        return (*local)(message);
    else
#endif  /*NVTX_DISABLE*/
        return (int)NVTX_NO_PUSH_POP_TRACKING;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0688：nvtxRangePop
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxRangePop_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)();；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (int)NVTX_NO_PUSH_POP_TRACKING;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0688
 */
/* 中文逐项-函数 F0688：nvtxRangePop
 * 用途与位置：从当前线程NVTX范围栈pop结束最近区间。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxRangePop_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)();；return (int)NVTX_NO_PUSH_POP_TRACKING;。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API nvtxRangePop(void)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2910：local
 * local（nvtxRangePop_impl_fntype；函数 nvtxRangePop 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxRangePop_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr;
    if(local!=0)
        return (*local)();
    else
#endif  /*NVTX_DISABLE*/
        return (int)NVTX_NO_PUSH_POP_TRACKING;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0689：nvtxNameCategoryA
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxNameCategoryA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(category, name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0689
 */
/* 中文逐项-函数 F0689：nvtxNameCategoryA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 category（uint32_t category）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxNameCategoryA_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCategoryA(uint32_t category, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2911：local
 * local（nvtxNameCategoryA_impl_fntype；函数 nvtxNameCategoryA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCategoryA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr;
    if(local!=0)
        (*local)(category, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0690：nvtxNameCategoryW
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxNameCategoryW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(category, name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0690
 */
/* 中文逐项-函数 F0690：nvtxNameCategoryW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 category（uint32_t category）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxNameCategoryW_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameCategoryW(uint32_t category, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2912：local
 * local（nvtxNameCategoryW_impl_fntype；函数 nvtxNameCategoryW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameCategoryW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr;
    if(local!=0)
        (*local)(category, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0691：nvtxNameOsThreadA
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxNameOsThreadA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(threadId, name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0691
 */
/* 中文逐项-函数 F0691：nvtxNameOsThreadA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 threadId（uint32_t threadId）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxNameOsThreadA_impl_init；src/proxy.cc::ncclProxyProgress。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameOsThreadA(uint32_t threadId, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2913：local
 * local（nvtxNameOsThreadA_impl_fntype；函数 nvtxNameOsThreadA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameOsThreadA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr;
    if(local!=0)
        (*local)(threadId, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0692：nvtxNameOsThreadW
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxNameOsThreadW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(threadId, name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0692
 */
/* 中文逐项-函数 F0692：nvtxNameOsThreadW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 threadId（uint32_t threadId）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxNameOsThreadW_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxNameOsThreadW(uint32_t threadId, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2914：local
 * local（nvtxNameOsThreadW_impl_fntype；函数 nvtxNameOsThreadW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxNameOsThreadW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr;
    if(local!=0)
        (*local)(threadId, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0693：nvtxDomainMarkEx
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainMarkEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(domain, eventAttrib);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0693
 */
/* 中文逐项-函数 F0693：nvtxDomainMarkEx
 * 用途与位置：向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainMarkEx_impl_init；src/include/nvtx3.hpp::mark。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainMarkEx(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2915：local
 * local（nvtxDomainMarkEx_impl_fntype；函数 nvtxDomainMarkEx 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainMarkEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr;
    if(local!=0)
        (*local)(domain, eventAttrib);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0694：nvtxDomainRangeStartEx
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainRangeStartEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(domain, eventAttrib);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxRangeId_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0694
 */
/* 中文逐项-函数 F0694：nvtxDomainRangeStartEx
 * 用途与位置：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainRangeStartEx_impl_init；src/include/nvtx3.hpp::start_range。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(domain, eventAttrib);；return (nvtxRangeId_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxRangeId_t NVTX_API nvtxDomainRangeStartEx(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2916：local
 * local（nvtxDomainRangeStartEx_impl_fntype；函数 nvtxDomainRangeStartEx 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainRangeStartEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr;
    if(local!=0)
        return (*local)(domain, eventAttrib);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxRangeId_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0695：nvtxDomainRangeEnd
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainRangeEnd_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(domain, id);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0695
 */
/* 中文逐项-函数 F0695：nvtxDomainRangeEnd
 * 用途与位置：按id结束显式NVTX时间范围。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 id（nvtxRangeId_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainRangeEnd_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainRangeEnd(nvtxDomainHandle_t domain, nvtxRangeId_t id)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2917：local
 * local（nvtxDomainRangeEnd_impl_fntype；函数 nvtxDomainRangeEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainRangeEnd_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr;
    if(local!=0)
        (*local)(domain, id);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0696：nvtxDomainRangePushEx
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainRangePushEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(domain, eventAttrib);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (int)NVTX_NO_PUSH_POP_TRACKING;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0696
 */
/* 中文逐项-函数 F0696：nvtxDomainRangePushEx
 * 用途与位置：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 eventAttrib（const nvtxEventAttributes_t* eventAttrib）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainRangePushEx_impl_init；src/include/nvtx3.hpp::domain_thread_range。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(domain, eventAttrib);；return (int)NVTX_NO_PUSH_POP_TRACKING;。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API nvtxDomainRangePushEx(nvtxDomainHandle_t domain, const nvtxEventAttributes_t* eventAttrib)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2918：local
 * local（nvtxDomainRangePushEx_impl_fntype；函数 nvtxDomainRangePushEx 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainRangePushEx_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr;
    if(local!=0)
        return (*local)(domain, eventAttrib);
    else
#endif  /*NVTX_DISABLE*/
        return (int)NVTX_NO_PUSH_POP_TRACKING;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0697：nvtxDomainRangePop
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainRangePop_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(domain);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (int)NVTX_NO_PUSH_POP_TRACKING;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0697
 */
/* 中文逐项-函数 F0697：nvtxDomainRangePop
 * 用途与位置：从当前线程NVTX范围栈pop结束最近区间。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainRangePop_impl_init；src/include/nvtx3.hpp::~domain_thread_range。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(domain);；return (int)NVTX_NO_PUSH_POP_TRACKING;。CHECK类宏还可能提前返回/跳转。
 */int NVTX_API nvtxDomainRangePop(nvtxDomainHandle_t domain)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2919：local
 * local（nvtxDomainRangePop_impl_fntype；函数 nvtxDomainRangePop 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainRangePop_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr;
    if(local!=0)
        return (*local)(domain);
    else
#endif  /*NVTX_DISABLE*/
        return (int)NVTX_NO_PUSH_POP_TRACKING;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0698：nvtxDomainResourceCreate
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainResourceCreate_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(domain, attribs);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxResourceHandle_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0698
 */
/* 中文逐项-函数 F0698：nvtxDomainResourceCreate
 * 用途与位置：在NVTX域中登记一个可被工具识别的资源属性对象。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 attribs（nvtxResourceAttributes_t* attribs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainResourceCreate_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(domain, attribs);；return (nvtxResourceHandle_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxResourceHandle_t NVTX_API nvtxDomainResourceCreate(nvtxDomainHandle_t domain, nvtxResourceAttributes_t* attribs)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2920：local
 * local（nvtxDomainResourceCreate_impl_fntype；函数 nvtxDomainResourceCreate 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainResourceCreate_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr;
    if(local!=0)
        return (*local)(domain, attribs);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxResourceHandle_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0699：nvtxDomainResourceDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainResourceDestroy_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(resource);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0699
 */
/* 中文逐项-函数 F0699：nvtxDomainResourceDestroy
 * 用途与位置：结束域内资源标记的生命周期。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 resource（nvtxResourceHandle_t resource）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainResourceDestroy_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainResourceDestroy(nvtxResourceHandle_t resource)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2921：local
 * local（nvtxDomainResourceDestroy_impl_fntype；函数 nvtxDomainResourceDestroy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainResourceDestroy_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr;
    if(local!=0)
        (*local)(resource);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0700：nvtxDomainNameCategoryA
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainNameCategoryA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(domain, category, name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0700
 */
/* 中文逐项-函数 F0700：nvtxDomainNameCategoryA
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 category（uint32_t category）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainNameCategoryA_impl_init；src/include/nvtx3.hpp::named_category。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainNameCategoryA(nvtxDomainHandle_t domain, uint32_t category, const char* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2922：local
 * local（nvtxDomainNameCategoryA_impl_fntype；函数 nvtxDomainNameCategoryA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainNameCategoryA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr;
    if(local!=0)
        (*local)(domain, category, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0701：nvtxDomainNameCategoryW
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainNameCategoryW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(domain, category, name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0701
 */
/* 中文逐项-函数 F0701：nvtxDomainNameCategoryW
 * 用途与位置：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 category（uint32_t category）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 name（const wchar_t* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainNameCategoryW_impl_init；src/include/nvtx3.hpp::named_category。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainNameCategoryW(nvtxDomainHandle_t domain, uint32_t category, const wchar_t* name)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2923：local
 * local（nvtxDomainNameCategoryW_impl_fntype；函数 nvtxDomainNameCategoryW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainNameCategoryW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr;
    if(local!=0)
        (*local)(domain, category, name);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0702：nvtxDomainRegisterStringA
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainRegisterStringA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(domain, string);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxStringHandle_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0702
 */
/* 中文逐项-函数 F0702：nvtxDomainRegisterStringA
 * 用途与位置：注册域内可复用字符串并返回句柄，降低重复消息处理开销。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 string（const char* string）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainRegisterStringA_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(domain, string);；return (nvtxStringHandle_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxStringHandle_t NVTX_API nvtxDomainRegisterStringA(nvtxDomainHandle_t domain, const char* string)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2924：local
 * local（nvtxDomainRegisterStringA_impl_fntype；函数 nvtxDomainRegisterStringA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainRegisterStringA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr;
    if(local!=0)
        return (*local)(domain, string);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxStringHandle_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0703：nvtxDomainRegisterStringW
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainRegisterStringW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(domain, string);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxStringHandle_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0703
 */
/* 中文逐项-函数 F0703：nvtxDomainRegisterStringW
 * 用途与位置：注册域内可复用字符串并返回句柄，降低重复消息处理开销。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 参数 string（const wchar_t* string）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainRegisterStringW_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(domain, string);；return (nvtxStringHandle_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxStringHandle_t NVTX_API nvtxDomainRegisterStringW(nvtxDomainHandle_t domain, const wchar_t* string)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2925：local
 * local（nvtxDomainRegisterStringW_impl_fntype；函数 nvtxDomainRegisterStringW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local =
 * NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainRegisterStringW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr;
    if(local!=0)
        return (*local)(domain, string);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxStringHandle_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0704：nvtxDomainCreateA
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainCreateA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(message);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxDomainHandle_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0704
 */
/* 中文逐项-函数 F0704：nvtxDomainCreateA
 * 用途与位置：创建带名字的NVTX标记域，隔离不同组件的事件/类别。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const char* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainCreateA_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(message);；return (nvtxDomainHandle_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxDomainHandle_t NVTX_API nvtxDomainCreateA(const char* message)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2926：local
 * local（nvtxDomainCreateA_impl_fntype；函数 nvtxDomainCreateA 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainCreateA_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr;
    if(local!=0)
        return (*local)(message);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxDomainHandle_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0705：nvtxDomainCreateW
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainCreateW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return (*local)(message);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return (nvtxDomainHandle_t)0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0705
 */
/* 中文逐项-函数 F0705：nvtxDomainCreateW
 * 用途与位置：创建带名字的NVTX标记域，隔离不同组件的事件/类别。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 message（const wchar_t* message）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainCreateW_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (*local)(message);；return (nvtxDomainHandle_t)0;。CHECK类宏还可能提前返回/跳转。
 */nvtxDomainHandle_t NVTX_API nvtxDomainCreateW(const wchar_t* message)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2927：local
 * local（nvtxDomainCreateW_impl_fntype；函数 nvtxDomainCreateW 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainCreateW_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr;
    if(local!=0)
        return (*local)(message);
    else
#endif  /*NVTX_DISABLE*/
        return (nvtxDomainHandle_t)0;
}

NVTX_DECLSPEC /* 中文内部逻辑 F0706：nvtxDomainDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxDomainDestroy_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0706
 */
/* 中文逐项-函数 F0706：nvtxDomainDestroy
 * 用途与位置：释放NVTX域句柄及工具相关引用。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 参数 domain（nvtxDomainHandle_t domain）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxDomainDestroy_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxDomainDestroy(nvtxDomainHandle_t domain)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2928：local
 * local（nvtxDomainDestroy_impl_fntype；函数 nvtxDomainDestroy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxDomainDestroy_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr;
    if(local!=0)
        (*local)(domain);
#endif /*NVTX_DISABLE*/
}

NVTX_DECLSPEC /* 中文内部逻辑 F0707：nvtxInitialize
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifndef NVTX_DISABLE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：nvtxInitialize_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(local!=0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：(*local)(reserved);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxImplCore_h.md#f0707
 */
/* 中文逐项-函数 F0707：nvtxInitialize
 * 用途与位置：显式触发NVTX工具初始化，通常API首次调用也能惰性触发；不创建NCCL通信资源。
 * 参数 reserved（const void* reserved）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/nvtx3/nvtxDetail/nvtxInitDefs.h::nvtxInitialize_impl_init。
 * 本体直接出现的调用/包装：(*local)。
 * 需要对照的边界/推进条件：local!=0。各条件所属分支见紧邻函数体。
 */void NVTX_API nvtxInitialize(const void* reserved)
{
#ifndef NVTX_DISABLE
/* 中文逐项-变量 V2929：local
 * local（nvtxInitialize_impl_fntype；函数 nvtxInitialize 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    nvtxInitialize_impl_fntype local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr;
    if(local!=0)
        (*local)(reserved);
#endif /*NVTX_DISABLE*/
}
