/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "core.h"
#include "devcomm.h"
#include "comm.h"
#include "topo.h"

/* 中文逐项-参数getter P0018：Nthreads
 * 生成函数 ncclParamNthreads()，读取 NCCL_NTHREADS，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(Nthreads, "NTHREADS", -2);
/* 中文逐项-参数getter P0019：Ll128Nthreads
 * 生成函数 ncclParamLl128Nthreads()，读取 NCCL_LL128_NTHREADS，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(Ll128Nthreads, "LL128_NTHREADS", -2);

/* 中文内部逻辑 F0528：getNthreads
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int nt = env;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(nt > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 条件判断：(nt % WARP_SIZE != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       5. 真分支：；仅条件为真执行以下子项。
 *         6. 调用 / 计算 / 写入：WARN("Invalid %s %d (must be a multiple of %d)", name, nt, WARP_SIZE);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         7. 调用 / 计算 / 写入：nt = max;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       8. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         9. 条件判断：(nt > max)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           10. 真分支：；仅条件为真执行以下子项。
 *             11. 调用 / 计算 / 写入：WARN("Invalid %s %d (maximum %d).", name, nt, max);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             12. 调用 / 计算 / 写入：nt = max;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           13. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             14. 条件判断：(nt < min)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               15. 真分支：；仅条件为真执行以下子项。
 *                 16. 调用 / 计算 / 写入：WARN("Invalid %s %d (minimum %d).", name, nt, min);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 17. 调用 / 计算 / 写入：nt = min;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   18. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     19. 调用 / 计算 / 写入：nt = def;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 函数返回：return nt;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__tuning_cc.md#f0528
 */
/* 中文逐项-函数 F0528：getNthreads
 * 用途与位置：读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。
 * 参数 name（const char* name）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 参数 env（int env）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 min（int min）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 max（int max）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 def（int def）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/tuning.cc::ncclTopoTuneModel。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：nt > 0；nt % WARP_SIZE != 0；nt > max；nt < min。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return nt;。CHECK类宏还可能提前返回/跳转。
 */
static int getNthreads(const char* name, int env, int min, int max, int def) {
/* 中文逐项-变量 V1649：nt
 * nt（int；函数 getNthreads 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nt = env；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int nt = env;
  if (nt > 0) {
    if (nt % WARP_SIZE != 0) {
      WARN("Invalid %s %d (must be a multiple of %d)", name, nt, WARP_SIZE);
      nt = max;
    } else if (nt > max) {
      WARN("Invalid %s %d (maximum %d).", name, nt, max);
      nt = max;
    } else if (nt < min) {
      WARN("Invalid %s %d (minimum %d).", name, nt, min);
      nt = min;
     }
  } else {
    nt = def;
  }
  return nt;
}

/* 中文内部逻辑 F0529：parseList
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int def, set;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(str[0] == '^')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：def = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     5. 调用 / 计算 / 写入：set = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：str++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   7. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     8. 调用 / 计算 / 写入：def = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 调用 / 计算 / 写入：set = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 循环：初始 int i=0;；继续条件 i<nelems；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   11. 调用 / 计算 / 写入：list[i] = def;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 准备局部状态：char* tokStr = strdup(str);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 准备局部状态：char* tmpStr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 准备局部状态：char* token = strtok_r(tokStr, ",", &tmpStr);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 16. 循环：初始 ；继续条件 (token)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   17. 循环：初始 int i=0;；继续条件 i<nelems；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     18. 条件判断：(strcasecmp(token, elems[i]) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       19. 真分支：；仅条件为真执行以下子项。
 *         20. 调用 / 计算 / 写入：list[i] = set;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   21. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   22. 调用 / 计算 / 写入：token = strtok_r(NULL, ",", &tmpStr);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 23. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 24. 调用 / 计算 / 写入：free(tokStr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 25. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__tuning_cc.md#f0529
 */
/* 中文逐项-函数 F0529：parseList
 * 用途与位置：解析算法/协议等配置名单和排除前缀，生成各候选是否启用的布尔表。
 * 参数 str（const char* str）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 参数 elems（const char* elems[]）：固定大小 work union 的不同元素布局，必须先看 header.type 再解释内存。
 * 参数 nelems（int nelems）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 list（int* list）：节点或链路指针列表，count给出有效条目数，遍历不得越过当前有效范围。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/tuning.cc::ncclTopoTuneModel。
 * 本体直接出现的调用/包装：free；strcasecmp；strdup；strtok_r。
 * 需要对照的边界/推进条件：str[0] == '^'；i<nelems；token；i<nelems。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t parseList(const char* str, const char* elems[], int nelems, int* list) {
/* 中文逐项-变量 V1650：def、set
 * def（int；函数 parseList 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：def = 1; set = 0; str++;。若由输出参数填入，必须先检查填充调用成功再读值。
 * set（int；函数 parseList 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：def = 1; set = 0; str++;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int def, set;
  if (str[0] == '^') {
    def = 1; set = 0; str++;
  } else {
    def = 0; set = 1;
  }
  for (/* 中文逐项-变量 V1651：i
 * i（int；函数 parseList 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nelems；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nelems; i++) list[i] = def;
/* 中文逐项-变量 V1652：tokStr
 * tokStr（char；函数 parseList 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 tokStr = strdup(str)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* tokStr = strdup(str);
/* 中文逐项-变量 V1653：tmpStr
 * tmpStr（char；函数 parseList 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：char* token = strtok_r(tokStr, ",", &tmpStr);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char* tmpStr;
/* 中文逐项-变量 V1654：token
 * token（char；函数 parseList 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 token = strtok_r(tokStr, ",", &tmpStr)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* token = strtok_r(tokStr, ",", &tmpStr);
  while (token) {
    for (/* 中文逐项-变量 V1655：i
 * i（int；函数 parseList 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nelems；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nelems; i++)
      if (strcasecmp(token, elems[i]) == 0) list[i] = set;
    token = strtok_r(NULL, ",", &tmpStr);
  }
  free(tokStr);
  return ncclSuccess;
}

// Latencies in us, Bandwidths in GB/s
// Tree { LL, LL128, Simple } , Ring { LL, LL128, Simple }
/* 中文逐项-变量 V1656：baseLat
 * baseLat（float；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 baseLat = { { 4.4, 4.4, 0 }, { 3.6, 10.0, 8.4 }, { 4.4, 4.4, 0 }
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static const float baseLat  [NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS] = { { 4.4, 4.4,  0 }, { 3.6, 10.0, 8.4 }, { 4.4, 4.4,  0 } };

// NVLink, PCI, Network
/* 中文逐项-常量 X0095：NCCL_HW_NVLINK
 * 编译期定义：NCCL_HW_NVLINK = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_HW_NVLINK 0
/* 中文逐项-常量 X0096：NCCL_HW_PCI
 * 编译期定义：NCCL_HW_PCI = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_HW_PCI 1
/* 中文逐项-常量 X0097：NCCL_HW_NET
 * 编译期定义：NCCL_HW_NET = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_HW_NET 2
// Tree/Simple is the latency a 256kB chunk, which is ~ base lat + 256k/12GB/s (+ 256k/12GB/s for the network).
/* 中文逐项-变量 V1657：hwLat
 * hwLat（float；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 hwLat = { / * NVLINK * / { / * Tree (LL/LL128/Simple)* / { .52, 1.25, 28 }, / * Ring (LL/LL128/Simple)* / {
 * .47, 1.9, 3.4 }, / * CollNet (LL/LL128/Simple)* / { .5, 1.2, 8.0 } }, / * PCI * / { / * Tree (LL/LL128/Simple)* / {
 * 1.0, 1.9, 28 } …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static float hwLat [3][NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS] =
{ /* NVLINK */
  { /* Tree (LL/LL128/Simple)*/ { .52, 1.25, 28 }, /* Ring (LL/LL128/Simple)*/ { .47, 1.9, 3.4 }, /* CollNet (LL/LL128/Simple)*/ {  .5, 1.2, 8.0 } },
  /* PCI */
  { /* Tree (LL/LL128/Simple)*/ { 1.0, 1.9, 28 }, /* Ring (LL/LL128/Simple)*/ { 1.0, 2.5, 5.7 }, /* CollNet (LL/LL128/Simple)*/ { 1.0, 1.9, 8.0 } },
  /* NET */
  { /* Tree (LL/LL128/Simple)*/ { 5.0, 8.5, 28 }, /* Ring (LL/LL128/Simple)*/ { 2.7, 4.0, 28 }, /* CollNet (LL/LL128/Simple)*/ { 5.0, 5.0, 10.7 } }
};

// LL128 max BW per channel
/* 中文逐项-变量 V1658：ll128MaxBwPerCh
 * ll128MaxBwPerCh（double；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ll128MaxBwPerCh = 20.0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
static const double ll128MaxBwPerCh = 20.0;
/* 中文逐项-变量 V1659：llMaxBws
 * llMaxBws（double；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 llMaxBws = { / * Volta-N1/Intel-N2/Intel-N4) * / {39.0, 39.0, 20.4}, / * Ampere-N1/AMD-N2/AMD-N4) * / {87.7,
 * 22.5 / *avg of ring & tree* /, 19.0} }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static const double llMaxBws[2][3] = { /* Volta-N1/Intel-N2/Intel-N4) */ {39.0, 39.0, 20.4}, /* Ampere-N1/AMD-N2/AMD-N4) */ {87.7, 22.5 /*avg of ring & tree*/, 19.0} };
/* 中文逐项-变量 V1660：perChMaxTreeBws
 * perChMaxTreeBws（double；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 perChMaxTreeBws = { / * Volta (N1/N2/N4) * / {26.5, 18.5, 10.0}, / * Ampere (N1/N2/N4) * / {24.0, 23.6, 17.8}
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static const double perChMaxTreeBws[2][3] = { /* Volta (N1/N2/N4) */ {26.5, 18.5, 10.0}, /* Ampere (N1/N2/N4) */ {24.0, 23.6, 17.8} };

/* 中文导读：
 * 按函数×算法×协议建立延迟、带宽和线程阈值模型，并应用 NCCL_ALGO/NCCL_PROTO 等筛选。
 * 模型供每次操作选路使用；初始化支持的算法不意味着某次操作都会执行它们。
 */
/* 中文精读：从硬件图生成算法的性能模型
 * 建立 coll × algorithm × protocol 的带宽和延迟表，结合 GPU 架构、节点数和图搜索结果。
 * 某组合带宽为 0 表示不可用；NCCL_ALGO/NCCL_PROTO 等环境配置可进一步限制候选。
 * 这里并不实际运行一次每种算法来测当前张量，而是使用经验模型指导后面的 getAlgoInfo。
 * 选择算法与选择 transport 是两层决策：Ring 同样可以混合使用本地 P2P 与跨机 NET。
 */
/* 中文内部逻辑 F0530：ncclTopoTuneModel
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int simpleDefaultThreads = (ringGraph->speedIntra*ringGraph->nChannels <= PCI_WIDTH) ? 256 : NCCL_SIMPLE_MAX_NTHREADS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_SIMPLE] = getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_SIMPLE_MAX_NTHREADS, simpleDefaultThreads);；读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。
 * 3. 调用 / 计算 / 写入：comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE] = getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_SIMPLE_MAX_NTHREADS, NCCL_SIMPLE_MAX_NTHREADS);；读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。
 * 4. 调用 / 计算 / 写入：comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE] = NCCL_SIMPLE_MAX_NTHREADS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_LL] = comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_LL] = comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_LL] = getNthreads("NCCL_NTHREADS", ncclParamNthreads( …（完整代码见函数体）；读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。
 * 6. 调用 / 计算 / 写入：comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_LL128] = comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_LL128] = comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_LL128] = getNthreads("NCCL_LL128_NTHREADS", ncc …（完整代码见函数体）；读取线程参数后检查最小、最大及 warp 粒度约束，形成某协议合法线程数。
 * 7. 准备局部状态：int nNodes = comm->nNodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 准备局部状态：int nRanks = comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 条件判断：(nRanks <= 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 准备局部状态：int compCap80 = minCompCap == 80 && maxCompCap == 80 ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：int cpuArch, cpuVendor, cpuModel;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor, &cpuModel));；失败时可能提前return，成功才继续；从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。
 * 15. 准备局部状态：int index2 = nNodes <= 2 ? nNodes-1 : 2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 16. 准备局部状态：int index1 = nNodes == 1 ? compCap80 : cpuVendor == NCCL_TOPO_CPU_VENDOR_AMD ? 1 : 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 17. 准备局部状态：double llMaxBw = llMaxBws[index1][index2];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 18. 准备局部状态：double perChMaxTreeBw = perChMaxTreeBws[compCap80][index2];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 19. 条件判断：(cpuArch == NCCL_TOPO_CPU_ARCH_POWER)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 调用 / 计算 / 写入：hwLat[NCCL_HW_PCI][NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE] = hwLat[NCCL_HW_PCI][NCCL_ALGO_RING][NCCL_PROTO_SIMPLE];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 22. 准备局部状态：float ppn = (float)nRanks / nNodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 23. 准备局部状态：struct ncclTopoGraph* graphs[NCCL_NUM_ALGORITHMS] = { treeGraph, ringGraph, collNetGraph };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 24. 准备局部状态：int intraHw[NCCL_NUM_ALGORITHMS], hw[NCCL_NUM_ALGORITHMS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 25. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   26. 调用 / 计算 / 写入：intraHw[a] = graphs[a]->typeIntra == LINK_NVL ? NCCL_HW_NVLINK : NCCL_HW_PCI;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 27. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 28. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   29. 调用 / 计算 / 写入：hw[a] = nNodes == 1 ? intraHw[a] : NCCL_HW_NET;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 31. 循环：初始 int coll=0;；继续条件 coll<NCCL_NUM_FUNCTIONS；每轮更新 coll++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   32. 准备局部状态：int nsteps = coll == ncclFuncAllReduce ? 2*(nRanks-1) : coll == ncclFuncReduceScatter || coll == ncclFuncAllGather ? nRanks-1 : nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   33. 准备局部状态：int nInterSteps = coll == ncclFuncAllReduce ? (nNodes > 1 ? 2*nNodes :0) : coll == ncclFuncReduceScatter || coll == ncclFuncAllGather ? nNodes-1 : nNodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   34. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     35. 条件判断：(coll != ncclFuncAllReduce && a != NCCL_ALGO_RING)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       36. 真分支：；仅条件为真执行以下子项。
 *         37. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *     38. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       39. 准备局部状态：float speed = nNodes <= 2 || a == NCCL_ALGO_COLLNET ? graphs[a]->speedIntra : graphs[a]->speedInter;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       40. 准备局部状态：float busBw = graphs[a]->nChannels * speed;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       41. 条件判断：(compCap80)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         42. 真分支：；仅条件为真执行以下子项。
 *           43. 调用 / 计算 / 写入：busBw = std::min(busBw, 235.0f);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       44. 条件判断：(a == NCCL_ALGO_RING && p == NCCL_PROTO_LL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         45. 真分支：；仅条件为真执行以下子项。
 *           46. 调用 / 计算 / 写入：busBw = std::min(llMaxBw, busBw * ((nNodes > 1 || coll == ncclFuncAllReduce || coll == ncclFuncReduce) ? 1.0/4.0 : 1.0/3.0));；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       47. 条件判断：(a == NCCL_ALGO_RING && p == NCCL_PROTO_LL128)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         48. 真分支：；仅条件为真执行以下子项。
 *           49. 调用 / 计算 / 写入：busBw = std::min(busBw * (ppn < 2 ? 0.7 : 0.92 ), ll128MaxBwPerCh*graphs[a]->nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       50. 条件判断：(a == NCCL_ALGO_TREE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         51. 真分支：；仅条件为真执行以下子项。
 *           52. 调用 / 计算 / 写入：busBw = std::min(busBw*.92, graphs[a]->nChannels*perChMaxTreeBw);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       53. 条件判断：(a == NCCL_ALGO_TREE && p == NCCL_PROTO_LL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         54. 真分支：；仅条件为真执行以下子项。
 *           55. 调用 / 计算 / 写入：busBw = std::min(busBw*1.0/3.8, llMaxBw);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       56. 条件判断：(a == NCCL_ALGO_TREE && p == NCCL_PROTO_LL128)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         57. 真分支：；仅条件为真执行以下子项。
 *           58. 调用 / 计算 / 写入：busBw = std::min(busBw * (nNodes == 1 ? 7.0/9.0 : 120.0/128.0), ll128MaxBwPerCh*graphs[a]->nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *       59. 条件判断：(a == NCCL_ALGO_COLLNET && p != NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         60. 真分支：；仅条件为真执行以下子项。
 *           61. 调用 / 计算 / 写入：busBw = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       62. 准备局部状态：float ratio = (a != NCCL_ALGO_RING) ? .5 : (1.0 * nRanks) / nsteps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       63. 调用 / 计算 / 写入：comm->bandwidths[coll][a][p] = busBw * ratio;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       64. 调用 / 计算 / 写入：comm->latencies[coll][a][p] = baseLat[a][p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       65. 准备局部状态：float intraLat = hwLat[intraHw[a]][a][p];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       66. 准备局部状态：float interLat = graphs[a]->latencyInter ? graphs[a]->latencyInter : hwLat[NCCL_HW_NET][a][p];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       67. 条件判断：(nNodes > 1 && p == NCCL_PROTO_LL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         68. 真分支：；仅条件为真执行以下子项。
 *           69. 调用 / 计算 / 写入：intraLat *= 1.8;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       70. 条件判断：(a == NCCL_ALGO_RING)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         71. 真分支：；仅条件为真执行以下子项。
 *           72. 准备局部状态：float lat = hwLat[hw[a]][a][p];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           73. 条件判断：((coll == ncclFuncReduce || coll == ncclFuncBroadcast))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             74. 真分支：；仅条件为真执行以下子项。
 *               75. 条件判断：(ringGraph->sameChannels)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 76. 真分支：；仅条件为真执行以下子项。
 *                   77. 调用 / 计算 / 写入：comm->latencies[coll][a][p] += lat;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 78. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   79. 条件判断：(p == NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     80. 真分支：；仅条件为真执行以下子项。
 *                       81. 调用 / 计算 / 写入：lat = hwLat[hw[a]][NCCL_ALGO_TREE][p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   82. 调用 / 计算 / 写入：comm->latencies[coll][a][p] += nsteps*lat;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             83. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *               84. 调用 / 计算 / 写入：comm->latencies[coll][a][p] += (nsteps-nInterSteps)*intraLat + nInterSteps*interLat;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         85. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           86. 条件判断：(a == NCCL_ALGO_TREE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             87. 真分支：；仅条件为真执行以下子项。
 *               88. 调用 / 计算 / 写入：comm->latencies[coll][a][p] += 2 * ((nRanks/nNodes-1) * intraLat + log2i(nNodes) * interLat);；用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。
 *             89. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *               90. 调用 / 计算 / 写入：comm->latencies[coll][a][p] += 2 * (std::min(1, (nRanks/nNodes-1)) * intraLat + (nRanks/nNodes-1) * 0.5) + interLat;；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     91. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   92. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 93. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 94. 准备局部状态：int protoEnable[NCCL_NUM_PROTOCOLS] = { 1, 2, 1 };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 95. 准备局部状态：int algoEnable[NCCL_NUM_ALGORITHMS] = { 1, 1, 1 };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 96. 准备局部状态：const char *protoStr = getenv("NCCL_PROTO");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 97. 条件判断：(protoStr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   98. 真分支：；仅条件为真执行以下子项。
 *     99. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_PROTO set by environment to %s", protoStr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     100. 调用 / 计算 / 写入：NCCLCHECK(parseList(protoStr, ncclProtoStr, NCCL_NUM_PROTOCOLS, protoEnable));；失败时可能提前return，成功才继续；解析算法/协议等配置名单和排除前缀，生成各候选是否启用的布尔表。
 * 101. 准备局部状态：const char *algoStr = getenv("NCCL_ALGO");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 102. 条件判断：(algoStr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   103. 真分支：；仅条件为真执行以下子项。
 *     104. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_ALGO set by environment to %s", algoStr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     105. 调用 / 计算 / 写入：NCCLCHECK(parseList(algoStr, ncclAlgoStr, NCCL_NUM_ALGORITHMS, algoEnable));；失败时可能提前return，成功才继续；解析算法/协议等配置名单和排除前缀，生成各候选是否启用的布尔表。
 * 106. 条件判断：(comm->collNetSupport == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   107. 真分支：；仅条件为真执行以下子项。
 *     108. 调用 / 计算 / 写入：algoEnable[NCCL_ALGO_COLLNET] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     109. 条件判断：(algoEnable[NCCL_ALGO_RING] == 0 && algoEnable[NCCL_ALGO_TREE] == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       110. 真分支：；仅条件为真执行以下子项。
 *         111. 调用 / 计算 / 写入：algoEnable[NCCL_ALGO_RING] = algoEnable[NCCL_ALGO_TREE] = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         112. 条件判断：(comm->rank == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           113. 真分支：；仅条件为真执行以下子项。
 *             114. 调用 / 计算 / 写入：WARN("CollNet is not supported or fails to initialize, ignoring NCCL_ALGO=COLLNET");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 115. 循环：初始 int c=0;；继续条件 c<NCCL_NUM_FUNCTIONS；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   116. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     117. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       118. 准备局部状态：int pEnable = protoEnable[p];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       119. 条件判断：(pEnable == 2 && p == NCCL_PROTO_LL128)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         120. 真分支：；仅条件为真执行以下子项。
 *           121. 调用 / 计算 / 写入：pEnable = (graphs[a]->typeInter <= PATH_PXB) && graphs[a]->typeIntra <= PATH_NVL && ((minCompCap == 70 && maxCompCap == 70) || (minCompCap == 80 && maxCompCap == 80)) ? 1 : 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       122. 条件判断：(pEnable == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         123. 真分支：；仅条件为真执行以下子项。
 *           124. 调用 / 计算 / 写入：comm->bandwidths[c][a][p] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       125. 条件判断：(c == ncclFuncAllReduce && algoEnable[a] == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         126. 真分支：；仅条件为真执行以下子项。
 *           127. 调用 / 计算 / 写入：comm->bandwidths[c][a][p] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     128. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   129. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 130. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 131. 条件判断：(comm->rank == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   132. 真分支：；仅条件为真执行以下子项。
 *     133. 准备局部状态：char line[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     134. 调用 / 计算 / 写入：sprintf(line, "Latency/AlgBw |");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     135. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       136. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         137. 调用 / 计算 / 写入：sprintf(line+strlen(line), " %7s/%6s |", ncclAlgoStr[a], ncclProtoStr[p]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       138. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     139. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     140. 调用 / 计算 / 写入：INFO(NCCL_TUNING, "%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     141. 调用 / 计算 / 写入：sprintf(line, " Max NThreads |");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     142. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       143. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         144. 调用 / 计算 / 写入：sprintf(line+strlen(line), " %14d |", comm->maxThreads[a][p]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       145. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     146. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     147. 调用 / 计算 / 写入：INFO(NCCL_TUNING, "%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     148. 循环：初始 int c=0;；继续条件 c<NCCL_NUM_FUNCTIONS；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       149. 调用 / 计算 / 写入：sprintf(line, "%13s |", ncclFuncStr[c]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       150. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         151. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           152. 调用 / 计算 / 写入：sprintf(line+strlen(line), "%8.1f/%6.1f |", comm->latencies[c][a][p], comm->bandwidths[c][a][p]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         153. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       154. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       155. 调用 / 计算 / 写入：INFO(NCCL_TUNING, "%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     156. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 157. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   158. 调用 / 计算 / 写入：comm->threadThresholds[a][NCCL_PROTO_LL] = NCCL_LL_THREAD_THRESHOLD;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   159. 调用 / 计算 / 写入：comm->threadThresholds[a][NCCL_PROTO_LL128] = NCCL_LL128_THREAD_THRESHOLD;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   160. 调用 / 计算 / 写入：comm->threadThresholds[a][NCCL_PROTO_SIMPLE] = NCCL_SIMPLE_THREAD_THRESHOLD;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 161. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 162. 调用 / 计算 / 写入：comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL] *= nRanks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 163. 调用 / 计算 / 写入：comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE] = 512;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 164. 准备局部状态：char* str = getenv("NCCL_THREAD_THRESHOLDS");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 165. 条件判断：(str)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   166. 真分支：；仅条件为真执行以下子项。
 *     167. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_THREAD_THRESHOLDS set by environment to %s", str);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     168. 准备局部状态：ssize_t t[NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS] = {{ -2, -2, -2 }, { -2, -2, -2}};；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     169. 调用 / 计算 / 写入：sscanf(str, "%ld %ld %ld %ld %ld %ld", t[0], t[0]+1, t[0]+2, t[1], t[1]+1, t[1]+2);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     170. 循环：初始 int a=0;；继续条件 a<NCCL_NUM_ALGORITHMS；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       171. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         172. 条件判断：(t[a][p] >= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           173. 真分支：；仅条件为真执行以下子项。
 *             174. 调用 / 计算 / 写入：comm->threadThresholds[a][p] = t[a][p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       175. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     176. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 177. 调用 / 计算 / 写入：INFO(NCCL_INIT, "threadThresholds %ld/%ld/%ld | %ld/%ld/%ld | %ld/%ld/%ld", comm->threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_LL], comm->threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_LL128], comm->threa …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 178. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__tuning_cc.md#f0530
 */
/* 中文逐项-函数 F0530：ncclTopoTuneModel
 * 用途与位置：结合拓扑图带宽、节点规模、GPU 架构和配置构建各 API/算法/协议的延迟与带宽表。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 minCompCap（int minCompCap）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 maxCompCap（int maxCompCap）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 treeGraph（struct ncclTopoGraph* treeGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ringGraph（struct ncclTopoGraph* ringGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 collNetGraph（struct ncclTopoGraph* collNetGraph）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；getNthreads；getenv；log2i；ncclParamLl128Nthreads；ncclParamNthreads；ncclTopoCpuType；parseList；sprintf；sscanf；std::min；strlen。
 * 需要对照的边界/推进条件：nRanks <= 1；cpuArch ==
 * NCCL_TOPO_CPU_ARCH_POWER；a<NCCL_NUM_ALGORITHMS；a<NCCL_NUM_ALGORITHMS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoTuneModel(struct ncclComm* comm, int minCompCap, int maxCompCap, struct ncclTopoGraph* treeGraph, struct ncclTopoGraph* ringGraph, struct ncclTopoGraph* collNetGraph) {
/* 中文逐项-变量 V1661：simpleDefaultThreads
 * simpleDefaultThreads（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 simpleDefaultThreads = (ringGraph->speedIntra*ringGraph->nChannels <= PCI_WIDTH) ? 256 :
 * NCCL_SIMPLE_MAX_NTHREADS；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int simpleDefaultThreads = (ringGraph->speedIntra*ringGraph->nChannels <= PCI_WIDTH) ? 256 : NCCL_SIMPLE_MAX_NTHREADS;
  comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_SIMPLE] =
    getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_SIMPLE_MAX_NTHREADS, simpleDefaultThreads);
  comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE] =
    getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_SIMPLE_MAX_NTHREADS, NCCL_SIMPLE_MAX_NTHREADS);
  comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE] = NCCL_SIMPLE_MAX_NTHREADS;
  comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_LL] = comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_LL] = comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_LL] =
    getNthreads("NCCL_NTHREADS", ncclParamNthreads(), 2*WARP_SIZE, NCCL_LL_MAX_NTHREADS, NCCL_LL_MAX_NTHREADS);
  comm->maxThreads[NCCL_ALGO_RING][NCCL_PROTO_LL128] = comm->maxThreads[NCCL_ALGO_TREE][NCCL_PROTO_LL128] = comm->maxThreads[NCCL_ALGO_COLLNET][NCCL_PROTO_LL128] =
    getNthreads("NCCL_LL128_NTHREADS", ncclParamLl128Nthreads(), NCCL_LL128_MAX_NTHREADS/4, NCCL_LL128_MAX_NTHREADS, NCCL_LL128_MAX_NTHREADS);

/* 中文逐项-变量 V1662：nNodes
 * nNodes（int；函数 ncclTopoTuneModel 内）：当前 communicator 跨越的节点数量；节点编号不等于 GPU rank。
 * 初值/来源 nNodes = comm->nNodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nNodes = comm->nNodes;
/* 中文逐项-变量 V1663：nRanks
 * nRanks（int；函数 ncclTopoTuneModel 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nRanks = comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nRanks = comm->nRanks;
  if (nRanks <= 1) return ncclSuccess;

/* 中文逐项-变量 V1664：compCap80
 * compCap80（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 compCap80 = minCompCap == 80 && maxCompCap == 80 ? 1 : 0；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int compCap80 = minCompCap == 80 && maxCompCap == 80 ? 1 : 0;
/* 中文逐项-变量 V1665：cpuArch、cpuVendor、cpuModel
 * cpuArch（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor,
 * &cpuModel));。若由输出参数填入，必须先检查填充调用成功再读值。
 * cpuVendor（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor,
 * &cpuModel));。若由输出参数填入，必须先检查填充调用成功再读值。
 * cpuModel（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor,
 * &cpuModel));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int cpuArch, cpuVendor, cpuModel;
  NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor, &cpuModel));
/* 中文逐项-变量 V1666：index2
 * index2（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 index2 = nNodes <= 2 ? nNodes-1 : 2；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int index2 = nNodes <= 2 ? nNodes-1 : 2;
  // LL: for single node, we look at GPU type; for multi-node, we look at CPU type
/* 中文逐项-变量 V1667：index1
 * index1（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 index1 = nNodes == 1 ? compCap80 : cpuVendor == NCCL_TOPO_CPU_VENDOR_AMD ? 1 :
 * 0；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int index1 = nNodes == 1 ? compCap80 : cpuVendor == NCCL_TOPO_CPU_VENDOR_AMD ? 1 : 0;
/* 中文逐项-变量 V1668：llMaxBw
 * llMaxBw（double；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 llMaxBw = llMaxBws[index1][index2]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  double llMaxBw = llMaxBws[index1][index2];
/* 中文逐项-变量 V1669：perChMaxTreeBw
 * perChMaxTreeBw（double；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 perChMaxTreeBw = perChMaxTreeBws[compCap80][index2]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  double perChMaxTreeBw = perChMaxTreeBws[compCap80][index2];
  // De-penalize Tree/Simple latency on Power systems to favor Tree than Ring
  if (cpuArch == NCCL_TOPO_CPU_ARCH_POWER) hwLat[NCCL_HW_PCI][NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE] = hwLat[NCCL_HW_PCI][NCCL_ALGO_RING][NCCL_PROTO_SIMPLE];
/* 中文逐项-变量 V1670：ppn
 * ppn（float；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ppn = (float)nRanks / nNodes；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  float ppn = (float)nRanks / nNodes; // if ppn < 2, then we are sending/receiving at the same GPU through the NIC, apply some bw discount

/* 中文逐项-变量 V1671：graphs
 * graphs（struct ncclTopoGraph；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 graphs = { treeGraph, ringGraph, collNetGraph }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclTopoGraph* graphs[NCCL_NUM_ALGORITHMS] = { treeGraph, ringGraph, collNetGraph };
/* 中文逐项-变量 V1672：intraHw、hw
 * intraHw（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) intraHw[a] = graphs[a]->typeIntra == LINK_NVL ?
 * NCCL_HW_NVLINK : NCCL_HW_PCI;。若由输出参数填入，必须先检查填充调用成功再读值。
 * hw（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int a=0; a<NCCL_NUM_ALGORITHMS; a++) hw[a] = nNodes == 1 ? intraHw[a] :
 * NCCL_HW_NET;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int intraHw[NCCL_NUM_ALGORITHMS], hw[NCCL_NUM_ALGORITHMS];
  for (/* 中文逐项-变量 V1673：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) intraHw[a] = graphs[a]->typeIntra == LINK_NVL ? NCCL_HW_NVLINK : NCCL_HW_PCI;
  for (/* 中文逐项-变量 V1674：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) hw[a] = nNodes == 1 ? intraHw[a] : NCCL_HW_NET;

  for (/* 中文逐项-变量 V1675：coll
 * coll（int；函数 ncclTopoTuneModel 内）：本任务的公开操作类别，如 AllReduce、Broadcast、Send 或 Recv。
 * 初值/来源 coll = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：coll<NCCL_NUM_FUNCTIONS；循环体与增量表达式决定每轮如何推进。
 */int coll=0; coll<NCCL_NUM_FUNCTIONS; coll++) {
/* 中文逐项-变量 V1676：nsteps
 * nsteps（int；函数 ncclTopoTuneModel 内）：本子任务需要推进的协议总 step 数，不直接等于包数、WQE 数或 QP 数。
 * 初值/来源 nsteps = coll == ncclFuncAllReduce ? 2*(nRanks-1) : coll == ncclFuncReduceScatter || coll ==
 * ncclFuncAllGather ? nRanks-1 : nRanks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int nsteps = coll == ncclFuncAllReduce ? 2*(nRanks-1) :
      coll == ncclFuncReduceScatter || coll == ncclFuncAllGather ? nRanks-1 :
      nRanks;
/* 中文逐项-变量 V1677：nInterSteps
 * nInterSteps（int；函数 ncclTopoTuneModel 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nInterSteps = coll == ncclFuncAllReduce ? (nNodes > 1 ? 2*nNodes :0) : coll == ncclFuncReduceScatter || coll
 * == ncclFuncAllGather ? nNodes-1 : nNodes；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int nInterSteps = coll == ncclFuncAllReduce ? (nNodes > 1 ? 2*nNodes :0) :
      coll == ncclFuncReduceScatter || coll == ncclFuncAllGather ? nNodes-1 :
      nNodes;

    for (/* 中文逐项-变量 V1678：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
      if (coll != ncclFuncAllReduce && a != NCCL_ALGO_RING) continue;

      for (/* 中文逐项-变量 V1679：p
 * p（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
/* 中文逐项-变量 V1680：speed
 * speed（float；函数 ncclTopoTuneModel 内）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 初值/来源 speed = nNodes <= 2 || a == NCCL_ALGO_COLLNET ? graphs[a]->speedIntra :
 * graphs[a]->speedInter；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        float speed = nNodes <= 2 || a == NCCL_ALGO_COLLNET ? graphs[a]->speedIntra : graphs[a]->speedInter;
/* 中文逐项-变量 V1681：busBw
 * busBw（float；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 busBw = graphs[a]->nChannels * speed；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        float busBw = graphs[a]->nChannels * speed;

        // Various model refinements
        if (compCap80) busBw = std::min(busBw, 235.0f);
        if (a == NCCL_ALGO_RING && p == NCCL_PROTO_LL) { busBw = std::min(llMaxBw, busBw * ((nNodes > 1 || coll == ncclFuncAllReduce || coll == ncclFuncReduce) ? 1.0/4.0 : 1.0/3.0)); }
        if (a == NCCL_ALGO_RING && p == NCCL_PROTO_LL128) busBw = std::min(busBw * (ppn < 2 ? 0.7 : 0.92 /*120.0/128.0*/), ll128MaxBwPerCh*graphs[a]->nChannels);
        if (a == NCCL_ALGO_TREE) busBw = std::min(busBw*.92, graphs[a]->nChannels*perChMaxTreeBw);
        if (a == NCCL_ALGO_TREE && p == NCCL_PROTO_LL) busBw = std::min(busBw*1.0/3.8, llMaxBw);
        if (a == NCCL_ALGO_TREE && p == NCCL_PROTO_LL128) busBw = std::min(busBw * (nNodes == 1 ? 7.0/9.0 : 120.0/128.0), ll128MaxBwPerCh*graphs[a]->nChannels);
        if (a == NCCL_ALGO_COLLNET && p != NCCL_PROTO_SIMPLE) busBw = 0;  // Oneshot CollNet only supports Simple

        // Convert bus BW to algorithm BW
/* 中文逐项-变量 V1682：ratio
 * ratio（float；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ratio = (a != NCCL_ALGO_RING) ? .5 : (1.0 * nRanks) / nsteps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        float ratio = (a != NCCL_ALGO_RING) ? .5 : (1.0 * nRanks) / nsteps;
        comm->bandwidths[coll][a][p] = busBw * ratio;

        comm->latencies[coll][a][p] = baseLat[a][p];
/* 中文逐项-变量 V1683：intraLat
 * intraLat（float；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 intraLat = hwLat[intraHw[a]][a][p]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        float intraLat = hwLat[intraHw[a]][a][p];
/* 中文逐项-变量 V1684：interLat
 * interLat（float；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 interLat = graphs[a]->latencyInter ? graphs[a]->latencyInter :
 * hwLat[NCCL_HW_NET][a][p]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        float interLat = graphs[a]->latencyInter ? graphs[a]->latencyInter : hwLat[NCCL_HW_NET][a][p];

        if (nNodes > 1 && p == NCCL_PROTO_LL) intraLat *= 1.8;
        if (a == NCCL_ALGO_RING) {
/* 中文逐项-变量 V1685：lat
 * lat（float；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lat = hwLat[hw[a]][a][p]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          float lat = hwLat[hw[a]][a][p];
          if ((coll == ncclFuncReduce || coll == ncclFuncBroadcast)) {
            if (ringGraph->sameChannels) {
              comm->latencies[coll][a][p] += lat;
            } else {
              if (p == NCCL_PROTO_SIMPLE) lat = hwLat[hw[a]][NCCL_ALGO_TREE][p]; // Add some chunk latency, waiting for proper chunk modeling
              comm->latencies[coll][a][p] += nsteps*lat;
            }
          } else {
            comm->latencies[coll][a][p] += (nsteps-nInterSteps)*intraLat + nInterSteps*interLat;
          }
        } else if (a == NCCL_ALGO_TREE) {
          comm->latencies[coll][a][p] +=
            2 * ((nRanks/nNodes-1) * intraLat + log2i(nNodes) * interLat);
        } else {
          comm->latencies[coll][a][p] +=
            2 * (std::min(1, (nRanks/nNodes-1)) * intraLat + (nRanks/nNodes-1) * 0.5) + interLat;  // Add 0.5 arity serialization latency
        }
      }
    }
  }

  // Protocols/Algorithms enable/disable, and user overrides.
  // All are enabled except ll128 which is enabled by default only in certain cases.
/* 中文逐项-变量 V1686：protoEnable
 * protoEnable（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 protoEnable = { 1, 2, 1 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  int protoEnable[NCCL_NUM_PROTOCOLS] = { 1, 2, 1 };
/* 中文逐项-变量 V1687：algoEnable
 * algoEnable（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 algoEnable = { 1, 1, 1 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  int algoEnable[NCCL_NUM_ALGORITHMS] = { 1, 1, 1 };

/* 中文逐项-变量 V1688：protoStr
 * protoStr（char；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 protoStr = getenv("NCCL_PROTO")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const char *protoStr = getenv("NCCL_PROTO");
  if (protoStr) {
    INFO(NCCL_ENV, "NCCL_PROTO set by environment to %s", protoStr);
    NCCLCHECK(parseList(protoStr, ncclProtoStr, NCCL_NUM_PROTOCOLS, protoEnable));
  }
/* 中文逐项-变量 V1689：algoStr
 * algoStr（char；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 algoStr = getenv("NCCL_ALGO")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const char *algoStr = getenv("NCCL_ALGO");
  if (algoStr) {
    INFO(NCCL_ENV, "NCCL_ALGO set by environment to %s", algoStr);
    NCCLCHECK(parseList(algoStr, ncclAlgoStr, NCCL_NUM_ALGORITHMS, algoEnable));
  }
  // Disable CollNet if it is not supported
  if (comm->collNetSupport == 0) {
    algoEnable[NCCL_ALGO_COLLNET] = 0;
    // If user has hard set NCCL_ALGO=COLLNET, ignore it
    if (algoEnable[NCCL_ALGO_RING] == 0 && algoEnable[NCCL_ALGO_TREE] == 0) {
      algoEnable[NCCL_ALGO_RING] = algoEnable[NCCL_ALGO_TREE] = 1;
      if (comm->rank == 0) WARN("CollNet is not supported or fails to initialize, ignoring NCCL_ALGO=COLLNET");
    }
  }

  for (/* 中文逐项-变量 V1690：c
 * c（int；函数 ncclTopoTuneModel 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<NCCL_NUM_FUNCTIONS；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<NCCL_NUM_FUNCTIONS; c++) for (/* 中文逐项-变量 V1691：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) for (/* 中文逐项-变量 V1692：p
 * p（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
/* 中文逐项-变量 V1693：pEnable
 * pEnable（int；函数 ncclTopoTuneModel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 pEnable = protoEnable[p]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int pEnable = protoEnable[p];
    if (pEnable == 2 && p == NCCL_PROTO_LL128) {
      // Enable LL128 by default only on Volta/Ampere+NVLink. Other cases are not tested and may cause silent data corruption.
      pEnable = (graphs[a]->typeInter <= PATH_PXB) && graphs[a]->typeIntra <= PATH_NVL &&
        ((minCompCap == 70 && maxCompCap == 70) || (minCompCap == 80 && maxCompCap == 80)) ? 1 : 0;
    }
    if (pEnable == 0) comm->bandwidths[c][a][p] = 0;
    // Only disable algo for Allreduce since others only have one
    if (c == ncclFuncAllReduce && algoEnable[a] == 0) comm->bandwidths[c][a][p] = 0;
  }

  if (comm->rank == 0) {
/* 中文逐项-变量 V1694：line
 * line（char；函数 ncclTopoTuneModel 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：sprintf(line, "Latency/AlgBw |");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char line[1024];
    sprintf(line, "Latency/AlgBw |");
    for (/* 中文逐项-变量 V1695：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
      for (/* 中文逐项-变量 V1696：p
 * p（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
        sprintf(line+strlen(line), " %7s/%6s |", ncclAlgoStr[a], ncclProtoStr[p]);
      }
    }
    INFO(NCCL_TUNING, "%s", line);
    sprintf(line, " Max NThreads |");
    for (/* 中文逐项-变量 V1697：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
      for (/* 中文逐项-变量 V1698：p
 * p（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
        sprintf(line+strlen(line), " %14d |", comm->maxThreads[a][p]);
      }
    }
    INFO(NCCL_TUNING, "%s", line);
    for (/* 中文逐项-变量 V1699：c
 * c（int；函数 ncclTopoTuneModel 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<NCCL_NUM_FUNCTIONS；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<NCCL_NUM_FUNCTIONS; c++) {
      sprintf(line, "%13s |", ncclFuncStr[c]);
      for (/* 中文逐项-变量 V1700：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
        for (/* 中文逐项-变量 V1701：p
 * p（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
          sprintf(line+strlen(line), "%8.1f/%6.1f |", comm->latencies[c][a][p], comm->bandwidths[c][a][p]);
        }
      }
      INFO(NCCL_TUNING, "%s", line);
    }
  }

  // Set per-thread amount of work before we increase nThreads and nChannels
  for (/* 中文逐项-变量 V1702：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
    comm->threadThresholds[a][NCCL_PROTO_LL] = NCCL_LL_THREAD_THRESHOLD;
    comm->threadThresholds[a][NCCL_PROTO_LL128] = NCCL_LL128_THREAD_THRESHOLD;
    comm->threadThresholds[a][NCCL_PROTO_SIMPLE] = NCCL_SIMPLE_THREAD_THRESHOLD;
  }
  comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL] *= nRanks;
  comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE] = 512;

  // Override defaults with user env
/* 中文逐项-变量 V1703：str
 * str（char；函数 ncclTopoTuneModel 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv("NCCL_THREAD_THRESHOLDS")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* str = getenv("NCCL_THREAD_THRESHOLDS");
  if (str) {
    INFO(NCCL_ENV, "NCCL_THREAD_THRESHOLDS set by environment to %s", str);
/* 中文逐项-变量 V1704：t
 * t（ssize_t；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 t = {{ -2, -2, -2 }, { -2, -2, -2}}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
    ssize_t t[NCCL_NUM_ALGORITHMS][NCCL_NUM_PROTOCOLS] = {{ -2, -2, -2 }, { -2, -2, -2}};
    sscanf(str, "%ld %ld %ld %ld %ld %ld", t[0], t[0]+1, t[0]+2, t[1], t[1]+1, t[1]+2);
    for (/* 中文逐项-变量 V1705：a
 * a（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<NCCL_NUM_ALGORITHMS；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<NCCL_NUM_ALGORITHMS; a++) {
      for (/* 中文逐项-变量 V1706：p
 * p（int；函数 ncclTopoTuneModel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
        if (t[a][p] >= 0) comm->threadThresholds[a][p] = t[a][p];
      }
    }
  }

  INFO(NCCL_INIT, "threadThresholds %ld/%ld/%ld | %ld/%ld/%ld | %ld/%ld/%ld",
      comm->threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_LL],
      comm->threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_LL128],
      comm->threadThresholds[NCCL_ALGO_TREE][NCCL_PROTO_SIMPLE],
      comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL],
      comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_LL128],
      comm->threadThresholds[NCCL_ALGO_RING][NCCL_PROTO_SIMPLE],
      comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_LL],
      comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_LL128],
      comm->threadThresholds[NCCL_ALGO_COLLNET][NCCL_PROTO_SIMPLE]);
  return ncclSuccess;
}

// Trees are not perfectly sticking to the model for medium sizes. Applying a static correction
// factor is not ideal but works quite well. Powers of two, 64 B to 256MB.
/* 中文逐项-变量 V1707：treeCorrectionFactor
 * treeCorrectionFactor（float；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 treeCorrectionFactor = { { 1.0, 1.0, 1.0, 1.0, .9, .8, .7, .7, .7, .7, .6, .5, .4, .4, .5, .6, .7, .8, .9,
 * 1.0, 1.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0, 1.0, .9, .8, .8, .8, .7, .6, .6, .6, .6, .6, .6, .8, .9, .9, .9, .9,
 * 1.0, 1.0 }, { .9, .9, .9, .9, .9, …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static float treeCorrectionFactor[NCCL_NUM_PROTOCOLS][23] = {
  { 1.0, 1.0, 1.0, 1.0,  .9,  .8,  .7,  .7,  .7,  .7,  .6,  .5,  .4,  .4,  .5,  .6,  .7,  .8,  .9, 1.0, 1.0, 1.0, 1.0 },
  { 1.0, 1.0, 1.0, 1.0, 1.0,  .9,  .8,  .8,  .8,  .7,  .6,  .6,  .6,  .6,  .6,  .6,  .8,  .9,  .9,  .9,  .9, 1.0, 1.0 },
  {  .9,  .9,  .9,  .9,  .9,  .9,  .9,  .8,  .7,  .6,  .6,  .5,  .5,  .5,  .5,  .6,  .7,  .8,  .7,  .7,  .8,  .9,  .9 }
};

/* 中文导读：
 * 依据模型估计候选执行时间，核心形式为延迟项加消息字节数/带宽项，并有大小与拓扑修正。
 * bw==0 表示该组合不可用。这里返回预测值，不是实测计时，也不是简单固定的小包/大包分界。
 */
/* 中文精读：为什么小消息和大消息可能选择不同算法
 * 预测 time = latency*latCount + nBytes/(1000*bw)，带宽按 GB/s、延迟按微秒的约定换算。
 * 小消息常由固定延迟主导，大消息更受有效带宽影响；Tree/Ring/协议各有不同参数和修正。
 * 聚合时 numPipeOps 影响延迟计数，info->nChannels 非零时也要按所用通道比例调整带宽。
 * 这是选路估计，不是本次运行的实测耗时，更不是对通信正确性的依赖条件。
 */
/* 中文内部逻辑 F0531：ncclTopoGetAlgoTime
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：float bw = info->comm->bandwidths[info->coll][algorithm][protocol];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：float lat = info->comm->latencies[info->coll][algorithm][protocol];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(bw == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：*time = -1.0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 准备局部状态：int logSize = log2i(info->nBytes>>6);；用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。
 * 8. 条件判断：(algorithm == NCCL_ALGO_TREE && logSize < 23)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：bw *= treeCorrectionFactor[protocol][logSize];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 条件判断：(info->nChannels != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 调用 / 计算 / 写入：bw = bw / info->comm->nChannels * info->nChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 条件判断：(algorithm == NCCL_ALGO_RING && protocol == NCCL_PROTO_SIMPLE && info->comm->nNodes > 1 && info->coll == ncclFuncAllReduce && info->nBytes >= info->comm->nRanks/16.0*65536)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 调用 / 计算 / 写入：lat *= 1.9;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 准备局部状态：int latCount = algorithm == NCCL_ALGO_RING ? numPipeOps : DIVUP(numPipeOps, NCCL_MAX_WORK_ELEMENTS);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 18. 调用 / 计算 / 写入：*time = lat * latCount + (info->nBytes) / (1000 * bw);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__tuning_cc.md#f0531
 */
/* 中文逐项-函数 F0531：ncclTopoGetAlgoTime
 * 用途与位置：用延迟项加字节数/带宽项预测当前候选耗时，加入 Tree/Ring 与聚合修正供选型。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 参数 algorithm（int algorithm）：当前集合算法 Ring、Tree 或 CollNet；决定邻居与数据交换次序。
 * 参数 protocol（int protocol）：当前数据编码/同步协议枚举 LL、LL128 或 Simple；决定有效载荷比例和推进方法。
 * 参数 numPipeOps（int numPipeOps）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 time（float* time）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::getAlgoInfo。
 * 本体直接出现的调用/包装：DIVUP；log2i。
 * 需要对照的边界/推进条件：bw == 0；algorithm == NCCL_ALGO_TREE && logSize < 23；info->nChannels != 0；algorithm == NCCL_ALGO_RING
 * && protocol == NCCL_PROTO_SIMPLE && info->comm->nNodes > 1 && info->coll == ncclFuncAllReduce && info->nBytes >=
 * info …（完整表达式见紧邻源码）。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclTopoGetAlgoTime(struct ncclInfo* info, int algorithm, int protocol, int numPipeOps, float* time) {
/* 中文逐项-变量 V1708：bw
 * bw（float；函数 ncclTopoGetAlgoTime 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bw = info->comm->bandwidths[info->coll][algorithm][protocol]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  float bw = info->comm->bandwidths[info->coll][algorithm][protocol];
/* 中文逐项-变量 V1709：lat
 * lat（float；函数 ncclTopoGetAlgoTime 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lat = info->comm->latencies[info->coll][algorithm][protocol]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  float lat = info->comm->latencies[info->coll][algorithm][protocol];
  if (bw == 0) {
    *time = -1.0; return ncclSuccess;
  }
/* 中文逐项-变量 V1710：logSize
 * logSize（int；函数 ncclTopoGetAlgoTime 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 logSize = log2i(info->nBytes>>6)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int logSize = log2i(info->nBytes>>6);
  if (algorithm == NCCL_ALGO_TREE && logSize < 23) bw *= treeCorrectionFactor[protocol][logSize];
  if (info->nChannels != 0) bw = bw / info->comm->nChannels * info->nChannels;
  if (algorithm == NCCL_ALGO_RING && protocol == NCCL_PROTO_SIMPLE && info->comm->nNodes > 1
      && info->coll == ncclFuncAllReduce && info->nBytes >= info->comm->nRanks/16.0*65536) lat *= 1.9; // Plateau effect of ring
  // Tree pipelining saves latency in aggregation cases
/* 中文逐项-变量 V1711：latCount
 * latCount（int；函数 ncclTopoGetAlgoTime 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 latCount = algorithm == NCCL_ALGO_RING ? numPipeOps : DIVUP(numPipeOps,
 * NCCL_MAX_WORK_ELEMENTS)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int latCount = algorithm == NCCL_ALGO_RING ? numPipeOps : DIVUP(numPipeOps, NCCL_MAX_WORK_ELEMENTS);
  *time = lat * latCount + (info->nBytes) / (1000 * bw);
  return ncclSuccess;
}
