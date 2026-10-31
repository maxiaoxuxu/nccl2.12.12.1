# src/graph/topo.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0493"></a>

## F0493：pciPathToInt64

用途：把 PCI 路径中的 BDF 片段解析成便于匹配和排序的整数标识。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char* str = path+offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(*str == '/')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>str--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 循环 | <code>初始 ；继续条件 (*str != '/')；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>str--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>str++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 准备局部状态 | <code>int64_t numid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(busIdToInt64(str, &numid));</code> | 失败时可能提前return，成功才继续；在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>numid -= numid & 0xf;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>*id = numid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char* str = path+offset;
  // Remove trailing "/"
  if (*str == '/') str--;
  // Find next /
  while (*str != '/') str--;
  str++;
  int64_t numid;
  NCCLCHECK(busIdToInt64(str, &numid));
  // Ignore subdevice because those should use the same PCI link so we want to merge nodes.
  numid -= numid & 0xf;
  *id = numid;
  return ncclSuccess;
}
```

</details>

<a id="f0494"></a>

## F0494：findLocalCpu

用途：沿硬件连接查找设备所在 CPU 节点，用于补建 CPU/PCI 关系。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*cpu = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(node-&gt;type == CPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>*cpu = node;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环 | <code>初始 int l=0;；继续条件 l&lt;node-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 条件判断 | <code>(node-&gt;links[l].type == LINK_PCI)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(findLocalCpu(node-&gt;links[l].remNode, cpu));</code> | 失败时可能提前return，成功才继续；沿硬件连接查找设备所在 CPU 节点，用于补建 CPU/PCI 关系。 |
| 10 | 1 / 条件判断 | <code>(*cpu != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *cpu = NULL;
  if (node->type == CPU) {
    *cpu = node;
    return ncclSuccess;
  }
  for (int l=0; l<node->nlinks; l++) {
    if (node->links[l].type == LINK_PCI) NCCLCHECK(findLocalCpu(node->links[l].remNode, cpu));
    if (*cpu != NULL) return ncclSuccess;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0495"></a>

## F0495：ncclTopoGetInterCpuWidth

用途：依据 CPU 架构/型号估计 CPU 之间的互联带宽，补全硬件图中 SYS 边容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*width = LOC_WIDTH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(cpu-&gt;cpu.arch == NCCL_TOPO_CPU_ARCH_POWER)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>*width = P9_WIDTH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 条件判断 | <code>(cpu-&gt;cpu.arch == NCCL_TOPO_CPU_ARCH_ARM)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>*width = ARM_WIDTH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 条件判断 | <code>(cpu-&gt;cpu.arch == NCCL_TOPO_CPU_ARCH_X86 && cpu-&gt;cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>*width = cpu-&gt;cpu.model == NCCL_TOPO_CPU_TYPE_SKL ? SKL_QPI_WIDTH : QPI_WIDTH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 条件判断 | <code>(cpu-&gt;cpu.arch == NCCL_TOPO_CPU_ARCH_X86 && cpu-&gt;cpu.vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>*width = cpu-&gt;cpu.model == NCCL_TOPO_CPU_TYPE_YONGFENG ? YONGFENG_ZPI_WIDTH : ZPI_WIDTH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f0496"></a>

## F0496：ncclTopoGetNode

用途：在系统指定类型节点表中按 id 查找对象，返回已有节点地址。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;system-&gt;nodes[type].count；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 条件判断 | <code>(system-&gt;nodes[type].nodes[i].id == id)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 3 / 调用 / 计算 / 写入 | <code>*node = system-&gt;nodes[type].nodes+i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int i=0; i<system->nodes[type].count; i++) {
    if (system->nodes[type].nodes[i].id == id) {
      *node = system->nodes[type].nodes+i;
      return ncclSuccess;
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0497"></a>

## F0497：ncclTopoCreateNode

用途：在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(system-&gt;nodes[type].count == NCCL_TOPO_MAX_NODES)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("Error : tried to create too many nodes of type %d", type);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclTopoNode* n = system-&gt;nodes[type].nodes+system-&gt;nodes[type].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>system-&gt;nodes[type].count++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>n-&gt;type = type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>n-&gt;id = id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 条件判断 | <code>(type == GPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>n-&gt;nlinks=1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>n-&gt;links[0].type = LINK_LOC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>n-&gt;links[0].remNode = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>n-&gt;links[0].width = LOC_WIDTH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>n-&gt;gpu.dev = NCCL_TOPO_UNDEF;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>n-&gt;gpu.rank = NCCL_TOPO_UNDEF;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>n-&gt;gpu.cudaCompCap = NCCL_TOPO_UNDEF;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 2 / 条件判断 | <code>(type == CPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>n-&gt;cpu.arch = NCCL_TOPO_UNDEF;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 4 / 调用 / 计算 / 写入 | <code>n-&gt;cpu.vendor = NCCL_TOPO_UNDEF;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>n-&gt;cpu.model = NCCL_TOPO_UNDEF;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 25 | 4 / 条件判断 | <code>(type == NET)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 6 / 调用 / 计算 / 写入 | <code>n-&gt;net.asic = 0ULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 6 / 调用 / 计算 / 写入 | <code>n-&gt;net.port = NCCL_TOPO_UNDEF;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 6 / 调用 / 计算 / 写入 | <code>n-&gt;net.width = 0.0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 6 / 调用 / 计算 / 写入 | <code>n-&gt;net.latency = 0.0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>*node = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (system->nodes[type].count == NCCL_TOPO_MAX_NODES) {
    WARN("Error : tried to create too many nodes of type %d", type);
    return ncclInternalError;
  }
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
```

</details>

<a id="f0498"></a>

## F0498：ncclTopoRemoveNode

用途：删除硬件节点并修正指向它的连接及数组位置，避免后续搜索引用被裁掉对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclTopoNode* delNode = system-&gt;nodes[type].nodes+index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int t=0;；继续条件 t&lt;NCCL_TOPO_NODE_TYPES；每轮更新 t++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>free(delNode-&gt;paths[t]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 1 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[t].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 2 / 准备局部状态 | <code>struct ncclTopoNode* node = system-&gt;nodes[t].nodes+n;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 条件判断 | <code>(node == delNode)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 9 | 2 / 循环 | <code>初始 int l=0;；继续条件 l&lt;node-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 3 / 循环 | <code>初始 ；继续条件 (l&lt;node-&gt;nlinks && node-&gt;links[l].remNode == delNode)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 4 / 调用 / 计算 / 写入 | <code>memmove(node-&gt;links+l, node-&gt;links+l+1, (node-&gt;nlinks-l-1)*sizeof(struct ncclTopoLink));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>node-&gt;nlinks--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 3 / 条件判断 | <code>(l&lt;node-&gt;nlinks && node-&gt;links[l].remNode-&gt;type == type && node-&gt;links[l].remNode &gt;= delNode)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 5 / 调用 / 计算 / 写入 | <code>node-&gt;links[l].remNode--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>memmove(delNode, delNode+1, (system-&gt;nodes[type].count-index-1)*sizeof(struct ncclTopoNode));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>system-&gt;nodes[type].count--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclTopoNode* delNode = system->nodes[type].nodes+index;
  for (int t=0; t<NCCL_TOPO_NODE_TYPES; t++) {
    free(delNode->paths[t]);
    for (int n=0; n<system->nodes[t].count; n++) {
      struct ncclTopoNode* node = system->nodes[t].nodes+n;
      if (node == delNode) continue;
      for (int l=0; l<node->nlinks; l++) {
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
```

</details>

<a id="f0499"></a>

## F0499：ncclTopoConnectNodes

用途：给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclTopoLink* link;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 link = node-&gt;links；继续条件 link-&gt;remNode；每轮更新 link++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(link-&gt;remNode == remNode && link-&gt;type == type)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 条件判断 | <code>(link-&gt;remNode == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>node-&gt;nlinks++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>link-&gt;type = type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>link-&gt;remNode = remNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>link-&gt;width += width;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 准备局部状态 | <code>struct ncclTopoLink linkSave;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&linkSave, link, sizeof(struct ncclTopoLink));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 0 / 循环 | <code>初始 ；继续条件 (link != node-&gt;links)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 1 / 条件判断 | <code>((link-1)-&gt;width &gt;= linkSave.width)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 19 | 1 / 调用 / 计算 / 写入 | <code>memcpy(link, link-1, sizeof(struct ncclTopoLink));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 1 / 调用 / 计算 / 写入 | <code>link--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>memcpy(link, &linkSave, sizeof(struct ncclTopoLink));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Aggregate links into higher width for NVLink
  struct ncclTopoLink* link;
  for (link = node->links; link->remNode; link++) {
    if (link->remNode == remNode && link->type == type) break;
  }
  if (link->remNode == NULL) node->nlinks++;
  link->type = type;
  link->remNode = remNode;
  link->width += width;

  // Sort links in BW descending order
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
```

</details>

<a id="f0500"></a>

## F0500：ncclTopoFlattenBcmSwitches

用途：识别可合并表示的 Broadcom PCI 交换结构，简化建图而不额外发起硬件传输。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;system-&gt;nodes[PCI].count；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclTopoNode* pciSwitch = system-&gt;nodes[PCI].nodes+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 准备局部状态 | <code>uint64_t device = pciSwitch-&gt;pci.device;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 条件判断 | <code>((device & 0xfffffffffffff000) == 0x1000c0101000a000)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 准备局部状态 | <code>int64_t* subSwIds;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&subSwIds, pciSwitch-&gt;nlinks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 3 / 准备局部状态 | <code>int subs = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 3 / 循环 | <code>初始 int l=0;；继续条件 l&lt;pciSwitch-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 4 / 准备局部状态 | <code>struct ncclTopoNode* sub = pciSwitch-&gt;links[l].remNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 4 / 条件判断 | <code>(sub-&gt;type != PCI &#124;&#124; sub-&gt;pci.device != device)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 6 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>subSwIds[subs++] = sub-&gt;id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>memmove(pciSwitch-&gt;links+l, pciSwitch-&gt;links+l+1, (pciSwitch-&gt;nlinks-l-1)*(sizeof(struct ncclTopoLink)));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>pciSwitch-&gt;nlinks--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>l--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 3 / 循环 | <code>初始 int s=0;；继续条件 s&lt;subs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 20 | 4 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoIdToIndex(system, PCI, subSwIds[s], &index));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 22 | 4 / 准备局部状态 | <code>struct ncclTopoNode* sub = system-&gt;nodes[PCI].nodes+index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 4 / 循环 | <code>初始 int l=0;；继续条件 l&lt;sub-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 5 / 准备局部状态 | <code>struct ncclTopoNode* remNode = sub-&gt;links[l].remNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 5 / 条件判断 | <code>(remNode == pciSwitch)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 28 | 5 / 调用 / 计算 / 写入 | <code>memcpy(pciSwitch-&gt;links+pciSwitch-&gt;nlinks, sub-&gt;links+l, sizeof(struct ncclTopoLink));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 5 / 调用 / 计算 / 写入 | <code>pciSwitch-&gt;nlinks++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 5 / 循环 | <code>初始 int rl=0;；继续条件 rl&lt;remNode-&gt;nlinks；每轮更新 rl++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 31 | 6 / 条件判断 | <code>(remNode-&gt;links[rl].remNode == sub)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 8 / 调用 / 计算 / 写入 | <code>remNode-&gt;links[rl].remNode = pciSwitch;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 8 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 35 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 36 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 37 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRemoveNode(system, PCI, index));</code> | 失败时可能提前return，成功才继续；删除硬件节点并修正指向它的连接及数组位置，避免后续搜索引用被裁掉对象。 |
| 38 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 39 | 3 / 调用 / 计算 / 写入 | <code>pciSwitch-&gt;pci.device = 0x1000c01010000000;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 3 / 调用 / 计算 / 写入 | <code>free(subSwIds);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 3 / 调用 / 计算 / 写入 | <code>s = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 43 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int s=0; s<system->nodes[PCI].count; s++) {
    struct ncclTopoNode* pciSwitch = system->nodes[PCI].nodes+s;
    uint64_t device = pciSwitch->pci.device;
    // Only flatten PEX Gen 4 switches in base mode
    if ((device & 0xfffffffffffff000) == 0x1000c0101000a000) {
      // Find sub switches with the same device ID.
      int64_t* subSwIds;
      NCCLCHECK(ncclCalloc(&subSwIds, pciSwitch->nlinks));
      int subs = 0;
      for (int l=0; l<pciSwitch->nlinks; l++) {
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

      for (int s=0; s<subs; s++) {
        // Find sub switch (system->nodes[PCI].nodes is changing every time we remove a node)
        int index;
        NCCLCHECK(ncclTopoIdToIndex(system, PCI, subSwIds[s], &index));
        struct ncclTopoNode* sub = system->nodes[PCI].nodes+index;
        // Connect all sub PCI devices to the parent switch
        for (int l=0; l<sub->nlinks; l++) {
          struct ncclTopoNode* remNode = sub->links[l].remNode;
          if (remNode == pciSwitch) continue;
          // Add link from parent PCI switch -> PCI device
          memcpy(pciSwitch->links+pciSwitch->nlinks, sub->links+l, sizeof(struct ncclTopoLink));
          pciSwitch->nlinks++;
          // Update link from PCI device -> parent PCI switch
          for (int rl=0; rl<remNode->nlinks; rl++) {
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
```

</details>

<a id="f0501"></a>

## F0501：ncclTopoConnectCpus

用途：根据 CPU 类型补建 CPU 之间的互连边，使跨 NUMA 路径可被统一搜索。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[CPU].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 循环 | <code>初始 int p=0;；继续条件 p&lt;system-&gt;nodes[CPU].count；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 2 / 条件判断 | <code>(n == p)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 4 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 6 | 2 / 准备局部状态 | <code>float width;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetInterCpuWidth(system-&gt;nodes[CPU].nodes+n, &width));</code> | 失败时可能提前return，成功才继续；依据 CPU 架构/型号估计 CPU 之间的互联带宽，补全硬件图中 SYS 边容量。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(system-&gt;nodes[CPU].nodes+n, system-&gt;nodes[CPU].nodes+p, LINK_SYS, width));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 9 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // And connect all CPU nodes together
  for (int n=0; n<system->nodes[CPU].count; n++) {
    for (int p=0; p<system->nodes[CPU].count; p++) {
      if (n == p) continue;
      float width;
      NCCLCHECK(ncclTopoGetInterCpuWidth(system->nodes[CPU].nodes+n, &width));
      NCCLCHECK(ncclTopoConnectNodes(system->nodes[CPU].nodes+n, system->nodes[CPU].nodes+p, LINK_SYS, width));
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0502"></a>

## F0502：ncclTopoPrintRec

用途：递归打印硬件节点、类型和直接链路，验证发现到的硬件树。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(node-&gt;type == GPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, "%s/%lX (%d)", topoNodeTypeStr[node-&gt;type], node-&gt;id, node-&gt;gpu.rank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 条件判断 | <code>(node-&gt;type == CPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 4 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, "%s/%lX (%d/%d/%d)", topoNodeTypeStr[node-&gt;type], node-&gt;id, node-&gt;cpu.arch, node-&gt;cpu.vendor, node-&gt;cpu.model);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 9 | 4 / 条件判断 | <code>(node-&gt;type == PCI)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 6 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, "%s/%lX (%lx)", topoNodeTypeStr[node-&gt;type], node-&gt;id, node-&gt;pci.device);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 6 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, "%s/%lX", topoNodeTypeStr[node-&gt;type], node-&gt;id);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;offset；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 1 / 调用 / 计算 / 写入 | <code>line[i] = ' ';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 循环 | <code>初始 int l=0;；继续条件 l&lt;node-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 1 / 准备局部状态 | <code>struct ncclTopoLink* link = node-&gt;links+l;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 1 / 条件判断 | <code>(link-&gt;type == LINK_LOC)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 23 | 1 / 条件判断 | <code>(link-&gt;type != LINK_PCI &#124;&#124; link-&gt;remNode != prevNode)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 3 / 调用 / 计算 / 写入 | <code>sprintf(line+offset, "+ %s[%2.1f] - ", topoLinkTypeStr[link-&gt;type], link-&gt;width);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 3 / 准备局部状态 | <code>int nextOffset = strlen(line);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 3 / 条件判断 | <code>(link-&gt;type == LINK_PCI)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPrintRec(link-&gt;remNode, node, line, nextOffset));</code> | 失败时可能提前return，成功才继续；递归打印硬件节点、类型和直接链路，验证发现到的硬件树。 |
| 30 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 31 | 5 / 条件判断 | <code>(link-&gt;remNode-&gt;type == NET)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 7 / 调用 / 计算 / 写入 | <code>sprintf(line+nextOffset, "%s/%lX (%lx/%d/%f)", topoNodeTypeStr[link-&gt;remNode-&gt;type], link-&gt;remNode-&gt;id, link-&gt;remNode-&gt;net.asic, link-&gt;remNode-&gt;net.port, link-&gt;remNode-&gt;net.width);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 34 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 35 | 7 / 调用 / 计算 / 写入 | <code>sprintf(line+nextOffset, "%s/%lX", topoNodeTypeStr[link-&gt;remNode-&gt;type], link-&gt;remNode-&gt;id);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 5 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 37 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
  for (int i=0; i<offset; i++) line[i] = ' ';

  for (int l=0; l<node->nlinks; l++) {
    struct ncclTopoLink* link = node->links+l;
    if (link->type == LINK_LOC) continue;
    if (link->type != LINK_PCI || link->remNode != prevNode) {
      sprintf(line+offset, "+ %s[%2.1f] - ", topoLinkTypeStr[link->type], link->width);
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
```

</details>

<a id="f0503"></a>

## F0503：ncclTopoPrint

用途：递归打印硬件节点、类型和直接链路，验证发现到的硬件树。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "=== System : maxWidth %2.1f totalWidth %2.1f ===", s-&gt;maxWidth, s-&gt;totalWidth);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 准备局部状态 | <code>char line[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;s-&gt;nodes[CPU].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPrintRec(s-&gt;nodes[CPU].nodes+n, NULL, line, 0));</code> | 失败时可能提前return，成功才继续；递归打印硬件节点、类型和直接链路，验证发现到的硬件树。 |
| 5 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "==========================================");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPrintPaths(s));</code> | 失败时可能提前return，成功才继续；打印缓存路径的跳数、带宽和类别，检查拓扑发现与策略改写后的可达性。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  INFO(NCCL_GRAPH, "=== System : maxWidth %2.1f totalWidth %2.1f ===", s->maxWidth, s->totalWidth);
  char line[1024];
  for (int n=0; n<s->nodes[CPU].count; n++) NCCLCHECK(ncclTopoPrintRec(s->nodes[CPU].nodes+n, NULL, line, 0));
  INFO(NCCL_GRAPH, "==========================================");
  NCCLCHECK(ncclTopoPrintPaths(s));
  return ncclSuccess;
}
```

</details>

<a id="f0504"></a>

## F0504：ncclTopoSort

用途：对节点/邻接链组织顺序做规范化，方便拓扑遍历和搜索使用一致输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(upNode)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>int l=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 循环 | <code>初始 ；继续条件 (node-&gt;links[l].remNode != upNode)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>l++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 2 / 准备局部状态 | <code>struct ncclTopoLink upLink;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>memcpy(&upLink, node-&gt;links+l, sizeof(struct ncclTopoLink));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 2 / 循环 | <code>初始 ；继续条件 (node-&gt;links[l+1].remNode)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>memcpy(node-&gt;links+l, node-&gt;links+l+1, sizeof(struct ncclTopoLink));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>l++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>memcpy(node-&gt;links+l, &upLink, sizeof(struct ncclTopoLink));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 循环 | <code>初始 int l=0;；继续条件 l&lt;node-&gt;nlinks；每轮更新 l++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 15 | 1 / 准备局部状态 | <code>struct ncclTopoLink* link = node-&gt;links+l;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 1 / 条件判断 | <code>(link-&gt;type == LINK_PCI && link-&gt;remNode != upNode)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSort(link-&gt;remNode, node));</code> | 失败时可能提前return，成功才继续；对节点/邻接链组织顺序做规范化，方便拓扑遍历和搜索使用一致输入。 |
| 19 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 20 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Shift all links to have upLink as last link
  if (upNode) {
    int l=0;
    while (node->links[l].remNode != upNode) l++;
    struct ncclTopoLink upLink;
    memcpy(&upLink, node->links+l, sizeof(struct ncclTopoLink));
    while (node->links[l+1].remNode) {
      memcpy(node->links+l, node->links+l+1, sizeof(struct ncclTopoLink));
      l++;
    }
    memcpy(node->links+l, &upLink, sizeof(struct ncclTopoLink));
  }

  // Recursively sort the PCI tree
  for (int l=0; l<node->nlinks; l++) {
    struct ncclTopoLink* link = node->links+l;
    if (link->type == LINK_PCI && link->remNode != upNode) NCCLCHECK(ncclTopoSort(link->remNode, node));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0505"></a>

## F0505：ncclTopoSortSystem

用途：对节点/邻接链组织顺序做规范化，方便拓扑遍历和搜索使用一致输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[CPU].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSort(system-&gt;nodes[CPU].nodes+n, NULL));</code> | 失败时可能提前return，成功才继续；对节点/邻接链组织顺序做规范化，方便拓扑遍历和搜索使用一致输入。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int n=0; n<system->nodes[CPU].count; n++) NCCLCHECK(ncclTopoSort(system->nodes[CPU].nodes+n, NULL));
  return ncclSuccess;
}
```

</details>

<a id="f0506"></a>

## F0506：ncclTopoAddNet

用途：从 XML 的网络/网卡属性建立 NIC/NET 硬件节点，并接到对应 PCI/CPU 层级。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int dev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlNet, "dev", &dev));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclTopoNode* net;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCreateNode(system, &net, NET, dev));</code> | 失败时可能提前return，成功才继续；在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。 |
| 5 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(xmlNet, "guid", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 7 | 0 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>sscanf(str, "0x%lx", &net-&gt;net.asic);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>net-&gt;net.asic = dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = NCCL_GRAPH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 准备局部状态 | <code>int mbps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "speed", &mbps, 0));</code> | 失败时可能提前return，成功才继续；读取可选整数属性，缺失时写调用者指定的默认值。 |
| 15 | 0 / 条件判断 | <code>(mbps &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>mbps = 10000;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>net-&gt;net.width = mbps / 8000.0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(xmlGetAttrFloat(xmlNet, "latency", &net-&gt;net.latency) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>net-&gt;net.latency = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "port", &net-&gt;net.port, 0));</code> | 失败时可能提前return，成功才继续；读取可选整数属性，缺失时写调用者指定的默认值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "gdr", &net-&gt;net.gdrSupport, 0));</code> | 失败时可能提前return，成功才继续；读取可选整数属性，缺失时写调用者指定的默认值。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "maxconn", &net-&gt;net.maxChannels, MAXCHANNELS));</code> | 失败时可能提前return，成功才继续；读取可选整数属性，缺失时写调用者指定的默认值。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIntDefault(xmlNet, "coll", &net-&gt;net.collSupport, 0));</code> | 失败时可能提前return，成功才继续；读取可选整数属性，缺失时写调用者指定的默认值。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(nic, net, LINK_NET, net-&gt;net.width));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(net, nic, LINK_NET, net-&gt;net.width));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 29 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int dev;
  NCCLCHECK(xmlGetAttrInt(xmlNet, "dev", &dev));

  struct ncclTopoNode* net;
  NCCLCHECK(ncclTopoCreateNode(system, &net, NET, dev));
  const char* str;
  NCCLCHECK(xmlGetAttr(xmlNet, "guid", &str));
  if (str) sscanf(str, "0x%lx", &net->net.asic);
  else net->net.asic = dev;

  ncclDebugNoWarn = NCCL_GRAPH;
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
```

</details>

<a id="f0507"></a>

## F0507：ncclTopoAddNic

用途：从 XML 的网络/网卡属性建立 NIC/NET 硬件节点，并接到对应 PCI/CPU 层级。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;xmlNic-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclXmlNode* xmlNet = xmlNic-&gt;subs[s];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 条件判断 | <code>(strcmp(xmlNet-&gt;name, "net") != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 6 | 1 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(xmlNet, "dev", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 8 | 1 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddNet(xmlNet, system, nic));</code> | 失败时可能提前return，成功才继续；从 XML 的网络/网卡属性建立 NIC/NET 硬件节点，并接到对应 PCI/CPU 层级。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int s=0; s<xmlNic->nSubs; s++) {
    struct ncclXmlNode* xmlNet = xmlNic->subs[s];
    if (strcmp(xmlNet->name, "net") != 0) continue;
    int index;
    NCCLCHECK(xmlGetAttrIndex(xmlNet, "dev", &index));
    if (index == -1) continue;
    NCCLCHECK(ncclTopoAddNet(xmlNet, system, nic));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0508"></a>

## F0508：ncclTopoAddGpu

用途：从 XML 的 GPU 属性建立设备节点、通信 rank 及能力信息，并关联父级硬件。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGpu, "sm", &gpu-&gt;gpu.cudaCompCap));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGpu, "rank", &gpu-&gt;gpu.rank));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGpu, "dev", &gpu-&gt;gpu.dev));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlGpu, "gdr", &gpu-&gt;gpu.gdrSupport));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(xmlGetAttrInt(xmlGpu, "sm", &gpu->gpu.cudaCompCap));
  NCCLCHECK(xmlGetAttrInt(xmlGpu, "rank", &gpu->gpu.rank));
  NCCLCHECK(xmlGetAttrInt(xmlGpu, "dev", &gpu->gpu.dev));
  NCCLCHECK(xmlGetAttrInt(xmlGpu, "gdr", &gpu->gpu.gdrSupport));
  // Do not go any further, nvlinks will be added in a second pass
  return ncclSuccess;
}
```

</details>

<a id="f0509"></a>

## F0509：ncclTopoAddPci

用途：递归建立 PCI 桥/交换节点与其子设备，使用链路代际和宽度推导带宽。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int type;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(xmlPci, "class", &str));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToInt(str, &type, kvDictPciClass));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 5 | 0 / 准备局部状态 | <code>int64_t busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(xmlPci, "busid", &str));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(busIdToInt64(str, &busId));</code> | 失败时可能提前return，成功才继续；在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。 |
| 8 | 0 / 准备局部状态 | <code>struct ncclTopoNode* node = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>struct ncclXmlNode* xmlGpu = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetSub(xmlPci, "gpu", &xmlGpu));</code> | 失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。 |
| 11 | 0 / 条件判断 | <code>(xmlGpu != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>type = GPU;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(xmlGpu, "rank", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 16 | 2 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCreateNode(system, &node, type, busId));</code> | 失败时可能提前return，成功才继续；在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddGpu(xmlGpu, system, node));</code> | 失败时可能提前return，成功才继续；从 XML 的 GPU 属性建立设备节点、通信 rank 及能力信息，并关联父级硬件。 |
| 21 | 0 / 准备局部状态 | <code>struct ncclXmlNode* xmlNic = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetSub(xmlPci, "nic", &xmlNic));</code> | 失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。 |
| 23 | 0 / 条件判断 | <code>(xmlNic != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>type = NIC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>busId &= 0xfffffffffffffff0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 2 / 准备局部状态 | <code>struct ncclTopoNode* nicNode = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNode(system, &nicNode, type, busId));</code> | 失败时可能提前return，成功才继续；在系统指定类型节点表中按 id 查找对象，返回已有节点地址。 |
| 29 | 2 / 条件判断 | <code>(nicNode == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCreateNode(system, &nicNode, type, busId));</code> | 失败时可能提前return，成功才继续；在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>node = nicNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddNic(xmlNic, system, nicNode));</code> | 失败时可能提前return，成功才继续；从 XML 的网络/网卡属性建立 NIC/NET 硬件节点，并接到对应 PCI/CPU 层级。 |
| 34 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 35 | 2 / 条件判断 | <code>(type == PCI)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCreateNode(system, &node, type, busId));</code> | 失败时可能提前return，成功才继续；在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(xmlPci, "vendor", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 39 | 4 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 6 / 调用 / 计算 / 写入 | <code>node-&gt;pci.device += strtol(str, NULL, 0) &lt;&lt; 48;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(xmlPci, "device", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 43 | 4 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 6 / 调用 / 计算 / 写入 | <code>node-&gt;pci.device += strtol(str, NULL, 0) &lt;&lt; 32;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(xmlPci, "subsystem_vendor", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 47 | 4 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 6 / 调用 / 计算 / 写入 | <code>node-&gt;pci.device += strtol(str, NULL, 0) &lt;&lt; 16;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(xmlPci, "subsystem_device", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 51 | 4 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 52 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 53 | 6 / 调用 / 计算 / 写入 | <code>node-&gt;pci.device += strtol(str, NULL, 0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 4 / 循环 | <code>初始 int s=0;；继续条件 s&lt;xmlPci-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 55 | 5 / 准备局部状态 | <code>struct ncclXmlNode* xmlSubPci = xmlPci-&gt;subs[s];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddPci(xmlSubPci, system, node));</code> | 失败时可能提前return，成功才继续；递归建立 PCI 桥/交换节点与其子设备，使用链路代际和宽度推导带宽。 |
| 57 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 58 | 0 / 条件判断 | <code>(node)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 2 / 准备局部状态 | <code>int width, speed;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 61 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlPci, "link_width", &width));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 62 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(xmlPci, "link_speed", &str));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 63 | 2 / 条件判断 | <code>(width == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 4 / 调用 / 计算 / 写入 | <code>width = 16;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 66 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToInt(str, &speed, kvDictPciGen));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 67 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(node, parent, LINK_PCI, width*speed/80.0));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 68 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(parent, node, LINK_PCI, width*speed/80.0));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 69 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const char* str;

  int type;
  NCCLCHECK(xmlGetAttrStr(xmlPci, "class", &str));
  NCCLCHECK(kvConvertToInt(str, &type, kvDictPciClass));

  int64_t busId;
  NCCLCHECK(xmlGetAttrStr(xmlPci, "busid", &str));
  NCCLCHECK(busIdToInt64(str, &busId));

  struct ncclTopoNode* node = NULL;
  struct ncclXmlNode* xmlGpu = NULL;
  NCCLCHECK(xmlGetSub(xmlPci, "gpu", &xmlGpu));
  if (xmlGpu != NULL) {
    type = GPU;
    int index;
    NCCLCHECK(xmlGetAttrIndex(xmlGpu, "rank", &index));
    if (index == -1) return ncclSuccess;
    NCCLCHECK(ncclTopoCreateNode(system, &node, type, busId));
    NCCLCHECK(ncclTopoAddGpu(xmlGpu, system, node));
  }
  struct ncclXmlNode* xmlNic = NULL;
  NCCLCHECK(xmlGetSub(xmlPci, "nic", &xmlNic));
  if (xmlNic != NULL) {
    type = NIC;
    // Ignore sub device ID and merge multi-port NICs into one PCI device.
    busId &= 0xfffffffffffffff0;
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

    for (int s=0; s<xmlPci->nSubs; s++) {
      struct ncclXmlNode* xmlSubPci = xmlPci->subs[s];
      NCCLCHECK(ncclTopoAddPci(xmlSubPci, system, node));
    }
  }

  if (node) {
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
```

</details>

<a id="f0510"></a>

## F0510：ncclTopoAddCpu

用途：建立 CPU/NUMA 节点并递归导入其 PCI、GPU、NIC 子树。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int numaId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlCpu, "numaid", &numaId));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclTopoNode* cpu;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCreateNode(system, &cpu, CPU, numaId));</code> | 失败时可能提前return，成功才继续；在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。 |
| 5 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(xmlCpu, "affinity", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 7 | 0 / 条件判断 | <code>(str != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclStrToCpuset(str, &cpu-&gt;cpu.affinity));</code> | 失败时可能提前return，成功才继续；把系统/配置的十六进制 CPU 位图解析成 cpu_set_t，支持线程亲和性设置。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(xmlCpu, "arch", &str));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToInt(str, &cpu-&gt;cpu.arch, kvDictCpuArch));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 12 | 0 / 条件判断 | <code>(cpu-&gt;cpu.arch == NCCL_TOPO_CPU_ARCH_X86)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(xmlCpu, "vendor", &str));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToInt(str, &cpu-&gt;cpu.vendor, kvDictCpuVendor));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 16 | 2 / 条件判断 | <code>(cpu-&gt;cpu.vendor == NCCL_TOPO_CPU_VENDOR_INTEL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 准备局部状态 | <code>int familyId, modelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlCpu, "familyid", &familyId));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlCpu, "modelid", &modelId));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>cpu-&gt;cpu.model = (familyId == 6 && modelId &gt;= 0x55) ? NCCL_TOPO_CPU_TYPE_SKL : NCCL_TOPO_CPU_INTEL_BDW;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 4 / 条件判断 | <code>(cpu-&gt;cpu.vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 6 / 准备局部状态 | <code>int familyId, modelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlCpu, "familyid", &familyId));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 27 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(xmlCpu, "modelid", &modelId));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 28 | 6 / 条件判断 | <code>(familyId == 7 && modelId == 0x5B)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 8 / 调用 / 计算 / 写入 | <code>cpu-&gt;cpu.model = NCCL_TOPO_CPU_TYPE_YONGFENG;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;xmlCpu-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 32 | 1 / 准备局部状态 | <code>struct ncclXmlNode* node = xmlCpu-&gt;subs[s];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 1 / 条件判断 | <code>(strcmp(node-&gt;name, "pci") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddPci(node, system, cpu));</code> | 失败时可能提前return，成功才继续；递归建立 PCI 桥/交换节点与其子设备，使用链路代际和宽度推导带宽。 |
| 36 | 1 / 条件判断 | <code>(strcmp(node-&gt;name, "nic") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 3 / 准备局部状态 | <code>struct ncclTopoNode* nic = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 39 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNode(system, &nic, NIC, 0));</code> | 失败时可能提前return，成功才继续；在系统指定类型节点表中按 id 查找对象，返回已有节点地址。 |
| 40 | 3 / 条件判断 | <code>(nic == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCreateNode(system, &nic, NIC, 0));</code> | 失败时可能提前return，成功才继续；在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。 |
| 43 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(cpu, nic, LINK_PCI, LOC_WIDTH));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 44 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(nic, cpu, LINK_PCI, LOC_WIDTH));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 45 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddNic(node, system, nic));</code> | 失败时可能提前return，成功才继续；从 XML 的网络/网卡属性建立 NIC/NET 硬件节点，并接到对应 PCI/CPU 层级。 |
| 46 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int numaId;
  NCCLCHECK(xmlGetAttrInt(xmlCpu, "numaid", &numaId));
  struct ncclTopoNode* cpu;
  NCCLCHECK(ncclTopoCreateNode(system, &cpu, CPU, numaId));
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
      int familyId, modelId;
      NCCLCHECK(xmlGetAttrInt(xmlCpu, "familyid", &familyId));
      NCCLCHECK(xmlGetAttrInt(xmlCpu, "modelid", &modelId));
      cpu->cpu.model = (familyId == 6 && modelId >= 0x55) ? NCCL_TOPO_CPU_TYPE_SKL : NCCL_TOPO_CPU_INTEL_BDW;
    } else if (cpu->cpu.vendor == NCCL_TOPO_CPU_VENDOR_ZHAOXIN) {
      int familyId, modelId;
      NCCLCHECK(xmlGetAttrInt(xmlCpu, "familyid", &familyId));
      NCCLCHECK(xmlGetAttrInt(xmlCpu, "modelid", &modelId));
      if (familyId == 7 && modelId == 0x5B) cpu->cpu.model = NCCL_TOPO_CPU_TYPE_YONGFENG;
    }
  }
  for (int s=0; s<xmlCpu->nSubs; s++) {
    struct ncclXmlNode* node = xmlCpu->subs[s];
    if (strcmp(node->name, "pci") == 0) NCCLCHECK(ncclTopoAddPci(node, system, cpu));
    if (strcmp(node->name, "nic") == 0) {
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
```

</details>

<a id="f0511"></a>

## F0511：ncclTopoAddNvLinks

用途：根据 XML/NVML 的 NVLink 远端信息补建 GPU/交换节点之间的高速链路。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(strcmp(node-&gt;name, "nvlink") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>struct ncclTopoNode* gpu = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 准备局部状态 | <code>int64_t pBusId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(busIdToInt64(parentBusId, &pBusId));</code> | 失败时可能提前return，成功才继续；在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNode(system, &gpu, GPU, pBusId));</code> | 失败时可能提前return，成功才继续；在系统指定类型节点表中按 id 查找对象，返回已有节点地址。 |
| 7 | 2 / 条件判断 | <code>(gpu == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>WARN("Add NVLink error : could not find GPU %lx", pBusId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 2 / 准备局部状态 | <code>int count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(node, "count", &count));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 13 | 2 / 准备局部状态 | <code>const char* targetClass;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(node, "tclass", &targetClass));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 15 | 2 / 准备局部状态 | <code>int targetType;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(kvConvertToInt(targetClass, &targetType, kvDictPciClass));</code> | 失败时可能提前return，成功才继续；在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。 |
| 17 | 2 / 准备局部状态 | <code>struct ncclTopoNode* remote = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 2 / 条件判断 | <code>(targetType == GPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 准备局部状态 | <code>const char* target;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(node, "target", &target));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 22 | 4 / 准备局部状态 | <code>int64_t busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(busIdToInt64(target, &busId));</code> | 失败时可能提前return，成功才继续；在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNode(system, &remote, GPU, busId));</code> | 失败时可能提前return，成功才继续；在系统指定类型节点表中按 id 查找对象，返回已有节点地址。 |
| 25 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 26 | 4 / 条件判断 | <code>(targetType == CPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(findLocalCpu(gpu, &remote));</code> | 失败时可能提前return，成功才继续；沿硬件连接查找设备所在 CPU 节点，用于补建 CPU/PCI 关系。 |
| 29 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 30 | 6 / 条件判断 | <code>(system-&gt;nodes[NVS].count == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 8 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCreateNode(system, &remote, NVS, 0));</code> | 失败时可能提前return，成功才继续；在固定节点表中新建指定类型/id 的硬件节点，初始化属性后交给调用者继续填充。 |
| 33 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 34 | 8 / 调用 / 计算 / 写入 | <code>remote = system-&gt;nodes[NVS].nodes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 2 / 条件判断 | <code>(remote)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 4 / 准备局部状态 | <code>float nvlSpeed = ncclTopoNVLinkSpeed(gpu-&gt;gpu.cudaCompCap);</code> | 按 GPU 计算能力选择该代 NVLink 单链估计速率，供路径通道预算使用。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(gpu, remote, LINK_NVL, count*nvlSpeed));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 39 | 4 / 条件判断 | <code>(remote-&gt;type != GPU)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectNodes(remote, gpu, LINK_NVL, count*nvlSpeed));</code> | 失败时可能提前return，成功才继续；给两个硬件节点增加或合并同类链路及宽度，表示自动发现的物理连接。 |
| 42 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 43 | 2 / 准备局部状态 | <code>const char* busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(node, "busid", &busId));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 45 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;node-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 46 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddNvLinks(node-&gt;subs[s], system, busId ? busId : parentBusId));</code> | 失败时可能提前return，成功才继续；根据 XML/NVML 的 NVLink 远端信息补建 GPU/交换节点之间的高速链路。 |
| 47 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 48 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (strcmp(node->name, "nvlink") == 0) {
    struct ncclTopoNode* gpu = NULL;
    int64_t pBusId;
    NCCLCHECK(busIdToInt64(parentBusId, &pBusId));
    NCCLCHECK(ncclTopoGetNode(system, &gpu, GPU, pBusId));
    if (gpu == NULL) {
      WARN("Add NVLink error : could not find GPU %lx", pBusId);
      return ncclInternalError;
    }
    int count;
    NCCLCHECK(xmlGetAttrInt(node, "count", &count));
    const char* targetClass;
    NCCLCHECK(xmlGetAttrStr(node, "tclass", &targetClass));
    int targetType;
    NCCLCHECK(kvConvertToInt(targetClass, &targetType, kvDictPciClass));
    struct ncclTopoNode* remote = NULL;
    if (targetType == GPU) {
      // NVL P2P connection to another GPU
      const char* target;
      NCCLCHECK(xmlGetAttrStr(node, "target", &target));
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
      float nvlSpeed = ncclTopoNVLinkSpeed(gpu->gpu.cudaCompCap);
      NCCLCHECK(ncclTopoConnectNodes(gpu, remote, LINK_NVL, count*nvlSpeed));
      if (remote->type != GPU) {
        NCCLCHECK(ncclTopoConnectNodes(remote, gpu, LINK_NVL, count*nvlSpeed));
      }
    }
  } else {
    const char* busId;
    NCCLCHECK(xmlGetAttr(node, "busid", &busId));
    for (int s=0; s<node->nSubs; s++) {
      NCCLCHECK(ncclTopoAddNvLinks(node->subs[s], system, busId ? busId : parentBusId));
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0512"></a>

## F0512：ncclTopoGetSystemFromXml

用途：把硬件 XML 的层级属性转换成 ncclTopoSystem 节点和双向链路模型。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(topoSystem, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclXmlNode* topNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlFindTag(xml, "system", &topNode));</code> | 失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。 |
| 4 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;topNode-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 准备局部状态 | <code>struct ncclXmlNode* node = topNode-&gt;subs[s];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 条件判断 | <code>(strcmp(node-&gt;name, "cpu") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddCpu(node, *topoSystem));</code> | 失败时可能提前return，成功才继续；建立 CPU/NUMA 节点并递归导入其 PCI、GPU、NIC 子树。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoAddNvLinks(topNode, *topoSystem, NULL));</code> | 失败时可能提前return，成功才继续；根据 XML/NVML 的 NVLink 远端信息补建 GPU/交换节点之间的高速链路。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFlattenBcmSwitches(*topoSystem));</code> | 失败时可能提前return，成功才继续；识别可合并表示的 Broadcom PCI 交换结构，简化建图而不额外发起硬件传输。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoConnectCpus(*topoSystem));</code> | 失败时可能提前return，成功才继续；根据 CPU 类型补建 CPU 之间的互连边，使跨 NUMA 路径可被统一搜索。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSortSystem(*topoSystem));</code> | 失败时可能提前return，成功才继续；对节点/邻接链组织顺序做规范化，方便拓扑遍历和搜索使用一致输入。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclCalloc(topoSystem, 1));
  struct ncclXmlNode* topNode;
  NCCLCHECK(xmlFindTag(xml, "system", &topNode));
  for (int s=0; s<topNode->nSubs; s++) {
    struct ncclXmlNode* node = topNode->subs[s];
    if (strcmp(node->name, "cpu") == 0) NCCLCHECK(ncclTopoAddCpu(node, *topoSystem));
  }
  NCCLCHECK(ncclTopoAddNvLinks(topNode, *topoSystem, NULL));

  NCCLCHECK(ncclTopoFlattenBcmSwitches(*topoSystem));
  NCCLCHECK(ncclTopoConnectCpus(*topoSystem));
  NCCLCHECK(ncclTopoSortSystem(*topoSystem));

  return ncclSuccess;
}
```

</details>

<a id="f0513"></a>

## F0513：xmlInitAttrInt

用途：只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>index = node-&gt;nAttrs++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].key, attrName, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>snprintf(node-&gt;attrs[index].value, MAX_STR_LEN, "%d", value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    snprintf(node->attrs[index].value, MAX_STR_LEN, "%d", value);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0514"></a>

## F0514：xmlInitAttrUint64

用途：只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>index = node-&gt;nAttrs++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].key, attrName, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>snprintf(node-&gt;attrs[index].value, MAX_STR_LEN, "0x%lx", value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    snprintf(node->attrs[index].value, MAX_STR_LEN, "0x%lx", value);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0515"></a>

## F0515：xmlInitAttrFloat

用途：只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>index = node-&gt;nAttrs++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].key, attrName, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>snprintf(node-&gt;attrs[index].value, MAX_STR_LEN, "%f", value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    snprintf(node->attrs[index].value, MAX_STR_LEN, "%f", value);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0516"></a>

## F0516：ncclTopoGetSystem

用途：读入可选拓扑文件并补齐当前 rank 所见 GPU/NIC 的自动发现信息，再生成硬件图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclXml* xml;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&xml, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 准备局部状态 | <code>char* xmlTopoFile = getenv("NCCL_TOPO_FILE");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(xmlTopoFile)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_TOPO_FILE set by environment to %s", xmlTopoFile);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromFile(xmlTopoFile, xml, 1));</code> | 失败时可能提前return，成功才继续；打开拓扑文件并解析指定版本的系统 XML，按 warn 决定缺失文件的处理方式。 |
| 8 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromFile("/var/run/nvidia-topologyd/virtualTopology.xml", xml, 0));</code> | 失败时可能提前return，成功才继续；打开拓扑文件并解析指定版本的系统 XML，按 warn 决定缺失文件的处理方式。 |
| 10 | 0 / 条件判断 | <code>(xml-&gt;maxIndex == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 准备局部状态 | <code>struct ncclXmlNode* top;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, NULL, "system", &top));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(top, "version", NCCL_TOPO_XML_VERSION));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 15 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;nRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 1 / 条件判断 | <code>(comm-&gt;peerInfo[r].hostHash == comm-&gt;peerInfo[comm-&gt;rank].hostHash)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 3 / 准备局部状态 | <code>char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(int64ToBusId(comm-&gt;peerInfo[r].busId, busId));</code> | 失败时可能提前return，成功才继续；在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。 |
| 20 | 3 / 准备局部状态 | <code>struct ncclXmlNode* node;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFillGpu(xml, busId, &node));</code> | 失败时可能提前return，成功才继续；在 XML 找到或添加指定 GPU，触发系统/NVML 自动发现补全其属性。 |
| 22 | 3 / 条件判断 | <code>(node == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 25 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(node, "keep", 1));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 26 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(node, "rank", r));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(node, "gdr", comm-&gt;peerInfo[r].gdrSupport));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 28 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 29 | 0 / 准备局部状态 | <code>int netDevCount = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 0 / 条件判断 | <code>(collNetSupport())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetDevices(&netDevCount));</code> | 失败时可能提前return，成功才继续；查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 33 | 2 / 循环 | <code>初始 int n=0;；继续条件 n&lt;netDevCount；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 34 | 3 / 准备局部状态 | <code>ncclNetProperties_t props;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetGetProperties(n, &props));</code> | 失败时可能提前return，成功才继续；读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 36 | 3 / 准备局部状态 | <code>struct ncclXmlNode* netNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFillNet(xml, props.pciPath, props.name, &netNode));</code> | 失败时可能提前return，成功才继续；将后端提供的网络设备属性补入 XML，关联其 PCI/NIC 层次与 GDR/CollNet 能力。 |
| 38 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(netNode, "keep", 1));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 39 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(netNode, "dev", n));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 40 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "speed", props.speed));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 41 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "port", props.port));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 42 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrUint64(netNode, "guid", props.guid));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 43 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "maxconn", props.maxComms));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 44 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "gdr", props.ptrSupport & NCCL_PTR_CUDA ? 1 : 0));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 45 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "coll", 1));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 46 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 0 / 条件判断 | <code>(netDevCount == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetDevices(&netDevCount));</code> | 失败时可能提前return，成功才继续；查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 50 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;netDevCount；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 51 | 1 / 准备局部状态 | <code>ncclNetProperties_t props;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetGetProperties(n, &props));</code> | 失败时可能提前return，成功才继续；读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 53 | 1 / 准备局部状态 | <code>struct ncclXmlNode* netNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoFillNet(xml, props.pciPath, props.name, &netNode));</code> | 失败时可能提前return，成功才继续；将后端提供的网络设备属性补入 XML，关联其 PCI/NIC 层次与 GDR/CollNet 能力。 |
| 55 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(netNode, "keep", 1));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 56 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(netNode, "dev", n));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 57 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "speed", props.speed));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 58 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "port", props.port));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 59 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrFloat(netNode, "latency", props.latency));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 60 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrUint64(netNode, "guid", props.guid));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 61 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "maxconn", props.maxComms));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 62 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlInitAttrInt(netNode, "gdr", props.ptrSupport & NCCL_PTR_CUDA ? 1 : 0));</code> | 失败时可能提前return，成功才继续；只在属性缺失时写入默认数值，保留用户拓扑文件明确提供的值。 |
| 63 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 64 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoTrimXml(xml));</code> | 失败时可能提前return，成功才继续；裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。 |
| 65 | 0 / 调用 / 计算 / 写入 | <code>xmlTopoFile = getenv("NCCL_TOPO_DUMP_FILE");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 66 | 0 / 条件判断 | <code>(xmlTopoFile && comm-&gt;rank == ncclParamTopoDumpFileRank())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 67 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 68 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_TOPO_DUMP_FILE set by environment to %s", xmlTopoFile);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 69 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoDumpXmlToFile(xmlTopoFile, xml));</code> | 失败时可能提前return，成功才继续；递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。 |
| 70 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetSystemFromXml(xml, system));</code> | 失败时可能提前return，成功才继续；把硬件 XML 的层级属性转换成 ncclTopoSystem 节点和双向链路模型。 |
| 71 | 0 / 调用 / 计算 / 写入 | <code>free(xml);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 72 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclXml* xml;
  NCCLCHECK(ncclCalloc(&xml, 1));
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
    struct ncclXmlNode* top;
    NCCLCHECK(xmlAddNode(xml, NULL, "system", &top));
    NCCLCHECK(xmlSetAttrInt(top, "version", NCCL_TOPO_XML_VERSION));
  }

  // Auto-detect GPUs if needed
  for (int r=0; r<comm->nRanks; r++) {
    if (comm->peerInfo[r].hostHash == comm->peerInfo[comm->rank].hostHash) {
      char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
      NCCLCHECK(int64ToBusId(comm->peerInfo[r].busId, busId));
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
  int netDevCount = 0;
  if (collNetSupport()) {
    NCCLCHECK(collNetDevices(&netDevCount));
    for (int n=0; n<netDevCount; n++) {
      ncclNetProperties_t props;
      NCCLCHECK(collNetGetProperties(n, &props));
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
  for (int n=0; n<netDevCount; n++) {
    ncclNetProperties_t props;
    NCCLCHECK(ncclNetGetProperties(n, &props));
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
```

</details>

<a id="f0517"></a>

## F0517：ncclTopoGetLocalNet

用途：按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));</code> | 失败时可能提前return，成功才继续；把硬件 id 或通信 rank 查找为给定节点类型数组下标，查不到时返回错误。 |
| 3 | 0 / 准备局部状态 | <code>int minType = PATH_SYS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>float maxWidth = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>int count = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int* nets;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&nets, system-&gt;nodes[NET].count));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;system-&gt;nodes[NET].count；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 准备局部状态 | <code>struct ncclTopoLinkList* path = system-&gt;nodes[NET].nodes[n].paths[GPU]+g;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 条件判断 | <code>(path-&gt;width &gt; maxWidth &#124;&#124; (path-&gt;width == maxWidth && path-&gt;type &lt; minType))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>maxWidth = path-&gt;width;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 3 / 调用 / 计算 / 写入 | <code>minType = path-&gt;type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 3 / 调用 / 计算 / 写入 | <code>count = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 1 / 条件判断 | <code>(path-&gt;width == maxWidth && path-&gt;type == minType)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>nets[count++] = system-&gt;nodes[NET].nodes[n].id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 0 / 条件判断 | <code>(count == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>*id = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>free(nets);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 24 | 0 / 准备局部状态 | <code>int rr = system-&gt;nodes[GPU].nodes[g].gpu.dev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>*id = nets[rr%count];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>free(nets);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int g;
  NCCLCHECK(ncclTopoRankToIndex(system, rank, &g));
  int minType = PATH_SYS;
  float maxWidth = 0;
  int count = 0;
  int* nets;
  NCCLCHECK(ncclCalloc(&nets, system->nodes[NET].count));
  for (int n=0; n<system->nodes[NET].count; n++) {
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

  int rr = system->nodes[GPU].nodes[g].gpu.dev;
  *id = nets[rr%count];
  free(nets);
  return ncclSuccess;
}
```

</details>

<a id="f0518"></a>

## F0518：ncclTopoCpuType

用途：从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*arch = system-&gt;nodes[CPU].nodes[0].cpu.arch;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>*vendor = system-&gt;nodes[CPU].nodes[0].cpu.vendor;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*model = system-&gt;nodes[CPU].nodes[0].cpu.model;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *arch = system->nodes[CPU].nodes[0].cpu.arch;
  *vendor = system->nodes[CPU].nodes[0].cpu.vendor;
  *model = system->nodes[CPU].nodes[0].cpu.model;
  return ncclSuccess;
}
```

</details>

<a id="f0519"></a>

## F0519：ncclTopoGetCpuAffinity

用途：结合 GPU 的近端 CPU/NUMA 亲和范围与当前进程约束，生成线程亲和性掩码。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclTopoNode* cpu = NULL, *gpu = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(system-&gt;nodes[GPU].nodes[g].gpu.rank == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>gpu = system-&gt;nodes[GPU].nodes+g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 准备局部状态 | <code>int cpuIndex = -1, minHops = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 3 / 循环 | <code>初始 int c=0;；继续条件 c&lt;system-&gt;nodes[CPU].count；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 4 / 准备局部状态 | <code>int nHops = system-&gt;nodes[GPU].nodes[g].paths[CPU][c].count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 4 / 条件判断 | <code>(cpuIndex == -1 &#124;&#124; nHops &lt; minHops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 6 / 调用 / 计算 / 写入 | <code>cpuIndex = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 6 / 调用 / 计算 / 写入 | <code>minHops = nHops;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 3 / 调用 / 计算 / 写入 | <code>cpu = system-&gt;nodes[CPU].nodes+cpuIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 0 / 条件判断 | <code>(cpu == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>WARN("Set CPU affinity : unable to find GPU/CPU for rank %d", rank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 20 | 0 / 准备局部状态 | <code>cpu_set_t mask;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(sched_getaffinity(0, sizeof(cpu_set_t), &mask), "sched_getaffinity");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 22 | 0 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 23 | 1 / 准备局部状态 | <code>char affinityStr[sizeof(cpu_set_t)*2];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCpusetToStr(&mask, affinityStr));</code> | 失败时可能提前return，成功才继续；把 cpu_set_t 格式化成十六进制文本，方便导出/诊断拓扑 CPU affinity。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "Current affinity for GPU %d is %s", gpu-&gt;gpu.dev, affinityStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 0 / 准备局部状态 | <code>cpu_set_t cpuMask = cpu-&gt;cpu.affinity;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 0 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 28 | 1 / 准备局部状态 | <code>char affinityStr[sizeof(cpu_set_t)*2];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCpusetToStr(&cpuMask, affinityStr));</code> | 失败时可能提前return，成功才继续；把 cpu_set_t 格式化成十六进制文本，方便导出/诊断拓扑 CPU affinity。 |
| 30 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "CPU GPU affinity for GPU %d is %s", gpu-&gt;gpu.dev, affinityStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 0 / 准备局部状态 | <code>cpu_set_t finalMask;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 0 / 条件判断 | <code>(ncclParamIgnoreCpuAffinity())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>finalMask = cpuMask;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>CPU_AND(&finalMask, &mask, &cpuMask);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>memcpy(affinity, &finalMask, sizeof(cpu_set_t));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 0 / 条件判断 | <code>(CPU_COUNT(&finalMask))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 2 / 准备局部状态 | <code>char affinityStr[sizeof(cpu_set_t)*2];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCpusetToStr(&finalMask, affinityStr));</code> | 失败时可能提前return，成功才继续；把 cpu_set_t 格式化成十六进制文本，方便导出/诊断拓扑 CPU affinity。 |
| 42 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Setting affinity for GPU %d to %s", gpu-&gt;gpu.dev, affinityStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclTopoNode* cpu = NULL, *gpu = NULL;
  for (int g=0; g<system->nodes[GPU].count; g++) {
    if (system->nodes[GPU].nodes[g].gpu.rank == rank) {
      gpu = system->nodes[GPU].nodes+g;
      // Find closer CPU
      int cpuIndex = -1, minHops = 0;
      for (int c=0; c<system->nodes[CPU].count; c++) {
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
  cpu_set_t mask;
  SYSCHECK(sched_getaffinity(0, sizeof(cpu_set_t), &mask), "sched_getaffinity");

#ifdef ENABLE_TRACE
  {
    char affinityStr[sizeof(cpu_set_t)*2];
    NCCLCHECK(ncclCpusetToStr(&mask, affinityStr));
    TRACE(NCCL_INIT, "Current affinity for GPU %d is %s", gpu->gpu.dev, affinityStr);
  }
#endif

  // Get the affinity of the CPU close to our GPU.
  cpu_set_t cpuMask = cpu->cpu.affinity;

#ifdef ENABLE_TRACE
  {
    char affinityStr[sizeof(cpu_set_t)*2];
    NCCLCHECK(ncclCpusetToStr(&cpuMask, affinityStr));
    TRACE(NCCL_INIT, "CPU GPU affinity for GPU %d is %s", gpu->gpu.dev, affinityStr);
  }
#endif

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
    char affinityStr[sizeof(cpu_set_t)*2];
    NCCLCHECK(ncclCpusetToStr(&finalMask, affinityStr));
    INFO(NCCL_INIT, "Setting affinity for GPU %d to %s", gpu->gpu.dev, affinityStr);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0520"></a>

## F0520：ncclTopoGetNetCount

用途：返回当前裁剪后硬件图中的 NET 设备数量，用于判断是否存在网络路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*count = system-&gt;nodes[NET].count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *count = system->nodes[NET].count;
  return ncclSuccess;
}
```

</details>

<a id="f0521"></a>

## F0521：ncclTopoGetCompCap

用途：遍历 GPU 得到计算能力的最小/最大值，使协议与调优模型兼容参与设备。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(system-&gt;nodes[GPU].count == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>int min, max;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>min = max = system-&gt;nodes[GPU].nodes[0].gpu.cudaCompCap;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环 | <code>初始 int g=1;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>min = std::min(min, system-&gt;nodes[GPU].nodes[g].gpu.cudaCompCap);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>max = std::max(max, system-&gt;nodes[GPU].nodes[g].gpu.cudaCompCap);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 条件判断 | <code>(ccMin)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>*ccMin = min;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 条件判断 | <code>(ccMax)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>*ccMax = max;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (system->nodes[GPU].count == 0) return ncclInternalError;
  int min, max;
  min = max = system->nodes[GPU].nodes[0].gpu.cudaCompCap;
  for (int g=1; g<system->nodes[GPU].count; g++) {
    min = std::min(min, system->nodes[GPU].nodes[g].gpu.cudaCompCap);
    max = std::max(max, system->nodes[GPU].nodes[g].gpu.cudaCompCap);
  }
  if (ccMin) *ccMin = min;
  if (ccMax) *ccMax = max;
  return ncclSuccess;
}
```

</details>

<a id="f0522"></a>

## F0522：ncclTopoGetLocalRank

用途：把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int g=0;；继续条件 g&lt;system-&gt;nodes[GPU].count；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 条件判断 | <code>(system-&gt;nodes[GPU].nodes[g].gpu.rank == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 3 / 调用 / 计算 / 写入 | <code>*localRank = g;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>WARN("Could not find local GPU with rank %d\n", rank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int g=0; g<system->nodes[GPU].count; g++) {
    if (system->nodes[GPU].nodes[g].gpu.rank == rank) {
      *localRank = g;
      return ncclSuccess;
    }
  }
  WARN("Could not find local GPU with rank %d\n", rank);
  return ncclInternalError;
}
```

</details>

