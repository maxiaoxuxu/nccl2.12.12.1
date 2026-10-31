/*************************************************************************
 * Copyright (c) 2015-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_ALIGN_H_
/* 中文逐项-常量 X0121：NCCL_ALIGN_H_
 * 编译期定义：NCCL_ALIGN_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_ALIGN_H_

/* 中文逐项-宏 M0048：DIVUP
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 DIVUP；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define DIVUP(x, y) \
    (((x)+(y)-1)/(y))

/* 中文逐项-宏 M0049：ROUNDUP
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 ROUNDUP；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define ROUNDUP(x, y) \
    (DIVUP((x), (y))*(y))

/* 中文逐项-宏 M0050：ALIGN_SIZE
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 ALIGN_SIZE；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define ALIGN_SIZE(size, align) \
  size = ((size + (align) - 1) / (align)) * (align);

#if !__CUDA_ARCH__
  #ifndef __host__
    #define __host__
  #endif
  #ifndef __device__
    #define __device__
  #endif
#endif

/* 中文逐项-函数 F0600：divUp
 * 用途与位置：计算整数向上取整的商，避免尾数不整除时少覆盖最后一块。
 * 参数 x（X x）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 y（Y y）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/all_reduce.h::runTreeUpDown；src/collectives/device/all_reduce.h::runTreeSplit；src/collectives/device/broadcast.h::runRing；src/collectives/device/common.h::copyToShmem；src/collectives/device/prims_ll.h::LLGenericOp；其余见调用清单。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (x+y-1)/y;。CHECK类宏还可能提前返回/跳转。
 */
template<typename X, typename Y, typename Z = decltype(X()+Y())>
__host__ __device__ constexpr Z divUp(X x, Y y) {
  return (x+y-1)/y;
}

/* 中文逐项-函数 F0601：roundUp
 * 用途与位置：把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。
 * 参数 x（X x）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 y（Y y）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/broadcast.h::runRing；src/collectives/device/prims_simple.h::loadRecvConn；src/collectives/device/prims_simple.h::loadSendConn；src/collectives/device/reduce.h::runRing；src/collectives/device/reduce_scatter.h::runRing。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (x+y-1) - (x+y-1)%y;。CHECK类宏还可能提前返回/跳转。
 */
template<typename X, typename Y, typename Z = decltype(X()+Y())>
__host__ __device__ constexpr Z roundUp(X x, Y y) {
  return (x+y-1) - (x+y-1)%y;
}

// assumes second argument is a power of 2
/* 中文逐项-函数 F0602：alignUp
 * 用途与位置：把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。
 * 参数 x（X x）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 a（int a）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：Z。
 * 源码返回表达式：return (x+a-1) & Z(-a);。CHECK类宏还可能提前返回/跳转。
 */
template<typename X, typename Z = decltype(X()+int())>
__host__ __device__ constexpr Z alignUp(X x, int a) {
  return (x+a-1) & Z(-a);
}

#endif
