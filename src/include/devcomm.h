/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_DEVICE_H_
#define NCCL_DEVICE_H_

#include "nccl.h"
#include "align.h"
#include <stdint.h>

#define NCCL_NUM_FUNCTIONS 5 // Send/Recv not included for now
typedef enum { ncclFuncBroadcast, ncclFuncReduce, ncclFuncAllGather, ncclFuncReduceScatter, ncclFuncAllReduce, ncclFuncSendRecv, ncclFuncSend, ncclFuncRecv, ncclNumFuncs} ncclFunc_t;
extern const char* ncclFuncStr[NCCL_NUM_FUNCTIONS];

#define NCCL_NUM_ALGORITHMS 3 // Tree/Ring/CollNet
#define NCCL_ALGO_TREE 0
#define NCCL_ALGO_RING 1
#define NCCL_ALGO_COLLNET 2
extern const char* ncclAlgoStr[NCCL_NUM_ALGORITHMS];

#define NCCL_NUM_PROTOCOLS 3 // Simple/LL/LL128
#define NCCL_PROTO_LL 0
#define NCCL_PROTO_LL128 1
#define NCCL_PROTO_SIMPLE 2
extern const char* ncclProtoStr[NCCL_NUM_PROTOCOLS];

/* 中文精读：两种循环队列，不要混淆
 * NCCL_MAX_OPS=2048 限制每 channel 的工作描述符 FIFO，存的是 ncclWork。
 * NCCL_STEPS=8 则切分每条连接的协议数据缓冲，承载真实通信数据并由 head/tail 控制复用。
 * 一个 work 可以触发许多数据 step；许多次操作也会继续复用同一批 8 个数据槽。
 * MAXCHANNELS=32 是本版编译期数组上限，实际值由拓扑与调度决定。
 */
#define NCCL_MAX_OPS 2048
#define NCCL_STEPS 8

/* 中文精读：LL 行的数据与完成标记
 * 一行 16 字节，data1/data2 合计 8 字节有效载荷，flag1/flag2 合计 8 字节序号。
 * 接收者必须看到两个 flag 都匹配当前 step 才读取这一行，防止新旧数据混合。
 * flag 不是一个全局布尔 ready：槽位会复用，必须用进度序号区分不同轮次。
 * 因此 LL 的有效载荷效率是 1/2，主机 Proxy 计算 nLoops 时要相应扣除标记开销。
 */
union ncclLLFifoLine {
  /* Flags have to be *after* data, because otherwise, an incomplete receive
     from the network may receive the flag but not the data.
     Note this is assuming that either we receive contiguous chunks of data
     (sockets) or data is written with an atomicity of 8 bytes (IB/RDMA). */
  struct {
    uint32_t data1;
    uint32_t flag1;
    uint32_t data2;
    uint32_t flag2;
  };
  uint64_t v[2];
  int4 i4;
};

#define WARP_SIZE 32
#define MAXCHANNELS 32
#define NCCL_MAX_NTHREADS 640
#define NCCL_SIMPLE_MAX_NTHREADS 512
#define NCCL_LL_MAX_NTHREADS 512
#define NCCL_LL_LINES_PER_THREAD 8
#ifdef TEST_LL_CLEANUP
#define NCCL_LL_CLEAN_MASK 0x078 // Set to 0x100 to disable cleanup
#define NCCL_LL_FLAG_MAX   0x100
#define NCCL_LL_FLAG(a) ((uint32_t)((a) % NCCL_LL_FLAG_MAX))
#else
#define NCCL_LL_CLEAN_MASK 0x7ffffff8
#define NCCL_LL_FLAG(a) ((uint32_t)(a))
#endif
// Make sure the clean mask will last for at least NCCL_NSTEPS
static_assert(NCCL_LL_CLEAN_MASK % NCCL_STEPS == 0, "Invalid NCCL_LL_CLEAN_MASK value");

#define NCCL_LL128_LINESIZE 128
#define NCCL_LL128_LINEELEMS (NCCL_LL128_LINESIZE/sizeof(uint64_t))
#define NCCL_LL128_DATAELEMS (NCCL_LL128_LINEELEMS-1)

#define NCCL_LL128_MAX_NTHREADS 640
#define NCCL_LL128_ELEMS_PER_THREAD 120

#define NCCL_LL128_SHMEM_ELEMS_PER_THREAD 8
#define NCCL_LL128_SHMEM_SIZE (NCCL_LL128_SHMEM_ELEMS_PER_THREAD*NCCL_LL128_MAX_NTHREADS)

#define NCCL_DIRECT_WRITE 0x01
#define NCCL_DIRECT_READ  0x02
#define NCCL_DIRECT_NIC   0x04
#define NCCL_IPC_WRITE    0x08
#define NCCL_IPC_READ     0x10

/* 中文导读：
 * GPU 原语实际使用的连接视图。buffs 按 LL/LL128/Simple 区分，head/tail 和 step 驱动槽位流控。
 * 此处没有 ibv_qp；网络 QP/MR 由主机端 NET/IB 后端持有，GPU 经这些共享字段与 Proxy 协作。
 */
struct ncclConnInfo {
  // Regular comm mechanism
  char *buffs[NCCL_NUM_PROTOCOLS]; // Local for recv, remote for send
  uint64_t *tail;     // Local for recv, remote for send
  uint64_t *head;     // Local for send, remote for recv

  int direct;         // Direct communication
  int shared;         // Buffers are shared
  void **ptrExchange; // Pointer exchange for direct communication
  uint64_t* redOpArgExchange; // PreOp scaler exchange for direct pull case

  int *sizesFifo;     // Sizes fifo from GPU to proxy
  int *offsFifo;      // Buffer fifo from proxy to GPU

  uint64_t step;      // Keep where we are
  uint64_t llLastCleaning;
};

struct ncclProxyConnector {
  int rank;
  int localRank;
  struct ncclProxyConnection* connection;
  struct ncclComm* comm;
};

/* 中文导读：
 * 一个 channel 到一个 peer、一个方向、一个连接索引的逻辑连接。
 * transportComm 是主机端回调表，transportResources 是具体 transport 的私有对象，conn 是 GPU 使用的数据。
 * connected 标记建链完成，使相同逻辑连接可被后续操作复用。逻辑连接数不直接等于 QP 数。
 */
struct ncclConnector {
  int connected;
  struct ncclProxyConnector proxyConn;
  struct ncclTransportComm* transportComm;
  void* transportResources;
  struct ncclConnInfo conn;
  struct ncclComm *comm;
};

/* 中文精读：设备算法中的两个 rank 坐标系
 * prev/next 是 communicator 的用户 rank，用于在 devPeers 中索引连接。
 * userRanks[0] 是自己，[1] 是下游，[nRanks-1] 是上游；数组从当前 rank 开始旋转。
 * index 是自己在共同环顺序中的内部位置，用来安排 AllReduce 各块的起始归约顺序。
 * AllGather/ReduceScatter 的输出段仍按用户 rank 排列，不会被拓扑优化打乱。
 */
struct ncclRing {
  // Shortcuts for userRanks[1] and userRanks[n-1]
  int prev;
  int next;

  // Maps an internal nccl index to user-specified rank order. This is necessary
  // since we need to know how the user expects data to be ordered across
  // devices. Ordered from current device.
  int* userRanks;
  int* devUserRanks;

  int index; // This rank's index in the ring
};


#define NCCL_MAX_TREE_ARITY 3
/* 中文精读：一条 channel 的树邻接关系
 * up 为父 rank，down 为孩子 rank，缺失位置用 -1；depth 为流水/分片调优提供深度信息。
 * 单 GPU rank 最多 3 个 down 槽，包含节点内部连接与节点间树拼接的需要。
 * 构造原语时 FanAsymmetric 限定编译期最大收发数，实际有效邻居数由 -1 终止的列表确定。
 * 树数组本身不代表已分配 QP；对应 connector 的 transport 才决定走本地还是网络。
 */
struct ncclTree {
  int depth;
  int up;
  int down[NCCL_MAX_TREE_ARITY];
};

#define NCCL_MAX_DIRECT_ARITY 7
struct ncclDirect {
  int depth;
  int out;
  int nHeads;
  int headRank;
  int shift;
  int up[NCCL_MAX_DIRECT_ARITY];
  int down[NCCL_MAX_DIRECT_ARITY];
};

#define NCCL_MAX_CONNS 2
/* 中文导读：
 * channel 内按 rank 索引的对端对象；send/recv 分开，每个方向有 NCCL_MAX_CONNS 个槽位。
 * 本版本初始化 Ring/Tree 使用索引 0，普通 Send/Recv 路径使用索引 1；CollNet 也有专门用法。
 * 数组被分配不意味着每个槽位都已建立连接，更不意味着都使用 RDMA。
 */
struct ncclPeer {
  struct ncclConnector send[NCCL_MAX_CONNS];
  struct ncclConnector recv[NCCL_MAX_CONNS];
};

struct ncclDevComm;

/* ncclWork is to be a power of two, currently 8x64 bytes, */
/* to make sure reads to host from the CUDA kernel are aligned. */
/* Make sure to adjust padding at the end of ncclWorkElem. */
#define NCCL_WORK_SIZE 512

enum ncclWorkElemType : uint8_t {
   ncclWorkTypeUnused=0,
   ncclWorkTypeColl=1,
   ncclWorkTypeP2p=2,
   ncclWorkTypeRegColl=3
};
enum ncclWorkElemSubType : uint8_t {
  ncclWorkSubTypeUnused =0,
  ncclWorkSubTypeSend,
  ncclWorkSubTypeRecv
};

/* 中文精读：GPU 如何知道这条任务要执行什么
 * type 区分 Unused/Collective/P2P/RegisteredCollective；funcIndex 编码具体设备函数。
 * nWarps 指定参与执行的线程数，isLast 表示本批任务到此结束。
 * header 与 union 中第一个元素重叠，使 kernel 可先看公共头，再按具体类型读取正确布局。
 * 此头部属于工作描述，不是发到网络上的报文头。
 */
struct ncclWorkElemHeader {
  uint16_t funcIndex;
  enum ncclWorkElemType type;
  unsigned nWarps:5;
  unsigned isLast:1;
};

/* 中文导读：
 * 主机调度器生成的集合通信工作描述：缓冲区、元素数量、channel 分片编号及执行函数索引。
 * count 是数据类型元素数，不能始终当成字节数；funcIndex 编码操作/归约/类型/算法/协议组合。
 */
struct ncclWorkElem {
  struct ncclWorkElemHeader header;
  uint8_t regUsed;
  uint8_t direct;
  uint8_t redOpArgIsPtr;

  const void * sendbuff;
  void * recvbuff;

  size_t count;
  size_t lastChunkSize;
  uint32_t root;
  uint8_t bid;
  uint8_t nChannels;
  uint64_t redOpArg;
  uint64_t pad;
};
static_assert(NCCL_WORK_SIZE % sizeof(struct ncclWorkElem) == 0, "ncclWorkElem size must be a multiple of ncclWork size");

/* 中文精读：P2P 元素逐字段阅读
 * peer/buff/count 指定通信对端、本次块起点、块长度；本版本 P2P 设备任务按 Int8 即字节处理。
 * chunkSize 是设备端继续切片的字节上限，ngroups 是同条 work 的线程组数量。
 * warpStart/nWarps 指定本项的线程范围；subType 用于主机装填时区分 Send 和 Recv。
 * 一条 work 可能有多个 peer 的收发，因此一条 channel/block 不固定只服务一个 peer。
 */
struct ncclWorkElemP2p {
  struct ncclWorkElemHeader header;
  int32_t peer;
  void* buff;
  size_t count;
  int chunkSize;
  uint8_t ngroups;
  uint8_t warpStart;
  uint8_t nWarps;
  enum ncclWorkElemSubType subType;
};
static_assert(NCCL_WORK_SIZE % sizeof(struct ncclWorkElemP2p) == 0, "ncclWorkElemP2p size must be a multiple of ncclWork size");

struct ncclWorkElemReg {
  struct ncclWorkElem elem;
  void* dnInputs[NCCL_MAX_DIRECT_ARITY+1];
  void* dnOutputs[NCCL_MAX_DIRECT_ARITY+1];
  void* upOutputs[NCCL_MAX_DIRECT_ARITY+1];
};
static_assert(NCCL_WORK_SIZE % sizeof(struct ncclWorkElemReg) == 0, "ncclWork size must be a multiple of ncclWorkElemReg size");
static_assert(sizeof(struct ncclWorkElemReg) % sizeof(struct ncclWorkElem) == 0, "ncclWorkElemReg size must be a multiple of ncclWorkElem size");

#define NCCL_MAX_WORK_ELEMENTS (NCCL_WORK_SIZE/sizeof(struct ncclWorkElem))
#define NCCL_MAX_WORK_ELEMENTS_P2P (NCCL_WORK_SIZE/sizeof(struct ncclWorkElemP2p))
#define NCCL_MAX_WORK_ELEMENTS_REG (NCCL_WORK_SIZE/sizeof(struct ncclWorkElemReg))
// Number of named barriers supported by CUDA
#define NCCL_MAX_GROUPS 16

/* 中文导读：
 * 固定 512 字节的工作记录，可装多个 collective 元素或 P2P 元素。
 * CPU 填入 channel 工作 FIFO，GPU 取出到共享内存后分派执行；isLast 标记本批最后一个记录。
 */
struct ncclWork {
  union {
    char pad[NCCL_WORK_SIZE];
    struct ncclWorkElemHeader header;
    struct ncclWorkElem elems[NCCL_MAX_WORK_ELEMENTS];
    struct ncclWorkElemP2p p2pElems[NCCL_MAX_WORK_ELEMENTS_P2P];
    struct ncclWorkElemReg regElems[NCCL_MAX_WORK_ELEMENTS_REG];
  };
};

static_assert(sizeof(struct ncclWork) == NCCL_WORK_SIZE, "ncclWork size needs to be well aligned");

/* 中文导读：
 * 独立的逻辑通信流水线：拥有 Ring/Tree 拓扑、peer 连接表和工作 FIFO。
 * 常规 kernel 以 blockIdx.x 选 channel；同一 channel 可执行一批工作，多个 channel 并行分担数据。
 * channel 不是网口、QP 或 CUDA stream；collective 与 P2P 使用同一 channels 数组中的对象。
 */
struct ncclChannel {
  union {
    struct {
      struct ncclRing ring;
      struct ncclTree tree;
      struct ncclDirect collTree;

      int id;

      // Communication structures
      struct ncclPeer* peers;
      struct ncclPeer* devPeers;

      // Operation list for aggregation
      struct ncclWork* workFifo;
      int workCount;
      size_t totalSize;
      uint64_t workFifoTail; // Only used by CPU
      uint16_t index;        // Only used by GPU

      // GDRCOPY support
      struct ncclWork* workFifoGdr;
      struct ncclWork* workFifoDev;
      void* gdrMemDesc;
    };
    int data[0x80];
  };
};
static_assert(sizeof(struct ncclChannel) == 0x80*sizeof(int), "ncclChannel must have a pow2 size");

/* 中文导读：
 * communicator 的 GPU 端精简视图，只保留执行需要的 rank、缓冲区大小、中止标记和 channel 指针。
 * 主机端 ncclComm 中的 socket、pthread 和 verbs 资源不会直接由 GPU kernel 操作。
 */
struct ncclDevComm {
  int rank;
  int nRanks;
  int buffSizes[NCCL_NUM_PROTOCOLS];

  // Flag to ask NCCL kernels to abort
  volatile uint32_t *abortFlag;

  // Channels, device side
  struct ncclChannel* channels;
};

/* 中文精读：为何 comm 与 channels 连续布局
 * 在设备端可以从 comm 指针强转得到紧随其后的 channels 数组，少一次间接全局内存读取。
 * 主机 devCommSetup 分配并构造这个完整对象，kernel 的 common.h 按相同布局定位 channel。
 * 这是固定 ABI/布局契约；即使只为阅读增加字段，也可能破坏偏移和大小约束。
 * 本注释分支只增加注释，不改变结构体成员、对齐或任何编译期常量。
 */
struct ncclDevCommAndChannels {
  ncclDevComm comm;
  ncclChannel channels[MAXCHANNELS];
};

#endif
