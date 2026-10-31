# src/collectives/device/all_reduce.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0058"></a>

## F0058：runRing

用途：执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。

- 固定一个channel后，外层gridOffset覆盖全输入；本轮真实块长按剩余量和协议grain调整。
- calcOffset对Simple使用channel连续布局，对LL/LL128使用交错布局；nelem截断尾部。
- 先发块(ringIx-1)，逐块接收归约，块ringIx完成时保存并直接开始结果传播。
- 后续仅复制完整结果；最后只recv收齐缺块。不能在AllGather阶段重复加本地输入。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const int nthreads = args-&gt;header.nWarps*WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const int bid = args-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const int nChannels = args-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>ncclRing *ring = &ncclShmem.channel.ring;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int ringIx = ring-&gt;index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLREDUCE_CHUNKSTEPS : 1));</code> | 计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。 |
| 8 | 0 / 准备局部状态 | <code>const int nranks = ncclShmem.comm.nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>const ssize_t loopSize = nChannels*nranks*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>const ssize_t size = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>int minChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T));</code> | 给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。 |
| 15 | 0 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_LL128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2;</code> | 给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。 |
| 18 | 0 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanSymmetric&lt;1&gt;, 1, Proto, 0&gt; prims (tid, nthreads, &ring-&gt;prev, &ring-&gt;next, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 0 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 20 | 1 / 准备局部状态 | <code>ssize_t realChunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 1 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks));</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 24 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));</code> | 把尺寸/偏移向上对齐到指定粒度，满足向量访存、页或协议槽位契约。 |
| 25 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 26 | 3 / 调用 / 计算 / 写入 | <code>realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks*minChunkSize)*minChunkSize);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>realChunkSize = int(realChunkSize);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 1 / 准备局部状态 | <code>auto calcOffset = [&] (int chunk)-&gt;ssize_t { if (Proto::Id == NCCL_PROTO_SIMPLE) return gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize; else return gridOffset + (chunk*nChannels + bid)*realChunkSize; };</code> | 定义并保存局部lambda对象；其body仅在后续调用时执行，不在声明处立即运行。 |
| 29 | 2 / 定义局部lambda | <code></code> | 此处建立可调用对象；以下body在以后调用该lambda时才执行，不是声明时立即执行。 |
| 30 | 3 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 5 / 函数返回 | <code>return gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 33 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 34 | 5 / 函数返回 | <code>return gridOffset + (chunk*nChannels + bid)*realChunkSize;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 35 | 1 / 准备局部状态 | <code>auto modRanks = [&] (int r)-&gt;int { return r - (r &gt;= nranks ? nranks : 0); };</code> | 定义并保存局部lambda对象；其body仅在后续调用时执行，不在声明处立即运行。 |
| 36 | 2 / 定义局部lambda | <code></code> | 此处建立可调用对象；以下body在以后调用该lambda时才执行，不是声明时立即执行。 |
| 37 | 3 / 函数返回 | <code>return r - (r &gt;= nranks ? nranks : 0);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 38 | 1 / 准备局部状态 | <code>ssize_t offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 39 | 1 / 准备局部状态 | <code>int nelem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 1 / 准备局部状态 | <code>int chunk;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 1 / 调用 / 计算 / 写入 | <code>chunk = modRanks(ringIx + nranks-1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 1 / 调用 / 计算 / 写入 | <code>offset = calcOffset(chunk);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 1 / 调用 / 计算 / 写入 | <code>nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 44 | 1 / 调用 / 计算 / 写入 | <code>prims.send(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 1 / 循环 | <code>初始 int j=2;；继续条件 j&lt;nranks；每轮更新 ++j</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 46 | 2 / 调用 / 计算 / 写入 | <code>chunk = modRanks(ringIx + nranks-j);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 2 / 调用 / 计算 / 写入 | <code>offset = calcOffset(chunk);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 2 / 调用 / 计算 / 写入 | <code>nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 49 | 2 / 调用 / 计算 / 写入 | <code>prims.recvReduceSend(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 50 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 51 | 1 / 调用 / 计算 / 写入 | <code>chunk = ringIx + 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 1 / 调用 / 计算 / 写入 | <code>offset = calcOffset(chunk);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 1 / 调用 / 计算 / 写入 | <code>nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 54 | 1 / 调用 / 计算 / 写入 | <code>prims.directRecvReduceCopySend(offset, offset, offset, nelem, true);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 55 | 1 / 循环 | <code>初始 int j=1;；继续条件 j&lt;nranks-1；每轮更新 ++j</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 56 | 2 / 调用 / 计算 / 写入 | <code>chunk = modRanks(ringIx + nranks-j);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 2 / 调用 / 计算 / 写入 | <code>offset = calcOffset(chunk);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 2 / 调用 / 计算 / 写入 | <code>nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 59 | 2 / 调用 / 计算 / 写入 | <code>prims.directRecvCopySend(offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 60 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 61 | 1 / 调用 / 计算 / 写入 | <code>chunk = modRanks(ringIx + 1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 1 / 调用 / 计算 / 写入 | <code>offset = calcOffset(chunk);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 63 | 1 / 调用 / 计算 / 写入 | <code>nelem = min(realChunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 64 | 1 / 调用 / 计算 / 写入 | <code>prims.directRecv(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 65 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclRing *ring = &ncclShmem.channel.ring;
    int ringIx = ring->index;
    const ssize_t chunkSize = int(Proto::calcBytePerStep()/sizeof(T) * (Proto::Id == NCCL_PROTO_SIMPLE ? ALLREDUCE_CHUNKSTEPS : 1));
    const int nranks = ncclShmem.comm.nRanks;
    const ssize_t loopSize = nChannels*nranks*chunkSize;
    const ssize_t size = args->count;

    int minChunkSize;
    if (Proto::Id == NCCL_PROTO_LL)
      minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T));
    if (Proto::Id == NCCL_PROTO_LL128) {
      // We should not need the final /2 but it makes performance much, much smoother. Might be a bug somewhere.
      minChunkSize = nthreads*(Proto::calcBytePerGrain()/sizeof(T))/2;
    }

    Primitives<T, RedOp, FanSymmetric<1>, 1, Proto, 0> prims
      (tid, nthreads, &ring->prev, &ring->next, args->sendbuff, args->recvbuff, args->redOpArg);

    for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
      ssize_t realChunkSize;
      if (Proto::Id == NCCL_PROTO_SIMPLE) {
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks));
        realChunkSize = roundUp(realChunkSize, (nthreads-WARP_SIZE)*sizeof(uint64_t)/sizeof(T));
      }
      else
        realChunkSize = min(chunkSize, divUp(size-gridOffset, nChannels*nranks*minChunkSize)*minChunkSize);
      realChunkSize = int(realChunkSize);

/* 中文精读：AllReduce 的地址公式与数值示例
 * 所有偏移以 T 元素计。Simple 使用 channel 连续布局：
 * offset=gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize。
 * 假设 4 rank、2 channel、nthreads=96、共 1024 个 float，且 chunkSize 至少为 128 元素。
 * 对齐粒度=(96-32)*8/4=128 元素，realChunkSize=ceil(1024/8)=128。
 * channel0 的 4 块覆盖 [0,512)，channel1 的 4 块覆盖 [512,1024)，块长都是 128 元素。
 * LL/LL128 用 (chunk*nChannels+bid)*realChunkSize 交错布局；不能把 Simple 公式套到所有协议。
 * 注意 realChunkSize 是对齐后的容量，不保证每个块都有这么多有效元素。
 */
      auto calcOffset = [&]__device__(int chunk)->ssize_t {
        if (Proto::Id == NCCL_PROTO_SIMPLE)
          return gridOffset + bid*nranks*realChunkSize + chunk*realChunkSize;
        else
          return gridOffset + (chunk*nChannels + bid)*realChunkSize;
      };
      auto modRanks = [&]__device__(int r)->int {
        return r - (r >= nranks ? nranks : 0);
      };

      ssize_t offset;
      int nelem;
      int chunk;

/* 中文导读：
 * 本轮先发送第一个块，再逐块把上游数据与本地输入归约后继续传递。
 * 各 rank 处理不同块，形成流水线；不是每一轮都发送整个输入数组。
 */
      // step 0: push data to next GPU
      chunk = modRanks(ringIx + nranks-1);
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.send(offset, nelem);

      // k-2 steps: reduce and copy to next GPU
      for (int j=2; j<nranks; ++j) {
        chunk = modRanks(ringIx + nranks-j);
        offset = calcOffset(chunk);
        nelem = min(realChunkSize, size-offset);
        prims.recvReduceSend(offset, nelem);
      }

      // step k-1: reduce this buffer and data, which will produce the final
      // result that we store in this data and push to the next GPU
/* 中文精读：归约分散与结果收集在这一调用衔接
 * 最初发送 (ringIx-1) 块，再依次处理前面的块；收到本 rank 的 ringIx 块时，
 * 它已经包含其余 rank 的部分结果，加上本地输入后才成为完整归约结果。
 * directRecvReduceCopySend 同时把最终值写本地 output 并发下游，启动结果收集阶段。
 * 对于 4 rank/单 channel，rank0 依次 send 块3，reduceSend 块2、块1，最终归约块0；
 * 再 copySend 块3、块2，最后 recv 块1。实际环顺序改变时，ringIx 的对应用户 rank 也改变。
 */
      chunk = ringIx + 0;
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.directRecvReduceCopySend(offset, offset, offset, nelem, /*postOp=*/true);

/* 中文导读：
 * 从这里进入结果传播阶段：继续转发已经完成归约的块，不再对这些块重复做全局求和。
 * 最后 directRecv 收齐本 rank 缺少的最后一个块，整段完成 ReduceScatter+AllGather 语义。
 */
      // k-2 steps: copy to next GPU
      for (int j=1; j<nranks-1; ++j) {
        chunk = modRanks(ringIx + nranks-j);
        offset = calcOffset(chunk);
        nelem = min(realChunkSize, size-offset);
        prims.directRecvCopySend(offset, offset, nelem);
      }

      // Make final copy from buffer to dest.
      chunk = modRanks(ringIx + 1);
      offset = calcOffset(chunk);
      nelem = min(realChunkSize, size-offset);
      prims.directRecv(offset, nelem);
    }
  }
```

</details>

<a id="f0059"></a>

## F0059：runTreeUpDown

用途：先向树根归约再向下广播结果，同一个线程组先后执行两个阶段。

- 上行把叶的send、中间节点的多收归约转发、根的最终recvReduceCopy分开。
- 同一线程组上行阶段结束后，换向构造广播原语；根从output发送，叶仅接收，中间节点复制转发。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const int nthreads = args-&gt;header.nWarps*WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const int bid = args-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const int nChannels = args-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>ncclTree *tree = &ncclShmem.channel.tree;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>ssize_t chunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? args-&gt;lastChunkSize : Proto::calcBytePerStep()/sizeof(T));</code> | 计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。 |
| 7 | 0 / 准备局部状态 | <code>const ssize_t minChunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads-2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T)) : nthreads*(Proto::calcBytePerGrain()/sizeof(T)));</code> | 给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。 |
| 8 | 0 / 准备局部状态 | <code>const ssize_t loopSize = int(nChannels*chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>const ssize_t size = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 条件判断 | <code>(loopSize &gt; size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>chunkSize = divUp((int)size, int(nChannels*minChunkSize))*int(minChunkSize);</code> | 计算整数向上取整的商，避免尾数不整除时少覆盖最后一块。 |
| 13 | 0 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;NCCL_MAX_DEV_ARITY, 1&gt;, 0, Proto, 0&gt; prims (tid, nthreads, tree-&gt;down, &tree-&gt;up, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 条件判断 | <code>(tree-&gt;up == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 3 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 3 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 19 | 3 / 调用 / 计算 / 写入 | <code>prims.recvReduceCopy(offset, offset, nelem, true);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 22 | 2 / 条件判断 | <code>(tree-&gt;down[0] == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 4 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 25 | 5 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 5 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 27 | 5 / 调用 / 计算 / 写入 | <code>prims.send(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 29 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 30 | 4 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 31 | 5 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 5 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 33 | 5 / 调用 / 计算 / 写入 | <code>prims.recvReduceSend(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 34 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 35 | 0 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;1, NCCL_MAX_DEV_ARITY&gt;, 1, Proto, 0&gt; prims (tid, nthreads, &tree-&gt;up, tree-&gt;down, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 36 | 0 / 条件判断 | <code>(tree-&gt;up == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 2 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 3 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 3 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 41 | 3 / 调用 / 计算 / 写入 | <code>prims.directSendFromOutput(offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 43 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 44 | 2 / 条件判断 | <code>(tree-&gt;down[0] == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 4 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 47 | 5 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 48 | 5 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 49 | 5 / 调用 / 计算 / 写入 | <code>prims.directRecv(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 50 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 51 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 52 | 4 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 53 | 5 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 54 | 5 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 55 | 5 / 调用 / 计算 / 写入 | <code>prims.directRecvCopySend(offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 56 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclTree *tree = &ncclShmem.channel.tree;
    ssize_t chunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? args->lastChunkSize
                   /* LL & LL128 */  : Proto::calcBytePerStep()/sizeof(T));
    const ssize_t minChunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads-2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T))
                   /* LL & LL128 */  : nthreads*(Proto::calcBytePerGrain()/sizeof(T)));
    const ssize_t loopSize = int(nChannels*chunkSize);
    const ssize_t size = args->count;

    if (loopSize > size)
      chunkSize = divUp((int)size, int(nChannels*minChunkSize))*int(minChunkSize);

    { // Reduce : max number of recv is 3, max number of send is 1 (binary tree + local)
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, /*Direct=*/0, Proto, 0> prims
        (tid, nthreads, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg);
      if (tree->up == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceCopy(offset, offset, nelem, /*postOp=*/true);
        }
      }
      else if (tree->down[0] == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
      else {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceSend(offset, nelem);
        }
      }
    }

    { // Broadcast : max number of recv is 1, max number of send is 3 (binary tree + local)
      Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0> prims
        (tid, nthreads, &tree->up, tree->down, args->sendbuff, args->recvbuff, args->redOpArg);
      if (tree->up == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directSendFromOutput(offset, offset, nelem);
        }
      }
      else if (tree->down[0] == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directRecv(offset, nelem);
        }
      }
      else {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directRecvCopySend(offset, offset, nelem);
        }
      }
    }
  }
```

</details>

<a id="f0060"></a>

## F0060：runTreeSplit

用途：把非根 block 线程分成上行归约与下行广播两组，用不同 barrier/group 形成重叠流水。

- 先按协议确定上下行线程拆分；非根上行组与下行组使用不同group/barrier。
- 根用融合原语完成多收归约、保留和多发，非根可让前一块下行与后一块上行重叠。
- 构造器Direct能力还要与对端地址交换契约兼容，即使某组不实际direct搬数据也不能任意改模板位。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const int nthreads = args-&gt;header.nWarps*WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const int bid = args-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const int nChannels = args-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>ncclTree *tree = &ncclShmem.channel.tree;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>ssize_t chunkSize = int( Proto::Id != NCCL_PROTO_LL ? args-&gt;lastChunkSize : Proto::calcBytePerStep()/sizeof(T));</code> | 计算该协议一个 FIFO step 中的有效载荷字节数，LL/LL128 扣除行内 flag 开销。 |
| 7 | 0 / 准备局部状态 | <code>const ssize_t minChunkSize = int( Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads - 2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T)) : Proto::Id == NCCL_PROTO_LL ? nthreads*(Proto::calcBytePerGrain()/sizeof(T)) : nthreads*(Proto::calcBytePerGrain()/sizeof(T))/8);</code> | 给出该协议按线程划分数据时的有效字节粒度，用于确定对齐和最小 chunk。 |
| 8 | 0 / 准备局部状态 | <code>const ssize_t loopSize = int(nChannels*chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>const ssize_t size = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>int nthreadsSplit;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 条件判断 | <code>(Proto::Id == NCCL_PROTO_SIMPLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>nthreadsSplit = nthreads/2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 条件判断 | <code>(nthreadsSplit &gt;= 256)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>nthreadsSplit += 64;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>nthreadsSplit = (nthreads*7/(10*WARP_SIZE))*WARP_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(loopSize &gt; size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>chunkSize = divUp((int)size, nChannels*int(minChunkSize))*int(minChunkSize);</code> | 计算整数向上取整的商，避免尾数不整除时少覆盖最后一块。 |
| 22 | 0 / 条件判断 | <code>(tree-&gt;up == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanSymmetric&lt;NCCL_MAX_DEV_ARITY&gt;, 1, Proto, 0&gt; prims(tid, nthreads, tree-&gt;down, tree-&gt;down, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 2 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 26 | 3 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 3 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>prims.directRecvReduceCopySend(offset, offset, offset, nelem, true);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 30 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 31 | 2 / 条件判断 | <code>(tid &lt; nthreadsSplit)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 4 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;NCCL_MAX_DEV_ARITY, 1&gt;, 1, Proto, 0&gt; prims(tid, nthreadsSplit, tree-&gt;down, &tree-&gt;up, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg, 0*Proto::MaxGroupWidth);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 4 / 条件判断 | <code>(tree-&gt;down[0] == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 6 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 37 | 7 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 7 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 39 | 7 / 调用 / 计算 / 写入 | <code>prims.send(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 41 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 42 | 6 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 43 | 7 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 7 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 45 | 7 / 调用 / 计算 / 写入 | <code>prims.recvReduceSend(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 48 | 4 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;1, NCCL_MAX_DEV_ARITY&gt;, 1, Proto, 0&gt; prims(tid-nthreadsSplit, nthreads-nthreadsSplit, &tree-&gt;up, tree-&gt;down, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg, 1*Proto::MaxGroupWidth);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 49 | 4 / 条件判断 | <code>(tree-&gt;down[0] == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 6 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 52 | 7 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 53 | 7 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 54 | 7 / 调用 / 计算 / 写入 | <code>prims.directRecv(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 55 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 56 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 57 | 6 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 58 | 7 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*int(chunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 7 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 60 | 7 / 调用 / 计算 / 写入 | <code>prims.directRecvCopySend(offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 61 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const int tid = threadIdx.x;
    const int nthreads = args->header.nWarps*WARP_SIZE;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    ncclTree *tree = &ncclShmem.channel.tree;
    ssize_t chunkSize = int(
      Proto::Id != NCCL_PROTO_LL ? args->lastChunkSize
                                 : Proto::calcBytePerStep()/sizeof(T));
    const ssize_t minChunkSize = int(
      Proto::Id == NCCL_PROTO_SIMPLE ? (nthreads - 2*WARP_SIZE)*8*(sizeof(uint64_t)/sizeof(T)) :
      Proto::Id == NCCL_PROTO_LL     ? nthreads*(Proto::calcBytePerGrain()/sizeof(T))
                   /* LL128 */       : nthreads*(Proto::calcBytePerGrain()/sizeof(T))/8);
    const ssize_t loopSize = int(nChannels*chunkSize);
    const ssize_t size = args->count;

/* 中文精读：Tree 分线程组的流水线
 * 非根节点把一组线程用于向上归约，另一组线程用于向下广播；二者使用不同 group/barrier。
 * Simple 大致均分后对归约组作调整，LL/LL128 约 70% 线程给更重的多输入归约。
 * 根用融合原语同时完成收集、归约、保存、向下分发；叶子没有有效 child，原语自动简化。
 * 线程组分割是设备执行优化，拓扑 SPLIT_TREE 则是选择跨机端点，两者不能混作一个开关。
 */
    int nthreadsSplit;
    if (Proto::Id == NCCL_PROTO_SIMPLE) {
      nthreadsSplit = nthreads/2;
      if (nthreadsSplit >= 256) nthreadsSplit += 64;
    } else { // LL & LL128
      // Receiving from up to 3 sources is more compute intensive than sending
      // to 3 dests. Use 70% for reduce and 30% for bcast.
      nthreadsSplit = (nthreads*7/(10*WARP_SIZE))*WARP_SIZE;
    }

    if (loopSize > size)
      chunkSize = divUp((int)size, nChannels*int(minChunkSize))*int(minChunkSize);

    if (tree->up == -1) {
      // Reduce and broadcast. Max number of recv is 3, max number of send is 3
      Primitives<T, RedOp, FanSymmetric<NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid, nthreads, tree->down, tree->down, args->sendbuff, args->recvbuff, args->redOpArg);
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        ssize_t offset = gridOffset + bid*int(chunkSize);
        int nelem = min(chunkSize, size-offset);
        prims.directRecvReduceCopySend(offset, offset, offset, nelem, /*doPost=*/true);
      }
    }
    else if (tid < nthreadsSplit) {
      /* Reduce up. Max number of recv is 3, max number of send is 1 (binary tree + local).
       * Why Direct=1????
       * Answer: Because despite not performing any direct operations, the ctor
       * must assume Direct so that it can exchange direct pointers with remote ctors
       * that are Direct, otherwise it hangs. A cleaner solution would be to seperate
       * into DirectRecv and DirectSend capabilities, this ctor would have both=0,
       * but the ctor above for tree roots would be DirectRecv=0 DirectSend=1.
       */
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DEV_ARITY, 1>, /*Direct=*/1, Proto, 0>
        prims(tid, nthreadsSplit, tree->down, &tree->up, args->sendbuff, args->recvbuff, args->redOpArg, 0*Proto::MaxGroupWidth);
      if (tree->down[0] == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
      else {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.recvReduceSend(offset, nelem);
        }
      }
    }
    else {
      // Broadcast down. Max number of recv is 1, max number of send is 3 (binary tree + local)
      Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DEV_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid-nthreadsSplit, nthreads-nthreadsSplit, &tree->up, tree->down, args->sendbuff, args->recvbuff, args->redOpArg, 1*Proto::MaxGroupWidth);
      if (tree->down[0] == -1) {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directRecv(offset, nelem);
        }
      }
      else {
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + bid*int(chunkSize);
          int nelem = min(chunkSize, size-offset);
          prims.directRecvCopySend(offset, offset, nelem);
        }
      }
    }
  }
```

</details>

<a id="f0061"></a>

## F0061：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>using Proto = ProtoSimple&lt;ALLREDUCE_CHUNKSTEPS/ALLREDUCE_SLICESTEPS, ALLREDUCE_SLICESTEPS&gt;;</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>runRing&lt;T, RedOp, Proto&gt;(args);</code> | 执行当前文件对应 API 的设备端环算法：各 channel 按 bid 计算地址，沿 prev/next 用协议原语流水推进。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    using Proto = ProtoSimple<ALLREDUCE_CHUNKSTEPS/ALLREDUCE_SLICESTEPS, ALLREDUCE_SLICESTEPS>;
    runRing<T, RedOp, Proto>(args);
  }
```

</details>

<a id="f0062"></a>

## F0062：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if CUDART_VERSION &gt;= 11020 && CUDART_VERSION &lt; 11040 && __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>runTreeUpDown&lt;T, RedOp, ProtoSimple&lt;1, 1&gt;&gt;(args);</code> | 先向树根归约再向下广播结果，同一个线程组先后执行两个阶段。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>runTreeSplit&lt;T, RedOp, ProtoSimple&lt;1, 1&gt;&gt;(args);</code> | 把非根 block 线程分成上行归约与下行广播两组，用不同 barrier/group 形成重叠流水。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    #if CUDART_VERSION >= 11020 && CUDART_VERSION < 11040 && __CUDA_ARCH__ >= 800
      runTreeUpDown<T, RedOp, ProtoSimple<1, 1>>(args);
    #else
      runTreeSplit<T, RedOp, ProtoSimple<1, 1>>(args);
    #endif
  }
```

</details>

<a id="f0063"></a>

## F0063：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>static constexpr int COLLNET_COPY_THREADS = 96;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const int bid = args-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const int nChannels = args-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclDirect* tree = &ncclShmem.channel.collTree;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>const ssize_t chunkSize = int(args-&gt;lastChunkSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>const ssize_t size = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>const ssize_t loopSize = nChannels*tree-&gt;nHeads*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>const int hasUp = (tree-&gt;up[0] &gt;= 0) ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>const int hasDn = (tree-&gt;down[0] &gt;= 0) ? 1 : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 准备局部状态 | <code>const int nThreadsScatter = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 3*COLLNET_COPY_THREADS : 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>const int nThreadsGather = ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 2*COLLNET_COPY_THREADS : 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>const int nThreadsBcast = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 0 : 2*COLLNET_COPY_THREADS);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>const int nThreadsReduce = args-&gt;header.nWarps*WARP_SIZE - nThreadsScatter - nThreadsGather - nThreadsBcast;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 准备局部状态 | <code>const int tidStartBcast = nThreadsGather;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 准备局部状态 | <code>const int tidStartScatter = tidStartBcast + nThreadsBcast;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 0 / 准备局部状态 | <code>const int tidStartReduce = tidStartScatter + nThreadsScatter;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 0 / 源码语法片段 | <code>using Proto = ProtoSimple&lt;1, 1&gt;;</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 19 | 0 / 条件判断 | <code>(tid &gt;= tidStartScatter && tid &lt; tidStartReduce && hasUp)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 准备局部状态 | <code>int group = (2*Proto::MaxGroupWidth) &#124; (1&lt;&lt;16);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 2 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;0, NCCL_MAX_DIRECT_ARITY&gt;, 1, Proto, 0&gt; prims(tid-tidStartScatter, nThreadsScatter, NULL, tree-&gt;up, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg, group, args);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 2 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 3 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*tree-&gt;nHeads*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 3 / 准备局部状态 | <code>int nelem = min(tree-&gt;nHeads*chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 26 | 3 / 条件判断 | <code>(args-&gt;regUsed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 5 / 调用 / 计算 / 写入 | <code>prims.directScatter(offset, nelem, chunkSize, tree-&gt;headRank, tree-&gt;shift);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 30 | 5 / 调用 / 计算 / 写入 | <code>prims.scatter(offset, nelem, chunkSize, tree-&gt;headRank, tree-&gt;shift);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 32 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 33 | 2 / 条件判断 | <code>(tid &gt;= tidStartReduce && tree-&gt;out != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 4 / 准备局部状态 | <code>int group = (3*Proto::MaxGroupWidth) &#124; (1&lt;&lt;16);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 36 | 4 / 条件判断 | <code>(hasDn)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 6 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;NCCL_MAX_DIRECT_ARITY, 1&gt;, 1, Proto, 0&gt; prims(tid-tidStartReduce, nThreadsReduce, tree-&gt;down, &tree-&gt;out, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg, group, args);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 39 | 6 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 40 | 7 / 准备局部状态 | <code>ssize_t offset = gridOffset + (bid*tree-&gt;nHeads+tree-&gt;headRank)*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 7 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 42 | 7 / 条件判断 | <code>(args-&gt;regUsed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 9 / 调用 / 计算 / 写入 | <code>prims.directRecvReduceSend(offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 46 | 9 / 调用 / 计算 / 写入 | <code>prims.recvReduceSend(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 48 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 49 | 6 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;0, 1&gt;, 0, Proto, 0&gt; prims(tid-tidStartReduce, nThreadsReduce, nullptr, &tree-&gt;out, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg, group);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 50 | 6 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 51 | 7 / 准备局部状态 | <code>ssize_t offset = gridOffset + (bid*tree-&gt;nHeads+tree-&gt;headRank)*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 7 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 53 | 7 / 调用 / 计算 / 写入 | <code>prims.send(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 54 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 55 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 56 | 4 / 条件判断 | <code>(tid &lt; tidStartBcast && hasUp)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 6 / 准备局部状态 | <code>int group = (0*Proto::MaxGroupWidth) &#124; (0&lt;&lt;16);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 6 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;NCCL_MAX_DIRECT_ARITY, 0&gt;, 1, Proto, 0&gt; prims(tid, nThreadsGather, tree-&gt;up, NULL, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg, group, args);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 60 | 6 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 61 | 7 / 准备局部状态 | <code>ssize_t offset = gridOffset + bid*tree-&gt;nHeads*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 62 | 7 / 准备局部状态 | <code>int nelem = min(tree-&gt;nHeads*chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 63 | 7 / 调用 / 计算 / 写入 | <code>prims.directGather(offset, nelem, chunkSize, tree-&gt;headRank, tree-&gt;shift);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 64 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 65 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 66 | 6 / 条件判断 | <code>(tid &gt;= tidStartBcast && tid &lt; tidStartScatter && tree-&gt;out != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 67 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 68 | 8 / 准备局部状态 | <code>int group = (1*Proto::MaxGroupWidth) &#124; (0&lt;&lt;16);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 69 | 8 / 条件判断 | <code>(hasDn)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 70 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 71 | 10 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;1, NCCL_MAX_DIRECT_ARITY&gt;, 1, Proto, 0&gt; prims(tid-tidStartBcast, nThreadsBcast, &tree-&gt;out, tree-&gt;down, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg, group, args);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 72 | 10 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 73 | 11 / 准备局部状态 | <code>ssize_t offset = gridOffset + (bid*tree-&gt;nHeads+tree-&gt;headRank)*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 74 | 11 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 75 | 11 / 调用 / 计算 / 写入 | <code>prims.recvCopyDirectSend(offset, offset, nelem, true);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 76 | 10 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 77 | 9 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 78 | 10 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;1, 0&gt;, 0, Proto, 0&gt; prims(tid-tidStartBcast, nThreadsBcast, &tree-&gt;out, nullptr, args-&gt;sendbuff, args-&gt;recvbuff, args-&gt;redOpArg, group);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 79 | 10 / 循环 | <code>初始 ssize_t gridOffset = 0;；继续条件 gridOffset &lt; size；每轮更新 gridOffset += loopSize</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 80 | 11 / 准备局部状态 | <code>ssize_t offset = gridOffset + (bid*tree-&gt;nHeads+tree-&gt;headRank)*chunkSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 81 | 11 / 准备局部状态 | <code>int nelem = min(chunkSize, size-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 82 | 11 / 调用 / 计算 / 写入 | <code>prims.recv(offset, nelem, true);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 83 | 10 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
/* 中文精读：CollNet 把一次 AllReduce 分成四组协作流水
 * Scatter 把本机输入分配给多个 head；Reduce 组在 head 聚合机内贡献并发往网络。
 * Broadcast 组把网络归约结果分发到机内参与者；Gather 组收齐各个 head 负责的片段。
 * loopSize=nChannels*nHeads*chunkSize，因此除 channel 分片外，还多了一层 head 分片。
 * 网络端点 tree->out 对应特殊 peer，Proxy 调插件 Iallreduce；GPU 不直接在交换机上执行归约。
 * 本版此特化只支持 Simple，启用需要 CollNet 插件与对应硬件/数据类型支持。
 */
    static constexpr int COLLNET_COPY_THREADS = 96;
    const int tid = threadIdx.x;
    const int bid = args->bid;
    const int nChannels = args->nChannels;
    struct ncclDirect* tree = &ncclShmem.channel.collTree;
    const ssize_t chunkSize = int(args->lastChunkSize);
    const ssize_t size = args->count;
    const ssize_t loopSize = nChannels*tree->nHeads*chunkSize;

    const int hasUp = (tree->up[0] >= 0) ? 1 : 0;
    const int hasDn = (tree->down[0] >= 0) ? 1 : 0;
    const int nThreadsScatter = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 3*COLLNET_COPY_THREADS : 0);
    const int nThreadsGather  =             ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 2*COLLNET_COPY_THREADS : 0);
    const int nThreadsBcast   = WARP_SIZE + ((hasUp && hasDn) ? COLLNET_COPY_THREADS : hasUp ? 0 : 2*COLLNET_COPY_THREADS);
    const int nThreadsReduce = args->header.nWarps*WARP_SIZE - nThreadsScatter - nThreadsGather - nThreadsBcast;
    const int tidStartBcast = nThreadsGather;
    const int tidStartScatter = tidStartBcast + nThreadsBcast;
    const int tidStartReduce = tidStartScatter + nThreadsScatter;

    using Proto = ProtoSimple<1, 1>;

    if (tid >= tidStartScatter && tid < tidStartReduce && hasUp) {
      // Scatter
      int group = (2*Proto::MaxGroupWidth) | (1<<16);
      Primitives<T, RedOp, FanAsymmetric<0, NCCL_MAX_DIRECT_ARITY>, /*Direct=*/1, Proto, 0>
        prims(tid-tidStartScatter, nThreadsScatter, NULL, tree->up, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        ssize_t offset = gridOffset + bid*tree->nHeads*chunkSize;
        int nelem = min(tree->nHeads*chunkSize, size-offset);
        if (args->regUsed) {
          prims.directScatter(offset, nelem, chunkSize, tree->headRank, tree->shift);
        } else {
          prims.scatter(offset, nelem, chunkSize, tree->headRank, tree->shift);
        }
      }
    } else if (tid >= tidStartReduce && tree->out != -1) {
      int group = (3*Proto::MaxGroupWidth) | (1<<16);
      if (hasDn) {
        // Reduce, send to network
        Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 1>, /*Direct=*/1, Proto, 0>
          prims(tid-tidStartReduce, nThreadsReduce, tree->down, &tree->out, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
          int nelem = min(chunkSize, size-offset);
          if (args->regUsed) {
            prims.directRecvReduceSend(offset, offset, nelem);
          } else {
            prims.recvReduceSend(offset, nelem);
          }
        }
      } else {
        // Directly send to network
        Primitives<T, RedOp, FanAsymmetric<0, 1>, /*Direct=*/0, Proto, 0>
          prims(tid-tidStartReduce, nThreadsReduce, nullptr, &tree->out, args->sendbuff, args->recvbuff, args->redOpArg, group);
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
          int nelem = min(chunkSize, size-offset);
          prims.send(offset, nelem);
        }
      }
    } else if (tid < tidStartBcast && hasUp) {
      // Gather
      int group = (0*Proto::MaxGroupWidth) | (0<<16);
      Primitives<T, RedOp, FanAsymmetric<NCCL_MAX_DIRECT_ARITY, 0>, /*Direct=*/1, Proto, 0>
        prims(tid, nThreadsGather, tree->up, NULL, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
      for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
        ssize_t offset = gridOffset + bid*tree->nHeads*chunkSize;
        int nelem = min(tree->nHeads*chunkSize, size-offset);
        prims.directGather(offset, nelem, chunkSize, tree->headRank, tree->shift);
      }
    } else if (tid >= tidStartBcast && tid < tidStartScatter && tree->out != -1) {
      int group = (1*Proto::MaxGroupWidth) | (0<<16);
      if (hasDn) {
        // Recv from network, broadcast
        Primitives<T, RedOp, FanAsymmetric<1, NCCL_MAX_DIRECT_ARITY>, /*Direct=*/1, Proto, 0>
          prims(tid-tidStartBcast, nThreadsBcast, &tree->out, tree->down, args->sendbuff, args->recvbuff, args->redOpArg, group, args);
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
          int nelem = min(chunkSize, size-offset);
          prims.recvCopyDirectSend(offset, offset, nelem, /*postOp=*/true);
        }
      } else {
        // Recv from network (no post thread needed)
        Primitives<T, RedOp, FanAsymmetric<1, 0>, /*Direct=*/0, Proto, 0>
          prims(tid-tidStartBcast, nThreadsBcast, &tree->out, nullptr, args->sendbuff, args->recvbuff, args->redOpArg, group);
        for (ssize_t gridOffset = 0; gridOffset < size; gridOffset += loopSize) {
          ssize_t offset = gridOffset + (bid*tree->nHeads+tree->headRank)*chunkSize;
          int nelem = min(chunkSize, size-offset);
          prims.recv(offset, nelem, /*postOp=*/true);
        }
      }
    }
  }
```

</details>

<a id="f0064"></a>

## F0064：run

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

<a id="f0065"></a>

## F0065：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>runTreeSplit&lt;T, RedOp, ProtoLL&gt;(args);</code> | 把非根 block 线程分成上行归约与下行广播两组，用不同 barrier/group 形成重叠流水。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    runTreeSplit<T, RedOp, ProtoLL>(args);
  }
```

</details>

<a id="f0066"></a>

## F0066：run

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

<a id="f0067"></a>

## F0067：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>runTreeSplit&lt;T, RedOp, ProtoLL128&gt;(args);</code> | 把非根 block 线程分成上行归约与下行广播两组，用不同 barrier/group 形成重叠流水。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    runTreeSplit<T, RedOp, ProtoLL128>(args);
  }
```

</details>

