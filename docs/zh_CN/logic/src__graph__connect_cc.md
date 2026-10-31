# src/graph/connect.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0429"></a>

## F0429：ncclTopoPreset

用途：将本机搜索图转成可交换的局部环/树端点摘要，为跨节点拼接准备元数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int rank = comm-&gt;rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int localRanks = comm-&gt;topo-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int nChannels = comm-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;ring.prev = channel-&gt;ring.next = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;tree.up = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_MAX_TREE_ARITY；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>channel-&gt;tree.down[i] = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.out = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.headRank = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.nHeads = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.shift = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_MAX_DIRECT_ARITY；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.up[i] = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_MAX_DIRECT_ARITY；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.down[i] = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 1 / 准备局部状态 | <code>int* ringIntra = ringGraph-&gt;intra+c*localRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 1 / 准备局部状态 | <code>int* treeIntra = treeGraph-&gt;intra+c*localRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;localRanks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 2 / 条件判断 | <code>(ringIntra[i] == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>topoRanks-&gt;ringRecv[c] = ringIntra[0];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 4 / 调用 / 计算 / 写入 | <code>topoRanks-&gt;ringSend[c] = ringIntra[localRanks-1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 4 / 调用 / 计算 / 写入 | <code>channel-&gt;ring.prev = (i == 0) ? -1 : ringIntra[i-1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>channel-&gt;ring.next = (i == localRanks-1) ? -1 : ringIntra[i+1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 2 / 条件判断 | <code>(treeIntra[i] == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 4 / 准备局部状态 | <code>int parentIndex = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 4 / 准备局部状态 | <code>int child0Index = treeGraph-&gt;pattern == NCCL_TOPO_PATTERN_TREE ? 0 : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 4 / 准备局部状态 | <code>int child1Index = treeGraph-&gt;pattern == NCCL_TOPO_PATTERN_SPLIT_TREE ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 4 / 调用 / 计算 / 写入 | <code>topoRanks-&gt;treeToParent[c] = treeIntra[parentIndex];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 4 / 调用 / 计算 / 写入 | <code>topoRanks-&gt;treeToChild0[c] = treeIntra[child0Index];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 4 / 调用 / 计算 / 写入 | <code>topoRanks-&gt;treeToChild1[c] = treeIntra[child1Index];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>channel-&gt;tree.up = i == 0 ? -1 : treeIntra[i-1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 4 / 调用 / 计算 / 写入 | <code>channel-&gt;tree.down[0] = i == localRanks-1 ? -1 : treeIntra[i+1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 41 | 1 / 调用 / 计算 / 写入 | <code>topoRanks-&gt;ringPrev[c] = channel-&gt;ring.prev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 1 / 调用 / 计算 / 写入 | <code>topoRanks-&gt;ringNext[c] = channel-&gt;ring.next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 44 | 0 / 准备局部状态 | <code>struct ncclChannel* channel0 = comm-&gt;channels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 0 / 准备局部状态 | <code>struct ncclChannel* channel1 = channel0+nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>memcpy(channel1, channel0, nChannels*sizeof(struct ncclChannel));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int rank = comm->rank;
  int localRanks = comm->topo->nodes[GPU].count;
  int nChannels = comm->nChannels;

  for (int c=0; c<nChannels; c++) {
    struct ncclChannel* channel = comm->channels+c;
    channel->ring.prev = channel->ring.next = -1;
    channel->tree.up = -1;
    for (int i=0; i<NCCL_MAX_TREE_ARITY; i++) channel->tree.down[i] = -1;
    channel->collTree.out = -1;
    channel->collTree.headRank = -1;
    channel->collTree.nHeads = 0;
    channel->collTree.shift = 0;
    for (int i=0; i<NCCL_MAX_DIRECT_ARITY; i++) channel->collTree.up[i] = -1;
    for (int i=0; i<NCCL_MAX_DIRECT_ARITY; i++) channel->collTree.down[i] = -1;

    int* ringIntra = ringGraph->intra+c*localRanks;
    int* treeIntra = treeGraph->intra+c*localRanks;

    for (int i=0; i<localRanks; i++) {
      if (ringIntra[i] == rank) {
        topoRanks->ringRecv[c] = ringIntra[0];
        topoRanks->ringSend[c] = ringIntra[localRanks-1];
        channel->ring.prev = (i == 0) ? -1 : ringIntra[i-1];
        channel->ring.next = (i == localRanks-1) ? -1 : ringIntra[i+1];
      }
      if (treeIntra[i] == rank) {
        int parentIndex = 0;
        int child0Index = treeGraph->pattern == NCCL_TOPO_PATTERN_TREE ? 0 : 1;
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
  struct ncclChannel* channel0 = comm->channels;
  struct ncclChannel* channel1 = channel0+nChannels;
  memcpy(channel1, channel0, nChannels*sizeof(struct ncclChannel));
  return ncclSuccess;
}
```

</details>

<a id="f0430"></a>

## F0430：connectRings

用途：把各节点局部 GPU 链的出口接到下一节点入口，补全完整环的前驱/后继。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int nChannels = comm-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nNodes = comm-&gt;nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 准备局部状态 | <code>int* recv = ringRecv+c*comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 1 / 准备局部状态 | <code>int* send = ringSend+c*comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 准备局部状态 | <code>int* prev = ringPrev+c*comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 准备局部状态 | <code>int* next = ringNext+c*comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 准备局部状态 | <code>struct ncclChannel* channel0 = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 1 / 准备局部状态 | <code>struct ncclChannel* channel1 = channel0+nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 循环 | <code>初始 int n=0;；继续条件 n&lt;nNodes；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 2 / 准备局部状态 | <code>int recvRank = recv[firstRanks[n]];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 2 / 准备局部状态 | <code>int prevSendRank = send[firstRanks[(n-1+nNodes)%nNodes]];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>prev[recvRank] = prevSendRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 条件判断 | <code>(comm-&gt;rank == recvRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>channel0-&gt;ring.prev = prevSendRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>channel1-&gt;ring.prev = prevSendRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 2 / 准备局部状态 | <code>int sendRank = send[firstRanks[n]];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 2 / 准备局部状态 | <code>int nextRecvRank = recv[firstRanks[(n+1)%nNodes]];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>next[sendRank] = nextRecvRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 2 / 条件判断 | <code>(comm-&gt;rank == sendRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>channel0-&gt;ring.next = nextRecvRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>channel1-&gt;ring.next = nextRecvRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_GRAPH, "Ring %d : %d -&gt; %d -&gt; %d", c, channel0-&gt;ring.prev, comm-&gt;rank, channel0-&gt;ring.next);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_GRAPH, "Ring %d : %d -&gt; %d -&gt; %d", c+nChannels, channel1-&gt;ring.prev, comm-&gt;rank, channel1-&gt;ring.next);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 29 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int nChannels = comm->nChannels;
  int nNodes = comm->nNodes;
  for (int c=0; c<nChannels; c++) {
    int* recv = ringRecv+c*comm->nRanks;
    int* send = ringSend+c*comm->nRanks;
    int* prev = ringPrev+c*comm->nRanks;
    int* next = ringNext+c*comm->nRanks;
    struct ncclChannel* channel0 = comm->channels+c;
    struct ncclChannel* channel1 = channel0+nChannels;
    for (int n=0; n<nNodes; n++) {
      int recvRank = recv[firstRanks[n]];
      int prevSendRank = send[firstRanks[(n-1+nNodes)%nNodes]];
      prev[recvRank] = prevSendRank;
      if (comm->rank == recvRank) {
        channel0->ring.prev = prevSendRank;
        channel1->ring.prev = prevSendRank;
      }
      int sendRank = send[firstRanks[n]];
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
```

</details>

<a id="f0431"></a>

## F0431：getIndexes

用途：把树连接端点的用户 rank 映射到节点索引，供节点级双树关系查表。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;nNodes；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>indexes[n] = ranks[firstRanks[n]];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
 for (int n=0; n<nNodes; n++) indexes[n] = ranks[firstRanks[n]];
 return ncclSuccess;
}
```

</details>

<a id="f0432"></a>

## F0432：setTreeUp

用途：把节点级父端点映射成当前 GPU 树的 up 邻居，保留无父节点的 -1 语义。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(u == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>tree-&gt;up = indexes[u];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (u == -1) return ncclSuccess;
  tree->up = indexes[u];
  return ncclSuccess;
}
```

</details>

<a id="f0433"></a>

## F0433：setTreeDown

用途：把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(d == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>int x = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 循环 | <code>初始 ；继续条件 (x &lt; NCCL_MAX_TREE_ARITY && tree-&gt;down[x] &gt;= 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>x++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 条件判断 | <code>(x == NCCL_MAX_TREE_ARITY)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>WARN("Internal error : tree already has %d children (%d %d %d)", x, tree-&gt;down[0], tree-&gt;down[1], tree-&gt;down[2]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>tree-&gt;down[x] = indexes[d];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (d == -1) return ncclSuccess;
  int x = 0;
  while (x < NCCL_MAX_TREE_ARITY && tree->down[x] >= 0) x++;
  if (x == NCCL_MAX_TREE_ARITY) {
    WARN("Internal error : tree already has %d children (%d %d %d)", x, tree->down[0], tree->down[1], tree->down[2]);
    return ncclInternalError;
  }
  tree->down[x] = indexes[d];
  return ncclSuccess;
}
```

</details>

<a id="f0434"></a>

## F0434：connectTrees

用途：用互补二叉树连接各节点的本地树片段，并复制通道来分摊中间节点负载。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int nChannels = comm-&gt;nChannels, nNodes = comm-&gt;nNodes, node = comm-&gt;node;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int* ranksToParent, *ranksToChild0, *ranksToChild1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&ranksToParent, nNodes));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&ranksToChild0, nNodes));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&ranksToChild1, nNodes));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 6 | 0 / 准备局部状态 | <code>int depth = comm-&gt;nRanks/nNodes - 1 + log2i(nNodes);</code> | 用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。 |
| 7 | 0 / 准备局部状态 | <code>int t0u, t0d0, t0d1, t0ChildType, t1u, t1d0, t1d1, t1ChildType;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1, &t1ChildType));</code> | 失败时可能提前return，成功才继续；在第一棵二叉树基础上用镜像或平移构造互补树，分摊中间节点负载。 |
| 9 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 1 / 准备局部状态 | <code>struct ncclChannel* channel0 = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 1 / 准备局部状态 | <code>struct ncclChannel* channel1 = channel0+nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getIndexes(treeToParent+c*comm-&gt;nRanks, ranksToParent, nNodes, firstRanks));</code> | 失败时可能提前return，成功才继续；把树连接端点的用户 rank 映射到节点索引，供节点级双树关系查表。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getIndexes(treeToChild0+c*comm-&gt;nRanks, ranksToChild0, nNodes, firstRanks));</code> | 失败时可能提前return，成功才继续；把树连接端点的用户 rank 映射到节点索引，供节点级双树关系查表。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getIndexes(treeToChild1+c*comm-&gt;nRanks, ranksToChild1, nNodes, firstRanks));</code> | 失败时可能提前return，成功才继续；把树连接端点的用户 rank 映射到节点索引，供节点级双树关系查表。 |
| 15 | 1 / 条件判断 | <code>(comm-&gt;rank == ranksToParent[node])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(setTreeUp(&channel0-&gt;tree, t0ChildType == 0 ? ranksToChild0 : ranksToChild1, t0u));</code> | 失败时可能提前return，成功才继续；把节点级父端点映射成当前 GPU 树的 up 邻居，保留无父节点的 -1 语义。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(setTreeUp(&channel1-&gt;tree, t1ChildType == 0 ? ranksToChild0 : ranksToChild1, t1u));</code> | 失败时可能提前return，成功才继续；把节点级父端点映射成当前 GPU 树的 up 邻居，保留无父节点的 -1 语义。 |
| 19 | 1 / 条件判断 | <code>(comm-&gt;rank == ranksToChild0[node])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(setTreeDown(&channel0-&gt;tree, ranksToParent, t0d0));</code> | 失败时可能提前return，成功才继续；把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。 |
| 22 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(setTreeDown(&channel1-&gt;tree, ranksToParent, t1d0));</code> | 失败时可能提前return，成功才继续；把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。 |
| 23 | 1 / 条件判断 | <code>(comm-&gt;rank == ranksToChild1[node])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(setTreeDown(&channel0-&gt;tree, ranksToParent, t0d1));</code> | 失败时可能提前return，成功才继续；把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。 |
| 26 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(setTreeDown(&channel1-&gt;tree, ranksToParent, t1d1));</code> | 失败时可能提前return，成功才继续；把节点级孩子端点追加到树 down 槽，检查最大邻居度以避免越界。 |
| 27 | 1 / 条件判断 | <code>(comm-&gt;rank == ranksToParent[node] &#124;&#124; comm-&gt;rank == ranksToChild0[node] &#124;&#124; comm-&gt;rank == ranksToChild1[node])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Tree %d : %d -&gt; %d -&gt; %d/%d/%d", c, channel0-&gt;tree.up, comm-&gt;rank, channel0-&gt;tree.down[0], channel0-&gt;tree.down[1], channel0-&gt;tree.down[2]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Tree %d : %d -&gt; %d -&gt; %d/%d/%d", c+nChannels, channel1-&gt;tree.up, comm-&gt;rank, channel1-&gt;tree.down[0], channel1-&gt;tree.down[1], channel1-&gt;tree.down[2]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 1 / 调用 / 计算 / 写入 | <code>channel0-&gt;tree.depth = channel1-&gt;tree.depth = depth;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>free(ranksToParent);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>free(ranksToChild0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>free(ranksToChild1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const int nChannels = comm->nChannels, nNodes = comm->nNodes, node = comm->node;
  int* ranksToParent, *ranksToChild0, *ranksToChild1;
  NCCLCHECK(ncclCalloc(&ranksToParent, nNodes));
  NCCLCHECK(ncclCalloc(&ranksToChild0, nNodes));
  NCCLCHECK(ncclCalloc(&ranksToChild1, nNodes));

  // Compute tree depth. Not an exact value but a good approximation in most
  // cases
  int depth = comm->nRanks/nNodes - 1 + log2i(nNodes);

  int t0u, t0d0, t0d1, t0ChildType, t1u, t1d0, t1d1, t1ChildType;
  NCCLCHECK(ncclGetDtree(nNodes, node, &t0u, &t0d0, &t0d1, &t0ChildType, &t1u, &t1d0, &t1d1, &t1ChildType));
  for (int c=0; c<nChannels; c++) {
     struct ncclChannel* channel0 = comm->channels+c;
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
```

</details>

<a id="f0435"></a>

## F0435：connectCollNet

用途：把本节点 GPU 组织为多个网络 head 的分散/归约/广播/收集关系。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int rank = comm-&gt;rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int localRanks = comm-&gt;localRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int nHeads = collNetGraph-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int *heads;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&heads, nHeads));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 6 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;nHeads；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 准备局部状态 | <code>int* collNetIntra = collNetGraph-&gt;intra+c*localRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>heads[c] = collNetIntra[0];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 1 / 准备局部状态 | <code>char line[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>sprintf(line, "CollNet channel %d rank %d ", c, rank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 1 / 准备局部状态 | <code>int nDown = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nHeads；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 2 / 条件判断 | <code>(rank == heads[i])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.headRank = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 4 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.out = comm-&gt;nRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 4 / 准备局部状态 | <code>int* collNetIntra = collNetGraph-&gt;intra+i*localRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), "down ");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 4 / 循环 | <code>初始 int r=0;；继续条件 r&lt;localRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 23 | 5 / 条件判断 | <code>(collNetIntra[r] == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 26 | 5 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.down[nDown++] = collNetIntra[r];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 5 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), " %d ", collNetIntra[r]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), "nDown %d ", nDown);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 4 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 31 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 32 | 1 / 准备局部状态 | <code>int nUp = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 1 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), "up ");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 34 | 1 / 循环 | <code>初始 int h=0;；继续条件 h&lt;nHeads；每轮更新 h++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 35 | 2 / 条件判断 | <code>(rank == heads[h])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 4 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 38 | 2 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.up[nUp++] = heads[h];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 2 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), " %d ", heads[h]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 41 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.nHeads = nHeads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.shift = (rank%localRanks)%nHeads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;collTree.depth = (nUp == 0 && nDown == 0) ? 1 : 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 1 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), "nUp %d nHeads %d ", nUp, nHeads);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 1 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), "headRank %d out %d shift %d", channel-&gt;collTree.headRank, channel-&gt;collTree.out, channel-&gt;collTree.shift);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 1 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>free(heads);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int rank = comm->rank;
  int localRanks = comm->localRanks;
  int nHeads = collNetGraph->nChannels;
  int *heads;
  NCCLCHECK(ncclCalloc(&heads, nHeads));
  // Find all head ranks
  // Head index is always 0
  for (int c=0; c<nHeads; c++) {
    int* collNetIntra = collNetGraph->intra+c*localRanks;
    heads[c] = collNetIntra[0];
  }
  // For all channels
  for (int c=0; c<comm->nChannels; c++) {
    struct ncclChannel* channel = comm->channels+c;
    char line[1024];
    sprintf(line, "CollNet channel %d rank %d ", c, rank);
    int nDown = 0;
    for (int i=0; i<nHeads; i++) {
      if (rank == heads[i]) { // is head
        channel->collTree.headRank = i; // Mark the index for deciding offset in the CUDA kernel
        channel->collTree.out = comm->nRanks; // Set root of collTree to id nranks
        int* collNetIntra = collNetGraph->intra+i*localRanks;
        sprintf(line+strlen(line), "down ");
        for (int r=0; r<localRanks; r++) {
          if (collNetIntra[r] == rank) continue;
          channel->collTree.down[nDown++] = collNetIntra[r];  // connect to all peers
          sprintf(line+strlen(line), " %d ", collNetIntra[r]);
        }
        sprintf(line+strlen(line), "nDown %d ", nDown);
        break;
      }
    }
    // Connect to all heads
    int nUp = 0;
    sprintf(line+strlen(line), "up ");
    for (int h=0; h<nHeads; h++) {
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
```

</details>

<a id="f0436"></a>

## F0436：ncclMinNchannels

用途：读取并校验集合通信 channel 上下限，兼容旧 NRINGS 配置名后限制到编译期容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int minNchannels = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(ncclParamMinNrings() != -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>minNchannels = ncclParamMinNrings();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(ncclParamMinNchannels() != -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>minNchannels = ncclParamMinNchannels();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 条件判断 | <code>(minNchannels &gt; MAXCHANNELS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>WARN("User asked for a minimum of %d channels, limiting to %d", minNchannels, MAXCHANNELS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>minNchannels = MAXCHANNELS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 条件判断 | <code>(minNchannels &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>minNchannels = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return minNchannels;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f0437"></a>

## F0437：ncclMaxNchannels

用途：读取并校验集合通信 channel 上下限，兼容旧 NRINGS 配置名后限制到编译期容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int maxNchannels = MAXCHANNELS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(ncclParamMaxNrings() != -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>maxNchannels = ncclParamMaxNrings();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(ncclParamMaxNchannels() != -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>maxNchannels = ncclParamMaxNchannels();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 条件判断 | <code>(maxNchannels &gt; MAXCHANNELS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>maxNchannels = MAXCHANNELS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 条件判断 | <code>(maxNchannels &lt; 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>WARN("User asked for a maximum of %d channels, setting it to 1", maxNchannels);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>maxNchannels = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return maxNchannels;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f0438"></a>

## F0438：copyChannels

用途：把已有通道的邻居关系复制到新增逻辑 channel，使最小并行度要求有可用上下文。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int nranks = comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 c=start；继续条件 c&lt;end；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>memcpy(ringPrev+c*nranks, ringPrev+(c-start)*nranks, nranks*sizeof(int));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>memcpy(ringNext+c*nranks, ringNext+(c-start)*nranks, nranks*sizeof(int));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>memcpy(comm-&gt;channels+c, comm-&gt;channels+c-start, sizeof(struct ncclChannel));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 函数返回 | <code>return c;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int nranks = comm->nRanks;
  int c;
  for (c=start; c<end; c++) {
    memcpy(ringPrev+c*nranks, ringPrev+(c-start)*nranks, nranks*sizeof(int));
    memcpy(ringNext+c*nranks, ringNext+(c-start)*nranks, nranks*sizeof(int));
    memcpy(comm->channels+c, comm->channels+c-start, sizeof(struct ncclChannel));
  }
  return c;
}
```

</details>

<a id="f0439"></a>

## F0439：ncclTopoPostset

用途：汇总全部节点的端点信息并拼接全局 Ring/Tree，复制/裁剪通道后验证环完整性。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int *ringRecv, *ringSend, *ringPrev, *ringNext, *treeToParent, *treeToChild0, *treeToChild1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nranks = comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int nChannels = comm-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&ringRecv, nranks*MAXCHANNELS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&ringSend, nranks*MAXCHANNELS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&ringPrev, nranks*MAXCHANNELS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&ringNext, nranks*MAXCHANNELS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&treeToParent, nranks*MAXCHANNELS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&treeToChild0, nranks*MAXCHANNELS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&treeToChild1, nranks*MAXCHANNELS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 11 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 循环 | <code>初始 int c=0;；继续条件 c&lt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>ringRecv[c*nranks+i] = allTopoRanks[i]-&gt;ringRecv[c];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>ringSend[c*nranks+i] = allTopoRanks[i]-&gt;ringSend[c];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>ringPrev[c*nranks+i] = allTopoRanks[i]-&gt;ringPrev[c];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>ringNext[c*nranks+i] = allTopoRanks[i]-&gt;ringNext[c];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>treeToParent[c*nranks+i] = allTopoRanks[i]-&gt;treeToParent[c];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>treeToChild0[c*nranks+i] = allTopoRanks[i]-&gt;treeToChild0[c];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>treeToChild1[c*nranks+i] = allTopoRanks[i]-&gt;treeToChild1[c];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(connectRings(comm, ringRecv, ringSend, ringPrev, ringNext, firstRanks));</code> | 失败时可能提前return，成功才继续；把各节点局部 GPU 链的出口接到下一节点入口，补全完整环的前驱/后继。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(connectTrees(comm, treeToParent, treeToChild0, treeToChild1, firstRanks, treePatterns));</code> | 失败时可能提前return，成功才继续；用互补二叉树连接各节点的本地树片段，并复制通道来分摊中间节点负载。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>memcpy(ringPrev+nChannels*nranks, ringPrev, nChannels*nranks*sizeof(int));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>memcpy(ringNext+nChannels*nranks, ringNext, nChannels*nranks*sizeof(int));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>nChannels = comm-&gt;nChannels = std::min(MAXCHANNELS,nChannels*2);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 27 | 0 / 条件判断 | <code>(comm-&gt;collNetSupport == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 2 / 条件判断 | <code>(collNetGraph-&gt;speedIntra &gt; collNetGraph-&gt;speedInter && comm-&gt;nRanks &gt; comm-&gt;nNodes)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 4 / 准备局部状态 | <code>int collNetNchannels = std::min(MAXCHANNELS, nChannels+nChannels/2);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>nChannels = comm-&gt;nChannels = copyChannels(comm, nChannels, collNetNchannels, ringPrev, ringNext);</code> | 把已有通道的邻居关系复制到新增逻辑 channel，使最小并行度要求有可用上下文。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(connectCollNet(comm, collNetGraph));</code> | 失败时可能提前return，成功才继续；把本节点 GPU 组织为多个网络 head 的分散/归约/广播/收集关系。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>nChannels = comm-&gt;nChannels = std::min((int)ncclMaxNchannels(), nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>nChannels = comm-&gt;nChannels = copyChannels(comm, nChannels, ncclMinNchannels(), ringPrev, ringNext);</code> | 把已有通道的邻居关系复制到新增逻辑 channel，使最小并行度要求有可用上下文。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclBuildRings(nChannels, rings, comm-&gt;rank, comm-&gt;nRanks, ringPrev, ringNext));</code> | 失败时可能提前return，成功才继续；跟随 next 构建每 channel 环顺序，验证 nranks 步回到起点且没有遗漏参与 rank。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>free(ringRecv);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>free(ringSend);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>free(ringPrev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>free(ringNext);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>free(treeToParent);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>free(treeToChild0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>free(treeToChild1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Gather data from all ranks
  int *ringRecv, *ringSend, *ringPrev, *ringNext, *treeToParent, *treeToChild0, *treeToChild1;
  int nranks = comm->nRanks;
  int nChannels = comm->nChannels;
  NCCLCHECK(ncclCalloc(&ringRecv, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&ringSend, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&ringPrev, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&ringNext, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&treeToParent, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&treeToChild0, nranks*MAXCHANNELS));
  NCCLCHECK(ncclCalloc(&treeToChild1, nranks*MAXCHANNELS));
  for (int i=0; i<nranks; i++) {
    for (int c=0; c<nChannels;c++) {
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
```

</details>

