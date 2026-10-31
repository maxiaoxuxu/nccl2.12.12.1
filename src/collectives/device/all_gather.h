/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "primitives.h"

namespace {
/* 中文逐项-函数 F0054：runRing
 * 用途与位置：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进；当前API文件为 all_gather。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/all_gather.h::run；src/collectives/device/all_reduce.h::run；src/collectives/device/broadcast.h::run；src/collectives/device/reduce.h::run；src/collectives/device/reduce_scatter.h::run。
 * 本体直接出现的调用/包装：Proto::calcBytePerGrain；Proto::calcBytePerStep；divUp；int；min；prims.directCopySend；prims.directRecv；prims.directRecvCopySend；prims.directSend；roundUp。
 * 需要对照的边界/推进条件：gridOffset < size；Proto::Id == NCCL_PROTO_SIMPLE；Proto::Id == NCCL_PROTO_LL；Proto::Id ==
 * NCCL_PROTO_LL128。各条件所属分支见紧邻函数体。
 */
  template<typename T, typename RedOp, typename Proto>
/* 中文导读：
 * Ring AllGather：将本 rank 的贡献放到按用户 rank 排列的输出位置，再沿环接收并转发其他贡献。
 * 与 AllReduce 的结果传播阶段相似，但不执行跨 rank 的归约运算；输出按用户 rank 而非内部环顺序排列。
 */
  __device__ __forceinline__ void runRing(ncclWorkElem *args) {
/* 中文逐项-变量 V0092：tid
 * tid（int；函数 runRing 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int tid = threadIdx.x;
/* 中文逐项-变量 V0093：nthreads
 * nthreads（int；函数 runRing 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nthreads = args->header.nWarps*WARP_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nthreads = args->header.nWarps*WARP_SIZE;
/* 中文逐项-变量 V0094：bid
 * bid（int；函数 runRing 内）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 * 初值/来源 bid = args->bid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int bid = args->bid;
/* 中文逐项-变量 V0095：nChannels
 * nChannels（int；函数 runRing 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = args->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nChannels = args->nChannels;
/* 中文逐项-变量 V0096：ring
 * ring（ncclRing；函数 runRing 内）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 * 初值/来源 ring = &ncclShmem.channel.ring；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    ncclRing *ring = &ncclShmem.channel.ring;
/* 中文逐项-变量 V0097：ringRanks
 * ringRanks（int；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ringRanks = ring->devUserRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int *ringRanks = ring->devUserRanks;
/* 中文逐项-变量 V0098：chunkSize
 * chunkSize（ssize_t；函数 runRing 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLGATHER_CHUNKSTEPS :
 * 1))；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLGATHER_CHUNKSTEPS : 1));
    // We should not need the final /2 but it makes performance much, much smoother. Might be a bug somewhere.
/* 中文逐项-变量 V0099：minChunkSizeLL128
 * minChunkSizeLL128（ssize_t；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minChunkSizeLL128 =
 * int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2);
/* 中文逐项-变量 V0100：nranks
 * nranks（int；函数 runRing 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = ncclShmem.comm.nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int nranks = ncclShmem.comm.nRanks;
/* 中文逐项-变量 V0101：loopSize
 * loopSize（ssize_t；函数 runRing 内）：一轮全部 channel（以及 Ring ranks/CollNet heads）覆盖的逻辑数据范围。
 * 初值/来源 loopSize = nChannels*int(chunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const ssize_t loopSize = nChannels*int(chunkSize);
/* 中文逐项-变量 V0102：size
 * size（ssize_t；函数 runRing 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const ssize_t size = args->count;

/* 中文逐项-变量 V0103：inputBuf
 * inputBuf（T；函数 runRing 内）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 初值/来源 inputBuf = (T*)args->sendbuff；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    T *inputBuf = (T*)args->sendbuff;
/* 中文逐项-变量 V0104：outputBuf
 * outputBuf（T；函数 runRing 内）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 初值/来源 outputBuf = (T*)args->recvbuff；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    T *outputBuf = (T*)args->recvbuff;
/* 中文逐项-变量 V0105：prims
 * prims（Primitives<T, RedOp, FanSymmetric<1>, 1, Proto, 0>；函数 runRing 内）：当前 GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次 kernel。
 * 初值/来源 prims = (tid, nthreads, &ring->prev, &ring->next, inputBuf, outputBuf,
 * args->redOpArg)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    Primitives<T, RedOp, FanSymmetric<1>, 1, Proto, 0> prims
      (tid, nthreads, &ring->prev, &ring->next, inputBuf, outputBuf, args->redOpArg);

    for (/* 中文逐项-变量 V0106：gridOffset
 * gridOffset（ssize_t；函数 runRing 内）：所有通道共同的当前轮起点，通常以设备元素数计，每轮增加 loopSize。
 * 初值/来源 gridOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：gridOffset < size；循环体与增量表达式决定每轮如何推进。
 */ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
/* 中文逐项-变量 V0107：realChunkSize
 * realChunkSize（ssize_t；函数 runRing 内）：结合剩余数据、并行通道和对齐粒度计算的本轮块容量，不保证每块都填满。
 * 此处未显式给初值；后续首个文本使用点：realChunkSize = min(chunkSize, divUp(size-gridOffset,nChannels));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset,nChannels));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else if (Proto::Id == NCCL_PROTO_LL)
        realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;
      else if (Proto::Id == NCCL_PROTO_LL128)
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128);
      realChunkSize = int(realChunkSize);

/* 中文逐项-变量 V0108：chunkOffset
 * chunkOffset（ssize_t；函数 runRing 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 chunkOffset = gridOffset + int(bid*realChunkSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      ssize_t chunkOffset = gridOffset + int(bid*realChunkSize);

      /////////////// begin AllGather steps ///////////////
/* 中文逐项-变量 V0109：offset
 * offset（ssize_t；函数 runRing 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：offset = chunkOffset + rankDest * size;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ssize_t offset;
/* 中文逐项-变量 V0110：nelem
 * nelem（int；函数 runRing 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(realChunkSize, size-chunkOffset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
      int nelem = min(realChunkSize, size-chunkOffset);
/* 中文逐项-变量 V0111：rankDest
 * rankDest（int；函数 runRing 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：rankDest = ringRanks[0];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int rankDest;

      // step 0: push data to next GPU
/* 中文精读：AllGather 的布局、分片与原地操作
 * size 是本 rank 贡献长度，输出总长度 nranks*size。该复制 API 已归一化成 Int8，故单位为字节。
 * channel 负责本地贡献的一段 chunkOffset，写入 output[用户rank*size + chunkOffset]。
 * 之后每步把上游其他用户 rank 的同一段保存并转发，最终每个 rank 都有完整有序输出。
 * 输入恰好等于自己在输出中的段时可直接发送；否则 copySend 兼顾本地写入和发下游。
 * 例：4 rank 每人 8B，rank2 的贡献最终总在 output[16,24)，不随环顺序改变。
 */
      rankDest = ringRanks[0];
      offset = chunkOffset + rankDest * size;

      if (inputBuf + chunkOffset == outputBuf + offset) { // In place
        prims.directSend(chunkOffset, offset, nelem);
      } else {
        prims.directCopySend(chunkOffset, offset, offset, nelem);
      }

      // k-2 steps: copy to next GPU
      for (/* 中文逐项-变量 V0112：j
 * j（int；函数 runRing 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：j<nranks-1；循环体与增量表达式决定每轮如何推进。
 */int j=1; j<nranks-1; ++j) {
        rankDest = ringRanks[nranks-j];
        offset = chunkOffset + rankDest * size;

        prims.directRecvCopySend(offset, offset, nelem);
      }

      // Make final copy from buffer to dest.
      rankDest = ringRanks[1];
      offset = chunkOffset + rankDest * size;

      // Final wait/copy.
      prims.directRecv(offset, nelem);
    }
  }
}

/* 中文逐项-类型 T0005：RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
  __device__ __forceinline__ /* 中文逐项-函数 F0055：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllGather, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, Proto>。
 */void run(ncclWorkElem *args) {
    using Proto = ProtoSimple<ALLGATHER_CHUNKSTEPS/ALLGATHER_SLICESTEPS, ALLGATHER_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
};

/* 中文逐项-类型 T0006：RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL> {
  __device__ __forceinline__ /* 中文逐项-函数 F0056：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllGather, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, ProtoLL>。
 */void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL>(args);
  }
};

/* 中文逐项-类型 T0007：RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>
 * 类型职责：具体API×算法×协议的设备模板特化，run转入同文件算法循环，模板实参决定本块执行哪种实现。
 */
template<typename T, typename RedOp>
struct RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128> {
  __device__ __forceinline__ /* 中文逐项-函数 F0057：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWorkElement<ncclFuncAllGather, T,
 * RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>。
 * 参数 args（ncclWorkElem *args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRing<T, RedOp, ProtoLL128>。
 */void run(ncclWorkElem *args) {
    runRing<T, RedOp, ProtoLL128>(args);
  }
};
