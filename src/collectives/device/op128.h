/*************************************************************************
 * Copyright (c) 2016-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef OP128_H_
/* 中文逐项-常量 X0010：OP128_H_
 * 编译期定义：OP128_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define OP128_H_

/* 中文精读：读 PTX 内联汇编的方法
 * ld/st.global 访问全局地址空间，ld/st.shared 访问 SM 片上共享内存，v2.u64 一次处理两个 64 位值。
 * cvta.to.shared 把通用指针转换为共享内存地址供 PTX 使用，不能把所有指针数值直接互换。
 * volatile 和 memory clobber 影响编译器访存处理；跨线程/跨设备发布仍需上层屏障和 fence。
 * 向量 load/store 的宽度不意味着对所有观察者具有整段 16B 原子性。
 */
/* 中文逐项-函数 F0130：load128
 * 用途与位置：一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。
 * 参数 ptr（const uint64_t* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 <未命名参数>（uint64_t &v0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 <未命名参数>（uint64_t &v1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::loadRegsBegin；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
inline __device__ void load128(const uint64_t* ptr, uint64_t &v0, uint64_t &v1) {
  asm volatile("ld.volatile.global.v2.u64 {%0,%1}, [%2];"
      : "=l"(v0), "=l"(v1) : "l"(ptr));
}

/* 中文逐项-函数 F0131：store128
 * 用途与位置：把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。
 * 参数 ptr（uint64_t* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 v0（uint64_t v0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 v1（uint64_t v1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::storeRegs；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
inline __device__ void store128(uint64_t* ptr, uint64_t v0, uint64_t v1) {
  asm volatile("st.volatile.global.v2.u64 [%2], {%0,%1};"
      :: "l"(v0), "l"(v1), "l"(ptr));
}

/* 中文逐项-函数 F0132：shmemCvtPtr
 * 用途与位置：把 CUDA 通用指针转换为 PTX shared 地址空间指针，供共享内存汇编指令使用。
 * 参数 shmemGenericPtr（volatile uint64_t* shmemGenericPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common.h::copyToShmem；src/collectives/device/prims_ll128.h::loadRegsBegin；src/collectives/device/prims_ll128.h::storeRegs。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return shmemAsmPtr;。CHECK类宏还可能提前返回/跳转。
 */
inline __device__ uint64_t* shmemCvtPtr(volatile uint64_t* shmemGenericPtr) {
/* 中文逐项-变量 V0458：shmemAsmPtr
 * shmemAsmPtr（uint64_t；函数 shmemCvtPtr 内）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 此处未显式给初值；后续首个文本使用点：asm volatile("cvta.to.shared.u64 %0, %1;" : "=l"(shmemAsmPtr) :
 * "l"(shmemGenericPtr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  uint64_t* shmemAsmPtr;
  asm volatile("cvta.to.shared.u64 %0, %1;" : "=l"(shmemAsmPtr) : "l"(shmemGenericPtr));
  return shmemAsmPtr;
}

/* 中文逐项-函数 F0133：loadShmem128
 * 用途与位置：从 SM 共享内存读取 16 字节，避免把共享内存地址误交给全局加载指令。
 * 参数 shmemAsmPtr（uint64_t* shmemAsmPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 <未命名参数>（uint64_t &v0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 <未命名参数>（uint64_t &v1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
inline __device__ void loadShmem128(uint64_t* shmemAsmPtr, uint64_t &v0, uint64_t &v1) {
  asm volatile("ld.volatile.shared.v2.u64 {%0,%1}, [%2];"
      : "=l"(v0), "=l"(v1) : "l"(shmemAsmPtr));
}

/* 中文逐项-函数 F0134：storeShmem128
 * 用途与位置：向 SM 共享内存写出 16 字节，供同 block 的线程在屏障后消费。
 * 参数 shmemAsmPtr（uint64_t* shmemAsmPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 v0（uint64_t v0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 v1（uint64_t v1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common.h::copyToShmem；src/collectives/device/prims_ll128.h::loadRegsBegin；src/collectives/device/prims_ll128.h::storeRegs。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
inline __device__ void storeShmem128(uint64_t* shmemAsmPtr, uint64_t v0, uint64_t v1) {
  asm volatile("st.volatile.shared.v2.u64 [%2], {%0,%1};"
      :: "l"(v0), "l"(v1), "l"(shmemAsmPtr));
}

/* 中文逐项-函数 F0135：loadShmemMisaligned128
 * 用途与位置：按元素宽度拼接可能不对齐的共享内存数据到两个 64 位寄存器。
 * 参数 ptr（T *ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 <未命名参数>（uint64_t &v0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 <未命名参数>（uint64_t &v1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::loadRegsBegin。
 * 本体直接出现的调用/包装：__funnelshift_r；int；reinterpret_cast<uint32_t*>；reinterpret_cast<uintptr_t>；uintptr_t。
 * 需要对照的边界/推进条件：sizeof(T) < 4；e < 4；sizeof(T) == 4；e < 4。各条件所属分支见紧邻函数体。
 */
template<typename T>
inline __device__ void loadShmemMisaligned128(T *ptr, uint64_t &v0, uint64_t &v1) {
/* 中文逐项-类型 T0063：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：tmp4、tmp8。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V0459：tmp4
 * tmp4（uint32_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    uint32_t tmp4[4];
/* 中文逐项-变量 V0460：tmp8
 * tmp8（uint64_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
    uint64_t tmp8[2];
  };
  if(sizeof(T) < 4) {
/* 中文逐项-变量 V0461：ptr4
 * ptr4（uint32_t；函数 loadShmemMisaligned128 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ptr4 = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(ptr) &
 * -uintptr_t(4))；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    uint32_t *ptr4 = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(ptr) & -uintptr_t(4));
    #pragma unroll
    for(/* 中文逐项-变量 V0462：e
 * e（int；函数 loadShmemMisaligned128 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 e = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：e < 4；循环体与增量表达式决定每轮如何推进。
 */int e=0; e < 4; e++) {
      // Produce 4 bytes of sub-register type by reading 2 4-byte
      // aligned values and shifting.
/* 中文逐项-变量 V0463：lo、hi
 * lo（uint32_t；函数 loadShmemMisaligned128 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：asm("ld.shared.b32 %0,[%1];" : "=r"(lo) : "l"(ptr4+e+0));。若由输出参数填入，必须先检查填充调用成功再读值。
 * hi（uint32_t；函数 loadShmemMisaligned128 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：asm("ld.shared.b32 %0,[%1];" : "=r"(hi) : "l"(ptr4+e+1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      uint32_t lo, hi;
      asm("ld.shared.b32 %0,[%1];" : "=r"(lo) : "l"(ptr4+e+0));
      asm("ld.shared.b32 %0,[%1];" : "=r"(hi) : "l"(ptr4+e+1));
      tmp4[e] = __funnelshift_r(lo, hi, 8*(int(reinterpret_cast<uintptr_t>(ptr))%4));
    }
  }
  else if(sizeof(T) == 4) {
    #pragma unroll
    for(/* 中文逐项-变量 V0464：e
 * e（int；函数 loadShmemMisaligned128 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 e = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：e < 4；循环体与增量表达式决定每轮如何推进。
 */int e=0; e < 4; e++)
      asm("ld.shared.b32 %0,[%1];" : "=r"(tmp4[e]) : "l"(ptr+e));
  }
  else /*sizeof(T)==8*/ {
    #pragma unroll
    for(/* 中文逐项-变量 V0465：e
 * e（int；函数 loadShmemMisaligned128 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 e = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：e < 2；循环体与增量表达式决定每轮如何推进。
 */int e=0; e < 2; e++)
      asm("ld.shared.b64 %0,[%1];" : "=l"(tmp8[e]) : "l"(ptr+e));
  }
  v0 = tmp8[0];
  v1 = tmp8[1];
}

#endif
