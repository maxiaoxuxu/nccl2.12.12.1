/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "enqueue.h"

NCCL_API(ncclResult_t, ncclAllReduce, const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t datatype, ncclRedOp_t op, ncclComm* comm, cudaStream_t stream);
/* 中文导读：
 * 公开 API 只将用户参数封装为 ncclInfo，指定 AllReduce 的 chunk/slice 参数，再进入统一排队入口。
 * 真正的数据归约在设备端 all_reduce.h 与协议原语中完成，跨机网络推进由 Proxy/NET 后端配合。
 */
/* 中文内部逻辑 F0051：ncclAllReduce
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 准备局部状态：struct ncclInfo info = { ncclFuncAllReduce, "AllReduce", sendbuff, recvbuff, count, datatype, op, 0, comm, stream, ALLREDUCE_CHUNKSTEPS, ALLREDUCE_SLICESTEPS };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 函数返回：return ncclEnqueueCheck(&info);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__collectives__all_reduce_cc.md#f0051
 */
/* 中文逐项-函数 F0051：ncclAllReduce
 * 用途与位置：把每个 rank 的同尺寸输入归约为每个 rank 都持有的结果，API 先封装任务再交给统一调度。
 * 参数 sendbuff（const void* sendbuff）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 参数 recvbuff（void* recvbuff）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 参数 count（size_t count）：公开API输入的datatype元素数；这里尚未归一化成字节，实际字节长度=count*ncclTypeSize(datatype)。
 * 参数 datatype（ncclDataType_t datatype）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 op（ncclRedOp_t op）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 comm（ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 stream（cudaStream_t stream）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclEnqueueCheck。
 * 源码返回表达式：return ncclEnqueueCheck(&info);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclAllReduce(const void* sendbuff, void* recvbuff, size_t count,
    ncclDataType_t datatype, ncclRedOp_t op, ncclComm* comm, cudaStream_t stream) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文精读：从这一 API 开始跟完整调用链
 * 每个 rank 输入 count 个元素，输出同样 count 个归约结果；可同地址原地操作。
 * 示例：rank0=[1,2]，rank1=[10,20]，Sum 后两边输出均为 [11,22]。
 * 此函数只构造 ncclInfo；EnqueueCheck -> SetupCollKernel -> computeColl 选算法/协议及分片。
 * HostSetup 同时装 GPU FIFO 和发布 Proxy 工作，再 LaunchKernel；设备进入 all_reduce.h。
 * Ring/Tree/CollNet 在设备端调用组合原语，最后 ReduceOrCopyMulti 完成数值计算与存储。
 */
/* 中文逐项-变量 V0090：info
 * info（struct ncclInfo；函数 ncclAllReduce 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncAllReduce, "AllReduce", sendbuff, recvbuff, count, datatype, op, 0, comm, stream, / * Args *
 * / ALLREDUCE_CHUNKSTEPS, ALLREDUCE_SLICESTEPS }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncAllReduce, "AllReduce",
    sendbuff, recvbuff, count, datatype, op, 0, comm, stream, /* Args */
    ALLREDUCE_CHUNKSTEPS, ALLREDUCE_SLICESTEPS };
  return ncclEnqueueCheck(&info);
}
