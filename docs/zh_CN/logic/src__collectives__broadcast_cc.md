# src/collectives/broadcast.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0052"></a>

## F0052：ncclBroadcast

用途：从 root 将同一输入分片流水传播到所有参与者，非 root 无需提供有效广播输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncBroadcast, "Broadcast", sendbuff, recvbuff, count, datatype, ncclSum, root, comm, stream, BROADCAST_CHUNKSTEPS, BROADCAST_SLICESTEPS };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 函数返回 | <code>return ncclEnqueueCheck(&info);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：Broadcast 的根与普通参与者
 * 所有 rank 都必须参与同一调用序列；只有 root 的 sendbuff 是广播输入。
 * 每个 rank 的 recvbuff 得到相同 count 元素，不增加 nRanks 倍的输出长度。
 * ArgsCheck 归一到 Int8，随后共用 enqueue 提交流程；设备 broadcast.h 按 root 切断环回边。
 * root copySend 或 send，中间 recvCopySend，root 前驱 recv；数据可以按 slice 流水传播。
 * 兼容 API ncclBcast 只把同一 buff 作为输入输出转调这里。
 */
  struct ncclInfo info = { ncclFuncBroadcast, "Broadcast",
    sendbuff, recvbuff, count, datatype, ncclSum, root, comm, stream, /* Args */
    BROADCAST_CHUNKSTEPS, BROADCAST_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
```

</details>

<a id="f0053"></a>

## F0053：ncclBcast

用途：把同一缓冲作为输入/输出转调 Broadcast，保留旧式原地 API 的兼容行为。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclBroadcast(buff, buff, count, datatype, root, comm, stream);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return ncclBroadcast(buff, buff, count, datatype, root, comm, stream);
}
```

</details>

