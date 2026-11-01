/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_COMM_H_
/* 中文逐项-常量 X0170：NCCL_COMM_H_
 * 编译期定义：NCCL_COMM_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_COMM_H_

#include "transport.h"
#include "p2p.h"
#include "collectives.h"

#if CUDART_VERSION < 9000
/* 中文逐项-类型 T0164：cudaLaunchParams
 * 类型职责：CUDA发射入口、grid/block、参数地址、共享内存和stream参数包，旧CUDA版本由本库提供兼容定义。
 * 声明内可见成员（含嵌套成员）：func、gridDim、blockDim、args、sharedMem、stream。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct cudaLaunchParams {
/* 中文逐项-变量 V1989：func
 * func（void；成员 cudaLaunchParams）：函数指针或 CUDA kernel 入口地址，由所选算法/函数表/解析器回调决定。
 */
  void *func;
/* 中文逐项-变量 V1990：gridDim
 * gridDim（dim3；成员 cudaLaunchParams）：CUDA 发射的 block 网格和每 block 线程配置；通信路径常每 channel 对应一个 block。
 */
  dim3 gridDim;
/* 中文逐项-变量 V1991：blockDim
 * blockDim（dim3；成员 cudaLaunchParams）：CUDA 发射的 block 网格和每 block 线程配置；通信路径常每 channel 对应一个 block。
 */
  dim3 blockDim;
/* 中文逐项-变量 V1992：args
 * args（void；成员 cudaLaunchParams）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 */
  void **args;
/* 中文逐项-变量 V1993：sharedMem
 * sharedMem（size_t；成员 cudaLaunchParams）：本次 kernel 动态共享内存字节数，存元数据和协议小暂存而非完整张量。
 */
  size_t sharedMem;
/* 中文逐项-变量 V1994：stream
 * stream（cudaStream_t；成员 cudaLaunchParams）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 */
  cudaStream_t stream;
};
#endif

/* 中文逐项-常量 X0171：CACHE_LINE_SIZE
 * 编译期定义：CACHE_LINE_SIZE = 128。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define CACHE_LINE_SIZE 128
/* 中文逐项-常量 X0172：MEM_ALIGN
 * 编译期定义：MEM_ALIGN = 4096。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MEM_ALIGN 4096
/* 中文逐项-常量 X0173：CUDA_IPC_MIN
 * 编译期定义：CUDA_IPC_MIN = 2097152UL。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define CUDA_IPC_MIN 2097152UL

// Channels / LL tuning
/* 中文逐项-常量 X0174：NCCL_LL_THREAD_THRESHOLD
 * 编译期定义：NCCL_LL_THREAD_THRESHOLD = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_LL_THREAD_THRESHOLD 8
/* 中文逐项-常量 X0175：NCCL_LL128_THREAD_THRESHOLD
 * 编译期定义：NCCL_LL128_THREAD_THRESHOLD = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_LL128_THREAD_THRESHOLD 8
/* 中文逐项-常量 X0176：NCCL_SIMPLE_THREAD_THRESHOLD
 * 编译期定义：NCCL_SIMPLE_THREAD_THRESHOLD = 64。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SIMPLE_THREAD_THRESHOLD 64

/* 中文导读：
 * 发送方向的控制区。head 表示消费者已经释放的进度，生产者据此判断环形槽位能否复用。
 * 它与 ncclRecvMem、协议数据缓冲区共同组成 GPU/对端/Proxy 之间的流控接口。
 */
/* 中文逐项-类型 T0165：ncclSendMem
 * 类型职责：连接控制内存布局，分别提供消费head或生产tail及大小/偏移FIFO，与数据缓冲协作。
 * 声明内可见成员（含嵌套成员）：head、pad1、ptrExchange、redOpArgExchange、pad2、offsFifo、pad3。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSendMem {
/* 中文逐项-类型 T0166：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：head、pad1、ptrExchange、redOpArgExchange、pad2、offsFifo、pad3。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-类型 T0167：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：head、pad1、ptrExchange、redOpArgExchange、pad2、offsFifo。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V1995：head
 * head（uint64_t；成员 ncclSendMem::<匿名>::<匿名>）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 */
      uint64_t head;
/* 中文逐项-变量 V1996：pad1
 * pad1（char；成员 ncclSendMem::<匿名>::<匿名>）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
      char pad1[CACHE_LINE_SIZE-sizeof(uint64_t)];
/* 中文逐项-变量 V1997：ptrExchange
 * ptrExchange（void；成员 ncclSendMem::<匿名>::<匿名>）：直接用户缓冲访问的指针交换区，不是所有 rank 的全局共享地址表。
 */
      void* ptrExchange;
/* 中文逐项-变量 V1998：redOpArgExchange
 * redOpArgExchange（uint64_t；成员 ncclSendMem::<匿名>::<匿名>）：直接拉取路径中交换预处理标量的控制区，保证远端输入使用正确缩放语义。
 */
      uint64_t redOpArgExchange[2];
/* 中文逐项-变量 V1999：pad2
 * pad2（char；成员 ncclSendMem::<匿名>::<匿名>）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
      char pad2[CACHE_LINE_SIZE-sizeof(void*)-2*sizeof(uint64_t)];
/* 中文逐项-变量 V2000：offsFifo
 * offsFifo（int；成员 ncclSendMem::<匿名>::<匿名>）：共享数据池每槽字节偏移，代理先提供地址，GPU 再按偏移读写本次槽位。
 */
      int offsFifo[NCCL_STEPS];
    };
/* 中文逐项-变量 V2001：pad3
 * pad3（char；成员 ncclSendMem::<匿名>）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
    char pad3[MEM_ALIGN];
  };
};

/* 中文导读：
 * 接收方向的控制区：tail 发布可读进度，sizesFifo 传递有效字节数，offsFifo 传递共享池偏移。
 * 这些字段承担控制与可见性协调；不要把它们与 net_ib.cc 的远端地址/rkey FIFO 混为一谈。
 */
/* 中文逐项-类型 T0168：ncclRecvMem
 * 类型职责：连接控制内存布局，分别提供消费head或生产tail及大小/偏移FIFO，与数据缓冲协作。
 * 声明内可见成员（含嵌套成员）：tail、pad1、sizesFifo、offsFifo、flush、pad4。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclRecvMem {
/* 中文逐项-类型 T0169：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：tail、pad1、sizesFifo、offsFifo、flush、pad4。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-类型 T0170：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：tail、pad1、sizesFifo、offsFifo、flush。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V2002：tail
 * tail（uint64_t；成员 ncclRecvMem::<匿名>::<匿名>）：生产者已发布的数据进度，消费者据此判断相应片段是否可读。
 */
      uint64_t tail;
/* 中文逐项-变量 V2003：pad1
 * pad1（char；成员 ncclRecvMem::<匿名>::<匿名>）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
      char pad1[CACHE_LINE_SIZE-sizeof(uint64_t)];
/* 中文逐项-变量 V2004：sizesFifo
 * sizesFifo（int；成员 ncclRecvMem::<匿名>::<匿名>）：GPU 向代理报告各数据槽有效物理字节数的控制数组，空/未就绪值需按后端约定处理。
 */
      int sizesFifo[NCCL_STEPS];
/* 中文逐项-变量 V2005：offsFifo
 * offsFifo（int；成员 ncclRecvMem::<匿名>::<匿名>）：共享数据池每槽字节偏移，代理先提供地址，GPU 再按偏移读写本次槽位。
 */
      int offsFifo[NCCL_STEPS];
/* 中文逐项-变量 V2006：flush
 * flush（int；成员 ncclRecvMem::<匿名>::<匿名>）：供GDRCopy等接收可见性刷新使用的小控制位置，不是完整接收数据缓存。
 */
      int flush; // For GDRCopy-based flush
    };
/* 中文逐项-变量 V2007：pad4
 * pad4（char；成员 ncclRecvMem::<匿名>）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
    char pad4[MEM_ALIGN];
  };
};

typedef cudaError_t(*pfn_cuMemGetAddressRange_t)(void**, size_t*, void*);

/* 中文逐项-类型 T0171：helperThreadState
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum helperThreadState {/* 中文逐项-枚举项 X0177：ThreadStart
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ThreadStart, /* 中文逐项-枚举项 X0178：ThreadStop
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ThreadStop};

/* 中文逐项-常量 X0179：NCCL_IPC_POOL_SIZE
 * 编译期定义：NCCL_IPC_POOL_SIZE = (2*NCCL_MAX_LOCAL_RANKS*NCCL_MAX_OPS)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_IPC_POOL_SIZE (2*NCCL_MAX_LOCAL_RANKS*NCCL_MAX_OPS)

/* 中文逐项-类型 T0172：ncclGraphHelperResources
 * 类型职责：图注册IPC延迟清理队列、线程锁和状态，在允许的CUDA上下文回收映射。
 * 声明内可见成员（含嵌套成员）：comm、threadLock、threadCond、threadState、ipcBases、ipcTail、ipcHead。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclGraphHelperResources {
/* 中文逐项-变量 V2008：comm
 * comm（ncclComm；成员 ncclGraphHelperResources）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  ncclComm* comm;
/* 中文逐项-变量 V2009：threadLock
 * threadLock（pthread_mutex_t；成员 ncclGraphHelperResources）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
  pthread_mutex_t threadLock;
/* 中文逐项-变量 V2010：threadCond
 * threadCond（pthread_cond_t；成员 ncclGraphHelperResources）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
  pthread_cond_t  threadCond;
/* 中文逐项-变量 V2011：threadState
 * threadState（enum helperThreadState；成员 ncclGraphHelperResources）：辅助/代理线程生命周期状态，停止时仍须唤醒并 join 才能回收其资源。
 */
  enum helperThreadState threadState;
/* 中文逐项-变量 V2012：ipcBases
 * ipcBases（void；成员 ncclGraphHelperResources）：图销毁后待关闭 IPC 基址的有界队列及读写游标。
 */
  void* ipcBases[NCCL_IPC_POOL_SIZE];
/* 中文逐项-变量 V2013：ipcTail
 * ipcTail（int；成员 ncclGraphHelperResources）：图销毁后待关闭 IPC 基址的有界队列及读写游标。
 */
  int ipcTail;
/* 中文逐项-变量 V2014：ipcHead
 * ipcHead（int；成员 ncclGraphHelperResources）：图销毁后待关闭 IPC 基址的有界队列及读写游标。
 */
  int ipcHead;
};

/* 中文逐项-类型 T0173：ncclUserRedOp
 * 类型职责：用户归约句柄槽及设备可执行的归约操作/标量表示，避免把主机句柄直接交给设备模板。
 * 声明内可见成员（含嵌套成员）：freeNext、datatype、opFull。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclUserRedOp {
/* 中文逐项-变量 V2015：freeNext
 * freeNext（int；成员 ncclUserRedOp）：用户 PreMulSum 句柄表、容量与空闲链状态，避免误用已销毁/别的comm的句柄。
 */
  int freeNext; // -1=allocated, otherwise index of next free entry in array
/* 中文逐项-变量 V2016：datatype
 * datatype（ncclDataType_t；成员 ncclUserRedOp）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 */
  ncclDataType_t datatype;
/* 中文逐项-变量 V2017：opFull
 * opFull（ncclDevRedOpFull；成员 ncclUserRedOp）：解析公开句柄后的设备归约操作、标量位模式和是否为设备指针的完整信息。
 */
  ncclDevRedOpFull opFull;
};

/* 中文逐项-类型 T0174：ncclNodeRanks
 * 类型职责：单节点参与者规模及局部到全局rank映射，支持不均匀节点规模的P2P调度。
 * 声明内可见成员（含嵌套成员）：localRanks、localRankToRank。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclNodeRanks {
/* 中文逐项-变量 V2018：localRanks
 * localRanks（int；成员 ncclNodeRanks）：当前节点内参与此 communicator 的 rank 数。
 */
  int localRanks;
/* 中文逐项-变量 V2019：localRankToRank
 * localRankToRank（int；成员 ncclNodeRanks）：节点内编号到全局用户 rank 的反向映射。
 */
  int* localRankToRank;
};

/* 中文导读：
 * communicator 的主机端总对象；同一通信组中的每个 rank 各有自己的实例。
 * 持有拓扑、channel、连接、调优模型、工作队列和 Proxy 状态；不是一个 RDMA QP。
 * 多 communicator 的工作与连接通常分别管理，不能据此假定硬件资源自动共享。
 */
/* 中文精读：图示绑定 V3.3 T0175 ncclComm
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=T0175
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0114] ncclComm：字段协作图（结构字段上下文）
 *   图：28-channel-generation, 20-transport
 *   定位：#symbol=T0175&binding=B0114
 */
/* 中文逐项-类型 T0175：ncclComm
 * 类型职责：每个rank的主机端通信总对象，拥有硬件拓扑、channel、调优模型、任务队列和Proxy状态。
 * 声明内可见成员（含嵌套成员）：channels、peerInfo、topo、bootstrap、connect、connectSend、connectRecv、rank、nRanks、cudaDev、busId、cpuAffinity、node、nNodes、localRank、localRanks、maxLocalRanks、rankToNode、rankToLocalRank、localRankToRank、nodeRanks、launchMode、userStream、userStreamSet、doneEvent、intDoneEvent、checkPointers、opCount、collOpCount、nChannels、p2pnChannels、p2pnChannelsPerPeer、p2pChannels、buffSizes、threadThresholds、latencies、bandwidths、maxThreads、groupCudaStream、groupStream、fatalError、abortFlag、devComm、hostDevComm、intraRank、intraRanks、intraBarrier、intraPhase、intraParams、myParams、intraThreads、intraCudaDevs、intraCGMode、intraCC、args、argsptrs、proxyState、collNetSupport、intraHighestTransportType、asyncOps、asyncOpCount、asyncTotalSize、channelSize、lastChannel、asyncAllocMode、p2pSends、p2pRecvs、p2pSendCount、p2pRecvCount、usingCudaGraph、enqueueInfo、nQueueInfoCreated、nQueueInfoDestroyed、lastSetupNode、lastCudaGraphId、driverVersion、pfnCuMemGetAddressRange、graphHelperThread、graphHelperResources、disableGraphHelper、graphRegister、userRedOpCapacity、userRedOpFreeHead、userRedOps。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclComm {
/* 中文逐项-变量 V2020：channels
 * channels（struct ncclChannel；成员 ncclComm）：communicator 的逻辑通道数组，GPU 常以 blockIdx.x 选择一项。
 */
  struct ncclChannel channels[MAXCHANNELS];

/* 中文逐项-变量 V2021：peerInfo
 * peerInfo（struct ncclPeerInfo；成员 ncclComm）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 */
  struct ncclPeerInfo* peerInfo;
/* 中文逐项-变量 V2022：topo
 * topo（struct ncclTopoSystem；成员 ncclComm）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 */
  struct ncclTopoSystem* topo;

/* 中文逐项-变量 V2023：bootstrap
 * bootstrap（void；成员 ncclComm）：初始化控制网络状态，传的是设备/拓扑/连接元信息而非通常的张量载荷。
 */
  void* bootstrap;
  // Bitmasks for ncclTransportP2pSetup
/* 中文逐项-变量 V2024：connect
 * connect（int；成员 ncclComm）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
  int connect;
/* 中文逐项-变量 V2025：connectSend
 * connectSend（uint32_t；成员 ncclComm）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
  uint32_t* connectSend;
/* 中文逐项-变量 V2026：connectRecv
 * connectRecv（uint32_t；成员 ncclComm）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
  uint32_t* connectRecv;

/* 中文逐项-变量 V2027：rank
 * rank（int；成员 ncclComm）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;    // my rank in the communicator
/* 中文逐项-变量 V2028：nRanks
 * nRanks（int；成员 ncclComm）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 */
  int nRanks;  // number of GPUs in communicator
/* 中文逐项-变量 V2029：cudaDev
 * cudaDev（int；成员 ncclComm）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 */
  int cudaDev; // my cuda device index
/* 中文逐项-变量 V2030：busId
 * busId（int64_t；成员 ncclComm）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 */
  int64_t busId;   // my PCI bus ID in int format
/* 中文逐项-变量 V2031：cpuAffinity
 * cpuAffinity（cpu_set_t；成员 ncclComm）：CPU集合掩码，约束后台线程尽量靠近所属GPU/NIC的NUMA范围。
 */
  cpu_set_t cpuAffinity; // CPU affinity of the GPU

/* 中文逐项-变量 V2032：node
 * node（int；成员 ncclComm）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 */
  int node;
/* 中文逐项-变量 V2033：nNodes
 * nNodes（int；成员 ncclComm）：当前 communicator 跨越的节点数量；节点编号不等于 GPU rank。
 */
  int nNodes;
/* 中文逐项-变量 V2034：localRank
 * localRank（int；成员 ncclComm）：本节点内 rank 下标，主要索引本地共享资源。
 */
  int localRank;
/* 中文逐项-变量 V2035：localRanks
 * localRanks（int；成员 ncclComm）：当前节点内参与此 communicator 的 rank 数。
 */
  int localRanks;
/* 中文逐项-变量 V2036：maxLocalRanks
 * maxLocalRanks（int；成员 ncclComm）：所有节点中最大的本地 rank 数，用于统一 P2P 调度步数与存储容量。
 */
  int maxLocalRanks;
/* 中文逐项-变量 V2037：rankToNode
 * rankToNode（int；成员 ncclComm）：全局用户 rank 到所属节点编号的查表映射。
 */
  int* rankToNode;
/* 中文逐项-变量 V2038：rankToLocalRank
 * rankToLocalRank（int；成员 ncclComm）：全局用户 rank 到其节点内编号的查表映射。
 */
  int* rankToLocalRank;
/* 中文逐项-变量 V2039：localRankToRank
 * localRankToRank（int；成员 ncclComm）：节点内编号到全局用户 rank 的反向映射。
 */
  int* localRankToRank;
  // localRanks and localRanktoRank for all nodes
/* 中文逐项-变量 V2040：nodeRanks
 * nodeRanks（struct ncclNodeRanks；成员 ncclComm）：每个节点的 rank 分组及局部到全局编号映射。
 */
  struct ncclNodeRanks* nodeRanks;

/* 中文逐项-变量 V2041：launchMode
 * launchMode（enum { GROUP, PARALLEL, GROUP_GRAPH }；成员 ncclComm）：本版 GROUP/PARALLEL/GROUP_GRAPH 发射组织方式，不等于集合算法选择。
 */
/* 中文逐项-类型 T0176：<匿名enum>
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 声明内可见成员（含嵌套成员）：launchMode。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
  enum { /* 中文逐项-枚举项 X0180：GROUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */GROUP, /* 中文逐项-枚举项 X0181：PARALLEL
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */PARALLEL, /* 中文逐项-枚举项 X0182：GROUP_GRAPH
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */GROUP_GRAPH } launchMode;
/* 中文逐项-变量 V2042：userStream
 * userStream（cudaStream_t；成员 ncclComm）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 */
  cudaStream_t userStream;
/* 中文逐项-变量 V2043：userStreamSet
 * userStreamSet（bool；成员 ncclComm）：本批是否已固定用户CUDA stream，用来检查后续Group调用的流使用一致性。
 */
  bool userStreamSet;
/* 中文逐项-变量 V2044：doneEvent
 * doneEvent（cudaEvent_t；成员 ncclComm）：通信完成事件及内部 stream 依赖事件；record 只排队，不表示已完成。
 */
  cudaEvent_t doneEvent;
/* 中文逐项-变量 V2045：intDoneEvent
 * intDoneEvent（cudaEvent_t；成员 ncclComm）：通信完成事件及内部 stream 依赖事件；record 只排队，不表示已完成。
 */
  cudaEvent_t intDoneEvent;
/* 中文逐项-变量 V2046：checkPointers
 * checkPointers（bool；成员 ncclComm）：是否启用额外 CUDA 缓冲指针检查，关闭检查不改变应用必须满足的契约。
 */
  bool checkPointers;

/* 中文精读：opCount 与 collOpCount 的用途
 * opCount 跟踪提交批次（含 P2P/collective），collOpCount 标记 collective 操作顺序。
 * Proxy 利用操作编号把兼容子任务聚合并保持同连接的推进顺序，它们不直接表示字节偏移。
 * 多个 channel 处理同一 collective 时共享该 collective 的编号，分片号则分别保存在 bid。
 * 读到 opCount 增加不能据此认为 GPU 已执行完成，也不能用它统计创建过多少 QP。
 */
  // Counter for tracking CUDA launches (P2P and collectives included)
/* 中文逐项-变量 V2047：opCount
 * opCount（uint64_t；成员 ncclComm）：提交批次/集合操作的单调编号，维持聚合和同连接先后关系，不是数据偏移。
 */
  uint64_t opCount;
  // Collective operation counter
/* 中文逐项-变量 V2048：collOpCount
 * collOpCount（uint64_t；成员 ncclComm）：提交批次/集合操作的单调编号，维持聚合和同连接先后关系，不是数据偏移。
 */
  uint64_t collOpCount;

  // Channels for collectives
/* 中文逐项-变量 V2049：nChannels
 * nChannels（int；成员 ncclComm）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 */
  int nChannels;
  // Channels (per peer) for p2p
/* 中文逐项-变量 V2050：p2pnChannels
 * p2pnChannels（int；成员 ncclComm）：P2P 全局逻辑通道编号空间大小，不是每个 peer 独占的数量。
 */
  int p2pnChannels;
/* 中文逐项-变量 V2051：p2pnChannelsPerPeer
 * p2pnChannelsPerPeer（int；成员 ncclComm）：一个 peer 可使用的通道预算上限，实际消息拆分可少于该值。
 */
  int p2pnChannelsPerPeer;
/* 中文逐项-变量 V2052：p2pChannels
 * p2pChannels（int；成员 ncclComm）：P2P 位反转生成的通道偏移表，按 peer 基准值加偏移后取模。
 */
  int p2pChannels[MAXCHANNELS];

  // Buffer sizes
/* 中文逐项-变量 V2053：buffSizes
 * buffSizes（int；成员 ncclComm）：各协议物理缓冲字节容量，先除8得到每step物理容量。
 */
  int buffSizes[NCCL_NUM_PROTOCOLS];

  // Algorithm/Protocols thresholds
/* 中文逐项-变量 V2054：threadThresholds
 * threadThresholds（ssize_t；成员 ncclComm）：算法/协议的每线程目标工作量阈值和最大线程配置，用于小消息降并行度。
 */
  ssize_t threadThresholds[NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V2055：latencies
 * latencies（float；成员 ncclComm）：API×算法×协议的性能模型参数，不是每次调用的实测耗时。
 */
  float latencies[NCCL_NUM_FUNCTIONS][NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V2056：bandwidths
 * bandwidths（float；成员 ncclComm）：API×算法×协议的性能模型参数，不是每次调用的实测耗时。
 */
  float bandwidths[NCCL_NUM_FUNCTIONS][NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V2057：maxThreads
 * maxThreads（int；成员 ncclComm）：算法/协议的每线程目标工作量阈值和最大线程配置，用于小消息降并行度。
 */
  int maxThreads[NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS];

  // An internal CUDA stream for NCCL kernel CGMD launches
/* 中文逐项-变量 V2058：groupCudaStream
 * groupCudaStream（int；成员 ncclComm）：是否使用内部 Group stream 的策略标志，需建立用户 stream 事件依赖。
 */
  int groupCudaStream;
/* 中文逐项-变量 V2059：groupStream
 * groupStream（cudaStream_t；成员 ncclComm）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 */
  cudaStream_t groupStream;

  // Whether there has been a fatal error in this communicator.
/* 中文逐项-变量 V2060：fatalError
 * fatalError（ncclResult_t；成员 ncclComm）：communicator 已记录的异步致命错误，供 GetAsyncError 查询。
 */
  ncclResult_t fatalError;

  // Flag to ask NCCL kernels to abort
/* 中文逐项-变量 V2061：abortFlag
 * abortFlag（uint32_t；成员 ncclComm）：主机/设备可观察的异常中止共享标志，用于打破协议或 socket 等待。
 */
  volatile uint32_t *abortFlag;

  // Device side of the communicator
/* 中文逐项-变量 V2062：devComm
 * devComm（struct ncclDevComm；成员 ncclComm）：设备精简comm的主机影子或GPU对象地址，不包含主机总comm里的pthread/socket对象。
 */
  struct ncclDevComm *devComm;
  // Host copy of the devComm (to free CUDA allocs)
/* 中文逐项-变量 V2063：hostDevComm
 * hostDevComm（struct ncclDevComm；成员 ncclComm）：设备精简comm的主机影子或GPU对象地址，不包含主机总comm里的pthread/socket对象。
 */
  struct ncclDevComm hostDevComm;

/* 中文精读：同进程多 GPU 与集群 rank 是不同范围
 * intraRank/intraRanks 是同进程发射组的编号/规模；localRank/localRanks 是本节点范围。
 * rank/nRanks 则是整个 communicator 的逻辑编号/规模。三者在单机单进程时可能看起来相同。
 * intraBarrier/intraParams 用于本地协调发射，跨进程通信仍由 bootstrap/transport 协议完成。
 * 排查多机运行时，应先确认一个字段属于哪个范围，避免把本地数组当成整个集群的表。
 */
  // Intra-process sync
/* 中文逐项-变量 V2064：intraRank
 * intraRank（int；成员 ncclComm）：同进程发射组的编号/规模；与节点内 localRank/localRanks 的作用域不同。
 */
  int intraRank;
/* 中文逐项-变量 V2065：intraRanks
 * intraRanks（int；成员 ncclComm）：同进程发射组的编号/规模；与节点内 localRank/localRanks 的作用域不同。
 */
  int intraRanks;
/* 中文逐项-变量 V2066：intraBarrier
 * intraBarrier（int；成员 ncclComm）：同进程发射屏障计数及双相位状态，避免本地 GPU 启动先后导致依赖死锁。
 */
  int* intraBarrier;
/* 中文逐项-变量 V2067：intraPhase
 * intraPhase（int；成员 ncclComm）：同进程发射屏障计数及双相位状态，避免本地 GPU 启动先后导致依赖死锁。
 */
  int intraPhase;

  // Storage for deferred intra-process launch
/* 中文逐项-变量 V2068：intraParams
 * intraParams（struct cudaLaunchParams；成员 ncclComm）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 */
  struct cudaLaunchParams * intraParams;
/* 中文逐项-变量 V2069：myParams
 * myParams（struct cudaLaunchParams；成员 ncclComm）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 */
  struct cudaLaunchParams *myParams;
/* 中文逐项-变量 V2070：intraThreads
 * intraThreads（pthread_t；成员 ncclComm）：本地发射组线程/设备列表及协作发射/计算能力信息。
 */
  pthread_t* intraThreads;
/* 中文逐项-变量 V2071：intraCudaDevs
 * intraCudaDevs（int；成员 ncclComm）：本地发射组线程/设备列表及协作发射/计算能力信息。
 */
  int* intraCudaDevs;
/* 中文逐项-变量 V2072：intraCGMode
 * intraCGMode（int；成员 ncclComm）：本地发射组线程/设备列表及协作发射/计算能力信息。
 */
  int* intraCGMode; // Whether we can use CUDA9 CGMD or not
/* 中文逐项-变量 V2073：intraCC
 * intraCC（int；成员 ncclComm）：本地发射组线程/设备列表及协作发射/计算能力信息。
 */
  int* intraCC; // Only to check all have the same ComputeCap and disable CGMode if not
/* 中文逐项-变量 V2074：args
 * args（struct ncclWorkElem；成员 ncclComm）：当前设备任务，含本次元素范围、bid、nChannels、缓冲与归约参数。
 */
  struct ncclWorkElem args;
/* 中文逐项-变量 V2075：argsptrs
 * argsptrs（void；成员 ncclComm）：传给 cudaLaunchKernel 的参数地址数组，包含 devComm 与首条内联 work 的地址。
 */
  void* argsptrs[2];

/* 中文逐项-变量 V2076：proxyState
 * proxyState（struct ncclProxyState；成员 ncclComm）：communicator的代理总状态或数据推进子状态，分别组织服务控制和活跃请求。
 */
  struct ncclProxyState proxyState;

  // Whether this communicator uses collNet
/* 中文逐项-变量 V2077：collNetSupport
 * collNetSupport（int；成员 ncclComm）：该communicator是否具备一致可用的CollNet路径，算法选型还要检查类型/归约支持。
 */
  int collNetSupport;
/* 中文逐项-变量 V2078：intraHighestTransportType
 * intraHighestTransportType（int；成员 ncclComm）：同进程/机内已选传输中开销最高的类型，用来限制要求全体P2P的优化。
 */
  int intraHighestTransportType;

  // Store info of async operations
/* 中文逐项-变量 V2079：asyncOps
 * asyncOps（struct ncclInfo；成员 ncclComm）：组内延迟 collective 描述数组及有效条数，GroupEnd 统一选型/生成 work。
 */
  struct ncclInfo* asyncOps;
/* 中文逐项-变量 V2080：asyncOpCount
 * asyncOpCount（int；成员 ncclComm）：组内延迟 collective 描述数组及有效条数，GroupEnd 统一选型/生成 work。
 */
  int asyncOpCount;
/* 中文逐项-变量 V2081：asyncTotalSize
 * asyncTotalSize（size_t；成员 ncclComm）：排队有效数据规模统计，供负载平衡或聚合选型使用。
 */
  size_t asyncTotalSize;
/* 中文逐项-变量 V2082：channelSize
 * channelSize（ssize_t；成员 ncclComm）：聚合时每通道目标规模、轮转游标和最短队列/轮转策略。
 */
  ssize_t channelSize;
/* 中文逐项-变量 V2083：lastChannel
 * lastChannel（int；成员 ncclComm）：聚合时每通道目标规模、轮转游标和最短队列/轮转策略。
 */
  int lastChannel;
/* 中文逐项-变量 V2084：asyncAllocMode
 * asyncAllocMode（enum { ROUND_ROBIN, SHORTEST_QUEUE }；成员 ncclComm）：聚合时每通道目标规模、轮转游标和最短队列/轮转策略。
 */
/* 中文逐项-类型 T0177：<匿名enum>
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 声明内可见成员（含嵌套成员）：asyncAllocMode。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
  enum { /* 中文逐项-枚举项 X0183：ROUND_ROBIN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ROUND_ROBIN, /* 中文逐项-枚举项 X0184：SHORTEST_QUEUE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */SHORTEST_QUEUE } asyncAllocMode;

  //list of async p2p operation queued in a group semantics
/* 中文逐项-变量 V2085：p2pSends
 * p2pSends（ncclP2Plist；成员 ncclComm）：按 peer 保存的 P2P 消息列表及本批未调度发送/接收数量。
 */
  ncclP2Plist** p2pSends;
/* 中文逐项-变量 V2086：p2pRecvs
 * p2pRecvs（ncclP2Plist；成员 ncclComm）：按 peer 保存的 P2P 消息列表及本批未调度发送/接收数量。
 */
  ncclP2Plist** p2pRecvs;
/* 中文逐项-变量 V2087：p2pSendCount
 * p2pSendCount（int；成员 ncclComm）：按 peer 保存的 P2P 消息列表及本批未调度发送/接收数量。
 */
  int p2pSendCount;
/* 中文逐项-变量 V2088：p2pRecvCount
 * p2pRecvCount（int；成员 ncclComm）：按 peer 保存的 P2P 消息列表及本批未调度发送/接收数量。
 */
  int p2pRecvCount;

  // Store info for cudaGraph
/* 中文逐项-变量 V2089：usingCudaGraph
 * usingCudaGraph（int；成员 ncclComm）：当前准备阶段是否在 CUDA Graph 捕获模式，队列生命周期与普通提交不同。
 */
  int usingCudaGraph; // Only use it during capture time, not launch time
/* 中文逐项-变量 V2090：enqueueInfo
 * enqueueInfo（struct ncclQueueInfo；成员 ncclComm）：本提交/图持有的排队元信息和可复用任务列表，HostSetup 根据它们装填真实 FIFO。
 */
  struct ncclQueueInfo* enqueueInfo;
/* 中文逐项-变量 V2091：nQueueInfoCreated
 * nQueueInfoCreated（int；成员 ncclComm）：队列对象建立/销毁计数，辅助协调 CUDA Graph 持有对象的异步生命周期。
 */
  int nQueueInfoCreated;
/* 中文逐项-变量 V2092：nQueueInfoDestroyed
 * nQueueInfoDestroyed（int；成员 ncclComm）：队列对象建立/销毁计数，辅助协调 CUDA Graph 持有对象的异步生命周期。
 */
  int nQueueInfoDestroyed;
/* 中文逐项-变量 V2093：lastSetupNode
 * lastSetupNode（cudaGraphNode_t；成员 ncclComm）：最近图 HostSetup 节点和捕获图标识，维护同图重放/依赖关系。
 */
  cudaGraphNode_t lastSetupNode;
/* 中文逐项-变量 V2094：lastCudaGraphId
 * lastCudaGraphId（unsigned long long；成员 ncclComm）：最近图 HostSetup 节点和捕获图标识，维护同图重放/依赖关系。
 */
  unsigned long long lastCudaGraphId;
/* 中文逐项-变量 V2095：driverVersion
 * driverVersion（int；成员 ncclComm）：CUDA 驱动版本与分配范围查询入口，用于判断图/IPC注册优化能力。
 */
  int driverVersion;
/* 中文逐项-变量 V2096：pfnCuMemGetAddressRange
 * pfnCuMemGetAddressRange（pfn_cuMemGetAddressRange_t；成员 ncclComm）：CUDA 驱动版本与分配范围查询入口，用于判断图/IPC注册优化能力。
 */
  pfn_cuMemGetAddressRange_t pfnCuMemGetAddressRange;
/* 中文逐项-变量 V2097：graphHelperThread
 * graphHelperThread（pthread_t；成员 ncclComm）：图注册优化的辅助线程、资源、禁用策略与注册配置，负责在允许上下文关闭IPC映射。
 */
  pthread_t graphHelperThread;
/* 中文逐项-变量 V2098：graphHelperResources
 * graphHelperResources（struct ncclGraphHelperResources；成员 ncclComm）：图注册优化的辅助线程、资源、禁用策略与注册配置，负责在允许上下文关闭IPC映射。
 */
  struct ncclGraphHelperResources* graphHelperResources;
/* 中文逐项-变量 V2099：disableGraphHelper
 * disableGraphHelper（int；成员 ncclComm）：图注册优化的辅助线程、资源、禁用策略与注册配置，负责在允许上下文关闭IPC映射。
 */
  int disableGraphHelper;
/* 中文逐项-变量 V2100：graphRegister
 * graphRegister（int；成员 ncclComm）：图注册优化的辅助线程、资源、禁用策略与注册配置，负责在允许上下文关闭IPC映射。
 */
  int graphRegister;

  // user-created reduction ops
/* 中文逐项-变量 V2101：userRedOpCapacity、userRedOpFreeHead
 * userRedOpCapacity（int；成员 ncclComm）：用户 PreMulSum 句柄表、容量与空闲链状态，避免误用已销毁/别的comm的句柄。
 * userRedOpFreeHead（int；成员 ncclComm）：用户 PreMulSum 句柄表、容量与空闲链状态，避免误用已销毁/别的comm的句柄。
 */
  int userRedOpCapacity, userRedOpFreeHead;
/* 中文逐项-变量 V2102：userRedOps
 * userRedOps（ncclUserRedOp；成员 ncclComm）：用户 PreMulSum 句柄表、容量与空闲链状态，避免误用已销毁/别的comm的句柄。
 */
  ncclUserRedOp *userRedOps;
};

// Scrambles the bits of non-builtin values of ncclRedOp_t according to the
// communicator memory address. Used to catch bugs so that integer handles
// associated with this communicator won't collide with handles of other
// communicatrs. This function is its own inverse.
/* 中文内部逻辑 F0627：ncclUserRedOpMangle
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(int(op) < int(ncclNumOps))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return op;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：uint64_t h = reinterpret_cast<uint64_t>(comm);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：h ^= h >> 32;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 调用 / 计算 / 写入：h *= 0x9e3779b97f4a7c13u;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：h >>= 32;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：h &= int(ncclMaxRedOp);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 准备局部状态：int op1 = int(h) ^ int(op);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 函数返回：return op1 < int(ncclNumOps) ? op : ncclRedOp_t(op1);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__comm_h.md#f0627
 */
/* 中文逐项-函数 F0627：ncclUserRedOpMangle
 * 用途与位置：用 communicator 地址导出的可逆异或扰动用户操作句柄，减少把别的 comm 句柄误用于本 comm 的风险。
 * 参数 comm（ncclComm *comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 op（ncclRedOp_t op）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::hostToDevRedOp；src/enqueue.cc::ncclRedOpCreatePreMulSum；src/enqueue.cc::ncclRedOpDestroy；src/misc/argcheck.cc::ArgsCheck。
 * 本体直接出现的调用/包装：int；ncclRedOp_t；reinterpret_cast<uint64_t>。
 * 需要对照的边界/推进条件：int(op) < int(ncclNumOps。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return op;；return op1 < int(ncclNumOps) ? op : ncclRedOp_t(op1);。CHECK类宏还可能提前返回/跳转。
 */
static inline ncclRedOp_t ncclUserRedOpMangle(ncclComm *comm, ncclRedOp_t op) {
  // Preserve the built-in values.
  if(int(op) < int(ncclNumOps))
    return op;
/* 中文逐项-变量 V2103：h
 * h（uint64_t；函数 ncclUserRedOpMangle 内）：GDRCopy固定缓冲句柄的内部表示，必须用匹配的unpin接口回收。
 * 初值/来源 h = reinterpret_cast<uint64_t>(comm)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  uint64_t h = reinterpret_cast<uint64_t>(comm);
  h ^= h >> 32;
  h *= 0x9e3779b97f4a7c13u; // Knuth's 64-bit magical hash constant
  h >>= 32; // h is now an excellent 32-bit hash of the comm pointer
  h &= int(ncclMaxRedOp); // ncclMaxRedOp is a power of 2 minus 1
/* 中文逐项-变量 V2104：op1
 * op1（int；函数 ncclUserRedOpMangle 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 op1 = int(h) ^ int(op)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int op1 = int(h) ^ int(op);
  // Since builtin values are preserved, we also have to preserve their preimage.
  return op1 < int(ncclNumOps) ? op : ncclRedOp_t(op1);
}

#endif
