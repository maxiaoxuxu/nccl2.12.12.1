# src/collectives/all_gather.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0050"></a>

## F0050：ncclAllGather

用途：收集每个 rank 的贡献到所有 rank 的输出，输出按用户 rank 分段；复制型任务不做数值求和。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncAllGather, "AllGather", sendbuff, recvbuff, sendcount, datatype, ncclSum, 0, comm, stream, ALLGATHER_CHUNKSTEPS, ALLGATHER_SLICESTEPS };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 函数返回 | <code>return ncclEnqueueCheck(&info);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：AllGather 的调用与尺寸契约
 * sendcount 是本 rank 贡献元素数，输出需容纳 nRanks*sendcount，按用户 rank 顺序排列。
 * 示例：rank0=[1,2]，rank1=[3,4]，两边输出都是 [1,2,3,4]，不进行求和。
 * EnqueueCheck 的 ArgsCheck 会把复制型任务归一为 Int8，再走共同的选型/排队/发射流程。
 * 设备 all_gather.h 的 Ring 每步收/存/转发别人的贡献，地址公式是 rank*size+chunkOffset。
 * 原地形式要求 sendbuff 指向 recvbuff 中属于自己的那一段，不是任意两个指针相同即可。
 */
  struct ncclInfo info = { ncclFuncAllGather, "AllGather",
    sendbuff, recvbuff, sendcount, datatype, ncclSum, 0, comm, stream, /* Args */
    ALLGATHER_CHUNKSTEPS, ALLGATHER_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
```

</details>

