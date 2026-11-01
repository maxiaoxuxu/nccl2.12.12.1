/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "primitives.h"

namespace {
/* 中文内部逻辑 F0370：runRing
 * 精读：size是每rank recvcount，不是整输入长度；输入共有P段，逐段读取同一channel偏移。
 * 精读：先发一个段，随后接收加本地同段再转发；最后把自己的完整段写到短输出的chunkOffset。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：const int nthreads = args->header.nWarps*WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const int bid = args->bid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：const int nChannels = args->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：ncclRing *ring = &ncclShmem.channel.ring;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：int const *ringRanks = ring->devUserRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 准备局部状态：const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? REDUCESCATTER_CHUNKSTEPS : 1));；计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。
 * 8. 准备局部状态：const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2);；给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 9. 准备局部状态：const int nranks = ncclShmem.comm.nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 准备局部状态：const ssize_t loopSize = nChannels*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 准备局部状态：const ssize_t size = args->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 准备局部状态：Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0> prims(tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   14. 准备局部状态：ssize_t realChunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   15. 条件判断：(Proto::Id == NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     16. 真分支：；仅条件为真执行以下子项。
 *       17. 调用 / 计算 / 写入：realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       18. 调用 / 计算 / 写入：realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));；把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。
 *     19. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       20. 条件判断：(Proto::Id == NCCL_PROTO_LL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         21. 真分支：；仅条件为真执行以下子项。
 *           22. 调用 / 计算 / 写入：realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         23. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           24. 条件判断：(Proto::Id == NCCL_PROTO_LL128)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             25. 真分支：；仅条件为真执行以下子项。
 *               26. 调用 / 计算 / 写入：realChunkSize = min(divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128, chunkSize);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   27. 调用 / 计算 / 写入：realChunkSize = int(realChunkSize);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   28. 准备局部状态：ssize_t chunkOffset = gridOffset + bid*int(realChunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   29. 准备局部状态：ssize_t offset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   30. 准备局部状态：int nelem = min(realChunkSize, size-chunkOffset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   31. 准备局部状态：int rankDest;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   32. 调用 / 计算 / 写入：rankDest = ringRanks[nranks-1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   33. 调用 / 计算 / 写入：offset = chunkOffset + rankDest * size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   34. 调用 / 计算 / 写入：prims.send(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   35. 循环：初始 int j=2;；继续条件 j<nranks；每轮更新 ++j；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     36. 调用 / 计算 / 写入：rankDest = ringRanks[nranks-j];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     37. 调用 / 计算 / 写入：offset = chunkOffset + rankDest * size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     38. 调用 / 计算 / 写入：prims.recvReduceSend(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   39. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   40. 调用 / 计算 / 写入：rankDest = ringRanks[0];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   41. 调用 / 计算 / 写入：offset = chunkOffset + rankDest * size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   42. 调用 / 计算 / 写入：prims.recvReduceCopy(offset, chunkOffset, nelem, true);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 43. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_scatter_h.md#f0370
 */
/* 中文逐项-函数 F0370：runRing
 * 用途与位置：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进；当前API文件为 reduce_scatter。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::run；src/collectives/device/all_reduce.h::run；src/collectives/device/broadcast.h::run；src/collectives/device/reduce.h::run；src/collectives/device/reduce_scatter.h::run。
 * 本体直接出现的调用/包装：Proto::calcBytePerGrain；Proto::calcBytePerStep；divUp；int；min；prims.recvReduceCopy；prims.recvReduceSend；prims.send；roundUp。
 * 需要对照的边界/推进条件：gridOffset < size；Proto::Id == NCCL_PROTO_SIMPLE；Proto::Id == NCCL_PROTO_LL；Proto::Id ==
 * NCCL_PROTO_LL128。各条件所属分支见紧邻函数体。
 */
  template<typename T, typename RedOp, typename Proto>
/* 中文导读：
 * Ring ReduceScatter：沿环逐块接收、合并本地输入并转发，最终只保留属于本 rank 的归约分块。
 * 与完整 AllReduce 的区别是没有再传播所有结果的 AllGather 阶段。
 */
  /* 中文精读V3.1：ReduceScatter：此处size=args->count代表每rank最终输出的元素数，用户sendbuff包含nranks个这样的段。首步发送ringRanks[nranks-1]段；中间recvReduceSend把本rank相同段的输入合入前驱部分和；末步recvReduceCopy(offset,chunkOffset,...)中特别注意src offset带rankDest*size、输出chunkOffset却不带：只写紧凑的本rank结果。逐轮示意参见 docs/zh_CN/ring-primitives-deep-dive.md 第2节。 */
  __device__ __forceinline__ void runRing(ncclWorkElem *args) {
/* 中文逐项-变量 V0814：tid
 * tid（int；函数 runRing 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int tid = threadIdx.x;
/* 中文逐项-变量 V0815：nthreads
 * nthreads（int；函数 runRing 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nthreads = args->header.nWarps*WARP_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nthreads = args->header.nWarps*WARP_SIZE;
/* 中文逐项-变量 V0816：bid
 * bid（int；函数 runRing 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = args->bid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int bid = args->bid;
/* 中文逐项-变量 V0817：nChannels
 * nChannels（int；函数 runRing 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = args->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nChannels = args->nChannels;
/* 中文逐项-变量 V0818：ring
 * ring（ncclRing；函数 runRing 内）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 * 初值/来源 ring = &ncclShmem.channel.ring；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    ncclRing *ring = &ncclShmem.channel.ring;
/* 中文逐项-变量 V0819：ringRanks
 * ringRanks（int；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ringRanks = ring->devUserRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int const *ringRanks = ring->devUserRanks;
/* 中文逐项-变量 V0820：chunkSize
 * chunkSize（ssize_t；函数 runRing 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ?
 * REDUCESCATTER_CHUNKSTEPS : 1))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? REDUCESCATTER_CHUNKSTEPS : 1));
    // We should not need the final /2 but it makes performance much, much smoother. Might be a bug somewhere.
/* 中文逐项-变量 V0821：minChunkSizeLL128
 * minChunkSizeLL128（ssize_t；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minChunkSizeLL128 =
 * int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2);
/* 中文逐项-变量 V0822：nranks
 * nranks（int；函数 runRing 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = ncclShmem.comm.nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nranks = ncclShmem.comm.nRanks;
/* 中文逐项-变量 V0823：loopSize
 * loopSize（ssize_t；函数 runRing 内）：一轮全部 channel（以及 Ring ranks/CollNet heads）覆盖的逻辑数据范围。
 * 初值/来源 loopSize = nChannels*chunkSize；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const ssize_t loopSize = nChannels*chunkSize;
/* 中文逐项-变量 V0824：size
 * size（ssize_t；函数 runRing 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t size = args->count;

/* 中文逐项-变量 V0825：prims
 * prims（Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>；函数 runRing 内）：当前 GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff,
 * args->redOpArg)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>
      prims(tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);

    for (/* 中文逐项-变量 V0826：gridOffset
 * gridOffset（ssize_t；函数 runRing 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0827：realChunkSize
 * realChunkSize（ssize_t；函数 runRing 内）：结合剩余数据、并行通道和对齐粒度计算的本轮块容量，不保证每块都填满。
 * 此处未显式给初值；后续首个文本使用点：realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else if (Proto::Id == NCCL_PROTO_LL)
        realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;
      else if (Proto::Id == NCCL_PROTO_LL128)
        realChunkSize = min(divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128, chunkSize);
      realChunkSize = int(realChunkSize);

/* 中文逐项-变量 V0828：chunkOffset
 * chunkOffset（ssize_t；函数 runRing 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 chunkOffset = gridOffset + bid*int(realChunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      ssize_t chunkOffset = gridOffset + bid*int(realChunkSize);

      /////////////// begin ReduceScatter steps ///////////////
/* 中文逐项-变量 V0829：offset
 * offset（ssize_t；函数 runRing 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：offset = chunkOffset + rankDest * size;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ssize_t offset;
/* 中文逐项-变量 V0830：nelem
 * nelem（int；函数 runRing 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(realChunkSize, size-chunkOffset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
      int nelem = min(realChunkSize, size-chunkOffset);
/* 中文逐项-变量 V0831：rankDest
 * rankDest（int；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：rankDest = ringRanks[nranks-1];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int rankDest;

      // step 0: push data to next GPU
/* 中文精读：ReduceScatter 的输入输出下标
 * size 是 recvcount，即每个 rank 最终保留的 T 元素数，输入总长度 nranks*size。
 * 本 channel 处理所有 rank 段中相同的 chunkOffset；offset=用户rank*size+chunkOffset。
 * 先发最后一段，逐步接收归约并转发，最后把属于自己的完整结果写到 output[chunkOffset]。
 * 例：4 rank、recvcount=8 float，每人输入 32 float，rank2 最终得到全局求和后的 [16,24) 段，
 * 但放在自己的 8 元素输出缓冲 [0,8) 中。
 */
      rankDest = ringRanks[nranks-1];
      offset = chunkOffset + rankDest * size;
      prims.send(offset, nelem);

      // k-2 steps: reduce and copy to next GPU
      for (/* 中文逐项-变量 V0832：j
 * j（int；函数 runRing 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = 2；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：j<nranks；循环体与增量表达式决定每轮如何推进。
 */int j=2; j<nranks; ++j) {
        rankDest = ringRanks[nranks-j];
        offset = chunkOffset + rankDest * size;
        prims.recvReduceSend(offset, nelem);
      }

      // step k-1: reduce this buffer and data, which will produce the final result
      rankDest = ringRanks[0];
      offset = chunkOffset + rankDest * size;
      prims.recvReduceCopy(offset, chunkOffset, nelem, /*postOp=*/true);
    }
  }
}

/* 中文逐项-类型 T0128：RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ /* 中文内部逻辑 F0371：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：using Proto = ProtoSimple<REDUCESCATTER_CHUNKSTEPS/REDUCESCATTER_SLICESTEPS, REDUCESCATTER_SLICESTEPS>;；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 调用 / 计算 / 写入：runRing<T, RedOp, Proto>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_scatter_h.md#f0371
 */
/* 中文逐项-函数 F0371：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncReduceScatter, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, Proto>。
 */void run(ncclWorkElem *args) {
    using Proto = ProtoSimple<REDUCESCATTER_CHUNKSTEPS/REDUCESCATTER_SLICESTEPS, REDUCESCATTER_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
};

/* 中文逐项-类型 T0129：RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL> {
  __device__ __forceinline__ /* 中文内部逻辑 F0372：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：runRing<T, RedOp, ProtoLL>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_scatter_h.md#f0372
 */
/* 中文逐项-函数 F0372：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncReduceScatter, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, ProtoLL>。
 */void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL>(args);
  }
};

/* 中文逐项-类型 T0130：RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128> {
  __device__ __forceinline__ /* 中文内部逻辑 F0373：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：runRing<T, RedOp, ProtoLL128>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__reduce_scatter_h.md#f0373
 */
/* 中文逐项-函数 F0373：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncReduceScatter, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, ProtoLL128>。
 */void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL128>(args);
  }
};
