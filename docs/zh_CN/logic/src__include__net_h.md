# src/include/net.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0660"></a>

## F0660：ncclNetName

用途：读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclNet-&gt;name;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return ncclNet->name; }
```

</details>

<a id="f0661"></a>

## F0661：ncclNetDevices

用途：查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;devices(ndev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->devices(ndev)); return ncclSuccess; }
```

</details>

<a id="f0662"></a>

## F0662：ncclNetGetProperties

用途：读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;getProperties(dev, props));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->getProperties(dev, props)); return ncclSuccess; }
```

</details>

<a id="f0663"></a>

## F0663：ncclNetListen

用途：创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;listen(dev, handle, listenComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->listen(dev, handle, listenComm)); return ncclSuccess; }
```

</details>

<a id="f0664"></a>

## F0664：ncclNetConnect

用途：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;connect(dev, handle, sendComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->connect(dev, handle, sendComm)); return ncclSuccess; }
```

</details>

<a id="f0665"></a>

## F0665：ncclNetAccept

用途：推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;accept(listenComm, recvComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->accept(listenComm, recvComm)); return ncclSuccess; }
```

</details>

<a id="f0666"></a>

## F0666：ncclNetRegMr

用途：注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;regMr(comm, data, size, type, mhandle));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->regMr(comm, data, size, type, mhandle)); return ncclSuccess; }
```

</details>

<a id="f0667"></a>

## F0667：ncclNetDeregMr

用途：解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;deregMr(comm, mhandle));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->deregMr(comm, mhandle)); return ncclSuccess; }
```

</details>

<a id="f0668"></a>

## F0668：ncclNetIsend

用途：提交异步发送请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;isend(sendComm, data, size, tag, mhandle, request));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->isend(sendComm, data, size, tag, mhandle, request)); return ncclSuccess; }
```

</details>

<a id="f0669"></a>

## F0669：ncclNetIrecv

用途：提交一组带地址/长度/tag的异步接收；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;irecv(recvComm, n, data, sizes, tags, mhandles, request));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->irecv(recvComm, n, data, sizes, tags, mhandles, request)); return ncclSuccess; }
```

</details>

<a id="f0670"></a>

## F0670：ncclNetIflush

用途：提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;iflush(recvComm, n, data, sizes, mhandles, request));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->iflush(recvComm, n, data, sizes, mhandles, request)); return ncclSuccess; }
```

</details>

<a id="f0671"></a>

## F0671：ncclNetTest

用途：查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;test(request, done, sizes));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->test(request, done, sizes)); return ncclSuccess; }
```

</details>

<a id="f0672"></a>

## F0672：ncclNetCloseSend

用途：关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;closeSend(sendComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->closeSend(sendComm)); return ncclSuccess; }
```

</details>

<a id="f0673"></a>

## F0673：ncclNetCloseRecv

用途：关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;closeRecv(recvComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->closeRecv(recvComm)); return ncclSuccess; }
```

</details>

<a id="f0674"></a>

## F0674：ncclNetCloseListen

用途：关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNet-&gt;closeListen(listenComm));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ NCCLCHECK(ncclNet->closeListen(listenComm)); return ncclSuccess; }
```

</details>

