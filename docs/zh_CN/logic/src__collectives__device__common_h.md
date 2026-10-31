# src/collectives/device/common.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0072"></a>

## F0072：barrierReduceAny

用途：执行 block 线程投票式屏障，检查任一线程是否报告中止，使整个 block 一致退出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>uint32_t popc;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>asm ("{" ".reg .pred barr_pred;" "setp.eq.u32 barr_pred, %1, 1;" "bar.red.popc.u32 %0, 2, barr_pred;" "}" : "=r"(popc) : "r"(bit));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return popc != 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  uint32_t popc;
  asm ("{"
    ".reg .pred barr_pred;"
    "setp.eq.u32 barr_pred, %1, 1;"
    "bar.red.popc.u32 %0, 2, barr_pred;"
  "}" : "=r"(popc) : "r"(bit));
  return popc != 0;
}
```

</details>

<a id="f0073"></a>

## F0073：copyToShmem

用途：由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>static_assert(sizeof(Tdst)%(2*sizeof(uint64_t)) == 0 && sizeof(Tsrc)%(2*sizeof(uint64_t)) == 0, "copyToShmem needs sizes which are multiple of 16B");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 源码语法片段 | <code>static_assert(sizeof(Tdst) &gt;= sizeof(Tsrc), "Tdst size is too small");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 3 | 0 / 源码语法片段 | <code>static_assert(sizeof(Tdst) &lt;= WARP_SIZE*2*sizeof(uint64_t), "copyToShmem limited to 512B to make sure it can always be done in one cycle");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 4 | 0 / 准备局部状态 | <code>uint64_t *d = reinterpret_cast&lt;uint64_t*&gt;(dst);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>uint64_t const *s = reinterpret_cast&lt;uint64_t const*&gt;(src);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>uint64_t *shmemPtr = shmemCvtPtr(d);</code> | 把 CUDA 通用指针转换为 PTX shared 地址空间指针，供共享内存汇编指令使用。 |
| 7 | 0 / 准备局部状态 | <code>int offset = 2*tid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>uint64_t v0, v1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 条件判断 | <code>(offset &gt;= sizeof(Tsrc)/sizeof(uint64_t))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>v0 = v1 = 0ULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>v0 = s[offset] ;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>v1 = s[offset+1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 条件判断 | <code>(offset &lt; sizeof(Tdst)/sizeof(uint64_t))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>storeShmem128(shmemPtr+offset, v0, v1);</code> | 向 SM 共享内存写出 16 字节，供同 block 的线程在屏障后消费。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static_assert(sizeof(Tdst)%(2*sizeof(uint64_t)) == 0 && sizeof(Tsrc)%(2*sizeof(uint64_t)) == 0,
      "copyToShmem needs sizes which are multiple of 16B");
  static_assert(sizeof(Tdst) >= sizeof(Tsrc), "Tdst size is too small");
  static_assert(sizeof(Tdst) <= WARP_SIZE*2*sizeof(uint64_t), "copyToShmem limited to 512B to make sure it can always be done in one cycle");
  uint64_t *d = reinterpret_cast<uint64_t*>(dst);
  uint64_t const *s = reinterpret_cast<uint64_t const*>(src);
  uint64_t *shmemPtr = shmemCvtPtr(d);
  int offset = 2*tid;
  uint64_t v0, v1;
  if (offset >= sizeof(Tsrc)/sizeof(uint64_t)) {
    v0 = v1 = 0ULL;
  } else {
    v0 = s[offset] ; v1 = s[offset+1];
  }
  if (offset < sizeof(Tdst)/sizeof(uint64_t)) storeShmem128(shmemPtr+offset, v0, v1);
}
```

</details>

<a id="f0074"></a>

## F0074：copyToShmem

用途：由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>static_assert(sizeof(uint64_t) &lt;= alignof(T), "Uhoh");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 准备局部状态 | <code>uint64_t *d = reinterpret_cast&lt;uint64_t*&gt;(dst);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>uint64_t const *s = reinterpret_cast&lt;uint64_t const*&gt;(src);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int t = threadIdx.x - turn;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(t &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>t += blockDim.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 准备局部状态 | <code>int n = sizeof(T)/sizeof(uint64_t);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>int delta = (n + WARP_SIZE-1) & -WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 条件判断 | <code>(delta &lt; blockDim.x)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>turn += delta;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 条件判断 | <code>(turn &gt;= blockDim.x)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>turn -= blockDim.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>turn = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>n -= t;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>d += t;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>s += t;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 22 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; divUp(sizeof(T), WARP_SIZE*sizeof(uint64_t))；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 23 | 1 / 条件判断 | <code>(n &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 3 / 调用 / 计算 / 写入 | <code>*d = *s;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 3 / 调用 / 计算 / 写入 | <code>d += blockDim.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>s += blockDim.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>n -= blockDim.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 30 | 0 / 函数返回 | <code>return turn;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static_assert(sizeof(uint64_t) <= alignof(T), "Uhoh");
  uint64_t *d = reinterpret_cast<uint64_t*>(dst);
  uint64_t const *s = reinterpret_cast<uint64_t const*>(src);
  int t = threadIdx.x - turn;
  if (t < 0) t += blockDim.x;
  int n = sizeof(T)/sizeof(uint64_t);

  int delta = (n + WARP_SIZE-1) & -WARP_SIZE; // round up to warp lane 0
  if (delta < blockDim.x) {
    turn += delta;
    if (turn >= blockDim.x) turn -= blockDim.x;
  }
  else
    turn = 0;

  n -= t;
  d += t;
  s += t;
  #pragma unroll
  for (int i=0; i < divUp(sizeof(T), WARP_SIZE*sizeof(uint64_t)); i++) {
    if (n > 0) {
      *d = *s;
      d += blockDim.x;
      s += blockDim.x;
      n -= blockDim.x;
    }
  }
  return turn;
}
```

</details>

<a id="f0075"></a>

## F0075：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // Put NOT IMPLEMENTED behavior here.
  }
```

</details>

<a id="f0076"></a>

## F0076：run

用途：作为当前 RunWork/RunWorkElement 特化的设备执行入口，按模板选择算法协议或遍历同一 work 内元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int wid = threadIdx.x / WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int inc = w-&gt;header.type == ncclWorkTypeRegColl ? sizeof(ncclWorkElemReg) / sizeof(ncclWorkElem) : 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 编译期分支/指令 | <code>#pragma unroll 1</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 0 / 循环 | <code>初始 int e=0;；继续条件 e &lt; NCCL_MAX_WORK_ELEMENTS && w-&gt;elems[e].header.type != ncclWorkTypeUnused；每轮更新 e += inc</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 条件判断 | <code>(wid &lt; w-&gt;header.nWarps)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>RunWorkElement&lt;Fn, T, RedOp, Algo, Proto&gt;().run(&w-&gt;elems[e]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    int wid = threadIdx.x / WARP_SIZE;
    int inc = w->header.type == ncclWorkTypeRegColl ? sizeof(ncclWorkElemReg) / sizeof(ncclWorkElem) : 1;
    #pragma unroll 1
    for(int e=0; e < NCCL_MAX_WORK_ELEMENTS && w->elems[e].header.type != ncclWorkTypeUnused; e += inc) {
      if (wid < w->header.nWarps)
        RunWorkElement<Fn, T, RedOp, Algo, Proto>().run(&w->elems[e]);
    }
  }
```

</details>

<a id="f0077"></a>

## F0077：ncclRedopPtrDeref

用途：若归约标量以设备指针传入，按地址对齐选择可安全加载的宽度，把值的位模式放回 work。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(we-&gt;header.type != ncclWorkTypeUnused && we-&gt;redOpArgIsPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 条件判断 | <code>(we-&gt;redOpArg%2 != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 4 / 调用 / 计算 / 写入 | <code>we-&gt;redOpArg = *reinterpret_cast&lt;uint8_t*&gt;(we-&gt;redOpArg);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 7 | 4 / 条件判断 | <code>(we-&gt;redOpArg%4 != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 6 / 调用 / 计算 / 写入 | <code>we-&gt;redOpArg = *reinterpret_cast&lt;uint16_t*&gt;(we-&gt;redOpArg);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 6 / 条件判断 | <code>(we-&gt;redOpArg%8 != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 8 / 调用 / 计算 / 写入 | <code>we-&gt;redOpArg = *reinterpret_cast&lt;uint32_t*&gt;(we-&gt;redOpArg);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 8 / 调用 / 计算 / 写入 | <code>we-&gt;redOpArg = *reinterpret_cast&lt;uint64_t*&gt;(we-&gt;redOpArg);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (we->header.type != ncclWorkTypeUnused && we->redOpArgIsPtr) {
    /* redOpArg is a pointer to the scalar value, so we'll dereference it
     * here so that redOpArg holds the bits of the scalar going forward.
     * The tricky thing is we don't know its type T since that's encoded in
     * the funcIndex. Because it would be difficult to get sizeof(T) from
     * funcIndex, we'll cheat and just dereference the largest possible size
     * given the alignment of the pointer. We might be reading in more bytes
     * than we need but that's harmless.
     */
    if (we->redOpArg%2 != 0)
      we->redOpArg = *reinterpret_cast<uint8_t*>(we->redOpArg);
    else if (we->redOpArg%4 != 0)
      we->redOpArg = *reinterpret_cast<uint16_t*>(we->redOpArg);
    else if (we->redOpArg%8 != 0)
      we->redOpArg = *reinterpret_cast<uint32_t*>(we->redOpArg);
    else
      we->redOpArg = *reinterpret_cast<uint64_t*>(we->redOpArg);
  }
}
```

</details>

<a id="f0078"></a>

## F0078：ncclKernel

用途：以 blockIdx.x 选择 channel，读取/释放描述符 FIFO 槽，再按 funcIndex 执行本批设备任务到 isLast。

- blockIdx.x选择实际channel，先把comm/channel元数据复制到共享内存。
- channel0可用首条内联参数，其他任务从FIFO复制；取走描述后就可把主机槽标Unused。
- 按funcIndex匹配静态模板或设备函数表执行；isLast结束当前批，abort则整block一致退出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int tid = threadIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nthreads = blockDim.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int bid = blockIdx.x;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int turn = copyToShmem(&ncclShmem.comm, comm);</code> | 由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。 |
| 5 | 0 / 准备局部状态 | <code>ncclChannel *channel = &((ncclDevCommAndChannels*)comm)-&gt;channels[bid];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>turn = copyToShmem(&ncclShmem.channel, channel, turn);</code> | 由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。 |
| 7 | 0 / 条件判断 | <code>(bid == 0 && first.header.type != ncclWorkTypeUnused)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>copyToShmem(&ncclShmem.work, &first, tid, nthreads);</code> | 由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>__syncthreads();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 11 | 0 / 准备局部状态 | <code>ncclWork *workFifoHost = ncclShmem.channel.workFifo;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 准备局部状态 | <code>ncclWork *workFifoDev = ncclShmem.channel.workFifoDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 0 / 准备局部状态 | <code>int workFifoIx = ncclShmem.channel.index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 条件判断 | <code>(bid == 0 && first.header.type != ncclWorkTypeUnused)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 控制转移 | <code>goto SkipLoadWork;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 17 | 0 / 循环 | <code>初始 ；继续条件 (true)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 18 | 1 / 调用 / 计算 / 写入 | <code>copyToShmem(&ncclShmem.work, &workFifoDev[workFifoIx], tid, nthreads);</code> | 由 block 线程协作把全局工作/通信元数据复制到共享内存，减少后续重复读取的全局访问。 |
| 19 | 1 / 准备局部状态 | <code>int aborted = tid == 0 ? *comm-&gt;abortFlag : 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 1 / 条件判断 | <code>(barrierReduceAny(aborted))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 23 | 1 / 条件判断 | <code>(tid == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 3 / 调用 / 计算 / 写入 | <code>workFifoHost[workFifoIx].header.type = ncclWorkTypeUnused;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 1 / 跳转标签 | <code>SkipLoadWork</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>workFifoIx = (workFifoIx + 1)%NCCL_MAX_OPS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 1 / 条件判断 | <code>(tid == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>channel-&gt;index = workFifoIx;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 1 / 调用 / 计算 / 写入 | <code>__syncwarp();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 32 | 1 / 条件判断 | <code>(ncclShmem.work.header.type == ncclWorkTypeColl)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 3 / 条件判断 | <code>(tid &lt; NCCL_MAX_WORK_ELEMENTS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 5 / 调用 / 计算 / 写入 | <code>ncclRedopPtrDeref(&ncclShmem.work.elems[tid]);</code> | 若归约标量以设备指针传入，按地址对齐选择可安全加载的宽度，把值的位模式放回 work。 |
| 37 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 38 | 3 / 条件判断 | <code>(ncclShmem.work.header.type == ncclWorkTypeRegColl)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 5 / 条件判断 | <code>(tid &lt; NCCL_MAX_WORK_ELEMENTS_REG)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 7 / 调用 / 计算 / 写入 | <code>ncclRedopPtrDeref(&ncclShmem.work.regElems[tid].elem);</code> | 若归约标量以设备指针传入，按地址对齐选择可安全加载的宽度，把值的位模式放回 work。 |
| 43 | 1 / 调用 / 计算 / 写入 | <code>__syncthreads();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 44 | 1 / 条件判断 | <code>(ncclShmem.work.header.funcIndex == FnIndex)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 3 / 调用 / 计算 / 写入 | <code>RunWork&lt;Fn, T, RedOp, Algo, Proto&gt;().run(&ncclShmem.work);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 48 | 3 / 调用 / 计算 / 写入 | <code>ncclFuncs[ncclShmem.work.header.funcIndex]();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 1 / 条件判断 | <code>(ncclShmem.work.header.isLast)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 52 | 1 / 调用 / 计算 / 写入 | <code>__syncthreads();</code> | 让当前参与线程/warp按此同步点会合；参与范围由该屏障接口决定。 |
| 53 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int tid = threadIdx.x;
  int nthreads = blockDim.x;
  int bid = blockIdx.x;

  int turn = copyToShmem(&ncclShmem.comm, comm);
  // get address of channel without incurring indirect load from ncclDevCom::channels
  ncclChannel *channel = &((ncclDevCommAndChannels*)comm)->channels[bid];
  turn = copyToShmem(&ncclShmem.channel, channel, turn);

  // To optimize for latency, (only) the first operation is passed as argument.
  if (bid == 0 && first.header.type != ncclWorkTypeUnused) {
    // Copy first elem to work and zero out the rest
    copyToShmem(&ncclShmem.work, &first, tid, nthreads);
  }
  __syncthreads(); // publish ncclShmem

  ncclWork *workFifoHost = ncclShmem.channel.workFifo;
  ncclWork *workFifoDev = ncclShmem.channel.workFifoDev;
  int workFifoIx = ncclShmem.channel.index;

  if (bid == 0 && first.header.type != ncclWorkTypeUnused)
    goto SkipLoadWork;

  while (true) {
    copyToShmem(&ncclShmem.work, &workFifoDev[workFifoIx], tid, nthreads);
    { // Check whether the last operation was aborted and make sure all threads exit
      int aborted = tid == 0 ? *comm->abortFlag : 0;
      if (barrierReduceAny(aborted)) // publish ncclShmem.work
        break;
      if (tid == 0)
        workFifoHost[workFifoIx].header.type = ncclWorkTypeUnused;
    }

  SkipLoadWork:
/* 中文精读：为什么提前释放 FIFO 槽仍然安全
 * 上面已经把整条 ncclWork 复制到共享内存，当前 block 接下来使用的是私有副本。
 * 因此可以把主机 FIFO type 清零，让 CPU 重用描述符槽；这不授权复用用户数据缓冲。
 * index 持久写回真实设备 channel，下次 kernel 从后续工作继续读取；本次遇 isLast 就退出。
 * 这是按批解释 FIFO 的 kernel，不是程序启动后永远常驻的后台循环。
 */
    workFifoIx = (workFifoIx + 1)%NCCL_MAX_OPS;
    if (tid == 0)
      channel->index = workFifoIx; // write back to real channel, not shmem shadow

    __syncwarp();
    if (ncclShmem.work.header.type == ncclWorkTypeColl) {
      if (tid < NCCL_MAX_WORK_ELEMENTS) ncclRedopPtrDeref(&ncclShmem.work.elems[tid]);
    } else if (ncclShmem.work.header.type == ncclWorkTypeRegColl) {
      if (tid < NCCL_MAX_WORK_ELEMENTS_REG) ncclRedopPtrDeref(&ncclShmem.work.regElems[tid].elem);
    }
    __syncthreads();

/* 中文精读：kernel 入口与实际设备算法可能不同
 * 若当前任务正好匹配编译期 FnIndex，直接实例化调用 RunWork；否则经 ncclFuncs 表间接分派。
 * 本版本为减少入口数量，很多非 LL/Sum 组合仍借用有限的全局 kernel 入口进入解释器。
 * 因此看到 kernel 符号含 LL/Sum，不足以断言所有 work 都实际执行 LL 或 Sum。
 * 应继续检查 work.header.funcIndex 和对应 RunWorkElement 特化。
 */
    if (ncclShmem.work.header.funcIndex == FnIndex)
      RunWork<Fn, T, RedOp, Algo, Proto>().run(&ncclShmem.work);
    else
      ncclFuncs[ncclShmem.work.header.funcIndex]();

    if (ncclShmem.work.header.isLast) break;
    __syncthreads();
  }
}
```

</details>

