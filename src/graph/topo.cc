/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "core.h"
#include "graph.h"
#include "topo.h"
#include "comm.h"
#include "nvmlwrap.h"
#include "net.h"
#include "coll_net.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "xml.h"
#include "cpuset.h"

/* 中文逐项-常量 X0039：BUSID_SIZE
 * 编译期定义：BUSID_SIZE = (sizeof("0000:00:00.0"))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define BUSID_SIZE (sizeof("0000:00:00.0"))
/* 中文逐项-常量 X0040：BUSID_REDUCED_SIZE
 * 编译期定义：BUSID_REDUCED_SIZE = (sizeof("0000:00"))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define BUSID_REDUCED_SIZE (sizeof("0000:00"))

/* 中文逐项-变量 V1474：topoNodeTypeStr
 * topoNodeTypeStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 topoNodeTypeStr = { "GPU", "PCI", "NVS", "CPU", "NIC", "NET"
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
const char* topoNodeTypeStr[] = { "GPU", "PCI", "NVS", "CPU", "NIC", "NET" };
/* 中文逐项-变量 V1475：topoLinkTypeStr
 * topoLinkTypeStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 topoLinkTypeStr = { "LOC", "NVL", "", "PCI", "", "", "", "SYS", "NET"
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
const char* topoLinkTypeStr[] = { "LOC", "NVL", "",    "PCI",    "",    "",    "", "SYS", "NET" };
/* 中文逐项-变量 V1476：topoPathTypeStr
 * topoPathTypeStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 topoPathTypeStr = { "LOC", "NVL", "NVB", "PIX", "PXB", "PXN", "PHB", "SYS", "DIS"
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
const char* topoPathTypeStr[] = { "LOC", "NVL", "NVB", "PIX", "PXB", "PXN", "PHB", "SYS", "DIS" };

/******************************************************************/
/******************* Graph Creation Functions *********************/
/******************************************************************/

// Get an int64 from a PCI path. For example, sys/class/pci0000:00/0000:00:02.0/0000:02:00.0/ will return 0x000002000.
/* 中文逐项-函数 F0493：pciPathToInt64
 * 用途与位置：把 PCI 路径中的 BDF 片段解析成便于匹配和排序的整数标识。
 * 参数 path（char* path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 offset（int offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 参数 minOffset（int minOffset）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 参数 id（int64_t* id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；busIdToInt64。
 * 需要对照的边界/推进条件：*str == '/'；*str != '/'。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t pciPathToInt64(char* path, int offset, int minOffset, int64_t* id) {
/* 中文逐项-变量 V1477：str
 * str（char；函数 pciPathToInt64 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = path+offset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  char* str = path+offset;
  // Remove trailing "/"
  if (*str == '/') str--;
  // Find next /
  while (*str != '/') str--;
  str++;
/* 中文逐项-变量 V1478：numid
 * numid（int64_t；函数 pciPathToInt64 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(busIdToInt64(str, &numid));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int64_t numid;
  NCCLCHECK(busIdToInt64(str, &numid));
  // Ignore subdevice because those should use the same PCI link so we want to merge nodes.
  numid -= numid & 0xf;
  *id = numid;
  return ncclSuccess;
}

/* 中文逐项-函数 F0494：findLocalCpu
 * 用途与位置：沿硬件连接查找设备所在 CPU 节点，用于补建 CPU/PCI 关系。
 * 参数 node（struct ncclTopoNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 cpu（struct ncclTopoNode** cpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::findLocalCpu；src/graph/topo.cc::ncclTopoAddNvLinks。
 * 本体直接出现的调用/包装：NCCLCHECK；findLocalCpu。
 * 需要对照的边界/推进条件：node->type == CPU；l<node->nlinks；node->links[l].type == LINK_PCI；*cpu != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t findLocalCpu(struct ncclTopoNode* node, struct ncclTopoNode** cpu) {
  *cpu = NULL;
  if (node->type == CPU) {
    *cpu = node;
    return ncclSuccess;
  }
  for (/* 中文逐项-变量 V1479：l
 * l（int；函数 findLocalCpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<node->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<node->nlinks; l++) {
    if (node->links[l].type == LINK_PCI) NCCLCHECK(findLocalCpu(node->links[l].remNode, cpu));
    if (*cpu != NULL) return ncclSuccess;
  }
  return ncclSuccess;
}

/* 中文逐项-变量 V1480：interCpuWidth
 * interCpuWidth（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 interCpuWidth = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
int interCpuWidth = 0;
/* 中文逐项-变量 V1481：cpuPciWidth
 * cpuPciWidth（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cpuPciWidth = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
int cpuPciWidth = 0;

/* 中文逐项-函数 F0495：ncclTopoGetInterCpuWidth
 * 用途与位置：依据 CPU 架构/型号估计 CPU 之间的互联带宽，补全硬件图中 SYS 边容量。
 * 参数 cpu（struct ncclTopoNode* cpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 参数 width（float* width）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoConnectCpus。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_POWER；cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_ARM；cpu->cpu.arch ==
 * NCCL_TOPO_CPU_ARCH_X86 && cpu->cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL；cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_X86 &&
 * cpu->cpu.vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclTopoGetInterCpuWidth(struct ncclTopoNode* cpu, float* width) {
  *width = LOC_WIDTH;
  if (cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_POWER) {
    *width = P9_WIDTH;
    return ncclSuccess;
  }
  if (cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_ARM) {
    *width = ARM_WIDTH;
    return ncclSuccess;
  }
  if (cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_X86 && cpu->cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL) {
    *width = cpu->cpu.model == NCCL_TOPO_CPU_TYPE_SKL ? SKL_QPI_WIDTH : QPI_WIDTH;
  }
  if (cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_X86 && cpu->cpu.vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN) {
    *width = cpu->cpu.model ==  NCCL_TOPO_CPU_TYPE_YONGFENG ? YONGFENG_ZPI_WIDTH : ZPI_WIDTH;
  }
  return ncclSuccess;
}

/* 中文逐项-类型 T0137：ncclNvLinkDeviceType
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclNvLinkDeviceType {
/* 中文逐项-枚举项 X0041：ncclNvLinkDeviceUnknown
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclNvLinkDeviceUnknown,
/* 中文逐项-枚举项 X0042：ncclNvLinkDeviceGpu
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclNvLinkDeviceGpu,
/* 中文逐项-枚举项 X0043：ncclNvLinkDeviceSwitch
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclNvLinkDeviceSwitch,
/* 中文逐项-枚举项 X0044：ncclNvLinkDeviceBridge
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclNvLinkDeviceBridge, // IBM/Power NVLink bridge (Device 04ea)
};

/* 中文逐项-函数 F0496：ncclTopoGetNode
 * 用途与位置：在系统指定类型节点表中按 id 查找对象，返回已有节点地址。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 node（struct ncclTopoNode** node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 id（uint64_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu；src/graph/topo.cc::ncclTopoAddNvLinks。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<system->nodes[type].count；system->nodes[type].nodes[i].id == id。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetNode(struct ncclTopoSystem* system, struct ncclTopoNode** node, int type, uint64_t id) {
  for (/* 中文逐项-变量 V1482：i
 * i（int；函数 ncclTopoGetNode 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[type].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[type].count; i++) {
    if (system->nodes[type].nodes[i].id == id) {
      *node = system->nodes[type].nodes+i;
      return ncclSuccess;
    }
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0497：ncclTopoCreateNode
 * 用途与位置：在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 node（struct ncclTopoNode** node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 id（uint64_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddNet；src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu；src/graph/topo.cc::ncclTopoAddNvLinks。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：system->nodes[type].count == NCCL_TOPO_MAX_NODES；type == GPU；type == CPU；type == NET。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoCreateNode(struct ncclTopoSystem* system, struct ncclTopoNode** node, int type, uint64_t id) {
  if (system->nodes[type].count == NCCL_TOPO_MAX_NODES) {
    WARN("Error : tried to create too many nodes of type %d", type);
    return ncclInternalError;
  }
/* 中文逐项-变量 V1483：n
 * n（struct ncclTopoNode；函数 ncclTopoCreateNode 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = system->nodes[type].nodes+system->nodes[type].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* n = system->nodes[type].nodes+system->nodes[type].count;
  system->nodes[type].count++;
  n->type = type;
  n->id = id;
  if (type == GPU) {
    // Create link to itself (used in some corner cases)
    n->nlinks=1;
    n->links[0].type = LINK_LOC;
    n->links[0].remNode = n;
    n->links[0].width = LOC_WIDTH;
    n->gpu.dev = NCCL_TOPO_UNDEF;
    n->gpu.rank = NCCL_TOPO_UNDEF;
    n->gpu.cudaCompCap = NCCL_TOPO_UNDEF;
  } else if (type == CPU) {
    n->cpu.arch = NCCL_TOPO_UNDEF;
    n->cpu.vendor = NCCL_TOPO_UNDEF;
    n->cpu.model = NCCL_TOPO_UNDEF;
  } else if (type == NET) {
    n->net.asic = 0ULL;
    n->net.port = NCCL_TOPO_UNDEF;
    n->net.width = 0.0;
    n->net.latency = 0.0;
  }
  *node = n;
  return ncclSuccess;
}

/* 中文逐项-函数 F0498：ncclTopoRemoveNode
 * 用途与位置：删除硬件节点并修正指向它的连接及数组位置，避免后续搜索引用被裁掉对象。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 index（int index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoTrimSystem；src/graph/topo.cc::ncclTopoFlattenBcmSwitches。
 * 本体直接出现的调用/包装：free；memmove。
 * 需要对照的边界/推进条件：t<NCCL_TOPO_NODE_TYPES；n<system->nodes[t].count；node == delNode；l<node->nlinks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoRemoveNode(struct ncclTopoSystem* system, int type, int index) {
/* 中文逐项-变量 V1484：delNode
 * delNode（struct ncclTopoNode；函数 ncclTopoRemoveNode 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 delNode = system->nodes[type].nodes+index；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* delNode = system->nodes[type].nodes+index;
  for (/* 中文逐项-变量 V1485：t
 * t（int；函数 ncclTopoRemoveNode 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：t<NCCL_TOPO_NODE_TYPES；循环体与增量表达式决定每轮如何推进。
 */int t=0; t<NCCL_TOPO_NODE_TYPES; t++) {
    free(delNode->paths[t]);
    for (/* 中文逐项-变量 V1486：n
 * n（int；函数 ncclTopoRemoveNode 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[t].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[t].count; n++) {
/* 中文逐项-变量 V1487：node
 * node（struct ncclTopoNode；函数 ncclTopoRemoveNode 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = system->nodes[t].nodes+n；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoNode* node = system->nodes[t].nodes+n;
      if (node == delNode) continue;
      for (/* 中文逐项-变量 V1488：l
 * l（int；函数 ncclTopoRemoveNode 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<node->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<node->nlinks; l++) {
        while (l<node->nlinks && node->links[l].remNode == delNode) {
          memmove(node->links+l, node->links+l+1, (node->nlinks-l-1)*sizeof(struct ncclTopoLink));
          node->nlinks--;
        }
        if (l<node->nlinks && node->links[l].remNode->type == type && node->links[l].remNode >= delNode) {
          node->links[l].remNode--;
        }
      }
    }
  }
  memmove(delNode, delNode+1, (system->nodes[type].count-index-1)*sizeof(struct ncclTopoNode));
  system->nodes[type].count--;
  return ncclSuccess;
}

/* 中文逐项-函数 F0499：ncclTopoConnectNodes
 * 用途与位置：给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。
 * 参数 node（struct ncclTopoNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 remNode（struct ncclTopoNode* remNode）：硬件边指向的另一个节点，路径搜索沿此指针扩展。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 width（float width）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoConnectCpus；src/graph/topo.cc::ncclTopoAddNet；src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu；src/graph/topo.cc::ncclTopoAddNvLinks。
 * 本体直接出现的调用/包装：memcpy。
 * 需要对照的边界/推进条件：link->remNode；link->remNode == remNode && link->type == type；link->remNode == NULL；link !=
 * node->links。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoConnectNodes(struct ncclTopoNode* node, struct ncclTopoNode* remNode, int type, float width) {
  // Aggregate links into higher width for NVLink
/* 中文逐项-变量 V1489：link
 * link（struct ncclTopoLink；函数 ncclTopoConnectNodes 内）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 此处未显式给初值；后续首个文本使用点：for (link = node->links; link->remNode; link++) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoLink* link;
  for (link = node->links; link->remNode; link++) {
    if (link->remNode == remNode && link->type == type) break;
  }
  if (link->remNode == NULL) node->nlinks++;
  link->type = type;
  link->remNode = remNode;
  link->width += width;

  // Sort links in BW descending order
/* 中文逐项-变量 V1490：linkSave
 * linkSave（struct ncclTopoLink；函数 ncclTopoConnectNodes 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memcpy(&linkSave, link, sizeof(struct ncclTopoLink));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoLink linkSave;
  memcpy(&linkSave, link, sizeof(struct ncclTopoLink));
  while (link != node->links) {
    if ((link-1)->width >= linkSave.width) break;
    memcpy(link, link-1, sizeof(struct ncclTopoLink));
    link--;
  }
  memcpy(link, &linkSave, sizeof(struct ncclTopoLink));
  return ncclSuccess;
}

// BCM Gen4 Switches present themselves as a two-level hierarchical switch
// even though they're supposed to sustain full BW across all ports.
// Flatten the switch as this extra level can break the search and make
// NCCL take wrong topology decisions.
/* 中文逐项-函数 F0500：ncclTopoFlattenBcmSwitches
 * 用途与位置：识别可合并表示的 Broadcom PCI 交换结构，简化建图而不额外发起硬件传输。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystemFromXml。
 * 本体直接出现的调用/包装：NCCLCHECK；free；memcpy；memmove；ncclCalloc；ncclTopoIdToIndex；ncclTopoRemoveNode。
 * 需要对照的边界/推进条件：s<system->nodes[PCI].count；device & 0xfffffffffffff000) ==
 * 0x1000c0101000a000；l<pciSwitch->nlinks；sub->type != PCI || sub->pci.device != device。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoFlattenBcmSwitches(struct ncclTopoSystem* system) {
  for (/* 中文逐项-变量 V1491：s
 * s（int；函数 ncclTopoFlattenBcmSwitches 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<system->nodes[PCI].count；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<system->nodes[PCI].count; s++) {
/* 中文逐项-变量 V1492：pciSwitch
 * pciSwitch（struct ncclTopoNode；函数 ncclTopoFlattenBcmSwitches 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pciSwitch = system->nodes[PCI].nodes+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* pciSwitch = system->nodes[PCI].nodes+s;
/* 中文逐项-变量 V1493：device
 * device（uint64_t；函数 ncclTopoFlattenBcmSwitches 内）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 初值/来源 device = pciSwitch->pci.device；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    uint64_t device = pciSwitch->pci.device;
    // Only flatten PEX Gen 4 switches in base mode
    if ((device & 0xfffffffffffff000) == 0x1000c0101000a000) {
      // Find sub switches with the same device ID.
/* 中文逐项-变量 V1494：subSwIds
 * subSwIds（int64_t；函数 ncclTopoFlattenBcmSwitches 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&subSwIds, pciSwitch->nlinks));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int64_t* subSwIds;
      NCCLCHECK(ncclCalloc(&subSwIds, pciSwitch->nlinks));
/* 中文逐项-变量 V1495：subs
 * subs（int；函数 ncclTopoFlattenBcmSwitches 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 subs = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int subs = 0;
      for (/* 中文逐项-变量 V1496：l
 * l（int；函数 ncclTopoFlattenBcmSwitches 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<pciSwitch->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<pciSwitch->nlinks; l++) {
/* 中文逐项-变量 V1497：sub
 * sub（struct ncclTopoNode；函数 ncclTopoFlattenBcmSwitches 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = pciSwitch->links[l].remNode；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclTopoNode* sub = pciSwitch->links[l].remNode;
        // Only fuse sub switches with the same device ID.
        if (sub->type != PCI || sub->pci.device != device) continue;
        // Save sub switch for later
        subSwIds[subs++] = sub->id;
        // Remove link to that sub switch
        memmove(pciSwitch->links+l, pciSwitch->links+l+1, (pciSwitch->nlinks-l-1)*(sizeof(struct ncclTopoLink)));
        pciSwitch->nlinks--;
        // Don't increase l for the next iteration as we just shifted all links by one.
        l--;
      }

      for (/* 中文逐项-变量 V1498：s
 * s（int；函数 ncclTopoFlattenBcmSwitches 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<subs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<subs; s++) {
        // Find sub switch (system->nodes[PCI].nodes is changing every time we remove a node)
/* 中文逐项-变量 V1499：index
 * index（int；函数 ncclTopoFlattenBcmSwitches 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoIdToIndex(system, PCI, subSwIds[s], &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int index;
        NCCLCHECK(ncclTopoIdToIndex(system, PCI, subSwIds[s], &index));
/* 中文逐项-变量 V1500：sub
 * sub（struct ncclTopoNode；函数 ncclTopoFlattenBcmSwitches 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = system->nodes[PCI].nodes+index；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclTopoNode* sub = system->nodes[PCI].nodes+index;
        // Connect all sub PCI devices to the parent switch
        for (/* 中文逐项-变量 V1501：l
 * l（int；函数 ncclTopoFlattenBcmSwitches 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<sub->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<sub->nlinks; l++) {
/* 中文逐项-变量 V1502：remNode
 * remNode（struct ncclTopoNode；函数 ncclTopoFlattenBcmSwitches 内）：硬件边指向的另一个节点，路径搜索沿此指针扩展。
 * 初值/来源 remNode = sub->links[l].remNode；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          struct ncclTopoNode* remNode = sub->links[l].remNode;
          if (remNode == pciSwitch) continue;
          // Add link from parent PCI switch -> PCI device
          memcpy(pciSwitch->links+pciSwitch->nlinks, sub->links+l, sizeof(struct ncclTopoLink));
          pciSwitch->nlinks++;
          // Update link from PCI device -> parent PCI switch
          for (/* 中文逐项-变量 V1503：rl
 * rl（int；函数 ncclTopoFlattenBcmSwitches 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rl = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：rl<remNode->nlinks；循环体与增量表达式决定每轮如何推进。
 */int rl=0; rl<remNode->nlinks; rl++) {
            if (remNode->links[rl].remNode == sub) {
              remNode->links[rl].remNode = pciSwitch;
              break;
            }
          }
        }
        NCCLCHECK(ncclTopoRemoveNode(system, PCI, index));
      }
      // Set subdevice to 0x0000 to make sure we don't merge this switch again.
      pciSwitch->pci.device = 0x1000c01010000000;
      free(subSwIds);
      // Restart, as system->nodes[PCI].nodes has changed.
      s = 0;
    }
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0501：ncclTopoConnectCpus
 * 用途与位置：根据 CPU 类型补建 CPU 之间的互连边，使跨 NUMA 路径可被统一搜索。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystemFromXml。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoConnectNodes；ncclTopoGetInterCpuWidth。
 * 需要对照的边界/推进条件：n<system->nodes[CPU].count；p<system->nodes[CPU].count；n == p。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoConnectCpus(struct ncclTopoSystem* system) {
  // And connect all CPU nodes together
  for (/* 中文逐项-变量 V1504：n
 * n（int；函数 ncclTopoConnectCpus 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[CPU].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[CPU].count; n++) {
    for (/* 中文逐项-变量 V1505：p
 * p（int；函数 ncclTopoConnectCpus 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<system->nodes[CPU].count；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<system->nodes[CPU].count; p++) {
      if (n == p) continue;
/* 中文逐项-变量 V1506：width
 * width（float；函数 ncclTopoConnectCpus 内）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetInterCpuWidth(system->nodes[CPU].nodes+n,
 * &width));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      float width;
      NCCLCHECK(ncclTopoGetInterCpuWidth(system->nodes[CPU].nodes+n, &width));
      NCCLCHECK(ncclTopoConnectNodes(system->nodes[CPU].nodes+n, system->nodes[CPU].nodes+p, LINK_SYS, width));
    }
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0502：ncclTopoPrintRec
 * 用途与位置：递归打印硬件节点、类型和直接链路，验证发现到的硬件树。
 * 参数 node（struct ncclTopoNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 prevNode（struct ncclTopoNode* prevNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 line（char* line）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 offset（int offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoPrintRec；src/graph/topo.cc::ncclTopoPrint。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoPrintRec；sprintf；strlen。
 * 需要对照的边界/推进条件：node->type == GPU；node->type == CPU；node->type == PCI；i<offset。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclTopoPrintRec(struct ncclTopoNode* node, struct ncclTopoNode* prevNode, char* line, int offset) {
  if (node->type == GPU) {
    sprintf(line+offset, "%s/%lX (%d)", topoNodeTypeStr[node->type], node->id, node->gpu.rank);
  } else if (node->type == CPU) {
    sprintf(line+offset, "%s/%lX (%d/%d/%d)", topoNodeTypeStr[node->type], node->id, node->cpu.arch, node->cpu.vendor, node->cpu.model);
  } else if (node->type == PCI) {
    sprintf(line+offset, "%s/%lX (%lx)", topoNodeTypeStr[node->type], node->id, node->pci.device);
  } else {
    sprintf(line+offset, "%s/%lX", topoNodeTypeStr[node->type], node->id);
  }
  INFO(NCCL_GRAPH, "%s", line);
  for (/* 中文逐项-变量 V1507：i
 * i（int；函数 ncclTopoPrintRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<offset；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<offset; i++) line[i] = ' ';

  for (/* 中文逐项-变量 V1508：l
 * l（int；函数 ncclTopoPrintRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<node->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<node->nlinks; l++) {
/* 中文逐项-变量 V1509：link
 * link（struct ncclTopoLink；函数 ncclTopoPrintRec 内）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 初值/来源 link = node->links+l；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoLink* link = node->links+l;
    if (link->type == LINK_LOC) continue;
    if (link->type != LINK_PCI || link->remNode != prevNode) {
      sprintf(line+offset, "+ %s[%2.1f] - ", topoLinkTypeStr[link->type], link->width);
/* 中文逐项-变量 V1510：nextOffset
 * nextOffset（int；函数 ncclTopoPrintRec 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 nextOffset = strlen(line)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int nextOffset = strlen(line);
      if (link->type == LINK_PCI) {
        NCCLCHECK(ncclTopoPrintRec(link->remNode, node, line, nextOffset));
      } else {
        if (link->remNode->type == NET) {
          sprintf(line+nextOffset, "%s/%lX (%lx/%d/%f)", topoNodeTypeStr[link->remNode->type], link->remNode->id, link->remNode->net.asic, link->remNode->net.port, link->remNode->net.width);
        } else {
          sprintf(line+nextOffset, "%s/%lX", topoNodeTypeStr[link->remNode->type], link->remNode->id);
        }
        INFO(NCCL_GRAPH, "%s", line);
      }
    }
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0503：ncclTopoPrint
 * 用途与位置：递归打印硬件节点、类型和直接链路，验证发现到的硬件树。
 * 参数 s（struct ncclTopoSystem* s）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoPrintPaths；ncclTopoPrintRec。
 * 需要对照的边界/推进条件：n<s->nodes[CPU].count。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoPrint(struct ncclTopoSystem* s) {
  INFO(NCCL_GRAPH, "=== System : maxWidth %2.1f totalWidth %2.1f ===", s->maxWidth, s->totalWidth);
/* 中文逐项-变量 V1511：line
 * line（char；函数 ncclTopoPrint 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：for (int n=0; n<s->nodes[CPU].count; n++) NCCLCHECK(ncclTopoPrintRec(s->nodes[CPU].nodes+n,
 * NULL, line, 0));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[1024];
  for (/* 中文逐项-变量 V1512：n
 * n（int；函数 ncclTopoPrint 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<s->nodes[CPU].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<s->nodes[CPU].count; n++) NCCLCHECK(ncclTopoPrintRec(s->nodes[CPU].nodes+n, NULL, line, 0));
  INFO(NCCL_GRAPH, "==========================================");
  NCCLCHECK(ncclTopoPrintPaths(s));
  return ncclSuccess;
}

/* 中文逐项-函数 F0504：ncclTopoSort
 * 用途与位置：对节点/邻接链组织顺序做规范化，方便拓扑遍历和搜索使用一致输入。
 * 参数 node（struct ncclTopoNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 upNode（struct ncclTopoNode* upNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoSort；src/graph/topo.cc::ncclTopoSortSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclTopoSort。
 * 需要对照的边界/推进条件：upNode；node->links[l].remNode != upNode；node->links[l+1].remNode；l<node->nlinks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclTopoSort(struct ncclTopoNode* node, struct ncclTopoNode* upNode) {
  // Shift all links to have upLink as last link
  if (upNode) {
/* 中文逐项-变量 V1513：l
 * l（int；函数 ncclTopoSort 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int l=0;
    while (node->links[l].remNode != upNode) l++;
/* 中文逐项-变量 V1514：upLink
 * upLink（struct ncclTopoLink；函数 ncclTopoSort 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memcpy(&upLink, node->links+l, sizeof(struct ncclTopoLink));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclTopoLink upLink;
    memcpy(&upLink, node->links+l, sizeof(struct ncclTopoLink));
    while (node->links[l+1].remNode) {
      memcpy(node->links+l, node->links+l+1, sizeof(struct ncclTopoLink));
      l++;
    }
    memcpy(node->links+l, &upLink, sizeof(struct ncclTopoLink));
  }

  // Recursively sort the PCI tree
  for (/* 中文逐项-变量 V1515：l
 * l（int；函数 ncclTopoSort 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<node->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<node->nlinks; l++) {
/* 中文逐项-变量 V1516：link
 * link（struct ncclTopoLink；函数 ncclTopoSort 内）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 初值/来源 link = node->links+l；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoLink* link = node->links+l;
    if (link->type == LINK_PCI && link->remNode != upNode) NCCLCHECK(ncclTopoSort(link->remNode, node));
  }
  return ncclSuccess;
}

// We want the graph to be organized to ease/accelerate traversal :
// 1. NVLinks (already the case)
// 2. PCI down
// 3. PCI up
// 4. SYS (already the case)
/* 中文逐项-函数 F0505：ncclTopoSortSystem
 * 用途与位置：对节点/邻接链组织顺序做规范化，方便拓扑遍历和搜索使用一致输入。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystemFromXml。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoSort。
 * 需要对照的边界/推进条件：n<system->nodes[CPU].count。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSortSystem(struct ncclTopoSystem* system) {
  for (/* 中文逐项-变量 V1517：n
 * n（int；函数 ncclTopoSortSystem 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[CPU].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[CPU].count; n++) NCCLCHECK(ncclTopoSort(system->nodes[CPU].nodes+n, NULL));
  return ncclSuccess;
}

/* 中文逐项-函数 F0506：ncclTopoAddNet
 * 用途与位置：从 XML 的网络/网卡属性建立 NIC/NET 硬件节点，并接到对应 PCI/CPU 层级。
 * 参数 xmlNet（struct ncclXmlNode* xmlNet）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 nic（struct ncclTopoNode* nic）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddNic。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoConnectNodes；ncclTopoCreateNode；sscanf；xmlGetAttr；xmlGetAttrFloat；xmlGetAttrInt；xmlGetAttrIntDefault。
 * 需要对照的边界/推进条件：str；mbps <= 0；xmlGetAttrFloat(xmlNet, "latency", &net->net.latency) != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoAddNet(struct ncclXmlNode* xmlNet, struct ncclTopoSystem* system, struct ncclTopoNode* nic) {
/* 中文逐项-变量 V1518：dev
 * dev（int；函数 ncclTopoAddNet 内）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlNet, "dev", &dev));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int dev;
  NCCLCHECK(xmlGetAttrInt(xmlNet, "dev", &dev));

/* 中文逐项-变量 V1519：net
 * net（struct ncclTopoNode；函数 ncclTopoAddNet 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCreateNode(system, &net, NET, dev));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoNode* net;
  NCCLCHECK(ncclTopoCreateNode(system, &net, NET, dev));
/* 中文逐项-变量 V1520：str
 * str（char；函数 ncclTopoAddNet 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(xmlNet, "guid", &str));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;
  NCCLCHECK(xmlGetAttr(xmlNet, "guid", &str));
  if (str) sscanf(str, "0x%lx", &net->net.asic);
  else net->net.asic = dev;

  ncclDebugNoWarn = NCCL_GRAPH;
/* 中文逐项-变量 V1521：mbps
 * mbps（int；函数 ncclTopoAddNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "speed", &mbps, 0));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int mbps;
  NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "speed", &mbps, 0));
  if (mbps <= 0) mbps = 10000; // Some NICs define speed = -1
  net->net.width = mbps / 8000.0;
  if (xmlGetAttrFloat(xmlNet, "latency", &net->net.latency) != ncclSuccess) net->net.latency = 0;
  NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "port", &net->net.port, 0));
  NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "gdr", &net->net.gdrSupport, 0));
  NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "maxconn", &net->net.maxChannels, MAXCHANNELS));
  NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "coll", &net->net.collSupport, 0));
  ncclDebugNoWarn = 0;

  NCCLCHECK(ncclTopoConnectNodes(nic, net, LINK_NET, net->net.width));
  NCCLCHECK(ncclTopoConnectNodes(net, nic, LINK_NET, net->net.width));
  return ncclSuccess;
}

/* 中文逐项-函数 F0507：ncclTopoAddNic
 * 用途与位置：从 XML 的网络/网卡属性建立 NIC/NET 硬件节点，并接到对应 PCI/CPU 层级。
 * 参数 xmlNic（struct ncclXmlNode* xmlNic）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 nic（struct ncclTopoNode* nic）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoAddNet；strcmp；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：s<xmlNic->nSubs；strcmp(xmlNet->name, "net") != 0；index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoAddNic(struct ncclXmlNode* xmlNic, struct ncclTopoSystem* system, struct ncclTopoNode* nic) {
  for (/* 中文逐项-变量 V1522：s
 * s（int；函数 ncclTopoAddNic 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<xmlNic->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<xmlNic->nSubs; s++) {
/* 中文逐项-变量 V1523：xmlNet
 * xmlNet（struct ncclXmlNode；函数 ncclTopoAddNic 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 xmlNet = xmlNic->subs[s]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* xmlNet = xmlNic->subs[s];
    if (strcmp(xmlNet->name, "net") != 0) continue;
/* 中文逐项-变量 V1524：index
 * index（int；函数 ncclTopoAddNic 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(xmlNet, "dev", &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int index;
    NCCLCHECK(xmlGetAttrIndex(xmlNet, "dev", &index));
    if (index == -1) continue;
    NCCLCHECK(ncclTopoAddNet(xmlNet, system, nic));
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0508：ncclTopoAddGpu
 * 用途与位置：从 XML 的 GPU 属性建立设备节点、通信 rank 及能力信息，并关联父级硬件。
 * 参数 xmlGpu（struct ncclXmlNode* xmlGpu）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 gpu（struct ncclTopoNode* gpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddPci。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlGetAttrInt。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoAddGpu(struct ncclXmlNode* xmlGpu, struct ncclTopoSystem* system, struct ncclTopoNode* gpu) {
  NCCLCHECK(xmlGetAttrInt(xmlGpu, "sm", &gpu->gpu.cudaCompCap));
  NCCLCHECK(xmlGetAttrInt(xmlGpu, "rank", &gpu->gpu.rank));
  NCCLCHECK(xmlGetAttrInt(xmlGpu, "dev", &gpu->gpu.dev));
  NCCLCHECK(xmlGetAttrInt(xmlGpu, "gdr", &gpu->gpu.gdrSupport));
  // Do not go any further, nvlinks will be added in a second pass
  return ncclSuccess;
}

/* 中文逐项-变量 V1525：kvDictPciClass
 * kvDictPciClass（struct kvDict；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 kvDictPciClass = { { "0x060400", PCI }, { "0x068000", NVS }, { "0x068001", CPU }, { "0x03", GPU }, { "0x02",
 * NIC }, { NULL, PCI / * Default fallback value * / } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct kvDict kvDictPciClass[] = { { "0x060400", PCI }, { "0x068000", NVS }, { "0x068001", CPU }, { "0x03", GPU }, { "0x02", NIC }, { NULL, PCI /* Default fallback value */ } };
/* 中文逐项-变量 V1526：kvDictPciGen
 * kvDictPciGen（struct kvDict；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 kvDictPciGen = { { "2.5 GT/s", 15 }, { "5 GT/s", 30 }, { "8 GT/s", 60 }, { "16 GT/s", 120 }, { "32 GT/s", 240
 * }, / * Kernel 5.6 and earlier * / { "2.5 GT/s PCIe", 15 }, { "5.0 GT/s PCIe", 30 }, { "8.0 GT/s PCIe", 60 }, {
 * "16.0 GT/s PCIe", 120 }, …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct kvDict kvDictPciGen[] = {
  { "2.5 GT/s", 15 }, { "5 GT/s", 30 }, { "8 GT/s", 60 }, { "16 GT/s", 120 }, { "32 GT/s", 240 }, /* Kernel 5.6 and earlier */
  { "2.5 GT/s PCIe", 15 }, { "5.0 GT/s PCIe", 30 }, { "8.0 GT/s PCIe", 60 }, { "16.0 GT/s PCIe", 120 }, { "32.0 GT/s PCIe", 240 }, { "64.0 GT/s PCIe", 480 },
  { NULL, 60 /* Default fallback */ } }; // x100 Mbps per lane
/* 中文逐项-函数 F0509：ncclTopoAddPci
 * 用途与位置：递归建立 PCI 桥/交换节点与其子设备，使用链路代际和宽度推导带宽。
 * 参数 xmlPci（struct ncclXmlNode* xmlPci）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 parent（struct ncclTopoNode* parent）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu。
 * 本体直接出现的调用/包装：NCCLCHECK；busIdToInt64；kvConvertToInt；ncclTopoAddGpu；ncclTopoAddNic；ncclTopoAddPci；ncclTopoConnectNodes；ncclTopoCreateNode；ncclTopoGetNode；strtol；xmlGetAttr；xmlGetAttrIndex；xmlGetAttrInt；xmlGetAttrStr；xmlGetSub。
 * 需要对照的边界/推进条件：xmlGpu != NULL；index == -1；xmlNic != NULL；nicNode == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoAddPci(struct ncclXmlNode* xmlPci, struct ncclTopoSystem* system, struct ncclTopoNode* parent) {
/* 中文逐项-变量 V1527：str
 * str（char；函数 ncclTopoAddPci 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrStr(xmlPci, "class", &str));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;

/* 中文逐项-变量 V1528：type
 * type（int；函数 ncclTopoAddPci 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(kvConvertToInt(str, &type, kvDictPciClass));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int type;
  NCCLCHECK(xmlGetAttrStr(xmlPci, "class", &str));
  NCCLCHECK(kvConvertToInt(str, &type, kvDictPciClass));

/* 中文逐项-变量 V1529：busId
 * busId（int64_t；函数 ncclTopoAddPci 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(busIdToInt64(str, &busId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int64_t busId;
  NCCLCHECK(xmlGetAttrStr(xmlPci, "busid", &str));
  NCCLCHECK(busIdToInt64(str, &busId));

/* 中文逐项-变量 V1530：node
 * node（struct ncclTopoNode；函数 ncclTopoAddPci 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclTopoNode* node = NULL;
/* 中文逐项-变量 V1531：xmlGpu
 * xmlGpu（struct ncclXmlNode；函数 ncclTopoAddPci 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 xmlGpu = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclXmlNode* xmlGpu = NULL;
  NCCLCHECK(xmlGetSub(xmlPci, "gpu", &xmlGpu));
  if (xmlGpu != NULL) {
    type = GPU;
/* 中文逐项-变量 V1532：index
 * index（int；函数 ncclTopoAddPci 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(xmlGpu, "rank", &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int index;
    NCCLCHECK(xmlGetAttrIndex(xmlGpu, "rank", &index));
    if (index == -1) return ncclSuccess;
    NCCLCHECK(ncclTopoCreateNode(system, &node, type, busId));
    NCCLCHECK(ncclTopoAddGpu(xmlGpu, system, node));
  }
/* 中文逐项-变量 V1533：xmlNic
 * xmlNic（struct ncclXmlNode；函数 ncclTopoAddPci 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 xmlNic = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclXmlNode* xmlNic = NULL;
  NCCLCHECK(xmlGetSub(xmlPci, "nic", &xmlNic));
  if (xmlNic != NULL) {
    type = NIC;
    // Ignore sub device ID and merge multi-port NICs into one PCI device.
    busId &= 0xfffffffffffffff0;
/* 中文逐项-变量 V1534：nicNode
 * nicNode（struct ncclTopoNode；函数 ncclTopoAddPci 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nicNode = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    struct ncclTopoNode* nicNode = NULL;
    NCCLCHECK(ncclTopoGetNode(system, &nicNode, type, busId));
    if (nicNode == NULL) {
      NCCLCHECK(ncclTopoCreateNode(system, &nicNode, type, busId));
      node = nicNode; // Connect it to parent later on
    }
    NCCLCHECK(ncclTopoAddNic(xmlNic, system, nicNode));
  } else if (type == PCI) {
    NCCLCHECK(ncclTopoCreateNode(system, &node, type, busId));
    NCCLCHECK(xmlGetAttr(xmlPci, "vendor", &str));
    if (str) node->pci.device += strtol(str, NULL, 0) << 48;
    NCCLCHECK(xmlGetAttr(xmlPci, "device", &str));
    if (str) node->pci.device += strtol(str, NULL, 0) << 32;
    NCCLCHECK(xmlGetAttr(xmlPci, "subsystem_vendor", &str));
    if (str) node->pci.device += strtol(str, NULL, 0) << 16;
    NCCLCHECK(xmlGetAttr(xmlPci, "subsystem_device", &str));
    if (str) node->pci.device += strtol(str, NULL, 0);

    for (/* 中文逐项-变量 V1535：s
 * s（int；函数 ncclTopoAddPci 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<xmlPci->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<xmlPci->nSubs; s++) {
/* 中文逐项-变量 V1536：xmlSubPci
 * xmlSubPci（struct ncclXmlNode；函数 ncclTopoAddPci 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 xmlSubPci = xmlPci->subs[s]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclXmlNode* xmlSubPci = xmlPci->subs[s];
      NCCLCHECK(ncclTopoAddPci(xmlSubPci, system, node));
    }
  }

  if (node) {
/* 中文逐项-变量 V1537：width、speed
 * width（int；函数 ncclTopoAddPci 内）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlPci, "link_width", &width));。若由输出参数填入，必须先检查填充调用成功再读值。
 * speed（int；函数 ncclTopoAddPci 内）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(kvConvertToInt(str, &speed, kvDictPciGen));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int width, speed;
    NCCLCHECK(xmlGetAttrInt(xmlPci, "link_width", &width));
    NCCLCHECK(xmlGetAttrStr(xmlPci, "link_speed", &str));

    // Manage cases where speed was not indicated in /sys
    if (width == 0) width = 16;
    NCCLCHECK(kvConvertToInt(str, &speed, kvDictPciGen)); // Values in 100Mbps, per lane (we want GB/s in the end)

    NCCLCHECK(ncclTopoConnectNodes(node, parent, LINK_PCI, width*speed/80.0));
    NCCLCHECK(ncclTopoConnectNodes(parent, node, LINK_PCI, width*speed/80.0));
  }
  return ncclSuccess;
}

/* 中文逐项-变量 V1538：kvDictCpuArch
 * kvDictCpuArch（struct kvDict；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 kvDictCpuArch = { { "x86_64", NCCL_TOPO_CPU_ARCH_X86 }, { "arm64", NCCL_TOPO_CPU_ARCH_ARM }, { "ppc64",
 * NCCL_TOPO_CPU_ARCH_POWER }, { NULL, 0 } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct kvDict kvDictCpuArch[] = { { "x86_64", NCCL_TOPO_CPU_ARCH_X86 }, { "arm64", NCCL_TOPO_CPU_ARCH_ARM }, { "ppc64", NCCL_TOPO_CPU_ARCH_POWER }, { NULL, 0 } };
/* 中文逐项-变量 V1539：kvDictCpuVendor
 * kvDictCpuVendor（struct kvDict；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 kvDictCpuVendor = { { "GenuineIntel", NCCL_TOPO_CPU_VENDOR_INTEL }, { "AuthenticAMD",
 * NCCL_TOPO_CPU_VENDOR_AMD }, { "CentaurHauls", NCCL_TOPO_CPU_VENDOR_ZHAOXIN }, { " Shanghai ",
 * NCCL_TOPO_CPU_VENDOR_ZHAOXIN }, { NULL, 0 } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct kvDict kvDictCpuVendor[] = { { "GenuineIntel", NCCL_TOPO_CPU_VENDOR_INTEL }, { "AuthenticAMD", NCCL_TOPO_CPU_VENDOR_AMD }, { "CentaurHauls", NCCL_TOPO_CPU_VENDOR_ZHAOXIN }, { "  Shanghai  ", NCCL_TOPO_CPU_VENDOR_ZHAOXIN }, { NULL, 0 } };

/* 中文逐项-函数 F0510：ncclTopoAddCpu
 * 用途与位置：建立 CPU/NUMA 节点并递归导入其 PCI、GPU、NIC 子树。
 * 参数 xmlCpu（struct ncclXmlNode* xmlCpu）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystemFromXml。
 * 本体直接出现的调用/包装：NCCLCHECK；kvConvertToInt；ncclStrToCpuset；ncclTopoAddNic；ncclTopoAddPci；ncclTopoConnectNodes；ncclTopoCreateNode；ncclTopoGetNode；strcmp；xmlGetAttr；xmlGetAttrInt；xmlGetAttrStr。
 * 需要对照的边界/推进条件：str != NULL；cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_X86；cpu->cpu.vendor ==
 * NCCL_TOPO_CPU_VENDOR_INTEL；cpu->cpu.vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoAddCpu(struct ncclXmlNode* xmlCpu, struct ncclTopoSystem* system) {
/* 中文逐项-变量 V1540：numaId
 * numaId（int；函数 ncclTopoAddCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlCpu, "numaid", &numaId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int numaId;
  NCCLCHECK(xmlGetAttrInt(xmlCpu, "numaid", &numaId));
/* 中文逐项-变量 V1541：cpu
 * cpu（struct ncclTopoNode；函数 ncclTopoAddCpu 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCreateNode(system, &cpu, CPU, numaId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoNode* cpu;
  NCCLCHECK(ncclTopoCreateNode(system, &cpu, CPU, numaId));
/* 中文逐项-变量 V1542：str
 * str（char；函数 ncclTopoAddCpu 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(xmlCpu, "affinity", &str));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;
  NCCLCHECK(xmlGetAttr(xmlCpu, "affinity", &str));
  if (str != NULL) {
    NCCLCHECK(ncclStrToCpuset(str, &cpu->cpu.affinity));
  }

  NCCLCHECK(xmlGetAttrStr(xmlCpu, "arch", &str));
  NCCLCHECK(kvConvertToInt(str, &cpu->cpu.arch, kvDictCpuArch));
  if (cpu->cpu.arch == NCCL_TOPO_CPU_ARCH_X86) {
    NCCLCHECK(xmlGetAttrStr(xmlCpu, "vendor", &str));
    NCCLCHECK(kvConvertToInt(str, &cpu->cpu.vendor, kvDictCpuVendor));
    if (cpu->cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL) {
/* 中文逐项-变量 V1543：familyId、modelId
 * familyId（int；函数 ncclTopoAddCpu 内）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlCpu, "familyid", &familyId));。若由输出参数填入，必须先检查填充调用成功再读值。
 * modelId（int；函数 ncclTopoAddCpu 内）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlCpu, "modelid", &modelId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int familyId, modelId;
      NCCLCHECK(xmlGetAttrInt(xmlCpu, "familyid", &familyId));
      NCCLCHECK(xmlGetAttrInt(xmlCpu, "modelid", &modelId));
      cpu->cpu.model = (familyId == 6 && modelId >= 0x55) ? NCCL_TOPO_CPU_TYPE_SKL : NCCL_TOPO_CPU_INTEL_BDW;
    } else if (cpu->cpu.vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN) {
/* 中文逐项-变量 V1544：familyId、modelId
 * familyId（int；函数 ncclTopoAddCpu 内）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlCpu, "familyid", &familyId));。若由输出参数填入，必须先检查填充调用成功再读值。
 * modelId（int；函数 ncclTopoAddCpu 内）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlCpu, "modelid", &modelId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int familyId, modelId;
      NCCLCHECK(xmlGetAttrInt(xmlCpu, "familyid", &familyId));
      NCCLCHECK(xmlGetAttrInt(xmlCpu, "modelid", &modelId));
      if (familyId == 7 && modelId == 0x5B) cpu->cpu.model = NCCL_TOPO_CPU_TYPE_YONGFENG;
    }
  }
  for (/* 中文逐项-变量 V1545：s
 * s（int；函数 ncclTopoAddCpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<xmlCpu->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<xmlCpu->nSubs; s++) {
/* 中文逐项-变量 V1546：node
 * node（struct ncclXmlNode；函数 ncclTopoAddCpu 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = xmlCpu->subs[s]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* node = xmlCpu->subs[s];
    if (strcmp(node->name, "pci") == 0) NCCLCHECK(ncclTopoAddPci(node, system, cpu));
    if (strcmp(node->name, "nic") == 0) {
/* 中文逐项-变量 V1547：nic
 * nic（struct ncclTopoNode；函数 ncclTopoAddCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nic = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
      struct ncclTopoNode* nic = NULL;
      NCCLCHECK(ncclTopoGetNode(system, &nic, NIC, 0));
      if (nic == NULL) {
        NCCLCHECK(ncclTopoCreateNode(system, &nic, NIC, 0));
        NCCLCHECK(ncclTopoConnectNodes(cpu, nic, LINK_PCI, LOC_WIDTH));
        NCCLCHECK(ncclTopoConnectNodes(nic, cpu, LINK_PCI, LOC_WIDTH));
      }
      NCCLCHECK(ncclTopoAddNic(node, system, nic));
    }
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0511：ncclTopoAddNvLinks
 * 用途与位置：根据 XML/NVML 的 NVLink 远端信息补建 GPU/交换节点之间的高速链路。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 parentBusId（const char* parentBusId）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddNvLinks；src/graph/topo.cc::ncclTopoGetSystemFromXml。
 * 本体直接出现的调用/包装：NCCLCHECK；busIdToInt64；findLocalCpu；kvConvertToInt；ncclTopoAddNvLinks；ncclTopoConnectNodes；ncclTopoCreateNode；ncclTopoGetNode；ncclTopoNVLinkSpeed；strcmp；xmlGetAttr；xmlGetAttrInt；xmlGetAttrStr。
 * 需要对照的边界/推进条件：strcmp(node->name, "nvlink") == 0；gpu == NULL；targetType == GPU；targetType == CPU。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoAddNvLinks(struct ncclXmlNode* node, struct ncclTopoSystem* system, const char* parentBusId) {
  if (strcmp(node->name, "nvlink") == 0) {
/* 中文逐项-变量 V1548：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoAddNvLinks 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    struct ncclTopoNode* gpu = NULL;
/* 中文逐项-变量 V1549：pBusId
 * pBusId（int64_t；函数 ncclTopoAddNvLinks 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(busIdToInt64(parentBusId, &pBusId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int64_t pBusId;
    NCCLCHECK(busIdToInt64(parentBusId, &pBusId));
    NCCLCHECK(ncclTopoGetNode(system, &gpu, GPU, pBusId));
    if (gpu == NULL) {
      WARN("Add NVLink error : could not find GPU %lx", pBusId);
      return ncclInternalError;
    }
/* 中文逐项-变量 V1550：count
 * count（int；函数 ncclTopoAddNvLinks 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(node, "count", &count));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int count;
    NCCLCHECK(xmlGetAttrInt(node, "count", &count));
/* 中文逐项-变量 V1551：targetClass
 * targetClass（char；函数 ncclTopoAddNvLinks 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrStr(node, "tclass", &targetClass));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    const char* targetClass;
    NCCLCHECK(xmlGetAttrStr(node, "tclass", &targetClass));
/* 中文逐项-变量 V1552：targetType
 * targetType（int；函数 ncclTopoAddNvLinks 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(kvConvertToInt(targetClass, &targetType, kvDictPciClass));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int targetType;
    NCCLCHECK(kvConvertToInt(targetClass, &targetType, kvDictPciClass));
/* 中文逐项-变量 V1553：remote
 * remote（struct ncclTopoNode；函数 ncclTopoAddNvLinks 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 remote = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    struct ncclTopoNode* remote = NULL;
    if (targetType == GPU) {
      // NVL P2P connection to another GPU
/* 中文逐项-变量 V1554：target
 * target（char；函数 ncclTopoAddNvLinks 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrStr(node, "target", &target));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      const char* target;
      NCCLCHECK(xmlGetAttrStr(node, "target", &target));
/* 中文逐项-变量 V1555：busId
 * busId（int64_t；函数 ncclTopoAddNvLinks 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(busIdToInt64(target, &busId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int64_t busId;
      NCCLCHECK(busIdToInt64(target, &busId));
      NCCLCHECK(ncclTopoGetNode(system, &remote, GPU, busId));
    } else if (targetType == CPU) {
      // NVL connection to the local CPU
      NCCLCHECK(findLocalCpu(gpu, &remote));
    } else {
      if (system->nodes[NVS].count == 0) {
        NCCLCHECK(ncclTopoCreateNode(system, &remote, NVS, 0));
      } else {
        remote = system->nodes[NVS].nodes;
      }
    }
    if (remote) {
/* 中文逐项-变量 V1556：nvlSpeed
 * nvlSpeed（float；函数 ncclTopoAddNvLinks 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvlSpeed = ncclTopoNVLinkSpeed(gpu->gpu.cudaCompCap)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      float nvlSpeed = ncclTopoNVLinkSpeed(gpu->gpu.cudaCompCap);
      NCCLCHECK(ncclTopoConnectNodes(gpu, remote, LINK_NVL, count*nvlSpeed));
      if (remote->type != GPU) {
        NCCLCHECK(ncclTopoConnectNodes(remote, gpu, LINK_NVL, count*nvlSpeed));
      }
    }
  } else {
/* 中文逐项-变量 V1557：busId
 * busId（char；函数 ncclTopoAddNvLinks 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(node, "busid", &busId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    const char* busId;
    NCCLCHECK(xmlGetAttr(node, "busid", &busId));
    for (/* 中文逐项-变量 V1558：s
 * s（int；函数 ncclTopoAddNvLinks 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<node->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<node->nSubs; s++) {
      NCCLCHECK(ncclTopoAddNvLinks(node->subs[s], system, busId ? busId : parentBusId));
    }
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0512：ncclTopoGetSystemFromXml
 * 用途与位置：把硬件 XML 的层级属性转换成 ncclTopoSystem 节点和双向链路模型。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 topoSystem（struct ncclTopoSystem** topoSystem）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc；ncclTopoAddCpu；ncclTopoAddNvLinks；ncclTopoConnectCpus；ncclTopoFlattenBcmSwitches；ncclTopoSortSystem；strcmp；xmlFindTag。
 * 需要对照的边界/推进条件：s<topNode->nSubs；strcmp(node->name, "cpu") == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetSystemFromXml(struct ncclXml* xml, struct ncclTopoSystem** topoSystem) {
  NCCLCHECK(ncclCalloc(topoSystem, 1));
/* 中文逐项-变量 V1559：topNode
 * topNode（struct ncclXmlNode；函数 ncclTopoGetSystemFromXml 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlFindTag(xml, "system", &topNode));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclXmlNode* topNode;
  NCCLCHECK(xmlFindTag(xml, "system", &topNode));
  for (/* 中文逐项-变量 V1560：s
 * s（int；函数 ncclTopoGetSystemFromXml 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<topNode->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<topNode->nSubs; s++) {
/* 中文逐项-变量 V1561：node
 * node（struct ncclXmlNode；函数 ncclTopoGetSystemFromXml 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = topNode->subs[s]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* node = topNode->subs[s];
    if (strcmp(node->name, "cpu") == 0) NCCLCHECK(ncclTopoAddCpu(node, *topoSystem));
  }
  NCCLCHECK(ncclTopoAddNvLinks(topNode, *topoSystem, NULL));

  NCCLCHECK(ncclTopoFlattenBcmSwitches(*topoSystem));
  NCCLCHECK(ncclTopoConnectCpus(*topoSystem));
  NCCLCHECK(ncclTopoSortSystem(*topoSystem));

  return ncclSuccess;
}

/* 中文逐项-参数getter P0016：TopoDumpFileRank
 * 生成函数 ncclParamTopoDumpFileRank()，读取 NCCL_TOPO_DUMP_FILE_RANK，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(TopoDumpFileRank, "TOPO_DUMP_FILE_RANK", 0);

// Only set values if not already set
/* 中文逐项-函数 F0513：xmlInitAttrInt
 * 用途与位置：只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const int value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；snprintf；strncpy；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlInitAttrInt(struct ncclXmlNode* node, const char* attrName, const int value) {
/* 中文逐项-变量 V1562：index
 * index（int；函数 xmlInitAttrInt 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    snprintf(node->attrs[index].value, MAX_STR_LEN, "%d", value);
  }
  return ncclSuccess;
}
/* 中文逐项-函数 F0514：xmlInitAttrUint64
 * 用途与位置：只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const uint64_t value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；snprintf；strncpy；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlInitAttrUint64(struct ncclXmlNode* node, const char* attrName, const uint64_t value) {
/* 中文逐项-变量 V1563：index
 * index（int；函数 xmlInitAttrUint64 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    snprintf(node->attrs[index].value, MAX_STR_LEN, "0x%lx", value);
  }
  return ncclSuccess;
}
/* 中文逐项-函数 F0515：xmlInitAttrFloat
 * 用途与位置：只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const float value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；snprintf；strncpy；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlInitAttrFloat(struct ncclXmlNode* node, const char* attrName, const float value) {
/* 中文逐项-变量 V1564：index
 * index（int；函数 xmlInitAttrFloat 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    snprintf(node->attrs[index].value, MAX_STR_LEN, "%f", value);
  }
  return ncclSuccess;
}


/* 中文导读：
 * 从 XML/系统设备信息、GPU/NVML 和所选网络后端属性建立本机拓扑。
 * NET 节点带有端口速度、GDR/collective 能力等属性；后续搜索把这些属性作为模型输入。
 */
/* 中文逐项-函数 F0516：ncclTopoGetSystem
 * 用途与位置：读入可选拓扑文件并补齐当前 rank 所见 GPU/NIC 的自动发现信息，再生成硬件图。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 system（struct ncclTopoSystem** system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；collNetDevices；collNetGetProperties；collNetSupport；free；getenv；int64ToBusId；ncclCalloc；ncclNetDevices；ncclNetGetProperties；ncclParamTopoDumpFileRank；ncclTopoDumpXmlToFile；ncclTopoFillGpu；ncclTopoFillNet；ncclTopoGetSystemFromXml；ncclTopoGetXmlFromFile；ncclTopoTrimXml；xmlAddNode；更多调用保留在函数体。
 * 需要对照的边界/推进条件：xmlTopoFile；xml->maxIndex == 0；r<comm->nRanks；comm->peerInfo[r].hostHash ==
 * comm->peerInfo[comm->rank].hostHash。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetSystem(struct ncclComm* comm, struct ncclTopoSystem** system) {
/* 中文逐项-变量 V1565：xml
 * xml（struct ncclXml；函数 ncclTopoGetSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&xml, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclXml* xml;
  NCCLCHECK(ncclCalloc(&xml, 1));
/* 中文逐项-变量 V1566：xmlTopoFile
 * xmlTopoFile（char；函数 ncclTopoGetSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 xmlTopoFile = getenv("NCCL_TOPO_FILE")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* xmlTopoFile = getenv("NCCL_TOPO_FILE");
  if (xmlTopoFile) {
    INFO(NCCL_ENV, "NCCL_TOPO_FILE set by environment to %s", xmlTopoFile);
    NCCLCHECK(ncclTopoGetXmlFromFile(xmlTopoFile, xml, 1));
  } else {
    // Try default XML topology location
    NCCLCHECK(ncclTopoGetXmlFromFile("/var/run/nvidia-topologyd/virtualTopology.xml", xml, 0));
  }
  if (xml->maxIndex == 0) {
    // Create top tag
/* 中文逐项-变量 V1567：top
 * top（struct ncclXmlNode；函数 ncclTopoGetSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlAddNode(xml, NULL, "system", &top));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclXmlNode* top;
    NCCLCHECK(xmlAddNode(xml, NULL, "system", &top));
    NCCLCHECK(xmlSetAttrInt(top, "version", NCCL_TOPO_XML_VERSION));
  }

  // Auto-detect GPUs if needed
  for (/* 中文逐项-变量 V1568：r
 * r（int；函数 ncclTopoGetSystem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->nRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->nRanks; r++) {
    if (comm->peerInfo[r].hostHash == comm->peerInfo[comm->rank].hostHash) {
/* 中文逐项-变量 V1569：busId
 * busId（char；函数 ncclTopoGetSystem 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(int64ToBusId(comm->peerInfo[r].busId, busId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
      NCCLCHECK(int64ToBusId(comm->peerInfo[r].busId, busId));
/* 中文逐项-变量 V1570：node
 * node（struct ncclXmlNode；函数 ncclTopoGetSystem 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoFillGpu(xml, busId, &node));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      struct ncclXmlNode* node;
      NCCLCHECK(ncclTopoFillGpu(xml, busId, &node));
      if (node == NULL) continue;
      NCCLCHECK(xmlSetAttrInt(node, "keep", 1));
      NCCLCHECK(xmlSetAttrInt(node, "rank", r));
      NCCLCHECK(xmlInitAttrInt(node, "gdr", comm->peerInfo[r].gdrSupport));
    }
  }
  // Auto-detect NICs if needed. net/collnet share the same xml/graph nodes,
  // so we start with collnet so that it has precedence.
/* 中文逐项-变量 V1571：netDevCount
 * netDevCount（int；函数 ncclTopoGetSystem 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 netDevCount = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int netDevCount = 0;
  if (collNetSupport()) {
    NCCLCHECK(collNetDevices(&netDevCount));
    for (/* 中文逐项-变量 V1572：n
 * n（int；函数 ncclTopoGetSystem 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<netDevCount；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<netDevCount; n++) {
/* 中文逐项-变量 V1573：props
 * props（ncclNetProperties_t；函数 ncclTopoGetSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(collNetGetProperties(n, &props));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      ncclNetProperties_t props;
      NCCLCHECK(collNetGetProperties(n, &props));
/* 中文逐项-变量 V1574：netNode
 * netNode（struct ncclXmlNode；函数 ncclTopoGetSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoFillNet(xml, props.pciPath, props.name, &netNode));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      struct ncclXmlNode* netNode;
      NCCLCHECK(ncclTopoFillNet(xml, props.pciPath, props.name, &netNode));
      NCCLCHECK(xmlSetAttrInt(netNode, "keep", 1));
      NCCLCHECK(xmlSetAttrInt(netNode, "dev", n));
      NCCLCHECK(xmlInitAttrInt(netNode, "speed", props.speed));
      NCCLCHECK(xmlInitAttrInt(netNode, "port", props.port));
      NCCLCHECK(xmlInitAttrUint64(netNode, "guid", props.guid));
      NCCLCHECK(xmlInitAttrInt(netNode, "maxconn", props.maxComms));
      NCCLCHECK(xmlInitAttrInt(netNode, "gdr", props.ptrSupport & NCCL_PTR_CUDA ? 1 : 0));
      NCCLCHECK(xmlInitAttrInt(netNode, "coll", 1));
    }
  }
  if (netDevCount == 0) {
    NCCLCHECK(ncclNetDevices(&netDevCount));
  }
  for (/* 中文逐项-变量 V1575：n
 * n（int；函数 ncclTopoGetSystem 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<netDevCount；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<netDevCount; n++) {
/* 中文逐项-变量 V1576：props
 * props（ncclNetProperties_t；函数 ncclTopoGetSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclNetGetProperties(n, &props));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    ncclNetProperties_t props;
    NCCLCHECK(ncclNetGetProperties(n, &props));
/* 中文逐项-变量 V1577：netNode
 * netNode（struct ncclXmlNode；函数 ncclTopoGetSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoFillNet(xml, props.pciPath, props.name, &netNode));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclXmlNode* netNode;
    NCCLCHECK(ncclTopoFillNet(xml, props.pciPath, props.name, &netNode));
    NCCLCHECK(xmlSetAttrInt(netNode, "keep", 1));
    NCCLCHECK(xmlSetAttrInt(netNode, "dev", n));
    NCCLCHECK(xmlInitAttrInt(netNode, "speed", props.speed));
    NCCLCHECK(xmlInitAttrInt(netNode, "port", props.port));
    NCCLCHECK(xmlInitAttrFloat(netNode, "latency", props.latency));
    NCCLCHECK(xmlInitAttrUint64(netNode, "guid", props.guid));
    NCCLCHECK(xmlInitAttrInt(netNode, "maxconn", props.maxComms));
    NCCLCHECK(xmlInitAttrInt(netNode, "gdr", props.ptrSupport & NCCL_PTR_CUDA ? 1 : 0));
  }

  // Remove XML branches which don't have a node with keep="1" (typically when importing a topology)
  NCCLCHECK(ncclTopoTrimXml(xml));

  xmlTopoFile = getenv("NCCL_TOPO_DUMP_FILE");
  if (xmlTopoFile && comm->rank == ncclParamTopoDumpFileRank()) {
    INFO(NCCL_ENV, "NCCL_TOPO_DUMP_FILE set by environment to %s", xmlTopoFile);
    NCCLCHECK(ncclTopoDumpXmlToFile(xmlTopoFile, xml));
  }

  NCCLCHECK(ncclTopoGetSystemFromXml(xml, system));
  free(xml);
  return ncclSuccess;
}

/* 中文逐项-函数 F0517：ncclTopoGetLocalNet
 * 用途与位置：按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 id（int* id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputePaths；src/graph/search.cc::ncclTopoGetNetDev；src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；free；ncclCalloc；ncclTopoRankToIndex。
 * 需要对照的边界/推进条件：n<system->nodes[NET].count；path->width > maxWidth || (path->width == maxWidth && path->type <
 * minType；path->width == maxWidth && path->type == minType；count == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetLocalNet(struct ncclTopoSystem* system, int rank, int* id) {
/* 中文逐项-变量 V1578：g
 * g（int；函数 ncclTopoGetLocalNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int g;
  NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));
/* 中文逐项-变量 V1579：minType
 * minType（int；函数 ncclTopoGetLocalNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minType = PATH_SYS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int minType = PATH_SYS;
/* 中文逐项-变量 V1580：maxWidth
 * maxWidth（float；函数 ncclTopoGetLocalNet 内）：硬件系统最大单路带宽和可聚合宽度，用于设置搜索速度与提前终止条件。
 * 初值/来源 maxWidth = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  float maxWidth = 0;
/* 中文逐项-变量 V1581：count
 * count（int；函数 ncclTopoGetLocalNet 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 初值/来源 count = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int count = 0;
/* 中文逐项-变量 V1582：nets
 * nets（int；函数 ncclTopoGetLocalNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int* nets;
  NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));
  for (/* 中文逐项-变量 V1583：n
 * n（int；函数 ncclTopoGetLocalNet 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[NET].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[NET].count; n++) {
/* 中文逐项-变量 V1584：path
 * path（struct ncclTopoLinkList；函数 ncclTopoGetLocalNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 path = system->nodes[NET].nodes[n].paths[GPU]+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoLinkList* path = system->nodes[NET].nodes[n].paths[GPU]+g;
    if (path->width > maxWidth || (path->width == maxWidth && path->type < minType)) {
      maxWidth = path->width;
      minType = path->type;
      count = 0;
    }
    if (path->width == maxWidth && path->type == minType) nets[count++] = system->nodes[NET].nodes[n].id;
  }
  if (count == 0) {
    *id = -1;
    free(nets);
    return ncclSuccess;
  }

/* 中文逐项-变量 V1585：rr
 * rr（int；函数 ncclTopoGetLocalNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rr = system->nodes[GPU].nodes[g].gpu.dev；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rr = system->nodes[GPU].nodes[g].gpu.dev;
  *id = nets[rr%count];
  free(nets);
  return ncclSuccess;
}

/****************************/
/* External query functions */
/****************************/

/* 中文逐项-函数 F0518：ncclTopoCpuType
 * 用途与位置：从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 arch（int* arch）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 * 参数 vendor（int* vendor）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 * 参数 model（int* model）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoCheckP2p；src/graph/tuning.cc::ncclTopoTuneModel；src/init.cc::computeBuffSizes。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoCpuType(struct ncclTopoSystem* system, int* arch, int* vendor, int* model) {
  *arch = system->nodes[CPU].nodes[0].cpu.arch;
  *vendor = system->nodes[CPU].nodes[0].cpu.vendor;
  *model = system->nodes[CPU].nodes[0].cpu.model;
  return ncclSuccess;
}

/* 中文逐项-参数getter P0017：IgnoreCpuAffinity
 * 生成函数 ncclParamIgnoreCpuAffinity()，读取 NCCL_IGNORE_CPU_AFFINITY，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IgnoreCpuAffinity, "IGNORE_CPU_AFFINITY", 0);

/* 中文逐项-函数 F0519：ncclTopoGetCpuAffinity
 * 用途与位置：结合 GPU 的近端 CPU/NUMA 亲和范围与当前进程约束，生成线程亲和性掩码。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 affinity（cpu_set_t* affinity）：CPU集合掩码，约束后台线程尽量靠近所属GPU/NIC的NUMA范围。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：CPU_AND；CPU_COUNT；NCCLCHECK；SYSCHECK；memcpy；ncclCpusetToStr；ncclParamIgnoreCpuAffinity；sched_getaffinity。
 * 需要对照的边界/推进条件：g<system->nodes[GPU].count；system->nodes[GPU].nodes[g].gpu.rank ==
 * rank；c<system->nodes[CPU].count；cpuIndex == -1 || nHops < minHops。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetCpuAffinity(struct ncclTopoSystem* system, int rank, cpu_set_t* affinity) {
/* 中文逐项-变量 V1586：cpu、gpu
 * cpu（struct ncclTopoNode；函数 ncclTopoGetCpuAffinity 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 cpu = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 * gpu（struct ncclTopoNode；函数 ncclTopoGetCpuAffinity 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclTopoNode* cpu = NULL, *gpu = NULL;
  for (/* 中文逐项-变量 V1587：g
 * g（int；函数 ncclTopoGetCpuAffinity 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
    if (system->nodes[GPU].nodes[g].gpu.rank == rank) {
      gpu = system->nodes[GPU].nodes+g;
      // Find closer CPU
/* 中文逐项-变量 V1588：cpuIndex、minHops
 * cpuIndex（int；函数 ncclTopoGetCpuAffinity 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cpuIndex = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 * minHops（int；函数 ncclTopoGetCpuAffinity 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minHops = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int cpuIndex = -1, minHops = 0;
      for (/* 中文逐项-变量 V1589：c
 * c（int；函数 ncclTopoGetCpuAffinity 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<system->nodes[CPU].count；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<system->nodes[CPU].count; c++) {
/* 中文逐项-变量 V1590：nHops
 * nHops（int；函数 ncclTopoGetCpuAffinity 内）：整个候选通信图累计的路径跳数，用于同带宽候选之间的比较。
 * 初值/来源 nHops = system->nodes[GPU].nodes[g].paths[CPU][c].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        int nHops = system->nodes[GPU].nodes[g].paths[CPU][c].count;
        if (cpuIndex == -1 || nHops < minHops) {
          cpuIndex = c;
          minHops = nHops;
        }
      }
      cpu = system->nodes[CPU].nodes+cpuIndex;
    }
  }
  if (cpu == NULL) {
    WARN("Set CPU affinity : unable to find GPU/CPU for rank %d", rank);
    return ncclInternalError;
  }

  // Query the CPU affinity set we were provided
/* 中文逐项-变量 V1591：mask
 * mask（cpu_set_t；函数 ncclTopoGetCpuAffinity 内）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 * 此处未显式给初值；后续首个文本使用点：SYSCHECK(sched_getaffinity(0, sizeof(cpu_set_t), &mask),
 * "sched_getaffinity");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cpu_set_t mask;
  SYSCHECK(sched_getaffinity(0, sizeof(cpu_set_t), &mask), "sched_getaffinity");

#ifdef ENABLE_TRACE
  {
/* 中文逐项-变量 V1592：affinityStr
 * affinityStr（char；函数 ncclTopoGetCpuAffinity 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCpusetToStr(&mask, affinityStr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char affinityStr[sizeof(cpu_set_t)*2];
    NCCLCHECK(ncclCpusetToStr(&mask, affinityStr));
    TRACE(NCCL_INIT, "Current affinity for GPU %d is %s", gpu->gpu.dev, affinityStr);
  }
#endif

  // Get the affinity of the CPU close to our GPU.
/* 中文逐项-变量 V1593：cpuMask
 * cpuMask（cpu_set_t；函数 ncclTopoGetCpuAffinity 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cpuMask = cpu->cpu.affinity；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  cpu_set_t cpuMask = cpu->cpu.affinity;

#ifdef ENABLE_TRACE
  {
/* 中文逐项-变量 V1594：affinityStr
 * affinityStr（char；函数 ncclTopoGetCpuAffinity 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCpusetToStr(&cpuMask, affinityStr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char affinityStr[sizeof(cpu_set_t)*2];
    NCCLCHECK(ncclCpusetToStr(&cpuMask, affinityStr));
    TRACE(NCCL_INIT, "CPU GPU affinity for GPU %d is %s", gpu->gpu.dev, affinityStr);
  }
#endif

/* 中文逐项-变量 V1595：finalMask
 * finalMask（cpu_set_t；函数 ncclTopoGetCpuAffinity 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：finalMask = cpuMask;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cpu_set_t finalMask;
  if (ncclParamIgnoreCpuAffinity())
    // Ignore the CPU affinity set and use the GPU one instead
    finalMask = cpuMask;
  else
    // Use a subset of the GPU affinity set
    CPU_AND(&finalMask, &mask, &cpuMask);

  memcpy(affinity, &finalMask, sizeof(cpu_set_t));

  // If there is a non empty set, use it to set affinity
  if (CPU_COUNT(&finalMask)) {
/* 中文逐项-变量 V1596：affinityStr
 * affinityStr（char；函数 ncclTopoGetCpuAffinity 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCpusetToStr(&finalMask, affinityStr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char affinityStr[sizeof(cpu_set_t)*2];
    NCCLCHECK(ncclCpusetToStr(&finalMask, affinityStr));
    INFO(NCCL_INIT, "Setting affinity for GPU %d to %s", gpu->gpu.dev, affinityStr);
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0520：ncclTopoGetNetCount
 * 用途与位置：返回当前裁剪后硬件图中的 NET 设备数量，用于判断是否存在网络路径。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 count（int* count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetNetCount(struct ncclTopoSystem* system, int* count) {
  *count = system->nodes[NET].count;
  return ncclSuccess;
}

/* 中文逐项-函数 F0521：ncclTopoGetCompCap
 * 用途与位置：遍历 GPU 得到计算能力的最小/最大值，使协议与调优模型兼容参与设备。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 ccMin（int* ccMin）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ccMax（int* ccMax）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoCompute。
 * 本体直接出现的调用/包装：std::max；std::min。
 * 需要对照的边界/推进条件：system->nodes[GPU].count == 0；g<system->nodes[GPU].count；ccMin；ccMax。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetCompCap(struct ncclTopoSystem* system, int* ccMin, int* ccMax) {
  if (system->nodes[GPU].count == 0) return ncclInternalError;
/* 中文逐项-变量 V1597：min、max
 * min（int；函数 ncclTopoGetCompCap 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：min = max = system->nodes[GPU].nodes[0].gpu.cudaCompCap;。若由输出参数填入，必须先检查填充调用成功再读值。
 * max（int；函数 ncclTopoGetCompCap 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：min = max = system->nodes[GPU].nodes[0].gpu.cudaCompCap;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int min, max;
  min = max = system->nodes[GPU].nodes[0].gpu.cudaCompCap;
  for (/* 中文逐项-变量 V1598：g
 * g（int；函数 ncclTopoGetCompCap 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=1; g<system->nodes[GPU].count; g++) {
    min = std::min(min, system->nodes[GPU].nodes[g].gpu.cudaCompCap);
    max = std::max(max, system->nodes[GPU].nodes[g].gpu.cudaCompCap);
  }
  if (ccMin) *ccMin = min;
  if (ccMax) *ccMax = max;
  return ncclSuccess;
}

/* 中文逐项-函数 F0522：ncclTopoGetLocalRank
 * 用途与位置：把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 localRank（int* localRank）：本节点内 rank 下标，主要索引本地共享资源。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyConnect；src/transport/coll_net.cc::sendSetup；src/transport/coll_net.cc::recvSetup；src/transport/net.cc::sendSetup；src/transport/net.cc::recvSetup。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：g<system->nodes[GPU].count；system->nodes[GPU].nodes[g].gpu.rank == rank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetLocalRank(struct ncclTopoSystem* system, int rank, int* localRank) {
  for (/* 中文逐项-变量 V1599：g
 * g（int；函数 ncclTopoGetLocalRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
    if (system->nodes[GPU].nodes[g].gpu.rank == rank) {
      *localRank = g;
      return ncclSuccess;
    }
  }
  WARN("Could not find local GPU with rank %d\n", rank);
  return ncclInternalError;
}
