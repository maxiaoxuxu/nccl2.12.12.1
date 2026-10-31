# src/collectives/device/reduce.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0260"></a>

## F0260：runRing

用途：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。

- root后继先send本地输入，中间rank接收并归约转发，root最终接收归约后保留结果。
- root处停止，不再结果广播；只有root接收缓冲要求持有完整归约输出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const int nthreads = args-&gt;header.nWarps*WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const int bid = args-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const int nChannels = args-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>ncclRing *ring = &ncclShmem.channel.ring;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? REDUCE_CHUNKSTEPS : 1));</code> | 计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。 |
| 7 | 0 / 准备局部状态 | <code>const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T)));</code> | 给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。 |
| 8 | 0 / 准备局部状态 | <code>const int nranks = ncclShmem.comm.nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>const ssize_t loopSize = nChannels*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>const ssize_t size = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>const int rank = ncclShmem.comm.rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>const int prevRank = ring-&gt;devUserRanks[nranks-1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>const int root = args-&gt;root;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanSymmetric&lt;1&gt;, 0, Proto, 0&gt; prims(tid, nthreads, &ring-&gt;prev, &ring-&gt;next, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 准备局部状态 | <code>auto calcChunkSize = [&] (ssize_t gridOffset)-&gt;int { int realChunkSize; if (Proto::Id == NCCL_PROTO_SIMPLE) { realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels)); realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T)); } else if (Proto::Id == NCCL_PROTO_LL) realChunkSize = size-gridOffset &lt; loopSize ? args-&gt;lastChunkSize : chunkSize; else if (Proto::Id == NCCL_PROTO_LL128) realChunkSize = min(divUp(size-gridOffset, nChannels*minC …（完整代码见函数体）</code> | 定义并保存局部lambda对象；其body仅在后续调用时执行，不在声明处立即运行。 |
| 16 | 1 / 定义局部lambda | <code></code> | 此处建立可调用对象；以下body在以后调用该lambda时才执行，不是声明时立即执行。 |
| 17 | 2 / 准备局部状态 | <code>int realChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 2 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));</code> | 把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。 |
| 22 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 4 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 6 / 调用 / 计算 / 写入 | <code>realChunkSize = size-gridOffset &lt; loopSize ? args-&gt;lastChunkSize : chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 27 | 6 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 8 / 调用 / 计算 / 写入 | <code>realChunkSize = min(divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128, chunkSize);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 30 | 2 / 函数返回 | <code>return realChunkSize;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 31 | 0 / 条件判断 | <code>(prevRank == root)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 2 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 34 | 3 / 准备局部状态 | <code>int realChunkSize = calcChunkSize(gridOffset);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 3 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*realChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 36 | 3 / 准备局部状态 | <code>int nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 37 | 3 / 调用 / 计算 / 写入 | <code>prims.send(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 39 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 40 | 2 / 条件判断 | <code>(rank == root)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 4 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 43 | 5 / 准备局部状态 | <code>int realChunkSize = calcChunkSize(gridOffset);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 5 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*realChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 5 / 准备局部状态 | <code>int nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 46 | 5 / 调用 / 计算 / 写入 | <code>prims.recvReduceCopy(offset, offset, nelem, true);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 48 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 49 | 4 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 50 | 5 / 准备局部状态 | <code>int realChunkSize = calcChunkSize(gridOffset);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 51 | 5 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*realChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 5 / 准备局部状态 | <code>int nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 53 | 5 / 调用 / 计算 / 写入 | <code>prims.recvReduceSend(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 54 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclRing *ring = &ncclShmem.channel.ring;
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? REDUCE_CHUNKSTEPS : 1));
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T)));
    const int nranks = ncclShmem.comm.nRanks;
    const ssize_t loopSize = nChannels*chunkSize;
    const ssize_t size = args->count;
    const int rank = ncclShmem.comm.rank;
    const int prevRank = ring->devUserRanks[nranks-1];
    const int root = args->root;

    Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>
      prims(tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);

    auto calcChunkSize = [&]__device__(ssize_t gridOffset)->int {
      int realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else if (Proto::Id == NCCL_PROTO_LL)
        realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;
      else if (Proto::Id == NCCL_PROTO_LL128)
        realChunkSize = min(divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128, chunkSize);
      return realChunkSize;
    };

/* 中文精读：Reduce 的流水线起点在 root 的后继
 * root 的后继没有上游需要合并的部分结果，因此先 send 本地输入。
 * 中间节点 recvReduceSend，将收到的部分结果加本地输入后向下游继续发。
 * root 最后 recvReduceCopy，把其余 rank 的归约结果与自己的输入合并，只有它需要最终输出。
 * 这里按 T 元素分片，count 是每 rank 输入长度；每一块都遵循同一条以 root 为终点的路径。
 * 与 AllReduce 不同，完成后没有再把完整结果广播给所有 rank 的阶段。
 */
    if (prevRank == root) {
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        int realChunkSize = calcChunkSize(gridOffset);
        ssize_t offset = gridOffset + bid*realChunkSize;
        int nelem = min(realChunkSize, size-offset);
        prims.send(offset, nelem);
      }
    }
    else if (rank == root) {
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        int realChunkSize = calcChunkSize(gridOffset);
        ssize_t offset = gridOffset + bid*realChunkSize;
        int nelem = min(realChunkSize, size-offset);
        prims.recvReduceCopy(offset, offset, nelem, /*postOp=*/true);
      }
    }
    else {
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        int realChunkSize = calcChunkSize(gridOffset);
        ssize_t offset = gridOffset + bid*realChunkSize;
        int nelem = min(realChunkSize, size-offset);
        prims.recvReduceSend(offset, nelem);
      }
    }
  }
```

</details>

<a id="f0261"></a>

## F0261：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>using Proto = ProtoSimple&lt;REDUCE_CHUNKSTEPS/REDUCE_SLICESTEPS, REDUCE_SLICESTEPS&gt;;</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>runRing&lt;T, RedOp, Proto&gt;(args);</code> | 执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    using Proto = ProtoSimple<REDUCE_CHUNKSTEPS/REDUCE_SLICESTEPS, REDUCE_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
```

</details>

<a id="f0262"></a>

## F0262：run

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

<a id="f0263"></a>

## F0263：run

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

