/*************************************************************************
 * Copyright (c) 2016-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "core.h"

/* 中文逐项-常量 X0025：MAXWIDTH
 * 编译期定义：MAXWIDTH = 20。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAXWIDTH 20
/* 中文逐项-常量 X0026：PREFIXLEN
 * 编译期定义：PREFIXLEN = 15。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PREFIXLEN 15
/* 中文逐项-常量 X0027：STRLENGTH
 * 编译期定义：STRLENGTH = (PREFIXLEN+5*MAXWIDTH)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define STRLENGTH (PREFIXLEN+5*MAXWIDTH)
/* 中文逐项-函数 F0460：dumpLine
 * 用途与位置：按固定宽度打印一段 rank 顺序，限制输出长度避免大规模日志过量。
 * 参数 values（int* values）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 prefix（const char* prefix）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/rings.cc::ncclBuildRings。
 * 本体直接出现的调用/包装：memset；sprintf；strlen；strncpy。
 * 需要对照的边界/推进条件：i<nranks && i<MAXWIDTH。各条件所属分支见紧邻函数体。
 */
void dumpLine(int* values, int nranks, const char* prefix) {
/* 中文逐项-变量 V1280：prefixlen
 * prefixlen（int；函数 dumpLine 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 prefixlen = strlen(prefix)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int prefixlen = strlen(prefix);
/* 中文逐项-变量 V1281：line
 * line（char；函数 dumpLine 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：line[STRLENGTH] = '\0';。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[STRLENGTH+1];
  line[STRLENGTH] = '\0';
  memset(line, ' ', STRLENGTH);
  strncpy(line, prefix, PREFIXLEN);
  for (/* 中文逐项-变量 V1282：i
 * i（int；函数 dumpLine 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nranks && i<MAXWIDTH；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nranks && i<MAXWIDTH; i++) sprintf(line+prefixlen+4*i, " %3d", values[i]);
  INFO(NCCL_INIT,"%s", line);
}

/* 中文精读：最终环的完整性检查
 * 从当前 rank 出发连续跟 next 走 nranks 步，记录 ring 的 rank 顺序。
 * 之后确认恰好回到起点，并逐个确认所有 rank 都被访问，避免局部小环或遗漏节点。
 * 这个验证发生在初始化；collective 运行时不再为每个数据块重新搜索一遍全局环。
 * prev/next 给通信邻居，rings/userRanks 给算法需要的全局排列，两份信息用途不同。
 */
/* 中文逐项-函数 F0461：ncclBuildRings
 * 用途与位置：跟随 next 构建每 channel 环顺序，验证 nranks 步回到起点且没有遗漏参与 rank。
 * 参数 nrings（int nrings）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 rings（int* rings）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 prev（int* prev）：环前后继或链表前后项，准确含义由所属 ring/队列类型决定。
 * 参数 next（int* next）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::ncclTopoPostset。
 * 本体直接出现的调用/包装：dumpLine；sprintf。
 * 需要对照的边界/推进条件：r<nrings；i<nranks；rank == 0；current != rank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclBuildRings(int nrings, int* rings, int rank, int nranks, int* prev, int* next) {
  for (/* 中文逐项-变量 V1283：r
 * r（int；函数 ncclBuildRings 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nrings；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nrings; r++) {
/* 中文逐项-变量 V1284：prefix
 * prefix（char；函数 ncclBuildRings 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：sprintf(prefix, "Channel %02d/%02d : ", r, nrings);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char prefix[40];
    /*sprintf(prefix, "[%d] Channel %d Prev : ", rank, r);
    dumpLine(prev+r*nranks, nranks, prefix);
    sprintf(prefix, "[%d] Channel %d Next : ", rank, r);
    dumpLine(next+r*nranks, nranks, prefix);*/

/* 中文逐项-变量 V1285：current
 * current（int；函数 ncclBuildRings 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 current = rank；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int current = rank;
    for (/* 中文逐项-变量 V1286：i
 * i（int；函数 ncclBuildRings 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nranks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nranks; i++) {
      rings[r*nranks+i] = current;
      current = next[r*nranks+current];
    }
    sprintf(prefix, "Channel %02d/%02d : ", r, nrings);
    if (rank == 0) dumpLine(rings+r*nranks, nranks, prefix);
    if (current != rank) {
      WARN("Error : ring %d does not loop back to start (%d != %d)", r, current, rank);
      return ncclInternalError;
    }
    // Check that all ranks are there
    for (/* 中文逐项-变量 V1287：i
 * i（int；函数 ncclBuildRings 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nranks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nranks; i++) {
/* 中文逐项-变量 V1288：found
 * found（int；函数 ncclBuildRings 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 found = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int found = 0;
      for (/* 中文逐项-变量 V1289：j
 * j（int；函数 ncclBuildRings 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：j<nranks；循环体与增量表达式决定每轮如何推进。
 */int j=0; j<nranks; j++) {
        if (rings[r*nranks+j] == i) {
          found = 1;
          break;
        }
      }
      if (found == 0) {
        WARN("Error : ring %d does not contain rank %d", r, i);
        return ncclInternalError;
      }
    }
  }
  return ncclSuccess;
}
