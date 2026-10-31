/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "primitives.h"

namespace {
/* 中文内部逻辑 F0058：runRing
 * 精读：固定一个channel后，外层gridOffset覆盖全输入；本轮真实块长按剩余量和协议grain调整。
 * 精读：calcOffset对Simple使用channel连续布局，对LL/LL128使用交错布局；nelem截断尾部。
 * 精读：先发块(ringIx-1)，逐块接收归约，块ringIx完成时保存并直接开始结果传播。
 * 精读：后续仅复制完整结果；最后只recv收齐缺块。不能在AllGather阶段重复加本地输入。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：const int nthreads = args->header.nWarps*WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const int bid = args->bid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：const int nChannels = args->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：ncclRing *ring = &ncclShmem.channel.ring;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：int ringIx = ring->index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 准备局部状态：const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLREDUCE_CHUNKSTEPS : 1));；计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。
 * 8. 准备局部状态：const int nranks = ncclShmem.comm.nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 准备局部状态：const ssize_t loopSize = nChannels*nranks*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 准备局部状态：const ssize_t size = args->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 准备局部状态：int minChunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 条件判断：(Proto::Id == NCCL_PROTO_LL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 调用 / 计算 / 写入：minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T));；给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 15. 条件判断：(Proto::Id == NCCL_PROTO_LL128)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   16. 真分支：；仅条件为真执行以下子项。
 *     17. 调用 / 计算 / 写入：minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2;；给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 18. 准备局部状态：Primitives<T, RedOp, FanSymmetric<1>, 1, Proto, 0> prims (tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 19. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   20. 准备局部状态：ssize_t realChunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   21. 条件判断：(Proto::Id == NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     22. 真分支：；仅条件为真执行以下子项。
 *       23. 调用 / 计算 / 写入：realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks));；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       24. 调用 / 计算 / 写入：realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));；把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。
 *     25. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       26. 调用 / 计算 / 写入：realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks*minChunkSize)*minChunkSize);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   27. 调用 / 计算 / 写入：realChunkSize = int(realChunkSize);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   28. 准备局部状态：auto calcOffset = [&] (int chunk)->ssize_t { if (Proto::Id == NCCL_PROTO_SIMPLE) return gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize; else return gridOffset + (chunk*nChannels + bid)*re …（完整代码见函数体）；定义并保存局部lambda对象；其body仅在后续调用时执行，不在声明处立即运行。
 *     29. 定义局部lambda：；此处建立可调用对象；以下body在以后调用该lambda时才执行，不是声明时立即执行。
 *       30. 条件判断：(Proto::Id == NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         31. 真分支：；仅条件为真执行以下子项。
 *           32. 函数返回：return gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *         33. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           34. 函数返回：return gridOffset + (chunk*nChannels + bid)*realChunkSize;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   35. 准备局部状态：auto modRanks = [&] (int r)->int { return r - (r >= nranks ? nranks : 0); };；定义并保存局部lambda对象；其body仅在后续调用时执行，不在声明处立即运行。
 *     36. 定义局部lambda：；此处建立可调用对象；以下body在以后调用该lambda时才执行，不是声明时立即执行。
 *       37. 函数返回：return r - (r >= nranks ? nranks : 0);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   38. 准备局部状态：ssize_t offset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   39. 准备局部状态：int nelem;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   40. 准备局部状态：int chunk;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   41. 调用 / 计算 / 写入：chunk = modRanks(ringIx + nranks-1);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   42. 调用 / 计算 / 写入：offset = calcOffset(chunk);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   43. 调用 / 计算 / 写入：nelem = min(realChunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   44. 调用 / 计算 / 写入：prims.send(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   45. 循环：初始 int j=2;；继续条件 j<nranks；每轮更新 ++j；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     46. 调用 / 计算 / 写入：chunk = modRanks(ringIx + nranks-j);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     47. 调用 / 计算 / 写入：offset = calcOffset(chunk);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     48. 调用 / 计算 / 写入：nelem = min(realChunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     49. 调用 / 计算 / 写入：prims.recvReduceSend(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   50. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   51. 调用 / 计算 / 写入：chunk = ringIx + 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   52. 调用 / 计算 / 写入：offset = calcOffset(chunk);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   53. 调用 / 计算 / 写入：nelem = min(realChunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   54. 调用 / 计算 / 写入：prims.directRecvReduceCopySend(offset, offset, offset, nelem, true);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   55. 循环：初始 int j=1;；继续条件 j<nranks-1；每轮更新 ++j；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     56. 调用 / 计算 / 写入：chunk = modRanks(ringIx + nranks-j);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     57. 调用 / 计算 / 写入：offset = calcOffset(chunk);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     58. 调用 / 计算 / 写入：nelem = min(realChunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     59. 调用 / 计算 / 写入：prims.directRecvCopySend(offset, offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   60. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   61. 调用 / 计算 / 写入：chunk = modRanks(ringIx + 1);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   62. 调用 / 计算 / 写入：offset = calcOffset(chunk);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   63. 调用 / 计算 / 写入：nelem = min(realChunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   64. 调用 / 计算 / 写入：prims.directRecv(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 65. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0058
 */
/* 中文逐项-函数 F0058：runRing
 * 用途与位置：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进；当前API文件为 all_reduce。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::run；src/collectives/device/all_reduce.h::run；src/collectives/device/broadcast.h::run；src/collectives/device/reduce.h::run；src/collectives/device/reduce_scatter.h::run。
 * 本体直接出现的调用/包装：Proto::calcBytePerGrain；Proto::calcBytePerStep；calcOffset；divUp；int；min；modRanks；prims.directRecv；prims.directRecvCopySend；prims.directRecvReduceCopySend；prims.recvReduceSend；prims.send；roundUp。
 * 需要对照的边界/推进条件：Proto::Id == NCCL_PROTO_LL；Proto::Id == NCCL_PROTO_LL128；gridOffset < size；Proto::Id ==
 * NCCL_PROTO_SIMPLE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize;；return gridOffset + (chunk*nChannels +
 * bid)*realChunkSize;；return r - (r >= nranks ? nranks : 0);。CHECK类宏还可能提前返回/跳转。
 */
  template<typename T, typename RedOp, typename Proto>
/* 中文导读：
 * Ring AllReduce 的设备端主循环。每个 channel/block 负责输入的一部分，沿 prev/next 传递分块。
 * 先 send 和 recvReduceSend 形成 ReduceScatter；最终归约块经 directRecvReduceCopySend 衔接到 AllGather。
 * 这里的 direct 仍受连接能力约束，不能据函数名判断为 GPU 直接提交 RDMA WQE。
 */
  __device__ __forceinline__ void runRing(ncclWorkElem *args) {
/* 中文逐项-变量 V0113：tid
 * tid（int；函数 runRing 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int tid = threadIdx.x;
/* 中文逐项-变量 V0114：nthreads
 * nthreads（int；函数 runRing 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nthreads = args->header.nWarps*WARP_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nthreads = args->header.nWarps*WARP_SIZE;
/* 中文逐项-变量 V0115：bid
 * bid（int；函数 runRing 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = args->bid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int bid = args->bid;
/* 中文逐项-变量 V0116：nChannels
 * nChannels（int；函数 runRing 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = args->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nChannels = args->nChannels;
/* 中文逐项-变量 V0117：ring
 * ring（ncclRing；函数 runRing 内）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 * 初值/来源 ring = &ncclShmem.channel.ring；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    ncclRing *ring = &ncclShmem.channel.ring;
/* 中文逐项-变量 V0118：ringIx
 * ringIx（int；函数 runRing 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 ringIx = ring->index；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int ringIx = ring->index;
/* 中文逐项-变量 V0119：chunkSize
 * chunkSize（ssize_t；函数 runRing 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLREDUCE_CHUNKSTEPS :
 * 1))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLREDUCE_CHUNKSTEPS : 1));
/* 中文逐项-变量 V0120：nranks
 * nranks（int；函数 runRing 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = ncclShmem.comm.nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nranks = ncclShmem.comm.nRanks;
/* 中文逐项-变量 V0121：loopSize
 * loopSize（ssize_t；函数 runRing 内）：一轮全部 channel（以及 Ring ranks/CollNet heads）覆盖的逻辑数据范围。
 * 初值/来源 loopSize = nChannels*nranks*chunkSize；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const ssize_t loopSize = nChannels*nranks*chunkSize;
/* 中文逐项-变量 V0122：size
 * size（ssize_t；函数 runRing 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t size = args->count;

/* 中文逐项-变量 V0123：minChunkSize
 * minChunkSize（int；函数 runRing 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 此处未显式给初值；后续首个文本使用点：minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int minChunkSize;
    if (Proto::Id == NCCL_PROTO_LL)
      minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T));
    if (Proto::Id == NCCL_PROTO_LL128) {
      // We should not need the final /2 but it makes performance much, much smoother. Might be a bug somewhere.
      minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2;
    }

/* 中文逐项-变量 V0124：prims
 * prims（Primitives<T, RedOp, FanSymmetric<1>, 1, Proto, 0>；函数 runRing 内）：当前 GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff,
 * args->redOpArg)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    Primitives<T, RedOp, FanSymmetric<1>, 1, Proto, 0> prims
      (tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);

    for (/* 中文逐项-变量 V0125：gridOffset
 * gridOffset（ssize_t；函数 runRing 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0126：realChunkSize
 * realChunkSize（ssize_t；函数 runRing 内）：结合剩余数据、并行通道和对齐粒度计算的本轮块容量，不保证每块都填满。
 * 此处未显式给初值；后续首个文本使用点：realChunkSize = min(chunkSize, divUp(size-gridOffset,
 * nChannels*nranks));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks*minChunkSize)*minChunkSize);
      realChunkSize = int(realChunkSize);

/* 中文精读：AllReduce 的地址公式与数值示例
 * 所有偏移以 T 元素计。Simple 使用 channel 连续布局：
 * offset=gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize。
 * 假设 4 rank、2 channel、nthreads=96、共 1024 个 float，且 chunkSize 至少为 128 元素。
 * 对齐粒度=(96-32)*8/4=128 元素，realChunkSize=ceil(1024/8)=128。
 * channel0 的 4 块覆盖 [0,512)，channel1 的 4 块覆盖 [512,1024)，块长都是 128 元素。
 * LL/LL128 用 (chunk*nChannels+bid)*realChunkSize 交错布局；不能把 Simple 公式套到所有协议。
 * 注意 realChunkSize 是对齐后的容量，不保证每个块都有这么多有效元素。
 */
/* 中文逐项-变量 V0127：calcOffset
 * calcOffset（auto；函数 runRing 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 calcOffset = [&]__device__(int chunk)->ssize_t { if (Proto::Id == NCCL_PROTO_SIMPLE) return gridOffset +
 * bid*nranks*realChunkSize + chunk*realChunkSize; else return gridOffset + (chunk*nChannels + bid)*realChunkSize;
 * }；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      auto calcOffset = [&]__device__(int chunk)->ssize_t {
        if (Proto::Id == NCCL_PROTO_SIMPLE)
          return gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize;
        else
          return gridOffset + (chunk*nChannels + bid)*realChunkSize;
      };
/* 中文逐项-变量 V0128：modRanks
 * modRanks（auto；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 modRanks = [&]__device__(int r)->int { return r - (r >= nranks ? nranks : 0);
 * }；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      auto modRanks = [&]__device__(int r)->int {
        return r - (r >= nranks ? nranks : 0);
      };

/* 中文逐项-变量 V0129：offset
 * offset（ssize_t；函数 runRing 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：offset = calcOffset(chunk);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ssize_t offset;
/* 中文逐项-变量 V0130：nelem
 * nelem（int；函数 runRing 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 此处未显式给初值；后续首个文本使用点：nelem = min(realChunkSize, size-offset);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int nelem;
/* 中文逐项-变量 V0131：chunk
 * chunk（int；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：chunk = modRanks(ringIx + nranks-1);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int chunk;

/* 中文导读：
 * 本轮先发送第一个块，再逐块把上游数据与本地输入归约后继续传递。
 * 各 rank 处理不同块，形成流水线；不是每一轮都发送整个输入数组。
 */
      // step 0: push data to next GPU
      chunk = modRanks(ringIx + nranks-1);
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.send(offset, nelem);

      // k-2 steps: reduce and copy to next GPU
      for (/* 中文逐项-变量 V0132：j
 * j（int；函数 runRing 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = 2；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：j<nranks；循环体与增量表达式决定每轮如何推进。
 */int j=2; j<nranks; ++j) {
        chunk = modRanks(ringIx + nranks-j);
        offset = calcOffset(chunk);
        nelem = min(realChunkSize, size-offset);
        prims.recvReduceSend(offset, nelem);
      }

      // step k-1: reduce this buffer and data, which will produce the final
      // result that we store in this data and push to the next GPU
/* 中文精读：归约分散与结果收集在这一调用衔接
 * 最初发送 (ringIx-1) 块，再依次处理前面的块；收到本 rank 的 ringIx 块时，
 * 它已经包含其余 rank 的部分结果，加上本地输入后才成为完整归约结果。
 * directRecvReduceCopySend 同时把最终值写本地 output 并发下游，启动结果收集阶段。
 * 对于 4 rank/单 channel，rank0 依次 send 块3，reduceSend 块2、块1，最终归约块0；
 * 再 copySend 块3、块2，最后 recv 块1。实际环顺序改变时，ringIx 的对应用户 rank 也改变。
 */
      chunk = ringIx + 0;
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.directRecvReduceCopySend(offset, offset, offset, nelem, /*postOp=*/true);

/* 中文导读：
 * 从这里进入结果传播阶段：继续转发已经完成归约的块，不再对这些块重复做全局求和。
 * 最后 directRecv 收齐本 rank 缺少的最后一个块，整段完成 ReduceScatter+AllGather 语义。
 */
      // k-2 steps: copy to next GPU
      for (/* 中文逐项-变量 V0133：j
 * j（int；函数 runRing 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：j<nranks-1；循环体与增量表达式决定每轮如何推进。
 */int j=1; j<nranks-1; ++j) {
        chunk = modRanks(ringIx + nranks-j);
        offset = calcOffset(chunk);
        nelem = min(realChunkSize, size-offset);
        prims.directRecvCopySend(offset, offset, nelem);
      }

      // Make final copy from buffer to dest.
      chunk = modRanks(ringIx + 1);
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.directRecv(offset, nelem);
    }
  }

/* 中文内部逻辑 F0059：runTreeUpDown
 * 精读：上行把叶的send、中间节点的多收归约转发、根的最终recvReduceCopy分开。
 * 精读：同一线程组上行阶段结束后，换向构造广播原语；根从output发送，叶仅接收，中间节点复制转发。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：const int nthreads = args->header.nWarps*WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const int bid = args->bid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：const int nChannels = args->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：ncclTree *tree = &ncclShmem.channel.tree;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：ssize_t chunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? args->lastChunkSize : Proto::calcBytePerStep()/sizeof(T));；计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。
 * 7. 准备局部状态：const ssize_t minChunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads-2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T)) : nthreads*(Proto::calcBytePerGrain()/sizeof(T)));；给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 8. 准备局部状态：const ssize_t loopSize = int(nChannels*chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 准备局部状态：const ssize_t size = args->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 条件判断：(loopSize > size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 调用 / 计算 / 写入：chunkSize = divUp((int)size, int(nChannels*minChunkSize))*int(minChunkSize);；计算整数向上取整的商，避免尾数不整除时少覆盖最后一块。
 * 13. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, 0, Proto, 0> prims (tid, nthreads, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 条件判断：(tree->up == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       17. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       18. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       19. 调用 / 计算 / 写入：prims.recvReduceCopy(offset, offset, nelem, true);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   21. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     22. 条件判断：(tree->down[0] == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       23. 真分支：；仅条件为真执行以下子项。
 *         24. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           25. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           26. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *           27. 调用 / 计算 / 写入：prims.send(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         28. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       29. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         30. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           31. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           32. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *           33. 调用 / 计算 / 写入：prims.recvReduceSend(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         34. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 35. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, 1, Proto, 0> prims (tid, nthreads, &tree->up, tree->down, args->sendbuff, args->recvbuff, args->redOpArg);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 36. 条件判断：(tree->up == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   37. 真分支：；仅条件为真执行以下子项。
 *     38. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       39. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       40. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       41. 调用 / 计算 / 写入：prims.directSendFromOutput(offset, offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     42. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   43. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     44. 条件判断：(tree->down[0] == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       45. 真分支：；仅条件为真执行以下子项。
 *         46. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           47. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           48. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *           49. 调用 / 计算 / 写入：prims.directRecv(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         50. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       51. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         52. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           53. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           54. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *           55. 调用 / 计算 / 写入：prims.directRecvCopySend(offset, offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         56. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0059
 */
/* 中文逐项-函数 F0059：runTreeUpDown
 * 用途与位置：先向树根归约再向下广播结果，同一个线程组先后执行两个阶段。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_reduce.h::run。
 * 本体直接出现的调用/包装：Proto::calcBytePerGrain；Proto::calcBytePerStep；divUp；int；min；prims.directRecv；prims.directRecvCopySend；prims.directSendFromOutput；prims.recvReduceCopy；prims.recvReduceSend；prims.send。
 * 需要对照的边界/推进条件：loopSize > size；tree->up == -1；gridOffset < size；tree->down[0] == -1。各条件所属分支见紧邻函数体。
 */
  template<typename T, typename RedOp, typename Proto>
/* 中文导读：
 * Tree 的先向上归约、再向下广播实现；根/叶/中间节点使用不同原语。
 * root 把孩子与自身输入归约成结果，之后沿反向边传播。所需邻居由初始化时的 tree.up/down 给出。
 */
  __device__ __forceinline__ void runTreeUpDown(ncclWorkElem *args) {
/* 中文逐项-变量 V0134：tid
 * tid（int；函数 runTreeUpDown 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int tid = threadIdx.x;
/* 中文逐项-变量 V0135：nthreads
 * nthreads（int；函数 runTreeUpDown 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nthreads = args->header.nWarps*WARP_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nthreads = args->header.nWarps*WARP_SIZE;
/* 中文逐项-变量 V0136：bid
 * bid（int；函数 runTreeUpDown 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = args->bid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int bid = args->bid;
/* 中文逐项-变量 V0137：nChannels
 * nChannels（int；函数 runTreeUpDown 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = args->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nChannels = args->nChannels;
/* 中文逐项-变量 V0138：tree
 * tree（ncclTree；函数 runTreeUpDown 内）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 * 初值/来源 tree = &ncclShmem.channel.tree；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    ncclTree *tree = &ncclShmem.channel.tree;
/* 中文逐项-变量 V0139：chunkSize
 * chunkSize（ssize_t；函数 runTreeUpDown 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? args->lastChunkSize / * LL & LL128 * / :
 * Proto::calcBytePerStep()/sizeof(T))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    ssize_t chunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? args->lastChunkSize
                   /* LL & LL128 */  : Proto::calcBytePerStep()/sizeof(T));
/* 中文逐项-变量 V0140：minChunkSize
 * minChunkSize（ssize_t；函数 runTreeUpDown 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 minChunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads-2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T)) /
 * * LL & LL128 * / : nthreads*(Proto::calcBytePerGrain()/sizeof(T)))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t minChunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads-2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T))
                   /* LL & LL128 */  : nthreads*(Proto::calcBytePerGrain()/sizeof(T)));
/* 中文逐项-变量 V0141：loopSize
 * loopSize（ssize_t；函数 runTreeUpDown 内）：一轮全部 channel（以及 Ring ranks/CollNet heads）覆盖的逻辑数据范围。
 * 初值/来源 loopSize = int(nChannels*chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const ssize_t loopSize = int(nChannels*chunkSize);
/* 中文逐项-变量 V0142：size
 * size（ssize_t；函数 runTreeUpDown 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t size = args->count;

    if (loopSize > size)
      chunkSize = divUp((int)size, int(nChannels*minChunkSize))*int(minChunkSize);

    { // Reduce : max number of recv is 3, max number of send is 1 (binary tree + local)
/* 中文逐项-变量 V0143：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, / *Direct=* /0, Proto, 0>；函数 runTreeUpDown 内）：当前
 * GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nthreads, tree->down, &tree->up, args->sendbuff, args->recvbuff,
 * args->redOpArg)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, /*Direct=*/0, Proto, 0> prims
        (tid, nthreads, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg);
      if (tree->up == -1) {
        for (/* 中文逐项-变量 V0144：gridOffset
 * gridOffset（ssize_t；函数 runTreeUpDown 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0145：offset
 * offset（ssize_t；函数 runTreeUpDown 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0146：nelem
 * nelem（int；函数 runTreeUpDown 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceCopy(offset, offset, nelem, /*postOp=*/true);
        }
      }
      else if (tree->down[0] == -1) {
        for (/* 中文逐项-变量 V0147：gridOffset
 * gridOffset（ssize_t；函数 runTreeUpDown 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0148：offset
 * offset（ssize_t；函数 runTreeUpDown 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0149：nelem
 * nelem（int；函数 runTreeUpDown 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
      else {
        for (/* 中文逐项-变量 V0150：gridOffset
 * gridOffset（ssize_t；函数 runTreeUpDown 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0151：offset
 * offset（ssize_t；函数 runTreeUpDown 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0152：nelem
 * nelem（int；函数 runTreeUpDown 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceSend(offset, nelem);
        }
      }
    }

    { // Broadcast : max number of recv is 1, max number of send is 3 (binary tree + local)
/* 中文逐项-变量 V0153：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, / *Direct=* /1, Proto, 0>；函数 runTreeUpDown 内）：当前
 * GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nthreads, &tree->up, tree->down, args->sendbuff, args->recvbuff,
 * args->redOpArg)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0> prims
        (tid, nthreads, &tree->up, tree->down, args->sendbuff, args->recvbuff, args->redOpArg);
      if (tree->up == -1) {
        for (/* 中文逐项-变量 V0154：gridOffset
 * gridOffset（ssize_t；函数 runTreeUpDown 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0155：offset
 * offset（ssize_t；函数 runTreeUpDown 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0156：nelem
 * nelem（int；函数 runTreeUpDown 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.directSendFromOutput(offset, offset, nelem);
        }
      }
      else if (tree->down[0] == -1) {
        for (/* 中文逐项-变量 V0157：gridOffset
 * gridOffset（ssize_t；函数 runTreeUpDown 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0158：offset
 * offset（ssize_t；函数 runTreeUpDown 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0159：nelem
 * nelem（int；函数 runTreeUpDown 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.directRecv(offset, nelem);
        }
      }
      else {
        for (/* 中文逐项-变量 V0160：gridOffset
 * gridOffset（ssize_t；函数 runTreeUpDown 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0161：offset
 * offset（ssize_t；函数 runTreeUpDown 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0162：nelem
 * nelem（int；函数 runTreeUpDown 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.directRecvCopySend(offset, offset, nelem);
        }
      }
    }
  }

/* 中文内部逻辑 F0060：runTreeSplit
 * 精读：先按协议确定上下行线程拆分；非根上行组与下行组使用不同group/barrier。
 * 精读：根用融合原语完成多收归约、保留和多发，非根可让前一块下行与后一块上行重叠。
 * 精读：构造器Direct能力还要与对端地址交换契约兼容，即使某组不实际direct搬数据也不能任意改模板位。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：const int nthreads = args->header.nWarps*WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const int bid = args->bid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：const int nChannels = args->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：ncclTree *tree = &ncclShmem.channel.tree;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：ssize_t chunkSize = int( Proto::Id != NCCL_PROTO_LL ? args->lastChunkSize : Proto::calcBytePerStep()/sizeof(T));；计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。
 * 7. 准备局部状态：const ssize_t minChunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads - 2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T)) : Proto::Id == NCCL_PROTO_LL ? nthreads*(Proto::calcBytePerGrain()/sizeof(T)) …（完整代码见函数体）；给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。
 * 8. 准备局部状态：const ssize_t loopSize = int(nChannels*chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 准备局部状态：const ssize_t size = args->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 准备局部状态：int nthreadsSplit;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 条件判断：(Proto::Id == NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 调用 / 计算 / 写入：nthreadsSplit = nthreads/2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     14. 条件判断：(nthreadsSplit >= 256)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       15. 真分支：；仅条件为真执行以下子项。
 *         16. 调用 / 计算 / 写入：nthreadsSplit += 64;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   17. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     18. 调用 / 计算 / 写入：nthreadsSplit = (nthreads*7/(10*WARP_SIZE))*WARP_SIZE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 条件判断：(loopSize > size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 调用 / 计算 / 写入：chunkSize = divUp((int)size, nChannels*int(minChunkSize))*int(minChunkSize);；计算整数向上取整的商，避免尾数不整除时少覆盖最后一块。
 * 22. 条件判断：(tree->up == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   23. 真分支：；仅条件为真执行以下子项。
 *     24. 准备局部状态：Primitives<T, RedOp, FanSymmetric<NCCL_MAX_DEV_ARITY>, 1, Proto, 0> prims(tid, nthreads, tree->down, tree->down, args->sendbuff, args->recvbuff, args->redOpArg);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     25. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       26. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       27. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       28. 调用 / 计算 / 写入：prims.directRecvReduceCopySend(offset, offset, offset, nelem, true);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     29. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   30. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     31. 条件判断：(tid < nthreadsSplit)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       32. 真分支：；仅条件为真执行以下子项。
 *         33. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, 1, Proto, 0> prims(tid, nthreadsSplit, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg, 0*Proto::MaxGroupWidth);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         34. 条件判断：(tree->down[0] == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           35. 真分支：；仅条件为真执行以下子项。
 *             36. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               37. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               38. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *               39. 调用 / 计算 / 写入：prims.send(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             40. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           41. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             42. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               43. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               44. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *               45. 调用 / 计算 / 写入：prims.recvReduceSend(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             46. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       47. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         48. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, 1, Proto, 0> prims(tid-nthreadsSplit, nthreads-nthreadsSplit, &tree->up, tree->down, args->sendbuff, args->recvbuff, args->redOpArg, 1*Proto: …（完整代码见函数体）；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         49. 条件判断：(tree->down[0] == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           50. 真分支：；仅条件为真执行以下子项。
 *             51. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               52. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               53. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *               54. 调用 / 计算 / 写入：prims.directRecv(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             55. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           56. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             57. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               58. 准备局部状态：ssize_t offset = gridOffset + bid*int(chunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               59. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *               60. 调用 / 计算 / 写入：prims.directRecvCopySend(offset, offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             61. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0060
 */
/* 中文逐项-函数 F0060：runTreeSplit
 * 用途与位置：把非根 block 线程分成上行归约与下行广播两组，用不同 barrier/group 形成重叠流水。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_reduce.h::run。
 * 本体直接出现的调用/包装：Proto::calcBytePerGrain；Proto::calcBytePerStep；divUp；int；min；prims.directRecv；prims.directRecvCopySend；prims.directRecvReduceCopySend；prims.recvReduceSend；prims.send。
 * 需要对照的边界/推进条件：Proto::Id == NCCL_PROTO_SIMPLE；nthreadsSplit >= 256；loopSize > size；tree->up == -1。各条件所属分支见紧邻函数体。
 */
  template<typename T, typename RedOp, typename Proto>
/* 中文导读：
 * 在同一 block 中分配不同线程组推进向上归约与向下广播，以改善流水重叠。
 * 这是设备端线程执行拆分；不要与 graph.h 中决定跨节点流量端点的 SPLIT_TREE 搜索模式混为一谈。
 */
  __device__ __forceinline__ void runTreeSplit(ncclWorkElem *args) {
/* 中文逐项-变量 V0163：tid
 * tid（int；函数 runTreeSplit 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int tid = threadIdx.x;
/* 中文逐项-变量 V0164：nthreads
 * nthreads（int；函数 runTreeSplit 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nthreads = args->header.nWarps*WARP_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nthreads = args->header.nWarps*WARP_SIZE;
/* 中文逐项-变量 V0165：bid
 * bid（int；函数 runTreeSplit 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = args->bid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int bid = args->bid;
/* 中文逐项-变量 V0166：nChannels
 * nChannels（int；函数 runTreeSplit 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = args->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nChannels = args->nChannels;
/* 中文逐项-变量 V0167：tree
 * tree（ncclTree；函数 runTreeSplit 内）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 * 初值/来源 tree = &ncclShmem.channel.tree；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    ncclTree *tree = &ncclShmem.channel.tree;
/* 中文逐项-变量 V0168：chunkSize
 * chunkSize（ssize_t；函数 runTreeSplit 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = int( Proto::Id != NCCL_PROTO_LL ? args->lastChunkSize :
 * Proto::calcBytePerStep()/sizeof(T))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    ssize_t chunkSize = int(
      Proto::Id != NCCL_PROTO_LL ? args->lastChunkSize
                                 : Proto::calcBytePerStep()/sizeof(T));
/* 中文逐项-变量 V0169：minChunkSize
 * minChunkSize（ssize_t；函数 runTreeSplit 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 minChunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads - 2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T))
 * : Proto::Id == NCCL_PROTO_LL ? nthreads*(Proto::calcBytePerGrain()/sizeof(T)) / * LL128 * / :
 * nthreads*(Proto::calcBytePerGrain()/sizeof …（完整表达式见紧邻源码）；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t minChunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads - 2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T)) :
      Proto::Id == NCCL_PROTO_LL     ? nthreads*(Proto::calcBytePerGrain()/sizeof(T))
                   /* LL128 */       : nthreads*(Proto::calcBytePerGrain()/sizeof(T))/8);
/* 中文逐项-变量 V0170：loopSize
 * loopSize（ssize_t；函数 runTreeSplit 内）：一轮全部 channel（以及 Ring ranks/CollNet heads）覆盖的逻辑数据范围。
 * 初值/来源 loopSize = int(nChannels*chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const ssize_t loopSize = int(nChannels*chunkSize);
/* 中文逐项-变量 V0171：size
 * size（ssize_t；函数 runTreeSplit 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t size = args->count;

/* 中文精读：Tree 分线程组的流水线
 * 非根节点把一组线程用于向上归约，另一组线程用于向下广播；二者使用不同 group/barrier。
 * Simple 大致均分后对归约组作调整，LL/LL128 约 70% 线程给更重的多输入归约。
 * 根用融合原语同时完成收集、归约、保存、向下分发；叶子没有有效 child，原语自动简化。
 * 线程组分割是设备执行优化，拓扑 SPLIT_TREE 则是选择跨机端点，两者不能混作一个开关。
 */
/* 中文逐项-变量 V0172：nthreadsSplit
 * nthreadsSplit（int；函数 runTreeSplit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：nthreadsSplit = nthreads/2;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int nthreadsSplit;
    if (Proto::Id == NCCL_PROTO_SIMPLE) {
      nthreadsSplit = nthreads/2;
      if (nthreadsSplit >= 256) nthreadsSplit += 64;
    } else { // LL & LL128
      // Receiving from up to 3 sources is more compute intensive than sending
      // to 3 dests. Use 70% for reduce and 30% for bcast.
      nthreadsSplit = (nthreads*7/(10*WARP_SIZE))*WARP_SIZE;
    }

    if (loopSize > size)
      chunkSize = divUp((int)size, nChannels*int(minChunkSize))*int(minChunkSize);

    if (tree->up == -1) {
      // Reduce and broadcast. Max number of recv is 3, max number of send is 3
/* 中文逐项-变量 V0173：prims
 * prims（Primitives<T, RedOp, FanSymmetric<NCCL_MAX_DEV_ARITY>, / *Direct=* /1, Proto, 0>；函数 runTreeSplit 内）：当前 GPU
 * 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nthreads, tree->down, tree->down, args->sendbuff, args->recvbuff,
 * args->redOpArg)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanSymmetric<NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid, nthreads, tree->down, tree->down, args->sendbuff, args->recvbuff, args->redOpArg);
      for (/* 中文逐项-变量 V0174：gridOffset
 * gridOffset（ssize_t；函数 runTreeSplit 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0175：offset
 * offset（ssize_t；函数 runTreeSplit 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0176：nelem
 * nelem（int；函数 runTreeSplit 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
        int nelem = min(chunkSize, size-offset);
        prims.directRecvReduceCopySend(offset, offset, offset, nelem, /*doPost=*/true);
      }
    }
    else if (tid < nthreadsSplit) {
      /* Reduce up. Max number of recv is 3, max number of send is 1 (binary tree + local).
       * Why Direct=1????
       * Answer: Because despite not performing any direct operations, the ctor
       * must assume Direct so that it can exchange direct pointers with remote ctors
       * that are Direct, otherwise it hangs. A cleaner solution would be to seperate
       * into DirectRecv and DirectSend capabilities, this ctor would have both=0,
       * but the ctor above for tree roots would be DirectRecv=0 DirectSend=1.
       */
/* 中文逐项-变量 V0177：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, / *Direct=* /1, Proto, 0>；函数 runTreeSplit 内）：当前
 * GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nthreadsSplit, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg,
 * 0*Proto::MaxGroupWidth)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, /*Direct=*/1, Proto, 0>
        prims(tid, nthreadsSplit, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg, 0*Proto::MaxGroupWidth);
      if (tree->down[0] == -1) {
        for (/* 中文逐项-变量 V0178：gridOffset
 * gridOffset（ssize_t；函数 runTreeSplit 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0179：offset
 * offset（ssize_t；函数 runTreeSplit 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0180：nelem
 * nelem（int；函数 runTreeSplit 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
      else {
        for (/* 中文逐项-变量 V0181：gridOffset
 * gridOffset（ssize_t；函数 runTreeSplit 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0182：offset
 * offset（ssize_t；函数 runTreeSplit 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0183：nelem
 * nelem（int；函数 runTreeSplit 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceSend(offset, nelem);
        }
      }
    }
    else {
      // Broadcast down. Max number of recv is 1, max number of send is 3 (binary tree + local)
/* 中文逐项-变量 V0184：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, / *Direct=* /1, Proto, 0>；函数 runTreeSplit 内）：当前
 * GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid-nthreadsSplit, nthreads-nthreadsSplit, &tree->up, tree->down, args->sendbuff, args->recvbuff,
 * args->redOpArg, 1*Proto::MaxGroupWidth)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid-nthreadsSplit, nthreads-nthreadsSplit, &tree->up, tree->down, args->sendbuff, args->recvbuff, args->redOpArg, 1*Proto::MaxGroupWidth);
      if (tree->down[0] == -1) {
        for (/* 中文逐项-变量 V0185：gridOffset
 * gridOffset（ssize_t；函数 runTreeSplit 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0186：offset
 * offset（ssize_t；函数 runTreeSplit 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0187：nelem
 * nelem（int；函数 runTreeSplit 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.directRecv(offset, nelem);
        }
      }
      else {
        for (/* 中文逐项-变量 V0188：gridOffset
 * gridOffset（ssize_t；函数 runTreeSplit 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0189：offset
 * offset（ssize_t；函数 runTreeSplit 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          ssize_t offset = gridOffset + bid*int(chunkSize);
/* 中文逐项-变量 V0190：nelem
 * nelem（int；函数 runTreeSplit 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.directRecvCopySend(offset, offset, nelem);
        }
      }
    }
  }
}

/* 中文逐项-类型 T0008：RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ /* 中文内部逻辑 F0061：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：using Proto = ProtoSimple<ALLREDUCE_CHUNKSTEPS/ALLREDUCE_SLICESTEPS, ALLREDUCE_SLICESTEPS>;；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 调用 / 计算 / 写入：runRing<T, RedOp, Proto>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0061
 */
/* 中文逐项-函数 F0061：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllReduce, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, Proto>。
 */void run(ncclWorkElem *args) {
    using Proto = ProtoSimple<ALLREDUCE_CHUNKSTEPS/ALLREDUCE_SLICESTEPS, ALLREDUCE_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
};

/* 中文逐项-类型 T0009：RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_SIMPLE>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ /* 中文内部逻辑 F0062：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#if CUDART_VERSION >= 11020 && CUDART_VERSION < 11040 && __CUDA_ARCH__ >= 800；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 调用 / 计算 / 写入：runTreeUpDown<T, RedOp, ProtoSimple<1, 1>>(args);；先向树根归约再向下广播结果，同一个线程组先后执行两个阶段。
 *   3. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     4. 调用 / 计算 / 写入：runTreeSplit<T, RedOp, ProtoSimple<1, 1>>(args);；把非根 block 线程分成上行归约与下行广播两组，用不同 barrier/group 形成重叠流水。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0062
 */
/* 中文逐项-函数 F0062：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllReduce, T,
 * RedOp, NCCL_ALGO_TREE, NCCL_PROTO_SIMPLE>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runTreeSplit<T, RedOp, ProtoSimple<1, 1>>；runTreeUpDown<T, RedOp, ProtoSimple<1, 1>>。
 */void run(ncclWorkElem *args) {
    #if CUDART_VERSION >= 11020 && CUDART_VERSION < 11040 && __CUDA_ARCH__ >= 800
      runTreeUpDown<T, RedOp, ProtoSimple<1, 1>>(args);
    #else
      runTreeSplit<T, RedOp, ProtoSimple<1, 1>>(args);
    #endif
  }
};

/* 中文逐项-类型 T0010：RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_COLLNET, NCCL_PROTO_SIMPLE>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_COLLNET, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ /* 中文内部逻辑 F0063：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：static constexpr int COLLNET_COPY_THREADS = 96;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：const int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：const int bid = args->bid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：const int nChannels = args->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：struct ncclDirect* tree = &ncclShmem.channel.collTree;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：const ssize_t chunkSize = int(args->lastChunkSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 准备局部状态：const ssize_t size = args->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 准备局部状态：const ssize_t loopSize = nChannels*tree->nHeads*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 准备局部状态：const int hasUp = (tree->up[0] >= 0) ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 准备局部状态：const int hasDn = (tree->down[0] >= 0) ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 准备局部状态：const int nThreadsScatter = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 3*COLLNET_COPY_THREADS : 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 准备局部状态：const int nThreadsGather = ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 2*COLLNET_COPY_THREADS : 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：const int nThreadsBcast = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 0 : 2*COLLNET_COPY_THREADS);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 准备局部状态：const int nThreadsReduce = args->header.nWarps*WARP_SIZE - nThreadsScatter - nThreadsGather - nThreadsBcast;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 准备局部状态：const int tidStartBcast = nThreadsGather;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 16. 准备局部状态：const int tidStartScatter = tidStartBcast + nThreadsBcast;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 17. 准备局部状态：const int tidStartReduce = tidStartScatter + nThreadsScatter;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 18. 源码语法片段：using Proto = ProtoSimple<1, 1>;；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 19. 条件判断：(tid >= tidStartScatter && tid < tidStartReduce && hasUp)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 准备局部状态：int group = (2*Proto::MaxGroupWidth) | (1<<16);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     22. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<0, NCCL_MAX_DIRECT_ARITY>, 1, Proto, 0> prims(tid-tidStartScatter, nThreadsScatter, NULL, tree->up, args->sendbuff, args->recvbuff, args->redOpArg, group, args);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     23. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       24. 准备局部状态：ssize_t offset = gridOffset + bid*tree->nHeads*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       25. 准备局部状态：int nelem = min(tree->nHeads*chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       26. 条件判断：(args->regUsed)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         27. 真分支：；仅条件为真执行以下子项。
 *           28. 调用 / 计算 / 写入：prims.directScatter(offset, nelem, chunkSize, tree->headRank, tree->shift);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         29. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           30. 调用 / 计算 / 写入：prims.scatter(offset, nelem, chunkSize, tree->headRank, tree->shift);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     31. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   32. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     33. 条件判断：(tid >= tidStartReduce && tree->out != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       34. 真分支：；仅条件为真执行以下子项。
 *         35. 准备局部状态：int group = (3*Proto::MaxGroupWidth) | (1<<16);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         36. 条件判断：(hasDn)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           37. 真分支：；仅条件为真执行以下子项。
 *             38. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 1>, 1, Proto, 0> prims(tid-tidStartReduce, nThreadsReduce, tree->down, &tree->out, args->sendbuff, args->recvbuff, args->redOpArg, group, args …（完整代码见函数体）；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             39. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               40. 准备局部状态：ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               41. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *               42. 条件判断：(args->regUsed)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 43. 真分支：；仅条件为真执行以下子项。
 *                   44. 调用 / 计算 / 写入：prims.directRecvReduceSend(offset, offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 45. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   46. 调用 / 计算 / 写入：prims.recvReduceSend(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             47. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           48. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             49. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<0, 1>, 0, Proto, 0> prims(tid-tidStartReduce, nThreadsReduce, nullptr, &tree->out, args->sendbuff, args->recvbuff, args->redOpArg, group);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             50. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               51. 准备局部状态：ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               52. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *               53. 调用 / 计算 / 写入：prims.send(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             54. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       55. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         56. 条件判断：(tid < tidStartBcast && hasUp)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           57. 真分支：；仅条件为真执行以下子项。
 *             58. 准备局部状态：int group = (0*Proto::MaxGroupWidth) | (0<<16);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             59. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 0>, 1, Proto, 0> prims(tid, nThreadsGather, tree->up, NULL, args->sendbuff, args->recvbuff, args->redOpArg, group, args);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             60. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               61. 准备局部状态：ssize_t offset = gridOffset + bid*tree->nHeads*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               62. 准备局部状态：int nelem = min(tree->nHeads*chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *               63. 调用 / 计算 / 写入：prims.directGather(offset, nelem, chunkSize, tree->headRank, tree->shift);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             64. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           65. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             66. 条件判断：(tid >= tidStartBcast && tid < tidStartScatter && tree->out != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               67. 真分支：；仅条件为真执行以下子项。
 *                 68. 准备局部状态：int group = (1*Proto::MaxGroupWidth) | (0<<16);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 69. 条件判断：(hasDn)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   70. 真分支：；仅条件为真执行以下子项。
 *                     71. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DIRECT_ARITY>, 1, Proto, 0> prims(tid-tidStartBcast, nThreadsBcast, &tree->out, tree->down, args->sendbuff, args->recvbuff, args->redOpArg, group, args);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                     72. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                       73. 准备局部状态：ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       74. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *                       75. 调用 / 计算 / 写入：prims.recvCopyDirectSend(offset, offset, nelem, true);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                     76. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *                   77. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                     78. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<1, 0>, 0, Proto, 0> prims(tid-tidStartBcast, nThreadsBcast, &tree->out, nullptr, args->sendbuff, args->recvbuff, args->redOpArg, group);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                     79. 循环：初始 ssize_t gridOffset = 0;；继续条件 gridOffset < size；每轮更新 gridOffset += loopSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                       80. 准备局部状态：ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       81. 准备局部状态：int nelem = min(chunkSize, size-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *                       82. 调用 / 计算 / 写入：prims.recv(offset, nelem, true);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                     83. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0063
 */
/* 中文逐项-函数 F0063：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllReduce, T,
 * RedOp, NCCL_ALGO_COLLNET, NCCL_PROTO_SIMPLE>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：int；min；prims.directGather；prims.directRecvReduceSend；prims.directScatter；prims.recv；prims.recvCopyDirectSend；prims.recvReduceSend；prims.scatter；prims.send。
 * 需要对照的边界/推进条件：tid >= tidStartScatter && tid < tidStartReduce && hasUp；gridOffset < size；args->regUsed；tid >=
 * tidStartReduce && tree->out != -1。各条件所属分支见紧邻函数体。
 */void run(ncclWorkElem *args) {
/* 中文精读：CollNet 把一次 AllReduce 分成四组协作流水
 * Scatter 把本机输入分配给多个 head；Reduce 组在 head 聚合机内贡献并发往网络。
 * Broadcast 组把网络归约结果分发到机内参与者；Gather 组收齐各个 head 负责的片段。
 * loopSize=nChannels*nHeads*chunkSize，因此除 channel 分片外，还多了一层 head 分片。
 * 网络端点 tree->out 对应特殊 peer，Proxy 调插件 Iallreduce；GPU 不直接在交换机上执行归约。
 * 本版此特化只支持 Simple，启用需要 CollNet 插件与对应硬件/数据类型支持。
 */
/* 中文逐项-变量 V0191：COLLNET_COPY_THREADS
 * COLLNET_COPY_THREADS（int；函数 run 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 COLLNET_COPY_THREADS = 96；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    static constexpr int COLLNET_COPY_THREADS = 96;
/* 中文逐项-变量 V0192：tid
 * tid（int；函数 run 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int tid = threadIdx.x;
/* 中文逐项-变量 V0193：bid
 * bid（int；函数 run 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = args->bid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int bid = args->bid;
/* 中文逐项-变量 V0194：nChannels
 * nChannels（int；函数 run 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = args->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nChannels = args->nChannels;
/* 中文逐项-变量 V0195：tree
 * tree（struct ncclDirect；函数 run 内）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 * 初值/来源 tree = &ncclShmem.channel.collTree；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    struct ncclDirect* tree = &ncclShmem.channel.collTree;
/* 中文逐项-变量 V0196：chunkSize
 * chunkSize（ssize_t；函数 run 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = int(args->lastChunkSize)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t chunkSize = int(args->lastChunkSize);
/* 中文逐项-变量 V0197：size
 * size（ssize_t；函数 run 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t size = args->count;
/* 中文逐项-变量 V0198：loopSize
 * loopSize（ssize_t；函数 run 内）：一轮全部 channel（以及 Ring ranks/CollNet heads）覆盖的逻辑数据范围。
 * 初值/来源 loopSize = nChannels*tree->nHeads*chunkSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t loopSize = nChannels*tree->nHeads*chunkSize;

/* 中文逐项-变量 V0199：hasUp
 * hasUp（int；函数 run 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 hasUp = (tree->up[0] >= 0) ? 1 : 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int hasUp = (tree->up[0] >= 0) ? 1 : 0;
/* 中文逐项-变量 V0200：hasDn
 * hasDn（int；函数 run 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 hasDn = (tree->down[0] >= 0) ? 1 : 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int hasDn = (tree->down[0] >= 0) ? 1 : 0;
/* 中文逐项-变量 V0201：nThreadsScatter
 * nThreadsScatter（int；函数 run 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nThreadsScatter = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 3*COLLNET_COPY_THREADS :
 * 0)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const int nThreadsScatter = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 3*COLLNET_COPY_THREADS : 0);
/* 中文逐项-变量 V0202：nThreadsGather
 * nThreadsGather（int；函数 run 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nThreadsGather = ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 2*COLLNET_COPY_THREADS :
 * 0)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const int nThreadsGather  =             ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 2*COLLNET_COPY_THREADS : 0);
/* 中文逐项-变量 V0203：nThreadsBcast
 * nThreadsBcast（int；函数 run 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nThreadsBcast = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 0 :
 * 2*COLLNET_COPY_THREADS)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const int nThreadsBcast   = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 0 : 2*COLLNET_COPY_THREADS);
/* 中文逐项-变量 V0204：nThreadsReduce
 * nThreadsReduce（int；函数 run 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nThreadsReduce = args->header.nWarps*WARP_SIZE - nThreadsScatter - nThreadsGather -
 * nThreadsBcast；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nThreadsReduce = args->header.nWarps*WARP_SIZE - nThreadsScatter - nThreadsGather - nThreadsBcast;
/* 中文逐项-变量 V0205：tidStartBcast
 * tidStartBcast（int；函数 run 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 tidStartBcast = nThreadsGather；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    const int tidStartBcast = nThreadsGather;
/* 中文逐项-变量 V0206：tidStartScatter
 * tidStartScatter（int；函数 run 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 tidStartScatter = tidStartBcast + nThreadsBcast；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const int tidStartScatter = tidStartBcast + nThreadsBcast;
/* 中文逐项-变量 V0207：tidStartReduce
 * tidStartReduce（int；函数 run 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 tidStartReduce = tidStartScatter + nThreadsScatter；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const int tidStartReduce = tidStartScatter + nThreadsScatter;

    using Proto = ProtoSimple<1, 1>;

    if (tid >= tidStartScatter && tid < tidStartReduce && hasUp) {
      // Scatter
/* 中文逐项-变量 V0208：group
 * group（int；函数 run 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = (2*Proto::MaxGroupWidth) | (1<<16)；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
      int group = (2*Proto::MaxGroupWidth) | (1<<16);
/* 中文逐项-变量 V0209：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<0, NCCL_MAX_DIRECT_ARITY>, / *Direct=* /1, Proto, 0>；函数 run 内）：当前 GPU
 * 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid-tidStartScatter, nThreadsScatter, NULL, tree->up, args->sendbuff, args->recvbuff,
 * args->redOpArg, group, args)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanAsymmetric<0, NCCL_MAX_DIRECT_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid-tidStartScatter, nThreadsScatter, NULL, tree->up, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
      for (/* 中文逐项-变量 V0210：gridOffset
 * gridOffset（ssize_t；函数 run 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0211：offset
 * offset（ssize_t；函数 run 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*tree->nHeads*chunkSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        ssize_t offset = gridOffset + bid*tree->nHeads*chunkSize;
/* 中文逐项-变量 V0212：nelem
 * nelem（int；函数 run 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(tree->nHeads*chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
        int nelem = min(tree->nHeads*chunkSize, size-offset);
        if (args->regUsed) {
          prims.directScatter(offset, nelem, chunkSize, tree->headRank, tree->shift);
        } else {
          prims.scatter(offset, nelem, chunkSize, tree->headRank, tree->shift);
        }
      }
    } else if (tid >= tidStartReduce && tree->out != -1) {
/* 中文逐项-变量 V0213：group
 * group（int；函数 run 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = (3*Proto::MaxGroupWidth) | (1<<16)；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
      int group = (3*Proto::MaxGroupWidth) | (1<<16);
      if (hasDn) {
        // Reduce, send to network
/* 中文逐项-变量 V0214：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 1>, / *Direct=* /1, Proto, 0>；函数 run 内）：当前 GPU
 * 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid-tidStartReduce, nThreadsReduce, tree->down, &tree->out, args->sendbuff, args->recvbuff,
 * args->redOpArg, group, args)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 1>, /*Direct=*/1, Proto, 0>
          prims(tid-tidStartReduce, nThreadsReduce, tree->down, &tree->out, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
        for (/* 中文逐项-变量 V0215：gridOffset
 * gridOffset（ssize_t；函数 run 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0216：offset
 * offset（ssize_t；函数 run 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
/* 中文逐项-变量 V0217：nelem
 * nelem（int；函数 run 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          if (args->regUsed) {
            prims.directRecvReduceSend(offset, offset, nelem);
          } else {
            prims.recvReduceSend(offset, nelem);
          }
        }
      } else {
        // Directly send to network
/* 中文逐项-变量 V0218：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<0, 1>, / *Direct=* /0, Proto, 0>；函数 run 内）：当前 GPU
 * 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid-tidStartReduce, nThreadsReduce, nullptr, &tree->out, args->sendbuff, args->recvbuff,
 * args->redOpArg, group)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        Primitives<T, RedOp, FanAsymmetric<0, 1>, /*Direct=*/0, Proto, 0>
          prims(tid-tidStartReduce, nThreadsReduce, nullptr, &tree->out, args->sendbuff, args->recvbuff, args->redOpArg, group);
        for (/* 中文逐项-变量 V0219：gridOffset
 * gridOffset（ssize_t；函数 run 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0220：offset
 * offset（ssize_t；函数 run 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
/* 中文逐项-变量 V0221：nelem
 * nelem（int；函数 run 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
    } else if (tid < tidStartBcast && hasUp) {
      // Gather
/* 中文逐项-变量 V0222：group
 * group（int；函数 run 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = (0*Proto::MaxGroupWidth) | (0<<16)；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
      int group = (0*Proto::MaxGroupWidth) | (0<<16);
/* 中文逐项-变量 V0223：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 0>, / *Direct=* /1, Proto, 0>；函数 run 内）：当前 GPU
 * 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nThreadsGather, tree->up, NULL, args->sendbuff, args->recvbuff, args->redOpArg, group,
 * args)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 0>, /*Direct=*/1, Proto, 0>
        prims(tid, nThreadsGather, tree->up, NULL, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
      for (/* 中文逐项-变量 V0224：gridOffset
 * gridOffset（ssize_t；函数 run 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0225：offset
 * offset（ssize_t；函数 run 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + bid*tree->nHeads*chunkSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        ssize_t offset = gridOffset + bid*tree->nHeads*chunkSize;
/* 中文逐项-变量 V0226：nelem
 * nelem（int；函数 run 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(tree->nHeads*chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
        int nelem = min(tree->nHeads*chunkSize, size-offset);
        prims.directGather(offset, nelem, chunkSize, tree->headRank, tree->shift);
      }
    } else if (tid >= tidStartBcast && tid < tidStartScatter && tree->out != -1) {
/* 中文逐项-变量 V0227：group
 * group（int；函数 run 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = (1*Proto::MaxGroupWidth) | (0<<16)；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
      int group = (1*Proto::MaxGroupWidth) | (0<<16);
      if (hasDn) {
        // Recv from network, broadcast
/* 中文逐项-变量 V0228：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DIRECT_ARITY>, / *Direct=* /1, Proto, 0>；函数 run 内）：当前 GPU
 * 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid-tidStartBcast, nThreadsBcast, &tree->out, tree->down, args->sendbuff, args->recvbuff,
 * args->redOpArg, group, args)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DIRECT_ARITY>, /*Direct=*/1, Proto, 0>
          prims(tid-tidStartBcast, nThreadsBcast, &tree->out, tree->down, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
        for (/* 中文逐项-变量 V0229：gridOffset
 * gridOffset（ssize_t；函数 run 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0230：offset
 * offset（ssize_t；函数 run 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
/* 中文逐项-变量 V0231：nelem
 * nelem（int；函数 run 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.recvCopyDirectSend(offset, offset, nelem, /*postOp=*/true);
        }
      } else {
        // Recv from network (no post thread needed)
/* 中文逐项-变量 V0232：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<1, 0>, / *Direct=* /0, Proto, 0>；函数 run 内）：当前 GPU
 * 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid-tidStartBcast, nThreadsBcast, &tree->out, nullptr, args->sendbuff, args->recvbuff,
 * args->redOpArg, group)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        Primitives<T, RedOp, FanAsymmetric<1, 0>, /*Direct=*/0, Proto, 0>
          prims(tid-tidStartBcast, nThreadsBcast, &tree->out, nullptr, args->sendbuff, args->recvbuff, args->redOpArg, group);
        for (/* 中文逐项-变量 V0233：gridOffset
 * gridOffset（ssize_t；函数 run 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0234：offset
 * offset（ssize_t；函数 run 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
/* 中文逐项-变量 V0235：nelem
 * nelem（int；函数 run 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, size-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
          int nelem = min(chunkSize, size-offset);
          prims.recv(offset, nelem, /*postOp=*/true);
        }
      }
    }
  }
};

/* 中文逐项-类型 T0011：RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL> {
  __device__ __forceinline__ /* 中文内部逻辑 F0064：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：runRing<T, RedOp, ProtoLL>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0064
 */
/* 中文逐项-函数 F0064：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllReduce, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, ProtoLL>。
 */void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL>(args);
  }
};

/* 中文逐项-类型 T0012：RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL> {
  __device__ __forceinline__ /* 中文内部逻辑 F0065：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：runTreeSplit<T, RedOp, ProtoLL>(args);；把非根 block 线程分成上行归约与下行广播两组，用不同 barrier/group 形成重叠流水。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0065
 */
/* 中文逐项-函数 F0065：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllReduce, T,
 * RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runTreeSplit<T, RedOp, ProtoLL>。
 */void run(ncclWorkElem *args) {
    runTreeSplit<T, RedOp, ProtoLL>(args);
  }
};

/* 中文逐项-类型 T0013：RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128> {
  __device__ __forceinline__ /* 中文内部逻辑 F0066：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：runRing<T, RedOp, ProtoLL128>(args);；执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0066
 */
/* 中文逐项-函数 F0066：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllReduce, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, ProtoLL128>。
 */void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL128>(args);
  }
};

/* 中文逐项-类型 T0014：RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL128>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL128> {
  __device__ __forceinline__ /* 中文内部逻辑 F0067：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：runTreeSplit<T, RedOp, ProtoLL128>(args);；把非根 block 线程分成上行归约与下行广播两组，用不同 barrier/group 形成重叠流水。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__all_reduce_h.md#f0067
 */
/* 中文逐项-函数 F0067：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllReduce, T,
 * RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL128>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runTreeSplit<T, RedOp, ProtoLL128>。
 */void run(ncclWorkElem *args) {
    runTreeSplit<T, RedOp, ProtoLL128>(args);
  }
};
