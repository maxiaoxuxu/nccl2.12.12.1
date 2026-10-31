/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "graph.h"
#include "utils.h"

/* 中文精读：GPU 间共享内存地址如何跨进程表达
 * 同进程可交换直接指针；跨进程不能把一个进程的普通虚拟地址直接当作另一进程的可用地址。
 * CUDA IPC handle 表达可导入的 GPU 分配，接收进程打开后得到自己地址空间的映射。
 * p2p setup/connect 将映射结果填到 conn.buffs/head/tail，再由 GPU 原语访问。
 * NVLink/PCIe 是实际硬件路径，IPC 是建立地址映射的机制，两者解决的问题不同。
 */
/* 中文逐项-类型 T0452：ncclP2pBuff
 * 类型职责：GPU内存直接指针/IPC句柄及P2P建链策略摘要，用于导入对端分配。
 * 声明内可见成员（含嵌套成员）：directPtr、devIpc。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclP2pBuff {
/* 中文逐项-变量 V4572：directPtr
 * directPtr（void；成员 ncclP2pBuff）：同进程可直接使用的GPU指针，跨进程应经IPC导入建立自己的地址视图。
 */
  void* directPtr;
/* 中文逐项-变量 V4573：devIpc
 * devIpc（cudaIpcMemHandle_t；成员 ncclP2pBuff）：可跨进程交换的CUDA内存导出句柄，与导入后本进程实际地址不同。
 */
  cudaIpcMemHandle_t devIpc;
};

/* 中文逐项-类型 T0453：p2pConnectInfo
 * 类型职责：GPU内存直接指针/IPC句柄及P2P建链策略摘要，用于导入对端分配。
 * 声明内可见成员（含嵌套成员）：rank、read、p2pBuff。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct p2pConnectInfo {
/* 中文逐项-变量 V4574：rank
 * rank（int；成员 p2pConnectInfo）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V4575：read
 * read（int；成员 p2pConnectInfo）：P2P/GDR方向是否采用读/拉取策略，发送读显存和接收写显存可能有不同能力条件。
 */
  int read;
/* 中文逐项-变量 V4576：p2pBuff
 * p2pBuff（struct ncclP2pBuff；成员 p2pConnectInfo）：P2P GPU分配的直接指针与IPC句柄摘要，connect时转换成本进程可用映射。
 */
  struct ncclP2pBuff p2pBuff;
};
static_assert(sizeof(p2pConnectInfo) <= CONNECT_SIZE, "P2P Connect info is too large");

/* 中文逐项-类型 T0454：p2pSendResources
 * 类型职责：对应方向的P2P控制区和导入映射，需按拥有分配/导入引用分别释放。
 * 声明内可见成员（含嵌套成员）：devMem、sendMemIpc、recvMemIpc。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct p2pSendResources {
/* 中文逐项-变量 V4577：devMem
 * devMem（struct ncclSendMem；成员 p2pSendResources）：当前方向拥有的主机或设备分配，地址空间由字段名及类型共同确认。
 */
  struct ncclSendMem* devMem;
/* 中文逐项-变量 V4578：sendMemIpc
 * sendMemIpc（void；成员 p2pSendResources）：P2P导入控制区的CUDA IPC映射，释放时需要关闭对应句柄。
 */
  void* sendMemIpc;
/* 中文逐项-变量 V4579：recvMemIpc
 * recvMemIpc（void；成员 p2pSendResources）：P2P导入控制区的CUDA IPC映射，释放时需要关闭对应句柄。
 */
  void* recvMemIpc;
};

/* 中文逐项-类型 T0455：p2pRecvResources
 * 类型职责：对应方向的P2P控制区和导入映射，需按拥有分配/导入引用分别释放。
 * 声明内可见成员（含嵌套成员）：devMem、sendMemIpc、recvMemIpc。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct p2pRecvResources {
/* 中文逐项-变量 V4580：devMem
 * devMem（struct ncclRecvMem；成员 p2pRecvResources）：当前方向拥有的主机或设备分配，地址空间由字段名及类型共同确认。
 */
  struct ncclRecvMem* devMem;
/* 中文逐项-变量 V4581：sendMemIpc
 * sendMemIpc（void；成员 p2pRecvResources）：P2P导入控制区的CUDA IPC映射，释放时需要关闭对应句柄。
 */
  void* sendMemIpc;
/* 中文逐项-变量 V4582：recvMemIpc
 * recvMemIpc（void；成员 p2pRecvResources）：P2P导入控制区的CUDA IPC映射，释放时需要关闭对应句柄。
 */
  void* recvMemIpc;
};

#include <sys/types.h>

/* Convert a PCI busId string into a local cudaDev device index (cf. CUDA_VISIBLE_DEVICES) */
/* 中文内部逻辑 F1153：busIdToCudaDev
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int ndev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(cudaGetDeviceCount(&ndev) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return -1;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 循环：初始 int i = 0;；继续条件 i < ndev；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 准备局部状态：char devBusIdStr[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   7. 条件判断：(cudaDeviceGetPCIBusId(devBusIdStr, NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE, i) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     8. 真分支：；仅条件为真执行以下子项。
 *       9. 函数返回：return -1;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   10. 准备局部状态：int64_t devBusId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   11. 调用 / 计算 / 写入：NCCLCHECK(busIdToInt64(devBusIdStr, &devBusId));；失败时可能提前return，成功才继续；在 PCI BDF 字符串和整数标识之间转换，统一关联 CUDA/NVML/NET 的同一硬件设备。
 *   12. 条件判断：(busId == devBusId)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     13. 真分支：；仅条件为真执行以下子项。
 *       14. 函数返回：return i;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 15. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 16. 函数返回：return -1;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1153
 */
/* 中文逐项-函数 F1153：busIdToCudaDev
 * 用途与位置：把 GPU PCI busId 映射到本进程 CUDA_VISIBLE_DEVICES 下可见设备序号。
 * 参数 busId（int64_t busId）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/p2p.cc::p2pCanConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；busIdToInt64；cudaDeviceGetPCIBusId；cudaGetDeviceCount。
 * 需要对照的边界/推进条件：cudaGetDeviceCount(&ndev) != cudaSuccess；i < ndev；cudaDeviceGetPCIBusId(devBusIdStr,
 * NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE, i) != cudaSuccess；busId == devBusId。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return -1;；return i;。CHECK类宏还可能提前返回/跳转。
 */
static int busIdToCudaDev(int64_t busId) {
/* 中文逐项-变量 V4583：ndev
 * ndev（int；函数 busIdToCudaDev 内）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 * 此处未显式给初值；后续首个文本使用点：if (cudaGetDeviceCount(&ndev) != cudaSuccess)。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ndev;
  if (cudaGetDeviceCount(&ndev) != cudaSuccess)
    return -1;
  for (/* 中文逐项-变量 V4584：i
 * i（int；函数 busIdToCudaDev 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < ndev；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < ndev; i++) {
/* 中文逐项-变量 V4585：devBusIdStr
 * devBusIdStr（char；函数 busIdToCudaDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (cudaDeviceGetPCIBusId(devBusIdStr, NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE, i) !=
 * cudaSuccess)。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char devBusIdStr[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
    if (cudaDeviceGetPCIBusId(devBusIdStr, NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE, i) != cudaSuccess)
      return -1;
/* 中文逐项-变量 V4586：devBusId
 * devBusId（int64_t；函数 busIdToCudaDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(busIdToInt64(devBusIdStr, &devBusId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int64_t devBusId;
    NCCLCHECK(busIdToInt64(devBusIdStr, &devBusId));
    if (busId == devBusId) return i;
  }
  // BusId was not found in our locally visible CUDA devices
  return -1;
}

/* Determine if two peers can communicate through p2p */
/* 中文导读：
 * 判断两个 GPU 是否能走 P2P，包括拓扑、CUDA peer access/IPC 能力等条件。
 * 这里的 GPU P2P transport 与公开 ncclSend/ncclRecv 的点对点语义不是同一个层次。
 */
/* 中文内部逻辑 F1154：p2pCanConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(info1->hostHash != info2->hostHash || info1->shmDev != info2->shmDev)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：*ret = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int intermediateRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCheckP2p(topo, info1->busId, info2->busId, ret, NULL, &intermediateRank));；失败时可能提前return，成功才继续；结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。
 * 7. 条件判断：(*ret == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 条件判断：(intermediateRank != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 13. 准备局部状态：int cudaDev1 = busIdToCudaDev(info1->busId);；把 GPU PCI busId 映射到本进程 CUDA_VISIBLE_DEVICES 下可见设备序号。
 * 14. 准备局部状态：int cudaDev2 = busIdToCudaDev(info2->busId);；把 GPU PCI busId 映射到本进程 CUDA_VISIBLE_DEVICES 下可见设备序号。
 * 15. 条件判断：(cudaDev1 == -1 || cudaDev2 == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   16. 真分支：；仅条件为真执行以下子项。
 *     17. 编译期分支/指令：#if CUDART_VERSION >= 10010；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *       19. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *         20. 调用 / 计算 / 写入：*ret = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         21. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 22. 准备局部状态：int p2p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 23. 条件判断：(cudaDeviceCanAccessPeer(&p2p, cudaDev1, cudaDev2) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   24. 真分支：；仅条件为真执行以下子项。
 *     25. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_P2P,"peer query failed between dev %d(=%lx) and dev %d(=%lx)", cudaDev1, info1->busId, cudaDev2, info2->busId);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     26. 调用 / 计算 / 写入：*ret = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     27. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 28. 条件判断：(p2p != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   29. 真分支：；仅条件为真执行以下子项。
 *     30. 准备局部状态：static int legacyIPC = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     31. 条件判断：(legacyIPC >= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       32. 真分支：；仅条件为真执行以下子项。
 *         33. 调用 / 计算 / 写入：*ret = legacyIPC;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         34. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     35. 准备局部状态：char *dummy;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     36. 准备局部状态：cudaIpcMemHandle_t ipc;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     37. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaCalloc(&dummy, CUDA_IPC_MIN));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     38. 条件判断：(cudaIpcGetMemHandle(&ipc, dummy) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       39. 真分支：；仅条件为真执行以下子项。
 *         40. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_P2P,"Legacy IPC not supported");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         41. 调用 / 计算 / 写入：*ret = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     42. 调用 / 计算 / 写入：CUDACHECK(cudaFree(dummy));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     43. 调用 / 计算 / 写入：legacyIPC = *ret;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     44. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 45. 条件判断：(p2p == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   46. 真分支：；仅条件为真执行以下子项。
 *     47. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_P2P,"Could not enable P2P between dev %d(=%lx) and dev %d(=%lx)", cudaDev1, info1->busId, cudaDev2, info2->busId);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     48. 调用 / 计算 / 写入：*ret = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     49. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 50. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1154
 */
/* 中文逐项-函数 F1154：p2pCanConnect
 * 用途与位置：检查同机/共享环境、拓扑策略、CUDA peer access 与 IPC 支持，决定能否走 GPU P2P transport。
 * 参数 ret（int* ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 topo（struct ncclTopoSystem* topo）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 info1（struct ncclPeerInfo* info1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 info2（struct ncclPeerInfo* info2）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；busIdToCudaDev；cudaDeviceCanAccessPeer；cudaFree；cudaIpcGetMemHandle；ncclCudaCalloc；ncclTopoCheckP2p。
 * 需要对照的边界/推进条件：info1->hostHash != info2->hostHash || info1->shmDev != info2->shmDev；*ret == 0；intermediateRank !=
 * -1；cudaDev1 == -1 || cudaDev2 == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t p2pCanConnect(int* ret, struct ncclTopoSystem* topo, struct ncclTopoGraph* graph, struct ncclPeerInfo* info1, struct ncclPeerInfo* info2) {
  // Rule out different nodes / isolated containers
  if (info1->hostHash != info2->hostHash || info1->shmDev != info2->shmDev) {
    *ret = 0;
    return ncclSuccess;
  }

  // Check topology / p2p level.
/* 中文逐项-变量 V4587：intermediateRank
 * intermediateRank（int；函数 p2pCanConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCheckP2p(topo, info1->busId, info2->busId, ret, NULL,
 * &intermediateRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int intermediateRank;
  NCCLCHECK(ncclTopoCheckP2p(topo, info1->busId, info2->busId, ret, NULL, &intermediateRank));
  if (*ret == 0) return ncclSuccess;
  if (intermediateRank != -1) return ncclSuccess;

  // Convert the peer's busId into a local cudaDev index (cf. CUDA_VISIBLE_DEVICES)
/* 中文逐项-变量 V4588：cudaDev1
 * cudaDev1（int；函数 p2pCanConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cudaDev1 = busIdToCudaDev(info1->busId)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int cudaDev1 = busIdToCudaDev(info1->busId);
/* 中文逐项-变量 V4589：cudaDev2
 * cudaDev2（int；函数 p2pCanConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cudaDev2 = busIdToCudaDev(info2->busId)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int cudaDev2 = busIdToCudaDev(info2->busId);
  if (cudaDev1 == -1 || cudaDev2 == -1) {
#if CUDART_VERSION >= 10010
    // CUDA 10.1 and later can use P2P with invisible devices.
    return ncclSuccess;
#else
    // Peer's CUDA device is not visible in this process : we can't communicate with it.
    *ret = 0;
    return ncclSuccess;
#endif
  }

  // Check that CUDA can do P2P
/* 中文逐项-变量 V4590：p2p
 * p2p（int；函数 p2pCanConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (cudaDeviceCanAccessPeer(&p2p, cudaDev1, cudaDev2) != cudaSuccess) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int p2p;
  if (cudaDeviceCanAccessPeer(&p2p, cudaDev1, cudaDev2) != cudaSuccess) {
    INFO(NCCL_INIT|NCCL_P2P,"peer query failed between dev %d(=%lx) and dev %d(=%lx)",
         cudaDev1, info1->busId, cudaDev2, info2->busId);
    *ret = 0;
    return ncclSuccess;
  }

  if (p2p != 0) {
    // Cached result of the legacyIPC detection
/* 中文逐项-变量 V4591：legacyIPC
 * legacyIPC（int；函数 p2pCanConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 legacyIPC = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
    static int legacyIPC = -1;
    if (legacyIPC >= 0) {
      *ret = legacyIPC;
      return ncclSuccess;
    }
    // Check that legacy IPC support is available (WSL WAR)
/* 中文逐项-变量 V4592：dummy
 * dummy（char；函数 p2pCanConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCudaCalloc(&dummy, CUDA_IPC_MIN));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char *dummy;
/* 中文逐项-变量 V4593：ipc
 * ipc（cudaIpcMemHandle_t；函数 p2pCanConnect 内）：可跨进程交换的CUDA内存导出句柄，与导入后本进程实际地址不同。
 * 此处未显式给初值；后续首个文本使用点：if (cudaIpcGetMemHandle(&ipc, dummy) != cudaSuccess) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    cudaIpcMemHandle_t ipc;
    NCCLCHECK(ncclCudaCalloc(&dummy, CUDA_IPC_MIN));
    if (cudaIpcGetMemHandle(&ipc, dummy) != cudaSuccess) {
      INFO(NCCL_INIT|NCCL_P2P,"Legacy IPC not supported");
      *ret = 0;
    }
    CUDACHECK(cudaFree(dummy));
    legacyIPC = *ret;
    return ncclSuccess;
  }

  if (p2p == 0) {
    INFO(NCCL_INIT|NCCL_P2P,"Could not enable P2P between dev %d(=%lx) and dev %d(=%lx)",
         cudaDev1, info1->busId, cudaDev2, info2->busId);
    *ret = 0;
    return ncclSuccess;
  }
  return ncclSuccess;
}

/* 中文逐项-宏 M0160：TRACE_DUMP_IPC
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 TRACE_DUMP_IPC；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define TRACE_DUMP_IPC(DEVIPC)                                                             \
  do {                                                                                     \
    unsigned long *devIpc = (unsigned long *) (DEVIPC);                                    \
    TRACE(P2P,"IPC: %016lx %016lx %016lx %016lx", devIpc[0], devIpc[1], devIpc[2], devIpc[3]); \
    TRACE(P2P,"IPC: %016lx %016lx %016lx %016lx", devIpc[4], devIpc[5], devIpc[6], devIpc[7]); \
  } while (0)


// Setting this to non zero causes P2P to use Reads rather than Writes
/* 中文逐项-参数getter P0054：P2pReadEnable
 * 生成函数 ncclParamP2pReadEnable()，读取 NCCL_P2P_READ_ENABLE，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(P2pReadEnable, "P2P_READ_ENABLE", -2);
/* 中文逐项-参数getter P0055：P2pDirectDisable
 * 生成函数 ncclParamP2pDirectDisable()，读取 NCCL_P2P_DIRECT_DISABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(P2pDirectDisable, "P2P_DIRECT_DISABLE", 0);

/* 中文内部逻辑 F1155：p2pGetInfo
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int p2p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCheckP2p(topo, info1->busId, info2->busId, &p2p, read, intermediateRank));；失败时可能提前return，成功才继续；结合拓扑距离、环境策略和硬件 P2P 状态判断 GPU 间能否直接访问及是否需中间 GPU。
 * 3. 准备局部状态：int readEnable = ncclParamP2pReadEnable();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 条件判断：(readEnable != -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：*read = readEnable;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1155
 */
/* 中文逐项-函数 F1155：p2pGetInfo
 * 用途与位置：查询 GPU 对的 P2P 可达性、读写模式及可能的中间 rank，把拓扑结果提供给 setup。
 * 参数 topo（struct ncclTopoSystem* topo）：本 rank 可见且裁剪后的硬件拓扑模型，供路径与通信图搜索使用。
 * 参数 info1（struct ncclPeerInfo* info1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 info2（struct ncclPeerInfo* info2）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 read（int* read）：P2P/GDR方向是否采用读/拉取策略，发送读显存和接收写显存可能有不同能力条件。
 * 参数 intermediateRank（int* intermediateRank）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/p2p.cc::p2pSendSetup；src/transport/p2p.cc::p2pRecvSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclParamP2pReadEnable；ncclTopoCheckP2p。
 * 需要对照的边界/推进条件：readEnable != -2。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t p2pGetInfo(struct ncclTopoSystem* topo, struct ncclPeerInfo* info1, struct ncclPeerInfo* info2, int* read, int* intermediateRank) {
/* 中文逐项-变量 V4594：p2p
 * p2p（int；函数 p2pGetInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCheckP2p(topo, info1->busId, info2->busId, &p2p, read,
 * intermediateRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int p2p;
  // Queries the topology to see if the GPUs are Ampere and
  // connected via NVLink, if so we enable P2P Read by default
  NCCLCHECK(ncclTopoCheckP2p(topo, info1->busId, info2->busId, &p2p, read, intermediateRank));

/* 中文逐项-变量 V4595：readEnable
 * readEnable（int；函数 p2pGetInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 readEnable = ncclParamP2pReadEnable()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
  int readEnable = ncclParamP2pReadEnable();
  if (readEnable != -2) *read = readEnable;
  return ncclSuccess;
}

/* 中文导读：
 * 把对端 GPU 缓冲区映射到本进程可用的设备地址，同进程直连与跨进程 CUDA IPC 分开处理。
 * 跨进程交换 IPC handle 后在本地打开映射，不是把另一个进程的裸虚拟地址直接拿来解引用。
 */
/* 中文内部逻辑 F1156：p2pMap
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(myInfo->pidHash == peerInfo->pidHash)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 条件判断：(peerInfo->cudaDev != myInfo->cudaDev)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       4. 真分支：；仅条件为真执行以下子项。
 *         5. 准备局部状态：cudaError_t err = cudaDeviceEnablePeerAccess(peerInfo->cudaDev, 0);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         6. 条件判断：(err == cudaErrorPeerAccessAlreadyEnabled)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           7. 真分支：；仅条件为真执行以下子项。
 *             8. 调用 / 计算 / 写入：cudaGetLastError();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           9. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             10. 条件判断：(err != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               11. 真分支：；仅条件为真执行以下子项。
 *                 12. 调用 / 计算 / 写入：WARN("failed to peer with device %d(=%lx): %d %s", peerInfo->cudaDev, peerInfo->busId, err, cudaGetErrorString(err));；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 13. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     14. 调用 / 计算 / 写入：*devMem = p2pBuff->directPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     15. 调用 / 计算 / 写入：*ipcPtr = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   16. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     17. 调用 / 计算 / 写入：CUDACHECK(cudaIpcOpenMemHandle(devMem, p2pBuff->devIpc, cudaIpcMemLazyEnablePeerAccess));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     18. 调用 / 计算 / 写入：*ipcPtr = *devMem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1156
 */
/* 中文逐项-函数 F1156：p2pMap
 * 用途与位置：同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。
 * 参数 myInfo（struct ncclPeerInfo* myInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peerInfo（struct ncclPeerInfo* peerInfo）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 参数 p2pBuff（struct ncclP2pBuff* p2pBuff）：P2P GPU分配的直接指针与IPC句柄摘要，connect时转换成本进程可用映射。
 * 参数 devMem（void** devMem）：当前方向拥有的主机或设备分配，地址空间由字段名及类型共同确认。
 * 参数 ipcPtr（void** ipcPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/p2p.cc::p2pSendSetup；src/transport/p2p.cc::p2pRecvSetup；src/transport/p2p.cc::p2pSendConnect；src/transport/p2p.cc::p2pRecvConnect。
 * 本体直接出现的调用/包装：CUDACHECK；cudaDeviceEnablePeerAccess；cudaGetErrorString；cudaGetLastError；cudaIpcOpenMemHandle。
 * 需要对照的边界/推进条件：myInfo->pidHash == peerInfo->pidHash；peerInfo->cudaDev != myInfo->cudaDev；err ==
 * cudaErrorPeerAccessAlreadyEnabled；err != cudaSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t p2pMap(struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo, struct ncclP2pBuff* p2pBuff, void** devMem, void** ipcPtr) {
  if (myInfo->pidHash == peerInfo->pidHash) {
    if (peerInfo->cudaDev != myInfo->cudaDev) {
      // Enable P2P access
/* 中文逐项-变量 V4596：err
 * err（cudaError_t；函数 p2pMap 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 err = cudaDeviceEnablePeerAccess(peerInfo->cudaDev, 0)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      cudaError_t err = cudaDeviceEnablePeerAccess(peerInfo->cudaDev, 0);
      if (err == cudaErrorPeerAccessAlreadyEnabled) {
        cudaGetLastError();
      } else if (err != cudaSuccess) {
        WARN("failed to peer with device %d(=%lx): %d %s",
            peerInfo->cudaDev, peerInfo->busId, err, cudaGetErrorString(err));
        return ncclInternalError;
      }
    }
    *devMem = p2pBuff->directPtr;
    *ipcPtr = NULL;
  } else {
    CUDACHECK(cudaIpcOpenMemHandle(devMem, p2pBuff->devIpc, cudaIpcMemLazyEnablePeerAccess));
    *ipcPtr = *devMem;
  }
  return ncclSuccess;
}

/* Send: Create and return connect structures for this peer to connect to me */
/* 中文内部逻辑 F1157：p2pSendSetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct p2pSendResources* resources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&resources, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：send->transportResources = resources;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 准备局部状态：int useRead, intermediateRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead, &intermediateRank));；失败时可能提前return，成功才继续；查询 GPU 对的 P2P 可达性、读写模式及可能的中间 rank，把拓扑结果提供给 setup。
 * 6. 源码语法片段：static_assert(sizeof(struct p2pConnectInfo) <= sizeof(struct ncclConnect), "p2p Connect Info is too big");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 7. 准备局部状态：struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：info->read = useRead;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 条件判断：(graph && connIndex == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 调用 / 计算 / 写入：info->read = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 准备局部状态：const char* useReadStr = info->read ? "/read" : "";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：int sendSize = sizeof(struct ncclSendMem);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 条件判断：(info->read)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 调用 / 计算 / 写入：sendSize += send->comm->buffSizes[NCCL_PROTO_SIMPLE];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：ALIGN_SIZE(sendSize, CUDA_IPC_MIN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 18. 条件判断：(intermediateRank == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   19. 真分支：；仅条件为真执行以下子项。
 *     20. 调用 / 计算 / 写入：info->rank = myInfo->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     21. 条件判断：(myInfo->pidHash == peerInfo->pidHash)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       22. 真分支：；仅条件为真执行以下子项。
 *         23. 条件判断：(ncclParamP2pDirectDisable() == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           24. 真分支：；仅条件为真执行以下子项。
 *             25. 调用 / 计算 / 写入：send->conn.direct |= info->read ? NCCL_DIRECT_READ : NCCL_DIRECT_WRITE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         26. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_P2P, "Channel %02d : %d[%lx] -> %d[%lx] via P2P/direct pointer%s", channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, useReadStr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       27. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         28. 调用 / 计算 / 写入：send->conn.direct |= info->read ? NCCL_IPC_READ : NCCL_IPC_WRITE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         29. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_P2P,"Channel %02d : %d[%lx] -> %d[%lx] via P2P/IPC%s", channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, useReadStr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   30. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     31. 调用 / 计算 / 写入：info->rank = intermediateRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     32. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_P2P, "Channel %02d : %d[%lx] -> %d[%lx] via P2P/indirect/%d[%lx]%s", channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, intermediateRank, comm->peerInfo[inter …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 33. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_P2P, 1, info->rank, &send->proxyConn));；失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。
 * 34. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgSetup, &sendSize, sizeof(int), &info->p2pBuff, sizeof(struct ncclP2pBuff)));；失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 35. 调用 / 计算 / 写入：NCCLCHECK(p2pMap(myInfo, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&resources->devMem, &resources->sendMemIpc));；失败时可能提前return，成功才继续；同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。
 * 36. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1157
 */
/* 中文逐项-函数 F1157：p2pSendSetup
 * 用途与位置：按方向分配/导出 GPU P2P 控制和协议缓冲，必要时借助中间 GPU 代理准备资源。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 myInfo（struct ncclPeerInfo* myInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peerInfo（struct ncclPeerInfo* peerInfo）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ALIGN_SIZE；NCCLCHECK；ncclCalloc；ncclParamP2pDirectDisable；ncclProxyCall；ncclProxyConnect；p2pGetInfo；p2pMap。
 * 需要对照的边界/推进条件：graph && connIndex == 1；info->read；intermediateRank == -1；myInfo->pidHash ==
 * peerInfo->pidHash。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t p2pSendSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo,
    struct ncclConnect* connectInfo, struct ncclConnector* send, int channelId, int connIndex) {
/* 中文逐项-变量 V4597：resources
 * resources（struct p2pSendResources；函数 p2pSendSetup 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&resources, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct p2pSendResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  send->transportResources = resources;
/* 中文逐项-变量 V4598：useRead、intermediateRank
 * useRead（int；函数 p2pSendSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead,
 * &intermediateRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 * intermediateRank（int；函数 p2pSendSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead,
 * &intermediateRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int useRead, intermediateRank;
  NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead, &intermediateRank));

  static_assert(sizeof(struct p2pConnectInfo) <= sizeof(struct ncclConnect), "p2p Connect Info is too big");
/* 中文逐项-变量 V4599：info
 * info（struct p2pConnectInfo；函数 p2pSendSetup 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct p2pConnectInfo*)connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;
  info->read = useRead;
  // For CollNet, use write for scatter-reduce (conn 1), read for broadcast-gather (conn 0)
  if (graph && connIndex == 1) info->read = 0;
/* 中文逐项-变量 V4600：useReadStr
 * useReadStr（char；函数 p2pSendSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 useReadStr = info->read ? "/read" : ""；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  const char* useReadStr = info->read ? "/read" : "";

/* 中文逐项-变量 V4601：sendSize
 * sendSize（int；函数 p2pSendSetup 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 sendSize = sizeof(struct ncclSendMem)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int sendSize = sizeof(struct ncclSendMem);
  // For P2P Read the SIMPLE buffer is tagged on the end of the ncclSendMem structure
  if (info->read) sendSize += send->comm->buffSizes[NCCL_PROTO_SIMPLE];
  ALIGN_SIZE(sendSize, CUDA_IPC_MIN);

  if (intermediateRank == -1) {
    info->rank = myInfo->rank;
    if (myInfo->pidHash == peerInfo->pidHash) {
      if (ncclParamP2pDirectDisable() == 0) send->conn.direct |= info->read ? NCCL_DIRECT_READ : NCCL_DIRECT_WRITE;
      INFO(NCCL_INIT|NCCL_P2P, "Channel %02d : %d[%lx] -> %d[%lx] via P2P/direct pointer%s",
          channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, useReadStr);
    } else {
      send->conn.direct |= info->read ? NCCL_IPC_READ : NCCL_IPC_WRITE;
      INFO(NCCL_INIT|NCCL_P2P,"Channel %02d : %d[%lx] -> %d[%lx] via P2P/IPC%s",
          channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, useReadStr);
    }
  } else {
    info->rank = intermediateRank;
    INFO(NCCL_INIT|NCCL_P2P, "Channel %02d : %d[%lx] -> %d[%lx] via P2P/indirect/%d[%lx]%s",
        channelId, myInfo->rank, myInfo->busId, peerInfo->rank, peerInfo->busId, intermediateRank,
	comm->peerInfo[intermediateRank].busId, useReadStr);
  }

  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_P2P, 1, info->rank, &send->proxyConn));
  NCCLCHECK(ncclProxyCall(&send->proxyConn, ncclProxyMsgSetup, &sendSize, sizeof(int), &info->p2pBuff, sizeof(struct ncclP2pBuff)));

  NCCLCHECK(p2pMap(myInfo, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&resources->devMem, &resources->sendMemIpc));
  return ncclSuccess;
}

/* Create and return connect structures for this peer to connect to me */
/* 中文内部逻辑 F1158：p2pRecvSetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct p2pRecvResources* resources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&resources, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：recv->transportResources = resources;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 准备局部状态：int useRead, intermediateRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead, &intermediateRank));；失败时可能提前return，成功才继续；查询 GPU 对的 P2P 可达性、读写模式及可能的中间 rank，把拓扑结果提供给 setup。
 * 6. 源码语法片段：static_assert(sizeof(struct p2pConnectInfo) <= sizeof(struct ncclConnect), "p2p Connect Info is too big");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 7. 准备局部状态：struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：info->read = useRead;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 条件判断：(graph && connIndex == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 调用 / 计算 / 写入：info->read = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 准备局部状态：int recvSize = sizeof(struct ncclRecvMem);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   14. 条件判断：(!(info->read && p == NCCL_PROTO_SIMPLE))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     15. 真分支：；仅条件为真执行以下子项。
 *       16. 调用 / 计算 / 写入：recvSize += recv->comm->buffSizes[p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 18. 调用 / 计算 / 写入：ALIGN_SIZE(recvSize, CUDA_IPC_MIN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 19. 条件判断：(intermediateRank == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 调用 / 计算 / 写入：info->rank = myInfo->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     22. 条件判断：(myInfo->pidHash == peerInfo->pidHash)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       23. 真分支：；仅条件为真执行以下子项。
 *         24. 条件判断：(ncclParamP2pDirectDisable() == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           25. 真分支：；仅条件为真执行以下子项。
 *             26. 调用 / 计算 / 写入：recv->conn.direct |= info->read ? NCCL_DIRECT_READ : NCCL_DIRECT_WRITE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       27. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         28. 调用 / 计算 / 写入：recv->conn.direct |= info->read ? NCCL_IPC_READ : NCCL_IPC_WRITE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   29. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     30. 调用 / 计算 / 写入：info->rank = intermediateRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 31. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_P2P, 0, info->rank, &recv->proxyConn));；失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。
 * 32. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgSetup, &recvSize, sizeof(int), &info->p2pBuff, sizeof(struct ncclP2pBuff)));；失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 33. 调用 / 计算 / 写入：NCCLCHECK(p2pMap(myInfo, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&resources->devMem, &resources->recvMemIpc));；失败时可能提前return，成功才继续；同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。
 * 34. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1158
 */
/* 中文逐项-函数 F1158：p2pRecvSetup
 * 用途与位置：按方向分配/导出 GPU P2P 控制和协议缓冲，必要时借助中间 GPU 代理准备资源。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 graph（struct ncclTopoGraph* graph）：通信搜索图或 CUDA Graph 句柄；请按声明类型区分两个不同层次的图。
 * 参数 myInfo（struct ncclPeerInfo* myInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peerInfo（struct ncclPeerInfo* peerInfo）：初始化收集的参与者身份和能力摘要表，用于建图与选择传输。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 recv（struct ncclConnector * recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ALIGN_SIZE；NCCLCHECK；ncclCalloc；ncclParamP2pDirectDisable；ncclProxyCall；ncclProxyConnect；p2pGetInfo；p2pMap。
 * 需要对照的边界/推进条件：graph && connIndex == 1；p<NCCL_NUM_PROTOCOLS；!(info->read && p == NCCL_PROTO_SIMPLE；intermediateRank
 * == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t p2pRecvSetup(struct ncclComm* comm, struct ncclTopoGraph* graph, struct ncclPeerInfo* myInfo, struct ncclPeerInfo* peerInfo,
    struct ncclConnect* connectInfo, struct ncclConnector * recv, int channelId, int connIndex) {
/* 中文逐项-变量 V4602：resources
 * resources（struct p2pRecvResources；函数 p2pRecvSetup 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&resources, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct p2pRecvResources* resources;
  NCCLCHECK(ncclCalloc(&resources, 1));
  recv->transportResources = resources;
/* 中文逐项-变量 V4603：useRead、intermediateRank
 * useRead（int；函数 p2pRecvSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead,
 * &intermediateRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 * intermediateRank（int；函数 p2pRecvSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead,
 * &intermediateRank));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int useRead, intermediateRank;
  NCCLCHECK(p2pGetInfo(comm->topo, myInfo, peerInfo, &useRead, &intermediateRank));

  static_assert(sizeof(struct p2pConnectInfo) <= sizeof(struct ncclConnect), "p2p Connect Info is too big");
/* 中文逐项-变量 V4604：info
 * info（struct p2pConnectInfo；函数 p2pRecvSetup 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct p2pConnectInfo*)connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;
  info->read = useRead;
  // For CollNet, use write for scatter-reduce (conn 1), read for broadcast-gather (conn 0)
  if (graph && connIndex == 1) info->read = 0;

/* 中文逐项-变量 V4605：recvSize
 * recvSize（int；函数 p2pRecvSetup 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 recvSize = sizeof(struct ncclRecvMem)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int recvSize = sizeof(struct ncclRecvMem);
  // For P2P Read the SIMPLE buffer is tagged on the end of the ncclSendMem structure
  for (/* 中文逐项-变量 V4606：p
 * p（int；函数 p2pRecvSetup 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) if (!(info->read && p == NCCL_PROTO_SIMPLE)) recvSize += recv->comm->buffSizes[p];
  ALIGN_SIZE(recvSize, CUDA_IPC_MIN);

  if (intermediateRank == -1) {
    info->rank = myInfo->rank;
    if (myInfo->pidHash == peerInfo->pidHash) {
      if (ncclParamP2pDirectDisable() == 0) recv->conn.direct |= info->read ? NCCL_DIRECT_READ : NCCL_DIRECT_WRITE;
    } else {
      recv->conn.direct |= info->read ? NCCL_IPC_READ : NCCL_IPC_WRITE;
    }
  } else {
    info->rank = intermediateRank;
  }

  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_P2P, 0, info->rank, &recv->proxyConn));
  NCCLCHECK(ncclProxyCall(&recv->proxyConn, ncclProxyMsgSetup, &recvSize, sizeof(int), &info->p2pBuff, sizeof(struct ncclP2pBuff)));

  NCCLCHECK(p2pMap(myInfo, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&resources->devMem, &resources->recvMemIpc));
  return ncclSuccess;
}

/* Connect/Send to this peer */
/* 中文内部逻辑 F1159：p2pSendConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct p2pSendResources* resources = (struct p2pSendResources*)send->transportResources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclRecvMem* remDevMem;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(p2pMap(comm->peerInfo+rank, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&remDevMem, &resources->recvMemIpc));；失败时可能提前return，成功才继续；同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。
 * 5. 准备局部状态：char* buff = (char*)(remDevMem+1);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 条件判断：(info->read && p == NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     8. 真分支：；仅条件为真执行以下子项。
 *       9. 调用 / 计算 / 写入：send->conn.buffs[p] = (char*)(resources->devMem+1);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       11. 调用 / 计算 / 写入：send->conn.buffs[p] = buff;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       12. 调用 / 计算 / 写入：buff += send->comm->buffSizes[p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 14. 调用 / 计算 / 写入：send->conn.tail = &remDevMem->tail;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 15. 调用 / 计算 / 写入：send->conn.head = &resources->devMem->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 16. 调用 / 计算 / 写入：send->conn.ptrExchange = &resources->devMem->ptrExchange;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：send->conn.redOpArgExchange = resources->devMem->redOpArgExchange;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1159
 */
/* 中文逐项-函数 F1159：p2pSendConnect
 * 用途与位置：导入对端 GPU 分配并填充协议缓冲、head/tail 和直接访问指针交换区。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；p2pMap。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS；info->read && p == NCCL_PROTO_SIMPLE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t p2pSendConnect(struct ncclComm* comm, struct ncclConnect* connectInfo, int nranks, int rank, struct ncclConnector* send) {
/* 中文逐项-变量 V4607：resources
 * resources（struct p2pSendResources；函数 p2pSendConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct p2pSendResources*)send->transportResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct p2pSendResources* resources = (struct p2pSendResources*)send->transportResources;
/* 中文逐项-变量 V4608：remDevMem
 * remDevMem（struct ncclRecvMem；函数 p2pSendConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(p2pMap(comm->peerInfo+rank, comm->peerInfo+info->rank, &info->p2pBuff,
 * (void**)&remDevMem, &resources->recvMemIpc));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclRecvMem* remDevMem;
/* 中文逐项-变量 V4609：info
 * info（struct p2pConnectInfo；函数 p2pSendConnect 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct p2pConnectInfo*)connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;

  NCCLCHECK(p2pMap(comm->peerInfo+rank, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&remDevMem, &resources->recvMemIpc));

/* 中文逐项-变量 V4610：buff
 * buff（char；函数 p2pSendConnect 内）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 * 初值/来源 buff = (char*)(remDevMem+1)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  char* buff = (char*)(remDevMem+1);
  for (/* 中文逐项-变量 V4611：p
 * p（int；函数 p2pSendConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (info->read && p == NCCL_PROTO_SIMPLE) {
      /* For P2P Read the SIMPLE buffer is local (ncclSendMem) */
      send->conn.buffs[p] = (char*)(resources->devMem+1);
    } else {
      send->conn.buffs[p] = buff;
      buff += send->comm->buffSizes[p];
    }
  }
  send->conn.tail = &remDevMem->tail;
  send->conn.head = &resources->devMem->head;
  send->conn.ptrExchange = &resources->devMem->ptrExchange;
  send->conn.redOpArgExchange = resources->devMem->redOpArgExchange;
  return ncclSuccess;
}

/* Connect/Recv from this peer */
/* 中文内部逻辑 F1160：p2pRecvConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct p2pRecvResources* resources = (struct p2pRecvResources*)recv->transportResources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclSendMem* remDevMem;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(p2pMap(comm->peerInfo+rank, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&remDevMem, &resources->sendMemIpc));；失败时可能提前return，成功才继续；同进程直接使用/启用 peer 访问，跨进程打开 CUDA IPC 句柄得到本进程可用映射。
 * 5. 准备局部状态：char* buff = (char*)(resources->devMem+1);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 条件判断：(info->read && p == NCCL_PROTO_SIMPLE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     8. 真分支：；仅条件为真执行以下子项。
 *       9. 调用 / 计算 / 写入：recv->conn.buffs[p] = (char*)(remDevMem+1);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       11. 调用 / 计算 / 写入：recv->conn.buffs[p] = buff;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       12. 调用 / 计算 / 写入：buff += recv->comm->buffSizes[p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 14. 调用 / 计算 / 写入：recv->conn.tail = &resources->devMem->tail;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 15. 调用 / 计算 / 写入：recv->conn.head = &remDevMem->head;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 16. 调用 / 计算 / 写入：recv->conn.ptrExchange = &remDevMem->ptrExchange;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：recv->conn.redOpArgExchange = remDevMem->redOpArgExchange;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1160
 */
/* 中文逐项-函数 F1160：p2pRecvConnect
 * 用途与位置：导入对端 GPU 分配并填充协议缓冲、head/tail 和直接访问指针交换区。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 connectInfo（struct ncclConnect* connectInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；p2pMap。
 * 需要对照的边界/推进条件：p<NCCL_NUM_PROTOCOLS；info->read && p == NCCL_PROTO_SIMPLE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t p2pRecvConnect(struct ncclComm* comm, struct ncclConnect* connectInfo, int nranks, int rank, struct ncclConnector* recv) {
/* 中文逐项-变量 V4612：resources
 * resources（struct p2pRecvResources；函数 p2pRecvConnect 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct p2pRecvResources*)recv->transportResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct p2pRecvResources* resources = (struct p2pRecvResources*)recv->transportResources;
/* 中文逐项-变量 V4613：remDevMem
 * remDevMem（struct ncclSendMem；函数 p2pRecvConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(p2pMap(comm->peerInfo+rank, comm->peerInfo+info->rank, &info->p2pBuff,
 * (void**)&remDevMem, &resources->sendMemIpc));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclSendMem* remDevMem;
/* 中文逐项-变量 V4614：info
 * info（struct p2pConnectInfo；函数 p2pRecvConnect 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = (struct p2pConnectInfo*)connectInfo；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct p2pConnectInfo* info = (struct p2pConnectInfo*)connectInfo;

  NCCLCHECK(p2pMap(comm->peerInfo+rank, comm->peerInfo+info->rank, &info->p2pBuff, (void**)&remDevMem, &resources->sendMemIpc));

/* 中文逐项-变量 V4615：buff
 * buff（char；函数 p2pRecvConnect 内）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 * 初值/来源 buff = (char*)(resources->devMem+1)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  char* buff = (char*)(resources->devMem+1);
  for (/* 中文逐项-变量 V4616：p
 * p（int；函数 p2pRecvConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    if (info->read && p == NCCL_PROTO_SIMPLE) {
      /* For P2P Read the SIMPLE buffer is remote (ncclSendMem) */
      recv->conn.buffs[p] = (char*)(remDevMem+1);
    } else {
      recv->conn.buffs[p] = buff;
      buff += recv->comm->buffSizes[p];
    }
  }
  recv->conn.tail = &resources->devMem->tail;
  recv->conn.head = &remDevMem->head;
  recv->conn.ptrExchange = &remDevMem->ptrExchange;
  recv->conn.redOpArgExchange = remDevMem->redOpArgExchange;
  return ncclSuccess;
}

/* 中文内部逻辑 F1161：p2pSendFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct p2pSendResources* resources = (struct p2pSendResources*)send->transportResources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(resources->sendMemIpc)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：CUDACHECK(cudaIpcCloseMemHandle(resources->sendMemIpc));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 条件判断：(resources->recvMemIpc)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：CUDACHECK(cudaIpcCloseMemHandle(resources->recvMemIpc));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 8. 调用 / 计算 / 写入：free(resources);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1161
 */
/* 中文逐项-函数 F1161：p2pSendFree
 * 用途与位置：关闭导入的 IPC 映射并归还拥有的 GPU 分配或代理资源。
 * 参数 send（struct ncclConnector* send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；cudaIpcCloseMemHandle；free。
 * 需要对照的边界/推进条件：resources->sendMemIpc；resources->recvMemIpc。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t p2pSendFree(struct ncclConnector* send) {
/* 中文逐项-变量 V4617：resources
 * resources（struct p2pSendResources；函数 p2pSendFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct p2pSendResources*)send->transportResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct p2pSendResources* resources = (struct p2pSendResources*)send->transportResources;
  if (resources->sendMemIpc) CUDACHECK(cudaIpcCloseMemHandle(resources->sendMemIpc));
  if (resources->recvMemIpc) CUDACHECK(cudaIpcCloseMemHandle(resources->recvMemIpc));
  free(resources);
  return ncclSuccess;
}

/* 中文内部逻辑 F1162：p2pRecvFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct p2pRecvResources* resources = (struct p2pRecvResources*)recv->transportResources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(resources->sendMemIpc)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：CUDACHECK(cudaIpcCloseMemHandle(resources->sendMemIpc));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 条件判断：(resources->recvMemIpc)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：CUDACHECK(cudaIpcCloseMemHandle(resources->recvMemIpc));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 8. 调用 / 计算 / 写入：free(resources);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1162
 */
/* 中文逐项-函数 F1162：p2pRecvFree
 * 用途与位置：关闭导入的 IPC 映射并归还拥有的 GPU 分配或代理资源。
 * 参数 recv（struct ncclConnector* recv）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；cudaIpcCloseMemHandle；free。
 * 需要对照的边界/推进条件：resources->sendMemIpc；resources->recvMemIpc。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t p2pRecvFree(struct ncclConnector* recv) {
/* 中文逐项-变量 V4618：resources
 * resources（struct p2pRecvResources；函数 p2pRecvFree 内）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 * 初值/来源 resources = (struct p2pRecvResources*)recv->transportResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct p2pRecvResources* resources = (struct p2pRecvResources*)recv->transportResources;
  if (resources->sendMemIpc) CUDACHECK(cudaIpcCloseMemHandle(resources->sendMemIpc));
  if (resources->recvMemIpc) CUDACHECK(cudaIpcCloseMemHandle(resources->recvMemIpc));
  free(resources);
  return ncclSuccess;
}

/* 中文内部逻辑 F1163：p2pProxySetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(reqSize != sizeof(int))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：int size = *((int*)reqBuff);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(respSize != sizeof(struct ncclP2pBuff))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 准备局部状态：struct ncclP2pBuff* p2pBuff = (struct ncclP2pBuff*)respBuff;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaCalloc((char**)&p2pBuff->directPtr, size));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 10. 调用 / 计算 / 写入：connection->transportResources = p2pBuff->directPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 准备局部状态：cudaError_t res = cudaIpcGetMemHandle(&p2pBuff->devIpc, p2pBuff->directPtr);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 条件判断：(res != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 调用 / 计算 / 写入：WARN("cudaIpcGetMemHandle failed : %s", cudaGetErrorString(res));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     15. 调用 / 计算 / 写入：cudaFree(p2pBuff->directPtr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     16. 调用 / 计算 / 写入：free(p2pBuff);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     17. 调用 / 计算 / 写入：CUDACHECK(res);；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 18. 调用 / 计算 / 写入：*done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1163
 */
/* 中文逐项-函数 F1163：p2pProxySetup
 * 用途与位置：由资源所属代理分配/导出 GPU 内存，供主线程建立 P2P/中间 GPU 路径。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaFree；cudaGetErrorString；cudaIpcGetMemHandle；free；ncclCudaCalloc。
 * 需要对照的边界/推进条件：reqSize != sizeof(int；respSize != sizeof(struct ncclP2pBuff；res != cudaSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t p2pProxySetup(struct ncclProxyConnection* connection, struct ncclComm* comm, void* reqBuff, int reqSize, void* respBuff, int respSize, int* done) {
  if (reqSize != sizeof(int)) return ncclInternalError;
/* 中文逐项-变量 V4619：size
 * size（int；函数 p2pProxySetup 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = *((int*)reqBuff)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int size = *((int*)reqBuff);
  if (respSize != sizeof(struct ncclP2pBuff)) return ncclInternalError;
/* 中文逐项-变量 V4620：p2pBuff
 * p2pBuff（struct ncclP2pBuff；函数 p2pProxySetup 内）：P2P GPU分配的直接指针与IPC句柄摘要，connect时转换成本进程可用映射。
 * 初值/来源 p2pBuff = (struct ncclP2pBuff*)respBuff；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclP2pBuff* p2pBuff = (struct ncclP2pBuff*)respBuff;
  NCCLCHECK(ncclCudaCalloc((char**)&p2pBuff->directPtr, size));
  connection->transportResources = p2pBuff->directPtr;
/* 中文逐项-变量 V4621：res
 * res（cudaError_t；函数 p2pProxySetup 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = cudaIpcGetMemHandle(&p2pBuff->devIpc, p2pBuff->directPtr)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  cudaError_t res = cudaIpcGetMemHandle(&p2pBuff->devIpc, p2pBuff->directPtr);
  if (res != cudaSuccess) {
    WARN("cudaIpcGetMemHandle failed : %s", cudaGetErrorString(res));
    cudaFree(p2pBuff->directPtr);
    free(p2pBuff);
    CUDACHECK(res);
  }
  *done = 1;
  return ncclSuccess;
}

/* 中文内部逻辑 F1164：p2pProxyFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：cudaFree(connection->transportResources);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__p2p_cc.md#f1164
 */
/* 中文逐项-函数 F1164：p2pProxyFree
 * 用途与位置：由拥有分配的代理回收 P2P GPU 资源，与映射导入方的解除映射分工。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：cudaFree。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t p2pProxyFree(struct ncclProxyConnection* connection, struct ncclComm* comm) {
  // Do not check return code as CUDA may have already shut down
  cudaFree(connection->transportResources);
  return ncclSuccess;
}

/* 中文逐项-变量 V4622：p2pTransport
 * p2pTransport（struct ncclTransport；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 p2pTransport = { "P2P", p2pCanConnect, { p2pSendSetup, p2pSendConnect, p2pSendFree, NULL, p2pProxySetup,
 * NULL, p2pProxyFree, NULL }, { p2pRecvSetup, p2pRecvConnect, p2pRecvFree, NULL, p2pProxySetup, NULL, p2pProxyFree,
 * NULL } }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
struct ncclTransport p2pTransport = {
  "P2P",
  p2pCanConnect,
  { p2pSendSetup, p2pSendConnect, p2pSendFree, NULL, p2pProxySetup, NULL, p2pProxyFree, NULL },
  { p2pRecvSetup, p2pRecvConnect, p2pRecvFree, NULL, p2pProxySetup, NULL, p2pProxyFree, NULL }
};
