/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "primitives.h"

namespace {
/* 中文内部逻辑 F0068：runRing
 * 精读：按root、root前驱和其他rank分三种原语：send/copySend、recv、recvCopySend。
 * 精读：在root前驱切断回环边，避免重复传回root；不同slice仍能形成流水。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：const int nthreads = args->header.nWarps*WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const int bid = args->bid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：const int nChannels = args->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：ncclRing *ring = &ncclShmem.channel.ring;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? BROADCAST_CHUNKSTEPS : 1));；计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。
 * 7. 准备局部状态：const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T)));；给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 8. 准备局部状态：const ssize_t loopSize = nChannels*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 准备局部状态：const ssize_t size = args->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 准备局部状态：const int rank = ring->devUserRanks[0];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 准备局部状态：const int nextRank = ring->devUserRanks[1];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 准备局部状态：const int root = args->root;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：T *inputBuf = (T*)args->sendbuff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 准备局部状态：T *outputBuf = (T*)args->recvbuff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 准备局部状态：Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0> prims(tid, nthreads, &ring->prev, &ring->next, inputBuf, outputBuf, args->redOpArg);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 16. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   17. 准备局部状态：ssize_t realChunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   18. 条件判断：(Proto::Id == NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     19. 真分支：；仅条件为真执行以下子项。
 *       20. 调用 / 计算 / 写入：realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       21. 调用 / 计算 / 写入：realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));；把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。
 *     22. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       23. 条件判断：(Proto::Id == NCCL_PROTO_LL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         24. 真分支：；仅条件为真执行以下子项。
 *           25. 调用 / 计算 / 写入：realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         26. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           27. 条件判断：(Proto::Id == NCCL_PROTO_LL128)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             28. 真分支：；仅条件为真执行以下子项。
 *               29. 调用 / 计算 / 写入：realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   30. 调用 / 计算 / 写入：realChunkSize = int(realChunkSize);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   31. 准备局部状态：ssize_t offset = gridOffset + int(bid*realChunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   32. 准备局部状态：int nelem = min(realChunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   33. 条件判断：(rank == root)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     34. 真分支：；仅条件为真执行以下子项。
 *       35. 条件判断：(inputBuf == outputBuf)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         36. 真分支：；仅条件为真执行以下子项。
 *           37. 调用 / 计算 / 写入：prims.send(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         38. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           39. 调用 / 计算 / 写入：prims.copySend(offset, offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     40. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       41. 条件判断：(nextRank == root)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         42. 真分支：；仅条件为真执行以下子项。
 *           43. 调用 / 计算 / 写入：prims.recv(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         44. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           45. 调用 / 计算 / 写入：prims.recvCopySend(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 46. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__broadcast_h.md#f0068
 */
/* 中文逐项-函数 F0068：runRing
 * 用途与位置：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进；当前API文件为 broadcast。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::run；src/collectives/device/all_reduce.h::run；src/collectives/device/broadcast.h::run；src/collectives/device/reduce.h::run；src/collectives/device/reduce_scatter.h::run。
 * 本体直接出现的调用/包装：Proto::calcBytePerGrain；Proto::calcBytePerStep；divUp；int；min；prims.copySend；prims.recv；prims.recvCopySend；prims.send；roundUp。
 * 需要对照的边界/推进条件：gridOffset < size；Proto::Id == NCCL_PROTO_SIMPLE；Proto::Id == NCCL_PROTO_LL；Proto::Id ==
 * NCCL_PROTO_LL128。各条件所属分支见紧邻函数体。
 */
  template<typename T, typename RedOp, typename Proto>
  /* 中文精读V3.1：Broadcast：由root开始沿一个ring方向传递相同chunk。root分输入输出是否相同选择send/copySend；其他rank中nextRank==root的是最后接收者，只recv不再转发以免回绕到root；中间rank执行recvCopySend。rank/root是用户rank而不是channel中的ring->index。换root后的环方向例子见 docs/zh_CN/ring-primitives-deep-dive.md 第4节。 */
  __device__ __forceinline__ void runRing(ncclWorkElem *args) {
/* 中文逐项-变量 V0236：tid
 * tid（int；函数 runRing 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int tid = threadIdx.x;
/* 中文逐项-变量 V0237：nthreads
 * nthreads（int；函数 runRing 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nthreads = args->header.nWarps*WARP_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nthreads = args->header.nWarps*WARP_SIZE;
/* 中文逐项-变量 V0238：bid
 * bid（int；函数 runRing 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = args->bid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int bid = args->bid;
/* 中文逐项-变量 V0239：nChannels
 * nChannels（int；函数 runRing 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = args->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nChannels = args->nChannels;
/* 中文逐项-变量 V0240：ring
 * ring（ncclRing；函数 runRing 内）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 * 初值/来源 ring = &ncclShmem.channel.ring；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    ncclRing *ring = &ncclShmem.channel.ring;
/* 中文逐项-变量 V0241：chunkSize
 * chunkSize（ssize_t；函数 runRing 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? BROADCAST_CHUNKSTEPS :
 * 1))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? BROADCAST_CHUNKSTEPS : 1));
/* 中文逐项-变量 V0242：minChunkSizeLL128
 * minChunkSizeLL128（ssize_t；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T)))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T)));
/* 中文逐项-变量 V0243：loopSize
 * loopSize（ssize_t；函数 runRing 内）：一轮全部 channel（以及 Ring ranks/CollNet heads）覆盖的逻辑数据范围。
 * 初值/来源 loopSize = nChannels*chunkSize；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const ssize_t loopSize = nChannels*chunkSize;
/* 中文逐项-变量 V0244：size
 * size（ssize_t；函数 runRing 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t size = args->count;
/* 中文逐项-变量 V0245：rank
 * rank（int；函数 runRing 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = ring->devUserRanks[0]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int rank = ring->devUserRanks[0];
/* 中文逐项-变量 V0246：nextRank
 * nextRank（int；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nextRank = ring->devUserRanks[1]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nextRank = ring->devUserRanks[1];
/* 中文逐项-变量 V0247：root
 * root（int；函数 runRing 内）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 * 初值/来源 root = args->root；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int root = args->root;

/* 中文逐项-变量 V0248：inputBuf
 * inputBuf（T；函数 runRing 内）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 初值/来源 inputBuf = (T*)args->sendbuff；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    T *inputBuf = (T*)args->sendbuff;
/* 中文逐项-变量 V0249：outputBuf
 * outputBuf（T；函数 runRing 内）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 初值/来源 outputBuf = (T*)args->recvbuff；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    T *outputBuf = (T*)args->recvbuff;
/* 中文逐项-变量 V0250：prims
 * prims（Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>；函数 runRing 内）：当前 GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nthreads, &ring->prev, &ring->next, inputBuf, outputBuf,
 * args->redOpArg)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>
      prims(tid, nthreads, &ring->prev, &ring->next, inputBuf, outputBuf, args->redOpArg);

    for (/* 中文逐项-变量 V0251：gridOffset
 * gridOffset（ssize_t；函数 runRing 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0252：realChunkSize
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
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128);
      realChunkSize = int(realChunkSize);

/* 中文逐项-变量 V0253：offset
 * offset（ssize_t；函数 runRing 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + int(bid*realChunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      ssize_t offset = gridOffset + int(bid*realChunkSize);
/* 中文逐项-变量 V0254：nelem
 * nelem（int；函数 runRing 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(realChunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
      int nelem = min(realChunkSize, size-offset);

/* 中文精读：Broadcast 如何把环变成从 root 开始的流水线
 * 每个 channel 对相同张量不同区间并行；本 API 已归一化为 Int8，offset/nelem 用字节。
 * root 发送，若输入输出不同则顺便本地复制；中间 rank recvCopySend；root 的前驱仅 recv。
 * 这切断了最后一条回到 root 的边，防止重复发送和 root 等待并不需要的输入。
 * 它不是先全量发完再由下一个 rank 发：slice 级别的原语使各 rank 能流水转发。
 * 本文件提供的是 Ring 特化；看到调度器 pattern 有 TreeDown 不等于本文件实现了 Tree Broadcast。
 */
      if (rank == root) {
        if (inputBuf == outputBuf) {
          prims.send(offset, nelem);
        } else {
          prims.copySend(offset, offset, nelem);
        }
      } else if (nextRank == root) {
        prims.recv(offset, nelem);
      } else {
        prims.recvCopySend(offset, nelem);
      }
    }
  }
}

/* 中文逐项-类型 T0015：RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ /* 中文内部逻辑 F0069：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：using Proto = ProtoSimple<BROADCAST_CHUNKSTEPS/BROADCAST_SLICESTEPS, BROADCAST_SLICESTEPS>;；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 调用 / 计算 / 写入：runRing<T, RedOp, Proto>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__broadcast_h.md#f0069
 */
/* 中文逐项-函数 F0069：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncBroadcast, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, Proto>。
 */void run(ncclWorkElem *args) {
    using Proto = ProtoSimple<BROADCAST_CHUNKSTEPS/BROADCAST_SLICESTEPS, BROADCAST_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
};

/* 中文逐项-类型 T0016：RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL> {
  __device__ __forceinline__ /* 中文内部逻辑 F0070：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：runRing<T, RedOp, ProtoLL>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__broadcast_h.md#f0070
 */
/* 中文逐项-函数 F0070：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncBroadcast, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, ProtoLL>。
 */void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL>(args);
  }
};

/* 中文逐项-类型 T0017：RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128> {
  __device__ __forceinline__ /* 中文内部逻辑 F0071：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：runRing<T, RedOp, ProtoLL128>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__broadcast_h.md#f0071
 */
/* 中文逐项-函数 F0071：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncBroadcast, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, ProtoLL128>。
 */void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL128>(args);
  }
};
