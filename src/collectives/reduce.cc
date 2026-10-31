/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"

NCCL_API(ncclResult_t, ncclReduce, const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t datatype, ncclRedOp_t op, int root, ncclComm_t comm, cudaStream_t stream);
/* 中文导读：
 * Reduce 汇总各 rank 的输入，但最终结果只要求在 root 的接收缓冲有效。
 * 数据归约由设备原语执行；API 层的 root 语义与 P2P 把同一字段用作 peer 的语义不同。
 */
/* 中文逐项-函数 F0377：ncclReduce
 * 用途与位置：沿选定通信路径把全部贡献归约到 root，其他 rank 不保证持有完整结果。
 * 参数 sendbuff（const void* sendbuff）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 recvbuff（void* recvbuff）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 count（size_t count）：公开API输入的datatype元素数；这里尚未归一化成字节，实际字节长度=count*ncclTypeSize(datatype)。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 op（ncclRedOp_t op）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 root（int root）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclEnqueueCheck。
 * 源码返回表达式：return ncclEnqueueCheck(&info);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclReduce(const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t datatype, ncclRedOp_t op, int root, ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：Reduce 只有 root 需要最终输出
 * 所有 rank 各提供 count 元素；归约结果仅保证放到 root 的 recvbuff。
 * 示例：两个 rank 输入 [1,2] 和 [10,20]，root 得到 [11,22]，其他 rank 无需完整输出。
 * 其主机入队流程与 AllReduce 共用，但 pattern 是 PipelineTo，Proxy 仅准备必要方向的边。
 * 设备 reduce.h 从 root 后继启动 send，沿环归约到 root 后结束，不再做 AllGather。
 * 选择 root 改变流水线的起点/终点，不改变数据类型和归约操作的数学含义。
 */
/* 中文逐项-变量 V0852：info
 * info（struct ncclInfo；函数 ncclReduce 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncReduce, "Reduce", sendbuff, recvbuff, count, datatype, op, root, comm, stream, / * Args * /
 * REDUCE_CHUNKSTEPS, REDUCE_SLICESTEPS }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncReduce, "Reduce",
    sendbuff, recvbuff, count, datatype, op, root, comm, stream, /* Args */
    REDUCE_CHUNKSTEPS, REDUCE_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
