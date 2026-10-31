/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_TRANSPORT_H_
/* 中文逐项-常量 X1124：NCCL_TRANSPORT_H_
 * 编译期定义：NCCL_TRANSPORT_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TRANSPORT_H_

#include "devcomm.h"
#include "graph.h"
#include "nvmlwrap.h"
#include "core.h"

/* 中文逐项-常量 X1125：NTRANSPORTS
 * 编译期定义：NTRANSPORTS = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NTRANSPORTS 4
/* 中文逐项-常量 X1126：TRANSPORT_P2P
 * 编译期定义：TRANSPORT_P2P = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define TRANSPORT_P2P 0
/* 中文逐项-常量 X1127：TRANSPORT_SHM
 * 编译期定义：TRANSPORT_SHM = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define TRANSPORT_SHM 1
/* 中文逐项-常量 X1128：TRANSPORT_NET
 * 编译期定义：TRANSPORT_NET = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define TRANSPORT_NET 2
/* 中文逐项-常量 X1129：TRANSPORT_COLLNET
 * 编译期定义：TRANSPORT_COLLNET = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define TRANSPORT_COLLNET 3

#include "proxy.h"

/* 中文逐项-变量 V3176：ncclTransports
 * ncclTransports（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern struct ncclTransport ncclTransports[];

// Forward declarations
struct ncclRing;
struct ncclConnector;
struct ncclComm;

/* 中文精读：初始化时交换的参与者摘要
 * rank/cudaDev/busId 识别通信组编号与设备，hostHash/pidHash 区分主机和进程。
 * shmDev 帮助判断容器是否共享同一 /dev/shm，gdrSupport 等信息参与路径选择。
 * comm 指针仅在确认同进程等条件后具有相应本地意义，不能当作跨机器可直接解引用的地址。
 * 这份 peerInfo 在 bootstrap 中汇总，用于建图/选 transport；并不保存每次用户张量的内容。
 */
/* 中文逐项-类型 T0380：ncclPeerInfo
 * 类型职责：初始化交换的参与者身份与能力摘要，使各rank对主机/进程/设备/GDR有共同认识。
 * 声明内可见成员（含嵌套成员）：rank、cudaDev、netDev、gdrSupport、hostHash、pidHash、shmDev、busId、comm、cudaCompCap。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclPeerInfo {
/* 中文逐项-变量 V3177：rank
 * rank（int；成员 ncclPeerInfo）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V3178：cudaDev
 * cudaDev（int；成员 ncclPeerInfo）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 */
  int cudaDev;
/* 中文逐项-变量 V3179：netDev
 * netDev（int；成员 ncclPeerInfo）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int netDev;
/* 中文逐项-变量 V3180：gdrSupport
 * gdrSupport（int；成员 ncclPeerInfo）：设备能否注册GPU内存、进行网络集合归约及接受哪种指针类型的能力描述。
 */
  int gdrSupport;
/* 中文逐项-变量 V3181：hostHash
 * hostHash（uint64_t；成员 ncclPeerInfo）：用于区分同机/同进程的身份摘要；帮助判断哪些裸指针和共享资源可复用。
 */
  uint64_t hostHash;
/* 中文逐项-变量 V3182：pidHash
 * pidHash（uint64_t；成员 ncclPeerInfo）：用于区分同机/同进程的身份摘要；帮助判断哪些裸指针和共享资源可复用。
 */
  uint64_t pidHash;
/* 中文逐项-变量 V3183：shmDev
 * shmDev（dev_t；成员 ncclPeerInfo）：/dev/shm 对应设备标识，用于识别容器是否能共享同一主机共享内存对象。
 */
  dev_t shmDev;
/* 中文逐项-变量 V3184：busId
 * busId（int64_t；成员 ncclPeerInfo）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 */
  int64_t busId;
/* 中文逐项-变量 V3185：comm
 * comm（struct ncclComm；成员 ncclPeerInfo）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  struct ncclComm* comm;
/* 中文逐项-变量 V3186：cudaCompCap
 * cudaCompCap（int；成员 ncclPeerInfo）：GPU计算能力整数编码，影响NVLink速率、协议支持和编译特化选择。
 */
  int cudaCompCap;
};

/* 中文逐项-常量 X1130：CONNECT_SIZE
 * 编译期定义：CONNECT_SIZE = 128。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define CONNECT_SIZE 128
/* 中文逐项-类型 T0381：ncclConnect
 * 类型职责：固定大小的可交换建链句柄，各transport以自己的结构布局写入并由对端解释。
 * 声明内可见成员（含嵌套成员）：data。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclConnect {
/* 中文逐项-变量 V3187：data
 * data（char；成员 ncclConnect）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 */
  char data[CONNECT_SIZE];
};

/* 中文导读：
 * 一条方向的 transport 回调契约：主线程执行 setup/connect/free，Proxy 执行后端资源操作及数据推进。
 * 普通 transport 与网络后端 ncclNet_t 属于不同抽象层；NET transport 再通过 ncclNet_t 使用 IB/Socket/插件。
 */
/* 中文精读：setup/connect/free 与 proxy 回调分工
 * setup 准备本地资源并生成可交换的 ncclConnect 句柄；connect 消费对端句柄完成建链。
 * free 回收这个逻辑 connector 的资源；Proxy 侧还有服务线程的 setup/connect/free 回调。
 * proxyProgress 是数据面每次推进的入口；为空表示这条连接不需要 CPU 数据面代理。
 * 上层只依赖这组函数表，P2P/SHM/NET 各自保存不同的 transportResources。
 */
/* 中文逐项-类型 T0382：ncclTransportComm
 * 类型职责：某方向的传输回调表及完整transport描述，封装能力判断、建链、释放和代理推进。
 * 声明内可见成员（含嵌套成员）：setup、connect、free、proxySharedInit、proxySetup、proxyConnect、proxyFree、proxyProgress。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTransportComm {
/* 中文逐项-变量 V3188：setup
 * setup 是函数指针字段/变量：分配本地方向资源并生成可交换建链句柄；函数表保存可调用入口，不是调用返回的数据。完整声明：(*setup)(struct ncclComm* comm, struct ncclTopoGraph*
 * graph, struct ncclPeerInfo*, struct ncclPeerInfo*, struct ncclConnect*, struct ncclConnector*, int channelId, int
 * connIndex)。
 * 回调参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 回调参数 <未命名参数>（struct ncclPeerInfo*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 <未命名参数>（struct ncclPeerInfo*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 <未命名参数>（struct ncclConnect*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 <未命名参数>（struct ncclConnector*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 回调参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * setup（ncclResult_t；成员 ncclTransportComm）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*setup)(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo*, struct ncclPeerInfo*, struct ncclConnect*, struct ncclConnector*, int channelId, int connIndex);
/* 中文逐项-变量 V3189：connect
 * connect 是函数指针字段/变量：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*connect)(struct ncclComm* comm,
 * struct ncclConnect*, int nranks, int rank, struct ncclConnector*)。
 * 回调参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 <未命名参数>（struct ncclConnect*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 回调参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 回调参数 <未命名参数>（struct ncclConnector*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * connect（ncclResult_t；成员 ncclTransportComm）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
  ncclResult_t (*connect)(struct ncclComm* comm, struct ncclConnect*, int nranks, int rank, struct ncclConnector*);
/* 中文逐项-变量 V3190：free
 * free 是函数指针字段/变量：释放该方向连接资源，最后共享引用才关闭真正对象；函数表保存可调用入口，不是调用返回的数据。完整声明：(*free)(struct ncclConnector*)。
 * 回调参数 <未命名参数>（struct ncclConnector*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * free（ncclResult_t；成员 ncclTransportComm）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*free)(struct ncclConnector*);
/* 中文逐项-变量 V3191：proxySharedInit
 * proxySharedInit
 * 是函数指针字段/变量：由该资源库/provider提供的proxySharedInit操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*proxySharedInit)(struct
 * ncclProxyConnection* connection, struct ncclComm* comm, int nChannels)。
 * 回调参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 回调参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 nChannels（int nChannels）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * proxySharedInit（ncclResult_t；成员 ncclTransportComm）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*proxySharedInit)(struct ncclProxyConnection* connection, struct ncclComm* comm, int nChannels);
/* 中文逐项-变量 V3192：proxySetup
 * proxySetup 是函数指针字段/变量：由实际代理进程准备后端方向资源，可用done输出表示多轮推进是否完成；函数表保存可调用入口，不是调用返回的数据。完整声明：(*proxySetup)(struct
 * ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize,
 * int* done)。
 * 回调参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 回调参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 回调参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 回调参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 回调参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 回调参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * proxySetup（ncclResult_t；成员 ncclTransportComm）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*proxySetup)(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done);
/* 中文逐项-变量 V3193：proxyConnect
 * proxyConnect 是函数指针字段/变量：在代理侧完成后端连接、缓冲和注册映射，多轮调用直到done；函数表保存可调用入口，不是调用返回的数据。完整声明：(*proxyConnect)(struct
 * ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize,
 * int* done)。
 * 回调参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 回调参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 回调参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 回调参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 回调参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 回调参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * proxyConnect（ncclResult_t；成员 ncclTransportComm）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*proxyConnect)(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done);
/* 中文逐项-变量 V3194：proxyFree
 * proxyFree 是函数指针字段/变量：在资源所属代理释放连接/注册/共享引用；函数表保存可调用入口，不是调用返回的数据。完整声明：(*proxyFree)(struct ncclProxyConnection*
 * connection, struct ncclComm* comm)。
 * 回调参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 回调参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * proxyFree（ncclResult_t；成员 ncclTransportComm）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*proxyFree)(struct ncclProxyConnection* connection, struct ncclComm* comm);
/* 中文逐项-变量 V3195：proxyProgress
 * proxyProgress 是函数指针字段/变量：非阻塞推进已建连接上的数据传输状态机；函数表保存可调用入口，不是调用返回的数据。完整声明：(*proxyProgress)(struct ncclComm* comm,
 * struct ncclProxyArgs*)。
 * 回调参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 <未命名参数>（struct ncclProxyArgs*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * proxyProgress（ncclResult_t；成员 ncclTransportComm）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*proxyProgress)(struct ncclComm* comm, struct ncclProxyArgs*);
};

/* 中文逐项-类型 T0383：ncclTransport
 * 类型职责：某方向的传输回调表及完整transport描述，封装能力判断、建链、释放和代理推进。
 * 声明内可见成员（含嵌套成员）：name、canConnect、send、recv。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTransport {
/* 中文逐项-变量 V3196：name
 * name（char；成员 ncclTransport）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  const char name[4];
/* 中文逐项-变量 V3197：canConnect
 * canConnect 是函数指针字段/变量：判断当前peer对和拓扑是否可用该transport，并通过输出参数返回布尔结果；函数表保存可调用入口，不是调用返回的数据。完整声明：(*canConnect)(int*, struct
 * ncclTopoSystem* topo, struct ncclTopoGraph* graph, struct ncclPeerInfo*, struct ncclPeerInfo*)。
 * 回调参数 <未命名参数>（int*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 topo（struct ncclTopoSystem* topo）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 回调参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 回调参数 <未命名参数>（struct ncclPeerInfo*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 <未命名参数>（struct ncclPeerInfo*）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * canConnect（ncclResult_t；成员 ncclTransport）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*canConnect)(int*, struct ncclTopoSystem* topo, struct ncclTopoGraph* graph, struct ncclPeerInfo*, struct ncclPeerInfo*);
/* 中文逐项-变量 V3198：send
 * send（struct ncclTransportComm；成员 ncclTransport）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 */
  struct ncclTransportComm send;
/* 中文逐项-变量 V3199：recv
 * recv（struct ncclTransportComm；成员 ncclTransport）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 */
  struct ncclTransportComm recv;
};

/* 中文逐项-接口 X1131：ncclTransportP2pConnect
 * 接口职责：为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTransportP2pConnect(struct ncclComm* comm, struct ncclChannel*
 * channel, int nrecv, int* peerRecv, int nsend, int* peerSend, int connIndex);
 */
ncclResult_t ncclTransportP2pConnect(struct ncclComm* comm, struct ncclChannel* channel, int nrecv, int* peerRecv, int nsend, int* peerSend, int connIndex);
/* 中文逐项-接口 X1132：ncclTransportP2pSetup
 * 接口职责：按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTransportP2pSetup(struct ncclComm* comm, struct ncclTopoGraph*
 * graph, int connIndex, int* highestTransportType=NULL);
 */
ncclResult_t ncclTransportP2pSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, int connIndex, int* highestTransportType=NULL);

/* 中文逐项-类型 T0384：<匿名enum>
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum { /* 中文逐项-枚举项 X1133：collNetRecv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */collNetRecv=0, /* 中文逐项-枚举项 X1134：collNetSend
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */collNetSend=1 };
/* 中文逐项-接口 X1135：ncclTransportCollNetSetup
 * 接口职责：为网络归约 head 建立特殊 CollNet 连接并交换网络参与者句柄。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int ncclTransportCollNetSetup(struct ncclComm* comm, struct ncclTopoGraph*
 * collNetGraph, struct ncclChannel* channel, int masterRank, int masterPeer, int collNetGraphChannelId, int type);
 */
int ncclTransportCollNetSetup(struct ncclComm* comm, struct ncclTopoGraph* collNetGraph, struct ncclChannel* channel, int masterRank, int masterPeer, int collNetGraphChannelId, int type);
/* 中文逐项-接口 X1136：ncclTransportCollNetCheck
 * 接口职责：全组汇总 CollNet 建链失败状态，一处不支持时一致回退，避免各 rank 选择不一致。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTransportCollNetCheck(struct ncclComm* comm, int
 * collNetSetupFail);
 */
ncclResult_t ncclTransportCollNetCheck(struct ncclComm* comm, int collNetSetupFail);
/* 中文逐项-接口 X1137：ncclTransportCollNetFree
 * 接口职责：关闭已创建的 CollNet 特殊连接，配合初始化失败回退或整体销毁。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTransportCollNetFree(struct ncclComm* comm);
 */
ncclResult_t ncclTransportCollNetFree(struct ncclComm* comm);
#endif
