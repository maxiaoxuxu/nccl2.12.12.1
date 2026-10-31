# src/graph/tuning.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0528"></a>

## F0528：getNthreads

用途：读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int nt = env;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(nt &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 条件判断 | <code>(nt % WARP_SIZE != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 调用 / 计算 / 写入 | <code>WARN("Invalid %s %d (must be a multiple of %d)", name, nt, WARP_SIZE);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 4 / 调用 / 计算 / 写入 | <code>nt = max;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 9 | 4 / 条件判断 | <code>(nt &gt; max)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 6 / 调用 / 计算 / 写入 | <code>WARN("Invalid %s %d (maximum %d).", name, nt, max);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 6 / 调用 / 计算 / 写入 | <code>nt = max;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 14 | 6 / 条件判断 | <code>(nt &lt; min)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 8 / 调用 / 计算 / 写入 | <code>WARN("Invalid %s %d (minimum %d).", name, nt, min);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 8 / 调用 / 计算 / 写入 | <code>nt = min;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>nt = def;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 函数返回 | <code>return nt;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int nt = env;
  if (nt > 0) {
    if (nt % WARP_SIZE != 0) {
      WARN("Invalid %s %d (must be a multiple of %d)", name, nt, WARP_SIZE);
      nt = max;
    } else if (nt > max) {
      WARN("Invalid %s %d (maximum %d).", name, nt, max);
      nt = max;
    } else if (nt < min) {
      WARN("Invalid %s %d (minimum %d).", name, nt, min);
      nt = min;
     }
  } else {
    nt = def;
  }
  return nt;
}
```

</details>

<a id="f0529"></a>

## F0529：parseList

用途：解析算法/协议等配置名单和排除前缀，生成各候选是否启用的布尔表。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int def, set;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(str[0] == '^')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>def = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>set = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>str++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>def = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>set = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nelems；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>list[i] = def;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 准备局部状态 | <code>char* tokStr = strdup(str);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>char* tmpStr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 准备局部状态 | <code>char* token = strtok_r(tokStr, ",", &tmpStr);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 循环 | <code>初始 ；继续条件 (token)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nelems；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 18 | 2 / 条件判断 | <code>(strcasecmp(token, elems[i]) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>list[i] = set;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 1 / 调用 / 计算 / 写入 | <code>token = strtok_r(NULL, ",", &tmpStr);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>free(tokStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int def, set;
  if (str[0] == '^') {
    def = 1; set = 0; str++;
  } else {
    def = 0; set = 1;
  }
  for (int i=0; i<nelems; i++) list[i] = def;
  char* tokStr = strdup(str);
  char* tmpStr;
  char* token = strtok_r(tokStr, ",", &tmpStr);
  while (token) {
    for (int i=0; i<nelems; i++)
      if (strcasecmp(token, elems[i]) == 0) list[i] = set;
    token = strtok_r(NULL, ",", &tmpStr);
  }
  free(tokStr);
  return ncclSuccess;
}
```

</details>

<a id="f0530"></a>

## F0530：ncclTopoTuneModel

用途：结合拓扑图带宽、节点规模、GPU 架构和配置构建各 API/算法/协议的延迟与带宽表。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int simpleDefaultThreads = (ringGraph-&gt;speedIntra*ringGraph-&gt;nChannels &lt;= PCI_WIDTH) ? 256 : NCCL_SIMPLE_MAX_NTHREADS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;maxThreads[NCCL_ALGO_RING][NCCL_PROTO_SIMPLE] = getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_SIMPLE_MAX_NTHREADS, simpleDefaultThreads);</code> | 读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE] = getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_SIMPLE_MAX_NTHREADS, NCCL_SIMPLE_MAX_NTHREADS);</code> | 读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE] = NCCL_SIMPLE_MAX_NTHREADS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;maxThreads[NCCL_ALGO_RING][NCCL_PROTO_LL] = comm-&gt;maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_LL] = comm-&gt;maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_LL] = getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_LL_MAX_NTHREADS, NCCL_LL_MAX_NTHREADS);</code> | 读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;maxThreads[NCCL_ALGO_RING][NCCL_PROTO_LL128] = comm-&gt;maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_LL128] = comm-&gt;maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_LL128] = getNthreads("NCCL_LL128_NTHREADS", ncclParamLl128Nthreads(), NCCL_LL128_MAX_NTHREADS/4, NCCL_LL128_MAX_NTHREADS, NCCL_LL128_MAX_NTHREADS);</code> | 读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。 |
| 7 | 0 / 准备局部状态 | <code>int nNodes = comm-&gt;nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>int nRanks = comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 条件判断 | <code>(nRanks &lt;= 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 准备局部状态 | <code>int compCap80 = minCompCap == 80 && maxCompCap == 80 ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>int cpuArch, cpuVendor, cpuModel;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCpuType(comm-&gt;topo, &cpuArch, &cpuVendor, &cpuModel));</code> | 失败时可能提前return，成功才继续；从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。 |
| 15 | 0 / 准备局部状态 | <code>int index2 = nNodes &lt;= 2 ? nNodes-1 : 2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 准备局部状态 | <code>int index1 = nNodes == 1 ? compCap80 : cpuVendor == NCCL_TOPO_CPU_VENDOR_AMD ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 0 / 准备局部状态 | <code>double llMaxBw = llMaxBws[index1][index2];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 0 / 准备局部状态 | <code>double perChMaxTreeBw = perChMaxTreeBws[compCap80][index2];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 0 / 条件判断 | <code>(cpuArch == NCCL_TOPO_CPU_ARCH_POWER)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>hwLat[NCCL_HW_PCI][NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE] = hwLat[NCCL_HW_PCI][NCCL_ALGO_RING][NCCL_PROTO_SIMPLE];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 准备局部状态 | <code>float ppn = (float)nRanks / nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 0 / 准备局部状态 | <code>struct ncclTopoGraph* graphs[NCCL_NUM_ALGORITHMS] = { treeGraph, ringGraph, collNetGraph };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 0 / 准备局部状态 | <code>int intraHw[NCCL_NUM_ALGORITHMS], hw[NCCL_NUM_ALGORITHMS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 0 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>intraHw[a] = graphs[a]-&gt;typeIntra == LINK_NVL ? NCCL_HW_NVLINK : NCCL_HW_PCI;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 28 | 0 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>hw[a] = nNodes == 1 ? intraHw[a] : NCCL_HW_NET;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 0 / 循环 | <code>初始 int coll=0;；继续条件 coll&lt;NCCL_NUM_FUNCTIONS；每轮更新 coll++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 32 | 1 / 准备局部状态 | <code>int nsteps = coll == ncclFuncAllReduce ? 2*(nRanks-1) : coll == ncclFuncReduceScatter &#124;&#124; coll == ncclFuncAllGather ? nRanks-1 : nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 1 / 准备局部状态 | <code>int nInterSteps = coll == ncclFuncAllReduce ? (nNodes &gt; 1 ? 2*nNodes :0) : coll == ncclFuncReduceScatter &#124;&#124; coll == ncclFuncAllGather ? nNodes-1 : nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 1 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 35 | 2 / 条件判断 | <code>(coll != ncclFuncAllReduce && a != NCCL_ALGO_RING)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 4 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 38 | 2 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 3 / 准备局部状态 | <code>float speed = nNodes &lt;= 2 &#124;&#124; a == NCCL_ALGO_COLLNET ? graphs[a]-&gt;speedIntra : graphs[a]-&gt;speedInter;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 3 / 准备局部状态 | <code>float busBw = graphs[a]-&gt;nChannels * speed;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 3 / 条件判断 | <code>(compCap80)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 5 / 调用 / 计算 / 写入 | <code>busBw = std::min(busBw, 235.0f);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 44 | 3 / 条件判断 | <code>(a == NCCL_ALGO_RING && p == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 5 / 调用 / 计算 / 写入 | <code>busBw = std::min(llMaxBw, busBw * ((nNodes &gt; 1 &#124;&#124; coll == ncclFuncAllReduce &#124;&#124; coll == ncclFuncReduce) ? 1.0/4.0 : 1.0/3.0));</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 47 | 3 / 条件判断 | <code>(a == NCCL_ALGO_RING && p == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 5 / 调用 / 计算 / 写入 | <code>busBw = std::min(busBw * (ppn &lt; 2 ? 0.7 : 0.92 ), ll128MaxBwPerCh*graphs[a]-&gt;nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 50 | 3 / 条件判断 | <code>(a == NCCL_ALGO_TREE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 5 / 调用 / 计算 / 写入 | <code>busBw = std::min(busBw*.92, graphs[a]-&gt;nChannels*perChMaxTreeBw);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 53 | 3 / 条件判断 | <code>(a == NCCL_ALGO_TREE && p == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 54 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 55 | 5 / 调用 / 计算 / 写入 | <code>busBw = std::min(busBw*1.0/3.8, llMaxBw);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 56 | 3 / 条件判断 | <code>(a == NCCL_ALGO_TREE && p == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 5 / 调用 / 计算 / 写入 | <code>busBw = std::min(busBw * (nNodes == 1 ? 7.0/9.0 : 120.0/128.0), ll128MaxBwPerCh*graphs[a]-&gt;nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 59 | 3 / 条件判断 | <code>(a == NCCL_ALGO_COLLNET && p != NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 60 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 61 | 5 / 调用 / 计算 / 写入 | <code>busBw = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 3 / 准备局部状态 | <code>float ratio = (a != NCCL_ALGO_RING) ? .5 : (1.0 * nRanks) / nsteps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 63 | 3 / 调用 / 计算 / 写入 | <code>comm-&gt;bandwidths[coll][a][p] = busBw * ratio;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 64 | 3 / 调用 / 计算 / 写入 | <code>comm-&gt;latencies[coll][a][p] = baseLat[a][p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 65 | 3 / 准备局部状态 | <code>float intraLat = hwLat[intraHw[a]][a][p];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 66 | 3 / 准备局部状态 | <code>float interLat = graphs[a]-&gt;latencyInter ? graphs[a]-&gt;latencyInter : hwLat[NCCL_HW_NET][a][p];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 67 | 3 / 条件判断 | <code>(nNodes &gt; 1 && p == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 5 / 调用 / 计算 / 写入 | <code>intraLat *= 1.8;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 3 / 条件判断 | <code>(a == NCCL_ALGO_RING)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 71 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 72 | 5 / 准备局部状态 | <code>float lat = hwLat[hw[a]][a][p];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 73 | 5 / 条件判断 | <code>((coll == ncclFuncReduce &#124;&#124; coll == ncclFuncBroadcast))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 74 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 75 | 7 / 条件判断 | <code>(ringGraph-&gt;sameChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 76 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 77 | 9 / 调用 / 计算 / 写入 | <code>comm-&gt;latencies[coll][a][p] += lat;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 79 | 9 / 条件判断 | <code>(p == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 80 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 81 | 11 / 调用 / 计算 / 写入 | <code>lat = hwLat[hw[a]][NCCL_ALGO_TREE][p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 9 / 调用 / 计算 / 写入 | <code>comm-&gt;latencies[coll][a][p] += nsteps*lat;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 84 | 7 / 调用 / 计算 / 写入 | <code>comm-&gt;latencies[coll][a][p] += (nsteps-nInterSteps)*intraLat + nInterSteps*interLat;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 86 | 5 / 条件判断 | <code>(a == NCCL_ALGO_TREE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 87 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 88 | 7 / 调用 / 计算 / 写入 | <code>comm-&gt;latencies[coll][a][p] += 2 * ((nRanks/nNodes-1) * intraLat + log2i(nNodes) * interLat);</code> | 用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。 |
| 89 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 90 | 7 / 调用 / 计算 / 写入 | <code>comm-&gt;latencies[coll][a][p] += 2 * (std::min(1, (nRanks/nNodes-1)) * intraLat + (nRanks/nNodes-1) * 0.5) + interLat;</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 91 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 92 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 93 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 94 | 0 / 准备局部状态 | <code>int protoEnable[NCCL_NUM_PROTOCOLS] = { 1, 2, 1 };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 95 | 0 / 准备局部状态 | <code>int algoEnable[NCCL_NUM_ALGORITHMS] = { 1, 1, 1 };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 96 | 0 / 准备局部状态 | <code>const char *protoStr = getenv("NCCL_PROTO");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 97 | 0 / 条件判断 | <code>(protoStr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 98 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 99 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_PROTO set by environment to %s", protoStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 100 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(parseList(protoStr, ncclProtoStr, NCCL_NUM_PROTOCOLS, protoEnable));</code> | 失败时可能提前return，成功才继续；解析算法/协议等配置名单和排除前缀，生成各候选是否启用的布尔表。 |
| 101 | 0 / 准备局部状态 | <code>const char *algoStr = getenv("NCCL_ALGO");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 102 | 0 / 条件判断 | <code>(algoStr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 103 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 104 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_ALGO set by environment to %s", algoStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 105 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(parseList(algoStr, ncclAlgoStr, NCCL_NUM_ALGORITHMS, algoEnable));</code> | 失败时可能提前return，成功才继续；解析算法/协议等配置名单和排除前缀，生成各候选是否启用的布尔表。 |
| 106 | 0 / 条件判断 | <code>(comm-&gt;collNetSupport == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 107 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 108 | 2 / 调用 / 计算 / 写入 | <code>algoEnable[NCCL_ALGO_COLLNET] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 109 | 2 / 条件判断 | <code>(algoEnable[NCCL_ALGO_RING] == 0 && algoEnable[NCCL_ALGO_TREE] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 110 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 111 | 4 / 调用 / 计算 / 写入 | <code>algoEnable[NCCL_ALGO_RING] = algoEnable[NCCL_ALGO_TREE] = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 112 | 4 / 条件判断 | <code>(comm-&gt;rank == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 113 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 114 | 6 / 调用 / 计算 / 写入 | <code>WARN("CollNet is not supported or fails to initialize, ignoring NCCL_ALGO=COLLNET");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 115 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;NCCL_NUM_FUNCTIONS；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 116 | 1 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 117 | 2 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 118 | 3 / 准备局部状态 | <code>int pEnable = protoEnable[p];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 119 | 3 / 条件判断 | <code>(pEnable == 2 && p == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 120 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 121 | 5 / 调用 / 计算 / 写入 | <code>pEnable = (graphs[a]-&gt;typeInter &lt;= PATH_PXB) && graphs[a]-&gt;typeIntra &lt;= PATH_NVL && ((minCompCap == 70 && maxCompCap == 70) &#124;&#124; (minCompCap == 80 && maxCompCap == 80)) ? 1 : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 122 | 3 / 条件判断 | <code>(pEnable == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 123 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 124 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;bandwidths[c][a][p] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 125 | 3 / 条件判断 | <code>(c == ncclFuncAllReduce && algoEnable[a] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 126 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 127 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;bandwidths[c][a][p] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 128 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 129 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 130 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 131 | 0 / 条件判断 | <code>(comm-&gt;rank == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 132 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 133 | 2 / 准备局部状态 | <code>char line[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 134 | 2 / 调用 / 计算 / 写入 | <code>sprintf(line, "Latency/AlgBw &#124;");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 135 | 2 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 136 | 3 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 137 | 4 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), " %7s/%6s &#124;", ncclAlgoStr[a], ncclProtoStr[p]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 138 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 139 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 140 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_TUNING, "%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 141 | 2 / 调用 / 计算 / 写入 | <code>sprintf(line, " Max NThreads &#124;");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 142 | 2 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 143 | 3 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 144 | 4 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), " %14d &#124;", comm-&gt;maxThreads[a][p]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 145 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 146 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 147 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_TUNING, "%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 148 | 2 / 循环 | <code>初始 int c=0;；继续条件 c&lt;NCCL_NUM_FUNCTIONS；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 149 | 3 / 调用 / 计算 / 写入 | <code>sprintf(line, "%13s &#124;", ncclFuncStr[c]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 150 | 3 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 151 | 4 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 152 | 5 / 调用 / 计算 / 写入 | <code>sprintf(line+strlen(line), "%8.1f/%6.1f &#124;", comm-&gt;latencies[c][a][p], comm-&gt;bandwidths[c][a][p]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 153 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 154 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 155 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_TUNING, "%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 156 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 157 | 0 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 158 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;threadThresholds[a][NCCL_PROTO_LL] = NCCL_LL_THREAD_THRESHOLD;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 159 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;threadThresholds[a][NCCL_PROTO_LL128] = NCCL_LL128_THREAD_THRESHOLD;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 160 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;threadThresholds[a][NCCL_PROTO_SIMPLE] = NCCL_SIMPLE_THREAD_THRESHOLD;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 161 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 162 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL] *= nRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 163 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE] = 512;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 164 | 0 / 准备局部状态 | <code>char* str = getenv("NCCL_THREAD_THRESHOLDS");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 165 | 0 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 166 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 167 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_THREAD_THRESHOLDS set by environment to %s", str);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 168 | 2 / 准备局部状态 | <code>ssize_t t[NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS] = {{ -2, -2, -2 }, { -2, -2, -2}};</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 169 | 2 / 调用 / 计算 / 写入 | <code>sscanf(str, "%ld %ld %ld %ld %ld %ld", t[0], t[0]+1, t[0]+2, t[1], t[1]+1, t[1]+2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 170 | 2 / 循环 | <code>初始 int a=0;；继续条件 a&lt;NCCL_NUM_ALGORITHMS；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 171 | 3 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 172 | 4 / 条件判断 | <code>(t[a][p] &gt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 173 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 174 | 6 / 调用 / 计算 / 写入 | <code>comm-&gt;threadThresholds[a][p] = t[a][p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 175 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 176 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 177 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "threadThresholds %ld/%ld/%ld &#124; %ld/%ld/%ld &#124; %ld/%ld/%ld", comm-&gt;threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_LL], comm-&gt;threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_LL128], comm-&gt;threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE], comm-&gt;threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL], comm-&gt;threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL128], comm-&gt;threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_SIMPLE], comm-&gt;threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_LL], co …（完整代码见函数体）</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 178 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int simpleDefaultThreads = (ringGraph->speedIntra*ringGraph->nChannels <= PCI_WIDTH) ? 256 : NCCL_SIMPLE_MAX_NTHREADS;
  comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_SIMPLE] =
    getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_SIMPLE_MAX_NTHREADS, simpleDefaultThreads);
  comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE] =
    getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_SIMPLE_MAX_NTHREADS, NCCL_SIMPLE_MAX_NTHREADS);
  comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE] = NCCL_SIMPLE_MAX_NTHREADS;
  comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_LL] = comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_LL] = comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_LL] =
    getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_LL_MAX_NTHREADS, NCCL_LL_MAX_NTHREADS);
  comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_LL128] = comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_LL128] = comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_LL128] =
    getNthreads("NCCL_LL128_NTHREADS", ncclParamLl128Nthreads(), NCCL_LL128_MAX_NTHREADS/4, NCCL_LL128_MAX_NTHREADS, NCCL_LL128_MAX_NTHREADS);

  int nNodes = comm->nNodes;
  int nRanks = comm->nRanks;
  if (nRanks <= 1) return ncclSuccess;

  int compCap80 = minCompCap == 80 && maxCompCap == 80 ? 1 : 0;
  int cpuArch, cpuVendor, cpuModel;
  NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor, &cpuModel));
  int index2 = nNodes <= 2 ? nNodes-1 : 2;
  // LL: for single node, we look at GPU type; for multi-node, we look at CPU type
  int index1 = nNodes == 1 ? compCap80 : cpuVendor == NCCL_TOPO_CPU_VENDOR_AMD ? 1 : 0;
  double llMaxBw = llMaxBws[index1][index2];
  double perChMaxTreeBw = perChMaxTreeBws[compCap80][index2];
  // De-penalize Tree/Simple latency on Power systems to favor Tree than Ring
  if (cpuArch == NCCL_TOPO_CPU_ARCH_POWER) hwLat[NCCL_HW_PCI][NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE] = hwLat[NCCL_HW_PCI][NCCL_ALGO_RING][NCCL_PROTO_SIMPLE];
  float ppn = (float)nRanks / nNodes; // if ppn < 2, then we are sending/receiving at the same GPU through the NIC, apply some bw discount

  struct ncclTopoGraph* graphs[NCCL_NUM_ALGORITHMS] = { treeGraph, ringGraph, collNetGraph };
  int intraHw[NCCL_NUM_ALGORITHMS], hw[NCCL_NUM_ALGORITHMS];
  for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) intraHw[a] = graphs[a]->typeIntra == LINK_NVL ? NCCL_HW_NVLINK : NCCL_HW_PCI;
  for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) hw[a] = nNodes == 1 ? intraHw[a] : NCCL_HW_NET;

  for (int coll=0; coll<NCCL_NUM_FUNCTIONS; coll++) {
    int nsteps = coll == ncclFuncAllReduce ? 2*(nRanks-1) :
      coll == ncclFuncReduceScatter || coll == ncclFuncAllGather ? nRanks-1 :
      nRanks;
    int nInterSteps = coll == ncclFuncAllReduce ? (nNodes > 1 ? 2*nNodes :0) :
      coll == ncclFuncReduceScatter || coll == ncclFuncAllGather ? nNodes-1 :
      nNodes;

    for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
      if (coll != ncclFuncAllReduce && a != NCCL_ALGO_RING) continue;

      for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
        float speed = nNodes <= 2 || a == NCCL_ALGO_COLLNET ? graphs[a]->speedIntra : graphs[a]->speedInter;
        float busBw = graphs[a]->nChannels * speed;

        // Various model refinements
        if (compCap80) busBw = std::min(busBw, 235.0f);
        if (a == NCCL_ALGO_RING && p == NCCL_PROTO_LL) { busBw = std::min(llMaxBw, busBw * ((nNodes > 1 || coll == ncclFuncAllReduce || coll == ncclFuncReduce) ? 1.0/4.0 : 1.0/3.0)); }
        if (a == NCCL_ALGO_RING && p == NCCL_PROTO_LL128) busBw = std::min(busBw * (ppn < 2 ? 0.7 : 0.92 /*120.0/128.0*/), ll128MaxBwPerCh*graphs[a]->nChannels);
        if (a == NCCL_ALGO_TREE) busBw = std::min(busBw*.92, graphs[a]->nChannels*perChMaxTreeBw);
        if (a == NCCL_ALGO_TREE && p == NCCL_PROTO_LL) busBw = std::min(busBw*1.0/3.8, llMaxBw);
        if (a == NCCL_ALGO_TREE && p == NCCL_PROTO_LL128) busBw = std::min(busBw * (nNodes == 1 ? 7.0/9.0 : 120.0/128.0), ll128MaxBwPerCh*graphs[a]->nChannels);
        if (a == NCCL_ALGO_COLLNET && p != NCCL_PROTO_SIMPLE) busBw = 0;  // Oneshot CollNet only supports Simple

        // Convert bus BW to algorithm BW
        float ratio = (a != NCCL_ALGO_RING) ? .5 : (1.0 * nRanks) / nsteps;
        comm->bandwidths[coll][a][p] = busBw * ratio;

        comm->latencies[coll][a][p] = baseLat[a][p];
        float intraLat = hwLat[intraHw[a]][a][p];
        float interLat = graphs[a]->latencyInter ? graphs[a]->latencyInter : hwLat[NCCL_HW_NET][a][p];

        if (nNodes > 1 && p == NCCL_PROTO_LL) intraLat *= 1.8;
        if (a == NCCL_ALGO_RING) {
          float lat = hwLat[hw[a]][a][p];
          if ((coll == ncclFuncReduce || coll == ncclFuncBroadcast)) {
            if (ringGraph->sameChannels) {
              comm->latencies[coll][a][p] += lat;
            } else {
              if (p == NCCL_PROTO_SIMPLE) lat = hwLat[hw[a]][NCCL_ALGO_TREE][p]; // Add some chunk latency, waiting for proper chunk modeling
              comm->latencies[coll][a][p] += nsteps*lat;
            }
          } else {
            comm->latencies[coll][a][p] += (nsteps-nInterSteps)*intraLat + nInterSteps*interLat;
          }
        } else if (a == NCCL_ALGO_TREE) {
          comm->latencies[coll][a][p] +=
            2 * ((nRanks/nNodes-1) * intraLat + log2i(nNodes) * interLat);
        } else {
          comm->latencies[coll][a][p] +=
            2 * (std::min(1, (nRanks/nNodes-1)) * intraLat + (nRanks/nNodes-1) * 0.5) + interLat;  // Add 0.5 arity serialization latency
        }
      }
    }
  }

  // Protocols/Algorithms enable/disable, and user overrides.
  // All are enabled except ll128 which is enabled by default only in certain cases.
  int protoEnable[NCCL_NUM_PROTOCOLS] = { 1, 2, 1 };
  int algoEnable[NCCL_NUM_ALGORITHMS] = { 1, 1, 1 };

  const char *protoStr = getenv("NCCL_PROTO");
  if (protoStr) {
    INFO(NCCL_ENV, "NCCL_PROTO set by environment to %s", protoStr);
    NCCLCHECK(parseList(protoStr, ncclProtoStr, NCCL_NUM_PROTOCOLS, protoEnable));
  }
  const char *algoStr = getenv("NCCL_ALGO");
  if (algoStr) {
    INFO(NCCL_ENV, "NCCL_ALGO set by environment to %s", algoStr);
    NCCLCHECK(parseList(algoStr, ncclAlgoStr, NCCL_NUM_ALGORITHMS, algoEnable));
  }
  // Disable CollNet if it is not supported
  if (comm->collNetSupport == 0) {
    algoEnable[NCCL_ALGO_COLLNET] = 0;
    // If user has hard set NCCL_ALGO=COLLNET, ignore it
    if (algoEnable[NCCL_ALGO_RING] == 0 && algoEnable[NCCL_ALGO_TREE] == 0) {
      algoEnable[NCCL_ALGO_RING] = algoEnable[NCCL_ALGO_TREE] = 1;
      if (comm->rank == 0) WARN("CollNet is not supported or fails to initialize, ignoring NCCL_ALGO=COLLNET");
    }
  }

  for (int c=0; c<NCCL_NUM_FUNCTIONS; c++) for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    int pEnable = protoEnable[p];
    if (pEnable == 2 && p == NCCL_PROTO_LL128) {
      // Enable LL128 by default only on Volta/Ampere+NVLink. Other cases are not tested and may cause silent data corruption.
      pEnable = (graphs[a]->typeInter <= PATH_PXB) && graphs[a]->typeIntra <= PATH_NVL &&
        ((minCompCap == 70 && maxCompCap == 70) || (minCompCap == 80 && maxCompCap == 80)) ? 1 : 0;
    }
    if (pEnable == 0) comm->bandwidths[c][a][p] = 0;
    // Only disable algo for Allreduce since others only have one
    if (c == ncclFuncAllReduce && algoEnable[a] == 0) comm->bandwidths[c][a][p] = 0;
  }

  if (comm->rank == 0) {
    char line[1024];
    sprintf(line, "Latency/AlgBw |");
    for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
      for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
        sprintf(line+strlen(line), " %7s/%6s |", ncclAlgoStr[a], ncclProtoStr[p]);
      }
    }
    INFO(NCCL_TUNING, "%s", line);
    sprintf(line, " Max NThreads |");
    for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
      for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
        sprintf(line+strlen(line), " %14d |", comm->maxThreads[a][p]);
      }
    }
    INFO(NCCL_TUNING, "%s", line);
    for (int c=0; c<NCCL_NUM_FUNCTIONS; c++) {
      sprintf(line, "%13s |", ncclFuncStr[c]);
      for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
        for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
          sprintf(line+strlen(line), "%8.1f/%6.1f |", comm->latencies[c][a][p], comm->bandwidths[c][a][p]);
        }
      }
      INFO(NCCL_TUNING, "%s", line);
    }
  }

  // Set per-thread amount of work before we increase nThreads and nChannels
  for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
    comm->threadThresholds[a][NCCL_PROTO_LL] = NCCL_LL_THREAD_THRESHOLD;
    comm->threadThresholds[a][NCCL_PROTO_LL128] = NCCL_LL128_THREAD_THRESHOLD;
    comm->threadThresholds[a][NCCL_PROTO_SIMPLE] = NCCL_SIMPLE_THREAD_THRESHOLD;
  }
  comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL] *= nRanks;
  comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE] = 512;

  // Override defaults with user env
  char* str = getenv("NCCL_THREAD_THRESHOLDS");
  if (str) {
    INFO(NCCL_ENV, "NCCL_THREAD_THRESHOLDS set by environment to %s", str);
    ssize_t t[NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS] = {{ -2, -2, -2 }, { -2, -2, -2}};
    sscanf(str, "%ld %ld %ld %ld %ld %ld", t[0], t[0]+1, t[0]+2, t[1], t[1]+1, t[1]+2);
    for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
      for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
        if (t[a][p] >= 0) comm->threadThresholds[a][p] = t[a][p];
      }
    }
  }

  INFO(NCCL_INIT, "threadThresholds %ld/%ld/%ld | %ld/%ld/%ld | %ld/%ld/%ld",
      comm->threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_LL],
      comm->threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_LL128],
      comm->threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE],
      comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL],
      comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL128],
      comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_SIMPLE],
      comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_LL],
      comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_LL128],
      comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE]);
  return ncclSuccess;
}
```

</details>

<a id="f0531"></a>

## F0531：ncclTopoGetAlgoTime

用途：用延迟项加字节数/带宽项预测当前候选耗时，加入 Tree/Ring 与聚合修正供选型。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>float bw = info-&gt;comm-&gt;bandwidths[info-&gt;coll][algorithm][protocol];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>float lat = info-&gt;comm-&gt;latencies[info-&gt;coll][algorithm][protocol];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(bw == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>*time = -1.0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>int logSize = log2i(info-&gt;nBytes&gt;&gt;6);</code> | 用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。 |
| 8 | 0 / 条件判断 | <code>(algorithm == NCCL_ALGO_TREE && logSize &lt; 23)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>bw *= treeCorrectionFactor[protocol][logSize];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 条件判断 | <code>(info-&gt;nChannels != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>bw = bw / info-&gt;comm-&gt;nChannels * info-&gt;nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 条件判断 | <code>(algorithm == NCCL_ALGO_RING && protocol == NCCL_PROTO_SIMPLE && info-&gt;comm-&gt;nNodes &gt; 1 && info-&gt;coll == ncclFuncAllReduce && info-&gt;nBytes &gt;= info-&gt;comm-&gt;nRanks/16.0*65536)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>lat *= 1.9;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 准备局部状态 | <code>int latCount = algorithm == NCCL_ALGO_RING ? numPipeOps : DIVUP(numPipeOps, NCCL_MAX_WORK_ELEMENTS);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>*time = lat * latCount + (info-&gt;nBytes) / (1000 * bw);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  float bw = info->comm->bandwidths[info->coll][algorithm][protocol];
  float lat = info->comm->latencies[info->coll][algorithm][protocol];
  if (bw == 0) {
    *time = -1.0; return ncclSuccess;
  }
  int logSize = log2i(info->nBytes>>6);
  if (algorithm == NCCL_ALGO_TREE && logSize < 23) bw *= treeCorrectionFactor[protocol][logSize];
  if (info->nChannels != 0) bw = bw / info->comm->nChannels * info->nChannels;
  if (algorithm == NCCL_ALGO_RING && protocol == NCCL_PROTO_SIMPLE && info->comm->nNodes > 1
      && info->coll == ncclFuncAllReduce && info->nBytes >= info->comm->nRanks/16.0*65536) lat *= 1.9; // Plateau effect of ring
  // Tree pipelining saves latency in aggregation cases
  int latCount = algorithm == NCCL_ALGO_RING ? numPipeOps : DIVUP(numPipeOps, NCCL_MAX_WORK_ELEMENTS);
  *time = lat * latCount + (info->nBytes) / (1000 * bw);
  return ncclSuccess;
}
```

</details>

