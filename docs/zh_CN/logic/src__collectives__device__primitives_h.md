# src/collectives/device/primitives.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0136"></a>

## F0136：calcBytePerStep

用途：计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclShmem.comm.buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return ncclShmem.comm.buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS;
  }
```

</details>

<a id="f0137"></a>

## F0137：calcBytePerGrain

用途：给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return sizeof(uint64_t);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return sizeof(uint64_t); // Bogus value? Nobody queries this metric for simple.
  }
```

</details>

<a id="f0138"></a>

## F0138：calcGroupWidth

用途：返回该协议线程组需要占用的 group/barrier 编号宽度，Simple 同步 warp 可能需要更宽分组。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return send && nthreads-WARP_SIZE &gt;= 64 ? 2 : 1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return send && nthreads-WARP_SIZE >= 64 ? 2 : 1;
  }
```

</details>

<a id="f0139"></a>

## F0139：calcBytePerStep

用途：计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclShmem.comm.buffSizes[NCCL_PROTO_LL]/NCCL_STEPS/2;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return ncclShmem.comm.buffSizes[NCCL_PROTO_LL]/NCCL_STEPS/2; // Half is data
  }
```

</details>

<a id="f0140"></a>

## F0140：calcBytePerGrain

用途：给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return sizeof(uint64_t);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return sizeof(uint64_t); // One 16-byte line has 8-bytes of data
  }
```

</details>

<a id="f0141"></a>

## F0141：calcGroupWidth

用途：返回该协议线程组需要占用的 group/barrier 编号宽度，Simple 同步 warp 可能需要更宽分组。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return 1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return 1;
  }
```

</details>

<a id="f0142"></a>

## F0142：calcBytePerStep

用途：计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (ncclShmem.comm.buffSizes[NCCL_PROTO_LL128]/NCCL_STEPS)*NCCL_LL128_DATAELEMS/NCCL_LL128_LINEELEMS;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return (ncclShmem.comm.buffSizes[NCCL_PROTO_LL128]/NCCL_STEPS)*NCCL_LL128_DATAELEMS/NCCL_LL128_LINEELEMS;
  }
```

</details>

<a id="f0143"></a>

## F0143：calcBytePerGrain

用途：给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return NCCL_LL128_SHMEM_ELEMS_PER_THREAD*NCCL_LL128_DATAELEMS*sizeof(uint64_t)/NCCL_LL128_LINEELEMS;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return NCCL_LL128_SHMEM_ELEMS_PER_THREAD*NCCL_LL128_DATAELEMS*sizeof(uint64_t)/NCCL_LL128_LINEELEMS;
  }
```

</details>

<a id="f0144"></a>

## F0144：calcGroupWidth

用途：返回该协议线程组需要占用的 group/barrier 编号宽度，Simple 同步 warp 可能需要更宽分组。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return 1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return 1;
  }
```

</details>

<a id="f0145"></a>

## F0145：FanAsymmetric

用途：构造当前原语的有效接收/发送邻居数量；对称版本要求收发数相同以节省状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // assert(nrecv <= MaxRecv && nsend <= MaxSend);
  }
```

</details>

<a id="f0146"></a>

## F0146：nrecv

用途：返回本原语实际有效邻居数；编译期最大邻居度为零时直接返回零，便于消除无用分支。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return MaxRecv ? nr : 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return MaxRecv ? nr : 0; }
```

</details>

<a id="f0147"></a>

## F0147：nsend

用途：返回本原语实际有效邻居数；编译期最大邻居度为零时直接返回零，便于消除无用分支。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return MaxSend ? ns : 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return MaxSend ? ns : 0; }
```

</details>

<a id="f0148"></a>

## F0148：FanSymmetric

用途：构造当前原语的有效接收/发送邻居数量；对称版本要求收发数相同以节省状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // assert(nrecv == nsend && nrecv <= MaxArity);
  }
```

</details>

<a id="f0149"></a>

## F0149：nrecv

用途：返回本原语实际有效邻居数；编译期最大邻居度为零时直接返回零，便于消除无用分支。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return n;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return n; }
```

</details>

<a id="f0150"></a>

## F0150：nsend

用途：返回本原语实际有效邻居数；编译期最大邻居度为零时直接返回零，便于消除无用分支。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return n;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return n; }
```

</details>

<a id="f0151"></a>

## F0151：directSend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>static_cast&lt;RealPrimitives*&gt;(this)-&gt;send(inpIx, eltN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    static_cast<RealPrimitives*>(this)->send(inpIx, eltN);
  }
```

</details>

<a id="f0152"></a>

## F0152：directSendFromOutput

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>static_cast&lt;RealPrimitives*&gt;(this)-&gt;sendFromOutput(outIx, eltN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    static_cast<RealPrimitives*>(this)->sendFromOutput(outIx, eltN);
  }
```

</details>

<a id="f0153"></a>

## F0153：directRecv

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>static_cast&lt;RealPrimitives*&gt;(this)-&gt;recv(outIx, eltN, false);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    static_cast<RealPrimitives*>(this)->recv(outIx, eltN, /*postOp=*/false);
  }
```

</details>

<a id="f0154"></a>

## F0154：directCopySend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>static_cast&lt;RealPrimitives*&gt;(this)-&gt;copySend(inpIx, outIx, eltN, postOp);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    static_cast<RealPrimitives*>(this)->copySend(inpIx, outIx, eltN, postOp);
  }
```

</details>

<a id="f0155"></a>

## F0155：directRecvCopySend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>static_cast&lt;RealPrimitives*&gt;(this)-&gt;recvCopySend(outIx, eltN, false);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    static_cast<RealPrimitives*>(this)->recvCopySend(outIx, eltN, /*postOp=*/false);
  }
```

</details>

<a id="f0156"></a>

## F0156：directRecvReduceCopySend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>static_cast&lt;RealPrimitives*&gt;(this)-&gt;recvReduceCopySend(inpIx, outIx, eltN, postOp);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // Direct is only for the send part
    static_cast<RealPrimitives*>(this)->recvReduceCopySend(inpIx, outIx, eltN, postOp);
  }
```

</details>

