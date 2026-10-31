# src/transport/coll_net.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1052"></a>

## F1052：canConnect

用途：由本文件 transport 的拓扑/主机/后端支持条件判断这对参与者是否可使用该传输。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*ret = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *ret = 1;
  return ncclSuccess;
}
```

</details>

<a id="f1053"></a>

## F1053：sendSetup

用途：为对应方向选择设备/代理并准备本地控制资源，生成可经 bootstrap 交换的建链句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct setupReq req;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int proxyRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNetDev(comm, myInfo-&gt;rank, graph, channelId, -1, &req.netDev, &proxyRank));</code> | 失败时可能提前return，成功才继续；按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckGdr(comm-&gt;topo, myInfo-&gt;busId, req.netDev, 1, &req.useGdr));</code> | 失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.direct &#124;= req.useGdr ? NCCL_DIRECT_NIC : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetLocalRank(comm-&gt;topo, myInfo-&gt;rank, &send-&gt;proxyConn.localRank));</code> | 失败时可能提前return，成功才继续；把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_COLLNET, 1, myInfo-&gt;rank, &send-&gt;proxyConn));</code> | 失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&send-&gt;proxyConn, ncclProxyMsgSetup, &req, sizeof(req), NULL, 0));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET,"CollNet %02d : %d [send] via COLLNET/%s/%d%s", channelId, myInfo-&gt;rank, collNetName(), req.netDev, req.useGdr ? "/GDRDMA" : "");</code> | 读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct setupReq req;

  int proxyRank;
  NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, -1, &req.netDev, &proxyRank));
  NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 1, &req.useGdr));
  send->conn.direct |= req.useGdr ? NCCL_DIRECT_NIC : 0;

  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &send->proxyConn.localRank));
  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_COLLNET, 1, myInfo->rank, &send->proxyConn));
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), NULL, 0));

  INFO(NCCL_INIT|NCCL_NET,"CollNet %02d : %d [send] via COLLNET/%s/%d%s", channelId, myInfo->rank, collNetName(), req.netDev,
      req.useGdr ? "/GDRDMA" : "");
  return ncclSuccess;
}
```

</details>

<a id="f1054"></a>

## F1054：recvSetup

用途：为对应方向选择设备/代理并准备本地控制资源，生成可经 bootstrap 交换的建链句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct setupReq req;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int proxyRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNetDev(comm, myInfo-&gt;rank, graph, channelId, -1, &req.netDev, &proxyRank));</code> | 失败时可能提前return，成功才继续；按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckGdr(comm-&gt;topo, myInfo-&gt;busId, req.netDev, 0, &req.useGdr));</code> | 失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.direct &#124;= req.useGdr ? NCCL_DIRECT_NIC : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetLocalRank(comm-&gt;topo, myInfo-&gt;rank, &recv-&gt;proxyConn.localRank));</code> | 失败时可能提前return，成功才继续；把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_COLLNET, 0, myInfo-&gt;rank, &recv-&gt;proxyConn));</code> | 失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。 |
| 8 | 0 / 准备局部状态 | <code>struct collNetRecvConnectInfo* info = (struct collNetRecvConnectInfo*) connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&recv-&gt;proxyConn, ncclProxyMsgSetup, &req, sizeof(req), &info-&gt;collNetHandle, sizeof(collNetHandle_t)));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET,"CollNet %02d : %d [receive] via COLLNET/%s/%d%s", channelId, myInfo-&gt;rank, collNetName(), req.netDev, req.useGdr ? "/GDRDMA" : "");</code> | 读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct setupReq req;

  int proxyRank;
  NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, -1, &req.netDev, &proxyRank));
  NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 0, &req.useGdr));
  recv->conn.direct |= req.useGdr ? NCCL_DIRECT_NIC : 0;

  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &recv->proxyConn.localRank));
  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_COLLNET, 0, myInfo->rank, &recv->proxyConn));
  struct collNetRecvConnectInfo* info = (struct collNetRecvConnectInfo*) connectInfo;
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), &info->collNetHandle, sizeof(collNetHandle_t)));

  INFO(NCCL_INIT|NCCL_NET,"CollNet %02d : %d [receive] via COLLNET/%s/%d%s", channelId, myInfo->rank, collNetName(), req.netDev,
      req.useGdr ? "/GDRDMA" : "");
  return ncclSuccess;
}
```

</details>

<a id="f1055"></a>

## F1055：collNetDumpMap

用途：打印后端各内存 bank 的 CPU/GPU 地址、大小和映射偏移，辅助检查跨进程地址视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>printf("Dump map\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct connectMapMem *mem = map-&gt;mems+NCCL_NET_MAP_HOSTMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>printf("Mem 0: Host mem (%x B) CPU %p GPU %p\n", mem-&gt;size, mem-&gt;cpuPtr, mem-&gt;gpuPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>mem = map-&gt;mems+NCCL_NET_MAP_DEVMEM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>printf("Mem 1: Vid mem CPU (%x B) %p GPU %p\n", mem-&gt;size, mem-&gt;cpuPtr, mem-&gt;gpuPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>mem = map-&gt;mems+NCCL_NET_MAP_SHARED_HOSTMEM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>printf("Mem 2: Shared Host mem (%x B) CPU %p GPU %p\n", mem-&gt;size, mem-&gt;cpuPtr, mem-&gt;gpuPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>mem = map-&gt;mems+NCCL_NET_MAP_SHARED_DEVMEM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>printf("Mem 3: Shared Vid (%x B) mem CPU %p GPU %p\n", mem-&gt;size, mem-&gt;cpuPtr, mem-&gt;gpuPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>printf("SendMem -&gt; Used %d Bank %d Offset %x, cpu %p gpu %p\n", map-&gt;offsets.sendMem & NCCL_NET_MAP_MASK_USED ? 1 : 0, NCCL_NET_MAP_OFFSET_BANK(map, sendMem), map-&gt;offsets.sendMem & NCCL_NET_MAP_MASK_OFFSET, NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem), NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>printf("RecvMem -&gt; Used %d Bank %d Offset %x, cpu %p gpu %p\n", map-&gt;offsets.recvMem & NCCL_NET_MAP_MASK_USED ? 1 : 0, NCCL_NET_MAP_OFFSET_BANK(map, recvMem), map-&gt;offsets.recvMem & NCCL_NET_MAP_MASK_OFFSET, NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem), NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>printf("Proto %d -&gt; Used %d Bank %d Offset %x, cpu %p, gpu %p\n", p, map-&gt;offsets.buffs[p] & NCCL_NET_MAP_MASK_USED ? 1 : 0, NCCL_NET_MAP_OFFSET_BANK(map, buffs[p]), map-&gt;offsets.buffs[p] & NCCL_NET_MAP_MASK_OFFSET, NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]), NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>printf("End of dump\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  printf("Dump map\n");
  struct connectMapMem *mem = map->mems+NCCL_NET_MAP_HOSTMEM;
  printf("Mem 0: Host mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_DEVMEM;
  printf("Mem 1: Vid  mem CPU (%x B) %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_SHARED_HOSTMEM;
  printf("Mem 2: Shared Host mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_SHARED_DEVMEM;
  printf("Mem 3: Shared Vid  (%x B) mem CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  printf("SendMem -> Used %d Bank %d Offset %x, cpu %p gpu %p\n",
      map->offsets.sendMem & NCCL_NET_MAP_MASK_USED ? 1 : 0,
      NCCL_NET_MAP_OFFSET_BANK(map, sendMem), map->offsets.sendMem & NCCL_NET_MAP_MASK_OFFSET,
      NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem), NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem));
  printf("RecvMem -> Used %d Bank %d Offset %x, cpu %p gpu %p\n",
      map->offsets.recvMem & NCCL_NET_MAP_MASK_USED ? 1 : 0,
      NCCL_NET_MAP_OFFSET_BANK(map, recvMem), map->offsets.recvMem & NCCL_NET_MAP_MASK_OFFSET,
      NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem), NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem));
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    printf("Proto %d -> Used %d Bank %d Offset %x, cpu %p, gpu %p\n", p,
        map->offsets.buffs[p] & NCCL_NET_MAP_MASK_USED ? 1 : 0,
        NCCL_NET_MAP_OFFSET_BANK(map, buffs[p]), map->offsets.buffs[p] & NCCL_NET_MAP_MASK_OFFSET,
        NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]), NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]));
  }
  printf("End of dump\n");
  return ncclSuccess;
}
```

</details>

<a id="f1056"></a>

## F1056：sendConnect

用途：消费远端句柄完成对应方向的连接和地址映射，把协议缓冲/进度指针填写到设备连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct collNetConnectArgs args = { rank, nranks, connectInfos };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct connectMap* map;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&send-&gt;proxyConn, ncclProxyMsgConnect, &args, sizeof(struct collNetConnectArgs), &map, sizeof(struct connectMap*)));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 4 | 0 / 条件判断 | <code>(map == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>void* gdcMem = map-&gt;mems[NCCL_NET_MAP_GDCMEM].gpuPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.head = gdcMem ? (uint64_t*)gdcMem : &sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 10 | 0 / 准备局部状态 | <code>struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.tail = &recvMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.sizesFifo = recvMem-&gt;sizesFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_STEPS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>send-&gt;conn.sizesFifo[i] = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.offsFifo = recvMem-&gt;offsFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 18 | 1 / 调用 / 计算 / 写入 | <code>send-&gt;conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 20 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // We're on the same process as the proxy. We can pass a pointer to a struct.
  struct collNetConnectArgs args = { rank, nranks, connectInfos };
  struct connectMap* map;
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgConnect, &args, sizeof(struct collNetConnectArgs), &map, sizeof(struct connectMap*)));

  // If collnet connect failed, propagate error to fallback on regular p2p
  if (map == NULL) return ncclSystemError;

  //NCCLCHECK(collNetDumpMap(map));

  struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);
  void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;
  send->conn.head = gdcMem ? (uint64_t*)gdcMem : &sendMem->head;

  struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);
  send->conn.tail = &recvMem->tail;
  send->conn.sizesFifo = recvMem->sizesFifo;
  for (int i=0; i<NCCL_STEPS; i++) send->conn.sizesFifo[i] = -1;
  send->conn.offsFifo = recvMem->offsFifo;

  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    send->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);
  return ncclSuccess;
}
```

</details>

<a id="f1057"></a>

## F1057：recvConnect

用途：消费远端句柄完成对应方向的连接和地址映射，把协议缓冲/进度指针填写到设备连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct collNetConnectArgs args = { rank, nranks, connectInfos };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct connectMap* map;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&recv-&gt;proxyConn, ncclProxyMsgConnect, &args, sizeof(struct collNetConnectArgs), &map, sizeof(struct connectMap*)));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 4 | 0 / 条件判断 | <code>(map == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.head = &sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 9 | 0 / 准备局部状态 | <code>struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>void* gdcMem = map-&gt;mems[NCCL_NET_MAP_GDCMEM].gpuPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.tail = gdcMem ? (uint64_t*)gdcMem : &recvMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.offsFifo = recvMem-&gt;offsFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // We're on the same process as the proxy. We can pass a pointer to a struct.
  struct collNetConnectArgs args = { rank, nranks, connectInfos };
  struct connectMap* map;
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgConnect, &args, sizeof(struct collNetConnectArgs), &map, sizeof(struct connectMap*)));

  // If collnet connect failed, propagate error to fallback on regular p2p
  if (map == NULL) return ncclSystemError;

  //NCCLCHECK(collNetDumpMap(map));

  struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);
  recv->conn.head = &sendMem->head;

  struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);
  void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;
  recv->conn.tail = gdcMem ? (uint64_t*)gdcMem : &recvMem->tail;
  recv->conn.offsFifo = recvMem->offsFifo;

  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    recv->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);
  }
  return ncclSuccess;
}
```

</details>

<a id="f1058"></a>

## F1058：sendFree

用途：释放对应逻辑连接持有的本地资源/映射，并按代理与共享计数规则释放后台对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return ncclSuccess;
}
```

</details>

<a id="f1059"></a>

## F1059：recvFree

用途：释放对应逻辑连接持有的本地资源/映射，并按代理与共享计数规则释放后台对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return ncclSuccess;
}
```

</details>

<a id="f1060"></a>

## F1060：sendProxySetup

用途：在实际代理进程中准备对应方向网络设备与连接资源；接收方向通常还要 listen 生成句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct setupReq* req = (struct setupReq*)reqBuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(reqSize != sizeof(struct setupReq))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct sendResources* resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;shared = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;netDev = req-&gt;netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;useGdr = req-&gt;useGdr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct setupReq* req = (struct setupReq*)reqBuff;
  if (reqSize != sizeof(struct setupReq)) return ncclInternalError;

  struct sendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  connection->transportResources = resources;
  connection->shared = 1;

  resources->netDev = req->netDev;
  resources->useGdr = req->useGdr;
  return ncclSuccess;
}
```

</details>

<a id="f1061"></a>

## F1061：sharedListen

用途：按网络设备复用 CollNet 监听对象并返回可交换网络句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct sharedResources* resources = (struct sharedResources*)comm-&gt;proxyState.progressState.collNet.resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(resources == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;proxyState.progressState.collNet.resources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 条件判断 | <code>(resources-&gt;collNetComms[netDev] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetListen(netDev, collNetHandle, resources-&gt;collNetListenComms+netDev));</code> | 失败时可能提前return，成功才继续；创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct sharedResources* resources = (struct sharedResources*)comm->proxyState.progressState.collNet.resources;
  if (resources == NULL) {
    NCCLCHECK(ncclCalloc(&resources, 1));
    comm->proxyState.progressState.collNet.resources = resources;
  }
  if (resources->collNetComms[netDev] == NULL)
    NCCLCHECK(collNetListen(netDev, collNetHandle, resources->collNetListenComms+netDev));
  return ncclSuccess;
}
```

</details>

<a id="f1062"></a>

## F1062：sharedConnect

用途：按设备引用计数复用 CollNet 集合连接，用完整网络参与句柄数组完成首次连接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct sharedResources* resources = (struct sharedResources*)comm-&gt;proxyState.progressState.collNet.resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(resources-&gt;collNetComms[netDev] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 准备局部状态 | <code>collNetHandle_t** handlePtrs = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&handlePtrs, nranks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 6 | 2 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 3 / 准备局部状态 | <code>struct collNetRecvConnectInfo* info = (struct collNetRecvConnectInfo*)(connectInfos+i);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>handlePtrs[i] = &(info-&gt;collNetHandle);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 2 / 准备局部状态 | <code>ncclResult_t ret = collNetConnect((void**)handlePtrs, nranks, rank, resources-&gt;collNetListenComms[netDev], resources-&gt;collNetComms+netDev);</code> | 推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>free(handlePtrs);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 2 / 条件判断 | <code>(ret == ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetCloseListen(resources-&gt;collNetListenComms[netDev]));</code> | 失败时可能提前return，成功才继续；关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 15 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>resources-&gt;collNetListenComms[netDev] = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>*collNetComm = resources-&gt;collNetComms[netDev];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 条件判断 | <code>(*collNetComm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>resources-&gt;commRefCount[netDev]++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct sharedResources* resources = (struct sharedResources*)comm->proxyState.progressState.collNet.resources;
  if (resources->collNetComms[netDev] == NULL) {
    // Connect to coll comm
    collNetHandle_t** handlePtrs = NULL;
    NCCLCHECK(ncclCalloc(&handlePtrs, nranks));
    for (int i = 0; i < nranks; i++) {
      struct collNetRecvConnectInfo* info = (struct collNetRecvConnectInfo*)(connectInfos+i);
      handlePtrs[i] = &(info->collNetHandle);
    }
    ncclResult_t ret = collNetConnect((void**)handlePtrs, nranks, rank,
          resources->collNetListenComms[netDev],
          resources->collNetComms+netDev);
    free(handlePtrs);
    if (ret == ncclSuccess) {
      // Close listen comm
      NCCLCHECK(collNetCloseListen(resources->collNetListenComms[netDev]));
    } else {
      resources->collNetListenComms[netDev] = NULL;
    }
  }
  *collNetComm = resources->collNetComms[netDev];
  if (*collNetComm) resources->commRefCount[netDev]++;
  return ncclSuccess;
}
```

</details>

<a id="f1063"></a>

## F1063：sharedFree

用途：减小共享 CollNet 连接引用，最后一个用户释放时才关闭网络集合对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct sharedResources* resources = (struct sharedResources*)comm-&gt;proxyState.progressState.collNet.resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;commRefCount[netDev]--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 条件判断 | <code>(resources-&gt;commRefCount[netDev] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetCloseColl(resources-&gt;collNetComms[netDev]));</code> | 失败时可能提前return，成功才继续；关闭集合网络连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 6 | 0 / 循环 | <code>初始 int n=0;；继续条件 n&lt;NCCL_MAX_NETDEVS；每轮更新 n++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 条件判断 | <code>(resources-&gt;commRefCount[n])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;proxyState.progressState.collNet.resources = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>free(resources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct sharedResources* resources = (struct sharedResources*)comm->proxyState.progressState.collNet.resources;
  resources->commRefCount[netDev]--;
  if (resources->commRefCount[netDev] == 0) {
    NCCLCHECK(collNetCloseColl(resources->collNetComms[netDev]));
  }
  for (int n=0; n<NCCL_MAX_NETDEVS; n++) if (resources->commRefCount[n]) return ncclSuccess;
  comm->proxyState.progressState.collNet.resources = NULL;
  free(resources);
  return ncclSuccess;
}
```

</details>

<a id="f1064"></a>

## F1064：sharedBuffersInit

用途：按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxySharedCollNet* state = &comm-&gt;proxyState.progressState.collNet;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;size == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>state-&gt;size = 2*comm-&gt;nChannels*comm-&gt;buffSizes[NCCL_PROTO_SIMPLE];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>*size = state-&gt;size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 条件判断 | <code>(cuda && state-&gt;cudaBuff == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaCalloc(&state-&gt;cudaBuff, *size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 0 / 条件判断 | <code>(!cuda && state-&gt;hostBuff == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostCalloc(&state-&gt;hostBuff, *size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>*gpuPtr = *cpuPtr = cuda ? state-&gt;cudaBuff : state-&gt;hostBuff;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxySharedCollNet* state = &comm->proxyState.progressState.collNet;
  if (state->size == 0) {
    state->size = 2*comm->nChannels*comm->buffSizes[NCCL_PROTO_SIMPLE];
  }

  *size = state->size;

  if (cuda && state->cudaBuff == NULL) {
    NCCLCHECK(ncclCudaCalloc(&state->cudaBuff, *size));
  }
  if (!cuda && state->hostBuff == NULL) {
    NCCLCHECK(ncclCudaHostCalloc(&state->hostBuff, *size));
  }
  *gpuPtr = *cpuPtr = cuda ? state->cudaBuff : state->hostBuff;
  return ncclSuccess;
}
```

</details>

<a id="f1065"></a>

## F1065：sharedBuffersGet

用途：按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int slotSize = comm-&gt;buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int globalSlot = (type*NCCL_STEPS+slot)*comm-&gt;nChannels+channel;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*offset = slotSize * globalSlot;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Use different pools for different channels and also separate send/recv.
  int slotSize = comm->buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS;
  int globalSlot = (type*NCCL_STEPS+slot)*comm->nChannels+channel;
  *offset = slotSize * globalSlot;
  return ncclSuccess;
}
```

</details>

<a id="f1066"></a>

## F1066：sharedBuffersDestroy

用途：降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclProxySharedCollNet* state = &comm-&gt;proxyState.progressState.collNet;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;size == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(state-&gt;cudaBuff));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostFree(state-&gt;hostBuff));</code> | 失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;size = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclProxySharedCollNet* state = &comm->proxyState.progressState.collNet;
  if (state->size == 0) return ncclSuccess;
  CUDACHECK(cudaFree(state->cudaBuff));
  NCCLCHECK(ncclCudaHostFree(state->hostBuff));
  // This will be called multiple times, with multiple channels and send/recv. Make sure we only do it once.
  state->size = 0;
  return ncclSuccess;
}
```

</details>

<a id="f1067"></a>

## F1067：recvProxySetup

用途：在实际代理进程中准备对应方向网络设备与连接资源；接收方向通常还要 listen 生成句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct setupReq* req = (struct setupReq*)reqBuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(reqSize != sizeof (struct setupReq))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct recvResources* resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;shared = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;netDev = req-&gt;netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;useGdr = req-&gt;useGdr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 准备局部状态 | <code>collNetHandle_t* netHandle = (collNetHandle_t*) respBuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 条件判断 | <code>(respSize != sizeof(collNetHandle_t))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedListen(comm, req-&gt;netDev, netHandle));</code> | 失败时可能提前return，成功才继续；按网络设备复用 CollNet 监听对象并返回可交换网络句柄。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct setupReq* req = (struct setupReq*)reqBuff;
  if (reqSize != sizeof (struct setupReq)) return ncclInternalError;

  struct recvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  connection->transportResources = resources;
  connection->shared = 1;

  resources->netDev = req->netDev;
  resources->useGdr = req->useGdr;

  collNetHandle_t* netHandle = (collNetHandle_t*) respBuff;
  if (respSize != sizeof(collNetHandle_t)) return ncclInternalError;

  NCCLCHECK(sharedListen(comm, req->netDev, netHandle));
  return ncclSuccess;
}
```

</details>

<a id="f1068"></a>

## F1068：sendProxyConnect

用途：在代理侧异步完成后端 connect/accept、协议缓冲分配/共享/注册，并返回主线程需要的映射信息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(reqSize != sizeof(struct collNetConnectArgs))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("sendProxyConnect: reqSize is %d != %ld\n", reqSize, sizeof(struct collNetConnectArgs));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct collNetConnectArgs* args = (struct collNetConnectArgs*)reqBuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct collNetSendConnectInfo* info = (struct collNetSendConnectInfo*)(args-&gt;connectInfos+args-&gt;rank);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>struct sendResources* resources = (struct sendResources*)(connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;collNetRank = args-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;reqFifo = (struct reqSlot (*)[NCCL_STEPS])(info-&gt;reqFifo);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>resources-&gt;recvMhandles[p] = info-&gt;mhandles[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedConnect(comm, resources-&gt;netDev, args-&gt;connectInfos, args-&gt;nranks, args-&gt;rank, &resources-&gt;collNetComm));</code> | 失败时可能提前return，成功才继续；按设备引用计数复用 CollNet 集合连接，用完整网络参与句柄数组完成首次连接。 |
| 14 | 0 / 条件判断 | <code>(respSize != sizeof(struct connectMap*))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>WARN("sendProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 0 / 条件判断 | <code>(resources-&gt;collNetComm == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>*((struct connectMap**)respBuff) = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;proxyAppendPtr = comm-&gt;proxyState.progressState.collNet.proxyAppend+2*resources-&gt;netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 准备局部状态 | <code>struct connectMap* map = &resources-&gt;map;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostCalloc(&map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map-&gt;mems[NCCL_NET_MAP_HOSTMEM].size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 0 / 条件判断 | <code>(ncclGdrCopy && ncclParamGdrCopySyncEnable())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 2 / 准备局部状态 | <code>uint64_t *cpuPtr, *gpuPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources-&gt;gdrDesc));</code> | 失败时可能提前return，成功才继续；分配显存并建立 GDRCopy 的 CPU 映射，返回 CUDA 地址、CPU 可写映射与释放所需描述符。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>resources-&gt;gdcSync = cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 2 / 准备局部状态 | <code>struct connectMapMem* gdcMem = map-&gt;mems+NCCL_NET_MAP_GDCMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;cpuPtr = (char*)cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;gpuPtr = (char*)gpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;size = sizeof(uint64_t);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;sendMem-&gt;head = -NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 40 | 0 / 准备局部状态 | <code>int bank = resources-&gt;useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 0 / 准备局部状态 | <code>struct connectMapMem* mapMem = map-&gt;mems+bank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersInit(comm, resources-&gt;useGdr, &mapMem-&gt;gpuPtr, &mapMem-&gt;cpuPtr, &mapMem-&gt;size));</code> | 失败时可能提前return，成功才继续；按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 1, resources-&gt;useGdr, mapMem-&gt;size, buffs[NCCL_PROTO_SIMPLE]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetRegMr(resources-&gt;collNetComm, mapMem-&gt;cpuPtr, mapMem-&gt;size, resources-&gt;useGdr ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources-&gt;sendMhandles[NCCL_PROTO_SIMPLE]));</code> | 失败时可能提前return，成功才继续；注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>*((struct connectMap**)respBuff) = &resources-&gt;map;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (reqSize != sizeof(struct collNetConnectArgs)) { WARN("sendProxyConnect: reqSize is %d != %ld\n", reqSize, sizeof(struct collNetConnectArgs)); return ncclInternalError; }
  struct collNetConnectArgs* args = (struct collNetConnectArgs*)reqBuff;
  struct collNetSendConnectInfo* info = (struct collNetSendConnectInfo*)(args->connectInfos+args->rank);

  struct sendResources* resources = (struct sendResources*)(connection->transportResources);

  // Get info from recv side
  resources->collNetRank = args->rank;
  resources->reqFifo = (struct reqSlot (*)[NCCL_STEPS])(info->reqFifo);

  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    resources->recvMhandles[p] = info->mhandles[p];

  NCCLCHECK(sharedConnect(comm, resources->netDev, args->connectInfos, args->nranks, args->rank, &resources->collNetComm));

  // Collnet connect is allowed to fail. Gracefully handle that case by returning NULL to the caller.
  if (respSize != sizeof(struct connectMap*)) { WARN("sendProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*)); return ncclInternalError; }
  if (resources->collNetComm == NULL) {
    *((struct connectMap**)respBuff) = NULL;
    return ncclSuccess;
  }
  connection->proxyAppendPtr = comm->proxyState.progressState.collNet.proxyAppend+2*resources->netDev;

  struct connectMap* map = &resources->map;

  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);
  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);

  NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
  map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;
  if (ncclGdrCopy && ncclParamGdrCopySyncEnable()) {
    uint64_t *cpuPtr, *gpuPtr;
    NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources->gdrDesc));

    resources->gdcSync = cpuPtr;
    struct connectMapMem* gdcMem = map->mems+NCCL_NET_MAP_GDCMEM;
    gdcMem->cpuPtr = (char*)cpuPtr;
    gdcMem->gpuPtr = (char*)gpuPtr;
    gdcMem->size = sizeof(uint64_t); // sendMem->head
  }

  resources->sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);
  resources->recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);
  // Don't give credits yet in shared mode.
  resources->sendMem->head = -NCCL_STEPS;

  // Allocate & Register shared buffers for the Simple protocol
  int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;
  struct connectMapMem* mapMem = map->mems+bank;
  NCCLCHECK(sharedBuffersInit(comm, resources->useGdr, &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size));
  NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);

  NCCLCHECK(collNetRegMr(resources->collNetComm, mapMem->cpuPtr, mapMem->size,
        resources->useGdr ? NCCL_PTR_CUDA : NCCL_PTR_HOST,
        &resources->sendMhandles[NCCL_PROTO_SIMPLE]));

  *((struct connectMap**)respBuff) = &resources->map;
  return ncclSuccess;
}
```

</details>

<a id="f1069"></a>

## F1069：recvProxyConnect

用途：在代理侧异步完成后端 connect/accept、协议缓冲分配/共享/注册，并返回主线程需要的映射信息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(reqSize != sizeof(struct collNetConnectArgs))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("recvProxyConnect: reqSize is %d != %ld\n", reqSize, sizeof(struct collNetConnectArgs));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct collNetConnectArgs* args = (struct collNetConnectArgs*)reqBuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*)(connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>struct collNetSendConnectInfo* info = (struct collNetSendConnectInfo*)(args-&gt;connectInfos+args-&gt;rank);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;collNetRank = args-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedConnect(comm, resources-&gt;netDev, args-&gt;connectInfos, args-&gt;nranks, args-&gt;rank, &resources-&gt;collNetComm));</code> | 失败时可能提前return，成功才继续；按设备引用计数复用 CollNet 集合连接，用完整网络参与句柄数组完成首次连接。 |
| 10 | 0 / 条件判断 | <code>(respSize != sizeof(struct connectMap*))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>WARN("sendProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 0 / 条件判断 | <code>(resources-&gt;collNetComm == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>*((struct connectMap**)respBuff) = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;proxyAppendPtr = comm-&gt;proxyState.progressState.collNet.proxyAppend+2*resources-&gt;netDev+1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 准备局部状态 | <code>struct connectMap* map = &resources-&gt;map;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostCalloc(&map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map-&gt;mems[NCCL_NET_MAP_HOSTMEM].size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 条件判断 | <code>(ncclGdrCopy)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 2 / 准备局部状态 | <code>uint64_t *cpuPtr, *gpuPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources-&gt;gdrDesc));</code> | 失败时可能提前return，成功才继续；分配显存并建立 GDRCopy 的 CPU 映射，返回 CUDA 地址、CPU 可写映射与释放所需描述符。 |
| 28 | 2 / 条件判断 | <code>(ncclParamGdrCopySyncEnable())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 4 / 调用 / 计算 / 写入 | <code>resources-&gt;gdcSync = cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 4 / 准备局部状态 | <code>struct connectMapMem* gdcMem = map-&gt;mems+NCCL_NET_MAP_GDCMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;cpuPtr = (char*)cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 4 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;gpuPtr = (char*)gpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 4 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;size = sizeof(uint64_t);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 2 / 条件判断 | <code>(ncclParamGdrCopyFlushEnable())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 4 / 调用 / 计算 / 写入 | <code>resources-&gt;gdcFlush = cpuPtr + 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 0 / 准备局部状态 | <code>int bank = resources-&gt;useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 0 / 准备局部状态 | <code>struct connectMapMem* mapMem = map-&gt;mems+bank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersInit(comm, resources-&gt;useGdr, &mapMem-&gt;gpuPtr, &mapMem-&gt;cpuPtr, &mapMem-&gt;size));</code> | 失败时可能提前return，成功才继续；按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 1, resources-&gt;useGdr, mapMem-&gt;size, buffs[NCCL_PROTO_SIMPLE]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetRegMr(resources-&gt;collNetComm, mapMem-&gt;cpuPtr, mapMem-&gt;size, resources-&gt;useGdr ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources-&gt;mhandles[NCCL_PROTO_SIMPLE]));</code> | 失败时可能提前return，成功才继续；注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;reqFifo = resources-&gt;reqFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 47 | 1 / 调用 / 计算 / 写入 | <code>info-&gt;mhandles[p] = resources-&gt;mhandles[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 49 | 0 / 条件判断 | <code>(respSize != sizeof(struct connectMap*))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 2 / 调用 / 计算 / 写入 | <code>WARN("recvProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 53 | 0 / 调用 / 计算 / 写入 | <code>*((struct connectMap**)respBuff) = &resources-&gt;map;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (reqSize != sizeof(struct collNetConnectArgs)) { WARN("recvProxyConnect: reqSize is %d != %ld\n", reqSize, sizeof(struct collNetConnectArgs)); return ncclInternalError; }
  struct collNetConnectArgs* args = (struct collNetConnectArgs*)reqBuff;

  struct recvResources* resources = (struct recvResources*)(connection->transportResources);
  struct collNetSendConnectInfo* info = (struct collNetSendConnectInfo*)(args->connectInfos+args->rank);
  resources->collNetRank = args->rank;

  NCCLCHECK(sharedConnect(comm, resources->netDev, args->connectInfos, args->nranks, args->rank, &resources->collNetComm));

  // Collnet connect is allowed to fail. Gracefully handle that case by returning NULL to the caller.
  if (respSize != sizeof(struct connectMap*)) { WARN("sendProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*)); return ncclInternalError; }
  if (resources->collNetComm == NULL) {
    *((struct connectMap**)respBuff) = NULL;
    return ncclSuccess;
  }
  connection->proxyAppendPtr = comm->proxyState.progressState.collNet.proxyAppend+2*resources->netDev+1;

  struct connectMap* map = &resources->map;

  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);
  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);

  NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
  map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;
  if (ncclGdrCopy) {
    uint64_t *cpuPtr, *gpuPtr;
    NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources->gdrDesc));

    if (ncclParamGdrCopySyncEnable()) {
      resources->gdcSync = cpuPtr;
      struct connectMapMem* gdcMem = map->mems+NCCL_NET_MAP_GDCMEM;
      gdcMem->cpuPtr = (char*)cpuPtr;
      gdcMem->gpuPtr = (char*)gpuPtr;
      gdcMem->size = sizeof(uint64_t);
    }
    if (ncclParamGdrCopyFlushEnable()) resources->gdcFlush = cpuPtr + 1;
  }

  resources->sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);
  resources->recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);

  // Allocate & Register shared buffers for the Simple protocol
  int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;
  struct connectMapMem* mapMem = map->mems+bank;
  NCCLCHECK(sharedBuffersInit(comm, resources->useGdr, &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size));
  NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);

  NCCLCHECK(collNetRegMr(resources->collNetComm, mapMem->cpuPtr, mapMem->size,
        resources->useGdr ? NCCL_PTR_CUDA : NCCL_PTR_HOST,
        &resources->mhandles[NCCL_PROTO_SIMPLE]));

  // Pass info to send side
  info->reqFifo = resources->reqFifo;
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    info->mhandles[p] = resources->mhandles[p];

  if (respSize != sizeof(struct connectMap*)) { WARN("recvProxyConnect: respSize is %d != %ld\n", respSize, sizeof(void*)); return ncclInternalError; }
  *((struct connectMap**)respBuff) = &resources->map;
  return ncclSuccess;
}
```

</details>

<a id="f1070"></a>

## F1070：sendProxyFree

用途：在拥有网络对象的代理侧注销内存、释放映射并按引用计数关闭 backend connection。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct sendResources* resources = (struct sendResources*)(connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(resources-&gt;sendMhandles[p])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetDeregMr(resources-&gt;collNetComm, resources-&gt;sendMhandles[p]));</code> | 失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 准备局部状态 | <code>struct connectMapMem* mems = resources-&gt;map.mems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));</code> | 失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 10 | 0 / 条件判断 | <code>(mems[NCCL_NET_MAP_GDCMEM].cpuPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaFree(resources-&gt;gdrDesc));</code> | 失败时可能提前return，成功才继续；按映射、pin、显存分配的依赖顺序撤销 GDRCopy 资源。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersDestroy(comm));</code> | 失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedFree(comm, resources-&gt;netDev));</code> | 失败时可能提前return，成功才继续；减小共享 CollNet 连接引用，最后一个用户释放时才关闭网络集合对象。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>free(connection-&gt;transportResources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct sendResources* resources = (struct sendResources*)(connection->transportResources);
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (resources->sendMhandles[p]) {
      NCCLCHECK(collNetDeregMr(resources->collNetComm, resources->sendMhandles[p]));
    }
  }
  struct connectMapMem* mems = resources->map.mems;
  NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));
  CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));
  if (mems[NCCL_NET_MAP_GDCMEM].cpuPtr) NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));
  NCCLCHECK(sharedBuffersDestroy(comm));
  NCCLCHECK(sharedFree(comm, resources->netDev));
  free(connection->transportResources);
  return ncclSuccess;
}
```

</details>

<a id="f1071"></a>

## F1071：recvProxyFree

用途：在拥有网络对象的代理侧注销内存、释放映射并按引用计数关闭 backend connection。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*)(connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(resources-&gt;mhandles[p])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetDeregMr(resources-&gt;collNetComm, resources-&gt;mhandles[p]));</code> | 失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 准备局部状态 | <code>struct connectMapMem* mems = resources-&gt;map.mems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));</code> | 失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 10 | 0 / 条件判断 | <code>(mems[NCCL_NET_MAP_GDCMEM].cpuPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaFree(resources-&gt;gdrDesc));</code> | 失败时可能提前return，成功才继续；按映射、pin、显存分配的依赖顺序撤销 GDRCopy 资源。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersDestroy(comm));</code> | 失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedFree(comm, resources-&gt;netDev));</code> | 失败时可能提前return，成功才继续；减小共享 CollNet 连接引用，最后一个用户释放时才关闭网络集合对象。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>free(connection-&gt;transportResources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct recvResources* resources = (struct recvResources*)(connection->transportResources);
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (resources->mhandles[p]) {
      NCCLCHECK(collNetDeregMr(resources->collNetComm, resources->mhandles[p]));
    }
  }
  struct connectMapMem* mems = resources->map.mems;
  NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));
  CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));
  if (mems[NCCL_NET_MAP_GDCMEM].cpuPtr) NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));
  NCCLCHECK(sharedBuffersDestroy(comm));
  NCCLCHECK(sharedFree(comm, resources->netDev));
  free(connection->transportResources);
  return ncclSuccess;
}
```

</details>

<a id="f1072"></a>

## F1072：sendProxyProgress

用途：对应后端发送状态机：准备槽位、等 GPU 数据就绪、提交网络请求、检测完成后回收；CollNet 分支提交网络归约。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(args-&gt;protocol != NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("CollNet does not support LL/LL128");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(args-&gt;state == ncclProxyOpReady)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 3 / 准备局部状态 | <code>struct sendResources* resources = (struct sendResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>sub-&gt;base = ROUNDUP(resources-&gt;step, args-&gt;chunkSteps);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>sub-&gt;posted = sub-&gt;received = sub-&gt;transmitted = sub-&gt;done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>resources-&gt;step = sub-&gt;base + sub-&gt;nsteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 13 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpProgress;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 条件判断 | <code>(args-&gt;state == ncclProxyOpProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 准备局部状态 | <code>int p = args-&gt;protocol;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 2 / 准备局部状态 | <code>int nGroups = DIVUP(args-&gt;nsubs, COLLNET_GROUP_NSUBS);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 2 / 准备局部状态 | <code>int perGroupSteps = NCCL_STEPS / nGroups;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 3 / 准备局部状态 | <code>struct sendResources* resources = (struct sendResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 3 / 准备局部状态 | <code>void* sendMhandle = resources-&gt;sendMhandles[p];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 3 / 准备局部状态 | <code>void* recvMhandle = resources-&gt;recvMhandles[p];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 3 / 准备局部状态 | <code>auto reqFifo = resources-&gt;reqFifo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 3 / 条件判断 | <code>(sub-&gt;posted &lt; sub-&gt;nsteps && sub-&gt;posted &lt; sub-&gt;done + NCCL_STEPS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;posted)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 30 | 5 / 准备局部状态 | <code>int sharedBuffSlot = sub-&gt;posted%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 31 | 5 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersGet(comm, 0, sharedBuffSlot, 0, &offset));</code> | 失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。 |
| 33 | 5 / 调用 / 计算 / 写入 | <code>resources-&gt;recvMem-&gt;offsFifo[buffSlot] = offset + s*args-&gt;chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 5 / 调用 / 计算 / 写入 | <code>__sync_synchronize();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 5 / 准备局部状态 | <code>volatile uint64_t* sendHead = resources-&gt;gdcSync ? resources-&gt;gdcSync : &resources-&gt;sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 36 | 5 / 调用 / 计算 / 写入 | <code>sub-&gt;posted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 37 | 5 / 调用 / 计算 / 写入 | <code>*sendHead = sub-&gt;base + sub-&gt;posted - NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 38 | 5 / 条件判断 | <code>(resources-&gt;gdcSync)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 7 / 调用 / 计算 / 写入 | <code>wc_store_fence();</code> | 按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。 |
| 41 | 3 / 准备局部状态 | <code>bool groupSync = (((s == 0) && ((sub+args-&gt;nsubs-1)-&gt;received == sub-&gt;received)) &#124;&#124; (s && (sub-1)-&gt;received &gt; sub-&gt;received));</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 42 | 3 / 条件判断 | <code>(groupSync && sub-&gt;received &lt; sub-&gt;posted && sub-&gt;received &lt; sub-&gt;done + perGroupSteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;received)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 45 | 5 / 准备局部状态 | <code>int sharedBuffSlot = sub-&gt;received%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 46 | 5 / 准备局部状态 | <code>volatile int* sizesFifo = resources-&gt;recvMem-&gt;sizesFifo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 47 | 5 / 准备局部状态 | <code>volatile uint64_t* recvTail = &resources-&gt;recvMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 48 | 5 / 准备局部状态 | <code>char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources-&gt;map, gpu, buffs[p]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 5 / 条件判断 | <code>(sizesFifo[buffSlot] != -1 && ((*recvTail &gt; (sub-&gt;base+sub-&gt;received))))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 7 / 准备局部状态 | <code>int ready = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 7 / 条件判断 | <code>(s == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 9 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 55 | 9 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersGet(comm, 0, sharedBuffSlot, 0, &offset));</code> | 失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。 |
| 56 | 9 / 调用 / 计算 / 写入 | <code>args-&gt;sharedBuff[sharedBuffSlot] = localBuff + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 9 / 调用 / 计算 / 写入 | <code>args-&gt;sharedSize[sharedBuffSlot] = args-&gt;chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 7 / 条件判断 | <code>(ready)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 9 / 调用 / 计算 / 写入 | <code>sizesFifo[buffSlot] = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 9 / 调用 / 计算 / 写入 | <code>sub-&gt;received += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 62 | 9 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 63 | 3 / 条件判断 | <code>(LAST_OF_GROUP(s) && (sub-&gt;transmitted &lt; sub-&gt;received))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 5 / 准备局部状态 | <code>int group = s / COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 66 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;transmitted)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 67 | 5 / 准备局部状态 | <code>int sharedBuffSlot = sub-&gt;transmitted%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 68 | 5 / 条件判断 | <code>(reqFifo[group][buffSlot].recvBuff != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 69 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 70 | 7 / 准备局部状态 | <code>int totalSize = (s-group*COLLNET_GROUP_NSUBS+1) * args-&gt;sharedSize[sharedBuffSlot];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 71 | 7 / 准备局部状态 | <code>int count = totalSize / ncclTypeSize(args-&gt;dtype);</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 72 | 7 / 调用 / 计算 / 写入 | <code>reqFifo[group][buffSlot].size = args-&gt;sharedSize[sharedBuffSlot];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 7 / 准备局部状态 | <code>char* sendAddress = (char*)args-&gt;sharedBuff[sharedBuffSlot] + group*COLLNET_GROUP_NSUBS*args-&gt;sharedSize[sharedBuffSlot];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 74 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetIallreduce(resources-&gt;collNetComm, sendAddress, (void*)(reqFifo[group][buffSlot].recvBuff), count, args-&gt;dtype, args-&gt;redOp, sendMhandle, recvMhandle, sub-&gt;requests+buffSlot));</code> | 失败时可能提前return，成功才继续；提交网络集合归约请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 75 | 7 / 条件判断 | <code>(sub-&gt;requests[buffSlot] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 76 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 77 | 9 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 78 | 7 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "sendProxy [%d/%d/%d] Iallreduce posted, size %d req %p", sub-&gt;transmitted, group, buffSlot, totalSize, sub-&gt;requests[buffSlot]);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 79 | 7 / 调用 / 计算 / 写入 | <code>__sync_synchronize();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 80 | 7 / 调用 / 计算 / 写入 | <code>sub-&gt;transmitted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 81 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 83 | 3 / 条件判断 | <code>(LAST_OF_GROUP(s) && sub-&gt;done &lt; sub-&gt;transmitted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 84 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 85 | 5 / 准备局部状态 | <code>int done, size;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 86 | 5 / 准备局部状态 | <code>int group = s / COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 87 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;done)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 88 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetTest((void*)(sub-&gt;requests[buffSlot]), &done, &size));</code> | 失败时可能提前return，成功才继续；查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 89 | 5 / 条件判断 | <code>(done)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 90 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 91 | 7 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "sendProxy [%d/%d/%d] request %p done, size %d", sub-&gt;done, group, buffSlot, sub-&gt;requests[buffSlot], size);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 92 | 7 / 调用 / 计算 / 写入 | <code>__sync_synchronize();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 93 | 7 / 调用 / 计算 / 写入 | <code>reqFifo[group][buffSlot].recvBuff = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 94 | 7 / 循环 | <code>初始 int i=group*COLLNET_GROUP_NSUBS;；继续条件 i&lt;=s；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 95 | 8 / 调用 / 计算 / 写入 | <code>args-&gt;subs[i].done += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 96 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 97 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 98 | 7 / 准备局部状态 | <code>int allDone = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 99 | 7 / 循环 | <code>初始 int i=0;；继续条件 i&lt;args-&gt;nsubs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 100 | 8 / 条件判断 | <code>(args-&gt;subs[i].done &lt; args-&gt;subs[i].nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 101 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 102 | 10 / 调用 / 计算 / 写入 | <code>allDone = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 103 | 10 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 104 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 105 | 7 / 条件判断 | <code>(allDone)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 106 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 107 | 9 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpNone;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 108 | 9 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "sendProxy [%d/%d] stopped", sub-&gt;done, s);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 109 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 110 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (args->protocol != NCCL_PROTO_SIMPLE) {
    WARN("CollNet does not support LL/LL128");
    return ncclInternalError;
  }
  if (args->state == ncclProxyOpReady) {
    for (int s=0; s<args->nsubs; s++) {
      struct ncclProxySubArgs* sub = args->subs+s;
      struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);
      // Round to next multiple of sliceSteps
      sub->base = ROUNDUP(resources->step, args->chunkSteps);
      sub->posted = sub->received = sub->transmitted = sub->done = 0;
      resources->step = sub->base + sub->nsteps;
    }
    args->state = ncclProxyOpProgress;
  }
  args->idle = 1;
  if (args->state == ncclProxyOpProgress) {
    int p = args->protocol;
    int nGroups = DIVUP(args->nsubs, COLLNET_GROUP_NSUBS);
    int perGroupSteps = NCCL_STEPS / nGroups;
    for (int s=0; s<args->nsubs; s++) {
      struct ncclProxySubArgs* sub = args->subs+s;
      struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);
      void* sendMhandle = resources->sendMhandles[p];
      void* recvMhandle = resources->recvMhandles[p];
      auto reqFifo = resources->reqFifo;
      if (sub->posted < sub->nsteps && sub->posted < sub->done + NCCL_STEPS) {
        int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
        int sharedBuffSlot = sub->posted%NCCL_STEPS;
        int offset;
        NCCLCHECK(sharedBuffersGet(comm, 0, sharedBuffSlot, 0, &offset));
        resources->recvMem->offsFifo[buffSlot] = offset + s*args->chunkSize;
        __sync_synchronize();
        volatile uint64_t* sendHead = resources->gdcSync ? resources->gdcSync : &resources->sendMem->head;
        sub->posted += args->sliceSteps;
        *sendHead = sub->base + sub->posted - NCCL_STEPS;
        if (resources->gdcSync) wc_store_fence(); // Flush out WC write
      }
      // Enforce sync between operations of the same group.
      bool groupSync = (((s == 0) && ((sub+args->nsubs-1)->received == sub->received)) || (s && (sub-1)->received > sub->received));
      if (groupSync && sub->received < sub->posted && sub->received < sub->done + perGroupSteps) {
        int buffSlot = (sub->base+sub->received)%NCCL_STEPS;
        int sharedBuffSlot = sub->received%NCCL_STEPS;
        volatile int* sizesFifo = resources->recvMem->sizesFifo;
        volatile uint64_t* recvTail = &resources->recvMem->tail;
        char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, gpu, buffs[p]);
        if (sizesFifo[buffSlot] != -1 && ((*recvTail > (sub->base+sub->received)))) {
          // We have something to receive, let's check whether data is ready.
          int ready = 1;
          if (s == 0) {
            int offset;
            NCCLCHECK(sharedBuffersGet(comm, 0, sharedBuffSlot, 0, &offset));
            args->sharedBuff[sharedBuffSlot] = localBuff + offset;
            args->sharedSize[sharedBuffSlot] = args->chunkSize;
          }
          if (ready) {
            sizesFifo[buffSlot] = -1;
            sub->received += args->sliceSteps;
            args->idle = 0;
            //continue;
          }
        }
      }
      if (LAST_OF_GROUP(s) && (sub->transmitted < sub->received)) {
        int group = s / COLLNET_GROUP_NSUBS;
        int buffSlot = (sub->base+sub->transmitted)%NCCL_STEPS;
        int sharedBuffSlot = sub->transmitted%NCCL_STEPS;
        if (reqFifo[group][buffSlot].recvBuff != NULL) {
          int totalSize = (s-group*COLLNET_GROUP_NSUBS+1) * args->sharedSize[sharedBuffSlot];
          int count = totalSize / ncclTypeSize(args->dtype);
          reqFifo[group][buffSlot].size = args->sharedSize[sharedBuffSlot];
          char* sendAddress = (char*)args->sharedBuff[sharedBuffSlot] + group*COLLNET_GROUP_NSUBS*args->sharedSize[sharedBuffSlot];
          NCCLCHECK(collNetIallreduce(resources->collNetComm, sendAddress, (void*)(reqFifo[group][buffSlot].recvBuff), count, args->dtype, args->redOp, sendMhandle, recvMhandle, sub->requests+buffSlot));
          if (sub->requests[buffSlot] == NULL) continue;

          TRACE(NCCL_NET, "sendProxy [%d/%d/%d] Iallreduce posted, size %d req %p", sub->transmitted, group, buffSlot, totalSize, sub->requests[buffSlot]);
          // Make sure size is reset to zero before we update the head.
          __sync_synchronize();
          sub->transmitted += args->sliceSteps;
          args->idle = 0;
          continue;
        }
      }
      // Check whether the network has completed some send operations.
      if (LAST_OF_GROUP(s) && sub->done < sub->transmitted) {
        int done, size;
        int group = s / COLLNET_GROUP_NSUBS;
        int buffSlot = (sub->base+sub->done)%NCCL_STEPS;
        NCCLCHECK(collNetTest((void*)(sub->requests[buffSlot]), &done, &size));
        if (done) {
          TRACE(NCCL_NET, "sendProxy [%d/%d/%d] request %p done, size %d", sub->done, group, buffSlot, sub->requests[buffSlot], size);
          // Make sure size is updated before we set recvBuff to NULL (from the view of recv proxy, concerning the flush)
          // (reordered store after store is possible on POWER, though not on x86)
          __sync_synchronize();
          reqFifo[group][buffSlot].recvBuff = NULL; // Notify recvProxy
          for (int i=group*COLLNET_GROUP_NSUBS; i<=s; i++) args->subs[i].done += args->sliceSteps;
          args->idle = 0;
          int allDone = 1;
          for (int i=0; i<args->nsubs; i++) {
            if (args->subs[i].done < args->subs[i].nsteps) { allDone = 0; break; }
          }
          if (allDone) {
            args->state = ncclProxyOpNone;
            TRACE(NCCL_NET, "sendProxy [%d/%d] stopped", sub->done, s);
          }
        }
      }
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f1073"></a>

## F1073：recvProxyProgress

用途：对应后端接收状态机：挂接收、检查网络完成、必要时 flush、发布给 GPU 并等待消费回收。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(args-&gt;protocol != NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("CollNet does not support LL/LL128");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(args-&gt;state == ncclProxyOpReady)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 3 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>sub-&gt;base = ROUNDUP(resources-&gt;step, args-&gt;chunkSteps);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>sub-&gt;posted = sub-&gt;received = sub-&gt;flushed = sub-&gt;transmitted = sub-&gt;done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>resources-&gt;step = sub-&gt;base + sub-&gt;nsteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 13 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpProgress;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 条件判断 | <code>(args-&gt;state == ncclProxyOpProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 准备局部状态 | <code>int p = args-&gt;protocol;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 2 / 准备局部状态 | <code>int nGroups = DIVUP(args-&gt;nsubs, COLLNET_GROUP_NSUBS);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 2 / 准备局部状态 | <code>int perGroupSteps = NCCL_STEPS / nGroups;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 3 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 3 / 准备局部状态 | <code>void* mhandle = resources-&gt;mhandles[p];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 3 / 准备局部状态 | <code>auto reqFifo = resources-&gt;reqFifo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 3 / 准备局部状态 | <code>char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources-&gt;map, cpu, buffs[p]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 3 / 条件判断 | <code>(LAST_OF_GROUP(s) && (sub-&gt;posted &lt; sub-&gt;done + perGroupSteps) && (sub-&gt;posted &lt; sub-&gt;nsteps))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 5 / 准备局部状态 | <code>int group = s / COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;posted)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 31 | 5 / 准备局部状态 | <code>int sharedBuffSlot = sub-&gt;posted%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 32 | 5 / 准备局部状态 | <code>int startChannel = group*COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 5 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));</code> | 失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。 |
| 35 | 5 / 调用 / 计算 / 写入 | <code>reqFifo[group][buffSlot].recvBuff = localBuff + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 5 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "recvProxy [%d/%d/%d] posted buffer %p", sub-&gt;posted, group, buffSlot, reqFifo[group][buffSlot].recvBuff);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 37 | 5 / 调用 / 计算 / 写入 | <code>sub-&gt;posted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 38 | 5 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 40 | 3 / 条件判断 | <code>(LAST_OF_GROUP(s) && (sub-&gt;posted &gt; sub-&gt;received))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 5 / 准备局部状态 | <code>int group = s / COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;received)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 44 | 5 / 准备局部状态 | <code>int sharedBuffSlot = sub-&gt;received%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 45 | 5 / 条件判断 | <code>(reqFifo[group][buffSlot].recvBuff == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;sharedSize[sharedBuffSlot] = reqFifo[group][buffSlot].size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 7 / 准备局部状态 | <code>int totalSize = args-&gt;sharedSize[sharedBuffSlot]*(s-group*COLLNET_GROUP_NSUBS+1);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 7 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "recvProxy [%d/%d/%d] received, size %d", sub-&gt;received, group, buffSlot, totalSize);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 50 | 7 / 调用 / 计算 / 写入 | <code>sub-&gt;received += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 51 | 7 / 调用 / 计算 / 写入 | <code>sub-&gt;requests[buffSlot] = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 7 / 条件判断 | <code>(reqFifo[group][buffSlot].size &gt; 0 && resources-&gt;useGdr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 9 / 条件判断 | <code>(resources-&gt;gdcFlush)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 55 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 56 | 11 / 编译期分支/指令 | <code>#if defined (__x86_64__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 57 | 12 / 调用 / 计算 / 写入 | <code>asm volatile ("mov (%0), %%eax" :: "l"(resources-&gt;gdcFlush) : "%eax");</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 58 | 12 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 59 | 13 / 调用 / 计算 / 写入 | <code>WARN("NET: GDR Flush only supported on x86_64");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 60 | 13 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 61 | 11 / 调用 / 计算 / 写入 | <code>sub-&gt;requests[buffSlot] = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 10 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 63 | 11 / 准备局部状态 | <code>int startChannel = group*COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 64 | 11 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 65 | 11 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));</code> | 失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。 |
| 66 | 11 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetIflush(resources-&gt;collNetComm, localBuff + offset, totalSize, mhandle, sub-&gt;requests+buffSlot));</code> | 失败时可能提前return，成功才继续；提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 67 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 68 | 9 / 循环 | <code>初始 int i=group*COLLNET_GROUP_NSUBS;；继续条件 i&lt;=s；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 69 | 10 / 调用 / 计算 / 写入 | <code>args-&gt;subs[i].flushed += args-&gt;sliceSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 9 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 71 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 7 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 73 | 3 / 条件判断 | <code>(LAST_OF_GROUP(s) && (sub-&gt;received &gt; sub-&gt;flushed))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 74 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 75 | 5 / 准备局部状态 | <code>int group = s / COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 76 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base + sub-&gt;flushed)%NCCL_STEPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 77 | 5 / 准备局部状态 | <code>int done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 78 | 5 / 条件判断 | <code>(sub-&gt;requests[buffSlot])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 79 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 80 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(collNetTest(sub-&gt;requests[buffSlot], &done, NULL));</code> | 失败时可能提前return，成功才继续；查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 81 | 5 / 条件判断 | <code>(done)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 82 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 83 | 7 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "recvProxy [%d/%d/%d] flushed", sub-&gt;flushed, group, buffSlot);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 84 | 7 / 循环 | <code>初始 int i=group*COLLNET_GROUP_NSUBS;；继续条件 i&lt;=s；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 85 | 8 / 调用 / 计算 / 写入 | <code>args-&gt;subs[i].flushed += args-&gt;sliceSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 86 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 87 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 88 | 3 / 条件判断 | <code>(sub-&gt;flushed &gt; sub-&gt;transmitted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 89 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 90 | 5 / 准备局部状态 | <code>int group = s / COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 91 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base + sub-&gt;transmitted)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 92 | 5 / 准备局部状态 | <code>int sharedBuffSlot = sub-&gt;transmitted%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 93 | 5 / 准备局部状态 | <code>int startChannel = group*COLLNET_GROUP_NSUBS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 94 | 5 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 95 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));</code> | 失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。 |
| 96 | 5 / 准备局部状态 | <code>volatile int* offsFifo = (volatile int*)resources-&gt;recvMem-&gt;offsFifo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 97 | 5 / 调用 / 计算 / 写入 | <code>offsFifo[buffSlot] = offset + (s%COLLNET_GROUP_NSUBS)*args-&gt;chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 98 | 5 / 调用 / 计算 / 写入 | <code>__sync_synchronize();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 99 | 5 / 准备局部状态 | <code>volatile uint64_t* recvTail = resources-&gt;gdcSync ? resources-&gt;gdcSync : &resources-&gt;recvMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 100 | 5 / 调用 / 计算 / 写入 | <code>*recvTail = sub-&gt;base + sub-&gt;flushed;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 101 | 5 / 条件判断 | <code>(resources-&gt;gdcSync)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 102 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 103 | 7 / 调用 / 计算 / 写入 | <code>wc_store_fence();</code> | 按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。 |
| 104 | 5 / 调用 / 计算 / 写入 | <code>sub-&gt;transmitted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 105 | 5 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 106 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 107 | 3 / 准备局部状态 | <code>bool groupSync = (((s == 0) && ((sub+args-&gt;nsubs-1)-&gt;done == sub-&gt;done)) &#124;&#124; (s && (sub-1)-&gt;done &gt; sub-&gt;done));</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 108 | 3 / 准备局部状态 | <code>volatile uint64_t* sendHead = &resources-&gt;sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 109 | 3 / 条件判断 | <code>(groupSync && sub-&gt;done &lt; sub-&gt;transmitted && (sub-&gt;base+sub-&gt;done) &lt; *sendHead)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 110 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 111 | 5 / 调用 / 计算 / 写入 | <code>sub-&gt;done += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 112 | 5 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 113 | 5 / 条件判断 | <code>(sub-&gt;done == sub-&gt;nsteps && s == args-&gt;nsubs-1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 114 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 115 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpNone;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 116 | 7 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "recvProxy [%d/%d] stopped", sub-&gt;done, s);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 117 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 118 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (args->protocol != NCCL_PROTO_SIMPLE) {
    WARN("CollNet does not support LL/LL128");
    return ncclInternalError;
  }
  if (args->state == ncclProxyOpReady) {
    for (int s=0; s<args->nsubs; s++) {
      struct ncclProxySubArgs* sub = args->subs+s;
      struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
      // Round to next multiple of sliceSteps
      sub->base = ROUNDUP(resources->step, args->chunkSteps);
      sub->posted = sub->received = sub->flushed = sub->transmitted = sub->done = 0;
      resources->step = sub->base + sub->nsteps;
    }
    args->state = ncclProxyOpProgress;
  }
  args->idle = 1;
  if (args->state == ncclProxyOpProgress) {
    int p = args->protocol;
    int nGroups = DIVUP(args->nsubs, COLLNET_GROUP_NSUBS);
    int perGroupSteps = NCCL_STEPS / nGroups;
    for (int s=0; s<args->nsubs; s++) {
      struct ncclProxySubArgs* sub = args->subs+s;
      struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
      void* mhandle = resources->mhandles[p];
      auto reqFifo = resources->reqFifo;
      char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);

      // Enforce sync between operations of the same group.
      if (LAST_OF_GROUP(s) && (sub->posted < sub->done + perGroupSteps) && (sub->posted < sub->nsteps)) {
        int group = s / COLLNET_GROUP_NSUBS;
        int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
        int sharedBuffSlot = sub->posted%NCCL_STEPS;
        int startChannel = group*COLLNET_GROUP_NSUBS;
        int offset;
        NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));
        reqFifo[group][buffSlot].recvBuff = localBuff + offset;
        TRACE(NCCL_NET, "recvProxy [%d/%d/%d] posted buffer %p", sub->posted, group, buffSlot, reqFifo[group][buffSlot].recvBuff);
        sub->posted += args->sliceSteps;
        args->idle = 0;
        continue;
      }
      if (LAST_OF_GROUP(s) && (sub->posted > sub->received)) {
        int group = s / COLLNET_GROUP_NSUBS;
        int buffSlot = (sub->base+sub->received)%NCCL_STEPS;
        int sharedBuffSlot = sub->received%NCCL_STEPS;
        if (reqFifo[group][buffSlot].recvBuff == NULL) { // Buffer is cleared : coll is complete
          args->sharedSize[sharedBuffSlot] = reqFifo[group][buffSlot].size;
          int totalSize = args->sharedSize[sharedBuffSlot]*(s-group*COLLNET_GROUP_NSUBS+1);
          TRACE(NCCL_NET, "recvProxy [%d/%d/%d] received, size %d", sub->received, group, buffSlot, totalSize);
          sub->received += args->sliceSteps;
          sub->requests[buffSlot] = NULL;
          if (reqFifo[group][buffSlot].size > 0 && resources->useGdr) {
            // GDRCOPY support
            if (resources->gdcFlush) {
#if defined (__x86_64__)
              // Force a PCI-E read from GPU memory
              asm volatile ("mov (%0), %%eax" :: "l"(resources->gdcFlush) : "%eax");
#else
              WARN("NET: GDR Flush only supported on x86_64");
              return ncclInternalError;
#endif
              sub->requests[buffSlot] = NULL;
            } else {
              int startChannel = group*COLLNET_GROUP_NSUBS;
              int offset;
              NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));
              NCCLCHECK(collNetIflush(resources->collNetComm, localBuff + offset, totalSize, mhandle, sub->requests+buffSlot));
            }
          } else {
            for (int i=group*COLLNET_GROUP_NSUBS; i<=s; i++) args->subs[i].flushed += args->sliceSteps;
          }
          args->idle = 0;
          continue;
        }
      }
      if (LAST_OF_GROUP(s) && (sub->received > sub->flushed)) {
        // Progress flush operations
        int group = s / COLLNET_GROUP_NSUBS;
        int buffSlot = (sub->base + sub->flushed)%NCCL_STEPS;
        int done = 1;
        if (sub->requests[buffSlot]) NCCLCHECK(collNetTest(sub->requests[buffSlot], &done, NULL));
        if (done) {
          TRACE(NCCL_NET, "recvProxy [%d/%d/%d] flushed", sub->flushed, group, buffSlot);
          for (int i=group*COLLNET_GROUP_NSUBS; i<=s; i++) args->subs[i].flushed += args->sliceSteps;
          args->idle = 0;
          //continue;
        }
      }
      if (sub->flushed > sub->transmitted) {
        int group = s / COLLNET_GROUP_NSUBS;
        int buffSlot = (sub->base + sub->transmitted)%NCCL_STEPS;
        int sharedBuffSlot = sub->transmitted%NCCL_STEPS;
        int startChannel = group*COLLNET_GROUP_NSUBS;
        int offset;
        NCCLCHECK(sharedBuffersGet(comm, 1, sharedBuffSlot, startChannel, &offset));
        volatile int* offsFifo = (volatile int*)resources->recvMem->offsFifo;
        offsFifo[buffSlot] = offset + (s%COLLNET_GROUP_NSUBS)*args->chunkSize;
        __sync_synchronize();
        volatile uint64_t* recvTail = resources->gdcSync ? resources->gdcSync : &resources->recvMem->tail;
        *recvTail = sub->base + sub->flushed;
        if (resources->gdcSync) wc_store_fence(); // Flush out WC write
        sub->transmitted += args->sliceSteps;
        args->idle = 0;
        continue;
      }
      // Enforce sync here to make sure the last sub doesn't increase "done" before all others in the group have
      // reached the same point, otherwise we would start posting buffers to the send proxy before we're done
      // processing all the shared buffer.
      bool groupSync = (((s == 0) && ((sub+args->nsubs-1)->done == sub->done)) || (s && (sub-1)->done > sub->done));
      volatile uint64_t* sendHead = &resources->sendMem->head;
      if (groupSync && sub->done < sub->transmitted && (sub->base+sub->done) < *sendHead) {
        sub->done += args->sliceSteps;
        args->idle = 0;
        if (sub->done == sub->nsteps && s == args->nsubs-1) {
          args->state = ncclProxyOpNone;
          TRACE(NCCL_NET, "recvProxy [%d/%d] stopped", sub->done, s);
        }
      }
    }
  }
  return ncclSuccess;
}
```

</details>

