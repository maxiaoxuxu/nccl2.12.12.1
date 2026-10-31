# src/collectives/reduce.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0377"></a>

## F0377：ncclReduce

用途：沿选定通信路径把全部贡献归约到 root，其他 rank 不保证持有完整结果。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncReduce, "Reduce", sendbuff, recvbuff, count, datatype, op, root, comm, stream, REDUCE_CHUNKSTEPS, REDUCE_SLICESTEPS };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 函数返回 | <code>return ncclEnqueueCheck(&info);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：Reduce 只有 root 需要最终输出
 * 所有 rank 各提供 count 元素；归约结果仅保证放到 root 的 recvbuff。
 * 示例：两个 rank 输入 [1,2] 和 [10,20]，root 得到 [11,22]，其他 rank 无需完整输出。
 * 其主机入队流程与 AllReduce 共用，但 pattern 是 PipelineTo，Proxy 仅准备必要方向的边。
 * 设备 reduce.h 从 root 后继启动 send，沿环归约到 root 后结束，不再做 AllGather。
 * 选择 root 改变流水线的起点/终点，不改变数据类型和归约操作的数学含义。
 */
  struct ncclInfo info = { ncclFuncReduce, "Reduce",
    sendbuff, recvbuff, count, datatype, op, root, comm, stream, /* Args */
    REDUCE_CHUNKSTEPS, REDUCE_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
```

</details>

