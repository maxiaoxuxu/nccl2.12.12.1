# src/misc/nvmlwrap.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0949"></a>

## F0949：ncclNvmlDeviceGetHandleByPciBusId

用途：按 PCI 地址定位 NVML 设备；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 2 | 0 / 准备局部状态 | <code>std::lock_guard&lt;std::mutex&gt; locked(lock);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NVMLCHECK(nvmlDeviceGetHandleByPciBusId, pciBusId, device);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNvmlEnsureInitialized());
  std::lock_guard<std::mutex> locked(lock);
  NVMLCHECK(nvmlDeviceGetHandleByPciBusId, pciBusId, device);
  return ncclSuccess;
}
```

</details>

<a id="f0950"></a>

## F0950：ncclNvmlDeviceGetHandleByIndex

用途：按 NVML 序号取得设备句柄；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>*device = ncclNvmlDevices[index].handle;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNvmlEnsureInitialized());
  *device = ncclNvmlDevices[index].handle;
  return ncclSuccess;
}
```

</details>

<a id="f0951"></a>

## F0951：ncclNvmlDeviceGetIndex

用途：由句柄查询 NVML 序号；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 2 | 0 / 循环 | <code>初始 int d=0;；继续条件 d &lt; ncclNvmlDeviceCount；每轮更新 d++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(ncclNvmlDevices[d].handle == device)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>*index = d;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNvmlEnsureInitialized());
  for (int d=0; d < ncclNvmlDeviceCount; d++) {
    if (ncclNvmlDevices[d].handle == device) {
      *index = d;
      return ncclSuccess;
    }
  }
  return ncclInvalidArgument;
}
```

</details>

<a id="f0952"></a>

## F0952：ncclNvmlDeviceGetNvLinkState

用途：查询指定 NVLink 是否激活；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 2 | 0 / 准备局部状态 | <code>std::lock_guard&lt;std::mutex&gt; locked(lock);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NVMLTRY(nvmlDeviceGetNvLinkState, device, link, isActive);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNvmlEnsureInitialized());
  std::lock_guard<std::mutex> locked(lock);
  NVMLTRY(nvmlDeviceGetNvLinkState, device, link, isActive);
  return ncclSuccess;
}
```

</details>

<a id="f0953"></a>

## F0953：ncclNvmlDeviceGetNvLinkRemotePciInfo

用途：查询 NVLink 远端 PCI 标识；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 2 | 0 / 准备局部状态 | <code>std::lock_guard&lt;std::mutex&gt; locked(lock);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NVMLTRY(nvmlDeviceGetNvLinkRemotePciInfo, device, link, pci);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNvmlEnsureInitialized());
  std::lock_guard<std::mutex> locked(lock);
  NVMLTRY(nvmlDeviceGetNvLinkRemotePciInfo, device, link, pci);
  return ncclSuccess;
}
```

</details>

<a id="f0954"></a>

## F0954：ncclNvmlDeviceGetNvLinkCapability

用途：查询 NVLink 能力；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 2 | 0 / 准备局部状态 | <code>std::lock_guard&lt;std::mutex&gt; locked(lock);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NVMLTRY(nvmlDeviceGetNvLinkCapability, device, link, capability, capResult);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNvmlEnsureInitialized());
  std::lock_guard<std::mutex> locked(lock);
  NVMLTRY(nvmlDeviceGetNvLinkCapability, device, link, capability, capResult);
  return ncclSuccess;
}
```

</details>

<a id="f0955"></a>

## F0955：ncclNvmlDeviceGetCudaComputeCapability

用途：查询 GPU 计算能力；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 2 | 0 / 循环 | <code>初始 int d=0;；继续条件 d &lt; ncclNvmlDeviceCount；每轮更新 d++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(device == ncclNvmlDevices[d].handle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>*major = ncclNvmlDevices[d].computeCapabilityMajor;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>*minor = ncclNvmlDevices[d].computeCapabilityMinor;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 9 | 0 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNvmlEnsureInitialized());

  for(int d=0; d < ncclNvmlDeviceCount; d++) {
    if(device == ncclNvmlDevices[d].handle) {
      *major = ncclNvmlDevices[d].computeCapabilityMajor;
      *minor = ncclNvmlDevices[d].computeCapabilityMinor;
      return ncclSuccess;
    }
  }
  return ncclInvalidArgument;
}
```

</details>

<a id="f0956"></a>

## F0956：ncclNvmlDeviceGetP2PStatus

用途：查询 GPU 对之间的特定 P2P 能力状态；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlEnsureInitialized());</code> | 失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。 |
| 2 | 0 / 条件判断 | <code>(p2pIndex == NVML_P2P_CAPS_INDEX_READ &#124;&#124; p2pIndex == NVML_P2P_CAPS_INDEX_WRITE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 准备局部状态 | <code>int a = -1, b = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 循环 | <code>初始 int d=0;；继续条件 d &lt; ncclNvmlDeviceCount；每轮更新 d++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 3 / 条件判断 | <code>(device1 == ncclNvmlDevices[d].handle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 调用 / 计算 / 写入 | <code>a = d;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 条件判断 | <code>(device2 == ncclNvmlDevices[d].handle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 5 / 调用 / 计算 / 写入 | <code>b = d;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 2 / 条件判断 | <code>(a == -1 &#124;&#124; b == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 16 | 2 / 条件判断 | <code>(p2pIndex == NVML_P2P_CAPS_INDEX_READ)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 调用 / 计算 / 写入 | <code>*p2pStatus = ncclNvmlDevicePairs[a][b].p2pStatusRead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>*p2pStatus = ncclNvmlDevicePairs[a][b].p2pStatusWrite;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 22 | 2 / 准备局部状态 | <code>std::lock_guard&lt;std::mutex&gt; locked(lock);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>NVMLCHECK(nvmlDeviceGetP2PStatus, device1, device2, p2pIndex, p2pStatus);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNvmlEnsureInitialized());

  if (p2pIndex == NVML_P2P_CAPS_INDEX_READ || p2pIndex == NVML_P2P_CAPS_INDEX_WRITE) {
    int a = -1, b = -1;
    for(int d=0; d < ncclNvmlDeviceCount; d++) {
      if(device1 == ncclNvmlDevices[d].handle) a = d;
      if(device2 == ncclNvmlDevices[d].handle) b = d;
    }
    if (a == -1 || b == -1) return ncclInvalidArgument;
    if (p2pIndex == NVML_P2P_CAPS_INDEX_READ)
      *p2pStatus = ncclNvmlDevicePairs[a][b].p2pStatusRead;
    else
      *p2pStatus = ncclNvmlDevicePairs[a][b].p2pStatusWrite;
  }
  else {
    std::lock_guard<std::mutex> locked(lock);
    NVMLCHECK(nvmlDeviceGetP2PStatus, device1, device2, p2pIndex, p2pStatus);
  }
  return ncclSuccess;
}
```

</details>

