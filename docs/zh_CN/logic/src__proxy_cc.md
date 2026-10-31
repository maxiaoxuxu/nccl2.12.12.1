# src/proxy.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1014"></a>

## F1014：NeedProxy

用途：按通信模式、root 与环邻居判断此方向是否真的需要代理，root 流水线端点可省掉无效边。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(pattern == ncclPatternRing &#124;&#124; pattern == ncclPatternRingTwice)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return true;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>const int myrank = 0, nextrank = 1, prevrank = nranks-1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>int index = pattern == ncclPatternPipelineFrom ? (type == proxyRecv ? myrank : nextrank ): (type == proxyRecv ? prevrank : myrank );</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int rank = ring-&gt;userRanks[index];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 函数返回 | <code>return (root != rank);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (pattern == ncclPatternRing || pattern == ncclPatternRingTwice) return true;

  /* In chains, one rank does not need a proxy. Let's figure out which one it is */
  /* Which index in the reorganized rings should we compare root against */
  const int myrank = 0, nextrank = 1, prevrank = nranks-1;
  int index = pattern == ncclPatternPipelineFrom ?
      /*                            no recv /  no send    if root = */
      /* bcast  */ (type == proxyRecv ?   myrank : nextrank ):
      /* reduce */ (type == proxyRecv ? prevrank :   myrank );
  int rank = ring->userRanks[index];
  return (root != rank);
}
```

</details>

<a id="f1015"></a>

## F1015：allocateArgs

用途：从可复用参数池分配运行态 ProxyArgs，容量不足时扩展池块而不逐请求零散分配。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyArgs* elem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;pool == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 准备局部状态 | <code>struct ncclProxyPool* newPool;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&newPool, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 6 | 2 / 准备局部状态 | <code>struct ncclProxyArgs* newElems = newPool-&gt;elems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;PROXYARGS_ALLOCATE_SIZE；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 3 / 条件判断 | <code>(i+1 &lt; PROXYARGS_ALLOCATE_SIZE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 5 / 调用 / 计算 / 写入 | <code>newElems[i].next = newElems+i+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>state-&gt;pool = newElems;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>newPool-&gt;next = state-&gt;pools;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>state-&gt;pools = newPool;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>elem = state-&gt;pool;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;pool = state-&gt;pool-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;next = elem-&gt;nextPeer = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>*argsptr = elem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyArgs* elem;
  if (state->pool == NULL) {
    // Allocate a new pool of elements. Make sure we allocate the memory close
    // to the network thread
    struct ncclProxyPool* newPool;
    NCCLCHECK(ncclCalloc(&newPool, 1));

    struct ncclProxyArgs* newElems = newPool->elems;
    // Chain newly allocated elements
    for (int i=0; i<PROXYARGS_ALLOCATE_SIZE; i++) {
      if (i+1 < PROXYARGS_ALLOCATE_SIZE) newElems[i].next = newElems+i+1;
    }
    // Add them all to the pool list
    state->pool = newElems;
    // Save the pool memory block for later resource release
    newPool->next = state->pools;
    state->pools = newPool;
  }
  elem = state->pool;
  state->pool = state->pool->next;
  elem->next = elem->nextPeer = NULL;
  *argsptr = elem;
  return ncclSuccess;
}
```

</details>

<a id="f1016"></a>

## F1016：getOpIndex

用途：计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyPool* pool = state-&gt;pools;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int p = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 ；继续条件 (pool)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 准备局部状态 | <code>uint64_t o = op-pool-&gt;elems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 1 / 条件判断 | <code>(o &lt; PROXYARGS_ALLOCATE_SIZE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>*opIndex = o;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>*poolIndex = p;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>pool = pool-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>p++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>WARN("Could not find pool of op %p\n", op);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyPool* pool = state->pools;
  int p = 0;
  while (pool) {
    uint64_t o = op-pool->elems;
    if (o < PROXYARGS_ALLOCATE_SIZE) {
      *opIndex = o;
      *poolIndex = p;
      return ncclSuccess;
    }
    pool = pool->next;
    p++;
  }
  WARN("Could not find pool of op %p\n", op);
  return ncclInternalError;
}
```

</details>

<a id="f1017"></a>

## F1017：printProxyOp

用途：打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>printf("[%d-%d&#124;%ld&#124; %s", poolIndex, opIndex, op-&gt;opCount, op-&gt;pattern == ncclPatternSend ? "Send" : op-&gt;pattern == ncclPatternRecv ? "Recv" : "Coll");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;op-&gt;nsubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = op-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 条件判断 | <code>(op-&gt;state == ncclProxyOpProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 准备局部状态 | <code>char status = ' ';</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 3 / 条件判断 | <code>(op-&gt;pattern == ncclPatternRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 5 / 条件判断 | <code>(sub-&gt;posted &lt; sub-&gt;nsteps && sub-&gt;posted &lt; sub-&gt;done + NCCL_STEPS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 7 / 调用 / 计算 / 写入 | <code>status = 'I';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 7 / 条件判断 | <code>(sub-&gt;received &lt; sub-&gt;posted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 9 / 调用 / 计算 / 写入 | <code>status = 'R';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 17 | 9 / 条件判断 | <code>(sub-&gt;received &lt; sub-&gt;transmitted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 11 / 调用 / 计算 / 写入 | <code>status = 'R';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 10 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 21 | 11 / 条件判断 | <code>(sub-&gt;transmitted &lt; sub-&gt;received)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 12 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 13 / 调用 / 计算 / 写入 | <code>status = 'F';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 12 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 25 | 13 / 条件判断 | <code>(sub-&gt;done &lt; sub-&gt;transmitted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 14 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 15 / 调用 / 计算 / 写入 | <code>status = 'G';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 14 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 29 | 15 / 调用 / 计算 / 写入 | <code>status = 'D';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 31 | 5 / 条件判断 | <code>(op-&gt;pattern == ncclPatternSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 7 / 条件判断 | <code>(sub-&gt;posted &lt; sub-&gt;nsteps && sub-&gt;posted &lt; sub-&gt;done + NCCL_STEPS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 9 / 调用 / 计算 / 写入 | <code>status = 'I';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 37 | 9 / 条件判断 | <code>(sub-&gt;transmitted &lt; sub-&gt;posted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 11 / 调用 / 计算 / 写入 | <code>status = 'G';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 10 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 41 | 11 / 条件判断 | <code>(sub-&gt;done &lt; sub-&gt;transmitted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 12 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 13 / 调用 / 计算 / 写入 | <code>status = 'S';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 12 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 45 | 13 / 调用 / 计算 / 写入 | <code>status = 'D';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 3 / 调用 / 计算 / 写入 | <code>printf(" %d%c/%d", sub-&gt;peer, status, sub-&gt;channelId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 48 | 3 / 调用 / 计算 / 写入 | <code>printf(" %d/%d", sub-&gt;peer, sub-&gt;channelId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 50 | 0 / 调用 / 计算 / 写入 | <code>printf("]");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 51 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  printf("[%d-%d|%ld| %s", poolIndex, opIndex, op->opCount, op->pattern == ncclPatternSend ? "Send" : op->pattern == ncclPatternRecv ? "Recv" : "Coll");
  for (int s=0; s<op->nsubs; s++) {
    struct ncclProxySubArgs* sub = op->subs+s;
    if (op->state == ncclProxyOpProgress) {
      char status = ' ';
      if (op->pattern == ncclPatternRecv) {
        if (sub->posted < sub->nsteps && sub->posted < sub->done + NCCL_STEPS) status = 'I'; // Init
        else if (sub->received < sub->posted) status = 'R'; // Receiving
        else if (sub->received < sub->transmitted) status = 'R'; // Receiving
        else if (sub->transmitted < sub->received) status = 'F'; // Flushing
        else if (sub->done < sub->transmitted) status = 'G'; // Waiting on GPU
        else status = 'D'; // Done
      } else if (op->pattern == ncclPatternSend) {
        if (sub->posted < sub->nsteps && sub->posted < sub->done + NCCL_STEPS) status = 'I'; // Init
        else if (sub->transmitted < sub->posted) status = 'G'; // Waiting on GPU
        else if (sub->done < sub->transmitted) status = 'S'; // Sending
        else status = 'D'; // Done
      }
      printf(" %d%c/%d", sub->peer, status, sub->channelId);
    } else {
      printf(" %d/%d", sub->peer, sub->channelId);
    }
  }
  printf("]");
  return ncclSuccess;
}
```

</details>

<a id="f1018"></a>

## F1018：dumpProxyState

用途：打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyArgs* op = state-&gt;active;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int poolIndex, opIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>printf("ACTIVE OPS\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 循环 | <code>初始 ；继续条件 (op)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));</code> | 失败时可能提前return，成功才继续；计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。 |
| 6 | 1 / 条件判断 | <code>(op-&gt;state & OP_SEEN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>WARN("List loop at element %d-%d", poolIndex, opIndex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(printProxyOp(op, poolIndex, opIndex));</code> | 失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>op-&gt;state &#124;= OP_SEEN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>printf("\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 1 / 准备局部状态 | <code>struct ncclProxyArgs* nextOp = op-&gt;nextPeer;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 循环 | <code>初始 ；继续条件 (nextOp)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getOpIndex(nextOp, state, &poolIndex, &opIndex));</code> | 失败时可能提前return，成功才继续；计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。 |
| 15 | 2 / 条件判断 | <code>(nextOp-&gt;state & OP_SEEN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>WARN("List loop at element %d-%d", poolIndex, opIndex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>printf("&#124; &#96;-&gt; ");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(printProxyOp(nextOp, poolIndex, opIndex));</code> | 失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>nextOp-&gt;state &#124;= OP_SEEN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>printf("\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 2 / 条件判断 | <code>(nextOp-&gt;next)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>WARN("Inactive op has next set!\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>nextOp = nextOp-&gt;nextPeer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 1 / 条件判断 | <code>(op-&gt;nextPeer == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 3 / 调用 / 计算 / 写入 | <code>printf("&#124;\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 1 / 调用 / 计算 / 写入 | <code>op = op-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 1 / 调用 / 计算 / 写入 | <code>printf("v\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>printf("[X]\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 34 | 0 / 编译期分支/指令 | <code># if 0</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 35 | 1 / 调用 / 计算 / 写入 | <code>printf("FREE OPS\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 1 / 调用 / 计算 / 写入 | <code>op = state-&gt;pool;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 1 / 循环 | <code>初始 ；继续条件 (op)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 38 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));</code> | 失败时可能提前return，成功才继续；计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。 |
| 39 | 2 / 条件判断 | <code>(op-&gt;state & OP_SEEN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 4 / 调用 / 计算 / 写入 | <code>WARN("List loop at element %d-%d", poolIndex, opIndex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(printProxyOp(op, poolIndex, opIndex));</code> | 失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。 |
| 43 | 2 / 调用 / 计算 / 写入 | <code>op-&gt;state &#124;= OP_SEEN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>printf("-&gt;");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 2 / 调用 / 计算 / 写入 | <code>op = op-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 1 / 调用 / 计算 / 写入 | <code>printf("[X]\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 48 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 49 | 2 / 调用 / 计算 / 写入 | <code>op = state-&gt;pool;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 2 / 循环 | <code>初始 ；继续条件 (op)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 51 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));</code> | 失败时可能提前return，成功才继续；计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。 |
| 52 | 3 / 条件判断 | <code>(op-&gt;state & OP_SEEN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 5 / 调用 / 计算 / 写入 | <code>WARN("List loop at element %d-%d", poolIndex, opIndex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 55 | 3 / 调用 / 计算 / 写入 | <code>op-&gt;state &#124;= OP_SEEN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 3 / 调用 / 计算 / 写入 | <code>op = op-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 58 | 0 / 准备局部状态 | <code>struct ncclProxyPool* pool = state-&gt;pools;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 0 / 调用 / 计算 / 写入 | <code>poolIndex = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 0 / 循环 | <code>初始 ；继续条件 (pool)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 61 | 1 / 准备局部状态 | <code>struct ncclProxyArgs* elem = pool-&gt;elems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 62 | 1 / 循环 | <code>初始 int e=0;；继续条件 e&lt;PROXYARGS_ALLOCATE_SIZE；每轮更新 e++, elem++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 63 | 2 / 条件判断 | <code>((elem-&gt;state & OP_SEEN) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 4 / 调用 / 计算 / 写入 | <code>printf("Elem %d-%d is not in any list:\n", poolIndex, e);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 66 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(printProxyOp(elem, poolIndex, e));</code> | 失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。 |
| 67 | 4 / 调用 / 计算 / 写入 | <code>printf("\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 68 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 69 | 4 / 调用 / 计算 / 写入 | <code>elem-&gt;state -= OP_SEEN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 71 | 1 / 调用 / 计算 / 写入 | <code>pool = pool-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 1 / 调用 / 计算 / 写入 | <code>poolIndex++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 74 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyArgs* op = state->active;
  int poolIndex, opIndex;
  printf("ACTIVE OPS\n");
  while (op) {
    NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));
    if (op->state & OP_SEEN) {
      WARN("List loop at element %d-%d", poolIndex, opIndex);
    }
    NCCLCHECK(printProxyOp(op, poolIndex, opIndex));
    op->state |= OP_SEEN;
    printf("\n");
    struct ncclProxyArgs* nextOp = op->nextPeer;
    while (nextOp) {
      NCCLCHECK(getOpIndex(nextOp, state, &poolIndex, &opIndex));
      if (nextOp->state & OP_SEEN) {
        WARN("List loop at element %d-%d", poolIndex, opIndex);
      }
      printf("| `-> ");
      NCCLCHECK(printProxyOp(nextOp, poolIndex, opIndex));
      nextOp->state |= OP_SEEN;
      printf("\n");
      if (nextOp->next) {
        WARN("Inactive op has next set!\n");
      }
      nextOp = nextOp->nextPeer;
    }
    if (op->nextPeer == NULL) printf("|\n");
    op = op->next;
    printf("v\n");
  }
  printf("[X]\n");

# if 0
  printf("FREE OPS\n");
  op = state->pool;
  while (op) {
    NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));
    if (op->state & OP_SEEN) {
      WARN("List loop at element %d-%d", poolIndex, opIndex);
    }
    NCCLCHECK(printProxyOp(op, poolIndex, opIndex));
    op->state |= OP_SEEN;
    printf("->");
    op = op->next;
  }
  printf("[X]\n");
#else
  op = state->pool;
  while (op) {
    NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));
    if (op->state & OP_SEEN) {
      WARN("List loop at element %d-%d", poolIndex, opIndex);
    }
    op->state |= OP_SEEN;
    op = op->next;
  }
#endif

  struct ncclProxyPool* pool = state->pools;
  poolIndex = 0;
  while (pool) {
    struct ncclProxyArgs* elem = pool->elems;
    for (int e=0; e<PROXYARGS_ALLOCATE_SIZE; e++, elem++) {
      if ((elem->state & OP_SEEN) == 0) {
        printf("Elem %d-%d is not in any list:\n", poolIndex, e);
        NCCLCHECK(printProxyOp(elem, poolIndex, e));
        printf("\n");
      } else {
        elem->state -= OP_SEEN;
      }
    }
    pool = pool->next;
    poolIndex++;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1019"></a>

## F1019：ncclProxyOpToArgs

用途：把紧凑提交描述展开到运行态 args/sub，保留协议、步数、连接和操作编号。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = args-&gt;subs+subIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(subIndex &gt;= NCCL_PROXY_MAX_SUBS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("Proxy append out of bounds");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>sub-&gt;connection = op-&gt;connection;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>sub-&gt;channelId = op-&gt;channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>sub-&gt;nsteps = op-&gt;nsteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>sub-&gt;nbytes = op-&gt;nbytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>sub-&gt;peer = op-&gt;root;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;nsubs = subIndex+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 条件判断 | <code>(subIndex)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 条件判断 | <code>((args-&gt;sliceSteps != op-&gt;sliceSteps) &#124;&#124; (args-&gt;chunkSteps != op-&gt;chunkSteps) &#124;&#124; (args-&gt;protocol != op-&gt;protocol) &#124;&#124; (args-&gt;dtype != op-&gt;dtype) &#124;&#124; (args-&gt;redOp != op-&gt;redOp))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>WARN("Proxy append mismatch");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 2 / 条件判断 | <code>(args-&gt;state != ncclProxyOpReady)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>WARN("Proxy append on running operation");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;opCount = op-&gt;opCount;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;sliceSteps = op-&gt;sliceSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;chunkSteps = op-&gt;chunkSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;chunkSize = op-&gt;chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;dtype = op-&gt;dtype;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;redOp = op-&gt;redOp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;pattern = op-&gt;pattern;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;protocol = op-&gt;protocol;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpReady;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;progress = op-&gt;connection-&gt;tcomm-&gt;proxyProgress;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;proxyAppendPtr = op-&gt;connection-&gt;proxyAppendPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxySubArgs* sub = args->subs+subIndex;
  if (subIndex >= NCCL_PROXY_MAX_SUBS) {
    WARN("Proxy append out of bounds");
    return ncclInternalError;
  }

  //memset(sub, 0, sizeof(struct ncclProxySubArgs));
  sub->connection = op->connection;
  sub->channelId = op->channelId;
  sub->nsteps = op->nsteps;
  sub->nbytes = op->nbytes;
  sub->peer = op->root;
  args->nsubs = subIndex+1;
  if (subIndex) {
    if ((args->sliceSteps != op->sliceSteps) ||
        (args->chunkSteps != op->chunkSteps) ||
        (args->protocol != op->protocol) ||
        (args->dtype != op->dtype) ||
        (args->redOp != op->redOp)) {
      WARN("Proxy append mismatch");
      return ncclInternalError;
    }
    if (args->state != ncclProxyOpReady) {
      WARN("Proxy append on running operation");
      return ncclInternalError;
    }
    return ncclSuccess;
  }
  //memset(&args->progress, 0, sizeof(struct ncclProxyArgs)-offsetof(struct ncclProxyArgs, progress));
  args->done = 0;
  args->opCount = op->opCount;
  args->sliceSteps = op->sliceSteps;
  args->chunkSteps = op->chunkSteps;
  args->chunkSize = op->chunkSize;
  args->dtype = op->dtype;
  args->redOp = op->redOp;
  args->pattern = op->pattern;
  args->protocol = op->protocol;
  args->state = ncclProxyOpReady;
  args->progress = op->connection->tcomm->proxyProgress;
  args->proxyAppendPtr = op->connection->proxyAppendPtr;
  return ncclSuccess;
}
```

</details>

<a id="f1020"></a>

## F1020：ProxyAppend

用途：把提交操作追加/聚合到活跃链或同连接后继链，维持共享连接上的先后顺序。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyConnection* connection = op-&gt;connection;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int shared = connection-&gt;shared;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclProxyArgs* args = *connection-&gt;proxyAppendPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(args)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 条件判断 | <code>(shared && args-&gt;opCount == op-&gt;opCount)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyOpToArgs(op, args, args-&gt;nsubs));</code> | 失败时可能提前return，成功才继续；把紧凑提交描述展开到运行态 args/sub，保留协议、步数、连接和操作编号。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>DEBUG_PROXY_PRINT("Insert (%d/%5ld/%5ld) as group with %5ld\n", shared, args-&gt;opCount, op-&gt;opCount, OP_INDEX(args));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 4 / 准备局部状态 | <code>struct ncclProxyArgs* prevArgs = args;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(allocateArgs(state, &args));</code> | 失败时可能提前return，成功才继续；从可复用参数池分配运行态 ProxyArgs，容量不足时扩展池块而不逐请求零散分配。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyOpToArgs(op, args, 0));</code> | 失败时可能提前return，成功才继续；把紧凑提交描述展开到运行态 args/sub，保留协议、步数、连接和操作编号。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>prevArgs-&gt;nextPeer = args;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>DEBUG_PROXY_PRINT("Insert %5ld (%d/%5ld/%5ld) as nextPeer of %5ld\n", OP_INDEX(args), shared, prevArgs-&gt;opCount, args-&gt;opCount, OP_INDEX(prevArgs));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>*(args-&gt;proxyAppendPtr) = args;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(allocateArgs(state, &args));</code> | 失败时可能提前return，成功才继续；从可复用参数池分配运行态 ProxyArgs，容量不足时扩展池块而不逐请求零散分配。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyOpToArgs(op, args, 0));</code> | 失败时可能提前return，成功才继续；把紧凑提交描述展开到运行态 args/sub，保留协议、步数、连接和操作编号。 |
| 20 | 2 / 条件判断 | <code>(state-&gt;active == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 4 / 调用 / 计算 / 写入 | <code>DEBUG_PROXY_PRINT("Insert %5ld (%d/%5ld) as first element\n", OP_INDEX(args), shared, args-&gt;opCount);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>state-&gt;active = args;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 25 | 4 / 准备局部状态 | <code>struct ncclProxyArgs* last = state-&gt;active;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 4 / 循环 | <code>初始 ；继续条件 (last-&gt;next)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 5 / 调用 / 计算 / 写入 | <code>last = last-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>last-&gt;next = args;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 4 / 调用 / 计算 / 写入 | <code>DEBUG_PROXY_PRINT("Insert %5ld (%d/%5ld) as last element\n", OP_INDEX(args), shared, args-&gt;opCount);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 2 / 调用 / 计算 / 写入 | <code>*(args-&gt;proxyAppendPtr) = args;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyConnection* connection = op->connection;
  int shared = connection->shared;
  struct ncclProxyArgs* args = *connection->proxyAppendPtr;

  if (args) {
    if (shared && args->opCount == op->opCount) {
      NCCLCHECK(ncclProxyOpToArgs(op, args, args->nsubs));
      DEBUG_PROXY_PRINT("Insert (%d/%5ld/%5ld) as group with %5ld\n", shared, args->opCount, op->opCount, OP_INDEX(args));
    } else {
      struct ncclProxyArgs* prevArgs = args;
      NCCLCHECK(allocateArgs(state, &args));
      NCCLCHECK(ncclProxyOpToArgs(op, args, 0));
      prevArgs->nextPeer = args;
      DEBUG_PROXY_PRINT("Insert  %5ld (%d/%5ld/%5ld) as nextPeer of %5ld\n", OP_INDEX(args), shared, prevArgs->opCount, args->opCount, OP_INDEX(prevArgs));
      *(args->proxyAppendPtr) = args;
    }
  } else {
    // Nothing running for that peer. Add to the list
    NCCLCHECK(allocateArgs(state, &args));
    NCCLCHECK(ncclProxyOpToArgs(op, args, 0));
    if (state->active == NULL) {
      // Create the list
      DEBUG_PROXY_PRINT("Insert  %5ld (%d/%5ld) as first element\n", OP_INDEX(args), shared, args->opCount);
      state->active = args;
    } else {
      // Append element at the end of the list
      struct ncclProxyArgs* last = state->active;
      while (last->next) last = last->next;
      last->next = args;
      DEBUG_PROXY_PRINT("Insert  %5ld (%d/%5ld) as last element\n", OP_INDEX(args), shared, args->opCount);
    }
    *(args->proxyAppendPtr) = args;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1021"></a>

## F1021：ncclProxyPost

用途：在锁内发布任务索引链，队列从空转为非空时通过条件变量唤醒 Progress 线程。

- 加锁后追加任务索引链，只有由空转为非空时需要signal唤醒等待线程。
- 锁保护的是CPU任务链元数据；真实数据仍按head/tail/flag协议与GPU协作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&pool-&gt;mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 条件判断 | <code>(pool-&gt;nextOps == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>pool-&gt;nextOps = nextOps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>pthread_cond_signal(&pool-&gt;cond);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>pool-&gt;ops[pool-&gt;nextOpsEnd].next = nextOps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>pool-&gt;nextOpsEnd = nextOpsEnd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&pool-&gt;mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  pthread_mutex_lock(&pool->mutex);
  if (pool->nextOps == -1) {
    pool->nextOps = nextOps;
    pthread_cond_signal(&pool->cond);
  } else {
    pool->ops[pool->nextOpsEnd].next = nextOps;
  }
  pool->nextOpsEnd = nextOpsEnd;
  pthread_mutex_unlock(&pool->mutex);
  return ncclSuccess;
}
```

</details>

<a id="f1022"></a>

## F1022：ncclLocalOpAppend

用途：从本地代理共享池取空闲项复制 ProxyOp，追加待发布链并在容量紧张时发布完整批次。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyOps* proxyOps = proxyConn-&gt;comm-&gt;proxyState.proxyOps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(proxyOps == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>proxyOps += proxyConn-&gt;localRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclProxyOpsPool* pool = proxyOps-&gt;pool;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>TIME_START(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 准备局部状态 | <code>int opIndex = proxyOps-&gt;freeOp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>struct ncclProxyOp* op;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 条件判断 | <code>(opIndex != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>op = pool-&gt;ops+opIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>proxyOps-&gt;freeOp = op-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 2 / 准备局部状态 | <code>int freeOp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 2 / 循环 | <code>初始 ；继续条件 ((freeOp = pool-&gt;freeOps[comm-&gt;localRank]) == -1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>sched_yield();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 2 / 准备局部状态 | <code>int freeOpNew;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 2 / 循环 | <code>初始 ；继续条件 ((freeOpNew = __sync_val_compare_and_swap(pool-&gt;freeOps+comm-&gt;localRank, freeOp, -1)) != freeOp)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>freeOp = freeOpNew;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>opIndex = freeOp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>op = pool-&gt;ops+opIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>proxyOps-&gt;freeOp = op-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 条件判断 | <code>(op-&gt;next != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>__builtin_prefetch(pool-&gt;ops+op-&gt;next);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>memcpy(op, proxyOp, sizeof(struct ncclProxyOp));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;next = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;connection = proxyConn-&gt;connection;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 条件判断 | <code>(proxyOps-&gt;nextOps == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>proxyOps-&gt;nextOps = proxyOps-&gt;nextOpsEnd = opIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>pool-&gt;ops[proxyOps-&gt;nextOpsEnd].next = opIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>proxyOps-&gt;nextOpsEnd = opIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 0 / 条件判断 | <code>(++proxyOps-&gt;count == MAX_OPS_PER_PEER)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 2 / 准备局部状态 | <code>uint64_t lastOpCount = pool-&gt;ops[proxyOps-&gt;nextOpsEnd].opCount;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 2 / 准备局部状态 | <code>int lastOp = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 2 / 准备局部状态 | <code>int toSend = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 2 / 准备局部状态 | <code>int ops = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 2 / 循环 | <code>初始 int op= proxyOps-&gt;nextOps;；继续条件 op != proxyOps-&gt;nextOpsEnd；每轮更新 op=pool-&gt;ops[op].next</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 45 | 3 / 调用 / 计算 / 写入 | <code>ops++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 3 / 条件判断 | <code>(pool-&gt;ops[op].opCount != lastOpCount)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 5 / 调用 / 计算 / 写入 | <code>lastOp = op;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 5 / 调用 / 计算 / 写入 | <code>toSend = ops;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 51 | 2 / 条件判断 | <code>(lastOp == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 52 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 53 | 4 / 调用 / 计算 / 写入 | <code>WARN("Unable to post incomplete proxy op chain %d..%d (opCount %ld)\n", proxyOps-&gt;nextOps, proxyOps-&gt;nextOpsEnd, lastOpCount);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 54 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 55 | 2 / 准备局部状态 | <code>int nextOps = proxyOps-&gt;nextOps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 2 / 调用 / 计算 / 写入 | <code>proxyOps-&gt;nextOps = pool-&gt;ops[lastOp].next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 2 / 调用 / 计算 / 写入 | <code>pool-&gt;ops[lastOp].next = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyPost(proxyOps-&gt;pool, nextOps, lastOp));</code> | 失败时可能提前return，成功才继续；在锁内发布任务索引链，队列从空转为非空时通过条件变量唤醒 Progress 线程。 |
| 59 | 2 / 调用 / 计算 / 写入 | <code>proxyOps-&gt;count -= toSend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 0 / 调用 / 计算 / 写入 | <code>TIME_STOP(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 61 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyOps* proxyOps = proxyConn->comm->proxyState.proxyOps;
  if (proxyOps == NULL) return ncclInternalError;
  proxyOps += proxyConn->localRank;
  struct ncclProxyOpsPool* pool = proxyOps->pool;

  TIME_START(0);
  int opIndex = proxyOps->freeOp;
  struct ncclProxyOp* op;
  if (opIndex != -1) {
    op = pool->ops+opIndex;
    proxyOps->freeOp = op->next;
  } else {
    int freeOp;
    while ((freeOp = pool->freeOps[comm->localRank]) == -1) sched_yield();
    int freeOpNew;
    while ((freeOpNew = __sync_val_compare_and_swap(pool->freeOps+comm->localRank, freeOp, -1)) != freeOp) freeOp = freeOpNew;
    opIndex = freeOp;
    op = pool->ops+opIndex;
    proxyOps->freeOp = op->next;
  }
  if (op->next != -1) __builtin_prefetch(pool->ops+op->next); // Prefetch next free op
  memcpy(op, proxyOp, sizeof(struct ncclProxyOp));
  op->next = -1;
  op->connection = proxyConn->connection;
  if (proxyOps->nextOps == -1) {
    proxyOps->nextOps = proxyOps->nextOpsEnd = opIndex;
  } else {
    pool->ops[proxyOps->nextOpsEnd].next = opIndex;
    proxyOps->nextOpsEnd = opIndex;
  }
  if (++proxyOps->count == MAX_OPS_PER_PEER) {
    // Post what we have so far to free some ops in the pool
    // Do not post last operations as we could have more coming with the same opCount, and posting
    // them in different batches would break proxyArgs aggregation with subs.
    uint64_t lastOpCount = pool->ops[proxyOps->nextOpsEnd].opCount;
    int lastOp = -1;
    int toSend = 0;
    int ops = 0;
    for (int op= proxyOps->nextOps; op != proxyOps->nextOpsEnd; op=pool->ops[op].next) {
      ops++;
      if (pool->ops[op].opCount != lastOpCount) {
        lastOp = op;
        toSend = ops;
      }
    }
    if (lastOp == -1) {
      WARN("Unable to post incomplete proxy op chain %d..%d (opCount %ld)\n", proxyOps->nextOps, proxyOps->nextOpsEnd, lastOpCount);
      return ncclInternalError;
    }
    // Cut chain at lastOp
    int nextOps = proxyOps->nextOps;
    proxyOps->nextOps = pool->ops[lastOp].next;
    pool->ops[lastOp].next = -1;
    NCCLCHECK(ncclProxyPost(proxyOps->pool, nextOps, lastOp));
    proxyOps->count -= toSend;
  }
  TIME_STOP(0);
  return ncclSuccess;
}
```

</details>

<a id="f1023"></a>

## F1023：SaveProxy

用途：找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(peer &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclPeer* peerComm = channel-&gt;peers+peer;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclConnector* connector = type == proxyRecv ? peerComm-&gt;recv+connIndex : peerComm-&gt;send+connIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(connector-&gt;transportComm == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>WARN("Rank %d has no transport for %s peer %d on channel %d/%d", connector-&gt;comm-&gt;rank, type == proxyRecv ? "recv" : "send", peer, channel-&gt;id, connIndex);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 条件判断 | <code>(connector-&gt;transportComm-&gt;proxyProgress == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclLocalOpAppend(connector-&gt;comm, &connector-&gt;proxyConn, op));</code> | 失败时可能提前return，成功才继续；从本地代理共享池取空闲项复制 ProxyOp，追加待发布链并在容量紧张时发布完整批次。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (peer < 0) return ncclSuccess;

  struct ncclPeer* peerComm = channel->peers+peer;
  struct ncclConnector* connector = type == proxyRecv ? peerComm->recv+connIndex : peerComm->send+connIndex;
  if (connector->transportComm == NULL) {
    WARN("Rank %d has no transport for %s peer %d on channel %d/%d", connector->comm->rank,
        type == proxyRecv ? "recv" : "send", peer, channel->id, connIndex);
    return ncclInternalError;
  }
  if (connector->transportComm->proxyProgress == NULL) return ncclSuccess;

  NCCLCHECK(ncclLocalOpAppend(connector->comm, &connector->proxyConn, op));
  return ncclSuccess;
}
```

</details>

<a id="f1024"></a>

## F1024：ncclProxySaveColl

用途：按 Ring/Tree/Pipeline/CollNet 模式展开必要邻居与方向，给相应连接保存代理工作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+op-&gt;channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int pattern = op-&gt;pattern;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(pattern == ncclPatternRing &#124;&#124; pattern == ncclPatternRingTwice &#124;&#124; pattern == ncclPatternPipelineFrom &#124;&#124; pattern == ncclPatternPipelineTo)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 准备局部状态 | <code>struct ncclRing* ring = &channel-&gt;ring;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 条件判断 | <code>(NeedProxy(proxyRecv, pattern, op-&gt;root, ring, nranks))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxyRecv, ring-&gt;prev, op, 0));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 9 | 2 / 条件判断 | <code>(NeedProxy(proxySend, pattern, op-&gt;root, ring, nranks))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxySend, ring-&gt;next, op, 0));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 12 | 0 / 条件判断 | <code>(pattern == ncclPatternTreeUp &#124;&#124; pattern == ncclPatternTreeUpDown)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 准备局部状态 | <code>struct ncclTree* tree = &channel-&gt;tree;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_MAX_TREE_ARITY；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxyRecv, tree-&gt;down[i], op, 0));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 17 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxySend, tree-&gt;up, op, 0));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 19 | 0 / 条件判断 | <code>(pattern == ncclPatternTreeDown &#124;&#124; pattern == ncclPatternTreeUpDown)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 准备局部状态 | <code>struct ncclTree* tree = &channel-&gt;tree;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt; NCCL_MAX_TREE_ARITY；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 23 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxySend, tree-&gt;down[i], op, 0));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 24 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxyRecv, tree-&gt;up, op, 0));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 26 | 0 / 条件判断 | <code>(pattern == ncclPatternCollTreeUpDown)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxySend, channel-&gt;collTree.out, op, 1));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxyRecv, channel-&gt;collTree.out, op, 0));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 30 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclChannel* channel = comm->channels+op->channelId;
  int pattern = op->pattern;
  if (pattern == ncclPatternRing || pattern == ncclPatternRingTwice || pattern == ncclPatternPipelineFrom || pattern == ncclPatternPipelineTo) {
    struct ncclRing* ring = &channel->ring;
    if (NeedProxy(proxyRecv, pattern, op->root, ring, nranks)) NCCLCHECK(SaveProxy(channel, proxyRecv, ring->prev, op, 0));
    if (NeedProxy(proxySend, pattern, op->root, ring, nranks)) NCCLCHECK(SaveProxy(channel, proxySend, ring->next, op, 0));
  }
  if (pattern == ncclPatternTreeUp || pattern == ncclPatternTreeUpDown) {
    // Tree up
    struct ncclTree* tree = &channel->tree;
    for (int i=0; i<NCCL_MAX_TREE_ARITY; i++) NCCLCHECK(SaveProxy(channel, proxyRecv, tree->down[i], op, 0));
    NCCLCHECK(SaveProxy(channel, proxySend, tree->up, op, 0));
  }
  if (pattern == ncclPatternTreeDown || pattern == ncclPatternTreeUpDown) {
    // Tree down
    struct ncclTree* tree = &channel->tree;
    for (int i=0; i< NCCL_MAX_TREE_ARITY; i++) NCCLCHECK(SaveProxy(channel, proxySend, tree->down[i], op, 0));
    NCCLCHECK(SaveProxy(channel, proxyRecv, tree->up, op, 0));
  }
  if (pattern == ncclPatternCollTreeUpDown) {
    // CollTree up
    NCCLCHECK(SaveProxy(channel, proxySend, channel->collTree.out, op, 1));  // For CollTree up, we are using push
    // CollTree down
    NCCLCHECK(SaveProxy(channel, proxyRecv, channel->collTree.out, op, 0));
  }
  return ncclSuccess;
}
```

</details>

<a id="f1025"></a>

## F1025：ncclProxyComputeP2p

用途：根据已切好的 P2P 块计算协议尺寸、step 数和 channel/peer 标签，使 CPU/GPU 计划一致。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>memset(op, 0, sizeof(struct ncclProxyOp));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>int channelId = info-&gt;channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclChannel* channel = info-&gt;comm-&gt;channels+channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;channelId = channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;sliceSteps = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;chunkSteps = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;protocol = NCCL_PROTO_SIMPLE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;dtype = info-&gt;datatype;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 准备局部状态 | <code>int stepSize = info-&gt;comm-&gt;buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS/SENDRECV_SLICEFACTOR;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;chunkSize = stepSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;root = info-&gt;root;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;nbytes = info-&gt;count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 准备局部状态 | <code>struct ncclPeer* peer = channel-&gt;peers + op-&gt;root;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 条件判断 | <code>(info-&gt;coll == ncclFuncSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>op-&gt;pattern = ncclPatternSend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 条件判断 | <code>(op-&gt;root != info-&gt;comm-&gt;rank && peer-&gt;send[1].transportComm && peer-&gt;send[1].transportComm-&gt;proxyProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 4 / 条件判断 | <code>(info-&gt;count &lt; stepSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 6 / 调用 / 计算 / 写入 | <code>info-&gt;chunkSize /= 4;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 6 / 条件判断 | <code>(info-&gt;count &lt; 8*stepSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 8 / 调用 / 计算 / 写入 | <code>info-&gt;chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 27 | 2 / 条件判断 | <code>(info-&gt;coll == ncclFuncRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>op-&gt;pattern = ncclPatternRecv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 4 / 条件判断 | <code>(op-&gt;root != info-&gt;comm-&gt;rank && peer-&gt;recv[1].transportComm && peer-&gt;recv[1].transportComm-&gt;proxyProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 6 / 条件判断 | <code>(info-&gt;count &lt; stepSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 8 / 调用 / 计算 / 写入 | <code>info-&gt;chunkSize /= 4;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 36 | 8 / 条件判断 | <code>(info-&gt;count &lt; 8*stepSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 10 / 调用 / 计算 / 写入 | <code>info-&gt;chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 40 | 4 / 调用 / 计算 / 写入 | <code>WARN("P2p operation is neither send or recv");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 42 | 0 / 条件判断 | <code>(ncclParamChunkSize() != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;chunkSize = ncclParamChunkSize();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;chunkSize = info-&gt;chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  memset(op, 0, sizeof(struct ncclProxyOp));
  int channelId = info->channelId;
  struct ncclChannel* channel = info->comm->channels+channelId;
  op->channelId = channelId;
  op->sliceSteps = 1;
  op->chunkSteps = 1;
  op->protocol = NCCL_PROTO_SIMPLE;
  op->dtype = info->datatype;

  int stepSize = info->comm->buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS/SENDRECV_SLICEFACTOR;
  info->chunkSize = stepSize;
  op->root = info->root;
  op->nbytes = info->count;
  struct ncclPeer* peer = channel->peers + op->root;

  if (info->coll == ncclFuncSend) {
    op->pattern = ncclPatternSend;
    if (op->root != info->comm->rank && peer->send[1].transportComm && peer->send[1].transportComm->proxyProgress) {
      // Tune chunk size for the network
      if (info->count < stepSize) info->chunkSize /= 4;
      else if (info->count < 8*stepSize) info->chunkSize /= 2;
    }
  } else if (info->coll == ncclFuncRecv) {
    op->pattern = ncclPatternRecv;
    if (op->root != info->comm->rank && peer->recv[1].transportComm && peer->recv[1].transportComm->proxyProgress) {
      // Tune chunk size for the network
      if (info->count < stepSize) info->chunkSize /= 4;
      else if (info->count < 8*stepSize) info->chunkSize /= 2;
    }
  } else {
    WARN("P2p operation is neither send or recv");
    return ncclInternalError;
  }
  if (ncclParamChunkSize() != 0) {
    info->chunkSize = ncclParamChunkSize();
  }
  op->chunkSize = info->chunkSize;
  return ncclSuccess;
}
```

</details>

<a id="f1026"></a>

## F1026：ncclProxySaveP2p

用途：按 P2P 任务方向和连接索引1保存代理需求，跳过 self 或无代理的本地路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+op-&gt;channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>op-&gt;opCount = channel-&gt;workFifoTail-1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 条件判断 | <code>(op-&gt;root == comm-&gt;rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 条件判断 | <code>(op-&gt;pattern == ncclPatternRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>op-&gt;nsteps = DIVUP(op-&gt;nbytes, op-&gt;chunkSize);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 条件判断 | <code>(op-&gt;nsteps == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 调用 / 计算 / 写入 | <code>op-&gt;nsteps = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxyRecv, op-&gt;root, op, 1));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 13 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 14 | 2 / 条件判断 | <code>(op-&gt;pattern == ncclPatternSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>op-&gt;nsteps = DIVUP(op-&gt;nbytes, op-&gt;chunkSize);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 4 / 条件判断 | <code>(op-&gt;nsteps == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 6 / 调用 / 计算 / 写入 | <code>op-&gt;nsteps = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(SaveProxy(channel, proxySend, op-&gt;root, op, 1));</code> | 失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。 |
| 21 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclChannel* channel = comm->channels+op->channelId;
  op->opCount = channel->workFifoTail-1;
  if (op->root == comm->rank) return ncclSuccess;
  if (op->pattern == ncclPatternRecv) {
    op->nsteps = DIVUP(op->nbytes, op->chunkSize);
    if (op->nsteps == 0) op->nsteps = 1;
    NCCLCHECK(SaveProxy(channel, proxyRecv, op->root, op, 1));
  } else if (op->pattern == ncclPatternSend) {
    op->nsteps = DIVUP(op->nbytes, op->chunkSize);
    if (op->nsteps == 0) op->nsteps = 1;
    NCCLCHECK(SaveProxy(channel, proxySend, op->root, op, 1));
  }
  return ncclSuccess;
}
```

</details>

<a id="f1027"></a>

## F1027：removeOp

用途：从活跃 Proxy 链移除完成项，提升同连接后继项并归还可复用参数对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyArgs* freeOp = *opPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclProxyArgs* next = freeOp-&gt;next;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>DEBUG_PROXY_PRINT("Remove %ld -&gt; %ld -&gt; %ld\n", OP_INDEX(*prevOpPtr), OP_INDEX(freeOp), OP_INDEX(next));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>*opPtr = next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(freeOp-&gt;nextPeer)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 准备局部状态 | <code>struct ncclProxyArgs* nextPeer = freeOp-&gt;nextPeer;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 条件判断 | <code>(*prevOpPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 4 / 调用 / 计算 / 写入 | <code>(*prevOpPtr)-&gt;next = nextPeer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>state-&gt;active = nextPeer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>nextPeer-&gt;next = next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>*(prevOpPtr) = nextPeer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>*(freeOp-&gt;proxyAppendPtr) = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 条件判断 | <code>(*prevOpPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 4 / 调用 / 计算 / 写入 | <code>(*prevOpPtr)-&gt;next = next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>state-&gt;active = next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>freeOp-&gt;next = state-&gt;pool;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;pool = freeOp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>DEBUG_PROXY_PRINT("Removed %5ld (%5ld) : ", OP_INDEX(freeOp), OP_INDEX(*freeOp-&gt;proxyAppendPtr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 0 / 编译期分支/指令 | <code>#ifdef DEBUG_PROXY</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(dumpProxyState(state));</code> | 失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。 |
| 27 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyArgs* freeOp = *opPtr;
  struct ncclProxyArgs* next = freeOp->next;
  DEBUG_PROXY_PRINT("Remove %ld -> %ld -> %ld\n", OP_INDEX(*prevOpPtr), OP_INDEX(freeOp), OP_INDEX(next));
  *opPtr = next;
  if (freeOp->nextPeer) {
    // replace op by nextPeer
    struct ncclProxyArgs* nextPeer = freeOp->nextPeer;
    if (*prevOpPtr) {
      (*prevOpPtr)->next = nextPeer;
    } else {
      state->active = nextPeer;
    }
    nextPeer->next = next;
    *(prevOpPtr) = nextPeer;
  } else {
    *(freeOp->proxyAppendPtr) = NULL;
    if (*prevOpPtr) {
      (*prevOpPtr)->next = next;
    } else {
      state->active = next;
    }
  }
  freeOp->next = state->pool;
  state->pool = freeOp;
  DEBUG_PROXY_PRINT("Removed %5ld (%5ld) : ", OP_INDEX(freeOp), OP_INDEX(*freeOp->proxyAppendPtr));
#ifdef DEBUG_PROXY
  NCCLCHECK(dumpProxyState(state));
#endif
  return ncclSuccess;
}
```

</details>

<a id="f1028"></a>

## F1028：progressOps

用途：遍历活跃操作并调用各 transport 的非阻塞进度函数，完成后摘链，未完成保留下一轮。

- 遍历活跃链，调用每个transport提供的非阻塞progress回调。
- 未完成项保留，完成项从链中摘除并按同连接先后规则提升下一项；一次回调不要求等整个张量完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyArgs* prevOp = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclProxyArgs* op = opStart;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 ；继续条件 (op)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(op-&gt;state == ncclProxyOpNone)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(op-&gt;progress(comm, op));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 10 | 1 / 条件判断 | <code>(op-&gt;idle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>TIME_STOP(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 3 / 调用 / 计算 / 写入 | <code>TIME_CANCEL(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 3 / 调用 / 计算 / 写入 | <code>TIME_CANCEL(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>TIME_STOP(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 1 / 调用 / 计算 / 写入 | <code>*idle &= op-&gt;idle;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 1 / 条件判断 | <code>(op-&gt;state == ncclProxyOpNone)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>TIME_START(2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(removeOp(state, &op, &prevOp));</code> | 失败时可能提前return，成功才继续；从活跃 Proxy 链移除完成项，提升同连接后继项并归还可复用参数对象。 |
| 22 | 3 / 调用 / 计算 / 写入 | <code>TIME_STOP(2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 24 | 3 / 调用 / 计算 / 写入 | <code>prevOp = op;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 3 / 调用 / 计算 / 写入 | <code>op = op-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyArgs* prevOp = NULL;
  struct ncclProxyArgs* op = opStart;
  while (op) {
    if (op->state == ncclProxyOpNone) return ncclInternalError;
    TIME_START(0); TIME_START(1);
    NCCLCHECK(op->progress(comm, op));
    if (op->idle) { TIME_STOP(1); TIME_CANCEL(0); } else { TIME_CANCEL(1); TIME_STOP(0); }
    *idle &= op->idle;
    if (op->state == ncclProxyOpNone) {
      TIME_START(2);
      NCCLCHECK(removeOp(state, &op, &prevOp));
      TIME_STOP(2);
    } else {
      prevOp = op;
      op = op->next;
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f1029"></a>

## F1029：ncclProxyGetPostedOps

用途：从共享池取走已发布工作，转换/聚合成运行态队列；无任务时可在条件变量上等待。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyProgressState* state = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;opsPool == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclProxyOpsPool* pool = state-&gt;opsPool;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclProxyArgs profArgs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 条件判断 | <code>(state-&gt;nextOps != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 控制转移 | <code>goto process_nextops;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 10 | 0 / 条件判断 | <code>(state-&gt;active != NULL && (pool-&gt;nextOps == -1 &#124;&#124; pthread_mutex_trylock(&pool-&gt;mutex) != 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 条件判断 | <code>(state-&gt;active == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&pool-&gt;mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 2 / 循环 | <code>初始 ；继续条件 (pool-&gt;nextOps == -1 && !state-&gt;stop)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 3 / 准备局部状态 | <code>struct ncclProxyArgs profArgs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileSleep);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 19 | 3 / 调用 / 计算 / 写入 | <code>pthread_cond_wait(&pool-&gt;cond, &pool-&gt;mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileWakeup);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 21 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 2 / 条件判断 | <code>(state-&gt;stop)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&pool-&gt;mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;nextOps = pool-&gt;nextOps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>pool-&gt;nextOps = pool-&gt;nextOpsEnd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&pool-&gt;mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 0 / 条件判断 | <code>(state-&gt;nextOps == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 32 | 0 / 跳转标签 | <code>process_nextops</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 33 | 1 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileAppend);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>TIME_START(2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 0 / 准备局部状态 | <code>int freeOp[NCCL_MAX_LOCAL_RANKS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 36 | 0 / 准备局部状态 | <code>int freeOpEnd[NCCL_MAX_LOCAL_RANKS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;localRanks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 38 | 1 / 调用 / 计算 / 写入 | <code>freeOp[i] = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 40 | 0 / 循环 | <code>初始 int opIndex = state-&gt;nextOps;；继续条件 opIndex != -1；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 41 | 1 / 准备局部状态 | <code>struct ncclProxyOp* peerOp = pool-&gt;ops+opIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 1 / 准备局部状态 | <code>int peer = opIndex / MAX_OPS_PER_PEER;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 1 / 条件判断 | <code>(peerOp-&gt;connection == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 46 | 1 / 条件判断 | <code>(peerOp-&gt;next != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 3 / 调用 / 计算 / 写入 | <code>__builtin_prefetch(pool-&gt;ops+peerOp-&gt;next);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ProxyAppend(state, peerOp));</code> | 失败时可能提前return，成功才继续；把提交操作追加/聚合到活跃链或同连接后继链，维持共享连接上的先后顺序。 |
| 50 | 1 / 调用 / 计算 / 写入 | <code>(*added)++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 1 / 准备局部状态 | <code>int lastOpIndex = opIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 1 / 调用 / 计算 / 写入 | <code>opIndex = peerOp-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 1 / 条件判断 | <code>(freeOp[peer] == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 54 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 55 | 3 / 调用 / 计算 / 写入 | <code>freeOpEnd[peer] = lastOpIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 57 | 3 / 调用 / 计算 / 写入 | <code>peerOp-&gt;next = freeOp[peer];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 1 / 调用 / 计算 / 写入 | <code>freeOp[peer] = lastOpIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 59 | 1 / 调用 / 计算 / 写入 | <code>state-&gt;nextOps = opIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 61 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;localRanks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 62 | 1 / 条件判断 | <code>(freeOp[i] == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 65 | 1 / 准备局部状态 | <code>int newFree = freeOp[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 66 | 1 / 准备局部状态 | <code>int oldFree = pool-&gt;freeOps[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 67 | 1 / 调用 / 计算 / 写入 | <code>pool-&gt;ops[freeOpEnd[i]].next = oldFree;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 68 | 1 / 条件判断 | <code>(oldFree == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 69 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 70 | 3 / 调用 / 计算 / 写入 | <code>pool-&gt;freeOps[i] = newFree;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 71 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 72 | 3 / 准备局部状态 | <code>int swap = __sync_val_compare_and_swap(pool-&gt;freeOps+i, oldFree, newFree);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 73 | 3 / 条件判断 | <code>(swap != oldFree)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 74 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 75 | 5 / 条件判断 | <code>(swap != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 76 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 77 | 7 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 78 | 5 / 调用 / 计算 / 写入 | <code>pool-&gt;ops[freeOpEnd[i]].next = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 5 / 调用 / 计算 / 写入 | <code>pool-&gt;freeOps[i] = newFree;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 80 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 81 | 0 / 调用 / 计算 / 写入 | <code>profArgs.opCount = *added;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 0 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileAppendEnd);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 83 | 0 / 调用 / 计算 / 写入 | <code>TIME_STOP(2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 84 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (state->opsPool == NULL) return ncclInternalError;
  struct ncclProxyOpsPool* pool = state->opsPool;

  struct ncclProxyArgs profArgs; // Only used for profiling purposes
  if (state->nextOps != -1) goto process_nextops;

  // If we have ops to progress, no need to block waiting for something to arrive or even wait for the lock
  // to be available. Exit, continue progress, and come back later.
  if (state->active != NULL && (pool->nextOps == -1 || pthread_mutex_trylock(&pool->mutex) != 0)) return ncclSuccess;

  if (state->active == NULL) {
    pthread_mutex_lock(&pool->mutex);
    while (pool->nextOps == -1 && !state->stop) {
      struct ncclProxyArgs profArgs; // Only used for profiling purposes
      ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileSleep);
      pthread_cond_wait(&pool->cond, &pool->mutex);
      ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileWakeup);
    }
    if (state->stop) { // We might have been woken up to stop.
      pthread_mutex_unlock(&pool->mutex);
      return ncclSuccess;
    }
  }

  state->nextOps = pool->nextOps;
  pool->nextOps = pool->nextOpsEnd = -1;
  pthread_mutex_unlock(&pool->mutex);
  if (state->nextOps == -1) return ncclInternalError;

process_nextops:
  ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileAppend);
  TIME_START(2);
  int freeOp[NCCL_MAX_LOCAL_RANKS];
  int freeOpEnd[NCCL_MAX_LOCAL_RANKS];
  for (int i=0; i<comm->localRanks; i++) freeOp[i] = -1;

  for (int opIndex = state->nextOps; opIndex != -1;) {
    struct ncclProxyOp* peerOp = pool->ops+opIndex;
    int peer = opIndex / MAX_OPS_PER_PEER;
    if (peerOp->connection == NULL) return ncclInternalError;
    if (peerOp->next != -1) __builtin_prefetch(pool->ops+peerOp->next);
    NCCLCHECK(ProxyAppend(state, peerOp));
    (*added)++;
    int lastOpIndex = opIndex;
    opIndex = peerOp->next;
    // Return op to peer pool
    if (freeOp[peer] == -1) {
      freeOpEnd[peer] = lastOpIndex;
    } else {
      peerOp->next = freeOp[peer];
    }
    freeOp[peer] = lastOpIndex;
    state->nextOps = opIndex;
  }

  for (int i=0; i<comm->localRanks; i++) {
    if (freeOp[i] == -1) continue;
    int newFree = freeOp[i];
    int oldFree = pool->freeOps[i];
    pool->ops[freeOpEnd[i]].next = oldFree;
    if (oldFree == -1) {
      // Nothing for the main thread to consume, we can set it.
      pool->freeOps[i] = newFree;
    } else {
      // The main thread may recycle free ops at any time, replace the freeOps value atomically and check it worked.
      int swap = __sync_val_compare_and_swap(pool->freeOps+i, oldFree, newFree);
      if (swap != oldFree) {
        if (swap != -1) return ncclInternalError;
        // Ops were recycled while we were trying to swap, just set the value directly now.
        pool->ops[freeOpEnd[i]].next = -1;
        pool->freeOps[i] = newFree;
      }
    }
  }
  profArgs.opCount = *added;
  ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileAppendEnd);
  TIME_STOP(2);
  return ncclSuccess;
}
```

</details>

<a id="f1030"></a>

## F1030：ncclDumpProxyState

用途：打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>dumpProxyState(ncclLastProxyState);</code> | 打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  dumpProxyState(ncclLastProxyState);
}
```

</details>

<a id="f1031"></a>

## F1031：ncclProxyProgress

用途：代理数据面后台循环，交错推进多个 channel/peer 的网络请求并处理退出/异步错误。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclComm* comm = (struct ncclComm*)comm_;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclProxyProgressState* state = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;nextOps = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>signal(SIGUSR1, ncclDumpProxyState);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>ncclLastProxyState = state;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 准备局部状态 | <code>char threadName[NCCL_THREAD_NAMELEN];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>snprintf(threadName, NCCL_THREAD_NAMELEN, "NCCL Progress%2d", comm-&gt;cudaDev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>nvtxNameOsThreadA(syscall(SYS_gettid), threadName);</code> | 为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |
| 9 | 0 / 准备局部状态 | <code>int lastIdle = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>struct ncclProxyArgs profArgs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 循环 | <code>初始 ；继续条件 (state-&gt;stop == 0 && *comm-&gt;abortFlag == 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 准备局部状态 | <code>int idle = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 准备局部状态 | <code>ncclResult_t ret = progressOps(comm, state, state-&gt;active, &idle);</code> | 遍历活跃操作并调用各 transport 的非阻塞进度函数，完成后摘链，未完成保留下一轮。 |
| 14 | 1 / 条件判断 | <code>(ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>comm-&gt;fatalError = ret;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ALL,"%s:%d -&gt; %d [Proxy Thread]", __FILE__, __LINE__, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 3 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 19 | 1 / 条件判断 | <code>(lastIdle == 0 && idle == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileIdle);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 22 | 1 / 条件判断 | <code>(lastIdle == 1 && idle == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 3 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileActive);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 25 | 1 / 条件判断 | <code>(idle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 3 / 准备局部状态 | <code>int added = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>TIME_START(3);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 3 / 调用 / 计算 / 写入 | <code>ret = ncclProxyGetPostedOps(comm, &added);</code> | 从共享池取走已发布工作，转换/聚合成运行态队列；无任务时可在条件变量上等待。 |
| 30 | 3 / 条件判断 | <code>(added)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 5 / 调用 / 计算 / 写入 | <code>TIME_STOP(3);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 34 | 5 / 调用 / 计算 / 写入 | <code>TIME_CANCEL(3);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 3 / 条件判断 | <code>(ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;fatalError = ret;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 5 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ALL,"%s:%d -&gt; %d [Proxy Thread]", __FILE__, __LINE__, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 3 / 条件判断 | <code>(added == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 5 / 调用 / 计算 / 写入 | <code>sched_yield();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 1 / 调用 / 计算 / 写入 | <code>lastIdle = idle;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 44 | 0 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclComm* comm = (struct ncclComm*)comm_;
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  state->nextOps = -1;
  signal(SIGUSR1, ncclDumpProxyState);
  ncclLastProxyState = state;
  char threadName[NCCL_THREAD_NAMELEN];
  snprintf(threadName, NCCL_THREAD_NAMELEN, "NCCL Progress%2d", comm->cudaDev);
  nvtxNameOsThreadA(syscall(SYS_gettid), threadName);

  int lastIdle = 0;
  struct ncclProxyArgs profArgs; // Only used for profiling purposes
  while (state->stop == 0 && *comm->abortFlag == 0) {
    int idle = 1;
    ncclResult_t ret = progressOps(comm, state, state->active, &idle);
    if (ret != ncclSuccess) {
      comm->fatalError = ret;
      INFO(NCCL_ALL,"%s:%d -> %d [Proxy Thread]", __FILE__, __LINE__, ret);
      return NULL;
    }
    if (lastIdle == 0 && idle == 1) ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileIdle);
    if (lastIdle == 1 && idle == 0) ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileActive);
    if (idle) {
      int added = 0;
      TIME_START(3);
      ret = ncclProxyGetPostedOps(comm, &added);
      if (added) { TIME_STOP(3); } else { TIME_CANCEL(3); }
      if (ret != ncclSuccess) {
        comm->fatalError = ret;
        INFO(NCCL_ALL,"%s:%d -> %d [Proxy Thread]", __FILE__, __LINE__, ret);
      }
      if (added == 0) {
        sched_yield(); // No request progressed. Let others run.
      }
    }
    lastIdle = idle;
  }
  return NULL;
}
```

</details>

<a id="f1032"></a>

## F1032：ncclProxyStart

用途：发布本批累积代理工作到相关本地代理的池，递增操作编号而不新建每消息线程。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyOps* proxyOps = comm-&gt;proxyState.proxyOps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(proxyOps == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>TIME_START(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;localRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 准备局部状态 | <code>struct ncclProxyOps* ops = proxyOps+r;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 条件判断 | <code>(ops-&gt;pool == NULL &#124;&#124; ops-&gt;nextOps == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyPost(ops-&gt;pool, ops-&gt;nextOps, ops-&gt;nextOpsEnd));</code> | 失败时可能提前return，成功才继续；在锁内发布任务索引链，队列从空转为非空时通过条件变量唤醒 Progress 线程。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>ops-&gt;nextOps = ops-&gt;nextOpsEnd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>ops-&gt;count = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;opCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>TIME_STOP(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyOps* proxyOps = comm->proxyState.proxyOps;
  if (proxyOps == NULL) return ncclSuccess;
  TIME_START(1);
  for (int r=0; r<comm->localRanks; r++) {
    struct ncclProxyOps* ops = proxyOps+r;
    if (ops->pool == NULL || ops->nextOps == -1) continue;
    NCCLCHECK(ncclProxyPost(ops->pool, ops->nextOps, ops->nextOpsEnd));
    ops->nextOps = ops->nextOpsEnd = -1;
    ops->count = 0;
  }
  comm->opCount++;
  TIME_STOP(1);
  return ncclSuccess;
}
```

</details>

<a id="f1033"></a>

## F1033：ncclProxyProgressCreate

用途：按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyProgressState* state = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(!state-&gt;thread)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>pthread_create(&state-&gt;thread, NULL, ncclProxyProgress, comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>ncclSetThreadName(state-&gt;thread, "NCCL Progress%2d", comm-&gt;cudaDev);</code> | 为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (!state->thread) {
    pthread_create(&state->thread, NULL, ncclProxyProgress, comm);
    ncclSetThreadName(state->thread, "NCCL Progress%2d", comm->cudaDev);
  }
  return ncclSuccess;
}
```

</details>

<a id="f1034"></a>

## F1034：ncclProxyProgressDestroy

用途：通知数据面线程停止并唤醒等待者，join 后回收共享任务和运行状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyProgressState* state = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;opsPool)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&state-&gt;opsPool-&gt;mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>state-&gt;stop = true;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>pthread_cond_signal(&state-&gt;opsPool-&gt;cond);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&state-&gt;opsPool-&gt;mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>pthread_join(state-&gt;thread, NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 循环 | <code>初始 ；继续条件 (state-&gt;pools != NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 1 / 准备局部状态 | <code>struct ncclProxyPool *next = state-&gt;pools-&gt;next;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>free(state-&gt;pools);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>state-&gt;pools = next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>ncclProfilingDump();</code> | 导出代理阶段耗时记录，帮助区分 GPU 等待、网络等待和 flush；禁用分支无操作。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>TIME_PRINT("Proxy");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;

  // Request the proxy to stop and then wake it
  if (state->opsPool) {
    pthread_mutex_lock(&state->opsPool->mutex);
    state->stop = true;
    pthread_cond_signal(&state->opsPool->cond);
    pthread_mutex_unlock(&state->opsPool->mutex);
    pthread_join(state->thread, NULL);
  }

  // Free off any memory allocated for the proxy arg pools
  while (state->pools != NULL) {
    struct ncclProxyPool *next = state->pools->next;
    free(state->pools);
    state->pools = next;
  }

  ncclProfilingDump();
  TIME_PRINT("Proxy");
  return ncclSuccess;
}
```

</details>

<a id="f1035"></a>

## F1035：ncclProxyNewConnection

用途：在分块连接池分配一个空连接槽，返回可用于控制消息索引的连接编号。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(pool-&gt;offset == NCCL_PROXY_CONN_POOL_SIZE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclRealloc(&pool-&gt;pools, pool-&gt;banks, pool-&gt;banks+1));</code> | 失败时可能提前return，成功才继续；分配更大且尾部清零的 CPU 数组，复制旧元素后释放旧内存，不支持缩小。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(pool-&gt;pools+pool-&gt;banks, NCCL_PROXY_CONN_POOL_SIZE));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>pool-&gt;banks++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>pool-&gt;offset = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>*id = ((pool-&gt;banks-1) &lt;&lt; NCCL_PROXY_CONN_POOL_SIZE_POW2) + pool-&gt;offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>pool-&gt;offset++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (pool->offset == NCCL_PROXY_CONN_POOL_SIZE) {
    NCCLCHECK(ncclRealloc(&pool->pools, pool->banks, pool->banks+1));
    NCCLCHECK(ncclCalloc(pool->pools+pool->banks, NCCL_PROXY_CONN_POOL_SIZE));
    pool->banks++;
    pool->offset = 0;
  }
  *id = ((pool->banks-1) << NCCL_PROXY_CONN_POOL_SIZE_POW2) + pool->offset;
  pool->offset++;
  return ncclSuccess;
}
```

</details>

<a id="f1036"></a>

## F1036：ncclProxyGetConnection

用途：按池编号定位已存在代理连接对象，校验池/索引后返回其地址。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int bank = id&gt;&gt;NCCL_PROXY_CONN_POOL_SIZE_POW2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int offset = id&NCCL_PROXY_CONN_POOL_MASK;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>((pool-&gt;pools == NULL) &#124;&#124; (bank &gt; pool-&gt;banks) &#124;&#124; (pool-&gt;pools[bank] == NULL))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>*conn = pool-&gt;pools[bank]+offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int bank = id>>NCCL_PROXY_CONN_POOL_SIZE_POW2;
  int offset = id&NCCL_PROXY_CONN_POOL_MASK;
  if ((pool->pools == NULL) || (bank > pool->banks) || (pool->pools[bank] == NULL)) return ncclInternalError;
  *conn = pool->pools[bank]+offset;
  return ncclSuccess;
}
```

</details>

<a id="f1037"></a>

## F1037：proxyFree

用途：调用连接 transport 的代理资源释放回调，按具体后端回收连接持有的对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(connection-&gt;send)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTransports[connection-&gt;transport].send.proxyFree(connection, comm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTransports[connection-&gt;transport].recv.proxyFree(connection, comm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (connection->send) {
    NCCLCHECK(ncclTransports[connection->transport].send.proxyFree(connection, comm));
  } else {
    NCCLCHECK(ncclTransports[connection->transport].recv.proxyFree(connection, comm));
  }
  return ncclSuccess;
}
```

</details>

<a id="f1038"></a>

## F1038：ncclProxyFreeConnections

用途：遍历代理连接池释放全部已分配连接及池块，避免退出后残留后台资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int b=0;；继续条件 b&lt;pool-&gt;banks；每轮更新 b++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>int max = b == pool-&gt;banks-1 ? pool-&gt;offset : NCCL_PROXY_CONN_POOL_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;max；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(proxyFree(pool-&gt;pools[b]+i, comm));</code> | 失败时可能提前return，成功才继续；调用连接 transport 的代理资源释放回调，按具体后端回收连接持有的对象。 |
| 5 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>free(pool-&gt;pools[b]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>free(pool-&gt;pools);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int b=0; b<pool->banks; b++) {
    int max = b == pool->banks-1 ? pool->offset : NCCL_PROXY_CONN_POOL_SIZE;
    for (int i=0; i<max; i++) {
      NCCLCHECK(proxyFree(pool->pools[b]+i, comm));
    }
    free(pool->pools[b]);
  }
  free(pool->pools);
  return ncclSuccess;
}
```

</details>

<a id="f1039"></a>

## F1039：ncclProxyConnect

用途：联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>proxyConn-&gt;connection = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>proxyConn-&gt;rank = rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 条件判断 | <code>(comm-&gt;proxyState.peerSocks == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;proxyState.peerSocks, comm-&gt;localRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;proxyState.proxyOps, comm-&gt;localRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;proxyState.sharedDevMems, comm-&gt;localRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 2 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;localRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>comm-&gt;proxyState.peerSocks[r].fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>comm-&gt;proxyState.peerSocks[r].abortFlag = comm-&gt;abortFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetLocalRank(comm-&gt;topo, rank, &proxyConn-&gt;localRank));</code> | 失败时可能提前return，成功才继续；把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。 |
| 13 | 0 / 准备局部状态 | <code>struct ncclSocket* sock = comm-&gt;proxyState.peerSocks+proxyConn-&gt;localRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 条件判断 | <code>(sock-&gt;fd == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>memcpy(&sock-&gt;addr, comm-&gt;proxyState.peerAddresses+rank, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketConnect(sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。 |
| 18 | 0 / 准备局部状态 | <code>int type = ncclProxyMsgInit;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(sock, &type, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(sock, &transport, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(sock, &send, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(sock, &comm-&gt;localRank, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &proxyConn-&gt;connection, sizeof(void*)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 24 | 0 / 准备局部状态 | <code>struct ncclTransportComm* tcomm = send ? &ncclTransports[transport].send : &ncclTransports[transport].recv;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 0 / 条件判断 | <code>(tcomm-&gt;proxyProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 2 / 准备局部状态 | <code>char poolPath[] = "/dev/shm/nccl-XXXXXX";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, poolPath+sizeof("/dev/shm/nccl-")-1, sizeof("XXXXXX")-1));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 29 | 2 / 准备局部状态 | <code>struct ncclProxyOps* proxyOps = comm-&gt;proxyState.proxyOps+proxyConn-&gt;localRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 2 / 条件判断 | <code>(proxyOps-&gt;pool == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmOpen(poolPath, sizeof(struct ncclProxyOpsPool), (void**)(&proxyOps-&gt;pool), NULL, 0));</code> | 失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。 |
| 33 | 4 / 调用 / 计算 / 写入 | <code>proxyOps-&gt;nextOps = proxyOps-&gt;nextOpsEnd = proxyOps-&gt;freeOp = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET, "Connection to proxy localRank %d -&gt; connection %p", proxyConn-&gt;localRank, proxyConn-&gt;connection);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>proxyConn-&gt;comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Keep one connection per mlocal rank
  proxyConn->connection = NULL;
  proxyConn->rank = rank;
  if (comm->proxyState.peerSocks == NULL) {
    NCCLCHECK(ncclCalloc(&comm->proxyState.peerSocks, comm->localRanks));
    NCCLCHECK(ncclCalloc(&comm->proxyState.proxyOps, comm->localRanks));
    NCCLCHECK(ncclCalloc(&comm->proxyState.sharedDevMems, comm->localRanks));
    for (int r=0; r<comm->localRanks; r++) {
      comm->proxyState.peerSocks[r].fd = -1;
      comm->proxyState.peerSocks[r].abortFlag = comm->abortFlag;
    }
  }
  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, rank, &proxyConn->localRank));
  struct ncclSocket* sock = comm->proxyState.peerSocks+proxyConn->localRank;
  if (sock->fd == -1) {
    memcpy(&sock->addr, comm->proxyState.peerAddresses+rank, sizeof(union ncclSocketAddress));
    NCCLCHECK(ncclSocketConnect(sock));
  }
  int type = ncclProxyMsgInit;
  NCCLCHECK(ncclSocketSend(sock, &type, sizeof(int)));
  NCCLCHECK(ncclSocketSend(sock, &transport, sizeof(int)));
  NCCLCHECK(ncclSocketSend(sock, &send, sizeof(int)));
  NCCLCHECK(ncclSocketSend(sock, &comm->localRank, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &proxyConn->connection, sizeof(void*)));
  struct ncclTransportComm* tcomm = send ? &ncclTransports[transport].send : &ncclTransports[transport].recv;
  // If we need proxy progress, map progress ops
  if (tcomm->proxyProgress) {
    char poolPath[] = "/dev/shm/nccl-XXXXXX";
    NCCLCHECK(ncclSocketRecv(sock, poolPath+sizeof("/dev/shm/nccl-")-1, sizeof("XXXXXX")-1));
    struct ncclProxyOps* proxyOps = comm->proxyState.proxyOps+proxyConn->localRank;
    if (proxyOps->pool == NULL) {
      NCCLCHECK(ncclShmOpen(poolPath, sizeof(struct ncclProxyOpsPool), (void**)(&proxyOps->pool), NULL, 0));
      proxyOps->nextOps = proxyOps->nextOpsEnd = proxyOps->freeOp = -1;
    }
  }
  INFO(NCCL_NET, "Connection to proxy localRank %d -> connection %p", proxyConn->localRank, proxyConn->connection);
  proxyConn->comm = comm;
  return ncclSuccess;
}
```

</details>

<a id="f1040"></a>

## F1040：ncclProxyCall

用途：通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(proxyConn-&gt;comm-&gt;proxyState.peerSocks == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclSocket* sock = proxyConn-&gt;comm-&gt;proxyState.peerSocks+proxyConn-&gt;localRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(sock-&gt;fd == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 准备局部状态 | <code>ncclResult_t ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSocketSend(sock, &type, sizeof(int)), ret, error);</code> | 失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSocketSend(sock, &proxyConn-&gt;connection, sizeof(void*)), ret, error);</code> | 失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSocketSend(sock, &reqSize, sizeof(int)), ret, error);</code> | 失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSocketSend(sock, &respSize, sizeof(int)), ret, error);</code> | 失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 13 | 0 / 条件判断 | <code>(reqSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSocketSend(sock, reqBuff, reqSize), ret, error);</code> | 失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 16 | 0 / 条件判断 | <code>(respSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSocketRecv(sock, respBuff, respSize), ret, error);</code> | 失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 20 | 0 / 跳转标签 | <code>error</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 21 | 1 / 调用 / 计算 / 写入 | <code>WARN("Proxy Call to rank %d failed (%s)", proxyConn-&gt;comm-&gt;localRankToRank[proxyConn-&gt;localRank], ncclProxyMsgTypeStr[type]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 函数返回 | <code>return ret;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (proxyConn->comm->proxyState.peerSocks == NULL) return ncclInternalError;
  struct ncclSocket* sock = proxyConn->comm->proxyState.peerSocks+proxyConn->localRank;
  if (sock->fd == -1) return ncclInternalError;
  ncclResult_t ret;

  NCCLCHECKGOTO(ncclSocketSend(sock, &type, sizeof(int)), ret, error);
  NCCLCHECKGOTO(ncclSocketSend(sock, &proxyConn->connection, sizeof(void*)), ret, error);
  NCCLCHECKGOTO(ncclSocketSend(sock, &reqSize, sizeof(int)), ret, error);
  NCCLCHECKGOTO(ncclSocketSend(sock, &respSize, sizeof(int)), ret, error);
  if (reqSize) NCCLCHECKGOTO(ncclSocketSend(sock, reqBuff, reqSize), ret, error);
  if (respSize) NCCLCHECKGOTO(ncclSocketRecv(sock, respBuff, respSize), ret, error);
  return ncclSuccess;
error:
  WARN("Proxy Call to rank %d failed (%s)", proxyConn->comm->localRankToRank[proxyConn->localRank], ncclProxyMsgTypeStr[type]);
  sock->fd = -1;
  return ret;
}
```

</details>

<a id="f1041"></a>

## F1041：proxyProgressInit

用途：创建/映射共享代理任务池并初始化跨进程锁、条件变量和空闲链，为数据提交准备接口。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyProgressState* state = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;opsPool == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 准备局部状态 | <code>int size = sizeof(struct ncclProxyOpsPool);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 准备局部状态 | <code>struct ncclProxyOpsPool* pool = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 准备局部状态 | <code>char shmPath[sizeof("/dev/shm/nccl-XXXXXX")];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>shmPath[0] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmOpen(shmPath, size, (void**)&pool, NULL, 1));</code> | 失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>pool-&gt;nextOps = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 循环 | <code>初始 ；继续条件 (comm-&gt;localRanks == 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>sched_yield();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 2 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;localRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 3 / 调用 / 计算 / 写入 | <code>pool-&gt;freeOps[r] = r*MAX_OPS_PER_PEER;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 3 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAX_OPS_PER_PEER-1；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>pool-&gt;ops[r*MAX_OPS_PER_PEER+i].next = r*MAX_OPS_PER_PEER+i+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>pool-&gt;ops[(r+1)*MAX_OPS_PER_PEER-1].next = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 20 | 2 / 准备局部状态 | <code>pthread_mutexattr_t mutexAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutexattr_init(&mutexAttr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_init(&pool-&gt;mutex, &mutexAttr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 2 / 准备局部状态 | <code>pthread_condattr_t condAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>pthread_cond_init(&pool-&gt;cond, &condAttr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>state-&gt;opsPool = pool;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>memcpy(state-&gt;opsPoolShmSuffix, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof("XXXXXX")-1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyProgressCreate(comm));</code> | 失败时可能提前return，成功才继续；按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。 |
| 30 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (state->opsPool == NULL) {
    int size = sizeof(struct ncclProxyOpsPool);
    struct ncclProxyOpsPool* pool = NULL;

    char shmPath[sizeof("/dev/shm/nccl-XXXXXX")];
    shmPath[0] = '\0';
    NCCLCHECK(ncclShmOpen(shmPath, size, (void**)&pool, NULL, 1));

    // Init pool
    pool->nextOps = -1;

    // The service thread may be launched already but localRanks may not be set yet.
    while (comm->localRanks == 0) sched_yield();

    for (int r=0; r<comm->localRanks; r++) {
      pool->freeOps[r] = r*MAX_OPS_PER_PEER;
      for (int i=0; i<MAX_OPS_PER_PEER-1; i++) pool->ops[r*MAX_OPS_PER_PEER+i].next = r*MAX_OPS_PER_PEER+i+1;
      pool->ops[(r+1)*MAX_OPS_PER_PEER-1].next = -1;
    }

    // Setup mutex/cond to work inter-process
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&pool->mutex, &mutexAttr);
    pthread_condattr_t condAttr;
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&pool->cond, &condAttr);
    state->opsPool = pool;

    memcpy(state->opsPoolShmSuffix, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof("XXXXXX")-1);

    // All ops structures are created, we can start the progress thread
    NCCLCHECK(ncclProxyProgressCreate(comm));
  }
  return ncclSuccess;
}
```

</details>

<a id="f1042"></a>

## F1042：proxyOpsFree

用途：回收当前 communicator 映射/分配的代理任务池视图，不混同于单条数据请求完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyProgressState* state = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(ncclShmClose(state-&gt;opsPool, NULL, sizeof(struct ncclProxyOpsPool)) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("[Service thread] shm close failed");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (ncclShmClose(state->opsPool, NULL, sizeof(struct ncclProxyOpsPool)) != ncclSuccess) {
    WARN("[Service thread] shm close failed");
  }
}
```

</details>

<a id="f1043"></a>

## F1043：ncclProxyShmUnlink

用途：移除代理共享内存对象名字，已有映射仍按各自引用和退出流程释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyProgressState* state = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;opsPool == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>char shmPath[] = "/dev/shm/nccl-XXXXXX";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>memcpy(shmPath+sizeof("/dev/shm/nccl-")-1, state-&gt;opsPoolShmSuffix, sizeof("XXXXXX")-1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(ncclShmUnlink(shmPath) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>WARN("[Service thread] shm unlink failed");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (state->opsPool == NULL) return ncclSuccess;

  char shmPath[] = "/dev/shm/nccl-XXXXXX";
  memcpy(shmPath+sizeof("/dev/shm/nccl-")-1, state->opsPoolShmSuffix, sizeof("XXXXXX")-1);
  if (ncclShmUnlink(shmPath) != ncclSuccess) {
    WARN("[Service thread] shm unlink failed");
  }
  return ncclSuccess;
}
```

</details>

<a id="f1044"></a>

## F1044：proxyConnInit

用途：处理服务端初始化消息，建立 peer 到连接池中 transport 对象的关联。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocket* sock = &peer-&gt;sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int id;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclProxyConnection* connection;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyNewConnection(connectionPool, &id));</code> | 失败时可能提前return，成功才继续；在分块连接池分配一个空连接槽，返回可用于控制消息索引的连接编号。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyGetConnection(connectionPool, id, &connection));</code> | 失败时可能提前return，成功才继续；按池编号定位已存在代理连接对象，校验池/索引后返回其地址。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;sock = sock;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &connection-&gt;transport, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &connection-&gt;send, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &peer-&gt;localRank, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;localRank = peer-&gt;localRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(sock, &connection, sizeof(void*)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;tcomm = connection-&gt;send ? &ncclTransports[connection-&gt;transport].send : &ncclTransports[connection-&gt;transport].recv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 条件判断 | <code>(connection-&gt;tcomm-&gt;proxyProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(proxyProgressInit(comm));</code> | 失败时可能提前return，成功才继续；创建/映射共享代理任务池并初始化跨进程锁、条件变量和空闲链，为数据提交准备接口。 |
| 16 | 2 / 准备局部状态 | <code>struct ncclProxyProgressState* state = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(sock, state-&gt;opsPoolShmSuffix, sizeof("XXXXXX")-1));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET, "New proxy %s connection %d from local rank %d, transport %d", connection-&gt;send ? "send":"recv", id, connection-&gt;localRank, connection-&gt;transport);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocket* sock = &peer->sock;
  int id;
  struct ncclProxyConnection* connection;
  NCCLCHECK(ncclProxyNewConnection(connectionPool, &id));
  NCCLCHECK(ncclProxyGetConnection(connectionPool, id, &connection));
  connection->sock = sock;
  NCCLCHECK(ncclSocketRecv(sock, &connection->transport, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &connection->send, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &peer->localRank, sizeof(int)));
  connection->localRank = peer->localRank;
  NCCLCHECK(ncclSocketSend(sock, &connection, sizeof(void*)));
  connection->tcomm = connection->send ? &ncclTransports[connection->transport].send : &ncclTransports[connection->transport].recv;
  // If we need proxy progress, let's allocate ops and start the thread
  if (connection->tcomm->proxyProgress) {
    NCCLCHECK(proxyProgressInit(comm));
    struct ncclProxyProgressState* state = &comm->proxyState.progressState;
    NCCLCHECK(ncclSocketSend(sock, state->opsPoolShmSuffix, sizeof("XXXXXX")-1));
  }
  INFO(NCCL_NET, "New proxy %s connection %d from local rank %d, transport %d", connection->send ? "send":"recv", id, connection->localRank, connection->transport);
  return ncclSuccess;
}
```

</details>

<a id="f1045"></a>

## F1045：proxyConnSharedInit

用途：处理共享资源初始化请求，为该代理和 peer 的 P2P 共享缓冲/队列建立基础容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocket* sock = &peer-&gt;sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclProxyConnection* connection;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &connection, sizeof(void*)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 4 | 0 / 准备局部状态 | <code>int reqSize, respSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &reqSize, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &respSize, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 7 | 0 / 条件判断 | <code>(reqSize != sizeof(int) &#124;&#124; respSize != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 准备局部状态 | <code>int nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &nChannels, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 12 | 0 / 条件判断 | <code>(connection-&gt;tcomm-&gt;proxySharedInit)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(connection-&gt;tcomm-&gt;proxySharedInit(connection, comm, nChannels));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocket* sock = &peer->sock;
  struct ncclProxyConnection* connection;
  NCCLCHECK(ncclSocketRecv(sock, &connection, sizeof(void*)));
  int reqSize, respSize;
  NCCLCHECK(ncclSocketRecv(sock, &reqSize, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &respSize, sizeof(int)));
  if (reqSize != sizeof(int) || respSize != 0) return ncclInternalError;
  int nChannels;
  NCCLCHECK(ncclSocketRecv(sock, &nChannels, sizeof(int)));
  if (connection->tcomm->proxySharedInit) NCCLCHECK(connection->tcomm->proxySharedInit(connection, comm, nChannels));
  return ncclSuccess;
}
```

</details>

<a id="f1046"></a>

## F1046：proxyProgressAsync

用途：推进一个尚未完成的代理控制请求，transport 声明 done 后才组织回复并移出待完成集合。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 2 | 0 / 条件判断 | <code>(op-&gt;type == ncclProxyMsgSetup)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(op-&gt;connection-&gt;tcomm-&gt;proxySetup(op-&gt;connection, comm, op-&gt;reqBuff, op-&gt;reqSize, op-&gt;respBuff, op-&gt;respSize, &done));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 6 | 2 / 条件判断 | <code>(op-&gt;type == ncclProxyMsgConnect)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(op-&gt;connection-&gt;tcomm-&gt;proxyConnect(op-&gt;connection, comm, op-&gt;reqBuff, op-&gt;reqSize, op-&gt;respBuff, op-&gt;respSize, &done));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 10 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 条件判断 | <code>(done)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 条件判断 | <code>(op-&gt;respSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(op-&gt;connection-&gt;sock, op-&gt;respBuff, op-&gt;respSize));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 16 | 2 / 条件判断 | <code>(op-&gt;reqBuff)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 调用 / 计算 / 写入 | <code>free(op-&gt;reqBuff);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 2 / 条件判断 | <code>(op-&gt;respBuff)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>free(op-&gt;respBuff);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>op-&gt;reqBuff = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>op-&gt;respBuff = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>op-&gt;type = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>(*asyncOpCount)--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int done = 1;
  if (op->type == ncclProxyMsgSetup) {
    NCCLCHECK(op->connection->tcomm->proxySetup(op->connection, comm, op->reqBuff, op->reqSize, op->respBuff, op->respSize, &done));
  } else if (op->type == ncclProxyMsgConnect) {
    NCCLCHECK(op->connection->tcomm->proxyConnect(op->connection, comm, op->reqBuff, op->reqSize, op->respBuff, op->respSize, &done));
  } else return ncclInternalError;
  if (done) {
    if (op->respSize) NCCLCHECK(ncclSocketSend(op->connection->sock, op->respBuff, op->respSize));
    if (op->reqBuff) free(op->reqBuff);
    if (op->respBuff) free(op->respBuff);
    op->reqBuff = NULL;
    op->respBuff = NULL;
    op->type = 0;
    (*asyncOpCount)--;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1047"></a>

## F1047：proxyConnSetupConnect

用途：接收 setup/connect 请求参数并开始执行对应 transport 回调，支持回调异步多轮完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocket* sock = &peer-&gt;sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclProxyAsyncOp* asyncOp = &peer-&gt;asyncOps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>asyncOp-&gt;type = type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &asyncOp-&gt;connection, sizeof(void*)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &asyncOp-&gt;reqSize, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &asyncOp-&gt;respSize, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 7 | 0 / 条件判断 | <code>(asyncOp-&gt;reqSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&asyncOp-&gt;reqBuff, asyncOp-&gt;reqSize));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, asyncOp-&gt;reqBuff, asyncOp-&gt;reqSize));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 11 | 0 / 条件判断 | <code>(asyncOp-&gt;respSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&asyncOp-&gt;respBuff, asyncOp-&gt;respSize));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>(*asyncOpCount)++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(proxyProgressAsync(asyncOp, comm, asyncOpCount));</code> | 失败时可能提前return，成功才继续；推进一个尚未完成的代理控制请求，transport 声明 done 后才组织回复并移出待完成集合。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocket* sock = &peer->sock;
  struct ncclProxyAsyncOp* asyncOp = &peer->asyncOps;
  asyncOp->type = type;
  NCCLCHECK(ncclSocketRecv(sock, &asyncOp->connection, sizeof(void*)));

  NCCLCHECK(ncclSocketRecv(sock, &asyncOp->reqSize, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &asyncOp->respSize, sizeof(int)));
  if (asyncOp->reqSize) {
    NCCLCHECK(ncclCalloc(&asyncOp->reqBuff, asyncOp->reqSize));
    NCCLCHECK(ncclSocketRecv(sock, asyncOp->reqBuff, asyncOp->reqSize));
  }
  if (asyncOp->respSize) NCCLCHECK(ncclCalloc(&asyncOp->respBuff, asyncOp->respSize));
  (*asyncOpCount)++;
  NCCLCHECK(proxyProgressAsync(asyncOp, comm, asyncOpCount));
  return ncclSuccess;
}
```

</details>

<a id="f1048"></a>

## F1048：ncclProxyService

用途：监听并处理代理控制 socket 的新连接、初始化、setup/connect/free/停止请求，与数据面循环分工。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclComm* comm = (struct ncclComm *) _args;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(cudaSetDevice(comm-&gt;cudaDev) != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("[Proxy Service] Failed to set CUDA device %d", comm-&gt;cudaDev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 条件判断 | <code>(CPU_COUNT(&comm-&gt;cpuAffinity))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>sched_setaffinity(0, sizeof(cpu_set_t), &comm-&gt;cpuAffinity);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 准备局部状态 | <code>struct ncclProxyConnectionPool connectionPool;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>connectionPool.pools = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>connectionPool.banks = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>connectionPool.offset = NCCL_PROXY_CONN_POOL_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 准备局部状态 | <code>struct pollfd pollfds[NCCL_MAX_LOCAL_RANKS+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>struct ncclProxyLocalPeer peers[NCCL_MAX_LOCAL_RANKS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>memset(&peers, 0, sizeof(struct ncclProxyLocalPeer)*NCCL_MAX_LOCAL_RANKS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;NCCL_MAX_LOCAL_RANKS；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 1 / 调用 / 计算 / 写入 | <code>peers[s].sock.fd = pollfds[s].fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 调用 / 计算 / 写入 | <code>pollfds[s].events = POLLHUP&#124;POLLIN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>pollfds[NCCL_MAX_LOCAL_RANKS].fd = comm-&gt;proxyState.listenSock-&gt;fd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>pollfds[NCCL_MAX_LOCAL_RANKS].events = POLLIN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 准备局部状态 | <code>int maxnpeers = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 0 / 准备局部状态 | <code>int npeers = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 0 / 准备局部状态 | <code>int stop = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 0 / 准备局部状态 | <code>int asyncOpCount = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 0 / 循环 | <code>初始 ；继续条件 (stop == 0 &#124;&#124; (stop == 1 && npeers &gt; 0))；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 26 | 1 / 条件判断 | <code>(int error = poll(pollfds, NCCL_MAX_LOCAL_RANKS+1, asyncOpCount ? 0 : -1) &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>WARN("[Proxy Service] Poll failed with error %d", error);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 3 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 30 | 1 / 条件判断 | <code>(pollfds[NCCL_MAX_LOCAL_RANKS].revents)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 3 / 准备局部状态 | <code>int s = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 3 / 循环 | <code>初始 ；继续条件 (s &lt; NCCL_MAX_LOCAL_RANKS && peers[s].sock.fd != -1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 34 | 4 / 调用 / 计算 / 写入 | <code>s++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 36 | 3 / 条件判断 | <code>(s == NCCL_MAX_LOCAL_RANKS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 5 / 调用 / 计算 / 写入 | <code>WARN("[Proxy service] Too many connections (%d max)", NCCL_MAX_LOCAL_RANKS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 5 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 40 | 3 / 条件判断 | <code>(maxnpeers &lt; s+1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 5 / 调用 / 计算 / 写入 | <code>maxnpeers = s+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 3 / 准备局部状态 | <code>struct ncclSocket* sock = &peers[s].sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 3 / 条件判断 | <code>(ncclSocketAccept(sock, comm-&gt;proxyState.listenSock) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 5 / 调用 / 计算 / 写入 | <code>WARN("[Service thread] Accept failed %s", strerror(errno));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 48 | 5 / 调用 / 计算 / 写入 | <code>pollfds[s].fd = sock-&gt;fd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 5 / 调用 / 计算 / 写入 | <code>npeers++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 5 / 调用 / 计算 / 写入 | <code>peers[s].localRank = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 1 / 循环 | <code>初始 int s=0;；继续条件 s&lt;maxnpeers；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 52 | 2 / 准备局部状态 | <code>struct ncclProxyLocalPeer* peer = peers+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 53 | 2 / 准备局部状态 | <code>struct ncclSocket* sock = &peer-&gt;sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 2 / 准备局部状态 | <code>struct ncclProxyAsyncOp* op = &peer-&gt;asyncOps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 55 | 2 / 准备局部状态 | <code>int closeConn = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 2 / 准备局部状态 | <code>int type = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 57 | 2 / 准备局部状态 | <code>ncclResult_t res = ncclSuccess;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 58 | 2 / 条件判断 | <code>(op-&gt;type != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 4 / 调用 / 计算 / 写入 | <code>res = proxyProgressAsync(op, comm, &asyncOpCount);</code> | 推进一个尚未完成的代理控制请求，transport 声明 done 后才组织回复并移出待完成集合。 |
| 61 | 4 / 调用 / 计算 / 写入 | <code>type = op-&gt;type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 4 / 条件判断 | <code>(res != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 6 / 调用 / 计算 / 写入 | <code>op-&gt;type = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 65 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 66 | 4 / 条件判断 | <code>(pollfds[s].revents & POLLIN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 67 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 68 | 6 / 准备局部状态 | <code>int closed;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 69 | 6 / 条件判断 | <code>(ncclSocketTryRecv(sock, &type, sizeof(int), &closed) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 70 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 71 | 8 / 调用 / 计算 / 写入 | <code>WARN("[Service thread] Could not receive type from localRank %d", peer-&gt;localRank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 72 | 8 / 调用 / 计算 / 写入 | <code>closeConn = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 74 | 8 / 条件判断 | <code>(closed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 75 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 76 | 10 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "[Service thread] Connection closed by localRank %d", peer-&gt;localRank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 77 | 10 / 调用 / 计算 / 写入 | <code>closeConn = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 9 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 79 | 10 / 条件判断 | <code>(type == ncclProxyMsgAbort)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 80 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 81 | 12 / 调用 / 计算 / 写入 | <code>stop = 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 12 / 调用 / 计算 / 写入 | <code>closeConn = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 11 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 84 | 12 / 条件判断 | <code>(type == ncclProxyMsgStop)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 85 | 13 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 86 | 14 / 调用 / 计算 / 写入 | <code>stop = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 87 | 14 / 调用 / 计算 / 写入 | <code>closeConn = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 88 | 13 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 89 | 14 / 条件判断 | <code>(type == ncclProxyMsgClose)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 90 | 15 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 91 | 16 / 调用 / 计算 / 写入 | <code>closeConn = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 92 | 15 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 93 | 16 / 条件判断 | <code>(type == ncclProxyMsgInit)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 94 | 17 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 95 | 18 / 调用 / 计算 / 写入 | <code>res = proxyConnInit(peers+s, &connectionPool, comm);</code> | 处理服务端初始化消息，建立 peer 到连接池中 transport 对象的关联。 |
| 96 | 17 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 97 | 18 / 条件判断 | <code>(type == ncclProxyMsgSharedInit)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 98 | 19 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 99 | 20 / 调用 / 计算 / 写入 | <code>res = proxyConnSharedInit(peers+s, &connectionPool, comm);</code> | 处理共享资源初始化请求，为该代理和 peer 的 P2P 共享缓冲/队列建立基础容量。 |
| 100 | 19 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 101 | 20 / 条件判断 | <code>(type == ncclProxyMsgSetup &#124;&#124; type == ncclProxyMsgConnect)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 102 | 21 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 103 | 22 / 调用 / 计算 / 写入 | <code>res = proxyConnSetupConnect(type, peers+s, &connectionPool, comm, &asyncOpCount);</code> | 接收 setup/connect 请求参数并开始执行对应 transport 回调，支持回调异步多轮完成。 |
| 104 | 21 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 105 | 22 / 调用 / 计算 / 写入 | <code>WARN("[Service thread] Unknown command %d from localRank %d\n", type, peer-&gt;localRank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 106 | 22 / 调用 / 计算 / 写入 | <code>closeConn = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 107 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 108 | 6 / 条件判断 | <code>(pollfds[s].revents & POLLHUP)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 109 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 110 | 8 / 调用 / 计算 / 写入 | <code>closeConn = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 111 | 2 / 条件判断 | <code>(res != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 112 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 113 | 4 / 调用 / 计算 / 写入 | <code>WARN("[Proxy Service %d] Failed to execute operation %s from rank %d, retcode %d", comm-&gt;rank, ncclProxyMsgTypeStr[type], comm-&gt;localRankToRank[peer-&gt;localRank], res);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 114 | 4 / 调用 / 计算 / 写入 | <code>closeConn = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 115 | 2 / 条件判断 | <code>(closeConn)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 116 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 117 | 4 / 调用 / 计算 / 写入 | <code>close(sock-&gt;fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 118 | 4 / 调用 / 计算 / 写入 | <code>sock-&gt;fd = pollfds[s].fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 119 | 4 / 调用 / 计算 / 写入 | <code>npeers--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 120 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 121 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 122 | 0 / 条件判断 | <code>(ncclProxyProgressDestroy(comm) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 123 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 124 | 2 / 调用 / 计算 / 写入 | <code>WARN("[Proxy Service] proxyDestroy failed");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 125 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;maxnpeers；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 126 | 1 / 条件判断 | <code>(peers[s].sock.fd != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 127 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 128 | 3 / 调用 / 计算 / 写入 | <code>close(peers[s].sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 129 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 130 | 0 / 调用 / 计算 / 写入 | <code>ncclProxyFreeConnections(&connectionPool, comm);</code> | 遍历代理连接池释放全部已分配连接及池块，避免退出后残留后台资源。 |
| 131 | 0 / 调用 / 计算 / 写入 | <code>close(comm-&gt;proxyState.listenSock-&gt;fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 132 | 0 / 调用 / 计算 / 写入 | <code>free(comm-&gt;proxyState.listenSock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 133 | 0 / 调用 / 计算 / 写入 | <code>proxyOpsFree(comm);</code> | 回收当前 communicator 映射/分配的代理任务池视图，不混同于单条数据请求完成。 |
| 134 | 0 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclComm* comm =  (struct ncclComm *) _args;
  if (cudaSetDevice(comm->cudaDev) != cudaSuccess) {
    WARN("[Proxy Service] Failed to set CUDA device %d", comm->cudaDev);
  }
  if (CPU_COUNT(&comm->cpuAffinity)) sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);

  // Prepare poll descriptor
  struct ncclProxyConnectionPool connectionPool;
  connectionPool.pools = NULL;
  connectionPool.banks = 0;
  connectionPool.offset = NCCL_PROXY_CONN_POOL_SIZE;

  struct pollfd pollfds[NCCL_MAX_LOCAL_RANKS+1];
  struct ncclProxyLocalPeer peers[NCCL_MAX_LOCAL_RANKS];
  memset(&peers, 0, sizeof(struct ncclProxyLocalPeer)*NCCL_MAX_LOCAL_RANKS);
  for (int s=0; s<NCCL_MAX_LOCAL_RANKS; s++) {
    peers[s].sock.fd = pollfds[s].fd = -1;
    pollfds[s].events = POLLHUP|POLLIN;
  }
  pollfds[NCCL_MAX_LOCAL_RANKS].fd = comm->proxyState.listenSock->fd;
  pollfds[NCCL_MAX_LOCAL_RANKS].events = POLLIN;

  int maxnpeers = 0;
  int npeers = 0;
  int stop = 0;
  int asyncOpCount = 0;
  while (stop == 0 || (stop == 1 && npeers > 0)) {
    if (int error = poll(pollfds, NCCL_MAX_LOCAL_RANKS+1, asyncOpCount ? 0 : -1) < 0) {
      WARN("[Proxy Service] Poll failed with error %d", error);
      return NULL;
    }
    if (pollfds[NCCL_MAX_LOCAL_RANKS].revents) {
      int s = 0;
      while (s < NCCL_MAX_LOCAL_RANKS && peers[s].sock.fd != -1) s++;
      if (s == NCCL_MAX_LOCAL_RANKS) {
        WARN("[Proxy service] Too many connections (%d max)", NCCL_MAX_LOCAL_RANKS);
        return NULL;
      }
      if (maxnpeers < s+1) maxnpeers = s+1;
      struct ncclSocket* sock = &peers[s].sock;
      if (ncclSocketAccept(sock, comm->proxyState.listenSock) != ncclSuccess) {
        WARN("[Service thread] Accept failed %s", strerror(errno));
      } else {
        pollfds[s].fd = sock->fd;
        npeers++;
        peers[s].localRank = -1;
      }
    }
    for (int s=0; s<maxnpeers; s++) {
      struct ncclProxyLocalPeer* peer = peers+s;
      struct ncclSocket* sock = &peer->sock;
      struct ncclProxyAsyncOp* op = &peer->asyncOps;
      int closeConn = 0;
      int type = 0;
      ncclResult_t res = ncclSuccess;
      if (op->type != 0) {
        res = proxyProgressAsync(op, comm, &asyncOpCount);
        type = op->type;
        if (res != ncclSuccess) op->type = 0;
      } else if (pollfds[s].revents & POLLIN) {
        int closed;
        if (ncclSocketTryRecv(sock, &type, sizeof(int), &closed) != ncclSuccess) {
          WARN("[Service thread] Could not receive type from localRank %d", peer->localRank);
          closeConn = 1;
        } else if (closed) {
          INFO(NCCL_INIT|NCCL_NET, "[Service thread] Connection closed by localRank %d", peer->localRank);
          closeConn = 1;
        } else {
          if (type == ncclProxyMsgAbort) {
            stop = 2;
            closeConn = 1;
          } else if (type == ncclProxyMsgStop) {
            stop = 1;
            closeConn = 1;
          } else if (type == ncclProxyMsgClose) {
            closeConn = 1;
          } else if (type == ncclProxyMsgInit) {
            res = proxyConnInit(peers+s, &connectionPool, comm);
          } else if (type == ncclProxyMsgSharedInit) {
            res = proxyConnSharedInit(peers+s, &connectionPool, comm);
          } else if (type == ncclProxyMsgSetup || type == ncclProxyMsgConnect) {
            res = proxyConnSetupConnect(type, peers+s, &connectionPool, comm, &asyncOpCount);
          } else {
            WARN("[Service thread] Unknown command %d from localRank %d\n", type, peer->localRank);
            closeConn = 1;
          }
        }
      } else if (pollfds[s].revents & POLLHUP) {
        closeConn = 1;
      } 
      if (res != ncclSuccess) {
        WARN("[Proxy Service %d] Failed to execute operation %s from rank %d, retcode %d", comm->rank, ncclProxyMsgTypeStr[type], comm->localRankToRank[peer->localRank], res);
        closeConn = 1;
      }
      if (closeConn) {
        close(sock->fd);
        sock->fd = pollfds[s].fd = -1;
        npeers--;
      }
    }
  }
  // Wait for all operations to complete and stop progress thread before freeing any resource
  if (ncclProxyProgressDestroy(comm) != ncclSuccess) {
    WARN("[Proxy Service] proxyDestroy failed");
  }
  for (int s=0; s<maxnpeers; s++) {
    if (peers[s].sock.fd != -1) close(peers[s].sock.fd);
  }
  ncclProxyFreeConnections(&connectionPool, comm);
  close(comm->proxyState.listenSock->fd);
  free(comm->proxyState.listenSock);
  proxyOpsFree(comm);
  return NULL;
}
```

</details>

<a id="f1049"></a>

## F1049：ncclProxyInit

用途：将 bootstrap 建立的监听 socket 和全组代理地址表接入当前 communicator 的 Proxy 状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;proxyState.listenSock = sock;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;proxyState.peerAddresses = peerAddresses;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  comm->proxyState.listenSock = sock;
  comm->proxyState.peerAddresses = peerAddresses;
  return ncclSuccess;
}
```

</details>

<a id="f1050"></a>

## F1050：ncclProxyCreate

用途：按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>pthread_create(&comm-&gt;proxyState.thread, NULL, ncclProxyService, comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>ncclSetThreadName(comm-&gt;proxyState.thread, "NCCL Service %2d", comm-&gt;cudaDev);</code> | 为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // comm->proxyState.thread is pthread_join()'d by commFree() in init.cc
  pthread_create(&comm->proxyState.thread, NULL, ncclProxyService, comm);
  ncclSetThreadName(comm->proxyState.thread, "NCCL Service %2d", comm->cudaDev);
  return ncclSuccess;
}
```

</details>

<a id="f1051"></a>

## F1051：ncclProxyDestroy

用途：向代理服务发送停止/关闭信号并等待服务线程结束，完成代理总生命周期清理。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxyState* state = &comm-&gt;proxyState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;peerAddresses)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 准备局部状态 | <code>struct ncclSocket sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>sock.abortFlag = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>sock.asyncFlag = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>memcpy(&sock.addr, comm-&gt;proxyState.peerAddresses+comm-&gt;rank, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketConnect(&sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。 |
| 9 | 2 / 准备局部状态 | <code>int type = (*comm-&gt;abortFlag) ? ncclProxyMsgAbort : ncclProxyMsgStop;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(&sock, &type, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>close(sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>free(state-&gt;peerAddresses);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 条件判断 | <code>(state-&gt;peerSocks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;localRanks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 3 / 条件判断 | <code>(state-&gt;peerSocks[i].fd != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 5 / 条件判断 | <code>(state-&gt;proxyOps[i].pool)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmClose(state-&gt;proxyOps[i].pool, NULL, sizeof(struct ncclProxyOpsPool)));</code> | 失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。 |
| 21 | 5 / 条件判断 | <code>(state-&gt;sharedDevMems[i])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 7 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcCloseMemHandle(state-&gt;sharedDevMems[i]));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 24 | 5 / 准备局部状态 | <code>int type = ncclProxyMsgClose;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 5 / 条件判断 | <code>(*comm-&gt;abortFlag == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(state-&gt;peerSocks+i, &type, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 28 | 5 / 调用 / 计算 / 写入 | <code>close(state-&gt;peerSocks[i].fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>free(state-&gt;peerSocks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 2 / 调用 / 计算 / 写入 | <code>free(state-&gt;proxyOps);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>free(state-&gt;sharedDevMems);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxyState* state = &comm->proxyState;
  if (state->peerAddresses) {
    struct ncclSocket sock;
    sock.abortFlag = NULL;
    sock.asyncFlag = 0;
    memcpy(&sock.addr, comm->proxyState.peerAddresses+comm->rank, sizeof(union ncclSocketAddress));
    NCCLCHECK(ncclSocketConnect(&sock));
    int type = (*comm->abortFlag) ? ncclProxyMsgAbort : ncclProxyMsgStop;
    NCCLCHECK(ncclSocketSend(&sock, &type, sizeof(int)));
    close(sock.fd);
    free(state->peerAddresses);
  }
  if (state->peerSocks) {
    for (int i=0; i<comm->localRanks; i++) {
      if (state->peerSocks[i].fd != -1) {
        if (state->proxyOps[i].pool) {
          NCCLCHECK(ncclShmClose(state->proxyOps[i].pool, NULL, sizeof(struct ncclProxyOpsPool)));
        }
        if (state->sharedDevMems[i]) {
          CUDACHECK(cudaIpcCloseMemHandle(state->sharedDevMems[i]));
        }
        int type = ncclProxyMsgClose;
        if (*comm->abortFlag == 0) NCCLCHECK(ncclSocketSend(state->peerSocks+i, &type, sizeof(int)));
        close(state->peerSocks[i].fd);
      }
    }
    free(state->peerSocks);
    free(state->proxyOps);
    free(state->sharedDevMems);
  }
  return ncclSuccess;
}
```

</details>

