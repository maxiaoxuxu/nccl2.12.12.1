/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "op128.h"

/* 中文精读：LL128 如何提高有效载荷比例
 * 每 128B 线包含 16 个 uint64_t，15 个数据元素和 1 个进度 flag，有效载荷 120/128。
 * 指定线程负责 flag，其余线程与之协作，把寄存器、warp shuffle 和共享内存暂存结合起来搬运。
 * 相较 LL 减少标记开销；适用性仍受硬件写入顺序/原子性和模型允许组合限制。
 * 名称 LL128 的 128 指协议行的字节格式，不是 128 个 rank、channel 或 RDMA QP。
 */
/* 中文逐项-常量 X0013：NCCL_LL128_FLAGTHREAD
 * 编译期定义：NCCL_LL128_FLAGTHREAD = (NCCL_LL128_LINEELEMS-1)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_LL128_FLAGTHREAD (NCCL_LL128_LINEELEMS-1)

/* 中文逐项-类型 T0076：Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>
 * 类型职责：当前协议的GPU通信原语实现/适配层，组合连接流控、数据地址、复制归约和进度发布。
 * 声明内可见成员（含嵌套成员）：MaxRecv、MaxSend、Input、Output、redOp、tid、nthreads、wid、stepSize、warp、flagThread、group、fan、userBufs、recvConn、recvConnHeadPtr、recvConnHead、sendConn、sendConnFifoPtr、sendConnTailPtr、sendConnTail、sendConnHeadPtr、sendConnHead、sendConnHeadCache、recvStep、sendStep、recvBuff、sendBuff、abort、WireWordPerSlice、DataEltPerSlice。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
template<typename T, typename RedOp, typename Fan, int Direct, int P2p>
class Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>:
  public PrimitivesWithoutDirect<Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>> {

/* 中文逐项-变量 V0550：MaxRecv、MaxSend
 * MaxRecv（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 * MaxSend（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：原语编译期最大接收/发送邻居度，用于数组容量和循环展开。
 */
  static constexpr int MaxRecv = Fan::MaxRecv, MaxSend = Fan::MaxSend;
/* 中文逐项-变量 V0551：Input、Output
 * Input（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：用户缓冲表的编译期输入/输出槽下标，不是发送/接收rank。
 * Output（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：用户缓冲表的编译期输入/输出槽下标，不是发送/接收rank。
 */
  static constexpr int Input=0, Output=1;
/* 中文逐项-变量 V0552：redOp
 * redOp（RedOp；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：归约/元素类型选择，决定设备数值运算与 count 到字节数的换算。
 */
  RedOp redOp;
/* 中文逐项-变量 V0553：tid
 * tid（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 */
  const int tid;
/* 中文逐项-变量 V0554：nthreads
 * nthreads（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 */
  const int nthreads;
/* 中文逐项-变量 V0555：wid
 * wid（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：线程在 warp 内的 lane 编号或代码定义的 warp 索引，必须以本处赋值为准。
 */
  const int wid;
/* 中文逐项-变量 V0556：stepSize
 * stepSize（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：一 FIFO step 的容量；设备 T 指针路径以元素计，主机缓冲路径以字节计。
 */
  const int stepSize;
/* 中文逐项-变量 V0557：warp
 * warp（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：当前线程所属 warp/warp 内分工相关编号，供 LL128 数据与 flag 协作。
 */
  const int warp;
/* 中文逐项-变量 V0558：flagThread
 * flagThread（bool；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：LL128 中承担行尾 flag 读写的线程标志，载荷线程按不同布局协作。
 */
  const bool flagThread;
/* 中文逐项-变量 V0559：group
 * group（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 */
  const int group;
/* 中文逐项-变量 V0560：fan
 * fan（Fan；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：原语实际收/发邻居数量，受编译期 MaxRecv/MaxSend 限制。
 */
  Fan fan;
/* 中文逐项-变量 V0561：userBufs
 * userBufs（T；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：本地输入与输出指针表，原语组合按 Input/Output 选择访问源/目标。
 */
  T *userBufs[2];
/* 中文逐项-变量 V0562：recvConn
 * recvConn（struct ncclConnInfo；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：负责接收/发送同步线程持有的设备连接视图，保存跨操作持续
 * step。
 */
  struct ncclConnInfo* recvConn = NULL;
/* 中文逐项-变量 V0563：recvConnHeadPtr
 * recvConnHeadPtr（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：指向接收消费、发送空间或发送发布计数的共享控制地址。
 */
  volatile uint64_t* recvConnHeadPtr = NULL;
/* 中文逐项-变量 V0564：recvConnHead
 * recvConnHead（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：同步角色缓存/推进的绝对进度，只有完成对应阶段才可发布。
 */
  uint64_t recvConnHead;

/* 中文逐项-变量 V0565：sendConn
 * sendConn（struct ncclConnInfo；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：负责接收/发送同步线程持有的设备连接视图，保存跨操作持续
 * step。
 */
  struct ncclConnInfo* sendConn = NULL;
/* 中文逐项-变量 V0566：sendConnFifoPtr
 * sendConnFifoPtr（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128,
 * P2p>）：同步发送线程缓存的sizesFifo地址，向Proxy报告本step协议物理字节数。
 */
  volatile int* sendConnFifoPtr = NULL;
/* 中文逐项-变量 V0567：sendConnTailPtr
 * sendConnTailPtr（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：指向接收消费、发送空间或发送发布计数的共享控制地址。
 */
  volatile uint64_t* sendConnTailPtr = NULL;
/* 中文逐项-变量 V0568：sendConnTail
 * sendConnTail（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：同步角色缓存/推进的绝对进度，只有完成对应阶段才可发布。
 */
  uint64_t sendConnTail;
/* 中文逐项-变量 V0569：sendConnHeadPtr
 * sendConnHeadPtr（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：指向接收消费、发送空间或发送发布计数的共享控制地址。
 */
  volatile uint64_t* sendConnHeadPtr = NULL;
/* 中文逐项-变量 V0570：sendConnHead
 * sendConnHead（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：同步角色缓存/推进的绝对进度，只有完成对应阶段才可发布。
 */
  uint64_t sendConnHead;
/* 中文逐项-变量 V0571：sendConnHeadCache
 * sendConnHeadCache（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：同步角色缓存/推进的绝对进度，只有完成对应阶段才可发布。
 */
  uint64_t sendConnHeadCache; // Cache last seen value

/* 中文逐项-变量 V0572：recvStep
 * recvStep（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：各邻居已推进的长期接收/发送序号，支持循环槽和行内标记复用。
 */
  uint64_t recvStep[MaxRecv];
/* 中文逐项-变量 V0573：sendStep
 * sendStep（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：各邻居已推进的长期接收/发送序号，支持循环槽和行内标记复用。
 */
  uint64_t sendStep[MaxSend];
/* 中文逐项-变量 V0574：recvBuff
 * recvBuff（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：各邻居的协议缓冲指针，按当前 step 计算实际槽位置。
 */
  uint64_t* recvBuff[MaxRecv];
/* 中文逐项-变量 V0575：sendBuff
 * sendBuff（uint64_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：各邻居的协议缓冲指针，按当前 step 计算实际槽位置。
 */
  uint64_t* sendBuff[MaxSend];

/* 中文逐项-函数 F0195：recvOffset
 * 用途与位置：用连接 step 对 NCCL_STEPS 取模再乘每槽大小，把单调进度映射到循环数据槽位。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::recvPtr；src/collectives/device/prims_ll128.h::recvPtr。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (recvStep[i]%NCCL_STEPS)*stepSize;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ int recvOffset(int i) { return (recvStep[i]%NCCL_STEPS)*stepSize; }
/* 中文逐项-函数 F0196：sendOffset
 * 用途与位置：用连接 step 对 NCCL_STEPS 取模再乘每槽大小，把单调进度映射到循环数据槽位。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::sendPtr；src/collectives/device/prims_ll128.h::sendPtr。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (sendStep[i]%NCCL_STEPS)*stepSize;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ int sendOffset(int i) { return (sendStep[i]%NCCL_STEPS)*stepSize; }
/* 中文逐项-函数 F0197：recvPtr
 * 用途与位置：以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::readLL；src/collectives/device/prims_ll.h::readLLBeginAll；src/collectives/device/prims_ll.h::readLLFinish；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体直接出现的调用/包装：recvOffset。
 * 源码返回表达式：return recvBuff[i]+recvOffset(i);。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ uint64_t* recvPtr(int i) { return recvBuff[i]+recvOffset(i); }
/* 中文逐项-函数 F0198：sendPtr
 * 用途与位置：以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::incSend；src/collectives/device/prims_ll.h::LLGenericOp；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体直接出现的调用/包装：sendOffset。
 * 源码返回表达式：return sendBuff[i]+sendOffset(i);。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ uint64_t* sendPtr(int i) { return sendBuff[i]+sendOffset(i); }
/* 中文逐项-函数 F0199：recvFlag
 * 用途与位置：把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::readLL；src/collectives/device/prims_ll.h::readLLFinish；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return recvStep[i]+1;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ uint64_t recvFlag(int i) { return recvStep[i]+1; }
/* 中文逐项-函数 F0200：sendFlag
 * 用途与位置：把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::incSend；src/collectives/device/prims_ll.h::LLGenericOp；src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return sendStep[i]+1;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ uint64_t sendFlag(int i) { return sendStep[i]+1; }

/* 中文逐项-函数 F0201：barrier
 * 用途与位置：同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::waitSend；src/collectives/device/prims_ll.h::postRecv；src/collectives/device/prims_ll.h::~Primitives；src/collectives/device/prims_ll128.h::GenericOp；src/collectives/device/prims_ll128.h::~Primitives；src/collectives/device/prims_simple.h::subBarrier；src/collectives/device/prims_simple.h::genericOp；其余见调用清单。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
  inline __device__ void barrier() {
    asm volatile ("bar.sync %1, %0;" :: "r"(nthreads), "r"(15-group));
  }

/* 中文逐项-变量 V0576：abort
 * abort（uint32_t；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：线程本地缓存的中止状态，周期读取共享 abortFlag 后更新以退出自旋。
 */
  uint32_t abort = 0;

/* 中文逐项-函数 F0202：checkAbort
 * 用途与位置：自旋达到检查间隔时读取 communicator 中止标志，使异常情况下的协议等待可以退出。
 * 参数 <未命名参数>（int &spins）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 send（int send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::waitSend；src/collectives/device/prims_ll.h::readLL；src/collectives/device/prims_ll.h::readLLFinish；src/collectives/device/prims_ll128.h::waitSend；src/collectives/device/prims_ll128.h::recvReduceSendCopy；src/collectives/device/prims_simple.h::waitPeer；src/collectives/device/prims_simple.h::setDataPtrs。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：abort == 0 && spins == NCCL_SPINS_BEFORE_CHECK_ABORT。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return abort;。CHECK类宏还可能提前返回/跳转。
 */
  inline __device__ int checkAbort(int &spins, int i, int send) {
    spins++;
    if (abort == 0 && spins == NCCL_SPINS_BEFORE_CHECK_ABORT) {
      abort = *ncclShmem.comm.abortFlag;
      spins = 0;
    }
    return abort;
  }

/* 中文逐项-函数 F0203：waitSend
 * 用途与位置：等待发送缓冲仍有可复用槽位，并在存在 Proxy 时填写本轮物理字节数，防止覆盖 NIC/对端仍使用的数据。
 * 参数 nbytes（int nbytes）：当前消息或片段的字节规模；info.nBytes 对 AG/RS 还计入 nRanks 段总规模。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp；src/collectives/device/prims_ll128.h::GenericOp。
 * 本体直接出现的调用/包装：checkAbort。
 * 需要对照的边界/推进条件：sendConnHeadPtr；sendConnHeadCache + NCCL_STEPS < sendConnHead + 1；checkAbort(spins, wid,
 * 1；sendConnFifoPtr。各条件所属分支见紧邻函数体。
 */
  inline __device__ void waitSend(int nbytes) {
    if (sendConnHeadPtr) {
/* 中文逐项-变量 V0577：spins
 * spins（int；函数 waitSend 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int spins = 0;
      while (sendConnHeadCache + NCCL_STEPS < sendConnHead + 1) {
        sendConnHeadCache = *sendConnHeadPtr;
        if (checkAbort(spins, wid, 1)) break;
      }
      if (sendConnFifoPtr) {
        sendConnFifoPtr[sendStep[wid]%NCCL_STEPS] = nbytes;
      }
      sendConnHead += 1;
    }
  }

/* 中文逐项-函数 F0204：postRecv
 * 用途与位置：在数据已消费后发布接收完成进度，通知生产者可以回收相应循环槽位。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::LLGenericOp；src/collectives/device/prims_ll128.h::GenericOp。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：recvConnHeadPtr。各条件所属分支见紧邻函数体。
 */
  inline __device__ void postRecv() {
    if (recvConnHeadPtr) *recvConnHeadPtr = recvConnHead += 1;
  }
/* 中文逐项-函数 F0205：postSend
 * 用途与位置：在数据/flag 写入完成并满足协议 fence 条件后发布发送进度，通知后续消费者。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::GenericOp。
 * 本体直接出现的调用/包装：__threadfence。
 * 需要对照的边界/推进条件：sendConnTailPtr。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
  inline __device__ void postSend() {
    if (sendConnTailPtr) { __threadfence(); *sendConnTailPtr = sendConnTail += 1; }
  }

/* 中文逐项-函数 F0206：loadRegsBegin
 * 用途与位置：按对齐情况提前把用户数据加载进 LL128 寄存器，延后 warp 重排以隐藏访存延迟。
 * 参数 <未命名参数>（uint64_t(&regs)[WordPerThread]）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 src（T const *src）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::GenericOp。
 * 本体直接出现的调用/包装：__syncwarp；load128；loadShmemMisaligned128；reinterpret_cast<uint64_t*>；reinterpret_cast<uintptr_t>；shmemCvtPtr；storeShmem128；uintptr_t。
 * 需要对照的边界/推进条件：reinterpret_cast<uintptr_t>(src)%16 == 0；g < WordPerThread/2；!flagThread || g%2==0；ix*EltPer16B <
 * eltN。各条件所属分支见紧邻函数体。
 */
  template<int WordPerThread>
  __device__ __forceinline__ void loadRegsBegin(uint64_t(&regs)[WordPerThread], T const *src, int eltN) {
/* 中文逐项-变量 V0578：EltPer16B
 * EltPer16B（int；函数 loadRegsBegin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 EltPer16B = 16/sizeof(T)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    constexpr int EltPer16B = 16/sizeof(T);
    if(reinterpret_cast<uintptr_t>(src)%16 == 0) {
      /* We are aligned to 16 bytes, so load directly to registers no shmem.
       * Flag threads load half as much data which gets shuffled to the even
       * registers during Finish. The point of splitting into two phases is to
       * defer that shuffle, which incurs a dependency stall, until after other
       * memops are launched by the caller.
       */
      #pragma unroll
      for(/* 中文逐项-变量 V0579：g
 * g（int；函数 loadRegsBegin 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g < WordPerThread/2；循环体与增量表达式决定每轮如何推进。
 */int g=0; g < WordPerThread/2; g++) {
/* 中文逐项-变量 V0580：ix
 * ix（int；函数 loadRegsBegin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);
        if(!flagThread || g%2==0) {
          if(ix*EltPer16B < eltN)
            load128((uint64_t*)(src + ix*EltPer16B), regs[2*g+0], regs[2*g+1]);
        }
      }
    }
    else {
      // Not aligned. Stage the smallest 16 byte aligned region subsuming the
      // buffer into shmem.
/* 中文逐项-变量 V0581：misalignment
 * misalignment（int；函数 loadRegsBegin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 misalignment = reinterpret_cast<uintptr_t>(src) % 16；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      int misalignment = reinterpret_cast<uintptr_t>(src) % 16;
/* 中文逐项-变量 V0582：src8
 * src8（uint64_t；函数 loadRegsBegin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 src8 = reinterpret_cast<uint64_t*>(reinterpret_cast<uintptr_t>(src) &
 * -uintptr_t(16))；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      uint64_t *src8 = reinterpret_cast<uint64_t*>(reinterpret_cast<uintptr_t>(src) & -uintptr_t(16));
/* 中文逐项-变量 V0583：shm8
 * shm8（uint64_t；函数 loadRegsBegin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shm8 = shmemCvtPtr(ncclShmem.ll128warp[warp])；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      uint64_t *shm8 = shmemCvtPtr(ncclShmem.ll128warp[warp]);
      #pragma unroll
      for(/* 中文逐项-变量 V0584：g
 * g（int；函数 loadRegsBegin 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g < WordPerThread/2；循环体与增量表达式决定每轮如何推进。
 */int g=0; g < WordPerThread/2; g++)
        if((g*WARP_SIZE + wid)*16 < misalignment + eltN*sizeof(T))
          load128(src8 + 2*(g*WARP_SIZE + wid), regs[2*g+0], regs[2*g+1]);
      #pragma unroll
      for(/* 中文逐项-变量 V0585：g
 * g（int；函数 loadRegsBegin 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g < WordPerThread/2；循环体与增量表达式决定每轮如何推进。
 */int g=0; g < WordPerThread/2; g++)
        storeShmem128(shm8 + 2*(g*WARP_SIZE + wid), regs[2*g+0], regs[2*g+1]);

      __syncwarp();

      // Now load from shmem stage to regs. Preserve the same pre-shuffled layout
      // as the aligned case since Finish() will be applied regardless.
/* 中文逐项-变量 V0586：shm
 * shm（T；函数 loadRegsBegin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shm = (T*)shm8 + misalignment/sizeof(T)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
      T *shm = (T*)shm8 + misalignment/sizeof(T);
      #pragma unroll
      for(/* 中文逐项-变量 V0587：g
 * g（int；函数 loadRegsBegin 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g < WordPerThread/2；循环体与增量表达式决定每轮如何推进。
 */int g=0; g < WordPerThread/2; g++) {
/* 中文逐项-变量 V0588：ix
 * ix（int；函数 loadRegsBegin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);
        if(!flagThread || g%2==0) {
          if(ix*EltPer16B < eltN)
            loadShmemMisaligned128(shm + ix*EltPer16B, regs[2*g+0], regs[2*g+1]);
        }
      }
    }
  }

/* 中文逐项-函数 F0207：loadRegsFinish
 * 用途与位置：完成 LL128 寄存器重排与共享内存辅助加载，使数据适合带 flag 的行布局。
 * 参数 <未命名参数>（uint64_t(&regs)[WordPerThread]）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::recvReduceSendCopy。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：g < WordPerThread/2；flagThread。各条件所属分支见紧邻函数体。
 */
  template<int WordPerThread>
  __device__ __forceinline__ void loadRegsFinish(uint64_t(&regs)[WordPerThread]) {
    // Move data out of flag registers into the vacant registers.
    #pragma unroll
    for (/* 中文逐项-变量 V0589：g
 * g（int；函数 loadRegsFinish 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：g < WordPerThread/2；循环体与增量表达式决定每轮如何推进。
 */int g=1; g < WordPerThread/2; g+=2) {
      if (flagThread) regs[2*g] = regs[2*g-1];
    }
  }

/* 中文逐项-函数 F0208：storeRegs
 * 用途与位置：把寄存器中的 LL128 数据还原写入用户缓冲，处理对齐与有效元素范围。
 * 参数 dst（T *dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 <未命名参数>（uint64_t(&regs)[WordPerThread]）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::GenericOp。
 * 本体直接出现的调用/包装：__syncwarp；reinterpret_cast<uintptr_t>；shmemCvtPtr；store128；storeShmem128。
 * 需要对照的边界/推进条件：g < WordPerThread/2；flagThread；g < WordPerThread/2；!flagThread || g%2==0。各条件所属分支见紧邻函数体。
 */
  template<int WordPerThread>
  __device__ __forceinline__ void storeRegs(T *dst, uint64_t(&regs)[WordPerThread], int eltN) {
/* 中文逐项-变量 V0590：EltPer16B
 * EltPer16B（int；函数 storeRegs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 EltPer16B = 16/sizeof(T)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    constexpr int EltPer16B = 16/sizeof(T);
    // Reverse Finish() register permuatation.
    #pragma unroll
    for (/* 中文逐项-变量 V0591：g
 * g（int；函数 storeRegs 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：g < WordPerThread/2；循环体与增量表达式决定每轮如何推进。
 */int g=1; g < WordPerThread/2; g+=2) {
      if (flagThread) regs[2*g-1] = regs[2*g];
    }
    // Write to dst if 16-byte aligned, shmem otherwise.
/* 中文逐项-变量 V0592：misalignment
 * misalignment（int；函数 storeRegs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 misalignment = reinterpret_cast<uintptr_t>(dst)%16；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int misalignment = reinterpret_cast<uintptr_t>(dst)%16;
/* 中文逐项-变量 V0593：shm8
 * shm8（uint64_t；函数 storeRegs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shm8 = shmemCvtPtr(ncclShmem.ll128warp[warp])；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    uint64_t *shm8 = shmemCvtPtr(ncclShmem.ll128warp[warp]);
    #pragma unroll
    for(/* 中文逐项-变量 V0594：g
 * g（int；函数 storeRegs 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g < WordPerThread/2；循环体与增量表达式决定每轮如何推进。
 */int g=0; g < WordPerThread/2; g++) {
/* 中文逐项-变量 V0595：ix
 * ix（int；函数 storeRegs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);
      if (!flagThread || g%2==0) {
        if(misalignment == 0 && (ix+1)*EltPer16B <= eltN)
          store128((uint64_t*)(dst + ix*EltPer16B), regs[2*g+0], regs[2*g+1]);
        else
          storeShmem128(shm8+2*ix, regs[2*g+0], regs[2*g+1]);
      }
    }
    __syncwarp();
    // Write rest from shmem to dst. No need to coalesce stores to 16-bytes,
    // the hardware keeps up fine.
/* 中文逐项-变量 V0596：shm
 * shm（T；函数 storeRegs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shm = (T*)ncclShmem.ll128warp[warp]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    T *shm = (T*)ncclShmem.ll128warp[warp];
/* 中文逐项-变量 V0597：skip
 * skip（int；函数 storeRegs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 skip = misalignment == 0 ? eltN & -EltPer16B : 0；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int skip = misalignment == 0 ? eltN & -EltPer16B : 0;
    for(/* 中文逐项-变量 V0598：i
 * i（int；函数 storeRegs 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = skip+wid；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 * 所在for循环的继续条件：i < eltN；循环体与增量表达式决定每轮如何推进。
 */int i=skip+wid; i < eltN; i += WARP_SIZE)
      dst[i] = shm[i];
  }

/* 中文逐项-常量 X0014：WARP_MASK
 * 编译期定义：WARP_MASK = 0xffffffff。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
  #define WARP_MASK 0xffffffff

/* 中文逐项-函数 F0209：recvReduceSendCopy
 * 用途与位置：实现 LL128 的一轮接收、寄存器归约、行内标记发送和可选本地保存。
 * 参数 <未命名参数>（uint64_t(&v)[ELEMS_PER_THREAD]）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 ll128Offset（int ll128Offset）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 参数 postOp（bool postOp）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::GenericOp。
 * 本体直接出现的调用/包装：MULTI<RedOp, T>；MULTI<RedOp, T>()；MULTI<RedOp, T>().postOp；MULTI<RedOp,
 * T>().preOp；__any_sync；__syncwarp；checkAbort；fan.nrecv；fan.nsend；load128；loadRegsFinish；recvFlag；recvPtr；sendFlag；sendPtr；store128。
 * 需要对照的边界/推进条件：RECV；u<ELEMS_PER_THREAD；SRC；SrcBuf == Input。各条件所属分支见紧邻函数体。
 */
  template <int ELEMS_PER_THREAD, int RECV, int SEND, int SrcBuf, int DstBuf>
  __device__ __forceinline__ void recvReduceSendCopy(uint64_t(&v)[ELEMS_PER_THREAD], int ll128Offset, bool postOp) {
/* 中文逐项-变量 V0599：SRC
 * SRC（int；函数 recvReduceSendCopy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 SRC = SrcBuf != -1 ? 1 : 0；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    constexpr int SRC = SrcBuf != -1 ? 1 : 0;
/* 中文逐项-变量 V0600：vr
 * vr（uint64_t；函数 recvReduceSendCopy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：load128(ptr+u*WARP_SIZE, vr[u], vr[u+1]);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    uint64_t vr[ELEMS_PER_THREAD];

    __syncwarp();
    /************************ Wait first recv ********************/
    if (RECV) {
/* 中文逐项-变量 V0601：ptr
 * ptr（uint64_t；函数 recvReduceSendCopy 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = recvPtr(0)+ll128Offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      uint64_t* ptr = recvPtr(0)+ll128Offset;
/* 中文逐项-变量 V0602：flag
 * flag（uint64_t；函数 recvReduceSendCopy 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = recvFlag(0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      uint64_t flag = recvFlag(0);
/* 中文逐项-变量 V0603：needReload
 * needReload（bool；函数 recvReduceSendCopy 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 此处未显式给初值；后续首个文本使用点：needReload = false;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      bool needReload;
/* 中文逐项-变量 V0604：spins
 * spins（int；函数 recvReduceSendCopy 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int spins = 0;
      do {
        needReload = false;
        #pragma unroll
        for (/* 中文逐项-变量 V0605：u
 * u（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u<ELEMS_PER_THREAD；循环体与增量表达式决定每轮如何推进。
 */int u=0; u<ELEMS_PER_THREAD; u+=2) {
          load128(ptr+u*WARP_SIZE, vr[u], vr[u+1]);
          needReload |= flagThread && (vr[u+1] != flag);
        }
      } while (__any_sync(WARP_MASK, needReload) && checkAbort(spins, 0, 0) == 0);
    }

    /************* Finish register load **************/
    if (SRC) {
      // By deferring register shuffle here we've overlapped spinning on first
      // peer's data with memory loads of src data.
      loadRegsFinish(v);
      if (SrcBuf == Input) {
        #pragma unroll
        for (/* 中文逐项-变量 V0606：u
 * u（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u<ELEMS_PER_THREAD；循环体与增量表达式决定每轮如何推进。
 */int u=0; u<ELEMS_PER_THREAD; u+=2) {
          v[u] = MULTI<RedOp, T>().preOp(redOp, v[u]);
          if (!flagThread)
            v[u+1] = MULTI<RedOp, T>().preOp(redOp, v[u+1]);
        }
      }
    }

    /************************ Recv rest *********************/
    if (RECV) {
      { // Consume data from first recv
/* 中文逐项-变量 V0607：ptr
 * ptr（uint64_t；函数 recvReduceSendCopy 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = recvPtr(0)+ll128Offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        uint64_t* ptr = recvPtr(0)+ll128Offset;
        #pragma unroll
        for (/* 中文逐项-变量 V0608：u
 * u（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u<ELEMS_PER_THREAD；循环体与增量表达式决定每轮如何推进。
 */int u=0; u<ELEMS_PER_THREAD; u+=2) {
          v[u] = SRC ? MULTI<RedOp, T>()(redOp, vr[u], v[u]) : vr[u];
          v[u+1] = SRC ? MULTI<RedOp, T>()(redOp, vr[u+1], v[u+1]) : vr[u+1];
        }
      }

      for (/* 中文逐项-变量 V0609：i
 * i（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MaxRecv && i<fan.nrecv()；循环体与增量表达式决定每轮如何推进。
 */int i=1; i<MaxRecv && i<fan.nrecv(); i++) {
/* 中文逐项-变量 V0610：flag
 * flag（uint64_t；函数 recvReduceSendCopy 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = recvFlag(i)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        uint64_t flag = recvFlag(i);
/* 中文逐项-变量 V0611：ptr
 * ptr（uint64_t；函数 recvReduceSendCopy 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = recvPtr(i)+ll128Offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        uint64_t* ptr = recvPtr(i)+ll128Offset;
/* 中文逐项-变量 V0612：needReload
 * needReload（bool；函数 recvReduceSendCopy 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 此处未显式给初值；后续首个文本使用点：needReload = false;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        bool needReload;
/* 中文逐项-变量 V0613：spins
 * spins（int；函数 recvReduceSendCopy 内）：本次等待中的自旋计数，到阈值后检查中止，避免每轮都访问共享标志。
 * 初值/来源 spins = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
        int spins = 0;
        do {
          needReload = false;
          #pragma unroll
          for (/* 中文逐项-变量 V0614：u
 * u（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u<ELEMS_PER_THREAD；循环体与增量表达式决定每轮如何推进。
 */int u=0; u<ELEMS_PER_THREAD; u+=2) {
            load128(ptr+u*WARP_SIZE, vr[u], vr[u+1]);
            needReload |= flagThread && (vr[u+1] != flag);
          }
        } while (__any_sync(WARP_MASK, needReload) && checkAbort(spins, i, 0) == 0);

        #pragma unroll
        for (/* 中文逐项-变量 V0615：u
 * u（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u<ELEMS_PER_THREAD；循环体与增量表达式决定每轮如何推进。
 */int u=0; u<ELEMS_PER_THREAD; u+=2) {
          v[u] = MULTI<RedOp, T>()(redOp, vr[u], v[u]);
          v[u+1] = MULTI<RedOp, T>()(redOp, vr[u+1], v[u+1]);
        }
      }
    }
    /********************** End Recv ************************/

    if (postOp && !FuncTraits<RedOp>::IsPostOpIdentity) {
      #pragma unroll
      for (/* 中文逐项-变量 V0616：u
 * u（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u<ELEMS_PER_THREAD；循环体与增量表达式决定每轮如何推进。
 */int u=0; u<ELEMS_PER_THREAD; u+=2) {
        v[u]   = MULTI<RedOp, T>().postOp(redOp, v[u]);
        v[u+1] = MULTI<RedOp, T>().postOp(redOp, v[u+1]);
      }
    }

    /************************ Send **************************/
    if (SEND) {
      for (/* 中文逐项-变量 V0617：i
 * i（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<MaxSend && i<fan.nsend()；循环体与增量表达式决定每轮如何推进。
 */int i=1; i<MaxSend && i<fan.nsend(); i++) {
/* 中文逐项-变量 V0618：flag
 * flag（uint64_t；函数 recvReduceSendCopy 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = sendFlag(i)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        uint64_t flag = sendFlag(i);
/* 中文逐项-变量 V0619：ptr
 * ptr（uint64_t；函数 recvReduceSendCopy 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = sendPtr(i)+ll128Offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        uint64_t* ptr = sendPtr(i)+ll128Offset;
        #pragma unroll
        for (/* 中文逐项-变量 V0620：u
 * u（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u<ELEMS_PER_THREAD；循环体与增量表达式决定每轮如何推进。
 */int u=0; u<ELEMS_PER_THREAD; u+=2) {
          store128(ptr+u*WARP_SIZE, v[u], flagThread ? flag : v[u+1]);
        }
      }
/* 中文逐项-变量 V0621：flag
 * flag（uint64_t；函数 recvReduceSendCopy 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = sendFlag(0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      uint64_t flag = sendFlag(0);
/* 中文逐项-变量 V0622：ptr
 * ptr（uint64_t；函数 recvReduceSendCopy 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = sendPtr(0)+ll128Offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      uint64_t* ptr = sendPtr(0)+ll128Offset;
      #pragma unroll
      for (/* 中文逐项-变量 V0623：u
 * u（int；函数 recvReduceSendCopy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 u = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：u<ELEMS_PER_THREAD；循环体与增量表达式决定每轮如何推进。
 */int u=0; u<ELEMS_PER_THREAD; u+=2) {
        store128(ptr+u*WARP_SIZE, v[u], flagThread ? flag : v[u+1]);
      }
    }
    /********************** End Send ************************/
  }

/* 中文逐项-变量 V0624：WireWordPerSlice
 * WireWordPerSlice（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：协议行/片的有效元素或物理字数常量，区分标记开销和真正用户载荷。
 */
  static constexpr int WireWordPerSlice = WARP_SIZE*NCCL_LL128_SHMEM_ELEMS_PER_THREAD;
/* 中文逐项-变量 V0625：DataEltPerSlice
 * DataEltPerSlice（int；成员 Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>）：协议行/片的有效元素或物理字数常量，区分标记开销和真正用户载荷。
 */
  static constexpr int DataEltPerSlice = (WireWordPerSlice - WireWordPerSlice/NCCL_LL128_LINEELEMS)*(sizeof(uint64_t)/sizeof(T));

/* 中文逐项-函数 F0210：GenericOp
 * 用途与位置：根据本地源/目的和收发模板参数组织 LL128 处理循环，决定何时加载、归约、复制及发布。
 * 参数 srcIx（intptr_t srcIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 dstIx（intptr_t dstIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 nelem（int nelem）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll128.h::send；src/collectives/device/prims_ll128.h::sendFromOutput；src/collectives/device/prims_ll128.h::recv；src/collectives/device/prims_ll128.h::recvReduceSend；src/collectives/device/prims_ll128.h::recvReduceCopy；src/collectives/device/prims_ll128.h::copySend；src/collectives/device/prims_ll128.h::recvCopySend；其余见调用清单。
 * 本体直接出现的调用/包装：assert；barrier；divUp；loadRegsBegin；min；postRecv；postSend；recvReduceSendCopy<NCCL_LL128_SHMEM_ELEMS_PER_THREAD,
 * RECV, SEND, SrcBuf, DstBuf>；storeRegs；waitSend。
 * 需要对照的边界/推进条件：SEND；nelem > 0；SRC；DST。各条件所属分支见紧邻函数体。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
  template <int RECV, int SEND, int SrcBuf, int DstBuf>
/* 中文导读：
 * LL128 以 128 字节通信行为粒度，其中 15 个 64 位元素承载数据、1 个承载 flag。
 * 相比 LL 提高有效载荷比例，但依赖其访问/可见性假设；能否使用还受调优模型和平台条件限制。
 */
  __device__ __forceinline__ void GenericOp(intptr_t srcIx, intptr_t dstIx, int nelem, bool postOp) {
/* 中文逐项-变量 V0626：SRC
 * SRC（int；函数 GenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 SRC = SrcBuf != -1 ? 1 : 0；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    constexpr int SRC = SrcBuf != -1 ? 1 : 0;
/* 中文逐项-变量 V0627：DST
 * DST（int；函数 GenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 DST = DstBuf != -1 ? 1 : 0；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    constexpr int DST = DstBuf != -1 ? 1 : 0;
    static_assert(-1<=SrcBuf && SrcBuf < 2, "Uhoh");
    static_assert(-1<=DstBuf && DstBuf < 2, "Uhoh");
    static_assert(DstBuf!=Input, "Mistake?");
    #if 0
    assert((SrcBuf==-1) == (srcIx==-1));
    assert((DstBuf==-1) == (dstIx==-1));
    #endif

/* 中文逐项-变量 V0628：srcPtr
 * srcPtr（T；函数 GenericOp 内）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 初值/来源 srcPtr = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    T const *srcPtr = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx;
/* 中文逐项-变量 V0629：dstPtr
 * dstPtr（T；函数 GenericOp 内）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 初值/来源 dstPtr = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    T       *dstPtr = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx;
/* 中文逐项-变量 V0630：wireOffset
 * wireOffset（int；函数 GenericOp 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 wireOffset = WireWordPerSlice*warp + 2*wid；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int wireOffset = WireWordPerSlice*warp + 2*wid;
/* 中文逐项-变量 V0631：nwarps
 * nwarps（int；函数 GenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nwarps = nthreads/WARP_SIZE；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    const int nwarps = nthreads/WARP_SIZE;
    nelem = nelem < 0 ? 0 : nelem;

    if (SEND) waitSend(divUp(nelem, DataEltPerSlice)*WireWordPerSlice*sizeof(uint64_t));
    barrier();
    nelem -= DataEltPerSlice*warp;
    srcPtr += DataEltPerSlice*warp;
    dstPtr += DataEltPerSlice*warp;
    while (nelem > 0) {
/* 中文逐项-变量 V0632：eltInSlice
 * eltInSlice（int；函数 GenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 eltInSlice = min(nelem, DataEltPerSlice)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
      const int eltInSlice = min(nelem, DataEltPerSlice);
/* 中文逐项-变量 V0633：regs
 * regs（uint64_t；函数 GenericOp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (SRC) loadRegsBegin(regs, srcPtr, eltInSlice);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      uint64_t regs[NCCL_LL128_SHMEM_ELEMS_PER_THREAD];
      if (SRC) loadRegsBegin(regs, srcPtr, eltInSlice);
      recvReduceSendCopy<NCCL_LL128_SHMEM_ELEMS_PER_THREAD, RECV, SEND, SrcBuf, DstBuf>(regs, wireOffset, postOp);
      if (DST) storeRegs(dstPtr, regs, eltInSlice);

      wireOffset += WireWordPerSlice*nwarps;
      srcPtr += DataEltPerSlice*nwarps;
      dstPtr += DataEltPerSlice*nwarps;
      nelem -= DataEltPerSlice*nwarps;
    }

    barrier();
    if (SEND) for (/* 中文逐项-变量 V0634：i
 * i（int；函数 GenericOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < MaxSend；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < MaxSend; i++) sendStep[i] += 1;
    if (SEND) postSend();
    if (RECV) for (/* 中文逐项-变量 V0635：i
 * i（int；函数 GenericOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < MaxRecv；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < MaxRecv; i++) recvStep[i] += 1;
    if (RECV) postRecv();
  }

  __device__ __forceinline__ /* 中文逐项-函数 F0211：loadRecvConn
 * 用途与位置：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。
 * 参数 conn（struct ncclConnInfo* conn）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：wid == i。各条件所属分支见紧邻函数体。
 */void loadRecvConn(struct ncclConnInfo* conn, int i) {
    recvBuff[i] = (uint64_t*)conn->buffs[NCCL_PROTO_LL128];
    recvStep[i] = conn->step;
    if (wid == i) recvConn = conn;
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0212：loadRecvSync
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

  __device__ __forceinline__ /* 中文逐项-函数 F0213：loadSendConn
 * 用途与位置：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。
 * 参数 conn（struct ncclConnInfo* conn）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 i（int i）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：wid == i。各条件所属分支见紧邻函数体。
 */void loadSendConn(struct ncclConnInfo* conn, int i) {
    sendBuff[i] = (uint64_t*)conn->buffs[NCCL_PROTO_LL128];
    sendStep[i] = conn->step;
    if (wid == i) sendConn = conn;
  }
  __device__ __forceinline__ /* 中文逐项-函数 F0214：loadSendSync
 * 用途与位置：为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives。
 * 本体直接出现的调用/包装：fan.nsend。
 * 需要对照的边界/推进条件：tid < fan.nsend；tid >= nthreads-WARP_SIZE && wid<fan.nsend；sendConn->sizesFifo。各条件所属分支见紧邻函数体。
 */void loadSendSync() {
    if (tid < fan.nsend()) {
      sendConnHeadPtr = sendConn->head;
      sendConnHeadCache = *sendConnHeadPtr;
      sendConnHead = sendConn->step;
      sendConnFifoPtr = sendConn->sizesFifo;
    }
    if (tid >= nthreads-WARP_SIZE && wid<fan.nsend()) {
      if (sendConn->sizesFifo) {
        sendConnTailPtr = sendConn->tail;
        sendConnTail = sendConn->step;
      }
    }
  }

public:
  __device__ /* 中文逐项-函数 F0215：Primitives
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
    tid(tid), nthreads(nthreads), wid(tid%WARP_SIZE), warp(tid/WARP_SIZE),
    flagThread((tid%8)==7), group(group),
    stepSize(ncclShmem.comm.buffSizes[NCCL_PROTO_LL128]/NCCL_STEPS/sizeof(uint64_t)) {

/* 中文逐项-变量 V0636：channel
 * channel（auto；函数 Primitives 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = &ncclShmem.channel；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    auto *channel = &ncclShmem.channel;
/* 中文逐项-变量 V0637：nrecv、nsend
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

  __device__ /* 中文逐项-函数 F0216：~Primitives
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

  __device__ /* 中文逐项-函数 F0217：setDataPtrs
 * 用途与位置：更新本原语的本地输入和输出缓冲指针；Simple 的 direct 路径还需完成对应地址交换。
 * 参数 inputBuf（void const *inputBuf）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 outputBuf（void *outputBuf）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/device/prims_ll.h::Primitives；src/collectives/device/prims_ll128.h::Primitives；src/collectives/device/prims_simple.h::Primitives。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */void setDataPtrs(void const *inputBuf, void *outputBuf) {
    userBufs[Input] = (T*)inputBuf;
    userBufs[Output] = (T*)outputBuf;
  }

  __device__ /* 中文逐项-函数 F0218：moveDataPtrs
 * 用途与位置：按元素偏移移动本原语的用户输入/输出指针，复用连接与协议状态处理另一段数据。
 * 参数 delta（intptr_t delta）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */void moveDataPtrs(intptr_t delta) {
    userBufs[Input] += delta;
    userBufs[Output] += delta;
  }

  __device__ /* 中文逐项-函数 F0219：send
 * 用途与位置：从本地输入读取并发送到原语的发送邻居，不要求写本地输出。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketProgressOpt。
 * 本体直接出现的调用/包装：GenericOp<0, 1, Input, -1>。
 * 源码返回表达式：return GenericOp<0, 1, Input, -1>(inpIx, -1, eltN, false);。CHECK类宏还可能提前返回/跳转。
 */void send(intptr_t inpIx, int eltN) {
    return GenericOp<0, 1, Input, -1>(inpIx, -1, eltN, false);
  }
  __device__ /* 中文逐项-函数 F0220：sendFromOutput
 * 用途与位置：从已生成的本地输出读数据并发送，避免错误地重新读取原始输入。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GenericOp<0, 1, Output, -1>。
 * 源码返回表达式：return GenericOp<0, 1, Output, -1>(outIx, -1, eltN, false);。CHECK类宏还可能提前返回/跳转。
 */void sendFromOutput(intptr_t outIx, int eltN) {
    return GenericOp<0, 1, Output, -1>(outIx, -1, eltN, false);
  }
  __device__ /* 中文逐项-函数 F0221：recv
 * 用途与位置：接收上游数据并写本地输出，本动作不加上本地输入。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketProgressOpt。
 * 本体直接出现的调用/包装：GenericOp<1, 0, -1, Output>。
 * 源码返回表达式：return GenericOp<1, 0, -1, Output>(-1, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void recv(intptr_t outIx, int eltN, bool postOp=false) {
    return GenericOp<1, 0, -1, Output>(-1, outIx, eltN, postOp);
  }
  __device__ /* 中文逐项-函数 F0222：recvReduceSend
 * 用途与位置：把上游部分结果与本地输入归约后发下游，不要求保留本地最终输出。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GenericOp<1, 1, Input, -1>。
 * 源码返回表达式：return GenericOp<1, 1, Input, -1>(inpIx, -1, eltN, false);。CHECK类宏还可能提前返回/跳转。
 */void recvReduceSend(intptr_t inpIx, int eltN) {
    return GenericOp<1, 1, Input, -1>(inpIx, -1, eltN, false);
  }
  __device__ /* 中文逐项-函数 F0223：recvReduceCopy
 * 用途与位置：把接收数据与本地输入归约后写本地输出，可在结果完成时执行 postOp。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GenericOp<1, 0, Input, Output>。
 * 源码返回表达式：return GenericOp<1, 0, Input, Output>(inpIx, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void recvReduceCopy(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    return GenericOp<1, 0, Input, Output>(inpIx, outIx, eltN, postOp);
  }
  __device__ /* 中文逐项-函数 F0224：copySend
 * 用途与位置：把本地输入同时写本地输出并发下游，把本地保存和转发合并一次数据加载。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GenericOp<0, 1, Input, Output>。
 * 源码返回表达式：return GenericOp<0, 1, Input, Output>(inpIx, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void copySend(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    return GenericOp<0, 1, Input, Output>(inpIx, outIx, eltN, postOp);
  }
  __device__ /* 中文逐项-函数 F0225：recvCopySend
 * 用途与位置：把已接收结果同时保存本地输出并转发下游，不再加入本地输入做归约。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GenericOp<1, 1, -1, Output>。
 * 源码返回表达式：return GenericOp<1, 1, -1, Output>(-1, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void recvCopySend(intptr_t outIx, int eltN, bool postOp=false) {
    return GenericOp<1, 1, -1, Output>(-1, outIx, eltN, postOp);
  }
  __device__ /* 中文逐项-函数 F0226：recvReduceCopySend
 * 用途与位置：把接收与本地输入融合归约，同时保存输出并转发，衔接归约与结果传播阶段。
 * 参数 inpIx（intptr_t inpIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 outIx（intptr_t outIx）：原语访问本地输入/输出或直接远端用户缓冲的元素下标，乘 sizeof(T) 才是字节偏移。
 * 参数 eltN（int eltN）：本轮有效数据元素数，尾块可小于对齐后的 chunk 容量。
 * 参数 postOp（bool postOp=false）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GenericOp<1, 1, Input, Output>。
 * 源码返回表达式：return GenericOp<1, 1, Input, Output>(inpIx, outIx, eltN, postOp);。CHECK类宏还可能提前返回/跳转。
 */void recvReduceCopySend(intptr_t inpIx, intptr_t outIx, int eltN, bool postOp=false) {
    return GenericOp<1, 1, Input, Output>(inpIx, outIx, eltN, postOp);
  }
};
