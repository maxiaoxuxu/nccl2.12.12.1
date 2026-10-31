/*************************************************************************
 * Copyright (c) 2016-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "utils.h"
#include "core.h"

#include "nvmlwrap.h"

// Get current Compute Capability
/* 中文内部逻辑 F0991：ncclCudaCompCap
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int cudaDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(cudaGetDevice(&cudaDev) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int ccMajor, ccMinor;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 条件判断：(cudaDeviceGetAttribute(&ccMajor, cudaDevAttrComputeCapabilityMajor, cudaDev) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 9. 条件判断：(cudaDeviceGetAttribute(&ccMinor, cudaDevAttrComputeCapabilityMinor, cudaDev) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 函数返回：return ccMajor*10+ccMinor;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0991
 */
/* 中文逐项-函数 F0991：ncclCudaCompCap
 * 用途与位置：查询当前 CUDA 设备主次计算能力并编码成整数，供协议和架构策略判断。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::fillInfo；src/init.cc::ncclCommSetIntraProc。
 * 本体直接出现的调用/包装：cudaDeviceGetAttribute；cudaGetDevice。
 * 需要对照的边界/推进条件：cudaGetDevice(&cudaDev) != cudaSuccess；cudaDeviceGetAttribute(&ccMajor,
 * cudaDevAttrComputeCapabilityMajor, cudaDev) != cudaSuccess；cudaDeviceGetAttribute(&ccMinor,
 * cudaDevAttrComputeCapabilityMinor, cudaDev) != cudaSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return 0;；return ccMajor*10+ccMinor;。CHECK类宏还可能提前返回/跳转。
 */
int ncclCudaCompCap() {
/* 中文逐项-变量 V3537：cudaDev
 * cudaDev（int；函数 ncclCudaCompCap 内）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 此处未显式给初值；后续首个文本使用点：if (cudaGetDevice(&cudaDev) != cudaSuccess) return 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int cudaDev;
  if (cudaGetDevice(&cudaDev) != cudaSuccess) return 0;
/* 中文逐项-变量 V3538：ccMajor、ccMinor
 * ccMajor（int；函数 ncclCudaCompCap 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (cudaDeviceGetAttribute(&ccMajor, cudaDevAttrComputeCapabilityMajor, cudaDev) != cudaSuccess)
 * return 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 * ccMinor（int；函数 ncclCudaCompCap 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (cudaDeviceGetAttribute(&ccMinor, cudaDevAttrComputeCapabilityMinor, cudaDev) != cudaSuccess)
 * return 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ccMajor, ccMinor;
  if (cudaDeviceGetAttribute(&ccMajor, cudaDevAttrComputeCapabilityMajor, cudaDev) != cudaSuccess) return 0;
  if (cudaDeviceGetAttribute(&ccMinor, cudaDevAttrComputeCapabilityMinor, cudaDev) != cudaSuccess) return 0;
  return ccMajor*10+ccMinor;
}

/* 中文内部逻辑 F0992：int64ToBusId
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：sprintf(busId, "%04lx:%02lx:%02lx.%01lx", (id) >> 20, (id & 0xff000) >> 12, (id & 0xff0) >> 4, (id & 0xf));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0992
 */
/* 中文逐项-函数 F0992：int64ToBusId
 * 用途与位置：在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。
 * 参数 id（int64_t id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 busId（char* busId）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：sprintf。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t int64ToBusId(int64_t id, char* busId) {
  sprintf(busId, "%04lx:%02lx:%02lx.%01lx", (id) >> 20, (id & 0xff000) >> 12, (id & 0xff0) >> 4, (id & 0xf));
  return ncclSuccess;
}

/* 中文内部逻辑 F0993：busIdToInt64
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char hexStr[17];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int hexOffset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 int i = 0;；继续条件 hexOffset < sizeof(hexStr) - 1；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 准备局部状态：char c = busId[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   5. 条件判断：(c == '.' || c == ':')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     6. 真分支：；仅条件为真执行以下子项。
 *       7. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   8. 条件判断：((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     9. 真分支：；仅条件为真执行以下子项。
 *       10. 调用 / 计算 / 写入：hexStr[hexOffset++] = busId[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       12. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 * 13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 14. 调用 / 计算 / 写入：hexStr[hexOffset] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：*id = strtol(hexStr, NULL, 16);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0993
 */
/* 中文逐项-函数 F0993：busIdToInt64
 * 用途与位置：在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。
 * 参数 busId（const char* busId）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 参数 id（int64_t* id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::pciPathToInt64；src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddNvLinks；src/misc/utils.cc::getBusId；src/transport/p2p.cc::busIdToCudaDev。
 * 本体直接出现的调用/包装：strtol。
 * 需要对照的边界/推进条件：hexOffset < sizeof(hexStr) - 1；c == '.' || c == ':'；c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
 * (c >= 'a' && c <= 'f'。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t busIdToInt64(const char* busId, int64_t* id) {
/* 中文逐项-变量 V3539：hexStr
 * hexStr（char；函数 busIdToInt64 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int i = 0; hexOffset < sizeof(hexStr) - 1; i++) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char hexStr[17];  // Longest possible int64 hex string + null terminator.
/* 中文逐项-变量 V3540：hexOffset
 * hexOffset（int；函数 busIdToInt64 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 hexOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int hexOffset = 0;
  for (/* 中文逐项-变量 V3541：i
 * i（int；函数 busIdToInt64 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：hexOffset < sizeof(hexStr) - 1；循环体与增量表达式决定每轮如何推进。
 */int i = 0; hexOffset < sizeof(hexStr) - 1; i++) {
/* 中文逐项-变量 V3542：c
 * c（char；函数 busIdToInt64 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 c = busId[i]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char c = busId[i];
    if (c == '.' || c == ':') continue;
    if ((c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'F') ||
        (c >= 'a' && c <= 'f')) {
      hexStr[hexOffset++] = busId[i];
    } else break;
  }
  hexStr[hexOffset] = '\0';
  *id = strtol(hexStr, NULL, 16);
  return ncclSuccess;
}

// Convert a logical cudaDev index to the NVML device minor number
/* 中文内部逻辑 F0994：getBusId
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char busIdStr[] = "00000000:00:00.0";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：CUDACHECK(cudaDeviceGetPCIBusId(busIdStr, sizeof(busIdStr), cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(busIdToInt64(busIdStr, busId));；失败时可能提前return，成功才继续；在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0994
 */
/* 中文逐项-函数 F0994：getBusId
 * 用途与位置：查询本进程 CUDA 设备的 PCI 地址并转为整数标识，不能把设备 ordinal 当作 busId。
 * 参数 cudaDev（int cudaDev）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 参数 busId（int64_t *busId）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::commAlloc。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；busIdToInt64；cudaDeviceGetPCIBusId。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t getBusId(int cudaDev, int64_t *busId) {
  // On most systems, the PCI bus ID comes back as in the 0000:00:00.0
  // format. Still need to allocate proper space in case PCI domain goes
  // higher.
/* 中文逐项-变量 V3543：busIdStr
 * busIdStr（char；函数 getBusId 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 busIdStr = "00000000:00:00.0"；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  char busIdStr[] = "00000000:00:00.0";
  CUDACHECK(cudaDeviceGetPCIBusId(busIdStr, sizeof(busIdStr), cudaDev));
  NCCLCHECK(busIdToInt64(busIdStr, busId));
  return ncclSuccess;
}

/* 中文内部逻辑 F0995：getHostName
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(gethostname(hostname, maxlen) != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：strncpy(hostname, "unknown", maxlen);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int i = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 循环：初始 ；继续条件 ((hostname[i] != delim) && (hostname[i] != '\0') && (i < maxlen-1))；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 调用 / 计算 / 写入：i++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 9. 调用 / 计算 / 写入：hostname[i] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0995
 */
/* 中文逐项-函数 F0995：getHostName
 * 用途与位置：取得并按分隔符截短主机名，主要用于标识/日志。
 * 参数 hostname（char* hostname）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 maxlen（int maxlen）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 delim（const char delim）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/debug.cc::ncclDebugInit；src/debug.cc::ncclDebugLog；src/misc/utils.cc::getHostHash。
 * 本体直接出现的调用/包装：gethostname；strncpy。
 * 需要对照的边界/推进条件：gethostname(hostname, maxlen) != 0；hostname[i] != delim) && (hostname[i] != '\0') && (i <
 * maxlen-1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t getHostName(char* hostname, int maxlen, const char delim) {
  if (gethostname(hostname, maxlen) != 0) {
    strncpy(hostname, "unknown", maxlen);
    return ncclSystemError;
  }
/* 中文逐项-变量 V3544：i
 * i（int；函数 getHostName 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int i = 0;
  while ((hostname[i] != delim) && (hostname[i] != '\0') && (i < maxlen-1)) i++;
  hostname[i] = '\0';
  return ncclSuccess;
}

/* 中文内部逻辑 F0996：getHash
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：uint64_t result = 5381;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 int c = 0;；继续条件 c < n；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 调用 / 计算 / 写入：result = ((result << 5) + result) ^ string[c];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 5. 函数返回：return result;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0996
 */
/* 中文逐项-函数 F0996：getHash
 * 用途与位置：对给定字符串字节计算哈希，供主机/进程身份摘要使用，不承担安全认证。
 * 参数 string（const char* string）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank；src/misc/utils.cc::getHostHash；src/misc/utils.cc::getPidHash。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：c < n。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return result;。CHECK类宏还可能提前返回/跳转。
 */
uint64_t getHash(const char* string, int n) {
  // Based on DJB2a, result = result * 33 ^ char
/* 中文逐项-变量 V3545：result
 * result（uint64_t；函数 getHash 内）：异步线程/请求保存的结果码，前台测试函数需要检查它而不只检查长度。
 * 初值/来源 result = 5381；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  uint64_t result = 5381;
  for (/* 中文逐项-变量 V3546：c
 * c（int；函数 getHash 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c < n；循环体与增量表达式决定每轮如何推进。
 */int c = 0; c < n; c++) {
    result = ((result << 5) + result) ^ string[c];
  }
  return result;
}

/* Generate a hash of the unique identifying string for this host
 * that will be unique for both bare-metal and container instances
 * Equivalent of a hash of;
 *
 * $(hostname)$(cat /proc/sys/kernel/random/boot_id)
 *
 * This string can be overridden by using the NCCL_HOSTID env var.
 */
/* 中文逐项-常量 X1171：HOSTID_FILE
 * 编译期定义：HOSTID_FILE = "/proc/sys/kernel/random/boot_id"。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define HOSTID_FILE "/proc/sys/kernel/random/boot_id"
/* 中文内部逻辑 F0997：getHostHash
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char hostHash[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：char *hostId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：(void) getHostName(hostHash, sizeof(hostHash), '\0');；取得并按分隔符截短主机名，主要用于标识/日志。
 * 4. 准备局部状态：int offset = strlen(hostHash);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：((hostId = getenv("NCCL_HOSTID")) != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_HOSTID set by environment to %s", hostId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     8. 调用 / 计算 / 写入：strncpy(hostHash, hostId, sizeof(hostHash));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   9. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     10. 准备局部状态：FILE *file = fopen(HOSTID_FILE, "r");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     11. 条件判断：(file != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       12. 真分支：；仅条件为真执行以下子项。
 *         13. 准备局部状态：char *p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         14. 条件判断：(fscanf(file, "%ms", &p) == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           15. 真分支：；仅条件为真执行以下子项。
 *             16. 调用 / 计算 / 写入：strncpy(hostHash+offset, p, sizeof(hostHash)-offset-1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             17. 调用 / 计算 / 写入：free(p);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     18. 调用 / 计算 / 写入：fclose(file);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 19. 调用 / 计算 / 写入：hostHash[sizeof(hostHash)-1]='\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 调用 / 计算 / 写入：TRACE(NCCL_INIT,"unique hostname '%s'", hostHash);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 21. 函数返回：return getHash(hostHash, strlen(hostHash));；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0997
 */
/* 中文逐项-函数 F0997：getHostHash
 * 用途与位置：由主机/启动环境相关身份生成一致摘要，帮助识别同一主机上的参与者。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::fillInfo。
 * 本体直接出现的调用/包装：fclose；fopen；free；fscanf；getHash；getHostName；getenv；strlen；strncpy。
 * 需要对照的边界/推进条件：hostId = getenv("NCCL_HOSTID")) != NULL；file != NULL；fscanf(file, "%ms", &p) == 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return getHash(hostHash, strlen(hostHash));。CHECK类宏还可能提前返回/跳转。
 */
uint64_t getHostHash(void) {
/* 中文逐项-变量 V3547：hostHash
 * hostHash（char；函数 getHostHash 内）：用于区分同机/同进程的身份摘要；帮助判断哪些裸指针和共享资源可复用。
 * 此处未显式给初值；后续首个文本使用点：(void) getHostName(hostHash, sizeof(hostHash), '\0');。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char hostHash[1024];
/* 中文逐项-变量 V3548：hostId
 * hostId（char；函数 getHostHash 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if ((hostId = getenv("NCCL_HOSTID")) != NULL) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char *hostId;

  // Fall back is the full hostname if something fails
  (void) getHostName(hostHash, sizeof(hostHash), '\0');
/* 中文逐项-变量 V3549：offset
 * offset（int；函数 getHostHash 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = strlen(hostHash)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int offset = strlen(hostHash);

  if ((hostId = getenv("NCCL_HOSTID")) != NULL) {
    INFO(NCCL_ENV, "NCCL_HOSTID set by environment to %s", hostId);
    strncpy(hostHash, hostId, sizeof(hostHash));
  } else {
/* 中文逐项-变量 V3550：file
 * file（FILE；函数 getHostHash 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 初值/来源 file = fopen(HOSTID_FILE, "r")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    FILE *file = fopen(HOSTID_FILE, "r");
    if (file != NULL) {
/* 中文逐项-变量 V3551：p
 * p（char；函数 getHostHash 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：if (fscanf(file, "%ms", &p) == 1) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char *p;
      if (fscanf(file, "%ms", &p) == 1) {
        strncpy(hostHash+offset, p, sizeof(hostHash)-offset-1);
        free(p);
      }
    }
    fclose(file);
  }

  // Make sure the string is terminated
  hostHash[sizeof(hostHash)-1]='\0';

  TRACE(NCCL_INIT,"unique hostname '%s'", hostHash);

  return getHash(hostHash, strlen(hostHash));
}

/* Generate a hash of the unique identifying string for this process
 * that will be unique for both bare-metal and container instances
 * Equivalent of a hash of;
 *
 * $$ $(readlink /proc/self/ns/pid)
 */
/* 中文内部逻辑 F0998：getPidHash
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char pname[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：sprintf(pname, "%ld", (long) getpid());；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 3. 准备局部状态：int plen = strlen(pname);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int len = readlink("/proc/self/ns/pid", pname+plen, sizeof(pname)-1-plen);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(len < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：len = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：pname[plen+len]='\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：TRACE(NCCL_INIT,"unique PID '%s'", pname);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return getHash(pname, strlen(pname));；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0998
 */
/* 中文逐项-函数 F0998：getPidHash
 * 用途与位置：结合进程命名空间/进程标识形成摘要，用于判断是否同进程共享本地指针与发射状态。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::fillInfo。
 * 本体直接出现的调用/包装：getHash；getpid；readlink；sprintf；strlen。
 * 需要对照的边界/推进条件：len < 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return getHash(pname, strlen(pname));。CHECK类宏还可能提前返回/跳转。
 */
uint64_t getPidHash(void) {
/* 中文逐项-变量 V3552：pname
 * pname（char；函数 getPidHash 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sprintf(pname, "%ld", (long) getpid());。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char pname[1024];
  // Start off with our pid ($$)
  sprintf(pname, "%ld", (long) getpid());
/* 中文逐项-变量 V3553：plen
 * plen（int；函数 getPidHash 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 plen = strlen(pname)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int plen = strlen(pname);
/* 中文逐项-变量 V3554：len
 * len（int；函数 getPidHash 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 len = readlink("/proc/self/ns/pid", pname+plen, sizeof(pname)-1-plen)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int len = readlink("/proc/self/ns/pid", pname+plen, sizeof(pname)-1-plen);
  if (len < 0) len = 0;

  pname[plen+len]='\0';
  TRACE(NCCL_INIT,"unique PID '%s'", pname);

  return getHash(pname, strlen(pname));
}

/* 中文内部逻辑 F0999：parseStringList
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(!string)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：const char* ptr = string;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：int ifNum = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：int ifC = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 准备局部状态：char c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 先执行一次的循环：初始 ；继续条件 (ifNum < maxList && c)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *   9. 调用 / 计算 / 写入：c = *ptr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   10. 条件判断：(c == ':')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     11. 真分支：；仅条件为真执行以下子项。
 *       12. 条件判断：(ifC > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         13. 真分支：；仅条件为真执行以下子项。
 *           14. 调用 / 计算 / 写入：ifList[ifNum].prefix[ifC] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           15. 调用 / 计算 / 写入：ifList[ifNum].port = atoi(ptr+1);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           16. 调用 / 计算 / 写入：ifNum++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           17. 调用 / 计算 / 写入：ifC = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       18. 循环：初始 ；继续条件 (c != ',' && c != '\0')；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         19. 调用 / 计算 / 写入：c = *(++ptr);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       20. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     21. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       22. 条件判断：(c == ',' || c == '\0')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         23. 真分支：；仅条件为真执行以下子项。
 *           24. 条件判断：(ifC > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             25. 真分支：；仅条件为真执行以下子项。
 *               26. 调用 / 计算 / 写入：ifList[ifNum].prefix[ifC] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               27. 调用 / 计算 / 写入：ifList[ifNum].port = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               28. 调用 / 计算 / 写入：ifNum++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               29. 调用 / 计算 / 写入：ifC = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         30. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           31. 调用 / 计算 / 写入：ifList[ifNum].prefix[ifC] = c;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           32. 调用 / 计算 / 写入：ifC++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   33. 调用 / 计算 / 写入：ptr++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 34. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 35. 函数返回：return ifNum;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f0999
 */
/* 中文逐项-函数 F0999：parseStringList
 * 用途与位置：把逗号分隔的接口名前缀及可选端口解析成过滤表。
 * 参数 string（const char* string）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ifList（struct netIf* ifList）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 maxList（int maxList）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::findInterfaces；src/misc/socket.cc::ncclGetSocketAddrFromString；src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：atoi。
 * 需要对照的边界/推进条件：!string；c == ':'；ifC > 0；c != ',' && c != '\0'。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return 0;；return ifNum;。CHECK类宏还可能提前返回/跳转。
 */
int parseStringList(const char* string, struct netIf* ifList, int maxList) {
  if (!string) return 0;

/* 中文逐项-变量 V3555：ptr
 * ptr（char；函数 parseStringList 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = string；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const char* ptr = string;

/* 中文逐项-变量 V3556：ifNum
 * ifNum（int；函数 parseStringList 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ifNum = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int ifNum = 0;
/* 中文逐项-变量 V3557：ifC
 * ifC（int；函数 parseStringList 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ifC = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int ifC = 0;
/* 中文逐项-变量 V3558：c
 * c（char；函数 parseStringList 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：c = *ptr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char c;
  do {
    c = *ptr;
    if (c == ':') {
      if (ifC > 0) {
        ifList[ifNum].prefix[ifC] = '\0';
        ifList[ifNum].port = atoi(ptr+1);
        ifNum++; ifC = 0;
      }
      while (c != ',' && c != '\0') c = *(++ptr);
    } else if (c == ',' || c == '\0') {
      if (ifC > 0) {
        ifList[ifNum].prefix[ifC] = '\0';
        ifList[ifNum].port = -1;
        ifNum++; ifC = 0;
      }
    } else {
      ifList[ifNum].prefix[ifC] = c;
      ifC++;
    }
    ptr++;
  } while (ifNum < maxList && c);
  return ifNum;
}

/* 中文内部逻辑 F1000：matchIf
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int matchLen = matchExact ? strlen(string) + 1 : strlen(ref);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 函数返回：return strncmp(string, ref, matchLen) == 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f1000
 */
/* 中文逐项-函数 F1000：matchIf
 * 用途与位置：按完整名字/前缀及可选端口规则判断接口是否命中配置列表。
 * 参数 string（const char* string）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ref（const char* ref）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 matchExact（bool matchExact）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/utils.cc::matchIfList。
 * 本体直接出现的调用/包装：strlen；strncmp。
 * 源码返回表达式：return strncmp(string, ref, matchLen) == 0;。CHECK类宏还可能提前返回/跳转。
 */
static bool matchIf(const char* string, const char* ref, bool matchExact) {
  // Make sure to include '\0' in the exact case
/* 中文逐项-变量 V3559：matchLen
 * matchLen（int；函数 matchIf 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 matchLen = matchExact ? strlen(string) + 1 : strlen(ref)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int matchLen = matchExact ? strlen(string) + 1 : strlen(ref);
  return strncmp(string, ref, matchLen) == 0;
}

/* 中文内部逻辑 F1001：matchPort
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(port1 == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return true;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 条件判断：(port2 == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 函数返回：return true;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 条件判断：(port1 == port2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 函数返回：return true;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 函数返回：return false;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f1001
 */
/* 中文逐项-函数 F1001：matchPort
 * 用途与位置：按完整名字/前缀及可选端口规则判断接口是否命中配置列表。
 * 参数 port1（const int port1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 port2（const int port2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/utils.cc::matchIfList。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：port1 == -1；port2 == -1；port1 == port2。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return true;；return false;。CHECK类宏还可能提前返回/跳转。
 */
static bool matchPort(const int port1, const int port2) {
  if (port1 == -1) return true;
  if (port2 == -1) return true;
  if (port1 == port2) return true;
  return false;
}


/* 中文内部逻辑 F1002：matchIfList
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(listSize == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return true;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 循环：初始 int i=0;；继续条件 i<listSize；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 条件判断：(matchIf(string, ifList[i].prefix, matchExact) && matchPort(port, ifList[i].port))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     6. 真分支：；仅条件为真执行以下子项。
 *       7. 函数返回：return true;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 9. 函数返回：return false;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__utils_cc.md#f1002
 */
/* 中文逐项-函数 F1002：matchIfList
 * 用途与位置：按完整名字/前缀及可选端口规则判断接口是否命中配置列表。
 * 参数 string（const char* string）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 port（int port）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 参数 ifList（struct netIf* ifList）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 listSize（int listSize）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 参数 matchExact（bool matchExact）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::findInterfaces；src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：matchIf；matchPort。
 * 需要对照的边界/推进条件：listSize == 0；i<listSize；matchIf(string, ifList[i].prefix, matchExact) && matchPort(port,
 * ifList[i].port。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return true;；return false;。CHECK类宏还可能提前返回/跳转。
 */
bool matchIfList(const char* string, int port, struct netIf* ifList, int listSize, bool matchExact) {
  // Make an exception for the case where no user list is defined
  if (listSize == 0) return true;

  for (/* 中文逐项-变量 V3560：i
 * i（int；函数 matchIfList 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<listSize；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<listSize; i++) {
    if (matchIf(string, ifList[i].prefix, matchExact)
        && matchPort(port, ifList[i].port)) {
      return true;
    }
  }
  return false;
}
