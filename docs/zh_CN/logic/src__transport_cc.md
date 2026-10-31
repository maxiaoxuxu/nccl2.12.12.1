# src/transport.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1172"></a>

## F1172：selectTransport

用途：按 P2P、SHM、NET、CollNet 顺序检查连接能力并调用首个可用后端的 setup。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclPeerInfo* myInfo = comm-&gt;peerInfo+comm-&gt;rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclPeerInfo* peerInfo = comm-&gt;peerInfo+peer;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclConnector* connector = (type == 1) ? comm-&gt;channels[channelId].peers[peer].send + connIndex : comm-&gt;channels[channelId].peers[peer].recv + connIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int t=0;；继续条件 t&lt;NTRANSPORTS；每轮更新 t++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 准备局部状态 | <code>struct ncclTransport *transport = ncclTransports+t;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 准备局部状态 | <code>struct ncclTransportComm* transportComm = type == 1 ? &transport-&gt;send : &transport-&gt;recv;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 准备局部状态 | <code>int ret = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(transport-&gt;canConnect(&ret, comm-&gt;topo, graph, myInfo, peerInfo));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 1 / 条件判断 | <code>(ret)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>connector-&gt;transportComm = transportComm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(transportComm-&gt;setup(comm, graph, myInfo, peerInfo, connect, connector, channelId, connIndex));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 13 | 3 / 条件判断 | <code>(transportType)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 5 / 调用 / 计算 / 写入 | <code>*transportType = t;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 17 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>WARN("No transport found for rank %d[%lx] -&gt; rank %d[%lx]", myInfo-&gt;rank, myInfo-&gt;busId, peerInfo-&gt;rank, peerInfo-&gt;busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 0 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclPeerInfo* myInfo = comm->peerInfo+comm->rank;
  struct ncclPeerInfo* peerInfo = comm->peerInfo+peer;
  struct ncclConnector* connector = (type == 1) ? comm->channels[channelId].peers[peer].send + connIndex :
                                                  comm->channels[channelId].peers[peer].recv + connIndex;
  for (int t=0; t<NTRANSPORTS; t++) {
    struct ncclTransport *transport = ncclTransports+t;
    struct ncclTransportComm* transportComm = type == 1 ? &transport->send : &transport->recv;
    int ret = 0;
    NCCLCHECK(transport->canConnect(&ret, comm->topo, graph, myInfo, peerInfo));
    if (ret) {
      connector->transportComm = transportComm;
      NCCLCHECK(transportComm->setup(comm, graph, myInfo, peerInfo, connect, connector, channelId, connIndex));
      if (transportType) *transportType = t;
      return ncclSuccess;
    }
  }
  WARN("No transport found for rank %d[%lx] -> rank %d[%lx]", myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId);
  return ncclSystemError;
}
```

</details>

<a id="f1173"></a>

## F1173：ncclTransportP2pConnect

用途：为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "nsend %d nrecv %d", nsend, nrecv);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>uint32_t mask = 1 &lt;&lt; channel-&gt;id;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nrecv；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 准备局部状态 | <code>int peer = peerRecv[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 1 / 条件判断 | <code>(peer == -1 &#124;&#124; peer &gt;= comm-&gt;nRanks &#124;&#124; peer == comm-&gt;rank &#124;&#124; channel-&gt;peers[peer].recv[connIndex].connected)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;connectRecv[peer] &#124;= mask;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nsend；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 1 / 准备局部状态 | <code>int peer = peerSend[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 1 / 条件判断 | <code>(peer == -1 &#124;&#124; peer &gt;= comm-&gt;nRanks &#124;&#124; peer == comm-&gt;rank &#124;&#124; channel-&gt;peers[peer].send[connIndex].connected)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;connectSend[peer] &#124;= mask;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  TRACE(NCCL_INIT, "nsend %d nrecv %d", nsend, nrecv);
  uint32_t mask = 1 << channel->id;
  for (int i=0; i<nrecv; i++) {
    int peer = peerRecv[i];
    if (peer == -1 || peer >= comm->nRanks || peer == comm->rank || channel->peers[peer].recv[connIndex].connected) continue;
    comm->connectRecv[peer] |= mask;
  }
  for (int i=0; i<nsend; i++) {
    int peer = peerSend[i];
    if (peer == -1 || peer >= comm->nRanks || peer == comm->rank || channel->peers[peer].send[connIndex].connected) continue;
    comm->connectSend[peer] |= mask;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1174"></a>

## F1174：dumpData

用途：按字节/机器字打印 transport 建链句柄内容，仅作控制元数据诊断。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;ndata；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>printf("[%d] ", n);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 1 / 准备局部状态 | <code>uint8_t* d = (uint8_t*)data;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;sizeof(struct ncclConnect)；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>printf("%02x", d[i]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>printf("\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int n=0; n<ndata; n++) {
    printf("[%d] ", n);
    uint8_t* d = (uint8_t*)data;
    for (int i=0; i<sizeof(struct ncclConnect); i++) printf("%02x", d[i]);
    printf("\n");
  }
}
```

</details>

<a id="f1175"></a>

## F1175：ncclTransportP2pSetup

用途：按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>cudaStream_t transportSetupStream;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamCreateWithFlags(&transportSetupStream, cudaStreamNonBlocking));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 准备局部状态 | <code>int highestType = TRANSPORT_P2P;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclConnect data[2*MAXCHANNELS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 循环 | <code>初始 int i=1;；继续条件 i&lt;comm-&gt;nRanks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 准备局部状态 | <code>int bootstrapTag = (i&lt;&lt;8) + (graph ? graph-&gt;id+1 : 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 准备局部状态 | <code>int recvPeer = (comm-&gt;rank - i + comm-&gt;nRanks) % comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 准备局部状态 | <code>int sendPeer = (comm-&gt;rank + i) % comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 1 / 准备局部状态 | <code>uint32_t recvMask = comm-&gt;connectRecv[recvPeer];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 准备局部状态 | <code>uint32_t sendMask = comm-&gt;connectSend[sendPeer];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 1 / 准备局部状态 | <code>struct ncclConnect* recvData = data;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 1 / 准备局部状态 | <code>int sendChannels = 0, recvChannels = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 准备局部状态 | <code>int type;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 1 / 循环 | <code>初始 int c=0;；继续条件 c&lt;MAXCHANNELS；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 2 / 条件判断 | <code>(recvMask & (1&lt;&lt;c))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(selectTransport&lt;0&gt;(comm, graph, recvData+recvChannels++, c, recvPeer, connIndex, &type));</code> | 失败时可能提前return，成功才继续；按 P2P、SHM、NET、CollNet 顺序检查连接能力并调用首个可用后端的 setup。 |
| 19 | 4 / 条件判断 | <code>(type &gt; highestType)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 6 / 调用 / 计算 / 写入 | <code>highestType = type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>TIME_STOP(0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 1 / 准备局部状态 | <code>struct ncclConnect* sendData = recvData+recvChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 1 / 循环 | <code>初始 int c=0;；继续条件 c&lt;MAXCHANNELS；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 2 / 条件判断 | <code>(sendMask & (1&lt;&lt;c))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(selectTransport&lt;1&gt;(comm, graph, sendData+sendChannels++, c, sendPeer, connIndex, &type));</code> | 失败时可能提前return，成功才继续；按 P2P、SHM、NET、CollNet 顺序检查连接能力并调用首个可用后端的 setup。 |
| 30 | 4 / 条件判断 | <code>(type &gt; highestType)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 6 / 调用 / 计算 / 写入 | <code>highestType = type;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 34 | 1 / 调用 / 计算 / 写入 | <code>TIME_STOP(1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 1 / 条件判断 | <code>(sendPeer == recvPeer)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 3 / 条件判断 | <code>(recvChannels+sendChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapSend(comm-&gt;bootstrap, recvPeer, bootstrapTag, data, sizeof(struct ncclConnect)*(recvChannels+sendChannels)));</code> | 失败时可能提前return，成功才继续；向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。 |
| 41 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapRecv(comm-&gt;bootstrap, recvPeer, bootstrapTag, data, sizeof(struct ncclConnect)*(recvChannels+sendChannels)));</code> | 失败时可能提前return，成功才继续；优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。 |
| 42 | 5 / 调用 / 计算 / 写入 | <code>sendData = data;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 5 / 调用 / 计算 / 写入 | <code>recvData = data+sendChannels;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 45 | 3 / 条件判断 | <code>(recvChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapSend(comm-&gt;bootstrap, recvPeer, bootstrapTag, recvData, sizeof(struct ncclConnect)*recvChannels));</code> | 失败时可能提前return，成功才继续；向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。 |
| 48 | 3 / 条件判断 | <code>(sendChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 49 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 50 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapSend(comm-&gt;bootstrap, sendPeer, bootstrapTag, sendData, sizeof(struct ncclConnect)*sendChannels));</code> | 失败时可能提前return，成功才继续；向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。 |
| 51 | 3 / 条件判断 | <code>(sendChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 52 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 53 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapRecv(comm-&gt;bootstrap, sendPeer, bootstrapTag, sendData, sizeof(struct ncclConnect)*sendChannels));</code> | 失败时可能提前return，成功才继续；优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。 |
| 54 | 3 / 条件判断 | <code>(recvChannels)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 55 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 56 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapRecv(comm-&gt;bootstrap, recvPeer, bootstrapTag, recvData, sizeof(struct ncclConnect)*recvChannels));</code> | 失败时可能提前return，成功才继续；优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。 |
| 57 | 1 / 调用 / 计算 / 写入 | <code>TIME_STOP(2);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 58 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(3);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 59 | 1 / 循环 | <code>初始 int c=0;；继续条件 c&lt;MAXCHANNELS；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 60 | 2 / 条件判断 | <code>(sendMask & (1&lt;&lt;c))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 61 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 62 | 4 / 准备局部状态 | <code>struct ncclConnector* conn = comm-&gt;channels[c].peers[sendPeer].send + connIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 63 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(conn-&gt;transportComm-&gt;connect(comm, sendData++, 1, comm-&gt;rank, conn));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 64 | 4 / 调用 / 计算 / 写入 | <code>conn-&gt;connected = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 65 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaMemcpyAsync(comm-&gt;channels[c].devPeers[sendPeer].send+connIndex, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice, transportSetupStream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 66 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 67 | 1 / 调用 / 计算 / 写入 | <code>TIME_STOP(3);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 68 | 1 / 调用 / 计算 / 写入 | <code>TIME_START(4);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 69 | 1 / 循环 | <code>初始 int c=0;；继续条件 c&lt;MAXCHANNELS；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 70 | 2 / 条件判断 | <code>(recvMask & (1&lt;&lt;c))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 71 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 72 | 4 / 准备局部状态 | <code>struct ncclConnector* conn = comm-&gt;channels[c].peers[recvPeer].recv + connIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 73 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(conn-&gt;transportComm-&gt;connect(comm, recvData++, 1, comm-&gt;rank, conn));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 74 | 4 / 调用 / 计算 / 写入 | <code>conn-&gt;connected = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 75 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaMemcpyAsync(comm-&gt;channels[c].devPeers[recvPeer].recv+connIndex, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice, transportSetupStream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 76 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 77 | 1 / 调用 / 计算 / 写入 | <code>TIME_STOP(4);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 78 | 1 / 调用 / 计算 / 写入 | <code>comm-&gt;connectRecv[recvPeer] = comm-&gt;connectSend[sendPeer] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 80 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamSynchronize(transportSetupStream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 81 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaStreamDestroy(transportSetupStream));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 82 | 0 / 条件判断 | <code>(highestTransportType != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 83 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 84 | 2 / 调用 / 计算 / 写入 | <code>*highestTransportType = highestType;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 0 / 调用 / 计算 / 写入 | <code>TIME_PRINT("P2P Setup/Connect");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 86 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Stream used during transport setup; need for P2P pre-connect + CUDA Graph
  cudaStream_t transportSetupStream;
  CUDACHECK(cudaStreamCreateWithFlags(&transportSetupStream, cudaStreamNonBlocking));
  int highestType = TRANSPORT_P2P;  // track highest transport type

  struct ncclConnect data[2*MAXCHANNELS];
  for (int i=1; i<comm->nRanks; i++) {
    int bootstrapTag = (i<<8) + (graph ? graph->id+1 : 0);
    int recvPeer = (comm->rank - i + comm->nRanks) % comm->nRanks;
    int sendPeer = (comm->rank + i) % comm->nRanks;
    uint32_t recvMask = comm->connectRecv[recvPeer];
    uint32_t sendMask = comm->connectSend[sendPeer];

    struct ncclConnect* recvData = data;
    int sendChannels = 0, recvChannels = 0;
    int type;
    TIME_START(0);
    for (int c=0; c<MAXCHANNELS; c++) {
      if (recvMask & (1<<c)) {
        NCCLCHECK(selectTransport<0>(comm, graph, recvData+recvChannels++, c, recvPeer, connIndex, &type));
        if (type > highestType) highestType = type;
      }
    }
    TIME_STOP(0);
    TIME_START(1);
    struct ncclConnect* sendData = recvData+recvChannels;
    for (int c=0; c<MAXCHANNELS; c++) {
      if (sendMask & (1<<c)) {
        NCCLCHECK(selectTransport<1>(comm, graph, sendData+sendChannels++, c, sendPeer, connIndex, &type));
        if (type > highestType) highestType = type;
      }
    }
    TIME_STOP(1);

    TIME_START(2);
    if (sendPeer == recvPeer) {
      if (recvChannels+sendChannels) {
         NCCLCHECK(bootstrapSend(comm->bootstrap, recvPeer, bootstrapTag, data, sizeof(struct ncclConnect)*(recvChannels+sendChannels)));
         NCCLCHECK(bootstrapRecv(comm->bootstrap, recvPeer, bootstrapTag, data, sizeof(struct ncclConnect)*(recvChannels+sendChannels)));
         sendData = data;
         recvData = data+sendChannels;
      }
    } else {
      if (recvChannels) NCCLCHECK(bootstrapSend(comm->bootstrap, recvPeer, bootstrapTag, recvData, sizeof(struct ncclConnect)*recvChannels));
      if (sendChannels) NCCLCHECK(bootstrapSend(comm->bootstrap, sendPeer, bootstrapTag, sendData, sizeof(struct ncclConnect)*sendChannels));
      if (sendChannels) NCCLCHECK(bootstrapRecv(comm->bootstrap, sendPeer, bootstrapTag, sendData, sizeof(struct ncclConnect)*sendChannels));
      if (recvChannels) NCCLCHECK(bootstrapRecv(comm->bootstrap, recvPeer, bootstrapTag, recvData, sizeof(struct ncclConnect)*recvChannels));
    }
    TIME_STOP(2);

    TIME_START(3);
    for (int c=0; c<MAXCHANNELS; c++) {
      if (sendMask & (1<<c)) {
        struct ncclConnector* conn = comm->channels[c].peers[sendPeer].send + connIndex;
        NCCLCHECK(conn->transportComm->connect(comm, sendData++, 1, comm->rank, conn));
        conn->connected = 1;
        CUDACHECK(cudaMemcpyAsync(comm->channels[c].devPeers[sendPeer].send+connIndex, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice, transportSetupStream));
      }
    }
    TIME_STOP(3);
    TIME_START(4);
    for (int c=0; c<MAXCHANNELS; c++) {
      if (recvMask & (1<<c)) {
        struct ncclConnector* conn = comm->channels[c].peers[recvPeer].recv + connIndex;
        NCCLCHECK(conn->transportComm->connect(comm, recvData++, 1, comm->rank, conn));
        conn->connected = 1;
        CUDACHECK(cudaMemcpyAsync(comm->channels[c].devPeers[recvPeer].recv+connIndex, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice, transportSetupStream));
      }
    }
    TIME_STOP(4);
    comm->connectRecv[recvPeer] = comm->connectSend[sendPeer] = 0;
  }
  CUDACHECK(cudaStreamSynchronize(transportSetupStream));
  CUDACHECK(cudaStreamDestroy(transportSetupStream));
  if (highestTransportType != NULL) *highestTransportType = highestType;
  TIME_PRINT("P2P Setup/Connect");
  return ncclSuccess;
}
```

</details>

<a id="f1176"></a>

## F1176：ncclTransportCollNetSetup

用途：为网络归约 head 建立特殊 CollNet 连接并交换网络参与者句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int fail = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int rank = comm-&gt;rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int nranks = comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int nMasters = comm-&gt;nNodes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>int rankInCollNet = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int isMaster = (rank == masterRank) ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>struct { int collNetRank; ncclConnect connect; } sendrecvExchange;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>struct ncclPeerInfo *myInfo = comm-&gt;peerInfo+rank, *peerInfo = comm-&gt;peerInfo+nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>peerInfo-&gt;rank = nranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 准备局部状态 | <code>int support = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 条件判断 | <code>(isMaster)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetTransport.canConnect(&support, comm-&gt;topo, collNetGraph, myInfo, peerInfo));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 0 / 条件判断 | <code>(isMaster && type == collNetSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapRecv(comm-&gt;bootstrap, masterPeer, collNetGraph-&gt;id, &sendrecvExchange, sizeof(sendrecvExchange)));</code> | 失败时可能提前return，成功才继续；优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>rankInCollNet = sendrecvExchange.collNetRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "CollNet [send] : rank %d collNetRank %d collNetNranks %d received connect from rank %d", rank, rankInCollNet, nMasters, masterPeer);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 19 | 0 / 准备局部状态 | <code>struct ncclPeer* root = channel-&gt;peers+nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 0 / 准备局部状态 | <code>struct ncclConnector* conn = (type == collNetRecv) ? root-&gt;recv+type : root-&gt;send+type;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 0 / 准备局部状态 | <code>struct ncclTransportComm* transportComm = (type == collNetRecv) ? &(collNetTransport.recv) : &(collNetTransport.send);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>conn-&gt;transportComm = transportComm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 准备局部状态 | <code>struct ncclConnect myConnect;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 0 / 条件判断 | <code>(isMaster && support)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(transportComm-&gt;setup(comm, collNetGraph, myInfo, peerInfo, &myConnect, conn, collNetGraphChannelId, type));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 27 | 0 / 准备局部状态 | <code>ncclResult_t res;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 0 / 准备局部状态 | <code>struct { int isMaster; ncclConnect connect; } *allConnects = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 0 / 准备局部状态 | <code>ncclConnect *masterConnects = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&masterConnects, nMasters));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 31 | 0 / 条件判断 | <code>(type == collNetRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&allConnects, nranks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>allConnects[rank].isMaster = isMaster;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>memcpy(&(allConnects[rank].connect), &myConnect, sizeof(struct ncclConnect));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(bootstrapAllGather(comm-&gt;bootstrap, allConnects, sizeof(*allConnects)), res, cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。 |
| 37 | 2 / 准备局部状态 | <code>int c = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 2 / 循环 | <code>初始 int r = 0;；继续条件 r &lt; nranks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 3 / 条件判断 | <code>(allConnects[r].isMaster)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 5 / 调用 / 计算 / 写入 | <code>memcpy(masterConnects+c, &(allConnects[r].connect), sizeof(struct ncclConnect));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 5 / 条件判断 | <code>(r == rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 7 / 调用 / 计算 / 写入 | <code>rankInCollNet = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 5 / 调用 / 计算 / 写入 | <code>c++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 48 | 2 / 条件判断 | <code>(isMaster)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 49 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 50 | 4 / 调用 / 计算 / 写入 | <code>memcpy(masterConnects+rankInCollNet, &(sendrecvExchange.connect), sizeof(struct ncclConnect));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 51 | 0 / 条件判断 | <code>(isMaster && support)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 52 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 53 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(transportComm-&gt;connect(comm, masterConnects, nMasters, rankInCollNet, conn), res, cleanup);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 54 | 2 / 准备局部状态 | <code>struct ncclPeer* devRoot = channel-&gt;devPeers+nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 55 | 2 / 准备局部状态 | <code>struct ncclConnector* devConn = (type == collNetRecv) ? devRoot-&gt;recv+type : devRoot-&gt;send+type;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaMemcpy(devConn, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice), res, cleanup);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 57 | 0 / 条件判断 | <code>(isMaster && type == collNetRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 58 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 59 | 2 / 调用 / 计算 / 写入 | <code>sendrecvExchange.collNetRank = rankInCollNet;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 2 / 调用 / 计算 / 写入 | <code>memcpy(&sendrecvExchange.connect, masterConnects+rankInCollNet, sizeof(struct ncclConnect));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 61 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(bootstrapSend(comm-&gt;bootstrap, masterPeer, collNetGraph-&gt;id, &sendrecvExchange, sizeof(sendrecvExchange)), res, cleanup);</code> | 失败时保存错误并跳到清理标签，成功才继续；向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。 |
| 62 | 2 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "CollNet [recv] : rank %d collNetRank %d collNetNranks %d sent connect to rank %d", rank, rankInCollNet, nMasters, masterPeer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 63 | 0 / 条件判断 | <code>(support)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 2 / 调用 / 计算 / 写入 | <code>fail = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 66 | 0 / 跳转标签 | <code>cleanup</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 67 | 1 / 条件判断 | <code>(allConnects != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 3 / 调用 / 计算 / 写入 | <code>free(allConnects);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 70 | 0 / 条件判断 | <code>(masterConnects != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 71 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 72 | 2 / 调用 / 计算 / 写入 | <code>free(masterConnects);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 73 | 0 / 函数返回 | <code>return fail;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int fail = 1;
  int rank = comm->rank;
  int nranks = comm->nRanks;
  int nMasters = comm->nNodes;
  int rankInCollNet = -1;
  int isMaster = (rank == masterRank) ? 1 : 0;
  struct {
    int collNetRank;
    ncclConnect connect;
  } sendrecvExchange;

  // check if we can connect to collnet, whose root is the nranks-th rank
  struct ncclPeerInfo *myInfo = comm->peerInfo+rank, *peerInfo = comm->peerInfo+nranks;
  peerInfo->rank = nranks;
  int support = 1;
  if (isMaster) {
    NCCLCHECK(collNetTransport.canConnect(&support, comm->topo, collNetGraph, myInfo, peerInfo));
  }

  // send master receives connect info from peer recv master
  if (isMaster && type == collNetSend) {
    NCCLCHECK(bootstrapRecv(comm->bootstrap, masterPeer, collNetGraph->id, &sendrecvExchange, sizeof(sendrecvExchange)));
    rankInCollNet = sendrecvExchange.collNetRank;
    TRACE(NCCL_INIT, "CollNet [send] : rank %d collNetRank %d collNetNranks %d received connect from rank %d", rank, rankInCollNet, nMasters, masterPeer);
  }

  // select
  struct ncclPeer* root = channel->peers+nranks;
  // connector index: 0 for recv, 1 for send
  struct ncclConnector* conn = (type == collNetRecv) ? root->recv+type : root->send+type;
  struct ncclTransportComm* transportComm = (type == collNetRecv) ? &(collNetTransport.recv) : &(collNetTransport.send);
  conn->transportComm = transportComm;
  // setup
  struct ncclConnect myConnect;
  if (isMaster && support) {
    NCCLCHECK(transportComm->setup(comm, collNetGraph, myInfo, peerInfo, &myConnect, conn, collNetGraphChannelId, type));
  }
  // prepare connect handles
  ncclResult_t res;
  struct {
    int isMaster;
    ncclConnect connect;
  } *allConnects = NULL;
  ncclConnect *masterConnects = NULL;
  NCCLCHECK(ncclCalloc(&masterConnects, nMasters));
  if (type == collNetRecv) {  // recv side: AllGather
    // all ranks must participate
    NCCLCHECK(ncclCalloc(&allConnects, nranks));
    allConnects[rank].isMaster = isMaster;
    memcpy(&(allConnects[rank].connect), &myConnect, sizeof(struct ncclConnect));
    NCCLCHECKGOTO(bootstrapAllGather(comm->bootstrap, allConnects, sizeof(*allConnects)), res, cleanup);
    // consolidate
    int c = 0;
    for (int r = 0; r < nranks; r++) {
      if (allConnects[r].isMaster) {
        memcpy(masterConnects+c, &(allConnects[r].connect), sizeof(struct ncclConnect));
        if (r == rank) rankInCollNet = c;
        c++;
      }
    }
  } else { // send side : copy in connect info received from peer recv master
    if (isMaster) memcpy(masterConnects+rankInCollNet, &(sendrecvExchange.connect), sizeof(struct ncclConnect));
  }
  // connect
  if (isMaster && support) {
    NCCLCHECKGOTO(transportComm->connect(comm, masterConnects, nMasters, rankInCollNet, conn), res, cleanup);
    struct ncclPeer* devRoot = channel->devPeers+nranks;
    struct ncclConnector* devConn = (type == collNetRecv) ? devRoot->recv+type : devRoot->send+type;
    CUDACHECKGOTO(cudaMemcpy(devConn, conn, sizeof(struct ncclConnector), cudaMemcpyHostToDevice), res, cleanup);
  }
  // recv side sends connect info to send side
  if (isMaster && type == collNetRecv) {
    sendrecvExchange.collNetRank = rankInCollNet;
    memcpy(&sendrecvExchange.connect, masterConnects+rankInCollNet, sizeof(struct ncclConnect));
    NCCLCHECKGOTO(bootstrapSend(comm->bootstrap, masterPeer, collNetGraph->id, &sendrecvExchange, sizeof(sendrecvExchange)), res, cleanup);
    TRACE(NCCL_INIT, "CollNet [recv] : rank %d collNetRank %d collNetNranks %d sent connect to rank %d", rank, rankInCollNet, nMasters, masterPeer);
  }
  if (support) fail = 0;
cleanup:
  if (allConnects != NULL) free(allConnects);
  if (masterConnects != NULL) free(masterConnects);
  return fail;
}
```

</details>

<a id="f1177"></a>

## F1177：ncclTransportCollNetCheck

用途：全组汇总 CollNet 建链失败状态，一处不支持时一致回退，避免各 rank 选择不一致。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int allGatherFailures[NCCL_MAX_LOCAL_RANKS] = {0};</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>allGatherFailures[comm-&gt;localRank] = collNetSetupFail;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapIntraNodeAllGather(comm-&gt;bootstrap, comm-&gt;localRankToRank, comm-&gt;localRank, comm-&gt;localRanks, allGatherFailures, sizeof(int)));</code> | 失败时可能提前return，成功才继续；只在所选节点内 rank 集合交换固定大小控制信息，按局部索引映射到全局 rank。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;localRanks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 条件判断 | <code>(allGatherFailures[i] != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>collNetSetupFail = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 条件判断 | <code>(collNetSetupFail)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 条件判断 | <code>(comm-&gt;localRank == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>WARN("Cannot initialize CollNet, using point-to-point network instead");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // AllGather collNet setup results
  int allGatherFailures[NCCL_MAX_LOCAL_RANKS] = {0};
  allGatherFailures[comm->localRank] = collNetSetupFail;
  NCCLCHECK(bootstrapIntraNodeAllGather(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, allGatherFailures, sizeof(int)));
  for (int i=0; i<comm->localRanks; i++) {
    if (allGatherFailures[i] != 0) {
      collNetSetupFail = 1;
      break;
    }
  }
  if (collNetSetupFail) {
    if (comm->localRank == 0) WARN("Cannot initialize CollNet, using point-to-point network instead");
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f1178"></a>

## F1178：ncclTransportCollNetFree

用途：关闭已创建的 CollNet 特殊连接，配合初始化失败回退或整体销毁。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;nChannels；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclChannel* channel = comm-&gt;channels+r;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 准备局部状态 | <code>struct ncclPeer* peer = channel-&gt;peers+comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 循环 | <code>初始 int b=0;；继续条件 b&lt;NCCL_MAX_CONNS；每轮更新 b++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 2 / 准备局部状态 | <code>struct ncclConnector* send = peer-&gt;send + b;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 条件判断 | <code>(send-&gt;transportResources && send-&gt;transportComm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(send-&gt;transportComm-&gt;free(send));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>send-&gt;transportResources = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 1 / 循环 | <code>初始 int b=0;；继续条件 b&lt;NCCL_MAX_CONNS；每轮更新 b++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 2 / 准备局部状态 | <code>struct ncclConnector* recv = peer-&gt;recv + b;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 2 / 条件判断 | <code>(recv-&gt;transportResources && recv-&gt;transportComm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(recv-&gt;transportComm-&gt;free(recv));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>recv-&gt;transportResources = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Free collNet resources
  for (int r=0; r<comm->nChannels; r++) {
    struct ncclChannel* channel = comm->channels+r;
    struct ncclPeer* peer = channel->peers+comm->nRanks;
    for (int b=0; b<NCCL_MAX_CONNS; b++) {
      struct ncclConnector* send = peer->send + b;
      if (send->transportResources && send->transportComm) NCCLCHECK(send->transportComm->free(send));
      send->transportResources = NULL; // avoid double free
    }
    for (int b=0; b<NCCL_MAX_CONNS; b++) {
      struct ncclConnector* recv = peer->recv + b;
      if (recv->transportResources && recv->transportComm) NCCLCHECK(recv->transportComm->free(recv));
      recv->transportResources = NULL; // avoid double free
    }
  }
  return ncclSuccess;
}
```

</details>

