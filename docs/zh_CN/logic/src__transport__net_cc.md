# src/transport/net.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1074"></a>

## F1074：canConnect

用途：由本文件 transport 的拓扑/主机/后端支持条件判断这对参与者是否可使用该传输。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(info1-&gt;hostHash == info2-&gt;hostHash)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 条件判断 | <code>(ncclParamNetDisableIntra() == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 4 / 调用 / 计算 / 写入 | <code>*ret = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>*ret = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Same host?
  if (info1->hostHash == info2->hostHash) {
    // User disabled NET for intra-node?
    if (ncclParamNetDisableIntra() == 1) {
      *ret = 0;
      return ncclSuccess;
    }
  }
  *ret = 1;
  return ncclSuccess;
}
```

</details>

<a id="f1075"></a>

## F1075：sendSetup

用途：为对应方向选择设备/代理并准备本地控制资源，生成可经 bootstrap 交换的建链句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct setupReq req;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ? ncclParamNetSharedBuffers() : 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>req.channelId = channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>req.connIndex = connIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 准备局部状态 | <code>int proxyRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNetDev(comm, myInfo-&gt;rank, graph, channelId, peerInfo-&gt;rank, &req.netDev, &proxyRank));</code> | 失败时可能提前return，成功才继续；按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckGdr(comm-&gt;topo, myInfo-&gt;busId, req.netDev, 1, &req.useGdr));</code> | 失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.direct &#124;= req.useGdr ? NCCL_DIRECT_NIC : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, proxyRank, &send-&gt;proxyConn));</code> | 失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>req.rank = myInfo-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetLocalRank(comm-&gt;topo, myInfo-&gt;rank, &req.localRank));</code> | 失败时可能提前return，成功才继续；把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>req.remoteRank = peerInfo-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&send-&gt;proxyConn, ncclProxyMsgSetup, &req, sizeof(req), NULL, 0));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 14 | 0 / 条件判断 | <code>(proxyRank == myInfo-&gt;rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET,"Channel %02d/%d : %d[%lx] -&gt; %d[%lx] [send] via NET/%s/%d%s%s", channelId, connIndex, myInfo-&gt;rank, myInfo-&gt;busId, peerInfo-&gt;rank, peerInfo-&gt;busId, ncclNetName(), req.netDev, req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");</code> | 读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 17 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET,"Channel %02d/%d : %d[%lx] -&gt; %d[%lx] [send] via NET/%s/%d(%d)%s%s", channelId, connIndex, myInfo-&gt;rank, myInfo-&gt;busId, peerInfo-&gt;rank, peerInfo-&gt;busId, ncclNetName(), req.netDev, proxyRank, req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");</code> | 读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>*((int*)connectInfo) = proxyRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct setupReq req;

  send->conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ? ncclParamNetSharedBuffers() : 1;
  req.channelId = channelId;
  req.connIndex = connIndex;

  int proxyRank;
  NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, peerInfo->rank, &req.netDev, &proxyRank));
  NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 1, &req.useGdr));
  send->conn.direct |= req.useGdr ? NCCL_DIRECT_NIC : 0;

  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, proxyRank, &send->proxyConn));
  req.rank = myInfo->rank;
  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &req.localRank));
  req.remoteRank = peerInfo->rank;
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), NULL, 0));

  if (proxyRank == myInfo->rank) {
    INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [send] via NET/%s/%d%s%s", channelId, connIndex, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, ncclNetName(), req.netDev,
        req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");
  } else {
    INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [send] via NET/%s/%d(%d)%s%s", channelId, connIndex, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, ncclNetName(), req.netDev,
        proxyRank, req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");
  }
  *((int*)connectInfo) = proxyRank;
  return ncclSuccess;
}
```

</details>

<a id="f1076"></a>

## F1076：recvSetup

用途：为对应方向选择设备/代理并准备本地控制资源，生成可经 bootstrap 交换的建链句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct setupReq req;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ? ncclParamNetSharedBuffers() : 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>req.channelId = channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>req.connIndex = connIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 准备局部状态 | <code>int proxyRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetNetDev(comm, myInfo-&gt;rank, graph, channelId, myInfo-&gt;rank, &req.netDev, &proxyRank));</code> | 失败时可能提前return，成功才继续；按图、channel 和 peer 选择实际 NIC 与 Proxy rank，处理本地出口和 PXN 代理条件。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoCheckGdr(comm-&gt;topo, myInfo-&gt;busId, req.netDev, 0, &req.useGdr));</code> | 失败时可能提前return，成功才继续；结合 NIC/GPU 能力、距离与发送读方向策略判断该路径能否使用 GPU Direct RDMA。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 0, myInfo-&gt;rank, &recv-&gt;proxyConn));</code> | 失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>req.rank = myInfo-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetLocalRank(comm-&gt;topo, myInfo-&gt;rank, &req.localRank));</code> | 失败时可能提前return，成功才继续；把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>req.remoteRank = peerInfo-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&recv-&gt;proxyConn, ncclProxyMsgSetup, &req, sizeof(req), connectInfo, sizeof(ncclNetHandle_t)));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET,"Channel %02d/%d : %d[%lx] -&gt; %d[%lx] [receive] via NET/%s/%d%s%s", channelId, connIndex, peerInfo-&gt;rank, peerInfo-&gt;busId, myInfo-&gt;rank, myInfo-&gt;busId, ncclNetName(), req.netDev, req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");</code> | 读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct setupReq req;

  recv->conn.shared = req.shared = graph ? 0 : ncclParamNetSharedBuffers() != -2 ? ncclParamNetSharedBuffers() : 1;
  req.channelId = channelId;
  req.connIndex = connIndex;

  // Use myInfo->rank as the receiver uses its own NIC
  int proxyRank;
  NCCLCHECK(ncclTopoGetNetDev(comm, myInfo->rank, graph, channelId, myInfo->rank, &req.netDev, &proxyRank));
  NCCLCHECK(ncclTopoCheckGdr(comm->topo, myInfo->busId, req.netDev, 0, &req.useGdr));

  // We don't support PXN on receive yet
  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 0, myInfo->rank, &recv->proxyConn));

  req.rank = myInfo->rank;
  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, myInfo->rank, &req.localRank));
  req.remoteRank = peerInfo->rank;
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgSetup, &req, sizeof(req), connectInfo, sizeof(ncclNetHandle_t)));

  INFO(NCCL_INIT|NCCL_NET,"Channel %02d/%d : %d[%lx] -> %d[%lx] [receive] via NET/%s/%d%s%s", channelId, connIndex, peerInfo->rank, peerInfo->busId, myInfo->rank, myInfo->busId, ncclNetName(), req.netDev,
      req.useGdr ? "/GDRDMA" : "", req.shared ? "/Shared" : "");
  return ncclSuccess;
}
```

</details>

<a id="f1077"></a>

## F1077：netMapShm

用途：把已有共享主机内存描述映射到当前进程，建立 CPU 与可选设备地址视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmOpen(mem-&gt;shmPath, mem-&gt;size, (void**)&mem-&gt;cpuPtr, (void**)&mem-&gt;gpuPtr, 0));</code> | 失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmUnlink(mem-&gt;shmPath));</code> | 失败时可能提前return，成功才继续；删除共享内存文件名，已打开的映射仍由各进程按生命周期单独解除。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclShmOpen(mem->shmPath, mem->size, (void**)&mem->cpuPtr, (void**)&mem->gpuPtr, 0));
  NCCLCHECK(ncclShmUnlink(mem->shmPath));
  return ncclSuccess;
}
```

</details>

<a id="f1078"></a>

## F1078：netCreateShm

用途：创建带共享名字的主机内存映射，供不同进程的 NET/Proxy 协作访问。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>mem-&gt;shmPath[0] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmOpen(mem-&gt;shmPath, mem-&gt;size, (void**)&mem-&gt;cpuPtr, NULL, 1));</code> | 失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  mem->shmPath[0] = '\0'; // Let ncclShmOpen create a tmp file
  NCCLCHECK(ncclShmOpen(mem->shmPath, mem->size, (void**)&mem->cpuPtr, NULL, 1));
  return ncclSuccess;
}
```

</details>

<a id="f1079"></a>

## F1079：netDumpMap

用途：打印后端各内存 bank 的 CPU/GPU 地址、大小和映射偏移，辅助检查跨进程地址视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>printf("Dump map same process %d shared %d\n", map-&gt;sameProcess, map-&gt;shared);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 准备局部状态 | <code>struct connectMapMem *mem = map-&gt;mems+NCCL_NET_MAP_HOSTMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>printf("Mem 0: Host mem %s (%x B) CPU %p GPU %p\n", mem-&gt;shmPath, mem-&gt;size, mem-&gt;cpuPtr, mem-&gt;gpuPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>mem = map-&gt;mems+NCCL_NET_MAP_DEVMEM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>printf("Mem 1: Vid mem (%x B) CPU %p GPU %p\n", mem-&gt;size, mem-&gt;cpuPtr, mem-&gt;gpuPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>mem = map-&gt;mems+NCCL_NET_MAP_SHARED_HOSTMEM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>printf("Mem 2: Shared Host mem %s (%x B) CPU %p GPU %p\n", mem-&gt;shmPath, mem-&gt;size, mem-&gt;cpuPtr, mem-&gt;gpuPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>mem = map-&gt;mems+NCCL_NET_MAP_SHARED_DEVMEM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>printf("Mem 3: Shared Vid mem (%x B) CPU %p GPU %p\n", mem-&gt;size, mem-&gt;cpuPtr, mem-&gt;gpuPtr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
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
  printf("Dump map same process %d shared %d\n", map->sameProcess, map->shared);
  struct connectMapMem *mem = map->mems+NCCL_NET_MAP_HOSTMEM;
  printf("Mem 0: Host mem %s (%x B) CPU %p GPU %p\n", mem->shmPath, mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_DEVMEM;
  printf("Mem 1: Vid  mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_SHARED_HOSTMEM;
  printf("Mem 2: Shared Host mem %s (%x B) CPU %p GPU %p\n", mem->shmPath, mem->size, mem->cpuPtr, mem->gpuPtr);
  mem = map->mems+NCCL_NET_MAP_SHARED_DEVMEM;
  printf("Mem 3: Shared Vid mem (%x B) CPU %p GPU %p\n", mem->size, mem->cpuPtr, mem->gpuPtr);
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

<a id="f1080"></a>

## F1080：sendConnect

用途：消费远端句柄完成对应方向的连接和地址映射，把协议缓冲/进度指针填写到设备连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct connectMap* map;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&map, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;transportResources = map;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&send-&gt;proxyConn, ncclProxyMsgConnect, connectInfo, sizeof(ncclNetHandle_t), map, sizeof(struct connectMap)));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 5 | 0 / 条件判断 | <code>(map-&gt;sameProcess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 条件判断 | <code>(map-&gt;cudaDev != comm-&gt;cudaDev)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 4 / 准备局部状态 | <code>cudaError_t err = cudaDeviceEnablePeerAccess(map-&gt;cudaDev, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 4 / 条件判断 | <code>(err == cudaErrorPeerAccessAlreadyEnabled)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 6 / 调用 / 计算 / 写入 | <code>cudaGetLastError();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 14 | 6 / 条件判断 | <code>(err != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 8 / 调用 / 计算 / 写入 | <code>WARN("failed to peer with device %d: %d %s", map-&gt;cudaDev, err, cudaGetErrorString(err));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 8 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(netMapShm(map-&gt;mems+NCCL_NET_MAP_HOSTMEM));</code> | 失败时可能提前return，成功才继续；把已有共享主机内存描述映射到当前进程，建立 CPU 与可选设备地址视图。 |
| 20 | 2 / 条件判断 | <code>(map-&gt;mems[NCCL_NET_MAP_DEVMEM].size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcOpenMemHandle((void**)&map-&gt;mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map-&gt;mems[NCCL_NET_MAP_DEVMEM].ipc, cudaIpcMemLazyEnablePeerAccess));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_DEVMEM].cpuPtr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 2 / 条件判断 | <code>(map-&gt;mems[NCCL_NET_MAP_SHARED_DEVMEM].size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 4 / 准备局部状态 | <code>void** sharedDevMemPtr = comm-&gt;proxyState.sharedDevMems+send-&gt;proxyConn.localRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 4 / 条件判断 | <code>(*sharedDevMemPtr == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 6 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcOpenMemHandle(sharedDevMemPtr, map-&gt;mems[NCCL_NET_MAP_SHARED_DEVMEM].ipc, cudaIpcMemLazyEnablePeerAccess));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 30 | 4 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_SHARED_DEVMEM].gpuPtr = (char*)(*sharedDevMemPtr);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 4 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_SHARED_DEVMEM].cpuPtr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 准备局部状态 | <code>struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 0 / 准备局部状态 | <code>void* gdcMem = map-&gt;mems[NCCL_NET_MAP_GDCMEM].gpuPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.head = gdcMem ? (uint64_t*)gdcMem : &sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 35 | 0 / 准备局部状态 | <code>struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.tail = &recvMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.sizesFifo = recvMem-&gt;sizesFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.offsFifo = map-&gt;shared ? recvMem-&gt;offsFifo : NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 40 | 1 / 调用 / 计算 / 写入 | <code>send-&gt;conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 42 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Setup device pointers
  struct connectMap* map;
  NCCLCHECK(ncclCalloc(&map, 1));
  send->transportResources = map;
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgConnect, connectInfo, sizeof(ncclNetHandle_t), map, sizeof(struct connectMap)));

  if (map->sameProcess) {
    if (map->cudaDev != comm->cudaDev) {
      // Enable P2P access
      cudaError_t err = cudaDeviceEnablePeerAccess(map->cudaDev, 0);
      if (err == cudaErrorPeerAccessAlreadyEnabled) {
        cudaGetLastError();
      } else if (err != cudaSuccess) {
        WARN("failed to peer with device %d: %d %s", map->cudaDev, err, cudaGetErrorString(err));
        return ncclInternalError;
      }
    }
  } else {
    NCCLCHECK(netMapShm(map->mems+NCCL_NET_MAP_HOSTMEM));
    if (map->mems[NCCL_NET_MAP_DEVMEM].size) {
      CUDACHECK(cudaIpcOpenMemHandle((void**)&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].ipc, cudaIpcMemLazyEnablePeerAccess));
      map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = NULL;
    }
    if (map->mems[NCCL_NET_MAP_SHARED_DEVMEM].size) {
      void** sharedDevMemPtr = comm->proxyState.sharedDevMems+send->proxyConn.localRank;
      if (*sharedDevMemPtr == NULL) {
        CUDACHECK(cudaIpcOpenMemHandle(sharedDevMemPtr, map->mems[NCCL_NET_MAP_SHARED_DEVMEM].ipc, cudaIpcMemLazyEnablePeerAccess));
      }
      map->mems[NCCL_NET_MAP_SHARED_DEVMEM].gpuPtr = (char*)(*sharedDevMemPtr);
      map->mems[NCCL_NET_MAP_SHARED_DEVMEM].cpuPtr = NULL;
    }
  }
  //NCCLCHECK(netDumpMap(map));

  struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);
  void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;
  send->conn.head = gdcMem ? (uint64_t*)gdcMem : &sendMem->head;

  struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);
  send->conn.tail = &recvMem->tail;
  send->conn.sizesFifo = recvMem->sizesFifo;
  // Only fuse P2P buffers, continue to allocate dedicated buffers for ring/tree
  send->conn.offsFifo = map->shared ? recvMem->offsFifo : NULL;

  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    send->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);
  return ncclSuccess;
}
```

</details>

<a id="f1081"></a>

## F1081：recvConnect

用途：消费远端句柄完成对应方向的连接和地址映射，把协议缓冲/进度指针填写到设备连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct connectMap* map;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&map, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;transportResources = map;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyCall(&recv-&gt;proxyConn, ncclProxyMsgConnect, connectInfo, sizeof(int), map, sizeof(struct connectMap)));</code> | 失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.head = &sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 7 | 0 / 准备局部状态 | <code>struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>void* gdcMem = map-&gt;mems[NCCL_NET_MAP_GDCMEM].gpuPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.tail = gdcMem ? (uint64_t*)gdcMem : &recvMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.sizesFifo = recvMem-&gt;sizesFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.offsFifo = map-&gt;shared ? recvMem-&gt;offsFifo : NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct connectMap* map;
  NCCLCHECK(ncclCalloc(&map, 1));
  recv->transportResources = map;
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgConnect, connectInfo, sizeof(int), map, sizeof(struct connectMap)));
  //NCCLCHECK(netDumpMap(map));

  struct ncclSendMem *sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, sendMem);
  recv->conn.head = &sendMem->head;

  struct ncclRecvMem *recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, gpu, recvMem);
  void* gdcMem = map->mems[NCCL_NET_MAP_GDCMEM].gpuPtr;
  recv->conn.tail = gdcMem ? (uint64_t*)gdcMem : &recvMem->tail;
  recv->conn.sizesFifo = recvMem->sizesFifo;
  // Only fuse P2P buffers, continue to allocate dedicated buffers for ring/tree
  recv->conn.offsFifo = map->shared ? recvMem->offsFifo : NULL;

  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++)
    recv->conn.buffs[p] = NCCL_NET_MAP_GET_POINTER(map, gpu, buffs[p]);
  return ncclSuccess;
}
```

</details>

<a id="f1082"></a>

## F1082：sendFree

用途：释放对应逻辑连接持有的本地资源/映射，并按代理与共享计数规则释放后台对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct connectMap* map = (struct connectMap*)(send-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(map-&gt;sameProcess == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmClose(map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map-&gt;mems[NCCL_NET_MAP_HOSTMEM].gpuPtr, map-&gt;mems[NCCL_NET_MAP_HOSTMEM].size));</code> | 失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。 |
| 5 | 2 / 条件判断 | <code>(map-&gt;mems[NCCL_NET_MAP_DEVMEM].size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcCloseMemHandle(map-&gt;mems[NCCL_NET_MAP_DEVMEM].gpuPtr));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct connectMap* map = (struct connectMap*)(send->transportResources);
  if (map->sameProcess == 0) {
    NCCLCHECK(ncclShmClose(map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
    if (map->mems[NCCL_NET_MAP_DEVMEM].size) {
      CUDACHECK(cudaIpcCloseMemHandle(map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr));
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f1083"></a>

## F1083：recvFree

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

<a id="f1084"></a>

## F1084：sharedBuffersInit

用途：按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(cuda == 0 && sameProcess == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("PXN should not use host buffers for data");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclProxyProgressState* progressState = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(progressState-&gt;localPeers == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&progressState-&gt;localPeers, comm-&gt;localRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 0 / 准备局部状态 | <code>struct ncclProxyPeer** localPeers = progressState-&gt;localPeers;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 条件判断 | <code>(localPeers[localRank] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(localPeers+localRank, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 13 | 0 / 准备局部状态 | <code>struct ncclProxyPeer* peer = localPeers[localRank];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>struct ncclProxySharedP2p* state = type == 0 ? &peer-&gt;send : &peer-&gt;recv;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;refcount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 条件判断 | <code>(state-&gt;size == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>state-&gt;size = nChannels*(NCCL_SHARED_STEPS/NCCL_STEPS)*comm-&gt;buffSizes[NCCL_PROTO_SIMPLE]/SENDRECV_SLICEFACTOR;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>*size = state-&gt;size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 条件判断 | <code>(cuda && state-&gt;cudaBuff == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaCalloc(&state-&gt;cudaBuff, state-&gt;size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 25 | 2 / 条件判断 | <code>(sameProcess == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcGetMemHandle(&state-&gt;ipc, state-&gt;cudaBuff));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 28 | 0 / 条件判断 | <code>(!cuda && state-&gt;hostBuff == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostCalloc(&state-&gt;hostBuff, state-&gt;size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 31 | 0 / 条件判断 | <code>(cpuPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>*cpuPtr = cuda ? state-&gt;cudaBuff : state-&gt;hostBuff;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 条件判断 | <code>(sameProcess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 2 / 条件判断 | <code>(gpuPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>*gpuPtr = *cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 40 | 2 / 条件判断 | <code>(gpuPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 4 / 调用 / 计算 / 写入 | <code>*gpuPtr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 2 / 条件判断 | <code>(ipc)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 4 / 调用 / 计算 / 写入 | <code>memcpy(ipc, &state-&gt;ipc, sizeof(cudaIpcMemHandle_t));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (cuda == 0 && sameProcess == 0) {
      WARN("PXN should not use host buffers for data");
      return ncclInternalError;
  }
  struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;
  if (progressState->localPeers == NULL) {
    NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));
  }
  struct ncclProxyPeer** localPeers = progressState->localPeers;
  if (localPeers[localRank] == NULL) {
    NCCLCHECK(ncclCalloc(localPeers+localRank, 1));
  }
  struct ncclProxyPeer* peer = localPeers[localRank];
  struct ncclProxySharedP2p* state = type == 0 ? &peer->send : &peer->recv;
  state->refcount++;
  if (state->size == 0) {
    state->size = nChannels*(NCCL_SHARED_STEPS/NCCL_STEPS)*comm->buffSizes[NCCL_PROTO_SIMPLE]/SENDRECV_SLICEFACTOR;
  }

  if (size) *size = state->size;

  if (cuda && state->cudaBuff == NULL) {
    NCCLCHECK(ncclCudaCalloc(&state->cudaBuff, state->size));
    if (sameProcess == 0) {
      CUDACHECK(cudaIpcGetMemHandle(&state->ipc, state->cudaBuff));
    }
  }
  if (!cuda && state->hostBuff == NULL) {
    NCCLCHECK(ncclCudaHostCalloc(&state->hostBuff, state->size));
  }
  if (cpuPtr) *cpuPtr = cuda ? state->cudaBuff : state->hostBuff;
  if (sameProcess) {
    if (gpuPtr) *gpuPtr = *cpuPtr;
  } else {
    if (gpuPtr) *gpuPtr = NULL;
    if (ipc) memcpy(ipc, &state->ipc, sizeof(cudaIpcMemHandle_t));
  }
  return ncclSuccess;
}
```

</details>

<a id="f1085"></a>

## F1085：sharedBuffersGet

用途：按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int slotSize = comm-&gt;buffSizes[NCCL_PROTO_SIMPLE]/(NCCL_STEPS*SENDRECV_SLICEFACTOR);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int globalSlot = (channel*NCCL_SHARED_STEPS)+slot;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*offset = slotSize * globalSlot;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Use different pools for different channels and also separate send/recv.
  int slotSize = comm->buffSizes[NCCL_PROTO_SIMPLE]/(NCCL_STEPS*SENDRECV_SLICEFACTOR);
  int globalSlot = (channel*NCCL_SHARED_STEPS)+slot;
  *offset = slotSize * globalSlot;
  return ncclSuccess;
}
```

</details>

<a id="f1086"></a>

## F1086：sharedBuffersDestroy

用途：降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(comm-&gt;proxyState.progressState.localPeers == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclInternalError);</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclProxyPeer* peer = comm-&gt;proxyState.progressState.localPeers[localRank];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(peer == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclInternalError;)</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 准备局部状态 | <code>struct ncclProxySharedP2p* state = type == 0 ? &peer-&gt;send : &peer-&gt;recv;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 条件判断 | <code>(state-&gt;size == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclInternalError);</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;refcount--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 条件判断 | <code>(state-&gt;refcount == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 条件判断 | <code>(state-&gt;cudaBuff)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(state-&gt;cudaBuff));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 18 | 2 / 条件判断 | <code>(state-&gt;hostBuff)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostFree(state-&gt;hostBuff));</code> | 失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。 |
| 21 | 0 / 条件判断 | <code>(peer-&gt;send.refcount &#124;&#124; peer-&gt;recv.refcount)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>free(peer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;proxyState.progressState.localPeers[localRank] = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;comm-&gt;localRanks；每轮更新 r++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 1 / 条件判断 | <code>(comm-&gt;proxyState.progressState.localPeers[r])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 30 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>free(comm-&gt;proxyState.progressState.localPeers);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;proxyState.progressState.localPeers = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (comm->proxyState.progressState.localPeers == NULL) NCCLCHECK(ncclInternalError);
  struct ncclProxyPeer* peer = comm->proxyState.progressState.localPeers[localRank];
  if (peer == NULL) NCCLCHECK(ncclInternalError;)
  struct ncclProxySharedP2p* state = type == 0 ? &peer->send : &peer->recv;
  if (state->size == 0) NCCLCHECK(ncclInternalError);
  state->refcount--;
  if (state->refcount == 0) {
    if (state->cudaBuff) CUDACHECK(cudaFree(state->cudaBuff));
    if (state->hostBuff) NCCLCHECK(ncclCudaHostFree(state->hostBuff));
  }
  if (peer->send.refcount || peer->recv.refcount) return ncclSuccess;
  free(peer);
  comm->proxyState.progressState.localPeers[localRank] = NULL;
  for (int r=0; r<comm->localRanks; r++) {
    if (comm->proxyState.progressState.localPeers[r]) return ncclSuccess;
  }
  // All peers are freed, free array
  free(comm->proxyState.progressState.localPeers);
  comm->proxyState.progressState.localPeers = NULL;
  return ncclSuccess;
}
```

</details>

<a id="f1087"></a>

## F1087：proxySharedInit

用途：在代理服务端提前初始化可容纳指定通道数的共享缓冲，避免首次数据推进时临时分配。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int rank = comm-&gt;localRankToRank[connection-&gt;localRank];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int sameProcess = comm-&gt;peerInfo[rank].pidHash == comm-&gt;peerInfo[comm-&gt;rank].pidHash ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersInit(comm, 1, connection-&gt;localRank, 0, sameProcess, nChannels, NULL, NULL, NULL, NULL));</code> | 失败时可能提前return，成功才继续；按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int rank = comm->localRankToRank[connection->localRank];
  int sameProcess = comm->peerInfo[rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;
  NCCLCHECK(sharedBuffersInit(comm, 1, connection->localRank, 0, sameProcess, nChannels, NULL, NULL, NULL, NULL));
  return ncclSuccess;
}
```

</details>

<a id="f1088"></a>

## F1088：sendProxySetup

用途：在实际代理进程中准备对应方向网络设备与连接资源；接收方向通常还要 listen 生成句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct setupReq* req = (struct setupReq*) reqBuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(reqSize != sizeof(struct setupReq))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct sendResources* resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;rank = req-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;localRank = req-&gt;localRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;remoteRank = req-&gt;remoteRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;netDev = req-&gt;netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;shared = connection-&gt;shared = req-&gt;shared;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;useGdr = req-&gt;useGdr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;channelId = req-&gt;channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;connIndex = req-&gt;connIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 准备局部状态 | <code>ncclNetProperties_t props;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetGetProperties(req-&gt;netDev, &props));</code> | 失败时可能提前return，成功才继续；读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;maxRecvs = props.maxRecvs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(respSize != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>*done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 23 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct setupReq* req = (struct setupReq*) reqBuff;
  if (reqSize != sizeof(struct setupReq)) return ncclInternalError;

  struct sendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  connection->transportResources = resources;

  resources->rank = req->rank;
  resources->localRank = req->localRank;
  resources->remoteRank = req->remoteRank;
  resources->netDev = req->netDev;
  resources->shared = connection->shared = req->shared;
  resources->useGdr = req->useGdr;
  resources->channelId = req->channelId;
  resources->connIndex = req->connIndex;
  ncclNetProperties_t props;
  NCCLCHECK(ncclNetGetProperties(req->netDev, &props));
  resources->maxRecvs = props.maxRecvs;

  // We don't return any data
  if (respSize != 0) return ncclInternalError;
  *done = 1;
  return ncclSuccess;
}
```

</details>

<a id="f1089"></a>

## F1089：recvProxySetup

用途：在实际代理进程中准备对应方向网络设备与连接资源；接收方向通常还要 listen 生成句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct setupReq* req = (struct setupReq*) reqBuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(reqSize != sizeof(struct setupReq))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct recvResources* resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>connection-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;rank = req-&gt;rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;localRank = req-&gt;localRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;remoteRank = req-&gt;remoteRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;netDev = req-&gt;netDev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;shared = connection-&gt;shared = req-&gt;shared;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;useGdr = req-&gt;useGdr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;channelId = req-&gt;channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;connIndex = req-&gt;connIndex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 准备局部状态 | <code>ncclNetProperties_t props;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetGetProperties(req-&gt;netDev, &props));</code> | 失败时可能提前return，成功才继续；读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;maxRecvs = props.maxRecvs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(respSize != sizeof(ncclNetHandle_t))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetListen(req-&gt;netDev, respBuff, &resources-&gt;netListenComm));</code> | 失败时可能提前return，成功才继续；创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>*done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 24 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct setupReq* req = (struct setupReq*) reqBuff;
  if (reqSize != sizeof(struct setupReq)) return ncclInternalError;

  struct recvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  connection->transportResources = resources;

  resources->rank = req->rank;
  resources->localRank = req->localRank;
  resources->remoteRank = req->remoteRank;
  resources->netDev = req->netDev;
  resources->shared = connection->shared = req->shared;
  resources->useGdr = req->useGdr;
  resources->channelId = req->channelId;
  resources->connIndex = req->connIndex;
  ncclNetProperties_t props;
  NCCLCHECK(ncclNetGetProperties(req->netDev, &props));
  resources->maxRecvs = props.maxRecvs;

  if (respSize != sizeof(ncclNetHandle_t)) return ncclInternalError;
  NCCLCHECK(ncclNetListen(req->netDev, respBuff, &resources->netListenComm));
  *done = 1;
  return ncclSuccess;
}
```

</details>

<a id="f1090"></a>

## F1090：sendProxyConnect

用途：在代理侧异步完成后端 connect/accept、协议缓冲分配/共享/注册，并返回主线程需要的映射信息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct sendResources* resources = (struct sendResources*)(connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(reqSize != sizeof(ncclNetHandle_t))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(resources-&gt;shared)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 准备局部状态 | <code>struct ncclProxyProgressState* progressState = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 条件判断 | <code>(progressState-&gt;localPeers == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&progressState-&gt;localPeers, comm-&gt;localRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 11 | 2 / 准备局部状态 | <code>struct ncclProxyPeer** localPeers = progressState-&gt;localPeers;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 2 / 条件判断 | <code>(localPeers[resources-&gt;localRank] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(localPeers+resources-&gt;localRank, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>connection-&gt;proxyAppendPtr = localPeers[resources-&gt;localRank]-&gt;send.proxyAppend+resources-&gt;channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 2 / 条件判断 | <code>(resources-&gt;maxRecvs &gt; 1 && ncclParamNetSharedComms())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 4 / 条件判断 | <code>(progressState-&gt;netComms[resources-&gt;netDev] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(progressState-&gt;netComms+resources-&gt;netDev, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 21 | 4 / 准备局部状态 | <code>struct ncclSharedNetComms* comms = progressState-&gt;netComms[resources-&gt;netDev]+resources-&gt;remoteRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 4 / 条件判断 | <code>(comms-&gt;sendComm[resources-&gt;channelId] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetConnect(resources-&gt;netDev, reqBuff, comms-&gt;sendComm+resources-&gt;channelId));</code> | 失败时可能提前return，成功才继续；推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 25 | 4 / 调用 / 计算 / 写入 | <code>resources-&gt;netSendComm = comms-&gt;sendComm[resources-&gt;channelId];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 4 / 条件判断 | <code>(comms-&gt;sendComm[resources-&gt;channelId])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 6 / 调用 / 计算 / 写入 | <code>comms-&gt;sendRefCount[resources-&gt;channelId]++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 30 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetConnect(resources-&gt;netDev, reqBuff, &resources-&gt;netSendComm));</code> | 失败时可能提前return，成功才继续；推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 31 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetConnect(resources-&gt;netDev, reqBuff, &resources-&gt;netSendComm));</code> | 失败时可能提前return，成功才继续；推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>connection-&gt;proxyAppendPtr = &connection-&gt;proxyAppend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 条件判断 | <code>(resources-&gt;netSendComm == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>*done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 37 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>*done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 39 | 0 / 准备局部状态 | <code>struct connectMap* map = &resources-&gt;map;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>map-&gt;sameProcess = comm-&gt;peerInfo[resources-&gt;rank].pidHash == comm-&gt;peerInfo[comm-&gt;rank].pidHash ? 1 : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>map-&gt;shared = resources-&gt;shared;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&map-&gt;cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 43 | 0 / 条件判断 | <code>(resources-&gt;shared == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 2 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 46 | 3 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, p!= NCCL_PROTO_LL && resources-&gt;useGdr, comm-&gt;buffSizes[p], buffs[p]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 3 / 调用 / 计算 / 写入 | <code>resources-&gt;buffSizes[p] = comm-&gt;buffSizes[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 49 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 50 | 2 / 准备局部状态 | <code>int bank = resources-&gt;useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 51 | 2 / 准备局部状态 | <code>struct connectMapMem* mapMem = map-&gt;mems+bank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersInit( comm, resources-&gt;useGdr, resources-&gt;localRank, 0, map-&gt;sameProcess, comm-&gt;p2pnChannels, &mapMem-&gt;gpuPtr, &mapMem-&gt;cpuPtr, &mapMem-&gt;size, &mapMem-&gt;ipc));</code> | 失败时可能提前return，成功才继续；按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。 |
| 53 | 2 / 调用 / 计算 / 写入 | <code>resources-&gt;buffSizes[NCCL_PROTO_SIMPLE] = mapMem-&gt;size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 2 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 1, resources-&gt;useGdr, mapMem-&gt;size, buffs[NCCL_PROTO_SIMPLE]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 55 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 56 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 57 | 0 / 条件判断 | <code>(map-&gt;mems[NCCL_NET_MAP_DEVMEM].size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 58 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 59 | 2 / 条件判断 | <code>(resources-&gt;shared == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 60 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 61 | 4 / 条件判断 | <code>(!map-&gt;sameProcess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 62 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 63 | 6 / 调用 / 计算 / 写入 | <code>ALIGN_SIZE(map-&gt;mems[NCCL_NET_MAP_DEVMEM].size, CUDA_IPC_MIN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 64 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaCalloc(&map-&gt;mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map-&gt;mems[NCCL_NET_MAP_DEVMEM].size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 65 | 4 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_DEVMEM].cpuPtr = map-&gt;mems[NCCL_NET_MAP_DEVMEM].gpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 66 | 2 / 条件判断 | <code>(!map-&gt;sameProcess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 67 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 68 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaIpcGetMemHandle(&map-&gt;mems[NCCL_NET_MAP_DEVMEM].ipc, map-&gt;mems[NCCL_NET_MAP_DEVMEM].gpuPtr));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 69 | 0 / 条件判断 | <code>(map-&gt;sameProcess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 70 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 71 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostCalloc(&map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map-&gt;mems[NCCL_NET_MAP_HOSTMEM].size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 72 | 2 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 74 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(netCreateShm(map-&gt;mems+NCCL_NET_MAP_HOSTMEM));</code> | 失败时可能提前return，成功才继续；创建带共享名字的主机内存映射，供不同进程的 NET/Proxy 协作访问。 |
| 75 | 0 / 条件判断 | <code>(ncclGdrCopy && map-&gt;sameProcess && ncclParamGdrCopySyncEnable())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 76 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 77 | 2 / 准备局部状态 | <code>uint64_t *cpuPtr, *gpuPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 78 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 1, &resources-&gt;gdrDesc));</code> | 失败时可能提前return，成功才继续；分配显存并建立 GDRCopy 的 CPU 映射，返回 CUDA 地址、CPU 可写映射与释放所需描述符。 |
| 79 | 2 / 调用 / 计算 / 写入 | <code>resources-&gt;gdcSync = cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 80 | 2 / 准备局部状态 | <code>struct connectMapMem* gdcMem = map-&gt;mems+NCCL_NET_MAP_GDCMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 81 | 2 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;cpuPtr = (char*)cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 2 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;gpuPtr = (char*)gpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 2 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;size = sizeof(uint64_t);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 86 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;sendMem-&gt;head = map-&gt;shared ? -NCCL_STEPS : 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 87 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_STEPS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 88 | 1 / 调用 / 计算 / 写入 | <code>resources-&gt;recvMem-&gt;sizesFifo[i] = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 89 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 90 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 91 | 1 / 调用 / 计算 / 写入 | <code>resources-&gt;buffers[p] = NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 92 | 1 / 条件判断 | <code>(resources-&gt;buffers[p])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 93 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 94 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetRegMr(resources-&gt;netSendComm, resources-&gt;buffers[p], resources-&gt;buffSizes[p], NCCL_NET_MAP_DEV_MEM(map, buffs[p]) ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources-&gt;mhandles[p]));</code> | 失败时可能提前return，成功才继续；注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 95 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 96 | 0 / 条件判断 | <code>(respSize != sizeof(struct connectMap))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 97 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 98 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 99 | 0 / 调用 / 计算 / 写入 | <code>memcpy(respBuff, map, sizeof(struct connectMap));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 100 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct sendResources* resources = (struct sendResources*)(connection->transportResources);
  if (reqSize != sizeof(ncclNetHandle_t)) return ncclInternalError;

  if (resources->shared) {
    // Shared buffers
    struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;
    if (progressState->localPeers == NULL) {
      NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));
    }
    struct ncclProxyPeer** localPeers = progressState->localPeers;
    if (localPeers[resources->localRank] == NULL) {
      NCCLCHECK(ncclCalloc(localPeers+resources->localRank, 1));
    }
    connection->proxyAppendPtr = localPeers[resources->localRank]->send.proxyAppend+resources->channelId;

    if (resources->maxRecvs > 1 && ncclParamNetSharedComms()) {
      // Connect or reuse connection for a netdev/remote rank.
      if (progressState->netComms[resources->netDev] == NULL) {
        NCCLCHECK(ncclCalloc(progressState->netComms+resources->netDev, comm->nRanks));
      }
      struct ncclSharedNetComms* comms = progressState->netComms[resources->netDev]+resources->remoteRank;
      if (comms->sendComm[resources->channelId] == NULL) NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, comms->sendComm+resources->channelId));
      resources->netSendComm = comms->sendComm[resources->channelId];
      if (comms->sendComm[resources->channelId]) comms->sendRefCount[resources->channelId]++;
    } else {
      NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, &resources->netSendComm));
    }
  } else {
    // Connect to remote peer
    NCCLCHECK(ncclNetConnect(resources->netDev, reqBuff, &resources->netSendComm));
    connection->proxyAppendPtr = &connection->proxyAppend;
  }
  if (resources->netSendComm == NULL) {
    *done = 0;
    return ncclSuccess;
  }
  *done = 1;

  // Create structures
  struct connectMap* map = &resources->map;
  map->sameProcess =
    comm->peerInfo[resources->rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;
  map->shared = resources->shared;
  CUDACHECK(cudaGetDevice(&map->cudaDev));

  if (resources->shared == 0) { // Only allocate dedicated buffers for ring/tree, not for p2p
    for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
      NCCL_NET_MAP_ADD_POINTER(map, 0, p!= NCCL_PROTO_LL && resources->useGdr, comm->buffSizes[p], buffs[p]);
      resources->buffSizes[p] = comm->buffSizes[p];
    }
  } else {
    // Get shared buffers
    int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;
    struct connectMapMem* mapMem = map->mems+bank;
    NCCLCHECK(sharedBuffersInit(
          comm, resources->useGdr, resources->localRank, 0, map->sameProcess, comm->p2pnChannels,
          &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size, &mapMem->ipc));
    resources->buffSizes[NCCL_PROTO_SIMPLE] = mapMem->size;
    NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);
  }

  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);
  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);

  if (map->mems[NCCL_NET_MAP_DEVMEM].size) {
    if (resources->shared == 0) {
      if (!map->sameProcess) {
        ALIGN_SIZE(map->mems[NCCL_NET_MAP_DEVMEM].size, CUDA_IPC_MIN);
      }
      NCCLCHECK(ncclCudaCalloc(&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].size));
      map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr;
    }
    if (!map->sameProcess) {
      CUDACHECK(cudaIpcGetMemHandle(&map->mems[NCCL_NET_MAP_DEVMEM].ipc, map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr));
    }
  }
  if (map->sameProcess) {
    NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
    map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;
  } else {
    NCCLCHECK(netCreateShm(map->mems+NCCL_NET_MAP_HOSTMEM));
  }
  if (ncclGdrCopy && map->sameProcess && ncclParamGdrCopySyncEnable()) {
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
  resources->sendMem->head = map->shared ? -NCCL_STEPS : 0;
  for (int i=0; i<NCCL_STEPS; i++) resources->recvMem->sizesFifo[i] = -1;

  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    resources->buffers[p] = NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]);
    if (resources->buffers[p]) {
      NCCLCHECK(ncclNetRegMr(resources->netSendComm, resources->buffers[p], resources->buffSizes[p], NCCL_NET_MAP_DEV_MEM(map, buffs[p]) ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources->mhandles[p]));
    }
  }

  //NCCLCHECK(netDumpMap(map));
  if (respSize != sizeof(struct connectMap)) return ncclInternalError;
  memcpy(respBuff, map, sizeof(struct connectMap));
  return ncclSuccess;
}
```

</details>

<a id="f1091"></a>

## F1091：recvProxyConnect

用途：在代理侧异步完成后端 connect/accept、协议缓冲分配/共享/注册，并返回主线程需要的映射信息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(reqSize != sizeof(int))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*)(connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;proxyRank = *(int*)reqBuff;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 条件判断 | <code>(resources-&gt;shared)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 准备局部状态 | <code>struct ncclProxyProgressState* progressState = &comm-&gt;proxyState.progressState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 条件判断 | <code>(progressState-&gt;localPeers == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&progressState-&gt;localPeers, comm-&gt;localRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 12 | 2 / 准备局部状态 | <code>struct ncclProxyPeer** localPeers = progressState-&gt;localPeers;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 2 / 条件判断 | <code>(localPeers[resources-&gt;localRank] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(localPeers+resources-&gt;localRank, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>connection-&gt;proxyAppendPtr = localPeers[resources-&gt;localRank]-&gt;recv.proxyAppend+resources-&gt;channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 条件判断 | <code>(resources-&gt;maxRecvs &gt; 1 && ncclParamNetSharedComms())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 4 / 条件判断 | <code>(progressState-&gt;netComms[resources-&gt;netDev] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(progressState-&gt;netComms+resources-&gt;netDev, comm-&gt;nRanks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 22 | 4 / 准备局部状态 | <code>struct ncclSharedNetComms* comms = progressState-&gt;netComms[resources-&gt;netDev]+resources-&gt;proxyRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 4 / 条件判断 | <code>(comms-&gt;recvComm[resources-&gt;channelId] == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetAccept(resources-&gt;netListenComm, comms-&gt;recvComm+resources-&gt;channelId));</code> | 失败时可能提前return，成功才继续；推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>resources-&gt;netRecvComm = comms-&gt;recvComm[resources-&gt;channelId];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 4 / 条件判断 | <code>(comms-&gt;recvComm[resources-&gt;channelId])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 6 / 调用 / 计算 / 写入 | <code>comms-&gt;recvRefCount[resources-&gt;channelId]++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 31 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetAccept(resources-&gt;netListenComm, &resources-&gt;netRecvComm));</code> | 失败时可能提前return，成功才继续；推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 32 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetAccept(resources-&gt;netListenComm, &resources-&gt;netRecvComm));</code> | 失败时可能提前return，成功才继续；推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>connection-&gt;proxyAppendPtr = &connection-&gt;proxyAppend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 0 / 条件判断 | <code>(resources-&gt;netRecvComm == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>*done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 38 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>*done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseListen(resources-&gt;netListenComm));</code> | 失败时可能提前return，成功才继续；关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 41 | 0 / 准备局部状态 | <code>struct connectMap* map = &resources-&gt;map;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>map-&gt;sameProcess = comm-&gt;peerInfo[resources-&gt;rank].pidHash == comm-&gt;peerInfo[comm-&gt;rank].pidHash ? 1 : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 条件判断 | <code>(map-&gt;sameProcess == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>map-&gt;shared = resources-&gt;shared;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 0 / 条件判断 | <code>(resources-&gt;shared == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 2 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 50 | 3 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, resources-&gt;useGdr, comm-&gt;buffSizes[p], buffs[p]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 51 | 3 / 调用 / 计算 / 写入 | <code>resources-&gt;buffSizes[p] = comm-&gt;buffSizes[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 53 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 54 | 2 / 准备局部状态 | <code>int bank = resources-&gt;useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 55 | 2 / 准备局部状态 | <code>struct connectMapMem* mapMem = map-&gt;mems+bank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersInit( comm, resources-&gt;useGdr, resources-&gt;localRank, 1, 1, comm-&gt;p2pnChannels, &mapMem-&gt;gpuPtr, &mapMem-&gt;cpuPtr, &mapMem-&gt;size, NULL));</code> | 失败时可能提前return，成功才继续；按本地 rank、方向和 CUDA/主机内存类型创建或复用共享协议数据池，引用计数管理生命周期。 |
| 57 | 2 / 调用 / 计算 / 写入 | <code>resources-&gt;buffSizes[NCCL_PROTO_SIMPLE] = mapMem-&gt;size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 2 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 1, resources-&gt;useGdr, mapMem-&gt;size, buffs[NCCL_PROTO_SIMPLE]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 59 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 60 | 0 / 调用 / 计算 / 写入 | <code>NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 61 | 0 / 条件判断 | <code>(map-&gt;mems[NCCL_NET_MAP_DEVMEM].size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 62 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 63 | 2 / 条件判断 | <code>(resources-&gt;shared == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaCalloc(&map-&gt;mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map-&gt;mems[NCCL_NET_MAP_DEVMEM].size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 66 | 4 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_DEVMEM].cpuPtr = map-&gt;mems[NCCL_NET_MAP_DEVMEM].gpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostCalloc(&map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map-&gt;mems[NCCL_NET_MAP_HOSTMEM].size));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 68 | 0 / 调用 / 计算 / 写入 | <code>map-&gt;mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map-&gt;mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 69 | 0 / 条件判断 | <code>(ncclGdrCopy && map-&gt;sameProcess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 70 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 71 | 2 / 准备局部状态 | <code>uint64_t *cpuPtr, *gpuPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 72 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaCalloc(&cpuPtr, &gpuPtr, 2, &resources-&gt;gdrDesc));</code> | 失败时可能提前return，成功才继续；分配显存并建立 GDRCopy 的 CPU 映射，返回 CUDA 地址、CPU 可写映射与释放所需描述符。 |
| 73 | 2 / 条件判断 | <code>(ncclParamGdrCopySyncEnable())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 74 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 75 | 4 / 调用 / 计算 / 写入 | <code>resources-&gt;gdcSync = cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 76 | 4 / 准备局部状态 | <code>struct connectMapMem* gdcMem = map-&gt;mems+NCCL_NET_MAP_GDCMEM;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 77 | 4 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;cpuPtr = (char*)cpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 4 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;gpuPtr = (char*)gpuPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 4 / 调用 / 计算 / 写入 | <code>gdcMem-&gt;size = sizeof(uint64_t);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 80 | 2 / 条件判断 | <code>(ncclParamGdrCopyFlushEnable())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 81 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 82 | 4 / 调用 / 计算 / 写入 | <code>resources-&gt;gdcFlush = cpuPtr + 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;sendMem = (struct ncclSendMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, sendMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;recvMem = (struct ncclRecvMem*) NCCL_NET_MAP_GET_POINTER(map, cpu, recvMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 86 | 1 / 调用 / 计算 / 写入 | <code>resources-&gt;buffers[p] = NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 87 | 1 / 条件判断 | <code>(resources-&gt;buffers[p])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 88 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 89 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetRegMr(resources-&gt;netRecvComm, resources-&gt;buffers[p], resources-&gt;buffSizes[p], NCCL_NET_MAP_DEV_MEM(map, buffs[p]) ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources-&gt;mhandles[p]));</code> | 失败时可能提前return，成功才继续；注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 90 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 91 | 0 / 条件判断 | <code>(respSize != sizeof(struct connectMap))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 92 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 93 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 94 | 0 / 调用 / 计算 / 写入 | <code>memcpy(respBuff, map, sizeof(struct connectMap));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 95 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (reqSize != sizeof(int)) return ncclInternalError;
  struct recvResources* resources = (struct recvResources*)(connection->transportResources);
  resources->proxyRank = *(int*)reqBuff;

  // Finish connection establishment from remote peer
  if (resources->shared) {
    // Shared buffers
    struct ncclProxyProgressState* progressState = &comm->proxyState.progressState;
    if (progressState->localPeers == NULL) {
      NCCLCHECK(ncclCalloc(&progressState->localPeers, comm->localRanks));
    }
    struct ncclProxyPeer** localPeers = progressState->localPeers;
    if (localPeers[resources->localRank] == NULL) {
      NCCLCHECK(ncclCalloc(localPeers+resources->localRank, 1));
    }
    connection->proxyAppendPtr = localPeers[resources->localRank]->recv.proxyAppend+resources->channelId;

    if (resources->maxRecvs > 1 && ncclParamNetSharedComms()) {
      // Connect or reuse connection for a netdev/remote rank.
      if (progressState->netComms[resources->netDev] == NULL) {
        NCCLCHECK(ncclCalloc(progressState->netComms+resources->netDev, comm->nRanks));
      }
      struct ncclSharedNetComms* comms = progressState->netComms[resources->netDev]+resources->proxyRank;
      if (comms->recvComm[resources->channelId] == NULL) NCCLCHECK(ncclNetAccept(resources->netListenComm, comms->recvComm+resources->channelId));
      resources->netRecvComm = comms->recvComm[resources->channelId];
      if (comms->recvComm[resources->channelId]) comms->recvRefCount[resources->channelId]++;
    } else {
      NCCLCHECK(ncclNetAccept(resources->netListenComm, &resources->netRecvComm));
    }
  } else {
    // Connect to remote peer
    NCCLCHECK(ncclNetAccept(resources->netListenComm, &resources->netRecvComm));
    connection->proxyAppendPtr = &connection->proxyAppend;
  }
  if (resources->netRecvComm == NULL) {
    *done = 0;
    return ncclSuccess;
  }
  *done = 1;
  NCCLCHECK(ncclNetCloseListen(resources->netListenComm));

  // Create structures
  struct connectMap* map = &resources->map;
  map->sameProcess =
    comm->peerInfo[resources->rank].pidHash == comm->peerInfo[comm->rank].pidHash ? 1 : 0;
  if (map->sameProcess == 0) return ncclInternalError; // We don't support remote proxy for recv
  map->shared = resources->shared;

  if (resources->shared == 0) { // Only allocate dedicated buffers for ring/tree, not for p2p
    for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
      NCCL_NET_MAP_ADD_POINTER(map, 0, resources->useGdr, comm->buffSizes[p], buffs[p]);
      resources->buffSizes[p] = comm->buffSizes[p];
    }
  } else {
    // Get shared buffers
    int bank = resources->useGdr ? NCCL_NET_MAP_SHARED_DEVMEM : NCCL_NET_MAP_SHARED_HOSTMEM;
    struct connectMapMem* mapMem = map->mems+bank;
    NCCLCHECK(sharedBuffersInit(
          comm, resources->useGdr, resources->localRank, 1, 1, comm->p2pnChannels,
          &mapMem->gpuPtr, &mapMem->cpuPtr, &mapMem->size, NULL));
    resources->buffSizes[NCCL_PROTO_SIMPLE] = mapMem->size;
    NCCL_NET_MAP_ADD_POINTER(map, 1, resources->useGdr, mapMem->size, buffs[NCCL_PROTO_SIMPLE]);
  }

  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclSendMem), sendMem);
  NCCL_NET_MAP_ADD_POINTER(map, 0, 0, sizeof(struct ncclRecvMem), recvMem);

  if (map->mems[NCCL_NET_MAP_DEVMEM].size) {
    if (resources->shared == 0) {
      NCCLCHECK(ncclCudaCalloc(&map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr, map->mems[NCCL_NET_MAP_DEVMEM].size));
      map->mems[NCCL_NET_MAP_DEVMEM].cpuPtr = map->mems[NCCL_NET_MAP_DEVMEM].gpuPtr;
    }
  }
  NCCLCHECK(ncclCudaHostCalloc(&map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, map->mems[NCCL_NET_MAP_HOSTMEM].size));
  map->mems[NCCL_NET_MAP_HOSTMEM].gpuPtr = map->mems[NCCL_NET_MAP_HOSTMEM].cpuPtr;
  if (ncclGdrCopy && map->sameProcess) {
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
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    resources->buffers[p] = NCCL_NET_MAP_GET_POINTER(map, cpu, buffs[p]);
    if (resources->buffers[p]) {
      NCCLCHECK(ncclNetRegMr(resources->netRecvComm, resources->buffers[p], resources->buffSizes[p], NCCL_NET_MAP_DEV_MEM(map, buffs[p]) ? NCCL_PTR_CUDA : NCCL_PTR_HOST, &resources->mhandles[p]));
    }
  }

  //NCCLCHECK(netDumpMap(map));
  if (respSize != sizeof(struct connectMap)) return ncclInternalError;
  memcpy(respBuff, map, sizeof(struct connectMap));
  return ncclSuccess;
}
```

</details>

<a id="f1092"></a>

## F1092：sendProxyFree

用途：在拥有网络对象的代理侧注销内存、释放映射并按引用计数关闭 backend connection。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct sendResources* resources = (struct sendResources*)(connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(resources == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersDestroy(comm, connection-&gt;localRank, 0));</code> | 失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 条件判断 | <code>(resources-&gt;buffers[p])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetDeregMr(resources-&gt;netSendComm, resources-&gt;mhandles[p]));</code> | 失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 准备局部状态 | <code>struct connectMapMem* mems = resources-&gt;map.mems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 条件判断 | <code>(resources-&gt;map.sameProcess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));</code> | 失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。 |
| 15 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmClose(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, NULL, mems[NCCL_NET_MAP_HOSTMEM].size));</code> | 失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 18 | 0 / 条件判断 | <code>(mems[NCCL_NET_MAP_GDCMEM].cpuPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaFree(resources-&gt;gdrDesc));</code> | 失败时可能提前return，成功才继续；按映射、pin、显存分配的依赖顺序撤销 GDRCopy 资源。 |
| 21 | 0 / 条件判断 | <code>(resources-&gt;shared)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersDestroy(comm, resources-&gt;localRank, 0));</code> | 失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。 |
| 24 | 2 / 条件判断 | <code>(resources-&gt;maxRecvs &gt; 1 && ncclParamNetSharedComms())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 4 / 准备局部状态 | <code>struct ncclSharedNetComms* comms = comm-&gt;proxyState.progressState.netComms[resources-&gt;netDev]+resources-&gt;remoteRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 4 / 调用 / 计算 / 写入 | <code>comms-&gt;sendRefCount[resources-&gt;channelId]--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 4 / 条件判断 | <code>(comms-&gt;sendRefCount[resources-&gt;channelId] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseSend(comms-&gt;sendComm[resources-&gt;channelId]));</code> | 失败时可能提前return，成功才继续；关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 31 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseSend(resources-&gt;netSendComm));</code> | 失败时可能提前return，成功才继续；关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 33 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseSend(resources-&gt;netSendComm));</code> | 失败时可能提前return，成功才继续；关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>free(resources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct sendResources* resources = (struct sendResources*)(connection->transportResources);
  if (resources == NULL) { // NVB Preconnect
    NCCLCHECK(sharedBuffersDestroy(comm, connection->localRank, 0));
    return ncclSuccess;
  }
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (resources->buffers[p]) {
      NCCLCHECK(ncclNetDeregMr(resources->netSendComm, resources->mhandles[p]));
    }
  }
  struct connectMapMem* mems = resources->map.mems;
  if (resources->map.sameProcess) {
    NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));
  } else {
    NCCLCHECK(ncclShmClose(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr, NULL, mems[NCCL_NET_MAP_HOSTMEM].size));
  }
  CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));
  if (mems[NCCL_NET_MAP_GDCMEM].cpuPtr) NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));
  if (resources->shared) {
    NCCLCHECK(sharedBuffersDestroy(comm, resources->localRank, 0));
    if (resources->maxRecvs > 1 && ncclParamNetSharedComms()) {
      struct ncclSharedNetComms* comms = comm->proxyState.progressState.netComms[resources->netDev]+resources->remoteRank;
      comms->sendRefCount[resources->channelId]--;
      if (comms->sendRefCount[resources->channelId] == 0) NCCLCHECK(ncclNetCloseSend(comms->sendComm[resources->channelId]));
    } else {
      NCCLCHECK(ncclNetCloseSend(resources->netSendComm));
    }
  } else {
    NCCLCHECK(ncclNetCloseSend(resources->netSendComm));
  }
  free(resources);
  return ncclSuccess;
}
```

</details>

<a id="f1093"></a>

## F1093：recvProxyFree

用途：在拥有网络对象的代理侧注销内存、释放映射并按引用计数关闭 backend connection。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*)(connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(resources == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersDestroy(comm, connection-&gt;localRank, 1));</code> | 失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 条件判断 | <code>(resources-&gt;buffers[p])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetDeregMr(resources-&gt;netRecvComm, resources-&gt;mhandles[p]));</code> | 失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 准备局部状态 | <code>struct connectMapMem* mems = resources-&gt;map.mems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));</code> | 失败时可能提前return，成功才继续；回收 cudaHostAlloc 对应的主机固定内存，不能用普通 free 替代。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 0 / 条件判断 | <code>(mems[NCCL_NET_MAP_GDCMEM].cpuPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGdrCudaFree(resources-&gt;gdrDesc));</code> | 失败时可能提前return，成功才继续；按映射、pin、显存分配的依赖顺序撤销 GDRCopy 资源。 |
| 17 | 0 / 条件判断 | <code>(resources-&gt;shared)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersDestroy(comm, resources-&gt;localRank, 1));</code> | 失败时可能提前return，成功才继续；降低共享数据池引用数，最后使用者退出时回收显存/主机映射与导出句柄。 |
| 20 | 2 / 条件判断 | <code>(resources-&gt;maxRecvs &gt; 1 && ncclParamNetSharedComms())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 4 / 准备局部状态 | <code>struct ncclSharedNetComms* comms = comm-&gt;proxyState.progressState.netComms[resources-&gt;netDev]+resources-&gt;proxyRank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>comms-&gt;recvRefCount[resources-&gt;channelId]--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 4 / 条件判断 | <code>(comms-&gt;recvRefCount[resources-&gt;channelId] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseRecv(comms-&gt;recvComm[resources-&gt;channelId]));</code> | 失败时可能提前return，成功才继续；关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 27 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 28 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseRecv(resources-&gt;netRecvComm));</code> | 失败时可能提前return，成功才继续；关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 29 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetCloseRecv(resources-&gt;netRecvComm));</code> | 失败时可能提前return，成功才继续；关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>free(resources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct recvResources* resources = (struct recvResources*)(connection->transportResources);
  if (resources == NULL) { // NVB Preconnect
    NCCLCHECK(sharedBuffersDestroy(comm, connection->localRank, 1));
    return ncclSuccess;
  }
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (resources->buffers[p]) {
      NCCLCHECK(ncclNetDeregMr(resources->netRecvComm, resources->mhandles[p]));
    }
  }
  struct connectMapMem* mems = resources->map.mems;
  NCCLCHECK(ncclCudaHostFree(mems[NCCL_NET_MAP_HOSTMEM].cpuPtr));
  CUDACHECK(cudaFree(mems[NCCL_NET_MAP_DEVMEM].cpuPtr));
  if (mems[NCCL_NET_MAP_GDCMEM].cpuPtr) NCCLCHECK(ncclGdrCudaFree(resources->gdrDesc));
  if (resources->shared) {
    NCCLCHECK(sharedBuffersDestroy(comm, resources->localRank, 1));
    if (resources->maxRecvs > 1 && ncclParamNetSharedComms()) {
      struct ncclSharedNetComms* comms = comm->proxyState.progressState.netComms[resources->netDev]+resources->proxyRank;
      comms->recvRefCount[resources->channelId]--;
      if (comms->recvRefCount[resources->channelId] == 0) NCCLCHECK(ncclNetCloseRecv(comms->recvComm[resources->channelId]));
    } else {
      NCCLCHECK(ncclNetCloseRecv(resources->netRecvComm));
    }
  } else {
    NCCLCHECK(ncclNetCloseRecv(resources->netRecvComm));
  }
  free(resources);
  return ncclSuccess;
}
```

</details>

<a id="f1094"></a>

## F1094：sendProxyProgress

用途：对应后端发送状态机：准备槽位、等 GPU 数据就绪、提交网络请求、检测完成后回收；CollNet 分支提交网络归约。

- 本文件NET发送Ready阶段只初始化本任务base和相对计数，后续回调从Progress继续，不应每轮重置。
- 先提供可写槽，再等GPU sizes/tail/flag就绪；Isend给出有效请求后transmitted前进。
- Test完成才能推进done并释放head；同一连接和MR继续保留给后续任务。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(args-&gt;state == ncclProxyOpReady)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 3 / 准备局部状态 | <code>struct sendResources* resources = (struct sendResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>sub-&gt;base = ROUNDUP(resources-&gt;step, args-&gt;chunkSteps);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>sub-&gt;posted = sub-&gt;transmitted = sub-&gt;done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 8 | 3 / 循环 | <code>初始 uint64_t step=0;；继续条件 step&lt;sub-&gt;nsteps；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s, step, ncclProxyProfileBegin);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 10 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpProgress;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 条件判断 | <code>(args-&gt;state == ncclProxyOpProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 准备局部状态 | <code>int p = args-&gt;protocol;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 2 / 准备局部状态 | <code>int maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args-&gt;nsubs);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 18 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 3 / 条件判断 | <code>(sub-&gt;done == sub-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 23 | 3 / 准备局部状态 | <code>struct sendResources* resources = (struct sendResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 3 / 准备局部状态 | <code>void* mhandle = resources-&gt;mhandles[p];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 3 / 准备局部状态 | <code>int stepSize = resources-&gt;buffSizes[p] / NCCL_STEPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 3 / 准备局部状态 | <code>char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources-&gt;map, cpu, buffs[p]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 3 / 准备局部状态 | <code>int buffSize = stepSize*args-&gt;sliceSteps;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 3 / 条件判断 | <code>(sub-&gt;nbytes &lt; buffSize)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 5 / 调用 / 计算 / 写入 | <code>buffSize = sub-&gt;nbytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 3 / 条件判断 | <code>(sub-&gt;posted &lt; sub-&gt;nsteps && sub-&gt;posted &lt; sub-&gt;done + maxDepth)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;posted)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 34 | 5 / 条件判断 | <code>(resources-&gt;shared)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 7 / 准备局部状态 | <code>int sharedBuffSlot = sub-&gt;posted%maxDepth;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 37 | 7 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersGet(comm, sub-&gt;channelId, sharedBuffSlot*args-&gt;nsubs+s, &offset));</code> | 失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。 |
| 39 | 7 / 调用 / 计算 / 写入 | <code>resources-&gt;recvMem-&gt;offsFifo[buffSlot] = offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 7 / 调用 / 计算 / 写入 | <code>__sync_synchronize();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 7 / 准备局部状态 | <code>volatile uint64_t* sendHead = resources-&gt;gdcSync ? resources-&gt;gdcSync : &resources-&gt;sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 42 | 7 / 调用 / 计算 / 写入 | <code>sub-&gt;posted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 43 | 7 / 调用 / 计算 / 写入 | <code>*sendHead = sub-&gt;base + sub-&gt;posted - NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 44 | 7 / 条件判断 | <code>(resources-&gt;gdcSync)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 9 / 调用 / 计算 / 写入 | <code>wc_store_fence();</code> | 按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。 |
| 47 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 48 | 7 / 调用 / 计算 / 写入 | <code>sub-&gt;posted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 49 | 5 / 循环 | <code>初始 uint64_t step=sub-&gt;posted-args-&gt;sliceSteps;；继续条件 step&lt;sub-&gt;posted；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 50 | 6 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s, step, ncclProxyProfileSendGPUWait);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 51 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 52 | 5 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 54 | 3 / 条件判断 | <code>(sub-&gt;transmitted &lt; sub-&gt;posted && sub-&gt;transmitted &lt; sub-&gt;done + NCCL_STEPS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 55 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 56 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;transmitted)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 57 | 5 / 准备局部状态 | <code>volatile int* sizesFifo = resources-&gt;recvMem-&gt;sizesFifo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 58 | 5 / 准备局部状态 | <code>volatile uint64_t* recvTail = &resources-&gt;recvMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 59 | 5 / 条件判断 | <code>(sizesFifo[buffSlot] != -1 && ((*recvTail &gt; (sub-&gt;base+sub-&gt;transmitted)) &#124;&#124; p == NCCL_PROTO_LL))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 60 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 61 | 7 / 准备局部状态 | <code>int size = sizesFifo[buffSlot];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 62 | 7 / 准备局部状态 | <code>char* buff = resources-&gt;shared ? localBuff+resources-&gt;recvMem-&gt;offsFifo[buffSlot] : localBuff+buffSlot*stepSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 63 | 7 / 准备局部状态 | <code>int ready = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 64 | 7 / 条件判断 | <code>(p == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 65 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 66 | 9 / 调用 / 计算 / 写入 | <code>ready = resources-&gt;useGdr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 9 / 条件判断 | <code>(!ready)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 11 / 准备局部状态 | <code>uint64_t flag = sub-&gt;base+sub-&gt;transmitted+1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 70 | 11 / 准备局部状态 | <code>int nFifoLines = DIVUP(sizesFifo[buffSlot], sizeof(uint64_t)*NCCL_LL128_LINEELEMS);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 71 | 11 / 准备局部状态 | <code>volatile uint64_t* lines = (volatile uint64_t*)buff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 72 | 11 / 调用 / 计算 / 写入 | <code>ready = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 11 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nFifoLines；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 74 | 12 / 条件判断 | <code>(lines[i*NCCL_LL128_LINEELEMS+NCCL_LL128_DATAELEMS] != flag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 75 | 13 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 76 | 14 / 调用 / 计算 / 写入 | <code>ready = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 77 | 14 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 78 | 11 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 79 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 80 | 9 / 条件判断 | <code>(p == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 81 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 82 | 11 / 准备局部状态 | <code>uint32_t flag = NCCL_LL_FLAG(sub-&gt;base+sub-&gt;transmitted+1);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 83 | 11 / 准备局部状态 | <code>int nFifoLines = DIVUP(size, sizeof(union ncclLLFifoLine));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 84 | 11 / 准备局部状态 | <code>union ncclLLFifoLine* lines = (union ncclLLFifoLine*)buff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 85 | 11 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nFifoLines；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 86 | 12 / 准备局部状态 | <code>volatile uint32_t *f1 = &lines[i].flag1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 87 | 12 / 准备局部状态 | <code>volatile uint32_t *f2 = &lines[i].flag2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 88 | 12 / 条件判断 | <code>(f1[0] != flag &#124;&#124; f2[0] != flag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 89 | 13 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 90 | 14 / 调用 / 计算 / 写入 | <code>ready = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 91 | 14 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 92 | 11 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 93 | 7 / 条件判断 | <code>(ready)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 94 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 95 | 9 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetIsend(resources-&gt;netSendComm, buff, size, resources-&gt;rank, mhandle, sub-&gt;requests+buffSlot));</code> | 失败时可能提前return，成功才继续；提交异步发送请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 96 | 9 / 条件判断 | <code>(sub-&gt;requests[buffSlot] != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 97 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 98 | 11 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "sendProxy [%ld/%d] Isend posted, req %p", sub-&gt;transmitted, buffSlot, sub-&gt;requests[buffSlot]);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 99 | 11 / 调用 / 计算 / 写入 | <code>sizesFifo[buffSlot] = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 100 | 11 / 调用 / 计算 / 写入 | <code>__sync_synchronize();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 101 | 11 / 调用 / 计算 / 写入 | <code>sub-&gt;transmitted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 102 | 11 / 循环 | <code>初始 uint64_t step=sub-&gt;transmitted-args-&gt;sliceSteps;；继续条件 step&lt;sub-&gt;transmitted；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 103 | 12 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s, step, ncclProxyProfileSendWait);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 104 | 11 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 105 | 11 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 106 | 11 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 107 | 3 / 条件判断 | <code>(sub-&gt;done &lt; sub-&gt;transmitted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 108 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 109 | 5 / 准备局部状态 | <code>int done;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 110 | 5 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;done)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 111 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetTest(sub-&gt;requests[buffSlot], &done, NULL));</code> | 失败时可能提前return，成功才继续；查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 112 | 5 / 条件判断 | <code>(done)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 113 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 114 | 7 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "sendProxy [%ld/%d] request %p done", sub-&gt;done, buffSlot, sub-&gt;requests[buffSlot]);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 115 | 7 / 调用 / 计算 / 写入 | <code>sub-&gt;done += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 116 | 7 / 循环 | <code>初始 uint64_t step=sub-&gt;done-args-&gt;sliceSteps;；继续条件 step&lt;sub-&gt;done；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 117 | 8 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s, step, ncclProxyProfileEnd);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 118 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 119 | 7 / 条件判断 | <code>(resources-&gt;shared == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 120 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 121 | 9 / 准备局部状态 | <code>volatile uint64_t* sendHead = resources-&gt;gdcSync ? resources-&gt;gdcSync : &resources-&gt;sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 122 | 9 / 调用 / 计算 / 写入 | <code>*sendHead = sub-&gt;base + sub-&gt;done;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 123 | 9 / 条件判断 | <code>(resources-&gt;gdcSync)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 124 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 125 | 11 / 调用 / 计算 / 写入 | <code>wc_store_fence();</code> | 按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。 |
| 126 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 127 | 7 / 条件判断 | <code>(sub-&gt;done == sub-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 128 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 129 | 9 / 调用 / 计算 / 写入 | <code>resources-&gt;step = sub-&gt;base + sub-&gt;nsteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 130 | 9 / 调用 / 计算 / 写入 | <code>args-&gt;done++;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 131 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 132 | 2 / 条件判断 | <code>(args-&gt;done == args-&gt;nsubs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 133 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 134 | 4 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpNone;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 135 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (args->state == ncclProxyOpReady) {
    for (int s=0; s<args->nsubs; s++) {
      struct ncclProxySubArgs* sub = args->subs+s;
      struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);
      // Round to next multiple of sliceSteps
      sub->base = ROUNDUP(resources->step, args->chunkSteps);
      sub->posted = sub->transmitted = sub->done = 0;
      for (uint64_t step=0; step<sub->nsteps; step++) ncclProfilingRecord(args, s, step, ncclProxyProfileBegin);
    }
    args->state = ncclProxyOpProgress;
  }
  args->idle = 1;
  if (args->state == ncclProxyOpProgress) {
    int p = args->protocol;
    int maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args->nsubs);
    for (int s=0; s<args->nsubs; s++) {
      struct ncclProxySubArgs* sub = args->subs+s;
      if (sub->done == sub->nsteps) continue;
      struct sendResources* resources = (struct sendResources*) (sub->connection->transportResources);
      void* mhandle = resources->mhandles[p];
      int stepSize = resources->buffSizes[p] / NCCL_STEPS;
      char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);
      int buffSize = stepSize*args->sliceSteps;
      if (sub->nbytes < buffSize) buffSize = sub->nbytes;
/* 中文精读：发送状态机第 1 阶段：给 GPU 可写空间
 * 限制 posted < done+maxDepth，避免在途数据超过可回收槽位。
 * 共享模式先分配 offsFifo 偏移，内存屏障后更新 head，让 GPU 知道当前槽可写。
 * 非共享模式使用固定缓冲槽，posted 仍跟踪允许推进的步数。
 * posted 仅表示槽位准备好，数据可能尚未由 GPU 写入，更没有保证已发往网络。
 */
      // Post buffers to the GPU
      if (sub->posted < sub->nsteps && sub->posted < sub->done + maxDepth) {
        int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
        if (resources->shared) {
          int sharedBuffSlot = sub->posted%maxDepth;
          int offset;
          NCCLCHECK(sharedBuffersGet(comm, sub->channelId, sharedBuffSlot*args->nsubs+s, &offset));
          resources->recvMem->offsFifo[buffSlot] = offset;
          __sync_synchronize();
          volatile uint64_t* sendHead = resources->gdcSync ? resources->gdcSync : &resources->sendMem->head;
          sub->posted += args->sliceSteps;
          *sendHead = sub->base + sub->posted - NCCL_STEPS;
          if (resources->gdcSync) wc_store_fence(); // Flush out WC write
        } else sub->posted += args->sliceSteps;
        for (uint64_t step=sub->posted-args->sliceSteps; step<sub->posted; step++) {
          ncclProfilingRecord(args, s, step, ncclProxyProfileSendGPUWait);
        }
        args->idle = 0;
        continue;
      }
/* 中文精读：发送状态机第 2 阶段：等 GPU 完成再提交网络
 * Simple 观察 sizesFifo 和 tail；LL/LL128 还要按具体缓冲位置检查协议 flag。
 * 只有整段数据就绪才调用 ncclNetIsend。若 request 返回 NULL，表示暂未接受，下一轮再试。
 * 请求有效后才清空 sizesFifo 并推进 transmitted，防止 GPU/Proxy 对一个槽重复提交。
 * 这里 CPU 只是提交 NIC 工作；GDR 时真实载荷可直接由 NIC 读取 GPU 缓冲，不必经 CPU memcpy。
 */
      // Check whether we received data from the GPU and send it to the network
      if (sub->transmitted < sub->posted && sub->transmitted < sub->done + NCCL_STEPS) {
        int buffSlot = (sub->base+sub->transmitted)%NCCL_STEPS;
        volatile int* sizesFifo = resources->recvMem->sizesFifo;
        volatile uint64_t* recvTail = &resources->recvMem->tail;
        if (sizesFifo[buffSlot] != -1 && ((*recvTail > (sub->base+sub->transmitted)) || p == NCCL_PROTO_LL)) {
          // We have something to receive, let's check if it's completely ready.
          int size = sizesFifo[buffSlot];
          char* buff = resources->shared ? localBuff+resources->recvMem->offsFifo[buffSlot] : localBuff+buffSlot*stepSize;
          int ready = 1;
          if (p == NCCL_PROTO_LL128) {
            ready = resources->useGdr;
            if (!ready) {
              // When data is in sysmem, we need to wait until all flags are correct since the GPU only
              // called threadfence()
              uint64_t flag = sub->base+sub->transmitted+1;
              int nFifoLines = DIVUP(sizesFifo[buffSlot], sizeof(uint64_t)*NCCL_LL128_LINEELEMS);
              volatile uint64_t* lines = (volatile uint64_t*)buff;
              ready = 1;
              for (int i=0; i<nFifoLines; i++) {
                if (lines[i*NCCL_LL128_LINEELEMS+NCCL_LL128_DATAELEMS] != flag) { ready = 0; break; }
              }
            }
          } else if (p == NCCL_PROTO_LL) {
            uint32_t flag = NCCL_LL_FLAG(sub->base+sub->transmitted+1);
            int nFifoLines = DIVUP(size, sizeof(union ncclLLFifoLine));
            union ncclLLFifoLine* lines = (union ncclLLFifoLine*)buff;
            for (int i=0; i<nFifoLines; i++) {
              volatile uint32_t *f1 = &lines[i].flag1;
              volatile uint32_t *f2 = &lines[i].flag2;
              if (f1[0] != flag || f2[0] != flag) { ready = 0; break; }
            }
          }
          if (ready) {
            // Data is ready, try to send.
            NCCLCHECK(ncclNetIsend(resources->netSendComm, buff, size, resources->rank, mhandle, sub->requests+buffSlot));
            if (sub->requests[buffSlot] != NULL) {
              TRACE(NCCL_NET, "sendProxy [%ld/%d] Isend posted, req %p", sub->transmitted, buffSlot, sub->requests[buffSlot]);
              sizesFifo[buffSlot] = -1;
              // Make sure size is reset to zero before we update the head.
              __sync_synchronize();
              sub->transmitted += args->sliceSteps;
              for (uint64_t step=sub->transmitted-args->sliceSteps; step<sub->transmitted; step++) ncclProfilingRecord(args, s, step, ncclProxyProfileSendWait);
              args->idle = 0;
              continue;
            }
          }
        }
      }
/* 中文精读：发送状态机第 3 阶段：网络完成后回收槽位
 * ncclNetTest 把异步请求推进到完成，done 才能增加，固定缓冲模式此时发布 head 给 GPU。
 * 先前 Isend 返回成功只表示请求被接受，不能立即让 GPU 覆盖仍可能被 NIC 读取的源数据。
 * 所有 sub 的 done 达到 nsteps 后，这个 ProxyArgs 才进入 None，连接本身仍保留供后续操作使用。
 * resources->step 记录跨操作的基准进度，与设备 conn->step 保持同一协议节奏。
 */
      // Check whether the network has completed some send operations.
      if (sub->done < sub->transmitted) {
        int done;
        int buffSlot = (sub->base+sub->done)%NCCL_STEPS;
        NCCLCHECK(ncclNetTest(sub->requests[buffSlot], &done, NULL));
        if (done) {
          TRACE(NCCL_NET, "sendProxy [%ld/%d] request %p done", sub->done, buffSlot, sub->requests[buffSlot]);
          sub->done += args->sliceSteps;
          for (uint64_t step=sub->done-args->sliceSteps; step<sub->done; step++) ncclProfilingRecord(args, s, step, ncclProxyProfileEnd);

          if (resources->shared == 0) {
            volatile uint64_t* sendHead = resources->gdcSync ? resources->gdcSync : &resources->sendMem->head;
            *sendHead = sub->base + sub->done;
            if (resources->gdcSync) wc_store_fence(); // Flush out WC write
          }
          args->idle = 0;
          if (sub->done == sub->nsteps) {
            resources->step = sub->base + sub->nsteps;
            args->done++;
          }
        }
      }
    }
    if (args->done == args->nsubs) {
      args->state = ncclProxyOpNone;
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f1095"></a>

## F1095：recvProxyProgress

用途：对应后端接收状态机：挂接收、检查网络完成、必要时 flush、发布给 GPU 并等待消费回收。

- Ready时把相同recvComm子项归组，Irecv挂接收后posted前进。
- Test完成先增加received；Simple+GDR等分支还需可见性flush，完成后才发布tail并推进transmitted。
- 最后等GPU消费head增加done；网络完成、GPU可读、GPU已消费是三个不同阶段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(args-&gt;state == ncclProxyOpReady)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>void* recvComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 准备局部状态 | <code>int groupSize = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 准备局部状态 | <code>int maxRecvs = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 3 / 条件判断 | <code>(groupSize == maxRecvs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 5 / 调用 / 计算 / 写入 | <code>groupSize = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 12 | 5 / 条件判断 | <code>(s&gt;0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 7 / 准备局部状态 | <code>int next;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 7 / 循环 | <code>初始 next=s；继续条件 next&lt;args-&gt;nsubs；每轮更新 next++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 8 / 准备局部状态 | <code>struct recvResources* nextRes = (struct recvResources*) (args-&gt;subs[next].connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 8 / 条件判断 | <code>(nextRes-&gt;netRecvComm == recvComm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 10 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 20 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 7 / 条件判断 | <code>(next == args-&gt;nsubs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 9 / 调用 / 计算 / 写入 | <code>groupSize = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 25 | 9 / 条件判断 | <code>(s != next)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 11 / 准备局部状态 | <code>struct ncclProxySubArgs temp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 11 / 调用 / 计算 / 写入 | <code>memcpy(&temp, sub, sizeof(struct ncclProxySubArgs));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 11 / 调用 / 计算 / 写入 | <code>memcpy(sub, args-&gt;subs+next, sizeof(struct ncclProxySubArgs));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 11 / 调用 / 计算 / 写入 | <code>memcpy(args-&gt;subs+next, &temp, sizeof(struct ncclProxySubArgs));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 3 / 调用 / 计算 / 写入 | <code>groupSize++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 3 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 3 / 调用 / 计算 / 写入 | <code>maxRecvs = resources-&gt;maxRecvs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 3 / 调用 / 计算 / 写入 | <code>recvComm = resources-&gt;netRecvComm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>sub-&gt;base = ROUNDUP(resources-&gt;step, args-&gt;chunkSteps);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 36 | 3 / 调用 / 计算 / 写入 | <code>sub-&gt;posted = sub-&gt;received = sub-&gt;transmitted = sub-&gt;done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 37 | 3 / 循环 | <code>初始 int i=0;；继续条件 i&lt;groupSize；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>sub[-i].groupSize = groupSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 40 | 3 / 循环 | <code>初始 uint64_t step=0;；继续条件 step&lt;sub-&gt;nsteps；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 41 | 4 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s, step, ncclProxyProfileBegin);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 42 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 43 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpProgress;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 0 / 条件判断 | <code>(args-&gt;state == ncclProxyOpProgress)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 2 / 准备局部状态 | <code>int p = args-&gt;protocol;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 2 / 准备局部状态 | <code>int maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args-&gt;nsubs);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 50 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s+=args-&gt;subs[s].groupSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 51 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* subGroup = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 3 / 准备局部状态 | <code>int subCount = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 53 | 3 / 准备局部状态 | <code>void* ptrs[NCCL_PROXY_MAX_SUBS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 3 / 准备局部状态 | <code>int sizes[NCCL_PROXY_MAX_SUBS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 55 | 3 / 准备局部状态 | <code>int tags[NCCL_PROXY_MAX_SUBS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 3 / 准备局部状态 | <code>void* mhandles[NCCL_PROXY_MAX_SUBS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 57 | 3 / 循环 | <code>初始 int i=0;；继续条件 i&lt;subGroup-&gt;groupSize；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 58 | 4 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = subGroup + i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 4 / 条件判断 | <code>(sub-&gt;posted &lt; sub-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 60 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 61 | 6 / 条件判断 | <code>(sub-&gt;posted &gt;= sub-&gt;done + maxDepth)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 62 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 63 | 8 / 调用 / 计算 / 写入 | <code>subCount = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 64 | 8 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 65 | 6 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 66 | 6 / 准备局部状态 | <code>int stepSize = resources-&gt;buffSizes[p] / NCCL_STEPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 67 | 6 / 准备局部状态 | <code>char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources-&gt;map, cpu, buffs[p]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 68 | 6 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;posted)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 69 | 6 / 条件判断 | <code>(resources-&gt;shared)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 70 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 71 | 8 / 准备局部状态 | <code>int sharedBuffSlot = sub-&gt;posted%maxDepth;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 72 | 8 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 73 | 8 / 调用 / 计算 / 写入 | <code>NCCLCHECK(sharedBuffersGet(comm, sub-&gt;channelId, sharedBuffSlot*args-&gt;nsubs+s+i, &offset));</code> | 失败时可能提前return，成功才继续；按 channel 和在途槽索引计算共享池字节偏移，后续通过 offsFifo 把该位置交给 GPU。 |
| 74 | 8 / 准备局部状态 | <code>volatile int* offsFifo = (volatile int*)resources-&gt;recvMem-&gt;offsFifo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 75 | 8 / 调用 / 计算 / 写入 | <code>offsFifo[buffSlot] = offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 76 | 8 / 调用 / 计算 / 写入 | <code>ptrs[subCount] = localBuff+offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 77 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 78 | 8 / 调用 / 计算 / 写入 | <code>ptrs[subCount] = localBuff+buffSlot*stepSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 6 / 调用 / 计算 / 写入 | <code>sizes[subCount] = stepSize*args-&gt;sliceSteps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 80 | 6 / 条件判断 | <code>(sub-&gt;nbytes &lt; sizes[subCount])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 81 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 82 | 8 / 调用 / 计算 / 写入 | <code>sizes[subCount] = sub-&gt;nbytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 6 / 调用 / 计算 / 写入 | <code>tags[subCount] = resources-&gt;remoteRank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 6 / 调用 / 计算 / 写入 | <code>mhandles[subCount] = resources-&gt;mhandles[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 6 / 调用 / 计算 / 写入 | <code>subCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 86 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 87 | 3 / 条件判断 | <code>(subCount)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 88 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 89 | 5 / 准备局部状态 | <code>uint64_t step = subGroup-&gt;posted;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 90 | 5 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (subGroup-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 91 | 5 / 准备局部状态 | <code>void** requestPtr = subGroup-&gt;requests+(step%NCCL_STEPS);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 92 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetIrecv(resources-&gt;netRecvComm, subCount, ptrs, sizes, tags, mhandles, requestPtr));</code> | 失败时可能提前return，成功才继续；提交一组带地址/长度/tag的异步接收；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 93 | 5 / 条件判断 | <code>(*requestPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 94 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 95 | 7 / 循环 | <code>初始 int i=0;；继续条件 i&lt;subGroup-&gt;groupSize；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 96 | 8 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = subGroup+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 97 | 8 / 调用 / 计算 / 写入 | <code>sub-&gt;posted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 98 | 8 / 循环 | <code>初始 uint64_t step=sub-&gt;posted-args-&gt;sliceSteps;；继续条件 step&lt;sub-&gt;posted；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 99 | 9 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvWait);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 100 | 8 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 101 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 102 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 103 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 104 | 2 / 条件判断 | <code>(args-&gt;idle == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 105 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 106 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 107 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s+=args-&gt;subs[s].groupSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 108 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* subGroup = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 109 | 3 / 条件判断 | <code>(subGroup-&gt;posted &gt; subGroup-&gt;received)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 110 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 111 | 5 / 准备局部状态 | <code>uint64_t step = subGroup-&gt;received;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 112 | 5 / 准备局部状态 | <code>int done;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 113 | 5 / 准备局部状态 | <code>void* ptrs[NCCL_PROXY_MAX_SUBS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 114 | 5 / 准备局部状态 | <code>int sizes[NCCL_PROXY_MAX_SUBS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 115 | 5 / 准备局部状态 | <code>void* mhandles[NCCL_PROXY_MAX_SUBS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 116 | 5 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_PROXY_MAX_SUBS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 117 | 6 / 调用 / 计算 / 写入 | <code>sizes[i] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 118 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 119 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetTest(subGroup-&gt;requests[step%NCCL_STEPS], &done, sizes));</code> | 失败时可能提前return，成功才继续；查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 120 | 5 / 条件判断 | <code>(done)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 121 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 122 | 7 / 准备局部状态 | <code>int useGdr = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 123 | 7 / 准备局部状态 | <code>int totalSize = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 124 | 7 / 循环 | <code>初始 int i=0;；继续条件 i&lt;NCCL_PROXY_MAX_SUBS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 125 | 8 / 调用 / 计算 / 写入 | <code>totalSize += sizes[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 126 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 127 | 7 / 循环 | <code>初始 int i=0;；继续条件 i&lt;subGroup-&gt;groupSize；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 128 | 8 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = subGroup + i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 129 | 8 / 调用 / 计算 / 写入 | <code>sub-&gt;received += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 130 | 8 / 循环 | <code>初始 uint64_t step=sub-&gt;received-args-&gt;sliceSteps;；继续条件 step&lt;sub-&gt;received；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 131 | 9 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvFlushWait);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 132 | 8 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 133 | 8 / 条件判断 | <code>(step &lt; sub-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 134 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 135 | 10 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 136 | 10 / 条件判断 | <code>(resources-&gt;useGdr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 137 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 138 | 12 / 调用 / 计算 / 写入 | <code>useGdr = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 139 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 140 | 7 / 调用 / 计算 / 写入 | <code>subGroup-&gt;requests[step%NCCL_STEPS] = NULL;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 141 | 7 / 条件判断 | <code>(totalSize &gt; 0 && p == NCCL_PROTO_SIMPLE && useGdr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 142 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 143 | 9 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (subGroup-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 144 | 9 / 条件判断 | <code>(resources-&gt;gdcFlush)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 145 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 146 | 11 / 编译期分支/指令 | <code>#if defined (__x86_64__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 147 | 12 / 调用 / 计算 / 写入 | <code>asm volatile ("mov (%0), %%eax" :: "l"(resources-&gt;gdcFlush) : "%eax");</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 148 | 12 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 149 | 13 / 调用 / 计算 / 写入 | <code>WARN("NET: GDR Flush only supported on x86_64");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 150 | 13 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 151 | 10 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 152 | 11 / 准备局部状态 | <code>int subCount = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 153 | 11 / 循环 | <code>初始 int i=0;；继续条件 i&lt;subGroup-&gt;groupSize；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 154 | 12 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = subGroup + i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 155 | 12 / 条件判断 | <code>(step &lt; sub-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 156 | 13 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 157 | 14 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 158 | 14 / 准备局部状态 | <code>int stepSize = resources-&gt;buffSizes[p] / NCCL_STEPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 159 | 14 / 准备局部状态 | <code>char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources-&gt;map, cpu, buffs[p]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 160 | 14 / 准备局部状态 | <code>int buffSlot = (sub-&gt;base+sub-&gt;posted)%NCCL_STEPS;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 161 | 14 / 调用 / 计算 / 写入 | <code>ptrs[subCount] = resources-&gt;shared ? localBuff+resources-&gt;recvMem-&gt;offsFifo[buffSlot] : localBuff+buffSlot*stepSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 162 | 14 / 调用 / 计算 / 写入 | <code>mhandles[subCount] = resources-&gt;mhandles[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 163 | 14 / 调用 / 计算 / 写入 | <code>subCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 164 | 11 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 165 | 11 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (subGroup-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 166 | 11 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetIflush(resources-&gt;netRecvComm, subCount, ptrs, sizes, mhandles, subGroup-&gt;requests+(step%NCCL_STEPS)));</code> | 失败时可能提前return，成功才继续；提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 167 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 168 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 169 | 2 / 条件判断 | <code>(args-&gt;idle == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 170 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 171 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 172 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s+=args-&gt;subs[s].groupSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 173 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* subGroup = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 174 | 3 / 条件判断 | <code>(subGroup-&gt;received &gt; subGroup-&gt;transmitted)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 175 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 176 | 5 / 准备局部状态 | <code>uint64_t step = subGroup-&gt;transmitted;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 177 | 5 / 准备局部状态 | <code>int done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 178 | 5 / 准备局部状态 | <code>void* request = subGroup-&gt;requests[step%NCCL_STEPS];</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 179 | 5 / 条件判断 | <code>(request)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 180 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 181 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNetTest(request, &done, NULL));</code> | 失败时可能提前return，成功才继续；查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。 |
| 182 | 5 / 条件判断 | <code>(done)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 183 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 184 | 7 / 循环 | <code>初始 int i=0;；继续条件 i&lt;subGroup-&gt;groupSize；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 185 | 8 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = subGroup + i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 186 | 8 / 调用 / 计算 / 写入 | <code>sub-&gt;transmitted += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 187 | 8 / 循环 | <code>初始 uint64_t step=sub-&gt;transmitted-args-&gt;sliceSteps;；继续条件 step&lt;sub-&gt;transmitted；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 188 | 9 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvGPUWait);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 189 | 8 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 190 | 8 / 条件判断 | <code>(step &lt; sub-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 191 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 192 | 10 / 调用 / 计算 / 写入 | <code>__sync_synchronize();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 193 | 10 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 194 | 10 / 准备局部状态 | <code>volatile uint64_t* recvTail = resources-&gt;gdcSync ? resources-&gt;gdcSync : &resources-&gt;recvMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 195 | 10 / 调用 / 计算 / 写入 | <code>*recvTail = sub-&gt;base + sub-&gt;transmitted;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 196 | 10 / 条件判断 | <code>(resources-&gt;gdcSync)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 197 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 198 | 12 / 调用 / 计算 / 写入 | <code>wc_store_fence();</code> | 按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。 |
| 199 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 200 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 201 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 202 | 2 / 条件判断 | <code>(args-&gt;idle == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 203 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 204 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 205 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;args-&gt;nsubs；每轮更新 s+=args-&gt;subs[s].groupSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 206 | 3 / 准备局部状态 | <code>struct ncclProxySubArgs* subGroup = args-&gt;subs+s;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 207 | 3 / 循环 | <code>初始 int i=0;；继续条件 i&lt;subGroup-&gt;groupSize；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 208 | 4 / 准备局部状态 | <code>struct ncclProxySubArgs* sub = subGroup + i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 209 | 4 / 条件判断 | <code>(sub-&gt;done == sub-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 210 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 211 | 6 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 212 | 4 / 条件判断 | <code>(sub-&gt;transmitted &gt; sub-&gt;done)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 213 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 214 | 6 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 215 | 6 / 准备局部状态 | <code>volatile uint64_t* sendHead = &resources-&gt;sendMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 216 | 6 / 准备局部状态 | <code>uint64_t done = *sendHead;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 217 | 6 / 循环 | <code>初始 ；继续条件 (done &gt; sub-&gt;base + sub-&gt;done && sub-&gt;transmitted &gt; sub-&gt;done)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 218 | 7 / 调用 / 计算 / 写入 | <code>sub-&gt;done += args-&gt;sliceSteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 219 | 7 / 循环 | <code>初始 uint64_t step=sub-&gt;done-args-&gt;sliceSteps;；继续条件 step&lt;sub-&gt;done；每轮更新 step++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 220 | 8 / 调用 / 计算 / 写入 | <code>ncclProfilingRecord(args, s+i, step, ncclProxyProfileEnd);</code> | 若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。 |
| 221 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 222 | 7 / 调用 / 计算 / 写入 | <code>args-&gt;idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 223 | 7 / 条件判断 | <code>(sub-&gt;done == sub-&gt;nsteps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 224 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 225 | 9 / 准备局部状态 | <code>struct recvResources* resources = (struct recvResources*) (sub-&gt;connection-&gt;transportResources);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 226 | 9 / 调用 / 计算 / 写入 | <code>resources-&gt;step = sub-&gt;base + sub-&gt;nsteps;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 227 | 9 / 调用 / 计算 / 写入 | <code>args-&gt;done++;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 228 | 9 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 229 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 230 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 231 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 232 | 2 / 条件判断 | <code>(args-&gt;done == args-&gt;nsubs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 233 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 234 | 4 / 调用 / 计算 / 写入 | <code>args-&gt;state = ncclProxyOpNone;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 235 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (args->state == ncclProxyOpReady) {
/* 中文精读：接收聚合不等于共享接收队列 SRQ
 * 把使用同一 backend recvComm 的子操作排在一起，最多按 maxRecvs 聚合一次 Irecv。
 * 每个 sub 仍有自己的 channel/连接/步数/指针/标签；后端据此匹配正确发送方。
 * 这里是 NCCL net 接口层的批量提交，是否使用 verbs SRQ 取决于后端实现。
 * 本仓库内置 net_ib 没有绑定 SRQ，不能由 maxRecvs>1 推断硬件 SRQ 已启用。
 */
    // Initialize subs and group them by same recvComm.
    void* recvComm;
    int groupSize = 0;
    int maxRecvs = 1;
    for (int s=0; s<args->nsubs; s++) {
      struct ncclProxySubArgs* sub = args->subs+s;
      if (groupSize == maxRecvs) {
        groupSize = 0;
      } else if (s>0) { // Find next sub with the same recvComm
        int next;
        for (next=s; next<args->nsubs; next++) {
          struct recvResources* nextRes = (struct recvResources*) (args->subs[next].connection->transportResources);
          if (nextRes->netRecvComm == recvComm) break;
        }
        if (next == args->nsubs) { // Not found
          groupSize = 0;
        } else if (s != next) { // We found a sub later with the same recvComm ; swap subs
          struct ncclProxySubArgs temp;
          memcpy(&temp, sub, sizeof(struct ncclProxySubArgs));
          memcpy(sub, args->subs+next, sizeof(struct ncclProxySubArgs));
          memcpy(args->subs+next, &temp, sizeof(struct ncclProxySubArgs));
        }
      }
      groupSize++;
      struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
      maxRecvs = resources->maxRecvs;
      recvComm = resources->netRecvComm;
      // Round to next multiple of sliceSteps
      sub->base = ROUNDUP(resources->step, args->chunkSteps);
      sub->posted = sub->received = sub->transmitted = sub->done = 0;
      for (int i=0; i<groupSize; i++) sub[-i].groupSize = groupSize;
      for (uint64_t step=0; step<sub->nsteps; step++) ncclProfilingRecord(args, s, step, ncclProxyProfileBegin);
    }
    args->state = ncclProxyOpProgress;
  }
  args->idle = 1;
  if (args->state == ncclProxyOpProgress) {
    int p = args->protocol;
    int maxDepth = std::min(NCCL_STEPS, NCCL_SHARED_STEPS/args->nsubs);
    for (int s=0; s<args->nsubs; s+=args->subs[s].groupSize) {
      struct ncclProxySubArgs* subGroup = args->subs+s;
      int subCount = 0;
      void* ptrs[NCCL_PROXY_MAX_SUBS];
      int sizes[NCCL_PROXY_MAX_SUBS];
      int tags[NCCL_PROXY_MAX_SUBS];
      void* mhandles[NCCL_PROXY_MAX_SUBS];

      for (int i=0; i<subGroup->groupSize; i++) {
        struct ncclProxySubArgs* sub = subGroup + i;
        if (sub->posted < sub->nsteps) {
          if (sub->posted >= sub->done + maxDepth) { subCount = 0; break; }
          struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
          int stepSize = resources->buffSizes[p] / NCCL_STEPS;
          char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);
          int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
          if (resources->shared) {
            int sharedBuffSlot = sub->posted%maxDepth;
            int offset;
            NCCLCHECK(sharedBuffersGet(comm, sub->channelId, sharedBuffSlot*args->nsubs+s+i, &offset));
            volatile int* offsFifo = (volatile int*)resources->recvMem->offsFifo;
            offsFifo[buffSlot] = offset;
            ptrs[subCount] = localBuff+offset;
          } else {
            ptrs[subCount] = localBuff+buffSlot*stepSize;
          }
          sizes[subCount] = stepSize*args->sliceSteps;
          if (sub->nbytes < sizes[subCount]) sizes[subCount] = sub->nbytes;
          tags[subCount] = resources->remoteRank;
          mhandles[subCount] = resources->mhandles[p];
          subCount++;
        }
      }
      if (subCount) {
        uint64_t step = subGroup->posted;
        struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);
        void** requestPtr = subGroup->requests+(step%NCCL_STEPS);
        NCCLCHECK(ncclNetIrecv(resources->netRecvComm, subCount, ptrs, sizes, tags, mhandles, requestPtr));
        if (*requestPtr) {
          for (int i=0; i<subGroup->groupSize; i++) {
            struct ncclProxySubArgs* sub = subGroup+i;
            sub->posted += args->sliceSteps;
            for (uint64_t step=sub->posted-args->sliceSteps; step<sub->posted; step++) ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvWait);
          }
          args->idle = 0;
        }
      }
    }
    if (args->idle == 0) return ncclSuccess;

    for (int s=0; s<args->nsubs; s+=args->subs[s].groupSize) {
      struct ncclProxySubArgs* subGroup = args->subs+s;
      if (subGroup->posted > subGroup->received) {
        uint64_t step = subGroup->received;
        int done;
        void* ptrs[NCCL_PROXY_MAX_SUBS];
        int sizes[NCCL_PROXY_MAX_SUBS];
        void* mhandles[NCCL_PROXY_MAX_SUBS];
        for (int i=0; i<NCCL_PROXY_MAX_SUBS; i++) sizes[i] = 0;
        NCCLCHECK(ncclNetTest(subGroup->requests[step%NCCL_STEPS], &done, sizes));
        if (done) {
          int useGdr = 0;
          int totalSize = 0;
          for (int i=0; i<NCCL_PROXY_MAX_SUBS; i++) totalSize += sizes[i];
          for (int i=0; i<subGroup->groupSize; i++) {
            struct ncclProxySubArgs* sub = subGroup + i;
            sub->received += args->sliceSteps;
            for (uint64_t step=sub->received-args->sliceSteps; step<sub->received; step++) ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvFlushWait);
            if (step < sub->nsteps) {
              struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
              if (resources->useGdr) useGdr = 1;
            }
          }
          subGroup->requests[step%NCCL_STEPS] = NULL;
/* 中文精读：接收完成还要考虑 GPU 可见性
 * 网络 Test 完成意味着后端请求结束，某些 GDR 路径还要执行 flush 确保 GPU 后续能看见数据。
 * 支持时走 GDRCopy 映射读，否则调用 net 后端 Iflush，再等待 flush 请求完成。
 * 之后才能发布接收 tail，GPU 才能离开 waitPeer 读取这段数据。
 * flush 不是把整个张量重新传一次；内置 IB 在特定配置下用小型自连接 RDMA read 实现。
 */
          if (totalSize > 0 && p == NCCL_PROTO_SIMPLE && useGdr) {
            // GDRCOPY support
            struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);
            if (resources->gdcFlush) {
#if defined (__x86_64__)
              // Force a PCI-E read from GPU memory
              asm volatile ("mov (%0), %%eax" :: "l"(resources->gdcFlush) : "%eax");
#else
              WARN("NET: GDR Flush only supported on x86_64");
              return ncclInternalError;
#endif
            } else {
              int subCount = 0;
              for (int i=0; i<subGroup->groupSize; i++) {
                struct ncclProxySubArgs* sub = subGroup + i;
                if (step < sub->nsteps) {
                  struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
                  int stepSize = resources->buffSizes[p] / NCCL_STEPS;
                  char* localBuff = NCCL_NET_MAP_GET_POINTER(&resources->map, cpu, buffs[p]);
                  int buffSlot = (sub->base+sub->posted)%NCCL_STEPS;
                  ptrs[subCount] = resources->shared ? localBuff+resources->recvMem->offsFifo[buffSlot] : localBuff+buffSlot*stepSize;
                  mhandles[subCount] = resources->mhandles[p];
                  subCount++;
                }
              }
              struct recvResources* resources = (struct recvResources*) (subGroup->connection->transportResources);
              NCCLCHECK(ncclNetIflush(resources->netRecvComm, subCount, ptrs, sizes, mhandles, subGroup->requests+(step%NCCL_STEPS)));
            }
          }
          args->idle = 0;
        }
      }
    }
    if (args->idle == 0) return ncclSuccess;

    for (int s=0; s<args->nsubs; s+=args->subs[s].groupSize) {
      struct ncclProxySubArgs* subGroup = args->subs+s;
      if (subGroup->received > subGroup->transmitted) {
        uint64_t step = subGroup->transmitted;
        int done = 1;
        void* request = subGroup->requests[step%NCCL_STEPS];
        if (request) NCCLCHECK(ncclNetTest(request, &done, NULL));
        if (done) {
          for (int i=0; i<subGroup->groupSize; i++) {
            struct ncclProxySubArgs* sub = subGroup + i;
            sub->transmitted += args->sliceSteps;
            for (uint64_t step=sub->transmitted-args->sliceSteps; step<sub->transmitted; step++) ncclProfilingRecord(args, s+i, step, ncclProxyProfileRecvGPUWait);
            if (step < sub->nsteps) {
              __sync_synchronize();
              struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
              volatile uint64_t* recvTail = resources->gdcSync ? resources->gdcSync : &resources->recvMem->tail;
              *recvTail = sub->base + sub->transmitted;
              if (resources->gdcSync) wc_store_fence(); // Flush out WC write
            }
          }
          args->idle = 0;
        }
      }
    }
    if (args->idle == 0) return ncclSuccess;

    for (int s=0; s<args->nsubs; s+=args->subs[s].groupSize) {
      struct ncclProxySubArgs* subGroup = args->subs+s;
      for (int i=0; i<subGroup->groupSize; i++) {
        struct ncclProxySubArgs* sub = subGroup + i;
        if (sub->done == sub->nsteps) continue;
        if (sub->transmitted > sub->done) {
          struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
          volatile uint64_t* sendHead = &resources->sendMem->head;
          uint64_t done = *sendHead;
          while (done > sub->base + sub->done &&
              // LL and LL128 can acknowledge 0-bytes send before they even happen. Don't go past what we transmitted.
              sub->transmitted > sub->done) {
            sub->done += args->sliceSteps;
            for (uint64_t step=sub->done-args->sliceSteps; step<sub->done; step++) ncclProfilingRecord(args, s+i, step, ncclProxyProfileEnd);
            args->idle = 0;
            if (sub->done == sub->nsteps) {
              struct recvResources* resources = (struct recvResources*) (sub->connection->transportResources);
              resources->step = sub->base + sub->nsteps;
              args->done++;
              break;
            }
          }
        }
      }
    }
    if (args->done == args->nsubs) {
      args->state = ncclProxyOpNone;
    }
  }
  return ncclSuccess;
}
```

</details>

