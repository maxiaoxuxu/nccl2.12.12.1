/*************************************************************************
 * Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "reduce_kernel.h"
#include "common.h"

namespace {
/* 中文内部逻辑 F0129：oneRankReduce
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：ncclWork *w = &ncclShmem.work;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int tn = blockDim.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 编译期分支/指令：#pragma unroll 1；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 5. 循环：初始 int e=0;；继续条件 e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused；每轮更新 e++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 准备局部状态：ncclWorkElem *we = &w->elems[e];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   7. 准备局部状态：intptr_t eltN = we->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 准备局部状态：int bid = we->bid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   9. 准备局部状态：int bn = we->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   10. 准备局部状态：T const *src = (T const*)we->sendbuff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   11. 准备局部状态：T *dst = (T*)we->recvbuff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   12. 准备局部状态：constexpr int EltPerPack = 16/sizeof(T);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   13. 准备局部状态：intptr_t packN = (eltN + EltPerPack-1) - (eltN + EltPerPack-1)%EltPerPack;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   14. 准备局部状态：intptr_t i0 = (bid+0)*(packN/bn) + (bid+0 < packN%bn ? bid+0 : packN%bn);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   15. 准备局部状态：intptr_t i1 = (bid+1)*(packN/bn) + (bid+1 < packN%bn ? bid+1 : packN%bn);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   16. 调用 / 计算 / 写入：i0 *= EltPerPack;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   17. 调用 / 计算 / 写入：i0 = i0 < eltN ? i0 : eltN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   18. 调用 / 计算 / 写入：i1 *= EltPerPack;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   19. 调用 / 计算 / 写入：i1 = i1 < eltN ? i1 : eltN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   20. 调用 / 计算 / 写入：src += i0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   21. 调用 / 计算 / 写入：dst += i0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   22. 调用 / 计算 / 写入：ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 1> (tid, tn, &(we->redOpArg), true, 1, &src, 1, &dst, i1-i0);；统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。
 * 23. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__onerank_reduce_cu.md#f0129
 */
/* 中文逐项-函数 F0129：oneRankReduce
 * 用途与位置：单 rank 仍需执行自定义 PreMulSum 时，在 GPU 上分配各 channel 范围并处理输入缩放。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 1>。
 * 需要对照的边界/推进条件：e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused。各条件所属分支见紧邻函数体。
 */
  template<typename T, typename RedOp>
/* 中文精读：只有一个 rank 时仍可能需要 GPU 运算
 * 普通内建操作在主机 SetupCollKernel 常可退化为 D2D copy 或无需操作。
 * PreMulSum 即使只有一个 rank，也必须把每个输入乘标量，因此保留这条设备执行路径。
 * 这里按 channel 计算局部范围，然后调用同一个 ReduceOrCopyMulti；没有跨 peer 通信。
 * 阅读下面 packN/i0/i1 时请按本版实际表达式跟踪，不要把单 rank 分段公式代入多 rank Ring。
 */
  __device__ __forceinline__ void oneRankReduce() {
/* 中文逐项-变量 V0444：w
 * w（ncclWork；函数 oneRankReduce 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 w = &ncclShmem.work；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    ncclWork *w = &ncclShmem.work;
/* 中文逐项-变量 V0445：tid
 * tid（int；函数 oneRankReduce 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int tid = threadIdx.x;
/* 中文逐项-变量 V0446：tn
 * tn（int；函数 oneRankReduce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 tn = blockDim.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int tn = blockDim.x;
    #pragma unroll 1
    for(/* 中文逐项-变量 V0447：e
 * e（int；函数 oneRankReduce 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 e = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused；循环体与增量表达式决定每轮如何推进。
 */int e=0; e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused; e++) {
/* 中文逐项-变量 V0448：we
 * we（ncclWorkElem；函数 oneRankReduce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 we = &w->elems[e]；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
      ncclWorkElem *we = &w->elems[e];
/* 中文逐项-变量 V0449：eltN
 * eltN（intptr_t；函数 oneRankReduce 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 eltN = we->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      intptr_t eltN = we->count;
/* 中文逐项-变量 V0450：bid
 * bid（int；函数 oneRankReduce 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = we->bid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int bid = we->bid;
/* 中文逐项-变量 V0451：bn
 * bn（int；函数 oneRankReduce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bn = we->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int bn = we->nChannels;
/* 中文逐项-变量 V0452：src
 * src（T；函数 oneRankReduce 内）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 初值/来源 src = (T const*)we->sendbuff；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      T const *src = (T const*)we->sendbuff;
/* 中文逐项-变量 V0453：dst
 * dst（T；函数 oneRankReduce 内）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 初值/来源 dst = (T*)we->recvbuff；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      T *dst = (T*)we->recvbuff;

      // each block/channel gets a roughly equal segment of 16 byte packs
/* 中文逐项-变量 V0454：EltPerPack
 * EltPerPack（int；函数 oneRankReduce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 EltPerPack = 16/sizeof(T)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
      constexpr int EltPerPack = 16/sizeof(T);
/* 中文逐项-变量 V0455：packN
 * packN（intptr_t；函数 oneRankReduce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 packN = (eltN + EltPerPack-1) - (eltN + EltPerPack-1)%EltPerPack；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      intptr_t packN = (eltN + EltPerPack-1) - (eltN + EltPerPack-1)%EltPerPack;
/* 中文逐项-变量 V0456：i0
 * i0（intptr_t；函数 oneRankReduce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 i0 = (bid+0)*(packN/bn) + (bid+0 < packN%bn ? bid+0 : packN%bn)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      intptr_t i0 = (bid+0)*(packN/bn) + (bid+0 < packN%bn ? bid+0 : packN%bn);
/* 中文逐项-变量 V0457：i1
 * i1（intptr_t；函数 oneRankReduce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 i1 = (bid+1)*(packN/bn) + (bid+1 < packN%bn ? bid+1 : packN%bn)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      intptr_t i1 = (bid+1)*(packN/bn) + (bid+1 < packN%bn ? bid+1 : packN%bn);
      i0 *= EltPerPack;
      i0 = i0 < eltN ? i0 : eltN;
      i1 *= EltPerPack;
      i1 = i1 < eltN ? i1 : eltN;
      src += i0;
      dst += i0;
      ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 1>
        (tid, tn, &(we->redOpArg), true, 1, &src, 1, &dst, i1-i0);
    }
  }
}

/* 中文逐项-宏 M0035：INSTANTIATE
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 INSTANTIATE；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define INSTANTIATE(devredop, type) \
  __device__ void NCCL_ONERANK_REDUCE_NAME(devredop, type)() { \
    oneRankReduce<type, Func##devredop<type>>(); \
  }

INSTANTIATE(PreMulSum, int8_t)
INSTANTIATE(PreMulSum, uint8_t)
INSTANTIATE(PreMulSum, int32_t)
INSTANTIATE(PreMulSum, uint32_t)
INSTANTIATE(PreMulSum, int64_t)
INSTANTIATE(PreMulSum, uint64_t)
INSTANTIATE(PreMulSum, half)
#if defined(__CUDA_BF16_TYPES_EXIST__)
INSTANTIATE(PreMulSum, __nv_bfloat16)
#endif
INSTANTIATE(PreMulSum, float)
INSTANTIATE(PreMulSum, double)
