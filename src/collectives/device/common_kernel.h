/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_COMMON_KERNEL_H_
/* 中文逐项-常量 X0008：NCCL_COMMON_KERNEL_H_
 * 编译期定义：NCCL_COMMON_KERNEL_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_COMMON_KERNEL_H_

#include "devcomm.h"
#include <cstdio>
#include <cstdint>

#include <cuda_runtime.h>

// Define min for ssize_t
/* 中文内部逻辑 F0079：min
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (a < b) ? a : b;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0079
 */
/* 中文逐项-函数 F0079：min
 * 用途与位置：对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 * 参数 a（int a）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 b（ssize_t b）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapNetRecv；src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/all_reduce.h::runTreeUpDown；src/collectives/device/all_reduce.h::runTreeSplit；src/collectives/device/all_reduce.h::run；src/collectives/device/broadcast.h::runRing；其余见调用清单。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (a < b) ? a : b;。CHECK类宏还可能提前返回/跳转。
 */
static __device__ int min(int a, ssize_t b) { return (a < b) ? a : b; }

/* 中文内部逻辑 F0080：loadInt
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int v;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：asm volatile("ld.volatile.global.u32 %0, [%1];" : "=r"(v) : "l"(ptr));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 函数返回：return v;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0080
 */
/* 中文逐项-函数 F0080：loadInt
 * 用途与位置：用 volatile 全局加载读取协议偏移/控制值，防止编译器把动态变化值长久缓存在寄存器。
 * 参数 ptr（int* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_simple.h::waitPeer。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return v;。CHECK类宏还可能提前返回/跳转。
 */
inline __device__ int loadInt(int* ptr) {
/* 中文逐项-变量 V0293：v
 * v（int；函数 loadInt 内）：LL行的两个64位或int4整行视图，用向量汇编同时读写数据和标记。
 * 此处未显式给初值；后续首个文本使用点：: "=r"(v) : "l"(ptr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int v;
  asm volatile("ld.volatile.global.u32 %0, [%1];"
      : "=r"(v) : "l"(ptr));
  return v;
}

typedef uint64_t PackType;

template<typename Fn>
struct FuncTraits /*{
  __device__ static T preOp(Fn, T);
  __device__ static T postOp(Fn, T);
}*/;

// unpack x and y to elements of type T and apply FUNC to each element
/* 中文逐项-类型 T0023：MULTI
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 */
template<class FUNC, typename T>
struct MULTI {
  __device__ PackType operator()(FUNC fn, const PackType x, const PackType y) const;
  __device__ PackType preOp(FUNC fn, PackType x) const;
  __device__ PackType postOp(FUNC fn, PackType x) const;
};

/* 中文逐项-类型 T0024：MULTI<FUNC, int8_t>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：storage、a、b、pack、elt、pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, int8_t> {
  static_assert(sizeof(PackType) == 2 * sizeof(uint32_t),
      "PackType must be twice the size of uint32_t.");
/* 中文逐项-类型 T0025：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter {
/* 中文逐项-变量 V0294：storage
 * storage（PackType；成员 MULTI<FUNC, int8_t>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    PackType storage;
/* 中文逐项-类型 T0026：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V0295：a、b
 * a（uint32_t；成员 MULTI<FUNC, int8_t>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * b（uint32_t；成员 MULTI<FUNC, int8_t>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */
      uint32_t a, b;
    };
  };

  __device__ /* 中文内部逻辑 F0081：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：cr.a = fn(cx.a, cy.a);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：cr.b = fn(cx.b, cy.b);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0081
 */
/* 中文逐项-函数 F0081：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0296：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a = fn(cx.a, cy.a);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    // for char, we do these as vector ops
    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
  __device__ /* 中文内部逻辑 F0082：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; int8_t elt[8]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 4. 循环：初始 int i=0;；继续条件 i < 8；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：u.elt[i] = FuncTraits<FUNC>().preOp(fn, u.elt[i]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0082
 */
/* 中文逐项-函数 F0082：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 需要对照的边界/推进条件：i < 8。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0297：u
 * u（union { PackType pack; int8_t elt[8]; }；函数 preOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0027：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0298：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0299：elt
 * elt（int8_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      int8_t elt[8];
    } u;
    u.pack = x;
    #pragma unroll
    for (/* 中文逐项-变量 V0300：i
 * i（int；函数 preOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < 8；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < 8; i++)
      u.elt[i] = FuncTraits<FUNC>().preOp(fn, u.elt[i]);
    return u.pack;
  }
  __device__ /* 中文内部逻辑 F0083：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; int8_t elt[8]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 4. 循环：初始 int i=0;；继续条件 i < 8；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：u.elt[i] = FuncTraits<FUNC>().postOp(fn, u.elt[i]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0083
 */
/* 中文逐项-函数 F0083：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 需要对照的边界/推进条件：i < 8。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0301：u
 * u（union { PackType pack; int8_t elt[8]; }；函数 postOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0028：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0302：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0303：elt
 * elt（int8_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      int8_t elt[8];
    } u;
    u.pack = x;
    #pragma unroll
    for (/* 中文逐项-变量 V0304：i
 * i（int；函数 postOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < 8；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < 8; i++)
      u.elt[i] = FuncTraits<FUNC>().postOp(fn, u.elt[i]);
    return u.pack;
  }
};

/* 中文逐项-类型 T0029：MULTI<FUNC, uint8_t>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：storage、a、b、pack、elt、pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, uint8_t> {
  static_assert(sizeof(PackType) == 2 * sizeof(uint32_t),
      "PackType must be twice the size of uint32_t.");
/* 中文逐项-类型 T0030：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter {
/* 中文逐项-变量 V0305：storage
 * storage（PackType；成员 MULTI<FUNC, uint8_t>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    PackType storage;
/* 中文逐项-类型 T0031：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V0306：a、b
 * a（uint32_t；成员 MULTI<FUNC, uint8_t>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * b（uint32_t；成员 MULTI<FUNC, uint8_t>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */
      uint32_t a, b;
    };
  };

  __device__ /* 中文内部逻辑 F0084：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：cr.a = fn(cx.a, cy.a);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：cr.b = fn(cx.b, cy.b);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0084
 */
/* 中文逐项-函数 F0084：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0307：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a = fn(cx.a, cy.a);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    // for char, we do these as vector ops
    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
  __device__ /* 中文内部逻辑 F0085：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; uint8_t elt[8]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 4. 循环：初始 int i=0;；继续条件 i < 8；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：u.elt[i] = FuncTraits<FUNC>().preOp(fn, u.elt[i]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0085
 */
/* 中文逐项-函数 F0085：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 需要对照的边界/推进条件：i < 8。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0308：u
 * u（union { PackType pack; uint8_t elt[8]; }；函数 preOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0032：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0309：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0310：elt
 * elt（uint8_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      uint8_t elt[8];
    } u;
    u.pack = x;
    #pragma unroll
    for (/* 中文逐项-变量 V0311：i
 * i（int；函数 preOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < 8；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < 8; i++)
      u.elt[i] = FuncTraits<FUNC>().preOp(fn, u.elt[i]);
    return u.pack;
  }
  __device__ /* 中文内部逻辑 F0086：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; uint8_t elt[8]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 4. 循环：初始 int i=0;；继续条件 i < 8；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：u.elt[i] = FuncTraits<FUNC>().postOp(fn, u.elt[i]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0086
 */
/* 中文逐项-函数 F0086：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 需要对照的边界/推进条件：i < 8。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0312：u
 * u（union { PackType pack; uint8_t elt[8]; }；函数 postOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0033：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0313：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0314：elt
 * elt（uint8_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      uint8_t elt[8];
    } u;
    u.pack = x;
    #pragma unroll
    for (/* 中文逐项-变量 V0315：i
 * i（int；函数 postOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < 8；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < 8; i++)
      u.elt[i] = FuncTraits<FUNC>().postOp(fn, u.elt[i]);
    return u.pack;
  }
};

/* 中文逐项-类型 T0034：MULTI<FUNC, int32_t>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：storage、a、b、pack、elt、pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, int32_t> {
  static_assert(sizeof(PackType) == 2 * sizeof(int32_t),
      "PackType must be twice the size of int.");
/* 中文逐项-类型 T0035：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter {
/* 中文逐项-变量 V0316：storage
 * storage（PackType；成员 MULTI<FUNC, int32_t>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    PackType storage;
/* 中文逐项-类型 T0036：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V0317：a、b
 * a（int32_t；成员 MULTI<FUNC, int32_t>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * b（int32_t；成员 MULTI<FUNC, int32_t>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */
      int32_t a, b;
    };
  };

  __device__ /* 中文内部逻辑 F0087：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：cr.a = fn(cx.a, cy.a);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：cr.b = fn(cx.b, cy.b);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0087
 */
/* 中文逐项-函数 F0087：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0318：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a = fn(cx.a, cy.a);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
  __device__ /* 中文内部逻辑 F0088：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; int32_t elt[2]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0088
 */
/* 中文逐项-函数 F0088：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0319：u
 * u（union { PackType pack; int32_t elt[2]; }；函数 preOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0037：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0320：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0321：elt
 * elt（int32_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      int32_t elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);
    return u.pack;
  }
  __device__ /* 中文内部逻辑 F0089：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; int32_t elt[2]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0089
 */
/* 中文逐项-函数 F0089：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0322：u
 * u（union { PackType pack; int32_t elt[2]; }；函数 postOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0038：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0323：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0324：elt
 * elt（int32_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      int32_t elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);
    return u.pack;
  }
};

/* 中文逐项-类型 T0039：MULTI<FUNC, uint32_t>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：storage、a、b、pack、elt、pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, uint32_t> {
  static_assert(sizeof(PackType) == 2 * sizeof(uint32_t),
      "PackType must be twice the size of int.");
/* 中文逐项-类型 T0040：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter {
/* 中文逐项-变量 V0325：storage
 * storage（PackType；成员 MULTI<FUNC, uint32_t>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    PackType storage;
/* 中文逐项-类型 T0041：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V0326：a、b
 * a（uint32_t；成员 MULTI<FUNC, uint32_t>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * b（uint32_t；成员 MULTI<FUNC, uint32_t>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */
      uint32_t a, b;
    };
  };

  __device__ /* 中文内部逻辑 F0090：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：cr.a = fn(cx.a, cy.a);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：cr.b = fn(cx.b, cy.b);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0090
 */
/* 中文逐项-函数 F0090：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0327：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a = fn(cx.a, cy.a);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
  __device__ /* 中文内部逻辑 F0091：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; uint32_t elt[2]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0091
 */
/* 中文逐项-函数 F0091：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0328：u
 * u（union { PackType pack; uint32_t elt[2]; }；函数 preOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0042：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0329：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0330：elt
 * elt（uint32_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      uint32_t elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);
    return u.pack;
  }
  __device__ /* 中文内部逻辑 F0092：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; uint32_t elt[2]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0092
 */
/* 中文逐项-函数 F0092：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0331：u
 * u（union { PackType pack; uint32_t elt[2]; }；函数 postOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0043：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0332：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0333：elt
 * elt（uint32_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      uint32_t elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);
    return u.pack;
  }
};

/* 中文逐项-类型 T0044：MULTI<FUNC, half>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：pack、h2。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, half> {
  static_assert(sizeof(PackType) == 4 * sizeof(half),
      "PackType must be four times the size of half.");

/* 中文逐项-类型 T0045：Converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：pack、h2。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union Converter {
/* 中文逐项-变量 V0334：pack
 * pack（PackType；成员 MULTI<FUNC, half>::Converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    PackType pack;
/* 中文逐项-变量 V0335：h2
 * h2（half2；成员 MULTI<FUNC, half>::Converter）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
    half2 h2[2];
  };
  __device__ /* 中文内部逻辑 F0093：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：Converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：cx.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：cy.pack = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：cr.h2[0] = fn(cx.h2[0], cy.h2[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：cr.h2[1] = fn(cx.h2[1], cy.h2[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return cr.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0093
 */
/* 中文逐项-函数 F0093：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return cr.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0336：cx、cy、cr
 * cx（Converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（Converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.pack = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（Converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.h2[0] = fn(cx.h2[0], cy.h2[0]);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    Converter cx, cy, cr;
    cx.pack = x;
    cy.pack = y;
    cr.h2[0] = fn(cx.h2[0], cy.h2[0]);
    cr.h2[1] = fn(cx.h2[1], cy.h2[1]);
    return cr.pack;
  }
  __device__ /* 中文内部逻辑 F0094：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：Converter c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：c.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：c.h2[0] = FuncTraits<FUNC>().preOp(fn, c.h2[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：c.h2[1] = FuncTraits<FUNC>().preOp(fn, c.h2[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return c.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0094
 */
/* 中文逐项-函数 F0094：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 源码返回表达式：return c.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0337：c
 * c（Converter；函数 preOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：c.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    Converter c;
    c.pack = x;
    c.h2[0] = FuncTraits<FUNC>().preOp(fn, c.h2[0]);
    c.h2[1] = FuncTraits<FUNC>().preOp(fn, c.h2[1]);
    return c.pack;
  }
  __device__ /* 中文内部逻辑 F0095：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：Converter c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：c.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：c.h2[0] = FuncTraits<FUNC>().postOp(fn, c.h2[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：c.h2[1] = FuncTraits<FUNC>().postOp(fn, c.h2[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return c.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0095
 */
/* 中文逐项-函数 F0095：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 源码返回表达式：return c.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0338：c
 * c（Converter；函数 postOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：c.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    Converter c;
    c.pack = x;
    c.h2[0] = FuncTraits<FUNC>().postOp(fn, c.h2[0]);
    c.h2[1] = FuncTraits<FUNC>().postOp(fn, c.h2[1]);
    return c.pack;
  }
};

#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-类型 T0046：MULTI<FUNC, __nv_bfloat16>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：pack、h2。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, __nv_bfloat16> {
  static_assert(sizeof(PackType) == 4 * sizeof(__nv_bfloat16),
      "PackType must be four times the size of __nv_bfloat16.");

/* 中文逐项-类型 T0047：Converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：pack、h2。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union Converter {
/* 中文逐项-变量 V0339：pack
 * pack（PackType；成员 MULTI<FUNC, __nv_bfloat16>::Converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    PackType pack;
/* 中文逐项-变量 V0340：h2
 * h2（__nv_bfloat162；成员 MULTI<FUNC, __nv_bfloat16>::Converter）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
    __nv_bfloat162 h2[2];
  };
  __device__ /* 中文内部逻辑 F0096：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：Converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：cx.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：cy.pack = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：cr.h2[0] = fn(cx.h2[0], cy.h2[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：cr.h2[1] = fn(cx.h2[1], cy.h2[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return cr.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0096
 */
/* 中文逐项-函数 F0096：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return cr.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0341：cx、cy、cr
 * cx（Converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（Converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.pack = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（Converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.h2[0] = fn(cx.h2[0], cy.h2[0]);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    Converter cx, cy, cr;
    cx.pack = x;
    cy.pack = y;
    cr.h2[0] = fn(cx.h2[0], cy.h2[0]);
    cr.h2[1] = fn(cx.h2[1], cy.h2[1]);
    return cr.pack;
  }
  __device__ /* 中文内部逻辑 F0097：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：Converter c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：c.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：c.h2[0] = FuncTraits<FUNC>().preOp(fn, c.h2[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：c.h2[1] = FuncTraits<FUNC>().preOp(fn, c.h2[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return c.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0097
 */
/* 中文逐项-函数 F0097：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 源码返回表达式：return c.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0342：c
 * c（Converter；函数 preOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：c.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    Converter c;
    c.pack = x;
    c.h2[0] = FuncTraits<FUNC>().preOp(fn, c.h2[0]);
    c.h2[1] = FuncTraits<FUNC>().preOp(fn, c.h2[1]);
    return c.pack;
  }
  __device__ /* 中文内部逻辑 F0098：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：Converter c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：c.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：c.h2[0] = FuncTraits<FUNC>().postOp(fn, c.h2[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：c.h2[1] = FuncTraits<FUNC>().postOp(fn, c.h2[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return c.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0098
 */
/* 中文逐项-函数 F0098：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 源码返回表达式：return c.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0343：c
 * c（Converter；函数 postOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：c.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    Converter c;
    c.pack = x;
    c.h2[0] = FuncTraits<FUNC>().postOp(fn, c.h2[0]);
    c.h2[1] = FuncTraits<FUNC>().postOp(fn, c.h2[1]);
    return c.pack;
  }
};
#endif

/* 中文逐项-类型 T0048：MULTI<FUNC, float>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：storage、a、b、pack、elt、pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, float> {
  static_assert(sizeof(PackType) == 2 * sizeof(float),
      "PackType must be twice the size of float.");
/* 中文逐项-类型 T0049：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter {
/* 中文逐项-变量 V0344：storage
 * storage（PackType；成员 MULTI<FUNC, float>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    PackType storage;
/* 中文逐项-类型 T0050：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：a、b。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V0345：a、b
 * a（float；成员 MULTI<FUNC, float>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * b（float；成员 MULTI<FUNC, float>::converter::<匿名>）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */
      float a, b;
    };
  };

  __device__ /* 中文内部逻辑 F0099：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：cr.a = fn(cx.a, cy.a);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：cr.b = fn(cx.b, cy.b);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0099
 */
/* 中文逐项-函数 F0099：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0346：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a = fn(cx.a, cy.a);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
  __device__ /* 中文内部逻辑 F0100：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; float elt[2]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0100
 */
/* 中文逐项-函数 F0100：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0347：u
 * u（union { PackType pack; float elt[2]; }；函数 preOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0051：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0348：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0349：elt
 * elt（float；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      float elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);
    return u.pack;
  }
  __device__ /* 中文内部逻辑 F0101：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; float elt[2]; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0101
 */
/* 中文逐项-函数 F0101：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0350：u
 * u（union { PackType pack; float elt[2]; }；函数 postOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0052：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0351：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0352：elt
 * elt（float；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      float elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);
    return u.pack;
  }
};

/* 中文逐项-类型 T0053：MULTI<FUNC, double>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：pack、elt、pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, double> {
  static_assert(sizeof(PackType) == sizeof(double),
      "PackType must be the same size as double.");
  __device__ /* 中文内部逻辑 F0102：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：double rv = fn(__longlong_as_double(x), __longlong_as_double(y));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return __double_as_longlong(rv);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0102
 */
/* 中文逐项-函数 F0102：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__double_as_longlong；__longlong_as_double；fn。
 * 源码返回表达式：return __double_as_longlong(rv);。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0353：rv
 * rv（double；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 rv = fn(__longlong_as_double(x), __longlong_as_double(y))；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    double rv = fn(__longlong_as_double(x), __longlong_as_double(y));
    return __double_as_longlong(rv);
  }
  __device__ /* 中文内部逻辑 F0103：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; double elt; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0103
 */
/* 中文逐项-函数 F0103：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0354：u
 * u（union { PackType pack; double elt; }；函数 preOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0054：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0355：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0356：elt
 * elt（double；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      double elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);
    return u.pack;
  }
  __device__ /* 中文内部逻辑 F0104：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; double elt; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0104
 */
/* 中文逐项-函数 F0104：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0357：u
 * u（union { PackType pack; double elt; }；函数 postOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0055：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0358：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0359：elt
 * elt（double；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      double elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);
    return u.pack;
  }
};

/* 中文逐项-类型 T0056：MULTI<FUNC, uint64_t>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：pack、elt、pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, uint64_t> {
  static_assert(sizeof(PackType) == sizeof(uint64_t),
      "PackType must be the same size as uint64_t.");
  __device__ /* 中文内部逻辑 F0105：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：uint64_t rv = fn(x, y);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return rv;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0105
 */
/* 中文逐项-函数 F0105：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return rv;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0360：rv
 * rv（uint64_t；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 rv = fn(x, y)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    uint64_t rv = fn(x, y);
    return rv;
  }
  __device__ /* 中文内部逻辑 F0106：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; uint64_t elt; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0106
 */
/* 中文逐项-函数 F0106：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0361：u
 * u（union { PackType pack; uint64_t elt; }；函数 preOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0057：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0362：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0363：elt
 * elt（uint64_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      uint64_t elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);
    return u.pack;
  }
  __device__ /* 中文内部逻辑 F0107：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; uint64_t elt; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0107
 */
/* 中文逐项-函数 F0107：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0364：u
 * u（union { PackType pack; uint64_t elt; }；函数 postOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0058：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0365：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0366：elt
 * elt（uint64_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      uint64_t elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);
    return u.pack;
  }
};

/* 中文逐项-类型 T0059：MULTI<FUNC, int64_t>
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 * 声明内可见成员（含嵌套成员）：pack、elt、pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<class FUNC>
struct MULTI<FUNC, int64_t> {
  static_assert(sizeof(PackType) == sizeof(int64_t),
      "PackType must be the same size as int64_t.");
  __device__ /* 中文内部逻辑 F0108：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int64_t rv = fn((int64_t)x, (int64_t)y);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return rv;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0108
 */
/* 中文逐项-函数 F0108：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（const PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const PackType y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn。
 * 源码返回表达式：return rv;。CHECK类宏还可能提前返回/跳转。
 */PackType operator()(FUNC fn, const PackType x, const PackType y) const {
/* 中文逐项-变量 V0367：rv
 * rv（int64_t；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 rv = fn((int64_t)x, (int64_t)y)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int64_t rv = fn((int64_t)x, (int64_t)y);
    return rv;
  }
  __device__ /* 中文内部逻辑 F0109：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; int64_t elt; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0109
 */
/* 中文逐项-函数 F0109：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().preOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType preOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0368：u
 * u（union { PackType pack; int64_t elt; }；函数 preOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0060：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0369：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0370：elt
 * elt（int64_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      int64_t elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);
    return u.pack;
  }
  __device__ /* 中文内部逻辑 F0110：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union { PackType pack; int64_t elt; } u;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：u.pack = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return u.pack;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0110
 */
/* 中文逐项-函数 F0110：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（PackType x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp。
 * 源码返回表达式：return u.pack;。CHECK类宏还可能提前返回/跳转。
 */PackType postOp(FUNC fn, PackType x) const {
/* 中文逐项-变量 V0371：u
 * u（union { PackType pack; int64_t elt; }；函数 postOp 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：u.pack = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0061：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pack、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0372：pack
 * pack（PackType；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      PackType pack;
/* 中文逐项-变量 V0373：elt
 * elt（int64_t；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      int64_t elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);
    return u.pack;
  }
};

/* 中文内部逻辑 F0111：vFetch
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return *ptr;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0111
 */
/* 中文逐项-函数 F0111：vFetch
 * 用途与位置：读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。
 * 参数 ptr（const volatile T* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common_kernel.h::ReduceCopyMulti。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return *ptr;。CHECK类宏还可能提前返回/跳转。
 */
template<typename T> inline __device__
T vFetch(const volatile T* ptr) {
  return *ptr;
}

/* 中文内部逻辑 F0112：vStore
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*ptr = val;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0112
 */
/* 中文逐项-函数 F0112：vStore
 * 用途与位置：把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。
 * 参数 ptr（volatile T* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 val（const T val）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common_kernel.h::ReduceCopyMulti。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
template<typename T> inline __device__
void vStore(volatile T* ptr, const T val) {
  *ptr = val;
}

#if CUDART_VERSION < 9000
/* 中文内部逻辑 F0113：vFetch<half>
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：half r;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：r.x = ptr->x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 函数返回：return r;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0113
 */
/* 中文逐项-函数 F0113：vFetch<half>
 * 用途与位置：读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。
 * 参数 ptr（const volatile half* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return r;。CHECK类宏还可能提前返回/跳转。
 */
template<> inline __device__
half vFetch<half>(const volatile half* ptr) {
/* 中文逐项-变量 V0374：r
 * r（half；函数 vFetch<half> 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：r.x = ptr->x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  half r;
  r.x = ptr->x;
  return r;
}

/* 中文内部逻辑 F0114：vStore<half>
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：ptr->x = val.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0114
 */
/* 中文逐项-函数 F0114：vStore<half>
 * 用途与位置：把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。
 * 参数 ptr（volatile half* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 val（const half val）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
template<> inline __device__
void vStore<half>(volatile half* ptr, const half val) {
  ptr->x = val.x;
}
#else
/* 中文内部逻辑 F0115：vFetch<half>
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：half r;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：r = ((half*)ptr)[0];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 函数返回：return r;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0115
 */
/* 中文逐项-函数 F0115：vFetch<half>
 * 用途与位置：读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。
 * 参数 ptr（const volatile half* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return r;。CHECK类宏还可能提前返回/跳转。
 */
template<> inline __device__
half vFetch<half>(const volatile half* ptr) {
/* 中文逐项-变量 V0375：r
 * r（half；函数 vFetch<half> 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：r = ((half*)ptr)[0];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  half r;
  r = ((half*)ptr)[0];
  return r;
}

/* 中文内部逻辑 F0116：vStore<half>
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：((half*)ptr)[0] = val;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0116
 */
/* 中文逐项-函数 F0116：vStore<half>
 * 用途与位置：把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。
 * 参数 ptr（volatile half* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 val（const half val）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
template<> inline __device__
void vStore<half>(volatile half* ptr, const half val) {
  ((half*)ptr)[0] = val;
}
#endif

#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文内部逻辑 F0117：vFetch<__nv_bfloat16>
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：__nv_bfloat16 r;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：r = ((__nv_bfloat16*)ptr)[0];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 函数返回：return r;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0117
 */
/* 中文逐项-函数 F0117：vFetch<__nv_bfloat16>
 * 用途与位置：以 BF16 类型特化读取用户/协议缓冲元素，处理该类型的 volatile 访问写法。
 * 参数 ptr（const volatile __nv_bfloat16* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return r;。CHECK类宏还可能提前返回/跳转。
 */
template<> inline __device__
__nv_bfloat16 vFetch<__nv_bfloat16>(const volatile __nv_bfloat16* ptr) {
/* 中文逐项-变量 V0376：r
 * r（__nv_bfloat16；函数 vFetch<__nv_bfloat16> 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：r = ((__nv_bfloat16*)ptr)[0];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  __nv_bfloat16 r;
  r = ((__nv_bfloat16*)ptr)[0];
  return r;
}

/* 中文内部逻辑 F0118：vStore<__nv_bfloat16>
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：((__nv_bfloat16*)ptr)[0] = val;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0118
 */
/* 中文逐项-函数 F0118：vStore<__nv_bfloat16>
 * 用途与位置：把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。
 * 参数 ptr（volatile __nv_bfloat16* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 val（const __nv_bfloat16 val）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
template<> inline __device__
void vStore<__nv_bfloat16>(volatile __nv_bfloat16* ptr, const __nv_bfloat16 val) {
  ((__nv_bfloat16*)ptr)[0] = val;
}
#endif

typedef ulong2 Pack128;

/* 中文逐项-类型 T0062：MULTI128
 * 类型职责：把64/128位寄存器包按T拆成多个元素执行归约/预后处理，再恢复打包表示，以提高数据搬运效率。
 */
template<class FUNC, typename T>
struct MULTI128 {
  __device__ /* 中文内部逻辑 F0119：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：x.x = MULTI<FUNC, T>()(fn, x.x, y.x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：x.y = MULTI<FUNC, T>()(fn, x.y, y.y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0119
 */
/* 中文逐项-函数 F0119：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 <未命名参数>（Pack128& x）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 <未命名参数>（Pack128 const& y）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：MULTI<FUNC, T>；MULTI<FUNC, T>()。
 */void operator()(FUNC fn, Pack128& x, Pack128 const& y) const {
    x.x = MULTI<FUNC, T>()(fn, x.x, y.x);
    x.y = MULTI<FUNC, T>()(fn, x.y, y.y);
  }
  __device__ /* 中文内部逻辑 F0120：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：x.x = MULTI<FUNC, T>().preOp(fn, x.x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：x.y = MULTI<FUNC, T>().preOp(fn, x.y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0120
 */
/* 中文逐项-函数 F0120：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 <未命名参数>（Pack128 &x）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：MULTI<FUNC, T>；MULTI<FUNC, T>().preOp。
 */void preOp(FUNC fn, Pack128 &x) const {
    x.x = MULTI<FUNC, T>().preOp(fn, x.x);
    x.y = MULTI<FUNC, T>().preOp(fn, x.y);
  }
  __device__ /* 中文内部逻辑 F0121：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：x.x = MULTI<FUNC, T>().postOp(fn, x.x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：x.y = MULTI<FUNC, T>().postOp(fn, x.y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0121
 */
/* 中文逐项-函数 F0121：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FUNC fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 <未命名参数>（Pack128 &x）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：MULTI<FUNC, T>；MULTI<FUNC, T>().postOp。
 */void postOp(FUNC fn, Pack128 &x) const {
    x.x = MULTI<FUNC, T>().postOp(fn, x.x);
    x.y = MULTI<FUNC, T>().postOp(fn, x.y);
  }
};

/* 中文内部逻辑 F0122：Fetch128
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：asm volatile("ld.volatile.global.v2.u64 {%0,%1}, [%2];" : "=l"(v.x), "=l"(v.y) : "l"(p) : "memory");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0122
 */
/* 中文逐项-函数 F0122：Fetch128
 * 用途与位置：一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。
 * 参数 <未命名参数>（Pack128& v）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 p（const Pack128* p）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common_kernel.h::ReduceCopy128bMulti。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
inline __device__ void Fetch128(Pack128& v, const Pack128* p) {
  asm volatile("ld.volatile.global.v2.u64 {%0,%1}, [%2];" : "=l"(v.x), "=l"(v.y) : "l"(p) : "memory");
}
/* 中文内部逻辑 F0123：Store128
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：asm volatile("st.volatile.global.v2.u64 [%0], {%1,%2};" :: "l"(p), "l"(v.x), "l"(v.y) : "memory");；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0123
 */
/* 中文逐项-函数 F0123：Store128
 * 用途与位置：把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。
 * 参数 p（Pack128* p）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 参数 <未命名参数>（Pack128& v）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common_kernel.h::ReduceCopy128bMulti。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
inline __device__ void Store128(Pack128* p, Pack128& v) {
  asm volatile("st.volatile.global.v2.u64 [%0], {%1,%2};" :: "l"(p), "l"(v.x), "l"(v.y) : "memory");
}

/* 中文内部逻辑 F0124：ReduceCopyMulti
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const Int inc = nw * UNROLL * WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：Int offset = w * UNROLL * WARP_SIZE + t;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const T* srcs[MAXSRCS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 int i=0;；继续条件 i<MAXSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：srcs[i] = s[i]+elemOffset+offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 准备局部状态：T* dsts[MAXDSTS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 循环：初始 int i=0;；继续条件 i<MAXDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   9. 调用 / 计算 / 写入：dsts[i] = d[i]+elemOffset+offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 循环：初始 ；继续条件 (offset < Nelem)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   12. 准备局部状态：T vals[UNROLL];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   13. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     14. 调用 / 计算 / 写入：vals[u] = vFetch(srcs[0]+u*WARP_SIZE);；读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。
 *   15. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   16. 条件判断：(PreOpN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     17. 真分支：；仅条件为真执行以下子项。
 *       18. 准备局部状态：FUNC fn(redOpArgs[0]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       19. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         20. 调用 / 计算 / 写入：vals[u] = FuncTraits<FUNC>().preOp(fn, vals[u]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       21. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   22. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   23. 循环：初始 int i=1;；继续条件 i<MINSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     24. 准备局部状态：T vals2[UNROLL];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     25. 准备局部状态：FUNC fn(redOpArgs[i]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     26. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       27. 调用 / 计算 / 写入：vals2[u] = vFetch(srcs[i]+u*WARP_SIZE);；读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。
 *     28. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     29. 条件判断：(i<PreOpN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       30. 真分支：；仅条件为真执行以下子项。
 *         31. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           32. 调用 / 计算 / 写入：vals2[u] = FuncTraits<FUNC>().preOp(fn, vals2[u]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         33. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     34. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       35. 调用 / 计算 / 写入：vals[u] = fn(vals[u], vals2[u]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     36. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   37. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   38. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   39. 循环：初始 int i=MINSRCS;；继续条件 i<MAXSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     40. 条件判断：(i<nsrcs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       41. 真分支：；仅条件为真执行以下子项。
 *         42. 准备局部状态：T vals2[UNROLL];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         43. 准备局部状态：FUNC fn(redOpArgs[i]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         44. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           45. 调用 / 计算 / 写入：vals2[u] = vFetch(srcs[i]+u*WARP_SIZE);；读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。
 *         46. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         47. 条件判断：(i<PreOpN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           48. 真分支：；仅条件为真执行以下子项。
 *             49. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               50. 调用 / 计算 / 写入：vals2[u] = FuncTraits<FUNC>().preOp(fn, vals2[u]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             51. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         52. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           53. 调用 / 计算 / 写入：vals[u] = fn(vals[u], vals2[u]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         54. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   55. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   56. 条件判断：(postOp)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     57. 真分支：；仅条件为真执行以下子项。
 *       58. 准备局部状态：FUNC fn(redOpArgs[0]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       59. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       60. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         61. 调用 / 计算 / 写入：vals[u] = FuncTraits<FUNC>().postOp(fn, vals[u]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       62. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   63. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   64. 循环：初始 int i = 0;；继续条件 i < MINDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     65. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       66. 调用 / 计算 / 写入：vStore(dsts[i]+u*WARP_SIZE, vals[u]);；把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。
 *     67. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   68. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   69. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   70. 循环：初始 int i=MINDSTS;；继续条件 i<MAXDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     71. 条件判断：(i<ndsts)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       72. 真分支：；仅条件为真执行以下子项。
 *         73. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           74. 调用 / 计算 / 写入：vStore(dsts[i]+u*WARP_SIZE, vals[u]);；把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。
 *         75. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   76. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   77. 循环：初始 int i=0;；继续条件 i<MAXSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     78. 调用 / 计算 / 写入：srcs[i] += inc;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   79. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   80. 循环：初始 int i=0;；继续条件 i<MAXDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     81. 调用 / 计算 / 写入：dsts[i] += inc;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   82. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   83. 调用 / 计算 / 写入：offset += inc;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 84. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0124
 */
/* 中文逐项-函数 F0124：ReduceCopyMulti
 * 用途与位置：线程按 warp/展开因子分工，逐类型加载多个源，在寄存器中归约后写入各目标，处理标量与尾部路径。
 * 参数 w（const int w）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 nw（const int nw）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 t（const int t）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 redOpArgs（uint64_t* redOpArgs）：融合多源原语需要的标量参数数组，各输入来源可能使用自己的预处理参数。
 * 参数 postOp（bool postOp）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 nsrcs（int nsrcs）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 s（const T** s）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 参数 ndsts（int ndsts）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 d（T** d）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 参数 elemOffset（const int elemOffset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 参数 Nelem（const Int Nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common_kernel.h::ReduceOrCopyMulti。
 * 本体直接出现的调用/包装：FuncTraits<FUNC>；FuncTraits<FUNC>().postOp；FuncTraits<FUNC>().preOp；fn；vFetch；vStore。
 * 需要对照的边界/推进条件：i<MAXSRCS；i<MAXDSTS；offset < Nelem；u < UNROLL。各条件所属分支见紧邻函数体。
 */
template<class FUNC, typename T, int UNROLL, int MINSRCS, int MAXSRCS, int MINDSTS, int MAXDSTS, int PreOpN, typename Int>
__device__ __forceinline__ void ReduceCopyMulti(const int w, const int nw, const int t,
    uint64_t* redOpArgs, bool postOp, int nsrcs, const T** s, int ndsts, T** d, const int elemOffset, const Int Nelem
  ) {
/* 中文逐项-变量 V0377：inc
 * inc（Int；函数 ReduceCopyMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 inc = nw * UNROLL * WARP_SIZE；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  const Int inc = nw * UNROLL * WARP_SIZE;
/* 中文逐项-变量 V0378：offset
 * offset（Int；函数 ReduceCopyMulti 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = w * UNROLL * WARP_SIZE + t；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  Int offset = w * UNROLL * WARP_SIZE + t;

/* 中文逐项-变量 V0379：srcs
 * srcs（T；函数 ReduceCopyMulti 内）：当前 slice 的有效源/目标地址表，包含本地缓冲与邻居协议/直接缓冲。
 * 此处未显式给初值；后续首个文本使用点：for (int i=0; i<MAXSRCS; i++) srcs[i] = s[i]+elemOffset+offset;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const T* srcs[MAXSRCS];
  for (/* 中文逐项-变量 V0380：i
 * i（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAXSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAXSRCS; i++) srcs[i] = s[i]+elemOffset+offset;
/* 中文逐项-变量 V0381：dsts
 * dsts（T；函数 ReduceCopyMulti 内）：当前 slice 的有效源/目标地址表，包含本地缓冲与邻居协议/直接缓冲。
 * 此处未显式给初值；后续首个文本使用点：for (int i=0; i<MAXDSTS; i++) dsts[i] = d[i]+elemOffset+offset;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  T* dsts[MAXDSTS];
  for (/* 中文逐项-变量 V0382：i
 * i（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAXDSTS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAXDSTS; i++) dsts[i] = d[i]+elemOffset+offset;

  while (offset < Nelem) {
/* 中文逐项-变量 V0383：vals
 * vals（T；函数 ReduceCopyMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int u = 0; u < UNROLL; ++u) vals[u] = vFetch(srcs[0]+u*WARP_SIZE);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    T vals[UNROLL];
    // Load and reduce
    for (/* 中文逐项-变量 V0384：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals[u] = vFetch(srcs[0]+u*WARP_SIZE);
    if (PreOpN) {
      FUNC fn(redOpArgs[0]);
      for (/* 中文逐项-变量 V0385：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals[u] = FuncTraits<FUNC>().preOp(fn, vals[u]);
    }

    #pragma unroll
    for (/* 中文逐项-变量 V0386：i
 * i（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MINSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=1; i<MINSRCS; i++) {
/* 中文逐项-变量 V0387：vals2
 * vals2（T；函数 ReduceCopyMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int u = 0; u < UNROLL; ++u) vals2[u] =
 * vFetch(srcs[i]+u*WARP_SIZE);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      T vals2[UNROLL];
      FUNC fn(redOpArgs[i]);
      for (/* 中文逐项-变量 V0388：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals2[u] = vFetch(srcs[i]+u*WARP_SIZE);
      if (i<PreOpN) {
        for (/* 中文逐项-变量 V0389：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals2[u] = FuncTraits<FUNC>().preOp(fn, vals2[u]);
      }
      for (/* 中文逐项-变量 V0390：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals[u] = fn(vals[u], vals2[u]);
    }
    #pragma unroll
    for (/* 中文逐项-变量 V0391：i
 * i（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = MINSRCS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MAXSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=MINSRCS; i<MAXSRCS; i++) {
      if (i<nsrcs) {
/* 中文逐项-变量 V0392：vals2
 * vals2（T；函数 ReduceCopyMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int u = 0; u < UNROLL; ++u) vals2[u] =
 * vFetch(srcs[i]+u*WARP_SIZE);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        T vals2[UNROLL];
        FUNC fn(redOpArgs[i]);
        for (/* 中文逐项-变量 V0393：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals2[u] = vFetch(srcs[i]+u*WARP_SIZE);
        if (i<PreOpN) {
          for (/* 中文逐项-变量 V0394：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals2[u] = FuncTraits<FUNC>().preOp(fn, vals2[u]);
        }
        for (/* 中文逐项-变量 V0395：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals[u] = fn(vals[u], vals2[u]);
      }
    }

    if (postOp) {
      FUNC fn(redOpArgs[0]);
      #pragma unroll
      for (/* 中文逐项-变量 V0396：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vals[u] = FuncTraits<FUNC>().postOp(fn, vals[u]);
    }

    // Store
    #pragma unroll
    for (/* 中文逐项-变量 V0397：i
 * i（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < MINDSTS；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < MINDSTS; i++) {
      for (/* 中文逐项-变量 V0398：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vStore(dsts[i]+u*WARP_SIZE, vals[u]);
    }
    #pragma unroll
    for (/* 中文逐项-变量 V0399：i
 * i（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = MINDSTS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MAXDSTS；循环体与增量表达式决定每轮如何推进。
 */int i=MINDSTS; i<MAXDSTS; i++) {
      if (i<ndsts) {
        for (/* 中文逐项-变量 V0400：u
 * u（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) vStore(dsts[i]+u*WARP_SIZE, vals[u]);
      }
    }
    for (/* 中文逐项-变量 V0401：i
 * i（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAXSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAXSRCS; i++) srcs[i] += inc;
    for (/* 中文逐项-变量 V0402：i
 * i（int；函数 ReduceCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAXDSTS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAXDSTS; i++) dsts[i] += inc;
    offset += inc;
  }
}

/* 中文内部逻辑 F0125：ReduceCopy128bMulti
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const Int inc = nw * UNROLL * WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：Int offset = w * UNROLL * WARP_SIZE + t;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const Pack128* srcs[MAXSRCS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 int i=0;；继续条件 i<MAXSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：srcs[i] = ((const Pack128*)(s[i]+elemOffset))+offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 准备局部状态：Pack128* dsts[MAXDSTS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 循环：初始 int i=0;；继续条件 i<MAXDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   9. 调用 / 计算 / 写入：dsts[i] = ((Pack128*)(d[i]+elemOffset))+offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 循环：初始 ；继续条件 (offset < Npack)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   12. 准备局部状态：Pack128 vals[UNROLL];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   13. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     14. 调用 / 计算 / 写入：Fetch128(vals[u], srcs[0]+u*WARP_SIZE);；一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。
 *   15. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   16. 条件判断：(PreOpN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     17. 真分支：；仅条件为真执行以下子项。
 *       18. 准备局部状态：FUNC fn(redOpArgs[0]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       19. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         20. 调用 / 计算 / 写入：MULTI128<FUNC, T>().preOp(fn, vals[u]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       21. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   22. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   23. 循环：初始 int i=1;；继续条件 i<MINSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     24. 准备局部状态：Pack128 vals2[UNROLL];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     25. 准备局部状态：FUNC fn(redOpArgs[i]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     26. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       27. 调用 / 计算 / 写入：Fetch128(vals2[u], srcs[i]+u*WARP_SIZE);；一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。
 *     28. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     29. 条件判断：(i<PreOpN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       30. 真分支：；仅条件为真执行以下子项。
 *         31. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           32. 调用 / 计算 / 写入：MULTI128<FUNC, T>().preOp(fn, vals2[u]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         33. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     34. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       35. 调用 / 计算 / 写入：MULTI128<FUNC, T>()(fn, vals[u], vals2[u]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     36. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   37. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   38. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   39. 循环：初始 int i=MINSRCS;；继续条件 i<MAXSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     40. 条件判断：(i<nsrcs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       41. 真分支：；仅条件为真执行以下子项。
 *         42. 准备局部状态：Pack128 vals2[UNROLL];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         43. 准备局部状态：FUNC fn(redOpArgs[i]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         44. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           45. 调用 / 计算 / 写入：Fetch128(vals2[u], srcs[i]+u*WARP_SIZE);；一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。
 *         46. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         47. 条件判断：(i<PreOpN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           48. 真分支：；仅条件为真执行以下子项。
 *             49. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               50. 调用 / 计算 / 写入：MULTI128<FUNC, T>().preOp(fn, vals2[u]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             51. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         52. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           53. 调用 / 计算 / 写入：MULTI128<FUNC, T>()(fn, vals[u], vals2[u]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         54. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   55. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   56. 条件判断：(postOp)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     57. 真分支：；仅条件为真执行以下子项。
 *       58. 准备局部状态：FUNC fn(redOpArgs[0]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       59. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       60. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         61. 调用 / 计算 / 写入：MULTI128<FUNC, T>().postOp(fn, vals[u]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       62. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   63. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   64. 循环：初始 int i = 0;；继续条件 i < MINDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     65. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       66. 调用 / 计算 / 写入：Store128(dsts[i]+u*WARP_SIZE, vals[u]);；把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。
 *     67. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   68. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   69. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   70. 循环：初始 int i=MINDSTS;；继续条件 i<MAXDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     71. 条件判断：(i<ndsts)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       72. 真分支：；仅条件为真执行以下子项。
 *         73. 循环：初始 int u = 0;；继续条件 u < UNROLL；每轮更新 ++u；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           74. 调用 / 计算 / 写入：Store128(dsts[i]+u*WARP_SIZE, vals[u]);；把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。
 *         75. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   76. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   77. 循环：初始 int i=0;；继续条件 i<MAXSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     78. 调用 / 计算 / 写入：srcs[i] += inc;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   79. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   80. 循环：初始 int i=0;；继续条件 i<MAXDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     81. 调用 / 计算 / 写入：dsts[i] += inc;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   82. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   83. 调用 / 计算 / 写入：offset += inc;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 84. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0125
 */
/* 中文逐项-函数 F0125：ReduceCopy128bMulti
 * 用途与位置：在 16 字节对齐快路径中加载多个 128 位包、归约并多目标写出，利用向量指令和循环展开。
 * 参数 w（const int w）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 nw（const int nw）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 t（const int t）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 redOpArgs（uint64_t* redOpArgs）：融合多源原语需要的标量参数数组，各输入来源可能使用自己的预处理参数。
 * 参数 postOp（bool postOp）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 nsrcs（int nsrcs）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 s（const T** s）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 参数 ndsts（int ndsts）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 d（T** d）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 参数 elemOffset（const int elemOffset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 参数 Npack（const Int Npack）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common_kernel.h::ReduceOrCopyMulti。
 * 本体直接出现的调用/包装：Fetch128；MULTI128<FUNC, T>；MULTI128<FUNC, T>()；MULTI128<FUNC, T>().postOp；MULTI128<FUNC,
 * T>().preOp；Store128。
 * 需要对照的边界/推进条件：i<MAXSRCS；i<MAXDSTS；offset < Npack；u < UNROLL。各条件所属分支见紧邻函数体。
 */
template<class FUNC, typename T, int UNROLL, int MINSRCS, int MAXSRCS, int MINDSTS, int MAXDSTS, int PreOpN, typename Int>
__device__ __forceinline__ void ReduceCopy128bMulti(const int w, const int nw, const int t,
    uint64_t* redOpArgs, bool postOp, int nsrcs, const T** s, int ndsts, T** d, const int elemOffset, const Int Npack
  ) {
/* 中文逐项-变量 V0403：inc
 * inc（Int；函数 ReduceCopy128bMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 inc = nw * UNROLL * WARP_SIZE；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  const Int inc = nw * UNROLL * WARP_SIZE;
/* 中文逐项-变量 V0404：offset
 * offset（Int；函数 ReduceCopy128bMulti 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = w * UNROLL * WARP_SIZE + t；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  Int offset = w * UNROLL * WARP_SIZE + t;

/* 中文逐项-变量 V0405：srcs
 * srcs（Pack128；函数 ReduceCopy128bMulti 内）：当前 slice 的有效源/目标地址表，包含本地缓冲与邻居协议/直接缓冲。
 * 此处未显式给初值；后续首个文本使用点：for (int i=0; i<MAXSRCS; i++) srcs[i] = ((const
 * Pack128*)(s[i]+elemOffset))+offset;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const Pack128* srcs[MAXSRCS];
  for (/* 中文逐项-变量 V0406：i
 * i（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAXSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAXSRCS; i++) srcs[i] = ((const Pack128*)(s[i]+elemOffset))+offset;
/* 中文逐项-变量 V0407：dsts
 * dsts（Pack128；函数 ReduceCopy128bMulti 内）：当前 slice 的有效源/目标地址表，包含本地缓冲与邻居协议/直接缓冲。
 * 此处未显式给初值；后续首个文本使用点：for (int i=0; i<MAXDSTS; i++) dsts[i] =
 * ((Pack128*)(d[i]+elemOffset))+offset;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  Pack128* dsts[MAXDSTS];
  for (/* 中文逐项-变量 V0408：i
 * i（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAXDSTS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAXDSTS; i++) dsts[i] = ((Pack128*)(d[i]+elemOffset))+offset;

  while (offset < Npack) {
/* 中文逐项-变量 V0409：vals
 * vals（Pack128；函数 ReduceCopy128bMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int u = 0; u < UNROLL; ++u) Fetch128(vals[u],
 * srcs[0]+u*WARP_SIZE);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    Pack128 vals[UNROLL];
    // Load and reduce
    for (/* 中文逐项-变量 V0410：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) Fetch128(vals[u], srcs[0]+u*WARP_SIZE);
    if (PreOpN) {
      FUNC fn(redOpArgs[0]);
      for (/* 中文逐项-变量 V0411：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>().preOp(fn, vals[u]);
    }

    #pragma unroll
    for (/* 中文逐项-变量 V0412：i
 * i（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MINSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=1; i<MINSRCS; i++) {
/* 中文逐项-变量 V0413：vals2
 * vals2（Pack128；函数 ReduceCopy128bMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int u = 0; u < UNROLL; ++u) Fetch128(vals2[u],
 * srcs[i]+u*WARP_SIZE);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      Pack128 vals2[UNROLL];
      FUNC fn(redOpArgs[i]);
      for (/* 中文逐项-变量 V0414：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) Fetch128(vals2[u], srcs[i]+u*WARP_SIZE);
      if (i<PreOpN) {
        for (/* 中文逐项-变量 V0415：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>().preOp(fn, vals2[u]);
      }
      for (/* 中文逐项-变量 V0416：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>()(fn, vals[u], vals2[u]);
    }
    #pragma unroll
    for (/* 中文逐项-变量 V0417：i
 * i（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = MINSRCS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MAXSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=MINSRCS; i<MAXSRCS; i++) {
      if (i<nsrcs) {
/* 中文逐项-变量 V0418：vals2
 * vals2（Pack128；函数 ReduceCopy128bMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int u = 0; u < UNROLL; ++u) Fetch128(vals2[u],
 * srcs[i]+u*WARP_SIZE);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        Pack128 vals2[UNROLL];
        FUNC fn(redOpArgs[i]);
        for (/* 中文逐项-变量 V0419：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) Fetch128(vals2[u], srcs[i]+u*WARP_SIZE);
        if (i<PreOpN) {
          for (/* 中文逐项-变量 V0420：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>().preOp(fn, vals2[u]);
        }
        for (/* 中文逐项-变量 V0421：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>()(fn, vals[u], vals2[u]);
      }
    }

    if (postOp) {
      FUNC fn(redOpArgs[0]);
      #pragma unroll
      for (/* 中文逐项-变量 V0422：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>().postOp(fn, vals[u]);
    }

    // Store
    #pragma unroll
    for (/* 中文逐项-变量 V0423：i
 * i（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < MINDSTS；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < MINDSTS; i++) {
      for (/* 中文逐项-变量 V0424：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) Store128(dsts[i]+u*WARP_SIZE, vals[u]);
    }
    #pragma unroll
    for (/* 中文逐项-变量 V0425：i
 * i（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = MINDSTS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MAXDSTS；循环体与增量表达式决定每轮如何推进。
 */int i=MINDSTS; i<MAXDSTS; i++) {
      if (i<ndsts) {
        for (/* 中文逐项-变量 V0426：u
 * u（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u < UNROLL；循环体与增量表达式决定每轮如何推进。
 */int u = 0; u < UNROLL; ++u) Store128(dsts[i]+u*WARP_SIZE, vals[u]);
      }
    }
    for (/* 中文逐项-变量 V0427：i
 * i（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAXSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAXSRCS; i++) srcs[i] += inc;
    for (/* 中文逐项-变量 V0428：i
 * i（int；函数 ReduceCopy128bMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAXDSTS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAXDSTS; i++) dsts[i] += inc;
    offset += inc;
  }
}

/* 中文内部逻辑 F0126：ptrAlign128
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (uint64_t)ptr % alignof(Pack128);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0126
 */
/* 中文逐项-函数 F0126：ptrAlign128
 * 用途与位置：返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。
 * 参数 ptr（T* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common_kernel.h::ReduceOrCopyMulti。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (uint64_t)ptr % alignof(Pack128);。CHECK类宏还可能提前返回/跳转。
 */
template <typename T>
__device__ int ptrAlign128(T* ptr) { return (uint64_t)ptr % alignof(Pack128); }

/* 中文逐项-常量 X0009：PACKELEMS
 * 编译期定义：PACKELEMS = (sizeof(Pack128) / sizeof(T))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PACKELEMS (sizeof(Pack128) / sizeof(T))

/* 中文内部逻辑 F0127：ReduceOrCopyMulti
 * 精读：先检查全部有效源目标的16B对齐，条件满足才走128bit包快路径。
 * 精读：依次处理完整展开包、少量不展开包、标量展开与最后余数，任何路径都不能漏尾元素。
 * 精读：归约发生在寄存器，目标可以是本地输出和多个下游地址；上层已负责流控与可见性。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：Int Nrem = N;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(Nrem <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int w = tid / WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：int nw = nthreads / WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 准备局部状态：int t = tid % WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 准备局部状态：int align = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 10. 循环：初始 int i=0;；继续条件 i<MINSRCS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   11. 调用 / 计算 / 写入：align |= ptrAlign128(srcs[i]);；返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 循环：初始 int i=MINSRCS;；继续条件 i<MAXSRCS && i<nsrcs；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   14. 调用 / 计算 / 写入：align |= ptrAlign128(srcs[i]);；返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。
 * 15. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 16. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 17. 循环：初始 int i=0;；继续条件 i<MINDSTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   18. 调用 / 计算 / 写入：align |= ptrAlign128(dsts[i]);；返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。
 * 19. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 20. 循环：初始 int i=MINDSTS;；继续条件 i<MAXDSTS && i<ndsts；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   21. 调用 / 计算 / 写入：align |= ptrAlign128(dsts[i]);；返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。
 * 22. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 23. 准备局部状态：Int offset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 24. 条件判断：(align == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   25. 真分支：；仅条件为真执行以下子项。
 *     26. 准备局部状态：Int Npack = (Nrem / (PACKELEMS*UNROLL*WARP_SIZE)) * (UNROLL*WARP_SIZE);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     27. 准备局部状态：Int Nelem = Npack * PACKELEMS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     28. 调用 / 计算 / 写入：ReduceCopy128bMulti<FUNC, T, UNROLL, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN> (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Npack);；在 16 字节对齐快路径中加载多个 128 位包、归约并多目标写出，利用向量指令和循环展开。
 *     29. 调用 / 计算 / 写入：Nrem -= Nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     30. 条件判断：(Nrem == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       31. 真分支：；仅条件为真执行以下子项。
 *         32. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     33. 调用 / 计算 / 写入：offset += Nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     34. 调用 / 计算 / 写入：Npack = Nrem / PACKELEMS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     35. 调用 / 计算 / 写入：Nelem = Npack * PACKELEMS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     36. 调用 / 计算 / 写入：ReduceCopy128bMulti<FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN> (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Npack);；在 16 字节对齐快路径中加载多个 128 位包、归约并多目标写出，利用向量指令和循环展开。
 *     37. 调用 / 计算 / 写入：Nrem -= Nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     38. 条件判断：(Nrem == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       39. 真分支：；仅条件为真执行以下子项。
 *         40. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     41. 调用 / 计算 / 写入：offset += Nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 42. 准备局部状态：Int Nelem = (Nrem / (UNROLL*PACKELEMS/2*WARP_SIZE)) * (UNROLL*PACKELEMS/2*WARP_SIZE);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 43. 调用 / 计算 / 写入：ReduceCopyMulti<FUNC, T, UNROLL*PACKELEMS/2, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN> (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Nelem);；线程按 warp/展开因子分工，逐类型加载多个源，在寄存器中归约后写入各目标，处理标量与尾部路径。
 * 44. 调用 / 计算 / 写入：Nrem -= Nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 45. 条件判断：(Nrem == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   46. 真分支：；仅条件为真执行以下子项。
 *     47. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 48. 调用 / 计算 / 写入：offset += Nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 49. 调用 / 计算 / 写入：ReduceCopyMulti<FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN> (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Nrem);；线程按 warp/展开因子分工，逐类型加载多个源，在寄存器中归约后写入各目标，处理标量与尾部路径。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_kernel_h.md#f0127
 */
/* 中文逐项-函数 F0127：ReduceOrCopyMulti
 * 用途与位置：统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。
 * 参数 tid（const int tid）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 参数 nthreads（const int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 参数 redOpArgs（uint64_t* redOpArgs）：融合多源原语需要的标量参数数组，各输入来源可能使用自己的预处理参数。
 * 参数 postOp（bool postOp）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 nsrcs（int nsrcs）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 srcs（const T** srcs）：当前 slice 的有效源/目标地址表，包含本地缓冲与邻居协议/直接缓冲。
 * 参数 ndsts（int ndsts）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 dsts（T** dsts）：当前 slice 的有效源/目标地址表，包含本地缓冲与邻居协议/直接缓冲。
 * 参数 N（Int N）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/onerank_reduce.cu::oneRankReduce；src/collectives/device/prims_simple.h::genericOp；src/collectives/device/prims_simple.h::ScatterGatherOp；src/collectives/device/sendrecv.h::runSend。
 * 本体直接出现的调用/包装：ReduceCopy128bMulti<FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>；ReduceCopy128bMulti<FUNC,
 * T, UNROLL, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>；ReduceCopyMulti<FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS,
 * MAXDSTS, PreOpN>；ReduceCopyMulti<FUNC, T, UNROLL*PACKELEMS/2, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS,
 * PreOpN>；ptrAlign128。
 * 需要对照的边界/推进条件：Nrem <= 0；i<MINSRCS；i<MAXSRCS && i<nsrcs；i<MINDSTS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 */
template<int UNROLL, class FUNC, typename T, int MINSRCS, int MAXSRCS, int MINDSTS, int MAXDSTS, int PreOpN, typename Int>
/* 中文精读：最终负责读、归约、写的线程搬运层
 * 原语已处理流控并装好 srcs/dsts，本函数负责本 slice 的实际数据运算。
 * 所有指针满足 16B 对齐时优先使用 128-bit 向量加载/写入；剩余尾数降级到小粒度路径。
 * 多个源在寄存器里归约，再把同一结果写到所有目标；线程按 warp 和 UNROLL 分摊不同元素。
 * postOp 在归约完成后应用；没有多个有效源时，这个框架自然退化为复制。
 * 它不知道 QP、socket 或拓扑搜索，只处理已经可安全访问的内存地址。
 */
__device__ __forceinline__ void ReduceOrCopyMulti(
    const int tid, const int nthreads, uint64_t* redOpArgs, bool postOp, int nsrcs, const T** srcs, int ndsts, T** dsts, Int N
  ) {
/* 中文逐项-变量 V0429：Nrem
 * Nrem（Int；函数 ReduceOrCopyMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 Nrem = N；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  Int Nrem = N;
  if (Nrem <= 0) return;

/* 中文逐项-变量 V0430：w
 * w（int；函数 ReduceOrCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 w = tid / WARP_SIZE；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int w = tid / WARP_SIZE;       // Warp number
/* 中文逐项-变量 V0431：nw
 * nw（int；函数 ReduceOrCopyMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nw = nthreads / WARP_SIZE；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int nw = nthreads / WARP_SIZE; // Number of warps
/* 中文逐项-变量 V0432：t
 * t（int；函数 ReduceOrCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = tid % WARP_SIZE；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int t = tid % WARP_SIZE;       // Thread (inside the warp)

  // Check that all is 16B aligned. If not don't use 16B load/stores.
/* 中文逐项-变量 V0433：align
 * align（int；函数 ReduceOrCopyMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 align = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int align = 0;
  #pragma unroll
  for (/* 中文逐项-变量 V0434：i
 * i（int；函数 ReduceOrCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MINSRCS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MINSRCS; i++) align |= ptrAlign128(srcs[i]);
  for (/* 中文逐项-变量 V0435：i
 * i（int；函数 ReduceOrCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = MINSRCS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MAXSRCS && i<nsrcs；循环体与增量表达式决定每轮如何推进。
 */int i=MINSRCS; i<MAXSRCS && i<nsrcs; i++) align |= ptrAlign128(srcs[i]);
  #pragma unroll
  for (/* 中文逐项-变量 V0436：i
 * i（int；函数 ReduceOrCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MINDSTS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MINDSTS; i++) align |= ptrAlign128(dsts[i]);
  for (/* 中文逐项-变量 V0437：i
 * i（int；函数 ReduceOrCopyMulti 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = MINDSTS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MAXDSTS && i<ndsts；循环体与增量表达式决定每轮如何推进。
 */int i=MINDSTS; i<MAXDSTS && i<ndsts; i++) align |= ptrAlign128(dsts[i]);

/* 中文逐项-变量 V0438：offset
 * offset（Int；函数 ReduceOrCopyMulti 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  Int offset = 0;
  if (align == 0) {
    // fast path: use 128b loads/stores to do the bulk of the work,
    // assuming the pointers we have are all 128-bit aligned.

    // main loop
/* 中文逐项-变量 V0439：Npack
 * Npack（Int；函数 ReduceOrCopyMulti 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 Npack = (Nrem / (PACKELEMS*UNROLL*WARP_SIZE)) * (UNROLL*WARP_SIZE)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    Int Npack = (Nrem / (PACKELEMS*UNROLL*WARP_SIZE)) * (UNROLL*WARP_SIZE); // round down
/* 中文逐项-变量 V0440：Nelem
 * Nelem（Int；函数 ReduceOrCopyMulti 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 Nelem = Npack * PACKELEMS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    Int Nelem = Npack * PACKELEMS;

    ReduceCopy128bMulti<FUNC, T, UNROLL, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>
      (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Npack);

    Nrem -= Nelem;
    if (Nrem == 0) return;
    offset += Nelem;

    // slightly less optimized for section when we don't have full unrolling
    Npack = Nrem / PACKELEMS;
    Nelem = Npack * PACKELEMS;

    ReduceCopy128bMulti<FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>
      (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Npack);

    Nrem -= Nelem;
    if (Nrem == 0) return;
    offset += Nelem;
  }

  // unrolled, by-type (mostly for unaligned buffers)
/* 中文逐项-变量 V0441：Nelem
 * Nelem（Int；函数 ReduceOrCopyMulti 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 Nelem = (Nrem / (UNROLL*PACKELEMS/2*WARP_SIZE)) *
 * (UNROLL*PACKELEMS/2*WARP_SIZE)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  Int Nelem = (Nrem / (UNROLL*PACKELEMS/2*WARP_SIZE)) * (UNROLL*PACKELEMS/2*WARP_SIZE); // round down

  ReduceCopyMulti<FUNC, T, UNROLL*PACKELEMS/2, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>
    (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Nelem);

  Nrem -= Nelem;
  if (Nrem == 0) return;
  offset += Nelem;

  // no unroll, by type. Should finish what's remaining.
  ReduceCopyMulti<FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>
    (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Nrem);
}

#endif // COMMON_KERNEL_H_
