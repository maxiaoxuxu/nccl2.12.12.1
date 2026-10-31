/*************************************************************************
 * Copyright (c) 2015-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/


#ifndef NCCL_REDUCE_KERNEL_H_
/* 中文逐项-常量 X0015：NCCL_REDUCE_KERNEL_H_
 * 编译期定义：NCCL_REDUCE_KERNEL_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_REDUCE_KERNEL_H_

#include "common_kernel.h"
#include <limits>
#include <type_traits>

/* 中文逐项-类型 T0083：FuncNull
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<typename T>
struct FuncNull {
  __device__ /* 中文内部逻辑 F0264：FuncNull
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0264
 */
/* 中文逐项-函数 F0264：FuncNull
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncNull(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0265：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0265
 */
/* 中文逐项-函数 F0265：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const T y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return 0;。CHECK类宏还可能提前返回/跳转。
 */T operator()(const T x, const T y) const {
    return 0;
  }
};

/* 中文逐项-类型 T0084：FuncSum
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<typename T>
/* 中文精读：数值操作层不负责通信
 * FuncSum/Prod/Min/Max 只实现两个 T 数值如何结合，模板特化优化 int8、half2 等打包运算。
 * 通信顺序与源/目标指针由算法和原语决定；这里没有网络请求，也不会创建新的 kernel。
 * 改变 Ring/Tree 会改变浮点累加顺序，因此数学语义相同不意味着所有浮点结果逐位相同。
 * preOp/postOp 把每个原始输入的预处理与最终输出的后处理分开，防止每个中间跳重复处理。
 */
struct FuncSum {
  __device__ /* 中文内部逻辑 F0266：FuncSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0266
 */
/* 中文逐项-函数 F0266：FuncSum
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncSum(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0267：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x + y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0267
 */
/* 中文逐项-函数 F0267：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const T y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x + y;。CHECK类宏还可能提前返回/跳转。
 */T operator()(const T x, const T y) const {
    return x + y;
  }
};

/* 中文逐项-类型 T0085：FuncProd
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<typename T>
struct FuncProd {
  __device__ /* 中文内部逻辑 F0268：FuncProd
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0268
 */
/* 中文逐项-函数 F0268：FuncProd
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncProd(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0269：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x * y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0269
 */
/* 中文逐项-函数 F0269：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const T y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x * y;。CHECK类宏还可能提前返回/跳转。
 */T operator()(const T x, const T y) const {
    return x * y;
  }
};

/* 中文逐项-类型 T0086：FuncMax
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<typename T>
struct FuncMax {
  __device__ /* 中文内部逻辑 F0270：FuncMax
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0270
 */
/* 中文逐项-函数 F0270：FuncMax
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMax(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0271：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (x < y) ? y : x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0271
 */
/* 中文逐项-函数 F0271：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const T y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (x < y) ? y : x;。CHECK类宏还可能提前返回/跳转。
 */T operator()(const T x, const T y) const {
    return (x < y) ? y : x;
  }
};

/* 中文逐项-类型 T0087：FuncMin
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<typename T>
struct FuncMin {
  __device__ /* 中文内部逻辑 F0272：FuncMin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0272
 */
/* 中文逐项-函数 F0272：FuncMin
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMin(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0273：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (x < y) ? x : y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0273
 */
/* 中文逐项-函数 F0273：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const T y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (x < y) ? x : y;。CHECK类宏还可能提前返回/跳转。
 */T operator()(const T x, const T y) const {
    return (x < y) ? x : y;
  }
};

/* 中文逐项-类型 T0088：FuncTraits
 * 类型职责：归约函数的输入预处理/结果后处理适配层，向融合搬运模板暴露一致接口。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename Fn>
struct FuncTraits { // generic implementation for FuncSum,Prod,Min,Max
/* 中文逐项-变量 V0745：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncTraits）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = true;
/* 中文逐项-变量 V0746：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncTraits）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = true;

/* 中文内部逻辑 F0274：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0274
 */
/* 中文逐项-函数 F0274：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 <未命名参数>（Fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x;。CHECK类宏还可能提前返回/跳转。
 */
  template<typename T>
  __device__ static T preOp(Fn, T x) { return x; }
/* 中文内部逻辑 F0275：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0275
 */
/* 中文逐项-函数 F0275：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 <未命名参数>（Fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x;。CHECK类宏还可能提前返回/跳转。
 */
  template<typename T>
  __device__ static T postOp(Fn, T x) { return x; }
};

/* 中文逐项-常量 X0016：MASK0
 * 编译期定义：MASK0 = 0x00ff00ff。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MASK0 0x00ff00ff
/* 中文逐项-常量 X0017：MASK1
 * 编译期定义：MASK1 = 0xff00ff00。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MASK1 0xff00ff00
/* 中文内部逻辑 F0276：addChar4
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const uint32_t x0 = x & MASK0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：const uint32_t x1 = x & MASK1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const uint32_t y0 = y & MASK0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：const uint32_t y1 = y & MASK1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：const uint32_t r0 = (x0+y0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：const uint32_t r1 = (x1+y1);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 函数返回：return (r0 & MASK0) | (r1 & MASK1);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0276
 */
/* 中文逐项-函数 F0276：addChar4
 * 用途与位置：用位掩码分别处理一个寄存器中的四个 8 位加法，避免跨字节进位污染其他元素。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/reduce_kernel.h::operator()。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (r0 & MASK0) | (r1 & MASK1);。CHECK类宏还可能提前返回/跳转。
 */
static __device__ uint32_t addChar4(const uint32_t x, const uint32_t y) {
  /* This can be used both for signed and unsigned 8-bit addition */
/* 中文逐项-变量 V0747：x0
 * x0（uint32_t；函数 addChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 x0 = x & MASK0；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const uint32_t x0 = x & MASK0;
/* 中文逐项-变量 V0748：x1
 * x1（uint32_t；函数 addChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 x1 = x & MASK1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const uint32_t x1 = x & MASK1;
/* 中文逐项-变量 V0749：y0
 * y0（uint32_t；函数 addChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 y0 = y & MASK0；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const uint32_t y0 = y & MASK0;
/* 中文逐项-变量 V0750：y1
 * y1（uint32_t；函数 addChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 y1 = y & MASK1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const uint32_t y1 = y & MASK1;
/* 中文逐项-变量 V0751：r0
 * r0（uint32_t；函数 addChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 r0 = (x0+y0)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  const uint32_t r0 = (x0+y0);
/* 中文逐项-变量 V0752：r1
 * r1（uint32_t；函数 addChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 r1 = (x1+y1)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  const uint32_t r1 = (x1+y1);
  return (r0 & MASK0) | (r1 & MASK1);
}

/* 中文逐项-类型 T0089：FuncSum<int8_t>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncSum<int8_t> {
  __device__ /* 中文内部逻辑 F0277：FuncSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0277
 */
/* 中文逐项-函数 F0277：FuncSum
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncSum(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0278：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：int32_t rv, z=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 调用 / 计算 / 写入：asm("vadd4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   4. 函数返回：return rv;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   5. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     6. 函数返回：return addChar4(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0278
 */
/* 中文逐项-函数 F0278：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：addChar4。
 * 源码返回表达式：return rv;；return addChar4(x, y);。CHECK类宏还可能提前返回/跳转。
 */uint32_t operator()(const uint32_t x, const uint32_t y) const {
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
/* 中文逐项-变量 V0753：rv、z
 * rv（int32_t；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：asm("vadd4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y),
 * "r"(z));。若由输出参数填入，必须先检查填充调用成功再读值。
 * z（int32_t；函数 operator() 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 z = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int32_t rv, z=0;
    asm("vadd4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
    return addChar4(x, y);
#endif
  }
  __device__ /* 中文内部逻辑 F0279：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x+y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0279
 */
/* 中文逐项-函数 F0279：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const int8_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const int8_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x+y;。CHECK类宏还可能提前返回/跳转。
 */int8_t operator()(const int8_t x, const int8_t y) const {
    return x+y;
  }
};
/* 中文逐项-类型 T0090：FuncSum<uint8_t>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncSum<uint8_t> {
  __device__ /* 中文内部逻辑 F0280：FuncSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0280
 */
/* 中文逐项-函数 F0280：FuncSum
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncSum(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0281：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：int32_t rv, z=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 调用 / 计算 / 写入：asm("vadd4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   4. 函数返回：return rv;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   5. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     6. 函数返回：return addChar4(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0281
 */
/* 中文逐项-函数 F0281：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：addChar4。
 * 源码返回表达式：return rv;；return addChar4(x, y);。CHECK类宏还可能提前返回/跳转。
 */uint32_t operator()(const uint32_t x, const uint32_t y) const {
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
/* 中文逐项-变量 V0754：rv、z
 * rv（int32_t；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：asm("vadd4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y),
 * "r"(z));。若由输出参数填入，必须先检查填充调用成功再读值。
 * z（int32_t；函数 operator() 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 z = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int32_t rv, z=0;
    asm("vadd4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
    return addChar4(x, y);
#endif
  }
  __device__ /* 中文内部逻辑 F0282：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x+y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0282
 */
/* 中文逐项-函数 F0282：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint8_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint8_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x+y;。CHECK类宏还可能提前返回/跳转。
 */uint8_t operator()(const uint8_t x, const uint8_t y) const {
    return x+y;
  }
};

/* 中文内部逻辑 F0283：mulChar4
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：union converter { uint32_t storage; char4 a; }；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：cr.a.x = cx.a.x * cy.a.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 调用 / 计算 / 写入：cr.a.y = cx.a.y * cy.a.y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：cr.a.z = cx.a.z * cy.a.z;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：cr.a.w = cx.a.w * cy.a.w;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0283
 */
/* 中文逐项-函数 F0283：mulChar4
 * 用途与位置：对寄存器中四个字节分别执行乘法并重组结果，用于 int8/uint8 向量化归约。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/reduce_kernel.h::operator()。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */
static __device__ uint32_t mulChar4(const uint32_t x, const uint32_t y) {
  /* This can be used both for signed and unsigned 8-bit multiplication */
/* 中文逐项-类型 T0091：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter { /* 中文逐项-变量 V0755：storage
 * storage（uint32_t；成员 converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */uint32_t storage; /* 中文逐项-变量 V0756：a
 * a（char4；成员 converter）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */char4 a; };
/* 中文逐项-变量 V0757：cx、cy、cr
 * cx（converter；函数 mulChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 mulChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 mulChar4 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a.x = cx.a.x * cy.a.x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  converter cx, cy, cr;
  cx.storage = x;
  cy.storage = y;
  cr.a.x = cx.a.x * cy.a.x;
  cr.a.y = cx.a.y * cy.a.y;
  cr.a.z = cx.a.z * cy.a.z;
  cr.a.w = cx.a.w * cy.a.w;
  return cr.storage;
}

/* 中文逐项-类型 T0092：FuncProd<int8_t>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncProd<int8_t> {
  __device__ /* 中文内部逻辑 F0284：FuncProd
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0284
 */
/* 中文逐项-函数 F0284：FuncProd
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncProd(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0285：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return mulChar4(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0285
 */
/* 中文逐项-函数 F0285：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：mulChar4。
 * 源码返回表达式：return mulChar4(x, y);。CHECK类宏还可能提前返回/跳转。
 */uint32_t operator()(const uint32_t x, const uint32_t y) const {
    return mulChar4(x, y);
  }
  __device__ /* 中文内部逻辑 F0286：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x*y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0286
 */
/* 中文逐项-函数 F0286：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const int8_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const int8_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x*y;。CHECK类宏还可能提前返回/跳转。
 */int8_t operator()(const int8_t x, const int8_t y) const {
    return x*y;
  }
};
/* 中文逐项-类型 T0093：FuncProd<uint8_t>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncProd<uint8_t> {
  __device__ /* 中文内部逻辑 F0287：FuncProd
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0287
 */
/* 中文逐项-函数 F0287：FuncProd
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncProd(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0288：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return mulChar4(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0288
 */
/* 中文逐项-函数 F0288：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：mulChar4。
 * 源码返回表达式：return mulChar4(x, y);。CHECK类宏还可能提前返回/跳转。
 */uint32_t operator()(const uint32_t x, const uint32_t y) const {
    return mulChar4(x, y);
  }
  __device__ /* 中文内部逻辑 F0289：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x*y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0289
 */
/* 中文逐项-函数 F0289：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint8_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint8_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x*y;。CHECK类宏还可能提前返回/跳转。
 */uint8_t operator()(const uint8_t x, const uint8_t y) const {
    return x*y;
  }
};

/* 中文逐项-类型 T0094：FuncMax<int8_t>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<>
struct FuncMax<int8_t> {
  __device__ /* 中文内部逻辑 F0290：FuncMax
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0290
 */
/* 中文逐项-函数 F0290：FuncMax
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMax(uint64_t opArg=0) {}
/* 中文逐项-类型 T0095：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter { /* 中文逐项-变量 V0758：storage
 * storage（uint32_t；成员 FuncMax<int8_t>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */uint32_t storage; /* 中文逐项-变量 V0759：a
 * a（char4；成员 FuncMax<int8_t>::converter）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */char4 a; };
  __device__ /* 中文内部逻辑 F0291：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：int32_t rv, z=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 调用 / 计算 / 写入：asm("vmax4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   4. 函数返回：return rv;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   5. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     6. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     7. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 调用 / 计算 / 写入：cr.a.x = max(cx.a.x, cy.a.x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 调用 / 计算 / 写入：cr.a.y = max(cx.a.y, cy.a.y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 调用 / 计算 / 写入：cr.a.z = max(cx.a.z, cy.a.z);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     12. 调用 / 计算 / 写入：cr.a.w = max(cx.a.w, cy.a.w);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0291
 */
/* 中文逐项-函数 F0291：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：max。
 * 源码返回表达式：return rv;；return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */uint32_t operator()(const uint32_t x, const uint32_t y) const {
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
/* 中文逐项-变量 V0760：rv、z
 * rv（int32_t；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：asm("vmax4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y),
 * "r"(z));。若由输出参数填入，必须先检查填充调用成功再读值。
 * z（int32_t；函数 operator() 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 z = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int32_t rv, z=0;
    asm("vmax4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
/* 中文逐项-变量 V0761：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a.x = max(cx.a.x, cy.a.x);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;
    cr.a.x = max(cx.a.x, cy.a.x);
    cr.a.y = max(cx.a.y, cy.a.y);
    cr.a.z = max(cx.a.z, cy.a.z);
    cr.a.w = max(cx.a.w, cy.a.w);
    return cr.storage;
#endif
  }
  __device__ /* 中文内部逻辑 F0292：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (x>y) ? x : y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0292
 */
/* 中文逐项-函数 F0292：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const int8_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const int8_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (x>y) ? x : y;。CHECK类宏还可能提前返回/跳转。
 */int8_t operator()(const int8_t x, const int8_t y) const {
    return (x>y) ? x : y;
  }
};
/* 中文逐项-类型 T0096：FuncMax<uint8_t>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<>
struct FuncMax<uint8_t> {
  __device__ /* 中文内部逻辑 F0293：FuncMax
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0293
 */
/* 中文逐项-函数 F0293：FuncMax
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMax(uint64_t opArg=0) {}
/* 中文逐项-类型 T0097：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter { /* 中文逐项-变量 V0762：storage
 * storage（uint32_t；成员 FuncMax<uint8_t>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */uint32_t storage; /* 中文逐项-变量 V0763：a
 * a（uchar4；成员 FuncMax<uint8_t>::converter）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */uchar4 a; };
  __device__ /* 中文内部逻辑 F0294：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：int32_t rv, z=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 调用 / 计算 / 写入：asm("vmax4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   4. 函数返回：return rv;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   5. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     6. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     7. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 调用 / 计算 / 写入：cr.a.x = max(cx.a.x, cy.a.x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 调用 / 计算 / 写入：cr.a.y = max(cx.a.y, cy.a.y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 调用 / 计算 / 写入：cr.a.z = max(cx.a.z, cy.a.z);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     12. 调用 / 计算 / 写入：cr.a.w = max(cx.a.w, cy.a.w);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0294
 */
/* 中文逐项-函数 F0294：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：max。
 * 源码返回表达式：return rv;；return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */uint32_t operator()(const uint32_t x, const uint32_t y) const {
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
/* 中文逐项-变量 V0764：rv、z
 * rv（int32_t；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：asm("vmax4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y),
 * "r"(z));。若由输出参数填入，必须先检查填充调用成功再读值。
 * z（int32_t；函数 operator() 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 z = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int32_t rv, z=0;
    asm("vmax4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
/* 中文逐项-变量 V0765：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a.x = max(cx.a.x, cy.a.x);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;
    cr.a.x = max(cx.a.x, cy.a.x);
    cr.a.y = max(cx.a.y, cy.a.y);
    cr.a.z = max(cx.a.z, cy.a.z);
    cr.a.w = max(cx.a.w, cy.a.w);
    return cr.storage;
#endif
  }
  __device__ /* 中文内部逻辑 F0295：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (x>y) ? x : y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0295
 */
/* 中文逐项-函数 F0295：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint8_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint8_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (x>y) ? x : y;。CHECK类宏还可能提前返回/跳转。
 */uint8_t operator()(const uint8_t x, const uint8_t y) const {
    return (x>y) ? x : y;
  }
};

/* 中文逐项-类型 T0098：FuncMin<int8_t>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<>
struct FuncMin<int8_t> {
  __device__ /* 中文内部逻辑 F0296：FuncMin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0296
 */
/* 中文逐项-函数 F0296：FuncMin
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMin(uint64_t opArg=0) {}
/* 中文逐项-类型 T0099：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter { /* 中文逐项-变量 V0766：storage
 * storage（uint32_t；成员 FuncMin<int8_t>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */uint32_t storage; /* 中文逐项-变量 V0767：a
 * a（char4；成员 FuncMin<int8_t>::converter）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */char4 a; };
  __device__ /* 中文内部逻辑 F0297：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：int32_t rv, z=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 调用 / 计算 / 写入：asm("vmin4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   4. 函数返回：return rv;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   5. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     6. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     7. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 调用 / 计算 / 写入：cr.a.x = min(cx.a.x, cy.a.x);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     10. 调用 / 计算 / 写入：cr.a.y = min(cx.a.y, cy.a.y);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     11. 调用 / 计算 / 写入：cr.a.z = min(cx.a.z, cy.a.z);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     12. 调用 / 计算 / 写入：cr.a.w = min(cx.a.w, cy.a.w);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     13. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0297
 */
/* 中文逐项-函数 F0297：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：min。
 * 源码返回表达式：return rv;；return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */uint32_t operator()(const uint32_t x, const uint32_t y) const {
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
/* 中文逐项-变量 V0768：rv、z
 * rv（int32_t；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：asm("vmin4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y),
 * "r"(z));。若由输出参数填入，必须先检查填充调用成功再读值。
 * z（int32_t；函数 operator() 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 z = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int32_t rv, z=0;
    asm("vmin4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
/* 中文逐项-变量 V0769：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a.x = min(cx.a.x, cy.a.x);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;
    cr.a.x = min(cx.a.x, cy.a.x);
    cr.a.y = min(cx.a.y, cy.a.y);
    cr.a.z = min(cx.a.z, cy.a.z);
    cr.a.w = min(cx.a.w, cy.a.w);
    return cr.storage;
#endif
  }
  __device__ /* 中文内部逻辑 F0298：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (x<y) ? x : y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0298
 */
/* 中文逐项-函数 F0298：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const int8_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const int8_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (x<y) ? x : y;。CHECK类宏还可能提前返回/跳转。
 */int8_t operator()(const int8_t x, const int8_t y) const {
    return (x<y) ? x : y;
  }
};
/* 中文逐项-类型 T0100：FuncMin<uint8_t>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<>
struct FuncMin<uint8_t> {
  __device__ /* 中文内部逻辑 F0299：FuncMin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0299
 */
/* 中文逐项-函数 F0299：FuncMin
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMin(uint64_t opArg=0) {}
/* 中文逐项-类型 T0101：converter
 * 类型职责：打包寄存器与小元素/向量分量的union视图转换器，不分配新的数据缓冲。
 * 声明内可见成员（含嵌套成员）：storage、a。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union converter { /* 中文逐项-变量 V0770：storage
 * storage（uint32_t；成员 FuncMin<uint8_t>::converter）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */uint32_t storage; /* 中文逐项-变量 V0771：a
 * a（uchar4；成员 FuncMin<uint8_t>::converter）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */uchar4 a; };
  __device__ /* 中文内部逻辑 F0300：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 准备局部状态：int32_t rv, z=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 调用 / 计算 / 写入：asm("vmin4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   4. 函数返回：return rv;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   5. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     6. 准备局部状态：converter cx, cy, cr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     7. 调用 / 计算 / 写入：cx.storage = x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：cy.storage = y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 调用 / 计算 / 写入：cr.a.x = min(cx.a.x, cy.a.x);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     10. 调用 / 计算 / 写入：cr.a.y = min(cx.a.y, cy.a.y);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     11. 调用 / 计算 / 写入：cr.a.z = min(cx.a.z, cy.a.z);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     12. 调用 / 计算 / 写入：cr.a.w = min(cx.a.w, cy.a.w);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     13. 函数返回：return cr.storage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0300
 */
/* 中文逐项-函数 F0300：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint32_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint32_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：min。
 * 源码返回表达式：return rv;；return cr.storage;。CHECK类宏还可能提前返回/跳转。
 */uint32_t operator()(const uint32_t x, const uint32_t y) const {
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
/* 中文逐项-变量 V0772：rv、z
 * rv（int32_t；函数 operator() 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：asm("vmin4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y),
 * "r"(z));。若由输出参数填入，必须先检查填充调用成功再读值。
 * z（int32_t；函数 operator() 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 z = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int32_t rv, z=0;
    asm("vmin4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
/* 中文逐项-变量 V0773：cx、cy、cr
 * cx（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cx.storage = x;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cy（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cy.storage = y;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cr（converter；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cr.a.x = min(cx.a.x, cy.a.x);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;
    cr.a.x = min(cx.a.x, cy.a.x);
    cr.a.y = min(cx.a.y, cy.a.y);
    cr.a.z = min(cx.a.z, cy.a.z);
    cr.a.w = min(cx.a.w, cy.a.w);
    return cr.storage;
#endif
  }
  __device__ /* 中文内部逻辑 F0301：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (x<y) ? x : y;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0301
 */
/* 中文逐项-函数 F0301：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const uint8_t x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const uint8_t y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (x<y) ? x : y;。CHECK类宏还可能提前返回/跳转。
 */uint8_t operator()(const uint8_t x, const uint8_t y) const {
    return (x<y) ? x : y;
  }
};

/* 中文逐项-类型 T0102：FuncSum<half>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncSum<half> {
  __device__ /* 中文内部逻辑 F0302：FuncSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0302
 */
/* 中文逐项-函数 F0302：FuncSum
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncSum(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0303：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hadd2(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 准备局部状态：float2 fx, fy, fr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：fx = __half22float2(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：fy = __half22float2(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：fr.x = fx.x + fy.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：fr.y = fx.y + fy.y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 函数返回：return __float22half2_rn(fr);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0303
 */
/* 中文逐项-函数 F0303：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const half2 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const half2 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float22half2_rn；__hadd2；__half22float2。
 * 源码返回表达式：return __hadd2(x, y);；return __float22half2_rn(fr);。CHECK类宏还可能提前返回/跳转。
 */half2 operator()(const half2 x, const half2 y) const {
#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
    return __hadd2(x, y);
#else
/* 中文逐项-变量 V0774：fx、fy、fr
 * fx（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fx = __half22float2(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fy（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fy = __half22float2(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fr（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fr.x = fx.x + fy.x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float2 fx, fy, fr;
    fx = __half22float2(x);
    fy = __half22float2(y);
    fr.x = fx.x + fy.x;
    fr.y = fx.y + fy.y;
    return __float22half2_rn(fr);
#endif
  }
  __device__ /* 中文内部逻辑 F0304：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hadd(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 函数返回：return __float2half( __half2float(x) + __half2float(y) );；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0304
 */
/* 中文逐项-函数 F0304：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const half x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const half y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float2half；__hadd；__half2float。
 * 源码返回表达式：return __hadd(x, y);；return __float2half( __half2float(x) + __half2float(y) );。CHECK类宏还可能提前返回/跳转。
 */half operator()(const half x, const half y) const {
#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
    return __hadd(x, y);
#else
    return __float2half( __half2float(x) + __half2float(y) );
#endif
  }
};

#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-类型 T0103：FuncSum<__nv_bfloat16>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncSum<__nv_bfloat16> {
  __device__ /* 中文内部逻辑 F0305：FuncSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0305
 */
/* 中文逐项-函数 F0305：FuncSum
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncSum(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0306：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hadd2(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 准备局部状态：float fxl, fxh, fyl, fyh;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：fxl = __low2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：fxh = __high2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：fyl = __low2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：fyh = __high2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 函数返回：return __floats2bfloat162_rn(fxl + fyl, fxh + fyh);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0306
 */
/* 中文逐项-函数 F0306：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const __nv_bfloat162 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const __nv_bfloat162 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__floats2bfloat162_rn；__hadd2；__high2float；__low2float。
 * 源码返回表达式：return __hadd2(x, y);；return __floats2bfloat162_rn(fxl + fyl, fxh + fyh);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat162 operator()(const __nv_bfloat162 x, const __nv_bfloat162 y) const {
#if __CUDA_ARCH__ >= 800
    return __hadd2(x, y);
#else
/* 中文逐项-变量 V0775：fxl、fxh、fyl、fyh
 * fxl（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxl = __low2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fxh（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxh = __high2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fyl（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fyl = __low2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fyh（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fyh = __high2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float fxl, fxh, fyl, fyh;
    fxl = __low2float(x);
    fxh = __high2float(x);
    fyl = __low2float(y);
    fyh = __high2float(y);
    return __floats2bfloat162_rn(fxl + fyl, fxh + fyh);
#endif
   }
  __device__ /* 中文内部逻辑 F0307：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hadd(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 函数返回：return __float2bfloat16( __bfloat162float(x) + __bfloat162float(y) );；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0307
 */
/* 中文逐项-函数 F0307：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const __nv_bfloat16 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const __nv_bfloat16 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__bfloat162float；__float2bfloat16；__hadd。
 * 源码返回表达式：return __hadd(x, y);；return __float2bfloat16( __bfloat162float(x) + __bfloat162float(y)
 * );。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat16 operator()(const __nv_bfloat16 x, const __nv_bfloat16 y) const {
#if __CUDA_ARCH__ >= 800
    return __hadd(x, y);
#else
    return __float2bfloat16( __bfloat162float(x) + __bfloat162float(y) );
#endif
  }
};
#endif

/* 中文逐项-类型 T0104：FuncProd<half>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncProd<half> {
  __device__ /* 中文内部逻辑 F0308：FuncProd
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0308
 */
/* 中文逐项-函数 F0308：FuncProd
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncProd(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0309：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hmul2(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 准备局部状态：float2 fx, fy, fr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：fx = __half22float2(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：fy = __half22float2(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：fr.x = fx.x * fy.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：fr.y = fx.y * fy.y;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 函数返回：return __float22half2_rn(fr);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0309
 */
/* 中文逐项-函数 F0309：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const half2 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const half2 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float22half2_rn；__half22float2；__hmul2。
 * 源码返回表达式：return __hmul2(x, y);；return __float22half2_rn(fr);。CHECK类宏还可能提前返回/跳转。
 */half2 operator()(const half2 x, const half2 y) const {
#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
    return __hmul2(x, y);
#else
/* 中文逐项-变量 V0776：fx、fy、fr
 * fx（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fx = __half22float2(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fy（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fy = __half22float2(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fr（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fr.x = fx.x * fy.x;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float2 fx, fy, fr;
    fx = __half22float2(x);
    fy = __half22float2(y);
    fr.x = fx.x * fy.x;
    fr.y = fx.y * fy.y;
    return __float22half2_rn(fr);
#endif
  }
  __device__ /* 中文内部逻辑 F0310：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hmul(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 函数返回：return __float2half( __half2float(x) * __half2float(y) );；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0310
 */
/* 中文逐项-函数 F0310：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const half x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const half y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float2half；__half2float；__hmul。
 * 源码返回表达式：return __hmul(x, y);；return __float2half( __half2float(x) * __half2float(y) );。CHECK类宏还可能提前返回/跳转。
 */half operator()(const half x, const half y) const {
#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
    return __hmul(x, y);
#else
    return __float2half( __half2float(x) * __half2float(y) );
#endif
  }
};

#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-类型 T0105：FuncProd<__nv_bfloat16>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncProd<__nv_bfloat16> {
  __device__ /* 中文内部逻辑 F0311：FuncProd
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0311
 */
/* 中文逐项-函数 F0311：FuncProd
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncProd(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0312：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hmul2(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 准备局部状态：float fxl, fxh, fyl, fyh;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：fxl = __low2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：fxh = __high2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：fyl = __low2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：fyh = __high2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 函数返回：return __floats2bfloat162_rn(fxl * fyl, fxh * fyh);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0312
 */
/* 中文逐项-函数 F0312：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const __nv_bfloat162 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const __nv_bfloat162 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__floats2bfloat162_rn；__high2float；__hmul2；__low2float。
 * 源码返回表达式：return __hmul2(x, y);；return __floats2bfloat162_rn(fxl * fyl, fxh * fyh);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat162 operator()(const __nv_bfloat162 x, const __nv_bfloat162 y) const {
#if __CUDA_ARCH__ >= 800
    return __hmul2(x, y);
#else
/* 中文逐项-变量 V0777：fxl、fxh、fyl、fyh
 * fxl（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxl = __low2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fxh（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxh = __high2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fyl（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fyl = __low2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fyh（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fyh = __high2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float fxl, fxh, fyl, fyh;
    fxl = __low2float(x);
    fxh = __high2float(x);
    fyl = __low2float(y);
    fyh = __high2float(y);
    return __floats2bfloat162_rn(fxl * fyl, fxh * fyh);
#endif
  }
  __device__ /* 中文内部逻辑 F0313：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hmul(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 函数返回：return __float2bfloat16( __bfloat162float(x) * __bfloat162float(y) );；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0313
 */
/* 中文逐项-函数 F0313：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const __nv_bfloat16 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const __nv_bfloat16 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__bfloat162float；__float2bfloat16；__hmul。
 * 源码返回表达式：return __hmul(x, y);；return __float2bfloat16( __bfloat162float(x) * __bfloat162float(y)
 * );。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat16 operator()(const __nv_bfloat16 x, const __nv_bfloat16 y) const {
#if __CUDA_ARCH__ >= 800
    return __hmul(x, y);
#else
    return __float2bfloat16( __bfloat162float(x) * __bfloat162float(y) );
#endif
  }
};
#endif

/* 中文逐项-类型 T0106：FuncMax<half>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncMax<half> {
  __device__ /* 中文内部逻辑 F0314：FuncMax
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0314
 */
/* 中文逐项-函数 F0314：FuncMax
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMax(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0315：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：float2 fx, fy, fr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：fx = __half22float2(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：fy = __half22float2(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：fr.x = fmaxf(fx.x, fy.x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：fr.y = fmaxf(fx.y, fy.y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return __float22half2_rn(fr);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0315
 */
/* 中文逐项-函数 F0315：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const half2 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const half2 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float22half2_rn；__half22float2；fmaxf。
 * 源码返回表达式：return __float22half2_rn(fr);。CHECK类宏还可能提前返回/跳转。
 */half2 operator()(const half2 x, const half2 y) const {
/* 中文逐项-变量 V0778：fx、fy、fr
 * fx（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fx = __half22float2(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fy（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fy = __half22float2(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fr（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fr.x = fmaxf(fx.x, fy.x);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float2 fx, fy, fr;
    fx = __half22float2(x);
    fy = __half22float2(y);
    fr.x = fmaxf(fx.x, fy.x);
    fr.y = fmaxf(fx.y, fy.y);
    return __float22half2_rn(fr);
  }
  __device__ /* 中文内部逻辑 F0316：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：float fx, fy, fm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：fx = __half2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：fy = __half2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：fm = fmaxf(fx, fy);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return __float2half(fm);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0316
 */
/* 中文逐项-函数 F0316：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const half x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const half y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float2half；__half2float；fmaxf。
 * 源码返回表达式：return __float2half(fm);。CHECK类宏还可能提前返回/跳转。
 */half operator()(const half x, const half y) const {
/* 中文逐项-变量 V0779：fx、fy、fm
 * fx（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fx = __half2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fy（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fy = __half2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fm（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fm = fmaxf(fx, fy);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float fx, fy, fm;
    fx = __half2float(x);
    fy = __half2float(y);
    fm = fmaxf(fx, fy);
    return __float2half(fm);
  }
};

#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-类型 T0107：FuncMax<__nv_bfloat16>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncMax<__nv_bfloat16> {
  __device__ /* 中文内部逻辑 F0317：FuncMax
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0317
 */
/* 中文逐项-函数 F0317：FuncMax
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMax(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0318：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hmax2(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 准备局部状态：float fxl, fxh, fyl, fyh;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：fxl = __low2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：fxh = __high2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：fyl = __low2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：fyh = __high2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 函数返回：return __floats2bfloat162_rn(fmaxf(fxl, fyl), fmaxf(fxh, fyh));；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0318
 */
/* 中文逐项-函数 F0318：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const __nv_bfloat162 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const __nv_bfloat162 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__floats2bfloat162_rn；__high2float；__hmax2；__low2float；fmaxf。
 * 源码返回表达式：return __hmax2(x, y);；return __floats2bfloat162_rn(fmaxf(fxl, fyl), fmaxf(fxh, fyh));。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat162 operator()(const __nv_bfloat162 x, const __nv_bfloat162 y) const {
#if __CUDA_ARCH__ >= 800
    return __hmax2(x, y);
#else
/* 中文逐项-变量 V0780：fxl、fxh、fyl、fyh
 * fxl（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxl = __low2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fxh（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxh = __high2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fyl（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fyl = __low2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fyh（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fyh = __high2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float fxl, fxh, fyl, fyh;
    fxl = __low2float(x);
    fxh = __high2float(x);
    fyl = __low2float(y);
    fyh = __high2float(y);
    return __floats2bfloat162_rn(fmaxf(fxl, fyl), fmaxf(fxh, fyh));
#endif
  }
  __device__ /* 中文内部逻辑 F0319：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hmax(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 准备局部状态：float fx, fy;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：fx = __bfloat162float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：fy = __bfloat162float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 函数返回：return __float2bfloat16(fmaxf(fx, fy));；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0319
 */
/* 中文逐项-函数 F0319：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const __nv_bfloat16 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const __nv_bfloat16 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__bfloat162float；__float2bfloat16；__hmax；fmaxf。
 * 源码返回表达式：return __hmax(x, y);；return __float2bfloat16(fmaxf(fx, fy));。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat16 operator()(const __nv_bfloat16 x, const __nv_bfloat16 y) const {
#if __CUDA_ARCH__ >= 800
    return __hmax(x, y);
#else
/* 中文逐项-变量 V0781：fx、fy
 * fx（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fx = __bfloat162float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fy（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fy = __bfloat162float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float fx, fy;
    fx = __bfloat162float(x);
    fy = __bfloat162float(y);
    return __float2bfloat16(fmaxf(fx, fy));
#endif
  }
};
#endif

/* 中文逐项-类型 T0108：FuncMin<half>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncMin<half> {
  __device__ /* 中文内部逻辑 F0320：FuncMin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0320
 */
/* 中文逐项-函数 F0320：FuncMin
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMin(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0321：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：float2 fx, fy, fr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：fx = __half22float2(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：fy = __half22float2(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：fr.x = fminf(fx.x, fy.x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：fr.y = fminf(fx.y, fy.y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return __float22half2_rn(fr);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0321
 */
/* 中文逐项-函数 F0321：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const half2 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const half2 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float22half2_rn；__half22float2；fminf。
 * 源码返回表达式：return __float22half2_rn(fr);。CHECK类宏还可能提前返回/跳转。
 */half2 operator()(const half2 x, const half2 y) const {
/* 中文逐项-变量 V0782：fx、fy、fr
 * fx（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fx = __half22float2(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fy（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fy = __half22float2(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fr（float2；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fr.x = fminf(fx.x, fy.x);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float2 fx, fy, fr;
    fx = __half22float2(x);
    fy = __half22float2(y);
    fr.x = fminf(fx.x, fy.x);
    fr.y = fminf(fx.y, fy.y);
    return __float22half2_rn(fr);
  }
  __device__ /* 中文内部逻辑 F0322：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：float fx, fy, fm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：fx = __half2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：fy = __half2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：fm = fminf(fx, fy);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return __float2half(fm);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0322
 */
/* 中文逐项-函数 F0322：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const half x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const half y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float2half；__half2float；fminf。
 * 源码返回表达式：return __float2half(fm);。CHECK类宏还可能提前返回/跳转。
 */half operator()(const half x, const half y) const {
/* 中文逐项-变量 V0783：fx、fy、fm
 * fx（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fx = __half2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fy（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fy = __half2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fm（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fm = fminf(fx, fy);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float fx, fy, fm;
    fx = __half2float(x);
    fy = __half2float(y);
    fm = fminf(fx, fy);
    return __float2half(fm);
  }
};

#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-类型 T0109：FuncMin<__nv_bfloat16>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncMin<__nv_bfloat16> {
  __device__ /* 中文内部逻辑 F0323：FuncMin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0323
 */
/* 中文逐项-函数 F0323：FuncMin
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMin(uint64_t opArg=0) {}
   __device__ /* 中文内部逻辑 F0324：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hmin2(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 准备局部状态：float fxl, fxh, fyl, fyh;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：fxl = __low2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：fxh = __high2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：fyl = __low2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：fyh = __high2float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 函数返回：return __floats2bfloat162_rn(fminf(fxl, fyl), fminf(fxh, fyh));；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0324
 */
/* 中文逐项-函数 F0324：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const __nv_bfloat162 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const __nv_bfloat162 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__floats2bfloat162_rn；__high2float；__hmin2；__low2float；fminf。
 * 源码返回表达式：return __hmin2(x, y);；return __floats2bfloat162_rn(fminf(fxl, fyl), fminf(fxh, fyh));。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat162 operator()(const __nv_bfloat162 x, const __nv_bfloat162 y) const {
#if __CUDA_ARCH__ >= 800
    return __hmin2(x, y);
#else
/* 中文逐项-变量 V0784：fxl、fxh、fyl、fyh
 * fxl（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxl = __low2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fxh（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxh = __high2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fyl（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fyl = __low2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fyh（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fyh = __high2float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float fxl, fxh, fyl, fyh;
    fxl = __low2float(x);
    fxh = __high2float(x);
    fyl = __low2float(y);
    fyh = __high2float(y);
    return __floats2bfloat162_rn(fminf(fxl, fyl), fminf(fxh, fyh));
#endif
  }
  __device__ /* 中文内部逻辑 F0325：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 函数返回：return __hmin(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 准备局部状态：float fx, fy;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：fx = __bfloat162float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：fy = __bfloat162float(y);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 函数返回：return __float2bfloat16(fminf(fx, fy));；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0325
 */
/* 中文逐项-函数 F0325：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（const __nv_bfloat16 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（const __nv_bfloat16 y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__bfloat162float；__float2bfloat16；__hmin；fminf。
 * 源码返回表达式：return __hmin(x, y);；return __float2bfloat16(fminf(fx, fy));。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat16 operator()(const __nv_bfloat16 x, const __nv_bfloat16 y) const {
#if __CUDA_ARCH__ >= 800
    return __hmin(x, y);
#else
/* 中文逐项-变量 V0785：fx、fy
 * fx（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fx = __bfloat162float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fy（float；函数 operator() 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fy = __bfloat162float(y);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    float fx, fy;
    fx = __bfloat162float(x);
    fy = __bfloat162float(y);
    return __float2bfloat16(fminf(fx, fy));
#endif
  }
};
#endif

/* 中文逐项-类型 T0110：FuncMax<float>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncMax<float> {
  __device__ /* 中文内部逻辑 F0326：FuncMax
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0326
 */
/* 中文逐项-函数 F0326：FuncMax
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMax(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0327：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return fmaxf(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0327
 */
/* 中文逐项-函数 F0327：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（float x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（float y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fmaxf。
 * 源码返回表达式：return fmaxf(x, y);。CHECK类宏还可能提前返回/跳转。
 */float operator()(float x, float y) const {
    return fmaxf(x, y);
  }
};
/* 中文逐项-类型 T0111：FuncMin<float>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncMin<float> {
  __device__ /* 中文内部逻辑 F0328：FuncMin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0328
 */
/* 中文逐项-函数 F0328：FuncMin
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMin(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0329：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return fminf(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0329
 */
/* 中文逐项-函数 F0329：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（float x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（float y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fminf。
 * 源码返回表达式：return fminf(x, y);。CHECK类宏还可能提前返回/跳转。
 */float operator()(float x, float y) const {
    return fminf(x, y);
  }
};

/* 中文逐项-类型 T0112：FuncMax<double>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncMax<double> {
  __device__ /* 中文内部逻辑 F0330：FuncMax
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0330
 */
/* 中文逐项-函数 F0330：FuncMax
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMax(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0331：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return fmax(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0331
 */
/* 中文逐项-函数 F0331：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（double x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（double y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fmax。
 * 源码返回表达式：return fmax(x, y);。CHECK类宏还可能提前返回/跳转。
 */double operator()(double x, double y) const {
    return fmax(x, y);
  }
};
/* 中文逐项-类型 T0113：FuncMin<double>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<>
struct FuncMin<double> {
  __device__ /* 中文内部逻辑 F0332：FuncMin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0332
 */
/* 中文逐项-函数 F0332：FuncMin
 * 用途与位置：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。
 * 参数 opArg（uint64_t opArg=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncMin(uint64_t opArg=0) {}
  __device__ /* 中文内部逻辑 F0333：operator()
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return fmin(x, y);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0333
 */
/* 中文逐项-函数 F0333：operator()
 * 用途与位置：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。
 * 参数 x（double x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 参数 y（double y）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fmin。
 * 源码返回表达式：return fmin(x, y);。CHECK类宏还可能提前返回/跳转。
 */double operator()(double x, double y) const {
    return fmin(x, y);
  }
};

/* 中文逐项-类型 T0114：IsFloatingPoint
 * 类型职责：编译期数值类型判断，用来分离浮点和整数平均/缩放实现，不存放运行时张量。
 */
template<typename T>
struct IsFloatingPoint: std::false_type {};
/* 中文逐项-类型 T0115：IsFloatingPoint<half>
 * 类型职责：编译期数值类型判断，用来分离浮点和整数平均/缩放实现，不存放运行时张量。
 */
template<>
struct IsFloatingPoint<half>: std::true_type {};
#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-类型 T0116：IsFloatingPoint<__nv_bfloat16>
 * 类型职责：编译期数值类型判断，用来分离浮点和整数平均/缩放实现，不存放运行时张量。
 */
template<>
struct IsFloatingPoint<__nv_bfloat16>: std::true_type {};
#endif
/* 中文逐项-类型 T0117：IsFloatingPoint<float>
 * 类型职责：编译期数值类型判断，用来分离浮点和整数平均/缩放实现，不存放运行时张量。
 */
template<>
struct IsFloatingPoint<float>: std::true_type {};
/* 中文逐项-类型 T0118：IsFloatingPoint<double>
 * 类型职责：编译期数值类型判断，用来分离浮点和整数平均/缩放实现，不存放运行时张量。
 */
template<>
struct IsFloatingPoint<double>: std::true_type {};

template<typename T, bool IsFloating=IsFloatingPoint<T>::value>
struct FuncSumPostDiv;

/* 中文逐项-类型 T0119：FuncSumPostDiv<T, / *IsFloating=* /false>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity、n。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T>
/* 中文精读：整数平均为何最后才做除法
 * 先保持整数求和的组合运算，到结果完成的地方才对 n 做整数除法。
 * 若每个 rank 先除 n，会提前截断小数，通常得不到 sum(x)/n 的整数结果。
 * FuncTraits 把 postOp 暴露给 ReduceOrCopyMulti；算法仅在最终归约点传 postOp=true。
 * 浮点平均使用另一种 PreMulSum 策略，不能把这里的整数实现泛化到所有数据类型。
 */
struct FuncSumPostDiv<T, /*IsFloating=*/false>: FuncSum<T> {
/* 中文逐项-变量 V0786：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncSumPostDiv<T, / *IsFloating=* /false>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = true;
/* 中文逐项-变量 V0787：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncSumPostDiv<T, / *IsFloating=* /false>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = false;
/* 中文逐项-变量 V0788：n
 * n（int；成员 FuncSumPostDiv<T, / *IsFloating=* /false>）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 */
  int n;
  __device__ /* 中文内部逻辑 F0334：FuncSumPostDiv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0334
 */
/* 中文逐项-函数 F0334：FuncSumPostDiv
 * 用途与位置：保存整数平均最后需要的除数，累加阶段继续使用求和，最终输出处才应用除法。
 * 参数 opArg（uint64_t opArg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncSumPostDiv(uint64_t opArg): n(opArg) {}
  // inherits FuncSum::operator()
  __device__ /* 中文内部逻辑 F0335：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0335
 */
/* 中文逐项-函数 F0335：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x;。CHECK类宏还可能提前返回/跳转。
 */T preOp(T x) const { return x; }
  __device__ /* 中文内部逻辑 F0336：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return T(x/n);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0336
 */
/* 中文逐项-函数 F0336：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：T。
 * 源码返回表达式：return T(x/n);。CHECK类宏还可能提前返回/跳转。
 */T postOp(T x) const { return T(x/n); }
};

/* 中文逐项-类型 T0120：FuncSumPostDiv<T, / *IsFloating=* /true>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 */
template<typename T>
struct FuncSumPostDiv<T, /*IsFloating=*/true> {
  static_assert(sizeof(T)!=sizeof(T), "FuncSumPostDiv is only for implementing ncclAvg on integral types.");
};

/* 中文逐项-类型 T0121：FuncPreMulSum
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity、scale。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T>
/* 中文精读：标量值以位模式存入任务
 * opArg 是 uint64_t 容器，这里按 T 重新解释其中位模式，并非把 uint64 整数数值转换成 T。
 * 每个原始输入先乘 scale，之后仍使用继承的 Sum 归约；中间部分和不应重复乘 scale。
 * 设备指针型标量已由 common.h 的 RedopPtrDeref 解引用，才走到这里构造函数。
 * 因此调试 redOpArg 时，应同时查看 scalarArgIsPtr 和任务 datatype。
 */
struct FuncPreMulSum: FuncSum<T> { // integral T since all floats are specialized below
/* 中文逐项-变量 V0789：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncPreMulSum）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = false;
/* 中文逐项-变量 V0790：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncPreMulSum）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = true;
/* 中文逐项-变量 V0791：scale
 * scale（T；成员 FuncPreMulSum）：PreMulSum 的输入缩放标量，只对原始贡献做一次预处理。
 */
  T scale;
  __device__ /* 中文内部逻辑 F0337：FuncPreMulSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：scale = *(T*)&opArg;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0337
 */
/* 中文逐项-函数 F0337：FuncPreMulSum
 * 用途与位置：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。
 * 参数 opArg（uint64_t opArg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncPreMulSum(uint64_t opArg) { scale = *(T*)&opArg; }
  // inherits FuncSum::operator()
  __device__ /* 中文内部逻辑 F0338：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x*scale;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0338
 */
/* 中文逐项-函数 F0338：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x*scale;。CHECK类宏还可能提前返回/跳转。
 */T preOp(T x) const { return x*scale; }
  __device__ /* 中文内部逻辑 F0339：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0339
 */
/* 中文逐项-函数 F0339：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（T x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return x;。CHECK类宏还可能提前返回/跳转。
 */T postOp(T x) const { return x; }
};

/* 中文逐项-类型 T0122：FuncPreMulSum<double>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity、scale。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<>
struct FuncPreMulSum<double>: FuncSum<double> {
/* 中文逐项-变量 V0792：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncPreMulSum<double>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = false;
/* 中文逐项-变量 V0793：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncPreMulSum<double>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = true;
/* 中文逐项-变量 V0794：scale
 * scale（double；成员 FuncPreMulSum<double>）：PreMulSum 的输入缩放标量，只对原始贡献做一次预处理。
 */
  double scale;
  __device__ /* 中文内部逻辑 F0340：FuncPreMulSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：scale = *(double*)&opArg;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0340
 */
/* 中文逐项-函数 F0340：FuncPreMulSum
 * 用途与位置：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。
 * 参数 opArg（uint64_t opArg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncPreMulSum(uint64_t opArg) {
    scale = *(double*)&opArg;
  }
  // inherits FuncSum::operator()
  __device__ /* 中文内部逻辑 F0341：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPreOpIdentity ? x : x*scale;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0341
 */
/* 中文逐项-函数 F0341：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（double x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return IsPreOpIdentity ? x : x*scale;。CHECK类宏还可能提前返回/跳转。
 */double preOp(double x) const {
    return IsPreOpIdentity ? x : x*scale;
  }
  __device__ /* 中文内部逻辑 F0342：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPostOpIdentity ? x : x*scale;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0342
 */
/* 中文逐项-函数 F0342：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（double x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return IsPostOpIdentity ? x : x*scale;。CHECK类宏还可能提前返回/跳转。
 */double postOp(double x) const {
    return IsPostOpIdentity ? x : x*scale;
  }
};

/* 中文逐项-类型 T0123：FuncPreMulSum<float>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity、scale。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<>
struct FuncPreMulSum<float>: FuncSum<float> {
/* 中文逐项-变量 V0795：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncPreMulSum<float>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = false;
/* 中文逐项-变量 V0796：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncPreMulSum<float>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = true;
/* 中文逐项-变量 V0797：scale
 * scale（float；成员 FuncPreMulSum<float>）：PreMulSum 的输入缩放标量，只对原始贡献做一次预处理。
 */
  float scale;
  __device__ /* 中文内部逻辑 F0343：FuncPreMulSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：scale = *(float*)&opArg;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0343
 */
/* 中文逐项-函数 F0343：FuncPreMulSum
 * 用途与位置：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。
 * 参数 opArg（uint64_t opArg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncPreMulSum(uint64_t opArg) {
    scale = *(float*)&opArg;
  }
  // inherits FuncSum::operator()
  __device__ /* 中文内部逻辑 F0344：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPreOpIdentity ? x : x*scale;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0344
 */
/* 中文逐项-函数 F0344：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（float x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return IsPreOpIdentity ? x : x*scale;。CHECK类宏还可能提前返回/跳转。
 */float preOp(float x) const {
    return IsPreOpIdentity ? x : x*scale;
  }
  __device__ /* 中文内部逻辑 F0345：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPostOpIdentity ? x : x*scale;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0345
 */
/* 中文逐项-函数 F0345：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（float x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return IsPostOpIdentity ? x : x*scale;。CHECK类宏还可能提前返回/跳转。
 */float postOp(float x) const {
    return IsPostOpIdentity ? x : x*scale;
  }
};

/* 中文逐项-类型 T0124：FuncPreMulSum<half>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity、scale、scale。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<>
struct FuncPreMulSum<half>: FuncSum<half> {
  // Change these to switch between all prescale, all postscale, or both by sqrt(N).
  // Obviously, the only invalid combination is both true. An improvement would be
  // make this parameterized as a build time setting and passed here through
  // preprocessor definitions.
/* 中文逐项-变量 V0798：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncPreMulSum<half>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = false;
/* 中文逐项-变量 V0799：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncPreMulSum<half>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = true;

#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
/* 中文逐项-变量 V0800：scale
 * scale（half2；成员 FuncPreMulSum<half>）：PreMulSum 的输入缩放标量，只对原始贡献做一次预处理。
 */
  half2 scale;
  __device__ /* 中文内部逻辑 F0346：FuncPreMulSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：scale.x = *(half*)&opArg;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：scale.y = scale.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0346
 */
/* 中文逐项-函数 F0346：FuncPreMulSum
 * 用途与位置：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。
 * 参数 opArg（uint64_t opArg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncPreMulSum(uint64_t opArg) {
    scale.x = *(half*)&opArg;
    scale.y = scale.x;
  }
  // inherits FuncSum::operator()
  __device__ /* 中文内部逻辑 F0347：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPreOpIdentity ? x : __hmul(x, scale.x);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0347
 */
/* 中文逐项-函数 F0347：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（half x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__hmul。
 * 源码返回表达式：return IsPreOpIdentity ? x : __hmul(x, scale.x);。CHECK类宏还可能提前返回/跳转。
 */half preOp(half x) const {
    return IsPreOpIdentity ? x : __hmul(x, scale.x);
  }
  __device__ /* 中文内部逻辑 F0348：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPreOpIdentity ? x : __hmul2(x, scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0348
 */
/* 中文逐项-函数 F0348：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（half2 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__hmul2。
 * 源码返回表达式：return IsPreOpIdentity ? x : __hmul2(x, scale);。CHECK类宏还可能提前返回/跳转。
 */half2 preOp(half2 x) const {
    return IsPreOpIdentity ? x : __hmul2(x, scale);
  }
  __device__ /* 中文内部逻辑 F0349：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPostOpIdentity ? x : __hmul(x, scale.x);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0349
 */
/* 中文逐项-函数 F0349：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（half x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__hmul。
 * 源码返回表达式：return IsPostOpIdentity ? x : __hmul(x, scale.x);。CHECK类宏还可能提前返回/跳转。
 */half postOp(half x) const {
    return IsPostOpIdentity ? x : __hmul(x, scale.x);
  }
  __device__ /* 中文内部逻辑 F0350：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPostOpIdentity ? x : __hmul2(x, scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0350
 */
/* 中文逐项-函数 F0350：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（half2 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__hmul2。
 * 源码返回表达式：return IsPostOpIdentity ? x : __hmul2(x, scale);。CHECK类宏还可能提前返回/跳转。
 */half2 postOp(half2 x) const {
    return IsPostOpIdentity ? x : __hmul2(x, scale);
  }
#else
/* 中文逐项-变量 V0801：scale
 * scale（float；成员 FuncPreMulSum<half>）：PreMulSum 的输入缩放标量，只对原始贡献做一次预处理。
 */
  float scale;
  __device__ /* 中文内部逻辑 F0351：FuncPreMulSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：scale = __half2float(*(half*)&opArg);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0351
 */
/* 中文逐项-函数 F0351：FuncPreMulSum
 * 用途与位置：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。
 * 参数 opArg（uint64_t opArg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__half2float。
 */FuncPreMulSum(uint64_t opArg) {
    scale = __half2float(*(half*)&opArg);
  }
  // inherits FuncSum::operator()
  __device__ /* 中文内部逻辑 F0352：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPreOpIdentity ? x : __float2half(__half2float(x)*scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0352
 */
/* 中文逐项-函数 F0352：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（half x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float2half；__half2float。
 * 源码返回表达式：return IsPreOpIdentity ? x : __float2half(__half2float(x)*scale);。CHECK类宏还可能提前返回/跳转。
 */half preOp(half x) const {
    return IsPreOpIdentity ? x : __float2half(__half2float(x)*scale);
  }
  __device__ /* 中文内部逻辑 F0353：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(IsPreOpIdentity)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   4. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     5. 准备局部状态：float2 a = __half22float2(x);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 调用 / 计算 / 写入：a.x *= scale;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：a.y *= scale;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 函数返回：return __float22half2_rn(a);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0353
 */
/* 中文逐项-函数 F0353：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（half2 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float22half2_rn；__half22float2。
 * 需要对照的边界/推进条件：IsPreOpIdentity。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return x;；return __float22half2_rn(a);。CHECK类宏还可能提前返回/跳转。
 */half2 preOp(half2 x) const {
    if (IsPreOpIdentity)
      return x;
    else {
/* 中文逐项-变量 V0802：a
 * a（float2；函数 preOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = __half22float2(x)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      float2 a = __half22float2(x);
      a.x *= scale;
      a.y *= scale;
      return __float22half2_rn(a);
    }
  }
  __device__ /* 中文内部逻辑 F0354：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPostOpIdentity ? x : __float2half(__half2float(x)*scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0354
 */
/* 中文逐项-函数 F0354：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（half x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float2half；__half2float。
 * 源码返回表达式：return IsPostOpIdentity ? x : __float2half(__half2float(x)*scale);。CHECK类宏还可能提前返回/跳转。
 */half postOp(half x) const {
    return IsPostOpIdentity ? x : __float2half(__half2float(x)*scale);
  }
  __device__ /* 中文内部逻辑 F0355：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(IsPostOpIdentity)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   4. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     5. 准备局部状态：float2 a = __half22float2(x);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 调用 / 计算 / 写入：a.x *= scale;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：a.y *= scale;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 函数返回：return __float22half2_rn(a);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0355
 */
/* 中文逐项-函数 F0355：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（half2 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__float22half2_rn；__half22float2。
 * 需要对照的边界/推进条件：IsPostOpIdentity。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return x;；return __float22half2_rn(a);。CHECK类宏还可能提前返回/跳转。
 */half2 postOp(half2 x) const {
    if (IsPostOpIdentity)
      return x;
    else {
/* 中文逐项-变量 V0803：a
 * a（float2；函数 postOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = __half22float2(x)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      float2 a = __half22float2(x);
      a.x *= scale;
      a.y *= scale;
      return __float22half2_rn(a);
    }
  }
#endif
};

#if defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-类型 T0125：FuncPreMulSum<__nv_bfloat16>
 * 类型职责：当前数值类型的归约函数/特化，operator()负责组合，preOp/postOp负责只应执行一次的输入/结果变换。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity、scale、scale。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<>
struct FuncPreMulSum<__nv_bfloat16>: FuncSum<__nv_bfloat16> {
  // Change these to switch between all prescale, all postscale, or both by sqrt(N).
  // Obviously, the only invalid combination is both true. An improvement would be
  // make this parameterized as a build time setting and passed here through
  // preprocessor definitions.
/* 中文逐项-变量 V0804：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncPreMulSum<__nv_bfloat16>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = false;
/* 中文逐项-变量 V0805：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncPreMulSum<__nv_bfloat16>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = true;

#if __CUDA_ARCH__ >= 800
/* 中文逐项-变量 V0806：scale
 * scale（__nv_bfloat162；成员 FuncPreMulSum<__nv_bfloat16>）：PreMulSum 的输入缩放标量，只对原始贡献做一次预处理。
 */
  __nv_bfloat162 scale;
  __device__ /* 中文内部逻辑 F0356：FuncPreMulSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：scale.x = *(__nv_bfloat16*)&opArg;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：scale.y = scale.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0356
 */
/* 中文逐项-函数 F0356：FuncPreMulSum
 * 用途与位置：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。
 * 参数 opArg（uint64_t opArg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncPreMulSum(uint64_t opArg) {
    scale.x = *(__nv_bfloat16*)&opArg;
    scale.y = scale.x;
  }
  // inherits FuncSum::operator()
  __device__ /* 中文内部逻辑 F0357：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPreOpIdentity ? x : __hmul(x, scale.x);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0357
 */
/* 中文逐项-函数 F0357：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（__nv_bfloat16 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__hmul。
 * 源码返回表达式：return IsPreOpIdentity ? x : __hmul(x, scale.x);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat16 preOp(__nv_bfloat16 x) const {
    return IsPreOpIdentity ? x : __hmul(x, scale.x);
  }
  __device__ /* 中文内部逻辑 F0358：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPreOpIdentity ? x : __hmul2(x, scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0358
 */
/* 中文逐项-函数 F0358：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（__nv_bfloat162 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__hmul2。
 * 源码返回表达式：return IsPreOpIdentity ? x : __hmul2(x, scale);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat162 preOp(__nv_bfloat162 x) const {
    return IsPreOpIdentity ? x : __hmul2(x, scale);
  }
  __device__ /* 中文内部逻辑 F0359：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPostOpIdentity ? x : __hmul(x, scale.x);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0359
 */
/* 中文逐项-函数 F0359：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（__nv_bfloat16 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__hmul。
 * 源码返回表达式：return IsPostOpIdentity ? x : __hmul(x, scale.x);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat16 postOp(__nv_bfloat16 x) const {
    return IsPostOpIdentity ? x : __hmul(x, scale.x);
  }
  __device__ /* 中文内部逻辑 F0360：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPostOpIdentity ? x : __hmul2(x, scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0360
 */
/* 中文逐项-函数 F0360：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（__nv_bfloat162 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__hmul2。
 * 源码返回表达式：return IsPostOpIdentity ? x : __hmul2(x, scale);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat162 postOp(__nv_bfloat162 x) const {
    return IsPostOpIdentity ? x : __hmul2(x, scale);
  }
#else
/* 中文逐项-变量 V0807：scale
 * scale（float；成员 FuncPreMulSum<__nv_bfloat16>）：PreMulSum 的输入缩放标量，只对原始贡献做一次预处理。
 */
  float scale;
  __device__ /* 中文内部逻辑 F0361：FuncPreMulSum
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：scale = *(__nv_bfloat16*)&opArg;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0361
 */
/* 中文逐项-函数 F0361：FuncPreMulSum
 * 用途与位置：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。
 * 参数 opArg（uint64_t opArg）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FuncPreMulSum(uint64_t opArg) {
    scale = *(__nv_bfloat16*)&opArg;
  }
  // inherits FuncSum::operator()
  __device__ /* 中文内部逻辑 F0362：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPreOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0362
 */
/* 中文逐项-函数 F0362：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（__nv_bfloat16 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__bfloat162float；__float2bfloat16。
 * 源码返回表达式：return IsPreOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat16 preOp(__nv_bfloat16 x) const {
    return IsPreOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);
  }
  __device__ /* 中文内部逻辑 F0363：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(IsPreOpIdentity)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   4. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     5. 准备局部状态：float fxl, fxh;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 调用 / 计算 / 写入：fxl = __low2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：fxh = __high2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 函数返回：return __floats2bfloat162_rn(fxl * scale, fxh * scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0363
 */
/* 中文逐项-函数 F0363：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 x（__nv_bfloat162 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__floats2bfloat162_rn；__high2float；__low2float。
 * 需要对照的边界/推进条件：IsPreOpIdentity。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return x;；return __floats2bfloat162_rn(fxl * scale, fxh * scale);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat162 preOp(__nv_bfloat162 x) const {
    if (IsPreOpIdentity)
      return x;
    else {
/* 中文逐项-变量 V0808：fxl、fxh
 * fxl（float；函数 preOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxl = __low2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fxh（float；函数 preOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxh = __high2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      float fxl, fxh;
      fxl = __low2float(x);
      fxh = __high2float(x);
      return __floats2bfloat162_rn(fxl * scale, fxh * scale);
    }
  }
  __device__ /* 中文内部逻辑 F0364：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return IsPostOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0364
 */
/* 中文逐项-函数 F0364：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（__nv_bfloat16 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__bfloat162float；__float2bfloat16。
 * 源码返回表达式：return IsPostOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat16 postOp(__nv_bfloat16 x) const {
    return IsPostOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);
  }
  __device__ /* 中文内部逻辑 F0365：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(IsPostOpIdentity)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return x;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   4. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     5. 准备局部状态：float fxl, fxh;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 调用 / 计算 / 写入：fxl = __low2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：fxh = __high2float(x);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 函数返回：return __floats2bfloat162_rn(fxl * scale, fxh * scale);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0365
 */
/* 中文逐项-函数 F0365：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 x（__nv_bfloat162 x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__floats2bfloat162_rn；__high2float；__low2float。
 * 需要对照的边界/推进条件：IsPostOpIdentity。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return x;；return __floats2bfloat162_rn(fxl * scale, fxh * scale);。CHECK类宏还可能提前返回/跳转。
 */__nv_bfloat162 postOp(__nv_bfloat162 x) const {
    if (IsPostOpIdentity)
      return x;
    else {
/* 中文逐项-变量 V0809：fxl、fxh
 * fxl（float；函数 postOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxl = __low2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 * fxh（float；函数 postOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fxh = __high2float(x);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      float fxl, fxh;
      fxl = __low2float(x);
      fxh = __high2float(x);
      return __floats2bfloat162_rn(fxl * scale, fxh * scale);
    }
  }
#endif
};
#endif

/* 中文逐项-类型 T0126：FuncTraits<FuncPreMulSum<T>>
 * 类型职责：归约函数的输入预处理/结果后处理适配层，向融合搬运模板暴露一致接口。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T>
struct FuncTraits<FuncPreMulSum<T>> {
/* 中文逐项-变量 V0810：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncTraits<FuncPreMulSum<T>>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = FuncPreMulSum<T>::IsPreOpIdentity;
/* 中文逐项-变量 V0811：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncTraits<FuncPreMulSum<T>>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = FuncPreMulSum<T>::IsPostOpIdentity;

/* 中文内部逻辑 F0366：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return fn.preOp(x);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0366
 */
/* 中文逐项-函数 F0366：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FuncPreMulSum<T> fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（U x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn.preOp。
 * 源码返回表达式：return fn.preOp(x);。CHECK类宏还可能提前返回/跳转。
 */
  template<typename U>
  __device__ static U preOp(FuncPreMulSum<T> fn, U x) {
    return fn.preOp(x);
  }
/* 中文内部逻辑 F0367：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return fn.postOp(x);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0367
 */
/* 中文逐项-函数 F0367：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FuncPreMulSum<T> fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（U x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn.postOp。
 * 源码返回表达式：return fn.postOp(x);。CHECK类宏还可能提前返回/跳转。
 */
  template<typename U>
  __device__ static U postOp(FuncPreMulSum<T> fn, U x) {
    return fn.postOp(x);
  }
};
/* 中文逐项-类型 T0127：FuncTraits<FuncSumPostDiv<T>>
 * 类型职责：归约函数的输入预处理/结果后处理适配层，向融合搬运模板暴露一致接口。
 * 声明内可见成员（含嵌套成员）：IsPreOpIdentity、IsPostOpIdentity。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T>
struct FuncTraits<FuncSumPostDiv<T>> {
/* 中文逐项-变量 V0812：IsPreOpIdentity
 * IsPreOpIdentity（bool；成员 FuncTraits<FuncSumPostDiv<T>>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPreOpIdentity = FuncSumPostDiv<T>::IsPreOpIdentity;
/* 中文逐项-变量 V0813：IsPostOpIdentity
 * IsPostOpIdentity（bool；成员 FuncTraits<FuncSumPostDiv<T>>）：编译期说明预处理/后处理是否恒等，帮助模板消除不需要的运算。
 */
  static constexpr bool IsPostOpIdentity = FuncSumPostDiv<T>::IsPostOpIdentity;

/* 中文内部逻辑 F0368：preOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return fn.preOp(x);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0368
 */
/* 中文逐项-函数 F0368：preOp
 * 用途与位置：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。
 * 参数 fn（FuncSumPostDiv<T> fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（U x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn.preOp。
 * 源码返回表达式：return fn.preOp(x);。CHECK类宏还可能提前返回/跳转。
 */
  template<typename U>
  __device__ static U preOp(FuncSumPostDiv<T> fn, U x) {
    return fn.preOp(x);
  }
/* 中文内部逻辑 F0369：postOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return fn.postOp(x);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_kernel_h.md#f0369
 */
/* 中文逐项-函数 F0369：postOp
 * 用途与位置：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。
 * 参数 fn（FuncSumPostDiv<T> fn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 x（U x）：本次数值运算的输入值或打包寄存器，所属T/模板决定元素类型与归约语义。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：fn.postOp。
 * 源码返回表达式：return fn.postOp(x);。CHECK类宏还可能提前返回/跳转。
 */
  template<typename U>
  __device__ static U postOp(FuncSumPostDiv<T> fn, U x) {
    return fn.postOp(x);
  }
};
#endif // REDUCE_KERNEL_H_
