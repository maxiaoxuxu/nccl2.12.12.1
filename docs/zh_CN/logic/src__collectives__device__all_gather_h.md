# src/collectives/device/all_gather.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0054"></a>

## F0054：runRing

用途：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。

- size代表本rank贡献长度；复制API已归一成字节，输入与输出偏移要按用户rank段解释。
- 先把本地贡献放到自己的输出段并发送；中间接收其他贡献后保留和转发，最后只接收。
- 全过程不执行跨rank求和，环顺序优化不改变公开输出按用户rank排列。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const int nthreads = args-&gt;header.nWarps*WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const int bid = args-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const int nChannels = args-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>ncclRing *ring = &ncclShmem.channel.ring;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>const int *ringRanks = ring-&gt;devUserRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLGATHER_CHUNKSTEPS : 1));</code> | 计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。 |
| 8 | 0 / 准备局部状态 | <code>const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2);</code> | 给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。 |
| 9 | 0 / 准备局部状态 | <code>const int nranks = ncclShmem.comm.nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>const ssize_t loopSize = nChannels*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>const ssize_t size = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>T *inputBuf = (T*)args-&gt;sendbuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>T *outputBuf = (T*)args-&gt;recvbuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanSymmetric&lt;1&gt;, 1, Proto, 0&gt; prims (tid, nthreads, &ring-&gt;prev, &ring-&gt;next, inputBuf, outputBuf, args-&gt;redOpArg);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 1 / 准备局部状态 | <code>ssize_t realChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 1 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = min(chunkSize, divUp(size-gridOffset,nChannels));</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));</code> | 把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。 |
| 21 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 22 | 3 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 5 / 调用 / 计算 / 写入 | <code>realChunkSize = size-gridOffset &lt; loopSize ? args-&gt;lastChunkSize : chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 26 | 5 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 7 / 调用 / 计算 / 写入 | <code>realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>realChunkSize = int(realChunkSize);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 1 / 准备局部状态 | <code>ssize_t chunkOffset = gridOffset + int(bid*realChunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 1 / 准备局部状态 | <code>ssize_t offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 1 / 准备局部状态 | <code>int nelem = min(realChunkSize, size-chunkOffset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 33 | 1 / 准备局部状态 | <code>int rankDest;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 1 / 调用 / 计算 / 写入 | <code>rankDest = ringRanks[0];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 1 / 调用 / 计算 / 写入 | <code>offset = chunkOffset + rankDest * size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 1 / 条件判断 | <code>(inputBuf + chunkOffset == outputBuf + offset)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 3 / 调用 / 计算 / 写入 | <code>prims.directSend(chunkOffset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 40 | 3 / 调用 / 计算 / 写入 | <code>prims.directCopySend(chunkOffset, offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 1 / 循环 | <code>初始 int j=1;；继续条件 j&lt;nranks-1；每轮更新 ++j</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 42 | 2 / 调用 / 计算 / 写入 | <code>rankDest = ringRanks[nranks-j];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 2 / 调用 / 计算 / 写入 | <code>offset = chunkOffset + rankDest * size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>prims.directRecvCopySend(offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 46 | 1 / 调用 / 计算 / 写入 | <code>rankDest = ringRanks[1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 1 / 调用 / 计算 / 写入 | <code>offset = chunkOffset + rankDest * size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 1 / 调用 / 计算 / 写入 | <code>prims.directRecv(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclRing *ring = &ncclShmem.channel.ring;
    const int *ringRanks = ring->devUserRanks;
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLGATHER_CHUNKSTEPS : 1));
    // We should not need the final /2 but it makes performance much, much smoother. Might be a bug somewhere.
    const ssize_t minChunkSizeLL128 = int(nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2);
    const int nranks = ncclShmem.comm.nRanks;
    const ssize_t loopSize = nChannels*int(chunkSize);
    const ssize_t size = args->count;

    T *inputBuf = (T*)args->sendbuff;
    T *outputBuf = (T*)args->recvbuff;
    Primitives<T, RedOp, FanSymmetric<1>, 1, Proto, 0> prims
      (tid, nthreads, &ring->prev, &ring->next, inputBuf, outputBuf, args->redOpArg);

    for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset,nChannels));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else if (Proto::Id == NCCL_PROTO_LL)
        realChunkSize = size-gridOffset < loopSize ? args->lastChunkSize : chunkSize;
      else if (Proto::Id == NCCL_PROTO_LL128)
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*minChunkSizeLL128)*minChunkSizeLL128);
      realChunkSize = int(realChunkSize);

      ssize_t chunkOffset = gridOffset + int(bid*realChunkSize);

      /////////////// begin AllGather steps ///////////////
      ssize_t offset;
      int nelem = min(realChunkSize, size-chunkOffset);
      int rankDest;

      // step 0: push data to next GPU
/* 中文精读：AllGather 的布局、分片与原地操作
 * size 是本 rank 贡献长度，输出总长度 nranks*size。该复制 API 已归一化成 Int8，故单位为字节。
 * channel 负责本地贡献的一段 chunkOffset，写入 output[用户rank*size + chunkOffset]。
 * 之后每步把上游其他用户 rank 的同一段保存并转发，最终每个 rank 都有完整有序输出。
 * 输入恰好等于自己在输出中的段时可直接发送；否则 copySend 兼顾本地写入和发下游。
 * 例：4 rank 每人 8B，rank2 的贡献最终总在 output[16,24)，不随环顺序改变。
 */
      rankDest = ringRanks[0];
      offset = chunkOffset + rankDest * size;

      if (inputBuf + chunkOffset == outputBuf + offset) { // In place
        prims.directSend(chunkOffset, offset, nelem);
      } else {
        prims.directCopySend(chunkOffset, offset, offset, nelem);
      }

      // k-2 steps: copy to next GPU
      for (int j=1; j<nranks-1; ++j) {
        rankDest = ringRanks[nranks-j];
        offset = chunkOffset + rankDest * size;

        prims.directRecvCopySend(offset, offset, nelem);
      }

      // Make final copy from buffer to dest.
      rankDest = ringRanks[1];
      offset = chunkOffset + rankDest * size;

      // Final wait/copy.
      prims.directRecv(offset, nelem);
    }
  }
```

</details>

<a id="f0055"></a>

## F0055：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>using Proto = ProtoSimple&lt;ALLGATHER_CHUNKSTEPS/ALLGATHER_SLICESTEPS, ALLGATHER_SLICESTEPS&gt;;</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>runRing&lt;T, RedOp, Proto&gt;(args);</code> | 执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    using Proto = ProtoSimple<ALLGATHER_CHUNKSTEPS/ALLGATHER_SLICESTEPS, ALLGATHER_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
```

</details>

<a id="f0056"></a>

## F0056：run

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

<a id="f0057"></a>

## F0057：run

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

