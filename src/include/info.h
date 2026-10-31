/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_INFO_H_
#define NCCL_INFO_H_

#include "nccl.h"
#include "devcomm.h"
#include "collectives.h"

typedef enum : uint8_t {
  ncclPatternRing,
  ncclPatternRingTwice,
  ncclPatternPipelineFrom,
  ncclPatternPipelineTo,
  ncclPatternTreeUp,
  ncclPatternTreeDown,
  ncclPatternTreeUpDown,
  ncclPatternCollTreeUpDown,
  ncclPatternSend,
  ncclPatternRecv
} ncclPattern_t;

// Used to pass NCCL call information between functions
/* 中文导读：
 * 一次公开 API 调用在主机端的中间表示。前半部保存调用参数，后半部由调度器补齐。
 * 随后转换成 GPU 的 ncclWorkElem 与 CPU 网络推进的 ncclProxyOp，实现同一操作的双侧协同。
 */
struct ncclInfo {
  ncclFunc_t coll;
  const char* opName;
  // NCCL Coll Args
  const void* sendbuff;
  void* recvbuff;
  size_t count;
  ncclDataType_t datatype;
  ncclRedOp_t op;
  int root; // peer for p2p operations
  ncclComm_t comm;
  cudaStream_t stream;
  // Algorithm details
  int chunkSteps;
  int sliceSteps;
/* 中文精读：输入、推导值与单位
 * count/datatype 可能被 ArgsCheck 归一化；nBytes 是模型用的整体字节规模。
 * algorithm/protocol/pattern 是选定的执行策略，nChannels/nThreads 是本次操作的并行度。
 * nstepsPerLoop/nchunksPerLoop 描述算法一轮，chunkSteps/sliceSteps 描述协议缓冲推进粒度。
 * channelId 主要由 P2P 调度指定；collective 则为每个 bid 在实际入 FIFO 时选择 channelId。
 */
  // Computed later
  ncclDevRedOpFull opFull;
  int algorithm;
  int protocol;
  ncclPattern_t pattern;
  int nChannels;
  int nThreads;
  size_t nBytes;
  int nstepsPerLoop;
  int nchunksPerLoop;
  int chunkSize;
  int channelId;
};

#endif
