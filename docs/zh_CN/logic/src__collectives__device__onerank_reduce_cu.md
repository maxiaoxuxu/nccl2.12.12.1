# src/collectives/device/onerank_reduce.cu：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0129"></a>

## F0129：oneRankReduce

用途：单 rank 仍需执行自定义 PreMulSum 时，在 GPU 上分配各 channel 范围并处理输入缩放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>ncclWork *w = &ncclShmem.work;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int tn = blockDim.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 编译期分支/指令 | <code>#pragma unroll 1</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 5 | 0 / 循环 | <code>初始 int e=0;；继续条件 e &lt; NCCL_MAX_WORK_ELEMENTS && w-&gt;elems[e].header.type != ncclWorkTypeUnused；每轮更新 e++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 准备局部状态 | <code>ncclWorkElem *we = &w-&gt;elems[e];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 准备局部状态 | <code>intptr_t eltN = we-&gt;count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 准备局部状态 | <code>int bid = we-&gt;bid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 1 / 准备局部状态 | <code>int bn = we-&gt;nChannels;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 准备局部状态 | <code>T const *src = (T const*)we-&gt;sendbuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 1 / 准备局部状态 | <code>T *dst = (T*)we-&gt;recvbuff;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 1 / 准备局部状态 | <code>constexpr int EltPerPack = 16/sizeof(T);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 准备局部状态 | <code>intptr_t packN = (eltN + EltPerPack-1) - (eltN + EltPerPack-1)%EltPerPack;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 1 / 准备局部状态 | <code>intptr_t i0 = (bid+0)*(packN/bn) + (bid+0 &lt; packN%bn ? bid+0 : packN%bn);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 1 / 准备局部状态 | <code>intptr_t i1 = (bid+1)*(packN/bn) + (bid+1 &lt; packN%bn ? bid+1 : packN%bn);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 1 / 调用 / 计算 / 写入 | <code>i0 *= EltPerPack;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 调用 / 计算 / 写入 | <code>i0 = i0 &lt; eltN ? i0 : eltN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 1 / 调用 / 计算 / 写入 | <code>i1 *= EltPerPack;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 1 / 调用 / 计算 / 写入 | <code>i1 = i1 &lt; eltN ? i1 : eltN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 1 / 调用 / 计算 / 写入 | <code>src += i0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 1 / 调用 / 计算 / 写入 | <code>dst += i0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 1 / 调用 / 计算 / 写入 | <code>ReduceOrCopyMulti&lt;COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 1&gt; (tid, tn, &(we-&gt;redOpArg), true, 1, &src, 1, &dst, i1-i0);</code> | 统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。 |
| 23 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    ncclWork *w = &ncclShmem.work;
    int tid = threadIdx.x;
    int tn = blockDim.x;
    #pragma unroll 1
    for(int e=0; e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused; e++) {
      ncclWorkElem *we = &w->elems[e];
      intptr_t eltN = we->count;
      int bid = we->bid;
      int bn = we->nChannels;
      T const *src = (T const*)we->sendbuff;
      T *dst = (T*)we->recvbuff;

      // each block/channel gets a roughly equal segment of 16 byte packs
      constexpr int EltPerPack = 16/sizeof(T);
      intptr_t packN = (eltN + EltPerPack-1) - (eltN + EltPerPack-1)%EltPerPack;
      intptr_t i0 = (bid+0)*(packN/bn) + (bid+0 < packN%bn ? bid+0 : packN%bn);
      intptr_t i1 = (bid+1)*(packN/bn) + (bid+1 < packN%bn ? bid+1 : packN%bn);
      i0 *= EltPerPack;
      i0 = i0 < eltN ? i0 : eltN;
      i1 *= EltPerPack;
      i1 = i1 < eltN ? i1 : eltN;
      src += i0;
      dst += i0;
      ReduceOrCopyMulti<COLL_UNROLL, RedOp, T, 1, 1, 1, 1, 1>
        (tid, tn, &(we->redOpArg), true, 1, &src, 1, &dst, i1-i0);
    }
  }
```

</details>

