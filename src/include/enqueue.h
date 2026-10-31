/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_ENQUEUE_H_
/* 中文逐项-常量 X0250：NCCL_ENQUEUE_H_
 * 编译期定义：NCCL_ENQUEUE_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_ENQUEUE_H_

#include "comm.h"
#include "group.h"
#include "collectives.h"

/* 中文逐项-常量 X0251：NCCL_MIN_CHANNEL_SIZE
 * 编译期定义：NCCL_MIN_CHANNEL_SIZE = (NCCL_LL_THREAD_THRESHOLD*64)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MIN_CHANNEL_SIZE (NCCL_LL_THREAD_THRESHOLD*64)
/* 中文逐项-常量 X0252：NCCL_AGG_CHANNEL_SIZE
 * 编译期定义：NCCL_AGG_CHANNEL_SIZE = (1LL << 21)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_AGG_CHANNEL_SIZE (1LL << 21) /* 2 MiB, ideal per-channel size to fully utilize bandwidth */

/* 中文逐项-接口 X0253：ncclKernMaxLocalSize
 * 接口职责：遍历入口 kernel 属性以求最大局部内存使用，供初始化设备栈/资源配置参考。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：size_t ncclKernMaxLocalSize();
 */
size_t ncclKernMaxLocalSize();
/* 中文逐项-接口 X0254：ncclKernSetSharedMemoryCarveout
 * 接口职责：给受支持的 CUDA kernel 入口设置共享内存划分偏好，影响 SM 资源分配而不改算法语义。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclKernSetSharedMemoryCarveout(int carveOut);
 */
ncclResult_t ncclKernSetSharedMemoryCarveout(int carveOut);
/* 中文逐项-接口 X0255：ncclEnqueueCheck
 * 接口职责：通信 API 的统一检查与分流入口，组内保存任务，普通路径准备队列并发布/发射执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclEnqueueCheck(struct ncclInfo* info);
 */
ncclResult_t ncclEnqueueCheck(struct ncclInfo* info);
/* 中文逐项-接口 X0256：ncclCpuBarrierIn
 * 接口职责：进入同进程多 GPU 发射的相位屏障，用原子计数选出最后到达者统一发射。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclCpuBarrierIn(struct ncclComm* comm, int* isLast);
 */
ncclResult_t ncclCpuBarrierIn(struct ncclComm* comm, int* isLast);
/* 中文逐项-接口 X0257：ncclCpuBarrierLast
 * 接口职责：最后到达者发布本相位已完成信号，使其他发射线程可离开屏障。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclCpuBarrierLast(struct ncclComm* comm);
 */
ncclResult_t ncclCpuBarrierLast(struct ncclComm* comm);
/* 中文逐项-接口 X0258：ncclCpuBarrierOut
 * 接口职责：等待本地发射相位完成并翻转相位，保证下一轮发射不会误用上轮计数。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclCpuBarrierOut(struct ncclComm* comm);
 */
ncclResult_t ncclCpuBarrierOut(struct ncclComm* comm);
/* 中文逐项-接口 X0259：ncclLaunchBarrier
 * 接口职责：建立用户 stream 与内部 stream 依赖，并在 GROUP 发射模式协调同进程 GPU 的启动。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclLaunchBarrier(struct ncclComm* comm);
 */
ncclResult_t ncclLaunchBarrier(struct ncclComm* comm);
/* 中文逐项-接口 X0260：ncclLaunchKernel
 * 接口职责：PARALLEL 模式真正调用 CUDA 发射，GROUP 模式等待已由最后线程执行的统一发射完成。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclLaunchKernel(ncclComm_t comm);
 */
ncclResult_t ncclLaunchKernel(ncclComm_t comm);
/* 中文逐项-接口 X0261：ncclRecordEvents
 * 接口职责：在执行 stream 记录完成事件并建立必要的用户 stream 依赖；record 本身不代表事件已经完成。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclRecordEvents(struct ncclComm* comm);
 */
ncclResult_t ncclRecordEvents(struct ncclComm* comm);
/* 中文逐项-接口 X0262：ncclLaunchReset
 * 接口职责：清理本批发射参数和入队计数，为后续提交复用；图路径按持久 QueueInfo 生命周期处理。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclLaunchReset(ncclComm_t comm);
 */
ncclResult_t ncclLaunchReset(ncclComm_t comm);
/* 中文逐项-接口 X0263：ncclSetupP2pKernel
 * 接口职责：为 P2P 块生成设备/Proxy 计划并更新发射所需的线程和 channel 范围。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSetupP2pKernel(struct ncclInfo* info);
 */
ncclResult_t ncclSetupP2pKernel(struct ncclInfo* info);
/* 中文逐项-接口 X0264：ncclSetupAsyncKernels
 * 接口职责：把 Group 保存的一组 collective 转成发射任务，对同构批次可统一选型并聚合工作元素。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSetupAsyncKernels(struct ncclComm* comm);
 */
ncclResult_t ncclSetupAsyncKernels(struct ncclComm* comm);
template<int USING_CUDA_GRAPH>
/* 中文逐项-接口 X0265：ncclEnqueueHostSetup
 * 接口职责：遍历提交批次并执行真正的 FIFO 装填、launch 整理和 Proxy 发布；图 replay 也复用此入口。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void CUDART_CB ncclEnqueueHostSetup(void* arg);
 */
void CUDART_CB ncclEnqueueHostSetup(void* arg);
/* 中文逐项-接口 X0266：ncclGetCudaGraph
 * 接口职责：检查当前 stream 是否捕获、图标识和驱动能力，维护图模式队列及生命周期状态。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclGetCudaGraph(ncclComm_t comm, cudaGraph_t* graph);
 */
ncclResult_t ncclGetCudaGraph(ncclComm_t comm, cudaGraph_t* graph);
/* 中文逐项-接口 X0267：ncclCudaGraphHostSetup
 * 接口职责：给 CUDA Graph 添加/关联 HostSetup 节点及持有对象，确保 replay 时重新发布 GPU/Proxy 工作。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclCudaGraphHostSetup(ncclComm_t comm, cudaGraph_t graph);
 */
ncclResult_t ncclCudaGraphHostSetup(ncclComm_t comm, cudaGraph_t graph);

/* 中文逐项-类型 T0201：ncclBuffRegInfo
 * 类型职责：CUDA Graph用户缓冲注册优化的IPC导出/导入地址信息，区别于普通verbs MR。
 * 声明内可见成员（含嵌套成员）：sendbuffsBase、recvbuffsBase、sendbuffs、recvbuffs、nBuffs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclBuffRegInfo {
/* 中文逐项-变量 V2225：sendbuffsBase
 * sendbuffsBase（void；成员 ncclBuffRegInfo）：图注册路径导入的分配基址及实际用户偏移地址，释放 IPC 要用对应基址。
 */
  void* sendbuffsBase[NCCL_MAX_LOCAL_RANKS];
/* 中文逐项-变量 V2226：recvbuffsBase
 * recvbuffsBase（void；成员 ncclBuffRegInfo）：图注册路径导入的分配基址及实际用户偏移地址，释放 IPC 要用对应基址。
 */
  void* recvbuffsBase[NCCL_MAX_LOCAL_RANKS];
/* 中文逐项-变量 V2227：sendbuffs
 * sendbuffs（void；成员 ncclBuffRegInfo）：图注册路径导入的分配基址及实际用户偏移地址，释放 IPC 要用对应基址。
 */
  void* sendbuffs[NCCL_MAX_LOCAL_RANKS];
/* 中文逐项-变量 V2228：recvbuffs
 * recvbuffs（void；成员 ncclBuffRegInfo）：图注册路径导入的分配基址及实际用户偏移地址，释放 IPC 要用对应基址。
 */
  void* recvbuffs[NCCL_MAX_LOCAL_RANKS];
/* 中文逐项-变量 V2229：nBuffs
 * nBuffs（int；成员 ncclBuffRegInfo）：当前注册/导入缓冲项数量，驱动图对象销毁时的 IPC 清理。
 */
  int nBuffs;
};

// Enqueue information (for kernel and proxy) for each operation
/* 中文精读：一项操作为什么保存两份执行描述
 * work 给 GPU，proxyOp 给 CPU 网络线程，buffRegInfo 保存特殊注册路径的地址信息。
 * 主机只保存指针和尺寸等元数据，不在这里把整个用户张量复制进队列。
 * 同一 QueueElem 展开到多个 channel 时，复制 work 模板并填写不同 bid/channelId。
 * 这是一项 API 到多条 channel 任务的展开，不是一个 QueueElem 对应一个 RDMA 连接。
 */
/* 中文逐项-类型 T0202：ncclQueueElem
 * 类型职责：一项操作的GPU/Proxy双计划及一个提交批次/图持有的计划列表，生命周期覆盖HostSetup/replay。
 * 声明内可见成员（含嵌套成员）：work、proxyOp、buffRegInfo。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclQueueElem {
/* 中文逐项-变量 V2230：work
 * work（struct ncclWork；成员 ncclQueueElem）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 */
  struct ncclWork work;
/* 中文逐项-变量 V2231：proxyOp
 * proxyOp（struct ncclProxyOp；成员 ncclQueueElem）：交给 CPU Proxy 的紧凑计划，与 GPU work 的协议和步数必须一致。
 */
  struct ncclProxyOp proxyOp;
/* 中文逐项-变量 V2232：buffRegInfo
 * buffRegInfo（struct ncclBuffRegInfo；成员 ncclQueueElem）：图注册缓冲的交换/映射信息，和普通网络 MR 句柄区分。
 */
  struct ncclBuffRegInfo buffRegInfo;
};

typedef ncclRecyclableList<struct ncclQueueElem> ncclQueueElemList;

// Structure passed to CUDA graph
/* 中文精读：提交批次与 CUDA Graph 生命周期
 * elemList 保存本批 API 生成的模板列表，maxChannels 保存本批最高 channel 范围。
 * 普通路径 HostSetup 后可复用队列；Graph 路径必须把这个对象保留到图不再使用它。
 * ret 记录 HostSetup 的错误，nRegBuffs 用来管理图注册缓冲的释放需求。
 * 所以图 replay 的 HostSetup 不能假定捕获时 comm 的临时入队状态仍未改变。
 */
/* 中文逐项-类型 T0203：ncclQueueInfo
 * 类型职责：一项操作的GPU/Proxy双计划及一个提交批次/图持有的计划列表，生命周期覆盖HostSetup/replay。
 * 声明内可见成员（含嵌套成员）：comm、maxChannels、ret、nRegBuffs、elemList。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclQueueInfo {
/* 中文逐项-变量 V2233：comm
 * comm（ncclComm_t；成员 ncclQueueInfo）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  ncclComm_t comm;
/* 中文逐项-变量 V2234：maxChannels
 * maxChannels（int；成员 ncclQueueInfo）：图/后端允许的逻辑通道上限和下限，受容量与配置共同约束。
 */
  int maxChannels;    // Dynamic version of gridDim
/* 中文逐项-变量 V2235：ret
 * ret（ncclResult_t；成员 ncclQueueInfo）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 */
  ncclResult_t ret;   // Return value of host setup call
/* 中文逐项-变量 V2236：nRegBuffs
 * nRegBuffs（int；成员 ncclQueueInfo）：当前注册/导入缓冲项数量，驱动图对象销毁时的 IPC 清理。
 */
  int nRegBuffs;
/* 中文逐项-变量 V2237：elemList
 * elemList（ncclQueueElemList；成员 ncclQueueInfo）：本提交/图持有的排队元信息和可复用任务列表，HostSetup 根据它们装填真实 FIFO。
 */
  ncclQueueElemList* elemList;
};

/* 中文内部逻辑 F0632：ncclCreateQueueInfo
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(eqInfo, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 2. 调用 / 计算 / 写入：(*eqInfo)->comm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：(*eqInfo)->elemList = new ncclQueueElemList();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：(*eqInfo)->comm->nQueueInfoCreated++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__enqueue_h.md#f0632
 */
/* 中文逐项-函数 F0632：ncclCreateQueueInfo
 * 用途与位置：分配一个提交/图持有的 QueueInfo 和可复用元素列表，记录 communicator 队列生命周期计数。
 * 参数 eqInfo（struct ncclQueueInfo** eqInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclLaunchReset；src/init.cc::commAlloc。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclCreateQueueInfo(struct ncclQueueInfo** eqInfo, ncclComm_t comm) {
  NCCLCHECK(ncclCalloc(eqInfo, 1));
  (*eqInfo)->comm = comm;
  (*eqInfo)->elemList = new ncclQueueElemList();
  (*eqInfo)->comm->nQueueInfoCreated++;
  return ncclSuccess;
}

// Reset element queue
/* 中文内部逻辑 F0633：ncclResetQueueInfo
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(eqInfo == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 调用 / 计算 / 写入：eqInfo->maxChannels = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：eqInfo->ret = ncclSuccess;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 调用 / 计算 / 写入：eqInfo->nRegBuffs = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：eqInfo->elemList->recycle();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__enqueue_h.md#f0633
 */
/* 中文逐项-函数 F0633：ncclResetQueueInfo
 * 用途与位置：清理一批已经提交的队列统计并 recycle 元素节点，保留容量供后续批次复用。
 * 参数 eqInfo（struct ncclQueueInfo* eqInfo）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclLaunchReset。
 * 本体直接出现的调用/包装：eqInfo->elemList->recycle。
 * 需要对照的边界/推进条件：eqInfo == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclResetQueueInfo(struct ncclQueueInfo* eqInfo) {
  if (eqInfo == NULL) return ncclInternalError;
  eqInfo->maxChannels = 0;
  eqInfo->ret = ncclSuccess;
  eqInfo->nRegBuffs = 0;
  eqInfo->elemList->recycle();
  return ncclSuccess;
}

// Destroy enqueue info space
// used by both CUDA graph and non CUDA graph
/* 中文内部逻辑 F0634：ncclDestroyQueueInfo
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(ptr == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：struct ncclQueueInfo* eqInfo = (struct ncclQueueInfo*)ptr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：struct ncclComm* comm = eqInfo->comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：struct ncclQueueElem* eqElem = eqInfo->elemList->begin();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 编译期分支/指令：#if 0；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   8. 循环：初始 ；继续条件 (eqElem != NULL)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     9. 循环：初始 int i=0;；继续条件 i<eqElem->buffRegInfo.nBuffs；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       10. 条件判断：(i == eqInfo->comm->localRank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         11. 真分支：；仅条件为真执行以下子项。
 *           12. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *       13. 调用 / 计算 / 写入：CUDACHECKIGNORE(cudaIpcCloseMemHandle(eqElem->buffRegInfo.sendbuffsBase[i]));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *       14. 调用 / 计算 / 写入：CUDACHECKIGNORE(cudaIpcCloseMemHandle(eqElem->buffRegInfo.recvbuffsBase[i]));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     15. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     16. 调用 / 计算 / 写入：eqElem = eqInfo->elemList->getNext();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   17. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   18. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     19. 准备局部状态：struct ncclGraphHelperResources* res = comm->graphHelperResources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     20. 准备局部状态：int ipcTailOld = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     21. 条件判断：(res == NULL || (!comm->graphHelperThread) || eqInfo->nRegBuffs == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       22. 真分支：；仅条件为真执行以下子项。
 *         23. 控制转移：goto skip;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     24. 调用 / 计算 / 写入：pthread_mutex_lock(&res->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     25. 调用 / 计算 / 写入：ipcTailOld = res->ipcTail;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     26. 循环：初始 ；继续条件 (eqElem != NULL)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       27. 循环：初始 int i=0;；继续条件 i<eqElem->buffRegInfo.nBuffs；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         28. 条件判断：(eqElem->buffRegInfo.sendbuffsBase[i] != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           29. 真分支：；仅条件为真执行以下子项。
 *             30. 调用 / 计算 / 写入：res->ipcBases[res->ipcTail] = eqElem->buffRegInfo.sendbuffsBase[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             31. 调用 / 计算 / 写入：res->ipcTail = (res->ipcTail+1)%NCCL_IPC_POOL_SIZE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         32. 条件判断：(eqElem->buffRegInfo.recvbuffsBase[i] != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           33. 真分支：；仅条件为真执行以下子项。
 *             34. 调用 / 计算 / 写入：res->ipcBases[res->ipcTail] = eqElem->buffRegInfo.recvbuffsBase[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             35. 调用 / 计算 / 写入：res->ipcTail = (res->ipcTail+1)%NCCL_IPC_POOL_SIZE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       36. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       37. 调用 / 计算 / 写入：eqElem = eqInfo->elemList->getNext();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     38. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     39. 条件判断：(res->ipcTail != ipcTailOld)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       40. 真分支：；仅条件为真执行以下子项。
 *         41. 调用 / 计算 / 写入：res->threadState = ThreadStart;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         42. 调用 / 计算 / 写入：TRACE(NCCL_COLL, "CUDA Graph destroy function signaling helper thread with %d IPC handles", res->ipcTail-ipcTailOld);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         43. 调用 / 计算 / 写入：pthread_cond_signal(&res->threadCond);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     44. 调用 / 计算 / 写入：pthread_mutex_unlock(&res->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 45. 跳转标签：skip；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   46. 调用 / 计算 / 写入：delete eqInfo->elemList;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 * 47. 调用 / 计算 / 写入：free(eqInfo);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 48. 调用 / 计算 / 写入：comm->nQueueInfoDestroyed++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 49. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__include__enqueue_h.md#f0634
 */
/* 中文逐项-函数 F0634：ncclDestroyQueueInfo
 * 用途与位置：释放图/普通提交持有的队列，把不能在图析构回调直接关闭的 CUDA IPC 句柄转交辅助线程。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::commDestroy。
 * 本体直接出现的调用/包装：CUDACHECKIGNORE；cudaIpcCloseMemHandle；eqInfo->elemList->begin；eqInfo->elemList->getNext；free；pthread_cond_signal；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：ptr == NULL；eqElem != NULL；i<eqElem->buffRegInfo.nBuffs；i == eqInfo->comm->localRank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
static void ncclDestroyQueueInfo(void* ptr) {
  if (ptr == NULL) return;
/* 中文逐项-变量 V2238：eqInfo
 * eqInfo（struct ncclQueueInfo；函数 ncclDestroyQueueInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 eqInfo = (struct ncclQueueInfo*)ptr；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclQueueInfo* eqInfo = (struct ncclQueueInfo*)ptr;
/* 中文逐项-变量 V2239：comm
 * comm（struct ncclComm；函数 ncclDestroyQueueInfo 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = eqInfo->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclComm* comm = eqInfo->comm;
  // Close IPC mem handles for registered buffers
/* 中文逐项-变量 V2240：eqElem
 * eqElem（struct ncclQueueElem；函数 ncclDestroyQueueInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 eqElem = eqInfo->elemList->begin()；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclQueueElem* eqElem = eqInfo->elemList->begin();
#if 0
  // Ideally, the deregistration should happen here
  // but currently the destroy function of CUDA objects does not allow CUDA API calls
  while (eqElem != NULL) {
    for (/* 中文逐项-变量 V2241：i
 * i（int；函数 ncclDestroyQueueInfo 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<eqElem->buffRegInfo.nBuffs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<eqElem->buffRegInfo.nBuffs; i++) {
      if (i == eqInfo->comm->localRank) continue;
      CUDACHECKIGNORE(cudaIpcCloseMemHandle(eqElem->buffRegInfo.sendbuffsBase[i]));
      CUDACHECKIGNORE(cudaIpcCloseMemHandle(eqElem->buffRegInfo.recvbuffsBase[i]));
    }
    eqElem = eqInfo->elemList->getNext();
  }
#else
  // Instead, we push these pointers to a pool owned by ncclComm
  // and asks a helper thread to close mem handles
/* 中文逐项-变量 V2242：res
 * res（struct ncclGraphHelperResources；函数 ncclDestroyQueueInfo 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = comm->graphHelperResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclGraphHelperResources* res = comm->graphHelperResources;
/* 中文逐项-变量 V2243：ipcTailOld
 * ipcTailOld（int；函数 ncclDestroyQueueInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ipcTailOld = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int ipcTailOld = 0;
  if (res == NULL || (!comm->graphHelperThread) || eqInfo->nRegBuffs == 0) goto skip;

  pthread_mutex_lock(&res->threadLock);
  ipcTailOld = res->ipcTail;
  while (eqElem != NULL) {
    for (/* 中文逐项-变量 V2244：i
 * i（int；函数 ncclDestroyQueueInfo 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<eqElem->buffRegInfo.nBuffs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<eqElem->buffRegInfo.nBuffs; i++) {
      if (eqElem->buffRegInfo.sendbuffsBase[i] != NULL) {
        res->ipcBases[res->ipcTail] = eqElem->buffRegInfo.sendbuffsBase[i];
        res->ipcTail = (res->ipcTail+1)%NCCL_IPC_POOL_SIZE;
      }
      if (eqElem->buffRegInfo.recvbuffsBase[i] != NULL) {
        res->ipcBases[res->ipcTail] = eqElem->buffRegInfo.recvbuffsBase[i];
        res->ipcTail = (res->ipcTail+1)%NCCL_IPC_POOL_SIZE;
      }
    }
    eqElem = eqInfo->elemList->getNext();
  }
  if (res->ipcTail != ipcTailOld) {
    res->threadState = ThreadStart;
    TRACE(NCCL_COLL, "CUDA Graph destroy function signaling helper thread with %d IPC handles", res->ipcTail-ipcTailOld);
    pthread_cond_signal(&res->threadCond);
  }
  pthread_mutex_unlock(&res->threadLock);
#endif

skip:
  delete eqInfo->elemList;
  free(eqInfo);
  comm->nQueueInfoDestroyed++;
  return;
}
#endif // End include guard
