/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_PROXY_H_
/* 中文逐项-常量 X1064：NCCL_PROXY_H_
 * 编译期定义：NCCL_PROXY_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROXY_H_

#include "devcomm.h"
#include "info.h"
#include "socket.h"
#include <pthread.h>

/* 中文逐项-类型 T0362：ncclProxyOpState
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclProxyOpState { /* 中文逐项-枚举项 X1065：ncclProxyOpNone
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclProxyOpNone, /* 中文逐项-枚举项 X1066：ncclProxyOpReady
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclProxyOpReady, /* 中文逐项-枚举项 X1067：ncclProxyOpProgress
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclProxyOpProgress };

struct ncclProxyArgs;
typedef ncclResult_t (*proxyProgressFunc_t)(struct ncclComm*, struct ncclProxyArgs*);

/* 中文逐项-常量 X1068：NCCL_PROXY_MAX_SUBS
 * 编译期定义：NCCL_PROXY_MAX_SUBS = MAXCHANNELS。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROXY_MAX_SUBS MAXCHANNELS
static_assert(NCCL_MAX_WORK_ELEMENTS <= MAXCHANNELS, "Not enough sub space for max work elements");

/* 中文导读：
 * 主线程提交给 Proxy 的紧凑任务描述；描述 channel、协议和应推进的步数，不携带全部用户数据。
 * 它会转换为 ncclProxyArgs/SubArgs；连接建好后，同一资源可以处理多次 ProxyOp。
 */
/* 中文逐项-类型 T0363：ncclProxyOp
 * 类型职责：代理紧凑提交描述、每连接子任务进度和聚合运行态，分离提交与异步执行生命周期。
 * 声明内可见成员（含嵌套成员）：connection、channelId、nsteps、nbytes、root、next、opCount、sliceSteps、chunkSteps、chunkSize、dtype、redOp、pattern、protocol、pad。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyOp {
/* 中文逐项-变量 V3055：connection
 * connection（struct ncclProxyConnection；成员 ncclProxyOp）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 */
  struct ncclProxyConnection* connection;
/* 中文逐项-变量 V3056：channelId
 * channelId（int；成员 ncclProxyOp）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 */
  int channelId;
/* 中文逐项-变量 V3057：nsteps
 * nsteps（int；成员 ncclProxyOp）：本子任务需要推进的协议总 step 数，不直接等于包数、WQE 数或 QP 数。
 */
  int nsteps;
/* 中文逐项-变量 V3058：nbytes
 * nbytes（ssize_t；成员 ncclProxyOp）：当前消息或片段的字节规模；info.nBytes 对 AG/RS 还计入 nRanks 段总规模。
 */
  ssize_t nbytes;
/* 中文逐项-变量 V3059：root
 * root（int；成员 ncclProxyOp）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 */
  int root;
/* 中文逐项-变量 V3060：next
 * next（int；成员 ncclProxyOp）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
  int next;

/* 中文逐项-变量 V3061：opCount
 * opCount（uint64_t；成员 ncclProxyOp）：提交批次/集合操作的单调编号，维持聚合和同连接先后关系，不是数据偏移。
 */
  uint64_t opCount;
/* 中文逐项-变量 V3062：sliceSteps
 * sliceSteps（int；成员 ncclProxyOp）：一个算法块/原语片段占用的 FIFO step 数，用来对齐 CPU Proxy 与 GPU 进度。
 */
  int sliceSteps;
/* 中文逐项-变量 V3063：chunkSteps
 * chunkSteps（int；成员 ncclProxyOp）：一个算法块/原语片段占用的 FIFO step 数，用来对齐 CPU Proxy 与 GPU 进度。
 */
  int chunkSteps;
/* 中文逐项-变量 V3064：chunkSize
 * chunkSize（int；成员 ncclProxyOp）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 */
  int chunkSize;
/* 中文逐项-变量 V3065：dtype
 * dtype（ncclDataType_t；成员 ncclProxyOp）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 */
  ncclDataType_t dtype;
/* 中文逐项-变量 V3066：redOp
 * redOp（ncclRedOp_t；成员 ncclProxyOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 */
  ncclRedOp_t redOp;
/* 中文逐项-变量 V3067：pattern
 * pattern（ncclPattern_t；成员 ncclProxyOp）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
  ncclPattern_t pattern; // uint8_t
/* 中文逐项-变量 V3068：protocol
 * protocol（uint8_t；成员 ncclProxyOp）：当前数据编码/同步协议枚举 LL、LL128 或 Simple；决定有效载荷比例和推进方法。
 */
  uint8_t protocol;
/* 中文逐项-变量 V3069：pad
 * pad（uint16_t；成员 ncclProxyOp）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
  uint16_t pad;
};
static_assert(sizeof(struct ncclProxyOp) == 64, "Keep ProxyOp aligned with cache lines for effective prefetch");

/* 中文导读：
 * 一个子操作的运行状态。posted/received/flushed/transmitted/done 表示不同阶段，含义依 send/recv 分支而定。
 * requests 保存异步网络请求；只有相应阶段完成才能发布进度或复用槽位，避免 GPU/NIC 并发覆盖。
 */
/* 中文逐项-类型 T0364：ncclProxySubArgs
 * 类型职责：代理紧凑提交描述、每连接子任务进度和聚合运行态，分离提交与异步执行生命周期。
 * 声明内可见成员（含嵌套成员）：connection、channelId、nsteps、nbytes、peer、groupSize、base、posted、received、flushed、transmitted、done、end、requests、profilingEvents。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxySubArgs {
/* 中文逐项-变量 V3070：connection
 * connection（struct ncclProxyConnection；成员 ncclProxySubArgs）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 */
  struct ncclProxyConnection* connection;
/* 中文逐项-变量 V3071：channelId
 * channelId（int；成员 ncclProxySubArgs）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 */
  int channelId;
/* 中文逐项-变量 V3072：nsteps
 * nsteps（int；成员 ncclProxySubArgs）：本子任务需要推进的协议总 step 数，不直接等于包数、WQE 数或 QP 数。
 */
  int nsteps;
/* 中文逐项-变量 V3073：nbytes
 * nbytes（ssize_t；成员 ncclProxySubArgs）：当前消息或片段的字节规模；info.nBytes 对 AG/RS 还计入 nRanks 段总规模。
 */
  ssize_t nbytes;
/* 中文逐项-变量 V3074：peer
 * peer（int；成员 ncclProxySubArgs）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 */
  int peer;

/* 中文逐项-变量 V3075：groupSize
 * groupSize（int；成员 ncclProxySubArgs）：连续使用同一个 recvComm 的子项数量，支持一次网络 Irecv 聚合；不意味着 verbs SRQ。
 */
  int groupSize; // Number of consecutive sub operations sharing the same recvComm
/* 中文逐项-变量 V3076：base
 * base（uint64_t；成员 ncclProxySubArgs）：本操作在长期连接 step 上的对齐起点，本任务 posted/done 等相对计数要加 base 才是绝对进度。
 */
  uint64_t base;
/* 中文逐项-变量 V3077：posted
 * posted（uint64_t；成员 ncclProxySubArgs）：Proxy 子任务各阶段的相对 step 计数，send/recv 分支对应的动作不同，不能互换。
 */
  uint64_t posted;
/* 中文逐项-变量 V3078：received
 * received（uint64_t；成员 ncclProxySubArgs）：Proxy 子任务各阶段的相对 step 计数，send/recv 分支对应的动作不同，不能互换。
 */
  uint64_t received;
/* 中文逐项-变量 V3079：flushed
 * flushed（uint64_t；成员 ncclProxySubArgs）：Proxy 子任务各阶段的相对 step 计数，send/recv 分支对应的动作不同，不能互换。
 */
  uint64_t flushed;
/* 中文逐项-变量 V3080：transmitted
 * transmitted（uint64_t；成员 ncclProxySubArgs）：Proxy 子任务各阶段的相对 step 计数，send/recv 分支对应的动作不同，不能互换。
 */
  uint64_t transmitted;
/* 中文逐项-变量 V3081：done
 * done（uint64_t；成员 ncclProxySubArgs）：Proxy 子任务各阶段的相对 step 计数，send/recv 分支对应的动作不同，不能互换。
 */
  uint64_t done;
/* 中文逐项-变量 V3082：end
 * end（uint64_t；成员 ncclProxySubArgs）：本任务结束序号或范围末端，通常由 base 与总步数推导。
 */
  uint64_t end;
/* 中文逐项-变量 V3083：requests
 * requests（void；成员 ncclProxySubArgs）：在途网络请求或固定请求池；只有 Test 完成后其槽才能重用。
 */
  void* requests[NCCL_STEPS];
/* 中文逐项-变量 V3084：profilingEvents
 * profilingEvents（void；成员 ncclProxySubArgs）：可选代理阶段测量记录，关闭profiling时不参与正常数据传输语义。
 */
  void* profilingEvents[NCCL_STEPS];
};

/* 中文精读：提交描述与运行状态分离
 * ncclProxyOp 是紧凑输入，ProxyArgs 是运行期对象，可包含多个兼容 sub 操作。
 * progress 指向 transport 的推进函数；state 在 Ready/Progress/None 间切换。
 * next/nextPeer/proxyAppendPtr 维护活跃任务及同连接的先后依赖，防止同连接被乱序推进。
 * idle 表示本轮没取得进展，调用者可让出 CPU；它不表示整个 communicator 已空闲。
 */
/* 中文逐项-类型 T0365：ncclProxyArgs
 * 类型职责：代理紧凑提交描述、每连接子任务进度和聚合运行态，分离提交与异步执行生命周期。
 * 声明内可见成员（含嵌套成员）：subs、progress、nsubs、done、opCount、sliceSteps、chunkSteps、chunkSize、dtype、redOp、pattern、protocol、state、sharedBuff、sharedSize、idle、next、nextPeer、proxyAppendPtr。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyArgs {
/* 中文逐项-变量 V3085：subs
 * subs（struct ncclProxySubArgs；成员 ncclProxyArgs）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 */
  struct ncclProxySubArgs subs[NCCL_PROXY_MAX_SUBS];
/* 中文逐项-变量 V3086：progress
 * progress（proxyProgressFunc_t；成员 ncclProxyArgs）：所选 transport 的非阻塞推进回调，反复调用直到状态进入完成。
 */
  proxyProgressFunc_t progress;
/* 中文逐项-变量 V3087：nsubs
 * nsubs（int；成员 ncclProxyArgs）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 */
  int nsubs;
/* 中文逐项-变量 V3088：done
 * done（int；成员 ncclProxyArgs）：该聚合操作已经完成的sub数量；达到nsubs后整个ProxyArgs才可退出活跃队列。
 */
  int done;
/* 中文逐项-变量 V3089：opCount
 * opCount（uint64_t；成员 ncclProxyArgs）：提交批次/集合操作的单调编号，维持聚合和同连接先后关系，不是数据偏移。
 */
  uint64_t opCount;
/* 中文逐项-变量 V3090：sliceSteps
 * sliceSteps（int；成员 ncclProxyArgs）：一个算法块/原语片段占用的 FIFO step 数，用来对齐 CPU Proxy 与 GPU 进度。
 */
  int sliceSteps;
/* 中文逐项-变量 V3091：chunkSteps
 * chunkSteps（int；成员 ncclProxyArgs）：一个算法块/原语片段占用的 FIFO step 数，用来对齐 CPU Proxy 与 GPU 进度。
 */
  int chunkSteps;
/* 中文逐项-变量 V3092：chunkSize
 * chunkSize（int；成员 ncclProxyArgs）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 */
  int chunkSize;
/* 中文逐项-变量 V3093：dtype
 * dtype（ncclDataType_t；成员 ncclProxyArgs）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 */
  ncclDataType_t dtype;
/* 中文逐项-变量 V3094：redOp
 * redOp（ncclRedOp_t；成员 ncclProxyArgs）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 */
  ncclRedOp_t redOp;
/* 中文逐项-变量 V3095：pattern
 * pattern（ncclPattern_t；成员 ncclProxyArgs）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
  ncclPattern_t pattern;
/* 中文逐项-变量 V3096：protocol
 * protocol（uint8_t；成员 ncclProxyArgs）：当前数据编码/同步协议枚举 LL、LL128 或 Simple；决定有效载荷比例和推进方法。
 */
  uint8_t protocol;
/* 中文逐项-变量 V3097：state
 * state（int；成员 ncclProxyArgs）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 */
  int state;
/* 中文逐项-变量 V3098：sharedBuff
 * sharedBuff（char；成员 ncclProxyArgs）：共享协议数据池的每步地址/大小及GPU/主机分配视图，按资源引用计数复用。
 */
  char* sharedBuff[NCCL_STEPS];
/* 中文逐项-变量 V3099：sharedSize
 * sharedSize（int；成员 ncclProxyArgs）：共享协议数据池的每步地址/大小及GPU/主机分配视图，按资源引用计数复用。
 */
  int sharedSize[NCCL_STEPS];

/* 中文逐项-变量 V3100：idle
 * idle（int；成员 ncclProxyArgs）：本轮推进是否未取得进展，用于 CPU 让出策略，不代表全体通信已经完成。
 */
  int idle;

  // Element linking
/* 中文逐项-变量 V3101：next
 * next（struct ncclProxyArgs；成员 ncclProxyArgs）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
  struct ncclProxyArgs* next;
/* 中文逐项-变量 V3102：nextPeer
 * nextPeer（struct ncclProxyArgs；成员 ncclProxyArgs）：同连接操作的后继链/尾部引用，维持共享连接上的执行顺序。
 */
  struct ncclProxyArgs* nextPeer;
/* 中文逐项-变量 V3103：proxyAppendPtr
 * proxyAppendPtr（struct ncclProxyArgs；成员 ncclProxyArgs）：同连接操作的后继链/尾部引用，维持共享连接上的执行顺序。
 */
  struct ncclProxyArgs** proxyAppendPtr;
};
/* 中文逐项-常量 X1069：NCCL_MAX_NETDEVS
 * 编译期定义：NCCL_MAX_NETDEVS = 128。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MAX_NETDEVS 128

// ProxyOps are used to communicate between main thread and service thread
// Make sure we have enough to store two full rounds of operations on all channels.
// Otherwise we'd be unable to post half of them to free new elements.
/* 中文逐项-常量 X1070：MAX_OPS_PER_PEER
 * 编译期定义：MAX_OPS_PER_PEER = (2*MAXCHANNELS*NCCL_MAX_WORK_ELEMENTS_P2P)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_OPS_PER_PEER (2*MAXCHANNELS*NCCL_MAX_WORK_ELEMENTS_P2P)
/* 中文逐项-常量 X1071：NCCL_MAX_LOCAL_RANKS
 * 编译期定义：NCCL_MAX_LOCAL_RANKS = 64。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MAX_LOCAL_RANKS 64
/* 中文精读：共享任务池的索引链表
 * ops 是固定容量 ProxyOp 数组，nextOps/nextOpsEnd 表示已发布工作链的头尾索引。
 * freeOps 按本地 rank 管理空闲项，生产者通过原子/锁协调取回可用项，减少动态分配。
 * mutex/cond 用于 CPU 发布和唤醒；进度线程把提交描述转为 ProxyArgs 后再独立推进。
 * 不要把这个元数据任务池误读成 NCCL_STEPS 个承载用户数据的协议环形缓冲。
 */
/* 中文逐项-类型 T0366：ncclProxyOpsPool
 * 类型职责：共享提交池、本地提交链视图和可复用运行参数池，降低每操作分配/进程通信开销。
 * 声明内可见成员（含嵌套成员）：ops、nextOps、nextOpsEnd、freeOps、mutex、cond。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyOpsPool {
/* 中文逐项-变量 V3104：ops
 * ops（struct ncclProxyOp；成员 ncclProxyOpsPool）：所处类型的provider函数表、代理待办数组或任务链，按声明类型读取，不能跨类型强转解释。
 */
  struct ncclProxyOp ops[MAX_OPS_PER_PEER*NCCL_MAX_LOCAL_RANKS];
/* 中文逐项-变量 V3105：nextOps
 * nextOps（int；成员 ncclProxyOpsPool）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 */
  volatile int nextOps;
/* 中文逐项-变量 V3106：nextOpsEnd
 * nextOpsEnd（int；成员 ncclProxyOpsPool）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 */
  volatile int nextOpsEnd;
/* 中文逐项-变量 V3107：freeOps
 * freeOps（int；成员 ncclProxyOpsPool）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 */
  volatile int freeOps[NCCL_MAX_LOCAL_RANKS];
/* 中文逐项-变量 V3108：mutex
 * mutex（pthread_mutex_t；成员 ncclProxyOpsPool）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
  pthread_mutex_t mutex;
/* 中文逐项-变量 V3109：cond
 * cond（pthread_cond_t；成员 ncclProxyOpsPool）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
  pthread_cond_t cond;
};

/* 中文逐项-类型 T0367：ncclProxyOps
 * 类型职责：共享提交池、本地提交链视图和可复用运行参数池，降低每操作分配/进程通信开销。
 * 声明内可见成员（含嵌套成员）：pool、count、freeOp、nextOps、nextOpsEnd。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyOps {
/* 中文逐项-变量 V3110：pool
 * pool（ncclProxyOpsPool；成员 ncclProxyOps）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 */
  ncclProxyOpsPool* pool;
/* 中文逐项-变量 V3111：count
 * count（int；成员 ncclProxyOps）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 */
  int count;
/* 中文逐项-变量 V3112：freeOp
 * freeOp（int；成员 ncclProxyOps）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 */
  int freeOp;
/* 中文逐项-变量 V3113：nextOps
 * nextOps（int；成员 ncclProxyOps）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 */
  int nextOps;
/* 中文逐项-变量 V3114：nextOpsEnd
 * nextOpsEnd（int；成员 ncclProxyOps）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 */
  int nextOpsEnd;
};

/* 中文逐项-类型 T0368：ncclProxySharedP2p
 * 类型职责：可共享协议缓冲/代理队列/后端连接及引用计数，按其维度复用而非全局任意合并。
 * 声明内可见成员（含嵌套成员）：refcount、size、cudaBuff、hostBuff、ipc、proxyAppend。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxySharedP2p {
/* 中文逐项-变量 V3115：refcount
 * refcount（int；成员 ncclProxySharedP2p）：共享对象引用计数；只有最后引用释放才允许回收真实资源。
 */
  int refcount;
/* 中文逐项-变量 V3116：size
 * size（int；成员 ncclProxySharedP2p）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  int size;
/* 中文逐项-变量 V3117：cudaBuff
 * cudaBuff（char；成员 ncclProxySharedP2p）：共享协议数据池的每步地址/大小及GPU/主机分配视图，按资源引用计数复用。
 */
  char* cudaBuff;
/* 中文逐项-变量 V3118：hostBuff
 * hostBuff（char；成员 ncclProxySharedP2p）：共享协议数据池的每步地址/大小及GPU/主机分配视图，按资源引用计数复用。
 */
  char* hostBuff;
/* 中文逐项-变量 V3119：ipc
 * ipc（cudaIpcMemHandle_t；成员 ncclProxySharedP2p）：可跨进程交换的CUDA内存导出句柄，与导入后本进程实际地址不同。
 */
  cudaIpcMemHandle_t ipc;
/* 中文逐项-变量 V3120：proxyAppend
 * proxyAppend（struct ncclProxyArgs；成员 ncclProxySharedP2p）：共享连接/通道当前最后代理操作引用，追加同连接工作时维持顺序。
 */
  struct ncclProxyArgs* proxyAppend[MAXCHANNELS]; // Separate send and recv
};

/* 中文逐项-类型 T0369：ncclProxySharedCollNet
 * 类型职责：可共享协议缓冲/代理队列/后端连接及引用计数，按其维度复用而非全局任意合并。
 * 声明内可见成员（含嵌套成员）：size、cudaBuff、hostBuff、proxyAppend、resources。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxySharedCollNet {
/* 中文逐项-变量 V3121：size
 * size（int；成员 ncclProxySharedCollNet）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  int size;
/* 中文逐项-变量 V3122：cudaBuff
 * cudaBuff（char；成员 ncclProxySharedCollNet）：共享协议数据池的每步地址/大小及GPU/主机分配视图，按资源引用计数复用。
 */
  char* cudaBuff;
/* 中文逐项-变量 V3123：hostBuff
 * hostBuff（char；成员 ncclProxySharedCollNet）：共享协议数据池的每步地址/大小及GPU/主机分配视图，按资源引用计数复用。
 */
  char* hostBuff;
/* 中文逐项-变量 V3124：proxyAppend
 * proxyAppend（struct ncclProxyArgs；成员 ncclProxySharedCollNet）：共享连接/通道当前最后代理操作引用，追加同连接工作时维持顺序。
 */
  struct ncclProxyArgs* proxyAppend[2*NCCL_MAX_NETDEVS];
/* 中文逐项-变量 V3125：resources
 * resources（void；成员 ncclProxySharedCollNet）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 */
  void* resources;
};

/* 中文逐项-类型 T0370：ncclProxyPeer
 * 类型职责：可共享协议缓冲/代理队列/后端连接及引用计数，按其维度复用而非全局任意合并。
 * 声明内可见成员（含嵌套成员）：send、recv。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyPeer {
/* 中文逐项-变量 V3126：send
 * send（struct ncclProxySharedP2p；成员 ncclProxyPeer）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 */
  struct ncclProxySharedP2p send;
/* 中文逐项-变量 V3127：recv
 * recv（struct ncclProxySharedP2p；成员 ncclProxyPeer）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 */
  struct ncclProxySharedP2p recv;
};

/* 中文导读：
 * 共享 NET 连接表中的一个远端条目，仍然按 channel 分别保存 sendComm/recvComm 及引用数。
 * 是否进入此表取决于 shared、后端 maxRecvs 和 NET_SHARED_COMMS；不能理解成所有 channel 共用一个 QP。
 */
/* 中文逐项-类型 T0371：ncclSharedNetComms
 * 类型职责：可共享协议缓冲/代理队列/后端连接及引用计数，按其维度复用而非全局任意合并。
 * 声明内可见成员（含嵌套成员）：sendComm、recvComm、sendRefCount、recvRefCount。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSharedNetComms {
/* 中文逐项-变量 V3128：sendComm
 * sendComm（void；成员 ncclSharedNetComms）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 */
  void* sendComm[MAXCHANNELS];
/* 中文逐项-变量 V3129：recvComm
 * recvComm（void；成员 ncclSharedNetComms）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 */
  void* recvComm[MAXCHANNELS];
/* 中文逐项-变量 V3130：sendRefCount
 * sendRefCount（int；成员 ncclSharedNetComms）：每channel共享发送/接收后端连接引用数，最后使用者释放才关闭连接。
 */
  int sendRefCount[MAXCHANNELS];
/* 中文逐项-变量 V3131：recvRefCount
 * recvRefCount（int；成员 ncclSharedNetComms）：每channel共享发送/接收后端连接引用数，最后使用者释放才关闭连接。
 */
  int recvRefCount[MAXCHANNELS];
};

struct ncclProxyPool;
/* 中文逐项-类型 T0372：ncclProxyProgressState
 * 类型职责：数据推进线程的活跃队列和整个代理服务总状态，把控制面与数据面组织分开。
 * 声明内可见成员（含嵌套成员）：opsPool、opsPoolShmSuffix、thread、stop、localPeers、netComms、collNet、active、pool、pools、nextOps。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyProgressState {
  // Used by main threads to send work to progress thread
/* 中文逐项-变量 V3132：opsPool
 * opsPool（struct ncclProxyOpsPool；成员 ncclProxyProgressState）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 */
  struct ncclProxyOpsPool* opsPool;
/* 中文逐项-变量 V3133：opsPoolShmSuffix
 * opsPoolShmSuffix（char；成员 ncclProxyProgressState）：共享代理任务池的文件名后缀，供其他本地进程建立相同对象映射。
 */
  char opsPoolShmSuffix[6];

/* 中文逐项-变量 V3134：thread
 * thread（pthread_t；成员 ncclProxyProgressState）：后台线程句柄和其任务/停止状态，资源回收前需要停止并join。
 */
  pthread_t thread;
/* 中文逐项-变量 V3135：stop
 * stop（bool；成员 ncclProxyProgressState）：辅助/代理线程生命周期状态，停止时仍须唤醒并 join 才能回收其资源。
 */
  bool stop;
/* 中文逐项-变量 V3136：localPeers
 * localPeers（struct ncclProxyPeer；成员 ncclProxyProgressState）：代理本地peer控制对象、控制socket及引导交换的地址表。
 */
  struct ncclProxyPeer** localPeers;
/* 中文逐项-变量 V3137：netComms
 * netComms（struct ncclSharedNetComms；成员 ncclProxyProgressState）：共享后端连接表及共享设备内存引用，按设备/peer/channel等维度查找。
 */
  struct ncclSharedNetComms* netComms[NCCL_MAX_NETDEVS];
/* 中文逐项-变量 V3138：collNet
 * collNet（struct ncclProxySharedCollNet；成员 ncclProxyProgressState）：图搜索任务是否服务网络集合归约，影响端点与速度约束。
 */
  struct ncclProxySharedCollNet collNet;
/* 中文逐项-变量 V3139：active
 * active（struct ncclProxyArgs；成员 ncclProxyProgressState）：代理当前可推进操作链表头，完成项移出后可能提升同连接后继。
 */
  struct ncclProxyArgs* active;
/* 中文逐项-变量 V3140：pool
 * pool（struct ncclProxyArgs；成员 ncclProxyProgressState）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 */
  struct ncclProxyArgs* pool;
/* 中文逐项-变量 V3141：pools
 * pools（struct ncclProxyPool；成员 ncclProxyProgressState）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 */
  struct ncclProxyPool* pools;
/* 中文逐项-变量 V3142：nextOps
 * nextOps（int；成员 ncclProxyProgressState）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 */
  int nextOps;
};

/* 中文导读：
 * 分开管理 service 线程与 progress 线程状态。
 * service 处理连接/分配/释放等请求；progress 轮询正在运行的数据传输。PXN 时发送 Proxy 可在本机其他 rank。
 */
/* 中文逐项-类型 T0373：ncclProxyState
 * 类型职责：数据推进线程的活跃队列和整个代理服务总状态，把控制面与数据面组织分开。
 * 声明内可见成员（含嵌套成员）：thread、listenSock、stop、peerAddresses、peerSocks、proxyOps、sharedDevMems、progressState。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyState {
  // Service thread
/* 中文逐项-变量 V3143：thread
 * thread（pthread_t；成员 ncclProxyState）：后台线程句柄和其任务/停止状态，资源回收前需要停止并join。
 */
  pthread_t thread;
/* 中文逐项-变量 V3144：listenSock
 * listenSock（struct ncclSocket；成员 ncclProxyState）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket* listenSock;
/* 中文逐项-变量 V3145：stop
 * stop（int；成员 ncclProxyState）：辅助/代理线程生命周期状态，停止时仍须唤醒并 join 才能回收其资源。
 */
  int stop;

  // Used by main thread
/* 中文逐项-变量 V3146：peerAddresses
 * peerAddresses（union ncclSocketAddress；成员 ncclProxyState）：代理本地peer控制对象、控制socket及引导交换的地址表。
 */
  union ncclSocketAddress* peerAddresses;
/* 中文逐项-变量 V3147：peerSocks
 * peerSocks（struct ncclSocket；成员 ncclProxyState）：代理本地peer控制对象、控制socket及引导交换的地址表。
 */
  struct ncclSocket* peerSocks;
/* 中文逐项-变量 V3148：proxyOps
 * proxyOps（struct ncclProxyOps；成员 ncclProxyState）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 */
  struct ncclProxyOps* proxyOps;
/* 中文逐项-变量 V3149：sharedDevMems
 * sharedDevMems（void；成员 ncclProxyState）：共享后端连接表及共享设备内存引用，按设备/peer/channel等维度查找。
 */
  void** sharedDevMems;

  // Progress thread
/* 中文逐项-变量 V3150：progressState
 * progressState（struct ncclProxyProgressState；成员 ncclProxyState）：communicator的代理总状态或数据推进子状态，分别组织服务控制和活跃请求。
 */
  struct ncclProxyProgressState progressState;
};

/* 中文逐项-类型 T0374：ncclProxyConnection
 * 类型职责：代理实际持有的传输连接资源及其分块存储池，由主线程控制请求索引访问。
 * 声明内可见成员（含嵌套成员）：send、transport、shared、localRank、sock、tcomm、proxyAppend、proxyAppendPtr、transportResources。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyConnection {
/* 中文逐项-变量 V3151：send、transport、shared
 * send（int；成员 ncclProxyConnection）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * transport（int；成员 ncclProxyConnection）：控制/连接的内部描述或transport类型编号，用于选择正确后台回调。
 * shared（int；成员 ncclProxyConnection）：是否使用共享缓冲资源的标志；不自动表示所有 channel 或全部 backend 连接合并。
 */
  int send, transport, shared;
/* 中文逐项-变量 V3152：localRank
 * localRank（int；成员 ncclProxyConnection）：本节点内 rank 下标，主要索引本地共享资源。
 */
  int localRank;
/* 中文逐项-变量 V3153：sock
 * sock（struct ncclSocket；成员 ncclProxyConnection）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket* sock;
/* 中文逐项-变量 V3154：tcomm
 * tcomm（struct ncclTransportComm；成员 ncclProxyConnection）：所选 transport 的回调表，规定建链、释放与代理推进方式。
 */
  struct ncclTransportComm* tcomm;
/* 中文逐项-变量 V3155：proxyAppend
 * proxyAppend（struct ncclProxyArgs；成员 ncclProxyConnection）：共享连接/通道当前最后代理操作引用，追加同连接工作时维持顺序。
 */
  struct ncclProxyArgs *proxyAppend;
/* 中文逐项-变量 V3156：proxyAppendPtr
 * proxyAppendPtr（struct ncclProxyArgs；成员 ncclProxyConnection）：同连接操作的后继链/尾部引用，维持共享连接上的执行顺序。
 */
  struct ncclProxyArgs **proxyAppendPtr;
/* 中文逐项-变量 V3157：transportResources
 * transportResources（void；成员 ncclProxyConnection）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 */
  void* transportResources;
};

typedef ncclResult_t (*threadFunc_t)(struct ncclProxyArgs*);

/* 中文逐项-类型 T0375：proxyMode
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum proxyMode {
/* 中文逐项-枚举项 X1072：proxyRing
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
  proxyRing = 0,
/* 中文逐项-枚举项 X1073：proxyFrom
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
  proxyFrom = 1,
/* 中文逐项-枚举项 X1074：proxyTo
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
  proxyTo = 2
};

/* 中文逐项-接口 X1075：ncclProxySaveColl
 * 接口职责：按 Ring/Tree/Pipeline/CollNet 模式展开必要邻居与方向，给相应连接保存代理工作。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxySaveColl(struct ncclComm* comm, struct ncclProxyOp* proxyOp,
 * int nranks);
 */
ncclResult_t ncclProxySaveColl(struct ncclComm* comm, struct ncclProxyOp* proxyOp, int nranks);
/* 中文逐项-接口 X1076：ncclProxyComputeP2p
 * 接口职责：根据已切好的 P2P 块计算协议尺寸、step 数和 channel/peer 标签，使 CPU/GPU 计划一致。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxyComputeP2p(struct ncclInfo* info, struct ncclProxyOp*
 * proxyOp);
 */
ncclResult_t ncclProxyComputeP2p(struct ncclInfo* info, struct ncclProxyOp* proxyOp);
/* 中文逐项-接口 X1077：ncclProxySaveP2p
 * 接口职责：按 P2P 任务方向和连接索引1保存代理需求，跳过 self 或无代理的本地路径。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxySaveP2p(struct ncclComm* comm, struct ncclProxyOp* proxyOp);
 */
ncclResult_t ncclProxySaveP2p(struct ncclComm* comm, struct ncclProxyOp* proxyOp);
/* 中文逐项-接口 X1078：ncclProxyStart
 * 接口职责：发布本批累积代理工作到相关本地代理的池，递增操作编号而不新建每消息线程。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxyStart(struct ncclComm* comm);
 */
ncclResult_t ncclProxyStart(struct ncclComm* comm);
/* 中文逐项-接口 X1079：ncclProxyInit
 * 接口职责：将 bootstrap 建立的监听 socket 和全组代理地址表接入当前 communicator 的 Proxy 状态。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxyInit(struct ncclComm* comm, struct ncclSocket* sock, union
 * ncclSocketAddress* peerAddresses);
 */
ncclResult_t ncclProxyInit(struct ncclComm* comm, struct ncclSocket* sock, union ncclSocketAddress* peerAddresses);
/* 中文逐项-接口 X1080：ncclProxyCreate
 * 接口职责：按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxyCreate(struct ncclComm* comm);
 */
ncclResult_t ncclProxyCreate(struct ncclComm* comm);
/* 中文逐项-接口 X1081：ncclProxyConnect
 * 接口职责：联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxyConnect(struct ncclComm* comm, int transport, int send, int
 * rank, struct ncclProxyConnector* proxyConn);
 */
ncclResult_t ncclProxyConnect(struct ncclComm* comm, int transport, int send, int rank, struct ncclProxyConnector* proxyConn);
/* 中文逐项-类型 T0376：ncclProxyMsgType
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclProxyMsgType {
/* 中文逐项-枚举项 X1082：ncclProxyMsgInit
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
  ncclProxyMsgInit = 1,
/* 中文逐项-枚举项 X1083：ncclProxyMsgSharedInit
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
  ncclProxyMsgSharedInit = 2,
/* 中文逐项-枚举项 X1084：ncclProxyMsgSetup
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
  ncclProxyMsgSetup = 3,
/* 中文逐项-枚举项 X1085：ncclProxyMsgConnect
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
  ncclProxyMsgConnect = 4,
/* 中文逐项-枚举项 X1086：ncclProxyMsgStart
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
  ncclProxyMsgStart = 5,
/* 中文逐项-枚举项 X1087：ncclProxyMsgClose
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
  ncclProxyMsgClose = 6,
/* 中文逐项-枚举项 X1088：ncclProxyMsgAbort
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：7
 */
  ncclProxyMsgAbort = 7,
/* 中文逐项-枚举项 X1089：ncclProxyMsgStop
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
  ncclProxyMsgStop = 8
};

/* 中文逐项-接口 X1090：ncclProxyCall
 * 接口职责：通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxyCall(struct ncclProxyConnector* proxyConn, int type, void*
 * reqBuff, int reqSize, void* respBuff, int respSize);
 */
ncclResult_t ncclProxyCall(struct ncclProxyConnector* proxyConn, int type, void* reqBuff, int reqSize, void* respBuff, int respSize);
/* 中文逐项-接口 X1091：ncclProxyDestroy
 * 接口职责：向代理服务发送停止/关闭信号并等待服务线程结束，完成代理总生命周期清理。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxyDestroy(struct ncclComm* comm);
 */
ncclResult_t ncclProxyDestroy(struct ncclComm* comm);
/* 中文逐项-接口 X1092：ncclProxyShmUnlink
 * 接口职责：移除代理共享内存对象名字，已有映射仍按各自引用和退出流程释放。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProxyShmUnlink(struct ncclComm* comm);
 */
ncclResult_t ncclProxyShmUnlink(struct ncclComm* comm);
#endif
