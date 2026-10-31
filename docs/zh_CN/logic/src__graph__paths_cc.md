# src/graph/paths.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0440"></a>

## F0440：getPath

用途：从指定源节点的 paths 缓存中找到目标硬件类型/id 对应的路径项。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;system-&gt;nodes[t].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 条件判断 | <code>(system-&gt;nodes[t].nodes[i].id == id)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 3 / 调用 / 计算 / 写入 | <code>*path = node-&gt;paths[t]+i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>WARN("Could not find node of type %d id %lx", t, id);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int i=0; i<system->nodes[t].count; i++) {
    if (system->nodes[t].nodes[i].id == id) {
      *path = node->paths[t]+i;
      return ncclSuccess;
    }
  }
  WARN("Could not find node of type %d id %lx", t, id);
  return ncclInternalError;
}
```

</details>

<a id="f0441"></a>

## F0441：ncclTopoSetPaths

用途：从硬件节点做带带宽与路径类型约束的广度优先扩展，建立到该节点的路径缓存。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(baseNode-&gt;paths[baseNode-&gt;type] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(baseNode-&gt;paths+baseNode-&gt;type, system-&gt;nodes[baseNode-&gt;type].count));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclTopoNodeList nodeList;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclTopoNodeList nextNodeList;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>nodeList.count = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>nodeList.list[0] = baseNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>nextNodeList.count = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 准备局部状态 | <code>struct ncclTopoLinkList* basePath;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getPath(system, baseNode, baseNode-&gt;type, baseNode-&gt;id, &basePath));</code> | 失败时可能提前return，成功才继续；从指定源节点的 paths 缓存中找到目标硬件类型/id 对应的路径项。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>basePath-&gt;count = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>basePath-&gt;width = LOC_WIDTH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>basePath-&gt;type = PATH_LOC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 循环 | <code>初始 ；继续条件 (nodeList.count)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>nextNodeList.count = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 1 / 循环 | <code>初始 int n=0;；继续条件 n&lt;nodeList.count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 2 / 准备局部状态 | <code>struct ncclTopoNode* node = nodeList.list[n];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 2 / 准备局部状态 | <code>struct ncclTopoLinkList* path;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getPath(system, node, baseNode-&gt;type, baseNode-&gt;id, &path));</code> | 失败时可能提前return，成功才继续；从指定源节点的 paths 缓存中找到目标硬件类型/id 对应的路径项。 |
| 20 | 2 / 循环 | <code>初始 int l=0;；继续条件 l&lt;node-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 21 | 3 / 准备局部状态 | <code>struct ncclTopoLink* link = node-&gt;links+l;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 3 / 准备局部状态 | <code>struct ncclTopoNode* remNode = link-&gt;remNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 3 / 条件判断 | <code>(remNode-&gt;paths[baseNode-&gt;type] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(remNode-&gt;paths+baseNode-&gt;type, system-&gt;nodes[baseNode-&gt;type].count));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 26 | 3 / 准备局部状态 | <code>struct ncclTopoLinkList* remPath;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getPath(system, remNode, baseNode-&gt;type, baseNode-&gt;id, &remPath));</code> | 失败时可能提前return，成功才继续；从指定源节点的 paths 缓存中找到目标硬件类型/id 对应的路径项。 |
| 28 | 3 / 准备局部状态 | <code>float width = std::min(path-&gt;width, link-&gt;width);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 29 | 3 / 条件判断 | <code>(node != baseNode && node-&gt;type == GPU && (ncclParamNvbDisable() &#124;&#124; link-&gt;type != LINK_NVL &#124;&#124; remNode-&gt;type != GPU &#124;&#124; path-&gt;count &gt; 1))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 32 | 3 / 条件判断 | <code>((remPath-&gt;width == 0 &#124;&#124; remPath-&gt;count &gt; path-&gt;count) && remPath-&gt;width &lt; width)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 5 / 循环 | <code>初始 int l=0;；继续条件 l&lt;remNode-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 35 | 6 / 条件判断 | <code>(remNode-&gt;links[l].remNode == node)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 8 / 调用 / 计算 / 写入 | <code>remPath-&gt;list[0] = remNode-&gt;links+l;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 8 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 39 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 40 | 5 / 条件判断 | <code>(remPath-&gt;list[0] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 7 / 调用 / 计算 / 写入 | <code>WARN("Failed to find reverse path from remNode %d/%lx nlinks %d to node %d/%lx", remNode-&gt;type, remNode-&gt;id, remNode-&gt;nlinks, node-&gt;type, node-&gt;id);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 7 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 44 | 5 / 循环 | <code>初始 int i=0;；继续条件 i&lt;path-&gt;count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 45 | 6 / 调用 / 计算 / 写入 | <code>remPath-&gt;list[i+1] = path-&gt;list[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 5 / 调用 / 计算 / 写入 | <code>remPath-&gt;count = path-&gt;count + 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 5 / 调用 / 计算 / 写入 | <code>remPath-&gt;width = width;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 5 / 准备局部状态 | <code>int type = link-&gt;type == LINK_NET ? LINK_LOC : link-&gt;type;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 50 | 5 / 条件判断 | <code>(node-&gt;type == PCI && remNode-&gt;type == PCI)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 7 / 调用 / 计算 / 写入 | <code>type = PATH_PXB;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 5 / 条件判断 | <code>(link-&gt;type == LINK_PCI && (node-&gt;type == CPU &#124;&#124; link-&gt;remNode-&gt;type == CPU))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 54 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 55 | 7 / 调用 / 计算 / 写入 | <code>type = PATH_PHB;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 5 / 条件判断 | <code>(node-&gt;type == GPU && path-&gt;type == PATH_NVL && type == PATH_NVL && remPath-&gt;count &gt; 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 7 / 调用 / 计算 / 写入 | <code>type = PATH_NVB;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 59 | 5 / 调用 / 计算 / 写入 | <code>remPath-&gt;type = std::max(path-&gt;type, type);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 5 / 准备局部状态 | <code>int i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 61 | 5 / 循环 | <code>初始 i=0；继续条件 i&lt;nextNodeList.count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 62 | 6 / 条件判断 | <code>(nextNodeList.list[i] == remNode)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 8 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 65 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 66 | 5 / 条件判断 | <code>(i == nextNodeList.count)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 67 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 68 | 7 / 调用 / 计算 / 写入 | <code>nextNodeList.list[nextNodeList.count++] = remNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 69 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 70 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 71 | 1 / 调用 / 计算 / 写入 | <code>memcpy(&nodeList, &nextNodeList, sizeof(nodeList));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 72 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 73 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (baseNode->paths[baseNode->type] == NULL) {
    NCCLCHECK(ncclCalloc(baseNode->paths+baseNode->type, system->nodes[baseNode->type].count));
  }

  // breadth-first search to set all paths to that node in the system
  struct ncclTopoNodeList nodeList;
  struct ncclTopoNodeList nextNodeList;
  nodeList.count = 1; nodeList.list[0] = baseNode;
  nextNodeList.count = 0;
  struct ncclTopoLinkList* basePath;
  NCCLCHECK(getPath(system, baseNode, baseNode->type, baseNode->id, &basePath));
  basePath->count = 0;
  basePath->width = LOC_WIDTH;
  basePath->type = PATH_LOC;

  while (nodeList.count) {
    nextNodeList.count = 0;
    for (int n=0; n<nodeList.count; n++) {
      struct ncclTopoNode* node = nodeList.list[n];
      struct ncclTopoLinkList* path;
      NCCLCHECK(getPath(system, node, baseNode->type, baseNode->id, &path));
      for (int l=0; l<node->nlinks; l++) {
        struct ncclTopoLink* link = node->links+l;
        struct ncclTopoNode* remNode = link->remNode;
        if (remNode->paths[baseNode->type] == NULL) {
          NCCLCHECK(ncclCalloc(remNode->paths+baseNode->type, system->nodes[baseNode->type].count));
        }
        struct ncclTopoLinkList* remPath;
        NCCLCHECK(getPath(system, remNode, baseNode->type, baseNode->id, &remPath));
        float width = std::min(path->width, link->width);

        // allow routing through a GPU only as 1 hop
        if (node != baseNode && node->type == GPU &&
            (ncclParamNvbDisable() || link->type != LINK_NVL || remNode->type != GPU || path->count > 1)) continue;

        if ((remPath->width == 0 || remPath->count > path->count) && remPath->width < width) {
          // Find reverse link
          for (int l=0; l<remNode->nlinks; l++) {
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
          for (int i=0; i<path->count; i++) remPath->list[i+1] = path->list[i];
          remPath->count = path->count + 1;
          remPath->width = width;

          // Start with path type = link type. PATH and LINK types are supposed to match.
          // Don't consider LINK_NET as we only care about the NIC->GPU path.
          int type = link->type == LINK_NET ? LINK_LOC : link->type;
          // Differentiate between one and multiple PCI switches
          if (node->type == PCI && remNode->type == PCI) type = PATH_PXB;
          // Consider a path going through the CPU as PATH_PHB
          if (link->type == LINK_PCI && (node->type == CPU || link->remNode->type == CPU)) type = PATH_PHB;
          // Set 1 hop NVLink as NVB
          if (node->type == GPU && path->type == PATH_NVL && type == PATH_NVL && remPath->count > 1) type = PATH_NVB;

          remPath->type = std::max(path->type, type);

          // Add to the list for the next iteration if not already in the list
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
```

</details>

<a id="f0442"></a>

## F0442：printNodePaths

用途：打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char line[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Paths from %s/%lX :", topoNodeTypeStr[node-&gt;type], node-&gt;id);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>sprintf(line, "%s/%lX :", topoNodeTypeStr[node-&gt;type], node-&gt;id);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 准备局部状态 | <code>int offset = strlen(line);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 循环 | <code>初始 int t=0;；继续条件 t&lt;NCCL_TOPO_NODE_TYPES；每轮更新 t++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 条件判断 | <code>(node-&gt;paths[t] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 11 | 1 / 循环 | <code>初始 int n = 0;；继续条件 n&lt;system-&gt;nodes[t].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 2 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 13 | 3 / 调用 / 计算 / 写入 | <code>line[0] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 3 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 3 / 循环 | <code>初始 int i=0;；继续条件 i&lt;node-&gt;paths[t][n].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 4 / 准备局部状态 | <code>struct ncclTopoLink* link = node-&gt;paths[t][n].list[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 4 / 准备局部状态 | <code>struct ncclTopoNode* remNode = link-&gt;remNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 4 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, "--%s-&gt;%s/%lX", topoLinkTypeStr[link-&gt;type], topoNodeTypeStr[remNode-&gt;type], remNode-&gt;id);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 4 / 调用 / 计算 / 写入 | <code>offset = strlen(line);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "%s (%f)", line, node-&gt;paths[t][n].width);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 3 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, "%s/%lX (%d/%f/%s) ", topoNodeTypeStr[t], system-&gt;nodes[t].nodes[n].id, node-&gt;paths[t][n].count, node-&gt;paths[t][n].width, topoPathTypeStr[node-&gt;paths[t][n].type]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>offset = strlen(line);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 26 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 0 / 编译期分支/指令 | <code>#ifndef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 28 | 1 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char line[1024];
#ifdef ENABLE_TRACE
  INFO(NCCL_GRAPH, "Paths from %s/%lX :", topoNodeTypeStr[node->type], node->id);
#else
  sprintf(line, "%s/%lX :", topoNodeTypeStr[node->type], node->id);
  int offset = strlen(line);
#endif
  for (int t=0; t<NCCL_TOPO_NODE_TYPES; t++) {
    if (node->paths[t] == NULL) continue;
    for (int n = 0; n<system->nodes[t].count; n++) {
#ifdef ENABLE_TRACE
      line[0] = 0;
      int offset = 0;
      for (int i=0; i<node->paths[t][n].count; i++) {
        struct ncclTopoLink* link = node->paths[t][n].list[i];
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
```

</details>

<a id="f0443"></a>

## F0443：ncclTopoPrintPaths

用途：打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;system-&gt;nodes[GPU].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>printNodePaths(system, system-&gt;nodes[GPU].nodes+i);</code> | 打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;system-&gt;nodes[NET].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>printNodePaths(system, system-&gt;nodes[NET].nodes+i);</code> | 打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int i=0; i<system->nodes[GPU].count; i++) {
    printNodePaths(system, system->nodes[GPU].nodes+i);
  }
  for (int i=0; i<system->nodes[NET].count; i++) {
    printNodePaths(system, system->nodes[NET].nodes+i);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0444"></a>

## F0444：getLocalCpu

用途：为 GPU/设备选择其局部 CPU 节点，供亲和性和通过 CPU 绕行的路径计算。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int minHops = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int localCpu = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclTopoLinkList* paths = system-&gt;nodes[GPU].nodes[gpu].paths[CPU];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;system-&gt;nodes[CPU].count；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 准备局部状态 | <code>int hops = paths[c].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 条件判断 | <code>(minHops == 0 &#124;&#124; hops &lt; minHops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>localCpu = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>minHops = hops;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 条件判断 | <code>(localCpu == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>WARN("Error : could not find CPU close to GPU %d", gpu);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>*retCpu = localCpu;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Find the closest CPU to a GPU
  int minHops = 0;
  int localCpu = -1;
  struct ncclTopoLinkList* paths = system->nodes[GPU].nodes[gpu].paths[CPU];
  for (int c=0; c<system->nodes[CPU].count; c++) {
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
```

</details>

<a id="f0445"></a>

## F0445：addInterStep

用途：用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclTopoNode* cpuNode = system-&gt;nodes[tx].nodes+ix;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclTopoNode* srcNode = system-&gt;nodes[t1].nodes+i1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int l=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;srcNode-&gt;paths[tx][ix].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>srcNode-&gt;paths[t2][i2].list[l++] = srcNode-&gt;paths[tx][ix].list[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;cpuNode-&gt;paths[t2][i2].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>srcNode-&gt;paths[t2][i2].list[l++] = cpuNode-&gt;paths[t2][i2].list[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>srcNode-&gt;paths[t2][i2].count = l;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>srcNode-&gt;paths[t2][i2].type = std::max(srcNode-&gt;paths[tx][ix].type, cpuNode-&gt;paths[t2][i2].type);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 条件判断 | <code>(tx == GPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>srcNode-&gt;paths[t2][i2].type = PATH_PXN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>srcNode-&gt;paths[t2][i2].width = std::min(srcNode-&gt;paths[tx][ix].width, cpuNode-&gt;paths[t2][i2].width);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclTopoNode* cpuNode = system->nodes[tx].nodes+ix;
  struct ncclTopoNode* srcNode = system->nodes[t1].nodes+i1;

  int l=0;
  // Node 1 -> CPU
  for (int i=0; i<srcNode->paths[tx][ix].count; i++) srcNode->paths[t2][i2].list[l++] = srcNode->paths[tx][ix].list[i];
  // CPU -> Node 2
  for (int i=0; i<cpuNode->paths[t2][i2].count; i++) srcNode->paths[t2][i2].list[l++] = cpuNode->paths[t2][i2].list[i];

  // Update path characteristics
  srcNode->paths[t2][i2].count = l;
  srcNode->paths[t2][i2].type = std::max(srcNode->paths[tx][ix].type, cpuNode->paths[t2][i2].type);
  if (tx == GPU) srcNode->paths[t2][i2].type = PATH_PXN;
  srcNode->paths[t2][i2].width = std::min(srcNode->paths[tx][ix].width, cpuNode->paths[t2][i2].width);
  return ncclSuccess;
}
```

</details>

<a id="f0446"></a>

## F0446：ncclTopoRemovePathType

用途：释放每个硬件节点到指定目标类型的路径缓存，避免拓扑裁剪后继续使用旧下标。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int t=0;；继续条件 t&lt;NCCL_TOPO_NODE_TYPES；每轮更新 t++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[t].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 2 / 准备局部状态 | <code>struct ncclTopoNode* node = system-&gt;nodes[t].nodes+n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>free(node-&gt;paths[nodeType]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>node-&gt;paths[nodeType] = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 1 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[nodeType].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 2 / 准备局部状态 | <code>struct ncclTopoNode* node = system-&gt;nodes[nodeType].nodes+n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>free(node-&gt;paths[t]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>node-&gt;paths[t] = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int t=0; t<NCCL_TOPO_NODE_TYPES; t++) {
    // Remove links _to_ the given type
    for (int n=0; n<system->nodes[t].count; n++) {
      struct ncclTopoNode* node = system->nodes[t].nodes+n;
      free(node->paths[nodeType]);
      node->paths[nodeType] = NULL;
    }
    // Remove links _from_ the given type
    for (int n=0; n<system->nodes[nodeType].count; n++) {
      struct ncclTopoNode* node = system->nodes[nodeType].nodes+n;
      free(node->paths[t]);
      node->paths[t] = NULL;
    }
  }
}
```

</details>

<a id="f0447"></a>

## F0447：ncclGetLevel

用途：把环境配置的拓扑距离名字或数字解析成内部路径级别，支持默认值与禁用策略。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(*level == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>int l = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 条件判断 | <code>(disableEnv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 准备局部状态 | <code>char* str = getenv(disableEnv);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 4 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 6 / 准备局部状态 | <code>int disable = strtol(str, NULL, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 6 / 条件判断 | <code>(disable == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 8 / 调用 / 计算 / 写入 | <code>l = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 条件判断 | <code>(l == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 准备局部状态 | <code>char* str = getenv(levelEnv);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 4 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 6 / 循环 | <code>初始 int i=0;；继续条件 i&lt;=PATH_SYS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 7 / 条件判断 | <code>(strcmp(str, topoPathTypeStr[i]) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 9 / 调用 / 计算 / 写入 | <code>l = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 9 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 23 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 24 | 6 / 条件判断 | <code>(l == -1 && str[0] &gt;= '0' && str[0] &lt;= '9')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 8 / 准备局部状态 | <code>int oldLevel = strtol(str, NULL, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 8 / 准备局部状态 | <code>const int maxOldLevel = sizeof(levelsOldToNew)/sizeof(int) - 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 8 / 条件判断 | <code>(oldLevel &gt; maxOldLevel)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 10 / 调用 / 计算 / 写入 | <code>oldLevel = maxOldLevel;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 8 / 调用 / 计算 / 写入 | <code>l = levelsOldToNew[oldLevel];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 2 / 条件判断 | <code>(l &gt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ALL, "%s set by environment to %s", levelEnv, topoPathTypeStr[l]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>*level = l &gt;= 0 ? l : -2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (*level == -1) {
    int l = -1;
    if (disableEnv) {
      char* str = getenv(disableEnv);
      if (str) {
        int disable = strtol(str, NULL, 0);
        if (disable == 1) l = 0;
      }
    }
    if (l == -1) {
      char* str = getenv(levelEnv);
      if (str) {
        for (int i=0; i<=PATH_SYS; i++) {
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
          int oldLevel = strtol(str, NULL, 0);
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
```

</details>

<a id="f0448"></a>

## F0448：ncclTopoCheckP2p

用途：结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*p2p = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(read)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>*read = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(intermediateRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>*intermediateRank = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 准备局部状态 | <code>int g1, g2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoIdToIndex(system, GPU, id1, &g1));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 10 | 0 / 准备局部状态 | <code>struct ncclTopoNode* gpu1 = system-&gt;nodes[GPU].nodes+g1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 条件判断 | <code>(ncclTopoIdToIndex(system, GPU, id2, &g2) == ncclInternalError)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 0 / 准备局部状态 | <code>int intermediateIndex = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 准备局部状态 | <code>struct ncclTopoLinkList* path = gpu1-&gt;paths[GPU]+g2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 条件判断 | <code>(path-&gt;count == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 准备局部状态 | <code>struct ncclTopoNode* intermediateNode = path-&gt;list[0]-&gt;remNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 2 / 条件判断 | <code>(intermediateNode-&gt;type == GPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>intermediateIndex = intermediateNode - system-&gt;nodes[GPU].nodes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 4 / 条件判断 | <code>(intermediateRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 6 / 调用 / 计算 / 写入 | <code>*intermediateRank = intermediateNode-&gt;gpu.rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 0 / 准备局部状态 | <code>int p2pLevel = PATH_SYS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 0 / 条件判断 | <code>(ncclTopoUserP2pLevel == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGetLevel(&ncclTopoUserP2pLevel, "NCCL_P2P_DISABLE", "NCCL_P2P_LEVEL"));</code> | 失败时可能提前return，成功才继续；把环境配置的拓扑距离名字或数字解析成内部路径级别，支持默认值与禁用策略。 |
| 29 | 0 / 条件判断 | <code>(ncclTopoUserP2pLevel != -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 2 / 调用 / 计算 / 写入 | <code>p2pLevel = ncclTopoUserP2pLevel;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 2 / 控制转移 | <code>goto compare;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 33 | 0 / 准备局部状态 | <code>int arch, vendor, model;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCpuType(system, &arch, &vendor, &model));</code> | 失败时可能提前return，成功才继续；从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。 |
| 35 | 0 / 条件判断 | <code>(arch == NCCL_TOPO_CPU_ARCH_ARM)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>p2pLevel = PATH_PXB;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 0 / 条件判断 | <code>(arch == NCCL_TOPO_CPU_ARCH_X86 && vendor == NCCL_TOPO_CPU_VENDOR_INTEL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 2 / 调用 / 计算 / 写入 | <code>p2pLevel = PATH_PXB;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 0 / 条件判断 | <code>(arch == NCCL_TOPO_CPU_ARCH_X86 && vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 2 / 调用 / 计算 / 写入 | <code>p2pLevel = PATH_PXB;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 0 / 跳转标签 | <code>compare</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 45 | 1 / 条件判断 | <code>(path-&gt;type &lt;= p2pLevel)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 3 / 调用 / 计算 / 写入 | <code>*p2p = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 条件判断 | <code>(*p2p == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 49 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 50 | 2 / 条件判断 | <code>(g1 != g2 && ncclParamIgnoreDisabledP2p() != 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 4 / 准备局部状态 | <code>int indexes[3] = {-1,-1,-1};</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 53 | 4 / 准备局部状态 | <code>int verticeN = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 55 | 4 / 调用 / 计算 / 写入 | <code>indexes[verticeN++] = system-&gt;nodes[GPU].nodes[g1].gpu.dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 4 / 条件判断 | <code>(intermediateIndex != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 6 / 调用 / 计算 / 写入 | <code>indexes[verticeN++] = system-&gt;nodes[GPU].nodes[intermediateIndex].gpu.dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 59 | 4 / 调用 / 计算 / 写入 | <code>indexes[verticeN++] = system-&gt;nodes[GPU].nodes[g2].gpu.dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 4 / 循环 | <code>初始 int i=1;；继续条件 i &lt; verticeN；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 61 | 5 / 准备局部状态 | <code>nvmlGpuP2PStatus_t status;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 62 | 5 / 调用 / 计算 / 写入 | <code>status = ncclNvmlDevicePairs[indexes[i-1]][indexes[i-0]].p2pStatusRead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 63 | 5 / 准备局部状态 | <code>bool good = status == NVML_P2P_STATUS_OK;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 64 | 5 / 调用 / 计算 / 写入 | <code>status = ncclNvmlDevicePairs[indexes[i-1]][indexes[i-0]].p2pStatusWrite;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 65 | 5 / 调用 / 计算 / 写入 | <code>good &= status == NVML_P2P_STATUS_OK;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 66 | 5 / 条件判断 | <code>(!good)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 67 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 68 | 7 / 条件判断 | <code>(ncclParamIgnoreDisabledP2p())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 69 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 70 | 9 / 调用 / 计算 / 写入 | <code>*p2p = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 71 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 72 | 9 / 条件判断 | <code>(path-&gt;type &lt;= PATH_NVB)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 73 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 74 | 11 / 调用 / 计算 / 写入 | <code>WARN("P2P is disabled between NVLINK connected GPUs %d and %d. This should not be the case given their connectivity, and is probably due to a hardware issue. If you still want to proceed, you can set NCCL_IGNORE_DISABLED_P2P=1.", indexes[i-1], indexes[i-0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 75 | 11 / 函数返回 | <code>return ncclUnhandledCudaError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 76 | 10 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 77 | 11 / 条件判断 | <code>(path-&gt;type &lt; PATH_SYS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 78 | 12 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 79 | 13 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "P2P is disabled between connected GPUs %d and %d. You can repress this message with NCCL_IGNORE_DISABLED_P2P=1.", indexes[i-1], indexes[i-0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 80 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 81 | 0 / 条件判断 | <code>(path-&gt;type == PATH_NVL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 82 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 83 | 2 / 准备局部状态 | <code>struct ncclTopoNode* gpu2 = system-&gt;nodes[GPU].nodes+g2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 84 | 2 / 条件判断 | <code>(read && (gpu1-&gt;gpu.cudaCompCap == gpu2-&gt;gpu.cudaCompCap) && (gpu1-&gt;gpu.cudaCompCap == 80))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 85 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 86 | 4 / 调用 / 计算 / 写入 | <code>*read = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 87 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *p2p = 0;
  if (read) *read = 0;
  if (intermediateRank) *intermediateRank = -1;

  // Get GPUs from topology
  int g1, g2;
  NCCLCHECK(ncclTopoIdToIndex(system, GPU, id1, &g1));
  struct ncclTopoNode* gpu1 = system->nodes[GPU].nodes+g1;
  if (ncclTopoIdToIndex(system, GPU, id2, &g2) == ncclInternalError) {
    // GPU not found, we can't use p2p.
    return ncclSuccess;
  }

  int intermediateIndex = -1;
  // Set intermediate GPU rank, if routing through an intermediate GPU.
  struct ncclTopoLinkList* path = gpu1->paths[GPU]+g2;
  if (path->count == 2) {
    struct ncclTopoNode* intermediateNode = path->list[0]->remNode;
    if (intermediateNode->type == GPU) {
      intermediateIndex = intermediateNode - system->nodes[GPU].nodes;
      if (intermediateRank) *intermediateRank = intermediateNode->gpu.rank;
    }
  }

  // In general, use P2P whenever we can.
  int p2pLevel = PATH_SYS;

  // User override
  if (ncclTopoUserP2pLevel == -1)
    NCCLCHECK(ncclGetLevel(&ncclTopoUserP2pLevel, "NCCL_P2P_DISABLE", "NCCL_P2P_LEVEL"));
  if (ncclTopoUserP2pLevel != -2) {
    p2pLevel = ncclTopoUserP2pLevel;
    goto compare;
  }

  // Don't use P2P through ARM CPUs
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
      int indexes[3] = {-1,-1,-1};
      int verticeN = 0;
      NCCLCHECK(ncclNvmlEnsureInitialized());

      indexes[verticeN++] = system->nodes[GPU].nodes[g1].gpu.dev;
      if (intermediateIndex != -1) indexes[verticeN++] = system->nodes[GPU].nodes[intermediateIndex].gpu.dev;
      indexes[verticeN++] = system->nodes[GPU].nodes[g2].gpu.dev;

      for (int i=1; i < verticeN; i++) {
        nvmlGpuP2PStatus_t status;
        status = ncclNvmlDevicePairs[indexes[i-1]][indexes[i-0]].p2pStatusRead;
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
    struct ncclTopoNode* gpu2 = system->nodes[GPU].nodes+g2;
    // Enable P2P Read for Ampere/NVLink only
    if (read && (gpu1->gpu.cudaCompCap == gpu2->gpu.cudaCompCap) && (gpu1->gpu.cudaCompCap == 80)) *read = 1;
  }

  return ncclSuccess;
}
```

</details>

<a id="f0449"></a>

## F0449：ncclTopoCheckGdr

用途：结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*useGdr = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 准备局部状态 | <code>int n, g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclTopoNode* net = system-&gt;nodes[NET].nodes+n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoIdToIndex(system, GPU, busId, &g));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclTopoNode* gpu = system-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 条件判断 | <code>(net-&gt;net.gdrSupport == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 条件判断 | <code>(gpu-&gt;gpu.gdrSupport == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 条件判断 | <code>(read)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 准备局部状态 | <code>int gdrReadParam = ncclParamNetGdrRead();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 2 / 条件判断 | <code>(gdrReadParam == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 19 | 2 / 条件判断 | <code>(gdrReadParam &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 4 / 准备局部状态 | <code>int nvlink = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 4 / 条件判断 | <code>(system-&gt;nodes[GPU].count == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 6 / 调用 / 计算 / 写入 | <code>nvlink = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 4 / 循环 | <code>初始 int i=0;；继续条件 i&lt;system-&gt;nodes[GPU].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 26 | 5 / 条件判断 | <code>(i == g)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 29 | 5 / 条件判断 | <code>(gpu-&gt;paths[GPU][i].type == PATH_NVL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 7 / 调用 / 计算 / 写入 | <code>nvlink = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 7 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 33 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 34 | 4 / 条件判断 | <code>(!nvlink)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 6 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 37 | 0 / 准备局部状态 | <code>int netGdrLevel = PATH_PXB;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGetLevel(&ncclTopoUserGdrLevel, NULL, "NCCL_NET_GDR_LEVEL"));</code> | 失败时可能提前return，成功才继续；把环境配置的拓扑距离名字或数字解析成内部路径级别，支持默认值与禁用策略。 |
| 39 | 0 / 条件判断 | <code>(ncclTopoUserGdrLevel != -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 2 / 调用 / 计算 / 写入 | <code>netGdrLevel = ncclTopoUserGdrLevel;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 准备局部状态 | <code>int distance = gpu-&gt;paths[NET][n].type;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 0 / 条件判断 | <code>(distance == PATH_PXN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 2 / 准备局部状态 | <code>int proxyRank, g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 46 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetIntermediateRank(system, gpu-&gt;gpu.rank, netDev, &proxyRank));</code> | 失败时可能提前return，成功才继续；找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。 |
| 47 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRankToIndex(system, proxyRank, &g));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 48 | 2 / 准备局部状态 | <code>struct ncclTopoNode* proxyGpu = system-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 2 / 调用 / 计算 / 写入 | <code>distance = proxyGpu-&gt;paths[NET][n].type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 0 / 条件判断 | <code>(distance &gt; netGdrLevel)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET,"GPU Direct RDMA Disabled for GPU %lx / HCA %d (distance %d &gt; %d)", busId, netDev, distance, netGdrLevel);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 53 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 54 | 0 / 调用 / 计算 / 写入 | <code>*useGdr = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET,"GPU Direct RDMA Enabled for GPU %lx / HCA %d (distance %d &lt;= %d), read %d", busId, netDev, distance, netGdrLevel, read);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *useGdr = 0;

  // Get GPU and NET
  int n, g;
  NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));
  struct ncclTopoNode* net = system->nodes[NET].nodes+n;
  NCCLCHECK(ncclTopoIdToIndex(system, GPU, busId, &g));
  struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;

  // Check that both the NIC and GPUs support it
  if (net->net.gdrSupport == 0) return ncclSuccess;
  if (gpu->gpu.gdrSupport == 0) return ncclSuccess;

  if (read) { // For reads (sends) only enable under certain conditions
    int gdrReadParam = ncclParamNetGdrRead();
    if (gdrReadParam == 0) return ncclSuccess;
    if (gdrReadParam < 0) {
      int nvlink = 0;
      // Since we don't know whether there are other communicators,
      // it's better to keep things local if we have a single GPU.
      if (system->nodes[GPU].count == 1) nvlink = 1;
      for (int i=0; i<system->nodes[GPU].count; i++) {
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
  int netGdrLevel = PATH_PXB;
  NCCLCHECK(ncclGetLevel(&ncclTopoUserGdrLevel, NULL, "NCCL_NET_GDR_LEVEL"));
  if (ncclTopoUserGdrLevel != -2) netGdrLevel = ncclTopoUserGdrLevel;
  int distance = gpu->paths[NET][n].type;
  if (distance == PATH_PXN) {
    // In case of PXN, use the intermediate GPU distance instead
    int proxyRank, g;
    NCCLCHECK(ncclTopoGetIntermediateRank(system, gpu->gpu.rank, netDev, &proxyRank));
    NCCLCHECK(ncclTopoRankToIndex(system, proxyRank, &g));
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
```

</details>

<a id="f0450"></a>

## F0450：ncclTopoGetIntermediateRank

用途：找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int n, g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclTopoNode* gpu = system-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclTopoLinkList* path = gpu-&gt;paths[NET]+n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(path-&gt;type == PATH_PXN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 准备局部状态 | <code>struct ncclTopoNode* node;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 准备局部状态 | <code>int type = NVS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;path-&gt;count && type == NVS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>node = path-&gt;list[i]-&gt;remNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>type = node-&gt;type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 2 / 条件判断 | <code>(type != GPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>WARN("Could not find intermediate GPU between GPU rank %d and NIC %d\n", rank, netDev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>*intermediateRank = node-&gt;gpu.rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>*intermediateRank = rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Get GPU and NET
  int n, g;
  NCCLCHECK(ncclTopoIdToIndex(system, NET, netDev, &n));
  NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));
  struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
  struct ncclTopoLinkList* path = gpu->paths[NET]+n;
  if (path->type == PATH_PXN) {
    struct ncclTopoNode* node;
    int type = NVS;
    for (int i=0; i<path->count && type == NVS; i++) {
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
```

</details>

<a id="f0451"></a>

## F0451：ncclPxnDisable

用途：汇总参数和环境条件判断是否禁止 PXN，避免选择当前后端不支持的中间 GPU 网络路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>static int pxnDisable = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(pxnDisable == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 条件判断 | <code>(ncclNetVersion() == 4)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "PXN Disabled as plugin is v4");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 4 / 调用 / 计算 / 写入 | <code>pxnDisable = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>pxnDisable = ncclParamPxnDisable();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 函数返回 | <code>return pxnDisable;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f0452"></a>

## F0452：ncclTopoGetPxnRanks

用途：列出本 rank 网络路径可能依赖的中间 GPU rank，为共享代理资源提前准备。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclTopoSystem* system = comm-&gt;topo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>*nranks = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*intermediateRanks = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(system-&gt;nodes[NET].count == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>int nr = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>int* ranks = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 循环 | <code>初始 int rank=0;；继续条件 rank&lt;comm-&gt;nRanks；每轮更新 rank++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 1 / 准备局部状态 | <code>int netDev, proxyRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNetDev(comm, comm-&gt;rank, NULL, 0, rank, &netDev, &proxyRank));</code> | 失败时可能提前return，成功才继续；按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。 |
| 12 | 1 / 条件判断 | <code>(proxyRank == comm-&gt;rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 15 | 1 / 准备局部状态 | <code>int useGdr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckGdr(comm-&gt;topo, comm-&gt;busId, netDev, 1, &useGdr));</code> | 失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。 |
| 17 | 1 / 条件判断 | <code>(useGdr == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 20 | 1 / 准备局部状态 | <code>int found = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 1 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nr；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 2 / 条件判断 | <code>(ranks[r] == proxyRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>found = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 26 | 1 / 条件判断 | <code>(!found)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclRealloc(&ranks, nr, nr+1));</code> | 失败时可能提前return，成功才继续；分配更大且尾部清零的 CPU 数组，复制旧元素后释放旧内存，不支持缩小。 |
| 29 | 3 / 调用 / 计算 / 写入 | <code>ranks[nr++] = proxyRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>*nranks = nr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>*intermediateRanks = ranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclTopoSystem* system = comm->topo;
  *nranks = 0;
  *intermediateRanks = NULL;
  if (system->nodes[NET].count == 0) return ncclSuccess;

  int nr = 0;
  int* ranks = NULL;
  for (int rank=0; rank<comm->nRanks; rank++) {
    int netDev, proxyRank;
    NCCLCHECK(ncclTopoGetNetDev(comm, comm->rank, NULL, 0, rank, &netDev, &proxyRank));
    if (proxyRank == comm->rank) continue;
    int useGdr;
    NCCLCHECK(ncclTopoCheckGdr(comm->topo, comm->busId, netDev, 1, &useGdr));
    if (useGdr == 0) continue;
    int found = 0;
    for (int r=0; r<nr; r++) {
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
```

</details>

<a id="f0453"></a>

## F0453：ncclTopoComputePaths

用途：计算 CPU/GPU/NET 路径，并按 P2P、GDR、PXN 策略替换不可直接使用的路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int t=0;；继续条件 t&lt;NCCL_TOPO_NODE_TYPES；每轮更新 t++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>ncclTopoRemovePathType(system, t);</code> | 释放每个硬件节点到指定目标类型的路径缓存，避免拓扑裁剪后继续使用旧下标。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 4 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;system-&gt;nodes[CPU].count；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSetPaths(system-&gt;nodes[CPU].nodes+c, system));</code> | 失败时可能提前return，成功才继续；从硬件节点做带带宽与路径类型约束的广度优先扩展，建立到该节点的路径缓存。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSetPaths(system-&gt;nodes[GPU].nodes+g, system));</code> | 失败时可能提前return，成功才继续；从硬件节点做带带宽与路径类型约束的广度优先扩展，建立到该节点的路径缓存。 |
| 9 | 1 / 循环 | <code>初始 int p=0;；继续条件 p&lt;system-&gt;nodes[GPU].count；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 2 / 准备局部状态 | <code>int p2p;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckP2p(system, system-&gt;nodes[GPU].nodes[p].id, system-&gt;nodes[GPU].nodes[g].id, &p2p, NULL, NULL));</code> | 失败时可能提前return，成功才继续；结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。 |
| 12 | 2 / 条件判断 | <code>(p2p == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 准备局部状态 | <code>int cpu;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getLocalCpu(system, g, &cpu));</code> | 失败时可能提前return，成功才继续；为 GPU/设备选择其局部 CPU 节点，供亲和性和通过 CPU 绕行的路径计算。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(addInterStep(system, CPU, cpu, GPU, p, GPU, g));</code> | 失败时可能提前return，成功才继续；用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。 |
| 17 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 1 / 条件判断 | <code>(peerInfos == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 21 | 1 / 准备局部状态 | <code>struct ncclPeerInfo* dstInfo = peerInfos+system-&gt;nodes[GPU].nodes[g].gpu.rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 1 / 循环 | <code>初始 int p=0;；继续条件 p&lt;system-&gt;nodes[GPU].count；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 23 | 2 / 条件判断 | <code>(p == g)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 4 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 26 | 2 / 准备局部状态 | <code>struct ncclPeerInfo* srcInfo = peerInfos+system-&gt;nodes[GPU].nodes[p].gpu.rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 2 / 准备局部状态 | <code>int shm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTransports[TRANSPORT_SHM].canConnect(&shm, system, NULL, srcInfo, dstInfo));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 29 | 2 / 准备局部状态 | <code>int p2p;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTransports[TRANSPORT_P2P].canConnect(&p2p, system, NULL, srcInfo, dstInfo));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 31 | 2 / 条件判断 | <code>(shm == 0 && p2p == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 4 / 调用 / 计算 / 写入 | <code>system-&gt;nodes[GPU].nodes[p].paths[GPU][g].count = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 35 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 36 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[NET].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 37 | 1 / 准备局部状态 | <code>struct ncclTopoNode* netNode = system-&gt;nodes[NET].nodes+n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSetPaths(netNode, system));</code> | 失败时可能提前return，成功才继续；从硬件节点做带带宽与路径类型约束的广度优先扩展，建立到该节点的路径缓存。 |
| 39 | 1 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 40 | 2 / 准备局部状态 | <code>struct ncclTopoNode* gpu = system-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 2 / 条件判断 | <code>(ncclPxnDisable() != 1 && gpu-&gt;paths[NET][n].type &gt; PATH_PXB)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 4 / 准备局部状态 | <code>int pxnGpu = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 4 / 循环 | <code>初始 int p=0;；继续条件 p&lt;system-&gt;nodes[GPU].count；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 45 | 5 / 条件判断 | <code>(p == g)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 48 | 5 / 准备局部状态 | <code>struct ncclTopoNode* peerNode = system-&gt;nodes[GPU].nodes+p;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 5 / 条件判断 | <code>(peerNode-&gt;paths[NET][n].type &gt; PATH_PXB &#124;&#124; peerNode-&gt;paths[GPU][g].type &gt; PATH_NVL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 52 | 5 / 调用 / 计算 / 写入 | <code>pxnGpu = p;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 5 / 准备局部状态 | <code>int netDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetLocalNet(system, peerNode-&gt;gpu.rank, &netDev));</code> | 失败时可能提前return，成功才继续；按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。 |
| 55 | 5 / 条件判断 | <code>(netDev == netNode-&gt;id)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 56 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 57 | 7 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 58 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 59 | 4 / 条件判断 | <code>(pxnGpu != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 60 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 61 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(addInterStep(system, GPU, pxnGpu, GPU, g, NET, n));</code> | 失败时可能提前return，成功才继续；用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。 |
| 62 | 2 / 准备局部状态 | <code>int gdr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 63 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckGdr(system, system-&gt;nodes[GPU].nodes[g].id, netNode-&gt;id, 0, &gdr));</code> | 失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。 |
| 64 | 2 / 条件判断 | <code>(gdr == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 65 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 66 | 4 / 准备局部状态 | <code>int localCpu;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 67 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getLocalCpu(system, g, &localCpu));</code> | 失败时可能提前return，成功才继续；为 GPU/设备选择其局部 CPU 节点，供亲和性和通过 CPU 绕行的路径计算。 |
| 68 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(addInterStep(system, CPU, localCpu, NET, n, GPU, g));</code> | 失败时可能提前return，成功才继续；用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。 |
| 69 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(addInterStep(system, CPU, localCpu, GPU, g, NET, n));</code> | 失败时可能提前return，成功才继续；用中间节点拼接两段已有路径，更新链路列表、瓶颈带宽与路径类别。 |
| 70 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 71 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 72 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Precompute paths between GPUs/NICs.

  // Remove everything in case we're re-computing
  for (int t=0; t<NCCL_TOPO_NODE_TYPES; t++) ncclTopoRemovePathType(system, t);

  // Set direct paths from/to CPUs. We need them in many cases.
  for (int c=0; c<system->nodes[CPU].count; c++) {
    NCCLCHECK(ncclTopoSetPaths(system->nodes[CPU].nodes+c, system));
  }

  // Set direct paths from/to GPUs.
  for (int g=0; g<system->nodes[GPU].count; g++) {
    // Compute paths to GPU g
    NCCLCHECK(ncclTopoSetPaths(system->nodes[GPU].nodes+g, system));

    // Update path when we don't want to / can't use GPU Direct P2P
    for (int p=0; p<system->nodes[GPU].count; p++) {
      int p2p;
      NCCLCHECK(ncclTopoCheckP2p(system, system->nodes[GPU].nodes[p].id, system->nodes[GPU].nodes[g].id, &p2p, NULL, NULL));
      if (p2p == 0) {
        // Divert all traffic through the CPU
        int cpu;
        NCCLCHECK(getLocalCpu(system, g, &cpu));
        NCCLCHECK(addInterStep(system, CPU, cpu, GPU, p, GPU, g));
      }
    }

    if (peerInfos == NULL) continue;
    // Remove GPUs we can't talk to because of containers.
    struct ncclPeerInfo* dstInfo = peerInfos+system->nodes[GPU].nodes[g].gpu.rank;
    for (int p=0; p<system->nodes[GPU].count; p++) {
      if (p == g) continue;
      struct ncclPeerInfo* srcInfo = peerInfos+system->nodes[GPU].nodes[p].gpu.rank;
      int shm;
      NCCLCHECK(ncclTransports[TRANSPORT_SHM].canConnect(&shm, system, NULL, srcInfo, dstInfo));
      int p2p;
      NCCLCHECK(ncclTransports[TRANSPORT_P2P].canConnect(&p2p, system, NULL, srcInfo, dstInfo));
      if (shm == 0 && p2p == 0) {
        // Mark this peer as inaccessible. We'll trim it later.
        system->nodes[GPU].nodes[p].paths[GPU][g].count = 0;
      }
    }
  }

  // Set direct paths from/to NICs.
  for (int n=0; n<system->nodes[NET].count; n++) {
    struct ncclTopoNode* netNode = system->nodes[NET].nodes+n;
    NCCLCHECK(ncclTopoSetPaths(netNode, system));

    for (int g=0; g<system->nodes[GPU].count; g++) {
      // Check whether we can access the NIC through another NVLink-connected GPU (PXN)
      struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
      if (ncclPxnDisable() != 1 && gpu->paths[NET][n].type > PATH_PXB) {
        int pxnGpu = -1;

        for (int p=0; p<system->nodes[GPU].count; p++) {
          if (p == g) continue;

          // PXN = PCI + NVLink.
          struct ncclTopoNode* peerNode = system->nodes[GPU].nodes+p;
          if (peerNode->paths[NET][n].type > PATH_PXB || peerNode->paths[GPU][g].type > PATH_NVL) continue;

          pxnGpu = p;

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
      int gdr;
      NCCLCHECK(ncclTopoCheckGdr(system, system->nodes[GPU].nodes[g].id, netNode->id, 0, &gdr));
      if (gdr == 0) {
        // We cannot use GPU Direct RDMA, divert all traffic through the CPU local to the GPU
        int localCpu;
        NCCLCHECK(getLocalCpu(system, g, &localCpu));
        NCCLCHECK(addInterStep(system, CPU, localCpu, NET, n, GPU, g));
        NCCLCHECK(addInterStep(system, CPU, localCpu, GPU, g, NET, n));
      }
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0454"></a>

## F0454：ncclTopoTrimSystem

用途：删除不属于当前通信可达域的硬件节点，并在完全机内通信时裁剪不需要的网络对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int *domains;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int64_t *ids;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&domains, system-&gt;nodes[GPU].count));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&ids, system-&gt;nodes[GPU].count));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 准备局部状态 | <code>int myDomain = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 准备局部状态 | <code>struct ncclTopoNode* gpu = system-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>domains[g] = g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>ids[g] = gpu-&gt;id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 1 / 循环 | <code>初始 int p=0;；继续条件 p&lt;g；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 2 / 条件判断 | <code>(gpu-&gt;paths[GPU][p].count &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>domains[g] = std::min(domains[g], domains[p]);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 14 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 1 / 条件判断 | <code>(gpu-&gt;gpu.rank == comm-&gt;rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>myDomain = domains[g];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ngpus；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 21 | 1 / 条件判断 | <code>(domains[i] == myDomain)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 24 | 1 / 准备局部状态 | <code>struct ncclTopoNode* gpu = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 1 / 准备局部状态 | <code>int g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 1 / 循环 | <code>初始 g=0；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>gpu = system-&gt;nodes[GPU].nodes+g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 2 / 条件判断 | <code>(gpu-&gt;id == ids[i])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 4 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 31 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>gpu=NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 34 | 1 / 条件判断 | <code>(gpu == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 3 / 调用 / 计算 / 写入 | <code>WARN("Could not find id %lx", ids[i]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 37 | 3 / 调用 / 计算 / 写入 | <code>free(domains);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 3 / 调用 / 计算 / 写入 | <code>free(ids);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 40 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRemoveNode(system, GPU, g));</code> | 失败时可能提前return，成功才继续；删除硬件节点并修正指向它的连接及数组位置，避免后续搜索引用被裁掉对象。 |
| 41 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 42 | 0 / 条件判断 | <code>(system-&gt;nodes[GPU].count == comm-&gt;nRanks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 2 / 循环 | <code>初始 int n=system-&gt;nodes[NET].count-1;；继续条件 n&gt;=0；每轮更新 n--</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 45 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRemoveNode(system, NET, n));</code> | 失败时可能提前return，成功才继续；删除硬件节点并修正指向它的连接及数组位置，避免后续搜索引用被裁掉对象。 |
| 46 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>free(domains);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>free(ids);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int *domains;
  int64_t *ids;
  NCCLCHECK(ncclCalloc(&domains, system->nodes[GPU].count));
  NCCLCHECK(ncclCalloc(&ids, system->nodes[GPU].count));
  int myDomain = 0;
  for (int g=0; g<system->nodes[GPU].count; g++) {
    struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
    domains[g] = g;
    ids[g] = gpu->id;
    for (int p=0; p<g; p++) {
      if (gpu->paths[GPU][p].count > 0) {
        domains[g] = std::min(domains[g], domains[p]);
      }
    }
    if (gpu->gpu.rank == comm->rank) myDomain = domains[g];
  }

  int ngpus = system->nodes[GPU].count;
  for (int i=0; i<ngpus; i++) {
    if (domains[i] == myDomain) continue;
    struct ncclTopoNode* gpu = NULL;
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
    for (int n=system->nodes[NET].count-1; n>=0; n--)
      NCCLCHECK(ncclTopoRemoveNode(system, NET, n));
  }
  free(domains);
  free(ids);
  return ncclSuccess;
}
```

</details>

<a id="f0455"></a>

## F0455：ncclTopoFree

用途：回收硬件系统的所有路径缓存和系统对象，本身不关闭 transport 已建立的连接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int t=0;；继续条件 t&lt;NCCL_TOPO_NODE_TYPES；每轮更新 t++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>ncclTopoRemovePathType(system, t);</code> | 释放每个硬件节点到指定目标类型的路径缓存，避免拓扑裁剪后继续使用旧下标。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>free(system);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int t=0; t<NCCL_TOPO_NODE_TYPES; t++) ncclTopoRemovePathType(system, t);
  free(system);
}
```

</details>

<a id="f0456"></a>

## F0456：ncclTopoGetNchannels

用途：按 peer 的本地/远端属性和 NVLink 带宽估计点对点通道预算，self 特殊处理。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int peer;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclTopoLinkList* path = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(ncclTopoRankToIndex(system, peerRank, &peer) == ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 条件判断 | <code>(g == peer)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 4 / 调用 / 计算 / 写入 | <code>*nChannels = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>path = system-&gt;nodes[GPU].nodes[peer].paths[GPU]+g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 条件判断 | <code>(path-&gt;type == PATH_NVL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 4 / 准备局部状态 | <code>float nvlWidth = ncclTopoNVLinkSpeed(system-&gt;nodes[GPU].nodes[g].gpu.cudaCompCap);</code> | 按 GPU 计算能力选择该代 NVLink 单链估计速率，供路径通道预算使用。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>*nChannels = 2*std::max(1, (int)(path-&gt;width / nvlWidth));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>*nChannels = 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>*nChannels = ncclParamNChannelsPerNetPeer();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int peer;
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
```

</details>

<a id="f0457"></a>

## F0457：nextPow2

用途：向上取最小二次幂，为 P2P 通道映射和位反转分布提供便于计算的容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int pow2 = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 ；继续条件 (pow2 &lt; v)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>pow2 &lt;&lt;= 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 5 | 0 / 函数返回 | <code>return pow2;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int pow2 = 1;
  while (pow2 < v) pow2 <<= 1;
  return pow2;
}
```

</details>

<a id="f0458"></a>

## F0458：ncclTopoComputeP2pChannels

用途：收敛所有本地 GPU 到 peer 的通道预算，计算全局 P2P 空间与每 peer 数量并生成位反转偏移。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pnChannels = std::min(comm-&gt;nChannels, (int)ncclParamMaxP2pNChannels());</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pnChannels = std::max(comm-&gt;p2pnChannels, (int)ncclParamMinP2pNChannels());</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 准备局部状态 | <code>int minChannels = comm-&gt;p2pnChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;comm-&gt;topo-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;nRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 2 / 准备局部状态 | <code>int nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNchannels(comm-&gt;topo, g, r, &nChannels));</code> | 失败时可能提前return，成功才继续；按 peer 的本地/远端属性和 NVLink 带宽估计点对点通道预算，self 特殊处理。 |
| 8 | 2 / 条件判断 | <code>(nChannels &gt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 4 / 调用 / 计算 / 写入 | <code>minChannels = std::min(minChannels, nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 11 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pnChannelsPerPeer = nextPow2(minChannels);</code> | 向上取最小二次幂，为 P2P 通道映射和位反转分布提供便于计算的容量。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pnChannels = nextPow2(comm-&gt;p2pnChannels);</code> | 向上取最小二次幂，为 P2P 通道映射和位反转分布提供便于计算的容量。 |
| 15 | 0 / 循环 | <code>初始 int c=comm-&gt;nChannels;；继续条件 c&lt;comm-&gt;p2pnChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(initChannel(comm, c));</code> | 失败时可能提前return，成功才继续；为单条逻辑 channel 分配环顺序表、peer 连接表和 GPU 可读取的工作描述符 FIFO。 |
| 17 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;p2pnChannelsPerPeer；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 1 / 准备局部状态 | <code>int mirror = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 1 / 循环 | <code>初始 int b=1, mb=(comm-&gt;p2pnChannels&gt;&gt;1);；继续条件 b&lt;comm-&gt;p2pnChannels；每轮更新 b&lt;&lt;=1, mb&gt;&gt;=1</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 21 | 2 / 条件判断 | <code>(c & b)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>mirror &#124;= mb;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pChannels[c] = mirror;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "%d coll channels, %d p2p channels, %d p2p channels per peer", comm-&gt;nChannels, comm-&gt;p2pnChannels, comm-&gt;p2pnChannelsPerPeer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  comm->p2pnChannels = std::min(comm->nChannels, (int)ncclParamMaxP2pNChannels());
  comm->p2pnChannels = std::max(comm->p2pnChannels, (int)ncclParamMinP2pNChannels());
  int minChannels = comm->p2pnChannels;
  // We need to loop through all local GPUs to have a global picture
  for (int g=0; g<comm->topo->nodes[GPU].count; g++) {
    for (int r=0; r<comm->nRanks; r++) {
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
  for (int c=comm->nChannels; c<comm->p2pnChannels; c++) NCCLCHECK(initChannel(comm, c));

  // We want to spread channels used when there aren't many and progressively
  // fill the whole space of nChannels. To do so we mirror the bits in the
  // nChannels space.
  for (int c=0; c<comm->p2pnChannelsPerPeer; c++) {
    int mirror = 0;
    for (int b=1, mb=(comm->p2pnChannels>>1); b<comm->p2pnChannels; b<<=1, mb>>=1) if (c & b) mirror |= mb;
    comm->p2pChannels[c] = mirror;
  }
  INFO(NCCL_INIT, "%d coll channels, %d p2p channels, %d p2p channels per peer", comm->nChannels, comm->p2pnChannels, comm->p2pnChannelsPerPeer);
  return ncclSuccess;
}
```

</details>

<a id="f0459"></a>

## F0459：ncclTopoGetNvbGpus

用途：找出可经一个中间 GPU 走 NVLink 桥接的 peer，为特定机内路径预连接提供候选。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(ranks, ngpus));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 准备局部状态 | <code>int nvbGpus = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;ngpus；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 准备局部状态 | <code>struct ncclTopoNode* gpu = system-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 条件判断 | <code>(gpu-&gt;gpu.rank != rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 9 | 1 / 循环 | <code>初始 int p=0;；继续条件 p&lt;ngpus；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 2 / 条件判断 | <code>(gpu-&gt;paths[GPU][p].type == PATH_NVB)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>(*ranks)[nvbGpus++] = system-&gt;nodes[GPU].nodes[p].gpu.rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>*nranks = nvbGpus;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int ngpus = system->nodes[GPU].count;
  NCCLCHECK(ncclCalloc(ranks, ngpus));
  int nvbGpus = 0;
  for (int g=0; g<ngpus; g++) {
    struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
    if (gpu->gpu.rank != rank) continue;
    for (int p=0; p<ngpus; p++) {
      if (gpu->paths[GPU][p].type == PATH_NVB) {
        (*ranks)[nvbGpus++] = system->nodes[GPU].nodes[p].gpu.rank;
      }
    }
  }
  *nranks = nvbGpus;
  return ncclSuccess;
}
```

</details>

