/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"

NCCL_API(ncclResult_t, ncclAllGather, const void* sendbuff, void* recvbuff, size_t sendcount,
    ncclDataType_t datatype, ncclComm_t comm, cudaStream_t stream);
/* 中文导读：
 * AllGather 的 count 是每个 rank 的输入贡献元素数，输出需能容纳 nRanks 份贡献。
 * API 包装后进入统一调度，rank 排序与设备 Ring 顺序的转换在设备算法中处理。
 */
/* 中文内部逻辑 F0050：ncclAllGather
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 准备局部状态：struct ncclInfo info = { ncclFuncAllGather, "AllGather", sendbuff, recvbuff, sendcount, datatype, ncclSum, 0, comm, stream, ALLGATHER_CHUNKSTEPS, ALLGATHER_SLICESTEPS };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 函数返回：return ncclEnqueueCheck(&info);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__all_gather_cc.md#f0050
 */
/* 中文逐项-函数 F0050：ncclAllGather
 * 用途与位置：收集每个 rank 的贡献到所有 rank 的输出，输出按用户 rank 分段；复制型任务不做数值求和。
 * 参数 sendbuff（const void* sendbuff）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 recvbuff（void* recvbuff）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 sendcount（size_t sendcount）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclEnqueueCheck。
 * 源码返回表达式：return ncclEnqueueCheck(&info);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclAllGather(const void* sendbuff, void* recvbuff, size_t sendcount,
    ncclDataType_t datatype, ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：AllGather 的调用与尺寸契约
 * sendcount 是本 rank 贡献元素数，输出需容纳 nRanks*sendcount，按用户 rank 顺序排列。
 * 示例：rank0=[1,2]，rank1=[3,4]，两边输出都是 [1,2,3,4]，不进行求和。
 * EnqueueCheck 的 ArgsCheck 会把复制型任务归一为 Int8，再走共同的选型/排队/发射流程。
 * 设备 all_gather.h 的 Ring 每步收/存/转发别人的贡献，地址公式是 rank*size+chunkOffset。
 * 原地形式要求 sendbuff 指向 recvbuff 中属于自己的那一段，不是任意两个指针相同即可。
 */
/* 中文逐项-变量 V0089：info
 * info（struct ncclInfo；函数 ncclAllGather 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncAllGather, "AllGather", sendbuff, recvbuff, sendcount, datatype, ncclSum, 0, comm, stream, /
 * * Args * / ALLGATHER_CHUNKSTEPS, ALLGATHER_SLICESTEPS }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncAllGather, "AllGather",
    sendbuff, recvbuff, sendcount, datatype, ncclSum, 0, comm, stream, /* Args */
    ALLGATHER_CHUNKSTEPS, ALLGATHER_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
