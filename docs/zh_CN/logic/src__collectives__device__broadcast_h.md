# src/collectives/device/broadcast.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0068"></a>

## F0068：runRing

用途：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。

- 按root、root前驱和其他rank分三种原语：send/copySend、recv、recvCopySend。
- 在root前驱切断回环边，避免重复传回root；不同slice仍能形成流水。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const int nthreads = args-&gt;header.nWarps*WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const int bid = args-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const int nChannels = args-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>ncclRing *ring = &ncclShmem.channel.ring;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? BROADCAST_CHUNKSTEPS : 1));</code> | 计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。 |
| 7 | 0 / 准备局部状态 | <code>const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T)));</code> | 给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。 |
| 8 | 0 / 准备局部状态 | <code>const ssize_t loopSize = nChannels*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>const ssize_t size = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>const int rank = ring-&gt;devUserRanks[0];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>const int nextRank = ring-&gt;devUserRanks[1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>const int root = args-&gt;root;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>T *inputBuf = (T*)args-&gt;sendbuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>T *outputBuf = (T*)args-&gt;recvbuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanSymmetric&lt;1&gt;, 0, Proto, 0&gt; prims(tid, nthreads, &ring-&gt;prev, &ring-&gt;next, inputBuf, outputBuf, args-&gt;redOpArg);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 1 / 准备局部状态 | <code>ssize_t realChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 1 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));</code> | 把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。 |
| 22 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 3 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 5 / 调用 / 计算 / 写入 | <code>realChunkSize = size-gridOffset &lt; loopSize ? args-&gt;lastChunkSize : chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 27 | 5 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 7 / 调用 / 计算 / 写入 | <code>realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 30 | 1 / 调用 / 计算 / 写入 | <code>realChunkSize = int(realChunkSize);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 1 / 准备局部状态 | <code>ssize_t offset = gridOffset + int(bid*realChunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 1 / 准备局部状态 | <code>int nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 33 | 1 / 条件判断 | <code>(rank == root)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 3 / 条件判断 | <code>(inputBuf == outputBuf)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 5 / 调用 / 计算 / 写入 | <code>prims.send(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 39 | 5 / 调用 / 计算 / 写入 | <code>prims.copySend(offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 41 | 3 / 条件判断 | <code>(nextRank == root)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 5 / 调用 / 计算 / 写入 | <code>prims.recv(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 45 | 5 / 调用 / 计算 / 写入 | <code>prims.recvCopySend(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclRing *ring = &ncclShmem.channel.ring;
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? BROADCAST_CHUNKSTEPS : 1));
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T)));
    const ssize_t loopSize = nChannels*chunkSize;
    const ssize_t size = args->count;
    const int rank = ring->devUserRanks[0];
    const int nextRank = ring->devUserRanks[1];
    const int root = args->root;

    T *inputBuf = (T*)args->sendbuff;
    T *outputBuf = (T*)args->recvbuff;
    Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>
      prims(tid, nthreads, &ring->prev, &ring->next, inputBuf, outputBuf, args->redOpArg);

    for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else if (Proto::Id == NCCL_PROTO_LL)
        realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;
      else if (Proto::Id == NCCL_PROTO_LL128)
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128);
      realChunkSize = int(realChunkSize);

      ssize_t offset = gridOffset + int(bid*realChunkSize);
      int nelem = min(realChunkSize, size-offset);

/* 中文精读：Broadcast 如何把环变成从 root 开始的流水线
 * 每个 channel 对相同张量不同区间并行；本 API 已归一化为 Int8，offset/nelem 用字节。
 * root 发送，若输入输出不同则顺便本地复制；中间 rank recvCopySend；root 的前驱仅 recv。
 * 这切断了最后一条回到 root 的边，防止重复发送和 root 等待并不需要的输入。
 * 它不是先全量发完再由下一个 rank 发：slice 级别的原语使各 rank 能流水转发。
 * 本文件提供的是 Ring 特化；看到调度器 pattern 有 TreeDown 不等于本文件实现了 Tree Broadcast。
 */
      if (rank == root) {
        if (inputBuf == outputBuf) {
          prims.send(offset, nelem);
        } else {
          prims.copySend(offset, offset, nelem);
        }
      } else if (nextRank == root) {
        prims.recv(offset, nelem);
      } else {
        prims.recvCopySend(offset, nelem);
      }
    }
  }
```

</details>

<a id="f0069"></a>

## F0069：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>using Proto = ProtoSimple&lt;BROADCAST_CHUNKSTEPS/BROADCAST_SLICESTEPS, BROADCAST_SLICESTEPS&gt;;</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>runRing&lt;T, RedOp, Proto&gt;(args);</code> | 执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    using Proto = ProtoSimple<BROADCAST_CHUNKSTEPS/BROADCAST_SLICESTEPS, BROADCAST_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
```

</details>

<a id="f0070"></a>

## F0070：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>runRing&lt;T, RedOp, ProtoLL&gt;(args);</code> | 执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    runRing<T, RedOp, ProtoLL>(args);
  }
```

</details>

<a id="f0071"></a>

## F0071：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>runRing&lt;T, RedOp, ProtoLL128&gt;(args);</code> | 执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    runRing<T, RedOp, ProtoLL128>(args);
  }
```

</details>

