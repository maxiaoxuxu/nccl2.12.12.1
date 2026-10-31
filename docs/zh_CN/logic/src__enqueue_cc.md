# src/enqueue.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0389"></a>

## F0389：ncclKernMaxLocalSize

用途：遍历入口 kernel 属性以求最大局部内存使用，供初始化设备栈/资源配置参考。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclResult_t res = ncclSuccess;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int numNcclKerns = sizeof(ncclKerns)/sizeof(ncclKerns[0]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>cudaFuncAttributes attr = {0};</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>size_t max = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; numNcclKerns；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaFuncGetAttributes(&attr, ncclKerns[i]), res, error);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 7 | 1 / 条件判断 | <code>(attr.localSizeBytes &gt; max)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>max = attr.localSizeBytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 跳转标签 | <code>error</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 12 | 1 / 函数返回 | <code>return (res != ncclSuccess) ? 0 : max;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclResult_t res = ncclSuccess;
  int numNcclKerns = sizeof(ncclKerns)/sizeof(ncclKerns[0]);
  cudaFuncAttributes attr = {0};
  size_t max = 0;
  for (int i = 0; i < numNcclKerns; i++) {
    CUDACHECKGOTO(cudaFuncGetAttributes(&attr, ncclKerns[i]), res, error);
    if (attr.localSizeBytes > max) max = attr.localSizeBytes;
  }

error:
  return (res != ncclSuccess) ? 0 : max;
}
```

</details>

<a id="f0390"></a>

## F0390：ncclKernSetSharedMemoryCarveout

用途：给受支持的 CUDA kernel 入口设置共享内存划分偏好，影响 SM 资源分配而不改算法语义。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclResult_t res = ncclSuccess;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int numNcclKerns = sizeof(ncclKerns)/sizeof(ncclKerns[0]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; numNcclKerns；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaFuncSetAttribute(ncclKerns[i], cudaFuncAttributePreferredSharedMemoryCarveout, carveOut), res, error);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 5 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 6 | 0 / 跳转标签 | <code>error</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 7 | 1 / 函数返回 | <code>return res;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclResult_t res = ncclSuccess;
  int numNcclKerns = sizeof(ncclKerns)/sizeof(ncclKerns[0]);
  for (int i = 0; i < numNcclKerns; i++) {
    CUDACHECKGOTO(cudaFuncSetAttribute(ncclKerns[i], cudaFuncAttributePreferredSharedMemoryCarveout, carveOut), res, error);
  }

error:
  return res;
}
```

</details>

<a id="f0391"></a>

## F0391：ncclLaunchCooperativeKernelMultiDevice

用途：优先用受支持的多设备协作发射，否则逐 GPU 切换设备后调用 cudaLaunchKernel。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 9000</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 条件判断 | <code>(cgMode & 0x01)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 3 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaLaunchCooperativeKernelMultiDevice(paramsList, numDevices, cudaCooperativeLaunchMultiDeviceNoPreSync&#124;cudaCooperativeLaunchMultiDeviceNoPostSync));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 准备局部状态 | <code>int savedDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&savedDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; numDevices；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 准备局部状态 | <code>struct cudaLaunchParams* params = paramsList+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaSetDevice(cudaDevs[i]));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaLaunchKernel(params-&gt;func, params-&gt;gridDim, params-&gt;blockDim, params-&gt;args, params-&gt;sharedMem, params-&gt;stream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaSetDevice(savedDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if CUDART_VERSION >= 9000
  if (cgMode & 0x01) {
    CUDACHECK(cudaLaunchCooperativeKernelMultiDevice(paramsList, numDevices,
            // These flags are to reduce the latency of using this API
            cudaCooperativeLaunchMultiDeviceNoPreSync|cudaCooperativeLaunchMultiDeviceNoPostSync));
    return ncclSuccess;
  }
#endif
  int savedDev;
  CUDACHECK(cudaGetDevice(&savedDev));
  for (int i = 0; i < numDevices; i++) {
    struct cudaLaunchParams* params = paramsList+i;
    CUDACHECK(cudaSetDevice(cudaDevs[i]));
    CUDACHECK(cudaLaunchKernel(params->func, params->gridDim, params->blockDim, params->args, params->sharedMem, params->stream));
  }
  CUDACHECK(cudaSetDevice(savedDev));
  return ncclSuccess;
}
```

</details>

<a id="f0392"></a>

## F0392：getNextOp

用途：等待 channel 工作 FIFO 某槽被 GPU 取走并标成 Unused，清零后分配给本批下一条 work。

- 先拒绝本批超过NCCL_MAX_OPS的聚合，再按单调workFifoTail取模定位描述符槽。
- 等待header.type变Unused表示GPU已把描述复制走，可以覆盖描述符；不表示用户数据传输已经完成。
- 清零后复制可选模板，递增写尾和本批计数，返回可装填work地址。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(channel-&gt;workCount == NCCL_MAX_OPS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("Too many aggregated operations on channel %d (%d max)", channel-&gt;id, NCCL_MAX_OPS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int opIndex = channel-&gt;workFifoTail%NCCL_MAX_OPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclWork* w = channel-&gt;workFifo+opIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>volatile uint8_t* typePtr = (volatile uint8_t*)&w-&gt;header.type;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 循环 | <code>初始 ；继续条件 (typePtr[0] != ncclWorkTypeUnused)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>sched_yield();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>memset(w, 0, sizeof(struct ncclWork));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 条件判断 | <code>(base)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>memcpy(w-&gt;elems, base, sizeof(struct ncclWorkElem));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>channel-&gt;workFifoTail++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>channel-&gt;workCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 条件判断 | <code>(work)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>*work = w;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (channel->workCount == NCCL_MAX_OPS) {
    WARN("Too many aggregated operations on channel %d (%d max)", channel->id, NCCL_MAX_OPS);
    return ncclInvalidUsage;
  }
  int opIndex = channel->workFifoTail%NCCL_MAX_OPS;
  struct ncclWork* w = channel->workFifo+opIndex;
  volatile uint8_t* typePtr = (volatile uint8_t*)&w->header.type;
  while (typePtr[0] != ncclWorkTypeUnused) sched_yield();
  memset(w, 0, sizeof(struct ncclWork));
  // Initialize with work elem if provided
  if (base) memcpy(w->elems, base, sizeof(struct ncclWorkElem));
  channel->workFifoTail++;
  channel->workCount++;
  if (work) *work = w;
  return ncclSuccess;
}
```

</details>

<a id="f0393"></a>

## F0393：setupLaunch

用途：确定本批 block/channel 范围，给空通道补 no-op，标记各通道 isLast，并准备可选 GDRCopy FIFO。

- 普通模式扫描实际有工作的最高channel来决定grid；图模式不能在运行时改捕获的发射范围。
- 每个将被启动的空channel补一个零warp任务，否则对应block没有合法终止记录。
- 标记各channel最后work的isLast，并处理channel0首条内联和GDRCopy FIFO镜像。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclComm_t comm = eqInfo-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct cudaLaunchParams* params = comm-&gt;myParams;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(!usingCudaGraph)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 准备局部状态 | <code>int nChannels = std::max(comm-&gt;nChannels, comm-&gt;p2pnChannels);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 循环 | <code>初始 int c=0;；继续条件 c&lt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 3 / 条件判断 | <code>(comm-&gt;channels[c].workCount)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 5 / 调用 / 计算 / 写入 | <code>params-&gt;gridDim.x = c+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>eqInfo-&gt;maxChannels = params-&gt;gridDim.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;eqInfo-&gt;maxChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 13 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 1 / 条件判断 | <code>(channel-&gt;workCount == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 3 / 准备局部状态 | <code>struct ncclWork* w;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getNextOp(channel, &w, NULL));</code> | 失败时可能提前return，成功才继续；等待 channel 工作 FIFO 某槽被 GPU 取走并标成 Unused，清零后分配给本批下一条 work。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>w-&gt;header.funcIndex = FUNC_INDEX_P2P;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 3 / 调用 / 计算 / 写入 | <code>w-&gt;header.type = ncclWorkTypeP2p;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>w-&gt;header.nWarps = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;workFifo[(channel-&gt;workFifoTail-1)%NCCL_MAX_OPS].header.isLast = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 1 / 条件判断 | <code>(c == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 3 / 准备局部状态 | <code>struct ncclWork* work = channel-&gt;workFifo+((channel-&gt;workFifoTail-channel-&gt;workCount)%NCCL_MAX_OPS);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 3 / 条件判断 | <code>(work-&gt;header.type == ncclWorkTypeColl && eqInfo-&gt;elemList-&gt;count() == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 5 / 调用 / 计算 / 写入 | <code>work-&gt;header.type = ncclWorkTypeUnused;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 1 / 条件判断 | <code>(channel-&gt;gdrMemDesc)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 3 / 准备局部状态 | <code>uint64_t first = (channel-&gt;workFifoTail-channel-&gt;workCount)%NCCL_MAX_OPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 3 / 准备局部状态 | <code>uint64_t nelems = channel-&gt;workCount;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 3 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "GDRCOPY : copy workFifo %p to %p first %ld nelems %zi", channel-&gt;workFifo, channel-&gt;workFifoGdr, first, nelems);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 3 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; nelems；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 34 | 4 / 准备局部状态 | <code>int elem = (first+i) % NCCL_MAX_OPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaCopy(channel-&gt;gdrMemDesc, channel-&gt;workFifoGdr+elem, channel-&gt;workFifo+elem, 1));</code> | 失败时可能提前return，成功才继续；通过 GDRCopy 映射把指定对象写入显存，用于低延迟提交小量控制/工作数据。 |
| 36 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 37 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclComm_t comm = eqInfo->comm;
  // Do not use comm->myParams in this function unless in non-graph mode
  // In graph mode, enqueue is async to capture, myParams can have been changed
  struct cudaLaunchParams* params = comm->myParams;

  // Only launch blocks where we have work to do.
  // This is not supported when we are in cudaGraph mode.
  // Because in cudaGraph mode the launch param needs to be determined
  // at capture time instead of launch time.
  if (!usingCudaGraph) {
    int nChannels = std::max(comm->nChannels, comm->p2pnChannels);
    for (int c=0; c<nChannels; c++) {
      if (comm->channels[c].workCount) params->gridDim.x = c+1;
    }
    eqInfo->maxChannels = params->gridDim.x;
  }

  // Set isLast = 1 for the last operation and add a no-op on empty channels (p2p case).
  for (int c=0; c<eqInfo->maxChannels; c++) {
    struct ncclChannel* channel = comm->channels+c;
    if (channel->workCount == 0) {
      struct ncclWork* w;
      NCCLCHECK(getNextOp(channel, &w, NULL));
      w->header.funcIndex = FUNC_INDEX_P2P;
      w->header.type = ncclWorkTypeP2p;
      w->header.nWarps = 0;
    }
    channel->workFifo[(channel->workFifoTail-1)%NCCL_MAX_OPS].header.isLast = 1;

    if (c == 0) {
      // As we inline the first coll directly, we can free it immediately.
      // Except P2P or aggregation or registration cases
      struct ncclWork* work = channel->workFifo+((channel->workFifoTail-channel->workCount)%NCCL_MAX_OPS);
      if (work->header.type == ncclWorkTypeColl && eqInfo->elemList->count() == 1)
        work->header.type = ncclWorkTypeUnused;
    }

    if (channel->gdrMemDesc) {
      // GDRCOPY support
      uint64_t first = (channel->workFifoTail-channel->workCount)%NCCL_MAX_OPS;
      uint64_t nelems = channel->workCount;
      TRACE(NCCL_INIT, "GDRCOPY : copy workFifo %p to %p first %ld nelems %zi",
            channel->workFifo, channel->workFifoGdr, first, nelems);

      for (int i = 0; i < nelems; i++) {
        int elem = (first+i) % NCCL_MAX_OPS;
        // Copy Host workFifo to CUDA workFifo via the GDRCOPY mapping
        NCCLCHECK(ncclGdrCudaCopy(channel->gdrMemDesc, channel->workFifoGdr+elem, channel->workFifo+elem, 1));
      }
    }
  }

  return ncclSuccess;
}
```

</details>

<a id="f0394"></a>

## F0394：ncclCpuBarrierIn

用途：进入同进程多 GPU 发射的相位屏障，用原子计数选出最后到达者统一发射。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>volatile int* ptr = (volatile int*)(comm-&gt;intraBarrier+comm-&gt;intraPhase);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int val = *ptr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>bool done = false;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 0 / 循环 | <code>初始 ；继续条件 (done == false)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 条件判断 | <code>(val &gt;= comm-&gt;intraRanks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>WARN("Trying to launch too many work elements, max is %d", NCCL_MAX_OPS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 3 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 1 / 条件判断 | <code>(val+1 == comm-&gt;intraRanks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>comm-&gt;intraBarrier[comm-&gt;intraPhase^1] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>*isLast = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>done = __sync_bool_compare_and_swap(ptr, val, val+1);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>val++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>*isLast = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  volatile int* ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase);
  int val = *ptr;
  bool done = false;
  while (done == false) {
    if (val >= comm->intraRanks) {
      WARN("Trying to launch too many work elements, max is %d", NCCL_MAX_OPS);
      return ncclInvalidUsage;
    }
    if (val+1 == comm->intraRanks) {
      // Reset the barrier.
      comm->intraBarrier[comm->intraPhase^1] = 0;
      *isLast = 1;
      return ncclSuccess;
    }
    done = __sync_bool_compare_and_swap(ptr, val, val+1);
    val++;
  }
  *isLast = 0;
  return ncclSuccess;
}
```

</details>

<a id="f0395"></a>

## F0395：ncclCpuBarrierLast

用途：最后到达者发布本相位已完成信号，使其他发射线程可离开屏障。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>volatile int* ptr = (volatile int*)(comm-&gt;intraBarrier+comm-&gt;intraPhase);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int val = *ptr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(__sync_bool_compare_and_swap(ptr, val, val+1) != true)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>WARN("Trying to launch too many work elements, max is %d", NCCL_MAX_OPS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  volatile int* ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase);
  int val = *ptr;
  if (__sync_bool_compare_and_swap(ptr, val, val+1) != true) {
    WARN("Trying to launch too many work elements, max is %d", NCCL_MAX_OPS);
    return ncclInternalError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0396"></a>

## F0396：ncclCpuBarrierOut

用途：等待本地发射相位完成并翻转相位，保证下一轮发射不会误用上轮计数。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>volatile int* ptr = (volatile int*)(comm-&gt;intraBarrier+comm-&gt;intraPhase);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 ；继续条件 (*ptr &lt; comm-&gt;intraRanks)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>pthread_yield();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;intraPhase ^= 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  volatile int* ptr = (volatile int*)(comm->intraBarrier+comm->intraPhase);
  while (*ptr < comm->intraRanks) pthread_yield();
  comm->intraPhase ^= 1;
  return ncclSuccess;
}
```

</details>

<a id="f0397"></a>

## F0397：ncclLaunchBarrier

用途：建立用户 stream 与内部 stream 依赖，并在 GROUP 发射模式协调同进程 GPU 的启动。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct cudaLaunchParams* params = comm-&gt;myParams;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(params-&gt;gridDim.x == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(comm-&gt;launchMode == ncclComm::GROUP && (comm-&gt;groupCudaStream &#124;&#124; comm-&gt;userStream == cudaStreamDefault &#124;&#124; comm-&gt;userStream == cudaStreamLegacy &#124;&#124; comm-&gt;userStream == cudaStreamPerThread))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaEventRecord(comm-&gt;intDoneEvent, comm-&gt;userStream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamWaitEvent(comm-&gt;groupStream, comm-&gt;intDoneEvent, 0));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>params-&gt;stream = comm-&gt;groupStream;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 2 / 条件判断 | <code>(comm-&gt;userStream != params-&gt;stream && !comm-&gt;usingCudaGraph)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamWaitEvent(comm-&gt;userStream, comm-&gt;doneEvent, 0));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>params-&gt;stream = comm-&gt;userStream;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 条件判断 | <code>(comm-&gt;launchMode == ncclComm::GROUP)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 准备局部状态 | <code>int isLast = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCpuBarrierIn(comm, &isLast));</code> | 失败时可能提前return，成功才继续；进入同进程多 GPU 发射的相位屏障，用原子计数选出最后到达者统一发射。 |
| 19 | 2 / 条件判断 | <code>(isLast)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclLaunchCooperativeKernelMultiDevice(comm-&gt;intraParams, comm-&gt;intraCudaDevs, comm-&gt;intraRanks, *comm-&gt;intraCGMode));</code> | 失败时可能提前return，成功才继续；优先用受支持的多设备协作发射，否则逐 GPU 切换设备后调用 cudaLaunchKernel。 |
| 22 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCpuBarrierLast(comm));</code> | 失败时可能提前return，成功才继续；最后到达者发布本相位已完成信号，使其他发射线程可离开屏障。 |
| 23 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct cudaLaunchParams* params = comm->myParams;
  if (params->gridDim.x == 0) return ncclSuccess;

  // Use internal NCCL stream for CGMD/GROUP launch if required or if the user stream is NULL
  if (comm->launchMode == ncclComm::GROUP &&
      (comm->groupCudaStream ||
       comm->userStream == cudaStreamDefault ||
       comm->userStream == cudaStreamLegacy ||
       comm->userStream == cudaStreamPerThread)) {
    // Enqueue event in user stream
    CUDACHECK(cudaEventRecord(comm->intDoneEvent, comm->userStream));
    // Create dependency between user stream and internal NCCL stream
    CUDACHECK(cudaStreamWaitEvent(comm->groupStream, comm->intDoneEvent, 0));
    params->stream = comm->groupStream;
  } else {
    if (comm->userStream != params->stream && !comm->usingCudaGraph) {
      // Stream changed from last call, create dependency against last NCCL kernel launch
      CUDACHECK(cudaStreamWaitEvent(comm->userStream, comm->doneEvent, 0));
    }
    params->stream = comm->userStream;
  }

  if (comm->launchMode == ncclComm::GROUP) {
    int isLast = 0;
    NCCLCHECK(ncclCpuBarrierIn(comm, &isLast));
    if (isLast) {
      // I'm the last. Launch all operations.
      NCCLCHECK(ncclLaunchCooperativeKernelMultiDevice(comm->intraParams, comm->intraCudaDevs, comm->intraRanks, *comm->intraCGMode));
      NCCLCHECK(ncclCpuBarrierLast(comm));
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0398"></a>

## F0398：ncclLaunchKernel

用途：PARALLEL 模式真正调用 CUDA 发射，GROUP 模式等待已由最后线程执行的统一发射完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct cudaLaunchParams *params = comm-&gt;myParams;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(params-&gt;gridDim.x == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(comm-&gt;rank == 0 && *comm-&gt;intraCGMode & 0x10)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>*comm-&gt;intraCGMode ^= 0x10;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT,"Launch mode %s%s%s", comm-&gt;launchMode == ncclComm::GROUP ? "Group" : "Parallel", *comm-&gt;intraCGMode ? "/CGMD" : "", (comm-&gt;launchMode == ncclComm::GROUP && comm-&gt;groupCudaStream) ? "/Stream" : "");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 条件判断 | <code>(comm-&gt;launchMode == ncclComm::GROUP)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCpuBarrierOut(comm));</code> | 失败时可能提前return，成功才继续；等待本地发射相位完成并翻转相位，保证下一轮发射不会误用上轮计数。 |
| 12 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaLaunchKernel(params-&gt;func, params-&gt;gridDim, params-&gt;blockDim, params-&gt;args, params-&gt;sharedMem, params-&gt;stream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct cudaLaunchParams *params = comm->myParams;
  if (params->gridDim.x == 0) return ncclSuccess;

  // We can't print the CG mode before the first barrier happened.
  if (comm->rank == 0 && *comm->intraCGMode & 0x10) {
    *comm->intraCGMode ^= 0x10;
    INFO(NCCL_INIT,"Launch mode %s%s%s",
        comm->launchMode == ncclComm::GROUP ? "Group" : "Parallel",
        *comm->intraCGMode ? "/CGMD" : "",
        (comm->launchMode == ncclComm::GROUP && comm->groupCudaStream) ? "/Stream" : "");
  }

  if (comm->launchMode == ncclComm::GROUP) {
    NCCLCHECK(ncclCpuBarrierOut(comm));
  } else {
    CUDACHECK(cudaLaunchKernel(params->func, params->gridDim, params->blockDim, params->args, params->sharedMem, params->stream));
  }

  return ncclSuccess;
}
```

</details>

<a id="f0399"></a>

## F0399：ncclLaunchProxy

用途：发布 HostSetup 已保存的 Proxy 任务并重置本批通道统计；普通调用点在实际 kernel 发射之前。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclComm_t comm = eqInfo-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(eqInfo-&gt;maxChannels == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;eqInfo-&gt;maxChannels；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+r;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;workCount = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;totalSize = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;lastChannel = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyStart(comm));</code> | 失败时可能提前return，成功才继续；发布本批累积代理工作到相关本地代理的池，递增操作编号而不新建每消息线程。 |
| 12 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Start the network proxies as soon as the kernel has been launched. We can't
  // perform any CUDA call between the two or having a cudaFree between the CUDA
  // launch and the ncclProxyStart call could cause a deadlock.
  // Also, starting the proxies after the CUDA launch seems to be better for
  // performance (latency).
  ncclComm_t comm = eqInfo->comm;
  if (eqInfo->maxChannels == 0) return ncclSuccess;

  for (int r=0; r<eqInfo->maxChannels; r++) {
    struct ncclChannel* channel = comm->channels+r;
    channel->workCount = 0;
    channel->totalSize = 0;
  }
  comm->lastChannel = 0;
  NCCLCHECK(ncclProxyStart(comm));
  return ncclSuccess;
}
```

</details>

<a id="f0400"></a>

## F0400：ncclRecordEvents

用途：在执行 stream 记录完成事件并建立必要的用户 stream 依赖；record 本身不代表事件已经完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct cudaLaunchParams *params = comm-&gt;myParams;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(!comm-&gt;usingCudaGraph)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaEventRecord(comm-&gt;doneEvent, params-&gt;stream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 条件判断 | <code>(comm-&gt;launchMode == ncclComm::GROUP && (comm-&gt;groupCudaStream &#124;&#124; comm-&gt;userStream == cudaStreamDefault &#124;&#124; comm-&gt;userStream == cudaStreamLegacy &#124;&#124; comm-&gt;userStream == cudaStreamPerThread))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaEventRecord(comm-&gt;intDoneEvent, params-&gt;stream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamWaitEvent(comm-&gt;userStream, comm-&gt;intDoneEvent, 0));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct cudaLaunchParams *params = comm->myParams;

  // Enqueue event after NCCL kernel (only in non-graph mode)
  if (!comm->usingCudaGraph) CUDACHECK(cudaEventRecord(comm->doneEvent, params->stream));
  // Use internal NCCL stream for CGMD/GROUP launch if required or if the user stream is NULL
  if (comm->launchMode == ncclComm::GROUP &&
      (comm->groupCudaStream ||
       comm->userStream == cudaStreamDefault ||
       comm->userStream == cudaStreamLegacy ||
       comm->userStream == cudaStreamPerThread)) {
    CUDACHECK(cudaEventRecord(comm->intDoneEvent, params->stream));
    // Create dependency between NCCL internal stream and user stream
    CUDACHECK(cudaStreamWaitEvent(comm->userStream, comm->intDoneEvent, 0));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0401"></a>

## F0401：ncclLaunchReset

用途：清理本批发射参数和入队计数，为后续提交复用；图路径按持久 QueueInfo 生命周期处理。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;userStreamSet = false;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(comm-&gt;usingCudaGraph)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCreateQueueInfo(&comm-&gt;enqueueInfo, comm));</code> | 失败时可能提前return，成功才继续；分配一个提交/图持有的 QueueInfo 和可复用元素列表，记录 communicator 队列生命周期计数。 |
| 5 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclResetQueueInfo(comm-&gt;enqueueInfo));</code> | 失败时可能提前return，成功才继续；清理一批已经提交的队列统计并 recycle 元素节点，保留容量供后续批次复用。 |
| 7 | 0 / 准备局部状态 | <code>struct cudaLaunchParams *params = comm-&gt;myParams;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;gridDim.x = params-&gt;blockDim.x = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;func = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 条件判断 | <code>(comm-&gt;launchMode == ncclComm::GROUP_GRAPH)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;launchMode = ncclComm::GROUP;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;usingCudaGraph = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  comm->userStreamSet = false;

  // We are finishing capture of the current launch
  // But we need to keep the current enqueue info for CUDA graph
  // Thus we need to creating a new enqueue info for the next run
  if (comm->usingCudaGraph) {
    NCCLCHECK(ncclCreateQueueInfo(&comm->enqueueInfo, comm));
  } else {
    // If not in CUDA graph mode, we reuse the same info space
    NCCLCHECK(ncclResetQueueInfo(comm->enqueueInfo));
  }

  // After capturing an op in graph mode or launching the op in non-graph mode
  // we can reset myParams for use in next op
  struct cudaLaunchParams *params = comm->myParams;
  params->gridDim.x = params->blockDim.x = 0;
  params->func = NULL;

  // Reset launch mode to GROUP if changed
  if (comm->launchMode == ncclComm::GROUP_GRAPH) comm->launchMode = ncclComm::GROUP;
  comm->usingCudaGraph = 0;

  return ncclSuccess;
}
```

</details>

<a id="f0402"></a>

## F0402：getCollNetSupport

用途：检查所选数据类型/归约是否被 CollNet 插件支持，以屏蔽不能正确执行的候选。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(info-&gt;comm-&gt;collNetSupport &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>ncclRedOp_t netOp = info-&gt;op == ncclAvg &#124;&#124; info-&gt;op &gt;= ncclNumOps ? ncclSum : info-&gt;op;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetReduceSupport(info-&gt;datatype, netOp, collNetTypeSupport));</code> | 失败时可能提前return，成功才继续；查询某数据类型和归约操作能否在网络内完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 5 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>*collNetTypeSupport = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (info->comm->collNetSupport > 0) {
    // Translate ncclAvg and PreMulSum
    ncclRedOp_t netOp = info->op == ncclAvg || info->op >= ncclNumOps ? ncclSum : info->op;
    NCCLCHECK(collNetReduceSupport(info->datatype, netOp, collNetTypeSupport));
  } else {
    *collNetTypeSupport = 0;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0403"></a>

## F0403：getAlgoInfo

用途：遍历可用算法协议的时间模型，再按消息尺寸缩减本次 channel 和线程数，平衡延迟与带宽。

- 单rank直接选Ring/Simple；多rank枚举算法和协议，跳过不支持CollNet，选择最小非负预测耗时。
- 先按nBytes与每线程目标工作量缩减channel；只有缩到较少通道后才继续缩线程，避免小消息启动大量空闲worker。
- Simple再补同步warp；因此block线程数不是纯复制线程数。此函数只做决策，不搬数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclComm* comm = info-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm-&gt;nRanks == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;algorithm = NCCL_ALGO_RING;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;protocol = NCCL_PROTO_SIMPLE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 7 | 2 / 准备局部状态 | <code>float minTime = 3600000000.0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;algorithm = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;protocol = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 准备局部状态 | <code>int nAlgos = NCCL_NUM_ALGORITHMS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 2 / 循环 | <code>初始 int a=0;；继续条件 a&lt;nAlgos；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 3 / 条件判断 | <code>(a == NCCL_ALGO_COLLNET && collNetTypeSupport != 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 15 | 3 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 4 / 准备局部状态 | <code>float time;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetAlgoTime(info, a, p, numPipeOps, &time));</code> | 失败时可能提前return，成功才继续；用延迟项加字节数/带宽项预测当前候选耗时，加入 Tree/Ring 与聚合修正供选型。 |
| 18 | 4 / 条件判断 | <code>(time &gt;= 0 && time &lt; minTime)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 6 / 调用 / 计算 / 写入 | <code>info-&gt;algorithm = a;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 6 / 调用 / 计算 / 写入 | <code>info-&gt;protocol = p;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 6 / 调用 / 计算 / 写入 | <code>minTime = time;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 24 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 25 | 2 / 条件判断 | <code>(info-&gt;algorithm == -1 &#124;&#124; info-&gt;protocol == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 4 / 调用 / 计算 / 写入 | <code>WARN("Error : no algorithm/protocol available");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_COLL, "%ld Bytes -&gt; Algo %d proto %d time %f", info-&gt;nBytes, info-&gt;algorithm, info-&gt;protocol, minTime);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 0 / 准备局部状态 | <code>int nc = (info-&gt;nChannels &gt; 0) ? info-&gt;nChannels : comm-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 0 / 准备局部状态 | <code>int nt = comm-&gt;maxThreads[info-&gt;algorithm][info-&gt;protocol];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 0 / 准备局部状态 | <code>int threadThreshold = comm-&gt;threadThresholds[info-&gt;algorithm][info-&gt;protocol];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 0 / 条件判断 | <code>(info-&gt;algorithm == NCCL_ALGO_COLLNET)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 2 / 准备局部状态 | <code>int ncSwitch = 16;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 36 | 2 / 准备局部状态 | <code>bool flag = true;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 2 / 循环 | <code>初始 ；继续条件 (ncSwitch &gt;= 1 && flag)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 38 | 3 / 循环 | <code>初始 ；继续条件 ((flag = info-&gt;nBytes &lt; nc*nt*info-&gt;comm-&gt;channels[0].collTree.nHeads*threadThreshold) && nc &gt; ncSwitch)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 4 / 条件判断 | <code>(nc == ncSwitch+ncSwitch/2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 6 / 调用 / 计算 / 写入 | <code>threadThreshold /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 4 / 调用 / 计算 / 写入 | <code>nc--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 44 | 3 / 调用 / 计算 / 写入 | <code>ncSwitch /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 46 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 47 | 2 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes &lt; nc*nt*threadThreshold)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 48 | 3 / 条件判断 | <code>(nc &gt;= 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 49 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 50 | 5 / 调用 / 计算 / 写入 | <code>nc--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 52 | 5 / 条件判断 | <code>((nt % 128) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 7 / 调用 / 计算 / 写入 | <code>nt/=2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 56 | 7 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 57 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 58 | 0 / 条件判断 | <code>(info-&gt;protocol == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 2 / 调用 / 计算 / 写入 | <code>nt += WARP_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 2 / 条件判断 | <code>(info-&gt;algorithm == NCCL_ALGO_TREE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 62 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 63 | 4 / 调用 / 计算 / 写入 | <code>nt += 3*WARP_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 64 | 2 / 条件判断 | <code>(info-&gt;algorithm == NCCL_ALGO_COLLNET)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 65 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 66 | 4 / 调用 / 计算 / 写入 | <code>nt += 3*WARP_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;nChannels = nc;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 68 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;nThreads = nt;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 69 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclComm* comm = info->comm;
  if (comm->nRanks == 1) {
    info->algorithm = NCCL_ALGO_RING;
    info->protocol = NCCL_PROTO_SIMPLE;
  }
  else {
    float minTime = 3600000000.0; // Hopefully no operation will take an hour to complete.
    // Find algorithm / protocol.
    info->algorithm = -1;
    info->protocol = -1;
    int nAlgos = NCCL_NUM_ALGORITHMS;
    for (int a=0; a<nAlgos; a++) {
      if (a == NCCL_ALGO_COLLNET && collNetTypeSupport != 1) continue;
      for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
        float time;
        NCCLCHECK(ncclTopoGetAlgoTime(info, a, p, numPipeOps, &time));
        if (time >= 0 && time < minTime) {
          info->algorithm = a;
          info->protocol = p;
          minTime = time;
        }
      }
    }
    if (info->algorithm == -1 || info->protocol == -1) {
      WARN("Error : no algorithm/protocol available");
      return ncclInternalError;
    }
    //if (comm->rank == 0) INFO(NCCL_TUNING, "%ld Bytes -> Algo %d proto %d time %f", info->nBytes, info->algorithm, info->protocol, minTime);
    TRACE(NCCL_COLL, "%ld Bytes -> Algo %d proto %d time %f", info->nBytes, info->algorithm, info->protocol, minTime);
  }

  int nc = (info->nChannels > 0) ? info->nChannels : comm->nChannels;
  int nt = comm->maxThreads[info->algorithm][info->protocol];
  int threadThreshold = comm->threadThresholds[info->algorithm][info->protocol];
  if (info->algorithm == NCCL_ALGO_COLLNET) {
    // CollNet channel tuning
    int ncSwitch = 16;
    bool flag = true;
    while (ncSwitch >= 1 && flag) {
      while ((flag = info->nBytes < nc*nt*info->comm->channels[0].collTree.nHeads*threadThreshold) && nc > ncSwitch) {
        if (nc == ncSwitch+ncSwitch/2) threadThreshold /= 2;
        nc--;
      }
      ncSwitch /= 2;
    }
  } else {
    // Ring/Tree channel tuning
    while (info->nBytes < nc*nt*threadThreshold) {
      if (nc >= 2) nc--;
      else if ((nt % 128) == 0) nt/=2;
      else break;
    }
  }
  if (info->protocol == NCCL_PROTO_SIMPLE) {
    nt += WARP_SIZE; // Extra warp for sync
    // More threads or sync warps needed due to split thread model
    if (info->algorithm == NCCL_ALGO_TREE) nt += 3*WARP_SIZE;
    if (info->algorithm == NCCL_ALGO_COLLNET) nt += 3*WARP_SIZE;
  }
  info->nChannels = nc;
  info->nThreads = nt;
  return ncclSuccess;
}
```

</details>

<a id="f0404"></a>

## F0404：getPatternInfo

用途：由 API 和算法映射实际通信方向模式，使 GPU 与 Proxy 选择一致的拓扑边。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 多路分派 | <code>(info-&gt;coll)</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 2 | 1 / case / default | <code>ncclFuncBroadcast</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;pattern = info-&gt;algorithm == NCCL_ALGO_TREE ? ncclPatternTreeDown : ncclPatternPipelineFrom;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 5 | 1 / case / default | <code>ncclFuncReduce</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;pattern = info-&gt;algorithm == NCCL_ALGO_TREE ? ncclPatternTreeUp : ncclPatternPipelineTo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 8 | 1 / case / default | <code>ncclFuncReduceScatter</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 9 | 1 / case / default | <code>ncclFuncAllGather</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;pattern = ncclPatternRing;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 12 | 1 / case / default | <code>ncclFuncAllReduce</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;pattern = info-&gt;algorithm == NCCL_ALGO_COLLNET ? ncclPatternCollTreeUpDown : info-&gt;algorithm == NCCL_ALGO_TREE ? ncclPatternTreeUpDown : ncclPatternRingTwice;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 15 | 1 / case / default | <code>default</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>WARN("Unknown pattern for collective %d algorithm %d", info-&gt;coll, info-&gt;algorithm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  switch (info->coll) {
    case ncclFuncBroadcast:
      info->pattern = info->algorithm == NCCL_ALGO_TREE ? ncclPatternTreeDown : ncclPatternPipelineFrom; break;
    case ncclFuncReduce:
      info->pattern = info->algorithm == NCCL_ALGO_TREE ? ncclPatternTreeUp : ncclPatternPipelineTo; break;
    case ncclFuncReduceScatter:
    case ncclFuncAllGather:
      info->pattern = ncclPatternRing; break;
    case ncclFuncAllReduce:
      info->pattern = info->algorithm == NCCL_ALGO_COLLNET ? ncclPatternCollTreeUpDown : info->algorithm == NCCL_ALGO_TREE ? ncclPatternTreeUpDown : ncclPatternRingTwice; break;
    default:
      WARN("Unknown pattern for collective %d algorithm %d", info->coll, info->algorithm);
      return ncclInternalError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0405"></a>

## F0405：getLoopInfo

用途：给通信模式设置每轮块数和算法步数，再由 computeColl 转成实际协议进度。

- Ring一轮有P块、P-1步；RingTwice为2(P-1)步；树/流水线按对应方向模式设置。
- 这些是算法记账量，还要乘chunkSteps才是FIFO进度，不是硬件网络包数量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 多路分派 | <code>(info-&gt;pattern)</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 2 | 1 / case / default | <code>ncclPatternTreeUp</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 3 | 1 / case / default | <code>ncclPatternTreeDown</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 4 | 1 / case / default | <code>ncclPatternTreeUpDown</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 5 | 1 / case / default | <code>ncclPatternPipelineFrom</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 6 | 1 / case / default | <code>ncclPatternPipelineTo</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;nstepsPerLoop = info-&gt; nchunksPerLoop = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 9 | 1 / case / default | <code>ncclPatternCollTreeUpDown</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;nstepsPerLoop = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;nchunksPerLoop = info-&gt;comm-&gt;channels[0].collTree.nHeads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 13 | 1 / case / default | <code>ncclPatternRing</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;nstepsPerLoop = info-&gt;comm-&gt;nRanks-1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;nchunksPerLoop = info-&gt;comm-&gt;nRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 17 | 1 / case / default | <code>ncclPatternRingTwice</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;nstepsPerLoop = 2*(info-&gt;comm-&gt;nRanks-1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;nchunksPerLoop = info-&gt;comm-&gt;nRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 21 | 1 / case / default | <code>default</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>WARN("Unknown pattern %d", info-&gt;pattern);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 24 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  switch (info->pattern) {
    case ncclPatternTreeUp:
    case ncclPatternTreeDown:
    case ncclPatternTreeUpDown:
    case ncclPatternPipelineFrom:
    case ncclPatternPipelineTo:
      info->nstepsPerLoop = info-> nchunksPerLoop = 1; break;
    case ncclPatternCollTreeUpDown:
      info->nstepsPerLoop = 1; info->nchunksPerLoop = info->comm->channels[0].collTree.nHeads; break;
    case ncclPatternRing:
      info->nstepsPerLoop = info->comm->nRanks-1; info->nchunksPerLoop = info->comm->nRanks; break;
    case ncclPatternRingTwice:
      info->nstepsPerLoop = 2*(info->comm->nRanks-1); info->nchunksPerLoop = info->comm->nRanks; break;
    default:
      WARN("Unknown pattern %d", info->pattern);
      return ncclInternalError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0406"></a>

## F0406：computeColl

用途：把一项 API 任务编译成 GPU work 与 ProxyOp，计算协议 chunk/slice、尾块和总推进步数。

- 先检查聚合是否已给定通道和线程；已给定时跳到comp_next，避免覆盖整批统一选型。
- 先填work的用户范围与设备函数索引，再计算物理step/chunk；Tree和CollNet按深度缩小块，LL扣除行flag开销。
- 最后用有效chunk容量估nLoops，把算法轮数转换为Proxy步数；同一操作的GPU/CPU计划必须一致。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int collNetTypeSupport = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(info-&gt;nChannels &gt; 0 && info-&gt;nThreads &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 控制转移 | <code>goto comp_next;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getCollNetSupport(info, &collNetTypeSupport));</code> | 失败时可能提前return，成功才继续；检查所选数据类型/归约是否被 CollNet 插件支持，以屏蔽不能正确执行的候选。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getAlgoInfo(info, collNetTypeSupport, 1));</code> | 失败时可能提前return，成功才继续；遍历可用算法协议的时间模型，再按消息尺寸缩减本次 channel 和线程数，平衡延迟与带宽。 |
| 7 | 0 / 跳转标签 | <code>comp_next</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getPatternInfo(info));</code> | 失败时可能提前return，成功才继续；由 API 和算法映射实际通信方向模式，使 GPU 与 Proxy 选择一致的拓扑边。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getLoopInfo(info));</code> | 失败时可能提前return，成功才继续；给通信模式设置每轮块数和算法步数，再由 computeColl 转成实际协议进度。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;header.type = ncclWorkTypeColl;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;sendbuff = info-&gt;sendbuff;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;recvbuff = info-&gt;recvbuff;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;root = info-&gt;root;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;count = info-&gt;count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;nChannels = info-&gt;nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;header.nWarps = info-&gt;nThreads / WARP_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;redOpArg = info-&gt;opFull.scalarArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;redOpArgIsPtr = info-&gt;opFull.scalarArgIsPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(info-&gt;comm-&gt;nRanks == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>work-&gt;header.funcIndex = 1 + int(info-&gt;datatype);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;header.funcIndex = FUNC_INDEX(info-&gt;coll, info-&gt;opFull.op, info-&gt;datatype, info-&gt;algorithm, info-&gt;protocol);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 准备局部状态 | <code>int stepSize = info-&gt;comm-&gt;buffSizes[info-&gt;protocol]/NCCL_STEPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 0 / 准备局部状态 | <code>int chunkSteps = (info-&gt;protocol == NCCL_PROTO_SIMPLE && info-&gt;algorithm == NCCL_ALGO_RING) ? info-&gt;chunkSteps : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 0 / 准备局部状态 | <code>int sliceSteps = (info-&gt;protocol == NCCL_PROTO_SIMPLE && info-&gt;algorithm == NCCL_ALGO_RING) ? info-&gt;sliceSteps : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 0 / 准备局部状态 | <code>int chunkSize = stepSize*chunkSteps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 0 / 条件判断 | <code>(info-&gt;algorithm == NCCL_ALGO_TREE && info-&gt;protocol == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 2 / 条件判断 | <code>(info-&gt;pattern == ncclPatternTreeUpDown)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 4 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes / (info-&gt;nChannels*chunkSize) &lt; info-&gt;comm-&gt;channels[0].tree.depth*8 && chunkSize &gt; 131072)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 33 | 5 / 调用 / 计算 / 写入 | <code>chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 35 | 4 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes / (info-&gt;nChannels*chunkSize) &lt; info-&gt;comm-&gt;channels[0].tree.depth*4 && chunkSize &gt; 65536)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 36 | 5 / 调用 / 计算 / 写入 | <code>chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 4 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes / (info-&gt;nChannels*chunkSize) &lt; info-&gt;comm-&gt;channels[0].tree.depth && chunkSize &gt; 32768)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 5 / 调用 / 计算 / 写入 | <code>chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 41 | 2 / 调用 / 计算 / 写入 | <code>work-&gt;lastChunkSize = chunkSize / ncclTypeSize(info-&gt;datatype);</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 42 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 43 | 2 / 条件判断 | <code>(info-&gt;algorithm == NCCL_ALGO_COLLNET && info-&gt;protocol == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 4 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes / (info-&gt;nChannels*info-&gt;comm-&gt;channels[0].collTree.nHeads*chunkSize) &lt; info-&gt;comm-&gt;channels[0].collTree.depth*64 && chunkSize &gt; 131072)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 46 | 5 / 调用 / 计算 / 写入 | <code>chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 48 | 4 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes / (info-&gt;nChannels*info-&gt;comm-&gt;channels[0].collTree.nHeads*chunkSize) &lt; info-&gt;comm-&gt;channels[0].collTree.depth*8 && chunkSize &gt; 65536)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 49 | 5 / 调用 / 计算 / 写入 | <code>chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 51 | 4 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes / (info-&gt;nChannels*info-&gt;comm-&gt;channels[0].collTree.nHeads*chunkSize) &lt; info-&gt;comm-&gt;channels[0].collTree.depth*8 && chunkSize &gt; 32768)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 52 | 5 / 调用 / 计算 / 写入 | <code>chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 54 | 4 / 调用 / 计算 / 写入 | <code>work-&gt;lastChunkSize = chunkSize / ncclTypeSize(info-&gt;datatype);</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 55 | 4 / 调用 / 计算 / 写入 | <code>work-&gt;direct = (info-&gt;nBytes / info-&gt;nChannels &lt;= 1024*1024) ? NCCL_DIRECT_WRITE : NCCL_DIRECT_READ;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 57 | 4 / 条件判断 | <code>(info-&gt;protocol == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 58 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 59 | 6 / 准备局部状态 | <code>const ssize_t sliceSize = stepSize*sizeof(uint64_t)/sizeof(union ncclLLFifoLine);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 60 | 6 / 准备局部状态 | <code>const ssize_t loopSize = info-&gt;nChannels*info-&gt;nchunksPerLoop*(ssize_t)sliceSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 61 | 6 / 调用 / 计算 / 写入 | <code>work-&gt;lastChunkSize = DIVUP((info-&gt;nBytes-(info-&gt;nBytes/loopSize)*loopSize), info-&gt;nChannels*info-&gt;nchunksPerLoop);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 6 / 调用 / 计算 / 写入 | <code>ALIGN_SIZE(work-&gt;lastChunkSize, info-&gt;nThreads*sizeof(uint64_t));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 63 | 6 / 调用 / 计算 / 写入 | <code>work-&gt;lastChunkSize /= ncclTypeSize(info-&gt;datatype);</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 64 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 65 | 6 / 条件判断 | <code>(info-&gt;algorithm == NCCL_ALGO_TREE && info-&gt;protocol == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 66 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 67 | 8 / 准备局部状态 | <code>int nNodes = info-&gt;comm-&gt;nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 68 | 8 / 准备局部状态 | <code>float ppn = info-&gt;comm-&gt;nRanks / (float)nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 69 | 8 / 准备局部状态 | <code>float nstepsLL128 = 1+log2i(nNodes) + 0.1*ppn;</code> | 用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。 |
| 70 | 8 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes / (info-&gt;nChannels*chunkSize) &lt; nstepsLL128*64/ppn && chunkSize &gt; 131072)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 71 | 9 / 调用 / 计算 / 写入 | <code>chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 8 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 73 | 8 / 循环 | <code>初始 ；继续条件 (info-&gt;nBytes / (info-&gt;nChannels*chunkSize) &lt; nstepsLL128*16/ppn && chunkSize &gt; 32768)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 74 | 9 / 调用 / 计算 / 写入 | <code>chunkSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 75 | 8 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 76 | 8 / 调用 / 计算 / 写入 | <code>work-&gt;lastChunkSize = chunkSize*NCCL_LL128_DATAELEMS/(NCCL_LL128_LINEELEMS*ncclTypeSize(info-&gt;datatype));</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 77 | 0 / 准备局部状态 | <code>int chunkEffectiveSize = chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 78 | 0 / 条件判断 | <code>(info-&gt;protocol == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 79 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 80 | 2 / 调用 / 计算 / 写入 | <code>chunkEffectiveSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 81 | 0 / 条件判断 | <code>(info-&gt;protocol == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 82 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 83 | 2 / 调用 / 计算 / 写入 | <code>chunkEffectiveSize = (chunkSize / NCCL_LL128_LINEELEMS) * NCCL_LL128_DATAELEMS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 0 / 准备局部状态 | <code>int nLoops = (int)(DIVUP(info-&gt;nBytes, (((size_t)(info-&gt;nChannels))*info-&gt;nchunksPerLoop*chunkEffectiveSize)));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 85 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;nsteps = info-&gt;nstepsPerLoop * nLoops * chunkSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 86 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;sliceSteps = sliceSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 87 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;chunkSteps = chunkSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 88 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;chunkSize = chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 89 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;protocol = info-&gt;protocol;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 90 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;dtype = info-&gt;datatype;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 91 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;redOp = info-&gt;algorithm != NCCL_ALGO_COLLNET ? ncclNumOps : info-&gt;opFull.op==ncclDevPreMulSum &#124;&#124; info-&gt;opFull.op==ncclDevSumPostDiv ? ncclSum : info-&gt;op;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 92 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;pattern = info-&gt;pattern;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 93 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;root = info-&gt;root;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 94 | 0 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;nbytes = stepSize*proxyOp-&gt;sliceSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 95 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_COLL,"opCount %lx slicesteps %d spl %d cpl %d nbytes %zi -&gt; protocol %d nchannels %d nthreads %d, nloops %d nsteps %d chunksize %d comm %p", proxyOp-&gt;opCount, sliceSteps, info-&gt;nstepsPerLoop, info-&gt;nchunksPerLoop, info-&gt;nBytes, info-&gt;protocol, info-&gt;nChannels, info-&gt;nThreads, nLoops, proxyOp-&gt;nsteps, chunkSize, info-&gt;comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 96 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int collNetTypeSupport = 0;
  // Check whether algo and proto have been preset (as in aggregation case)
  // If so, skip the calculation
  if (info->nChannels > 0 && info->nThreads > 0) goto comp_next;
  NCCLCHECK(getCollNetSupport(info, &collNetTypeSupport));
  NCCLCHECK(getAlgoInfo(info, collNetTypeSupport, 1));

comp_next:
  // Set nstepsPerLoop and nchunksPerLoop
  NCCLCHECK(getPatternInfo(info));
  NCCLCHECK(getLoopInfo(info));

  work->header.type = ncclWorkTypeColl;
  work->sendbuff = info->sendbuff;
  work->recvbuff = info->recvbuff;
  work->root = info->root;
  work->count = info->count;
  work->nChannels = info->nChannels;
  work->header.nWarps = info->nThreads / WARP_SIZE;
  work->redOpArg = info->opFull.scalarArg;
  work->redOpArgIsPtr = info->opFull.scalarArgIsPtr;

  if (info->comm->nRanks == 1) {
    // one-rank reduce index
    work->header.funcIndex = 1 + int(info->datatype);
    return ncclSuccess;
  }

  work->header.funcIndex = FUNC_INDEX(info->coll, info->opFull.op, info->datatype, info->algorithm, info->protocol);

/* 中文精读：分片尺寸的第一层：协议环形缓冲区
 * 这里 stepSize/chunkSize 都是字节。NCCL_STEPS=8，把协议缓冲分成 8 个可循环使用的 step。
 * 只有 Simple+Ring 使用 API 给定的 chunkSteps/sliceSteps；其余组合在这里按 1 step 处理。
 * chunkSize=stepSize*chunkSteps；一个算法块可含多个 slice，每个 slice 占 sliceSteps 个 step。
 * GPU 侧会再除 sizeof(T) 转换成元素数。不能把主机的字节尺寸直接代进设备下标。
 * 采用有限槽位和分片，是为了让 GPU 计算、网络传输、下一块处理形成流水，同时限制临时内存。
 */
  int stepSize   = info->comm->buffSizes[info->protocol]/NCCL_STEPS;
  int chunkSteps = (info->protocol == NCCL_PROTO_SIMPLE && info->algorithm == NCCL_ALGO_RING) ? info->chunkSteps : 1;
  int sliceSteps = (info->protocol == NCCL_PROTO_SIMPLE && info->algorithm == NCCL_ALGO_RING) ? info->sliceSteps : 1;
  int chunkSize  = stepSize*chunkSteps;

  // Compute lastChunkSize
  if (info->algorithm == NCCL_ALGO_TREE && info->protocol == NCCL_PROTO_SIMPLE) {
    if (info->pattern == ncclPatternTreeUpDown) {
      // Optimize chunkSize / nSteps
      while (info->nBytes / (info->nChannels*chunkSize) < info->comm->channels[0].tree.depth*8 && chunkSize > 131072) chunkSize /= 2;
      while (info->nBytes / (info->nChannels*chunkSize) < info->comm->channels[0].tree.depth*4 && chunkSize > 65536) chunkSize /= 2;
      while (info->nBytes / (info->nChannels*chunkSize) < info->comm->channels[0].tree.depth && chunkSize > 32768) chunkSize /= 2;
    }
    // Use lastChunkSize as chunkSize
    work->lastChunkSize = chunkSize / ncclTypeSize(info->datatype);
  } else if (info->algorithm == NCCL_ALGO_COLLNET && info->protocol == NCCL_PROTO_SIMPLE) {
    // Optimize chunkSize / nSteps
    while (info->nBytes / (info->nChannels*info->comm->channels[0].collTree.nHeads*chunkSize) < info->comm->channels[0].collTree.depth*64 && chunkSize > 131072) chunkSize /= 2;
    while (info->nBytes / (info->nChannels*info->comm->channels[0].collTree.nHeads*chunkSize) < info->comm->channels[0].collTree.depth*8 && chunkSize > 65536) chunkSize /= 2;
    while (info->nBytes / (info->nChannels*info->comm->channels[0].collTree.nHeads*chunkSize) < info->comm->channels[0].collTree.depth*8 && chunkSize > 32768) chunkSize /= 2;
    // Use lastChunkSize as chunkSize
    work->lastChunkSize = chunkSize / ncclTypeSize(info->datatype);
    // Set direct direction for broadcast-gather (read or write)
    work->direct = (info->nBytes / info->nChannels <= 1024*1024) ? NCCL_DIRECT_WRITE : NCCL_DIRECT_READ;
  } else if (info->protocol == NCCL_PROTO_LL) {
    const ssize_t sliceSize = stepSize*sizeof(uint64_t)/sizeof(union ncclLLFifoLine);
    const ssize_t loopSize = info->nChannels*info->nchunksPerLoop*(ssize_t)sliceSize;
    work->lastChunkSize = DIVUP((info->nBytes-(info->nBytes/loopSize)*loopSize), info->nChannels*info->nchunksPerLoop);
    ALIGN_SIZE(work->lastChunkSize, info->nThreads*sizeof(uint64_t));
    work->lastChunkSize /= ncclTypeSize(info->datatype);
  } else if (info->algorithm == NCCL_ALGO_TREE && info->protocol == NCCL_PROTO_LL128) {
    int nNodes = info->comm->nNodes;
    float ppn = info->comm->nRanks / (float)nNodes;
    float nstepsLL128 = 1+log2i(nNodes) + 0.1*ppn;
    while (info->nBytes / (info->nChannels*chunkSize) < nstepsLL128*64/ppn && chunkSize > 131072) chunkSize /= 2;
    while (info->nBytes / (info->nChannels*chunkSize) < nstepsLL128*16/ppn && chunkSize > 32768) chunkSize /= 2;
    // Use lastChunkSize as chunkSize
    work->lastChunkSize = chunkSize*NCCL_LL128_DATAELEMS/(NCCL_LL128_LINEELEMS*ncclTypeSize(info->datatype));
  }

/* 中文精读：让 GPU 与 Proxy 对同一分片达成一致
 * LL 的 16 字节 FIFO 行只有 8 字节有效数据；LL128 每 128 字节中有 120 字节有效数据。
 * 所以用于覆盖用户数据的 chunkEffectiveSize 要扣掉 flag 开销，而网络缓冲仍按物理字节分配。
 * nLoops=ceil(nBytes/(nChannels*nchunksPerLoop*chunkEffectiveSize))，
 * nsteps=nstepsPerLoop*nLoops*chunkSteps。尾块不足也要保持两侧推进步数匹配。
 * 这个公式是本版本调度器给 Proxy 的预算，不表示每个 step 必然对应一个独立 verbs 请求。
 */
  // Compute nSteps for proxies
  int chunkEffectiveSize = chunkSize;
  if (info->protocol == NCCL_PROTO_LL) chunkEffectiveSize /= 2;
  if (info->protocol == NCCL_PROTO_LL128) chunkEffectiveSize = (chunkSize / NCCL_LL128_LINEELEMS) * NCCL_LL128_DATAELEMS;
  //if (info->comm->rank == 0) printf("Coll %d, size %ld -> %dx%d, chunkSize %d (algo %d proto%d)\n", info->coll, info->nBytes, info->nChannels, info->nThreads, chunkSize, info->algorithm, info->protocol);
  int nLoops = (int)(DIVUP(info->nBytes, (((size_t)(info->nChannels))*info->nchunksPerLoop*chunkEffectiveSize)));
  proxyOp->nsteps = info->nstepsPerLoop * nLoops * chunkSteps;
  proxyOp->sliceSteps = sliceSteps;
  proxyOp->chunkSteps = chunkSteps;
  proxyOp->chunkSize = chunkSize;
  proxyOp->protocol = info->protocol;
  proxyOp->dtype = info->datatype;
  proxyOp->redOp = info->algorithm != NCCL_ALGO_COLLNET ? ncclNumOps : // Only set redOp when using CollNet
                     info->opFull.op==ncclDevPreMulSum || info->opFull.op==ncclDevSumPostDiv ? ncclSum : // Network sees avg as sum
                     info->op;
  proxyOp->pattern = info->pattern;
  proxyOp->root = info->root;
  // This is used by P2P to reduce the receive buffer size. We don't use it in collectives
  // because some protocols need to transmit more than the total size, plus they sometimes
  // round up
  proxyOp->nbytes = stepSize*proxyOp->sliceSteps;

  TRACE(NCCL_COLL,"opCount %lx slicesteps %d spl %d cpl %d nbytes %zi -> protocol %d nchannels %d nthreads %d, nloops %d nsteps %d chunksize %d comm %p",
      proxyOp->opCount, sliceSteps, info->nstepsPerLoop, info->nchunksPerLoop, info->nBytes, info->protocol, info->nChannels, info->nThreads,
      nLoops, proxyOp->nsteps, chunkSize, info->comm);
  return ncclSuccess;
}
```

</details>

<a id="f0407"></a>

## F0407：checkSetStream

用途：保存本批使用的 CUDA stream，并检查 Group 内同一 communicator 的 stream 使用契约。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(info-&gt;comm-&gt;userStreamSet == false)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;comm-&gt;userStream = info-&gt;stream;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;comm-&gt;userStreamSet = true;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 6 | 2 / 条件判断 | <code>(info-&gt;stream != info-&gt;comm-&gt;userStream)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>WARN("Error : mixing different streams within a group call is not supported.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 4 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
 if (info->comm->userStreamSet == false) {
    info->comm->userStream = info->stream;
    info->comm->userStreamSet = true;
  } else if (info->stream != info->comm->userStream) {
    WARN("Error : mixing different streams within a group call is not supported.");
    return ncclInvalidUsage;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0408"></a>

## F0408：ncclRegBuffAndExchange

用途：图注册优化下导出/导入用户缓冲 IPC 信息并交换地址，让 CollNet 可直接使用注册缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclComm_t comm = info-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm-&gt;localRanks == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(comm-&gt;pfnCuMemGetAddressRange == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 准备局部状态 | <code>ncclResult_t ret = ncclSuccess;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>struct ncclBuffRegHandle regHandles[NCCL_MAX_LOCAL_RANKS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaIpcGetMemHandle(&regHandles[comm-&gt;localRank].sendBuffIpc, (void*)info-&gt;sendbuff), ret, reg_fallback);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaIpcGetMemHandle(&regHandles[comm-&gt;localRank].recvBuffIpc, (void*)info-&gt;recvbuff), ret, reg_fallback);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 12 | 0 / 准备局部状态 | <code>void* baseAddr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>size_t size;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(comm-&gt;pfnCuMemGetAddressRange(&baseAddr, &size, (void*)info-&gt;sendbuff));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>regHandles[comm-&gt;localRank].sendBuffOffset = (char*)info-&gt;sendbuff - (char*)baseAddr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(comm-&gt;pfnCuMemGetAddressRange(&baseAddr, &size, (void*)info-&gt;recvbuff));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>regHandles[comm-&gt;localRank].recvBuffOffset = (char*)info-&gt;recvbuff - (char*)baseAddr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_COLL, "Base %p size %lu offset %ld", baseAddr, size, regHandles[comm-&gt;localRank].recvBuffOffset);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapIntraNodeAllGather(comm-&gt;bootstrap, comm-&gt;localRankToRank, comm-&gt;localRank, comm-&gt;localRanks, regHandles, sizeof(struct ncclBuffRegHandle)));</code> | 失败时可能提前return，成功才继续；只在所选节点内 rank 集合交换固定大小控制信息，按局部索引映射到全局 rank。 |
| 20 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;localRanks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 21 | 1 / 条件判断 | <code>(i == comm-&gt;localRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 3 / 调用 / 计算 / 写入 | <code>regInfo-&gt;sendbuffsBase[i] = regInfo-&gt;recvbuffsBase[i] = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcOpenMemHandle(regInfo-&gt;sendbuffsBase+i, regHandles[i].sendBuffIpc, cudaIpcMemLazyEnablePeerAccess));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcOpenMemHandle(regInfo-&gt;recvbuffsBase+i, regHandles[i].recvBuffIpc, cudaIpcMemLazyEnablePeerAccess));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>regInfo-&gt;sendbuffs[i] = (char*)regInfo-&gt;sendbuffsBase[i] + regHandles[i].sendBuffOffset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 1 / 调用 / 计算 / 写入 | <code>regInfo-&gt;recvbuffs[i] = (char*)regInfo-&gt;recvbuffsBase[i] + regHandles[i].recvBuffOffset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>regInfo-&gt;nBuffs = comm-&gt;localRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_COLL, "Rank %d exchanged %d buffers", comm-&gt;rank, regInfo-&gt;nBuffs);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 33 | 0 / 跳转标签 | <code>reg_fallback</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 34 | 1 / 调用 / 计算 / 写入 | <code>(void)ret;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>WARN("Unable to register user buffers");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclComm_t comm = info->comm;
  if (comm->localRanks == 1) return ncclSuccess;
  if (comm->pfnCuMemGetAddressRange == NULL) return ncclSuccess;  // CUDA toolkit or driver version too old

  ncclResult_t ret = ncclSuccess;
  struct ncclBuffRegHandle regHandles[NCCL_MAX_LOCAL_RANKS];
  // Get IPC handles
  // Note: the handle only corresponds to the base address of the allocation
  CUDACHECKGOTO(cudaIpcGetMemHandle(&regHandles[comm->localRank].sendBuffIpc, (void*)info->sendbuff), ret, reg_fallback);
  CUDACHECKGOTO(cudaIpcGetMemHandle(&regHandles[comm->localRank].recvBuffIpc, (void*)info->recvbuff), ret, reg_fallback);
  // Get offset of user buffer within allocation
  void* baseAddr;
  size_t size;
  // Get base address
  CUDACHECK(comm->pfnCuMemGetAddressRange(&baseAddr, &size, (void*)info->sendbuff));
  regHandles[comm->localRank].sendBuffOffset = (char*)info->sendbuff - (char*)baseAddr;
  CUDACHECK(comm->pfnCuMemGetAddressRange(&baseAddr, &size, (void*)info->recvbuff));
  regHandles[comm->localRank].recvBuffOffset = (char*)info->recvbuff - (char*)baseAddr;
  TRACE(NCCL_COLL, "Base %p size %lu offset %ld", baseAddr, size, regHandles[comm->localRank].recvBuffOffset);

  // Exchange handles within node
  NCCLCHECK(bootstrapIntraNodeAllGather(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, regHandles, sizeof(struct ncclBuffRegHandle)));
  // Open handles at local process
  for (int i=0; i<comm->localRanks; i++) {
    // Skip myself
    if (i == comm->localRank) {
      regInfo->sendbuffsBase[i] = regInfo->recvbuffsBase[i] = NULL;
      continue;
    }
    // Get base address of mapping
    CUDACHECK(cudaIpcOpenMemHandle(regInfo->sendbuffsBase+i, regHandles[i].sendBuffIpc, cudaIpcMemLazyEnablePeerAccess));
    CUDACHECK(cudaIpcOpenMemHandle(regInfo->recvbuffsBase+i, regHandles[i].recvBuffIpc, cudaIpcMemLazyEnablePeerAccess));
    // Get real buffer address by adding offset in the mapping
    regInfo->sendbuffs[i] = (char*)regInfo->sendbuffsBase[i] + regHandles[i].sendBuffOffset;
    regInfo->recvbuffs[i] = (char*)regInfo->recvbuffsBase[i] + regHandles[i].recvBuffOffset;
  }
  // Marks the operation as being buffer registered
  regInfo->nBuffs = comm->localRanks;
  TRACE(NCCL_COLL, "Rank %d exchanged %d buffers", comm->rank, regInfo->nBuffs);
  return ncclSuccess;

reg_fallback:
  // If we cannot register specific buffer types, we just bypass this stage, and continue without failing
  (void)ret;
  WARN("Unable to register user buffers");
  return ncclSuccess;
}
```

</details>

<a id="f0409"></a>

## F0409：ncclSetupCollKernel

用途：创建 collective 排队模板、计算 GPU/Proxy 参数、更新 grid/block 配置并选择入口 kernel。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclComm_t comm = info-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm-&gt;nRanks == 1 && info-&gt;op &lt; ncclNumOps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 条件判断 | <code>(info-&gt;sendbuff != info-&gt;recvbuff)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaMemcpyAsync(info-&gt;recvbuff, info-&gt;sendbuff, info-&gt;nBytes, cudaMemcpyDeviceToDevice, info-&gt;stream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 准备局部状态 | <code>struct ncclQueueElem* eqElem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(comm-&gt;enqueueInfo-&gt;elemList-&gt;getNewElem(&eqElem));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 10 | 0 / 准备局部状态 | <code>struct ncclWork* work = &eqElem-&gt;work;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(computeColl(info, work-&gt;elems, &eqElem-&gt;proxyOp));</code> | 失败时可能提前return，成功才继续；把一项 API 任务编译成 GPU work 与 ProxyOp，计算协议 chunk/slice、尾块和总推进步数。 |
| 12 | 0 / 准备局部状态 | <code>struct cudaLaunchParams* params = comm-&gt;myParams;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;gridDim.x += info-&gt;nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;gridDim.x = std::min&lt;unsigned&gt;(params-&gt;gridDim.x, comm-&gt;nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;blockDim.x = std::max&lt;unsigned&gt;(params-&gt;blockDim.x, info-&gt;nThreads);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;enqueueInfo-&gt;maxChannels = params-&gt;gridDim.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 条件判断 | <code>(params-&gt;func == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>params-&gt;func = ncclKerns[work-&gt;header.funcIndex];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 2 / 条件判断 | <code>(work-&gt;header.type == ncclWorkTypeColl)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 4 / 调用 / 计算 / 写入 | <code>memcpy(&comm-&gt;args, work-&gt;elems, sizeof(struct ncclWorkElem));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;args.bid = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;args.header.isLast = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 0 / 条件判断 | <code>(comm-&gt;usingCudaGraph && comm-&gt;graphRegister == 1 && info-&gt;algorithm == NCCL_ALGO_COLLNET && comm-&gt;intraHighestTransportType == TRANSPORT_P2P && comm-&gt;intraRanks == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclRegBuffAndExchange(info, &eqElem-&gt;buffRegInfo));</code> | 失败时可能提前return，成功才继续；图注册优化下导出/导入用户缓冲 IPC 信息并交换地址，让 CollNet 可直接使用注册缓冲。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;enqueueInfo-&gt;nRegBuffs += eqElem-&gt;buffRegInfo.nBuffs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>work-&gt;header.type = ncclWorkTypeRegColl;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;args.header.type = ncclWorkTypeUnused;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclComm_t comm = info->comm;
  if (comm->nRanks == 1 &&
      // User-defined reduction ops may need alter the data even for unitary reductions
      info->op < ncclNumOps) {
    if (info->sendbuff != info->recvbuff)
      CUDACHECK(cudaMemcpyAsync(info->recvbuff, info->sendbuff, info->nBytes, cudaMemcpyDeviceToDevice, info->stream));
    return ncclSuccess;
  }

  // Compute cuda kernel arg and proxy arg templates
  struct ncclQueueElem* eqElem;
  NCCLCHECK(comm->enqueueInfo->elemList->getNewElem(&eqElem));
  struct ncclWork* work = &eqElem->work;
  NCCLCHECK(computeColl(info, work->elems, &eqElem->proxyOp));

  // Determine grid size
  struct cudaLaunchParams* params = comm->myParams;
  params->gridDim.x += info->nChannels;
  params->gridDim.x = std::min<unsigned>(params->gridDim.x, comm->nChannels);
  params->blockDim.x = std::max<unsigned>(params->blockDim.x, info->nThreads);
  comm->enqueueInfo->maxChannels = params->gridDim.x;  // params may be varied by a second graph hence we need to capture it here

  // Inline the first kernel
  if (params->func == NULL) {
    params->func = ncclKerns[work->header.funcIndex];
    if (work->header.type == ncclWorkTypeColl) {
      // Copy the first operation to the inline argument. Type may be set later to
      // ncclWorkTypeUnused if we have more than one coll element.
      memcpy(&comm->args, work->elems, sizeof(struct ncclWorkElem));
      comm->args.bid = 0;    // Only inline for channel 0
      comm->args.header.isLast = 1; // I am so far the last element
    }
  }

  // Register and exchange input and output buffers
  if (comm->usingCudaGraph &&                   // only in CUDA graph mode
      comm->graphRegister == 1 &&               // when registration is enabled
      info->algorithm == NCCL_ALGO_COLLNET &&   // limited to CollNet for now
      comm->intraHighestTransportType == TRANSPORT_P2P && // only when all ranks can p2p each other
      comm->intraRanks == 1) {                  // only in multi-process mode
    NCCLCHECK(ncclRegBuffAndExchange(info, &eqElem->buffRegInfo));
    comm->enqueueInfo->nRegBuffs += eqElem->buffRegInfo.nBuffs;
    work->header.type = ncclWorkTypeRegColl;
    // Disable inline argument because we need kernel to copy the entire ncclWork from workFifo
    // because the registered addresses are in ncclWorkElemReg
    comm->args.header.type = ncclWorkTypeUnused;
  }

  return ncclSuccess;
}
```

</details>

<a id="f0410"></a>

## F0410：findShortestChannel

用途：getNextChannel 的最短队列策略：按已排队字节数选负载较小的逻辑 channel。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>size_t minSize = SIZE_MAX;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int minC = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;nChannels；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 1 / 条件判断 | <code>(channel-&gt;totalSize &lt; minSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>minSize = channel-&gt;totalSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>minC = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 函数返回 | <code>return minC;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  size_t minSize = SIZE_MAX;
  int minC = 0;
  for (int c=0; c<comm->nChannels; c++) {
    struct ncclChannel* channel = comm->channels+c;
    if (channel->totalSize < minSize) {
      minSize = channel->totalSize;
      minC = c;
    }
  }
  return minC;
}
```

</details>

<a id="f0411"></a>

## F0411：getNextChannel

用途：按轮转或最短队列策略为当前 collective 分片选择实际 channelId。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int nextChannel = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(aggMode && comm-&gt;asyncAllocMode == ncclComm::SHORTEST_QUEUE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>nextChannel = findShortestChannel(comm);</code> | getNextChannel 的最短队列策略：按已排队字节数选负载较小的逻辑 channel。 |
| 5 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>nextChannel = comm-&gt;lastChannel % comm-&gt;nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;lastChannel++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 函数返回 | <code>return nextChannel;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int nextChannel = 0;
  if (aggMode && comm->asyncAllocMode == ncclComm::SHORTEST_QUEUE) {
    nextChannel = findShortestChannel(comm);
  } else {
    nextChannel = comm->lastChannel % comm->nChannels;
    comm->lastChannel++;
  }
  return nextChannel;
}
```

</details>

<a id="f0412"></a>

## F0412：ncclSetupAsyncKernels

用途：把 Group 保存的一组 collective 转成发射任务，对同构批次可统一选型并聚合工作元素。

- 无任务直接返回，单任务走独立选型，多任务才估计每通道目标字节量。
- 同构操作可对整批统一选算法/协议/线程，单项仍有各自地址、count和channel分工。
- 异构任务分别准备；最后只清本轮异步计数，不拆掉长期连接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(comm-&gt;asyncOpCount == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 条件判断 | <code>(comm-&gt;asyncOpCount == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 4 / 准备局部状态 | <code>struct ncclInfo* info = comm-&gt;asyncOps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>info-&gt;nChannels = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSetupCollKernel(info));</code> | 失败时可能提前return，成功才继续；创建 collective 排队模板、计算 GPU/Proxy 参数、更新 grid/block 配置并选择入口 kernel。 |
| 10 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 4 / 准备局部状态 | <code>size_t channelSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 4 / 条件判断 | <code>(comm-&gt;channelSize &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 6 / 调用 / 计算 / 写入 | <code>channelSize = comm-&gt;channelSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 16 | 6 / 条件判断 | <code>(comm-&gt;collNetSupport && comm-&gt;asyncOps[0].coll == ncclFuncAllReduce)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 8 / 调用 / 计算 / 写入 | <code>channelSize = 256 * 1024;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 20 | 8 / 调用 / 计算 / 写入 | <code>channelSize = NCCL_AGG_CHANNEL_SIZE * std::min(16, comm-&gt;nRanks);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 21 | 4 / 循环 | <code>初始 ；继续条件 (comm-&gt;asyncTotalSize &lt; channelSize * comm-&gt;nChannels && channelSize &gt; NCCL_MIN_CHANNEL_SIZE)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 5 / 调用 / 计算 / 写入 | <code>channelSize /= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 24 | 4 / 准备局部状态 | <code>int channelUsed = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 4 / 准备局部状态 | <code>int homogeneous = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 4 / 准备局部状态 | <code>int allCollNetSupport = comm-&gt;collNetSupport;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 4 / 循环 | <code>初始 int c = 0;；继续条件 c &lt; comm-&gt;asyncOpCount；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 28 | 5 / 准备局部状态 | <code>struct ncclInfo* info = comm-&gt;asyncOps+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 5 / 调用 / 计算 / 写入 | <code>info-&gt;nChannels = std::min(std::max(1, (int)DIVUP(info-&gt;nBytes, channelSize)), comm-&gt;nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 30 | 5 / 调用 / 计算 / 写入 | <code>channelUsed += info-&gt;nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 5 / 调用 / 计算 / 写入 | <code>homogeneous &= info-&gt;coll == comm-&gt;asyncOps[0].coll && info-&gt;opFull.op == comm-&gt;asyncOps[0].opFull.op && info-&gt;datatype == comm-&gt;asyncOps[0].datatype;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 5 / 条件判断 | <code>(allCollNetSupport &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getCollNetSupport(info, &allCollNetSupport));</code> | 失败时可能提前return，成功才继续；检查所选数据类型/归约是否被 CollNet 插件支持，以屏蔽不能正确执行的候选。 |
| 35 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 36 | 4 / 准备局部状态 | <code>struct ncclInfo total;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 4 / 调用 / 计算 / 写入 | <code>total.comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>total.coll = comm-&gt;asyncOps[0].coll;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 4 / 调用 / 计算 / 写入 | <code>total.nBytes = comm-&gt;asyncTotalSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 4 / 调用 / 计算 / 写入 | <code>total.nChannels = std::min(channelUsed, comm-&gt;nChannels);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 41 | 4 / 准备局部状态 | <code>int perChannelOps = DIVUP(channelUsed, total.nChannels);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 4 / 条件判断 | <code>(homogeneous)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getAlgoInfo(&total, allCollNetSupport, perChannelOps));</code> | 失败时可能提前return，成功才继续；遍历可用算法协议的时间模型，再按消息尺寸缩减本次 channel 和线程数，平衡延迟与带宽。 |
| 45 | 4 / 循环 | <code>初始 int c = 0;；继续条件 c &lt; comm-&gt;asyncOpCount；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 46 | 5 / 准备局部状态 | <code>struct ncclInfo* info = comm-&gt;asyncOps+c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 47 | 5 / 条件判断 | <code>(homogeneous)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 7 / 调用 / 计算 / 写入 | <code>info-&gt;algorithm = total.algorithm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 7 / 调用 / 计算 / 写入 | <code>info-&gt;protocol = total.protocol;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 7 / 调用 / 计算 / 写入 | <code>info-&gt;nThreads = total.nThreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSetupCollKernel(info));</code> | 失败时可能提前return，成功才继续；创建 collective 排队模板、计算 GPU/Proxy 参数、更新 grid/block 配置并选择入口 kernel。 |
| 53 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 54 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;args.header.type = ncclWorkTypeUnused;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncOpCount = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncTotalSize = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (comm->asyncOpCount == 0) {
    return ncclSuccess;
  } else if (comm->asyncOpCount == 1) {
    // No aggregation
    struct ncclInfo* info = comm->asyncOps;
    info->nChannels = 0;
    NCCLCHECK(ncclSetupCollKernel(info));
  } else {
    // Aggregation
    // Determine a per-channel chunk size used to divide an operation into multiple channels
    size_t channelSize;
    if (comm->channelSize > 0) {
      // Set by user
      channelSize = comm->channelSize;
    } else if (comm->collNetSupport && comm->asyncOps[0].coll == ncclFuncAllReduce) {
      // CollNet specific size (tuned based on experiments)
      channelSize = 256 * 1024;
    } else {
      // Latency increases as scale increases
      // We would thus want to increase the chunk size to compensate for the lost efficiency
      channelSize = NCCL_AGG_CHANNEL_SIZE * std::min(16, comm->nRanks);
    }
    // Reduce the per-channel size if we cannot fully utilize the channels
    while (comm->asyncTotalSize < channelSize * comm->nChannels && channelSize > NCCL_MIN_CHANNEL_SIZE) channelSize /= 2;
    // Check whether the ops have same reduce and data types (and hence can be packed in same ncclWork)
    int channelUsed = 0;
    int homogeneous = 1;
    int allCollNetSupport = comm->collNetSupport;
    for (int c = 0; c < comm->asyncOpCount; c++) {
      struct ncclInfo* info = comm->asyncOps+c;
      info->nChannels = std::min(std::max(1, (int)DIVUP(info->nBytes, channelSize)), comm->nChannels); // assign number of channels
      channelUsed += info->nChannels;
      // We can use fast path if all collectives are the same
      homogeneous &= info->coll == comm->asyncOps[0].coll &&
                     info->opFull.op == comm->asyncOps[0].opFull.op &&
                     info->datatype == comm->asyncOps[0].datatype;
      if (allCollNetSupport > 0) NCCLCHECK(getCollNetSupport(info, &allCollNetSupport));
    }
    // Compute algo, proto, nthreads for the entire kernel
    // Prepare a synthetic op info to calculate the collective algo
    struct ncclInfo total;
    total.comm = comm;
    total.coll = comm->asyncOps[0].coll;
    total.nBytes = comm->asyncTotalSize;
    total.nChannels = std::min(channelUsed, comm->nChannels);
    int perChannelOps = DIVUP(channelUsed, total.nChannels);
    if (homogeneous) NCCLCHECK(getAlgoInfo(&total, allCollNetSupport, perChannelOps));
    // Set for each op
    for (int c = 0; c < comm->asyncOpCount; c++) {
      struct ncclInfo* info = comm->asyncOps+c;
      if (homogeneous) {
        // Set fields to skip the individual computeColl in ncclSetupCollKernel
        info->algorithm = total.algorithm;
        info->protocol = total.protocol;
        info->nThreads = total.nThreads;
      }
      NCCLCHECK(ncclSetupCollKernel(info));
    }
    comm->args.header.type = ncclWorkTypeUnused;  // disable inline argument
  }
  // Reset counters
  comm->asyncOpCount = 0;
  comm->asyncTotalSize = 0;
  return ncclSuccess;
}
```

</details>

<a id="f0413"></a>

## F0413：ncclSaveAsyncColl

用途：复制当前 collective 的主机中间描述到可延迟提交列表，保留 GroupEnd 计算所需参数。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclComm_t comm = info-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm-&gt;asyncOpCount &gt;= NCCL_MAX_OPS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("Too many async operations in progress, max is %d", NCCL_MAX_OPS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>memcpy(comm-&gt;asyncOps+comm-&gt;asyncOpCount, info, sizeof(struct ncclInfo));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncOpCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;asyncTotalSize += info-&gt;nBytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclComm_t comm = info->comm;
  if (comm->asyncOpCount >= NCCL_MAX_OPS) {
    WARN("Too many async operations in progress, max is %d", NCCL_MAX_OPS);
    return ncclInvalidUsage;
  }
  memcpy(comm->asyncOps+comm->asyncOpCount, info, sizeof(struct ncclInfo));
  comm->asyncOpCount++;
  comm->asyncTotalSize += info->nBytes;
  return ncclSuccess;
}
```

</details>

<a id="f0414"></a>

## F0414：ncclSaveP2p

用途：按 peer 保存 P2P 消息和字节数，并标记尚未建立的 channel/方向/索引1连接需求。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclComm* comm = info-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int peer = info-&gt;root;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>ssize_t nBytes = info-&gt;count*ncclTypeSize(info-&gt;datatype);</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 4 | 0 / 准备局部状态 | <code>int channelBaseId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelComputeBase(comm, peer, info-&gt;coll, &channelBaseId));</code> | 失败时可能提前return，成功才继续；由节点距离与节点内 rank 差计算 P2P 基准通道，Send/Recv 用相反公式确保两端匹配。 |
| 6 | 0 / 条件判断 | <code>(info-&gt;coll == ncclFuncSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 条件判断 | <code>(peer != comm-&gt;rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 4 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;p2pnChannelsPerPeer；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 5 / 准备局部状态 | <code>int channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelComputeFromBase(comm, channelBaseId, c, &channelId));</code> | 失败时可能提前return，成功才继续；将基准通道加位反转分散偏移，对全局 P2P 通道空间取模，多个 peer 可映射到同通道。 |
| 13 | 5 / 条件判断 | <code>(comm-&gt;channels[channelId].peers[peer].send[1].connected == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 7 / 调用 / 计算 / 写入 | <code>comm-&gt;connectSend[peer] &#124;= (1&lt;&lt;channelId);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 7 / 调用 / 计算 / 写入 | <code>comm-&gt;connect = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSaveP2pInfo(comm-&gt;p2pSends[info-&gt;root], info-&gt;recvbuff, nBytes));</code> | 失败时可能提前return，成功才继续；按 peer 的可复用列表保存缓冲指针和字节数，供最外层 GroupEnd 统一分块和配对调度。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pSendCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 21 | 2 / 条件判断 | <code>(peer != comm-&gt;rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 循环 | <code>初始 int c=0;；继续条件 c&lt;comm-&gt;p2pnChannelsPerPeer；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 5 / 准备局部状态 | <code>int channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelComputeFromBase(comm, channelBaseId, c, &channelId));</code> | 失败时可能提前return，成功才继续；将基准通道加位反转分散偏移，对全局 P2P 通道空间取模，多个 peer 可映射到同通道。 |
| 26 | 5 / 条件判断 | <code>(comm-&gt;channels[channelId].peers[peer].recv[1].connected == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 7 / 调用 / 计算 / 写入 | <code>comm-&gt;connectRecv[peer] &#124;= (1&lt;&lt;channelId);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 7 / 调用 / 计算 / 写入 | <code>comm-&gt;connect = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSaveP2pInfo(comm-&gt;p2pRecvs[info-&gt;root], info-&gt;recvbuff, nBytes));</code> | 失败时可能提前return，成功才继续；按 peer 的可复用列表保存缓冲指针和字节数，供最外层 GroupEnd 统一分块和配对调度。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;p2pRecvCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclComm* comm = info->comm;
  int peer = info->root;
  ssize_t nBytes = info->count*ncclTypeSize(info->datatype);
  int channelBaseId;
  NCCLCHECK(ncclChannelComputeBase(comm, peer, info->coll, &channelBaseId));
  if (info->coll == ncclFuncSend) {
    if (peer != comm->rank) {
      // Mark channels that need pre-connect
      for (int c=0; c<comm->p2pnChannelsPerPeer; c++) {
        int channelId;
        NCCLCHECK(ncclChannelComputeFromBase(comm, channelBaseId, c, &channelId));
        if (comm->channels[channelId].peers[peer].send[1].connected == 0) { // P2P uses only 1 connector
          comm->connectSend[peer] |= (1<<channelId);
          comm->connect = 1;
        }
      }
    }
    NCCLCHECK(ncclSaveP2pInfo(comm->p2pSends[info->root], info->recvbuff, nBytes));
    comm->p2pSendCount++;
  } else {
    if (peer != comm->rank) {
      // Mark channels that need pre-connect
      for (int c=0; c<comm->p2pnChannelsPerPeer; c++) {
        int channelId;
        NCCLCHECK(ncclChannelComputeFromBase(comm, channelBaseId, c, &channelId));
        if (comm->channels[channelId].peers[peer].recv[1].connected == 0) { // P2P uses only 1 connector
          comm->connectRecv[peer] |= (1<<channelId);
          comm->connect = 1;
        }
      }
    }
    NCCLCHECK(ncclSaveP2pInfo(comm->p2pRecvs[info->root], info->recvbuff, nBytes));
    comm->p2pRecvCount++;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0415"></a>

## F0415：getSegment

用途：在已有 work 里寻找可放入当前类型/peer/方向任务的槽位，避免破坏固定收发分组布局。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(work-&gt;header.type && (work-&gt;header.type != type))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 条件判断 | <code>(type == ncclWorkTypeP2p)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 准备局部状态 | <code>int start = subType == ncclWorkSubTypeRecv ? 0 : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 循环 | <code>初始 int s=start;；继续条件 s&lt;NCCL_MAX_WORK_ELEMENTS_P2P；每轮更新 s+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 3 / 条件判断 | <code>(work-&gt;p2pElems[s].peer == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 5 / 函数返回 | <code>return s;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 3 / 条件判断 | <code>(work-&gt;p2pElems[s].peer == peer)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 5 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 16 | 2 / 条件判断 | <code>(type == ncclWorkTypeRegColl)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 循环 | <code>初始 int s=0;；继续条件 s&lt;NCCL_MAX_WORK_ELEMENTS_REG；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 5 / 条件判断 | <code>(work-&gt;regElems[s].elem.header.type == ncclWorkTypeUnused)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 7 / 函数返回 | <code>return s;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 23 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 24 | 4 / 条件判断 | <code>(type == ncclWorkTypeColl)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 6 / 循环 | <code>初始 int s=0;；继续条件 s&lt;NCCL_MAX_WORK_ELEMENTS；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 7 / 条件判断 | <code>(work-&gt;elems[s].header.type == ncclWorkTypeUnused)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 9 / 函数返回 | <code>return s;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 30 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 0 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (work->header.type && (work->header.type != type)) return -1;

  if (type == ncclWorkTypeP2p) {  // P2P
    int start = subType == ncclWorkSubTypeRecv ? 0 : 1;
    for (int s=start; s<NCCL_MAX_WORK_ELEMENTS_P2P; s+=2) {
      if (work->p2pElems[s].peer == -1) return s;
      // Do not aggregate multiple sends to the same peer (or receives from the same peer)
      if (work->p2pElems[s].peer == peer) return -1;
    }
  } else if (type == ncclWorkTypeRegColl) { // CollNet
    for (int s=0; s<NCCL_MAX_WORK_ELEMENTS_REG; s++) {
      if (work->regElems[s].elem.header.type == ncclWorkTypeUnused) return s;
    }
  } else if (type == ncclWorkTypeColl) {  // Ring or Tree
    for (int s=0; s<NCCL_MAX_WORK_ELEMENTS; s++) {
      if (work->elems[s].header.type == ncclWorkTypeUnused) return s;
    }
  }
  return -1;
}
```

</details>

<a id="f0416"></a>

## F0416：computeP2pWorkElem

用途：把单个已分块 P2P 任务转换成设备元素，记录字节长度、peer、方向与协议 chunkSize。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;header.type = ncclWorkTypeP2p;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;header.funcIndex = FUNC_INDEX_P2P;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;header.nWarps = NCCL_MAX_NTHREADS/WARP_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;buff = info-&gt;recvbuff;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;subType = info-&gt;coll == ncclFuncSend ? ncclWorkSubTypeSend : ncclWorkSubTypeRecv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;count = info-&gt;count;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;chunkSize = info-&gt;chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>elem-&gt;peer = info-&gt;root;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  elem->header.type = ncclWorkTypeP2p;
  elem->header.funcIndex = FUNC_INDEX_P2P;
  elem->header.nWarps = NCCL_MAX_NTHREADS/WARP_SIZE;
  elem->buff = info->recvbuff;
  elem->subType = info->coll == ncclFuncSend ? ncclWorkSubTypeSend : ncclWorkSubTypeRecv;
  elem->count = info->count;
  elem->chunkSize = info->chunkSize;
  elem->peer = info->root;
  return ncclSuccess;
}
```

</details>

<a id="f0417"></a>

## F0417：enqueueSegOp

用途：把元素装进固定大小 work，对 P2P 计算各收发组 warp 范围，对 collective 复制对应描述。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(type == ncclWorkTypeP2p)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>memcpy(work-&gt;p2pElems+s, elem, sizeof(struct ncclWorkElemP2p));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 准备局部状态 | <code>int nelems = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_MAX_WORK_ELEMENTS_P2P；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 3 / 条件判断 | <code>(work-&gt;p2pElems[i].header.type)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 调用 / 计算 / 写入 | <code>nelems = i+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 2 / 准备局部状态 | <code>int ngroups = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 2 / 循环 | <code>初始 ；继续条件 (ngroups &lt; nelems)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>ngroups *= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 2 / 准备局部状态 | <code>int nWarps = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 2 / 循环 | <code>初始 ；继续条件 (nWarps*ngroups &lt;= elem-&gt;header.nWarps/2)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>nWarps *= 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ngroups；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 3 / 调用 / 计算 / 写入 | <code>work-&gt;p2pElems[i].ngroups = ngroups;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>work-&gt;p2pElems[i].warpStart = i*(NCCL_MAX_NTHREADS/WARP_SIZE)/ngroups;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 3 / 准备局部状态 | <code>int extraWarp = nWarps &gt;= 2 ? i%2 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 3 / 调用 / 计算 / 写入 | <code>work-&gt;p2pElems[i].nWarps = nWarps + extraWarp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 24 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>memcpy(work-&gt;elems+s, elem, sizeof(struct ncclWorkElem));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 0 / 条件判断 | <code>(regInfo-&gt;nBuffs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 29 | 0 / 准备局部状态 | <code>struct ncclWorkElemReg* regElem = (struct ncclWorkElemReg*)(work-&gt;elems+s);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_MAX_DIRECT_ARITY；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 31 | 1 / 准备局部状态 | <code>int peer = channel-&gt;collTree.down[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 1 / 条件判断 | <code>(peer == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 35 | 1 / 准备局部状态 | <code>int j = comm-&gt;rankToLocalRank[peer];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 36 | 1 / 条件判断 | <code>(j &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 3 / 调用 / 计算 / 写入 | <code>WARN("Invalid intra-node rank %d for peer %d", j, peer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 40 | 1 / 调用 / 计算 / 写入 | <code>regElem-&gt;dnInputs[i] = regInfo-&gt;sendbuffs[j];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 1 / 调用 / 计算 / 写入 | <code>regElem-&gt;dnOutputs[i] = regInfo-&gt;recvbuffs[j];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 43 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_MAX_DIRECT_ARITY；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 44 | 1 / 准备局部状态 | <code>int peer = channel-&gt;collTree.up[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 1 / 条件判断 | <code>(peer == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 48 | 1 / 准备局部状态 | <code>int j = comm-&gt;rankToLocalRank[peer];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 1 / 条件判断 | <code>(j &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 3 / 调用 / 计算 / 写入 | <code>WARN("Invalid intra-node rank %d for peer %d", j, peer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 52 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 53 | 1 / 调用 / 计算 / 写入 | <code>regElem-&gt;upOutputs[i] = regInfo-&gt;recvbuffs[j];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 55 | 0 / 调用 / 计算 / 写入 | <code>work-&gt;elems[s].regUsed = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{

  if (type == ncclWorkTypeP2p) {
    memcpy(work->p2pElems+s, elem, sizeof(struct ncclWorkElemP2p));
    int nelems = 0;
    for (int i=0; i<NCCL_MAX_WORK_ELEMENTS_P2P; i++) {
      if (work->p2pElems[i].header.type) nelems = i+1;
    }

    int ngroups = 1;
    while (ngroups < nelems) ngroups *= 2;
    int nWarps = 1;
    while (nWarps*ngroups <= elem->header.nWarps/2) nWarps *= 2;

    for (int i=0; i<ngroups; i++) {
      work->p2pElems[i].ngroups = ngroups;
      work->p2pElems[i].warpStart =
        i*(NCCL_MAX_NTHREADS/WARP_SIZE)/ngroups;
      int extraWarp = nWarps >= 2 ? i%2 : 0;
      work->p2pElems[i].nWarps = nWarps + extraWarp;
    }
    return ncclSuccess;
  }

  memcpy(work->elems+s, elem, sizeof(struct ncclWorkElem));

  if (regInfo->nBuffs == 0) return ncclSuccess;

  // Copy registered buffer addresses into ncclWork
  struct ncclWorkElemReg* regElem = (struct ncclWorkElemReg*)(work->elems+s);
  // For CollNet
  for (int i=0; i<NCCL_MAX_DIRECT_ARITY; i++) {
    int peer = channel->collTree.down[i];
    if (peer == -1) break;
    // Get intra-node slot
    int j = comm->rankToLocalRank[peer];
    if (j < 0) {
      WARN("Invalid intra-node rank %d for peer %d", j, peer);
      return ncclInternalError;
    }
    // Input buffer of leaf peer
    regElem->dnInputs[i] = regInfo->sendbuffs[j];
    // Output buffer of leaf peer
    regElem->dnOutputs[i] = regInfo->recvbuffs[j];
  }
  for (int i=0; i<NCCL_MAX_DIRECT_ARITY; i++) {
    int peer = channel->collTree.up[i];
    if (peer == -1) break;
    int j = comm->rankToLocalRank[peer];
    if (j < 0) {
      WARN("Invalid intra-node rank %d for peer %d", j, peer);
      return ncclInternalError;
    }
    // Output buffer of root peer
    regElem->upOutputs[i] = regInfo->recvbuffs[j];
  }
  work->elems[s].regUsed = 1;
  return ncclSuccess;
}
```

</details>

<a id="f0418"></a>

## F0418：ncclEnqueueP2pKernel

用途：把已指定 channel 的 P2P 元素和 ProxyOp 装入实际工作 FIFO，可与兼容任务合并。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclWorkElemP2p* workElem = eqElem-&gt;work.p2pElems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclProxyOp* proxyOp = &eqElem-&gt;proxyOp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+proxyOp-&gt;channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int opIndex = (channel-&gt;workFifoTail-1+NCCL_MAX_OPS)%NCCL_MAX_OPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclWork* w = channel-&gt;workFifo+opIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int segment = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 条件判断 | <code>(channel-&gt;workCount)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>segment = getSegment(ncclWorkTypeP2p, workElem-&gt;subType, workElem-&gt;peer, w);</code> | 在已有 work 里寻找可放入当前类型/peer/方向任务的槽位，避免破坏固定收发分组布局。 |
| 10 | 0 / 条件判断 | <code>(segment == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getNextOp(channel, &w, NULL));</code> | 失败时可能提前return，成功才继续；等待 channel 工作 FIFO 某槽被 GPU 取走并标成 Unused，清零后分配给本批下一条 work。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>segment = workElem-&gt;subType == ncclWorkSubTypeRecv ? 0 : 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>w-&gt;header.type = ncclWorkTypeP2p;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_MAX_WORK_ELEMENTS_P2P；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>w-&gt;p2pElems[i].peer = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxySaveP2p(comm, proxyOp));</code> | 失败时可能提前return，成功才继续；按 P2P 任务方向和连接索引1保存代理需求，跳过 self 或无代理的本地路径。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(enqueueSegOp(ncclWorkTypeP2p, &eqElem-&gt;work, w, segment, &eqElem-&gt;buffRegInfo, channel, comm));</code> | 失败时可能提前return，成功才继续；把元素装进固定大小 work，对 P2P 计算各收发组 warp 范围，对 collective 复制对应描述。 |
| 20 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclWorkElemP2p* workElem = eqElem->work.p2pElems;
  struct ncclProxyOp* proxyOp = &eqElem->proxyOp;

  // Try to reuse last p2p operation if not full yet
  struct ncclChannel* channel = comm->channels+proxyOp->channelId;
  int opIndex = (channel->workFifoTail-1+NCCL_MAX_OPS)%NCCL_MAX_OPS;
  struct ncclWork* w = channel->workFifo+opIndex;
  int segment = -1;
  if (channel->workCount) {
    // Try to pack more segments into a single operation
    segment = getSegment(ncclWorkTypeP2p, workElem->subType, workElem->peer, w);
  }
  if (segment == -1) {
    NCCLCHECK(getNextOp(channel, &w, NULL));
    segment = workElem->subType == ncclWorkSubTypeRecv ? 0 : 1;
    // Initialize work as P2P, set peer=-1 to designate the p2p elem is not used.
    w->header.type = ncclWorkTypeP2p;
    for (int i=0; i<NCCL_MAX_WORK_ELEMENTS_P2P; i++) w->p2pElems[i].peer = -1;
  }
  //printf("%s to %d -> Channel %d OpCount %ld Segment %d\n", workElem->subType == ncclWorkSubTypeRecv ? "Recv" : "Send", proxyOp->root, channel->id, channel->workFifoTail-1, segment);

  // store work element into FIFO
  NCCLCHECK(ncclProxySaveP2p(comm, proxyOp));
  NCCLCHECK(enqueueSegOp(ncclWorkTypeP2p, &eqElem->work, w, segment, &eqElem->buffRegInfo, channel, comm));
  return ncclSuccess;
}
```

</details>

<a id="f0419"></a>

## F0419：ncclSetupP2pKernel

用途：为 P2P 块生成设备/Proxy 计划并更新发射所需的线程和 channel 范围。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclComm* comm = info-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclQueueElem* eqElem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(comm-&gt;enqueueInfo-&gt;elemList-&gt;getNewElem(&eqElem));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyComputeP2p(info, &eqElem-&gt;proxyOp));</code> | 失败时可能提前return，成功才继续；根据已切好的 P2P 块计算协议尺寸、step 数和 channel/peer 标签，使 CPU/GPU 计划一致。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(computeP2pWorkElem(info, eqElem-&gt;work.p2pElems));</code> | 失败时可能提前return，成功才继续；把单个已分块 P2P 任务转换成设备元素，记录字节长度、peer、方向与协议 chunkSize。 |
| 6 | 0 / 准备局部状态 | <code>int channelId = info-&gt;channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>struct cudaLaunchParams* params = comm-&gt;myParams;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;gridDim.x = std::max&lt;unsigned&gt;(params-&gt;gridDim.x, channelId+1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>params-&gt;blockDim.x = std::max&lt;unsigned&gt;(params-&gt;blockDim.x, eqElem-&gt;work.header.nWarps*WARP_SIZE);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;enqueueInfo-&gt;maxChannels = params-&gt;gridDim.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 条件判断 | <code>(params-&gt;func == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>params-&gt;func = ncclKerns[eqElem-&gt;work.header.funcIndex];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;args.header.type = ncclWorkTypeUnused;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclComm* comm = info->comm;
  // Compute cuda kernel arg and proxy arg templates
  struct ncclQueueElem* eqElem;
  NCCLCHECK(comm->enqueueInfo->elemList->getNewElem(&eqElem));
  // The proxy code will set and tune the send/recv chunk size, make sure to run it first.
  NCCLCHECK(ncclProxyComputeP2p(info, &eqElem->proxyOp));
  NCCLCHECK(computeP2pWorkElem(info, eqElem->work.p2pElems));
  // Compute grid size
  int channelId = info->channelId;
  struct cudaLaunchParams* params = comm->myParams;
  params->gridDim.x = std::max<unsigned>(params->gridDim.x, channelId+1);
  params->blockDim.x = std::max<unsigned>(params->blockDim.x, eqElem->work.header.nWarps*WARP_SIZE);
  comm->enqueueInfo->maxChannels = params->gridDim.x;  // params may be varied by a second graph hence we need to capture it here

  // Record the first kernel to launch
  // Just for CUDA kernel to know this is a P2P operation
  // The CUDA kernel does not use the inlined first work element as fastpath argument
  if (params->func == NULL) {
    params->func = ncclKerns[eqElem->work.header.funcIndex];
    comm->args.header.type = ncclWorkTypeUnused;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0420"></a>

## F0420：ncclEnqueueCollKernel

用途：把 collective 模板展开为多个 bid，选择 channel、保存对应 Proxy 工作并装填 FIFO。

- 对bid逐个选择实际channelId；聚合最短队列/轮转策略会使bid与物理channelId不同。
- 先记录该channel对应的Proxy工作，再尝试把兼容funcIndex/线程数的元素装进已有work，装不下才取新FIFO项。
- 全部bid共享本次collective编号；循环结束后collOpCount才递增。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclWork* work = &eqElem-&gt;work;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclWorkElem* elem = work-&gt;elems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclProxyOp* proxyOp = &eqElem-&gt;proxyOp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int nChannels = elem-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>size_t channelSize = elem-&gt;count*ncclTypeSize(proxyOp-&gt;dtype)/elem-&gt;nChannels;</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 6 | 0 / 准备局部状态 | <code>enum ncclWorkElemType workElemType = proxyOp-&gt;redOp == ncclNumOps ? ncclWorkTypeColl : ncclWorkTypeRegColl;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 循环 | <code>初始 int bid=0;；继续条件 bid&lt;nChannels；每轮更新 bid++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 准备局部状态 | <code>int channelId = getNextChannel(comm, aggMode);</code> | 按轮转或最短队列策略为当前 collective 分片选择实际 channelId。 |
| 9 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+channelId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;channelId = channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>proxyOp-&gt;opCount = comm-&gt;collOpCount;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 1 / 条件判断 | <code>(proxyOp-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxySaveColl(comm, proxyOp, comm-&gt;nRanks));</code> | 失败时可能提前return，成功才继续；按 Ring/Tree/Pipeline/CollNet 模式展开必要邻居与方向，给相应连接保存代理工作。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>elem-&gt;bid = bid % nChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 1 / 准备局部状态 | <code>struct ncclWork* w = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 1 / 准备局部状态 | <code>int segment = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 1 / 条件判断 | <code>(aggMode && channel-&gt;workCount)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 准备局部状态 | <code>int opIndex = (channel-&gt;workFifoTail-1+NCCL_MAX_OPS)%NCCL_MAX_OPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>w = channel-&gt;workFifo+opIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 3 / 条件判断 | <code>(w-&gt;header.funcIndex == work-&gt;header.funcIndex && w-&gt;header.nWarps == work-&gt;header.nWarps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 5 / 调用 / 计算 / 写入 | <code>segment = getSegment(workElemType, ncclWorkSubTypeUnused, 0, w);</code> | 在已有 work 里寻找可放入当前类型/peer/方向任务的槽位，避免破坏固定收发分组布局。 |
| 25 | 1 / 条件判断 | <code>(segment == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getNextOp(channel, &w, NULL));</code> | 失败时可能提前return，成功才继续；等待 channel 工作 FIFO 某槽被 GPU 取走并标成 Unused，清零后分配给本批下一条 work。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>segment = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(enqueueSegOp(workElemType, work, w, segment, &eqElem-&gt;buffRegInfo, channel, comm));</code> | 失败时可能提前return，成功才继续；把元素装进固定大小 work，对 P2P 计算各收发组 warp 范围，对 collective 复制对应描述。 |
| 30 | 1 / 调用 / 计算 / 写入 | <code>channel-&gt;totalSize += channelSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;collOpCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclWork* work = &eqElem->work;
  struct ncclWorkElem* elem = work->elems;
  struct ncclProxyOp* proxyOp = &eqElem->proxyOp;

  int nChannels = elem->nChannels;
  size_t channelSize = elem->count*ncclTypeSize(proxyOp->dtype)/elem->nChannels;
  enum ncclWorkElemType workElemType = proxyOp->redOp == ncclNumOps ? ncclWorkTypeColl : ncclWorkTypeRegColl;  // redOp is only set when using CollNet

  for (int bid=0; bid<nChannels; bid++) {
    int channelId = getNextChannel(comm, aggMode);
    struct ncclChannel* channel = comm->channels+channelId;

    // Proxy
    proxyOp->channelId = channelId;
    proxyOp->opCount = comm->collOpCount;
    if (proxyOp->nsteps) NCCLCHECK(ncclProxySaveColl(comm, proxyOp, comm->nRanks));

    elem->bid = bid % nChannels;
    struct ncclWork* w = NULL;
    int segment = -1;
    if (aggMode && channel->workCount) {
      // Try to pack more segments into a single operation
      int opIndex = (channel->workFifoTail-1+NCCL_MAX_OPS)%NCCL_MAX_OPS;
      w = channel->workFifo+opIndex;
      // All elems in work must have same (funcIndex,nThreads),
      // see "src/collectives/device/common.h"
      if (w->header.funcIndex == work->header.funcIndex &&
          w->header.nWarps == work->header.nWarps) {
        segment = getSegment(workElemType, ncclWorkSubTypeUnused, 0, w);
      }
    }
    if (segment == -1) {
      NCCLCHECK(getNextOp(channel, &w, NULL));
      segment = 0;
    }

    // store work element into FIFO
    NCCLCHECK(enqueueSegOp(workElemType, work, w, segment, &eqElem->buffRegInfo, channel, comm));
    channel->totalSize += channelSize;
  }
  comm->collOpCount++;
  return ncclSuccess;
}
```

</details>

<a id="f0421"></a>

## F0421：ncclEnqueueHostSetup

用途：遍历提交批次并执行真正的 FIFO 装填、launch 整理和 Proxy 发布；图 replay 也复用此入口。

- 遍历QueueInfo元素，按P2P或collective分派装入实际channel FIFO。
- 全部元素装填后setupLaunch补齐终止状态，再LaunchProxy发布任务；该函数本身不直接调用CUDA通信kernel。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>ncclResult_t ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclQueueInfo* eqInfo = (struct ncclQueueInfo*)arg;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>ncclComm_t comm = eqInfo-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>int aggMode = eqInfo-&gt;elemList-&gt;count() &gt; 1 ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclQueueElem* eqElem = eqInfo-&gt;elemList-&gt;begin();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 循环 | <code>初始 ；继续条件 (eqElem != NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 条件判断 | <code>(eqElem-&gt;work.header.funcIndex == FUNC_INDEX_P2P)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclEnqueueP2pKernel(comm, eqElem), ret, cb_end);</code> | 失败时保存错误并跳到清理标签，成功才继续；把已指定 channel 的 P2P 元素和 ProxyOp 装入实际工作 FIFO，可与兼容任务合并。 |
| 11 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclEnqueueCollKernel(comm, eqElem, aggMode), ret, cb_end);</code> | 失败时保存错误并跳到清理标签，成功才继续；把 collective 模板展开为多个 bid，选择 channel、保存对应 Proxy 工作并装填 FIFO。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>eqElem = eqInfo-&gt;elemList-&gt;getNext();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(setupLaunch(eqInfo, USING_CUDA_GRAPH), ret, cb_end);</code> | 失败时保存错误并跳到清理标签，成功才继续；确定本批 block/channel 范围，给空通道补 no-op，标记各通道 isLast，并准备可选 GDRCopy FIFO。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclLaunchProxy(eqInfo), ret, cb_end);</code> | 失败时保存错误并跳到清理标签，成功才继续；发布 HostSetup 已保存的 Proxy 任务并重置本批通道统计；普通调用点在实际 kernel 发射之前。 |
| 17 | 0 / 跳转标签 | <code>cb_end</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 18 | 1 / 条件判断 | <code>(ret != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>WARN("Failure in host setup : %s", ncclGetErrorString(ret));</code> | 把 NCCL 结果枚举映射成稳定可读字符串，不改变 communicator 的执行状态。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>eqInfo-&gt;ret = ret;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  ncclResult_t ret;
  // All work for current launch has been captured in Queue Info
  struct ncclQueueInfo* eqInfo = (struct ncclQueueInfo*)arg;
  ncclComm_t comm = eqInfo->comm;
  int aggMode = eqInfo->elemList->count() > 1 ? 1 : 0;

  // Iterate through the element list
  struct ncclQueueElem* eqElem = eqInfo->elemList->begin();
  while (eqElem != NULL) {
    if (eqElem->work.header.funcIndex == FUNC_INDEX_P2P) {
      NCCLCHECKGOTO(ncclEnqueueP2pKernel(comm, eqElem), ret, cb_end);
    } else {
      NCCLCHECKGOTO(ncclEnqueueCollKernel(comm, eqElem, aggMode), ret, cb_end);
    }
    eqElem = eqInfo->elemList->getNext();
  }

  NCCLCHECKGOTO(setupLaunch(eqInfo, USING_CUDA_GRAPH), ret, cb_end);
  NCCLCHECKGOTO(ncclLaunchProxy(eqInfo), ret, cb_end);

cb_end:
  if (ret != ncclSuccess) {
    WARN("Failure in host setup : %s", ncclGetErrorString(ret));
  }
  eqInfo->ret = ret;
}
```

</details>

<a id="f0422"></a>

## F0422：graphHelperFunc

用途：在允许调用 CUDA 的独立线程中关闭图注册缓冲的 IPC 映射，避开图销毁回调的限制。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclGraphHelperResources* res = (struct ncclGraphHelperResources*)args;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(res == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("CUDA Graph helper resource is null");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 准备局部状态 | <code>int dev = res-&gt;comm-&gt;cudaDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECKIGNORE(cudaSetDevice(dev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_COLL, "CUDA Graph helper thread created for device %d", dev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 准备局部状态 | <code>volatile enum helperThreadState* state = &res-&gt;threadState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>volatile int* ipcTail = &res-&gt;ipcTail;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 循环 | <code>初始 ；继续条件 (1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 准备局部状态 | <code>int ipcTailMark = *ipcTail;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 准备局部状态 | <code>int ipcCount = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 1 / 循环 | <code>初始 ；继续条件 (res-&gt;ipcHead != ipcTailMark)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 15 | 2 / 条件判断 | <code>(res-&gt;ipcBases[res-&gt;ipcHead] != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECKIGNORE(cudaIpcCloseMemHandle(res-&gt;ipcBases[res-&gt;ipcHead]));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>res-&gt;ipcBases[res-&gt;ipcHead] = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>res-&gt;ipcHead = (res-&gt;ipcHead+1)%NCCL_IPC_POOL_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>ipcCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_COLL, "CUDA Graph helper thread closed %d IPC handles", ipcCount);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 1 / 循环 | <code>初始 ；继续条件 (res-&gt;ipcHead == *ipcTail && *state != ThreadStop)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>pthread_cond_wait(&res-&gt;threadCond, &res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 1 / 条件判断 | <code>(*state == ThreadStop)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_COLL, "CUDA Graph helper thread for device %d returning", dev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 3 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 32 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclGraphHelperResources* res = (struct ncclGraphHelperResources*)args;
  if (res == NULL) {
    WARN("CUDA Graph helper resource is null");
    return NULL;
  }
  int dev = res->comm->cudaDev;
  CUDACHECKIGNORE(cudaSetDevice(dev));
  INFO(NCCL_COLL, "CUDA Graph helper thread created for device %d", dev);

  volatile enum helperThreadState* state = &res->threadState;
  volatile int* ipcTail = &res->ipcTail;
  while (1) {
    // Last IPC entry enqueue so far
    int ipcTailMark = *ipcTail;
    int ipcCount = 0;
    // Close IPC till the last entry
    while (res->ipcHead != ipcTailMark) {
      if (res->ipcBases[res->ipcHead] != NULL)
        CUDACHECKIGNORE(cudaIpcCloseMemHandle(res->ipcBases[res->ipcHead]));
      res->ipcBases[res->ipcHead] = NULL;
      res->ipcHead = (res->ipcHead+1)%NCCL_IPC_POOL_SIZE;
      ipcCount++;
    }
    TRACE(NCCL_COLL, "CUDA Graph helper thread closed %d IPC handles", ipcCount);
    pthread_mutex_lock(&res->threadLock);
    // Check for exit signal
    while (res->ipcHead == *ipcTail && *state != ThreadStop) {
      pthread_cond_wait(&res->threadCond, &res->threadLock);
    }
    pthread_mutex_unlock(&res->threadLock);
    if (*state == ThreadStop) {
      INFO(NCCL_COLL, "CUDA Graph helper thread for device %d returning", dev);
      return NULL;
    }
  }
}
```

</details>

<a id="f0423"></a>

## F0423：ncclGetCudaGraph

用途：检查当前 stream 是否捕获、图标识和驱动能力，维护图模式队列及生命周期状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;usingCudaGraph = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 11030</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 3 | 1 / 准备局部状态 | <code>cudaStreamCaptureStatus captureStatus;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 准备局部状态 | <code>unsigned long long cudaGraphId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 1 / 准备局部状态 | <code>ncclResult_t ret = ncclSuccess;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 条件判断 | <code>(comm-&gt;driverVersion &lt; 11030)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 控制转移 | <code>goto enh_compat_end;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaStreamGetCaptureInfo_v2(comm-&gt;userStream, &captureStatus, &cudaGraphId, graph, NULL, NULL), ret, enh_compat_end);</code> | 失败时保存错误并跳到清理标签，成功才继续；提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。 |
| 10 | 1 / 条件判断 | <code>(captureStatus == cudaStreamCaptureStatusActive)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 3 / 条件判断 | <code>(cudaGraphId != comm-&gt;lastCudaGraphId)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 5 / 调用 / 计算 / 写入 | <code>INFO(NCCL_COLL, "stream is being captured by a new graph, id %llu", cudaGraphId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;lastCudaGraphId = cudaGraphId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;lastSetupNode = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 3 / 条件判断 | <code>(comm-&gt;launchMode == ncclComm::GROUP)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;launchMode = ncclComm::GROUP_GRAPH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>comm-&gt;usingCudaGraph = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 3 / 条件判断 | <code>((!comm-&gt;graphHelperThread) && comm-&gt;graphRegister == 1 && comm-&gt;disableGraphHelper == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 5 / 调用 / 计算 / 写入 | <code>pthread_mutex_init(&comm-&gt;graphHelperResources-&gt;threadLock, NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 5 / 调用 / 计算 / 写入 | <code>pthread_cond_init(&comm-&gt;graphHelperResources-&gt;threadCond, NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 5 / 调用 / 计算 / 写入 | <code>comm-&gt;graphHelperResources-&gt;threadState = ThreadStart;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 5 / 调用 / 计算 / 写入 | <code>pthread_create(&comm-&gt;graphHelperThread, NULL, graphHelperFunc, comm-&gt;graphHelperResources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 5 / 调用 / 计算 / 写入 | <code>ncclSetThreadName(comm-&gt;graphHelperThread, "NCCL GrHelper%2d", comm-&gt;cudaDev);</code> | 为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。 |
| 28 | 1 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamIsCapturing(comm-&gt;userStream, &captureStatus));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 30 | 1 / 条件判断 | <code>(captureStatus != cudaStreamCaptureStatusNone)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 3 / 调用 / 计算 / 写入 | <code>WARN("The installed CUDA driver is older than the minimum version (R465) required for NCCL's CUDA Graphs support");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 3 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 34 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  comm->usingCudaGraph = 0;
  // Feature requires CUDA 11.3/R465 or above
#if CUDART_VERSION >= 11030
  cudaStreamCaptureStatus captureStatus;
  unsigned long long cudaGraphId;
  ncclResult_t ret = ncclSuccess;
  if (comm->driverVersion < 11030) {
    // Runtime driver version older than compiler version
    // Enhanced compat fallback
    goto enh_compat_end;
  }
  // Get CUDA Graph handle
  CUDACHECKGOTO(cudaStreamGetCaptureInfo_v2(comm->userStream, &captureStatus, &cudaGraphId, graph, NULL, NULL), ret, enh_compat_end);
  if (captureStatus == cudaStreamCaptureStatusActive) {
    if (cudaGraphId != comm->lastCudaGraphId) {
      INFO(NCCL_COLL, "stream is being captured by a new graph, id %llu", cudaGraphId);
      // We are in a new graph, hence need to forget the last setup node so that
      // the first setup node in the new graph will not have a dependency
      comm->lastCudaGraphId = cudaGraphId;
      comm->lastSetupNode = NULL;
    }
    if (comm->launchMode == ncclComm::GROUP) comm->launchMode = ncclComm::GROUP_GRAPH;
    comm->usingCudaGraph = 1;

    // Create helper thread that closes IPC handles during graph destruction
    // Only create this thread when buffer registration is enabled
    if ((!comm->graphHelperThread) && comm->graphRegister == 1 && comm->disableGraphHelper == 0) {
      pthread_mutex_init(&comm->graphHelperResources->threadLock, NULL);
      // Init signaling method between Graph destroy function and helper thread
      pthread_cond_init(&comm->graphHelperResources->threadCond, NULL);
      // Set state
      comm->graphHelperResources->threadState = ThreadStart;
      // Create thread
      pthread_create(&comm->graphHelperThread, NULL, graphHelperFunc, comm->graphHelperResources);
      // Name thread
      ncclSetThreadName(comm->graphHelperThread, "NCCL GrHelper%2d", comm->cudaDev);
    }
  }
  return ncclSuccess;

enh_compat_end: // Enhanced compat fallback
  (void)ret;
  CUDACHECK(cudaStreamIsCapturing(comm->userStream, &captureStatus));
  if (captureStatus != cudaStreamCaptureStatusNone) {
    WARN("The installed CUDA driver is older than the minimum version (R465) required for NCCL's CUDA Graphs support");
    return ncclInvalidUsage;
  }
  // If we are not in capture mode, we can ignore the driver being lower
#endif
  return ncclSuccess;
}
```

</details>

<a id="f0424"></a>

## F0424：ncclCudaGraphHostSetup

用途：给 CUDA Graph 添加/关联 HostSetup 节点及持有对象，确保 replay 时重新发布 GPU/Proxy 工作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 11030</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclQueueInfo* eqInfo = comm-&gt;enqueueInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 准备局部状态 | <code>cudaUserObject_t object;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaUserObjectCreate(&object, eqInfo, ncclDestroyQueueInfo, 1 , cudaUserObjectNoDestructorSync));</code> | 失败时可能提前return，成功才继续；提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGraphRetainUserObject(graph, object, 1, cudaGraphUserObjectMove));</code> | 失败时可能提前return，成功才继续；提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。 |
| 6 | 1 / 准备局部状态 | <code>cudaHostFn_t fn = ncclEnqueueHostSetup&lt;1&gt;;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 准备局部状态 | <code>cudaGraphNode_t setupNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 准备局部状态 | <code>cudaHostNodeParams setupNodeParams = {fn, eqInfo};</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 1 / 准备局部状态 | <code>int numDependencies = comm-&gt;lastSetupNode == NULL ? 0 : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGraphAddHostNode(&setupNode, graph, &comm-&gt;lastSetupNode, numDependencies, &setupNodeParams));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamUpdateCaptureDependencies(comm-&gt;userStream, &setupNode, 1, cudaStreamAddCaptureDependencies));</code> | 失败时可能提前return，成功才继续；提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;lastSetupNode = setupNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 1 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>WARN("NCCL does not support this CUDA version for CUDA graph feature");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if CUDART_VERSION >= 11030
  struct ncclQueueInfo* eqInfo = comm->enqueueInfo;
  // Create a CUDA object to wrap around the argument space
  // which CUDA graph would manage lifetime of
  cudaUserObject_t object;
  CUDACHECK(cudaUserObjectCreate(&object, eqInfo, ncclDestroyQueueInfo, 1/*initialRefcount*/, cudaUserObjectNoDestructorSync));
  // Hand over ownership to CUDA Graph
  CUDACHECK(cudaGraphRetainUserObject(graph, object, 1, cudaGraphUserObjectMove));

  cudaHostFn_t fn = ncclEnqueueHostSetup<1>;
  // Add a CPU node to the graph
  cudaGraphNode_t setupNode;
  // Function + parameter space for that function (i.e. enqueue info)
  cudaHostNodeParams setupNodeParams = {fn, eqInfo};
  int numDependencies = comm->lastSetupNode == NULL ? 0 : 1;
  CUDACHECK(cudaGraphAddHostNode(&setupNode, graph, &comm->lastSetupNode, numDependencies, &setupNodeParams));
  // Create dependency from last setup node in the same graph
  CUDACHECK(cudaStreamUpdateCaptureDependencies(comm->userStream, &setupNode, 1, cudaStreamAddCaptureDependencies));
  comm->lastSetupNode = setupNode;
  return ncclSuccess;
#else
  WARN("NCCL does not support this CUDA version for CUDA graph feature");
  return ncclInternalError;
#endif
}
```

</details>

<a id="f0425"></a>

## F0425：hostToDevRedOp

用途：把公开归约句柄转换成设备操作和标量位模式，处理 Avg、用户 PreMulSum 与句柄有效性。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>union { int8_t i8; uint8_t u8; int32_t i32; uint32_t u32; int64_t i64; uint64_t u64; half f16; #if defined(__CUDA_BF16_TYPES_EXIST__) __nv_bfloat16 bf16; #endif float f32; double f64; void *ptr; }</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u64 = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>opFull-&gt;scalarArgIsPtr = false;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 多路分派 | <code>(int(op))</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 5 | 1 / case / default | <code>ncclSum</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>opFull-&gt;op = ncclDevSum;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 8 | 1 / case / default | <code>ncclProd</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>opFull-&gt;op = ncclDevProd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 11 | 1 / case / default | <code>ncclMax</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>opFull-&gt;op = ncclDevMax;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 14 | 1 / case / default | <code>ncclMin</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>opFull-&gt;op = ncclDevMin;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 17 | 1 / case / default | <code>ncclAvg</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 18 | 2 / 多路分派 | <code>((int)datatype)</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 19 | 3 / case / default | <code>ncclInt8</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 20 | 3 / case / default | <code>ncclInt32</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 21 | 3 / case / default | <code>ncclInt64</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 22 | 3 / case / default | <code>ncclUint8</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 23 | 3 / case / default | <code>ncclUint32</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 24 | 3 / case / default | <code>ncclUint64</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 25 | 4 / 调用 / 计算 / 写入 | <code>opFull-&gt;op = ncclDevSumPostDiv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>u64 = comm-&gt;nRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 4 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 28 | 3 / case / default | <code>ncclFloat16</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>opFull-&gt;op = ncclDevPreMulSum;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 4 / 调用 / 计算 / 写入 | <code>f16 = __float2half(float(1.0/comm-&gt;nRanks));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 4 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 32 | 3 / 编译期分支/指令 | <code>#if defined(__CUDA_BF16_TYPES_EXIST__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 33 | 3 / case / default | <code>ncclFloat32</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 34 | 4 / 调用 / 计算 / 写入 | <code>opFull-&gt;op = ncclDevPreMulSum;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 4 / 调用 / 计算 / 写入 | <code>f32 = float(1.0/comm-&gt;nRanks);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 4 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 37 | 3 / case / default | <code>ncclFloat64</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>opFull-&gt;op = ncclDevPreMulSum;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 4 / 调用 / 计算 / 写入 | <code>f64 = 1.0/comm-&gt;nRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 4 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 41 | 2 / 调用 / 计算 / 写入 | <code>opFull-&gt;scalarArgIsPtr = false;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 2 / 调用 / 计算 / 写入 | <code>opFull-&gt;scalarArg = u64;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 44 | 1 / case / default | <code>default</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 45 | 2 / 准备局部状态 | <code>int ix = int(ncclUserRedOpMangle(comm, op)) - int(ncclNumOps);</code> | 用 communicator 地址导出的可逆异或扰动用户操作句柄，减少把别的 comm 句柄误用于本 comm 的风险。 |
| 46 | 2 / 准备局部状态 | <code>ncclUserRedOp *user = &comm-&gt;userRedOps[ix];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 47 | 2 / 条件判断 | <code>(datatype != user-&gt;datatype)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 4 / 调用 / 计算 / 写入 | <code>WARN("Data type supplied to user-created ncclRedOp_t does not match type " "given to reduction operation");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 50 | 4 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 51 | 2 / 调用 / 计算 / 写入 | <code>*opFull = user-&gt;opFull;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 53 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  union {
    int8_t i8;
    uint8_t u8;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    half f16;
    #if defined(__CUDA_BF16_TYPES_EXIST__)
      __nv_bfloat16 bf16;
    #endif
    float f32;
    double f64;
    void *ptr;
  };
  u64 = 0;
  opFull->scalarArgIsPtr = false;
  switch (int(op)) {
  case ncclSum:  opFull->op = ncclDevSum;  break;
  case ncclProd: opFull->op = ncclDevProd; break;
  case ncclMax:  opFull->op = ncclDevMax;  break;
  case ncclMin:  opFull->op = ncclDevMin;  break;
  case ncclAvg:
    switch ((int)datatype) {
    case ncclInt8:  case ncclInt32:  case ncclInt64:
    case ncclUint8: case ncclUint32: case ncclUint64:
      opFull->op = ncclDevSumPostDiv;
      u64 = comm->nRanks;
      break;
    case ncclFloat16:
      opFull->op = ncclDevPreMulSum;
      f16 = __float2half(float(1.0/comm->nRanks)); // __double2half not supported pre CUDA 11.x
      break;
    #if defined(__CUDA_BF16_TYPES_EXIST__)
    case ncclBfloat16:
      opFull->op = ncclDevPreMulSum;
      bf16 = __float2bfloat16(float(1.0/comm->nRanks));
      break;
    #endif
    case ncclFloat32:
      opFull->op = ncclDevPreMulSum;
      f32 = float(1.0/comm->nRanks);
      break;
    case ncclFloat64:
      opFull->op = ncclDevPreMulSum;
      f64 = 1.0/comm->nRanks;
      break;
    }
    opFull->scalarArgIsPtr = false;
    opFull->scalarArg = u64;
    break;
  default: // user created
    int ix = int(ncclUserRedOpMangle(comm, op)) - int(ncclNumOps);
    ncclUserRedOp *user = &comm->userRedOps[ix];
    if (datatype != user->datatype) {
      WARN("Data type supplied to user-created ncclRedOp_t does not match type "
           "given to reduction operation");
      return ncclInvalidArgument;
    }
    *opFull = user->opFull;
    break;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0426"></a>

## F0426：ncclEnqueueCheck

用途：通信 API 的统一检查与分流入口，组内保存任务，普通路径准备队列并发布/发射执行。

- 先完成参数检查和归约句柄复制，防止组内等待期间用户销毁句柄影响任务。
- Group内只保存collective或P2P描述；普通路径立即生成计划、执行HostSetup，随后发射kernel并排事件。
- 普通HostSetup已经发布Proxy工作；不要按邻近旧注释把真实调用先后倒置。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclResult_t ret = ncclSuccess;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>bool isAsync = ncclAsyncMode();</code> | 查询本线程是否处于 Group 嵌套内部，决定 API 立即准备还是延迟到最外层 GroupEnd。 |
| 3 | 0 / 准备局部状态 | <code>int savedDev = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(PtrCheck(info-&gt;comm, info-&gt;opName, "comm"));</code> | 失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。 |
| 5 | 0 / 条件判断 | <code>(isAsync && info-&gt;comm-&gt;checkPointers)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaGetDevice(&savedDev), ret, end);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaSetDevice(info-&gt;comm-&gt;cudaDev), ret, end);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ArgsCheck(info), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；验证 root/peer、类型、归约句柄及可选缓冲指针，并归一化复制型操作尺寸和总字节数。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(hostToDevRedOp(&info-&gt;opFull, info-&gt;op, info-&gt;datatype, info-&gt;comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；把公开归约句柄转换成设备操作和标量位模式，处理 Avg、用户 PreMulSum 与句柄有效性。 |
| 11 | 0 / 条件判断 | <code>(isAsync)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclAsyncColl(info-&gt;comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；把 communicator 加入本批 Group 提交集合，重复遇到相同 comm 时避免多次登记。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(checkSetStream(info), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；保存本批使用的 CUDA stream，并检查 Group 内同一 communicator 的 stream 使用契约。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_COLL,"%s: opCount %lx sendbuff %p recvbuff %p count %zi datatype %d op %d root %d comm %p [nranks=%d] stream %p", info-&gt;opName, info-&gt;comm-&gt;opCount, info-&gt;sendbuff, info-&gt;recvbuff, info-&gt;count, info-&gt;datatype, info-&gt;op, info-&gt;root, info-&gt;comm, info-&gt;comm-&gt;nRanks, info-&gt;stream);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 2 / 条件判断 | <code>(info-&gt;coll == ncclFuncSend &#124;&#124; info-&gt;coll == ncclFuncRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSaveP2p(info), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；按 peer 保存 P2P 消息和字节数，并标记尚未建立的 channel/方向/索引1连接需求。 |
| 19 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSaveAsyncColl(info), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；复制当前 collective 的主机中间描述到可延迟提交列表，保留 GroupEnd 计算所需参数。 |
| 21 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(checkSetStream(info), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；保存本批使用的 CUDA stream，并检查 Group 内同一 communicator 的 stream 使用契约。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_COLL,"%s: opCount %lx sendbuff %p recvbuff %p count %zi datatype %d op %d root %d comm %p [nranks=%d] stream %p", info-&gt;opName, info-&gt;comm-&gt;opCount, info-&gt;sendbuff, info-&gt;recvbuff, info-&gt;count, info-&gt;datatype, info-&gt;op, info-&gt;root, info-&gt;comm, info-&gt;comm-&gt;nRanks, info-&gt;stream);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 2 / 准备局部状态 | <code>cudaGraph_t graph;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 2 / 准备局部状态 | <code>ncclComm_t comm = info-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclGetCudaGraph(comm, &graph), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；检查当前 stream 是否捕获、图标识和驱动能力，维护图模式队列及生命周期状态。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSetupCollKernel(info), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；创建 collective 排队模板、计算 GPU/Proxy 参数、更新 grid/block 配置并选择入口 kernel。 |
| 28 | 2 / 条件判断 | <code>(comm-&gt;usingCudaGraph)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclCudaGraphHostSetup(comm, graph), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；给 CUDA Graph 添加/关联 HostSetup 节点及持有对象，确保 replay 时重新发布 GPU/Proxy 工作。 |
| 31 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>ncclEnqueueHostSetup&lt;0&gt;(comm-&gt;enqueueInfo);</code> | 遍历提交批次并执行真正的 FIFO 装填、launch 整理和 Proxy 发布；图 replay 也复用此入口。 |
| 33 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(comm-&gt;enqueueInfo-&gt;ret, ret, end);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclLaunchBarrier(comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；建立用户 stream 与内部 stream 依赖，并在 GROUP 发射模式协调同进程 GPU 的启动。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclLaunchKernel(comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；PARALLEL 模式真正调用 CUDA 发射，GROUP 模式等待已由最后线程执行的统一发射完成。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclRecordEvents(comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；在执行 stream 记录完成事件并建立必要的用户 stream 依赖；record 本身不代表事件已经完成。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclLaunchReset(comm), ret, end);</code> | 失败时保存错误并跳到清理标签，成功才继续；清理本批发射参数和入队计数，为后续提交复用；图路径按持久 QueueInfo 生命周期处理。 |
| 38 | 0 / 跳转标签 | <code>end</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 39 | 1 / 条件判断 | <code>(isAsync && savedDev != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 3 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaSetDevice(savedDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 42 | 0 / 条件判断 | <code>(isAsync)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>ncclAsyncErrCheck(ret);</code> | 把组内调用错误记录到线程局部 Group 状态，供统一清理和返回。 |
| 45 | 0 / 函数返回 | <code>return ret;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclResult_t ret = ncclSuccess;
  bool isAsync = ncclAsyncMode();
  int savedDev = -1;
  // Check arguments
  NCCLCHECK(PtrCheck(info->comm, info->opName, "comm"));
  if (isAsync && info->comm->checkPointers) {
    CUDACHECKGOTO(cudaGetDevice(&savedDev), ret, end);
    CUDACHECKGOTO(cudaSetDevice(info->comm->cudaDev), ret, end);
  }
  NCCLCHECKGOTO(ArgsCheck(info), ret, end);

  // Copy reduction op state from op handle into info struct here since the
  // op handle may be destroyed before ncclGroupEnd().
  NCCLCHECKGOTO(hostToDevRedOp(&info->opFull, info->op, info->datatype, info->comm), ret, end);

  // Launch asynchronously if needed
  if (isAsync) {
    // Always register comm even in case of error to make sure ncclGroupEnd
    // cleans it up.
    NCCLCHECKGOTO(ncclAsyncColl(info->comm), ret, end);
    NCCLCHECKGOTO(checkSetStream(info), ret, end);

    INFO(NCCL_COLL,"%s: opCount %lx sendbuff %p recvbuff %p count %zi datatype %d op %d root %d comm %p [nranks=%d] stream %p",
        info->opName, info->comm->opCount, info->sendbuff, info->recvbuff, info->count,
        info->datatype, info->op, info->root, info->comm, info->comm->nRanks, info->stream);

    if (info->coll == ncclFuncSend || info->coll == ncclFuncRecv) { //p2p stored separately
      NCCLCHECKGOTO(ncclSaveP2p(info), ret, end);
    } else {
      NCCLCHECKGOTO(ncclSaveAsyncColl(info), ret, end);
    }
  } else {
    NCCLCHECKGOTO(checkSetStream(info), ret, end);

    INFO(NCCL_COLL,"%s: opCount %lx sendbuff %p recvbuff %p count %zi datatype %d op %d root %d comm %p [nranks=%d] stream %p",
        info->opName, info->comm->opCount, info->sendbuff, info->recvbuff, info->count,
        info->datatype, info->op, info->root, info->comm, info->comm->nRanks, info->stream);

    // Check whether we are in cuda graph mode
    cudaGraph_t graph;
    ncclComm_t comm = info->comm;
    NCCLCHECKGOTO(ncclGetCudaGraph(comm, &graph), ret, end);

    // Common part between graph mode and non-graph mode
    NCCLCHECKGOTO(ncclSetupCollKernel(info), ret, end);

    // Host setup
    if (comm->usingCudaGraph) {
      NCCLCHECKGOTO(ncclCudaGraphHostSetup(comm, graph), ret, end);
    } else {
      ncclEnqueueHostSetup<0>(comm->enqueueInfo);
      NCCLCHECKGOTO(comm->enqueueInfo->ret, ret, end);
    }

    // Common part between graph mode and non-graph mode
    NCCLCHECKGOTO(ncclLaunchBarrier(comm), ret, end);
    NCCLCHECKGOTO(ncclLaunchKernel(comm), ret, end);
    NCCLCHECKGOTO(ncclRecordEvents(comm), ret, end);
    NCCLCHECKGOTO(ncclLaunchReset(comm), ret, end);
  }
end:
  if (isAsync && savedDev != -1) CUDACHECK(cudaSetDevice(savedDev));
  if (isAsync) ncclAsyncErrCheck(ret);
  return ret;
}
```

</details>

<a id="f0427"></a>

## F0427：ncclRedOpCreatePreMulSum

用途：在 communicator 中分配并保存先乘标量再求和的操作句柄，按 residence 保存值或设备指针。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(comm-&gt;userRedOpFreeHead == comm-&gt;userRedOpCapacity)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>int cap = 2*comm-&gt;userRedOpCapacity;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 条件判断 | <code>(cap &lt; 4)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 调用 / 计算 / 写入 | <code>cap = 4;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 准备局部状态 | <code>ncclUserRedOp *ops = new ncclUserRedOp[cap];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>std::memcpy(ops, comm-&gt;userRedOps, comm-&gt;userRedOpCapacity*sizeof(ncclUserRedOp));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 2 / 循环 | <code>初始 int ix=comm-&gt;userRedOpCapacity;；继续条件 ix &lt; cap；每轮更新 ix++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>ops[ix].freeNext = ix + 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>delete[] comm-&gt;userRedOps;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;userRedOps = ops;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;userRedOpCapacity = cap;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 准备局部状态 | <code>int ix = comm-&gt;userRedOpFreeHead;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 准备局部状态 | <code>ncclUserRedOp *user = &comm-&gt;userRedOps[ix];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;userRedOpFreeHead = user-&gt;freeNext;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>user-&gt;freeNext = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>user-&gt;datatype = datatype;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>user-&gt;opFull.op = ncclDevPreMulSum;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 条件判断 | <code>(residence == ncclScalarHostImmediate)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>user-&gt;opFull.scalarArgIsPtr = false;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>std::memcpy(&user-&gt;opFull.scalarArg, scalar, ncclTypeSize(datatype));</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 25 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>user-&gt;opFull.scalarArgIsPtr = true;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>user-&gt;opFull.scalarArg = reinterpret_cast&lt;uint64_t&gt;(scalar);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>*op = ncclRedOp_t(int(ncclNumOps) + ix);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>*op = ncclUserRedOpMangle(comm, *op);</code> | 用 communicator 地址导出的可逆异或扰动用户操作句柄，减少把别的 comm 句柄误用于本 comm 的风险。 |
| 30 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (comm->userRedOpFreeHead == comm->userRedOpCapacity) {
    // double capacity and resize
    int cap = 2*comm->userRedOpCapacity;
    if (cap < 4) cap = 4;
    ncclUserRedOp *ops = new ncclUserRedOp[cap];
    std::memcpy(ops, comm->userRedOps, comm->userRedOpCapacity*sizeof(ncclUserRedOp));
    for(int ix=comm->userRedOpCapacity; ix < cap; ix++)
      ops[ix].freeNext = ix + 1;
    delete[] comm->userRedOps;
    comm->userRedOps = ops;
    comm->userRedOpCapacity = cap;
  }
  // pop from free list
  int ix = comm->userRedOpFreeHead;
  ncclUserRedOp *user = &comm->userRedOps[ix];
  comm->userRedOpFreeHead = user->freeNext;

  user->freeNext = -1; // allocated
  user->datatype = datatype;
  user->opFull.op = ncclDevPreMulSum;
  if (residence == ncclScalarHostImmediate) {
    user->opFull.scalarArgIsPtr = false;
    std::memcpy(&user->opFull.scalarArg, scalar, ncclTypeSize(datatype));
  } else {
    user->opFull.scalarArgIsPtr = true;
    user->opFull.scalarArg = reinterpret_cast<uint64_t>(scalar);
  }
  *op = ncclRedOp_t(int(ncclNumOps) + ix);
  *op = ncclUserRedOpMangle(comm, *op);
  return ncclSuccess;
}
```

</details>

<a id="f0428"></a>

## F0428：ncclRedOpDestroy

用途：验证并归还用户归约句柄槽，不会销毁已在入队时复制到任务中的操作状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(0 &lt;= int(op) && int(op) &lt; int(ncclNumOps))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("ncclRedOpDestroy : operator is a NCCL builtin.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(int(op) &lt; 0 &#124;&#124; int(ncclMaxRedOp) &lt; int(op))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>WARN("ncclRedOpDestroy : operator is garbage.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 准备局部状态 | <code>int ix = int(ncclUserRedOpMangle(comm, op)) - int(ncclNumOps);</code> | 用 communicator 地址导出的可逆异或扰动用户操作句柄，减少把别的 comm 句柄误用于本 comm 的风险。 |
| 10 | 0 / 条件判断 | <code>(comm-&gt;userRedOpCapacity &lt;= ix &#124;&#124; comm-&gt;userRedOps[ix].freeNext != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>WARN("ncclRedOpDestroy : operator unknown to this communicator.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;userRedOps[ix].freeNext = comm-&gt;userRedOpFreeHead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;userRedOpFreeHead = ix;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (0 <= int(op) && int(op) < int(ncclNumOps)) {
    WARN("ncclRedOpDestroy : operator is a NCCL builtin.");
    return ncclInvalidArgument;
  }
  if (int(op) < 0 || int(ncclMaxRedOp) < int(op)) {
    WARN("ncclRedOpDestroy :  operator is garbage.");
    return ncclInvalidArgument;
  }
  int ix = int(ncclUserRedOpMangle(comm, op)) - int(ncclNumOps);
  if (comm->userRedOpCapacity <= ix || comm->userRedOps[ix].freeNext != -1) {
    WARN("ncclRedOpDestroy : operator unknown to this communicator.");
    return ncclInvalidArgument;
  }
  // push to free list
  comm->userRedOps[ix].freeNext = comm->userRedOpFreeHead;
  comm->userRedOpFreeHead = ix;
  return ncclSuccess;
}
```

</details>

