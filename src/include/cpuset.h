/*************************************************************************
 * Copyright (c) 2018-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_CPUSET_H_
/* 中文逐项-常量 X0186：NCCL_CPUSET_H_
 * 编译期定义：NCCL_CPUSET_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_CPUSET_H_

// Convert local_cpus, e.g. 0003ff,f0003fff to cpu_set_t

/* 中文逐项-函数 F0629：hexToInt
 * 用途与位置：把十六进制字符转换成位掩码需要的数值，辅助 CPU affinity 字符串解析。
 * 参数 c（char c）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/cpuset.h::ncclStrToCpuset。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：v < 0；v > 9；v < 0) || (v > 15。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return -1;；return v;。CHECK类宏还可能提前返回/跳转。
 */
static int hexToInt(char c) {
/* 中文逐项-变量 V2105：v
 * v（int；函数 hexToInt 内）：LL行的两个64位或int4整行视图，用向量汇编同时读写数据和标记。
 * 初值/来源 v = c - '0'；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int v = c - '0';
  if (v < 0) return -1;
  if (v > 9) v = 10 + c - 'a';
  if ((v < 0) || (v > 15)) return -1;
  return v;
}

/* 中文逐项-常量 X0187：CPU_SET_N_U32
 * 编译期定义：CPU_SET_N_U32 = (sizeof(cpu_set_t)/sizeof(uint32_t))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define CPU_SET_N_U32 (sizeof(cpu_set_t)/sizeof(uint32_t))

/* 中文逐项-函数 F0630：ncclStrToCpuset
 * 用途与位置：把系统/配置的十六进制 CPU 位图解析成 cpu_set_t，支持线程亲和性设置。
 * 参数 str（const char* str）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 参数 mask（cpu_set_t* mask）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddCpu。
 * 本体直接出现的调用/包装：hexToInt；memcpy；strlen。
 * 需要对照的边界/推进条件：o<strlen(str；c == ','；v == -1；m<CPU_SET_N_U32。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclStrToCpuset(const char* str, cpu_set_t* mask) {
/* 中文逐项-变量 V2106：cpumasks
 * cpumasks（uint32_t；函数 ncclStrToCpuset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cpumasks[m] = 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  uint32_t cpumasks[CPU_SET_N_U32];
/* 中文逐项-变量 V2107：m
 * m（int；函数 ncclStrToCpuset 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 m = CPU_SET_N_U32-1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int m = CPU_SET_N_U32-1;
  cpumasks[m] = 0;
  for (/* 中文逐项-变量 V2108：o
 * o（int；函数 ncclStrToCpuset 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 o = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：o<strlen(str)；循环体与增量表达式决定每轮如何推进。
 */int o=0; o<strlen(str); o++) {
/* 中文逐项-变量 V2109：c
 * c（char；函数 ncclStrToCpuset 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 c = str[o]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char c = str[o];
    if (c == ',') {
      m--;
      cpumasks[m] = 0;
    } else {
/* 中文逐项-变量 V2110：v
 * v（int；函数 ncclStrToCpuset 内）：LL行的两个64位或int4整行视图，用向量汇编同时读写数据和标记。
 * 初值/来源 v = hexToInt(c)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int v = hexToInt(c);
      if (v == -1) break;
      cpumasks[m] <<= 4;
      cpumasks[m] += v;
    }
  }
  // Copy cpumasks to mask
  for (/* 中文逐项-变量 V2111：a
 * a（int；函数 ncclStrToCpuset 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：m<CPU_SET_N_U32；循环体与增量表达式决定每轮如何推进。
 */int a=0; m<CPU_SET_N_U32; a++,m++) {
    memcpy(((uint32_t*)mask)+a, cpumasks+m, sizeof(uint32_t));
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0631：ncclCpusetToStr
 * 用途与位置：把 cpu_set_t 格式化成十六进制文本，方便导出/诊断拓扑 CPU affinity。
 * 参数 mask（cpu_set_t* mask）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 * 参数 str（char* str）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetCpuAffinity。
 * 本体直接出现的调用/包装：sprintf。
 * 需要对照的边界/推进条件：o>=0；c == 0 && m8[o] == 0；o && o%4 == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclCpusetToStr(cpu_set_t* mask, char* str) {
/* 中文逐项-变量 V2112：c
 * c（int；函数 ncclCpusetToStr 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int c = 0;
/* 中文逐项-变量 V2113：m8
 * m8（uint8_t；函数 ncclCpusetToStr 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 m8 = (uint8_t*)mask；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  uint8_t* m8 = (uint8_t*)mask;
  for (/* 中文逐项-变量 V2114：o
 * o（int；函数 ncclCpusetToStr 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 o = sizeof(cpu_set_t)-1；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 * 所在for循环的继续条件：o>=0；循环体与增量表达式决定每轮如何推进。
 */int o=sizeof(cpu_set_t)-1; o>=0; o--) {
    if (c == 0 && m8[o] == 0) continue;
    sprintf(str+c, "%02x", m8[o]);
    c+=2;
    if (o && o%4 == 0) {
      sprintf(str+c, ",");
      c++;
    }
  }
  str[c] = '\0';
  return ncclSuccess;
}

#endif
