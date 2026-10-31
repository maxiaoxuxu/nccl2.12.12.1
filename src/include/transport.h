/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_TRANSPORT_H_
#define NCCL_TRANSPORT_H_

#include "devcomm.h"
#include "graph.h"
#include "nvmlwrap.h"
#include "core.h"

#define NTRANSPORTS 4
#define TRANSPORT_P2P 0
#define TRANSPORT_SHM 1
#define TRANSPORT_NET 2
#define TRANSPORT_COLLNET 3

#include "proxy.h"

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
struct ncclPeerInfo {
  int rank;
  int cudaDev;
  int netDev;
  int gdrSupport;
  uint64_t hostHash;
  uint64_t pidHash;
  dev_t shmDev;
  int64_t busId;
  struct ncclComm* comm;
  int cudaCompCap;
};

#define CONNECT_SIZE 128
struct ncclConnect {
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
struct ncclTransportComm {
  ncclResult_t (*setup)(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo*, struct ncclPeerInfo*, struct ncclConnect*, struct ncclConnector*, int channelId, int connIndex);
  ncclResult_t (*connect)(struct ncclComm* comm, struct ncclConnect*, int nranks, int rank, struct ncclConnector*);
  ncclResult_t (*free)(struct ncclConnector*);
  ncclResult_t (*proxySharedInit)(struct ncclProxyConnection* connection, struct ncclComm* comm, int nChannels);
  ncclResult_t (*proxySetup)(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done);
  ncclResult_t (*proxyConnect)(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done);
  ncclResult_t (*proxyFree)(struct ncclProxyConnection* connection, struct ncclComm* comm);
  ncclResult_t (*proxyProgress)(struct ncclComm* comm, struct ncclProxyArgs*);
};

struct ncclTransport {
  const char name[4];
  ncclResult_t (*canConnect)(int*, struct ncclTopoSystem* topo, struct ncclTopoGraph* graph, struct ncclPeerInfo*, struct ncclPeerInfo*);
  struct ncclTransportComm send;
  struct ncclTransportComm recv;
};

ncclResult_t ncclTransportP2pConnect(struct ncclComm* comm, struct ncclChannel* channel, int nrecv, int* peerRecv, int nsend, int* peerSend, int connIndex);
ncclResult_t ncclTransportP2pSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, int connIndex, int* highestTransportType=NULL);

enum { collNetRecv=0, collNetSend=1 };
int ncclTransportCollNetSetup(struct ncclComm* comm, struct ncclTopoGraph* collNetGraph, struct ncclChannel* channel, int masterRank, int masterPeer, int collNetGraphChannelId, int type);
ncclResult_t ncclTransportCollNetCheck(struct ncclComm* comm, int collNetSetupFail);
ncclResult_t ncclTransportCollNetFree(struct ncclComm* comm);
#endif
