# src/net.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1003"></a>

## F1003：ncclNet_v4_as_v5_getProperties

用途：调用旧版插件属性接口并补上 v5 新增字段的兼容默认值，如 maxRecvs=1。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclNetProperties_v4_t p4;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>ncclResult_t ans = ncclNet_v4-&gt;getProperties(dev, &p4);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(ans != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ans;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;name = p4.name;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;pciPath = p4.pciPath;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;guid = p4.guid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;ptrSupport = p4.ptrSupport;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;speed = p4.speed;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;port = p4.port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;maxComms = p4.maxComms;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;maxRecvs = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;latency = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclNetProperties_v4_t p4;
  ncclResult_t ans = ncclNet_v4->getProperties(dev, &p4);
  if (ans != ncclSuccess) return ans;
  props->name = p4.name;
  props->pciPath = p4.pciPath;
  props->guid = p4.guid;
  props->ptrSupport = p4.ptrSupport;
  props->speed = p4.speed;
  props->port = p4.port;
  props->maxComms = p4.maxComms;
  props->maxRecvs = 1;
  props->latency = 0;
  return ncclSuccess;
}
```

</details>

<a id="f1004"></a>

## F1004：ncclNet_v4_as_v5_isend

用途：把 v5 发送调用转成 v4 签名，旧接口没有 v5 tag 参数。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclNet_v4-&gt;isend(sendComm, data, size, mhandle, request);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return ncclNet_v4->isend(sendComm, data, size, mhandle, request);
}
```

</details>

<a id="f1005"></a>

## F1005：ncclNet_v4_as_v5_irecv

用途：校验 v5 批量请求只有一个元素后转调旧 v4 单请求接口，避免错误模拟多接收能力。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(n == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 条件判断 | <code>(n != 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 函数返回 | <code>return ncclNet_v4-&gt;irecv(recvComm, data[0], sizes[0], mhandles[0], request);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (n == 0) return ncclSuccess;
  if (n != 1) return ncclInvalidArgument;
  return ncclNet_v4->irecv(recvComm, data[0], sizes[0], mhandles[0], request);
}
```

</details>

<a id="f1006"></a>

## F1006：ncclNet_v4_as_v5_iflush

用途：校验 v5 批量请求只有一个元素后转调旧 v4 单请求接口，避免错误模拟多接收能力。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(n == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 条件判断 | <code>(n != 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 函数返回 | <code>return ncclNet_v4-&gt;iflush(recvComm, data[0], sizes[0], mhandles[0], request);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (n == 0) return ncclSuccess;
  if (n != 1) return ncclInvalidArgument;
  return ncclNet_v4->iflush(recvComm, data[0], sizes[0], mhandles[0], request);
}
```

</details>

<a id="f1007"></a>

## F1007：ncclNet_v4_as_v5_init

用途：初始化旧插件后构造 v5 适配函数表，保持旧插件可在本版本网络层使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet_v4-&gt;init(logfn));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.name = ncclNet_v4-&gt;name;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.devices = ncclNet_v4-&gt;devices;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.getProperties = ncclNet_v4_as_v5_getProperties;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.listen = ncclNet_v4-&gt;listen;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.connect = ncclNet_v4-&gt;connect;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.accept = ncclNet_v4-&gt;accept;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.regMr = ncclNet_v4-&gt;regMr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.deregMr = ncclNet_v4-&gt;deregMr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.isend = ncclNet_v4_as_v5_isend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.irecv = ncclNet_v4_as_v5_irecv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.iflush = ncclNet_v4_as_v5_iflush;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.test = ncclNet_v4-&gt;test;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.closeSend = ncclNet_v4-&gt;closeSend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.closeRecv = ncclNet_v4-&gt;closeRecv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.closeListen = ncclNet_v4-&gt;closeListen;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclNet_v4->init(logfn));
  ncclNet_v4_as_v5.name = ncclNet_v4->name;
  ncclNet_v4_as_v5.devices = ncclNet_v4->devices;
  ncclNet_v4_as_v5.getProperties = ncclNet_v4_as_v5_getProperties;
  ncclNet_v4_as_v5.listen = ncclNet_v4->listen;
  ncclNet_v4_as_v5.connect = ncclNet_v4->connect;
  ncclNet_v4_as_v5.accept = ncclNet_v4->accept;
  ncclNet_v4_as_v5.regMr = ncclNet_v4->regMr;
  ncclNet_v4_as_v5.deregMr = ncclNet_v4->deregMr;
  ncclNet_v4_as_v5.isend = ncclNet_v4_as_v5_isend;
  ncclNet_v4_as_v5.irecv = ncclNet_v4_as_v5_irecv;
  ncclNet_v4_as_v5.iflush = ncclNet_v4_as_v5_iflush;
  ncclNet_v4_as_v5.test = ncclNet_v4->test;
  ncclNet_v4_as_v5.closeSend = ncclNet_v4->closeSend;
  ncclNet_v4_as_v5.closeRecv = ncclNet_v4->closeRecv;
  ncclNet_v4_as_v5.closeListen = ncclNet_v4->closeListen;
  return ncclSuccess;
}
```

</details>

<a id="f1008"></a>

## F1008：ncclCollNet_v4_as_v5_getProperties

用途：调用旧版插件属性接口并补上 v5 新增字段的兼容默认值，如 maxRecvs=1。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclNetProperties_v4_t p4;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>ncclResult_t ans = ncclCollNet_v4-&gt;getProperties(dev, &p4);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(ans != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ans;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;name = p4.name;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;pciPath = p4.pciPath;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;guid = p4.guid;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;ptrSupport = p4.ptrSupport;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;speed = p4.speed;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;port = p4.port;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;maxComms = p4.maxComms;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;maxRecvs = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;latency = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ncclNetProperties_v4_t p4;
  ncclResult_t ans = ncclCollNet_v4->getProperties(dev, &p4);
  if (ans != ncclSuccess) return ans;
  props->name = p4.name;
  props->pciPath = p4.pciPath;
  props->guid = p4.guid;
  props->ptrSupport = p4.ptrSupport;
  props->speed = p4.speed;
  props->port = p4.port;
  props->maxComms = p4.maxComms;
  props->maxRecvs = 1;
  props->latency = 0;
  return ncclSuccess;
}
```

</details>

<a id="f1009"></a>

## F1009：ncclCollNet_v4_as_v5_init

用途：初始化旧插件后构造 v5 适配函数表，保持旧插件可在本版本网络层使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet_v4-&gt;init(logfn));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.name = ncclCollNet_v4-&gt;name;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.devices = ncclCollNet_v4-&gt;devices;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.getProperties = ncclCollNet_v4_as_v5_getProperties;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.listen = ncclCollNet_v4-&gt;listen;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.connect = ncclCollNet_v4-&gt;connect;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.reduceSupport = ncclCollNet_v4-&gt;reduceSupport;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.regMr = ncclCollNet_v4-&gt;regMr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.deregMr = ncclCollNet_v4-&gt;deregMr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.iallreduce = ncclCollNet_v4-&gt;iallreduce;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.iflush = ncclCollNet_v4-&gt;iflush;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.test = ncclCollNet_v4-&gt;test;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.closeColl = ncclCollNet_v4-&gt;closeColl;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.closeListen = ncclCollNet_v4-&gt;closeListen;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclCollNet_v4->init(logfn));
  ncclCollNet_v4_as_v5.name = ncclCollNet_v4->name;
  ncclCollNet_v4_as_v5.devices = ncclCollNet_v4->devices;
  ncclCollNet_v4_as_v5.getProperties = ncclCollNet_v4_as_v5_getProperties;
  ncclCollNet_v4_as_v5.listen = ncclCollNet_v4->listen;
  ncclCollNet_v4_as_v5.connect = ncclCollNet_v4->connect;
  ncclCollNet_v4_as_v5.reduceSupport = ncclCollNet_v4->reduceSupport;
  ncclCollNet_v4_as_v5.regMr = ncclCollNet_v4->regMr;
  ncclCollNet_v4_as_v5.deregMr = ncclCollNet_v4->deregMr;
  ncclCollNet_v4_as_v5.iallreduce = ncclCollNet_v4->iallreduce;
  ncclCollNet_v4_as_v5.iflush = ncclCollNet_v4->iflush;
  ncclCollNet_v4_as_v5.test = ncclCollNet_v4->test;
  ncclCollNet_v4_as_v5.closeColl = ncclCollNet_v4->closeColl;
  ncclCollNet_v4_as_v5.closeListen = ncclCollNet_v4->closeListen;
  return ncclSuccess;
}
```

</details>

<a id="f1010"></a>

## F1010：initPlugin

用途：动态装载命名网络插件，优先找 v5 符号，必要时适配 v4，并独立发现 CollNet 接口。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char ncclNetPluginName[128];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const char* envPluginName = getenv("NCCL_NET_PLUGIN");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(envPluginName && strlen(envPluginName))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>snprintf(ncclNetPluginName, 128, "libnccl-net-%s.so", envPluginName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Plugin name set by env to %s", ncclNetPluginName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>sprintf(ncclNetPluginName, "libnccl-net.so");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 准备局部状态 | <code>void* netPluginLib = dlopen(ncclNetPluginName, RTLD_NOW &#124; RTLD_LOCAL);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 条件判断 | <code>(netPluginLib == nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 条件判断 | <code>(errno == ENOENT)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "NET/Plugin : No plugin found (%s), using internal implementation", ncclNetPluginName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "NET/Plugin : Plugin load returned %d : %s.", errno, dlerror());</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>*net = (ncclNet_v5_t*)dlsym(netPluginLib, "ncclNetPlugin_v5");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(*net == nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "NET/Plugin: Failed to find ncclNetPlugin_v5 symbol.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>ncclNet_v4 = (ncclNet_v4_t*)dlsym(netPluginLib, "ncclNetPlugin_v4");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 2 / 条件判断 | <code>(ncclNet_v4 == nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "NET/Plugin: Failed to find ncclNetPlugin_v4 symbol.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 4 / 条件判断 | <code>(netPluginLib != nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 6 / 调用 / 计算 / 写入 | <code>dlclose(netPluginLib);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 4 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>*net = &ncclNet_v4_as_v5;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 2 / 调用 / 计算 / 写入 | <code>ncclNet_v4_as_v5.init = ncclNet_v4_as_v5_init;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>*collnet = (ncclCollNet_v5_t*)dlsym(netPluginLib, "ncclCollNetPlugin_v5");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 条件判断 | <code>(*collnet == nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "NET/Plugin: Failed to find ncclCollNetPlugin_v5 symbol.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4 = (ncclCollNet_v4_t*)dlsym(netPluginLib, "ncclCollNetPlugin_v4");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 2 / 条件判断 | <code>(ncclCollNet_v4 == nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET, "NET/Plugin: Failed to find ncclCollNetPlugin_v4 symbol.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 41 | 4 / 调用 / 计算 / 写入 | <code>*collnet = &ncclCollNet_v4_as_v5;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 4 / 调用 / 计算 / 写入 | <code>ncclCollNet_v4_as_v5.init = ncclCollNet_v4_as_v5_init;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char ncclNetPluginName[128];
  const char* envPluginName = getenv("NCCL_NET_PLUGIN");
  if (envPluginName && strlen(envPluginName)) {
    snprintf(ncclNetPluginName, 128, "libnccl-net-%s.so", envPluginName);
    INFO(NCCL_INIT, "Plugin name set by env to %s", ncclNetPluginName);
  } else {
    sprintf(ncclNetPluginName, "libnccl-net.so");
  }
  void* netPluginLib = dlopen(ncclNetPluginName, RTLD_NOW | RTLD_LOCAL);
  if (netPluginLib == nullptr) {
    // dlopen does not guarantee to set errno, but dlerror only gives us a
    // string, so checking errno doesn't hurt to try to provide a better
    // error message
    if (errno == ENOENT) {
      INFO(NCCL_INIT|NCCL_NET, "NET/Plugin : No plugin found (%s), using internal implementation", ncclNetPluginName);
    } else {
      INFO(NCCL_INIT|NCCL_NET, "NET/Plugin : Plugin load returned %d : %s.", errno, dlerror());
    }
    return;
  }

  *net = (ncclNet_v5_t*)dlsym(netPluginLib, "ncclNetPlugin_v5");
  if (*net == nullptr) {
    INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclNetPlugin_v5 symbol.");
    ncclNet_v4 = (ncclNet_v4_t*)dlsym(netPluginLib, "ncclNetPlugin_v4");
    if (ncclNet_v4 == nullptr) {
      INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclNetPlugin_v4 symbol.");
      if (netPluginLib != nullptr) dlclose(netPluginLib);
      return;
    }
    *net = &ncclNet_v4_as_v5;
    ncclNet_v4_as_v5.init = ncclNet_v4_as_v5_init;
  }

  // Check for CollNet
  *collnet = (ncclCollNet_v5_t*)dlsym(netPluginLib, "ncclCollNetPlugin_v5");
  if (*collnet == nullptr) {
    INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclCollNetPlugin_v5 symbol.");
    ncclCollNet_v4 = (ncclCollNet_v4_t*)dlsym(netPluginLib, "ncclCollNetPlugin_v4");
    if (ncclCollNet_v4 == nullptr) {
      INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclCollNetPlugin_v4 symbol.");
    } else {
      *collnet = &ncclCollNet_v4_as_v5;
      ncclCollNet_v4_as_v5.init = ncclCollNet_v4_as_v5_init;
    }
  }
  return;
}
```

</details>

<a id="f1011"></a>

## F1011：ncclNetInit

用途：总是初始化 bootstrap，再按外部插件、内置 IB、内置 Socket 顺序选可用数据后端，支持显式名字筛选。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetInit());</code> | 失败时可能提前return，成功才继续；选择初始化控制面使用的本机网络接口和地址；其选择不等于数据面必然使用 TCP。 |
| 2 | 0 / 准备局部状态 | <code>ncclNet_t* nets[3] = { nullptr, &ncclNetIb, &ncclNetSocket };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>ncclCollNet_t* collNets[3] = { nullptr, nullptr, nullptr };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>initPlugin(&nets[0], &collNets[0]);</code> | 动态装载命名网络插件，优先找 v5 符号，必要时适配 v4，并独立发现 CollNet 接口。 |
| 5 | 0 / 准备局部状态 | <code>char* netName = getenv("NCCL_NET");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>bool ok = false;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;3；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 条件判断 | <code>(nets[i] == nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 11 | 1 / 条件判断 | <code>(netName && strcmp(netName, nets[i]-&gt;name) != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 14 | 1 / 准备局部状态 | <code>int ndev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 1 / 条件判断 | <code>(nets[i]-&gt;init(ncclDebugLog) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 18 | 1 / 条件判断 | <code>(nets[i]-&gt;devices(&ndev) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 21 | 1 / 条件判断 | <code>(ndev &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>ncclNet = nets[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>ok = true;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 1 / 条件判断 | <code>(collNets[i])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 3 / 先执行一次的循环 | <code>初始 ；继续条件 (0)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 29 | 4 / 条件判断 | <code>(collNets[i]-&gt;init(ncclDebugLog) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 6 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 32 | 4 / 条件判断 | <code>(collNets[i]-&gt;devices(&ndev) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 6 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 35 | 4 / 条件判断 | <code>(ndev &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 6 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>ncclCollNet = collNets[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 40 | 1 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 41 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 42 | 0 / 条件判断 | <code>(!ok)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>WARN("Error: network %s not found.", netName ? netName : "");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 2 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 46 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Always initialize bootstrap network
  NCCLCHECK(bootstrapNetInit());

  // Initialize main communication network
  ncclNet_t* nets[3] = { nullptr, &ncclNetIb, &ncclNetSocket };
  ncclCollNet_t* collNets[3] = { nullptr, nullptr, nullptr };
  initPlugin(&nets[0], &collNets[0]);
  char* netName = getenv("NCCL_NET");
  bool ok = false;

  for (int i=0; i<3; i++) {
    if (nets[i] == nullptr) continue;
    if (netName && strcmp(netName, nets[i]->name) != 0) continue;

    // net plugin is already initialized
    int ndev;
    if (nets[i]->init(ncclDebugLog) != ncclSuccess) continue;
    if (nets[i]->devices(&ndev) != ncclSuccess) continue;
    if (ndev <= 0) continue;
    ncclNet = nets[i];
    ok = true;

    if (collNets[i]) {
      do {
        if (collNets[i]->init(ncclDebugLog) != ncclSuccess) break;
        if (collNets[i]->devices(&ndev) != ncclSuccess) break;
        if (ndev <= 0) break;
        ncclCollNet = collNets[i];
      } while(0);
    }
    break;
  }

  if (!ok) {
    WARN("Error: network %s not found.", netName ? netName : "");
    return ncclInvalidUsage;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1012"></a>

## F1012：ncclGpuGdrSupport

用途：优先查询 CUDA GDR 属性；旧运行时通过临时连接和 GPU 缓冲注册探测，探测资源用后回收。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>constexpr int GPU_BUF_SIZE = 2*1024*1024;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 11030</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 3 | 1 / 准备局部状态 | <code>int driverVersion;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaDriverGetVersion(&driverVersion));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 1 / 条件判断 | <code>(driverVersion &gt;= 11030)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 准备局部状态 | <code>int cudaDev, attr = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaDeviceGetAttribute(&attr, cudaDevAttrGPUDirectRDMASupported, cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>*gdrSupport = attr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 准备局部状态 | <code>int netDevs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetDevices(&netDevs));</code> | 失败时可能提前return，成功才继续；查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>*gdrSupport = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 循环 | <code>初始 int dev=0;；继续条件 dev&lt;netDevs；每轮更新 dev++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 1 / 准备局部状态 | <code>ncclNetProperties_t props;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetGetProperties(dev, &props));</code> | 失败时可能提前return，成功才继续；读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 18 | 1 / 条件判断 | <code>((props.ptrSupport & NCCL_PTR_CUDA) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 21 | 1 / 准备局部状态 | <code>void *lComm = NULL, *sComm = NULL, *rComm = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 1 / 准备局部状态 | <code>ncclNetHandle_t handle;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 1 / 准备局部状态 | <code>void* gpuPtr = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 1 / 准备局部状态 | <code>void* mHandle = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 1 / 准备局部状态 | <code>ncclResult_t ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = NCCL_NET;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclNetListen(dev, &handle, &lComm), ret, cleanup1);</code> | 失败时保存错误并跳到清理标签，成功才继续；创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 28 | 1 / 循环 | <code>初始 ；继续条件 (sComm == NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclNetConnect(dev, &handle, &sComm), ret, cleanup2);</code> | 失败时保存错误并跳到清理标签，成功才继续；推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 30 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 1 / 循环 | <code>初始 ；继续条件 (rComm == NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclNetAccept(lComm, &rComm), ret, cleanup3);</code> | 失败时保存错误并跳到清理标签，成功才继续；推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 33 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 34 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaMalloc(&gpuPtr, GPU_BUF_SIZE), ret, cleanup4);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 35 | 1 / 条件判断 | <code>(ncclNetRegMr(sComm, gpuPtr, GPU_BUF_SIZE, NCCL_PTR_CUDA, &mHandle) == ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetDeregMr(sComm, mHandle));</code> | 失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 38 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetRegMr(rComm, gpuPtr, GPU_BUF_SIZE, NCCL_PTR_CUDA, &mHandle));</code> | 失败时可能提前return，成功才继续；注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 39 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetDeregMr(rComm, mHandle));</code> | 失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 40 | 3 / 调用 / 计算 / 写入 | <code>*gdrSupport = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 1 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 1 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(gpuPtr));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 43 | 1 / 跳转标签 | <code>cleanup4</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseRecv(rComm));</code> | 失败时可能提前return，成功才继续；关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 45 | 1 / 跳转标签 | <code>cleanup3</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 46 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseSend(sComm));</code> | 失败时可能提前return，成功才继续；关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 47 | 1 / 跳转标签 | <code>cleanup2</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 48 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseListen(lComm));</code> | 失败时可能提前return，成功才继续；关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 49 | 1 / 跳转标签 | <code>cleanup1</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 50 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 51 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 52 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  constexpr int GPU_BUF_SIZE = 2*1024*1024;
#if CUDART_VERSION >= 11030
  // In CUDA 11.3 and later we can now query the cudaDevAttrGPUDirectRDMASupported attribute
  int driverVersion;
  CUDACHECK(cudaDriverGetVersion(&driverVersion));
  if (driverVersion >= 11030) {
    int cudaDev, attr = 0;
    CUDACHECK(cudaGetDevice(&cudaDev));
    CUDACHECK(cudaDeviceGetAttribute(&attr, cudaDevAttrGPUDirectRDMASupported, cudaDev));
    *gdrSupport = attr;
    return ncclSuccess;
  }
#endif
  int netDevs;
  NCCLCHECK(ncclNetDevices(&netDevs));
  *gdrSupport = 0;
  for (int dev=0; dev<netDevs; dev++) {
    // Find a net device which is GDR-capable
    ncclNetProperties_t props;
    NCCLCHECK(ncclNetGetProperties(dev, &props));
    if ((props.ptrSupport & NCCL_PTR_CUDA) == 0) continue;

    // Allocate memory on the GPU and try to register it on the NIC.
    void *lComm = NULL, *sComm = NULL, *rComm = NULL;
    ncclNetHandle_t handle;
    void* gpuPtr = NULL;
    void* mHandle = NULL;
    ncclResult_t ret;
    ncclDebugNoWarn = NCCL_NET;
    NCCLCHECKGOTO(ncclNetListen(dev, &handle, &lComm), ret, cleanup1);
    while (sComm == NULL) {
      NCCLCHECKGOTO(ncclNetConnect(dev, &handle, &sComm), ret, cleanup2);
    }
    while (rComm == NULL) {
      NCCLCHECKGOTO(ncclNetAccept(lComm, &rComm), ret, cleanup3);
    }
    CUDACHECKGOTO(cudaMalloc(&gpuPtr, GPU_BUF_SIZE), ret, cleanup4);
    if (ncclNetRegMr(sComm, gpuPtr, GPU_BUF_SIZE, NCCL_PTR_CUDA, &mHandle) == ncclSuccess) {
      NCCLCHECK(ncclNetDeregMr(sComm, mHandle));
      NCCLCHECK(ncclNetRegMr(rComm, gpuPtr, GPU_BUF_SIZE, NCCL_PTR_CUDA, &mHandle));
      NCCLCHECK(ncclNetDeregMr(rComm, mHandle));
      *gdrSupport = 1;
    }
    ncclDebugNoWarn = 0;
    CUDACHECK(cudaFree(gpuPtr));
cleanup4:
    NCCLCHECK(ncclNetCloseRecv(rComm));
cleanup3:
    NCCLCHECK(ncclNetCloseSend(sComm));
cleanup2:
    NCCLCHECK(ncclNetCloseListen(lComm));
cleanup1:
    break;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1013"></a>

## F1013：ncclNetVersion

用途：报告当前选中网络函数表是 v4 兼容还是原生 v5，供依赖接口能力的策略判断。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (ncclNet == &ncclNet_v4_as_v5) ? 4 : 5;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return (ncclNet == &ncclNet_v4_as_v5) ? 4 : 5;
}
```

</details>

