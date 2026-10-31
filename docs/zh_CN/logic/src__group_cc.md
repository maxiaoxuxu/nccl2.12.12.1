# src/group.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0589"></a>

## F0589：ncclAsyncMode

用途：查询本线程是否处于 Group 嵌套内部，决定 API 立即准备还是延迟到最外层 GroupEnd。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclGroupMode &gt; 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return ncclGroupMode > 0;
}
```

</details>

<a id="f0590"></a>

## F0590：ncclAsyncErrCheck

用途：把组内调用错误记录到线程局部 Group 状态，供统一清理和返回。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ncclGroupError == ncclSuccess &#124;&#124; ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>ncclGroupError = ret;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ret;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ncclGroupError == ncclSuccess || ret != ncclSuccess) ncclGroupError = ret;
  return ret;
}
```

</details>

<a id="f0591"></a>

## F0591：ncclAsyncThreadMain

用途：后台执行一项延迟 communicator 初始化，并把结果存回组任务参数。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKTHREAD(args-&gt;init.func(args-&gt;init.newcomm, args-&gt;init.ndev, args-&gt;init.commId, args-&gt;init.myrank, args-&gt;init.cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 函数返回 | <code>return args;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;
  NCCLCHECKTHREAD(args->init.func(args->init.newcomm, args->init.ndev, args->init.commId, args->init.myrank, args->init.cudaDev));
  return args;
}
```

</details>

<a id="f0592"></a>

## F0592：ncclAsyncInit

用途：把初始化函数、rank、设备和 uniqueId 复制到本线程 Group 任务数组。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ncclGroupIndex &gt;= MAX_ASYNC_OPS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclAsyncErrCheck(ncclInvalidUsage);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int index = ncclGroupIndex++;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;funcType = ASYNC_FUNC_INIT;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;init.func = func;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;init.cudaDev = cudaDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;init.newcomm = newcomm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;init.ndev = ndev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&args-&gt;init.commId, &commId, sizeof(commId));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;init.myrank = myrank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ncclGroupIndex >= MAX_ASYNC_OPS) {
    WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);
    return ncclAsyncErrCheck(ncclInvalidUsage);
  }
  int index = ncclGroupIndex++;
  struct ncclAsyncArgs* args = ncclGroupArgs+index;
  args->funcType = ASYNC_FUNC_INIT;
  args->init.func = func;
  args->init.cudaDev = cudaDev;
  args->init.newcomm = newcomm;
  args->init.ndev = ndev;
  memcpy(&args->init.commId, &commId, sizeof(commId));
  args->init.myrank = myrank;
  return ncclSuccess;
}
```

</details>

<a id="f0593"></a>

## F0593：ncclAsyncColl

用途：把 communicator 加入本批 Group 提交集合，重复遇到相同 comm 时避免多次登记。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(args-&gt;coll.comm == comm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>args++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 条件判断 | <code>(ncclGroupIndex &gt;= MAX_ASYNC_OPS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 函数返回 | <code>return ncclAsyncErrCheck(ncclInvalidUsage);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>ncclGroupIndex++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;funcType = ASYNC_FUNC_COLL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;coll.comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclAsyncArgs* args = ncclGroupArgs;
  for (int i=0; i<ncclGroupIndex; i++) {
    if (args->coll.comm == comm) return ncclSuccess;
    args++;
  }
  if (ncclGroupIndex >= MAX_ASYNC_OPS) {
    WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);
    return ncclAsyncErrCheck(ncclInvalidUsage);
  }
  ncclGroupIndex++;
  args->funcType = ASYNC_FUNC_COLL;
  args->coll.comm = comm;
  return ncclSuccess;
}
```

</details>

<a id="f0594"></a>

## F0594：ncclGroupStart

用途：进入线程局部 Group 嵌套，最外层首次进入时清理组任务状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 条件判断 | <code>(ncclGroupMode == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>memset(ncclGroupArgs, 0, sizeof(struct ncclAsyncArgs)*MAX_ASYNC_OPS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>ncclGroupMode++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (ncclGroupMode == 0) {
    memset(ncclGroupArgs, 0, sizeof(struct ncclAsyncArgs)*MAX_ASYNC_OPS);
  }
  ncclGroupMode++;
  return ncclSuccess;
}
```

</details>

<a id="f0595"></a>

## F0595：scheduleSend

用途：把 Group 切出的字节块封装为 P2P 任务，按 peer/块号映射 channel 后调用 SetupP2pKernel。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncSend, "Send", NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm-&gt;userStream, 1, 1 };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm-&gt;p2pnChannelsPerPeer, ncclFuncSend, &channelId));</code> | 失败时可能提前return，成功才继续；组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>info.channelId = channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSetupP2pKernel(&info));</code> | 失败时可能提前return，成功才继续；为 P2P 块生成设备/Proxy 计划并更新发射所需的线程和 channel 范围。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclInfo info = { ncclFuncSend, "Send",
    NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, /* Args */
    1, 1 };
  int channelId;
  NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncSend, &channelId));
  info.channelId = channelId;
  NCCLCHECK(ncclSetupP2pKernel(&info));
  return ncclSuccess;
}
```

</details>

<a id="f0596"></a>

## F0596：scheduleRecv

用途：把 Group 切出的字节块封装为 P2P 任务，按 peer/块号映射 channel 后调用 SetupP2pKernel。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncRecv, "Recv", NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm-&gt;userStream, 1, 1 };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm-&gt;p2pnChannelsPerPeer, ncclFuncRecv, &channelId));</code> | 失败时可能提前return，成功才继续；组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>info.channelId = channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSetupP2pKernel(&info));</code> | 失败时可能提前return，成功才继续；为 P2P 块生成设备/Proxy 计划并更新发射所需的线程和 channel 范围。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclInfo info = { ncclFuncRecv, "Recv",
    NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, /* Args */
    1, 1 };
  int channelId;
  NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncRecv, &channelId));
  info.channelId = channelId;
  NCCLCHECK(ncclSetupP2pKernel(&info));
  return ncclSuccess;
}
```

</details>

<a id="f0597"></a>

## F0597：ncclAsyncThreadPreconnect

用途：在正确 CUDA 设备和 CPU 亲和性下完成本批 P2P 缺失连接的预建链。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclComm* comm = args-&gt;coll.comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECKTHREAD(cudaSetDevice(comm-&gt;cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 条件判断 | <code>(CPU_COUNT(&comm-&gt;cpuAffinity))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>sched_setaffinity(0, sizeof(cpu_set_t), &comm-&gt;cpuAffinity);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKTHREAD(ncclTransportP2pSetup(comm, NULL, 1));</code> | 失败时可能提前return，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。 |
| 8 | 0 / 函数返回 | <code>return args;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;
  struct ncclComm* comm = args->coll.comm;
  CUDACHECKTHREAD(cudaSetDevice(comm->cudaDev));
  if (CPU_COUNT(&comm->cpuAffinity)) sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);
  NCCLCHECKTHREAD(ncclTransportP2pSetup(comm, NULL, 1));
  return args;
}
```

</details>

<a id="f0598"></a>

## F0598：getP2pChunkSize

用途：在最小/最大通道预算内倍增并行度，将每 peer 消息划成按 step 对齐的块。

- 先按最小通道数估每块尺寸，超过最大块阈值时倍增通道数直到预算上限。
- 最后按minSize对齐，所以小消息不一定用满per-peer预算；粒度/并发和调度开销在这里折中。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>size_t size = std::max(minSize, DIVUP(totalSize, minChannels));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nChannels = minChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 ；继续条件 (size &gt; maxSize && nChannels &lt;= maxChannels/2)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>nChannels *= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>size = DIVUP(totalSize, nChannels);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>ALIGN_SIZE(size, minSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return size;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  size_t size = std::max(minSize, DIVUP(totalSize, minChannels));
  int nChannels = minChannels;
  while (size > maxSize && nChannels <= maxChannels/2) {
    nChannels *= 2;
    size = DIVUP(totalSize, nChannels);
  }
  ALIGN_SIZE(size, minSize);
  return size;
}
```

</details>

<a id="f0599"></a>

## F0599：ncclGroupEnd

用途：最外层出口统一执行初始化、P2P 预建链、消息分块、collective 聚合、Proxy 发布和 kernel 发射。

- 只有最外层GroupEnd真正提交；先汇总前序错误，再执行初始化线程与P2P预建链。
- 按双方一致的节点距离和局部rank步号取peer消息，再切块调度，0字节同步与无消息分开。
- 准备所有comm后分轮执行HostSetup/发射屏障、kernel发射、事件和reset；避免本地部分GPU永远等尚未启动的伙伴。
- 错误清理标签回收本批状态，不能把goto后的代码当正常顺序继续执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 条件判断 | <code>(ncclGroupMode == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("ncclGroupEnd: not in a group call.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>ncclGroupMode--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 条件判断 | <code>(ncclGroupMode &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 准备局部状态 | <code>int savedDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&savedDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 12 | 0 / 准备局部状态 | <code>int activeThreads = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>int doneArray[MAX_ASYNC_OPS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>doneArray[i] = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 17 | 0 / 准备局部状态 | <code>ncclResult_t ret = ncclGroupError;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 0 / 准备局部状态 | <code>int usingCudaGraphAll = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 0 / 准备局部状态 | <code>cudaGraph_t* graphs = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 0 / 条件判断 | <code>(ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 2 / 控制转移 | <code>goto group_cleanup;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 23 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_INIT)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadMain, args);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>activeThreads++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 3 / 调用 / 计算 / 写入 | <code>doneArray[i] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 0 / 循环 | <code>初始 ；继续条件 (activeThreads)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 32 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 33 | 2 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 2 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_INIT && doneArray[i] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 4 / 准备局部状态 | <code>int err = pthread_tryjoin_np(ncclGroupThreads[i], NULL);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 4 / 条件判断 | <code>(err == EBUSY)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 6 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 40 | 4 / 条件判断 | <code>(err != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 6 / 调用 / 计算 / 写入 | <code>ret = ncclSystemError;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 4 / 条件判断 | <code>(args-&gt;ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 6 / 调用 / 计算 / 写入 | <code>ret = args-&gt;ret;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 4 / 调用 / 计算 / 写入 | <code>doneArray[i] = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 4 / 调用 / 计算 / 写入 | <code>activeThreads--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 49 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 50 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 51 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_COLL && args-&gt;coll.comm-&gt;connect)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 3 / 调用 / 计算 / 写入 | <code>pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadPreconnect, args);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 55 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 56 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 57 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 58 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_COLL && args-&gt;coll.comm-&gt;connect)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 3 / 准备局部状态 | <code>int err = pthread_join(ncclGroupThreads[i], NULL);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 61 | 3 / 条件判断 | <code>(err != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 62 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 63 | 5 / 调用 / 计算 / 写入 | <code>WARN("Error waiting for pthread_join : %s", strerror(errno));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 64 | 5 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 65 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(args-&gt;ret, ret, end);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 66 | 3 / 调用 / 计算 / 写入 | <code>args-&gt;coll.comm-&gt;connect = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 68 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 69 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 70 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_COLL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 71 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 72 | 3 / 准备局部状态 | <code>struct ncclComm* comm = args-&gt;coll.comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 73 | 3 / 准备局部状态 | <code>int node = comm-&gt;node;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 74 | 3 / 准备局部状态 | <code>int nNodes = comm-&gt;nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 75 | 3 / 准备局部状态 | <code>int localRank = comm-&gt;localRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 76 | 3 / 准备局部状态 | <code>ssize_t stepSize = comm-&gt;buffSizes[NCCL_PROTO_SIMPLE] / NCCL_STEPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 77 | 3 / 准备局部状态 | <code>int nChannelsMax = comm-&gt;p2pnChannelsPerPeer;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 78 | 3 / 准备局部状态 | <code>int nChannelsMin = nChannelsMax;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 79 | 3 / 循环 | <code>初始 ；继续条件 (nChannelsMin*comm-&gt;nRanks &gt; comm-&gt;p2pnChannels && nChannelsMin &gt; 1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 80 | 4 / 调用 / 计算 / 写入 | <code>nChannelsMin /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 81 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 82 | 3 / 循环 | <code>初始 ；继续条件 (nChannelsMax*comm-&gt;nRanks &gt; comm-&gt;p2pnChannels*4 && nChannelsMax &gt; 1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 83 | 4 / 调用 / 计算 / 写入 | <code>nChannelsMax /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 85 | 3 / 循环 | <code>初始 ；继续条件 (comm-&gt;p2pSendCount &gt; 0 &#124;&#124; comm-&gt;p2pRecvCount &gt; 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 86 | 4 / 循环 | <code>初始 int d=0;；继续条件 d&lt;=nNodes/4；每轮更新 d++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 87 | 5 / 准备局部状态 | <code>int deltas[4] = { d, (nNodes-d)%nNodes, nNodes/2-d, (nNodes-(nNodes/2-d))%nNodes };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 88 | 5 / 准备局部状态 | <code>int index = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 89 | 5 / 准备局部状态 | <code>int delta = deltas[index];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 90 | 5 / 跳转标签 | <code>sched_delta</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 91 | 6 / 准备局部状态 | <code>uint32_t recvNode = (node+nNodes-delta)%nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 92 | 5 / 准备局部状态 | <code>uint32_t sendNode = (node+delta)%nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 93 | 5 / 准备局部状态 | <code>int steps = comm-&gt;maxLocalRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 94 | 5 / 循环 | <code>初始 int s=0;；继续条件 s&lt;steps；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 95 | 6 / 准备局部状态 | <code>int recvIndex = (localRank-s+steps)%steps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 96 | 6 / 准备局部状态 | <code>int recvPeer = recvIndex&lt;comm-&gt;nodeRanks[recvNode].localRanks ? comm-&gt;nodeRanks[recvNode].localRankToRank[recvIndex] : -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 97 | 6 / 准备局部状态 | <code>int sendIndex = (localRank+s)%steps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 98 | 6 / 准备局部状态 | <code>int sendPeer = sendIndex&lt;comm-&gt;nodeRanks[sendNode].localRanks ? comm-&gt;nodeRanks[sendNode].localRankToRank[sendIndex] : -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 99 | 6 / 准备局部状态 | <code>struct ncclP2Pinfo* recv = recvPeer != -1 && comm-&gt;p2pRecvs[recvPeer] ? comm-&gt;p2pRecvs[recvPeer]-&gt;getNext() : NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 100 | 6 / 准备局部状态 | <code>struct ncclP2Pinfo* send = sendPeer != -1 && comm-&gt;p2pSends[sendPeer] ? comm-&gt;p2pSends[sendPeer]-&gt;getNext() : NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 101 | 6 / 条件判断 | <code>(recv != NULL &#124;&#124; send != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 102 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 103 | 8 / 准备局部状态 | <code>ssize_t totRecvBytes = -1, totSendBytes = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 104 | 8 / 条件判断 | <code>(recv != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 105 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 106 | 10 / 调用 / 计算 / 写入 | <code>totRecvBytes = recv-&gt;nbytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 107 | 8 / 条件判断 | <code>(send != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 108 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 109 | 10 / 调用 / 计算 / 写入 | <code>totSendBytes = send-&gt;nbytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 110 | 8 / 条件判断 | <code>(recv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 111 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 112 | 10 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pRecvCount--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 113 | 8 / 条件判断 | <code>(send)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 114 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 115 | 10 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pSendCount--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 116 | 8 / 条件判断 | <code>(recvPeer == comm-&gt;rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 117 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 118 | 10 / 条件判断 | <code>(sendPeer != comm-&gt;rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 119 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 120 | 12 / 调用 / 计算 / 写入 | <code>WARN("Sendrecv schedule not aligned for self");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 121 | 12 / 调用 / 计算 / 写入 | <code>ret = ncclInternalError;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 122 | 12 / 控制转移 | <code>goto group_cleanup;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 123 | 10 / 条件判断 | <code>(send && recv == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 124 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 125 | 12 / 调用 / 计算 / 写入 | <code>WARN("Trying to send to self without a matching recv");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 126 | 12 / 调用 / 计算 / 写入 | <code>ret = ncclInvalidUsage;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 127 | 12 / 控制转移 | <code>goto group_cleanup;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 128 | 10 / 条件判断 | <code>(send == NULL && recv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 129 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 130 | 12 / 调用 / 计算 / 写入 | <code>WARN("Trying to recv to self without a matching send");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 131 | 12 / 调用 / 计算 / 写入 | <code>ret = ncclInvalidUsage;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 132 | 12 / 控制转移 | <code>goto group_cleanup;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 133 | 8 / 准备局部状态 | <code>void* recvBuff = recv ? recv-&gt;buff : NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 134 | 8 / 准备局部状态 | <code>void* sendBuff = send ? send-&gt;buff : NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 135 | 8 / 条件判断 | <code>(recv && comm-&gt;p2pRecvs[recvPeer]-&gt;peakNext() == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 136 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 137 | 10 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pRecvs[recvPeer]-&gt;recycle();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 138 | 8 / 条件判断 | <code>(send && comm-&gt;p2pSends[sendPeer]-&gt;peakNext() == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 139 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 140 | 10 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pSends[sendPeer]-&gt;recycle();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 141 | 8 / 准备局部状态 | <code>ssize_t recvChunkSize = getP2pChunkSize(totRecvBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);</code> | 在最小/最大通道预算内倍增并行度，将每 peer 消息划成按 step 对齐的块。 |
| 142 | 8 / 准备局部状态 | <code>ssize_t sendChunkSize = getP2pChunkSize(totSendBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);</code> | 在最小/最大通道预算内倍增并行度，将每 peer 消息划成按 step 对齐的块。 |
| 143 | 8 / 准备局部状态 | <code>ssize_t sendOffset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 144 | 8 / 准备局部状态 | <code>ssize_t recvOffset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 145 | 8 / 准备局部状态 | <code>int sendRemaining = 1, recvRemaining = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 146 | 8 / 准备局部状态 | <code>int chunk = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 147 | 8 / 先执行一次的循环 | <code>初始 ；继续条件 (sendRemaining &#124;&#124; recvRemaining)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 148 | 9 / 准备局部状态 | <code>ssize_t recvbytes = totRecvBytes-recvOffset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 149 | 9 / 准备局部状态 | <code>ssize_t sendbytes = totSendBytes-sendOffset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 150 | 9 / 条件判断 | <code>(recvbytes &gt; recvChunkSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 151 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 152 | 11 / 调用 / 计算 / 写入 | <code>recvbytes = recvChunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 153 | 10 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 154 | 11 / 调用 / 计算 / 写入 | <code>recvRemaining = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 155 | 9 / 条件判断 | <code>(sendbytes &gt; sendChunkSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 156 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 157 | 11 / 调用 / 计算 / 写入 | <code>sendbytes = sendChunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 158 | 10 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 159 | 11 / 调用 / 计算 / 写入 | <code>sendRemaining = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 160 | 9 / 条件判断 | <code>(sendbytes &lt; 0 &#124;&#124; (sendbytes == 0 && totSendBytes != 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 161 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 162 | 11 / 调用 / 计算 / 写入 | <code>send = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 163 | 9 / 条件判断 | <code>(recvbytes &lt; 0 &#124;&#124; (recvbytes == 0 && totRecvBytes != 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 164 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 165 | 11 / 调用 / 计算 / 写入 | <code>recv = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 166 | 9 / 条件判断 | <code>(recv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 167 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 168 | 11 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(scheduleRecv(comm, recvPeer, chunk, recvbytes, ((char*)recvBuff)+recvOffset), ret, group_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；把 Group 切出的字节块封装为 P2P 任务，按 peer/块号映射 channel 后调用 SetupP2pKernel。 |
| 169 | 9 / 条件判断 | <code>(send)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 170 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 171 | 11 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(scheduleSend(comm, sendPeer, chunk, sendbytes, ((char*)sendBuff)+sendOffset), ret, group_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；把 Group 切出的字节块封装为 P2P 任务，按 peer/块号映射 channel 后调用 SetupP2pKernel。 |
| 172 | 9 / 调用 / 计算 / 写入 | <code>recvOffset += recvChunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 173 | 9 / 调用 / 计算 / 写入 | <code>sendOffset += sendChunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 174 | 9 / 调用 / 计算 / 写入 | <code>chunk++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 175 | 8 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 176 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 177 | 5 / 调用 / 计算 / 写入 | <code>index++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 178 | 5 / 条件判断 | <code>(index == 1 && deltas[1] == deltas[0])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 179 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 180 | 7 / 调用 / 计算 / 写入 | <code>index++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 181 | 5 / 条件判断 | <code>(index == 2 && deltas[2] == deltas[0])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 182 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 183 | 7 / 调用 / 计算 / 写入 | <code>index++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 184 | 5 / 条件判断 | <code>(index == 3 && deltas[3] == deltas[2])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 185 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 186 | 7 / 调用 / 计算 / 写入 | <code>index++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 187 | 5 / 条件判断 | <code>(index == 3 && deltas[3] == deltas[1])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 188 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 189 | 7 / 调用 / 计算 / 写入 | <code>index++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 190 | 5 / 条件判断 | <code>(index &lt; 4)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 191 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 192 | 7 / 调用 / 计算 / 写入 | <code>delta = deltas[index];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 193 | 7 / 控制转移 | <code>goto sched_delta;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 194 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 195 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 196 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 197 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&graphs, ncclGroupIndex));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 198 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 199 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 200 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_COLL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 201 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 202 | 3 / 准备局部状态 | <code>ncclComm_t comm = args-&gt;coll.comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 203 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclGetCudaGraph(comm, graphs+i), ret, group_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；检查当前 stream 是否捕获、图标识和驱动能力，维护图模式队列及生命周期状态。 |
| 204 | 3 / 条件判断 | <code>(usingCudaGraphAll == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 205 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 206 | 5 / 调用 / 计算 / 写入 | <code>usingCudaGraphAll = comm-&gt;usingCudaGraph;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 207 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 208 | 5 / 条件判断 | <code>(usingCudaGraphAll != comm-&gt;usingCudaGraph)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 209 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 210 | 7 / 调用 / 计算 / 写入 | <code>WARN("Illegal to have some communicators in graph mode while others not");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 211 | 7 / 调用 / 计算 / 写入 | <code>ret = ncclInvalidUsage;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 212 | 7 / 控制转移 | <code>goto group_cleanup;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 213 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 214 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 215 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 216 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_COLL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 217 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 218 | 3 / 准备局部状态 | <code>ncclComm_t comm = args-&gt;coll.comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 219 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSetupAsyncKernels(comm), ret, group_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；把 Group 保存的一组 collective 转成发射任务，对同构批次可统一选型并聚合工作元素。 |
| 220 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 221 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 222 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 223 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_COLL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 224 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 225 | 3 / 条件判断 | <code>(args-&gt;coll.comm-&gt;userStream == cudaStreamDefault &#124;&#124; args-&gt;coll.comm-&gt;userStream == cudaStreamPerThread &#124;&#124; args-&gt;coll.comm-&gt;userStream == cudaStreamLegacy)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 226 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 227 | 5 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaSetDevice(args-&gt;coll.comm-&gt;cudaDev), ret, end);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 228 | 3 / 条件判断 | <code>(usingCudaGraphAll == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 229 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 230 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclCudaGraphHostSetup(args-&gt;coll.comm, graphs[i]), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；给 CUDA Graph 添加/关联 HostSetup 节点及持有对象，确保 replay 时重新发布 GPU/Proxy 工作。 |
| 231 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 232 | 5 / 调用 / 计算 / 写入 | <code>ncclEnqueueHostSetup&lt;0&gt;(args-&gt;coll.comm-&gt;enqueueInfo);</code> | 遍历提交批次并执行真正的 FIFO 装填、launch 整理和 Proxy 发布；图 replay 也复用此入口。 |
| 233 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclLaunchBarrier(args-&gt;coll.comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；建立用户 stream 与内部 stream 依赖，并在 GROUP 发射模式协调同进程 GPU 的启动。 |
| 234 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 235 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 236 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 237 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_COLL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 238 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 239 | 3 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaSetDevice(args-&gt;coll.comm-&gt;cudaDev), ret, end);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 240 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclLaunchKernel(args-&gt;coll.comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；PARALLEL 模式真正调用 CUDA 发射，GROUP 模式等待已由最后线程执行的统一发射完成。 |
| 241 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 242 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 243 | 1 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 244 | 1 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_COLL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 245 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 246 | 3 / 条件判断 | <code>(args-&gt;coll.comm-&gt;userStream == cudaStreamDefault &#124;&#124; args-&gt;coll.comm-&gt;userStream == cudaStreamPerThread &#124;&#124; args-&gt;coll.comm-&gt;userStream == cudaStreamLegacy)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 247 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 248 | 5 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaSetDevice(args-&gt;coll.comm-&gt;cudaDev), ret, end);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 249 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclRecordEvents(args-&gt;coll.comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；在执行 stream 记录完成事件并建立必要的用户 stream 依赖；record 本身不代表事件已经完成。 |
| 250 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclLaunchReset(args-&gt;coll.comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；清理本批发射参数和入队计数，为后续提交复用；图路径按持久 QueueInfo 生命周期处理。 |
| 251 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 252 | 0 / 控制转移 | <code>goto end;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 253 | 0 / 跳转标签 | <code>group_cleanup</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 254 | 1 / 条件判断 | <code>(ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 255 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 256 | 3 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclGroupIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 257 | 4 / 准备局部状态 | <code>struct ncclAsyncArgs* args = ncclGroupArgs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 258 | 4 / 条件判断 | <code>(args-&gt;funcType == ASYNC_FUNC_INIT)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 259 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 260 | 6 / 条件判断 | <code>(args-&gt;init.newcomm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 261 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 262 | 8 / 调用 / 计算 / 写入 | <code>ncclCommDestroy(*args-&gt;init.newcomm);</code> | 正常销毁公开 communicator 句柄，按本版生命周期要求回收长期通信对象。 |
| 263 | 6 / 调用 / 计算 / 写入 | <code>*args-&gt;init.newcomm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 264 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 265 | 6 / 准备局部状态 | <code>struct ncclComm* comm = args-&gt;coll.comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 266 | 6 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncOpCount = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 267 | 6 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncTotalSize = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 268 | 6 / 条件判断 | <code>(comm-&gt;p2pSendCount &gt; 0 &#124;&#124; comm-&gt;p2pRecvCount &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 269 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 270 | 8 / 循环 | <code>初始 int peer=0;；继续条件 peer&lt;comm-&gt;nRanks；每轮更新 peer++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 271 | 9 / 条件判断 | <code>(comm-&gt;p2pSends[peer])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 272 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 273 | 11 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pSends[peer]-&gt;recycle();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 274 | 9 / 条件判断 | <code>(comm-&gt;p2pRecvs[peer])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 275 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 276 | 11 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pRecvs[peer]-&gt;recycle();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 277 | 8 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 278 | 8 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pSendCount = comm-&gt;p2pRecvCount = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 279 | 6 / 调用 / 计算 / 写入 | <code>ncclLaunchReset(comm);</code> | 清理本批发射参数和入队计数，为后续提交复用；图路径按持久 QueueInfo 生命周期处理。 |
| 280 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 281 | 0 / 跳转标签 | <code>end</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 282 | 1 / 调用 / 计算 / 写入 | <code>ncclGroupError = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 283 | 0 / 调用 / 计算 / 写入 | <code>ncclGroupIndex = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 284 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaSetDevice(savedDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 285 | 0 / 条件判断 | <code>(graphs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 286 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 287 | 2 / 调用 / 计算 / 写入 | <code>free(graphs);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 288 | 0 / 函数返回 | <code>return ret;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (ncclGroupMode == 0) {
    WARN("ncclGroupEnd: not in a group call.");
    return ncclInvalidUsage;
  }
  ncclGroupMode--;
  if (ncclGroupMode > 0) return ncclSuccess;
  int savedDev;
  CUDACHECK(cudaGetDevice(&savedDev));
  int activeThreads = 0;
  int doneArray[MAX_ASYNC_OPS];
  for (int i=0; i<ncclGroupIndex; i++) doneArray[i] = 1;
  ncclResult_t ret = ncclGroupError;
  int usingCudaGraphAll = -1;
  cudaGraph_t* graphs = NULL;
  if (ret != ncclSuccess) goto group_cleanup;

  /* Launch async ncclCommInitRank */
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_INIT) {
      pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadMain, args);
      activeThreads++;
      doneArray[i] = 0;
    }
  }
  /* For init, since we use threads, we just wait for threads to complete */
  while (activeThreads) {
    for (int i=0; i<ncclGroupIndex; i++) {
      struct ncclAsyncArgs* args = ncclGroupArgs+i;
      if (args->funcType == ASYNC_FUNC_INIT && doneArray[i] == 0) {
        int err = pthread_tryjoin_np(ncclGroupThreads[i], NULL);
        if (err == EBUSY) continue;
        if (err != 0) ret = ncclSystemError;
        if (args->ret != ncclSuccess) ret = args->ret;
        doneArray[i] = 1;
        activeThreads--;
      }
    }
  }

  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL && args->coll.comm->connect) {
      pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadPreconnect, args);
    }
  }

  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL && args->coll.comm->connect) {
      int err = pthread_join(ncclGroupThreads[i], NULL);
      if (err != 0) {
        WARN("Error waiting for pthread_join : %s", strerror(errno));
        return ncclSystemError;
      }
      NCCLCHECKGOTO(args->ret, ret, end);
      args->coll.comm->connect = 0;
    }
  }

  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      struct ncclComm* comm = args->coll.comm;
      int node = comm->node;
      int nNodes = comm->nNodes;
      int localRank = comm->localRank;

/* 中文精读：P2P 并行度为什么不是永远开到最大
 * 每 peer 的 channel 预算是上限；很多 peer 同时收发会争用全局 p2pnChannels。
 * 这里按 rank 数缩小 nChannelsMin/Max，减少一个 block 被塞入过多收发项而损失同步 warp 的情况。
 * 因此初始化日志中的 per peer 数不等于每条消息实际占用数；消息尺寸与同批工作量都会影响拆分。
 */
      // Compute how much to split operations
      // Natural step size matching buffer steps.
      ssize_t stepSize = comm->buffSizes[NCCL_PROTO_SIMPLE] / NCCL_STEPS;
      // Try to use all channels
      int nChannelsMax = comm->p2pnChannelsPerPeer;
      int nChannelsMin = nChannelsMax;
      // Try to use all channels, but one channel per operation.
      while (nChannelsMin*comm->nRanks > comm->p2pnChannels && nChannelsMin > 1) nChannelsMin /= 2;
      // Avoid overloading channels with 8+ operations as we loose the sync warp, hence a bit of bandwidth.
      while (nChannelsMax*comm->nRanks > comm->p2pnChannels*4 && nChannelsMax > 1) nChannelsMax /= 2;

      while (comm->p2pSendCount > 0 || comm->p2pRecvCount > 0) {
/* 中文精读：为什么收发必须采用双方一致的顺序
 * 用节点相对距离 delta 和节点内步号 s 选 sendPeer/recvPeer，再按统一规则跳过重复距离。
 * 这是显式排列 peer 对，而不是让所有 rank 按各自任意顺序串行等待；匹配收发才能持续前进。
 * 接着将每 peer 消息按 chunk 分发，Send 的偏移递增，Recv 的偏移独立递增。
 * 0 字节是有效同步任务，-1 表示这一方向没有任务，二者不能合并成同一种空操作。
 */
        // schedule delta 0, +1, -1, +2, -2, ...
        // also make sure we don't do 0 twice, nor +n/2 and -n/2 if n is even.
        for (int d=0; d<=nNodes/4; d++) {
          int deltas[4] = { d, (nNodes-d)%nNodes, nNodes/2-d, (nNodes-(nNodes/2-d))%nNodes };
          int index = 0;
          int delta = deltas[index];
sched_delta:
          uint32_t recvNode = (node+nNodes-delta)%nNodes;
          uint32_t sendNode = (node+delta)%nNodes;
          int steps = comm->maxLocalRanks;
          for (int s=0; s<steps; s++) {
            int recvIndex = (localRank-s+steps)%steps;
            int recvPeer = recvIndex<comm->nodeRanks[recvNode].localRanks ? comm->nodeRanks[recvNode].localRankToRank[recvIndex] : -1;
            int sendIndex = (localRank+s)%steps;
            int sendPeer = sendIndex<comm->nodeRanks[sendNode].localRanks ? comm->nodeRanks[sendNode].localRankToRank[sendIndex] : -1;
            struct ncclP2Pinfo* recv = recvPeer != -1 && comm->p2pRecvs[recvPeer] ? comm->p2pRecvs[recvPeer]->getNext() : NULL;
            struct ncclP2Pinfo* send = sendPeer != -1 && comm->p2pSends[sendPeer] ? comm->p2pSends[sendPeer]->getNext() : NULL;
            if (recv != NULL || send != NULL) {
              ssize_t totRecvBytes = -1, totSendBytes = -1;
              if (recv != NULL) totRecvBytes = recv->nbytes;
              if (send != NULL) totSendBytes = send->nbytes;
              if (recv) comm->p2pRecvCount--;
              if (send) comm->p2pSendCount--;
              if (recvPeer == comm->rank) { // Check self send/recv
                if (sendPeer != comm->rank) { WARN("Sendrecv schedule not aligned for self"); ret = ncclInternalError; goto group_cleanup; }
                if (send && recv == NULL) { WARN("Trying to send to self without a matching recv"); ret = ncclInvalidUsage; goto group_cleanup; }
                if (send == NULL && recv) { WARN("Trying to recv to self without a matching send"); ret = ncclInvalidUsage; goto group_cleanup; }
              }
              void* recvBuff = recv ? recv->buff : NULL;
              void* sendBuff = send ? send->buff : NULL;
              // After we recycle p2pSend/Recv, we're no longer allowed to dereference send or recv, only use them as boolean NULL/not NULL.
              if (recv && comm->p2pRecvs[recvPeer]->peakNext() == NULL) comm->p2pRecvs[recvPeer]->recycle();
              if (send && comm->p2pSends[sendPeer]->peakNext() == NULL) comm->p2pSends[sendPeer]->recycle();

              ssize_t recvChunkSize = getP2pChunkSize(totRecvBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);
              ssize_t sendChunkSize = getP2pChunkSize(totSendBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);

              ssize_t sendOffset = 0;
              ssize_t recvOffset = 0;
              int sendRemaining = 1, recvRemaining = 1;
              int chunk = 0;
              do {
                // Shuffle channels with s intra-node, and delta inter-node. Inter-node, make sure
                // to use multiple channels to guarantee progress on all ranks from the same node.
                ssize_t recvbytes = totRecvBytes-recvOffset;
                ssize_t sendbytes = totSendBytes-sendOffset;
                if (recvbytes > recvChunkSize) { recvbytes = recvChunkSize; } else { recvRemaining = 0; }
                if (sendbytes > sendChunkSize) { sendbytes = sendChunkSize; } else { sendRemaining = 0; }
                // 0-bytes send/recv are considered as syncs. Make sure we only add syncs when requested
                // (total size == 0), otherwise set size to -1.
                if (sendbytes < 0 || (sendbytes == 0 && totSendBytes != 0)) send = NULL;
                if (recvbytes < 0 || (recvbytes == 0 && totRecvBytes != 0)) recv = NULL;
                if (recv) {
                  NCCLCHECKGOTO(scheduleRecv(comm, recvPeer, chunk, recvbytes, ((char*)recvBuff)+recvOffset), ret, group_cleanup);
                }
                if (send) {
                  NCCLCHECKGOTO(scheduleSend(comm, sendPeer, chunk, sendbytes, ((char*)sendBuff)+sendOffset), ret, group_cleanup);
                }
                recvOffset += recvChunkSize;
                sendOffset += sendChunkSize;
                chunk++;
              } while (sendRemaining || recvRemaining);
            }
          }
          index++;
          if (index == 1 && deltas[1] == deltas[0]) index++;
          if (index == 2 && deltas[2] == deltas[0]) index++;
          if (index == 3 && deltas[3] == deltas[2]) index++;
          if (index == 3 && deltas[3] == deltas[1]) index++;
          if (index < 4) {
            delta = deltas[index];
            goto sched_delta;
          }
        }
      }
    }
  }

  /* Collectives are done in three steps :
   * 0. Save kernels previously enqueued. Compute channel, algo, proto, etc.
   * 1. Barrier Check In. Only the last call may call cudaLaunchKernel[cooperative]
   * 2. Barrier Wait. No CUDA call is permitted
   * 3. Enqueue Events. CUDA event wait/enqueue.
   * This is needed because step 2 cannot call any CUDA primitive, otherwise if
   * cudaFree happens between 1 and 3, it could block that CUDA call and
   * prevent some ranks from launching their network threads, which would
   * prevent the NCCL call from completing, blocking the cudaFree call.
   */

/* 中文精读：Group 最后阶段的调用顺序
 * 先统一检查各 communicator 是否都处于图捕获模式，再 SetupAsyncKernels 生成各自计划。
 * 普通模式对每个 comm 执行 HostSetup（其中发布 Proxy 任务）和 LaunchBarrier，
 * 然后再统一调用 LaunchKernel，最后 RecordEvents/LaunchReset。
 * 把准备、屏障、发射、事件拆为多轮，是为了同进程多 GPU 能互相等到匹配的工作。
 */
  // Check whether we are in cuda graph mode
  NCCLCHECK(ncclCalloc(&graphs, ncclGroupIndex));
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      ncclComm_t comm = args->coll.comm;
      NCCLCHECKGOTO(ncclGetCudaGraph(comm, graphs+i), ret, group_cleanup);
      if (usingCudaGraphAll == -1) {
        usingCudaGraphAll = comm->usingCudaGraph;
      } else if (usingCudaGraphAll != comm->usingCudaGraph) {
        WARN("Illegal to have some communicators in graph mode while others not");
        ret = ncclInvalidUsage;
        goto group_cleanup;
      }
    }
  }
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      ncclComm_t comm = args->coll.comm;
      NCCLCHECKGOTO(ncclSetupAsyncKernels(comm), ret, group_cleanup);
    }
  }
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      if (args->coll.comm->userStream == cudaStreamDefault ||
          args->coll.comm->userStream == cudaStreamPerThread ||
          args->coll.comm->userStream == cudaStreamLegacy)
        CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      if (usingCudaGraphAll == 1) {
        NCCLCHECKGOTO(ncclCudaGraphHostSetup(args->coll.comm, graphs[i]), ret, end);
      } else {
        ncclEnqueueHostSetup<0>(args->coll.comm->enqueueInfo);
      }
      NCCLCHECKGOTO(ncclLaunchBarrier(args->coll.comm), ret, end);
    }
  }
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      NCCLCHECKGOTO(ncclLaunchKernel(args->coll.comm), ret, end);
    }
  }
  for (int i=0; i<ncclGroupIndex; i++) {
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      if (args->coll.comm->userStream == cudaStreamDefault ||
          args->coll.comm->userStream == cudaStreamPerThread ||
          args->coll.comm->userStream == cudaStreamLegacy)
        CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      NCCLCHECKGOTO(ncclRecordEvents(args->coll.comm), ret, end);
      NCCLCHECKGOTO(ncclLaunchReset(args->coll.comm), ret, end);
    }
  }

  goto end;
group_cleanup:
  if (ret != ncclSuccess) {
    // At least one call in the group failed. Since we want to make that group
    // an atomic operation, we need to cancel all operations.
    for (int i=0; i<ncclGroupIndex; i++) {
      struct ncclAsyncArgs* args = ncclGroupArgs+i;
      if (args->funcType == ASYNC_FUNC_INIT) {
        if (args->init.newcomm) ncclCommDestroy(*args->init.newcomm);
        *args->init.newcomm = NULL;
      } else {
        struct ncclComm* comm = args->coll.comm;
        // Reset aggregation counters
        comm->asyncOpCount = 0;
        comm->asyncTotalSize = 0;
        // Dequeue p2p lists
        if (comm->p2pSendCount > 0 || comm->p2pRecvCount > 0) {
          for (int peer=0; peer<comm->nRanks; peer++) {
            if (comm->p2pSends[peer]) comm->p2pSends[peer]->recycle();
            if (comm->p2pRecvs[peer]) comm->p2pRecvs[peer]->recycle();
          }
          comm->p2pSendCount = comm->p2pRecvCount = 0;
        }
        ncclLaunchReset(comm);
      }
    }
  }
end:
  ncclGroupError = ncclSuccess;
  ncclGroupIndex = 0;
  CUDACHECK(cudaSetDevice(savedDev)); // do other clean-ups first before calling cudaSetDevice, because this call can fail too
  if (graphs) free(graphs);
  return ret;
}
```

</details>

