# src/misc/gdrwrap.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0912"></a>

## F0912：wrap_gdr_symbols

用途：解析 libgdrapi 动态符号；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdrState == gdrInitialized)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 条件判断 | <code>(gdrState == gdrError)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 条件判断 | <code>(__sync_bool_compare_and_swap(&gdrState, gdrUninitialized, gdrInitializing) == false)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 循环 | <code>初始 ；继续条件 (gdrState == gdrInitializing)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>pthread_yield();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 2 / 函数返回 | <code>return (gdrState == gdrInitialized) ? ncclSuccess : ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 准备局部状态 | <code>static void* gdrhandle = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>void* tmp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 准备局部状态 | <code>void** cast;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>gdrhandle=dlopen(GDRAPI_LIBNAME, RTLD_NOW);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 条件判断 | <code>(!gdrhandle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>WARN("Failed to open %s", GDRAPI_LIBNAME);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 2 / 控制转移 | <code>goto teardown;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_open", gdr_internal_open);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_close", gdr_internal_close);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_pin_buffer", gdr_internal_pin_buffer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_unpin_buffer", gdr_internal_unpin_buffer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_get_info", gdr_internal_get_info);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_map", gdr_internal_map);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_unmap", gdr_internal_unmap);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_runtime_get_version", gdr_internal_runtime_get_version);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_driver_get_version", gdr_internal_driver_get_version);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_copy_to_mapping", gdr_internal_copy_to_mapping);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(gdrhandle, "gdr_copy_from_mapping", gdr_internal_copy_from_mapping);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>gdrState = gdrInitialized;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 34 | 0 / 跳转标签 | <code>teardown</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 35 | 1 / 调用 / 计算 / 写入 | <code>gdr_internal_open = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_close = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_pin_buffer = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_unpin_buffer = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_get_info = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_map = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_unmap = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_runtime_get_version = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_driver_get_version = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_copy_to_mapping = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_copy_from_mapping = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 0 / 条件判断 | <code>(gdrhandle != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 2 / 调用 / 计算 / 写入 | <code>dlclose(gdrhandle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 0 / 调用 / 计算 / 写入 | <code>gdrState = gdrError;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 0 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdrState == gdrInitialized)
    return ncclSuccess;
  if (gdrState == gdrError)
    return ncclSystemError;

  if (__sync_bool_compare_and_swap(&gdrState, gdrUninitialized, gdrInitializing) == false) {
    // Another thread raced in front of us. Wait for it to be done.
    while (gdrState == gdrInitializing) pthread_yield();
    return (gdrState == gdrInitialized) ? ncclSuccess : ncclSystemError;
  }

  static void* gdrhandle = NULL;
  void* tmp;
  void** cast;

  gdrhandle=dlopen(GDRAPI_LIBNAME, RTLD_NOW);
  if (!gdrhandle) {
    WARN("Failed to open %s", GDRAPI_LIBNAME);
    goto teardown;
  }

  /* Load the function pointers from the DL library image */
  LOAD_SYM(gdrhandle, "gdr_open", gdr_internal_open);
  LOAD_SYM(gdrhandle, "gdr_close", gdr_internal_close);
  LOAD_SYM(gdrhandle, "gdr_pin_buffer", gdr_internal_pin_buffer);
  LOAD_SYM(gdrhandle, "gdr_unpin_buffer", gdr_internal_unpin_buffer);
  LOAD_SYM(gdrhandle, "gdr_get_info", gdr_internal_get_info);
  LOAD_SYM(gdrhandle, "gdr_map", gdr_internal_map);
  LOAD_SYM(gdrhandle, "gdr_unmap", gdr_internal_unmap);
  LOAD_SYM(gdrhandle, "gdr_runtime_get_version", gdr_internal_runtime_get_version);
  LOAD_SYM(gdrhandle, "gdr_driver_get_version", gdr_internal_driver_get_version);
  LOAD_SYM(gdrhandle, "gdr_copy_to_mapping", gdr_internal_copy_to_mapping);
  LOAD_SYM(gdrhandle, "gdr_copy_from_mapping", gdr_internal_copy_from_mapping);

  gdrState = gdrInitialized;
  return ncclSuccess;

teardown:
  gdr_internal_open = NULL;
  gdr_internal_close = NULL;
  gdr_internal_pin_buffer = NULL;
  gdr_internal_unpin_buffer = NULL;
  gdr_internal_get_info = NULL;
  gdr_internal_map = NULL;
  gdr_internal_unmap = NULL;
  gdr_internal_runtime_get_version = NULL;
  gdr_internal_driver_get_version = NULL;
  gdr_internal_copy_to_mapping = NULL;
  gdr_internal_copy_from_mapping = NULL;

  if (gdrhandle != NULL) dlclose(gdrhandle);
  gdrState = gdrError;
  return ncclSystemError;
}
```

</details>

<a id="f0913"></a>

## F0913：wrap_gdr_open

用途：打开 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_open == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 函数返回 | <code>return gdr_internal_open();</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_open == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return NULL;
  }
  return gdr_internal_open();
}
```

</details>

<a id="f0914"></a>

## F0914：wrap_gdr_close

用途：关闭 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_close == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ret = gdr_internal_close(g);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(ret != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>WARN("gdr_close() failed: %d", ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_close == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  int ret = gdr_internal_close(g);
  if (ret != 0) {
    WARN("gdr_close() failed: %d", ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0915"></a>

## F0915：wrap_gdr_pin_buffer

用途：固定 GPU 显存以供映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_pin_buffer == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>GDRLOCKCALL(gdr_internal_pin_buffer(g, addr, size, p2p_token, va_space, handle), ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(ret != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>WARN("gdr_pin_buffer(addr %lx, size %zi) failed: %d", addr, size, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_pin_buffer == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  int ret;
  GDRLOCKCALL(gdr_internal_pin_buffer(g, addr, size, p2p_token, va_space, handle), ret);
  if (ret != 0) {
    WARN("gdr_pin_buffer(addr %lx, size %zi) failed: %d", addr, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0916"></a>

## F0916：wrap_gdr_unpin_buffer

用途：解除 GPU 显存固定；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_unpin_buffer == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>GDRLOCKCALL(gdr_internal_unpin_buffer(g, handle), ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(ret != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>WARN("gdr_unpin_buffer(handle %lx) failed: %d", handle.h, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_unpin_buffer == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  int ret;
  GDRLOCKCALL(gdr_internal_unpin_buffer(g, handle), ret);
  if (ret != 0) {
    WARN("gdr_unpin_buffer(handle %lx) failed: %d", handle.h, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0917"></a>

## F0917：wrap_gdr_get_info

用途：读取 GPU 显存映射属性及页偏移；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_get_info == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>GDRLOCKCALL(gdr_internal_get_info(g, handle, info), ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(ret != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>WARN("gdr_get_info(handle %lx) failed: %d", handle.h, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_get_info == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  int ret;
  GDRLOCKCALL(gdr_internal_get_info(g, handle, info), ret);
  if (ret != 0) {
    WARN("gdr_get_info(handle %lx) failed: %d", handle.h, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0918"></a>

## F0918：wrap_gdr_map

用途：把已固定 GPU 内存映射到 CPU 地址空间；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_map == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>GDRLOCKCALL(gdr_internal_map(g, handle, va, size), ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(ret != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>WARN("gdr_map(handle %lx, size %zi) failed: %d", handle.h, size, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_map == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  int ret;
  GDRLOCKCALL(gdr_internal_map(g, handle, va, size), ret);
  if (ret != 0) {
    WARN("gdr_map(handle %lx, size %zi) failed: %d", handle.h, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0919"></a>

## F0919：wrap_gdr_unmap

用途：解除 CPU 映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_unmap == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>GDRLOCKCALL(gdr_internal_unmap(g, handle, va, size), ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(ret != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>WARN("gdr_unmap(handle %lx, va %p, size %zi) failed: %d", handle.h, va, size, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_unmap == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  int ret;
  GDRLOCKCALL(gdr_internal_unmap(g, handle, va, size), ret);
  if (ret != 0) {
    WARN("gdr_unmap(handle %lx, va %p, size %zi) failed: %d", handle.h, va, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0920"></a>

## F0920：wrap_gdr_runtime_get_version

用途：查询 GDRCopy 用户库版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_runtime_get_version == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_runtime_get_version(major, minor);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_runtime_get_version == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  gdr_internal_runtime_get_version(major, minor);
  return ncclSuccess;
}
```

</details>

<a id="f0921"></a>

## F0921：wrap_gdr_driver_get_version

用途：查询 GDRCopy 驱动版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_driver_get_version == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>gdr_internal_driver_get_version(g, major, minor);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_driver_get_version == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  gdr_internal_driver_get_version(g, major, minor);
  return ncclSuccess;
}
```

</details>

<a id="f0922"></a>

## F0922：wrap_gdr_copy_to_mapping

用途：向 CPU 可见的 GPU 映射复制数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_copy_to_mapping == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>GDRLOCKCALL(gdr_internal_copy_to_mapping(handle, map_d_ptr, h_ptr, size), ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(ret != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>WARN("gdr_copy_to_mapping(handle %lx, map_d_ptr %p, h_ptr %p, size %zi) failed: %d", handle.h, map_d_ptr, h_ptr, size, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_copy_to_mapping == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  int ret;
  GDRLOCKCALL(gdr_internal_copy_to_mapping(handle, map_d_ptr, h_ptr, size), ret);
  if (ret != 0) {
    WARN("gdr_copy_to_mapping(handle %lx, map_d_ptr %p, h_ptr %p, size %zi) failed: %d", handle.h, map_d_ptr, h_ptr, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0923"></a>

## F0923：wrap_gdr_copy_from_mapping

用途：从 CPU 可见的 GPU 映射读回数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(gdr_internal_copy_from_mapping == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("GDRCOPY lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>int ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>GDRLOCKCALL(gdr_internal_copy_from_mapping(handle, h_ptr, map_d_ptr, size), ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(ret != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>WARN("gdr_copy_from_mapping(handle %lx, h_ptr %p, map_d_ptr %p, size %zi) failed: %d", handle.h, h_ptr, map_d_ptr, size, ret);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (gdr_internal_copy_from_mapping == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  int ret;
  GDRLOCKCALL(gdr_internal_copy_from_mapping(handle, h_ptr, map_d_ptr, size), ret);
  if (ret != 0) {
    WARN("gdr_copy_from_mapping(handle %lx, h_ptr %p, map_d_ptr %p, size %zi) failed: %d", handle.h, h_ptr, map_d_ptr, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

