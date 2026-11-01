/*************************************************************************
 * Copyright (c) 2018-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "core.h"
#include "graph.h"
#include "topo.h"
#include "comm.h"
#include "net.h"
#include "channel.h"

// Pre-compute GPU->NIC, GPU->GPU and NIC->GPU paths

/* 中文逐项-类型 T0135：ncclTopoNodeList
 * 类型职责：广度优先路径搜索的一层待扩展节点队列，层间切换保证按跳数逐层遍历。
 * 声明内可见成员（含嵌套成员）：list、count。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTopoNodeList {
/* 中文逐项-变量 V1139：list
 * list（struct ncclTopoNode；成员 ncclTopoNodeList）：节点或链路指针列表，count给出有效条目数，遍历不得越过当前有效范围。
 */
  struct ncclTopoNode* list[NCCL_TOPO_MAX_NODES];
/* 中文逐项-变量 V1140：count
 * count（int；成员 ncclTopoNodeList）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 */
  int count;
};

/* 中文内部逻辑 F0440：getPath
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int i=0;；继续条件 i<system->nodes[t].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 条件判断：(system->nodes[t].nodes[i].id == id)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     3. 真分支：；仅条件为真执行以下子项。
 *       4. 调用 / 计算 / 写入：*path = node->paths[t]+i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 调用 / 计算 / 写入：WARN("Could not find node of type %d id %lx", t, id);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0440
 */
/* 中文逐项-函数 F0440：getPath
 * 用途与位置：从指定源节点的 paths 缓存中找到目标硬件类型/id 对应的路径项。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 node（struct ncclTopoNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 t（int t）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 id（int64_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 path（struct ncclTopoLinkList** path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoSetPaths。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<system->nodes[t].count；system->nodes[t].nodes[i].id == id。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getPath(struct ncclTopoSystem* system, struct ncclTopoNode* node, int t, int64_t id, struct ncclTopoLinkList** path) {
  for (/* 中文逐项-变量 V1141：i
 * i（int；函数 getPath 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[t].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[t].count; i++) {
    if (system->nodes[t].nodes[i].id == id) {
      *path = node->paths[t]+i;
      return ncclSuccess;
    }
  }
  WARN("Could not find node of type %d id %lx", t, id);
  return ncclInternalError;
}

/* 中文逐项-参数getter P0007：NvbDisable
 * 生成函数 ncclParamNvbDisable()，读取 NCCL_NVB_DISABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(NvbDisable, "NVB_DISABLE", 0);

/* 中文精读：硬件图上的路径预计算
 * 从 baseNode 出发按层扩展可达节点，记录路径链路、跳数、瓶颈带宽与路径类别。
 * 带宽由沿途最窄边限制；路径类别反映 NVLink/PCIe/跨 CPU 等代价，不只是最少跳数。
 * 随后 P2P/GDR 检查还会按实际访问能力改写/绕行路径，因此物理可达不等于可用 CUDA P2P。
 * 这些路径是 Search 的输入；它们不是用户张量中 chunk 的路径描述。
 */
/* 中文内部逻辑 F0441：ncclTopoSetPaths
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(baseNode->paths[baseNode->type] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(baseNode->paths+baseNode->type, system->nodes[baseNode->type].count));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 准备局部状态：struct ncclTopoNodeList nodeList;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：struct ncclTopoNodeList nextNodeList;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：nodeList.count = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：nodeList.list[0] = baseNode;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：nextNodeList.count = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 准备局部状态：struct ncclTopoLinkList* basePath;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 调用 / 计算 / 写入：NCCLCHECK(getPath(system, baseNode, baseNode->type, baseNode->id, &basePath));；失败时可能提前return，成功才继续；从指定源节点的 paths 缓存中找到目标硬件类型/id 对应的路径项。
 * 11. 调用 / 计算 / 写入：basePath->count = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：basePath->width = LOC_WIDTH;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：basePath->type = PATH_LOC;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 循环：初始 ；继续条件 (nodeList.count)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   15. 调用 / 计算 / 写入：nextNodeList.count = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   16. 循环：初始 int n=0;；继续条件 n<nodeList.count；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     17. 准备局部状态：struct ncclTopoNode* node = nodeList.list[n];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     18. 准备局部状态：struct ncclTopoLinkList* path;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     19. 调用 / 计算 / 写入：NCCLCHECK(getPath(system, node, baseNode->type, baseNode->id, &path));；失败时可能提前return，成功才继续；从指定源节点的 paths 缓存中找到目标硬件类型/id 对应的路径项。
 *     20. 循环：初始 int l=0;；继续条件 l<node->nlinks；每轮更新 l++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       21. 准备局部状态：struct ncclTopoLink* link = node->links+l;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       22. 准备局部状态：struct ncclTopoNode* remNode = link->remNode;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       23. 条件判断：(remNode->paths[baseNode->type] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         24. 真分支：；仅条件为真执行以下子项。
 *           25. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(remNode->paths+baseNode->type, system->nodes[baseNode->type].count));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *       26. 准备局部状态：struct ncclTopoLinkList* remPath;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       27. 调用 / 计算 / 写入：NCCLCHECK(getPath(system, remNode, baseNode->type, baseNode->id, &remPath));；失败时可能提前return，成功才继续；从指定源节点的 paths 缓存中找到目标硬件类型/id 对应的路径项。
 *       28. 准备局部状态：float width = std::min(path->width, link->width);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       29. 条件判断：(node != baseNode && node->type == GPU && (ncclParamNvbDisable() || link->type != LINK_NVL || remNode->type != GPU || path->count > 1))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         30. 真分支：；仅条件为真执行以下子项。
 *           31. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       32. 条件判断：((remPath->width == 0 || remPath->count > path->count) && remPath->width < width)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         33. 真分支：；仅条件为真执行以下子项。
 *           34. 循环：初始 int l=0;；继续条件 l<remNode->nlinks；每轮更新 l++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *             35. 条件判断：(remNode->links[l].remNode == node)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               36. 真分支：；仅条件为真执行以下子项。
 *                 37. 调用 / 计算 / 写入：remPath->list[0] = remNode->links+l;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 38. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *           39. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           40. 条件判断：(remPath->list[0] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             41. 真分支：；仅条件为真执行以下子项。
 *               42. 调用 / 计算 / 写入：WARN("Failed to find reverse path from remNode %d/%lx nlinks %d to node %d/%lx", remNode->type, remNode->id, remNode->nlinks, node->type, node->id);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *               43. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *           44. 循环：初始 int i=0;；继续条件 i<path->count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *             45. 调用 / 计算 / 写入：remPath->list[i+1] = path->list[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           46. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           47. 调用 / 计算 / 写入：remPath->count = path->count + 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           48. 调用 / 计算 / 写入：remPath->width = width;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           49. 准备局部状态：int type = link->type == LINK_NET ? LINK_LOC : link->type;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           50. 条件判断：(node->type == PCI && remNode->type == PCI)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             51. 真分支：；仅条件为真执行以下子项。
 *               52. 调用 / 计算 / 写入：type = PATH_PXB;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           53. 条件判断：(link->type == LINK_PCI && (node->type == CPU || link->remNode->type == CPU))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             54. 真分支：；仅条件为真执行以下子项。
 *               55. 调用 / 计算 / 写入：type = PATH_PHB;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           56. 条件判断：(node->type == GPU && path->type == PATH_NVL && type == PATH_NVL && remPath->count > 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             57. 真分支：；仅条件为真执行以下子项。
 *               58. 调用 / 计算 / 写入：type = PATH_NVB;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           59. 调用 / 计算 / 写入：remPath->type = std::max(path->type, type);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           60. 准备局部状态：int i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           61. 循环：初始 i=0；继续条件 i<nextNodeList.count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *             62. 条件判断：(nextNodeList.list[i] == remNode)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               63. 真分支：；仅条件为真执行以下子项。
 *                 64. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *           65. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           66. 条件判断：(i == nextNodeList.count)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             67. 真分支：；仅条件为真执行以下子项。
 *               68. 调用 / 计算 / 写入：nextNodeList.list[nextNodeList.count++] = remNode;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     69. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   70. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   71. 调用 / 计算 / 写入：memcpy(&nodeList, &nextNodeList, sizeof(nodeList));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 72. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 73. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0441
 */
/* 中文逐项-函数 F0441：ncclTopoSetPaths
 * 用途与位置：从硬件节点做带带宽与路径类型约束的广度优先扩展，建立到该节点的路径缓存。
 * 参数 baseNode（struct ncclTopoNode* baseNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputePaths。
 * 本体直接出现的调用/包装：NCCLCHECK；getPath；memcpy；ncclCalloc；ncclParamNvbDisable；std::max；std::min。
 * 需要对照的边界/推进条件：baseNode->paths[baseNode->type] == NULL；nodeList.count；n<nodeList.count；l<node->nlinks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclTopoSetPaths(struct ncclTopoNode* baseNode, struct ncclTopoSystem* system) {
  if (baseNode->paths[baseNode->type] == NULL) {
    NCCLCHECK(ncclCalloc(baseNode->paths+baseNode->type, system->nodes[baseNode->type].count));
  }

  // breadth-first search to set all paths to that node in the system
/* 中文逐项-变量 V1142：nodeList
 * nodeList（struct ncclTopoNodeList；函数 ncclTopoSetPaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：nodeList.count = 1; nodeList.list[0] = baseNode;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoNodeList nodeList;
/* 中文逐项-变量 V1143：nextNodeList
 * nextNodeList（struct ncclTopoNodeList；函数 ncclTopoSetPaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：nextNodeList.count = 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoNodeList nextNodeList;
  nodeList.count = 1; nodeList.list[0] = baseNode;
  nextNodeList.count = 0;
/* 中文逐项-变量 V1144：basePath
 * basePath（struct ncclTopoLinkList；函数 ncclTopoSetPaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getPath(system, baseNode, baseNode->type, baseNode->id,
 * &basePath));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoLinkList* basePath;
  NCCLCHECK(getPath(system, baseNode, baseNode->type, baseNode->id, &basePath));
  basePath->count = 0;
  basePath->width = LOC_WIDTH;
  basePath->type = PATH_LOC;

  while (nodeList.count) {
    nextNodeList.count = 0;
    for (/* 中文逐项-变量 V1145：n
 * n（int；函数 ncclTopoSetPaths 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<nodeList.count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<nodeList.count; n++) {
/* 中文逐项-变量 V1146：node
 * node（struct ncclTopoNode；函数 ncclTopoSetPaths 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = nodeList.list[n]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoNode* node = nodeList.list[n];
/* 中文逐项-变量 V1147：path
 * path（struct ncclTopoLinkList；函数 ncclTopoSetPaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getPath(system, node, baseNode->type, baseNode->id, &path));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      struct ncclTopoLinkList* path;
      NCCLCHECK(getPath(system, node, baseNode->type, baseNode->id, &path));
      for (/* 中文逐项-变量 V1148：l
 * l（int；函数 ncclTopoSetPaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<node->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<node->nlinks; l++) {
/* 中文逐项-变量 V1149：link
 * link（struct ncclTopoLink；函数 ncclTopoSetPaths 内）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 初值/来源 link = node->links+l；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclTopoLink* link = node->links+l;
/* 中文逐项-变量 V1150：remNode
 * remNode（struct ncclTopoNode；函数 ncclTopoSetPaths 内）：硬件边指向的另一个节点，路径搜索沿此指针扩展。
 * 初值/来源 remNode = link->remNode；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclTopoNode* remNode = link->remNode;
        if (remNode->paths[baseNode->type] == NULL) {
          NCCLCHECK(ncclCalloc(remNode->paths+baseNode->type, system->nodes[baseNode->type].count));
        }
/* 中文逐项-变量 V1151：remPath
 * remPath（struct ncclTopoLinkList；函数 ncclTopoSetPaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getPath(system, remNode, baseNode->type, baseNode->id,
 * &remPath));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        struct ncclTopoLinkList* remPath;
        NCCLCHECK(getPath(system, remNode, baseNode->type, baseNode->id, &remPath));
/* 中文逐项-变量 V1152：width
 * width（float；函数 ncclTopoSetPaths 内）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 初值/来源 width = std::min(path->width, link->width)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
        float width = std::min(path->width, link->width);

        // allow routing through a GPU only as 1 hop
        if (node != baseNode && node->type == GPU &&
            (ncclParamNvbDisable() || link->type != LINK_NVL || remNode->type != GPU || path->count > 1)) continue;

        if ((remPath->width == 0 || remPath->count > path->count) && remPath->width < width) {
          // Find reverse link
          for (/* 中文逐项-变量 V1153：l
 * l（int；函数 ncclTopoSetPaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<remNode->nlinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<remNode->nlinks; l++) {
            if (remNode->links[l].remNode == node) {
              remPath->list[0] = remNode->links+l;
              break;
            }
          }
          if (remPath->list[0] == NULL) {
            WARN("Failed to find reverse path from remNode %d/%lx nlinks %d to node %d/%lx",
                 remNode->type, remNode->id, remNode->nlinks, node->type, node->id);
            return ncclInternalError;
          }
          // Copy the rest of the path
          for (/* 中文逐项-变量 V1154：i
 * i（int；函数 ncclTopoSetPaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<path->count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<path->count; i++) remPath->list[i+1] = path->list[i];
          remPath->count = path->count + 1;
          remPath->width = width;

          // Start with path type = link type. PATH and LINK types are supposed to match.
          // Don't consider LINK_NET as we only care about the NIC->GPU path.
/* 中文逐项-变量 V1155：type
 * type（int；函数 ncclTopoSetPaths 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 初值/来源 type = link->type == LINK_NET ? LINK_LOC : link->type；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          int type = link->type == LINK_NET ? LINK_LOC : link->type;
          // Differentiate between one and multiple PCI switches
          if (node->type == PCI && remNode->type == PCI) type = PATH_PXB;
          // Consider a path going through the CPU as PATH_PHB
          if (link->type == LINK_PCI && (node->type == CPU || link->remNode->type == CPU)) type = PATH_PHB;
          // Set 1 hop NVLink as NVB
          if (node->type == GPU && path->type == PATH_NVL && type == PATH_NVL && remPath->count > 1) type = PATH_NVB;

          remPath->type = std::max(path->type, type);

          // Add to the list for the next iteration if not already in the list
/* 中文逐项-变量 V1156：i
 * i（int；函数 ncclTopoSetPaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：for (i=0; i<nextNodeList.count; i++) if (nextNodeList.list[i] == remNode)
 * break;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
          int i;
          for (i=0; i<nextNodeList.count; i++) if (nextNodeList.list[i] == remNode) break;
          if (i == nextNodeList.count) nextNodeList.list[nextNodeList.count++] = remNode;
        }
      }
    }
    memcpy(&nodeList, &nextNodeList, sizeof(nodeList));
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0442：printNodePaths
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char line[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 编译期分支/指令：#ifdef ENABLE_TRACE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   3. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Paths from %s/%lX :", topoNodeTypeStr[node->type], node->id);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   4. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     5. 调用 / 计算 / 写入：sprintf(line, "%s/%lX :", topoNodeTypeStr[node->type], node->id);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 准备局部状态：int offset = strlen(line);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 循环：初始 int t=0;；继续条件 t<NCCL_TOPO_NODE_TYPES；每轮更新 t++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   8. 条件判断：(node->paths[t] == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     9. 真分支：；仅条件为真执行以下子项。
 *       10. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   11. 循环：初始 int n = 0;；继续条件 n<system->nodes[t].count；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     12. 编译期分支/指令：#ifdef ENABLE_TRACE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       13. 调用 / 计算 / 写入：line[0] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       14. 准备局部状态：int offset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       15. 循环：初始 int i=0;；继续条件 i<node->paths[t][n].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         16. 准备局部状态：struct ncclTopoLink* link = node->paths[t][n].list[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         17. 准备局部状态：struct ncclTopoNode* remNode = link->remNode;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         18. 调用 / 计算 / 写入：sprintf(line+offset, "--%s->%s/%lX", topoLinkTypeStr[link->type], topoNodeTypeStr[remNode->type], remNode->id);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         19. 调用 / 计算 / 写入：offset = strlen(line);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       21. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "%s (%f)", line, node->paths[t][n].width);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       22. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *         23. 调用 / 计算 / 写入：sprintf(line+offset, "%s/%lX (%d/%f/%s) ", topoNodeTypeStr[t], system->nodes[t].nodes[n].id, node->paths[t][n].count, node->paths[t][n].width, topoPathTypeStr[node->paths[t][n].type]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         24. 调用 / 计算 / 写入：offset = strlen(line);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   25. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 26. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 27. 编译期分支/指令：#ifndef ENABLE_TRACE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   28. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0442
 */
/* 中文逐项-函数 F0442：printNodePaths
 * 用途与位置：打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 node（struct ncclTopoNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoPrintPaths。
 * 本体直接出现的调用/包装：sprintf；strlen。
 * 需要对照的边界/推进条件：t<NCCL_TOPO_NODE_TYPES；node->paths[t] ==
 * NULL；n<system->nodes[t].count；i<node->paths[t][n].count。各条件所属分支见紧邻函数体。
 */
static void printNodePaths(struct ncclTopoSystem* system, struct ncclTopoNode* node) {
/* 中文逐项-变量 V1157：line
 * line（char；函数 printNodePaths 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：sprintf(line, "%s/%lX :", topoNodeTypeStr[node->type], node->id);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[1024];
#ifdef ENABLE_TRACE
  INFO(NCCL_GRAPH, "Paths from %s/%lX :", topoNodeTypeStr[node->type], node->id);
#else
  sprintf(line, "%s/%lX :", topoNodeTypeStr[node->type], node->id);
/* 中文逐项-变量 V1158：offset
 * offset（int；函数 printNodePaths 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = strlen(line)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int offset = strlen(line);
#endif
  for (/* 中文逐项-变量 V1159：t
 * t（int；函数 printNodePaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：t<NCCL_TOPO_NODE_TYPES；循环体与增量表达式决定每轮如何推进。
 */int t=0; t<NCCL_TOPO_NODE_TYPES; t++) {
    if (node->paths[t] == NULL) continue;
    for (/* 中文逐项-变量 V1160：n
 * n（int；函数 printNodePaths 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[t].count；循环体与增量表达式决定每轮如何推进。
 */int n = 0; n<system->nodes[t].count; n++) {
#ifdef ENABLE_TRACE
      line[0] = 0;
/* 中文逐项-变量 V1161：offset
 * offset（int；函数 printNodePaths 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int offset = 0;
      for (/* 中文逐项-变量 V1162：i
 * i（int；函数 printNodePaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<node->paths[t][n].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<node->paths[t][n].count; i++) {
/* 中文逐项-变量 V1163：link
 * link（struct ncclTopoLink；函数 printNodePaths 内）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 初值/来源 link = node->paths[t][n].list[i]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclTopoLink* link = node->paths[t][n].list[i];
/* 中文逐项-变量 V1164：remNode
 * remNode（struct ncclTopoNode；函数 printNodePaths 内）：硬件边指向的另一个节点，路径搜索沿此指针扩展。
 * 初值/来源 remNode = link->remNode；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclTopoNode* remNode = link->remNode;
        sprintf(line+offset, "--%s->%s/%lX", topoLinkTypeStr[link->type], topoNodeTypeStr[remNode->type], remNode->id);
        offset = strlen(line);
      }
      INFO(NCCL_GRAPH, "%s (%f)", line, node->paths[t][n].width);
#else
      sprintf(line+offset, "%s/%lX (%d/%f/%s) ", topoNodeTypeStr[t], system->nodes[t].nodes[n].id, node->paths[t][n].count, node->paths[t][n].width, topoPathTypeStr[node->paths[t][n].type]);
      offset = strlen(line);
#endif
    }
  }
#ifndef ENABLE_TRACE
  INFO(NCCL_GRAPH, "%s", line);
#endif
}

/* 中文内部逻辑 F0443：ncclTopoPrintPaths
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int i=0;；继续条件 i<system->nodes[GPU].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 调用 / 计算 / 写入：printNodePaths(system, system->nodes[GPU].nodes+i);；打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。
 * 3. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 4. 循环：初始 int i=0;；继续条件 i<system->nodes[NET].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：printNodePaths(system, system->nodes[NET].nodes+i);；打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0443
 */
/* 中文逐项-函数 F0443：ncclTopoPrintPaths
 * 用途与位置：打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoPrint。
 * 本体直接出现的调用/包装：printNodePaths。
 * 需要对照的边界/推进条件：i<system->nodes[GPU].count；i<system->nodes[NET].count。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoPrintPaths(struct ncclTopoSystem* system) {
  for (/* 中文逐项-变量 V1165：i
 * i（int；函数 ncclTopoPrintPaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[GPU].count; i++) {
    printNodePaths(system, system->nodes[GPU].nodes+i);
  }
  for (/* 中文逐项-变量 V1166：i
 * i（int；函数 ncclTopoPrintPaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[NET].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[NET].count; i++) {
    printNodePaths(system, system->nodes[NET].nodes+i);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0444：getLocalCpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int minHops = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int localCpu = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct ncclTopoLinkList* paths = system->nodes[GPU].nodes[gpu].paths[CPU];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 int c=0;；继续条件 c<system->nodes[CPU].count；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 准备局部状态：int hops = paths[c].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   6. 条件判断：(minHops == 0 || hops < minHops)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     7. 真分支：；仅条件为真执行以下子项。
 *       8. 调用 / 计算 / 写入：localCpu = c;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       9. 调用 / 计算 / 写入：minHops = hops;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 条件判断：(localCpu == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 调用 / 计算 / 写入：WARN("Error : could not find CPU close to GPU %d", gpu);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     14. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 15. 调用 / 计算 / 写入：*retCpu = localCpu;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0444
 */
/* 中文逐项-函数 F0444：getLocalCpu
 * 用途与位置：为 GPU/设备选择其局部 CPU 节点，供亲和性和通过 CPU 绕行的路径计算。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 gpu（int gpu）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 参数 retCpu（int* retCpu）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputePaths。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：c<system->nodes[CPU].count；minHops == 0 || hops < minHops；localCpu == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getLocalCpu(struct ncclTopoSystem* system, int gpu, int* retCpu) {
  // Find the closest CPU to a GPU
/* 中文逐项-变量 V1167：minHops
 * minHops（int；函数 getLocalCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minHops = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int minHops = 0;
/* 中文逐项-变量 V1168：localCpu
 * localCpu（int；函数 getLocalCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 localCpu = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int localCpu = -1;
/* 中文逐项-变量 V1169：paths
 * paths（struct ncclTopoLinkList；函数 getLocalCpu 内）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 * 初值/来源 paths = system->nodes[GPU].nodes[gpu].paths[CPU]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoLinkList* paths = system->nodes[GPU].nodes[gpu].paths[CPU];
  for (/* 中文逐项-变量 V1170：c
 * c（int；函数 getLocalCpu 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<system->nodes[CPU].count；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<system->nodes[CPU].count; c++) {
/* 中文逐项-变量 V1171：hops
 * hops（int；函数 getLocalCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 hops = paths[c].count；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int hops = paths[c].count;
    if (minHops == 0 || hops < minHops) {
      localCpu = c;
      minHops = hops;
    }
  }
  if (localCpu == -1) {
    WARN("Error : could not find CPU close to GPU %d", gpu);
    return ncclInternalError;
  }
  *retCpu = localCpu;
  return ncclSuccess;
}

/* 中文内部逻辑 F0445：addInterStep
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclTopoNode* cpuNode = system->nodes[tx].nodes+ix;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclTopoNode* srcNode = system->nodes[t1].nodes+i1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int l=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 int i=0;；继续条件 i<srcNode->paths[tx][ix].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：srcNode->paths[t2][i2].list[l++] = srcNode->paths[tx][ix].list[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 循环：初始 int i=0;；继续条件 i<cpuNode->paths[t2][i2].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   8. 调用 / 计算 / 写入：srcNode->paths[t2][i2].list[l++] = cpuNode->paths[t2][i2].list[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 10. 调用 / 计算 / 写入：srcNode->paths[t2][i2].count = l;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：srcNode->paths[t2][i2].type = std::max(srcNode->paths[tx][ix].type, cpuNode->paths[t2][i2].type);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 条件判断：(tx == GPU)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 调用 / 计算 / 写入：srcNode->paths[t2][i2].type = PATH_PXN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：srcNode->paths[t2][i2].width = std::min(srcNode->paths[tx][ix].width, cpuNode->paths[t2][i2].width);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0445
 */
/* 中文逐项-函数 F0445：addInterStep
 * 用途与位置：用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 tx（int tx）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 ix（int ix）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 t1（int t1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 i1（int i1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 t2（int t2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 i2（int i2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputePaths。
 * 本体直接出现的调用/包装：std::max；std::min。
 * 需要对照的边界/推进条件：i<srcNode->paths[tx][ix].count；i<cpuNode->paths[t2][i2].count；tx == GPU。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t addInterStep(struct ncclTopoSystem* system, int tx, int ix, int t1, int i1, int t2, int i2) {
/* 中文逐项-变量 V1172：cpuNode
 * cpuNode（struct ncclTopoNode；函数 addInterStep 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cpuNode = system->nodes[tx].nodes+ix；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* cpuNode = system->nodes[tx].nodes+ix;
/* 中文逐项-变量 V1173：srcNode
 * srcNode（struct ncclTopoNode；函数 addInterStep 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 srcNode = system->nodes[t1].nodes+i1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* srcNode = system->nodes[t1].nodes+i1;

/* 中文逐项-变量 V1174：l
 * l（int；函数 addInterStep 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int l=0;
  // Node 1 -> CPU
  for (/* 中文逐项-变量 V1175：i
 * i（int；函数 addInterStep 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<srcNode->paths[tx][ix].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<srcNode->paths[tx][ix].count; i++) srcNode->paths[t2][i2].list[l++] = srcNode->paths[tx][ix].list[i];
  // CPU -> Node 2
  for (/* 中文逐项-变量 V1176：i
 * i（int；函数 addInterStep 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<cpuNode->paths[t2][i2].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<cpuNode->paths[t2][i2].count; i++) srcNode->paths[t2][i2].list[l++] = cpuNode->paths[t2][i2].list[i];

  // Update path characteristics
  srcNode->paths[t2][i2].count = l;
  srcNode->paths[t2][i2].type = std::max(srcNode->paths[tx][ix].type, cpuNode->paths[t2][i2].type);
  if (tx == GPU) srcNode->paths[t2][i2].type = PATH_PXN;
  srcNode->paths[t2][i2].width = std::min(srcNode->paths[tx][ix].width, cpuNode->paths[t2][i2].width);
  return ncclSuccess;
}

// Remove/free paths for a given type
/* 中文内部逻辑 F0446：ncclTopoRemovePathType
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int t=0;；继续条件 t<NCCL_TOPO_NODE_TYPES；每轮更新 t++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 循环：初始 int n=0;；继续条件 n<system->nodes[t].count；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     3. 准备局部状态：struct ncclTopoNode* node = system->nodes[t].nodes+n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     4. 调用 / 计算 / 写入：free(node->paths[nodeType]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 调用 / 计算 / 写入：node->paths[nodeType] = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   7. 循环：初始 int n=0;；继续条件 n<system->nodes[nodeType].count；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     8. 准备局部状态：struct ncclTopoNode* node = system->nodes[nodeType].nodes+n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 调用 / 计算 / 写入：free(node->paths[t]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     10. 调用 / 计算 / 写入：node->paths[t] = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0446
 */
/* 中文逐项-函数 F0446：ncclTopoRemovePathType
 * 用途与位置：释放每个硬件节点到指定目标类型的路径缓存，避免拓扑裁剪后继续使用旧下标。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 nodeType（int nodeType）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputePaths；src/graph/paths.cc::ncclTopoFree。
 * 本体直接出现的调用/包装：free。
 * 需要对照的边界/推进条件：t<NCCL_TOPO_NODE_TYPES；n<system->nodes[t].count；n<system->nodes[nodeType].count。各条件所属分支见紧邻函数体。
 */
static void ncclTopoRemovePathType(struct ncclTopoSystem* system, int nodeType) {
  for (/* 中文逐项-变量 V1177：t
 * t（int；函数 ncclTopoRemovePathType 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：t<NCCL_TOPO_NODE_TYPES；循环体与增量表达式决定每轮如何推进。
 */int t=0; t<NCCL_TOPO_NODE_TYPES; t++) {
    // Remove links _to_ the given type
    for (/* 中文逐项-变量 V1178：n
 * n（int；函数 ncclTopoRemovePathType 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[t].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[t].count; n++) {
/* 中文逐项-变量 V1179：node
 * node（struct ncclTopoNode；函数 ncclTopoRemovePathType 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = system->nodes[t].nodes+n；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoNode* node = system->nodes[t].nodes+n;
      free(node->paths[nodeType]);
      node->paths[nodeType] = NULL;
    }
    // Remove links _from_ the given type
    for (/* 中文逐项-变量 V1180：n
 * n（int；函数 ncclTopoRemovePathType 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[nodeType].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[nodeType].count; n++) {
/* 中文逐项-变量 V1181：node
 * node（struct ncclTopoNode；函数 ncclTopoRemovePathType 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = system->nodes[nodeType].nodes+n；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoNode* node = system->nodes[nodeType].nodes+n;
      free(node->paths[t]);
      node->paths[t] = NULL;
    }
  }
}

/* 中文逐项-变量 V1182：levelsOldToNew
 * levelsOldToNew（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 levelsOldToNew = { PATH_LOC, PATH_PIX, PATH_PXB, PATH_PHB, PATH_SYS, PATH_SYS
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static const int levelsOldToNew[] = { PATH_LOC, PATH_PIX, PATH_PXB, PATH_PHB, PATH_SYS, PATH_SYS };
/* 中文内部逻辑 F0447：ncclGetLevel
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(*level == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 准备局部状态：int l = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     4. 条件判断：(disableEnv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       5. 真分支：；仅条件为真执行以下子项。
 *         6. 准备局部状态：char* str = getenv(disableEnv);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         7. 条件判断：(str)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           8. 真分支：；仅条件为真执行以下子项。
 *             9. 准备局部状态：int disable = strtol(str, NULL, 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             10. 条件判断：(disable == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               11. 真分支：；仅条件为真执行以下子项。
 *                 12. 调用 / 计算 / 写入：l = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 条件判断：(l == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       14. 真分支：；仅条件为真执行以下子项。
 *         15. 准备局部状态：char* str = getenv(levelEnv);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         16. 条件判断：(str)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           17. 真分支：；仅条件为真执行以下子项。
 *             18. 循环：初始 int i=0;；继续条件 i<=PATH_SYS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               19. 条件判断：(strcmp(str, topoPathTypeStr[i]) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 20. 真分支：；仅条件为真执行以下子项。
 *                   21. 调用 / 计算 / 写入：l = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   22. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *             23. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *             24. 条件判断：(l == -1 && str[0] >= '0' && str[0] <= '9')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               25. 真分支：；仅条件为真执行以下子项。
 *                 26. 准备局部状态：int oldLevel = strtol(str, NULL, 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 27. 准备局部状态：const int maxOldLevel = sizeof(levelsOldToNew)/sizeof(int) - 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 28. 条件判断：(oldLevel > maxOldLevel)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   29. 真分支：；仅条件为真执行以下子项。
 *                     30. 调用 / 计算 / 写入：oldLevel = maxOldLevel;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 31. 调用 / 计算 / 写入：l = levelsOldToNew[oldLevel];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     32. 条件判断：(l >= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       33. 真分支：；仅条件为真执行以下子项。
 *         34. 调用 / 计算 / 写入：INFO(NCCL_ALL, "%s set by environment to %s", levelEnv, topoPathTypeStr[l]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     35. 调用 / 计算 / 写入：*level = l >= 0 ? l : -2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 36. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0447
 */
/* 中文逐项-函数 F0447：ncclGetLevel
 * 用途与位置：把环境配置的拓扑距离名字或数字解析成内部路径级别，支持默认值与禁用策略。
 * 参数 level（int* level）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 disableEnv（const char* disableEnv）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 levelEnv（const char* levelEnv）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoCheckP2p；src/graph/paths.cc::ncclTopoCheckGdr。
 * 本体直接出现的调用/包装：getenv；strcmp；strtol。
 * 需要对照的边界/推进条件：*level == -1；disableEnv；str；disable == 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGetLevel(int* level, const char* disableEnv, const char* levelEnv) {
  if (*level == -1) {
/* 中文逐项-变量 V1183：l
 * l（int；函数 ncclGetLevel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
    int l = -1;
    if (disableEnv) {
/* 中文逐项-变量 V1184：str
 * str（char；函数 ncclGetLevel 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv(disableEnv)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      char* str = getenv(disableEnv);
      if (str) {
/* 中文逐项-变量 V1185：disable
 * disable（int；函数 ncclGetLevel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 disable = strtol(str, NULL, 0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        int disable = strtol(str, NULL, 0);
        if (disable == 1) l = 0;
      }
    }
    if (l == -1) {
/* 中文逐项-变量 V1186：str
 * str（char；函数 ncclGetLevel 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv(levelEnv)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      char* str = getenv(levelEnv);
      if (str) {
        for (/* 中文逐项-变量 V1187：i
 * i（int；函数 ncclGetLevel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<=PATH_SYS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<=PATH_SYS; i++) {
          if (strcmp(str, topoPathTypeStr[i]) == 0) {
            l = i;
            break;
          }
        }
        // Old style numbering
        // levelsOldToNew to is an array with each index corresponding to the
        // "old level" int, and each value mapping to the correct value defined in topo.h
        // maxOldLevel is a quick check to handle out of bounds (based on the length of levelsOldToNew)
        if (l == -1 && str[0] >= '0' && str[0] <= '9') {
/* 中文逐项-变量 V1188：oldLevel
 * oldLevel（int；函数 ncclGetLevel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 oldLevel = strtol(str, NULL, 0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          int oldLevel = strtol(str, NULL, 0);
/* 中文逐项-变量 V1189：maxOldLevel
 * maxOldLevel（int；函数 ncclGetLevel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxOldLevel = sizeof(levelsOldToNew)/sizeof(int) - 1；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
          const int maxOldLevel = sizeof(levelsOldToNew)/sizeof(int) - 1;
          if (oldLevel > maxOldLevel) oldLevel = maxOldLevel;
          l = levelsOldToNew[oldLevel];
        }
      }
    }
    if (l >= 0) INFO(NCCL_ALL, "%s set by environment to %s", levelEnv, topoPathTypeStr[l]);
    *level = l >= 0 ? l : -2;
  }
  return ncclSuccess;
}

/* 中文逐项-参数getter P0008：IgnoreDisabledP2p
 * 生成函数 ncclParamIgnoreDisabledP2p()，读取 NCCL_IGNORE_DISABLED_P2P，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IgnoreDisabledP2p, "IGNORE_DISABLED_P2P", 0);

/* 中文逐项-变量 V1190：ncclTopoUserP2pLevel
 * ncclTopoUserP2pLevel（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclTopoUserP2pLevel = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
int ncclTopoUserP2pLevel = -1;
/* 中文内部逻辑 F0448：ncclTopoCheckP2p
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*p2p = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 条件判断：(read)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：*read = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 条件判断：(intermediateRank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：*intermediateRank = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 准备局部状态：int g1, g2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoIdToIndex(system, GPU, id1, &g1));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 * 10. 准备局部状态：struct ncclTopoNode* gpu1 = system->nodes[GPU].nodes+g1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 条件判断：(ncclTopoIdToIndex(system, GPU, id2, &g2) == ncclInternalError)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 14. 准备局部状态：int intermediateIndex = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 准备局部状态：struct ncclTopoLinkList* path = gpu1->paths[GPU]+g2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 16. 条件判断：(path->count == 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   17. 真分支：；仅条件为真执行以下子项。
 *     18. 准备局部状态：struct ncclTopoNode* intermediateNode = path->list[0]->remNode;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     19. 条件判断：(intermediateNode->type == GPU)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       20. 真分支：；仅条件为真执行以下子项。
 *         21. 调用 / 计算 / 写入：intermediateIndex = intermediateNode - system->nodes[GPU].nodes;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         22. 条件判断：(intermediateRank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           23. 真分支：；仅条件为真执行以下子项。
 *             24. 调用 / 计算 / 写入：*intermediateRank = intermediateNode->gpu.rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 25. 准备局部状态：int p2pLevel = PATH_SYS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 26. 条件判断：(ncclTopoUserP2pLevel == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   27. 真分支：；仅条件为真执行以下子项。
 *     28. 调用 / 计算 / 写入：NCCLCHECK(ncclGetLevel(&ncclTopoUserP2pLevel, "NCCL_P2P_DISABLE", "NCCL_P2P_LEVEL"));；失败时可能提前return，成功才继续；把环境配置的拓扑距离名字或数字解析成内部路径级别，支持默认值与禁用策略。
 * 29. 条件判断：(ncclTopoUserP2pLevel != -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   30. 真分支：；仅条件为真执行以下子项。
 *     31. 调用 / 计算 / 写入：p2pLevel = ncclTopoUserP2pLevel;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     32. 控制转移：goto compare;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 33. 准备局部状态：int arch, vendor, model;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 34. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCpuType(system, &arch, &vendor, &model));；失败时可能提前return，成功才继续；从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。
 * 35. 条件判断：(arch == NCCL_TOPO_CPU_ARCH_ARM)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   36. 真分支：；仅条件为真执行以下子项。
 *     37. 调用 / 计算 / 写入：p2pLevel = PATH_PXB;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 38. 条件判断：(arch == NCCL_TOPO_CPU_ARCH_X86 && vendor == NCCL_TOPO_CPU_VENDOR_INTEL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   39. 真分支：；仅条件为真执行以下子项。
 *     40. 调用 / 计算 / 写入：p2pLevel = PATH_PXB;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 41. 条件判断：(arch == NCCL_TOPO_CPU_ARCH_X86 && vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   42. 真分支：；仅条件为真执行以下子项。
 *     43. 调用 / 计算 / 写入：p2pLevel = PATH_PXB;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 44. 跳转标签：compare；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   45. 条件判断：(path->type <= p2pLevel)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     46. 真分支：；仅条件为真执行以下子项。
 *       47. 调用 / 计算 / 写入：*p2p = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 48. 条件判断：(*p2p == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   49. 真分支：；仅条件为真执行以下子项。
 *     50. 条件判断：(g1 != g2 && ncclParamIgnoreDisabledP2p() != 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       51. 真分支：；仅条件为真执行以下子项。
 *         52. 准备局部状态：int indexes[3] = {-1,-1,-1};；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         53. 准备局部状态：int verticeN = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         54. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 *         55. 调用 / 计算 / 写入：indexes[verticeN++] = system->nodes[GPU].nodes[g1].gpu.dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         56. 条件判断：(intermediateIndex != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           57. 真分支：；仅条件为真执行以下子项。
 *             58. 调用 / 计算 / 写入：indexes[verticeN++] = system->nodes[GPU].nodes[intermediateIndex].gpu.dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         59. 调用 / 计算 / 写入：indexes[verticeN++] = system->nodes[GPU].nodes[g2].gpu.dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         60. 循环：初始 int i=1;；继续条件 i < verticeN；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           61. 准备局部状态：nvmlGpuP2PStatus_t status;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           62. 调用 / 计算 / 写入：status = ncclNvmlDevicePairs[indexes[i-1]][indexes[i-0]].p2pStatusRead;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           63. 准备局部状态：bool good = status == NVML_P2P_STATUS_OK;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           64. 调用 / 计算 / 写入：status = ncclNvmlDevicePairs[indexes[i-1]][indexes[i-0]].p2pStatusWrite;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           65. 调用 / 计算 / 写入：good &= status == NVML_P2P_STATUS_OK;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           66. 条件判断：(!good)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             67. 真分支：；仅条件为真执行以下子项。
 *               68. 条件判断：(ncclParamIgnoreDisabledP2p())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 69. 真分支：；仅条件为真执行以下子项。
 *                   70. 调用 / 计算 / 写入：*p2p = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 71. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   72. 条件判断：(path->type <= PATH_NVB)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     73. 真分支：；仅条件为真执行以下子项。
 *                       74. 调用 / 计算 / 写入：WARN("P2P is disabled between NVLINK connected GPUs %d and %d. This should not be the case given their connectivity, and is probably due to a hardware issue. If you still want to proceed, you can set  …（完整代码见函数体）；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                       75. 函数返回：return ncclUnhandledCudaError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *                     76. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                       77. 条件判断：(path->type < PATH_SYS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                         78. 真分支：；仅条件为真执行以下子项。
 *                           79. 调用 / 计算 / 写入：INFO(NCCL_INIT, "P2P is disabled between connected GPUs %d and %d. You can repress this message with NCCL_IGNORE_DISABLED_P2P=1.", indexes[i-1], indexes[i-0]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         80. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 81. 条件判断：(path->type == PATH_NVL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   82. 真分支：；仅条件为真执行以下子项。
 *     83. 准备局部状态：struct ncclTopoNode* gpu2 = system->nodes[GPU].nodes+g2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     84. 条件判断：(read && (gpu1->gpu.cudaCompCap == gpu2->gpu.cudaCompCap) && (gpu1->gpu.cudaCompCap == 80))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       85. 真分支：；仅条件为真执行以下子项。
 *         86. 调用 / 计算 / 写入：*read = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 87. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0448
 */
/* 中文逐项-函数 F0448：ncclTopoCheckP2p
 * 用途与位置：结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 id1（int64_t id1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 id2（int64_t id2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 p2p（int* p2p）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 read（int *read）：P2P/GDR方向是否采用读/拉取策略，发送读显存和接收写显存可能有不同能力条件。
 * 参数 intermediateRank（int* intermediateRank）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputePaths；src/transport/p2p.cc::p2pCanConnect；src/transport/p2p.cc::p2pGetInfo。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclGetLevel；ncclNvmlEnsureInitialized；ncclParamIgnoreDisabledP2p；ncclTopoCpuType；ncclTopoIdToIndex。
 * 需要对照的边界/推进条件：read；intermediateRank；ncclTopoIdToIndex(system, GPU, id2, &g2) == ncclInternalError；path->count ==
 * 2。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclUnhandledCudaError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoCheckP2p(struct ncclTopoSystem* system, int64_t id1, int64_t id2, int* p2p, int *read, int* intermediateRank) {
  *p2p = 0;
  if (read) *read = 0;
  if (intermediateRank) *intermediateRank = -1;

  // Get GPUs from topology
/* 中文逐项-变量 V1191：g1、g2
 * g1（int；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoIdToIndex(system, GPU, id1, &g1));。若由输出参数填入，必须先检查填充调用成功再读值。
 * g2（int；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclTopoIdToIndex(system, GPU, id2, &g2) == ncclInternalError) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int g1, g2;
  NCCLCHECK(ncclTopoIdToIndex(system, GPU, id1, &g1));
/* 中文逐项-变量 V1192：gpu1
 * gpu1（struct ncclTopoNode；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gpu1 = system->nodes[GPU].nodes+g1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* gpu1 = system->nodes[GPU].nodes+g1;
  if (ncclTopoIdToIndex(system, GPU, id2, &g2) == ncclInternalError) {
    // GPU not found, we can't use p2p.
    return ncclSuccess;
  }

/* 中文逐项-变量 V1193：intermediateIndex
 * intermediateIndex（int；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 intermediateIndex = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int intermediateIndex = -1;
  // Set intermediate GPU rank, if routing through an intermediate GPU.
/* 中文逐项-变量 V1194：path
 * path（struct ncclTopoLinkList；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 path = gpu1->paths[GPU]+g2；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoLinkList* path = gpu1->paths[GPU]+g2;
  if (path->count == 2) {
/* 中文逐项-变量 V1195：intermediateNode
 * intermediateNode（struct ncclTopoNode；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 intermediateNode = path->list[0]->remNode；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* intermediateNode = path->list[0]->remNode;
    if (intermediateNode->type == GPU) {
      intermediateIndex = intermediateNode - system->nodes[GPU].nodes;
      if (intermediateRank) *intermediateRank = intermediateNode->gpu.rank;
    }
  }

  // In general, use P2P whenever we can.
/* 中文逐项-变量 V1196：p2pLevel
 * p2pLevel（int；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 p2pLevel = PATH_SYS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int p2pLevel = PATH_SYS;

  // User override
  if (ncclTopoUserP2pLevel == -1)
    NCCLCHECK(ncclGetLevel(&ncclTopoUserP2pLevel, "NCCL_P2P_DISABLE", "NCCL_P2P_LEVEL"));
  if (ncclTopoUserP2pLevel != -2) {
    p2pLevel = ncclTopoUserP2pLevel;
    goto compare;
  }

  // Don't use P2P through ARM CPUs
/* 中文逐项-变量 V1197：arch、vendor、model
 * arch（int；函数 ncclTopoCheckP2p 内）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(system, &arch, &vendor, &model));。若由输出参数填入，必须先检查填充调用成功再读值。
 * vendor（int；函数 ncclTopoCheckP2p 内）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(system, &arch, &vendor, &model));。若由输出参数填入，必须先检查填充调用成功再读值。
 * model（int；函数 ncclTopoCheckP2p 内）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(system, &arch, &vendor, &model));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int arch, vendor, model;
  NCCLCHECK(ncclTopoCpuType(system, &arch, &vendor, &model));
  if (arch == NCCL_TOPO_CPU_ARCH_ARM) p2pLevel = PATH_PXB;
  if (arch == NCCL_TOPO_CPU_ARCH_X86 && vendor == NCCL_TOPO_CPU_VENDOR_INTEL) {
    p2pLevel = PATH_PXB;
  }
  if (arch == NCCL_TOPO_CPU_ARCH_X86 && vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN) {
    p2pLevel = PATH_PXB;
  }

compare:
  // Compute the PCI distance and compare with the p2pLevel.
  if (path->type <= p2pLevel) *p2p = 1;

  if (*p2p == 1) {
    // NCCL_IGNORE_DISABLED_P2P=2 is used by unit tests that don't want to
    // validate against NVML at all since they are pretending to be on other hw.
    if (g1 != g2 && ncclParamIgnoreDisabledP2p() != 2) {
/* 中文逐项-变量 V1198：indexes
 * indexes（int；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 indexes = {-1,-1,-1}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
      int indexes[3] = {-1,-1,-1};
/* 中文逐项-变量 V1199：verticeN
 * verticeN（int；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 verticeN = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int verticeN = 0;
      NCCLCHECK(ncclNvmlEnsureInitialized());

      indexes[verticeN++] = system->nodes[GPU].nodes[g1].gpu.dev;
      if (intermediateIndex != -1) indexes[verticeN++] = system->nodes[GPU].nodes[intermediateIndex].gpu.dev;
      indexes[verticeN++] = system->nodes[GPU].nodes[g2].gpu.dev;

      for (/* 中文逐项-变量 V1200：i
 * i（int；函数 ncclTopoCheckP2p 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i < verticeN；循环体与增量表达式决定每轮如何推进。
 */int i=1; i < verticeN; i++) {
/* 中文逐项-变量 V1201：status
 * status（nvmlGpuP2PStatus_t；函数 ncclTopoCheckP2p 内）：完成/调用的通用状态和厂商错误细节，非成功不能把目标数据当作有效结果。
 * 此处未显式给初值；后续首个文本使用点：status = ncclNvmlDevicePairs[indexes[i-1]][indexes[i-0]].p2pStatusRead;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        nvmlGpuP2PStatus_t status;
        status = ncclNvmlDevicePairs[indexes[i-1]][indexes[i-0]].p2pStatusRead;
/* 中文逐项-变量 V1202：good
 * good（bool；函数 ncclTopoCheckP2p 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 good = status == NVML_P2P_STATUS_OK；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        bool good = status == NVML_P2P_STATUS_OK;
        status = ncclNvmlDevicePairs[indexes[i-1]][indexes[i-0]].p2pStatusWrite;
        good &= status == NVML_P2P_STATUS_OK;
        if (!good) {
          if (ncclParamIgnoreDisabledP2p()) {
            *p2p = 0;
          } else if (path->type <= PATH_NVB) {
            WARN("P2P is disabled between NVLINK connected GPUs %d and %d. This should not be the case given their connectivity, and is probably due to a hardware issue. If you still want to proceed, you can set NCCL_IGNORE_DISABLED_P2P=1.", indexes[i-1], indexes[i-0]);
            return ncclUnhandledCudaError;
          } else if (path->type < PATH_SYS) {
            INFO(NCCL_INIT, "P2P is disabled between connected GPUs %d and %d. You can repress this message with NCCL_IGNORE_DISABLED_P2P=1.", indexes[i-1], indexes[i-0]);
          }
        }
      }
    }
  }

  if (path->type == PATH_NVL) {
/* 中文逐项-变量 V1203：gpu2
 * gpu2（struct ncclTopoNode；函数 ncclTopoCheckP2p 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gpu2 = system->nodes[GPU].nodes+g2；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* gpu2 = system->nodes[GPU].nodes+g2;
    // Enable P2P Read for Ampere/NVLink only
    if (read && (gpu1->gpu.cudaCompCap == gpu2->gpu.cudaCompCap) && (gpu1->gpu.cudaCompCap == 80)) *read = 1;
  }

  return ncclSuccess;
}

/* 中文逐项-参数getter P0009：NetGdrRead
 * 生成函数 ncclParamNetGdrRead()，读取 NCCL_NET_GDR_READ，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(NetGdrRead, "NET_GDR_READ", -2);
/* 中文逐项-变量 V1204：ncclTopoUserGdrLevel
 * ncclTopoUserGdrLevel（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclTopoUserGdrLevel = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
int ncclTopoUserGdrLevel = -1;

/* 中文导读：
 * 检查 GPU 与所选 NET 设备能否使用 GPUDirect RDMA，并结合方向、路径距离和环境配置决定。
 * GDR 表示 NIC 可访问 GPU 内存；本版本内置 IB 的 WQE 提交和 CQ 轮询仍由 CPU Proxy 执行。
 */
/* 中文精读：GPU Direct RDMA 是能力与策略的共同结果
 * 检查 GPU 与 NIC 的 GDR 支持、二者的拓扑距离，以及发送读 GPU 显存时的额外策略。
 * read 参数区分 NIC 从 GPU 读数据的发送侧与 NIC 向 GPU 写入的接收侧，允许结果不同。
 * useGdr 决定 NET 分配/注册 GPU 缓冲还是主机缓冲；不决定由 CPU 还是 GPU 调用 ibv_post_send。
 * 本版本内置 IB 的 verbs 请求仍由 CPU Proxy 推进。
 */
/* 中文内部逻辑 F0449：ncclTopoCheckGdr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*useGdr = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 准备局部状态：int n, g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 * 4. 准备局部状态：struct ncclTopoNode* net = system->nodes[NET].nodes+n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoIdToIndex(system, GPU, busId, &g));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 * 6. 准备局部状态：struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 条件判断：(net->net.gdrSupport == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 条件判断：(gpu->gpu.gdrSupport == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 13. 条件判断：(read)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 准备局部状态：int gdrReadParam = ncclParamNetGdrRead();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     16. 条件判断：(gdrReadParam == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       17. 真分支：；仅条件为真执行以下子项。
 *         18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     19. 条件判断：(gdrReadParam < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       20. 真分支：；仅条件为真执行以下子项。
 *         21. 准备局部状态：int nvlink = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         22. 条件判断：(system->nodes[GPU].count == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           23. 真分支：；仅条件为真执行以下子项。
 *             24. 调用 / 计算 / 写入：nvlink = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         25. 循环：初始 int i=0;；继续条件 i<system->nodes[GPU].count；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           26. 条件判断：(i == g)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             27. 真分支：；仅条件为真执行以下子项。
 *               28. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *           29. 条件判断：(gpu->paths[GPU][i].type == PATH_NVL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             30. 真分支：；仅条件为真执行以下子项。
 *               31. 调用 / 计算 / 写入：nvlink = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               32. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         33. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         34. 条件判断：(!nvlink)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           35. 真分支：；仅条件为真执行以下子项。
 *             36. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 37. 准备局部状态：int netGdrLevel = PATH_PXB;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 38. 调用 / 计算 / 写入：NCCLCHECK(ncclGetLevel(&ncclTopoUserGdrLevel, NULL, "NCCL_NET_GDR_LEVEL"));；失败时可能提前return，成功才继续；把环境配置的拓扑距离名字或数字解析成内部路径级别，支持默认值与禁用策略。
 * 39. 条件判断：(ncclTopoUserGdrLevel != -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   40. 真分支：；仅条件为真执行以下子项。
 *     41. 调用 / 计算 / 写入：netGdrLevel = ncclTopoUserGdrLevel;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 42. 准备局部状态：int distance = gpu->paths[NET][n].type;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 43. 条件判断：(distance == PATH_PXN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   44. 真分支：；仅条件为真执行以下子项。
 *     45. 准备局部状态：int proxyRank, g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     46. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetIntermediateRank(system, gpu->gpu.rank, netDev, &proxyRank));；失败时可能提前return，成功才继续；找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。
 *     47. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoRankToIndex(system, proxyRank, &g));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 *     48. 准备局部状态：struct ncclTopoNode* proxyGpu = system->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     49. 调用 / 计算 / 写入：distance = proxyGpu->paths[NET][n].type;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 50. 条件判断：(distance > netGdrLevel)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   51. 真分支：；仅条件为真执行以下子项。
 *     52. 调用 / 计算 / 写入：INFO(NCCL_NET,"GPU Direct RDMA Disabled for GPU %lx / HCA %d (distance %d > %d)", busId, netDev, distance, netGdrLevel);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     53. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 54. 调用 / 计算 / 写入：*useGdr = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 55. 调用 / 计算 / 写入：INFO(NCCL_NET,"GPU Direct RDMA Enabled for GPU %lx / HCA %d (distance %d <= %d), read %d", busId, netDev, distance, netGdrLevel, read);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 56. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0449
 */
/* 中文逐项-函数 F0449：ncclTopoCheckGdr
 * 用途与位置：结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 busId（int64_t busId）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 参数 netDev（int netDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 参数 read（int read）：P2P/GDR方向是否采用读/拉取策略，发送读显存和接收写显存可能有不同能力条件。
 * 参数 useGdr（int* useGdr）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoGetPxnRanks；src/graph/paths.cc::ncclTopoComputePaths；src/transport/coll_net.cc::sendSetup；src/transport/coll_net.cc::recvSetup；src/transport/net.cc::sendSetup；src/transport/net.cc::recvSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclGetLevel；ncclParamNetGdrRead；ncclTopoGetIntermediateRank；ncclTopoIdToIndex；ncclTopoRankToIndex。
 * 需要对照的边界/推进条件：net->net.gdrSupport == 0；gpu->gpu.gdrSupport == 0；read；gdrReadParam == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoCheckGdr(struct ncclTopoSystem* system, int64_t busId, int netDev, int read, int* useGdr) {
  *useGdr = 0;

  // Get GPU and NET
/* 中文逐项-变量 V1205：n、g
 * n（int；函数 ncclTopoCheckGdr 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));。若由输出参数填入，必须先检查填充调用成功再读值。
 * g（int；函数 ncclTopoCheckGdr 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoIdToIndex(system, GPU, busId, &g));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int n, g;
  NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));
/* 中文逐项-变量 V1206：net
 * net（struct ncclTopoNode；函数 ncclTopoCheckGdr 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 net = system->nodes[NET].nodes+n；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* net = system->nodes[NET].nodes+n;
  NCCLCHECK(ncclTopoIdToIndex(system, GPU, busId, &g));
/* 中文逐项-变量 V1207：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoCheckGdr 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = system->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;

  // Check that both the NIC and GPUs support it
  if (net->net.gdrSupport == 0) return ncclSuccess;
  if (gpu->gpu.gdrSupport == 0) return ncclSuccess;

  if (read) { // For reads (sends) only enable under certain conditions
/* 中文逐项-变量 V1208：gdrReadParam
 * gdrReadParam（int；函数 ncclTopoCheckGdr 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdrReadParam = ncclParamNetGdrRead()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
    int gdrReadParam = ncclParamNetGdrRead();
    if (gdrReadParam == 0) return ncclSuccess;
    if (gdrReadParam < 0) {
/* 中文逐项-变量 V1209：nvlink
 * nvlink（int；函数 ncclTopoCheckGdr 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvlink = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int nvlink = 0;
      // Since we don't know whether there are other communicators,
      // it's better to keep things local if we have a single GPU.
      if (system->nodes[GPU].count == 1) nvlink = 1;
      for (/* 中文逐项-变量 V1210：i
 * i（int；函数 ncclTopoCheckGdr 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<system->nodes[GPU].count; i++) {
        if (i == g) continue;
        if (gpu->paths[GPU][i].type == PATH_NVL) {
          nvlink = 1;
          break;
        }
      }
      if (!nvlink) return ncclSuccess;
    }
  }

  // Check if we are close enough that it makes sense to enable GDR
/* 中文逐项-变量 V1211：netGdrLevel
 * netGdrLevel（int；函数 ncclTopoCheckGdr 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 netGdrLevel = PATH_PXB；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int netGdrLevel = PATH_PXB;
  NCCLCHECK(ncclGetLevel(&ncclTopoUserGdrLevel, NULL, "NCCL_NET_GDR_LEVEL"));
  if (ncclTopoUserGdrLevel != -2) netGdrLevel = ncclTopoUserGdrLevel;
/* 中文逐项-变量 V1212：distance
 * distance（int；函数 ncclTopoCheckGdr 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 distance = gpu->paths[NET][n].type；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int distance = gpu->paths[NET][n].type;
  if (distance == PATH_PXN) {
    // In case of PXN, use the intermediate GPU distance instead
/* 中文逐项-变量 V1213：proxyRank、g
 * proxyRank（int；函数 ncclTopoCheckGdr 内）：实际负责网络出口的代理 rank，PXN 时可能与发起通信的 GPU rank 不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetIntermediateRank(system, gpu->gpu.rank, netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 * g（int；函数 ncclTopoCheckGdr 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoRankToIndex(system, proxyRank, &g));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int proxyRank, g;
    NCCLCHECK(ncclTopoGetIntermediateRank(system, gpu->gpu.rank, netDev, &proxyRank));
    NCCLCHECK(ncclTopoRankToIndex(system, proxyRank, &g));
/* 中文逐项-变量 V1214：proxyGpu
 * proxyGpu（struct ncclTopoNode；函数 ncclTopoCheckGdr 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 proxyGpu = system->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* proxyGpu = system->nodes[GPU].nodes+g;
    distance = proxyGpu->paths[NET][n].type;
  }
  if (distance > netGdrLevel) {
    INFO(NCCL_NET,"GPU Direct RDMA Disabled for GPU %lx / HCA %d (distance %d > %d)", busId, netDev, distance, netGdrLevel);
    return ncclSuccess;
  }

  *useGdr = 1;
  INFO(NCCL_NET,"GPU Direct RDMA Enabled for GPU %lx / HCA %d (distance %d <= %d), read %d", busId, netDev, distance, netGdrLevel, read);
  return ncclSuccess;
}

/* 中文内部逻辑 F0450：ncclTopoGetIntermediateRank
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int n, g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));；失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。
 * 4. 准备局部状态：struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：struct ncclTopoLinkList* path = gpu->paths[NET]+n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 条件判断：(path->type == PATH_PXN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 准备局部状态：struct ncclTopoNode* node;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 准备局部状态：int type = NVS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     10. 循环：初始 int i=0;；继续条件 i<path->count && type == NVS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       11. 调用 / 计算 / 写入：node = path->list[i]->remNode;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       12. 调用 / 计算 / 写入：type = node->type;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     14. 条件判断：(type != GPU)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       15. 真分支：；仅条件为真执行以下子项。
 *         16. 调用 / 计算 / 写入：WARN("Could not find intermediate GPU between GPU rank %d and NIC %d\n", rank, netDev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         17. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     18. 调用 / 计算 / 写入：*intermediateRank = node->gpu.rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   19. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     20. 调用 / 计算 / 写入：*intermediateRank = rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 21. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0450
 */
/* 中文逐项-函数 F0450：ncclTopoGetIntermediateRank
 * 用途与位置：找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 netDev（int netDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 参数 intermediateRank（int* intermediateRank）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoCheckGdr；src/graph/search.cc::ncclTopoGetNetDev。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoIdToIndex；ncclTopoRankToIndex。
 * 需要对照的边界/推进条件：path->type == PATH_PXN；i<path->count && type == NVS；type != GPU。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetIntermediateRank(struct ncclTopoSystem* system, int rank, int netDev, int* intermediateRank) {
  // Get GPU and NET
/* 中文逐项-变量 V1215：n、g
 * n（int；函数 ncclTopoGetIntermediateRank 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));。若由输出参数填入，必须先检查填充调用成功再读值。
 * g（int；函数 ncclTopoGetIntermediateRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int n, g;
  NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));
  NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));
/* 中文逐项-变量 V1216：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoGetIntermediateRank 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = system->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
/* 中文逐项-变量 V1217：path
 * path（struct ncclTopoLinkList；函数 ncclTopoGetIntermediateRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 path = gpu->paths[NET]+n；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoLinkList* path = gpu->paths[NET]+n;
  if (path->type == PATH_PXN) {
/* 中文逐项-变量 V1218：node
 * node（struct ncclTopoNode；函数 ncclTopoGetIntermediateRank 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 此处未显式给初值；后续首个文本使用点：node = path->list[i]->remNode;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclTopoNode* node;
/* 中文逐项-变量 V1219：type
 * type（int；函数 ncclTopoGetIntermediateRank 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 初值/来源 type = NVS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int type = NVS;
    for (/* 中文逐项-变量 V1220：i
 * i（int；函数 ncclTopoGetIntermediateRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<path->count && type == NVS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<path->count && type == NVS; i++) {
      node = path->list[i]->remNode;
      type = node->type;
    }
    if (type != GPU) {
      WARN("Could not find intermediate GPU between GPU rank %d and NIC %d\n", rank, netDev);
      return ncclInternalError;
    }
    *intermediateRank = node->gpu.rank;
  } else {
    *intermediateRank = rank;
  }
  return ncclSuccess;
}

/* 中文逐项-参数getter P0010：PxnDisable
 * 生成函数 ncclParamPxnDisable()，读取 NCCL_PXN_DISABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(PxnDisable, "PXN_DISABLE", 0);

// Net v4 plugins don't have non-blocking connect/accept. We can't therefore use
// remote proxies without risking deadlocks
/* 中文内部逻辑 F0451：ncclPxnDisable
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：static int pxnDisable = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(pxnDisable == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 条件判断：(ncclNetVersion() == 4)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       5. 真分支：；仅条件为真执行以下子项。
 *         6. 调用 / 计算 / 写入：INFO(NCCL_INIT, "PXN Disabled as plugin is v4");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         7. 调用 / 计算 / 写入：pxnDisable = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       8. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         9. 调用 / 计算 / 写入：pxnDisable = ncclParamPxnDisable();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 函数返回：return pxnDisable;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0451
 */
/* 中文逐项-函数 F0451：ncclPxnDisable
 * 用途与位置：汇总参数和环境条件判断是否禁止 PXN，避免选择当前后端不支持的中间 GPU 网络路径。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputePaths；src/graph/search.cc::ncclTopoGetNetDev；src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：ncclNetVersion；ncclParamPxnDisable。
 * 需要对照的边界/推进条件：pxnDisable == -1；ncclNetVersion() == 4。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return pxnDisable;。CHECK类宏还可能提前返回/跳转。
 */
int ncclPxnDisable() {
/* 中文逐项-变量 V1221：pxnDisable
 * pxnDisable（int；函数 ncclPxnDisable 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pxnDisable = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  static int pxnDisable = -1;
  if (pxnDisable == -1) {
    if (ncclNetVersion() == 4) {
      INFO(NCCL_INIT, "PXN Disabled as plugin is v4");
      pxnDisable = 1;
    } else {
      pxnDisable = ncclParamPxnDisable();
    }
  }
  return pxnDisable;
}

/* 中文内部逻辑 F0452：ncclTopoGetPxnRanks
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclTopoSystem* system = comm->topo;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：*nranks = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：*intermediateRanks = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 条件判断：(system->nodes[NET].count == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 准备局部状态：int nr = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 准备局部状态：int* ranks = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 循环：初始 int rank=0;；继续条件 rank<comm->nRanks；每轮更新 rank++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   10. 准备局部状态：int netDev, proxyRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   11. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetNetDev(comm, comm->rank, NULL, 0, rank, &netDev, &proxyRank));；失败时可能提前return，成功才继续；按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。
 *   12. 条件判断：(proxyRank == comm->rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     13. 真分支：；仅条件为真执行以下子项。
 *       14. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   15. 准备局部状态：int useGdr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   16. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCheckGdr(comm->topo, comm->busId, netDev, 1, &useGdr));；失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。
 *   17. 条件判断：(useGdr == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     18. 真分支：；仅条件为真执行以下子项。
 *       19. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   20. 准备局部状态：int found = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   21. 循环：初始 int r=0;；继续条件 r<nr；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     22. 条件判断：(ranks[r] == proxyRank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       23. 真分支：；仅条件为真执行以下子项。
 *         24. 调用 / 计算 / 写入：found = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   25. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   26. 条件判断：(!found)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     27. 真分支：；仅条件为真执行以下子项。
 *       28. 调用 / 计算 / 写入：NCCLCHECK(ncclRealloc(&ranks, nr, nr+1));；失败时可能提前return，成功才继续；分配更大且尾部清零的 CPU 数组，复制旧元素后释放旧内存，不支持缩小。
 *       29. 调用 / 计算 / 写入：ranks[nr++] = proxyRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 31. 调用 / 计算 / 写入：*nranks = nr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 调用 / 计算 / 写入：*intermediateRanks = ranks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 33. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0452
 */
/* 中文逐项-函数 F0452：ncclTopoGetPxnRanks
 * 用途与位置：列出本 rank 网络路径可能依赖的中间 GPU rank，为共享代理资源提前准备。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 intermediateRanks（int** intermediateRanks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int* nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclRealloc；ncclTopoCheckGdr；ncclTopoGetNetDev。
 * 需要对照的边界/推进条件：system->nodes[NET].count == 0；rank<comm->nRanks；proxyRank == comm->rank；useGdr == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetPxnRanks(struct ncclComm* comm, int** intermediateRanks, int* nranks) {
/* 中文逐项-变量 V1222：system
 * system（struct ncclTopoSystem；函数 ncclTopoGetPxnRanks 内）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 初值/来源 system = comm->topo；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclTopoSystem* system = comm->topo;
  *nranks = 0;
  *intermediateRanks = NULL;
  if (system->nodes[NET].count == 0) return ncclSuccess;

/* 中文逐项-变量 V1223：nr
 * nr（int；函数 ncclTopoGetPxnRanks 内）：非对称 Fan 的运行时有效接收/发送邻居数。
 * 初值/来源 nr = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int nr = 0;
/* 中文逐项-变量 V1224：ranks
 * ranks（int；函数 ncclTopoGetPxnRanks 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ranks = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  int* ranks = NULL;
  for (/* 中文逐项-变量 V1225：rank
 * rank（int；函数 ncclTopoGetPxnRanks 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：rank<comm->nRanks；循环体与增量表达式决定每轮如何推进。
 */int rank=0; rank<comm->nRanks; rank++) {
/* 中文逐项-变量 V1226：netDev、proxyRank
 * netDev（int；函数 ncclTopoGetPxnRanks 内）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNetDev(comm, comm->rank, NULL, 0, rank, &netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 * proxyRank（int；函数 ncclTopoGetPxnRanks 内）：实际负责网络出口的代理 rank，PXN 时可能与发起通信的 GPU rank 不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNetDev(comm, comm->rank, NULL, 0, rank, &netDev,
 * &proxyRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int netDev, proxyRank;
    NCCLCHECK(ncclTopoGetNetDev(comm, comm->rank, NULL, 0, rank, &netDev, &proxyRank));
    if (proxyRank == comm->rank) continue;
/* 中文逐项-变量 V1227：useGdr
 * useGdr（int；函数 ncclTopoGetPxnRanks 内）：该方向协议缓冲是否允许 NIC 直接访问 GPU 内存，不表示 GPU 自己提交 verbs 请求。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCheckGdr(comm->topo, comm->busId, netDev, 1,
 * &useGdr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int useGdr;
    NCCLCHECK(ncclTopoCheckGdr(comm->topo, comm->busId, netDev, 1, &useGdr));
    if (useGdr == 0) continue;
/* 中文逐项-变量 V1228：found
 * found（int；函数 ncclTopoGetPxnRanks 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 found = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int found = 0;
    for (/* 中文逐项-变量 V1229：r
 * r（int；函数 ncclTopoGetPxnRanks 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nr；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nr; r++) {
      if (ranks[r] == proxyRank) found = 1;
    }
    if (!found) {
      NCCLCHECK(ncclRealloc(&ranks, nr, nr+1));
      ranks[nr++] = proxyRank;
    }
  }
  *nranks = nr;
  *intermediateRanks = ranks;
  return ncclSuccess;
}

/* 中文内部逻辑 F0453：ncclTopoComputePaths
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int t=0;；继续条件 t<NCCL_TOPO_NODE_TYPES；每轮更新 t++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 调用 / 计算 / 写入：ncclTopoRemovePathType(system, t);；释放每个硬件节点到指定目标类型的路径缓存，避免拓扑裁剪后继续使用旧下标。
 * 3. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 4. 循环：初始 int c=0;；继续条件 c<system->nodes[CPU].count；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSetPaths(system->nodes[CPU].nodes+c, system));；失败时可能提前return，成功才继续；从硬件节点做带带宽与路径类型约束的广度优先扩展，建立到该节点的路径缓存。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   8. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSetPaths(system->nodes[GPU].nodes+g, system));；失败时可能提前return，成功才继续；从硬件节点做带带宽与路径类型约束的广度优先扩展，建立到该节点的路径缓存。
 *   9. 循环：初始 int p=0;；继续条件 p<system->nodes[GPU].count；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     10. 准备局部状态：int p2p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     11. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCheckP2p(system, system->nodes[GPU].nodes[p].id, system->nodes[GPU].nodes[g].id, &p2p, NULL, NULL));；失败时可能提前return，成功才继续；结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。
 *     12. 条件判断：(p2p == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       13. 真分支：；仅条件为真执行以下子项。
 *         14. 准备局部状态：int cpu;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         15. 调用 / 计算 / 写入：NCCLCHECK(getLocalCpu(system, g, &cpu));；失败时可能提前return，成功才继续；为 GPU/设备选择其局部 CPU 节点，供亲和性和通过 CPU 绕行的路径计算。
 *         16. 调用 / 计算 / 写入：NCCLCHECK(addInterStep(system, CPU, cpu, GPU, p, GPU, g));；失败时可能提前return，成功才继续；用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。
 *   17. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   18. 条件判断：(peerInfos == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     19. 真分支：；仅条件为真执行以下子项。
 *       20. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   21. 准备局部状态：struct ncclPeerInfo* dstInfo = peerInfos+system->nodes[GPU].nodes[g].gpu.rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   22. 循环：初始 int p=0;；继续条件 p<system->nodes[GPU].count；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     23. 条件判断：(p == g)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       24. 真分支：；仅条件为真执行以下子项。
 *         25. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *     26. 准备局部状态：struct ncclPeerInfo* srcInfo = peerInfos+system->nodes[GPU].nodes[p].gpu.rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     27. 准备局部状态：int shm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     28. 调用 / 计算 / 写入：NCCLCHECK(ncclTransports[TRANSPORT_SHM].canConnect(&shm, system, NULL, srcInfo, dstInfo));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     29. 准备局部状态：int p2p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     30. 调用 / 计算 / 写入：NCCLCHECK(ncclTransports[TRANSPORT_P2P].canConnect(&p2p, system, NULL, srcInfo, dstInfo));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     31. 条件判断：(shm == 0 && p2p == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       32. 真分支：；仅条件为真执行以下子项。
 *         33. 调用 / 计算 / 写入：system->nodes[GPU].nodes[p].paths[GPU][g].count = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   34. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 35. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 36. 循环：初始 int n=0;；继续条件 n<system->nodes[NET].count；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   37. 准备局部状态：struct ncclTopoNode* netNode = system->nodes[NET].nodes+n;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   38. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSetPaths(netNode, system));；失败时可能提前return，成功才继续；从硬件节点做带带宽与路径类型约束的广度优先扩展，建立到该节点的路径缓存。
 *   39. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     40. 准备局部状态：struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     41. 条件判断：(ncclPxnDisable() != 1 && gpu->paths[NET][n].type > PATH_PXB)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       42. 真分支：；仅条件为真执行以下子项。
 *         43. 准备局部状态：int pxnGpu = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         44. 循环：初始 int p=0;；继续条件 p<system->nodes[GPU].count；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           45. 条件判断：(p == g)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             46. 真分支：；仅条件为真执行以下子项。
 *               47. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *           48. 准备局部状态：struct ncclTopoNode* peerNode = system->nodes[GPU].nodes+p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           49. 条件判断：(peerNode->paths[NET][n].type > PATH_PXB || peerNode->paths[GPU][g].type > PATH_NVL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             50. 真分支：；仅条件为真执行以下子项。
 *               51. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *           52. 调用 / 计算 / 写入：pxnGpu = p;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           53. 准备局部状态：int netDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           54. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetLocalNet(system, peerNode->gpu.rank, &netDev));；失败时可能提前return，成功才继续；按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。
 *           55. 条件判断：(netDev == netNode->id)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             56. 真分支：；仅条件为真执行以下子项。
 *               57. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         58. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         59. 条件判断：(pxnGpu != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           60. 真分支：；仅条件为真执行以下子项。
 *             61. 调用 / 计算 / 写入：NCCLCHECK(addInterStep(system, GPU, pxnGpu, GPU, g, NET, n));；失败时可能提前return，成功才继续；用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。
 *     62. 准备局部状态：int gdr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     63. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCheckGdr(system, system->nodes[GPU].nodes[g].id, netNode->id, 0, &gdr));；失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。
 *     64. 条件判断：(gdr == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       65. 真分支：；仅条件为真执行以下子项。
 *         66. 准备局部状态：int localCpu;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         67. 调用 / 计算 / 写入：NCCLCHECK(getLocalCpu(system, g, &localCpu));；失败时可能提前return，成功才继续；为 GPU/设备选择其局部 CPU 节点，供亲和性和通过 CPU 绕行的路径计算。
 *         68. 调用 / 计算 / 写入：NCCLCHECK(addInterStep(system, CPU, localCpu, NET, n, GPU, g));；失败时可能提前return，成功才继续；用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。
 *         69. 调用 / 计算 / 写入：NCCLCHECK(addInterStep(system, CPU, localCpu, GPU, g, NET, n));；失败时可能提前return，成功才继续；用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。
 *   70. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 71. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 72. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0453
 */
/* 中文逐项-函数 F0453：ncclTopoComputePaths
 * 用途与位置：计算 CPU/GPU/NET 路径，并按 P2P、GDR、PXN 策略替换不可直接使用的路径。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 peerInfos（struct ncclPeerInfo* peerInfos）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；addInterStep；getLocalCpu；ncclPxnDisable；ncclTopoCheckGdr；ncclTopoCheckP2p；ncclTopoGetLocalNet；ncclTopoRemovePathType；ncclTopoSetPaths；ncclTransports[TRANSPORT_P2P].canConnect；ncclTransports[TRANSPORT_SHM].canConnect。
 * 需要对照的边界/推进条件：t<NCCL_TOPO_NODE_TYPES；c<system->nodes[CPU].count；g<system->nodes[GPU].count；p<system->nodes[GPU].count。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoComputePaths(struct ncclTopoSystem* system, struct ncclPeerInfo* peerInfos) {
  // Precompute paths between GPUs/NICs.

  // Remove everything in case we're re-computing
  for (/* 中文逐项-变量 V1230：t
 * t（int；函数 ncclTopoComputePaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：t<NCCL_TOPO_NODE_TYPES；循环体与增量表达式决定每轮如何推进。
 */int t=0; t<NCCL_TOPO_NODE_TYPES; t++) ncclTopoRemovePathType(system, t);

  // Set direct paths from/to CPUs. We need them in many cases.
  for (/* 中文逐项-变量 V1231：c
 * c（int；函数 ncclTopoComputePaths 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<system->nodes[CPU].count；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<system->nodes[CPU].count; c++) {
    NCCLCHECK(ncclTopoSetPaths(system->nodes[CPU].nodes+c, system));
  }

  // Set direct paths from/to GPUs.
  for (/* 中文逐项-变量 V1232：g
 * g（int；函数 ncclTopoComputePaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
    // Compute paths to GPU g
    NCCLCHECK(ncclTopoSetPaths(system->nodes[GPU].nodes+g, system));

    // Update path when we don't want to / can't use GPU Direct P2P
    for (/* 中文逐项-变量 V1233：p
 * p（int；函数 ncclTopoComputePaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<system->nodes[GPU].count; p++) {
/* 中文逐项-变量 V1234：p2p
 * p2p（int；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCheckP2p(system, system->nodes[GPU].nodes[p].id,
 * system->nodes[GPU].nodes[g].id, &p2p, NULL, NULL));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int p2p;
      NCCLCHECK(ncclTopoCheckP2p(system, system->nodes[GPU].nodes[p].id, system->nodes[GPU].nodes[g].id, &p2p, NULL, NULL));
      if (p2p == 0) {
        // Divert all traffic through the CPU
/* 中文逐项-变量 V1235：cpu
 * cpu（int；函数 ncclTopoComputePaths 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getLocalCpu(system, g, &cpu));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int cpu;
        NCCLCHECK(getLocalCpu(system, g, &cpu));
        NCCLCHECK(addInterStep(system, CPU, cpu, GPU, p, GPU, g));
      }
    }

    if (peerInfos == NULL) continue;
    // Remove GPUs we can't talk to because of containers.
/* 中文逐项-变量 V1236：dstInfo
 * dstInfo（struct ncclPeerInfo；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 dstInfo = peerInfos+system->nodes[GPU].nodes[g].gpu.rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclPeerInfo* dstInfo = peerInfos+system->nodes[GPU].nodes[g].gpu.rank;
    for (/* 中文逐项-变量 V1237：p
 * p（int；函数 ncclTopoComputePaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<system->nodes[GPU].count; p++) {
      if (p == g) continue;
/* 中文逐项-变量 V1238：srcInfo
 * srcInfo（struct ncclPeerInfo；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 srcInfo = peerInfos+system->nodes[GPU].nodes[p].gpu.rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclPeerInfo* srcInfo = peerInfos+system->nodes[GPU].nodes[p].gpu.rank;
/* 中文逐项-变量 V1239：shm
 * shm（int；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTransports[TRANSPORT_SHM].canConnect(&shm, system, NULL, srcInfo,
 * dstInfo));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int shm;
      NCCLCHECK(ncclTransports[TRANSPORT_SHM].canConnect(&shm, system, NULL, srcInfo, dstInfo));
/* 中文逐项-变量 V1240：p2p
 * p2p（int；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTransports[TRANSPORT_P2P].canConnect(&p2p, system, NULL, srcInfo,
 * dstInfo));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int p2p;
      NCCLCHECK(ncclTransports[TRANSPORT_P2P].canConnect(&p2p, system, NULL, srcInfo, dstInfo));
      if (shm == 0 && p2p == 0) {
        // Mark this peer as inaccessible. We'll trim it later.
        system->nodes[GPU].nodes[p].paths[GPU][g].count = 0;
      }
    }
  }

  // Set direct paths from/to NICs.
  for (/* 中文逐项-变量 V1241：n
 * n（int；函数 ncclTopoComputePaths 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<system->nodes[NET].count；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<system->nodes[NET].count; n++) {
/* 中文逐项-变量 V1242：netNode
 * netNode（struct ncclTopoNode；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 netNode = system->nodes[NET].nodes+n；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* netNode = system->nodes[NET].nodes+n;
    NCCLCHECK(ncclTopoSetPaths(netNode, system));

    for (/* 中文逐项-变量 V1243：g
 * g（int；函数 ncclTopoComputePaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
      // Check whether we can access the NIC through another NVLink-connected GPU (PXN)
/* 中文逐项-变量 V1244：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoComputePaths 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = system->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
      if (ncclPxnDisable() != 1 && gpu->paths[NET][n].type > PATH_PXB) {
/* 中文逐项-变量 V1245：pxnGpu
 * pxnGpu（int；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pxnGpu = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
        int pxnGpu = -1;

        for (/* 中文逐项-变量 V1246：p
 * p（int；函数 ncclTopoComputePaths 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<system->nodes[GPU].count; p++) {
          if (p == g) continue;

          // PXN = PCI + NVLink.
/* 中文逐项-变量 V1247：peerNode
 * peerNode（struct ncclTopoNode；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 peerNode = system->nodes[GPU].nodes+p；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          struct ncclTopoNode* peerNode = system->nodes[GPU].nodes+p;
          if (peerNode->paths[NET][n].type > PATH_PXB || peerNode->paths[GPU][g].type > PATH_NVL) continue;

          pxnGpu = p;

/* 中文逐项-变量 V1248：netDev
 * netDev（int；函数 ncclTopoComputePaths 内）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetLocalNet(system, peerNode->gpu.rank, &netDev));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
          int netDev;
          NCCLCHECK(ncclTopoGetLocalNet(system, peerNode->gpu.rank, &netDev));
          // To ensure proper balancing, use preferably a local GPU which advertised that NIC as its preferred one.
          if (netDev == netNode->id) break;
        }
        if (pxnGpu != -1) {
          // We can use that GPU as relay to communicate with that NIC.
          // Only enabling it in the GPU->NIC direction for now to favor
          // receiving locally and sending remotely (consistent with net.cc)
          NCCLCHECK(addInterStep(system, GPU, pxnGpu, GPU, g, NET, n));
        }
      }
      // Update path when we dont want to / can't use GPU Direct RDMA.
/* 中文逐项-变量 V1249：gdr
 * gdr（int；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCheckGdr(system, system->nodes[GPU].nodes[g].id, netNode->id, 0,
 * &gdr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int gdr;
      NCCLCHECK(ncclTopoCheckGdr(system, system->nodes[GPU].nodes[g].id, netNode->id, 0, &gdr));
      if (gdr == 0) {
        // We cannot use GPU Direct RDMA, divert all traffic through the CPU local to the GPU
/* 中文逐项-变量 V1250：localCpu
 * localCpu（int；函数 ncclTopoComputePaths 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getLocalCpu(system, g, &localCpu));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int localCpu;
        NCCLCHECK(getLocalCpu(system, g, &localCpu));
        NCCLCHECK(addInterStep(system, CPU, localCpu, NET, n, GPU, g));
        NCCLCHECK(addInterStep(system, CPU, localCpu, GPU, g, NET, n));
      }
    }
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0454：ncclTopoTrimSystem
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int *domains;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int64_t *ids;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&domains, system->nodes[GPU].count));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&ids, system->nodes[GPU].count));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 准备局部状态：int myDomain = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 循环：初始 int g=0;；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 准备局部状态：struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 调用 / 计算 / 写入：domains[g] = g;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   9. 调用 / 计算 / 写入：ids[g] = gpu->id;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   10. 循环：初始 int p=0;；继续条件 p<g；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     11. 条件判断：(gpu->paths[GPU][p].count > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       12. 真分支：；仅条件为真执行以下子项。
 *         13. 调用 / 计算 / 写入：domains[g] = std::min(domains[g], domains[p]);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   15. 条件判断：(gpu->gpu.rank == comm->rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     16. 真分支：；仅条件为真执行以下子项。
 *       17. 调用 / 计算 / 写入：myDomain = domains[g];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 19. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 20. 循环：初始 int i=0;；继续条件 i<ngpus；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   21. 条件判断：(domains[i] == myDomain)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     22. 真分支：；仅条件为真执行以下子项。
 *       23. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   24. 准备局部状态：struct ncclTopoNode* gpu = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   25. 准备局部状态：int g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   26. 循环：初始 g=0；继续条件 g<system->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     27. 调用 / 计算 / 写入：gpu = system->nodes[GPU].nodes+g;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     28. 条件判断：(gpu->id == ids[i])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       29. 真分支：；仅条件为真执行以下子项。
 *         30. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *       31. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         32. 调用 / 计算 / 写入：gpu=NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   33. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   34. 条件判断：(gpu == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     35. 真分支：；仅条件为真执行以下子项。
 *       36. 调用 / 计算 / 写入：WARN("Could not find id %lx", ids[i]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       37. 调用 / 计算 / 写入：free(domains);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       38. 调用 / 计算 / 写入：free(ids);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       39. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   40. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoRemoveNode(system, GPU, g));；失败时可能提前return，成功才继续；删除硬件节点并修正指向它的连接及数组位置，避免后续搜索引用被裁掉对象。
 * 41. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 42. 条件判断：(system->nodes[GPU].count == comm->nRanks)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   43. 真分支：；仅条件为真执行以下子项。
 *     44. 循环：初始 int n=system->nodes[NET].count-1;；继续条件 n>=0；每轮更新 n--；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       45. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoRemoveNode(system, NET, n));；失败时可能提前return，成功才继续；删除硬件节点并修正指向它的连接及数组位置，避免后续搜索引用被裁掉对象。
 *     46. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 47. 调用 / 计算 / 写入：free(domains);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 48. 调用 / 计算 / 写入：free(ids);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 49. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0454
 */
/* 中文逐项-函数 F0454：ncclTopoTrimSystem
 * 用途与位置：删除不属于当前通信可达域的硬件节点，并在完全机内通信时裁剪不需要的网络对象。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；free；ncclCalloc；ncclTopoRemoveNode；std::min。
 * 需要对照的边界/推进条件：g<system->nodes[GPU].count；p<g；gpu->paths[GPU][p].count > 0；gpu->gpu.rank == comm->rank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoTrimSystem(struct ncclTopoSystem* system, struct ncclComm* comm) {
/* 中文逐项-变量 V1251：domains
 * domains（int；函数 ncclTopoTrimSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&domains, system->nodes[GPU].count));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int *domains;
/* 中文逐项-变量 V1252：ids
 * ids（int64_t；函数 ncclTopoTrimSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&ids, system->nodes[GPU].count));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int64_t *ids;
  NCCLCHECK(ncclCalloc(&domains, system->nodes[GPU].count));
  NCCLCHECK(ncclCalloc(&ids, system->nodes[GPU].count));
/* 中文逐项-变量 V1253：myDomain
 * myDomain（int；函数 ncclTopoTrimSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 myDomain = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int myDomain = 0;
  for (/* 中文逐项-变量 V1254：g
 * g（int；函数 ncclTopoTrimSystem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<system->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<system->nodes[GPU].count; g++) {
/* 中文逐项-变量 V1255：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoTrimSystem 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = system->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
    domains[g] = g;
    ids[g] = gpu->id;
    for (/* 中文逐项-变量 V1256：p
 * p（int；函数 ncclTopoTrimSystem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<g；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<g; p++) {
      if (gpu->paths[GPU][p].count > 0) {
        domains[g] = std::min(domains[g], domains[p]);
      }
    }
    if (gpu->gpu.rank == comm->rank) myDomain = domains[g];
  }

/* 中文逐项-变量 V1257：ngpus
 * ngpus（int；函数 ncclTopoTrimSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;
  for (/* 中文逐项-变量 V1258：i
 * i（int；函数 ncclTopoTrimSystem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ngpus；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ngpus; i++) {
    if (domains[i] == myDomain) continue;
/* 中文逐项-变量 V1259：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoTrimSystem 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    struct ncclTopoNode* gpu = NULL;
/* 中文逐项-变量 V1260：g
 * g（int；函数 ncclTopoTrimSystem 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：for (g=0; g<system->nodes[GPU].count ; g++) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int g;
    for (g=0; g<system->nodes[GPU].count /* This one varies over the loops */; g++) {
      gpu = system->nodes[GPU].nodes+g;
      if (gpu->id == ids[i]) break; else gpu=NULL;
    }
    if (gpu == NULL) {
      WARN("Could not find id %lx", ids[i]);
      free(domains);
      free(ids);
      return ncclInternalError;
    }
    NCCLCHECK(ncclTopoRemoveNode(system, GPU, g));
  }

  if (system->nodes[GPU].count == comm->nRanks) {
    for (/* 中文逐项-变量 V1261：n
 * n（int；函数 ncclTopoTrimSystem 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = system->nodes[NET].count-1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：n>=0；循环体与增量表达式决定每轮如何推进。
 */int n=system->nodes[NET].count-1; n>=0; n--)
      NCCLCHECK(ncclTopoRemoveNode(system, NET, n));
  }
  free(domains);
  free(ids);
  return ncclSuccess;
}

/* 中文内部逻辑 F0455：ncclTopoFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int t=0;；继续条件 t<NCCL_TOPO_NODE_TYPES；每轮更新 t++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 调用 / 计算 / 写入：ncclTopoRemovePathType(system, t);；释放每个硬件节点到指定目标类型的路径缓存，避免拓扑裁剪后继续使用旧下标。
 * 3. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 4. 调用 / 计算 / 写入：free(system);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0455
 */
/* 中文逐项-函数 F0455：ncclTopoFree
 * 用途与位置：回收硬件系统的所有路径缓存和系统对象，本身不关闭 transport 已建立的连接。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：free；ncclTopoRemovePathType。
 * 需要对照的边界/推进条件：t<NCCL_TOPO_NODE_TYPES。各条件所属分支见紧邻函数体。
 */
void ncclTopoFree(struct ncclTopoSystem* system) {
  for (/* 中文逐项-变量 V1262：t
 * t（int；函数 ncclTopoFree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：t<NCCL_TOPO_NODE_TYPES；循环体与增量表达式决定每轮如何推进。
 */int t=0; t<NCCL_TOPO_NODE_TYPES; t++) ncclTopoRemovePathType(system, t);
  free(system);
}

/* 中文逐项-参数getter P0011：NChannelsPerNetPeer
 * 生成函数 ncclParamNChannelsPerNetPeer()，读取 NCCL_NCHANNELS_PER_NET_PEER，编译默认值为 2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(NChannelsPerNetPeer, "NCHANNELS_PER_NET_PEER", 2);

/* 中文内部逻辑 F0456：ncclTopoGetNchannels
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int peer;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclTopoLinkList* path = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(ncclTopoRankToIndex(system, peerRank, &peer) == ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 条件判断：(g == peer)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       6. 真分支：；仅条件为真执行以下子项。
 *         7. 调用 / 计算 / 写入：*nChannels = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     9. 调用 / 计算 / 写入：path = system->nodes[GPU].nodes[peer].paths[GPU]+g;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 条件判断：(path->type == PATH_NVL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       11. 真分支：；仅条件为真执行以下子项。
 *         12. 准备局部状态：float nvlWidth = ncclTopoNVLinkSpeed(system->nodes[GPU].nodes[g].gpu.cudaCompCap);；按 GPU 计算能力选择该代 NVLink 单链估计速率，供路径通道预算使用。
 *         13. 调用 / 计算 / 写入：*nChannels = 2*std::max(1, (int)(path->width / nvlWidth));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       14. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         15. 调用 / 计算 / 写入：*nChannels = 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   16. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     17. 调用 / 计算 / 写入：*nChannels = ncclParamNChannelsPerNetPeer();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0456
 */
/* 中文逐项-函数 F0456：ncclTopoGetNchannels
 * 用途与位置：按 peer 的本地/远端属性和 NVLink 带宽估计点对点通道预算，self 特殊处理。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 g（int g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 peerRank（int peerRank）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 nChannels（int* nChannels）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputeP2pChannels。
 * 本体直接出现的调用/包装：ncclParamNChannelsPerNetPeer；ncclTopoNVLinkSpeed；ncclTopoRankToIndex；std::max。
 * 需要对照的边界/推进条件：ncclTopoRankToIndex(system, peerRank, &peer) == ncclSuccess；g == peer；path->type ==
 * PATH_NVL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclTopoGetNchannels(struct ncclTopoSystem* system, int g /*local gpu index*/, int peerRank, int* nChannels) {
/* 中文逐项-变量 V1263：peer
 * peer（int；函数 ncclTopoGetNchannels 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 此处未显式给初值；后续首个文本使用点：if (ncclTopoRankToIndex(system, peerRank, &peer) == ncclSuccess) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int peer;
/* 中文逐项-变量 V1264：path
 * path（struct ncclTopoLinkList；函数 ncclTopoGetNchannels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 path = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclTopoLinkList* path = NULL;
  if (ncclTopoRankToIndex(system, peerRank, &peer) == ncclSuccess) {
    // Same rank
    if (g == peer) {
      *nChannels = -1;
      return ncclSuccess;
    }
    // Local rank
    path = system->nodes[GPU].nodes[peer].paths[GPU]+g;
    if (path->type == PATH_NVL) {
/* 中文逐项-变量 V1265：nvlWidth
 * nvlWidth（float；函数 ncclTopoGetNchannels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvlWidth = ncclTopoNVLinkSpeed(system->nodes[GPU].nodes[g].gpu.cudaCompCap)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      float nvlWidth = ncclTopoNVLinkSpeed(system->nodes[GPU].nodes[g].gpu.cudaCompCap);
      *nChannels = 2*std::max(1, (int)(path->width / nvlWidth));
    } else {
      *nChannels = 2;
    }
  } else {
    // Remote rank, use network
    *nChannels = ncclParamNChannelsPerNetPeer();
  }
  return ncclSuccess;
}

/* 中文逐项-参数getter P0012：MinP2pNChannels
 * 生成函数 ncclParamMinP2pNChannels()，读取 NCCL_MIN_P2P_NCHANNELS，编译默认值为 1。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(MinP2pNChannels, "MIN_P2P_NCHANNELS", 1);
/* 中文逐项-参数getter P0013：MaxP2pNChannels
 * 生成函数 ncclParamMaxP2pNChannels()，读取 NCCL_MAX_P2P_NCHANNELS，编译默认值为 MAXCHANNELS。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(MaxP2pNChannels, "MAX_P2P_NCHANNELS", MAXCHANNELS);

/* 中文内部逻辑 F0457：nextPow2
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int pow2 = 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 ；继续条件 (pow2 < v)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 调用 / 计算 / 写入：pow2 <<= 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 5. 函数返回：return pow2;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0457
 */
/* 中文逐项-函数 F0457：nextPow2
 * 用途与位置：向上取最小二次幂，为 P2P 通道映射和位反转分布提供便于计算的容量。
 * 参数 v（int v）：LL行的两个64位或int4整行视图，用向量汇编同时读写数据和标记。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclTopoComputeP2pChannels。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：pow2 < v。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return pow2;。CHECK类宏还可能提前返回/跳转。
 */
static int nextPow2(int v) {
/* 中文逐项-变量 V1266：pow2
 * pow2（int；函数 nextPow2 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pow2 = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int pow2 = 1;
  while (pow2 < v) pow2 <<= 1;
  return pow2;
}

/* 中文导读：
 * 分别计算 p2pnChannels（全局通道空间）与 p2pnChannelsPerPeer（单 peer 的通道预算）。
 * 遍历本地 GPU/所有 peer 取限制，再向上对齐到 2 的幂，并用位反转偏移让通道分布更均匀。
 * 日志里的 8 p2p channels、2 per peer 不是 8×2 个独立 channel；具体任务还会按大小调整拆分。
 */
/* 中文精读：图示绑定 V3.3 F0458 ncclTopoComputeP2pChannels
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0458
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0094] ncclTopoComputeP2pChannels：计算P2P总channel空间与每peer预算；预算不是已经对所有peer建立了相应连接（直接对应）
 *   图：28-channel-generation, 25-p2p
 *   定位：#symbol=F0458&binding=B0094
 */
/* 中文内部逻辑 F0458：ncclTopoComputeP2pChannels
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：comm->p2pnChannels = std::min(comm->nChannels, (int)ncclParamMaxP2pNChannels());；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 * 2. 调用 / 计算 / 写入：comm->p2pnChannels = std::max(comm->p2pnChannels, (int)ncclParamMinP2pNChannels());；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 准备局部状态：int minChannels = comm->p2pnChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 int g=0;；继续条件 g<comm->topo->nodes[GPU].count；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 循环：初始 int r=0;；继续条件 r<comm->nRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     6. 准备局部状态：int nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetNchannels(comm->topo, g, r, &nChannels));；失败时可能提前return，成功才继续；按 peer 的本地/远端属性和 NVLink 带宽估计点对点通道预算，self 特殊处理。
 *     8. 条件判断：(nChannels >= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       9. 真分支：；仅条件为真执行以下子项。
 *         10. 调用 / 计算 / 写入：minChannels = std::min(minChannels, nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 调用 / 计算 / 写入：comm->p2pnChannelsPerPeer = nextPow2(minChannels);；向上取最小二次幂，为 P2P 通道映射和位反转分布提供便于计算的容量。
 * 14. 调用 / 计算 / 写入：comm->p2pnChannels = nextPow2(comm->p2pnChannels);；向上取最小二次幂，为 P2P 通道映射和位反转分布提供便于计算的容量。
 * 15. 循环：初始 int c=comm->nChannels;；继续条件 c<comm->p2pnChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   16. 调用 / 计算 / 写入：NCCLCHECK(initChannel(comm, c));；失败时可能提前return，成功才继续；为单条逻辑 channel 分配环顺序表、peer 连接表和 GPU 可读取的工作描述符 FIFO。
 * 17. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 18. 循环：初始 int c=0;；继续条件 c<comm->p2pnChannelsPerPeer；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   19. 准备局部状态：int mirror = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   20. 循环：初始 int b=1, mb=(comm->p2pnChannels>>1);；继续条件 b<comm->p2pnChannels；每轮更新 b<<=1, mb>>=1；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     21. 条件判断：(c & b)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       22. 真分支：；仅条件为真执行以下子项。
 *         23. 调用 / 计算 / 写入：mirror |= mb;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   24. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   25. 调用 / 计算 / 写入：comm->p2pChannels[c] = mirror;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 26. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 27. 调用 / 计算 / 写入：INFO(NCCL_INIT, "%d coll channels, %d p2p channels, %d p2p channels per peer", comm->nChannels, comm->p2pnChannels, comm->p2pnChannelsPerPeer);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 28. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0458
 */
/* 中文逐项-函数 F0458：ncclTopoComputeP2pChannels
 * 用途与位置：收敛所有本地 GPU 到 peer 的通道预算，计算全局 P2P 空间与每 peer 数量并生成位反转偏移。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；initChannel；ncclParamMaxP2pNChannels；ncclParamMinP2pNChannels；ncclTopoGetNchannels；nextPow2；std::max；std::min。
 * 需要对照的边界/推进条件：g<comm->topo->nodes[GPU].count；r<comm->nRanks；nChannels >= 0；c<comm->p2pnChannels。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoComputeP2pChannels(struct ncclComm* comm) {
  comm->p2pnChannels = std::min(comm->nChannels, (int)ncclParamMaxP2pNChannels());
  comm->p2pnChannels = std::max(comm->p2pnChannels, (int)ncclParamMinP2pNChannels());
/* 中文逐项-变量 V1267：minChannels
 * minChannels（int；函数 ncclTopoComputeP2pChannels 内）：图/后端允许的逻辑通道上限和下限，受容量与配置共同约束。
 * 初值/来源 minChannels = comm->p2pnChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int minChannels = comm->p2pnChannels;
  // We need to loop through all local GPUs to have a global picture
  for (/* 中文逐项-变量 V1268：g
 * g（int；函数 ncclTopoComputeP2pChannels 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<comm->topo->nodes[GPU].count；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<comm->topo->nodes[GPU].count; g++) {
    for (/* 中文逐项-变量 V1269：r
 * r（int；函数 ncclTopoComputeP2pChannels 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->nRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->nRanks; r++) {
/* 中文逐项-变量 V1270：nChannels
 * nChannels（int；函数 ncclTopoComputeP2pChannels 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNchannels(comm->topo, g, r, &nChannels));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int nChannels;
      NCCLCHECK(ncclTopoGetNchannels(comm->topo, g, r, &nChannels));
      if (nChannels >= 0) minChannels = std::min(minChannels, nChannels);
    }
  }

/* 中文精读：8 个 P2P channel、每 peer 2 个的具体映射示例
 * 向上对齐到 2 的幂后，以 3 位反转生成偏移：全局 8 条时，c=0 -> 0，c=1 -> 4。
 * 若单 peer 预算为 2，base=1 的这个 peer 就用 (1+0)%8=1 和 (1+4)%8=5。
 * 另一个 peer 的 base 也可能为 1，复用 channel 1/5，但 peers[peer] 的 connector 不同。
 * Group 再按消息大小决定实际拆多少块，chunk 号对 perPeer 取模来轮转这些 channel。
 */
  // Round to next pow2 nChannelsPerPeer and nChannels
  comm->p2pnChannelsPerPeer = nextPow2(minChannels);
  comm->p2pnChannels = nextPow2(comm->p2pnChannels);

  // Init channels that weren't used so far
  for (/* 中文逐项-变量 V1271：c
 * c（int；函数 ncclTopoComputeP2pChannels 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = comm->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：c<comm->p2pnChannels；循环体与增量表达式决定每轮如何推进。
 */int c=comm->nChannels; c<comm->p2pnChannels; c++) NCCLCHECK(initChannel(comm, c));

  // We want to spread channels used when there aren't many and progressively
  // fill the whole space of nChannels. To do so we mirror the bits in the
  // nChannels space.
  for (/* 中文逐项-变量 V1272：c
 * c（int；函数 ncclTopoComputeP2pChannels 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->p2pnChannelsPerPeer；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->p2pnChannelsPerPeer; c++) {
/* 中文逐项-变量 V1273：mirror
 * mirror（int；函数 ncclTopoComputeP2pChannels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mirror = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int mirror = 0;
    for (/* 中文逐项-变量 V1274：b、mb
 * b（int；函数 ncclTopoComputeP2pChannels 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * mb（int；函数 ncclTopoComputeP2pChannels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mb = (comm->p2pnChannels>>1)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：b<comm->p2pnChannels；循环体与增量表达式决定每轮如何推进。
 */int b=1, mb=(comm->p2pnChannels>>1); b<comm->p2pnChannels; b<<=1, mb>>=1) if (c & b) mirror |= mb;
    comm->p2pChannels[c] = mirror;
  }
  INFO(NCCL_INIT, "%d coll channels, %d p2p channels, %d p2p channels per peer", comm->nChannels, comm->p2pnChannels, comm->p2pnChannelsPerPeer);
  return ncclSuccess;
}

/* 中文内部逻辑 F0459：ncclTopoGetNvbGpus
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int ngpus = system->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(ranks, ngpus));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 准备局部状态：int nvbGpus = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 int g=0;；继续条件 g<ngpus；每轮更新 g++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 准备局部状态：struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   6. 条件判断：(gpu->gpu.rank != rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     7. 真分支：；仅条件为真执行以下子项。
 *       8. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   9. 循环：初始 int p=0;；继续条件 p<ngpus；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     10. 条件判断：(gpu->paths[GPU][p].type == PATH_NVB)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       11. 真分支：；仅条件为真执行以下子项。
 *         12. 调用 / 计算 / 写入：(*ranks)[nvbGpus++] = system->nodes[GPU].nodes[p].gpu.rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 15. 调用 / 计算 / 写入：*nranks = nvbGpus;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__paths_cc.md#f0459
 */
/* 中文逐项-函数 F0459：ncclTopoGetNvbGpus
 * 用途与位置：找出可经一个中间 GPU 走 NVLink 桥接的 peer，为特定机内路径预连接提供候选。
 * 参数 system（struct ncclTopoSystem* system）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 nranks（int* nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 ranks（int** ranks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc。
 * 需要对照的边界/推进条件：g<ngpus；gpu->gpu.rank != rank；p<ngpus；gpu->paths[GPU][p].type == PATH_NVB。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetNvbGpus(struct ncclTopoSystem* system, int rank, int* nranks, int** ranks) {
/* 中文逐项-变量 V1275：ngpus
 * ngpus（int；函数 ncclTopoGetNvbGpus 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ngpus = system->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ngpus = system->nodes[GPU].count;
  NCCLCHECK(ncclCalloc(ranks, ngpus));
/* 中文逐项-变量 V1276：nvbGpus
 * nvbGpus（int；函数 ncclTopoGetNvbGpus 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvbGpus = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int nvbGpus = 0;
  for (/* 中文逐项-变量 V1277：g
 * g（int；函数 ncclTopoGetNvbGpus 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 g = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：g<ngpus；循环体与增量表达式决定每轮如何推进。
 */int g=0; g<ngpus; g++) {
/* 中文逐项-变量 V1278：gpu
 * gpu（struct ncclTopoNode；函数 ncclTopoGetNvbGpus 内）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 初值/来源 gpu = system->nodes[GPU].nodes+g；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
    if (gpu->gpu.rank != rank) continue;
    for (/* 中文逐项-变量 V1279：p
 * p（int；函数 ncclTopoGetNvbGpus 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<ngpus；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<ngpus; p++) {
      if (gpu->paths[GPU][p].type == PATH_NVB) {
        (*ranks)[nvbGpus++] = system->nodes[GPU].nodes[p].gpu.rank;
      }
    }
  }
  *nranks = nvbGpus;
  return ncclSuccess;
}
