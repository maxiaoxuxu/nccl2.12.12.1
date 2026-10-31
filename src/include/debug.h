/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_DEBUG_H_
/* 中文逐项-常量 X0188：NCCL_DEBUG_H_
 * 编译期定义：NCCL_DEBUG_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_DEBUG_H_

#include "nccl_net.h"
#include <stdio.h>
#include <chrono>

#include <sys/syscall.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>

// Conform to pthread and NVTX standard
/* 中文逐项-常量 X0189：NCCL_THREAD_NAMELEN
 * 编译期定义：NCCL_THREAD_NAMELEN = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_THREAD_NAMELEN 16

/* 中文逐项-变量 V2115：ncclDebugLevel
 * ncclDebugLevel（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern int ncclDebugLevel;
/* 中文逐项-变量 V2116：ncclDebugMask
 * ncclDebugMask（uint64_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern uint64_t ncclDebugMask;
/* 中文逐项-变量 V2117：ncclDebugOutputLock
 * ncclDebugOutputLock（pthread_mutex_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern pthread_mutex_t ncclDebugOutputLock;
/* 中文逐项-变量 V2118：ncclDebugFile
 * ncclDebugFile（FILE；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern FILE *ncclDebugFile;
/* 中文逐项-接口 X0190：getHostName
 * 接口职责：取得并按分隔符截短主机名，主要用于标识/日志。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：extern ncclResult_t getHostName(char* hostname, int maxlen, const char delim);
 */
extern ncclResult_t getHostName(char* hostname, int maxlen, const char delim);

/* 中文逐项-接口 X0191：ncclDebugLog
 * 接口职责：按级别/子系统过滤并格式化日志，附加主机进程线程设备信息帮助追踪交错执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void ncclDebugLog(ncclDebugLogLevel level, unsigned long flags, const char
 * *filefunc, int line, const char *fmt, ...) __attribute__ ((format (printf, 5, 6)));
 */
void ncclDebugLog(ncclDebugLogLevel level, unsigned long flags, const char *filefunc, int line, const char *fmt, ...) __attribute__ ((format (printf, 5, 6)));

// Let code temporarily downgrade WARN into INFO
/* 中文逐项-变量 V2119：ncclDebugNoWarn
 * ncclDebugNoWarn（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern thread_local int ncclDebugNoWarn;

/* 中文逐项-宏 M0083：WARN
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 WARN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define WARN(...) ncclDebugLog(NCCL_LOG_WARN, NCCL_ALL, __FILE__, __LINE__, __VA_ARGS__)
/* 中文逐项-宏 M0084：INFO
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 INFO；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define INFO(FLAGS, ...) ncclDebugLog(NCCL_LOG_INFO, (FLAGS), __func__, __LINE__, __VA_ARGS__)

#ifdef ENABLE_TRACE
/* 中文逐项-宏 M0085：TRACE
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TRACE；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TRACE(FLAGS, ...) ncclDebugLog(NCCL_LOG_TRACE, (FLAGS), __func__, __LINE__, __VA_ARGS__)
/* 中文逐项-变量 V2120：ncclEpoch
 * ncclEpoch（std::chrono::high_resolution_clock::time_point；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern std::chrono::high_resolution_clock::time_point ncclEpoch;
#else
/* 中文逐项-宏 M0086：TRACE
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TRACE；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TRACE(...)
#endif

/* 中文逐项-接口 X0192：ncclSetThreadName
 * 接口职责：为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void ncclSetThreadName(pthread_t thread, const char *fmt, ...);
 */
void ncclSetThreadName(pthread_t thread, const char *fmt, ...);

#endif
