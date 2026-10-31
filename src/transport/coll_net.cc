/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "coll_net.h"
#include "graph.h"
#include "proxy.h"
#include "gdrwrap.h"

int64_t ncclParamGdrCopySyncEnable();
int64_t ncclParamGdrCopyFlushEnable();

/* 中文逐项-类型 T0405：collNetRecvConnectInfo
 * 类型职责：CollNet网络集合建链信息、共享连接生命周期和请求协调槽。
 * 声明内可见成员（含嵌套成员）：rank、nranks、collNetHandle。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct collNetRecvConnectInfo {
/* 中文逐项-变量 V3755：rank
 * rank（int；成员 collNetRecvConnectInfo）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V3756：nranks
 * nranks（int；成员 collNetRecvConnectInfo）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 */
  int nranks;
/* 中文逐项-变量 V3757：collNetHandle
 * collNetHandle（collNetHandle_t；成员 collNetRecvConnectInfo）：CollNet建链需交换的句柄和全组连接信息数组。
 */
  collNetHandle_t collNetHandle;
};

/* 中文逐项-类型 T0406：collNetSendConnectInfo
 * 类型职责：CollNet网络集合建链信息、共享连接生命周期和请求协调槽。
 * 声明内可见成员（含嵌套成员）：mhandles、reqFifo。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct collNetSendConnectInfo {
/* 中文逐项-变量 V3758：mhandles
 * mhandles（void；成员 collNetSendConnectInfo）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 */
  void* mhandles[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3759：reqFifo
 * reqFifo（void；成员 collNetSendConnectInfo）：CollNet发送/接收侧协作使用的网络请求FIFO，协调共享集合归约结果。
 */
  void* reqFifo;
};

/* 中文逐项-常量 X1244：COLLNET_GROUP_NSUBS
 * 编译期定义：COLLNET_GROUP_NSUBS = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define COLLNET_GROUP_NSUBS 8
/* 中文逐项-常量 X1245：COLLNET_MAX_GROUPS
 * 编译期定义：COLLNET_MAX_GROUPS = (NCCL_PROXY_MAX_SUBS/COLLNET_GROUP_NSUBS)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define COLLNET_MAX_GROUPS (NCCL_PROXY_MAX_SUBS/COLLNET_GROUP_NSUBS)

/* 中文逐项-常量 X1246：NCCL_NET_MAP_HOSTMEM
 * 编译期定义：NCCL_NET_MAP_HOSTMEM = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_HOSTMEM 0
/* 中文逐项-常量 X1247：NCCL_NET_MAP_DEVMEM
 * 编译期定义：NCCL_NET_MAP_DEVMEM = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_DEVMEM 1
/* 中文逐项-常量 X1248：NCCL_NET_MAP_SHARED_HOSTMEM
 * 编译期定义：NCCL_NET_MAP_SHARED_HOSTMEM = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_SHARED_HOSTMEM 2
/* 中文逐项-常量 X1249：NCCL_NET_MAP_SHARED_DEVMEM
 * 编译期定义：NCCL_NET_MAP_SHARED_DEVMEM = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_SHARED_DEVMEM 3
/* 中文逐项-常量 X1250：NCCL_NET_MAP_GDCMEM
 * 编译期定义：NCCL_NET_MAP_GDCMEM = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_GDCMEM 4
/* 中文逐项-常量 X1251：NCCL_NET_MAP_MEMS
 * 编译期定义：NCCL_NET_MAP_MEMS = 5。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MEMS 5

/* 中文逐项-常量 X1252：NCCL_NET_MAP_MASK_DEVMEM
 * 编译期定义：NCCL_NET_MAP_MASK_DEVMEM = 0x40000000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MASK_DEVMEM 0x40000000
/* 中文逐项-常量 X1253：NCCL_NET_MAP_MASK_SHARED
 * 编译期定义：NCCL_NET_MAP_MASK_SHARED = 0x80000000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MASK_SHARED 0x80000000
/* 中文逐项-常量 X1254：NCCL_NET_MAP_MASK_USED
 * 编译期定义：NCCL_NET_MAP_MASK_USED = 0x20000000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MASK_USED   0x20000000
/* 中文逐项-常量 X1255：NCCL_NET_MAP_MASK_OFFSET
 * 编译期定义：NCCL_NET_MAP_MASK_OFFSET = 0x1fffffff。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAP_MASK_OFFSET 0x1fffffff

/* 中文逐项-宏 M0149：NCCL_NET_MAP_OFFSET_BANK
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_OFFSET_BANK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_OFFSET_BANK(mapStruct, offsetName) \
  ((mapStruct)->offsets.offsetName >> 30)

/* 中文逐项-宏 M0150：NCCL_NET_MAP_OFFSET_NULL
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_OFFSET_NULL；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_OFFSET_NULL(mapStruct, offsetName) \
  (((mapStruct)->offsets.offsetName >> 29) == 0)

/* 中文逐项-宏 M0151：NCCL_NET_MAP_GET_POINTER
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_GET_POINTER；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_GET_POINTER(mapStruct, cpuOrGpu, offsetName) \
  (NCCL_NET_MAP_OFFSET_NULL(mapStruct, offsetName) ? NULL : \
   (mapStruct)->mems[NCCL_NET_MAP_OFFSET_BANK(mapStruct, offsetName)].cpuOrGpu##Ptr + ((mapStruct)->offsets.offsetName & NCCL_NET_MAP_MASK_OFFSET))

/* 中文逐项-宏 M0152：NCCL_NET_MAP_DEV_MEM
 * 宏职责：对内存bank、地址或偏移做编码/取值；按展开式判断CPU/GPU视图与字节偏移，宏本身不自动分配数据。
 * 原始宏名 NCCL_NET_MAP_DEV_MEM；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NET_MAP_DEV_MEM(mapStruct, offsetName) \
  (((mapStruct)->offsets.offsetName & NCCL_NET_MAP_MASK_DEVMEM) != 0)

/* 中文逐项-宏 M0153：NCCL_NET_MAP_ADD_POINTER
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

/* 中文逐项-类型 T0407：connectMapMem
 * 类型职责：NET或CollNet各bank的CPU/GPU基址、大小、共享名字和控制/数据偏移表。
 * 声明内可见成员（含嵌套成员）：gpuPtr、cpuPtr、size。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct connectMapMem{
/* 中文逐项-变量 V3760：gpuPtr
 * gpuPtr（char；成员 connectMapMem）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 */
  char* gpuPtr;
/* 中文逐项-变量 V3761：cpuPtr
 * cpuPtr（char；成员 connectMapMem）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 */
  char* cpuPtr;
/* 中文逐项-变量 V3762：size
 * size（int；成员 connectMapMem）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  int size;
};

/* 中文逐项-类型 T0408：connectMap
 * 类型职责：NET或CollNet各bank的CPU/GPU基址、大小、共享名字和控制/数据偏移表。
 * 声明内可见成员（含嵌套成员）：shared、mems、offsets、sendMem、recvMem、buffs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct connectMap {
/* 中文逐项-变量 V3763：shared
 * shared（int；成员 connectMap）：是否使用共享缓冲资源的标志；不自动表示所有 channel 或全部 backend 连接合并。
 */
  int shared;
  // First 3 bits of offsets determine the mem bank. 001 is host mem, 011 is dev mem, 101 is shared host mem and 111 is shared dev mem.
/* 中文逐项-变量 V3764：mems
 * mems（struct connectMapMem；成员 connectMap）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
  struct connectMapMem mems[NCCL_NET_MAP_MEMS];
  // Offsets. 3 MSBs indicate mem bank, 111 indicates NULL.
/* 中文逐项-变量 V3765：offsets
 * offsets（struct { uint32_t sendMem; uint32_t recvMem; uint32_t buffs[NCCL_NUM_PROTOCOLS]; }；成员
 * connectMap）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
/* 中文逐项-类型 T0409：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：offsets、sendMem、recvMem、buffs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct {
/* 中文逐项-变量 V3766：sendMem
 * sendMem（uint32_t；成员 connectMap::<匿名>）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
    uint32_t sendMem;
/* 中文逐项-变量 V3767：recvMem
 * recvMem（uint32_t；成员 connectMap::<匿名>）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
    uint32_t recvMem;
/* 中文逐项-变量 V3768：buffs
 * buffs（uint32_t；成员 connectMap::<匿名>）：按 LL/LL128/Simple 区分的协议缓冲基址集合，承载分片流水数据。
 */
    uint32_t buffs[NCCL_NUM_PROTOCOLS];
  } offsets;
};

/* 中文逐项-类型 T0410：reqSlot
 * 类型职责：CollNet网络集合建链信息、共享连接生命周期和请求协调槽。
 * 声明内可见成员（含嵌套成员）：recvBuff、size。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct reqSlot {
/* 中文逐项-变量 V3769：recvBuff
 * recvBuff（void；成员 reqSlot）：各邻居的协议缓冲指针，按当前 step 计算实际槽位置。
 */
  volatile void* recvBuff;
/* 中文逐项-变量 V3770：size
 * size（int；成员 reqSlot）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  volatile int size;
};

/* 中文逐项-类型 T0411：sendResources
 * 类型职责：当前transport方向私有的控制/协议缓冲、后端句柄、注册和进度资源，字段依所在文件不同。
 * 声明内可见成员（含嵌套成员）：map、collNetComm、sendMem、recvMem、rank、nranks、netDev、useGdr、gdcSync、gdrDesc、sendMhandles、recvMhandles、step、reqFifo、collNetRank。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct sendResources {
/* 中文逐项-变量 V3771：map
 * map（struct connectMap；成员 sendResources）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
  struct connectMap map;
/* 中文逐项-变量 V3772：collNetComm
 * collNetComm（void；成员 sendResources）：CollNet后端集合/监听连接，多个逻辑通道可能按设备共享。
 */
  void* collNetComm;
/* 中文逐项-变量 V3773：sendMem
 * sendMem（struct ncclSendMem；成员 sendResources）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
  struct ncclSendMem* sendMem;
/* 中文逐项-变量 V3774：recvMem
 * recvMem（struct ncclRecvMem；成员 sendResources）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
  struct ncclRecvMem* recvMem;

/* 中文逐项-变量 V3775：rank
 * rank（int；成员 sendResources）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V3776：nranks
 * nranks（int；成员 sendResources）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 */
  int nranks;
/* 中文逐项-变量 V3777：netDev
 * netDev（int；成员 sendResources）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int netDev;
/* 中文逐项-变量 V3778：useGdr
 * useGdr（int；成员 sendResources）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 */
  int useGdr;
/* 中文逐项-变量 V3779：gdcSync
 * gdcSync（uint64_t；成员 sendResources）：可选 GDRCopy 映射控制地址，分别用于低延迟进度发布或接收可见性刷新。
 */
  uint64_t* gdcSync;
/* 中文逐项-变量 V3780：gdrDesc
 * gdrDesc（void；成员 sendResources）：撤销 GDRCopy pin/map 及显存分配所需的描述对象。
 */
  void* gdrDesc;
/* 中文逐项-变量 V3781：sendMhandles
 * sendMhandles（void；成员 sendResources）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 */
  void* sendMhandles[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3782：recvMhandles
 * recvMhandles（void；成员 sendResources）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 */
  void* recvMhandles[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3783：step
 * step（uint64_t；成员 sendResources）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 */
  uint64_t step;
/* 中文逐项-变量 V3784：reqFifo
 * reqFifo 是函数指针字段/变量：CollNet发送/接收侧协作使用的网络请求FIFO，协调共享集合归约结果；函数表保存可调用入口，不是调用返回的数据。完整声明：(*reqFifo)[NCCL_STEPS]。
 * reqFifo（struct reqSlot；成员 sendResources）：CollNet发送/接收侧协作使用的网络请求FIFO，协调共享集合归约结果。
 */
  struct reqSlot (*reqFifo)[NCCL_STEPS];
/* 中文逐项-变量 V3785：collNetRank
 * collNetRank（int；成员 sendResources）：当前网络集合参与者编号，可能只枚举head，范围与全体GPU rank不同。
 */
  int collNetRank;
};

/* 中文逐项-类型 T0412：recvResources
 * 类型职责：当前transport方向私有的控制/协议缓冲、后端句柄、注册和进度资源，字段依所在文件不同。
 * 声明内可见成员（含嵌套成员）：map、collNetComm、sendMem、recvMem、rank、nranks、netDev、useGdr、gdcSync、gdcFlush、gdrDesc、mhandles、step、reqFifo、collNetRank。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct recvResources {
/* 中文逐项-变量 V3786：map
 * map（struct connectMap；成员 recvResources）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
  struct connectMap map;
/* 中文逐项-变量 V3787：collNetComm
 * collNetComm（void；成员 recvResources）：CollNet后端集合/监听连接，多个逻辑通道可能按设备共享。
 */
  void* collNetComm;
/* 中文逐项-变量 V3788：sendMem
 * sendMem（struct ncclSendMem；成员 recvResources）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
  struct ncclSendMem* sendMem;
/* 中文逐项-变量 V3789：recvMem
 * recvMem（struct ncclRecvMem；成员 recvResources）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 */
  struct ncclRecvMem* recvMem;

/* 中文逐项-变量 V3790：rank
 * rank（int；成员 recvResources）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V3791：nranks
 * nranks（int；成员 recvResources）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 */
  int nranks;
/* 中文逐项-变量 V3792：netDev
 * netDev（int；成员 recvResources）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int netDev;
/* 中文逐项-变量 V3793：useGdr
 * useGdr（int；成员 recvResources）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 */
  int useGdr;
/* 中文逐项-变量 V3794：gdcSync
 * gdcSync（uint64_t；成员 recvResources）：可选 GDRCopy 映射控制地址，分别用于低延迟进度发布或接收可见性刷新。
 */
  uint64_t* gdcSync;
/* 中文逐项-变量 V3795：gdcFlush
 * gdcFlush（uint64_t；成员 recvResources）：可选 GDRCopy 映射控制地址，分别用于低延迟进度发布或接收可见性刷新。
 */
  uint64_t* gdcFlush;
/* 中文逐项-变量 V3796：gdrDesc
 * gdrDesc（void；成员 recvResources）：撤销 GDRCopy pin/map 及显存分配所需的描述对象。
 */
  void* gdrDesc;
/* 中文逐项-变量 V3797：mhandles
 * mhandles（void；成员 recvResources）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 */
  void* mhandles[NCCL_NUM_PROTOCOLS];
/* 中文逐项-变量 V3798：step
 * step（uint64_t；成员 recvResources）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 */
  uint64_t step;
/* 中文逐项-变量 V3799：reqFifo
 * reqFifo（struct reqSlot；成员 recvResources）：CollNet发送/接收侧协作使用的网络请求FIFO，协调共享集合归约结果。
 */
  struct reqSlot reqFifo[COLLNET_MAX_GROUPS][NCCL_STEPS];
/* 中文逐项-变量 V3800：collNetRank
 * collNetRank（int；成员 recvResources）：当前网络集合参与者编号，可能只枚举head，范围与全体GPU rank不同。
 */
  int collNetRank;
};

/* Determine if we can communicate with the peer */
/* 中文逐项-函数 F1052：canConnect
 * 用途与位置：由本文件 transport 的拓扑/主机/后端支持条件判断这对参与者是否可使用该传输。
 * 参数 ret（int* ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 topo（struct ncclTopoSystem* topo）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 info1（struct ncclPeerInfo* info1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 info2（struct ncclPeerInfo* info2）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t canConnect(int* ret, struct ncclTopoSystem* topo, struct ncclTopoGraph* graph, struct ncclPeerInfo* info1, struct ncclPeerInfo* info2) {
  *ret = 1;
  return ncclSuccess;
}

/* 中文逐项-类型 T0413：setupReq
 * 类型职责：主线程传给实际代理的建链参数包，包含设备、channel、peer等必要策略信息。
 * 声明内可见成员（含嵌套成员）：netDev、useGdr。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct setupReq {
/* 中文逐项-变量 V3801：netDev
 * netDev（int；成员 setupReq）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int netDev;
/* 中文逐项-变量 V3802：useGdr
 * useGdr（int；成员 setupReq）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 */
  int useGdr;
};


/* Setup send connector, and return connect information for others in the coll
 * communicator to connect to me */
/* 中文逐项-函数 F1053：sendSetup
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
 * 本体直接出现的调用/包装：NCCLCHECK；collNetName；ncclProxyCall；ncclProxyConnect；ncclTopoCheckGdr；ncclTopoGetLocalRank；ncclTopoGetNetDev。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclConnect* connectInfo, struct ncclConnector* send, int channelId, int connIndex) {
/* 中文逐项-变量 V3803：req
 * req（struct setupReq；函数 sendSetup 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, -1, &req.netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct setupReq req;

/* 中文逐项-变量 V3804：proxyRank
 * proxyRank（int；函数 sendSetup 内）：实际负责网络出口的代理 rank，PXN 时可能与发起通信的 GPU rank 不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, -1, &req.netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int proxyRank;
  NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, -1, &req.netDev, &proxyRank));
  NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 1, &req.useGdr));
  send->conn.direct |= req.useGdr ? NCCL_DIRECT_NIC : 0;

  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &send->proxyConn.localRank));
  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_COLLNET, 1, myInfo->rank, &send->proxyConn));
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), NULL, 0));

  INFO(NCCL_INIT|NCCL_NET,"CollNet %02d : %d [send] via COLLNET/%s/%d%s", channelId, myInfo->rank, collNetName(), req.netDev,
      req.useGdr ? "/GDRDMA" : "");
  return ncclSuccess;
}

/* 中文逐项-函数 F1054：recvSetup
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
 * 本体直接出现的调用/包装：NCCLCHECK；collNetName；ncclProxyCall；ncclProxyConnect；ncclTopoCheckGdr；ncclTopoGetLocalRank；ncclTopoGetNetDev。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclConnect* connectInfo, struct ncclConnector* recv, int channelId, int connIndex) {
/* 中文逐项-变量 V3805：req
 * req（struct setupReq；函数 recvSetup 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, -1, &req.netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct setupReq req;

/* 中文逐项-变量 V3806：proxyRank
 * proxyRank（int；函数 recvSetup 内）：实际负责网络出口的代理 rank，PXN 时可能与发起通信的 GPU rank 不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, -1, &req.netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int proxyRank;
  NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, -1, &req.netDev, &proxyRank));
  NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 0, &req.useGdr));
  recv->conn.direct |= req.useGdr ? NCCL_DIRECT_NIC : 0;

  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &recv->proxyConn.localRank));
  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_COLLNET, 0, myInfo->rank, &recv->proxyConn));
/* 中文逐项-变量 V3807：info
 * info（struct collNetRecvConnectInfo；函数 recvSetup 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct collNetRecvConnectInfo*) connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct collNetRecvConnectInfo* info = (struct collNetRecvConnectInfo*) connectInfo;
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), &info->collNetHandle, sizeof(collNetHandle_t)));

  INFO(NCCL_INIT|NCCL_NET,"CollNet %02d : %d [receive] via COLLNET/%s/%d%s", channelId, myInfo->rank, collNetName(), req.netDev,
      req.useGdr ? "/GDRDMA" : "");
  return ncclSuccess;
}

/* 中文逐项-函数 F1055：collNetDumpMap
 * 用途与位置：打印后端各内存 bank 的 CPU/GPU 地址、大小和映射偏移，辅助检查跨进程地址视图。
 * 参数 map（struct connectMap* map）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCL_NET_MAP_GET_POINTER；NCCL_NET_MAP_OFFSET_BANK。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetDumpMap(struct connectMap* map) {
  printf("Dump map\n");
/* 中文逐项-变量 V3808：mem
 * mem（struct connectMapMem；函数 collNetDumpMap 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mem = map->mems+NCCL_NET_MAP_HOSTMEM；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMapMem *mem = map->mems+NCCL_NET_MAP_HOSTMEM;
  printf("Mem 0: Host mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_DEVMEM;
  printf("Mem 1: Vid  mem CPU (%x B) %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_SHARED_HOSTMEM;
  printf("Mem 2: Shared Host mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_SHARED_DEVMEM;
  printf("Mem 3: Shared Vid  (%x B) mem CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  printf("SendMem -> Used %d Bank %d Offset %x, cpu %p gpu %p\n",
      map->offsets.sendMem & NCCL_NET_MAP_MASK_USED ? 1 : 0,
      NCCL_NET_MAP_OFFSET_BANK(map, sendMem), map->offsets.sendMem & NCCL_NET_MAP_MASK_OFFSET,
      NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem), NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem));
  printf("RecvMem -> Used %d Bank %d Offset %x, cpu %p gpu %p\n",
      map->offsets.recvMem & NCCL_NET_MAP_MASK_USED ? 1 : 0,
      NCCL_NET_MAP_OFFSET_BANK(map, recvMem), map->offsets.recvMem & NCCL_NET_MAP_MASK_OFFSET,
      NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem), NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem));
  for (/* 中文逐项-变量 V3809：p
 * p（int；函数 collNetDumpMap 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
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

/* 中文逐项-类型 T0414：collNetConnectArgs
 * 类型职责：CollNet网络集合建链信息、共享连接生命周期和请求协调槽。
 * 声明内可见成员（含嵌套成员）：rank、nranks、connectInfos。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct collNetConnectArgs {
/* 中文逐项-变量 V3810：rank
 * rank（int；成员 collNetConnectArgs）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V3811：nranks
 * nranks（int；成员 collNetConnectArgs）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 */
  int nranks;
/* 中文逐项-变量 V3812：connectInfos
 * connectInfos（struct ncclConnect；成员 collNetConnectArgs）：CollNet建链需交换的句柄和全组连接信息数组。
 */
  struct ncclConnect* connectInfos;
};

/* 中文逐项-函数 F1056：sendConnect
 * 用途与位置：消费远端句柄完成对应方向的连接和地址映射，把协议缓冲/进度指针填写到设备连接视图。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 connectInfos（struct ncclConnect* connectInfos）：CollNet建链需交换的句柄和全组连接信息数组。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCL_NET_MAP_GET_POINTER；ncclProxyCall。
 * 需要对照的边界/推进条件：map == NULL；i<NCCL_STEPS；p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendConnect(struct ncclComm* comm, struct ncclConnect* connectInfos, int nranks, int rank, struct ncclConnector* send) {
  // We're on the same process as the proxy. We can pass a pointer to a struct.
/* 中文逐项-变量 V3813：args
 * args（struct collNetConnectArgs；函数 sendConnect 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = { rank, nranks, connectInfos }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct collNetConnectArgs args = { rank, nranks, connectInfos };
/* 中文逐项-变量 V3814：map
 * map（struct connectMap；函数 sendConnect 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgConnect, &args, sizeof(struct
 * collNetConnectArgs), &map, sizeof(struct connectMap*)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct connectMap* map;
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgConnect, &args, sizeof(struct collNetConnectArgs), &map, sizeof(struct connectMap*)));

  // If collnet connect failed, propagate error to fallback on regular p2p
  if (map == NULL) return ncclSystemError;

  //NCCLCHECK(collNetDumpMap(map));

/* 中文逐项-变量 V3815：sendMem
 * sendMem（struct ncclSendMem；函数 sendConnect 内）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 * 初值/来源 sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu,
 * sendMem)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);
/* 中文逐项-变量 V3816：gdcMem
 * gdcMem（void；函数 sendConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;
  send->conn.head = gdcMem ? (uint64_t*)gdcMem : &sendMem->head;

/* 中文逐项-变量 V3817：recvMem
 * recvMem（struct ncclRecvMem；函数 sendConnect 内）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 * 初值/来源 recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu,
 * recvMem)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);
  send->conn.tail = &recvMem->tail;
  send->conn.sizesFifo = recvMem->sizesFifo;
  for (/* 中文逐项-变量 V3818：i
 * i（int；函数 sendConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_STEPS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_STEPS; i++) send->conn.sizesFifo[i] = -1;
  send->conn.offsFifo = recvMem->offsFifo;

  for (/* 中文逐项-变量 V3819：p
 * p（int；函数 sendConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    send->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);
  return ncclSuccess;
}

/* 中文逐项-函数 F1057：recvConnect
 * 用途与位置：消费远端句柄完成对应方向的连接和地址映射，把协议缓冲/进度指针填写到设备连接视图。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 connectInfos（struct ncclConnect* connectInfos）：CollNet建链需交换的句柄和全组连接信息数组。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCL_NET_MAP_GET_POINTER；ncclProxyCall。
 * 需要对照的边界/推进条件：map == NULL；p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvConnect(struct ncclComm* comm, struct ncclConnect* connectInfos, int nranks, int rank, struct ncclConnector* recv) {
  // We're on the same process as the proxy. We can pass a pointer to a struct.
/* 中文逐项-变量 V3820：args
 * args（struct collNetConnectArgs；函数 recvConnect 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = { rank, nranks, connectInfos }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct collNetConnectArgs args = { rank, nranks, connectInfos };
/* 中文逐项-变量 V3821：map
 * map（struct connectMap；函数 recvConnect 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgConnect, &args, sizeof(struct
 * collNetConnectArgs), &map, sizeof(struct connectMap*)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct connectMap* map;
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgConnect, &args, sizeof(struct collNetConnectArgs), &map, sizeof(struct connectMap*)));

  // If collnet connect failed, propagate error to fallback on regular p2p
  if (map == NULL) return ncclSystemError;

  //NCCLCHECK(collNetDumpMap(map));

/* 中文逐项-变量 V3822：sendMem
 * sendMem（struct ncclSendMem；函数 recvConnect 内）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 * 初值/来源 sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu,
 * sendMem)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);
  recv->conn.head = &sendMem->head;

/* 中文逐项-变量 V3823：recvMem
 * recvMem（struct ncclRecvMem；函数 recvConnect 内）：连接发送消费控制区/接收生产控制区，保存head/tail及大小/偏移FIFO。
 * 初值/来源 recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu,
 * recvMem)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);
/* 中文逐项-变量 V3824：gdcMem
 * gdcMem（void；函数 recvConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;
  recv->conn.tail = gdcMem ? (uint64_t*)gdcMem : &recvMem->tail;
  recv->conn.offsFifo = recvMem->offsFifo;

  for (/* 中文逐项-变量 V3825：p
 * p（int；函数 recvConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    recv->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F1058：sendFree
 * 用途与位置：释放对应逻辑连接持有的本地资源/映射，并按代理与共享计数规则释放后台对象。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendFree(struct ncclConnector* send) {
  return ncclSuccess;
}

/* 中文逐项-函数 F1059：recvFree
 * 用途与位置：释放对应逻辑连接持有的本地资源/映射，并按代理与共享计数规则释放后台对象。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvFree(struct ncclConnector* recv) {
  return ncclSuccess;
}

/* 中文逐项-函数 F1060：sendProxySetup
 * 用途与位置：在实际代理进程中准备对应方向网络设备与连接资源；接收方向通常还要 listen 生成句柄。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc。
 * 需要对照的边界/推进条件：reqSize != sizeof(struct setupReq。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendProxySetup(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
/* 中文逐项-变量 V3826：req
 * req（struct setupReq；函数 sendProxySetup 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 初值/来源 req = (struct setupReq*)reqBuff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct setupReq* req = (struct setupReq*)reqBuff;
  if (reqSize != sizeof(struct setupReq)) return ncclInternalError;

/* 中文逐项-变量 V3827：resources
 * resources（struct sendResources；函数 sendProxySetup 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&resources, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct sendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  connection->transportResources = resources;
  connection->shared = 1;

  resources->netDev = req->netDev;
  resources->useGdr = req->useGdr;
  return ncclSuccess;
}

/* 中文逐项-类型 T0415：sharedResources
 * 类型职责：CollNet网络集合建链信息、共享连接生命周期和请求协调槽。
 * 声明内可见成员（含嵌套成员）：collNetListenComms、collNetComms、commRefCount。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct sharedResources {
/* 中文逐项-变量 V3828：collNetListenComms
 * collNetListenComms（void；成员 sharedResources）：CollNet后端集合/监听连接，多个逻辑通道可能按设备共享。
 */
  void* collNetListenComms[MAXCHANNELS];
/* 中文逐项-变量 V3829：collNetComms
 * collNetComms（void；成员 sharedResources）：CollNet后端集合/监听连接，多个逻辑通道可能按设备共享。
 */
  void* collNetComms[MAXCHANNELS];
/* 中文逐项-变量 V3830：commRefCount
 * commRefCount（int；成员 sharedResources）：共享对象引用计数；只有最后引用释放才允许回收真实资源。
 */
  int commRefCount[NCCL_MAX_NETDEVS];
};

/* 中文逐项-函数 F1061：sharedListen
 * 用途与位置：按网络设备复用 CollNet 监听对象并返回可交换网络句柄。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 netDev（int netDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 参数 collNetHandle（void* collNetHandle）：CollNet建链需交换的句柄和全组连接信息数组。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::recvProxySetup。
 * 本体直接出现的调用/包装：NCCLCHECK；collNetListen；ncclCalloc。
 * 需要对照的边界/推进条件：resources == NULL；resources->collNetComms[netDev] == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t sharedListen(struct ncclComm* comm, int netDev, void* collNetHandle) {
/* 中文逐项-变量 V3831：resources
 * resources（struct sharedResources；函数 sharedListen 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct
 * sharedResources*)comm->proxyState.progressState.collNet.resources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct sharedResources* resources = (struct sharedResources*)comm->proxyState.progressState.collNet.resources;
  if (resources == NULL) {
    NCCLCHECK(ncclCalloc(&resources, 1));
    comm->proxyState.progressState.collNet.resources = resources;
  }
  if (resources->collNetComms[netDev] == NULL)
    NCCLCHECK(collNetListen(netDev, collNetHandle, resources->collNetListenComms+netDev));
  return ncclSuccess;
}

/* 中文精读：CollNet 连接共享的作用域
 * 按 netDev 管理共享资源与引用计数，多个逻辑 channel 可引用同一 collNetComm。
 * 首次建立时用参与者句柄数组和 rank 信息调用插件 connect；后续路径只增加引用。
 * 最后一个使用者释放时才 closeColl，避免某条逻辑连接释放后破坏仍在使用的共享对象。
 * 这里的共享是 NCCL/插件对象生命周期，内部有多少 QP/MR 必须继续看具体插件。
 */
/* 中文逐项-函数 F1062：sharedConnect
 * 用途与位置：按设备引用计数复用 CollNet 集合连接，用完整网络参与句柄数组完成首次连接。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 netDev（int netDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 参数 connectInfos（struct ncclConnect* connectInfos）：CollNet建链需交换的句柄和全组连接信息数组。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 collNetComm（void** collNetComm）：CollNet后端集合/监听连接，多个逻辑通道可能按设备共享。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyConnect；src/transport/coll_net.cc::recvProxyConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；collNetCloseListen；collNetConnect；free；ncclCalloc。
 * 需要对照的边界/推进条件：resources->collNetComms[netDev] == NULL；i < nranks；ret == ncclSuccess；*collNetComm。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sharedConnect(struct ncclComm* comm, int netDev, struct ncclConnect* connectInfos, int nranks, int rank, void** collNetComm) {
/* 中文逐项-变量 V3832：resources
 * resources（struct sharedResources；函数 sharedConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct
 * sharedResources*)comm->proxyState.progressState.collNet.resources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct sharedResources* resources = (struct sharedResources*)comm->proxyState.progressState.collNet.resources;
  if (resources->collNetComms[netDev] == NULL) {
    // Connect to coll comm
/* 中文逐项-变量 V3833：handlePtrs
 * handlePtrs（collNetHandle_t；函数 sharedConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlePtrs = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    collNetHandle_t** handlePtrs = NULL;
    NCCLCHECK(ncclCalloc(&handlePtrs, nranks));
    for (/* 中文逐项-变量 V3834：i
 * i（int；函数 sharedConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < nranks；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < nranks; i++) {
/* 中文逐项-变量 V3835：info
 * info（struct collNetRecvConnectInfo；函数 sharedConnect 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct collNetRecvConnectInfo*)(connectInfos+i)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      struct collNetRecvConnectInfo* info = (struct collNetRecvConnectInfo*)(connectInfos+i);
      handlePtrs[i] = &(info->collNetHandle);
    }
/* 中文逐项-变量 V3836：ret
 * ret（ncclResult_t；函数 sharedConnect 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = collNetConnect((void**)handlePtrs, nranks, rank, resources->collNetListenComms[netDev],
 * resources->collNetComms+netDev)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    ncclResult_t ret = collNetConnect((void**)handlePtrs, nranks, rank,
          resources->collNetListenComms[netDev],
          resources->collNetComms+netDev);
    free(handlePtrs);
    if (ret == ncclSuccess) {
      // Close listen comm
      NCCLCHECK(collNetCloseListen(resources->collNetListenComms[netDev]));
    } else {
      resources->collNetListenComms[netDev] = NULL;
    }
  }
  *collNetComm = resources->collNetComms[netDev];
  if (*collNetComm) resources->commRefCount[netDev]++;
  return ncclSuccess;
}

/* 中文逐项-函数 F1063：sharedFree
 * 用途与位置：减小共享 CollNet 连接引用，最后一个用户释放时才关闭网络集合对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 netDev（int netDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyFree；src/transport/coll_net.cc::recvProxyFree。
 * 本体直接出现的调用/包装：NCCLCHECK；collNetCloseColl；free。
 * 需要对照的边界/推进条件：resources->commRefCount[netDev] == 0；n<NCCL_MAX_NETDEVS；resources->commRefCount[n]。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sharedFree(struct ncclComm* comm, int netDev) {
/* 中文逐项-变量 V3837：resources
 * resources（struct sharedResources；函数 sharedFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct
 * sharedResources*)comm->proxyState.progressState.collNet.resources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct sharedResources* resources = (struct sharedResources*)comm->proxyState.progressState.collNet.resources;
  resources->commRefCount[netDev]--;
  if (resources->commRefCount[netDev] == 0) {
    NCCLCHECK(collNetCloseColl(resources->collNetComms[netDev]));
  }
  for (/* 中文逐项-变量 V3838：n
 * n（int；函数 sharedFree 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<NCCL_MAX_NETDEVS；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<NCCL_MAX_NETDEVS; n++) if (resources->commRefCount[n]) return ncclSuccess;
  comm->proxyState.progressState.collNet.resources = NULL;
  free(resources);
  return ncclSuccess;
}

/* 中文逐项-函数 F1064：sharedBuffersInit
 * 用途与位置：按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 cuda（int cuda）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 gpuPtr（char** gpuPtr）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 参数 cpuPtr（char** cpuPtr）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 参数 size（int* size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyConnect；src/transport/coll_net.cc::recvProxyConnect；src/transport/net.cc::proxySharedInit；src/transport/net.cc::sendProxyConnect；src/transport/net.cc::recvProxyConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCudaCalloc；ncclCudaHostCalloc。
 * 需要对照的边界/推进条件：state->size == 0；cuda && state->cudaBuff == NULL；!cuda && state->hostBuff == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sharedBuffersInit(struct ncclComm* comm, int cuda, char** gpuPtr, char** cpuPtr, int* size) {
/* 中文逐项-变量 V3839：state
 * state（struct ncclProxySharedCollNet；函数 sharedBuffersInit 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState.collNet；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxySharedCollNet* state = &comm->proxyState.progressState.collNet;
  if (state->size == 0) {
    state->size = 2*comm->nChannels*comm->buffSizes[NCCL_PROTO_SIMPLE];
  }

  *size = state->size;

  if (cuda && state->cudaBuff == NULL) {
    NCCLCHECK(ncclCudaCalloc(&state->cudaBuff, *size));
  }
  if (!cuda && state->hostBuff == NULL) {
    NCCLCHECK(ncclCudaHostCalloc(&state->hostBuff, *size));
  }
  *gpuPtr = *cpuPtr = cuda ? state->cudaBuff : state->hostBuff;
  return ncclSuccess;
}

/* 中文逐项-函数 F1065：sharedBuffersGet
 * 用途与位置：按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 slot（int slot）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 参数 channel（int channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 offset（int* offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyProgress；src/transport/coll_net.cc::recvProxyProgress；src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sharedBuffersGet(struct ncclComm* comm, int type, int slot, int channel, int* offset) {
  // Use different pools for different channels and also separate send/recv.
/* 中文逐项-变量 V3840：slotSize
 * slotSize（int；函数 sharedBuffersGet 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 slotSize = comm->buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int slotSize = comm->buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS;
/* 中文逐项-变量 V3841：globalSlot
 * globalSlot（int；函数 sharedBuffersGet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 globalSlot = (type*NCCL_STEPS+slot)*comm->nChannels+channel；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int globalSlot = (type*NCCL_STEPS+slot)*comm->nChannels+channel;
  *offset = slotSize * globalSlot;
  return ncclSuccess;
}

/* 中文逐项-函数 F1066：sharedBuffersDestroy
 * 用途与位置：降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyFree；src/transport/coll_net.cc::recvProxyFree；src/transport/net.cc::sendProxyFree；src/transport/net.cc::recvProxyFree。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaFree；ncclCudaHostFree。
 * 需要对照的边界/推进条件：state->size == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sharedBuffersDestroy(struct ncclComm* comm) {
/* 中文逐项-变量 V3842：state
 * state（struct ncclProxySharedCollNet；函数 sharedBuffersDestroy 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState.collNet；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxySharedCollNet* state = &comm->proxyState.progressState.collNet;
  if (state->size == 0) return ncclSuccess;
  CUDACHECK(cudaFree(state->cudaBuff));
  NCCLCHECK(ncclCudaHostFree(state->hostBuff));
  // This will be called multiple times, with multiple channels and send/recv. Make sure we only do it once.
  state->size = 0;
  return ncclSuccess;
}

/* 中文逐项-函数 F1067：recvProxySetup
 * 用途与位置：在实际代理进程中准备对应方向网络设备与连接资源；接收方向通常还要 listen 生成句柄。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc；sharedListen。
 * 需要对照的边界/推进条件：reqSize != sizeof (struct setupReq；respSize != sizeof(collNetHandle_t。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvProxySetup(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
/* 中文逐项-变量 V3843：req
 * req（struct setupReq；函数 recvProxySetup 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 初值/来源 req = (struct setupReq*)reqBuff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct setupReq* req = (struct setupReq*)reqBuff;
  if (reqSize != sizeof (struct setupReq)) return ncclInternalError;

/* 中文逐项-变量 V3844：resources
 * resources（struct recvResources；函数 recvProxySetup 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&resources, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct recvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  connection->transportResources = resources;
  connection->shared = 1;

  resources->netDev = req->netDev;
  resources->useGdr = req->useGdr;

/* 中文逐项-变量 V3845：netHandle
 * netHandle（collNetHandle_t；函数 recvProxySetup 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 netHandle = (collNetHandle_t*) respBuff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  collNetHandle_t* netHandle = (collNetHandle_t*) respBuff;
  if (respSize != sizeof(collNetHandle_t)) return ncclInternalError;

  NCCLCHECK(sharedListen(comm, req->netDev, netHandle));
  return ncclSuccess;
}

/* 中文逐项-函数 F1068：sendProxyConnect
 * 用途与位置：在代理侧异步完成后端 connect/accept、协议缓冲分配/共享/注册，并返回主线程需要的映射信息。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCL_NET_MAP_ADD_POINTER；NCCL_NET_MAP_GET_POINTER；collNetRegMr；ncclCudaHostCalloc；ncclGdrCudaCalloc；ncclParamGdrCopySyncEnable；sharedBuffersInit；sharedConnect。
 * 需要对照的边界/推进条件：reqSize != sizeof(struct collNetConnectArgs；p<NCCL_NUM_PROTOCOLS；respSize != sizeof(struct
 * connectMap*；resources->collNetComm == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendProxyConnect(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
  if (reqSize != sizeof(struct collNetConnectArgs)) { WARN("sendProxyConnect: reqSize is %d != %ld\n", reqSize, sizeof(struct collNetConnectArgs)); return ncclInternalError; }
/* 中文逐项-变量 V3846：args
 * args（struct collNetConnectArgs；函数 sendProxyConnect 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = (struct collNetConnectArgs*)reqBuff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct collNetConnectArgs* args = (struct collNetConnectArgs*)reqBuff;
/* 中文逐项-变量 V3847：info
 * info（struct collNetSendConnectInfo；函数 sendProxyConnect 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct collNetSendConnectInfo*)(args->connectInfos+args->rank)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct collNetSendConnectInfo* info = (struct collNetSendConnectInfo*)(args->connectInfos+args->rank);

/* 中文逐项-变量 V3848：resources
 * resources（struct sendResources；函数 sendProxyConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct sendResources*)(connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct sendResources* resources = (struct sendResources*)(connection->transportResources);

  // Get info from recv side
  resources->collNetRank = args->rank;
  resources->reqFifo = (struct reqSlot (*)[NCCL_STEPS])(info->reqFifo);

  for (/* 中文逐项-变量 V3849：p
 * p（int；函数 sendProxyConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    resources->recvMhandles[p] = info->mhandles[p];

  NCCLCHECK(sharedConnect(comm, resources->netDev, args->connectInfos, args->nranks, args->rank, &resources->collNetComm));

  // Collnet connect is allowed to fail. Gracefully handle that case by returning NULL to the caller.
  if (respSize != sizeof(struct connectMap*)) { WARN("sendProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*)); return ncclInternalError; }
  if (resources->collNetComm == NULL) {
    *((struct connectMap**)respBuff) = NULL;
    return ncclSuccess;
  }
  connection->proxyAppendPtr = comm->proxyState.progressState.collNet.proxyAppend+2*resources->netDev;

/* 中文逐项-变量 V3850：map
 * map（struct connectMap；函数 sendProxyConnect 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 map = &resources->map；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct connectMap* map = &resources->map;

  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);
  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);

  NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
  map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;
  if (ncclGdrCopy && ncclParamGdrCopySyncEnable()) {
/* 中文逐项-变量 V3851：cpuPtr、gpuPtr
 * cpuPtr（uint64_t；函数 sendProxyConnect 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources->gdrDesc));。若由输出参数填入，必须先检查填充调用成功再读值。
 * gpuPtr（uint64_t；函数 sendProxyConnect 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources->gdrDesc));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    uint64_t *cpuPtr, *gpuPtr;
    NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources->gdrDesc));

    resources->gdcSync = cpuPtr;
/* 中文逐项-变量 V3852：gdcMem
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
  resources->sendMem->head = -NCCL_STEPS;

  // Allocate & Register shared buffers for the Simple protocol
/* 中文逐项-变量 V3853：bank
 * bank（int；函数 sendProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM :
 * NCCL_NET_MAP_SHARED_HOSTMEM；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;
/* 中文逐项-变量 V3854：mapMem
 * mapMem（struct connectMapMem；函数 sendProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mapMem = map->mems+bank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMapMem* mapMem = map->mems+bank;
  NCCLCHECK(sharedBuffersInit(comm, resources->useGdr, &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size));
  NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);

  NCCLCHECK(collNetRegMr(resources->collNetComm, mapMem->cpuPtr, mapMem->size,
        resources->useGdr ? NCCL_PTR_CUDA : NCCL_PTR_HOST,
        &resources->sendMhandles[NCCL_PROTO_SIMPLE]));

  *((struct connectMap**)respBuff) = &resources->map;
  return ncclSuccess;
}

/* 中文逐项-函数 F1069：recvProxyConnect
 * 用途与位置：在代理侧异步完成后端 connect/accept、协议缓冲分配/共享/注册，并返回主线程需要的映射信息。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCL_NET_MAP_ADD_POINTER；NCCL_NET_MAP_GET_POINTER；collNetRegMr；ncclCudaHostCalloc；ncclGdrCudaCalloc；ncclParamGdrCopyFlushEnable；ncclParamGdrCopySyncEnable；sharedBuffersInit；sharedConnect。
 * 需要对照的边界/推进条件：reqSize != sizeof(struct collNetConnectArgs；respSize != sizeof(struct
 * connectMap*；resources->collNetComm == NULL；ncclGdrCopy。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvProxyConnect(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
  if (reqSize != sizeof(struct collNetConnectArgs)) { WARN("recvProxyConnect: reqSize is %d != %ld\n", reqSize, sizeof(struct collNetConnectArgs)); return ncclInternalError; }
/* 中文逐项-变量 V3855：args
 * args（struct collNetConnectArgs；函数 recvProxyConnect 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = (struct collNetConnectArgs*)reqBuff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct collNetConnectArgs* args = (struct collNetConnectArgs*)reqBuff;

/* 中文逐项-变量 V3856：resources
 * resources（struct recvResources；函数 recvProxyConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*)(connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct recvResources* resources = (struct recvResources*)(connection->transportResources);
/* 中文逐项-变量 V3857：info
 * info（struct collNetSendConnectInfo；函数 recvProxyConnect 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct collNetSendConnectInfo*)(args->connectInfos+args->rank)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct collNetSendConnectInfo* info = (struct collNetSendConnectInfo*)(args->connectInfos+args->rank);
  resources->collNetRank = args->rank;

  NCCLCHECK(sharedConnect(comm, resources->netDev, args->connectInfos, args->nranks, args->rank, &resources->collNetComm));

  // Collnet connect is allowed to fail. Gracefully handle that case by returning NULL to the caller.
  if (respSize != sizeof(struct connectMap*)) { WARN("sendProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*)); return ncclInternalError; }
  if (resources->collNetComm == NULL) {
    *((struct connectMap**)respBuff) = NULL;
    return ncclSuccess;
  }
  connection->proxyAppendPtr = comm->proxyState.progressState.collNet.proxyAppend+2*resources->netDev+1;

/* 中文逐项-变量 V3858：map
 * map（struct connectMap；函数 recvProxyConnect 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 map = &resources->map；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct connectMap* map = &resources->map;

  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);
  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);

  NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
  map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;
  if (ncclGdrCopy) {
/* 中文逐项-变量 V3859：cpuPtr、gpuPtr
 * cpuPtr（uint64_t；函数 recvProxyConnect 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources->gdrDesc));。若由输出参数填入，必须先检查填充调用成功再读值。
 * gpuPtr（uint64_t；函数 recvProxyConnect 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources->gdrDesc));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    uint64_t *cpuPtr, *gpuPtr;
    NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources->gdrDesc));

    if (ncclParamGdrCopySyncEnable()) {
      resources->gdcSync = cpuPtr;
/* 中文逐项-变量 V3860：gdcMem
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

  // Allocate & Register shared buffers for the Simple protocol
/* 中文逐项-变量 V3861：bank
 * bank（int；函数 recvProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM :
 * NCCL_NET_MAP_SHARED_HOSTMEM；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;
/* 中文逐项-变量 V3862：mapMem
 * mapMem（struct connectMapMem；函数 recvProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mapMem = map->mems+bank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMapMem* mapMem = map->mems+bank;
  NCCLCHECK(sharedBuffersInit(comm, resources->useGdr, &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size));
  NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);

  NCCLCHECK(collNetRegMr(resources->collNetComm, mapMem->cpuPtr, mapMem->size,
        resources->useGdr ? NCCL_PTR_CUDA : NCCL_PTR_HOST,
        &resources->mhandles[NCCL_PROTO_SIMPLE]));

  // Pass info to send side
  info->reqFifo = resources->reqFifo;
  for (/* 中文逐项-变量 V3863：p
 * p（int；函数 recvProxyConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    info->mhandles[p] = resources->mhandles[p];

  if (respSize != sizeof(struct connectMap*)) { WARN("recvProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*)); return ncclInternalError; }
  *((struct connectMap**)respBuff) = &resources->map;
  return ncclSuccess;
}

/* 中文逐项-函数 F1070：sendProxyFree
 * 用途与位置：在拥有网络对象的代理侧注销内存、释放映射并按引用计数关闭 backend connection。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；collNetDeregMr；cudaFree；free；ncclCudaHostFree；ncclGdrCudaFree；sharedBuffersDestroy；sharedFree。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS；resources->sendMhandles[p]；mems[NCCL_NET_MAP_GDCMEM].cpuPtr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendProxyFree(struct ncclProxyConnection* connection, struct ncclComm* comm) {
/* 中文逐项-变量 V3864：resources
 * resources（struct sendResources；函数 sendProxyFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct sendResources*)(connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct sendResources* resources = (struct sendResources*)(connection->transportResources);
  for (/* 中文逐项-变量 V3865：p
 * p（int；函数 sendProxyFree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (resources->sendMhandles[p]) {
      NCCLCHECK(collNetDeregMr(resources->collNetComm, resources->sendMhandles[p]));
    }
  }
/* 中文逐项-变量 V3866：mems
 * mems（struct connectMapMem；函数 sendProxyFree 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 mems = resources->map.mems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMapMem* mems = resources->map.mems;
  NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));
  CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));
  if (mems[NCCL_NET_MAP_GDCMEM].cpuPtr) NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));
  NCCLCHECK(sharedBuffersDestroy(comm));
  NCCLCHECK(sharedFree(comm, resources->netDev));
  free(connection->transportResources);
  return ncclSuccess;
}

/* 中文逐项-函数 F1071：recvProxyFree
 * 用途与位置：在拥有网络对象的代理侧注销内存、释放映射并按引用计数关闭 backend connection。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；collNetDeregMr；cudaFree；free；ncclCudaHostFree；ncclGdrCudaFree；sharedBuffersDestroy；sharedFree。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS；resources->mhandles[p]；mems[NCCL_NET_MAP_GDCMEM].cpuPtr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvProxyFree(struct ncclProxyConnection* connection, struct ncclComm* comm) {
/* 中文逐项-变量 V3867：resources
 * resources（struct recvResources；函数 recvProxyFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*)(connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct recvResources* resources = (struct recvResources*)(connection->transportResources);
  for (/* 中文逐项-变量 V3868：p
 * p（int；函数 recvProxyFree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (resources->mhandles[p]) {
      NCCLCHECK(collNetDeregMr(resources->collNetComm, resources->mhandles[p]));
    }
  }
/* 中文逐项-变量 V3869：mems
 * mems（struct connectMapMem；函数 recvProxyFree 内）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 * 初值/来源 mems = resources->map.mems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct connectMapMem* mems = resources->map.mems;
  NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));
  CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));
  if (mems[NCCL_NET_MAP_GDCMEM].cpuPtr) NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));
  NCCLCHECK(sharedBuffersDestroy(comm));
  NCCLCHECK(sharedFree(comm, resources->netDev));
  free(connection->transportResources);
  return ncclSuccess;
}


/* 中文逐项-宏 M0154：LAST_OF_GROUP
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 LAST_OF_GROUP；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define LAST_OF_GROUP(s) \
  (s % COLLNET_GROUP_NSUBS == COLLNET_GROUP_NSUBS-1 || s == args->nsubs-1)

/* 中文精读：CollNet Proxy 与普通点对点 NET 的区别
 * 普通 NET 提交 Isend/Irecv，CollNet 在 send 侧按准备好的槽位提交网络 Iallreduce 请求。
 * recv 侧协同请求 FIFO、完成检查和 flush，再把结果可读进度发布给 GPU。
 * 它需要插件承诺相同数据类型/归约操作语义；不是把任意一组 IB QP 自动变成硬件归约。
 * 资源由 sharedConnect/sharedBuffers 等机制管理，不能把普通 net_ib 的每连接公式直接套过来。
 */
/* 中文逐项-函数 F1072：sendProxyProgress
 * 用途与位置：对应后端发送状态机：准备槽位、等 GPU 数据就绪、提交网络请求、检测完成后回收；CollNet 分支提交网络归约。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 args（struct ncclProxyArgs* args）：代理聚合运行态，保存各连接子任务的posted/received/transmitted/done进度和回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：DIVUP；LAST_OF_GROUP；NCCLCHECK；NCCL_NET_MAP_GET_POINTER；ROUNDUP；__sync_synchronize；collNetIallreduce；collNetTest；ncclTypeSize；sharedBuffersGet；wc_store_fence。
 * 需要对照的边界/推进条件：args->protocol != NCCL_PROTO_SIMPLE；args->state == ncclProxyOpReady；s<args->nsubs；args->state ==
 * ncclProxyOpProgress。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t sendProxyProgress(struct ncclComm* comm, struct ncclProxyArgs* args) {
  if (args->protocol != NCCL_PROTO_SIMPLE) {
    WARN("CollNet does not support LL/LL128");
    return ncclInternalError;
  }
  if (args->state == ncclProxyOpReady) {
    for (/* 中文逐项-变量 V3870：s
 * s（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s++) {
/* 中文逐项-变量 V3871：sub
 * sub（struct ncclProxySubArgs；函数 sendProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* sub = args->subs+s;
/* 中文逐项-变量 V3872：resources
 * resources（struct sendResources；函数 sendProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct sendResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);
      // Round to next multiple of sliceSteps
      sub->base = ROUNDUP(resources->step, args->chunkSteps);
      sub->posted = sub->received = sub->transmitted = sub->done = 0;
      resources->step = sub->base + sub->nsteps;
    }
    args->state = ncclProxyOpProgress;
  }
  args->idle = 1;
  if (args->state == ncclProxyOpProgress) {
/* 中文逐项-变量 V3873：p
 * p（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = args->protocol；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int p = args->protocol;
/* 中文逐项-变量 V3874：nGroups
 * nGroups（int；函数 sendProxyProgress 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nGroups = DIVUP(args->nsubs, COLLNET_GROUP_NSUBS)；向上取整分配，确保不能整除时也覆盖最后不足一块的数据。
 */
    int nGroups = DIVUP(args->nsubs, COLLNET_GROUP_NSUBS);
/* 中文逐项-变量 V3875：perGroupSteps
 * perGroupSteps（int；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 perGroupSteps = NCCL_STEPS / nGroups；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int perGroupSteps = NCCL_STEPS / nGroups;
    for (/* 中文逐项-变量 V3876：s
 * s（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s++) {
/* 中文逐项-变量 V3877：sub
 * sub（struct ncclProxySubArgs；函数 sendProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* sub = args->subs+s;
/* 中文逐项-变量 V3878：resources
 * resources（struct sendResources；函数 sendProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct sendResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);
/* 中文逐项-变量 V3879：sendMhandle
 * sendMhandle（void；函数 sendProxyProgress 内）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 初值/来源 sendMhandle = resources->sendMhandles[p]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void* sendMhandle = resources->sendMhandles[p];
/* 中文逐项-变量 V3880：recvMhandle
 * recvMhandle（void；函数 sendProxyProgress 内）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 初值/来源 recvMhandle = resources->recvMhandles[p]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void* recvMhandle = resources->recvMhandles[p];
/* 中文逐项-变量 V3881：reqFifo
 * reqFifo（auto；函数 sendProxyProgress 内）：CollNet发送/接收侧协作使用的网络请求FIFO，协调共享集合归约结果。
 * 初值/来源 reqFifo = resources->reqFifo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      auto reqFifo = resources->reqFifo;
      if (sub->posted < sub->nsteps && sub->posted < sub->done + NCCL_STEPS) {
/* 中文逐项-变量 V3882：buffSlot
 * buffSlot（int；函数 sendProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->posted)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
/* 中文逐项-变量 V3883：sharedBuffSlot
 * sharedBuffSlot（int；函数 sendProxyProgress 内）：共享数据池中的在途槽号，与独立连接8槽的绝对 step/索引分开。
 * 初值/来源 sharedBuffSlot = sub->posted%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int sharedBuffSlot = sub->posted%NCCL_STEPS;
/* 中文逐项-变量 V3884：offset
 * offset（int；函数 sendProxyProgress 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(sharedBuffersGet(comm, 0, sharedBuffSlot, 0, &offset));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int offset;
        NCCLCHECK(sharedBuffersGet(comm, 0, sharedBuffSlot, 0, &offset));
        resources->recvMem->offsFifo[buffSlot] = offset + s*args->chunkSize;
        __sync_synchronize();
/* 中文逐项-变量 V3885：sendHead
 * sendHead（uint64_t；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendHead = resources->gdcSync ? resources->gdcSync :
 * &resources->sendMem->head；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        volatile uint64_t* sendHead = resources->gdcSync ? resources->gdcSync : &resources->sendMem->head;
        sub->posted += args->sliceSteps;
        *sendHead = sub->base + sub->posted - NCCL_STEPS;
        if (resources->gdcSync) wc_store_fence(); // Flush out WC write
      }
      // Enforce sync between operations of the same group.
/* 中文逐项-变量 V3886：groupSync
 * groupSync（bool；函数 sendProxyProgress 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 groupSync = (((s == 0) && ((sub+args->nsubs-1)->received == sub->received)) || (s && (sub-1)->received >
 * sub->received))；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      bool groupSync = (((s == 0) && ((sub+args->nsubs-1)->received == sub->received)) || (s && (sub-1)->received > sub->received));
      if (groupSync && sub->received < sub->posted && sub->received < sub->done + perGroupSteps) {
/* 中文逐项-变量 V3887：buffSlot
 * buffSlot（int；函数 sendProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->received)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->received)%NCCL_STEPS;
/* 中文逐项-变量 V3888：sharedBuffSlot
 * sharedBuffSlot（int；函数 sendProxyProgress 内）：共享数据池中的在途槽号，与独立连接8槽的绝对 step/索引分开。
 * 初值/来源 sharedBuffSlot = sub->received%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int sharedBuffSlot = sub->received%NCCL_STEPS;
/* 中文逐项-变量 V3889：sizesFifo
 * sizesFifo（int；函数 sendProxyProgress 内）：GPU 向代理报告各数据槽有效物理字节数的控制数组，空/未就绪值需按后端约定处理。
 * 初值/来源 sizesFifo = resources->recvMem->sizesFifo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        volatile int* sizesFifo = resources->recvMem->sizesFifo;
/* 中文逐项-变量 V3890：recvTail
 * recvTail（uint64_t；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvTail = &resources->recvMem->tail；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
        volatile uint64_t* recvTail = &resources->recvMem->tail;
/* 中文逐项-变量 V3891：localBuff
 * localBuff（char；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, gpu, buffs[p])；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, gpu, buffs[p]);
        if (sizesFifo[buffSlot] != -1 && ((*recvTail > (sub->base+sub->received)))) {
          // We have something to receive, let's check whether data is ready.
/* 中文逐项-变量 V3892：ready
 * ready（int；函数 sendProxyProgress 内）：握手/连接是否完整就绪的状态，未就绪时须继续推进控制握手。
 * 初值/来源 ready = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          int ready = 1;
          if (s == 0) {
/* 中文逐项-变量 V3893：offset
 * offset（int；函数 sendProxyProgress 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(sharedBuffersGet(comm, 0, sharedBuffSlot, 0, &offset));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            int offset;
            NCCLCHECK(sharedBuffersGet(comm, 0, sharedBuffSlot, 0, &offset));
            args->sharedBuff[sharedBuffSlot] = localBuff + offset;
            args->sharedSize[sharedBuffSlot] = args->chunkSize;
          }
          if (ready) {
            sizesFifo[buffSlot] = -1;
            sub->received += args->sliceSteps;
            args->idle = 0;
            //continue;
          }
        }
      }
      if (LAST_OF_GROUP(s) && (sub->transmitted < sub->received)) {
/* 中文逐项-变量 V3894：group
 * group（int；函数 sendProxyProgress 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = s / COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int group = s / COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3895：buffSlot
 * buffSlot（int；函数 sendProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->transmitted)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->transmitted)%NCCL_STEPS;
/* 中文逐项-变量 V3896：sharedBuffSlot
 * sharedBuffSlot（int；函数 sendProxyProgress 内）：共享数据池中的在途槽号，与独立连接8槽的绝对 step/索引分开。
 * 初值/来源 sharedBuffSlot = sub->transmitted%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int sharedBuffSlot = sub->transmitted%NCCL_STEPS;
        if (reqFifo[group][buffSlot].recvBuff != NULL) {
/* 中文逐项-变量 V3897：totalSize
 * totalSize（int；函数 sendProxyProgress 内）：排队有效数据规模统计，供负载平衡或聚合选型使用。
 * 初值/来源 totalSize = (s-group*COLLNET_GROUP_NSUBS+1) *
 * args->sharedSize[sharedBuffSlot]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          int totalSize = (s-group*COLLNET_GROUP_NSUBS+1) * args->sharedSize[sharedBuffSlot];
/* 中文逐项-变量 V3898：count
 * count（int；函数 sendProxyProgress 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 初值/来源 count = totalSize / ncclTypeSize(args->dtype)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          int count = totalSize / ncclTypeSize(args->dtype);
          reqFifo[group][buffSlot].size = args->sharedSize[sharedBuffSlot];
/* 中文逐项-变量 V3899：sendAddress
 * sendAddress（char；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendAddress = (char*)args->sharedBuff[sharedBuffSlot] +
 * group*COLLNET_GROUP_NSUBS*args->sharedSize[sharedBuffSlot]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          char* sendAddress = (char*)args->sharedBuff[sharedBuffSlot] + group*COLLNET_GROUP_NSUBS*args->sharedSize[sharedBuffSlot];
          NCCLCHECK(collNetIallreduce(resources->collNetComm, sendAddress, (void*)(reqFifo[group][buffSlot].recvBuff), count, args->dtype, args->redOp, sendMhandle, recvMhandle, sub->requests+buffSlot));
          if (sub->requests[buffSlot] == NULL) continue;

          TRACE(NCCL_NET, "sendProxy [%d/%d/%d] Iallreduce posted, size %d req %p", sub->transmitted, group, buffSlot, totalSize, sub->requests[buffSlot]);
          // Make sure size is reset to zero before we update the head.
          __sync_synchronize();
          sub->transmitted += args->sliceSteps;
          args->idle = 0;
          continue;
        }
      }
      // Check whether the network has completed some send operations.
      if (LAST_OF_GROUP(s) && sub->done < sub->transmitted) {
/* 中文逐项-变量 V3900：done、size
 * done（int；函数 sendProxyProgress 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 此处未显式给初值；后续首个文本使用点：int buffSlot = (sub->base+sub->done)%NCCL_STEPS;。若由输出参数填入，必须先检查填充调用成功再读值。
 * size（int；函数 sendProxyProgress 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(collNetTest((void*)(sub->requests[buffSlot]), &done, &size));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int done, size;
/* 中文逐项-变量 V3901：group
 * group（int；函数 sendProxyProgress 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = s / COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int group = s / COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3902：buffSlot
 * buffSlot（int；函数 sendProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->done)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->done)%NCCL_STEPS;
        NCCLCHECK(collNetTest((void*)(sub->requests[buffSlot]), &done, &size));
        if (done) {
          TRACE(NCCL_NET, "sendProxy [%d/%d/%d] request %p done, size %d", sub->done, group, buffSlot, sub->requests[buffSlot], size);
          // Make sure size is updated before we set recvBuff to NULL (from the view of recv proxy, concerning the flush)
          // (reordered store after store is possible on POWER, though not on x86)
          __sync_synchronize();
          reqFifo[group][buffSlot].recvBuff = NULL; // Notify recvProxy
          for (/* 中文逐项-变量 V3903：i
 * i（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = group*COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 * 所在for循环的继续条件：i<=s；循环体与增量表达式决定每轮如何推进。
 */int i=group*COLLNET_GROUP_NSUBS; i<=s; i++) args->subs[i].done += args->sliceSteps;
          args->idle = 0;
/* 中文逐项-变量 V3904：allDone
 * allDone（int；函数 sendProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 allDone = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          int allDone = 1;
          for (/* 中文逐项-变量 V3905：i
 * i（int；函数 sendProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<args->nsubs; i++) {
            if (args->subs[i].done < args->subs[i].nsteps) { allDone = 0; break; }
          }
          if (allDone) {
            args->state = ncclProxyOpNone;
            TRACE(NCCL_NET, "sendProxy [%d/%d] stopped", sub->done, s);
          }
        }
      }
    }
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F1073：recvProxyProgress
 * 用途与位置：对应后端接收状态机：挂接收、检查网络完成、必要时 flush、发布给 GPU 并等待消费回收。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 args（struct ncclProxyArgs* args）：代理聚合运行态，保存各连接子任务的posted/received/transmitted/done进度和回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：DIVUP；LAST_OF_GROUP；NCCLCHECK；NCCL_NET_MAP_GET_POINTER；ROUNDUP；__sync_synchronize；collNetIflush；collNetTest；sharedBuffersGet；wc_store_fence。
 * 需要对照的边界/推进条件：args->protocol != NCCL_PROTO_SIMPLE；args->state == ncclProxyOpReady；s<args->nsubs；args->state ==
 * ncclProxyOpProgress。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t recvProxyProgress(struct ncclComm* comm, struct ncclProxyArgs* args) {
  if (args->protocol != NCCL_PROTO_SIMPLE) {
    WARN("CollNet does not support LL/LL128");
    return ncclInternalError;
  }
  if (args->state == ncclProxyOpReady) {
    for (/* 中文逐项-变量 V3906：s
 * s（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s++) {
/* 中文逐项-变量 V3907：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* sub = args->subs+s;
/* 中文逐项-变量 V3908：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
      // Round to next multiple of sliceSteps
      sub->base = ROUNDUP(resources->step, args->chunkSteps);
      sub->posted = sub->received = sub->flushed = sub->transmitted = sub->done = 0;
      resources->step = sub->base + sub->nsteps;
    }
    args->state = ncclProxyOpProgress;
  }
  args->idle = 1;
  if (args->state == ncclProxyOpProgress) {
/* 中文逐项-变量 V3909：p
 * p（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = args->protocol；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int p = args->protocol;
/* 中文逐项-变量 V3910：nGroups
 * nGroups（int；函数 recvProxyProgress 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nGroups = DIVUP(args->nsubs, COLLNET_GROUP_NSUBS)；向上取整分配，确保不能整除时也覆盖最后不足一块的数据。
 */
    int nGroups = DIVUP(args->nsubs, COLLNET_GROUP_NSUBS);
/* 中文逐项-变量 V3911：perGroupSteps
 * perGroupSteps（int；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 perGroupSteps = NCCL_STEPS / nGroups；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int perGroupSteps = NCCL_STEPS / nGroups;
    for (/* 中文逐项-变量 V3912：s
 * s（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<args->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<args->nsubs; s++) {
/* 中文逐项-变量 V3913：sub
 * sub（struct ncclProxySubArgs；函数 recvProxyProgress 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = args->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxySubArgs* sub = args->subs+s;
/* 中文逐项-变量 V3914：resources
 * resources（struct recvResources；函数 recvProxyProgress 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct recvResources*) (sub->connection->transportResources)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
/* 中文逐项-变量 V3915：mhandle
 * mhandle（void；函数 recvProxyProgress 内）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 初值/来源 mhandle = resources->mhandles[p]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      void* mhandle = resources->mhandles[p];
/* 中文逐项-变量 V3916：reqFifo
 * reqFifo（auto；函数 recvProxyProgress 内）：CollNet发送/接收侧协作使用的网络请求FIFO，协调共享集合归约结果。
 * 初值/来源 reqFifo = resources->reqFifo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      auto reqFifo = resources->reqFifo;
/* 中文逐项-变量 V3917：localBuff
 * localBuff（char；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p])；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);

      // Enforce sync between operations of the same group.
      if (LAST_OF_GROUP(s) && (sub->posted < sub->done + perGroupSteps) && (sub->posted < sub->nsteps)) {
/* 中文逐项-变量 V3918：group
 * group（int；函数 recvProxyProgress 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = s / COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int group = s / COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3919：buffSlot
 * buffSlot（int；函数 recvProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->posted)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
/* 中文逐项-变量 V3920：sharedBuffSlot
 * sharedBuffSlot（int；函数 recvProxyProgress 内）：共享数据池中的在途槽号，与独立连接8槽的绝对 step/索引分开。
 * 初值/来源 sharedBuffSlot = sub->posted%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int sharedBuffSlot = sub->posted%NCCL_STEPS;
/* 中文逐项-变量 V3921：startChannel
 * startChannel（int；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 startChannel = group*COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int startChannel = group*COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3922：offset
 * offset（int；函数 recvProxyProgress 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel,
 * &offset));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int offset;
        NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));
        reqFifo[group][buffSlot].recvBuff = localBuff + offset;
        TRACE(NCCL_NET, "recvProxy [%d/%d/%d] posted buffer %p", sub->posted, group, buffSlot, reqFifo[group][buffSlot].recvBuff);
        sub->posted += args->sliceSteps;
        args->idle = 0;
        continue;
      }
      if (LAST_OF_GROUP(s) && (sub->posted > sub->received)) {
/* 中文逐项-变量 V3923：group
 * group（int；函数 recvProxyProgress 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = s / COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int group = s / COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3924：buffSlot
 * buffSlot（int；函数 recvProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base+sub->received)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base+sub->received)%NCCL_STEPS;
/* 中文逐项-变量 V3925：sharedBuffSlot
 * sharedBuffSlot（int；函数 recvProxyProgress 内）：共享数据池中的在途槽号，与独立连接8槽的绝对 step/索引分开。
 * 初值/来源 sharedBuffSlot = sub->received%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int sharedBuffSlot = sub->received%NCCL_STEPS;
        if (reqFifo[group][buffSlot].recvBuff == NULL) { // Buffer is cleared : coll is complete
          args->sharedSize[sharedBuffSlot] = reqFifo[group][buffSlot].size;
/* 中文逐项-变量 V3926：totalSize
 * totalSize（int；函数 recvProxyProgress 内）：排队有效数据规模统计，供负载平衡或聚合选型使用。
 * 初值/来源 totalSize =
 * args->sharedSize[sharedBuffSlot]*(s-group*COLLNET_GROUP_NSUBS+1)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          int totalSize = args->sharedSize[sharedBuffSlot]*(s-group*COLLNET_GROUP_NSUBS+1);
          TRACE(NCCL_NET, "recvProxy [%d/%d/%d] received, size %d", sub->received, group, buffSlot, totalSize);
          sub->received += args->sliceSteps;
          sub->requests[buffSlot] = NULL;
          if (reqFifo[group][buffSlot].size > 0 && resources->useGdr) {
            // GDRCOPY support
            if (resources->gdcFlush) {
#if defined (__x86_64__)
              // Force a PCI-E read from GPU memory
              asm volatile ("mov (%0), %%eax" :: "l"(resources->gdcFlush) : "%eax");
#else
              WARN("NET: GDR Flush only supported on x86_64");
              return ncclInternalError;
#endif
              sub->requests[buffSlot] = NULL;
            } else {
/* 中文逐项-变量 V3927：startChannel
 * startChannel（int；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 startChannel = group*COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
              int startChannel = group*COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3928：offset
 * offset（int；函数 recvProxyProgress 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel,
 * &offset));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
              int offset;
              NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));
              NCCLCHECK(collNetIflush(resources->collNetComm, localBuff + offset, totalSize, mhandle, sub->requests+buffSlot));
            }
          } else {
            for (/* 中文逐项-变量 V3929：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = group*COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 * 所在for循环的继续条件：i<=s；循环体与增量表达式决定每轮如何推进。
 */int i=group*COLLNET_GROUP_NSUBS; i<=s; i++) args->subs[i].flushed += args->sliceSteps;
          }
          args->idle = 0;
          continue;
        }
      }
      if (LAST_OF_GROUP(s) && (sub->received > sub->flushed)) {
        // Progress flush operations
/* 中文逐项-变量 V3930：group
 * group（int；函数 recvProxyProgress 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = s / COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int group = s / COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3931：buffSlot
 * buffSlot（int；函数 recvProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base + sub->flushed)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base + sub->flushed)%NCCL_STEPS;
/* 中文逐项-变量 V3932：done
 * done（int；函数 recvProxyProgress 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 初值/来源 done = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        int done = 1;
        if (sub->requests[buffSlot]) NCCLCHECK(collNetTest(sub->requests[buffSlot], &done, NULL));
        if (done) {
          TRACE(NCCL_NET, "recvProxy [%d/%d/%d] flushed", sub->flushed, group, buffSlot);
          for (/* 中文逐项-变量 V3933：i
 * i（int；函数 recvProxyProgress 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = group*COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 * 所在for循环的继续条件：i<=s；循环体与增量表达式决定每轮如何推进。
 */int i=group*COLLNET_GROUP_NSUBS; i<=s; i++) args->subs[i].flushed += args->sliceSteps;
          args->idle = 0;
          //continue;
        }
      }
      if (sub->flushed > sub->transmitted) {
/* 中文逐项-变量 V3934：group
 * group（int；函数 recvProxyProgress 内）：线程组/命名屏障及共享指针表索引；Simple 构造参数高16位还可携带 connIndex。
 * 初值/来源 group = s / COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int group = s / COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3935：buffSlot
 * buffSlot（int；函数 recvProxyProgress 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 buffSlot = (sub->base + sub->transmitted)%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int buffSlot = (sub->base + sub->transmitted)%NCCL_STEPS;
/* 中文逐项-变量 V3936：sharedBuffSlot
 * sharedBuffSlot（int；函数 recvProxyProgress 内）：共享数据池中的在途槽号，与独立连接8槽的绝对 step/索引分开。
 * 初值/来源 sharedBuffSlot = sub->transmitted%NCCL_STEPS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
        int sharedBuffSlot = sub->transmitted%NCCL_STEPS;
/* 中文逐项-变量 V3937：startChannel
 * startChannel（int；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 startChannel = group*COLLNET_GROUP_NSUBS；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
        int startChannel = group*COLLNET_GROUP_NSUBS;
/* 中文逐项-变量 V3938：offset
 * offset（int；函数 recvProxyProgress 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel,
 * &offset));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int offset;
        NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));
/* 中文逐项-变量 V3939：offsFifo
 * offsFifo（int；函数 recvProxyProgress 内）：共享数据池每槽字节偏移，代理先提供地址，GPU 再按偏移读写本次槽位。
 * 初值/来源 offsFifo = (volatile int*)resources->recvMem->offsFifo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        volatile int* offsFifo = (volatile int*)resources->recvMem->offsFifo;
        offsFifo[buffSlot] = offset + (s%COLLNET_GROUP_NSUBS)*args->chunkSize;
        __sync_synchronize();
/* 中文逐项-变量 V3940：recvTail
 * recvTail（uint64_t；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvTail = resources->gdcSync ? resources->gdcSync :
 * &resources->recvMem->tail；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        volatile uint64_t* recvTail = resources->gdcSync ? resources->gdcSync : &resources->recvMem->tail;
        *recvTail = sub->base + sub->flushed;
        if (resources->gdcSync) wc_store_fence(); // Flush out WC write
        sub->transmitted += args->sliceSteps;
        args->idle = 0;
        continue;
      }
      // Enforce sync here to make sure the last sub doesn't increase "done" before all others in the group have
      // reached the same point, otherwise we would start posting buffers to the send proxy before we're done
      // processing all the shared buffer.
/* 中文逐项-变量 V3941：groupSync
 * groupSync（bool；函数 recvProxyProgress 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 groupSync = (((s == 0) && ((sub+args->nsubs-1)->done == sub->done)) || (s && (sub-1)->done >
 * sub->done))；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      bool groupSync = (((s == 0) && ((sub+args->nsubs-1)->done == sub->done)) || (s && (sub-1)->done > sub->done));
/* 中文逐项-变量 V3942：sendHead
 * sendHead（uint64_t；函数 recvProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendHead = &resources->sendMem->head；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
      volatile uint64_t* sendHead = &resources->sendMem->head;
      if (groupSync && sub->done < sub->transmitted && (sub->base+sub->done) < *sendHead) {
        sub->done += args->sliceSteps;
        args->idle = 0;
        if (sub->done == sub->nsteps && s == args->nsubs-1) {
          args->state = ncclProxyOpNone;
          TRACE(NCCL_NET, "recvProxy [%d/%d] stopped", sub->done, s);
        }
      }
    }
  }
  return ncclSuccess;
}

/* 中文导读：
 * CollNet transport 通过集合网络插件推进卸载通信，与普通 NET 的逐对 send/recv 连接模型不同。
 * 其底层是否使用 SHARP、QP 如何复用由实际插件决定，不能套用 net_ib.cc 的 RC 连接公式。
 */
/* 中文逐项-变量 V3943：collNetTransport
 * collNetTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNetTransport = { "COL", canConnect, { sendSetup, sendConnect, sendFree, NULL, sendProxySetup,
 * sendProxyConnect, sendProxyFree, sendProxyProgress }, { recvSetup, recvConnect, recvFree, NULL, recvProxySetup,
 * recvProxyConnect, recvProxyFree, recvPr …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct ncclTransport collNetTransport = {
  "COL",
  canConnect,
  { sendSetup, sendConnect, sendFree, NULL, sendProxySetup, sendProxyConnect, sendProxyFree, sendProxyProgress },
  { recvSetup, recvConnect, recvFree, NULL, recvProxySetup, recvProxyConnect, recvProxyFree, recvProxyProgress }
};
