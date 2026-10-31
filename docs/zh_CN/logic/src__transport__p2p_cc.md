# src/transport/p2p.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1153"></a>

## F1153：busIdToCudaDev

用途：把 GPU PCI busId 映射到本进程 CUDA_VISIBLE_DEVICES 下可见设备序号。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int ndev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(cudaGetDeviceCount(&ndev) != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; ndev；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 准备局部状态 | <code>char devBusIdStr[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 条件判断 | <code>(cudaDeviceGetPCIBusId(devBusIdStr, NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE, i) != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 1 / 准备局部状态 | <code>int64_t devBusId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(busIdToInt64(devBusIdStr, &devBusId));</code> | 失败时可能提前return，成功才继续；在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。 |
| 12 | 1 / 条件判断 | <code>(busId == devBusId)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 3 / 函数返回 | <code>return i;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 15 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 0 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int ndev;
  if (cudaGetDeviceCount(&ndev) != cudaSuccess)
    return -1;
  for (int i = 0; i < ndev; i++) {
    char devBusIdStr[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
    if (cudaDeviceGetPCIBusId(devBusIdStr, NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE, i) != cudaSuccess)
      return -1;
    int64_t devBusId;
    NCCLCHECK(busIdToInt64(devBusIdStr, &devBusId));
    if (busId == devBusId) return i;
  }
  // BusId was not found in our locally visible CUDA devices
  return -1;
}
```

</details>

<a id="f1154"></a>

## F1154：p2pCanConnect

用途：检查同机/共享环境、拓扑策略、CUDA peer access 与 IPC 支持，决定能否走 GPU P2P transport。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(info1-&gt;hostHash != info2-&gt;hostHash &#124;&#124; info1-&gt;shmDev != info2-&gt;shmDev)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>*ret = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int intermediateRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckP2p(topo, info1-&gt;busId, info2-&gt;busId, ret, NULL, &intermediateRank));</code> | 失败时可能提前return，成功才继续；结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。 |
| 7 | 0 / 条件判断 | <code>(*ret == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 条件判断 | <code>(intermediateRank != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 准备局部状态 | <code>int cudaDev1 = busIdToCudaDev(info1-&gt;busId);</code> | 把 GPU PCI busId 映射到本进程 CUDA_VISIBLE_DEVICES 下可见设备序号。 |
| 14 | 0 / 准备局部状态 | <code>int cudaDev2 = busIdToCudaDev(info2-&gt;busId);</code> | 把 GPU PCI busId 映射到本进程 CUDA_VISIBLE_DEVICES 下可见设备序号。 |
| 15 | 0 / 条件判断 | <code>(cudaDev1 == -1 &#124;&#124; cudaDev2 == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 10010</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 18 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 19 | 3 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>*ret = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 0 / 准备局部状态 | <code>int p2p;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 0 / 条件判断 | <code>(cudaDeviceCanAccessPeer(&p2p, cudaDev1, cudaDev2) != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_P2P,"peer query failed between dev %d(=%lx) and dev %d(=%lx)", cudaDev1, info1-&gt;busId, cudaDev2, info2-&gt;busId);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>*ret = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 28 | 0 / 条件判断 | <code>(p2p != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 2 / 准备局部状态 | <code>static int legacyIPC = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 2 / 条件判断 | <code>(legacyIPC &gt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 4 / 调用 / 计算 / 写入 | <code>*ret = legacyIPC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 35 | 2 / 准备局部状态 | <code>char *dummy;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 36 | 2 / 准备局部状态 | <code>cudaIpcMemHandle_t ipc;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaCalloc(&dummy, CUDA_IPC_MIN));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 38 | 2 / 条件判断 | <code>(cudaIpcGetMemHandle(&ipc, dummy) != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_P2P,"Legacy IPC not supported");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 4 / 调用 / 计算 / 写入 | <code>*ret = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(dummy));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 43 | 2 / 调用 / 计算 / 写入 | <code>legacyIPC = *ret;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 45 | 0 / 条件判断 | <code>(p2p == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_P2P,"Could not enable P2P between dev %d(=%lx) and dev %d(=%lx)", cudaDev1, info1-&gt;busId, cudaDev2, info2-&gt;busId);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 2 / 调用 / 计算 / 写入 | <code>*ret = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 50 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Rule out different nodes / isolated containers
  if (info1->hostHash != info2->hostHash || info1->shmDev != info2->shmDev) {
    *ret = 0;
    return ncclSuccess;
  }

  // Check topology / p2p level.
  int intermediateRank;
  NCCLCHECK(ncclTopoCheckP2p(topo, info1->busId, info2->busId, ret, NULL, &intermediateRank));
  if (*ret == 0) return ncclSuccess;
  if (intermediateRank != -1) return ncclSuccess;

  // Convert the peer's busId into a local cudaDev index (cf. CUDA_VISIBLE_DEVICES)
  int cudaDev1 = busIdToCudaDev(info1->busId);
  int cudaDev2 = busIdToCudaDev(info2->busId);
  if (cudaDev1 == -1 || cudaDev2 == -1) {
#if CUDART_VERSION >= 10010
    // CUDA 10.1 and later can use P2P with invisible devices.
    return ncclSuccess;
#else
    // Peer's CUDA device is not visible in this process : we can't communicate with it.
    *ret = 0;
    return ncclSuccess;
#endif
  }

  // Check that CUDA can do P2P
  int p2p;
  if (cudaDeviceCanAccessPeer(&p2p, cudaDev1, cudaDev2) != cudaSuccess) {
    INFO(NCCL_INIT|NCCL_P2P,"peer query failed between dev %d(=%lx) and dev %d(=%lx)",
         cudaDev1, info1->busId, cudaDev2, info2->busId);
    *ret = 0;
    return ncclSuccess;
  }

  if (p2p != 0) {
    // Cached result of the legacyIPC detection
    static int legacyIPC = -1;
    if (legacyIPC >= 0) {
      *ret = legacyIPC;
      return ncclSuccess;
    }
    // Check that legacy IPC support is available (WSL WAR)
    char *dummy;
    cudaIpcMemHandle_t ipc;
    NCCLCHECK(ncclCudaCalloc(&dummy, CUDA_IPC_MIN));
    if (cudaIpcGetMemHandle(&ipc, dummy) != cudaSuccess) {
      INFO(NCCL_INIT|NCCL_P2P,"Legacy IPC not supported");
      *ret = 0;
    }
    CUDACHECK(cudaFree(dummy));
    legacyIPC = *ret;
    return ncclSuccess;
  }

  if (p2p == 0) {
    INFO(NCCL_INIT|NCCL_P2P,"Could not enable P2P between dev %d(=%lx) and dev %d(=%lx)",
         cudaDev1, info1->busId, cudaDev2, info2->busId);
    *ret = 0;
    return ncclSuccess;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1155"></a>

## F1155：p2pGetInfo

用途：查询 GPU 对的 P2P 可达性、读写模式及可能的中间 rank，把拓扑结果提供给 setup。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int p2p;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckP2p(topo, info1-&gt;busId, info2-&gt;busId, &p2p, read, intermediateRank));</code> | 失败时可能提前return，成功才继续；结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。 |
| 3 | 0 / 准备局部状态 | <code>int readEnable = ncclParamP2pReadEnable();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(readEnable != -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>*read = readEnable;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int p2p;
  // Queries the topology to see if the GPUs are Ampere and
  // connected via NVLink, if so we enable P2P Read by default
  NCCLCHECK(ncclTopoCheckP2p(topo, info1->busId, info2->busId, &p2p, read, intermediateRank));

  int readEnable = ncclParamP2pReadEnable();
  if (readEnable != -2) *read = readEnable;
  return ncclSuccess;
}
```

</details>

<a id="f1156"></a>

## F1156：p2pMap

用途：同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(myInfo-&gt;pidHash == peerInfo-&gt;pidHash)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 条件判断 | <code>(peerInfo-&gt;cudaDev != myInfo-&gt;cudaDev)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 4 / 准备局部状态 | <code>cudaError_t err = cudaDeviceEnablePeerAccess(peerInfo-&gt;cudaDev, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 4 / 条件判断 | <code>(err == cudaErrorPeerAccessAlreadyEnabled)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 6 / 调用 / 计算 / 写入 | <code>cudaGetLastError();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 10 | 6 / 条件判断 | <code>(err != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 8 / 调用 / 计算 / 写入 | <code>WARN("failed to peer with device %d(=%lx): %d %s", peerInfo-&gt;cudaDev, peerInfo-&gt;busId, err, cudaGetErrorString(err));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 8 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>*devMem = p2pBuff-&gt;directPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>*ipcPtr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcOpenMemHandle(devMem, p2pBuff-&gt;devIpc, cudaIpcMemLazyEnablePeerAccess));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>*ipcPtr = *devMem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (myInfo->pidHash == peerInfo->pidHash) {
    if (peerInfo->cudaDev != myInfo->cudaDev) {
      // Enable P2P access
      cudaError_t err = cudaDeviceEnablePeerAccess(peerInfo->cudaDev, 0);
      if (err == cudaErrorPeerAccessAlreadyEnabled) {
        cudaGetLastError();
      } else if (err != cudaSuccess) {
        WARN("failed to peer with device %d(=%lx): %d %s",
            peerInfo->cudaDev, peerInfo->busId, err, cudaGetErrorString(err));
        return ncclInternalError;
      }
    }
    *devMem = p2pBuff->directPtr;
    *ipcPtr = NULL;
  } else {
    CUDACHECK(cudaIpcOpenMemHandle(devMem, p2pBuff->devIpc, cudaIpcMemLazyEnablePeerAccess));
    *ipcPtr = *devMem;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1157"></a>

## F1157：p2pSendSetup

用途：按方向分配/导出 GPU P2P 控制和协议缓冲，必要时借助中间 GPU 代理准备资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct p2pSendResources* resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 准备局部状态 | <code>int useRead, intermediateRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(p2pGetInfo(comm-&gt;topo, myInfo, peerInfo, &useRead, &intermediateRank));</code> | 失败时可能提前return，成功才继续；查询 GPU 对的 P2P 可达性、读写模式及可能的中间 rank，把拓扑结果提供给 setup。 |
| 6 | 0 / 源码语法片段 | <code>static_assert(sizeof(struct p2pConnectInfo) &lt;= sizeof(struct ncclConnect), "p2p Connect Info is too big");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 7 | 0 / 准备局部状态 | <code>struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;read = useRead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 条件判断 | <code>(graph && connIndex == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;read = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 准备局部状态 | <code>const char* useReadStr = info-&gt;read ? "/read" : "";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>int sendSize = sizeof(struct ncclSendMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 条件判断 | <code>(info-&gt;read)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>sendSize += send-&gt;comm-&gt;buffSizes[NCCL_PROTO_SIMPLE];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>ALIGN_SIZE(sendSize, CUDA_IPC_MIN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 0 / 条件判断 | <code>(intermediateRank == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;rank = myInfo-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 2 / 条件判断 | <code>(myInfo-&gt;pidHash == peerInfo-&gt;pidHash)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 条件判断 | <code>(ncclParamP2pDirectDisable() == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 6 / 调用 / 计算 / 写入 | <code>send-&gt;conn.direct &#124;= info-&gt;read ? NCCL_DIRECT_READ : NCCL_DIRECT_WRITE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_P2P, "Channel %02d : %d[%lx] -&gt; %d[%lx] via P2P/direct pointer%s", channelId, myInfo-&gt;rank, myInfo-&gt;busId, peerInfo-&gt;rank, peerInfo-&gt;busId, useReadStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 28 | 4 / 调用 / 计算 / 写入 | <code>send-&gt;conn.direct &#124;= info-&gt;read ? NCCL_IPC_READ : NCCL_IPC_WRITE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_P2P,"Channel %02d : %d[%lx] -&gt; %d[%lx] via P2P/IPC%s", channelId, myInfo-&gt;rank, myInfo-&gt;busId, peerInfo-&gt;rank, peerInfo-&gt;busId, useReadStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 31 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;rank = intermediateRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_P2P, "Channel %02d : %d[%lx] -&gt; %d[%lx] via P2P/indirect/%d[%lx]%s", channelId, myInfo-&gt;rank, myInfo-&gt;busId, peerInfo-&gt;rank, peerInfo-&gt;busId, intermediateRank, comm-&gt;peerInfo[intermediateRank].busId, useReadStr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_P2P, 1, info-&gt;rank, &send-&gt;proxyConn));</code> | 失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&send-&gt;proxyConn, ncclProxyMsgSetup, &sendSize, sizeof(int), &info-&gt;p2pBuff, sizeof(struct ncclP2pBuff)));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(p2pMap(myInfo, comm-&gt;peerInfo+info-&gt;rank, &info-&gt;p2pBuff, (void**)&resources-&gt;devMem, &resources-&gt;sendMemIpc));</code> | 失败时可能提前return，成功才继续；同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。 |
| 36 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct p2pSendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  send->transportResources = resources;
  int useRead, intermediateRank;
  NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead, &intermediateRank));

  static_assert(sizeof(struct p2pConnectInfo) <= sizeof(struct ncclConnect), "p2p Connect Info is too big");
  struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;
  info->read = useRead;
  // For CollNet, use write for scatter-reduce (conn 1), read for broadcast-gather (conn 0)
  if (graph && connIndex == 1) info->read = 0;
  const char* useReadStr = info->read ? "/read" : "";

  int sendSize = sizeof(struct ncclSendMem);
  // For P2P Read the SIMPLE buffer is tagged on the end of the ncclSendMem structure
  if (info->read) sendSize += send->comm->buffSizes[NCCL_PROTO_SIMPLE];
  ALIGN_SIZE(sendSize, CUDA_IPC_MIN);

  if (intermediateRank == -1) {
    info->rank = myInfo->rank;
    if (myInfo->pidHash == peerInfo->pidHash) {
      if (ncclParamP2pDirectDisable() == 0) send->conn.direct |= info->read ? NCCL_DIRECT_READ : NCCL_DIRECT_WRITE;
      INFO(NCCL_INIT|NCCL_P2P, "Channel %02d : %d[%lx] -> %d[%lx] via P2P/direct pointer%s",
          channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, useReadStr);
    } else {
      send->conn.direct |= info->read ? NCCL_IPC_READ : NCCL_IPC_WRITE;
      INFO(NCCL_INIT|NCCL_P2P,"Channel %02d : %d[%lx] -> %d[%lx] via P2P/IPC%s",
          channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, useReadStr);
    }
  } else {
    info->rank = intermediateRank;
    INFO(NCCL_INIT|NCCL_P2P, "Channel %02d : %d[%lx] -> %d[%lx] via P2P/indirect/%d[%lx]%s",
        channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, intermediateRank,
	comm->peerInfo[intermediateRank].busId, useReadStr);
  }

  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_P2P, 1, info->rank, &send->proxyConn));
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgSetup, &sendSize, sizeof(int), &info->p2pBuff, sizeof(struct ncclP2pBuff)));

  NCCLCHECK(p2pMap(myInfo, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&resources->devMem, &resources->sendMemIpc));
  return ncclSuccess;
}
```

</details>

<a id="f1158"></a>

## F1158：p2pRecvSetup

用途：按方向分配/导出 GPU P2P 控制和协议缓冲，必要时借助中间 GPU 代理准备资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct p2pRecvResources* resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 准备局部状态 | <code>int useRead, intermediateRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(p2pGetInfo(comm-&gt;topo, myInfo, peerInfo, &useRead, &intermediateRank));</code> | 失败时可能提前return，成功才继续；查询 GPU 对的 P2P 可达性、读写模式及可能的中间 rank，把拓扑结果提供给 setup。 |
| 6 | 0 / 源码语法片段 | <code>static_assert(sizeof(struct p2pConnectInfo) &lt;= sizeof(struct ncclConnect), "p2p Connect Info is too big");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 7 | 0 / 准备局部状态 | <code>struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;read = useRead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 条件判断 | <code>(graph && connIndex == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;read = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 准备局部状态 | <code>int recvSize = sizeof(struct ncclRecvMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 1 / 条件判断 | <code>(!(info-&gt;read && p == NCCL_PROTO_SIMPLE))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>recvSize += recv-&gt;comm-&gt;buffSizes[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>ALIGN_SIZE(recvSize, CUDA_IPC_MIN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 0 / 条件判断 | <code>(intermediateRank == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;rank = myInfo-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 2 / 条件判断 | <code>(myInfo-&gt;pidHash == peerInfo-&gt;pidHash)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 4 / 条件判断 | <code>(ncclParamP2pDirectDisable() == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 6 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.direct &#124;= info-&gt;read ? NCCL_DIRECT_READ : NCCL_DIRECT_WRITE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 28 | 4 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.direct &#124;= info-&gt;read ? NCCL_IPC_READ : NCCL_IPC_WRITE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;rank = intermediateRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_P2P, 0, info-&gt;rank, &recv-&gt;proxyConn));</code> | 失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&recv-&gt;proxyConn, ncclProxyMsgSetup, &recvSize, sizeof(int), &info-&gt;p2pBuff, sizeof(struct ncclP2pBuff)));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(p2pMap(myInfo, comm-&gt;peerInfo+info-&gt;rank, &info-&gt;p2pBuff, (void**)&resources-&gt;devMem, &resources-&gt;recvMemIpc));</code> | 失败时可能提前return，成功才继续；同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。 |
| 34 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct p2pRecvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  recv->transportResources = resources;
  int useRead, intermediateRank;
  NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead, &intermediateRank));

  static_assert(sizeof(struct p2pConnectInfo) <= sizeof(struct ncclConnect), "p2p Connect Info is too big");
  struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;
  info->read = useRead;
  // For CollNet, use write for scatter-reduce (conn 1), read for broadcast-gather (conn 0)
  if (graph && connIndex == 1) info->read = 0;

  int recvSize = sizeof(struct ncclRecvMem);
  // For P2P Read the SIMPLE buffer is tagged on the end of the ncclSendMem structure
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) if (!(info->read && p == NCCL_PROTO_SIMPLE)) recvSize += recv->comm->buffSizes[p];
  ALIGN_SIZE(recvSize, CUDA_IPC_MIN);

  if (intermediateRank == -1) {
    info->rank = myInfo->rank;
    if (myInfo->pidHash == peerInfo->pidHash) {
      if (ncclParamP2pDirectDisable() == 0) recv->conn.direct |= info->read ? NCCL_DIRECT_READ : NCCL_DIRECT_WRITE;
    } else {
      recv->conn.direct |= info->read ? NCCL_IPC_READ : NCCL_IPC_WRITE;
    }
  } else {
    info->rank = intermediateRank;
  }

  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_P2P, 0, info->rank, &recv->proxyConn));
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgSetup, &recvSize, sizeof(int), &info->p2pBuff, sizeof(struct ncclP2pBuff)));

  NCCLCHECK(p2pMap(myInfo, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&resources->devMem, &resources->recvMemIpc));
  return ncclSuccess;
}
```

</details>

<a id="f1159"></a>

## F1159：p2pSendConnect

用途：导入对端 GPU 分配并填充协议缓冲、head/tail 和直接访问指针交换区。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct p2pSendResources* resources = (struct p2pSendResources*)send-&gt;transportResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclRecvMem* remDevMem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(p2pMap(comm-&gt;peerInfo+rank, comm-&gt;peerInfo+info-&gt;rank, &info-&gt;p2pBuff, (void**)&remDevMem, &resources-&gt;recvMemIpc));</code> | 失败时可能提前return，成功才继续；同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。 |
| 5 | 0 / 准备局部状态 | <code>char* buff = (char*)(remDevMem+1);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 条件判断 | <code>(info-&gt;read && p == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>send-&gt;conn.buffs[p] = (char*)(resources-&gt;devMem+1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>send-&gt;conn.buffs[p] = buff;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>buff += send-&gt;comm-&gt;buffSizes[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.tail = &remDevMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.head = &resources-&gt;devMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.ptrExchange = &resources-&gt;devMem-&gt;ptrExchange;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.redOpArgExchange = resources-&gt;devMem-&gt;redOpArgExchange;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct p2pSendResources* resources = (struct p2pSendResources*)send->transportResources;
  struct ncclRecvMem* remDevMem;
  struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;

  NCCLCHECK(p2pMap(comm->peerInfo+rank, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&remDevMem, &resources->recvMemIpc));

  char* buff = (char*)(remDevMem+1);
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (info->read && p == NCCL_PROTO_SIMPLE) {
      /* For P2P Read the SIMPLE buffer is local (ncclSendMem) */
      send->conn.buffs[p] = (char*)(resources->devMem+1);
    } else {
      send->conn.buffs[p] = buff;
      buff += send->comm->buffSizes[p];
    }
  }
  send->conn.tail = &remDevMem->tail;
  send->conn.head = &resources->devMem->head;
  send->conn.ptrExchange = &resources->devMem->ptrExchange;
  send->conn.redOpArgExchange = resources->devMem->redOpArgExchange;
  return ncclSuccess;
}
```

</details>

<a id="f1160"></a>

## F1160：p2pRecvConnect

用途：导入对端 GPU 分配并填充协议缓冲、head/tail 和直接访问指针交换区。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct p2pRecvResources* resources = (struct p2pRecvResources*)recv-&gt;transportResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclSendMem* remDevMem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(p2pMap(comm-&gt;peerInfo+rank, comm-&gt;peerInfo+info-&gt;rank, &info-&gt;p2pBuff, (void**)&remDevMem, &resources-&gt;sendMemIpc));</code> | 失败时可能提前return，成功才继续；同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。 |
| 5 | 0 / 准备局部状态 | <code>char* buff = (char*)(resources-&gt;devMem+1);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 条件判断 | <code>(info-&gt;read && p == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.buffs[p] = (char*)(remDevMem+1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.buffs[p] = buff;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>buff += recv-&gt;comm-&gt;buffSizes[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.tail = &resources-&gt;devMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.head = &remDevMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.ptrExchange = &remDevMem-&gt;ptrExchange;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.redOpArgExchange = remDevMem-&gt;redOpArgExchange;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct p2pRecvResources* resources = (struct p2pRecvResources*)recv->transportResources;
  struct ncclSendMem* remDevMem;
  struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;

  NCCLCHECK(p2pMap(comm->peerInfo+rank, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&remDevMem, &resources->sendMemIpc));

  char* buff = (char*)(resources->devMem+1);
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (info->read && p == NCCL_PROTO_SIMPLE) {
      /* For P2P Read the SIMPLE buffer is remote (ncclSendMem) */
      recv->conn.buffs[p] = (char*)(remDevMem+1);
    } else {
      recv->conn.buffs[p] = buff;
      buff += recv->comm->buffSizes[p];
    }
  }
  recv->conn.tail = &resources->devMem->tail;
  recv->conn.head = &remDevMem->head;
  recv->conn.ptrExchange = &remDevMem->ptrExchange;
  recv->conn.redOpArgExchange = remDevMem->redOpArgExchange;
  return ncclSuccess;
}
```

</details>

<a id="f1161"></a>

## F1161：p2pSendFree

用途：关闭导入的 IPC 映射并归还拥有的 GPU 分配或代理资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct p2pSendResources* resources = (struct p2pSendResources*)send-&gt;transportResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(resources-&gt;sendMemIpc)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcCloseMemHandle(resources-&gt;sendMemIpc));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 条件判断 | <code>(resources-&gt;recvMemIpc)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcCloseMemHandle(resources-&gt;recvMemIpc));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>free(resources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct p2pSendResources* resources = (struct p2pSendResources*)send->transportResources;
  if (resources->sendMemIpc) CUDACHECK(cudaIpcCloseMemHandle(resources->sendMemIpc));
  if (resources->recvMemIpc) CUDACHECK(cudaIpcCloseMemHandle(resources->recvMemIpc));
  free(resources);
  return ncclSuccess;
}
```

</details>

<a id="f1162"></a>

## F1162：p2pRecvFree

用途：关闭导入的 IPC 映射并归还拥有的 GPU 分配或代理资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct p2pRecvResources* resources = (struct p2pRecvResources*)recv-&gt;transportResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(resources-&gt;sendMemIpc)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcCloseMemHandle(resources-&gt;sendMemIpc));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 条件判断 | <code>(resources-&gt;recvMemIpc)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcCloseMemHandle(resources-&gt;recvMemIpc));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>free(resources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct p2pRecvResources* resources = (struct p2pRecvResources*)recv->transportResources;
  if (resources->sendMemIpc) CUDACHECK(cudaIpcCloseMemHandle(resources->sendMemIpc));
  if (resources->recvMemIpc) CUDACHECK(cudaIpcCloseMemHandle(resources->recvMemIpc));
  free(resources);
  return ncclSuccess;
}
```

</details>

<a id="f1163"></a>

## F1163：p2pProxySetup

用途：由资源所属代理分配/导出 GPU 内存，供主线程建立 P2P/中间 GPU 路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(reqSize != sizeof(int))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>int size = *((int*)reqBuff);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(respSize != sizeof(struct ncclP2pBuff))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 准备局部状态 | <code>struct ncclP2pBuff* p2pBuff = (struct ncclP2pBuff*)respBuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaCalloc((char**)&p2pBuff-&gt;directPtr, size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;transportResources = p2pBuff-&gt;directPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 准备局部状态 | <code>cudaError_t res = cudaIpcGetMemHandle(&p2pBuff-&gt;devIpc, p2pBuff-&gt;directPtr);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 条件判断 | <code>(res != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>WARN("cudaIpcGetMemHandle failed : %s", cudaGetErrorString(res));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>cudaFree(p2pBuff-&gt;directPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>free(p2pBuff);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(res);</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>*done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (reqSize != sizeof(int)) return ncclInternalError;
  int size = *((int*)reqBuff);
  if (respSize != sizeof(struct ncclP2pBuff)) return ncclInternalError;
  struct ncclP2pBuff* p2pBuff = (struct ncclP2pBuff*)respBuff;
  NCCLCHECK(ncclCudaCalloc((char**)&p2pBuff->directPtr, size));
  connection->transportResources = p2pBuff->directPtr;
  cudaError_t res = cudaIpcGetMemHandle(&p2pBuff->devIpc, p2pBuff->directPtr);
  if (res != cudaSuccess) {
    WARN("cudaIpcGetMemHandle failed : %s", cudaGetErrorString(res));
    cudaFree(p2pBuff->directPtr);
    free(p2pBuff);
    CUDACHECK(res);
  }
  *done = 1;
  return ncclSuccess;
}
```

</details>

<a id="f1164"></a>

## F1164：p2pProxyFree

用途：由拥有分配的代理回收 P2P GPU 资源，与映射导入方的解除映射分工。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>cudaFree(connection-&gt;transportResources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Do not check return code as CUDA may have already shut down
  cudaFree(connection->transportResources);
  return ncclSuccess;
}
```

</details>

