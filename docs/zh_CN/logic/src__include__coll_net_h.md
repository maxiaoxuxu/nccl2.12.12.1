# src/include/coll_net.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0613"></a>

## F0613：collNetName

用途：读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclCollNet-&gt;name;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return ncclCollNet->name; }
```

</details>

<a id="f0614"></a>

## F0614：collNetDevices

用途：查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;devices(ndev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->devices(ndev)); return ncclSuccess; }
```

</details>

<a id="f0615"></a>

## F0615：collNetGetProperties

用途：读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;getProperties(dev, props));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->getProperties(dev, props)); return ncclSuccess; }
```

</details>

<a id="f0616"></a>

## F0616：collNetListen

用途：创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;listen(dev, handle, listenComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->listen(dev, handle, listenComm)); return ncclSuccess; }
```

</details>

<a id="f0617"></a>

## F0617：collNetConnect

用途：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;connect(handles, nranks, rank, listenComm, collComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->connect(handles, nranks, rank, listenComm, collComm)); return ncclSuccess; }
```

</details>

<a id="f0618"></a>

## F0618：collNetReduceSupport

用途：查询某数据类型和归约操作能否在网络内完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;reduceSupport(dataType, redOp, supported));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->reduceSupport(dataType, redOp, supported)); return ncclSuccess; }
```

</details>

<a id="f0619"></a>

## F0619：collNetRegMr

用途：注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;regMr(comm, data, size, type, mhandle));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->regMr(comm, data, size, type, mhandle)); return ncclSuccess; }
```

</details>

<a id="f0620"></a>

## F0620：collNetDeregMr

用途：解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;deregMr(comm, mhandle));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->deregMr(comm, mhandle)); return ncclSuccess; }
```

</details>

<a id="f0621"></a>

## F0621：collNetIallreduce

用途：提交网络集合归约请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;iallreduce(collComm, sendData, recvData, count, dataType, redOp, sendMhandle, recvMhandle, request));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclCollNet->iallreduce(collComm, sendData, recvData, count, dataType, redOp, sendMhandle, recvMhandle, request)); return ncclSuccess; }
```

</details>

<a id="f0622"></a>

## F0622：collNetIflush

用途：提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;iflush(collComm, data, size, mhandle, request));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->iflush(collComm, data, size, mhandle, request)); return ncclSuccess; }
```

</details>

<a id="f0623"></a>

## F0623：collNetTest

用途：查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;test(request, done, size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->test(request, done, size)); return ncclSuccess; }
```

</details>

<a id="f0624"></a>

## F0624：collNetCloseColl

用途：关闭集合网络连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;closeColl(collComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->closeColl(collComm)); return ncclSuccess; }
```

</details>

<a id="f0625"></a>

## F0625：collNetCloseListen

用途：关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCollNet-&gt;closeListen(listenComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclCollNet->closeListen(listenComm)); return ncclSuccess; }
```

</details>

<a id="f0626"></a>

## F0626：collNetSupport

用途：判断已选 CollNet 接口是否存在且能使用；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclCollNet != nullptr ? 1 : 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return ncclCollNet != nullptr ? 1 : 0; }
```

</details>

