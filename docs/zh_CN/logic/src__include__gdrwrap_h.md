# src/include/gdrwrap.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0635"></a>

## F0635：wc_store_fence

用途：按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>asm volatile("sync") ;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ asm volatile("sync") ; }
```

</details>

<a id="f0636"></a>

## F0636：wc_store_fence

用途：按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>_mm_sfence();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ _mm_sfence(); }
```

</details>

<a id="f0637"></a>

## F0637：wc_store_fence

用途：按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>std::atomic_thread_fence(std::memory_order_release);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ std::atomic_thread_fence(std::memory_order_release); }
```

</details>

<a id="f0638"></a>

## F0638：wc_store_fence

用途：按 CPU 架构执行写合并内存的 store fence，确保映射显存控制写的发布顺序；无该支持时走相应实现。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>atomic_thread_fence(memory_order_release);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ atomic_thread_fence(memory_order_release); }
```

</details>

<a id="f0639"></a>

## F0639：wrap_gdr_symbols

用途：解析 libgdrapi 动态符号；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return ncclSuccess; }
```

</details>

<a id="f0640"></a>

## F0640：wrap_gdr_open

用途：打开 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>gdr_t g = gdr_open();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 函数返回 | <code>return g;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ gdr_t g = gdr_open(); return g; }
```

</details>

<a id="f0641"></a>

## F0641：wrap_gdr_close

用途：关闭 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>GDRCHECK(gdr_close(g));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ GDRCHECK(gdr_close(g)); return ncclSuccess; }
```

</details>

<a id="f0642"></a>

## F0642：wrap_gdr_pin_buffer

用途：固定 GPU 显存以供映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>GDRCHECK(gdr_pin_buffer(g, addr, size, p2p_token, va_space, handle));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  GDRCHECK(gdr_pin_buffer(g, addr, size, p2p_token, va_space, handle));
  return ncclSuccess;
}
```

</details>

<a id="f0643"></a>

## F0643：wrap_gdr_unpin_buffer

用途：解除 GPU 显存固定；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>GDRCHECK(gdr_unpin_buffer(g, handle));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  GDRCHECK(gdr_unpin_buffer(g, handle));
  return ncclSuccess;
}
```

</details>

<a id="f0644"></a>

## F0644：wrap_gdr_get_info

用途：读取 GPU 显存映射属性及页偏移；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>GDRCHECK(gdr_get_info(g, handle, info));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  GDRCHECK(gdr_get_info(g, handle, info));
  return ncclSuccess;
}
```

</details>

<a id="f0645"></a>

## F0645：wrap_gdr_map

用途：把已固定 GPU 内存映射到 CPU 地址空间；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>GDRCHECK(gdr_map(gdr_t g, gdr_mh_t handle, void **va, size_t size));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  GDRCHECK(gdr_map(gdr_t g, gdr_mh_t handle, void **va, size_t size));
  return ncclSuccess;
}
```

</details>

<a id="f0646"></a>

## F0646：wrap_gdr_unmap

用途：解除 CPU 映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>GDRCHECK(gdr_unmap(gdr_t g, gdr_mh_t handle, void **va, size_t size));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  GDRCHECK(gdr_unmap(gdr_t g, gdr_mh_t handle, void **va, size_t size));
  return ncclSuccess;
}
```

</details>

<a id="f0647"></a>

## F0647：wrap_gdr_runtime_get_version

用途：查询 GDRCopy 用户库版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>gdr_runtime_get_version(major, minor);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  gdr_runtime_get_version(major, minor);
  return ncclSuccess;
}
```

</details>

<a id="f0648"></a>

## F0648：wrap_gdr_driver_get_version

用途：查询 GDRCopy 驱动版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>gdr_driver_get_version(g, major, minor);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  gdr_driver_get_version(g, major, minor);
  return ncclSuccess;
}
```

</details>

<a id="f0649"></a>

## F0649：wrap_gdr_copy_to_mapping

用途：向 CPU 可见的 GPU 映射复制数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>GDRCHECK(gdr_copy_to_mapping(handle, map_d_ptr, h_ptr, size));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  GDRCHECK(gdr_copy_to_mapping(handle, map_d_ptr, h_ptr, size));
  return ncclSuccess;
}
```

</details>

<a id="f0650"></a>

## F0650：wrap_gdr_copy_from_mapping

用途：从 CPU 可见的 GPU 映射读回数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>GDRCHECK(gdr_copy_from_mapping(handle, h_ptr, map_d_ptr, size));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  GDRCHECK(gdr_copy_from_mapping(handle, h_ptr, map_d_ptr, size));
  return ncclSuccess;
}
```

</details>

<a id="f0651"></a>

## F0651：ncclGdrInit

用途：动态初始化 GDRCopy 并取得映射句柄，失败时由调用者决定是否使用普通回退路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int libMajor, libMinor, drvMajor, drvMinor;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>gdr_t handle = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(wrap_gdr_symbols() == ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>handle = wrap_gdr_open();</code> | 打开 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 6 | 2 / 条件判断 | <code>(handle != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 4 / 准备局部状态 | <code>ncclResult_t res;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(wrap_gdr_runtime_get_version(&libMajor, &libMinor), res, error);</code> | 失败时保存错误并跳到清理标签，成功才继续；查询 GDRCopy 用户库版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 10 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(wrap_gdr_driver_get_version(handle, &drvMajor, &drvMinor), res, error);</code> | 失败时保存错误并跳到清理标签，成功才继续；查询 GDRCopy 驱动版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 11 | 4 / 条件判断 | <code>(libMajor &lt; 2 &#124;&#124; (libMajor == 2 && libMinor &lt; 1) &#124;&#124; drvMajor &lt; 2 &#124;&#124; (drvMajor == 2 && drvMinor &lt; 1))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 6 / 控制转移 | <code>goto error;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 14 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 6 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "GDRCOPY enabled library %d.%d driver %d.%d", libMajor, libMinor, drvMajor, drvMinor);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 函数返回 | <code>return handle;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 17 | 0 / 跳转标签 | <code>error</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 18 | 1 / 条件判断 | <code>(handle != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>(void) wrap_gdr_close(handle);</code> | 关闭 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 21 | 0 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int libMajor, libMinor, drvMajor, drvMinor;
  gdr_t handle = NULL;
  // Dynamically load the GDRAPI library symbols
  if (wrap_gdr_symbols() == ncclSuccess) {
    handle = wrap_gdr_open();

    if (handle != NULL) {
      ncclResult_t res;

      // Query the version of libgdrapi
      NCCLCHECKGOTO(wrap_gdr_runtime_get_version(&libMajor, &libMinor), res, error);

      // Query the version of gdrdrv driver
      NCCLCHECKGOTO(wrap_gdr_driver_get_version(handle, &drvMajor, &drvMinor), res, error);

      // Only support GDRAPI 2.1 and later
      if (libMajor < 2 || (libMajor == 2 && libMinor < 1) || drvMajor < 2 || (drvMajor == 2 && drvMinor < 1)) {
        goto error;
      }
      else
        INFO(NCCL_INIT, "GDRCOPY enabled library %d.%d driver %d.%d", libMajor, libMinor, drvMajor, drvMinor);
    }
  }
  return handle;
error:
  if (handle != NULL) (void) wrap_gdr_close(handle);
  return NULL;
}
```

</details>

<a id="f0652"></a>

## F0652：ncclGdrCudaCalloc

用途：分配显存并建立 GDRCopy 的 CPU 映射，返回 CUDA 地址、CPU 可写映射与释放所需描述符。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>gdr_info_t info;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>size_t mapSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>gdr_mh_t mh;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>char *devMem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>void *gdrMap;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>mapSize = sizeof(T)*nelem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>ALIGN_SIZE(mapSize, GPU_PAGE_SIZE);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCudaCalloc(&devMem, mapSize+GPU_PAGE_SIZE-1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 0 / 准备局部状态 | <code>uint64_t alignedAddr = (((uint64_t) devMem) + GPU_PAGE_OFFSET) & GPU_PAGE_MASK;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>size_t align = alignedAddr - (uint64_t)devMem;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_gdr_pin_buffer(ncclGdrCopy, alignedAddr, mapSize, 0, 0, &mh));</code> | 失败时可能提前return，成功才继续；固定 GPU 显存以供映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_gdr_map(ncclGdrCopy, mh, &gdrMap, mapSize));</code> | 失败时可能提前return，成功才继续；把已固定 GPU 内存映射到 CPU 地址空间；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_gdr_get_info(ncclGdrCopy, mh, &info));</code> | 失败时可能提前return，成功才继续；读取 GPU 显存映射属性及页偏移；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 14 | 0 / 准备局部状态 | <code>ssize_t off = info.va - alignedAddr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 准备局部状态 | <code>gdr_mem_desc_t* md;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&md, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>md-&gt;gdrDevMem = devMem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>md-&gt;gdrMap = gdrMap;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>md-&gt;gdrMapSize = mapSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>md-&gt;gdrOffset = off+align;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>md-&gt;gdrMh = mh;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>*gdrHandle = md;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>*ptr = (T *)((char *)gdrMap+off);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 条件判断 | <code>(devPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>*devPtr = (T *)(devMem+off+align);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "GDRCOPY : allocated devMem %p gdrMap %p offset %lx mh %lx mapSize %zi at %p", md-&gt;gdrDevMem, md-&gt;gdrMap, md-&gt;gdrOffset, md-&gt;gdrMh.h, md-&gt;gdrMapSize, *ptr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  gdr_info_t info;
  size_t mapSize;
  gdr_mh_t mh;
  char *devMem;
  void *gdrMap;

  mapSize = sizeof(T)*nelem;

  // GDRCOPY Pinned buffer has to be a minimum of a GPU_PAGE_SIZE
  ALIGN_SIZE(mapSize, GPU_PAGE_SIZE);
  // GDRCOPY Pinned buffer has to be GPU_PAGE_SIZE aligned too
  NCCLCHECK(ncclCudaCalloc(&devMem, mapSize+GPU_PAGE_SIZE-1));
  uint64_t alignedAddr = (((uint64_t) devMem) + GPU_PAGE_OFFSET) & GPU_PAGE_MASK;
  size_t align = alignedAddr - (uint64_t)devMem;

  //TRACE(NCCL_INIT, "GDRCOPY: Pin buffer 0x%lx (%p) align %zi size %zi", alignedAddr, devMem, align, mapSize);
  NCCLCHECK(wrap_gdr_pin_buffer(ncclGdrCopy, alignedAddr, mapSize, 0, 0, &mh));

  NCCLCHECK(wrap_gdr_map(ncclGdrCopy, mh, &gdrMap, mapSize));
  //TRACE(NCCL_INIT, "GDRCOPY : mapped %p (0x%lx) at %p", devMem, alignedAddr, gdrMap);

  NCCLCHECK(wrap_gdr_get_info(ncclGdrCopy, mh, &info));

  // Will offset ever be non zero ?
  ssize_t off = info.va - alignedAddr;

  gdr_mem_desc_t* md;
  NCCLCHECK(ncclCalloc(&md, 1));
  md->gdrDevMem = devMem;
  md->gdrMap = gdrMap;
  md->gdrMapSize = mapSize;
  md->gdrOffset = off+align;
  md->gdrMh = mh;
  *gdrHandle = md;

  *ptr = (T *)((char *)gdrMap+off);
  if (devPtr) *devPtr = (T *)(devMem+off+align);

  TRACE(NCCL_INIT, "GDRCOPY : allocated devMem %p gdrMap %p offset %lx mh %lx mapSize %zi at %p",
       md->gdrDevMem, md->gdrMap, md->gdrOffset, md->gdrMh.h, md->gdrMapSize, *ptr);

  return ncclSuccess;
}
```

</details>

<a id="f0653"></a>

## F0653：ncclGdrCudaCopy

用途：通过 GDRCopy 映射把指定对象写入显存，用于低延迟提交小量控制/工作数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>gdr_mem_desc_t *md = (gdr_mem_desc_t*)gdrHandle;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_gdr_copy_to_mapping(md-&gt;gdrMh, dst, src, nelem*sizeof(T)));</code> | 失败时可能提前return，成功才继续；向 CPU 可见的 GPU 映射复制数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  gdr_mem_desc_t *md = (gdr_mem_desc_t*)gdrHandle;
  NCCLCHECK(wrap_gdr_copy_to_mapping(md->gdrMh, dst, src, nelem*sizeof(T)));
  return ncclSuccess;
}
```

</details>

<a id="f0654"></a>

## F0654：ncclGdrCudaFree

用途：按映射、pin、显存分配的依赖顺序撤销 GDRCopy 资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>gdr_mem_desc_t *md = (gdr_mem_desc_t*)gdrHandle;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_gdr_unmap(ncclGdrCopy, md-&gt;gdrMh, md-&gt;gdrMap, md-&gt;gdrMapSize));</code> | 失败时可能提前return，成功才继续；解除 CPU 映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(wrap_gdr_unpin_buffer(ncclGdrCopy, md-&gt;gdrMh));</code> | 失败时可能提前return，成功才继续；解除 GPU 显存固定；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaFree(md-&gt;gdrDevMem));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>free(md);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  gdr_mem_desc_t *md = (gdr_mem_desc_t*)gdrHandle;
  NCCLCHECK(wrap_gdr_unmap(ncclGdrCopy, md->gdrMh, md->gdrMap, md->gdrMapSize));
  NCCLCHECK(wrap_gdr_unpin_buffer(ncclGdrCopy, md->gdrMh));
  CUDACHECK(cudaFree(md->gdrDevMem));
  free(md);

  return ncclSuccess;
}
```

</details>

