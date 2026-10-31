/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "argcheck.h"
#include "comm.h"

/* 中文内部逻辑 F0909：CudaPtrCheck
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 查询属性：cudaPointerGetAttributes(&attr, pointer)；先获取地址的CUDA属性；调用失败或devicePointer为空均不可继续使用该用户地址。
 * 2. 条件判断：err != cudaSuccess || attr.devicePointer == NULL；满足时报告具体API/参数名并return ncclInvalidArgument。
 * 3. 编译期类型字段：CUDART_VERSION >= 10000 ? attr.type : attr.memoryType；这里是两种CUDA头文件布局的条件编译选择，不是运行时同时执行两条if。
 * 4. 设备匹配：内存是Device且attr.device != comm->cudaDev；设备分配不属于本comm时报告不匹配并return ncclInvalidArgument。
 * 5. 正常出口：return ncclSuccess；地址属性与设备所属检查均通过后成功返回。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__argcheck_cc.md#f0909
 */
/* 中文逐项-函数 F0909：CudaPtrCheck
 * 用途与位置：用 CUDA 指针属性验证用户缓冲有效性以及所属设备是否符合 communicator 契约。
 * 参数 pointer（const void* pointer）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 ptrname（const char* ptrname）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 参数 opname（const char* opname）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/argcheck.cc::ArgsCheck。
 * 本体直接出现的调用/包装：cudaPointerGetAttributes。
 * 需要对照的边界/推进条件：err != cudaSuccess || attr.devicePointer == NULL；attr.memoryType == cudaMemoryTypeDevice &&
 * attr.device != comm->cudaDev。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t CudaPtrCheck(const void* pointer, struct ncclComm* comm, const char* ptrname, const char* opname) {
/* 中文逐项-变量 V3343：attr
 * attr（cudaPointerAttributes；文件/静态存储作用域）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 此处未显式给初值；后续首个文本使用点：cudaError_t err = cudaPointerGetAttributes(&attr, pointer);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cudaPointerAttributes attr;
/* 中文逐项-变量 V3344：err
 * err（cudaError_t；文件/静态存储作用域）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 err = cudaPointerGetAttributes(&attr, pointer)；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
  cudaError_t err = cudaPointerGetAttributes(&attr, pointer);
  if (err != cudaSuccess || attr.devicePointer == NULL) {
    WARN("%s : %s %p is not a valid pointer", opname, ptrname, pointer);
    return ncclInvalidArgument;
  }
#if CUDART_VERSION >= 10000
  if (attr.type == cudaMemoryTypeDevice && attr.device != comm->cudaDev) {
#else
  if (attr.memoryType == cudaMemoryTypeDevice && attr.device != comm->cudaDev) {
#endif
    WARN("%s : %s allocated on device %d mismatchs with NCCL device %d", opname, ptrname, attr.device, comm->cudaDev);
    return ncclInvalidArgument;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0910：PtrCheck
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(ptr == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：WARN("%s : %s argument is NULL", opname, ptrname);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__argcheck_cc.md#f0910
 */
/* 中文逐项-函数 F0910：PtrCheck
 * 用途与位置：检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 opname（const char* opname）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 参数 ptrname（const char* ptrname）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/init.cc::ncclGetUniqueId；src/init.cc::ncclCommInitRankDev；src/init.cc::ncclCommInitAll；src/init.cc::ncclCommGetAsyncError；src/init.cc::ncclCommCount；src/init.cc::ncclCommCuDevice；其余见调用清单。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：ptr == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t PtrCheck(void* ptr, const char* opname, const char* ptrname) {
  if (ptr == NULL) {
    WARN("%s : %s argument is NULL", opname, ptrname);
    return ncclInvalidArgument;
  }
  return ncclSuccess;
}

/* 中文导读：
 * 除参数检查外还统一计量：先计算 count*datatypeSize，将 AllGather/Broadcast 转成字节类型。
 * AllGather/ReduceScatter 的 nBytes 再乘 nRanks，因为公开 count 是单 rank 的贡献/输出规模。
 * 因此 nBytes、内部 count 与公开 API count 不能跨不同 collective 直接套同一解释。
 */
/* 中文内部逻辑 F0911：ArgsCheck
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(info->root < 0 || info->root >= info->comm->nRanks)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：WARN("%s : invalid root %d (root should be in the 0..%d range)", info->opName, info->root, info->comm->nRanks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 条件判断：(info->datatype < 0 || info->datatype >= ncclNumTypes)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：WARN("%s : invalid type %d", info->opName, info->datatype);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     8. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 9. 调用 / 计算 / 写入：info->nBytes = info->count * ncclTypeSize(info->datatype);；把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。
 * 10. 条件判断：(info->coll == ncclFuncAllGather || info->coll == ncclFuncBroadcast)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 调用 / 计算 / 写入：info->count = info->nBytes;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 调用 / 计算 / 写入：info->datatype = ncclInt8;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 条件判断：(info->coll == ncclFuncAllGather || info->coll == ncclFuncReduceScatter)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 调用 / 计算 / 写入：info->nBytes *= info->comm->nRanks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 条件判断：(info->op < 0 || ncclMaxRedOp < info->op)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   18. 真分支：；仅条件为真执行以下子项。
 *     19. 调用 / 计算 / 写入：WARN("%s : invalid reduction operation %d", info->opName, info->op);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     20. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 21. 准备局部状态：int opIx = int(ncclUserRedOpMangle(info->comm, info->op)) - int(ncclNumOps);；用 communicator 地址导出的可逆异或扰动用户操作句柄，减少把别的 comm 句柄误用于本 comm 的风险。
 * 22. 条件判断：(ncclNumOps <= info->op && (info->comm->userRedOpCapacity <= opIx || info->comm->userRedOps[opIx].freeNext != -1))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   23. 真分支：；仅条件为真执行以下子项。
 *     24. 调用 / 计算 / 写入：WARN("%s : reduction operation %d unknown to this communicator", info->opName, info->op);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     25. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 26. 条件判断：(info->comm->checkPointers)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   27. 真分支：；仅条件为真执行以下子项。
 *     28. 条件判断：((info->coll == ncclFuncSend || info->coll == ncclFuncRecv))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       29. 真分支：；仅条件为真执行以下子项。
 *         30. 条件判断：(info->count >0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           31. 真分支：；仅条件为真执行以下子项。
 *             32. 调用 / 计算 / 写入：NCCLCHECK(CudaPtrCheck(info->recvbuff, info->comm, "buff", info->opName));；失败时可能提前return，成功才继续；用 CUDA 指针属性验证用户缓冲有效性以及所属设备是否符合 communicator 契约。
 *       33. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         34. 条件判断：(info->coll != ncclFuncBroadcast || info->comm->rank == info->root)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           35. 真分支：；仅条件为真执行以下子项。
 *             36. 调用 / 计算 / 写入：NCCLCHECK(CudaPtrCheck(info->sendbuff, info->comm, "sendbuff", info->opName));；失败时可能提前return，成功才继续；用 CUDA 指针属性验证用户缓冲有效性以及所属设备是否符合 communicator 契约。
 *         37. 条件判断：(info->coll != ncclFuncReduce || info->comm->rank == info->root)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           38. 真分支：；仅条件为真执行以下子项。
 *             39. 调用 / 计算 / 写入：NCCLCHECK(CudaPtrCheck(info->recvbuff, info->comm, "recvbuff", info->opName));；失败时可能提前return，成功才继续；用 CUDA 指针属性验证用户缓冲有效性以及所属设备是否符合 communicator 契约。
 * 40. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__argcheck_cc.md#f0911
 */
/* 中文逐项-函数 F0911：ArgsCheck
 * 用途与位置：验证 root/peer、类型、归约句柄及可选缓冲指针，并归一化复制型操作尺寸和总字节数。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck。
 * 本体直接出现的调用/包装：CudaPtrCheck；NCCLCHECK；int；ncclTypeSize；ncclUserRedOpMangle。
 * 需要对照的边界/推进条件：info->root < 0 || info->root >= info->comm->nRanks；info->datatype < 0 || info->datatype >=
 * ncclNumTypes；info->coll == ncclFuncAllGather || info->coll == ncclFuncBroadcast；info->coll == ncclFuncAllGather ||
 * info->coll == ncclFuncReduceScatter。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ArgsCheck(struct ncclInfo* info) {
  // First, the easy ones
  if (info->root < 0 || info->root >= info->comm->nRanks) {
    WARN("%s : invalid root %d (root should be in the 0..%d range)", info->opName, info->root, info->comm->nRanks);
    return ncclInvalidArgument;
  }
  if (info->datatype < 0 || info->datatype >= ncclNumTypes) {
    WARN("%s : invalid type %d", info->opName, info->datatype);
    return ncclInvalidArgument;
  }
/* 中文精读：API 参数归一化，分片公式从这里开始
 * 输入 count 的单位是用户 datatype 元素；先乘类型大小得到 nBytes。
 * AllGather/Broadcast 只复制，不需要按数值类型运算，因此 count 换成字节且 datatype 换成 Int8。
 * AllGather 的 sendcount、ReduceScatter 的 recvcount 都是每 rank 的片段长度，
 * 用于性能模型和 Proxy 循环的 nBytes 再乘 nRanks；设备 args->count 仍表示单段长度。
 * 例：4 rank、每 rank 8 个 float，AG 的原始 count=8，归一后 count=32 字节、nBytes=128；
 * RS 的 count=8 个 float、nBytes=128 字节。二者设备下标单位因此不同。
 */
  // Type is OK, compute nbytes. Convert Allgather/Broadcast/P2P calls to chars.
  info->nBytes = info->count * ncclTypeSize(info->datatype);
  if (info->coll == ncclFuncAllGather || info->coll == ncclFuncBroadcast) {
    info->count = info->nBytes;
    info->datatype = ncclInt8;
  }
  if (info->coll == ncclFuncAllGather || info->coll == ncclFuncReduceScatter) info->nBytes *= info->comm->nRanks; // count is per rank

  if (info->op < 0 || ncclMaxRedOp < info->op) {
    WARN("%s : invalid reduction operation %d", info->opName, info->op);
    return ncclInvalidArgument;
  }
/* 中文逐项-变量 V3345：opIx
 * opIx（int；函数 ArgsCheck 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 opIx = int(ncclUserRedOpMangle(info->comm, info->op)) - int(ncclNumOps)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int opIx = int(ncclUserRedOpMangle(info->comm, info->op)) - int(ncclNumOps);
  if (ncclNumOps <= info->op &&
      (info->comm->userRedOpCapacity <= opIx || info->comm->userRedOps[opIx].freeNext != -1)) {
    WARN("%s : reduction operation %d unknown to this communicator", info->opName, info->op);
    return ncclInvalidArgument;
  }

  if (info->comm->checkPointers) {
    if ((info->coll == ncclFuncSend || info->coll == ncclFuncRecv)) {
      if (info->count >0)
        NCCLCHECK(CudaPtrCheck(info->recvbuff, info->comm, "buff", info->opName));
    } else {
      // Check CUDA device pointers
      if (info->coll != ncclFuncBroadcast || info->comm->rank == info->root) {
        NCCLCHECK(CudaPtrCheck(info->sendbuff, info->comm, "sendbuff", info->opName));
      }
      if (info->coll != ncclFuncReduce || info->comm->rank == info->root) {
        NCCLCHECK(CudaPtrCheck(info->recvbuff, info->comm, "recvbuff", info->opName));
      }
    }
  }
  return ncclSuccess;
}
