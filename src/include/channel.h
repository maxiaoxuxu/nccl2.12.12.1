/*************************************************************************
 * Copyright (c) 2015-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_CHANNEL_H_
/* 中文逐项-常量 X0138：NCCL_CHANNEL_H_
 * 编译期定义：NCCL_CHANNEL_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_CHANNEL_H_
#include "comm.h"

/* 中文逐项-接口 X0139：initChannel
 * 接口职责：为单条逻辑 channel 分配环顺序表、peer 连接表和 GPU 可读取的工作描述符 FIFO。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t initChannel(struct ncclComm* comm, int channelid);
 */
ncclResult_t initChannel(struct ncclComm* comm, int channelid);
/* 中文逐项-接口 X0140：freeChannel
 * 接口职责：回收一条 channel 的工作 FIFO、环顺序表和各 transport 连接资源，先发后收满足共享依赖。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t freeChannel(struct ncclChannel* channel, int nRanks);
 */
ncclResult_t freeChannel(struct ncclChannel* channel, int nRanks);
/* 中文导读：
 * 根据节点相对距离和节点内 rank 顺序计算 peer 的基准 channel。
 * Send 与 Recv 的相反方向公式保证两端为匹配的通信选出一致通道，而非按 peer 独占一个 channel。
 */
/* 中文逐项-函数 F0610：ncclChannelComputeBase
 * 用途与位置：由节点距离与节点内 rank 差计算 P2P 基准通道，Send/Recv 用相反公式确保两端匹配。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 coll（int coll）：本任务的公开操作类别，如 AllReduce、Broadcast、Send 或 Recv。
 * 参数 channelBase（int*channelBase）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclSaveP2p；src/include/channel.h::ncclChannelCompute。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：coll == ncclFuncSend；coll == ncclFuncRecv。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclChannelComputeBase(struct ncclComm* comm, int peer, int coll, int*channelBase) {
/* 中文逐项-变量 V1978：p2pGroupSize
 * p2pGroupSize（int；函数 ncclChannelComputeBase 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 p2pGroupSize = NCCL_MAX_WORK_ELEMENTS_P2P/2；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int p2pGroupSize = NCCL_MAX_WORK_ELEMENTS_P2P/2;
/* 中文逐项-变量 V1979：peerNode
 * peerNode（int；函数 ncclChannelComputeBase 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 peerNode = comm->rankToNode[peer]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int peerNode = comm->rankToNode[peer];
/* 中文逐项-变量 V1980：peerIndex
 * peerIndex（int；函数 ncclChannelComputeBase 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 peerIndex = comm->rankToLocalRank[peer]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int peerIndex = comm->rankToLocalRank[peer];
/* 中文逐项-变量 V1981：nsteps
 * nsteps（int；函数 ncclChannelComputeBase 内）：本子任务需要推进的协议总 step 数，不直接等于包数、WQE 数或 QP 数。
 * 初值/来源 nsteps = comm->maxLocalRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nsteps = comm->maxLocalRanks;
/* 中文逐项-变量 V1982：rankIndex
 * rankIndex（int；函数 ncclChannelComputeBase 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rankIndex = comm->rankToLocalRank[comm->rank]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rankIndex = comm->rankToLocalRank[comm->rank];
/* 中文逐项-变量 V1983：step、delta
 * step（int；函数 ncclChannelComputeBase 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 此处未显式给初值；后续首个文本使用点：step = (nsteps + peerIndex - rankIndex)%nsteps;。若由输出参数填入，必须先检查填充调用成功再读值。
 * delta（int；函数 ncclChannelComputeBase 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：delta = (comm->nNodes + peerNode - comm->node) % comm->nNodes;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int step, delta;
  if (coll == ncclFuncSend) {
    step = (nsteps + peerIndex - rankIndex)%nsteps;
    delta = (comm->nNodes + peerNode - comm->node) % comm->nNodes;
  } else if (coll == ncclFuncRecv) {
    step = (nsteps + rankIndex - peerIndex)%nsteps;
    delta = (comm->nNodes + comm->node - peerNode) % comm->nNodes;
  } else {
    return ncclInternalError;
  }
  *channelBase = comm->nNodes > 1 ? delta+(step/p2pGroupSize) : step;
  return ncclSuccess;
}

/* 中文导读：
 * 将基准值加上预先生成的 channel 偏移，再对 P2P 总 channel 数取模。
 * 不同 peer 可以映射到相同 channel；其连接仍通过 peers[peer] 分开。
 */
/* 中文逐项-函数 F0611：ncclChannelComputeFromBase
 * 用途与位置：将基准通道加位反转分散偏移，对全局 P2P 通道空间取模，多个 peer 可映射到同通道。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 base（int base）：本操作在长期连接 step 上的对齐起点，本任务 posted/done 等相对计数要加 base 才是绝对进度。
 * 参数 channelInc（int channelInc）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 channelId（int*channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclSaveP2p；src/include/channel.h::ncclChannelCompute。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclChannelComputeFromBase(struct ncclComm* comm, int base, int channelInc, int*channelId) {
  *channelId = (base+comm->p2pChannels[channelInc]) % comm->p2pnChannels;
  return ncclSuccess;
}

/* 中文逐项-函数 F0612：ncclChannelCompute
 * 用途与位置：组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 channelInc（int channelInc）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 coll（int coll）：本任务的公开操作类别，如 AllReduce、Broadcast、Send 或 Recv。
 * 参数 channelId（int*channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/group.cc::scheduleSend；src/group.cc::scheduleRecv；src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclChannelComputeBase；ncclChannelComputeFromBase。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclChannelCompute(struct ncclComm* comm, int peer, int channelInc, int coll, int*channelId) {
/* 中文逐项-变量 V1984：base
 * base（int；函数 ncclChannelCompute 内）：本操作在长期连接 step 上的对齐起点，本任务 posted/done 等相对计数要加 base 才是绝对进度。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclChannelComputeBase(comm, peer, coll, &base));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int base;
  NCCLCHECK(ncclChannelComputeBase(comm, peer, coll, &base));
  NCCLCHECK(ncclChannelComputeFromBase(comm, base, channelInc, channelId));
  return ncclSuccess;
}

#endif
