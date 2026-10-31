/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "shm.h"

struct shmConnectInfo {
  char shmName[7];
  int shmSize;
};
static_assert(sizeof(shmConnectInfo) <= CONNECT_SIZE, "SHM Connect info is too large");

struct shmSendResources {
  int remShmSize;
  struct ncclRecvMem* remHostMem;
  struct ncclRecvMem* devRemHostMem;
  int shmSize;
  struct ncclSendMem* hostMem;
  struct ncclSendMem* devHostMem;
};

struct shmRecvResources {
  int remShmSize;
  struct ncclSendMem* remHostMem;
  struct ncclSendMem* devRemHostMem;
  int shmSize;
  struct ncclRecvMem* hostMem;
  struct ncclRecvMem* devHostMem;
};

NCCL_PARAM(ShmDisable, "SHM_DISABLE", 0);

/* Determine two peers can communicate with SHM */
/* 中文导读：
 * 在同主机且共享内存条件满足时允许 SHM transport，作为本地 GPU P2P 不可用时的候选路径。
 * SHM 使用主机共享区和 GPU 可访问映射，不会因此创建跨机 RDMA QP。
 */
ncclResult_t shmCanConnect(int* ret, struct ncclTopoSystem* topo, struct ncclTopoGraph* graph, struct ncclPeerInfo* info1, struct ncclPeerInfo* info2) {
  *ret = 0;

  if (ncclParamShmDisable() == 1) return ncclSuccess;

  // Same host?
  TRACE(NCCL_INIT|NCCL_SHM, "peer1 hostHash %lx peer2 hostHash %lx", info1->hostHash, info2->hostHash);
  if (info1->hostHash != info2->hostHash) return ncclSuccess;

  // Common /dev/shm (between containers) ?
  TRACE(NCCL_INIT|NCCL_SHM, "peer1 shmDev %lx peer2 shmDev %lx", info1->shmDev, info2->shmDev);
  if (info1->shmDev != info2->shmDev) return ncclSuccess;

  *ret = 1;

  return ncclSuccess;
}

#define MAX_SHM_NAME_LEN 1024

/* Create and return connect structures for this peer to connect to me */
ncclResult_t shmSendSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclConnect* connectInfo, struct ncclConnector* send, int channelId, int connIndex) {
  struct shmSendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  send->transportResources = resources;

  static_assert(sizeof(struct shmConnectInfo) <= sizeof(struct ncclConnect), "shm Connect Info is too big");
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

  char shmPath[PATH_MAX];
  shmPath[0] = '\0';
  info->shmSize = resources->shmSize = sizeof(struct ncclSendMem);
  NCCLCHECK(ncclShmOpen(shmPath, resources->shmSize, (void**)&resources->hostMem, (void**)&resources->devHostMem, 1));
  TRACE(NCCL_SHM,"Opened shmName %s shmSize %d", shmPath, info->shmSize);
  memcpy(info->shmName, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof(info->shmName));

  INFO(NCCL_INIT|NCCL_SHM,"Channel %02d : %d[%lx] -> %d[%lx] via direct shared memory", channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId);
  return ncclSuccess;
}

/* 中文精读：SHM 接收缓冲为什么包含三个协议区
 * 接收者分配控制区 ncclRecvMem，再为每种协议追加 buffSizes[p] 字节的数据区。
 * 发送者通过共享文件名导入同一主机页，并取得 GPU 可访问映射后把 conn.buffs 指向对应区间。
 * GPU 原语可以按同一接口读写这块主机共享内存，代价和本地显存 P2P 不同。
 * 文件名/句柄通过 bootstrap 交换；用户张量本身不通过 bootstrap socket 传输。
 */
ncclResult_t shmRecvSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclConnect* connectInfo, struct ncclConnector* recv, int channelId, int connIndex) {
  struct shmRecvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  recv->transportResources = resources;

  static_assert(sizeof(struct shmConnectInfo) <= sizeof(struct ncclConnect), "shm Connect Info is too big");
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

  char shmPath[PATH_MAX];
  shmPath[0] = '\0';
  int shmSize = sizeof(struct ncclRecvMem);
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) shmSize += recv->comm->buffSizes[p];
  info->shmSize = resources->shmSize = shmSize;
  NCCLCHECK(ncclShmOpen(shmPath, resources->shmSize, (void**)&resources->hostMem, (void**)&resources->devHostMem, 1));
  TRACE(NCCL_SHM,"Opened shmName %s shmSize %d", shmPath, info->shmSize);
  memcpy(info->shmName, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof(info->shmName));

  return ncclSuccess;
}

/* Connect to this peer */
ncclResult_t shmSendConnect(struct ncclComm* comm, struct ncclConnect* connectInfo, int nranks, int rank, struct ncclConnector* send) {
  // Setup device pointers
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;
  struct shmSendResources* resources = (struct shmSendResources*)send->transportResources;

  char shmPath[PATH_MAX];
  sprintf(shmPath, "/dev/shm/nccl-%s", info->shmName);
  resources->remShmSize = info->shmSize;
  TRACE(NCCL_SHM,"Open shmName %s shmSize %d", shmPath, info->shmSize);
  NCCLCHECK(ncclShmOpen(shmPath, resources->remShmSize, (void**)&resources->remHostMem, (void**)&resources->devRemHostMem, 0));
  // Remove the file to ensure proper clean-up
  NCCLCHECK(ncclShmUnlink(shmPath));

  send->transportResources = resources;
  int offset = 0;
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    send->conn.buffs[p] = (char*)(resources->devRemHostMem+1) + offset;
    offset += send->comm->buffSizes[p];
  }
  send->conn.tail = &resources->devRemHostMem->tail;

  send->conn.head = &resources->devHostMem->head;
  return ncclSuccess;
}

ncclResult_t shmRecvConnect(struct ncclComm* comm, struct ncclConnect* connectInfo, int nranks, int rank, struct ncclConnector* recv) {
  // Setup device pointers
  struct shmRecvResources* resources = (struct shmRecvResources*)recv->transportResources;
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

  char shmPath[PATH_MAX];
  sprintf(shmPath, "/dev/shm/nccl-%s", info->shmName);
  resources->remShmSize = info->shmSize;
  TRACE(NCCL_SHM,"Open shmName %s shmSize %d", shmPath, info->shmSize);
  NCCLCHECK(ncclShmOpen(shmPath, resources->remShmSize, (void**)&resources->remHostMem, (void**)&resources->devRemHostMem, 0));
  NCCLCHECK(ncclShmUnlink(shmPath));
  recv->conn.head = &resources->devRemHostMem->head;

  int offset = 0;
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    recv->conn.buffs[p] = (char*)(resources->devHostMem+1) + offset;
    offset += recv->comm->buffSizes[p];
  }
  recv->conn.tail = &resources->devHostMem->tail;
  return ncclSuccess;
}

ncclResult_t shmSendFree(struct ncclConnector* send) {
  struct shmRecvResources* resources = (struct shmRecvResources*)send->transportResources;
  NCCLCHECK(ncclShmClose(resources->hostMem, resources->devHostMem, resources->shmSize));
  NCCLCHECK(ncclShmClose(resources->remHostMem, resources->devRemHostMem, resources->remShmSize));
  free(resources);
  return ncclSuccess;
}

ncclResult_t shmRecvFree(struct ncclConnector* recv) {
  struct shmRecvResources* resources = (struct shmRecvResources*)recv->transportResources;
  NCCLCHECK(ncclShmClose(resources->hostMem, resources->devHostMem, resources->shmSize));
  NCCLCHECK(ncclShmClose(resources->remHostMem, resources->devRemHostMem, resources->remShmSize));
  free(resources);
  return ncclSuccess;
}

struct ncclTransport shmTransport = {
  "SHM",
  shmCanConnect,
  { shmSendSetup, shmSendConnect, shmSendFree, NULL, NULL, NULL, NULL, NULL },
  { shmRecvSetup, shmRecvConnect, shmRecvFree, NULL, NULL, NULL, NULL, NULL }
};
