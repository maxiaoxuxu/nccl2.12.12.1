/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

/* 中文逐项-类型 T0070：Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>
 * 类型职责：当前协议的GPU通信原语实现/适配层，组合连接流控、数据地址、复制归约和进度发布。
 * 声明内可见成员（含嵌套成员）：MaxRecv、MaxSend、Input、Output、redOp、tid、nthreads、wid、group、stepLines、fan、userBufs、recvConn、recvConnHeadPtr、recvConnHead、sendConn、sendConnFifoPtr、sendConnHeadPtr、sendConnHead、sendConnHeadCache、recvStep、sendStep、recvBuff、sendBuff、abort、EltPerLine、elt、u2、u4、u8、elt、u2、u4、u8、misalign、u4、u8、elt、u8、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T, typename RedOp, typename Fan, int Direct, int P2p>
class Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>:
  public PrimitivesWithoutDirect<Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>> {

/* 中文逐项-变量 V0479：MaxRecv、MaxSend
 * MaxRecv（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 * MaxSend（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 */
  static constexpr int MaxRecv = Fan::MaxRecv, MaxSend = Fan::MaxSend;
/* 中文逐项-变量 V0480：Input、Output
 * Input（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：用户缓冲表的编译期输入/输出槽下标，不是发送/接收rank。
 * Output（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：用户缓冲表的编译期输入/输出槽下标，不是发送/接收rank。
 */
  static constexpr int Input=0, Output=1;
/* 中文逐项-变量 V0481：redOp
 * redOp（RedOp；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：归约/元素类型选择，决定设备数值运算与 count 到字节数的换算。
 */
  RedOp redOp;
/* 中文逐项-变量 V0482：tid
 * tid（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 */
  const int tid;
/* 中文逐项-变量 V0483：nthreads
 * nthreads（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 */
  const int nthreads;
/* 中文逐项-变量 V0484：wid
 * wid（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：线程在 warp 内的 lane 编号或代码定义的 warp 索引，必须以本处赋值为准。
 */
  const int wid;
/* 中文逐项-变量 V0485：group
 * group（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 */
  const int group;
/* 中文逐项-变量 V0486：stepLines
 * stepLines（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：一个 LL FIFO step 包含的物理协议行数，乘行大小才是物理字节。
 */
  const int stepLines;
/* 中文逐项-变量 V0487：fan
 * fan（Fan；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：原语实际收/发邻居数量，受编译期 MaxRecv/MaxSend 限制。
 */
  Fan fan;
/* 中文逐项-变量 V0488：userBufs
 * userBufs（T；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：本地输入与输出指针表，原语组合按 Input/Output 选择访问源/目标。
 */
  T *userBufs[2];
/* 中文逐项-变量 V0489：recvConn
 * recvConn（struct ncclConnInfo；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：负责接收/发送同步线程持有的设备连接视图，保存跨操作持续 step。
 */
  struct ncclConnInfo* recvConn = NULL;
/* 中文逐项-变量 V0490：recvConnHeadPtr
 * recvConnHeadPtr（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：指向接收消费、发送空间或发送发布计数的共享控制地址。
 */
  volatile uint64_t* recvConnHeadPtr = NULL;
/* 中文逐项-变量 V0491：recvConnHead
 * recvConnHead（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：同步角色缓存/推进的绝对进度，只有完成对应阶段才可发布。
 */
  uint64_t recvConnHead;

/* 中文逐项-变量 V0492：sendConn
 * sendConn（struct ncclConnInfo；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：负责接收/发送同步线程持有的设备连接视图，保存跨操作持续 step。
 */
  struct ncclConnInfo* sendConn = NULL;
/* 中文逐项-变量 V0493：sendConnFifoPtr
 * sendConnFifoPtr（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：同步发送线程缓存的sizesFifo地址，向Proxy报告本step协议物理字节数。
 */
  volatile int* sendConnFifoPtr = NULL;
/* 中文逐项-变量 V0494：sendConnHeadPtr
 * sendConnHeadPtr（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：指向接收消费、发送空间或发送发布计数的共享控制地址。
 */
  volatile uint64_t* sendConnHeadPtr = NULL;
/* 中文逐项-变量 V0495：sendConnHead
 * sendConnHead（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：同步角色缓存/推进的绝对进度，只有完成对应阶段才可发布。
 */
  uint64_t sendConnHead;
/* 中文逐项-变量 V0496：sendConnHeadCache
 * sendConnHeadCache（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：同步角色缓存/推进的绝对进度，只有完成对应阶段才可发布。
 */
  uint64_t sendConnHeadCache; // Cache last seen value

/* 中文逐项-变量 V0497：recvStep
 * recvStep（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：各邻居已推进的长期接收/发送序号，支持循环槽和行内标记复用。
 */
  uint64_t recvStep[MaxRecv];
/* 中文逐项-变量 V0498：sendStep
 * sendStep（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：各邻居已推进的长期接收/发送序号，支持循环槽和行内标记复用。
 */
  uint64_t sendStep[MaxSend];
/* 中文逐项-变量 V0499：recvBuff
 * recvBuff（union ncclLLFifoLine；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：各邻居的协议缓冲指针，按当前 step 计算实际槽位置。
 */
  union ncclLLFifoLine* recvBuff[MaxRecv];
/* 中文逐项-变量 V0500：sendBuff
 * sendBuff（union ncclLLFifoLine；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：各邻居的协议缓冲指针，按当前 step 计算实际槽位置。
 */
  union ncclLLFifoLine* sendBuff[MaxSend];

/* 中文内部逻辑 F0157：recvOffset
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (recvStep[i]%NCCL_STEPS)*stepLines;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0157
 */
/* 中文逐项-函数 F0157：recvOffset
 * 用途与位置：用连接 step 对 NCCL_STEPS 取模再乘每槽大小，把单调进度映射到循环数据槽位。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::recvPtr；src/collectives/device/prims_ll128.h::recvPtr。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (recvStep[i]%NCCL_STEPS)*stepLines;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ int recvOffset(int i) { return (recvStep[i]%NCCL_STEPS)*stepLines; }
/* 中文内部逻辑 F0158：sendOffset
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (sendStep[i]%NCCL_STEPS)*stepLines;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0158
 */
/* 中文逐项-函数 F0158：sendOffset
 * 用途与位置：用连接 step 对 NCCL_STEPS 取模再乘每槽大小，把单调进度映射到循环数据槽位。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::sendPtr；src/collectives/device/prims_ll128.h::sendPtr。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (sendStep[i]%NCCL_STEPS)*stepLines;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ int sendOffset(int i) { return (sendStep[i]%NCCL_STEPS)*stepLines; }
/* 中文内部逻辑 F0159：recvPtr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return recvBuff[i]+recvOffset(i);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0159
 */
/* 中文逐项-函数 F0159：recvPtr
 * 用途与位置：以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::readLL；src/collectives/device/prims_ll.h::readLLBeginAll；src/collectives/device/prims_ll.h::readLLFinish；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体直接出现的调用/包装：recvOffset。
 * 源码返回表达式：return recvBuff[i]+recvOffset(i);。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ union ncclLLFifoLine* recvPtr(int i) { return recvBuff[i]+recvOffset(i); }
/* 中文内部逻辑 F0160：sendPtr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return sendBuff[i]+sendOffset(i);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0160
 */
/* 中文逐项-函数 F0160：sendPtr
 * 用途与位置：以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::incSend；src/collectives/device/prims_ll.h::LLGenericOp；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体直接出现的调用/包装：sendOffset。
 * 源码返回表达式：return sendBuff[i]+sendOffset(i);。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ union ncclLLFifoLine* sendPtr(int i) { return sendBuff[i]+sendOffset(i); }
/* 中文内部逻辑 F0161：recvFlag
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return NCCL_LL_FLAG(recvStep[i]+1);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0161
 */
/* 中文逐项-函数 F0161：recvFlag
 * 用途与位置：把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::readLL；src/collectives/device/prims_ll.h::readLLFinish；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体直接出现的调用/包装：NCCL_LL_FLAG。
 * 源码返回表达式：return NCCL_LL_FLAG(recvStep[i]+1);。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ uint32_t recvFlag(int i) { return NCCL_LL_FLAG(recvStep[i]+1); }
/* 中文内部逻辑 F0162：sendFlag
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return NCCL_LL_FLAG(sendStep[i]+1);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0162
 */
/* 中文逐项-函数 F0162：sendFlag
 * 用途与位置：把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::incSend；src/collectives/device/prims_ll.h::LLGenericOp；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体直接出现的调用/包装：NCCL_LL_FLAG。
 * 源码返回表达式：return NCCL_LL_FLAG(sendStep[i]+1);。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ uint32_t sendFlag(int i) { return NCCL_LL_FLAG(sendStep[i]+1); }

/* 中文内部逻辑 F0163：barrier
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：asm volatile ("bar.sync %1, %0;" :: "r"(nthreads), "r"(15-group));；让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0163
 */
/* 中文逐项-函数 F0163：barrier
 * 用途与位置：同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::waitSend；src/collectives/device/prims_ll.h::postRecv；src/collectives/device/prims_ll.h::~Primitives；src/collectives/device/prims_ll128.h::GenericOp；src/collectives/device/prims_ll128.h::~Primitives；src/collectives/device/prims_simple.h::subBarrier；src/collectives/device/prims_simple.h::genericOp；其余见调用清单。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  inline __device__ void barrier() {
    asm volatile ("bar.sync %1, %0;" :: "r"(nthreads), "r"(15-group));
  }

/* 中文逐项-变量 V0501：abort
 * abort（uint32_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：线程本地缓存的中止状态，周期读取共享 abortFlag 后更新以退出自旋。
 */
  uint32_t abort = 0;

/* 中文内部逻辑 F0164：checkAbort
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：spins++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 条件判断：(abort == 0 && spins == NCCL_SPINS_BEFORE_CHECK_ABORT)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：abort = *ncclShmem.comm.abortFlag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     5. 调用 / 计算 / 写入：spins = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return abort;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0164
 */
/* 中文逐项-函数 F0164：checkAbort
 * 用途与位置：自旋达到检查间隔时读取 communicator 中止标志，使异常情况下的协议等待可以退出。
 * 参数 <未命名参数>（int &spins）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 send（int send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::waitSend；src/collectives/device/prims_ll.h::readLL；src/collectives/device/prims_ll.h::readLLFinish；src/collectives/device/prims_ll128.h::waitSend；src/collectives/device/prims_ll128.h::recvReduceSendCopy；src/collectives/device/prims_simple.h::waitPeer；src/collectives/device/prims_simple.h::setDataPtrs。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：abort == 0 && spins == NCCL_SPINS_BEFORE_CHECK_ABORT。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return abort;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ int checkAbort(int &spins, int send) {
    spins++;
    if (abort == 0 && spins == NCCL_SPINS_BEFORE_CHECK_ABORT) {
      abort = *ncclShmem.comm.abortFlag;
      spins = 0;
    }
    return abort;
  }

/* 中文精读：LL 仍然需要空间流控
 * 行内 flag 解决“数据是否到达”，head 解决“旧数据是否被消费，槽位能否重写”。
 * 发送端仍要求 head+NCCL_STEPS 足以覆盖下一 step，并通过 sizesFifo 告诉 Proxy 物理字节数。
 * 清理轮可能必须发送完整 step 的行内容，即使有效数据很少，也要把可能遗留的 flag 处理干净。
 * 因此不能把 LL 理解成无缓冲、无流控的任意远端写。
 */
/* 中文内部逻辑 F0165：waitSend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(sendConnHeadPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 准备局部状态：int spins = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     4. 循环：初始 ；继续条件 (sendConnHeadCache + NCCL_STEPS < sendConnHead + 1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       5. 调用 / 计算 / 写入：sendConnHeadCache = *sendConnHeadPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 条件判断：(checkAbort(spins, 1))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         7. 真分支：；仅条件为真执行以下子项。
 *           8. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *     9. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     10. 条件判断：(sendConnFifoPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       11. 真分支：；仅条件为真执行以下子项。
 *         12. 准备局部状态：int size = ((sendConnHead & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK) ? stepLines*sizeof(union ncclLLFifoLine) : nbytes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         13. 调用 / 计算 / 写入：sendConnFifoPtr[sendConnHead%NCCL_STEPS] = size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     14. 调用 / 计算 / 写入：sendConnHead += 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：barrier();；同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0165
 */
/* 中文逐项-函数 F0165：waitSend
 * 用途与位置：等待发送缓冲仍有可复用槽位，并在存在 Proxy 时填写本轮物理字节数，防止覆盖 NIC/对端仍使用的数据。
 * 参数 nbytes（int nbytes）：当前消息或片段的字节规模；info.nBytes 对 AG/RS 还计入 nRanks 段总规模。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp；src/collectives/device/prims_ll128.h::GenericOp。
 * 本体直接出现的调用/包装：barrier；checkAbort。
 * 需要对照的边界/推进条件：sendConnHeadPtr；sendConnHeadCache + NCCL_STEPS < sendConnHead + 1；checkAbort(spins,
 * 1；sendConnFifoPtr。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
  inline __device__ void waitSend(int nbytes) {
    if (sendConnHeadPtr) {
/* 中文逐项-变量 V0502：spins
 * spins（int；函数 waitSend 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int spins = 0;
      while (sendConnHeadCache + NCCL_STEPS < sendConnHead + 1) {
        sendConnHeadCache = *sendConnHeadPtr;
        if (checkAbort(spins, 1)) break;
      }
      if (sendConnFifoPtr) {
/* 中文逐项-变量 V0503：size
 * size（int；函数 waitSend 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = ((sendConnHead & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK) ? stepLines*sizeof(union ncclLLFifoLine) :
 * nbytes；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
        int size = ((sendConnHead & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK) ? stepLines*sizeof(union ncclLLFifoLine) : nbytes;
        sendConnFifoPtr[sendConnHead%NCCL_STEPS] = size;
      }
      sendConnHead += 1;
    }
    barrier();
  }

/* 中文内部逻辑 F0166：incRecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：recvStep[i] += 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0166
 */
/* 中文逐项-函数 F0166：incRecv
 * 用途与位置：递增指定接收邻居的协议 step，下一轮会选择后续槽位和新的期望 flag。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  inline __device__ void incRecv(int i) {
    recvStep[i] += 1;
  }
/* 中文内部逻辑 F0167：postRecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：barrier();；同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。
 * 2. 条件判断：(recvConnHeadPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：*recvConnHeadPtr = recvConnHead += 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0167
 */
/* 中文逐项-函数 F0167：postRecv
 * 用途与位置：在数据已消费后发布接收完成进度，通知生产者可以回收相应循环槽位。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp；src/collectives/device/prims_ll128.h::GenericOp。
 * 本体直接出现的调用/包装：barrier。
 * 需要对照的边界/推进条件：recvConnHeadPtr。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
  inline __device__ void postRecv() {
    barrier();
    if (recvConnHeadPtr) *recvConnHeadPtr = recvConnHead += 1;
  }

/* 中文内部逻辑 F0168：incSend
 * 精读：当step命中清理掩码时，把未覆盖行的flag也清理/重写，防止序号回绕后误识别旧数据。
 * 精读：随后才递增该邻居发送step；普通行内就绪判断与缓冲空间head流控仍独立存在。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：((sendStep[i] & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 循环：初始 int o = offset;；继续条件 o<stepLines；每轮更新 o+=nthreads；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       4. 调用 / 计算 / 写入：storeLL(sendPtr(i)+o, 0, sendFlag(i));；把有效数据与两个进度 flag 编码进 LL 行并写出，以行级就绪取代整片等待。
 *     5. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 6. 调用 / 计算 / 写入：sendStep[i]++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0168
 */
/* 中文逐项-函数 F0168：incSend
 * 用途与位置：推进发送方协议 step，LL 标志回绕前会清理未使用行，防止旧 flag 被误判为新数据。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 offset（int offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp。
 * 本体直接出现的调用/包装：sendFlag；sendPtr；storeLL。
 * 需要对照的边界/推进条件：sendStep[i] & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK；o<stepLines。各条件所属分支见紧邻函数体。
 */
  inline __device__ void incSend(int i, int offset) {
    // LL Cleanup : write all flags in the slice to make sure we don't have
    // data corruption when flag loops over.
    if ((sendStep[i] & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK) {
      for (/* 中文逐项-变量 V0504：o
 * o（int；函数 incSend 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 o = offset；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：o<stepLines；循环体与增量表达式决定每轮如何推进。
 */int o = offset; o<stepLines; o+=nthreads) storeLL(sendPtr(i)+o, 0, sendFlag(i));
    }
    sendStep[i]++;
  }

/* 中文精读：LL 接收为何逐行轮询两个 flag
 * 读取 data/flag 的 16B 行，只有 flag1 与 flag2 都等于 recvStep+1 才接受这一行。
 * 数据和完成标记放在同一格式内，降低单独等待整片 tail/fence 的延迟，代价是 50% flag 开销。
 * 循环会检查 abort；正常情况下数据可以边到边被下游消费，形成细粒度流水。
 * flag 发生回绕时 incSend 的 cleanup 会清理整段，防止旧 flag 与新期望值碰巧相同。
 */
  __device__ /* 中文内部逻辑 F0169：readLL
 * 精读：每次读整行的data/flag视图，只有flag1和flag2同时等于当前期望序号才拼出有效64位数据。
 * 精读：中途只看到一个新flag不够；等待中周期检查abort，不把部分新旧混合行作为正确载荷。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union ncclLLFifoLine* src = recvPtr(i) + offset;；以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。
 * 2. 准备局部状态：uint32_t flag = recvFlag(i);；把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。
 * 3. 准备局部状态：uint32_t data1, flag1, data2, flag2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int spins = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 先执行一次的循环：初始 ；继续条件 ((flag1 != flag) || (flag2 != flag))；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *   6. 调用 / 计算 / 写入：asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(data1), "=r"(flag1), "=r"(data2), "=r"(flag2) : "l"(&src->i4));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   7. 条件判断：(checkAbort(spins, 0))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     8. 真分支：；仅条件为真执行以下子项。
 *       9. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 准备局部状态：uint64_t val64 = data1 + (((uint64_t)data2) << 32);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 函数返回：return val64;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0169
 */
/* 中文逐项-函数 F0169：readLL
 * 用途与位置：反复读取一个 LL 行直到两个 flag 都匹配本轮序号，再取出其中 8 字节有效数据。
 * 参数 offset（int offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp。
 * 本体直接出现的调用/包装：checkAbort；recvFlag；recvPtr。
 * 需要对照的边界/推进条件：checkAbort(spins, 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return val64;。CHECK类宏还可能提前返回/跳转。
 */uint64_t readLL(int offset, int i) {
/* 中文逐项-变量 V0505：src
 * src（union ncclLLFifoLine；函数 readLL 内）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 初值/来源 src = recvPtr(i) + offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    union ncclLLFifoLine* src = recvPtr(i) + offset;
/* 中文逐项-变量 V0506：flag
 * flag（uint32_t；函数 readLL 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = recvFlag(i)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    uint32_t flag = recvFlag(i);
/* 中文逐项-变量 V0507：data1、flag1、data2、flag2
 * data1（uint32_t；函数 readLL 内）：一个LL物理行中的两个32位用户载荷分量，配套flag1/flag2才表明数据完整。
 * 此处未显式给初值；后续首个文本使用点：asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(data1), "=r"(flag1), "=r"(data2),
 * "=r"(flag2) : "l"(&src->i4));。若由输出参数填入，必须先检查填充调用成功再读值。
 * flag1（uint32_t；函数 readLL 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 此处未显式给初值；后续首个文本使用点：asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(data1), "=r"(flag1), "=r"(data2),
 * "=r"(flag2) : "l"(&src->i4));。若由输出参数填入，必须先检查填充调用成功再读值。
 * data2（uint32_t；函数 readLL 内）：一个LL物理行中的两个32位用户载荷分量，配套flag1/flag2才表明数据完整。
 * 此处未显式给初值；后续首个文本使用点：asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(data1), "=r"(flag1), "=r"(data2),
 * "=r"(flag2) : "l"(&src->i4));。若由输出参数填入，必须先检查填充调用成功再读值。
 * flag2（uint32_t；函数 readLL 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 此处未显式给初值；后续首个文本使用点：asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(data1), "=r"(flag1), "=r"(data2),
 * "=r"(flag2) : "l"(&src->i4));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    uint32_t data1, flag1, data2, flag2;
/* 中文逐项-变量 V0508：spins
 * spins（int；函数 readLL 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int spins = 0;
    do {
      asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(data1), "=r"(flag1), "=r"(data2), "=r"(flag2) : "l"(&src->i4));
      if (checkAbort(spins, 0)) break;
    } while ((flag1 != flag) || (flag2 != flag));
/* 中文逐项-变量 V0509：val64
 * val64（uint64_t；函数 readLL 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 val64 = data1 + (((uint64_t)data2) << 32)；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
    uint64_t val64 = data1 + (((uint64_t)data2) << 32);
    return val64;
  }

/* 中文内部逻辑 F0170：readLLBeginAll
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 2. 循环：初始 int i=BeginIx;；继续条件 i < MaxRecv；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 条件判断：(i < fan.nrecv())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 准备局部状态：union ncclLLFifoLine* src = recvPtr(i) + offset;；以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。
 *       6. 调用 / 计算 / 写入：asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(line[i].data1), "=r"(line[i].flag1), "=r"(line[i].data2), "=r"(line[i].flag2) : "l"(&src->i4));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0170
 */
/* 中文逐项-函数 F0170：readLLBeginAll
 * 用途与位置：先向多个接收邻居发出 LL 行读取，把访存提前以便后续 Finish 隐藏加载延迟。
 * 参数 offset（int offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 参数 <未命名参数>（ncclLLFifoLine(&line)[MaxRecv]）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp。
 * 本体直接出现的调用/包装：fan.nrecv；recvPtr。
 * 需要对照的边界/推进条件：i < MaxRecv；i < fan.nrecv。各条件所属分支见紧邻函数体。
 */
  template<int BeginIx>
  __device__ void readLLBeginAll(int offset, ncclLLFifoLine(&line)[MaxRecv]) {
    #pragma unroll
    for (/* 中文逐项-变量 V0510：i
 * i（int；函数 readLLBeginAll 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = BeginIx；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i < MaxRecv；循环体与增量表达式决定每轮如何推进。
 */int i=BeginIx; i < MaxRecv; i++) {
      if (i < fan.nrecv()) {
/* 中文逐项-变量 V0511：src
 * src（union ncclLLFifoLine；函数 readLLBeginAll 内）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 初值/来源 src = recvPtr(i) + offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        union ncclLLFifoLine* src = recvPtr(i) + offset;
        asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(line[i].data1), "=r"(line[i].flag1), "=r"(line[i].data2), "=r"(line[i].flag2) : "l"(&src->i4));
      }
    }
  }
  __device__ /* 中文内部逻辑 F0171：readLLFinish
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：union ncclLLFifoLine* src = recvPtr(i) + offset;；以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。
 * 2. 准备局部状态：uint32_t flag = recvFlag(i);；把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。
 * 3. 准备局部状态：int spins = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 ；继续条件 (line[i].flag1 != flag || line[i].flag2 != flag)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(line[i].data1), "=r"(line[i].flag1), "=r"(line[i].data2), "=r"(line[i].flag2) : "l"(&src->i4));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   6. 条件判断：(checkAbort(spins, 0))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     7. 真分支：；仅条件为真执行以下子项。
 *       8. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 * 9. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 10. 准备局部状态：uint64_t val64 = line[i].data1 + (((uint64_t)line[i].data2) << 32);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 函数返回：return val64;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0171
 */
/* 中文逐项-函数 F0171：readLLFinish
 * 用途与位置：检查预加载 LL 行的 flag，不匹配时继续读，直到能够安全提取数据或中止。
 * 参数 offset（int offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 参数 <未命名参数>（ncclLLFifoLine(&line)[MaxRecv]）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp。
 * 本体直接出现的调用/包装：checkAbort；recvFlag；recvPtr。
 * 需要对照的边界/推进条件：line[i].flag1 != flag || line[i].flag2 != flag；checkAbort(spins, 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return val64;。CHECK类宏还可能提前返回/跳转。
 */uint64_t readLLFinish(int offset, ncclLLFifoLine(&line)[MaxRecv], int i) {
/* 中文逐项-变量 V0512：src
 * src（union ncclLLFifoLine；函数 readLLFinish 内）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 初值/来源 src = recvPtr(i) + offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    union ncclLLFifoLine* src = recvPtr(i) + offset;
/* 中文逐项-变量 V0513：flag
 * flag（uint32_t；函数 readLLFinish 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = recvFlag(i)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    uint32_t flag = recvFlag(i);
/* 中文逐项-变量 V0514：spins
 * spins（int；函数 readLLFinish 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int spins = 0;
    while (line[i].flag1 != flag || line[i].flag2 != flag) {
      asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(line[i].data1), "=r"(line[i].flag1), "=r"(line[i].data2), "=r"(line[i].flag2) : "l"(&src->i4));
      if (checkAbort(spins, 0)) break;
    }
/* 中文逐项-变量 V0515：val64
 * val64（uint64_t；函数 readLLFinish 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 val64 = line[i].data1 + (((uint64_t)line[i].data2) << 32)；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
    uint64_t val64 = line[i].data1 + (((uint64_t)line[i].data2) << 32);
    return val64;
  }

  __device__ /* 中文内部逻辑 F0172：storeLL
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：asm volatile("st.volatile.global.v4.u32 [%0], {%1,%2,%3,%4};" :: "l"(&dst->i4), "r"((uint32_t)val), "r"(flag), "r"((uint32_t)(val >> 32)), "r"(flag));；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0172
 */
/* 中文逐项-函数 F0172：storeLL
 * 用途与位置：把有效数据与两个进度 flag 编码进 LL 行并写出，以行级就绪取代整片等待。
 * 参数 dst（union ncclLLFifoLine* dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 val（uint64_t val）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 参数 flag（uint32_t flag）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::incSend；src/collectives/device/prims_ll.h::LLGenericOp。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */void storeLL(union ncclLLFifoLine* dst, uint64_t val, uint32_t flag) {
    asm volatile("st.volatile.global.v4.u32 [%0], {%1,%2,%3,%4};" :: "l"(&dst->i4), "r"((uint32_t)val), "r"(flag), "r"((uint32_t)(val >> 32)), "r"(flag));
  }

/* 中文逐项-变量 V0516：EltPerLine
 * EltPerLine（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>）：协议行/片的有效元素或物理字数常量，区分标记开销和真正用户载荷。
 */
  static constexpr int EltPerLine = sizeof(uint64_t)/sizeof(T);

/* 中文内部逻辑 F0173：load
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：union { U elt; uint16_t u2; uint32_t u4; uint64_t u8; }；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 条件判断：(sizeof(U) == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：asm("ld.volatile.global.b8 %0,[%1];" : "=r"(u4) : "l"(src));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   5. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     6. 条件判断：(sizeof(U) == 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       7. 真分支：；仅条件为真执行以下子项。
 *         8. 调用 / 计算 / 写入：asm("ld.volatile.global.b16 %0,[%1];" : "=h"(u2) : "l"(src));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       9. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         10. 条件判断：(sizeof(U) == 4)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           11. 真分支：；仅条件为真执行以下子项。
 *             12. 调用 / 计算 / 写入：asm("ld.volatile.global.b32 %0,[%1];" : "=r"(u4) : "l"(src));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           13. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             14. 调用 / 计算 / 写入：asm("ld.volatile.global.b64 %0,[%1];" : "=l"(u8) : "l"(src));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return elt;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0173
 */
/* 中文逐项-函数 F0173：load
 * 用途与位置：把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。
 * 参数 src（U *src）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::loadBegin。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：sizeof(U) == 1；sizeof(U) == 2；sizeof(U) == 4。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return elt;。CHECK类宏还可能提前返回/跳转。
 */
  template<typename U>
  __device__ static U load(U *src) {
/* 中文逐项-类型 T0071：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：elt、u2、u4、u8。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0517：elt
 * elt（U；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      U elt;
/* 中文逐项-变量 V0518：u2
 * u2（uint16_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint16_t u2;
/* 中文逐项-变量 V0519：u4
 * u4（uint32_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint32_t u4;
/* 中文逐项-变量 V0520：u8
 * u8（uint64_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint64_t u8;
    };
    if(sizeof(U) == 1)
      asm("ld.volatile.global.b8 %0,[%1];" : "=r"(u4) : "l"(src));
    else if(sizeof(U) == 2)
      asm("ld.volatile.global.b16 %0,[%1];" : "=h"(u2) : "l"(src));
    else if(sizeof(U) == 4)
      asm("ld.volatile.global.b32 %0,[%1];" : "=r"(u4) : "l"(src));
    else
      asm("ld.volatile.global.b64 %0,[%1];" : "=l"(u8) : "l"(src));
    return elt;
  }

/* 中文内部逻辑 F0174：store
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：union { U elt; uint16_t u2; uint32_t u4; uint64_t u8; }；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 调用 / 计算 / 写入：elt = val;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 条件判断：(sizeof(U) == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：asm("st.volatile.global.b8 [%0],%1;" :: "l"(dst), "r"(u4));；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *   6. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     7. 条件判断：(sizeof(U) == 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       8. 真分支：；仅条件为真执行以下子项。
 *         9. 调用 / 计算 / 写入：asm("st.volatile.global.b16 [%0],%1;" :: "l"(dst), "h"(u2));；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *       10. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         11. 条件判断：(sizeof(U) == 4)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           12. 真分支：；仅条件为真执行以下子项。
 *             13. 调用 / 计算 / 写入：asm("st.volatile.global.b32 [%0],%1;" :: "l"(dst), "r"(u4));；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *           14. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             15. 调用 / 计算 / 写入：asm("st.volatile.global.b64 [%0],%1;" :: "l"(dst), "l"(u8));；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0174
 */
/* 中文逐项-函数 F0174：store
 * 用途与位置：把协议寄存器值按用户元素布局写回目标，处理类型宽度和不足整包的尾部。
 * 参数 dst（U *dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 val（U val）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：sizeof(U) == 1；sizeof(U) == 2；sizeof(U) == 4。各条件所属分支见紧邻函数体。
 */
  template<typename U>
  __device__ static void store(U *dst, U val) {
/* 中文逐项-类型 T0072：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：elt、u2、u4、u8。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0521：elt
 * elt（U；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      U elt;
/* 中文逐项-变量 V0522：u2
 * u2（uint16_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint16_t u2;
/* 中文逐项-变量 V0523：u4
 * u4（uint32_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint32_t u4;
/* 中文逐项-变量 V0524：u8
 * u8（uint64_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint64_t u8;
    };
    elt = val;
    if(sizeof(U) == 1)
      asm("st.volatile.global.b8 [%0],%1;" :: "l"(dst), "r"(u4));
    else if(sizeof(U) == 2)
      asm("st.volatile.global.b16 [%0],%1;" :: "l"(dst), "h"(u2));
    else if(sizeof(U) == 4)
      asm("st.volatile.global.b32 [%0],%1;" :: "l"(dst), "r"(u4));
    else
      asm("st.volatile.global.b64 [%0],%1;" :: "l"(dst), "l"(u8));
  }

/* 中文逐项-类型 T0073：DataLoader
 * 类型职责：本文件的struct类型，用成员把下面的相关状态放在一起；创建、修改和释放位置由使用它的函数控制。
 * 声明内可见成员（含嵌套成员）：misalign、u4、u8、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct DataLoader {
/* 中文逐项-变量 V0525：misalign
 * misalign（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>::DataLoader）：输入地址相对协议向量对齐的偏移，决定如何拼接不对齐加载。
 */
    int misalign;
/* 中文逐项-类型 T0074：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：u4、u8、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0526：u4
 * u4（uint32_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>::DataLoader::<匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint32_t u4[sizeof(T) <= 2 ? 3 : 2];
/* 中文逐项-变量 V0527：u8
 * u8（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>::DataLoader::<匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint64_t u8;
/* 中文逐项-变量 V0528：elt
 * elt（T；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL,
 * P2p>::DataLoader::<匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      T elt[EltPerLine];
    };

    __device__ /* 中文内部逻辑 F0175：loadBegin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(sizeof(T) <= 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：misalign = reinterpret_cast<uintptr_t>(src)%4;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     4. 准备局部状态：uint32_t *p = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(src) & -uintptr_t(4));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：u4[0] = load(p+0);；把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。
 *     6. 调用 / 计算 / 写入：u4[1] = misalign + eltN*sizeof(T) > 4 ? load(p+1) : 0;；把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。
 *     7. 调用 / 计算 / 写入：u4[sizeof(T) <= 2 ? 2 : 0] = misalign + eltN*sizeof(T) > 8 ? load(p+2) : 0;；把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。
 *   8. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     9. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     10. 循环：初始 int i=0;；继续条件 i < EltPerLine；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       11. 条件判断：(i==0 || i < eltN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         12. 真分支：；仅条件为真执行以下子项。
 *           13. 调用 / 计算 / 写入：elt[i] = load(src + i);；把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。
 *     14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0175
 */
/* 中文逐项-函数 F0175：loadBegin
 * 用途与位置：把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。
 * 参数 src（T *src）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：load；reinterpret_cast<uint32_t*>；reinterpret_cast<uintptr_t>；uintptr_t。
 * 需要对照的边界/推进条件：sizeof(T) <= 2；i < EltPerLine；i==0 || i < eltN。各条件所属分支见紧邻函数体。
 */void loadBegin(T *src, int eltN) {
      if (sizeof(T) <= 2) {
        misalign = reinterpret_cast<uintptr_t>(src)%4;
/* 中文逐项-变量 V0529：p
 * p（uint32_t；函数 loadBegin 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(src) &
 * -uintptr_t(4))；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        uint32_t *p = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(src) & -uintptr_t(4));
        u4[0] = load(p+0);
        u4[1] = misalign + eltN*sizeof(T) > 4 ? load(p+1) : 0;
        // u4[2] would be simpler, but that throws warnings on some compilers
        u4[sizeof(T) <= 2 ? 2 : 0] = misalign + eltN*sizeof(T) > 8 ? load(p+2) : 0;
      }
      else {
        #pragma unroll
        for(/* 中文逐项-变量 V0530：i
 * i（int；函数 loadBegin 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < EltPerLine；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < EltPerLine; i++) {
          if(i==0 || i < eltN)
            elt[i] = load(src + i);
        }
      }
    }

    __device__ /* 中文内部逻辑 F0176：loadFinish
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(sizeof(T) <= 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：u4[0] = __funnelshift_r(u4[0], u4[1], 8*misalign);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     4. 调用 / 计算 / 写入：u4[1] = __funnelshift_r(u4[1], u4[sizeof(T) <= 2 ? 2 : 0], 8*misalign);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return u8;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0176
 */
/* 中文逐项-函数 F0176：loadFinish
 * 用途与位置：把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：__funnelshift_r。
 * 需要对照的边界/推进条件：sizeof(T) <= 2。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return u8;。CHECK类宏还可能提前返回/跳转。
 */uint64_t loadFinish() {
      if (sizeof(T) <= 2) {
        u4[0] = __funnelshift_r(u4[0], u4[1], 8*misalign);
        // u4[2] would be simpler, but that throws warnings on some compilers
        u4[1] = __funnelshift_r(u4[1], u4[sizeof(T) <= 2 ? 2 : 0], 8*misalign);
      }
      return u8;
    }
  };

  __device__ /* 中文内部逻辑 F0177：storeData
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：union { uint64_t u8; T elt[EltPerLine]; }；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 调用 / 计算 / 写入：u8 = val;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 编译期分支/指令：#pragma unroll；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 * 4. 循环：初始 int i=0;；继续条件 i < EltPerLine；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 条件判断：(i==0 || i < eltN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     6. 真分支：；仅条件为真执行以下子项。
 *       7. 调用 / 计算 / 写入：dst[i] = elt[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0177
 */
/* 中文逐项-函数 F0177：storeData
 * 用途与位置：把协议寄存器值按用户元素布局写回目标，处理类型宽度和不足整包的尾部。
 * 参数 dst（T *dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 val（uint64_t val）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i < EltPerLine；i==0 || i < eltN。各条件所属分支见紧邻函数体。
 */void storeData(T *dst, uint64_t val, int eltN) {
/* 中文逐项-类型 T0075：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：u8、elt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-变量 V0531：u8
 * u8（uint64_t；成员 <匿名>）：同一位模式的标量/向量重解释或暂存，方便按寄存器包处理多个小元素。
 */
      uint64_t u8;
/* 中文逐项-变量 V0532：elt
 * elt（T；成员 <匿名>）：当前union/转换器的分量视图，按其声明元素类型解释同一storage/pack位模式中的小元素。
 */
      T elt[EltPerLine];
    };
    u8 = val;
    #pragma unroll
    for(/* 中文逐项-变量 V0533：i
 * i（int；函数 storeData 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < EltPerLine；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < EltPerLine; i++) {
      if (i==0 || i < eltN)
        //store(dst+i, elt[i]);
        dst[i] = elt[i];
    }
  }

/* 中文内部逻辑 F0178：LLGenericOp
 * 精读：SEND先waitSend获取可写空间并考虑标记回绕清理；每线程按LL行跨步处理元素。
 * 精读：Begin本地加载与预读其他邻居在前，首邻居readLL等待两个flag，再完成本地预处理和多源归约。
 * 精读：可选postOp后把data/flag发下游，并按DST保留本地；最后推进邻居step并postRecv。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：constexpr int SRC = SrcBuf != -1 ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：constexpr int DST = DstBuf != -1 ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：T *srcElts = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：T *dstElts = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：nelem = nelem < 0 ? 0 : nelem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 条件判断：(SEND)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 调用 / 计算 / 写入：waitSend(divUp(nelem, EltPerLine)*sizeof(ncclLLFifoLine));；等待发送缓冲仍有可复用槽位，并在存在 Proxy 时填写本轮物理字节数，防止覆盖 NIC/对端仍使用的数据。
 * 9. 调用 / 计算 / 写入：nelem -= tid*EltPerLine;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：srcElts += tid*EltPerLine;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：dstElts += tid*EltPerLine;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 准备局部状态：int offset = tid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：int eltPerTrip = nthreads*EltPerLine;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 循环：初始 ；继续条件 (nelem > 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   15. 准备局部状态：int eltInLine = EltPerLine < nelem ? EltPerLine : nelem;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   16. 准备局部状态：DataLoader dl;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   17. 准备局部状态：ncclLLFifoLine line[MaxRecv];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   18. 准备局部状态：uint64_t data, peerData;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   19. 条件判断：(SRC)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     20. 真分支：；仅条件为真执行以下子项。
 *       21. 调用 / 计算 / 写入：dl.loadBegin(srcElts, eltInLine);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       22. 调用 / 计算 / 写入：srcElts += eltPerTrip;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   23. 条件判断：(RECV)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     24. 真分支：；仅条件为真执行以下子项。
 *       25. 调用 / 计算 / 写入：readLLBeginAll<1>(offset, line);；先向多个接收邻居发出 LL 行读取，把访存提前以便后续 Finish 隐藏加载延迟。
 *       26. 调用 / 计算 / 写入：peerData = readLL(offset, 0);；反复读取一个 LL 行直到两个 flag 都匹配本轮序号，再取出其中 8 字节有效数据。
 *   27. 条件判断：(SRC)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     28. 真分支：；仅条件为真执行以下子项。
 *       29. 调用 / 计算 / 写入：data = dl.loadFinish();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       30. 条件判断：(SrcBuf == Input)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         31. 真分支：；仅条件为真执行以下子项。
 *           32. 调用 / 计算 / 写入：data = MULTI<RedOp, T>().preOp(redOp, data);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   33. 条件判断：(RECV)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     34. 真分支：；仅条件为真执行以下子项。
 *       35. 调用 / 计算 / 写入：data = !SRC ? peerData : MULTI<RedOp,T>()(redOp, peerData, data);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       36. 编译期分支/指令：#pragma unroll MaxRecv；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       37. 循环：初始 int i=1;；继续条件 i < MaxRecv && i < fan.nrecv()；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         38. 调用 / 计算 / 写入：peerData = readLLFinish(offset, line, i);；检查预加载 LL 行的 flag，不匹配时继续读，直到能够安全提取数据或中止。
 *         39. 调用 / 计算 / 写入：data = MULTI<RedOp,T>()(redOp, peerData, data);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       40. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   41. 条件判断：(postOp)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     42. 真分支：；仅条件为真执行以下子项。
 *       43. 调用 / 计算 / 写入：data = MULTI<RedOp, T>().postOp(redOp, data);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   44. 条件判断：(SEND)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     45. 真分支：；仅条件为真执行以下子项。
 *       46. 循环：初始 int i=1;；继续条件 i < MaxSend && i < fan.nsend()；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         47. 调用 / 计算 / 写入：storeLL(sendPtr(i)+offset, data, sendFlag(i));；把有效数据与两个进度 flag 编码进 LL 行并写出，以行级就绪取代整片等待。
 *       48. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       49. 调用 / 计算 / 写入：storeLL(sendPtr(0)+offset, data, sendFlag(0));；把有效数据与两个进度 flag 编码进 LL 行并写出，以行级就绪取代整片等待。
 *   50. 条件判断：(DST)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     51. 真分支：；仅条件为真执行以下子项。
 *       52. 调用 / 计算 / 写入：storeData(dstElts, data, eltInLine);；把协议寄存器值按用户元素布局写回目标，处理类型宽度和不足整包的尾部。
 *       53. 调用 / 计算 / 写入：dstElts += eltPerTrip;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   54. 调用 / 计算 / 写入：nelem -= eltPerTrip;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   55. 调用 / 计算 / 写入：offset += nthreads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 56. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 57. 条件判断：(RECV)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   58. 真分支：；仅条件为真执行以下子项。
 *     59. 循环：初始 int i=0;；继续条件 i < MaxRecv；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       60. 调用 / 计算 / 写入：incRecv(i);；递增指定接收邻居的协议 step，下一轮会选择后续槽位和新的期望 flag。
 *     61. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     62. 调用 / 计算 / 写入：postRecv();；在数据已消费后发布接收完成进度，通知生产者可以回收相应循环槽位。
 * 63. 条件判断：(SEND)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   64. 真分支：；仅条件为真执行以下子项。
 *     65. 循环：初始 int i=1;；继续条件 i < MaxSend && i < fan.nsend()；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       66. 调用 / 计算 / 写入：incSend(i, offset);；推进发送方协议 step，LL 标志回绕前会清理未使用行，防止旧 flag 被误判为新数据。
 *     67. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     68. 调用 / 计算 / 写入：incSend(0, offset);；推进发送方协议 step，LL 标志回绕前会清理未使用行，防止旧 flag 被误判为新数据。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0178
 */
/* 中文逐项-函数 F0178：LLGenericOp
 * 用途与位置：依据编译期收发/输入输出组合，按 LL 行等待、融合归约、写下游并推进 step。
 * 参数 srcIx（intptr_t srcIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 dstIx（intptr_t dstIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 nelem（int nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::send；src/collectives/device/prims_ll.h::sendFromOutput；src/collectives/device/prims_ll.h::recv；src/collectives/device/prims_ll.h::recvReduceSend；src/collectives/device/prims_ll.h::recvReduceCopy；src/collectives/device/prims_ll.h::copySend；src/collectives/device/prims_ll.h::recvCopySend；其余见调用清单。
 * 本体直接出现的调用/包装：MULTI<RedOp, T>；MULTI<RedOp, T>().postOp；MULTI<RedOp,
 * T>().preOp；MULTI<RedOp,T>；MULTI<RedOp,T>()；divUp；dl.loadBegin；dl.loadFinish；fan.nrecv；fan.nsend；incRecv；incSend；postRecv；readLL；readLLBeginAll<1>；readLLFinish；sendFlag；sendPtr；更多调用保留在函数体。
 * 需要对照的边界/推进条件：SEND；nelem > 0；SRC；RECV。各条件所属分支见紧邻函数体。
 */
  template <int RECV, int SEND, int SrcBuf, int DstBuf>
/* 中文导读：
 * LL 将有效数据与递增 flag 一起传递，接收方轮询 flag 判断每个粒度的数据是否就绪。
 * 16 字节 ncclLLFifoLine 中只有 8 字节有效载荷，付出带宽开销以获得细粒度低延迟同步。
 */
  __device__ __forceinline__ void LLGenericOp(intptr_t srcIx, intptr_t dstIx, int nelem, bool postOp) {
/* 中文逐项-变量 V0534：SRC
 * SRC（int；函数 LLGenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 SRC = SrcBuf != -1 ? 1 : 0；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    constexpr int SRC = SrcBuf != -1 ? 1 : 0;
/* 中文逐项-变量 V0535：DST
 * DST（int；函数 LLGenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 DST = DstBuf != -1 ? 1 : 0；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    constexpr int DST = DstBuf != -1 ? 1 : 0;
/* 中文逐项-变量 V0536：srcElts
 * srcElts（T；函数 LLGenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 srcElts = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    T *srcElts = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx;
/* 中文逐项-变量 V0537：dstElts
 * dstElts（T；函数 LLGenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 dstElts = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    T *dstElts = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx;

    // Always waitSend in case of cleanup
    nelem = nelem < 0 ? 0 : nelem;
    if (SEND) waitSend(divUp(nelem, EltPerLine)*sizeof(ncclLLFifoLine));

    nelem -= tid*EltPerLine;
    srcElts += tid*EltPerLine;
    dstElts += tid*EltPerLine;
/* 中文逐项-变量 V0538：offset
 * offset（int；函数 LLGenericOp 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = tid；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int offset = tid;
/* 中文逐项-变量 V0539：eltPerTrip
 * eltPerTrip（int；函数 LLGenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 eltPerTrip = nthreads*EltPerLine；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int eltPerTrip = nthreads*EltPerLine;
    while (nelem > 0) {
/* 中文逐项-变量 V0540：eltInLine
 * eltInLine（int；函数 LLGenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 eltInLine = EltPerLine < nelem ? EltPerLine : nelem；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int eltInLine = EltPerLine < nelem ? EltPerLine : nelem;

/* 中文逐项-变量 V0541：dl
 * dl（DataLoader；函数 LLGenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：dl.loadBegin(srcElts, eltInLine);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      DataLoader dl;
/* 中文逐项-变量 V0542：line
 * line（ncclLLFifoLine；函数 LLGenericOp 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：readLLBeginAll<1>(offset, line);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ncclLLFifoLine line[MaxRecv];
/* 中文逐项-变量 V0543：data、peerData
 * data（uint64_t；函数 LLGenericOp 内）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 此处未显式给初值；后续首个文本使用点：data = dl.loadFinish();。若由输出参数填入，必须先检查填充调用成功再读值。
 * peerData（uint64_t；函数 LLGenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：peerData = readLL(offset, 0);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      uint64_t data, peerData;
      if (SRC) {
        dl.loadBegin(srcElts, eltInLine);
        srcElts += eltPerTrip;
      }
      if (RECV) {
        readLLBeginAll<1>(offset, line);
        peerData = readLL(offset, 0);
      }
      if (SRC) {
        data = dl.loadFinish();
        if (SrcBuf == Input) data = MULTI<RedOp, T>().preOp(redOp, data);
      }
      if (RECV) {
        data = !SRC ? peerData : MULTI<RedOp,T>()(redOp, peerData, data);
        #pragma unroll MaxRecv
        for (/* 中文逐项-变量 V0544：i
 * i（int；函数 LLGenericOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i < MaxRecv && i < fan.nrecv()；循环体与增量表达式决定每轮如何推进。
 */int i=1; i < MaxRecv && i < fan.nrecv(); i++) {
          peerData = readLLFinish(offset, line, i);
          data = MULTI<RedOp,T>()(redOp, peerData, data);
        }
      }

      if (postOp) data = MULTI<RedOp, T>().postOp(redOp, data);

      // Send : inter-node, then intra-node, then local
      if (SEND) {
        for (/* 中文逐项-变量 V0545：i
 * i（int；函数 LLGenericOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i < MaxSend && i < fan.nsend()；循环体与增量表达式决定每轮如何推进。
 */int i=1; i < MaxSend && i < fan.nsend(); i++)
          storeLL(sendPtr(i)+offset, data, sendFlag(i));
        storeLL(sendPtr(0)+offset, data, sendFlag(0));
      }
      if (DST) {
        storeData(dstElts, data, eltInLine);
        dstElts += eltPerTrip;
      }
      nelem -= eltPerTrip;
      offset += nthreads;
    }

    if (RECV) {
      for (/* 中文逐项-变量 V0546：i
 * i（int；函数 LLGenericOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < MaxRecv；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < MaxRecv; i++) incRecv(i);
      postRecv();
    }
    if (SEND) {
      for (/* 中文逐项-变量 V0547：i
 * i（int；函数 LLGenericOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i < MaxSend && i < fan.nsend()；循环体与增量表达式决定每轮如何推进。
 */int i=1; i < MaxSend && i < fan.nsend(); i++)
        incSend(i, offset);
      incSend(0, offset);
    }
  }

  __device__ __forceinline__ /* 中文内部逻辑 F0179：loadRecvConn
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：recvBuff[i] = (union ncclLLFifoLine*)conn->buffs[NCCL_PROTO_LL];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：recvStep[i] = conn->step;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 3. 条件判断：(wid == i)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：recvConn = conn;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0179
 */
/* 中文逐项-函数 F0179：loadRecvConn
 * 用途与位置：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。
 * 参数 conn（struct ncclConnInfo* conn）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：wid == i。各条件所属分支见紧邻函数体。
 */void loadRecvConn(struct ncclConnInfo* conn, int i) {
    recvBuff[i] = (union ncclLLFifoLine*)conn->buffs[NCCL_PROTO_LL];
    recvStep[i] = conn->step;
    if (wid == i) recvConn = conn;
  }
  __device__ __forceinline__ /* 中文内部逻辑 F0180：loadRecvSync
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(tid >= nthreads-WARP_SIZE && wid < fan.nrecv())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：recvConnHeadPtr = recvConn->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     4. 调用 / 计算 / 写入：recvConnHead = recvConn->step;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0180
 */
/* 中文逐项-函数 F0180：loadRecvSync
 * 用途与位置：为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives。
 * 本体直接出现的调用/包装：fan.nrecv。
 * 需要对照的边界/推进条件：tid >= nthreads-WARP_SIZE && wid < fan.nrecv。各条件所属分支见紧邻函数体。
 */void loadRecvSync() {
    if (tid >= nthreads-WARP_SIZE && wid < fan.nrecv()) {
      recvConnHeadPtr = recvConn->head;
      recvConnHead = recvConn->step;
    }
  }

  __device__ __forceinline__ /* 中文内部逻辑 F0181：loadSendConn
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：sendBuff[i] = (union ncclLLFifoLine*)conn->buffs[NCCL_PROTO_LL];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：sendStep[i] = conn->step;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 3. 条件判断：(wid == i)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：sendConn = conn;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0181
 */
/* 中文逐项-函数 F0181：loadSendConn
 * 用途与位置：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。
 * 参数 conn（struct ncclConnInfo* conn）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：wid == i。各条件所属分支见紧邻函数体。
 */void loadSendConn(struct ncclConnInfo* conn, int i) {
    sendBuff[i] = (union ncclLLFifoLine*)conn->buffs[NCCL_PROTO_LL];
    sendStep[i] = conn->step;
    if (wid == i) sendConn = conn;
  }
  __device__ __forceinline__ /* 中文内部逻辑 F0182：loadSendSync
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(tid < fan.nsend())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：sendConnHeadPtr = sendConn->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     4. 调用 / 计算 / 写入：sendConnHeadCache = *sendConnHeadPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     5. 调用 / 计算 / 写入：sendConnHead = sendConn->step;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     6. 调用 / 计算 / 写入：sendConnFifoPtr = sendConn->sizesFifo;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0182
 */
/* 中文逐项-函数 F0182：loadSendSync
 * 用途与位置：为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives。
 * 本体直接出现的调用/包装：fan.nsend。
 * 需要对照的边界/推进条件：tid < fan.nsend。各条件所属分支见紧邻函数体。
 */void loadSendSync() {
    if (tid < fan.nsend()) {
      sendConnHeadPtr = sendConn->head;
      sendConnHeadCache = *sendConnHeadPtr;
      sendConnHead = sendConn->step;
      sendConnFifoPtr = sendConn->sizesFifo;
    }
  }

 public:
  __device__  /* 中文内部逻辑 F0183：Primitives
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：auto *channel = &ncclShmem.channel;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int nrecv=0, nsend=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 ；继续条件 (nrecv < MaxRecv && recvPeers[nrecv] >= 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 调用 / 计算 / 写入：loadRecvConn(&channel->devPeers[recvPeers[nrecv]].recv->conn, nrecv);；从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。
 *   5. 调用 / 计算 / 写入：nrecv++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 循环：初始 ；继续条件 (nsend < MaxSend && sendPeers[nsend] >= 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   8. 调用 / 计算 / 写入：loadSendConn(&channel->devPeers[sendPeers[nsend]].send->conn, nsend);；从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。
 *   9. 调用 / 计算 / 写入：nsend++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 调用 / 计算 / 写入：this->fan = Fan(nrecv, nsend);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：loadRecvSync();；为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。
 * 13. 调用 / 计算 / 写入：loadSendSync();；为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。
 * 14. 调用 / 计算 / 写入：setDataPtrs(inputBuf, outputBuf);；更新本原语的本地输入和输出缓冲指针；Simple 的 direct 路径还需完成对应地址交换。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0183
 */
/* 中文逐项-函数 F0183：Primitives
 * 用途与位置：为一个线程组建立原语上下文：实际邻居度、协议缓冲、线程角色、同步编号及用户数据指针。
 * 参数 tid（const int tid）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 参数 nthreads（const int nthreads）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 参数 recvPeers（int const *recvPeers）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 sendPeers（int const *sendPeers）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 inputBuf（void const *inputBuf）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 outputBuf（void *outputBuf）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 redOpArg（uint64_t redOpArg）：归约标量的位模式容器或设备指针，需结合对应 IsPtr 标志理解。
 * 参数 group（int group=0）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：Fan；loadRecvConn；loadRecvSync；loadSendConn；loadSendSync；setDataPtrs。
 * 需要对照的边界/推进条件：nrecv < MaxRecv && recvPeers[nrecv] >= 0；nsend < MaxSend && sendPeers[nsend] >= 0。各条件所属分支见紧邻函数体。
 */Primitives(
      const int tid, const int nthreads, int const *recvPeers, int const *sendPeers,
      void const *inputBuf, void *outputBuf, uint64_t redOpArg, int group=0
    ):
    redOp(redOpArg),
    tid(tid), nthreads(nthreads), wid(tid%WARP_SIZE), group(group),
    stepLines(ncclShmem.comm.buffSizes[NCCL_PROTO_LL]/NCCL_STEPS/sizeof(ncclLLFifoLine)) {

/* 中文逐项-变量 V0548：channel
 * channel（auto；函数 Primitives 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = &ncclShmem.channel；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    auto *channel = &ncclShmem.channel;
    // If we are going to support oneshot collNet + LL, then we would need to add connector index here
/* 中文逐项-变量 V0549：nrecv、nsend
 * nrecv（int；函数 Primitives 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nrecv = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * nsend（int；函数 Primitives 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nsend = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int nrecv=0, nsend=0;
    while (nrecv < MaxRecv && recvPeers[nrecv] >= 0) {
      loadRecvConn(&channel->devPeers[recvPeers[nrecv]].recv->conn, nrecv);
      nrecv++;
    }
    while (nsend < MaxSend && sendPeers[nsend] >= 0) {
      loadSendConn(&channel->devPeers[sendPeers[nsend]].send->conn, nsend);
      nsend++;
    }
    this->fan = Fan(nrecv, nsend);
    loadRecvSync();
    loadSendSync();
    setDataPtrs(inputBuf, outputBuf);
  }

  __device__ /* 中文内部逻辑 F0184：~Primitives
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(tid >= nthreads-WARP_SIZE && wid < fan.nrecv())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：recvConn->step = recvConnHead;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 4. 条件判断：(tid < fan.nsend())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：sendConn->step = sendConnHead;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 7. 调用 / 计算 / 写入：barrier();；同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0184
 */
/* 中文逐项-函数 F0184：~Primitives
 * 用途与位置：把本次推进后的 step 写回长期连接并按协议要求同步，后续操作继续复用同一连接。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：barrier；fan.nrecv；fan.nsend。
 * 需要对照的边界/推进条件：tid >= nthreads-WARP_SIZE && wid < fan.nrecv；tid < fan.nsend。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */~Primitives() {
    // Save steps for the next operation
    if (tid >= nthreads-WARP_SIZE && wid < fan.nrecv())
      recvConn->step = recvConnHead;
    if (tid < fan.nsend())
      sendConn->step = sendConnHead;
    // Ensure all steps written back
    barrier();
  }

  __device__ /* 中文内部逻辑 F0185：setDataPtrs
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：userBufs[Input] = (T*)inputBuf;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：userBufs[Output] = (T*)outputBuf;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0185
 */
/* 中文逐项-函数 F0185：setDataPtrs
 * 用途与位置：更新本原语的本地输入和输出缓冲指针；Simple 的 direct 路径还需完成对应地址交换。
 * 参数 inputBuf（void const *inputBuf）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 outputBuf（void *outputBuf）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */void setDataPtrs(void const *inputBuf, void *outputBuf) {
    userBufs[Input] = (T*)inputBuf;
    userBufs[Output] = (T*)outputBuf;
  }

  __device__ /* 中文内部逻辑 F0186：moveDataPtrs
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：userBufs[Input] += delta;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：userBufs[Output] += delta;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0186
 */
/* 中文逐项-函数 F0186：moveDataPtrs
 * 用途与位置：按元素偏移移动本原语的用户输入/输出指针，复用连接与协议状态处理另一段数据。
 * 参数 delta（intptr_t delta）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */void moveDataPtrs(intptr_t delta) {
    userBufs[Input] += delta;
    userBufs[Output] += delta;
  }

  __device__ /* 中文内部逻辑 F0187：send
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return LLGenericOp<0, 1, Input, -1>(inpIx, -1, eltN, false);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0187
 */
/* 中文逐项-函数 F0187：send
 * 用途与位置：从本地输入读取并发送到原语的发送邻居，不要求写本地输出。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketProgressOpt。
 * 本体直接出现的调用/包装：LLGenericOp<0, 1, Input, -1>。
 * 源码返回表达式：return LLGenericOp<0, 1, Input, -1>(inpIx, -1, eltN, false);。CHECK类宏还可能提前返回/跳转。
 */void send(intptr_t inpIx, int eltN) {
    return LLGenericOp<0, 1, Input, -1>(inpIx, -1, eltN, false);
  }
  __device__ /* 中文内部逻辑 F0188：sendFromOutput
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return LLGenericOp<0, 1, Output, -1>(outIx, -1, eltN, false);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0188
 */
/* 中文逐项-函数 F0188：sendFromOutput
 * 用途与位置：从已生成的本地输出读数据并发送，避免错误地重新读取原始输入。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：LLGenericOp<0, 1, Output, -1>。
 * 源码返回表达式：return LLGenericOp<0, 1, Output, -1>(outIx, -1, eltN, false);。CHECK类宏还可能提前返回/跳转。
 */void sendFromOutput(intptr_t outIx, int eltN) {
    return LLGenericOp<0, 1, Output, -1>(outIx, -1, eltN, false);
  }
  __device__ /* 中文内部逻辑 F0189：recv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return LLGenericOp<1, 0, -1, Output>(-1, outIx, eltN, postOp);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0189
 */
/* 中文逐项-函数 F0189：recv
 * 用途与位置：接收上游数据并写本地输出，本动作不加上本地输入。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketProgressOpt。
 * 本体直接出现的调用/包装：LLGenericOp<1, 0, -1, Output>。
 * 源码返回表达式：return LLGenericOp<1, 0, -1, Output>(-1, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void recv(intptr_t outIx, int eltN, bool postOp=false) {
    return LLGenericOp<1, 0, -1, Output>(-1, outIx, eltN, postOp);
  }
  __device__ /* 中文内部逻辑 F0190：recvReduceSend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return LLGenericOp<1, 1, Input, -1>(inpIx, -1, eltN, false);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0190
 */
/* 中文逐项-函数 F0190：recvReduceSend
 * 用途与位置：把上游部分结果与本地输入归约后发下游，不要求保留本地最终输出。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：LLGenericOp<1, 1, Input, -1>。
 * 源码返回表达式：return LLGenericOp<1, 1, Input, -1>(inpIx, -1, eltN, false);。CHECK类宏还可能提前返回/跳转。
 */void recvReduceSend(intptr_t inpIx, int eltN) {
    return LLGenericOp<1, 1, Input, -1>(inpIx, -1, eltN, false);
  }
  __device__ /* 中文内部逻辑 F0191：recvReduceCopy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return LLGenericOp<1, 0, Input, Output>(inpIx, outIx, eltN, postOp);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0191
 */
/* 中文逐项-函数 F0191：recvReduceCopy
 * 用途与位置：把接收数据与本地输入归约后写本地输出，可在结果完成时执行 postOp。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：LLGenericOp<1, 0, Input, Output>。
 * 源码返回表达式：return LLGenericOp<1, 0, Input, Output>(inpIx, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void recvReduceCopy(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    return LLGenericOp<1, 0, Input, Output>(inpIx, outIx, eltN, postOp);
  }
  __device__ /* 中文内部逻辑 F0192：copySend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return LLGenericOp<0, 1, Input, Output>(inpIx, outIx, eltN, postOp);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0192
 */
/* 中文逐项-函数 F0192：copySend
 * 用途与位置：把本地输入同时写本地输出并发下游，把本地保存和转发合并一次数据加载。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：LLGenericOp<0, 1, Input, Output>。
 * 源码返回表达式：return LLGenericOp<0, 1, Input, Output>(inpIx, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void copySend(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    return LLGenericOp<0, 1, Input, Output>(inpIx, outIx, eltN, postOp);
  }
  __device__ /* 中文内部逻辑 F0193：recvCopySend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return LLGenericOp<1, 1, -1, Output>(-1, outIx, eltN, postOp);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0193
 */
/* 中文逐项-函数 F0193：recvCopySend
 * 用途与位置：把已接收结果同时保存本地输出并转发下游，不再加入本地输入做归约。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：LLGenericOp<1, 1, -1, Output>。
 * 源码返回表达式：return LLGenericOp<1, 1, -1, Output>(-1, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void recvCopySend(intptr_t outIx, int eltN, bool postOp=false) {
    return LLGenericOp<1, 1, -1, Output>(-1, outIx, eltN, postOp);
  }
  __device__ /* 中文内部逻辑 F0194：recvReduceCopySend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return LLGenericOp<1, 1, Input, Output>(inpIx, outIx, eltN, postOp);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__device__prims_ll_h.md#f0194
 */
/* 中文逐项-函数 F0194：recvReduceCopySend
 * 用途与位置：把接收与本地输入融合归约，同时保存输出并转发，衔接归约与结果传播阶段。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：LLGenericOp<1, 1, Input, Output>。
 * 源码返回表达式：return LLGenericOp<1, 1, Input, Output>(inpIx, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void recvReduceCopySend(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    return LLGenericOp<1, 1, Input, Output>(inpIx, outIx, eltN, postOp);
  }
};
