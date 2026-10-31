# src/include/enqueue.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0632"></a>

## F0632：ncclCreateQueueInfo

用途：分配一个提交/图持有的 QueueInfo 和可复用元素列表，记录 communicator 队列生命周期计数。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(eqInfo, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>(*eqInfo)-&gt;comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>(*eqInfo)-&gt;elemList = new ncclQueueElemList();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>(*eqInfo)-&gt;comm-&gt;nQueueInfoCreated++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclCalloc(eqInfo, 1));
  (*eqInfo)->comm = comm;
  (*eqInfo)->elemList = new ncclQueueElemList();
  (*eqInfo)->comm->nQueueInfoCreated++;
  return ncclSuccess;
}
```

</details>

<a id="f0633"></a>

## F0633：ncclResetQueueInfo

用途：清理一批已经提交的队列统计并 recycle 元素节点，保留容量供后续批次复用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(eqInfo == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>eqInfo-&gt;maxChannels = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>eqInfo-&gt;ret = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>eqInfo-&gt;nRegBuffs = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>eqInfo-&gt;elemList-&gt;recycle();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (eqInfo == NULL) return ncclInternalError;
  eqInfo->maxChannels = 0;
  eqInfo->ret = ncclSuccess;
  eqInfo->nRegBuffs = 0;
  eqInfo->elemList->recycle();
  return ncclSuccess;
}
```

</details>

<a id="f0634"></a>

## F0634：ncclDestroyQueueInfo

用途：释放图/普通提交持有的队列，把不能在图析构回调直接关闭的 CUDA IPC 句柄转交辅助线程。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ptr == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclQueueInfo* eqInfo = (struct ncclQueueInfo*)ptr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclComm* comm = eqInfo-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclQueueElem* eqElem = eqInfo-&gt;elemList-&gt;begin();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 编译期分支/指令 | <code>#if 0</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 8 | 1 / 循环 | <code>初始 ；继续条件 (eqElem != NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;eqElem-&gt;buffRegInfo.nBuffs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 3 / 条件判断 | <code>(i == eqInfo-&gt;comm-&gt;localRank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 13 | 3 / 调用 / 计算 / 写入 | <code>CUDACHECKIGNORE(cudaIpcCloseMemHandle(eqElem-&gt;buffRegInfo.sendbuffsBase[i]));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 3 / 调用 / 计算 / 写入 | <code>CUDACHECKIGNORE(cudaIpcCloseMemHandle(eqElem-&gt;buffRegInfo.recvbuffsBase[i]));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 15 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>eqElem = eqInfo-&gt;elemList-&gt;getNext();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 19 | 2 / 准备局部状态 | <code>struct ncclGraphHelperResources* res = comm-&gt;graphHelperResources;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 2 / 准备局部状态 | <code>int ipcTailOld = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 2 / 条件判断 | <code>(res == NULL &#124;&#124; (!comm-&gt;graphHelperThread) &#124;&#124; eqInfo-&gt;nRegBuffs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 控制转移 | <code>goto skip;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>ipcTailOld = res-&gt;ipcTail;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 2 / 循环 | <code>初始 ；继续条件 (eqElem != NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 3 / 循环 | <code>初始 int i=0;；继续条件 i&lt;eqElem-&gt;buffRegInfo.nBuffs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 28 | 4 / 条件判断 | <code>(eqElem-&gt;buffRegInfo.sendbuffsBase[i] != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 6 / 调用 / 计算 / 写入 | <code>res-&gt;ipcBases[res-&gt;ipcTail] = eqElem-&gt;buffRegInfo.sendbuffsBase[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 6 / 调用 / 计算 / 写入 | <code>res-&gt;ipcTail = (res-&gt;ipcTail+1)%NCCL_IPC_POOL_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 4 / 条件判断 | <code>(eqElem-&gt;buffRegInfo.recvbuffsBase[i] != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 6 / 调用 / 计算 / 写入 | <code>res-&gt;ipcBases[res-&gt;ipcTail] = eqElem-&gt;buffRegInfo.recvbuffsBase[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 6 / 调用 / 计算 / 写入 | <code>res-&gt;ipcTail = (res-&gt;ipcTail+1)%NCCL_IPC_POOL_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 37 | 3 / 调用 / 计算 / 写入 | <code>eqElem = eqInfo-&gt;elemList-&gt;getNext();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 39 | 2 / 条件判断 | <code>(res-&gt;ipcTail != ipcTailOld)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 4 / 调用 / 计算 / 写入 | <code>res-&gt;threadState = ThreadStart;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 4 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_COLL, "CUDA Graph destroy function signaling helper thread with %d IPC handles", res-&gt;ipcTail-ipcTailOld);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 4 / 调用 / 计算 / 写入 | <code>pthread_cond_signal(&res-&gt;threadCond);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 0 / 跳转标签 | <code>skip</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 46 | 1 / 调用 / 计算 / 写入 | <code>delete eqInfo-&gt;elemList;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>free(eqInfo);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;nQueueInfoDestroyed++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 0 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ptr == NULL) return;
  struct ncclQueueInfo* eqInfo = (struct ncclQueueInfo*)ptr;
  struct ncclComm* comm = eqInfo->comm;
  // Close IPC mem handles for registered buffers
  struct ncclQueueElem* eqElem = eqInfo->elemList->begin();
#if 0
  // Ideally, the deregistration should happen here
  // but currently the destroy function of CUDA objects does not allow CUDA API calls
  while (eqElem != NULL) {
    for (int i=0; i<eqElem->buffRegInfo.nBuffs; i++) {
      if (i == eqInfo->comm->localRank) continue;
      CUDACHECKIGNORE(cudaIpcCloseMemHandle(eqElem->buffRegInfo.sendbuffsBase[i]));
      CUDACHECKIGNORE(cudaIpcCloseMemHandle(eqElem->buffRegInfo.recvbuffsBase[i]));
    }
    eqElem = eqInfo->elemList->getNext();
  }
#else
  // Instead, we push these pointers to a pool owned by ncclComm
  // and asks a helper thread to close mem handles
  struct ncclGraphHelperResources* res = comm->graphHelperResources;
  int ipcTailOld = 0;
  if (res == NULL || (!comm->graphHelperThread) || eqInfo->nRegBuffs == 0) goto skip;

  pthread_mutex_lock(&res->threadLock);
  ipcTailOld = res->ipcTail;
  while (eqElem != NULL) {
    for (int i=0; i<eqElem->buffRegInfo.nBuffs; i++) {
      if (eqElem->buffRegInfo.sendbuffsBase[i] != NULL) {
        res->ipcBases[res->ipcTail] = eqElem->buffRegInfo.sendbuffsBase[i];
        res->ipcTail = (res->ipcTail+1)%NCCL_IPC_POOL_SIZE;
      }
      if (eqElem->buffRegInfo.recvbuffsBase[i] != NULL) {
        res->ipcBases[res->ipcTail] = eqElem->buffRegInfo.recvbuffsBase[i];
        res->ipcTail = (res->ipcTail+1)%NCCL_IPC_POOL_SIZE;
      }
    }
    eqElem = eqInfo->elemList->getNext();
  }
  if (res->ipcTail != ipcTailOld) {
    res->threadState = ThreadStart;
    TRACE(NCCL_COLL, "CUDA Graph destroy function signaling helper thread with %d IPC handles", res->ipcTail-ipcTailOld);
    pthread_cond_signal(&res->threadCond);
  }
  pthread_mutex_unlock(&res->threadLock);
#endif

skip:
  delete eqInfo->elemList;
  free(eqInfo);
  comm->nQueueInfoDestroyed++;
  return;
}
```

</details>

