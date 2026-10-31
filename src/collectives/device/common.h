/*************************************************************************
 * Copyright (c) 2017-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_DEVICE_COMMON_H_
/* 中文逐项-常量 X0004：NCCL_DEVICE_COMMON_H_
 * 编译期定义：NCCL_DEVICE_COMMON_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_DEVICE_COMMON_H_

#include "collectives.h"
#include "devcomm.h"
#include "op128.h"

#if __CUDA_ARCH__ >= 800
/* 中文逐项-常量 X0005：COLL_UNROLL
 * 编译期定义：COLL_UNROLL = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define COLL_UNROLL 8
#else
/* 中文逐项-常量 X0006：COLL_UNROLL
 * 编译期定义：COLL_UNROLL = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define COLL_UNROLL 4
#endif

/* 中文逐项-常量 X0007：NCCL_MAX_DEV_ARITY
 * 编译期定义：NCCL_MAX_DEV_ARITY = (NCCL_MAX_TREE_ARITY-1) // Using balanced tree instead of split tree。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MAX_DEV_ARITY (NCCL_MAX_TREE_ARITY-1)  // Using balanced tree instead of split tree

__device__ /* 中文内部逻辑 F0072：barrierReduceAny
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：uint32_t popc;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：asm ("{" ".reg .pred barr_pred;" "setp.eq.u32 barr_pred, %1, 1;" "bar.red.popc.u32 %0, 2, barr_pred;" "}" : "=r"(popc) : "r"(bit));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 函数返回：return popc != 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_h.md#f0072
 */
/* 中文逐项-函数 F0072：barrierReduceAny
 * 用途与位置：执行 block 线程投票式屏障，检查任一线程是否报告中止，使整个 block 一致退出。
 * 参数 bit（int bit）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common.h::ncclKernel。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return popc != 0;。CHECK类宏还可能提前返回/跳转。
 */inline bool barrierReduceAny(int bit) {
/* 中文逐项-变量 V0255：popc
 * popc（uint32_t；函数 barrierReduceAny 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点："bar.red.popc.u32 %0, 2, barr_pred;"。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  uint32_t popc;
  asm ("{"
    ".reg .pred barr_pred;"
    "setp.eq.u32 barr_pred, %1, 1;"
    "bar.red.popc.u32 %0, 2, barr_pred;"
  "}" : "=r"(popc) : "r"(bit));
  return popc != 0;
}

// Copy src to dst and fill extra size with zeroes
/* 中文内部逻辑 F0073：copyToShmem
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：static_assert(sizeof(Tdst)%(2*sizeof(uint64_t)) == 0 && sizeof(Tsrc)%(2*sizeof(uint64_t)) == 0, "copyToShmem needs sizes which are multiple of 16B");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 源码语法片段：static_assert(sizeof(Tdst) >= sizeof(Tsrc), "Tdst size is too small");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 3. 源码语法片段：static_assert(sizeof(Tdst) <= WARP_SIZE*2*sizeof(uint64_t), "copyToShmem limited to 512B to make sure it can always be done in one cycle");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 4. 准备局部状态：uint64_t *d = reinterpret_cast<uint64_t*>(dst);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：uint64_t const *s = reinterpret_cast<uint64_t const*>(src);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：uint64_t *shmemPtr = shmemCvtPtr(d);；把 CUDA 通用指针转换为 PTX shared 地址空间指针，供共享内存汇编指令使用。
 * 7. 准备局部状态：int offset = 2*tid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 准备局部状态：uint64_t v0, v1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 条件判断：(offset >= sizeof(Tsrc)/sizeof(uint64_t))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 调用 / 计算 / 写入：v0 = v1 = 0ULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   12. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     13. 调用 / 计算 / 写入：v0 = s[offset] ;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     14. 调用 / 计算 / 写入：v1 = s[offset+1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 条件判断：(offset < sizeof(Tdst)/sizeof(uint64_t))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   16. 真分支：；仅条件为真执行以下子项。
 *     17. 调用 / 计算 / 写入：storeShmem128(shmemPtr+offset, v0, v1);；向 SM 共享内存写出 16 字节，供同 block 的线程在屏障后消费。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_h.md#f0073
 */
/* 中文逐项-函数 F0073：copyToShmem
 * 用途与位置：由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。
 * 参数 dst（Tdst *dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 src（Tsrc const *src）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 tid（int tid）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 参数 nthreads（int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common.h::ncclKernel。
 * 本体直接出现的调用/包装：reinterpret_cast<uint64_t const*>；reinterpret_cast<uint64_t*>；shmemCvtPtr；storeShmem128。
 * 需要对照的边界/推进条件：offset >= sizeof(Tsrc)/sizeof(uint64_t；offset < sizeof(Tdst)/sizeof(uint64_t。各条件所属分支见紧邻函数体。
 */
template<typename Tdst, typename Tsrc>
__device__ void copyToShmem(Tdst *dst, Tsrc const *src, int tid, int nthreads) {
  static_assert(sizeof(Tdst)%(2*sizeof(uint64_t)) == 0 && sizeof(Tsrc)%(2*sizeof(uint64_t)) == 0,
      "copyToShmem needs sizes which are multiple of 16B");
  static_assert(sizeof(Tdst) >= sizeof(Tsrc), "Tdst size is too small");
  static_assert(sizeof(Tdst) <= WARP_SIZE*2*sizeof(uint64_t), "copyToShmem limited to 512B to make sure it can always be done in one cycle");
/* 中文逐项-变量 V0256：d
 * d（uint64_t；函数 copyToShmem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = reinterpret_cast<uint64_t*>(dst)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  uint64_t *d = reinterpret_cast<uint64_t*>(dst);
/* 中文逐项-变量 V0257：s
 * s（uint64_t；函数 copyToShmem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = reinterpret_cast<uint64_t const*>(src)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  uint64_t const *s = reinterpret_cast<uint64_t const*>(src);
/* 中文逐项-变量 V0258：shmemPtr
 * shmemPtr（uint64_t；函数 copyToShmem 内）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 初值/来源 shmemPtr = shmemCvtPtr(d)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  uint64_t *shmemPtr = shmemCvtPtr(d);
/* 中文逐项-变量 V0259：offset
 * offset（int；函数 copyToShmem 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 2*tid；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int offset = 2*tid;
/* 中文逐项-变量 V0260：v0、v1
 * v0（uint64_t；函数 copyToShmem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：v0 = v1 = 0ULL;。若由输出参数填入，必须先检查填充调用成功再读值。
 * v1（uint64_t；函数 copyToShmem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：v0 = v1 = 0ULL;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  uint64_t v0, v1;
  if (offset >= sizeof(Tsrc)/sizeof(uint64_t)) {
    v0 = v1 = 0ULL;
  } else {
    v0 = s[offset] ; v1 = s[offset+1];
  }
  if (offset < sizeof(Tdst)/sizeof(uint64_t)) storeShmem128(shmemPtr+offset, v0, v1);
}

/* 中文内部逻辑 F0074：copyToShmem
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：static_assert(sizeof(uint64_t) <= alignof(T), "Uhoh");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 准备局部状态：uint64_t *d = reinterpret_cast<uint64_t*>(dst);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：uint64_t const *s = reinterpret_cast<uint64_t const*>(src);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int t = threadIdx.x - turn;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(t < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：t += blockDim.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 准备局部状态：int n = sizeof(T)/sizeof(uint64_t);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 准备局部状态：int delta = (n + WARP_SIZE-1) & -WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 条件判断：(delta < blockDim.x)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 调用 / 计算 / 写入：turn += delta;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 条件判断：(turn >= blockDim.x)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       14. 真分支：；仅条件为真执行以下子项。
 *         15. 调用 / 计算 / 写入：turn -= blockDim.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   16. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     17. 调用 / 计算 / 写入：turn = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 调用 / 计算 / 写入：n -= t;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 调用 / 计算 / 写入：d += t;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 调用 / 计算 / 写入：s += t;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 21. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 22. 循环：初始 int i=0;；继续条件 i < divUp(sizeof(T), WARP_SIZE*sizeof(uint64_t))；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   23. 条件判断：(n > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     24. 真分支：；仅条件为真执行以下子项。
 *       25. 调用 / 计算 / 写入：*d = *s;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       26. 调用 / 计算 / 写入：d += blockDim.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       27. 调用 / 计算 / 写入：s += blockDim.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       28. 调用 / 计算 / 写入：n -= blockDim.x;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 29. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 30. 函数返回：return turn;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_h.md#f0074
 */
/* 中文逐项-函数 F0074：copyToShmem
 * 用途与位置：由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。
 * 参数 dst（T *dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 src（T const *src）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 turn（int turn=0）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common.h::ncclKernel。
 * 本体直接出现的调用/包装：divUp；reinterpret_cast<uint64_t const*>；reinterpret_cast<uint64_t*>。
 * 需要对照的边界/推进条件：t < 0；delta < blockDim.x；turn >= blockDim.x；i < divUp(sizeof(T),
 * WARP_SIZE*sizeof(uint64_t。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return turn;。CHECK类宏还可能提前返回/跳转。
 */
template<typename T>
__device__ int copyToShmem(T *dst, T const *src, int turn=0) {
  static_assert(sizeof(uint64_t) <= alignof(T), "Uhoh");
/* 中文逐项-变量 V0261：d
 * d（uint64_t；函数 copyToShmem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = reinterpret_cast<uint64_t*>(dst)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  uint64_t *d = reinterpret_cast<uint64_t*>(dst);
/* 中文逐项-变量 V0262：s
 * s（uint64_t；函数 copyToShmem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = reinterpret_cast<uint64_t const*>(src)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  uint64_t const *s = reinterpret_cast<uint64_t const*>(src);
/* 中文逐项-变量 V0263：t
 * t（int；函数 copyToShmem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = threadIdx.x - turn；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int t = threadIdx.x - turn;
  if (t < 0) t += blockDim.x;
/* 中文逐项-变量 V0264：n
 * n（int；函数 copyToShmem 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = sizeof(T)/sizeof(uint64_t)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int n = sizeof(T)/sizeof(uint64_t);

/* 中文逐项-变量 V0265：delta
 * delta（int；函数 copyToShmem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 delta = (n + WARP_SIZE-1) & -WARP_SIZE；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int delta = (n + WARP_SIZE-1) & -WARP_SIZE; // round up to warp lane 0
  if (delta < blockDim.x) {
    turn += delta;
    if (turn >= blockDim.x) turn -= blockDim.x;
  }
  else
    turn = 0;

  n -= t;
  d += t;
  s += t;
  #pragma unroll
  for (/* 中文逐项-变量 V0266：i
 * i（int；函数 copyToShmem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < divUp(sizeof(T), WARP_SIZE*sizeof(uint64_t))；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < divUp(sizeof(T), WARP_SIZE*sizeof(uint64_t)); i++) {
    if (n > 0) {
      *d = *s;
      d += blockDim.x;
      s += blockDim.x;
      n -= blockDim.x;
    }
  }
  return turn;
}

/* 中文逐项-类型 T0018：RunWorkElement
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<ncclFunc_t Fn, typename T, typename RedOp, int Algo, int Proto>
struct RunWorkElement {
  __device__ /* 中文内部逻辑 F0075：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_h.md#f0075
 */
/* 中文逐项-函数 F0075：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement。
 * 参数 <未命名参数>（ncclWorkElem*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */void run(ncclWorkElem*) {
    // Put NOT IMPLEMENTED behavior here.
  }
};

/* 中文逐项-类型 T0019：RunWork
 * 类型职责：设备work解释/分派模板，决定一条work中的多个任务怎样由线程执行。
 */
template<ncclFunc_t Fn, typename T, typename RedOp, int Algo, int Proto>
struct RunWork {
  // This __forceinline__ is necessary. The compiler was inserting a function call
  // here from the LL ncclKernel.
  __device__ __forceinline__ /* 中文内部逻辑 F0076：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int wid = threadIdx.x / WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int inc = w->header.type == ncclWorkTypeRegColl ? sizeof(ncclWorkElemReg) / sizeof(ncclWorkElem) : 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 编译期分支/指令：#pragma unroll 1；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 4. 循环：初始 int e=0;；继续条件 e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused；每轮更新 e += inc；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 条件判断：(wid < w->header.nWarps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     6. 真分支：；仅条件为真执行以下子项。
 *       7. 调用 / 计算 / 写入：RunWorkElement<Fn, T, RedOp, Algo, Proto>().run(&w->elems[e]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_h.md#f0076
 */
/* 中文逐项-函数 F0076：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWork。
 * 参数 w（ncclWork *w）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：RunWorkElement<Fn, T, RedOp, Algo, Proto>；RunWorkElement<Fn, T, RedOp, Algo, Proto>().run。
 * 需要对照的边界/推进条件：e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused；wid <
 * w->header.nWarps。各条件所属分支见紧邻函数体。
 */void run(ncclWork *w) {
/* 中文逐项-变量 V0267：wid
 * wid（int；函数 run 内）：线程在 warp 内的 lane 编号或代码定义的 warp 索引，必须以本处赋值为准。
 * 初值/来源 wid = threadIdx.x / WARP_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int wid = threadIdx.x / WARP_SIZE;
/* 中文逐项-变量 V0268：inc
 * inc（int；函数 run 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 inc = w->header.type == ncclWorkTypeRegColl ? sizeof(ncclWorkElemReg) / sizeof(ncclWorkElem) :
 * 1；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    int inc = w->header.type == ncclWorkTypeRegColl ? sizeof(ncclWorkElemReg) / sizeof(ncclWorkElem) : 1;
    #pragma unroll 1
    for(/* 中文逐项-变量 V0269：e
 * e（int；函数 run 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 e = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused；循环体与增量表达式决定每轮如何推进。
 */int e=0; e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused; e += inc) {
      if (wid < w->header.nWarps)
        RunWorkElement<Fn, T, RedOp, Algo, Proto>().run(&w->elems[e]);
    }
  }
};

typedef void(*ncclKern_t)();
/* 中文逐项-变量 V0270：ncclFuncs
 * ncclFuncs（ncclKern_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclFuncs[ncclShmem.work.header.funcIndex]();。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern __device__ ncclKern_t ncclFuncs[];

/* 中文逐项-类型 T0020：ncclShmemGroup
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：recvConns、sendConns、srcs、dsts、totalSendSize。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclShmemGroup {
/* 中文逐项-变量 V0271：recvConns
 * recvConns（ncclConnInfo；成员 ncclShmemGroup）：本block各原语组使用的接收/发送连接指针表，用于共享指针交换和数据路径选择。
 */
  ncclConnInfo *recvConns[NCCL_MAX_DIRECT_ARITY];
/* 中文逐项-变量 V0272：sendConns
 * sendConns（ncclConnInfo；成员 ncclShmemGroup）：本block各原语组使用的接收/发送连接指针表，用于共享指针交换和数据路径选择。
 */
  ncclConnInfo *sendConns[NCCL_MAX_DIRECT_ARITY];
/* 中文逐项-变量 V0273：srcs
 * srcs（void；成员 ncclShmemGroup）：当前 slice 的有效源/目标地址表，包含本地缓冲与邻居协议/直接缓冲。
 */
  void* srcs[NCCL_MAX_DIRECT_ARITY+1];
/* 中文逐项-变量 V0274：dsts
 * dsts（void；成员 ncclShmemGroup）：当前 slice 的有效源/目标地址表，包含本地缓冲与邻居协议/直接缓冲。
 */
  void* dsts[NCCL_MAX_DIRECT_ARITY+1];
/* 中文逐项-变量 V0275：totalSendSize
 * totalSendSize（int；成员 ncclShmemGroup）：本slice实际对外写出的总字节统计，确定发送发布前是否需要系统可见性fence。
 */
  int totalSendSize[NCCL_MAX_SLICE_PER_CHUNK];
};

/* 中文精读：共享内存装的是元数据和小块暂存
 * comm/channel/work 从全局内存搬到 block 共享内存，使线程重复读取时开销更低。
 * groups 的 srcs/dsts 是指针表，指向输入、输出或通信缓冲，并不装下整个用户张量。
 * LL128 的 warp 暂存与 Simple 的 groups 用 union 复用空间，由具体协议决定如何解释。
 * SM 共享内存是片上存储，与 GPU 显存不同；用户大数组通常仍在显存或映射的通信缓冲中。
 */
/* 中文逐项-类型 T0021：ncclShmemData
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：ll128warp、groups、redOpArgs、comm、channel、pad、work。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclShmemData {
/* 中文逐项-类型 T0022：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：ll128warp、groups。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V0276：ll128warp
 * ll128warp（uint64_t；成员 ncclShmemData::<匿名>）：按warp划分的LL128共享内存暂存，辅助寄存器重排和不对齐数据搬运。
 */
    uint64_t ll128warp[NCCL_LL128_MAX_NTHREADS/WARP_SIZE][NCCL_LL128_SHMEM_ELEMS_PER_THREAD*WARP_SIZE];
/* 中文逐项-变量 V0277：groups
 * groups（struct ncclShmemGroup；成员 ncclShmemData::<匿名>）：Simple原语各线程组的共享上下文，含源/目标地址表与连接引用。
 */
    struct ncclShmemGroup groups[NCCL_MAX_GROUPS];
  };
/* 中文逐项-变量 V0278：redOpArgs
 * redOpArgs（uint64_t；成员 ncclShmemData）：融合多源原语需要的标量参数数组，各输入来源可能使用自己的预处理参数。
 */
  uint64_t redOpArgs[NCCL_MAX_DIRECT_ARITY+1];
/* 中文逐项-变量 V0279：comm
 * comm（struct ncclDevComm；成员 ncclShmemData）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  struct ncclDevComm comm;
/* 中文逐项-变量 V0280：channel
 * channel（struct ncclChannel；成员 ncclShmemData）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 */
  struct ncclChannel channel;
/* 中文逐项-变量 V0281：pad
 * pad（uint64_t；成员 ncclShmemData）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
  uint64_t pad;
/* 中文逐项-变量 V0282：work
 * work（struct ncclWork；成员 ncclShmemData）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 */
  struct ncclWork work;
};
static_assert(offsetof(struct ncclShmemData, work)%16 == 0, "shmem.work needs to be 16B aligned");

/* 中文内部逻辑 F0077：ncclRedopPtrDeref
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(we->header.type != ncclWorkTypeUnused && we->redOpArgIsPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 条件判断：(we->redOpArg%2 != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       4. 真分支：；仅条件为真执行以下子项。
 *         5. 调用 / 计算 / 写入：we->redOpArg = *reinterpret_cast<uint8_t*>(we->redOpArg);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         7. 条件判断：(we->redOpArg%4 != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           8. 真分支：；仅条件为真执行以下子项。
 *             9. 调用 / 计算 / 写入：we->redOpArg = *reinterpret_cast<uint16_t*>(we->redOpArg);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           10. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             11. 条件判断：(we->redOpArg%8 != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               12. 真分支：；仅条件为真执行以下子项。
 *                 13. 调用 / 计算 / 写入：we->redOpArg = *reinterpret_cast<uint32_t*>(we->redOpArg);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               14. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                 15. 调用 / 计算 / 写入：we->redOpArg = *reinterpret_cast<uint64_t*>(we->redOpArg);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_h.md#f0077
 */
/* 中文逐项-函数 F0077：ncclRedopPtrDeref
 * 用途与位置：若归约标量以设备指针传入，按地址对齐选择可安全加载的宽度，把值的位模式放回 work。
 * 参数 we（struct ncclWorkElem* we）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/common.h::ncclKernel。
 * 本体直接出现的调用/包装：reinterpret_cast<uint16_t*>；reinterpret_cast<uint32_t*>；reinterpret_cast<uint64_t*>；reinterpret_cast<uint8_t*>。
 * 需要对照的边界/推进条件：we->header.type != ncclWorkTypeUnused && we->redOpArgIsPtr；we->redOpArg%2 != 0；we->redOpArg%4 !=
 * 0；we->redOpArg%8 != 0。各条件所属分支见紧邻函数体。
 */
static __device__ void ncclRedopPtrDeref(struct ncclWorkElem* we) {
  if (we->header.type != ncclWorkTypeUnused && we->redOpArgIsPtr) {
    /* redOpArg is a pointer to the scalar value, so we'll dereference it
     * here so that redOpArg holds the bits of the scalar going forward.
     * The tricky thing is we don't know its type T since that's encoded in
     * the funcIndex. Because it would be difficult to get sizeof(T) from
     * funcIndex, we'll cheat and just dereference the largest possible size
     * given the alignment of the pointer. We might be reading in more bytes
     * than we need but that's harmless.
     */
    if (we->redOpArg%2 != 0)
      we->redOpArg = *reinterpret_cast<uint8_t*>(we->redOpArg);
    else if (we->redOpArg%4 != 0)
      we->redOpArg = *reinterpret_cast<uint16_t*>(we->redOpArg);
    else if (we->redOpArg%8 != 0)
      we->redOpArg = *reinterpret_cast<uint32_t*>(we->redOpArg);
    else
      we->redOpArg = *reinterpret_cast<uint64_t*>(we->redOpArg);
  }
}

/* 中文逐项-变量 V0283：ncclShmemData
 * ncclShmemData（__shared__；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern __shared__ ncclShmemData ncclShmem;

/* 中文内部逻辑 F0078：ncclKernel
 * 精读：blockIdx.x选择实际channel，先把comm/channel元数据复制到共享内存。
 * 精读：channel0可用首条内联参数，其他任务从FIFO复制；取走描述后就可把主机槽标Unused。
 * 精读：按funcIndex匹配静态模板或设备函数表执行；isLast结束当前批，abort则整block一致退出。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int nthreads = blockDim.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int bid = blockIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int turn = copyToShmem(&ncclShmem.comm, comm);；由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。
 * 5. 准备局部状态：ncclChannel *channel = &((ncclDevCommAndChannels*)comm)->channels[bid];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：turn = copyToShmem(&ncclShmem.channel, channel, turn);；由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。
 * 7. 条件判断：(bid == 0 && first.header.type != ncclWorkTypeUnused)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：copyToShmem(&ncclShmem.work, &first, tid, nthreads);；由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。
 * 10. 调用 / 计算 / 写入：__syncthreads();；让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。
 * 11. 准备局部状态：ncclWork *workFifoHost = ncclShmem.channel.workFifo;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 准备局部状态：ncclWork *workFifoDev = ncclShmem.channel.workFifoDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：int workFifoIx = ncclShmem.channel.index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 条件判断：(bid == 0 && first.header.type != ncclWorkTypeUnused)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 控制转移：goto SkipLoadWork;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 17. 循环：初始 ；继续条件 (true)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   18. 调用 / 计算 / 写入：copyToShmem(&ncclShmem.work, &workFifoDev[workFifoIx], tid, nthreads);；由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。
 *   19. 准备局部状态：int aborted = tid == 0 ? *comm->abortFlag : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   20. 条件判断：(barrierReduceAny(aborted))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     21. 真分支：；仅条件为真执行以下子项。
 *       22. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *   23. 条件判断：(tid == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     24. 真分支：；仅条件为真执行以下子项。
 *       25. 调用 / 计算 / 写入：workFifoHost[workFifoIx].header.type = ncclWorkTypeUnused;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   26. 跳转标签：SkipLoadWork；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     27. 调用 / 计算 / 写入：workFifoIx = (workFifoIx + 1)%NCCL_MAX_OPS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   28. 条件判断：(tid == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     29. 真分支：；仅条件为真执行以下子项。
 *       30. 调用 / 计算 / 写入：channel->index = workFifoIx;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   31. 调用 / 计算 / 写入：__syncwarp();；让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。
 *   32. 条件判断：(ncclShmem.work.header.type == ncclWorkTypeColl)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     33. 真分支：；仅条件为真执行以下子项。
 *       34. 条件判断：(tid < NCCL_MAX_WORK_ELEMENTS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         35. 真分支：；仅条件为真执行以下子项。
 *           36. 调用 / 计算 / 写入：ncclRedopPtrDeref(&ncclShmem.work.elems[tid]);；若归约标量以设备指针传入，按地址对齐选择可安全加载的宽度，把值的位模式放回 work。
 *     37. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       38. 条件判断：(ncclShmem.work.header.type == ncclWorkTypeRegColl)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         39. 真分支：；仅条件为真执行以下子项。
 *           40. 条件判断：(tid < NCCL_MAX_WORK_ELEMENTS_REG)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             41. 真分支：；仅条件为真执行以下子项。
 *               42. 调用 / 计算 / 写入：ncclRedopPtrDeref(&ncclShmem.work.regElems[tid].elem);；若归约标量以设备指针传入，按地址对齐选择可安全加载的宽度，把值的位模式放回 work。
 *   43. 调用 / 计算 / 写入：__syncthreads();；让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。
 *   44. 条件判断：(ncclShmem.work.header.funcIndex == FnIndex)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     45. 真分支：；仅条件为真执行以下子项。
 *       46. 调用 / 计算 / 写入：RunWork<Fn, T, RedOp, Algo, Proto>().run(&ncclShmem.work);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     47. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       48. 调用 / 计算 / 写入：ncclFuncs[ncclShmem.work.header.funcIndex]();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   49. 条件判断：(ncclShmem.work.header.isLast)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     50. 真分支：；仅条件为真执行以下子项。
 *       51. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *   52. 调用 / 计算 / 写入：__syncthreads();；让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。
 * 53. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__common_h.md#f0078
 */
/* 中文逐项-函数 F0078：ncclKernel
 * 用途与位置：以 blockIdx.x 选择 channel，读取/释放描述符 FIFO 槽，再按 funcIndex 执行本批设备任务到 isLast。
 * 参数 comm（struct ncclDevComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 first（ncclWorkElem first）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：RunWork<Fn, T, RedOp, Algo, Proto>；RunWork<Fn, T, RedOp, Algo,
 * Proto>().run；__syncthreads；__syncwarp；barrierReduceAny；copyToShmem；ncclFuncs[ncclShmem.work.header.funcIndex]；ncclRedopPtrDeref。
 * 需要对照的边界/推进条件：bid == 0 && first.header.type != ncclWorkTypeUnused；bid == 0 && first.header.type !=
 * ncclWorkTypeUnused；true；barrierReduceAny(aborted。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
template<ncclFunc_t Fn, typename T, typename RedOp, int Algo, int Proto, int FnIndex>
/* 中文导读：
 * GPU 通信 kernel 的工作解释器：blockIdx.x 选择 channel，复制执行上下文到共享内存。
 * 从 FIFO 取 ncclWork，按 funcIndex 调用 RunWork 或设备函数表；遇到 isLast 退出本批任务。
 * 它可以执行一批已排队工作，不应被描述为一个永不退出的全局后台 kernel。
 */
__device__ void ncclKernel(struct ncclDevComm* comm, ncclWorkElem first)  {
/* 中文逐项-变量 V0284：tid
 * tid（int；函数 ncclKernel 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int tid = threadIdx.x;
/* 中文逐项-变量 V0285：nthreads
 * nthreads（int；函数 ncclKernel 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nthreads = blockDim.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nthreads = blockDim.x;
/* 中文逐项-变量 V0286：bid
 * bid（int；函数 ncclKernel 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = blockIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int bid = blockIdx.x;

/* 中文逐项-变量 V0287：turn
 * turn（int；函数 ncclKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 turn = copyToShmem(&ncclShmem.comm, comm)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int turn = copyToShmem(&ncclShmem.comm, comm);
  // get address of channel without incurring indirect load from ncclDevCom::channels
/* 中文逐项-变量 V0288：channel
 * channel（ncclChannel；函数 ncclKernel 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = &((ncclDevCommAndChannels*)comm)->channels[bid]；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  ncclChannel *channel = &((ncclDevCommAndChannels*)comm)->channels[bid];
  turn = copyToShmem(&ncclShmem.channel, channel, turn);

  // To optimize for latency, (only) the first operation is passed as argument.
  if (bid == 0 && first.header.type != ncclWorkTypeUnused) {
    // Copy first elem to work and zero out the rest
    copyToShmem(&ncclShmem.work, &first, tid, nthreads);
  }
  __syncthreads(); // publish ncclShmem

/* 中文逐项-变量 V0289：workFifoHost
 * workFifoHost（ncclWork；函数 ncclKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 workFifoHost = ncclShmem.channel.workFifo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclWork *workFifoHost = ncclShmem.channel.workFifo;
/* 中文逐项-变量 V0290：workFifoDev
 * workFifoDev（ncclWork；函数 ncclKernel 内）：GPU 读取的工作队列视图或 GDRCopy 主机映射地址，可能与普通主机 FIFO 分开。
 * 初值/来源 workFifoDev = ncclShmem.channel.workFifoDev；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclWork *workFifoDev = ncclShmem.channel.workFifoDev;
/* 中文逐项-变量 V0291：workFifoIx
 * workFifoIx（int；函数 ncclKernel 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 workFifoIx = ncclShmem.channel.index；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int workFifoIx = ncclShmem.channel.index;

  if (bid == 0 && first.header.type != ncclWorkTypeUnused)
    goto SkipLoadWork;

  while (true) {
    copyToShmem(&ncclShmem.work, &workFifoDev[workFifoIx], tid, nthreads);
    { // Check whether the last operation was aborted and make sure all threads exit
/* 中文逐项-变量 V0292：aborted
 * aborted（int；函数 ncclKernel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 aborted = tid == 0 ? *comm->abortFlag : 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int aborted = tid == 0 ? *comm->abortFlag : 0;
      if (barrierReduceAny(aborted)) // publish ncclShmem.work
        break;
      if (tid == 0)
        workFifoHost[workFifoIx].header.type = ncclWorkTypeUnused;
    }

  SkipLoadWork:
/* 中文精读：为什么提前释放 FIFO 槽仍然安全
 * 上面已经把整条 ncclWork 复制到共享内存，当前 block 接下来使用的是私有副本。
 * 因此可以把主机 FIFO type 清零，让 CPU 重用描述符槽；这不授权复用用户数据缓冲。
 * index 持久写回真实设备 channel，下次 kernel 从后续工作继续读取；本次遇 isLast 就退出。
 * 这是按批解释 FIFO 的 kernel，不是程序启动后永远常驻的后台循环。
 */
    workFifoIx = (workFifoIx + 1)%NCCL_MAX_OPS;
    if (tid == 0)
      channel->index = workFifoIx; // write back to real channel, not shmem shadow

    __syncwarp();
    if (ncclShmem.work.header.type == ncclWorkTypeColl) {
      if (tid < NCCL_MAX_WORK_ELEMENTS) ncclRedopPtrDeref(&ncclShmem.work.elems[tid]);
    } else if (ncclShmem.work.header.type == ncclWorkTypeRegColl) {
      if (tid < NCCL_MAX_WORK_ELEMENTS_REG) ncclRedopPtrDeref(&ncclShmem.work.regElems[tid].elem);
    }
    __syncthreads();

/* 中文精读：kernel 入口与实际设备算法可能不同
 * 若当前任务正好匹配编译期 FnIndex，直接实例化调用 RunWork；否则经 ncclFuncs 表间接分派。
 * 本版本为减少入口数量，很多非 LL/Sum 组合仍借用有限的全局 kernel 入口进入解释器。
 * 因此看到 kernel 符号含 LL/Sum，不足以断言所有 work 都实际执行 LL 或 Sum。
 * 应继续检查 work.header.funcIndex 和对应 RunWorkElement 特化。
 */
    if (ncclShmem.work.header.funcIndex == FnIndex)
      RunWork<Fn, T, RedOp, Algo, Proto>().run(&ncclShmem.work);
    else
      ncclFuncs[ncclShmem.work.header.funcIndex]();

    if (ncclShmem.work.header.isLast) break;
    __syncthreads();
  }
}

// Only generate kernels for SUM
#if NCCL_OP == 0
/* 中文逐项-宏 M0001：IMPL_COLL_KERN
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_KERN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_KERN(func, algo, proto, devredop, type, fIndex) \
__global__ void NCCL_KERN_NAME(func, algo, proto, devredop, type)(struct ncclDevComm* comm, struct ncclWorkElem first) { \
  ncclKernel<ncclFunc##func, type, Func##devredop<type>, NCCL_ALGO_##algo, NCCL_PROTO_##proto, fIndex>(comm, first); \
}
#else
/* 中文逐项-宏 M0002：IMPL_COLL_KERN
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_KERN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_KERN(func, algo, proto, devredop, type, fInded)
#endif

// Examples :     AllReduce, RING, LL,    Sum,   uint8
/* 中文逐项-宏 M0003：IMPL_COLL_FUNC
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_FUNC；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_FUNC(func, algo, proto, devredop, type) \
__device__ void NCCL_FUNC_NAME(func, algo, proto, devredop, type)() { \
  RunWork<ncclFunc##func, type, Func##devredop<type>, NCCL_ALGO_##algo, NCCL_PROTO_##proto>().run(&ncclShmem.work); \
}

// Only generate inline kernels for LL
/* 中文逐项-宏 M0004：IMPL_COLL4
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL4；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL4(func, algo, devredop, type, ncclType) \
  IMPL_COLL_FUNC(func, algo, LL,     devredop, type) \
  IMPL_COLL_FUNC(func, algo, LL128,  devredop, type) \
  IMPL_COLL_FUNC(func, algo, SIMPLE, devredop, type) \
  IMPL_COLL_KERN(func, algo, LL,     devredop, type, FUNC_INDEX(ncclFunc##func, ncclDev##devredop, ncclType, NCCL_ALGO_##algo, NCCL_PROTO_LL)) \

/* 中文逐项-宏 M0005：IMPL_COLL3
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL3；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL3(func, devredop, type, ncclType) \
  IMPL_COLL4(func, TREE,    devredop, type, ncclType) \
  IMPL_COLL4(func, RING,    devredop, type, ncclType) \
  IMPL_COLL4(func, COLLNET, devredop, type, ncclType)

#if NCCL_TYPE == 0
/* 中文逐项-宏 M0006：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, int8_t,   ncclInt8)
#elif NCCL_TYPE == 1
/* 中文逐项-宏 M0007：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, uint8_t,  ncclUint8)
#elif NCCL_TYPE == 2
/* 中文逐项-宏 M0008：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, int32_t,  ncclInt32)
#elif NCCL_TYPE == 3
/* 中文逐项-宏 M0009：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, uint32_t, ncclUint32)
#elif NCCL_TYPE == 4
/* 中文逐项-宏 M0010：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, int64_t,  ncclInt64)
#elif NCCL_TYPE == 5
/* 中文逐项-宏 M0011：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, uint64_t, ncclUint64)
#elif NCCL_TYPE == 6
/* 中文逐项-宏 M0012：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, half,     ncclFloat16)
#elif NCCL_TYPE == 7
/* 中文逐项-宏 M0013：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, float,    ncclFloat32)
#elif NCCL_TYPE == 8
/* 中文逐项-宏 M0014：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, double,   ncclFloat64)
#elif NCCL_TYPE == 9 && defined(__CUDA_BF16_TYPES_EXIST__)
/* 中文逐项-宏 M0015：IMPL_COLL2
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL2(func, devredop) IMPL_COLL3(func, devredop, __nv_bfloat16, ncclBfloat16)
#endif

// Reduction define all functions
#if NCCL_OP == 0
/* 中文逐项-宏 M0016：IMPL_COLL_R
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_R；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_R(func) IMPL_COLL2(func, Sum);
#elif NCCL_OP == 1
/* 中文逐项-宏 M0017：IMPL_COLL_R
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_R；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_R(func) IMPL_COLL2(func, Prod);
#elif NCCL_OP == 2
/* 中文逐项-宏 M0018：IMPL_COLL_R
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_R；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_R(func) IMPL_COLL2(func, Min);
#elif NCCL_OP == 3
/* 中文逐项-宏 M0019：IMPL_COLL_R
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_R；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_R(func) IMPL_COLL2(func, Max);
#elif NCCL_OP == 4
/* 中文逐项-宏 M0020：IMPL_COLL_R
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_R；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_R(func) IMPL_COLL2(func, PreMulSum);
#elif NCCL_OP == 5
  #if NCCL_TYPE < 6
/* 中文逐项-宏 M0021：IMPL_COLL_R
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_R；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
    #define IMPL_COLL_R(func) IMPL_COLL2(func, SumPostDiv);
  #else
/* 中文逐项-宏 M0022：IMPL_COLL_R
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_R；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
    #define IMPL_COLL_R(func) // skip SumPostDiv for floating point
  #endif
#endif

#if NCCL_OP == 0 && NCCL_TYPE == 0
// Copy primitives only define one function for copy
/* 中文逐项-宏 M0023：IMPL_COLL_C
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_C；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_C(func) IMPL_COLL3(func, Sum, int8_t, ncclInt8);

// Point-to-point primitives only have one function/kernel.
/* 中文逐项-宏 M0024：IMPL_COLL_P
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_P；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_P(func) \
  IMPL_COLL_FUNC(func, RING, SIMPLE, Sum, int8_t); \
  IMPL_COLL_KERN(func, RING, SIMPLE, Sum, int8_t, 0);
#else
/* 中文逐项-宏 M0025：IMPL_COLL_C
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_C；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_C(func)
/* 中文逐项-宏 M0026：IMPL_COLL_P
 * 宏职责：编译期展开设备函数声明、模板实例或函数表名字/条目；#与##用于字符串化/拼接，运行时不会按宏名额外调用一次函数。
 * 原始宏名 IMPL_COLL_P；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IMPL_COLL_P(func)
#endif

#endif
