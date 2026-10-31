# src/include/nvtx3/nvtxDetail/nvtxImpl.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0675"></a>

## F0675：nvtxEtiGetModuleFunctionTable

用途：按模块与接口版本导出NVTX回调函数表，让注入工具为对应API登记实现。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>unsigned int bytes = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>NvtxFunctionTable table = (NvtxFunctionTable)0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 多路分派 | <code>(module)</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 4 | 1 / case / default | <code>NVTX_CB_MODULE_CORE</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 8 | 1 / case / default | <code>NVTX_CB_MODULE_CUDA</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDA;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDA);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 12 | 1 / case / default | <code>NVTX_CB_MODULE_OPENCL</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_OPENCL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_OPENCL);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 16 | 1 / case / default | <code>NVTX_CB_MODULE_CUDART</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDART;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDART);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 20 | 1 / case / default | <code>NVTX_CB_MODULE_CORE2</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE2);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 24 | 1 / case / default | <code>NVTX_CB_MODULE_SYNC</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_SYNC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_SYNC);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 2 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 28 | 1 / case / default | <code>default</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 29 | 2 / 函数返回 | <code>return 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 30 | 0 / 条件判断 | <code>(out_size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>*out_size = (bytes / (unsigned int)sizeof(NvtxFunctionPointer*)) - 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 条件判断 | <code>(out_table)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>*out_table = table;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 0 / 函数返回 | <code>return 1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    unsigned int bytes = 0;
    NvtxFunctionTable table = (NvtxFunctionTable)0;

    switch (module)
    {
    case NVTX_CB_MODULE_CORE:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE);
        break;
    case NVTX_CB_MODULE_CUDA:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDA;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDA);
        break;
    case NVTX_CB_MODULE_OPENCL:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_OPENCL;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_OPENCL);
        break;
    case NVTX_CB_MODULE_CUDART:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDART;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CUDART);
        break;
    case NVTX_CB_MODULE_CORE2:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE2;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_CORE2);
        break;
    case NVTX_CB_MODULE_SYNC:
        table = NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_SYNC;
        bytes = (unsigned int)sizeof(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).functionTable_SYNC);
        break;
    default: return 0;
    }

    if (out_size)
        *out_size = (bytes / (unsigned int)sizeof(NvtxFunctionPointer*)) - 1;

    if (out_table)
        *out_table = table;

    return 1;
}
```

</details>

<a id="f0676"></a>

## F0676：nvtxGetExportTable

用途：根据导出表id提供版本/回调导出入口，未知id不返回任意布局对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 多路分派 | <code>(exportTableId)</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 2 | 1 / case / default | <code>NVTX_ETID_CALLBACKS</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 3 | 2 / 函数返回 | <code>return &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).etblCallbacks;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 1 / case / default | <code>NVTX_ETID_VERSIONINFO</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 5 | 2 / 函数返回 | <code>return &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).etblVersionInfo;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 1 / case / default | <code>default</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 7 | 2 / 函数返回 | <code>return 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    switch (exportTableId)
    {
    case NVTX_ETID_CALLBACKS:       return &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).etblCallbacks;
    case NVTX_ETID_VERSIONINFO:     return &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).etblVersionInfo;
    default:                        return 0;
    }
}
```

</details>

<a id="f0677"></a>

## F0677：nvtxEtiSetInjectionNvtxVersion

用途：记录注入工具宣告的NVTX版本信息，配合回调表协商。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>(void)version;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    /* Reserved for custom implementations to resolve problems with tools */
    (void)version;
}
```

</details>

