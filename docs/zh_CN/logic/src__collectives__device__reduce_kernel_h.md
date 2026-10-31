# src/collectives/device/reduce_kernel.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0264"></a>

## F0264：FuncNull

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0265"></a>

## F0265：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return 0;
  }
```

</details>

<a id="f0266"></a>

## F0266：FuncSum

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0267"></a>

## F0267：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x + y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return x + y;
  }
```

</details>

<a id="f0268"></a>

## F0268：FuncProd

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0269"></a>

## F0269：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x * y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return x * y;
  }
```

</details>

<a id="f0270"></a>

## F0270：FuncMax

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0271"></a>

## F0271：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (x &lt; y) ? y : x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return (x < y) ? y : x;
  }
```

</details>

<a id="f0272"></a>

## F0272：FuncMin

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0273"></a>

## F0273：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (x &lt; y) ? x : y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return (x < y) ? x : y;
  }
```

</details>

<a id="f0274"></a>

## F0274：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return x; }
```

</details>

<a id="f0275"></a>

## F0275：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return x; }
```

</details>

<a id="f0276"></a>

## F0276：addChar4

用途：用位掩码分别处理一个寄存器中的四个 8 位加法，避免跨字节进位污染其他元素。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const uint32_t x0 = x & MASK0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const uint32_t x1 = x & MASK1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>const uint32_t y0 = y & MASK0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>const uint32_t y1 = y & MASK1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>const uint32_t r0 = (x0+y0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>const uint32_t r1 = (x1+y1);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 函数返回 | <code>return (r0 & MASK0) &#124; (r1 & MASK1);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  /* This can be used both for signed and unsigned 8-bit addition */
  const uint32_t x0 = x & MASK0;
  const uint32_t x1 = x & MASK1;
  const uint32_t y0 = y & MASK0;
  const uint32_t y1 = y & MASK1;
  const uint32_t r0 = (x0+y0);
  const uint32_t r1 = (x1+y1);
  return (r0 & MASK0) | (r1 & MASK1);
}
```

</details>

<a id="f0277"></a>

## F0277：FuncSum

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0278"></a>

## F0278：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if (__CUDA_ARCH__ &gt;= 300) && (__CUDA_ARCH__ &lt; 500)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>int32_t rv, z=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>asm("vadd4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 1 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 2 / 函数返回 | <code>return addChar4(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
    int32_t rv, z=0;
    asm("vadd4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
    return addChar4(x, y);
#endif
  }
```

</details>

<a id="f0279"></a>

## F0279：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x+y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return x+y;
  }
```

</details>

<a id="f0280"></a>

## F0280：FuncSum

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0281"></a>

## F0281：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if (__CUDA_ARCH__ &gt;= 300) && (__CUDA_ARCH__ &lt; 500)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>int32_t rv, z=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>asm("vadd4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 1 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 2 / 函数返回 | <code>return addChar4(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
    int32_t rv, z=0;
    asm("vadd4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
    return addChar4(x, y);
#endif
  }
```

</details>

<a id="f0282"></a>

## F0282：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x+y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return x+y;
  }
```

</details>

<a id="f0283"></a>

## F0283：mulChar4

用途：对寄存器中四个字节分别执行乘法并重组结果，用于 int8/uint8 向量化归约。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>union converter { uint32_t storage; char4 a; }</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cr.a.x = cx.a.x * cy.a.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>cr.a.y = cx.a.y * cy.a.y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>cr.a.z = cx.a.z * cy.a.z;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>cr.a.w = cx.a.w * cy.a.w;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  /* This can be used both for signed and unsigned 8-bit multiplication */
  union converter { uint32_t storage; char4 a; };
  converter cx, cy, cr;
  cx.storage = x;
  cy.storage = y;
  cr.a.x = cx.a.x * cy.a.x;
  cr.a.y = cx.a.y * cy.a.y;
  cr.a.z = cx.a.z * cy.a.z;
  cr.a.w = cx.a.w * cy.a.w;
  return cr.storage;
}
```

</details>

<a id="f0284"></a>

## F0284：FuncProd

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0285"></a>

## F0285：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return mulChar4(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return mulChar4(x, y);
  }
```

</details>

<a id="f0286"></a>

## F0286：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x*y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return x*y;
  }
```

</details>

<a id="f0287"></a>

## F0287：FuncProd

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0288"></a>

## F0288：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return mulChar4(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return mulChar4(x, y);
  }
```

</details>

<a id="f0289"></a>

## F0289：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x*y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return x*y;
  }
```

</details>

<a id="f0290"></a>

## F0290：FuncMax

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0291"></a>

## F0291：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if (__CUDA_ARCH__ &gt;= 300) && (__CUDA_ARCH__ &lt; 500)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>int32_t rv, z=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>asm("vmax4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 1 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 2 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>cr.a.x = max(cx.a.x, cy.a.x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>cr.a.y = max(cx.a.y, cy.a.y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>cr.a.z = max(cx.a.z, cy.a.z);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>cr.a.w = max(cx.a.w, cy.a.w);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
    int32_t rv, z=0;
    asm("vmax4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;
    cr.a.x = max(cx.a.x, cy.a.x);
    cr.a.y = max(cx.a.y, cy.a.y);
    cr.a.z = max(cx.a.z, cy.a.z);
    cr.a.w = max(cx.a.w, cy.a.w);
    return cr.storage;
#endif
  }
```

</details>

<a id="f0292"></a>

## F0292：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (x&gt;y) ? x : y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return (x>y) ? x : y;
  }
```

</details>

<a id="f0293"></a>

## F0293：FuncMax

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0294"></a>

## F0294：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if (__CUDA_ARCH__ &gt;= 300) && (__CUDA_ARCH__ &lt; 500)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>int32_t rv, z=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>asm("vmax4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 1 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 2 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>cr.a.x = max(cx.a.x, cy.a.x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>cr.a.y = max(cx.a.y, cy.a.y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>cr.a.z = max(cx.a.z, cy.a.z);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>cr.a.w = max(cx.a.w, cy.a.w);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
    int32_t rv, z=0;
    asm("vmax4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;
    cr.a.x = max(cx.a.x, cy.a.x);
    cr.a.y = max(cx.a.y, cy.a.y);
    cr.a.z = max(cx.a.z, cy.a.z);
    cr.a.w = max(cx.a.w, cy.a.w);
    return cr.storage;
#endif
  }
```

</details>

<a id="f0295"></a>

## F0295：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (x&gt;y) ? x : y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return (x>y) ? x : y;
  }
```

</details>

<a id="f0296"></a>

## F0296：FuncMin

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0297"></a>

## F0297：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if (__CUDA_ARCH__ &gt;= 300) && (__CUDA_ARCH__ &lt; 500)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>int32_t rv, z=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>asm("vmin4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 1 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 2 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>cr.a.x = min(cx.a.x, cy.a.x);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>cr.a.y = min(cx.a.y, cy.a.y);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>cr.a.z = min(cx.a.z, cy.a.z);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>cr.a.w = min(cx.a.w, cy.a.w);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 13 | 2 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
    int32_t rv, z=0;
    asm("vmin4.s32.s32.s32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;
    cr.a.x = min(cx.a.x, cy.a.x);
    cr.a.y = min(cx.a.y, cy.a.y);
    cr.a.z = min(cx.a.z, cy.a.z);
    cr.a.w = min(cx.a.w, cy.a.w);
    return cr.storage;
#endif
  }
```

</details>

<a id="f0298"></a>

## F0298：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (x&lt;y) ? x : y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return (x<y) ? x : y;
  }
```

</details>

<a id="f0299"></a>

## F0299：FuncMin

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0300"></a>

## F0300：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if (__CUDA_ARCH__ &gt;= 300) && (__CUDA_ARCH__ &lt; 500)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>int32_t rv, z=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>asm("vmin4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 1 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 2 / 准备局部状态 | <code>converter cx, cy, cr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>cx.storage = x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>cy.storage = y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>cr.a.x = min(cx.a.x, cy.a.x);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>cr.a.y = min(cx.a.y, cy.a.y);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>cr.a.z = min(cx.a.z, cy.a.z);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>cr.a.w = min(cx.a.w, cy.a.w);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 13 | 2 / 函数返回 | <code>return cr.storage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if (__CUDA_ARCH__ >= 300) && (__CUDA_ARCH__ < 500)
    int32_t rv, z=0;
    asm("vmin4.u32.u32.u32 %0, %1, %2, %3;" : "=r"(rv) : "r"(x), "r"(y), "r"(z));
    return rv;
#else
    converter cx, cy, cr;
    cx.storage = x;
    cy.storage = y;
    cr.a.x = min(cx.a.x, cy.a.x);
    cr.a.y = min(cx.a.y, cy.a.y);
    cr.a.z = min(cx.a.z, cy.a.z);
    cr.a.w = min(cx.a.w, cy.a.w);
    return cr.storage;
#endif
  }
```

</details>

<a id="f0301"></a>

## F0301：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (x&lt;y) ? x : y;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return (x<y) ? x : y;
  }
```

</details>

<a id="f0302"></a>

## F0302：FuncSum

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0303"></a>

## F0303：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 530 && __CUDA_ARCH__ != 610</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hadd2(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 准备局部状态 | <code>float2 fx, fy, fr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fx = __half22float2(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fy = __half22float2(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>fr.x = fx.x + fy.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>fr.y = fx.y + fy.y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 函数返回 | <code>return __float22half2_rn(fr);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
    return __hadd2(x, y);
#else
    float2 fx, fy, fr;
    fx = __half22float2(x);
    fy = __half22float2(y);
    fr.x = fx.x + fy.x;
    fr.y = fx.y + fy.y;
    return __float22half2_rn(fr);
#endif
  }
```

</details>

<a id="f0304"></a>

## F0304：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 530 && __CUDA_ARCH__ != 610</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hadd(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 函数返回 | <code>return __float2half( __half2float(x) + __half2float(y) );</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
    return __hadd(x, y);
#else
    return __float2half( __half2float(x) + __half2float(y) );
#endif
  }
```

</details>

<a id="f0305"></a>

## F0305：FuncSum

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0306"></a>

## F0306：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hadd2(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 准备局部状态 | <code>float fxl, fxh, fyl, fyh;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fxl = __low2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fxh = __high2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>fyl = __low2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>fyh = __high2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 函数返回 | <code>return __floats2bfloat162_rn(fxl + fyl, fxh + fyh);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 800
    return __hadd2(x, y);
#else
    float fxl, fxh, fyl, fyh;
    fxl = __low2float(x);
    fxh = __high2float(x);
    fyl = __low2float(y);
    fyh = __high2float(y);
    return __floats2bfloat162_rn(fxl + fyl, fxh + fyh);
#endif
   }
```

</details>

<a id="f0307"></a>

## F0307：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hadd(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 函数返回 | <code>return __float2bfloat16( __bfloat162float(x) + __bfloat162float(y) );</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 800
    return __hadd(x, y);
#else
    return __float2bfloat16( __bfloat162float(x) + __bfloat162float(y) );
#endif
  }
```

</details>

<a id="f0308"></a>

## F0308：FuncProd

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0309"></a>

## F0309：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 530 && __CUDA_ARCH__ != 610</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hmul2(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 准备局部状态 | <code>float2 fx, fy, fr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fx = __half22float2(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fy = __half22float2(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>fr.x = fx.x * fy.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>fr.y = fx.y * fy.y;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 函数返回 | <code>return __float22half2_rn(fr);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
    return __hmul2(x, y);
#else
    float2 fx, fy, fr;
    fx = __half22float2(x);
    fy = __half22float2(y);
    fr.x = fx.x * fy.x;
    fr.y = fx.y * fy.y;
    return __float22half2_rn(fr);
#endif
  }
```

</details>

<a id="f0310"></a>

## F0310：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 530 && __CUDA_ARCH__ != 610</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hmul(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 函数返回 | <code>return __float2half( __half2float(x) * __half2float(y) );</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 530 && __CUDA_ARCH__ != 610
    return __hmul(x, y);
#else
    return __float2half( __half2float(x) * __half2float(y) );
#endif
  }
```

</details>

<a id="f0311"></a>

## F0311：FuncProd

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0312"></a>

## F0312：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hmul2(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 准备局部状态 | <code>float fxl, fxh, fyl, fyh;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fxl = __low2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fxh = __high2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>fyl = __low2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>fyh = __high2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 函数返回 | <code>return __floats2bfloat162_rn(fxl * fyl, fxh * fyh);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 800
    return __hmul2(x, y);
#else
    float fxl, fxh, fyl, fyh;
    fxl = __low2float(x);
    fxh = __high2float(x);
    fyl = __low2float(y);
    fyh = __high2float(y);
    return __floats2bfloat162_rn(fxl * fyl, fxh * fyh);
#endif
  }
```

</details>

<a id="f0313"></a>

## F0313：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hmul(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 函数返回 | <code>return __float2bfloat16( __bfloat162float(x) * __bfloat162float(y) );</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 800
    return __hmul(x, y);
#else
    return __float2bfloat16( __bfloat162float(x) * __bfloat162float(y) );
#endif
  }
```

</details>

<a id="f0314"></a>

## F0314：FuncMax

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0315"></a>

## F0315：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>float2 fx, fy, fr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>fx = __half22float2(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>fy = __half22float2(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>fr.x = fmaxf(fx.x, fy.x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>fr.y = fmaxf(fx.y, fy.y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return __float22half2_rn(fr);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    float2 fx, fy, fr;
    fx = __half22float2(x);
    fy = __half22float2(y);
    fr.x = fmaxf(fx.x, fy.x);
    fr.y = fmaxf(fx.y, fy.y);
    return __float22half2_rn(fr);
  }
```

</details>

<a id="f0316"></a>

## F0316：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>float fx, fy, fm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>fx = __half2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>fy = __half2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>fm = fmaxf(fx, fy);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return __float2half(fm);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    float fx, fy, fm;
    fx = __half2float(x);
    fy = __half2float(y);
    fm = fmaxf(fx, fy);
    return __float2half(fm);
  }
```

</details>

<a id="f0317"></a>

## F0317：FuncMax

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0318"></a>

## F0318：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hmax2(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 准备局部状态 | <code>float fxl, fxh, fyl, fyh;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fxl = __low2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fxh = __high2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>fyl = __low2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>fyh = __high2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 函数返回 | <code>return __floats2bfloat162_rn(fmaxf(fxl, fyl), fmaxf(fxh, fyh));</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 800
    return __hmax2(x, y);
#else
    float fxl, fxh, fyl, fyh;
    fxl = __low2float(x);
    fxh = __high2float(x);
    fyl = __low2float(y);
    fyh = __high2float(y);
    return __floats2bfloat162_rn(fmaxf(fxl, fyl), fmaxf(fxh, fyh));
#endif
  }
```

</details>

<a id="f0319"></a>

## F0319：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hmax(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 准备局部状态 | <code>float fx, fy;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fx = __bfloat162float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fy = __bfloat162float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 函数返回 | <code>return __float2bfloat16(fmaxf(fx, fy));</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 800
    return __hmax(x, y);
#else
    float fx, fy;
    fx = __bfloat162float(x);
    fy = __bfloat162float(y);
    return __float2bfloat16(fmaxf(fx, fy));
#endif
  }
```

</details>

<a id="f0320"></a>

## F0320：FuncMin

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0321"></a>

## F0321：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>float2 fx, fy, fr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>fx = __half22float2(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>fy = __half22float2(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>fr.x = fminf(fx.x, fy.x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>fr.y = fminf(fx.y, fy.y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return __float22half2_rn(fr);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    float2 fx, fy, fr;
    fx = __half22float2(x);
    fy = __half22float2(y);
    fr.x = fminf(fx.x, fy.x);
    fr.y = fminf(fx.y, fy.y);
    return __float22half2_rn(fr);
  }
```

</details>

<a id="f0322"></a>

## F0322：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>float fx, fy, fm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>fx = __half2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>fy = __half2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>fm = fminf(fx, fy);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return __float2half(fm);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    float fx, fy, fm;
    fx = __half2float(x);
    fy = __half2float(y);
    fm = fminf(fx, fy);
    return __float2half(fm);
  }
```

</details>

<a id="f0323"></a>

## F0323：FuncMin

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0324"></a>

## F0324：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hmin2(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 准备局部状态 | <code>float fxl, fxh, fyl, fyh;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fxl = __low2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fxh = __high2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>fyl = __low2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>fyh = __high2float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 函数返回 | <code>return __floats2bfloat162_rn(fminf(fxl, fyl), fminf(fxh, fyh));</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 800
    return __hmin2(x, y);
#else
    float fxl, fxh, fyl, fyh;
    fxl = __low2float(x);
    fxh = __high2float(x);
    fyl = __low2float(y);
    fyh = __high2float(y);
    return __floats2bfloat162_rn(fminf(fxl, fyl), fminf(fxh, fyh));
#endif
  }
```

</details>

<a id="f0325"></a>

## F0325：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#if __CUDA_ARCH__ &gt;= 800</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 函数返回 | <code>return __hmin(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 3 | 1 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 4 | 2 / 准备局部状态 | <code>float fx, fy;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>fx = __bfloat162float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fy = __bfloat162float(y);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 函数返回 | <code>return __float2bfloat16(fminf(fx, fy));</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#if __CUDA_ARCH__ >= 800
    return __hmin(x, y);
#else
    float fx, fy;
    fx = __bfloat162float(x);
    fy = __bfloat162float(y);
    return __float2bfloat16(fminf(fx, fy));
#endif
  }
```

</details>

<a id="f0326"></a>

## F0326：FuncMax

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0327"></a>

## F0327：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return fmaxf(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return fmaxf(x, y);
  }
```

</details>

<a id="f0328"></a>

## F0328：FuncMin

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0329"></a>

## F0329：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return fminf(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return fminf(x, y);
  }
```

</details>

<a id="f0330"></a>

## F0330：FuncMax

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0331"></a>

## F0331：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return fmax(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return fmax(x, y);
  }
```

</details>

<a id="f0332"></a>

## F0332：FuncMin

用途：构造无额外运行状态的归约函数对象；运算语义由所属 Func 类型决定，普通操作忽略 opArg。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0333"></a>

## F0333：operator()

用途：执行当前类型/归约函数的运算；具体加、乘、最大或最小语义由所属模板/特化确定。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return fmin(x, y);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return fmin(x, y);
  }
```

</details>

<a id="f0334"></a>

## F0334：FuncSumPostDiv

用途：保存整数平均最后需要的除数，累加阶段继续使用求和，最终输出处才应用除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

<a id="f0335"></a>

## F0335：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return x; }
```

</details>

<a id="f0336"></a>

## F0336：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return T(x/n);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return T(x/n); }
```

</details>

<a id="f0337"></a>

## F0337：FuncPreMulSum

用途：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>scale = *(T*)&opArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ scale = *(T*)&opArg; }
```

</details>

<a id="f0338"></a>

## F0338：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x*scale;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return x*scale; }
```

</details>

<a id="f0339"></a>

## F0339：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return x; }
```

</details>

<a id="f0340"></a>

## F0340：FuncPreMulSum

用途：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>scale = *(double*)&opArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    scale = *(double*)&opArg;
  }
```

</details>

<a id="f0341"></a>

## F0341：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPreOpIdentity ? x : x*scale;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPreOpIdentity ? x : x*scale;
  }
```

</details>

<a id="f0342"></a>

## F0342：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPostOpIdentity ? x : x*scale;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPostOpIdentity ? x : x*scale;
  }
```

</details>

<a id="f0343"></a>

## F0343：FuncPreMulSum

用途：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>scale = *(float*)&opArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    scale = *(float*)&opArg;
  }
```

</details>

<a id="f0344"></a>

## F0344：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPreOpIdentity ? x : x*scale;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPreOpIdentity ? x : x*scale;
  }
```

</details>

<a id="f0345"></a>

## F0345：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPostOpIdentity ? x : x*scale;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPostOpIdentity ? x : x*scale;
  }
```

</details>

<a id="f0346"></a>

## F0346：FuncPreMulSum

用途：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>scale.x = *(half*)&opArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>scale.y = scale.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    scale.x = *(half*)&opArg;
    scale.y = scale.x;
  }
```

</details>

<a id="f0347"></a>

## F0347：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPreOpIdentity ? x : __hmul(x, scale.x);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPreOpIdentity ? x : __hmul(x, scale.x);
  }
```

</details>

<a id="f0348"></a>

## F0348：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPreOpIdentity ? x : __hmul2(x, scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPreOpIdentity ? x : __hmul2(x, scale);
  }
```

</details>

<a id="f0349"></a>

## F0349：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPostOpIdentity ? x : __hmul(x, scale.x);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPostOpIdentity ? x : __hmul(x, scale.x);
  }
```

</details>

<a id="f0350"></a>

## F0350：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPostOpIdentity ? x : __hmul2(x, scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPostOpIdentity ? x : __hmul2(x, scale);
  }
```

</details>

<a id="f0351"></a>

## F0351：FuncPreMulSum

用途：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>scale = __half2float(*(half*)&opArg);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    scale = __half2float(*(half*)&opArg);
  }
```

</details>

<a id="f0352"></a>

## F0352：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPreOpIdentity ? x : __float2half(__half2float(x)*scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPreOpIdentity ? x : __float2half(__half2float(x)*scale);
  }
```

</details>

<a id="f0353"></a>

## F0353：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(IsPreOpIdentity)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 准备局部状态 | <code>float2 a = __half22float2(x);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>a.x *= scale;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>a.y *= scale;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 函数返回 | <code>return __float22half2_rn(a);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (IsPreOpIdentity)
      return x;
    else {
      float2 a = __half22float2(x);
      a.x *= scale;
      a.y *= scale;
      return __float22half2_rn(a);
    }
  }
```

</details>

<a id="f0354"></a>

## F0354：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPostOpIdentity ? x : __float2half(__half2float(x)*scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPostOpIdentity ? x : __float2half(__half2float(x)*scale);
  }
```

</details>

<a id="f0355"></a>

## F0355：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(IsPostOpIdentity)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 准备局部状态 | <code>float2 a = __half22float2(x);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>a.x *= scale;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>a.y *= scale;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 函数返回 | <code>return __float22half2_rn(a);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (IsPostOpIdentity)
      return x;
    else {
      float2 a = __half22float2(x);
      a.x *= scale;
      a.y *= scale;
      return __float22half2_rn(a);
    }
  }
```

</details>

<a id="f0356"></a>

## F0356：FuncPreMulSum

用途：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>scale.x = *(__nv_bfloat16*)&opArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>scale.y = scale.x;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    scale.x = *(__nv_bfloat16*)&opArg;
    scale.y = scale.x;
  }
```

</details>

<a id="f0357"></a>

## F0357：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPreOpIdentity ? x : __hmul(x, scale.x);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPreOpIdentity ? x : __hmul(x, scale.x);
  }
```

</details>

<a id="f0358"></a>

## F0358：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPreOpIdentity ? x : __hmul2(x, scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPreOpIdentity ? x : __hmul2(x, scale);
  }
```

</details>

<a id="f0359"></a>

## F0359：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPostOpIdentity ? x : __hmul(x, scale.x);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPostOpIdentity ? x : __hmul(x, scale.x);
  }
```

</details>

<a id="f0360"></a>

## F0360：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPostOpIdentity ? x : __hmul2(x, scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPostOpIdentity ? x : __hmul2(x, scale);
  }
```

</details>

<a id="f0361"></a>

## F0361：FuncPreMulSum

用途：从任务中解码缩放标量，原始输入 preOp 乘该值，而中间部分和不重复缩放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>scale = *(__nv_bfloat16*)&opArg;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    scale = *(__nv_bfloat16*)&opArg;
  }
```

</details>

<a id="f0362"></a>

## F0362：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPreOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPreOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);
  }
```

</details>

<a id="f0363"></a>

## F0363：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(IsPreOpIdentity)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 准备局部状态 | <code>float fxl, fxh;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fxl = __low2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>fxh = __high2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 函数返回 | <code>return __floats2bfloat162_rn(fxl * scale, fxh * scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (IsPreOpIdentity)
      return x;
    else {
      float fxl, fxh;
      fxl = __low2float(x);
      fxh = __high2float(x);
      return __floats2bfloat162_rn(fxl * scale, fxh * scale);
    }
  }
```

</details>

<a id="f0364"></a>

## F0364：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return IsPostOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return IsPostOpIdentity ? x : __float2bfloat16(__bfloat162float(x)*scale);
  }
```

</details>

<a id="f0365"></a>

## F0365：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(IsPostOpIdentity)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return x;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 5 | 2 / 准备局部状态 | <code>float fxl, fxh;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>fxl = __low2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>fxh = __high2float(x);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 函数返回 | <code>return __floats2bfloat162_rn(fxl * scale, fxh * scale);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (IsPostOpIdentity)
      return x;
    else {
      float fxl, fxh;
      fxl = __low2float(x);
      fxh = __high2float(x);
      return __floats2bfloat162_rn(fxl * scale, fxh * scale);
    }
  }
```

</details>

<a id="f0366"></a>

## F0366：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return fn.preOp(x);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return fn.preOp(x);
  }
```

</details>

<a id="f0367"></a>

## F0367：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return fn.postOp(x);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return fn.postOp(x);
  }
```

</details>

<a id="f0368"></a>

## F0368：preOp

用途：对原始输入应用本归约类型的预处理；普通归约为恒等，PreMulSum 为乘标量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return fn.preOp(x);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return fn.preOp(x);
  }
```

</details>

<a id="f0369"></a>

## F0369：postOp

用途：对完整结果应用本归约类型的后处理；普通归约为恒等，整数 Avg 的 SumPostDiv 在这里做最终除法。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return fn.postOp(x);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    return fn.postOp(x);
  }
```

</details>

