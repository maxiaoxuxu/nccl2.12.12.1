# src/transport/net_ib.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1096"></a>

## F1096：ncclIbAsyncThreadMain

用途：持续读取并确认 IB 设备异步事件，发现端口/设备异常时记录诊断信息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ibv_context* context = (struct ibv_context*)args;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 ；继续条件 (1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 准备局部状态 | <code>struct ibv_async_event event;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 条件判断 | <code>(ncclSuccess != wrap_ibv_get_async_event(context, &event))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 7 | 1 / 准备局部状态 | <code>char *str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 条件判断 | <code>(ncclSuccess != wrap_ibv_event_type_str(&str, event.event_type))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 11 | 1 / 条件判断 | <code>(event.event_type != IBV_EVENT_COMM_EST)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 3 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : Got async event : %s", str);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 1 / 条件判断 | <code>(ncclSuccess != wrap_ibv_ack_async_event(&event))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 17 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ibv_context* context = (struct ibv_context*)args;
  while (1) {
    struct ibv_async_event event;
    if (ncclSuccess != wrap_ibv_get_async_event(context, &event)) { break; }
    char *str;
    if (ncclSuccess != wrap_ibv_event_type_str(&str, event.event_type)) { break; }
    if (event.event_type != IBV_EVENT_COMM_EST)
      WARN("NET/IB : Got async event : %s", str);
    if (ncclSuccess != wrap_ibv_ack_async_event(&event)) { break; }
  }
  return NULL;
}
```

</details>

<a id="f1097"></a>

## F1097：ncclIbGetPciPath

用途：获取 verbs 设备在 sysfs 的 PCI 路径和实际端口信息，使拓扑可关联该 NIC。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char devicePath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>snprintf(devicePath, PATH_MAX, "/sys/class/infiniband/%s/device", devName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 准备局部状态 | <code>char* p = realpath(devicePath, NULL);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(p == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>WARN("Could not find real path of %s (%s)", devName, devicePath);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>p[strlen(p)-1] = '0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>p[strlen(p)-3] = '0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>*realPort = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 循环 | <code>初始 int d=0;；继续条件 d&lt;ncclNIbDevs；每轮更新 d++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 3 / 条件判断 | <code>(strcmp(p, ncclIbDevs[d].pciPath) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 5 / 调用 / 计算 / 写入 | <code>(*realPort)++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>*path = p;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char devicePath[PATH_MAX];
  snprintf(devicePath, PATH_MAX, "/sys/class/infiniband/%s/device", devName);
  char* p = realpath(devicePath, NULL);
  if (p == NULL) {
    WARN("Could not find real path of %s (%s)", devName, devicePath);
  } else {
    // Merge multi-port NICs into the same PCI device
    p[strlen(p)-1] = '0';
    // Also merge virtual functions (VF) into the same device
    p[strlen(p)-3] = '0';
    // And keep the real port aside (the ibv port is always 1 on recent cards)
    *realPort = 0;
    for (int d=0; d<ncclNIbDevs; d++) {
      if (strcmp(p, ncclIbDevs[d].pciPath) == 0) (*realPort)++;
    }
  }
  *path = p;
  return ncclSuccess;
}
```

</details>

<a id="f1098"></a>

## F1098：firstBitSet

用途：定位位掩码最低有效位，把 IB 能力字段转换为对应宽度/速度条目。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int i = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 ；继续条件 (i&lt;max && ((val & (1&lt;&lt;i)) == 0))；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>i++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 5 | 0 / 函数返回 | <code>return i;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int i = 0;
  while (i<max && ((val & (1<<i)) == 0)) i++;
  return i;
}
```

</details>

<a id="f1099"></a>

## F1099：ncclIbWidth

用途：把 verbs 端口 active_width/active_speed 编码转成数值，用于报告链路带宽。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ibvWidths[firstBitSet(width, sizeof(ibvWidths)/sizeof(int)-1)];</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return ibvWidths[firstBitSet(width, sizeof(ibvWidths)/sizeof(int)-1)];
}
```

</details>

<a id="f1100"></a>

## F1100：ncclIbSpeed

用途：把 verbs 端口 active_width/active_speed 编码转成数值，用于报告链路带宽。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ibvSpeeds[firstBitSet(speed, sizeof(ibvSpeeds)/sizeof(int)-1)];</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return ibvSpeeds[firstBitSet(speed, sizeof(ibvSpeeds)/sizeof(int)-1)];
}
```

</details>

<a id="f1101"></a>

## F1101：ncclIbRelaxedOrderingCapable

用途：检查编译/运行支持和参数策略，判断 MR 注册能否启用 PCIe relaxed ordering。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int roMode = ncclParamIbPciRelaxedOrdering();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>ncclResult_t r = ncclInternalError;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(roMode == 1 &#124;&#124; roMode == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>r = wrap_ibv_reg_mr_iova2(NULL, NULL, NULL, 0, 0, 0);</code> | 注册内存并指定 HCA 使用的 IOVA 地址及访问权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 6 | 0 / 函数返回 | <code>return r == ncclInternalError ? 0 : 1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int roMode = ncclParamIbPciRelaxedOrdering();
  ncclResult_t r = ncclInternalError;
  if (roMode == 1 || roMode == 2) {
    // Query IBVERBS_1.8 API - needed for IBV_ACCESS_RELAXED_ORDERING support
    r = wrap_ibv_reg_mr_iova2(NULL, NULL, NULL, 0, 0, 0);
  }
  return r == ncclInternalError ? 0 : 1;
}
```

</details>

<a id="f1102"></a>

## F1102：ncclIbInit

用途：初始化 libibverbs、枚举活动设备/端口、筛选接口、获取属性并启动设备异步事件监控。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ncclParamIbDisable())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>static int shownIbHcaEnv = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(wrap_ibv_symbols() != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 条件判断 | <code>(ncclNIbDevs == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&ncclIbLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>wrap_ibv_fork_init();</code> | 初始化 verbs 注册页的 fork 处理；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 12 | 2 / 条件判断 | <code>(ncclNIbDevs == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>ncclNIbDevs = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 4 / 条件判断 | <code>(ncclFindInterfaces(ncclIbIfName, &ncclIbIfAddr, MAX_IF_NAME_SIZE, 1) != 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 6 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : No IP interface found.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 6 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 19 | 4 / 准备局部状态 | <code>int nIbDevs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 4 / 准备局部状态 | <code>struct ibv_device** devices;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 4 / 准备局部状态 | <code>char* userIbEnv = getenv("NCCL_IB_HCA");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 4 / 条件判断 | <code>(userIbEnv != NULL && shownIbHcaEnv++ == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 6 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET&#124;NCCL_ENV, "NCCL_IB_HCA set to %s", userIbEnv);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 4 / 准备局部状态 | <code>struct netIf userIfs[MAX_IB_DEVS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 4 / 准备局部状态 | <code>bool searchNot = userIbEnv && userIbEnv[0] == '^';</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 4 / 条件判断 | <code>(searchNot)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 6 / 调用 / 计算 / 写入 | <code>userIbEnv++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 4 / 准备局部状态 | <code>bool searchExact = userIbEnv && userIbEnv[0] == '=';</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 4 / 条件判断 | <code>(searchExact)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 6 / 调用 / 计算 / 写入 | <code>userIbEnv++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 4 / 准备局部状态 | <code>int nUserIfs = parseStringList(userIbEnv, userIfs, MAX_IB_DEVS);</code> | 把逗号分隔的接口名前缀及可选端口解析成过滤表。 |
| 35 | 4 / 条件判断 | <code>(ncclSuccess != wrap_ibv_get_device_list(&devices, &nIbDevs))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 6 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 38 | 4 / 循环 | <code>初始 int d=0;；继续条件 d&lt;nIbDevs && ncclNIbDevs&lt;MAX_IB_DEVS；每轮更新 d++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 5 / 准备局部状态 | <code>struct ibv_context * context;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 5 / 条件判断 | <code>(ncclSuccess != wrap_ibv_open_device(&context, devices[d]) &#124;&#124; context == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 7 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : Unable to open device %s", devices[d]-&gt;name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 44 | 5 / 准备局部状态 | <code>int nPorts = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 5 / 准备局部状态 | <code>struct ibv_device_attr devAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 46 | 5 / 调用 / 计算 / 写入 | <code>memset(&devAttr, 0, sizeof(devAttr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 5 / 条件判断 | <code>(ncclSuccess != wrap_ibv_query_device(context, &devAttr))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 7 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : Unable to query device %s", devices[d]-&gt;name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 50 | 7 / 条件判断 | <code>(ncclSuccess != wrap_ibv_close_device(context))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 9 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 53 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 54 | 5 / 循环 | <code>初始 int port = 1;；继续条件 port &lt;= devAttr.phys_port_cnt；每轮更新 port++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 55 | 6 / 准备局部状态 | <code>struct ibv_port_attr portAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 6 / 条件判断 | <code>(ncclSuccess != wrap_ibv_query_port(context, port, &portAttr))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 8 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : Unable to query port %d", port);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 59 | 8 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 60 | 6 / 条件判断 | <code>(portAttr.state != IBV_PORT_ACTIVE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 61 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 62 | 8 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 63 | 6 / 条件判断 | <code>(portAttr.link_layer != IBV_LINK_LAYER_INFINIBAND && portAttr.link_layer != IBV_LINK_LAYER_ETHERNET)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 8 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 66 | 6 / 条件判断 | <code>(! (matchIfList(devices[d]-&gt;name, port, userIfs, nUserIfs, searchExact) ^ searchNot))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 67 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 68 | 8 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 69 | 6 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT&#124;NCCL_NET,"NET/IB: [%d] %s:%d/%s ", d, devices[d]-&gt;name, port, portAttr.link_layer == IBV_LINK_LAYER_INFINIBAND ? "IB" : "RoCE");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 6 / 调用 / 计算 / 写入 | <code>pthread_mutex_init(&ncclIbDevs[ncclNIbDevs].lock, NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 71 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].device = d;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].guid = devAttr.sys_image_guid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].port = port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 74 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].link = portAttr.link_layer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 75 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].speed = ncclIbSpeed(portAttr.active_speed) * ncclIbWidth(portAttr.active_width);</code> | 把 verbs 端口 active_width/active_speed 编码转成数值，用于报告链路带宽。 |
| 76 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].context = context;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 77 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].pdRefs = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].pd = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 6 / 调用 / 计算 / 写入 | <code>strncpy(ncclIbDevs[ncclNIbDevs].devName, devices[d]-&gt;name, MAXNAMESIZE);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 80 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbGetPciPath(ncclIbDevs[ncclNIbDevs].devName, &ncclIbDevs[ncclNIbDevs].pciPath, &ncclIbDevs[ncclNIbDevs].realPort));</code> | 失败时可能提前return，成功才继续；获取 verbs 设备在 sysfs 的 PCI 路径和实际端口信息，使拓扑可关联该 NIC。 |
| 81 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].maxQp = devAttr.max_qp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].mrCache.capacity = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].mrCache.population = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 6 / 调用 / 计算 / 写入 | <code>ncclIbDevs[ncclNIbDevs].mrCache.slots = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 6 / 调用 / 计算 / 写入 | <code>pthread_create(&ncclIbAsyncThread, NULL, ncclIbAsyncThreadMain, context);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 86 | 6 / 调用 / 计算 / 写入 | <code>ncclSetThreadName(ncclIbAsyncThread, "NCCL IbAsync %2d", ncclNIbDevs);</code> | 为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。 |
| 87 | 6 / 调用 / 计算 / 写入 | <code>pthread_detach(ncclIbAsyncThread);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 88 | 6 / 调用 / 计算 / 写入 | <code>ncclNIbDevs++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 89 | 6 / 调用 / 计算 / 写入 | <code>nPorts++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 90 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 91 | 5 / 条件判断 | <code>(nPorts == 0 && ncclSuccess != wrap_ibv_close_device(context))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 92 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 93 | 7 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 94 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 95 | 4 / 条件判断 | <code>(nIbDevs && (ncclSuccess != wrap_ibv_free_device_list(devices)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 96 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 97 | 6 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 98 | 4 / 调用 / 计算 / 写入 | <code>;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 99 | 2 / 条件判断 | <code>(ncclNIbDevs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 100 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 101 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "NET/IB : No device found.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 102 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 103 | 4 / 准备局部状态 | <code>char line[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 104 | 4 / 调用 / 计算 / 写入 | <code>line[0] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 105 | 4 / 调用 / 计算 / 写入 | <code>ncclIbRelaxedOrderingEnabled = ncclIbRelaxedOrderingCapable();</code> | 检查编译/运行支持和参数策略，判断 MR 注册能否启用 PCIe relaxed ordering。 |
| 106 | 4 / 循环 | <code>初始 int d=0;；继续条件 d&lt;ncclNIbDevs；每轮更新 d++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 107 | 5 / 调用 / 计算 / 写入 | <code>snprintf(line+strlen(line), 1023-strlen(line), " [%d]%s:%d/%s", d, ncclIbDevs[d].devName, ncclIbDevs[d].port, ncclIbDevs[d].link == IBV_LINK_LAYER_INFINIBAND ? "IB" : "RoCE");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 108 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 109 | 4 / 调用 / 计算 / 写入 | <code>line[1023] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 110 | 4 / 准备局部状态 | <code>char addrline[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 111 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "NET/IB : Using%s %s; OOB %s:%s", line, ncclIbRelaxedOrderingEnabled ? "[RO]" : "", ncclIbIfName, ncclSocketToString(&ncclIbIfAddr, addrline));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 112 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclIbLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 113 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ncclParamIbDisable()) return ncclInternalError;
  static int shownIbHcaEnv = 0;
  if(wrap_ibv_symbols() != ncclSuccess) { return ncclInternalError; }

  if (ncclNIbDevs == -1) {
    pthread_mutex_lock(&ncclIbLock);
    wrap_ibv_fork_init();
    if (ncclNIbDevs == -1) {
      ncclNIbDevs = 0;
      if (ncclFindInterfaces(ncclIbIfName, &ncclIbIfAddr, MAX_IF_NAME_SIZE, 1) != 1) {
        WARN("NET/IB : No IP interface found.");
        return ncclInternalError;
      }

      // Detect IB cards
      int nIbDevs;
      struct ibv_device** devices;

      // Check if user defined which IB device:port to use
      char* userIbEnv = getenv("NCCL_IB_HCA");
      if (userIbEnv != NULL && shownIbHcaEnv++ == 0) INFO(NCCL_NET|NCCL_ENV, "NCCL_IB_HCA set to %s", userIbEnv);
      struct netIf userIfs[MAX_IB_DEVS];
      bool searchNot = userIbEnv && userIbEnv[0] == '^';
      if (searchNot) userIbEnv++;
      bool searchExact = userIbEnv && userIbEnv[0] == '=';
      if (searchExact) userIbEnv++;
      int nUserIfs = parseStringList(userIbEnv, userIfs, MAX_IB_DEVS);

      if (ncclSuccess != wrap_ibv_get_device_list(&devices, &nIbDevs)) return ncclInternalError;

      for (int d=0; d<nIbDevs && ncclNIbDevs<MAX_IB_DEVS; d++) {
        struct ibv_context * context;
        if (ncclSuccess != wrap_ibv_open_device(&context, devices[d]) || context == NULL) {
          WARN("NET/IB : Unable to open device %s", devices[d]->name);
          continue;
        }
        int nPorts = 0;
        struct ibv_device_attr devAttr;
        memset(&devAttr, 0, sizeof(devAttr));
        if (ncclSuccess != wrap_ibv_query_device(context, &devAttr)) {
          WARN("NET/IB : Unable to query device %s", devices[d]->name);
          if (ncclSuccess != wrap_ibv_close_device(context)) { return ncclInternalError; }
          continue;
        }
        for (int port = 1; port <= devAttr.phys_port_cnt; port++) {
          struct ibv_port_attr portAttr;
          if (ncclSuccess != wrap_ibv_query_port(context, port, &portAttr)) {
            WARN("NET/IB : Unable to query port %d", port);
            continue;
          }
          if (portAttr.state != IBV_PORT_ACTIVE) continue;
          if (portAttr.link_layer != IBV_LINK_LAYER_INFINIBAND
              && portAttr.link_layer != IBV_LINK_LAYER_ETHERNET) continue;

          // check against user specified HCAs/ports
          if (! (matchIfList(devices[d]->name, port, userIfs, nUserIfs, searchExact) ^ searchNot)) {
            continue;
          }
          TRACE(NCCL_INIT|NCCL_NET,"NET/IB: [%d] %s:%d/%s ", d, devices[d]->name, port,
              portAttr.link_layer == IBV_LINK_LAYER_INFINIBAND ? "IB" : "RoCE");
          pthread_mutex_init(&ncclIbDevs[ncclNIbDevs].lock, NULL);
          ncclIbDevs[ncclNIbDevs].device = d;
          ncclIbDevs[ncclNIbDevs].guid = devAttr.sys_image_guid;
          ncclIbDevs[ncclNIbDevs].port = port;
          ncclIbDevs[ncclNIbDevs].link = portAttr.link_layer;
          ncclIbDevs[ncclNIbDevs].speed = ncclIbSpeed(portAttr.active_speed) * ncclIbWidth(portAttr.active_width);
          ncclIbDevs[ncclNIbDevs].context = context;
          ncclIbDevs[ncclNIbDevs].pdRefs = 0;
          ncclIbDevs[ncclNIbDevs].pd = NULL;
          strncpy(ncclIbDevs[ncclNIbDevs].devName, devices[d]->name, MAXNAMESIZE);
          NCCLCHECK(ncclIbGetPciPath(ncclIbDevs[ncclNIbDevs].devName, &ncclIbDevs[ncclNIbDevs].pciPath, &ncclIbDevs[ncclNIbDevs].realPort));
          ncclIbDevs[ncclNIbDevs].maxQp = devAttr.max_qp;
          ncclIbDevs[ncclNIbDevs].mrCache.capacity = 0;
          ncclIbDevs[ncclNIbDevs].mrCache.population = 0;
          ncclIbDevs[ncclNIbDevs].mrCache.slots = NULL;

          pthread_create(&ncclIbAsyncThread, NULL, ncclIbAsyncThreadMain, context);
          ncclSetThreadName(ncclIbAsyncThread, "NCCL IbAsync %2d", ncclNIbDevs);
          pthread_detach(ncclIbAsyncThread); // will not be pthread_join()'d
          ncclNIbDevs++;
          nPorts++;
        }
        if (nPorts == 0 && ncclSuccess != wrap_ibv_close_device(context)) { return ncclInternalError; }
      }
      if (nIbDevs && (ncclSuccess != wrap_ibv_free_device_list(devices))) { return ncclInternalError; };
    }
    if (ncclNIbDevs == 0) {
      INFO(NCCL_INIT|NCCL_NET, "NET/IB : No device found.");
    } else {
      char line[1024];
      line[0] = '\0';
      // Determine whether RELAXED_ORDERING is enabled and possible
      ncclIbRelaxedOrderingEnabled = ncclIbRelaxedOrderingCapable();
      for (int d=0; d<ncclNIbDevs; d++) {
        snprintf(line+strlen(line), 1023-strlen(line), " [%d]%s:%d/%s", d, ncclIbDevs[d].devName,
            ncclIbDevs[d].port, ncclIbDevs[d].link == IBV_LINK_LAYER_INFINIBAND ? "IB" : "RoCE");
      }
      line[1023] = '\0';
      char addrline[SOCKET_NAME_MAXLEN+1];
      INFO(NCCL_INIT|NCCL_NET, "NET/IB : Using%s %s; OOB %s:%s", line, ncclIbRelaxedOrderingEnabled ? "[RO]" : "",
           ncclIbIfName, ncclSocketToString(&ncclIbIfAddr, addrline));
    }
    pthread_mutex_unlock(&ncclIbLock);
  }
  return ncclSuccess;
}
```

</details>

<a id="f1103"></a>

## F1103：ncclIbDevices

用途：返回当前已初始化并通过筛选的 IB/RoCE 数据端口数量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*ndev = ncclNIbDevs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *ndev = ncclNIbDevs;
  return ncclSuccess;
}
```

</details>

<a id="f1104"></a>

## F1104：ncclIbGdrSupport

用途：按设备能力检测能否注册 GPU 缓冲，给上层报告 GDR 支持状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>static int moduleLoaded = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(moduleLoaded == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>moduleLoaded = ((access("/sys/kernel/mm/memory_peers/nv_mem/version", F_OK) == -1) && (access("/sys/kernel/mm/memory_peers/nvidia-peermem/version", F_OK) == -1)) ? 0 : 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(moduleLoaded == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static int moduleLoaded = -1;
  if (moduleLoaded == -1) {
    // Check for the nv_peer_mem module being loaded
    moduleLoaded = ((access("/sys/kernel/mm/memory_peers/nv_mem/version", F_OK) == -1) &&
                    // Also support the new nvidia-peermem module
                    (access("/sys/kernel/mm/memory_peers/nvidia-peermem/version", F_OK) == -1)) ? 0 : 1;
  }
  if (moduleLoaded == 0) return ncclSystemError;
  return ncclSuccess;
}
```

</details>

<a id="f1105"></a>

## F1105：GetSocketAddr

用途：为所选后端设备获得建链控制 socket 地址；控制握手地址与实际数据载荷通路不同。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>memcpy(addr, &ncclIbIfAddr, sizeof(*addr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  memcpy(addr, &ncclIbIfAddr, sizeof(*addr));
  return ncclSuccess;
}
```

</details>

<a id="f1106"></a>

## F1106：ncclIbGetProperties

用途：把端口名、PCI 路径、速度、GDR 支持及接口接收能力报告给 NCCL 拓扑/NET 层。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;name = ncclIbDevs[dev].devName;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;pciPath = ncclIbDevs[dev].pciPath;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;guid = ncclIbDevs[dev].guid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;ptrSupport = NCCL_PTR_HOST;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(ncclIbGdrSupport(dev) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET,"NET/IB : GPU Direct RDMA Disabled for HCA %d '%s' (no module)", dev, ncclIbDevs[dev].devName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>props-&gt;ptrSupport &#124;= NCCL_PTR_CUDA;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;speed = ncclIbDevs[dev].speed;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;latency = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;port = ncclIbDevs[dev].port + ncclIbDevs[dev].realPort;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;maxComms = ncclIbDevs[dev].maxQp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;maxRecvs = NCCL_NET_IB_MAX_RECVS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  props->name = ncclIbDevs[dev].devName;
  props->pciPath = ncclIbDevs[dev].pciPath;
  props->guid = ncclIbDevs[dev].guid;
  props->ptrSupport = NCCL_PTR_HOST;
  if (ncclIbGdrSupport(dev) != ncclSuccess) {
    INFO(NCCL_NET,"NET/IB : GPU Direct RDMA Disabled for HCA %d '%s' (no module)", dev, ncclIbDevs[dev].devName);
  } else {
    props->ptrSupport |= NCCL_PTR_CUDA;
  }
  props->speed = ncclIbDevs[dev].speed;
  props->latency = 0; // Not set
  props->port = ncclIbDevs[dev].port + ncclIbDevs[dev].realPort;
  props->maxComms = ncclIbDevs[dev].maxQp;
  props->maxRecvs = NCCL_NET_IB_MAX_RECVS;
  return ncclSuccess;
}
```

</details>

<a id="f1107"></a>

## F1107：ncclIbInitVerbs

用途：为一个 backend 连接准备共享设备 PD 引用、CQ 与请求池，供其多个数据 QP 使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>verbs-&gt;dev = dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&ncclIbDevs[dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 条件判断 | <code>(0 == ncclIbDevs[dev].pdRefs++)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 准备局部状态 | <code>ncclResult_t res;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(wrap_ibv_alloc_pd(&ncclIbDevs[dev].pd, ctx), res, failure);</code> | 失败时保存错误并跳到清理标签，成功才继续；分配保护域，隔离允许访问的 QP/MR 集合；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 7 | 2 / 条件判断 | <code>(0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 4 / 跳转标签 | <code>failure</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 10 | 5 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclIbDevs[dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 4 / 函数返回 | <code>return res;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>verbs-&gt;pd = ncclIbDevs[dev].pd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclIbDevs[dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_create_cq(&verbs-&gt;cq, ctx, 2*MAX_REQUESTS*ncclParamIbQpsPerConn(), NULL, NULL, 0));</code> | 失败时可能提前return，成功才继续；创建完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  verbs->dev = dev;

  pthread_mutex_lock(&ncclIbDevs[dev].lock);
  if (0 == ncclIbDevs[dev].pdRefs++) {
    ncclResult_t res;
    NCCLCHECKGOTO(wrap_ibv_alloc_pd(&ncclIbDevs[dev].pd, ctx), res, failure);
    if (0) {
    failure:
      pthread_mutex_unlock(&ncclIbDevs[dev].lock);
      return res;
    }
  }
  verbs->pd = ncclIbDevs[dev].pd;
  pthread_mutex_unlock(&ncclIbDevs[dev].lock);

  // Recv requests can generate 2 completions (one for the post FIFO, one for the Recv).
  NCCLCHECK(wrap_ibv_create_cq(&verbs->cq, ctx, 2*MAX_REQUESTS*ncclParamIbQpsPerConn(), NULL, NULL, 0));
  return ncclSuccess;
}
```

</details>

<a id="f1108"></a>

## F1108：ncclIbDestroyVerbs

用途：销毁连接 CQ 等私有对象，并按引用计数释放设备共享的保护域。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclResult_t res;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_destroy_cq(verbs-&gt;cq));</code> | 失败时可能提前return，成功才继续；销毁完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&ncclIbDevs[verbs-&gt;dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 条件判断 | <code>(0 == --ncclIbDevs[verbs-&gt;dev].pdRefs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(wrap_ibv_dealloc_pd(ncclIbDevs[verbs-&gt;dev].pd), res, returning);</code> | 失败时保存错误并跳到清理标签，成功才继续；释放保护域；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>res = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 跳转标签 | <code>returning</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclIbDevs[verbs-&gt;dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return res;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclResult_t res;
  NCCLCHECK(wrap_ibv_destroy_cq(verbs->cq));

  pthread_mutex_lock(&ncclIbDevs[verbs->dev].lock);
  if (0 == --ncclIbDevs[verbs->dev].pdRefs) {
    NCCLCHECKGOTO(wrap_ibv_dealloc_pd(ncclIbDevs[verbs->dev].pd), res, returning);
  }
  res = ncclSuccess;
returning:
  pthread_mutex_unlock(&ncclIbDevs[verbs->dev].lock);
  return res;
}
```

</details>

<a id="f1109"></a>

## F1109：ncclIbCreateQp

用途：创建绑定本连接 CQ/PD 的 RC QP 并设到 INIT，配置队列容量和访问权限。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ibv_qp_init_attr qpInitAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>memset(&qpInitAttr, 0, sizeof(struct ibv_qp_init_attr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>qpInitAttr.send_cq = verbs-&gt;cq;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>qpInitAttr.recv_cq = verbs-&gt;cq;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>qpInitAttr.qp_type = IBV_QPT_RC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>qpInitAttr.cap.max_send_wr = 2*MAX_REQUESTS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>qpInitAttr.cap.max_recv_wr = MAX_REQUESTS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>qpInitAttr.cap.max_send_sge = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>qpInitAttr.cap.max_recv_sge = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>qpInitAttr.cap.max_inline_data = ncclParamIbUseInline() ? sizeof(struct ncclIbSendFifo) : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_create_qp(qp, verbs-&gt;pd, &qpInitAttr));</code> | 失败时可能提前return，成功才继续；创建指定类型和队列容量的 QP；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 12 | 0 / 准备局部状态 | <code>struct ibv_qp_attr qpAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.qp_state = IBV_QPS_INIT;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.pkey_index = ncclParamIbPkey();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.port_num = ib_port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.qp_access_flags = access_flags;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_modify_qp(*qp, &qpAttr, IBV_QP_STATE &#124; IBV_QP_PKEY_INDEX &#124; IBV_QP_PORT &#124; IBV_QP_ACCESS_FLAGS));</code> | 失败时可能提前return，成功才继续；按属性掩码修改 QP 状态/路由/权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ibv_qp_init_attr qpInitAttr;
  memset(&qpInitAttr, 0, sizeof(struct ibv_qp_init_attr));
  qpInitAttr.send_cq = verbs->cq;
  qpInitAttr.recv_cq = verbs->cq;
  qpInitAttr.qp_type = IBV_QPT_RC;
  // We might send 2 messages per send (RDMA and RDMA_WITH_IMM)
  qpInitAttr.cap.max_send_wr = 2*MAX_REQUESTS;
  qpInitAttr.cap.max_recv_wr = MAX_REQUESTS;
  qpInitAttr.cap.max_send_sge = 1;
  qpInitAttr.cap.max_recv_sge = 1;
  qpInitAttr.cap.max_inline_data = ncclParamIbUseInline() ? sizeof(struct ncclIbSendFifo) : 0;
  NCCLCHECK(wrap_ibv_create_qp(qp, verbs->pd, &qpInitAttr));
  struct ibv_qp_attr qpAttr;
  memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));
  qpAttr.qp_state = IBV_QPS_INIT;
  qpAttr.pkey_index = ncclParamIbPkey();
  qpAttr.port_num = ib_port;
  qpAttr.qp_access_flags = access_flags;
  NCCLCHECK(wrap_ibv_modify_qp(*qp, &qpAttr, IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS));
  return ncclSuccess;
}
```

</details>

<a id="f1110"></a>

## F1110：ncclIbRtrQp

用途：根据远端 QPN/LID/GID 等握手参数把 QP 切到 RTR，使接收方向就绪。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ibv_qp_attr qpAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.qp_state = IBV_QPS_RTR;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.path_mtu = info-&gt;mtu;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.dest_qp_num = qpn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.rq_psn = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.max_dest_rd_atomic = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.min_rnr_timer = 12;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 条件判断 | <code>(info-&gt;link_layer == IBV_LINK_LAYER_ETHERNET)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.is_global = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.grh.dgid.global.subnet_prefix = info-&gt;spn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.grh.dgid.global.interface_id = info-&gt;iid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.grh.flow_label = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.grh.sgid_index = ncclParamIbGidIndex();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.grh.hop_limit = 255;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.grh.traffic_class = ncclParamIbTc();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.is_global = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.dlid = info-&gt;lid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.sl = ncclParamIbSl();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.src_path_bits = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.ah_attr.port_num = info-&gt;ib_port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_modify_qp(qp, &qpAttr, IBV_QP_STATE &#124; IBV_QP_AV &#124; IBV_QP_PATH_MTU &#124; IBV_QP_DEST_QPN &#124; IBV_QP_RQ_PSN &#124; IBV_QP_MAX_DEST_RD_ATOMIC &#124; IBV_QP_MIN_RNR_TIMER));</code> | 失败时可能提前return，成功才继续；按属性掩码修改 QP 状态/路由/权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 25 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ibv_qp_attr qpAttr;
  memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));
  qpAttr.qp_state = IBV_QPS_RTR;
  qpAttr.path_mtu = info->mtu;
  qpAttr.dest_qp_num = qpn;
  qpAttr.rq_psn = 0;
  qpAttr.max_dest_rd_atomic = 1;
  qpAttr.min_rnr_timer = 12;
  if (info->link_layer == IBV_LINK_LAYER_ETHERNET) {
    qpAttr.ah_attr.is_global = 1;
    qpAttr.ah_attr.grh.dgid.global.subnet_prefix = info->spn;
    qpAttr.ah_attr.grh.dgid.global.interface_id = info->iid;
    qpAttr.ah_attr.grh.flow_label = 0;
    qpAttr.ah_attr.grh.sgid_index = ncclParamIbGidIndex();
    qpAttr.ah_attr.grh.hop_limit = 255;
    qpAttr.ah_attr.grh.traffic_class = ncclParamIbTc();
  } else {
    qpAttr.ah_attr.is_global = 0;
    qpAttr.ah_attr.dlid = info->lid;
  }
  qpAttr.ah_attr.sl = ncclParamIbSl();
  qpAttr.ah_attr.src_path_bits = 0;
  qpAttr.ah_attr.port_num = info->ib_port;
  NCCLCHECK(wrap_ibv_modify_qp(qp, &qpAttr, IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN | IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER));
  return ncclSuccess;
}
```

</details>

<a id="f1111"></a>

## F1111：ncclIbRtsQp

用途：配置超时、重试和发送序号等，把 QP 切到 RTS 允许发送工作请求。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ibv_qp_attr qpAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.qp_state = IBV_QPS_RTS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.timeout = ncclParamIbTimeout();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.retry_cnt = ncclParamIbRetryCnt();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.rnr_retry = 7;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.sq_psn = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>qpAttr.max_rd_atomic = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_modify_qp(qp, &qpAttr, IBV_QP_STATE &#124; IBV_QP_TIMEOUT &#124; IBV_QP_RETRY_CNT &#124; IBV_QP_RNR_RETRY &#124; IBV_QP_SQ_PSN &#124; IBV_QP_MAX_QP_RD_ATOMIC));</code> | 失败时可能提前return，成功才继续；按属性掩码修改 QP 状态/路由/权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ibv_qp_attr qpAttr;
  memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));
  qpAttr.qp_state = IBV_QPS_RTS;
  qpAttr.timeout = ncclParamIbTimeout();
  qpAttr.retry_cnt = ncclParamIbRetryCnt();
  qpAttr.rnr_retry = 7;
  qpAttr.sq_psn = 0;
  qpAttr.max_rd_atomic = 1;
  NCCLCHECK(wrap_ibv_modify_qp(qp, &qpAttr, IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC));
  return ncclSuccess;
}
```

</details>

<a id="f1112"></a>

## F1112：ncclIbListen

用途：建立接收方向的控制监听对象，将设备/地址编码到可交换句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbListenComm* comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&comm, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclIbHandle* handle = (struct ncclIbHandle*) opaqueHandle;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 源码语法片段 | <code>static_assert(sizeof(struct ncclIbHandle) &lt; NCCL_NET_HANDLE_MAXSIZE, "ncclIbHandle size too large");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>memset(handle, 0, sizeof(struct ncclIbHandle));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;dev = dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(GetSocketAddr(&comm-&gt;sock.addr));</code> | 失败时可能提前return，成功才继续；为所选后端设备获得建链控制 socket 地址；控制握手地址与实际数据载荷通路不同。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketListen(&comm-&gt;sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&handle-&gt;connectAddr, &comm-&gt;sock.addr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>*listenComm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbListenComm* comm;
  NCCLCHECK(ncclCalloc(&comm, 1));
  struct ncclIbHandle* handle = (struct ncclIbHandle*) opaqueHandle;
  static_assert(sizeof(struct ncclIbHandle) < NCCL_NET_HANDLE_MAXSIZE, "ncclIbHandle size too large");
  memset(handle, 0, sizeof(struct ncclIbHandle));
  comm->dev = dev;
  NCCLCHECK(GetSocketAddr(&comm->sock.addr));
  NCCLCHECK(ncclSocketListen(&comm->sock));
  memcpy(&handle->connectAddr, &comm->sock.addr, sizeof(union ncclSocketAddress));
  *listenComm = comm;
  return ncclSuccess;
}
```

</details>

<a id="f1113"></a>

## F1113：ncclIbConnect

用途：非阻塞推进发送端控制握手，创建数据 QP 和控制 FIFO MR，并交换远端连接信息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbHandle* handle = (struct ncclIbHandle*) opaqueHandle;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>enum ncclSocketState conState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclIbCommStage* stage = &handle-&gt;stage;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclIbSendComm* comm = (struct ncclIbSendComm*)stage-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>*sendComm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 条件判断 | <code>(stage-&gt;state == ncclIbCommStateConnect)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 控制转移 | <code>goto ib_connect_check;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 9 | 0 / 条件判断 | <code>(stage-&gt;state == ncclIbCommStateSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 控制转移 | <code>goto ib_send;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 12 | 0 / 条件判断 | <code>(stage-&gt;state != ncclIbCommStateStart)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>WARN("Error: trying to connect already connected sendComm");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbMalloc((void**)&comm, sizeof(struct ncclIbSendComm)));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketInit(&comm-&gt;sock, &handle-&gt;connectAddr, NULL, 1));</code> | 失败时可能提前return，成功才继续；本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclIbCommStateConnect;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketConnect(&comm-&gt;sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。 |
| 21 | 0 / 跳转标签 | <code>ib_connect_check</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 22 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGetSocketState(&comm-&gt;sock, &conState));</code> | 失败时可能提前return，成功才继续；检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。 |
| 23 | 0 / 条件判断 | <code>(conState == ncclSocketConnecting)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 27 | 2 / 条件判断 | <code>(conState == ncclSocketError)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 4 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 30 | 0 / 准备局部状态 | <code>struct ibv_context* ctx;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>ctx = ncclIbDevs[dev].context;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbInitVerbs(dev, ctx, &comm-&gt;verbs));</code> | 失败时可能提前return，成功才继续；为一个 backend 连接准备共享设备 PD 引用、CQ 与请求池，供其多个数据 QP 使用。 |
| 33 | 0 / 准备局部状态 | <code>uint8_t ib_port;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>ib_port = ncclIbDevs[dev].port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;nqps = ncclParamIbQpsPerConn();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 0 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 37 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbCreateQp(ib_port, &comm-&gt;verbs, IBV_ACCESS_REMOTE_WRITE, comm-&gt;qps+q));</code> | 失败时可能提前return，成功才继续；创建绑定本连接 CQ/PD 的 RC QP 并设到 INIT，配置队列容量和访问权限。 |
| 38 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 39 | 0 / 准备局部状态 | <code>struct ibv_port_attr portAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_query_port(ctx, ib_port, &portAttr));</code> | 失败时可能提前return，成功才继续；读取端口状态/地址与速率能力；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 41 | 0 / 准备局部状态 | <code>struct ncclIbQpInfo qpInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.ib_port = ib_port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 44 | 1 / 调用 / 计算 / 写入 | <code>qpInfo.qpn[q] = comm-&gt;qps[q]-&gt;qp_num;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.mtu = portAttr.active_mtu;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_reg_mr(&comm-&gt;fifoMr, comm-&gt;verbs.pd, comm-&gt;fifo, sizeof(struct ncclIbSendFifo)*MAX_REQUESTS*NCCL_NET_IB_MAX_RECVS, IBV_ACCESS_LOCAL_WRITE&#124;IBV_ACCESS_REMOTE_WRITE&#124;IBV_ACCESS_REMOTE_READ));</code> | 失败时可能提前return，成功才继续；把指定虚拟地址范围注册到 PD 并取得 lkey/rkey；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.fifoRkey = comm-&gt;fifoMr-&gt;rkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.fifoAddr = (uint64_t)comm-&gt;fifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.lid = portAttr.lid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.link_layer = portAttr.link_layer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 0 / 条件判断 | <code>(qpInfo.link_layer == IBV_LINK_LAYER_INFINIBAND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 2 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 55 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET,"NET/IB: Dev %d Port %d qpn %d mtu %d LID %d", dev, ib_port, qpInfo.qpn[q], qpInfo.mtu, qpInfo.lid);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 56 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 57 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 58 | 2 / 准备局部状态 | <code>union ibv_gid gid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_query_gid(ctx, ib_port, ncclParamIbGidIndex(), &gid));</code> | 失败时可能提前return，成功才继续；读取指定端口/GID索引的地址；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 60 | 2 / 调用 / 计算 / 写入 | <code>qpInfo.spn = gid.global.subnet_prefix;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 2 / 调用 / 计算 / 写入 | <code>qpInfo.iid = gid.global.interface_id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 2 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 63 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET,"NET/IB: Dev %d Port %d qpn %d mtu %d GID %ld (%lX/%lX)", dev, ib_port, qpInfo.qpn[q], qpInfo.mtu, ncclParamIbGidIndex(), qpInfo.spn, qpInfo.iid);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 64 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 65 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclIbCommStateSend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 66 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;offset = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbMalloc((void**)&stage-&gt;buffer, sizeof(qpInfo)));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 68 | 0 / 调用 / 计算 / 写入 | <code>memcpy(stage-&gt;buffer, &qpInfo, sizeof(qpInfo));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 69 | 0 / 跳转标签 | <code>ib_send</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 70 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, &comm-&gt;sock, stage-&gt;buffer, sizeof(qpInfo), &stage-&gt;offset));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 71 | 0 / 条件判断 | <code>(stage-&gt;offset != sizeof(qpInfo))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 72 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 73 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 74 | 0 / 调用 / 计算 / 写入 | <code>free(stage-&gt;buffer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 75 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclIbCommStateConnected;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 76 | 0 / 调用 / 计算 / 写入 | <code>*sendComm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 77 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbHandle* handle = (struct ncclIbHandle*) opaqueHandle;
  enum ncclSocketState conState;
  struct ncclIbCommStage* stage = &handle->stage;
  struct ncclIbSendComm* comm = (struct ncclIbSendComm*)stage->comm;
  *sendComm = NULL;

  if (stage->state == ncclIbCommStateConnect) goto ib_connect_check;
  if (stage->state == ncclIbCommStateSend) goto ib_send;
  if (stage->state != ncclIbCommStateStart) {
    WARN("Error: trying to connect already connected sendComm");
    return ncclInternalError;
  }

  NCCLCHECK(ncclIbMalloc((void**)&comm, sizeof(struct ncclIbSendComm)));
  NCCLCHECK(ncclSocketInit(&comm->sock, &handle->connectAddr, NULL, 1));
  stage->comm = comm;
  stage->state = ncclIbCommStateConnect;
  NCCLCHECK(ncclSocketConnect(&comm->sock));

ib_connect_check:
  /* since ncclSocketConnect is async, we must check if connection is complete */
  NCCLCHECK(ncclGetSocketState(&comm->sock, &conState));
  if (conState == ncclSocketConnecting) {
    /* expect user to call again */
    return ncclSuccess;
  } else if (conState == ncclSocketError) {
    return ncclSystemError;
  }

  // IB Setup
  struct ibv_context* ctx;
  ctx = ncclIbDevs[dev].context;
  NCCLCHECK(ncclIbInitVerbs(dev, ctx, &comm->verbs));
  uint8_t ib_port;
  ib_port = ncclIbDevs[dev].port;
  comm->nqps = ncclParamIbQpsPerConn();
  for (int q=0; q<comm->nqps; q++) {
    NCCLCHECK(ncclIbCreateQp(ib_port, &comm->verbs, IBV_ACCESS_REMOTE_WRITE, comm->qps+q));
  }

  // Send my QP Info to receiver through the socket. Hope this won't block.
  struct ibv_port_attr portAttr;
  NCCLCHECK(wrap_ibv_query_port(ctx, ib_port, &portAttr));
  struct ncclIbQpInfo qpInfo;
  qpInfo.ib_port = ib_port;
  for (int q=0; q<comm->nqps; q++) qpInfo.qpn[q] = comm->qps[q]->qp_num;
  qpInfo.mtu = portAttr.active_mtu;

  // Prepare my fifo
  NCCLCHECK(wrap_ibv_reg_mr(&comm->fifoMr, comm->verbs.pd, comm->fifo, sizeof(struct ncclIbSendFifo)*MAX_REQUESTS*NCCL_NET_IB_MAX_RECVS, IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ));
  qpInfo.fifoRkey = comm->fifoMr->rkey;
  qpInfo.fifoAddr = (uint64_t)comm->fifo;

  // RoCE support
  qpInfo.lid = portAttr.lid;
  qpInfo.link_layer = portAttr.link_layer;
  if (qpInfo.link_layer == IBV_LINK_LAYER_INFINIBAND) { // IB
    for (int q=0; q<comm->nqps; q++)
      INFO(NCCL_NET,"NET/IB: Dev %d Port %d qpn %d mtu %d LID %d", dev, ib_port, qpInfo.qpn[q], qpInfo.mtu, qpInfo.lid);
  } else { // RoCE
    union ibv_gid gid;
    NCCLCHECK(wrap_ibv_query_gid(ctx, ib_port, ncclParamIbGidIndex(), &gid));
    qpInfo.spn = gid.global.subnet_prefix;
    qpInfo.iid = gid.global.interface_id;
    for (int q=0; q<comm->nqps; q++)
      INFO(NCCL_NET,"NET/IB: Dev %d Port %d qpn %d mtu %d GID %ld (%lX/%lX)", dev, ib_port, qpInfo.qpn[q], qpInfo.mtu, ncclParamIbGidIndex(), qpInfo.spn, qpInfo.iid);
  }

  stage->state = ncclIbCommStateSend;
  stage->offset = 0;
  NCCLCHECK(ncclIbMalloc((void**)&stage->buffer, sizeof(qpInfo)));
  memcpy(stage->buffer, &qpInfo, sizeof(qpInfo));

ib_send:
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, &comm->sock, stage->buffer, sizeof(qpInfo), &stage->offset));
  if (stage->offset != sizeof(qpInfo))
    return ncclSuccess;

  free(stage->buffer);
  stage->state = ncclIbCommStateConnected;
  *sendComm = comm;
  return ncclSuccess;
}
```

</details>

<a id="f1114"></a>

## F1114：ncclIbAccept

用途：非阻塞推进接收端握手，建立数据 QP、控制区及满足条件时的 GDR flush 资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbListenComm* lComm = (struct ncclIbListenComm*)listenComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclIbCommStage* stage = &lComm-&gt;stage;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclIbRecvComm* rComm = (struct ncclIbRecvComm*)stage-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>*recvComm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(stage-&gt;state == ncclIbCommStateAccept)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 控制转移 | <code>goto ib_accept;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 8 | 0 / 条件判断 | <code>(stage-&gt;state == ncclIbCommStateRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 控制转移 | <code>goto ib_recv;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 11 | 0 / 条件判断 | <code>(stage-&gt;state == ncclIbCommStateSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 控制转移 | <code>goto ib_send;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 14 | 0 / 条件判断 | <code>(stage-&gt;state != ncclIbCommStateStart)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>WARN("Listencomm in unknown state %d\n", stage-&gt;state);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbMalloc((void**)&rComm, sizeof(struct ncclIbRecvComm)));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;comm = rComm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclIbCommStateAccept;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>lComm-&gt;sock.asyncFlag = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;sock.asyncFlag = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 跳转标签 | <code>ib_accept</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketAccept(&rComm-&gt;sock, &lComm-&gt;sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。 |
| 25 | 0 / 条件判断 | <code>(rComm-&gt;sock.fd == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 28 | 0 / 准备局部状态 | <code>struct ncclIbQpInfo remQpInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclIbCommStateRecv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;offset = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbMalloc((void**)&stage-&gt;buffer, sizeof(remQpInfo)));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 32 | 0 / 跳转标签 | <code>ib_recv</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 33 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &rComm-&gt;sock, stage-&gt;buffer, sizeof(remQpInfo), &stage-&gt;offset));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 34 | 0 / 条件判断 | <code>(stage-&gt;offset != sizeof(remQpInfo))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&remQpInfo, stage-&gt;buffer, sizeof(struct ncclIbQpInfo));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 0 / 准备局部状态 | <code>struct ibv_context* ctx;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 39 | 0 / 准备局部状态 | <code>uint8_t ib_port;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>ctx = ncclIbDevs[lComm-&gt;dev].context;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>ib_port = ncclIbDevs[lComm-&gt;dev].port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 准备局部状态 | <code>struct ibv_port_attr portAttr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_query_port(ctx, ib_port, &portAttr));</code> | 失败时可能提前return，成功才继续；读取端口状态/地址与速率能力；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 44 | 0 / 准备局部状态 | <code>union ibv_gid gid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_query_gid(ctx, ib_port, ncclParamIbGidIndex(), &gid));</code> | 失败时可能提前return，成功才继续；读取指定端口/GID索引的地址；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbInitVerbs(lComm-&gt;dev, ctx, &rComm-&gt;verbs));</code> | 失败时可能提前return，成功才继续；为一个 backend 连接准备共享设备 PD 引用、CQ 与请求池，供其多个数据 QP 使用。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;nqps = ncclParamIbQpsPerConn();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 循环 | <code>初始 int q=0;；继续条件 q&lt;rComm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 49 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbCreateQp(ib_port, &rComm-&gt;verbs, IBV_ACCESS_REMOTE_WRITE, rComm-&gt;qps+q));</code> | 失败时可能提前return，成功才继续；创建绑定本连接 CQ/PD 的 RC QP 并设到 INIT，配置队列容量和访问权限。 |
| 50 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 51 | 0 / 调用 / 计算 / 写入 | <code>remQpInfo.mtu = (enum ibv_mtu)std::min(remQpInfo.mtu, portAttr.active_mtu);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 52 | 0 / 循环 | <code>初始 int q=0;；继续条件 q&lt;rComm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 53 | 1 / 准备局部状态 | <code>struct ibv_qp* qp = rComm-&gt;qps[q];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbRtrQp(qp, remQpInfo.qpn[q], &remQpInfo));</code> | 失败时可能提前return，成功才继续；根据远端 QPN/LID/GID 等握手参数把 QP 切到 RTR，使接收方向就绪。 |
| 55 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbRtsQp(qp));</code> | 失败时可能提前return，成功才继续；配置超时、重试和发送序号等，把 QP 切到 RTS 允许发送工作请求。 |
| 56 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 57 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;remFifo.rkey = remQpInfo.fifoRkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;remFifo.addr = remQpInfo.fifoAddr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 59 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_reg_mr(&rComm-&gt;remFifo.mr, rComm-&gt;verbs.pd, &rComm-&gt;remFifo.elems, sizeof(struct ncclIbSendFifo)*MAX_REQUESTS*NCCL_NET_IB_MAX_RECVS, IBV_ACCESS_REMOTE_WRITE&#124;IBV_ACCESS_LOCAL_WRITE&#124;IBV_ACCESS_REMOTE_READ));</code> | 失败时可能提前return，成功才继续；把指定虚拟地址范围注册到 PD 并取得 lkey/rkey；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 60 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;remFifo.sge.lkey = rComm-&gt;remFifo.mr-&gt;lkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 0 / 条件判断 | <code>(ncclParamIbUseInline())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 62 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 63 | 2 / 调用 / 计算 / 写入 | <code>rComm-&gt;remFifo.flags = IBV_SEND_INLINE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 64 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;gpuFlush.enabled = (ncclIbGdrSupport(lComm-&gt;dev) == 0) && (ncclParamIbGdrFlushDisable() == 0) ? 1 : 0;</code> | 按设备能力检测能否注册 GPU 缓冲，给上层报告 GDR 支持状态。 |
| 65 | 0 / 条件判断 | <code>(rComm-&gt;gpuFlush.enabled)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 66 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 67 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_reg_mr(&rComm-&gt;gpuFlush.hostMr, rComm-&gt;verbs.pd, &rComm-&gt;gpuFlush.hostMem, sizeof(int), IBV_ACCESS_LOCAL_WRITE));</code> | 失败时可能提前return，成功才继续；把指定虚拟地址范围注册到 PD 并取得 lkey/rkey；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 68 | 2 / 调用 / 计算 / 写入 | <code>rComm-&gt;gpuFlush.sge.addr = (uint64_t)&rComm-&gt;gpuFlush.hostMem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 69 | 2 / 调用 / 计算 / 写入 | <code>rComm-&gt;gpuFlush.sge.length = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 2 / 调用 / 计算 / 写入 | <code>rComm-&gt;gpuFlush.sge.lkey = rComm-&gt;gpuFlush.hostMr-&gt;lkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 71 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbCreateQp(ib_port, &rComm-&gt;verbs, IBV_ACCESS_LOCAL_WRITE &#124; IBV_ACCESS_REMOTE_READ, &rComm-&gt;gpuFlush.qp));</code> | 失败时可能提前return，成功才继续；创建绑定本连接 CQ/PD 的 RC QP 并设到 INIT，配置队列容量和访问权限。 |
| 72 | 2 / 准备局部状态 | <code>struct ncclIbQpInfo localQpInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 73 | 2 / 调用 / 计算 / 写入 | <code>localQpInfo.lid=portAttr.lid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 74 | 2 / 调用 / 计算 / 写入 | <code>localQpInfo.link_layer=portAttr.link_layer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 75 | 2 / 调用 / 计算 / 写入 | <code>localQpInfo.ib_port=ib_port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 76 | 2 / 调用 / 计算 / 写入 | <code>localQpInfo.spn=gid.global.subnet_prefix;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 77 | 2 / 调用 / 计算 / 写入 | <code>localQpInfo.iid=gid.global.interface_id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 2 / 调用 / 计算 / 写入 | <code>localQpInfo.mtu=portAttr.active_mtu;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbRtrQp(rComm-&gt;gpuFlush.qp, rComm-&gt;gpuFlush.qp-&gt;qp_num, &localQpInfo));</code> | 失败时可能提前return，成功才继续；根据远端 QPN/LID/GID 等握手参数把 QP 切到 RTR，使接收方向就绪。 |
| 80 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbRtsQp(rComm-&gt;gpuFlush.qp));</code> | 失败时可能提前return，成功才继续；配置超时、重试和发送序号等，把 QP 切到 RTS 允许发送工作请求。 |
| 81 | 0 / 准备局部状态 | <code>struct ncclIbQpInfo qpInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 82 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.lid=portAttr.lid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.link_layer=portAttr.link_layer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.ib_port=ib_port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 0 / 循环 | <code>初始 int q=0;；继续条件 q&lt;rComm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 86 | 1 / 调用 / 计算 / 写入 | <code>qpInfo.qpn[q]=rComm-&gt;qps[q]-&gt;qp_num;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 87 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 88 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.spn=gid.global.subnet_prefix;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 89 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.iid=gid.global.interface_id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 90 | 0 / 调用 / 计算 / 写入 | <code>qpInfo.mtu=remQpInfo.mtu;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 91 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclIbCommStateSend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 92 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;offset = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 93 | 0 / 条件判断 | <code>(stage-&gt;buffer)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 94 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 95 | 2 / 调用 / 计算 / 写入 | <code>free(stage-&gt;buffer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 96 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbMalloc((void**)&stage-&gt;buffer, sizeof(struct ncclIbQpInfo)));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 97 | 0 / 调用 / 计算 / 写入 | <code>memcpy(stage-&gt;buffer, &qpInfo, sizeof(struct ncclIbQpInfo));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 98 | 0 / 跳转标签 | <code>ib_send</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 99 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, &rComm-&gt;sock, stage-&gt;buffer, sizeof(struct ncclIbQpInfo), &stage-&gt;offset));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 100 | 0 / 条件判断 | <code>(stage-&gt;offset &lt; sizeof(struct ncclIbQpInfo))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 101 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 102 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 103 | 0 / 调用 / 计算 / 写入 | <code>free(stage-&gt;buffer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 104 | 0 / 调用 / 计算 / 写入 | <code>*recvComm = rComm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 105 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclIbCommStateStart;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 106 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;offset = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 107 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;comm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 108 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;buffer = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 109 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbListenComm* lComm = (struct ncclIbListenComm*)listenComm;
  struct ncclIbCommStage* stage = &lComm->stage;
  struct ncclIbRecvComm* rComm = (struct ncclIbRecvComm*)stage->comm;
  *recvComm = NULL;

  if (stage->state == ncclIbCommStateAccept) goto ib_accept;
  if (stage->state == ncclIbCommStateRecv) goto ib_recv;
  if (stage->state == ncclIbCommStateSend) goto ib_send;
  if (stage->state != ncclIbCommStateStart) {
    WARN("Listencomm in unknown state %d\n", stage->state);
    return ncclInternalError;
  }

  NCCLCHECK(ncclIbMalloc((void**)&rComm, sizeof(struct ncclIbRecvComm)));
  stage->comm = rComm;
  stage->state = ncclIbCommStateAccept;
  lComm->sock.asyncFlag = 1;
  rComm->sock.asyncFlag = 1;

ib_accept:
  NCCLCHECK(ncclSocketAccept(&rComm->sock, &lComm->sock));
  if (rComm->sock.fd == -1)
    return ncclSuccess;

  struct ncclIbQpInfo remQpInfo;
  stage->state = ncclIbCommStateRecv;
  stage->offset = 0;
  NCCLCHECK(ncclIbMalloc((void**)&stage->buffer, sizeof(remQpInfo)));
ib_recv:
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &rComm->sock, stage->buffer, sizeof(remQpInfo), &stage->offset));
  if (stage->offset != sizeof(remQpInfo))
    return ncclSuccess;

  /* copy back the received info */
  memcpy(&remQpInfo, stage->buffer, sizeof(struct ncclIbQpInfo));

  // IB setup
  struct ibv_context* ctx;
  uint8_t ib_port;
  ctx = ncclIbDevs[lComm->dev].context;
  ib_port = ncclIbDevs[lComm->dev].port;
  struct ibv_port_attr portAttr;
  NCCLCHECK(wrap_ibv_query_port(ctx, ib_port, &portAttr));
  union ibv_gid gid;
  NCCLCHECK(wrap_ibv_query_gid(ctx, ib_port, ncclParamIbGidIndex(), &gid));

  // QP Creation
  NCCLCHECK(ncclIbInitVerbs(lComm->dev, ctx, &rComm->verbs));
  rComm->nqps = ncclParamIbQpsPerConn();
  for (int q=0; q<rComm->nqps; q++) {
    NCCLCHECK(ncclIbCreateQp(ib_port, &rComm->verbs, IBV_ACCESS_REMOTE_WRITE, rComm->qps+q));
  }

  // Adjust the MTU
  remQpInfo.mtu = (enum ibv_mtu)std::min(remQpInfo.mtu, portAttr.active_mtu);

  // Setup QP
  for (int q=0; q<rComm->nqps; q++) {
    struct ibv_qp* qp = rComm->qps[q];
    NCCLCHECK(ncclIbRtrQp(qp, remQpInfo.qpn[q], &remQpInfo));
    NCCLCHECK(ncclIbRtsQp(qp));
  }

  // Retain remote fifo info and prepare my RDMA ops
  rComm->remFifo.rkey = remQpInfo.fifoRkey;
  rComm->remFifo.addr = remQpInfo.fifoAddr;
  NCCLCHECK(wrap_ibv_reg_mr(&rComm->remFifo.mr, rComm->verbs.pd, &rComm->remFifo.elems, sizeof(struct ncclIbSendFifo)*MAX_REQUESTS*NCCL_NET_IB_MAX_RECVS, IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_READ));
  rComm->remFifo.sge.lkey = rComm->remFifo.mr->lkey;
  if (ncclParamIbUseInline()) rComm->remFifo.flags = IBV_SEND_INLINE;

/* 中文导读：
 * 条件启用的额外硬件资源：每个这样的 RecvComm 增加 1 个自连接 RC QP 和 1 个小 host MR。
 * 该 QP 用 RDMA_READ 协助 GPU 内存可见性，不是又一条跨机数据连接，数量也不随 nqps 倍增。
 */
  // Allocate Flush dummy buffer for GPU Direct RDMA
  rComm->gpuFlush.enabled = (ncclIbGdrSupport(lComm->dev) == 0) && (ncclParamIbGdrFlushDisable() == 0) ? 1 : 0;
  if (rComm->gpuFlush.enabled) {
    NCCLCHECK(wrap_ibv_reg_mr(&rComm->gpuFlush.hostMr, rComm->verbs.pd, &rComm->gpuFlush.hostMem, sizeof(int), IBV_ACCESS_LOCAL_WRITE));
    rComm->gpuFlush.sge.addr = (uint64_t)&rComm->gpuFlush.hostMem;
    rComm->gpuFlush.sge.length = 1;
    rComm->gpuFlush.sge.lkey = rComm->gpuFlush.hostMr->lkey;
    NCCLCHECK(ncclIbCreateQp(ib_port, &rComm->verbs, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ, &rComm->gpuFlush.qp));
    struct ncclIbQpInfo localQpInfo;
    localQpInfo.lid=portAttr.lid;
    localQpInfo.link_layer=portAttr.link_layer;
    localQpInfo.ib_port=ib_port;
    localQpInfo.spn=gid.global.subnet_prefix;
    localQpInfo.iid=gid.global.interface_id;
    localQpInfo.mtu=portAttr.active_mtu;
    NCCLCHECK(ncclIbRtrQp(rComm->gpuFlush.qp, rComm->gpuFlush.qp->qp_num, &localQpInfo));
    NCCLCHECK(ncclIbRtsQp(rComm->gpuFlush.qp));
  }

  // Fill Handle
  struct ncclIbQpInfo qpInfo;
  qpInfo.lid=portAttr.lid;
  qpInfo.link_layer=portAttr.link_layer;
  qpInfo.ib_port=ib_port;
  for (int q=0; q<rComm->nqps; q++) qpInfo.qpn[q]=rComm->qps[q]->qp_num;
  qpInfo.spn=gid.global.subnet_prefix;
  qpInfo.iid=gid.global.interface_id;
  qpInfo.mtu=remQpInfo.mtu;

  stage->state = ncclIbCommStateSend;
  stage->offset = 0;
  if (stage->buffer) free(stage->buffer);
  NCCLCHECK(ncclIbMalloc((void**)&stage->buffer, sizeof(struct ncclIbQpInfo)));
  memcpy(stage->buffer, &qpInfo, sizeof(struct ncclIbQpInfo));
ib_send:
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, &rComm->sock, stage->buffer, sizeof(struct ncclIbQpInfo), &stage->offset));
  if (stage->offset < sizeof(struct ncclIbQpInfo)) return ncclSuccess;

  free(stage->buffer);
  *recvComm = rComm;

  /* reset lComm stage */
  stage->state = ncclIbCommStateStart;
  stage->offset = 0;
  stage->comm = NULL;
  stage->buffer = NULL;
  return ncclSuccess;
}
```

</details>

<a id="f1115"></a>

## F1115：ncclIbGetRequest

用途：从固定请求池取空闲项并初始化类型/事件计数，避免每次网络请求动态 malloc。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAX_REQUESTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclIbRequest* r = verbs-&gt;reqs+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 条件判断 | <code>(r-&gt;type == NCCL_NET_IB_REQ_UNUSED)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;verbs = verbs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;events = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;addr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>*req = r;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : unable to allocate requests");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>*req = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int i=0; i<MAX_REQUESTS; i++) {
    struct ncclIbRequest* r = verbs->reqs+i;
    if (r->type == NCCL_NET_IB_REQ_UNUSED) {
      r->verbs = verbs;
      r->events = 1;
      r->addr = NULL;
      *req = r;
      return ncclSuccess;
    }
  }
  WARN("NET/IB : unable to allocate requests");
  *req = NULL;
  return ncclInternalError;
}
```

</details>

<a id="f1116"></a>

## F1116：ncclIbFreeRequest

用途：把已完成请求槽标记为可复用，只有其关联完成事件耗尽后才能调用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>r-&gt;type = NCCL_NET_IB_REQ_UNUSED;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  r->type = NCCL_NET_IB_REQ_UNUSED;
  return ncclSuccess;
}
```

</details>

<a id="f1117"></a>

## F1117：ncclSendCheck

用途：推进/验证握手后的 ready 状态，确保后续网络请求不会使用尚未完整连接的资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbQpInfo remQpInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int bytes = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &comm-&gt;sock, &remQpInfo, sizeof(remQpInfo), &bytes));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 4 | 0 / 条件判断 | <code>(bytes == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, &comm-&gt;sock, &remQpInfo, sizeof(remQpInfo), &bytes));</code> | 失败时可能提前return，成功才继续；重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。 |
| 8 | 0 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 准备局部状态 | <code>struct ibv_qp* qp = comm-&gt;qps[q];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbRtrQp(qp, remQpInfo.qpn[q], &remQpInfo));</code> | 失败时可能提前return，成功才继续；根据远端 QPN/LID/GID 等握手参数把 QP 切到 RTR，使接收方向就绪。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbRtsQp(qp));</code> | 失败时可能提前return，成功才继续；配置超时、重试和发送序号等，把 QP 切到 RTS 允许发送工作请求。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;ready = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(&comm-&gt;sock, &comm-&gt;ready, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbQpInfo remQpInfo;

  // Do not block on this receive, return if not ready.
  int bytes = 0;
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &comm->sock, &remQpInfo, sizeof(remQpInfo), &bytes));
  if (bytes == 0) return ncclSuccess; // Try again later
  NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, &comm->sock, &remQpInfo, sizeof(remQpInfo), &bytes));

  for (int q=0; q<comm->nqps; q++) {
    struct ibv_qp* qp = comm->qps[q];
    NCCLCHECK(ncclIbRtrQp(qp, remQpInfo.qpn[q], &remQpInfo));
    NCCLCHECK(ncclIbRtsQp(qp));
  }
  comm->ready = 1;
  // Block until this is done. It *should* not block indefinitely.
  NCCLCHECK(ncclSocketSend(&comm->sock, &comm->ready, sizeof(int)));

  return ncclSuccess;
}
```

</details>

<a id="f1118"></a>

## F1118：ncclRecvCheck

用途：推进/验证握手后的 ready 状态，确保后续网络请求不会使用尚未完整连接的资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int bytes = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &comm-&gt;sock, &comm-&gt;ready, sizeof(int), &bytes));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 3 | 0 / 条件判断 | <code>(bytes == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, &comm-&gt;sock, &comm-&gt;ready, sizeof(int), &bytes));</code> | 失败时可能提前return，成功才继续；重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Do not block on this receive, return if not ready.
  int bytes = 0;
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &comm->sock, &comm->ready, sizeof(int), &bytes));
  if (bytes == 0) return ncclSuccess; // Try again later
  NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, &comm->sock, &comm->ready, sizeof(int), &bytes));
  return ncclSuccess;
}
```

</details>

<a id="f1119"></a>

## F1119：ncclIbRegMr

用途：按页对齐范围查 MR 缓存，命中增加引用，否则向 verbs 注册缓冲并保存键值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>static_assert(offsetof(struct ncclIbSendComm, verbs) == offsetof(struct ncclIbRecvComm, verbs), "Send and recv comms must have verbs at the same offset");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>assert(size &gt; 0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 准备局部状态 | <code>static __thread uintptr_t pageSize = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(pageSize == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>pageSize = sysconf(_SC_PAGESIZE);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 准备局部状态 | <code>struct ncclIbVerbs* verbs = (struct ncclIbVerbs*)comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>struct ncclIbMrCache* cache = &ncclIbDevs[verbs-&gt;dev].mrCache;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>uintptr_t addr = (uintptr_t)data & -pageSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>int pages = ((uintptr_t)data + size - addr + pageSize-1)/pageSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>ncclResult_t res;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&ncclIbDevs[verbs-&gt;dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 循环 | <code>初始 int slot=0;；继续条件 见原语法；每轮更新 slot++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 1 / 条件判断 | <code>(slot == cache-&gt;population)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 3 / 条件判断 | <code>(cache-&gt;population == cache-&gt;capacity)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 5 / 调用 / 计算 / 写入 | <code>cache-&gt;capacity = cache-&gt;capacity &lt; 32 ? 32 : 2*cache-&gt;capacity;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclRealloc(&cache-&gt;slots, cache-&gt;population, cache-&gt;capacity), res, returning);</code> | 失败时保存错误并跳到清理标签，成功才继续；分配更大且尾部清零的 CPU 数组，复制旧元素后释放旧内存，不支持缩小。 |
| 20 | 3 / 准备局部状态 | <code>struct ibv_mr* mr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 3 / 准备局部状态 | <code>unsigned int flags = IBV_ACCESS_LOCAL_WRITE&#124;IBV_ACCESS_REMOTE_WRITE&#124;IBV_ACCESS_REMOTE_READ;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 3 / 条件判断 | <code>(ncclIbRelaxedOrderingEnabled)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(wrap_ibv_reg_mr_iova2(&mr, verbs-&gt;pd, (void*)addr, pages*pageSize, (uintptr_t)addr, flags&#124;IBV_ACCESS_RELAXED_ORDERING), res, returning);</code> | 失败时保存错误并跳到清理标签，成功才继续；注册内存并指定 HCA 使用的 IOVA 地址及访问权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 25 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 26 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(wrap_ibv_reg_mr(&mr, verbs-&gt;pd, (void*)addr, pages*pageSize, flags), res, returning);</code> | 失败时保存错误并跳到清理标签，成功才继续；把指定虚拟地址范围注册到 PD 并取得 lkey/rkey；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT,"regAddr %llx size %lld rkey %x", (unsigned long long)addr, (long long)pages*pageSize, mr-&gt;rkey);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>cache-&gt;population += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 3 / 调用 / 计算 / 写入 | <code>cache-&gt;slots[slot].addr = addr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>cache-&gt;slots[slot].pages = pages;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 3 / 调用 / 计算 / 写入 | <code>cache-&gt;slots[slot].refs = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 3 / 调用 / 计算 / 写入 | <code>cache-&gt;slots[slot].mr = mr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 3 / 调用 / 计算 / 写入 | <code>*mhandle = (void*)mr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 3 / 调用 / 计算 / 写入 | <code>res = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 3 / 控制转移 | <code>goto returning;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 36 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 37 | 3 / 条件判断 | <code>(cache-&gt;slots[slot].addr == addr && cache-&gt;slots[slot].pages == pages)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 5 / 调用 / 计算 / 写入 | <code>cache-&gt;slots[slot].refs += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 5 / 调用 / 计算 / 写入 | <code>*mhandle = (void*)cache-&gt;slots[slot].mr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 5 / 调用 / 计算 / 写入 | <code>res = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 5 / 控制转移 | <code>goto returning;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 43 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 44 | 0 / 跳转标签 | <code>returning</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 45 | 1 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclIbDevs[verbs-&gt;dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 0 / 函数返回 | <code>return res;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static_assert(offsetof(struct ncclIbSendComm, verbs) == offsetof(struct ncclIbRecvComm, verbs), "Send and recv comms must have verbs at the same offset");
  assert(size > 0);

  static __thread uintptr_t pageSize = 0;
  if (pageSize == 0) pageSize = sysconf(_SC_PAGESIZE);

  struct ncclIbVerbs* verbs = (struct ncclIbVerbs*)comm;
  struct ncclIbMrCache* cache = &ncclIbDevs[verbs->dev].mrCache;
  uintptr_t addr = (uintptr_t)data & -pageSize;
  int pages = ((uintptr_t)data + size - addr + pageSize-1)/pageSize;
  ncclResult_t res;
  pthread_mutex_lock(&ncclIbDevs[verbs->dev].lock);
  for (int slot=0; /*true*/; slot++) {
    if (slot == cache->population) { // didn't find in cache
      if (cache->population == cache->capacity) { // must grow cache
        cache->capacity = cache->capacity < 32 ? 32 : 2*cache->capacity;
        NCCLCHECKGOTO(ncclRealloc(&cache->slots, cache->population, cache->capacity), res, returning);
      }
      // Deregister / register
      struct ibv_mr* mr;
      unsigned int flags = IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ;
      if (ncclIbRelaxedOrderingEnabled) {
        // Use IBVERBS_1.8 API - needed for IBV_ACCESS_RELAXED_ORDERING support
        NCCLCHECKGOTO(wrap_ibv_reg_mr_iova2(&mr, verbs->pd, (void*)addr, pages*pageSize, (uintptr_t)addr, flags|IBV_ACCESS_RELAXED_ORDERING), res, returning);
      }
      else {
        NCCLCHECKGOTO(wrap_ibv_reg_mr(&mr, verbs->pd, (void*)addr, pages*pageSize, flags), res, returning);
      }
      TRACE(NCCL_INIT,"regAddr %llx size %lld rkey %x", (unsigned long long)addr, (long long)pages*pageSize, mr->rkey);
      cache->population += 1;
      cache->slots[slot].addr = addr;
      cache->slots[slot].pages = pages;
      cache->slots[slot].refs = 1;
      cache->slots[slot].mr = mr;
      *mhandle = (void*)mr;
      res = ncclSuccess;
      goto returning;
    }
    else if (cache->slots[slot].addr == addr && cache->slots[slot].pages == pages) {
      cache->slots[slot].refs += 1;
      *mhandle = (void*)cache->slots[slot].mr;
      res = ncclSuccess;
      goto returning;
    }
  }
returning:
  pthread_mutex_unlock(&ncclIbDevs[verbs->dev].lock);
  return res;
}
```

</details>

<a id="f1120"></a>

## F1120：ncclIbDeregMr

用途：按缓存引用计数释放注册，最后一个引用消失时真正 dereg_mr 并移除缓存项。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbVerbs* verbs = (struct ncclIbVerbs*)comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclIbMrCache* cache = &ncclIbDevs[verbs-&gt;dev].mrCache;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>ncclResult_t res;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&ncclIbDevs[verbs-&gt;dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; cache-&gt;population；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 条件判断 | <code>(mhandle == cache-&gt;slots[i].mr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 条件判断 | <code>(0 == --cache-&gt;slots[i].refs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 5 / 调用 / 计算 / 写入 | <code>memmove(&cache-&gt;slots[i], &cache-&gt;slots[--cache-&gt;population], sizeof(struct ncclIbMr));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 5 / 条件判断 | <code>(cache-&gt;population == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 7 / 调用 / 计算 / 写入 | <code>free(cache-&gt;slots);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 7 / 调用 / 计算 / 写入 | <code>cache-&gt;slots = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 7 / 调用 / 计算 / 写入 | <code>cache-&gt;capacity = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(wrap_ibv_dereg_mr((struct ibv_mr*)mhandle), res, returning);</code> | 失败时保存错误并跳到清理标签，成功才继续；注销一个真实 MR；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>res = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 3 / 控制转移 | <code>goto returning;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 19 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>WARN("NET/IB: could not find mr %p inside cache of %d entries", mhandle, cache-&gt;population);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>res = ncclInternalError;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 跳转标签 | <code>returning</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclIbDevs[verbs-&gt;dev].lock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 0 / 函数返回 | <code>return res;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbVerbs* verbs = (struct ncclIbVerbs*)comm;
  struct ncclIbMrCache* cache = &ncclIbDevs[verbs->dev].mrCache;
  ncclResult_t res;
  pthread_mutex_lock(&ncclIbDevs[verbs->dev].lock);
  for (int i=0; i < cache->population; i++) {
    if (mhandle == cache->slots[i].mr) {
      if (0 == --cache->slots[i].refs) {
        memmove(&cache->slots[i], &cache->slots[--cache->population], sizeof(struct ncclIbMr));
        if (cache->population == 0) {
          free(cache->slots);
          cache->slots = NULL;
          cache->capacity = 0;
        }
        NCCLCHECKGOTO(wrap_ibv_dereg_mr((struct ibv_mr*)mhandle), res, returning);
      }
      res = ncclSuccess;
      goto returning;
    }
  }
  WARN("NET/IB: could not find mr %p inside cache of %d entries", mhandle, cache->population);
  res = ncclInternalError;
returning:
  pthread_mutex_unlock(&ncclIbDevs[verbs->dev].lock);
  return res;
}
```

</details>

<a id="f1121"></a>

## F1121：ncclIbMultiSend

用途：把匹配接收的一个或多个发送请求分到数据 QP，组织 RDMA_WRITE/WRITE_WITH_IMM 链和完成标识。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbRequest** reqs = comm-&gt;fifoReqs[slot];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>volatile struct ncclIbSendFifo* slots = comm-&gt;fifo[slot];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int nreqs = slots[0].nreqs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(nreqs &gt; NCCL_NET_IB_MAX_RECVS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>uint64_t wr_id = 0ULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nreqs；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 准备局部状态 | <code>struct ibv_send_wr* wr = comm-&gt;wrs+r;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>memset(wr, 0, sizeof(struct ibv_send_wr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 1 / 准备局部状态 | <code>struct ibv_sge* sge = comm-&gt;sges+r;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>sge-&gt;addr=(uintptr_t)reqs[r]-&gt;send.data;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>sge-&gt;lkey=reqs[r]-&gt;send.lkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>wr-&gt;opcode = IBV_WR_RDMA_WRITE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>wr-&gt;send_flags = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 1 / 调用 / 计算 / 写入 | <code>wr-&gt;wr.rdma.remote_addr = slots[r].addr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 调用 / 计算 / 写入 | <code>wr-&gt;wr.rdma.rkey = slots[r].rkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 1 / 调用 / 计算 / 写入 | <code>wr-&gt;next = wr+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 1 / 调用 / 计算 / 写入 | <code>wr_id += (reqs[r] - comm-&gt;verbs.reqs) &lt;&lt; (r*8);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 0 / 准备局部状态 | <code>uint64_t immData = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 0 / 条件判断 | <code>(nreqs == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>immData = reqs[0]-&gt;send.size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 26 | 2 / 准备局部状态 | <code>uint8_t* multiImmData = (uint8_t*)&immData;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 2 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nreqs；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>multiImmData[r] = reqs[r]-&gt;send.size ? 1 : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 30 | 0 / 准备局部状态 | <code>struct ibv_send_wr* lastWr = comm-&gt;wrs+nreqs-1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 0 / 条件判断 | <code>(nreqs &gt; 1 &#124;&#124; reqs[0]-&gt;send.size &gt; ncclParamIbArThreshold())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>lastWr++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>memset(lastWr, 0, sizeof(struct ibv_send_wr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>lastWr-&gt;wr_id = wr_id;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>lastWr-&gt;opcode = IBV_WR_RDMA_WRITE_WITH_IMM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>lastWr-&gt;imm_data = immData;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>lastWr-&gt;next = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>lastWr-&gt;send_flags = IBV_SEND_SIGNALED;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 0 / 准备局部状态 | <code>const int align = 128;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 0 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 42 | 1 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nreqs；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 43 | 2 / 准备局部状态 | <code>int chunkSize = DIVUP(DIVUP(reqs[r]-&gt;send.size, comm-&gt;nqps), align) * align;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 2 / 准备局部状态 | <code>int length = std::min(reqs[r]-&gt;send.size-reqs[r]-&gt;send.offset, chunkSize);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 45 | 2 / 条件判断 | <code>(length &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;wrs[r].sg_list = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;wrs[r].num_sge = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 50 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;sges[r].length = length;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;wrs[r].sg_list = comm-&gt;sges+r;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 4 / 调用 / 计算 / 写入 | <code>comm-&gt;wrs[r].num_sge = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 54 | 1 / 准备局部状态 | <code>struct ibv_send_wr* bad_wr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 55 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_post_send(comm-&gt;qps[q], comm-&gt;wrs, &bad_wr));</code> | 失败时可能提前return，成功才继续；通过 QP 的 provider 操作表提交发送工作请求链，立即返回仅表示提交，不代表载荷完成。 |
| 56 | 1 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nreqs；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 57 | 2 / 准备局部状态 | <code>int chunkSize = DIVUP(DIVUP(reqs[r]-&gt;send.size, comm-&gt;nqps), align) * align;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 58 | 2 / 调用 / 计算 / 写入 | <code>reqs[r]-&gt;send.offset += chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 59 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;sges[r].addr += chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;wrs[r].wr.rdma.remote_addr += chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 62 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 63 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbRequest** reqs = comm->fifoReqs[slot];
  volatile struct ncclIbSendFifo* slots = comm->fifo[slot];
  int nreqs = slots[0].nreqs;
  if (nreqs > NCCL_NET_IB_MAX_RECVS) return ncclInternalError;

  uint64_t wr_id = 0ULL;

  for (int r=0; r<nreqs; r++) {
    struct ibv_send_wr* wr = comm->wrs+r;
    memset(wr, 0, sizeof(struct ibv_send_wr));

    struct ibv_sge* sge = comm->sges+r;
    sge->addr=(uintptr_t)reqs[r]->send.data;
    sge->lkey=reqs[r]->send.lkey;

    wr->opcode = IBV_WR_RDMA_WRITE;
    wr->send_flags = 0;
    wr->wr.rdma.remote_addr = slots[r].addr;
    wr->wr.rdma.rkey = slots[r].rkey;
    wr->next = wr+1;
    wr_id += (reqs[r] - comm->verbs.reqs) << (r*8);
  }

  // Write size as immediate data. In the case of multi-send, only write
  // 0 or 1 as size to indicate whether there was data sent or received.
  uint64_t immData = 0;
  if (nreqs == 1) {
    immData = reqs[0]->send.size;
  } else {
    uint8_t* multiImmData = (uint8_t*)&immData;
    for (int r=0; r<nreqs; r++) {
      multiImmData[r] = reqs[r]->send.size ? 1 : 0;
    }
  }

  struct ibv_send_wr* lastWr = comm->wrs+nreqs-1;
  if (nreqs > 1 || reqs[0]->send.size > ncclParamIbArThreshold()) {
    // When using adaptive routing, send the bulk of the data first as an
    // RDMA_WRITE, then a 0-byte RDMA_WRITE_WITH_IMM to trigger a remote
    // completion.
    lastWr++;
    memset(lastWr, 0, sizeof(struct ibv_send_wr));
  }
  lastWr->wr_id = wr_id;
  lastWr->opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
  lastWr->imm_data = immData;
  lastWr->next = NULL;
  lastWr->send_flags = IBV_SEND_SIGNALED;

  // Multi-QP: make sure IB writes are multiples of 128B so that LL and LL128 protocols still work
  const int align = 128;
  for (int q=0; q<comm->nqps; q++) {
    for (int r=0; r<nreqs; r++) {
      int chunkSize = DIVUP(DIVUP(reqs[r]->send.size, comm->nqps), align) * align;
      int length = std::min(reqs[r]->send.size-reqs[r]->send.offset, chunkSize);
      if (length <= 0) {
        comm->wrs[r].sg_list = NULL;
        comm->wrs[r].num_sge = 0;
      } else {
        comm->sges[r].length = length;
        comm->wrs[r].sg_list = comm->sges+r;
        comm->wrs[r].num_sge = 1;
      }
    }
    struct ibv_send_wr* bad_wr;
    NCCLCHECK(wrap_ibv_post_send(comm->qps[q], comm->wrs, &bad_wr));

    for (int r=0; r<nreqs; r++) {
      int chunkSize = DIVUP(DIVUP(reqs[r]->send.size, comm->nqps), align) * align;
      reqs[r]->send.offset += chunkSize;
      comm->sges[r].addr += chunkSize;
      comm->wrs[r].wr.rdma.remote_addr += chunkSize;
    }
  }

  return ncclSuccess;
}
```

</details>

<a id="f1122"></a>

## F1122：ncclIbIsend

用途：先等待/匹配远端 FIFO 中的地址、rkey、tag 与容量，再把可发送项交给 MultiSend。

- 先检查发送连接就绪，再按接收方发布FIFO的序号/tag/容量匹配可用目标。
- 匹配不足时可返回成功但request为空，调用者下轮继续；有完整匹配后才提交RDMA工作链。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbSendComm* comm = (struct ncclIbSendComm*)sendComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm-&gt;ready == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSendCheck(comm));</code> | 失败时可能提前return，成功才继续；推进/验证握手后的 ready 状态，确保后续网络请求不会使用尚未完整连接的资源。 |
| 5 | 0 / 条件判断 | <code>(comm-&gt;ready == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>*request = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 准备局部状态 | <code>struct ibv_mr* mr = (struct ibv_mr*)mhandle;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>int nreqs = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>volatile struct ncclIbSendFifo* slots;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>int slot = (comm-&gt;fifoHead)%MAX_REQUESTS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>struct ncclIbRequest** reqs = comm-&gt;fifoReqs[slot];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>slots = comm-&gt;fifo[slot];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 准备局部状态 | <code>int idx = comm-&gt;fifoHead+1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 条件判断 | <code>(slots[0].idx != idx)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>*request = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>nreqs = slots[0].nreqs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 循环 | <code>初始 int r=1;；继续条件 r&lt;nreqs；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 1 / 循环 | <code>初始 ；继续条件 (slots[r].idx != idx)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 24 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 25 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>__sync_synchronize();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nreqs；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 28 | 1 / 条件判断 | <code>(reqs[r] != NULL &#124;&#124; slots[r].tag != tag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 31 | 1 / 条件判断 | <code>(size &gt; slots[r].size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 3 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 3 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : req %d/%d tag %x peer %s collective mismatch error, local size %d remote size %d", r, nreqs, tag, ncclSocketToString(&comm-&gt;sock.addr, line), size, slots[r].size);</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 35 | 3 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 36 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 37 | 3 / 条件判断 | <code>(slots[r].size &lt; 0 &#124;&#124; slots[r].addr == 0 &#124;&#124; slots[r].rkey == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 5 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 5 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : req %d/%d tag %x peer %s posted incorrect receive info: size %d addr %lx rkey %x", r, nreqs, tag, ncclSocketToString(&comm-&gt;sock.addr, line), slots[r].size, slots[r].addr, slots[r].rkey);</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 41 | 5 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 42 | 1 / 准备局部状态 | <code>struct ncclIbRequest* req;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbGetRequest(&comm-&gt;verbs, &req));</code> | 失败时可能提前return，成功才继续；从固定请求池取空闲项并初始化类型/事件计数，避免每次网络请求动态 malloc。 |
| 44 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;type = NCCL_NET_IB_REQ_SEND;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;addr = &comm-&gt;sock.addr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;verbs = &comm-&gt;verbs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;nreqs = nreqs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;send.size = size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;send.data = data;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;send.lkey = mr-&gt;lkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;send.offset = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;addr = &comm-&gt;sock.addr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;events = comm-&gt;nqps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 1 / 调用 / 计算 / 写入 | <code>*request = reqs[r] = req;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 1 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nreqs；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 56 | 2 / 条件判断 | <code>(reqs[r] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 59 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 60 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 61 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbMultiSend(comm, slot));</code> | 失败时可能提前return，成功才继续；把匹配接收的一个或多个发送请求分到数据 QP，组织 RDMA_WRITE/WRITE_WITH_IMM 链和完成标识。 |
| 62 | 1 / 调用 / 计算 / 写入 | <code>memset((void*)slots, 0, sizeof(struct ncclIbSendFifo));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 63 | 1 / 调用 / 计算 / 写入 | <code>memset(reqs, 0, NCCL_NET_IB_MAX_RECVS*sizeof(struct ncclIbRequest*));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 64 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;fifoHead++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 65 | 1 / 调用 / 计算 / 写入 | <code>TIME_STOP(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 66 | 1 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 67 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 68 | 0 / 调用 / 计算 / 写入 | <code>*request = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 69 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbSendComm* comm = (struct ncclIbSendComm*)sendComm;
  if (comm->ready == 0) NCCLCHECK(ncclSendCheck(comm));
  if (comm->ready == 0) { *request = NULL; return ncclSuccess; }

  struct ibv_mr* mr = (struct ibv_mr*)mhandle;

  // Wait for the receiver to have posted the corresponding receive
  int nreqs = 0;
  volatile struct ncclIbSendFifo* slots;

  int slot = (comm->fifoHead)%MAX_REQUESTS;
  struct ncclIbRequest** reqs = comm->fifoReqs[slot];
  slots = comm->fifo[slot];
  int idx = comm->fifoHead+1;
  if (slots[0].idx != idx) { *request = NULL; return ncclSuccess; }
  nreqs = slots[0].nreqs;
  // Wait until all data has arrived
  for (int r=1; r<nreqs; r++) while(slots[r].idx != idx);
  __sync_synchronize(); // order the nreqsPtr load against tag/rkey/addr loads below
  for (int r=0; r<nreqs; r++) {
    if (reqs[r] != NULL || slots[r].tag != tag) continue;

    // Sanity checks to catch user collective call count/size mismatches
    if (size > slots[r].size) {
      char line[SOCKET_NAME_MAXLEN+1];
      WARN("NET/IB : req %d/%d tag %x peer %s collective mismatch error, local size %d remote size %d",
           r, nreqs, tag, ncclSocketToString(&comm->sock.addr, line), size, slots[r].size);
      return ncclInvalidUsage;
    } // plus any potential programming errors
    else if (slots[r].size < 0 || slots[r].addr == 0 || slots[r].rkey == 0) {
     char line[SOCKET_NAME_MAXLEN+1];
     WARN("NET/IB : req %d/%d tag %x peer %s posted incorrect receive info: size %d addr %lx rkey %x",
          r, nreqs, tag, ncclSocketToString(&comm->sock.addr, line), slots[r].size, slots[r].addr, slots[r].rkey);
      return ncclInternalError;
    }
    struct ncclIbRequest* req;
    NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));
    req->type = NCCL_NET_IB_REQ_SEND;
    req->addr = &comm->sock.addr;
    req->verbs = &comm->verbs;
    req->nreqs = nreqs;
    req->send.size = size;
    req->send.data = data;
    req->send.lkey = mr->lkey;
    req->send.offset = 0;
    req->addr = &comm->sock.addr;
    req->events = comm->nqps;
    *request = reqs[r] = req;

    // If this is a multi-recv, send only when all requests have matched.
    for (int r=0; r<nreqs; r++) {
      if (reqs[r] == NULL) return ncclSuccess;
    }

    TIME_START(0);
    NCCLCHECK(ncclIbMultiSend(comm, slot));

    // Clear slots[0]->nreqs, as well as other fields to help debugging and sanity checks
    memset((void*)slots, 0, sizeof(struct ncclIbSendFifo));
    memset(reqs, 0, NCCL_NET_IB_MAX_RECVS*sizeof(struct ncclIbRequest*));
    comm->fifoHead++;
    TIME_STOP(0);
    return ncclSuccess;
  }

  *request = NULL;
  return ncclSuccess;
}
```

</details>

<a id="f1123"></a>

## F1123：ncclIbPostFifo

用途：把接收缓冲地址/rkey/尺寸/tag/序号 RDMA 写回发送端，授予本轮写入目标并维持周期 signaled 完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ibv_send_wr wr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>memset(&wr, 0, sizeof(wr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 准备局部状态 | <code>int slot = comm-&gt;remFifo.fifoTail%MAX_REQUESTS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclIbSendFifo* localElem = comm-&gt;remFifo.elems[slot];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;n；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>localElem[i].addr = (uint64_t)data[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 1 / 准备局部状态 | <code>struct ibv_mr* mr = (struct ibv_mr*)mhandles[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>localElem[i].rkey = mr-&gt;rkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>localElem[i].nreqs = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>localElem[i].size = sizes[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>localElem[i].tag = tags[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>localElem[i].idx = comm-&gt;remFifo.fifoTail+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>wr.wr.rdma.remote_addr = comm-&gt;remFifo.addr + slot*NCCL_NET_IB_MAX_RECVS*sizeof(struct ncclIbSendFifo);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>wr.wr.rdma.rkey = comm-&gt;remFifo.rkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;remFifo.sge.addr = (uint64_t)localElem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;remFifo.sge.length = n*sizeof(struct ncclIbSendFifo);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>wr.sg_list = &comm-&gt;remFifo.sge;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>wr.num_sge = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>wr.opcode = IBV_WR_RDMA_WRITE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>wr.send_flags = comm-&gt;remFifo.flags;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 条件判断 | <code>(slot == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>wr.send_flags &#124;= IBV_SEND_SIGNALED;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>wr.wr_id = req - comm-&gt;verbs.reqs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>req-&gt;events++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 准备局部状态 | <code>struct ibv_send_wr* bad_wr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_post_send(comm-&gt;qps[0], &wr, &bad_wr));</code> | 失败时可能提前return，成功才继续；通过 QP 的 provider 操作表提交发送工作请求链，立即返回仅表示提交，不代表载荷完成。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;remFifo.fifoTail++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ibv_send_wr wr;
  memset(&wr, 0, sizeof(wr));

  int slot = comm->remFifo.fifoTail%MAX_REQUESTS;
  struct ncclIbSendFifo* localElem = comm->remFifo.elems[slot];

  for (int i=0; i<n; i++) {
    localElem[i].addr = (uint64_t)data[i];
    struct ibv_mr* mr = (struct ibv_mr*)mhandles[i];
    localElem[i].rkey = mr->rkey;
    localElem[i].nreqs = n;
    localElem[i].size = sizes[i]; // Sanity/Debugging
    localElem[i].tag = tags[i];
    localElem[i].idx = comm->remFifo.fifoTail+1;
  }

  wr.wr.rdma.remote_addr = comm->remFifo.addr + slot*NCCL_NET_IB_MAX_RECVS*sizeof(struct ncclIbSendFifo);
  wr.wr.rdma.rkey = comm->remFifo.rkey;
  comm->remFifo.sge.addr = (uint64_t)localElem;
  comm->remFifo.sge.length = n*sizeof(struct ncclIbSendFifo);
  wr.sg_list = &comm->remFifo.sge;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_RDMA_WRITE;
  wr.send_flags = comm->remFifo.flags; // IBV_SEND_INLINE

  // We need to occasionally post a request with the IBV_SEND_SIGNALED flag, otherwise
  // the send queue will never empty.
  //
  // From https://www.rdmamojo.com/2014/06/30/working-unsignaled-completions/
  // "How to use Unsignaled Completion?" / "Gotchas and Pitfalls"
  // All posted Send Requested, Signaled and Unsignaled, are considered outstanding until
  // a Work Completion that they, or Send Requests that were posted after them, was polled
  // from the Completion Queue associated with the Send Queue. This means if one works with
  // a Queue Pair that was configured to work with Unsignaled Completions, he must make
  // sure that occasionally (before the Send Queue is full with outstanding Send Requests)
  // a Send Request that generate Work Completion will be posted.
  //
  // Not following this rule may lead to a case that the Send Queue is full with Send
  // Requests that won't generate Work Completion:
  //
  //  - The Send Queue is full, so no new Send Requests can be posted to it
  //  - The Send Queue can't be emptied, since no Work Completion can be generated anymore
  //    (the reason is that no Work Completion, that can generate Work Completion that
  //    polling it will empty the Send Queue, can be posted)
  //  - The status of all posted Send Request is considered unknown
  //
  if (slot == 0) {
    wr.send_flags |= IBV_SEND_SIGNALED;
    wr.wr_id = req - comm->verbs.reqs;
    req->events++;
  }

  struct ibv_send_wr* bad_wr;
  NCCLCHECK(wrap_ibv_post_send(comm->qps[0], &wr, &bad_wr));
  comm->remFifo.fifoTail++;

  return ncclSuccess;
}
```

</details>

<a id="f1124"></a>

## F1124：ncclIbIrecv

用途：在每个数据 QP 挂零 SGE 接收 WQE以接收立即数通知，再发布本轮接收目标地址。

- 先为每个数据QP投递零SGE接收WQE以接收立即数完成通知。
- 载荷本身由RDMA写入目标MR；随后发布地址/rkey/tag，允许远端安全提交写入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm-&gt;ready == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclRecvCheck(comm));</code> | 失败时可能提前return，成功才继续；推进/验证握手后的 ready 状态，确保后续网络请求不会使用尚未完整连接的资源。 |
| 5 | 0 / 条件判断 | <code>(comm-&gt;ready == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>*request = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 条件判断 | <code>(n &gt; NCCL_NET_IB_MAX_RECVS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 准备局部状态 | <code>struct ncclIbRequest* req;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbGetRequest(&comm-&gt;verbs, &req));</code> | 失败时可能提前return，成功才继续；从固定请求池取空闲项并初始化类型/事件计数，避免每次网络请求动态 malloc。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>req-&gt;type = NCCL_NET_IB_REQ_RECV;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>req-&gt;addr = &comm-&gt;sock.addr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>req-&gt;nreqs = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;n；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 18 | 1 / 调用 / 计算 / 写入 | <code>req-&gt;recv.sizes[i] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 20 | 0 / 准备局部状态 | <code>struct ibv_recv_wr wr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>memset(&wr, 0, sizeof(wr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>wr.wr_id = req - comm-&gt;verbs.reqs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>wr.sg_list = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>wr.num_sge = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>TIME_START(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 0 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 1 / 准备局部状态 | <code>struct ibv_qp* qp = comm-&gt;qps[q];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 1 / 准备局部状态 | <code>struct ibv_recv_wr* bad_wr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_post_recv(qp, &wr, &bad_wr));</code> | 失败时可能提前return，成功才继续；向指定 QP 的接收队列投递 WQE，用于 IB 接收通知；该接口不自动变成 SRQ。 |
| 30 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>TIME_STOP(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>req-&gt;events = comm-&gt;nqps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>*request = req;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>TIME_START(2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbPostFifo(comm, n, data, sizes, tags, mhandles, req));</code> | 失败时可能提前return，成功才继续；把接收缓冲地址/rkey/尺寸/tag/序号 RDMA 写回发送端，授予本轮写入目标并维持周期 signaled 完成。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>TIME_STOP(2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 37 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;
  if (comm->ready == 0) NCCLCHECK(ncclRecvCheck(comm));
  if (comm->ready == 0) { *request = NULL; return ncclSuccess; }
  if (n > NCCL_NET_IB_MAX_RECVS) return ncclInternalError;

  struct ncclIbRequest* req;
  NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));
  req->type = NCCL_NET_IB_REQ_RECV;
  req->addr = &comm->sock.addr;
  req->nreqs = n;
  for (int i=0; i<n; i++) req->recv.sizes[i] = 0;

  struct ibv_recv_wr wr;
  memset(&wr, 0, sizeof(wr));
  wr.wr_id = req - comm->verbs.reqs;

  wr.sg_list = NULL;
  wr.num_sge = 0;

  TIME_START(1);
  for (int q=0; q<comm->nqps; q++) {
    struct ibv_qp* qp = comm->qps[q];
    struct ibv_recv_wr* bad_wr;
    NCCLCHECK(wrap_ibv_post_recv(qp, &wr, &bad_wr));
  }
  TIME_STOP(1);
  req->events = comm->nqps;

  *request = req;

  // Post to FIFO to notify sender
  TIME_START(2);
  NCCLCHECK(ncclIbPostFifo(comm, n, data, sizes, tags, mhandles, req));
  TIME_STOP(2);
  return ncclSuccess;
}
```

</details>

<a id="f1125"></a>

## F1125：ncclIbIflush

用途：需要 GDR 可见性保障时，对目标 GPU 缓冲发小型自连接 RDMA read 并返回可 test 的请求。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int last = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;n；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(sizes[i])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>last = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 条件判断 | <code>(comm-&gt;gpuFlush.enabled == 0 &#124;&#124; last == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 准备局部状态 | <code>struct ncclIbRequest* req;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbGetRequest(&comm-&gt;verbs, &req));</code> | 失败时可能提前return，成功才继续；从固定请求池取空闲项并初始化类型/事件计数，避免每次网络请求动态 malloc。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>req-&gt;type = NCCL_NET_IB_REQ_FLUSH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>req-&gt;addr = &comm-&gt;sock.addr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 准备局部状态 | <code>struct ibv_mr* mr = (struct ibv_mr*)mhandles[last];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 准备局部状态 | <code>struct ibv_send_wr wr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>memset(&wr, 0, sizeof(wr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>wr.wr_id = req - comm-&gt;verbs.reqs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>wr.wr.rdma.remote_addr = (uint64_t)data[last];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>wr.wr.rdma.rkey = mr-&gt;rkey;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>wr.sg_list = &comm-&gt;gpuFlush.sge;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>wr.num_sge = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>wr.opcode = IBV_WR_RDMA_READ;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>wr.send_flags = IBV_SEND_SIGNALED;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>TIME_START(4);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 0 / 准备局部状态 | <code>struct ibv_send_wr* bad_wr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_post_send(comm-&gt;gpuFlush.qp, &wr, &bad_wr));</code> | 失败时可能提前return，成功才继续；通过 QP 的 provider 操作表提交发送工作请求链，立即返回仅表示提交，不代表载荷完成。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>TIME_STOP(4);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>*request = req;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;
  int last = -1;
  for (int i=0; i<n; i++) if (sizes[i]) last = i;
  if (comm->gpuFlush.enabled == 0 || last == -1) return ncclSuccess;

  // Only flush once using the last non-zero receive
  struct ncclIbRequest* req;
  NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));
  req->type = NCCL_NET_IB_REQ_FLUSH;
  req->addr = &comm->sock.addr;
  struct ibv_mr* mr = (struct ibv_mr*)mhandles[last];

  struct ibv_send_wr wr;
  memset(&wr, 0, sizeof(wr));
  wr.wr_id = req - comm->verbs.reqs;

  wr.wr.rdma.remote_addr = (uint64_t)data[last];
  wr.wr.rdma.rkey = mr->rkey;
  wr.sg_list = &comm->gpuFlush.sge;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_RDMA_READ;
  wr.send_flags = IBV_SEND_SIGNALED;

  TIME_START(4);
  struct ibv_send_wr* bad_wr;
  NCCLCHECK(wrap_ibv_post_send(comm->gpuFlush.qp, &wr, &bad_wr));
  TIME_STOP(4);

  *request = req;
  return ncclSuccess;
}
```

</details>

<a id="f1126"></a>

## F1126：ncclIbTest

用途：轮询 CQ 并按 wr_id 找回请求递减完成事件，只有全部事件完成才返回 done 和接收大小。

- 当请求events为零才宣布done并回收槽；否则poll同一CQ的若干完成。
- 完成可能属于其他在途请求，必须按wr_id解码找到真正对象递减事件，不能把一项WC当整消息完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbRequest *r = (struct ncclIbRequest*)request;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>*done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 3 | 0 / 循环 | <code>初始 ；继续条件 (1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(r-&gt;events == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>*done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 7 | 3 / 条件判断 | <code>(sizes && r-&gt;type == NCCL_NET_IB_REQ_RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 5 / 循环 | <code>初始 int i=0;；继续条件 i&lt;r-&gt;nreqs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 6 / 调用 / 计算 / 写入 | <code>sizes[i] = r-&gt;recv.sizes[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbFreeRequest(r));</code> | 失败时可能提前return，成功才继续；把已完成请求槽标记为可复用，只有其关联完成事件耗尽后才能调用。 |
| 13 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 1 / 准备局部状态 | <code>int wrDone = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 1 / 准备局部状态 | <code>struct ibv_wc wcs[4];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(3);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_poll_cq(r-&gt;verbs-&gt;cq, 4, wcs, &wrDone));</code> | 失败时可能提前return，成功才继续；从 CQ 轮询完成项并统一错误处理，返回数量可为零而不代表失败。 |
| 18 | 1 / 条件判断 | <code>(wrDone == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>TIME_CANCEL(3);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 22 | 3 / 调用 / 计算 / 写入 | <code>TIME_STOP(3);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 1 / 条件判断 | <code>(wrDone == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 1 / 循环 | <code>初始 int w=0;；继续条件 w&lt;wrDone；每轮更新 w++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 2 / 准备局部状态 | <code>struct ibv_wc *wc = wcs+w;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 2 / 条件判断 | <code>(wc-&gt;status != IBV_WC_SUCCESS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 4 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 4 / 调用 / 计算 / 写入 | <code>WARN("NET/IB : Got completion from peer %s with error %d, opcode %d, len %d, vendor err %d", ncclSocketToString(r-&gt;addr, line), wc-&gt;status, wc-&gt;opcode, wc-&gt;byte_len, wc-&gt;vendor_err);</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 32 | 4 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 33 | 2 / 准备局部状态 | <code>struct ncclIbRequest* req = r-&gt;verbs-&gt;reqs+(wc-&gt;wr_id & 0xff);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 2 / 条件判断 | <code>(req-&gt;type == NCCL_NET_IB_REQ_SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 4 / 循环 | <code>初始 int i=0;；继续条件 i&lt;req-&gt;nreqs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 37 | 5 / 准备局部状态 | <code>struct ncclIbRequest* sendReq = r-&gt;verbs-&gt;reqs+((wc-&gt;wr_id &gt;&gt; (i*8)) & 0xff);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 5 / 条件判断 | <code>((sendReq-&gt;events &lt;= 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 7 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 41 | 5 / 调用 / 计算 / 写入 | <code>sendReq-&gt;events--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 43 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 44 | 4 / 条件判断 | <code>(req && wc-&gt;opcode == IBV_WC_RECV_RDMA_WITH_IMM)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 6 / 条件判断 | <code>(req-&gt;type != NCCL_NET_IB_REQ_RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 8 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 49 | 6 / 条件判断 | <code>(req-&gt;nreqs &gt; 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 8 / 准备局部状态 | <code>uint8_t* sizes = (uint8_t*)&wc-&gt;imm_data;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 8 / 循环 | <code>初始 int i=0;；继续条件 i&lt;req-&gt;nreqs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 53 | 9 / 调用 / 计算 / 写入 | <code>req-&gt;recv.sizes[i] &#124;= sizes[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 8 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 55 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 56 | 8 / 调用 / 计算 / 写入 | <code>req-&gt;recv.sizes[0] += wc-&gt;imm_data;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 4 / 调用 / 计算 / 写入 | <code>req-&gt;events--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 59 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbRequest *r = (struct ncclIbRequest*)request;
  *done = 0;

  while (1) {
    if (r->events == 0) {
      *done = 1;
      if (sizes && r->type == NCCL_NET_IB_REQ_RECV) {
        for (int i=0; i<r->nreqs; i++) sizes[i] = r->recv.sizes[i];
      }
      NCCLCHECK(ncclIbFreeRequest(r));
      return ncclSuccess;
    }

    int wrDone = 0;
    struct ibv_wc wcs[4];
    TIME_START(3);
    NCCLCHECK(wrap_ibv_poll_cq(r->verbs->cq, 4, wcs, &wrDone));
    if (wrDone == 0) { TIME_CANCEL(3); } else { TIME_STOP(3); }
    if (wrDone == 0) return ncclSuccess;

    for (int w=0; w<wrDone; w++) {
      struct ibv_wc *wc = wcs+w;
      if (wc->status != IBV_WC_SUCCESS) {
        char line[SOCKET_NAME_MAXLEN+1];
        WARN("NET/IB : Got completion from peer %s with error %d, opcode %d, len %d, vendor err %d",
             ncclSocketToString(r->addr, line), wc->status, wc->opcode, wc->byte_len, wc->vendor_err);
        return ncclSystemError;
      }

      struct ncclIbRequest* req = r->verbs->reqs+(wc->wr_id & 0xff);
      if (req->type == NCCL_NET_IB_REQ_SEND) {
        for (int i=0; i<req->nreqs; i++) {
          struct ncclIbRequest* sendReq = r->verbs->reqs+((wc->wr_id >> (i*8)) & 0xff);
          if ((sendReq->events <= 0)) return ncclInternalError;
          sendReq->events--;
        }
      } else {
        if (req && wc->opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
          if (req->type != NCCL_NET_IB_REQ_RECV) return ncclInternalError;
          if (req->nreqs > 1) {
            // In the case of a multi recv, we only set sizes to 0 or 1.
            uint8_t* sizes = (uint8_t*)&wc->imm_data;
            for (int i=0; i<req->nreqs; i++) {
              req->recv.sizes[i] |= sizes[i];
            }
          } else {
            req->recv.sizes[0] += wc->imm_data;
          }
        }
        req->events--;
      }
    }
  }
}
```

</details>

<a id="f1127"></a>

## F1127：ncclIbCloseSend

用途：关闭该方向的 IB 控制 socket、数据 QP、控制 MR及相关 verbs/flush 对象，按持有资源清理。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbSendComm* comm = (struct ncclIbSendComm*)sendComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>close(comm-&gt;sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 3 / 条件判断 | <code>(comm-&gt;qps[q] != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_destroy_qp(comm-&gt;qps[q]));</code> | 失败时可能提前return，成功才继续；销毁队列对；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 9 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 2 / 条件判断 | <code>(comm-&gt;fifoMr != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_dereg_mr(comm-&gt;fifoMr));</code> | 失败时可能提前return，成功才继续；注销一个真实 MR；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbDestroyVerbs(&comm-&gt;verbs));</code> | 失败时可能提前return，成功才继续；销毁连接 CQ 等私有对象，并按引用计数释放设备共享的保护域。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>free(comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>TIME_PRINT("IB");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbSendComm* comm = (struct ncclIbSendComm*)sendComm;
  if (comm) {
    close(comm->sock.fd);
    for (int q=0; q<comm->nqps; q++)
      if (comm->qps[q] != NULL) NCCLCHECK(wrap_ibv_destroy_qp(comm->qps[q]));
    if (comm->fifoMr != NULL) NCCLCHECK(wrap_ibv_dereg_mr(comm->fifoMr));
    NCCLCHECK(ncclIbDestroyVerbs(&comm->verbs));
    free(comm);
  }
  TIME_PRINT("IB");
  return ncclSuccess;
}
```

</details>

<a id="f1128"></a>

## F1128：ncclIbCloseRecv

用途：关闭该方向的 IB 控制 socket、数据 QP、控制 MR及相关 verbs/flush 对象，按持有资源清理。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>close(comm-&gt;sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 循环 | <code>初始 int q=0;；继续条件 q&lt;comm-&gt;nqps；每轮更新 q++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 3 / 条件判断 | <code>(comm-&gt;qps[q] != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_destroy_qp(comm-&gt;qps[q]));</code> | 失败时可能提前return，成功才继续；销毁队列对；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 9 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 2 / 条件判断 | <code>(comm-&gt;gpuFlush.enabled)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 4 / 条件判断 | <code>(comm-&gt;gpuFlush.qp != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_destroy_qp(comm-&gt;gpuFlush.qp));</code> | 失败时可能提前return，成功才继续；销毁队列对；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 15 | 4 / 条件判断 | <code>(comm-&gt;gpuFlush.hostMr != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_dereg_mr(comm-&gt;gpuFlush.hostMr));</code> | 失败时可能提前return，成功才继续；注销一个真实 MR；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 18 | 2 / 条件判断 | <code>(comm-&gt;remFifo.mr != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_ibv_dereg_mr(comm-&gt;remFifo.mr));</code> | 失败时可能提前return，成功才继续；注销一个真实 MR；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclIbDestroyVerbs(&comm-&gt;verbs));</code> | 失败时可能提前return，成功才继续；销毁连接 CQ 等私有对象，并按引用计数释放设备共享的保护域。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>free(comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;
  if (comm) {
    close(comm->sock.fd);
    for (int q=0; q<comm->nqps; q++)
      if (comm->qps[q] != NULL) NCCLCHECK(wrap_ibv_destroy_qp(comm->qps[q]));
    if (comm->gpuFlush.enabled) {
      if (comm->gpuFlush.qp != NULL) NCCLCHECK(wrap_ibv_destroy_qp(comm->gpuFlush.qp));
      if (comm->gpuFlush.hostMr != NULL) NCCLCHECK(wrap_ibv_dereg_mr(comm->gpuFlush.hostMr));
    }
    if (comm->remFifo.mr != NULL) NCCLCHECK(wrap_ibv_dereg_mr(comm->remFifo.mr));
    NCCLCHECK(ncclIbDestroyVerbs(&comm->verbs));
    free(comm);
  }
  return ncclSuccess;
}
```

</details>

<a id="f1129"></a>

## F1129：ncclIbCloseListen

用途：关闭监听 socket 并释放 IB listen 对象，不负责释放独立的已建立连接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclIbListenComm* comm = (struct ncclIbListenComm*)listenComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>close(comm-&gt;sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>free(comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclIbListenComm* comm = (struct ncclIbListenComm*)listenComm;
  if (comm) {
    close(comm->sock.fd);
    free(comm);
  }
  return ncclSuccess;
}
```

</details>

