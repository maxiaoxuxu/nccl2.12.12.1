/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"
#include "collectives.h"

NCCL_API(ncclResult_t, ncclBroadcast, const void* sendbuff, void* recvbuff, size_t count, ncclDataType_t datatype, int root,
    ncclComm_t comm, cudaStream_t stream);
/* 中文导读：
 * Broadcast 将 root 的输入传播到各 rank，公开入口使用统一 ncclInfo 描述。
 * 实际沿环流水或树向下执行，取决于该版本可用实现与调度结果；它不进行跨 rank 求和。
 */
/* 中文内部逻辑 F0052：ncclBroadcast
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 准备局部状态：struct ncclInfo info = { ncclFuncBroadcast, "Broadcast", sendbuff, recvbuff, count, datatype, ncclSum, root, comm, stream, BROADCAST_CHUNKSTEPS, BROADCAST_SLICESTEPS };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 函数返回：return ncclEnqueueCheck(&info);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__broadcast_cc.md#f0052
 */
/* 中文逐项-函数 F0052：ncclBroadcast
 * 用途与位置：从 root 将同一输入分片流水传播到所有参与者，非 root 无需提供有效广播输入。
 * 参数 sendbuff（const void* sendbuff）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 recvbuff（void* recvbuff）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 count（size_t count）：公开API输入的datatype元素数；这里尚未归一化成字节，实际字节长度=count*ncclTypeSize(datatype)。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 root（int root）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/broadcast.cc::ncclBcast。
 * 本体直接出现的调用/包装：ncclEnqueueCheck。
 * 源码返回表达式：return ncclEnqueueCheck(&info);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclBroadcast(const void* sendbuff, void* recvbuff, size_t count, ncclDataType_t datatype, int root,
    ncclComm_t comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：Broadcast 的根与普通参与者
 * 所有 rank 都必须参与同一调用序列；只有 root 的 sendbuff 是广播输入。
 * 每个 rank 的 recvbuff 得到相同 count 元素，不增加 nRanks 倍的输出长度。
 * ArgsCheck 归一到 Int8，随后共用 enqueue 提交流程；设备 broadcast.h 按 root 切断环回边。
 * root copySend 或 send，中间 recvCopySend，root 前驱 recv；数据可以按 slice 流水传播。
 * 兼容 API ncclBcast 只把同一 buff 作为输入输出转调这里。
 */
/* 中文逐项-变量 V0091：info
 * info（struct ncclInfo；函数 ncclBroadcast 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncBroadcast, "Broadcast", sendbuff, recvbuff, count, datatype, ncclSum, root, comm, stream, /
 * * Args * / BROADCAST_CHUNKSTEPS, BROADCAST_SLICESTEPS }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncBroadcast, "Broadcast",
    sendbuff, recvbuff, count, datatype, ncclSum, root, comm, stream, /* Args */
    BROADCAST_CHUNKSTEPS, BROADCAST_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
/* Deprecated original "in place" function, similar to MPI */
NCCL_API(ncclResult_t, ncclBcast, void* buff, size_t count, ncclDataType_t datatype, int root,
    ncclComm_t comm, cudaStream_t stream);
/* 中文内部逻辑 F0053：ncclBcast
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclBroadcast(buff, buff, count, datatype, root, comm, stream);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__broadcast_cc.md#f0053
 */
/* 中文逐项-函数 F0053：ncclBcast
 * 用途与位置：把同一缓冲作为输入/输出转调 Broadcast，保留旧式原地 API 的兼容行为。
 * 参数 buff（void* buff）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 * 参数 count（size_t count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 root（int root）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclBroadcast。
 * 源码返回表达式：return ncclBroadcast(buff, buff, count, datatype, root, comm, stream);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclBcast(void* buff, size_t count, ncclDataType_t datatype, int root,
    ncclComm_t comm, cudaStream_t stream) {
  return ncclBroadcast(buff, buff, count, datatype, root, comm, stream);
}

