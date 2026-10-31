# src/include/nvtx3/nvtxDetail/nvtxInit.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0742"></a>

## F0742：nvtxInitializeInjectionLibrary

用途：查找并加载性能工具注入库，协商导出表并建立可用回调；没有工具时使用空操作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const char* const initFuncName = "InitializeInjectionNvtx2";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>NvtxInitializeInjectionNvtxFunc_t init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>NVTX_DLLHANDLE injectionLibraryHandle = (NVTX_DLLHANDLE)0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int entryPointStatus = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 编译期分支/指令 | <code>#if NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)NVTX_DLLFUNC(0, initFuncName);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 编译期分支/指令 | <code>#if NVTX_SUPPORT_DYNAMIC_INJECTION_LIBRARY</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 8 | 1 / 条件判断 | <code>(!init_fnptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 编译期分支/指令 | <code>#if NVTX_SUPPORT_ENV_VARS</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 11 | 4 / 准备局部状态 | <code>const NVTX_PATHCHAR* const nvtxEnvVarName = (sizeof(void*) == 4) ? NVTX_STR("NVTX_INJECTION32_PATH") : NVTX_STR("NVTX_INJECTION64_PATH");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 3 / 准备局部状态 | <code>NVTX_PATHCHAR injectionLibraryPathBuf[NVTX_BUFSIZE];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 3 / 准备局部状态 | <code>const NVTX_PATHCHAR* injectionLibraryPath = (const NVTX_PATHCHAR*)0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 3 / 调用 / 计算 / 写入 | <code>(void)injectionLibraryPathBuf;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 15 | 3 / 编译期分支/指令 | <code>#if NVTX_SUPPORT_ENV_VARS</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 16 | 4 / 编译期分支/指令 | <code>#if defined(_MSC_VER)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>injectionLibraryPath = NVTX_GETENV(nvtxEnvVarName);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 4 / 编译期分支/指令 | <code>#if defined(_MSC_VER)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 19 | 3 / 编译期分支/指令 | <code>#if defined(__ANDROID__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 20 | 4 / 条件判断 | <code>(!injectionLibraryPath)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 6 / 准备局部状态 | <code>const char *bits = (sizeof(void*) == 4) ? "32" : "64";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 6 / 准备局部状态 | <code>char cmdlineBuf[32];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 6 / 准备局部状态 | <code>char pkgName[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 6 / 准备局部状态 | <code>int count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 6 / 准备局部状态 | <code>int pid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 6 / 准备局部状态 | <code>FILE *fp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 6 / 准备局部状态 | <code>size_t bytesRead;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 6 / 准备局部状态 | <code>size_t pos;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 6 / 调用 / 计算 / 写入 | <code>pid = (int)getpid();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 6 / 调用 / 计算 / 写入 | <code>count = snprintf(cmdlineBuf, sizeof(cmdlineBuf), "/proc/%d/cmdline", pid);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 6 / 条件判断 | <code>(count &lt;= 0 &#124;&#124; count &gt;= (int)sizeof(cmdlineBuf))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 8 / 调用 / 计算 / 写入 | <code>NVTX_ERR("Path buffer too small for: /proc/%d/cmdline\n", pid);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 8 / 函数返回 | <code>return NVTX_ERR_INIT_ACCESS_LIBRARY;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 36 | 6 / 调用 / 计算 / 写入 | <code>fp = fopen(cmdlineBuf, "r");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 6 / 条件判断 | <code>(!fp)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 8 / 调用 / 计算 / 写入 | <code>NVTX_ERR("File couldn't be opened: %s\n", cmdlineBuf);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 8 / 函数返回 | <code>return NVTX_ERR_INIT_ACCESS_LIBRARY;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 41 | 6 / 调用 / 计算 / 写入 | <code>bytesRead = fread(pkgName, 1, sizeof(pkgName) - 1, fp);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 6 / 调用 / 计算 / 写入 | <code>fclose(fp);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 6 / 条件判断 | <code>(bytesRead == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 8 / 调用 / 计算 / 写入 | <code>NVTX_ERR("Package name couldn't be read from file: %s\n", cmdlineBuf);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 8 / 函数返回 | <code>return NVTX_ERR_INIT_ACCESS_LIBRARY;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 47 | 6 / 调用 / 计算 / 写入 | <code>pkgName[bytesRead] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 6 / 调用 / 计算 / 写入 | <code>pos = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 6 / 循环 | <code>初始 ；继续条件 (pos &lt; bytesRead && pkgName[pos] != ':' && pkgName[pos] != '\0')；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 50 | 7 / 调用 / 计算 / 写入 | <code>++pos;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 52 | 6 / 调用 / 计算 / 写入 | <code>pkgName[pos] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 53 | 6 / 调用 / 计算 / 写入 | <code>count = snprintf(injectionLibraryPathBuf, NVTX_BUFSIZE, "/data/data/%s/files/libNvtxInjection%s.so", pkgName, bits);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 6 / 条件判断 | <code>(count &lt;= 0 &#124;&#124; count &gt;= NVTX_BUFSIZE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 55 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 56 | 8 / 调用 / 计算 / 写入 | <code>NVTX_ERR("Path buffer too small for: /data/data/%s/files/libNvtxInjection%s.so\n", pkgName, bits);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 57 | 8 / 函数返回 | <code>return NVTX_ERR_INIT_ACCESS_LIBRARY;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 58 | 6 / 条件判断 | <code>(injectionLibraryPathBuf[0] == '/')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 59 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 60 | 8 / 编译期分支/指令 | <code>#if (__ANDROID_API__ &lt; 21)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 61 | 9 / 准备局部状态 | <code>int access_err = access(injectionLibraryPathBuf, F_OK &#124; R_OK);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 62 | 9 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 63 | 10 / 准备局部状态 | <code>int access_err = faccessat(AT_FDCWD, injectionLibraryPathBuf, F_OK &#124; R_OK, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 64 | 8 / 条件判断 | <code>(access_err != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 65 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 66 | 10 / 调用 / 计算 / 写入 | <code>NVTX_ERR("Injection library path wasn't accessible [code=%s] [path=%s]\n", strerror(errno), injectionLibraryPathBuf);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 10 / 函数返回 | <code>return NVTX_ERR_INIT_ACCESS_LIBRARY;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 68 | 6 / 调用 / 计算 / 写入 | <code>injectionLibraryPath = injectionLibraryPathBuf;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 69 | 3 / 条件判断 | <code>(injectionLibraryPath)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 70 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 71 | 5 / 调用 / 计算 / 写入 | <code>injectionLibraryHandle = NVTX_DLLOPEN(injectionLibraryPath);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 5 / 条件判断 | <code>(!injectionLibraryHandle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 73 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 74 | 7 / 调用 / 计算 / 写入 | <code>NVTX_ERR("Failed to load injection library\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 75 | 7 / 函数返回 | <code>return NVTX_ERR_INIT_LOAD_LIBRARY;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 76 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 77 | 7 / 调用 / 计算 / 写入 | <code>init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)NVTX_DLLFUNC(injectionLibraryHandle, initFuncName);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 7 / 条件判断 | <code>(!init_fnptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 79 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 80 | 9 / 调用 / 计算 / 写入 | <code>NVTX_DLLCLOSE(injectionLibraryHandle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 81 | 9 / 调用 / 计算 / 写入 | <code>NVTX_ERR("Failed to get address of function InitializeInjectionNvtx2 from injection library\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 82 | 9 / 函数返回 | <code>return NVTX_ERR_INIT_MISSING_LIBRARY_ENTRY_POINT;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 83 | 0 / 编译期分支/指令 | <code>#if NVTX_SUPPORT_STATIC_INJECTION_LIBRARY</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 84 | 1 / 条件判断 | <code>(!init_fnptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 85 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 86 | 3 / 条件判断 | <code>(InitializeInjectionNvtx2_fnptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 87 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 88 | 5 / 调用 / 计算 / 写入 | <code>init_fnptr = InitializeInjectionNvtx2_fnptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 89 | 0 / 条件判断 | <code>(!init_fnptr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 90 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 91 | 2 / 函数返回 | <code>return NVTX_ERR_NO_INJECTION_LIBRARY_AVAILABLE;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 92 | 0 / 调用 / 计算 / 写入 | <code>entryPointStatus = init_fnptr(NVTX_VERSIONED_IDENTIFIER(nvtxGetExportTable));</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 93 | 0 / 条件判断 | <code>(entryPointStatus == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 94 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 95 | 2 / 调用 / 计算 / 写入 | <code>NVTX_ERR("Failed to initialize injection library -- initialization function returned 0\n");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 96 | 2 / 条件判断 | <code>(injectionLibraryHandle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 97 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 98 | 4 / 调用 / 计算 / 写入 | <code>NVTX_DLLCLOSE(injectionLibraryHandle);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 99 | 2 / 函数返回 | <code>return NVTX_ERR_INIT_FAILED_LIBRARY_ENTRY_POINT;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 100 | 0 / 函数返回 | <code>return NVTX_SUCCESS;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    const char* const initFuncName = "InitializeInjectionNvtx2";
    NvtxInitializeInjectionNvtxFunc_t init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)0;
    NVTX_DLLHANDLE injectionLibraryHandle = (NVTX_DLLHANDLE)0;
    int entryPointStatus = 0;

#if NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY
    /* Use POSIX global symbol chain to query for init function from any module */
    init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)NVTX_DLLFUNC(0, initFuncName);
#endif

#if NVTX_SUPPORT_DYNAMIC_INJECTION_LIBRARY
    /* Try discovering dynamic injection library to load */
    if (!init_fnptr)
    {
#if NVTX_SUPPORT_ENV_VARS
        /* If env var NVTX_INJECTION64_PATH is set, it should contain the path
        *  to a 64-bit dynamic NVTX injection library (and similar for 32-bit). */
        const NVTX_PATHCHAR* const nvtxEnvVarName = (sizeof(void*) == 4)
            ? NVTX_STR("NVTX_INJECTION32_PATH")
            : NVTX_STR("NVTX_INJECTION64_PATH");
#endif /* NVTX_SUPPORT_ENV_VARS */
        NVTX_PATHCHAR injectionLibraryPathBuf[NVTX_BUFSIZE];
        const NVTX_PATHCHAR* injectionLibraryPath = (const NVTX_PATHCHAR*)0;

        /* Refer to this variable explicitly in case all references to it are #if'ed out */
        (void)injectionLibraryPathBuf;

#if NVTX_SUPPORT_ENV_VARS
        /* Disable the warning for getenv & _wgetenv -- this usage is safe because
        *  these functions are not called again before using the returned value. */
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif
        injectionLibraryPath = NVTX_GETENV(nvtxEnvVarName);
#if defined(_MSC_VER)
#pragma warning( pop )
#endif
#endif

#if defined(__ANDROID__)
        if (!injectionLibraryPath)
        {
            const char *bits = (sizeof(void*) == 4) ? "32" : "64";
            char cmdlineBuf[32];
            char pkgName[PATH_MAX];
            int count;
            int pid;
            FILE *fp;
            size_t bytesRead;
            size_t pos;

            pid = (int)getpid();
            count = snprintf(cmdlineBuf, sizeof(cmdlineBuf), "/proc/%d/cmdline", pid);
            if (count <= 0 || count >= (int)sizeof(cmdlineBuf))
            {
                NVTX_ERR("Path buffer too small for: /proc/%d/cmdline\n", pid);
                return NVTX_ERR_INIT_ACCESS_LIBRARY;
            }

            fp = fopen(cmdlineBuf, "r");
            if (!fp)
            {
                NVTX_ERR("File couldn't be opened: %s\n", cmdlineBuf);
                return NVTX_ERR_INIT_ACCESS_LIBRARY;
            }

            bytesRead = fread(pkgName, 1, sizeof(pkgName) - 1, fp);
            fclose(fp);
            if (bytesRead == 0)
            {
                NVTX_ERR("Package name couldn't be read from file: %s\n", cmdlineBuf);
                return NVTX_ERR_INIT_ACCESS_LIBRARY;
            }

            pkgName[bytesRead] = 0;

            /* String can contain colon as a process separator. In this case the package name is before the colon. */
            pos = 0;
            while (pos < bytesRead && pkgName[pos] != ':' && pkgName[pos] != '\0')
            {
                ++pos;
            }
            pkgName[pos] = 0;

            count = snprintf(injectionLibraryPathBuf, NVTX_BUFSIZE, "/data/data/%s/files/libNvtxInjection%s.so", pkgName, bits);
            if (count <= 0 || count >= NVTX_BUFSIZE)
            {
                NVTX_ERR("Path buffer too small for: /data/data/%s/files/libNvtxInjection%s.so\n", pkgName, bits);
                return NVTX_ERR_INIT_ACCESS_LIBRARY;
            }

            /* On Android, verify path is accessible due to aggressive file access restrictions. */
            /* For dlopen, if the filename contains a leading slash, then it is interpreted as a */
            /* relative or absolute pathname; otherwise it will follow the rules in ld.so. */
            if (injectionLibraryPathBuf[0] == '/')
            {
#if (__ANDROID_API__ < 21)
                int access_err = access(injectionLibraryPathBuf, F_OK | R_OK);
#else
                int access_err = faccessat(AT_FDCWD, injectionLibraryPathBuf, F_OK | R_OK, 0);
#endif
                if (access_err != 0)
                {
                    NVTX_ERR("Injection library path wasn't accessible [code=%s] [path=%s]\n", strerror(errno), injectionLibraryPathBuf);
                    return NVTX_ERR_INIT_ACCESS_LIBRARY;
                }
            }
            injectionLibraryPath = injectionLibraryPathBuf;
        }
#endif

        /* At this point, injectionLibraryPath is specified if a dynamic
        *  injection library was specified by a tool. */
        if (injectionLibraryPath)
        {
            /* Load the injection library */
            injectionLibraryHandle = NVTX_DLLOPEN(injectionLibraryPath);
            if (!injectionLibraryHandle)
            {
                NVTX_ERR("Failed to load injection library\n");
                return NVTX_ERR_INIT_LOAD_LIBRARY;
            }
            else
            {
                /* Attempt to get the injection library's entry-point */
                init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)NVTX_DLLFUNC(injectionLibraryHandle, initFuncName);
                if (!init_fnptr)
                {
                    NVTX_DLLCLOSE(injectionLibraryHandle);
                    NVTX_ERR("Failed to get address of function InitializeInjectionNvtx2 from injection library\n");
                    return NVTX_ERR_INIT_MISSING_LIBRARY_ENTRY_POINT;
                }
            }
        }
    }
#endif

#if NVTX_SUPPORT_STATIC_INJECTION_LIBRARY
    if (!init_fnptr)
    {
        /* Check weakly-defined function pointer.  A statically-linked injection can define this as
        *  a normal symbol and it will take precedence over a dynamic injection. */
        if (InitializeInjectionNvtx2_fnptr)
        {
            init_fnptr = InitializeInjectionNvtx2_fnptr;
        }
    }
#endif

    /* At this point, if init_fnptr is not set, then no tool has specified
    *  an NVTX injection library -- return non-success result so all NVTX
    *  API functions will be set to no-ops. */
    if (!init_fnptr)
    {
        return NVTX_ERR_NO_INJECTION_LIBRARY_AVAILABLE;
    }

    /* Invoke injection library's initialization function.  If it returns
    *  0 (failure) and a dynamic injection was loaded, unload it. */
    entryPointStatus = init_fnptr(NVTX_VERSIONED_IDENTIFIER(nvtxGetExportTable));
    if (entryPointStatus == 0)
    {
        NVTX_ERR("Failed to initialize injection library -- initialization function returned 0\n");
        if (injectionLibraryHandle)
        {
            NVTX_DLLCLOSE(injectionLibraryHandle);
        }
        return NVTX_ERR_INIT_FAILED_LIBRARY_ENTRY_POINT;
    }

    return NVTX_SUCCESS;
}
```

</details>

<a id="f0743"></a>

## F0743：nvtxInitOnce

用途：以一次性初始化状态和原子/锁协调工具注入，初始化后让标记API走真实工具回调或no-op。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>unsigned int old;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState == NVTX_INIT_STATE_COMPLETE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NVTX_ATOMIC_CAS_32( old, &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState, NVTX_INIT_STATE_STARTED, NVTX_INIT_STATE_FRESH);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 条件判断 | <code>(old == NVTX_INIT_STATE_FRESH)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 准备局部状态 | <code>int result;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 准备局部状态 | <code>int forceAllToNoops;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>result = NVTX_VERSIONED_IDENTIFIER(nvtxInitializeInjectionLibrary)();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>forceAllToNoops = result != NVTX_SUCCESS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>NVTX_VERSIONED_IDENTIFIER(nvtxSetInitFunctionsToNoops)(forceAllToNoops);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>NVTX_ATOMIC_WRITE_32( &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState, NVTX_INIT_STATE_COMPLETE);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>NVTX_MEMBAR();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 2 / 循环 | <code>初始 ；继续条件 (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState != NVTX_INIT_STATE_COMPLETE)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>NVTX_YIELD();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>NVTX_MEMBAR();</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    unsigned int old;
    if (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState == NVTX_INIT_STATE_COMPLETE)
    {
        return;
    }

    NVTX_ATOMIC_CAS_32(
        old,
        &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState,
        NVTX_INIT_STATE_STARTED,
        NVTX_INIT_STATE_FRESH);
    if (old == NVTX_INIT_STATE_FRESH)
    {
        int result;
        int forceAllToNoops;

        /* Load & initialize injection library -- it will assign the function pointers */
        result = NVTX_VERSIONED_IDENTIFIER(nvtxInitializeInjectionLibrary)();

        /* Set all pointers not assigned by the injection to null */
        forceAllToNoops = result != NVTX_SUCCESS; /* Set all to null if injection init failed */
        NVTX_VERSIONED_IDENTIFIER(nvtxSetInitFunctionsToNoops)(forceAllToNoops);

        /* Signal that initialization has finished, so now the assigned function pointers will be used */
        NVTX_ATOMIC_WRITE_32(
            &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState,
            NVTX_INIT_STATE_COMPLETE);
    }
    else /* Spin-wait until initialization has finished */
    {
        NVTX_MEMBAR();
        while (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState != NVTX_INIT_STATE_COMPLETE)
        {
            NVTX_YIELD();
            NVTX_MEMBAR();
        }
    }
}
```

</details>

