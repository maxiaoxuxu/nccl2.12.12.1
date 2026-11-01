/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "core.h"
#include "graph.h"
#include "topo.h"
#include "xml.h"
#include <math.h>

// Initialize system->maxWidth. This is the per-channel (i.e. per-SM)
// max speed.
/* 中文内部逻辑 F0462：getMaxWidth
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：float maxWidth = 0.0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 int i=0;；继续条件 i<system->nodes[type].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 准备局部状态：struct ncclTopoLinkList* path = gpu->paths[type]+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   4. 准备局部状态：float width = path->width;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   5. 条件判断：(path->count == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     6. 真分支：；仅条件为真执行以下子项。
 *       7. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   8. 调用 / 计算 / 写入：maxWidth = std::max(maxWidth, width);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 10. 函数返回：return maxWidth;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0462
 */
/* 中文逐项-函数 F0462：getMaxWidth
 * 用途与位置：查看当前 GPU 到指定硬件类型的候选路径，获得可供搜索的最大单路径带宽。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 gpu（struct ncclTopoNode* gpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchInit。
 * 本体直接出现的调用/包装：std::max。
 * 需要对照的边界/推进条件：i<system->nodes[type].count；path->count == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return maxWidth;。CHECK类宏还可能提前返回/跳转。
 */
static float getMaxWidth(struct ncclTopoSystem* system, struct ncclTopoNode* gpu, int type) {
/* 中文逐项-变量 V1290：maxWidth
 * maxWidth（float；函数 getMaxWidth 内）：硬件系统最大单路带宽和可聚合宽度，用于设置搜索速度与提前终止条件。
 * 初值/来源 maxWidth = 0.0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  float maxWidth = 0.0;
  for (/* 中文逐项-变量 V1291：i
 * i（int；函数 getMaxWidth 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[type].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[type].count; i++) {
/* 中文逐项-变量 V1292：path
 * path（struct ncclTopoLinkList；函数 getMaxWidth 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 path = gpu->paths[type]+i；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoLinkList* path = gpu->paths[type]+i;
/* 中文逐项-变量 V1293：width
 * width（float；函数 getMaxWidth 内）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 初值/来源 width = path->width；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    float width = path->width;
    if (path->count == 0) continue;
    maxWidth = std::max(maxWidth, width);
  }
  return maxWidth;
}
/* 中文内部逻辑 F0463：getTotalWidth
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：float nvlinkWidth = 0.0, pciWidth = 0.0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 int l=0;；继续条件 l<gpu->nlinks；每轮更新 l++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 准备局部状态：struct ncclTopoLink* link = gpu->links+l;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   4. 条件判断：(link->type == LINK_NVL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 调用 / 计算 / 写入：nvlinkWidth += link->width;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   7. 条件判断：(link->type == LINK_PCI)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     8. 真分支：；仅条件为真执行以下子项。
 *       9. 调用 / 计算 / 写入：pciWidth = link->width;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 函数返回：return std::max(pciWidth, nvlinkWidth);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0463
 */
/* 中文逐项-函数 F0463：getTotalWidth
 * 用途与位置：估计当前 GPU 可同时利用的链路总带宽，作为多 channel 搜索的容量上界。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 gpu（struct ncclTopoNode* gpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchInit。
 * 本体直接出现的调用/包装：std::max。
 * 需要对照的边界/推进条件：l<gpu->nlinks；link->type == LINK_NVL；link->type == LINK_PCI。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return std::max(pciWidth, nvlinkWidth);。CHECK类宏还可能提前返回/跳转。
 */
static float getTotalWidth(struct ncclTopoSystem* system, struct ncclTopoNode* gpu) {
/* 中文逐项-变量 V1294：nvlinkWidth、pciWidth
 * nvlinkWidth（float；函数 getTotalWidth 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvlinkWidth = 0.0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * pciWidth（float；函数 getTotalWidth 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pciWidth = 0.0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  float nvlinkWidth = 0.0, pciWidth = 0.0;
  for (/* 中文逐项-变量 V1295：l
 * l（int；函数 getTotalWidth 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<gpu->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<gpu->nlinks; l++) {
/* 中文逐项-变量 V1296：link
 * link（struct ncclTopoLink；函数 getTotalWidth 内）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 初值/来源 link = gpu->links+l；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoLink* link = gpu->links+l;
    if (link->type == LINK_NVL) nvlinkWidth += link->width;
    if (link->type == LINK_PCI) pciWidth = link->width;
  }
  return std::max(pciWidth, nvlinkWidth);
}
/* 中文内部逻辑 F0464：ncclTopoSearchInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：system->maxWidth = 0.0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：system->totalWidth = 0.0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 准备局部状态：int inter = system->nodes[NET].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 条件判断：(inter == 0 && system->nodes[GPU].count == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：system->maxWidth = LOC_WIDTH;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   9. 准备局部状态：struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   10. 调用 / 计算 / 写入：system->maxWidth = std::max(system->maxWidth, getMaxWidth(system, gpu, inter ? NET : GPU));；查看当前 GPU 到指定硬件类型的候选路径，获得可供搜索的最大单路径带宽。
 *   11. 调用 / 计算 / 写入：system->totalWidth = std::max(system->totalWidth, getTotalWidth(system, gpu));；估计当前 GPU 可同时利用的链路总带宽，作为多 channel 搜索的容量上界。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0464
 */
/* 中文逐项-函数 F0464：ncclTopoSearchInit
 * 用途与位置：初始化拓扑搜索的最大宽度和总宽度，建立速度候选与提前终止判断依据。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：getMaxWidth；getTotalWidth；std::max。
 * 需要对照的边界/推进条件：inter == 0 && system->nodes[GPU].count == 1；g<system->nodes[GPU].count。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSearchInit(struct ncclTopoSystem* system) {
  system->maxWidth = 0.0;
  system->totalWidth = 0.0;
/* 中文逐项-变量 V1297：inter
 * inter（int；函数 ncclTopoSearchInit 内）：按channel记录机内GPU用户rank排列及两个网络端点，后续转成具体邻居连接。
 * 初值/来源 inter = system->nodes[NET].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int inter = system->nodes[NET].count;
  if (inter == 0 && system->nodes[GPU].count == 1) {
    system->maxWidth = LOC_WIDTH;
    return ncclSuccess;
  }
  for (/* 中文逐项-变量 V1298：g
 * g（int；函数 ncclTopoSearchInit 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
/* 中文逐项-变量 V1299：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoSearchInit 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = system->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
    system->maxWidth = std::max(system->maxWidth, getMaxWidth(system, gpu, inter ? NET : GPU));
    system->totalWidth = std::max(system->totalWidth, getTotalWidth(system, gpu));
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0465：findRevLink
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int l=0;；继续条件 l<node2->nlinks；每轮更新 l++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 准备局部状态：struct ncclTopoLink* link = node2->links+l;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(link->remNode == node1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：*revLink = link;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 调用 / 计算 / 写入：WARN("Could not find rev link for %d/%ld -> %d/%ld", node1->type, node1->id, node2->type, node2->id);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0465
 */
/* 中文逐项-函数 F0465：findRevLink
 * 用途与位置：在对端节点中找回指向原节点的反向链路，配合双向带宽占用/恢复。
 * 参数 node1（struct ncclTopoNode* node1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 node2（struct ncclTopoNode* node2）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 revLink（struct ncclTopoLink** revLink）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::followPath。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：l<node2->nlinks；link->remNode == node1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t findRevLink(struct ncclTopoNode* node1, struct ncclTopoNode* node2, struct ncclTopoLink** revLink) {
  for (/* 中文逐项-变量 V1300：l
 * l（int；函数 findRevLink 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<node2->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<node2->nlinks; l++) {
/* 中文逐项-变量 V1301：link
 * link（struct ncclTopoLink；函数 findRevLink 内）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 初值/来源 link = node2->links+l；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoLink* link = node2->links+l;
    if (link->remNode == node1) {
      *revLink = link;
      return ncclSuccess;
    }
  }
  WARN("Could not find rev link for %d/%ld -> %d/%ld", node1->type, node1->id, node2->type, node2->id);
  return ncclInternalError;
}

// This is unfortunately needed since manipulating floats often results in rounding errors.
/* 中文逐项-宏 M0044：SUB_ROUND
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 SUB_ROUND；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define SUB_ROUND(a, b) (a = roundf((a-b)*1000)/1000)

/* 中文精读：搜索阶段如何扣减链路带宽
 * 搜索一个 channel 时，沿候选硬件路径暂时扣除所需 speed；容量不足就不能再走这条边。
 * 回溯时用相反符号恢复已占带宽，才能尝试另一组 GPU/NIC 顺序。
 * 这只是主机内存中的拓扑模型计算，不是在配置 NVLink 路由器或 NIC 硬件带宽限额。
 * 多个逻辑 channel 可以复用同一物理路径，因此 channel 数没有“等于物理链路数”的约束。
 */
/* 中文内部逻辑 F0466：followPath
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：float pciSpeed = speed;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 int step=0;；继续条件 step<path->count；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 准备局部状态：struct ncclTopoNode* node = path->list[step]->remNode;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *   4. 条件判断：(node->type == CPU)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 条件判断：(path->type == PATH_PHB && start->type == GPU && node->cpu.arch == NCCL_TOPO_CPU_ARCH_X86 && node->cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         7. 真分支：；仅条件为真执行以下子项。
 *           8. 调用 / 计算 / 写入：pciSpeed = INTEL_P2P_OVERHEAD(speed);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 10. 准备局部状态：struct ncclTopoNode* node = start;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 循环：初始 int step=0;；继续条件 step<maxSteps；每轮更新 step++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   12. 准备局部状态：struct ncclTopoLink* link = path->list[step];；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *   13. 准备局部状态：struct ncclTopoLink* revLink = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   14. 准备局部状态：float fwSpeed = link->type == LINK_PCI ? pciSpeed : speed;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   15. 准备局部状态：float revSpeed = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   16. 条件判断：(link->remNode->type == GPU && link->remNode->gpu.cudaCompCap < 80 && start->type != GPU)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     17. 真分支：；仅条件为真执行以下子项。
 *       18. 条件判断：(revLink == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         19. 真分支：；仅条件为真执行以下子项。
 *           20. 调用 / 计算 / 写入：NCCLCHECK(findRevLink(node, link->remNode, &revLink));；失败时可能提前return，成功才继续；在对端节点中找回指向原节点的反向链路，配合双向带宽占用/恢复。
 *       21. 调用 / 计算 / 写入：revSpeed += fwSpeed/8;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   22. 条件判断：(link->remNode->type == CPU && link->type == LINK_NVL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     23. 真分支：；仅条件为真执行以下子项。
 *       24. 条件判断：(revLink == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         25. 真分支：；仅条件为真执行以下子项。
 *           26. 调用 / 计算 / 写入：NCCLCHECK(findRevLink(node, link->remNode, &revLink));；失败时可能提前return，成功才继续；在对端节点中找回指向原节点的反向链路，配合双向带宽占用/恢复。
 *       27. 调用 / 计算 / 写入：revSpeed += fwSpeed;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   28. 条件判断：(link->width < fwSpeed || (revSpeed && revLink->width < revSpeed))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     29. 真分支：；仅条件为真执行以下子项。
 *       30. 调用 / 计算 / 写入：*steps = step;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *       31. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   32. 调用 / 计算 / 写入：SUB_ROUND(link->width, fwSpeed);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   33. 条件判断：(revSpeed)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     34. 真分支：；仅条件为真执行以下子项。
 *       35. 调用 / 计算 / 写入：SUB_ROUND(revLink->width, revSpeed);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   36. 调用 / 计算 / 写入：node = link->remNode;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 37. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 38. 调用 / 计算 / 写入：*steps = maxSteps;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 39. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0466
 */
/* 中文逐项-函数 F0466：followPath
 * 用途与位置：沿硬件路径扣减候选 channel 所需带宽，若失败则报告已走步数以便恢复；负 speed 用于回溯。
 * 参数 path（struct ncclTopoLinkList* path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 start（struct ncclTopoNode* start）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 maxSteps（int maxSteps）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 speed（float speed）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 参数 steps（int* steps）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoFollowPath。
 * 本体直接出现的调用/包装：INTEL_P2P_OVERHEAD；NCCLCHECK；SUB_ROUND；findRevLink。
 * 需要对照的边界/推进条件：step<path->count；node->type == CPU；path->type == PATH_PHB && start->type == GPU && node->cpu.arch ==
 * NCCL_TOPO_CPU_ARCH_X86 && node->cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL；step<maxSteps。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t followPath(struct ncclTopoLinkList* path, struct ncclTopoNode* start, int maxSteps, float speed, int* steps) {
/* 中文逐项-变量 V1302：pciSpeed
 * pciSpeed（float；函数 followPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pciSpeed = speed；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  float pciSpeed = speed;
  for (/* 中文逐项-变量 V1303：step
 * step（int；函数 followPath 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：step<path->count；循环体与增量表达式决定每轮如何推进。
 */int step=0; step<path->count; step++) {
/* 中文逐项-变量 V1304：node
 * node（struct ncclTopoNode；函数 followPath 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = path->list[step]->remNode；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* node = path->list[step]->remNode;
    if (node->type == CPU) {
      // Account for P2P inefficiency through Intel CPU RC
      if (path->type == PATH_PHB && start->type == GPU &&
          node->cpu.arch == NCCL_TOPO_CPU_ARCH_X86 &&
          node->cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL) {
        pciSpeed = INTEL_P2P_OVERHEAD(speed);
      }
    }
  }

/* 中文逐项-变量 V1305：node
 * node（struct ncclTopoNode；函数 followPath 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = start；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  struct ncclTopoNode* node = start;
  for (/* 中文逐项-变量 V1306：step
 * step（int；函数 followPath 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：step<maxSteps；循环体与增量表达式决定每轮如何推进。
 */int step=0; step<maxSteps; step++) {
/* 中文逐项-变量 V1307：link
 * link（struct ncclTopoLink；函数 followPath 内）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 初值/来源 link = path->list[step]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoLink* link = path->list[step];
/* 中文逐项-变量 V1308：revLink
 * revLink（struct ncclTopoLink；函数 followPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 revLink = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    struct ncclTopoLink* revLink = NULL;
/* 中文逐项-变量 V1309：fwSpeed
 * fwSpeed（float；函数 followPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 fwSpeed = link->type == LINK_PCI ? pciSpeed : speed；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    float fwSpeed = link->type == LINK_PCI ? pciSpeed : speed;
/* 中文逐项-变量 V1310：revSpeed
 * revSpeed（float；函数 followPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 revSpeed = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    float revSpeed = 0;
    if (link->remNode->type == GPU && link->remNode->gpu.cudaCompCap < 80 && start->type != GPU) {
      if (revLink == NULL) NCCLCHECK(findRevLink(node, link->remNode, &revLink));
      revSpeed += fwSpeed/8;
    }
    if (link->remNode->type == CPU && link->type == LINK_NVL) {
      if (revLink == NULL) NCCLCHECK(findRevLink(node, link->remNode, &revLink));
      revSpeed += fwSpeed;
    }
    if (link->width < fwSpeed || (revSpeed && revLink->width < revSpeed)) { *steps = step; return ncclSuccess; }
    SUB_ROUND(link->width, fwSpeed);
    if (revSpeed) SUB_ROUND(revLink->width, revSpeed);
    node = link->remNode;
  }
  *steps = maxSteps;
  return ncclSuccess;
}

// Try to go from node type1/index1 to no type2/index2. mult indicates whether we are counting the bandwidth (1) or undoing (-1).
/* 中文内部逻辑 F0467：ncclTopoFollowPath
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*node = system->nodes[type2].nodes+index2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 条件判断：(type1 == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：struct ncclTopoNode* node1 = system->nodes[type1].nodes+index1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：struct ncclTopoLinkList* path = node1->paths[type2]+index2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 条件判断：(path->count == 0 )；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 调用 / 计算 / 写入：*node = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 准备局部状态：int intra = type1 == GPU && type2 == GPU;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 准备局部状态：float speed = intra ? graph->speedIntra : graph->speedInter;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：int type = intra ? graph->typeIntra : graph->typeInter;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 条件判断：(mult == 1 && (path->type > type))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 17. 调用 / 计算 / 写入：speed *= mult;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 准备局部状态：int step = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 19. 调用 / 计算 / 写入：NCCLCHECK(followPath(path, node1, path->count, speed, &step));；失败时可能提前return，成功才继续；沿硬件路径扣减候选 channel 所需带宽，若失败则报告已走步数以便恢复；负 speed 用于回溯。
 * 20. 条件判断：(step < path->count)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   21. 真分支：；仅条件为真执行以下子项。
 *     22. 控制转移：goto rewind;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 23. 调用 / 计算 / 写入：graph->nHops += mult*path->count;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 24. 调用 / 计算 / 写入：*node = system->nodes[type2].nodes+index2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 25. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 26. 跳转标签：rewind；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   27. 调用 / 计算 / 写入：NCCLCHECK(followPath(path, node1, step, -speed, &step));；失败时可能提前return，成功才继续；沿硬件路径扣减候选 channel 所需带宽，若失败则报告已走步数以便恢复；负 speed 用于回溯。
 * 28. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0467
 */
/* 中文逐项-函数 F0467：ncclTopoFollowPath
 * 用途与位置：包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 type1（int type1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 index1（int index1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 type2（int type2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 index2（int index2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 mult（int mult）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 node（struct ncclTopoNode** node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchTryGpu；src/graph/search.cc::ncclTopoSearchRecGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；followPath。
 * 需要对照的边界/推进条件：type1 == -1；path->count == 0；mult == 1 && (path->type > type；step < path->count。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclTopoFollowPath(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, int type1, int index1, int type2, int index2, int mult, struct ncclTopoNode** node) {
  // First handle easy cases
  *node = system->nodes[type2].nodes+index2;
  if (type1 == -1) return ncclSuccess;
/* 中文逐项-变量 V1311：node1
 * node1（struct ncclTopoNode；函数 ncclTopoFollowPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 node1 = system->nodes[type1].nodes+index1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* node1 = system->nodes[type1].nodes+index1;
/* 中文逐项-变量 V1312：path
 * path（struct ncclTopoLinkList；函数 ncclTopoFollowPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 path = node1->paths[type2]+index2；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoLinkList* path = node1->paths[type2]+index2;
  if (path->count == 0 ) return ncclSuccess;

  // Now check link type
  *node = NULL;
/* 中文逐项-变量 V1313：intra
 * intra（int；函数 ncclTopoFollowPath 内）：按channel记录机内GPU用户rank排列及两个网络端点，后续转成具体邻居连接。
 * 初值/来源 intra = type1 == GPU && type2 == GPU；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int intra = type1 == GPU && type2 == GPU;
/* 中文逐项-变量 V1314：speed
 * speed（float；函数 ncclTopoFollowPath 内）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 初值/来源 speed = intra ? graph->speedIntra : graph->speedInter；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  float speed = intra ? graph->speedIntra : graph->speedInter;
/* 中文逐项-变量 V1315：type
 * type（int；函数 ncclTopoFollowPath 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 初值/来源 type = intra ? graph->typeIntra : graph->typeInter；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int type = intra ? graph->typeIntra : graph->typeInter;

  if (mult == 1 && (path->type > type)) return ncclSuccess;

  speed *= mult;

  // Check there is enough bandwidth on paths.
/* 中文逐项-变量 V1316：step
 * step（int；函数 ncclTopoFollowPath 内）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 初值/来源 step = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int step = 0;
  NCCLCHECK(followPath(path, node1, path->count, speed, &step));
  if (step < path->count) goto rewind;

  // Enough bandwidth : return destination node.
  graph->nHops += mult*path->count;
  *node = system->nodes[type2].nodes+index2;
  return ncclSuccess;

rewind:
  // Not enough bandwidth : rewind and exit.
  NCCLCHECK(followPath(path, node1, step, -speed, &step));
  return ncclSuccess;
}

/* 中文内部逻辑 F0468：gpuPciWidth
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int l=0;；继续条件 l<gpu->nlinks；每轮更新 l++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 准备局部状态：struct ncclTopoLink* gpuLink = gpu->links+l;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(gpuLink->type != LINK_PCI)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   6. 准备局部状态：struct ncclTopoNode* pci = gpuLink->remNode;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   7. 循环：初始 int l=0;；继续条件 l<pci->nlinks；每轮更新 l++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     8. 准备局部状态：struct ncclTopoLink* pciLink = pci->links+l;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 条件判断：(pciLink->remNode != gpu)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       10. 真分支：；仅条件为真执行以下子项。
 *         11. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *     12. 函数返回：return std::min(gpuLink->width, pciLink->width);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 15. 函数返回：return -1;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0468
 */
/* 中文逐项-函数 F0468：gpuPciWidth
 * 用途与位置：统计 GPU 的 PCI 通路宽度，为下一 GPU 候选排序加入 PCI 容量因素。
 * 参数 gpu（struct ncclTopoNode* gpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchNextGpuSort；src/graph/search.cc::ncclTopoSearchRecNet。
 * 本体直接出现的调用/包装：std::min。
 * 需要对照的边界/推进条件：l<gpu->nlinks；gpuLink->type != LINK_PCI；l<pci->nlinks；pciLink->remNode != gpu。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return std::min(gpuLink->width, pciLink->width);；return -1;。CHECK类宏还可能提前返回/跳转。
 */
static int gpuPciWidth(struct ncclTopoNode* gpu) {
  for (/* 中文逐项-变量 V1317：l
 * l（int；函数 gpuPciWidth 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<gpu->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<gpu->nlinks; l++) {
/* 中文逐项-变量 V1318：gpuLink
 * gpuLink（struct ncclTopoLink；函数 gpuPciWidth 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gpuLink = gpu->links+l；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoLink* gpuLink = gpu->links+l;
    if (gpuLink->type != LINK_PCI) continue;
/* 中文逐项-变量 V1319：pci
 * pci（struct ncclTopoNode；函数 gpuPciWidth 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 pci = gpuLink->remNode；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* pci = gpuLink->remNode;
    for (/* 中文逐项-变量 V1320：l
 * l（int；函数 gpuPciWidth 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<pci->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<pci->nlinks; l++) {
/* 中文逐项-变量 V1321：pciLink
 * pciLink（struct ncclTopoLink；函数 gpuPciWidth 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pciLink = pci->links+l；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoLink* pciLink = pci->links+l;
      if (pciLink->remNode != gpu) continue;
      return std::min(gpuLink->width, pciLink->width);
    }
  }
  return -1;
}

/* Choose the order in which we try next GPUs. This is critical for the search
   to quickly converge to the best solution even if it eventually times out. */
/* 中文逐项-类型 T0136：ncclGpuScore
 * 类型职责：下一GPU候选的跳数/机内带宽/机间带宽评分，用于调整启发式搜索先后。
 * 声明内可见成员（含嵌套成员）：g、startIndex、intraNhops、intraWidth、interNhops、interPciWidth、interWidth。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclGpuScore {
/* 中文逐项-变量 V1322：g
 * g（int；成员 ncclGpuScore）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 */
  int g;             // Retain the index
/* 中文逐项-变量 V1323：startIndex
 * startIndex（int；成员 ncclGpuScore）：候选/图重放的起始下标，用于避免重复遍历并保持顺序。
 */
  int startIndex;    // Least important
/* 中文逐项-变量 V1324：intraNhops
 * intraNhops（int；成员 ncclGpuScore）：下一GPU候选排序的机内/机间跳数与带宽评分，控制搜索优先顺序。
 */
  int intraNhops;
/* 中文逐项-变量 V1325：intraWidth
 * intraWidth（int；成员 ncclGpuScore）：下一GPU候选排序的机内/机间跳数与带宽评分，控制搜索优先顺序。
 */
  int intraWidth;
/* 中文逐项-变量 V1326：interNhops
 * interNhops（int；成员 ncclGpuScore）：下一GPU候选排序的机内/机间跳数与带宽评分，控制搜索优先顺序。
 */
  int interNhops;
/* 中文逐项-变量 V1327：interPciWidth
 * interPciWidth（int；成员 ncclGpuScore）：下一GPU候选排序的机内/机间跳数与带宽评分，控制搜索优先顺序。
 */
  int interPciWidth;
/* 中文逐项-变量 V1328：interWidth
 * interWidth（int；成员 ncclGpuScore）：下一GPU候选排序的机内/机间跳数与带宽评分，控制搜索优先顺序。
 */
  int interWidth;    // Most important
};

/* 中文内部逻辑 F0469：cmpScore
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclGpuScore *s1 = (struct ncclGpuScore*)g1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclGpuScore *s2 = (struct ncclGpuScore*)g2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int d;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 条件判断：((d = (s2->interWidth - s1->interWidth)))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 函数返回：return d;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 条件判断：((d = (s2->interPciWidth - s1->interPciWidth)))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 函数返回：return d;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 条件判断：((d = (s1->interNhops - s2->interNhops)))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 函数返回：return d;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 13. 条件判断：((d = (s2->intraWidth - s1->intraWidth)))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 函数返回：return d;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 16. 条件判断：((d = (s1->intraNhops - s2->intraNhops)))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   17. 真分支：；仅条件为真执行以下子项。
 *     18. 函数返回：return d;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 19. 函数返回：return s1->startIndex - s2->startIndex;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0469
 */
/* 中文逐项-函数 F0469：cmpScore
 * 用途与位置：比较 GPU 候选的网络宽度、机内宽度与跳数等评分，改变搜索先后以更快找到好图。
 * 参数 g1（const void * g1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 g2（const void * g2）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：d = (s2->interWidth - s1->interWidth；d = (s2->interPciWidth - s1->interPciWidth；d = (s1->interNhops -
 * s2->interNhops；d = (s2->intraWidth - s1->intraWidth。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return d;；return s1->startIndex - s2->startIndex;。CHECK类宏还可能提前返回/跳转。
 */
static int cmpScore(const void * g1, const void * g2) {
/* 中文逐项-变量 V1329：s1
 * s1（struct ncclGpuScore；函数 cmpScore 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 s1 = (struct ncclGpuScore*)g1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
   struct ncclGpuScore *s1 = (struct ncclGpuScore*)g1;
/* 中文逐项-变量 V1330：s2
 * s2（struct ncclGpuScore；函数 cmpScore 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 s2 = (struct ncclGpuScore*)g2；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
   struct ncclGpuScore *s2 = (struct ncclGpuScore*)g2;
/* 中文逐项-变量 V1331：d
 * d（int；函数 cmpScore 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：if ((d = (s2->interWidth - s1->interWidth))) return d;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
   int d;
   if ((d = (s2->interWidth - s1->interWidth))) return d;
   if ((d = (s2->interPciWidth - s1->interPciWidth))) return d;
   if ((d = (s1->interNhops - s2->interNhops))) return d;
   if ((d = (s2->intraWidth - s1->intraWidth))) return d;
   if ((d = (s1->intraNhops - s2->intraNhops))) return d;
   return s1->startIndex - s2->startIndex;
}

/* 中文内部逻辑 F0470：cmpIntraScores
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int intraWidth = scores[0].intraWidth;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int intraNhops = scores[0].intraNhops;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 int i=1;；继续条件 i<count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 条件判断：(scores[i].intraWidth != intraWidth || scores[i].intraNhops != intraNhops)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 函数返回：return 1;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0470
 */
/* 中文逐项-函数 F0470：cmpIntraScores
 * 用途与位置：比较 GPU 候选的网络宽度、机内宽度与跳数等评分，改变搜索先后以更快找到好图。
 * 参数 scores（struct ncclGpuScore* scores）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 count（int count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchNextGpuSort。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<count；scores[i].intraWidth != intraWidth || scores[i].intraNhops != intraNhops。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return 1;；return 0;。CHECK类宏还可能提前返回/跳转。
 */
static int cmpIntraScores(struct ncclGpuScore* scores, int count) {
/* 中文逐项-变量 V1332：intraWidth
 * intraWidth（int；函数 cmpIntraScores 内）：下一GPU候选排序的机内/机间跳数与带宽评分，控制搜索优先顺序。
 * 初值/来源 intraWidth = scores[0].intraWidth；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int intraWidth = scores[0].intraWidth;
/* 中文逐项-变量 V1333：intraNhops
 * intraNhops（int；函数 cmpIntraScores 内）：下一GPU候选排序的机内/机间跳数与带宽评分，控制搜索优先顺序。
 * 初值/来源 intraNhops = scores[0].intraNhops；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int intraNhops = scores[0].intraNhops;
  for (/* 中文逐项-变量 V1334：i
 * i（int；函数 cmpIntraScores 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<count；循环体与增量表达式决定每轮如何推进。
 */int i=1; i<count; i++) {
    if (scores[i].intraWidth != intraWidth || scores[i].intraNhops != intraNhops) return 1;
  }
  return 0;
}

/* 中文内部逻辑 F0471：getGpuIndex
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 条件判断：(system->nodes[GPU].nodes[g].gpu.rank == rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     3. 真分支：；仅条件为真执行以下子项。
 *       4. 调用 / 计算 / 写入：*index = g;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 调用 / 计算 / 写入：WARN("Could not find gpu rank %d", rank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0471
 */
/* 中文逐项-函数 F0471：getGpuIndex
 * 用途与位置：把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 index（int* index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRecGpu。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：g<system->nodes[GPU].count；system->nodes[GPU].nodes[g].gpu.rank == rank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getGpuIndex(struct ncclTopoSystem* system, int rank, int* index) {
  for (/* 中文逐项-变量 V1335：g
 * g（int；函数 getGpuIndex 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
    if (system->nodes[GPU].nodes[g].gpu.rank == rank) {
      *index = g;
      return ncclSuccess;
    }
  }
  WARN("Could not find gpu rank %d", rank);
  return ncclInternalError;
}

/* 中文内部逻辑 F0472：getNetIndex
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int n=0;；继续条件 n<system->nodes[NET].count；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 条件判断：(system->nodes[NET].nodes[n].id == id)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     3. 真分支：；仅条件为真执行以下子项。
 *       4. 调用 / 计算 / 写入：*index = n;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 调用 / 计算 / 写入：WARN("Could not find net id %lx", id);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0472
 */
/* 中文逐项-函数 F0472：getNetIndex
 * 用途与位置：把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 id（int64_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 index（int* index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::getNetPaths；src/graph/search.cc::ncclTopoSearchRecGpu。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：n<system->nodes[NET].count；system->nodes[NET].nodes[n].id == id。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getNetIndex(struct ncclTopoSystem* system, int64_t id, int* index) {
  for (/* 中文逐项-变量 V1336：n
 * n（int；函数 getNetIndex 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[NET].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[NET].count; n++) {
    if (system->nodes[NET].nodes[n].id == id) {
      *index = n;
      return ncclSuccess;
    }
  }
  WARN("Could not find net id %lx", id);
  return ncclInternalError;
}

/* 中文内部逻辑 F0473：getNetPaths
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int netId = graph->inter[graph->nChannels*2];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(getNetIndex(system, netId, &n));；失败时可能提前return，成功才继续；把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。
 * 4. 调用 / 计算 / 写入：*netPaths=system->nodes[NET].nodes[n].paths[GPU];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0473
 */
/* 中文逐项-函数 F0473：getNetPaths
 * 用途与位置：按当前跨节点路径级别获取 NET 路径集合，供候选 GPU/NIC 组合筛选。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 netPaths（struct ncclTopoLinkList** netPaths）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchNextGpuSort。
 * 本体直接出现的调用/包装：NCCLCHECK；getNetIndex。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getNetPaths(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, struct ncclTopoLinkList** netPaths) {
/* 中文逐项-变量 V1337：netId
 * netId（int；函数 getNetPaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 netId = graph->inter[graph->nChannels*2]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int netId = graph->inter[graph->nChannels*2];
/* 中文逐项-变量 V1338：n
 * n（int；函数 getNetPaths 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getNetIndex(system, netId, &n));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int n;
  NCCLCHECK(getNetIndex(system, netId, &n));
  *netPaths=system->nodes[NET].nodes[n].paths[GPU];
  return ncclSuccess;
}

/* 中文内部逻辑 F0474：ncclTopoSearchNextGpuSort
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const uint64_t flag = 1ULL<<(graph->nChannels);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct ncclTopoLinkList* paths = gpu->paths[GPU];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：struct ncclTopoLinkList* netPaths = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(sortNet)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(getNetPaths(system, graph, &netPaths));；失败时可能提前return，成功才继续；按当前跨节点路径级别获取 NET 路径集合，供候选 GPU/NIC 组合筛选。
 * 8. 准备局部状态：struct ncclGpuScore scores[NCCL_TOPO_MAX_NODES];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 调用 / 计算 / 写入：memset(scores, 0, ngpus*sizeof(struct ncclGpuScore));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 准备局部状态：int start = gpu-system->nodes[GPU].nodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 准备局部状态：int count = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 循环：初始 int i=1;；继续条件 i<ngpus；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   13. 准备局部状态：int g = (start+i)%ngpus;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   14. 条件判断：(paths[g].count == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     15. 真分支：；仅条件为真执行以下子项。
 *       16. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   17. 条件判断：(system->nodes[GPU].nodes[g].used & flag)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     18. 真分支：；仅条件为真执行以下子项。
 *       19. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   20. 调用 / 计算 / 写入：scores[count].g = g;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   21. 调用 / 计算 / 写入：scores[count].startIndex = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   22. 调用 / 计算 / 写入：scores[count].intraNhops = paths[g].count;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   23. 调用 / 计算 / 写入：scores[count].intraWidth = paths[g].width;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   24. 条件判断：(netPaths)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     25. 真分支：；仅条件为真执行以下子项。
 *       26. 调用 / 计算 / 写入：scores[count].interNhops = netPaths[g].count;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       27. 调用 / 计算 / 写入：scores[count].interPciWidth = gpuPciWidth(system->nodes[GPU].nodes+g);；统计 GPU 的 PCI 通路宽度，为下一 GPU 候选排序加入 PCI 容量因素。
 *       28. 调用 / 计算 / 写入：scores[count].interWidth = netPaths[g].width;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   29. 调用 / 计算 / 写入：count++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 31. 调用 / 计算 / 写入：qsort(scores, count, sizeof(struct ncclGpuScore), cmpScore);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 32. 条件判断：(sortNet == -1 && cmpIntraScores(scores, count) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   33. 真分支：；仅条件为真执行以下子项。
 *     34. 循环：初始 int i=0;；继续条件 i<count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       35. 调用 / 计算 / 写入：next[i] = scores[count-1-i].g;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     36. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   37. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     38. 循环：初始 int i=0;；继续条件 i<count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       39. 调用 / 计算 / 写入：next[i] = scores[i].g;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     40. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 41. 调用 / 计算 / 写入：*countPtr = count;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 42. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0474
 */
/* 中文逐项-函数 F0474：ncclTopoSearchNextGpuSort
 * 用途与位置：给尚未使用的下一 GPU 候选打分排序，优先搜索带宽高、路径代价合适的邻居。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 gpu（struct ncclTopoNode* gpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 参数 next（int* next）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 参数 countPtr（int* countPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 sortNet（int sortNet）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRecGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；cmpIntraScores；getNetPaths；gpuPciWidth；memset；qsort。
 * 需要对照的边界/推进条件：sortNet；i<ngpus；paths[g].count == 0；system->nodes[GPU].nodes[g].used & flag。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSearchNextGpuSort(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, struct ncclTopoNode* gpu, int* next, int* countPtr, int sortNet) {
/* 中文逐项-变量 V1339：flag
 * flag（uint64_t；函数 ncclTopoSearchNextGpuSort 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = 1ULL<<(graph->nChannels)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  const uint64_t flag = 1ULL<<(graph->nChannels);
/* 中文逐项-变量 V1340：ngpus
 * ngpus（int；函数 ncclTopoSearchNextGpuSort 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;
/* 中文逐项-变量 V1341：paths
 * paths（struct ncclTopoLinkList；函数 ncclTopoSearchNextGpuSort 内）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 * 初值/来源 paths = gpu->paths[GPU]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoLinkList* paths = gpu->paths[GPU];
/* 中文逐项-变量 V1342：netPaths
 * netPaths（struct ncclTopoLinkList；函数 ncclTopoSearchNextGpuSort 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 netPaths = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclTopoLinkList* netPaths = NULL;
  if (sortNet) NCCLCHECK(getNetPaths(system, graph, &netPaths));

/* 中文逐项-变量 V1343：scores
 * scores（struct ncclGpuScore；函数 ncclTopoSearchNextGpuSort 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(scores, 0, ngpus*sizeof(struct ncclGpuScore));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclGpuScore scores[NCCL_TOPO_MAX_NODES];
  memset(scores, 0, ngpus*sizeof(struct ncclGpuScore));
/* 中文逐项-变量 V1344：start
 * start（int；函数 ncclTopoSearchNextGpuSort 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 start = gpu-system->nodes[GPU].nodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int start = gpu-system->nodes[GPU].nodes;
/* 中文逐项-变量 V1345：count
 * count（int；函数 ncclTopoSearchNextGpuSort 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 初值/来源 count = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int count = 0;
  for (/* 中文逐项-变量 V1346：i
 * i（int；函数 ncclTopoSearchNextGpuSort 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<ngpus；循环体与增量表达式决定每轮如何推进。
 */int i=1; i<ngpus; i++) {
/* 中文逐项-变量 V1347：g
 * g（int；函数 ncclTopoSearchNextGpuSort 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = (start+i)%ngpus；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int g = (start+i)%ngpus;
    if (paths[g].count == 0) continue; // There is no path to that GPU
    if (system->nodes[GPU].nodes[g].used & flag) continue;
    scores[count].g = g;
    scores[count].startIndex = i;
    scores[count].intraNhops = paths[g].count;
    scores[count].intraWidth = paths[g].width;
    if (netPaths) {
      scores[count].interNhops = netPaths[g].count;
      scores[count].interPciWidth = gpuPciWidth(system->nodes[GPU].nodes+g);
      scores[count].interWidth = netPaths[g].width;
    }
    count++;
  }

  // Sort GPUs
  qsort(scores, count, sizeof(struct ncclGpuScore), cmpScore);

  // Check if all have the same intra-node score in which case we go reverse for sortNet = -1
  if (sortNet == -1 && cmpIntraScores(scores, count) == 0) {
    for (/* 中文逐项-变量 V1348：i
 * i（int；函数 ncclTopoSearchNextGpuSort 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<count; i++) next[i] = scores[count-1-i].g;
  } else {
    for (/* 中文逐项-变量 V1349：i
 * i（int；函数 ncclTopoSearchNextGpuSort 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<count; i++) next[i] = scores[i].g;
  }
  *countPtr = count;
  return ncclSuccess;
}

/* 中文逐项-接口 X0029：ncclTopoSearchRec
 * 接口职责：为一次候选搜索初始化已用节点状态与剩余预算，启动网络或机内递归搜索。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoSearchRec(struct ncclTopoSystem* system, struct ncclTopoGraph*
 * graph, struct ncclTopoGraph* saveGraph, int* time);
 */
ncclResult_t ncclTopoSearchRec(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, struct ncclTopoGraph* saveGraph, int* time);

// Try to keep all searchs within one second
/* 中文逐项-常量 X0030：NCCL_SEARCH_GLOBAL_TIMEOUT
 * 编译期定义：NCCL_SEARCH_GLOBAL_TIMEOUT = (1ULL<<18)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SEARCH_GLOBAL_TIMEOUT (1ULL<<18)
/* 中文逐项-常量 X0031：NCCL_SEARCH_TIMEOUT
 * 编译期定义：NCCL_SEARCH_TIMEOUT = (1<<14)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SEARCH_TIMEOUT (1<<14)
/* 中文逐项-常量 X0032：NCCL_SEARCH_TIMEOUT_TREE
 * 编译期定义：NCCL_SEARCH_TIMEOUT_TREE = (1<<14)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SEARCH_TIMEOUT_TREE (1<<14)
/* 中文逐项-常量 X0033：NCCL_SEARCH_TIMEOUT_SAMECHANNELS
 * 编译期定义：NCCL_SEARCH_TIMEOUT_SAMECHANNELS = (1<<8)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SEARCH_TIMEOUT_SAMECHANNELS (1<<8)

/* 中文逐项-常量 X0034：FORCED_ORDER_PCI
 * 编译期定义：FORCED_ORDER_PCI = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define FORCED_ORDER_PCI 1
/* 中文逐项-常量 X0035：FORCED_ORDER_REPLAY
 * 编译期定义：FORCED_ORDER_REPLAY = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define FORCED_ORDER_REPLAY 2

/* 中文内部逻辑 F0475：ncclTopoReplayGetGpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*g = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 条件判断：(graph->nChannels == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：int nextRank = graph->intra[(graph->nChannels-1)*ngpus+step+1];；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 7. 循环：初始 int i=0;；继续条件 i<ngpus；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   8. 条件判断：(system->nodes[GPU].nodes[i].gpu.rank == nextRank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     9. 真分支：；仅条件为真执行以下子项。
 *       10. 调用 / 计算 / 写入：*g = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 条件判断：(*g == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0475
 */
/* 中文逐项-函数 F0475：ncclTopoReplayGetGpu
 * 用途与位置：重放已有 channel 的 GPU 顺序，允许 sameChannels 搜索复用前面找到的排列。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 step（int step）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 参数 g（int* g）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRecGpu；src/graph/search.cc::ncclTopoSearchRecNet；src/graph/search.cc::ncclTopoSearchRec。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：graph->nChannels == 0；i<ngpus；system->nodes[GPU].nodes[i].gpu.rank == nextRank；*g == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoReplayGetGpu(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, int step, int* g) {
  *g = -1;
  if (graph->nChannels == 0) return ncclInternalError;
/* 中文逐项-变量 V1350：ngpus
 * ngpus（int；函数 ncclTopoReplayGetGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;
/* 中文逐项-变量 V1351：nextRank
 * nextRank（int；函数 ncclTopoReplayGetGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nextRank = graph->intra[(graph->nChannels-1)*ngpus+step+1]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nextRank = graph->intra[(graph->nChannels-1)*ngpus+step+1];
  for (/* 中文逐项-变量 V1352：i
 * i（int；函数 ncclTopoReplayGetGpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ngpus；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ngpus; i++) if (system->nodes[GPU].nodes[i].gpu.rank == nextRank) {
    *g = i;
    return ncclSuccess;
  }
  if (*g == -1) return ncclInternalError;
  return ncclSuccess;
}

/* 中文逐项-接口 X0036：ncclTopoSearchRecGpu
 * 接口职责：递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoSearchRecGpu(struct ncclTopoSystem* system, struct
 * ncclTopoGraph* graph, struct ncclTopoGraph* saveGraph, struct ncclTopoNode* gpu, int step, int backToNet, int
 * backToFirstRank, int forcedOrder, int …（完整表达式见紧邻源码）
 */
ncclResult_t ncclTopoSearchRecGpu(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, struct ncclTopoGraph* saveGraph, struct ncclTopoNode* gpu, int step, int backToNet, int backToFirstRank, int forcedOrder, int *time);

/* 中文内部逻辑 F0476：ncclTopoSearchTryGpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const uint64_t flag = 1ULL<<(graph->nChannels);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclTopoNode* gpu;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoFollowPath(system, graph, type, index, GPU, g, 1, &gpu));；失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。
 * 4. 条件判断：(gpu)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：gpu->used ^= flag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, step, backToNet, backToFirstRank, forcedOrder, time));；失败时可能提前return，成功才继续；递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。
 *     8. 调用 / 计算 / 写入：gpu->used ^= flag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoFollowPath(system, graph, type, index, GPU, g, -1, &gpu));；失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0476
 */
/* 中文逐项-函数 F0476：ncclTopoSearchTryGpu
 * 用途与位置：尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 saveGraph（struct ncclTopoGraph* saveGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 step（int step）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 参数 backToNet（int backToNet）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 backToFirstRank（int backToFirstRank）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 forcedOrder（int forcedOrder）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 time（int *time）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 index（int index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 参数 g（int g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRecGpu；src/graph/search.cc::ncclTopoSearchRecNet；src/graph/search.cc::ncclTopoSearchRec。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoFollowPath；ncclTopoSearchRecGpu。
 * 需要对照的边界/推进条件：gpu。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSearchTryGpu(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, struct ncclTopoGraph* saveGraph, int step, int backToNet, int backToFirstRank, int forcedOrder, int *time, int type, int index, int g) {
/* 中文逐项-变量 V1353：flag
 * flag（uint64_t；函数 ncclTopoSearchTryGpu 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = 1ULL<<(graph->nChannels)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  const uint64_t flag = 1ULL<<(graph->nChannels);
/* 中文逐项-变量 V1354：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoSearchTryGpu 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoFollowPath(system, graph, type, index, GPU, g, 1,
 * &gpu));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoNode* gpu;
  NCCLCHECK(ncclTopoFollowPath(system, graph, type, index, GPU, g, 1, &gpu));
  if (gpu) {
    gpu->used ^= flag;
    NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, step, backToNet, backToFirstRank, forcedOrder, time));
    gpu->used ^= flag;
    NCCLCHECK(ncclTopoFollowPath(system, graph, type, index, GPU, g, -1, &gpu));
  }
  return ncclSuccess;
}

/* 中文精读：搜索目标是有效带宽与路径代价的折中
 * 候选包含通道数、每通道机内/机间速度、路径类型和跳数；这里比较候选是否优于已保存结果。
 * 不能只追求 nChannels 更大：通道过多但每条速度下降，可能没有收益。
 * 外层 Compute 还受搜索预算、sameChannels/crossNic/路径级别的限制，因此它是受限启发式搜索，
 * 不是对所有可能拓扑穷举后保证数学意义上的全局最优。
 */
/* 中文内部逻辑 F0477：ncclTopoCompareGraphs
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(graph->nChannels < graph->minChannels)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 条件判断：(graph->nChannels*graph->speedIntra < refGraph->nChannels*refGraph->speedIntra)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 条件判断：(graph->nChannels*graph->speedIntra > refGraph->nChannels*refGraph->speedIntra)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：*copy = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 11. 条件判断：(graph->pattern == refGraph->pattern && graph->crossNic == refGraph->crossNic && graph->nHops < refGraph->nHops)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 调用 / 计算 / 写入：*copy = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0477
 */
/* 中文逐项-函数 F0477：ncclTopoCompareGraphs
 * 用途与位置：按通道总带宽、机内速度与跳数等指标比较候选图与已保存图，决定是否替换。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 refGraph（struct ncclTopoGraph* refGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 copy（int* copy）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRecGpu。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：graph->nChannels < graph->minChannels；graph->nChannels*graph->speedIntra <
 * refGraph->nChannels*refGraph->speedIntra；graph->nChannels*graph->speedIntra >
 * refGraph->nChannels*refGraph->speedIntra；graph->pattern == refGraph->pattern && graph->crossNic ==
 * refGraph->crossNic && graph->nHops < refGraph->nHops。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoCompareGraphs(struct ncclTopoGraph* graph, struct ncclTopoGraph* refGraph, int* copy) {
  // 1. Constraint to get the same nChannels between Rings and Trees
  if (graph->nChannels < graph->minChannels) return ncclSuccess;

  // 2. Try to get better bandwidth
  if (graph->nChannels*graph->speedIntra < refGraph->nChannels*refGraph->speedIntra) return ncclSuccess;
  if (graph->nChannels*graph->speedIntra > refGraph->nChannels*refGraph->speedIntra) {
    *copy = 1;
    return ncclSuccess;
  }
  // 3. Less hops (but not at the price of going cross NICs)
  if (graph->pattern == refGraph->pattern && graph->crossNic == refGraph->crossNic && graph->nHops < refGraph->nHops) *copy = 1;
  return ncclSuccess;
}

// Build a list of the best NETs to try.
//
// "gpu" can be set to -1 to build a list suitable for all GPUs (search start) or to a given gpu
//  index when trying to get back to the NIC.
//
// The list is built the following way:
// 1. Select NETs starting with those close to GPU(s), based on paths[n].type.
// 2. For each GPU, once that list of NICs with a given distance is prepared, shuffle the list
//    based on the GPU NVML index so that e.g. GPU 1 chooses NIC 1 first instead of NIC 0 which
//    might have been choosen by GPU 0 (case with multiple independent communicators per node)
// 3. Then add the NETs to the final list if they were not already added by another closer GPU.

/* 中文内部逻辑 F0478：ncclTopoSelectNets
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int netCount = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int localNetCount;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int* localNets;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&localNets, system->nodes[NET].count));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 循环：初始 int t=0;；继续条件 t <= typeInter；每轮更新 t++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     7. 条件判断：(gpu != -1 && gpu != g)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       8. 真分支：；仅条件为真执行以下子项。
 *         9. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *     10. 调用 / 计算 / 写入：localNetCount = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 准备局部状态：struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     12. 准备局部状态：struct ncclTopoLinkList* paths = gpu->paths[NET];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     13. 循环：初始 int n=0;；继续条件 n<system->nodes[NET].count；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       14. 条件判断：(paths[n].type == t)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         15. 真分支：；仅条件为真执行以下子项。
 *           16. 调用 / 计算 / 写入：localNets[localNetCount++] = n;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     17. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     18. 条件判断：(localNetCount == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       19. 真分支：；仅条件为真执行以下子项。
 *         20. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *     21. 循环：初始 int r=0;；继续条件 r<system->nodes[GPU].nodes[g].gpu.dev % localNetCount；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       22. 准备局部状态：int net0 = localNets[0];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       23. 循环：初始 int i=0;；继续条件 i<localNetCount-1；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         24. 调用 / 计算 / 写入：localNets[i] = localNets[i+1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       25. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       26. 调用 / 计算 / 写入：localNets[localNetCount-1] = net0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     27. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     28. 循环：初始 int i=0;；继续条件 i<localNetCount；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       29. 准备局部状态：int n = localNets[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       30. 准备局部状态：int found = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       31. 循环：初始 ；继续条件 (nets[found] != n && found<netCount)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         32. 调用 / 计算 / 写入：found++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       33. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       34. 条件判断：(found == netCount)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         35. 真分支：；仅条件为真执行以下子项。
 *           36. 调用 / 计算 / 写入：nets[netCount++] = n;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     37. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   38. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 39. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 40. 调用 / 计算 / 写入：*netCountRet = netCount;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 41. 调用 / 计算 / 写入：free(localNets);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 42. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0478
 */
/* 中文逐项-函数 F0478：ncclTopoSelectNets
 * 用途与位置：按路径级别、GPU 亲和性和网络设备分组挑出候选 NIC，使机间端点选择与机内路径匹配。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 typeInter（int typeInter）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 * 参数 gpu（int gpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 参数 nets（int* nets）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 netCountRet（int* netCountRet）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRecGpu；src/graph/search.cc::ncclTopoSearchRecNet。
 * 本体直接出现的调用/包装：NCCLCHECK；free；ncclCalloc。
 * 需要对照的边界/推进条件：t <= typeInter；g<system->nodes[GPU].count；gpu != -1 && gpu !=
 * g；n<system->nodes[NET].count。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSelectNets(struct ncclTopoSystem* system, int typeInter, int gpu, int* nets, int* netCountRet) {
/* 中文逐项-变量 V1355：netCount
 * netCount（int；函数 ncclTopoSelectNets 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 netCount = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int netCount = 0;
/* 中文逐项-变量 V1356：localNetCount
 * localNetCount（int；函数 ncclTopoSelectNets 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 此处未显式给初值；后续首个文本使用点：localNetCount = 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int localNetCount;
/* 中文逐项-变量 V1357：localNets
 * localNets（int；函数 ncclTopoSelectNets 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&localNets, system->nodes[NET].count));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int* localNets;
  NCCLCHECK(ncclCalloc(&localNets, system->nodes[NET].count));

  for (/* 中文逐项-变量 V1358：t
 * t（int；函数 ncclTopoSelectNets 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：t <= typeInter；循环体与增量表达式决定每轮如何推进。
 */int t=0; t <= typeInter; t++) {
    for (/* 中文逐项-变量 V1359：g
 * g（int；函数 ncclTopoSelectNets 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
      if (gpu != -1 && gpu != g) continue;
      localNetCount = 0;
/* 中文逐项-变量 V1360：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoSelectNets 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = system->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
/* 中文逐项-变量 V1361：paths
 * paths（struct ncclTopoLinkList；函数 ncclTopoSelectNets 内）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 * 初值/来源 paths = gpu->paths[NET]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoLinkList* paths = gpu->paths[NET];
      for (/* 中文逐项-变量 V1362：n
 * n（int；函数 ncclTopoSelectNets 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[NET].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[NET].count; n++) {
        if (paths[n].type == t) localNets[localNetCount++] = n;
      }
      if (localNetCount == 0) continue;
      // Shuffle by gpu NVML device number so that GPUs on the same PCI switch
      // with multiple NICs don't use the same one as first choice.
      for (/* 中文逐项-变量 V1363：r
 * r（int；函数 ncclTopoSelectNets 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<system->nodes[GPU].nodes[g].gpu.dev % localNetCount；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<system->nodes[GPU].nodes[g].gpu.dev % localNetCount; r++) {
/* 中文逐项-变量 V1364：net0
 * net0（int；函数 ncclTopoSelectNets 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 net0 = localNets[0]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        int net0 = localNets[0];
        for (/* 中文逐项-变量 V1365：i
 * i（int；函数 ncclTopoSelectNets 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<localNetCount-1；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<localNetCount-1; i++) localNets[i] = localNets[i+1];
        localNets[localNetCount-1] = net0;
      }
      // Append NICs to list
      for (/* 中文逐项-变量 V1366：i
 * i（int；函数 ncclTopoSelectNets 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<localNetCount；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<localNetCount; i++) {
/* 中文逐项-变量 V1367：n
 * n（int；函数 ncclTopoSelectNets 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = localNets[i]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        int n = localNets[i];
/* 中文逐项-变量 V1368：found
 * found（int；函数 ncclTopoSelectNets 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 found = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
        int found = 0;
        while (nets[found] != n && found<netCount) found++;
        if (found == netCount) nets[netCount++] = n;
      }
    }
  }

  *netCountRet = netCount;
  free(localNets);

  return ncclSuccess;
}

/* 中文内部逻辑 F0479：ncclTopoSearchRecGpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：((*time) <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 调用 / 计算 / 写入：(*time)--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 条件判断：(step == ngpus)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 准备局部状态：int copy = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 调用 / 计算 / 写入：graph->nChannels++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCompareGraphs(graph, saveGraph, &copy));；失败时可能提前return，成功才继续；按通道总带宽、机内速度与跳数等指标比较候选图与已保存图，决定是否替换。
 *     11. 条件判断：(copy)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       12. 真分支：；仅条件为真执行以下子项。
 *         13. 调用 / 计算 / 写入：memcpy(saveGraph, graph, sizeof(struct ncclTopoGraph));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         14. 条件判断：(graph->nChannels == graph->maxChannels)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           15. 真分支：；仅条件为真执行以下子项。
 *             16. 调用 / 计算 / 写入：*time = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     17. 条件判断：(graph->nChannels < graph->maxChannels)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       18. 真分支：；仅条件为真执行以下子项。
 *         19. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchRec(system, graph, saveGraph, time));；失败时可能提前return，成功才继续；为一次候选搜索初始化已用节点状态与剩余预算，启动网络或机内递归搜索。
 *     20. 调用 / 计算 / 写入：graph->nChannels--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     21. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 22. 调用 / 计算 / 写入：graph->intra[graph->nChannels*ngpus+step] = gpu->gpu.rank;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 23. 准备局部状态：int g = gpu - system->nodes[GPU].nodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 24. 条件判断：(step == backToNet)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   25. 真分支：；仅条件为真执行以下子项。
 *     26. 条件判断：(system->nodes[NET].count)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       27. 真分支：；仅条件为真执行以下子项。
 *         28. 准备局部状态：int startNetIndex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         29. 调用 / 计算 / 写入：NCCLCHECK(getNetIndex(system, graph->inter[graph->nChannels*2], &startNetIndex));；失败时可能提前return，成功才继续；把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。
 *         30. 准备局部状态：struct ncclTopoNode* startNet = system->nodes[NET].nodes+startNetIndex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         31. 准备局部状态：int netcount;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         32. 准备局部状态：int* nets;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         33. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         34. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSelectNets(system, graph->typeInter, g, nets, &netcount));；失败时可能提前return，成功才继续；按路径级别、GPU 亲和性和网络设备分组挑出候选 NIC，使机间端点选择与机内路径匹配。
 *         35. 循环：初始 int i=0;；继续条件 i<netcount；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           36. 准备局部状态：int n = nets[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           37. 准备局部状态：struct ncclTopoNode* net = system->nodes[NET].nodes+n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           38. 条件判断：(graph->pattern == NCCL_TOPO_PATTERN_TREE && net->id != startNet->id)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             39. 真分支：；仅条件为真执行以下子项。
 *               40. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *           41. 条件判断：(graph->crossNic != 1 && (net->net.asic != startNet->net.asic || net->net.port != startNet->net.port))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             42. 真分支：；仅条件为真执行以下子项。
 *               43. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *           44. 准备局部状态：int nextBackToNet = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           45. 准备局部状态：float speedInterSave = graph->speedInter;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           46. 条件判断：(graph->pattern == NCCL_TOPO_PATTERN_BALANCED_TREE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             47. 真分支：；仅条件为真执行以下子项。
 *               48. 条件判断：(step == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 49. 真分支：；仅条件为真执行以下子项。
 *                   50. 调用 / 计算 / 写入：nextBackToNet = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 51. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   52. 条件判断：(net->id != graph->inter[graph->nChannels*2+1])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     53. 真分支：；仅条件为真执行以下子项。
 *                       54. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *               55. 调用 / 计算 / 写入：graph->speedInter /= 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           56. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, NET, n, 1, &net));；失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。
 *           57. 调用 / 计算 / 写入：graph->speedInter = speedInterSave;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           58. 条件判断：(net)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             59. 真分支：；仅条件为真执行以下子项。
 *               60. 调用 / 计算 / 写入：graph->inter[graph->nChannels*2+1] = net->id;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               61. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, step, nextBackToNet, backToFirstRank, forcedOrder, time));；失败时可能提前return，成功才继续；递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。
 *               62. 条件判断：(graph->pattern == NCCL_TOPO_PATTERN_BALANCED_TREE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 63. 真分支：；仅条件为真执行以下子项。
 *                   64. 调用 / 计算 / 写入：graph->speedInter /= 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               65. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, NET, n, -1, &net));；失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。
 *               66. 调用 / 计算 / 写入：graph->speedInter = speedInterSave;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         67. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         68. 调用 / 计算 / 写入：free(nets);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   69. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     70. 条件判断：(step < system->nodes[GPU].count-1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       71. 真分支：；仅条件为真执行以下子项。
 *         72. 准备局部状态：int next[NCCL_TOPO_MAX_NODES];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         73. 准备局部状态：int count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         74. 条件判断：(forcedOrder == FORCED_ORDER_PCI)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           75. 真分支：；仅条件为真执行以下子项。
 *             76. 调用 / 计算 / 写入：next[0] = step+1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *             77. 调用 / 计算 / 写入：count = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           78. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             79. 条件判断：(forcedOrder == FORCED_ORDER_REPLAY)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               80. 真分支：；仅条件为真执行以下子项。
 *                 81. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoReplayGetGpu(system, graph, step, next));；失败时可能提前return，成功才继续；重放已有 channel 的 GPU 顺序，允许 sameChannels 搜索复用前面找到的排列。
 *                 82. 调用 / 计算 / 写入：count = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               83. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                 84. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchNextGpuSort(system, graph, gpu, next, &count, backToNet == -1 ? 0 : backToNet == step+1 ? 1 : -1 ));；失败时可能提前return，成功才继续；给尚未使用的下一 GPU 候选打分排序，优先搜索带宽高、路径代价合适的邻居。
 *         85. 循环：初始 int i=0;；继续条件 i<count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           86. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, step+1, backToNet, backToFirstRank, forcedOrder, time, GPU, g, next[i]));；失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。
 *         87. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       88. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         89. 条件判断：(step == backToFirstRank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           90. 真分支：；仅条件为真执行以下子项。
 *             91. 准备局部状态：int p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             92. 调用 / 计算 / 写入：NCCLCHECK(getGpuIndex(system, graph->intra[graph->nChannels*ngpus], &p));；失败时可能提前return，成功才继续；把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。
 *             93. 准备局部状态：struct ncclTopoNode* firstGpu;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             94. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, GPU, p, 1, &firstGpu));；失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。
 *             95. 条件判断：(firstGpu)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               96. 真分支：；仅条件为真执行以下子项。
 *                 97. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, firstGpu, step+1, backToNet, -1, forcedOrder, time));；失败时可能提前return，成功才继续；递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。
 *                 98. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, GPU, p, -1, &firstGpu));；失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。
 *           99. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             100. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, ngpus, -1, -1, forcedOrder, time));；失败时可能提前return，成功才继续；递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。
 * 101. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0479
 */
/* 中文逐项-函数 F0479：ncclTopoSearchRecGpu
 * 用途与位置：递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 saveGraph（struct ncclTopoGraph* saveGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 gpu（struct ncclTopoNode* gpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 参数 step（int step）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 参数 backToNet（int backToNet）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 backToFirstRank（int backToFirstRank）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 forcedOrder（int forcedOrder）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 time（int *time）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchTryGpu；src/graph/search.cc::ncclTopoSearchRecGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；free；getGpuIndex；getNetIndex；memcpy；ncclCalloc；ncclTopoCompareGraphs；ncclTopoFollowPath；ncclTopoReplayGetGpu；ncclTopoSearchNextGpuSort；ncclTopoSearchRec；ncclTopoSearchRecGpu；ncclTopoSearchTryGpu；ncclTopoSelectNets。
 * 需要对照的边界/推进条件：*time) <= 0；step == ngpus；copy；graph->nChannels == graph->maxChannels。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSearchRecGpu(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, struct ncclTopoGraph* saveGraph, struct ncclTopoNode* gpu, int step, int backToNet, int backToFirstRank, int forcedOrder, int *time) {
  if ((*time) <= 0) return ncclSuccess;
  (*time)--;

/* 中文逐项-变量 V1369：ngpus
 * ngpus（int；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;
  if (step == ngpus) {
    // Determine whether we found a better solution or not
/* 中文逐项-变量 V1370：copy
 * copy（int；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 copy = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int copy = 0;
    graph->nChannels++;
    NCCLCHECK(ncclTopoCompareGraphs(graph, saveGraph, &copy));
    if (copy) {
      memcpy(saveGraph, graph, sizeof(struct ncclTopoGraph));
      if (graph->nChannels == graph->maxChannels) *time = -1;
    }
    if (graph->nChannels < graph->maxChannels) {
      NCCLCHECK(ncclTopoSearchRec(system, graph, saveGraph, time));
    }
    graph->nChannels--;
    return ncclSuccess;
  }
  graph->intra[graph->nChannels*ngpus+step] = gpu->gpu.rank;
/* 中文逐项-变量 V1371：g
 * g（int；函数 ncclTopoSearchRecGpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = gpu - system->nodes[GPU].nodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int g = gpu - system->nodes[GPU].nodes;
  if (step == backToNet) {
    // first get back to NIC
    if (system->nodes[NET].count) {
/* 中文逐项-变量 V1372：startNetIndex
 * startNetIndex（int；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getNetIndex(system, graph->inter[graph->nChannels*2],
 * &startNetIndex));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int startNetIndex;
      NCCLCHECK(getNetIndex(system, graph->inter[graph->nChannels*2], &startNetIndex));
/* 中文逐项-变量 V1373：startNet
 * startNet（struct ncclTopoNode；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 startNet = system->nodes[NET].nodes+startNetIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoNode* startNet = system->nodes[NET].nodes+startNetIndex;
/* 中文逐项-变量 V1374：netcount
 * netcount（int；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoSelectNets(system, graph->typeInter, g, nets,
 * &netcount));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int netcount;
/* 中文逐项-变量 V1375：nets
 * nets（int；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int* nets;
      NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));
      NCCLCHECK(ncclTopoSelectNets(system, graph->typeInter, g, nets, &netcount));
      for (/* 中文逐项-变量 V1376：i
 * i（int；函数 ncclTopoSearchRecGpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<netcount；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<netcount; i++) {
/* 中文逐项-变量 V1377：n
 * n（int；函数 ncclTopoSearchRecGpu 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = nets[i]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        int n = nets[i];
/* 中文逐项-变量 V1378：net
 * net（struct ncclTopoNode；函数 ncclTopoSearchRecGpu 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 net = system->nodes[NET].nodes+n；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclTopoNode* net = system->nodes[NET].nodes+n;
        if (graph->pattern == NCCL_TOPO_PATTERN_TREE && net->id != startNet->id) continue; // Trees are symmetric
        if (graph->crossNic != 1 && (net->net.asic != startNet->net.asic || net->net.port != startNet->net.port)) continue;

        // Balanced Tree : count half of the bandwidth on first two GPUs
/* 中文逐项-变量 V1379：nextBackToNet
 * nextBackToNet（int；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nextBackToNet = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
        int nextBackToNet = -1;
/* 中文逐项-变量 V1380：speedInterSave
 * speedInterSave（float；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 speedInterSave = graph->speedInter；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        float speedInterSave = graph->speedInter;
        if (graph->pattern == NCCL_TOPO_PATTERN_BALANCED_TREE) {
          // Count half of the bandwidth on each of the first two GPUs
          if (step == 0) nextBackToNet = 1;
          else if (net->id != graph->inter[graph->nChannels*2+1]) continue;
          graph->speedInter /= 2;
        }

        NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, NET, n, 1, &net));
        graph->speedInter = speedInterSave;
        if (net) {
          graph->inter[graph->nChannels*2+1] = net->id;
          NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, step, nextBackToNet, backToFirstRank, forcedOrder, time));

          if (graph->pattern == NCCL_TOPO_PATTERN_BALANCED_TREE) graph->speedInter /= 2;
          NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, NET, n, -1, &net));
          graph->speedInter = speedInterSave;
        }
      }
      free(nets);
    }
  } else if (step < system->nodes[GPU].count-1) {
    // Go to next GPU
/* 中文逐项-变量 V1381：next
 * next（int；函数 ncclTopoSearchRecGpu 内）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 此处未显式给初值；后续首个文本使用点：next[0] = step+1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int next[NCCL_TOPO_MAX_NODES];
/* 中文逐项-变量 V1382：count
 * count（int；函数 ncclTopoSearchRecGpu 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 此处未显式给初值；后续首个文本使用点：count = 1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int count;
    if (forcedOrder == FORCED_ORDER_PCI) { // Try the PCI order
      next[0] = step+1;
      count = 1;
    } else if (forcedOrder == FORCED_ORDER_REPLAY) { // Try last channel order
      NCCLCHECK(ncclTopoReplayGetGpu(system, graph, step, next));
      count = 1;
    } else { // Normal search
      NCCLCHECK(ncclTopoSearchNextGpuSort(system, graph, gpu, next, &count, backToNet == -1 ? 0 : backToNet == step+1 ? 1 : -1 ));
    }
    for (/* 中文逐项-变量 V1383：i
 * i（int；函数 ncclTopoSearchRecGpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<count; i++) {
      NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, step+1, backToNet, backToFirstRank, forcedOrder, time, GPU, g, next[i]));
    }
  } else if (step == backToFirstRank) {
    // Find first GPU and loop back to it
/* 中文逐项-变量 V1384：p
 * p（int；函数 ncclTopoSearchRecGpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getGpuIndex(system, graph->intra[graph->nChannels*ngpus],
 * &p));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int p;
    NCCLCHECK(getGpuIndex(system, graph->intra[graph->nChannels*ngpus], &p));
/* 中文逐项-变量 V1385：firstGpu
 * firstGpu（struct ncclTopoNode；函数 ncclTopoSearchRecGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, GPU, p, 1,
 * &firstGpu));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclTopoNode* firstGpu;
    NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, GPU, p, 1, &firstGpu));
    if (firstGpu) {
      NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, firstGpu, step+1, backToNet, -1, forcedOrder, time));
      NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, GPU, p, -1, &firstGpu));
    }
  } else {
    // Next path
    NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, ngpus, -1, -1, forcedOrder, time));
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0480：ncclTopoSearchRecNet
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const int speed = graph->speedInter;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int* nets;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 准备局部状态：int netcount;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSelectNets(system, graph->typeInter, -1, nets, &netcount));；失败时可能提前return，成功才继续；按路径级别、GPU 亲和性和网络设备分组挑出候选 NIC，使机间端点选择与机内路径匹配。
 * 6. 循环：初始 int i=0;；继续条件 i<netcount；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 准备局部状态：int n = nets[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 准备局部状态：struct ncclTopoNode* net = system->nodes[NET].nodes+n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   9. 准备局部状态：struct ncclTopoNode* gpu;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   10. 条件判断：(graph->collNet && net->net.collSupport == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     11. 真分支：；仅条件为真执行以下子项。
 *       12. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   13. 条件判断：(net->net.width < speed)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     14. 真分支：；仅条件为真执行以下子项。
 *       15. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   16. 条件判断：(net->net.maxChannels == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     17. 真分支：；仅条件为真执行以下子项。
 *       18. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   19. 调用 / 计算 / 写入：graph->inter[graph->nChannels*2] = net->id;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   20. 调用 / 计算 / 写入：graph->latencyInter = net->net.latency;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   21. 循环：初始 int i=0;；继续条件 i<system->nodes[NET].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     22. 条件判断：((system->nodes[NET].nodes[i].net.asic == net->net.asic) && (system->nodes[NET].nodes[i].net.port == net->net.port))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       23. 真分支：；仅条件为真执行以下子项。
 *         24. 调用 / 计算 / 写入：system->nodes[NET].nodes[i].net.width -= speed;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   25. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   26. 调用 / 计算 / 写入：net->net.maxChannels--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   27. 条件判断：(graph->nChannels > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     28. 真分支：；仅条件为真执行以下子项。
 *       29. 准备局部状态：int g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       30. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));；失败时可能提前return，成功才继续；重放已有 channel 的 GPU 顺序，允许 sameChannels 搜索复用前面找到的排列。
 *       31. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_REPLAY, time, NET, n, g));；失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。
 *   32. 条件判断：(graph->nChannels == 0 || graph->sameChannels == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     33. 真分支：；仅条件为真执行以下子项。
 *       34. 条件判断：(graph->nChannels == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         35. 真分支：；仅条件为真执行以下子项。
 *           36. 准备局部状态：int t = 1 << 10;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           37. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_PCI, &t, NET, n, 0));；失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。
 *           38. 条件判断：(t == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             39. 真分支：；仅条件为真执行以下子项。
 *               40. 调用 / 计算 / 写入：*time = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       41. 准备局部状态：float maxWidth = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       42. 准备局部状态：int minHops = 0xfffffff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       43. 准备局部状态：struct ncclTopoLinkList* paths = net->paths[GPU];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       44. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         45. 条件判断：(paths[g].width > maxWidth)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           46. 真分支：；仅条件为真执行以下子项。
 *             47. 调用 / 计算 / 写入：maxWidth = paths[g].width;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             48. 调用 / 计算 / 写入：minHops = paths[g].count;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           49. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             50. 条件判断：(paths[g].width == maxWidth && paths[g].count < minHops)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               51. 真分支：；仅条件为真执行以下子项。
 *                 52. 调用 / 计算 / 写入：minHops = paths[g].count;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       53. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       54. 条件判断：(maxWidth >= speed)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         55. 真分支：；仅条件为真执行以下子项。
 *           56. 循环：初始 int tryGpuBidir=0;；继续条件 tryGpuBidir<2；每轮更新 tryGpuBidir++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *             57. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               58. 条件判断：(paths[g].width == maxWidth && paths[g].count == minHops)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 59. 真分支：；仅条件为真执行以下子项。
 *                   60. 调用 / 计算 / 写入：gpu = system->nodes[GPU].nodes+g;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   61. 准备局部状态：int gpuUsed = gpuPciWidth(gpu) > 0 ? 0 : 1;；统计 GPU 的 PCI 通路宽度，为下一 GPU 候选排序加入 PCI 容量因素。
 *                   62. 条件判断：(tryGpuBidir == gpuUsed)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     63. 真分支：；仅条件为真执行以下子项。
 *                       64. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, 0, time, NET, n, g));；失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。
 *             65. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           66. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   67. 调用 / 计算 / 写入：net->net.maxChannels++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   68. 循环：初始 int i=0;；继续条件 i<system->nodes[NET].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     69. 条件判断：((system->nodes[NET].nodes[i].net.asic == net->net.asic) && (system->nodes[NET].nodes[i].net.port == net->net.port))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       70. 真分支：；仅条件为真执行以下子项。
 *         71. 调用 / 计算 / 写入：system->nodes[NET].nodes[i].net.width += speed;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   72. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 73. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 74. 调用 / 计算 / 写入：free(nets);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 75. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0480
 */
/* 中文逐项-函数 F0480：ncclTopoSearchRecNet
 * 用途与位置：递归尝试通道的网络入口/出口和机内 GPU 起点，并根据 crossNic 限制组合。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 saveGraph（struct ncclTopoGraph* saveGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 backToNet（int backToNet）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 backToFirstRank（int backToFirstRank）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 time（int* time）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRec。
 * 本体直接出现的调用/包装：NCCLCHECK；free；gpuPciWidth；ncclCalloc；ncclTopoReplayGetGpu；ncclTopoSearchTryGpu；ncclTopoSelectNets。
 * 需要对照的边界/推进条件：i<netcount；graph->collNet && net->net.collSupport == 0；net->net.width < speed；net->net.maxChannels ==
 * 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSearchRecNet(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, struct ncclTopoGraph* saveGraph, int backToNet, int backToFirstRank, int* time) {
/* 中文逐项-变量 V1386：speed
 * speed（int；函数 ncclTopoSearchRecNet 内）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 初值/来源 speed = graph->speedInter；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  const int speed = graph->speedInter;
/* 中文逐项-变量 V1387：nets
 * nets（int；函数 ncclTopoSearchRecNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int* nets;
  NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));
/* 中文逐项-变量 V1388：netcount
 * netcount（int；函数 ncclTopoSearchRecNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoSelectNets(system, graph->typeInter, -1, nets,
 * &netcount));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int netcount;
  NCCLCHECK(ncclTopoSelectNets(system, graph->typeInter, -1, nets, &netcount));
  for (/* 中文逐项-变量 V1389：i
 * i（int；函数 ncclTopoSearchRecNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<netcount；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<netcount; i++) {
/* 中文逐项-变量 V1390：n
 * n（int；函数 ncclTopoSearchRecNet 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = nets[i]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int n = nets[i];
/* 中文逐项-变量 V1391：net
 * net（struct ncclTopoNode；函数 ncclTopoSearchRecNet 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 net = system->nodes[NET].nodes+n；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* net = system->nodes[NET].nodes+n;
/* 中文逐项-变量 V1392：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoSearchRecNet 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 此处未显式给初值；后续首个文本使用点：gpu = system->nodes[GPU].nodes+g;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclTopoNode* gpu;
    if (graph->collNet && net->net.collSupport == 0) continue;
    if (net->net.width < speed) continue;
    if (net->net.maxChannels == 0) continue;

    graph->inter[graph->nChannels*2] = net->id;
    graph->latencyInter = net->net.latency;

    for (/* 中文逐项-变量 V1393：i
 * i（int；函数 ncclTopoSearchRecNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[NET].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[NET].count; i++) {
      if ((system->nodes[NET].nodes[i].net.asic == net->net.asic) &&
          (system->nodes[NET].nodes[i].net.port == net->net.port)) {
        system->nodes[NET].nodes[i].net.width -= speed;
      }
    }
    net->net.maxChannels--;

    // First try to replay the last channel
    if (graph->nChannels > 0) {
/* 中文逐项-变量 V1394：g
 * g（int；函数 ncclTopoSearchRecNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int g;
      NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));
      NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_REPLAY, time, NET, n, g));
    }
    if (graph->nChannels == 0 || graph->sameChannels == 0) {
      if (graph->nChannels == 0) {
        // Always try the PCI order first to set a reference, but don't count in the timeout nor let it run for long
/* 中文逐项-变量 V1395：t
 * t（int；函数 ncclTopoSearchRecNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = 1 << 10；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
        int t = 1 << 10;
        NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_PCI, &t, NET, n, 0));
        if (t == -1) *time = -1;
      }

      // Then try the most local GPUs
/* 中文逐项-变量 V1396：maxWidth
 * maxWidth（float；函数 ncclTopoSearchRecNet 内）：硬件系统最大单路带宽和可聚合宽度，用于设置搜索速度与提前终止条件。
 * 初值/来源 maxWidth = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      float maxWidth = 0;
/* 中文逐项-变量 V1397：minHops
 * minHops（int；函数 ncclTopoSearchRecNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minHops = 0xfffffff；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int minHops = 0xfffffff;
/* 中文逐项-变量 V1398：paths
 * paths（struct ncclTopoLinkList；函数 ncclTopoSearchRecNet 内）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 * 初值/来源 paths = net->paths[GPU]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoLinkList* paths = net->paths[GPU];
      for (/* 中文逐项-变量 V1399：g
 * g（int；函数 ncclTopoSearchRecNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
        if (paths[g].width > maxWidth) {
          maxWidth = paths[g].width;
          minHops = paths[g].count;
        } else if (paths[g].width == maxWidth && paths[g].count < minHops) {
          minHops = paths[g].count;
        }
      }
      if (maxWidth >= speed) {
        // In the first loop, avoid using GPUs in both directions between channels (one channel
        // sending from that GPU and one channel receiving to that GPU), since that usually leads
        // to lower BW.
        for (/* 中文逐项-变量 V1400：tryGpuBidir
 * tryGpuBidir（int；函数 ncclTopoSearchRecNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 tryGpuBidir = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：tryGpuBidir<2；循环体与增量表达式决定每轮如何推进。
 */int tryGpuBidir=0; tryGpuBidir<2; tryGpuBidir++) {
          for (/* 中文逐项-变量 V1401：g
 * g（int；函数 ncclTopoSearchRecNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
            if (paths[g].width == maxWidth && paths[g].count == minHops) {
              gpu = system->nodes[GPU].nodes+g;
/* 中文逐项-变量 V1402：gpuUsed
 * gpuUsed（int；函数 ncclTopoSearchRecNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gpuUsed = gpuPciWidth(gpu) > 0 ? 0 : 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
              int gpuUsed = gpuPciWidth(gpu) > 0 ? 0 : 1;
              if (tryGpuBidir == gpuUsed) {
                NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, 0, time, NET, n, g));
              }
            }
          }
        }
      }
    }

    net->net.maxChannels++;
    for (/* 中文逐项-变量 V1403：i
 * i（int；函数 ncclTopoSearchRecNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[NET].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[NET].count; i++) {
      if ((system->nodes[NET].nodes[i].net.asic == net->net.asic) &&
          (system->nodes[NET].nodes[i].net.port == net->net.port)) {
        system->nodes[NET].nodes[i].net.width += speed;
      }
    }
  }
  free(nets);
  return ncclSuccess;
}

/* Search Patterns
 *
 *     Intra-node
 * Ring            : GPU a -> GPU b -> .. -> GPU x -> GPU a
 * (=Split Tree Loop)
 * Tree            : GPU a -> GPU b -> .. -> GPU x
 * (=Split Tree)
 *
 *     Inter-node
 * Ring            : NET n -> GPU a -> GPU b -> .. -> GPU x -> NET n (or m if crossNic)
 * Tree            : NET n -> GPU a -> GPU b -> .. -> GPU x
 *                              `--> NET n (or m if crossNic)
 * Split Tree      : NET n -> GPU a -> GPU b -> .. -> GPU x
 *                                       `--> NET n (or m if crossNic)
 * Split Tree Loop : NET n -> GPU a -> GPU b -> .. -> GPU x -> GPU a
 *                                       `--> NET n (or m if crossNic)
 */
/* 中文内部逻辑 F0481：ncclTopoSearchParams
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(system->nodes[NET].count)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 条件判断：(pattern == NCCL_TOPO_PATTERN_RING)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       4. 真分支：；仅条件为真执行以下子项。
 *         5. 调用 / 计算 / 写入：*backToNet = system->nodes[GPU].count-1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         7. 条件判断：(pattern == NCCL_TOPO_PATTERN_SPLIT_TREE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           8. 真分支：；仅条件为真执行以下子项。
 *             9. 调用 / 计算 / 写入：*backToNet = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           10. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             11. 调用 / 计算 / 写入：*backToNet = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     12. 调用 / 计算 / 写入：*backToFirstRank = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   13. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     14. 调用 / 计算 / 写入：*backToNet = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     15. 条件判断：(pattern == NCCL_TOPO_PATTERN_RING)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       16. 真分支：；仅条件为真执行以下子项。
 *         17. 调用 / 计算 / 写入：*backToFirstRank = system->nodes[GPU].count-1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       18. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         19. 调用 / 计算 / 写入：*backToFirstRank = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0481
 */
/* 中文逐项-函数 F0481：ncclTopoSearchParams
 * 用途与位置：把 Ring/Tree/Split/Balanced 等模式转换成何时返回 NIC、何时闭合首 GPU 的搜索约束。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 pattern（int pattern）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 * 参数 backToNet（int* backToNet）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 backToFirstRank（int* backToFirstRank）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRec。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：system->nodes[NET].count；pattern == NCCL_TOPO_PATTERN_RING；pattern ==
 * NCCL_TOPO_PATTERN_SPLIT_TREE；pattern == NCCL_TOPO_PATTERN_RING。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSearchParams(struct ncclTopoSystem* system, int pattern, int* backToNet, int* backToFirstRank) {
  if (system->nodes[NET].count) {
    if (pattern == NCCL_TOPO_PATTERN_RING) *backToNet = system->nodes[GPU].count-1;
    else if (pattern == NCCL_TOPO_PATTERN_SPLIT_TREE) *backToNet = 1;
    else *backToNet = 0;
    *backToFirstRank = -1;
  } else {
    *backToNet = -1;
    if (pattern == NCCL_TOPO_PATTERN_RING) *backToFirstRank = system->nodes[GPU].count-1;
    else *backToFirstRank = -1;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0482：ncclTopoSearchRec
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int backToNet, backToFirstRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchParams(system, graph->pattern, &backToNet, &backToFirstRank));；失败时可能提前return，成功才继续；把 Ring/Tree/Split/Balanced 等模式转换成何时返回 NIC、何时闭合首 GPU 的搜索约束。
 * 3. 条件判断：(system->nodes[NET].count)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：ncclTopoSearchRecNet(system, graph, saveGraph, backToNet, backToFirstRank, time);；递归尝试通道的网络入口/出口和机内 GPU 起点，并根据 crossNic 限制组合。
 *   6. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     7. 条件判断：(graph->nChannels == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       8. 真分支：；仅条件为真执行以下子项。
 *         9. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_PCI, time, -1, -1, 0));；失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。
 *       10. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         11. 准备局部状态：int g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         12. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));；失败时可能提前return，成功才继续；重放已有 channel 的 GPU 顺序，允许 sameChannels 搜索复用前面找到的排列。
 *         13. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_REPLAY, time, -1, -1, g));；失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。
 *     14. 条件判断：(graph->sameChannels == 0 || graph->nChannels == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       15. 真分支：；仅条件为真执行以下子项。
 *         16. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           17. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, 0, time, -1, -1, g));；失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。
 *         18. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0482
 */
/* 中文逐项-函数 F0482：ncclTopoSearchRec
 * 用途与位置：为一次候选搜索初始化已用节点状态与剩余预算，启动网络或机内递归搜索。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 saveGraph（struct ncclTopoGraph* saveGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 time（int* time）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoSearchRecGpu；src/graph/search.cc::ncclTopoCompute。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoReplayGetGpu；ncclTopoSearchParams；ncclTopoSearchRecNet；ncclTopoSearchTryGpu。
 * 需要对照的边界/推进条件：system->nodes[NET].count；graph->nChannels == 0；graph->sameChannels == 0 || graph->nChannels ==
 * 0；g<system->nodes[GPU].count。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSearchRec(struct ncclTopoSystem* system, struct ncclTopoGraph* graph, struct ncclTopoGraph* saveGraph, int* time) {
/* 中文逐项-变量 V1404：backToNet、backToFirstRank
 * backToNet（int；函数 ncclTopoSearchRec 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoSearchParams(system, graph->pattern, &backToNet,
 * &backToFirstRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 * backToFirstRank（int；函数 ncclTopoSearchRec 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoSearchParams(system, graph->pattern, &backToNet,
 * &backToFirstRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int backToNet, backToFirstRank;
  NCCLCHECK(ncclTopoSearchParams(system, graph->pattern, &backToNet, &backToFirstRank));
  if (system->nodes[NET].count) {
    // Start from NET
    ncclTopoSearchRecNet(system, graph, saveGraph, backToNet, backToFirstRank, time);
  } else {
    // Intra-node only.
    if (graph->nChannels == 0) {
      // Try PCI order first
      NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_PCI, time, -1, -1, 0));
    } else {
      // Also try to replay previous channel
/* 中文逐项-变量 V1405：g
 * g（int；函数 ncclTopoSearchRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int g;
      NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));
      NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_REPLAY, time, -1, -1, g));
    }
    if (graph->sameChannels == 0 || graph->nChannels == 0) {
      // Finally, try all other possibilities unless we are forced to use the same channels
      for (/* 中文逐项-变量 V1406：g
 * g（int；函数 ncclTopoSearchRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
        NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, 0, time, -1, -1, g));
      }
    }
  }
  return ncclSuccess;
}

/************************************/
/* User defined graph from XML file */
/************************************/

/* 中文逐项-变量 V1407：kvDictLinkType
 * kvDictLinkType（struct kvDict；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 kvDictLinkType = { { "LOC", PATH_LOC }, { "NVL", PATH_NVL }, { "NVB", PATH_NVB }, { "PIX", PATH_PIX }, {
 * "PXB", PATH_PXB }, { "PXN", PATH_PXN }, { "PHB", PATH_PHB }, { "SYS", PATH_SYS }, { NULL, 0 }
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct kvDict kvDictLinkType[] = {
  { "LOC", PATH_LOC },
  { "NVL", PATH_NVL },
  { "NVB", PATH_NVB },
  { "PIX", PATH_PIX },
  { "PXB", PATH_PXB },
  { "PXN", PATH_PXN },
  { "PHB", PATH_PHB },
  { "SYS", PATH_SYS },
  { NULL, 0 }
};

/* 中文内部逻辑 F0483：ncclTopoGetChannelFromXml
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int* inter = graph->inter+2*c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int* intra = graph->intra+ngpus*c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int n=0, g=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 循环：初始 int s=0;；继续条件 s<xmlChannel->nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 准备局部状态：struct ncclXmlNode* sub = xmlChannel->subs[s];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   7. 准备局部状态：int dev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(sub, "dev", &dev));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 *   9. 条件判断：(strcmp(sub->name, "net") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     10. 真分支：；仅条件为真执行以下子项。
 *       11. 调用 / 计算 / 写入：inter[n++] = dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     12. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       13. 条件判断：(strcmp(sub->name, "gpu") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         14. 真分支：；仅条件为真执行以下子项。
 *           15. 准备局部状态：int rank = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           16. 循环：初始 int g=0;；继续条件 g<ngpus；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *             17. 条件判断：(system->nodes[GPU].nodes[g].gpu.dev == dev)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               18. 真分支：；仅条件为真执行以下子项。
 *                 19. 调用 / 计算 / 写入：rank = system->nodes[GPU].nodes[g].gpu.rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           21. 条件判断：(rank == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             22. 真分支：；仅条件为真执行以下子项。
 *               23. 调用 / 计算 / 写入：WARN("XML Import Channel : dev %d not found.", dev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *               24. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *           25. 调用 / 计算 / 写入：intra[g++] = rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 26. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 27. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0483
 */
/* 中文逐项-函数 F0483：ncclTopoGetChannelFromXml
 * 用途与位置：从一个 XML channel 读取 GPU 顺序和 NIC 端点，校验并写入搜索图数组。
 * 参数 xmlChannel（struct ncclXmlNode *xmlChannel）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 c（int c）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetGraphFromXmlSub。
 * 本体直接出现的调用/包装：NCCLCHECK；strcmp；xmlGetAttrInt。
 * 需要对照的边界/推进条件：s<xmlChannel->nSubs；strcmp(sub->name, "net") == 0；strcmp(sub->name, "gpu") == 0；g<ngpus。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetChannelFromXml(struct ncclXmlNode *xmlChannel, int c, struct ncclTopoSystem* system, struct ncclTopoGraph* graph) {
/* 中文逐项-变量 V1408：ngpus
 * ngpus（int；函数 ncclTopoGetChannelFromXml 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;
/* 中文逐项-变量 V1409：inter
 * inter（int；函数 ncclTopoGetChannelFromXml 内）：按channel记录机内GPU用户rank排列及两个网络端点，后续转成具体邻居连接。
 * 初值/来源 inter = graph->inter+2*c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int* inter = graph->inter+2*c;
/* 中文逐项-变量 V1410：intra
 * intra（int；函数 ncclTopoGetChannelFromXml 内）：按channel记录机内GPU用户rank排列及两个网络端点，后续转成具体邻居连接。
 * 初值/来源 intra = graph->intra+ngpus*c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int* intra = graph->intra+ngpus*c;
/* 中文逐项-变量 V1411：n、g
 * n（int；函数 ncclTopoGetChannelFromXml 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * g（int；函数 ncclTopoGetChannelFromXml 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int n=0, g=0;
  for (/* 中文逐项-变量 V1412：s
 * s（int；函数 ncclTopoGetChannelFromXml 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<xmlChannel->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<xmlChannel->nSubs; s++) {
/* 中文逐项-变量 V1413：sub
 * sub（struct ncclXmlNode；函数 ncclTopoGetChannelFromXml 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = xmlChannel->subs[s]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* sub = xmlChannel->subs[s];
/* 中文逐项-变量 V1414：dev
 * dev（int；函数 ncclTopoGetChannelFromXml 内）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(sub, "dev", &dev));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int dev;
    NCCLCHECK(xmlGetAttrInt(sub, "dev", &dev));
    if (strcmp(sub->name, "net") == 0) {
      inter[n++] = dev;
    } else if (strcmp(sub->name, "gpu") == 0) {
/* 中文逐项-变量 V1415：rank
 * rank（int；函数 ncclTopoGetChannelFromXml 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
      int rank = -1;
      for (/* 中文逐项-变量 V1416：g
 * g（int；函数 ncclTopoGetChannelFromXml 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<ngpus；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<ngpus; g++) {
        if (system->nodes[GPU].nodes[g].gpu.dev == dev) rank = system->nodes[GPU].nodes[g].gpu.rank;
      }
      if (rank == -1) {
        WARN("XML Import Channel : dev %d not found.", dev);
        return ncclSystemError;
      }
      intra[g++] = rank;
    }
  }
  return ncclSuccess;
}
/* 中文内部逻辑 F0484：ncclTopoGetGraphFromXmlSub
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int id;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(xmlGraph, "id", &id));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 3. 条件判断：(graph->id != id)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 准备局部状态：int crossNic;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(xmlGraph, "crossnic", &crossNic));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 8. 条件判断：(graph->crossNic == 0 && crossNic == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 11. 调用 / 计算 / 写入：graph->crossNic = crossNic;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(xmlGraph, "pattern", &graph->pattern));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 13. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(xmlGraph, "nchannels", &graph->nChannels));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 14. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrFloat(xmlGraph, "speedintra", &graph->speedIntra));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 15. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrFloat(xmlGraph, "speedinter", &graph->speedInter));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 16. 条件判断：(xmlGetAttrFloat(xmlGraph, "latencyinter", &graph->latencyInter) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   17. 真分支：；仅条件为真执行以下子项。
 *     18. 调用 / 计算 / 写入：graph->latencyInter = 0.0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 准备局部状态：const char* str;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 20. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(xmlGraph, "typeintra", &str));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 21. 调用 / 计算 / 写入：NCCLCHECK(kvConvertToInt(str, &graph->typeIntra, kvDictLinkType));；失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。
 * 22. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(xmlGraph, "typeinter", &str));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 23. 调用 / 计算 / 写入：NCCLCHECK(kvConvertToInt(str, &graph->typeInter, kvDictLinkType));；失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。
 * 24. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(xmlGraph, "samechannels", &graph->sameChannels));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 25. 循环：初始 int s=0;；继续条件 s<xmlGraph->nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   26. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetChannelFromXml(xmlGraph->subs[s], s, system, graph));；失败时可能提前return，成功才继续；从一个 XML channel 读取 GPU 顺序和 NIC 端点，校验并写入搜索图数组。
 * 27. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 28. 调用 / 计算 / 写入：*nChannels = xmlGraph->nSubs;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 29. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0484
 */
/* 中文逐项-函数 F0484：ncclTopoGetGraphFromXmlSub
 * 用途与位置：解析匹配 graph 的速度、模式与多条通道，恢复预先导出的搜索方案。
 * 参数 xmlGraph（struct ncclXmlNode *xmlGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 nChannels（int* nChannels）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetGraphFromXml。
 * 本体直接出现的调用/包装：NCCLCHECK；kvConvertToInt；ncclTopoGetChannelFromXml；xmlGetAttr；xmlGetAttrFloat；xmlGetAttrInt。
 * 需要对照的边界/推进条件：graph->id != id；graph->crossNic == 0 && crossNic == 1；xmlGetAttrFloat(xmlGraph, "latencyinter",
 * &graph->latencyInter) != ncclSuccess；s<xmlGraph->nSubs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetGraphFromXmlSub(struct ncclXmlNode *xmlGraph, struct ncclTopoSystem* system, struct ncclTopoGraph* graph, int* nChannels) {
/* 中文逐项-变量 V1417：id
 * id（int；函数 ncclTopoGetGraphFromXmlSub 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlGraph, "id", &id));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int id;
  NCCLCHECK(xmlGetAttrInt(xmlGraph, "id", &id));
  if (graph->id != id) return ncclSuccess;

/* 中文逐项-变量 V1418：crossNic
 * crossNic（int；函数 ncclTopoGetGraphFromXmlSub 内）：图搜索是否允许进出不同NIC、是否重放相同机内GPU顺序的限制。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(xmlGraph, "crossnic", &crossNic));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int crossNic;
  NCCLCHECK(xmlGetAttrInt(xmlGraph, "crossnic", &crossNic));
  if (graph->crossNic == 0 && crossNic == 1) return ncclSuccess;
  graph->crossNic = crossNic;

  NCCLCHECK(xmlGetAttrInt(xmlGraph, "pattern", &graph->pattern));
  NCCLCHECK(xmlGetAttrInt(xmlGraph, "nchannels", &graph->nChannels));
  NCCLCHECK(xmlGetAttrFloat(xmlGraph, "speedintra", &graph->speedIntra));
  NCCLCHECK(xmlGetAttrFloat(xmlGraph, "speedinter", &graph->speedInter));
  if (xmlGetAttrFloat(xmlGraph, "latencyinter", &graph->latencyInter) != ncclSuccess) graph->latencyInter = 0.0;
/* 中文逐项-变量 V1419：str
 * str（char；函数 ncclTopoGetGraphFromXmlSub 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(xmlGraph, "typeintra", &str));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;
  NCCLCHECK(xmlGetAttr(xmlGraph, "typeintra", &str));
  NCCLCHECK(kvConvertToInt(str, &graph->typeIntra, kvDictLinkType));
  NCCLCHECK(xmlGetAttr(xmlGraph, "typeinter", &str));
  NCCLCHECK(kvConvertToInt(str, &graph->typeInter, kvDictLinkType));
  NCCLCHECK(xmlGetAttrInt(xmlGraph, "samechannels", &graph->sameChannels));
  for (/* 中文逐项-变量 V1420：s
 * s（int；函数 ncclTopoGetGraphFromXmlSub 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<xmlGraph->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<xmlGraph->nSubs; s++) {
    NCCLCHECK(ncclTopoGetChannelFromXml(xmlGraph->subs[s], s, system, graph));
  }
  *nChannels = xmlGraph->nSubs;
  return ncclSuccess;
}
/* 中文内部逻辑 F0485：ncclTopoGetGraphFromXml
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int s=0;；继续条件 s<xmlGraphs->nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetGraphFromXmlSub(xmlGraphs->subs[s], system, graph, nChannels));；失败时可能提前return，成功才继续；解析匹配 graph 的速度、模式与多条通道，恢复预先导出的搜索方案。
 * 3. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0485
 */
/* 中文逐项-函数 F0485：ncclTopoGetGraphFromXml
 * 用途与位置：在 XML 中找到当前 graph id 的对象，再调用子解析器填充通信图。
 * 参数 xmlGraphs（struct ncclXmlNode *xmlGraphs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 nChannels（int* nChannels）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoCompute。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoGetGraphFromXmlSub。
 * 需要对照的边界/推进条件：s<xmlGraphs->nSubs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetGraphFromXml(struct ncclXmlNode *xmlGraphs, struct ncclTopoSystem* system, struct ncclTopoGraph* graph, int* nChannels) {
  for (/* 中文逐项-变量 V1421：s
 * s（int；函数 ncclTopoGetGraphFromXml 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<xmlGraphs->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<xmlGraphs->nSubs; s++) {
    NCCLCHECK(ncclTopoGetGraphFromXmlSub(xmlGraphs->subs[s], system, graph, nChannels));
  }
  return ncclSuccess;
}

/* And the reverse : graph->xml */
/* 中文内部逻辑 F0486：ncclTopoGetXmlFromChannel
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclXmlNode* xmlChannel;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int* inter = graph->inter+2*c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int* intra = graph->intra+ngpus*c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, parent, "channel", &xmlChannel));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 * 6. 准备局部状态：struct ncclXmlNode* node;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 条件判断：(system->nodes[NET].count)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, xmlChannel, "net", &node));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 *     10. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(node, "dev", inter[0]));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 11. 循环：初始 int g=0;；继续条件 g<ngpus；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   12. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, xmlChannel, "gpu", &node));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 *   13. 准备局部状态：int dev = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   14. 循环：初始 int i=0;；继续条件 i<ngpus；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     15. 条件判断：(system->nodes[GPU].nodes[i].gpu.rank == intra[g])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       16. 真分支：；仅条件为真执行以下子项。
 *         17. 调用 / 计算 / 写入：dev = system->nodes[GPU].nodes[i].gpu.dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   18. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   19. 条件判断：(dev == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     20. 真分支：；仅条件为真执行以下子项。
 *       21. 调用 / 计算 / 写入：WARN("XML Export Channel : rank %d not found.", intra[g]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       22. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   23. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(node, "dev", dev));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 24. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 25. 条件判断：(system->nodes[NET].count)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   26. 真分支：；仅条件为真执行以下子项。
 *     27. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, xmlChannel, "net", &node));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 *     28. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(node, "dev", inter[1]));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 29. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0486
 */
/* 中文逐项-函数 F0486：ncclTopoGetXmlFromChannel
 * 用途与位置：把单条 channel 的 GPU/NIC 顺序写成 XML 节点，供人工检查或离线固定图。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 c（int c）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 xml（struct ncclXml *xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 parent（struct ncclXmlNode* parent）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetXmlFromGraph。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlAddNode；xmlSetAttrInt。
 * 需要对照的边界/推进条件：system->nodes[NET].count；g<ngpus；i<ngpus；system->nodes[GPU].nodes[i].gpu.rank ==
 * intra[g]。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetXmlFromChannel(struct ncclTopoGraph* graph, int c, struct ncclTopoSystem* system, struct ncclXml *xml, struct ncclXmlNode* parent) {
/* 中文逐项-变量 V1422：xmlChannel
 * xmlChannel（struct ncclXmlNode；函数 ncclTopoGetXmlFromChannel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlAddNode(xml, parent, "channel", &xmlChannel));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclXmlNode* xmlChannel;
/* 中文逐项-变量 V1423：ngpus
 * ngpus（int；函数 ncclTopoGetXmlFromChannel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;
/* 中文逐项-变量 V1424：inter
 * inter（int；函数 ncclTopoGetXmlFromChannel 内）：按channel记录机内GPU用户rank排列及两个网络端点，后续转成具体邻居连接。
 * 初值/来源 inter = graph->inter+2*c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int* inter = graph->inter+2*c;
/* 中文逐项-变量 V1425：intra
 * intra（int；函数 ncclTopoGetXmlFromChannel 内）：按channel记录机内GPU用户rank排列及两个网络端点，后续转成具体邻居连接。
 * 初值/来源 intra = graph->intra+ngpus*c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int* intra = graph->intra+ngpus*c;
  NCCLCHECK(xmlAddNode(xml, parent, "channel", &xmlChannel));
/* 中文逐项-变量 V1426：node
 * node（struct ncclXmlNode；函数 ncclTopoGetXmlFromChannel 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlAddNode(xml, xmlChannel, "net", &node));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclXmlNode* node;
  if (system->nodes[NET].count) {
    NCCLCHECK(xmlAddNode(xml, xmlChannel, "net", &node));
    NCCLCHECK(xmlSetAttrInt(node, "dev", inter[0]));
  }
  for (/* 中文逐项-变量 V1427：g
 * g（int；函数 ncclTopoGetXmlFromChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<ngpus；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<ngpus; g++) {
    NCCLCHECK(xmlAddNode(xml, xmlChannel, "gpu", &node));
/* 中文逐项-变量 V1428：dev
 * dev（int；函数 ncclTopoGetXmlFromChannel 内）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 初值/来源 dev = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
    int dev = -1;
    for (/* 中文逐项-变量 V1429：i
 * i（int；函数 ncclTopoGetXmlFromChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ngpus；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ngpus; i++) {
      if (system->nodes[GPU].nodes[i].gpu.rank == intra[g]) dev = system->nodes[GPU].nodes[i].gpu.dev;
    }
    if (dev == -1) {
      WARN("XML Export Channel : rank %d not found.", intra[g]);
      return ncclInternalError;
    }
    NCCLCHECK(xmlSetAttrInt(node, "dev", dev));
  }
  if (system->nodes[NET].count) {
    NCCLCHECK(xmlAddNode(xml, xmlChannel, "net", &node));
    NCCLCHECK(xmlSetAttrInt(node, "dev", inter[1]));
  }
  return ncclSuccess;
}
/* 中文内部逻辑 F0487：ncclTopoGetXmlFromGraph
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclXmlNode* xmlGraph;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, parent, "graph", &xmlGraph));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(xmlGraph, "id", graph->id));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(xmlGraph, "pattern", graph->pattern));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(xmlGraph, "crossnic", graph->crossNic));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(xmlGraph, "nchannels", graph->nChannels));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrFloat(xmlGraph, "speedintra", graph->speedIntra));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrFloat(xmlGraph, "speedinter", graph->speedInter));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrFloat(xmlGraph, "latencyinter", graph->latencyInter));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 10. 准备局部状态：const char* str;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 调用 / 计算 / 写入：NCCLCHECK(kvConvertToStr(graph->typeIntra, &str, kvDictLinkType));；失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(xmlGraph, "typeintra", str));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 13. 调用 / 计算 / 写入：NCCLCHECK(kvConvertToStr(graph->typeInter, &str, kvDictLinkType));；失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。
 * 14. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(xmlGraph, "typeinter", str));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 15. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(xmlGraph, "samechannels", graph->sameChannels));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 16. 循环：初始 int c=0;；继续条件 c<graph->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   17. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromChannel(graph, c, system, xml, xmlGraph));；失败时可能提前return，成功才继续；把单条 channel 的 GPU/NIC 顺序写成 XML 节点，供人工检查或离线固定图。
 * 18. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0487
 */
/* 中文逐项-函数 F0487：ncclTopoGetXmlFromGraph
 * 用途与位置：把通信图参数和各 channel 顺序写成 XML graph 对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 xml（struct ncclXml *xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 parent（struct ncclXmlNode* parent）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetXmlFromGraphs。
 * 本体直接出现的调用/包装：NCCLCHECK；kvConvertToStr；ncclTopoGetXmlFromChannel；xmlAddNode；xmlSetAttr；xmlSetAttrFloat；xmlSetAttrInt。
 * 需要对照的边界/推进条件：c<graph->nChannels。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetXmlFromGraph(struct ncclTopoGraph* graph, struct ncclTopoSystem* system, struct ncclXml *xml, struct ncclXmlNode* parent) {
/* 中文逐项-变量 V1430：xmlGraph
 * xmlGraph（struct ncclXmlNode；函数 ncclTopoGetXmlFromGraph 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlAddNode(xml, parent, "graph", &xmlGraph));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclXmlNode* xmlGraph;
  NCCLCHECK(xmlAddNode(xml, parent, "graph", &xmlGraph));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "id", graph->id));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "pattern", graph->pattern));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "crossnic", graph->crossNic));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "nchannels", graph->nChannels));
  NCCLCHECK(xmlSetAttrFloat(xmlGraph, "speedintra", graph->speedIntra));
  NCCLCHECK(xmlSetAttrFloat(xmlGraph, "speedinter", graph->speedInter));
  NCCLCHECK(xmlSetAttrFloat(xmlGraph, "latencyinter", graph->latencyInter));
/* 中文逐项-变量 V1431：str
 * str（char；函数 ncclTopoGetXmlFromGraph 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(kvConvertToStr(graph->typeIntra, &str, kvDictLinkType));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;
  NCCLCHECK(kvConvertToStr(graph->typeIntra, &str, kvDictLinkType));
  NCCLCHECK(xmlSetAttr(xmlGraph, "typeintra", str));
  NCCLCHECK(kvConvertToStr(graph->typeInter, &str, kvDictLinkType));
  NCCLCHECK(xmlSetAttr(xmlGraph, "typeinter", str));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "samechannels", graph->sameChannels));
  for (/* 中文逐项-变量 V1432：c
 * c（int；函数 ncclTopoGetXmlFromGraph 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<graph->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<graph->nChannels; c++) {
    NCCLCHECK(ncclTopoGetXmlFromChannel(graph, c, system, xml, xmlGraph));
  }
  return ncclSuccess;
}
/* 中文内部逻辑 F0488：ncclTopoGetXmlFromGraphs
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：xml->maxIndex = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 准备局部状态：struct ncclXmlNode* xmlGraphs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, NULL, "graphs", &xmlGraphs));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(xmlGraphs, "version", NCCL_GRAPH_XML_VERSION));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 5. 循环：初始 int g=0;；继续条件 g<ngraphs；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromGraph(graphs[g], system, xml, xmlGraphs));；失败时可能提前return，成功才继续；把通信图参数和各 channel 顺序写成 XML graph 对象。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0488
 */
/* 中文逐项-函数 F0488：ncclTopoGetXmlFromGraphs
 * 用途与位置：创建 XML 根并导出多种算法的通信图，保持模式/id 信息。
 * 参数 ngraphs（int ngraphs）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 graphs（struct ncclTopoGraph** graphs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 xml（struct ncclXml *xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoDumpGraphs。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoGetXmlFromGraph；xmlAddNode；xmlSetAttrInt。
 * 需要对照的边界/推进条件：g<ngraphs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetXmlFromGraphs(int ngraphs, struct ncclTopoGraph** graphs, struct ncclTopoSystem* system, struct ncclXml *xml) {
  xml->maxIndex = 0;
/* 中文逐项-变量 V1433：xmlGraphs
 * xmlGraphs（struct ncclXmlNode；函数 ncclTopoGetXmlFromGraphs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlAddNode(xml, NULL, "graphs", &xmlGraphs));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclXmlNode* xmlGraphs;
  NCCLCHECK(xmlAddNode(xml, NULL, "graphs", &xmlGraphs));
  NCCLCHECK(xmlSetAttrInt(xmlGraphs, "version", NCCL_GRAPH_XML_VERSION));
  for (/* 中文逐项-变量 V1434：g
 * g（int；函数 ncclTopoGetXmlFromGraphs 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<ngraphs；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<ngraphs; g++) {
    NCCLCHECK(ncclTopoGetXmlFromGraph(graphs[g], system, xml, xmlGraphs));
  }
  return ncclSuccess;
}

/* 中文逐项-变量 V1435：speedArrayIntra
 * speedArrayIntra（float；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 speedArrayIntra = { 44.0, 30.0, 22.0, 18.0, 15.0, 12.0, 10.0, 9.0, 7.0, 6.0, 5.0, 4.0, 3.0
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
float speedArrayIntra[] = { 44.0, 30.0, 22.0, 18.0, 15.0, 12.0, 10.0, 9.0, 7.0, 6.0, 5.0, 4.0, 3.0 };
/* 中文逐项-变量 V1436：speedArrayInter
 * speedArrayInter（float；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 speedArrayInter = { 48.0, 30.0, 24.0, 22.0, 18.0, 15.0, 12.0, 10.0, 9.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.4, 1.2,
 * 0.24, 0.12 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
float speedArrayInter[] = { 48.0, 30.0, 24.0, 22.0, 18.0, 15.0, 12.0, 10.0, 9.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.4, 1.2, 0.24, 0.12 };
/* 中文逐项-常量 X0037：NSPEEDSINTRA
 * 编译期定义：NSPEEDSINTRA = (sizeof(speedArrayIntra)/sizeof(float))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NSPEEDSINTRA (sizeof(speedArrayIntra)/sizeof(float))
/* 中文逐项-常量 X0038：NSPEEDSINTER
 * 编译期定义：NSPEEDSINTER = (sizeof(speedArrayInter)/sizeof(float))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NSPEEDSINTER (sizeof(speedArrayInter)/sizeof(float))

/* 中文逐项-参数getter P0014：CrossNic
 * 生成函数 ncclParamCrossNic()，读取 NCCL_CROSS_NIC，编译默认值为 2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(CrossNic, "CROSS_NIC", 2);

/* 中文导读：
 * 从候选路径、带宽和 channel 数等条件搜索通信图，受搜索预算限制并可使用 XML 图输入。
 * 它寻找适合当前硬件的方案，不直接分配 QP，也不能被理解为固定按 GPU 数生成全连接。
 */
/* 中文精读：图示绑定 V3.3 F0489 ncclTopoCompute
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0489
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0091] ncclTopoCompute：硬件图内搜索带宽/路径约束下的通道候选；结果还须跨rank协调和全局拼接（直接对应）
 *   图：28-channel-generation
 *   定位：#symbol=F0489&binding=B0091
 */
/* 中文内部逻辑 F0489：ncclTopoCompute
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：graph->crossNic = ncclParamCrossNic();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 准备局部状态：int crossNic = (system->nodes[NET].count > 1) && graph->crossNic ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：graph->speedIntra = graph->speedInter = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：graph->latencyInter = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 条件判断：(graph->crossNic == 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 调用 / 计算 / 写入：graph->crossNic = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：graph->typeIntra = ngpus == 1 ? PATH_LOC : PATH_NVL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：graph->typeInter = PATH_PIX;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：graph->nChannels = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：graph->sameChannels = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 准备局部状态：char* str = getenv("NCCL_GRAPH_FILE");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 条件判断：(str)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_GRAPH_FILE set by environment to %s", str);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     17. 准备局部状态：struct ncclXml* xml;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     18. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&xml, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     19. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlGraphFromFile(str, xml));；失败时可能提前return，成功才继续；打开并解析通信图 XML 文件，校验图版本以恢复预先保存的搜索结果。
 *     20. 准备局部状态：int nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     21. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetGraphFromXml(xml->nodes, system, graph, &nChannels));；失败时可能提前return，成功才继续；在 XML 中找到当前 graph id 的对象，再调用子解析器填充通信图。
 *     22. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Search %d : %d channels loaded from XML graph", graph->id, nChannels);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     23. 调用 / 计算 / 写入：free(xml);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     24. 条件判断：(graph->nChannels > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       25. 真分支：；仅条件为真执行以下子项。
 *         26. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 27. 条件判断：(ngpus == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   28. 真分支：；仅条件为真执行以下子项。
 *     29. 条件判断：(graph->pattern != NCCL_TOPO_PATTERN_RING)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       30. 真分支：；仅条件为真执行以下子项。
 *         31. 调用 / 计算 / 写入：graph->pattern = NCCL_TOPO_PATTERN_TREE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 准备局部状态：int ccMin;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 33. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetCompCap(system, &ccMin, NULL));；失败时可能提前return，成功才继续；遍历 GPU 得到计算能力的最小/最大值，使协议与调优模型兼容参与设备。
 * 34. 准备局部状态：struct ncclTopoGraph tmpGraph;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 35. 调用 / 计算 / 写入：memcpy(&tmpGraph, graph, sizeof(struct ncclTopoGraph));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 36. 准备局部状态：int nspeeds = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 37. 准备局部状态：float* speedArray = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 38. 条件判断：(system->nodes[NET].count == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   39. 真分支：；仅条件为真执行以下子项。
 *     40. 调用 / 计算 / 写入：nspeeds = NSPEEDSINTRA;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     41. 调用 / 计算 / 写入：speedArray = speedArrayIntra;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   42. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     43. 调用 / 计算 / 写入：nspeeds = NSPEEDSINTER;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     44. 调用 / 计算 / 写入：speedArray = speedArrayInter;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 45. 准备局部状态：int pass = 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 46. 准备局部状态：int speedIndex = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 47. 循环：初始 ；继续条件 (speedArray[speedIndex] > system->maxWidth && speedIndex < nspeeds-1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   48. 调用 / 计算 / 写入：speedIndex++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 49. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 50. 调用 / 计算 / 写入：tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 51. 准备局部状态：int64_t globalTimeout = NCCL_SEARCH_GLOBAL_TIMEOUT;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 52. 跳转标签：search；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   53. 准备局部状态：int time = tmpGraph.sameChannels ? NCCL_SEARCH_TIMEOUT_SAMECHANNELS : tmpGraph.pattern == NCCL_TOPO_PATTERN_TREE ? NCCL_SEARCH_TIMEOUT_TREE : NCCL_SEARCH_TIMEOUT;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 54. 调用 / 计算 / 写入：tmpGraph.nChannels = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 55. 调用 / 计算 / 写入：globalTimeout -= time;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 56. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchRec(system, &tmpGraph, graph, &time));；失败时可能提前return，成功才继续；为一次候选搜索初始化已用节点状态与剩余预算，启动网络或机内递归搜索。
 * 57. 编译期分支/指令：#if 0；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   58. 调用 / 计算 / 写入：printf("Pattern %d, crossNic %d, Speed %g/%g, type %d/%d, channels %d-%d sameChannels %d -> nChannels %dx%g/%g %s\n", tmpGraph.pattern, tmpGraph.crossNic, tmpGraph.speedInter, tmpGraph.speedIntra, tmp …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   59. 循环：初始 int c=0;；继续条件 c<graph->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     60. 调用 / 计算 / 写入：printf("%2d : ", c);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     61. 循环：初始 int g=0;；继续条件 g<ngpus；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       62. 调用 / 计算 / 写入：printf("%d ", graph->intra[c*ngpus+g]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     63. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     64. 调用 / 计算 / 写入：printf("[%d %d]", graph->inter[c*2+0], graph->inter[c*2+1]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     65. 调用 / 计算 / 写入：printf("\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   66. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 67. 条件判断：(time == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   68. 真分支：；仅条件为真执行以下子项。
 *     69. 控制转移：goto done;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 70. 条件判断：(graph->nChannels*graph->speedInter >= system->totalWidth)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   71. 真分支：；仅条件为真执行以下子项。
 *     72. 控制转移：goto done;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 73. 条件判断：(pass == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   74. 真分支：；仅条件为真执行以下子项。
 *     75. 条件判断：(tmpGraph.sameChannels == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       76. 真分支：；仅条件为真执行以下子项。
 *         77. 调用 / 计算 / 写入：tmpGraph.sameChannels = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         78. 控制转移：goto search;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     79. 调用 / 计算 / 写入：tmpGraph.sameChannels = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     80. 条件判断：(time != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       81. 真分支：；仅条件为真执行以下子项。
 *         82. 调用 / 计算 / 写入：globalTimeout += time;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       83. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         84. 调用 / 计算 / 写入：globalTimeout = NCCL_SEARCH_GLOBAL_TIMEOUT;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     85. 条件判断：(globalTimeout < 0 && graph->nChannels)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       86. 真分支：；仅条件为真执行以下子项。
 *         87. 控制转移：goto done;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     88. 准备局部状态：int maxTypeIntra = system->nodes[NET].count > 0 ? tmpGraph.typeInter : PATH_SYS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     89. 条件判断：(tmpGraph.typeIntra < maxTypeIntra && (graph->nChannels == 0 || tmpGraph.typeIntra < graph->typeIntra))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       90. 真分支：；仅条件为真执行以下子项。
 *         91. 调用 / 计算 / 写入：tmpGraph.typeIntra += 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         92. 控制转移：goto search;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     93. 调用 / 计算 / 写入：tmpGraph.typeIntra = ngpus == 1 ? PATH_LOC : PATH_NVL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     94. 条件判断：(system->nodes[NET].count > 0 && tmpGraph.typeInter < PATH_SYS && (graph->nChannels == 0 || tmpGraph.typeInter < graph->typeInter || tmpGraph.typeInter < PATH_PXN))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       95. 真分支：；仅条件为真执行以下子项。
 *         96. 调用 / 计算 / 写入：tmpGraph.typeInter += 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         97. 控制转移：goto search;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     98. 调用 / 计算 / 写入：tmpGraph.typeInter = PATH_PIX;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     99. 条件判断：(crossNic && tmpGraph.crossNic == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       100. 真分支：；仅条件为真执行以下子项。
 *         101. 调用 / 计算 / 写入：tmpGraph.crossNic = crossNic;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         102. 控制转移：goto search;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     103. 调用 / 计算 / 写入：tmpGraph.crossNic = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     104. 条件判断：(tmpGraph.pattern == NCCL_TOPO_PATTERN_SPLIT_TREE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       105. 真分支：；仅条件为真执行以下子项。
 *         106. 调用 / 计算 / 写入：tmpGraph.pattern = NCCL_TOPO_PATTERN_TREE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         107. 控制转移：goto search;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     108. 调用 / 计算 / 写入：tmpGraph.pattern = graph->pattern;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     109. 条件判断：((speedIndex < nspeeds-1) && (graph->nChannels == 0 || (speedArray[speedIndex+1]/graph->speedInter > .49)))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       110. 真分支：；仅条件为真执行以下子项。
 *         111. 调用 / 计算 / 写入：tmpGraph.speedInter = tmpGraph.speedIntra = speedArray[++speedIndex];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         112. 控制转移：goto search;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     113. 调用 / 计算 / 写入：speedIndex = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     114. 循环：初始 ；继续条件 (speedArray[speedIndex] > system->maxWidth && speedIndex < nspeeds-1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       115. 调用 / 计算 / 写入：speedIndex++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     116. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     117. 调用 / 计算 / 写入：tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 118. 跳转标签：done；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   119. 条件判断：(pass == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     120. 真分支：；仅条件为真执行以下子项。
 *       121. 调用 / 计算 / 写入：time = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       122. 调用 / 计算 / 写入：memcpy(&tmpGraph, graph, sizeof(tmpGraph));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       123. 调用 / 计算 / 写入：speedIndex = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       124. 循环：初始 ；继续条件 (speedArray[speedIndex] > graph->speedInter && speedIndex < nspeeds-1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         125. 调用 / 计算 / 写入：speedIndex++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       126. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       127. 调用 / 计算 / 写入：tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       128. 调用 / 计算 / 写入：tmpGraph.minChannels = graph->nChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       129. 调用 / 计算 / 写入：pass = 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 130. 条件判断：(pass == 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   131. 真分支：；仅条件为真执行以下子项。
 *     132. 条件判断：(time != 0 && graph->pattern != NCCL_TOPO_PATTERN_RING && tmpGraph.speedIntra == graph->speedIntra && tmpGraph.speedIntra < tmpGraph.speedInter*2 && speedIndex > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       133. 真分支：；仅条件为真执行以下子项。
 *         134. 调用 / 计算 / 写入：tmpGraph.speedIntra = speedArray[--speedIndex];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         135. 控制转移：goto search;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     136. 调用 / 计算 / 写入：time = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     137. 调用 / 计算 / 写入：memcpy(&tmpGraph, graph, sizeof(tmpGraph));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 138. 条件判断：(graph->nChannels == 0 && graph->collNet == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   139. 真分支：；仅条件为真执行以下子项。
 *     140. 调用 / 计算 / 写入：WARN("Could not find a path for pattern %d, falling back to simple order", graph->pattern);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     141. 循环：初始 int i=0;；继续条件 i<ngpus；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       142. 调用 / 计算 / 写入：graph->intra[i] = system->nodes[GPU].nodes[i].gpu.rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     143. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     144. 调用 / 计算 / 写入：graph->inter[0] = graph->inter[1] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     145. 调用 / 计算 / 写入：graph->speedIntra = graph->speedInter = 0.1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     146. 调用 / 计算 / 写入：graph->typeIntra = graph->typeInter = PATH_SYS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     147. 调用 / 计算 / 写入：graph->nChannels = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 148. 条件判断：(graph->speedIntra >= 25.0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   149. 真分支：；仅条件为真执行以下子项。
 *     150. 准备局部状态：int dupChannels = std::min(graph->nChannels*2, graph->maxChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     151. 调用 / 计算 / 写入：memcpy(graph->intra+graph->nChannels*ngpus, graph->intra, (dupChannels-graph->nChannels)*ngpus*sizeof(int));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     152. 调用 / 计算 / 写入：memcpy(graph->inter+graph->nChannels*2,graph->inter, (dupChannels-graph->nChannels)*2*sizeof(int));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     153. 调用 / 计算 / 写入：graph->speedIntra /= DIVUP(dupChannels, graph->nChannels);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     154. 调用 / 计算 / 写入：graph->speedInter /= DIVUP(dupChannels, graph->nChannels);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     155. 调用 / 计算 / 写入：graph->nChannels = dupChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 156. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0489
 */
/* 中文逐项-函数 F0489：ncclTopoCompute
 * 用途与位置：在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。
 * 参数 system（ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：DIVUP；NCCLCHECK；free；getenv；memcpy；ncclCalloc；ncclParamCrossNic；ncclTopoGetCompCap；ncclTopoGetGraphFromXml；ncclTopoGetXmlGraphFromFile；ncclTopoSearchRec；std::min。
 * 需要对照的边界/推进条件：graph->crossNic == 2；str；graph->nChannels > 0；ngpus == 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoCompute(ncclTopoSystem* system, struct ncclTopoGraph* graph) {
/* 中文逐项-变量 V1437：ngpus
 * ngpus（int；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;
  graph->crossNic = ncclParamCrossNic();
/* 中文逐项-变量 V1438：crossNic
 * crossNic（int；函数 ncclTopoCompute 内）：图搜索是否允许进出不同NIC、是否重放相同机内GPU顺序的限制。
 * 初值/来源 crossNic = (system->nodes[NET].count > 1) && graph->crossNic ? 1 : 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int crossNic = (system->nodes[NET].count > 1) && graph->crossNic ? 1 : 0;
  graph->speedIntra = graph->speedInter = 0;
  graph->latencyInter = 0;
  if (graph->crossNic == 2) graph->crossNic = 0;
  graph->typeIntra = ngpus == 1 ? PATH_LOC : PATH_NVL;
  graph->typeInter = PATH_PIX;
  graph->nChannels = 0;
  graph->sameChannels = 1;

/* 中文逐项-变量 V1439：str
 * str（char；函数 ncclTopoCompute 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv("NCCL_GRAPH_FILE")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* str = getenv("NCCL_GRAPH_FILE");
  if (str) {
    INFO(NCCL_ENV, "NCCL_GRAPH_FILE set by environment to %s", str);
/* 中文逐项-变量 V1440：xml
 * xml（struct ncclXml；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&xml, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclXml* xml;
    NCCLCHECK(ncclCalloc(&xml, 1));
    NCCLCHECK(ncclTopoGetXmlGraphFromFile(str, xml));
/* 中文逐项-变量 V1441：nChannels
 * nChannels（int；函数 ncclTopoCompute 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetGraphFromXml(xml->nodes, system, graph,
 * &nChannels));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int nChannels;
    NCCLCHECK(ncclTopoGetGraphFromXml(xml->nodes, system, graph, &nChannels));
    INFO(NCCL_GRAPH, "Search %d : %d channels loaded from XML graph", graph->id, nChannels);
    free(xml);
    if (graph->nChannels > 0) return ncclSuccess;
  }

  if (ngpus == 1) if (graph->pattern != NCCL_TOPO_PATTERN_RING) graph->pattern = NCCL_TOPO_PATTERN_TREE;

  // SPLIT_TREE works better on older archs.
/* 中文逐项-变量 V1442：ccMin
 * ccMin（int；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetCompCap(system, &ccMin, NULL));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ccMin;
  NCCLCHECK(ncclTopoGetCompCap(system, &ccMin, NULL));

/* 中文逐项-变量 V1443：tmpGraph
 * tmpGraph（struct ncclTopoGraph；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memcpy(&tmpGraph, graph, sizeof(struct ncclTopoGraph));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoGraph tmpGraph;
  memcpy(&tmpGraph, graph, sizeof(struct ncclTopoGraph));

  // First try crossnic, then decrease speed and finally increase speedIntra.
/* 中文逐项-变量 V1444：nspeeds
 * nspeeds（int；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nspeeds = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int nspeeds = 0;
/* 中文逐项-变量 V1445：speedArray
 * speedArray（float；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 speedArray = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  float* speedArray = NULL;
  if (system->nodes[NET].count == 0) {
    nspeeds = NSPEEDSINTRA;
    speedArray = speedArrayIntra;
  } else {
    nspeeds = NSPEEDSINTER;
    speedArray = speedArrayInter;
  }
/* 中文逐项-变量 V1446：pass
 * pass（int；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pass = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int pass = 1;
/* 中文逐项-变量 V1447：speedIndex
 * speedIndex（int；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 speedIndex = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int speedIndex = 0;
  while (speedArray[speedIndex] > system->maxWidth && speedIndex < nspeeds-1) speedIndex++;
  tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];
/* 中文逐项-变量 V1448：globalTimeout
 * globalTimeout（int64_t；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 globalTimeout = NCCL_SEARCH_GLOBAL_TIMEOUT；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int64_t globalTimeout = NCCL_SEARCH_GLOBAL_TIMEOUT;

/* 中文精读：channel 数不是由数据大小直接生成的
 * 初始化时尚不知道未来用户张量大小；此处依据 GPU/NIC/链路容量搜索可用通信图。
 * 先尝试同样的通道路径，再放宽不同通道顺序、路径类型、crossNic 或速度，遇到预算限制退出。
 * 选出图后可能复制高速通道来增加并行 block，最终 Postset 还会继续调整。
 * 真正收到一条消息时，getAlgoInfo 才按 nBytes 缩减本次 channel/线程数。
 */
search:
/* 中文逐项-变量 V1449：time
 * time（int；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 time = tmpGraph.sameChannels ? NCCL_SEARCH_TIMEOUT_SAMECHANNELS : tmpGraph.pattern == NCCL_TOPO_PATTERN_TREE
 * ? NCCL_SEARCH_TIMEOUT_TREE : NCCL_SEARCH_TIMEOUT；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int time = tmpGraph.sameChannels ? NCCL_SEARCH_TIMEOUT_SAMECHANNELS :
    tmpGraph.pattern == NCCL_TOPO_PATTERN_TREE ? NCCL_SEARCH_TIMEOUT_TREE : NCCL_SEARCH_TIMEOUT;
  tmpGraph.nChannels = 0;
  globalTimeout -= time;

  NCCLCHECK(ncclTopoSearchRec(system, &tmpGraph, graph, &time));
#if 0
  printf("Pattern %d, crossNic %d, Speed %g/%g, type %d/%d, channels %d-%d sameChannels %d -> nChannels %dx%g/%g %s\n", tmpGraph.pattern, tmpGraph.crossNic, tmpGraph.speedInter, tmpGraph.speedIntra, tmpGraph.typeInter, tmpGraph.typeIntra, tmpGraph.minChannels, tmpGraph.maxChannels, tmpGraph.sameChannels, graph->nChannels, graph->speedInter, graph->speedIntra, time == 0 ? "TIMEOUT" : time == -1 ? "PERFECT" : "");
  for (/* 中文逐项-变量 V1450：c
 * c（int；函数 ncclTopoCompute 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<graph->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<graph->nChannels; c++) {
    printf("%2d : ", c);
    for (/* 中文逐项-变量 V1451：g
 * g（int；函数 ncclTopoCompute 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<ngpus；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<ngpus; g++) {
      printf("%d ", graph->intra[c*ngpus+g]);
    }
    printf("[%d %d]", graph->inter[c*2+0], graph->inter[c*2+1]);
    printf("\n");
  }
#endif
  // Optimal solution, stop here
  if (time == -1) goto done;
  if (graph->nChannels*graph->speedInter >= system->totalWidth) goto done;

  if (pass == 1) {
    // First pass, we don't have a solution yet ; try other options

    // Try having different channels
    if (tmpGraph.sameChannels == 1) {
      tmpGraph.sameChannels = 0;
      goto search;
    }
    tmpGraph.sameChannels = 1;

    if (time != -1) globalTimeout += time;
    else globalTimeout = NCCL_SEARCH_GLOBAL_TIMEOUT;
    if (globalTimeout < 0 && graph->nChannels) goto done;

/* 中文逐项-变量 V1452：maxTypeIntra
 * maxTypeIntra（int；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxTypeIntra = system->nodes[NET].count > 0 ? tmpGraph.typeInter : PATH_SYS；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int maxTypeIntra = system->nodes[NET].count > 0 ? tmpGraph.typeInter : PATH_SYS;
    if (tmpGraph.typeIntra < maxTypeIntra && (graph->nChannels == 0 || tmpGraph.typeIntra < graph->typeIntra)) {
      tmpGraph.typeIntra += 1;
      goto search;
    }
    tmpGraph.typeIntra = ngpus == 1 ? PATH_LOC : PATH_NVL;

    if (system->nodes[NET].count > 0 && tmpGraph.typeInter < PATH_SYS && (graph->nChannels == 0 || tmpGraph.typeInter < graph->typeInter || tmpGraph.typeInter < PATH_PXN)) {
      tmpGraph.typeInter += 1;
      goto search;
    }
    tmpGraph.typeInter = PATH_PIX;

    if (crossNic && tmpGraph.crossNic == 0) {
      // Try again with crossNic if permitted
      tmpGraph.crossNic = crossNic;
      goto search;
    }
    tmpGraph.crossNic = 0;

    // Try a simpler tree
    if (tmpGraph.pattern == NCCL_TOPO_PATTERN_SPLIT_TREE) {
      tmpGraph.pattern = NCCL_TOPO_PATTERN_TREE;
      goto search;
    }
    tmpGraph.pattern = graph->pattern;

    // Decrease speed until we find a solution
    if ((speedIndex < nspeeds-1) && (graph->nChannels == 0 || (speedArray[speedIndex+1]/graph->speedInter > .49))) {
      tmpGraph.speedInter = tmpGraph.speedIntra = speedArray[++speedIndex];
      goto search;
    }
    speedIndex = 0;
    while (speedArray[speedIndex] > system->maxWidth && speedIndex < nspeeds-1) speedIndex++;
    tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];

  }

done:
  // We have a solution. Start from that solution and move to pass 2.
  if (pass == 1) {
    time = -1;
    memcpy(&tmpGraph, graph, sizeof(tmpGraph));
    speedIndex = 0;
    while (speedArray[speedIndex] > graph->speedInter && speedIndex < nspeeds-1) speedIndex++;
    tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];
    tmpGraph.minChannels = graph->nChannels;
    pass = 2;
  }

  // 3. See if we can increase speedIntra for trees (2 nodes or collnet)
  if (pass == 2) {
    if (time != 0 && graph->pattern != NCCL_TOPO_PATTERN_RING &&
        tmpGraph.speedIntra == graph->speedIntra && tmpGraph.speedIntra < tmpGraph.speedInter*2 &&
        speedIndex > 0) {
      tmpGraph.speedIntra = speedArray[--speedIndex];
      goto search;
    }
    time = -1;
    memcpy(&tmpGraph, graph, sizeof(tmpGraph));
  }

  if (graph->nChannels == 0 && graph->collNet == 0) {
    WARN("Could not find a path for pattern %d, falling back to simple order", graph->pattern);
    for (/* 中文逐项-变量 V1453：i
 * i（int；函数 ncclTopoCompute 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ngpus；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ngpus; i++) graph->intra[i] = system->nodes[GPU].nodes[i].gpu.rank;
    graph->inter[0] = graph->inter[1] = 0;
    graph->speedIntra = graph->speedInter = 0.1;
    graph->typeIntra = graph->typeInter = PATH_SYS;
    graph->nChannels = 1;
  }

  if (graph->speedIntra >= 25.0) {
/* 中文逐项-变量 V1454：dupChannels
 * dupChannels（int；函数 ncclTopoCompute 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 dupChannels = std::min(graph->nChannels*2, graph->maxChannels)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
    int dupChannels = std::min(graph->nChannels*2, graph->maxChannels);
    memcpy(graph->intra+graph->nChannels*ngpus, graph->intra, (dupChannels-graph->nChannels)*ngpus*sizeof(int));
    memcpy(graph->inter+graph->nChannels*2,graph->inter, (dupChannels-graph->nChannels)*2*sizeof(int));
    graph->speedIntra /= DIVUP(dupChannels, graph->nChannels);
    graph->speedInter /= DIVUP(dupChannels, graph->nChannels);
    graph->nChannels = dupChannels;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0490：ncclTopoPrintGraph
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Pattern %d, crossNic %d, nChannels %d, speed %f/%f, type %s/%s, sameChannels %d", graph->pattern, graph->crossNic, graph->nChannels, graph->speedIntra, graph->speedInter, topoPathTyp …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：char line[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 int c=0;；继续条件 c<graph->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：sprintf(line, "%2d :", c);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   6. 准备局部状态：int offset = strlen(line);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   7. 条件判断：(system->nodes[NET].count > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     8. 真分支：；仅条件为真执行以下子项。
 *       9. 调用 / 计算 / 写入：sprintf(line+offset, " %s/%d", topoNodeTypeStr[NET], graph->inter[2*c]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       10. 调用 / 计算 / 写入：offset = strlen(line);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   11. 循环：初始 int i=0;；继续条件 i<ngpus；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     12. 调用 / 计算 / 写入：sprintf(line+offset, " %s/%d", topoNodeTypeStr[GPU], graph->intra[ngpus*c+i]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     13. 调用 / 计算 / 写入：offset = strlen(line);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   15. 条件判断：(system->nodes[NET].count > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     16. 真分支：；仅条件为真执行以下子项。
 *       17. 调用 / 计算 / 写入：sprintf(line+offset, " %s/%d", topoNodeTypeStr[NET], graph->inter[2*c+1]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       18. 调用 / 计算 / 写入：offset = strlen(line);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   19. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 21. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0490
 */
/* 中文逐项-函数 F0490：ncclTopoPrintGraph
 * 用途与位置：输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：sprintf；strlen。
 * 需要对照的边界/推进条件：c<graph->nChannels；system->nodes[NET].count > 0；i<ngpus；system->nodes[NET].count > 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoPrintGraph(struct ncclTopoSystem* system, struct ncclTopoGraph* graph) {
  INFO(NCCL_GRAPH, "Pattern %d, crossNic %d, nChannels %d, speed %f/%f, type %s/%s, sameChannels %d", graph->pattern, graph->crossNic, graph->nChannels, graph->speedIntra, graph->speedInter, topoPathTypeStr[graph->typeIntra], topoPathTypeStr[graph->typeInter], graph->sameChannels);
/* 中文逐项-变量 V1455：ngpus
 * ngpus（int；函数 ncclTopoPrintGraph 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;

/* 中文逐项-变量 V1456：line
 * line（char；函数 ncclTopoPrintGraph 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：sprintf(line, "%2d :", c);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[1024];
  for (/* 中文逐项-变量 V1457：c
 * c（int；函数 ncclTopoPrintGraph 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<graph->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<graph->nChannels; c++) {
    sprintf(line, "%2d :", c);
/* 中文逐项-变量 V1458：offset
 * offset（int；函数 ncclTopoPrintGraph 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = strlen(line)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int offset = strlen(line);
    if (system->nodes[NET].count > 0) {
      sprintf(line+offset, " %s/%d", topoNodeTypeStr[NET], graph->inter[2*c]);
      offset = strlen(line);
    }
    for (/* 中文逐项-变量 V1459：i
 * i（int；函数 ncclTopoPrintGraph 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ngpus；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ngpus; i++) {
      sprintf(line+offset, " %s/%d", topoNodeTypeStr[GPU], graph->intra[ngpus*c+i]);
      offset = strlen(line);
    }
    if (system->nodes[NET].count > 0) {
      sprintf(line+offset, " %s/%d", topoNodeTypeStr[NET], graph->inter[2*c+1]);
      offset = strlen(line);
    }
    INFO(NCCL_GRAPH, "%s", line);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0491：ncclTopoDumpGraphs
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char* str = getenv("NCCL_GRAPH_DUMP_FILE");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(str)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_GRAPH_DUMP_FILE set by environment to %s", str);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 准备局部状态：struct ncclXml* xml;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&xml, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromGraphs(ngraphs, graphs, system, xml));；失败时可能提前return，成功才继续；创建 XML 根并导出多种算法的通信图，保持模式/id 信息。
 *     8. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoDumpXmlToFile(str, xml));；失败时可能提前return，成功才继续；递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。
 *     9. 调用 / 计算 / 写入：free(xml);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0491
 */
/* 中文逐项-函数 F0491：ncclTopoDumpGraphs
 * 用途与位置：输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 ngraphs（int ngraphs）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 graphs（struct ncclTopoGraph** graphs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；free；getenv；ncclCalloc；ncclTopoDumpXmlToFile；ncclTopoGetXmlFromGraphs。
 * 需要对照的边界/推进条件：str。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoDumpGraphs(struct ncclTopoSystem* system, int ngraphs, struct ncclTopoGraph** graphs) {
/* 中文逐项-变量 V1460：str
 * str（char；函数 ncclTopoDumpGraphs 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv("NCCL_GRAPH_DUMP_FILE")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* str = getenv("NCCL_GRAPH_DUMP_FILE");
  if (str) {
    INFO(NCCL_ENV, "NCCL_GRAPH_DUMP_FILE set by environment to %s", str);
/* 中文逐项-变量 V1461：xml
 * xml（struct ncclXml；函数 ncclTopoDumpGraphs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&xml, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclXml* xml;
    NCCLCHECK(ncclCalloc(&xml, 1));
    NCCLCHECK(ncclTopoGetXmlFromGraphs(ngraphs, graphs, system, xml));
    NCCLCHECK(ncclTopoDumpXmlToFile(str, xml));
    free(xml);
  }
  return ncclSuccess;
}

// 0: don't use PXN for P2P, 1: use PXN if needed, 2: use PXN as much as possible to maximize aggregation
/* 中文逐项-参数getter P0015：P2pPxnLevel
 * 生成函数 ncclParamP2pPxnLevel()，读取 NCCL_P2P_PXN_LEVEL，编译默认值为 2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(P2pPxnLevel, "P2P_PXN_LEVEL", 2);

#include "comm.h"
/* 中文导读：
 * 为本次 channel/peer 选择网络设备及发送代理 rank。
 * 有通信图时遵循图中的端点；无图的 P2P 路径还要考虑本地亲和性/PXN，rank 与实际出口不一定一一对应。
 */
/* 中文内部逻辑 F0492：ncclTopoGetNetDev
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(graph)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 准备局部状态：int channel = channelId%graph->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     4. 准备局部状态：int ngpus = comm->topo->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 准备局部状态：int index = graph->intra[channel*ngpus] == rank ? 0 : 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 调用 / 计算 / 写入：*dev = graph->inter[channel*2+index];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetIntermediateRank(comm->topo, rank, *dev, proxyRank));；失败时可能提前return，成功才继续；找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。
 *   8. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     9. 条件判断：(peerRank == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       10. 真分支：；仅条件为真执行以下子项。
 *         11. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *       12. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         13. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetLocalNet(comm->topo, rank, dev));；失败时可能提前return，成功才继续；按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。
 *         14. 调用 / 计算 / 写入：*proxyRank = rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         15. 准备局部状态：int pxnLevel = ncclPxnDisable() == 1 ? 0 : ncclParamP2pPxnLevel();；汇总参数和环境条件判断是否禁止 PXN，避免选择当前后端不支持的中间 GPU 网络路径。
 *         16. 条件判断：(ncclParamCrossNic() == 0 || (pxnLevel != 0))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           17. 真分支：；仅条件为真执行以下子项。
 *             18. 准备局部状态：int netDev = comm->peerInfo[peerRank].netDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             19. 准备局部状态：int n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             20. 条件判断：(ncclParamCrossNic() == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               21. 真分支：；仅条件为真执行以下子项。
 *                 22. 条件判断：(ncclTopoIdToIndex(comm->topo, NET, netDev, &n) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   23. 真分支：；仅条件为真执行以下子项。
 *                     24. 调用 / 计算 / 写入：WARN("Rank %d requires NIC %d but that NIC is not available for rank %d", peerRank, netDev, rank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                     25. 函数返回：return ncclInvalidUsage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *                 26. 调用 / 计算 / 写入：*dev = netDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             27. 条件判断：(pxnLevel == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               28. 真分支：；仅条件为真执行以下子项。
 *                 29. 准备局部状态：int g, n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 30. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoRankToIndex(comm->topo, rank, &g));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 *                 31. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoIdToIndex(comm->topo, NET, netDev, &n));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 *                 32. 准备局部状态：struct ncclTopoNode* gpu = comm->topo->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 33. 条件判断：(gpu->paths[NET][n].type <= PATH_PXN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   34. 真分支：；仅条件为真执行以下子项。
 *                     35. 调用 / 计算 / 写入：*dev = netDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                     36. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetIntermediateRank(comm->topo, rank, *dev, proxyRank));；失败时可能提前return，成功才继续；找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。
 *               37. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                 38. 条件判断：(pxnLevel == 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   39. 真分支：；仅条件为真执行以下子项。
 *                     40. 循环：初始 int r=0;；继续条件 r<comm->localRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                       41. 准备局部状态：int peerRank = comm->localRankToRank[r];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                       42. 条件判断：(comm->peerInfo[peerRank].netDev == netDev)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                         43. 真分支：；仅条件为真执行以下子项。
 *                           44. 准备局部状态：int g1, g2, n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                           45. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoRankToIndex(comm->topo, rank, &g1));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 *                           46. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoRankToIndex(comm->topo, peerRank, &g2));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 *                           47. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoIdToIndex(comm->topo, NET, netDev, &n));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 *                           48. 准备局部状态：struct ncclTopoNode* peerGpu = comm->topo->nodes[GPU].nodes+g2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                           49. 条件判断：(peerGpu->paths[GPU][g1].type <= PATH_NVL && peerGpu->paths[NET][n].type <= PATH_PXB)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                             50. 真分支：；仅条件为真执行以下子项。
 *                               51. 调用 / 计算 / 写入：*proxyRank = peerRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                               52. 调用 / 计算 / 写入：*dev = netDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                               53. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *                     54. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 55. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__search_cc.md#f0492
 */
/* 中文逐项-函数 F0492：ncclTopoGetNetDev
 * 用途与位置：按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 peerRank（int peerRank）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 dev（int* dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 proxyRank（int* proxyRank）：实际负责网络出口的代理 rank，PXN 时可能与发起通信的 GPU rank 不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoGetPxnRanks；src/transport/coll_net.cc::sendSetup；src/transport/coll_net.cc::recvSetup；src/transport/net.cc::sendSetup；src/transport/net.cc::recvSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclParamCrossNic；ncclParamP2pPxnLevel；ncclPxnDisable；ncclTopoGetIntermediateRank；ncclTopoGetLocalNet；ncclTopoIdToIndex；ncclTopoRankToIndex。
 * 需要对照的边界/推进条件：graph；peerRank == -1；ncclParamCrossNic() == 0 || (pxnLevel != 0；ncclParamCrossNic() ==
 * 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclInvalidUsage;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetNetDev(struct ncclComm* comm, int rank, struct ncclTopoGraph* graph, int channelId, int peerRank, int* dev, int* proxyRank) {
  if (graph) {
    // Honor the net device in the graph
/* 中文逐项-变量 V1462：channel
 * channel（int；函数 ncclTopoGetNetDev 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = channelId%graph->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int channel = channelId%graph->nChannels;
/* 中文逐项-变量 V1463：ngpus
 * ngpus（int；函数 ncclTopoGetNetDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = comm->topo->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int ngpus = comm->topo->nodes[GPU].count;
/* 中文逐项-变量 V1464：index
 * index（int；函数 ncclTopoGetNetDev 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 初值/来源 index = graph->intra[channel*ngpus] == rank ? 0 : 1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int index = graph->intra[channel*ngpus] == rank ? 0 : 1;
    *dev = graph->inter[channel*2+index];
    NCCLCHECK(ncclTopoGetIntermediateRank(comm->topo, rank, *dev, proxyRank));
  } else if (peerRank == -1) {
    return ncclInternalError;
  } else {
    // Start with our local NIC and local Rank
    NCCLCHECK(ncclTopoGetLocalNet(comm->topo, rank, dev));
    *proxyRank = rank;

/* 中文逐项-变量 V1465：pxnLevel
 * pxnLevel（int；函数 ncclTopoGetNetDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pxnLevel = ncclPxnDisable() == 1 ? 0 : ncclParamP2pPxnLevel()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
    int pxnLevel = ncclPxnDisable() == 1 ? 0 : ncclParamP2pPxnLevel();
    // See whether we can use the remote rank preferred device.
    if (ncclParamCrossNic() == 0 || (pxnLevel != 0)) {
/* 中文逐项-变量 V1466：netDev
 * netDev（int；函数 ncclTopoGetNetDev 内）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 初值/来源 netDev = comm->peerInfo[peerRank].netDev；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int netDev = comm->peerInfo[peerRank].netDev;
/* 中文逐项-变量 V1467：n
 * n（int；函数 ncclTopoGetNetDev 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 此处未显式给初值；后续首个文本使用点：if (ncclTopoIdToIndex(comm->topo, NET, netDev, &n) != ncclSuccess) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int n;
      // Check that device exists on our node
      if (ncclParamCrossNic() == 0) {
        if (ncclTopoIdToIndex(comm->topo, NET, netDev, &n) != ncclSuccess) {
          WARN("Rank %d requires NIC %d but that NIC is not available for rank %d", peerRank, netDev, rank);
          return ncclInvalidUsage;
        }
        *dev = netDev;
      }
      if (pxnLevel == 1) {
/* 中文逐项-变量 V1468：g、n
 * g（int；函数 ncclTopoGetNetDev 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoRankToIndex(comm->topo, rank, &g));。若由输出参数填入，必须先检查填充调用成功再读值。
 * n（int；函数 ncclTopoGetNetDev 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoIdToIndex(comm->topo, NET, netDev, &n));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int g, n;
        NCCLCHECK(ncclTopoRankToIndex(comm->topo, rank, &g));
        NCCLCHECK(ncclTopoIdToIndex(comm->topo, NET, netDev, &n));
/* 中文逐项-变量 V1469：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoGetNetDev 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = comm->topo->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclTopoNode* gpu = comm->topo->nodes[GPU].nodes+g;
        if (gpu->paths[NET][n].type <= PATH_PXN) {
          *dev = netDev;
          NCCLCHECK(ncclTopoGetIntermediateRank(comm->topo, rank, *dev, proxyRank));
        }
      } else if (pxnLevel == 2) {
        // Check whether we can access it through our node-local GPU for that NIC.
        for (/* 中文逐项-变量 V1470：r
 * r（int；函数 ncclTopoGetNetDev 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->localRanks; r++) {
/* 中文逐项-变量 V1471：peerRank
 * peerRank（int；函数 ncclTopoGetNetDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 peerRank = comm->localRankToRank[r]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          int peerRank = comm->localRankToRank[r];
          if (comm->peerInfo[peerRank].netDev == netDev) {
/* 中文逐项-变量 V1472：g1、g2、n
 * g1（int；函数 ncclTopoGetNetDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoRankToIndex(comm->topo, rank, &g1));。若由输出参数填入，必须先检查填充调用成功再读值。
 * g2（int；函数 ncclTopoGetNetDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoRankToIndex(comm->topo, peerRank, &g2));。若由输出参数填入，必须先检查填充调用成功再读值。
 * n（int；函数 ncclTopoGetNetDev 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoIdToIndex(comm->topo, NET, netDev, &n));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            int g1, g2, n;
            NCCLCHECK(ncclTopoRankToIndex(comm->topo, rank, &g1));
            NCCLCHECK(ncclTopoRankToIndex(comm->topo, peerRank, &g2));
            NCCLCHECK(ncclTopoIdToIndex(comm->topo, NET, netDev, &n));
/* 中文逐项-变量 V1473：peerGpu
 * peerGpu（struct ncclTopoNode；函数 ncclTopoGetNetDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 peerGpu = comm->topo->nodes[GPU].nodes+g2；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            struct ncclTopoNode* peerGpu = comm->topo->nodes[GPU].nodes+g2;
            if (peerGpu->paths[GPU][g1].type <= PATH_NVL && peerGpu->paths[NET][n].type <= PATH_PXB) {
              *proxyRank = peerRank;
              *dev = netDev;
              return ncclSuccess;
            }
          }
        }
      }
    }
  }
  return ncclSuccess;
}
