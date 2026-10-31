# src/include/core.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0628"></a>

## F0628：ncclTypeSize

用途：把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 多路分派 | <code>(type)</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 2 | 1 / case / default | <code>ncclInt8</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 3 | 1 / case / default | <code>ncclUint8</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 4 | 2 / 函数返回 | <code>return 1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 1 / case / default | <code>ncclFloat16</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 6 | 1 / 编译期分支/指令 | <code>#if defined(__CUDA_BF16_TYPES_EXIST__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 7 | 1 / 函数返回 | <code>return 2;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 1 / case / default | <code>ncclInt32</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 9 | 1 / case / default | <code>ncclUint32</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 10 | 1 / case / default | <code>ncclFloat32</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 11 | 2 / 函数返回 | <code>return 4;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 1 / case / default | <code>ncclInt64</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 13 | 1 / case / default | <code>ncclUint64</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 14 | 1 / case / default | <code>ncclFloat64</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 15 | 2 / 函数返回 | <code>return 8;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 16 | 1 / case / default | <code>default</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 17 | 2 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  switch (type) {
    case ncclInt8:
    case ncclUint8:
      return 1;
    case ncclFloat16:
#if defined(__CUDA_BF16_TYPES_EXIST__)
    case ncclBfloat16:
#endif
      return 2;
    case ncclInt32:
    case ncclUint32:
    case ncclFloat32:
      return 4;
    case ncclInt64:
    case ncclUint64:
    case ncclFloat64:
      return 8;
    default:
      return -1;
  }
}
```

</details>

