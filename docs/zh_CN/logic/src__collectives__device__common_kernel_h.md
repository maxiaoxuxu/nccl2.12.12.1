# src/collectives/device/common_kernel.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0079"></a>

## F0079：min

用途：对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (a &lt; b) ? a : b;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return (a < b) ? a : b; }
```

</details>

<a id="f0080"></a>

## F0080：loadInt

用途：用 volatile 全局加载读取协议偏移/控制值，防止编译器把动态变化值长久缓存在寄存器。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int v;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("ld.volatile.global.u32 %0, [%1];" : "=r"(v) : "l"(ptr));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return v;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int v;
  asm volatile("ld.volatile.global.u32 %0, [%1];"
      : "=r"(v) : "l"(ptr));
  return v;
}
```

</details>

<a id="f0081"></a>

## F0081：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cr.a = fn(cx.a, cy.a);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cr.b = fn(cx.b, cy.b);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    // for char, we do these as vector ops
    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
```

</details>

<a id="f0082"></a>

## F0082：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; int8_t elt[8]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; 8；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>u.elt[i] = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt[i]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      int8_t elt[8];
    } u;
    u.pack = x;
    #pragma unroll
    for (int i=0; i < 8; i++)
      u.elt[i] = FuncTraits<FUNC>().preOp(fn, u.elt[i]);
    return u.pack;
  }
```

</details>

<a id="f0083"></a>

## F0083：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; int8_t elt[8]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; 8；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>u.elt[i] = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt[i]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      int8_t elt[8];
    } u;
    u.pack = x;
    #pragma unroll
    for (int i=0; i < 8; i++)
      u.elt[i] = FuncTraits<FUNC>().postOp(fn, u.elt[i]);
    return u.pack;
  }
```

</details>

<a id="f0084"></a>

## F0084：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cr.a = fn(cx.a, cy.a);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cr.b = fn(cx.b, cy.b);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    // for char, we do these as vector ops
    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
```

</details>

<a id="f0085"></a>

## F0085：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; uint8_t elt[8]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; 8；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>u.elt[i] = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt[i]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      uint8_t elt[8];
    } u;
    u.pack = x;
    #pragma unroll
    for (int i=0; i < 8; i++)
      u.elt[i] = FuncTraits<FUNC>().preOp(fn, u.elt[i]);
    return u.pack;
  }
```

</details>

<a id="f0086"></a>

## F0086：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; uint8_t elt[8]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; 8；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>u.elt[i] = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt[i]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      uint8_t elt[8];
    } u;
    u.pack = x;
    #pragma unroll
    for (int i=0; i < 8; i++)
      u.elt[i] = FuncTraits<FUNC>().postOp(fn, u.elt[i]);
    return u.pack;
  }
```

</details>

<a id="f0087"></a>

## F0087：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cr.a = fn(cx.a, cy.a);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cr.b = fn(cx.b, cy.b);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
```

</details>

<a id="f0088"></a>

## F0088：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; int32_t elt[2]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt[0] = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>u.elt[1] = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      int32_t elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);
    return u.pack;
  }
```

</details>

<a id="f0089"></a>

## F0089：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; int32_t elt[2]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt[0] = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>u.elt[1] = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      int32_t elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);
    return u.pack;
  }
```

</details>

<a id="f0090"></a>

## F0090：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cr.a = fn(cx.a, cy.a);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cr.b = fn(cx.b, cy.b);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
```

</details>

<a id="f0091"></a>

## F0091：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; uint32_t elt[2]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt[0] = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>u.elt[1] = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      uint32_t elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);
    return u.pack;
  }
```

</details>

<a id="f0092"></a>

## F0092：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; uint32_t elt[2]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt[0] = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>u.elt[1] = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      uint32_t elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);
    return u.pack;
  }
```

</details>

<a id="f0093"></a>

## F0093：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>Converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>cx.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cy.pack = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cr.h2[0] = fn(cx.h2[0], cy.h2[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cr.h2[1] = fn(cx.h2[1], cy.h2[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return cr.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    Converter cx, cy, cr;
    cx.pack = x;
    cy.pack = y;
    cr.h2[0] = fn(cx.h2[0], cy.h2[0]);
    cr.h2[1] = fn(cx.h2[1], cy.h2[1]);
    return cr.pack;
  }
```

</details>

<a id="f0094"></a>

## F0094：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>Converter c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>c.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>c.h2[0] = FuncTraits&lt;FUNC&gt;().preOp(fn, c.h2[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>c.h2[1] = FuncTraits&lt;FUNC&gt;().preOp(fn, c.h2[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return c.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    Converter c;
    c.pack = x;
    c.h2[0] = FuncTraits<FUNC>().preOp(fn, c.h2[0]);
    c.h2[1] = FuncTraits<FUNC>().preOp(fn, c.h2[1]);
    return c.pack;
  }
```

</details>

<a id="f0095"></a>

## F0095：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>Converter c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>c.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>c.h2[0] = FuncTraits&lt;FUNC&gt;().postOp(fn, c.h2[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>c.h2[1] = FuncTraits&lt;FUNC&gt;().postOp(fn, c.h2[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return c.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    Converter c;
    c.pack = x;
    c.h2[0] = FuncTraits<FUNC>().postOp(fn, c.h2[0]);
    c.h2[1] = FuncTraits<FUNC>().postOp(fn, c.h2[1]);
    return c.pack;
  }
```

</details>

<a id="f0096"></a>

## F0096：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>Converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>cx.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cy.pack = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cr.h2[0] = fn(cx.h2[0], cy.h2[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cr.h2[1] = fn(cx.h2[1], cy.h2[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return cr.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    Converter cx, cy, cr;
    cx.pack = x;
    cy.pack = y;
    cr.h2[0] = fn(cx.h2[0], cy.h2[0]);
    cr.h2[1] = fn(cx.h2[1], cy.h2[1]);
    return cr.pack;
  }
```

</details>

<a id="f0097"></a>

## F0097：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>Converter c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>c.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>c.h2[0] = FuncTraits&lt;FUNC&gt;().preOp(fn, c.h2[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>c.h2[1] = FuncTraits&lt;FUNC&gt;().preOp(fn, c.h2[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return c.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    Converter c;
    c.pack = x;
    c.h2[0] = FuncTraits<FUNC>().preOp(fn, c.h2[0]);
    c.h2[1] = FuncTraits<FUNC>().preOp(fn, c.h2[1]);
    return c.pack;
  }
```

</details>

<a id="f0098"></a>

## F0098：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>Converter c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>c.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>c.h2[0] = FuncTraits&lt;FUNC&gt;().postOp(fn, c.h2[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>c.h2[1] = FuncTraits&lt;FUNC&gt;().postOp(fn, c.h2[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return c.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    Converter c;
    c.pack = x;
    c.h2[0] = FuncTraits<FUNC>().postOp(fn, c.h2[0]);
    c.h2[1] = FuncTraits<FUNC>().postOp(fn, c.h2[1]);
    return c.pack;
  }
```

</details>

<a id="f0099"></a>

## F0099：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cr.a = fn(cx.a, cy.a);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cr.b = fn(cx.b, cy.b);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;

    cr.a = fn(cx.a, cy.a);
    cr.b = fn(cx.b, cy.b);

    return cr.storage;
  }
```

</details>

<a id="f0100"></a>

## F0100：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; float elt[2]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt[0] = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>u.elt[1] = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      float elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().preOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().preOp(fn, u.elt[1]);
    return u.pack;
  }
```

</details>

<a id="f0101"></a>

## F0101：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; float elt[2]; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt[0] = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt[0]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>u.elt[1] = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt[1]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      float elt[2];
    } u;
    u.pack = x;
    u.elt[0] = FuncTraits<FUNC>().postOp(fn, u.elt[0]);
    u.elt[1] = FuncTraits<FUNC>().postOp(fn, u.elt[1]);
    return u.pack;
  }
```

</details>

<a id="f0102"></a>

## F0102：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>double rv = fn(__longlong_as_double(x), __longlong_as_double(y));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 函数返回 | <code>return __double_as_longlong(rv);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    double rv = fn(__longlong_as_double(x), __longlong_as_double(y));
    return __double_as_longlong(rv);
  }
```

</details>

<a id="f0103"></a>

## F0103：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; double elt; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      double elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);
    return u.pack;
  }
```

</details>

<a id="f0104"></a>

## F0104：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; double elt; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      double elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);
    return u.pack;
  }
```

</details>

<a id="f0105"></a>

## F0105：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>uint64_t rv = fn(x, y);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    uint64_t rv = fn(x, y);
    return rv;
  }
```

</details>

<a id="f0106"></a>

## F0106：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; uint64_t elt; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      uint64_t elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);
    return u.pack;
  }
```

</details>

<a id="f0107"></a>

## F0107：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; uint64_t elt; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      uint64_t elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);
    return u.pack;
  }
```

</details>

<a id="f0108"></a>

## F0108：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int64_t rv = fn((int64_t)x, (int64_t)y);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    int64_t rv = fn((int64_t)x, (int64_t)y);
    return rv;
  }
```

</details>

<a id="f0109"></a>

## F0109：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; int64_t elt; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt = FuncTraits&lt;FUNC&gt;().preOp(fn, u.elt);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      int64_t elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().preOp(fn, u.elt);
    return u.pack;
  }
```

</details>

<a id="f0110"></a>

## F0110：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>union { PackType pack; int64_t elt; } u;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>u.pack = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>u.elt = FuncTraits&lt;FUNC&gt;().postOp(fn, u.elt);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return u.pack;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    union {
      PackType pack;
      int64_t elt;
    } u;
    u.pack = x;
    u.elt = FuncTraits<FUNC>().postOp(fn, u.elt);
    return u.pack;
  }
```

</details>

<a id="f0111"></a>

## F0111：vFetch

用途：读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return *ptr;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return *ptr;
}
```

</details>

<a id="f0112"></a>

## F0112：vStore

用途：把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*ptr = val;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *ptr = val;
}
```

</details>

<a id="f0113"></a>

## F0113：vFetch<half>

用途：读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>half r;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>r.x = ptr-&gt;x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return r;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  half r;
  r.x = ptr->x;
  return r;
}
```

</details>

<a id="f0114"></a>

## F0114：vStore<half>

用途：把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>ptr-&gt;x = val.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ptr->x = val.x;
}
```

</details>

<a id="f0115"></a>

## F0115：vFetch<half>

用途：读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>half r;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>r = ((half*)ptr)[0];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return r;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  half r;
  r = ((half*)ptr)[0];
  return r;
}
```

</details>

<a id="f0116"></a>

## F0116：vStore<half>

用途：把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>((half*)ptr)[0] = val;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ((half*)ptr)[0] = val;
}
```

</details>

<a id="f0117"></a>

## F0117：vFetch<__nv_bfloat16>

用途：以 BF16 类型特化读取用户/协议缓冲元素，处理该类型的 volatile 访问写法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>__nv_bfloat16 r;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>r = ((__nv_bfloat16*)ptr)[0];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return r;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  __nv_bfloat16 r;
  r = ((__nv_bfloat16*)ptr)[0];
  return r;
}
```

</details>

<a id="f0118"></a>

## F0118：vStore<__nv_bfloat16>

用途：把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>((__nv_bfloat16*)ptr)[0] = val;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  ((__nv_bfloat16*)ptr)[0] = val;
}
```

</details>

<a id="f0119"></a>

## F0119：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>x.x = MULTI&lt;FUNC, T&gt;()(fn, x.x, y.x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>x.y = MULTI&lt;FUNC, T&gt;()(fn, x.y, y.y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    x.x = MULTI<FUNC, T>()(fn, x.x, y.x);
    x.y = MULTI<FUNC, T>()(fn, x.y, y.y);
  }
```

</details>

<a id="f0120"></a>

## F0120：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>x.x = MULTI&lt;FUNC, T&gt;().preOp(fn, x.x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>x.y = MULTI&lt;FUNC, T&gt;().preOp(fn, x.y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    x.x = MULTI<FUNC, T>().preOp(fn, x.x);
    x.y = MULTI<FUNC, T>().preOp(fn, x.y);
  }
```

</details>

<a id="f0121"></a>

## F0121：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>x.x = MULTI&lt;FUNC, T&gt;().postOp(fn, x.x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>x.y = MULTI&lt;FUNC, T&gt;().postOp(fn, x.y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    x.x = MULTI<FUNC, T>().postOp(fn, x.x);
    x.y = MULTI<FUNC, T>().postOp(fn, x.y);
  }
```

</details>

<a id="f0122"></a>

## F0122：Fetch128

用途：一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("ld.volatile.global.v2.u64 {%0,%1}, [%2];" : "=l"(v.x), "=l"(v.y) : "l"(p) : "memory");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  asm volatile("ld.volatile.global.v2.u64 {%0,%1}, [%2];" : "=l"(v.x), "=l"(v.y) : "l"(p) : "memory");
}
```

</details>

<a id="f0123"></a>

## F0123：Store128

用途：把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("st.volatile.global.v2.u64 [%0], {%1,%2};" :: "l"(p), "l"(v.x), "l"(v.y) : "memory");</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  asm volatile("st.volatile.global.v2.u64 [%0], {%1,%2};" :: "l"(p), "l"(v.x), "l"(v.y) : "memory");
}
```

</details>

<a id="f0124"></a>

## F0124：ReduceCopyMulti

用途：线程按 warp/展开因子分工，逐类型加载多个源，在寄存器中归约后写入各目标，处理标量与尾部路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const Int inc = nw * UNROLL * WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>Int offset = w * UNROLL * WARP_SIZE + t;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const T* srcs[MAXSRCS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAXSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>srcs[i] = s[i]+elemOffset+offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 准备局部状态 | <code>T* dsts[MAXDSTS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAXDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>dsts[i] = d[i]+elemOffset+offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 循环 | <code>初始 ；继续条件 (offset &lt; Nelem)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 准备局部状态 | <code>T vals[UNROLL];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>vals[u] = vFetch(srcs[0]+u*WARP_SIZE);</code> | 读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。 |
| 15 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 1 / 条件判断 | <code>(PreOpN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 3 / 准备局部状态 | <code>FUNC fn(redOpArgs[0]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 3 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>vals[u] = FuncTraits&lt;FUNC&gt;().preOp(fn, vals[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 1 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 23 | 1 / 循环 | <code>初始 int i=1;；继续条件 i&lt;MINSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 2 / 准备局部状态 | <code>T vals2[UNROLL];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 2 / 准备局部状态 | <code>FUNC fn(redOpArgs[i]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 2 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>vals2[u] = vFetch(srcs[i]+u*WARP_SIZE);</code> | 读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。 |
| 28 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 29 | 2 / 条件判断 | <code>(i&lt;PreOpN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 4 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 32 | 5 / 调用 / 计算 / 写入 | <code>vals2[u] = FuncTraits&lt;FUNC&gt;().preOp(fn, vals2[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 34 | 2 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>vals[u] = fn(vals[u], vals2[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 37 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 1 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 39 | 1 / 循环 | <code>初始 int i=MINSRCS;；继续条件 i&lt;MAXSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 40 | 2 / 条件判断 | <code>(i&lt;nsrcs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 4 / 准备局部状态 | <code>T vals2[UNROLL];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 4 / 准备局部状态 | <code>FUNC fn(redOpArgs[i]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 4 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 45 | 5 / 调用 / 计算 / 写入 | <code>vals2[u] = vFetch(srcs[i]+u*WARP_SIZE);</code> | 读取一个具有协议所需可见性语义的元素，半精度/BF16 特化处理该类型的访问限制。 |
| 46 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 4 / 条件判断 | <code>(i&lt;PreOpN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 6 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 50 | 7 / 调用 / 计算 / 写入 | <code>vals2[u] = FuncTraits&lt;FUNC&gt;().preOp(fn, vals2[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 52 | 4 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 53 | 5 / 调用 / 计算 / 写入 | <code>vals[u] = fn(vals[u], vals2[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 55 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 56 | 1 / 条件判断 | <code>(postOp)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 3 / 准备局部状态 | <code>FUNC fn(redOpArgs[0]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 3 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 60 | 3 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 61 | 4 / 调用 / 计算 / 写入 | <code>vals[u] = FuncTraits&lt;FUNC&gt;().postOp(fn, vals[u]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 63 | 1 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 64 | 1 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; MINDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 65 | 2 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 66 | 3 / 调用 / 计算 / 写入 | <code>vStore(dsts[i]+u*WARP_SIZE, vals[u]);</code> | 把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。 |
| 67 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 68 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 69 | 1 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 70 | 1 / 循环 | <code>初始 int i=MINDSTS;；继续条件 i&lt;MAXDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 71 | 2 / 条件判断 | <code>(i&lt;ndsts)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 72 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 73 | 4 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 74 | 5 / 调用 / 计算 / 写入 | <code>vStore(dsts[i]+u*WARP_SIZE, vals[u]);</code> | 把一个元素存入协议目标，类型特化保证半精度/BF16 能使用适合其表示的写法。 |
| 75 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 76 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 77 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAXSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 78 | 2 / 调用 / 计算 / 写入 | <code>srcs[i] += inc;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 80 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAXDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 81 | 2 / 调用 / 计算 / 写入 | <code>dsts[i] += inc;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 83 | 1 / 调用 / 计算 / 写入 | <code>offset += inc;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const Int inc = nw * UNROLL * WARP_SIZE;
  Int offset = w * UNROLL * WARP_SIZE + t;

  const T* srcs[MAXSRCS];
  for (int i=0; i<MAXSRCS; i++) srcs[i] = s[i]+elemOffset+offset;
  T* dsts[MAXDSTS];
  for (int i=0; i<MAXDSTS; i++) dsts[i] = d[i]+elemOffset+offset;

  while (offset < Nelem) {
    T vals[UNROLL];
    // Load and reduce
    for (int u = 0; u < UNROLL; ++u) vals[u] = vFetch(srcs[0]+u*WARP_SIZE);
    if (PreOpN) {
      FUNC fn(redOpArgs[0]);
      for (int u = 0; u < UNROLL; ++u) vals[u] = FuncTraits<FUNC>().preOp(fn, vals[u]);
    }

    #pragma unroll
    for (int i=1; i<MINSRCS; i++) {
      T vals2[UNROLL];
      FUNC fn(redOpArgs[i]);
      for (int u = 0; u < UNROLL; ++u) vals2[u] = vFetch(srcs[i]+u*WARP_SIZE);
      if (i<PreOpN) {
        for (int u = 0; u < UNROLL; ++u) vals2[u] = FuncTraits<FUNC>().preOp(fn, vals2[u]);
      }
      for (int u = 0; u < UNROLL; ++u) vals[u] = fn(vals[u], vals2[u]);
    }
    #pragma unroll
    for (int i=MINSRCS; i<MAXSRCS; i++) {
      if (i<nsrcs) {
        T vals2[UNROLL];
        FUNC fn(redOpArgs[i]);
        for (int u = 0; u < UNROLL; ++u) vals2[u] = vFetch(srcs[i]+u*WARP_SIZE);
        if (i<PreOpN) {
          for (int u = 0; u < UNROLL; ++u) vals2[u] = FuncTraits<FUNC>().preOp(fn, vals2[u]);
        }
        for (int u = 0; u < UNROLL; ++u) vals[u] = fn(vals[u], vals2[u]);
      }
    }

    if (postOp) {
      FUNC fn(redOpArgs[0]);
      #pragma unroll
      for (int u = 0; u < UNROLL; ++u) vals[u] = FuncTraits<FUNC>().postOp(fn, vals[u]);
    }

    // Store
    #pragma unroll
    for (int i = 0; i < MINDSTS; i++) {
      for (int u = 0; u < UNROLL; ++u) vStore(dsts[i]+u*WARP_SIZE, vals[u]);
    }
    #pragma unroll
    for (int i=MINDSTS; i<MAXDSTS; i++) {
      if (i<ndsts) {
        for (int u = 0; u < UNROLL; ++u) vStore(dsts[i]+u*WARP_SIZE, vals[u]);
      }
    }
    for (int i=0; i<MAXSRCS; i++) srcs[i] += inc;
    for (int i=0; i<MAXDSTS; i++) dsts[i] += inc;
    offset += inc;
  }
}
```

</details>

<a id="f0125"></a>

## F0125：ReduceCopy128bMulti

用途：在 16 字节对齐快路径中加载多个 128 位包、归约并多目标写出，利用向量指令和循环展开。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const Int inc = nw * UNROLL * WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>Int offset = w * UNROLL * WARP_SIZE + t;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const Pack128* srcs[MAXSRCS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAXSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>srcs[i] = ((const Pack128*)(s[i]+elemOffset))+offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 7 | 0 / 准备局部状态 | <code>Pack128* dsts[MAXDSTS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAXDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>dsts[i] = ((Pack128*)(d[i]+elemOffset))+offset;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 0 / 循环 | <code>初始 ；继续条件 (offset &lt; Npack)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 准备局部状态 | <code>Pack128 vals[UNROLL];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>Fetch128(vals[u], srcs[0]+u*WARP_SIZE);</code> | 一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。 |
| 15 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 1 / 条件判断 | <code>(PreOpN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 3 / 准备局部状态 | <code>FUNC fn(redOpArgs[0]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 3 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>MULTI128&lt;FUNC, T&gt;().preOp(fn, vals[u]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 1 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 23 | 1 / 循环 | <code>初始 int i=1;；继续条件 i&lt;MINSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 2 / 准备局部状态 | <code>Pack128 vals2[UNROLL];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 2 / 准备局部状态 | <code>FUNC fn(redOpArgs[i]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 2 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>Fetch128(vals2[u], srcs[i]+u*WARP_SIZE);</code> | 一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。 |
| 28 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 29 | 2 / 条件判断 | <code>(i&lt;PreOpN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 4 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 32 | 5 / 调用 / 计算 / 写入 | <code>MULTI128&lt;FUNC, T&gt;().preOp(fn, vals2[u]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 34 | 2 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 35 | 3 / 调用 / 计算 / 写入 | <code>MULTI128&lt;FUNC, T&gt;()(fn, vals[u], vals2[u]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 37 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 38 | 1 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 39 | 1 / 循环 | <code>初始 int i=MINSRCS;；继续条件 i&lt;MAXSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 40 | 2 / 条件判断 | <code>(i&lt;nsrcs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 4 / 准备局部状态 | <code>Pack128 vals2[UNROLL];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 4 / 准备局部状态 | <code>FUNC fn(redOpArgs[i]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 4 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 45 | 5 / 调用 / 计算 / 写入 | <code>Fetch128(vals2[u], srcs[i]+u*WARP_SIZE);</code> | 一次加载两个 64 位寄存器组成的 16 字节数据包，用于向量化搬运。 |
| 46 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 4 / 条件判断 | <code>(i&lt;PreOpN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 6 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 50 | 7 / 调用 / 计算 / 写入 | <code>MULTI128&lt;FUNC, T&gt;().preOp(fn, vals2[u]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 51 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 52 | 4 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 53 | 5 / 调用 / 计算 / 写入 | <code>MULTI128&lt;FUNC, T&gt;()(fn, vals[u], vals2[u]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 54 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 55 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 56 | 1 / 条件判断 | <code>(postOp)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 3 / 准备局部状态 | <code>FUNC fn(redOpArgs[0]);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 3 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 60 | 3 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 61 | 4 / 调用 / 计算 / 写入 | <code>MULTI128&lt;FUNC, T&gt;().postOp(fn, vals[u]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 62 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 63 | 1 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 64 | 1 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; MINDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 65 | 2 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 66 | 3 / 调用 / 计算 / 写入 | <code>Store128(dsts[i]+u*WARP_SIZE, vals[u]);</code> | 把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。 |
| 67 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 68 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 69 | 1 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 70 | 1 / 循环 | <code>初始 int i=MINDSTS;；继续条件 i&lt;MAXDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 71 | 2 / 条件判断 | <code>(i&lt;ndsts)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 72 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 73 | 4 / 循环 | <code>初始 int u = 0;；继续条件 u &lt; UNROLL；每轮更新 ++u</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 74 | 5 / 调用 / 计算 / 写入 | <code>Store128(dsts[i]+u*WARP_SIZE, vals[u]);</code> | 把两个 64 位寄存器组成的 16 字节包写到全局目标，减少标量访存指令数量。 |
| 75 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 76 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 77 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAXSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 78 | 2 / 调用 / 计算 / 写入 | <code>srcs[i] += inc;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 80 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAXDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 81 | 2 / 调用 / 计算 / 写入 | <code>dsts[i] += inc;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 83 | 1 / 调用 / 计算 / 写入 | <code>offset += inc;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 84 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const Int inc = nw * UNROLL * WARP_SIZE;
  Int offset = w * UNROLL * WARP_SIZE + t;

  const Pack128* srcs[MAXSRCS];
  for (int i=0; i<MAXSRCS; i++) srcs[i] = ((const Pack128*)(s[i]+elemOffset))+offset;
  Pack128* dsts[MAXDSTS];
  for (int i=0; i<MAXDSTS; i++) dsts[i] = ((Pack128*)(d[i]+elemOffset))+offset;

  while (offset < Npack) {
    Pack128 vals[UNROLL];
    // Load and reduce
    for (int u = 0; u < UNROLL; ++u) Fetch128(vals[u], srcs[0]+u*WARP_SIZE);
    if (PreOpN) {
      FUNC fn(redOpArgs[0]);
      for (int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>().preOp(fn, vals[u]);
    }

    #pragma unroll
    for (int i=1; i<MINSRCS; i++) {
      Pack128 vals2[UNROLL];
      FUNC fn(redOpArgs[i]);
      for (int u = 0; u < UNROLL; ++u) Fetch128(vals2[u], srcs[i]+u*WARP_SIZE);
      if (i<PreOpN) {
        for (int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>().preOp(fn, vals2[u]);
      }
      for (int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>()(fn, vals[u], vals2[u]);
    }
    #pragma unroll
    for (int i=MINSRCS; i<MAXSRCS; i++) {
      if (i<nsrcs) {
        Pack128 vals2[UNROLL];
        FUNC fn(redOpArgs[i]);
        for (int u = 0; u < UNROLL; ++u) Fetch128(vals2[u], srcs[i]+u*WARP_SIZE);
        if (i<PreOpN) {
          for (int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>().preOp(fn, vals2[u]);
        }
        for (int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>()(fn, vals[u], vals2[u]);
      }
    }

    if (postOp) {
      FUNC fn(redOpArgs[0]);
      #pragma unroll
      for (int u = 0; u < UNROLL; ++u) MULTI128<FUNC, T>().postOp(fn, vals[u]);
    }

    // Store
    #pragma unroll
    for (int i = 0; i < MINDSTS; i++) {
      for (int u = 0; u < UNROLL; ++u) Store128(dsts[i]+u*WARP_SIZE, vals[u]);
    }
    #pragma unroll
    for (int i=MINDSTS; i<MAXDSTS; i++) {
      if (i<ndsts) {
        for (int u = 0; u < UNROLL; ++u) Store128(dsts[i]+u*WARP_SIZE, vals[u]);
      }
    }
    for (int i=0; i<MAXSRCS; i++) srcs[i] += inc;
    for (int i=0; i<MAXDSTS; i++) dsts[i] += inc;
    offset += inc;
  }
}
```

</details>

<a id="f0126"></a>

## F0126：ptrAlign128

用途：返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (uint64_t)ptr % alignof(Pack128);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return (uint64_t)ptr % alignof(Pack128); }
```

</details>

<a id="f0127"></a>

## F0127：ReduceOrCopyMulti

用途：统一选择 128-bit 对齐主路径与逐类型尾部路径，融合多源归约及多目标复制。

- 先检查全部有效源目标的16B对齐，条件满足才走128bit包快路径。
- 依次处理完整展开包、少量不展开包、标量展开与最后余数，任何路径都不能漏尾元素。
- 归约发生在寄存器，目标可以是本地输出和多个下游地址；上层已负责流控与可见性。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>Int Nrem = N;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(Nrem &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int w = tid / WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>int nw = nthreads / WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>int t = tid % WARP_SIZE;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>int align = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 10 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MINSRCS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>align &#124;= ptrAlign128(srcs[i]);</code> | 返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 循环 | <code>初始 int i=MINSRCS;；继续条件 i&lt;MAXSRCS && i&lt;nsrcs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>align &#124;= ptrAlign128(srcs[i]);</code> | 返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。 |
| 15 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 0 / 编译期分支/指令 | <code>#pragma unroll</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 17 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MINDSTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 18 | 1 / 调用 / 计算 / 写入 | <code>align &#124;= ptrAlign128(dsts[i]);</code> | 返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。 |
| 19 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 20 | 0 / 循环 | <code>初始 int i=MINDSTS;；继续条件 i&lt;MAXDSTS && i&lt;ndsts；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 21 | 1 / 调用 / 计算 / 写入 | <code>align &#124;= ptrAlign128(dsts[i]);</code> | 返回指针相对 128-bit 对齐的余数，供搬运内核决定能否走向量化快路径。 |
| 22 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 23 | 0 / 准备局部状态 | <code>Int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 0 / 条件判断 | <code>(align == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 2 / 准备局部状态 | <code>Int Npack = (Nrem / (PACKELEMS*UNROLL*WARP_SIZE)) * (UNROLL*WARP_SIZE);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 2 / 准备局部状态 | <code>Int Nelem = Npack * PACKELEMS;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>ReduceCopy128bMulti&lt;FUNC, T, UNROLL, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN&gt; (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Npack);</code> | 在 16 字节对齐快路径中加载多个 128 位包、归约并多目标写出，利用向量指令和循环展开。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>Nrem -= Nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 2 / 条件判断 | <code>(Nrem == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 4 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>offset += Nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 2 / 调用 / 计算 / 写入 | <code>Npack = Nrem / PACKELEMS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>Nelem = Npack * PACKELEMS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>ReduceCopy128bMulti&lt;FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN&gt; (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Npack);</code> | 在 16 字节对齐快路径中加载多个 128 位包、归约并多目标写出，利用向量指令和循环展开。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>Nrem -= Nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 2 / 条件判断 | <code>(Nrem == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 4 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 41 | 2 / 调用 / 计算 / 写入 | <code>offset += Nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 准备局部状态 | <code>Int Nelem = (Nrem / (UNROLL*PACKELEMS/2*WARP_SIZE)) * (UNROLL*PACKELEMS/2*WARP_SIZE);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>ReduceCopyMulti&lt;FUNC, T, UNROLL*PACKELEMS/2, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN&gt; (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Nelem);</code> | 线程按 warp/展开因子分工，逐类型加载多个源，在寄存器中归约后写入各目标，处理标量与尾部路径。 |
| 44 | 0 / 调用 / 计算 / 写入 | <code>Nrem -= Nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 0 / 条件判断 | <code>(Nrem == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 46 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 47 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>offset += Nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 0 / 调用 / 计算 / 写入 | <code>ReduceCopyMulti&lt;FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN&gt; (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Nrem);</code> | 线程按 warp/展开因子分工，逐类型加载多个源，在寄存器中归约后写入各目标，处理标量与尾部路径。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  Int Nrem = N;
  if (Nrem <= 0) return;

  int w = tid / WARP_SIZE;       // Warp number
  int nw = nthreads / WARP_SIZE; // Number of warps
  int t = tid % WARP_SIZE;       // Thread (inside the warp)

  // Check that all is 16B aligned. If not don't use 16B load/stores.
  int align = 0;
  #pragma unroll
  for (int i=0; i<MINSRCS; i++) align |= ptrAlign128(srcs[i]);
  for (int i=MINSRCS; i<MAXSRCS && i<nsrcs; i++) align |= ptrAlign128(srcs[i]);
  #pragma unroll
  for (int i=0; i<MINDSTS; i++) align |= ptrAlign128(dsts[i]);
  for (int i=MINDSTS; i<MAXDSTS && i<ndsts; i++) align |= ptrAlign128(dsts[i]);

  Int offset = 0;
  if (align == 0) {
    // fast path: use 128b loads/stores to do the bulk of the work,
    // assuming the pointers we have are all 128-bit aligned.

    // main loop
    Int Npack = (Nrem / (PACKELEMS*UNROLL*WARP_SIZE)) * (UNROLL*WARP_SIZE); // round down
    Int Nelem = Npack * PACKELEMS;

    ReduceCopy128bMulti<FUNC, T, UNROLL, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>
      (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Npack);

    Nrem -= Nelem;
    if (Nrem == 0) return;
    offset += Nelem;

    // slightly less optimized for section when we don't have full unrolling
    Npack = Nrem / PACKELEMS;
    Nelem = Npack * PACKELEMS;

    ReduceCopy128bMulti<FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>
      (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Npack);

    Nrem -= Nelem;
    if (Nrem == 0) return;
    offset += Nelem;
  }

  // unrolled, by-type (mostly for unaligned buffers)
  Int Nelem = (Nrem / (UNROLL*PACKELEMS/2*WARP_SIZE)) * (UNROLL*PACKELEMS/2*WARP_SIZE); // round down

  ReduceCopyMulti<FUNC, T, UNROLL*PACKELEMS/2, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>
    (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Nelem);

  Nrem -= Nelem;
  if (Nrem == 0) return;
  offset += Nelem;

  // no unroll, by type. Should finish what's remaining.
  ReduceCopyMulti<FUNC, T, 1, MINSRCS, MAXSRCS, MINDSTS, MAXDSTS, PreOpN>
    (w, nw, t, redOpArgs, postOp, nsrcs, srcs, ndsts, dsts, offset, Nrem);
}
```

</details>

