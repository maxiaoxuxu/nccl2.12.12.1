# src/graph/search.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0462"></a>

## F0462：getMaxWidth

用途：查看当前 GPU 到指定硬件类型的候选路径，获得可供搜索的最大单路径带宽。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>float maxWidth = 0.0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;system-&gt;nodes[type].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 准备局部状态 | <code>struct ncclTopoLinkList* path = gpu-&gt;paths[type]+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 准备局部状态 | <code>float width = path-&gt;width;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 1 / 条件判断 | <code>(path-&gt;count == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>maxWidth = std::max(maxWidth, width);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 函数返回 | <code>return maxWidth;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  float maxWidth = 0.0;
  for (int i=0; i<system->nodes[type].count; i++) {
    struct ncclTopoLinkList* path = gpu->paths[type]+i;
    float width = path->width;
    if (path->count == 0) continue;
    maxWidth = std::max(maxWidth, width);
  }
  return maxWidth;
}
```

</details>

<a id="f0463"></a>

## F0463：getTotalWidth

用途：估计当前 GPU 可同时利用的链路总带宽，作为多 channel 搜索的容量上界。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>float nvlinkWidth = 0.0, pciWidth = 0.0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int l=0;；继续条件 l&lt;gpu-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 准备局部状态 | <code>struct ncclTopoLink* link = gpu-&gt;links+l;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 条件判断 | <code>(link-&gt;type == LINK_NVL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>nvlinkWidth += link-&gt;width;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 1 / 条件判断 | <code>(link-&gt;type == LINK_PCI)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>pciWidth = link-&gt;width;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 函数返回 | <code>return std::max(pciWidth, nvlinkWidth);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  float nvlinkWidth = 0.0, pciWidth = 0.0;
  for (int l=0; l<gpu->nlinks; l++) {
    struct ncclTopoLink* link = gpu->links+l;
    if (link->type == LINK_NVL) nvlinkWidth += link->width;
    if (link->type == LINK_PCI) pciWidth = link->width;
  }
  return std::max(pciWidth, nvlinkWidth);
}
```

</details>

<a id="f0464"></a>

## F0464：ncclTopoSearchInit

用途：初始化拓扑搜索的最大宽度和总宽度，建立速度候选与提前终止判断依据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>system-&gt;maxWidth = 0.0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>system-&gt;totalWidth = 0.0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 准备局部状态 | <code>int inter = system-&gt;nodes[NET].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(inter == 0 && system-&gt;nodes[GPU].count == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>system-&gt;maxWidth = LOC_WIDTH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 准备局部状态 | <code>struct ncclTopoNode* gpu = system-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>system-&gt;maxWidth = std::max(system-&gt;maxWidth, getMaxWidth(system, gpu, inter ? NET : GPU));</code> | 查看当前 GPU 到指定硬件类型的候选路径，获得可供搜索的最大单路径带宽。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>system-&gt;totalWidth = std::max(system-&gt;totalWidth, getTotalWidth(system, gpu));</code> | 估计当前 GPU 可同时利用的链路总带宽，作为多 channel 搜索的容量上界。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  system->maxWidth = 0.0;
  system->totalWidth = 0.0;
  int inter = system->nodes[NET].count;
  if (inter == 0 && system->nodes[GPU].count == 1) {
    system->maxWidth = LOC_WIDTH;
    return ncclSuccess;
  }
  for (int g=0; g<system->nodes[GPU].count; g++) {
    struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
    system->maxWidth = std::max(system->maxWidth, getMaxWidth(system, gpu, inter ? NET : GPU));
    system->totalWidth = std::max(system->totalWidth, getTotalWidth(system, gpu));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0465"></a>

## F0465：findRevLink

用途：在对端节点中找回指向原节点的反向链路，配合双向带宽占用/恢复。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int l=0;；继续条件 l&lt;node2-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclTopoLink* link = node2-&gt;links+l;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 条件判断 | <code>(link-&gt;remNode == node1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>*revLink = link;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>WARN("Could not find rev link for %d/%ld -&gt; %d/%ld", node1-&gt;type, node1-&gt;id, node2-&gt;type, node2-&gt;id);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int l=0; l<node2->nlinks; l++) {
    struct ncclTopoLink* link = node2->links+l;
    if (link->remNode == node1) {
      *revLink = link;
      return ncclSuccess;
    }
  }
  WARN("Could not find rev link for %d/%ld -> %d/%ld", node1->type, node1->id, node2->type, node2->id);
  return ncclInternalError;
}
```

</details>

<a id="f0466"></a>

## F0466：followPath

用途：沿硬件路径扣减候选 channel 所需带宽，若失败则报告已走步数以便恢复；负 speed 用于回溯。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>float pciSpeed = speed;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int step=0;；继续条件 step&lt;path-&gt;count；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 准备局部状态 | <code>struct ncclTopoNode* node = path-&gt;list[step]-&gt;remNode;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 1 / 条件判断 | <code>(node-&gt;type == CPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 条件判断 | <code>(path-&gt;type == PATH_PHB && start-&gt;type == GPU && node-&gt;cpu.arch == NCCL_TOPO_CPU_ARCH_X86 && node-&gt;cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 调用 / 计算 / 写入 | <code>pciSpeed = INTEL_P2P_OVERHEAD(speed);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 准备局部状态 | <code>struct ncclTopoNode* node = start;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 循环 | <code>初始 int step=0;；继续条件 step&lt;maxSteps；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 准备局部状态 | <code>struct ncclTopoLink* link = path-&gt;list[step];</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 13 | 1 / 准备局部状态 | <code>struct ncclTopoLink* revLink = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 1 / 准备局部状态 | <code>float fwSpeed = link-&gt;type == LINK_PCI ? pciSpeed : speed;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 1 / 准备局部状态 | <code>float revSpeed = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 1 / 条件判断 | <code>(link-&gt;remNode-&gt;type == GPU && link-&gt;remNode-&gt;gpu.cudaCompCap &lt; 80 && start-&gt;type != GPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 3 / 条件判断 | <code>(revLink == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(findRevLink(node, link-&gt;remNode, &revLink));</code> | 失败时可能提前return，成功才继续；在对端节点中找回指向原节点的反向链路，配合双向带宽占用/恢复。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>revSpeed += fwSpeed/8;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 1 / 条件判断 | <code>(link-&gt;remNode-&gt;type == CPU && link-&gt;type == LINK_NVL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 3 / 条件判断 | <code>(revLink == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(findRevLink(node, link-&gt;remNode, &revLink));</code> | 失败时可能提前return，成功才继续；在对端节点中找回指向原节点的反向链路，配合双向带宽占用/恢复。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>revSpeed += fwSpeed;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 1 / 条件判断 | <code>(link-&gt;width &lt; fwSpeed &#124;&#124; (revSpeed && revLink-&gt;width &lt; revSpeed))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>*steps = step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 31 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 32 | 1 / 调用 / 计算 / 写入 | <code>SUB_ROUND(link-&gt;width, fwSpeed);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 1 / 条件判断 | <code>(revSpeed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>SUB_ROUND(revLink-&gt;width, revSpeed);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 1 / 调用 / 计算 / 写入 | <code>node = link-&gt;remNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>*steps = maxSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  float pciSpeed = speed;
  for (int step=0; step<path->count; step++) {
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

  struct ncclTopoNode* node = start;
  for (int step=0; step<maxSteps; step++) {
    struct ncclTopoLink* link = path->list[step];
    struct ncclTopoLink* revLink = NULL;
    float fwSpeed = link->type == LINK_PCI ? pciSpeed : speed;
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
```

</details>

<a id="f0467"></a>

## F0467：ncclTopoFollowPath

用途：包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*node = system-&gt;nodes[type2].nodes+index2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(type1 == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclTopoNode* node1 = system-&gt;nodes[type1].nodes+index1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclTopoLinkList* path = node1-&gt;paths[type2]+index2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 条件判断 | <code>(path-&gt;count == 0 )</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>*node = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 准备局部状态 | <code>int intra = type1 == GPU && type2 == GPU;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>float speed = intra ? graph-&gt;speedIntra : graph-&gt;speedInter;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>int type = intra ? graph-&gt;typeIntra : graph-&gt;typeInter;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 条件判断 | <code>(mult == 1 && (path-&gt;type &gt; type))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>speed *= mult;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 准备局部状态 | <code>int step = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(followPath(path, node1, path-&gt;count, speed, &step));</code> | 失败时可能提前return，成功才继续；沿硬件路径扣减候选 channel 所需带宽，若失败则报告已走步数以便恢复；负 speed 用于回溯。 |
| 20 | 0 / 条件判断 | <code>(step &lt; path-&gt;count)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 2 / 控制转移 | <code>goto rewind;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;nHops += mult*path-&gt;count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>*node = system-&gt;nodes[type2].nodes+index2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 0 / 跳转标签 | <code>rewind</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(followPath(path, node1, step, -speed, &step));</code> | 失败时可能提前return，成功才继续；沿硬件路径扣减候选 channel 所需带宽，若失败则报告已走步数以便恢复；负 speed 用于回溯。 |
| 28 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // First handle easy cases
  *node = system->nodes[type2].nodes+index2;
  if (type1 == -1) return ncclSuccess;
  struct ncclTopoNode* node1 = system->nodes[type1].nodes+index1;
  struct ncclTopoLinkList* path = node1->paths[type2]+index2;
  if (path->count == 0 ) return ncclSuccess;

  // Now check link type
  *node = NULL;
  int intra = type1 == GPU && type2 == GPU;
  float speed = intra ? graph->speedIntra : graph->speedInter;
  int type = intra ? graph->typeIntra : graph->typeInter;

  if (mult == 1 && (path->type > type)) return ncclSuccess;

  speed *= mult;

  // Check there is enough bandwidth on paths.
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
```

</details>

<a id="f0468"></a>

## F0468：gpuPciWidth

用途：统计 GPU 的 PCI 通路宽度，为下一 GPU 候选排序加入 PCI 容量因素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int l=0;；继续条件 l&lt;gpu-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclTopoLink* gpuLink = gpu-&gt;links+l;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 条件判断 | <code>(gpuLink-&gt;type != LINK_PCI)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 6 | 1 / 准备局部状态 | <code>struct ncclTopoNode* pci = gpuLink-&gt;remNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 循环 | <code>初始 int l=0;；继续条件 l&lt;pci-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 2 / 准备局部状态 | <code>struct ncclTopoLink* pciLink = pci-&gt;links+l;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 条件判断 | <code>(pciLink-&gt;remNode != gpu)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 12 | 2 / 函数返回 | <code>return std::min(gpuLink-&gt;width, pciLink-&gt;width);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int l=0; l<gpu->nlinks; l++) {
    struct ncclTopoLink* gpuLink = gpu->links+l;
    if (gpuLink->type != LINK_PCI) continue;
    struct ncclTopoNode* pci = gpuLink->remNode;
    for (int l=0; l<pci->nlinks; l++) {
      struct ncclTopoLink* pciLink = pci->links+l;
      if (pciLink->remNode != gpu) continue;
      return std::min(gpuLink->width, pciLink->width);
    }
  }
  return -1;
}
```

</details>

<a id="f0469"></a>

## F0469：cmpScore

用途：比较 GPU 候选的网络宽度、机内宽度与跳数等评分，改变搜索先后以更快找到好图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclGpuScore *s1 = (struct ncclGpuScore*)g1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclGpuScore *s2 = (struct ncclGpuScore*)g2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int d;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>((d = (s2-&gt;interWidth - s1-&gt;interWidth)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return d;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 条件判断 | <code>((d = (s2-&gt;interPciWidth - s1-&gt;interPciWidth)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 函数返回 | <code>return d;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 条件判断 | <code>((d = (s1-&gt;interNhops - s2-&gt;interNhops)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 函数返回 | <code>return d;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 条件判断 | <code>((d = (s2-&gt;intraWidth - s1-&gt;intraWidth)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 函数返回 | <code>return d;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 16 | 0 / 条件判断 | <code>((d = (s1-&gt;intraNhops - s2-&gt;intraNhops)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 函数返回 | <code>return d;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 19 | 0 / 函数返回 | <code>return s1-&gt;startIndex - s2-&gt;startIndex;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
   struct ncclGpuScore *s1 = (struct ncclGpuScore*)g1;
   struct ncclGpuScore *s2 = (struct ncclGpuScore*)g2;
   int d;
   if ((d = (s2->interWidth - s1->interWidth))) return d;
   if ((d = (s2->interPciWidth - s1->interPciWidth))) return d;
   if ((d = (s1->interNhops - s2->interNhops))) return d;
   if ((d = (s2->intraWidth - s1->intraWidth))) return d;
   if ((d = (s1->intraNhops - s2->intraNhops))) return d;
   return s1->startIndex - s2->startIndex;
}
```

</details>

<a id="f0470"></a>

## F0470：cmpIntraScores

用途：比较 GPU 候选的网络宽度、机内宽度与跳数等评分，改变搜索先后以更快找到好图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int intraWidth = scores[0].intraWidth;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int intraNhops = scores[0].intraNhops;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int i=1;；继续条件 i&lt;count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(scores[i].intraWidth != intraWidth &#124;&#124; scores[i].intraNhops != intraNhops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 函数返回 | <code>return 1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 函数返回 | <code>return 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int intraWidth = scores[0].intraWidth;
  int intraNhops = scores[0].intraNhops;
  for (int i=1; i<count; i++) {
    if (scores[i].intraWidth != intraWidth || scores[i].intraNhops != intraNhops) return 1;
  }
  return 0;
}
```

</details>

<a id="f0471"></a>

## F0471：getGpuIndex

用途：把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 条件判断 | <code>(system-&gt;nodes[GPU].nodes[g].gpu.rank == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 3 / 调用 / 计算 / 写入 | <code>*index = g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>WARN("Could not find gpu rank %d", rank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int g=0; g<system->nodes[GPU].count; g++) {
    if (system->nodes[GPU].nodes[g].gpu.rank == rank) {
      *index = g;
      return ncclSuccess;
    }
  }
  WARN("Could not find gpu rank %d", rank);
  return ncclInternalError;
}
```

</details>

<a id="f0472"></a>

## F0472：getNetIndex

用途：把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[NET].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 条件判断 | <code>(system-&gt;nodes[NET].nodes[n].id == id)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 3 / 调用 / 计算 / 写入 | <code>*index = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>WARN("Could not find net id %lx", id);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int n=0; n<system->nodes[NET].count; n++) {
    if (system->nodes[NET].nodes[n].id == id) {
      *index = n;
      return ncclSuccess;
    }
  }
  WARN("Could not find net id %lx", id);
  return ncclInternalError;
}
```

</details>

<a id="f0473"></a>

## F0473：getNetPaths

用途：按当前跨节点路径级别获取 NET 路径集合，供候选 GPU/NIC 组合筛选。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int netId = graph-&gt;inter[graph-&gt;nChannels*2];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getNetIndex(system, netId, &n));</code> | 失败时可能提前return，成功才继续；把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>*netPaths=system-&gt;nodes[NET].nodes[n].paths[GPU];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int netId = graph->inter[graph->nChannels*2];
  int n;
  NCCLCHECK(getNetIndex(system, netId, &n));
  *netPaths=system->nodes[NET].nodes[n].paths[GPU];
  return ncclSuccess;
}
```

</details>

<a id="f0474"></a>

## F0474：ncclTopoSearchNextGpuSort

用途：给尚未使用的下一 GPU 候选打分排序，优先搜索带宽高、路径代价合适的邻居。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const uint64_t flag = 1ULL&lt;&lt;(graph-&gt;nChannels);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclTopoLinkList* paths = gpu-&gt;paths[GPU];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclTopoLinkList* netPaths = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(sortNet)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getNetPaths(system, graph, &netPaths));</code> | 失败时可能提前return，成功才继续；按当前跨节点路径级别获取 NET 路径集合，供候选 GPU/NIC 组合筛选。 |
| 8 | 0 / 准备局部状态 | <code>struct ncclGpuScore scores[NCCL_TOPO_MAX_NODES];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>memset(scores, 0, ngpus*sizeof(struct ncclGpuScore));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 准备局部状态 | <code>int start = gpu-system-&gt;nodes[GPU].nodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>int count = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 循环 | <code>初始 int i=1;；继续条件 i&lt;ngpus；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 13 | 1 / 准备局部状态 | <code>int g = (start+i)%ngpus;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 1 / 条件判断 | <code>(paths[g].count == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 17 | 1 / 条件判断 | <code>(system-&gt;nodes[GPU].nodes[g].used & flag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 20 | 1 / 调用 / 计算 / 写入 | <code>scores[count].g = g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 1 / 调用 / 计算 / 写入 | <code>scores[count].startIndex = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 1 / 调用 / 计算 / 写入 | <code>scores[count].intraNhops = paths[g].count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>scores[count].intraWidth = paths[g].width;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 1 / 条件判断 | <code>(netPaths)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 3 / 调用 / 计算 / 写入 | <code>scores[count].interNhops = netPaths[g].count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>scores[count].interPciWidth = gpuPciWidth(system-&gt;nodes[GPU].nodes+g);</code> | 统计 GPU 的 PCI 通路宽度，为下一 GPU 候选排序加入 PCI 容量因素。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>scores[count].interWidth = netPaths[g].width;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>count++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>qsort(scores, count, sizeof(struct ncclGpuScore), cmpScore);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 条件判断 | <code>(sortNet == -1 && cmpIntraScores(scores, count) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>next[i] = scores[count-1-i].g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 37 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 38 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 3 / 调用 / 计算 / 写入 | <code>next[i] = scores[i].g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>*countPtr = count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const uint64_t flag = 1ULL<<(graph->nChannels);
  int ngpus = system->nodes[GPU].count;
  struct ncclTopoLinkList* paths = gpu->paths[GPU];
  struct ncclTopoLinkList* netPaths = NULL;
  if (sortNet) NCCLCHECK(getNetPaths(system, graph, &netPaths));

  struct ncclGpuScore scores[NCCL_TOPO_MAX_NODES];
  memset(scores, 0, ngpus*sizeof(struct ncclGpuScore));
  int start = gpu-system->nodes[GPU].nodes;
  int count = 0;
  for (int i=1; i<ngpus; i++) {
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
    for (int i=0; i<count; i++) next[i] = scores[count-1-i].g;
  } else {
    for (int i=0; i<count; i++) next[i] = scores[i].g;
  }
  *countPtr = count;
  return ncclSuccess;
}
```

</details>

<a id="f0475"></a>

## F0475：ncclTopoReplayGetGpu

用途：重放已有 channel 的 GPU 顺序，允许 sameChannels 搜索复用前面找到的排列。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*g = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(graph-&gt;nChannels == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int nextRank = graph-&gt;intra[(graph-&gt;nChannels-1)*ngpus+step+1];</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 7 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ngpus；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 条件判断 | <code>(system-&gt;nodes[GPU].nodes[i].gpu.rank == nextRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>*g = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 条件判断 | <code>(*g == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *g = -1;
  if (graph->nChannels == 0) return ncclInternalError;
  int ngpus = system->nodes[GPU].count;
  int nextRank = graph->intra[(graph->nChannels-1)*ngpus+step+1];
  for (int i=0; i<ngpus; i++) if (system->nodes[GPU].nodes[i].gpu.rank == nextRank) {
    *g = i;
    return ncclSuccess;
  }
  if (*g == -1) return ncclInternalError;
  return ncclSuccess;
}
```

</details>

<a id="f0476"></a>

## F0476：ncclTopoSearchTryGpu

用途：尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const uint64_t flag = 1ULL&lt;&lt;(graph-&gt;nChannels);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclTopoNode* gpu;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFollowPath(system, graph, type, index, GPU, g, 1, &gpu));</code> | 失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。 |
| 4 | 0 / 条件判断 | <code>(gpu)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>gpu-&gt;used ^= flag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, step, backToNet, backToFirstRank, forcedOrder, time));</code> | 失败时可能提前return，成功才继续；递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>gpu-&gt;used ^= flag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFollowPath(system, graph, type, index, GPU, g, -1, &gpu));</code> | 失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const uint64_t flag = 1ULL<<(graph->nChannels);
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
```

</details>

<a id="f0477"></a>

## F0477：ncclTopoCompareGraphs

用途：按通道总带宽、机内速度与跳数等指标比较候选图与已保存图，决定是否替换。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(graph-&gt;nChannels &lt; graph-&gt;minChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 条件判断 | <code>(graph-&gt;nChannels*graph-&gt;speedIntra &lt; refGraph-&gt;nChannels*refGraph-&gt;speedIntra)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 条件判断 | <code>(graph-&gt;nChannels*graph-&gt;speedIntra &gt; refGraph-&gt;nChannels*refGraph-&gt;speedIntra)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>*copy = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 条件判断 | <code>(graph-&gt;pattern == refGraph-&gt;pattern && graph-&gt;crossNic == refGraph-&gt;crossNic && graph-&gt;nHops &lt; refGraph-&gt;nHops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>*copy = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f0478"></a>

## F0478：ncclTopoSelectNets

用途：按路径级别、GPU 亲和性和网络设备分组挑出候选 NIC，使机间端点选择与机内路径匹配。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int netCount = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int localNetCount;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int* localNets;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&localNets, system-&gt;nodes[NET].count));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 循环 | <code>初始 int t=0;；继续条件 t &lt;= typeInter；每轮更新 t++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 2 / 条件判断 | <code>(gpu != -1 && gpu != g)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 4 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>localNetCount = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 准备局部状态 | <code>struct ncclTopoNode* gpu = system-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 2 / 准备局部状态 | <code>struct ncclTopoLinkList* paths = gpu-&gt;paths[NET];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 2 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[NET].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 3 / 条件判断 | <code>(paths[n].type == t)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 5 / 调用 / 计算 / 写入 | <code>localNets[localNetCount++] = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 2 / 条件判断 | <code>(localNetCount == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 21 | 2 / 循环 | <code>初始 int r=0;；继续条件 r&lt;system-&gt;nodes[GPU].nodes[g].gpu.dev % localNetCount；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 3 / 准备局部状态 | <code>int net0 = localNets[0];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 3 / 循环 | <code>初始 int i=0;；继续条件 i&lt;localNetCount-1；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>localNets[i] = localNets[i+1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 26 | 3 / 调用 / 计算 / 写入 | <code>localNets[localNetCount-1] = net0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 28 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;localNetCount；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 29 | 3 / 准备局部状态 | <code>int n = localNets[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 3 / 准备局部状态 | <code>int found = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 3 / 循环 | <code>初始 ；继续条件 (nets[found] != n && found&lt;netCount)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>found++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 34 | 3 / 条件判断 | <code>(found == netCount)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 5 / 调用 / 计算 / 写入 | <code>nets[netCount++] = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 39 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>*netCountRet = netCount;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>free(localNets);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int netCount = 0;
  int localNetCount;
  int* localNets;
  NCCLCHECK(ncclCalloc(&localNets, system->nodes[NET].count));

  for (int t=0; t <= typeInter; t++) {
    for (int g=0; g<system->nodes[GPU].count; g++) {
      if (gpu != -1 && gpu != g) continue;
      localNetCount = 0;
      struct ncclTopoNode* gpu = system->nodes[GPU].nodes+g;
      struct ncclTopoLinkList* paths = gpu->paths[NET];
      for (int n=0; n<system->nodes[NET].count; n++) {
        if (paths[n].type == t) localNets[localNetCount++] = n;
      }
      if (localNetCount == 0) continue;
      // Shuffle by gpu NVML device number so that GPUs on the same PCI switch
      // with multiple NICs don't use the same one as first choice.
      for (int r=0; r<system->nodes[GPU].nodes[g].gpu.dev % localNetCount; r++) {
        int net0 = localNets[0];
        for (int i=0; i<localNetCount-1; i++) localNets[i] = localNets[i+1];
        localNets[localNetCount-1] = net0;
      }
      // Append NICs to list
      for (int i=0; i<localNetCount; i++) {
        int n = localNets[i];
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
```

</details>

<a id="f0479"></a>

## F0479：ncclTopoSearchRecGpu

用途：递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>((*time) &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>(*time)--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(step == ngpus)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 准备局部状态 | <code>int copy = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;nChannels++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCompareGraphs(graph, saveGraph, &copy));</code> | 失败时可能提前return，成功才继续；按通道总带宽、机内速度与跳数等指标比较候选图与已保存图，决定是否替换。 |
| 11 | 2 / 条件判断 | <code>(copy)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>memcpy(saveGraph, graph, sizeof(struct ncclTopoGraph));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 4 / 条件判断 | <code>(graph-&gt;nChannels == graph-&gt;maxChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 6 / 调用 / 计算 / 写入 | <code>*time = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 条件判断 | <code>(graph-&gt;nChannels &lt; graph-&gt;maxChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchRec(system, graph, saveGraph, time));</code> | 失败时可能提前return，成功才继续；为一次候选搜索初始化已用节点状态与剩余预算，启动网络或机内递归搜索。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;nChannels--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;intra[graph-&gt;nChannels*ngpus+step] = gpu-&gt;gpu.rank;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 23 | 0 / 准备局部状态 | <code>int g = gpu - system-&gt;nodes[GPU].nodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 0 / 条件判断 | <code>(step == backToNet)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 2 / 条件判断 | <code>(system-&gt;nodes[NET].count)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 4 / 准备局部状态 | <code>int startNetIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getNetIndex(system, graph-&gt;inter[graph-&gt;nChannels*2], &startNetIndex));</code> | 失败时可能提前return，成功才继续；把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。 |
| 30 | 4 / 准备局部状态 | <code>struct ncclTopoNode* startNet = system-&gt;nodes[NET].nodes+startNetIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 4 / 准备局部状态 | <code>int netcount;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 4 / 准备局部状态 | <code>int* nets;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&nets, system-&gt;nodes[NET].count));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 34 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSelectNets(system, graph-&gt;typeInter, g, nets, &netcount));</code> | 失败时可能提前return，成功才继续；按路径级别、GPU 亲和性和网络设备分组挑出候选 NIC，使机间端点选择与机内路径匹配。 |
| 35 | 4 / 循环 | <code>初始 int i=0;；继续条件 i&lt;netcount；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 36 | 5 / 准备局部状态 | <code>int n = nets[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 5 / 准备局部状态 | <code>struct ncclTopoNode* net = system-&gt;nodes[NET].nodes+n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 5 / 条件判断 | <code>(graph-&gt;pattern == NCCL_TOPO_PATTERN_TREE && net-&gt;id != startNet-&gt;id)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 41 | 5 / 条件判断 | <code>(graph-&gt;crossNic != 1 && (net-&gt;net.asic != startNet-&gt;net.asic &#124;&#124; net-&gt;net.port != startNet-&gt;net.port))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 44 | 5 / 准备局部状态 | <code>int nextBackToNet = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 5 / 准备局部状态 | <code>float speedInterSave = graph-&gt;speedInter;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 46 | 5 / 条件判断 | <code>(graph-&gt;pattern == NCCL_TOPO_PATTERN_BALANCED_TREE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 7 / 条件判断 | <code>(step == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 49 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 50 | 9 / 调用 / 计算 / 写入 | <code>nextBackToNet = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 52 | 9 / 条件判断 | <code>(net-&gt;id != graph-&gt;inter[graph-&gt;nChannels*2+1])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 11 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 55 | 7 / 调用 / 计算 / 写入 | <code>graph-&gt;speedInter /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, NET, n, 1, &net));</code> | 失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。 |
| 57 | 5 / 调用 / 计算 / 写入 | <code>graph-&gt;speedInter = speedInterSave;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 5 / 条件判断 | <code>(net)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 7 / 调用 / 计算 / 写入 | <code>graph-&gt;inter[graph-&gt;nChannels*2+1] = net-&gt;id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, step, nextBackToNet, backToFirstRank, forcedOrder, time));</code> | 失败时可能提前return，成功才继续；递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。 |
| 62 | 7 / 条件判断 | <code>(graph-&gt;pattern == NCCL_TOPO_PATTERN_BALANCED_TREE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 9 / 调用 / 计算 / 写入 | <code>graph-&gt;speedInter /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 65 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, NET, n, -1, &net));</code> | 失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。 |
| 66 | 7 / 调用 / 计算 / 写入 | <code>graph-&gt;speedInter = speedInterSave;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 68 | 4 / 调用 / 计算 / 写入 | <code>free(nets);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 69 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 70 | 2 / 条件判断 | <code>(step &lt; system-&gt;nodes[GPU].count-1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 71 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 72 | 4 / 准备局部状态 | <code>int next[NCCL_TOPO_MAX_NODES];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 73 | 4 / 准备局部状态 | <code>int count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 74 | 4 / 条件判断 | <code>(forcedOrder == FORCED_ORDER_PCI)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 75 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 76 | 6 / 调用 / 计算 / 写入 | <code>next[0] = step+1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 77 | 6 / 调用 / 计算 / 写入 | <code>count = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 79 | 6 / 条件判断 | <code>(forcedOrder == FORCED_ORDER_REPLAY)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 80 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 81 | 8 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoReplayGetGpu(system, graph, step, next));</code> | 失败时可能提前return，成功才继续；重放已有 channel 的 GPU 顺序，允许 sameChannels 搜索复用前面找到的排列。 |
| 82 | 8 / 调用 / 计算 / 写入 | <code>count = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 84 | 8 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchNextGpuSort(system, graph, gpu, next, &count, backToNet == -1 ? 0 : backToNet == step+1 ? 1 : -1 ));</code> | 失败时可能提前return，成功才继续；给尚未使用的下一 GPU 候选打分排序，优先搜索带宽高、路径代价合适的邻居。 |
| 85 | 4 / 循环 | <code>初始 int i=0;；继续条件 i&lt;count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 86 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, step+1, backToNet, backToFirstRank, forcedOrder, time, GPU, g, next[i]));</code> | 失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。 |
| 87 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 88 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 89 | 4 / 条件判断 | <code>(step == backToFirstRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 90 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 91 | 6 / 准备局部状态 | <code>int p;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 92 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getGpuIndex(system, graph-&gt;intra[graph-&gt;nChannels*ngpus], &p));</code> | 失败时可能提前return，成功才继续；把硬件 GPU rank 或 NET id 转成系统节点数组下标，供搜索数组访问。 |
| 93 | 6 / 准备局部状态 | <code>struct ncclTopoNode* firstGpu;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 94 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, GPU, p, 1, &firstGpu));</code> | 失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。 |
| 95 | 6 / 条件判断 | <code>(firstGpu)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 96 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 97 | 8 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, firstGpu, step+1, backToNet, -1, forcedOrder, time));</code> | 失败时可能提前return，成功才继续；递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。 |
| 98 | 8 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFollowPath(system, graph, GPU, g, GPU, p, -1, &firstGpu));</code> | 失败时可能提前return，成功才继续；包装路径选择、容量扣减与失败回滚，使递归搜索不会污染其他候选的剩余带宽。 |
| 99 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 100 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchRecGpu(system, graph, saveGraph, gpu, ngpus, -1, -1, forcedOrder, time));</code> | 失败时可能提前return，成功才继续；递归安排当前通道的 GPU 顺序和返回网络/闭环步骤，完成一条后继续搜索更多通道。 |
| 101 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if ((*time) <= 0) return ncclSuccess;
  (*time)--;

  int ngpus = system->nodes[GPU].count;
  if (step == ngpus) {
    // Determine whether we found a better solution or not
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
  int g = gpu - system->nodes[GPU].nodes;
  if (step == backToNet) {
    // first get back to NIC
    if (system->nodes[NET].count) {
      int startNetIndex;
      NCCLCHECK(getNetIndex(system, graph->inter[graph->nChannels*2], &startNetIndex));
      struct ncclTopoNode* startNet = system->nodes[NET].nodes+startNetIndex;
      int netcount;
      int* nets;
      NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));
      NCCLCHECK(ncclTopoSelectNets(system, graph->typeInter, g, nets, &netcount));
      for (int i=0; i<netcount; i++) {
        int n = nets[i];
        struct ncclTopoNode* net = system->nodes[NET].nodes+n;
        if (graph->pattern == NCCL_TOPO_PATTERN_TREE && net->id != startNet->id) continue; // Trees are symmetric
        if (graph->crossNic != 1 && (net->net.asic != startNet->net.asic || net->net.port != startNet->net.port)) continue;

        // Balanced Tree : count half of the bandwidth on first two GPUs
        int nextBackToNet = -1;
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
    int next[NCCL_TOPO_MAX_NODES];
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
    for (int i=0; i<count; i++) {
      NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, step+1, backToNet, backToFirstRank, forcedOrder, time, GPU, g, next[i]));
    }
  } else if (step == backToFirstRank) {
    // Find first GPU and loop back to it
    int p;
    NCCLCHECK(getGpuIndex(system, graph->intra[graph->nChannels*ngpus], &p));
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
```

</details>

<a id="f0480"></a>

## F0480：ncclTopoSearchRecNet

用途：递归尝试通道的网络入口/出口和机内 GPU 起点，并根据 crossNic 限制组合。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int speed = graph-&gt;speedInter;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int* nets;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&nets, system-&gt;nodes[NET].count));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 准备局部状态 | <code>int netcount;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSelectNets(system, graph-&gt;typeInter, -1, nets, &netcount));</code> | 失败时可能提前return，成功才继续；按路径级别、GPU 亲和性和网络设备分组挑出候选 NIC，使机间端点选择与机内路径匹配。 |
| 6 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;netcount；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 准备局部状态 | <code>int n = nets[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 准备局部状态 | <code>struct ncclTopoNode* net = system-&gt;nodes[NET].nodes+n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 1 / 准备局部状态 | <code>struct ncclTopoNode* gpu;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 条件判断 | <code>(graph-&gt;collNet && net-&gt;net.collSupport == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 13 | 1 / 条件判断 | <code>(net-&gt;net.width &lt; speed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 16 | 1 / 条件判断 | <code>(net-&gt;net.maxChannels == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 19 | 1 / 调用 / 计算 / 写入 | <code>graph-&gt;inter[graph-&gt;nChannels*2] = net-&gt;id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 1 / 调用 / 计算 / 写入 | <code>graph-&gt;latencyInter = net-&gt;net.latency;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;system-&gt;nodes[NET].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 2 / 条件判断 | <code>((system-&gt;nodes[NET].nodes[i].net.asic == net-&gt;net.asic) && (system-&gt;nodes[NET].nodes[i].net.port == net-&gt;net.port))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>system-&gt;nodes[NET].nodes[i].net.width -= speed;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>net-&gt;net.maxChannels--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 1 / 条件判断 | <code>(graph-&gt;nChannels &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 3 / 准备局部状态 | <code>int g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));</code> | 失败时可能提前return，成功才继续；重放已有 channel 的 GPU 顺序，允许 sameChannels 搜索复用前面找到的排列。 |
| 31 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_REPLAY, time, NET, n, g));</code> | 失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。 |
| 32 | 1 / 条件判断 | <code>(graph-&gt;nChannels == 0 &#124;&#124; graph-&gt;sameChannels == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 3 / 条件判断 | <code>(graph-&gt;nChannels == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 5 / 准备局部状态 | <code>int t = 1 &lt;&lt; 10;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_PCI, &t, NET, n, 0));</code> | 失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。 |
| 38 | 5 / 条件判断 | <code>(t == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 7 / 调用 / 计算 / 写入 | <code>*time = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 3 / 准备局部状态 | <code>float maxWidth = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 3 / 准备局部状态 | <code>int minHops = 0xfffffff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 3 / 准备局部状态 | <code>struct ncclTopoLinkList* paths = net-&gt;paths[GPU];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 3 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 45 | 4 / 条件判断 | <code>(paths[g].width &gt; maxWidth)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 6 / 调用 / 计算 / 写入 | <code>maxWidth = paths[g].width;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 6 / 调用 / 计算 / 写入 | <code>minHops = paths[g].count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 50 | 6 / 条件判断 | <code>(paths[g].width == maxWidth && paths[g].count &lt; minHops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 8 / 调用 / 计算 / 写入 | <code>minHops = paths[g].count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 54 | 3 / 条件判断 | <code>(maxWidth &gt;= speed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 55 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 56 | 5 / 循环 | <code>初始 int tryGpuBidir=0;；继续条件 tryGpuBidir&lt;2；每轮更新 tryGpuBidir++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 57 | 6 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 58 | 7 / 条件判断 | <code>(paths[g].width == maxWidth && paths[g].count == minHops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 9 / 调用 / 计算 / 写入 | <code>gpu = system-&gt;nodes[GPU].nodes+g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 9 / 准备局部状态 | <code>int gpuUsed = gpuPciWidth(gpu) &gt; 0 ? 0 : 1;</code> | 统计 GPU 的 PCI 通路宽度，为下一 GPU 候选排序加入 PCI 容量因素。 |
| 62 | 9 / 条件判断 | <code>(tryGpuBidir == gpuUsed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 11 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, 0, time, NET, n, g));</code> | 失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。 |
| 65 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 66 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 67 | 1 / 调用 / 计算 / 写入 | <code>net-&gt;net.maxChannels++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 68 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;system-&gt;nodes[NET].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 69 | 2 / 条件判断 | <code>((system-&gt;nodes[NET].nodes[i].net.asic == net-&gt;net.asic) && (system-&gt;nodes[NET].nodes[i].net.port == net-&gt;net.port))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 70 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 71 | 4 / 调用 / 计算 / 写入 | <code>system-&gt;nodes[NET].nodes[i].net.width += speed;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 73 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 74 | 0 / 调用 / 计算 / 写入 | <code>free(nets);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 75 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const int speed = graph->speedInter;
  int* nets;
  NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));
  int netcount;
  NCCLCHECK(ncclTopoSelectNets(system, graph->typeInter, -1, nets, &netcount));
  for (int i=0; i<netcount; i++) {
    int n = nets[i];
    struct ncclTopoNode* net = system->nodes[NET].nodes+n;
    struct ncclTopoNode* gpu;
    if (graph->collNet && net->net.collSupport == 0) continue;
    if (net->net.width < speed) continue;
    if (net->net.maxChannels == 0) continue;

    graph->inter[graph->nChannels*2] = net->id;
    graph->latencyInter = net->net.latency;

    for (int i=0; i<system->nodes[NET].count; i++) {
      if ((system->nodes[NET].nodes[i].net.asic == net->net.asic) &&
          (system->nodes[NET].nodes[i].net.port == net->net.port)) {
        system->nodes[NET].nodes[i].net.width -= speed;
      }
    }
    net->net.maxChannels--;

    // First try to replay the last channel
    if (graph->nChannels > 0) {
      int g;
      NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));
      NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_REPLAY, time, NET, n, g));
    }
    if (graph->nChannels == 0 || graph->sameChannels == 0) {
      if (graph->nChannels == 0) {
        // Always try the PCI order first to set a reference, but don't count in the timeout nor let it run for long
        int t = 1 << 10;
        NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_PCI, &t, NET, n, 0));
        if (t == -1) *time = -1;
      }

      // Then try the most local GPUs
      float maxWidth = 0;
      int minHops = 0xfffffff;
      struct ncclTopoLinkList* paths = net->paths[GPU];
      for (int g=0; g<system->nodes[GPU].count; g++) {
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
        for (int tryGpuBidir=0; tryGpuBidir<2; tryGpuBidir++) {
          for (int g=0; g<system->nodes[GPU].count; g++) {
            if (paths[g].width == maxWidth && paths[g].count == minHops) {
              gpu = system->nodes[GPU].nodes+g;
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
    for (int i=0; i<system->nodes[NET].count; i++) {
      if ((system->nodes[NET].nodes[i].net.asic == net->net.asic) &&
          (system->nodes[NET].nodes[i].net.port == net->net.port)) {
        system->nodes[NET].nodes[i].net.width += speed;
      }
    }
  }
  free(nets);
  return ncclSuccess;
}
```

</details>

<a id="f0481"></a>

## F0481：ncclTopoSearchParams

用途：把 Ring/Tree/Split/Balanced 等模式转换成何时返回 NIC、何时闭合首 GPU 的搜索约束。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(system-&gt;nodes[NET].count)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 条件判断 | <code>(pattern == NCCL_TOPO_PATTERN_RING)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 4 / 调用 / 计算 / 写入 | <code>*backToNet = system-&gt;nodes[GPU].count-1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 7 | 4 / 条件判断 | <code>(pattern == NCCL_TOPO_PATTERN_SPLIT_TREE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 6 / 调用 / 计算 / 写入 | <code>*backToNet = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 6 / 调用 / 计算 / 写入 | <code>*backToNet = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>*backToFirstRank = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>*backToNet = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 条件判断 | <code>(pattern == NCCL_TOPO_PATTERN_RING)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>*backToFirstRank = system-&gt;nodes[GPU].count-1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 4 / 调用 / 计算 / 写入 | <code>*backToFirstRank = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f0482"></a>

## F0482：ncclTopoSearchRec

用途：为一次候选搜索初始化已用节点状态与剩余预算，启动网络或机内递归搜索。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int backToNet, backToFirstRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchParams(system, graph-&gt;pattern, &backToNet, &backToFirstRank));</code> | 失败时可能提前return，成功才继续；把 Ring/Tree/Split/Balanced 等模式转换成何时返回 NIC、何时闭合首 GPU 的搜索约束。 |
| 3 | 0 / 条件判断 | <code>(system-&gt;nodes[NET].count)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>ncclTopoSearchRecNet(system, graph, saveGraph, backToNet, backToFirstRank, time);</code> | 递归尝试通道的网络入口/出口和机内 GPU 起点，并根据 crossNic 限制组合。 |
| 6 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 7 | 2 / 条件判断 | <code>(graph-&gt;nChannels == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_PCI, time, -1, -1, 0));</code> | 失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。 |
| 10 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 4 / 准备局部状态 | <code>int g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));</code> | 失败时可能提前return，成功才继续；重放已有 channel 的 GPU 顺序，允许 sameChannels 搜索复用前面找到的排列。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_REPLAY, time, -1, -1, g));</code> | 失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。 |
| 14 | 2 / 条件判断 | <code>(graph-&gt;sameChannels == 0 &#124;&#124; graph-&gt;nChannels == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 4 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, 0, time, -1, -1, g));</code> | 失败时可能提前return，成功才继续；尝试占用到候选 GPU 的硬件路径，递归下一步后恢复占用以继续搜索其他分支。 |
| 18 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
      int g;
      NCCLCHECK(ncclTopoReplayGetGpu(system, graph, -1, &g));
      NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, FORCED_ORDER_REPLAY, time, -1, -1, g));
    }
    if (graph->sameChannels == 0 || graph->nChannels == 0) {
      // Finally, try all other possibilities unless we are forced to use the same channels
      for (int g=0; g<system->nodes[GPU].count; g++) {
        NCCLCHECK(ncclTopoSearchTryGpu(system, graph, saveGraph, 0, backToNet, backToFirstRank, 0, time, -1, -1, g));
      }
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0483"></a>

## F0483：ncclTopoGetChannelFromXml

用途：从一个 XML channel 读取 GPU 顺序和 NIC 端点，校验并写入搜索图数组。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int* inter = graph-&gt;inter+2*c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int* intra = graph-&gt;intra+ngpus*c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int n=0, g=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;xmlChannel-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 准备局部状态 | <code>struct ncclXmlNode* sub = xmlChannel-&gt;subs[s];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 准备局部状态 | <code>int dev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(sub, "dev", &dev));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 9 | 1 / 条件判断 | <code>(strcmp(sub-&gt;name, "net") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>inter[n++] = dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 3 / 条件判断 | <code>(strcmp(sub-&gt;name, "gpu") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 5 / 准备局部状态 | <code>int rank = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 5 / 循环 | <code>初始 int g=0;；继续条件 g&lt;ngpus；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 6 / 条件判断 | <code>(system-&gt;nodes[GPU].nodes[g].gpu.dev == dev)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 8 / 调用 / 计算 / 写入 | <code>rank = system-&gt;nodes[GPU].nodes[g].gpu.rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 5 / 条件判断 | <code>(rank == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 7 / 调用 / 计算 / 写入 | <code>WARN("XML Import Channel : dev %d not found.", dev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 7 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 25 | 5 / 调用 / 计算 / 写入 | <code>intra[g++] = rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int ngpus = system->nodes[GPU].count;
  int* inter = graph->inter+2*c;
  int* intra = graph->intra+ngpus*c;
  int n=0, g=0;
  for (int s=0; s<xmlChannel->nSubs; s++) {
    struct ncclXmlNode* sub = xmlChannel->subs[s];
    int dev;
    NCCLCHECK(xmlGetAttrInt(sub, "dev", &dev));
    if (strcmp(sub->name, "net") == 0) {
      inter[n++] = dev;
    } else if (strcmp(sub->name, "gpu") == 0) {
      int rank = -1;
      for (int g=0; g<ngpus; g++) {
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
```

</details>

<a id="f0484"></a>

## F0484：ncclTopoGetGraphFromXmlSub

用途：解析匹配 graph 的速度、模式与多条通道，恢复预先导出的搜索方案。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int id;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGraph, "id", &id));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 3 | 0 / 条件判断 | <code>(graph-&gt;id != id)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 准备局部状态 | <code>int crossNic;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGraph, "crossnic", &crossNic));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 8 | 0 / 条件判断 | <code>(graph-&gt;crossNic == 0 && crossNic == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;crossNic = crossNic;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGraph, "pattern", &graph-&gt;pattern));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGraph, "nchannels", &graph-&gt;nChannels));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrFloat(xmlGraph, "speedintra", &graph-&gt;speedIntra));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrFloat(xmlGraph, "speedinter", &graph-&gt;speedInter));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 16 | 0 / 条件判断 | <code>(xmlGetAttrFloat(xmlGraph, "latencyinter", &graph-&gt;latencyInter) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;latencyInter = 0.0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(xmlGraph, "typeintra", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToInt(str, &graph-&gt;typeIntra, kvDictLinkType));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(xmlGraph, "typeinter", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToInt(str, &graph-&gt;typeInter, kvDictLinkType));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGraph, "samechannels", &graph-&gt;sameChannels));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 25 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;xmlGraph-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetChannelFromXml(xmlGraph-&gt;subs[s], s, system, graph));</code> | 失败时可能提前return，成功才继续；从一个 XML channel 读取 GPU 顺序和 NIC 端点，校验并写入搜索图数组。 |
| 27 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>*nChannels = xmlGraph-&gt;nSubs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int id;
  NCCLCHECK(xmlGetAttrInt(xmlGraph, "id", &id));
  if (graph->id != id) return ncclSuccess;

  int crossNic;
  NCCLCHECK(xmlGetAttrInt(xmlGraph, "crossnic", &crossNic));
  if (graph->crossNic == 0 && crossNic == 1) return ncclSuccess;
  graph->crossNic = crossNic;

  NCCLCHECK(xmlGetAttrInt(xmlGraph, "pattern", &graph->pattern));
  NCCLCHECK(xmlGetAttrInt(xmlGraph, "nchannels", &graph->nChannels));
  NCCLCHECK(xmlGetAttrFloat(xmlGraph, "speedintra", &graph->speedIntra));
  NCCLCHECK(xmlGetAttrFloat(xmlGraph, "speedinter", &graph->speedInter));
  if (xmlGetAttrFloat(xmlGraph, "latencyinter", &graph->latencyInter) != ncclSuccess) graph->latencyInter = 0.0;
  const char* str;
  NCCLCHECK(xmlGetAttr(xmlGraph, "typeintra", &str));
  NCCLCHECK(kvConvertToInt(str, &graph->typeIntra, kvDictLinkType));
  NCCLCHECK(xmlGetAttr(xmlGraph, "typeinter", &str));
  NCCLCHECK(kvConvertToInt(str, &graph->typeInter, kvDictLinkType));
  NCCLCHECK(xmlGetAttrInt(xmlGraph, "samechannels", &graph->sameChannels));
  for (int s=0; s<xmlGraph->nSubs; s++) {
    NCCLCHECK(ncclTopoGetChannelFromXml(xmlGraph->subs[s], s, system, graph));
  }
  *nChannels = xmlGraph->nSubs;
  return ncclSuccess;
}
```

</details>

<a id="f0485"></a>

## F0485：ncclTopoGetGraphFromXml

用途：在 XML 中找到当前 graph id 的对象，再调用子解析器填充通信图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;xmlGraphs-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetGraphFromXmlSub(xmlGraphs-&gt;subs[s], system, graph, nChannels));</code> | 失败时可能提前return，成功才继续；解析匹配 graph 的速度、模式与多条通道，恢复预先导出的搜索方案。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int s=0; s<xmlGraphs->nSubs; s++) {
    NCCLCHECK(ncclTopoGetGraphFromXmlSub(xmlGraphs->subs[s], system, graph, nChannels));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0486"></a>

## F0486：ncclTopoGetXmlFromChannel

用途：把单条 channel 的 GPU/NIC 顺序写成 XML 节点，供人工检查或离线固定图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclXmlNode* xmlChannel;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int* inter = graph-&gt;inter+2*c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int* intra = graph-&gt;intra+ngpus*c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, parent, "channel", &xmlChannel));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclXmlNode* node;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 条件判断 | <code>(system-&gt;nodes[NET].count)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, xmlChannel, "net", &node));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(node, "dev", inter[0]));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 11 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;ngpus；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, xmlChannel, "gpu", &node));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 13 | 1 / 准备局部状态 | <code>int dev = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ngpus；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 15 | 2 / 条件判断 | <code>(system-&gt;nodes[GPU].nodes[i].gpu.rank == intra[g])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>dev = system-&gt;nodes[GPU].nodes[i].gpu.dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 1 / 条件判断 | <code>(dev == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>WARN("XML Export Channel : rank %d not found.", intra[g]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(node, "dev", dev));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 24 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 25 | 0 / 条件判断 | <code>(system-&gt;nodes[NET].count)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, xmlChannel, "net", &node));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(node, "dev", inter[1]));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 29 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclXmlNode* xmlChannel;
  int ngpus = system->nodes[GPU].count;
  int* inter = graph->inter+2*c;
  int* intra = graph->intra+ngpus*c;
  NCCLCHECK(xmlAddNode(xml, parent, "channel", &xmlChannel));
  struct ncclXmlNode* node;
  if (system->nodes[NET].count) {
    NCCLCHECK(xmlAddNode(xml, xmlChannel, "net", &node));
    NCCLCHECK(xmlSetAttrInt(node, "dev", inter[0]));
  }
  for (int g=0; g<ngpus; g++) {
    NCCLCHECK(xmlAddNode(xml, xmlChannel, "gpu", &node));
    int dev = -1;
    for (int i=0; i<ngpus; i++) {
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
```

</details>

<a id="f0487"></a>

## F0487：ncclTopoGetXmlFromGraph

用途：把通信图参数和各 channel 顺序写成 XML graph 对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclXmlNode* xmlGraph;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, parent, "graph", &xmlGraph));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(xmlGraph, "id", graph-&gt;id));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(xmlGraph, "pattern", graph-&gt;pattern));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(xmlGraph, "crossnic", graph-&gt;crossNic));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(xmlGraph, "nchannels", graph-&gt;nChannels));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrFloat(xmlGraph, "speedintra", graph-&gt;speedIntra));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrFloat(xmlGraph, "speedinter", graph-&gt;speedInter));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrFloat(xmlGraph, "latencyinter", graph-&gt;latencyInter));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 10 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToStr(graph-&gt;typeIntra, &str, kvDictLinkType));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(xmlGraph, "typeintra", str));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToStr(graph-&gt;typeInter, &str, kvDictLinkType));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(xmlGraph, "typeinter", str));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(xmlGraph, "samechannels", graph-&gt;sameChannels));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 16 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;graph-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromChannel(graph, c, system, xml, xmlGraph));</code> | 失败时可能提前return，成功才继续；把单条 channel 的 GPU/NIC 顺序写成 XML 节点，供人工检查或离线固定图。 |
| 18 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclXmlNode* xmlGraph;
  NCCLCHECK(xmlAddNode(xml, parent, "graph", &xmlGraph));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "id", graph->id));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "pattern", graph->pattern));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "crossnic", graph->crossNic));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "nchannels", graph->nChannels));
  NCCLCHECK(xmlSetAttrFloat(xmlGraph, "speedintra", graph->speedIntra));
  NCCLCHECK(xmlSetAttrFloat(xmlGraph, "speedinter", graph->speedInter));
  NCCLCHECK(xmlSetAttrFloat(xmlGraph, "latencyinter", graph->latencyInter));
  const char* str;
  NCCLCHECK(kvConvertToStr(graph->typeIntra, &str, kvDictLinkType));
  NCCLCHECK(xmlSetAttr(xmlGraph, "typeintra", str));
  NCCLCHECK(kvConvertToStr(graph->typeInter, &str, kvDictLinkType));
  NCCLCHECK(xmlSetAttr(xmlGraph, "typeinter", str));
  NCCLCHECK(xmlSetAttrInt(xmlGraph, "samechannels", graph->sameChannels));
  for (int c=0; c<graph->nChannels; c++) {
    NCCLCHECK(ncclTopoGetXmlFromChannel(graph, c, system, xml, xmlGraph));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0488"></a>

## F0488：ncclTopoGetXmlFromGraphs

用途：创建 XML 根并导出多种算法的通信图，保持模式/id 信息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>xml-&gt;maxIndex = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclXmlNode* xmlGraphs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, NULL, "graphs", &xmlGraphs));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(xmlGraphs, "version", NCCL_GRAPH_XML_VERSION));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 5 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;ngraphs；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromGraph(graphs[g], system, xml, xmlGraphs));</code> | 失败时可能提前return，成功才继续；把通信图参数和各 channel 顺序写成 XML graph 对象。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  xml->maxIndex = 0;
  struct ncclXmlNode* xmlGraphs;
  NCCLCHECK(xmlAddNode(xml, NULL, "graphs", &xmlGraphs));
  NCCLCHECK(xmlSetAttrInt(xmlGraphs, "version", NCCL_GRAPH_XML_VERSION));
  for (int g=0; g<ngraphs; g++) {
    NCCLCHECK(ncclTopoGetXmlFromGraph(graphs[g], system, xml, xmlGraphs));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0489"></a>

## F0489：ncclTopoCompute

用途：在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;crossNic = ncclParamCrossNic();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 准备局部状态 | <code>int crossNic = (system-&gt;nodes[NET].count &gt; 1) && graph-&gt;crossNic ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;speedIntra = graph-&gt;speedInter = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;latencyInter = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 条件判断 | <code>(graph-&gt;crossNic == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;crossNic = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;typeIntra = ngpus == 1 ? PATH_LOC : PATH_NVL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;typeInter = PATH_PIX;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;nChannels = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>graph-&gt;sameChannels = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 准备局部状态 | <code>char* str = getenv("NCCL_GRAPH_FILE");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_GRAPH_FILE set by environment to %s", str);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 2 / 准备局部状态 | <code>struct ncclXml* xml;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&xml, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlGraphFromFile(str, xml));</code> | 失败时可能提前return，成功才继续；打开并解析通信图 XML 文件，校验图版本以恢复预先保存的搜索结果。 |
| 20 | 2 / 准备局部状态 | <code>int nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetGraphFromXml(xml-&gt;nodes, system, graph, &nChannels));</code> | 失败时可能提前return，成功才继续；在 XML 中找到当前 graph id 的对象，再调用子解析器填充通信图。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Search %d : %d channels loaded from XML graph", graph-&gt;id, nChannels);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>free(xml);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 2 / 条件判断 | <code>(graph-&gt;nChannels &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 27 | 0 / 条件判断 | <code>(ngpus == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 2 / 条件判断 | <code>(graph-&gt;pattern != NCCL_TOPO_PATTERN_RING)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 4 / 调用 / 计算 / 写入 | <code>graph-&gt;pattern = NCCL_TOPO_PATTERN_TREE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 准备局部状态 | <code>int ccMin;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetCompCap(system, &ccMin, NULL));</code> | 失败时可能提前return，成功才继续；遍历 GPU 得到计算能力的最小/最大值，使协议与调优模型兼容参与设备。 |
| 34 | 0 / 准备局部状态 | <code>struct ncclTopoGraph tmpGraph;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&tmpGraph, graph, sizeof(struct ncclTopoGraph));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 0 / 准备局部状态 | <code>int nspeeds = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 0 / 准备局部状态 | <code>float* speedArray = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 0 / 条件判断 | <code>(system-&gt;nodes[NET].count == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 2 / 调用 / 计算 / 写入 | <code>nspeeds = NSPEEDSINTRA;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 2 / 调用 / 计算 / 写入 | <code>speedArray = speedArrayIntra;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 43 | 2 / 调用 / 计算 / 写入 | <code>nspeeds = NSPEEDSINTER;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>speedArray = speedArrayInter;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 0 / 准备局部状态 | <code>int pass = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 46 | 0 / 准备局部状态 | <code>int speedIndex = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 47 | 0 / 循环 | <code>初始 ；继续条件 (speedArray[speedIndex] &gt; system-&gt;maxWidth && speedIndex &lt; nspeeds-1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 48 | 1 / 调用 / 计算 / 写入 | <code>speedIndex++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 50 | 0 / 调用 / 计算 / 写入 | <code>tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 0 / 准备局部状态 | <code>int64_t globalTimeout = NCCL_SEARCH_GLOBAL_TIMEOUT;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 0 / 跳转标签 | <code>search</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 53 | 1 / 准备局部状态 | <code>int time = tmpGraph.sameChannels ? NCCL_SEARCH_TIMEOUT_SAMECHANNELS : tmpGraph.pattern == NCCL_TOPO_PATTERN_TREE ? NCCL_SEARCH_TIMEOUT_TREE : NCCL_SEARCH_TIMEOUT;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 0 / 调用 / 计算 / 写入 | <code>tmpGraph.nChannels = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 0 / 调用 / 计算 / 写入 | <code>globalTimeout -= time;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchRec(system, &tmpGraph, graph, &time));</code> | 失败时可能提前return，成功才继续；为一次候选搜索初始化已用节点状态与剩余预算，启动网络或机内递归搜索。 |
| 57 | 0 / 编译期分支/指令 | <code>#if 0</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 58 | 1 / 调用 / 计算 / 写入 | <code>printf("Pattern %d, crossNic %d, Speed %g/%g, type %d/%d, channels %d-%d sameChannels %d -&gt; nChannels %dx%g/%g %s\n", tmpGraph.pattern, tmpGraph.crossNic, tmpGraph.speedInter, tmpGraph.speedIntra, tmpGraph.typeInter, tmpGraph.typeIntra, tmpGraph.minChannels, tmpGraph.maxChannels, tmpGraph.sameChannels, graph-&gt;nChannels, graph-&gt;speedInter, graph-&gt;speedIntra, time == 0 ? "TIMEOUT" : time == -1 ? "PERFECT" : "");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 59 | 1 / 循环 | <code>初始 int c=0;；继续条件 c&lt;graph-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 60 | 2 / 调用 / 计算 / 写入 | <code>printf("%2d : ", c);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 61 | 2 / 循环 | <code>初始 int g=0;；继续条件 g&lt;ngpus；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 62 | 3 / 调用 / 计算 / 写入 | <code>printf("%d ", graph-&gt;intra[c*ngpus+g]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 63 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 64 | 2 / 调用 / 计算 / 写入 | <code>printf("[%d %d]", graph-&gt;inter[c*2+0], graph-&gt;inter[c*2+1]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 65 | 2 / 调用 / 计算 / 写入 | <code>printf("\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 66 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 67 | 0 / 条件判断 | <code>(time == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 2 / 控制转移 | <code>goto done;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 70 | 0 / 条件判断 | <code>(graph-&gt;nChannels*graph-&gt;speedInter &gt;= system-&gt;totalWidth)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 71 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 72 | 2 / 控制转移 | <code>goto done;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 73 | 0 / 条件判断 | <code>(pass == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 74 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 75 | 2 / 条件判断 | <code>(tmpGraph.sameChannels == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 76 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 77 | 4 / 调用 / 计算 / 写入 | <code>tmpGraph.sameChannels = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 4 / 控制转移 | <code>goto search;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 79 | 2 / 调用 / 计算 / 写入 | <code>tmpGraph.sameChannels = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 80 | 2 / 条件判断 | <code>(time != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 81 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 82 | 4 / 调用 / 计算 / 写入 | <code>globalTimeout += time;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 84 | 4 / 调用 / 计算 / 写入 | <code>globalTimeout = NCCL_SEARCH_GLOBAL_TIMEOUT;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 2 / 条件判断 | <code>(globalTimeout &lt; 0 && graph-&gt;nChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 86 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 87 | 4 / 控制转移 | <code>goto done;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 88 | 2 / 准备局部状态 | <code>int maxTypeIntra = system-&gt;nodes[NET].count &gt; 0 ? tmpGraph.typeInter : PATH_SYS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 89 | 2 / 条件判断 | <code>(tmpGraph.typeIntra &lt; maxTypeIntra && (graph-&gt;nChannels == 0 &#124;&#124; tmpGraph.typeIntra &lt; graph-&gt;typeIntra))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 90 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 91 | 4 / 调用 / 计算 / 写入 | <code>tmpGraph.typeIntra += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 92 | 4 / 控制转移 | <code>goto search;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 93 | 2 / 调用 / 计算 / 写入 | <code>tmpGraph.typeIntra = ngpus == 1 ? PATH_LOC : PATH_NVL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 94 | 2 / 条件判断 | <code>(system-&gt;nodes[NET].count &gt; 0 && tmpGraph.typeInter &lt; PATH_SYS && (graph-&gt;nChannels == 0 &#124;&#124; tmpGraph.typeInter &lt; graph-&gt;typeInter &#124;&#124; tmpGraph.typeInter &lt; PATH_PXN))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 95 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 96 | 4 / 调用 / 计算 / 写入 | <code>tmpGraph.typeInter += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 97 | 4 / 控制转移 | <code>goto search;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 98 | 2 / 调用 / 计算 / 写入 | <code>tmpGraph.typeInter = PATH_PIX;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 99 | 2 / 条件判断 | <code>(crossNic && tmpGraph.crossNic == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 100 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 101 | 4 / 调用 / 计算 / 写入 | <code>tmpGraph.crossNic = crossNic;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 102 | 4 / 控制转移 | <code>goto search;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 103 | 2 / 调用 / 计算 / 写入 | <code>tmpGraph.crossNic = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 104 | 2 / 条件判断 | <code>(tmpGraph.pattern == NCCL_TOPO_PATTERN_SPLIT_TREE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 105 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 106 | 4 / 调用 / 计算 / 写入 | <code>tmpGraph.pattern = NCCL_TOPO_PATTERN_TREE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 107 | 4 / 控制转移 | <code>goto search;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 108 | 2 / 调用 / 计算 / 写入 | <code>tmpGraph.pattern = graph-&gt;pattern;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 109 | 2 / 条件判断 | <code>((speedIndex &lt; nspeeds-1) && (graph-&gt;nChannels == 0 &#124;&#124; (speedArray[speedIndex+1]/graph-&gt;speedInter &gt; .49)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 110 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 111 | 4 / 调用 / 计算 / 写入 | <code>tmpGraph.speedInter = tmpGraph.speedIntra = speedArray[++speedIndex];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 112 | 4 / 控制转移 | <code>goto search;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 113 | 2 / 调用 / 计算 / 写入 | <code>speedIndex = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 114 | 2 / 循环 | <code>初始 ；继续条件 (speedArray[speedIndex] &gt; system-&gt;maxWidth && speedIndex &lt; nspeeds-1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 115 | 3 / 调用 / 计算 / 写入 | <code>speedIndex++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 116 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 117 | 2 / 调用 / 计算 / 写入 | <code>tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 118 | 0 / 跳转标签 | <code>done</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 119 | 1 / 条件判断 | <code>(pass == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 120 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 121 | 3 / 调用 / 计算 / 写入 | <code>time = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 122 | 3 / 调用 / 计算 / 写入 | <code>memcpy(&tmpGraph, graph, sizeof(tmpGraph));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 123 | 3 / 调用 / 计算 / 写入 | <code>speedIndex = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 124 | 3 / 循环 | <code>初始 ；继续条件 (speedArray[speedIndex] &gt; graph-&gt;speedInter && speedIndex &lt; nspeeds-1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 125 | 4 / 调用 / 计算 / 写入 | <code>speedIndex++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 126 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 127 | 3 / 调用 / 计算 / 写入 | <code>tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 128 | 3 / 调用 / 计算 / 写入 | <code>tmpGraph.minChannels = graph-&gt;nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 129 | 3 / 调用 / 计算 / 写入 | <code>pass = 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 130 | 0 / 条件判断 | <code>(pass == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 131 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 132 | 2 / 条件判断 | <code>(time != 0 && graph-&gt;pattern != NCCL_TOPO_PATTERN_RING && tmpGraph.speedIntra == graph-&gt;speedIntra && tmpGraph.speedIntra &lt; tmpGraph.speedInter*2 && speedIndex &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 133 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 134 | 4 / 调用 / 计算 / 写入 | <code>tmpGraph.speedIntra = speedArray[--speedIndex];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 135 | 4 / 控制转移 | <code>goto search;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 136 | 2 / 调用 / 计算 / 写入 | <code>time = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 137 | 2 / 调用 / 计算 / 写入 | <code>memcpy(&tmpGraph, graph, sizeof(tmpGraph));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 138 | 0 / 条件判断 | <code>(graph-&gt;nChannels == 0 && graph-&gt;collNet == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 139 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 140 | 2 / 调用 / 计算 / 写入 | <code>WARN("Could not find a path for pattern %d, falling back to simple order", graph-&gt;pattern);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 141 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ngpus；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 142 | 3 / 调用 / 计算 / 写入 | <code>graph-&gt;intra[i] = system-&gt;nodes[GPU].nodes[i].gpu.rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 143 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 144 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;inter[0] = graph-&gt;inter[1] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 145 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;speedIntra = graph-&gt;speedInter = 0.1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 146 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;typeIntra = graph-&gt;typeInter = PATH_SYS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 147 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;nChannels = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 148 | 0 / 条件判断 | <code>(graph-&gt;speedIntra &gt;= 25.0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 149 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 150 | 2 / 准备局部状态 | <code>int dupChannels = std::min(graph-&gt;nChannels*2, graph-&gt;maxChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 151 | 2 / 调用 / 计算 / 写入 | <code>memcpy(graph-&gt;intra+graph-&gt;nChannels*ngpus, graph-&gt;intra, (dupChannels-graph-&gt;nChannels)*ngpus*sizeof(int));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 152 | 2 / 调用 / 计算 / 写入 | <code>memcpy(graph-&gt;inter+graph-&gt;nChannels*2,graph-&gt;inter, (dupChannels-graph-&gt;nChannels)*2*sizeof(int));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 153 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;speedIntra /= DIVUP(dupChannels, graph-&gt;nChannels);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 154 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;speedInter /= DIVUP(dupChannels, graph-&gt;nChannels);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 155 | 2 / 调用 / 计算 / 写入 | <code>graph-&gt;nChannels = dupChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 156 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int ngpus = system->nodes[GPU].count;
  graph->crossNic = ncclParamCrossNic();
  int crossNic = (system->nodes[NET].count > 1) && graph->crossNic ? 1 : 0;
  graph->speedIntra = graph->speedInter = 0;
  graph->latencyInter = 0;
  if (graph->crossNic == 2) graph->crossNic = 0;
  graph->typeIntra = ngpus == 1 ? PATH_LOC : PATH_NVL;
  graph->typeInter = PATH_PIX;
  graph->nChannels = 0;
  graph->sameChannels = 1;

  char* str = getenv("NCCL_GRAPH_FILE");
  if (str) {
    INFO(NCCL_ENV, "NCCL_GRAPH_FILE set by environment to %s", str);
    struct ncclXml* xml;
    NCCLCHECK(ncclCalloc(&xml, 1));
    NCCLCHECK(ncclTopoGetXmlGraphFromFile(str, xml));
    int nChannels;
    NCCLCHECK(ncclTopoGetGraphFromXml(xml->nodes, system, graph, &nChannels));
    INFO(NCCL_GRAPH, "Search %d : %d channels loaded from XML graph", graph->id, nChannels);
    free(xml);
    if (graph->nChannels > 0) return ncclSuccess;
  }

  if (ngpus == 1) if (graph->pattern != NCCL_TOPO_PATTERN_RING) graph->pattern = NCCL_TOPO_PATTERN_TREE;

  // SPLIT_TREE works better on older archs.
  int ccMin;
  NCCLCHECK(ncclTopoGetCompCap(system, &ccMin, NULL));

  struct ncclTopoGraph tmpGraph;
  memcpy(&tmpGraph, graph, sizeof(struct ncclTopoGraph));

  // First try crossnic, then decrease speed and finally increase speedIntra.
  int nspeeds = 0;
  float* speedArray = NULL;
  if (system->nodes[NET].count == 0) {
    nspeeds = NSPEEDSINTRA;
    speedArray = speedArrayIntra;
  } else {
    nspeeds = NSPEEDSINTER;
    speedArray = speedArrayInter;
  }
  int pass = 1;
  int speedIndex = 0;
  while (speedArray[speedIndex] > system->maxWidth && speedIndex < nspeeds-1) speedIndex++;
  tmpGraph.speedIntra = tmpGraph.speedInter = speedArray[speedIndex];
  int64_t globalTimeout = NCCL_SEARCH_GLOBAL_TIMEOUT;

/* 中文精读：channel 数不是由数据大小直接生成的
 * 初始化时尚不知道未来用户张量大小；此处依据 GPU/NIC/链路容量搜索可用通信图。
 * 先尝试同样的通道路径，再放宽不同通道顺序、路径类型、crossNic 或速度，遇到预算限制退出。
 * 选出图后可能复制高速通道来增加并行 block，最终 Postset 还会继续调整。
 * 真正收到一条消息时，getAlgoInfo 才按 nBytes 缩减本次 channel/线程数。
 */
search:
  int time = tmpGraph.sameChannels ? NCCL_SEARCH_TIMEOUT_SAMECHANNELS :
    tmpGraph.pattern == NCCL_TOPO_PATTERN_TREE ? NCCL_SEARCH_TIMEOUT_TREE : NCCL_SEARCH_TIMEOUT;
  tmpGraph.nChannels = 0;
  globalTimeout -= time;

  NCCLCHECK(ncclTopoSearchRec(system, &tmpGraph, graph, &time));
#if 0
  printf("Pattern %d, crossNic %d, Speed %g/%g, type %d/%d, channels %d-%d sameChannels %d -> nChannels %dx%g/%g %s\n", tmpGraph.pattern, tmpGraph.crossNic, tmpGraph.speedInter, tmpGraph.speedIntra, tmpGraph.typeInter, tmpGraph.typeIntra, tmpGraph.minChannels, tmpGraph.maxChannels, tmpGraph.sameChannels, graph->nChannels, graph->speedInter, graph->speedIntra, time == 0 ? "TIMEOUT" : time == -1 ? "PERFECT" : "");
  for (int c=0; c<graph->nChannels; c++) {
    printf("%2d : ", c);
    for (int g=0; g<ngpus; g++) {
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
    for (int i=0; i<ngpus; i++) graph->intra[i] = system->nodes[GPU].nodes[i].gpu.rank;
    graph->inter[0] = graph->inter[1] = 0;
    graph->speedIntra = graph->speedInter = 0.1;
    graph->typeIntra = graph->typeInter = PATH_SYS;
    graph->nChannels = 1;
  }

  if (graph->speedIntra >= 25.0) {
    int dupChannels = std::min(graph->nChannels*2, graph->maxChannels);
    memcpy(graph->intra+graph->nChannels*ngpus, graph->intra, (dupChannels-graph->nChannels)*ngpus*sizeof(int));
    memcpy(graph->inter+graph->nChannels*2,graph->inter, (dupChannels-graph->nChannels)*2*sizeof(int));
    graph->speedIntra /= DIVUP(dupChannels, graph->nChannels);
    graph->speedInter /= DIVUP(dupChannels, graph->nChannels);
    graph->nChannels = dupChannels;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0490"></a>

## F0490：ncclTopoPrintGraph

用途：输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Pattern %d, crossNic %d, nChannels %d, speed %f/%f, type %s/%s, sameChannels %d", graph-&gt;pattern, graph-&gt;crossNic, graph-&gt;nChannels, graph-&gt;speedIntra, graph-&gt;speedInter, topoPathTypeStr[graph-&gt;typeIntra], topoPathTypeStr[graph-&gt;typeInter], graph-&gt;sameChannels);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>int ngpus = system-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>char line[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;graph-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>sprintf(line, "%2d :", c);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 1 / 准备局部状态 | <code>int offset = strlen(line);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 条件判断 | <code>(system-&gt;nodes[NET].count &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, " %s/%d", topoNodeTypeStr[NET], graph-&gt;inter[2*c]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>offset = strlen(line);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ngpus；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, " %s/%d", topoNodeTypeStr[GPU], graph-&gt;intra[ngpus*c+i]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>offset = strlen(line);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 1 / 条件判断 | <code>(system-&gt;nodes[NET].count &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, " %s/%d", topoNodeTypeStr[NET], graph-&gt;inter[2*c+1]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>offset = strlen(line);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 1 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  INFO(NCCL_GRAPH, "Pattern %d, crossNic %d, nChannels %d, speed %f/%f, type %s/%s, sameChannels %d", graph->pattern, graph->crossNic, graph->nChannels, graph->speedIntra, graph->speedInter, topoPathTypeStr[graph->typeIntra], topoPathTypeStr[graph->typeInter], graph->sameChannels);
  int ngpus = system->nodes[GPU].count;

  char line[1024];
  for (int c=0; c<graph->nChannels; c++) {
    sprintf(line, "%2d :", c);
    int offset = strlen(line);
    if (system->nodes[NET].count > 0) {
      sprintf(line+offset, " %s/%d", topoNodeTypeStr[NET], graph->inter[2*c]);
      offset = strlen(line);
    }
    for (int i=0; i<ngpus; i++) {
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
```

</details>

<a id="f0491"></a>

## F0491：ncclTopoDumpGraphs

用途：输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char* str = getenv("NCCL_GRAPH_DUMP_FILE");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_GRAPH_DUMP_FILE set by environment to %s", str);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 准备局部状态 | <code>struct ncclXml* xml;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&xml, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromGraphs(ngraphs, graphs, system, xml));</code> | 失败时可能提前return，成功才继续；创建 XML 根并导出多种算法的通信图，保持模式/id 信息。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoDumpXmlToFile(str, xml));</code> | 失败时可能提前return，成功才继续；递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>free(xml);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char* str = getenv("NCCL_GRAPH_DUMP_FILE");
  if (str) {
    INFO(NCCL_ENV, "NCCL_GRAPH_DUMP_FILE set by environment to %s", str);
    struct ncclXml* xml;
    NCCLCHECK(ncclCalloc(&xml, 1));
    NCCLCHECK(ncclTopoGetXmlFromGraphs(ngraphs, graphs, system, xml));
    NCCLCHECK(ncclTopoDumpXmlToFile(str, xml));
    free(xml);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0492"></a>

## F0492：ncclTopoGetNetDev

用途：按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(graph)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>int channel = channelId%graph-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 准备局部状态 | <code>int ngpus = comm-&gt;topo-&gt;nodes[GPU].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 准备局部状态 | <code>int index = graph-&gt;intra[channel*ngpus] == rank ? 0 : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>*dev = graph-&gt;inter[channel*2+index];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetIntermediateRank(comm-&gt;topo, rank, *dev, proxyRank));</code> | 失败时可能提前return，成功才继续；找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。 |
| 8 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 9 | 2 / 条件判断 | <code>(peerRank == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetLocalNet(comm-&gt;topo, rank, dev));</code> | 失败时可能提前return，成功才继续；按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>*proxyRank = rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 4 / 准备局部状态 | <code>int pxnLevel = ncclPxnDisable() == 1 ? 0 : ncclParamP2pPxnLevel();</code> | 汇总参数和环境条件判断是否禁止 PXN，避免选择当前后端不支持的中间 GPU 网络路径。 |
| 16 | 4 / 条件判断 | <code>(ncclParamCrossNic() == 0 &#124;&#124; (pxnLevel != 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 6 / 准备局部状态 | <code>int netDev = comm-&gt;peerInfo[peerRank].netDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 6 / 准备局部状态 | <code>int n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 6 / 条件判断 | <code>(ncclParamCrossNic() == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 8 / 条件判断 | <code>(ncclTopoIdToIndex(comm-&gt;topo, NET, netDev, &n) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 10 / 调用 / 计算 / 写入 | <code>WARN("Rank %d requires NIC %d but that NIC is not available for rank %d", peerRank, netDev, rank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 10 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 8 / 调用 / 计算 / 写入 | <code>*dev = netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 6 / 条件判断 | <code>(pxnLevel == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 8 / 准备局部状态 | <code>int g, n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 8 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRankToIndex(comm-&gt;topo, rank, &g));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 31 | 8 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoIdToIndex(comm-&gt;topo, NET, netDev, &n));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 32 | 8 / 准备局部状态 | <code>struct ncclTopoNode* gpu = comm-&gt;topo-&gt;nodes[GPU].nodes+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 8 / 条件判断 | <code>(gpu-&gt;paths[NET][n].type &lt;= PATH_PXN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 10 / 调用 / 计算 / 写入 | <code>*dev = netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 10 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetIntermediateRank(comm-&gt;topo, rank, *dev, proxyRank));</code> | 失败时可能提前return，成功才继续；找出 GPU 到 NET 路径中的中间 GPU rank，决定是否采用 PXN 代理出口。 |
| 37 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 38 | 8 / 条件判断 | <code>(pxnLevel == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 10 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;localRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 41 | 11 / 准备局部状态 | <code>int peerRank = comm-&gt;localRankToRank[r];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 11 / 条件判断 | <code>(comm-&gt;peerInfo[peerRank].netDev == netDev)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 12 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 13 / 准备局部状态 | <code>int g1, g2, n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 13 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRankToIndex(comm-&gt;topo, rank, &g1));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 46 | 13 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRankToIndex(comm-&gt;topo, peerRank, &g2));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 47 | 13 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoIdToIndex(comm-&gt;topo, NET, netDev, &n));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 48 | 13 / 准备局部状态 | <code>struct ncclTopoNode* peerGpu = comm-&gt;topo-&gt;nodes[GPU].nodes+g2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 13 / 条件判断 | <code>(peerGpu-&gt;paths[GPU][g1].type &lt;= PATH_NVL && peerGpu-&gt;paths[NET][n].type &lt;= PATH_PXB)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 14 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 15 / 调用 / 计算 / 写入 | <code>*proxyRank = peerRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 15 / 调用 / 计算 / 写入 | <code>*dev = netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 15 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 54 | 10 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 55 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (graph) {
    // Honor the net device in the graph
    int channel = channelId%graph->nChannels;
    int ngpus = comm->topo->nodes[GPU].count;
    int index = graph->intra[channel*ngpus] == rank ? 0 : 1;
    *dev = graph->inter[channel*2+index];
    NCCLCHECK(ncclTopoGetIntermediateRank(comm->topo, rank, *dev, proxyRank));
  } else if (peerRank == -1) {
    return ncclInternalError;
  } else {
    // Start with our local NIC and local Rank
    NCCLCHECK(ncclTopoGetLocalNet(comm->topo, rank, dev));
    *proxyRank = rank;

    int pxnLevel = ncclPxnDisable() == 1 ? 0 : ncclParamP2pPxnLevel();
    // See whether we can use the remote rank preferred device.
    if (ncclParamCrossNic() == 0 || (pxnLevel != 0)) {
      int netDev = comm->peerInfo[peerRank].netDev;
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
        int g, n;
        NCCLCHECK(ncclTopoRankToIndex(comm->topo, rank, &g));
        NCCLCHECK(ncclTopoIdToIndex(comm->topo, NET, netDev, &n));
        struct ncclTopoNode* gpu = comm->topo->nodes[GPU].nodes+g;
        if (gpu->paths[NET][n].type <= PATH_PXN) {
          *dev = netDev;
          NCCLCHECK(ncclTopoGetIntermediateRank(comm->topo, rank, *dev, proxyRank));
        }
      } else if (pxnLevel == 2) {
        // Check whether we can access it through our node-local GPU for that NIC.
        for (int r=0; r<comm->localRanks; r++) {
          int peerRank = comm->localRankToRank[r];
          if (comm->peerInfo[peerRank].netDev == netDev) {
            int g1, g2, n;
            NCCLCHECK(ncclTopoRankToIndex(comm->topo, rank, &g1));
            NCCLCHECK(ncclTopoRankToIndex(comm->topo, peerRank, &g2));
            NCCLCHECK(ncclTopoIdToIndex(comm->topo, NET, netDev, &n));
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
```

</details>

