/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "devcomm.h"
#include "collectives.h"
#include "primitives.h"

/* 中文逐项-类型 T0131：RunWork<ncclFuncSendRecv, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>
 * 类型职责：设备work解释/分派模板，决定一条work中的多个任务怎样由线程执行。
 */
template<typename T, typename RedOp>
/* 中文导读：
 * P2P 的设备端分派：把 work 中的多个发送/接收元素分给线程组，使用 Simple 协议和连接索引 1。
 * 对 self 的匹配通信可转为本地复制；对其他 peer 通过连接能力选择直接访问或缓冲区路径。
 */
struct RunWork<ncclFuncSendRecv, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE> {
/* 中文精读：图示绑定 V3.3 F0374 runSend
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0374
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0070] runSend：设备按chunkSize继续切块；self由发送组复制；零字节用do-while推进一次，与-1无此方向不同（直接对应）
 *   图：25-p2p
 *   定位：#symbol=F0374&binding=B0070
 */
  __device__ __forceinline__ /* 中文内部逻辑 F0374：runSend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(args->peer == ncclShmem.comm.rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 准备局部状态：struct ncclWorkElemP2p* recvArgs = args-1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     4. 条件判断：(args->buff != recvArgs->buff)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       5. 真分支：；仅条件为真执行以下子项。
 *         6. 调用 / 计算 / 写入：ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 0>(tid, nthreads, nullptr, false, 1, (const T**)&args->buff, 1, (T**)&recvArgs->buff, args->count);；统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。
 *   7. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     8. 源码语法片段：using Proto = ProtoSimple<1, 1>;；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 *     9. 准备局部状态：ssize_t const count = args->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     10. 准备局部状态：int const chunkSize = args->chunkSize/sizeof(T);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     11. 准备局部状态：int const peer = args->peer;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     12. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<0, 1>, 1, Proto, 1> prims (tid, nthreads, nullptr, &peer, args->buff, nullptr, 0, group);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     13. 准备局部状态：ssize_t offset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     14. 先执行一次的循环：初始 ；继续条件 (offset < count)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *       15. 准备局部状态：int nelem = min(chunkSize, count-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       16. 调用 / 计算 / 写入：prims.directSend(offset, offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       17. 调用 / 计算 / 写入：offset += nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     18. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__sendrecv_h.md#f0374
 */
/* 中文逐项-函数 F0374：runSend
 * 用途与位置：执行分给当前 P2P 线程组的发送任务；self peer 走本地复制，其他 peer 用 Simple 原语分块。
 * 参数 tid（const int tid）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 参数 nthreads（const int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 参数 group（const int group）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 参数 args（struct ncclWorkElemP2p* args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/sendrecv.h::run。
 * 本体直接出现的调用/包装：ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 0>；min；prims.directSend。
 * 需要对照的边界/推进条件：args->peer == ncclShmem.comm.rank；args->buff != recvArgs->buff。各条件所属分支见紧邻函数体。
 */void runSend(const int tid, const int nthreads, const int group, struct ncclWorkElemP2p* args) {
    if (args->peer == ncclShmem.comm.rank) {
/* 中文逐项-变量 V0833：recvArgs
 * recvArgs（struct ncclWorkElemP2p；函数 runSend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvArgs = args-1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      struct ncclWorkElemP2p* recvArgs = args-1;
      if (args->buff != recvArgs->buff) {
        ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 0>(tid, nthreads, nullptr, false, 1, (const T**)&args->buff, 1, (T**)&recvArgs->buff, args->count);
      }
    } else {
      using Proto = ProtoSimple<1, 1>;
/* 中文逐项-变量 V0834：count
 * count（ssize_t；函数 runSend 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 初值/来源 count = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      ssize_t const count = args->count;
/* 中文逐项-变量 V0835：chunkSize
 * chunkSize（int；函数 runSend 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = args->chunkSize/sizeof(T)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
      int const chunkSize = args->chunkSize/sizeof(T);
/* 中文逐项-变量 V0836：peer
 * peer（int；函数 runSend 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = args->peer；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int const peer = args->peer;
/* 中文逐项-变量 V0837：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<0, 1>, 1, Proto, 1>；函数 runSend 内）：当前 GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次
 * kernel。
 * 初值/来源 prims = (tid, nthreads, nullptr, &peer, args->buff, nullptr, / *redOpArg(ignored)=* /0,
 * group)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanAsymmetric<0, 1>, 1, Proto, 1> prims
        (tid, nthreads, nullptr, &peer, args->buff, nullptr, /*redOpArg(ignored)=*/0, group);
/* 中文精读：P2P 的第二层切块：在选定 channel 内推进
 * Group 已把原消息切成分配给 channel 的块，此处对当前 args->count 再按 chunkSize 循环。
 * 每轮 directSend 内还有 Simple 的 wait/copy/post，CPU Proxy 根据同一尺寸推进网络。
 * do-while 使 count=0 也执行一次协议同步；它并非遗漏了空数据检查。
 * 这条路径只复制数据，不做集合归约；All-to-All 可由上层为所有 peer 组合这样的 Send/Recv。
 */
/* 中文逐项-变量 V0838：offset
 * offset（ssize_t；函数 runSend 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      ssize_t offset = 0;
      do {
/* 中文逐项-变量 V0839：nelem
 * nelem（int；函数 runSend 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, count-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
        int nelem = min(chunkSize, count-offset);
        prims.directSend(offset, offset, nelem);
        offset += nelem;
      } while(offset < count);
    }
  }

/* 中文精读：图示绑定 V3.3 F0375 runRecv
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0375
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0071] runRecv：设备按chunkSize继续切块；self由发送组复制；零字节用do-while推进一次，与-1无此方向不同（直接对应）
 *   图：25-p2p
 *   定位：#symbol=F0375&binding=B0071
 */
  __device__ __forceinline__ /* 中文内部逻辑 F0375：runRecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(args->peer != ncclShmem.comm.rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 源码语法片段：using Proto = ProtoSimple<1, 1>;；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 *     4. 准备局部状态：ssize_t const count = args->count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 准备局部状态：int const chunkSize = args->chunkSize/sizeof(T);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 准备局部状态：int const peer = args->peer;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     7. 准备局部状态：Primitives<T, RedOp, FanAsymmetric<1, 0>, 1, Proto, 1> prims (tid, nthreads, &peer, nullptr, nullptr, args->buff, 0, group);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     8. 准备局部状态：ssize_t offset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 先执行一次的循环：初始 ；继续条件 (offset < count)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *       10. 准备局部状态：int nelem = min(chunkSize, count-offset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       11. 调用 / 计算 / 写入：prims.directRecv(offset, nelem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       12. 调用 / 计算 / 写入：offset += nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__sendrecv_h.md#f0375
 */
/* 中文逐项-函数 F0375：runRecv
 * 用途与位置：执行 P2P 接收线程组任务；self peer 的数据由配对发送组复制，其他 peer 用 Simple 接收。
 * 参数 tid（const int tid）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 参数 nthreads（const int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 参数 group（const int group）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 参数 args（struct ncclWorkElemP2p* args）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/sendrecv.h::run。
 * 本体直接出现的调用/包装：min；prims.directRecv。
 * 需要对照的边界/推进条件：args->peer != ncclShmem.comm.rank。各条件所属分支见紧邻函数体。
 */void runRecv(const int tid, const int nthreads, const int group, struct ncclWorkElemP2p* args) {
    if (args->peer != ncclShmem.comm.rank) {
      using Proto = ProtoSimple<1, 1>;
/* 中文逐项-变量 V0840：count
 * count（ssize_t；函数 runRecv 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 初值/来源 count = args->count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      ssize_t const count = args->count;
/* 中文逐项-变量 V0841：chunkSize
 * chunkSize（int；函数 runRecv 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = args->chunkSize/sizeof(T)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
      int const chunkSize = args->chunkSize/sizeof(T);
/* 中文逐项-变量 V0842：peer
 * peer（int；函数 runRecv 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = args->peer；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int const peer = args->peer;
/* 中文逐项-变量 V0843：prims
 * prims（Primitives<T, RedOp, FanAsymmetric<1, 0>, 1, Proto, 1>；函数 runRecv 内）：当前 GPU 线程组的协议原语对象，持有连接、缓冲和进度，不是单独的一次
 * kernel。
 * 初值/来源 prims = (tid, nthreads, &peer, nullptr, nullptr, args->buff, / *redOpArg(ignored)=* /0,
 * group)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      Primitives<T, RedOp, FanAsymmetric<1, 0>, 1, Proto, 1> prims
        (tid, nthreads, &peer, nullptr, nullptr, args->buff, /*redOpArg(ignored)=*/0, group);
/* 中文逐项-变量 V0844：offset
 * offset（ssize_t；函数 runRecv 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      ssize_t offset = 0;
      do {
/* 中文逐项-变量 V0845：nelem
 * nelem（int；函数 runRecv 内）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 初值/来源 nelem = min(chunkSize, count-offset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
        int nelem = min(chunkSize, count-offset);
        prims.directRecv(offset, nelem);
        offset += nelem;
      } while(offset < count);
    }
  }

/* 中文精读：图示绑定 V3.3 F0376 run
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0376
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0072] run：设备按chunkSize继续切块；self由发送组复制；零字节用do-while推进一次，与-1无此方向不同（直接对应）
 *   图：25-p2p
 *   定位：#symbol=F0376&binding=B0072
 */
  __device__ __forceinline__ /* 中文内部逻辑 F0376：run
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclWorkElemP2p* args = work->p2pElems;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int ngroups = args->ngroups;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int tid = threadIdx.x;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int wid = tid / WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 编译期分支/指令：#define NWARPS (NCCL_MAX_NTHREADS/WARP_SIZE)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 6. 准备局部状态：int group = ngroups-1- (NWARPS-1-wid) * ngroups / NWARPS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：args += group;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 条件判断：(args->header.type == ncclWorkTypeUnused)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 11. 调用 / 计算 / 写入：tid -= args->warpStart * WARP_SIZE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 准备局部状态：int nthreads = args->nWarps * WARP_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 调用 / 计算 / 写入：group |= 1<<16;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 条件判断：(tid >= nthreads || args->peer == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 17. 条件判断：((group%2) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   18. 真分支：；仅条件为真执行以下子项。
 *     19. 调用 / 计算 / 写入：runRecv(tid, nthreads, group, args);；执行 P2P 接收线程组任务；self peer 的数据由配对发送组复制，其他 peer 用 Simple 接收。
 *   20. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     21. 调用 / 计算 / 写入：runSend(tid, nthreads, group, args);；执行分给当前 P2P 线程组的发送任务；self peer 走本地复制，其他 peer 用 Simple 原语分块。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__sendrecv_h.md#f0376
 */
/* 中文逐项-函数 F0376：run
 * 用途与位置：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素；所属特化 RunWork<ncclFuncSendRecv, T, RedOp,
 * NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>。
 * 参数 work（ncclWork *work）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：runRecv；runSend。
 * 需要对照的边界/推进条件：args->header.type == ncclWorkTypeUnused；tid >= nthreads || args->peer == -1；group%2) ==
 * 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 */void run(ncclWork *work) {
/* 中文逐项-变量 V0846：args
 * args（struct ncclWorkElemP2p；函数 run 内）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 * 初值/来源 args = work->p2pElems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclWorkElemP2p* args = work->p2pElems;
/* 中文逐项-变量 V0847：ngroups
 * ngroups（int；函数 run 内）：同一 P2P work 内同时划分的收发线程组数，向上按二次幂组织。
 * 初值/来源 ngroups = args->ngroups；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int ngroups = args->ngroups;
/* 中文逐项-变量 V0848：tid
 * tid（int；函数 run 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = threadIdx.x；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int tid = threadIdx.x;
/* 中文逐项-变量 V0849：wid
 * wid（int；函数 run 内）：线程在 warp 内的 lane 编号或代码定义的 warp 索引，必须以本处赋值为准。
 * 初值/来源 wid = tid / WARP_SIZE；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int wid = tid / WARP_SIZE;
    // This has to work even for groups of 2.5 warps (which is 8 groups, and means 3
    // warps for send, 2 warps for recv).
    // warpStarts were rounded thanks to int division, but for group number we need to round the other way around
    // So we mirror wid then mirror again the group.
/* 中文逐项-常量 X0018：NWARPS
 * 编译期定义：NWARPS = (NCCL_MAX_NTHREADS/WARP_SIZE)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
    #define NWARPS (NCCL_MAX_NTHREADS/WARP_SIZE)
/* 中文逐项-变量 V0850：group
 * group（int；函数 run 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = ngroups-1- (NWARPS-1-wid) * ngroups / NWARPS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int group = ngroups-1- (NWARPS-1-wid) * ngroups / NWARPS;
    args += group;
    if (args->header.type == ncclWorkTypeUnused) return;

    tid -= args->warpStart * WARP_SIZE;
/* 中文逐项-变量 V0851：nthreads
 * nthreads（int；函数 run 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nthreads = args->nWarps * WARP_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int nthreads = args->nWarps * WARP_SIZE;
/* 中文精读：线程组编码与连接索引
 * work 里收发按偶/奇槽分组，先算当前 warp 属于哪个元素，再减 warpStart 得到组内 tid。
 * 第 16 位是传给 Simple 构造函数的连接索引编码，使 P2P 找 peers[peer].send/recv[1]。
 * 这个位不是 channelId，也不是 GPU rank；低位仍用于组共享内存和 barrier 分配。
 * self send 在 runSend 中直接复制到前一个匹配 recv 元素的缓冲，runRecv 的 self 分支不重复搬运。
 */
    group |= 1<<16; // Used to select connIndex 1
    if (tid >= nthreads || args->peer == -1) return;
    if ((group%2) == 0) {
      runRecv(tid, nthreads, group, args);
    } else {
      runSend(tid, nthreads, group, args);
    }
  }
};
