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
/* 中文内部逻辑 F0460：dumpLine
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int prefixlen = strlen(prefix);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：char line[STRLENGTH+1];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：line[STRLENGTH] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：memset(line, ' ', STRLENGTH);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 调用 / 计算 / 写入：strncpy(line, prefix, PREFIXLEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 循环：初始 int i=0;；继续条件 i<nranks && i<MAXWIDTH；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 调用 / 计算 / 写入：sprintf(line+prefixlen+4*i, " %3d", values[i]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 9. 调用 / 计算 / 写入：INFO(NCCL_INIT,"%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__rings_cc.md#f0460
 */
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
/* 中文内部逻辑 F0461：ncclBuildRings
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int r=0;；继续条件 r<nrings；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 准备局部状态：char prefix[40];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 准备局部状态：int current = rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   4. 循环：初始 int i=0;；继续条件 i<nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     5. 调用 / 计算 / 写入：rings[r*nranks+i] = current;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：current = next[r*nranks+current];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   8. 调用 / 计算 / 写入：sprintf(prefix, "Channel %02d/%02d : ", r, nrings);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   9. 条件判断：(rank == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     10. 真分支：；仅条件为真执行以下子项。
 *       11. 调用 / 计算 / 写入：dumpLine(rings+r*nranks, nranks, prefix);；按固定宽度打印一段 rank 顺序，限制输出长度避免大规模日志过量。
 *   12. 条件判断：(current != rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     13. 真分支：；仅条件为真执行以下子项。
 *       14. 调用 / 计算 / 写入：WARN("Error : ring %d does not loop back to start (%d != %d)", r, current, rank);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       15. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   16. 循环：初始 int i=0;；继续条件 i<nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     17. 准备局部状态：int found = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     18. 循环：初始 int j=0;；继续条件 j<nranks；每轮更新 j++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       19. 条件判断：(rings[r*nranks+j] == i)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         20. 真分支：；仅条件为真执行以下子项。
 *           21. 调用 / 计算 / 写入：found = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           22. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *     23. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     24. 条件判断：(found == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       25. 真分支：；仅条件为真执行以下子项。
 *         26. 调用 / 计算 / 写入：WARN("Error : ring %d does not contain rank %d", r, i);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         27. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   28. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 29. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 30. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__rings_cc.md#f0461
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
