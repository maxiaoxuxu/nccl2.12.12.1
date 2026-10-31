/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_GRAPH_H_
/* 中文逐项-常量 X0286：NCCL_GRAPH_H_
 * 编译期定义：NCCL_GRAPH_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_GRAPH_H_

#include "nccl.h"
#include "devcomm.h"
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sched.h>

ncclResult_t ncclTopoCudaPath(int cudaDev, char** path);

struct ncclTopoSystem;
// Build the topology
/* 中文逐项-接口 X0287：ncclTopoGetSystem
 * 接口职责：读入可选拓扑文件并补齐当前 rank 所见 GPU/NIC 的自动发现信息，再生成硬件图。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetSystem(struct ncclComm* comm, struct ncclTopoSystem**
 * system);
 */
ncclResult_t ncclTopoGetSystem(struct ncclComm* comm, struct ncclTopoSystem** system);
/* 中文逐项-接口 X0288：ncclTopoSortSystem
 * 接口职责：对节点/邻接链组织顺序做规范化，方便拓扑遍历和搜索使用一致输入。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoSortSystem(struct ncclTopoSystem* system);
 */
ncclResult_t ncclTopoSortSystem(struct ncclTopoSystem* system);
/* 中文逐项-接口 X0289：ncclTopoPrint
 * 接口职责：递归打印硬件节点、类型和直接链路，验证发现到的硬件树。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoPrint(struct ncclTopoSystem* system);
 */
ncclResult_t ncclTopoPrint(struct ncclTopoSystem* system);

/* 中文逐项-接口 X0290：ncclTopoComputePaths
 * 接口职责：计算 CPU/GPU/NET 路径，并按 P2P、GDR、PXN 策略替换不可直接使用的路径。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoComputePaths(struct ncclTopoSystem* system, struct
 * ncclPeerInfo* info);
 */
ncclResult_t ncclTopoComputePaths(struct ncclTopoSystem* system, struct ncclPeerInfo* info);
/* 中文逐项-接口 X0291：ncclTopoFree
 * 接口职责：回收硬件系统的所有路径缓存和系统对象，本身不关闭 transport 已建立的连接。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void ncclTopoFree(struct ncclTopoSystem* system);
 */
void ncclTopoFree(struct ncclTopoSystem* system);
/* 中文逐项-接口 X0292：ncclTopoTrimSystem
 * 接口职责：删除不属于当前通信可达域的硬件节点，并在完全机内通信时裁剪不需要的网络对象。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoTrimSystem(struct ncclTopoSystem* system, struct ncclComm*
 * comm);
 */
ncclResult_t ncclTopoTrimSystem(struct ncclTopoSystem* system, struct ncclComm* comm);
/* 中文逐项-接口 X0293：ncclTopoComputeP2pChannels
 * 接口职责：收敛所有本地 GPU 到 peer 的通道预算，计算全局 P2P 空间与每 peer 数量并生成位反转偏移。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoComputeP2pChannels(struct ncclComm* comm);
 */
ncclResult_t ncclTopoComputeP2pChannels(struct ncclComm* comm);
/* 中文逐项-接口 X0294：ncclTopoGetNvbGpus
 * 接口职责：找出可经一个中间 GPU 走 NVLink 桥接的 peer，为特定机内路径预连接提供候选。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetNvbGpus(struct ncclTopoSystem* system, int rank, int*
 * nranks, int** ranks);
 */
ncclResult_t ncclTopoGetNvbGpus(struct ncclTopoSystem* system, int rank, int* nranks, int** ranks);

// Query topology
/* 中文逐项-接口 X0295：ncclTopoGetNetDev
 * 接口职责：按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetNetDev(struct ncclComm* comm, int rank, struct
 * ncclTopoGraph* graph, int channelId, int peerRank, int* net, int* proxyRank);
 */
ncclResult_t ncclTopoGetNetDev(struct ncclComm* comm, int rank, struct ncclTopoGraph* graph, int channelId, int peerRank, int* net, int* proxyRank);
/* 中文逐项-接口 X0296：ncclTopoCheckP2p
 * 接口职责：结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoCheckP2p(struct ncclTopoSystem* system, int64_t id1, int64_t
 * id2, int* p2p, int *read, int* intermediateRank);
 */
ncclResult_t ncclTopoCheckP2p(struct ncclTopoSystem* system, int64_t id1, int64_t id2, int* p2p, int *read, int* intermediateRank);
/* 中文逐项-接口 X0297：ncclTopoCheckGdr
 * 接口职责：结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoCheckGdr(struct ncclTopoSystem* topo, int64_t busId, int
 * netDev, int read, int* useGdr);
 */
ncclResult_t ncclTopoCheckGdr(struct ncclTopoSystem* topo, int64_t busId, int netDev, int read, int* useGdr);
/* 中文逐项-接口 X0298：ncclPxnDisable
 * 接口职责：汇总参数和环境条件判断是否禁止 PXN，避免选择当前后端不支持的中间 GPU 网络路径。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int ncclPxnDisable();
 */
int ncclPxnDisable();
/* 中文逐项-接口 X0299：ncclTopoGetPxnRanks
 * 接口职责：列出本 rank 网络路径可能依赖的中间 GPU rank，为共享代理资源提前准备。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetPxnRanks(struct ncclComm* comm, int** intermediateRanks,
 * int* nranks);
 */
ncclResult_t ncclTopoGetPxnRanks(struct ncclComm* comm, int** intermediateRanks, int* nranks);
/* 中文逐项-接口 X0300：ncclTopoGetLocalRank
 * 接口职责：把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetLocalRank(struct ncclTopoSystem* system, int rank, int*
 * localRank);
 */
ncclResult_t ncclTopoGetLocalRank(struct ncclTopoSystem* system, int rank, int* localRank);

// Find CPU affinity
/* 中文逐项-接口 X0301：ncclTopoGetCpuAffinity
 * 接口职责：结合 GPU 的近端 CPU/NUMA 亲和范围与当前进程约束，生成线程亲和性掩码。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetCpuAffinity(struct ncclTopoSystem* system, int rank,
 * cpu_set_t* affinity);
 */
ncclResult_t ncclTopoGetCpuAffinity(struct ncclTopoSystem* system, int rank, cpu_set_t* affinity);

/* 中文逐项-常量 X0302：NCCL_TOPO_CPU_ARCH_X86
 * 编译期定义：NCCL_TOPO_CPU_ARCH_X86 = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_ARCH_X86 1
/* 中文逐项-常量 X0303：NCCL_TOPO_CPU_ARCH_POWER
 * 编译期定义：NCCL_TOPO_CPU_ARCH_POWER = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_ARCH_POWER 2
/* 中文逐项-常量 X0304：NCCL_TOPO_CPU_ARCH_ARM
 * 编译期定义：NCCL_TOPO_CPU_ARCH_ARM = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_ARCH_ARM 3
/* 中文逐项-常量 X0305：NCCL_TOPO_CPU_VENDOR_INTEL
 * 编译期定义：NCCL_TOPO_CPU_VENDOR_INTEL = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_VENDOR_INTEL 1
/* 中文逐项-常量 X0306：NCCL_TOPO_CPU_VENDOR_AMD
 * 编译期定义：NCCL_TOPO_CPU_VENDOR_AMD = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_VENDOR_AMD 2
/* 中文逐项-常量 X0307：NCCL_TOPO_CPU_VENDOR_ZHAOXIN
 * 编译期定义：NCCL_TOPO_CPU_VENDOR_ZHAOXIN = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_VENDOR_ZHAOXIN 3
/* 中文逐项-常量 X0308：NCCL_TOPO_CPU_TYPE_BDW
 * 编译期定义：NCCL_TOPO_CPU_TYPE_BDW = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_TYPE_BDW 1
/* 中文逐项-常量 X0309：NCCL_TOPO_CPU_TYPE_SKL
 * 编译期定义：NCCL_TOPO_CPU_TYPE_SKL = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_TYPE_SKL 2
/* 中文逐项-常量 X0310：NCCL_TOPO_CPU_TYPE_YONGFENG
 * 编译期定义：NCCL_TOPO_CPU_TYPE_YONGFENG = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_CPU_TYPE_YONGFENG 1
/* 中文逐项-接口 X0311：ncclTopoCpuType
 * 接口职责：从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoCpuType(struct ncclTopoSystem* system, int* arch, int* vendor,
 * int* model);
 */
ncclResult_t ncclTopoCpuType(struct ncclTopoSystem* system, int* arch, int* vendor, int* model);
/* 中文逐项-接口 X0312：ncclTopoGetNetCount
 * 接口职责：返回当前裁剪后硬件图中的 NET 设备数量，用于判断是否存在网络路径。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetNetCount(struct ncclTopoSystem* system, int* count);
 */
ncclResult_t ncclTopoGetNetCount(struct ncclTopoSystem* system, int* count);
/* 中文逐项-接口 X0313：ncclTopoGetLocalNet
 * 接口职责：按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetLocalNet(struct ncclTopoSystem* system, int rank, int* id);
 */
ncclResult_t ncclTopoGetLocalNet(struct ncclTopoSystem* system, int rank, int* id);

/* 中文逐项-常量 X0314：NCCL_TOPO_MAX_NODES
 * 编译期定义：NCCL_TOPO_MAX_NODES = 256。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_MAX_NODES 256

// Init search. Needs to be done before calling ncclTopoCompute
/* 中文逐项-接口 X0315：ncclTopoSearchInit
 * 接口职责：初始化拓扑搜索的最大宽度和总宽度，建立速度候选与提前终止判断依据。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoSearchInit(struct ncclTopoSystem* system);
 */
ncclResult_t ncclTopoSearchInit(struct ncclTopoSystem* system);

/* 中文逐项-常量 X0316：NCCL_TOPO_PATTERN_BALANCED_TREE
 * 编译期定义：NCCL_TOPO_PATTERN_BALANCED_TREE = 1 // Spread NIC traffic between two GPUs (Tree parent + one child on first
 * GPU, second child on second GPU)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_PATTERN_BALANCED_TREE 1   // Spread NIC traffic between two GPUs (Tree parent + one child on first GPU, second child on second GPU)
/* 中文逐项-常量 X0317：NCCL_TOPO_PATTERN_SPLIT_TREE
 * 编译期定义：NCCL_TOPO_PATTERN_SPLIT_TREE = 2 // Spread NIC traffic between two GPUs (Tree parent on first GPU, tree
 * children on the second GPU)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_PATTERN_SPLIT_TREE 2      // Spread NIC traffic between two GPUs (Tree parent on first GPU, tree children on the second GPU)
/* 中文逐项-常量 X0318：NCCL_TOPO_PATTERN_TREE
 * 编译期定义：NCCL_TOPO_PATTERN_TREE = 3 // All NIC traffic going to/from the same GPU。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_PATTERN_TREE 3            // All NIC traffic going to/from the same GPU
/* 中文逐项-常量 X0319：NCCL_TOPO_PATTERN_RING
 * 编译期定义：NCCL_TOPO_PATTERN_RING = 4 // Ring。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_PATTERN_RING 4            // Ring
/* 中文导读：
 * 拓扑搜索得到的通信方案：每个 channel 的本地 GPU 顺序及跨节点网络端点。
 * speedIntra/speedInter 等用于搜索与性能估计，不是已经测得的运行时吞吐率，也不是已创建的连接。
 */
/* 中文精读：通信搜索图字段的阅读坐标
 * id/pattern 标识搜索任务及端点约束，minChannels/maxChannels 限定候选范围。
 * nChannels 是已找到的图条数；speedIntra/speedInter 为每通道机内/机间目标速度。
 * intra 按 channel 存 GPU 用户 rank 顺序，inter 每 channel 存两个 NET 端点。
 * typeIntra/typeInter 约束允许的硬件路径类别，sameChannels/crossNic 影响候选空间。
 * 本对象是初始化时的搜索结果，运行时再转成 comm.channels 的具体邻居关系。
 */
/* 中文逐项-类型 T0207：ncclTopoGraph
 * 类型职责：某种算法搜索出的各channel GPU/NIC顺序、速度和路径约束，初始化后转成实际邻居。
 * 声明内可见成员（含嵌套成员）：id、pattern、crossNic、collNet、minChannels、maxChannels、nChannels、speedIntra、speedInter、latencyInter、typeIntra、typeInter、sameChannels、nHops、intra、inter。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTopoGraph {
  // Input / output
/* 中文逐项-变量 V2277：id
 * id（int；成员 ncclTopoGraph）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  int id; // ring : 0, tree : 1, collnet : 2
/* 中文逐项-变量 V2278：pattern
 * pattern（int；成员 ncclTopoGraph）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
  int pattern;
/* 中文逐项-变量 V2279：crossNic
 * crossNic（int；成员 ncclTopoGraph）：图搜索是否允许进出不同NIC、是否重放相同机内GPU顺序的限制。
 */
  int crossNic;
/* 中文逐项-变量 V2280：collNet
 * collNet（int；成员 ncclTopoGraph）：图搜索任务是否服务网络集合归约，影响端点与速度约束。
 */
  int collNet;
/* 中文逐项-变量 V2281：minChannels
 * minChannels（int；成员 ncclTopoGraph）：图/后端允许的逻辑通道上限和下限，受容量与配置共同约束。
 */
  int minChannels;
/* 中文逐项-变量 V2282：maxChannels
 * maxChannels（int；成员 ncclTopoGraph）：图/后端允许的逻辑通道上限和下限，受容量与配置共同约束。
 */
  int maxChannels;
  // Output
/* 中文逐项-变量 V2283：nChannels
 * nChannels（int；成员 ncclTopoGraph）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 */
  int nChannels;
/* 中文逐项-变量 V2284：speedIntra
 * speedIntra（float；成员 ncclTopoGraph）：图中每通道机内/机间速度与机间延迟估计，不是运行时逐包测量。
 */
  float speedIntra;
/* 中文逐项-变量 V2285：speedInter
 * speedInter（float；成员 ncclTopoGraph）：图中每通道机内/机间速度与机间延迟估计，不是运行时逐包测量。
 */
  float speedInter;
/* 中文逐项-变量 V2286：latencyInter
 * latencyInter（float；成员 ncclTopoGraph）：图中每通道机内/机间速度与机间延迟估计，不是运行时逐包测量。
 */
  float latencyInter;
/* 中文逐项-变量 V2287：typeIntra
 * typeIntra（int；成员 ncclTopoGraph）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
  int typeIntra;
/* 中文逐项-变量 V2288：typeInter
 * typeInter（int；成员 ncclTopoGraph）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
  int typeInter;
/* 中文逐项-变量 V2289：sameChannels
 * sameChannels（int；成员 ncclTopoGraph）：图搜索是否允许进出不同NIC、是否重放相同机内GPU顺序的限制。
 */
  int sameChannels;
/* 中文逐项-变量 V2290：nHops
 * nHops（int；成员 ncclTopoGraph）：整个候选通信图累计的路径跳数，用于同带宽候选之间的比较。
 */
  int nHops;
/* 中文逐项-变量 V2291：intra
 * intra（int；成员 ncclTopoGraph）：按channel记录机内GPU用户rank排列及两个网络端点，后续转成具体邻居连接。
 */
  int intra[MAXCHANNELS*NCCL_TOPO_MAX_NODES];
/* 中文逐项-变量 V2292：inter
 * inter（int；成员 ncclTopoGraph）：按channel记录机内GPU用户rank排列及两个网络端点，后续转成具体邻居连接。
 */
  int inter[MAXCHANNELS*2];
};
/* 中文逐项-接口 X0320：ncclTopoCompute
 * 接口职责：在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoCompute(struct ncclTopoSystem* system, struct ncclTopoGraph*
 * graph);
 */
ncclResult_t ncclTopoCompute(struct ncclTopoSystem* system, struct ncclTopoGraph* graph);

/* 中文逐项-接口 X0321：ncclTopoPrintGraph
 * 接口职责：输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoPrintGraph(struct ncclTopoSystem* system, struct
 * ncclTopoGraph* graph);
 */
ncclResult_t ncclTopoPrintGraph(struct ncclTopoSystem* system, struct ncclTopoGraph* graph);
/* 中文逐项-接口 X0322：ncclTopoDumpGraphs
 * 接口职责：输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoDumpGraphs(struct ncclTopoSystem* system, int ngraphs, struct
 * ncclTopoGraph** graphs);
 */
ncclResult_t ncclTopoDumpGraphs(struct ncclTopoSystem* system, int ngraphs, struct ncclTopoGraph** graphs);

/* 中文逐项-类型 T0208：ncclTopoRanks
 * 类型职责：各rank通过控制网络交换的局部环/树端点摘要，供跨节点全局拼接。
 * 声明内可见成员（含嵌套成员）：ringRecv、ringSend、ringPrev、ringNext、treeToParent、treeToChild0、treeToChild1。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTopoRanks {
/* 中文逐项-变量 V2293：ringRecv
 * ringRecv（int；成员 ncclTopoRanks）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 */
  int ringRecv[MAXCHANNELS];
/* 中文逐项-变量 V2294：ringSend
 * ringSend（int；成员 ncclTopoRanks）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 */
  int ringSend[MAXCHANNELS];
/* 中文逐项-变量 V2295：ringPrev
 * ringPrev（int；成员 ncclTopoRanks）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 */
  int ringPrev[MAXCHANNELS];
/* 中文逐项-变量 V2296：ringNext
 * ringNext（int；成员 ncclTopoRanks）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 */
  int ringNext[MAXCHANNELS];
/* 中文逐项-变量 V2297：treeToParent
 * treeToParent（int；成员 ncclTopoRanks）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 */
  int treeToParent[MAXCHANNELS];
/* 中文逐项-变量 V2298：treeToChild0
 * treeToChild0（int；成员 ncclTopoRanks）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 */
  int treeToChild0[MAXCHANNELS];
/* 中文逐项-变量 V2299：treeToChild1
 * treeToChild1（int；成员 ncclTopoRanks）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 */
  int treeToChild1[MAXCHANNELS];
};

/* 中文逐项-接口 X0323：ncclTopoPreset
 * 接口职责：将本机搜索图转成可交换的局部环/树端点摘要，为跨节点拼接准备元数据。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoPreset(struct ncclComm* comm, struct ncclTopoGraph* treeGraph,
 * struct ncclTopoGraph* ringGraph, struct ncclTopoRanks* topoRanks);
 */
ncclResult_t ncclTopoPreset(struct ncclComm* comm,
    struct ncclTopoGraph* treeGraph, struct ncclTopoGraph* ringGraph,
    struct ncclTopoRanks* topoRanks);

/* 中文逐项-接口 X0324：ncclTopoPostset
 * 接口职责：汇总全部节点的端点信息并拼接全局 Ring/Tree，复制/裁剪通道后验证环完整性。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoPostset(struct ncclComm* comm, int* firstRanks, int*
 * treePatterns, struct ncclTopoRanks** allTopoRanks, int* rings, struct ncclTopoGraph* collNetGraph);
 */
ncclResult_t ncclTopoPostset(struct ncclComm* comm, int* firstRanks, int* treePatterns,
    struct ncclTopoRanks** allTopoRanks, int* rings, struct ncclTopoGraph* collNetGraph);

/* 中文逐项-接口 X0325：ncclTopoTuneModel
 * 接口职责：结合拓扑图带宽、节点规模、GPU 架构和配置构建各 API/算法/协议的延迟与带宽表。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoTuneModel(struct ncclComm* comm, int minCompCap, int
 * maxCompCap, struct ncclTopoGraph* treeGraph, struct ncclTopoGraph* ringGraph, struct ncclTopoGraph* collNetGraph);
 */
ncclResult_t ncclTopoTuneModel(struct ncclComm* comm, int minCompCap, int maxCompCap, struct ncclTopoGraph* treeGraph, struct ncclTopoGraph* ringGraph, struct ncclTopoGraph* collNetGraph);
#include "info.h"
/* 中文逐项-接口 X0326：ncclTopoGetAlgoTime
 * 接口职责：用延迟项加字节数/带宽项预测当前候选耗时，加入 Tree/Ring 与聚合修正供选型。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetAlgoTime(struct ncclInfo* info, int algorithm, int
 * protocol, int numPipeOps, float* time);
 */
ncclResult_t ncclTopoGetAlgoTime(struct ncclInfo* info, int algorithm, int protocol, int numPipeOps, float* time);

#endif
