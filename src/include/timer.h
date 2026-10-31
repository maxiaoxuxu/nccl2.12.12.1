/*************************************************************************
 * Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_TIMER_H_
/* 中文逐项-常量 X1123：NCCL_TIMER_H_
 * 编译期定义：NCCL_TIMER_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TIMER_H_
#if ENABLE_TIMER
#include <unistd.h>
#include <sys/time.h>
#include <x86intrin.h>
/* 中文逐项-变量 V3167：freq
 * freq（double；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 freq = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
static double freq = -1;
/* 中文逐项-函数 F0870：calibrate
 * 用途与位置：估计 CPU 计数器与时间单位的换算系数，服务轻量性能时间戳。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/timer.h::gettime。
 * 本体直接出现的调用/包装：__rdtsc；gettimeofday。
 * 需要对照的边界/推进条件：i<10000。各条件所属分支见紧邻函数体。
 */
static void calibrate() {
/* 中文逐项-变量 V3168：tv
 * tv（struct timeval；函数 calibrate 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：gettimeofday(&tv, NULL);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct timeval tv;
  gettimeofday(&tv, NULL);
/* 中文逐项-变量 V3169：timeCycles
 * timeCycles（uint64_t；函数 calibrate 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 timeCycles = __rdtsc()；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  uint64_t timeCycles = __rdtsc();
/* 中文逐项-变量 V3170：time
 * time（double；函数 calibrate 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 time = - tv.tv_sec*1E6 - tv.tv_usec；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  double time = - tv.tv_sec*1E6 - tv.tv_usec;
/* 中文逐项-变量 V3171：total
 * total（uint64_t；函数 calibrate 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 total = 0ULL；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  uint64_t total = 0ULL;
  for (/* 中文逐项-变量 V3172：i
 * i（int；函数 calibrate 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<10000；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<10000; i++) total += __rdtsc();
  gettimeofday(&tv, NULL);
  timeCycles = __rdtsc() - timeCycles;
  time += tv.tv_sec*1E6 + tv.tv_usec;
  freq = timeCycles/time;
}
/* 中文逐项-函数 F0871：gettime
 * 用途与位置：读取本平台的轻量时间源/计数器并转成 profiling 需要的时间表示。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/profiler.cc::ncclProfilingRecord。
 * 本体直接出现的调用/包装：__rdtsc；calibrate。
 * 需要对照的边界/推进条件：freq == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return __rdtsc()/freq;。CHECK类宏还可能提前返回/跳转。
 */
static inline double gettime() {
  if (freq == -1) calibrate();
  return __rdtsc()/freq;
}
/* 中文逐项-变量 V3173：counts
 * counts（uint64_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：counts[index]++; \。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static uint64_t counts[8];
/* 中文逐项-变量 V3174：times
 * times（double；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：times[index] += gettime() - startTimes[index]; \。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static double times[8];
/* 中文逐项-变量 V3175：startTimes
 * startTimes（double；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：startTimes[index] = gettime(); \。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static double startTimes[8];
/* 中文逐项-宏 M0123：TIME_START
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TIME_START；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TIME_START(index) do { \
  counts[index]++; \
  startTimes[index] = gettime(); \
} while (0);

/* 中文逐项-宏 M0124：TIME_STOP
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TIME_STOP；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TIME_STOP(index) do { \
  times[index] += gettime() - startTimes[index]; \
} while (0);

/* 中文逐项-宏 M0125：TIME_CANCEL
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TIME_CANCEL；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TIME_CANCEL(index) do { \
  counts[index]--; \
} while (0);

/* 中文逐项-宏 M0126：TIME_PRINT
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TIME_PRINT；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TIME_PRINT(name) do { \
  printf("%s stats", name); \
  for (int i=0; i<8; i++) { \
    if (counts[i]) printf(" [%d] %g/%ld = %g", i, times[i], counts[i], times[i]/counts[i]); \
    counts[i] = 0; \
  } \
  printf("\n"); \
} while (0);
#else
/* 中文逐项-宏 M0127：TIME_START
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TIME_START；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TIME_START(index) while(0);
/* 中文逐项-宏 M0128：TIME_STOP
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TIME_STOP；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TIME_STOP(index) while(0);
/* 中文逐项-宏 M0129：TIME_CANCEL
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TIME_CANCEL；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TIME_CANCEL(index) while(0);
/* 中文逐项-宏 M0130：TIME_PRINT
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TIME_PRINT；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TIME_PRINT(name)
#endif
#endif
