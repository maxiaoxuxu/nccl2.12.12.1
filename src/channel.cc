/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "channel.h"
#include "param.h"
#include "gdrwrap.h"

// GDRCOPY support: FIFO_ENABLE when enabled locates a workFifo in CUDA memory
/* 中文逐项-参数getter P0001：GdrCopyFifoEnable
 * 生成函数 ncclParamGdrCopyFifoEnable()，读取 NCCL_GDRCOPY_FIFO_ENABLE，编译默认值为 1。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(GdrCopyFifoEnable, "GDRCOPY_FIFO_ENABLE", 1);

/* 中文导读：
 * 为一个 channel 分配 peer 表、Ring rank 顺序及工作 FIFO 等对象。
 * peer 表预留了 rank 槽位；只有 transport setup/connect 成功的 connector 才有相应通信资源。
 */
/* 中文逐项-函数 F0048：initChannel
 * 用途与位置：为单条逻辑 channel 分配环顺序表、peer 连接表和 GPU 可读取的工作描述符 FIFO。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 channelid（int channelid）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputeP2pChannels；src/init.cc::setupChannel。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc；ncclCudaCalloc；ncclCudaHostCalloc；ncclGdrCudaCalloc；ncclParamGdrCopyFifoEnable。
 * 需要对照的边界/推进条件：channel->id != -1；i<comm->nRanks+1；b<NCCL_MAX_CONNS；ncclGdrCopy != NULL &&
 * ncclParamGdrCopyFifoEnable() == 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t initChannel(struct ncclComm* comm, int channelid) {
/* 中文逐项-变量 V0080：channel
 * channel（struct ncclChannel；函数 initChannel 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+channelid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclChannel* channel = comm->channels+channelid;
  if (channel->id != -1) return ncclSuccess;
  channel->id = channelid;

  // Ring index to user rank table.
  NCCLCHECK(ncclCudaCalloc(&channel->ring.devUserRanks, comm->nRanks));
  NCCLCHECK(ncclCalloc(&channel->ring.userRanks, comm->nRanks));

  // Communication structures with peers.
  NCCLCHECK(ncclCudaCalloc(&channel->devPeers, comm->nRanks+1)); // The extra one rank is for collnet root (i.e. network)
  NCCLCHECK(ncclCalloc(&channel->peers, comm->nRanks+1));
  for (/* 中文逐项-变量 V0081：i
 * i（size_t；函数 initChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->nRanks+1；循环体与增量表达式决定每轮如何推进。
 */size_t i=0; i<comm->nRanks+1; ++i) {
    for (/* 中文逐项-变量 V0082：b
 * b（int；函数 initChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：b<NCCL_MAX_CONNS；循环体与增量表达式决定每轮如何推进。
 */int b=0; b<NCCL_MAX_CONNS; b++) {
      channel->peers[i].send[b].comm = comm;
      channel->peers[i].recv[b].comm = comm;
    }
  }

  // Per-channel operation list.
  NCCLCHECK(ncclCudaHostCalloc(&channel->workFifo, NCCL_MAX_OPS));
  if (ncclGdrCopy != NULL && ncclParamGdrCopyFifoEnable() == 1) {
    // GDRCOPY support
    // We allocate a workFifo in GDR mapped CUDA memory
    // But we still allocate the Host workFifo so that we
    // can copy the work elements to CUDA memory on kernel launch
    NCCLCHECK(ncclGdrCudaCalloc(&channel->workFifoGdr, &channel->workFifoDev, NCCL_MAX_OPS, &channel->gdrMemDesc));
  } else {
    // The device workFifo is the Host one
    channel->workFifoDev = channel->workFifo;
  }

  return ncclSuccess;
}

/* 中文精读：沿初始化的反方向释放 channel
 * 回收 work FIFO 和可选 GDR 映射、Ring rank 表、各 peer 的 transportResources，最后释放 peer 表。
 * 先释放发送资源再释放接收资源，是为 CollNet 资源关系保留的顺序约束。
 * transport 的 free 可能还把工作交给 Proxy 服务，并按共享引用计数决定是否关闭实际连接。
 * 这里处理的是长期对象销毁，不是一次通信完成后立即把所有连接全部拆掉。
 */
/* 中文逐项-函数 F0049：freeChannel
 * 用途与位置：回收一条 channel 的工作 FIFO、环顺序表和各 transport 连接资源，先发后收满足共享依赖。
 * 参数 channel（struct ncclChannel* channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 nRanks（int nRanks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaFree；free；ncclCudaHostFree；ncclGdrCudaFree；peer->recv[b].transportComm->free；peer->send[b].transportComm->free。
 * 需要对照的边界/推进条件：channel->id == -1；channel->gdrMemDesc；r<nRanks+1；b<NCCL_MAX_CONNS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t freeChannel(struct ncclChannel* channel, int nRanks) {
  if (channel->id == -1) return ncclSuccess;
  // Operation list
  NCCLCHECK(ncclCudaHostFree(channel->workFifo));
  if (channel->gdrMemDesc) {
    // GDRCOPY support
    NCCLCHECK(ncclGdrCudaFree(channel->gdrMemDesc));
  }

  // Free Ring index to rank tables
  free(channel->ring.userRanks);
  CUDACHECK(cudaFree(channel->ring.devUserRanks));

  // Free transport proxy resources
  // Note: free all send resources first due to CollNet arrangement
  for (/* 中文逐项-变量 V0083：r
 * r（int；函数 freeChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nRanks+1；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nRanks+1; r++) {
/* 中文逐项-变量 V0084：peer
 * peer（struct ncclPeer；函数 freeChannel 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = channel->peers+r；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclPeer* peer = channel->peers+r;
    for (/* 中文逐项-变量 V0085：b
 * b（int；函数 freeChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：b<NCCL_MAX_CONNS；循环体与增量表达式决定每轮如何推进。
 */int b=0; b<NCCL_MAX_CONNS; b++) {
      if (peer->send[b].transportComm) NCCLCHECK(peer->send[b].transportComm->free(peer->send+b));
    }
  }
  for (/* 中文逐项-变量 V0086：r
 * r（int；函数 freeChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nRanks+1；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nRanks+1; r++) {
/* 中文逐项-变量 V0087：peer
 * peer（struct ncclPeer；函数 freeChannel 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = channel->peers+r；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclPeer* peer = channel->peers+r;
    for (/* 中文逐项-变量 V0088：b
 * b（int；函数 freeChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：b<NCCL_MAX_CONNS；循环体与增量表达式决定每轮如何推进。
 */int b=0; b<NCCL_MAX_CONNS; b++) {
      if (peer->recv[b].transportComm) NCCLCHECK(peer->recv[b].transportComm->free(peer->recv+b));
    }
  }

  // Free the peer structures.
  CUDACHECK(cudaFree(channel->devPeers));
  free(channel->peers);

  return ncclSuccess;
}
