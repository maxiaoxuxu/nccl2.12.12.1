/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "net.h"
#include "graph.h"
#include "proxy.h"
#include "collectives.h"
#include "gdrwrap.h"
#include "shm.h"
#include "profiler.h"

static_assert(sizeof(ncclNetHandle_t) <= CONNECT_SIZE, "NET Connect info is too large");

/* 中文逐项-常量 X1256：NCCL_NET_MAP_HOSTMEM
 * 编译期定义：NCCL_NET_MAP_HOSTMEM = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_HOSTMEM 0
/* 中文逐项-常量 X1257：NCCL_NET_MAP_DEVMEM
 * 编译期定义：NCCL_NET_MAP_DEVMEM = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_DEVMEM 1
/* 中文逐项-常量 X1258：NCCL_NET_MAP_SHARED_HOSTMEM
 * 编译期定义：NCCL_NET_MAP_SHARED_HOSTMEM = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_SHARED_HOSTMEM 2
/* 中文逐项-常量 X1259：NCCL_NET_MAP_SHARED_DEVMEM
 * 编译期定义：NCCL_NET_MAP_SHARED_DEVMEM = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_SHARED_DEVMEM 3
/* 中文逐项-常量 X1260：NCCL_NET_MAP_GDCMEM
 * 编译期定义：NCCL_NET_MAP_GDCMEM = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_GDCMEM 4
/* 中文逐项-常量 X1261：NCCL_NET_MAP_MEMS
 * 编译期定义：NCCL_NET_MAP_MEMS = 5。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MEMS 5

/* 中文逐项-常量 X1262：NCCL_NET_MAP_MASK_DEVMEM
 * 编译期定义：NCCL_NET_MAP_MASK_DEVMEM = 0x40000000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MASK_DEVMEM 0x40000000
/* 中文逐项-常量 X1263：NCCL_NET_MAP_MASK_SHARED
 * 编译期定义：NCCL_NET_MAP_MASK_SHARED = 0x80000000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MASK_SHARED 0x80000000
/* 中文逐项-常量 X1264：NCCL_NET_MAP_MASK_USED
 * 编译期定义：NCCL_NET_MAP_MASK_USED = 0x20000000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MASK_USED   0x20000000
/* 中文逐项-常量 X1265：NCCL_NET_MAP_MASK_OFFSET
 * 编译期定义：NCCL_NET_MAP_MASK_OFFSET = 0x1fffffff。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MASK_OFFSET 0x1fffffff

/* 中文逐项-宏 M0155：NCCL_NET_MAP_OFFSET_BANK
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_OFFSET_BANK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_OFFSET_BANK(mapStruct, offsetName) \
  ((mapStruct)->offsets.offsetName >> 30)

/* 中文逐项-宏 M0156：NCCL_NET_MAP_OFFSET_NULL
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_OFFSET_NULL；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_OFFSET_NULL(mapStruct, offsetName) \
  (((mapStruct)->offsets.offsetName >> 29) == 0)

/* 中文逐项-宏 M0157：NCCL_NET_MAP_GET_POINTER
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_GET_POINTER；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_GET_POINTER(mapStruct, cpuOrGpu, offsetName) \
  (NCCL_NET_MAP_OFFSET_NULL(mapStruct, offsetName) ? NULL : \
   (mapStruct)->mems[NCCL_NET_MAP_OFFSET_BANK(mapStruct, offsetName)].cpuOrGpu##Ptr + ((mapStruct)->offsets.offsetName & NCCL_NET_MAP_MASK_OFFSET))

/* 中文逐项-宏 M0158：NCCL_NET_MAP_DEV_MEM
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_DEV_MEM；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_DEV_MEM(mapStruct, offsetName) \
  (((mapStruct)->offsets.offsetName & NCCL_NET_MAP_MASK_DEVMEM) != 0)

/* 中文逐项-宏 M0159：NCCL_NET_MAP_ADD_POINTER
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_ADD_POINTER；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_ADD_POINTER(mapStruct, shared, dev, memSize, offsetName) do { \
    int bank = NCCL_NET_MAP_MASK_USED + (dev)*NCCL_NET_MAP_MASK_DEVMEM + (shared)*NCCL_NET_MAP_MASK_SHARED; \
    if ((shared) == 0) { \
      if (dev) { \
        (mapStruct)->offsets.offsetName = bank + (mapStruct)->mems[NCCL_NET_MAP_DEVMEM].size; \
        (mapStruct)->mems[NCCL_NET_MAP_DEVMEM].size += memSize; \
      } else { \
        (mapStruct)->offsets.offsetName = bank + (mapStruct)->mems[NCCL_NET_MAP_HOSTMEM].size; \
        (mapStruct)->mems[NCCL_NET_MAP_HOSTMEM].size += memSize; \
      } \
    } else { \
      (mapStruct)->offsets.offsetName = bank; \
    } \
} while (0);

/* 中文逐项-类型 T0416：connectMapMem
 * 类型职责：NET或CollNet各bank的CPU/GPU基址、大小、共享名字和控制/数据偏移表。
 * 声明内可见成员（含嵌套成员）：gpuPtr、cpuPtr、size、shmPath、ipc。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct connectMapMem{
/* 中文逐项-变量 V3944：gpuPtr
 * gpuPtr（char；成员 connectMapMem）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 */
  char* gpuPtr;
/* 中文逐项-变量 V3945：cpuPtr
 * cpuPtr（char；成员 connectMapMem）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 */
  char* cpuPtr;
/* 中文逐项-变量 V3946：size
 * size（int；成员 connectMapMem）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  int size;
/* 中文逐项-类型 T0417：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：shmPath、ipc。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V3947：shmPath
 * shmPath（char；成员 connectMapMem::<匿名>）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 */
    char shmPath[PATH_MAX];
/* 中文逐项-变量 V3948：ipc
 * ipc（cudaIpcMemHandle_t；成员 connectMapMem::<匿名>）：可跨进程交换的CUDA内存导出句柄，与导入后本进程实际地址不同。
 */
    cudaIpcMemHandle_t ipc;
  };
};

/* 中文逐项-类型 T0418：connectMap
 * 类型职责：NET或CollNet各bank的CPU/GPU基址、大小、共享名字和控制/数据偏移表。
 * 声明内可见成员（含嵌套成员）：sameProcess、shared、cudaDev、mems、offsets、sendMem、recvMem、buffs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct connectMap {
/* 中文逐项-变量 V3949：sameProcess
 * sameProcess（int；成员 connectMap）：资源拥有者与使用者是否同进程，决定可复用裸指针还是必须导入共享映射。
 */
  int sameProcess;
/* 中文逐项-变量 V3950：shared
 * shared（int；成员 connectMap）：是否使用共享缓冲资源的标志；不自动表示所有 channel 或全部 backend 连接合并。
 */
  int shared;
/* 中文逐项-变量 V3951：cudaDev
 * cudaDev（int；成员 connectMap）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 */
  int cudaDev;
  // First 3 bits of offsets determine the mem bank. 001 is host mem, 011 is dev mem, 101 is shared host mem and 111 is shared dev mem.
/* 中文逐项-变量 V3952：mems
 * mems（struct connectMapMem；成员 connectMap）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
  struct connectMapMem mems[NCCL_NET_MAP_MEMS];
  // Offsets. 3 MSBs indicate mem bank, 111 indicates NULL.
/* 中文逐项-变量 V3953：offsets
 * offsets（struct { uint32_t sendMem; uint32_t recvMem; uint32_t buffs[NCCL_NUM_PROTOCOLS]; }；成员
 * connectMap）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
/* 中文逐项-类型 T0419：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：offsets、sendMem、recvMem、buffs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct {
/* 中文逐项-变量 V3954：sendMem
 * sendMem（uint32_t；成员 connectMap::<匿名>）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
    uint32_t sendMem;
/* 中文逐项-变量 V3955：recvMem
 * recvMem（uint32_t；成员 connectMap::<匿名>）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
    uint32_t recvMem;
/* 中文逐项-变量 V3956：buffs
 * buffs（uint32_t；成员 connectMap::<匿名>）：按 LL/LL128/Simple 区分的协议缓冲基址集合，承载分片流水数据。
 */
    uint32_t buffs[NCCL_NUM_PROTOCOLS];
  } offsets;
};

/* 中文逐项-类型 T0420：sendResources
 * 类型职责：当前transport方向私有的控制/协议缓冲、后端句柄、注册和进度资源，字段依所在文件不同。
 * 声明内可见成员（含嵌套成员）：map、netSendComm、sendMem、recvMem、rank、localRank、remoteRank、netDev、useGdr、maxRecvs、gdcSync、gdrDesc、shared、channelId、connIndex、buffers、buffSizes、mhandles、step、llLastCleaning。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct sendResources {
/* 中文逐项-变量 V3957：map
 * map（struct connectMap；成员 sendResources）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
  struct connectMap map;
/* 中文逐项-变量 V3958：netSendComm
 * netSendComm（void；成员 sendResources）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 */
  void* netSendComm;
/* 中文逐项-变量 V3959：sendMem
 * sendMem（struct ncclSendMem；成员 sendResources）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
  struct ncclSendMem* sendMem;
/* 中文逐项-变量 V3960：recvMem
 * recvMem（struct ncclRecvMem；成员 sendResources）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
  struct ncclRecvMem* recvMem;

/* 中文逐项-变量 V3961：rank
 * rank（int；成员 sendResources）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V3962：localRank
 * localRank（int；成员 sendResources）：本节点内 rank 下标，主要索引本地共享资源。
 */
  int localRank;
/* 中文逐项-变量 V3963：remoteRank
 * remoteRank（int；成员 sendResources）：该后台连接的远端逻辑rank，tag匹配和共享聚合时用它区分来源。
 */
  int remoteRank;
/* 中文逐项-变量 V3964：netDev
 * netDev（int；成员 sendResources）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int netDev;
/* 中文逐项-变量 V3965：useGdr
 * useGdr（int；成员 sendResources）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 */
  int useGdr;
/* 中文逐项-变量 V3966：maxRecvs
 * maxRecvs（int；成员 sendResources）：网络后端支持的并发连接数量及一次Irecv可聚合的接收项数，不等同于QP数量或SRQ。
 */
  int maxRecvs;
/* 中文逐项-变量 V3967：gdcSync
 * gdcSync（uint64_t；成员 sendResources）：可选 GDRCopy 映射控制地址，分别用于低延迟进度发布或接收可见性刷新。
 */
  uint64_t* gdcSync;
/* 中文逐项-变量 V3968：gdrDesc
 * gdrDesc（void；成员 sendResources）：撤销 GDRCopy pin/map 及显存分配所需的描述对象。
 */
  void* gdrDesc;
/* 中文逐项-变量 V3969：shared
 * shared（int；成员 sendResources）：是否使用共享缓冲资源的标志；不自动表示所有 channel 或全部 backend 连接合并。
 */
  int shared;
/* 中文逐项-变量 V3970：channelId
 * channelId（int；成员 sendResources）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 */
  int channelId;
/* 中文逐项-变量 V3971：connIndex
 * connIndex（int；成员 sendResources）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 */
  int connIndex;
/* 中文逐项-变量 V3972：buffers
 * buffers（char；成员 sendResources）：各协议分配/映射的数据缓冲集合，具体是否共享由资源配置决定。
 */
  char* buffers[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3973：buffSizes
 * buffSizes（int；成员 sendResources）：各协议物理缓冲字节容量，先除8得到每step物理容量。
 */
  int buffSizes[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3974：mhandles
 * mhandles（void；成员 sendResources）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 */
  void* mhandles[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3975：step
 * step（uint64_t；成员 sendResources）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 */
  uint64_t step;
/* 中文逐项-变量 V3976：llLastCleaning
 * llLastCleaning（uint64_t；成员 sendResources）：记录 LL 标记回绕清理进度，避免旧行标记被误认为新数据。
 */
  uint64_t llLastCleaning;
};

/* 中文逐项-类型 T0421：recvResources
 * 类型职责：当前transport方向私有的控制/协议缓冲、后端句柄、注册和进度资源，字段依所在文件不同。
 * 声明内可见成员（含嵌套成员）：map、netListenComm、netRecvComm、sendMem、recvMem、rank、localRank、remoteRank、proxyRank、netDev、useGdr、maxRecvs、gdcSync、gdcFlush、gdrDesc、shared、channelId、connIndex、buffers、buffSizes、mhandles、step、llLastCleaning。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct recvResources {
/* 中文逐项-变量 V3977：map
 * map（struct connectMap；成员 recvResources）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
  struct connectMap map;
/* 中文逐项-变量 V3978：netListenComm
 * netListenComm（void；成员 recvResources）：NET后端监听对象，用于accept之后的网络数据连接建立。
 */
  void* netListenComm;
/* 中文逐项-变量 V3979：netRecvComm
 * netRecvComm（void；成员 recvResources）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 */
  void* netRecvComm;
/* 中文逐项-变量 V3980：sendMem
 * sendMem（struct ncclSendMem；成员 recvResources）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
  struct ncclSendMem* sendMem;
/* 中文逐项-变量 V3981：recvMem
 * recvMem（struct ncclRecvMem；成员 recvResources）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
  struct ncclRecvMem* recvMem;

/* 中文逐项-变量 V3982：rank
 * rank（int；成员 recvResources）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V3983：localRank
 * localRank（int；成员 recvResources）：本节点内 rank 下标，主要索引本地共享资源。
 */
  int localRank;
/* 中文逐项-变量 V3984：remoteRank
 * remoteRank（int；成员 recvResources）：该后台连接的远端逻辑rank，tag匹配和共享聚合时用它区分来源。
 */
  int remoteRank;
/* 中文逐项-变量 V3985：proxyRank
 * proxyRank（int；成员 recvResources）：实际负责网络出口的代理 rank，PXN 时可能与发起通信的 GPU rank 不同。
 */
  int proxyRank;
/* 中文逐项-变量 V3986：netDev
 * netDev（int；成员 recvResources）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int netDev;
/* 中文逐项-变量 V3987：useGdr
 * useGdr（int；成员 recvResources）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 */
  int useGdr;
/* 中文逐项-变量 V3988：maxRecvs
 * maxRecvs（int；成员 recvResources）：网络后端支持的并发连接数量及一次Irecv可聚合的接收项数，不等同于QP数量或SRQ。
 */
  int maxRecvs;
/* 中文逐项-变量 V3989：gdcSync
 * gdcSync（uint64_t；成员 recvResources）：可选 GDRCopy 映射控制地址，分别用于低延迟进度发布或接收可见性刷新。
 */
  uint64_t* gdcSync;
/* 中文逐项-变量 V3990：gdcFlush
 * gdcFlush（uint64_t；成员 recvResources）：可选 GDRCopy 映射控制地址，分别用于低延迟进度发布或接收可见性刷新。
 */
  uint64_t* gdcFlush;
/* 中文逐项-变量 V3991：gdrDesc
 * gdrDesc（void；成员 recvResources）：撤销 GDRCopy pin/map 及显存分配所需的描述对象。
 */
  void* gdrDesc;
/* 中文逐项-变量 V3992：shared
 * shared（int；成员 recvResources）：是否使用共享缓冲资源的标志；不自动表示所有 channel 或全部 backend 连接合并。
 */
  int shared;
/* 中文逐项-变量 V3993：channelId
 * channelId（int；成员 recvResources）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 */
  int channelId;
/* 中文逐项-变量 V3994：connIndex
 * connIndex（int；成员 recvResources）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 */
  int connIndex;
/* 中文逐项-变量 V3995：buffers
 * buffers（char；成员 recvResources）：各协议分配/映射的数据缓冲集合，具体是否共享由资源配置决定。
 */
  char* buffers[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3996：buffSizes
 * buffSizes（int；成员 recvResources）：各协议物理缓冲字节容量，先除8得到每step物理容量。
 */
  int buffSizes[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3997：mhandles
 * mhandles（void；成员 recvResources）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 */
  void* mhandles[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3998：step
 * step（uint64_t；成员 recvResources）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 */
  uint64_t step;
/* 中文逐项-变量 V3999：llLastCleaning
 * llLastCleaning（uint64_t；成员 recvResources）：记录 LL 标记回绕清理进度，避免旧行标记被误认为新数据。
 */
  uint64_t llLastCleaning;
};

/* 中文逐项-参数getter P0035：NetDisableIntra
 * 生成函数 ncclParamNetDisableIntra()，读取 NCCL_NET_DISABLE_INTRA，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(NetDisableIntra, "NET_DISABLE_INTRA", 0);

/* Determine if two peers can communicate with NET */
/* 中文内部逻辑 F1074：canConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(info1->hostHash == info2->hostHash)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 条件判断：(ncclParamNetDisableIntra() == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       4. 真分支：；仅条件为真执行以下子项。
 *         5. 调用 / 计算 / 写入：*ret = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 调用 / 计算 / 写入：*ret = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1074
 */
/* 中文逐项-函数 F1074：canConnect
 * 用途与位置：由本文件 transport 的拓扑/主机/后端支持条件判断这对参与者是否可使用该传输。
 * 参数 ret（int* ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 topo（struct ncclTopoSystem* topo）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 info1（struct ncclPeerInfo* info1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 info2（struct ncclPeerInfo* info2）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclParamNetDisableIntra。
 * 需要对照的边界/推进条件：info1->hostHash == info2->hostHash；ncclParamNetDisableIntra() == 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t canConnect(int* ret, struct ncclTopoSystem* topo, struct ncclTopoGraph* graph, struct ncclPeerInfo* info1, struct ncclPeerInfo* info2) {
  // Same host?
  if (info1->hostHash == info2->hostHash) {
    // User disabled NET for intra-node?
    if (ncclParamNetDisableIntra() == 1) {
      *ret = 0;
      return ncclSuccess;
    }
  }
  *ret = 1;
  return ncclSuccess;
}

/* 中文逐项-参数getter P0036：NetSharedBuffers
 * 生成函数 ncclParamNetSharedBuffers()，读取 NCCL_NET_SHARED_BUFFERS，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(NetSharedBuffers, "NET_SHARED_BUFFERS", -2);
/* 中文逐项-参数getter P0037：NetSharedComms
 * 生成函数 ncclParamNetSharedComms()，读取 NCCL_NET_SHARED_COMMS，编译默认值为 1。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(NetSharedComms, "NET_SHARED_COMMS", 1);

/* 中文逐项-类型 T0422：setupReq
 * 类型职责：主线程传给实际代理的建链参数包，包含设备、channel、peer等必要策略信息。
 * 声明内可见成员（含嵌套成员）：rank、localRank、remoteRank、shared、netDev、useGdr、channelId、connIndex。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct setupReq {
/* 中文逐项-变量 V4000：rank
 * rank（int；成员 setupReq）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V4001：localRank
 * localRank（int；成员 setupReq）：本节点内 rank 下标，主要索引本地共享资源。
 */
  int localRank;
/* 中文逐项-变量 V4002：remoteRank
 * remoteRank（int；成员 setupReq）：该后台连接的远端逻辑rank，tag匹配和共享聚合时用它区分来源。
 */
  int remoteRank;
/* 中文逐项-变量 V4003：shared
 * shared（int；成员 setupReq）：是否使用共享缓冲资源的标志；不自动表示所有 channel 或全部 backend 连接合并。
 */
  int shared;
/* 中文逐项-变量 V4004：netDev
 * netDev（int；成员 setupReq）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int netDev;
/* 中文逐项-变量 V4005：useGdr
 * useGdr（int；成员 setupReq）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 */
  int useGdr;
/* 中文逐项-变量 V4006：channelId
 * channelId（int；成员 setupReq）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 */
  int channelId;
/* 中文逐项-变量 V4007：connIndex
 * connIndex（int；成员 setupReq）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 */
  int connIndex;
};

/* Determine if we will use this transport for this peer and return connect
 * information for this peer */
/* 中文导读：
 * NET 发送方向配置：选择 NIC/Proxy rank，判断 GDR 能力，然后向 service 发送 setup 请求。
 * graph 非空时此处使用专用缓冲；普通无图 P2P 默认可使用共享缓冲。setup 与 connect 是分离阶段。
 */
/* 中文内部逻辑 F1075：sendSetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct setupReq req;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：send->conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ? ncclParamNetSharedBuffers() : 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：req.channelId = channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：req.connIndex = connIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 准备局部状态：int proxyRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, peerInfo->rank, &req.netDev, &proxyRank));；失败时可能提前return，成功才继续；按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 1, &req.useGdr));；失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。
 * 8. 调用 / 计算 / 写入：send->conn.direct |= req.useGdr ? NCCL_DIRECT_NIC : 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, proxyRank, &send->proxyConn));；失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。
 * 10. 调用 / 计算 / 写入：req.rank = myInfo->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &req.localRank));；失败时可能提前return，成功才继续；把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。
 * 12. 调用 / 计算 / 写入：req.remoteRank = peerInfo->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), NULL, 0));；失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 14. 条件判断：(proxyRank == myInfo->rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [send] via NET/%s/%d%s%s", channelId, connIndex, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, ncclNetName(), req.netDev,  …（完整代码见函数体）；读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   17. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     18. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [send] via NET/%s/%d(%d)%s%s", channelId, connIndex, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, ncclNetName(), req.netD …（完整代码见函数体）；读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 19. 调用 / 计算 / 写入：*((int*)connectInfo) = proxyRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1075
 */
/* 中文逐项-函数 F1075：sendSetup
 * 用途与位置：为对应方向选择设备/代理并准备本地控制资源，生成可经 bootstrap 交换的建链句柄。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 myInfo（struct ncclPeerInfo* myInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peerInfo（struct ncclPeerInfo* peerInfo）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNetName；ncclParamNetSharedBuffers；ncclProxyCall；ncclProxyConnect；ncclTopoCheckGdr；ncclTopoGetLocalRank；ncclTopoGetNetDev。
 * 需要对照的边界/推进条件：proxyRank == myInfo->rank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclConnect* connectInfo, struct ncclConnector* send, int channelId, int connIndex) {
/* 中文逐项-变量 V4008：req
 * req（struct setupReq；函数 sendSetup 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 此处未显式给初值；后续首个文本使用点：send->conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ?
 * ncclParamNetSharedBuffers() : 1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct setupReq req;

  send->conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ? ncclParamNetSharedBuffers() : 1;
  req.channelId = channelId;
  req.connIndex = connIndex;

/* 中文逐项-变量 V4009：proxyRank
 * proxyRank（int；函数 sendSetup 内）：实际负责网络出口的代理 rank，PXN 时可能与发起通信的 GPU rank 不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, peerInfo->rank, &req.netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int proxyRank;
  NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, peerInfo->rank, &req.netDev, &proxyRank));
  NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 1, &req.useGdr));
  send->conn.direct |= req.useGdr ? NCCL_DIRECT_NIC : 0;

  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, proxyRank, &send->proxyConn));
  req.rank = myInfo->rank;
  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &req.localRank));
  req.remoteRank = peerInfo->rank;
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), NULL, 0));

  if (proxyRank == myInfo->rank) {
    INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [send] via NET/%s/%d%s%s", channelId, connIndex, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, ncclNetName(), req.netDev,
        req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");
  } else {
    INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [send] via NET/%s/%d(%d)%s%s", channelId, connIndex, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, ncclNetName(), req.netDev,
        proxyRank, req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");
  }
  *((int*)connectInfo) = proxyRank;
  return ncclSuccess;
}

// GDRCOPY support: TAIL_ENABLE When enabled locates the RX proxy tail in CUDA memory
/* 中文逐项-参数getter P0038：GdrCopySyncEnable
 * 生成函数 ncclParamGdrCopySyncEnable()，读取 NCCL_GDRCOPY_SYNC_ENABLE，编译默认值为 1。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(GdrCopySyncEnable, "GDRCOPY_SYNC_ENABLE", 1);
// GDRCOPY support: FLUSH_ENABLE When enabled uses a PCI-E read to flush GDRDMA buffers
/* 中文逐项-参数getter P0039：GdrCopyFlushEnable
 * 生成函数 ncclParamGdrCopyFlushEnable()，读取 NCCL_GDRCOPY_FLUSH_ENABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(GdrCopyFlushEnable, "GDRCOPY_FLUSH_ENABLE", 0);

/* Setup recv connector */
/* 中文内部逻辑 F1076：recvSetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct setupReq req;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：recv->conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ? ncclParamNetSharedBuffers() : 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：req.channelId = channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：req.connIndex = connIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 准备局部状态：int proxyRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, myInfo->rank, &req.netDev, &proxyRank));；失败时可能提前return，成功才继续；按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 0, &req.useGdr));；失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 0, myInfo->rank, &recv->proxyConn));；失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。
 * 9. 调用 / 计算 / 写入：req.rank = myInfo->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &req.localRank));；失败时可能提前return，成功才继续；把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。
 * 11. 调用 / 计算 / 写入：req.remoteRank = peerInfo->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), connectInfo, sizeof(ncclNetHandle_t)));；失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 13. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [receive] via NET/%s/%d%s%s", channelId, connIndex, peerInfo->rank, peerInfo->busId, myInfo->rank, myInfo->busId, ncclNetName(), req.netDe …（完整代码见函数体）；读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 14. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1076
 */
/* 中文逐项-函数 F1076：recvSetup
 * 用途与位置：为对应方向选择设备/代理并准备本地控制资源，生成可经 bootstrap 交换的建链句柄。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 myInfo（struct ncclPeerInfo* myInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peerInfo（struct ncclPeerInfo* peerInfo）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNetName；ncclParamNetSharedBuffers；ncclProxyCall；ncclProxyConnect；ncclTopoCheckGdr；ncclTopoGetLocalRank；ncclTopoGetNetDev。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclConnect* connectInfo, struct ncclConnector* recv, int channelId, int connIndex) {
/* 中文逐项-变量 V4010：req
 * req（struct setupReq；函数 recvSetup 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 此处未显式给初值；后续首个文本使用点：recv->conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ?
 * ncclParamNetSharedBuffers() : 1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct setupReq req;

  recv->conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ? ncclParamNetSharedBuffers() : 1;
  req.channelId = channelId;
  req.connIndex = connIndex;

  // Use myInfo->rank as the receiver uses its own NIC
/* 中文逐项-变量 V4011：proxyRank
 * proxyRank（int；函数 recvSetup 内）：实际负责网络出口的代理 rank，PXN 时可能与发起通信的 GPU rank 不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, myInfo->rank, &req.netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int proxyRank;
  NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, myInfo->rank, &req.netDev, &proxyRank));
  NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 0, &req.useGdr));

  // We don't support PXN on receive yet
  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 0, myInfo->rank, &recv->proxyConn));

  req.rank = myInfo->rank;
  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &req.localRank));
  req.remoteRank = peerInfo->rank;
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), connectInfo, sizeof(ncclNetHandle_t)));

  INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [receive] via NET/%s/%d%s%s", channelId, connIndex, peerInfo->rank, peerInfo->busId, myInfo->rank, myInfo->busId, ncclNetName(), req.netDev,
      req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");
  return ncclSuccess;
}

/* 中文内部逻辑 F1077：netMapShm
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclShmOpen(mem->shmPath, mem->size, (void**)&mem->cpuPtr, (void**)&mem->gpuPtr, 0));；失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclShmUnlink(mem->shmPath));；失败时可能提前return，成功才继续；删除共享内存文件名，已打开的映射仍由各进程按生命周期单独解除。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1077
 */
/* 中文逐项-函数 F1077：netMapShm
 * 用途与位置：把已有共享主机内存描述映射到当前进程，建立 CPU 与可选设备地址视图。
 * 参数 mem（struct connectMapMem* mem）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net.cc::sendConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclShmOpen；ncclShmUnlink。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t netMapShm(struct connectMapMem* mem) {
  NCCLCHECK(ncclShmOpen(mem->shmPath, mem->size, (void**)&mem->cpuPtr, (void**)&mem->gpuPtr, 0));
  NCCLCHECK(ncclShmUnlink(mem->shmPath));
  return ncclSuccess;
}
/* 中文内部逻辑 F1078：netCreateShm
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：mem->shmPath[0] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclShmOpen(mem->shmPath, mem->size, (void**)&mem->cpuPtr, NULL, 1));；失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1078
 */
/* 中文逐项-函数 F1078：netCreateShm
 * 用途与位置：创建带共享名字的主机内存映射，供不同进程的 NET/Proxy 协作访问。
 * 参数 mem（struct connectMapMem* mem）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net.cc::sendProxyConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclShmOpen。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t netCreateShm(struct connectMapMem* mem) {
  mem->shmPath[0] = '\0'; // Let ncclShmOpen create a tmp file
  NCCLCHECK(ncclShmOpen(mem->shmPath, mem->size, (void**)&mem->cpuPtr, NULL, 1));
  return ncclSuccess;
}

/* 中文内部逻辑 F1079：netDumpMap
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：printf("Dump map same process %d shared %d\n", map->sameProcess, map->shared);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 准备局部状态：struct connectMapMem *mem = map->mems+NCCL_NET_MAP_HOSTMEM;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：printf("Mem 0: Host mem %s (%x B) CPU %p GPU %p\n", mem->shmPath, mem->size, mem->cpuPtr, mem->gpuPtr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 调用 / 计算 / 写入：mem = map->mems+NCCL_NET_MAP_DEVMEM;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：printf("Mem 1: Vid mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 调用 / 计算 / 写入：mem = map->mems+NCCL_NET_MAP_SHARED_HOSTMEM;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：printf("Mem 2: Shared Host mem %s (%x B) CPU %p GPU %p\n", mem->shmPath, mem->size, mem->cpuPtr, mem->gpuPtr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 调用 / 计算 / 写入：mem = map->mems+NCCL_NET_MAP_SHARED_DEVMEM;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：printf("Mem 3: Shared Vid mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 调用 / 计算 / 写入：printf("SendMem -> Used %d Bank %d Offset %x, cpu %p gpu %p\n", map->offsets.sendMem & NCCL_NET_MAP_MASK_USED ? 1 : 0, NCCL_NET_MAP_OFFSET_BANK(map, sendMem), map->offsets.sendMem & NCCL_NET_MAP_MASK_ …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 11. 调用 / 计算 / 写入：printf("RecvMem -> Used %d Bank %d Offset %x, cpu %p gpu %p\n", map->offsets.recvMem & NCCL_NET_MAP_MASK_USED ? 1 : 0, NCCL_NET_MAP_OFFSET_BANK(map, recvMem), map->offsets.recvMem & NCCL_NET_MAP_MASK_ …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 12. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   13. 调用 / 计算 / 写入：printf("Proto %d -> Used %d Bank %d Offset %x, cpu %p, gpu %p\n", p, map->offsets.buffs[p] & NCCL_NET_MAP_MASK_USED ? 1 : 0, NCCL_NET_MAP_OFFSET_BANK(map, buffs[p]), map->offsets.buffs[p] & NCCL_NET_M …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 15. 调用 / 计算 / 写入：printf("End of dump\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1079
 */
/* 中文逐项-函数 F1079：netDumpMap
 * 用途与位置：打印后端各内存 bank 的 CPU/GPU 地址、大小和映射偏移，辅助检查跨进程地址视图。
 * 参数 map（struct connectMap* map）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCL_NET_MAP_GET_POINTER；NCCL_NET_MAP_OFFSET_BANK。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t netDumpMap(struct connectMap* map) {
  printf("Dump map same process %d shared %d\n", map->sameProcess, map->shared);
/* 中文逐项-变量 V4012：mem
 * mem（struct connectMapMem；函数 netDumpMap 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mem = map->mems+NCCL_NET_MAP_HOSTMEM；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMapMem *mem = map->mems+NCCL_NET_MAP_HOSTMEM;
  printf("Mem 0: Host mem %s (%x B) CPU %p GPU %p\n", mem->shmPath, mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_DEVMEM;
  printf("Mem 1: Vid  mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_SHARED_HOSTMEM;
  printf("Mem 2: Shared Host mem %s (%x B) CPU %p GPU %p\n", mem->shmPath, mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_SHARED_DEVMEM;
  printf("Mem 3: Shared Vid mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  printf("SendMem -> Used %d Bank %d Offset %x, cpu %p gpu %p\n",
      map->offsets.sendMem & NCCL_NET_MAP_MASK_USED ? 1 : 0,
      NCCL_NET_MAP_OFFSET_BANK(map, sendMem), map->offsets.sendMem & NCCL_NET_MAP_MASK_OFFSET,
      NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem), NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem));
  printf("RecvMem -> Used %d Bank %d Offset %x, cpu %p gpu %p\n",
      map->offsets.recvMem & NCCL_NET_MAP_MASK_USED ? 1 : 0,
      NCCL_NET_MAP_OFFSET_BANK(map, recvMem), map->offsets.recvMem & NCCL_NET_MAP_MASK_OFFSET,
      NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem), NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem));
  for (/* 中文逐项-变量 V4013：p
 * p（int；函数 netDumpMap 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    printf("Proto %d -> Used %d Bank %d Offset %x, cpu %p, gpu %p\n", p,
        map->offsets.buffs[p] & NCCL_NET_MAP_MASK_USED ? 1 : 0,
        NCCL_NET_MAP_OFFSET_BANK(map, buffs[p]), map->offsets.buffs[p] & NCCL_NET_MAP_MASK_OFFSET,
        NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]), NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]));
  }
  printf("End of dump\n");
  return ncclSuccess;
}

/* 中文内部逻辑 F1080：sendConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct connectMap* map;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&map, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：send->transportResources = map;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgConnect, connectInfo, sizeof(ncclNetHandle_t), map, sizeof(struct connectMap)));；失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 5. 条件判断：(map->sameProcess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 条件判断：(map->cudaDev != comm->cudaDev)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       8. 真分支：；仅条件为真执行以下子项。
 *         9. 准备局部状态：cudaError_t err = cudaDeviceEnablePeerAccess(map->cudaDev, 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         10. 条件判断：(err == cudaErrorPeerAccessAlreadyEnabled)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           11. 真分支：；仅条件为真执行以下子项。
 *             12. 调用 / 计算 / 写入：cudaGetLastError();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           13. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             14. 条件判断：(err != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               15. 真分支：；仅条件为真执行以下子项。
 *                 16. 调用 / 计算 / 写入：WARN("failed to peer with device %d: %d %s", map->cudaDev, err, cudaGetErrorString(err));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 17. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   18. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     19. 调用 / 计算 / 写入：NCCLCHECK(netMapShm(map->mems+NCCL_NET_MAP_HOSTMEM));；失败时可能提前return，成功才继续；把已有共享主机内存描述映射到当前进程，建立 CPU 与可选设备地址视图。
 *     20. 条件判断：(map->mems[NCCL_NET_MAP_DEVMEM].size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       21. 真分支：；仅条件为真执行以下子项。
 *         22. 调用 / 计算 / 写入：CUDACHECK(cudaIpcOpenMemHandle((void**)&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].ipc, cudaIpcMemLazyEnablePeerAccess));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         23. 调用 / 计算 / 写入：map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     24. 条件判断：(map->mems[NCCL_NET_MAP_SHARED_DEVMEM].size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       25. 真分支：；仅条件为真执行以下子项。
 *         26. 准备局部状态：void** sharedDevMemPtr = comm->proxyState.sharedDevMems+send->proxyConn.localRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         27. 条件判断：(*sharedDevMemPtr == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           28. 真分支：；仅条件为真执行以下子项。
 *             29. 调用 / 计算 / 写入：CUDACHECK(cudaIpcOpenMemHandle(sharedDevMemPtr, map->mems[NCCL_NET_MAP_SHARED_DEVMEM].ipc, cudaIpcMemLazyEnablePeerAccess));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         30. 调用 / 计算 / 写入：map->mems[NCCL_NET_MAP_SHARED_DEVMEM].gpuPtr = (char*)(*sharedDevMemPtr);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         31. 调用 / 计算 / 写入：map->mems[NCCL_NET_MAP_SHARED_DEVMEM].cpuPtr = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 准备局部状态：struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 33. 准备局部状态：void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 34. 调用 / 计算 / 写入：send->conn.head = gdcMem ? (uint64_t*)gdcMem : &sendMem->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 35. 准备局部状态：struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 36. 调用 / 计算 / 写入：send->conn.tail = &recvMem->tail;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 37. 调用 / 计算 / 写入：send->conn.sizesFifo = recvMem->sizesFifo;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 38. 调用 / 计算 / 写入：send->conn.offsFifo = map->shared ? recvMem->offsFifo : NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 39. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   40. 调用 / 计算 / 写入：send->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 41. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 42. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1080
 */
/* 中文逐项-函数 F1080：sendConnect
 * 用途与位置：消费远端句柄完成对应方向的连接和地址映射，把协议缓冲/进度指针填写到设备连接视图。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；NCCL_NET_MAP_GET_POINTER；cudaDeviceEnablePeerAccess；cudaGetErrorString；cudaGetLastError；cudaIpcOpenMemHandle；ncclCalloc；ncclProxyCall；netMapShm。
 * 需要对照的边界/推进条件：map->sameProcess；map->cudaDev != comm->cudaDev；err == cudaErrorPeerAccessAlreadyEnabled；err !=
 * cudaSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendConnect(struct ncclComm* comm, struct ncclConnect* connectInfo, int nranks, int rank, struct ncclConnector* send) {
  // Setup device pointers
/* 中文逐项-变量 V4014：map
 * map（struct connectMap；函数 sendConnect 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&map, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct connectMap* map;
  NCCLCHECK(ncclCalloc(&map, 1));
  send->transportResources = map;
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgConnect, connectInfo, sizeof(ncclNetHandle_t), map, sizeof(struct connectMap)));

  if (map->sameProcess) {
    if (map->cudaDev != comm->cudaDev) {
      // Enable P2P access
/* 中文逐项-变量 V4015：err
 * err（cudaError_t；函数 sendConnect 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 err = cudaDeviceEnablePeerAccess(map->cudaDev, 0)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      cudaError_t err = cudaDeviceEnablePeerAccess(map->cudaDev, 0);
      if (err == cudaErrorPeerAccessAlreadyEnabled) {
        cudaGetLastError();
      } else if (err != cudaSuccess) {
        WARN("failed to peer with device %d: %d %s", map->cudaDev, err, cudaGetErrorString(err));
        return ncclInternalError;
      }
    }
  } else {
    NCCLCHECK(netMapShm(map->mems+NCCL_NET_MAP_HOSTMEM));
    if (map->mems[NCCL_NET_MAP_DEVMEM].size) {
      CUDACHECK(cudaIpcOpenMemHandle((void**)&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].ipc, cudaIpcMemLazyEnablePeerAccess));
      map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = NULL;
    }
    if (map->mems[NCCL_NET_MAP_SHARED_DEVMEM].size) {
/* 中文逐项-变量 V4016：sharedDevMemPtr
 * sharedDevMemPtr（void；函数 sendConnect 内）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 初值/来源 sharedDevMemPtr = comm->proxyState.sharedDevMems+send->proxyConn.localRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void** sharedDevMemPtr = comm->proxyState.sharedDevMems+send->proxyConn.localRank;
      if (*sharedDevMemPtr == NULL) {
        CUDACHECK(cudaIpcOpenMemHandle(sharedDevMemPtr, map->mems[NCCL_NET_MAP_SHARED_DEVMEM].ipc, cudaIpcMemLazyEnablePeerAccess));
      }
      map->mems[NCCL_NET_MAP_SHARED_DEVMEM].gpuPtr = (char*)(*sharedDevMemPtr);
      map->mems[NCCL_NET_MAP_SHARED_DEVMEM].cpuPtr = NULL;
    }
  }
  //NCCLCHECK(netDumpMap(map));

/* 中文逐项-变量 V4017：sendMem
 * sendMem（struct ncclSendMem；函数 sendConnect 内）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 * 初值/来源 sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu,
 * sendMem)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);
/* 中文逐项-变量 V4018：gdcMem
 * gdcMem（void；函数 sendConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;
  send->conn.head = gdcMem ? (uint64_t*)gdcMem : &sendMem->head;

/* 中文逐项-变量 V4019：recvMem
 * recvMem（struct ncclRecvMem；函数 sendConnect 内）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 * 初值/来源 recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu,
 * recvMem)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);
  send->conn.tail = &recvMem->tail;
  send->conn.sizesFifo = recvMem->sizesFifo;
  // Only fuse P2P buffers, continue to allocate dedicated buffers for ring/tree
  send->conn.offsFifo = map->shared ? recvMem->offsFifo : NULL;

  for (/* 中文逐项-变量 V4020：p
 * p（int；函数 sendConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    send->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);
  return ncclSuccess;
}

/* Connect to this peer */
/* 中文内部逻辑 F1081：recvConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct connectMap* map;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&map, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：recv->transportResources = map;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgConnect, connectInfo, sizeof(int), map, sizeof(struct connectMap)));；失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 5. 准备局部状态：struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：recv->conn.head = &sendMem->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 7. 准备局部状态：struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 准备局部状态：void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 调用 / 计算 / 写入：recv->conn.tail = gdcMem ? (uint64_t*)gdcMem : &recvMem->tail;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 10. 调用 / 计算 / 写入：recv->conn.sizesFifo = recvMem->sizesFifo;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：recv->conn.offsFifo = map->shared ? recvMem->offsFifo : NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   13. 调用 / 计算 / 写入：recv->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1081
 */
/* 中文逐项-函数 F1081：recvConnect
 * 用途与位置：消费远端句柄完成对应方向的连接和地址映射，把协议缓冲/进度指针填写到设备连接视图。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCL_NET_MAP_GET_POINTER；ncclCalloc；ncclProxyCall。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvConnect(struct ncclComm* comm, struct ncclConnect* connectInfo, int nranks, int rank, struct ncclConnector* recv) {
/* 中文逐项-变量 V4021：map
 * map（struct connectMap；函数 recvConnect 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&map, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct connectMap* map;
  NCCLCHECK(ncclCalloc(&map, 1));
  recv->transportResources = map;
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgConnect, connectInfo, sizeof(int), map, sizeof(struct connectMap)));
  //NCCLCHECK(netDumpMap(map));

/* 中文逐项-变量 V4022：sendMem
 * sendMem（struct ncclSendMem；函数 recvConnect 内）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 * 初值/来源 sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu,
 * sendMem)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);
  recv->conn.head = &sendMem->head;

/* 中文逐项-变量 V4023：recvMem
 * recvMem（struct ncclRecvMem；函数 recvConnect 内）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 * 初值/来源 recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu,
 * recvMem)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);
/* 中文逐项-变量 V4024：gdcMem
 * gdcMem（void；函数 recvConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;
  recv->conn.tail = gdcMem ? (uint64_t*)gdcMem : &recvMem->tail;
  recv->conn.sizesFifo = recvMem->sizesFifo;
  // Only fuse P2P buffers, continue to allocate dedicated buffers for ring/tree
  recv->conn.offsFifo = map->shared ? recvMem->offsFifo : NULL;

  for (/* 中文逐项-变量 V4025：p
 * p（int；函数 recvConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    recv->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);
  return ncclSuccess;
}

/* 中文内部逻辑 F1082：sendFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct connectMap* map = (struct connectMap*)(send->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(map->sameProcess == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：NCCLCHECK(ncclShmClose(map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));；失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。
 *     5. 条件判断：(map->mems[NCCL_NET_MAP_DEVMEM].size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       6. 真分支：；仅条件为真执行以下子项。
 *         7. 调用 / 计算 / 写入：CUDACHECK(cudaIpcCloseMemHandle(map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1082
 */
/* 中文逐项-函数 F1082：sendFree
 * 用途与位置：释放对应逻辑连接持有的本地资源/映射，并按代理与共享计数规则释放后台对象。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaIpcCloseMemHandle；ncclShmClose。
 * 需要对照的边界/推进条件：map->sameProcess == 0；map->mems[NCCL_NET_MAP_DEVMEM].size。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendFree(struct ncclConnector* send) {
/* 中文逐项-变量 V4026：map
 * map（struct connectMap；函数 sendFree 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 map = (struct connectMap*)(send->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMap* map = (struct connectMap*)(send->transportResources);
  if (map->sameProcess == 0) {
    NCCLCHECK(ncclShmClose(map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
    if (map->mems[NCCL_NET_MAP_DEVMEM].size) {
      CUDACHECK(cudaIpcCloseMemHandle(map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr));
    }
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1083：recvFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1083
 */
/* 中文逐项-函数 F1083：recvFree
 * 用途与位置：释放对应逻辑连接持有的本地资源/映射，并按代理与共享计数规则释放后台对象。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvFree(struct ncclConnector* recv) {
  return ncclSuccess;
}

/* 中文逐项-常量 X1266：NCCL_SHARED_STEPS
 * 编译期定义：NCCL_SHARED_STEPS = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SHARED_STEPS 16
/* 中文导读：
 * 为某个 localRank 和方向维护共享缓冲池，按 channel/槽位切分空间，并用引用数管理生命周期。
 * 共享缓冲可让多个逻辑 peer 注册相同页范围；它与共享 NET 连接是两个独立的复用维度。
 */
/* 中文内部逻辑 F1084：sharedBuffersInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(cuda == 0 && sameProcess == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：WARN("PXN should not use host buffers for data");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 条件判断：(progressState->localPeers == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 9. 准备局部状态：struct ncclProxyPeer** localPeers = progressState->localPeers;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 条件判断：(localPeers[localRank] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(localPeers+localRank, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 13. 准备局部状态：struct ncclProxyPeer* peer = localPeers[localRank];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 准备局部状态：struct ncclProxySharedP2p* state = type == 0 ? &peer->send : &peer->recv;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 调用 / 计算 / 写入：state->refcount++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 条件判断：(state->size == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   17. 真分支：；仅条件为真执行以下子项。
 *     18. 调用 / 计算 / 写入：state->size = nChannels*(NCCL_SHARED_STEPS/NCCL_STEPS)*comm->buffSizes[NCCL_PROTO_SIMPLE]/SENDRECV_SLICEFACTOR;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 条件判断：(size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 调用 / 计算 / 写入：*size = state->size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 22. 条件判断：(cuda && state->cudaBuff == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   23. 真分支：；仅条件为真执行以下子项。
 *     24. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaCalloc(&state->cudaBuff, state->size));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     25. 条件判断：(sameProcess == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       26. 真分支：；仅条件为真执行以下子项。
 *         27. 调用 / 计算 / 写入：CUDACHECK(cudaIpcGetMemHandle(&state->ipc, state->cudaBuff));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 28. 条件判断：(!cuda && state->hostBuff == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   29. 真分支：；仅条件为真执行以下子项。
 *     30. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaHostCalloc(&state->hostBuff, state->size));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 31. 条件判断：(cpuPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   32. 真分支：；仅条件为真执行以下子项。
 *     33. 调用 / 计算 / 写入：*cpuPtr = cuda ? state->cudaBuff : state->hostBuff;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 34. 条件判断：(sameProcess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   35. 真分支：；仅条件为真执行以下子项。
 *     36. 条件判断：(gpuPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       37. 真分支：；仅条件为真执行以下子项。
 *         38. 调用 / 计算 / 写入：*gpuPtr = *cpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   39. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     40. 条件判断：(gpuPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       41. 真分支：；仅条件为真执行以下子项。
 *         42. 调用 / 计算 / 写入：*gpuPtr = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     43. 条件判断：(ipc)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       44. 真分支：；仅条件为真执行以下子项。
 *         45. 调用 / 计算 / 写入：memcpy(ipc, &state->ipc, sizeof(cudaIpcMemHandle_t));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 46. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1084
 */
/* 中文逐项-函数 F1084：sharedBuffersInit
 * 用途与位置：按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 cuda（int cuda）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 localRank（int localRank）：本节点内 rank 下标，主要索引本地共享资源。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 sameProcess（int sameProcess）：资源拥有者与使用者是否同进程，决定可复用裸指针还是必须导入共享映射。
 * 参数 nChannels（int nChannels）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 参数 gpuPtr（char** gpuPtr）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 参数 cpuPtr（char** cpuPtr）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 参数 size（int* size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 ipc（cudaIpcMemHandle_t* ipc）：可跨进程交换的CUDA内存导出句柄，与导入后本进程实际地址不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyConnect；src/transport/coll_net.cc::recvProxyConnect；src/transport/net.cc::proxySharedInit；src/transport/net.cc::sendProxyConnect；src/transport/net.cc::recvProxyConnect。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaIpcGetMemHandle；memcpy；ncclCalloc；ncclCudaCalloc；ncclCudaHostCalloc。
 * 需要对照的边界/推进条件：cuda == 0 && sameProcess == 0；progressState->localPeers == NULL；localPeers[localRank] ==
 * NULL；state->size == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sharedBuffersInit(struct ncclComm* comm, int cuda, int localRank, int type, int sameProcess,
    int nChannels, char** gpuPtr, char** cpuPtr, int* size, cudaIpcMemHandle_t* ipc) {
  if (cuda == 0 && sameProcess == 0) {
      WARN("PXN should not use host buffers for data");
      return ncclInternalError;
  }
/* 中文逐项-变量 V4027：progressState
 * progressState（struct ncclProxyProgressState；函数 sharedBuffersInit 内）：communicator的代理总状态或数据推进子状态，分别组织服务控制和活跃请求。
 * 初值/来源 progressState = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;
  if (progressState->localPeers == NULL) {
    NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));
  }
/* 中文逐项-变量 V4028：localPeers
 * localPeers（struct ncclProxyPeer；函数 sharedBuffersInit 内）：代理本地peer控制对象、控制socket及引导交换的地址表。
 * 初值/来源 localPeers = progressState->localPeers；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyPeer** localPeers = progressState->localPeers;
  if (localPeers[localRank] == NULL) {
    NCCLCHECK(ncclCalloc(localPeers+localRank, 1));
  }
/* 中文逐项-变量 V4029：peer
 * peer（struct ncclProxyPeer；函数 sharedBuffersInit 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = localPeers[localRank]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  struct ncclProxyPeer* peer = localPeers[localRank];
/* 中文逐项-变量 V4030：state
 * state（struct ncclProxySharedP2p；函数 sharedBuffersInit 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = type == 0 ? &peer->send : &peer->recv；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxySharedP2p* state = type == 0 ? &peer->send : &peer->recv;
  state->refcount++;
  if (state->size == 0) {
    state->size = nChannels*(NCCL_SHARED_STEPS/NCCL_STEPS)*comm->buffSizes[NCCL_PROTO_SIMPLE]/SENDRECV_SLICEFACTOR;
  }

  if (size) *size = state->size;

  if (cuda && state->cudaBuff == NULL) {
    NCCLCHECK(ncclCudaCalloc(&state->cudaBuff, state->size));
    if (sameProcess == 0) {
      CUDACHECK(cudaIpcGetMemHandle(&state->ipc, state->cudaBuff));
    }
  }
  if (!cuda && state->hostBuff == NULL) {
    NCCLCHECK(ncclCudaHostCalloc(&state->hostBuff, state->size));
  }
  if (cpuPtr) *cpuPtr = cuda ? state->cudaBuff : state->hostBuff;
  if (sameProcess) {
    if (gpuPtr) *gpuPtr = *cpuPtr;
  } else {
    if (gpuPtr) *gpuPtr = NULL;
    if (ipc) memcpy(ipc, &state->ipc, sizeof(cudaIpcMemHandle_t));
  }
  return ncclSuccess;
}

/* 中文精读：共享数据池如何避免每连接分一整块缓冲
 * 按 channel 和活跃 slot 算出共享池偏移，再通过 offsFifo 通知 GPU 本次应访问哪一段。
 * 逻辑 connector 仍独立保存进度；复用的是有限在途数据槽，不是让所有 peer 同时覆盖同一地址。
 * 共享池减少缓冲内存和可能的重复注册；连接是否共享还要看 NET_SHARED_COMMS 等独立条件。
 * 具体偏移用字节，设备端使用 T* 时需除 sizeof(T)。
 */
/* 中文内部逻辑 F1085：sharedBuffersGet
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int slotSize = comm->buffSizes[NCCL_PROTO_SIMPLE]/(NCCL_STEPS*SENDRECV_SLICEFACTOR);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int globalSlot = (channel*NCCL_SHARED_STEPS)+slot;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：*offset = slotSize * globalSlot;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1085
 */
/* 中文逐项-函数 F1085：sharedBuffersGet
 * 用途与位置：按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 channel（int channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 slot（int slot）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 参数 offset（int* offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyProgress；src/transport/coll_net.cc::recvProxyProgress；src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sharedBuffersGet(struct ncclComm* comm, int channel, int slot, int* offset) {
  // Use different pools for different channels and also separate send/recv.
/* 中文逐项-变量 V4031：slotSize
 * slotSize（int；函数 sharedBuffersGet 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 slotSize =
 * comm->buffSizes[NCCL_PROTO_SIMPLE]/(NCCL_STEPS*SENDRECV_SLICEFACTOR)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int slotSize = comm->buffSizes[NCCL_PROTO_SIMPLE]/(NCCL_STEPS*SENDRECV_SLICEFACTOR);
/* 中文逐项-变量 V4032：globalSlot
 * globalSlot（int；函数 sharedBuffersGet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 globalSlot = (channel*NCCL_SHARED_STEPS)+slot；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int globalSlot = (channel*NCCL_SHARED_STEPS)+slot;
  *offset = slotSize * globalSlot;
  return ncclSuccess;
}

/* 中文内部逻辑 F1086：sharedBuffersDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(comm->proxyState.progressState.localPeers == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：NCCLCHECK(ncclInternalError);；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 准备局部状态：struct ncclProxyPeer* peer = comm->proxyState.progressState.localPeers[localRank];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(peer == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(ncclInternalError;)；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 8. 准备局部状态：struct ncclProxySharedP2p* state = type == 0 ? &peer->send : &peer->recv;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 条件判断：(state->size == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 调用 / 计算 / 写入：NCCLCHECK(ncclInternalError);；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 12. 调用 / 计算 / 写入：state->refcount--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 条件判断：(state->refcount == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 条件判断：(state->cudaBuff)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       16. 真分支：；仅条件为真执行以下子项。
 *         17. 调用 / 计算 / 写入：CUDACHECK(cudaFree(state->cudaBuff));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     18. 条件判断：(state->hostBuff)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       19. 真分支：；仅条件为真执行以下子项。
 *         20. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaHostFree(state->hostBuff));；失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。
 * 21. 条件判断：(peer->send.refcount || peer->recv.refcount)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   22. 真分支：；仅条件为真执行以下子项。
 *     23. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 24. 调用 / 计算 / 写入：free(peer);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 25. 调用 / 计算 / 写入：comm->proxyState.progressState.localPeers[localRank] = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 26. 循环：初始 int r=0;；继续条件 r<comm->localRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   27. 条件判断：(comm->proxyState.progressState.localPeers[r])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     28. 真分支：；仅条件为真执行以下子项。
 *       29. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 30. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 31. 调用 / 计算 / 写入：free(comm->proxyState.progressState.localPeers);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 32. 调用 / 计算 / 写入：comm->proxyState.progressState.localPeers = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 33. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1086
 */
/* 中文逐项-函数 F1086：sharedBuffersDestroy
 * 用途与位置：降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 localRank（int localRank）：本节点内 rank 下标，主要索引本地共享资源。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyFree；src/transport/coll_net.cc::recvProxyFree；src/transport/net.cc::sendProxyFree；src/transport/net.cc::recvProxyFree。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaFree；free；ncclCudaHostFree。
 * 需要对照的边界/推进条件：comm->proxyState.progressState.localPeers == NULL；peer == NULL；state->size == 0；state->refcount ==
 * 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sharedBuffersDestroy(struct ncclComm* comm, int localRank, int type) {
  if (comm->proxyState.progressState.localPeers == NULL) NCCLCHECK(ncclInternalError);
/* 中文逐项-变量 V4033：peer
 * peer（struct ncclProxyPeer；函数 sharedBuffersDestroy 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = comm->proxyState.progressState.localPeers[localRank]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyPeer* peer = comm->proxyState.progressState.localPeers[localRank];
  if (peer == NULL) NCCLCHECK(ncclInternalError;)
/* 中文逐项-变量 V4034：state
 * state（struct ncclProxySharedP2p；函数 sharedBuffersDestroy 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = type == 0 ? &peer->send : &peer->recv；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxySharedP2p* state = type == 0 ? &peer->send : &peer->recv;
  if (state->size == 0) NCCLCHECK(ncclInternalError);
  state->refcount--;
  if (state->refcount == 0) {
    if (state->cudaBuff) CUDACHECK(cudaFree(state->cudaBuff));
    if (state->hostBuff) NCCLCHECK(ncclCudaHostFree(state->hostBuff));
  }
  if (peer->send.refcount || peer->recv.refcount) return ncclSuccess;
  free(peer);
  comm->proxyState.progressState.localPeers[localRank] = NULL;
  for (/* 中文逐项-变量 V4035：r
 * r（int；函数 sharedBuffersDestroy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->localRanks; r++) {
    if (comm->proxyState.progressState.localPeers[r]) return ncclSuccess;
  }
  // All peers are freed, free array
  free(comm->proxyState.progressState.localPeers);
  comm->proxyState.progressState.localPeers = NULL;
  return ncclSuccess;
}

/* 中文内部逻辑 F1087：proxySharedInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int rank = comm->localRankToRank[connection->localRank];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int sameProcess = comm->peerInfo[rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersInit(comm, 1, connection->localRank, 0, sameProcess, nChannels, NULL, NULL, NULL, NULL));；失败时可能提前return，成功才继续；按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1087
 */
/* 中文逐项-函数 F1087：proxySharedInit
 * 用途与位置：在代理服务端提前初始化可容纳指定通道数的共享缓冲，避免首次数据推进时临时分配。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 nChannels（int nChannels）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；sharedBuffersInit。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t proxySharedInit(struct ncclProxyConnection* connection, struct ncclComm* comm, int nChannels) {
/* 中文逐项-变量 V4036：rank
 * rank（int；函数 proxySharedInit 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = comm->localRankToRank[connection->localRank]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = comm->localRankToRank[connection->localRank];
/* 中文逐项-变量 V4037：sameProcess
 * sameProcess（int；函数 proxySharedInit 内）：资源拥有者与使用者是否同进程，决定可复用裸指针还是必须导入共享映射。
 * 初值/来源 sameProcess = comm->peerInfo[rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 :
 * 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int sameProcess = comm->peerInfo[rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;
  NCCLCHECK(sharedBuffersInit(comm, 1, connection->localRank, 0, sameProcess, nChannels, NULL, NULL, NULL, NULL));
  return ncclSuccess;
}

/* 中文内部逻辑 F1088：sendProxySetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct setupReq* req = (struct setupReq*) reqBuff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(reqSize != sizeof(struct setupReq))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：struct sendResources* resources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&resources, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 7. 调用 / 计算 / 写入：connection->transportResources = resources;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：resources->rank = req->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：resources->localRank = req->localRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：resources->remoteRank = req->remoteRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：resources->netDev = req->netDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：resources->shared = connection->shared = req->shared;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：resources->useGdr = req->useGdr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 调用 / 计算 / 写入：resources->channelId = req->channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：resources->connIndex = req->connIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 准备局部状态：ncclNetProperties_t props;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 17. 调用 / 计算 / 写入：NCCLCHECK(ncclNetGetProperties(req->netDev, &props));；失败时可能提前return，成功才继续；读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 18. 调用 / 计算 / 写入：resources->maxRecvs = props.maxRecvs;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 条件判断：(respSize != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 22. 调用 / 计算 / 写入：*done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 23. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1088
 */
/* 中文逐项-函数 F1088：sendProxySetup
 * 用途与位置：在实际代理进程中准备对应方向网络设备与连接资源；接收方向通常还要 listen 生成句柄。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc；ncclNetGetProperties。
 * 需要对照的边界/推进条件：reqSize != sizeof(struct setupReq；respSize != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendProxySetup(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
/* 中文逐项-变量 V4038：req
 * req（struct setupReq；函数 sendProxySetup 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 初值/来源 req = (struct setupReq*) reqBuff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct setupReq* req = (struct setupReq*) reqBuff;
  if (reqSize != sizeof(struct setupReq)) return ncclInternalError;

/* 中文逐项-变量 V4039：resources
 * resources（struct sendResources；函数 sendProxySetup 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&resources, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct sendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  connection->transportResources = resources;

  resources->rank = req->rank;
  resources->localRank = req->localRank;
  resources->remoteRank = req->remoteRank;
  resources->netDev = req->netDev;
  resources->shared = connection->shared = req->shared;
  resources->useGdr = req->useGdr;
  resources->channelId = req->channelId;
  resources->connIndex = req->connIndex;
/* 中文逐项-变量 V4040：props
 * props（ncclNetProperties_t；函数 sendProxySetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclNetGetProperties(req->netDev, &props));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclNetProperties_t props;
  NCCLCHECK(ncclNetGetProperties(req->netDev, &props));
  resources->maxRecvs = props.maxRecvs;

  // We don't return any data
  if (respSize != 0) return ncclInternalError;
  *done = 1;
  return ncclSuccess;
}

/* 中文内部逻辑 F1089：recvProxySetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct setupReq* req = (struct setupReq*) reqBuff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(reqSize != sizeof(struct setupReq))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：struct recvResources* resources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&resources, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 7. 调用 / 计算 / 写入：connection->transportResources = resources;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：resources->rank = req->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：resources->localRank = req->localRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：resources->remoteRank = req->remoteRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：resources->netDev = req->netDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：resources->shared = connection->shared = req->shared;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：resources->useGdr = req->useGdr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 调用 / 计算 / 写入：resources->channelId = req->channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：resources->connIndex = req->connIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 准备局部状态：ncclNetProperties_t props;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 17. 调用 / 计算 / 写入：NCCLCHECK(ncclNetGetProperties(req->netDev, &props));；失败时可能提前return，成功才继续；读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 18. 调用 / 计算 / 写入：resources->maxRecvs = props.maxRecvs;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 条件判断：(respSize != sizeof(ncclNetHandle_t))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 22. 调用 / 计算 / 写入：NCCLCHECK(ncclNetListen(req->netDev, respBuff, &resources->netListenComm));；失败时可能提前return，成功才继续；创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 23. 调用 / 计算 / 写入：*done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 24. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1089
 */
/* 中文逐项-函数 F1089：recvProxySetup
 * 用途与位置：在实际代理进程中准备对应方向网络设备与连接资源；接收方向通常还要 listen 生成句柄。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc；ncclNetGetProperties；ncclNetListen。
 * 需要对照的边界/推进条件：reqSize != sizeof(struct setupReq；respSize != sizeof(ncclNetHandle_t。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvProxySetup(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
/* 中文逐项-变量 V4041：req
 * req（struct setupReq；函数 recvProxySetup 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 初值/来源 req = (struct setupReq*) reqBuff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct setupReq* req = (struct setupReq*) reqBuff;
  if (reqSize != sizeof(struct setupReq)) return ncclInternalError;

/* 中文逐项-变量 V4042：resources
 * resources（struct recvResources；函数 recvProxySetup 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&resources, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct recvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  connection->transportResources = resources;

  resources->rank = req->rank;
  resources->localRank = req->localRank;
  resources->remoteRank = req->remoteRank;
  resources->netDev = req->netDev;
  resources->shared = connection->shared = req->shared;
  resources->useGdr = req->useGdr;
  resources->channelId = req->channelId;
  resources->connIndex = req->connIndex;
/* 中文逐项-变量 V4043：props
 * props（ncclNetProperties_t；函数 recvProxySetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclNetGetProperties(req->netDev, &props));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclNetProperties_t props;
  NCCLCHECK(ncclNetGetProperties(req->netDev, &props));
  resources->maxRecvs = props.maxRecvs;

  if (respSize != sizeof(ncclNetHandle_t)) return ncclInternalError;
  NCCLCHECK(ncclNetListen(req->netDev, respBuff, &resources->netListenComm));
  *done = 1;
  return ncclSuccess;
}

/* 中文导读：
 * service 侧完成底层 sendComm 建立并分配/注册缓冲区，再把映射信息返回调用方。
 * shared 且 maxRecvs>1 且 NET_SHARED_COMMS 启用时，尝试按 netDev/remoteRank/channel 复用 sendComm。
 * 注册按协议缓冲区执行，而非按 qps[] 循环执行，所以增加单连接 QP 数不直接倍增这些 MR。
 */
/* 中文内部逻辑 F1090：sendProxyConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct sendResources* resources = (struct sendResources*)(connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(reqSize != sizeof(ncclNetHandle_t))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 条件判断：(resources->shared)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 准备局部状态：struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     8. 条件判断：(progressState->localPeers == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       9. 真分支：；仅条件为真执行以下子项。
 *         10. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     11. 准备局部状态：struct ncclProxyPeer** localPeers = progressState->localPeers;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     12. 条件判断：(localPeers[resources->localRank] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       13. 真分支：；仅条件为真执行以下子项。
 *         14. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(localPeers+resources->localRank, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     15. 调用 / 计算 / 写入：connection->proxyAppendPtr = localPeers[resources->localRank]->send.proxyAppend+resources->channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     16. 条件判断：(resources->maxRecvs > 1 && ncclParamNetSharedComms())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       17. 真分支：；仅条件为真执行以下子项。
 *         18. 条件判断：(progressState->netComms[resources->netDev] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           19. 真分支：；仅条件为真执行以下子项。
 *             20. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(progressState->netComms+resources->netDev, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         21. 准备局部状态：struct ncclSharedNetComms* comms = progressState->netComms[resources->netDev]+resources->remoteRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         22. 条件判断：(comms->sendComm[resources->channelId] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           23. 真分支：；仅条件为真执行以下子项。
 *             24. 调用 / 计算 / 写入：NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, comms->sendComm+resources->channelId));；失败时可能提前return，成功才继续；推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *         25. 调用 / 计算 / 写入：resources->netSendComm = comms->sendComm[resources->channelId];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         26. 条件判断：(comms->sendComm[resources->channelId])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           27. 真分支：；仅条件为真执行以下子项。
 *             28. 调用 / 计算 / 写入：comms->sendRefCount[resources->channelId]++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       29. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         30. 调用 / 计算 / 写入：NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, &resources->netSendComm));；失败时可能提前return，成功才继续；推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   31. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     32. 调用 / 计算 / 写入：NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, &resources->netSendComm));；失败时可能提前return，成功才继续；推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *     33. 调用 / 计算 / 写入：connection->proxyAppendPtr = &connection->proxyAppend;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 34. 条件判断：(resources->netSendComm == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   35. 真分支：；仅条件为真执行以下子项。
 *     36. 调用 / 计算 / 写入：*done = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     37. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 38. 调用 / 计算 / 写入：*done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 39. 准备局部状态：struct connectMap* map = &resources->map;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 40. 调用 / 计算 / 写入：map->sameProcess = comm->peerInfo[resources->rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 41. 调用 / 计算 / 写入：map->shared = resources->shared;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 42. 调用 / 计算 / 写入：CUDACHECK(cudaGetDevice(&map->cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 43. 条件判断：(resources->shared == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   44. 真分支：；仅条件为真执行以下子项。
 *     45. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       46. 调用 / 计算 / 写入：NCCL_NET_MAP_ADD_POINTER(map, 0, p!= NCCL_PROTO_LL && resources->useGdr, comm->buffSizes[p], buffs[p]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       47. 调用 / 计算 / 写入：resources->buffSizes[p] = comm->buffSizes[p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     48. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   49. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     50. 准备局部状态：int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     51. 准备局部状态：struct connectMapMem* mapMem = map->mems+bank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     52. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersInit( comm, resources->useGdr, resources->localRank, 0, map->sameProcess, comm->p2pnChannels, &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size, &mapMem->ipc));；失败时可能提前return，成功才继续；按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。
 *     53. 调用 / 计算 / 写入：resources->buffSizes[NCCL_PROTO_SIMPLE] = mapMem->size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     54. 调用 / 计算 / 写入：NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 55. 调用 / 计算 / 写入：NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 56. 调用 / 计算 / 写入：NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 57. 条件判断：(map->mems[NCCL_NET_MAP_DEVMEM].size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   58. 真分支：；仅条件为真执行以下子项。
 *     59. 条件判断：(resources->shared == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       60. 真分支：；仅条件为真执行以下子项。
 *         61. 条件判断：(!map->sameProcess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           62. 真分支：；仅条件为真执行以下子项。
 *             63. 调用 / 计算 / 写入：ALIGN_SIZE(map->mems[NCCL_NET_MAP_DEVMEM].size, CUDA_IPC_MIN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         64. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaCalloc(&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].size));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         65. 调用 / 计算 / 写入：map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     66. 条件判断：(!map->sameProcess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       67. 真分支：；仅条件为真执行以下子项。
 *         68. 调用 / 计算 / 写入：CUDACHECK(cudaIpcGetMemHandle(&map->mems[NCCL_NET_MAP_DEVMEM].ipc, map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 69. 条件判断：(map->sameProcess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   70. 真分支：；仅条件为真执行以下子项。
 *     71. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     72. 调用 / 计算 / 写入：map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   73. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     74. 调用 / 计算 / 写入：NCCLCHECK(netCreateShm(map->mems+NCCL_NET_MAP_HOSTMEM));；失败时可能提前return，成功才继续；创建带共享名字的主机内存映射，供不同进程的 NET/Proxy 协作访问。
 * 75. 条件判断：(ncclGdrCopy && map->sameProcess && ncclParamGdrCopySyncEnable())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   76. 真分支：；仅条件为真执行以下子项。
 *     77. 准备局部状态：uint64_t *cpuPtr, *gpuPtr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     78. 调用 / 计算 / 写入：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources->gdrDesc));；失败时可能提前return，成功才继续；分配显存并建立 GDRCopy 的 CPU 映射，返回 CUDA 地址、CPU 可写映射与释放所需描述符。
 *     79. 调用 / 计算 / 写入：resources->gdcSync = cpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     80. 准备局部状态：struct connectMapMem* gdcMem = map->mems+NCCL_NET_MAP_GDCMEM;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     81. 调用 / 计算 / 写入：gdcMem->cpuPtr = (char*)cpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     82. 调用 / 计算 / 写入：gdcMem->gpuPtr = (char*)gpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     83. 调用 / 计算 / 写入：gdcMem->size = sizeof(uint64_t);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 84. 调用 / 计算 / 写入：resources->sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 85. 调用 / 计算 / 写入：resources->recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 86. 调用 / 计算 / 写入：resources->sendMem->head = map->shared ? -NCCL_STEPS : 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 87. 循环：初始 int i=0;；继续条件 i<NCCL_STEPS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   88. 调用 / 计算 / 写入：resources->recvMem->sizesFifo[i] = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 89. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 90. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   91. 调用 / 计算 / 写入：resources->buffers[p] = NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   92. 条件判断：(resources->buffers[p])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     93. 真分支：；仅条件为真执行以下子项。
 *       94. 调用 / 计算 / 写入：NCCLCHECK(ncclNetRegMr(resources->netSendComm, resources->buffers[p], resources->buffSizes[p], NCCL_NET_MAP_DEV_MEM(map, buffs[p]) ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources->mhandles[p]));；失败时可能提前return，成功才继续；注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 95. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 96. 条件判断：(respSize != sizeof(struct connectMap))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   97. 真分支：；仅条件为真执行以下子项。
 *     98. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 99. 调用 / 计算 / 写入：memcpy(respBuff, map, sizeof(struct connectMap));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 100. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1090
 */
/* 中文逐项-函数 F1090：sendProxyConnect
 * 用途与位置：在代理侧异步完成后端 connect/accept、协议缓冲分配/共享/注册，并返回主线程需要的映射信息。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ALIGN_SIZE；CUDACHECK；NCCLCHECK；NCCL_NET_MAP_ADD_POINTER；NCCL_NET_MAP_DEV_MEM；NCCL_NET_MAP_GET_POINTER；cudaGetDevice；cudaIpcGetMemHandle；memcpy；ncclCalloc；ncclCudaCalloc；ncclCudaHostCalloc；ncclGdrCudaCalloc；ncclNetConnect；ncclNetRegMr；ncclParamGdrCopySyncEnable；ncclParamNetSharedComms；netCreateShm；更多调用保留在函数体。
 * 需要对照的边界/推进条件：reqSize != sizeof(ncclNetHandle_t；resources->shared；progressState->localPeers ==
 * NULL；localPeers[resources->localRank] == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendProxyConnect(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
/* 中文逐项-变量 V4044：resources
 * resources（struct sendResources；函数 sendProxyConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct sendResources*)(connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct sendResources* resources = (struct sendResources*)(connection->transportResources);
  if (reqSize != sizeof(ncclNetHandle_t)) return ncclInternalError;

  if (resources->shared) {
    // Shared buffers
/* 中文逐项-变量 V4045：progressState
 * progressState（struct ncclProxyProgressState；函数 sendProxyConnect 内）：communicator的代理总状态或数据推进子状态，分别组织服务控制和活跃请求。
 * 初值/来源 progressState = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;
    if (progressState->localPeers == NULL) {
      NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));
    }
/* 中文逐项-变量 V4046：localPeers
 * localPeers（struct ncclProxyPeer；函数 sendProxyConnect 内）：代理本地peer控制对象、控制socket及引导交换的地址表。
 * 初值/来源 localPeers = progressState->localPeers；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyPeer** localPeers = progressState->localPeers;
    if (localPeers[resources->localRank] == NULL) {
      NCCLCHECK(ncclCalloc(localPeers+resources->localRank, 1));
    }
    connection->proxyAppendPtr = localPeers[resources->localRank]->send.proxyAppend+resources->channelId;

    if (resources->maxRecvs > 1 && ncclParamNetSharedComms()) {
      // Connect or reuse connection for a netdev/remote rank.
      if (progressState->netComms[resources->netDev] == NULL) {
        NCCLCHECK(ncclCalloc(progressState->netComms+resources->netDev, comm->nRanks));
      }
/* 中文逐项-变量 V4047：comms
 * comms（struct ncclSharedNetComms；函数 sendProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 comms = progressState->netComms[resources->netDev]+resources->remoteRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclSharedNetComms* comms = progressState->netComms[resources->netDev]+resources->remoteRank;
      if (comms->sendComm[resources->channelId] == NULL) NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, comms->sendComm+resources->channelId));
      resources->netSendComm = comms->sendComm[resources->channelId];
      if (comms->sendComm[resources->channelId]) comms->sendRefCount[resources->channelId]++;
    } else {
      NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, &resources->netSendComm));
    }
  } else {
    // Connect to remote peer
    NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, &resources->netSendComm));
    connection->proxyAppendPtr = &connection->proxyAppend;
  }
  if (resources->netSendComm == NULL) {
    *done = 0;
    return ncclSuccess;
  }
  *done = 1;

  // Create structures
/* 中文逐项-变量 V4048：map
 * map（struct connectMap；函数 sendProxyConnect 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 map = &resources->map；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct connectMap* map = &resources->map;
  map->sameProcess =
    comm->peerInfo[resources->rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;
  map->shared = resources->shared;
  CUDACHECK(cudaGetDevice(&map->cudaDev));

  if (resources->shared == 0) { // Only allocate dedicated buffers for ring/tree, not for p2p
    for (/* 中文逐项-变量 V4049：p
 * p（int；函数 sendProxyConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
      NCCL_NET_MAP_ADD_POINTER(map, 0, p!= NCCL_PROTO_LL && resources->useGdr, comm->buffSizes[p], buffs[p]);
      resources->buffSizes[p] = comm->buffSizes[p];
    }
  } else {
    // Get shared buffers
/* 中文逐项-变量 V4050：bank
 * bank（int；函数 sendProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM :
 * NCCL_NET_MAP_SHARED_HOSTMEM；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;
/* 中文逐项-变量 V4051：mapMem
 * mapMem（struct connectMapMem；函数 sendProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mapMem = map->mems+bank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct connectMapMem* mapMem = map->mems+bank;
    NCCLCHECK(sharedBuffersInit(
          comm, resources->useGdr, resources->localRank, 0, map->sameProcess, comm->p2pnChannels,
          &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size, &mapMem->ipc));
    resources->buffSizes[NCCL_PROTO_SIMPLE] = mapMem->size;
    NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);
  }

  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);
  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);

  if (map->mems[NCCL_NET_MAP_DEVMEM].size) {
    if (resources->shared == 0) {
      if (!map->sameProcess) {
        ALIGN_SIZE(map->mems[NCCL_NET_MAP_DEVMEM].size, CUDA_IPC_MIN);
      }
      NCCLCHECK(ncclCudaCalloc(&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].size));
      map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr;
    }
    if (!map->sameProcess) {
      CUDACHECK(cudaIpcGetMemHandle(&map->mems[NCCL_NET_MAP_DEVMEM].ipc, map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr));
    }
  }
  if (map->sameProcess) {
    NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
    map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;
  } else {
    NCCLCHECK(netCreateShm(map->mems+NCCL_NET_MAP_HOSTMEM));
  }
  if (ncclGdrCopy && map->sameProcess && ncclParamGdrCopySyncEnable()) {
/* 中文逐项-变量 V4052：cpuPtr、gpuPtr
 * cpuPtr（uint64_t；函数 sendProxyConnect 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources->gdrDesc));。若由输出参数填入，必须先检查填充调用成功再读值。
 * gpuPtr（uint64_t；函数 sendProxyConnect 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources->gdrDesc));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    uint64_t *cpuPtr, *gpuPtr;
    NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources->gdrDesc));

    resources->gdcSync = cpuPtr;
/* 中文逐项-变量 V4053：gdcMem
 * gdcMem（struct connectMapMem；函数 sendProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdcMem = map->mems+NCCL_NET_MAP_GDCMEM；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct connectMapMem* gdcMem = map->mems+NCCL_NET_MAP_GDCMEM;
    gdcMem->cpuPtr = (char*)cpuPtr;
    gdcMem->gpuPtr = (char*)gpuPtr;
    gdcMem->size = sizeof(uint64_t); // sendMem->head
  }

  resources->sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);
  resources->recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);

  // Don't give credits yet in shared mode.
  resources->sendMem->head = map->shared ? -NCCL_STEPS : 0;
  for (/* 中文逐项-变量 V4054：i
 * i（int；函数 sendProxyConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_STEPS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_STEPS; i++) resources->recvMem->sizesFifo[i] = -1;

  for (/* 中文逐项-变量 V4055：p
 * p（int；函数 sendProxyConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    resources->buffers[p] = NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]);
    if (resources->buffers[p]) {
      NCCLCHECK(ncclNetRegMr(resources->netSendComm, resources->buffers[p], resources->buffSizes[p], NCCL_NET_MAP_DEV_MEM(map, buffs[p]) ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources->mhandles[p]));
    }
  }

  //NCCLCHECK(netDumpMap(map));
  if (respSize != sizeof(struct connectMap)) return ncclInternalError;
  memcpy(respBuff, map, sizeof(struct connectMap));
  return ncclSuccess;
}

/* 中文导读：
 * 完成接收方向底层 accept、共享/专用缓冲分配和协议 MR 注册。
 * 共享连接表按 netDev/proxyRank/channel 索引，条件不满足时为逻辑连接单独 accept。
 * closeListen 只关闭监听资源，不会把已经建立的 netRecvComm 一起销毁。
 */
/* 中文内部逻辑 F1091：recvProxyConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(reqSize != sizeof(int))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：struct recvResources* resources = (struct recvResources*)(connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：resources->proxyRank = *(int*)reqBuff;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 条件判断：(resources->shared)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 准备局部状态：struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 条件判断：(progressState->localPeers == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       10. 真分支：；仅条件为真执行以下子项。
 *         11. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     12. 准备局部状态：struct ncclProxyPeer** localPeers = progressState->localPeers;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     13. 条件判断：(localPeers[resources->localRank] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       14. 真分支：；仅条件为真执行以下子项。
 *         15. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(localPeers+resources->localRank, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     16. 调用 / 计算 / 写入：connection->proxyAppendPtr = localPeers[resources->localRank]->recv.proxyAppend+resources->channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     17. 条件判断：(resources->maxRecvs > 1 && ncclParamNetSharedComms())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       18. 真分支：；仅条件为真执行以下子项。
 *         19. 条件判断：(progressState->netComms[resources->netDev] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           20. 真分支：；仅条件为真执行以下子项。
 *             21. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(progressState->netComms+resources->netDev, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         22. 准备局部状态：struct ncclSharedNetComms* comms = progressState->netComms[resources->netDev]+resources->proxyRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         23. 条件判断：(comms->recvComm[resources->channelId] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           24. 真分支：；仅条件为真执行以下子项。
 *             25. 调用 / 计算 / 写入：NCCLCHECK(ncclNetAccept(resources->netListenComm, comms->recvComm+resources->channelId));；失败时可能提前return，成功才继续；推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *         26. 调用 / 计算 / 写入：resources->netRecvComm = comms->recvComm[resources->channelId];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         27. 条件判断：(comms->recvComm[resources->channelId])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           28. 真分支：；仅条件为真执行以下子项。
 *             29. 调用 / 计算 / 写入：comms->recvRefCount[resources->channelId]++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       30. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         31. 调用 / 计算 / 写入：NCCLCHECK(ncclNetAccept(resources->netListenComm, &resources->netRecvComm));；失败时可能提前return，成功才继续；推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   32. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     33. 调用 / 计算 / 写入：NCCLCHECK(ncclNetAccept(resources->netListenComm, &resources->netRecvComm));；失败时可能提前return，成功才继续；推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *     34. 调用 / 计算 / 写入：connection->proxyAppendPtr = &connection->proxyAppend;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 35. 条件判断：(resources->netRecvComm == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   36. 真分支：；仅条件为真执行以下子项。
 *     37. 调用 / 计算 / 写入：*done = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     38. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 39. 调用 / 计算 / 写入：*done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 40. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseListen(resources->netListenComm));；失败时可能提前return，成功才继续；关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 41. 准备局部状态：struct connectMap* map = &resources->map;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 42. 调用 / 计算 / 写入：map->sameProcess = comm->peerInfo[resources->rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 43. 条件判断：(map->sameProcess == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   44. 真分支：；仅条件为真执行以下子项。
 *     45. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 46. 调用 / 计算 / 写入：map->shared = resources->shared;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 47. 条件判断：(resources->shared == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   48. 真分支：；仅条件为真执行以下子项。
 *     49. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       50. 调用 / 计算 / 写入：NCCL_NET_MAP_ADD_POINTER(map, 0, resources->useGdr, comm->buffSizes[p], buffs[p]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       51. 调用 / 计算 / 写入：resources->buffSizes[p] = comm->buffSizes[p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     52. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   53. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     54. 准备局部状态：int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     55. 准备局部状态：struct connectMapMem* mapMem = map->mems+bank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     56. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersInit( comm, resources->useGdr, resources->localRank, 1, 1, comm->p2pnChannels, &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size, NULL));；失败时可能提前return，成功才继续；按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。
 *     57. 调用 / 计算 / 写入：resources->buffSizes[NCCL_PROTO_SIMPLE] = mapMem->size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     58. 调用 / 计算 / 写入：NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 59. 调用 / 计算 / 写入：NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 60. 调用 / 计算 / 写入：NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 61. 条件判断：(map->mems[NCCL_NET_MAP_DEVMEM].size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   62. 真分支：；仅条件为真执行以下子项。
 *     63. 条件判断：(resources->shared == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       64. 真分支：；仅条件为真执行以下子项。
 *         65. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaCalloc(&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].size));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         66. 调用 / 计算 / 写入：map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 67. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 68. 调用 / 计算 / 写入：map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 69. 条件判断：(ncclGdrCopy && map->sameProcess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   70. 真分支：；仅条件为真执行以下子项。
 *     71. 准备局部状态：uint64_t *cpuPtr, *gpuPtr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     72. 调用 / 计算 / 写入：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources->gdrDesc));；失败时可能提前return，成功才继续；分配显存并建立 GDRCopy 的 CPU 映射，返回 CUDA 地址、CPU 可写映射与释放所需描述符。
 *     73. 条件判断：(ncclParamGdrCopySyncEnable())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       74. 真分支：；仅条件为真执行以下子项。
 *         75. 调用 / 计算 / 写入：resources->gdcSync = cpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         76. 准备局部状态：struct connectMapMem* gdcMem = map->mems+NCCL_NET_MAP_GDCMEM;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         77. 调用 / 计算 / 写入：gdcMem->cpuPtr = (char*)cpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         78. 调用 / 计算 / 写入：gdcMem->gpuPtr = (char*)gpuPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         79. 调用 / 计算 / 写入：gdcMem->size = sizeof(uint64_t);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     80. 条件判断：(ncclParamGdrCopyFlushEnable())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       81. 真分支：；仅条件为真执行以下子项。
 *         82. 调用 / 计算 / 写入：resources->gdcFlush = cpuPtr + 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 83. 调用 / 计算 / 写入：resources->sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 84. 调用 / 计算 / 写入：resources->recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 85. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   86. 调用 / 计算 / 写入：resources->buffers[p] = NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   87. 条件判断：(resources->buffers[p])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     88. 真分支：；仅条件为真执行以下子项。
 *       89. 调用 / 计算 / 写入：NCCLCHECK(ncclNetRegMr(resources->netRecvComm, resources->buffers[p], resources->buffSizes[p], NCCL_NET_MAP_DEV_MEM(map, buffs[p]) ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources->mhandles[p]));；失败时可能提前return，成功才继续；注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 90. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 91. 条件判断：(respSize != sizeof(struct connectMap))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   92. 真分支：；仅条件为真执行以下子项。
 *     93. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 94. 调用 / 计算 / 写入：memcpy(respBuff, map, sizeof(struct connectMap));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 95. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1091
 */
/* 中文逐项-函数 F1091：recvProxyConnect
 * 用途与位置：在代理侧异步完成后端 connect/accept、协议缓冲分配/共享/注册，并返回主线程需要的映射信息。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCL_NET_MAP_ADD_POINTER；NCCL_NET_MAP_DEV_MEM；NCCL_NET_MAP_GET_POINTER；memcpy；ncclCalloc；ncclCudaCalloc；ncclCudaHostCalloc；ncclGdrCudaCalloc；ncclNetAccept；ncclNetCloseListen；ncclNetRegMr；ncclParamGdrCopyFlushEnable；ncclParamGdrCopySyncEnable；ncclParamNetSharedComms；sharedBuffersInit。
 * 需要对照的边界/推进条件：reqSize != sizeof(int；resources->shared；progressState->localPeers ==
 * NULL；localPeers[resources->localRank] == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvProxyConnect(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
  if (reqSize != sizeof(int)) return ncclInternalError;
/* 中文逐项-变量 V4056：resources
 * resources（struct recvResources；函数 recvProxyConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*)(connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct recvResources* resources = (struct recvResources*)(connection->transportResources);
  resources->proxyRank = *(int*)reqBuff;

  // Finish connection establishment from remote peer
  if (resources->shared) {
    // Shared buffers
/* 中文逐项-变量 V4057：progressState
 * progressState（struct ncclProxyProgressState；函数 recvProxyConnect 内）：communicator的代理总状态或数据推进子状态，分别组织服务控制和活跃请求。
 * 初值/来源 progressState = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;
    if (progressState->localPeers == NULL) {
      NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));
    }
/* 中文逐项-变量 V4058：localPeers
 * localPeers（struct ncclProxyPeer；函数 recvProxyConnect 内）：代理本地peer控制对象、控制socket及引导交换的地址表。
 * 初值/来源 localPeers = progressState->localPeers；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyPeer** localPeers = progressState->localPeers;
    if (localPeers[resources->localRank] == NULL) {
      NCCLCHECK(ncclCalloc(localPeers+resources->localRank, 1));
    }
    connection->proxyAppendPtr = localPeers[resources->localRank]->recv.proxyAppend+resources->channelId;

    if (resources->maxRecvs > 1 && ncclParamNetSharedComms()) {
      // Connect or reuse connection for a netdev/remote rank.
      if (progressState->netComms[resources->netDev] == NULL) {
        NCCLCHECK(ncclCalloc(progressState->netComms+resources->netDev, comm->nRanks));
      }
/* 中文逐项-变量 V4059：comms
 * comms（struct ncclSharedNetComms；函数 recvProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 comms = progressState->netComms[resources->netDev]+resources->proxyRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclSharedNetComms* comms = progressState->netComms[resources->netDev]+resources->proxyRank;
      if (comms->recvComm[resources->channelId] == NULL) NCCLCHECK(ncclNetAccept(resources->netListenComm, comms->recvComm+resources->channelId));
      resources->netRecvComm = comms->recvComm[resources->channelId];
      if (comms->recvComm[resources->channelId]) comms->recvRefCount[resources->channelId]++;
    } else {
      NCCLCHECK(ncclNetAccept(resources->netListenComm, &resources->netRecvComm));
    }
  } else {
    // Connect to remote peer
    NCCLCHECK(ncclNetAccept(resources->netListenComm, &resources->netRecvComm));
    connection->proxyAppendPtr = &connection->proxyAppend;
  }
  if (resources->netRecvComm == NULL) {
    *done = 0;
    return ncclSuccess;
  }
  *done = 1;
  NCCLCHECK(ncclNetCloseListen(resources->netListenComm));

  // Create structures
/* 中文逐项-变量 V4060：map
 * map（struct connectMap；函数 recvProxyConnect 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 map = &resources->map；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct connectMap* map = &resources->map;
  map->sameProcess =
    comm->peerInfo[resources->rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;
  if (map->sameProcess == 0) return ncclInternalError; // We don't support remote proxy for recv
  map->shared = resources->shared;

  if (resources->shared == 0) { // Only allocate dedicated buffers for ring/tree, not for p2p
    for (/* 中文逐项-变量 V4061：p
 * p（int；函数 recvProxyConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
      NCCL_NET_MAP_ADD_POINTER(map, 0, resources->useGdr, comm->buffSizes[p], buffs[p]);
      resources->buffSizes[p] = comm->buffSizes[p];
    }
  } else {
    // Get shared buffers
/* 中文逐项-变量 V4062：bank
 * bank（int；函数 recvProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM :
 * NCCL_NET_MAP_SHARED_HOSTMEM；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;
/* 中文逐项-变量 V4063：mapMem
 * mapMem（struct connectMapMem；函数 recvProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mapMem = map->mems+bank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct connectMapMem* mapMem = map->mems+bank;
    NCCLCHECK(sharedBuffersInit(
          comm, resources->useGdr, resources->localRank, 1, 1, comm->p2pnChannels,
          &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size, NULL));
    resources->buffSizes[NCCL_PROTO_SIMPLE] = mapMem->size;
    NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);
  }

  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);
  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);

  if (map->mems[NCCL_NET_MAP_DEVMEM].size) {
    if (resources->shared == 0) {
      NCCLCHECK(ncclCudaCalloc(&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].size));
      map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr;
    }
  }
  NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
  map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;
  if (ncclGdrCopy && map->sameProcess) {
/* 中文逐项-变量 V4064：cpuPtr、gpuPtr
 * cpuPtr（uint64_t；函数 recvProxyConnect 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources->gdrDesc));。若由输出参数填入，必须先检查填充调用成功再读值。
 * gpuPtr（uint64_t；函数 recvProxyConnect 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources->gdrDesc));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    uint64_t *cpuPtr, *gpuPtr;
    NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources->gdrDesc));

    if (ncclParamGdrCopySyncEnable()) {
      resources->gdcSync = cpuPtr;
/* 中文逐项-变量 V4065：gdcMem
 * gdcMem（struct connectMapMem；函数 recvProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdcMem = map->mems+NCCL_NET_MAP_GDCMEM；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct connectMapMem* gdcMem = map->mems+NCCL_NET_MAP_GDCMEM;
      gdcMem->cpuPtr = (char*)cpuPtr;
      gdcMem->gpuPtr = (char*)gpuPtr;
      gdcMem->size = sizeof(uint64_t);
    }
    if (ncclParamGdrCopyFlushEnable()) resources->gdcFlush = cpuPtr + 1;
  }

  resources->sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);
  resources->recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);
  for (/* 中文逐项-变量 V4066：p
 * p（int；函数 recvProxyConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    resources->buffers[p] = NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]);
    if (resources->buffers[p]) {
      NCCLCHECK(ncclNetRegMr(resources->netRecvComm, resources->buffers[p], resources->buffSizes[p], NCCL_NET_MAP_DEV_MEM(map, buffs[p]) ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources->mhandles[p]));
    }
  }

  //NCCLCHECK(netDumpMap(map));
  if (respSize != sizeof(struct connectMap)) return ncclInternalError;
  memcpy(respBuff, map, sizeof(struct connectMap));
  return ncclSuccess;
}

/* 中文内部逻辑 F1092：sendProxyFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct sendResources* resources = (struct sendResources*)(connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(resources == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersDestroy(comm, connection->localRank, 0));；失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。
 *     5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 条件判断：(resources->buffers[p])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     8. 真分支：；仅条件为真执行以下子项。
 *       9. 调用 / 计算 / 写入：NCCLCHECK(ncclNetDeregMr(resources->netSendComm, resources->mhandles[p]));；失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 准备局部状态：struct connectMapMem* mems = resources->map.mems;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 条件判断：(resources->map.sameProcess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));；失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。
 *   15. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     16. 调用 / 计算 / 写入：NCCLCHECK(ncclShmClose(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, NULL, mems[NCCL_NET_MAP_HOSTMEM].size));；失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。
 * 17. 调用 / 计算 / 写入：CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 18. 条件判断：(mems[NCCL_NET_MAP_GDCMEM].cpuPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   19. 真分支：；仅条件为真执行以下子项。
 *     20. 调用 / 计算 / 写入：NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));；失败时可能提前return，成功才继续；按映射、pin、显存分配的依赖顺序撤销 GDRCopy 资源。
 * 21. 条件判断：(resources->shared)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   22. 真分支：；仅条件为真执行以下子项。
 *     23. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersDestroy(comm, resources->localRank, 0));；失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。
 *     24. 条件判断：(resources->maxRecvs > 1 && ncclParamNetSharedComms())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       25. 真分支：；仅条件为真执行以下子项。
 *         26. 准备局部状态：struct ncclSharedNetComms* comms = comm->proxyState.progressState.netComms[resources->netDev]+resources->remoteRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         27. 调用 / 计算 / 写入：comms->sendRefCount[resources->channelId]--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         28. 条件判断：(comms->sendRefCount[resources->channelId] == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           29. 真分支：；仅条件为真执行以下子项。
 *             30. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseSend(comms->sendComm[resources->channelId]));；失败时可能提前return，成功才继续；关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *       31. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         32. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseSend(resources->netSendComm));；失败时可能提前return，成功才继续；关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   33. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     34. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseSend(resources->netSendComm));；失败时可能提前return，成功才继续；关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 35. 调用 / 计算 / 写入：free(resources);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 36. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1092
 */
/* 中文逐项-函数 F1092：sendProxyFree
 * 用途与位置：在拥有网络对象的代理侧注销内存、释放映射并按引用计数关闭 backend connection。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaFree；free；ncclCudaHostFree；ncclGdrCudaFree；ncclNetCloseSend；ncclNetDeregMr；ncclParamNetSharedComms；ncclShmClose；sharedBuffersDestroy。
 * 需要对照的边界/推进条件：resources == NULL；p<NCCL_NUM_PROTOCOLS；resources->buffers[p]；resources->map.sameProcess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendProxyFree(struct ncclProxyConnection* connection, struct ncclComm* comm) {
/* 中文逐项-变量 V4067：resources
 * resources（struct sendResources；函数 sendProxyFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct sendResources*)(connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct sendResources* resources = (struct sendResources*)(connection->transportResources);
  if (resources == NULL) { // NVB Preconnect
    NCCLCHECK(sharedBuffersDestroy(comm, connection->localRank, 0));
    return ncclSuccess;
  }
  for (/* 中文逐项-变量 V4068：p
 * p（int；函数 sendProxyFree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (resources->buffers[p]) {
      NCCLCHECK(ncclNetDeregMr(resources->netSendComm, resources->mhandles[p]));
    }
  }
/* 中文逐项-变量 V4069：mems
 * mems（struct connectMapMem；函数 sendProxyFree 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 mems = resources->map.mems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMapMem* mems = resources->map.mems;
  if (resources->map.sameProcess) {
    NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));
  } else {
    NCCLCHECK(ncclShmClose(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, NULL, mems[NCCL_NET_MAP_HOSTMEM].size));
  }
  CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));
  if (mems[NCCL_NET_MAP_GDCMEM].cpuPtr) NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));
  if (resources->shared) {
    NCCLCHECK(sharedBuffersDestroy(comm, resources->localRank, 0));
    if (resources->maxRecvs > 1 && ncclParamNetSharedComms()) {
/* 中文逐项-变量 V4070：comms
 * comms（struct ncclSharedNetComms；函数 sendProxyFree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 comms =
 * comm->proxyState.progressState.netComms[resources->netDev]+resources->remoteRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclSharedNetComms* comms = comm->proxyState.progressState.netComms[resources->netDev]+resources->remoteRank;
      comms->sendRefCount[resources->channelId]--;
      if (comms->sendRefCount[resources->channelId] == 0) NCCLCHECK(ncclNetCloseSend(comms->sendComm[resources->channelId]));
    } else {
      NCCLCHECK(ncclNetCloseSend(resources->netSendComm));
    }
  } else {
    NCCLCHECK(ncclNetCloseSend(resources->netSendComm));
  }
  free(resources);
  return ncclSuccess;
}

/* 中文内部逻辑 F1093：recvProxyFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct recvResources* resources = (struct recvResources*)(connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(resources == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersDestroy(comm, connection->localRank, 1));；失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。
 *     5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 条件判断：(resources->buffers[p])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     8. 真分支：；仅条件为真执行以下子项。
 *       9. 调用 / 计算 / 写入：NCCLCHECK(ncclNetDeregMr(resources->netRecvComm, resources->mhandles[p]));；失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 准备局部状态：struct connectMapMem* mems = resources->map.mems;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));；失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。
 * 13. 调用 / 计算 / 写入：CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 14. 条件判断：(mems[NCCL_NET_MAP_GDCMEM].cpuPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 调用 / 计算 / 写入：NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));；失败时可能提前return，成功才继续；按映射、pin、显存分配的依赖顺序撤销 GDRCopy 资源。
 * 17. 条件判断：(resources->shared)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   18. 真分支：；仅条件为真执行以下子项。
 *     19. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersDestroy(comm, resources->localRank, 1));；失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。
 *     20. 条件判断：(resources->maxRecvs > 1 && ncclParamNetSharedComms())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       21. 真分支：；仅条件为真执行以下子项。
 *         22. 准备局部状态：struct ncclSharedNetComms* comms = comm->proxyState.progressState.netComms[resources->netDev]+resources->proxyRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         23. 调用 / 计算 / 写入：comms->recvRefCount[resources->channelId]--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         24. 条件判断：(comms->recvRefCount[resources->channelId] == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           25. 真分支：；仅条件为真执行以下子项。
 *             26. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseRecv(comms->recvComm[resources->channelId]));；失败时可能提前return，成功才继续；关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *       27. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         28. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseRecv(resources->netRecvComm));；失败时可能提前return，成功才继续；关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   29. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     30. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseRecv(resources->netRecvComm));；失败时可能提前return，成功才继续；关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 31. 调用 / 计算 / 写入：free(resources);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 32. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1093
 */
/* 中文逐项-函数 F1093：recvProxyFree
 * 用途与位置：在拥有网络对象的代理侧注销内存、释放映射并按引用计数关闭 backend connection。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaFree；free；ncclCudaHostFree；ncclGdrCudaFree；ncclNetCloseRecv；ncclNetDeregMr；ncclParamNetSharedComms；sharedBuffersDestroy。
 * 需要对照的边界/推进条件：resources ==
 * NULL；p<NCCL_NUM_PROTOCOLS；resources->buffers[p]；mems[NCCL_NET_MAP_GDCMEM].cpuPtr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvProxyFree(struct ncclProxyConnection* connection, struct ncclComm* comm) {
/* 中文逐项-变量 V4071：resources
 * resources（struct recvResources；函数 recvProxyFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*)(connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct recvResources* resources = (struct recvResources*)(connection->transportResources);
  if (resources == NULL) { // NVB Preconnect
    NCCLCHECK(sharedBuffersDestroy(comm, connection->localRank, 1));
    return ncclSuccess;
  }
  for (/* 中文逐项-变量 V4072：p
 * p（int；函数 recvProxyFree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (resources->buffers[p]) {
      NCCLCHECK(ncclNetDeregMr(resources->netRecvComm, resources->mhandles[p]));
    }
  }
/* 中文逐项-变量 V4073：mems
 * mems（struct connectMapMem；函数 recvProxyFree 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 mems = resources->map.mems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMapMem* mems = resources->map.mems;
  NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));
  CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));
  if (mems[NCCL_NET_MAP_GDCMEM].cpuPtr) NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));
  if (resources->shared) {
    NCCLCHECK(sharedBuffersDestroy(comm, resources->localRank, 1));
    if (resources->maxRecvs > 1 && ncclParamNetSharedComms()) {
/* 中文逐项-变量 V4074：comms
 * comms（struct ncclSharedNetComms；函数 recvProxyFree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 comms =
 * comm->proxyState.progressState.netComms[resources->netDev]+resources->proxyRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclSharedNetComms* comms = comm->proxyState.progressState.netComms[resources->netDev]+resources->proxyRank;
      comms->recvRefCount[resources->channelId]--;
      if (comms->recvRefCount[resources->channelId] == 0) NCCLCHECK(ncclNetCloseRecv(comms->recvComm[resources->channelId]));
    } else {
      NCCLCHECK(ncclNetCloseRecv(resources->netRecvComm));
    }
  } else {
    NCCLCHECK(ncclNetCloseRecv(resources->netRecvComm));
  }
  free(resources);
  return ncclSuccess;
}

static_assert(NCCL_STEPS <= NCCL_NET_MAX_REQUESTS, "Not enough net requests to cover for steps");

/* 中文导读：
 * 发送状态机：给 GPU 提供可用槽位/偏移 → 等待 GPU 发布大小和就绪状态 → isend → test → 回收槽位。
 * LL/LL128 的就绪判断有协议专门分支；不能只看 tail 就认定所有协议数据均已可发送。
 * GDR 模式下 Proxy 提交的是指向 GPU 内存的网络请求，不要求 CPU 把 payload 拷入系统内存。
 */
/* 中文内部逻辑 F1094：sendProxyProgress
 * 精读：本文件NET发送Ready阶段只初始化本任务base和相对计数，后续回调从Progress继续，不应每轮重置。
 * 精读：先提供可写槽，再等GPU sizes/tail/flag就绪；Isend给出有效请求后transmitted前进。
 * 精读：Test完成才能推进done并释放head；同一连接和MR继续保留给后续任务。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(args->state == ncclProxyOpReady)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 循环：初始 int s=0;；继续条件 s<args->nsubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       4. 准备局部状态：struct ncclProxySubArgs* sub = args->subs+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       5. 准备局部状态：struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       6. 调用 / 计算 / 写入：sub->base = ROUNDUP(resources->step, args->chunkSteps);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *       7. 调用 / 计算 / 写入：sub->posted = sub->transmitted = sub->done = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *       8. 循环：初始 uint64_t step=0;；继续条件 step<sub->nsteps；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         9. 调用 / 计算 / 写入：ncclProfilingRecord(args, s, step, ncclProxyProfileBegin);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *       10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     12. 调用 / 计算 / 写入：args->state = ncclProxyOpProgress;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：args->idle = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 条件判断：(args->state == ncclProxyOpProgress)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 准备局部状态：int p = args->protocol;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     17. 准备局部状态：int maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args->nsubs);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     18. 循环：初始 int s=0;；继续条件 s<args->nsubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       19. 准备局部状态：struct ncclProxySubArgs* sub = args->subs+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       20. 条件判断：(sub->done == sub->nsteps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         21. 真分支：；仅条件为真执行以下子项。
 *           22. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       23. 准备局部状态：struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       24. 准备局部状态：void* mhandle = resources->mhandles[p];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       25. 准备局部状态：int stepSize = resources->buffSizes[p] / NCCL_STEPS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       26. 准备局部状态：char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       27. 准备局部状态：int buffSize = stepSize*args->sliceSteps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       28. 条件判断：(sub->nbytes < buffSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         29. 真分支：；仅条件为真执行以下子项。
 *           30. 调用 / 计算 / 写入：buffSize = sub->nbytes;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       31. 条件判断：(sub->posted < sub->nsteps && sub->posted < sub->done + maxDepth)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         32. 真分支：；仅条件为真执行以下子项。
 *           33. 准备局部状态：int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           34. 条件判断：(resources->shared)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             35. 真分支：；仅条件为真执行以下子项。
 *               36. 准备局部状态：int sharedBuffSlot = sub->posted%maxDepth;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *               37. 准备局部状态：int offset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               38. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersGet(comm, sub->channelId, sharedBuffSlot*args->nsubs+s, &offset));；失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。
 *               39. 调用 / 计算 / 写入：resources->recvMem->offsFifo[buffSlot] = offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               40. 调用 / 计算 / 写入：__sync_synchronize();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *               41. 准备局部状态：volatile uint64_t* sendHead = resources->gdcSync ? resources->gdcSync : &resources->sendMem->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *               42. 调用 / 计算 / 写入：sub->posted += args->sliceSteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *               43. 调用 / 计算 / 写入：*sendHead = sub->base + sub->posted - NCCL_STEPS;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *               44. 条件判断：(resources->gdcSync)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 45. 真分支：；仅条件为真执行以下子项。
 *                   46. 调用 / 计算 / 写入：wc_store_fence();；按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。
 *             47. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *               48. 调用 / 计算 / 写入：sub->posted += args->sliceSteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           49. 循环：初始 uint64_t step=sub->posted-args->sliceSteps;；继续条件 step<sub->posted；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *             50. 调用 / 计算 / 写入：ncclProfilingRecord(args, s, step, ncclProxyProfileSendGPUWait);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *           51. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           52. 调用 / 计算 / 写入：args->idle = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           53. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       54. 条件判断：(sub->transmitted < sub->posted && sub->transmitted < sub->done + NCCL_STEPS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         55. 真分支：；仅条件为真执行以下子项。
 *           56. 准备局部状态：int buffSlot = (sub->base+sub->transmitted)%NCCL_STEPS;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           57. 准备局部状态：volatile int* sizesFifo = resources->recvMem->sizesFifo;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           58. 准备局部状态：volatile uint64_t* recvTail = &resources->recvMem->tail;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           59. 条件判断：(sizesFifo[buffSlot] != -1 && ((*recvTail > (sub->base+sub->transmitted)) || p == NCCL_PROTO_LL))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             60. 真分支：；仅条件为真执行以下子项。
 *               61. 准备局部状态：int size = sizesFifo[buffSlot];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               62. 准备局部状态：char* buff = resources->shared ? localBuff+resources->recvMem->offsFifo[buffSlot] : localBuff+buffSlot*stepSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               63. 准备局部状态：int ready = 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               64. 条件判断：(p == NCCL_PROTO_LL128)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 65. 真分支：；仅条件为真执行以下子项。
 *                   66. 调用 / 计算 / 写入：ready = resources->useGdr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   67. 条件判断：(!ready)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     68. 真分支：；仅条件为真执行以下子项。
 *                       69. 准备局部状态：uint64_t flag = sub->base+sub->transmitted+1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                       70. 准备局部状态：int nFifoLines = DIVUP(sizesFifo[buffSlot], sizeof(uint64_t)*NCCL_LL128_LINEELEMS);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       71. 准备局部状态：volatile uint64_t* lines = (volatile uint64_t*)buff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       72. 调用 / 计算 / 写入：ready = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                       73. 循环：初始 int i=0;；继续条件 i<nFifoLines；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                         74. 条件判断：(lines[i*NCCL_LL128_LINEELEMS+NCCL_LL128_DATAELEMS] != flag)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                           75. 真分支：；仅条件为真执行以下子项。
 *                             76. 调用 / 计算 / 写入：ready = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                             77. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *                       78. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *                 79. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   80. 条件判断：(p == NCCL_PROTO_LL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     81. 真分支：；仅条件为真执行以下子项。
 *                       82. 准备局部状态：uint32_t flag = NCCL_LL_FLAG(sub->base+sub->transmitted+1);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                       83. 准备局部状态：int nFifoLines = DIVUP(size, sizeof(union ncclLLFifoLine));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       84. 准备局部状态：union ncclLLFifoLine* lines = (union ncclLLFifoLine*)buff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       85. 循环：初始 int i=0;；继续条件 i<nFifoLines；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                         86. 准备局部状态：volatile uint32_t *f1 = &lines[i].flag1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                         87. 准备局部状态：volatile uint32_t *f2 = &lines[i].flag2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                         88. 条件判断：(f1[0] != flag || f2[0] != flag)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                           89. 真分支：；仅条件为真执行以下子项。
 *                             90. 调用 / 计算 / 写入：ready = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                             91. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *                       92. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               93. 条件判断：(ready)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 94. 真分支：；仅条件为真执行以下子项。
 *                   95. 调用 / 计算 / 写入：NCCLCHECK(ncclNetIsend(resources->netSendComm, buff, size, resources->rank, mhandle, sub->requests+buffSlot));；失败时可能提前return，成功才继续；提交异步发送请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *                   96. 条件判断：(sub->requests[buffSlot] != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     97. 真分支：；仅条件为真执行以下子项。
 *                       98. 调用 / 计算 / 写入：TRACE(NCCL_NET, "sendProxy [%ld/%d] Isend posted, req %p", sub->transmitted, buffSlot, sub->requests[buffSlot]);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                       99. 调用 / 计算 / 写入：sizesFifo[buffSlot] = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                       100. 调用 / 计算 / 写入：__sync_synchronize();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                       101. 调用 / 计算 / 写入：sub->transmitted += args->sliceSteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                       102. 循环：初始 uint64_t step=sub->transmitted-args->sliceSteps;；继续条件 step<sub->transmitted；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                         103. 调用 / 计算 / 写入：ncclProfilingRecord(args, s, step, ncclProxyProfileSendWait);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *                       104. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *                       105. 调用 / 计算 / 写入：args->idle = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                       106. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       107. 条件判断：(sub->done < sub->transmitted)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         108. 真分支：；仅条件为真执行以下子项。
 *           109. 准备局部状态：int done;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           110. 准备局部状态：int buffSlot = (sub->base+sub->done)%NCCL_STEPS;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           111. 调用 / 计算 / 写入：NCCLCHECK(ncclNetTest(sub->requests[buffSlot], &done, NULL));；失败时可能提前return，成功才继续；查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *           112. 条件判断：(done)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             113. 真分支：；仅条件为真执行以下子项。
 *               114. 调用 / 计算 / 写入：TRACE(NCCL_NET, "sendProxy [%ld/%d] request %p done", sub->done, buffSlot, sub->requests[buffSlot]);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *               115. 调用 / 计算 / 写入：sub->done += args->sliceSteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *               116. 循环：初始 uint64_t step=sub->done-args->sliceSteps;；继续条件 step<sub->done；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                 117. 调用 / 计算 / 写入：ncclProfilingRecord(args, s, step, ncclProxyProfileEnd);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *               118. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               119. 条件判断：(resources->shared == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 120. 真分支：；仅条件为真执行以下子项。
 *                   121. 准备局部状态：volatile uint64_t* sendHead = resources->gdcSync ? resources->gdcSync : &resources->sendMem->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                   122. 调用 / 计算 / 写入：*sendHead = sub->base + sub->done;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                   123. 条件判断：(resources->gdcSync)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     124. 真分支：；仅条件为真执行以下子项。
 *                       125. 调用 / 计算 / 写入：wc_store_fence();；按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。
 *               126. 调用 / 计算 / 写入：args->idle = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               127. 条件判断：(sub->done == sub->nsteps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 128. 真分支：；仅条件为真执行以下子项。
 *                   129. 调用 / 计算 / 写入：resources->step = sub->base + sub->nsteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                   130. 调用 / 计算 / 写入：args->done++;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     131. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     132. 条件判断：(args->done == args->nsubs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       133. 真分支：；仅条件为真执行以下子项。
 *         134. 调用 / 计算 / 写入：args->state = ncclProxyOpNone;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 135. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1094
 */
/* 中文逐项-函数 F1094：sendProxyProgress
 * 用途与位置：对应后端发送状态机：准备槽位、等 GPU 数据就绪、提交网络请求、检测完成后回收；CollNet 分支提交网络归约。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 args（struct ncclProxyArgs* args）：代理聚合运行态，保存各连接子任务的posted/received/transmitted/done进度和回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：DIVUP；NCCLCHECK；NCCL_LL_FLAG；NCCL_NET_MAP_GET_POINTER；ROUNDUP；__sync_synchronize；ncclNetIsend；ncclNetTest；ncclProfilingRecord；sharedBuffersGet；std::min；wc_store_fence。
 * 需要对照的边界/推进条件：args->state == ncclProxyOpReady；s<args->nsubs；step<sub->nsteps；args->state ==
 * ncclProxyOpProgress。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendProxyProgress(struct ncclComm* comm, struct ncclProxyArgs* args) {
  if (args->state == ncclProxyOpReady) {
    for (/* 中文逐项-变量 V4075：s
 * s（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s++) {
/* 中文逐项-变量 V4076：sub
 * sub（struct ncclProxySubArgs；函数 sendProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* sub = args->subs+s;
/* 中文逐项-变量 V4077：resources
 * resources（struct sendResources；函数 sendProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct sendResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);
      // Round to next multiple of sliceSteps
      sub->base = ROUNDUP(resources->step, args->chunkSteps);
      sub->posted = sub->transmitted = sub->done = 0;
      for (/* 中文逐项-变量 V4078：step
 * step（uint64_t；函数 sendProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：step<sub->nsteps；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=0; step<sub->nsteps; step++) ncclProfilingRecord(args, s, step, ncclProxyProfileBegin);
    }
    args->state = ncclProxyOpProgress;
  }
  args->idle = 1;
  if (args->state == ncclProxyOpProgress) {
/* 中文逐项-变量 V4079：p
 * p（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = args->protocol；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int p = args->protocol;
/* 中文逐项-变量 V4080：maxDepth
 * maxDepth（int；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args->nsubs)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
    int maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args->nsubs);
    for (/* 中文逐项-变量 V4081：s
 * s（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s++) {
/* 中文逐项-变量 V4082：sub
 * sub（struct ncclProxySubArgs；函数 sendProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* sub = args->subs+s;
      if (sub->done == sub->nsteps) continue;
/* 中文逐项-变量 V4083：resources
 * resources（struct sendResources；函数 sendProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct sendResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);
/* 中文逐项-变量 V4084：mhandle
 * mhandle（void；函数 sendProxyProgress 内）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 初值/来源 mhandle = resources->mhandles[p]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void* mhandle = resources->mhandles[p];
/* 中文逐项-变量 V4085：stepSize
 * stepSize（int；函数 sendProxyProgress 内）：一 FIFO step 的容量；设备 T 指针路径以元素计，主机缓冲路径以字节计。
 * 初值/来源 stepSize = resources->buffSizes[p] / NCCL_STEPS；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int stepSize = resources->buffSizes[p] / NCCL_STEPS;
/* 中文逐项-变量 V4086：localBuff
 * localBuff（char；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p])；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);
/* 中文逐项-变量 V4087：buffSize
 * buffSize（int；函数 sendProxyProgress 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 buffSize = stepSize*args->sliceSteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int buffSize = stepSize*args->sliceSteps;
      if (sub->nbytes < buffSize) buffSize = sub->nbytes;
/* 中文精读：发送状态机第 1 阶段：给 GPU 可写空间
 * 限制 posted < done+maxDepth，避免在途数据超过可回收槽位。
 * 共享模式先分配 offsFifo 偏移，内存屏障后更新 head，让 GPU 知道当前槽可写。
 * 非共享模式使用固定缓冲槽，posted 仍跟踪允许推进的步数。
 * posted 仅表示槽位准备好，数据可能尚未由 GPU 写入，更没有保证已发往网络。
 */
      // Post buffers to the GPU
      if (sub->posted < sub->nsteps && sub->posted < sub->done + maxDepth) {
/* 中文逐项-变量 V4088：buffSlot
 * buffSlot（int；函数 sendProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->posted)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
        if (resources->shared) {
/* 中文逐项-变量 V4089：sharedBuffSlot
 * sharedBuffSlot（int；函数 sendProxyProgress 内）：共享数据池中的在途槽号，与独立连接8槽的绝对 step/索引分开。
 * 初值/来源 sharedBuffSlot = sub->posted%maxDepth；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          int sharedBuffSlot = sub->posted%maxDepth;
/* 中文逐项-变量 V4090：offset
 * offset（int；函数 sendProxyProgress 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(sharedBuffersGet(comm, sub->channelId, sharedBuffSlot*args->nsubs+s,
 * &offset));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
          int offset;
          NCCLCHECK(sharedBuffersGet(comm, sub->channelId, sharedBuffSlot*args->nsubs+s, &offset));
          resources->recvMem->offsFifo[buffSlot] = offset;
          __sync_synchronize();
/* 中文逐项-变量 V4091：sendHead
 * sendHead（uint64_t；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendHead = resources->gdcSync ? resources->gdcSync :
 * &resources->sendMem->head；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          volatile uint64_t* sendHead = resources->gdcSync ? resources->gdcSync : &resources->sendMem->head;
          sub->posted += args->sliceSteps;
          *sendHead = sub->base + sub->posted - NCCL_STEPS;
          if (resources->gdcSync) wc_store_fence(); // Flush out WC write
        } else sub->posted += args->sliceSteps;
        for (/* 中文逐项-变量 V4092：step
 * step（uint64_t；函数 sendProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = sub->posted-args->sliceSteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：step<sub->posted；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=sub->posted-args->sliceSteps; step<sub->posted; step++) {
          ncclProfilingRecord(args, s, step, ncclProxyProfileSendGPUWait);
        }
        args->idle = 0;
        continue;
      }
/* 中文精读：发送状态机第 2 阶段：等 GPU 完成再提交网络
 * Simple 观察 sizesFifo 和 tail；LL/LL128 还要按具体缓冲位置检查协议 flag。
 * 只有整段数据就绪才调用 ncclNetIsend。若 request 返回 NULL，表示暂未接受，下一轮再试。
 * 请求有效后才清空 sizesFifo 并推进 transmitted，防止 GPU/Proxy 对一个槽重复提交。
 * 这里 CPU 只是提交 NIC 工作；GDR 时真实载荷可直接由 NIC 读取 GPU 缓冲，不必经 CPU memcpy。
 */
      // Check whether we received data from the GPU and send it to the network
      if (sub->transmitted < sub->posted && sub->transmitted < sub->done + NCCL_STEPS) {
/* 中文逐项-变量 V4093：buffSlot
 * buffSlot（int；函数 sendProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->transmitted)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->transmitted)%NCCL_STEPS;
/* 中文逐项-变量 V4094：sizesFifo
 * sizesFifo（int；函数 sendProxyProgress 内）：GPU 向代理报告各数据槽有效物理字节数的控制数组，空/未就绪值需按后端约定处理。
 * 初值/来源 sizesFifo = resources->recvMem->sizesFifo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        volatile int* sizesFifo = resources->recvMem->sizesFifo;
/* 中文逐项-变量 V4095：recvTail
 * recvTail（uint64_t；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvTail = &resources->recvMem->tail；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
        volatile uint64_t* recvTail = &resources->recvMem->tail;
        if (sizesFifo[buffSlot] != -1 && ((*recvTail > (sub->base+sub->transmitted)) || p == NCCL_PROTO_LL)) {
          // We have something to receive, let's check if it's completely ready.
/* 中文逐项-变量 V4096：size
 * size（int；函数 sendProxyProgress 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = sizesFifo[buffSlot]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          int size = sizesFifo[buffSlot];
/* 中文逐项-变量 V4097：buff
 * buff（char；函数 sendProxyProgress 内）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 * 初值/来源 buff = resources->shared ? localBuff+resources->recvMem->offsFifo[buffSlot] :
 * localBuff+buffSlot*stepSize；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          char* buff = resources->shared ? localBuff+resources->recvMem->offsFifo[buffSlot] : localBuff+buffSlot*stepSize;
/* 中文逐项-变量 V4098：ready
 * ready（int；函数 sendProxyProgress 内）：握手/连接是否完整就绪的状态，未就绪时须继续推进控制握手。
 * 初值/来源 ready = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          int ready = 1;
          if (p == NCCL_PROTO_LL128) {
            ready = resources->useGdr;
            if (!ready) {
              // When data is in sysmem, we need to wait until all flags are correct since the GPU only
              // called threadfence()
/* 中文逐项-变量 V4099：flag
 * flag（uint64_t；函数 sendProxyProgress 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = sub->base+sub->transmitted+1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              uint64_t flag = sub->base+sub->transmitted+1;
/* 中文逐项-变量 V4100：nFifoLines
 * nFifoLines（int；函数 sendProxyProgress 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nFifoLines = DIVUP(sizesFifo[buffSlot], sizeof(uint64_t)*NCCL_LL128_LINEELEMS)；向上取整分配，确保不能整除时也覆盖最后不足一块的数据。
 */
              int nFifoLines = DIVUP(sizesFifo[buffSlot], sizeof(uint64_t)*NCCL_LL128_LINEELEMS);
/* 中文逐项-变量 V4101：lines
 * lines（uint64_t；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lines = (volatile uint64_t*)buff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
              volatile uint64_t* lines = (volatile uint64_t*)buff;
              ready = 1;
              for (/* 中文逐项-变量 V4102：i
 * i（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nFifoLines；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nFifoLines; i++) {
                if (lines[i*NCCL_LL128_LINEELEMS+NCCL_LL128_DATAELEMS] != flag) { ready = 0; break; }
              }
            }
          } else if (p == NCCL_PROTO_LL) {
/* 中文逐项-变量 V4103：flag
 * flag（uint32_t；函数 sendProxyProgress 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = NCCL_LL_FLAG(sub->base+sub->transmitted+1)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            uint32_t flag = NCCL_LL_FLAG(sub->base+sub->transmitted+1);
/* 中文逐项-变量 V4104：nFifoLines
 * nFifoLines（int；函数 sendProxyProgress 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nFifoLines = DIVUP(size, sizeof(union ncclLLFifoLine))；向上取整分配，确保不能整除时也覆盖最后不足一块的数据。
 */
            int nFifoLines = DIVUP(size, sizeof(union ncclLLFifoLine));
/* 中文逐项-变量 V4105：lines
 * lines（union ncclLLFifoLine；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lines = (union ncclLLFifoLine*)buff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
            union ncclLLFifoLine* lines = (union ncclLLFifoLine*)buff;
            for (/* 中文逐项-变量 V4106：i
 * i（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nFifoLines；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nFifoLines; i++) {
/* 中文逐项-变量 V4107：f1
 * f1（uint32_t；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 f1 = &lines[i].flag1；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
              volatile uint32_t *f1 = &lines[i].flag1;
/* 中文逐项-变量 V4108：f2
 * f2（uint32_t；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 f2 = &lines[i].flag2；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
              volatile uint32_t *f2 = &lines[i].flag2;
              if (f1[0] != flag || f2[0] != flag) { ready = 0; break; }
            }
          }
          if (ready) {
            // Data is ready, try to send.
            NCCLCHECK(ncclNetIsend(resources->netSendComm, buff, size, resources->rank, mhandle, sub->requests+buffSlot));
            if (sub->requests[buffSlot] != NULL) {
              TRACE(NCCL_NET, "sendProxy [%ld/%d] Isend posted, req %p", sub->transmitted, buffSlot, sub->requests[buffSlot]);
              sizesFifo[buffSlot] = -1;
              // Make sure size is reset to zero before we update the head.
              __sync_synchronize();
              sub->transmitted += args->sliceSteps;
              for (/* 中文逐项-变量 V4109：step
 * step（uint64_t；函数 sendProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = sub->transmitted-args->sliceSteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：step<sub->transmitted；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=sub->transmitted-args->sliceSteps; step<sub->transmitted; step++) ncclProfilingRecord(args, s, step, ncclProxyProfileSendWait);
              args->idle = 0;
              continue;
            }
          }
        }
      }
/* 中文精读：发送状态机第 3 阶段：网络完成后回收槽位
 * ncclNetTest 把异步请求推进到完成，done 才能增加，固定缓冲模式此时发布 head 给 GPU。
 * 先前 Isend 返回成功只表示请求被接受，不能立即让 GPU 覆盖仍可能被 NIC 读取的源数据。
 * 所有 sub 的 done 达到 nsteps 后，这个 ProxyArgs 才进入 None，连接本身仍保留供后续操作使用。
 * resources->step 记录跨操作的基准进度，与设备 conn->step 保持同一协议节奏。
 */
      // Check whether the network has completed some send operations.
      if (sub->done < sub->transmitted) {
/* 中文逐项-变量 V4110：done
 * done（int；函数 sendProxyProgress 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 此处未显式给初值；后续首个文本使用点：int buffSlot = (sub->base+sub->done)%NCCL_STEPS;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int done;
/* 中文逐项-变量 V4111：buffSlot
 * buffSlot（int；函数 sendProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->done)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->done)%NCCL_STEPS;
        NCCLCHECK(ncclNetTest(sub->requests[buffSlot], &done, NULL));
        if (done) {
          TRACE(NCCL_NET, "sendProxy [%ld/%d] request %p done", sub->done, buffSlot, sub->requests[buffSlot]);
          sub->done += args->sliceSteps;
          for (/* 中文逐项-变量 V4112：step
 * step（uint64_t；函数 sendProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = sub->done-args->sliceSteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：step<sub->done；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=sub->done-args->sliceSteps; step<sub->done; step++) ncclProfilingRecord(args, s, step, ncclProxyProfileEnd);

          if (resources->shared == 0) {
/* 中文逐项-变量 V4113：sendHead
 * sendHead（uint64_t；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendHead = resources->gdcSync ? resources->gdcSync :
 * &resources->sendMem->head；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            volatile uint64_t* sendHead = resources->gdcSync ? resources->gdcSync : &resources->sendMem->head;
            *sendHead = sub->base + sub->done;
            if (resources->gdcSync) wc_store_fence(); // Flush out WC write
          }
          args->idle = 0;
          if (sub->done == sub->nsteps) {
            resources->step = sub->base + sub->nsteps;
            args->done++;
          }
        }
      }
    }
    if (args->done == args->nsubs) {
      args->state = ncclProxyOpNone;
    }
  }
  return ncclSuccess;
}

/* 中文导读：
 * 接收状态机：按相同 recvComm 分组挂接收 → test 网络完成 → 必要时 flush → 发布给 GPU → 等待消费。
 * 网络完成与 GPU 可见性分开处理；Simple+GDR 的条件分支可能调用 iflush。
 * maxRecvs 描述一次接口调用可聚合的接收数，不等于 verbs SRQ。
 */
/* 中文内部逻辑 F1095：recvProxyProgress
 * 精读：Ready时把相同recvComm子项归组，Irecv挂接收后posted前进。
 * 精读：Test完成先增加received；Simple+GDR等分支还需可见性flush，完成后才发布tail并推进transmitted。
 * 精读：最后等GPU消费head增加done；网络完成、GPU可读、GPU已消费是三个不同阶段。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(args->state == ncclProxyOpReady)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 准备局部状态：void* recvComm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     4. 准备局部状态：int groupSize = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 准备局部状态：int maxRecvs = 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 循环：初始 int s=0;；继续条件 s<args->nsubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       7. 准备局部状态：struct ncclProxySubArgs* sub = args->subs+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       8. 条件判断：(groupSize == maxRecvs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         9. 真分支：；仅条件为真执行以下子项。
 *           10. 调用 / 计算 / 写入：groupSize = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         11. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           12. 条件判断：(s>0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             13. 真分支：；仅条件为真执行以下子项。
 *               14. 准备局部状态：int next;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               15. 循环：初始 next=s；继续条件 next<args->nsubs；每轮更新 next++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                 16. 准备局部状态：struct recvResources* nextRes = (struct recvResources*) (args->subs[next].connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 17. 条件判断：(nextRes->netRecvComm == recvComm)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   18. 真分支：；仅条件为真执行以下子项。
 *                     19. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *               20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               21. 条件判断：(next == args->nsubs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 22. 真分支：；仅条件为真执行以下子项。
 *                   23. 调用 / 计算 / 写入：groupSize = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 24. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   25. 条件判断：(s != next)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     26. 真分支：；仅条件为真执行以下子项。
 *                       27. 准备局部状态：struct ncclProxySubArgs temp;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       28. 调用 / 计算 / 写入：memcpy(&temp, sub, sizeof(struct ncclProxySubArgs));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                       29. 调用 / 计算 / 写入：memcpy(sub, args->subs+next, sizeof(struct ncclProxySubArgs));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                       30. 调用 / 计算 / 写入：memcpy(args->subs+next, &temp, sizeof(struct ncclProxySubArgs));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       31. 调用 / 计算 / 写入：groupSize++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       32. 准备局部状态：struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       33. 调用 / 计算 / 写入：maxRecvs = resources->maxRecvs;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       34. 调用 / 计算 / 写入：recvComm = resources->netRecvComm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       35. 调用 / 计算 / 写入：sub->base = ROUNDUP(resources->step, args->chunkSteps);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *       36. 调用 / 计算 / 写入：sub->posted = sub->received = sub->transmitted = sub->done = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *       37. 循环：初始 int i=0;；继续条件 i<groupSize；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         38. 调用 / 计算 / 写入：sub[-i].groupSize = groupSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       39. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       40. 循环：初始 uint64_t step=0;；继续条件 step<sub->nsteps；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         41. 调用 / 计算 / 写入：ncclProfilingRecord(args, s, step, ncclProxyProfileBegin);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *       42. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     43. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     44. 调用 / 计算 / 写入：args->state = ncclProxyOpProgress;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 45. 调用 / 计算 / 写入：args->idle = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 46. 条件判断：(args->state == ncclProxyOpProgress)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   47. 真分支：；仅条件为真执行以下子项。
 *     48. 准备局部状态：int p = args->protocol;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     49. 准备局部状态：int maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args->nsubs);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     50. 循环：初始 int s=0;；继续条件 s<args->nsubs；每轮更新 s+=args->subs[s].groupSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       51. 准备局部状态：struct ncclProxySubArgs* subGroup = args->subs+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       52. 准备局部状态：int subCount = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       53. 准备局部状态：void* ptrs[NCCL_PROXY_MAX_SUBS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       54. 准备局部状态：int sizes[NCCL_PROXY_MAX_SUBS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       55. 准备局部状态：int tags[NCCL_PROXY_MAX_SUBS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       56. 准备局部状态：void* mhandles[NCCL_PROXY_MAX_SUBS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       57. 循环：初始 int i=0;；继续条件 i<subGroup->groupSize；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         58. 准备局部状态：struct ncclProxySubArgs* sub = subGroup + i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         59. 条件判断：(sub->posted < sub->nsteps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           60. 真分支：；仅条件为真执行以下子项。
 *             61. 条件判断：(sub->posted >= sub->done + maxDepth)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               62. 真分支：；仅条件为真执行以下子项。
 *                 63. 调用 / 计算 / 写入：subCount = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 64. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *             65. 准备局部状态：struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             66. 准备局部状态：int stepSize = resources->buffSizes[p] / NCCL_STEPS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             67. 准备局部状态：char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             68. 准备局部状态：int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *             69. 条件判断：(resources->shared)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               70. 真分支：；仅条件为真执行以下子项。
 *                 71. 准备局部状态：int sharedBuffSlot = sub->posted%maxDepth;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                 72. 准备局部状态：int offset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 73. 调用 / 计算 / 写入：NCCLCHECK(sharedBuffersGet(comm, sub->channelId, sharedBuffSlot*args->nsubs+s+i, &offset));；失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。
 *                 74. 准备局部状态：volatile int* offsFifo = (volatile int*)resources->recvMem->offsFifo;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 75. 调用 / 计算 / 写入：offsFifo[buffSlot] = offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 76. 调用 / 计算 / 写入：ptrs[subCount] = localBuff+offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               77. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                 78. 调用 / 计算 / 写入：ptrs[subCount] = localBuff+buffSlot*stepSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             79. 调用 / 计算 / 写入：sizes[subCount] = stepSize*args->sliceSteps;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             80. 条件判断：(sub->nbytes < sizes[subCount])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               81. 真分支：；仅条件为真执行以下子项。
 *                 82. 调用 / 计算 / 写入：sizes[subCount] = sub->nbytes;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             83. 调用 / 计算 / 写入：tags[subCount] = resources->remoteRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             84. 调用 / 计算 / 写入：mhandles[subCount] = resources->mhandles[p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             85. 调用 / 计算 / 写入：subCount++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       86. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       87. 条件判断：(subCount)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         88. 真分支：；仅条件为真执行以下子项。
 *           89. 准备局部状态：uint64_t step = subGroup->posted;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           90. 准备局部状态：struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           91. 准备局部状态：void** requestPtr = subGroup->requests+(step%NCCL_STEPS);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           92. 调用 / 计算 / 写入：NCCLCHECK(ncclNetIrecv(resources->netRecvComm, subCount, ptrs, sizes, tags, mhandles, requestPtr));；失败时可能提前return，成功才继续；提交一组带地址/长度/tag的异步接收；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *           93. 条件判断：(*requestPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             94. 真分支：；仅条件为真执行以下子项。
 *               95. 循环：初始 int i=0;；继续条件 i<subGroup->groupSize；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                 96. 准备局部状态：struct ncclProxySubArgs* sub = subGroup+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 97. 调用 / 计算 / 写入：sub->posted += args->sliceSteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                 98. 循环：初始 uint64_t step=sub->posted-args->sliceSteps;；继续条件 step<sub->posted；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                   99. 调用 / 计算 / 写入：ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvWait);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *                 100. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               101. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               102. 调用 / 计算 / 写入：args->idle = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     103. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     104. 条件判断：(args->idle == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       105. 真分支：；仅条件为真执行以下子项。
 *         106. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     107. 循环：初始 int s=0;；继续条件 s<args->nsubs；每轮更新 s+=args->subs[s].groupSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       108. 准备局部状态：struct ncclProxySubArgs* subGroup = args->subs+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       109. 条件判断：(subGroup->posted > subGroup->received)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         110. 真分支：；仅条件为真执行以下子项。
 *           111. 准备局部状态：uint64_t step = subGroup->received;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           112. 准备局部状态：int done;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           113. 准备局部状态：void* ptrs[NCCL_PROXY_MAX_SUBS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           114. 准备局部状态：int sizes[NCCL_PROXY_MAX_SUBS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           115. 准备局部状态：void* mhandles[NCCL_PROXY_MAX_SUBS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           116. 循环：初始 int i=0;；继续条件 i<NCCL_PROXY_MAX_SUBS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *             117. 调用 / 计算 / 写入：sizes[i] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           118. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           119. 调用 / 计算 / 写入：NCCLCHECK(ncclNetTest(subGroup->requests[step%NCCL_STEPS], &done, sizes));；失败时可能提前return，成功才继续；查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *           120. 条件判断：(done)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             121. 真分支：；仅条件为真执行以下子项。
 *               122. 准备局部状态：int useGdr = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               123. 准备局部状态：int totalSize = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               124. 循环：初始 int i=0;；继续条件 i<NCCL_PROXY_MAX_SUBS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                 125. 调用 / 计算 / 写入：totalSize += sizes[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               126. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               127. 循环：初始 int i=0;；继续条件 i<subGroup->groupSize；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                 128. 准备局部状态：struct ncclProxySubArgs* sub = subGroup + i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 129. 调用 / 计算 / 写入：sub->received += args->sliceSteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                 130. 循环：初始 uint64_t step=sub->received-args->sliceSteps;；继续条件 step<sub->received；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                   131. 调用 / 计算 / 写入：ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvFlushWait);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *                 132. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *                 133. 条件判断：(step < sub->nsteps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   134. 真分支：；仅条件为真执行以下子项。
 *                     135. 准备局部状态：struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                     136. 条件判断：(resources->useGdr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                       137. 真分支：；仅条件为真执行以下子项。
 *                         138. 调用 / 计算 / 写入：useGdr = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               139. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               140. 调用 / 计算 / 写入：subGroup->requests[step%NCCL_STEPS] = NULL;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *               141. 条件判断：(totalSize > 0 && p == NCCL_PROTO_SIMPLE && useGdr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 142. 真分支：；仅条件为真执行以下子项。
 *                   143. 准备局部状态：struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                   144. 条件判断：(resources->gdcFlush)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     145. 真分支：；仅条件为真执行以下子项。
 *                       146. 编译期分支/指令：#if defined (__x86_64__)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *                         147. 调用 / 计算 / 写入：asm volatile ("mov (%0), %%eax" :: "l"(resources->gdcFlush) : "%eax");；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *                         148. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *                           149. 调用 / 计算 / 写入：WARN("NET: GDR Flush only supported on x86_64");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                           150. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *                     151. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                       152. 准备局部状态：int subCount = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       153. 循环：初始 int i=0;；继续条件 i<subGroup->groupSize；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                         154. 准备局部状态：struct ncclProxySubArgs* sub = subGroup + i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                         155. 条件判断：(step < sub->nsteps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                           156. 真分支：；仅条件为真执行以下子项。
 *                             157. 准备局部状态：struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                             158. 准备局部状态：int stepSize = resources->buffSizes[p] / NCCL_STEPS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                             159. 准备局部状态：char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                             160. 准备局部状态：int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                             161. 调用 / 计算 / 写入：ptrs[subCount] = resources->shared ? localBuff+resources->recvMem->offsFifo[buffSlot] : localBuff+buffSlot*stepSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                             162. 调用 / 计算 / 写入：mhandles[subCount] = resources->mhandles[p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                             163. 调用 / 计算 / 写入：subCount++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                       164. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *                       165. 准备局部状态：struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       166. 调用 / 计算 / 写入：NCCLCHECK(ncclNetIflush(resources->netRecvComm, subCount, ptrs, sizes, mhandles, subGroup->requests+(step%NCCL_STEPS)));；失败时可能提前return，成功才继续；提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *               167. 调用 / 计算 / 写入：args->idle = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     168. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     169. 条件判断：(args->idle == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       170. 真分支：；仅条件为真执行以下子项。
 *         171. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     172. 循环：初始 int s=0;；继续条件 s<args->nsubs；每轮更新 s+=args->subs[s].groupSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       173. 准备局部状态：struct ncclProxySubArgs* subGroup = args->subs+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       174. 条件判断：(subGroup->received > subGroup->transmitted)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         175. 真分支：；仅条件为真执行以下子项。
 *           176. 准备局部状态：uint64_t step = subGroup->transmitted;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           177. 准备局部状态：int done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           178. 准备局部状态：void* request = subGroup->requests[step%NCCL_STEPS];；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           179. 条件判断：(request)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             180. 真分支：；仅条件为真执行以下子项。
 *               181. 调用 / 计算 / 写入：NCCLCHECK(ncclNetTest(request, &done, NULL));；失败时可能提前return，成功才继续；查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *           182. 条件判断：(done)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             183. 真分支：；仅条件为真执行以下子项。
 *               184. 循环：初始 int i=0;；继续条件 i<subGroup->groupSize；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                 185. 准备局部状态：struct ncclProxySubArgs* sub = subGroup + i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 186. 调用 / 计算 / 写入：sub->transmitted += args->sliceSteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                 187. 循环：初始 uint64_t step=sub->transmitted-args->sliceSteps;；继续条件 step<sub->transmitted；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                   188. 调用 / 计算 / 写入：ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvGPUWait);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *                 189. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *                 190. 条件判断：(step < sub->nsteps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   191. 真分支：；仅条件为真执行以下子项。
 *                     192. 调用 / 计算 / 写入：__sync_synchronize();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                     193. 准备局部状态：struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                     194. 准备局部状态：volatile uint64_t* recvTail = resources->gdcSync ? resources->gdcSync : &resources->recvMem->tail;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                     195. 调用 / 计算 / 写入：*recvTail = sub->base + sub->transmitted;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                     196. 条件判断：(resources->gdcSync)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                       197. 真分支：；仅条件为真执行以下子项。
 *                         198. 调用 / 计算 / 写入：wc_store_fence();；按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。
 *               199. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               200. 调用 / 计算 / 写入：args->idle = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     201. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     202. 条件判断：(args->idle == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       203. 真分支：；仅条件为真执行以下子项。
 *         204. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     205. 循环：初始 int s=0;；继续条件 s<args->nsubs；每轮更新 s+=args->subs[s].groupSize；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       206. 准备局部状态：struct ncclProxySubArgs* subGroup = args->subs+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       207. 循环：初始 int i=0;；继续条件 i<subGroup->groupSize；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         208. 准备局部状态：struct ncclProxySubArgs* sub = subGroup + i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         209. 条件判断：(sub->done == sub->nsteps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           210. 真分支：；仅条件为真执行以下子项。
 *             211. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *         212. 条件判断：(sub->transmitted > sub->done)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           213. 真分支：；仅条件为真执行以下子项。
 *             214. 准备局部状态：struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             215. 准备局部状态：volatile uint64_t* sendHead = &resources->sendMem->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *             216. 准备局部状态：uint64_t done = *sendHead;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *             217. 循环：初始 ；继续条件 (done > sub->base + sub->done && sub->transmitted > sub->done)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               218. 调用 / 计算 / 写入：sub->done += args->sliceSteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *               219. 循环：初始 uint64_t step=sub->done-args->sliceSteps;；继续条件 step<sub->done；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                 220. 调用 / 计算 / 写入：ncclProfilingRecord(args, s+i, step, ncclProxyProfileEnd);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *               221. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               222. 调用 / 计算 / 写入：args->idle = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               223. 条件判断：(sub->done == sub->nsteps)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 224. 真分支：；仅条件为真执行以下子项。
 *                   225. 准备局部状态：struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                   226. 调用 / 计算 / 写入：resources->step = sub->base + sub->nsteps;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                   227. 调用 / 计算 / 写入：args->done++;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *                   228. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *             229. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       230. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     231. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     232. 条件判断：(args->done == args->nsubs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       233. 真分支：；仅条件为真执行以下子项。
 *         234. 调用 / 计算 / 写入：args->state = ncclProxyOpNone;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 235. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_cc.md#f1095
 */
/* 中文逐项-函数 F1095：recvProxyProgress
 * 用途与位置：对应后端接收状态机：挂接收、检查网络完成、必要时 flush、发布给 GPU 并等待消费回收。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 args（struct ncclProxyArgs* args）：代理聚合运行态，保存各连接子任务的posted/received/transmitted/done进度和回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCL_NET_MAP_GET_POINTER；ROUNDUP；__sync_synchronize；memcpy；ncclNetIflush；ncclNetIrecv；ncclNetTest；ncclProfilingRecord；sharedBuffersGet；std::min；wc_store_fence。
 * 需要对照的边界/推进条件：args->state == ncclProxyOpReady；s<args->nsubs；groupSize == maxRecvs；s>0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvProxyProgress(struct ncclComm* comm, struct ncclProxyArgs* args) {
  if (args->state == ncclProxyOpReady) {
/* 中文精读：接收聚合不等于共享接收队列 SRQ
 * 把使用同一 backend recvComm 的子操作排在一起，最多按 maxRecvs 聚合一次 Irecv。
 * 每个 sub 仍有自己的 channel/连接/步数/指针/标签；后端据此匹配正确发送方。
 * 这里是 NCCL net 接口层的批量提交，是否使用 verbs SRQ 取决于后端实现。
 * 本仓库内置 net_ib 没有绑定 SRQ，不能由 maxRecvs>1 推断硬件 SRQ 已启用。
 */
    // Initialize subs and group them by same recvComm.
/* 中文逐项-变量 V4114：recvComm
 * recvComm（void；函数 recvProxyProgress 内）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 此处未显式给初值；后续首个文本使用点：if (nextRes->netRecvComm == recvComm) break;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    void* recvComm;
/* 中文逐项-变量 V4115：groupSize
 * groupSize（int；函数 recvProxyProgress 内）：连续使用同一个 recvComm 的子项数量，支持一次网络 Irecv 聚合；不意味着 verbs SRQ。
 * 初值/来源 groupSize = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int groupSize = 0;
/* 中文逐项-变量 V4116：maxRecvs
 * maxRecvs（int；函数 recvProxyProgress 内）：网络后端支持的并发连接数量及一次Irecv可聚合的接收项数，不等同于QP数量或SRQ。
 * 初值/来源 maxRecvs = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int maxRecvs = 1;
    for (/* 中文逐项-变量 V4117：s
 * s（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s++) {
/* 中文逐项-变量 V4118：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* sub = args->subs+s;
      if (groupSize == maxRecvs) {
        groupSize = 0;
      } else if (s>0) { // Find next sub with the same recvComm
/* 中文逐项-变量 V4119：next
 * next（int；函数 recvProxyProgress 内）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 此处未显式给初值；后续首个文本使用点：for (next=s; next<args->nsubs; next++) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int next;
        for (next=s; next<args->nsubs; next++) {
/* 中文逐项-变量 V4120：nextRes
 * nextRes（struct recvResources；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nextRes = (struct recvResources*)
 * (args->subs[next].connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          struct recvResources* nextRes = (struct recvResources*) (args->subs[next].connection->transportResources);
          if (nextRes->netRecvComm == recvComm) break;
        }
        if (next == args->nsubs) { // Not found
          groupSize = 0;
        } else if (s != next) { // We found a sub later with the same recvComm ; swap subs
/* 中文逐项-变量 V4121：temp
 * temp（struct ncclProxySubArgs；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memcpy(&temp, sub, sizeof(struct ncclProxySubArgs));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
          struct ncclProxySubArgs temp;
          memcpy(&temp, sub, sizeof(struct ncclProxySubArgs));
          memcpy(sub, args->subs+next, sizeof(struct ncclProxySubArgs));
          memcpy(args->subs+next, &temp, sizeof(struct ncclProxySubArgs));
        }
      }
      groupSize++;
/* 中文逐项-变量 V4122：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
      maxRecvs = resources->maxRecvs;
      recvComm = resources->netRecvComm;
      // Round to next multiple of sliceSteps
      sub->base = ROUNDUP(resources->step, args->chunkSteps);
      sub->posted = sub->received = sub->transmitted = sub->done = 0;
      for (/* 中文逐项-变量 V4123：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<groupSize；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<groupSize; i++) sub[-i].groupSize = groupSize;
      for (/* 中文逐项-变量 V4124：step
 * step（uint64_t；函数 recvProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：step<sub->nsteps；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=0; step<sub->nsteps; step++) ncclProfilingRecord(args, s, step, ncclProxyProfileBegin);
    }
    args->state = ncclProxyOpProgress;
  }
  args->idle = 1;
  if (args->state == ncclProxyOpProgress) {
/* 中文逐项-变量 V4125：p
 * p（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = args->protocol；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int p = args->protocol;
/* 中文逐项-变量 V4126：maxDepth
 * maxDepth（int；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args->nsubs)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
    int maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args->nsubs);
    for (/* 中文逐项-变量 V4127：s
 * s（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s+=args->subs[s].groupSize) {
/* 中文逐项-变量 V4128：subGroup
 * subGroup（struct ncclProxySubArgs；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 subGroup = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* subGroup = args->subs+s;
/* 中文逐项-变量 V4129：subCount
 * subCount（int；函数 recvProxyProgress 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 subCount = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int subCount = 0;
/* 中文逐项-变量 V4130：ptrs
 * ptrs（void；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ptrs[subCount] = localBuff+offset;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      void* ptrs[NCCL_PROXY_MAX_SUBS];
/* 中文逐项-变量 V4131：sizes
 * sizes（int；函数 recvProxyProgress 内）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 此处未显式给初值；后续首个文本使用点：sizes[subCount] = stepSize*args->sliceSteps;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int sizes[NCCL_PROXY_MAX_SUBS];
/* 中文逐项-变量 V4132：tags
 * tags（int；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：tags[subCount] = resources->remoteRank;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int tags[NCCL_PROXY_MAX_SUBS];
/* 中文逐项-变量 V4133：mhandles
 * mhandles（void；函数 recvProxyProgress 内）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 此处未显式给初值；后续首个文本使用点：mhandles[subCount] = resources->mhandles[p];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      void* mhandles[NCCL_PROXY_MAX_SUBS];

      for (/* 中文逐项-变量 V4134：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<subGroup->groupSize；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<subGroup->groupSize; i++) {
/* 中文逐项-变量 V4135：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = subGroup + i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        struct ncclProxySubArgs* sub = subGroup + i;
        if (sub->posted < sub->nsteps) {
          if (sub->posted >= sub->done + maxDepth) { subCount = 0; break; }
/* 中文逐项-变量 V4136：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
/* 中文逐项-变量 V4137：stepSize
 * stepSize（int；函数 recvProxyProgress 内）：一 FIFO step 的容量；设备 T 指针路径以元素计，主机缓冲路径以字节计。
 * 初值/来源 stepSize = resources->buffSizes[p] / NCCL_STEPS；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          int stepSize = resources->buffSizes[p] / NCCL_STEPS;
/* 中文逐项-变量 V4138：localBuff
 * localBuff（char；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p])；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);
/* 中文逐项-变量 V4139：buffSlot
 * buffSlot（int；函数 recvProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->posted)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
          int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
          if (resources->shared) {
/* 中文逐项-变量 V4140：sharedBuffSlot
 * sharedBuffSlot（int；函数 recvProxyProgress 内）：共享数据池中的在途槽号，与独立连接8槽的绝对 step/索引分开。
 * 初值/来源 sharedBuffSlot = sub->posted%maxDepth；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            int sharedBuffSlot = sub->posted%maxDepth;
/* 中文逐项-变量 V4141：offset
 * offset（int；函数 recvProxyProgress 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(sharedBuffersGet(comm, sub->channelId, sharedBuffSlot*args->nsubs+s+i,
 * &offset));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            int offset;
            NCCLCHECK(sharedBuffersGet(comm, sub->channelId, sharedBuffSlot*args->nsubs+s+i, &offset));
/* 中文逐项-变量 V4142：offsFifo
 * offsFifo（int；函数 recvProxyProgress 内）：共享数据池每槽字节偏移，代理先提供地址，GPU 再按偏移读写本次槽位。
 * 初值/来源 offsFifo = (volatile int*)resources->recvMem->offsFifo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            volatile int* offsFifo = (volatile int*)resources->recvMem->offsFifo;
            offsFifo[buffSlot] = offset;
            ptrs[subCount] = localBuff+offset;
          } else {
            ptrs[subCount] = localBuff+buffSlot*stepSize;
          }
          sizes[subCount] = stepSize*args->sliceSteps;
          if (sub->nbytes < sizes[subCount]) sizes[subCount] = sub->nbytes;
          tags[subCount] = resources->remoteRank;
          mhandles[subCount] = resources->mhandles[p];
          subCount++;
        }
      }
      if (subCount) {
/* 中文逐项-变量 V4143：step
 * step（uint64_t；函数 recvProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = subGroup->posted；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        uint64_t step = subGroup->posted;
/* 中文逐项-变量 V4144：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*)
 * (subGroup->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);
/* 中文逐项-变量 V4145：requestPtr
 * requestPtr（void；函数 recvProxyProgress 内）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 初值/来源 requestPtr = subGroup->requests+(step%NCCL_STEPS)；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        void** requestPtr = subGroup->requests+(step%NCCL_STEPS);
        NCCLCHECK(ncclNetIrecv(resources->netRecvComm, subCount, ptrs, sizes, tags, mhandles, requestPtr));
        if (*requestPtr) {
          for (/* 中文逐项-变量 V4146：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<subGroup->groupSize；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<subGroup->groupSize; i++) {
/* 中文逐项-变量 V4147：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = subGroup+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
            struct ncclProxySubArgs* sub = subGroup+i;
            sub->posted += args->sliceSteps;
            for (/* 中文逐项-变量 V4148：step
 * step（uint64_t；函数 recvProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = sub->posted-args->sliceSteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：step<sub->posted；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=sub->posted-args->sliceSteps; step<sub->posted; step++) ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvWait);
          }
          args->idle = 0;
        }
      }
    }
    if (args->idle == 0) return ncclSuccess;

    for (/* 中文逐项-变量 V4149：s
 * s（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s+=args->subs[s].groupSize) {
/* 中文逐项-变量 V4150：subGroup
 * subGroup（struct ncclProxySubArgs；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 subGroup = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* subGroup = args->subs+s;
      if (subGroup->posted > subGroup->received) {
/* 中文逐项-变量 V4151：step
 * step（uint64_t；函数 recvProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = subGroup->received；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        uint64_t step = subGroup->received;
/* 中文逐项-变量 V4152：done
 * done（int；函数 recvProxyProgress 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclNetTest(subGroup->requests[step%NCCL_STEPS], &done,
 * sizes));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int done;
/* 中文逐项-变量 V4153：ptrs
 * ptrs（void；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ptrs[subCount] = resources->shared ? localBuff+resources->recvMem->offsFifo[buffSlot] :
 * localBuff+buffSlot*stepSize;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        void* ptrs[NCCL_PROXY_MAX_SUBS];
/* 中文逐项-变量 V4154：sizes
 * sizes（int；函数 recvProxyProgress 内）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 此处未显式给初值；后续首个文本使用点：for (int i=0; i<NCCL_PROXY_MAX_SUBS; i++) sizes[i] = 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int sizes[NCCL_PROXY_MAX_SUBS];
/* 中文逐项-变量 V4155：mhandles
 * mhandles（void；函数 recvProxyProgress 内）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 此处未显式给初值；后续首个文本使用点：mhandles[subCount] = resources->mhandles[p];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        void* mhandles[NCCL_PROXY_MAX_SUBS];
        for (/* 中文逐项-变量 V4156：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_PROXY_MAX_SUBS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_PROXY_MAX_SUBS; i++) sizes[i] = 0;
        NCCLCHECK(ncclNetTest(subGroup->requests[step%NCCL_STEPS], &done, sizes));
        if (done) {
/* 中文逐项-变量 V4157：useGdr
 * useGdr（int；函数 recvProxyProgress 内）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 * 初值/来源 useGdr = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
          int useGdr = 0;
/* 中文逐项-变量 V4158：totalSize
 * totalSize（int；函数 recvProxyProgress 内）：排队有效数据规模统计，供负载平衡或聚合选型使用。
 * 初值/来源 totalSize = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
          int totalSize = 0;
          for (/* 中文逐项-变量 V4159：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_PROXY_MAX_SUBS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_PROXY_MAX_SUBS; i++) totalSize += sizes[i];
          for (/* 中文逐项-变量 V4160：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<subGroup->groupSize；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<subGroup->groupSize; i++) {
/* 中文逐项-变量 V4161：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = subGroup + i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
            struct ncclProxySubArgs* sub = subGroup + i;
            sub->received += args->sliceSteps;
            for (/* 中文逐项-变量 V4162：step
 * step（uint64_t；函数 recvProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = sub->received-args->sliceSteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：step<sub->received；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=sub->received-args->sliceSteps; step<sub->received; step++) ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvFlushWait);
            if (step < sub->nsteps) {
/* 中文逐项-变量 V4163：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
              if (resources->useGdr) useGdr = 1;
            }
          }
          subGroup->requests[step%NCCL_STEPS] = NULL;
/* 中文精读：接收完成还要考虑 GPU 可见性
 * 网络 Test 完成意味着后端请求结束，某些 GDR 路径还要执行 flush 确保 GPU 后续能看见数据。
 * 支持时走 GDRCopy 映射读，否则调用 net 后端 Iflush，再等待 flush 请求完成。
 * 之后才能发布接收 tail，GPU 才能离开 waitPeer 读取这段数据。
 * flush 不是把整个张量重新传一次；内置 IB 在特定配置下用小型自连接 RDMA read 实现。
 */
          if (totalSize > 0 && p == NCCL_PROTO_SIMPLE && useGdr) {
            // GDRCOPY support
/* 中文逐项-变量 V4164：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*)
 * (subGroup->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);
            if (resources->gdcFlush) {
#if defined (__x86_64__)
              // Force a PCI-E read from GPU memory
              asm volatile ("mov (%0), %%eax" :: "l"(resources->gdcFlush) : "%eax");
#else
              WARN("NET: GDR Flush only supported on x86_64");
              return ncclInternalError;
#endif
            } else {
/* 中文逐项-变量 V4165：subCount
 * subCount（int；函数 recvProxyProgress 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 subCount = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
              int subCount = 0;
              for (/* 中文逐项-变量 V4166：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<subGroup->groupSize；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<subGroup->groupSize; i++) {
/* 中文逐项-变量 V4167：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = subGroup + i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
                struct ncclProxySubArgs* sub = subGroup + i;
                if (step < sub->nsteps) {
/* 中文逐项-变量 V4168：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
                  struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
/* 中文逐项-变量 V4169：stepSize
 * stepSize（int；函数 recvProxyProgress 内）：一 FIFO step 的容量；设备 T 指针路径以元素计，主机缓冲路径以字节计。
 * 初值/来源 stepSize = resources->buffSizes[p] / NCCL_STEPS；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
                  int stepSize = resources->buffSizes[p] / NCCL_STEPS;
/* 中文逐项-变量 V4170：localBuff
 * localBuff（char；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p])；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
                  char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);
/* 中文逐项-变量 V4171：buffSlot
 * buffSlot（int；函数 recvProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->posted)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
                  int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
                  ptrs[subCount] = resources->shared ? localBuff+resources->recvMem->offsFifo[buffSlot] : localBuff+buffSlot*stepSize;
                  mhandles[subCount] = resources->mhandles[p];
                  subCount++;
                }
              }
/* 中文逐项-变量 V4172：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*)
 * (subGroup->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);
              NCCLCHECK(ncclNetIflush(resources->netRecvComm, subCount, ptrs, sizes, mhandles, subGroup->requests+(step%NCCL_STEPS)));
            }
          }
          args->idle = 0;
        }
      }
    }
    if (args->idle == 0) return ncclSuccess;

    for (/* 中文逐项-变量 V4173：s
 * s（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s+=args->subs[s].groupSize) {
/* 中文逐项-变量 V4174：subGroup
 * subGroup（struct ncclProxySubArgs；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 subGroup = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* subGroup = args->subs+s;
      if (subGroup->received > subGroup->transmitted) {
/* 中文逐项-变量 V4175：step
 * step（uint64_t；函数 recvProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = subGroup->transmitted；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        uint64_t step = subGroup->transmitted;
/* 中文逐项-变量 V4176：done
 * done（int；函数 recvProxyProgress 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 初值/来源 done = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        int done = 1;
/* 中文逐项-变量 V4177：request
 * request（void；函数 recvProxyProgress 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 初值/来源 request = subGroup->requests[step%NCCL_STEPS]；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        void* request = subGroup->requests[step%NCCL_STEPS];
        if (request) NCCLCHECK(ncclNetTest(request, &done, NULL));
        if (done) {
          for (/* 中文逐项-变量 V4178：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<subGroup->groupSize；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<subGroup->groupSize; i++) {
/* 中文逐项-变量 V4179：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = subGroup + i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
            struct ncclProxySubArgs* sub = subGroup + i;
            sub->transmitted += args->sliceSteps;
            for (/* 中文逐项-变量 V4180：step
 * step（uint64_t；函数 recvProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = sub->transmitted-args->sliceSteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：step<sub->transmitted；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=sub->transmitted-args->sliceSteps; step<sub->transmitted; step++) ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvGPUWait);
            if (step < sub->nsteps) {
              __sync_synchronize();
/* 中文逐项-变量 V4181：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
/* 中文逐项-变量 V4182：recvTail
 * recvTail（uint64_t；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvTail = resources->gdcSync ? resources->gdcSync :
 * &resources->recvMem->tail；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              volatile uint64_t* recvTail = resources->gdcSync ? resources->gdcSync : &resources->recvMem->tail;
              *recvTail = sub->base + sub->transmitted;
              if (resources->gdcSync) wc_store_fence(); // Flush out WC write
            }
          }
          args->idle = 0;
        }
      }
    }
    if (args->idle == 0) return ncclSuccess;

    for (/* 中文逐项-变量 V4183：s
 * s（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s+=args->subs[s].groupSize) {
/* 中文逐项-变量 V4184：subGroup
 * subGroup（struct ncclProxySubArgs；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 subGroup = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* subGroup = args->subs+s;
      for (/* 中文逐项-变量 V4185：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<subGroup->groupSize；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<subGroup->groupSize; i++) {
/* 中文逐项-变量 V4186：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = subGroup + i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        struct ncclProxySubArgs* sub = subGroup + i;
        if (sub->done == sub->nsteps) continue;
        if (sub->transmitted > sub->done) {
/* 中文逐项-变量 V4187：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
/* 中文逐项-变量 V4188：sendHead
 * sendHead（uint64_t；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendHead = &resources->sendMem->head；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
          volatile uint64_t* sendHead = &resources->sendMem->head;
/* 中文逐项-变量 V4189：done
 * done（uint64_t；函数 recvProxyProgress 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 初值/来源 done = *sendHead；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          uint64_t done = *sendHead;
          while (done > sub->base + sub->done &&
              // LL and LL128 can acknowledge 0-bytes send before they even happen. Don't go past what we transmitted.
              sub->transmitted > sub->done) {
            sub->done += args->sliceSteps;
            for (/* 中文逐项-变量 V4190：step
 * step（uint64_t；函数 recvProxyProgress 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = sub->done-args->sliceSteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：step<sub->done；循环体与增量表达式决定每轮如何推进。
 */uint64_t step=sub->done-args->sliceSteps; step<sub->done; step++) ncclProfilingRecord(args, s+i, step, ncclProxyProfileEnd);
            args->idle = 0;
            if (sub->done == sub->nsteps) {
/* 中文逐项-变量 V4191：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
              resources->step = sub->base + sub->nsteps;
              args->done++;
              break;
            }
          }
        }
      }
    }
    if (args->done == args->nsubs) {
      args->state = ncclProxyOpNone;
    }
  }
  return ncclSuccess;
}

/* 中文逐项-变量 V4192：netTransport
 * netTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 netTransport = { "NET", canConnect, { sendSetup, sendConnect, sendFree, proxySharedInit, sendProxySetup,
 * sendProxyConnect, sendProxyFree, sendProxyProgress }, { recvSetup, recvConnect, recvFree, proxySharedInit,
 * recvProxySetup, recvProxyConnect, …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct ncclTransport netTransport = {
  "NET",
  canConnect,
  { sendSetup, sendConnect, sendFree, proxySharedInit, sendProxySetup, sendProxyConnect, sendProxyFree, sendProxyProgress },
  { recvSetup, recvConnect, recvFree, proxySharedInit, recvProxySetup, recvProxyConnect, recvProxyFree, recvProxyProgress }
};
