# src/transport/shm.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1165"></a>

## F1165：shmCanConnect

用途：检查同机与同 /dev/shm 设备且 SHM 未禁用，决定共享主机缓冲是否可作回退 transport。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*ret = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(ncclParamShmDisable() == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT&#124;NCCL_SHM, "peer1 hostHash %lx peer2 hostHash %lx", info1-&gt;hostHash, info2-&gt;hostHash);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 条件判断 | <code>(info1-&gt;hostHash != info2-&gt;hostHash)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT&#124;NCCL_SHM, "peer1 shmDev %lx peer2 shmDev %lx", info1-&gt;shmDev, info2-&gt;shmDev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 条件判断 | <code>(info1-&gt;shmDev != info2-&gt;shmDev)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>*ret = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *ret = 0;

  if (ncclParamShmDisable() == 1) return ncclSuccess;

  // Same host?
  TRACE(NCCL_INIT|NCCL_SHM, "peer1 hostHash %lx peer2 hostHash %lx", info1->hostHash, info2->hostHash);
  if (info1->hostHash != info2->hostHash) return ncclSuccess;

  // Common /dev/shm (between containers) ?
  TRACE(NCCL_INIT|NCCL_SHM, "peer1 shmDev %lx peer2 shmDev %lx", info1->shmDev, info2->shmDev);
  if (info1->shmDev != info2->shmDev) return ncclSuccess;

  *ret = 1;

  return ncclSuccess;
}
```

</details>

<a id="f1166"></a>

## F1166：shmSendSetup

用途：分别创建发送控制区或接收控制加协议数据区，并把可导入共享文件信息放入建链句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct shmSendResources* resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 源码语法片段 | <code>static_assert(sizeof(struct shmConnectInfo) &lt;= sizeof(struct ncclConnect), "shm Connect Info is too big");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 5 | 0 / 准备局部状态 | <code>struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>char shmPath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>shmPath[0] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;shmSize = resources-&gt;shmSize = sizeof(struct ncclSendMem);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmOpen(shmPath, resources-&gt;shmSize, (void**)&resources-&gt;hostMem, (void**)&resources-&gt;devHostMem, 1));</code> | 失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_SHM,"Opened shmName %s shmSize %d", shmPath, info-&gt;shmSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>memcpy(info-&gt;shmName, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof(info-&gt;shmName));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_SHM,"Channel %02d : %d[%lx] -&gt; %d[%lx] via direct shared memory", channelId, myInfo-&gt;rank, myInfo-&gt;busId, peerInfo-&gt;rank, peerInfo-&gt;busId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct shmSendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  send->transportResources = resources;

  static_assert(sizeof(struct shmConnectInfo) <= sizeof(struct ncclConnect), "shm Connect Info is too big");
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

  char shmPath[PATH_MAX];
  shmPath[0] = '\0';
  info->shmSize = resources->shmSize = sizeof(struct ncclSendMem);
  NCCLCHECK(ncclShmOpen(shmPath, resources->shmSize, (void**)&resources->hostMem, (void**)&resources->devHostMem, 1));
  TRACE(NCCL_SHM,"Opened shmName %s shmSize %d", shmPath, info->shmSize);
  memcpy(info->shmName, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof(info->shmName));

  INFO(NCCL_INIT|NCCL_SHM,"Channel %02d : %d[%lx] -> %d[%lx] via direct shared memory", channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId);
  return ncclSuccess;
}
```

</details>

<a id="f1167"></a>

## F1167：shmRecvSetup

用途：分别创建发送控制区或接收控制加协议数据区，并把可导入共享文件信息放入建链句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct shmRecvResources* resources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&resources, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 源码语法片段 | <code>static_assert(sizeof(struct shmConnectInfo) &lt;= sizeof(struct ncclConnect), "shm Connect Info is too big");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 5 | 0 / 准备局部状态 | <code>struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>char shmPath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>shmPath[0] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 准备局部状态 | <code>int shmSize = sizeof(struct ncclRecvMem);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>shmSize += recv-&gt;comm-&gt;buffSizes[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;shmSize = resources-&gt;shmSize = shmSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmOpen(shmPath, resources-&gt;shmSize, (void**)&resources-&gt;hostMem, (void**)&resources-&gt;devHostMem, 1));</code> | 失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_SHM,"Opened shmName %s shmSize %d", shmPath, info-&gt;shmSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>memcpy(info-&gt;shmName, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof(info-&gt;shmName));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct shmRecvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  recv->transportResources = resources;

  static_assert(sizeof(struct shmConnectInfo) <= sizeof(struct ncclConnect), "shm Connect Info is too big");
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

  char shmPath[PATH_MAX];
  shmPath[0] = '\0';
  int shmSize = sizeof(struct ncclRecvMem);
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) shmSize += recv->comm->buffSizes[p];
  info->shmSize = resources->shmSize = shmSize;
  NCCLCHECK(ncclShmOpen(shmPath, resources->shmSize, (void**)&resources->hostMem, (void**)&resources->devHostMem, 1));
  TRACE(NCCL_SHM,"Opened shmName %s shmSize %d", shmPath, info->shmSize);
  memcpy(info->shmName, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof(info->shmName));

  return ncclSuccess;
}
```

</details>

<a id="f1168"></a>

## F1168：shmSendConnect

用途：打开对端共享文件，取得 CPU/GPU 映射地址并按各协议区偏移填写连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct shmSendResources* resources = (struct shmSendResources*)send-&gt;transportResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>char shmPath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>sprintf(shmPath, "/dev/shm/nccl-%s", info-&gt;shmName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;remShmSize = info-&gt;shmSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_SHM,"Open shmName %s shmSize %d", shmPath, info-&gt;shmSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmOpen(shmPath, resources-&gt;remShmSize, (void**)&resources-&gt;remHostMem, (void**)&resources-&gt;devRemHostMem, 0));</code> | 失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmUnlink(shmPath));</code> | 失败时可能提前return，成功才继续；删除共享内存文件名，已打开的映射仍由各进程按生命周期单独解除。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;transportResources = resources;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>send-&gt;conn.buffs[p] = (char*)(resources-&gt;devRemHostMem+1) + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>offset += send-&gt;comm-&gt;buffSizes[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.tail = &resources-&gt;devRemHostMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>send-&gt;conn.head = &resources-&gt;devHostMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Setup device pointers
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;
  struct shmSendResources* resources = (struct shmSendResources*)send->transportResources;

  char shmPath[PATH_MAX];
  sprintf(shmPath, "/dev/shm/nccl-%s", info->shmName);
  resources->remShmSize = info->shmSize;
  TRACE(NCCL_SHM,"Open shmName %s shmSize %d", shmPath, info->shmSize);
  NCCLCHECK(ncclShmOpen(shmPath, resources->remShmSize, (void**)&resources->remHostMem, (void**)&resources->devRemHostMem, 0));
  // Remove the file to ensure proper clean-up
  NCCLCHECK(ncclShmUnlink(shmPath));

  send->transportResources = resources;
  int offset = 0;
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    send->conn.buffs[p] = (char*)(resources->devRemHostMem+1) + offset;
    offset += send->comm->buffSizes[p];
  }
  send->conn.tail = &resources->devRemHostMem->tail;

  send->conn.head = &resources->devHostMem->head;
  return ncclSuccess;
}
```

</details>

<a id="f1169"></a>

## F1169：shmRecvConnect

用途：打开对端共享文件，取得 CPU/GPU 映射地址并按各协议区偏移填写连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct shmRecvResources* resources = (struct shmRecvResources*)recv-&gt;transportResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>char shmPath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>sprintf(shmPath, "/dev/shm/nccl-%s", info-&gt;shmName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>resources-&gt;remShmSize = info-&gt;shmSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_SHM,"Open shmName %s shmSize %d", shmPath, info-&gt;shmSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmOpen(shmPath, resources-&gt;remShmSize, (void**)&resources-&gt;remHostMem, (void**)&resources-&gt;devRemHostMem, 0));</code> | 失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmUnlink(shmPath));</code> | 失败时可能提前return，成功才继续；删除共享内存文件名，已打开的映射仍由各进程按生命周期单独解除。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.head = &resources-&gt;devRemHostMem-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 10 | 0 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 循环 | <code>初始 int p=0;；继续条件 p&lt;NCCL_NUM_PROTOCOLS；每轮更新 p++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.buffs[p] = (char*)(resources-&gt;devHostMem+1) + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>offset += recv-&gt;comm-&gt;buffSizes[p];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>recv-&gt;conn.tail = &resources-&gt;devHostMem-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Setup device pointers
  struct shmRecvResources* resources = (struct shmRecvResources*)recv->transportResources;
  struct shmConnectInfo* info = (struct shmConnectInfo*)connectInfo;

  char shmPath[PATH_MAX];
  sprintf(shmPath, "/dev/shm/nccl-%s", info->shmName);
  resources->remShmSize = info->shmSize;
  TRACE(NCCL_SHM,"Open shmName %s shmSize %d", shmPath, info->shmSize);
  NCCLCHECK(ncclShmOpen(shmPath, resources->remShmSize, (void**)&resources->remHostMem, (void**)&resources->devRemHostMem, 0));
  NCCLCHECK(ncclShmUnlink(shmPath));
  recv->conn.head = &resources->devRemHostMem->head;

  int offset = 0;
  for (int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    recv->conn.buffs[p] = (char*)(resources->devHostMem+1) + offset;
    offset += recv->comm->buffSizes[p];
  }
  recv->conn.tail = &resources->devHostMem->tail;
  return ncclSuccess;
}
```

</details>

<a id="f1170"></a>

## F1170：shmSendFree

用途：解除对端与本地共享主机映射及 CUDA 注册，按拥有关系删除共享对象名字。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct shmRecvResources* resources = (struct shmRecvResources*)send-&gt;transportResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmClose(resources-&gt;hostMem, resources-&gt;devHostMem, resources-&gt;shmSize));</code> | 失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmClose(resources-&gt;remHostMem, resources-&gt;devRemHostMem, resources-&gt;remShmSize));</code> | 失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>free(resources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct shmRecvResources* resources = (struct shmRecvResources*)send->transportResources;
  NCCLCHECK(ncclShmClose(resources->hostMem, resources->devHostMem, resources->shmSize));
  NCCLCHECK(ncclShmClose(resources->remHostMem, resources->devRemHostMem, resources->remShmSize));
  free(resources);
  return ncclSuccess;
}
```

</details>

<a id="f1171"></a>

## F1171：shmRecvFree

用途：解除对端与本地共享主机映射及 CUDA 注册，按拥有关系删除共享对象名字。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct shmRecvResources* resources = (struct shmRecvResources*)recv-&gt;transportResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmClose(resources-&gt;hostMem, resources-&gt;devHostMem, resources-&gt;shmSize));</code> | 失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclShmClose(resources-&gt;remHostMem, resources-&gt;devRemHostMem, resources-&gt;remShmSize));</code> | 失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>free(resources);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct shmRecvResources* resources = (struct shmRecvResources*)recv->transportResources;
  NCCLCHECK(ncclShmClose(resources->hostMem, resources->devHostMem, resources->shmSize));
  NCCLCHECK(ncclShmClose(resources->remHostMem, resources->devRemHostMem, resources->remShmSize));
  free(resources);
  return ncclSuccess;
}
```

</details>

