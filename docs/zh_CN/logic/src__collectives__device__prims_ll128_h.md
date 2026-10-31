# src/collectives/device/prims_ll128.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0195"></a>

## F0195：recvOffset

用途：用连接 step 对 NCCL_STEPS 取模再乘每槽大小，把单调进度映射到循环数据槽位。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (recvStep[i]%NCCL_STEPS)*stepSize;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return (recvStep[i]%NCCL_STEPS)*stepSize; }
```

</details>

<a id="f0196"></a>

## F0196：sendOffset

用途：用连接 step 对 NCCL_STEPS 取模再乘每槽大小，把单调进度映射到循环数据槽位。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (sendStep[i]%NCCL_STEPS)*stepSize;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return (sendStep[i]%NCCL_STEPS)*stepSize; }
```

</details>

<a id="f0197"></a>

## F0197：recvPtr

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

<a id="f0198"></a>

## F0198：sendPtr

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

<a id="f0199"></a>

## F0199：recvFlag

用途：把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return recvStep[i]+1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return recvStep[i]+1; }
```

</details>

<a id="f0200"></a>

## F0200：sendFlag

用途：把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return sendStep[i]+1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return sendStep[i]+1; }
```

</details>

<a id="f0201"></a>

## F0201：barrier

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

<a id="f0202"></a>

## F0202：checkAbort

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

<a id="f0203"></a>

## F0203：waitSend

用途：等待发送缓冲仍有可复用槽位，并在存在 Proxy 时填写本轮物理字节数，防止覆盖 NIC/对端仍使用的数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(sendConnHeadPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 循环 | <code>初始 ；继续条件 (sendConnHeadCache + NCCL_STEPS &lt; sendConnHead + 1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>sendConnHeadCache = *sendConnHeadPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 条件判断 | <code>(checkAbort(spins, wid, 1))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 9 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 2 / 条件判断 | <code>(sendConnFifoPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>sendConnFifoPtr[sendStep[wid]%NCCL_STEPS] = nbytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>sendConnHead += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (sendConnHeadPtr) {
      int spins = 0;
      while (sendConnHeadCache + NCCL_STEPS < sendConnHead + 1) {
        sendConnHeadCache = *sendConnHeadPtr;
        if (checkAbort(spins, wid, 1)) break;
      }
      if (sendConnFifoPtr) {
        sendConnFifoPtr[sendStep[wid]%NCCL_STEPS] = nbytes;
      }
      sendConnHead += 1;
    }
  }
```

</details>

<a id="f0204"></a>

## F0204：postRecv

用途：在数据已消费后发布接收完成进度，通知生产者可以回收相应循环槽位。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(recvConnHeadPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>*recvConnHeadPtr = recvConnHead += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (recvConnHeadPtr) *recvConnHeadPtr = recvConnHead += 1;
  }
```

</details>

<a id="f0205"></a>

## F0205：postSend

用途：在数据/flag 写入完成并满足协议 fence 条件后发布发送进度，通知后续消费者。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(sendConnTailPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>__threadfence();</code> | 约束先前数据写与随后进度发布的可见性顺序；这不是等待所有rank完成。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>*sendConnTailPtr = sendConnTail += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (sendConnTailPtr) { __threadfence(); *sendConnTailPtr = sendConnTail += 1; }
  }
```

</details>

<a id="f0206"></a>

## F0206：loadRegsBegin

用途：按对齐情况提前把用户数据加载进 LL128 寄存器，延后 warp 重排以隐藏访存延迟。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>constexpr int EltPer16B = 16/sizeof(T);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(reinterpret_cast&lt;uintptr_t&gt;(src)%16 == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 5 | 2 / 循环 | <code>初始 int g=0;；继续条件 g &lt; WordPerThread/2；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 3 / 准备局部状态 | <code>int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 3 / 条件判断 | <code>(!flagThread &#124;&#124; g%2==0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 5 / 条件判断 | <code>(ix*EltPer16B &lt; eltN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 7 / 调用 / 计算 / 写入 | <code>load128((uint64_t*)(src + ix*EltPer16B), regs[2*g+0], regs[2*g+1]);</code> | 一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。 |
| 12 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 14 | 2 / 准备局部状态 | <code>int misalignment = reinterpret_cast&lt;uintptr_t&gt;(src) % 16;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 2 / 准备局部状态 | <code>uint64_t *src8 = reinterpret_cast&lt;uint64_t*&gt;(reinterpret_cast&lt;uintptr_t&gt;(src) & -uintptr_t(16));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 2 / 准备局部状态 | <code>uint64_t *shm8 = shmemCvtPtr(ncclShmem.ll128warp[warp]);</code> | 把 CUDA 通用指针转换为 PTX shared 地址空间指针，供共享内存汇编指令使用。 |
| 17 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 18 | 2 / 循环 | <code>初始 int g=0;；继续条件 g &lt; WordPerThread/2；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 3 / 条件判断 | <code>((g*WARP_SIZE + wid)*16 &lt; misalignment + eltN*sizeof(T))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 5 / 调用 / 计算 / 写入 | <code>load128(src8 + 2*(g*WARP_SIZE + wid), regs[2*g+0], regs[2*g+1]);</code> | 一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。 |
| 22 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 23 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 24 | 2 / 循环 | <code>初始 int g=0;；继续条件 g &lt; WordPerThread/2；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 25 | 3 / 调用 / 计算 / 写入 | <code>storeShmem128(shm8 + 2*(g*WARP_SIZE + wid), regs[2*g+0], regs[2*g+1]);</code> | 向 SM 共享内存写出 16 字节，供同 block 的线程在屏障后消费。 |
| 26 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>__syncwarp();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 28 | 2 / 准备局部状态 | <code>T *shm = (T*)shm8 + misalignment/sizeof(T);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 30 | 2 / 循环 | <code>初始 int g=0;；继续条件 g &lt; WordPerThread/2；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 31 | 3 / 准备局部状态 | <code>int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 3 / 条件判断 | <code>(!flagThread &#124;&#124; g%2==0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 5 / 条件判断 | <code>(ix*EltPer16B &lt; eltN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 7 / 调用 / 计算 / 写入 | <code>loadShmemMisaligned128(shm + ix*EltPer16B, regs[2*g+0], regs[2*g+1]);</code> | 按元素宽度拼接可能不对齐的共享内存数据到两个 64 位寄存器。 |
| 37 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    constexpr int EltPer16B = 16/sizeof(T);
    if(reinterpret_cast<uintptr_t>(src)%16 == 0) {
      /* We are aligned to 16 bytes, so load directly to registers no shmem.
       * Flag threads load half as much data which gets shuffled to the even
       * registers during Finish. The point of splitting into two phases is to
       * defer that shuffle, which incurs a dependency stall, until after other
       * memops are launched by the caller.
       */
      #pragma unroll
      for(int g=0; g < WordPerThread/2; g++) {
        int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);
        if(!flagThread || g%2==0) {
          if(ix*EltPer16B < eltN)
            load128((uint64_t*)(src + ix*EltPer16B), regs[2*g+0], regs[2*g+1]);
        }
      }
    }
    else {
      // Not aligned. Stage the smallest 16 byte aligned region subsuming the
      // buffer into shmem.
      int misalignment = reinterpret_cast<uintptr_t>(src) % 16;
      uint64_t *src8 = reinterpret_cast<uint64_t*>(reinterpret_cast<uintptr_t>(src) & -uintptr_t(16));
      uint64_t *shm8 = shmemCvtPtr(ncclShmem.ll128warp[warp]);
      #pragma unroll
      for(int g=0; g < WordPerThread/2; g++)
        if((g*WARP_SIZE + wid)*16 < misalignment + eltN*sizeof(T))
          load128(src8 + 2*(g*WARP_SIZE + wid), regs[2*g+0], regs[2*g+1]);
      #pragma unroll
      for(int g=0; g < WordPerThread/2; g++)
        storeShmem128(shm8 + 2*(g*WARP_SIZE + wid), regs[2*g+0], regs[2*g+1]);

      __syncwarp();

      // Now load from shmem stage to regs. Preserve the same pre-shuffled layout
      // as the aligned case since Finish() will be applied regardless.
      T *shm = (T*)shm8 + misalignment/sizeof(T);
      #pragma unroll
      for(int g=0; g < WordPerThread/2; g++) {
        int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);
        if(!flagThread || g%2==0) {
          if(ix*EltPer16B < eltN)
            loadShmemMisaligned128(shm + ix*EltPer16B, regs[2*g+0], regs[2*g+1]);
        }
      }
    }
  }
```

</details>

<a id="f0207"></a>

## F0207：loadRegsFinish

用途：完成 LL128 寄存器重排与共享内存辅助加载，使数据适合带 flag 的行布局。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 0 / 循环 | <code>初始 int g=1;；继续条件 g &lt; WordPerThread/2；每轮更新 g+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(flagThread)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>regs[2*g] = regs[2*g-1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // Move data out of flag registers into the vacant registers.
    #pragma unroll
    for (int g=1; g < WordPerThread/2; g+=2) {
      if (flagThread) regs[2*g] = regs[2*g-1];
    }
  }
```

</details>

<a id="f0208"></a>

## F0208：storeRegs

用途：把寄存器中的 LL128 数据还原写入用户缓冲，处理对齐与有效元素范围。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>constexpr int EltPer16B = 16/sizeof(T);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 3 | 0 / 循环 | <code>初始 int g=1;；继续条件 g &lt; WordPerThread/2；每轮更新 g+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(flagThread)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>regs[2*g-1] = regs[2*g];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 准备局部状态 | <code>int misalignment = reinterpret_cast&lt;uintptr_t&gt;(dst)%16;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>uint64_t *shm8 = shmemCvtPtr(ncclShmem.ll128warp[warp]);</code> | 把 CUDA 通用指针转换为 PTX shared 地址空间指针，供共享内存汇编指令使用。 |
| 10 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 11 | 0 / 循环 | <code>初始 int g=0;；继续条件 g &lt; WordPerThread/2；每轮更新 g++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 准备局部状态 | <code>int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 条件判断 | <code>(!flagThread &#124;&#124; g%2==0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 3 / 条件判断 | <code>(misalignment == 0 && (ix+1)*EltPer16B &lt;= eltN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 5 / 调用 / 计算 / 写入 | <code>store128((uint64_t*)(dst + ix*EltPer16B), regs[2*g+0], regs[2*g+1]);</code> | 把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。 |
| 18 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 5 / 调用 / 计算 / 写入 | <code>storeShmem128(shm8+2*ix, regs[2*g+0], regs[2*g+1]);</code> | 向 SM 共享内存写出 16 字节，供同 block 的线程在屏障后消费。 |
| 20 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>__syncwarp();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 22 | 0 / 准备局部状态 | <code>T *shm = (T*)ncclShmem.ll128warp[warp];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 0 / 准备局部状态 | <code>int skip = misalignment == 0 ? eltN & -EltPer16B : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 0 / 循环 | <code>初始 int i=skip+wid;；继续条件 i &lt; eltN；每轮更新 i += WARP_SIZE</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>dst[i] = shm[i];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    constexpr int EltPer16B = 16/sizeof(T);
    // Reverse Finish() register permuatation.
    #pragma unroll
    for (int g=1; g < WordPerThread/2; g+=2) {
      if (flagThread) regs[2*g-1] = regs[2*g];
    }
    // Write to dst if 16-byte aligned, shmem otherwise.
    int misalignment = reinterpret_cast<uintptr_t>(dst)%16;
    uint64_t *shm8 = shmemCvtPtr(ncclShmem.ll128warp[warp]);
    #pragma unroll
    for(int g=0; g < WordPerThread/2; g++) {
      int ix = g*WARP_SIZE - 4*(g/2) + wid - (g%2)*(wid/8);
      if (!flagThread || g%2==0) {
        if(misalignment == 0 && (ix+1)*EltPer16B <= eltN)
          store128((uint64_t*)(dst + ix*EltPer16B), regs[2*g+0], regs[2*g+1]);
        else
          storeShmem128(shm8+2*ix, regs[2*g+0], regs[2*g+1]);
      }
    }
    __syncwarp();
    // Write rest from shmem to dst. No need to coalesce stores to 16-bytes,
    // the hardware keeps up fine.
    T *shm = (T*)ncclShmem.ll128warp[warp];
    int skip = misalignment == 0 ? eltN & -EltPer16B : 0;
    for(int i=skip+wid; i < eltN; i += WARP_SIZE)
      dst[i] = shm[i];
  }
```

</details>

<a id="f0209"></a>

## F0209：recvReduceSendCopy

用途：实现 LL128 的一轮接收、寄存器归约、行内标记发送和可选本地保存。

- 负责flag的lane检查行尾序号，经any_sync决定整个warp是否重读。
- 等待首邻居期间本地加载已提前开始，之后Finish重排并对原始输入做preOp。
- 逐个接收其他邻居归约，最终postOp后把数据连同行尾flag发给下游，寄存器值再供本地store使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>constexpr int SRC = SrcBuf != -1 ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>uint64_t vr[ELEMS_PER_THREAD];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>__syncwarp();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 4 | 0 / 条件判断 | <code>(RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 准备局部状态 | <code>uint64_t* ptr = recvPtr(0)+ll128Offset;</code> | 以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。 |
| 7 | 2 / 准备局部状态 | <code>uint64_t flag = recvFlag(0);</code> | 把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。 |
| 8 | 2 / 准备局部状态 | <code>bool needReload;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 先执行一次的循环 | <code>初始 ；继续条件 (__any_sync(WARP_MASK, needReload) && checkAbort(spins, 0, 0) == 0)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>needReload = false;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 13 | 3 / 循环 | <code>初始 int u=0;；继续条件 u&lt;ELEMS_PER_THREAD；每轮更新 u+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>load128(ptr+u*WARP_SIZE, vr[u], vr[u+1]);</code> | 一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>needReload &#124;= flagThread && (vr[u+1] != flag);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 17 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 条件判断 | <code>(SRC)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>loadRegsFinish(v);</code> | 完成 LL128 寄存器重排与共享内存辅助加载，使数据适合带 flag 的行布局。 |
| 21 | 2 / 条件判断 | <code>(SrcBuf == Input)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 24 | 4 / 循环 | <code>初始 int u=0;；继续条件 u&lt;ELEMS_PER_THREAD；每轮更新 u+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 25 | 5 / 调用 / 计算 / 写入 | <code>v[u] = MULTI&lt;RedOp, T&gt;().preOp(redOp, v[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 5 / 条件判断 | <code>(!flagThread)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 7 / 调用 / 计算 / 写入 | <code>v[u+1] = MULTI&lt;RedOp, T&gt;().preOp(redOp, v[u+1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 30 | 0 / 条件判断 | <code>(RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 2 / 准备局部状态 | <code>uint64_t* ptr = recvPtr(0)+ll128Offset;</code> | 以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。 |
| 33 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 34 | 2 / 循环 | <code>初始 int u=0;；继续条件 u&lt;ELEMS_PER_THREAD；每轮更新 u+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>v[u] = SRC ? MULTI&lt;RedOp, T&gt;()(redOp, vr[u], v[u]) : vr[u];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 3 / 调用 / 计算 / 写入 | <code>v[u+1] = SRC ? MULTI&lt;RedOp, T&gt;()(redOp, vr[u+1], v[u+1]) : vr[u+1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 2 / 循环 | <code>初始 int i=1;；继续条件 i&lt;MaxRecv && i&lt;fan.nrecv()；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 3 / 准备局部状态 | <code>uint64_t flag = recvFlag(i);</code> | 把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。 |
| 40 | 3 / 准备局部状态 | <code>uint64_t* ptr = recvPtr(i)+ll128Offset;</code> | 以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。 |
| 41 | 3 / 准备局部状态 | <code>bool needReload;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 3 / 准备局部状态 | <code>int spins = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 3 / 先执行一次的循环 | <code>初始 ；继续条件 (__any_sync(WARP_MASK, needReload) && checkAbort(spins, i, 0) == 0)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 44 | 4 / 调用 / 计算 / 写入 | <code>needReload = false;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 4 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 46 | 4 / 循环 | <code>初始 int u=0;；继续条件 u&lt;ELEMS_PER_THREAD；每轮更新 u+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 47 | 5 / 调用 / 计算 / 写入 | <code>load128(ptr+u*WARP_SIZE, vr[u], vr[u+1]);</code> | 一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。 |
| 48 | 5 / 调用 / 计算 / 写入 | <code>needReload &#124;= flagThread && (vr[u+1] != flag);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 50 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 51 | 3 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 52 | 3 / 循环 | <code>初始 int u=0;；继续条件 u&lt;ELEMS_PER_THREAD；每轮更新 u+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 53 | 4 / 调用 / 计算 / 写入 | <code>v[u] = MULTI&lt;RedOp, T&gt;()(redOp, vr[u], v[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 4 / 调用 / 计算 / 写入 | <code>v[u+1] = MULTI&lt;RedOp, T&gt;()(redOp, vr[u+1], v[u+1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 56 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 57 | 0 / 条件判断 | <code>(postOp && !FuncTraits&lt;RedOp&gt;::IsPostOpIdentity)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 58 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 59 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 60 | 2 / 循环 | <code>初始 int u=0;；继续条件 u&lt;ELEMS_PER_THREAD；每轮更新 u+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 61 | 3 / 调用 / 计算 / 写入 | <code>v[u] = MULTI&lt;RedOp, T&gt;().postOp(redOp, v[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 3 / 调用 / 计算 / 写入 | <code>v[u+1] = MULTI&lt;RedOp, T&gt;().postOp(redOp, v[u+1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 63 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 64 | 0 / 条件判断 | <code>(SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 65 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 66 | 2 / 循环 | <code>初始 int i=1;；继续条件 i&lt;MaxSend && i&lt;fan.nsend()；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 67 | 3 / 准备局部状态 | <code>uint64_t flag = sendFlag(i);</code> | 把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。 |
| 68 | 3 / 准备局部状态 | <code>uint64_t* ptr = sendPtr(i)+ll128Offset;</code> | 以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。 |
| 69 | 3 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 70 | 3 / 循环 | <code>初始 int u=0;；继续条件 u&lt;ELEMS_PER_THREAD；每轮更新 u+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 71 | 4 / 调用 / 计算 / 写入 | <code>store128(ptr+u*WARP_SIZE, v[u], flagThread ? flag : v[u+1]);</code> | 把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。 |
| 72 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 73 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 74 | 2 / 准备局部状态 | <code>uint64_t flag = sendFlag(0);</code> | 把当前 step 加一映射为本轮协议行的期望标记，用序号区分复用槽中的新旧数据。 |
| 75 | 2 / 准备局部状态 | <code>uint64_t* ptr = sendPtr(0)+ll128Offset;</code> | 以协议缓冲基址加当前槽位偏移得到本轮读/写地址，不分配新内存。 |
| 76 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 77 | 2 / 循环 | <code>初始 int u=0;；继续条件 u&lt;ELEMS_PER_THREAD；每轮更新 u+=2</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 78 | 3 / 调用 / 计算 / 写入 | <code>store128(ptr+u*WARP_SIZE, v[u], flagThread ? flag : v[u+1]);</code> | 把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。 |
| 79 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    constexpr int SRC = SrcBuf != -1 ? 1 : 0;
    uint64_t vr[ELEMS_PER_THREAD];

    __syncwarp();
    /************************ Wait first recv ********************/
    if (RECV) {
      uint64_t* ptr = recvPtr(0)+ll128Offset;
      uint64_t flag = recvFlag(0);
      bool needReload;
      int spins = 0;
      do {
        needReload = false;
        #pragma unroll
        for (int u=0; u<ELEMS_PER_THREAD; u+=2) {
          load128(ptr+u*WARP_SIZE, vr[u], vr[u+1]);
          needReload |= flagThread && (vr[u+1] != flag);
        }
      } while (__any_sync(WARP_MASK, needReload) && checkAbort(spins, 0, 0) == 0);
    }

    /************* Finish register load **************/
    if (SRC) {
      // By deferring register shuffle here we've overlapped spinning on first
      // peer's data with memory loads of src data.
      loadRegsFinish(v);
      if (SrcBuf == Input) {
        #pragma unroll
        for (int u=0; u<ELEMS_PER_THREAD; u+=2) {
          v[u] = MULTI<RedOp, T>().preOp(redOp, v[u]);
          if (!flagThread)
            v[u+1] = MULTI<RedOp, T>().preOp(redOp, v[u+1]);
        }
      }
    }

    /************************ Recv rest *********************/
    if (RECV) {
      { // Consume data from first recv
        uint64_t* ptr = recvPtr(0)+ll128Offset;
        #pragma unroll
        for (int u=0; u<ELEMS_PER_THREAD; u+=2) {
          v[u] = SRC ? MULTI<RedOp, T>()(redOp, vr[u], v[u]) : vr[u];
          v[u+1] = SRC ? MULTI<RedOp, T>()(redOp, vr[u+1], v[u+1]) : vr[u+1];
        }
      }

      for (int i=1; i<MaxRecv && i<fan.nrecv(); i++) {
        uint64_t flag = recvFlag(i);
        uint64_t* ptr = recvPtr(i)+ll128Offset;
        bool needReload;
        int spins = 0;
        do {
          needReload = false;
          #pragma unroll
          for (int u=0; u<ELEMS_PER_THREAD; u+=2) {
            load128(ptr+u*WARP_SIZE, vr[u], vr[u+1]);
            needReload |= flagThread && (vr[u+1] != flag);
          }
        } while (__any_sync(WARP_MASK, needReload) && checkAbort(spins, i, 0) == 0);

        #pragma unroll
        for (int u=0; u<ELEMS_PER_THREAD; u+=2) {
          v[u] = MULTI<RedOp, T>()(redOp, vr[u], v[u]);
          v[u+1] = MULTI<RedOp, T>()(redOp, vr[u+1], v[u+1]);
        }
      }
    }
    /********************** End Recv ************************/

    if (postOp && !FuncTraits<RedOp>::IsPostOpIdentity) {
      #pragma unroll
      for (int u=0; u<ELEMS_PER_THREAD; u+=2) {
        v[u]   = MULTI<RedOp, T>().postOp(redOp, v[u]);
        v[u+1] = MULTI<RedOp, T>().postOp(redOp, v[u+1]);
      }
    }

    /************************ Send **************************/
    if (SEND) {
      for (int i=1; i<MaxSend && i<fan.nsend(); i++) {
        uint64_t flag = sendFlag(i);
        uint64_t* ptr = sendPtr(i)+ll128Offset;
        #pragma unroll
        for (int u=0; u<ELEMS_PER_THREAD; u+=2) {
          store128(ptr+u*WARP_SIZE, v[u], flagThread ? flag : v[u+1]);
        }
      }
      uint64_t flag = sendFlag(0);
      uint64_t* ptr = sendPtr(0)+ll128Offset;
      #pragma unroll
      for (int u=0; u<ELEMS_PER_THREAD; u+=2) {
        store128(ptr+u*WARP_SIZE, v[u], flagThread ? flag : v[u+1]);
      }
    }
    /********************** End Send ************************/
  }
```

</details>

<a id="f0210"></a>

## F0210：GenericOp

用途：根据本地源/目的和收发模板参数组织 LL128 处理循环，决定何时加载、归约、复制及发布。

- 先获取发送空间并barrier，按warp计算wireOffset和本地元素范围。
- 每轮Begin预取本地寄存器、收/归约/发，再按DST写用户结果，地址按全部warp跨度前进。
- 末尾barrier后推进收发step，再分别postSend/postRecv；不能套用Simple的逐slice系统fence图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>constexpr int SRC = SrcBuf != -1 ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>constexpr int DST = DstBuf != -1 ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 源码语法片段 | <code>static_assert(-1&lt;=SrcBuf && SrcBuf &lt; 2, "Uhoh");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 4 | 0 / 源码语法片段 | <code>static_assert(-1&lt;=DstBuf && DstBuf &lt; 2, "Uhoh");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 5 | 0 / 源码语法片段 | <code>static_assert(DstBuf!=Input, "Mistake?");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 6 | 0 / 编译期分支/指令 | <code>#if 0</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>assert((SrcBuf==-1) == (srcIx==-1));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>assert((DstBuf==-1) == (dstIx==-1));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 准备局部状态 | <code>T const *srcPtr = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>T *dstPtr = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>int wireOffset = WireWordPerSlice*warp + 2*wid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>const int nwarps = nthreads/WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>nelem = nelem &lt; 0 ? 0 : nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 条件判断 | <code>(SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>waitSend(divUp(nelem, DataEltPerSlice)*WireWordPerSlice*sizeof(uint64_t));</code> | 等待发送缓冲仍有可复用槽位，并在存在 Proxy 时填写本轮物理字节数，防止覆盖 NIC/对端仍使用的数据。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>nelem -= DataEltPerSlice*warp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>srcPtr += DataEltPerSlice*warp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>dstPtr += DataEltPerSlice*warp;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 循环 | <code>初始 ；继续条件 (nelem &gt; 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 1 / 准备局部状态 | <code>const int eltInSlice = min(nelem, DataEltPerSlice);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 23 | 1 / 准备局部状态 | <code>uint64_t regs[NCCL_LL128_SHMEM_ELEMS_PER_THREAD];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 1 / 条件判断 | <code>(SRC)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 3 / 调用 / 计算 / 写入 | <code>loadRegsBegin(regs, srcPtr, eltInSlice);</code> | 按对齐情况提前把用户数据加载进 LL128 寄存器，延后 warp 重排以隐藏访存延迟。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>recvReduceSendCopy&lt;NCCL_LL128_SHMEM_ELEMS_PER_THREAD, RECV, SEND, SrcBuf, DstBuf&gt;(regs, wireOffset, postOp);</code> | 实现 LL128 的一轮接收、寄存器归约、行内标记发送和可选本地保存。 |
| 28 | 1 / 条件判断 | <code>(DST)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>storeRegs(dstPtr, regs, eltInSlice);</code> | 把寄存器中的 LL128 数据还原写入用户缓冲，处理对齐与有效元素范围。 |
| 31 | 1 / 调用 / 计算 / 写入 | <code>wireOffset += WireWordPerSlice*nwarps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 1 / 调用 / 计算 / 写入 | <code>srcPtr += DataEltPerSlice*nwarps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 1 / 调用 / 计算 / 写入 | <code>dstPtr += DataEltPerSlice*nwarps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 1 / 调用 / 计算 / 写入 | <code>nelem -= DataEltPerSlice*nwarps;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>barrier();</code> | 同步本原语全部参与线程，确保数据搬运和进度发布阶段保持一致，命名 barrier 由 group 区分。 |
| 37 | 0 / 条件判断 | <code>(SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 2 / 循环 | <code>初始 int i=0;；继续条件 i &lt; MaxSend；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 40 | 3 / 调用 / 计算 / 写入 | <code>sendStep[i] += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 42 | 0 / 条件判断 | <code>(SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>postSend();</code> | 在数据/flag 写入完成并满足协议 fence 条件后发布发送进度，通知后续消费者。 |
| 45 | 0 / 条件判断 | <code>(RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 2 / 循环 | <code>初始 int i=0;；继续条件 i &lt; MaxRecv；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 48 | 3 / 调用 / 计算 / 写入 | <code>recvStep[i] += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 50 | 0 / 条件判断 | <code>(RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 2 / 调用 / 计算 / 写入 | <code>postRecv();</code> | 在数据已消费后发布接收完成进度，通知生产者可以回收相应循环槽位。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    constexpr int SRC = SrcBuf != -1 ? 1 : 0;
    constexpr int DST = DstBuf != -1 ? 1 : 0;
    static_assert(-1<=SrcBuf && SrcBuf < 2, "Uhoh");
    static_assert(-1<=DstBuf && DstBuf < 2, "Uhoh");
    static_assert(DstBuf!=Input, "Mistake?");
    #if 0
    assert((SrcBuf==-1) == (srcIx==-1));
    assert((DstBuf==-1) == (dstIx==-1));
    #endif

    T const *srcPtr = SrcBuf == -1 ? nullptr : userBufs[SrcBuf] + srcIx;
    T       *dstPtr = DstBuf == -1 ? nullptr : userBufs[DstBuf] + dstIx;
    int wireOffset = WireWordPerSlice*warp + 2*wid;
    const int nwarps = nthreads/WARP_SIZE;
    nelem = nelem < 0 ? 0 : nelem;

    if (SEND) waitSend(divUp(nelem, DataEltPerSlice)*WireWordPerSlice*sizeof(uint64_t));
    barrier();
    nelem -= DataEltPerSlice*warp;
    srcPtr += DataEltPerSlice*warp;
    dstPtr += DataEltPerSlice*warp;
    while (nelem > 0) {
      const int eltInSlice = min(nelem, DataEltPerSlice);
      uint64_t regs[NCCL_LL128_SHMEM_ELEMS_PER_THREAD];
      if (SRC) loadRegsBegin(regs, srcPtr, eltInSlice);
      recvReduceSendCopy<NCCL_LL128_SHMEM_ELEMS_PER_THREAD, RECV, SEND, SrcBuf, DstBuf>(regs, wireOffset, postOp);
      if (DST) storeRegs(dstPtr, regs, eltInSlice);

      wireOffset += WireWordPerSlice*nwarps;
      srcPtr += DataEltPerSlice*nwarps;
      dstPtr += DataEltPerSlice*nwarps;
      nelem -= DataEltPerSlice*nwarps;
    }

    barrier();
    if (SEND) for (int i=0; i < MaxSend; i++) sendStep[i] += 1;
    if (SEND) postSend();
    if (RECV) for (int i=0; i < MaxRecv; i++) recvStep[i] += 1;
    if (RECV) postRecv();
  }
```

</details>

<a id="f0211"></a>

## F0211：loadRecvConn

用途：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>recvBuff[i] = (uint64_t*)conn-&gt;buffs[NCCL_PROTO_LL128];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>recvStep[i] = conn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 3 | 0 / 条件判断 | <code>(wid == i)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>recvConn = conn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    recvBuff[i] = (uint64_t*)conn->buffs[NCCL_PROTO_LL128];
    recvStep[i] = conn->step;
    if (wid == i) recvConn = conn;
  }
```

</details>

<a id="f0212"></a>

## F0212：loadRecvSync

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

<a id="f0213"></a>

## F0213：loadSendConn

用途：从指定 peer/连接索引读取原语所需缓冲、进度和 direct/shared 能力，建立线程局部连接视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>sendBuff[i] = (uint64_t*)conn-&gt;buffs[NCCL_PROTO_LL128];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>sendStep[i] = conn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 3 | 0 / 条件判断 | <code>(wid == i)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>sendConn = conn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    sendBuff[i] = (uint64_t*)conn->buffs[NCCL_PROTO_LL128];
    sendStep[i] = conn->step;
    if (wid == i) sendConn = conn;
  }
```

</details>

<a id="f0214"></a>

## F0214：loadSendSync

用途：为负责同步的线程装入 head/tail 等进度地址和初始计数，恢复跨操作持续的 step。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(tid &lt; fan.nsend())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>sendConnHeadPtr = sendConn-&gt;head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>sendConnHeadCache = *sendConnHeadPtr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>sendConnHead = sendConn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>sendConnFifoPtr = sendConn-&gt;sizesFifo;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 条件判断 | <code>(tid &gt;= nthreads-WARP_SIZE && wid&lt;fan.nsend())</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 条件判断 | <code>(sendConn-&gt;sizesFifo)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 调用 / 计算 / 写入 | <code>sendConnTailPtr = sendConn-&gt;tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>sendConnTail = sendConn-&gt;step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |

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
    if (tid >= nthreads-WARP_SIZE && wid<fan.nsend()) {
      if (sendConn->sizesFifo) {
        sendConnTailPtr = sendConn->tail;
        sendConnTail = sendConn->step;
      }
    }
  }
```

</details>

<a id="f0215"></a>

## F0215：Primitives

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

<a id="f0216"></a>

## F0216：~Primitives

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

<a id="f0217"></a>

## F0217：setDataPtrs

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

<a id="f0218"></a>

## F0218：moveDataPtrs

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

<a id="f0219"></a>

## F0219：send

用途：从本地输入读取并发送到原语的发送邻居，不要求写本地输出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return GenericOp&lt;0, 1, Input, -1&gt;(inpIx, -1, eltN, false);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return GenericOp<0, 1, Input, -1>(inpIx, -1, eltN, false);
  }
```

</details>

<a id="f0220"></a>

## F0220：sendFromOutput

用途：从已生成的本地输出读数据并发送，避免错误地重新读取原始输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return GenericOp&lt;0, 1, Output, -1&gt;(outIx, -1, eltN, false);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return GenericOp<0, 1, Output, -1>(outIx, -1, eltN, false);
  }
```

</details>

<a id="f0221"></a>

## F0221：recv

用途：接收上游数据并写本地输出，本动作不加上本地输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return GenericOp&lt;1, 0, -1, Output&gt;(-1, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return GenericOp<1, 0, -1, Output>(-1, outIx, eltN, postOp);
  }
```

</details>

<a id="f0222"></a>

## F0222：recvReduceSend

用途：把上游部分结果与本地输入归约后发下游，不要求保留本地最终输出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return GenericOp&lt;1, 1, Input, -1&gt;(inpIx, -1, eltN, false);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return GenericOp<1, 1, Input, -1>(inpIx, -1, eltN, false);
  }
```

</details>

<a id="f0223"></a>

## F0223：recvReduceCopy

用途：把接收数据与本地输入归约后写本地输出，可在结果完成时执行 postOp。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return GenericOp&lt;1, 0, Input, Output&gt;(inpIx, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return GenericOp<1, 0, Input, Output>(inpIx, outIx, eltN, postOp);
  }
```

</details>

<a id="f0224"></a>

## F0224：copySend

用途：把本地输入同时写本地输出并发下游，把本地保存和转发合并一次数据加载。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return GenericOp&lt;0, 1, Input, Output&gt;(inpIx, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return GenericOp<0, 1, Input, Output>(inpIx, outIx, eltN, postOp);
  }
```

</details>

<a id="f0225"></a>

## F0225：recvCopySend

用途：把已接收结果同时保存本地输出并转发下游，不再加入本地输入做归约。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return GenericOp&lt;1, 1, -1, Output&gt;(-1, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return GenericOp<1, 1, -1, Output>(-1, outIx, eltN, postOp);
  }
```

</details>

<a id="f0226"></a>

## F0226：recvReduceCopySend

用途：把接收与本地输入融合归约，同时保存输出并转发，衔接归约与结果传播阶段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return GenericOp&lt;1, 1, Input, Output&gt;(inpIx, outIx, eltN, postOp);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return GenericOp<1, 1, Input, Output>(inpIx, outIx, eltN, postOp);
  }
```

</details>

