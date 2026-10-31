# src/collectives/device/prims_simple.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0227"></a>

## F0227：barrier

用途：同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(nthreads == WARP_SIZE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>__syncwarp();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>asm volatile("bar.sync %0, %1;" :: "r"(15-group), "r"(nthreads));</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>flags &#124;= ThreadsSynced;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (nthreads == WARP_SIZE)
      __syncwarp();
    else
      asm volatile("bar.sync %0, %1;" :: "r"(15-group), "r"(nthreads));
    flags |= ThreadsSynced;
  }
```

</details>

<a id="f0228"></a>

## F0228：subBarrier

用途：只同步负责数据搬运的 worker 子组，配合同步 warp 让复制与进度发布流水重叠。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(nworkers == nthreads)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>asm volatile("bar.sync %0, %1;" :: "r"(8-group), "r"(nworkers));</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (nworkers == nthreads)
      barrier();
    else
      asm volatile("bar.sync %0, %1;" :: "r"(8-group), "r"(nworkers));
  }
```

</details>

<a id="f0229"></a>

## F0229：checkAbort

用途：自旋达到检查间隔时读取 communicator 中止标志，使异常情况下的协议等待可以退出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>spins++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(!(flags & Aborted) && spins == NCCL_SPINS_BEFORE_CHECK_ABORT)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>flags &#124;= *ncclShmem.comm.abortFlag ? Aborted : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>spins = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return flags & Aborted;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    spins++;
    if (!(flags & Aborted) && spins == NCCL_SPINS_BEFORE_CHECK_ABORT) {
      flags |= *ncclShmem.comm.abortFlag ? Aborted : 0;
      spins = 0;
    }
    return flags & Aborted;
  }
```

</details>

<a id="f0230"></a>

## F0230：waitPeer

用途：发送侧等空槽、接收侧等数据，并按 direct/shared/固定 FIFO 条件装配本 slice 的实际指针。

- 先辨认当前线程负责发送还是接收，并按具体direct谓词决定可省去哪些等待。
- 发送等head+8足够覆盖目标step，接收等tail达到目标step；等待中刷新进度缓存并查abort。
- 随后按shared偏移、直接用户缓冲或固定FIFO槽装配实际srcs/dsts，Wait角色更新自己的寄存器step。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const bool isSendNotRecv = (Send && Recv) ? (flags & RoleWaitSend) : Send;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const bool noRecvWait = DirectRecv && Src && (flags & DirectRead);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const bool noSendWait = DirectSend && (flags & (DirectRead&#124;DirectWrite));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(((flags & (Recv*RoleWaitRecv)) && !noRecvWait) &#124;&#124; ((flags & (Send*RoleWaitSend)) && !noSendWait))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 循环 | <code>初始 ；继续条件 (connStepCache + (isSendNotRecv ? NCCL_STEPS : 0) &lt; step + StepPerSlice)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>connStepCache = *connStepPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 条件判断 | <code>(checkAbort(spins))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 12 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 条件判断 | <code>(flags & (Recv*RoleWaitRecv &#124; Send*RoleWaitSend))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 条件判断 | <code>(isSendNotRecv && (flags & SizesFifoEnabled))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>connSizesFifoPtr[step%NCCL_STEPS] = nelts*sizeof(T);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 18 | 2 / 准备局部状态 | <code>void **ptrs = isSendNotRecv ? (ncclShmem.groups[group].dsts + Dst) : (ncclShmem.groups[group].srcs + Src);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 2 / 条件判断 | <code>(flags & OffsFifoEnabled)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>ptrs[index] = connEltsFifo + loadInt(connOffsFifoPtr + (step%NCCL_STEPS))/sizeof(T);</code> | 用 volatile 全局加载读取协议偏移/控制值，防止编译器把动态变化值长久缓存在寄存器。 |
| 22 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 4 / 条件判断 | <code>(isSendNotRecv && DirectSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 6 / 条件判断 | <code>(flags & DirectWrite)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 8 / 调用 / 计算 / 写入 | <code>ptrs[index] = directBuff + remoteIx + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 29 | 8 / 条件判断 | <code>(flags & DirectRead)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 10 / 调用 / 计算 / 写入 | <code>ptrs[index] = nullptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 9 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 33 | 10 / 调用 / 计算 / 写入 | <code>ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 34 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 35 | 6 / 条件判断 | <code>(!isSendNotRecv && DirectRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 8 / 条件判断 | <code>(flags & DirectRead)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 10 / 调用 / 计算 / 写入 | <code>ptrs[index] = directBuff + remoteIx + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 9 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 41 | 10 / 条件判断 | <code>(flags & DirectWrite)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 12 / 调用 / 计算 / 写入 | <code>ptrs[index] = directBuff + dstIx + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 11 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 45 | 12 / 调用 / 计算 / 写入 | <code>ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 46 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 47 | 8 / 调用 / 计算 / 写入 | <code>ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 48 | 2 / 调用 / 计算 / 写入 | <code>step += StepPerSlice;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const bool isSendNotRecv = (Send && Recv) ? (flags & RoleWaitSend) : Send;
    const bool noRecvWait = DirectRecv && Src && (flags & DirectRead);        // no wait when directly reading from remote input
    const bool noSendWait = DirectSend && (flags & (DirectRead|DirectWrite)); // no wait in empty send (e.g. directScatter) or direct remote write
    if (((flags & (Recv*RoleWaitRecv)) && !noRecvWait) ||
        ((flags & (Send*RoleWaitSend)) && !noSendWait)) {
      int spins = 0;
/* 中文精读：把 waitPeer 的不等式读成人话
 * 发送等待：消费者 head + 8 >= 本次结束 step，表示环形数据槽尚未被覆盖，可以写。
 * 接收等待：生产者 tail >= 本次结束 step，表示所需数据已经发布，可以读。
 * connStepCache 先用寄存器缓存，只有不足时才反复读取共享进度，减少热路径访存。
 * DirectRead/Write 在满足协议契约时能跳过部分等待，但仍要遵守指针交换与生命周期约束。
 */
      while (connStepCache + (isSendNotRecv ? NCCL_STEPS : 0) < step + StepPerSlice) {
        connStepCache = *connStepPtr;
        if (checkAbort(spins)) break;
        //if (spins == 0) printf("r=%d b=%d t=%d SPUN OUT got=%d want=%d\n", ncclShmem.comm.rank, blockIdx.x, threadIdx.x, int(connStepCache + (isSendNotRecv ? NCCL_STEPS : 0)), int(step+StepPerSlice));
      }
    }

    if (flags & (Recv*RoleWaitRecv | Send*RoleWaitSend)) {
      if (isSendNotRecv && (flags & SizesFifoEnabled))
        connSizesFifoPtr[step%NCCL_STEPS] = nelts*sizeof(T);

      void **ptrs = isSendNotRecv ? (ncclShmem.groups[group].dsts + Dst)
                                  : (ncclShmem.groups[group].srcs + Src);
      if (flags & OffsFifoEnabled)
        ptrs[index] = connEltsFifo + loadInt(connOffsFifoPtr + (step%NCCL_STEPS))/sizeof(T);
      else if (isSendNotRecv && DirectSend) {
        if (flags & DirectWrite) {
          ptrs[index] = directBuff + remoteIx + offset;
        } else if (flags & DirectRead) {  // empty send
          ptrs[index] = nullptr;
        } else {
          ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
        }
      } else if (!isSendNotRecv && DirectRecv) {
        if (flags & DirectRead) {
          ptrs[index] = directBuff + remoteIx + offset;
        } else if (flags & DirectWrite) {
          ptrs[index] = directBuff + dstIx + offset;  // send to next from my output buffer
        } else {
          ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
        }
      }
      else {
        ptrs[index] = connEltsFifo + (step%NCCL_STEPS)*stepSize;
      }
      step += StepPerSlice;
    }
  }
```

</details>

<a id="f0231"></a>

## F0231：postPeer

用途：发布本 slice 完成后的连接进度，让消费者能读或生产者能回收空间。

- 只有Post角色写共享进度；其本地step与Wait角色的寄存器副本独立，不是同一变量重复加步数。
- 调用者在此前完成对应barrier/fence；本函数只是推进并发布控制值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(flags & (Recv*RolePostRecv &#124; Send*RolePostSend))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>step += StepPerSlice;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>*connStepPtr = step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (flags & (Recv*RolePostRecv | Send*RolePostSend)) {
      step += StepPerSlice;
      *connStepPtr = step;
    }
  }
```

</details>

<a id="f0232"></a>

## F0232：genericOp

用途：Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。

- 把非正nelem归零后仍保留约定SlicePerChunk；空数据不等于可跳过协议。
- worker路径设置本地指针、waitPeer选邻居指针、subBarrier同步，再选择direct特例或融合搬运。
- 非worker在另一循环参加同一barrier；发布角色先满足fence，再postPeer推进连接。
- 两个循环共同保证有效与空slice的屏障次数一致；不是两个线程集合各自任意结束。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>constexpr int DirectRecv = 1 && Direct && DirectRecv1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>constexpr int DirectSend = 1 && Direct && DirectSend1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>constexpr int Src = SrcBuf != -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>constexpr int Dst = DstBuf != -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>nelem = nelem &lt; 0 ? 0 : nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 准备局部状态 | <code>int sliceSize = stepSize*StepPerSlice;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>sliceSize = max(divUp(nelem, 16*SlicePerChunk)*16, sliceSize/32);</code> | 计算整数向上取整的商，避免尾数不整除时少覆盖最后一块。 |
| 8 | 0 / 准备局部状态 | <code>int slice = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 条件判断 | <code>(tid &lt; nworkers && offset &lt; nelem)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &lt; 700</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 13 | 3 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 14 | 2 / 先执行一次的循环 | <code>初始 ；继续条件 (slice &lt; SlicePerChunk && offset &lt; nelem)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 15 | 3 / 调用 / 计算 / 写入 | <code>sliceSize = sliceSize &lt; nelem-offset ? sliceSize : nelem-offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 3 / 条件判断 | <code>(Src && (flags & (SrcBuf==Input ? RoleInput : RoleOutput)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 5 / 调用 / 计算 / 写入 | <code>ncclShmem.groups[group].srcs[0] = userBuff + srcIx + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 3 / 条件判断 | <code>(Dst && (flags & (DstBuf==Input ? RoleInput : RoleOutput)))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 5 / 调用 / 计算 / 写入 | <code>ncclShmem.groups[group].dsts[0] = userBuff + dstIx + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 3 / 调用 / 计算 / 写入 | <code>waitPeer&lt;DirectRecv, DirectSend, Recv, Send, Src, Dst&gt;(dstIx, remoteIx, offset, sliceSize);</code> | 发送侧等空槽、接收侧等数据，并按 direct/shared/固定 FIFO 条件装配本 slice 的实际指针。 |
| 23 | 3 / 调用 / 计算 / 写入 | <code>subBarrier();</code> | 只同步负责数据搬运的 worker 子组，配合同步 warp 让复制与进度发布流水重叠。 |
| 24 | 3 / 条件判断 | <code>(DirectRecv && ncclShmem.groups[group].srcs[0] == ncclShmem.groups[group].dsts[0])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 5 / 条件判断 | <code>(Send)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 7 / 调用 / 计算 / 写入 | <code>ReduceOrCopyMulti&lt;Unroll, RedOp, T, 1, 1, 1, (1-Send)+MaxSend, 0&gt; (tid, nworkers, nullptr, false, 1, (T const**)ncclShmem.groups[group].srcs, fan.nsend(), (T**)ncclShmem.groups[group].dsts+1, sliceSize);</code> | 统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。 |
| 29 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 30 | 5 / 条件判断 | <code>(DirectSend && !DirectRecv && SrcBuf != Input && ncclShmem.groups[group].dsts[Dst] == nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 7 / 调用 / 计算 / 写入 | <code>ReduceOrCopyMulti&lt;Unroll, RedOp, T, 1, 1, 1, 1, 0&gt; (tid, nworkers, ncclShmem.redOpArgs, postOp, Recv, (T const**)ncclShmem.groups[group].srcs, Dst, (T**)ncclShmem.groups[group].dsts, sliceSize);</code> | 统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。 |
| 33 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 34 | 7 / 准备局部状态 | <code>constexpr int PreOpN = SrcBuf != Input ? 0 : DirectRecv*MaxRecv == NCCL_MAX_DIRECT_ARITY ? (1+NCCL_MAX_DIRECT_ARITY) : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 7 / 调用 / 计算 / 写入 | <code>ReduceOrCopyMulti&lt;Unroll, RedOp, T, Recv+Src, Recv*MaxRecv+Src, Send+Dst, Send*MaxSend+Dst, PreOpN&gt; (tid, nworkers, ncclShmem.redOpArgs, postOp, Recv*fan.nrecv()+Src, (T const**)ncclShmem.groups[group].srcs, Send*fan.nsend()+Dst, (T**)ncclShmem.groups[group].dsts, sliceSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 3 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |
| 37 | 3 / 条件判断 | <code>(Send && (flags & RolePostSend) && index == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 5 / 调用 / 计算 / 写入 | <code>__threadfence_system();</code> | 约束先前数据写与随后进度发布的可见性顺序；这不是等待所有rank完成。 |
| 40 | 3 / 调用 / 计算 / 写入 | <code>__syncwarp();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 41 | 3 / 调用 / 计算 / 写入 | <code>postPeer&lt;Recv, Send&gt;();</code> | 发布本 slice 完成后的连接进度，让消费者能读或生产者能回收空间。 |
| 42 | 3 / 调用 / 计算 / 写入 | <code>offset += sliceSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 3 / 调用 / 计算 / 写入 | <code>slice += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 45 | 0 / 编译期分支/指令 | <code>#pragma unroll 1</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 46 | 0 / 循环 | <code>初始 ；继续条件 (slice &lt; SlicePerChunk)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 47 | 1 / 调用 / 计算 / 写入 | <code>sliceSize = sliceSize &lt; nelem-offset ? sliceSize : nelem-offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 1 / 调用 / 计算 / 写入 | <code>waitPeer&lt;DirectRecv, DirectSend, Recv, Send, Src, Dst&gt;(0, 0, 0, 0);</code> | 发送侧等空槽、接收侧等数据，并按 direct/shared/固定 FIFO 条件装配本 slice 的实际指针。 |
| 49 | 1 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |
| 50 | 1 / 条件判断 | <code>(Send && (flags & RolePostSend) && sliceSize &gt; 0 && index == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 3 / 调用 / 计算 / 写入 | <code>__threadfence_system();</code> | 约束先前数据写与随后进度发布的可见性顺序；这不是等待所有rank完成。 |
| 53 | 1 / 调用 / 计算 / 写入 | <code>__syncwarp();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 54 | 1 / 调用 / 计算 / 写入 | <code>postPeer&lt;Recv, Send&gt;();</code> | 发布本 slice 完成后的连接进度，让消费者能读或生产者能回收空间。 |
| 55 | 1 / 调用 / 计算 / 写入 | <code>offset += sliceSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 1 / 调用 / 计算 / 写入 | <code>slice += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    constexpr int DirectRecv = 1 && Direct && DirectRecv1;
    constexpr int DirectSend = 1 && Direct && DirectSend1;
    constexpr int Src = SrcBuf != -1;
    constexpr int Dst = DstBuf != -1;

/* 中文精读：尾块、空块与通信进度
 * 某 channel 的尾部 offset 可能已经超出 size，算法会传入非正 nelem；这里归一到 0。
 * 空数据仍须按固定 SlicePerChunk 推进同步和 step，不能简单 return 导致另一端永远等进度。
 * sliceSize 根据 nelem、片数与 step 容量调小，并保留最小搬运粒度，改善小尾块的延迟。
 * 它是有效元素数，乘 sizeof(T) 后才是 sizesFifo 中交给 Proxy 的字节数。
 */
    nelem = nelem < 0 ? 0 : nelem;
    int sliceSize = stepSize*StepPerSlice;
    sliceSize = max(divUp(nelem, 16*SlicePerChunk)*16, sliceSize/32);
    int slice = 0;
    int offset = 0;

    if (tid < nworkers && offset < nelem) {
      // Worker-only loop for non-empty slices. Non-workers and empty slices are
      // processed in the loop following this if block. The benefit of splitting
      // the loop like this is we pull two branches out of the critical path.
      // Using "number of branch insns (taken or not) encountered dynamically"
      // as the performance metric, then:
      //   perf_orig = 2*numslices
      //   perf_new = 2+numslices
      // So the new code and old code behave the same for numslices=2, and for
      // numslices>2 the new code is superior. And note that in the case
      // numslices=1, the loop is trivially unrollable (single iteration) so we
      // don't incur that that tail branch and we still have perf_new=2.
      //
      // ORIGINAL CODE:
      //   unrolled for(slices) {
      //     if(worker) { // This branch removed
      //       wait();
      //       subBarrier();
      //       if(slice not empty) // This branch removed
      //         ReduceCopyMulti();
      //     }
      //     barrier();
      //     post();
      //   } // Since we no longer unroll, new branch added here
      #if __CUDA_ARCH__ < 700
        // Yeah, so all that above don't matter a lick on older hardware.
        #pragma unroll SlicePerChunk
      #else
        #pragma unroll 1
      #endif
      do {
        sliceSize = sliceSize < nelem-offset ? sliceSize : nelem-offset;
        if (Src && (flags & (SrcBuf==Input ? RoleInput : RoleOutput)))
          ncclShmem.groups[group].srcs[0] = userBuff + srcIx + offset;
        if (Dst && (flags & (DstBuf==Input ? RoleInput : RoleOutput)))
          ncclShmem.groups[group].dsts[0] = userBuff + dstIx + offset;
        waitPeer<DirectRecv, DirectSend, Recv, Send, Src, Dst>(dstIx, remoteIx, offset, sliceSize);
        subBarrier();
        if (DirectRecv && ncclShmem.groups[group].srcs[0] == ncclShmem.groups[group].dsts[0]) {
          // We can only have one direct receive. Since srcs[0] == dstPtr+offset, skip one copy
          if (Send) {
            // (1-Send) is only there to avoid compilation errors in case MaxSend=0 (and Send=0).
            ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, (1-Send)+MaxSend, 0>
              (tid, nworkers, nullptr, false,
               1, (T const**)ncclShmem.groups[group].srcs,
               fan.nsend(), (T**)ncclShmem.groups[group].dsts+1,
               sliceSize);
          }
        } else if (DirectSend && !DirectRecv && SrcBuf != Input && ncclShmem.groups[group].dsts[Dst] == nullptr) {
          // For broadcast in CollNet to do empty send
          ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, 0>
            (tid, nworkers, ncclShmem.redOpArgs, postOp,
             Recv, (T const**)ncclShmem.groups[group].srcs,
             Dst, (T**)ncclShmem.groups[group].dsts,
             sliceSize);
        } else {
/* 中文精读：多个原语怎样共享一个融合搬运内核
 * srcs 合并“本地输入（如果有）+ 接收邻居”，dsts 合并“本地输出（如果有）+ 发送邻居”。
 * ReduceOrCopyMulti 对同一批加载值执行归约并写入多个目标，避免先写临时结果再启动另一个 kernel。
 * PreOpN 限定哪些源需要输入预处理；postOp 只在全局结果完成处执行，例如整数平均最后除法。
 * 中间转发如果重复应用最终 postOp，会改变数学结果，因此算法层明确传 true/false。
 */
          constexpr int PreOpN = SrcBuf != Input ? 0 :
                                 DirectRecv*MaxRecv == NCCL_MAX_DIRECT_ARITY ? (1+NCCL_MAX_DIRECT_ARITY) : 1;
          ReduceOrCopyMulti<Unroll, RedOp, T, Recv+Src, Recv*MaxRecv+Src, Send+Dst, Send*MaxSend+Dst, PreOpN>
            (tid, nworkers, ncclShmem.redOpArgs, postOp,
             Recv*fan.nrecv()+Src, (T const**)ncclShmem.groups[group].srcs,
             Send*fan.nsend()+Dst, (T**)ncclShmem.groups[group].dsts,
             sliceSize);
        }
        barrier(); // This barrier has a counterpart in following loop
        if (Send && (flags & RolePostSend) && index == 0) __threadfence_system();
        __syncwarp();
        postPeer<Recv, Send>();
        offset += sliceSize;
        slice += 1;
      } while (slice < SlicePerChunk && offset < nelem);
    }

/* 中文精读：同步线程与复制线程必须走匹配的屏障
 * worker 在上面的循环复制有效 slice，非 worker 直接进入这里参与同一组 barrier/postPeer。
 * worker 遇到剩余空 slice 后也进入这里，确保所有参与线程的屏障次数一致。
 * Send 发布 tail 之前先做系统级 fence，使数据写入对 GPU/CPU/NIC 协作路径可见。
 * volatile 让进度被反复读取，barrier 协调本组线程，fence 约束可见性；三者职责不同。
 */
    // Non-workers come straight here. Workers too but only once the remaining
    // slices are all empty. Since empty slices are the uncommon case, and
    // worker perf is the limiter, perf-wise this loop is effectively unentered,
    // hence just a single branch insn.
    #pragma unroll 1
    while (slice < SlicePerChunk) {
      sliceSize = sliceSize < nelem-offset ? sliceSize : nelem-offset;
      { // Only workers could have Wait roles so we know the slice must be empty
        // since we've exited the loop above.
        waitPeer<DirectRecv, DirectSend, Recv, Send, Src, Dst>(0, 0, 0, 0);
      }
      barrier(); // Has couterpart in preceding worker-only loop.
      if (Send && (flags & RolePostSend) && sliceSize > 0 && index == 0) __threadfence_system();
      __syncwarp();
      postPeer<Recv, Send>();
      offset += sliceSize;
      slice += 1;
    }
  }
```

</details>

<a id="f0233"></a>

## F0233：ScatterGatherOp

用途：按 peer 分配或收集不同数据段，支持 CollNet 多 head 的分散/汇集以及直接缓冲访问优化。

- 每slice先算有效peer范围，再按shift轮转peer次序，skip跳过自己负责的段。
- Scatter与Gather使用不同源目标组合；direct重合地址可以省复制，但仍保留屏障和进度发布。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>constexpr int DirectRecv = 1 && Direct && DirectRecv1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>constexpr int DirectSend = 1 && Direct && DirectSend1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int sliceSize = stepSize*StepPerSlice;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>int dataSize = max(DIVUP(peerElem, 16*SlicePerChunk)*16, sliceSize/32);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 7 | 0 / 循环 | <code>初始 int slice=0;；继续条件 slice&lt;SlicePerChunk；每轮更新 ++slice</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 准备局部状态 | <code>int realSize = max(0, min(dataSize, peerElem-offset));</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 9 | 1 / 条件判断 | <code>(tid &lt; nworkers)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 3 / 条件判断 | <code>(Send)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 5 / 准备局部状态 | <code>constexpr int PreOpN = DirectSend ? 0 : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 5 / 条件判断 | <code>(flags & RoleInput)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 7 / 调用 / 计算 / 写入 | <code>ncclShmem.groups[group].srcs[0] = userBuff + inpIx + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 5 / 条件判断 | <code>(tid == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 7 / 调用 / 计算 / 写入 | <code>ncclShmem.groups[group].totalSendSize[slice] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 5 / 调用 / 计算 / 写入 | <code>waitPeer&lt;0, DirectSend, 0, 1, 1, 0&gt;(0, inpIx, offset, realSize);</code> | 发送侧等空槽、接收侧等数据，并按 direct/shared/固定 FIFO 条件装配本 slice 的实际指针。 |
| 21 | 5 / 调用 / 计算 / 写入 | <code>subBarrier();</code> | 只同步负责数据搬运的 worker 子组，配合同步 warp 让复制与进度发布流水重叠。 |
| 22 | 5 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 23 | 5 / 循环 | <code>初始 int j=0;；继续条件 j&lt;fan.nsend()；每轮更新 j++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 6 / 准备局部状态 | <code>int i = (j+shift)%fan.nsend();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 6 / 准备局部状态 | <code>int peerOffset = i*peerElem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 6 / 条件判断 | <code>(skip &gt;= 0 && i &gt;= skip)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 8 / 调用 / 计算 / 写入 | <code>peerOffset += peerElem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 6 / 准备局部状态 | <code>const T* src0 = (T*)ncclShmem.groups[group].srcs[0] + peerOffset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 6 / 准备局部状态 | <code>int realPeerSize = min(realSize, totalElem-peerOffset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 31 | 6 / 条件判断 | <code>(realPeerSize &gt; 0 && ncclShmem.groups[group].dsts[i] != nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 8 / 调用 / 计算 / 写入 | <code>ReduceOrCopyMulti&lt;Unroll, RedOp, T, 1, 1, 1, 1, PreOpN&gt;(tid, nworkers, ncclShmem.redOpArgs, false, 1, &src0, 1, (T**)ncclShmem.groups[group].dsts+i, realPeerSize);</code> | 统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。 |
| 34 | 8 / 条件判断 | <code>(tid == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 10 / 调用 / 计算 / 写入 | <code>ncclShmem.groups[group].totalSendSize[slice] += realPeerSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 5 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 39 | 5 / 条件判断 | <code>(Recv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 7 / 条件判断 | <code>(flags & RoleOutput)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 9 / 调用 / 计算 / 写入 | <code>ncclShmem.groups[group].dsts[0] = userBuff + outIx + offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 7 / 准备局部状态 | <code>int peerOffset = index*peerElem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 7 / 条件判断 | <code>(skip &gt;= 0 && index &gt;= skip)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 9 / 调用 / 计算 / 写入 | <code>peerOffset += peerElem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 7 / 调用 / 计算 / 写入 | <code>waitPeer&lt;DirectRecv, 0, 1, 0, 0, 1&gt;(outIx, outIx+peerOffset, offset, realSize);</code> | 发送侧等空槽、接收侧等数据，并按 direct/shared/固定 FIFO 条件装配本 slice 的实际指针。 |
| 49 | 7 / 调用 / 计算 / 写入 | <code>subBarrier();</code> | 只同步负责数据搬运的 worker 子组，配合同步 warp 让复制与进度发布流水重叠。 |
| 50 | 7 / 条件判断 | <code>(DirectRecv && ncclShmem.groups[group].srcs[0] == ncclShmem.groups[group].dsts[0])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 53 | 9 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 54 | 9 / 循环 | <code>初始 int j=0;；继续条件 j&lt;fan.nrecv()；每轮更新 j++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 55 | 10 / 准备局部状态 | <code>int i = (j+shift)%fan.nrecv();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 10 / 调用 / 计算 / 写入 | <code>peerOffset = i*peerElem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 10 / 条件判断 | <code>(skip &gt;= 0 && i &gt;= skip)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 58 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 59 | 12 / 调用 / 计算 / 写入 | <code>peerOffset += peerElem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 10 / 准备局部状态 | <code>T* dst0 = (T*)ncclShmem.groups[group].dsts[0] + peerOffset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 61 | 10 / 准备局部状态 | <code>int realPeerSize = min(realSize, totalElem-peerOffset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 62 | 10 / 条件判断 | <code>(realPeerSize &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 12 / 调用 / 计算 / 写入 | <code>ReduceOrCopyMulti&lt;Unroll, RedOp, T, 1, 1, 1, 1, 0&gt;(tid, nworkers, ncclShmem.redOpArgs, postOp, 1, (const T**)ncclShmem.groups[group].srcs+i, 1, &dst0, realPeerSize);</code> | 统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。 |
| 65 | 9 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 66 | 1 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |
| 67 | 1 / 条件判断 | <code>(Send && (flags & RolePostSend) && ncclShmem.groups[group].totalSendSize[slice] &gt; 0 && index == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 3 / 调用 / 计算 / 写入 | <code>__threadfence_system();</code> | 约束先前数据写与随后进度发布的可见性顺序；这不是等待所有rank完成。 |
| 70 | 1 / 调用 / 计算 / 写入 | <code>__syncwarp();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 71 | 1 / 调用 / 计算 / 写入 | <code>postPeer&lt;Recv, Send&gt;();</code> | 发布本 slice 完成后的连接进度，让消费者能读或生产者能回收空间。 |
| 72 | 1 / 调用 / 计算 / 写入 | <code>offset += realSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    constexpr int DirectRecv = 1 && Direct && DirectRecv1;
    constexpr int DirectSend = 1 && Direct && DirectSend1;
    int offset = 0; // slice offset
    int sliceSize = stepSize*StepPerSlice;
    int dataSize = max(DIVUP(peerElem, 16*SlicePerChunk)*16, sliceSize/32);  // per-peer slice size

    #pragma unroll
    for (int slice=0; slice<SlicePerChunk; ++slice) {
      int realSize = max(0, min(dataSize, peerElem-offset));
      if (tid < nworkers) {
        if (Send) {
          // Scatter pre-scales data of input buffer only in non-Direct case
          constexpr int PreOpN = DirectSend ? 0 : 1;
          if (flags & RoleInput) ncclShmem.groups[group].srcs[0] = userBuff + inpIx + offset;
          if (tid == 0) ncclShmem.groups[group].totalSendSize[slice] = 0; // Skip the threadfence
          // realSize is not accurate here; but intra-node does not rely on sizes FIFO
          waitPeer<0, DirectSend, 0, 1, 1, 0>(0, inpIx, offset, realSize);
          subBarrier();
          #pragma unroll
          // Loop over peers
          for (int j=0; j<fan.nsend(); j++) {
            int i = (j+shift)%fan.nsend();
            int peerOffset = i*peerElem;
            // Skip the data I am responsible of reducing myself
            if (skip >= 0 && i >= skip) peerOffset += peerElem;
            const T* src0 = (T*)ncclShmem.groups[group].srcs[0] + peerOffset;
            int realPeerSize = min(realSize, totalElem-peerOffset);
            if (realPeerSize > 0 && ncclShmem.groups[group].dsts[i] != nullptr) {
              ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, PreOpN>(tid, nworkers, ncclShmem.redOpArgs, false, 1, &src0, 1, (T**)ncclShmem.groups[group].dsts+i, realPeerSize);
              // Mark for threadfence at the end
              if (tid == 0) ncclShmem.groups[group].totalSendSize[slice] += realPeerSize;
            }
          }
        } else if (Recv) {
          if (flags & RoleOutput) ncclShmem.groups[group].dsts[0] = userBuff + outIx + offset;
          int peerOffset = index*peerElem;
          if (skip >= 0 && index >= skip) peerOffset += peerElem;
          // Adjust remote index with peer offset in case we are directly pulling from peer's output buffer
          waitPeer<DirectRecv, 0, 1, 0, 0, 1>(outIx, outIx+peerOffset, offset, realSize);
          subBarrier();
          if (DirectRecv && ncclShmem.groups[group].srcs[0] == ncclShmem.groups[group].dsts[0]) {
            // Since waitPeer sets srcs[0] to output buffer + offset, we are doing a direct-write based recv
            // Do nothing
          } else {
            #pragma unroll
            for (int j=0; j<fan.nrecv(); j++) {
              int i = (j+shift)%fan.nrecv();
              peerOffset = i*peerElem;
              if (skip >= 0 && i >= skip) peerOffset += peerElem;
              T* dst0 = (T*)ncclShmem.groups[group].dsts[0] + peerOffset;
              int realPeerSize = min(realSize, totalElem-peerOffset);
              if (realPeerSize > 0) ReduceOrCopyMulti<Unroll, RedOp, T, 1, 1, 1, 1, 0>(tid, nworkers, ncclShmem.redOpArgs, postOp, 1, (const T**)ncclShmem.groups[group].srcs+i, 1, &dst0, realPeerSize);
            }
          }
        }
      }
      barrier();
      // If we indeed send something, threadfence
      if (Send && (flags & RolePostSend) && ncclShmem.groups[group].totalSendSize[slice] > 0 && index == 0)
        __threadfence_system();
      __syncwarp();
      postPeer<Recv, Send>();
      offset += realSize;
    }
  }
```

</details>

<a id="f0234"></a>

## F0234：loadRecvConn

用途：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(flags & (RoleWaitRecv&#124;RolePostRecv))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>auto *conn = &peer-&gt;recv[connIndex].conn;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>step = conn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>step = roundUp(step, SlicePerChunk*StepPerSlice);</code> | 把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。 |
| 6 | 2 / 条件判断 | <code>(flags & RolePostRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>connStepPtr = conn-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>*connStepPtr = step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 10 | 2 / 条件判断 | <code>(flags & RoleWaitRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>ncclShmem.groups[group].recvConns[index] = conn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>connStepPtr = conn-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>connStepCache = *connStepPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>flags &#124;= (conn-&gt;offsFifo != nullptr) ? OffsFifoEnabled : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 4 / 条件判断 | <code>(Direct)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 6 / 条件判断 | <code>((conn-&gt;direct & (NCCL_IPC_READ&#124;NCCL_IPC_WRITE)) && e != nullptr && e-&gt;regUsed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 8 / 条件判断 | <code>(connIndex == 1 && P2p == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 10 / 调用 / 计算 / 写入 | <code>flags &#124;= DirectRead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 9 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 24 | 10 / 调用 / 计算 / 写入 | <code>flags &#124;= (e-&gt;direct & NCCL_DIRECT_WRITE) ? DirectWrite : (e-&gt;direct & NCCL_DIRECT_READ) ? DirectRead : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 26 | 8 / 条件判断 | <code>(conn-&gt;direct & (NCCL_DIRECT_WRITE&#124;NCCL_DIRECT_READ))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 10 / 条件判断 | <code>(connIndex == 1 && P2p == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 12 / 调用 / 计算 / 写入 | <code>flags &#124;= DirectRead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 11 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 32 | 12 / 调用 / 计算 / 写入 | <code>flags &#124;= (conn-&gt;direct & NCCL_DIRECT_WRITE) ? DirectWrite : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 4 / 条件判断 | <code>(flags & OffsFifoEnabled)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 6 / 调用 / 计算 / 写入 | <code>connOffsFifoPtr = conn-&gt;offsFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 4 / 调用 / 计算 / 写入 | <code>connEltsFifo = (T*)conn-&gt;buffs[NCCL_PROTO_SIMPLE];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (flags & (RoleWaitRecv|RolePostRecv)) {
      auto *conn = &peer->recv[connIndex].conn;
      step = conn->step;
      step = roundUp(step, SlicePerChunk*StepPerSlice);
      if (flags & RolePostRecv) {
        connStepPtr = conn->head;
        *connStepPtr = step; // Return credits in case we rounded up.
      }
      if (flags & RoleWaitRecv) {
        ncclShmem.groups[group].recvConns[index] = conn; // WaitRecv role saves since that's who needs it in setDataPtrs()
        connStepPtr = conn->tail;
        connStepCache = *connStepPtr;
        flags |= (conn->offsFifo != nullptr) ? OffsFifoEnabled : 0;
        if (Direct) {
          // User buffers have been registered
          if ((conn->direct & (NCCL_IPC_READ|NCCL_IPC_WRITE)) && e != nullptr && e->regUsed) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              flags |= (e->direct & NCCL_DIRECT_WRITE) ? DirectWrite :
                       (e->direct & NCCL_DIRECT_READ)  ? DirectRead  : 0;
            }
          } else if (conn->direct & (NCCL_DIRECT_WRITE|NCCL_DIRECT_READ)) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              // direct read not allowed in non-register case
              // otherwise, in one-to-multi send, we could mix empty send and intermediate send
              flags |= (conn->direct & NCCL_DIRECT_WRITE) ? DirectWrite : 0;
            }
          }
        }
        if (flags & OffsFifoEnabled)
          connOffsFifoPtr = conn->offsFifo;
        connEltsFifo = (T*)conn->buffs[NCCL_PROTO_SIMPLE];
      }
    }
  }
```

</details>

<a id="f0235"></a>

## F0235：loadSendConn

用途：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(flags & (RoleWaitSend&#124;RolePostSend))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>auto *conn = &peer-&gt;send[connIndex].conn;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>step = conn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>step = roundUp(step, SlicePerChunk*StepPerSlice);</code> | 把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。 |
| 6 | 2 / 条件判断 | <code>(flags & RolePostSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>connStepPtr = conn-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 9 | 2 / 条件判断 | <code>(flags & RoleWaitSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 调用 / 计算 / 写入 | <code>ncclShmem.groups[group].sendConns[index] = conn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>connStepPtr = conn-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>connStepCache = *connStepPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>flags &#124;= (conn-&gt;offsFifo != nullptr) ? OffsFifoEnabled : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 4 / 条件判断 | <code>(flags & OffsFifoEnabled)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 6 / 调用 / 计算 / 写入 | <code>connOffsFifoPtr = conn-&gt;offsFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 4 / 调用 / 计算 / 写入 | <code>connEltsFifo = (T*)conn-&gt;buffs[NCCL_PROTO_SIMPLE];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 4 / 条件判断 | <code>(conn-&gt;sizesFifo != nullptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 6 / 调用 / 计算 / 写入 | <code>flags &#124;= SizesFifoEnabled;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 6 / 调用 / 计算 / 写入 | <code>connSizesFifoPtr = conn-&gt;sizesFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 24 | 6 / 条件判断 | <code>(Direct)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 8 / 条件判断 | <code>((conn-&gt;direct & (NCCL_IPC_READ&#124;NCCL_IPC_WRITE)) && e != nullptr && e-&gt;regUsed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 10 / 条件判断 | <code>(connIndex == 1 && P2p == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 12 / 调用 / 计算 / 写入 | <code>flags &#124;= DirectRead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 11 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 32 | 12 / 调用 / 计算 / 写入 | <code>flags &#124;= (e-&gt;direct & NCCL_DIRECT_WRITE) ? DirectWrite : (e-&gt;direct & NCCL_DIRECT_READ) ? DirectRead : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 9 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 34 | 10 / 条件判断 | <code>(conn-&gt;direct & (NCCL_DIRECT_WRITE&#124;NCCL_DIRECT_READ))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 12 / 条件判断 | <code>(connIndex == 1 && P2p == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 13 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 14 / 调用 / 计算 / 写入 | <code>flags &#124;= DirectRead;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 13 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 40 | 14 / 调用 / 计算 / 写入 | <code>flags &#124;= (conn-&gt;direct & NCCL_DIRECT_WRITE) ? DirectWrite : 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (flags & (RoleWaitSend|RolePostSend)) {
      auto *conn = &peer->send[connIndex].conn;
      step = conn->step;
      step = roundUp(step, SlicePerChunk*StepPerSlice);
      if (flags & RolePostSend) {
        connStepPtr = conn->tail;
      }
      if (flags & RoleWaitSend) {
        ncclShmem.groups[group].sendConns[index] = conn; // WaitSend role saves since that's who needs it in setDataPtrs()
        connStepPtr = conn->head;
        connStepCache = *connStepPtr;
        flags |= (conn->offsFifo != nullptr) ? OffsFifoEnabled : 0;
        if (flags & OffsFifoEnabled)
          connOffsFifoPtr = conn->offsFifo;
        connEltsFifo = (T*)conn->buffs[NCCL_PROTO_SIMPLE];

        if (conn->sizesFifo != nullptr) {
          flags |= SizesFifoEnabled;
          connSizesFifoPtr = conn->sizesFifo;
        } else if (Direct) {
          // User buffers have been registered
          if ((conn->direct & (NCCL_IPC_READ|NCCL_IPC_WRITE)) && e != nullptr && e->regUsed) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              flags |= (e->direct & NCCL_DIRECT_WRITE) ? DirectWrite :
                       (e->direct & NCCL_DIRECT_READ)  ? DirectRead  : 0;
            }
          } else if (conn->direct & (NCCL_DIRECT_WRITE|NCCL_DIRECT_READ)) {
            if (connIndex == 1 && P2p == 0) {
              flags |= DirectRead;  // scatter-reduce use direct pull
            } else {
              // direct read not allowed in non-register case
              // otherwise, in one-to-multi send, we could mix empty send and intermediate send
              flags |= (conn->direct & NCCL_DIRECT_WRITE) ? DirectWrite : 0;
            }
          }
        }
      }
    }
  }
```

</details>

<a id="f0236"></a>

## F0236：Primitives

用途：为一个线程组建立原语上下文：实际邻居度、协议缓冲、线程角色、同步编号及用户数据指针。

- 从邻居列表计算实际收发数并确定worker和额外同步warp。
- 按8线程角色分组设置Wait/Input/Output/Post位，再加载对应方向的连接视图。
- 最后交换用户缓冲指针/缩放参数；构造过程可能等待配对方，不只是赋几个普通字段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>this-&gt;nthreads = nthreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>this-&gt;nworkers = nthreads - (MaxSend &gt; 0 && nthreads-WARP_SIZE &gt;= 64 ? WARP_SIZE : 0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>this-&gt;group = group & (uint16_t)0xFFFF;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 准备局部状态 | <code>int connIndex = group &gt;&gt; 16;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>int nrecv=0, nsend=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 循环 | <code>初始 ；继续条件 (nrecv &lt; MaxRecv && recvPeers[nrecv] != -1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>nrecv++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 9 | 0 / 循环 | <code>初始 ；继续条件 (nsend &lt; MaxSend && sendPeers[nsend] != -1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>nsend++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>this-&gt;fan = Fan(nrecv, nsend);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 准备局部状态 | <code>constexpr int ThreadPerSync = 8;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 源码语法片段 | <code>static_assert(MaxSend &lt; ThreadPerSync && MaxRecv &lt; ThreadPerSync, "Not enough threads to cover all peers");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 15 | 0 / 准备局部状态 | <code>int g = tid / ThreadPerSync;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 准备局部状态 | <code>int ng = nthreads / ThreadPerSync;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>index = tid % ThreadPerSync;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>flags = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(g == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 条件判断 | <code>(index &lt; nrecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>flags &#124;= RoleWaitRecv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 2 / 条件判断 | <code>(index == nrecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>flags &#124;= RoleInput;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 28 | 2 / 条件判断 | <code>(g == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 4 / 条件判断 | <code>(index &lt; nsend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 6 / 调用 / 计算 / 写入 | <code>flags &#124;= RoleWaitSend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 4 / 条件判断 | <code>(index == nsend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 6 / 调用 / 计算 / 写入 | <code>flags &#124;= RoleOutput;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 37 | 4 / 条件判断 | <code>(g == ng - 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 6 / 条件判断 | <code>(index &lt; nrecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 8 / 调用 / 计算 / 写入 | <code>flags &#124;= RolePostRecv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 43 | 6 / 条件判断 | <code>(g == ng - 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 8 / 条件判断 | <code>(index &lt; nsend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 10 / 调用 / 计算 / 写入 | <code>flags &#124;= RolePostSend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 准备局部状态 | <code>int peer = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 0 / 条件判断 | <code>(flags & (RoleWaitRecv&#124;RolePostRecv))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 2 / 调用 / 计算 / 写入 | <code>peer = recvPeers[index];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 0 / 条件判断 | <code>(flags & (RoleWaitSend&#124;RolePostSend))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 2 / 调用 / 计算 / 写入 | <code>peer = sendPeers[index];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 0 / 调用 / 计算 / 写入 | <code>loadRecvConn(&ncclShmem.channel.devPeers[peer], connIndex, e);</code> | 从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。 |
| 56 | 0 / 调用 / 计算 / 写入 | <code>loadSendConn(&ncclShmem.channel.devPeers[peer], connIndex, e);</code> | 从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。 |
| 57 | 0 / 调用 / 计算 / 写入 | <code>setDataPtrs(inputBuf, outputBuf, redOpArg, (struct ncclWorkElemReg*)e);</code> | 更新本原语的本地输入和输出缓冲指针；Simple 的 direct 路径还需完成对应地址交换。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{

    // For send operations, we need an extra warp to overlap the threadfence and the copy
    this->nthreads = nthreads;
    this->nworkers = nthreads - (MaxSend > 0 && nthreads-WARP_SIZE >= 64 ? WARP_SIZE : 0);
    this->group = group & (uint16_t)0xFFFF;
    int connIndex = group >> 16;

    int nrecv=0, nsend=0;
    while (nrecv < MaxRecv && recvPeers[nrecv] != -1) nrecv++;
    while (nsend < MaxSend && sendPeers[nsend] != -1) nsend++;
    this->fan = Fan(nrecv, nsend);

    constexpr int ThreadPerSync = 8;
    static_assert(MaxSend < ThreadPerSync && MaxRecv < ThreadPerSync, "Not enough threads to cover all peers");

    int g = tid / ThreadPerSync;
    int ng = nthreads / ThreadPerSync;
    index = tid % ThreadPerSync;
    flags = 0;
    if (g == 0) {
      if (index < nrecv) flags |= RoleWaitRecv;
      if (index == nrecv) flags |= RoleInput;
    } else if (g == 1) {
      if (index < nsend) flags |= RoleWaitSend;
      if (index == nsend) flags |= RoleOutput;
    } else if (g == ng - 2) {
      if (index < nrecv) flags |= RolePostRecv;
    } else if (g == ng - 1) {
      if (index < nsend) flags |= RolePostSend;
    }

    int peer = 0;
    if (flags & (RoleWaitRecv|RolePostRecv)) peer = recvPeers[index];
    if (flags & (RoleWaitSend|RolePostSend)) peer = sendPeers[index];

    loadRecvConn(&ncclShmem.channel.devPeers[peer], connIndex, e);
    loadSendConn(&ncclShmem.channel.devPeers[peer], connIndex, e);

    setDataPtrs(inputBuf, outputBuf, redOpArg, (struct ncclWorkElemReg*)e);
  }
```

</details>

<a id="f0237"></a>

## F0237：~Primitives

用途：把本次推进后的 step 写回长期连接并按协议要求同步，后续操作继续复用同一连接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(!(flags & ThreadsSynced))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |
| 4 | 0 / 条件判断 | <code>(flags & (RolePostSend&#124;RolePostRecv))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 准备局部状态 | <code>auto *conns = (flags & RolePostSend) ? ncclShmem.groups[group].sendConns : ncclShmem.groups[group].recvConns;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>conns[index]-&gt;step = step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // Ensure ncclShmem.groups[].send/recvConns are available
    if (!(flags & ThreadsSynced))
      barrier();
    // Save steps for the next operation
    if (flags & (RolePostSend|RolePostRecv)) {
      auto *conns = (flags & RolePostSend) ? ncclShmem.groups[group].sendConns : ncclShmem.groups[group].recvConns;
      conns[index]->step = step;
    }
    // Make sure all threads are done writing back conn->step and done using
    // ncclShmem.groups[group]
    barrier();
  }
```

</details>

<a id="f0238"></a>

## F0238：setDataPtrs

用途：更新本原语的本地输入和输出缓冲指针；Simple 的 direct 路径还需完成对应地址交换。

- 普通路径只设置本地Input/Output；direct路径再分提供者和接收者。
- 直接写由接收方提供output，直接读由发送方提供input或output；指针槽以nullptr表示空，所以传输时采用异或编码。
- 直接读还分两段交换标量，每段加有效标记，支持数值本身为零；取走后清槽避免覆盖上一轮。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(flags & RoleInput)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>userBuff = (T*)inputBuf;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>ncclShmem.redOpArgs[0] = redOpArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(flags & RoleOutput)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>userBuff = (T*)outputBuf;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 准备局部状态 | <code>bool recvProvider = flags == (flags&#124;RoleWaitRecv&#124;DirectWrite);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>bool sendAcceptor = flags == (flags&#124;RoleWaitSend&#124;DirectWrite);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>bool sendProvider = flags == (flags&#124;RoleWaitSend&#124;DirectRead);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>bool recvAcceptor = flags == (flags&#124;RoleWaitRecv&#124;DirectRead);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>int regUsed = e != nullptr ? e-&gt;elem.regUsed : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 条件判断 | <code>(Direct && recvProvider)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 2 / 准备局部状态 | <code>void *volatile *slot = ncclShmem.groups[group].recvConns[index]-&gt;ptrExchange;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 2 / 循环 | <code>初始 ；继续条件 (*slot != nullptr && !checkAbort(spins))；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 19 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>directBuff = (T*)outputBuf;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>*slot = reinterpret_cast&lt;T*&gt;(reinterpret_cast&lt;uintptr_t&gt;(directBuff) ^ reinterpret_cast&lt;uintptr_t&gt;(slot));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 条件判断 | <code>(Direct && sendAcceptor)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 2 / 准备局部状态 | <code>void *volatile *slot = ncclShmem.groups[group].sendConns[index]-&gt;ptrExchange;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 2 / 准备局部状态 | <code>void *ptr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 2 / 循环 | <code>初始 ；继续条件 (true)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>ptr = *slot;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 3 / 条件判断 | <code>(ptr != nullptr &#124;&#124; checkAbort(spins))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 32 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>directBuff = regUsed ? (T*)(e-&gt;dnOutputs[index]) : reinterpret_cast&lt;T*&gt;(reinterpret_cast&lt;uintptr_t&gt;(ptr) ^ reinterpret_cast&lt;uintptr_t&gt;(slot));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>*slot = nullptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 0 / 条件判断 | <code>(Direct && sendProvider)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 2 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 2 / 准备局部状态 | <code>void *volatile *slot = ncclShmem.groups[group].sendConns[index]-&gt;ptrExchange;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 39 | 2 / 准备局部状态 | <code>volatile uint64_t* argSlot0 = ncclShmem.groups[group].sendConns[index]-&gt;redOpArgExchange;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 2 / 准备局部状态 | <code>volatile uint64_t* argSlot1 = ncclShmem.groups[group].sendConns[index]-&gt;redOpArgExchange+1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 2 / 循环 | <code>初始 ；继续条件 ((*slot != nullptr &#124;&#124; *argSlot0 != 0 &#124;&#124; *argSlot1 !=0) && !checkAbort(spins))；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 42 | 3 / 调用 / 计算 / 写入 | <code>;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 43 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>directBuff = MaxRecv == 0 ? (T*)inputBuf : (T*)outputBuf;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 2 / 调用 / 计算 / 写入 | <code>*argSlot0 = (uint64_t(1)&lt;&lt;32) &#124; (uint32_t)redOpArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 2 / 调用 / 计算 / 写入 | <code>*argSlot1 = (uint64_t(1)&lt;&lt;32) &#124; (uint32_t)(redOpArg&gt;&gt;32);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 2 / 调用 / 计算 / 写入 | <code>*slot = reinterpret_cast&lt;T*&gt;(reinterpret_cast&lt;uintptr_t&gt;(directBuff) ^ reinterpret_cast&lt;uintptr_t&gt;(slot));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 条件判断 | <code>(Direct && recvAcceptor)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 49 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 50 | 2 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 51 | 2 / 准备局部状态 | <code>void *volatile *slot = ncclShmem.groups[group].recvConns[index]-&gt;ptrExchange;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 2 / 准备局部状态 | <code>volatile uint64_t* argSlot0 = ncclShmem.groups[group].recvConns[index]-&gt;redOpArgExchange;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 53 | 2 / 准备局部状态 | <code>volatile uint64_t* argSlot1 = ncclShmem.groups[group].recvConns[index]-&gt;redOpArgExchange+1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 2 / 准备局部状态 | <code>void *ptr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 55 | 2 / 循环 | <code>初始 ；继续条件 (true)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 56 | 3 / 调用 / 计算 / 写入 | <code>ptr = *slot;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 3 / 条件判断 | <code>(ptr != nullptr &#124;&#124; checkAbort(spins))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 58 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 59 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 60 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 61 | 2 / 调用 / 计算 / 写入 | <code>directBuff = regUsed ? (T*)(MaxSend == 0 ? e-&gt;upOutputs[index] : e-&gt;dnInputs[index]) : reinterpret_cast&lt;T*&gt;(reinterpret_cast&lt;uintptr_t&gt;(ptr) ^ reinterpret_cast&lt;uintptr_t&gt;(slot));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 2 / 条件判断 | <code>(MaxSend != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 4 / 准备局部状态 | <code>uint64_t arg0, arg1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 65 | 4 / 循环 | <code>初始 ；继续条件 (true)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 66 | 5 / 调用 / 计算 / 写入 | <code>arg0 = *argSlot0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 5 / 调用 / 计算 / 写入 | <code>arg1 = *argSlot1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 68 | 5 / 条件判断 | <code>((arg0 != 0 && arg1 != 0) &#124;&#124; checkAbort(spins))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 69 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 70 | 7 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 71 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 72 | 4 / 调用 / 计算 / 写入 | <code>ncclShmem.redOpArgs[1+index] = ((arg1 & 0xffffffff)&lt;&lt;32) &#124; (arg0 & 0xffffffff);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 2 / 调用 / 计算 / 写入 | <code>*argSlot0 = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 74 | 2 / 调用 / 计算 / 写入 | <code>*argSlot1 = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 75 | 2 / 调用 / 计算 / 写入 | <code>*slot = nullptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (flags & RoleInput) {
      userBuff = (T*)inputBuf;
      ncclShmem.redOpArgs[0] = redOpArg;  // scaler for local input
    }
    if (flags & RoleOutput) userBuff = (T*)outputBuf;
    bool recvProvider = flags == (flags|RoleWaitRecv|DirectWrite);
    bool sendAcceptor = flags == (flags|RoleWaitSend|DirectWrite);
    bool sendProvider = flags == (flags|RoleWaitSend|DirectRead); // sender provides direct buffer (to be fetched)
    bool recvAcceptor = flags == (flags|RoleWaitRecv|DirectRead); // receiver accepts direct buffer
    int regUsed = e != nullptr ? e->elem.regUsed : 0;

    if (Direct && recvProvider) {
      int spins = 0;
      void *volatile *slot = ncclShmem.groups[group].recvConns[index]->ptrExchange;
      // Wait for consumer to consume previous value before trampling it.
      while (*slot != nullptr && !checkAbort(spins));
      directBuff = (T*)outputBuf;
      // Encode pointer by XOR'ing against some address they definitely wouldn't send
      // since we want to allow them sending us nullptr while not colliding with
      // the empty slot value.
      *slot = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(directBuff) ^ reinterpret_cast<uintptr_t>(slot));
    }
    if (Direct && sendAcceptor) {
      int spins = 0;
      void *volatile *slot = ncclShmem.groups[group].sendConns[index]->ptrExchange;
      void *ptr;
      while (true) {
        ptr = *slot;
        if (ptr != nullptr || checkAbort(spins)) break;
      }
      directBuff = regUsed ? (T*)(e->dnOutputs[index]) :
                   reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) ^ reinterpret_cast<uintptr_t>(slot));
      *slot = nullptr;
    }
    if (Direct && sendProvider) {
      int spins = 0;
      void *volatile *slot = ncclShmem.groups[group].sendConns[index]->ptrExchange;
      volatile uint64_t* argSlot0 = ncclShmem.groups[group].sendConns[index]->redOpArgExchange;
      volatile uint64_t* argSlot1 = ncclShmem.groups[group].sendConns[index]->redOpArgExchange+1;
      // Wait for consumer to consume previous value before trampling it.
      while ((*slot != nullptr || *argSlot0 != 0 || *argSlot1 !=0) && !checkAbort(spins));
      // If there is no recv, then we are directly pulling from input buffer (e.g. directScatter)
      // Otherwise, we are pulling from output buffer (e.g. recvCopyDirectSend)
      directBuff = MaxRecv == 0 ? (T*)inputBuf : (T*)outputBuf;
      // Exchange pre-scalers for use in direct pull
      *argSlot0 = (uint64_t(1)<<32) | (uint32_t)redOpArg;
      *argSlot1 = (uint64_t(1)<<32) | (uint32_t)(redOpArg>>32);
      // Encode pointer by XOR'ing against some address they definitely wouldn't send
      // since we want to allow them sending us nullptr while not colliding with
      // the empty slot value.
      *slot = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(directBuff) ^ reinterpret_cast<uintptr_t>(slot));
    }
    if (Direct && recvAcceptor) {
      int spins = 0;
      void *volatile *slot = ncclShmem.groups[group].recvConns[index]->ptrExchange;
      volatile uint64_t* argSlot0 = ncclShmem.groups[group].recvConns[index]->redOpArgExchange;
      volatile uint64_t* argSlot1 = ncclShmem.groups[group].recvConns[index]->redOpArgExchange+1;
      void *ptr;
      while (true) {
        ptr = *slot;
        if (ptr != nullptr || checkAbort(spins)) break;
      }
      directBuff = regUsed ? (T*)(MaxSend == 0 ? e->upOutputs[index] : e->dnInputs[index]) :
                   reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) ^ reinterpret_cast<uintptr_t>(slot));
      if (MaxSend != 0) { // reduce group rather than gather group
        // Store scalers for remote inputs
        uint64_t arg0, arg1;
        while (true) {
          arg0 = *argSlot0;
          arg1 = *argSlot1;
          if ((arg0 != 0 && arg1 != 0) || checkAbort(spins)) break;
        }
        ncclShmem.redOpArgs[1+index] = ((arg1 & 0xffffffff)<<32) | (arg0 & 0xffffffff);
      }
      *argSlot0 = 0; *argSlot1 = 0;
      *slot = nullptr;
    }
  }
```

</details>

<a id="f0239"></a>

## F0239：moveDataPtrs

用途：按元素偏移移动本原语的用户输入/输出指针，复用连接与协议状态处理另一段数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(flags & (RoleInput&#124;RoleOutput))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>userBuff += delta;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (flags & (RoleInput|RoleOutput))
      userBuff += delta;
  }
```

</details>

<a id="f0240"></a>

## F0240：send

用途：从本地输入读取并发送到原语的发送邻居，不要求写本地输出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 0, 0, 1, Input, -1&gt;(inpIx, -1, -1, eltN, false);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 0, 0, 1, Input, -1>(inpIx, -1, -1, eltN, false);
  }
```

</details>

<a id="f0241"></a>

## F0241：sendFromOutput

用途：从已生成的本地输出读数据并发送，避免错误地重新读取原始输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 0, 0, 1, Output, -1&gt;(outIx, -1, -1, eltN, false);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 0, 0, 1, Output, -1>(outIx, -1, -1, eltN, false);
  }
```

</details>

<a id="f0242"></a>

## F0242：directSend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 1, 0, 1, Input, -1&gt;(inpIx, -1, remoteOutIx, eltN, false);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 1, 0, 1, Input, -1>(inpIx, -1, remoteOutIx, eltN, false);
  }
```

</details>

<a id="f0243"></a>

## F0243：directSendFromOutput

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 1, 0, 1, Output, -1&gt;(outIx, -1, remoteOutIx, eltN, false);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 1, 0, 1, Output, -1>(outIx, -1, remoteOutIx, eltN, false);
  }
```

</details>

<a id="f0244"></a>

## F0244：recv

用途：接收上游数据并写本地输出，本动作不加上本地输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 0, 1, 0, -1, Output&gt;(-1, outIx, -1, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 0, 1, 0, -1, Output>(-1, outIx, -1, eltN, postOp);
  }
```

</details>

<a id="f0245"></a>

## F0245：directRecv

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;1, 0, 1, 0, -1, Output&gt;(-1, outIx, -1, eltN, false);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<1, 0, 1, 0, -1, Output>(-1, outIx, -1, eltN, /*postOp=*/false);
  }
```

</details>

<a id="f0246"></a>

## F0246：copySend

用途：把本地输入同时写本地输出并发下游，把本地保存和转发合并一次数据加载。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 0, 0, 1, Input, Output&gt;(inpIx, outIx, -1, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 0, 0, 1, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }
```

</details>

<a id="f0247"></a>

## F0247：directCopySend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 1, 0, 1, Input, Output&gt;(inpIx, outIx, remoteOutIx, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 1, 0, 1, Input, Output>(inpIx, outIx, remoteOutIx, eltN, postOp);
  }
```

</details>

<a id="f0248"></a>

## F0248：recvCopySend

用途：把已接收结果同时保存本地输出并转发下游，不再加入本地输入做归约。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 0, 1, 1, -1, Output&gt;(-1, outIx, -1, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 0, 1, 1, -1, Output>(-1, outIx, -1, eltN, postOp);
  }
```

</details>

<a id="f0249"></a>

## F0249：directRecvCopySend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;1, 1, 1, 1, -1, Output&gt;(-1, outIx, remoteOutIx, eltN, false);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<1, 1, 1, 1, -1, Output>(-1, outIx, remoteOutIx, eltN, false);
  }
```

</details>

<a id="f0250"></a>

## F0250：recvCopyDirectSend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 1, 1, 1, -1, Output&gt;(-1, outIx, remoteOutIx, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 1, 1, 1, -1, Output>(-1, outIx, remoteOutIx, eltN, postOp);
  }
```

</details>

<a id="f0251"></a>

## F0251：recvReduceCopy

用途：把接收数据与本地输入归约后写本地输出，可在结果完成时执行 postOp。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 0, 1, 0, Input, Output&gt;(inpIx, outIx, -1, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 0, 1, 0, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }
```

</details>

<a id="f0252"></a>

## F0252：recvReduceSend

用途：把上游部分结果与本地输入归约后发下游，不要求保留本地最终输出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 0, 1, 1, Input, -1&gt;(inpIx, -1, -1, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 0, 1, 1, Input, -1>(inpIx, -1, -1, eltN, postOp);
  }
```

</details>

<a id="f0253"></a>

## F0253：directRecvReduceSend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;1, 0, 1, 1, Input, -1&gt;(inpIx, -1, remoteInpIx, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<1, 0, 1, 1, Input, -1>(inpIx, -1, remoteInpIx, eltN, postOp);
  }
```

</details>

<a id="f0254"></a>

## F0254：recvReduceCopySend

用途：把接收与本地输入融合归约，同时保存输出并转发，衔接归约与结果传播阶段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 0, 1, 1, Input, Output&gt;(inpIx, outIx, -1, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    genericOp<0, 0, 1, 1, Input, Output>(inpIx, outIx, -1, eltN, postOp);
  }
```

</details>

<a id="f0255"></a>

## F0255：directRecvReduceCopySend

用途：在连接与协议支持时使用直接用户缓冲访问来完成对应收发/复制/归约动作，否则按实现退回协议缓冲。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>genericOp&lt;0, 1, 1, 1, Input, Output&gt;(inpIx, outIx, remoteOutIx, eltN, postOp);</code> | Simple 协议的组合原语核心，逐 slice 等待并融合搬运/归约，随后屏障/fence 并发布进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // Direct is only for the send part
    genericOp<0, 1, 1, 1, Input, Output>(inpIx, outIx, remoteOutIx, eltN, postOp);
  }
```

</details>

<a id="f0256"></a>

## F0256：scatter

用途：把当前输入范围按每 peer 的片段尺寸分散给多个上游 head，直接模式利用已交换的用户缓冲地址。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>ScatterGatherOp&lt;0, 0, 0, 1&gt;(inpIx, -1, totalElem, peerElem, skip, shift, false);</code> | 按 peer 分配或收集不同数据段，支持 CollNet 多 head 的分散/汇集以及直接缓冲访问优化。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    ScatterGatherOp<0, 0, 0, 1>(inpIx, -1, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }
```

</details>

<a id="f0257"></a>

## F0257：directScatter

用途：把当前输入范围按每 peer 的片段尺寸分散给多个上游 head，直接模式利用已交换的用户缓冲地址。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>ScatterGatherOp&lt;0, 1, 0, 1&gt;(inpIx, -1, totalElem, peerElem, skip, shift, false);</code> | 按 peer 分配或收集不同数据段，支持 CollNet 多 head 的分散/汇集以及直接缓冲访问优化。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    ScatterGatherOp<0, 1, 0, 1>(inpIx, -1, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }
```

</details>

<a id="f0258"></a>

## F0258：gather

用途：从多个 head 收集各自负责的结果片段到本地输出，保持整体张量布局。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>ScatterGatherOp&lt;0, 0, 1, 0&gt;(-1, outIx, totalElem, peerElem, skip, shift, postOp);</code> | 按 peer 分配或收集不同数据段，支持 CollNet 多 head 的分散/汇集以及直接缓冲访问优化。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    ScatterGatherOp<0, 0, 1, 0>(-1, outIx, totalElem, peerElem, skip, shift, postOp);
  }
```

</details>

<a id="f0259"></a>

## F0259：directGather

用途：从多个 head 收集各自负责的结果片段到本地输出，保持整体张量布局。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>ScatterGatherOp&lt;1, 0, 1, 0&gt;(-1, outIx, totalElem, peerElem, skip, shift, false);</code> | 按 peer 分配或收集不同数据段，支持 CollNet 多 head 的分散/汇集以及直接缓冲访问优化。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    ScatterGatherOp<1, 0, 1, 0>(-1, outIx, totalElem, peerElem, skip, shift, /*postOp=*/false);
  }
```

</details>

