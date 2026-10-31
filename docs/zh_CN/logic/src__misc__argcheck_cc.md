# src/misc/argcheck.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0909"></a>

## F0909：CudaPtrCheck

用途：用 CUDA 指针属性验证用户缓冲有效性以及所属设备是否符合 communicator 契约。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 查询属性 | <code>cudaPointerGetAttributes(&attr, pointer)</code> | 先获取地址的CUDA属性；调用失败或devicePointer为空均不可继续使用该用户地址。 |
| 2 | 0 / 条件判断 | <code>err != cudaSuccess &#124;&#124; attr.devicePointer == NULL</code> | 满足时报告具体API/参数名并return ncclInvalidArgument。 |
| 3 | 0 / 编译期类型字段 | <code>CUDART_VERSION &gt;= 10000 ? attr.type : attr.memoryType</code> | 这里是两种CUDA头文件布局的条件编译选择，不是运行时同时执行两条if。 |
| 4 | 0 / 设备匹配 | <code>内存是Device且attr.device != comm-&gt;cudaDev</code> | 设备分配不属于本comm时报告不匹配并return ncclInvalidArgument。 |
| 5 | 0 / 正常出口 | <code>return ncclSuccess</code> | 地址属性与设备所属检查均通过后成功返回。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  cudaPointerAttributes attr;
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
```

</details>

<a id="f0910"></a>

## F0910：PtrCheck

用途：检查必要的指针参数非空，失败时报告具体 API/参数名称。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ptr == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("%s : %s argument is NULL", opname, ptrname);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ptr == NULL) {
    WARN("%s : %s argument is NULL", opname, ptrname);
    return ncclInvalidArgument;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0911"></a>

## F0911：ArgsCheck

用途：验证 root/peer、类型、归约句柄及可选缓冲指针，并归一化复制型操作尺寸和总字节数。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(info-&gt;root &lt; 0 &#124;&#124; info-&gt;root &gt;= info-&gt;comm-&gt;nRanks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("%s : invalid root %d (root should be in the 0..%d range)", info-&gt;opName, info-&gt;root, info-&gt;comm-&gt;nRanks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(info-&gt;datatype &lt; 0 &#124;&#124; info-&gt;datatype &gt;= ncclNumTypes)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>WARN("%s : invalid type %d", info-&gt;opName, info-&gt;datatype);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>info-&gt;nBytes = info-&gt;count * ncclTypeSize(info-&gt;datatype);</code> | 把 NCCL 数据类型枚举映射到每元素字节数，用于尺寸换算；未知类型返回 -1。 |
| 10 | 0 / 条件判断 | <code>(info-&gt;coll == ncclFuncAllGather &#124;&#124; info-&gt;coll == ncclFuncBroadcast)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;count = info-&gt;nBytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;datatype = ncclInt8;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 条件判断 | <code>(info-&gt;coll == ncclFuncAllGather &#124;&#124; info-&gt;coll == ncclFuncReduceScatter)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>info-&gt;nBytes *= info-&gt;comm-&gt;nRanks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 条件判断 | <code>(info-&gt;op &lt; 0 &#124;&#124; ncclMaxRedOp &lt; info-&gt;op)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>WARN("%s : invalid reduction operation %d", info-&gt;opName, info-&gt;op);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 21 | 0 / 准备局部状态 | <code>int opIx = int(ncclUserRedOpMangle(info-&gt;comm, info-&gt;op)) - int(ncclNumOps);</code> | 用 communicator 地址导出的可逆异或扰动用户操作句柄，减少把别的 comm 句柄误用于本 comm 的风险。 |
| 22 | 0 / 条件判断 | <code>(ncclNumOps &lt;= info-&gt;op && (info-&gt;comm-&gt;userRedOpCapacity &lt;= opIx &#124;&#124; info-&gt;comm-&gt;userRedOps[opIx].freeNext != -1))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>WARN("%s : reduction operation %d unknown to this communicator", info-&gt;opName, info-&gt;op);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 0 / 条件判断 | <code>(info-&gt;comm-&gt;checkPointers)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 2 / 条件判断 | <code>((info-&gt;coll == ncclFuncSend &#124;&#124; info-&gt;coll == ncclFuncRecv))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 4 / 条件判断 | <code>(info-&gt;count &gt;0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(CudaPtrCheck(info-&gt;recvbuff, info-&gt;comm, "buff", info-&gt;opName));</code> | 失败时可能提前return，成功才继续；用 CUDA 指针属性验证用户缓冲有效性以及所属设备是否符合 communicator 契约。 |
| 33 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 34 | 4 / 条件判断 | <code>(info-&gt;coll != ncclFuncBroadcast &#124;&#124; info-&gt;comm-&gt;rank == info-&gt;root)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(CudaPtrCheck(info-&gt;sendbuff, info-&gt;comm, "sendbuff", info-&gt;opName));</code> | 失败时可能提前return，成功才继续；用 CUDA 指针属性验证用户缓冲有效性以及所属设备是否符合 communicator 契约。 |
| 37 | 4 / 条件判断 | <code>(info-&gt;coll != ncclFuncReduce &#124;&#124; info-&gt;comm-&gt;rank == info-&gt;root)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(CudaPtrCheck(info-&gt;recvbuff, info-&gt;comm, "recvbuff", info-&gt;opName));</code> | 失败时可能提前return，成功才继续；用 CUDA 指针属性验证用户缓冲有效性以及所属设备是否符合 communicator 契约。 |
| 40 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

