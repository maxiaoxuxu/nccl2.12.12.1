# src/include/nvtx3/nvtxDetail/nvtxInitDefs.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0744"></a>

## F0744：nvtxMarkEx_impl_init

用途：向性能工具记录一个瞬时标记；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxMarkEx(eventAttrib);</code> | 向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxMarkEx(eventAttrib);
}
```

</details>

<a id="f0745"></a>

## F0745：nvtxMarkA_impl_init

用途：向性能工具记录一个瞬时标记；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxMarkA(message);</code> | 向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxMarkA(message);
}
```

</details>

<a id="f0746"></a>

## F0746：nvtxMarkW_impl_init

用途：向性能工具记录一个瞬时标记；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxMarkW(message);</code> | 向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxMarkW(message);
}
```

</details>

<a id="f0747"></a>

## F0747：nvtxRangeStartEx_impl_init

用途：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxRangeStartEx(eventAttrib);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangeStartEx(eventAttrib);
}
```

</details>

<a id="f0748"></a>

## F0748：nvtxRangeStartA_impl_init

用途：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxRangeStartA(message);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangeStartA(message);
}
```

</details>

<a id="f0749"></a>

## F0749：nvtxRangeStartW_impl_init

用途：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxRangeStartW(message);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangeStartW(message);
}
```

</details>

<a id="f0750"></a>

## F0750：nvtxRangeEnd_impl_init

用途：按id结束显式NVTX时间范围；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxRangeEnd(id);</code> | 按id结束显式NVTX时间范围。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxRangeEnd(id);
}
```

</details>

<a id="f0751"></a>

## F0751：nvtxRangePushEx_impl_init

用途：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxRangePushEx(eventAttrib);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangePushEx(eventAttrib);
}
```

</details>

<a id="f0752"></a>

## F0752：nvtxRangePushA_impl_init

用途：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxRangePushA(message);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangePushA(message);
}
```

</details>

<a id="f0753"></a>

## F0753：nvtxRangePushW_impl_init

用途：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxRangePushW(message);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangePushW(message);
}
```

</details>

<a id="f0754"></a>

## F0754：nvtxRangePop_impl_init

用途：从当前线程NVTX范围栈pop结束最近区间；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxRangePop();</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxRangePop();
}
```

</details>

<a id="f0755"></a>

## F0755：nvtxNameCategoryA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxNameCategoryA(category, name);</code> | 为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxNameCategoryA(category, name);
}
```

</details>

<a id="f0756"></a>

## F0756：nvtxNameCategoryW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxNameCategoryW(category, name);</code> | 为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxNameCategoryW(category, name);
}
```

</details>

<a id="f0757"></a>

## F0757：nvtxNameOsThreadA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxNameOsThreadA(threadId, name);</code> | 为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxNameOsThreadA(threadId, name);
}
```

</details>

<a id="f0758"></a>

## F0758：nvtxNameOsThreadW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxNameOsThreadW(threadId, name);</code> | 为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxNameOsThreadW(threadId, name);
}
```

</details>

<a id="f0759"></a>

## F0759：nvtxDomainMarkEx_impl_init

用途：向性能工具记录一个瞬时标记；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxDomainMarkEx(domain, eventAttrib);</code> | 向性能工具记录一个瞬时标记。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainMarkEx(domain, eventAttrib);
}
```

</details>

<a id="f0760"></a>

## F0760：nvtxDomainRangeStartEx_impl_init

用途：建立显式NVTX时间范围并返回id，之后可用RangeEnd配对结束；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxDomainRangeStartEx(domain, eventAttrib);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRangeStartEx(domain, eventAttrib);
}
```

</details>

<a id="f0761"></a>

## F0761：nvtxDomainRangeEnd_impl_init

用途：按id结束显式NVTX时间范围；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxDomainRangeEnd(domain, id);</code> | 按id结束显式NVTX时间范围。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainRangeEnd(domain, id);
}
```

</details>

<a id="f0762"></a>

## F0762：nvtxDomainRangePushEx_impl_init

用途：在当前线程的NVTX范围栈push一项，嵌套区间需与pop配对；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxDomainRangePushEx(domain, eventAttrib);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRangePushEx(domain, eventAttrib);
}
```

</details>

<a id="f0763"></a>

## F0763：nvtxDomainRangePop_impl_init

用途：从当前线程NVTX范围栈pop结束最近区间；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxDomainRangePop(domain);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRangePop(domain);
}
```

</details>

<a id="f0764"></a>

## F0764：nvtxDomainResourceCreate_impl_init

用途：在NVTX域中登记一个可被工具识别的资源属性对象；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxDomainResourceCreate(domain, attribs);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainResourceCreate(domain, attribs);
}
```

</details>

<a id="f0765"></a>

## F0765：nvtxDomainResourceDestroy_impl_init

用途：结束域内资源标记的生命周期；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxDomainResourceDestroy(resource);</code> | 结束域内资源标记的生命周期。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainResourceDestroy(resource);
}
```

</details>

<a id="f0766"></a>

## F0766：nvtxDomainNameCategoryA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxDomainNameCategoryA(domain, category, name);</code> | 为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainNameCategoryA(domain, category, name);
}
```

</details>

<a id="f0767"></a>

## F0767：nvtxDomainNameCategoryW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxDomainNameCategoryW(domain, category, name);</code> | 为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainNameCategoryW(domain, category, name);
}
```

</details>

<a id="f0768"></a>

## F0768：nvtxDomainRegisterStringA_impl_init

用途：注册域内可复用字符串并返回句柄，降低重复消息处理开销；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxDomainRegisterStringA(domain, string);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRegisterStringA(domain, string);
}
```

</details>

<a id="f0769"></a>

## F0769：nvtxDomainRegisterStringW_impl_init

用途：注册域内可复用字符串并返回句柄，降低重复消息处理开销；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxDomainRegisterStringW(domain, string);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainRegisterStringW(domain, string);
}
```

</details>

<a id="f0770"></a>

## F0770：nvtxDomainCreateA_impl_init

用途：创建带名字的NVTX标记域，隔离不同组件的事件/类别；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxDomainCreateA(message);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainCreateA(message);
}
```

</details>

<a id="f0771"></a>

## F0771：nvtxDomainCreateW_impl_init

用途：创建带名字的NVTX标记域，隔离不同组件的事件/类别；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return nvtxDomainCreateW(message);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    return nvtxDomainCreateW(message);
}
```

</details>

<a id="f0772"></a>

## F0772：nvtxDomainDestroy_impl_init

用途：释放NVTX域句柄及工具相关引用；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxDomainDestroy(domain);</code> | 释放NVTX域句柄及工具相关引用。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxDomainDestroy(domain);
}
```

</details>

<a id="f0773"></a>

## F0773：nvtxInitialize_impl_init

用途：触发或协商NVTX性能工具接口的一次性初始化；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>nvtxInitialize(reserved);</code> | 显式触发NVTX工具初始化，通常API首次调用也能惰性触发；不创建NCCL通信资源。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    nvtxInitialize(reserved);
}
```

</details>

<a id="f0774"></a>

## F0774：nvtxNameCuDeviceA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCuDeviceA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(device, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCuDeviceA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr;
    if (local)
        local(device, name);
}
```

</details>

<a id="f0775"></a>

## F0775：nvtxNameCuDeviceW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCuDeviceW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(device, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCuDeviceW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr;
    if (local)
        local(device, name);
}
```

</details>

<a id="f0776"></a>

## F0776：nvtxNameCuContextA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCuContextA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(context, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCuContextA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr;
    if (local)
        local(context, name);
}
```

</details>

<a id="f0777"></a>

## F0777：nvtxNameCuContextW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCuContextW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(context, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCuContextW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr;
    if (local)
        local(context, name);
}
```

</details>

<a id="f0778"></a>

## F0778：nvtxNameCuStreamA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCuStreamA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(stream, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCuStreamA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr;
    if (local)
        local(stream, name);
}
```

</details>

<a id="f0779"></a>

## F0779：nvtxNameCuStreamW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCuStreamW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(stream, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCuStreamW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr;
    if (local)
        local(stream, name);
}
```

</details>

<a id="f0780"></a>

## F0780：nvtxNameCuEventA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCuEventA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(event, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCuEventA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr;
    if (local)
        local(event, name);
}
```

</details>

<a id="f0781"></a>

## F0781：nvtxNameCuEventW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCuEventW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(event, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCuEventW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr;
    if (local)
        local(event, name);
}
```

</details>

<a id="f0782"></a>

## F0782：nvtxNameCudaDeviceA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCudaDeviceA_impl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(device, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCudaDeviceA_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr;
    if (local)
        local(device, name);
}
```

</details>

<a id="f0783"></a>

## F0783：nvtxNameCudaDeviceW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCudaDeviceW_impl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(device, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCudaDeviceW_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr;
    if (local)
        local(device, name);
}
```

</details>

<a id="f0784"></a>

## F0784：nvtxNameCudaStreamA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCudaStreamA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(stream, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCudaStreamA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr;
    if (local)
        local(stream, name);
}
```

</details>

<a id="f0785"></a>

## F0785：nvtxNameCudaStreamW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCudaStreamW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(stream, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCudaStreamW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr;
    if (local)
        local(stream, name);
}
```

</details>

<a id="f0786"></a>

## F0786：nvtxNameCudaEventA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCudaEventA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(event, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCudaEventA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr;
    if (local)
        local(event, name);
}
```

</details>

<a id="f0787"></a>

## F0787：nvtxNameCudaEventW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameCudaEventW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(event, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameCudaEventW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr;
    if (local)
        local(event, name);
}
```

</details>

<a id="f0788"></a>

## F0788：nvtxNameClDeviceA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClDeviceA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(device, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClDeviceA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr;
    if (local)
        local(device, name);
}
```

</details>

<a id="f0789"></a>

## F0789：nvtxNameClDeviceW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClDeviceW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(device, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClDeviceW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr;
    if (local)
        local(device, name);
}
```

</details>

<a id="f0790"></a>

## F0790：nvtxNameClContextA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClContextA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(context, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClContextA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr;
    if (local)
        local(context, name);
}
```

</details>

<a id="f0791"></a>

## F0791：nvtxNameClContextW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClContextW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(context, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClContextW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr;
    if (local)
        local(context, name);
}
```

</details>

<a id="f0792"></a>

## F0792：nvtxNameClCommandQueueA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClCommandQueueA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(command_queue, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClCommandQueueA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr;
    if (local)
        local(command_queue, name);
}
```

</details>

<a id="f0793"></a>

## F0793：nvtxNameClCommandQueueW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClCommandQueueW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(command_queue, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClCommandQueueW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr;
    if (local)
        local(command_queue, name);
}
```

</details>

<a id="f0794"></a>

## F0794：nvtxNameClMemObjectA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClMemObjectA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(memobj, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClMemObjectA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr;
    if (local)
        local(memobj, name);
}
```

</details>

<a id="f0795"></a>

## F0795：nvtxNameClMemObjectW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClMemObjectW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(memobj, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClMemObjectW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr;
    if (local)
        local(memobj, name);
}
```

</details>

<a id="f0796"></a>

## F0796：nvtxNameClSamplerA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClSamplerA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(sampler, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClSamplerA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr;
    if (local)
        local(sampler, name);
}
```

</details>

<a id="f0797"></a>

## F0797：nvtxNameClSamplerW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClSamplerW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(sampler, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClSamplerW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr;
    if (local)
        local(sampler, name);
}
```

</details>

<a id="f0798"></a>

## F0798：nvtxNameClProgramA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClProgramA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(program, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClProgramA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr;
    if (local)
        local(program, name);
}
```

</details>

<a id="f0799"></a>

## F0799：nvtxNameClProgramW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClProgramW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(program, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClProgramW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr;
    if (local)
        local(program, name);
}
```

</details>

<a id="f0800"></a>

## F0800：nvtxNameClEventA_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClEventA_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(evnt, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClEventA_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr;
    if (local)
        local(evnt, name);
}
```

</details>

<a id="f0801"></a>

## F0801：nvtxNameClEventW_impl_init

用途：为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxNameClEventW_fakeimpl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(evnt, name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxNameClEventW_fakeimpl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr;
    if (local)
        local(evnt, name);
}
```

</details>

<a id="f0802"></a>

## F0802：nvtxDomainSyncUserCreate_impl_init

用途：创建带属性的用户同步标记对象供性能工具追踪；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxDomainSyncUserCreate_impl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return local(domain, attribs);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 函数返回 | <code>return (nvtxSyncUser_t)0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxDomainSyncUserCreate_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr;
    if (local) {
        return local(domain, attribs);
    }
    return (nvtxSyncUser_t)0;
}
```

</details>

<a id="f0803"></a>

## F0803：nvtxDomainSyncUserDestroy_impl_init

用途：结束用户同步标记对象的工具生命周期；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxDomainSyncUserDestroy_impl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(handle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxDomainSyncUserDestroy_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr;
    if (local)
        local(handle);
}
```

</details>

<a id="f0804"></a>

## F0804：nvtxDomainSyncUserAcquireStart_impl_init

用途：报告用户同步对象的一次获取尝试开始；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxDomainSyncUserAcquireStart_impl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(handle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxDomainSyncUserAcquireStart_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr;
    if (local)
        local(handle);
}
```

</details>

<a id="f0805"></a>

## F0805：nvtxDomainSyncUserAcquireFailed_impl_init

用途：报告用户同步对象获取失败；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxDomainSyncUserAcquireFailed_impl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(handle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxDomainSyncUserAcquireFailed_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr;
    if (local)
        local(handle);
}
```

</details>

<a id="f0806"></a>

## F0806：nvtxDomainSyncUserAcquireSuccess_impl_init

用途：报告用户同步对象获取成功；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxDomainSyncUserAcquireSuccess_impl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(handle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxDomainSyncUserAcquireSuccess_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr;
    if (local)
        local(handle);
}
```

</details>

<a id="f0807"></a>

## F0807：nvtxDomainSyncUserReleasing_impl_init

用途：报告用户同步对象即将释放；此函数是首次调用初始化跳板，先InitOnce再转入替换后的回调。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>nvtxDomainSyncUserReleasing_impl_fntype local;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(local)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>local(handle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    nvtxDomainSyncUserReleasing_impl_fntype local;
    NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)();
    local = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr;
    if (local)
        local(handle);
}
```

</details>

<a id="f0808"></a>

## F0808：nvtxSetInitFunctionsToNoops

用途：把尚未被工具实现的初始化跳板替换为空回调，避免每次标记重复初始化或调用空指针。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkEx_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartEx_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeEnd_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushEx_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePop_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 52 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 56 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 57 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 62 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 63 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 64 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 65 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 66 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 71 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 72 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 74 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 75 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 76 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 77 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 78 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 80 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 81 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 83 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 84 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 85 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 86 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 87 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 88 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 89 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 90 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 91 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 92 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 93 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 94 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 95 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 96 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 97 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 98 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 99 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 100 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 101 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 102 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 103 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 104 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 105 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 106 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 107 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 108 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 109 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 110 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 111 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 112 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 113 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 114 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 115 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 116 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 117 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 118 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 119 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 120 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 121 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 122 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 123 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 124 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 125 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 126 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 127 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 128 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 129 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 130 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainMarkEx_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 131 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 132 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 133 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeStartEx_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 134 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 135 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 136 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeEnd_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 137 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 138 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 139 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePushEx_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 140 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 141 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 142 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePop_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 143 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 144 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 145 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceCreate_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 146 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 147 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 148 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceDestroy_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 149 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 150 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 151 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 152 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 153 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 154 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 155 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 156 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 157 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 158 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 159 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 160 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 161 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 162 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 163 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateA_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 164 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 165 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 166 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateW_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 167 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 168 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 169 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainDestroy_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 170 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 171 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 172 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxInitialize_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 173 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 174 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 175 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserCreate_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 176 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 177 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 178 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserDestroy_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 179 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 180 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 181 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireStart_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 182 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 183 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 184 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireFailed_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 185 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 186 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 187 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireSuccess_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 188 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 189 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 190 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserReleasing_impl_init) &#124;&#124; forceAllToNoops)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 191 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 192 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxMarkW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxMarkW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeStartW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeStartW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangeEnd_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangeEnd_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePushW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePushW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxRangePop_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxRangePop_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCategoryW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCategoryW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameOsThreadW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameOsThreadW_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuDeviceW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuDeviceW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuContextW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuContextW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuStreamW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuStreamW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCuEventW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCuEventW_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClDeviceW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClDeviceW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClContextW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClContextW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClCommandQueueW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClCommandQueueW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClMemObjectW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClMemObjectW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClSamplerW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClSamplerW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClProgramW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClProgramW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameClEventW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameClEventW_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaDeviceW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaDeviceW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaStreamW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaStreamW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxNameCudaEventW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxNameCudaEventW_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainMarkEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainMarkEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeStartEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeStartEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangeEnd_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangeEnd_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePushEx_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePushEx_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRangePop_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRangePop_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceCreate_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceCreate_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainResourceDestroy_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainResourceDestroy_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainNameCategoryW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainNameCategoryW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainRegisterStringW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainRegisterStringW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateA_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateA_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainCreateW_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainCreateW_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainDestroy_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainDestroy_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxInitialize_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxInitialize_impl_fnptr = NULL;

    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserCreate_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserCreate_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserDestroy_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserDestroy_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireStart_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireStart_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireFailed_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireFailed_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserAcquireSuccess_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserAcquireSuccess_impl_fnptr = NULL;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr == NVTX_VERSIONED_IDENTIFIER(nvtxDomainSyncUserReleasing_impl_init) || forceAllToNoops)
        NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).nvtxDomainSyncUserReleasing_impl_fnptr = NULL;
}
```

</details>

