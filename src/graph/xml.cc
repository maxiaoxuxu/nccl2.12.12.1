/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "core.h"
#include "nvmlwrap.h"
/* 中文精读：从系统发现信息到图搜索输入
 * 本文件负责 XML 解析/写出，并从 sysfs/NVML 等补齐硬件属性。
 * TopoGetSystem 可以读用户文件，也可自动发现，再转换成 graph/topo.h 的节点/链路对象。
 * 硬件树、路径缓存、通信搜索图是连续的三个表示阶段，应按转换函数逐层跟踪。
 * 此层只处理元数据，真正的 GPU/NIC 数据传输在 transport 和 collectives/device。
 */
#include "xml.h"

/*******************/
/* XML File Parser */
/*******************/

/* 中文内部逻辑 F0532：xmlGetChar
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(fread(c, 1, 1, file) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：WARN("XML Parse : Unexpected EOF");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0532
 */
/* 中文逐项-函数 F0532：xmlGetChar
 * 用途与位置：从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 c（char* c）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::xmlGetValue；src/graph/xml.cc::xmlGetToken。
 * 本体直接出现的调用/包装：fread。
 * 需要对照的边界/推进条件：fread(c, 1, 1, file) == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t xmlGetChar(FILE* file, char* c) {
  if (fread(c, 1, 1, file) == 0) {
    WARN("XML Parse : Unexpected EOF");
    return ncclInternalError;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0533：xmlGetValue
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetChar(file, &c));；失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。
 * 3. 条件判断：(c != '"' && c != '\'')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 编译期分支/指令：#if INT_OK；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       6. 准备局部状态：int o = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       7. 先执行一次的循环：初始 ；继续条件 (c >= '0' && c <= '9')；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *         8. 调用 / 计算 / 写入：value[o++] = c;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         9. 调用 / 计算 / 写入：NCCLCHECK(xmlGetChar(file, &c));；失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。
 *       10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       11. 调用 / 计算 / 写入：value[o] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       12. 调用 / 计算 / 写入：*last = c;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *       14. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *         15. 调用 / 计算 / 写入：WARN("XML Parse : Expected (double) quote.");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         16. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 17. 准备局部状态：int o = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 18. 先执行一次的循环：初始 ；继续条件 (c != '"')；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *   19. 调用 / 计算 / 写入：NCCLCHECK(xmlGetChar(file, &c));；失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。
 *   20. 调用 / 计算 / 写入：value[o++] = c;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 21. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 22. 调用 / 计算 / 写入：value[o-1] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 23. 调用 / 计算 / 写入：NCCLCHECK(xmlGetChar(file, last));；失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。
 * 24. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0533
 */
/* 中文逐项-函数 F0533：xmlGetValue
 * 用途与位置：解析 XML 的引号属性值或对应值片段，限制目标容量避免读取越界。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 value（char* value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 参数 last（char* last）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::xmlGetToken。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlGetChar。
 * 需要对照的边界/推进条件：c != '"' && c != '\''。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t xmlGetValue(FILE* file, char* value, char* last) {
/* 中文逐项-变量 V1712：c
 * c（char；函数 xmlGetValue 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetChar(file, &c));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char c;
  NCCLCHECK(xmlGetChar(file, &c));
  if (c != '"' && c != '\'') {
#if INT_OK
/* 中文逐项-变量 V1713：o
 * o（int；函数 xmlGetValue 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 o = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int o = 0;
    do {
      value[o++] = c;
      NCCLCHECK(xmlGetChar(file, &c));
    } while (c >= '0' && c <= '9');
    value[o] = '\0';
    *last = c;
    return ncclSuccess;
#else
    WARN("XML Parse : Expected (double) quote.");
    return ncclInternalError;
#endif
  }
/* 中文逐项-变量 V1714：o
 * o（int；函数 xmlGetValue 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 o = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int o = 0;
  do {
    NCCLCHECK(xmlGetChar(file, &c));
    value[o++] = c;
  } while (c != '"');
  value[o-1] = '\0';
  NCCLCHECK(xmlGetChar(file, last));
  return ncclSuccess;
}

/* 中文内部逻辑 F0534：xmlGetToken
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：char* ptr = name;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int o = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 先执行一次的循环：初始 ；继续条件 (c != ' ' && c != '>' && c != '/' && c != '\n' && c != '\r')；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *   5. 调用 / 计算 / 写入：NCCLCHECK(xmlGetChar(file, &c));；失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。
 *   6. 条件判断：(c == '=')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     7. 真分支：；仅条件为真执行以下子项。
 *       8. 调用 / 计算 / 写入：ptr[o] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       9. 条件判断：(value == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         10. 真分支：；仅条件为真执行以下子项。
 *           11. 调用 / 计算 / 写入：WARN("XML Parse : Unexpected value with name %s", ptr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           12. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *       13. 函数返回：return xmlGetValue(file, value, last);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   14. 调用 / 计算 / 写入：ptr[o] = c;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   15. 条件判断：(o == MAX_STR_LEN-1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     16. 真分支：；仅条件为真执行以下子项。
 *       17. 调用 / 计算 / 写入：ptr[o] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       18. 调用 / 计算 / 写入：WARN("Error : name %s too long (max %d)", ptr, MAX_STR_LEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       19. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   20. 调用 / 计算 / 写入：o++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 21. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 22. 调用 / 计算 / 写入：ptr[o-1] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 23. 调用 / 计算 / 写入：*last = c;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 24. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0534
 */
/* 中文逐项-函数 F0534：xmlGetToken
 * 用途与位置：扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 name（char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 参数 value（char* value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 参数 last（char* last）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::xmlGetNode。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlGetChar；xmlGetValue。
 * 需要对照的边界/推进条件：c == '='；value == NULL；o == MAX_STR_LEN-1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return xmlGetValue(file, value, last);；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t xmlGetToken(FILE* file, char* name, char* value, char* last) {
/* 中文逐项-变量 V1715：c
 * c（char；函数 xmlGetToken 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetChar(file, &c));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char c;
/* 中文逐项-变量 V1716：ptr
 * ptr（char；函数 xmlGetToken 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = name；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* ptr = name;
/* 中文逐项-变量 V1717：o
 * o（int；函数 xmlGetToken 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 o = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int o = 0;
  do {
    NCCLCHECK(xmlGetChar(file, &c));
    if (c == '=') {
      ptr[o] = '\0';
      if (value == NULL) {
        WARN("XML Parse : Unexpected value with name %s", ptr);
        return ncclInternalError;
      }
      return xmlGetValue(file, value, last);
    }
    ptr[o] = c;
    if (o == MAX_STR_LEN-1) {
      ptr[o] = '\0';
      WARN("Error : name %s too long (max %d)", ptr, MAX_STR_LEN);
      return ncclInternalError;
    }
    o++;
  } while (c != ' ' && c != '>' && c != '/' && c != '\n' && c != '\r');
  ptr[o-1] = '\0';
  *last = c;
  return ncclSuccess;
}

// Shift the 3-chars string by one char and append c at the end
/* 中文逐项-宏 M0047：SHIFT_APPEND
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 SHIFT_APPEND；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define SHIFT_APPEND(s, c) do { s[0]=s[1]; s[1]=s[2]; s[2]=c; } while(0)
/* 中文内部逻辑 F0535：xmlSkipComment
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char end[4] = "...";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 int i=0;；继续条件 i<strlen(start)；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 调用 / 计算 / 写入：SHIFT_APPEND(end, start[i]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 5. 调用 / 计算 / 写入：SHIFT_APPEND(end, next);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 循环：初始 ；继续条件 (strcmp(end, "-->") != 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 准备局部状态：int c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 条件判断：(fread(&c, 1, 1, file) != 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     9. 真分支：；仅条件为真执行以下子项。
 *       10. 调用 / 计算 / 写入：WARN("XML Parse error : unterminated comment");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       11. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   12. 调用 / 计算 / 写入：SHIFT_APPEND(end, c);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 14. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0535
 */
/* 中文逐项-函数 F0535：xmlSkipComment
 * 用途与位置：跳过 XML 注释直到终止符，不把注释内容误识别成拓扑节点。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 start（char* start）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 next（char next）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::xmlGetNode。
 * 本体直接出现的调用/包装：SHIFT_APPEND；fread；strcmp；strlen。
 * 需要对照的边界/推进条件：i<strlen(start；strcmp(end, "-->") != 0；fread(&c, 1, 1, file) != 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t xmlSkipComment(FILE* file, char* start, char next) {
  // Start from something neutral with \0 at the end.
/* 中文逐项-变量 V1718：end
 * end（char；函数 xmlSkipComment 内）：本任务结束序号或范围末端，通常由 base 与总步数推导。
 * 初值/来源 end = "..."；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char end[4] = "...";

  // Inject all trailing chars from previous reads. We don't need
  // to check for --> here because there cannot be a > in the name.
  for (/* 中文逐项-变量 V1719：i
 * i（int；函数 xmlSkipComment 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<strlen(start)；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<strlen(start); i++) SHIFT_APPEND(end, start[i]);
  SHIFT_APPEND(end, next);

  // Stop when we find "-->"
  while (strcmp(end, "-->") != 0) {
/* 中文逐项-变量 V1720：c
 * c（int；函数 xmlSkipComment 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 此处未显式给初值；后续首个文本使用点：if (fread(&c, 1, 1, file) != 1) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int c;
    if (fread(&c, 1, 1, file) != 1) {
      WARN("XML Parse error : unterminated comment");
      return ncclInternalError;
    }
    SHIFT_APPEND(end, c);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0536：xmlGetNode
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：node->type = NODE_TYPE_NONE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 准备局部状态：char c = ' ';；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 ；继续条件 (c == ' ' || c == '\n' || c == '\r')；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 条件判断：(fread(&c, 1, 1, file) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 条件判断：(c != '<')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：WARN("XML Parse error : expecting '<', got '%c'", c);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     11. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(xmlGetToken(file, node->name, NULL, &c));；失败时可能提前return，成功才继续；扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。
 * 13. 条件判断：(strncmp(node->name, "!--", 3) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 调用 / 计算 / 写入：NCCLCHECK(xmlSkipComment(file, node->name+3, c));；失败时可能提前return，成功才继续；跳过 XML 注释直到终止符，不把注释内容误识别成拓扑节点。
 *     16. 函数返回：return xmlGetNode(file, node);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 17. 条件判断：(node->name[0] == '\0' && c == '/')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   18. 真分支：；仅条件为真执行以下子项。
 *     19. 调用 / 计算 / 写入：node->type = NODE_TYPE_CLOSE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     20. 调用 / 计算 / 写入：NCCLCHECK(xmlGetToken(file, node->name, NULL, &c));；失败时可能提前return，成功才继续；扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。
 *     21. 条件判断：(c != '>')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       22. 真分支：；仅条件为真执行以下子项。
 *         23. 调用 / 计算 / 写入：WARN("XML Parse error : unexpected trailing %c in closing tag %s", c, node->name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         24. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     25. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 26. 调用 / 计算 / 写入：node->type = NODE_TYPE_OPEN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 27. 准备局部状态：int a = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 28. 循环：初始 ；继续条件 (c == ' ')；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   29. 调用 / 计算 / 写入：NCCLCHECK(xmlGetToken(file, node->attrs[a].key, node->attrs[a].value, &c));；失败时可能提前return，成功才继续；扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。
 *   30. 条件判断：(a == MAX_ATTR_COUNT)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     31. 真分支：；仅条件为真执行以下子项。
 *       32. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "XML Parse : Ignoring extra attributes (max %d)", MAX_ATTR_COUNT);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     33. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       34. 调用 / 计算 / 写入：a++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 35. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 36. 调用 / 计算 / 写入：node->nAttrs = a;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 37. 条件判断：(c == '/')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   38. 真分支：；仅条件为真执行以下子项。
 *     39. 调用 / 计算 / 写入：node->type = NODE_TYPE_SINGLE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     40. 准备局部状态：char str[MAX_STR_LEN];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     41. 调用 / 计算 / 写入：NCCLCHECK(xmlGetToken(file, str, NULL, &c));；失败时可能提前return，成功才继续；扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。
 * 42. 条件判断：(c != '>')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   43. 真分支：；仅条件为真执行以下子项。
 *     44. 调用 / 计算 / 写入：WARN("XML Parse : expected >, got '%c'", c);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     45. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 46. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0536
 */
/* 中文逐项-函数 F0536：xmlGetNode
 * 用途与位置：读取一个 XML 标签的名字、开闭类型和属性，填入节点表示。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::xmlGetNode；src/graph/xml.cc::xmlLoadSub。
 * 本体直接出现的调用/包装：NCCLCHECK；fread；strncmp；xmlGetNode；xmlGetToken；xmlSkipComment。
 * 需要对照的边界/推进条件：c == ' ' || c == '\n' || c == '\r'；fread(&c, 1, 1, file) == 0；c != '<'；strncmp(node->name, "!--", 3)
 * == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;；return xmlGetNode(file, node);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t xmlGetNode(FILE* file, struct ncclXmlNode* node) {
  node->type = NODE_TYPE_NONE;
/* 中文逐项-变量 V1721：c
 * c（char；函数 xmlGetNode 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = ' '；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char c = ' ';
  while (c == ' ' || c == '\n' || c == '\r') {
    if (fread(&c, 1, 1, file) == 0) return ncclSuccess;
  }
  if (c != '<') {
    WARN("XML Parse error : expecting '<', got '%c'", c);
    return ncclInternalError;
  }
  // Read XML element name
  NCCLCHECK(xmlGetToken(file, node->name, NULL, &c));

  // Check for comments
  if (strncmp(node->name, "!--", 3) == 0) {
    NCCLCHECK(xmlSkipComment(file, node->name+3, c));
    return xmlGetNode(file, node);
  }

  // Check for closing tag
  if (node->name[0] == '\0' && c == '/') {
    node->type = NODE_TYPE_CLOSE;
    // Re-read the name, we got '/' in the first call
    NCCLCHECK(xmlGetToken(file, node->name, NULL, &c));
    if (c != '>') {
      WARN("XML Parse error : unexpected trailing %c in closing tag %s", c, node->name);
      return ncclInternalError;
    }
    return ncclSuccess;
  }

  node->type = NODE_TYPE_OPEN;

  // Get Attributes
/* 中文逐项-变量 V1722：a
 * a（int；函数 xmlGetNode 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int a = 0;
  while (c == ' ') {
    NCCLCHECK(xmlGetToken(file, node->attrs[a].key, node->attrs[a].value, &c));
    if (a == MAX_ATTR_COUNT) {
      INFO(NCCL_GRAPH, "XML Parse : Ignoring extra attributes (max %d)", MAX_ATTR_COUNT);
      // Actually we need to still consume the extra attributes so we have an extra one.
    } else a++;
  }
  node->nAttrs = a;
  if (c == '/') {
    node->type = NODE_TYPE_SINGLE;
/* 中文逐项-变量 V1723：str
 * str（char；函数 xmlGetNode 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetToken(file, str, NULL, &c));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char str[MAX_STR_LEN];
    NCCLCHECK(xmlGetToken(file, str, NULL, &c));
  }
  if (c != '>') {
    WARN("XML Parse : expected >, got '%c'", c);
    return ncclInternalError;
  }
  return ncclSuccess;
}

typedef ncclResult_t (*xmlHandlerFunc_t)(FILE*, struct ncclXml*, struct ncclXmlNode*);

/* 中文逐项-类型 T0148：xmlHandler
 * 类型职责：XML标签及其加载回调的分派项，决定遇到某种子标签怎样继续解析。
 * 声明内可见成员（含嵌套成员）：name、func。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct xmlHandler {
/* 中文逐项-变量 V1724：name
 * name（char；成员 xmlHandler）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  const char * name;
/* 中文逐项-变量 V1725：func
 * func（xmlHandlerFunc_t；成员 xmlHandler）：函数指针或 CUDA kernel 入口地址，由所选算法/函数表/解析器回调决定。
 */
  xmlHandlerFunc_t func;
};

/* 中文内部逻辑 F0537：xmlLoadSub
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(head && head->type == NODE_TYPE_SINGLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 循环：初始 ；继续条件 (1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 条件判断：(xml->maxIndex == MAX_NODES)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     6. 真分支：；仅条件为真执行以下子项。
 *       7. 调用 / 计算 / 写入：WARN("Error : XML parser is limited to 1024 nodes");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       8. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   9. 准备局部状态：struct ncclXmlNode* node = xml->nodes+xml->maxIndex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   10. 调用 / 计算 / 写入：memset(node, 0, sizeof(struct ncclXmlNode));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   11. 调用 / 计算 / 写入：NCCLCHECK(xmlGetNode(file, node));；失败时可能提前return，成功才继续；读取一个 XML 标签的名字、开闭类型和属性，填入节点表示。
 *   12. 条件判断：(node->type == NODE_TYPE_NONE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     13. 真分支：；仅条件为真执行以下子项。
 *       14. 条件判断：(head)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         15. 真分支：；仅条件为真执行以下子项。
 *           16. 调用 / 计算 / 写入：WARN("XML Parse : unterminated %s", head->name);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           17. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *         18. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   20. 条件判断：(head && node->type == NODE_TYPE_CLOSE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     21. 真分支：；仅条件为真执行以下子项。
 *       22. 条件判断：(strcmp(node->name, head->name) != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         23. 真分支：；仅条件为真执行以下子项。
 *           24. 调用 / 计算 / 写入：WARN("XML Mismatch : %s / %s", head->name, node->name);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *           25. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *       26. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   27. 准备局部状态：int found = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   28. 循环：初始 int h=0;；继续条件 h<nHandlers；每轮更新 h++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     29. 条件判断：(strcmp(node->name, handlers[h].name) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       30. 真分支：；仅条件为真执行以下子项。
 *         31. 条件判断：(head)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           32. 真分支：；仅条件为真执行以下子项。
 *             33. 调用 / 计算 / 写入：head->subs[head->nSubs++] = node;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *         34. 调用 / 计算 / 写入：node->parent = head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *         35. 调用 / 计算 / 写入：node->nSubs = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         36. 调用 / 计算 / 写入：xml->maxIndex++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         37. 调用 / 计算 / 写入：NCCLCHECK(handlers[h].func(file, xml, node));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         38. 调用 / 计算 / 写入：found = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         39. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *   40. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   41. 条件判断：(!found)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     42. 真分支：；仅条件为真执行以下子项。
 *       43. 条件判断：(nHandlers)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         44. 真分支：；仅条件为真执行以下子项。
 *           45. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Ignoring element %s", node->name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       46. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, node, NULL, 0));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 47. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0537
 */
/* 中文逐项-函数 F0537：xmlLoadSub
 * 用途与位置：递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 参数 handlers（struct xmlHandler handlers[]）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 nHandlers（int nHandlers）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::xmlLoadSub；src/graph/xml.cc::ncclTopoXmlLoadNvlink；src/graph/xml.cc::ncclTopoXmlLoadGpu；src/graph/xml.cc::ncclTopoXmlLoadNet；src/graph/xml.cc::ncclTopoXmlLoadNic；src/graph/xml.cc::ncclTopoXmlLoadPci；src/graph/xml.cc::ncclTopoXmlLoadCpu；其余见调用清单。
 * 本体直接出现的调用/包装：NCCLCHECK；handlers[h].func；memset；strcmp；xmlGetNode；xmlLoadSub。
 * 需要对照的边界/推进条件：head && head->type == NODE_TYPE_SINGLE；1；xml->maxIndex == MAX_NODES；node->type ==
 * NODE_TYPE_NONE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t xmlLoadSub(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head, struct xmlHandler handlers[], int nHandlers) {
  if (head && head->type == NODE_TYPE_SINGLE) return ncclSuccess;
  while (1) {
    if (xml->maxIndex == MAX_NODES) {
      WARN("Error : XML parser is limited to 1024 nodes");
      return ncclInternalError;
    }
/* 中文逐项-变量 V1726：node
 * node（struct ncclXmlNode；函数 xmlLoadSub 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = xml->nodes+xml->maxIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* node = xml->nodes+xml->maxIndex;
    memset(node, 0, sizeof(struct ncclXmlNode));
    NCCLCHECK(xmlGetNode(file, node));
    if (node->type == NODE_TYPE_NONE) {
      if (head) {
        WARN("XML Parse : unterminated %s", head->name);
        return ncclInternalError;
      } else {
        // All done
        return ncclSuccess;
      }
    }
    if (head && node->type == NODE_TYPE_CLOSE) {
      if (strcmp(node->name, head->name) != 0) {
        WARN("XML Mismatch : %s / %s", head->name, node->name);
        return ncclInternalError;
      }
      return ncclSuccess;
    }
/* 中文逐项-变量 V1727：found
 * found（int；函数 xmlLoadSub 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 found = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int found = 0;
    for (/* 中文逐项-变量 V1728：h
 * h（int；函数 xmlLoadSub 内）：GDRCopy固定缓冲句柄的内部表示，必须用匹配的unpin接口回收。
 * 初值/来源 h = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：h<nHandlers；循环体与增量表达式决定每轮如何推进。
 */int h=0; h<nHandlers; h++) {
      if (strcmp(node->name, handlers[h].name) == 0) {
        if (head) head->subs[head->nSubs++] = node;
        node->parent = head;
        node->nSubs = 0;
        xml->maxIndex++;
        NCCLCHECK(handlers[h].func(file, xml, node));
        found = 1;
        break;
      }
    }
    if (!found) {
      if (nHandlers) INFO(NCCL_GRAPH, "Ignoring element %s", node->name);
      NCCLCHECK(xmlLoadSub(file, xml, node, NULL, 0));
    }
  }
}

/**************/
/* XML Writer */
/**************/

/* 中文内部逻辑 F0538：ncclTopoDumpXmlRec
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int i=0;；继续条件 i<indent；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 调用 / 计算 / 写入：fprintf(file, " ");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 3. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 4. 调用 / 计算 / 写入：fprintf(file, "<%s", node->name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 循环：初始 int a=0;；继续条件 a<node->nAttrs；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 调用 / 计算 / 写入：fprintf(file, " %s=\"%s\"", node->attrs[a].key, node->attrs[a].value);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 条件判断：(node->nSubs == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：fprintf(file, "/>\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   11. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     12. 调用 / 计算 / 写入：fprintf(file, ">\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     13. 循环：初始 int s=0;；继续条件 s<node->nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       14. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoDumpXmlRec(indent+2, file, node->subs[s]));；失败时可能提前return，成功才继续；递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。
 *     15. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     16. 循环：初始 int i=0;；继续条件 i<indent；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       17. 调用 / 计算 / 写入：fprintf(file, " ");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     18. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     19. 调用 / 计算 / 写入：fprintf(file, "</%s>\n", node->name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 20. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0538
 */
/* 中文逐项-函数 F0538：ncclTopoDumpXmlRec
 * 用途与位置：递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。
 * 参数 indent（int indent）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoDumpXmlRec；src/graph/xml.cc::ncclTopoDumpXmlToFile。
 * 本体直接出现的调用/包装：NCCLCHECK；fprintf；ncclTopoDumpXmlRec。
 * 需要对照的边界/推进条件：i<indent；a<node->nAttrs；node->nSubs == 0；s<node->nSubs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoDumpXmlRec(int indent, FILE* file, struct ncclXmlNode* node) {
  for (/* 中文逐项-变量 V1729：i
 * i（int；函数 ncclTopoDumpXmlRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<indent；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<indent; i++) fprintf(file, " ");
  fprintf(file, "<%s", node->name);

  for (/* 中文逐项-变量 V1730：a
 * a（int；函数 ncclTopoDumpXmlRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<node->nAttrs；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<node->nAttrs; a++) {
    fprintf(file, " %s=\"%s\"", node->attrs[a].key, node->attrs[a].value);
  }
  if (node->nSubs == 0) {
    fprintf(file, "/>\n");
  } else {
    fprintf(file, ">\n");
    for (/* 中文逐项-变量 V1731：s
 * s（int；函数 ncclTopoDumpXmlRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<node->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<node->nSubs; s++) {
      NCCLCHECK(ncclTopoDumpXmlRec(indent+2, file, node->subs[s]));
    }
    for (/* 中文逐项-变量 V1732：i
 * i（int；函数 ncclTopoDumpXmlRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<indent；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<indent; i++) fprintf(file, " ");
    fprintf(file, "</%s>\n", node->name);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0539：ncclTopoDumpXmlToFile
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：FILE* file = fopen(xmlTopoFile, "w");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(file == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("Unable to open %s, not dumping topology.", xmlTopoFile);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoDumpXmlRec(0, file, xml->nodes));；失败时可能提前return，成功才继续；递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。
 * 7. 调用 / 计算 / 写入：fclose(file);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0539
 */
/* 中文逐项-函数 F0539：ncclTopoDumpXmlToFile
 * 用途与位置：递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。
 * 参数 xmlTopoFile（const char* xmlTopoFile）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoDumpGraphs；src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；fclose；fopen；ncclTopoDumpXmlRec。
 * 需要对照的边界/推进条件：file == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoDumpXmlToFile(const char* xmlTopoFile, struct ncclXml* xml) {
/* 中文逐项-变量 V1733：file
 * file（FILE；函数 ncclTopoDumpXmlToFile 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 初值/来源 file = fopen(xmlTopoFile, "w")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  FILE* file = fopen(xmlTopoFile, "w");
  if (file == NULL) {
    WARN("Unable to open %s, not dumping topology.", xmlTopoFile);
    return ncclSuccess;
  }
  NCCLCHECK(ncclTopoDumpXmlRec(0, file, xml->nodes));
  fclose(file);
  return ncclSuccess;
}

/****************************************/
/* Parser rules for our specific format */
/****************************************/

/* 中文内部逻辑 F0540：ncclTopoXmlLoadNvlink
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0540
 */
/* 中文逐项-函数 F0540：ncclTopoXmlLoadNvlink
 * 用途与位置：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlLoadNvlink(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
  NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));
  return ncclSuccess;
}

/* 中文内部逻辑 F0541：ncclTopoXmlLoadGpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct xmlHandler handlers[] = { { "nvlink", ncclTopoXmlLoadNvlink } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0541
 */
/* 中文逐项-函数 F0541：ncclTopoXmlLoadGpu
 * 用途与位置：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlLoadGpu(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
/* 中文逐项-变量 V1734：handlers
 * handlers（struct xmlHandler；函数 ncclTopoXmlLoadGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "nvlink", ncclTopoXmlLoadNvlink } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "nvlink", ncclTopoXmlLoadNvlink } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));
  return ncclSuccess;
}

/* 中文内部逻辑 F0542：ncclTopoXmlLoadNet
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0542
 */
/* 中文逐项-函数 F0542：ncclTopoXmlLoadNet
 * 用途与位置：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlLoadNet(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
  NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));
  return ncclSuccess;
}

/* 中文内部逻辑 F0543：ncclTopoXmlLoadNic
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct xmlHandler handlers[] = { { "net", ncclTopoXmlLoadNet } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0543
 */
/* 中文逐项-函数 F0543：ncclTopoXmlLoadNic
 * 用途与位置：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlLoadNic(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
/* 中文逐项-变量 V1735：handlers
 * handlers（struct xmlHandler；函数 ncclTopoXmlLoadNic 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "net", ncclTopoXmlLoadNet } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "net", ncclTopoXmlLoadNet } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));
  return ncclSuccess;
}

/* 中文内部逻辑 F0544：ncclTopoXmlLoadPci
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct xmlHandler handlers[] = { { "pci", ncclTopoXmlLoadPci }, { "gpu", ncclTopoXmlLoadGpu }, { "nic", ncclTopoXmlLoadNic} };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 3));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0544
 */
/* 中文逐项-函数 F0544：ncclTopoXmlLoadPci
 * 用途与位置：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlLoadPci(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
/* 中文逐项-变量 V1736：handlers
 * handlers（struct xmlHandler；函数 ncclTopoXmlLoadPci 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "pci", ncclTopoXmlLoadPci }, { "gpu", ncclTopoXmlLoadGpu }, { "nic", ncclTopoXmlLoadNic}
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "pci", ncclTopoXmlLoadPci }, { "gpu", ncclTopoXmlLoadGpu }, { "nic", ncclTopoXmlLoadNic} };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 3));
  return ncclSuccess;
}

/* 中文内部逻辑 F0545：ncclTopoXmlLoadCpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct xmlHandler handlers[] = { { "pci", ncclTopoXmlLoadPci }, { "nic", ncclTopoXmlLoadNic } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 2));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0545
 */
/* 中文逐项-函数 F0545：ncclTopoXmlLoadCpu
 * 用途与位置：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlLoadCpu(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
/* 中文逐项-变量 V1737：handlers
 * handlers（struct xmlHandler；函数 ncclTopoXmlLoadCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "pci", ncclTopoXmlLoadPci }, { "nic", ncclTopoXmlLoadNic }
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "pci", ncclTopoXmlLoadPci }, { "nic", ncclTopoXmlLoadNic } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 2));
  return ncclSuccess;
}

/* 中文内部逻辑 F0546：ncclTopoXmlLoadSystem
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int version;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(head, "version", &version));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 3. 条件判断：(version != NCCL_TOPO_XML_VERSION)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：WARN("XML Topology has wrong version %d, %d needed", version, NCCL_TOPO_XML_VERSION);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 函数返回：return ncclInvalidUsage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 准备局部状态：const char* name;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(head, "name", &name));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 9. 条件判断：(name != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Loading topology %s", name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   12. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     13. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Loading unnamed topology");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 14. 准备局部状态：struct xmlHandler handlers[] = { { "cpu", ncclTopoXmlLoadCpu } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0546
 */
/* 中文逐项-函数 F0546：ncclTopoXmlLoadSystem
 * 用途与位置：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlGetAttr；xmlGetAttrInt；xmlLoadSub。
 * 需要对照的边界/推进条件：version != NCCL_TOPO_XML_VERSION；name != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlLoadSystem(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
/* 中文逐项-变量 V1738：version
 * version（int；函数 ncclTopoXmlLoadSystem 内）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(head, "version", &version));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int version;
  NCCLCHECK(xmlGetAttrInt(head, "version", &version));
  if (version != NCCL_TOPO_XML_VERSION) {
    WARN("XML Topology has wrong version %d, %d needed", version, NCCL_TOPO_XML_VERSION);
    return ncclInvalidUsage;
  }
/* 中文逐项-变量 V1739：name
 * name（char；函数 ncclTopoXmlLoadSystem 内）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(head, "name", &name));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* name;
  NCCLCHECK(xmlGetAttr(head, "name", &name));
  if (name != NULL) INFO(NCCL_GRAPH, "Loading topology %s", name);
  else INFO(NCCL_GRAPH, "Loading unnamed topology");

/* 中文逐项-变量 V1740：handlers
 * handlers（struct xmlHandler；函数 ncclTopoXmlLoadSystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "cpu", ncclTopoXmlLoadCpu } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "cpu", ncclTopoXmlLoadCpu } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));
  return ncclSuccess;
}

/* 中文内部逻辑 F0547：ncclTopoGetXmlFromFile
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：FILE* file = fopen(xmlTopoFile, "r");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(file == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 条件判断：(warn)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       5. 真分支：；仅条件为真执行以下子项。
 *         6. 调用 / 计算 / 写入：WARN("Could not open XML topology file %s : %s", xmlTopoFile, strerror(errno));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Loading topology file %s", xmlTopoFile);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 准备局部状态：struct xmlHandler handlers[] = { { "system", ncclTopoXmlLoadSystem } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 调用 / 计算 / 写入：xml->maxIndex = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, NULL, handlers, 1));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 12. 调用 / 计算 / 写入：fclose(file);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0547
 */
/* 中文逐项-函数 F0547：ncclTopoGetXmlFromFile
 * 用途与位置：打开拓扑文件并解析指定版本的系统 XML，按 warn 决定缺失文件的处理方式。
 * 参数 xmlTopoFile（const char* xmlTopoFile）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 warn（int warn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；fclose；fopen；strerror；xmlLoadSub。
 * 需要对照的边界/推进条件：file == NULL；warn。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetXmlFromFile(const char* xmlTopoFile, struct ncclXml* xml, int warn) {
/* 中文逐项-变量 V1741：file
 * file（FILE；函数 ncclTopoGetXmlFromFile 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 初值/来源 file = fopen(xmlTopoFile, "r")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  FILE* file = fopen(xmlTopoFile, "r");
  if (file == NULL) {
    if (warn) {
      WARN("Could not open XML topology file %s : %s", xmlTopoFile, strerror(errno));
    }
    return ncclSuccess;
  }
  INFO(NCCL_GRAPH, "Loading topology file %s", xmlTopoFile);
/* 中文逐项-变量 V1742：handlers
 * handlers（struct xmlHandler；函数 ncclTopoGetXmlFromFile 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "system", ncclTopoXmlLoadSystem } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "system", ncclTopoXmlLoadSystem } };
  xml->maxIndex = 0;
  NCCLCHECK(xmlLoadSub(file, xml, NULL, handlers, 1));
  fclose(file);
  return ncclSuccess;
}

/**********************/
/* XML creation       */
/* from autodetection */
/**********************/

/* 中文逐项-常量 X0098：BUSID_SIZE
 * 编译期定义：BUSID_SIZE = (sizeof("0000:00:00.0"))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define BUSID_SIZE (sizeof("0000:00:00.0"))
/* 中文逐项-常量 X0099：BUSID_REDUCED_SIZE
 * 编译期定义：BUSID_REDUCED_SIZE = (sizeof("0000:00"))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define BUSID_REDUCED_SIZE (sizeof("0000:00"))
/* 中文内部逻辑 F0548：memcpylower
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int i=0;；继续条件 i<size；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 调用 / 计算 / 写入：dst[i] = tolower(src[i]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0548
 */
/* 中文逐项-函数 F0548：memcpylower
 * 用途与位置：复制字符同时转为小写，规范 sysfs 路径/属性中的文本比较。
 * 参数 dst（char* dst）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 src（const char* src）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 size（const size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::getPciPath。
 * 本体直接出现的调用/包装：tolower。
 * 需要对照的边界/推进条件：i<size。各条件所属分支见紧邻函数体。
 */
static void memcpylower(char* dst, const char* src, const size_t size) {
  for (/* 中文逐项-变量 V1743：i
 * i（int；函数 memcpylower 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<size；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<size; i++) dst[i] = tolower(src[i]);
}
/* 中文内部逻辑 F0549：getPciPath
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char busPath[] = "/sys/class/pci_bus/0000:00/../../0000:00:00.0";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：memcpylower(busPath+sizeof("/sys/class/pci_bus/")-1, busId, BUSID_REDUCED_SIZE-1);；复制字符同时转为小写，规范 sysfs 路径/属性中的文本比较。
 * 3. 调用 / 计算 / 写入：memcpylower(busPath+sizeof("/sys/class/pci_bus/0000:00/../../")-1, busId, BUSID_SIZE-1);；复制字符同时转为小写，规范 sysfs 路径/属性中的文本比较。
 * 4. 调用 / 计算 / 写入：*path = realpath(busPath, NULL);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 条件判断：(*path == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：WARN("Could not find real path of %s", busPath);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     8. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0549
 */
/* 中文逐项-函数 F0549：getPciPath
 * 用途与位置：解析设备对应的系统 PCI 实际路径，供网卡/GPU 归属层级匹配。
 * 参数 busId（const char* busId）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 参数 path（char** path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoGetXmlFromGpu。
 * 本体直接出现的调用/包装：memcpylower；realpath。
 * 需要对照的边界/推进条件：*path == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getPciPath(const char* busId, char** path) {
/* 中文逐项-变量 V1744：busPath
 * busPath（char；函数 getPciPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 busPath = "/sys/class/pci_bus/0000:00/../../0000:00:00.0"；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  char busPath[] = "/sys/class/pci_bus/0000:00/../../0000:00:00.0";
  memcpylower(busPath+sizeof("/sys/class/pci_bus/")-1, busId, BUSID_REDUCED_SIZE-1);
  memcpylower(busPath+sizeof("/sys/class/pci_bus/0000:00/../../")-1, busId, BUSID_SIZE-1);
  *path = realpath(busPath, NULL);
  if (*path == NULL) {
    WARN("Could not find real path of %s", busPath);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0550：ncclTopoGetStrFromSys
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char filePath[PATH_MAX];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：sprintf(filePath, "%s/%s", path, fileName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 3. 准备局部状态：int offset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：FILE* file;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：((file = fopen(filePath, "r")) != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 循环：初始 ；继续条件 (feof(file) == 0 && ferror(file) == 0 && offset < MAX_STR_LEN)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       8. 准备局部状态：int len = fread(strValue+offset, 1, MAX_STR_LEN-offset, file);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       9. 调用 / 计算 / 写入：offset += len;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     11. 调用 / 计算 / 写入：fclose(file);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 12. 条件判断：(offset == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 调用 / 计算 / 写入：strValue[0] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     15. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Topology detection : could not read %s, ignoring", filePath);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   16. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     17. 调用 / 计算 / 写入：strValue[offset-1] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0550
 */
/* 中文逐项-函数 F0550：ncclTopoGetStrFromSys
 * 用途与位置：从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。
 * 参数 path（const char* path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 fileName（const char* fileName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 strValue（char* strValue）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoSetAttrFromSys；src/graph/xml.cc::ncclTopoGetXmlFromSys。
 * 本体直接出现的调用/包装：fclose；feof；ferror；fopen；fread；sprintf。
 * 需要对照的边界/推进条件：file = fopen(filePath, "r")) != NULL；feof(file) == 0 && ferror(file) == 0 && offset <
 * MAX_STR_LEN；offset == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetStrFromSys(const char* path, const char* fileName, char* strValue) {
/* 中文逐项-变量 V1745：filePath
 * filePath（char；函数 ncclTopoGetStrFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sprintf(filePath, "%s/%s", path, fileName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char filePath[PATH_MAX];
  sprintf(filePath, "%s/%s", path, fileName);
/* 中文逐项-变量 V1746：offset
 * offset（int；函数 ncclTopoGetStrFromSys 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int offset = 0;
/* 中文逐项-变量 V1747：file
 * file（FILE；函数 ncclTopoGetStrFromSys 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：if ((file = fopen(filePath, "r")) != NULL) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  FILE* file;
  if ((file = fopen(filePath, "r")) != NULL) {
    while (feof(file) == 0 && ferror(file) == 0 && offset < MAX_STR_LEN) {
/* 中文逐项-变量 V1748：len
 * len（int；函数 ncclTopoGetStrFromSys 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 len = fread(strValue+offset, 1, MAX_STR_LEN-offset, file)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      int len = fread(strValue+offset, 1, MAX_STR_LEN-offset, file);
      offset += len;
    }
    fclose(file);
  }
  if (offset == 0) {
    strValue[0] = '\0';
    INFO(NCCL_GRAPH, "Topology detection : could not read %s, ignoring", filePath);
  } else {
    strValue[offset-1] = '\0';
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0551：ncclTopoSetAttrFromSys
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char strValue[MAX_STR_LEN];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetStrFromSys(path, fileName, strValue));；失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。
 * 3. 条件判断：(strValue[0] != '\0')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(pciNode, attrName, strValue));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 6. 调用 / 计算 / 写入：TRACE(NCCL_GRAPH, "Read from sys %s/%s -> %s=%s", path, fileName, attrName, strValue);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0551
 */
/* 中文逐项-函数 F0551：ncclTopoSetAttrFromSys
 * 用途与位置：把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。
 * 参数 pciNode（struct ncclXmlNode* pciNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 path（const char* path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 fileName（const char* fileName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromCpu；src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoGetXmlFromGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoGetStrFromSys；xmlSetAttr。
 * 需要对照的边界/推进条件：strValue[0] != '\0'。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoSetAttrFromSys(struct ncclXmlNode* pciNode, const char* path, const char* fileName, const char* attrName) {
/* 中文逐项-变量 V1749：strValue
 * strValue（char；函数 ncclTopoSetAttrFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetStrFromSys(path, fileName, strValue));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char strValue[MAX_STR_LEN];
  NCCLCHECK(ncclTopoGetStrFromSys(path, fileName, strValue));
  if (strValue[0] != '\0') { NCCLCHECK(xmlSetAttr(pciNode, attrName, strValue)); }
  TRACE(NCCL_GRAPH, "Read from sys %s/%s -> %s=%s", path, fileName, attrName, strValue);
  return ncclSuccess;
}

/* 中文内部逻辑 F0552：ncclTopoGetXmlFromCpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(cpuNode, "affinity", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 3. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 准备局部状态：const char* numaId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(cpuNode, "numaid", &numaId));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 *     7. 条件判断：(numaId == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       8. 真分支：；仅条件为真执行以下子项。
 *         9. 调用 / 计算 / 写入：WARN("GetXmlFromCpu : could not find CPU numa ID.");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         10. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     11. 准备局部状态：char cpumaskPath[] = "/sys/devices/system/node/node0000";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     12. 调用 / 计算 / 写入：sprintf(cpumaskPath, "/sys/devices/system/node/node%s", numaId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     13. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSetAttrFromSys(cpuNode, cpumaskPath, "cpumap", "affinity"));；失败时可能提前return，成功才继续；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。
 * 14. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(cpuNode, "arch", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 15. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   16. 真分支：；仅条件为真执行以下子项。
 *     17. 编译期分支/指令：#if defined(__PPC__)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       18. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(cpuNode, "arch", "ppc64"));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *       19. 编译期分支/指令：#elif defined(__aarch64__)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *         20. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(cpuNode, "arch", "arm64"));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *         21. 编译期分支/指令：#elif defined(__x86_64__)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *           22. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(cpuNode, "arch", "x86_64"));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 23. 编译期分支/指令：#if defined(__x86_64__)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   24. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(cpuNode, "vendor", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 *   25. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     26. 真分支：；仅条件为真执行以下子项。
 *       27. 准备局部状态：union { struct { uint32_t ebx; uint32_t edx; uint32_t ecx; }; char vendor[12]; } cpuid0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       28. 调用 / 计算 / 写入：asm volatile("cpuid" : "=b" (cpuid0.ebx), "=c" (cpuid0.ecx), "=d" (cpuid0.edx) : "a" (0) : "memory");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       29. 准备局部状态：char vendor[13];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       30. 调用 / 计算 / 写入：strncpy(vendor, cpuid0.vendor, 12);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       31. 调用 / 计算 / 写入：vendor[12] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       32. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(cpuNode, "vendor", vendor));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *   33. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(cpuNode, "familyid", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 *   34. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     35. 真分支：；仅条件为真执行以下子项。
 *       36. 准备局部状态：union { struct { unsigned steppingId:4; unsigned modelId:4; unsigned familyId:4; unsigned processorType:2; unsigned resv0:2; unsigned extModelId:4; unsigned extFamilyId:8; unsigned resv1:4; }; uint32_ …（完整代码见函数体）；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       37. 调用 / 计算 / 写入：asm volatile("cpuid" : "=a" (cpuid1.val) : "a" (1) : "memory");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       38. 准备局部状态：int familyId = cpuid1.familyId + (cpuid1.extFamilyId << 4);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       39. 准备局部状态：int modelId = cpuid1.modelId + (cpuid1.extModelId << 4);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       40. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(cpuNode, "familyid", familyId));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 *       41. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(cpuNode, "modelid", modelId));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 42. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0552
 */
/* 中文逐项-函数 F0552：ncclTopoGetXmlFromCpu
 * 用途与位置：读取 CPU/NUMA 属性及亲和性，建立/补齐硬件 XML 的 CPU 层。
 * 参数 cpuNode（struct ncclXmlNode* cpuNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromSys。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoSetAttrFromSys；sprintf；strncpy；xmlGetAttr；xmlGetAttrIndex；xmlSetAttr；xmlSetAttrInt。
 * 需要对照的边界/推进条件：index == -1；numaId == NULL；index == -1；index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetXmlFromCpu(struct ncclXmlNode* cpuNode, struct ncclXml* xml) {
/* 中文逐项-变量 V1750：index
 * index（int；函数 ncclTopoGetXmlFromCpu 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(cpuNode, "affinity", &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(cpuNode, "affinity", &index));
  if (index == -1) {
/* 中文逐项-变量 V1751：numaId
 * numaId（char；函数 ncclTopoGetXmlFromCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(cpuNode, "numaid", &numaId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    const char* numaId;
    NCCLCHECK(xmlGetAttr(cpuNode, "numaid", &numaId));
    if (numaId == NULL) {
      WARN("GetXmlFromCpu : could not find CPU numa ID.");
      return ncclInternalError;
    }
    // Set affinity
/* 中文逐项-变量 V1752：cpumaskPath
 * cpumaskPath（char；函数 ncclTopoGetXmlFromCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cpumaskPath = "/sys/devices/system/node/node0000"；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    char cpumaskPath[] = "/sys/devices/system/node/node0000";
    sprintf(cpumaskPath, "/sys/devices/system/node/node%s", numaId);
    NCCLCHECK(ncclTopoSetAttrFromSys(cpuNode, cpumaskPath, "cpumap", "affinity"));
  }

  NCCLCHECK(xmlGetAttrIndex(cpuNode, "arch", &index));
  if (index == -1) {
    // Fill CPU type / vendor / model
#if defined(__PPC__)
    NCCLCHECK(xmlSetAttr(cpuNode, "arch", "ppc64"));
#elif defined(__aarch64__)
    NCCLCHECK(xmlSetAttr(cpuNode, "arch", "arm64"));
#elif defined(__x86_64__)
    NCCLCHECK(xmlSetAttr(cpuNode, "arch", "x86_64"));
#endif
  }

#if defined(__x86_64__)
  NCCLCHECK(xmlGetAttrIndex(cpuNode, "vendor", &index));
  if (index == -1) {
/* 中文逐项-变量 V1753：cpuid0
 * cpuid0（union { struct { // CPUID 0 String register order uint32_t ebx; uint32_t edx; uint32_t ecx; }; char
 * vendor[12] …（完整表达式见紧邻源码）；函数 ncclTopoGetXmlFromCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：asm volatile("cpuid" : "=b" (cpuid0.ebx), "=c" (cpuid0.ecx), "=d" (cpuid0.edx) : "a" (0) :
 * "memory");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0149：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：ebx、edx、ecx、vendor。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-类型 T0150：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：ebx、edx、ecx。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
      struct {
        // CPUID 0 String register order
/* 中文逐项-变量 V1754：ebx
 * ebx（uint32_t；成员 <匿名>::<匿名>）：CPUID指令返回寄存器分量，用于识别CPU厂商、型号和扩展属性。
 */
        uint32_t ebx;
/* 中文逐项-变量 V1755：edx
 * edx（uint32_t；成员 <匿名>::<匿名>）：CPUID指令返回寄存器分量，用于识别CPU厂商、型号和扩展属性。
 */
        uint32_t edx;
/* 中文逐项-变量 V1756：ecx
 * ecx（uint32_t；成员 <匿名>::<匿名>）：CPUID指令返回寄存器分量，用于识别CPU厂商、型号和扩展属性。
 */
        uint32_t ecx;
      };
/* 中文逐项-变量 V1757：vendor
 * vendor（char；成员 <匿名>）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 */
      char vendor[12];
    } cpuid0;

    asm volatile("cpuid" : "=b" (cpuid0.ebx), "=c" (cpuid0.ecx), "=d" (cpuid0.edx) : "a" (0) : "memory");
/* 中文逐项-变量 V1758：vendor
 * vendor（char；函数 ncclTopoGetXmlFromCpu 内）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 * 此处未显式给初值；后续首个文本使用点：strncpy(vendor, cpuid0.vendor, 12);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char vendor[13];
    strncpy(vendor, cpuid0.vendor, 12);
    vendor[12] = '\0';
    NCCLCHECK(xmlSetAttr(cpuNode, "vendor", vendor));
  }

  NCCLCHECK(xmlGetAttrIndex(cpuNode, "familyid", &index));
  if (index == -1) {
/* 中文逐项-变量 V1759：cpuid1
 * cpuid1（union { struct { unsigned steppingId:4; unsigned modelId:4; unsigned familyId:4; unsigned processorType:2;
 * uns …（完整表达式见紧邻源码）；函数 ncclTopoGetXmlFromCpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：asm volatile("cpuid" : "=a" (cpuid1.val) : "a" (1) : "memory");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0151：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：steppingId、modelId、familyId、processorType、resv0、extModelId、extFamilyId、resv1、val。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
    union {
/* 中文逐项-类型 T0152：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：steppingId、modelId、familyId、processorType、resv0、extModelId、extFamilyId、resv1。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
      struct {
/* 中文逐项-变量 V1760：steppingId
 * steppingId（unsigned；成员 <匿名>::<匿名>）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 */
        unsigned steppingId:4;
/* 中文逐项-变量 V1761：modelId
 * modelId（unsigned；成员 <匿名>::<匿名>）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 */
        unsigned modelId:4;
/* 中文逐项-变量 V1762：familyId
 * familyId（unsigned；成员 <匿名>::<匿名>）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 */
        unsigned familyId:4;
/* 中文逐项-变量 V1763：processorType
 * processorType（unsigned；成员 <匿名>::<匿名>）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 */
        unsigned processorType:2;
/* 中文逐项-变量 V1764：resv0
 * resv0（unsigned；成员 <匿名>::<匿名>）：ABI/硬件位域保留空间，保留大小和对齐，不能擅自解释为有效配置。
 */
        unsigned resv0:2;
/* 中文逐项-变量 V1765：extModelId
 * extModelId（unsigned；成员 <匿名>::<匿名>）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 */
        unsigned extModelId:4;
/* 中文逐项-变量 V1766：extFamilyId
 * extFamilyId（unsigned；成员 <匿名>::<匿名>）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 */
        unsigned extFamilyId:8;
/* 中文逐项-变量 V1767：resv1
 * resv1（unsigned；成员 <匿名>::<匿名>）：ABI/硬件位域保留空间，保留大小和对齐，不能擅自解释为有效配置。
 */
        unsigned resv1:4;
      };
/* 中文逐项-变量 V1768：val
 * val（uint32_t；成员 <匿名>）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 */
      uint32_t val;
    } cpuid1;
    asm volatile("cpuid" : "=a" (cpuid1.val) : "a" (1) : "memory");
/* 中文逐项-变量 V1769：familyId
 * familyId（int；函数 ncclTopoGetXmlFromCpu 内）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 * 初值/来源 familyId = cpuid1.familyId + (cpuid1.extFamilyId << 4)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int familyId = cpuid1.familyId + (cpuid1.extFamilyId << 4);
/* 中文逐项-变量 V1770：modelId
 * modelId（int；函数 ncclTopoGetXmlFromCpu 内）：CPUID型号位域，组合后决定CPU型号/架构策略与互连模型。
 * 初值/来源 modelId = cpuid1.modelId + (cpuid1.extModelId << 4)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int modelId = cpuid1.modelId + (cpuid1.extModelId << 4);
    NCCLCHECK(xmlSetAttrInt(cpuNode, "familyid", familyId));
    NCCLCHECK(xmlSetAttrInt(cpuNode, "modelid", modelId));
  }
#endif
  return ncclSuccess;
}

/* 中文内部逻辑 F0553：ncclTopoGetPciNode
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(xmlFindTagKv(xml, "pci", pciNode, "busid", busId));；失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 * 2. 条件判断：(*pciNode == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, NULL, "pci", pciNode));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 *     5. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(*pciNode, "busid", busId));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0553
 */
/* 中文逐项-函数 F0553：ncclTopoGetPciNode
 * 用途与位置：按 PCI 地址在 XML 中查找已有节点，避免同一设备被重复创建。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 busId（const char* busId）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 参数 pciNode（struct ncclXmlNode** pciNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoFillGpu；src/graph/xml.cc::ncclTopoFillNet。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlAddNode；xmlFindTagKv；xmlSetAttr。
 * 需要对照的边界/推进条件：*pciNode == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetPciNode(struct ncclXml* xml, const char* busId, struct ncclXmlNode** pciNode) {
  NCCLCHECK(xmlFindTagKv(xml, "pci", pciNode, "busid", busId));
  if (*pciNode == NULL) {
    NCCLCHECK(xmlAddNode(xml, NULL, "pci", pciNode));
    NCCLCHECK(xmlSetAttr(*pciNode, "busid", busId));
  }
  return ncclSuccess;
}

// Check whether a string is in BDF format or not.
// BDF (Bus-Device-Function) is "BBBB:BB:DD.F" where B, D and F are hex digits.
// There can be trailing chars.
/* 中文内部逻辑 F0554：isHex
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0554
 */
/* 中文逐项-函数 F0554：isHex
 * 用途与位置：检查字符是否为十六进制数字，辅助验证 PCI BDF 路径格式。
 * 参数 c（char c）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::checkBDFFormat。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));。CHECK类宏还可能提前返回/跳转。
 */
int isHex(char c) { return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')); }
/* 中文内部逻辑 F0555：checkBDFFormat
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(bdf[4] != ':' || bdf[7] != ':' || bdf[10] != '.')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 条件判断：(isHex(bdf[0]) == 0 || isHex(bdf[1] == 0) || isHex(bdf[2] == 0) || isHex(bdf[3] == 0) || isHex(bdf[5] == 0) || isHex(bdf[6] == 0) || isHex(bdf[8] == 0) || isHex(bdf[9] == 0) || isHex(bdf[11] == 0))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 函数返回：return 1;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0555
 */
/* 中文逐项-函数 F0555：checkBDFFormat
 * 用途与位置：验证 PCI domain:bus:device.function 文本格式，防止非设备目录被当作 PCI 节点。
 * 参数 bdf（char* bdf）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromSys。
 * 本体直接出现的调用/包装：isHex。
 * 需要对照的边界/推进条件：bdf[4] != ':' || bdf[7] != ':' || bdf[10] != '.'；isHex(bdf[0]) == 0 || isHex(bdf[1] == 0) ||
 * isHex(bdf[2] == 0) || isHex(bdf[3] == 0) || isHex(bdf[5] == 0) || isHex(bdf[6] == 0) || isHex(bdf[8]
 * …（完整表达式见紧邻源码）。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return 0;；return 1;。CHECK类宏还可能提前返回/跳转。
 */
int checkBDFFormat(char* bdf) {
  if (bdf[4] != ':' || bdf[7] != ':' || bdf[10] != '.') return 0;
  if (isHex(bdf[0]) == 0 || isHex(bdf[1] == 0) || isHex(bdf[2] == 0) || isHex(bdf[3] == 0) ||
      isHex(bdf[5] == 0) || isHex(bdf[6] == 0) || isHex(bdf[8] == 0) || isHex(bdf[9] == 0) ||
      isHex(bdf[11] == 0)) return 0;
  return 1;
}

/* 中文内部逻辑 F0556：ncclTopoGetXmlFromSys
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const char* busId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 3. 准备局部状态：char* path = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：ncclDebugNoWarn = NCCL_GRAPH;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：getPciPath(busId, &path);；解析设备对应的系统 PCI 实际路径，供网卡/GPU 归属层级匹配。
 * 6. 调用 / 计算 / 写入：ncclDebugNoWarn = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 条件判断：(path)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSetAttrFromSys(pciNode, path, "class", "class"));；失败时可能提前return，成功才继续；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。
 * 10. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 调用 / 计算 / 写入：ncclDebugNoWarn = NCCL_GRAPH;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(pciNode, "vendor", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 13. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 条件判断：(path)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       16. 真分支：；仅条件为真执行以下子项。
 *         17. 调用 / 计算 / 写入：ncclTopoSetAttrFromSys(pciNode, path, "vendor", "vendor");；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。
 * 18. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(pciNode, "device", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 19. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 条件判断：(path)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       22. 真分支：；仅条件为真执行以下子项。
 *         23. 调用 / 计算 / 写入：ncclTopoSetAttrFromSys(pciNode, path, "device", "device");；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。
 * 24. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(pciNode, "subsystem_vendor", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 25. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   26. 真分支：；仅条件为真执行以下子项。
 *     27. 条件判断：(path)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       28. 真分支：；仅条件为真执行以下子项。
 *         29. 调用 / 计算 / 写入：ncclTopoSetAttrFromSys(pciNode, path, "subsystem_vendor", "subsystem_vendor");；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。
 * 30. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(pciNode, "subsystem_device", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 31. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   32. 真分支：；仅条件为真执行以下子项。
 *     33. 条件判断：(path)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       34. 真分支：；仅条件为真执行以下子项。
 *         35. 调用 / 计算 / 写入：ncclTopoSetAttrFromSys(pciNode, path, "subsystem_device", "subsystem_device");；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。
 * 36. 调用 / 计算 / 写入：ncclDebugNoWarn = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 37. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(pciNode, "link_speed", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 38. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   39. 真分支：；仅条件为真执行以下子项。
 *     40. 条件判断：(path)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       41. 真分支：；仅条件为真执行以下子项。
 *         42. 准备局部状态：char deviceSpeedStr[MAX_STR_LEN];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         43. 准备局部状态：float deviceSpeed;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         44. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_speed", deviceSpeedStr));；失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。
 *         45. 调用 / 计算 / 写入：sscanf(deviceSpeedStr, "%f GT/s", &deviceSpeed);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         46. 准备局部状态：char portSpeedStr[MAX_STR_LEN];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         47. 准备局部状态：float portSpeed;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         48. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_speed", portSpeedStr));；失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。
 *         49. 调用 / 计算 / 写入：sscanf(portSpeedStr, "%f GT/s", &portSpeed);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         50. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(pciNode, "link_speed", portSpeed < deviceSpeed ? portSpeedStr : deviceSpeedStr));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *       51. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         52. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(pciNode, "link_speed", ""));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 53. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(pciNode, "link_width", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 54. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   55. 真分支：；仅条件为真执行以下子项。
 *     56. 条件判断：(path)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       57. 真分支：；仅条件为真执行以下子项。
 *         58. 准备局部状态：char strValue[MAX_STR_LEN];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         59. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_width", strValue));；失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。
 *         60. 准备局部状态：int deviceWidth = strtol(strValue, NULL, 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         61. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_width", strValue));；失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。
 *         62. 准备局部状态：int portWidth = strtol(strValue, NULL, 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         63. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(pciNode, "link_width", std::min(deviceWidth,portWidth)));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 *       64. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         65. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(pciNode, "link_width", ""));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 66. 准备局部状态：struct ncclXmlNode* parent = pciNode->parent;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 67. 条件判断：(parent == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   68. 真分支：；仅条件为真执行以下子项。
 *     69. 条件判断：(path)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       70. 真分支：；仅条件为真执行以下子项。
 *         71. 准备局部状态：char numaIdStr[MAX_STR_LEN];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         72. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetStrFromSys(path, "numa_node", numaIdStr));；失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。
 *         73. 准备局部状态：int slashCount = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         74. 准备局部状态：int parentOffset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         75. 循环：初始 parentOffset = strlen(path)-1；继续条件 parentOffset>0；每轮更新 parentOffset--；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           76. 条件判断：(path[parentOffset] == '/')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             77. 真分支：；仅条件为真执行以下子项。
 *               78. 调用 / 计算 / 写入：slashCount++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               79. 调用 / 计算 / 写入：path[parentOffset] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               80. 准备局部状态：int start = parentOffset - 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               81. 循环：初始 ；继续条件 (start>0 && path[start] != '/')；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                 82. 调用 / 计算 / 写入：start--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               83. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *               84. 条件判断：(checkBDFFormat(path+start+1) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 85. 真分支：；仅条件为真执行以下子项。
 *                   86. 准备局部状态：struct ncclXmlNode* topNode;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                   87. 调用 / 计算 / 写入：NCCLCHECK(xmlFindTag(xml, "system", &topNode));；失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 *                   88. 调用 / 计算 / 写入：NCCLCHECK(xmlGetSubKv(topNode, "cpu", &parent, "numaid", numaIdStr));；失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 *                   89. 条件判断：(parent == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     90. 真分支：；仅条件为真执行以下子项。
 *                       91. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, topNode, "cpu", &parent));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 *                       92. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(parent, "numaid", numaIdStr));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *                 93. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   94. 条件判断：(slashCount == 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     95. 真分支：；仅条件为真执行以下子项。
 *                       96. 循环：初始 int i = strlen(path)-1;；继续条件 i>0；每轮更新 i--；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                         97. 条件判断：(path[i] == '/')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                           98. 真分支：；仅条件为真执行以下子项。
 *                             99. 调用 / 计算 / 写入：NCCLCHECK(xmlFindTagKv(xml, "pci", &parent, "busid", path+i+1));；失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 *                             100. 条件判断：(parent == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                               101. 真分支：；仅条件为真执行以下子项。
 *                                 102. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, NULL, "pci", &parent));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 *                                 103. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(parent, "busid", path+i+1));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *                             104. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *                       105. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           106. 条件判断：(parent)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             107. 真分支：；仅条件为真执行以下子项。
 *               108. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         109. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       110. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         111. 调用 / 计算 / 写入：NCCLCHECK(xmlFindTagKv(xml, "cpu", &parent, "numaid", "-1"));；失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 *         112. 条件判断：(parent == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           113. 真分支：；仅条件为真执行以下子项。
 *             114. 准备局部状态：struct ncclXmlNode* topNode;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             115. 调用 / 计算 / 写入：NCCLCHECK(xmlFindTag(xml, "system", &topNode));；失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 *             116. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, topNode, "cpu", &parent));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 *             117. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(parent, "numaid", "-1"));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *             118. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromCpu(parent, xml));；失败时可能提前return，成功才继续；读取 CPU/NUMA 属性及亲和性，建立/补齐硬件 XML 的 CPU 层。
 *     119. 调用 / 计算 / 写入：pciNode->parent = parent;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     120. 调用 / 计算 / 写入：parent->subs[parent->nSubs++] = pciNode;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 121. 条件判断：(strcmp(parent->name, "pci") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   122. 真分支：；仅条件为真执行以下子项。
 *     123. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromSys(parent, xml));；失败时可能提前return，成功才继续；沿 sysfs 真实设备路径建立 PCI 父层级，并读取链路速度/宽度等属性。
 *   124. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     125. 条件判断：(strcmp(parent->name, "cpu") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       126. 真分支：；仅条件为真执行以下子项。
 *         127. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromCpu(parent, xml));；失败时可能提前return，成功才继续；读取 CPU/NUMA 属性及亲和性，建立/补齐硬件 XML 的 CPU 层。
 * 128. 调用 / 计算 / 写入：free(path);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 129. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0556
 */
/* 中文逐项-函数 F0556：ncclTopoGetXmlFromSys
 * 用途与位置：沿 sysfs 真实设备路径建立 PCI 父层级，并读取链路速度/宽度等属性。
 * 参数 pciNode（struct ncclXmlNode* pciNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoFillGpu；src/graph/xml.cc::ncclTopoFillNet。
 * 本体直接出现的调用/包装：NCCLCHECK；checkBDFFormat；free；getPciPath；ncclTopoGetStrFromSys；ncclTopoGetXmlFromCpu；ncclTopoGetXmlFromSys；ncclTopoSetAttrFromSys；sscanf；std::min；strcmp；strlen；strtol；xmlAddNode；xmlFindTag；xmlFindTagKv；xmlGetAttr；xmlGetAttrIndex；更多调用保留在函数体。
 * 需要对照的边界/推进条件：path；index == -1；path；index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetXmlFromSys(struct ncclXmlNode* pciNode, struct ncclXml* xml) {
  // Fill info, then parent
/* 中文逐项-变量 V1771：busId
 * busId（char；函数 ncclTopoGetXmlFromSys 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* busId;
  NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));
/* 中文逐项-变量 V1772：path
 * path（char；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 path = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  char* path = NULL;
  ncclDebugNoWarn = NCCL_GRAPH;
  getPciPath(busId, &path);
  ncclDebugNoWarn = 0;

  if (path) {
    NCCLCHECK(ncclTopoSetAttrFromSys(pciNode, path, "class", "class"));
  }
/* 中文逐项-变量 V1773：index
 * index（int；函数 ncclTopoGetXmlFromSys 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(pciNode, "vendor", &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  ncclDebugNoWarn = NCCL_GRAPH;
  NCCLCHECK(xmlGetAttrIndex(pciNode, "vendor", &index));
  if (index == -1) {
    if (path) ncclTopoSetAttrFromSys(pciNode, path, "vendor", "vendor");
  }
  NCCLCHECK(xmlGetAttrIndex(pciNode, "device", &index));
  if (index == -1) {
    if (path) ncclTopoSetAttrFromSys(pciNode, path, "device", "device");
  }
  NCCLCHECK(xmlGetAttrIndex(pciNode, "subsystem_vendor", &index));
  if (index == -1) {
    if (path) ncclTopoSetAttrFromSys(pciNode, path, "subsystem_vendor", "subsystem_vendor");
  }
  NCCLCHECK(xmlGetAttrIndex(pciNode, "subsystem_device", &index));
  if (index == -1) {
    if (path) ncclTopoSetAttrFromSys(pciNode, path, "subsystem_device", "subsystem_device");
  }
  ncclDebugNoWarn = 0;
  NCCLCHECK(xmlGetAttrIndex(pciNode, "link_speed", &index));
  if (index == -1) {
    if (path) {
/* 中文逐项-变量 V1774：deviceSpeedStr
 * deviceSpeedStr（char；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_speed",
 * deviceSpeedStr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char deviceSpeedStr[MAX_STR_LEN];
/* 中文逐项-变量 V1775：deviceSpeed
 * deviceSpeed（float；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sscanf(deviceSpeedStr, "%f GT/s", &deviceSpeed);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      float deviceSpeed;
      NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_speed", deviceSpeedStr));
      sscanf(deviceSpeedStr, "%f GT/s", &deviceSpeed);
/* 中文逐项-变量 V1776：portSpeedStr
 * portSpeedStr（char；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_speed",
 * portSpeedStr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char portSpeedStr[MAX_STR_LEN];
/* 中文逐项-变量 V1777：portSpeed
 * portSpeed（float；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sscanf(portSpeedStr, "%f GT/s", &portSpeed);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      float portSpeed;
      NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_speed", portSpeedStr));
      sscanf(portSpeedStr, "%f GT/s", &portSpeed);
      NCCLCHECK(xmlSetAttr(pciNode, "link_speed", portSpeed < deviceSpeed ? portSpeedStr : deviceSpeedStr));
    } else {
      NCCLCHECK(xmlSetAttr(pciNode, "link_speed", ""));
    }
  }
  NCCLCHECK(xmlGetAttrIndex(pciNode, "link_width", &index));
  if (index == -1) {
    if (path) {
/* 中文逐项-变量 V1778：strValue
 * strValue（char；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_width", strValue));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char strValue[MAX_STR_LEN];
      NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_width", strValue));
/* 中文逐项-变量 V1779：deviceWidth
 * deviceWidth（int；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 deviceWidth = strtol(strValue, NULL, 0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int deviceWidth = strtol(strValue, NULL, 0);
      NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_width", strValue));
/* 中文逐项-变量 V1780：portWidth
 * portWidth（int；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 portWidth = strtol(strValue, NULL, 0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int portWidth = strtol(strValue, NULL, 0);
      NCCLCHECK(xmlSetAttrInt(pciNode, "link_width", std::min(deviceWidth,portWidth)));
    } else {
      NCCLCHECK(xmlSetAttr(pciNode, "link_width", ""));
    }
  }
/* 中文逐项-变量 V1781：parent
 * parent（struct ncclXmlNode；函数 ncclTopoGetXmlFromSys 内）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 * 初值/来源 parent = pciNode->parent；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclXmlNode* parent = pciNode->parent;
  if (parent == NULL) {
    if (path) {
      // Save that for later in case next step is a CPU
/* 中文逐项-变量 V1782：numaIdStr
 * numaIdStr（char；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetStrFromSys(path, "numa_node", numaIdStr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char numaIdStr[MAX_STR_LEN];
      NCCLCHECK(ncclTopoGetStrFromSys(path, "numa_node", numaIdStr));

      // Go up one level in the PCI tree. Rewind two "/" and follow the upper PCI
      // switch, or stop if we reach a CPU root complex.
/* 中文逐项-变量 V1783：slashCount
 * slashCount（int；函数 ncclTopoGetXmlFromSys 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 slashCount = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int slashCount = 0;
/* 中文逐项-变量 V1784：parentOffset
 * parentOffset（int；函数 ncclTopoGetXmlFromSys 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 此处未显式给初值；后续首个文本使用点：for (parentOffset = strlen(path)-1; parentOffset>0; parentOffset--) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int parentOffset;
      for (parentOffset = strlen(path)-1; parentOffset>0; parentOffset--) {
        if (path[parentOffset] == '/') {
          slashCount++;
          path[parentOffset] = '\0';
/* 中文逐项-变量 V1785：start
 * start（int；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 start = parentOffset - 1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          int start = parentOffset - 1;
          while (start>0 && path[start] != '/') start--;
          // Check whether the parent path looks like "BBBB:BB:DD.F" or not.
          if (checkBDFFormat(path+start+1) == 0) {
            // This a CPU root complex. Create a CPU tag and stop there.
/* 中文逐项-变量 V1786：topNode
 * topNode（struct ncclXmlNode；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlFindTag(xml, "system", &topNode));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
            struct ncclXmlNode* topNode;
            NCCLCHECK(xmlFindTag(xml, "system", &topNode));
            NCCLCHECK(xmlGetSubKv(topNode, "cpu", &parent, "numaid", numaIdStr));
            if (parent == NULL) {
              NCCLCHECK(xmlAddNode(xml, topNode, "cpu", &parent));
              NCCLCHECK(xmlSetAttr(parent, "numaid", numaIdStr));
            }
          } else if (slashCount == 2) {
            // Continue on the upper PCI switch
            for (/* 中文逐项-变量 V1787：i
 * i（int；函数 ncclTopoGetXmlFromSys 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = strlen(path)-1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 * 所在for循环的继续条件：i>0；循环体与增量表达式决定每轮如何推进。
 */int i = strlen(path)-1; i>0; i--) {
              if (path[i] == '/') {
                NCCLCHECK(xmlFindTagKv(xml, "pci", &parent, "busid", path+i+1));
                if (parent == NULL) {
                  NCCLCHECK(xmlAddNode(xml, NULL, "pci", &parent));
                  NCCLCHECK(xmlSetAttr(parent, "busid", path+i+1));
                }
                break;
              }
            }
          }
        }
        if (parent) break;
      }
    } else {
      // No information on /sys, attach GPU to unknown CPU
      NCCLCHECK(xmlFindTagKv(xml, "cpu", &parent, "numaid", "-1"));
      if (parent == NULL) {
/* 中文逐项-变量 V1788：topNode
 * topNode（struct ncclXmlNode；函数 ncclTopoGetXmlFromSys 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlFindTag(xml, "system", &topNode));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        struct ncclXmlNode* topNode;
        NCCLCHECK(xmlFindTag(xml, "system", &topNode));
        NCCLCHECK(xmlAddNode(xml, topNode, "cpu", &parent));
        NCCLCHECK(xmlSetAttr(parent, "numaid", "-1"));
        NCCLCHECK(ncclTopoGetXmlFromCpu(parent, xml));
      }
    }
    pciNode->parent = parent;
    parent->subs[parent->nSubs++] = pciNode;
  }
  if (strcmp(parent->name, "pci") == 0) {
    NCCLCHECK(ncclTopoGetXmlFromSys(parent, xml));
  } else if (strcmp(parent->name, "cpu") == 0) {
    NCCLCHECK(ncclTopoGetXmlFromCpu(parent, xml));
  }
  free(path);
  return ncclSuccess;
}

/* 中文内部逻辑 F0557：ncclTopoGetXmlFromGpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclXmlNode* gpuNode = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetSub(pciNode, "gpu", &gpuNode));；失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 * 3. 条件判断：(gpuNode == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, pciNode, "gpu", &gpuNode));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 * 6. 准备局部状态：int index = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 准备局部状态：int dev = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(gpuNode, "dev", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 9. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 条件判断：(nvmlDev == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       12. 真分支：；仅条件为真执行以下子项。
 *         13. 准备局部状态：const char* busId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         14. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 *         15. 条件判断：(busId == NULL || cudaDeviceGetByPCIBusId(&dev, busId) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           16. 真分支：；仅条件为真执行以下子项。
 *             17. 调用 / 计算 / 写入：dev = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       18. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         19. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlDeviceGetIndex(nvmlDev, (unsigned int*)&dev));；失败时可能提前return，成功才继续；由句柄查询 NVML 序号；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 *     20. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(gpuNode, "dev", dev));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 21. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(gpuNode, "dev", &dev));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 22. 条件判断：(dev == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   23. 真分支：；仅条件为真执行以下子项。
 *     24. 调用 / 计算 / 写入：*gpuNodeRet = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     25. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 26. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(gpuNode, "sm", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 27. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   28. 真分支：；仅条件为真执行以下子项。
 *     29. 准备局部状态：int cudaMajor, cudaMinor;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     30. 条件判断：(nvmlDev == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       31. 真分支：；仅条件为真执行以下子项。
 *         32. 准备局部状态：cudaDeviceProp devProp;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         33. 调用 / 计算 / 写入：CUDACHECK(cudaGetDeviceProperties(&devProp, dev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *         34. 调用 / 计算 / 写入：cudaMajor = devProp.major;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         35. 调用 / 计算 / 写入：cudaMinor = devProp.minor;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       36. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         37. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlDeviceGetCudaComputeCapability(nvmlDev, &cudaMajor, &cudaMinor));；失败时可能提前return，成功才继续；查询 GPU 计算能力；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 *     38. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(gpuNode, "sm", cudaMajor*10+cudaMinor));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 39. 准备局部状态：int sm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 40. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(gpuNode, "sm", &sm));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 41. 准备局部状态：struct ncclXmlNode* nvlNode = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 42. 调用 / 计算 / 写入：NCCLCHECK(xmlGetSub(gpuNode, "nvlink", &nvlNode));；失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 * 43. 条件判断：(nvlNode == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   44. 真分支：；仅条件为真执行以下子项。
 *     45. 准备局部状态：int maxNvLinks = (sm < 60) ? 0 : (sm < 70) ? 4 : (sm < 80) ? 6 : 12;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     46. 条件判断：(maxNvLinks > 0 && nvmlDev == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       47. 真分支：；仅条件为真执行以下子项。
 *         48. 调用 / 计算 / 写入：WARN("No NVML device handle. Skipping nvlink detection.");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         49. 调用 / 计算 / 写入：maxNvLinks = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     50. 循环：初始 int l=0;；继续条件 l<maxNvLinks；每轮更新 ++l；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       51. 准备局部状态：unsigned canP2P;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       52. 条件判断：((ncclNvmlDeviceGetNvLinkCapability(nvmlDev, l, NVML_NVLINK_CAP_P2P_SUPPORTED, &canP2P) != ncclSuccess) || !canP2P)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         53. 真分支：；仅条件为真执行以下子项。
 *           54. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       55. 准备局部状态：nvmlEnableState_t isActive;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       56. 条件判断：((ncclNvmlDeviceGetNvLinkState(nvmlDev, l, &isActive) != ncclSuccess) || (isActive != NVML_FEATURE_ENABLED))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         57. 真分支：；仅条件为真执行以下子项。
 *           58. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       59. 准备局部状态：nvmlPciInfo_t remoteProc;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       60. 条件判断：(ncclNvmlDeviceGetNvLinkRemotePciInfo(nvmlDev, l, &remoteProc) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         61. 真分支：；仅条件为真执行以下子项。
 *           62. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       63. 准备局部状态：char* p = remoteProc.busId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       64. 准备局部状态：char lowerId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       65. 循环：初始 int c=0;；继续条件 c<NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         66. 调用 / 计算 / 写入：lowerId[c] = tolower(p[c]);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         67. 条件判断：(p[c] == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           68. 真分支：；仅条件为真执行以下子项。
 *             69. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *       70. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       71. 调用 / 计算 / 写入：NCCLCHECK(xmlGetSubKv(gpuNode, "nvlink", &nvlNode, "target", lowerId));；失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 *       72. 条件判断：(nvlNode == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         73. 真分支：；仅条件为真执行以下子项。
 *           74. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, gpuNode, "nvlink", &nvlNode));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 *           75. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(nvlNode, "target", lowerId));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *           76. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(nvlNode, "count", 1));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 *         77. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           78. 准备局部状态：int count;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           79. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(nvlNode, "count", &count));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 *           80. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrInt(nvlNode, "count", count+1));；失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 *     81. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 82. 循环：初始 int s=0;；继续条件 s<gpuNode->nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   83. 准备局部状态：struct ncclXmlNode* sub = gpuNode->subs[s];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   84. 条件判断：(strcmp(sub->name, "nvlink") != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     85. 真分支：；仅条件为真执行以下子项。
 *       86. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   87. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   88. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(sub, "tclass", &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 *   89. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     90. 真分支：；仅条件为真执行以下子项。
 *       91. 准备局部状态：const char* busId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       92. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(sub, "target", &busId));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 *       93. 准备局部状态：char* path;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       94. 调用 / 计算 / 写入：ncclDebugNoWarn = NCCL_GRAPH;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       95. 调用 / 计算 / 写入：getPciPath(busId, &path);；解析设备对应的系统 PCI 实际路径，供网卡/GPU 归属层级匹配。
 *       96. 调用 / 计算 / 写入：ncclDebugNoWarn = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       97. 条件判断：(path == NULL || strcmp(busId, "fffffff:ffff:ff") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         98. 真分支：；仅条件为真执行以下子项。
 *           99. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(sub, "tclass", "0x068000"));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 *         100. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           101. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSetAttrFromSys(sub, path, "class", "tclass"));；失败时可能提前return，成功才继续；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。
 *           102. 调用 / 计算 / 写入：free(path);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 103. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 104. 调用 / 计算 / 写入：*gpuNodeRet = gpuNode;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 105. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0557
 */
/* 中文逐项-函数 F0557：ncclTopoGetXmlFromGpu
 * 用途与位置：利用 CUDA/NVML 信息补齐 GPU 的 PCI、计算能力和 NVLink 属性。
 * 参数 pciNode（struct ncclXmlNode* pciNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nvmlDev（nvmlDevice_t nvmlDev）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 gpuNodeRet（struct ncclXmlNode** gpuNodeRet）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoFillGpu。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaDeviceGetByPCIBusId；cudaGetDeviceProperties；free；getPciPath；ncclNvmlDeviceGetCudaComputeCapability；ncclNvmlDeviceGetIndex；ncclNvmlDeviceGetNvLinkCapability；ncclNvmlDeviceGetNvLinkRemotePciInfo；ncclNvmlDeviceGetNvLinkState；ncclTopoSetAttrFromSys；strcmp；tolower；xmlAddNode；xmlGetAttr；xmlGetAttrIndex；xmlGetAttrInt；更多调用保留在函数体。
 * 需要对照的边界/推进条件：gpuNode == NULL；index == -1；nvmlDev == NULL；busId == NULL || cudaDeviceGetByPCIBusId(&dev, busId) !=
 * cudaSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetXmlFromGpu(struct ncclXmlNode* pciNode, nvmlDevice_t nvmlDev, struct ncclXml* xml, struct ncclXmlNode** gpuNodeRet) {
/* 中文逐项-变量 V1789：gpuNode
 * gpuNode（struct ncclXmlNode；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gpuNode = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclXmlNode* gpuNode = NULL;
  NCCLCHECK(xmlGetSub(pciNode, "gpu", &gpuNode));
  if (gpuNode == NULL) NCCLCHECK(xmlAddNode(xml, pciNode, "gpu", &gpuNode));

/* 中文逐项-变量 V1790：index
 * index（int；函数 ncclTopoGetXmlFromGpu 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 初值/来源 index = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int index = -1;

/* 中文逐项-变量 V1791：dev
 * dev（int；函数 ncclTopoGetXmlFromGpu 内）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 初值/来源 dev = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int dev = -1;
  NCCLCHECK(xmlGetAttrIndex(gpuNode, "dev", &index));
  if (index == -1) {
    if (nvmlDev == NULL) {
/* 中文逐项-变量 V1792：busId
 * busId（char；函数 ncclTopoGetXmlFromGpu 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      const char* busId;
      NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));
      if (busId == NULL || cudaDeviceGetByPCIBusId(&dev, busId) != cudaSuccess) dev = -1;
    } else {
      NCCLCHECK(ncclNvmlDeviceGetIndex(nvmlDev, (unsigned int*)&dev));
    }
    NCCLCHECK(xmlSetAttrInt(gpuNode, "dev", dev));
  }
  NCCLCHECK(xmlGetAttrInt(gpuNode, "dev", &dev));
  if (dev == -1) { *gpuNodeRet = NULL; return ncclSuccess; }

  NCCLCHECK(xmlGetAttrIndex(gpuNode, "sm", &index));
  if (index == -1) {
/* 中文逐项-变量 V1793：cudaMajor、cudaMinor
 * cudaMajor（int；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cudaMajor = devProp.major; cudaMinor = devProp.minor;。若由输出参数填入，必须先检查填充调用成功再读值。
 * cudaMinor（int；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cudaMajor = devProp.major; cudaMinor = devProp.minor;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int cudaMajor, cudaMinor;
    if (nvmlDev == NULL) {
/* 中文逐项-变量 V1794：devProp
 * devProp（cudaDeviceProp；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaGetDeviceProperties(&devProp, dev));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      cudaDeviceProp devProp;
      CUDACHECK(cudaGetDeviceProperties(&devProp, dev));
      cudaMajor = devProp.major; cudaMinor = devProp.minor;
    } else {
      NCCLCHECK(ncclNvmlDeviceGetCudaComputeCapability(nvmlDev, &cudaMajor, &cudaMinor));
    }
    NCCLCHECK(xmlSetAttrInt(gpuNode, "sm", cudaMajor*10+cudaMinor));
  }
/* 中文逐项-变量 V1795：sm
 * sm（int；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(gpuNode, "sm", &sm));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int sm;
  NCCLCHECK(xmlGetAttrInt(gpuNode, "sm", &sm));

/* 中文逐项-变量 V1796：nvlNode
 * nvlNode（struct ncclXmlNode；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvlNode = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclXmlNode* nvlNode = NULL;
  NCCLCHECK(xmlGetSub(gpuNode, "nvlink", &nvlNode));
  if (nvlNode == NULL) {
    // NVML NVLink detection
/* 中文逐项-变量 V1797：maxNvLinks
 * maxNvLinks（int；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxNvLinks = (sm < 60) ? 0 : (sm < 70) ? 4 : (sm < 80) ? 6 : 12；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int maxNvLinks = (sm < 60) ? 0 : (sm < 70) ? 4 : (sm < 80) ? 6 : 12;

    if (maxNvLinks > 0 && nvmlDev == NULL) {
      WARN("No NVML device handle. Skipping nvlink detection.");
      maxNvLinks = 0;
    }

    for (/* 中文逐项-变量 V1798：l
 * l（int；函数 ncclTopoGetXmlFromGpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 l = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：l<maxNvLinks；循环体与增量表达式决定每轮如何推进。
 */int l=0; l<maxNvLinks; ++l) {
      // Check whether we can use this NVLink for P2P
/* 中文逐项-变量 V1799：canP2P
 * canP2P（unsigned；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if ((ncclNvmlDeviceGetNvLinkCapability(nvmlDev, l, NVML_NVLINK_CAP_P2P_SUPPORTED, &canP2P) !=
 * ncclSuccess) || !canP2P) continue;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      unsigned canP2P;
      if ((ncclNvmlDeviceGetNvLinkCapability(nvmlDev, l, NVML_NVLINK_CAP_P2P_SUPPORTED, &canP2P) != ncclSuccess) || !canP2P) continue;

      // Make sure the Nvlink is up. The previous call should have trained the link.
/* 中文逐项-变量 V1800：isActive
 * isActive（nvmlEnableState_t；函数 ncclTopoGetXmlFromGpu 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 此处未显式给初值；后续首个文本使用点：if ((ncclNvmlDeviceGetNvLinkState(nvmlDev, l, &isActive) != ncclSuccess) || (isActive !=
 * NVML_FEATURE_ENABLED)) continue;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      nvmlEnableState_t isActive;
      if ((ncclNvmlDeviceGetNvLinkState(nvmlDev, l, &isActive) != ncclSuccess) || (isActive != NVML_FEATURE_ENABLED)) continue;

      // Try to figure out what's on the other side of the NVLink
/* 中文逐项-变量 V1801：remoteProc
 * remoteProc（nvmlPciInfo_t；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclNvmlDeviceGetNvLinkRemotePciInfo(nvmlDev, l, &remoteProc) != ncclSuccess)
 * continue;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      nvmlPciInfo_t remoteProc;
      if (ncclNvmlDeviceGetNvLinkRemotePciInfo(nvmlDev, l, &remoteProc) != ncclSuccess) continue;

      // Make a lower case copy of the bus ID for calling ncclDeviceType
      // PCI system path is in lower case
/* 中文逐项-变量 V1802：p
 * p（char；函数 ncclTopoGetXmlFromGpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = remoteProc.busId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      char* p = remoteProc.busId;
/* 中文逐项-变量 V1803：lowerId
 * lowerId（char；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：lowerId[c] = tolower(p[c]);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char lowerId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
      for (/* 中文逐项-变量 V1804：c
 * c（int；函数 ncclTopoGetXmlFromGpu 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE; c++) {
        lowerId[c] = tolower(p[c]);
        if (p[c] == 0) break;
      }

      NCCLCHECK(xmlGetSubKv(gpuNode, "nvlink", &nvlNode, "target", lowerId));
      if (nvlNode == NULL) {
        NCCLCHECK(xmlAddNode(xml, gpuNode, "nvlink", &nvlNode));
        NCCLCHECK(xmlSetAttr(nvlNode, "target", lowerId));
        NCCLCHECK(xmlSetAttrInt(nvlNode, "count", 1));
      } else {
/* 中文逐项-变量 V1805：count
 * count（int；函数 ncclTopoGetXmlFromGpu 内）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(nvlNode, "count", &count));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int count;
        NCCLCHECK(xmlGetAttrInt(nvlNode, "count", &count));
        NCCLCHECK(xmlSetAttrInt(nvlNode, "count", count+1));
      }
    }
  }
  // Fill target classes
  for (/* 中文逐项-变量 V1806：s
 * s（int；函数 ncclTopoGetXmlFromGpu 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<gpuNode->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<gpuNode->nSubs; s++) {
/* 中文逐项-变量 V1807：sub
 * sub（struct ncclXmlNode；函数 ncclTopoGetXmlFromGpu 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = gpuNode->subs[s]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* sub = gpuNode->subs[s];
    if (strcmp(sub->name, "nvlink") != 0) continue;
/* 中文逐项-变量 V1808：index
 * index（int；函数 ncclTopoGetXmlFromGpu 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(sub, "tclass", &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int index;
    NCCLCHECK(xmlGetAttrIndex(sub, "tclass", &index));
    if (index == -1) {
/* 中文逐项-变量 V1809：busId
 * busId（char；函数 ncclTopoGetXmlFromGpu 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(sub, "target", &busId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      const char* busId;
      NCCLCHECK(xmlGetAttr(sub, "target", &busId));
/* 中文逐项-变量 V1810：path
 * path（char；函数 ncclTopoGetXmlFromGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：getPciPath(busId, &path);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char* path;
      ncclDebugNoWarn = NCCL_GRAPH;
      getPciPath(busId, &path);
      ncclDebugNoWarn = 0;
      if (path == NULL || strcmp(busId, "fffffff:ffff:ff") == 0) {
        // Remote NVLink device is not visible inside this VM. Assume NVSwitch.
        NCCLCHECK(xmlSetAttr(sub, "tclass", "0x068000"));
      } else {
        NCCLCHECK(ncclTopoSetAttrFromSys(sub, path, "class", "tclass"));
        free(path);
      }
    }
  }
  *gpuNodeRet = gpuNode;
  return ncclSuccess;
}

/* 中文内部逻辑 F0558：ncclTopoFillGpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclXmlNode* node;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetPciNode(xml, busId, &node));；失败时可能提前return，成功才继续；按 PCI 地址在 XML 中查找已有节点，避免同一设备被重复创建。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrIfUnset(node, "class", "0x03"));；失败时可能提前return，成功才继续；只给缺失属性设置默认值，避免自动发现结果覆盖用户明确配置。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromSys(node, xml));；失败时可能提前return，成功才继续；沿 sysfs 真实设备路径建立 PCI 父层级，并读取链路速度/宽度等属性。
 * 5. 准备局部状态：nvmlDevice_t nvmlDev = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 条件判断：(ncclNvmlDeviceGetHandleByPciBusId(busId, &nvmlDev) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 调用 / 计算 / 写入：nvmlDev = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromGpu(node, nvmlDev, xml, gpuNode));；失败时可能提前return，成功才继续；利用 CUDA/NVML 信息补齐 GPU 的 PCI、计算能力和 NVLink 属性。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0558
 */
/* 中文逐项-函数 F0558：ncclTopoFillGpu
 * 用途与位置：在 XML 找到或添加指定 GPU，触发系统/NVML 自动发现补全其属性。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 busId（const char* busId）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 参数 gpuNode（struct ncclXmlNode** gpuNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNvmlDeviceGetHandleByPciBusId；ncclTopoGetPciNode；ncclTopoGetXmlFromGpu；ncclTopoGetXmlFromSys；xmlSetAttrIfUnset。
 * 需要对照的边界/推进条件：ncclNvmlDeviceGetHandleByPciBusId(busId, &nvmlDev) != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoFillGpu(struct ncclXml* xml, const char* busId, struct ncclXmlNode** gpuNode) {
/* 中文逐项-变量 V1811：node
 * node（struct ncclXmlNode；函数 ncclTopoFillGpu 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetPciNode(xml, busId, &node));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclXmlNode* node;
  NCCLCHECK(ncclTopoGetPciNode(xml, busId, &node));
  NCCLCHECK(xmlSetAttrIfUnset(node, "class", "0x03"));
  NCCLCHECK(ncclTopoGetXmlFromSys(node, xml));
/* 中文逐项-变量 V1812：nvmlDev
 * nvmlDev（nvmlDevice_t；函数 ncclTopoFillGpu 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nvmlDev = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  nvmlDevice_t nvmlDev = NULL;
  if (ncclNvmlDeviceGetHandleByPciBusId(busId, &nvmlDev) != ncclSuccess) nvmlDev = NULL;
  NCCLCHECK(ncclTopoGetXmlFromGpu(node, nvmlDev, xml, gpuNode));
  return ncclSuccess;
}

// Returns the subsystem name of a path, i.e. the end of the path
// where sysPath/subsystem points to.
/* 中文内部逻辑 F0559：ncclTopoGetSubsystem
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char subSysPath[PATH_MAX];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：sprintf(subSysPath, "%s/subsystem", sysPath);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 3. 准备局部状态：char* path = realpath(subSysPath, NULL);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 条件判断：(path == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：subSys[0] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   7. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     8. 准备局部状态：int offset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 循环：初始 offset = strlen(path)；继续条件 offset > 0 && path[offset] != '/'；每轮更新 offset--；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       10. 调用 / 计算 / 写入：;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *     11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     12. 调用 / 计算 / 写入：strcpy(subSys, path+offset+1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     13. 调用 / 计算 / 写入：free(path);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 14. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0559
 */
/* 中文逐项-函数 F0559：ncclTopoGetSubsystem
 * 用途与位置：读取 PCI 子系统标识，辅助识别设备类别与硬件拓扑特殊处理。
 * 参数 sysPath（const char* sysPath）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 subSys（char* subSys）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoFillNet。
 * 本体直接出现的调用/包装：free；realpath；sprintf；strcpy；strlen。
 * 需要对照的边界/推进条件：path == NULL；offset > 0 && path[offset] != '/'。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetSubsystem(const char* sysPath, char* subSys) {
/* 中文逐项-变量 V1813：subSysPath
 * subSysPath（char；函数 ncclTopoGetSubsystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sprintf(subSysPath, "%s/subsystem", sysPath);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char subSysPath[PATH_MAX];
  sprintf(subSysPath, "%s/subsystem", sysPath);
/* 中文逐项-变量 V1814：path
 * path（char；函数 ncclTopoGetSubsystem 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 path = realpath(subSysPath, NULL)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* path = realpath(subSysPath, NULL);
  if (path == NULL) {
    subSys[0] = '\0';
  } else {
/* 中文逐项-变量 V1815：offset
 * offset（int；函数 ncclTopoGetSubsystem 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：for (offset = strlen(path); offset > 0 && path[offset] != '/';
 * offset--);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int offset;
    for (offset = strlen(path); offset > 0 && path[offset] != '/'; offset--);
    strcpy(subSys, path+offset+1);
    free(path);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0560：ncclTopoFillNet
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(xmlFindTagKv(xml, "net", netNode, "name", netName));；失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 * 2. 条件判断：(*netNode != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：const char* pciSysPath = pciPath;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 条件判断：(pciSysPath)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 准备局部状态：char subSystem[PATH_MAX];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetSubsystem(pciSysPath, subSystem));；失败时可能提前return，成功才继续；读取 PCI 子系统标识，辅助识别设备类别与硬件拓扑特殊处理。
 *     10. 条件判断：(strcmp(subSystem, "pci") != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       11. 真分支：；仅条件为真执行以下子项。
 *         12. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Topology detection: network path %s is not a PCI device (%s). Attaching to first CPU", pciSysPath, subSystem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         13. 调用 / 计算 / 写入：pciSysPath = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 准备局部状态：struct ncclXmlNode* parent = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 条件判断：(pciSysPath)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   16. 真分支：；仅条件为真执行以下子项。
 *     17. 准备局部状态：int offset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     18. 循环：初始 offset=strlen(pciSysPath)-1；继续条件 pciSysPath[offset] != '/'；每轮更新 offset--；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       19. 调用 / 计算 / 写入：;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *     20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     21. 准备局部状态：char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     22. 调用 / 计算 / 写入：strcpy(busId, pciSysPath+offset+1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     23. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetPciNode(xml, busId, &parent));；失败时可能提前return，成功才继续；按 PCI 地址在 XML 中查找已有节点，避免同一设备被重复创建。
 *     24. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttrIfUnset(parent, "class", "0x02"));；失败时可能提前return，成功才继续；只给缺失属性设置默认值，避免自动发现结果覆盖用户明确配置。
 *     25. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetXmlFromSys(parent, xml));；失败时可能提前return，成功才继续；沿 sysfs 真实设备路径建立 PCI 父层级，并读取链路速度/宽度等属性。
 *   26. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     27. 调用 / 计算 / 写入：NCCLCHECK(xmlFindTag(xml, "cpu", &parent));；失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 * 28. 准备局部状态：struct ncclXmlNode* nicNode = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 29. 调用 / 计算 / 写入：NCCLCHECK(xmlGetSub(parent, "nic", &nicNode));；失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 * 30. 条件判断：(nicNode == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   31. 真分支：；仅条件为真执行以下子项。
 *     32. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, parent, "nic", &nicNode));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 * 33. 调用 / 计算 / 写入：NCCLCHECK(xmlAddNode(xml, nicNode, "net", netNode));；失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 * 34. 调用 / 计算 / 写入：NCCLCHECK(xmlSetAttr(*netNode, "name", netName));；失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 35. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0560
 */
/* 中文逐项-函数 F0560：ncclTopoFillNet
 * 用途与位置：将后端提供的网络设备属性补入 XML，关联其 PCI/NIC 层次与 GDR/CollNet 能力。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 pciPath（const char* pciPath）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 参数 netName（const char* netName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 netNode（struct ncclXmlNode** netNode）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoGetPciNode；ncclTopoGetSubsystem；ncclTopoGetXmlFromSys；strcmp；strcpy；strlen；xmlAddNode；xmlFindTag；xmlFindTagKv；xmlGetSub；xmlSetAttr；xmlSetAttrIfUnset。
 * 需要对照的边界/推进条件：*netNode != NULL；pciSysPath；strcmp(subSystem, "pci") != 0；pciSysPath。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoFillNet(struct ncclXml* xml, const char* pciPath, const char* netName, struct ncclXmlNode** netNode) {
  NCCLCHECK(xmlFindTagKv(xml, "net", netNode, "name", netName));
  if (*netNode != NULL) return ncclSuccess;

/* 中文逐项-变量 V1816：pciSysPath
 * pciSysPath（char；函数 ncclTopoFillNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pciSysPath = pciPath；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const char* pciSysPath = pciPath;
  if (pciSysPath) {
/* 中文逐项-变量 V1817：subSystem
 * subSystem（char；函数 ncclTopoFillNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetSubsystem(pciSysPath, subSystem));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char subSystem[PATH_MAX];
    NCCLCHECK(ncclTopoGetSubsystem(pciSysPath, subSystem));
    // This is not a PCI device (virtual, usb, ...).
    if (strcmp(subSystem, "pci") != 0) {
      INFO(NCCL_GRAPH, "Topology detection: network path %s is not a PCI device (%s). Attaching to first CPU", pciSysPath, subSystem);
      pciSysPath = NULL;
    }
  }

/* 中文逐项-变量 V1818：parent
 * parent（struct ncclXmlNode；函数 ncclTopoFillNet 内）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 * 初值/来源 parent = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclXmlNode* parent = NULL;
  if (pciSysPath) {
/* 中文逐项-变量 V1819：offset
 * offset（int；函数 ncclTopoFillNet 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 此处未显式给初值；后续首个文本使用点：for (offset=strlen(pciSysPath)-1; pciSysPath[offset] != '/'; offset--);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int offset;
    for (offset=strlen(pciSysPath)-1; pciSysPath[offset] != '/'; offset--);
/* 中文逐项-变量 V1820：busId
 * busId（char；函数 ncclTopoFillNet 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 此处未显式给初值；后续首个文本使用点：strcpy(busId, pciSysPath+offset+1);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
    strcpy(busId, pciSysPath+offset+1);
    NCCLCHECK(ncclTopoGetPciNode(xml, busId, &parent));
    NCCLCHECK(xmlSetAttrIfUnset(parent, "class", "0x02"));
    NCCLCHECK(ncclTopoGetXmlFromSys(parent, xml));
  } else {
    // Virtual NIC, no PCI device, attach to first CPU
    NCCLCHECK(xmlFindTag(xml, "cpu", &parent));
  }

/* 中文逐项-变量 V1821：nicNode
 * nicNode（struct ncclXmlNode；函数 ncclTopoFillNet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nicNode = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclXmlNode* nicNode = NULL;
  NCCLCHECK(xmlGetSub(parent, "nic", &nicNode));
  if (nicNode == NULL) {
    NCCLCHECK(xmlAddNode(xml, parent, "nic", &nicNode));
  }

  // We know that this net does not exist yet (we searched for it at the
  // beginning of this function), so we can add it.
  NCCLCHECK(xmlAddNode(xml, nicNode, "net", netNode));
  NCCLCHECK(xmlSetAttr(*netNode, "name", netName));
  return ncclSuccess;
}

/* 中文内部逻辑 F0561：ncclTopoTrimXmlRec
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const char* str;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(node, "keep", &str));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 3. 条件判断：(str && strcmp(str, "1") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：NCCLCHECK(xmlUnsetAttr(node, "keep"));；失败时可能提前return，成功才继续；删除指定属性并整理数组，防止遗留键值参与之后的匹配/导出。
 *   6. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     7. 准备局部状态：struct ncclXmlNode* subs[MAX_SUBS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     8. 准备局部状态：int nSubs = node->nSubs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     9. 调用 / 计算 / 写入：memcpy(subs, node->subs, node->nSubs*sizeof(struct ncclXmlNode*));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     10. 循环：初始 int s=0;；继续条件 s<nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       11. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoTrimXmlRec(subs[s]));；失败时可能提前return，成功才继续；裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。
 *     12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     13. 条件判断：(node->nSubs == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       14. 真分支：；仅条件为真执行以下子项。
 *         15. 调用 / 计算 / 写入：NCCLCHECK(xmlRemoveNode(node));；失败时可能提前return，成功才继续；从父节点子项表摘除目标节点并整理关系，裁剪不需要的拓扑分支。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0561
 */
/* 中文逐项-函数 F0561：ncclTopoTrimXmlRec
 * 用途与位置：裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoTrimXmlRec；src/graph/xml.cc::ncclTopoTrimXml。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclTopoTrimXmlRec；strcmp；xmlGetAttr；xmlRemoveNode；xmlUnsetAttr。
 * 需要对照的边界/推进条件：str && strcmp(str, "1") == 0；s<nSubs；node->nSubs == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoTrimXmlRec(struct ncclXmlNode* node) {
/* 中文逐项-变量 V1822：str
 * str（char；函数 ncclTopoTrimXmlRec 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(node, "keep", &str));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;
  NCCLCHECK(xmlGetAttr(node, "keep", &str));
  if (str && strcmp(str, "1") == 0) {
    NCCLCHECK(xmlUnsetAttr(node, "keep"));
  } else {
    // Copy nSubs and subs as they could change as we trim recursively.
/* 中文逐项-变量 V1823：subs
 * subs（struct ncclXmlNode；函数 ncclTopoTrimXmlRec 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 此处未显式给初值；后续首个文本使用点：memcpy(subs, node->subs, node->nSubs*sizeof(struct ncclXmlNode*));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclXmlNode* subs[MAX_SUBS];
/* 中文逐项-变量 V1824：nSubs
 * nSubs（int；函数 ncclTopoTrimXmlRec 内）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 * 初值/来源 nSubs = node->nSubs；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int nSubs = node->nSubs;
    memcpy(subs, node->subs, node->nSubs*sizeof(struct ncclXmlNode*));
    for (/* 中文逐项-变量 V1825：s
 * s（int；函数 ncclTopoTrimXmlRec 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<nSubs; s++) {
      NCCLCHECK(ncclTopoTrimXmlRec(subs[s]));
    }
    if (node->nSubs == 0) NCCLCHECK(xmlRemoveNode(node));
  }
  return ncclSuccess;
}
/* 中文内部逻辑 F0562：ncclTopoTrimXml
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoTrimXmlRec(xml->nodes));；失败时可能提前return，成功才继续；裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0562
 */
/* 中文逐项-函数 F0562：ncclTopoTrimXml
 * 用途与位置：裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTopoTrimXmlRec。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoTrimXml(struct ncclXml* xml) {
  NCCLCHECK(ncclTopoTrimXmlRec(xml->nodes));
  return ncclSuccess;
}

/**************************************************/
/* Parser rules for the user-defined graph search */
/**************************************************/

/* 中文内部逻辑 F0563：ncclTopoXmlGraphLoadGpu
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0563
 */
/* 中文逐项-函数 F0563：ncclTopoXmlGraphLoadGpu
 * 用途与位置：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlGraphLoadGpu(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
  NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));
  return ncclSuccess;
}

/* 中文内部逻辑 F0564：ncclTopoXmlGraphLoadNet
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0564
 */
/* 中文逐项-函数 F0564：ncclTopoXmlGraphLoadNet
 * 用途与位置：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlGraphLoadNet(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
  NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));
  return ncclSuccess;
}

/* 中文内部逻辑 F0565：ncclTopoXmlGraphLoadChannel
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct xmlHandler handlers[] = { { "net", ncclTopoXmlGraphLoadNet }, { "gpu", ncclTopoXmlGraphLoadGpu } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 2));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0565
 */
/* 中文逐项-函数 F0565：ncclTopoXmlGraphLoadChannel
 * 用途与位置：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlGraphLoadChannel(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
/* 中文逐项-变量 V1826：handlers
 * handlers（struct xmlHandler；函数 ncclTopoXmlGraphLoadChannel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "net", ncclTopoXmlGraphLoadNet }, { "gpu", ncclTopoXmlGraphLoadGpu }
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "net", ncclTopoXmlGraphLoadNet }, { "gpu", ncclTopoXmlGraphLoadGpu } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 2));
  return ncclSuccess;
}

/* 中文内部逻辑 F0566：ncclTopoXmlGraphLoadGraph
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct xmlHandler handlers[] = { { "channel", ncclTopoXmlGraphLoadChannel } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0566
 */
/* 中文逐项-函数 F0566：ncclTopoXmlGraphLoadGraph
 * 用途与位置：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlLoadSub。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlGraphLoadGraph(FILE* file, struct ncclXml* xml, struct ncclXmlNode* head) {
/* 中文逐项-变量 V1827：handlers
 * handlers（struct xmlHandler；函数 ncclTopoXmlGraphLoadGraph 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "channel", ncclTopoXmlGraphLoadChannel } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "channel", ncclTopoXmlGraphLoadChannel } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));
  return ncclSuccess;
}

/* 中文内部逻辑 F0567：ncclTopoXmlGraphLoadGraphs
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int version;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrInt(head, "version", &version));；失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 3. 条件判断：(version != NCCL_GRAPH_XML_VERSION)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：WARN("XML Graph has wrong version %d, %d needed", version, NCCL_GRAPH_XML_VERSION);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 函数返回：return ncclInvalidUsage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 准备局部状态：const char* name;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(head, "name", &name));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 9. 条件判断：(name != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Loading graphs for topology %s", name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   12. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     13. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Loading graphs");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 14. 准备局部状态：struct xmlHandler handlers[] = { { "graph", ncclTopoXmlGraphLoadGraph } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xmlGraph, head, handlers, 1));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0567
 */
/* 中文逐项-函数 F0567：ncclTopoXmlGraphLoadGraphs
 * 用途与位置：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。
 * 参数 file（FILE* file）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 参数 xmlGraph（struct ncclXml* xmlGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 head（struct ncclXmlNode* head）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlGetAttr；xmlGetAttrInt；xmlLoadSub。
 * 需要对照的边界/推进条件：version != NCCL_GRAPH_XML_VERSION；name != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoXmlGraphLoadGraphs(FILE* file, struct ncclXml* xmlGraph, struct ncclXmlNode* head) {
/* 中文逐项-变量 V1828：version
 * version（int；函数 ncclTopoXmlGraphLoadGraphs 内）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrInt(head, "version", &version));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int version;
  NCCLCHECK(xmlGetAttrInt(head, "version", &version));
  if (version != NCCL_GRAPH_XML_VERSION) {
    WARN("XML Graph has wrong version %d, %d needed", version, NCCL_GRAPH_XML_VERSION);
    return ncclInvalidUsage;
  }
/* 中文逐项-变量 V1829：name
 * name（char；函数 ncclTopoXmlGraphLoadGraphs 内）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(head, "name", &name));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* name;
  NCCLCHECK(xmlGetAttr(head, "name", &name));
  if (name != NULL) INFO(NCCL_GRAPH, "Loading graphs for topology %s", name);
  else INFO(NCCL_GRAPH, "Loading graphs");

/* 中文逐项-变量 V1830：handlers
 * handlers（struct xmlHandler；函数 ncclTopoXmlGraphLoadGraphs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "graph", ncclTopoXmlGraphLoadGraph } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "graph", ncclTopoXmlGraphLoadGraph } };
  NCCLCHECK(xmlLoadSub(file, xmlGraph, head, handlers, 1));
  return ncclSuccess;
}

/* 中文内部逻辑 F0568：ncclTopoGetXmlGraphFromFile
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：FILE* file = fopen(xmlGraphFile, "r");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(file == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("Could not open XML graph file %s : %s", xmlGraphFile, strerror(errno));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 准备局部状态：struct xmlHandler handlers[] = { { "graphs", ncclTopoXmlGraphLoadGraphs } };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：xml->maxIndex = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(xmlLoadSub(file, xml, NULL, handlers, 1));；失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。
 * 9. 调用 / 计算 / 写入：fclose(file);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_cc.md#f0568
 */
/* 中文逐项-函数 F0568：ncclTopoGetXmlGraphFromFile
 * 用途与位置：打开并解析通信图 XML 文件，校验图版本以恢复预先保存的搜索结果。
 * 参数 xmlGraphFile（const char* xmlGraphFile）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoCompute。
 * 本体直接出现的调用/包装：NCCLCHECK；fclose；fopen；strerror；xmlLoadSub。
 * 需要对照的边界/推进条件：file == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetXmlGraphFromFile(const char* xmlGraphFile, struct ncclXml* xml) {
/* 中文逐项-变量 V1831：file
 * file（FILE；函数 ncclTopoGetXmlGraphFromFile 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 初值/来源 file = fopen(xmlGraphFile, "r")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  FILE* file = fopen(xmlGraphFile, "r");
  if (file == NULL) {
    WARN("Could not open XML graph file %s : %s", xmlGraphFile, strerror(errno));
    return ncclSystemError;
  }
/* 中文逐项-变量 V1832：handlers
 * handlers（struct xmlHandler；函数 ncclTopoGetXmlGraphFromFile 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 handlers = { { "graphs", ncclTopoXmlGraphLoadGraphs } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct xmlHandler handlers[] = { { "graphs", ncclTopoXmlGraphLoadGraphs } };
  xml->maxIndex = 0;
  NCCLCHECK(xmlLoadSub(file, xml, NULL, handlers, 1));
  fclose(file);
  return ncclSuccess;
}
