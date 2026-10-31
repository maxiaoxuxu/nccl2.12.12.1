/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_PRIMITIVES_H_
/* 中文逐项-常量 X0011：NCCL_PRIMITIVES_H_
 * 编译期定义：NCCL_PRIMITIVES_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PRIMITIVES_H_

#include <type_traits>
#include "reduce_kernel.h" // for reduction funcs
#include "common.h"

/* 中文逐项-常量 X0012：NCCL_SPINS_BEFORE_CHECK_ABORT
 * 编译期定义：NCCL_SPINS_BEFORE_CHECK_ABORT = 1000000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SPINS_BEFORE_CHECK_ABORT 1000000

/* Protocol classes: ProtoSimple, ProtoLL, ProtoLL128
 * We use these as template args to the Primtiives class instead of integral
 * enums (e.g. NCCL_PROTO_LL) because for SIMPLE we need to carry a few extra
 * numbers. Also these types hold methods which let us compute numbers important
 * to how that protocol operates with a consistent interface so that our
 * algorithm code can operate protocol parametrically.
 */
/* 中文精读：算法与协议通过模板解耦
 * 算法只说 send/recvReduceSend 等动作；协议类告诉原语每 step 的有效数据和推进粒度。
 * Simple 额外携带 SlicePerChunk、StepPerSlice、Unroll，让常量在编译时展开，减少热路径判断。
 * LL/LL128 以带 flag 的小行格式搬数据，calcBytePerStep 返回有效载荷而非全部物理字节。
 * 同一个 Ring 算法可套不同协议，不表示每种协议都支持同样的 direct 优化。
 */
/* 中文逐项-类型 T0064：ProtoSimple
 * 类型职责：协议的编译期参数与有效载荷尺寸接口，让相同算法可切换Simple/LL/LL128而不复制全部算法逻辑。
 * 声明内可见成员（含嵌套成员）：Id、SlicePerChunk、StepPerSlice、Unroll、MaxGroupWidth。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<int SlicePerChunk_1, int StepPerSlice_1, int Unroll_1 = COLL_UNROLL>
struct ProtoSimple {
/* 中文逐项-变量 V0466：Id
 * Id（int；成员 ProtoSimple）：协议类的编译期枚举编号，选择buffSizes和对应协议分支。
 */
  static constexpr int Id = NCCL_PROTO_SIMPLE;
/* 中文逐项-变量 V0467：SlicePerChunk
 * SlicePerChunk（int；成员 ProtoSimple）：Simple 协议编译期片数和每片步数，模板常量减少热路径动态分支。
 */
  static constexpr int SlicePerChunk = SlicePerChunk_1;
/* 中文逐项-变量 V0468：StepPerSlice
 * StepPerSlice（int；成员 ProtoSimple）：Simple 协议编译期片数和每片步数，模板常量减少热路径动态分支。
 */
  static constexpr int StepPerSlice = StepPerSlice_1;
/* 中文逐项-变量 V0469：Unroll
 * Unroll（int；成员 ProtoSimple）：每线程循环展开因子，增加一次处理的寄存器数据量以减少循环开销。
 */
  static constexpr int Unroll = Unroll_1;

  // Data bytes (no flags etc) in one step of the fifo queue.
  __device__ /* 中文逐项-函数 F0136：calcBytePerStep
 * 用途与位置：计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/all_reduce.h::runTreeUpDown；src/collectives/device/all_reduce.h::runTreeSplit；src/collectives/device/broadcast.h::runRing；src/collectives/device/reduce.h::runRing；src/collectives/device/reduce_scatter.h::runRing。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclShmem.comm.buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS;。CHECK类宏还可能提前返回/跳转。
 */static int calcBytePerStep() {
    return ncclShmem.comm.buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS;
  }
  // Granularity of data bytes transferred per thread.
  __device__ /* 中文逐项-函数 F0137：calcBytePerGrain
 * 用途与位置：给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/all_reduce.h::runTreeUpDown；src/collectives/device/all_reduce.h::runTreeSplit；src/collectives/device/broadcast.h::runRing；src/collectives/device/reduce.h::runRing；src/collectives/device/reduce_scatter.h::runRing。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return sizeof(uint64_t);。CHECK类宏还可能提前返回/跳转。
 */static int calcBytePerGrain() {
    return sizeof(uint64_t); // Bogus value? Nobody queries this metric for simple.
  }
  // Group width is how many consecutive group values a subchannel occupies.
/* 中文逐项-变量 V0470：MaxGroupWidth
 * MaxGroupWidth（int；成员 ProtoSimple）：该协议一个原语组占用的最大命名同步组宽度，防止并行收发组屏障冲突。
 */
  static constexpr int MaxGroupWidth = 2;
  __device__ /* 中文逐项-函数 F0138：calcGroupWidth
 * 用途与位置：返回该协议线程组需要占用的 group/barrier 编号宽度，Simple 同步 warp 可能需要更宽分组。
 * 参数 send（bool send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 nthreads（int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return send && nthreads-WARP_SIZE >= 64 ? 2 : 1;。CHECK类宏还可能提前返回/跳转。
 */static int calcGroupWidth(bool send, int nthreads) {
    return send && nthreads-WARP_SIZE >= 64 ? 2 : 1;
  }
};

/* 中文逐项-类型 T0065：ProtoLL
 * 类型职责：协议的编译期参数与有效载荷尺寸接口，让相同算法可切换Simple/LL/LL128而不复制全部算法逻辑。
 * 声明内可见成员（含嵌套成员）：Id、MaxGroupWidth。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ProtoLL {
/* 中文逐项-变量 V0471：Id
 * Id（int；成员 ProtoLL）：协议类的编译期枚举编号，选择buffSizes和对应协议分支。
 */
  static constexpr int Id = NCCL_PROTO_LL;

  // Data bytes (no flags etc) in one step of the fifo queue.
  __device__ /* 中文逐项-函数 F0139：calcBytePerStep
 * 用途与位置：计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/all_reduce.h::runTreeUpDown；src/collectives/device/all_reduce.h::runTreeSplit；src/collectives/device/broadcast.h::runRing；src/collectives/device/reduce.h::runRing；src/collectives/device/reduce_scatter.h::runRing。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclShmem.comm.buffSizes[NCCL_PROTO_LL]/NCCL_STEPS/2;。CHECK类宏还可能提前返回/跳转。
 */static int calcBytePerStep() {
    return ncclShmem.comm.buffSizes[NCCL_PROTO_LL]/NCCL_STEPS/2; // Half is data
  }
  // Granularity of data bytes transferred per thread.
  __device__ /* 中文逐项-函数 F0140：calcBytePerGrain
 * 用途与位置：给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/all_reduce.h::runTreeUpDown；src/collectives/device/all_reduce.h::runTreeSplit；src/collectives/device/broadcast.h::runRing；src/collectives/device/reduce.h::runRing；src/collectives/device/reduce_scatter.h::runRing。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return sizeof(uint64_t);。CHECK类宏还可能提前返回/跳转。
 */static int calcBytePerGrain() {
    return sizeof(uint64_t); // One 16-byte line has 8-bytes of data
  }
  // Group width is how many consecutive group values a subchannel occupies.
/* 中文逐项-变量 V0472：MaxGroupWidth
 * MaxGroupWidth（int；成员 ProtoLL）：该协议一个原语组占用的最大命名同步组宽度，防止并行收发组屏障冲突。
 */
  static constexpr int MaxGroupWidth = 1;
  __device__ /* 中文逐项-函数 F0141：calcGroupWidth
 * 用途与位置：返回该协议线程组需要占用的 group/barrier 编号宽度，Simple 同步 warp 可能需要更宽分组。
 * 参数 send（bool send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 nthreads（int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return 1;。CHECK类宏还可能提前返回/跳转。
 */static int calcGroupWidth(bool send, int nthreads) {
    return 1;
  }
};

/* 中文逐项-类型 T0066：ProtoLL128
 * 类型职责：协议的编译期参数与有效载荷尺寸接口，让相同算法可切换Simple/LL/LL128而不复制全部算法逻辑。
 * 声明内可见成员（含嵌套成员）：Id、MaxGroupWidth。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ProtoLL128 {
/* 中文逐项-变量 V0473：Id
 * Id（int；成员 ProtoLL128）：协议类的编译期枚举编号，选择buffSizes和对应协议分支。
 */
  static constexpr int Id = NCCL_PROTO_LL128;

  // Data bytes (no flags etc) in one step of the fifo queue.
  __device__ /* 中文逐项-函数 F0142：calcBytePerStep
 * 用途与位置：计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/all_reduce.h::runTreeUpDown；src/collectives/device/all_reduce.h::runTreeSplit；src/collectives/device/broadcast.h::runRing；src/collectives/device/reduce.h::runRing；src/collectives/device/reduce_scatter.h::runRing。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return
 * (ncclShmem.comm.buffSizes[NCCL_PROTO_LL128]/NCCL_STEPS)*NCCL_LL128_DATAELEMS/NCCL_LL128_LINEELEMS;。CHECK类宏还可能提前返回/跳转。
 */static int calcBytePerStep() {
    return (ncclShmem.comm.buffSizes[NCCL_PROTO_LL128]/NCCL_STEPS)*NCCL_LL128_DATAELEMS/NCCL_LL128_LINEELEMS;
  }
  // Granularity of data bytes transferred per thread.
  __device__ /* 中文逐项-函数 F0143：calcBytePerGrain
 * 用途与位置：给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::runRing；src/collectives/device/all_reduce.h::runRing；src/collectives/device/all_reduce.h::runTreeUpDown；src/collectives/device/all_reduce.h::runTreeSplit；src/collectives/device/broadcast.h::runRing；src/collectives/device/reduce.h::runRing；src/collectives/device/reduce_scatter.h::runRing。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return
 * NCCL_LL128_SHMEM_ELEMS_PER_THREAD*NCCL_LL128_DATAELEMS*sizeof(uint64_t)/NCCL_LL128_LINEELEMS;。CHECK类宏还可能提前返回/跳转。
 */static int calcBytePerGrain() {
    return NCCL_LL128_SHMEM_ELEMS_PER_THREAD*NCCL_LL128_DATAELEMS*sizeof(uint64_t)/NCCL_LL128_LINEELEMS;
  }
  // Group width is how many consecutive group values a subchannel occupies.
/* 中文逐项-变量 V0474：MaxGroupWidth
 * MaxGroupWidth（int；成员 ProtoLL128）：该协议一个原语组占用的最大命名同步组宽度，防止并行收发组屏障冲突。
 */
  static constexpr int MaxGroupWidth = 1;
  __device__ /* 中文逐项-函数 F0144：calcGroupWidth
 * 用途与位置：返回该协议线程组需要占用的 group/barrier 编号宽度，Simple 同步 warp 可能需要更宽分组。
 * 参数 send（bool send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 nthreads（int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return 1;。CHECK类宏还可能提前返回/跳转。
 */static int calcGroupWidth(bool send, int nthreads) {
    return 1;
  }
};

/* Fan (as in fan-in & fan-out) classes hold recv and send counts. The template
 * arguments are static bounds on the maximum values. Asymmetric counts are
 * independent. Symmetric is a static guarantee that nrecv==nsend, so it only
 * stores one value at runtime. This optimization save 32-bit register, but more
 * importantly uses fewer predicate registers when unrolling loops.
 */
/* 中文精读：Fan 描述同时归约/广播的邻居数量
 * FanAsymmetric 的接收数和发送数可不同，适合树的多收一发或一收多发。
 * FanSymmetric 保证二者相同，环通常是 1 收 1 发；编译器可少保存变量和分支状态。
 * 模板上限控制指针数组和循环展开，运行时 nr/ns 决定实际有效邻居数。
 * Fan 是原语的邻居度，不是 channel 数，也不是一个通信操作的 rank 总数。
 */
/* 中文逐项-类型 T0067：FanAsymmetric
 * 类型职责：编译期邻居度上界和运行时实际收发数，帮助展开多源归约/多目标广播并省略无效分支。
 * 声明内可见成员（含嵌套成员）：MaxRecv、MaxSend、nr、ns。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<int MaxRecv_, int MaxSend_>
struct FanAsymmetric {
/* 中文逐项-变量 V0475：MaxRecv、MaxSend
 * MaxRecv（int；成员 FanAsymmetric）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 * MaxSend（int；成员 FanAsymmetric）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 */
  static constexpr int MaxRecv = MaxRecv_, MaxSend = MaxSend_;
/* 中文逐项-变量 V0476：nr、ns
 * nr（int；成员 FanAsymmetric）：非对称 Fan 的运行时有效接收/发送邻居数。
 * ns（int；成员 FanAsymmetric）：非对称 Fan 的运行时有效接收/发送邻居数。
 */
  int nr, ns;
  FanAsymmetric() = default;
  __device__ /* 中文逐项-函数 F0145：FanAsymmetric
 * 用途与位置：构造当前原语的有效接收/发送邻居数量；对称版本要求收发数相同以节省状态。
 * 参数 nrecv（int nrecv）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 nsend（int nsend）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FanAsymmetric(int nrecv, int nsend): nr(nrecv), ns(nsend) {
    // assert(nrecv <= MaxRecv && nsend <= MaxSend);
  }
  __device__ /* 中文逐项-函数 F0146：nrecv
 * 用途与位置：返回本原语实际有效邻居数；编译期最大邻居度为零时直接返回零，便于消除无用分支。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return MaxRecv ? nr : 0;。CHECK类宏还可能提前返回/跳转。
 */int nrecv() const { return MaxRecv ? nr : 0; }
  __device__ /* 中文逐项-函数 F0147：nsend
 * 用途与位置：返回本原语实际有效邻居数；编译期最大邻居度为零时直接返回零，便于消除无用分支。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return MaxSend ? ns : 0;。CHECK类宏还可能提前返回/跳转。
 */int nsend() const { return MaxSend ? ns : 0; }
};

/* 中文逐项-类型 T0068：FanSymmetric
 * 类型职责：编译期邻居度上界和运行时实际收发数，帮助展开多源归约/多目标广播并省略无效分支。
 * 声明内可见成员（含嵌套成员）：MaxRecv、MaxSend、n。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<int MaxArity>
struct FanSymmetric {
/* 中文逐项-变量 V0477：MaxRecv、MaxSend
 * MaxRecv（int；成员 FanSymmetric）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 * MaxSend（int；成员 FanSymmetric）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 */
  static constexpr int MaxRecv = MaxArity, MaxSend = MaxArity;
/* 中文逐项-变量 V0478：n
 * n（int；成员 FanSymmetric）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 */
  int n;
  FanSymmetric() = default;
  __device__ /* 中文逐项-函数 F0148：FanSymmetric
 * 用途与位置：构造当前原语的有效接收/发送邻居数量；对称版本要求收发数相同以节省状态。
 * 参数 nrecv（int nrecv）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 nsend（int nsend）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */FanSymmetric(int nrecv, int nsend): n(nrecv) {
    // assert(nrecv == nsend && nrecv <= MaxArity);
  }
  __device__ /* 中文逐项-函数 F0149：nrecv
 * 用途与位置：返回本原语实际有效邻居数；编译期最大邻居度为零时直接返回零，便于消除无用分支。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return n;。CHECK类宏还可能提前返回/跳转。
 */int nrecv() const { return n; }
  __device__ /* 中文逐项-函数 F0150：nsend
 * 用途与位置：返回本原语实际有效邻居数；编译期最大邻居度为零时直接返回零，便于消除无用分支。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return n;。CHECK类宏还可能提前返回/跳转。
 */int nsend() const { return n; }
};

// The primitives class. Specialized per protocol in the other headers.
template<typename T, typename RedOp, typename Fan, int Direct, typename Proto, int P2p>
class Primitives;

// Used by LL & LL128 to implement direct members in the naive way.
/* 中文逐项-类型 T0069：PrimitivesWithoutDirect
 * 类型职责：当前协议的GPU通信原语实现/适配层，组合连接流控、数据地址、复制归约和进度发布。
 */
template<typename RealPrimitives>
/* 中文精读：同名 direct 原语不保证零拷贝
 * LL/LL128 继承这里的退化实现，directSend 实际转调普通 send，其他 direct 包装同理。
 * 这样算法层能复用相同调用接口，但协议层可以不提供直接读写用户缓冲的优化。
 * Simple 的 direct 也要看连接能力与指针交换结果；没有能力时仍走协议 FIFO。
 * 所以 direct、GPU P2P、GDR 和 GPU 发起网络操作不能仅凭函数名字画等号。
 */
struct PrimitivesWithoutDirect {
  __device__ /* 中文逐项-函数 F0151：directSend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：static_cast<RealPrimitives*>；static_cast<RealPrimitives*>(this)->send。
 */void directSend(intptr_t inpIx, intptr_t remoteOutIx, int eltN) {
    static_cast<RealPrimitives*>(this)->send(inpIx, eltN);
  }
  __device__ /* 中文逐项-函数 F0152：directSendFromOutput
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：static_cast<RealPrimitives*>；static_cast<RealPrimitives*>(this)->sendFromOutput。
 */void directSendFromOutput(intptr_t outIx, intptr_t remoteOutIx, int eltN) {
    static_cast<RealPrimitives*>(this)->sendFromOutput(outIx, eltN);
  }
  __device__ /* 中文逐项-函数 F0153：directRecv
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：static_cast<RealPrimitives*>；static_cast<RealPrimitives*>(this)->recv。
 */void directRecv(intptr_t outIx, int eltN) {
    static_cast<RealPrimitives*>(this)->recv(outIx, eltN, /*postOp=*/false);
  }
  __device__ /* 中文逐项-函数 F0154：directCopySend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：static_cast<RealPrimitives*>；static_cast<RealPrimitives*>(this)->copySend。
 */void directCopySend(intptr_t inpIx, intptr_t outIx, intptr_t remoteOutIx, int eltN, bool postOp=false) {
    static_cast<RealPrimitives*>(this)->copySend(inpIx, outIx, eltN, postOp);
  }
  __device__ /* 中文逐项-函数 F0155：directRecvCopySend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：static_cast<RealPrimitives*>；static_cast<RealPrimitives*>(this)->recvCopySend。
 */void directRecvCopySend(intptr_t outIx, intptr_t remoteOutIx, int eltN) {
    static_cast<RealPrimitives*>(this)->recvCopySend(outIx, eltN, /*postOp=*/false);
  }
  __device__ /* 中文逐项-函数 F0156：directRecvReduceCopySend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：static_cast<RealPrimitives*>；static_cast<RealPrimitives*>(this)->recvReduceCopySend。
 */void directRecvReduceCopySend(intptr_t inpIx, intptr_t outIx, intptr_t remoteOutIx, int eltN, bool postOp=false) {
    // Direct is only for the send part
    static_cast<RealPrimitives*>(this)->recvReduceCopySend(inpIx, outIx, eltN, postOp);
  }
};

#include "prims_simple.h"
#include "prims_ll.h"
#include "prims_ll128.h"
#endif
