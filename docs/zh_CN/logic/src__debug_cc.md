# src/debug.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0381"></a>

## F0381：ncclDebugInit

用途：读取日志级别、子系统及输出路径配置，建立线程安全的日志输出状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&ncclDebugLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 2 | 0 / 条件判断 | <code>(ncclDebugLevel != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclDebugLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 准备局部状态 | <code>const char* nccl_debug = getenv("NCCL_DEBUG");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 条件判断 | <code>(nccl_debug == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>ncclDebugLevel = NCCL_LOG_NONE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 2 / 条件判断 | <code>(strcasecmp(nccl_debug, "VERSION") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>ncclDebugLevel = NCCL_LOG_VERSION;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 4 / 条件判断 | <code>(strcasecmp(nccl_debug, "WARN") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 6 / 调用 / 计算 / 写入 | <code>ncclDebugLevel = NCCL_LOG_WARN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 6 / 条件判断 | <code>(strcasecmp(nccl_debug, "INFO") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 8 / 调用 / 计算 / 写入 | <code>ncclDebugLevel = NCCL_LOG_INFO;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 7 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 8 / 条件判断 | <code>(strcasecmp(nccl_debug, "ABORT") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 9 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 10 / 调用 / 计算 / 写入 | <code>ncclDebugLevel = NCCL_LOG_ABORT;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 9 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 27 | 10 / 条件判断 | <code>(strcasecmp(nccl_debug, "TRACE") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 11 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 12 / 调用 / 计算 / 写入 | <code>ncclDebugLevel = NCCL_LOG_TRACE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 准备局部状态 | <code>char* ncclDebugSubsysEnv = getenv("NCCL_DEBUG_SUBSYS");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 0 / 条件判断 | <code>(ncclDebugSubsysEnv != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 2 / 准备局部状态 | <code>int invert = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 34 | 2 / 条件判断 | <code>(ncclDebugSubsysEnv[0] == '^')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 4 / 调用 / 计算 / 写入 | <code>invert = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 4 / 调用 / 计算 / 写入 | <code>ncclDebugSubsysEnv++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 2 / 调用 / 计算 / 写入 | <code>ncclDebugMask = invert ? ~0ULL : 0ULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 2 / 准备局部状态 | <code>char *ncclDebugSubsys = strdup(ncclDebugSubsysEnv);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 2 / 准备局部状态 | <code>char *subsys = strtok(ncclDebugSubsys, ",");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 2 / 循环 | <code>初始 ；继续条件 (subsys != NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 42 | 3 / 准备局部状态 | <code>uint64_t mask = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 3 / 条件判断 | <code>(strcasecmp(subsys, "INIT") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 5 / 调用 / 计算 / 写入 | <code>mask = NCCL_INIT;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 47 | 5 / 条件判断 | <code>(strcasecmp(subsys, "COLL") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 7 / 调用 / 计算 / 写入 | <code>mask = NCCL_COLL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 51 | 7 / 条件判断 | <code>(strcasecmp(subsys, "P2P") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 52 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 53 | 9 / 调用 / 计算 / 写入 | <code>mask = NCCL_P2P;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 54 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 55 | 9 / 条件判断 | <code>(strcasecmp(subsys, "SHM") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 56 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 57 | 11 / 调用 / 计算 / 写入 | <code>mask = NCCL_SHM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 58 | 10 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 59 | 11 / 条件判断 | <code>(strcasecmp(subsys, "NET") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 60 | 12 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 61 | 13 / 调用 / 计算 / 写入 | <code>mask = NCCL_NET;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 62 | 12 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 63 | 13 / 条件判断 | <code>(strcasecmp(subsys, "GRAPH") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 14 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 15 / 调用 / 计算 / 写入 | <code>mask = NCCL_GRAPH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 66 | 14 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 67 | 15 / 条件判断 | <code>(strcasecmp(subsys, "TUNING") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 16 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 17 / 调用 / 计算 / 写入 | <code>mask = NCCL_TUNING;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 16 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 71 | 17 / 条件判断 | <code>(strcasecmp(subsys, "ENV") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 72 | 18 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 73 | 19 / 调用 / 计算 / 写入 | <code>mask = NCCL_ENV;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 74 | 18 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 75 | 19 / 条件判断 | <code>(strcasecmp(subsys, "ALLOC") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 76 | 20 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 77 | 21 / 调用 / 计算 / 写入 | <code>mask = NCCL_ALLOC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 78 | 20 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 79 | 21 / 条件判断 | <code>(strcasecmp(subsys, "ALL") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 80 | 22 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 81 | 23 / 调用 / 计算 / 写入 | <code>mask = NCCL_ALL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 82 | 3 / 条件判断 | <code>(mask)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 83 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 84 | 5 / 条件判断 | <code>(invert)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 85 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 86 | 7 / 调用 / 计算 / 写入 | <code>ncclDebugMask &= ~mask;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 87 | 6 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 88 | 7 / 调用 / 计算 / 写入 | <code>ncclDebugMask &#124;= mask;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 89 | 3 / 调用 / 计算 / 写入 | <code>subsys = strtok(NULL, ",");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 90 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 91 | 2 / 调用 / 计算 / 写入 | <code>free(ncclDebugSubsys);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 92 | 0 / 准备局部状态 | <code>const char* ncclDebugFileEnv = getenv("NCCL_DEBUG_FILE");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 93 | 0 / 条件判断 | <code>(ncclDebugLevel &gt; NCCL_LOG_VERSION && ncclDebugFileEnv != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 94 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 95 | 2 / 准备局部状态 | <code>int c = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 96 | 2 / 准备局部状态 | <code>char debugFn[PATH_MAX+1] = "";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 97 | 2 / 准备局部状态 | <code>char *dfn = debugFn;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 98 | 2 / 循环 | <code>初始 ；继续条件 (ncclDebugFileEnv[c] != '\0' && c &lt; PATH_MAX)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 99 | 3 / 条件判断 | <code>(ncclDebugFileEnv[c++] != '%')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 100 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 101 | 5 / 调用 / 计算 / 写入 | <code>*dfn++ = ncclDebugFileEnv[c-1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 102 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 103 | 3 / 多路分派 | <code>(ncclDebugFileEnv[c++])</code> | 按case匹配进入；没有break/return/goto时可能贯穿下一个case。 |
| 104 | 4 / case / default | <code>'%'</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 105 | 5 / 调用 / 计算 / 写入 | <code>*dfn++ = '%';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 106 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 107 | 4 / case / default | <code>'h'</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 108 | 5 / 准备局部状态 | <code>char hostname[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 109 | 5 / 调用 / 计算 / 写入 | <code>getHostName(hostname, 1024, '.');</code> | 取得并按分隔符截短主机名，主要用于标识/日志。 |
| 110 | 5 / 调用 / 计算 / 写入 | <code>dfn += snprintf(dfn, PATH_MAX, "%s", hostname);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 111 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 112 | 4 / case / default | <code>'p'</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 113 | 5 / 调用 / 计算 / 写入 | <code>dfn += snprintf(dfn, PATH_MAX, "%d", getpid());</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 114 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 115 | 4 / case / default | <code>default</code> | 从匹配项进入，随后依源码直到break等跳转。 |
| 116 | 5 / 调用 / 计算 / 写入 | <code>*dfn++ = '%';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 117 | 5 / 调用 / 计算 / 写入 | <code>*dfn++ = ncclDebugFileEnv[c-1];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 118 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 119 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 120 | 2 / 调用 / 计算 / 写入 | <code>*dfn = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 121 | 2 / 条件判断 | <code>(debugFn[0] != '\0')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 122 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 123 | 4 / 准备局部状态 | <code>FILE *file = fopen(debugFn, "w");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 124 | 4 / 条件判断 | <code>(file != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 125 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 126 | 6 / 调用 / 计算 / 写入 | <code>ncclDebugFile = file;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 127 | 0 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 128 | 1 / 调用 / 计算 / 写入 | <code>ncclEpoch = std::chrono::high_resolution_clock::now();</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 129 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclDebugLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  pthread_mutex_lock(&ncclDebugLock);
  if (ncclDebugLevel != -1) { pthread_mutex_unlock(&ncclDebugLock); return; }
  const char* nccl_debug = getenv("NCCL_DEBUG");
  if (nccl_debug == NULL) {
    ncclDebugLevel = NCCL_LOG_NONE;
  } else if (strcasecmp(nccl_debug, "VERSION") == 0) {
    ncclDebugLevel = NCCL_LOG_VERSION;
  } else if (strcasecmp(nccl_debug, "WARN") == 0) {
    ncclDebugLevel = NCCL_LOG_WARN;
  } else if (strcasecmp(nccl_debug, "INFO") == 0) {
    ncclDebugLevel = NCCL_LOG_INFO;
  } else if (strcasecmp(nccl_debug, "ABORT") == 0) {
    ncclDebugLevel = NCCL_LOG_ABORT;
  } else if (strcasecmp(nccl_debug, "TRACE") == 0) {
    ncclDebugLevel = NCCL_LOG_TRACE;
  }

  /* Parse the NCCL_DEBUG_SUBSYS env var
   * This can be a comma separated list such as INIT,COLL
   * or ^INIT,COLL etc
   */
  char* ncclDebugSubsysEnv = getenv("NCCL_DEBUG_SUBSYS");
  if (ncclDebugSubsysEnv != NULL) {
    int invert = 0;
    if (ncclDebugSubsysEnv[0] == '^') { invert = 1; ncclDebugSubsysEnv++; }
    ncclDebugMask = invert ? ~0ULL : 0ULL;
    char *ncclDebugSubsys = strdup(ncclDebugSubsysEnv);
    char *subsys = strtok(ncclDebugSubsys, ",");
    while (subsys != NULL) {
      uint64_t mask = 0;
      if (strcasecmp(subsys, "INIT") == 0) {
        mask = NCCL_INIT;
      } else if (strcasecmp(subsys, "COLL") == 0) {
        mask = NCCL_COLL;
      } else if (strcasecmp(subsys, "P2P") == 0) {
        mask = NCCL_P2P;
      } else if (strcasecmp(subsys, "SHM") == 0) {
        mask = NCCL_SHM;
      } else if (strcasecmp(subsys, "NET") == 0) {
        mask = NCCL_NET;
      } else if (strcasecmp(subsys, "GRAPH") == 0) {
        mask = NCCL_GRAPH;
      } else if (strcasecmp(subsys, "TUNING") == 0) {
        mask = NCCL_TUNING;
      } else if (strcasecmp(subsys, "ENV") == 0) {
        mask = NCCL_ENV;
      } else if (strcasecmp(subsys, "ALLOC") == 0) {
        mask = NCCL_ALLOC;
      } else if (strcasecmp(subsys, "ALL") == 0) {
        mask = NCCL_ALL;
      }
      if (mask) {
        if (invert) ncclDebugMask &= ~mask; else ncclDebugMask |= mask;
      }
      subsys = strtok(NULL, ",");
    }
    free(ncclDebugSubsys);
  }

  /* Parse and expand the NCCL_DEBUG_FILE path and
   * then create the debug file. But don't bother unless the
   * NCCL_DEBUG level is > VERSION
   */
  const char* ncclDebugFileEnv = getenv("NCCL_DEBUG_FILE");
  if (ncclDebugLevel > NCCL_LOG_VERSION && ncclDebugFileEnv != NULL) {
    int c = 0;
    char debugFn[PATH_MAX+1] = "";
    char *dfn = debugFn;
    while (ncclDebugFileEnv[c] != '\0' && c < PATH_MAX) {
      if (ncclDebugFileEnv[c++] != '%') {
        *dfn++ = ncclDebugFileEnv[c-1];
        continue;
      }
      switch (ncclDebugFileEnv[c++]) {
        case '%': // Double %
          *dfn++ = '%';
          break;
        case 'h': // %h = hostname
          char hostname[1024];
          getHostName(hostname, 1024, '.');
          dfn += snprintf(dfn, PATH_MAX, "%s", hostname);
          break;
        case 'p': // %p = pid
          dfn += snprintf(dfn, PATH_MAX, "%d", getpid());
          break;
        default: // Echo everything we don't understand
          *dfn++ = '%';
          *dfn++ = ncclDebugFileEnv[c-1];
          break;
      }
    }
    *dfn = '\0';
    if (debugFn[0] != '\0') {
      FILE *file = fopen(debugFn, "w");
      if (file != NULL) {
        ncclDebugFile = file;
      }
    }
  }

#ifdef ENABLE_TRACE
  ncclEpoch = std::chrono::high_resolution_clock::now();
#endif
  pthread_mutex_unlock(&ncclDebugLock);
}
```

</details>

<a id="f0382"></a>

## F0382：ncclDebugLog

用途：按级别/子系统过滤并格式化日志，附加主机进程线程设备信息帮助追踪交错执行。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ncclDebugLevel == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>ncclDebugInit();</code> | 读取日志级别、子系统及输出路径配置，建立线程安全的日志输出状态。 |
| 4 | 0 / 条件判断 | <code>(ncclDebugNoWarn != 0 && level == NCCL_LOG_WARN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>level = NCCL_LOG_INFO;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>flags = ncclDebugNoWarn;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 条件判断 | <code>(ncclDebugLevel &lt; level &#124;&#124; ((flags & ncclDebugMask) == 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 准备局部状态 | <code>char hostname[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>getHostName(hostname, 1024, '.');</code> | 取得并按分隔符截短主机名，主要用于标识/日志。 |
| 13 | 0 / 准备局部状态 | <code>int cudaDev;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>cudaGetDevice(&cudaDev);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 0 / 准备局部状态 | <code>int pid = getpid();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 0 / 准备局部状态 | <code>int tid = syscall(SYS_gettid);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 0 / 准备局部状态 | <code>char buffer[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 0 / 准备局部状态 | <code>size_t len = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&ncclDebugLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 0 / 条件判断 | <code>(level == NCCL_LOG_WARN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>len = snprintf(buffer, sizeof(buffer), "\n%s:%d:%d [%d] %s:%d NCCL WARN ", hostname, pid, tid, cudaDev, filefunc, line);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 24 | 2 / 条件判断 | <code>(level == NCCL_LOG_INFO)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>len = snprintf(buffer, sizeof(buffer), "%s:%d:%d [%d] NCCL INFO ", hostname, pid, tid, cudaDev);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 28 | 1 / 准备局部状态 | <code>else if (level == NCCL_LOG_TRACE)</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 1 / 准备局部状态 | <code>auto delta = std::chrono::high_resolution_clock::now() - ncclEpoch;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 1 / 准备局部状态 | <code>double timestamp = std::chrono::duration_cast&lt;std::chrono::duration&lt;double&gt;&gt;(delta).count()*1000;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 1 / 调用 / 计算 / 写入 | <code>len = snprintf(buffer, sizeof(buffer), "%s:%d:%d [%d] %f %s:%d NCCL TRACE ", hostname, pid, tid, cudaDev, timestamp, filefunc, line);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 0 / 条件判断 | <code>(len)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 2 / 准备局部状态 | <code>va_list vargs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>va_start(vargs, fmt);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 2 / 调用 / 计算 / 写入 | <code>(void) vsnprintf(buffer+len, sizeof(buffer)-len, fmt, vargs);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 37 | 2 / 调用 / 计算 / 写入 | <code>va_end(vargs);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 2 / 调用 / 计算 / 写入 | <code>fprintf(ncclDebugFile,"%s\n", buffer);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 2 / 调用 / 计算 / 写入 | <code>fflush(ncclDebugFile);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclDebugLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ncclDebugLevel == -1) ncclDebugInit();
  if (ncclDebugNoWarn != 0 && level == NCCL_LOG_WARN) { level = NCCL_LOG_INFO; flags = ncclDebugNoWarn; }
  if (ncclDebugLevel < level || ((flags & ncclDebugMask) == 0)) return;

  // Gather the rank information. This can take > 1us so we want to make sure
  // we only do it when needed.
  char hostname[1024];
  getHostName(hostname, 1024, '.');
  int cudaDev;
  cudaGetDevice(&cudaDev);
  int pid = getpid();
  int tid = syscall(SYS_gettid);

  char buffer[1024];
  size_t len = 0;
  pthread_mutex_lock(&ncclDebugLock);
  if (level == NCCL_LOG_WARN)
    len = snprintf(buffer, sizeof(buffer),
        "\n%s:%d:%d [%d] %s:%d NCCL WARN ", hostname, pid, tid, cudaDev, filefunc, line);
  else if (level == NCCL_LOG_INFO)
    len = snprintf(buffer, sizeof(buffer),
        "%s:%d:%d [%d] NCCL INFO ", hostname, pid, tid, cudaDev);
#ifdef ENABLE_TRACE
  else if (level == NCCL_LOG_TRACE) {
    auto delta = std::chrono::high_resolution_clock::now() - ncclEpoch;
    double timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(delta).count()*1000;
    len = snprintf(buffer, sizeof(buffer),
        "%s:%d:%d [%d] %f %s:%d NCCL TRACE ", hostname, pid, tid, cudaDev, timestamp, filefunc, line);
  }
#endif
  if (len) {
    va_list vargs;
    va_start(vargs, fmt);
    (void) vsnprintf(buffer+len, sizeof(buffer)-len, fmt, vargs);
    va_end(vargs);
    fprintf(ncclDebugFile,"%s\n", buffer);
    fflush(ncclDebugFile);
  }
  pthread_mutex_unlock(&ncclDebugLock);
}
```

</details>

<a id="f0383"></a>

## F0383：ncclSetThreadName

用途：为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#ifdef _GNU_SOURCE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 条件判断 | <code>(ncclParamSetThreadName() != 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 3 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 1 / 准备局部状态 | <code>char threadName[NCCL_THREAD_NAMELEN];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 准备局部状态 | <code>va_list vargs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>va_start(vargs, fmt);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>vsnprintf(threadName, NCCL_THREAD_NAMELEN, fmt, vargs);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>va_end(vargs);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>pthread_setname_np(thread, threadName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // pthread_setname_np is nonstandard GNU extension
  // needs the following feature test macro
#ifdef _GNU_SOURCE
  if (ncclParamSetThreadName() != 1) return;
  char threadName[NCCL_THREAD_NAMELEN];
  va_list vargs;
  va_start(vargs, fmt);
  vsnprintf(threadName, NCCL_THREAD_NAMELEN, fmt, vargs);
  va_end(vargs);
  pthread_setname_np(thread, threadName);
#endif
}
```

</details>

