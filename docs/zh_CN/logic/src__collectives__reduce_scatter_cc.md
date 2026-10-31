# src/collectives/reduce_scatter.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0378"></a>

## F0378：ncclReduceScatter

用途：先归约各 rank 的完整输入，再只把属于当前用户 rank 的结果段保留在本地输出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncReduceScatter, "ReduceScatter", sendbuff, recvbuff, recvcount, datatype, op, 0, comm, stream, REDUCESCATTER_CHUNKSTEPS, REDUCESCATTER_SLICESTEPS };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 函数返回 | <code>return ncclEnqueueCheck(&info);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：ReduceScatter 的调用与尺寸契约
 * recvcount 是本 rank 的输出元素数，输入需容纳 nRanks*recvcount，所有 rank 输入长度相同。
 * 示例：2 rank、recvcount=2，输入 [1,2,3,4] 和 [10,20,30,40]，
 * rank0 输出 [11,22]，rank1 输出 [33,44]，即完整求和结果按 rank 分段。
 * EnqueueCheck/computeColl 生成 Ring 工作，设备 reduce_scatter.h 做 send/reduceSend/reduceCopy。
 * 最终归约写到自己短输出的 chunkOffset，而输入读取对应用户 rank 的完整段偏移。
 */
  struct ncclInfo info = { ncclFuncReduceScatter, "ReduceScatter",
    sendbuff, recvbuff, recvcount, datatype, op, 0, comm, stream, /* Args */
    REDUCESCATTER_CHUNKSTEPS, REDUCESCATTER_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
```

</details>

