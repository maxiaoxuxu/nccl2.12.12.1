/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"

NCCL_API(ncclResult_t, ncclReduceScatter, const void* sendbuff, void* recvbuff, size_t recvcount,
    ncclDataType_t datatype, ncclRedOp_t op, ncclComm* comm, cudaStream_t stream);
/* 中文导读：
 * ReduceScatter 的 recvcount 是本 rank 最终输出的元素数，输入覆盖所有 rank 的分块。
 * 统一调度器计算通信字节数时会处理该语义，不能把 recvcount 直接当作整个输入大小。
 */
/* 中文内部逻辑 F0378：ncclReduceScatter
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 准备局部状态：struct ncclInfo info = { ncclFuncReduceScatter, "ReduceScatter", sendbuff, recvbuff, recvcount, datatype, op, 0, comm, stream, REDUCESCATTER_CHUNKSTEPS, REDUCESCATTER_SLICESTEPS };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 函数返回：return ncclEnqueueCheck(&info);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__reduce_scatter_cc.md#f0378
 */
/* 中文逐项-函数 F0378：ncclReduceScatter
 * 用途与位置：先归约各 rank 的完整输入，再只把属于当前用户 rank 的结果段保留在本地输出。
 * 参数 sendbuff（const void* sendbuff）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 recvbuff（void* recvbuff）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 recvcount（size_t recvcount）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 op（ncclRedOp_t op）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 comm（ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclEnqueueCheck。
 * 源码返回表达式：return ncclEnqueueCheck(&info);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclReduceScatter(const void* sendbuff, void* recvbuff, size_t recvcount,
    ncclDataType_t datatype, ncclRedOp_t op, ncclComm* comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：ReduceScatter 的调用与尺寸契约
 * recvcount 是本 rank 的输出元素数，输入需容纳 nRanks*recvcount，所有 rank 输入长度相同。
 * 示例：2 rank、recvcount=2，输入 [1,2,3,4] 和 [10,20,30,40]，
 * rank0 输出 [11,22]，rank1 输出 [33,44]，即完整求和结果按 rank 分段。
 * EnqueueCheck/computeColl 生成 Ring 工作，设备 reduce_scatter.h 做 send/reduceSend/reduceCopy。
 * 最终归约写到自己短输出的 chunkOffset，而输入读取对应用户 rank 的完整段偏移。
 */
/* 中文逐项-变量 V0853：info
 * info（struct ncclInfo；函数 ncclReduceScatter 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncReduceScatter, "ReduceScatter", sendbuff, recvbuff, recvcount, datatype, op, 0, comm,
 * stream, / * Args * / REDUCESCATTER_CHUNKSTEPS, REDUCESCATTER_SLICESTEPS
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncReduceScatter, "ReduceScatter",
    sendbuff, recvbuff, recvcount, datatype, op, 0, comm, stream, /* Args */
    REDUCESCATTER_CHUNKSTEPS, REDUCESCATTER_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
