# src/collectives/device/reduce_scatter.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0370"></a>

## F0370：runRing

用途：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。

- size是每rank recvcount，不是整输入长度；输入共有P段，逐段读取同一channel偏移。
- 先发一个段，随后接收加本地同段再转发；最后把自己的完整段写到短输出的chunkOffset。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const int nthreads = args-&gt;header.nWarps*WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const int bid = args-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const int nChannels = args-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>ncclRing *ring = &ncclShmem.channel.ring;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int const *ringRanks = ring-&gt;devUserRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? REDUCESCATTER_CHUNKSTEPS : 1));</code> | 计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。 |
| 8 | 0 / 准备局部状态 | <code>const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2);</code> | 给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。 |
| 9 | 0 / 准备局部状态 | <code>const int nranks = ncclShmem.comm.nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>const ssize_t loopSize = nChannels*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>const ssize_t size = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanSymmetric&lt;1&gt;, 0, Proto, 0&gt; prims(tid, nthreads, &ring-&gt;prev, &ring-&gt;next, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 1 / 准备局部状态 | <code>ssize_t realChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 1 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));</code> | 把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。 |
| 19 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 20 | 3 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 5 / 调用 / 计算 / 写入 | <code>realChunkSize = size-gridOffset &lt; loopSize ? args-&gt;lastChunkSize : chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 24 | 5 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 7 / 调用 / 计算 / 写入 | <code>realChunkSize = min(divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128, chunkSize);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>realChunkSize = int(realChunkSize);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 1 / 准备局部状态 | <code>ssize_t chunkOffset = gridOffset + bid*int(realChunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 1 / 准备局部状态 | <code>ssize_t offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 1 / 准备局部状态 | <code>int nelem = min(realChunkSize, size-chunkOffset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 31 | 1 / 准备局部状态 | <code>int rankDest;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 1 / 调用 / 计算 / 写入 | <code>rankDest = ringRanks[nranks-1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 1 / 调用 / 计算 / 写入 | <code>offset = chunkOffset + rankDest * size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 1 / 调用 / 计算 / 写入 | <code>prims.send(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 1 / 循环 | <code>初始 int j=2;；继续条件 j&lt;nranks；每轮更新 ++j</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>rankDest = ringRanks[nranks-j];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>offset = chunkOffset + rankDest * size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 2 / 调用 / 计算 / 写入 | <code>prims.recvReduceSend(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 40 | 1 / 调用 / 计算 / 写入 | <code>rankDest = ringRanks[0];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 1 / 调用 / 计算 / 写入 | <code>offset = chunkOffset + rankDest * size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 1 / 调用 / 计算 / 写入 | <code>prims.recvReduceCopy(offset, chunkOffset, nelem, true);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclRing *ring = &ncclShmem.channel.ring;
    int const *ringRanks = ring->devUserRanks;
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? REDUCESCATTER_CHUNKSTEPS : 1));
    // We should not need the final /2 but it makes performance much, much smoother. Might be a bug somewhere.
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2);
    const int nranks = ncclShmem.comm.nRanks;
    const ssize_t loopSize = nChannels*chunkSize;
    const ssize_t size = args->count;

    Primitives<T, RedOp, FanSymmetric<1>, 0, Proto, 0>
      prims(tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);

    for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else if (Proto::Id == NCCL_PROTO_LL)
        realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;
      else if (Proto::Id == NCCL_PROTO_LL128)
        realChunkSize = min(divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128, chunkSize);
      realChunkSize = int(realChunkSize);

      ssize_t chunkOffset = gridOffset + bid*int(realChunkSize);

      /////////////// begin ReduceScatter steps ///////////////
      ssize_t offset;
      int nelem = min(realChunkSize, size-chunkOffset);
      int rankDest;

      // step 0: push data to next GPU
/* 中文精读：ReduceScatter 的输入输出下标
 * size 是 recvcount，即每个 rank 最终保留的 T 元素数，输入总长度 nranks*size。
 * 本 channel 处理所有 rank 段中相同的 chunkOffset；offset=用户rank*size+chunkOffset。
 * 先发最后一段，逐步接收归约并转发，最后把属于自己的完整结果写到 output[chunkOffset]。
 * 例：4 rank、recvcount=8 float，每人输入 32 float，rank2 最终得到全局求和后的 [16,24) 段，
 * 但放在自己的 8 元素输出缓冲 [0,8) 中。
 */
      rankDest = ringRanks[nranks-1];
      offset = chunkOffset + rankDest * size;
      prims.send(offset, nelem);

      // k-2 steps: reduce and copy to next GPU
      for (int j=2; j<nranks; ++j) {
        rankDest = ringRanks[nranks-j];
        offset = chunkOffset + rankDest * size;
        prims.recvReduceSend(offset, nelem);
      }

      // step k-1: reduce this buffer and data, which will produce the final result
      rankDest = ringRanks[0];
      offset = chunkOffset + rankDest * size;
      prims.recvReduceCopy(offset, chunkOffset, nelem, /*postOp=*/true);
    }
  }
```

</details>

<a id="f0371"></a>

## F0371：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>using Proto = ProtoSimple&lt;REDUCESCATTER_CHUNKSTEPS/REDUCESCATTER_SLICESTEPS, REDUCESCATTER_SLICESTEPS&gt;;</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>runRing&lt;T, RedOp, Proto&gt;(args);</code> | 执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    using Proto = ProtoSimple<REDUCESCATTER_CHUNKSTEPS/REDUCESCATTER_SLICESTEPS, REDUCESCATTER_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
```

</details>

<a id="f0372"></a>

## F0372：run

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

<a id="f0373"></a>

## F0373：run

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

