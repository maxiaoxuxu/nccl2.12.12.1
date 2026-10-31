# src/collectives/sendrecv.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0379"></a>

## F0379：ncclSend

用途：把消息按 peer 加入 P2P 排队，通过嵌套 Group 支持批量匹配发射；不在此直接执行网络发送。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncSend, "Send", NULL, (void*)sendbuff, count, datatype, ncclSum, peer, comm, stream, 1, 1 };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>ncclResult_t ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGroupStart());</code> | 失败时可能提前return，成功才继续；进入线程局部 Group 嵌套，最外层首次进入时清理组任务状态。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>ret = ncclEnqueueCheck(&info);</code> | 通信 API 的统一检查与分流入口，组内保存任务，普通路径准备队列并发布/发射执行。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGroupEnd());</code> | 失败时可能提前return，成功才继续；最外层出口统一执行初始化、P2P 预建链、消息分块、collective 聚合、Proxy 发布和 kernel 发射。 |
| 7 | 0 / 函数返回 | <code>return ret;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  struct ncclInfo info = { ncclFuncSend, "Send",
    NULL, (void*)sendbuff, count, datatype, ncclSum, peer, comm, stream, /* Args */
    1, 1 };
  ncclResult_t ret;
  NCCLCHECK(ncclGroupStart());
  ret = ncclEnqueueCheck(&info);
  NCCLCHECK(ncclGroupEnd());
  return ret;
}
```

</details>

<a id="f0380"></a>

## F0380：ncclRecv

用途：把匹配 peer 的接收加入 P2P 排队，实际缓冲搬运在设备原语与必要的 Proxy 中完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX3_FUNC_RANGE_IN(nccl_domain);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclInfo info = { ncclFuncRecv, "Recv", NULL, recvbuff, count, datatype, ncclSum, peer, comm, stream, 1, 1 };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>ncclResult_t ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGroupStart());</code> | 失败时可能提前return，成功才继续；进入线程局部 Group 嵌套，最外层首次进入时清理组任务状态。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>ret = ncclEnqueueCheck(&info);</code> | 通信 API 的统一检查与分流入口，组内保存任务，普通路径准备队列并发布/发射执行。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGroupEnd());</code> | 失败时可能提前return，成功才继续；最外层出口统一执行初始化、P2P 预建链、消息分块、collective 聚合、Proxy 发布和 kernel 发射。 |
| 7 | 0 / 函数返回 | <code>return ret;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  struct ncclInfo info = { ncclFuncRecv, "Recv",
    NULL, recvbuff, count, datatype, ncclSum, peer, comm, stream, /* Args */
    1, 1 };
  ncclResult_t ret;
  NCCLCHECK(ncclGroupStart());
  ret = ncclEnqueueCheck(&info);
  NCCLCHECK(ncclGroupEnd());
  return ret;
}
```

</details>

