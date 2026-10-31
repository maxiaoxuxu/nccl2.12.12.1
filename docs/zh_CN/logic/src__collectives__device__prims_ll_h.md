# src/collectives/device/prims_ll.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0157"></a>

## F0157：recvOffset

用途：用连接 step 对 NCCL_STEPS 取模再乘每槽大小，把单调进度映射到循环数据槽位。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (recvStep[i]%NCCL_STEPS)*stepLines;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return (recvStep[i]%NCCL_STEPS)*stepLines; }
```

</details>

<a id="f0158"></a>

## F0158：sendOffset

用途：用连接 step 对 NCCL_STEPS 取模再乘每槽大小，把单调进度映射到循环数据槽位。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (sendStep[i]%NCCL_STEPS)*stepLines;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return (sendStep[i]%NCCL_STEPS)*stepLines; }
```

</details>

<a id="f0159"></a>

## F0159：recvPtr

用途：以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return recvBuff[i]+recvOffset(i);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return recvBuff[i]+recvOffset(i); }
```

</details>

<a id="f0160"></a>

## F0160：sendPtr

用途：以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return sendBuff[i]+sendOffset(i);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return sendBuff[i]+sendOffset(i); }
```

</details>

<a id="f0161"></a>

## F0161：recvFlag

用途：把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return NCCL_LL_FLAG(recvStep[i]+1);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return NCCL_LL_FLAG(recvStep[i]+1); }
```

</details>

<a id="f0162"></a>

## F0162：sendFlag

用途：把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return NCCL_LL_FLAG(sendStep[i]+1);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return NCCL_LL_FLAG(sendStep[i]+1); }
```

</details>

<a id="f0163"></a>

## F0163：barrier

用途：同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile ("bar.sync %1, %0;" :: "r"(nthreads), "r"(15-group));</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    asm volatile ("bar.sync %1, %0;" :: "r"(nthreads), "r"(15-group));
  }
```

</details>

<a id="f0164"></a>

## F0164：checkAbort

用途：自旋达到检查间隔时读取 communicator 中止标志，使异常情况下的协议等待可以退出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>spins++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(abort == 0 && spins == NCCL_SPINS_BEFORE_CHECK_ABORT)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>abort = *ncclShmem.comm.abortFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>spins = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return abort;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    spins++;
    if (abort == 0 && spins == NCCL_SPINS_BEFORE_CHECK_ABORT) {
      abort = *ncclShmem.comm.abortFlag;
      spins = 0;
    }
    return abort;
  }
```

</details>

<a id="f0165"></a>

## F0165：waitSend

用途：等待发送缓冲仍有可复用槽位，并在存在 Proxy 时填写本轮物理字节数，防止覆盖 NIC/对端仍使用的数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(sendConnHeadPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 循环 | <code>初始 ；继续条件 (sendConnHeadCache + NCCL_STEPS &lt; sendConnHead + 1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>sendConnHeadCache = *sendConnHeadPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 条件判断 | <code>(checkAbort(spins, 1))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 9 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 2 / 条件判断 | <code>(sendConnFifoPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 4 / 准备局部状态 | <code>int size = ((sendConnHead & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK) ? stepLines*sizeof(union ncclLLFifoLine) : nbytes;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>sendConnFifoPtr[sendConnHead%NCCL_STEPS] = size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>sendConnHead += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (sendConnHeadPtr) {
      int spins = 0;
      while (sendConnHeadCache + NCCL_STEPS < sendConnHead + 1) {
        sendConnHeadCache = *sendConnHeadPtr;
        if (checkAbort(spins, 1)) break;
      }
      if (sendConnFifoPtr) {
        int size = ((sendConnHead & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK) ? stepLines*sizeof(union ncclLLFifoLine) : nbytes;
        sendConnFifoPtr[sendConnHead%NCCL_STEPS] = size;
      }
      sendConnHead += 1;
    }
    barrier();
  }
```

</details>

<a id="f0166"></a>

## F0166：incRecv

用途：递增指定接收邻居的协议 step，下一轮会选择后续槽位和新的期望 flag。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>recvStep[i] += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    recvStep[i] += 1;
  }
```

</details>

<a id="f0167"></a>

## F0167：postRecv

用途：在数据已消费后发布接收完成进度，通知生产者可以回收相应循环槽位。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |
| 2 | 0 / 条件判断 | <code>(recvConnHeadPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>*recvConnHeadPtr = recvConnHead += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    barrier();
    if (recvConnHeadPtr) *recvConnHeadPtr = recvConnHead += 1;
  }
```

</details>

<a id="f0168"></a>

## F0168：incSend

用途：推进发送方协议 step，LL 标志回绕前会清理未使用行，防止旧 flag 被误判为新数据。

- 当step命中清理掩码时，把未覆盖行的flag也清理/重写，防止序号回绕后误识别旧数据。
- 随后才递增该邻居发送step；普通行内就绪判断与缓冲空间head流控仍独立存在。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>((sendStep[i] & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 循环 | <code>初始 int o = offset;；继续条件 o&lt;stepLines；每轮更新 o+=nthreads</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 3 / 调用 / 计算 / 写入 | <code>storeLL(sendPtr(i)+o, 0, sendFlag(i));</code> | 把有效数据与两个进度 flag 编码进 LL 行并写出，以行级就绪取代整片等待。 |
| 5 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>sendStep[i]++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // LL Cleanup : write all flags in the slice to make sure we don't have
    // data corruption when flag loops over.
    if ((sendStep[i] & NCCL_LL_CLEAN_MASK) == NCCL_LL_CLEAN_MASK) {
      for (int o = offset; o<stepLines; o+=nthreads) storeLL(sendPtr(i)+o, 0, sendFlag(i));
    }
    sendStep[i]++;
  }
```

</details>

<a id="f0169"></a>

## F0169：readLL

用途：反复读取一个 LL 行直到两个 flag 都匹配本轮序号，再取出其中 8 字节有效数据。

- 每次读整行的data/flag视图，只有flag1和flag2同时等于当前期望序号才拼出有效64位数据。
- 中途只看到一个新flag不够；等待中周期检查abort，不把部分新旧混合行作为正确载荷。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union ncclLLFifoLine* src = recvPtr(i) + offset;</code> | 以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。 |
| 2 | 0 / 准备局部状态 | <code>uint32_t flag = recvFlag(i);</code> | 把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。 |
| 3 | 0 / 准备局部状态 | <code>uint32_t data1, flag1, data2, flag2;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 先执行一次的循环 | <code>初始 ；继续条件 ((flag1 != flag) &#124;&#124; (flag2 != flag))；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(data1), "=r"(flag1), "=r"(data2), "=r"(flag2) : "l"(&src-&gt;i4));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 1 / 条件判断 | <code>(checkAbort(spins, 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 准备局部状态 | <code>uint64_t val64 = data1 + (((uint64_t)data2) &lt;&lt; 32);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 函数返回 | <code>return val64;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union ncclLLFifoLine* src = recvPtr(i) + offset;
    uint32_t flag = recvFlag(i);
    uint32_t data1, flag1, data2, flag2;
    int spins = 0;
    do {
      asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(data1), "=r"(flag1), "=r"(data2), "=r"(flag2) : "l"(&src->i4));
      if (checkAbort(spins, 0)) break;
    } while ((flag1 != flag) || (flag2 != flag));
    uint64_t val64 = data1 + (((uint64_t)data2) << 32);
    return val64;
  }
```

</details>

<a id="f0170"></a>

## F0170：readLLBeginAll

用途：先向多个接收邻居发出 LL 行读取，把访存提前以便后续 Finish 隐藏加载延迟。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 0 / 循环 | <code>初始 int i=BeginIx;；继续条件 i &lt; MaxRecv；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(i &lt; fan.nrecv())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 准备局部状态 | <code>union ncclLLFifoLine* src = recvPtr(i) + offset;</code> | 以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(line[i].data1), "=r"(line[i].flag1), "=r"(line[i].data2), "=r"(line[i].flag2) : "l"(&src-&gt;i4));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    #pragma unroll
    for (int i=BeginIx; i < MaxRecv; i++) {
      if (i < fan.nrecv()) {
        union ncclLLFifoLine* src = recvPtr(i) + offset;
        asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(line[i].data1), "=r"(line[i].flag1), "=r"(line[i].data2), "=r"(line[i].flag2) : "l"(&src->i4));
      }
    }
  }
```

</details>

<a id="f0171"></a>

## F0171：readLLFinish

用途：检查预加载 LL 行的 flag，不匹配时继续读，直到能够安全提取数据或中止。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union ncclLLFifoLine* src = recvPtr(i) + offset;</code> | 以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。 |
| 2 | 0 / 准备局部状态 | <code>uint32_t flag = recvFlag(i);</code> | 把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。 |
| 3 | 0 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 ；继续条件 (line[i].flag1 != flag &#124;&#124; line[i].flag2 != flag)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(line[i].data1), "=r"(line[i].flag1), "=r"(line[i].data2), "=r"(line[i].flag2) : "l"(&src-&gt;i4));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 1 / 条件判断 | <code>(checkAbort(spins, 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 准备局部状态 | <code>uint64_t val64 = line[i].data1 + (((uint64_t)line[i].data2) &lt;&lt; 32);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 函数返回 | <code>return val64;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union ncclLLFifoLine* src = recvPtr(i) + offset;
    uint32_t flag = recvFlag(i);
    int spins = 0;
    while (line[i].flag1 != flag || line[i].flag2 != flag) {
      asm("ld.volatile.global.v4.u32 {%0,%1,%2,%3}, [%4];" : "=r"(line[i].data1), "=r"(line[i].flag1), "=r"(line[i].data2), "=r"(line[i].flag2) : "l"(&src->i4));
      if (checkAbort(spins, 0)) break;
    }
    uint64_t val64 = line[i].data1 + (((uint64_t)line[i].data2) << 32);
    return val64;
  }
```

</details>

<a id="f0172"></a>

## F0172：storeLL

用途：把有效数据与两个进度 flag 编码进 LL 行并写出，以行级就绪取代整片等待。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("st.volatile.global.v4.u32 [%0], {%1,%2,%3,%4};" :: "l"(&dst-&gt;i4), "r"((uint32_t)val), "r"(flag), "r"((uint32_t)(val &gt;&gt; 32)), "r"(flag));</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    asm volatile("st.volatile.global.v4.u32 [%0], {%1,%2,%3,%4};" :: "l"(&dst->i4), "r"((uint32_t)val), "r"(flag), "r"((uint32_t)(val >> 32)), "r"(flag));
  }
```

</details>

<a id="f0173"></a>

## F0173：load

用途：把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>union { U elt; uint16_t u2; uint32_t u4; uint64_t u8; }</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 条件判断 | <code>(sizeof(U) == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>asm("ld.volatile.global.b8 %0,[%1];" : "=r"(u4) : "l"(src));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 6 | 2 / 条件判断 | <code>(sizeof(U) == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>asm("ld.volatile.global.b16 %0,[%1];" : "=h"(u2) : "l"(src));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 10 | 4 / 条件判断 | <code>(sizeof(U) == 4)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 6 / 调用 / 计算 / 写入 | <code>asm("ld.volatile.global.b32 %0,[%1];" : "=r"(u4) : "l"(src));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 14 | 6 / 调用 / 计算 / 写入 | <code>asm("ld.volatile.global.b64 %0,[%1];" : "=l"(u8) : "l"(src));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return elt;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      U elt;
      uint16_t u2;
      uint32_t u4;
      uint64_t u8;
    };
    if(sizeof(U) == 1)
      asm("ld.volatile.global.b8 %0,[%1];" : "=r"(u4) : "l"(src));
    else if(sizeof(U) == 2)
      asm("ld.volatile.global.b16 %0,[%1];" : "=h"(u2) : "l"(src));
    else if(sizeof(U) == 4)
      asm("ld.volatile.global.b32 %0,[%1];" : "=r"(u4) : "l"(src));
    else
      asm("ld.volatile.global.b64 %0,[%1];" : "=l"(u8) : "l"(src));
    return elt;
  }
```

</details>

<a id="f0174"></a>

## F0174：store

用途：把协议寄存器值按用户元素布局写回目标，处理类型宽度和不足整包的尾部。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>union { U elt; uint16_t u2; uint32_t u4; uint64_t u8; }</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>elt = val;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 条件判断 | <code>(sizeof(U) == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>asm("st.volatile.global.b8 [%0],%1;" :: "l"(dst), "r"(u4));</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 6 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 7 | 2 / 条件判断 | <code>(sizeof(U) == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>asm("st.volatile.global.b16 [%0],%1;" :: "l"(dst), "h"(u2));</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 10 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 4 / 条件判断 | <code>(sizeof(U) == 4)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 6 / 调用 / 计算 / 写入 | <code>asm("st.volatile.global.b32 [%0],%1;" :: "l"(dst), "r"(u4));</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 14 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 6 / 调用 / 计算 / 写入 | <code>asm("st.volatile.global.b64 [%0],%1;" :: "l"(dst), "l"(u8));</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      U elt;
      uint16_t u2;
      uint32_t u4;
      uint64_t u8;
    };
    elt = val;
    if(sizeof(U) == 1)
      asm("st.volatile.global.b8 [%0],%1;" :: "l"(dst), "r"(u4));
    else if(sizeof(U) == 2)
      asm("st.volatile.global.b16 [%0],%1;" :: "l"(dst), "h"(u2));
    else if(sizeof(U) == 4)
      asm("st.volatile.global.b32 [%0],%1;" :: "l"(dst), "r"(u4));
    else
      asm("st.volatile.global.b64 [%0],%1;" :: "l"(dst), "l"(u8));
  }
```

</details>

<a id="f0175"></a>

## F0175：loadBegin

用途：把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(sizeof(T) &lt;= 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>misalign = reinterpret_cast&lt;uintptr_t&gt;(src)%4;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 2 / 准备局部状态 | <code>uint32_t *p = reinterpret_cast&lt;uint32_t*&gt;(reinterpret_cast&lt;uintptr_t&gt;(src) & -uintptr_t(4));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>u4[0] = load(p+0);</code> | 把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>u4[1] = misalign + eltN*sizeof(T) &gt; 4 ? load(p+1) : 0;</code> | 把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>u4[sizeof(T) &lt;= 2 ? 2 : 0] = misalign + eltN*sizeof(T) &gt; 8 ? load(p+2) : 0;</code> | 把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。 |
| 8 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 9 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 10 | 2 / 循环 | <code>初始 int i=0;；继续条件 i &lt; EltPerLine；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 3 / 条件判断 | <code>(i==0 &#124;&#124; i &lt; eltN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 5 / 调用 / 计算 / 写入 | <code>elt[i] = load(src + i);</code> | 把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。 |
| 14 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
      if (sizeof(T) <= 2) {
        misalign = reinterpret_cast<uintptr_t>(src)%4;
        uint32_t *p = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(src) & -uintptr_t(4));
        u4[0] = load(p+0);
        u4[1] = misalign + eltN*sizeof(T) > 4 ? load(p+1) : 0;
        // u4[2] would be simpler, but that throws warnings on some compilers
        u4[sizeof(T) <= 2 ? 2 : 0] = misalign + eltN*sizeof(T) > 8 ? load(p+2) : 0;
      }
      else {
        #pragma unroll
        for(int i=0; i < EltPerLine; i++) {
          if(i==0 || i < eltN)
            elt[i] = load(src + i);
        }
      }
    }
```

</details>

<a id="f0176"></a>

## F0176：loadFinish

用途：把用户元素加载到协议寄存器表示；Begin 提前发起加载，Finish 完成拼接/整理以减少依赖停顿。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(sizeof(T) &lt;= 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>u4[0] = __funnelshift_r(u4[0], u4[1], 8*misalign);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>u4[1] = __funnelshift_r(u4[1], u4[sizeof(T) &lt;= 2 ? 2 : 0], 8*misalign);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return u8;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
      if (sizeof(T) <= 2) {
        u4[0] = __funnelshift_r(u4[0], u4[1], 8*misalign);
        // u4[2] would be simpler, but that throws warnings on some compilers
        u4[1] = __funnelshift_r(u4[1], u4[sizeof(T) <= 2 ? 2 : 0], 8*misalign);
      }
      return u8;
    }
```

</details>

<a id="f0177"></a>

## F0177：storeData

用途：把协议寄存器值按用户元素布局写回目标，处理类型宽度和不足整包的尾部。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>union { uint64_t u8; T elt[EltPerLine]; }</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u8 = val;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; EltPerLine；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 条件判断 | <code>(i==0 &#124;&#124; i &lt; eltN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>dst[i] = elt[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      uint64_t u8;
      T elt[EltPerLine];
    };
    u8 = val;
    #pragma unroll
    for(int i=0; i < EltPerLine; i++) {
      if (i==0 || i < eltN)
        //store(dst+i, elt[i]);
        dst[i] = elt[i];
    }
  }
```

</details>

<a id="f0178"></a>

## F0178：LLGenericOp

用途：依据编译期收发/输入输出组合，按 LL 行等待、融合归约、写下游并推进 step。

- SEND先waitSend获取可写空间并考虑标记回绕清理；每线程按LL行跨步处理元素。
- Begin本地加载与预读其他邻居在前，首邻居readLL等待两个flag，再完成本地预处理和多源归约。
- 可选postOp后把data/flag发下游，并按DST保留本地；最后推进邻居step并postRecv。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>constexpr int SRC = SrcBuf != -1 ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>constexpr int DST = DstBuf != -1 ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>T *srcElts = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>T *dstElts = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>nelem = nelem &lt; 0 ? 0 : nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 条件判断 | <code>(SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>waitSend(divUp(nelem, EltPerLine)*sizeof(ncclLLFifoLine));</code> | 等待发送缓冲仍有可复用槽位，并在存在 Proxy 时填写本轮物理字节数，防止覆盖 NIC/对端仍使用的数据。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>nelem -= tid*EltPerLine;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>srcElts += tid*EltPerLine;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>dstElts += tid*EltPerLine;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 准备局部状态 | <code>int offset = tid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>int eltPerTrip = nthreads*EltPerLine;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 循环 | <code>初始 ；继续条件 (nelem &gt; 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 15 | 1 / 准备局部状态 | <code>int eltInLine = EltPerLine &lt; nelem ? EltPerLine : nelem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 1 / 准备局部状态 | <code>DataLoader dl;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 1 / 准备局部状态 | <code>ncclLLFifoLine line[MaxRecv];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 1 / 准备局部状态 | <code>uint64_t data, peerData;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 1 / 条件判断 | <code>(SRC)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>dl.loadBegin(srcElts, eltInLine);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 3 / 调用 / 计算 / 写入 | <code>srcElts += eltPerTrip;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 1 / 条件判断 | <code>(RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 3 / 调用 / 计算 / 写入 | <code>readLLBeginAll&lt;1&gt;(offset, line);</code> | 先向多个接收邻居发出 LL 行读取，把访存提前以便后续 Finish 隐藏加载延迟。 |
| 26 | 3 / 调用 / 计算 / 写入 | <code>peerData = readLL(offset, 0);</code> | 反复读取一个 LL 行直到两个 flag 都匹配本轮序号，再取出其中 8 字节有效数据。 |
| 27 | 1 / 条件判断 | <code>(SRC)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 3 / 调用 / 计算 / 写入 | <code>data = dl.loadFinish();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 3 / 条件判断 | <code>(SrcBuf == Input)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 5 / 调用 / 计算 / 写入 | <code>data = MULTI&lt;RedOp, T&gt;().preOp(redOp, data);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 1 / 条件判断 | <code>(RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>data = !SRC ? peerData : MULTI&lt;RedOp,T&gt;()(redOp, peerData, data);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 3 / 编译期分支/指令 | <code>#pragma unroll MaxRecv</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 37 | 3 / 循环 | <code>初始 int i=1;；继续条件 i &lt; MaxRecv && i &lt; fan.nrecv()；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>peerData = readLLFinish(offset, line, i);</code> | 检查预加载 LL 行的 flag，不匹配时继续读，直到能够安全提取数据或中止。 |
| 39 | 4 / 调用 / 计算 / 写入 | <code>data = MULTI&lt;RedOp,T&gt;()(redOp, peerData, data);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 41 | 1 / 条件判断 | <code>(postOp)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 3 / 调用 / 计算 / 写入 | <code>data = MULTI&lt;RedOp, T&gt;().postOp(redOp, data);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 1 / 条件判断 | <code>(SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 3 / 循环 | <code>初始 int i=1;；继续条件 i &lt; MaxSend && i &lt; fan.nsend()；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 47 | 4 / 调用 / 计算 / 写入 | <code>storeLL(sendPtr(i)+offset, data, sendFlag(i));</code> | 把有效数据与两个进度 flag 编码进 LL 行并写出，以行级就绪取代整片等待。 |
| 48 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 49 | 3 / 调用 / 计算 / 写入 | <code>storeLL(sendPtr(0)+offset, data, sendFlag(0));</code> | 把有效数据与两个进度 flag 编码进 LL 行并写出，以行级就绪取代整片等待。 |
| 50 | 1 / 条件判断 | <code>(DST)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 3 / 调用 / 计算 / 写入 | <code>storeData(dstElts, data, eltInLine);</code> | 把协议寄存器值按用户元素布局写回目标，处理类型宽度和不足整包的尾部。 |
| 53 | 3 / 调用 / 计算 / 写入 | <code>dstElts += eltPerTrip;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 1 / 调用 / 计算 / 写入 | <code>nelem -= eltPerTrip;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 1 / 调用 / 计算 / 写入 | <code>offset += nthreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 57 | 0 / 条件判断 | <code>(RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 58 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 59 | 2 / 循环 | <code>初始 int i=0;；继续条件 i &lt; MaxRecv；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 60 | 3 / 调用 / 计算 / 写入 | <code>incRecv(i);</code> | 递增指定接收邻居的协议 step，下一轮会选择后续槽位和新的期望 flag。 |
| 61 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 62 | 2 / 调用 / 计算 / 写入 | <code>postRecv();</code> | 在数据已消费后发布接收完成进度，通知生产者可以回收相应循环槽位。 |
| 63 | 0 / 条件判断 | <code>(SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 2 / 循环 | <code>初始 int i=1;；继续条件 i &lt; MaxSend && i &lt; fan.nsend()；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 66 | 3 / 调用 / 计算 / 写入 | <code>incSend(i, offset);</code> | 推进发送方协议 step，LL 标志回绕前会清理未使用行，防止旧 flag 被误判为新数据。 |
| 67 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 68 | 2 / 调用 / 计算 / 写入 | <code>incSend(0, offset);</code> | 推进发送方协议 step，LL 标志回绕前会清理未使用行，防止旧 flag 被误判为新数据。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    constexpr int SRC = SrcBuf != -1 ? 1 : 0;
    constexpr int DST = DstBuf != -1 ? 1 : 0;
    T *srcElts = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx;
    T *dstElts = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx;

    // Always waitSend in case of cleanup
    nelem = nelem < 0 ? 0 : nelem;
    if (SEND) waitSend(divUp(nelem, EltPerLine)*sizeof(ncclLLFifoLine));

    nelem -= tid*EltPerLine;
    srcElts += tid*EltPerLine;
    dstElts += tid*EltPerLine;
    int offset = tid;
    int eltPerTrip = nthreads*EltPerLine;
    while (nelem > 0) {
      int eltInLine = EltPerLine < nelem ? EltPerLine : nelem;

      DataLoader dl;
      ncclLLFifoLine line[MaxRecv];
      uint64_t data, peerData;
      if (SRC) {
        dl.loadBegin(srcElts, eltInLine);
        srcElts += eltPerTrip;
      }
      if (RECV) {
        readLLBeginAll<1>(offset, line);
        peerData = readLL(offset, 0);
      }
      if (SRC) {
        data = dl.loadFinish();
        if (SrcBuf == Input) data = MULTI<RedOp, T>().preOp(redOp, data);
      }
      if (RECV) {
        data = !SRC ? peerData : MULTI<RedOp,T>()(redOp, peerData, data);
        #pragma unroll MaxRecv
        for (int i=1; i < MaxRecv && i < fan.nrecv(); i++) {
          peerData = readLLFinish(offset, line, i);
          data = MULTI<RedOp,T>()(redOp, peerData, data);
        }
      }

      if (postOp) data = MULTI<RedOp, T>().postOp(redOp, data);

      // Send : inter-node, then intra-node, then local
      if (SEND) {
        for (int i=1; i < MaxSend && i < fan.nsend(); i++)
          storeLL(sendPtr(i)+offset, data, sendFlag(i));
        storeLL(sendPtr(0)+offset, data, sendFlag(0));
      }
      if (DST) {
        storeData(dstElts, data, eltInLine);
        dstElts += eltPerTrip;
      }
      nelem -= eltPerTrip;
      offset += nthreads;
    }

    if (RECV) {
      for (int i=0; i < MaxRecv; i++) incRecv(i);
      postRecv();
    }
    if (SEND) {
      for (int i=1; i < MaxSend && i < fan.nsend(); i++)
        incSend(i, offset);
      incSend(0, offset);
    }
  }
```

</details>

<a id="f0179"></a>

## F0179：loadRecvConn

用途：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>recvBuff[i] = (union ncclLLFifoLine*)conn-&gt;buffs[NCCL_PROTO_LL];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>recvStep[i] = conn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 3 | 0 / 条件判断 | <code>(wid == i)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>recvConn = conn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    recvBuff[i] = (union ncclLLFifoLine*)conn->buffs[NCCL_PROTO_LL];
    recvStep[i] = conn->step;
    if (wid == i) recvConn = conn;
  }
```

</details>

<a id="f0180"></a>

## F0180：loadRecvSync

用途：为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(tid &gt;= nthreads-WARP_SIZE && wid &lt; fan.nrecv())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>recvConnHeadPtr = recvConn-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>recvConnHead = recvConn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (tid >= nthreads-WARP_SIZE && wid < fan.nrecv()) {
      recvConnHeadPtr = recvConn->head;
      recvConnHead = recvConn->step;
    }
  }
```

</details>

<a id="f0181"></a>

## F0181：loadSendConn

用途：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>sendBuff[i] = (union ncclLLFifoLine*)conn-&gt;buffs[NCCL_PROTO_LL];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>sendStep[i] = conn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 3 | 0 / 条件判断 | <code>(wid == i)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>sendConn = conn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    sendBuff[i] = (union ncclLLFifoLine*)conn->buffs[NCCL_PROTO_LL];
    sendStep[i] = conn->step;
    if (wid == i) sendConn = conn;
  }
```

</details>

<a id="f0182"></a>

## F0182：loadSendSync

用途：为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(tid &lt; fan.nsend())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>sendConnHeadPtr = sendConn-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>sendConnHeadCache = *sendConnHeadPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>sendConnHead = sendConn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>sendConnFifoPtr = sendConn-&gt;sizesFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (tid < fan.nsend()) {
      sendConnHeadPtr = sendConn->head;
      sendConnHeadCache = *sendConnHeadPtr;
      sendConnHead = sendConn->step;
      sendConnFifoPtr = sendConn->sizesFifo;
    }
  }
```

</details>

<a id="f0183"></a>

## F0183：Primitives

用途：为一个线程组建立原语上下文：实际邻居度、协议缓冲、线程角色、同步编号及用户数据指针。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>auto *channel = &ncclShmem.channel;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nrecv=0, nsend=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 ；继续条件 (nrecv &lt; MaxRecv && recvPeers[nrecv] &gt;= 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>loadRecvConn(&channel-&gt;devPeers[recvPeers[nrecv]].recv-&gt;conn, nrecv);</code> | 从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>nrecv++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 循环 | <code>初始 ；继续条件 (nsend &lt; MaxSend && sendPeers[nsend] &gt;= 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>loadSendConn(&channel-&gt;devPeers[sendPeers[nsend]].send-&gt;conn, nsend);</code> | 从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>nsend++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>this-&gt;fan = Fan(nrecv, nsend);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>loadRecvSync();</code> | 为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>loadSendSync();</code> | 为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>setDataPtrs(inputBuf, outputBuf);</code> | 更新本原语的本地输入和输出缓冲指针；Simple 的 direct 路径还需完成对应地址交换。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{

    auto *channel = &ncclShmem.channel;
    // If we are going to support oneshot collNet + LL, then we would need to add connector index here
    int nrecv=0, nsend=0;
    while (nrecv < MaxRecv && recvPeers[nrecv] >= 0) {
      loadRecvConn(&channel->devPeers[recvPeers[nrecv]].recv->conn, nrecv);
      nrecv++;
    }
    while (nsend < MaxSend && sendPeers[nsend] >= 0) {
      loadSendConn(&channel->devPeers[sendPeers[nsend]].send->conn, nsend);
      nsend++;
    }
    this->fan = Fan(nrecv, nsend);
    loadRecvSync();
    loadSendSync();
    setDataPtrs(inputBuf, outputBuf);
  }
```

</details>

<a id="f0184"></a>

## F0184：~Primitives

用途：把本次推进后的 step 写回长期连接并按协议要求同步，后续操作继续复用同一连接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(tid &gt;= nthreads-WARP_SIZE && wid &lt; fan.nrecv())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>recvConn-&gt;step = recvConnHead;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 0 / 条件判断 | <code>(tid &lt; fan.nsend())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>sendConn-&gt;step = sendConnHead;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // Save steps for the next operation
    if (tid >= nthreads-WARP_SIZE && wid < fan.nrecv())
      recvConn->step = recvConnHead;
    if (tid < fan.nsend())
      sendConn->step = sendConnHead;
    // Ensure all steps written back
    barrier();
  }
```

</details>

<a id="f0185"></a>

## F0185：setDataPtrs

用途：更新本原语的本地输入和输出缓冲指针；Simple 的 direct 路径还需完成对应地址交换。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>userBufs[Input] = (T*)inputBuf;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>userBufs[Output] = (T*)outputBuf;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    userBufs[Input] = (T*)inputBuf;
    userBufs[Output] = (T*)outputBuf;
  }
```

</details>

<a id="f0186"></a>

## F0186：moveDataPtrs

用途：按元素偏移移动本原语的用户输入/输出指针，复用连接与协议状态处理另一段数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>userBufs[Input] += delta;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>userBufs[Output] += delta;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    userBufs[Input] += delta;
    userBufs[Output] += delta;
  }
```

</details>

<a id="f0187"></a>

## F0187：send

用途：从本地输入读取并发送到原语的发送邻居，不要求写本地输出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return LLGenericOp&lt;0, 1, Input, -1&gt;(inpIx, -1, eltN, false);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return LLGenericOp<0, 1, Input, -1>(inpIx, -1, eltN, false);
  }
```

</details>

<a id="f0188"></a>

## F0188：sendFromOutput

用途：从已生成的本地输出读数据并发送，避免错误地重新读取原始输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return LLGenericOp&lt;0, 1, Output, -1&gt;(outIx, -1, eltN, false);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return LLGenericOp<0, 1, Output, -1>(outIx, -1, eltN, false);
  }
```

</details>

<a id="f0189"></a>

## F0189：recv

用途：接收上游数据并写本地输出，本动作不加上本地输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return LLGenericOp&lt;1, 0, -1, Output&gt;(-1, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return LLGenericOp<1, 0, -1, Output>(-1, outIx, eltN, postOp);
  }
```

</details>

<a id="f0190"></a>

## F0190：recvReduceSend

用途：把上游部分结果与本地输入归约后发下游，不要求保留本地最终输出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return LLGenericOp&lt;1, 1, Input, -1&gt;(inpIx, -1, eltN, false);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return LLGenericOp<1, 1, Input, -1>(inpIx, -1, eltN, false);
  }
```

</details>

<a id="f0191"></a>

## F0191：recvReduceCopy

用途：把接收数据与本地输入归约后写本地输出，可在结果完成时执行 postOp。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return LLGenericOp&lt;1, 0, Input, Output&gt;(inpIx, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return LLGenericOp<1, 0, Input, Output>(inpIx, outIx, eltN, postOp);
  }
```

</details>

<a id="f0192"></a>

## F0192：copySend

用途：把本地输入同时写本地输出并发下游，把本地保存和转发合并一次数据加载。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return LLGenericOp&lt;0, 1, Input, Output&gt;(inpIx, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return LLGenericOp<0, 1, Input, Output>(inpIx, outIx, eltN, postOp);
  }
```

</details>

<a id="f0193"></a>

## F0193：recvCopySend

用途：把已接收结果同时保存本地输出并转发下游，不再加入本地输入做归约。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return LLGenericOp&lt;1, 1, -1, Output&gt;(-1, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return LLGenericOp<1, 1, -1, Output>(-1, outIx, eltN, postOp);
  }
```

</details>

<a id="f0194"></a>

## F0194：recvReduceCopySend

用途：把接收与本地输入融合归约，同时保存输出并转发，衔接归约与结果传播阶段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return LLGenericOp&lt;1, 1, Input, Output&gt;(inpIx, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return LLGenericOp<1, 1, Input, Output>(inpIx, outIx, eltN, postOp);
  }
```

</details>

