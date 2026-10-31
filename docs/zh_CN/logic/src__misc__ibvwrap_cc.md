# src/misc/ibvwrap.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0924"></a>

## F0924：wrap_ibv_symbols

用途：动态装载并解析 libibverbs 符号表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ibvState == ibvInitialized)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 条件判断 | <code>(ibvState == ibvError)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 条件判断 | <code>(__sync_bool_compare_and_swap(&ibvState, ibvUninitialized, ibvInitializing) == false)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 循环 | <code>初始 ；继续条件 (ibvState == ibvInitializing)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>pthread_yield();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 2 / 函数返回 | <code>return (ibvState == ibvInitialized) ? ncclSuccess : ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 0 / 准备局部状态 | <code>static void* ibvhandle = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>void* tmp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 准备局部状态 | <code>void** cast;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>ibvhandle=dlopen("libibverbs.so", RTLD_NOW);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 条件判断 | <code>(!ibvhandle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>ibvhandle=dlopen("libibverbs.so.1", RTLD_NOW);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 2 / 条件判断 | <code>(!ibvhandle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Failed to open libibverbs.so[.1]");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 4 / 控制转移 | <code>goto teardown;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 24 | 0 / 编译期分支/指令 | <code>#define LOAD_SYM(handle, symbol, funcptr) do { \</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 25 | 0 / 编译期分支/指令 | <code>#define LOAD_SYM_VERSION(handle, symbol, funcptr, version) do { \</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_get_device_list", ibv_internal_get_device_list);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_free_device_list", ibv_internal_free_device_list);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_get_device_name", ibv_internal_get_device_name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_open_device", ibv_internal_open_device);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_close_device", ibv_internal_close_device);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_get_async_event", ibv_internal_get_async_event);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_ack_async_event", ibv_internal_ack_async_event);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_query_device", ibv_internal_query_device);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_query_port", ibv_internal_query_port);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_query_gid", ibv_internal_query_gid);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_query_qp", ibv_internal_query_qp);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_alloc_pd", ibv_internal_alloc_pd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_dealloc_pd", ibv_internal_dealloc_pd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_reg_mr", ibv_internal_reg_mr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM_VERSION(ibvhandle, "ibv_reg_mr_iova2", ibv_internal_reg_mr_iova2, "IBVERBS_1.8");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_dereg_mr", ibv_internal_dereg_mr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_create_cq", ibv_internal_create_cq);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_destroy_cq", ibv_internal_destroy_cq);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_create_qp", ibv_internal_create_qp);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_modify_qp", ibv_internal_modify_qp);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_destroy_qp", ibv_internal_destroy_qp);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_fork_init", ibv_internal_fork_init);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>LOAD_SYM(ibvhandle, "ibv_event_type_str", ibv_internal_event_type_str);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 0 / 调用 / 计算 / 写入 | <code>ibvState = ibvInitialized;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 51 | 0 / 跳转标签 | <code>teardown</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 52 | 1 / 调用 / 计算 / 写入 | <code>ibv_internal_get_device_list = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_free_device_list = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_get_device_name = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_open_device = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 56 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_close_device = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_get_async_event = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_ack_async_event = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 59 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_query_device = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_query_port = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 61 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_query_gid = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_query_qp = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 63 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_alloc_pd = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 64 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_dealloc_pd = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 65 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_reg_mr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 66 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_reg_mr_iova2 = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_dereg_mr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 68 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_create_cq = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 69 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_destroy_cq = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_create_qp = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 71 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_modify_qp = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_destroy_qp = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 73 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_fork_init = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 74 | 0 / 调用 / 计算 / 写入 | <code>ibv_internal_event_type_str = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 75 | 0 / 条件判断 | <code>(ibvhandle != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 76 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 77 | 2 / 调用 / 计算 / 写入 | <code>dlclose(ibvhandle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 78 | 0 / 调用 / 计算 / 写入 | <code>ibvState = ibvError;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 0 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ibvState == ibvInitialized)
    return ncclSuccess;
  if (ibvState == ibvError)
    return ncclSystemError;

  if (__sync_bool_compare_and_swap(&ibvState, ibvUninitialized, ibvInitializing) == false) {
    // Another thread raced in front of us. Wait for it to be done.
    while (ibvState == ibvInitializing) pthread_yield();
    return (ibvState == ibvInitialized) ? ncclSuccess : ncclSystemError;
  }

  static void* ibvhandle = NULL;
  void* tmp;
  void** cast;

  ibvhandle=dlopen("libibverbs.so", RTLD_NOW);
  if (!ibvhandle) {
    ibvhandle=dlopen("libibverbs.so.1", RTLD_NOW);
    if (!ibvhandle) {
      INFO(NCCL_INIT, "Failed to open libibverbs.so[.1]");
      goto teardown;
    }
  }

#define LOAD_SYM(handle, symbol, funcptr) do {           \
    cast = (void**)&funcptr;                             \
    tmp = dlvsym(handle, symbol, IBVERBS_VERSION);       \
    if (tmp == NULL) {                                   \
      WARN("dlvsym failed on %s - %s version %s", symbol, dlerror(), IBVERBS_VERSION);  \
      goto teardown;                                     \
    }                                                    \
    *cast = tmp;                                         \
  } while (0)

// Attempt to load a specific symbol version - fail silently
#define LOAD_SYM_VERSION(handle, symbol, funcptr, version) do {  \
    cast = (void**)&funcptr;                                     \
    *cast = dlvsym(handle, symbol, version);                     \
  } while (0)

  LOAD_SYM(ibvhandle, "ibv_get_device_list", ibv_internal_get_device_list);
  LOAD_SYM(ibvhandle, "ibv_free_device_list", ibv_internal_free_device_list);
  LOAD_SYM(ibvhandle, "ibv_get_device_name", ibv_internal_get_device_name);
  LOAD_SYM(ibvhandle, "ibv_open_device", ibv_internal_open_device);
  LOAD_SYM(ibvhandle, "ibv_close_device", ibv_internal_close_device);
  LOAD_SYM(ibvhandle, "ibv_get_async_event", ibv_internal_get_async_event);
  LOAD_SYM(ibvhandle, "ibv_ack_async_event", ibv_internal_ack_async_event);
  LOAD_SYM(ibvhandle, "ibv_query_device", ibv_internal_query_device);
  LOAD_SYM(ibvhandle, "ibv_query_port", ibv_internal_query_port);
  LOAD_SYM(ibvhandle, "ibv_query_gid", ibv_internal_query_gid);
  LOAD_SYM(ibvhandle, "ibv_query_qp", ibv_internal_query_qp);
  LOAD_SYM(ibvhandle, "ibv_alloc_pd", ibv_internal_alloc_pd);
  LOAD_SYM(ibvhandle, "ibv_dealloc_pd", ibv_internal_dealloc_pd);
  LOAD_SYM(ibvhandle, "ibv_reg_mr", ibv_internal_reg_mr);
  // Cherry-pick the ibv_reg_mr_iova2 API from IBVERBS 1.8
  LOAD_SYM_VERSION(ibvhandle, "ibv_reg_mr_iova2", ibv_internal_reg_mr_iova2, "IBVERBS_1.8");
  LOAD_SYM(ibvhandle, "ibv_dereg_mr", ibv_internal_dereg_mr);
  LOAD_SYM(ibvhandle, "ibv_create_cq", ibv_internal_create_cq);
  LOAD_SYM(ibvhandle, "ibv_destroy_cq", ibv_internal_destroy_cq);
  LOAD_SYM(ibvhandle, "ibv_create_qp", ibv_internal_create_qp);
  LOAD_SYM(ibvhandle, "ibv_modify_qp", ibv_internal_modify_qp);
  LOAD_SYM(ibvhandle, "ibv_destroy_qp", ibv_internal_destroy_qp);
  LOAD_SYM(ibvhandle, "ibv_fork_init", ibv_internal_fork_init);
  LOAD_SYM(ibvhandle, "ibv_event_type_str", ibv_internal_event_type_str);

  ibvState = ibvInitialized;
  return ncclSuccess;

teardown:
  ibv_internal_get_device_list = NULL;
  ibv_internal_free_device_list = NULL;
  ibv_internal_get_device_name = NULL;
  ibv_internal_open_device = NULL;
  ibv_internal_close_device = NULL;
  ibv_internal_get_async_event = NULL;
  ibv_internal_ack_async_event = NULL;
  ibv_internal_query_device = NULL;
  ibv_internal_query_port = NULL;
  ibv_internal_query_gid = NULL;
  ibv_internal_query_qp = NULL;
  ibv_internal_alloc_pd = NULL;
  ibv_internal_dealloc_pd = NULL;
  ibv_internal_reg_mr = NULL;
  ibv_internal_reg_mr_iova2 = NULL;
  ibv_internal_dereg_mr = NULL;
  ibv_internal_create_cq = NULL;
  ibv_internal_destroy_cq = NULL;
  ibv_internal_create_qp = NULL;
  ibv_internal_modify_qp = NULL;
  ibv_internal_destroy_qp = NULL;
  ibv_internal_fork_init = NULL;
  ibv_internal_event_type_str = NULL;

  if (ibvhandle != NULL) dlclose(ibvhandle);
  ibvState = ibvError;
  return ncclSystemError;
}
```

</details>

<a id="f0925"></a>

## F0925：wrap_ibv_fork_init

用途：初始化 verbs 注册页的 fork 处理；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK(ibv_internal_fork_init, ibv_internal_fork_init(), -1, "ibv_fork_init");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_INT_CHECK(ibv_internal_fork_init, ibv_internal_fork_init(), -1, "ibv_fork_init");
}
```

</details>

<a id="f0926"></a>

## F0926：wrap_ibv_get_device_list

用途：获取可用 RDMA 设备列表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*ret = ibv_internal_get_device_list(num_devices);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 条件判断 | <code>(*ret == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>*num_devices = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *ret = ibv_internal_get_device_list(num_devices);
  if (*ret == NULL) *num_devices = 0;
  return ncclSuccess;
}
```

</details>

<a id="f0927"></a>

## F0927：wrap_ibv_free_device_list

用途：归还设备枚举列表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_PASSTHRU(ibv_internal_free_device_list, ibv_internal_free_device_list(list));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_PASSTHRU(ibv_internal_free_device_list, ibv_internal_free_device_list(list));
}
```

</details>

<a id="f0928"></a>

## F0928：wrap_ibv_get_device_name

用途：读取设备名字；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ibv_internal_get_device_name == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>exit(-1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 函数返回 | <code>return ibv_internal_get_device_name(device);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ibv_internal_get_device_name == NULL) {
    WARN("lib wrapper not initialized.");
    exit(-1);
  }
  return ibv_internal_get_device_name(device);
}
```

</details>

<a id="f0929"></a>

## F0929：wrap_ibv_open_device

用途：打开设备得到 verbs context；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_PTR_CHECK(ibv_internal_open_device, ibv_internal_open_device(device), *ret, NULL, "ibv_open_device");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ /*returns 0 on success, -1 on failure*/
  IBV_PTR_CHECK(ibv_internal_open_device, ibv_internal_open_device(device), *ret, NULL, "ibv_open_device");
}
```

</details>

<a id="f0930"></a>

## F0930：wrap_ibv_close_device

用途：关闭设备上下文；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK(ibv_internal_close_device, ibv_internal_close_device(context), -1, "ibv_close_device");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ /*returns 0 on success, -1 on failure*/
  IBV_INT_CHECK(ibv_internal_close_device, ibv_internal_close_device(context), -1, "ibv_close_device");
}
```

</details>

<a id="f0931"></a>

## F0931：wrap_ibv_get_async_event

用途：等待读取设备异步事件；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK(ibv_internal_get_async_event, ibv_internal_get_async_event(context, event), -1, "ibv_get_async_event");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ /*returns 0 on success, and -1 on error*/
  IBV_INT_CHECK(ibv_internal_get_async_event, ibv_internal_get_async_event(context, event), -1, "ibv_get_async_event");
}
```

</details>

<a id="f0932"></a>

## F0932：wrap_ibv_ack_async_event

用途：确认并释放已处理异步事件；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_PASSTHRU(ibv_internal_ack_async_event, ibv_internal_ack_async_event(event));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_PASSTHRU(ibv_internal_ack_async_event, ibv_internal_ack_async_event(event));
}
```

</details>

<a id="f0933"></a>

## F0933：wrap_ibv_query_device

用途：读取设备能力限制；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_device, ibv_internal_query_device(context, device_attr), 0, "ibv_query_device");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_device, ibv_internal_query_device(context, device_attr), 0, "ibv_query_device");
}
```

</details>

<a id="f0934"></a>

## F0934：wrap_ibv_query_port

用途：读取端口状态/地址与速率能力；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_port, ibv_internal_query_port(context, port_num, port_attr), 0, "ibv_query_port");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_port, ibv_internal_query_port(context, port_num, port_attr), 0, "ibv_query_port");
}
```

</details>

<a id="f0935"></a>

## F0935：wrap_ibv_query_gid

用途：读取指定端口/GID索引的地址；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_gid, ibv_internal_query_gid(context, port_num, index, gid), 0, "ibv_query_gid");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_gid, ibv_internal_query_gid(context, port_num, index, gid), 0, "ibv_query_gid");
}
```

</details>

<a id="f0936"></a>

## F0936：wrap_ibv_query_qp

用途：查询 QP 当前状态和属性；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_qp, ibv_internal_query_qp(qp, attr, attr_mask, init_attr), 0, "ibv_query_qp");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_qp, ibv_internal_query_qp(qp, attr, attr_mask, init_attr), 0, "ibv_query_qp");
}
```

</details>

<a id="f0937"></a>

## F0937：wrap_ibv_alloc_pd

用途：分配保护域，隔离允许访问的 QP/MR 集合；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_PTR_CHECK(ibv_internal_alloc_pd, ibv_internal_alloc_pd(context), *ret, NULL, "ibv_alloc_pd");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_PTR_CHECK(ibv_internal_alloc_pd, ibv_internal_alloc_pd(context), *ret, NULL, "ibv_alloc_pd");
}
```

</details>

<a id="f0938"></a>

## F0938：wrap_ibv_dealloc_pd

用途：释放保护域；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_dealloc_pd, ibv_internal_dealloc_pd(pd), 0, "ibv_dealloc_pd");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_dealloc_pd, ibv_internal_dealloc_pd(pd), 0, "ibv_dealloc_pd");
}
```

</details>

<a id="f0939"></a>

## F0939：wrap_ibv_reg_mr

用途：把指定虚拟地址范围注册到 PD 并取得 lkey/rkey；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_PTR_CHECK(ibv_internal_reg_mr, ibv_internal_reg_mr(pd, addr, length, access), *ret, NULL, "ibv_reg_mr");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_PTR_CHECK(ibv_internal_reg_mr, ibv_internal_reg_mr(pd, addr, length, access), *ret, NULL, "ibv_reg_mr");
}
```

</details>

<a id="f0940"></a>

## F0940：wrap_direct_ibv_reg_mr

用途：直接调用选定 verbs 注册入口并返回 MR 指针，供需要原生指针/错误约定的能力探测使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ibv_internal_reg_mr == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("lib wrapper not initialized.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 函数返回 | <code>return ibv_internal_reg_mr(pd, addr, length, access);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ibv_internal_reg_mr == NULL) {
    WARN("lib wrapper not initialized.");
    return NULL;
  }
  return ibv_internal_reg_mr(pd, addr, length, access);
}
```

</details>

<a id="f0941"></a>

## F0941：wrap_ibv_reg_mr_iova2

用途：注册内存并指定 HCA 使用的 IOVA 地址及访问权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ibv_internal_reg_mr_iova2 == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 条件判断 | <code>(ret == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>IBV_PTR_CHECK(ibv_internal_reg_mr_iova2, ibv_internal_reg_mr_iova2(pd, addr, length, iova, access), *ret, NULL, "ibv_reg_mr_iova2");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ibv_internal_reg_mr_iova2 == NULL) {
    return ncclInternalError;
  }
  if (ret == NULL) { return ncclSuccess; } // Assume dummy call
  IBV_PTR_CHECK(ibv_internal_reg_mr_iova2, ibv_internal_reg_mr_iova2(pd, addr, length, iova, access), *ret, NULL, "ibv_reg_mr_iova2");
}
```

</details>

<a id="f0942"></a>

## F0942：wrap_ibv_dereg_mr

用途：注销一个真实 MR；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_dereg_mr, ibv_internal_dereg_mr(mr), 0, "ibv_dereg_mr");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_dereg_mr, ibv_internal_dereg_mr(mr), 0, "ibv_dereg_mr");
}
```

</details>

<a id="f0943"></a>

## F0943：wrap_ibv_create_cq

用途：创建完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_PTR_CHECK(ibv_internal_create_cq, ibv_internal_create_cq(context, cqe, cq_context, channel, comp_vector), *ret, NULL, "ibv_create_cq");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_PTR_CHECK(ibv_internal_create_cq, ibv_internal_create_cq(context, cqe, cq_context, channel, comp_vector), *ret, NULL, "ibv_create_cq");
}
```

</details>

<a id="f0944"></a>

## F0944：wrap_ibv_destroy_cq

用途：销毁完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_destroy_cq, ibv_internal_destroy_cq(cq), 0, "ibv_destroy_cq");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_destroy_cq, ibv_internal_destroy_cq(cq), 0, "ibv_destroy_cq");
}
```

</details>

<a id="f0945"></a>

## F0945：wrap_ibv_destroy_qp

用途：销毁队列对；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_destroy_qp, ibv_internal_destroy_qp(qp), 0, "ibv_destroy_qp");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_destroy_qp, ibv_internal_destroy_qp(qp), 0, "ibv_destroy_qp");
}
```

</details>

<a id="f0946"></a>

## F0946：wrap_ibv_create_qp

用途：创建指定类型和队列容量的 QP；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_PTR_CHECK(ibv_internal_create_qp, ibv_internal_create_qp(pd, qp_init_attr), *ret, NULL, "ibv_create_qp");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  IBV_PTR_CHECK(ibv_internal_create_qp, ibv_internal_create_qp(pd, qp_init_attr), *ret, NULL, "ibv_create_qp");
}
```

</details>

<a id="f0947"></a>

## F0947：wrap_ibv_modify_qp

用途：按属性掩码修改 QP 状态/路由/权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>IBV_INT_CHECK_RET_ERRNO(ibv_internal_modify_qp, ibv_internal_modify_qp(qp, attr, attr_mask), 0, "ibv_modify_qp");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_modify_qp, ibv_internal_modify_qp(qp, attr, attr_mask), 0, "ibv_modify_qp");
}
```

</details>

<a id="f0948"></a>

## F0948：wrap_ibv_event_type_str

用途：把设备事件枚举转换成诊断字符串；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*ret = (char *) ibv_internal_event_type_str(event);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *ret = (char *) ibv_internal_event_type_str(event);
  return ncclSuccess;
}
```

</details>

