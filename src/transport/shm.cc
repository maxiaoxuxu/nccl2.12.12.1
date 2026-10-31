/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "shm.h"

/* 中文逐项-类型 T0456：shmConnectInfo
 * 类型职责：共享内存建链名字/尺寸及本地/对端CPU/GPU映射资源。
 * 声明内可见成员（含嵌套成员）：shmName、shmSize。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct shmConnectInfo {
/* 中文逐项-变量 V4623：shmName
 * shmName（char；成员 shmConnectInfo）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 */
  char shmName[7];
/* 中文逐项-变量 V4624：shmSize
 * shmSize（int；成员 shmConnectInfo）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 */
  int shmSize;
};
static_assert(sizeof(shmConnectInfo) <= CONNECT_SIZE, "SHM Connect info is too large");

/* 中文逐项-类型 T0457：shmSendResources
 * 类型职责：共享内存建链名字/尺寸及本地/对端CPU/GPU映射资源。
 * 声明内可见成员（含嵌套成员）：remShmSize、remHostMem、devRemHostMem、shmSize、hostMem、devHostMem。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct shmSendResources {
/* 中文逐项-变量 V4625：remShmSize
 * remShmSize（int；成员 shmSendResources）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 */
  int remShmSize;
/* 中文逐项-变量 V4626：remHostMem
 * remHostMem（struct ncclRecvMem；成员 shmSendResources）：对端或本地共享主机内存的CPU/设备地址视图，同一物理页的虚拟地址可不同。
 */
  struct ncclRecvMem* remHostMem;
/* 中文逐项-变量 V4627：devRemHostMem
 * devRemHostMem（struct ncclRecvMem；成员 shmSendResources）：对端或本地共享主机内存的CPU/设备地址视图，同一物理页的虚拟地址可不同。
 */
  struct ncclRecvMem* devRemHostMem;
/* 中文逐项-变量 V4628：shmSize
 * shmSize（int；成员 shmSendResources）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 */
  int shmSize;
/* 中文逐项-变量 V4629：hostMem
 * hostMem（struct ncclSendMem；成员 shmSendResources）：当前方向拥有的主机或设备分配，地址空间由字段名及类型共同确认。
 */
  struct ncclSendMem* hostMem;
/* 中文逐项-变量 V4630：devHostMem
 * devHostMem（struct ncclSendMem；成员 shmSendResources）：对端或本地共享主机内存的CPU/设备地址视图，同一物理页的虚拟地址可不同。
 */
  struct ncclSendMem* devHostMem;
};

/* 中文逐项-类型 T0458：shmRecvResources
 * 类型职责：共享内存建链名字/尺寸及本地/对端CPU/GPU映射资源。
 * 声明内可见成员（含嵌套成员）：remShmSize、remHostMem、devRemHostMem、shmSize、hostMem、devHostMem。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct shmRecvResources {
/* 中文逐项-变量 V4631：remShmSize
 * remShmSize（int；成员 shmRecvResources）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 */
  int remShmSize;
/* 中文逐项-变量 V4632：remHostMem
 * remHostMem（struct ncclSendMem；成员 shmRecvResources）：对端或本地共享主机内存的CPU/设备地址视图，同一物理页的虚拟地址可不同。
 */
  struct ncclSendMem* remHostMem;
/* 中文逐项-变量 V4633：devRemHostMem
 * devRemHostMem（struct ncclSendMem；成员 shmRecvResources）：对端或本地共享主机内存的CPU/设备地址视图，同一物理页的虚拟地址可不同。
 */
  struct ncclSendMem* devRemHostMem;
/* 中文逐项-变量 V4634：shmSize
 * shmSize（int；成员 shmRecvResources）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 */
  int shmSize;
/* 中文逐项-变量 V4635：hostMem
 * hostMem（struct ncclRecvMem；成员 shmRecvResources）：当前方向拥有的主机或设备分配，地址空间由字段名及类型共同确认。
 */
  struct ncclRecvMem* hostMem;
/* 中文逐项-变量 V4636：devHostMem
 * devHostMem（struct ncclRecvMem；成员 shmRecvResources）：对端或本地共享主机内存的CPU/设备地址视图，同一物理页的虚拟地址可不同。
 */
  struct ncclRecvMem* devHostMem;
};

/* 中文逐项-参数getter P0056：ShmDisable
 * 生成函数 ncclParamShmDisable()，读取 NCCL_SHM_DISABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(ShmDisable, "SHM_DISABLE", 0);

/* Determine two peers can communicate with SHM */
/* 中文导读：
 * 在同主机且共享内存条件满足时允许 SHM transport，作为本地 GPU P2P 不可用时的候选路径。
 * SHM 使用主机共享区和 GPU 可访问映射，不会因此创建跨机 RDMA QP。
 */
/* 中文逐项-函数 F1165：shmCanConnect
 * 用途与位置：检查同机与同 /dev/shm 设备且 SHM 未禁用，决定共享主机缓冲是否可作回退 transport。
 * 参数 ret（int* ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 topo（struct ncclTopoSystem* topo）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 info1（struct ncclPeerInfo* info1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 info2（struct ncclPeerInfo* info2）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclParamShmDisable。
 * 需要对照的边界/推进条件：ncclParamShmDisable() == 1；info1->hostHash != info2->hostHash；info1->shmDev !=
 * info2->shmDev。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
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

/* 中文逐项-常量 X1296：MAX_SHM_NAME_LEN
 * 编译期定义：MAX_SHM_NAME_LEN = 1024。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_SHM_NAME_LEN 1024

/* Create and return connect structures for this peer to connect to me */
/* 中文逐项-函数 F1166：shmSendSetup
 * 用途与位置：分别创建发送控制区或接收控制加协议数据区，并把可导入共享文件信息放入建链句柄。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 myInfo（struct ncclPeerInfo* myInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peerInfo（struct ncclPeerInfo* peerInfo）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclCalloc；ncclShmOpen。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t shmSendSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclConnect* connectInfo, struct ncclConnector* send, int channelId, int connIndex) {
/* 中文逐项-变量 V4637：resources
 * resources（struct shmSendResources；函数 shmSendSetup 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&resources, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct shmSendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  send->transportResources = resources;

  static_assert(sizeof(struct shmConnectInfo) <= sizeof(struct ncclConnect), "shm Connect Info is too big");
/* 中文逐项-变量 V4638：info
 * info（struct shmConnectInfo；函数 shmSendSetup 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct shmConnectInfo*)connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

/* 中文逐项-变量 V4639：shmPath
 * shmPath（char；函数 shmSendSetup 内）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 此处未显式给初值；后续首个文本使用点：shmPath[0] = '\0';。若由输出参数填入，必须先检查填充调用成功再读值。
 */
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
/* 中文逐项-函数 F1167：shmRecvSetup
 * 用途与位置：分别创建发送控制区或接收控制加协议数据区，并把可导入共享文件信息放入建链句柄。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 myInfo（struct ncclPeerInfo* myInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peerInfo（struct ncclPeerInfo* peerInfo）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclCalloc；ncclShmOpen。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t shmRecvSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclConnect* connectInfo, struct ncclConnector* recv, int channelId, int connIndex) {
/* 中文逐项-变量 V4640：resources
 * resources（struct shmRecvResources；函数 shmRecvSetup 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&resources, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct shmRecvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  recv->transportResources = resources;

  static_assert(sizeof(struct shmConnectInfo) <= sizeof(struct ncclConnect), "shm Connect Info is too big");
/* 中文逐项-变量 V4641：info
 * info（struct shmConnectInfo；函数 shmRecvSetup 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct shmConnectInfo*)connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

/* 中文逐项-变量 V4642：shmPath
 * shmPath（char；函数 shmRecvSetup 内）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 此处未显式给初值；后续首个文本使用点：shmPath[0] = '\0';。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char shmPath[PATH_MAX];
  shmPath[0] = '\0';
/* 中文逐项-变量 V4643：shmSize
 * shmSize（int；函数 shmRecvSetup 内）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 * 初值/来源 shmSize = sizeof(struct ncclRecvMem)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int shmSize = sizeof(struct ncclRecvMem);
  for (/* 中文逐项-变量 V4644：p
 * p（int；函数 shmRecvSetup 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) shmSize += recv->comm->buffSizes[p];
  info->shmSize = resources->shmSize = shmSize;
  NCCLCHECK(ncclShmOpen(shmPath, resources->shmSize, (void**)&resources->hostMem, (void**)&resources->devHostMem, 1));
  TRACE(NCCL_SHM,"Opened shmName %s shmSize %d", shmPath, info->shmSize);
  memcpy(info->shmName, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof(info->shmName));

  return ncclSuccess;
}

/* Connect to this peer */
/* 中文逐项-函数 F1168：shmSendConnect
 * 用途与位置：打开对端共享文件，取得 CPU/GPU 映射地址并按各协议区偏移填写连接视图。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclShmOpen；ncclShmUnlink；sprintf。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t shmSendConnect(struct ncclComm* comm, struct ncclConnect* connectInfo, int nranks, int rank, struct ncclConnector* send) {
  // Setup device pointers
/* 中文逐项-变量 V4645：info
 * info（struct shmConnectInfo；函数 shmSendConnect 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct shmConnectInfo*)connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;
/* 中文逐项-变量 V4646：resources
 * resources（struct shmSendResources；函数 shmSendConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct shmSendResources*)send->transportResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct shmSendResources* resources = (struct shmSendResources*)send->transportResources;

/* 中文逐项-变量 V4647：shmPath
 * shmPath（char；函数 shmSendConnect 内）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 此处未显式给初值；后续首个文本使用点：sprintf(shmPath, "/dev/shm/nccl-%s", info->shmName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char shmPath[PATH_MAX];
  sprintf(shmPath, "/dev/shm/nccl-%s", info->shmName);
  resources->remShmSize = info->shmSize;
  TRACE(NCCL_SHM,"Open shmName %s shmSize %d", shmPath, info->shmSize);
  NCCLCHECK(ncclShmOpen(shmPath, resources->remShmSize, (void**)&resources->remHostMem, (void**)&resources->devRemHostMem, 0));
  // Remove the file to ensure proper clean-up
  NCCLCHECK(ncclShmUnlink(shmPath));

  send->transportResources = resources;
/* 中文逐项-变量 V4648：offset
 * offset（int；函数 shmSendConnect 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int offset = 0;
  for (/* 中文逐项-变量 V4649：p
 * p（int；函数 shmSendConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    send->conn.buffs[p] = (char*)(resources->devRemHostMem+1) + offset;
    offset += send->comm->buffSizes[p];
  }
  send->conn.tail = &resources->devRemHostMem->tail;

  send->conn.head = &resources->devHostMem->head;
  return ncclSuccess;
}

/* 中文逐项-函数 F1169：shmRecvConnect
 * 用途与位置：打开对端共享文件，取得 CPU/GPU 映射地址并按各协议区偏移填写连接视图。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclShmOpen；ncclShmUnlink；sprintf。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t shmRecvConnect(struct ncclComm* comm, struct ncclConnect* connectInfo, int nranks, int rank, struct ncclConnector* recv) {
  // Setup device pointers
/* 中文逐项-变量 V4650：resources
 * resources（struct shmRecvResources；函数 shmRecvConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct shmRecvResources*)recv->transportResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct shmRecvResources* resources = (struct shmRecvResources*)recv->transportResources;
/* 中文逐项-变量 V4651：info
 * info（struct shmConnectInfo；函数 shmRecvConnect 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct shmConnectInfo*)connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

/* 中文逐项-变量 V4652：shmPath
 * shmPath（char；函数 shmRecvConnect 内）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 此处未显式给初值；后续首个文本使用点：sprintf(shmPath, "/dev/shm/nccl-%s", info->shmName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char shmPath[PATH_MAX];
  sprintf(shmPath, "/dev/shm/nccl-%s", info->shmName);
  resources->remShmSize = info->shmSize;
  TRACE(NCCL_SHM,"Open shmName %s shmSize %d", shmPath, info->shmSize);
  NCCLCHECK(ncclShmOpen(shmPath, resources->remShmSize, (void**)&resources->remHostMem, (void**)&resources->devRemHostMem, 0));
  NCCLCHECK(ncclShmUnlink(shmPath));
  recv->conn.head = &resources->devRemHostMem->head;

/* 中文逐项-变量 V4653：offset
 * offset（int；函数 shmRecvConnect 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int offset = 0;
  for (/* 中文逐项-变量 V4654：p
 * p（int；函数 shmRecvConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    recv->conn.buffs[p] = (char*)(resources->devHostMem+1) + offset;
    offset += recv->comm->buffSizes[p];
  }
  recv->conn.tail = &resources->devHostMem->tail;
  return ncclSuccess;
}

/* 中文逐项-函数 F1170：shmSendFree
 * 用途与位置：解除对端与本地共享主机映射及 CUDA 注册，按拥有关系删除共享对象名字。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；free；ncclShmClose。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t shmSendFree(struct ncclConnector* send) {
/* 中文逐项-变量 V4655：resources
 * resources（struct shmRecvResources；函数 shmSendFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct shmRecvResources*)send->transportResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct shmRecvResources* resources = (struct shmRecvResources*)send->transportResources;
  NCCLCHECK(ncclShmClose(resources->hostMem, resources->devHostMem, resources->shmSize));
  NCCLCHECK(ncclShmClose(resources->remHostMem, resources->devRemHostMem, resources->remShmSize));
  free(resources);
  return ncclSuccess;
}

/* 中文逐项-函数 F1171：shmRecvFree
 * 用途与位置：解除对端与本地共享主机映射及 CUDA 注册，按拥有关系删除共享对象名字。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；free；ncclShmClose。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t shmRecvFree(struct ncclConnector* recv) {
/* 中文逐项-变量 V4656：resources
 * resources（struct shmRecvResources；函数 shmRecvFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct shmRecvResources*)recv->transportResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct shmRecvResources* resources = (struct shmRecvResources*)recv->transportResources;
  NCCLCHECK(ncclShmClose(resources->hostMem, resources->devHostMem, resources->shmSize));
  NCCLCHECK(ncclShmClose(resources->remHostMem, resources->devRemHostMem, resources->remShmSize));
  free(resources);
  return ncclSuccess;
}

/* 中文逐项-变量 V4657：shmTransport
 * shmTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shmTransport = { "SHM", shmCanConnect, { shmSendSetup, shmSendConnect, shmSendFree, NULL, NULL, NULL, NULL,
 * NULL }, { shmRecvSetup, shmRecvConnect, shmRecvFree, NULL, NULL, NULL, NULL, NULL }
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct ncclTransport shmTransport = {
  "SHM",
  shmCanConnect,
  { shmSendSetup, shmSendConnect, shmSendFree, NULL, NULL, NULL, NULL, NULL },
  { shmRecvSetup, shmRecvConnect, shmRecvFree, NULL, NULL, NULL, NULL, NULL }
};
