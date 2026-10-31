/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_TOPO_H_
/* 中文逐项-常量 X0045：NCCL_TOPO_H_
 * 编译期定义：NCCL_TOPO_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_H_

#include "graph.h"
#include "core.h"

/* 中文逐项-常量 X0046：LOC_WIDTH
 * 编译期定义：LOC_WIDTH = 5000.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define LOC_WIDTH 5000.0
/* 中文逐项-常量 X0047：SM60_NVLINK_WIDTH
 * 编译期定义：SM60_NVLINK_WIDTH = 18.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define SM60_NVLINK_WIDTH 18.0
/* 中文逐项-常量 X0048：SM70_NVLINK_WIDTH
 * 编译期定义：SM70_NVLINK_WIDTH = 22.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define SM70_NVLINK_WIDTH 22.0
/* 中文逐项-常量 X0049：SM80_NVLINK_WIDTH
 * 编译期定义：SM80_NVLINK_WIDTH = 22.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define SM80_NVLINK_WIDTH 22.0
/* 中文逐项-常量 X0050：SM86_NVLINK_WIDTH
 * 编译期定义：SM86_NVLINK_WIDTH = 12.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define SM86_NVLINK_WIDTH 12.0
/* 中文逐项-常量 X0051：PCI_WIDTH
 * 编译期定义：PCI_WIDTH = 12.0 // PCI Gen3 x16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PCI_WIDTH 12.0           // PCI Gen3 x16
/* 中文逐项-常量 X0052：QPI_WIDTH
 * 编译期定义：QPI_WIDTH = 6.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define QPI_WIDTH 6.0
/* 中文逐项-常量 X0053：SKL_QPI_WIDTH
 * 编译期定义：SKL_QPI_WIDTH = 9.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define SKL_QPI_WIDTH 9.0
/* 中文逐项-常量 X0054：ZPI_WIDTH
 * 编译期定义：ZPI_WIDTH = 6.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ZPI_WIDTH 6.0
/* 中文逐项-常量 X0055：YONGFENG_ZPI_WIDTH
 * 编译期定义：YONGFENG_ZPI_WIDTH = 9.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define YONGFENG_ZPI_WIDTH 9.0
/* 中文逐项-常量 X0056：P9_WIDTH
 * 编译期定义：P9_WIDTH = 32.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define P9_WIDTH 32.0
/* 中文逐项-常量 X0057：ARM_WIDTH
 * 编译期定义：ARM_WIDTH = 6.0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ARM_WIDTH 6.0
/* 中文逐项-常量 X0058：NET_WIDTH
 * 编译期定义：NET_WIDTH = 12.0 // 100Gbit。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NET_WIDTH 12.0           // 100Gbit

// Intel CPU convert GPU P2P traffic into 64B PCI TLPs, so GPU
// to GPU traffic consumes more PCI bandwidth.
/* 中文逐项-宏 M0045：INTEL_P2P_OVERHEAD
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 INTEL_P2P_OVERHEAD；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define INTEL_P2P_OVERHEAD(speed) (speed*6/5)

/* 中文逐项-常量 X0059：NCCL_TOPO_NODE_TYPES
 * 编译期定义：NCCL_TOPO_NODE_TYPES = 7。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_NODE_TYPES 7
/* 中文逐项-常量 X0060：GPU
 * 编译期定义：GPU = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define GPU 0
/* 中文逐项-常量 X0061：PCI
 * 编译期定义：PCI = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PCI 1
/* 中文逐项-常量 X0062：NVS
 * 编译期定义：NVS = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVS 2
/* 中文逐项-常量 X0063：CPU
 * 编译期定义：CPU = 3 // Actually NUMA domains。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define CPU 3 // Actually NUMA domains
/* 中文逐项-常量 X0064：NIC
 * 编译期定义：NIC = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NIC 4
/* 中文逐项-常量 X0065：NET
 * 编译期定义：NET = 5。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NET 5
/* 中文逐项-变量 V1600：topoNodeTypeStr
 * topoNodeTypeStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern const char* topoNodeTypeStr[];

// We want link types and path types to match as much as possible
/* 中文逐项-常量 X0066：LINK_LOC
 * 编译期定义：LINK_LOC = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define LINK_LOC 0
/* 中文逐项-常量 X0067：LINK_NVL
 * 编译期定义：LINK_NVL = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define LINK_NVL 1
// Skipping 2 for PATH_NVB
/* 中文逐项-常量 X0068：LINK_PCI
 * 编译期定义：LINK_PCI = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define LINK_PCI 3
// Skipping 4 for PATH_PXB
// Skipping 5 for PATH_PXN
// Skipping 6 for PATH_PHB
/* 中文逐项-常量 X0069：LINK_SYS
 * 编译期定义：LINK_SYS = 7。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define LINK_SYS 7
/* 中文逐项-常量 X0070：LINK_NET
 * 编译期定义：LINK_NET = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define LINK_NET 8
/* 中文逐项-变量 V1601：topoLinkTypeStr
 * topoLinkTypeStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern const char* topoLinkTypeStr[];

// Local (myself)
/* 中文逐项-常量 X0071：PATH_LOC
 * 编译期定义：PATH_LOC = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_LOC 0

// Connection traversing NVLink
/* 中文逐项-常量 X0072：PATH_NVL
 * 编译期定义：PATH_NVL = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_NVL 1

// Connection through NVLink using an intermediate GPU
/* 中文逐项-常量 X0073：PATH_NVB
 * 编译期定义：PATH_NVB = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_NVB 2

// Connection traversing at most a single PCIe bridge
/* 中文逐项-常量 X0074：PATH_PIX
 * 编译期定义：PATH_PIX = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_PIX 3

// Connection traversing multiple PCIe bridges (without traversing the PCIe Host Bridge)
/* 中文逐项-常量 X0075：PATH_PXB
 * 编译期定义：PATH_PXB = 4。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_PXB 4

// Connection between a GPU and a NIC using an intermediate GPU. Used to enable rail-local, aggregated network send/recv operations.
/* 中文逐项-常量 X0076：PATH_PXN
 * 编译期定义：PATH_PXN = 5。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_PXN 5

// Connection traversing PCIe as well as a PCIe Host Bridge (typically the CPU)
/* 中文逐项-常量 X0077：PATH_PHB
 * 编译期定义：PATH_PHB = 6。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_PHB 6

// Connection traversing PCIe as well as the SMP interconnect between NUMA nodes (e.g., QPI/UPI)
/* 中文逐项-常量 X0078：PATH_SYS
 * 编译期定义：PATH_SYS = 7。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_SYS 7
/* 中文逐项-常量 X0079：PATH_DIS
 * 编译期定义：PATH_DIS = 7。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PATH_DIS 7
/* 中文逐项-变量 V1602：topoPathTypeStr
 * topoPathTypeStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern const char* topoPathTypeStr[];

struct ncclTopoNode;
/* 中文逐项-类型 T0138：ncclTopoLink
 * 类型职责：硬件直接链路及一条多跳路径的链路列表/跳数/瓶颈带宽/路径类别。
 * 声明内可见成员（含嵌套成员）：type、width、remNode。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTopoLink {
/* 中文逐项-变量 V1603：type
 * type（int；成员 ncclTopoLink）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  int type;
/* 中文逐项-变量 V1604：width
 * width（float；成员 ncclTopoLink）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 */
  float width;
/* 中文逐项-变量 V1605：remNode
 * remNode（struct ncclTopoNode；成员 ncclTopoLink）：硬件边指向的另一个节点，路径搜索沿此指针扩展。
 */
  struct ncclTopoNode* remNode;
};
/* 中文逐项-常量 X0080：NCCL_TOPO_MAX_LINKS
 * 编译期定义：NCCL_TOPO_MAX_LINKS = 32。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_MAX_LINKS 32
/* 中文逐项-常量 X0081：NCCL_TOPO_MAX_HOPS
 * 编译期定义：NCCL_TOPO_MAX_HOPS = (NCCL_TOPO_MAX_NODES*NCCL_TOPO_NODE_TYPES)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_MAX_HOPS (NCCL_TOPO_MAX_NODES*NCCL_TOPO_NODE_TYPES)

/* 中文逐项-类型 T0139：ncclTopoLinkList
 * 类型职责：硬件直接链路及一条多跳路径的链路列表/跳数/瓶颈带宽/路径类别。
 * 声明内可见成员（含嵌套成员）：list、count、width、type。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTopoLinkList {
/* 中文逐项-变量 V1606：list
 * list（struct ncclTopoLink；成员 ncclTopoLinkList）：节点或链路指针列表，count给出有效条目数，遍历不得越过当前有效范围。
 */
  struct ncclTopoLink* list[NCCL_TOPO_MAX_HOPS];
/* 中文逐项-变量 V1607：count
 * count（int；成员 ncclTopoLinkList）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 */
  int count;
/* 中文逐项-变量 V1608：width
 * width（float；成员 ncclTopoLinkList）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 */
  float width;
/* 中文逐项-变量 V1609：type
 * type（int；成员 ncclTopoLinkList）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  int type;
};

/* 中文逐项-常量 X0082：NCCL_TOPO_CPU_INTEL_BDW
 * 编译期定义：NCCL_TOPO_CPU_INTEL_BDW = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_INTEL_BDW 1
/* 中文逐项-常量 X0083：NCCL_TOPO_CPU_INTEL_SKL
 * 编译期定义：NCCL_TOPO_CPU_INTEL_SKL = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_INTEL_SKL 2

/* 中文逐项-常量 X0084：NCCL_TOPO_UNDEF
 * 编译期定义：NCCL_TOPO_UNDEF = (-1)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_UNDEF (-1)

/* 中文精读：硬件节点、边与缓存路径
 * type/id 决定这是 CPU、PCI、GPU、NET 等哪类硬件对象；union 中保存该类型的专有属性。
 * links 记录直接邻接的硬件边及带宽，paths 则缓存到各类目标的多跳路径。
 * 搜索沿 links/path 检查可达性和容量；一个硬件节点不等于一个 communicator rank。
 * 同一个 GPU 设备在 peerInfo 中有通信组身份，在 topo 中有硬件身份，两层通过 rank/busId 关联。
 */
/* 中文逐项-类型 T0140：ncclTopoNode
 * 类型职责：单个硬件对象、同类节点数组和全系统拓扑容器，供路径计算和图搜索使用。
 * 声明内可见成员（含嵌套成员）：type、id、gpu、dev、rank、cudaCompCap、gdrSupport、net、asic、port、width、latency、gdrSupport、collSupport、maxChannels、cpu、arch、vendor、model、affinity、pci、device、nlinks、links、paths、used。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTopoNode {
/* 中文逐项-变量 V1610：type
 * type（int；成员 ncclTopoNode）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  int type;
/* 中文逐项-变量 V1611：id
 * id（int64_t；成员 ncclTopoNode）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  int64_t id;
  // Type specific data
/* 中文逐项-类型 T0141：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：gpu、dev、rank、cudaCompCap、gdrSupport、net、asic、port、width、latency、gdrSupport、collSupport、maxChannels、cpu、arch、vendor、model、affinity、pci、device。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V1612：gpu
 * gpu（struct { int dev; // NVML dev number int rank; int cudaCompCap; int gdrSupport; }；成员
 * ncclTopoNode::<匿名>）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 */
/* 中文逐项-类型 T0142：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：gpu、dev、rank、cudaCompCap、gdrSupport。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V1613：dev
 * dev（int；成员 ncclTopoNode::<匿名>::<匿名>）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 */
      int dev; // NVML dev number
/* 中文逐项-变量 V1614：rank
 * rank（int；成员 ncclTopoNode::<匿名>::<匿名>）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
      int rank;
/* 中文逐项-变量 V1615：cudaCompCap
 * cudaCompCap（int；成员 ncclTopoNode::<匿名>::<匿名>）：GPU计算能力整数编码，影响NVLink速率、协议支持和编译特化选择。
 */
      int cudaCompCap;
/* 中文逐项-变量 V1616：gdrSupport
 * gdrSupport（int；成员 ncclTopoNode::<匿名>::<匿名>）：设备能否注册GPU内存、进行网络集合归约及接受哪种指针类型的能力描述。
 */
      int gdrSupport;
    }gpu;
/* 中文逐项-变量 V1617：net
 * net（struct { uint64_t asic; int port; float width; float latency; int gdrSupport; int collSupport; int maxChannels
 * …（完整表达式见紧邻源码）；成员 ncclTopoNode::<匿名>）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 */
/* 中文逐项-类型 T0143：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：net、asic、port、width、latency、gdrSupport、collSupport、maxChannels。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V1618：asic
 * asic（uint64_t；成员 ncclTopoNode::<匿名>::<匿名>）：网络设备ASIC标识，用于判断多个端口是否属于同一网卡芯片/共享带宽域。
 */
      uint64_t asic;
/* 中文逐项-变量 V1619：port
 * port（int；成员 ncclTopoNode::<匿名>::<匿名>）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
      int port;
/* 中文逐项-变量 V1620：width
 * width（float；成员 ncclTopoNode::<匿名>::<匿名>）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 */
      float width;
/* 中文逐项-变量 V1621：latency
 * latency（float；成员 ncclTopoNode::<匿名>::<匿名>）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 */
      float latency;
/* 中文逐项-变量 V1622：gdrSupport
 * gdrSupport（int；成员 ncclTopoNode::<匿名>::<匿名>）：设备能否注册GPU内存、进行网络集合归约及接受哪种指针类型的能力描述。
 */
      int gdrSupport;
/* 中文逐项-变量 V1623：collSupport
 * collSupport（int；成员 ncclTopoNode::<匿名>::<匿名>）：设备能否注册GPU内存、进行网络集合归约及接受哪种指针类型的能力描述。
 */
      int collSupport;
/* 中文逐项-变量 V1624：maxChannels
 * maxChannels（int；成员 ncclTopoNode::<匿名>::<匿名>）：图/后端允许的逻辑通道上限和下限，受容量与配置共同约束。
 */
      int maxChannels;
    }net;
/* 中文逐项-变量 V1625：cpu
 * cpu（struct { int arch; int vendor; int model; cpu_set_t affinity; }；成员
 * ncclTopoNode::<匿名>）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 */
/* 中文逐项-类型 T0144：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：cpu、arch、vendor、model、affinity。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V1626：arch
 * arch（int；成员 ncclTopoNode::<匿名>::<匿名>）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 */
      int arch;
/* 中文逐项-变量 V1627：vendor
 * vendor（int；成员 ncclTopoNode::<匿名>::<匿名>）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 */
      int vendor;
/* 中文逐项-变量 V1628：model
 * model（int；成员 ncclTopoNode::<匿名>::<匿名>）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 */
      int model;
/* 中文逐项-变量 V1629：affinity
 * affinity（cpu_set_t；成员 ncclTopoNode::<匿名>::<匿名>）：CPU集合掩码，约束后台线程尽量靠近所属GPU/NIC的NUMA范围。
 */
      cpu_set_t affinity;
    }cpu;
/* 中文逐项-变量 V1630：pci
 * pci（struct { uint64_t device; }；成员 ncclTopoNode::<匿名>）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 */
/* 中文逐项-类型 T0145：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：pci、device。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V1631：device
 * device（uint64_t；成员 ncclTopoNode::<匿名>::<匿名>）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 */
      uint64_t device;
    }pci;
  };
/* 中文逐项-变量 V1632：nlinks
 * nlinks（int；成员 ncclTopoNode）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 */
  int nlinks;
/* 中文逐项-变量 V1633：links
 * links（struct ncclTopoLink；成员 ncclTopoNode）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 */
  struct ncclTopoLink links[NCCL_TOPO_MAX_LINKS];
  // Pre-computed paths to GPUs and NICs
/* 中文逐项-变量 V1634：paths
 * paths（struct ncclTopoLinkList；成员 ncclTopoNode）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 */
  struct ncclTopoLinkList* paths[NCCL_TOPO_NODE_TYPES];
  // Used during search
/* 中文逐项-变量 V1635：used
 * used（uint64_t；成员 ncclTopoNode）：搜索中已使用GPU/节点的位图或状态，回溯后须恢复以尝试其他候选。
 */
  uint64_t used;
};

/* 中文逐项-类型 T0146：ncclTopoNodeSet
 * 类型职责：单个硬件对象、同类节点数组和全系统拓扑容器，供路径计算和图搜索使用。
 * 声明内可见成员（含嵌套成员）：count、nodes。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTopoNodeSet {
/* 中文逐项-变量 V1636：count
 * count（int；成员 ncclTopoNodeSet）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 */
  int count;
/* 中文逐项-变量 V1637：nodes
 * nodes（struct ncclTopoNode；成员 ncclTopoNodeSet）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 */
  struct ncclTopoNode nodes[NCCL_TOPO_MAX_NODES];
};

/* 中文导读：
 * 当前 rank 看到的硬件拓扑：GPU、NVSwitch、PCI、NUMA CPU、NIC/NET 节点及其链路。
 * 先从硬件描述构图，再预计算路径，最后在此图上搜索 Ring/Tree；硬件图与通信算法图分属不同层。
 */
/* 中文逐项-类型 T0147：ncclTopoSystem
 * 类型职责：单个硬件对象、同类节点数组和全系统拓扑容器，供路径计算和图搜索使用。
 * 声明内可见成员（含嵌套成员）：nodes、maxWidth、totalWidth。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTopoSystem {
/* 中文逐项-变量 V1638：nodes
 * nodes（struct ncclTopoNodeSet；成员 ncclTopoSystem）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 */
  struct ncclTopoNodeSet nodes[NCCL_TOPO_NODE_TYPES];
/* 中文逐项-变量 V1639：maxWidth
 * maxWidth（float；成员 ncclTopoSystem）：硬件系统最大单路带宽和可聚合宽度，用于设置搜索速度与提前终止条件。
 */
  float maxWidth;
/* 中文逐项-变量 V1640：totalWidth
 * totalWidth（float；成员 ncclTopoSystem）：硬件系统最大单路带宽和可聚合宽度，用于设置搜索速度与提前终止条件。
 */
  float totalWidth;
};

/* 中文逐项-接口 X0085：ncclTopoGetNode
 * 接口职责：在系统指定类型节点表中按 id 查找对象，返回已有节点地址。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetNode(struct ncclTopoSystem* system, struct ncclTopoNode**
 * node, int type, uint64_t id);
 */
ncclResult_t ncclTopoGetNode(struct ncclTopoSystem* system, struct ncclTopoNode** node, int type, uint64_t id);
/* 中文逐项-接口 X0086：ncclTopoCreateNode
 * 接口职责：在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoCreateNode(struct ncclTopoSystem* system, struct
 * ncclTopoNode** node, int type, uint64_t id);
 */
ncclResult_t ncclTopoCreateNode(struct ncclTopoSystem* system, struct ncclTopoNode** node, int type, uint64_t id);
/* 中文逐项-接口 X0087：ncclTopoRemoveNode
 * 接口职责：删除硬件节点并修正指向它的连接及数组位置，避免后续搜索引用被裁掉对象。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoRemoveNode(struct ncclTopoSystem* system, int type, int id);
 */
ncclResult_t ncclTopoRemoveNode(struct ncclTopoSystem* system, int type, int id);
/* 中文逐项-接口 X0088：ncclTopoConnectNodes
 * 接口职责：给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoConnectNodes(struct ncclTopoNode* node, struct ncclTopoNode*
 * remNode, int type, float width);
 */
ncclResult_t ncclTopoConnectNodes(struct ncclTopoNode* node, struct ncclTopoNode* remNode, int type, float width);
/* 中文逐项-接口 X0089：ncclTopoPrintPaths
 * 接口职责：打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoPrintPaths(struct ncclTopoSystem* system);
 */
ncclResult_t ncclTopoPrintPaths(struct ncclTopoSystem* system);
ncclResult_t ncclTopoLoadSystem(const char* xmlTopoFile, struct ncclTopoSystem* system);
/* 中文逐项-接口 X0090：ncclTopoGetIntermediateRank
 * 接口职责：找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetIntermediateRank(struct ncclTopoSystem* system, int rank,
 * int netDev, int* intermediateRank);
 */
ncclResult_t ncclTopoGetIntermediateRank(struct ncclTopoSystem* system, int rank, int netDev, int* intermediateRank);

/* 中文逐项-接口 X0091：ncclTopoGetSystemFromXml
 * 接口职责：把硬件 XML 的层级属性转换成 ncclTopoSystem 节点和双向链路模型。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetSystemFromXml(struct ncclXml* xml, struct ncclTopoSystem**
 * topoSystem);
 */
ncclResult_t ncclTopoGetSystemFromXml(struct ncclXml* xml, struct ncclTopoSystem** topoSystem);
/* 中文逐项-接口 X0092：ncclTopoGetGraphFromXml
 * 接口职责：在 XML 中找到当前 graph id 的对象，再调用子解析器填充通信图。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetGraphFromXml(struct ncclXmlNode *xmlGraphs, struct
 * ncclTopoSystem* system, struct ncclTopoGraph* graph, int* nChannels);
 */
ncclResult_t ncclTopoGetGraphFromXml(struct ncclXmlNode *xmlGraphs, struct ncclTopoSystem* system, struct ncclTopoGraph* graph, int* nChannels);
/* 中文逐项-接口 X0093：ncclTopoGetXmlFromGraphs
 * 接口职责：创建 XML 根并导出多种算法的通信图，保持模式/id 信息。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetXmlFromGraphs(int ngraphs, struct ncclTopoGraph** graphs,
 * struct ncclTopoSystem* system, struct ncclXml *xml);
 */
ncclResult_t ncclTopoGetXmlFromGraphs(int ngraphs, struct ncclTopoGraph** graphs, struct ncclTopoSystem* system, struct ncclXml *xml);

/* 中文逐项-接口 X0094：ncclTopoGetCompCap
 * 接口职责：遍历 GPU 得到计算能力的最小/最大值，使协议与调优模型兼容参与设备。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetCompCap(struct ncclTopoSystem* system, int* ccMin, int*
 * ccMax);
 */
ncclResult_t ncclTopoGetCompCap(struct ncclTopoSystem* system, int* ccMin, int* ccMax);

/* 中文逐项-函数 F0523：ncclTopoIdToIndex
 * 用途与位置：把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 id（int64_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 index（int* index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoCheckP2p；src/graph/paths.cc::ncclTopoCheckGdr；src/graph/paths.cc::ncclTopoGetIntermediateRank；src/graph/search.cc::ncclTopoGetNetDev；src/graph/topo.cc::ncclTopoFlattenBcmSwitches。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<system->nodes[type].count；system->nodes[type].nodes[i].id == id。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclTopoIdToIndex(struct ncclTopoSystem* system, int type, int64_t id, int* index) {
  *index = -1;
  for (/* 中文逐项-变量 V1641：i
 * i（int；函数 ncclTopoIdToIndex 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[type].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[type].count; i++) {
    if (system->nodes[type].nodes[i].id == id) {
      *index = i;
      return ncclSuccess;
    }
  }
  return ncclInternalError;
}

/* 中文逐项-函数 F0524：ncclTopoRankToIndex
 * 用途与位置：把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 index（int* index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoCheckGdr；src/graph/paths.cc::ncclTopoGetIntermediateRank；src/graph/paths.cc::ncclTopoGetNchannels；src/graph/search.cc::ncclTopoGetNetDev；src/graph/topo.cc::ncclTopoGetLocalNet。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<system->nodes[GPU].count；system->nodes[GPU].nodes[i].gpu.rank == rank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclTopoRankToIndex(struct ncclTopoSystem* system, int rank, int* index) {
  *index = -1;
  for (/* 中文逐项-变量 V1642：i
 * i（int；函数 ncclTopoRankToIndex 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[GPU].count; i++) {
    if (system->nodes[GPU].nodes[i].gpu.rank == rank) {
      *index = i;
      return ncclSuccess;
    }
  }
  return ncclInternalError;
}

// Returns NVLink speed in GB/s
/* 中文逐项-函数 F0525：ncclTopoNVLinkSpeed
 * 用途与位置：按 GPU 计算能力选择该代 NVLink 单链估计速率，供路径通道预算使用。
 * 参数 cudaCompCap（int cudaCompCap）：GPU计算能力整数编码，影响NVLink速率、协议支持和编译特化选择。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoGetNchannels；src/graph/topo.cc::ncclTopoAddNvLinks。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return cudaCompCap == 86 ? SM86_NVLINK_WIDTH : cudaCompCap >= 80 ? SM80_NVLINK_WIDTH : cudaCompCap >= 70 ?
 * SM70_NVLINK_WIDTH : cudaCompCap > …（完整表达式见紧邻源码）。CHECK类宏还可能提前返回/跳转。
 */
static float ncclTopoNVLinkSpeed(int cudaCompCap) {
  return
    cudaCompCap == 86 ? SM86_NVLINK_WIDTH :
    cudaCompCap >= 80 ? SM80_NVLINK_WIDTH :
    cudaCompCap >= 70 ? SM70_NVLINK_WIDTH :
    cudaCompCap >= 60 ? SM60_NVLINK_WIDTH :
    SM80_NVLINK_WIDTH;
}
#endif
