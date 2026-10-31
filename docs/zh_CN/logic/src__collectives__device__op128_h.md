# src/collectives/device/op128.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0130"></a>

## F0130：load128

用途：一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("ld.volatile.global.v2.u64 {%0,%1}, [%2];" : "=l"(v0), "=l"(v1) : "l"(ptr));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  asm volatile("ld.volatile.global.v2.u64 {%0,%1}, [%2];"
      : "=l"(v0), "=l"(v1) : "l"(ptr));
}
```

</details>

<a id="f0131"></a>

## F0131：store128

用途：把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("st.volatile.global.v2.u64 [%2], {%0,%1};" :: "l"(v0), "l"(v1), "l"(ptr));</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  asm volatile("st.volatile.global.v2.u64 [%2], {%0,%1};"
      :: "l"(v0), "l"(v1), "l"(ptr));
}
```

</details>

<a id="f0132"></a>

## F0132：shmemCvtPtr

用途：把 CUDA 通用指针转换为 PTX shared 地址空间指针，供共享内存汇编指令使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>uint64_t* shmemAsmPtr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("cvta.to.shared.u64 %0, %1;" : "=l"(shmemAsmPtr) : "l"(shmemGenericPtr));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return shmemAsmPtr;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  uint64_t* shmemAsmPtr;
  asm volatile("cvta.to.shared.u64 %0, %1;" : "=l"(shmemAsmPtr) : "l"(shmemGenericPtr));
  return shmemAsmPtr;
}
```

</details>

<a id="f0133"></a>

## F0133：loadShmem128

用途：从 SM 共享内存读取 16 字节，避免把共享内存地址误交给全局加载指令。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("ld.volatile.shared.v2.u64 {%0,%1}, [%2];" : "=l"(v0), "=l"(v1) : "l"(shmemAsmPtr));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  asm volatile("ld.volatile.shared.v2.u64 {%0,%1}, [%2];"
      : "=l"(v0), "=l"(v1) : "l"(shmemAsmPtr));
}
```

</details>

<a id="f0134"></a>

## F0134：storeShmem128

用途：向 SM 共享内存写出 16 字节，供同 block 的线程在屏障后消费。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("st.volatile.shared.v2.u64 [%2], {%0,%1};" :: "l"(v0), "l"(v1), "l"(shmemAsmPtr));</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  asm volatile("st.volatile.shared.v2.u64 [%2], {%0,%1};"
      :: "l"(v0), "l"(v1), "l"(shmemAsmPtr));
}
```

</details>

<a id="f0135"></a>

## F0135：loadShmemMisaligned128

用途：按元素宽度拼接可能不对齐的共享内存数据到两个 64 位寄存器。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>union { uint32_t tmp4[4]; uint64_t tmp8[2]; }</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 条件判断 | <code>(sizeof(T) &lt; 4)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 准备局部状态 | <code>uint32_t *ptr4 = reinterpret_cast&lt;uint32_t*&gt;(reinterpret_cast&lt;uintptr_t&gt;(ptr) & -uintptr_t(4));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 2 / 循环 | <code>初始 int e=0;；继续条件 e &lt; 4；每轮更新 e++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 3 / 准备局部状态 | <code>uint32_t lo, hi;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>asm("ld.shared.b32 %0,[%1];" : "=r"(lo) : "l"(ptr4+e+0));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>asm("ld.shared.b32 %0,[%1];" : "=r"(hi) : "l"(ptr4+e+1));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>tmp4[e] = __funnelshift_r(lo, hi, 8*(int(reinterpret_cast&lt;uintptr_t&gt;(ptr))%4));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 2 / 条件判断 | <code>(sizeof(T) == 4)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 16 | 4 / 循环 | <code>初始 int e=0;；继续条件 e &lt; 4；每轮更新 e++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 5 / 调用 / 计算 / 写入 | <code>asm("ld.shared.b32 %0,[%1];" : "=r"(tmp4[e]) : "l"(ptr+e));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 20 | 4 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 21 | 4 / 循环 | <code>初始 int e=0;；继续条件 e &lt; 2；每轮更新 e++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 22 | 5 / 调用 / 计算 / 写入 | <code>asm("ld.shared.b64 %0,[%1];" : "=l"(tmp8[e]) : "l"(ptr+e));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>v0 = tmp8[0];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>v1 = tmp8[1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  union {
    uint32_t tmp4[4];
    uint64_t tmp8[2];
  };
  if(sizeof(T) < 4) {
    uint32_t *ptr4 = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(ptr) & -uintptr_t(4));
    #pragma unroll
    for(int e=0; e < 4; e++) {
      // Produce 4 bytes of sub-register type by reading 2 4-byte
      // aligned values and shifting.
      uint32_t lo, hi;
      asm("ld.shared.b32 %0,[%1];" : "=r"(lo) : "l"(ptr4+e+0));
      asm("ld.shared.b32 %0,[%1];" : "=r"(hi) : "l"(ptr4+e+1));
      tmp4[e] = __funnelshift_r(lo, hi, 8*(int(reinterpret_cast<uintptr_t>(ptr))%4));
    }
  }
  else if(sizeof(T) == 4) {
    #pragma unroll
    for(int e=0; e < 4; e++)
      asm("ld.shared.b32 %0,[%1];" : "=r"(tmp4[e]) : "l"(ptr+e));
  }
  else /*sizeof(T)==8*/ {
    #pragma unroll
    for(int e=0; e < 2; e++)
      asm("ld.shared.b64 %0,[%1];" : "=l"(tmp8[e]) : "l"(ptr+e));
  }
  v0 = tmp8[0];
  v1 = tmp8[1];
}
```

</details>

