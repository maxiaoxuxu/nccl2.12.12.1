# src/collectives/all_reduce.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0051"></a>

## F0051：ncclAllReduce

用途：把每个 rank 的同尺寸输入归约为每个 rank 都持有的结果，API 先封装任务再交给统一调度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncAllReduce, "AllReduce", sendbuff, recvbuff, count, datatype, op, 0, comm, stream, ALLREDUCE_CHUNKSTEPS, ALLREDUCE_SLICESTEPS };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 函数返回 | <code>return ncclEnqueueCheck(&info);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：从这一 API 开始跟完整调用链
 * 每个 rank 输入 count 个元素，输出同样 count 个归约结果；可同地址原地操作。
 * 示例：rank0=[1,2]，rank1=[10,20]，Sum 后两边输出均为 [11,22]。
 * 此函数只构造 ncclInfo；EnqueueCheck -> SetupCollKernel -> computeColl 选算法/协议及分片。
 * HostSetup 同时装 GPU FIFO 和发布 Proxy 工作，再 LaunchKernel；设备进入 all_reduce.h。
 * Ring/Tree/CollNet 在设备端调用组合原语，最后 ReduceOrCopyMulti 完成数值计算与存储。
 */
  struct ncclInfo info = { ncclFuncAllReduce, "AllReduce",
    sendbuff, recvbuff, count, datatype, op, 0, comm, stream, /* Args */
    ALLREDUCE_CHUNKSTEPS, ALLREDUCE_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
```

</details>

