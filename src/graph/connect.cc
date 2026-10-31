/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "graph.h"
#include "trees.h"
#include "rings.h"
#include "topo.h"

/******************************************************************/
/********************* Internode connection ***********************/
/******************************************************************/

/* 中文精读：先构造机内片段，再交换跨机拼接信息
 * 搜索图 intra 给出每条 channel 的机内 GPU 顺序，inter 给出网络端点。
 * Preset 把这些顺序写成局部 ring.prev/next 与 tree 上下关系，并导出 ncclTopoRanks。
 * 各 rank 交换该摘要后，Postset 才知道每台机器应与哪些远端端点拼接。
 * 先本地搜索、后全局拼接，避免每个 rank 独立猜测一个互不一致的完整集群拓扑。
 */
/* 中文内部逻辑 F0429：ncclTopoPreset
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int rank = comm->rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int localRanks = comm->topo->nodes[GPU].count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int nChannels = comm->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 循环：初始 int c=0;；继续条件 c<nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 准备局部状态：struct ncclChannel* channel = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   6. 调用 / 计算 / 写入：channel->ring.prev = channel->ring.next = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   7. 调用 / 计算 / 写入：channel->tree.up = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   8. 循环：初始 int i=0;；继续条件 i<NCCL_MAX_TREE_ARITY；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     9. 调用 / 计算 / 写入：channel->tree.down[i] = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   11. 调用 / 计算 / 写入：channel->collTree.out = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   12. 调用 / 计算 / 写入：channel->collTree.headRank = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   13. 调用 / 计算 / 写入：channel->collTree.nHeads = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   14. 调用 / 计算 / 写入：channel->collTree.shift = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   15. 循环：初始 int i=0;；继续条件 i<NCCL_MAX_DIRECT_ARITY；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     16. 调用 / 计算 / 写入：channel->collTree.up[i] = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   17. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   18. 循环：初始 int i=0;；继续条件 i<NCCL_MAX_DIRECT_ARITY；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     19. 调用 / 计算 / 写入：channel->collTree.down[i] = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   21. 准备局部状态：int* ringIntra = ringGraph->intra+c*localRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   22. 准备局部状态：int* treeIntra = treeGraph->intra+c*localRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   23. 循环：初始 int i=0;；继续条件 i<localRanks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     24. 条件判断：(ringIntra[i] == rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       25. 真分支：；仅条件为真执行以下子项。
 *         26. 调用 / 计算 / 写入：topoRanks->ringRecv[c] = ringIntra[0];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         27. 调用 / 计算 / 写入：topoRanks->ringSend[c] = ringIntra[localRanks-1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         28. 调用 / 计算 / 写入：channel->ring.prev = (i == 0) ? -1 : ringIntra[i-1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         29. 调用 / 计算 / 写入：channel->ring.next = (i == localRanks-1) ? -1 : ringIntra[i+1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     30. 条件判断：(treeIntra[i] == rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       31. 真分支：；仅条件为真执行以下子项。
 *         32. 准备局部状态：int parentIndex = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         33. 准备局部状态：int child0Index = treeGraph->pattern == NCCL_TOPO_PATTERN_TREE ? 0 : 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         34. 准备局部状态：int child1Index = treeGraph->pattern == NCCL_TOPO_PATTERN_SPLIT_TREE ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         35. 调用 / 计算 / 写入：topoRanks->treeToParent[c] = treeIntra[parentIndex];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         36. 调用 / 计算 / 写入：topoRanks->treeToChild0[c] = treeIntra[child0Index];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         37. 调用 / 计算 / 写入：topoRanks->treeToChild1[c] = treeIntra[child1Index];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         38. 调用 / 计算 / 写入：channel->tree.up = i == 0 ? -1 : treeIntra[i-1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         39. 调用 / 计算 / 写入：channel->tree.down[0] = i == localRanks-1 ? -1 : treeIntra[i+1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   40. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   41. 调用 / 计算 / 写入：topoRanks->ringPrev[c] = channel->ring.prev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   42. 调用 / 计算 / 写入：topoRanks->ringNext[c] = channel->ring.next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 43. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 44. 准备局部状态：struct ncclChannel* channel0 = comm->channels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 45. 准备局部状态：struct ncclChannel* channel1 = channel0+nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 46. 调用 / 计算 / 写入：memcpy(channel1, channel0, nChannels*sizeof(struct ncclChannel));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 47. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0429
 */
/* 中文逐项-函数 F0429：ncclTopoPreset
 * 用途与位置：将本机搜索图转成可交换的局部环/树端点摘要，为跨节点拼接准备元数据。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 treeGraph（struct ncclTopoGraph* treeGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ringGraph（struct ncclTopoGraph* ringGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 topoRanks（struct ncclTopoRanks* topoRanks）：该rank的局部拓扑端点摘要，初始化AllGather后供全局Ring/Tree拼接。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：memcpy。
 * 需要对照的边界/推进条件：c<nChannels；i<NCCL_MAX_TREE_ARITY；i<NCCL_MAX_DIRECT_ARITY；i<NCCL_MAX_DIRECT_ARITY。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoPreset(struct ncclComm* comm,
    struct ncclTopoGraph* treeGraph, struct ncclTopoGraph* ringGraph,
    struct ncclTopoRanks* topoRanks) {
/* 中文逐项-变量 V1075：rank
 * rank（int；函数 ncclTopoPreset 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = comm->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = comm->rank;
/* 中文逐项-变量 V1076：localRanks
 * localRanks（int；函数 ncclTopoPreset 内）：当前节点内参与此 communicator 的 rank 数。
 * 初值/来源 localRanks = comm->topo->nodes[GPU].count；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int localRanks = comm->topo->nodes[GPU].count;
/* 中文逐项-变量 V1077：nChannels
 * nChannels（int；函数 ncclTopoPreset 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = comm->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nChannels = comm->nChannels;

  for (/* 中文逐项-变量 V1078：c
 * c（int；函数 ncclTopoPreset 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<nChannels; c++) {
/* 中文逐项-变量 V1079：channel
 * channel（struct ncclChannel；函数 ncclTopoPreset 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+c;
    channel->ring.prev = channel->ring.next = -1;
    channel->tree.up = -1;
    for (/* 中文逐项-变量 V1080：i
 * i（int；函数 ncclTopoPreset 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_MAX_TREE_ARITY；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_MAX_TREE_ARITY; i++) channel->tree.down[i] = -1;
    channel->collTree.out = -1;
    channel->collTree.headRank = -1;
    channel->collTree.nHeads = 0;
    channel->collTree.shift = 0;
    for (/* 中文逐项-变量 V1081：i
 * i（int；函数 ncclTopoPreset 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_MAX_DIRECT_ARITY；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_MAX_DIRECT_ARITY; i++) channel->collTree.up[i] = -1;
    for (/* 中文逐项-变量 V1082：i
 * i（int；函数 ncclTopoPreset 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_MAX_DIRECT_ARITY；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_MAX_DIRECT_ARITY; i++) channel->collTree.down[i] = -1;

/* 中文逐项-变量 V1083：ringIntra
 * ringIntra（int；函数 ncclTopoPreset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ringIntra = ringGraph->intra+c*localRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int* ringIntra = ringGraph->intra+c*localRanks;
/* 中文逐项-变量 V1084：treeIntra
 * treeIntra（int；函数 ncclTopoPreset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 treeIntra = treeGraph->intra+c*localRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int* treeIntra = treeGraph->intra+c*localRanks;

    for (/* 中文逐项-变量 V1085：i
 * i（int；函数 ncclTopoPreset 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<localRanks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<localRanks; i++) {
      if (ringIntra[i] == rank) {
        topoRanks->ringRecv[c] = ringIntra[0];
        topoRanks->ringSend[c] = ringIntra[localRanks-1];
        channel->ring.prev = (i == 0) ? -1 : ringIntra[i-1];
        channel->ring.next = (i == localRanks-1) ? -1 : ringIntra[i+1];
      }
      if (treeIntra[i] == rank) {
/* 中文逐项-变量 V1086：parentIndex
 * parentIndex（int；函数 ncclTopoPreset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 parentIndex = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
        int parentIndex = 0;
/* 中文逐项-变量 V1087：child0Index
 * child0Index（int；函数 ncclTopoPreset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 child0Index = treeGraph->pattern == NCCL_TOPO_PATTERN_TREE ? 0 : 1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        int child0Index = treeGraph->pattern == NCCL_TOPO_PATTERN_TREE ? 0 : 1;
/* 中文逐项-变量 V1088：child1Index
 * child1Index（int；函数 ncclTopoPreset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 child1Index = treeGraph->pattern == NCCL_TOPO_PATTERN_SPLIT_TREE ? 1 : 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        int child1Index = treeGraph->pattern == NCCL_TOPO_PATTERN_SPLIT_TREE ? 1 : 0;

        topoRanks->treeToParent[c] = treeIntra[parentIndex];
        topoRanks->treeToChild0[c] = treeIntra[child0Index];
        topoRanks->treeToChild1[c] = treeIntra[child1Index];
        channel->tree.up         = i == 0 ? -1 : treeIntra[i-1];
        channel->tree.down[0]    = i == localRanks-1 ? -1 : treeIntra[i+1];
      }
    }
    topoRanks->ringPrev[c] = channel->ring.prev;
    topoRanks->ringNext[c] = channel->ring.next;
  }
  // Duplicate channels rings/trees
/* 中文逐项-变量 V1089：channel0
 * channel0（struct ncclChannel；函数 ncclTopoPreset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channel0 = comm->channels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclChannel* channel0 = comm->channels;
/* 中文逐项-变量 V1090：channel1
 * channel1（struct ncclChannel；函数 ncclTopoPreset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channel1 = channel0+nChannels；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclChannel* channel1 = channel0+nChannels;
  memcpy(channel1, channel0, nChannels*sizeof(struct ncclChannel));
  return ncclSuccess;
}

/* 中文精读：把多台机器的局部链串成全局环
 * 每节点机内 ring 片段有入口 ringRecv 和出口 ringSend。
 * 按节点顺序，把本节点出口接到下一节点入口，同时补全相应 prev/next。
 * 每个 rank 最终只需要知道本 channel 的直接前驱和后继；完整顺序再由 BuildRings 检查生成。
 * 环中节点很多并不意味着每个 rank 都直接连接所有 rank。
 */
/* 中文内部逻辑 F0430：connectRings
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int nChannels = comm->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int nNodes = comm->nNodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 int c=0;；继续条件 c<nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 准备局部状态：int* recv = ringRecv+c*comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   5. 准备局部状态：int* send = ringSend+c*comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   6. 准备局部状态：int* prev = ringPrev+c*comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   7. 准备局部状态：int* next = ringNext+c*comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 准备局部状态：struct ncclChannel* channel0 = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   9. 准备局部状态：struct ncclChannel* channel1 = channel0+nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   10. 循环：初始 int n=0;；继续条件 n<nNodes；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     11. 准备局部状态：int recvRank = recv[firstRanks[n]];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     12. 准备局部状态：int prevSendRank = send[firstRanks[(n-1+nNodes)%nNodes]];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     13. 调用 / 计算 / 写入：prev[recvRank] = prevSendRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     14. 条件判断：(comm->rank == recvRank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       15. 真分支：；仅条件为真执行以下子项。
 *         16. 调用 / 计算 / 写入：channel0->ring.prev = prevSendRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         17. 调用 / 计算 / 写入：channel1->ring.prev = prevSendRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     18. 准备局部状态：int sendRank = send[firstRanks[n]];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     19. 准备局部状态：int nextRecvRank = recv[firstRanks[(n+1)%nNodes]];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     20. 调用 / 计算 / 写入：next[sendRank] = nextRecvRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     21. 条件判断：(comm->rank == sendRank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       22. 真分支：；仅条件为真执行以下子项。
 *         23. 调用 / 计算 / 写入：channel0->ring.next = nextRecvRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         24. 调用 / 计算 / 写入：channel1->ring.next = nextRecvRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   25. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   26. 调用 / 计算 / 写入：TRACE(NCCL_GRAPH, "Ring %d : %d -> %d -> %d", c, channel0->ring.prev, comm->rank, channel0->ring.next);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   27. 调用 / 计算 / 写入：TRACE(NCCL_GRAPH, "Ring %d : %d -> %d -> %d", c+nChannels, channel1->ring.prev, comm->rank, channel1->ring.next);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 28. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 29. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0430
 */
/* 中文逐项-函数 F0430：connectRings
 * 用途与位置：把各节点局部 GPU 链的出口接到下一节点入口，补全完整环的前驱/后继。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 ringRecv（int* ringRecv）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 参数 ringSend（int* ringSend）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 参数 ringPrev（int* ringPrev）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 参数 ringNext（int* ringNext）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 参数 firstRanks（int* firstRanks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::ncclTopoPostset。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：c<nChannels；n<nNodes；comm->rank == recvRank；comm->rank == sendRank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t connectRings(struct ncclComm* comm, int* ringRecv, int* ringSend, int* ringPrev, int* ringNext, int* firstRanks) {
/* 中文逐项-变量 V1091：nChannels
 * nChannels（int；函数 connectRings 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = comm->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nChannels = comm->nChannels;
/* 中文逐项-变量 V1092：nNodes
 * nNodes（int；函数 connectRings 内）：当前 communicator 跨越的节点数量；节点编号不等于 GPU rank。
 * 初值/来源 nNodes = comm->nNodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nNodes = comm->nNodes;
  for (/* 中文逐项-变量 V1093：c
 * c（int；函数 connectRings 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<nChannels; c++) {
/* 中文逐项-变量 V1094：recv
 * recv（int；函数 connectRings 内）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 初值/来源 recv = ringRecv+c*comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int* recv = ringRecv+c*comm->nRanks;
/* 中文逐项-变量 V1095：send
 * send（int；函数 connectRings 内）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 初值/来源 send = ringSend+c*comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int* send = ringSend+c*comm->nRanks;
/* 中文逐项-变量 V1096：prev
 * prev（int；函数 connectRings 内）：环前后继或链表前后项，准确含义由所属 ring/队列类型决定。
 * 初值/来源 prev = ringPrev+c*comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int* prev = ringPrev+c*comm->nRanks;
/* 中文逐项-变量 V1097：next
 * next（int；函数 connectRings 内）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 初值/来源 next = ringNext+c*comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int* next = ringNext+c*comm->nRanks;
/* 中文逐项-变量 V1098：channel0
 * channel0（struct ncclChannel；函数 connectRings 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channel0 = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel0 = comm->channels+c;
/* 中文逐项-变量 V1099：channel1
 * channel1（struct ncclChannel；函数 connectRings 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channel1 = channel0+nChannels；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclChannel* channel1 = channel0+nChannels;
    for (/* 中文逐项-变量 V1100：n
 * n（int；函数 connectRings 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<nNodes；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<nNodes; n++) {
/* 中文逐项-变量 V1101：recvRank
 * recvRank（int；函数 connectRings 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvRank = recv[firstRanks[n]]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int recvRank = recv[firstRanks[n]];
/* 中文逐项-变量 V1102：prevSendRank
 * prevSendRank（int；函数 connectRings 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 prevSendRank = send[firstRanks[(n-1+nNodes)%nNodes]]；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      int prevSendRank = send[firstRanks[(n-1+nNodes)%nNodes]];
      prev[recvRank] = prevSendRank;
      if (comm->rank == recvRank) {
        channel0->ring.prev = prevSendRank;
        channel1->ring.prev = prevSendRank;
      }
/* 中文逐项-变量 V1103：sendRank
 * sendRank（int；函数 connectRings 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendRank = send[firstRanks[n]]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int sendRank = send[firstRanks[n]];
/* 中文逐项-变量 V1104：nextRecvRank
 * nextRecvRank（int；函数 connectRings 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nextRecvRank = recv[firstRanks[(n+1)%nNodes]]；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      int nextRecvRank = recv[firstRanks[(n+1)%nNodes]];
      next[sendRank] = nextRecvRank;
      if (comm->rank == sendRank) {
        channel0->ring.next = nextRecvRank;
        channel1->ring.next = nextRecvRank;
      }
    }
    TRACE(NCCL_GRAPH, "Ring %d : %d -> %d -> %d", c, channel0->ring.prev, comm->rank, channel0->ring.next);
    TRACE(NCCL_GRAPH, "Ring %d : %d -> %d -> %d", c+nChannels, channel1->ring.prev, comm->rank, channel1->ring.next);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0431：getIndexes
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int n=0;；继续条件 n<nNodes；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 调用 / 计算 / 写入：indexes[n] = ranks[firstRanks[n]];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0431
 */
/* 中文逐项-函数 F0431：getIndexes
 * 用途与位置：把树连接端点的用户 rank 映射到节点索引，供节点级双树关系查表。
 * 参数 ranks（int* ranks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 indexes（int* indexes）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nNodes（int nNodes）：当前 communicator 跨越的节点数量；节点编号不等于 GPU rank。
 * 参数 firstRanks（int* firstRanks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::connectTrees。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：n<nNodes。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getIndexes(int* ranks, int* indexes, int nNodes, int* firstRanks) {
 for (/* 中文逐项-变量 V1105：n
 * n（int；函数 getIndexes 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<nNodes；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<nNodes; n++) indexes[n] = ranks[firstRanks[n]];
 return ncclSuccess;
}

/* 中文内部逻辑 F0432：setTreeUp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(u == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 调用 / 计算 / 写入：tree->up = indexes[u];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0432
 */
/* 中文逐项-函数 F0432：setTreeUp
 * 用途与位置：把节点级父端点映射成当前 GPU 树的 up 邻居，保留无父节点的 -1 语义。
 * 参数 tree（struct ncclTree* tree）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 * 参数 indexes（int* indexes）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 u（int u）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::connectTrees。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：u == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t setTreeUp(struct ncclTree* tree, int* indexes, int u) {
  if (u == -1) return ncclSuccess;
  tree->up = indexes[u];
  return ncclSuccess;
}

/* 中文内部逻辑 F0433：setTreeDown
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(d == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：int x = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 循环：初始 ；继续条件 (x < NCCL_MAX_TREE_ARITY && tree->down[x] >= 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 调用 / 计算 / 写入：x++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 条件判断：(x == NCCL_MAX_TREE_ARITY)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：WARN("Internal error : tree already has %d children (%d %d %d)", x, tree->down[0], tree->down[1], tree->down[2]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     11. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 调用 / 计算 / 写入：tree->down[x] = indexes[d];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0433
 */
/* 中文逐项-函数 F0433：setTreeDown
 * 用途与位置：把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。
 * 参数 tree（struct ncclTree* tree）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 * 参数 indexes（int* indexes）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 d（int d）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::connectTrees。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：d == -1；x < NCCL_MAX_TREE_ARITY && tree->down[x] >= 0；x == NCCL_MAX_TREE_ARITY。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t setTreeDown(struct ncclTree* tree, int* indexes, int d) {
  if (d == -1) return ncclSuccess;
/* 中文逐项-变量 V1106：x
 * x（int；函数 setTreeDown 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 x = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int x = 0;
  while (x < NCCL_MAX_TREE_ARITY && tree->down[x] >= 0) x++;
  if (x == NCCL_MAX_TREE_ARITY) {
    WARN("Internal error : tree already has %d children (%d %d %d)", x, tree->down[0], tree->down[1], tree->down[2]);
    return ncclInternalError;
  }
  tree->down[x] = indexes[d];
  return ncclSuccess;
}

/* 中文精读：双树为什么需要复制 channel
 * 对节点级别构造两棵互补二叉树，再把跨节点父/子连接接到局部 GPU 树的端点。
 * 把树的中间节点负载分摊到不同 GPU/节点，减少所有流量总压在同一组中间节点的问题。
 * 两个 channel 集合承载两棵树的不同数据部分；这不是对同一个完整张量重复归约两次。
 * 请同时阅读 graph/trees.cc 的 GetBtree/GetDtree，以及 device/all_reduce.h 的上下行线程拆分。
 */
/* 中文内部逻辑 F0434：connectTrees
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const int nChannels = comm->nChannels, nNodes = comm->nNodes, node = comm->node;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int* ranksToParent, *ranksToChild0, *ranksToChild1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&ranksToParent, nNodes));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&ranksToChild0, nNodes));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&ranksToChild1, nNodes));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 6. 准备局部状态：int depth = comm->nRanks/nNodes - 1 + log2i(nNodes);；用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。
 * 7. 准备局部状态：int t0u, t0d0, t0d1, t0ChildType, t1u, t1d0, t1d1, t1ChildType;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1, &t1ChildType));；失败时可能提前return，成功才继续；在第一棵二叉树基础上用镜像或平移构造互补树，分摊中间节点负载。
 * 9. 循环：初始 int c=0;；继续条件 c<nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   10. 准备局部状态：struct ncclChannel* channel0 = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   11. 准备局部状态：struct ncclChannel* channel1 = channel0+nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   12. 调用 / 计算 / 写入：NCCLCHECK(getIndexes(treeToParent+c*comm->nRanks, ranksToParent, nNodes, firstRanks));；失败时可能提前return，成功才继续；把树连接端点的用户 rank 映射到节点索引，供节点级双树关系查表。
 *   13. 调用 / 计算 / 写入：NCCLCHECK(getIndexes(treeToChild0+c*comm->nRanks, ranksToChild0, nNodes, firstRanks));；失败时可能提前return，成功才继续；把树连接端点的用户 rank 映射到节点索引，供节点级双树关系查表。
 *   14. 调用 / 计算 / 写入：NCCLCHECK(getIndexes(treeToChild1+c*comm->nRanks, ranksToChild1, nNodes, firstRanks));；失败时可能提前return，成功才继续；把树连接端点的用户 rank 映射到节点索引，供节点级双树关系查表。
 *   15. 条件判断：(comm->rank == ranksToParent[node])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     16. 真分支：；仅条件为真执行以下子项。
 *       17. 调用 / 计算 / 写入：NCCLCHECK(setTreeUp(&channel0->tree, t0ChildType == 0 ? ranksToChild0 : ranksToChild1, t0u));；失败时可能提前return，成功才继续；把节点级父端点映射成当前 GPU 树的 up 邻居，保留无父节点的 -1 语义。
 *       18. 调用 / 计算 / 写入：NCCLCHECK(setTreeUp(&channel1->tree, t1ChildType == 0 ? ranksToChild0 : ranksToChild1, t1u));；失败时可能提前return，成功才继续；把节点级父端点映射成当前 GPU 树的 up 邻居，保留无父节点的 -1 语义。
 *   19. 条件判断：(comm->rank == ranksToChild0[node])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     20. 真分支：；仅条件为真执行以下子项。
 *       21. 调用 / 计算 / 写入：NCCLCHECK(setTreeDown(&channel0->tree, ranksToParent, t0d0));；失败时可能提前return，成功才继续；把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。
 *       22. 调用 / 计算 / 写入：NCCLCHECK(setTreeDown(&channel1->tree, ranksToParent, t1d0));；失败时可能提前return，成功才继续；把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。
 *   23. 条件判断：(comm->rank == ranksToChild1[node])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     24. 真分支：；仅条件为真执行以下子项。
 *       25. 调用 / 计算 / 写入：NCCLCHECK(setTreeDown(&channel0->tree, ranksToParent, t0d1));；失败时可能提前return，成功才继续；把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。
 *       26. 调用 / 计算 / 写入：NCCLCHECK(setTreeDown(&channel1->tree, ranksToParent, t1d1));；失败时可能提前return，成功才继续；把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。
 *   27. 条件判断：(comm->rank == ranksToParent[node] || comm->rank == ranksToChild0[node] || comm->rank == ranksToChild1[node])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     28. 真分支：；仅条件为真执行以下子项。
 *       29. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Tree %d : %d -> %d -> %d/%d/%d", c, channel0->tree.up, comm->rank, channel0->tree.down[0], channel0->tree.down[1], channel0->tree.down[2]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       30. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Tree %d : %d -> %d -> %d/%d/%d", c+nChannels, channel1->tree.up, comm->rank, channel1->tree.down[0], channel1->tree.down[1], channel1->tree.down[2]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   31. 调用 / 计算 / 写入：channel0->tree.depth = channel1->tree.depth = depth;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 33. 调用 / 计算 / 写入：free(ranksToParent);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 34. 调用 / 计算 / 写入：free(ranksToChild0);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 35. 调用 / 计算 / 写入：free(ranksToChild1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 36. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0434
 */
/* 中文逐项-函数 F0434：connectTrees
 * 用途与位置：用互补二叉树连接各节点的本地树片段，并复制通道来分摊中间节点负载。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 treeToParent（int* treeToParent）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 * 参数 treeToChild0（int* treeToChild0）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 * 参数 treeToChild1（int* treeToChild1）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 * 参数 firstRanks（int* firstRanks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 treePatterns（int* treePatterns）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::ncclTopoPostset。
 * 本体直接出现的调用/包装：NCCLCHECK；free；getIndexes；log2i；ncclCalloc；ncclGetDtree；setTreeDown；setTreeUp。
 * 需要对照的边界/推进条件：c<nChannels；comm->rank == ranksToParent[node]；comm->rank == ranksToChild0[node]；comm->rank ==
 * ranksToChild1[node]。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t connectTrees(struct ncclComm* comm, int* treeToParent, int* treeToChild0, int* treeToChild1, int* firstRanks, int* treePatterns) {
/* 中文逐项-变量 V1107：nChannels、nNodes、node
 * nChannels（int；函数 connectTrees 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = comm->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * nNodes（int；函数 connectTrees 内）：当前 communicator 跨越的节点数量；节点编号不等于 GPU rank。
 * 初值/来源 nNodes = comm->nNodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * node（int；函数 connectTrees 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = comm->node；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  const int nChannels = comm->nChannels, nNodes = comm->nNodes, node = comm->node;
/* 中文逐项-变量 V1108：ranksToParent、ranksToChild0、ranksToChild1
 * ranksToParent（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&ranksToParent, nNodes));。若由输出参数填入，必须先检查填充调用成功再读值。
 * ranksToChild0（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&ranksToChild0, nNodes));。若由输出参数填入，必须先检查填充调用成功再读值。
 * ranksToChild1（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&ranksToChild1, nNodes));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int* ranksToParent, *ranksToChild0, *ranksToChild1;
  NCCLCHECK(ncclCalloc(&ranksToParent, nNodes));
  NCCLCHECK(ncclCalloc(&ranksToChild0, nNodes));
  NCCLCHECK(ncclCalloc(&ranksToChild1, nNodes));

  // Compute tree depth. Not an exact value but a good approximation in most
  // cases
/* 中文逐项-变量 V1109：depth
 * depth（int；函数 connectTrees 内）：通信树估计深度，用于分片和流水调优。
 * 初值/来源 depth = comm->nRanks/nNodes - 1 + log2i(nNodes)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int depth = comm->nRanks/nNodes - 1 + log2i(nNodes);

/* 中文逐项-变量 V1110：t0u、t0d0、t0d1、t0ChildType、t1u、t1d0、t1d1、t1ChildType
 * t0u（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1,
 * &t1ChildType));。若由输出参数填入，必须先检查填充调用成功再读值。
 * t0d0（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1,
 * &t1ChildType));。若由输出参数填入，必须先检查填充调用成功再读值。
 * t0d1（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1,
 * &t1ChildType));。若由输出参数填入，必须先检查填充调用成功再读值。
 * t0ChildType（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1,
 * &t1ChildType));。若由输出参数填入，必须先检查填充调用成功再读值。
 * t1u（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1,
 * &t1ChildType));。若由输出参数填入，必须先检查填充调用成功再读值。
 * t1d0（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1,
 * &t1ChildType));。若由输出参数填入，必须先检查填充调用成功再读值。
 * t1d1（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1,
 * &t1ChildType));。若由输出参数填入，必须先检查填充调用成功再读值。
 * t1ChildType（int；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1,
 * &t1ChildType));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int t0u, t0d0, t0d1, t0ChildType, t1u, t1d0, t1d1, t1ChildType;
  NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1, &t1ChildType));
  for (/* 中文逐项-变量 V1111：c
 * c（int；函数 connectTrees 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<nChannels; c++) {
/* 中文逐项-变量 V1112：channel0
 * channel0（struct ncclChannel；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channel0 = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
     struct ncclChannel* channel0 = comm->channels+c;
/* 中文逐项-变量 V1113：channel1
 * channel1（struct ncclChannel；函数 connectTrees 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channel1 = channel0+nChannels；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
     struct ncclChannel* channel1 = channel0+nChannels;
     NCCLCHECK(getIndexes(treeToParent+c*comm->nRanks, ranksToParent, nNodes, firstRanks));
     NCCLCHECK(getIndexes(treeToChild0+c*comm->nRanks, ranksToChild0, nNodes, firstRanks));
     NCCLCHECK(getIndexes(treeToChild1+c*comm->nRanks, ranksToChild1, nNodes, firstRanks));
     if (comm->rank == ranksToParent[node]) {
       NCCLCHECK(setTreeUp(&channel0->tree, t0ChildType == 0 ? ranksToChild0 : ranksToChild1, t0u));
       NCCLCHECK(setTreeUp(&channel1->tree, t1ChildType == 0 ? ranksToChild0 : ranksToChild1, t1u));
     }
     if (comm->rank == ranksToChild0[node]) {
       NCCLCHECK(setTreeDown(&channel0->tree, ranksToParent, t0d0));
       NCCLCHECK(setTreeDown(&channel1->tree, ranksToParent, t1d0));
     }
     if (comm->rank == ranksToChild1[node]) {
       NCCLCHECK(setTreeDown(&channel0->tree, ranksToParent, t0d1));
       NCCLCHECK(setTreeDown(&channel1->tree, ranksToParent, t1d1));
     }
     if (comm->rank == ranksToParent[node] ||
         comm->rank == ranksToChild0[node] ||
         comm->rank == ranksToChild1[node]) {
       INFO(NCCL_GRAPH, "Tree %d : %d -> %d -> %d/%d/%d", c,           channel0->tree.up, comm->rank, channel0->tree.down[0], channel0->tree.down[1], channel0->tree.down[2]);
       INFO(NCCL_GRAPH, "Tree %d : %d -> %d -> %d/%d/%d", c+nChannels, channel1->tree.up, comm->rank, channel1->tree.down[0], channel1->tree.down[1], channel1->tree.down[2]);
     }
     channel0->tree.depth = channel1->tree.depth = depth;
  }
  free(ranksToParent);
  free(ranksToChild0);
  free(ranksToChild1);
  return ncclSuccess;
}

/* 中文内部逻辑 F0435：connectCollNet
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int rank = comm->rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int localRanks = comm->localRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int nHeads = collNetGraph->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int *heads;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&heads, nHeads));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 6. 循环：初始 int c=0;；继续条件 c<nHeads；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 准备局部状态：int* collNetIntra = collNetGraph->intra+c*localRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 调用 / 计算 / 写入：heads[c] = collNetIntra[0];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 10. 循环：初始 int c=0;；继续条件 c<comm->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   11. 准备局部状态：struct ncclChannel* channel = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   12. 准备局部状态：char line[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   13. 调用 / 计算 / 写入：sprintf(line, "CollNet channel %d rank %d ", c, rank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   14. 准备局部状态：int nDown = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   15. 循环：初始 int i=0;；继续条件 i<nHeads；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     16. 条件判断：(rank == heads[i])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       17. 真分支：；仅条件为真执行以下子项。
 *         18. 调用 / 计算 / 写入：channel->collTree.headRank = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         19. 调用 / 计算 / 写入：channel->collTree.out = comm->nRanks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         20. 准备局部状态：int* collNetIntra = collNetGraph->intra+i*localRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         21. 调用 / 计算 / 写入：sprintf(line+strlen(line), "down ");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         22. 循环：初始 int r=0;；继续条件 r<localRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           23. 条件判断：(collNetIntra[r] == rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             24. 真分支：；仅条件为真执行以下子项。
 *               25. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *           26. 调用 / 计算 / 写入：channel->collTree.down[nDown++] = collNetIntra[r];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           27. 调用 / 计算 / 写入：sprintf(line+strlen(line), " %d ", collNetIntra[r]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         28. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         29. 调用 / 计算 / 写入：sprintf(line+strlen(line), "nDown %d ", nDown);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         30. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *   31. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   32. 准备局部状态：int nUp = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   33. 调用 / 计算 / 写入：sprintf(line+strlen(line), "up ");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   34. 循环：初始 int h=0;；继续条件 h<nHeads；每轮更新 h++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     35. 条件判断：(rank == heads[h])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       36. 真分支：；仅条件为真执行以下子项。
 *         37. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *     38. 调用 / 计算 / 写入：channel->collTree.up[nUp++] = heads[h];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     39. 调用 / 计算 / 写入：sprintf(line+strlen(line), " %d ", heads[h]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   40. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   41. 调用 / 计算 / 写入：channel->collTree.nHeads = nHeads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   42. 调用 / 计算 / 写入：channel->collTree.shift = (rank%localRanks)%nHeads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   43. 调用 / 计算 / 写入：channel->collTree.depth = (nUp == 0 && nDown == 0) ? 1 : 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   44. 调用 / 计算 / 写入：sprintf(line+strlen(line), "nUp %d nHeads %d ", nUp, nHeads);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   45. 调用 / 计算 / 写入：sprintf(line+strlen(line), "headRank %d out %d shift %d", channel->collTree.headRank, channel->collTree.out, channel->collTree.shift);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   46. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 47. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 48. 调用 / 计算 / 写入：free(heads);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 49. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0435
 */
/* 中文逐项-函数 F0435：connectCollNet
 * 用途与位置：把本节点 GPU 组织为多个网络 head 的分散/归约/广播/收集关系。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 collNetGraph（struct ncclTopoGraph* collNetGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::ncclTopoPostset。
 * 本体直接出现的调用/包装：NCCLCHECK；free；ncclCalloc；sprintf；strlen。
 * 需要对照的边界/推进条件：c<nHeads；c<comm->nChannels；i<nHeads；rank == heads[i]。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t connectCollNet(struct ncclComm* comm, struct ncclTopoGraph* collNetGraph) {
/* 中文逐项-变量 V1114：rank
 * rank（int；函数 connectCollNet 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = comm->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = comm->rank;
/* 中文逐项-变量 V1115：localRanks
 * localRanks（int；函数 connectCollNet 内）：当前节点内参与此 communicator 的 rank 数。
 * 初值/来源 localRanks = comm->localRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int localRanks = comm->localRanks;
/* 中文逐项-变量 V1116：nHeads
 * nHeads（int；函数 connectCollNet 内）：当前 CollNet 分工的网络 head 数，额外参与一轮数据块数量计算。
 * 初值/来源 nHeads = collNetGraph->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nHeads = collNetGraph->nChannels;
/* 中文逐项-变量 V1117：heads
 * heads（int；函数 connectCollNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&heads, nHeads));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int *heads;
  NCCLCHECK(ncclCalloc(&heads, nHeads));
  // Find all head ranks
  // Head index is always 0
  for (/* 中文逐项-变量 V1118：c
 * c（int；函数 connectCollNet 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<nHeads；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<nHeads; c++) {
/* 中文逐项-变量 V1119：collNetIntra
 * collNetIntra（int；函数 connectCollNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNetIntra = collNetGraph->intra+c*localRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int* collNetIntra = collNetGraph->intra+c*localRanks;
    heads[c] = collNetIntra[0];
  }
  // For all channels
  for (/* 中文逐项-变量 V1120：c
 * c（int；函数 connectCollNet 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->nChannels; c++) {
/* 中文逐项-变量 V1121：channel
 * channel（struct ncclChannel；函数 connectCollNet 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+c;
/* 中文逐项-变量 V1122：line
 * line（char；函数 connectCollNet 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：sprintf(line, "CollNet channel %d rank %d ", c, rank);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char line[1024];
    sprintf(line, "CollNet channel %d rank %d ", c, rank);
/* 中文逐项-变量 V1123：nDown
 * nDown（int；函数 connectCollNet 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nDown = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int nDown = 0;
    for (/* 中文逐项-变量 V1124：i
 * i（int；函数 connectCollNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nHeads；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nHeads; i++) {
      if (rank == heads[i]) { // is head
        channel->collTree.headRank = i; // Mark the index for deciding offset in the CUDA kernel
        channel->collTree.out = comm->nRanks; // Set root of collTree to id nranks
/* 中文逐项-变量 V1125：collNetIntra
 * collNetIntra（int；函数 connectCollNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNetIntra = collNetGraph->intra+i*localRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        int* collNetIntra = collNetGraph->intra+i*localRanks;
        sprintf(line+strlen(line), "down ");
        for (/* 中文逐项-变量 V1126：r
 * r（int；函数 connectCollNet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<localRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<localRanks; r++) {
          if (collNetIntra[r] == rank) continue;
          channel->collTree.down[nDown++] = collNetIntra[r];  // connect to all peers
          sprintf(line+strlen(line), " %d ", collNetIntra[r]);
        }
        sprintf(line+strlen(line), "nDown %d ", nDown);
        break;
      }
    }
    // Connect to all heads
/* 中文逐项-变量 V1127：nUp
 * nUp（int；函数 connectCollNet 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nUp = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int nUp = 0;
    sprintf(line+strlen(line), "up ");
    for (/* 中文逐项-变量 V1128：h
 * h（int；函数 connectCollNet 内）：GDRCopy固定缓冲句柄的内部表示，必须用匹配的unpin接口回收。
 * 初值/来源 h = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：h<nHeads；循环体与增量表达式决定每轮如何推进。
 */int h=0; h<nHeads; h++) {
      if (rank == heads[h]) continue;
      channel->collTree.up[nUp++] = heads[h];
      sprintf(line+strlen(line), " %d ", heads[h]);
    }
    channel->collTree.nHeads = nHeads;
    channel->collTree.shift = (rank%localRanks)%nHeads; // Shift by intraRank so that leaves don't send to same head simultaneously
    channel->collTree.depth = (nUp == 0 && nDown == 0) ? 1 : 2;
    sprintf(line+strlen(line), "nUp %d nHeads %d ", nUp, nHeads);
    sprintf(line+strlen(line), "headRank %d out %d shift %d", channel->collTree.headRank, channel->collTree.out, channel->collTree.shift);
    INFO(NCCL_GRAPH, "%s", line);
  }
  free(heads);
  return ncclSuccess;
}

// Legacy naming
/* 中文逐项-参数getter P0003：MinNrings
 * 生成函数 ncclParamMinNrings()，读取 NCCL_MIN_NRINGS，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(MinNrings, "MIN_NRINGS", -2);
/* 中文逐项-参数getter P0004：MaxNrings
 * 生成函数 ncclParamMaxNrings()，读取 NCCL_MAX_NRINGS，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(MaxNrings, "MAX_NRINGS", -2);
// New naming
/* 中文逐项-参数getter P0005：MinNchannels
 * 生成函数 ncclParamMinNchannels()，读取 NCCL_MIN_NCHANNELS，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(MinNchannels, "MIN_NCHANNELS", -2);
/* 中文逐项-参数getter P0006：MaxNchannels
 * 生成函数 ncclParamMaxNchannels()，读取 NCCL_MAX_NCHANNELS，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(MaxNchannels, "MAX_NCHANNELS", -2);

/* 中文内部逻辑 F0436：ncclMinNchannels
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int minNchannels = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(ncclParamMinNrings() != -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：minNchannels = ncclParamMinNrings();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 条件判断：(ncclParamMinNchannels() != -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：minNchannels = ncclParamMinNchannels();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 条件判断：(minNchannels > MAXCHANNELS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：WARN("User asked for a minimum of %d channels, limiting to %d", minNchannels, MAXCHANNELS);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     11. 调用 / 计算 / 写入：minNchannels = MAXCHANNELS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 条件判断：(minNchannels < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 调用 / 计算 / 写入：minNchannels = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return minNchannels;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0436
 */
/* 中文逐项-函数 F0436：ncclMinNchannels
 * 用途与位置：读取并校验集合通信 channel 上下限，兼容旧 NRINGS 配置名后限制到编译期容量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::ncclTopoPostset。
 * 本体直接出现的调用/包装：ncclParamMinNchannels；ncclParamMinNrings。
 * 需要对照的边界/推进条件：ncclParamMinNrings() != -2；ncclParamMinNchannels() != -2；minNchannels > MAXCHANNELS；minNchannels <
 * 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return minNchannels;。CHECK类宏还可能提前返回/跳转。
 */
int ncclMinNchannels() {
/* 中文逐项-变量 V1129：minNchannels
 * minNchannels（int；函数 ncclMinNchannels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minNchannels = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int minNchannels = 0;
  if (ncclParamMinNrings() != -2) minNchannels = ncclParamMinNrings();
  if (ncclParamMinNchannels() != -2) minNchannels = ncclParamMinNchannels();
  if (minNchannels > MAXCHANNELS) {
    WARN("User asked for a minimum of %d channels, limiting to %d", minNchannels, MAXCHANNELS);
    minNchannels = MAXCHANNELS;
  }
  if (minNchannels < 0) minNchannels = 0;
  return minNchannels;
}
/* 中文内部逻辑 F0437：ncclMaxNchannels
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int maxNchannels = MAXCHANNELS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(ncclParamMaxNrings() != -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：maxNchannels = ncclParamMaxNrings();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 条件判断：(ncclParamMaxNchannels() != -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：maxNchannels = ncclParamMaxNchannels();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 条件判断：(maxNchannels > MAXCHANNELS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：maxNchannels = MAXCHANNELS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 条件判断：(maxNchannels < 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 调用 / 计算 / 写入：WARN("User asked for a maximum of %d channels, setting it to 1", maxNchannels);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     14. 调用 / 计算 / 写入：maxNchannels = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return maxNchannels;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0437
 */
/* 中文逐项-函数 F0437：ncclMaxNchannels
 * 用途与位置：读取并校验集合通信 channel 上下限，兼容旧 NRINGS 配置名后限制到编译期容量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::ncclTopoPostset。
 * 本体直接出现的调用/包装：ncclParamMaxNchannels；ncclParamMaxNrings。
 * 需要对照的边界/推进条件：ncclParamMaxNrings() != -2；ncclParamMaxNchannels() != -2；maxNchannels > MAXCHANNELS；maxNchannels <
 * 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return maxNchannels;。CHECK类宏还可能提前返回/跳转。
 */
int ncclMaxNchannels() {
/* 中文逐项-变量 V1130：maxNchannels
 * maxNchannels（int；函数 ncclMaxNchannels 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxNchannels = MAXCHANNELS；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int maxNchannels = MAXCHANNELS;
  if (ncclParamMaxNrings() != -2) maxNchannels = ncclParamMaxNrings();
  if (ncclParamMaxNchannels() != -2) maxNchannels = ncclParamMaxNchannels();
  if (maxNchannels > MAXCHANNELS) maxNchannels = MAXCHANNELS;
  if (maxNchannels < 1) {
    WARN("User asked for a maximum of %d channels, setting it to 1", maxNchannels);
    maxNchannels = 1;
  }
  return maxNchannels;
}

/* 中文内部逻辑 F0438：copyChannels
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int nranks = comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 c=start；继续条件 c<end；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 调用 / 计算 / 写入：memcpy(ringPrev+c*nranks, ringPrev+(c-start)*nranks, nranks*sizeof(int));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   5. 调用 / 计算 / 写入：memcpy(ringNext+c*nranks, ringNext+(c-start)*nranks, nranks*sizeof(int));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   6. 调用 / 计算 / 写入：memcpy(comm->channels+c, comm->channels+c-start, sizeof(struct ncclChannel));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 函数返回：return c;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0438
 */
/* 中文逐项-函数 F0438：copyChannels
 * 用途与位置：把已有通道的邻居关系复制到新增逻辑 channel，使最小并行度要求有可用上下文。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 start（int start）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 end（int end）：本任务结束序号或范围末端，通常由 base 与总步数推导。
 * 参数 ringPrev（int* ringPrev）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 参数 ringNext（int* ringNext）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::ncclTopoPostset。
 * 本体直接出现的调用/包装：memcpy。
 * 需要对照的边界/推进条件：c<end。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return c;。CHECK类宏还可能提前返回/跳转。
 */
static int copyChannels(struct ncclComm* comm, int start, int end, int* ringPrev, int* ringNext) {
/* 中文逐项-变量 V1131：nranks
 * nranks（int；函数 copyChannels 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nranks = comm->nRanks;
/* 中文逐项-变量 V1132：c
 * c（int；函数 copyChannels 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 此处未显式给初值；后续首个文本使用点：for (c=start; c<end; c++) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int c;
  for (c=start; c<end; c++) {
    memcpy(ringPrev+c*nranks, ringPrev+(c-start)*nranks, nranks*sizeof(int));
    memcpy(ringNext+c*nranks, ringNext+(c-start)*nranks, nranks*sizeof(int));
    memcpy(comm->channels+c, comm->channels+c-start, sizeof(struct ncclChannel));
  }
  return c;
}

/* 中文导读：
 * 把各节点的局部拓扑结果拼成全局 Ring/Tree，并应用 channel 数限制/复制策略。
 * 这是硬件局部搜索结果变成每 rank 可执行邻居关系的关键阶段，随后才进入 transport 建链。
 */
/* 中文内部逻辑 F0439：ncclTopoPostset
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int *ringRecv, *ringSend, *ringPrev, *ringNext, *treeToParent, *treeToChild0, *treeToChild1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int nranks = comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int nChannels = comm->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&ringRecv, nranks*MAXCHANNELS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&ringSend, nranks*MAXCHANNELS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&ringPrev, nranks*MAXCHANNELS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&ringNext, nranks*MAXCHANNELS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&treeToParent, nranks*MAXCHANNELS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&treeToChild0, nranks*MAXCHANNELS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 10. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&treeToChild1, nranks*MAXCHANNELS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 11. 循环：初始 int i=0;；继续条件 i<nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   12. 循环：初始 int c=0;；继续条件 c<nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     13. 调用 / 计算 / 写入：ringRecv[c*nranks+i] = allTopoRanks[i]->ringRecv[c];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     14. 调用 / 计算 / 写入：ringSend[c*nranks+i] = allTopoRanks[i]->ringSend[c];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     15. 调用 / 计算 / 写入：ringPrev[c*nranks+i] = allTopoRanks[i]->ringPrev[c];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     16. 调用 / 计算 / 写入：ringNext[c*nranks+i] = allTopoRanks[i]->ringNext[c];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     17. 调用 / 计算 / 写入：treeToParent[c*nranks+i] = allTopoRanks[i]->treeToParent[c];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     18. 调用 / 计算 / 写入：treeToChild0[c*nranks+i] = allTopoRanks[i]->treeToChild0[c];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     19. 调用 / 计算 / 写入：treeToChild1[c*nranks+i] = allTopoRanks[i]->treeToChild1[c];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 21. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 22. 调用 / 计算 / 写入：NCCLCHECK(connectRings(comm, ringRecv, ringSend, ringPrev, ringNext, firstRanks));；失败时可能提前return，成功才继续；把各节点局部 GPU 链的出口接到下一节点入口，补全完整环的前驱/后继。
 * 23. 调用 / 计算 / 写入：NCCLCHECK(connectTrees(comm, treeToParent, treeToChild0, treeToChild1, firstRanks, treePatterns));；失败时可能提前return，成功才继续；用互补二叉树连接各节点的本地树片段，并复制通道来分摊中间节点负载。
 * 24. 调用 / 计算 / 写入：memcpy(ringPrev+nChannels*nranks, ringPrev, nChannels*nranks*sizeof(int));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 25. 调用 / 计算 / 写入：memcpy(ringNext+nChannels*nranks, ringNext, nChannels*nranks*sizeof(int));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 26. 调用 / 计算 / 写入：nChannels = comm->nChannels = std::min(MAXCHANNELS,nChannels*2);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 * 27. 条件判断：(comm->collNetSupport == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   28. 真分支：；仅条件为真执行以下子项。
 *     29. 条件判断：(collNetGraph->speedIntra > collNetGraph->speedInter && comm->nRanks > comm->nNodes)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       30. 真分支：；仅条件为真执行以下子项。
 *         31. 准备局部状态：int collNetNchannels = std::min(MAXCHANNELS, nChannels+nChannels/2);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *         32. 调用 / 计算 / 写入：nChannels = comm->nChannels = copyChannels(comm, nChannels, collNetNchannels, ringPrev, ringNext);；把已有通道的邻居关系复制到新增逻辑 channel，使最小并行度要求有可用上下文。
 *     33. 调用 / 计算 / 写入：NCCLCHECK(connectCollNet(comm, collNetGraph));；失败时可能提前return，成功才继续；把本节点 GPU 组织为多个网络 head 的分散/归约/广播/收集关系。
 * 34. 调用 / 计算 / 写入：nChannels = comm->nChannels = std::min((int)ncclMaxNchannels(), nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 * 35. 调用 / 计算 / 写入：nChannels = comm->nChannels = copyChannels(comm, nChannels, ncclMinNchannels(), ringPrev, ringNext);；把已有通道的邻居关系复制到新增逻辑 channel，使最小并行度要求有可用上下文。
 * 36. 调用 / 计算 / 写入：NCCLCHECK(ncclBuildRings(nChannels, rings, comm->rank, comm->nRanks, ringPrev, ringNext));；失败时可能提前return，成功才继续；跟随 next 构建每 channel 环顺序，验证 nranks 步回到起点且没有遗漏参与 rank。
 * 37. 调用 / 计算 / 写入：free(ringRecv);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 38. 调用 / 计算 / 写入：free(ringSend);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 39. 调用 / 计算 / 写入：free(ringPrev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 40. 调用 / 计算 / 写入：free(ringNext);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 41. 调用 / 计算 / 写入：free(treeToParent);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 42. 调用 / 计算 / 写入：free(treeToChild0);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 43. 调用 / 计算 / 写入：free(treeToChild1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 44. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__connect_cc.md#f0439
 */
/* 中文逐项-函数 F0439：ncclTopoPostset
 * 用途与位置：汇总全部节点的端点信息并拼接全局 Ring/Tree，复制/裁剪通道后验证环完整性。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 firstRanks（int* firstRanks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 treePatterns（int* treePatterns）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 allTopoRanks（struct ncclTopoRanks** allTopoRanks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 rings（int* rings）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 collNetGraph（struct ncclTopoGraph* collNetGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；connectCollNet；connectRings；connectTrees；copyChannels；free；memcpy；ncclBuildRings；ncclCalloc；ncclMaxNchannels；ncclMinNchannels；std::min。
 * 需要对照的边界/推进条件：i<nranks；c<nChannels；comm->collNetSupport == 1；collNetGraph->speedIntra > collNetGraph->speedInter &&
 * comm->nRanks > comm->nNodes。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoPostset(struct ncclComm* comm, int* firstRanks, int* treePatterns, struct ncclTopoRanks** allTopoRanks, int* rings, struct ncclTopoGraph* collNetGraph) {
  // Gather data from all ranks
/* 中文逐项-变量 V1133：ringRecv、ringSend、ringPrev、ringNext、treeToParent、treeToChild0、treeToChild1
 * ringRecv（int；函数 ncclTopoPostset 内）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&ringRecv, nranks*MAXCHANNELS));。若由输出参数填入，必须先检查填充调用成功再读值。
 * ringSend（int；函数 ncclTopoPostset 内）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&ringSend, nranks*MAXCHANNELS));。若由输出参数填入，必须先检查填充调用成功再读值。
 * ringPrev（int；函数 ncclTopoPostset 内）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&ringPrev, nranks*MAXCHANNELS));。若由输出参数填入，必须先检查填充调用成功再读值。
 * ringNext（int；函数 ncclTopoPostset 内）：全局拼接时每节点入口/出口与各rank前后继摘要，按channel/rank二维索引。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&ringNext, nranks*MAXCHANNELS));。若由输出参数填入，必须先检查填充调用成功再读值。
 * treeToParent（int；函数 ncclTopoPostset 内）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&treeToParent, nranks*MAXCHANNELS));。若由输出参数填入，必须先检查填充调用成功再读值。
 * treeToChild0（int；函数 ncclTopoPostset 内）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&treeToChild0, nranks*MAXCHANNELS));。若由输出参数填入，必须先检查填充调用成功再读值。
 * treeToChild1（int；函数 ncclTopoPostset 内）：节点局部树连接到父/两个孩子节点所用的GPU端点摘要。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&treeToChild1, nranks*MAXCHANNELS));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int *ringRecv, *ringSend, *ringPrev, *ringNext, *treeToParent, *treeToChild0, *treeToChild1;
/* 中文逐项-变量 V1134：nranks
 * nranks（int；函数 ncclTopoPostset 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nranks = comm->nRanks;
/* 中文逐项-变量 V1135：nChannels
 * nChannels（int；函数 ncclTopoPostset 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = comm->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nChannels = comm->nChannels;
  NCCLCHECK(ncclCalloc(&ringRecv, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&ringSend, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&ringPrev, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&ringNext, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&treeToParent, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&treeToChild0, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&treeToChild1, nranks*MAXCHANNELS));
  for (/* 中文逐项-变量 V1136：i
 * i（int；函数 ncclTopoPostset 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nranks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nranks; i++) {
    for (/* 中文逐项-变量 V1137：c
 * c（int；函数 ncclTopoPostset 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<nChannels;c++) {
      ringRecv[c*nranks+i] = allTopoRanks[i]->ringRecv[c];
      ringSend[c*nranks+i] = allTopoRanks[i]->ringSend[c];
      ringPrev[c*nranks+i] = allTopoRanks[i]->ringPrev[c];
      ringNext[c*nranks+i] = allTopoRanks[i]->ringNext[c];
      treeToParent[c*nranks+i] = allTopoRanks[i]->treeToParent[c];
      treeToChild0[c*nranks+i] = allTopoRanks[i]->treeToChild0[c];
      treeToChild1[c*nranks+i] = allTopoRanks[i]->treeToChild1[c];
    }
  }

  // Connect rings and trees. This should also duplicate the channels.
  NCCLCHECK(connectRings(comm, ringRecv, ringSend, ringPrev, ringNext, firstRanks));
  NCCLCHECK(connectTrees(comm, treeToParent, treeToChild0, treeToChild1, firstRanks, treePatterns));

  // Duplicate ringPrev/ringNext for ncclBuildRing
  memcpy(ringPrev+nChannels*nranks, ringPrev, nChannels*nranks*sizeof(int));
  memcpy(ringNext+nChannels*nranks, ringNext, nChannels*nranks*sizeof(int));

/* 中文精读：通道复制与最终 MIN/MAX 调整
 * 这里 nChannels 翻倍配合前面树的复制，Ring 的前驱/后继表也同步复制。
 * 若 CollNet 需要更多机内并行度，还可增加通道；最后先应用最大值，再按最小值复制补足。
 * 复制的是逻辑执行上下文/通信图，后面会为相应 connector 建链；不是凭空增加物理带宽。
 * 实际能否获益要看 NIC、NVLink、SM 占用和消息尺寸，不宜仅凭最终通道数判断性能。
 */
  // Duplication should be complete now
  nChannels = comm->nChannels = std::min(MAXCHANNELS,nChannels*2);

  // Setup CollNet
  if (comm->collNetSupport == 1) {
    // Add more channels to saturate intra-node bandwidth, except the 1 PPN case
    if (collNetGraph->speedIntra > collNetGraph->speedInter && comm->nRanks > comm->nNodes) {
/* 中文逐项-变量 V1138：collNetNchannels
 * collNetNchannels（int；函数 ncclTopoPostset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNetNchannels = std::min(MAXCHANNELS, nChannels+nChannels/2)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
      int collNetNchannels = std::min(MAXCHANNELS, nChannels+nChannels/2);
      nChannels = comm->nChannels = copyChannels(comm, nChannels, collNetNchannels, ringPrev, ringNext);
    }
    NCCLCHECK(connectCollNet(comm, collNetGraph));
  }

  // Honor NCCL_MIN_NRINGS/NCCL_MAX_NRINGS.
  // We permit combining max, then min, to only use the first channels, then duplicate them.
  nChannels = comm->nChannels = std::min((int)ncclMaxNchannels(), nChannels);
  nChannels = comm->nChannels = copyChannels(comm, nChannels, ncclMinNchannels(), ringPrev, ringNext);

  // Create rings array and check all is fine
  NCCLCHECK(ncclBuildRings(nChannels, rings, comm->rank, comm->nRanks, ringPrev, ringNext));

  free(ringRecv);
  free(ringSend);
  free(ringPrev);
  free(ringNext);
  free(treeToParent);
  free(treeToChild0);
  free(treeToChild1);

  return ncclSuccess;
}
