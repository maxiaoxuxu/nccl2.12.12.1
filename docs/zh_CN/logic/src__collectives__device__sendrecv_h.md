# src/collectives/device/sendrecv.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0374"></a>

## F0374：runSend

用途：执行分给当前 P2P 线程组的发送任务；self peer 走本地复制，其他 peer 用 Simple 原语分块。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(args-&gt;peer == ncclShmem.comm.rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 准备局部状态 | <code>struct ncclWorkElemP2p* recvArgs = args-1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 2 / 条件判断 | <code>(args-&gt;buff != recvArgs-&gt;buff)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 调用 / 计算 / 写入 | <code>ReduceOrCopyMulti&lt;COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 0&gt;(tid, nthreads, nullptr, false, 1, (const T**)&args-&gt;buff, 1, (T**)&recvArgs-&gt;buff, args-&gt;count);</code> | 统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。 |
| 7 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 8 | 2 / 源码语法片段 | <code>using Proto = ProtoSimple&lt;1, 1&gt;;</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 9 | 2 / 准备局部状态 | <code>ssize_t const count = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 准备局部状态 | <code>int const chunkSize = args-&gt;chunkSize/sizeof(T);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 2 / 准备局部状态 | <code>int const peer = args-&gt;peer;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 2 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;0, 1&gt;, 1, Proto, 1&gt; prims (tid, nthreads, nullptr, &peer, args-&gt;buff, nullptr, 0, group);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 2 / 准备局部状态 | <code>ssize_t offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 2 / 先执行一次的循环 | <code>初始 ；继续条件 (offset &lt; count)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 15 | 3 / 准备局部状态 | <code>int nelem = min(chunkSize, count-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>prims.directSend(offset, offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>offset += nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (args->peer == ncclShmem.comm.rank) {
      struct ncclWorkElemP2p* recvArgs = args-1;
      if (args->buff != recvArgs->buff) {
        ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 0>(tid, nthreads, nullptr, false, 1, (const T**)&args->buff, 1, (T**)&recvArgs->buff, args->count);
      }
    } else {
      using Proto = ProtoSimple<1, 1>;
      ssize_t const count = args->count;
      int const chunkSize = args->chunkSize/sizeof(T);
      int const peer = args->peer;
      Primitives<T, RedOp, FanAsymmetric<0, 1>, 1, Proto, 1> prims
        (tid, nthreads, nullptr, &peer, args->buff, nullptr, /*redOpArg(ignored)=*/0, group);
/* 中文精读：P2P 的第二层切块：在选定 channel 内推进
 * Group 已把原消息切成分配给 channel 的块，此处对当前 args->count 再按 chunkSize 循环。
 * 每轮 directSend 内还有 Simple 的 wait/copy/post，CPU Proxy 根据同一尺寸推进网络。
 * do-while 使 count=0 也执行一次协议同步；它并非遗漏了空数据检查。
 * 这条路径只复制数据，不做集合归约；All-to-All 可由上层为所有 peer 组合这样的 Send/Recv。
 */
      ssize_t offset = 0;
      do {
        int nelem = min(chunkSize, count-offset);
        prims.directSend(offset, offset, nelem);
        offset += nelem;
      } while(offset < count);
    }
  }
```

</details>

<a id="f0375"></a>

## F0375：runRecv

用途：执行 P2P 接收线程组任务；self peer 的数据由配对发送组复制，其他 peer 用 Simple 接收。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(args-&gt;peer != ncclShmem.comm.rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 源码语法片段 | <code>using Proto = ProtoSimple&lt;1, 1&gt;;</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 4 | 2 / 准备局部状态 | <code>ssize_t const count = args-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 准备局部状态 | <code>int const chunkSize = args-&gt;chunkSize/sizeof(T);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 准备局部状态 | <code>int const peer = args-&gt;peer;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 准备局部状态 | <code>Primitives&lt;T, RedOp, FanAsymmetric&lt;1, 0&gt;, 1, Proto, 1&gt; prims (tid, nthreads, &peer, nullptr, nullptr, args-&gt;buff, 0, group);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 准备局部状态 | <code>ssize_t offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 先执行一次的循环 | <code>初始 ；继续条件 (offset &lt; count)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 10 | 3 / 准备局部状态 | <code>int nelem = min(chunkSize, count-offset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>prims.directRecv(offset, nelem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>offset += nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (args->peer != ncclShmem.comm.rank) {
      using Proto = ProtoSimple<1, 1>;
      ssize_t const count = args->count;
      int const chunkSize = args->chunkSize/sizeof(T);
      int const peer = args->peer;
      Primitives<T, RedOp, FanAsymmetric<1, 0>, 1, Proto, 1> prims
        (tid, nthreads, &peer, nullptr, nullptr, args->buff, /*redOpArg(ignored)=*/0, group);
      ssize_t offset = 0;
      do {
        int nelem = min(chunkSize, count-offset);
        prims.directRecv(offset, nelem);
        offset += nelem;
      } while(offset < count);
    }
  }
```

</details>

<a id="f0376"></a>

## F0376：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclWorkElemP2p* args = work-&gt;p2pElems;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int ngroups = args-&gt;ngroups;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int wid = tid / WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 编译期分支/指令 | <code>#define NWARPS (NCCL_MAX_NTHREADS/WARP_SIZE)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 0 / 准备局部状态 | <code>int group = ngroups-1- (NWARPS-1-wid) * ngroups / NWARPS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>args += group;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 条件判断 | <code>(args-&gt;header.type == ncclWorkTypeUnused)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>tid -= args-&gt;warpStart * WARP_SIZE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 准备局部状态 | <code>int nthreads = args-&gt;nWarps * WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>group &#124;= 1&lt;&lt;16;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 条件判断 | <code>(tid &gt;= nthreads &#124;&#124; args-&gt;peer == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 17 | 0 / 条件判断 | <code>((group%2) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>runRecv(tid, nthreads, group, args);</code> | 执行 P2P 接收线程组任务；self peer 的数据由配对发送组复制，其他 peer 用 Simple 接收。 |
| 20 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>runSend(tid, nthreads, group, args);</code> | 执行分给当前 P2P 线程组的发送任务；self peer 走本地复制，其他 peer 用 Simple 原语分块。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    struct ncclWorkElemP2p* args = work->p2pElems;
    int ngroups = args->ngroups;
    int tid = threadIdx.x;
    int wid = tid / WARP_SIZE;
    // This has to work even for groups of 2.5 warps (which is 8 groups, and means 3
    // warps for send, 2 warps for recv).
    // warpStarts were rounded thanks to int division, but for group number we need to round the other way around
    // So we mirror wid then mirror again the group.
    #define NWARPS (NCCL_MAX_NTHREADS/WARP_SIZE)
    int group = ngroups-1- (NWARPS-1-wid) * ngroups / NWARPS;
    args += group;
    if (args->header.type == ncclWorkTypeUnused) return;

    tid -= args->warpStart * WARP_SIZE;
    int nthreads = args->nWarps * WARP_SIZE;
/* 中文精读：线程组编码与连接索引
 * work 里收发按偶/奇槽分组，先算当前 warp 属于哪个元素，再减 warpStart 得到组内 tid。
 * 第 16 位是传给 Simple 构造函数的连接索引编码，使 P2P 找 peers[peer].send/recv[1]。
 * 这个位不是 channelId，也不是 GPU rank；低位仍用于组共享内存和 barrier 分配。
 * self send 在 runSend 中直接复制到前一个匹配 recv 元素的缓冲，runRecv 的 self 分支不重复搬运。
 */
    group |= 1<<16; // Used to select connIndex 1
    if (tid >= nthreads || args->peer == -1) return;
    if ((group%2) == 0) {
      runRecv(tid, nthreads, group, args);
    } else {
      runSend(tid, nthreads, group, args);
    }
  }
```

</details>

