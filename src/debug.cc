/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "core.h"
#include "nccl_net.h"
#include <stdlib.h>
#include <stdarg.h>

/* 中文逐项-变量 V0858：ncclDebugLevel
 * ncclDebugLevel（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclDebugLevel = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
int ncclDebugLevel = -1;
/* 中文逐项-变量 V0859：ncclDebugNoWarn
 * ncclDebugNoWarn（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclDebugNoWarn = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
thread_local int ncclDebugNoWarn = 0;
/* 中文逐项-变量 V0860：ncclDebugMask
 * ncclDebugMask（uint64_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclDebugMask = NCCL_INIT；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
uint64_t ncclDebugMask = NCCL_INIT; // Default debug sub-system mask is INIT
/* 中文逐项-变量 V0861：ncclDebugFile
 * ncclDebugFile（FILE；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclDebugFile = stdout；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
FILE *ncclDebugFile = stdout;
/* 中文逐项-变量 V0862：ncclDebugLock
 * ncclDebugLock（pthread_mutex_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclDebugLock = PTHREAD_MUTEX_INITIALIZER；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
pthread_mutex_t ncclDebugLock = PTHREAD_MUTEX_INITIALIZER;

/* 中文精读：日志开关如何帮助对应源码阶段
 * NCCL_DEBUG 控制级别，NCCL_DEBUG_SUBSYS 选择 INIT/COLL/NET/GRAPH 等子系统。
 * 同一个 INFO 日志调用带 subsystem 位掩码，过滤后才格式化输出，减少不必要开销。
 * 日志含主机、进程、线程、设备等信息；多 rank 输出会交错，应按这些标识追踪同一路径。
 * 初始化日志的 channel 数是配置结果，实际某次操作并行度还需结合调度代码与运行信息。
 */
/* 中文内部逻辑 F0381：ncclDebugInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：pthread_mutex_lock(&ncclDebugLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 条件判断：(ncclDebugLevel != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：pthread_mutex_unlock(&ncclDebugLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 准备局部状态：const char* nccl_debug = getenv("NCCL_DEBUG");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 条件判断：(nccl_debug == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：ncclDebugLevel = NCCL_LOG_NONE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   10. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     11. 条件判断：(strcasecmp(nccl_debug, "VERSION") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       12. 真分支：；仅条件为真执行以下子项。
 *         13. 调用 / 计算 / 写入：ncclDebugLevel = NCCL_LOG_VERSION;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       14. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         15. 条件判断：(strcasecmp(nccl_debug, "WARN") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           16. 真分支：；仅条件为真执行以下子项。
 *             17. 调用 / 计算 / 写入：ncclDebugLevel = NCCL_LOG_WARN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           18. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             19. 条件判断：(strcasecmp(nccl_debug, "INFO") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               20. 真分支：；仅条件为真执行以下子项。
 *                 21. 调用 / 计算 / 写入：ncclDebugLevel = NCCL_LOG_INFO;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               22. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                 23. 条件判断：(strcasecmp(nccl_debug, "ABORT") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   24. 真分支：；仅条件为真执行以下子项。
 *                     25. 调用 / 计算 / 写入：ncclDebugLevel = NCCL_LOG_ABORT;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   26. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                     27. 条件判断：(strcasecmp(nccl_debug, "TRACE") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                       28. 真分支：；仅条件为真执行以下子项。
 *                         29. 调用 / 计算 / 写入：ncclDebugLevel = NCCL_LOG_TRACE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 准备局部状态：char* ncclDebugSubsysEnv = getenv("NCCL_DEBUG_SUBSYS");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 31. 条件判断：(ncclDebugSubsysEnv != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   32. 真分支：；仅条件为真执行以下子项。
 *     33. 准备局部状态：int invert = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     34. 条件判断：(ncclDebugSubsysEnv[0] == '^')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       35. 真分支：；仅条件为真执行以下子项。
 *         36. 调用 / 计算 / 写入：invert = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         37. 调用 / 计算 / 写入：ncclDebugSubsysEnv++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     38. 调用 / 计算 / 写入：ncclDebugMask = invert ? ~0ULL : 0ULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     39. 准备局部状态：char *ncclDebugSubsys = strdup(ncclDebugSubsysEnv);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     40. 准备局部状态：char *subsys = strtok(ncclDebugSubsys, ",");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     41. 循环：初始 ；继续条件 (subsys != NULL)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       42. 准备局部状态：uint64_t mask = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       43. 条件判断：(strcasecmp(subsys, "INIT") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         44. 真分支：；仅条件为真执行以下子项。
 *           45. 调用 / 计算 / 写入：mask = NCCL_INIT;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         46. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           47. 条件判断：(strcasecmp(subsys, "COLL") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             48. 真分支：；仅条件为真执行以下子项。
 *               49. 调用 / 计算 / 写入：mask = NCCL_COLL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             50. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *               51. 条件判断：(strcasecmp(subsys, "P2P") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 52. 真分支：；仅条件为真执行以下子项。
 *                   53. 调用 / 计算 / 写入：mask = NCCL_P2P;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 54. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   55. 条件判断：(strcasecmp(subsys, "SHM") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     56. 真分支：；仅条件为真执行以下子项。
 *                       57. 调用 / 计算 / 写入：mask = NCCL_SHM;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                     58. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                       59. 条件判断：(strcasecmp(subsys, "NET") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                         60. 真分支：；仅条件为真执行以下子项。
 *                           61. 调用 / 计算 / 写入：mask = NCCL_NET;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                         62. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                           63. 条件判断：(strcasecmp(subsys, "GRAPH") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                             64. 真分支：；仅条件为真执行以下子项。
 *                               65. 调用 / 计算 / 写入：mask = NCCL_GRAPH;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                             66. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                               67. 条件判断：(strcasecmp(subsys, "TUNING") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                                 68. 真分支：；仅条件为真执行以下子项。
 *                                   69. 调用 / 计算 / 写入：mask = NCCL_TUNING;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                                 70. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                                   71. 条件判断：(strcasecmp(subsys, "ENV") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                                     72. 真分支：；仅条件为真执行以下子项。
 *                                       73. 调用 / 计算 / 写入：mask = NCCL_ENV;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                                     74. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                                       75. 条件判断：(strcasecmp(subsys, "ALLOC") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                                         76. 真分支：；仅条件为真执行以下子项。
 *                                           77. 调用 / 计算 / 写入：mask = NCCL_ALLOC;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                                         78. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                                           79. 条件判断：(strcasecmp(subsys, "ALL") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                                             80. 真分支：；仅条件为真执行以下子项。
 *                                               81. 调用 / 计算 / 写入：mask = NCCL_ALL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       82. 条件判断：(mask)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         83. 真分支：；仅条件为真执行以下子项。
 *           84. 条件判断：(invert)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             85. 真分支：；仅条件为真执行以下子项。
 *               86. 调用 / 计算 / 写入：ncclDebugMask &= ~mask;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             87. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *               88. 调用 / 计算 / 写入：ncclDebugMask |= mask;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       89. 调用 / 计算 / 写入：subsys = strtok(NULL, ",");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     90. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     91. 调用 / 计算 / 写入：free(ncclDebugSubsys);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 92. 准备局部状态：const char* ncclDebugFileEnv = getenv("NCCL_DEBUG_FILE");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 93. 条件判断：(ncclDebugLevel > NCCL_LOG_VERSION && ncclDebugFileEnv != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   94. 真分支：；仅条件为真执行以下子项。
 *     95. 准备局部状态：int c = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     96. 准备局部状态：char debugFn[PATH_MAX+1] = "";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     97. 准备局部状态：char *dfn = debugFn;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     98. 循环：初始 ；继续条件 (ncclDebugFileEnv[c] != '\0' && c < PATH_MAX)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       99. 条件判断：(ncclDebugFileEnv[c++] != '%')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         100. 真分支：；仅条件为真执行以下子项。
 *           101. 调用 / 计算 / 写入：*dfn++ = ncclDebugFileEnv[c-1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           102. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       103. 多路分派：(ncclDebugFileEnv[c++])；按case匹配进入；没有break/return/goto时可能贯穿下一个case。
 *         104. case / default：'%'；从匹配项进入，随后依源码直到break等跳转。
 *           105. 调用 / 计算 / 写入：*dfn++ = '%';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           106. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         107. case / default：'h'；从匹配项进入，随后依源码直到break等跳转。
 *           108. 准备局部状态：char hostname[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           109. 调用 / 计算 / 写入：getHostName(hostname, 1024, '.');；取得并按分隔符截短主机名，主要用于标识/日志。
 *           110. 调用 / 计算 / 写入：dfn += snprintf(dfn, PATH_MAX, "%s", hostname);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           111. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         112. case / default：'p'；从匹配项进入，随后依源码直到break等跳转。
 *           113. 调用 / 计算 / 写入：dfn += snprintf(dfn, PATH_MAX, "%d", getpid());；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           114. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         115. case / default：default；从匹配项进入，随后依源码直到break等跳转。
 *           116. 调用 / 计算 / 写入：*dfn++ = '%';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           117. 调用 / 计算 / 写入：*dfn++ = ncclDebugFileEnv[c-1];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           118. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *     119. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     120. 调用 / 计算 / 写入：*dfn = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     121. 条件判断：(debugFn[0] != '\0')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       122. 真分支：；仅条件为真执行以下子项。
 *         123. 准备局部状态：FILE *file = fopen(debugFn, "w");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         124. 条件判断：(file != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           125. 真分支：；仅条件为真执行以下子项。
 *             126. 调用 / 计算 / 写入：ncclDebugFile = file;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 127. 编译期分支/指令：#ifdef ENABLE_TRACE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   128. 调用 / 计算 / 写入：ncclEpoch = std::chrono::high_resolution_clock::now();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 129. 调用 / 计算 / 写入：pthread_mutex_unlock(&ncclDebugLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__debug_cc.md#f0381
 */
/* 中文逐项-函数 F0381：ncclDebugInit
 * 用途与位置：读取日志级别、子系统及输出路径配置，建立线程安全的日志输出状态。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/debug.cc::ncclDebugLog。
 * 本体直接出现的调用/包装：fopen；free；getHostName；getenv；getpid；pthread_mutex_lock；pthread_mutex_unlock；snprintf；std::chrono::high_resolution_clock::now；strcasecmp；strdup；strtok。
 * 需要对照的边界/推进条件：ncclDebugLevel != -1；nccl_debug == NULL；strcasecmp(nccl_debug, "VERSION") == 0；strcasecmp(nccl_debug,
 * "WARN") == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
void ncclDebugInit() {
  pthread_mutex_lock(&ncclDebugLock);
  if (ncclDebugLevel != -1) { pthread_mutex_unlock(&ncclDebugLock); return; }
/* 中文逐项-变量 V0863：nccl_debug
 * nccl_debug（char；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nccl_debug = getenv("NCCL_DEBUG")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
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
/* 中文逐项-变量 V0864：ncclDebugSubsysEnv
 * ncclDebugSubsysEnv（char；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclDebugSubsysEnv = getenv("NCCL_DEBUG_SUBSYS")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* ncclDebugSubsysEnv = getenv("NCCL_DEBUG_SUBSYS");
  if (ncclDebugSubsysEnv != NULL) {
/* 中文逐项-变量 V0865：invert
 * invert（int；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 invert = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int invert = 0;
    if (ncclDebugSubsysEnv[0] == '^') { invert = 1; ncclDebugSubsysEnv++; }
    ncclDebugMask = invert ? ~0ULL : 0ULL;
/* 中文逐项-变量 V0866：ncclDebugSubsys
 * ncclDebugSubsys（char；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclDebugSubsys = strdup(ncclDebugSubsysEnv)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char *ncclDebugSubsys = strdup(ncclDebugSubsysEnv);
/* 中文逐项-变量 V0867：subsys
 * subsys（char；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 subsys = strtok(ncclDebugSubsys, ",")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char *subsys = strtok(ncclDebugSubsys, ",");
    while (subsys != NULL) {
/* 中文逐项-变量 V0868：mask
 * mask（uint64_t；函数 ncclDebugInit 内）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 * 初值/来源 mask = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
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
/* 中文逐项-变量 V0869：ncclDebugFileEnv
 * ncclDebugFileEnv（char；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclDebugFileEnv = getenv("NCCL_DEBUG_FILE")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const char* ncclDebugFileEnv = getenv("NCCL_DEBUG_FILE");
  if (ncclDebugLevel > NCCL_LOG_VERSION && ncclDebugFileEnv != NULL) {
/* 中文逐项-变量 V0870：c
 * c（int；函数 ncclDebugInit 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int c = 0;
/* 中文逐项-变量 V0871：debugFn
 * debugFn（char；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 debugFn = ""；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char debugFn[PATH_MAX+1] = "";
/* 中文逐项-变量 V0872：dfn
 * dfn（char；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 dfn = debugFn；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
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
/* 中文逐项-变量 V0873：hostname
 * hostname（char；函数 ncclDebugInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：getHostName(hostname, 1024, '.');。若由输出参数填入，必须先检查填充调用成功再读值。
 */
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
/* 中文逐项-变量 V0874：file
 * file（FILE；函数 ncclDebugInit 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 初值/来源 file = fopen(debugFn, "w")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
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

/* Common logging function used by the INFO, WARN and TRACE macros
 * Also exported to the dynamically loadable Net transport modules so
 * they can share the debugging mechanisms and output files
 */
/* 中文内部逻辑 F0382：ncclDebugLog
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(ncclDebugLevel == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：ncclDebugInit();；读取日志级别、子系统及输出路径配置，建立线程安全的日志输出状态。
 * 4. 条件判断：(ncclDebugNoWarn != 0 && level == NCCL_LOG_WARN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：level = NCCL_LOG_INFO;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：flags = ncclDebugNoWarn;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 条件判断：(ncclDebugLevel < level || ((flags & ncclDebugMask) == 0))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 11. 准备局部状态：char hostname[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 调用 / 计算 / 写入：getHostName(hostname, 1024, '.');；取得并按分隔符截短主机名，主要用于标识/日志。
 * 13. 准备局部状态：int cudaDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 调用 / 计算 / 写入：cudaGetDevice(&cudaDev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 15. 准备局部状态：int pid = getpid();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 16. 准备局部状态：int tid = syscall(SYS_gettid);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 17. 准备局部状态：char buffer[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 18. 准备局部状态：size_t len = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 19. 调用 / 计算 / 写入：pthread_mutex_lock(&ncclDebugLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 20. 条件判断：(level == NCCL_LOG_WARN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   21. 真分支：；仅条件为真执行以下子项。
 *     22. 调用 / 计算 / 写入：len = snprintf(buffer, sizeof(buffer), "\n%s:%d:%d [%d] %s:%d NCCL WARN ", hostname, pid, tid, cudaDev, filefunc, line);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   23. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     24. 条件判断：(level == NCCL_LOG_INFO)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       25. 真分支：；仅条件为真执行以下子项。
 *         26. 调用 / 计算 / 写入：len = snprintf(buffer, sizeof(buffer), "%s:%d:%d [%d] NCCL INFO ", hostname, pid, tid, cudaDev);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 27. 编译期分支/指令：#ifdef ENABLE_TRACE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   28. 准备局部状态：else if (level == NCCL_LOG_TRACE)；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   29. 准备局部状态：auto delta = std::chrono::high_resolution_clock::now() - ncclEpoch;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   30. 准备局部状态：double timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(delta).count()*1000;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   31. 调用 / 计算 / 写入：len = snprintf(buffer, sizeof(buffer), "%s:%d:%d [%d] %f %s:%d NCCL TRACE ", hostname, pid, tid, cudaDev, timestamp, filefunc, line);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 条件判断：(len)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   33. 真分支：；仅条件为真执行以下子项。
 *     34. 准备局部状态：va_list vargs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     35. 调用 / 计算 / 写入：va_start(vargs, fmt);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     36. 调用 / 计算 / 写入：(void) vsnprintf(buffer+len, sizeof(buffer)-len, fmt, vargs);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     37. 调用 / 计算 / 写入：va_end(vargs);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     38. 调用 / 计算 / 写入：fprintf(ncclDebugFile,"%s\n", buffer);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     39. 调用 / 计算 / 写入：fflush(ncclDebugFile);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 40. 调用 / 计算 / 写入：pthread_mutex_unlock(&ncclDebugLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__debug_cc.md#f0382
 */
/* 中文逐项-函数 F0382：ncclDebugLog
 * 用途与位置：按级别/子系统过滤并格式化日志，附加主机进程线程设备信息帮助追踪交错执行。
 * 参数 level（ncclDebugLogLevel level）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 flags（unsigned long flags）：按位组合的当前线程角色/直接访问/共享偏移/中止等状态，按位测试而非单一枚举。
 * 参数 filefunc（const char *filefunc）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 line（int line）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 fmt（const char *fmt）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：cudaGetDevice；fflush；fprintf；getHostName；getpid；ncclDebugInit；pthread_mutex_lock；pthread_mutex_unlock；snprintf；std::chrono::duration_cast<std::chrono::duration<double>>；std::chrono::duration_cast<std::chrono::duration<double>>(delta).count；std::chrono::high_resolution_clock::now；syscall；va_end；va_start；vsnprintf。
 * 需要对照的边界/推进条件：ncclDebugLevel == -1；ncclDebugNoWarn != 0 && level == NCCL_LOG_WARN；ncclDebugLevel < level || ((flags
 * & ncclDebugMask) == 0；level == NCCL_LOG_WARN。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
void ncclDebugLog(ncclDebugLogLevel level, unsigned long flags, const char *filefunc, int line, const char *fmt, ...) {
  if (ncclDebugLevel == -1) ncclDebugInit();
  if (ncclDebugNoWarn != 0 && level == NCCL_LOG_WARN) { level = NCCL_LOG_INFO; flags = ncclDebugNoWarn; }
  if (ncclDebugLevel < level || ((flags & ncclDebugMask) == 0)) return;

  // Gather the rank information. This can take > 1us so we want to make sure
  // we only do it when needed.
/* 中文逐项-变量 V0875：hostname
 * hostname（char；函数 ncclDebugLog 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：getHostName(hostname, 1024, '.');。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char hostname[1024];
  getHostName(hostname, 1024, '.');
/* 中文逐项-变量 V0876：cudaDev
 * cudaDev（int；函数 ncclDebugLog 内）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 此处未显式给初值；后续首个文本使用点：cudaGetDevice(&cudaDev);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int cudaDev;
  cudaGetDevice(&cudaDev);
/* 中文逐项-变量 V0877：pid
 * pid（int；函数 ncclDebugLog 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pid = getpid()；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int pid = getpid();
/* 中文逐项-变量 V0878：tid
 * tid（int；函数 ncclDebugLog 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = syscall(SYS_gettid)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int tid = syscall(SYS_gettid);

/* 中文逐项-变量 V0879：buffer
 * buffer（char；函数 ncclDebugLog 内）：握手暂存或数据区，须按所在状态机阶段解释其内容和长度。
 * 此处未显式给初值；后续首个文本使用点：len = snprintf(buffer, sizeof(buffer),。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char buffer[1024];
/* 中文逐项-变量 V0880：len
 * len（size_t；函数 ncclDebugLog 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 len = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  size_t len = 0;
  pthread_mutex_lock(&ncclDebugLock);
  if (level == NCCL_LOG_WARN)
    len = snprintf(buffer, sizeof(buffer),
        "\n%s:%d:%d [%d] %s:%d NCCL WARN ", hostname, pid, tid, cudaDev, filefunc, line);
  else if (level == NCCL_LOG_INFO)
    len = snprintf(buffer, sizeof(buffer),
        "%s:%d:%d [%d] NCCL INFO ", hostname, pid, tid, cudaDev);
#ifdef ENABLE_TRACE
/* 中文逐项-变量 V0881：if
 * if（else；函数 ncclDebugLog 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 if = (level == NCCL_LOG_TRACE)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  else if (level == NCCL_LOG_TRACE) {
/* 中文逐项-变量 V0882：delta
 * delta（auto；函数 ncclDebugLog 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 delta = std::chrono::high_resolution_clock::now() - ncclEpoch；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    auto delta = std::chrono::high_resolution_clock::now() - ncclEpoch;
/* 中文逐项-变量 V0883：timestamp
 * timestamp（double；函数 ncclDebugLog 内）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 初值/来源 timestamp =
 * std::chrono::duration_cast<std::chrono::duration<double>>(delta).count()*1000；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
    double timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(delta).count()*1000;
    len = snprintf(buffer, sizeof(buffer),
        "%s:%d:%d [%d] %f %s:%d NCCL TRACE ", hostname, pid, tid, cudaDev, timestamp, filefunc, line);
  }
#endif
  if (len) {
/* 中文逐项-变量 V0884：vargs
 * vargs（va_list；函数 ncclDebugLog 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：va_start(vargs, fmt);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    va_list vargs;
    va_start(vargs, fmt);
    (void) vsnprintf(buffer+len, sizeof(buffer)-len, fmt, vargs);
    va_end(vargs);
    fprintf(ncclDebugFile,"%s\n", buffer);
    fflush(ncclDebugFile);
  }
  pthread_mutex_unlock(&ncclDebugLock);
}

/* 中文逐项-参数getter P0002：SetThreadName
 * 生成函数 ncclParamSetThreadName()，读取 NCCL_SET_THREAD_NAME，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(SetThreadName, "SET_THREAD_NAME", 0);

/* 中文内部逻辑 F0383：ncclSetThreadName
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 编译期分支/指令：#ifdef _GNU_SOURCE；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   2. 条件判断：(ncclParamSetThreadName() != 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     3. 真分支：；仅条件为真执行以下子项。
 *       4. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   5. 准备局部状态：char threadName[NCCL_THREAD_NAMELEN];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   6. 准备局部状态：va_list vargs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   7. 调用 / 计算 / 写入：va_start(vargs, fmt);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   8. 调用 / 计算 / 写入：vsnprintf(threadName, NCCL_THREAD_NAMELEN, fmt, vargs);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   9. 调用 / 计算 / 写入：va_end(vargs);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   10. 调用 / 计算 / 写入：pthread_setname_np(thread, threadName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__debug_cc.md#f0383
 */
/* 中文逐项-函数 F0383：ncclSetThreadName
 * 用途与位置：为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。
 * 参数 thread（pthread_t thread）：后台线程句柄和其任务/停止状态，资源回收前需要停止并join。
 * 参数 fmt（const char *fmt）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapCreateRoot；src/enqueue.cc::ncclGetCudaGraph；src/proxy.cc::ncclProxyProgressCreate；src/proxy.cc::ncclProxyCreate；src/transport/net_ib.cc::ncclIbInit；src/transport/net_socket.cc::ncclSocketGetTask。
 * 本体直接出现的调用/包装：ncclParamSetThreadName；pthread_setname_np；va_end；va_start；vsnprintf。
 * 需要对照的边界/推进条件：ncclParamSetThreadName() != 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 */
void ncclSetThreadName(pthread_t thread, const char *fmt, ...) {
  // pthread_setname_np is nonstandard GNU extension
  // needs the following feature test macro
#ifdef _GNU_SOURCE
  if (ncclParamSetThreadName() != 1) return;
/* 中文逐项-变量 V0885：threadName
 * threadName（char；函数 ncclSetThreadName 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：vsnprintf(threadName, NCCL_THREAD_NAMELEN, fmt, vargs);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char threadName[NCCL_THREAD_NAMELEN];
/* 中文逐项-变量 V0886：vargs
 * vargs（va_list；函数 ncclSetThreadName 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：va_start(vargs, fmt);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  va_list vargs;
  va_start(vargs, fmt);
  vsnprintf(threadName, NCCL_THREAD_NAMELEN, fmt, vargs);
  va_end(vargs);
  pthread_setname_np(thread, threadName);
#endif
}
