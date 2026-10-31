/*************************************************************************
 * Copyright (c) 2015-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_CHANNEL_H_
#define NCCL_CHANNEL_H_
#include "comm.h"

ncclResult_t initChannel(struct ncclComm* comm, int channelid);
ncclResult_t freeChannel(struct ncclChannel* channel, int nRanks);
/* 中文导读：
 * 根据节点相对距离和节点内 rank 顺序计算 peer 的基准 channel。
 * Send 与 Recv 的相反方向公式保证两端为匹配的通信选出一致通道，而非按 peer 独占一个 channel。
 */
static ncclResult_t ncclChannelComputeBase(struct ncclComm* comm, int peer, int coll, int*channelBase) {
  int p2pGroupSize = NCCL_MAX_WORK_ELEMENTS_P2P/2;
  int peerNode = comm->rankToNode[peer];
  int peerIndex = comm->rankToLocalRank[peer];
  int nsteps = comm->maxLocalRanks;
  int rankIndex = comm->rankToLocalRank[comm->rank];
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
static ncclResult_t ncclChannelComputeFromBase(struct ncclComm* comm, int base, int channelInc, int*channelId) {
  *channelId = (base+comm->p2pChannels[channelInc]) % comm->p2pnChannels;
  return ncclSuccess;
}

static ncclResult_t ncclChannelCompute(struct ncclComm* comm, int peer, int channelInc, int coll, int*channelId) {
  int base;
  NCCLCHECK(ncclChannelComputeBase(comm, peer, coll, &base));
  NCCLCHECK(ncclChannelComputeFromBase(comm, base, channelInc, channelId));
  return ncclSuccess;
}

#endif
