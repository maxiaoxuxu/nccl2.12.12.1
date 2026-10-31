/*
* Copyright 2009-2020  NVIDIA Corporation.  All rights reserved.
*
* Licensed under the Apache License v2.0 with LLVM Exceptions.
* See https://llvm.org/LICENSE.txt for license information.
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#ifndef NVTX_IMPL_GUARD
#error Never include this file directly -- it is automatically included by nvToolsExt.h (except when NVTX_NO_IMPL is defined).
#endif

/* ---- Platform-independent helper definitions and functions ---- */

/* Prefer macros over inline functions to reduce symbol resolution at link time */

#if defined(_WIN32) 
/* 中文逐项-常量 X0887：NVTX_PATHCHAR
 * 编译期定义：NVTX_PATHCHAR = wchar_t。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_PATHCHAR   wchar_t
/* 中文逐项-宏 M0105：NVTX_STR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_STR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_STR(x)     L##x
/* 中文逐项-常量 X0888：NVTX_GETENV
 * 编译期定义：NVTX_GETENV = _wgetenv。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_GETENV     _wgetenv
/* 中文逐项-常量 X0889：NVTX_BUFSIZE
 * 编译期定义：NVTX_BUFSIZE = MAX_PATH。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_BUFSIZE    MAX_PATH
/* 中文逐项-常量 X0890：NVTX_DLLHANDLE
 * 编译期定义：NVTX_DLLHANDLE = HMODULE。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_DLLHANDLE  HMODULE
/* 中文逐项-宏 M0106：NVTX_DLLOPEN
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_DLLOPEN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_DLLOPEN(x) LoadLibraryW(x)
/* 中文逐项-常量 X0891：NVTX_DLLFUNC
 * 编译期定义：NVTX_DLLFUNC = GetProcAddress。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_DLLFUNC    GetProcAddress
/* 中文逐项-常量 X0892：NVTX_DLLCLOSE
 * 编译期定义：NVTX_DLLCLOSE = FreeLibrary。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_DLLCLOSE   FreeLibrary
/* 中文逐项-宏 M0107：NVTX_YIELD
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_YIELD；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_YIELD()    SwitchToThread()
/* 中文逐项-宏 M0108：NVTX_MEMBAR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_MEMBAR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_MEMBAR()   MemoryBarrier()
/* 中文逐项-宏 M0109：NVTX_ATOMIC_WRITE_32
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_ATOMIC_WRITE_32；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_ATOMIC_WRITE_32(address, value)                        InterlockedExchange((volatile LONG*)address, value)
/* 中文逐项-宏 M0110：NVTX_ATOMIC_CAS_32
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_ATOMIC_CAS_32；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_ATOMIC_CAS_32(old, address, exchange, comparand) old = InterlockedCompareExchange((volatile LONG*)address, exchange, comparand)
#elif defined(__GNUC__)
/* 中文逐项-常量 X0893：NVTX_PATHCHAR
 * 编译期定义：NVTX_PATHCHAR = char。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_PATHCHAR   char
/* 中文逐项-宏 M0111：NVTX_STR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_STR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_STR(x)     x
/* 中文逐项-常量 X0894：NVTX_GETENV
 * 编译期定义：NVTX_GETENV = getenv。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_GETENV     getenv
/* 中文逐项-常量 X0895：NVTX_BUFSIZE
 * 编译期定义：NVTX_BUFSIZE = PATH_MAX。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_BUFSIZE    PATH_MAX
/* 中文逐项-常量 X0896：NVTX_DLLHANDLE
 * 编译期定义：NVTX_DLLHANDLE = void*。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_DLLHANDLE  void*
/* 中文逐项-宏 M0112：NVTX_DLLOPEN
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_DLLOPEN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_DLLOPEN(x) dlopen(x, RTLD_LAZY)
/* 中文逐项-常量 X0897：NVTX_DLLFUNC
 * 编译期定义：NVTX_DLLFUNC = dlsym。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_DLLFUNC    dlsym
/* 中文逐项-常量 X0898：NVTX_DLLCLOSE
 * 编译期定义：NVTX_DLLCLOSE = dlclose。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_DLLCLOSE   dlclose
/* 中文逐项-宏 M0113：NVTX_YIELD
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_YIELD；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_YIELD()    sched_yield()
/* 中文逐项-宏 M0114：NVTX_MEMBAR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_MEMBAR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_MEMBAR()   __sync_synchronize()
/* Ensure full memory barrier for atomics, to match Windows functions */
/* 中文逐项-宏 M0115：NVTX_ATOMIC_WRITE_32
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_ATOMIC_WRITE_32；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_ATOMIC_WRITE_32(address, value)                  __sync_synchronize();       __sync_lock_test_and_set(address, value)
/* 中文逐项-宏 M0116：NVTX_ATOMIC_CAS_32
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NVTX_ATOMIC_CAS_32；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVTX_ATOMIC_CAS_32(old, address, exchange, comparand) __sync_synchronize(); old = __sync_val_compare_and_swap(address, exchange, comparand)
#else
#error The library does not support your configuration!
#endif

/* Define this to 1 for platforms that where pre-injected libraries can be discovered. */
#if defined(_WIN32)
/* TODO */
/* 中文逐项-常量 X0899：NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY
 * 编译期定义：NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY 0
#else
/* 中文逐项-常量 X0900：NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY
 * 编译期定义：NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY 0
#endif

/* Define this to 1 for platforms that support environment variables */
/* TODO: Detect UWP, a.k.a. Windows Store app, and set this to 0. */
/* Try:  #if defined(WINAPI_FAMILY_PARTITION) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) */
/* 中文逐项-常量 X0901：NVTX_SUPPORT_ENV_VARS
 * 编译期定义：NVTX_SUPPORT_ENV_VARS = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_SUPPORT_ENV_VARS 1

/* Define this to 1 for platforms that support dynamic/shared libraries */
/* 中文逐项-常量 X0902：NVTX_SUPPORT_DYNAMIC_INJECTION_LIBRARY
 * 编译期定义：NVTX_SUPPORT_DYNAMIC_INJECTION_LIBRARY = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_SUPPORT_DYNAMIC_INJECTION_LIBRARY 1

/* Injection libraries implementing InitializeInjectionNvtx2 may be statically linked,
*  and this will override any dynamic injection.  Useful for platforms where dynamic
*  injection is not available.  Since weak symbols not explicitly marked extern are
*  guaranteed to be initialized to zero if no definitions are found by the linker, the
*  dynamic injection process proceeds normally if pfnInitializeInjectionNvtx2 is 0. */
#if defined(__GNUC__) && !defined(_WIN32) && !defined(__CYGWIN__)
/* 中文逐项-常量 X0903：NVTX_SUPPORT_STATIC_INJECTION_LIBRARY
 * 编译期定义：NVTX_SUPPORT_STATIC_INJECTION_LIBRARY = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_SUPPORT_STATIC_INJECTION_LIBRARY 1
/* To statically inject an NVTX library, define InitializeInjectionNvtx2_fnptr as a normal
*  symbol (not weak) pointing to the implementation of InitializeInjectionNvtx2 (which
*  does not need to be named "InitializeInjectionNvtx2" as is necessary in a dynamic
*  injection library. */
/* 中文逐项-变量 V2964：InitializeInjectionNvtx2_fnptr
 * InitializeInjectionNvtx2_fnptr（NvtxInitializeInjectionNvtxFunc_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (InitializeInjectionNvtx2_fnptr)。若由输出参数填入，必须先检查填充调用成功再读值。
 */
__attribute__((weak)) NvtxInitializeInjectionNvtxFunc_t InitializeInjectionNvtx2_fnptr;
#else
/* 中文逐项-常量 X0904：NVTX_SUPPORT_STATIC_INJECTION_LIBRARY
 * 编译期定义：NVTX_SUPPORT_STATIC_INJECTION_LIBRARY = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVTX_SUPPORT_STATIC_INJECTION_LIBRARY 0
#endif

/* This function tries to find or load an NVTX injection library and get the
*  address of its InitializeInjection2 function.  If such a function pointer
*  is found, it is called, and passed the address of this NVTX instance's
*  nvtxGetExportTable function, so the injection can attach to this instance.
*  If the initialization fails for any reason, any dynamic library loaded will
*  be freed, and all NVTX implementation functions will be set to no-ops.  If
*  initialization succeeds, NVTX functions not attached to the tool will be set
*  to no-ops.  This is implemented as one function instead of several small
*  functions to minimize the number of weak symbols the linker must resolve.
*  Order of search is:
*  - Pre-injected library exporting InitializeInjectionNvtx2
*  - Loadable library exporting InitializeInjectionNvtx2
*      - Path specified by env var NVTX_INJECTION??_PATH (?? is 32 or 64)
*      - On Android, libNvtxInjection??.so within the package (?? is 32 or 64)
*  - Statically-linked injection library defining InitializeInjectionNvtx2_fnptr
*/
NVTX_LINKONCE_FWDDECL_FUNCTION int NVTX_VERSIONED_IDENTIFIER(nvtxInitializeInjectionLibrary)(void);
NVTX_LINKONCE_DEFINE_FUNCTION /* 中文内部逻辑 F0742：nvtxInitializeInjectionLibrary
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const char* const initFuncName = "InitializeInjectionNvtx2";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：NvtxInitializeInjectionNvtxFunc_t init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：NVTX_DLLHANDLE injectionLibraryHandle = (NVTX_DLLHANDLE)0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int entryPointStatus = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 编译期分支/指令：#if NVTX_SUPPORT_ALREADY_INJECTED_LIBRARY；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   6. 调用 / 计算 / 写入：init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)NVTX_DLLFUNC(0, initFuncName);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 编译期分支/指令：#if NVTX_SUPPORT_DYNAMIC_INJECTION_LIBRARY；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   8. 条件判断：(!init_fnptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     9. 真分支：；仅条件为真执行以下子项。
 *       10. 编译期分支/指令：#if NVTX_SUPPORT_ENV_VARS；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *         11. 准备局部状态：const NVTX_PATHCHAR* const nvtxEnvVarName = (sizeof(void*) == 4) ? NVTX_STR("NVTX_INJECTION32_PATH") : NVTX_STR("NVTX_INJECTION64_PATH");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       12. 准备局部状态：NVTX_PATHCHAR injectionLibraryPathBuf[NVTX_BUFSIZE];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       13. 准备局部状态：const NVTX_PATHCHAR* injectionLibraryPath = (const NVTX_PATHCHAR*)0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       14. 调用 / 计算 / 写入：(void)injectionLibraryPathBuf;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *       15. 编译期分支/指令：#if NVTX_SUPPORT_ENV_VARS；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *         16. 编译期分支/指令：#if defined(_MSC_VER)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *         17. 调用 / 计算 / 写入：injectionLibraryPath = NVTX_GETENV(nvtxEnvVarName);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         18. 编译期分支/指令：#if defined(_MSC_VER)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       19. 编译期分支/指令：#if defined(__ANDROID__)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *         20. 条件判断：(!injectionLibraryPath)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           21. 真分支：；仅条件为真执行以下子项。
 *             22. 准备局部状态：const char *bits = (sizeof(void*) == 4) ? "32" : "64";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             23. 准备局部状态：char cmdlineBuf[32];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             24. 准备局部状态：char pkgName[PATH_MAX];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             25. 准备局部状态：int count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             26. 准备局部状态：int pid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             27. 准备局部状态：FILE *fp;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             28. 准备局部状态：size_t bytesRead;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             29. 准备局部状态：size_t pos;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             30. 调用 / 计算 / 写入：pid = (int)getpid();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             31. 调用 / 计算 / 写入：count = snprintf(cmdlineBuf, sizeof(cmdlineBuf), "/proc/%d/cmdline", pid);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             32. 条件判断：(count <= 0 || count >= (int)sizeof(cmdlineBuf))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               33. 真分支：；仅条件为真执行以下子项。
 *                 34. 调用 / 计算 / 写入：NVTX_ERR("Path buffer too small for: /proc/%d/cmdline\n", pid);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 35. 函数返回：return NVTX_ERR_INIT_ACCESS_LIBRARY;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *             36. 调用 / 计算 / 写入：fp = fopen(cmdlineBuf, "r");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             37. 条件判断：(!fp)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               38. 真分支：；仅条件为真执行以下子项。
 *                 39. 调用 / 计算 / 写入：NVTX_ERR("File couldn't be opened: %s\n", cmdlineBuf);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 40. 函数返回：return NVTX_ERR_INIT_ACCESS_LIBRARY;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *             41. 调用 / 计算 / 写入：bytesRead = fread(pkgName, 1, sizeof(pkgName) - 1, fp);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             42. 调用 / 计算 / 写入：fclose(fp);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             43. 条件判断：(bytesRead == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               44. 真分支：；仅条件为真执行以下子项。
 *                 45. 调用 / 计算 / 写入：NVTX_ERR("Package name couldn't be read from file: %s\n", cmdlineBuf);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 46. 函数返回：return NVTX_ERR_INIT_ACCESS_LIBRARY;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *             47. 调用 / 计算 / 写入：pkgName[bytesRead] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             48. 调用 / 计算 / 写入：pos = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             49. 循环：初始 ；继续条件 (pos < bytesRead && pkgName[pos] != ':' && pkgName[pos] != '\0')；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               50. 调用 / 计算 / 写入：++pos;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             51. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *             52. 调用 / 计算 / 写入：pkgName[pos] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             53. 调用 / 计算 / 写入：count = snprintf(injectionLibraryPathBuf, NVTX_BUFSIZE, "/data/data/%s/files/libNvtxInjection%s.so", pkgName, bits);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             54. 条件判断：(count <= 0 || count >= NVTX_BUFSIZE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               55. 真分支：；仅条件为真执行以下子项。
 *                 56. 调用 / 计算 / 写入：NVTX_ERR("Path buffer too small for: /data/data/%s/files/libNvtxInjection%s.so\n", pkgName, bits);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 57. 函数返回：return NVTX_ERR_INIT_ACCESS_LIBRARY;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *             58. 条件判断：(injectionLibraryPathBuf[0] == '/')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               59. 真分支：；仅条件为真执行以下子项。
 *                 60. 编译期分支/指令：#if (__ANDROID_API__ < 21)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *                   61. 准备局部状态：int access_err = access(injectionLibraryPathBuf, F_OK | R_OK);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                   62. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *                     63. 准备局部状态：int access_err = faccessat(AT_FDCWD, injectionLibraryPathBuf, F_OK | R_OK, 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 64. 条件判断：(access_err != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   65. 真分支：；仅条件为真执行以下子项。
 *                     66. 调用 / 计算 / 写入：NVTX_ERR("Injection library path wasn't accessible [code=%s] [path=%s]\n", strerror(errno), injectionLibraryPathBuf);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                     67. 函数返回：return NVTX_ERR_INIT_ACCESS_LIBRARY;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *             68. 调用 / 计算 / 写入：injectionLibraryPath = injectionLibraryPathBuf;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       69. 条件判断：(injectionLibraryPath)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         70. 真分支：；仅条件为真执行以下子项。
 *           71. 调用 / 计算 / 写入：injectionLibraryHandle = NVTX_DLLOPEN(injectionLibraryPath);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           72. 条件判断：(!injectionLibraryHandle)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             73. 真分支：；仅条件为真执行以下子项。
 *               74. 调用 / 计算 / 写入：NVTX_ERR("Failed to load injection library\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *               75. 函数返回：return NVTX_ERR_INIT_LOAD_LIBRARY;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *             76. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *               77. 调用 / 计算 / 写入：init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)NVTX_DLLFUNC(injectionLibraryHandle, initFuncName);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               78. 条件判断：(!init_fnptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 79. 真分支：；仅条件为真执行以下子项。
 *                   80. 调用 / 计算 / 写入：NVTX_DLLCLOSE(injectionLibraryHandle);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                   81. 调用 / 计算 / 写入：NVTX_ERR("Failed to get address of function InitializeInjectionNvtx2 from injection library\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                   82. 函数返回：return NVTX_ERR_INIT_MISSING_LIBRARY_ENTRY_POINT;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 83. 编译期分支/指令：#if NVTX_SUPPORT_STATIC_INJECTION_LIBRARY；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   84. 条件判断：(!init_fnptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     85. 真分支：；仅条件为真执行以下子项。
 *       86. 条件判断：(InitializeInjectionNvtx2_fnptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         87. 真分支：；仅条件为真执行以下子项。
 *           88. 调用 / 计算 / 写入：init_fnptr = InitializeInjectionNvtx2_fnptr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 89. 条件判断：(!init_fnptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   90. 真分支：；仅条件为真执行以下子项。
 *     91. 函数返回：return NVTX_ERR_NO_INJECTION_LIBRARY_AVAILABLE;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 92. 调用 / 计算 / 写入：entryPointStatus = init_fnptr(NVTX_VERSIONED_IDENTIFIER(nvtxGetExportTable));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 93. 条件判断：(entryPointStatus == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   94. 真分支：；仅条件为真执行以下子项。
 *     95. 调用 / 计算 / 写入：NVTX_ERR("Failed to initialize injection library -- initialization function returned 0\n");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     96. 条件判断：(injectionLibraryHandle)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       97. 真分支：；仅条件为真执行以下子项。
 *         98. 调用 / 计算 / 写入：NVTX_DLLCLOSE(injectionLibraryHandle);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     99. 函数返回：return NVTX_ERR_INIT_FAILED_LIBRARY_ENTRY_POINT;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 100. 函数返回：return NVTX_SUCCESS;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxInit_h.md#f0742
 */
/* 中文逐项-函数 F0742：nvtxInitializeInjectionLibrary
 * 用途与位置：查找并加载性能工具注入库，协商导出表并建立可用回调；没有工具时使用空操作。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：access；faccessat；fclose；fopen；fread；getpid；init_fnptr；snprintf；strerror。
 * 需要对照的边界/推进条件：!init_fnptr；!injectionLibraryPath；count <= 0 || count >= (int)sizeof(cmdlineBuf；!fp。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NVTX_ERR_INIT_ACCESS_LIBRARY;；return NVTX_ERR_INIT_LOAD_LIBRARY;；return
 * NVTX_ERR_INIT_MISSING_LIBRARY_ENTRY_POINT;；return NVTX_ERR_NO_INJECTION_LIBRARY_AVAILABLE;；return
 * NVTX_ERR_INIT_FAILED_LIBRARY_ENTRY_POINT;；其余错误/特化返回见函数体。CHECK类宏还可能提前返回/跳转。
 */int NVTX_VERSIONED_IDENTIFIER(nvtxInitializeInjectionLibrary)(void)
{
/* 中文逐项-变量 V2965：initFuncName
 * initFuncName（char；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 initFuncName = "InitializeInjectionNvtx2"；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    const char* const initFuncName = "InitializeInjectionNvtx2";
/* 中文逐项-变量 V2966：init_fnptr
 * init_fnptr（NvtxInitializeInjectionNvtxFunc_t；函数 nvtxInitializeInjectionLibrary
 * 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)0；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    NvtxInitializeInjectionNvtxFunc_t init_fnptr = (NvtxInitializeInjectionNvtxFunc_t)0;
/* 中文逐项-变量 V2967：injectionLibraryHandle
 * injectionLibraryHandle（NVTX_DLLHANDLE；函数 nvtxInitializeInjectionLibrary 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 injectionLibraryHandle = (NVTX_DLLHANDLE)0；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    NVTX_DLLHANDLE injectionLibraryHandle = (NVTX_DLLHANDLE)0;
/* 中文逐项-变量 V2968：entryPointStatus
 * entryPointStatus（int；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 entryPointStatus = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
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
/* 中文逐项-变量 V2969：nvtxEnvVarName
 * nvtxEnvVarName（NVTX_PATHCHAR；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvtxEnvVarName = (sizeof(void*) == 4) ? NVTX_STR("NVTX_INJECTION32_PATH") :
 * NVTX_STR("NVTX_INJECTION64_PATH")；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
        const NVTX_PATHCHAR* const nvtxEnvVarName = (sizeof(void*) == 4)
            ? NVTX_STR("NVTX_INJECTION32_PATH")
            : NVTX_STR("NVTX_INJECTION64_PATH");
#endif /* NVTX_SUPPORT_ENV_VARS */
/* 中文逐项-变量 V2970：injectionLibraryPathBuf
 * injectionLibraryPathBuf（NVTX_PATHCHAR；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：(void)injectionLibraryPathBuf;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        NVTX_PATHCHAR injectionLibraryPathBuf[NVTX_BUFSIZE];
/* 中文逐项-变量 V2971：injectionLibraryPath
 * injectionLibraryPath（NVTX_PATHCHAR；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 injectionLibraryPath = (const NVTX_PATHCHAR*)0；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
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
/* 中文逐项-变量 V2972：bits
 * bits（char；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bits = (sizeof(void*) == 4) ? "32" : "64"；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
            const char *bits = (sizeof(void*) == 4) ? "32" : "64";
/* 中文逐项-变量 V2973：cmdlineBuf
 * cmdlineBuf（char；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：count = snprintf(cmdlineBuf, sizeof(cmdlineBuf), "/proc/%d/cmdline",
 * pid);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            char cmdlineBuf[32];
/* 中文逐项-变量 V2974：pkgName
 * pkgName（char；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：bytesRead = fread(pkgName, 1, sizeof(pkgName) - 1, fp);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            char pkgName[PATH_MAX];
/* 中文逐项-变量 V2975：count
 * count（int；函数 nvtxInitializeInjectionLibrary 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 此处未显式给初值；后续首个文本使用点：count = snprintf(cmdlineBuf, sizeof(cmdlineBuf), "/proc/%d/cmdline",
 * pid);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            int count;
/* 中文逐项-变量 V2976：pid
 * pid（int；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：pid = (int)getpid();。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            int pid;
/* 中文逐项-变量 V2977：fp
 * fp（FILE；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：fp = fopen(cmdlineBuf, "r");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            FILE *fp;
/* 中文逐项-变量 V2978：bytesRead
 * bytesRead（size_t；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：bytesRead = fread(pkgName, 1, sizeof(pkgName) - 1, fp);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            size_t bytesRead;
/* 中文逐项-变量 V2979：pos
 * pos（size_t；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：pos = 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
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
/* 中文逐项-变量 V2980：access_err
 * access_err（int；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 access_err = access(injectionLibraryPathBuf, F_OK | R_OK)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
                int access_err = access(injectionLibraryPathBuf, F_OK | R_OK);
#else
/* 中文逐项-变量 V2981：access_err
 * access_err（int；函数 nvtxInitializeInjectionLibrary 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 access_err = faccessat(AT_FDCWD, injectionLibraryPathBuf, F_OK | R_OK, 0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
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

NVTX_LINKONCE_DEFINE_FUNCTION /* 中文内部逻辑 F0743：nvtxInitOnce
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：unsigned int old;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState == NVTX_INIT_STATE_COMPLETE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 调用 / 计算 / 写入：NVTX_ATOMIC_CAS_32( old, &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState, NVTX_INIT_STATE_STARTED, NVTX_INIT_STATE_FRESH);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 条件判断：(old == NVTX_INIT_STATE_FRESH)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 准备局部状态：int result;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 准备局部状态：int forceAllToNoops;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     10. 调用 / 计算 / 写入：result = NVTX_VERSIONED_IDENTIFIER(nvtxInitializeInjectionLibrary)();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 调用 / 计算 / 写入：forceAllToNoops = result != NVTX_SUCCESS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     12. 调用 / 计算 / 写入：NVTX_VERSIONED_IDENTIFIER(nvtxSetInitFunctionsToNoops)(forceAllToNoops);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     13. 调用 / 计算 / 写入：NVTX_ATOMIC_WRITE_32( &NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState, NVTX_INIT_STATE_COMPLETE);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   14. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     15. 调用 / 计算 / 写入：NVTX_MEMBAR();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     16. 循环：初始 ；继续条件 (NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState != NVTX_INIT_STATE_COMPLETE)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       17. 调用 / 计算 / 写入：NVTX_YIELD();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       18. 调用 / 计算 / 写入：NVTX_MEMBAR();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     19. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__nvtx3__nvtxDetail__nvtxInit_h.md#f0743
 */
/* 中文逐项-函数 F0743：nvtxInitOnce
 * 用途与位置：以一次性初始化状态和原子/锁协调工具注入，初始化后让标记API走真实工具回调或no-op。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState == NVTX_INIT_STATE_COMPLETE；old ==
 * NVTX_INIT_STATE_FRESH；NVTX_VERSIONED_IDENTIFIER(nvtxGlobals).initState != NVTX_INIT_STATE_COMPLETE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 */void NVTX_VERSIONED_IDENTIFIER(nvtxInitOnce)(void)
{
/* 中文逐项-变量 V2982：old
 * old（unsigned int；函数 nvtxInitOnce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：old,。若由输出参数填入，必须先检查填充调用成功再读值。
 */
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
/* 中文逐项-变量 V2983：result
 * result（int；函数 nvtxInitOnce 内）：异步线程/请求保存的结果码，前台测试函数需要检查它而不只检查长度。
 * 此处未显式给初值；后续首个文本使用点：result = NVTX_VERSIONED_IDENTIFIER(nvtxInitializeInjectionLibrary)();。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int result;
/* 中文逐项-变量 V2984：forceAllToNoops
 * forceAllToNoops（int；函数 nvtxInitOnce 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：forceAllToNoops = result != NVTX_SUCCESS;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
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
