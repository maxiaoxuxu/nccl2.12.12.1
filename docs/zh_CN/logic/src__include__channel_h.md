# src/include/channel.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0610"></a>

## F0610：ncclChannelComputeBase

用途：由节点距离与节点内 rank 差计算 P2P 基准通道，Send/Recv 用相反公式确保两端匹配。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int p2pGroupSize = NCCL_MAX_WORK_ELEMENTS_P2P/2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int peerNode = comm-&gt;rankToNode[peer];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int peerIndex = comm-&gt;rankToLocalRank[peer];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int nsteps = comm-&gt;maxLocalRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>int rankIndex = comm-&gt;rankToLocalRank[comm-&gt;rank];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int step, delta;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 7 | 0 / 条件判断 | <code>(coll == ncclFuncSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>step = (nsteps + peerIndex - rankIndex)%nsteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>delta = (comm-&gt;nNodes + peerNode - comm-&gt;node) % comm-&gt;nNodes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 12 | 2 / 条件判断 | <code>(coll == ncclFuncRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>step = (nsteps + rankIndex - peerIndex)%nsteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>delta = (comm-&gt;nNodes + comm-&gt;node - peerNode) % comm-&gt;nNodes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 17 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>*channelBase = comm-&gt;nNodes &gt; 1 ? delta+(step/p2pGroupSize) : step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int p2pGroupSize = NCCL_MAX_WORK_ELEMENTS_P2P/2;
  int peerNode = comm->rankToNode[peer];
  int peerIndex = comm->rankToLocalRank[peer];
  int nsteps = comm->maxLocalRanks;
  int rankIndex = comm->rankToLocalRank[comm->rank];
  int step, delta;
  if (coll == ncclFuncSend) {
    step = (nsteps + peerIndex - rankIndex)%nsteps;
    delta = (comm->nNodes + peerNode - comm->node) % comm->nNodes;
  } else if (coll == ncclFuncRecv) {
    step = (nsteps + rankIndex - peerIndex)%nsteps;
    delta = (comm->nNodes + comm->node - peerNode) % comm->nNodes;
  } else {
    return ncclInternalError;
  }
  *channelBase = comm->nNodes > 1 ? delta+(step/p2pGroupSize) : step;
  return ncclSuccess;
}
```

</details>

<a id="f0611"></a>

## F0611：ncclChannelComputeFromBase

用途：将基准通道加位反转分散偏移，对全局 P2P 通道空间取模，多个 peer 可映射到同通道。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*channelId = (base+comm-&gt;p2pChannels[channelInc]) % comm-&gt;p2pnChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *channelId = (base+comm->p2pChannels[channelInc]) % comm->p2pnChannels;
  return ncclSuccess;
}
```

</details>

<a id="f0612"></a>

## F0612：ncclChannelCompute

用途：组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int base;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelComputeBase(comm, peer, coll, &base));</code> | 失败时可能提前return，成功才继续；由节点距离与节点内 rank 差计算 P2P 基准通道，Send/Recv 用相反公式确保两端匹配。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclChannelComputeFromBase(comm, base, channelInc, channelId));</code> | 失败时可能提前return，成功才继续；将基准通道加位反转分散偏移，对全局 P2P 通道空间取模，多个 peer 可映射到同通道。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int base;
  NCCLCHECK(ncclChannelComputeBase(comm, peer, coll, &base));
  NCCLCHECK(ncclChannelComputeFromBase(comm, base, channelInc, channelId));
  return ncclSuccess;
}
```

</details>

