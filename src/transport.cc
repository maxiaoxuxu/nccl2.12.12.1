/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "info.h"
#include "bootstrap.h"
/* 中文逐项-常量 X1297：ENABLE_TIMER
 * 编译期定义：ENABLE_TIMER = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ENABLE_TIMER 0
#include "timer.h"

/* 中文逐项-变量 V4658：p2pTransport
 * p2pTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：p2pTransport,。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern struct ncclTransport p2pTransport;
/* 中文逐项-变量 V4659：shmTransport
 * shmTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：shmTransport,。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern struct ncclTransport shmTransport;
/* 中文逐项-变量 V4660：netTransport
 * netTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：netTransport,。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern struct ncclTransport netTransport;
/* 中文逐项-变量 V4661：collNetTransport
 * collNetTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：collNetTransport。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern struct ncclTransport collNetTransport;

/* 中文逐项-变量 V4662：ncclTransports
 * ncclTransports（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclTransports = { p2pTransport, shmTransport, netTransport, collNetTransport
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct ncclTransport ncclTransports[NTRANSPORTS] = {
  p2pTransport,
  shmTransport,
  netTransport,
  collNetTransport
};

/* 中文逐项-函数 F1172：selectTransport
 * 用途与位置：按 P2P、SHM、NET、CollNet 顺序检查连接能力并调用首个可用后端的 setup。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 connect（struct ncclConnect* connect）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 参数 transportType（int* transportType）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport.cc::ncclTransportP2pSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；transport->canConnect；transportComm->setup。
 * 需要对照的边界/推进条件：t<NTRANSPORTS；ret；transportType。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclSystemError;。CHECK类宏还可能提前返回/跳转。
 */
template <int type>
/* 中文导读：
 * 按照 ncclTransports 的顺序询问 canConnect，命中后调用该 transport 的 setup。
 * 这里的 P2pSetup 是逻辑对端建链框架，内部可能选择 NET，因此函数名不保证数据走 NVLink。
 */
static ncclResult_t selectTransport(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclConnect* connect, int channelId, int peer, int connIndex, int* transportType) {
/* 中文逐项-变量 V4663：myInfo
 * myInfo（struct ncclPeerInfo；函数 selectTransport 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 myInfo = comm->peerInfo+comm->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclPeerInfo* myInfo = comm->peerInfo+comm->rank;
/* 中文逐项-变量 V4664：peerInfo
 * peerInfo（struct ncclPeerInfo；函数 selectTransport 内）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 初值/来源 peerInfo = comm->peerInfo+peer；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclPeerInfo* peerInfo = comm->peerInfo+peer;
/* 中文逐项-变量 V4665：connector
 * connector（struct ncclConnector；函数 selectTransport 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 connector = (type == 1) ? comm->channels[channelId].peers[peer].send + connIndex :
 * comm->channels[channelId].peers[peer].recv + connIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclConnector* connector = (type == 1) ? comm->channels[channelId].peers[peer].send + connIndex :
                                                  comm->channels[channelId].peers[peer].recv + connIndex;
  for (/* 中文逐项-变量 V4666：t
 * t（int；函数 selectTransport 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：t<NTRANSPORTS；循环体与增量表达式决定每轮如何推进。
 */int t=0; t<NTRANSPORTS; t++) {
/* 中文逐项-变量 V4667：transport
 * transport（struct ncclTransport；函数 selectTransport 内）：控制/连接的内部描述或transport类型编号，用于选择正确后台回调。
 * 初值/来源 transport = ncclTransports+t；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclTransport *transport = ncclTransports+t;
/* 中文逐项-变量 V4668：transportComm
 * transportComm（struct ncclTransportComm；函数 selectTransport 内）：所选 transport 的回调表，规定建链、释放与代理推进方式。
 * 初值/来源 transportComm = type == 1 ? &transport->send : &transport->recv；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTransportComm* transportComm = type == 1 ? &transport->send : &transport->recv;
/* 中文逐项-变量 V4669：ret
 * ret（int；函数 selectTransport 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int ret = 0;
    NCCLCHECK(transport->canConnect(&ret, comm->topo, graph, myInfo, peerInfo));
    if (ret) {
      connector->transportComm = transportComm;
      NCCLCHECK(transportComm->setup(comm, graph, myInfo, peerInfo, connect, connector, channelId, connIndex));
      if (transportType) *transportType = t;
      return ncclSuccess;
    }
  }
  WARN("No transport found for rank %d[%lx] -> rank %d[%lx]", myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId);
  return ncclSystemError;
}

/* 中文导读：
 * 只把需要连接的 channel 写进每 peer 的 send/recv 位图，跳过 self、无效 peer 和已建好的槽位。
 * 真正的资源分配与连接握手发生在 ncclTransportP2pSetup 中，而不是本函数。
 */
/* 中文精读：先标记需要的连接，再批量执行建链
 * 把给定邻居的 channel 位写入 connectRecv/connectSend 掩码；已 connected 的槽位跳过。
 * 此处主要记录需求，真正 setup/connect 在 ncclTransportP2pSetup 完成。
 * 集合通信预先标记拓扑邻居，Send/Recv 则可在首次遇到新 peer 时按需标记连接索引 1。
 * 所以 init 完成后的 QP/MR 不一定是未来所有 P2P 负载所需的峰值。
 */
/* 中文逐项-函数 F1173：ncclTransportP2pConnect
 * 用途与位置：为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 channel（struct ncclChannel* channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 nrecv（int nrecv）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 peerRecv（int* peerRecv）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nsend（int nsend）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 peerSend（int* peerSend）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<nrecv；peer == -1 || peer >= comm->nRanks || peer == comm->rank ||
 * channel->peers[peer].recv[connIndex].connected；i<nsend；peer == -1 || peer >= comm->nRanks || peer == comm->rank ||
 * channel->peers[peer].send[connIndex].connected。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTransportP2pConnect(struct ncclComm* comm, struct ncclChannel* channel, int nrecv, int* peerRecv, int nsend, int* peerSend, int connIndex) {
  TRACE(NCCL_INIT, "nsend %d nrecv %d", nsend, nrecv);
/* 中文逐项-变量 V4670：mask
 * mask（uint32_t；函数 ncclTransportP2pConnect 内）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 * 初值/来源 mask = 1 << channel->id；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  uint32_t mask = 1 << channel->id;
  for (/* 中文逐项-变量 V4671：i
 * i（int；函数 ncclTransportP2pConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nrecv；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nrecv; i++) {
/* 中文逐项-变量 V4672：peer
 * peer（int；函数 ncclTransportP2pConnect 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = peerRecv[i]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int peer = peerRecv[i];
    if (peer == -1 || peer >= comm->nRanks || peer == comm->rank || channel->peers[peer].recv[connIndex].connected) continue;
    comm->connectRecv[peer] |= mask;
  }
  for (/* 中文逐项-变量 V4673：i
 * i（int；函数 ncclTransportP2pConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nsend；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nsend; i++) {
/* 中文逐项-变量 V4674：peer
 * peer（int；函数 ncclTransportP2pConnect 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = peerSend[i]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int peer = peerSend[i];
    if (peer == -1 || peer >= comm->nRanks || peer == comm->rank || channel->peers[peer].send[connIndex].connected) continue;
    comm->connectSend[peer] |= mask;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F1174：dumpData
 * 用途与位置：按字节/机器字打印 transport 建链句柄内容，仅作控制元数据诊断。
 * 参数 data（struct ncclConnect* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 ndata（int ndata）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：n<ndata；i<sizeof(struct ncclConnect。各条件所属分支见紧邻函数体。
 */
void dumpData(struct ncclConnect* data, int ndata) {
  for (/* 中文逐项-变量 V4675：n
 * n（int；函数 dumpData 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<ndata；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<ndata; n++) {
    printf("[%d] ", n);
/* 中文逐项-变量 V4676：d
 * d（uint8_t；函数 dumpData 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = (uint8_t*)data；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    uint8_t* d = (uint8_t*)data;
    for (/* 中文逐项-变量 V4677：i
 * i（int；函数 dumpData 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<sizeof(struct ncclConnect)；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<sizeof(struct ncclConnect); i++) printf("%02x", d[i]);
    printf("\n");
  }
}

/* 中文导读：
 * 消费待连接位图：选择 transport/setup → bootstrap 交换连接句柄 → connect → 同步设备端 connector。
 * connIndex 区分连接用途；一次逻辑边可能使用本地 P2P，也可能落到网络后端的一组 QP。
 */
/* 中文逐项-函数 F1175：ncclTransportP2pSetup
 * 用途与位置：按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 参数 highestTransportType（int* highestTransportType）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/group.cc::ncclAsyncThreadPreconnect；src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；TIME_PRINT；TIME_START；TIME_STOP；bootstrapRecv；bootstrapSend；conn->transportComm->connect；cudaMemcpyAsync；cudaStreamCreateWithFlags；cudaStreamDestroy；cudaStreamSynchronize；selectTransport<0>；selectTransport<1>。
 * 需要对照的边界/推进条件：i<comm->nRanks；c<MAXCHANNELS；recvMask & (1<<c；type > highestType。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTransportP2pSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, int connIndex, int* highestTransportType/*=NULL*/) {
  // Stream used during transport setup; need for P2P pre-connect + CUDA Graph
/* 中文逐项-变量 V4678：transportSetupStream
 * transportSetupStream（cudaStream_t；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaStreamCreateWithFlags(&transportSetupStream,
 * cudaStreamNonBlocking));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cudaStream_t transportSetupStream;
  CUDACHECK(cudaStreamCreateWithFlags(&transportSetupStream, cudaStreamNonBlocking));
/* 中文逐项-变量 V4679：highestType
 * highestType（int；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 highestType = TRANSPORT_P2P；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int highestType = TRANSPORT_P2P;  // track highest transport type

/* 中文逐项-变量 V4680：data
 * data（struct ncclConnect；函数 ncclTransportP2pSetup 内）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 此处未显式给初值；后续首个文本使用点：struct ncclConnect* recvData = data;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclConnect data[2*MAXCHANNELS];
  for (/* 中文逐项-变量 V4681：i
 * i（int；函数 ncclTransportP2pSetup 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<comm->nRanks；循环体与增量表达式决定每轮如何推进。
 */int i=1; i<comm->nRanks; i++) {
/* 中文逐项-变量 V4682：bootstrapTag
 * bootstrapTag（int；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bootstrapTag = (i<<8) + (graph ? graph->id+1 : 0)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int bootstrapTag = (i<<8) + (graph ? graph->id+1 : 0);
/* 中文逐项-变量 V4683：recvPeer
 * recvPeer（int；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvPeer = (comm->rank - i + comm->nRanks) % comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int recvPeer = (comm->rank - i + comm->nRanks) % comm->nRanks;
/* 中文逐项-变量 V4684：sendPeer
 * sendPeer（int；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendPeer = (comm->rank + i) % comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int sendPeer = (comm->rank + i) % comm->nRanks;
/* 中文逐项-变量 V4685：recvMask
 * recvMask（uint32_t；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvMask = comm->connectRecv[recvPeer]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    uint32_t recvMask = comm->connectRecv[recvPeer];
/* 中文逐项-变量 V4686：sendMask
 * sendMask（uint32_t；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendMask = comm->connectSend[sendPeer]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    uint32_t sendMask = comm->connectSend[sendPeer];

/* 中文逐项-变量 V4687：recvData
 * recvData（struct ncclConnect；函数 ncclTransportP2pSetup 内）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 初值/来源 recvData = data；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    struct ncclConnect* recvData = data;
/* 中文逐项-变量 V4688：sendChannels、recvChannels
 * sendChannels（int；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendChannels = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * recvChannels（int；函数 ncclTransportP2pSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvChannels = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int sendChannels = 0, recvChannels = 0;
/* 中文逐项-变量 V4689：type
 * type（int；函数 ncclTransportP2pSetup 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(selectTransport<0>(comm, graph, recvData+recvChannels++, c, recvPeer, connIndex,
 * &type));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int type;
    TIME_START(0);
    for (/* 中文逐项-变量 V4690：c
 * c（int；函数 ncclTransportP2pSetup 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<MAXCHANNELS；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<MAXCHANNELS; c++) {
      if (recvMask & (1<<c)) {
        NCCLCHECK(selectTransport<0>(comm, graph, recvData+recvChannels++, c, recvPeer, connIndex, &type));
        if (type > highestType) highestType = type;
      }
    }
    TIME_STOP(0);
    TIME_START(1);
/* 中文逐项-变量 V4691：sendData
 * sendData（struct ncclConnect；函数 ncclTransportP2pSetup 内）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 初值/来源 sendData = recvData+recvChannels；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclConnect* sendData = recvData+recvChannels;
    for (/* 中文逐项-变量 V4692：c
 * c（int；函数 ncclTransportP2pSetup 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<MAXCHANNELS；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<MAXCHANNELS; c++) {
      if (sendMask & (1<<c)) {
        NCCLCHECK(selectTransport<1>(comm, graph, sendData+sendChannels++, c, sendPeer, connIndex, &type));
        if (type > highestType) highestType = type;
      }
    }
    TIME_STOP(1);

    TIME_START(2);
    if (sendPeer == recvPeer) {
      if (recvChannels+sendChannels) {
         NCCLCHECK(bootstrapSend(comm->bootstrap, recvPeer, bootstrapTag, data, sizeof(struct ncclConnect)*(recvChannels+sendChannels)));
         NCCLCHECK(bootstrapRecv(comm->bootstrap, recvPeer, bootstrapTag, data, sizeof(struct ncclConnect)*(recvChannels+sendChannels)));
         sendData = data;
         recvData = data+sendChannels;
      }
    } else {
      if (recvChannels) NCCLCHECK(bootstrapSend(comm->bootstrap, recvPeer, bootstrapTag, recvData, sizeof(struct ncclConnect)*recvChannels));
      if (sendChannels) NCCLCHECK(bootstrapSend(comm->bootstrap, sendPeer, bootstrapTag, sendData, sizeof(struct ncclConnect)*sendChannels));
      if (sendChannels) NCCLCHECK(bootstrapRecv(comm->bootstrap, sendPeer, bootstrapTag, sendData, sizeof(struct ncclConnect)*sendChannels));
      if (recvChannels) NCCLCHECK(bootstrapRecv(comm->bootstrap, recvPeer, bootstrapTag, recvData, sizeof(struct ncclConnect)*recvChannels));
    }
    TIME_STOP(2);

    TIME_START(3);
    for (/* 中文逐项-变量 V4693：c
 * c（int；函数 ncclTransportP2pSetup 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<MAXCHANNELS；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<MAXCHANNELS; c++) {
      if (sendMask & (1<<c)) {
/* 中文逐项-变量 V4694：conn
 * conn（struct ncclConnector；函数 ncclTransportP2pSetup 内）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 初值/来源 conn = comm->channels[c].peers[sendPeer].send + connIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclConnector* conn = comm->channels[c].peers[sendPeer].send + connIndex;
        NCCLCHECK(conn->transportComm->connect(comm, sendData++, 1, comm->rank, conn));
        conn->connected = 1;
        CUDACHECK(cudaMemcpyAsync(comm->channels[c].devPeers[sendPeer].send+connIndex, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice, transportSetupStream));
      }
    }
    TIME_STOP(3);
    TIME_START(4);
    for (/* 中文逐项-变量 V4695：c
 * c（int；函数 ncclTransportP2pSetup 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<MAXCHANNELS；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<MAXCHANNELS; c++) {
      if (recvMask & (1<<c)) {
/* 中文逐项-变量 V4696：conn
 * conn（struct ncclConnector；函数 ncclTransportP2pSetup 内）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 初值/来源 conn = comm->channels[c].peers[recvPeer].recv + connIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclConnector* conn = comm->channels[c].peers[recvPeer].recv + connIndex;
        NCCLCHECK(conn->transportComm->connect(comm, recvData++, 1, comm->rank, conn));
        conn->connected = 1;
        CUDACHECK(cudaMemcpyAsync(comm->channels[c].devPeers[recvPeer].recv+connIndex, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice, transportSetupStream));
      }
    }
    TIME_STOP(4);
    comm->connectRecv[recvPeer] = comm->connectSend[sendPeer] = 0;
  }
  CUDACHECK(cudaStreamSynchronize(transportSetupStream));
  CUDACHECK(cudaStreamDestroy(transportSetupStream));
  if (highestTransportType != NULL) *highestTransportType = highestType;
  TIME_PRINT("P2P Setup/Connect");
  return ncclSuccess;
}

/* 中文逐项-变量 V4697：collNetTransport
 * collNetTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(collNetTransport.canConnect(&support, comm->topo, collNetGraph, myInfo,
 * peerInfo));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern struct ncclTransport collNetTransport;

// All ranks must participate in collNetSetup call
// We do not NCCLCHECK this call because we would fall back to P2P network in case CollNet setup fails
/* 中文逐项-函数 F1176：ncclTransportCollNetSetup
 * 用途与位置：为网络归约 head 建立特殊 CollNet 连接并交换网络参与者句柄。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 collNetGraph（struct ncclTopoGraph* collNetGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 channel（struct ncclChannel* channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 masterRank（int masterRank）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 masterPeer（int masterPeer）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 collNetGraphChannelId（int collNetGraphChannelId）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：CUDACHECKGOTO；NCCLCHECK；NCCLCHECKGOTO；bootstrapAllGather；bootstrapRecv；bootstrapSend；collNetTransport.canConnect；cudaMemcpy；free；memcpy；ncclCalloc；transportComm->connect；transportComm->setup。
 * 需要对照的边界/推进条件：isMaster；isMaster && type == collNetSend；isMaster && support；type == collNetRecv。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return fail;。CHECK类宏还可能提前返回/跳转。
 */
int ncclTransportCollNetSetup(struct ncclComm* comm, struct ncclTopoGraph* collNetGraph, struct ncclChannel* channel, int masterRank, int masterPeer, int collNetGraphChannelId, int type) {
/* 中文逐项-变量 V4698：fail
 * fail（int；函数 ncclTransportCollNetSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 fail = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int fail = 1;
/* 中文逐项-变量 V4699：rank
 * rank（int；函数 ncclTransportCollNetSetup 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = comm->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = comm->rank;
/* 中文逐项-变量 V4700：nranks
 * nranks（int；函数 ncclTransportCollNetSetup 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nranks = comm->nRanks;
/* 中文逐项-变量 V4701：nMasters
 * nMasters（int；函数 ncclTransportCollNetSetup 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nMasters = comm->nNodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nMasters = comm->nNodes;
/* 中文逐项-变量 V4702：rankInCollNet
 * rankInCollNet（int；函数 ncclTransportCollNetSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rankInCollNet = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int rankInCollNet = -1;
/* 中文逐项-变量 V4703：isMaster
 * isMaster（int；函数 ncclTransportCollNetSetup 内）：当前参与者是否承担该分组/初始化任务的主协调者角色，控制只需执行一次的动作。
 * 初值/来源 isMaster = (rank == masterRank) ? 1 : 0；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int isMaster = (rank == masterRank) ? 1 : 0;
/* 中文逐项-变量 V4704：sendrecvExchange
 * sendrecvExchange（struct { int collNetRank; ncclConnect connect; }；函数 ncclTransportCollNetSetup
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(bootstrapRecv(comm->bootstrap, masterPeer, collNetGraph->id, &sendrecvExchange,
 * sizeof(sendrecvExchange)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0459：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：collNetRank、connect。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct {
/* 中文逐项-变量 V4705：collNetRank
 * collNetRank（int；成员 <匿名>）：当前网络集合参与者编号，可能只枚举head，范围与全体GPU rank不同。
 */
    int collNetRank;
/* 中文逐项-变量 V4706：connect
 * connect（ncclConnect；成员 <匿名>）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
    ncclConnect connect;
  } sendrecvExchange;

  // check if we can connect to collnet, whose root is the nranks-th rank
/* 中文逐项-变量 V4707：myInfo、peerInfo
 * myInfo（struct ncclPeerInfo；函数 ncclTransportCollNetSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 myInfo = comm->peerInfo+rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * peerInfo（struct ncclPeerInfo；函数 ncclTransportCollNetSetup 内）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 初值/来源 peerInfo = comm->peerInfo+nranks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclPeerInfo *myInfo = comm->peerInfo+rank, *peerInfo = comm->peerInfo+nranks;
  peerInfo->rank = nranks;
/* 中文逐项-变量 V4708：support
 * support（int；函数 ncclTransportCollNetSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 support = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int support = 1;
  if (isMaster) {
    NCCLCHECK(collNetTransport.canConnect(&support, comm->topo, collNetGraph, myInfo, peerInfo));
  }

  // send master receives connect info from peer recv master
  if (isMaster && type == collNetSend) {
    NCCLCHECK(bootstrapRecv(comm->bootstrap, masterPeer, collNetGraph->id, &sendrecvExchange, sizeof(sendrecvExchange)));
    rankInCollNet = sendrecvExchange.collNetRank;
    TRACE(NCCL_INIT, "CollNet [send] : rank %d collNetRank %d collNetNranks %d received connect from rank %d", rank, rankInCollNet, nMasters, masterPeer);
  }

  // select
/* 中文逐项-变量 V4709：root
 * root（struct ncclPeer；函数 ncclTransportCollNetSetup 内）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 * 初值/来源 root = channel->peers+nranks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclPeer* root = channel->peers+nranks;
  // connector index: 0 for recv, 1 for send
/* 中文逐项-变量 V4710：conn
 * conn（struct ncclConnector；函数 ncclTransportCollNetSetup 内）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 初值/来源 conn = (type == collNetRecv) ? root->recv+type : root->send+type；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclConnector* conn = (type == collNetRecv) ? root->recv+type : root->send+type;
/* 中文逐项-变量 V4711：transportComm
 * transportComm（struct ncclTransportComm；函数 ncclTransportCollNetSetup 内）：所选 transport 的回调表，规定建链、释放与代理推进方式。
 * 初值/来源 transportComm = (type == collNetRecv) ? &(collNetTransport.recv) :
 * &(collNetTransport.send)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTransportComm* transportComm = (type == collNetRecv) ? &(collNetTransport.recv) : &(collNetTransport.send);
  conn->transportComm = transportComm;
  // setup
/* 中文逐项-变量 V4712：myConnect
 * myConnect（struct ncclConnect；函数 ncclTransportCollNetSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(transportComm->setup(comm, collNetGraph, myInfo, peerInfo, &myConnect, conn,
 * collNetGraphChannelId, type));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclConnect myConnect;
  if (isMaster && support) {
    NCCLCHECK(transportComm->setup(comm, collNetGraph, myInfo, peerInfo, &myConnect, conn, collNetGraphChannelId, type));
  }
  // prepare connect handles
/* 中文逐项-变量 V4713：res
 * res（ncclResult_t；函数 ncclTransportCollNetSetup 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(bootstrapAllGather(comm->bootstrap, allConnects, sizeof(*allConnects)), res,
 * cleanup);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t res;
/* 中文逐项-变量 V4714：allConnects
 * allConnects（struct { int isMaster; ncclConnect connect; }；函数 ncclTransportCollNetSetup
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 allConnects = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
/* 中文逐项-类型 T0460：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：isMaster、connect。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct {
/* 中文逐项-变量 V4715：isMaster
 * isMaster（int；成员 <匿名>）：当前参与者是否承担该分组/初始化任务的主协调者角色，控制只需执行一次的动作。
 */
    int isMaster;
/* 中文逐项-变量 V4716：connect
 * connect（ncclConnect；成员 <匿名>）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
    ncclConnect connect;
  } *allConnects = NULL;
/* 中文逐项-变量 V4717：masterConnects
 * masterConnects（ncclConnect；函数 ncclTransportCollNetSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 masterConnects = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  ncclConnect *masterConnects = NULL;
  NCCLCHECK(ncclCalloc(&masterConnects, nMasters));
  if (type == collNetRecv) {  // recv side: AllGather
    // all ranks must participate
    NCCLCHECK(ncclCalloc(&allConnects, nranks));
    allConnects[rank].isMaster = isMaster;
    memcpy(&(allConnects[rank].connect), &myConnect, sizeof(struct ncclConnect));
    NCCLCHECKGOTO(bootstrapAllGather(comm->bootstrap, allConnects, sizeof(*allConnects)), res, cleanup);
    // consolidate
/* 中文逐项-变量 V4718：c
 * c（int；函数 ncclTransportCollNetSetup 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int c = 0;
    for (/* 中文逐项-变量 V4719：r
 * r（int；函数 ncclTransportCollNetSetup 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r < nranks；循环体与增量表达式决定每轮如何推进。
 */int r = 0; r < nranks; r++) {
      if (allConnects[r].isMaster) {
        memcpy(masterConnects+c, &(allConnects[r].connect), sizeof(struct ncclConnect));
        if (r == rank) rankInCollNet = c;
        c++;
      }
    }
  } else { // send side : copy in connect info received from peer recv master
    if (isMaster) memcpy(masterConnects+rankInCollNet, &(sendrecvExchange.connect), sizeof(struct ncclConnect));
  }
  // connect
  if (isMaster && support) {
    NCCLCHECKGOTO(transportComm->connect(comm, masterConnects, nMasters, rankInCollNet, conn), res, cleanup);
/* 中文逐项-变量 V4720：devRoot
 * devRoot（struct ncclPeer；函数 ncclTransportCollNetSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 devRoot = channel->devPeers+nranks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclPeer* devRoot = channel->devPeers+nranks;
/* 中文逐项-变量 V4721：devConn
 * devConn（struct ncclConnector；函数 ncclTransportCollNetSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 devConn = (type == collNetRecv) ? devRoot->recv+type : devRoot->send+type；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclConnector* devConn = (type == collNetRecv) ? devRoot->recv+type : devRoot->send+type;
    CUDACHECKGOTO(cudaMemcpy(devConn, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice), res, cleanup);
  }
  // recv side sends connect info to send side
  if (isMaster && type == collNetRecv) {
    sendrecvExchange.collNetRank = rankInCollNet;
    memcpy(&sendrecvExchange.connect, masterConnects+rankInCollNet, sizeof(struct ncclConnect));
    NCCLCHECKGOTO(bootstrapSend(comm->bootstrap, masterPeer, collNetGraph->id, &sendrecvExchange, sizeof(sendrecvExchange)), res, cleanup);
    TRACE(NCCL_INIT, "CollNet [recv] : rank %d collNetRank %d collNetNranks %d sent connect to rank %d", rank, rankInCollNet, nMasters, masterPeer);
  }
  if (support) fail = 0;
cleanup:
  if (allConnects != NULL) free(allConnects);
  if (masterConnects != NULL) free(masterConnects);
  return fail;
}

/* 中文逐项-函数 F1177：ncclTransportCollNetCheck
 * 用途与位置：全组汇总 CollNet 建链失败状态，一处不支持时一致回退，避免各 rank 选择不一致。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 collNetSetupFail（int collNetSetupFail）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapIntraNodeAllGather。
 * 需要对照的边界/推进条件：i<comm->localRanks；allGatherFailures[i] != 0；collNetSetupFail；comm->localRank == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTransportCollNetCheck(struct ncclComm* comm, int collNetSetupFail) {
  // AllGather collNet setup results
/* 中文逐项-变量 V4722：allGatherFailures
 * allGatherFailures（int；函数 ncclTransportCollNetCheck 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 allGatherFailures = {0}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  int allGatherFailures[NCCL_MAX_LOCAL_RANKS] = {0};
  allGatherFailures[comm->localRank] = collNetSetupFail;
  NCCLCHECK(bootstrapIntraNodeAllGather(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, allGatherFailures, sizeof(int)));
  for (/* 中文逐项-变量 V4723：i
 * i（int；函数 ncclTransportCollNetCheck 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->localRanks; i++) {
    if (allGatherFailures[i] != 0) {
      collNetSetupFail = 1;
      break;
    }
  }
  if (collNetSetupFail) {
    if (comm->localRank == 0) WARN("Cannot initialize CollNet, using point-to-point network instead");
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F1178：ncclTransportCollNetFree
 * 用途与位置：关闭已创建的 CollNet 特殊连接，配合初始化失败回退或整体销毁。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；recv->transportComm->free；send->transportComm->free。
 * 需要对照的边界/推进条件：r<comm->nChannels；b<NCCL_MAX_CONNS；send->transportResources &&
 * send->transportComm；b<NCCL_MAX_CONNS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTransportCollNetFree(struct ncclComm* comm) {
  // Free collNet resources
  for (/* 中文逐项-变量 V4724：r
 * r（int；函数 ncclTransportCollNetFree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->nChannels; r++) {
/* 中文逐项-变量 V4725：channel
 * channel（struct ncclChannel；函数 ncclTransportCollNetFree 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+r；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+r;
/* 中文逐项-变量 V4726：peer
 * peer（struct ncclPeer；函数 ncclTransportCollNetFree 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = channel->peers+comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclPeer* peer = channel->peers+comm->nRanks;
    for (/* 中文逐项-变量 V4727：b
 * b（int；函数 ncclTransportCollNetFree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：b<NCCL_MAX_CONNS；循环体与增量表达式决定每轮如何推进。
 */int b=0; b<NCCL_MAX_CONNS; b++) {
/* 中文逐项-变量 V4728：send
 * send（struct ncclConnector；函数 ncclTransportCollNetFree 内）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 初值/来源 send = peer->send + b；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclConnector* send = peer->send + b;
      if (send->transportResources && send->transportComm) NCCLCHECK(send->transportComm->free(send));
      send->transportResources = NULL; // avoid double free
    }
    for (/* 中文逐项-变量 V4729：b
 * b（int；函数 ncclTransportCollNetFree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：b<NCCL_MAX_CONNS；循环体与增量表达式决定每轮如何推进。
 */int b=0; b<NCCL_MAX_CONNS; b++) {
/* 中文逐项-变量 V4730：recv
 * recv（struct ncclConnector；函数 ncclTransportCollNetFree 内）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 初值/来源 recv = peer->recv + b；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclConnector* recv = peer->recv + b;
      if (recv->transportResources && recv->transportComm) NCCLCHECK(recv->transportComm->free(recv));
      recv->transportResources = NULL; // avoid double free
    }
  }
  return ncclSuccess;
}
