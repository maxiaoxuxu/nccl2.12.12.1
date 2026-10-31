/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "group.h"
#include "debug.h"
#include "enqueue.h"
#include "transport.h"
#include "channel.h"

#define MAX_ASYNC_OPS 128
thread_local pthread_t ncclGroupThreads[MAX_ASYNC_OPS];
thread_local int ncclGroupIndex = 0;
thread_local int ncclGroupMode = 0;
thread_local ncclResult_t ncclGroupError = ncclSuccess;

bool ncclAsyncMode() {
  return ncclGroupMode > 0;
}

ncclResult_t ncclAsyncErrCheck(ncclResult_t ret) {
  if (ncclGroupError == ncclSuccess || ret != ncclSuccess) ncclGroupError = ret;
  return ret;
}

struct ncclInitArgs {
  ncclInitFunc_t func;
  int cudaDev;
  ncclComm_t* newcomm;
  int ndev;
  ncclUniqueId commId;
  int myrank;
};
struct ncclCollArgs {
  ncclComm_t comm;
};

enum ncclAsyncFuncType {
  ASYNC_FUNC_INVALID = 0,
  ASYNC_FUNC_INIT = 1,
  ASYNC_FUNC_COLL = 2,
};
struct ncclAsyncArgs {
  ncclResult_t ret;
  enum ncclAsyncFuncType funcType;
  union {
    ncclCollArgs coll;
    ncclInitArgs init;
  };
};

thread_local struct ncclAsyncArgs ncclGroupArgs[MAX_ASYNC_OPS];

void* ncclAsyncThreadMain(void* args_) {
  struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;
  NCCLCHECKTHREAD(args->init.func(args->init.newcomm, args->init.ndev, args->init.commId, args->init.myrank, args->init.cudaDev));
  return args;
}

ncclResult_t ncclAsyncInit(ncclInitFunc_t func, ncclComm_t* newcomm, int ndev, ncclUniqueId commId, int myrank, int cudaDev) {
  if (ncclGroupIndex >= MAX_ASYNC_OPS) {
    WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);
    return ncclAsyncErrCheck(ncclInvalidUsage);
  }
  int index = ncclGroupIndex++;
  struct ncclAsyncArgs* args = ncclGroupArgs+index;
  args->funcType = ASYNC_FUNC_INIT;
  args->init.func = func;
  args->init.cudaDev = cudaDev;
  args->init.newcomm = newcomm;
  args->init.ndev = ndev;
  memcpy(&args->init.commId, &commId, sizeof(commId));
  args->init.myrank = myrank;
  return ncclSuccess;
}

ncclResult_t ncclAsyncColl(ncclComm_t comm) {
  struct ncclAsyncArgs* args = ncclGroupArgs;
  for (int i=0; i<ncclGroupIndex; i++) {
    if (args->coll.comm == comm) return ncclSuccess;
    args++;
  }
  if (ncclGroupIndex >= MAX_ASYNC_OPS) {
    WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);
    return ncclAsyncErrCheck(ncclInvalidUsage);
  }
  ncclGroupIndex++;
  args->funcType = ASYNC_FUNC_COLL;
  args->coll.comm = comm;
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclGroupStart);
/* 中文导读：
 * 增加线程局部的 group 嵌套深度；组内通信调用先记录，不立即按单次 API 路径完整启动。
 * 用于多 GPU 发射协调及批量 Send/Recv。嵌套 group 只有最外层 GroupEnd 才真正处理。
 */
ncclResult_t ncclGroupStart() {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (ncclGroupMode == 0) {
    memset(ncclGroupArgs, 0, sizeof(struct ncclAsyncArgs)*MAX_ASYNC_OPS);
  }
  ncclGroupMode++;
  return ncclSuccess;
}

static ncclResult_t scheduleSend(struct ncclComm* comm, int peer, int chunk, size_t count, void* buff) {
  struct ncclInfo info = { ncclFuncSend, "Send",
    NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, /* Args */
    1, 1 };
  int channelId;
  NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncSend, &channelId));
  info.channelId = channelId;
  NCCLCHECK(ncclSetupP2pKernel(&info));
  return ncclSuccess;
}
static ncclResult_t scheduleRecv(struct ncclComm* comm, int peer, int chunk, size_t count, void* buff) {
  struct ncclInfo info = { ncclFuncRecv, "Recv",
    NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, /* Args */
    1, 1 };
  int channelId;
  NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncRecv, &channelId));
  info.channelId = channelId;
  NCCLCHECK(ncclSetupP2pKernel(&info));
  return ncclSuccess;
}

void* ncclAsyncThreadPreconnect(void* args_) {
  struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;
  struct ncclComm* comm = args->coll.comm;
  CUDACHECKTHREAD(cudaSetDevice(comm->cudaDev));
  if (CPU_COUNT(&comm->cpuAffinity)) sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);
  NCCLCHECKTHREAD(ncclTransportP2pSetup(comm, NULL, 1));
  return args;
}

/* 中文精读：P2P 的第一层切块：把一条消息铺到若干 channel
 * 从 totalSize/minChannels 得到每块候选尺寸；若太大，就在 maxChannels 范围内倍增通道数。
 * 最后按 minSize 向上对齐；调用处 minSize 是 Simple 的一个 step，避免产生过碎的小块。
 * 大块降低调度开销，小块增加并行度；实际折中还考虑同一 work 内同时活跃的 peer 数。
 * 这层 chunk 是 Group 给 channel 的消息块，设备 runSend/runRecv 内还会继续分成协议块。
 */
static size_t getP2pChunkSize(size_t totalSize, int minChannels, int maxChannels, size_t minSize, size_t maxSize) {
  size_t size = std::max(minSize, DIVUP(totalSize, minChannels));
  int nChannels = minChannels;
  while (size > maxSize && nChannels <= maxChannels/2) {
    nChannels *= 2;
    size = DIVUP(totalSize, nChannels);
  }
  ALIGN_SIZE(size, minSize);
  return size;
}

NCCL_API(ncclResult_t, ncclGroupEnd);
/* 中文导读：
 * group 的实际提交点：执行初始化任务、补建 P2P 连接、按节点/peer 分块调度通信、准备并启动 kernels。
 * 将同进程多个 communicator 的准备与发射阶段分开，避免一部分 GPU 还未启动而其他路径先阻塞。
 * 调用完成仍不代表 CUDA stream 上的全部通信已经完成；应用需按 CUDA 流/事件规则等待。
 */
ncclResult_t ncclGroupEnd() {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (ncclGroupMode == 0) {
    WARN("ncclGroupEnd: not in a group call.");
    return ncclInvalidUsage;
  }
  ncclGroupMode--;
  if (ncclGroupMode > 0) return ncclSuccess;
  int savedDev;
  CUDACHECK(cudaGetDevice(&savedDev));
  int activeThreads = 0;
  int doneArray[MAX_ASYNC_OPS];
  for (int i=0; i<ncclGroupIndex; i++) doneArray[i] = 1;
  ncclResult_t ret = ncclGroupError;
  int usingCudaGraphAll = -1;
  cudaGraph_t* graphs = NULL;
  if (ret != ncclSuccess) goto group_cleanup;

  /* Launch async ncclCommInitRank */
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_INIT) {
      pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadMain, args);
      activeThreads++;
      doneArray[i] = 0;
    }
  }
  /* For init, since we use threads, we just wait for threads to complete */
  while (activeThreads) {
    for (int i=0; i<ncclGroupIndex; i++) {
      struct ncclAsyncArgs* args = ncclGroupArgs+i;
      if (args->funcType == ASYNC_FUNC_INIT && doneArray[i] == 0) {
        int err = pthread_tryjoin_np(ncclGroupThreads[i], NULL);
        if (err == EBUSY) continue;
        if (err != 0) ret = ncclSystemError;
        if (args->ret != ncclSuccess) ret = args->ret;
        doneArray[i] = 1;
        activeThreads--;
      }
    }
  }

  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL && args->coll.comm->connect) {
      pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadPreconnect, args);
    }
  }

  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL && args->coll.comm->connect) {
      int err = pthread_join(ncclGroupThreads[i], NULL);
      if (err != 0) {
        WARN("Error waiting for pthread_join : %s", strerror(errno));
        return ncclSystemError;
      }
      NCCLCHECKGOTO(args->ret, ret, end);
      args->coll.comm->connect = 0;
    }
  }

  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      struct ncclComm* comm = args->coll.comm;
      int node = comm->node;
      int nNodes = comm->nNodes;
      int localRank = comm->localRank;

/* 中文精读：P2P 并行度为什么不是永远开到最大
 * 每 peer 的 channel 预算是上限；很多 peer 同时收发会争用全局 p2pnChannels。
 * 这里按 rank 数缩小 nChannelsMin/Max，减少一个 block 被塞入过多收发项而损失同步 warp 的情况。
 * 因此初始化日志中的 per peer 数不等于每条消息实际占用数；消息尺寸与同批工作量都会影响拆分。
 */
      // Compute how much to split operations
      // Natural step size matching buffer steps.
      ssize_t stepSize = comm->buffSizes[NCCL_PROTO_SIMPLE] / NCCL_STEPS;
      // Try to use all channels
      int nChannelsMax = comm->p2pnChannelsPerPeer;
      int nChannelsMin = nChannelsMax;
      // Try to use all channels, but one channel per operation.
      while (nChannelsMin*comm->nRanks > comm->p2pnChannels && nChannelsMin > 1) nChannelsMin /= 2;
      // Avoid overloading channels with 8+ operations as we loose the sync warp, hence a bit of bandwidth.
      while (nChannelsMax*comm->nRanks > comm->p2pnChannels*4 && nChannelsMax > 1) nChannelsMax /= 2;

      while (comm->p2pSendCount > 0 || comm->p2pRecvCount > 0) {
/* 中文精读：为什么收发必须采用双方一致的顺序
 * 用节点相对距离 delta 和节点内步号 s 选 sendPeer/recvPeer，再按统一规则跳过重复距离。
 * 这是显式排列 peer 对，而不是让所有 rank 按各自任意顺序串行等待；匹配收发才能持续前进。
 * 接着将每 peer 消息按 chunk 分发，Send 的偏移递增，Recv 的偏移独立递增。
 * 0 字节是有效同步任务，-1 表示这一方向没有任务，二者不能合并成同一种空操作。
 */
        // schedule delta 0, +1, -1, +2, -2, ...
        // also make sure we don't do 0 twice, nor +n/2 and -n/2 if n is even.
        for (int d=0; d<=nNodes/4; d++) {
          int deltas[4] = { d, (nNodes-d)%nNodes, nNodes/2-d, (nNodes-(nNodes/2-d))%nNodes };
          int index = 0;
          int delta = deltas[index];
sched_delta:
          uint32_t recvNode = (node+nNodes-delta)%nNodes;
          uint32_t sendNode = (node+delta)%nNodes;
          int steps = comm->maxLocalRanks;
          for (int s=0; s<steps; s++) {
            int recvIndex = (localRank-s+steps)%steps;
            int recvPeer = recvIndex<comm->nodeRanks[recvNode].localRanks ? comm->nodeRanks[recvNode].localRankToRank[recvIndex] : -1;
            int sendIndex = (localRank+s)%steps;
            int sendPeer = sendIndex<comm->nodeRanks[sendNode].localRanks ? comm->nodeRanks[sendNode].localRankToRank[sendIndex] : -1;
            struct ncclP2Pinfo* recv = recvPeer != -1 && comm->p2pRecvs[recvPeer] ? comm->p2pRecvs[recvPeer]->getNext() : NULL;
            struct ncclP2Pinfo* send = sendPeer != -1 && comm->p2pSends[sendPeer] ? comm->p2pSends[sendPeer]->getNext() : NULL;
            if (recv != NULL || send != NULL) {
              ssize_t totRecvBytes = -1, totSendBytes = -1;
              if (recv != NULL) totRecvBytes = recv->nbytes;
              if (send != NULL) totSendBytes = send->nbytes;
              if (recv) comm->p2pRecvCount--;
              if (send) comm->p2pSendCount--;
              if (recvPeer == comm->rank) { // Check self send/recv
                if (sendPeer != comm->rank) { WARN("Sendrecv schedule not aligned for self"); ret = ncclInternalError; goto group_cleanup; }
                if (send && recv == NULL) { WARN("Trying to send to self without a matching recv"); ret = ncclInvalidUsage; goto group_cleanup; }
                if (send == NULL && recv) { WARN("Trying to recv to self without a matching send"); ret = ncclInvalidUsage; goto group_cleanup; }
              }
              void* recvBuff = recv ? recv->buff : NULL;
              void* sendBuff = send ? send->buff : NULL;
              // After we recycle p2pSend/Recv, we're no longer allowed to dereference send or recv, only use them as boolean NULL/not NULL.
              if (recv && comm->p2pRecvs[recvPeer]->peakNext() == NULL) comm->p2pRecvs[recvPeer]->recycle();
              if (send && comm->p2pSends[sendPeer]->peakNext() == NULL) comm->p2pSends[sendPeer]->recycle();

              ssize_t recvChunkSize = getP2pChunkSize(totRecvBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);
              ssize_t sendChunkSize = getP2pChunkSize(totSendBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);

              ssize_t sendOffset = 0;
              ssize_t recvOffset = 0;
              int sendRemaining = 1, recvRemaining = 1;
              int chunk = 0;
              do {
                // Shuffle channels with s intra-node, and delta inter-node. Inter-node, make sure
                // to use multiple channels to guarantee progress on all ranks from the same node.
                ssize_t recvbytes = totRecvBytes-recvOffset;
                ssize_t sendbytes = totSendBytes-sendOffset;
                if (recvbytes > recvChunkSize) { recvbytes = recvChunkSize; } else { recvRemaining = 0; }
                if (sendbytes > sendChunkSize) { sendbytes = sendChunkSize; } else { sendRemaining = 0; }
                // 0-bytes send/recv are considered as syncs. Make sure we only add syncs when requested
                // (total size == 0), otherwise set size to -1.
                if (sendbytes < 0 || (sendbytes == 0 && totSendBytes != 0)) send = NULL;
                if (recvbytes < 0 || (recvbytes == 0 && totRecvBytes != 0)) recv = NULL;
                if (recv) {
                  NCCLCHECKGOTO(scheduleRecv(comm, recvPeer, chunk, recvbytes, ((char*)recvBuff)+recvOffset), ret, group_cleanup);
                }
                if (send) {
                  NCCLCHECKGOTO(scheduleSend(comm, sendPeer, chunk, sendbytes, ((char*)sendBuff)+sendOffset), ret, group_cleanup);
                }
                recvOffset += recvChunkSize;
                sendOffset += sendChunkSize;
                chunk++;
              } while (sendRemaining || recvRemaining);
            }
          }
          index++;
          if (index == 1 && deltas[1] == deltas[0]) index++;
          if (index == 2 && deltas[2] == deltas[0]) index++;
          if (index == 3 && deltas[3] == deltas[2]) index++;
          if (index == 3 && deltas[3] == deltas[1]) index++;
          if (index < 4) {
            delta = deltas[index];
            goto sched_delta;
          }
        }
      }
    }
  }

  /* Collectives are done in three steps :
   * 0. Save kernels previously enqueued. Compute channel, algo, proto, etc.
   * 1. Barrier Check In. Only the last call may call cudaLaunchKernel[cooperative]
   * 2. Barrier Wait. No CUDA call is permitted
   * 3. Enqueue Events. CUDA event wait/enqueue.
   * This is needed because step 2 cannot call any CUDA primitive, otherwise if
   * cudaFree happens between 1 and 3, it could block that CUDA call and
   * prevent some ranks from launching their network threads, which would
   * prevent the NCCL call from completing, blocking the cudaFree call.
   */

/* 中文精读：Group 最后阶段的调用顺序
 * 先统一检查各 communicator 是否都处于图捕获模式，再 SetupAsyncKernels 生成各自计划。
 * 普通模式对每个 comm 执行 HostSetup（其中发布 Proxy 任务）和 LaunchBarrier，
 * 然后再统一调用 LaunchKernel，最后 RecordEvents/LaunchReset。
 * 把准备、屏障、发射、事件拆为多轮，是为了同进程多 GPU 能互相等到匹配的工作。
 */
  // Check whether we are in cuda graph mode
  NCCLCHECK(ncclCalloc(&graphs, ncclGroupIndex));
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      ncclComm_t comm = args->coll.comm;
      NCCLCHECKGOTO(ncclGetCudaGraph(comm, graphs+i), ret, group_cleanup);
      if (usingCudaGraphAll == -1) {
        usingCudaGraphAll = comm->usingCudaGraph;
      } else if (usingCudaGraphAll != comm->usingCudaGraph) {
        WARN("Illegal to have some communicators in graph mode while others not");
        ret = ncclInvalidUsage;
        goto group_cleanup;
      }
    }
  }
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      ncclComm_t comm = args->coll.comm;
      NCCLCHECKGOTO(ncclSetupAsyncKernels(comm), ret, group_cleanup);
    }
  }
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      if (args->coll.comm->userStream == cudaStreamDefault ||
          args->coll.comm->userStream == cudaStreamPerThread ||
          args->coll.comm->userStream == cudaStreamLegacy)
        CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      if (usingCudaGraphAll == 1) {
        NCCLCHECKGOTO(ncclCudaGraphHostSetup(args->coll.comm, graphs[i]), ret, end);
      } else {
        ncclEnqueueHostSetup<0>(args->coll.comm->enqueueInfo);
      }
      NCCLCHECKGOTO(ncclLaunchBarrier(args->coll.comm), ret, end);
    }
  }
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      NCCLCHECKGOTO(ncclLaunchKernel(args->coll.comm), ret, end);
    }
  }
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      if (args->coll.comm->userStream == cudaStreamDefault ||
          args->coll.comm->userStream == cudaStreamPerThread ||
          args->coll.comm->userStream == cudaStreamLegacy)
        CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      NCCLCHECKGOTO(ncclRecordEvents(args->coll.comm), ret, end);
      NCCLCHECKGOTO(ncclLaunchReset(args->coll.comm), ret, end);
    }
  }

  goto end;
group_cleanup:
  if (ret != ncclSuccess) {
    // At least one call in the group failed. Since we want to make that group
    // an atomic operation, we need to cancel all operations.
    for (int i=0; i<ncclGroupIndex; i++) {
      struct ncclAsyncArgs* args = ncclGroupArgs+i;
      if (args->funcType == ASYNC_FUNC_INIT) {
        if (args->init.newcomm) ncclCommDestroy(*args->init.newcomm);
        *args->init.newcomm = NULL;
      } else {
        struct ncclComm* comm = args->coll.comm;
        // Reset aggregation counters
        comm->asyncOpCount = 0;
        comm->asyncTotalSize = 0;
        // Dequeue p2p lists
        if (comm->p2pSendCount > 0 || comm->p2pRecvCount > 0) {
          for (int peer=0; peer<comm->nRanks; peer++) {
            if (comm->p2pSends[peer]) comm->p2pSends[peer]->recycle();
            if (comm->p2pRecvs[peer]) comm->p2pRecvs[peer]->recycle();
          }
          comm->p2pSendCount = comm->p2pRecvCount = 0;
        }
        ncclLaunchReset(comm);
      }
    }
  }
end:
  ncclGroupError = ncclSuccess;
  ncclGroupIndex = 0;
  CUDACHECK(cudaSetDevice(savedDev)); // do other clean-ups first before calling cudaSetDevice, because this call can fail too
  if (graphs) free(graphs);
  return ret;
}
