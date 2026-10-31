/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

/* 中文逐项-类型 T0077：Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p >
 * 类型职责：当前协议的GPU通信原语实现/适配层，组合连接流控、数据地址、复制归约和进度发布。
 * 声明内可见成员（含嵌套成员）：MaxRecv、MaxSend、Input、Output、RoleInput、RoleOutput、RoleWaitRecv、RoleWaitSend、RolePostSend、RolePostRecv、Aborted、OffsFifoEnabled、SizesFifoEnabled、DirectWrite、DirectRead、ThreadsSynced、tid、nthreads、nworkers、stepSize、fan、index、flags、group、step、connOffsFifoPtr、userBuff、connEltsFifo、connSizesFifoPtr、directBuff、connStepPtr、connStepCache。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T, typename RedOp, typename Fan, int Direct,
         int SlicePerChunk, int StepPerSlice, int Unroll, int P2p>
class Primitives<
    T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
  > {
/* 中文逐项-变量 V0638：MaxRecv、MaxSend
 * MaxRecv（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 * MaxSend（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 */
  static constexpr int MaxRecv = Fan::MaxRecv, MaxSend = Fan::MaxSend;
/* 中文逐项-变量 V0639：Input、Output
 * Input（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：用户缓冲表的编译期输入/输出槽下标，不是发送/接收rank。
 * Output（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：用户缓冲表的编译期输入/输出槽下标，不是发送/接收rank。
 */
  static constexpr int Input=0, Output=1;
/* 中文逐项-变量 V0640：RoleInput、RoleOutput、RoleWaitRecv、RoleWaitSend、RolePostSend、RolePostRecv、Aborted、OffsFifoEnabled、SizesFifoEnabled、DirectWrite、DirectRead、ThreadsSynced
 * RoleInput（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple线程职责位，分别选择本地指针提供、等待与进度发布角色。
 * RoleOutput（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple线程职责位，分别选择本地指针提供、等待与进度发布角色。
 * RoleWaitRecv（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple线程职责位，分别选择本地指针提供、等待与进度发布角色。
 * RoleWaitSend（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple线程职责位，分别选择本地指针提供、等待与进度发布角色。
 * RolePostSend（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple线程职责位，分别选择本地指针提供、等待与进度发布角色。
 * RolePostRecv（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple线程职责位，分别选择本地指针提供、等待与进度发布角色。
 * Aborted（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：线程本地缓存的中止状态，周期读取共享 abortFlag 后更新以退出自旋。
 * OffsFifoEnabled（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple协议能力/状态位，控制共享偏移、有效大小、直接用户缓冲和同步路径。
 * SizesFifoEnabled（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple协议能力/状态位，控制共享偏移、有效大小、直接用户缓冲和同步路径。
 * DirectWrite（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple协议能力/状态位，控制共享偏移、有效大小、直接用户缓冲和同步路径。
 * DirectRead（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple协议能力/状态位，控制共享偏移、有效大小、直接用户缓冲和同步路径。
 * ThreadsSynced（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：Simple协议能力/状态位，控制共享偏移、有效大小、直接用户缓冲和同步路径。
 */
  static constexpr int RoleInput = 0x01,
                       RoleOutput = 0x02,
                       RoleWaitRecv = 0x04,
                       RoleWaitSend = 0x08,
                       RolePostSend = 0x10,
                       RolePostRecv = 0x20,
                       Aborted = 0x40,
                       OffsFifoEnabled = 0x80,
                       SizesFifoEnabled = 0x100,
                       DirectWrite = 0x200,
                       DirectRead = 0x400,
                       ThreadsSynced = 0x800;
/* 中文逐项-变量 V0641：tid
 * tid（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 */
  const int tid;
/* 中文逐项-变量 V0642：nthreads
 * nthreads（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 */
  int nthreads;
/* 中文逐项-变量 V0643：nworkers
 * nworkers（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p >）：真正搬运数据的
 * worker 数，Simple 可能从 nthreads 中扣掉同步 warp。
 */
  int nworkers;
/* 中文逐项-变量 V0644：stepSize
 * stepSize（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p >）：一 FIFO
 * step 的容量；设备 T 指针路径以元素计，主机缓冲路径以字节计。
 */
  const int stepSize;
/* 中文逐项-变量 V0645：fan
 * fan（Fan；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：原语实际收/发邻居数量，受编译期 MaxRecv/MaxSend 限制。
 */
  Fan fan;
/* 中文逐项-变量 V0646：index
 * index（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 */
  int index; // Peer index I'm responsible for
/* 中文逐项-变量 V0647：flags
 * flags（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：按位组合的当前线程角色/直接访问/共享偏移/中止等状态，按位测试而非单一枚举。
 */
  int flags;
/* 中文逐项-变量 V0648：group
 * group（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 */
  int group;
/* 中文逐项-变量 V0649：step
 * step（uint64_t；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 */
  uint64_t step;
/* 中文逐项-变量 V0650：connOffsFifoPtr
 * connOffsFifoPtr（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：本线程缓存的共享偏移、有效大小或协议数据区地址，减少重复查连接表。
 */
  int *connOffsFifoPtr;   // (flags & OffsFifoEnabled)
/* 中文逐项-类型 T0078：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：userBuff、connEltsFifo。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V0651：userBuff
 * userBuff（T；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >::<匿名>）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 */
    T *userBuff;            // (flags & (RoleInput|RoleOutput))
/* 中文逐项-变量 V0652：connEltsFifo
 * connEltsFifo（T；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >::<匿名>）：本线程缓存的共享偏移、有效大小或协议数据区地址，减少重复查连接表。
 */
    T *connEltsFifo;        // !(flags & (RoleInput|RoleOutput))
  };
/* 中文逐项-类型 T0079：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：connSizesFifoPtr、directBuff。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V0653：connSizesFifoPtr
 * connSizesFifoPtr（int；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >::<匿名>）：本线程缓存的共享偏移、有效大小或协议数据区地址，减少重复查连接表。
 */
    int volatile *connSizesFifoPtr; //  (flags & SizesFifoEnabled)
/* 中文逐项-变量 V0654：directBuff
 * directBuff（T；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >::<匿名>）：已通过协议交换获得的用户缓冲直接访问地址，仅连接能力允许时使用。
 */
    T *directBuff;                  // !(flags & SizesFifoEnabled)
  };
/* 中文逐项-变量 V0655：connStepPtr
 * connStepPtr（uint64_t；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：当前角色要观察或发布的连接 head/tail 地址，发送和接收角色含义相反。
 */
  uint64_t volatile *connStepPtr;
/* 中文逐项-变量 V0656：connStepCache
 * connStepCache（uint64_t；成员 Primitives< T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p
 * >）：最近观察到的连接进度寄存器缓存，充足时避免反复全局读取。
 */
  uint64_t connStepCache; // Cache last seen value of (*connStepPtr)

  // Don't use barrier 0 as it's used by the final sync
/* 中文逐项-函数 F0227：barrier
 * 用途与位置：同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::waitSend；src/collectives/device/prims_ll.h::postRecv；src/collectives/device/prims_ll.h::~Primitives；src/collectives/device/prims_ll128.h::GenericOp；src/collectives/device/prims_ll128.h::~Primitives；src/collectives/device/prims_simple.h::subBarrier；src/collectives/device/prims_simple.h::genericOp；其余见调用清单。
 * 本体直接出现的调用/包装：__syncwarp。
 * 需要对照的边界/推进条件：nthreads == WARP_SIZE。各条件所属分支见紧邻函数体。
 */
  inline __device__ void barrier() {
    if (nthreads == WARP_SIZE)
      __syncwarp();
    else
      asm volatile("bar.sync %0, %1;" :: "r"(15-group), "r"(nthreads));
    flags |= ThreadsSynced;
  }
/* 中文逐项-函数 F0228：subBarrier
 * 用途与位置：只同步负责数据搬运的 worker 子组，配合同步 warp 让复制与进度发布流水重叠。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_simple.h::genericOp；src/collectives/device/prims_simple.h::ScatterGatherOp。
 * 本体直接出现的调用/包装：barrier。
 * 需要对照的边界/推进条件：nworkers == nthreads。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
  inline __device__ void subBarrier() {
    if (nworkers == nthreads)
      barrier();
    else
      asm volatile("bar.sync %0, %1;" :: "r"(8-group), "r"(nworkers));
  }

/* 中文逐项-函数 F0229：checkAbort
 * 用途与位置：自旋达到检查间隔时读取 communicator 中止标志，使异常情况下的协议等待可以退出。
 * 参数 <未命名参数>（int &spins）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::waitSend；src/collectives/device/prims_ll.h::readLL；src/collectives/device/prims_ll.h::readLLFinish；src/collectives/device/prims_ll128.h::waitSend；src/collectives/device/prims_ll128.h::recvReduceSendCopy；src/collectives/device/prims_simple.h::waitPeer；src/collectives/device/prims_simple.h::setDataPtrs。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：!(flags & Aborted) && spins == NCCL_SPINS_BEFORE_CHECK_ABORT。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return flags & Aborted;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ bool checkAbort(int &spins) {
    spins++;
    if (!(flags & Aborted) && spins == NCCL_SPINS_BEFORE_CHECK_ABORT) {
      flags |= *ncclShmem.comm.abortFlag ? Aborted : 0;
      spins = 0;
    }
    return flags & Aborted;
  }

/* 中文逐项-函数 F0230：waitPeer
 * 用途与位置：发送侧等空槽、接收侧等数据，并按 direct/shared/固定 FIFO 条件装配本 slice 的实际指针。
 * 参数 dstIx（intptr_t dstIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteIx（intptr_t remoteIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 offset（int offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 参数 nelts（int nelts）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_simple.h::genericOp；src/collectives/device/prims_simple.h::ScatterGatherOp。
 * 本体直接出现的调用/包装：checkAbort；loadInt。
 * 需要对照的边界/推进条件：flags & (Recv*RoleWaitRecv)) && !noRecvWait) || ((flags & (Send*RoleWaitSend)) &&
 * !noSendWait；connStepCache + (isSendNotRecv ? NCCL_STEPS : 0) < step + StepPerSlice；checkAbort(spins；flags &
 * (Recv*RoleWaitRecv | Send*RoleWaitSend。各条件所属分支见紧邻函数体。
 */
  template <int DirectRecv, int DirectSend, int Recv, int Send, int Src, int Dst>
/* 中文导读：
 * 在真正读写数据前等待输入就绪或输出槽位有空闲，并根据 direct/shared 能力计算本次地址。
 * 等待依据连接进度与 slot，而非每次都做一次全 communicator 同步；中止标志用于打破异常等待。
 */
  __device__ __forceinline__ void waitPeer(intptr_t dstIx, intptr_t remoteIx, int offset, int nelts) {
/* 中文逐项-变量 V0657：isSendNotRecv
 * isSendNotRecv（bool；函数 waitPeer 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 isSendNotRecv = (Send && Recv) ? (flags & RoleWaitSend) : Send；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    const bool isSendNotRecv = (Send && Recv) ? (flags & RoleWaitSend) : Send;
/* 中文逐项-变量 V0658：noRecvWait
 * noRecvWait（bool；函数 waitPeer 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 noRecvWait = DirectRecv && Src && (flags & DirectRead)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    const bool noRecvWait = DirectRecv && Src && (flags & DirectRead);        // no wait when directly reading from remote input
/* 中文逐项-变量 V0659：noSendWait
 * noSendWait（bool；函数 waitPeer 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 noSendWait = DirectSend && (flags & (DirectRead|DirectWrite))；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    const bool noSendWait = DirectSend && (flags & (DirectRead|DirectWrite)); // no wait in empty send (e.g. directScatter) or direct remote write
    if (((flags & (Recv*RoleWaitRecv)) && !noRecvWait) ||
        ((flags & (Send*RoleWaitSend)) && !noSendWait)) {
/* 中文逐项-变量 V0660：spins
 * spins（int；函数 waitPeer 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int spins = 0;
/* 中文精读：把 waitPeer 的不等式读成人话
 * 发送等待：消费者 head + 8 >= 本次结束 step，表示环形数据槽尚未被覆盖，可以写。
 * 接收等待：生产者 tail >= 本次结束 step，表示所需数据已经发布，可以读。
 * connStepCache 先用寄存器缓存，只有不足时才反复读取共享进度，减少热路径访存。
 * DirectRead/Write 在满足协议契约时能跳过部分等待，但仍要遵守指针交换与生命周期约束。
 */
      while (connStepCache + (isSendNotRecv ? NCCL_STEPS : 0) < step + StepPerSlice) {
        connStepCache = *connStepPtr;
        if (checkAbort(spins)) break;
        //if (spins == 0) printf("r=%d b=%d t=%d SPUN OUT got=%d want=%d\n", ncclShmem.comm.rank, blockIdx.x, threadIdx.x, int(connStepCache + (isSendNotRecv ? NCCL_STEPS : 0)), int(step+StepPerSlice));
      }
    }

    if (flags & (Recv*RoleWaitRecv | Send*RoleWaitSend)) {
      if (isSendNotRecv && (flags & SizesFifoEnabled))
        connSizesFifoPtr[step%NCCL_STEPS] = nelts*sizeof(T);

/* 中文逐项-变量 V0661：ptrs
 * ptrs（void；函数 waitPeer 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ptrs = isSendNotRecv ? (ncclShmem.groups[group].dsts + Dst) : (ncclShmem.groups[group].srcs +
 * Src)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void **ptrs = isSendNotRecv ? (ncclShmem.groups[group].dsts + Dst)
                                  : (ncclShmem.groups[group].srcs + Src);
      if (flags & OffsFifoEnabled)
        ptrs[index] = connEltsFifo + loadInt(connOffsFifoPtr + (step%NCCL_STEPS))/sizeof(T);
      else if (isSendNotRecv && DirectSend) {
        if (flags & DirectWrite) {
          ptrs[index] = directBuff + remoteIx + offset;
        } else if (flags & DirectRead) {  // empty send
          ptrs[index] = nullptr;
        } else {
          ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
        }
      } else if (!isSendNotRecv && DirectRecv) {
        if (flags & DirectRead) {
          ptrs[index] = directBuff + remoteIx + offset;
        } else if (flags & DirectWrite) {
          ptrs[index] = directBuff + dstIx + offset;  // send to next from my output buffer
        } else {
          ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
        }
      }
      else {
        ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
      }
      step += StepPerSlice;
    }
  }

/* 中文逐项-函数 F0231：postPeer
 * 用途与位置：发布本 slice 完成后的连接进度，让消费者能读或生产者能回收空间。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_simple.h::genericOp；src/collectives/device/prims_simple.h::ScatterGatherOp。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：flags & (Recv*RolePostRecv | Send*RolePostSend。各条件所属分支见紧邻函数体。
 */
  template<int Recv, int Send>
/* 中文导读：
 * 完成本 slice 后发布对应连接的进度，使远端/GPU/Proxy 能消费数据或回收槽位。
 * 必须结合 genericOp 中的数据写入、线程同步和 fence 阅读，只有 volatile 不能单独保证所有可见性。
 */
  inline __device__ void postPeer() {
    if (flags & (Recv*RolePostRecv | Send*RolePostSend)) {
      step += StepPerSlice;
      *connStepPtr = step;
    }
  }

/* 中文逐项-函数 F0232：genericOp
 * 用途与位置：Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。
 * 参数 srcIx（intptr_t srcIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 dstIx（intptr_t dstIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteIx（intptr_t remoteIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 nelem（int nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_simple.h::send；src/collectives/device/prims_simple.h::sendFromOutput；src/collectives/device/prims_simple.h::directSend；src/collectives/device/prims_simple.h::directSendFromOutput；src/collectives/device/prims_simple.h::recv；src/collectives/device/prims_simple.h::directRecv；src/collectives/device/prims_simple.h::copySend；其余见调用清单。
 * 本体直接出现的调用/包装：ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, (1-Send)+MaxSend, 0>；ReduceOrCopyMulti<Unroll, RedOp, T,
 * 1, 1, 1, 1, 0>；ReduceOrCopyMulti<Unroll, RedOp, T, Recv+Src, Recv*MaxRecv+Src, Send+Dst, Send*MaxSend+Dst,
 * PreOpN>；__syncwarp；__threadfence_system；barrier；divUp；fan.nrecv；fan.nsend；max；postPeer<Recv,
 * Send>；subBarrier；waitPeer<DirectRecv, DirectSend, Recv, Send, Src, Dst>。
 * 需要对照的边界/推进条件：tid < nworkers && offset < nelem；Src && (flags & (SrcBuf==Input ? RoleInput : RoleOutput；Dst && (flags
 * & (DstBuf==Input ? RoleInput : RoleOutput；DirectRecv && ncclShmem.groups[group].srcs[0] ==
 * ncclShmem.groups[group].dsts[0]。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
  template <int DirectRecv1, int DirectSend1, int Recv, int Send, int SrcBuf, int DstBuf>
/* 中文导读：
 * Simple 协议的组合原语核心，模板参数选择收/发、本地输入/输出及 direct 访问。
 * 按 slice 等待、装配源/目标指针、执行融合复制/归约、同步并发布进度，避免为每个步骤另启 kernel。
 */
  __device__ __forceinline__ void genericOp(
      intptr_t srcIx, intptr_t dstIx, intptr_t remoteIx, int nelem, bool postOp
    ) {
/* 中文逐项-变量 V0662：DirectRecv
 * DirectRecv（int；函数 genericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 DirectRecv = 1 && Direct && DirectRecv1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    constexpr int DirectRecv = 1 && Direct && DirectRecv1;
/* 中文逐项-变量 V0663：DirectSend
 * DirectSend（int；函数 genericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 DirectSend = 1 && Direct && DirectSend1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    constexpr int DirectSend = 1 && Direct && DirectSend1;
/* 中文逐项-变量 V0664：Src
 * Src（int；函数 genericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 Src = SrcBuf != -1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    constexpr int Src = SrcBuf != -1;
/* 中文逐项-变量 V0665：Dst
 * Dst（int；函数 genericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 Dst = DstBuf != -1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    constexpr int Dst = DstBuf != -1;

/* 中文精读：尾块、空块与通信进度
 * 某 channel 的尾部 offset 可能已经超出 size，算法会传入非正 nelem；这里归一到 0。
 * 空数据仍须按固定 SlicePerChunk 推进同步和 step，不能简单 return 导致另一端永远等进度。
 * sliceSize 根据 nelem、片数与 step 容量调小，并保留最小搬运粒度，改善小尾块的延迟。
 * 它是有效元素数，乘 sizeof(T) 后才是 sizesFifo 中交给 Proxy 的字节数。
 */
    nelem = nelem < 0 ? 0 : nelem;
/* 中文逐项-变量 V0666：sliceSize
 * sliceSize（int；函数 genericOp 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 sliceSize = stepSize*StepPerSlice；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int sliceSize = stepSize*StepPerSlice;
    sliceSize = max(divUp(nelem, 16*SlicePerChunk)*16, sliceSize/32);
/* 中文逐项-变量 V0667：slice
 * slice（int；函数 genericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 slice = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int slice = 0;
/* 中文逐项-变量 V0668：offset
 * offset（int；函数 genericOp 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int offset = 0;

    if (tid < nworkers && offset < nelem) {
      // Worker-only loop for non-empty slices. Non-workers and empty slices are
      // processed in the loop following this if block. The benefit of splitting
      // the loop like this is we pull two branches out of the critical path.
      // Using "number of branch insns (taken or not) encountered dynamically"
      // as the performance metric, then:
      //   perf_orig = 2*numslices
      //   perf_new = 2+numslices
      // So the new code and old code behave the same for numslices=2, and for
      // numslices>2 the new code is superior. And note that in the case
      // numslices=1, the loop is trivially unrollable (single iteration) so we
      // don't incur that that tail branch and we still have perf_new=2.
      //
      // ORIGINAL CODE:
      //   unrolled for(slices) {
      //     if(worker) { // This branch removed
      //       wait();
      //       subBarrier();
      //       if(slice not empty) // This branch removed
      //         ReduceCopyMulti();
      //     }
      //     barrier();
      //     post();
      //   } // Since we no longer unroll, new branch added here
      #if __CUDA_ARCH__ < 700
        // Yeah, so all that above don't matter a lick on older hardware.
        #pragma unroll SlicePerChunk
      #else
        #pragma unroll 1
      #endif
      do {
        sliceSize = sliceSize < nelem-offset ? sliceSize : nelem-offset;
        if (Src && (flags & (SrcBuf==Input ? RoleInput : RoleOutput)))
          ncclShmem.groups[group].srcs[0] = userBuff + srcIx + offset;
        if (Dst && (flags & (DstBuf==Input ? RoleInput : RoleOutput)))
          ncclShmem.groups[group].dsts[0] = userBuff + dstIx + offset;
        waitPeer<DirectRecv, DirectSend, Recv, Send, Src, Dst>(dstIx, remoteIx, offset, sliceSize);
        subBarrier();
        if (DirectRecv && ncclShmem.groups[group].srcs[0] == ncclShmem.groups[group].dsts[0]) {
          // We can only have one direct receive. Since srcs[0] == dstPtr+offset, skip one copy
          if (Send) {
            // (1-Send) is only there to avoid compilation errors in case MaxSend=0 (and Send=0).
            ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, (1-Send)+MaxSend, 0>
              (tid, nworkers, nullptr, false,
               1, (T const**)ncclShmem.groups[group].srcs,
               fan.nsend(), (T**)ncclShmem.groups[group].dsts+1,
               sliceSize);
          }
        } else if (DirectSend && !DirectRecv && SrcBuf != Input && ncclShmem.groups[group].dsts[Dst] == nullptr) {
          // For broadcast in CollNet to do empty send
          ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, 0>
            (tid, nworkers, ncclShmem.redOpArgs, postOp,
             Recv, (T const**)ncclShmem.groups[group].srcs,
             Dst, (T**)ncclShmem.groups[group].dsts,
             sliceSize);
        } else {
/* 中文精读：多个原语怎样共享一个融合搬运内核
 * srcs 合并“本地输入（如果有）+ 接收邻居”，dsts 合并“本地输出（如果有）+ 发送邻居”。
 * ReduceOrCopyMulti 对同一批加载值执行归约并写入多个目标，避免先写临时结果再启动另一个 kernel。
 * PreOpN 限定哪些源需要输入预处理；postOp 只在全局结果完成处执行，例如整数平均最后除法。
 * 中间转发如果重复应用最终 postOp，会改变数学结果，因此算法层明确传 true/false。
 */
/* 中文逐项-变量 V0669：PreOpN
 * PreOpN（int；函数 genericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 PreOpN = SrcBuf != Input ? 0 : DirectRecv*MaxRecv == NCCL_MAX_DIRECT_ARITY ? (1+NCCL_MAX_DIRECT_ARITY) :
 * 1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          constexpr int PreOpN = SrcBuf != Input ? 0 :
                                 DirectRecv*MaxRecv == NCCL_MAX_DIRECT_ARITY ? (1+NCCL_MAX_DIRECT_ARITY) : 1;
          ReduceOrCopyMulti<Unroll, RedOp, T, Recv+Src, Recv*MaxRecv+Src, Send+Dst, Send*MaxSend+Dst, PreOpN>
            (tid, nworkers, ncclShmem.redOpArgs, postOp,
             Recv*fan.nrecv()+Src, (T const**)ncclShmem.groups[group].srcs,
             Send*fan.nsend()+Dst, (T**)ncclShmem.groups[group].dsts,
             sliceSize);
        }
        barrier(); // This barrier has a counterpart in following loop
        if (Send && (flags & RolePostSend) && index == 0) __threadfence_system();
        __syncwarp();
        postPeer<Recv, Send>();
        offset += sliceSize;
        slice += 1;
      } while (slice < SlicePerChunk && offset < nelem);
    }

/* 中文精读：同步线程与复制线程必须走匹配的屏障
 * worker 在上面的循环复制有效 slice，非 worker 直接进入这里参与同一组 barrier/postPeer。
 * worker 遇到剩余空 slice 后也进入这里，确保所有参与线程的屏障次数一致。
 * Send 发布 tail 之前先做系统级 fence，使数据写入对 GPU/CPU/NIC 协作路径可见。
 * volatile 让进度被反复读取，barrier 协调本组线程，fence 约束可见性；三者职责不同。
 */
    // Non-workers come straight here. Workers too but only once the remaining
    // slices are all empty. Since empty slices are the uncommon case, and
    // worker perf is the limiter, perf-wise this loop is effectively unentered,
    // hence just a single branch insn.
    #pragma unroll 1
    while (slice < SlicePerChunk) {
      sliceSize = sliceSize < nelem-offset ? sliceSize : nelem-offset;
      { // Only workers could have Wait roles so we know the slice must be empty
        // since we've exited the loop above.
        waitPeer<DirectRecv, DirectSend, Recv, Send, Src, Dst>(0, 0, 0, 0);
      }
      barrier(); // Has couterpart in preceding worker-only loop.
      if (Send && (flags & RolePostSend) && sliceSize > 0 && index == 0) __threadfence_system();
      __syncwarp();
      postPeer<Recv, Send>();
      offset += sliceSize;
      slice += 1;
    }
  }

  // Scatter/Gather generic op
  // skip: my own rank order in the buffer chunks
  // shift: peer offset to avoid all ranks sending to or receiving from same peer
/* 中文逐项-函数 F0233：ScatterGatherOp
 * 用途与位置：按 peer 分配或收集不同数据段，支持 CollNet 多 head 的分散/汇集以及直接缓冲访问优化。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 totalElem（int totalElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 peerElem（int peerElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 skip（int skip）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 shift（int shift）：在多 head/peer 间轮转分片次序的偏移，平衡并发访问。
 * 参数 postOp（bool postOp）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_simple.h::scatter；src/collectives/device/prims_simple.h::directScatter；src/collectives/device/prims_simple.h::gather；src/collectives/device/prims_simple.h::directGather。
 * 本体直接出现的调用/包装：DIVUP；ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, 0>；ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1,
 * 1, PreOpN>；__syncwarp；__threadfence_system；barrier；fan.nrecv；fan.nsend；max；min；postPeer<Recv,
 * Send>；subBarrier；waitPeer<0, DirectSend, 0, 1, 1, 0>；waitPeer<DirectRecv, 0, 1, 0, 0, 1>。
 * 需要对照的边界/推进条件：slice<SlicePerChunk；tid < nworkers；Send；flags & RoleInput。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
  template <int DirectRecv1, int DirectSend1, int Recv, int Send>
  __device__ __forceinline__ void
  ScatterGatherOp(intptr_t inpIx, intptr_t outIx, int totalElem, int peerElem, int skip, int shift, bool postOp) {
/* 中文逐项-变量 V0670：DirectRecv
 * DirectRecv（int；函数 ScatterGatherOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 DirectRecv = 1 && Direct && DirectRecv1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    constexpr int DirectRecv = 1 && Direct && DirectRecv1;
/* 中文逐项-变量 V0671：DirectSend
 * DirectSend（int；函数 ScatterGatherOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 DirectSend = 1 && Direct && DirectSend1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    constexpr int DirectSend = 1 && Direct && DirectSend1;
/* 中文逐项-变量 V0672：offset
 * offset（int；函数 ScatterGatherOp 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int offset = 0; // slice offset
/* 中文逐项-变量 V0673：sliceSize
 * sliceSize（int；函数 ScatterGatherOp 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 sliceSize = stepSize*StepPerSlice；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int sliceSize = stepSize*StepPerSlice;
/* 中文逐项-变量 V0674：dataSize
 * dataSize（int；函数 ScatterGatherOp 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 dataSize = max(DIVUP(peerElem, 16*SlicePerChunk)*16, sliceSize/32)；保留下限或较大候选，避免并行度/尺寸低于需要的最小值。
 */
    int dataSize = max(DIVUP(peerElem, 16*SlicePerChunk)*16, sliceSize/32);  // per-peer slice size

    #pragma unroll
    for (/* 中文逐项-变量 V0675：slice
 * slice（int；函数 ScatterGatherOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 slice = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：slice<SlicePerChunk；循环体与增量表达式决定每轮如何推进。
 */int slice=0; slice<SlicePerChunk; ++slice) {
/* 中文逐项-变量 V0676：realSize
 * realSize（int；函数 ScatterGatherOp 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 realSize = max(0, min(dataSize, peerElem-offset))；保留下限或较大候选，避免并行度/尺寸低于需要的最小值。
 */
      int realSize = max(0, min(dataSize, peerElem-offset));
      if (tid < nworkers) {
        if (Send) {
          // Scatter pre-scales data of input buffer only in non-Direct case
/* 中文逐项-变量 V0677：PreOpN
 * PreOpN（int；函数 ScatterGatherOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 PreOpN = DirectSend ? 0 : 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          constexpr int PreOpN = DirectSend ? 0 : 1;
          if (flags & RoleInput) ncclShmem.groups[group].srcs[0] = userBuff + inpIx + offset;
          if (tid == 0) ncclShmem.groups[group].totalSendSize[slice] = 0; // Skip the threadfence
          // realSize is not accurate here; but intra-node does not rely on sizes FIFO
          waitPeer<0, DirectSend, 0, 1, 1, 0>(0, inpIx, offset, realSize);
          subBarrier();
          #pragma unroll
          // Loop over peers
          for (/* 中文逐项-变量 V0678：j
 * j（int；函数 ScatterGatherOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：j<fan.nsend()；循环体与增量表达式决定每轮如何推进。
 */int j=0; j<fan.nsend(); j++) {
/* 中文逐项-变量 V0679：i
 * i（int；函数 ScatterGatherOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = (j+shift)%fan.nsend()；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            int i = (j+shift)%fan.nsend();
/* 中文逐项-变量 V0680：peerOffset
 * peerOffset（int；函数 ScatterGatherOp 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 peerOffset = i*peerElem；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
            int peerOffset = i*peerElem;
            // Skip the data I am responsible of reducing myself
            if (skip >= 0 && i >= skip) peerOffset += peerElem;
/* 中文逐项-变量 V0681：src0
 * src0（T；函数 ScatterGatherOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 src0 = (T*)ncclShmem.groups[group].srcs[0] + peerOffset；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            const T* src0 = (T*)ncclShmem.groups[group].srcs[0] + peerOffset;
/* 中文逐项-变量 V0682：realPeerSize
 * realPeerSize（int；函数 ScatterGatherOp 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 realPeerSize = min(realSize, totalElem-peerOffset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
            int realPeerSize = min(realSize, totalElem-peerOffset);
            if (realPeerSize > 0 && ncclShmem.groups[group].dsts[i] != nullptr) {
              ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, PreOpN>(tid, nworkers, ncclShmem.redOpArgs, false, 1, &src0, 1, (T**)ncclShmem.groups[group].dsts+i, realPeerSize);
              // Mark for threadfence at the end
              if (tid == 0) ncclShmem.groups[group].totalSendSize[slice] += realPeerSize;
            }
          }
        } else if (Recv) {
          if (flags & RoleOutput) ncclShmem.groups[group].dsts[0] = userBuff + outIx + offset;
/* 中文逐项-变量 V0683：peerOffset
 * peerOffset（int；函数 ScatterGatherOp 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 peerOffset = index*peerElem；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          int peerOffset = index*peerElem;
          if (skip >= 0 && index >= skip) peerOffset += peerElem;
          // Adjust remote index with peer offset in case we are directly pulling from peer's output buffer
          waitPeer<DirectRecv, 0, 1, 0, 0, 1>(outIx, outIx+peerOffset, offset, realSize);
          subBarrier();
          if (DirectRecv && ncclShmem.groups[group].srcs[0] == ncclShmem.groups[group].dsts[0]) {
            // Since waitPeer sets srcs[0] to output buffer + offset, we are doing a direct-write based recv
            // Do nothing
          } else {
            #pragma unroll
            for (/* 中文逐项-变量 V0684：j
 * j（int；函数 ScatterGatherOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：j<fan.nrecv()；循环体与增量表达式决定每轮如何推进。
 */int j=0; j<fan.nrecv(); j++) {
/* 中文逐项-变量 V0685：i
 * i（int；函数 ScatterGatherOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = (j+shift)%fan.nrecv()；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              int i = (j+shift)%fan.nrecv();
              peerOffset = i*peerElem;
              if (skip >= 0 && i >= skip) peerOffset += peerElem;
/* 中文逐项-变量 V0686：dst0
 * dst0（T；函数 ScatterGatherOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 dst0 = (T*)ncclShmem.groups[group].dsts[0] + peerOffset；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              T* dst0 = (T*)ncclShmem.groups[group].dsts[0] + peerOffset;
/* 中文逐项-变量 V0687：realPeerSize
 * realPeerSize（int；函数 ScatterGatherOp 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 realPeerSize = min(realSize, totalElem-peerOffset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
              int realPeerSize = min(realSize, totalElem-peerOffset);
              if (realPeerSize > 0) ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, 0>(tid, nworkers, ncclShmem.redOpArgs, postOp, 1, (const T**)ncclShmem.groups[group].srcs+i, 1, &dst0, realPeerSize);
            }
          }
        }
      }
      barrier();
      // If we indeed send something, threadfence
      if (Send && (flags & RolePostSend) && ncclShmem.groups[group].totalSendSize[slice] > 0 && index == 0)
        __threadfence_system();
      __syncwarp();
      postPeer<Recv, Send>();
      offset += realSize;
    }
  }

/* 中文精读：把 connector 状态装入原语的线程角色
 * 只有承担 WaitRecv/PostRecv 的特定线程加载对应连接和进度指针，其他线程负责搬运计算。
 * 根据 direct/shared 能力选择指针交换、固定 FIFO 或 offsFifo 动态偏移。
 * 接收一侧读 tail 等生产，写 head 释放消费；发送一侧读 head 等空间，写 tail 发布生产。
 * 这里的 head/tail 是连接视图，具体位于哪一侧 CPU/GPU 内存由 transport connect 填好。
 */
  __device__ __forceinline__ /* 中文逐项-函数 F0234：loadRecvConn
 * 用途与位置：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。
 * 参数 peer（ncclPeer *peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 参数 e（struct ncclWorkElem* e）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体直接出现的调用/包装：roundUp。
 * 需要对照的边界/推进条件：flags & (RoleWaitRecv|RolePostRecv；flags & RolePostRecv；flags & RoleWaitRecv；Direct。各条件所属分支见紧邻函数体。
 */void loadRecvConn(ncclPeer *peer, int connIndex, struct ncclWorkElem* e) {
    if (flags & (RoleWaitRecv|RolePostRecv)) {
/* 中文逐项-变量 V0688：conn
 * conn（auto；函数 loadRecvConn 内）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 初值/来源 conn = &peer->recv[connIndex].conn；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
      auto *conn = &peer->recv[connIndex].conn;
      step = conn->step;
      step = roundUp(step, SlicePerChunk*StepPerSlice);
      if (flags & RolePostRecv) {
        connStepPtr = conn->head;
        *connStepPtr = step; // Return credits in case we rounded up.
      }
      if (flags & RoleWaitRecv) {
        ncclShmem.groups[group].recvConns[index] = conn; // WaitRecv role saves since that's who needs it in setDataPtrs()
        connStepPtr = conn->tail;
        connStepCache = *connStepPtr;
        flags |= (conn->offsFifo != nullptr) ? OffsFifoEnabled : 0;
        if (Direct) {
          // User buffers have been registered
          if ((conn->direct & (NCCL_IPC_READ|NCCL_IPC_WRITE)) && e != nullptr && e->regUsed) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              flags |= (e->direct & NCCL_DIRECT_WRITE) ? DirectWrite :
                       (e->direct & NCCL_DIRECT_READ)  ? DirectRead  : 0;
            }
          } else if (conn->direct & (NCCL_DIRECT_WRITE|NCCL_DIRECT_READ)) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              // direct read not allowed in non-register case
              // otherwise, in one-to-multi send, we could mix empty send and intermediate send
              flags |= (conn->direct & NCCL_DIRECT_WRITE) ? DirectWrite : 0;
            }
          }
        }
        if (flags & OffsFifoEnabled)
          connOffsFifoPtr = conn->offsFifo;
        connEltsFifo = (T*)conn->buffs[NCCL_PROTO_SIMPLE];
      }
    }
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0235：loadSendConn
 * 用途与位置：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。
 * 参数 peer（ncclPeer *peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 参数 e（struct ncclWorkElem* e）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体直接出现的调用/包装：roundUp。
 * 需要对照的边界/推进条件：flags & (RoleWaitSend|RolePostSend；flags & RolePostSend；flags & RoleWaitSend；flags &
 * OffsFifoEnabled。各条件所属分支见紧邻函数体。
 */void loadSendConn(ncclPeer *peer, int connIndex, struct ncclWorkElem* e) {
    if (flags & (RoleWaitSend|RolePostSend)) {
/* 中文逐项-变量 V0689：conn
 * conn（auto；函数 loadSendConn 内）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 初值/来源 conn = &peer->send[connIndex].conn；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
      auto *conn = &peer->send[connIndex].conn;
      step = conn->step;
      step = roundUp(step, SlicePerChunk*StepPerSlice);
      if (flags & RolePostSend) {
        connStepPtr = conn->tail;
      }
      if (flags & RoleWaitSend) {
        ncclShmem.groups[group].sendConns[index] = conn; // WaitSend role saves since that's who needs it in setDataPtrs()
        connStepPtr = conn->head;
        connStepCache = *connStepPtr;
        flags |= (conn->offsFifo != nullptr) ? OffsFifoEnabled : 0;
        if (flags & OffsFifoEnabled)
          connOffsFifoPtr = conn->offsFifo;
        connEltsFifo = (T*)conn->buffs[NCCL_PROTO_SIMPLE];

        if (conn->sizesFifo != nullptr) {
          flags |= SizesFifoEnabled;
          connSizesFifoPtr = conn->sizesFifo;
        } else if (Direct) {
          // User buffers have been registered
          if ((conn->direct & (NCCL_IPC_READ|NCCL_IPC_WRITE)) && e != nullptr && e->regUsed) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              flags |= (e->direct & NCCL_DIRECT_WRITE) ? DirectWrite :
                       (e->direct & NCCL_DIRECT_READ)  ? DirectRead  : 0;
            }
          } else if (conn->direct & (NCCL_DIRECT_WRITE|NCCL_DIRECT_READ)) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              // direct read not allowed in non-register case
              // otherwise, in one-to-multi send, we could mix empty send and intermediate send
              flags |= (conn->direct & NCCL_DIRECT_WRITE) ? DirectWrite : 0;
            }
          }
        }
      }
    }
  }

 public:
/* 中文精读：构造原语就是建立本次线程组的执行上下文
 * tid/nthreads 是本原语线程组内编号，不一定等于整个 block 的线程数。
 * 扫描有效 recvPeers/sendPeers 决定 fan；把部分线程指定为 Wait/Post/Input/Output 角色。
 * group 的低位区分共享内存/命名 barrier 组，高位携带 connIndex，P2P 使用索引 1。
 * 最后装载连接并交换 direct 指针；匹配双方必须执行相容构造路径，否则可能在交换阶段等待。
 */
  __device__ /* 中文逐项-函数 F0236：Primitives
 * 用途与位置：为一个线程组建立原语上下文：实际邻居度、协议缓冲、线程角色、同步编号及用户数据指针。
 * 参数 tid（int tid）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 参数 nthreads（int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 参数 recvPeers（int const *recvPeers）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 sendPeers（int const *sendPeers）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 inputBuf（void const *inputBuf）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 outputBuf（void *outputBuf）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 redOpArg（uint64_t redOpArg）：归约标量的位模式容器或设备指针，需结合对应 IsPtr 标志理解。
 * 参数 group（uint32_t group=0）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 参数 e（struct ncclWorkElem* e = nullptr）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：Fan；loadRecvConn；loadSendConn；setDataPtrs。
 * 需要对照的边界/推进条件：nrecv < MaxRecv && recvPeers[nrecv] != -1；nsend < MaxSend && sendPeers[nsend] != -1；g == 0；index <
 * nrecv。各条件所属分支见紧邻函数体。
 */Primitives(
      int tid, int nthreads, int const *recvPeers, int const *sendPeers,
      void const *inputBuf, void *outputBuf, uint64_t redOpArg, uint32_t group=0, struct ncclWorkElem* e = nullptr
    ):
    tid(tid),
    stepSize(ncclShmem.comm.buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS/sizeof(T)) {

    // For send operations, we need an extra warp to overlap the threadfence and the copy
    this->nthreads = nthreads;
    this->nworkers = nthreads - (MaxSend > 0 && nthreads-WARP_SIZE >= 64 ? WARP_SIZE : 0);
    this->group = group & (uint16_t)0xFFFF;
/* 中文逐项-变量 V0690：connIndex
 * connIndex（int；函数 Primitives 内）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 初值/来源 connIndex = group >> 16；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
    int connIndex = group >> 16;

/* 中文逐项-变量 V0691：nrecv、nsend
 * nrecv（int；函数 Primitives 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nrecv = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * nsend（int；函数 Primitives 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nsend = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int nrecv=0, nsend=0;
    while (nrecv < MaxRecv && recvPeers[nrecv] != -1) nrecv++;
    while (nsend < MaxSend && sendPeers[nsend] != -1) nsend++;
    this->fan = Fan(nrecv, nsend);

/* 中文逐项-变量 V0692：ThreadPerSync
 * ThreadPerSync（int；函数 Primitives 内）：同步角色分组的线程粒度，限制可容纳的收发邻居数。
 * 初值/来源 ThreadPerSync = 8；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    constexpr int ThreadPerSync = 8;
    static_assert(MaxSend < ThreadPerSync && MaxRecv < ThreadPerSync, "Not enough threads to cover all peers");

/* 中文逐项-变量 V0693：g
 * g（int；函数 Primitives 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = tid / ThreadPerSync；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int g = tid / ThreadPerSync;
/* 中文逐项-变量 V0694：ng
 * ng（int；函数 Primitives 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ng = nthreads / ThreadPerSync；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int ng = nthreads / ThreadPerSync;
    index = tid % ThreadPerSync;
    flags = 0;
    if (g == 0) {
      if (index < nrecv) flags |= RoleWaitRecv;
      if (index == nrecv) flags |= RoleInput;
    } else if (g == 1) {
      if (index < nsend) flags |= RoleWaitSend;
      if (index == nsend) flags |= RoleOutput;
    } else if (g == ng - 2) {
      if (index < nrecv) flags |= RolePostRecv;
    } else if (g == ng - 1) {
      if (index < nsend) flags |= RolePostSend;
    }

/* 中文逐项-变量 V0695：peer
 * peer（int；函数 Primitives 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int peer = 0;
    if (flags & (RoleWaitRecv|RolePostRecv)) peer = recvPeers[index];
    if (flags & (RoleWaitSend|RolePostSend)) peer = sendPeers[index];

    loadRecvConn(&ncclShmem.channel.devPeers[peer], connIndex, e);
    loadSendConn(&ncclShmem.channel.devPeers[peer], connIndex, e);

    setDataPtrs(inputBuf, outputBuf, redOpArg, (struct ncclWorkElemReg*)e);
  }

/* 中文精读：step 必须跨原语对象和操作保留
 * 原语内部推进的 step 先保存在各个角色线程的局部状态，析构时写回连接 conn->step。
 * 下一个操作继续同一进度，避免旧 flag/旧槽位内容被误认为本次新数据。
 * 析构还要保证线程同步满足协议要求，不仅仅是 C++ 对象释放内存。
 * 这也是不能把一次原语调用解释为“重新创建一条连接”的原因。
 */
  __device__ /* 中文逐项-函数 F0237：~Primitives
 * 用途与位置：把本次推进后的 step 写回长期连接并按协议要求同步，后续操作继续复用同一连接。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：barrier。
 * 需要对照的边界/推进条件：!(flags & ThreadsSynced；flags & (RolePostSend|RolePostRecv。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */~Primitives() {
    // Ensure ncclShmem.groups[].send/recvConns are available
    if (!(flags & ThreadsSynced))
      barrier();
    // Save steps for the next operation
    if (flags & (RolePostSend|RolePostRecv)) {
/* 中文逐项-变量 V0696：conns
 * conns（auto；函数 ~Primitives 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 conns = (flags & RolePostSend) ? ncclShmem.groups[group].sendConns :
 * ncclShmem.groups[group].recvConns；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      auto *conns = (flags & RolePostSend) ? ncclShmem.groups[group].sendConns : ncclShmem.groups[group].recvConns;
      conns[index]->step = step;
    }
    // Make sure all threads are done writing back conn->step and done using
    // ncclShmem.groups[group]
    barrier();
  }

  __device__ /* 中文逐项-函数 F0238：setDataPtrs
 * 用途与位置：更新本原语的本地输入和输出缓冲指针；Simple 的 direct 路径还需完成对应地址交换。
 * 参数 inputBuf（void const *inputBuf）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 outputBuf（void *outputBuf）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 redOpArg（uint64_t redOpArg）：归约标量的位模式容器或设备指针，需结合对应 IsPtr 标志理解。
 * 参数 e（struct ncclWorkElemReg* e）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体直接出现的调用/包装：checkAbort；reinterpret_cast<T*>；reinterpret_cast<uintptr_t>；uint64_t。
 * 需要对照的边界/推进条件：flags & RoleInput；flags & RoleOutput；Direct && recvProvider；*slot != nullptr &&
 * !checkAbort(spins。各条件所属分支见紧邻函数体。
 */void setDataPtrs(void const *inputBuf, void *outputBuf, uint64_t redOpArg, struct ncclWorkElemReg* e) {
    if (flags & RoleInput) {
      userBuff = (T*)inputBuf;
      ncclShmem.redOpArgs[0] = redOpArg;  // scaler for local input
    }
    if (flags & RoleOutput) userBuff = (T*)outputBuf;
/* 中文逐项-变量 V0697：recvProvider
 * recvProvider（bool；函数 setDataPtrs 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 recvProvider = flags == (flags|RoleWaitRecv|DirectWrite)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    bool recvProvider = flags == (flags|RoleWaitRecv|DirectWrite);
/* 中文逐项-变量 V0698：sendAcceptor
 * sendAcceptor（bool；函数 setDataPtrs 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 sendAcceptor = flags == (flags|RoleWaitSend|DirectWrite)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    bool sendAcceptor = flags == (flags|RoleWaitSend|DirectWrite);
/* 中文逐项-变量 V0699：sendProvider
 * sendProvider（bool；函数 setDataPtrs 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 sendProvider = flags == (flags|RoleWaitSend|DirectRead)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    bool sendProvider = flags == (flags|RoleWaitSend|DirectRead); // sender provides direct buffer (to be fetched)
/* 中文逐项-变量 V0700：recvAcceptor
 * recvAcceptor（bool；函数 setDataPtrs 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 recvAcceptor = flags == (flags|RoleWaitRecv|DirectRead)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    bool recvAcceptor = flags == (flags|RoleWaitRecv|DirectRead); // receiver accepts direct buffer
/* 中文逐项-变量 V0701：regUsed
 * regUsed（int；函数 setDataPtrs 内）：是否采用已注册用户缓冲优化，决定使用普通协议缓冲还是直接地址路径。
 * 初值/来源 regUsed = e != nullptr ? e->elem.regUsed : 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int regUsed = e != nullptr ? e->elem.regUsed : 0;

    if (Direct && recvProvider) {
/* 中文逐项-变量 V0702：spins
 * spins（int；函数 setDataPtrs 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int spins = 0;
/* 中文逐项-变量 V0703：slot
 * slot（void；函数 setDataPtrs 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 slot = ncclShmem.groups[group].recvConns[index]->ptrExchange；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void *volatile *slot = ncclShmem.groups[group].recvConns[index]->ptrExchange;
      // Wait for consumer to consume previous value before trampling it.
      while (*slot != nullptr && !checkAbort(spins));
      directBuff = (T*)outputBuf;
      // Encode pointer by XOR'ing against some address they definitely wouldn't send
      // since we want to allow them sending us nullptr while not colliding with
      // the empty slot value.
      *slot = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(directBuff) ^ reinterpret_cast<uintptr_t>(slot));
    }
    if (Direct && sendAcceptor) {
/* 中文逐项-变量 V0704：spins
 * spins（int；函数 setDataPtrs 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int spins = 0;
/* 中文逐项-变量 V0705：slot
 * slot（void；函数 setDataPtrs 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 slot = ncclShmem.groups[group].sendConns[index]->ptrExchange；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void *volatile *slot = ncclShmem.groups[group].sendConns[index]->ptrExchange;
/* 中文逐项-变量 V0706：ptr
 * ptr（void；函数 setDataPtrs 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 此处未显式给初值；后续首个文本使用点：ptr = *slot;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      void *ptr;
      while (true) {
        ptr = *slot;
        if (ptr != nullptr || checkAbort(spins)) break;
      }
      directBuff = regUsed ? (T*)(e->dnOutputs[index]) :
                   reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) ^ reinterpret_cast<uintptr_t>(slot));
      *slot = nullptr;
    }
    if (Direct && sendProvider) {
/* 中文逐项-变量 V0707：spins
 * spins（int；函数 setDataPtrs 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int spins = 0;
/* 中文逐项-变量 V0708：slot
 * slot（void；函数 setDataPtrs 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 slot = ncclShmem.groups[group].sendConns[index]->ptrExchange；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void *volatile *slot = ncclShmem.groups[group].sendConns[index]->ptrExchange;
/* 中文逐项-变量 V0709：argSlot0
 * argSlot0（uint64_t；函数 setDataPtrs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 argSlot0 = ncclShmem.groups[group].sendConns[index]->redOpArgExchange；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      volatile uint64_t* argSlot0 = ncclShmem.groups[group].sendConns[index]->redOpArgExchange;
/* 中文逐项-变量 V0710：argSlot1
 * argSlot1（uint64_t；函数 setDataPtrs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 argSlot1 = ncclShmem.groups[group].sendConns[index]->redOpArgExchange+1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      volatile uint64_t* argSlot1 = ncclShmem.groups[group].sendConns[index]->redOpArgExchange+1;
      // Wait for consumer to consume previous value before trampling it.
      while ((*slot != nullptr || *argSlot0 != 0 || *argSlot1 !=0) && !checkAbort(spins));
      // If there is no recv, then we are directly pulling from input buffer (e.g. directScatter)
      // Otherwise, we are pulling from output buffer (e.g. recvCopyDirectSend)
      directBuff = MaxRecv == 0 ? (T*)inputBuf : (T*)outputBuf;
      // Exchange pre-scalers for use in direct pull
      *argSlot0 = (uint64_t(1)<<32) | (uint32_t)redOpArg;
      *argSlot1 = (uint64_t(1)<<32) | (uint32_t)(redOpArg>>32);
      // Encode pointer by XOR'ing against some address they definitely wouldn't send
      // since we want to allow them sending us nullptr while not colliding with
      // the empty slot value.
      *slot = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(directBuff) ^ reinterpret_cast<uintptr_t>(slot));
    }
    if (Direct && recvAcceptor) {
/* 中文逐项-变量 V0711：spins
 * spins（int；函数 setDataPtrs 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int spins = 0;
/* 中文逐项-变量 V0712：slot
 * slot（void；函数 setDataPtrs 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 slot = ncclShmem.groups[group].recvConns[index]->ptrExchange；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void *volatile *slot = ncclShmem.groups[group].recvConns[index]->ptrExchange;
/* 中文逐项-变量 V0713：argSlot0
 * argSlot0（uint64_t；函数 setDataPtrs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 argSlot0 = ncclShmem.groups[group].recvConns[index]->redOpArgExchange；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      volatile uint64_t* argSlot0 = ncclShmem.groups[group].recvConns[index]->redOpArgExchange;
/* 中文逐项-变量 V0714：argSlot1
 * argSlot1（uint64_t；函数 setDataPtrs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 argSlot1 = ncclShmem.groups[group].recvConns[index]->redOpArgExchange+1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      volatile uint64_t* argSlot1 = ncclShmem.groups[group].recvConns[index]->redOpArgExchange+1;
/* 中文逐项-变量 V0715：ptr
 * ptr（void；函数 setDataPtrs 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 此处未显式给初值；后续首个文本使用点：ptr = *slot;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      void *ptr;
      while (true) {
        ptr = *slot;
        if (ptr != nullptr || checkAbort(spins)) break;
      }
      directBuff = regUsed ? (T*)(MaxSend == 0 ? e->upOutputs[index] : e->dnInputs[index]) :
                   reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) ^ reinterpret_cast<uintptr_t>(slot));
      if (MaxSend != 0) { // reduce group rather than gather group
        // Store scalers for remote inputs
/* 中文逐项-变量 V0716：arg0、arg1
 * arg0（uint64_t；函数 setDataPtrs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：arg0 = *argSlot0;。若由输出参数填入，必须先检查填充调用成功再读值。
 * arg1（uint64_t；函数 setDataPtrs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：arg1 = *argSlot1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        uint64_t arg0, arg1;
        while (true) {
          arg0 = *argSlot0;
          arg1 = *argSlot1;
          if ((arg0 != 0 && arg1 != 0) || checkAbort(spins)) break;
        }
        ncclShmem.redOpArgs[1+index] = ((arg1 & 0xffffffff)<<32) | (arg0 & 0xffffffff);
      }
      *argSlot0 = 0; *argSlot1 = 0;
      *slot = nullptr;
    }
  }

  __device__ /* 中文逐项-函数 F0239：moveDataPtrs
 * 用途与位置：按元素偏移移动本原语的用户输入/输出指针，复用连接与协议状态处理另一段数据。
 * 参数 delta（intptr_t delta）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：flags & (RoleInput|RoleOutput。各条件所属分支见紧邻函数体。
 */void moveDataPtrs(intptr_t delta) {
    if (flags & (RoleInput|RoleOutput))
      userBuff += delta;
  }

/* 中文精读：从原语名字反推源与目的
 * send: 本地 input -> 下游；recv: 上游 -> 本地 output；copySend: input -> output 和下游。
 * recvReduceSend: 上游 + 本地 input -> 下游；recvReduceCopy: 上游 + input -> output。
 * recvCopySend: 上游 -> output 和下游；recvReduceCopySend: 上游 + input -> output 和下游。
 * 下面每个包装仅选择 genericOp 的模板位，不会每次单独发射 CUDA kernel。
 * Input/Output 下标都是 T 元素偏移；remoteOutIx 只在 direct 访问远端用户缓冲时需要。
 */
  __device__ __forceinline__ /* 中文逐项-函数 F0240：send
 * 用途与位置：从本地输入读取并发送到原语的发送邻居，不要求写本地输出。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketProgressOpt。
 * 本体直接出现的调用/包装：genericOp<0, 0, 0, 1, Input, -1>。
 */void send(intptr_t inpIx, int eltN) {
    genericOp<0, 0, 0, 1, Input, -1>(inpIx, -1, -1, eltN, false);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0241：sendFromOutput
 * 用途与位置：从已生成的本地输出读数据并发送，避免错误地重新读取原始输入。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 0, 0, 1, Output, -1>。
 */void sendFromOutput(intptr_t outIx, int eltN) {
    genericOp<0, 0, 0, 1, Output, -1>(outIx, -1, -1, eltN, false);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0242：directSend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 1, 0, 1, Input, -1>。
 */void directSend(intptr_t inpIx, intptr_t remoteOutIx, int eltN) {
    genericOp<0, 1, 0, 1, Input, -1>(inpIx, -1, remoteOutIx, eltN, false);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0243：directSendFromOutput
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 1, 0, 1, Output, -1>。
 */void directSendFromOutput(intptr_t outIx, intptr_t remoteOutIx, int eltN) {
    genericOp<0, 1, 0, 1, Output, -1>(outIx, -1, remoteOutIx, eltN, false);
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0244：recv
 * 用途与位置：接收上游数据并写本地输出，本动作不加上本地输入。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketProgressOpt。
 * 本体直接出现的调用/包装：genericOp<0, 0, 1, 0, -1, Output>。
 */void recv(intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 0, -1, Output>(-1, outIx, -1, eltN, postOp);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0245：directRecv
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<1, 0, 1, 0, -1, Output>。
 */void directRecv(intptr_t outIx, int eltN) {
    genericOp<1, 0, 1, 0, -1, Output>(-1, outIx, -1, eltN, /*postOp=*/false);
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0246：copySend
 * 用途与位置：把本地输入同时写本地输出并发下游，把本地保存和转发合并一次数据加载。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 0, 0, 1, Input, Output>。
 */void copySend(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 0, 1, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0247：directCopySend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 1, 0, 1, Input, Output>。
 */void directCopySend(intptr_t inpIx, intptr_t outIx, intptr_t remoteOutIx, int eltN, bool postOp=false) {
    genericOp<0, 1, 0, 1, Input, Output>(inpIx, outIx, remoteOutIx, eltN, postOp);
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0248：recvCopySend
 * 用途与位置：把已接收结果同时保存本地输出并转发下游，不再加入本地输入做归约。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 0, 1, 1, -1, Output>。
 */void recvCopySend(intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 1, -1, Output>(-1, outIx, -1, eltN, postOp);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0249：directRecvCopySend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<1, 1, 1, 1, -1, Output>。
 */void directRecvCopySend(intptr_t outIx, intptr_t remoteOutIx, int eltN) {
    genericOp<1, 1, 1, 1, -1, Output>(-1, outIx, remoteOutIx, eltN, false);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0250：recvCopyDirectSend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 1, 1, 1, -1, Output>。
 */void recvCopyDirectSend(intptr_t outIx, intptr_t remoteOutIx, int eltN, bool postOp=false) {
    genericOp<0, 1, 1, 1, -1, Output>(-1, outIx, remoteOutIx, eltN, postOp);
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0251：recvReduceCopy
 * 用途与位置：把接收数据与本地输入归约后写本地输出，可在结果完成时执行 postOp。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 0, 1, 0, Input, Output>。
 */void recvReduceCopy(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 0, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0252：recvReduceSend
 * 用途与位置：把上游部分结果与本地输入归约后发下游，不要求保留本地最终输出。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 0, 1, 1, Input, -1>。
 */void recvReduceSend(intptr_t inpIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 1, Input, -1>(inpIx, -1, -1, eltN, postOp);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0253：directRecvReduceSend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteInpIx（intptr_t remoteInpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<1, 0, 1, 1, Input, -1>。
 */void directRecvReduceSend(intptr_t inpIx, intptr_t remoteInpIx, int eltN, bool postOp=false) {
    genericOp<1, 0, 1, 1, Input, -1>(inpIx, -1, remoteInpIx, eltN, postOp);
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0254：recvReduceCopySend
 * 用途与位置：把接收与本地输入融合归约，同时保存输出并转发，衔接归约与结果传播阶段。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 0, 1, 1, Input, Output>。
 */void recvReduceCopySend(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    genericOp<0, 0, 1, 1, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0255：directRecvReduceCopySend
 * 用途与位置：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 remoteOutIx（intptr_t remoteOutIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：genericOp<0, 1, 1, 1, Input, Output>。
 */void directRecvReduceCopySend(intptr_t inpIx, intptr_t outIx, intptr_t remoteOutIx, int eltN, bool postOp=false) {
    // Direct is only for the send part
    genericOp<0, 1, 1, 1, Input, Output>(inpIx, outIx, remoteOutIx, eltN, postOp);
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0256：scatter
 * 用途与位置：把当前输入范围按每 peer 的片段尺寸分散给多个上游 head，直接模式利用已交换的用户缓冲地址。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 totalElem（int totalElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 peerElem（int peerElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 skip（int skip）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 shift（int shift）：在多 head/peer 间轮转分片次序的偏移，平衡并发访问。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ScatterGatherOp<0, 0, 0, 1>。
 */void
  scatter(intptr_t inpIx, int totalElem, int peerElem, int skip, int shift) {
    ScatterGatherOp<0, 0, 0, 1>(inpIx, -1, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0257：directScatter
 * 用途与位置：把当前输入范围按每 peer 的片段尺寸分散给多个上游 head，直接模式利用已交换的用户缓冲地址。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 totalElem（int totalElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 peerElem（int peerElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 skip（int skip）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 shift（int shift）：在多 head/peer 间轮转分片次序的偏移，平衡并发访问。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ScatterGatherOp<0, 1, 0, 1>。
 */void
  directScatter(intptr_t inpIx, int totalElem, int peerElem, int skip, int shift) {
    ScatterGatherOp<0, 1, 0, 1>(inpIx, -1, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0258：gather
 * 用途与位置：从多个 head 收集各自负责的结果片段到本地输出，保持整体张量布局。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 totalElem（int totalElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 peerElem（int peerElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 skip（int skip）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 shift（int shift）：在多 head/peer 间轮转分片次序的偏移，平衡并发访问。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ScatterGatherOp<0, 0, 1, 0>。
 */void
  gather(intptr_t outIx, int totalElem, int peerElem, int skip, int shift, bool postOp=false) {
    ScatterGatherOp<0, 0, 1, 0>(-1, outIx, totalElem, peerElem, skip, shift, postOp);
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0259：directGather
 * 用途与位置：从多个 head 收集各自负责的结果片段到本地输出，保持整体张量布局。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 totalElem（int totalElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 peerElem（int peerElem）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 skip（int skip）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 shift（int shift）：在多 head/peer 间轮转分片次序的偏移，平衡并发访问。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ScatterGatherOp<1, 0, 1, 0>。
 */void
  directGather(intptr_t outIx, int totalElem, int peerElem, int skip, int shift) {
    ScatterGatherOp<1, 0, 1, 0>(-1, outIx, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }
};
