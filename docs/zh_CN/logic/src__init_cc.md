# src/init.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0881"></a>

## F0881：initGdrCopy

用途：按编译支持及运行环境尝试初始化 GDRCopy，供小量显存映射访问优化使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ncclParamGdrCopyEnable() == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>ncclGdrCopy = ncclGdrInit();</code> | 动态初始化 GDRCopy 并取得映射句柄，失败时由调用者决定是否使用普通回退路径。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ncclParamGdrCopyEnable() == 1) {
    ncclGdrCopy = ncclGdrInit();
  }
  return ncclSuccess;
}
```

</details>

<a id="f0882"></a>

## F0882：ncclInit

用途：进程级单次初始化配置、网络与相关支持库，避免每个 communicator 重复初始化全局组件。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(initialized)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&initLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 条件判断 | <code>(!initialized)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>initEnv();</code> | 按用户配置、系统配置顺序加载配置文件，已有键不会被后读文件覆盖。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>initGdrCopy();</code> | 按编译支持及运行环境尝试初始化 GDRCopy，供小量显存映射访问优化使用。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>maxLocalSizeBytes = ncclKernMaxLocalSize();</code> | 遍历入口 kernel 属性以求最大局部内存使用，供初始化设备栈/资源配置参考。 |
| 10 | 2 / 准备局部状态 | <code>int carveout = ncclParamL1SharedMemoryCarveout();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 2 / 条件判断 | <code>(carveout)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>ncclKernSetSharedMemoryCarveout(carveout);</code> | 给受支持的 CUDA kernel 入口设置共享内存划分偏好，影响 SM 资源分配而不改算法语义。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetInit());</code> | 失败时可能提前return，成功才继续；总是初始化 bootstrap，再按外部插件、内置 IB、内置 Socket 顺序选可用数据后端，支持显式名字筛选。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Using network %s", ncclNetName());</code> | 读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>initialized = true;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&initLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (initialized) return ncclSuccess;
  pthread_mutex_lock(&initLock);
  if (!initialized) {
    initEnv();
    initGdrCopy();
    maxLocalSizeBytes = ncclKernMaxLocalSize();
    int carveout = ncclParamL1SharedMemoryCarveout();
    if (carveout) ncclKernSetSharedMemoryCarveout(carveout);
    NCCLCHECK(ncclNetInit());
    INFO(NCCL_INIT, "Using network %s", ncclNetName());
    initialized = true;
  }
  pthread_mutex_unlock(&initLock);
  return ncclSuccess;
}
```

</details>

<a id="f0883"></a>

## F0883：ncclGetVersion

用途：把本库编译版本写到用户输出参数，帮助应用确认实际链接的 NCCL 版本。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(version == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>*version = NCCL_VERSION_CODE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (version == NULL) return ncclInvalidArgument;
  *version = NCCL_VERSION_CODE;
  return ncclSuccess;
}
```

</details>

<a id="f0884"></a>

## F0884：ncclGetUniqueId

用途：先完成库级初始化，再生成应用需要分发给全组 rank 的 bootstrap 标识。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclInit());</code> | 失败时可能提前return，成功才继续；进程级单次初始化配置、网络与相关支持库，避免每个 communicator 重复初始化全局组件。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(out, "GetUniqueId", "out"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 3 | 0 / 函数返回 | <code>return bootstrapGetUniqueId(out);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclInit());
  NCCLCHECK(PtrCheck(out, "GetUniqueId", "out"));
  return bootstrapGetUniqueId(out);
}
```

</details>

<a id="f0885"></a>

## F0885：commPoison

用途：在释放/销毁流程给 communicator 关键字段写入无效状态，辅助发现后续误用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;rank = comm-&gt;cudaDev = comm-&gt;busId = comm-&gt;nRanks = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  comm->rank = comm->cudaDev = comm->busId = comm->nRanks = -1;
}
```

</details>

<a id="f0886"></a>

## F0886：commAlloc

用途：分配并初始化当前 rank 的 communicator 总对象、排队状态、事件与连接需求表。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ndev &lt; 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("invalid device count (%d) requested", ndev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(rank &gt;= ndev &#124;&#124; rank &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>WARN("rank %d exceeds ndev=%d", rank, ndev);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 准备局部状态 | <code>cudaEvent_t doneEvent;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaEventCreateWithFlags(&doneEvent, cudaEventDisableTiming));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 11 | 0 / 准备局部状态 | <code>cudaEvent_t intDoneEvent;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaEventCreateWithFlags(&intDoneEvent, cudaEventDisableTiming));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 13 | 0 / 准备局部状态 | <code>struct ncclComm* comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;rank = comm-&gt;hostDevComm.rank = rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;nRanks = comm-&gt;hostDevComm.nRanks = ndev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>cudaGetDevice(&comm-&gt;cudaDev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getBusId(comm-&gt;cudaDev, &comm-&gt;busId));</code> | 失败时可能提前return，成功才继续；查询本进程 CUDA 设备的 PCI 地址并转为整数标识，不能把设备 ordinal 当作 busId。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx", comm, rank, ndev, comm-&gt;cudaDev, comm-&gt;busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;doneEvent = doneEvent;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;intDoneEvent = intDoneEvent;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;checkPointers = ncclParamCheckPointers() == 1 ? true : false;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 9020</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;groupCudaStream = ncclParamGroupCudaStream();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;groupCudaStream = NCCL_GROUP_CUDA_STREAM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;fatalError = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostCalloc((uint32_t**)&comm-&gt;abortFlag, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;hostDevComm.abortFlag = comm-&gt;abortFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>*comm-&gt;abortFlag = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;argsptrs[0] = &comm-&gt;devComm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;argsptrs[1] = &comm-&gt;args;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;collNetSupport = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;asyncOps, NCCL_MAX_OPS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncOpCount = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncTotalSize = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;channelSize = ncclParamAggChannelSize();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncAllocMode = ncclComm::SHORTEST_QUEUE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 准备局部状态 | <code>char* str = getenv("NCCL_AGG_ALLOC_MODE");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 0 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_AGG_ALLOC_MODE set by environment to %s", str);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 0 / 条件判断 | <code>(str && strcmp(str, "ROUND_ROBIN") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncAllocMode = ncclComm::ROUND_ROBIN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaDriverGetVersion(&comm-&gt;driverVersion));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCreateQueueInfo(&comm-&gt;enqueueInfo, comm));</code> | 失败时可能提前return，成功才继续；分配一个提交/图持有的 QueueInfo 和可复用元素列表，记录 communicator 队列生命周期计数。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;lastSetupNode = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;lastCudaGraphId = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;disableGraphHelper = ncclParamDisableGraphHelper();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;graphRegister = ncclParamGraphRegister();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 0 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 11030</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 53 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;graphHelperResources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 54 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;graphHelperResources-&gt;comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 1 / 条件判断 | <code>(comm-&gt;driverVersion &gt;= 11030)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 56 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 57 | 3 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDriverEntryPoint("cuMemGetAddressRange", (void**)&comm-&gt;pfnCuMemGetAddressRange, cudaEnableDefault));</code> | 失败时可能提前return，成功才继续；提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。 |
| 58 | 0 / 源码语法片段 | <code>static_assert(MAXCHANNELS &lt;= sizeof(*comm-&gt;connectSend)*8, "comm-&gt;connectSend must have enough bits for all channels");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 59 | 0 / 源码语法片段 | <code>static_assert(MAXCHANNELS &lt;= sizeof(*comm-&gt;connectRecv)*8, "comm-&gt;connectRecv must have enough bits for all channels");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 60 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;connectSend, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 61 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;connectRecv, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 62 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pSendCount = comm-&gt;p2pRecvCount = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 63 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;p2pSends, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 64 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;p2pRecvs, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 65 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;MAXCHANNELS；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 66 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;channels[c].id = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 68 | 0 / 调用 / 计算 / 写入 | <code>*comret = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 69 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ndev < 1) {
    WARN("invalid device count (%d) requested", ndev);
    return ncclInvalidArgument;
  }
  if (rank >= ndev || rank < 0) {
    WARN("rank %d exceeds ndev=%d", rank, ndev);
    return ncclInvalidArgument;
  }

  // Try to create a CUDA object right away. If there is something wrong with
  // the device we're on (failure cause #1) , better know it early.
  cudaEvent_t doneEvent;
  CUDACHECK(cudaEventCreateWithFlags(&doneEvent, cudaEventDisableTiming));
  cudaEvent_t intDoneEvent;
  CUDACHECK(cudaEventCreateWithFlags(&intDoneEvent, cudaEventDisableTiming));

  struct ncclComm* comm;
  NCCLCHECK(ncclCalloc(&comm, 1));

  comm->rank = comm->hostDevComm.rank = rank;
  comm->nRanks = comm->hostDevComm.nRanks = ndev;
  cudaGetDevice(&comm->cudaDev);
  NCCLCHECK(getBusId(comm->cudaDev, &comm->busId));
  TRACE(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx", comm, rank, ndev, comm->cudaDev, comm->busId);

  comm->doneEvent = doneEvent;
  comm->intDoneEvent = intDoneEvent;
  comm->checkPointers = ncclParamCheckPointers() == 1 ? true : false;
#if CUDART_VERSION >= 9020
  comm->groupCudaStream = ncclParamGroupCudaStream();
#else
  // Don't allow the user to overload the default setting in older CUDA builds
  comm->groupCudaStream = NCCL_GROUP_CUDA_STREAM;
#endif
  comm->fatalError = ncclSuccess;

  NCCLCHECK(ncclCudaHostCalloc((uint32_t**)&comm->abortFlag, 1));
  comm->hostDevComm.abortFlag = comm->abortFlag;
  *comm->abortFlag = 0;

  comm->argsptrs[0] = &comm->devComm;
  comm->argsptrs[1] = &comm->args;
  comm->collNetSupport = 0;

  NCCLCHECK(ncclCalloc(&comm->asyncOps, NCCL_MAX_OPS));
  comm->asyncOpCount = 0;
  comm->asyncTotalSize = 0;
  comm->channelSize = ncclParamAggChannelSize();
  comm->asyncAllocMode = ncclComm::SHORTEST_QUEUE;
  char* str = getenv("NCCL_AGG_ALLOC_MODE");
  if (str) INFO(NCCL_ENV, "NCCL_AGG_ALLOC_MODE set by environment to %s", str);
  if (str && strcmp(str, "ROUND_ROBIN") == 0) {
    comm->asyncAllocMode = ncclComm::ROUND_ROBIN;
  }

  CUDACHECK(cudaDriverGetVersion(&comm->driverVersion));

  NCCLCHECK(ncclCreateQueueInfo(&comm->enqueueInfo, comm));
  comm->lastSetupNode = NULL;
  comm->lastCudaGraphId = -1;
  comm->disableGraphHelper = ncclParamDisableGraphHelper();
  comm->graphRegister = ncclParamGraphRegister();
#if CUDART_VERSION >= 11030
  NCCLCHECK(ncclCalloc(&comm->graphHelperResources, 1));
  comm->graphHelperResources->comm = comm;
  if (comm->driverVersion >= 11030)
    // cudaGetDriverEntryPoint requires R465 or above (enhanced compat need)
    CUDACHECK(cudaGetDriverEntryPoint("cuMemGetAddressRange", (void**)&comm->pfnCuMemGetAddressRange, cudaEnableDefault));
#endif

  static_assert(MAXCHANNELS <= sizeof(*comm->connectSend)*8, "comm->connectSend must have enough bits for all channels");
  static_assert(MAXCHANNELS <= sizeof(*comm->connectRecv)*8, "comm->connectRecv must have enough bits for all channels");
  NCCLCHECK(ncclCalloc(&comm->connectSend, comm->nRanks));
  NCCLCHECK(ncclCalloc(&comm->connectRecv, comm->nRanks));

  comm->p2pSendCount = comm->p2pRecvCount = 0;
  NCCLCHECK(ncclCalloc(&comm->p2pSends, comm->nRanks));
  NCCLCHECK(ncclCalloc(&comm->p2pRecvs, comm->nRanks));

  // Mark channels as non initialized.
  for (int c=0; c<MAXCHANNELS; c++) comm->channels[c].id = -1;

  *comret = comm;
  return ncclSuccess;
}
```

</details>

<a id="f0887"></a>

## F0887：devCommSetup

用途：构造设备端精简 communicator 和连续 channel 布局，把主机连接视图转换成 GPU 可访问地址。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclDevCommAndChannels *devCommAndChans;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaCalloc(&devCommAndChans, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;devComm = &devCommAndChans-&gt;comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;hostDevComm.channels = devCommAndChans-&gt;channels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 准备局部状态 | <code>int nChannels = std::max(comm-&gt;nChannels, comm-&gt;p2pnChannels);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaMemcpy(comm-&gt;hostDevComm.channels, comm-&gt;channels, nChannels));</code> | 失败时可能提前return，成功才继续；按模板元素数量调用 cudaMemcpyDefault，实际字节数乘 sizeof(T)。 |
| 7 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;nChannels；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaMemcpy(comm-&gt;channels[r].ring.devUserRanks, comm-&gt;channels[r].ring.userRanks, comm-&gt;nRanks));</code> | 失败时可能提前return，成功才继续；按模板元素数量调用 cudaMemcpyDefault，实际字节数乘 sizeof(T)。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaMemcpy(comm-&gt;devComm, &comm-&gt;hostDevComm, 1));</code> | 失败时可能提前return，成功才继续；按模板元素数量调用 cudaMemcpyDefault，实际字节数乘 sizeof(T)。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclDevCommAndChannels *devCommAndChans;
  NCCLCHECK(ncclCudaCalloc(&devCommAndChans, 1));
  comm->devComm = &devCommAndChans->comm;
  comm->hostDevComm.channels = devCommAndChans->channels;

  // Duplicate the channels on the device
  int nChannels = std::max(comm->nChannels, comm->p2pnChannels);
  NCCLCHECK(ncclCudaMemcpy(comm->hostDevComm.channels, comm->channels, nChannels));

  // Copy userRanks and peers
  for (int r=0; r<comm->nChannels; r++) {
    NCCLCHECK(ncclCudaMemcpy(comm->channels[r].ring.devUserRanks, comm->channels[r].ring.userRanks, comm->nRanks));
  }

  // Duplicate the dev comm on the device
  NCCLCHECK(ncclCudaMemcpy(comm->devComm, &comm->hostDevComm, 1));
  return ncclSuccess;
}
```

</details>

<a id="f0888"></a>

## F0888：showVersion

用途：按日志配置打印库版本和构建相关信息，辅助确认实际加载的库。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>static int shown = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(shown == 0 && ncclDebugLevel &gt;= NCCL_LOG_VERSION)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>printf("%s\n", VERSION_STRING);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fflush(stdout);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 条件判断 | <code>(ncclDebugFile != stdout)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ALL,"%s", VERSION_STRING);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>shown = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static int shown = 0;
  if (shown == 0 && ncclDebugLevel >= NCCL_LOG_VERSION) {
    printf("%s\n", VERSION_STRING);
    fflush(stdout);
    if (ncclDebugFile != stdout)
      INFO(NCCL_ALL,"%s", VERSION_STRING); // Also log NCCL version in one of the files
    shown = 1;
  }
}
```

</details>

<a id="f0889"></a>

## F0889：fillInfo

用途：收集当前 rank 的主机、进程、GPU、共享内存和 GDR 能力摘要供 bootstrap 汇总。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;rank = comm-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&info-&gt;cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;hostHash=getHostHash()+commHash;</code> | 由主机/启动环境相关身份生成一致摘要，帮助识别同一主机上的参与者。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;pidHash=getPidHash()+commHash;</code> | 结合进程命名空间/进程标识形成摘要，用于判断是否同进程共享本地指针与发射状态。 |
| 5 | 0 / 准备局部状态 | <code>struct stat statbuf;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(stat("/dev/shm", &statbuf), "stat");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;shmDev = statbuf.st_dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;busId = comm-&gt;busId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGpuGdrSupport(&info-&gt;gdrSupport));</code> | 失败时可能提前return，成功才继续；优先查询 CUDA GDR 属性；旧运行时通过临时连接和 GPU 缓冲注册探测，探测资源用后回收。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;cudaCompCap = ncclCudaCompCap();</code> | 查询当前 CUDA 设备主次计算能力并编码成整数，供协议和架构策略判断。 |
| 12 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  info->rank = comm->rank;
  CUDACHECK(cudaGetDevice(&info->cudaDev));
  info->hostHash=getHostHash()+commHash;
  info->pidHash=getPidHash()+commHash;

  // Get the device MAJOR:MINOR of /dev/shm so we can use that
  // information to decide whether we can use SHM for inter-process
  // communication in a container environment
  struct stat statbuf;
  SYSCHECK(stat("/dev/shm", &statbuf), "stat");
  info->shmDev = statbuf.st_dev;

  info->busId = comm->busId;

  NCCLCHECK(ncclGpuGdrSupport(&info->gdrSupport));
  info->comm = comm;
  info->cudaCompCap = ncclCudaCompCap();
  return ncclSuccess;
}
```

</details>

<a id="f0890"></a>

## F0890：setupChannel

用途：把拓扑环顺序旋转到本 rank 开头，建立用户 rank 表及本 rank 内部环索引。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d", rank, nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(initChannel(comm, channelId));</code> | 失败时可能提前return，成功才继续；为单条逻辑 channel 分配环顺序表、peer 连接表和 GPU 可读取的工作描述符 FIFO。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclRing* ring = &comm-&gt;channels[channelId].ring;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int ixZero=0, ixRank=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 条件判断 | <code>(ringRanks[i] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>ixZero = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 1 / 条件判断 | <code>(ringRanks[i] == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>ixRank = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>ring-&gt;index = (ixRank-ixZero + nranks)%nranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>ring-&gt;userRanks[i] = ringRanks[(i+ixRank)%nranks];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  TRACE(NCCL_INIT, "rank %d nranks %d", rank, nranks);
  NCCLCHECK(initChannel(comm, channelId));

  struct ncclRing* ring = &comm->channels[channelId].ring;
  // Find our ring-distance from rank zero and reorganize ranks to start with rank.
  int ixZero=0, ixRank=0;
  for (int i=0; i < nranks; i++) {
    if (ringRanks[i] == 0) ixZero = i;
    if (ringRanks[i] == rank) ixRank = i;
  }
  ring->index = (ixRank-ixZero + nranks)%nranks;
  for (int i=0; i<nranks; i++) {
    ring->userRanks[i] = ringRanks[(i+ixRank)%nranks];
  }
  return ncclSuccess;
}
```

</details>

<a id="f0891"></a>

## F0891：waitForNonNullPtr

用途：等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>volatile void** ptr = (volatile void**) p;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 ；继续条件 (*ptr == NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>sched_yield();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 5 | 0 / 函数返回 | <code>return (void*)*ptr;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  volatile void** ptr = (volatile void**) p;
  while (*ptr == NULL) sched_yield();
  return (void*)*ptr;
}
```

</details>

<a id="f0892"></a>

## F0892：initParams

用途：初始化当前 communicator 的 CUDA 发射参数、参数指针与设备 kernel 所需上下文。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct cudaLaunchParams* params = comm-&gt;myParams = comm-&gt;intraParams+comm-&gt;intraRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;args = comm-&gt;argsptrs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;stream = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;sharedMem = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;blockDim.x = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;blockDim.y = params-&gt;blockDim.z = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;gridDim.x = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;gridDim.y = params-&gt;gridDim.z = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct cudaLaunchParams* params = comm->myParams = comm->intraParams+comm->intraRank;
  params->args = comm->argsptrs;
  params->stream = NULL;
  params->sharedMem = 0;
  params->blockDim.x = 0; params->blockDim.y = params->blockDim.z = 1;
  params->gridDim.x = 0; params->gridDim.y = params->gridDim.z = 1;
  return ncclSuccess;
}
```

</details>

<a id="f0893"></a>

## F0893：ncclCommSetIntraProc

用途：把同进程 rank 加入本地发射协调组，设置屏障、设备列表及协作发射能力。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;intraRank = rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;intraRanks = ranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;intraPhase = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(rank == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>assert(comm == comm0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 准备局部状态 | <code>int* bar;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&bar, 2));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>bar[0] = bar[1] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraBarrier = bar;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;intraParams, comm-&gt;intraRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;intraThreads, comm-&gt;intraRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;intraCudaDevs, comm-&gt;intraRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 2 / 准备局部状态 | <code>int* CGMode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&CGMode, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>*CGMode = 0x11;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraCGMode = CGMode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 2 / 准备局部状态 | <code>int* CC;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&CC, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>*CC = ncclCudaCompCap();</code> | 查询当前 CUDA 设备主次计算能力并编码成整数，供协议和架构策略判断。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraCC = CC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraBarrier = (int*)waitForNonNullPtr(&comm0-&gt;intraBarrier);</code> | 等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraParams = (struct cudaLaunchParams*)waitForNonNullPtr(&comm0-&gt;intraParams);</code> | 等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraThreads = (pthread_t*)waitForNonNullPtr(&comm0-&gt;intraThreads);</code> | 等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraCudaDevs = (int*)waitForNonNullPtr(&comm0-&gt;intraCudaDevs);</code> | 等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraCGMode = (int*)waitForNonNullPtr(&comm0-&gt;intraCGMode);</code> | 等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraCC = (int*)waitForNonNullPtr(&comm0-&gt;intraCC);</code> | 等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;intraCudaDevs[comm-&gt;intraRank] = comm-&gt;cudaDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;intraThreads[comm-&gt;intraRank] = comm-&gt;proxyState.thread;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(initParams(comm));</code> | 失败时可能提前return，成功才继续；初始化当前 communicator 的 CUDA 发射参数、参数指针与设备 kernel 所需上下文。 |
| 32 | 0 / 准备局部状态 | <code>int cgMdLaunch = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;launchMode = ncclComm::PARALLEL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 准备局部状态 | <code>char* str = getenv("NCCL_LAUNCH_MODE");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 0 / 条件判断 | <code>(str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_LAUNCH_MODE set by environment to %s", str);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 0 / 条件判断 | <code>(str && strcmp(str, "GROUP") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;launchMode = ncclComm::GROUP;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 0 / 条件判断 | <code>(comm-&gt;launchMode == ncclComm::GROUP)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamCreateWithFlags(&comm-&gt;groupStream, cudaStreamNonBlocking));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 44 | 2 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 9000</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 45 | 3 / 条件判断 | <code>(*comm-&gt;intraCC && (ncclCudaCompCap() == *comm-&gt;intraCC))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 5 / 调用 / 计算 / 写入 | <code>(void) cudaDeviceGetAttribute(&cgMdLaunch, cudaDevAttrCooperativeMultiDeviceLaunch, comm-&gt;cudaDev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 48 | 0 / 条件判断 | <code>(cgMdLaunch == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 49 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 50 | 2 / 调用 / 计算 / 写入 | <code>*comm-&gt;intraCGMode = 0x10;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  comm->intraRank = rank;
  comm->intraRanks = ranks;
  comm->intraPhase = 0;

  // Alloc shared structures
  if (rank == 0) {
    assert(comm == comm0);
    int* bar;
    NCCLCHECK(ncclCalloc(&bar, 2));
    bar[0] = bar[1] = 0;
    comm->intraBarrier = bar;
    NCCLCHECK(ncclCalloc(&comm->intraParams, comm->intraRanks));
    NCCLCHECK(ncclCalloc(&comm->intraThreads, comm->intraRanks));
    NCCLCHECK(ncclCalloc(&comm->intraCudaDevs, comm->intraRanks));
    int* CGMode;
    NCCLCHECK(ncclCalloc(&CGMode, 1));
    *CGMode = 0x11;
    comm->intraCGMode = CGMode;
    int* CC;
    NCCLCHECK(ncclCalloc(&CC, 1));
    *CC = ncclCudaCompCap();
    comm->intraCC = CC;
  } else {
    comm->intraBarrier = (int*)waitForNonNullPtr(&comm0->intraBarrier);
    comm->intraParams = (struct cudaLaunchParams*)waitForNonNullPtr(&comm0->intraParams);
    comm->intraThreads = (pthread_t*)waitForNonNullPtr(&comm0->intraThreads);
    comm->intraCudaDevs = (int*)waitForNonNullPtr(&comm0->intraCudaDevs);
    comm->intraCGMode = (int*)waitForNonNullPtr(&comm0->intraCGMode);
    comm->intraCC = (int*)waitForNonNullPtr(&comm0->intraCC);
  }
  comm->intraCudaDevs[comm->intraRank] = comm->cudaDev;
  comm->intraThreads[comm->intraRank] = comm->proxyState.thread;
  NCCLCHECK(initParams(comm));

  int cgMdLaunch = 0;

  // Set CG Mode
  comm->launchMode = ncclComm::PARALLEL;
  char* str = getenv("NCCL_LAUNCH_MODE");
  if (str) INFO(NCCL_ENV, "NCCL_LAUNCH_MODE set by environment to %s", str);
  if (str && strcmp(str, "GROUP") == 0) {
    comm->launchMode = ncclComm::GROUP;
  }
  if (comm->launchMode == ncclComm::GROUP) {
    CUDACHECK(cudaStreamCreateWithFlags(&comm->groupStream, cudaStreamNonBlocking));
#if CUDART_VERSION >= 9000
    if (*comm->intraCC && (ncclCudaCompCap() == *comm->intraCC)) {
      // Check whether the GPU supports Cooperative Group Multi Device Launch
      (void) cudaDeviceGetAttribute(&cgMdLaunch, cudaDevAttrCooperativeMultiDeviceLaunch, comm->cudaDev);
    }
#endif
  }

  // Disable cgMdLaunch if any rank does not support it
  if (cgMdLaunch == 0) {
    *comm->intraCGMode = 0x10;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0894"></a>

## F0894：computeBuffSizes

用途：读取/选择 LL、LL128、Simple 的协议缓冲尺寸，为后续 channel 传输与分片提供容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int cpuArch, cpuVendor, cpuModel;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCpuType(comm-&gt;topo, &cpuArch, &cpuVendor, &cpuModel));</code> | 失败时可能提前return，成功才继续；从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。 |
| 3 | 0 / 准备局部状态 | <code>int64_t envs[NCCL_NUM_PROTOCOLS] = { ncclParamLlBuffSize(), ncclParamLl128BuffSize(), ncclParamBuffSize() };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int defaults[NCCL_NUM_PROTOCOLS] = { DEFAULT_LL_BUFFSIZE, DEFAULT_LL128_BUFFSIZE, DEFAULT_BUFFSIZE };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(cpuArch == NCCL_TOPO_CPU_ARCH_ARM)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>defaults[NCCL_PROTO_SIMPLE] = DEFAULT_BUFFSIZE_ARM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;buffSizes[p] = comm-&gt;hostDevComm.buffSizes[p] = envs[p] != -2 ? envs[p] : defaults[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int cpuArch, cpuVendor, cpuModel;
  NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor, &cpuModel));

  int64_t envs[NCCL_NUM_PROTOCOLS] = { ncclParamLlBuffSize(), ncclParamLl128BuffSize(), ncclParamBuffSize() };
  int defaults[NCCL_NUM_PROTOCOLS] = { DEFAULT_LL_BUFFSIZE, DEFAULT_LL128_BUFFSIZE, DEFAULT_BUFFSIZE };

  if (cpuArch == NCCL_TOPO_CPU_ARCH_ARM) defaults[NCCL_PROTO_SIMPLE] = DEFAULT_BUFFSIZE_ARM;

  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    comm->buffSizes[p] = comm->hostDevComm.buffSizes[p] = envs[p] != -2 ? envs[p] : defaults[p];
  }
  return ncclSuccess;
}
```

</details>

<a id="f0895"></a>

## F0895：initTransportsRank

用途：串联 bootstrap、硬件建图、路径搜索、全局图拼接、传输建链和性能模型，建立当前 rank 通信环境。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int rank = comm-&gt;rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nranks = comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>uint64_t commHash = getHash(commId-&gt;internal, NCCL_UNIQUE_ID_BYTES);</code> | 对给定字符串字节计算哈希，供主机/进程身份摘要使用，不承担安全认证。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "comm %p, commHash %lx, rank %d nranks %d - BEGIN", comm, commHash, rank, nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapInit(commId, comm));</code> | 失败时可能提前return，成功才继续；让当前 rank 加入控制网络并交换 peer/proxy 地址，为拓扑发现和传输建链准备控制通道。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;peerInfo, nranks+1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(fillInfo(comm, comm-&gt;peerInfo+rank, commHash));</code> | 失败时可能提前return，成功才继续；收集当前 rank 的主机、进程、GPU、共享内存和 GDR 能力摘要供 bootstrap 汇总。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapAllGather(comm-&gt;bootstrap, comm-&gt;peerInfo, sizeof(struct ncclPeerInfo)));</code> | 失败时可能提前return，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。 |
| 9 | 0 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 1 / 条件判断 | <code>((i != rank) && (comm-&gt;peerInfo[i].hostHash == comm-&gt;peerInfo[rank].hostHash) && (comm-&gt;peerInfo[i].busId == comm-&gt;peerInfo[rank].busId))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>WARN("Duplicate GPU detected : rank %d and rank %d both on CUDA device %lx", rank, i, comm-&gt;peerInfo[rank].busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 3 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetSystem(comm, &comm-&gt;topo));</code> | 失败时可能提前return，成功才继续；读入可选拓扑文件并补齐当前 rank 所见 GPU/NIC 的自动发现信息，再生成硬件图。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoComputePaths(comm-&gt;topo, comm-&gt;peerInfo));</code> | 失败时可能提前return，成功才继续；计算 CPU/GPU/NET 路径，并按 P2P、GDR、PXN 策略替换不可直接使用的路径。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoTrimSystem(comm-&gt;topo, comm));</code> | 失败时可能提前return，成功才继续；删除不属于当前通信可达域的硬件节点，并在完全机内通信时裁剪不需要的网络对象。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoComputePaths(comm-&gt;topo, comm-&gt;peerInfo));</code> | 失败时可能提前return，成功才继续；计算 CPU/GPU/NET 路径，并按 P2P、GDR、PXN 策略替换不可直接使用的路径。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSearchInit(comm-&gt;topo));</code> | 失败时可能提前return，成功才继续；初始化拓扑搜索的最大宽度和总宽度，建立速度候选与提前终止判断依据。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPrint(comm-&gt;topo));</code> | 失败时可能提前return，成功才继续；递归打印硬件节点、类型和直接链路，验证发现到的硬件树。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetCpuAffinity(comm-&gt;topo, comm-&gt;rank, &comm-&gt;cpuAffinity));</code> | 失败时可能提前return，成功才继续；结合 GPU 的近端 CPU/NUMA 亲和范围与当前进程约束，生成线程亲和性掩码。 |
| 22 | 0 / 准备局部状态 | <code>cpu_set_t affinitySave;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 0 / 条件判断 | <code>(CPU_COUNT(&comm-&gt;cpuAffinity))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>sched_getaffinity(0, sizeof(cpu_set_t), &affinitySave);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>sched_setaffinity(0, sizeof(cpu_set_t), &comm-&gt;cpuAffinity);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 0 / 准备局部状态 | <code>ncclResult_t ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCreate(comm));</code> | 失败时可能提前return，成功才继续；按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。 |
| 29 | 0 / 准备局部状态 | <code>struct ncclTopoGraph ringGraph;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>ringGraph.id = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>ringGraph.pattern = NCCL_TOPO_PATTERN_RING;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>ringGraph.collNet = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>ringGraph.minChannels = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>ringGraph.maxChannels = MAXCHANNELS/2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCompute(comm-&gt;topo, &ringGraph));</code> | 失败时可能提前return，成功才继续；在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPrintGraph(comm-&gt;topo, &ringGraph));</code> | 失败时可能提前return，成功才继续；输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。 |
| 37 | 0 / 准备局部状态 | <code>struct ncclTopoGraph treeGraph;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>treeGraph.id = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>treeGraph.pattern = NCCL_TOPO_PATTERN_BALANCED_TREE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>treeGraph.collNet = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>treeGraph.minChannels = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>treeGraph.maxChannels = ringGraph.nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCompute(comm-&gt;topo, &treeGraph));</code> | 失败时可能提前return，成功才继续；在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。 |
| 44 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPrintGraph(comm-&gt;topo, &treeGraph));</code> | 失败时可能提前return，成功才继续；输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。 |
| 45 | 0 / 准备局部状态 | <code>struct ncclTopoGraph collNetGraph;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>collNetGraph.id = 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>collNetGraph.pattern = NCCL_TOPO_PATTERN_TREE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>collNetGraph.collNet = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 0 / 调用 / 计算 / 写入 | <code>collNetGraph.minChannels = collNetGraph.maxChannels = ringGraph.nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCompute(comm-&gt;topo, &collNetGraph));</code> | 失败时可能提前return，成功才继续；在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。 |
| 51 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPrintGraph(comm-&gt;topo, &collNetGraph));</code> | 失败时可能提前return，成功才继续；输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。 |
| 52 | 0 / 条件判断 | <code>(comm-&gt;rank == ncclParamGraphDumpFileRank())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 2 / 准备局部状态 | <code>struct ncclTopoGraph* graphs[3] = { &ringGraph, &treeGraph, &collNetGraph };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 55 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoDumpGraphs(comm-&gt;topo, 3, graphs));</code> | 失败时可能提前return，成功才继续；输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。 |
| 56 | 0 / 条件判断 | <code>(collNetSupport())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 2 / 准备局部状态 | <code>char *collNetEnable = getenv("NCCL_COLLNET_ENABLE");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 2 / 条件判断 | <code>(collNetEnable != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 60 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 61 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ALL, "NCCL_COLLNET_ENABLE set by environment to %s.", collNetEnable);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 62 | 4 / 条件判断 | <code>(strcmp(collNetEnable, "1") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 6 / 调用 / 计算 / 写入 | <code>comm-&gt;collNetSupport = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 65 | 0 / 条件判断 | <code>(comm-&gt;collNetSupport == 1 && collNetGraph.nChannels &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 66 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 67 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;collNetSupport = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 68 | 0 / 源码语法片段 | <code>struct ncclGraphInfo { int pattern; int nChannels; int sameChannels; float speedIntra; float speedInter; int typeIntra; int typeInter; }</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 69 | 0 / 准备局部状态 | <code>struct { int netDev; int collNetSupport; struct ncclGraphInfo tree; struct ncclGraphInfo ring; struct ncclGraphInfo collNet; struct ncclTopoRanks topoRanks; } *allGather3Data;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 70 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&allGather3Data, nranks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 71 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetLocalNet(comm-&gt;topo, rank, &allGather3Data[rank].netDev));</code> | 失败时可能提前return，成功才继续；按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。 |
| 72 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].tree.pattern = treeGraph.pattern;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].tree.nChannels = treeGraph.nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 74 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].tree.sameChannels = treeGraph.sameChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 75 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].tree.speedIntra = treeGraph.speedIntra;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 76 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].tree.speedInter = treeGraph.speedInter;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 77 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].tree.typeIntra = treeGraph.typeIntra;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].tree.typeInter = treeGraph.typeInter;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].ring.pattern = ringGraph.pattern;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 80 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].ring.nChannels = ringGraph.nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 81 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].ring.sameChannels = ringGraph.sameChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].ring.speedIntra = ringGraph.speedIntra;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].ring.speedInter = ringGraph.speedInter;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].ring.typeIntra = ringGraph.typeIntra;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].ring.typeInter = ringGraph.typeInter;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 86 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].collNet.pattern = collNetGraph.pattern;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 87 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].collNet.nChannels = collNetGraph.nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 88 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].collNet.sameChannels = collNetGraph.sameChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 89 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].collNet.speedIntra = collNetGraph.speedIntra;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 90 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].collNet.speedInter = collNetGraph.speedInter;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 91 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].collNet.typeIntra = collNetGraph.typeIntra;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 92 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].collNet.typeInter = collNetGraph.typeInter;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 93 | 0 / 调用 / 计算 / 写入 | <code>allGather3Data[rank].collNetSupport = comm-&gt;collNetSupport;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 94 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;nChannels = std::min(treeGraph.nChannels, ringGraph.nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 95 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPreset(comm, &treeGraph, &ringGraph, &allGather3Data[rank].topoRanks));</code> | 失败时可能提前return，成功才继续；将本机搜索图转成可交换的局部环/树端点摘要，为跨节点拼接准备元数据。 |
| 96 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapAllGather(comm-&gt;bootstrap, allGather3Data, sizeof(*allGather3Data)));</code> | 失败时可能提前return，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。 |
| 97 | 0 / 准备局部状态 | <code>int *nodesFirstRank, *nodesTreePatterns;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 98 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&nodesFirstRank, nranks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 99 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&nodesTreePatterns, nranks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 100 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;rankToNode, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 101 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nranks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 102 | 1 / 准备局部状态 | <code>int node;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 103 | 1 / 准备局部状态 | <code>int firstRank = allGather3Data[r].topoRanks.ringRecv[0];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 104 | 1 / 循环 | <code>初始 node=0；继续条件 node&lt;comm-&gt;nNodes && nodesFirstRank[node] != firstRank；每轮更新 node++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 105 | 2 / 调用 / 计算 / 写入 | <code>;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 106 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 107 | 1 / 条件判断 | <code>(node == comm-&gt;nNodes)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 108 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 109 | 3 / 调用 / 计算 / 写入 | <code>comm-&gt;nNodes++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 110 | 3 / 调用 / 计算 / 写入 | <code>nodesFirstRank[node] = firstRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 111 | 3 / 调用 / 计算 / 写入 | <code>nodesTreePatterns[node] = allGather3Data[r].tree.pattern;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 112 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;rankToNode[r] = node;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 113 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 114 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;nodeRanks, comm-&gt;nNodes));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 115 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;rankToLocalRank, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 116 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;nRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 117 | 1 / 准备局部状态 | <code>int node = comm-&gt;rankToNode[r];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 118 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;rankToLocalRank[r] = comm-&gt;nodeRanks[node].localRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 119 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;nodeRanks[node].localRanks++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 120 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 121 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;comm-&gt;nNodes；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 122 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm-&gt;nodeRanks[n].localRankToRank, comm-&gt;nodeRanks[n].localRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 123 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;maxLocalRanks = std::max(comm-&gt;maxLocalRanks, comm-&gt;nodeRanks[n].localRanks);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 124 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;nodeRanks[n].localRanks = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 125 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 126 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;nRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 127 | 1 / 准备局部状态 | <code>int node = comm-&gt;rankToNode[r];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 128 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;nodeRanks[node].localRankToRank[comm-&gt;nodeRanks[node].localRanks++] = r;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 129 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 130 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;node = comm-&gt;rankToNode[rank];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 131 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;localRankToRank = comm-&gt;nodeRanks[comm-&gt;node].localRankToRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 132 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;localRank = comm-&gt;rankToLocalRank[rank];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 133 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;localRanks = comm-&gt;nodeRanks[comm-&gt;node].localRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 134 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT,"hostHash[%d] %lx localRank %d localRanks %d localRank0 %d", rank, comm-&gt;peerInfo[rank].hostHash, comm-&gt;localRank, comm-&gt;localRanks, comm-&gt;localRankToRank[0]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 135 | 0 / 条件判断 | <code>(comm-&gt;localRank == -1 &#124;&#124; comm-&gt;localRankToRank[0] == -1 &#124;&#124; comm-&gt;localRanks == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 136 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 137 | 2 / 调用 / 计算 / 写入 | <code>WARN("Failed to determine local ranks rank %d hostHash %lx pidHash %lx localRank %d localRanks %d localRank0 %d", rank, comm-&gt;peerInfo[rank].hostHash, comm-&gt;peerInfo[rank].pidHash, comm-&gt;localRank, comm-&gt;localRanks, comm-&gt;localRankToRank[0]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 138 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 139 | 0 / 准备局部状态 | <code>int nChannelsOrig = comm-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 140 | 0 / 准备局部状态 | <code>struct ncclTopoRanks** allTopoRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 141 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&allTopoRanks, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 142 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 143 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;peerInfo[i].netDev = allGather3Data[i].netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 144 | 1 / 调用 / 计算 / 写入 | <code>allTopoRanks[i] = &allGather3Data[i].topoRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 145 | 1 / 调用 / 计算 / 写入 | <code>treeGraph.nChannels = std::min(allGather3Data[i].tree.nChannels, treeGraph.nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 146 | 1 / 调用 / 计算 / 写入 | <code>treeGraph.sameChannels = std::min(allGather3Data[i].tree.sameChannels, treeGraph.sameChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 147 | 1 / 调用 / 计算 / 写入 | <code>treeGraph.speedIntra = std::min(allGather3Data[i].tree.speedIntra, treeGraph.speedIntra);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 148 | 1 / 调用 / 计算 / 写入 | <code>treeGraph.speedInter = std::min(allGather3Data[i].tree.speedInter, treeGraph.speedInter);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 149 | 1 / 调用 / 计算 / 写入 | <code>treeGraph.typeIntra = std::max(allGather3Data[i].tree.typeIntra, treeGraph.typeIntra);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 150 | 1 / 调用 / 计算 / 写入 | <code>treeGraph.typeInter = std::max(allGather3Data[i].tree.typeInter, treeGraph.typeInter);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 151 | 1 / 调用 / 计算 / 写入 | <code>ringGraph.nChannels = std::min(allGather3Data[i].ring.nChannels, ringGraph.nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 152 | 1 / 调用 / 计算 / 写入 | <code>ringGraph.sameChannels = std::min(allGather3Data[i].ring.sameChannels, ringGraph.sameChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 153 | 1 / 调用 / 计算 / 写入 | <code>ringGraph.speedIntra = std::min(allGather3Data[i].ring.speedIntra, ringGraph.speedIntra);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 154 | 1 / 调用 / 计算 / 写入 | <code>ringGraph.speedInter = std::min(allGather3Data[i].ring.speedInter, ringGraph.speedInter);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 155 | 1 / 调用 / 计算 / 写入 | <code>ringGraph.typeIntra = std::max(allGather3Data[i].ring.typeIntra, ringGraph.typeIntra);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 156 | 1 / 调用 / 计算 / 写入 | <code>ringGraph.typeInter = std::max(allGather3Data[i].ring.typeInter, ringGraph.typeInter);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 157 | 1 / 调用 / 计算 / 写入 | <code>collNetGraph.nChannels = std::min(allGather3Data[i].collNet.nChannels, collNetGraph.nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 158 | 1 / 调用 / 计算 / 写入 | <code>collNetGraph.sameChannels = std::min(allGather3Data[i].collNet.sameChannels, collNetGraph.sameChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 159 | 1 / 调用 / 计算 / 写入 | <code>collNetGraph.speedIntra = std::min(allGather3Data[i].collNet.speedIntra, collNetGraph.speedIntra);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 160 | 1 / 调用 / 计算 / 写入 | <code>collNetGraph.speedInter = std::min(allGather3Data[i].collNet.speedInter, collNetGraph.speedInter);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 161 | 1 / 调用 / 计算 / 写入 | <code>collNetGraph.typeIntra = std::max(allGather3Data[i].collNet.typeIntra, collNetGraph.typeIntra);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 162 | 1 / 调用 / 计算 / 写入 | <code>collNetGraph.typeInter = std::max(allGather3Data[i].collNet.typeInter, collNetGraph.typeInter);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 163 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;collNetSupport = std::min(allGather3Data[i].collNetSupport, comm-&gt;collNetSupport);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 164 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 165 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;nChannels = treeGraph.nChannels = ringGraph.nChannels = std::min(treeGraph.nChannels, ringGraph.nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 166 | 0 / 条件判断 | <code>(comm-&gt;nChannels &lt; nChannelsOrig)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 167 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 168 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;nChannels；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 169 | 3 / 调用 / 计算 / 写入 | <code>memcpy(comm-&gt;channels+comm-&gt;nChannels+i, comm-&gt;channels+nChannelsOrig+i, sizeof(struct ncclChannel));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 170 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 171 | 0 / 条件判断 | <code>(comm-&gt;collNetSupport == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 172 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 173 | 2 / 准备局部状态 | <code>int collNetNodeThreshold = ncclParamCollNetNodeThreshold();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 174 | 2 / 条件判断 | <code>(comm-&gt;nNodes &lt; collNetNodeThreshold)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 175 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 176 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Communicator has %d nodes which is less than CollNet node threshold %d, disabling CollNet", comm-&gt;nNodes, collNetNodeThreshold);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 177 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;collNetSupport = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 178 | 2 / 循环 | <code>初始 int n=0;；继续条件 n&lt;comm-&gt;nNodes；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 179 | 3 / 条件判断 | <code>(comm-&gt;nodeRanks[n].localRanks &gt; NCCL_MAX_DIRECT_ARITY+1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 180 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 181 | 5 / 调用 / 计算 / 写入 | <code>WARN("CollNet currently only supports up to %d GPUs per node, disabling CollNet", NCCL_MAX_DIRECT_ARITY+1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 182 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;collNetSupport = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 183 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 184 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 185 | 0 / 准备局部状态 | <code>int *rings;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 186 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&rings, nranks*MAXCHANNELS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 187 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoPostset(comm, nodesFirstRank, nodesTreePatterns, allTopoRanks, rings, &collNetGraph));</code> | 失败时可能提前return，成功才继续；汇总全部节点的端点信息并拼接全局 Ring/Tree，复制/裁剪通道后验证环完整性。 |
| 188 | 0 / 调用 / 计算 / 写入 | <code>free(allTopoRanks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 189 | 0 / 调用 / 计算 / 写入 | <code>free(nodesTreePatterns);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 190 | 0 / 调用 / 计算 / 写入 | <code>free(nodesFirstRank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 191 | 0 / 调用 / 计算 / 写入 | <code>free(allGather3Data);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 192 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d - BUILT %d TREES/RINGS", rank, nranks, comm-&gt;nChannels);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 193 | 0 / 准备局部状态 | <code>char line[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 194 | 0 / 调用 / 计算 / 写入 | <code>line[0]='\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 195 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 196 | 1 / 准备局部状态 | <code>struct ncclTree* tree = &comm-&gt;channels[c].tree;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 197 | 1 / 调用 / 计算 / 写入 | <code>snprintf(line+strlen(line), 1023-strlen(line), " [%d] %d/%d/%d-&gt;%d-&gt;%d", c, tree-&gt;down[0], tree-&gt;down[1], tree-&gt;down[2], rank, tree-&gt;up);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 198 | 1 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Ring %02d : %d -&gt; %d -&gt; %d", c, comm-&gt;channels[c].ring.prev, comm-&gt;rank, comm-&gt;channels[c].ring.next);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 199 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 200 | 0 / 调用 / 计算 / 写入 | <code>line[1023] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 201 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Trees%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 202 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(computeBuffSizes(comm));</code> | 失败时可能提前return，成功才继续；读取/选择 LL、LL128、Simple 的协议缓冲尺寸，为后续 channel 传输与分片提供容量。 |
| 203 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 204 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 205 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(setupChannel(comm, c, rank, nranks, rings+c*nranks), ret, affinity_restore);</code> | 失败时保存错误并跳到清理标签，成功才继续；把拓扑环顺序旋转到本 rank 开头，建立用户 rank 表及本 rank 内部环索引。 |
| 206 | 1 / 条件判断 | <code>(comm-&gt;nRanks == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 207 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 208 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 209 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, 1, &channel-&gt;ring.prev, 1, &channel-&gt;ring.next, 0), ret, affinity_restore);</code> | 失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。 |
| 210 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 211 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &ringGraph, 0), ret, affinity_restore);</code> | 失败时保存错误并跳到清理标签，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。 |
| 212 | 0 / 调用 / 计算 / 写入 | <code>free(rings);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 213 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Connected all rings");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 214 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 215 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 216 | 1 / 条件判断 | <code>(comm-&gt;nRanks == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 217 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 218 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 219 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, NCCL_MAX_TREE_ARITY, channel-&gt;tree.down, 1, &channel-&gt;tree.up, 0), ret, affinity_restore);</code> | 失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。 |
| 220 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, 1, &channel-&gt;tree.up, NCCL_MAX_TREE_ARITY, channel-&gt;tree.down, 0), ret, affinity_restore);</code> | 失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。 |
| 221 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 222 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &treeGraph, 0), ret, affinity_restore);</code> | 失败时保存错误并跳到清理标签，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。 |
| 223 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Connected all trees");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 224 | 0 / 条件判断 | <code>(comm-&gt;collNetSupport &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 225 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 226 | 2 / 准备局部状态 | <code>int collNetSetupFail = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 227 | 2 / 准备局部状态 | <code>int highestTypes[NCCL_MAX_LOCAL_RANKS] = {TRANSPORT_P2P};</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 228 | 2 / 准备局部状态 | <code>int nHeads = collNetGraph.nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 229 | 2 / 准备局部状态 | <code>int *heads;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 230 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&heads, nHeads));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 231 | 2 / 循环 | <code>初始 int c=0;；继续条件 c&lt;nHeads；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 232 | 3 / 调用 / 计算 / 写入 | <code>heads[c] = collNetGraph.intra[c*comm-&gt;localRanks+0];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 233 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 234 | 2 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 235 | 3 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 236 | 3 / 循环 | <code>初始 int h=0;；继续条件 h&lt;nHeads；每轮更新 h++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 237 | 4 / 准备局部状态 | <code>const int head = heads[h];</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 238 | 4 / 调用 / 计算 / 写入 | <code>collNetSetupFail = ncclTransportCollNetSetup(comm, &collNetGraph, channel, head, head, h, collNetRecv);</code> | 为网络归约 head 建立特殊 CollNet 连接并交换网络参与者句柄。 |
| 239 | 4 / 条件判断 | <code>(!collNetSetupFail)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 240 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 241 | 6 / 调用 / 计算 / 写入 | <code>collNetSetupFail = ncclTransportCollNetSetup(comm, &collNetGraph, channel, head, head, h, collNetSend);</code> | 为网络归约 head 建立特殊 CollNet 连接并交换网络参与者句柄。 |
| 242 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 243 | 3 / 条件判断 | <code>(c == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 244 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 245 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportCollNetCheck(comm, collNetSetupFail), ret, collnet_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；全组汇总 CollNet 建链失败状态，一处不支持时一致回退，避免各 rank 选择不一致。 |
| 246 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 247 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportCollNetCheck(comm, collNetSetupFail), ret, collnet_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；全组汇总 CollNet 建链失败状态，一处不支持时一致回退，避免各 rank 选择不一致。 |
| 248 | 2 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d Connected inter-node CollNet", rank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 249 | 2 / 准备局部状态 | <code>int highestTransportType0, highestTransportType1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 250 | 2 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 251 | 3 / 准备局部状态 | <code>struct ncclChannel* channelRecv = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 252 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channelRecv, NCCL_MAX_DIRECT_ARITY, channelRecv-&gt;collTree.up, NCCL_MAX_DIRECT_ARITY, channelRecv-&gt;collTree.down, 0), ret, collnet_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。 |
| 253 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 254 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 0, &highestTransportType0), ret, collnet_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。 |
| 255 | 2 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 256 | 3 / 准备局部状态 | <code>struct ncclChannel* channelSend = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 257 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channelSend, NCCL_MAX_DIRECT_ARITY, channelSend-&gt;collTree.down, NCCL_MAX_DIRECT_ARITY, channelSend-&gt;collTree.up, 1), ret, collnet_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。 |
| 258 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 259 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 1, &highestTransportType1), ret, collnet_cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。 |
| 260 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;intraHighestTransportType = highestTypes[comm-&gt;localRank] = highestTransportType0 &gt; highestTransportType1 ? highestTransportType0 : highestTransportType1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 261 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapIntraNodeAllGather(comm-&gt;bootstrap, comm-&gt;localRankToRank, comm-&gt;localRank, comm-&gt;localRanks, highestTypes, sizeof(int)));</code> | 失败时可能提前return，成功才继续；只在所选节点内 rank 集合交换固定大小控制信息，按局部索引映射到全局 rank。 |
| 262 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;localRanks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 263 | 3 / 条件判断 | <code>(highestTypes[i] &gt; comm-&gt;intraHighestTransportType)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 264 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 265 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;intraHighestTransportType = highestTypes[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 266 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 267 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "rank %d Connected CollNet", rank);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 268 | 2 / 跳转标签 | <code>collnet_cleanup</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 269 | 3 / 调用 / 计算 / 写入 | <code>free(heads);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 270 | 2 / 条件判断 | <code>(ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 271 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 272 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTransportCollNetFree(comm));</code> | 失败时可能提前return，成功才继续；关闭已创建的 CollNet 特殊连接，配合初始化失败回退或整体销毁。 |
| 273 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;collNetSupport = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 274 | 4 / 调用 / 计算 / 写入 | <code>ret = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 275 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d - CONNECTED %d RINGS AND TREES", rank, nranks, comm-&gt;nChannels);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 276 | 0 / 先执行一次的循环 | <code>初始 ；继续条件 (0)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 277 | 1 / 准备局部状态 | <code>int myCompCap = comm-&gt;peerInfo[rank].cudaCompCap;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 278 | 1 / 准备局部状态 | <code>int minCompCap = myCompCap, maxCompCap = myCompCap;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 279 | 1 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 280 | 2 / 调用 / 计算 / 写入 | <code>minCompCap = std::min(comm-&gt;peerInfo[i].cudaCompCap, minCompCap);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 281 | 2 / 调用 / 计算 / 写入 | <code>maxCompCap = std::max(comm-&gt;peerInfo[i].cudaCompCap, maxCompCap);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 282 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 283 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoTuneModel(comm, minCompCap, maxCompCap, &treeGraph, &ringGraph, &collNetGraph));</code> | 失败时可能提前return，成功才继续；结合拓扑图带宽、节点规模、GPU 架构和配置构建各 API/算法/协议的延迟与带宽表。 |
| 284 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 285 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoComputeP2pChannels(comm));</code> | 失败时可能提前return，成功才继续；收敛所有本地 GPU 到 peer 的通道预算，计算全局 P2P 空间与每 peer 数量并生成位反转偏移。 |
| 286 | 0 / 条件判断 | <code>(ncclParamNvbPreconnect())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 287 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 288 | 2 / 准备局部状态 | <code>int nvbNpeers;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 289 | 2 / 准备局部状态 | <code>int* nvbPeers;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 290 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNvbGpus(comm-&gt;topo, comm-&gt;rank, &nvbNpeers, &nvbPeers));</code> | 失败时可能提前return，成功才继续；找出可经一个中间 GPU 走 NVLink 桥接的 peer，为特定机内路径预连接提供候选。 |
| 291 | 2 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nvbNpeers；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 292 | 3 / 准备局部状态 | <code>int peer = nvbPeers[r];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 293 | 3 / 准备局部状态 | <code>int channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 294 | 3 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;p2pnChannelsPerPeer；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 295 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncSend, &channelId));</code> | 失败时可能提前return，成功才继续；组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。 |
| 296 | 4 / 条件判断 | <code>(comm-&gt;channels[channelId].peers[peer].send[1].connected == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 297 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 298 | 6 / 调用 / 计算 / 写入 | <code>comm-&gt;connectSend[peer] &#124;= (1&lt;&lt;channelId);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 299 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 300 | 3 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;p2pnChannelsPerPeer；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 301 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncRecv, &channelId));</code> | 失败时可能提前return，成功才继续；组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。 |
| 302 | 4 / 条件判断 | <code>(comm-&gt;channels[channelId].peers[peer].recv[1].connected == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 303 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 304 | 6 / 调用 / 计算 / 写入 | <code>comm-&gt;connectRecv[peer] &#124;= (1&lt;&lt;channelId);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 305 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 306 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 307 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTransportP2pSetup(comm, NULL, 1));</code> | 失败时可能提前return，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。 |
| 308 | 2 / 调用 / 计算 / 写入 | <code>free(nvbPeers);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 309 | 0 / 准备局部状态 | <code>struct ncclProxyConnector proxyConn;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 310 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, comm-&gt;rank, &proxyConn));</code> | 失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。 |
| 311 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&proxyConn, ncclProxyMsgSharedInit, &comm-&gt;p2pnChannels, sizeof(int), NULL, 0));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 312 | 0 / 条件判断 | <code>(ncclPxnDisable() == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 313 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 314 | 2 / 准备局部状态 | <code>int nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 315 | 2 / 准备局部状态 | <code>int* pxnPeers;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 316 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetPxnRanks(comm, &pxnPeers, &nranks));</code> | 失败时可能提前return，成功才继续；列出本 rank 网络路径可能依赖的中间 GPU rank，为共享代理资源提前准备。 |
| 317 | 2 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nranks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 318 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, pxnPeers[r], &proxyConn));</code> | 失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。 |
| 319 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&proxyConn, ncclProxyMsgSharedInit, &comm-&gt;p2pnChannels, sizeof(int), NULL, 0));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 320 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 321 | 2 / 调用 / 计算 / 写入 | <code>free(pxnPeers);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 322 | 0 / 先执行一次的循环 | <code>初始 ；继续条件 (0)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 323 | 1 / 准备局部状态 | <code>int intraProcRank0 = -1, intraProcRank = -1, intraProcRanks = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 324 | 1 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 325 | 2 / 条件判断 | <code>((comm-&gt;peerInfo[i].hostHash == comm-&gt;peerInfo[rank].hostHash) && (comm-&gt;peerInfo[i].pidHash == comm-&gt;peerInfo[rank].pidHash))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 326 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 327 | 4 / 条件判断 | <code>(intraProcRanks == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 328 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 329 | 6 / 调用 / 计算 / 写入 | <code>intraProcRank0 = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 330 | 4 / 条件判断 | <code>(i == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 331 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 332 | 6 / 调用 / 计算 / 写入 | <code>intraProcRank = intraProcRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 333 | 4 / 调用 / 计算 / 写入 | <code>intraProcRanks++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 334 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 335 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT,"pidHash[%d] %lx intraProcRank %d intraProcRanks %d intraProcRank0 %d", rank, comm-&gt;peerInfo[rank].pidHash, intraProcRank, intraProcRanks, intraProcRank0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 336 | 1 / 条件判断 | <code>(intraProcRank == -1 &#124;&#124; intraProcRank0 == -1 &#124;&#124; comm-&gt;peerInfo[intraProcRank0].comm == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 337 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 338 | 3 / 调用 / 计算 / 写入 | <code>WARN("Failed to determine intra proc ranks rank %d hostHash %lx pidHash %lx intraProcRank %d intraProcRanks %d intraProcRank0 %d", rank, comm-&gt;peerInfo[rank].hostHash, comm-&gt;peerInfo[rank].pidHash, intraProcRank, intraProcRanks, intraProcRank0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 339 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 340 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCommSetIntraProc(comm, intraProcRank, intraProcRanks, comm-&gt;peerInfo[intraProcRank0].comm));</code> | 失败时可能提前return，成功才继续；把同进程 rank 加入本地发射协调组，设置屏障、设备列表及协作发射能力。 |
| 341 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 342 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapBarrier(comm-&gt;bootstrap, comm-&gt;localRankToRank, comm-&gt;localRank, comm-&gt;localRanks, comm-&gt;localRankToRank[0]));</code> | 失败时可能提前return，成功才继续；使用倍增距离的控制消息交换，使所选 rank 子集合完成初始化会合。 |
| 343 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyShmUnlink(comm));</code> | 失败时可能提前return，成功才继续；移除代理共享内存对象名字，已有映射仍按各自引用和退出流程释放。 |
| 344 | 0 / 跳转标签 | <code>affinity_restore</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 345 | 1 / 条件判断 | <code>(CPU_COUNT(&comm-&gt;cpuAffinity))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 346 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 347 | 3 / 调用 / 计算 / 写入 | <code>sched_setaffinity(0, sizeof(cpu_set_t), &affinitySave);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 348 | 0 / 条件判断 | <code>(ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 349 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 350 | 2 / 函数返回 | <code>return ret;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 351 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d - DONE", rank, nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 352 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // We use 2 AllGathers
  // 1. { peerInfo, comm, compCap}
  // 2. { nChannels, graphInfo, topoRanks }

  int rank = comm->rank;
  int nranks = comm->nRanks;
  uint64_t commHash = getHash(commId->internal, NCCL_UNIQUE_ID_BYTES);
  TRACE(NCCL_INIT, "comm %p, commHash %lx, rank %d nranks %d - BEGIN", comm, commHash, rank, nranks);
  NCCLCHECK(bootstrapInit(commId, comm));

/* 中文导读：
 * 这一轮 AllGather 是 bootstrap 控制面操作，交换 peerInfo，而非用户调用的 GPU AllGather 集合通信。
 * 使用 hostHash/pidHash/busId 等识别主机、进程和 GPU，为去重、路径选择和 IPC 建立依据。
 */
  // AllGather1 - begin
  NCCLCHECK(ncclCalloc(&comm->peerInfo, nranks+1)); // Extra rank to represent CollNet root
  NCCLCHECK(fillInfo(comm, comm->peerInfo+rank, commHash));
  NCCLCHECK(bootstrapAllGather(comm->bootstrap, comm->peerInfo, sizeof(struct ncclPeerInfo)));

  for (int i = 0; i < nranks; i++) {
    if ((i != rank) && (comm->peerInfo[i].hostHash == comm->peerInfo[rank].hostHash) && (comm->peerInfo[i].busId == comm->peerInfo[rank].busId)) {
      WARN("Duplicate GPU detected : rank %d and rank %d both on CUDA device %lx", rank, i, comm->peerInfo[rank].busId);
      return ncclInvalidUsage;
    }
  }

  // AllGather1 - end

/* 中文导读：
 * 先算路径、裁剪不可达/无用节点，再重算路径；搜索器只在有效硬件子图上工作。
 * GPU 物理互联能力、网卡亲和性与后端 GDR 支持共同影响最终路径。
 */
  // Topo detection / System graph creation
  NCCLCHECK(ncclTopoGetSystem(comm, &comm->topo));
  // Compute paths between GPUs and NICs
  NCCLCHECK(ncclTopoComputePaths(comm->topo, comm->peerInfo));
  // Remove inaccessible GPUs and unused NICs
  NCCLCHECK(ncclTopoTrimSystem(comm->topo, comm));
  // Recompute paths after trimming
  NCCLCHECK(ncclTopoComputePaths(comm->topo, comm->peerInfo));
  // Init search
  NCCLCHECK(ncclTopoSearchInit(comm->topo));
  // Print final topology
  NCCLCHECK(ncclTopoPrint(comm->topo));

  // Set Affinity to a CPU local the our GPU, so that all memory we allocate
  // on the host is local.
  NCCLCHECK(ncclTopoGetCpuAffinity(comm->topo, comm->rank, &comm->cpuAffinity));
  cpu_set_t affinitySave;
  if (CPU_COUNT(&comm->cpuAffinity)) {
    sched_getaffinity(0, sizeof(cpu_set_t), &affinitySave);
    sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);
  }
  ncclResult_t ret;

  // Launch proxy service thread
  NCCLCHECK(ncclProxyCreate(comm));

/* 中文导读：
 * 分别搜索 Ring、Balanced Tree 和 CollNet 图。本版本 Ring 初始搜索上限是 MAXCHANNELS/2。
 * 后续 Postset 会拼接节点并可能复制 channel；最终 comm->nChannels 不能只看这里的 maxChannels。
 */
  // Get rings and trees
  struct ncclTopoGraph ringGraph;
  ringGraph.id = 0;
  ringGraph.pattern = NCCL_TOPO_PATTERN_RING;
  ringGraph.collNet = 0;
  ringGraph.minChannels = 1;
  ringGraph.maxChannels = MAXCHANNELS/2;
  NCCLCHECK(ncclTopoCompute(comm->topo, &ringGraph));
  NCCLCHECK(ncclTopoPrintGraph(comm->topo, &ringGraph));

  struct ncclTopoGraph treeGraph;
  treeGraph.id = 1;
  treeGraph.pattern = NCCL_TOPO_PATTERN_BALANCED_TREE;
  treeGraph.collNet = 0;
  treeGraph.minChannels = 1;
  treeGraph.maxChannels = ringGraph.nChannels;
  NCCLCHECK(ncclTopoCompute(comm->topo, &treeGraph));
  NCCLCHECK(ncclTopoPrintGraph(comm->topo, &treeGraph));

  struct ncclTopoGraph collNetGraph;
  collNetGraph.id = 2;
  collNetGraph.pattern = NCCL_TOPO_PATTERN_TREE;
  collNetGraph.collNet = 1;
  collNetGraph.minChannels = collNetGraph.maxChannels = ringGraph.nChannels;
  NCCLCHECK(ncclTopoCompute(comm->topo, &collNetGraph));
  NCCLCHECK(ncclTopoPrintGraph(comm->topo, &collNetGraph));

  if (comm->rank == ncclParamGraphDumpFileRank()) {
    struct ncclTopoGraph* graphs[3] = { &ringGraph, &treeGraph, &collNetGraph };
    NCCLCHECK(ncclTopoDumpGraphs(comm->topo, 3, graphs));
  }

  // Determine local CollNet support before all-gather
  if (collNetSupport()) {
    char *collNetEnable = getenv("NCCL_COLLNET_ENABLE");
    if (collNetEnable != NULL) {
      INFO(NCCL_ALL, "NCCL_COLLNET_ENABLE set by environment to %s.", collNetEnable);
      if (strcmp(collNetEnable, "1") == 0) {
        comm->collNetSupport = 1;
      }
    }
  }
  if (comm->collNetSupport == 1 && collNetGraph.nChannels <= 0) comm->collNetSupport = 0;

  // AllGather3 - begin
  struct ncclGraphInfo {
    int pattern;
    int nChannels;
    int sameChannels;
    float speedIntra;
    float speedInter;
    int typeIntra;
    int typeInter;
  };

  struct {
    int netDev;
    int collNetSupport;
    struct ncclGraphInfo tree;
    struct ncclGraphInfo ring;
    struct ncclGraphInfo collNet;
    struct ncclTopoRanks topoRanks;
  } *allGather3Data;

  NCCLCHECK(ncclCalloc(&allGather3Data, nranks));
  NCCLCHECK(ncclTopoGetLocalNet(comm->topo, rank, &allGather3Data[rank].netDev));
  allGather3Data[rank].tree.pattern = treeGraph.pattern;
  allGather3Data[rank].tree.nChannels = treeGraph.nChannels;
  allGather3Data[rank].tree.sameChannels = treeGraph.sameChannels;
  allGather3Data[rank].tree.speedIntra = treeGraph.speedIntra;
  allGather3Data[rank].tree.speedInter = treeGraph.speedInter;
  allGather3Data[rank].tree.typeIntra = treeGraph.typeIntra;
  allGather3Data[rank].tree.typeInter = treeGraph.typeInter;
  allGather3Data[rank].ring.pattern = ringGraph.pattern;
  allGather3Data[rank].ring.nChannels = ringGraph.nChannels;
  allGather3Data[rank].ring.sameChannels = ringGraph.sameChannels;
  allGather3Data[rank].ring.speedIntra = ringGraph.speedIntra;
  allGather3Data[rank].ring.speedInter = ringGraph.speedInter;
  allGather3Data[rank].ring.typeIntra = ringGraph.typeIntra;
  allGather3Data[rank].ring.typeInter = ringGraph.typeInter;
  allGather3Data[rank].collNet.pattern = collNetGraph.pattern;
  allGather3Data[rank].collNet.nChannels = collNetGraph.nChannels;
  allGather3Data[rank].collNet.sameChannels = collNetGraph.sameChannels;
  allGather3Data[rank].collNet.speedIntra = collNetGraph.speedIntra;
  allGather3Data[rank].collNet.speedInter = collNetGraph.speedInter;
  allGather3Data[rank].collNet.typeIntra = collNetGraph.typeIntra;
  allGather3Data[rank].collNet.typeInter = collNetGraph.typeInter;
  allGather3Data[rank].collNetSupport = comm->collNetSupport;

  comm->nChannels = std::min(treeGraph.nChannels, ringGraph.nChannels);
  NCCLCHECK(ncclTopoPreset(comm, &treeGraph, &ringGraph, &allGather3Data[rank].topoRanks));

  NCCLCHECK(bootstrapAllGather(comm->bootstrap, allGather3Data, sizeof(*allGather3Data)));

  // Determine nNodes, firstRanks, ...
  int *nodesFirstRank, *nodesTreePatterns;
  NCCLCHECK(ncclCalloc(&nodesFirstRank, nranks));
  NCCLCHECK(ncclCalloc(&nodesTreePatterns, nranks));
  NCCLCHECK(ncclCalloc(&comm->rankToNode, comm->nRanks));
  for (int r=0; r<nranks; r++) {
    int node;
    int firstRank = allGather3Data[r].topoRanks.ringRecv[0];
    for (node=0; node<comm->nNodes && nodesFirstRank[node] != firstRank; node++);
    if (node == comm->nNodes) {
      comm->nNodes++;
      nodesFirstRank[node] = firstRank;
      // Record tree pattern of each node as they can be different depending on sm arch
      nodesTreePatterns[node] = allGather3Data[r].tree.pattern;
    }
    comm->rankToNode[r] = node;
  }
  // Now that we know nNodes, alloc nodeRanks and compute localRanks for each node
  NCCLCHECK(ncclCalloc(&comm->nodeRanks, comm->nNodes));
  NCCLCHECK(ncclCalloc(&comm->rankToLocalRank, comm->nRanks));
  for (int r=0; r<comm->nRanks; r++) {
    int node = comm->rankToNode[r];
    comm->rankToLocalRank[r] = comm->nodeRanks[node].localRanks;
    comm->nodeRanks[node].localRanks++;
  }
  // Allocate ranks arrays for each node
  for (int n=0; n<comm->nNodes; n++) {
    NCCLCHECK(ncclCalloc(&comm->nodeRanks[n].localRankToRank, comm->nodeRanks[n].localRanks));
    comm->maxLocalRanks = std::max(comm->maxLocalRanks, comm->nodeRanks[n].localRanks);
    comm->nodeRanks[n].localRanks = 0;
  }
  // And fill the ranks arrays
  for (int r=0; r<comm->nRanks; r++) {
    int node = comm->rankToNode[r];
    comm->nodeRanks[node].localRankToRank[comm->nodeRanks[node].localRanks++] = r;
  }
  comm->node = comm->rankToNode[rank];
  comm->localRankToRank = comm->nodeRanks[comm->node].localRankToRank;
  comm->localRank = comm->rankToLocalRank[rank];
  comm->localRanks = comm->nodeRanks[comm->node].localRanks;

  TRACE(NCCL_INIT,"hostHash[%d] %lx localRank %d localRanks %d localRank0 %d",
        rank, comm->peerInfo[rank].hostHash, comm->localRank, comm->localRanks, comm->localRankToRank[0]);
  if (comm->localRank == -1 || comm->localRankToRank[0] == -1 || comm->localRanks == 0) {
    WARN("Failed to determine local ranks rank %d hostHash %lx pidHash %lx localRank %d localRanks %d localRank0 %d",
         rank, comm->peerInfo[rank].hostHash, comm->peerInfo[rank].pidHash,
         comm->localRank, comm->localRanks, comm->localRankToRank[0]);
    return ncclInternalError;
  }

  int nChannelsOrig = comm->nChannels;
  struct ncclTopoRanks** allTopoRanks;
  NCCLCHECK(ncclCalloc(&allTopoRanks, comm->nRanks));
  for (int i=0; i<nranks; i++) {
    comm->peerInfo[i].netDev = allGather3Data[i].netDev;
    allTopoRanks[i] = &allGather3Data[i].topoRanks;
    // Make sure we align all ranks so that the tuning is consistent across ranks
    treeGraph.nChannels = std::min(allGather3Data[i].tree.nChannels, treeGraph.nChannels);
    treeGraph.sameChannels = std::min(allGather3Data[i].tree.sameChannels, treeGraph.sameChannels);
    treeGraph.speedIntra = std::min(allGather3Data[i].tree.speedIntra, treeGraph.speedIntra);
    treeGraph.speedInter = std::min(allGather3Data[i].tree.speedInter, treeGraph.speedInter);
    treeGraph.typeIntra = std::max(allGather3Data[i].tree.typeIntra, treeGraph.typeIntra);
    treeGraph.typeInter = std::max(allGather3Data[i].tree.typeInter, treeGraph.typeInter);
    ringGraph.nChannels = std::min(allGather3Data[i].ring.nChannels, ringGraph.nChannels);
    ringGraph.sameChannels = std::min(allGather3Data[i].ring.sameChannels, ringGraph.sameChannels);
    ringGraph.speedIntra = std::min(allGather3Data[i].ring.speedIntra, ringGraph.speedIntra);
    ringGraph.speedInter = std::min(allGather3Data[i].ring.speedInter, ringGraph.speedInter);
    ringGraph.typeIntra = std::max(allGather3Data[i].ring.typeIntra, ringGraph.typeIntra);
    ringGraph.typeInter = std::max(allGather3Data[i].ring.typeInter, ringGraph.typeInter);
    collNetGraph.nChannels = std::min(allGather3Data[i].collNet.nChannels, collNetGraph.nChannels);
    collNetGraph.sameChannels = std::min(allGather3Data[i].collNet.sameChannels, collNetGraph.sameChannels);
    collNetGraph.speedIntra = std::min(allGather3Data[i].collNet.speedIntra, collNetGraph.speedIntra);
    collNetGraph.speedInter = std::min(allGather3Data[i].collNet.speedInter, collNetGraph.speedInter);
    collNetGraph.typeIntra = std::max(allGather3Data[i].collNet.typeIntra, collNetGraph.typeIntra);
    collNetGraph.typeInter = std::max(allGather3Data[i].collNet.typeInter, collNetGraph.typeInter);
    comm->collNetSupport = std::min(allGather3Data[i].collNetSupport, comm->collNetSupport);
  }

/* 中文精读：跨 rank 收敛后的 channel 数
 * 各 rank/节点的本地图搜索结果可能不同，前面的 AllGather 后用最小可用通道/速度收敛。
 * 这里把 Ring/Tree 对齐到共同数量；后续 TopoPostset 还会复制通道并应用 MIN/MAX 约束。
 * 所以 graph 搜索日志的 nChannels、最终 comm->nChannels、本次 API 的 info->nChannels
 * 处于三个阶段，不能拿其中一个数代表另外两个。
 */
  comm->nChannels = treeGraph.nChannels = ringGraph.nChannels = std::min(treeGraph.nChannels, ringGraph.nChannels);
  if (comm->nChannels < nChannelsOrig) {
    // We started duplicating channels during Preset(), so we need to move the
    // duplicated channels since we have removed some.
    for (int i=0; i<comm->nChannels; i++) memcpy(comm->channels+comm->nChannels+i, comm->channels+nChannelsOrig+i, sizeof(struct ncclChannel));
  }

  // Determine CollNet support after all-gather now that we know nNodes and each node localRanks
  if (comm->collNetSupport == 1) {
    int collNetNodeThreshold = ncclParamCollNetNodeThreshold();
    if (comm->nNodes < collNetNodeThreshold) {
      INFO(NCCL_INIT, "Communicator has %d nodes which is less than CollNet node threshold %d, disabling CollNet", comm->nNodes, collNetNodeThreshold);
      comm->collNetSupport = 0;
    }
    for (int n=0; n<comm->nNodes; n++) {
      if (comm->nodeRanks[n].localRanks > NCCL_MAX_DIRECT_ARITY+1) {
        WARN("CollNet currently only supports up to %d GPUs per node, disabling CollNet", NCCL_MAX_DIRECT_ARITY+1);
        comm->collNetSupport = 0;
        break;
      }
    }
  }

  int *rings;
  NCCLCHECK(ncclCalloc(&rings, nranks*MAXCHANNELS));
  NCCLCHECK(ncclTopoPostset(comm, nodesFirstRank, nodesTreePatterns, allTopoRanks, rings, &collNetGraph));

  free(allTopoRanks);
  free(nodesTreePatterns);
  free(nodesFirstRank);
  free(allGather3Data);

  // AllGather3 - end

  TRACE(NCCL_INIT, "rank %d nranks %d - BUILT %d TREES/RINGS", rank, nranks, comm->nChannels);

  char line[1024];
  line[0]='\0';
  for (int c=0; c<comm->nChannels; c++) {
    struct ncclTree* tree = &comm->channels[c].tree;
    snprintf(line+strlen(line), 1023-strlen(line), " [%d] %d/%d/%d->%d->%d",
        c, tree->down[0], tree->down[1], tree->down[2], rank, tree->up);
    INFO(NCCL_GRAPH, "Ring %02d : %d -> %d -> %d", c, comm->channels[c].ring.prev, comm->rank, comm->channels[c].ring.next);
  }
  line[1023] = '\0';
  INFO(NCCL_INIT, "Trees%s", line);

  NCCLCHECK(computeBuffSizes(comm));

/* 中文导读：
 * 每个 channel 先标记 Ring 前驱接收/后继发送，再统一 Setup 交换句柄并建链。
 * connectSend/connectRecv 是待建链位图；已 connected 的同槽连接不重复创建。
 */
  // Connect with prev/next for each ring
  for (int c=0; c<comm->nChannels; c++) {
    struct ncclChannel* channel = comm->channels+c;
    NCCLCHECKGOTO(setupChannel(comm, c, rank, nranks, rings+c*nranks), ret, affinity_restore);
    if (comm->nRanks == 1) continue;
    NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, 1, &channel->ring.prev, 1, &channel->ring.next, 0), ret, affinity_restore);
  }
  NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &ringGraph, 0), ret, affinity_restore);
  free(rings);
  INFO(NCCL_INIT, "Connected all rings");

/* 中文导读：
 * 在同一个连接索引 0 上补齐 Tree 的父子双向边。
 * 若 Ring 已经连接了相同 channel/peer/方向，连接标记逻辑会复用该对象，不能简单相加 Ring 与 Tree 边数。
 */
  // Connect Trees
  for (int c=0; c<comm->nChannels; c++) {
    struct ncclChannel* channel = comm->channels+c;
    if (comm->nRanks == 1) continue;
    NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, NCCL_MAX_TREE_ARITY, channel->tree.down, 1, &channel->tree.up, 0), ret, affinity_restore);
    NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, 1, &channel->tree.up, NCCL_MAX_TREE_ARITY, channel->tree.down, 0), ret, affinity_restore);
  }
  NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &treeGraph, 0), ret, affinity_restore);
  INFO(NCCL_INIT, "Connected all trees");

  // Check if we can setup CollNet
  if (comm->collNetSupport > 0) {
    int collNetSetupFail = 0;
    int highestTypes[NCCL_MAX_LOCAL_RANKS] = {TRANSPORT_P2P};
    // Find all head ranks
    int nHeads = collNetGraph.nChannels;
    int *heads;
    NCCLCHECK(ncclCalloc(&heads, nHeads));
    // Head GPU index is always 0
    for (int c=0; c<nHeads; c++) {
      heads[c] = collNetGraph.intra[c*comm->localRanks+0];
    }
    for (int c=0; c<comm->nChannels; c++) {
      struct ncclChannel* channel = comm->channels+c;
      for (int h=0; h<nHeads; h++) {
        const int head = heads[h];
        collNetSetupFail = ncclTransportCollNetSetup(comm, &collNetGraph, channel, head, head, h, collNetRecv);
        if (!collNetSetupFail) collNetSetupFail = ncclTransportCollNetSetup(comm, &collNetGraph, channel, head, head, h, collNetSend);
      }
      // Verify CollNet setup across ranks after trying the first channel
      if (c == 0) {
        NCCLCHECKGOTO(ncclTransportCollNetCheck(comm, collNetSetupFail), ret, collnet_cleanup);
      }
    }
    // Verify CollNet setup across ranks after trying all channels
    NCCLCHECKGOTO(ncclTransportCollNetCheck(comm, collNetSetupFail), ret, collnet_cleanup);
    TRACE(NCCL_INIT, "rank %d Connected inter-node CollNet", rank);

    // Connect intra-node CollNet
    int highestTransportType0, highestTransportType1;
    for (int c=0; c<comm->nChannels; c++) {
      struct ncclChannel* channelRecv = comm->channels+c;
      NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channelRecv, NCCL_MAX_DIRECT_ARITY, channelRecv->collTree.up, NCCL_MAX_DIRECT_ARITY, channelRecv->collTree.down, 0), ret, collnet_cleanup);
    }
    NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 0, &highestTransportType0), ret, collnet_cleanup);
    for (int c=0; c<comm->nChannels; c++) {
      struct ncclChannel* channelSend = comm->channels+c;
      NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channelSend, NCCL_MAX_DIRECT_ARITY, channelSend->collTree.down, NCCL_MAX_DIRECT_ARITY, channelSend->collTree.up, 1), ret, collnet_cleanup);
    }
    NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 1, &highestTransportType1), ret, collnet_cleanup);

    // Exchange highest intra-node transport type among ranks
    // because we need to know whether all ranks can p2p each other to determine whether we can directly read/write registered user buffer
    comm->intraHighestTransportType = highestTypes[comm->localRank] = highestTransportType0 > highestTransportType1 ? highestTransportType0 : highestTransportType1;
    NCCLCHECK(bootstrapIntraNodeAllGather(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, highestTypes, sizeof(int)));
    for (int i=0; i<comm->localRanks; i++) {
      if (highestTypes[i] > comm->intraHighestTransportType)
        comm->intraHighestTransportType = highestTypes[i];
    }
    INFO(NCCL_INIT, "rank %d Connected CollNet", rank);

collnet_cleanup:
    free(heads);
    if (ret != ncclSuccess) {
      NCCLCHECK(ncclTransportCollNetFree(comm));
      comm->collNetSupport = 0;
      ret = ncclSuccess;
    }
  }
  TRACE(NCCL_INIT, "rank %d nranks %d - CONNECTED %d RINGS AND TREES", rank, nranks, comm->nChannels);

  // Compute time models for algorithm and protocol combinations
  do {
    int myCompCap = comm->peerInfo[rank].cudaCompCap;
    int minCompCap = myCompCap, maxCompCap = myCompCap;
    for (int i = 0; i < nranks; i++) {
      minCompCap = std::min(comm->peerInfo[i].cudaCompCap, minCompCap);
      maxCompCap = std::max(comm->peerInfo[i].cudaCompCap, maxCompCap);
    }
    NCCLCHECK(ncclTopoTuneModel(comm, minCompCap, maxCompCap, &treeGraph, &ringGraph, &collNetGraph));
  } while(0);

/* 中文导读：
 * 计算 P2P 总 channel 空间与每 peer channel 数，生成映射；这一步不等于对所有 rank 全互联。
 * 普通 Send/Recv 还会在任务出现时记录待连接位图，由 GroupEnd 预连接路径处理。
 */
  // Compute nChannels per peer for p2p
  NCCLCHECK(ncclTopoComputeP2pChannels(comm));

  if (ncclParamNvbPreconnect()) {
    // Connect p2p when using NVB path
    int nvbNpeers;
    int* nvbPeers;
    NCCLCHECK(ncclTopoGetNvbGpus(comm->topo, comm->rank, &nvbNpeers, &nvbPeers));
    for (int r=0; r<nvbNpeers; r++) {
      int peer = nvbPeers[r];
      int channelId;
      for (int c=0; c<comm->p2pnChannelsPerPeer; c++) {
        NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncSend, &channelId));
        if (comm->channels[channelId].peers[peer].send[1].connected == 0) {
          comm->connectSend[peer] |= (1<<channelId);
        }
      }
      for (int c=0; c<comm->p2pnChannelsPerPeer; c++) {
        NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncRecv, &channelId));
        if (comm->channels[channelId].peers[peer].recv[1].connected == 0) {
          comm->connectRecv[peer] |= (1<<channelId);
        }
      }
    }
    NCCLCHECK(ncclTransportP2pSetup(comm, NULL, 1));
    free(nvbPeers);
  }

  // Connect to local net proxy
  struct ncclProxyConnector proxyConn;
  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, comm->rank, &proxyConn));
  NCCLCHECK(ncclProxyCall(&proxyConn, ncclProxyMsgSharedInit, &comm->p2pnChannels, sizeof(int), NULL, 0));

  // Then to remote ones when using PXN
  if (ncclPxnDisable() == 0) {
    int nranks;
    int* pxnPeers;
    NCCLCHECK(ncclTopoGetPxnRanks(comm, &pxnPeers, &nranks));
    for (int r=0; r<nranks; r++) {
      NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, pxnPeers[r], &proxyConn));
      NCCLCHECK(ncclProxyCall(&proxyConn, ncclProxyMsgSharedInit, &comm->p2pnChannels, sizeof(int), NULL, 0));
    }
    free(pxnPeers);
  }

  do {
    // Compute intra-process ranks
    int intraProcRank0 = -1, intraProcRank = -1, intraProcRanks = 0;
    for (int i = 0; i < nranks; i++) {
      if ((comm->peerInfo[i].hostHash == comm->peerInfo[rank].hostHash)
          && (comm->peerInfo[i].pidHash == comm->peerInfo[rank].pidHash)) {
        // Rank is in same process
        if (intraProcRanks == 0) intraProcRank0 = i;
        if (i == rank) intraProcRank = intraProcRanks;
        intraProcRanks++;
      }
    }
    TRACE(NCCL_INIT,"pidHash[%d] %lx intraProcRank %d intraProcRanks %d intraProcRank0 %d",
        rank, comm->peerInfo[rank].pidHash, intraProcRank, intraProcRanks, intraProcRank0);
    if (intraProcRank == -1 || intraProcRank0 == -1 || comm->peerInfo[intraProcRank0].comm == NULL) {
      WARN("Failed to determine intra proc ranks rank %d hostHash %lx pidHash %lx intraProcRank %d intraProcRanks %d intraProcRank0 %d",
          rank, comm->peerInfo[rank].hostHash, comm->peerInfo[rank].pidHash,
          intraProcRank, intraProcRanks, intraProcRank0);
      return ncclInternalError;
    }
    NCCLCHECK(ncclCommSetIntraProc(comm, intraProcRank, intraProcRanks, comm->peerInfo[intraProcRank0].comm));
  } while(0);

  /* Local intra-node barrier */
  NCCLCHECK(bootstrapBarrier(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, comm->localRankToRank[0]));

  // Unlink proxy shm to make sure it will be properly cleaned up.
  NCCLCHECK(ncclProxyShmUnlink(comm));

  // We should have allocated all buffers, collective fifos, ... we can
  // restore the affinity.
affinity_restore:
  if (CPU_COUNT(&comm->cpuAffinity)) sched_setaffinity(0, sizeof(cpu_set_t), &affinitySave);
  if (ret != ncclSuccess) return ret;

  TRACE(NCCL_INIT, "rank %d nranks %d - DONE", rank, nranks);
  return ncclSuccess;
}
```

</details>

<a id="f0896"></a>

## F0896：ncclCommInitRankSync

用途：同步建立一个 rank 的 communicator，完成设备选择、总对象分配、传输初始化和设备视图构造。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclResult_t res;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaSetDevice(cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 条件判断 | <code>(maxLocalSizeBytes &gt; 0 && ncclParamSetStackSize() == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "Setting cudaLimitStackSize to %zi", maxLocalSizeBytes);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECKIGNORE(cudaDeviceSetLimit(cudaLimitStackSize, maxLocalSizeBytes));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>*newcomm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(commAlloc(newcomm, nranks, myrank), res, cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；分配并初始化当前 rank 的 communicator 总对象、排队状态、事件与连接需求表。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(initTransportsRank(*newcomm, &commId), res, cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；串联 bootstrap、硬件建图、路径搜索、全局图拼接、传输建链和性能模型，建立当前 rank 通信环境。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(devCommSetup(*newcomm), res, cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；构造设备端精简 communicator 和连续 channel 布局，把主机连接视图转换成 GPU 可访问地址。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Init COMPLETE", *newcomm, myrank, nranks, (*newcomm)-&gt;cudaDev, (*newcomm)-&gt;busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 跳转标签 | <code>cleanup</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 14 | 1 / 条件判断 | <code>((*newcomm) && (*newcomm)-&gt;bootstrap)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>bootstrapAbort((*newcomm)-&gt;bootstrap);</code> | 异常退出时关闭控制网络并回收状态，配合中止流程打破初始化等待。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>*newcomm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 函数返回 | <code>return res;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclResult_t res;

  CUDACHECK(cudaSetDevice(cudaDev));
  // Set the maximum kernel stack size of all kernels to avoid
  // a CUDA memory reconfig on load (c.f. NVSHMEM issue)
  if (maxLocalSizeBytes > 0 && ncclParamSetStackSize() == 1) {
    TRACE(NCCL_INIT, "Setting cudaLimitStackSize to %zi", maxLocalSizeBytes);
    CUDACHECKIGNORE(cudaDeviceSetLimit(cudaLimitStackSize, maxLocalSizeBytes));
  }
  *newcomm = NULL;
  NCCLCHECKGOTO(commAlloc(newcomm, nranks, myrank), res, cleanup);
  NCCLCHECKGOTO(initTransportsRank(*newcomm, &commId), res, cleanup);
  NCCLCHECKGOTO(devCommSetup(*newcomm), res, cleanup);

  INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Init COMPLETE", *newcomm, myrank, nranks, (*newcomm)->cudaDev, (*newcomm)->busId);

  return ncclSuccess;
cleanup:
  if ((*newcomm) && (*newcomm)->bootstrap) bootstrapAbort((*newcomm)->bootstrap);
  *newcomm = NULL;
  return res;
}
```

</details>

<a id="f0897"></a>

## F0897：ncclCommInitRankDev

用途：按是否处于 Group 选择立即初始化或保存异步初始化任务，并处理设备上下文。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclResult_t res;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>char* env = getenv("NCCL_COMM_ID");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(env && myrank == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", env);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(bootstrapCreateRoot(&commId, true), res, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；创建 bootstrap root 监听对象和后台线程，为后续 rank 会合提供入口。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclInit(), res, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；进程级单次初始化配置、网络与相关支持库，避免每个 communicator 重复初始化全局组件。 |
| 8 | 0 / 条件判断 | <code>(myrank == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>showVersion();</code> | 按日志配置打印库版本和构建相关信息，辅助确认实际加载的库。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaFree(NULL), res, end);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(PtrCheck(newcomm, "CommInitRank", "newcomm"), res, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 13 | 0 / 条件判断 | <code>(nranks &lt; 1 &#124;&#124; myrank &lt; 0 &#124;&#124; myrank &gt;= nranks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>WARN("Invalid rank requested : %d/%d", myrank, nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>res = ncclInvalidArgument;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 控制转移 | <code>goto end;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 18 | 0 / 条件判断 | <code>(ncclAsyncMode())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclAsyncInit(ncclCommInitRankSync, newcomm, nranks, commId, myrank, cudaDev), res, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；把初始化函数、rank、设备和 uniqueId 复制到本线程 Group 任务数组。 |
| 21 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclCommInitRankSync(newcomm, nranks, commId, myrank, cudaDev), res, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；同步建立一个 rank 的 communicator，完成设备选择、总对象分配、传输初始化和设备视图构造。 |
| 23 | 0 / 跳转标签 | <code>end</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 24 | 1 / 条件判断 | <code>(ncclAsyncMode())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 3 / 函数返回 | <code>return ncclAsyncErrCheck(res);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 27 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 28 | 3 / 函数返回 | <code>return res;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclResult_t res;
  char* env = getenv("NCCL_COMM_ID");
  if (env && myrank == 0) {
    INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", env);
    NCCLCHECKGOTO(bootstrapCreateRoot(&commId, true), res, end);
  }

  NCCLCHECKGOTO(ncclInit(), res, end);
  if (myrank == 0) showVersion();

  // Make sure the CUDA runtime is initialized.
  CUDACHECKGOTO(cudaFree(NULL), res, end);

  NCCLCHECKGOTO(PtrCheck(newcomm, "CommInitRank", "newcomm"), res, end);
  if (nranks < 1 || myrank < 0 || myrank >= nranks) {
    WARN("Invalid rank requested : %d/%d", myrank, nranks);
    res = ncclInvalidArgument;
    goto end;
  }

  if (ncclAsyncMode()) {
    NCCLCHECKGOTO(ncclAsyncInit(ncclCommInitRankSync, newcomm, nranks, commId, myrank, cudaDev), res, end);
  } else {
    NCCLCHECKGOTO(ncclCommInitRankSync(newcomm, nranks, commId, myrank, cudaDev), res, end);
  }

end:
  if (ncclAsyncMode()) return ncclAsyncErrCheck(res);
  else return res;
}
```

</details>

<a id="f0898"></a>

## F0898：ncclCommInitRank

用途：公开多进程/多机初始化入口，使用当前设备、全组规模和 uniqueId 建立当前 rank。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>int cudaDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCommInitRankDev(newcomm, nranks, commId, myrank, cudaDev));</code> | 失败时可能提前return，成功才继续；按是否处于 Group 选择立即初始化或保存异步初始化任务，并处理设备上下文。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  int cudaDev;
  CUDACHECK(cudaGetDevice(&cudaDev));
  NCCLCHECK(ncclCommInitRankDev(newcomm, nranks, commId, myrank, cudaDev));
  return ncclSuccess;
}
```

</details>

<a id="f0899"></a>

## F0899：ncclCommInitAll

用途：单进程本地多 GPU 便捷初始化，使用同一 uniqueId 和 Group 协调各设备加入同一通信组。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(comms, "CommInitAll", "comms"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 3 | 0 / 条件判断 | <code>(ndev &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>WARN("Invalid device count requested : %d", ndev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>ncclUniqueId uniqueId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGetUniqueId(&uniqueId));</code> | 失败时可能提前return，成功才继续；先完成库级初始化，再生成应用需要分发给全组 rank 的 bootstrap 标识。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGroupStart());</code> | 失败时可能提前return，成功才继续；进入线程局部 Group 嵌套，最外层首次进入时清理组任务状态。 |
| 10 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ndev；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>ncclCommInitRankDev(comms+i, ndev, uniqueId, i, devlist ? devlist[i] : i);</code> | 按是否处于 Group 选择立即初始化或保存异步初始化任务，并处理设备上下文。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGroupEnd());</code> | 失败时可能提前return，成功才继续；最外层出口统一执行初始化、P2P 预建链、消息分块、collective 聚合、Proxy 发布和 kernel 发射。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  NCCLCHECK(PtrCheck(comms, "CommInitAll", "comms"));
  if (ndev < 0) {
    WARN("Invalid device count requested : %d", ndev);
    return ncclInvalidArgument;
  }

  ncclUniqueId uniqueId;
  NCCLCHECK(ncclGetUniqueId(&uniqueId));
  NCCLCHECK(ncclGroupStart());
  for (int i=0; i<ndev; i++) {
    // Ignore return codes .. we need to call ncclGroupEnd to clean up anyway
    ncclCommInitRankDev(comms+i, ndev, uniqueId, i, devlist ? devlist[i] : i);
  }
  NCCLCHECK(ncclGroupEnd());
  return ncclSuccess;
}
```

</details>

<a id="f0900"></a>

## F0900：ncclGraphHelperDestroy

用途：停止并等待图 IPC 清理辅助线程，回收其锁、队列与同步对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>auto res = comm-&gt;graphHelperResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm-&gt;graphHelperThread && res)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>res-&gt;threadState = ThreadStop;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>pthread_cond_signal(&res-&gt;threadCond);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>pthread_join(comm-&gt;graphHelperThread, NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 条件判断 | <code>(res)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>free(res);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>res = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  auto res = comm->graphHelperResources;
  if (comm->graphHelperThread && res) {
    pthread_mutex_lock(&res->threadLock);
    res->threadState = ThreadStop;
    pthread_cond_signal(&res->threadCond);
    pthread_mutex_unlock(&res->threadLock);
    pthread_join(comm->graphHelperThread, NULL);
  }
  if (res) {
    free(res);
    res = NULL;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0901"></a>

## F0901：commDestroy

用途：协调 Proxy 和图辅助资源停止，再进入 communicator 总资源释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(comm-&gt;rank == -1 &#124;&#124; comm-&gt;nRanks &lt;= 0 &#124;&#124; comm-&gt;cudaDev == -1 &#124;&#124; comm-&gt;busId == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("comm %p has already been destroyed", comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int savedDevice;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&savedDevice));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 准备局部状态 | <code>int commDevice = comm-&gt;cudaDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 条件判断 | <code>(savedDevice != commDevice)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaSetDevice(commDevice));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "Destroying comm %p rank %d abortFlag %d fatalError %d", comm, comm-&gt;rank, *comm-&gt;abortFlag, comm-&gt;fatalError);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamSynchronize(comm-&gt;groupStream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>ncclDestroyQueueInfo(comm-&gt;enqueueInfo);</code> | 释放图/普通提交持有的队列，把不能在图析构回调直接关闭的 CUDA IPC 句柄转交辅助线程。 |
| 14 | 0 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 11030</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGraphHelperDestroy(comm));</code> | 失败时可能提前return，成功才继续；停止并等待图 IPC 清理辅助线程，回收其锁、队列与同步对象。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_COLL, "Created %d queue info, destroyed %d", comm-&gt;nQueueInfoCreated, comm-&gt;nQueueInfoDestroyed);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(commFree(comm));</code> | 失败时可能提前return，成功才继续；回收 communicator 分配的 channel、拓扑、事件、队列、映射等对象，按依赖顺序释放。 |
| 18 | 0 / 条件判断 | <code>(savedDevice != commDevice)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaSetDevice(savedDevice));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 21 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Try and prevent a double free of the comm struct (user error)
  if (comm->rank == -1 || comm->nRanks <= 0 || comm->cudaDev == -1 || comm->busId == -1) {
    WARN("comm %p has already been destroyed", comm);
    return ncclInvalidArgument;
  }

  int savedDevice;
  CUDACHECK(cudaGetDevice(&savedDevice));
  int commDevice = comm->cudaDev;

  if (savedDevice != commDevice) {
    CUDACHECK(cudaSetDevice(commDevice));
  }

  TRACE(NCCL_INIT, "Destroying comm %p rank %d abortFlag %d fatalError %d", comm, comm->rank, *comm->abortFlag, comm->fatalError);

  CUDACHECK(cudaStreamSynchronize(comm->groupStream));

  ncclDestroyQueueInfo(comm->enqueueInfo);
#if CUDART_VERSION >= 11030
  NCCLCHECK(ncclGraphHelperDestroy(comm));
#endif
  INFO(NCCL_COLL, "Created %d queue info, destroyed %d", comm->nQueueInfoCreated, comm->nQueueInfoDestroyed);

  NCCLCHECK(commFree(comm));

  if (savedDevice != commDevice)
    CUDACHECK(cudaSetDevice(savedDevice));

  return ncclSuccess;
}
```

</details>

<a id="f0902"></a>

## F0902：ncclCommDestroy

用途：正常销毁公开 communicator 句柄，按本版生命周期要求回收长期通信对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 条件判断 | <code>(comm == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int rank = comm-&gt;rank, nranks = comm-&gt;nRanks, cudaDev = comm-&gt;cudaDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int64_t busId = comm-&gt;busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "comm %p rank %d nRanks %d cudaDev %d busId %lx", comm, rank, nranks, cudaDev, busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(commDestroy(comm));</code> | 失败时可能提前return，成功才继续；协调 Proxy 和图辅助资源停止，再进入 communicator 总资源释放。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Destroy COMPLETE", comm, rank, nranks, cudaDev, busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (comm == NULL)
    return ncclSuccess;

  int rank = comm->rank, nranks = comm->nRanks, cudaDev = comm->cudaDev;
  int64_t busId = comm->busId;
  TRACE(NCCL_INIT, "comm %p rank %d nRanks %d cudaDev %d busId %lx", comm, rank, nranks, cudaDev, busId);

  NCCLCHECK(commDestroy(comm));
  INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Destroy COMPLETE", comm, rank, nranks, cudaDev, busId);
  return ncclSuccess;
}
```

</details>

<a id="f0903"></a>

## F0903：ncclCommAbort

用途：设置共享中止标志并执行异常清理，帮助 GPU/Proxy/socket 等待退出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 条件判断 | <code>(comm == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int rank = comm-&gt;rank, nranks = comm-&gt;nRanks, cudaDev = comm-&gt;cudaDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int64_t busId = comm-&gt;busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "comm %p rank %d nRanks %d cudaDev %d busId %lx", comm, rank, nranks, cudaDev, busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>*comm-&gt;abortFlag = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(commDestroy(comm));</code> | 失败时可能提前return，成功才继续；协调 Proxy 和图辅助资源停止，再进入 communicator 总资源释放。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Abort COMPLETE", comm, rank, nranks, cudaDev, busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (comm == NULL)
    return ncclSuccess;

  int rank = comm->rank, nranks = comm->nRanks, cudaDev = comm->cudaDev;
  int64_t busId = comm->busId;
  TRACE(NCCL_INIT, "comm %p rank %d nRanks %d cudaDev %d busId %lx", comm, rank, nranks, cudaDev, busId);

  // Ask anything that might still be running on the device to quit
  *comm->abortFlag = 1;

  NCCLCHECK(commDestroy(comm));
  INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Abort COMPLETE", comm, rank, nranks, cudaDev, busId);
  return ncclSuccess;
}
```

</details>

<a id="f0904"></a>

## F0904：ncclGetErrorString

用途：把 NCCL 结果枚举映射成稳定可读字符串，不改变 communicator 的执行状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 多路分派 | <code>(code)</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 2 | 1 / case / default | <code>ncclSuccess</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 3 | 2 / 函数返回 | <code>return "no error";</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 1 / case / default | <code>ncclUnhandledCudaError</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 5 | 2 / 函数返回 | <code>return "unhandled cuda error";</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 1 / case / default | <code>ncclSystemError</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 7 | 2 / 函数返回 | <code>return "unhandled system error";</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 1 / case / default | <code>ncclInternalError</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 9 | 2 / 函数返回 | <code>return "internal error";</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 1 / case / default | <code>ncclInvalidArgument</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 11 | 2 / 函数返回 | <code>return "invalid argument";</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 1 / case / default | <code>ncclInvalidUsage</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 13 | 2 / 函数返回 | <code>return "invalid usage";</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 1 / case / default | <code>default</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 15 | 2 / 函数返回 | <code>return "unknown result code";</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  switch (code) {
    case ncclSuccess                : return "no error";
    case ncclUnhandledCudaError     : return "unhandled cuda error";
    case ncclSystemError            : return "unhandled system error";
    case ncclInternalError          : return "internal error";
    case ncclInvalidArgument        : return "invalid argument";
    case ncclInvalidUsage           : return "invalid usage";
    default                         : return "unknown result code";
  }
}
```

</details>

<a id="f0905"></a>

## F0905：ncclCommGetAsyncError

用途：查询 communicator 已记录的异步失败，使应用可在提交成功后检测后台错误。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(comm, "ncclGetAsyncError", "comm"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(asyncError, "ncclGetAsyncError", "asyncError"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*asyncError = comm-&gt;fatalError;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(PtrCheck(comm, "ncclGetAsyncError", "comm"));
  NCCLCHECK(PtrCheck(asyncError, "ncclGetAsyncError", "asyncError"));
  *asyncError = comm->fatalError;
  return ncclSuccess;
}
```

</details>

<a id="f0906"></a>

## F0906：ncclCommCount

用途：读取通信组 rank 总数到用户输出参数，返回的是逻辑参与规模。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(comm, "CommCount", "comm"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(count, "CommCount", "count"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>*count = comm-&gt;nRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  NCCLCHECK(PtrCheck(comm, "CommCount", "comm"));
  NCCLCHECK(PtrCheck(count, "CommCount", "count"));
  *count = comm->nRanks;
  return ncclSuccess;
}
```

</details>

<a id="f0907"></a>

## F0907：ncclCommCuDevice

用途：查询该 communicator 所属本地 CUDA 设备序号，不能把它等同于全局 rank。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(comm, "CommCuDevice", "comm"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(devid, "CommCuDevice", "devid"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>*devid = comm-&gt;cudaDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  NCCLCHECK(PtrCheck(comm, "CommCuDevice", "comm"));
  NCCLCHECK(PtrCheck(devid, "CommCuDevice", "devid"));
  *devid = comm->cudaDev;
  return ncclSuccess;
}
```

</details>

<a id="f0908"></a>

## F0908：ncclCommUserRank

用途：查询本 communicator 当前对象对应的用户 rank 编号。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(comm, "CommUserRank", "comm"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(rank, "CommUserRank", "rank"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>*rank = comm-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  NCCLCHECK(PtrCheck(comm, "CommUserRank", "comm"));
  NCCLCHECK(PtrCheck(rank, "CommUserRank", "rank"));
  *rank = comm->rank;
  return ncclSuccess;
}
```

</details>

