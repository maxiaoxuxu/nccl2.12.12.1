# 函数与类型逐项索引

本索引指向当前注释版源码中的说明块。下载后即使编辑器不支持GitHub行锚点，也可在对应文件搜索F/T编号。

类型中的enum/union/匿名嵌套对象分别计数；函数含重载与条件编译定义。仅有声明的接口、宏生成getter和变量另列在机器清单，不能把本表当成运行时动态调用图。

## ext-net/dummy/plugin.c

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0001](../../ext-net/dummy/plugin.c#L21) | 函数 | `pluginInit` |
| [F0002](../../ext-net/dummy/plugin.c#L34) | 函数 | `pluginDevices` |
| [F0003](../../ext-net/dummy/plugin.c#L46) | 函数 | `pluginPciPath` |
| [F0004](../../ext-net/dummy/plugin.c#L59) | 函数 | `pluginPtrSupport` |
| [F0005](../../ext-net/dummy/plugin.c#L72) | 函数 | `pluginListen` |
| [F0006](../../ext-net/dummy/plugin.c#L86) | 函数 | `pluginConnect` |
| [F0007](../../ext-net/dummy/plugin.c#L100) | 函数 | `pluginAccept` |
| [F0008](../../ext-net/dummy/plugin.c#L113) | 函数 | `pluginRegMr` |
| [F0009](../../ext-net/dummy/plugin.c#L129) | 函数 | `pluginDeregMr` |
| [F0010](../../ext-net/dummy/plugin.c#L142) | 函数 | `pluginIsend` |
| [F0011](../../ext-net/dummy/plugin.c#L158) | 函数 | `pluginIrecv` |
| [F0012](../../ext-net/dummy/plugin.c#L174) | 函数 | `pluginFlush` |
| [F0013](../../ext-net/dummy/plugin.c#L189) | 函数 | `pluginTest` |
| [F0014](../../ext-net/dummy/plugin.c#L203) | 函数 | `pluginCloseSend` |
| [F0015](../../ext-net/dummy/plugin.c#L215) | 函数 | `pluginCloseRecv` |
| [F0016](../../ext-net/dummy/plugin.c#L227) | 函数 | `pluginCloseListen` |
| [F0017](../../ext-net/dummy/plugin.c#L266) | 函数 | `pluginCollNetInit` |
| [F0018](../../ext-net/dummy/plugin.c#L279) | 函数 | `pluginCollNetDevices` |
| [F0019](../../ext-net/dummy/plugin.c#L291) | 函数 | `pluginCollNetPciPath` |
| [F0020](../../ext-net/dummy/plugin.c#L304) | 函数 | `pluginCollNetPtrSupport` |
| [F0021](../../ext-net/dummy/plugin.c#L317) | 函数 | `pluginCollNetListen` |
| [F0022](../../ext-net/dummy/plugin.c#L331) | 函数 | `pluginCollNetConnect` |
| [F0023](../../ext-net/dummy/plugin.c#L347) | 函数 | `pluginCollNetReduceSupport` |
| [F0024](../../ext-net/dummy/plugin.c#L361) | 函数 | `pluginCollNetRegMr` |
| [F0025](../../ext-net/dummy/plugin.c#L377) | 函数 | `pluginCollNetDeregMr` |
| [F0026](../../ext-net/dummy/plugin.c#L390) | 函数 | `pluginCollNetIallreduce` |
| [F0027](../../ext-net/dummy/plugin.c#L410) | 函数 | `pluginCollNetFlush` |
| [F0028](../../ext-net/dummy/plugin.c#L425) | 函数 | `pluginCollNetTest` |
| [F0029](../../ext-net/dummy/plugin.c#L439) | 函数 | `pluginCollNetCloseColl` |
| [F0030](../../ext-net/dummy/plugin.c#L451) | 函数 | `pluginCollNetCloseListen` |

## src/bootstrap.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0031](../../src/bootstrap.cc#L74) | 函数 | `bootstrapNetInit` |
| [F0032](../../src/bootstrap.cc#L156) | 函数 | `bootstrapNetSend` |
| [F0033](../../src/bootstrap.cc#L182) | 函数 | `bootstrapNetRecv` |
| [F0034](../../src/bootstrap.cc#L241) | 函数 | `setFilesLimit` |
| [F0035](../../src/bootstrap.cc#L323) | 函数 | `bootstrapRoot` |
| [F0036](../../src/bootstrap.cc#L460) | 函数 | `bootstrapCreateRoot` |
| [F0037](../../src/bootstrap.cc#L518) | 函数 | `bootstrapGetUniqueId` |
| [F0038](../../src/bootstrap.cc#L685) | 函数 | `bootstrapInit` |
| [F0039](../../src/bootstrap.cc#L825) | 函数 | `bootstrapAllGather` |
| [F0040](../../src/bootstrap.cc#L903) | 函数 | `bootstrapSend` |
| [F0041](../../src/bootstrap.cc#L958) | 函数 | `bootstrapBarrier` |
| [F0042](../../src/bootstrap.cc#L1025) | 函数 | `bootstrapIntraNodeAllGather` |
| [F0043](../../src/bootstrap.cc#L1089) | 函数 | `unexpectedEnqueue` |
| [F0044](../../src/bootstrap.cc#L1149) | 函数 | `unexpectedDequeue` |
| [F0045](../../src/bootstrap.cc#L1221) | 函数 | `bootstrapRecv` |
| [F0046](../../src/bootstrap.cc#L1292) | 函数 | `bootstrapClose` |
| [F0047](../../src/bootstrap.cc#L1341) | 函数 | `bootstrapAbort` |
| [T0001](../../src/bootstrap.cc#L136) | 类型 | `bootstrapInterface_t` |
| [T0002](../../src/bootstrap.cc#L207) | 类型 | `extInfo` |
| [T0003](../../src/bootstrap.cc#L554) | 类型 | `unexConn` |
| [T0004](../../src/bootstrap.cc#L577) | 类型 | `bootstrapState` |

## src/channel.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0048](../../src/channel.cc#L49) | 函数 | `initChannel` |
| [F0049](../../src/channel.cc#L144) | 函数 | `freeChannel` |

## src/collectives/all_gather.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0050](../../src/collectives/all_gather.cc#L23) | 函数 | `ncclAllGather` |

## src/collectives/all_reduce.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0051](../../src/collectives/all_reduce.cc#L22) | 函数 | `ncclAllReduce` |

## src/collectives/broadcast.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0052](../../src/collectives/broadcast.cc#L23) | 函数 | `ncclBroadcast` |
| [F0053](../../src/collectives/broadcast.cc#L64) | 函数 | `ncclBcast` |

## src/collectives/device/all_gather.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0054](../../src/collectives/device/all_gather.h#L68) | 函数 | `runRing` |
| [F0055](../../src/collectives/device/all_gather.h#L252) | 函数 | `run` |
| [F0056](../../src/collectives/device/all_gather.h#L274) | 函数 | `run` |
| [F0057](../../src/collectives/device/all_gather.h#L295) | 函数 | `run` |
| [T0005](../../src/collectives/device/all_gather.h#L241) | 类型 | `RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>` |
| [T0006](../../src/collectives/device/all_gather.h#L264) | 类型 | `RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>` |
| [T0007](../../src/collectives/device/all_gather.h#L285) | 类型 | `RunWorkElement<ncclFuncAllGather, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>` |

## src/collectives/device/all_reduce.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0058](../../src/collectives/device/all_reduce.h#L85) | 函数 | `runRing` |
| [F0059](../../src/collectives/device/all_reduce.h#L361) | 函数 | `runTreeUpDown` |
| [F0060](../../src/collectives/device/all_reduce.h#L634) | 函数 | `runTreeSplit` |
| [F0061](../../src/collectives/device/all_reduce.h#L871) | 函数 | `run` |
| [F0062](../../src/collectives/device/all_reduce.h#L896) | 函数 | `run` |
| [F0063](../../src/collectives/device/all_reduce.h#L1003) | 函数 | `run` |
| [F0064](../../src/collectives/device/all_reduce.h#L1320) | 函数 | `run` |
| [F0065](../../src/collectives/device/all_reduce.h#L1341) | 函数 | `run` |
| [F0066](../../src/collectives/device/all_reduce.h#L1362) | 函数 | `run` |
| [F0067](../../src/collectives/device/all_reduce.h#L1383) | 函数 | `run` |
| [T0008](../../src/collectives/device/all_reduce.h#L860) | 类型 | `RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>` |
| [T0009](../../src/collectives/device/all_reduce.h#L883) | 类型 | `RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_SIMPLE>` |
| [T0010](../../src/collectives/device/all_reduce.h#L911) | 类型 | `RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_COLLNET, NCCL_PROTO_SIMPLE>` |
| [T0011](../../src/collectives/device/all_reduce.h#L1310) | 类型 | `RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>` |
| [T0012](../../src/collectives/device/all_reduce.h#L1331) | 类型 | `RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL>` |
| [T0013](../../src/collectives/device/all_reduce.h#L1352) | 类型 | `RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>` |
| [T0014](../../src/collectives/device/all_reduce.h#L1373) | 类型 | `RunWorkElement<ncclFuncAllReduce, T, RedOp, NCCL_ALGO_TREE, NCCL_PROTO_LL128>` |

## src/collectives/device/broadcast.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0068](../../src/collectives/device/broadcast.h#L64) | 函数 | `runRing` |
| [F0069](../../src/collectives/device/broadcast.h#L218) | 函数 | `run` |
| [F0070](../../src/collectives/device/broadcast.h#L240) | 函数 | `run` |
| [F0071](../../src/collectives/device/broadcast.h#L261) | 函数 | `run` |
| [T0015](../../src/collectives/device/broadcast.h#L207) | 类型 | `RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>` |
| [T0016](../../src/collectives/device/broadcast.h#L230) | 类型 | `RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>` |
| [T0017](../../src/collectives/device/broadcast.h#L251) | 类型 | `RunWorkElement<ncclFuncBroadcast, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>` |

## src/collectives/device/common.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0072](../../src/collectives/device/common.h#L45) | 函数 | `barrierReduceAny` |
| [F0073](../../src/collectives/device/common.h#L87) | 函数 | `copyToShmem` |
| [F0074](../../src/collectives/device/common.h#L172) | 函数 | `copyToShmem` |
| [F0075](../../src/collectives/device/common.h#L248) | 函数 | `run` |
| [F0076](../../src/collectives/device/common.h#L277) | 函数 | `run` |
| [F0077](../../src/collectives/device/common.h#L410) | 函数 | `ncclRedopPtrDeref` |
| [F0078](../../src/collectives/device/common.h#L504) | 函数 | `ncclKernel` |
| [T0018](../../src/collectives/device/common.h#L239) | 类型 | `RunWorkElement` |
| [T0019](../../src/collectives/device/common.h#L258) | 类型 | `RunWork` |
| [T0020](../../src/collectives/device/common.h#L315) | 类型 | `ncclShmemGroup` |
| [T0021](../../src/collectives/device/common.h#L348) | 类型 | `ncclShmemData` |
| [T0022](../../src/collectives/device/common.h#L353) | 类型 | `<匿名union>` |

## src/collectives/device/common_kernel.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0079](../../src/collectives/device/common_kernel.h#L26) | 函数 | `min` |
| [F0080](../../src/collectives/device/common_kernel.h#L43) | 函数 | `loadInt` |
| [F0081](../../src/collectives/device/common_kernel.h#L121) | 函数 | `operator()` |
| [F0082](../../src/collectives/device/common_kernel.h#L159) | 函数 | `preOp` |
| [F0083](../../src/collectives/device/common_kernel.h#L208) | 函数 | `postOp` |
| [F0084](../../src/collectives/device/common_kernel.h#L289) | 函数 | `operator()` |
| [F0085](../../src/collectives/device/common_kernel.h#L327) | 函数 | `preOp` |
| [F0086](../../src/collectives/device/common_kernel.h#L376) | 函数 | `postOp` |
| [F0087](../../src/collectives/device/common_kernel.h#L457) | 函数 | `operator()` |
| [F0088](../../src/collectives/device/common_kernel.h#L492) | 函数 | `preOp` |
| [F0089](../../src/collectives/device/common_kernel.h#L533) | 函数 | `postOp` |
| [F0090](../../src/collectives/device/common_kernel.h#L608) | 函数 | `operator()` |
| [F0091](../../src/collectives/device/common_kernel.h#L643) | 函数 | `preOp` |
| [F0092](../../src/collectives/device/common_kernel.h#L684) | 函数 | `postOp` |
| [F0093](../../src/collectives/device/common_kernel.h#L752) | 函数 | `operator()` |
| [F0094](../../src/collectives/device/common_kernel.h#L785) | 函数 | `preOp` |
| [F0095](../../src/collectives/device/common_kernel.h#L812) | 函数 | `postOp` |
| [F0096](../../src/collectives/device/common_kernel.h#L867) | 函数 | `operator()` |
| [F0097](../../src/collectives/device/common_kernel.h#L900) | 函数 | `preOp` |
| [F0098](../../src/collectives/device/common_kernel.h#L927) | 函数 | `postOp` |
| [F0099](../../src/collectives/device/common_kernel.h#L989) | 函数 | `operator()` |
| [F0100](../../src/collectives/device/common_kernel.h#L1024) | 函数 | `preOp` |
| [F0101](../../src/collectives/device/common_kernel.h#L1065) | 函数 | `postOp` |
| [F0102](../../src/collectives/device/common_kernel.h#L1113) | 函数 | `operator()` |
| [F0103](../../src/collectives/device/common_kernel.h#L1137) | 函数 | `preOp` |
| [F0104](../../src/collectives/device/common_kernel.h#L1176) | 函数 | `postOp` |
| [F0105](../../src/collectives/device/common_kernel.h#L1223) | 函数 | `operator()` |
| [F0106](../../src/collectives/device/common_kernel.h#L1247) | 函数 | `preOp` |
| [F0107](../../src/collectives/device/common_kernel.h#L1286) | 函数 | `postOp` |
| [F0108](../../src/collectives/device/common_kernel.h#L1333) | 函数 | `operator()` |
| [F0109](../../src/collectives/device/common_kernel.h#L1357) | 函数 | `preOp` |
| [F0110](../../src/collectives/device/common_kernel.h#L1396) | 函数 | `postOp` |
| [F0111](../../src/collectives/device/common_kernel.h#L1434) | 函数 | `vFetch` |
| [F0112](../../src/collectives/device/common_kernel.h#L1451) | 函数 | `vStore` |
| [F0113](../../src/collectives/device/common_kernel.h#L1471) | 函数 | `vFetch<half>` |
| [F0114](../../src/collectives/device/common_kernel.h#L1494) | 函数 | `vStore<half>` |
| [F0115](../../src/collectives/device/common_kernel.h#L1513) | 函数 | `vFetch<half>` |
| [F0116](../../src/collectives/device/common_kernel.h#L1536) | 函数 | `vStore<half>` |
| [F0117](../../src/collectives/device/common_kernel.h#L1557) | 函数 | `vFetch<__nv_bfloat16>` |
| [F0118](../../src/collectives/device/common_kernel.h#L1580) | 函数 | `vStore<__nv_bfloat16>` |
| [F0119](../../src/collectives/device/common_kernel.h#L1606) | 函数 | `operator()` |
| [F0120](../../src/collectives/device/common_kernel.h#L1623) | 函数 | `preOp` |
| [F0121](../../src/collectives/device/common_kernel.h#L1639) | 函数 | `postOp` |
| [F0122](../../src/collectives/device/common_kernel.h#L1656) | 函数 | `Fetch128` |
| [F0123](../../src/collectives/device/common_kernel.h#L1671) | 函数 | `Store128` |
| [F0124](../../src/collectives/device/common_kernel.h#L1770) | 函数 | `ReduceCopyMulti` |
| [F0125](../../src/collectives/device/common_kernel.h#L2048) | 函数 | `ReduceCopy128bMulti` |
| [F0126](../../src/collectives/device/common_kernel.h#L2247) | 函数 | `ptrAlign128` |
| [F0127](../../src/collectives/device/common_kernel.h#L2319) | 函数 | `ReduceOrCopyMulti` |
| [T0023](../../src/collectives/device/common_kernel.h#L70) | 类型 | `MULTI` |
| [T0024](../../src/collectives/device/common_kernel.h#L80) | 类型 | `MULTI<FUNC, int8_t>` |
| [T0025](../../src/collectives/device/common_kernel.h#L88) | 类型 | `converter` |
| [T0026](../../src/collectives/device/common_kernel.h#L98) | 类型 | `<匿名struct>` |
| [T0027](../../src/collectives/device/common_kernel.h#L172) | 类型 | `<匿名union>` |
| [T0028](../../src/collectives/device/common_kernel.h#L221) | 类型 | `<匿名union>` |
| [T0029](../../src/collectives/device/common_kernel.h#L248) | 类型 | `MULTI<FUNC, uint8_t>` |
| [T0030](../../src/collectives/device/common_kernel.h#L256) | 类型 | `converter` |
| [T0031](../../src/collectives/device/common_kernel.h#L266) | 类型 | `<匿名struct>` |
| [T0032](../../src/collectives/device/common_kernel.h#L340) | 类型 | `<匿名union>` |
| [T0033](../../src/collectives/device/common_kernel.h#L389) | 类型 | `<匿名union>` |
| [T0034](../../src/collectives/device/common_kernel.h#L416) | 类型 | `MULTI<FUNC, int32_t>` |
| [T0035](../../src/collectives/device/common_kernel.h#L424) | 类型 | `converter` |
| [T0036](../../src/collectives/device/common_kernel.h#L434) | 类型 | `<匿名struct>` |
| [T0037](../../src/collectives/device/common_kernel.h#L504) | 类型 | `<匿名union>` |
| [T0038](../../src/collectives/device/common_kernel.h#L545) | 类型 | `<匿名union>` |
| [T0039](../../src/collectives/device/common_kernel.h#L567) | 类型 | `MULTI<FUNC, uint32_t>` |
| [T0040](../../src/collectives/device/common_kernel.h#L575) | 类型 | `converter` |
| [T0041](../../src/collectives/device/common_kernel.h#L585) | 类型 | `<匿名struct>` |
| [T0042](../../src/collectives/device/common_kernel.h#L655) | 类型 | `<匿名union>` |
| [T0043](../../src/collectives/device/common_kernel.h#L696) | 类型 | `<匿名union>` |
| [T0044](../../src/collectives/device/common_kernel.h#L718) | 类型 | `MULTI<FUNC, half>` |
| [T0045](../../src/collectives/device/common_kernel.h#L727) | 类型 | `Converter` |
| [T0046](../../src/collectives/device/common_kernel.h#L833) | 类型 | `MULTI<FUNC, __nv_bfloat16>` |
| [T0047](../../src/collectives/device/common_kernel.h#L842) | 类型 | `Converter` |
| [T0048](../../src/collectives/device/common_kernel.h#L948) | 类型 | `MULTI<FUNC, float>` |
| [T0049](../../src/collectives/device/common_kernel.h#L956) | 类型 | `converter` |
| [T0050](../../src/collectives/device/common_kernel.h#L966) | 类型 | `<匿名struct>` |
| [T0051](../../src/collectives/device/common_kernel.h#L1036) | 类型 | `<匿名union>` |
| [T0052](../../src/collectives/device/common_kernel.h#L1077) | 类型 | `<匿名union>` |
| [T0053](../../src/collectives/device/common_kernel.h#L1099) | 类型 | `MULTI<FUNC, double>` |
| [T0054](../../src/collectives/device/common_kernel.h#L1149) | 类型 | `<匿名union>` |
| [T0055](../../src/collectives/device/common_kernel.h#L1188) | 类型 | `<匿名union>` |
| [T0056](../../src/collectives/device/common_kernel.h#L1209) | 类型 | `MULTI<FUNC, uint64_t>` |
| [T0057](../../src/collectives/device/common_kernel.h#L1259) | 类型 | `<匿名union>` |
| [T0058](../../src/collectives/device/common_kernel.h#L1298) | 类型 | `<匿名union>` |
| [T0059](../../src/collectives/device/common_kernel.h#L1319) | 类型 | `MULTI<FUNC, int64_t>` |
| [T0060](../../src/collectives/device/common_kernel.h#L1369) | 类型 | `<匿名union>` |
| [T0061](../../src/collectives/device/common_kernel.h#L1408) | 类型 | `<匿名union>` |
| [T0062](../../src/collectives/device/common_kernel.h#L1595) | 类型 | `MULTI128` |

## src/collectives/device/functions.cu

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0128](../../src/collectives/device/functions.cu#L166) | 函数 | `ncclWorkaroundClangD55580` |

## src/collectives/device/onerank_reduce.cu

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0129](../../src/collectives/device/onerank_reduce.cu#L40) | 函数 | `oneRankReduce` |

## src/collectives/device/op128.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0130](../../src/collectives/device/op128.h#L25) | 函数 | `load128` |
| [F0131](../../src/collectives/device/op128.h#L43) | 函数 | `store128` |
| [F0132](../../src/collectives/device/op128.h#L63) | 函数 | `shmemCvtPtr` |
| [F0133](../../src/collectives/device/op128.h#L86) | 函数 | `loadShmem128` |
| [F0134](../../src/collectives/device/op128.h#L104) | 函数 | `storeShmem128` |
| [F0135](../../src/collectives/device/op128.h#L146) | 函数 | `loadShmemMisaligned128` |
| [T0063](../../src/collectives/device/op128.h#L157) | 类型 | `<匿名union>` |

## src/collectives/device/primitives.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0136](../../src/collectives/device/primitives.h#L66) | 函数 | `calcBytePerStep` |
| [F0137](../../src/collectives/device/primitives.h#L81) | 函数 | `calcBytePerGrain` |
| [F0138](../../src/collectives/device/primitives.h#L100) | 函数 | `calcGroupWidth` |
| [F0139](../../src/collectives/device/primitives.h#L128) | 函数 | `calcBytePerStep` |
| [F0140](../../src/collectives/device/primitives.h#L143) | 函数 | `calcBytePerGrain` |
| [F0141](../../src/collectives/device/primitives.h#L162) | 函数 | `calcGroupWidth` |
| [F0142](../../src/collectives/device/primitives.h#L190) | 函数 | `calcBytePerStep` |
| [F0143](../../src/collectives/device/primitives.h#L206) | 函数 | `calcBytePerGrain` |
| [F0144](../../src/collectives/device/primitives.h#L226) | 函数 | `calcGroupWidth` |
| [F0145](../../src/collectives/device/primitives.h#L271) | 函数 | `FanAsymmetric` |
| [F0146](../../src/collectives/device/primitives.h#L285) | 函数 | `nrecv` |
| [F0147](../../src/collectives/device/primitives.h#L297) | 函数 | `nsend` |
| [F0148](../../src/collectives/device/primitives.h#L326) | 函数 | `FanSymmetric` |
| [F0149](../../src/collectives/device/primitives.h#L340) | 函数 | `nrecv` |
| [F0150](../../src/collectives/device/primitives.h#L352) | 函数 | `nsend` |
| [F0151](../../src/collectives/device/primitives.h#L382) | 函数 | `directSend` |
| [F0152](../../src/collectives/device/primitives.h#L397) | 函数 | `directSendFromOutput` |
| [F0153](../../src/collectives/device/primitives.h#L412) | 函数 | `directRecv` |
| [F0154](../../src/collectives/device/primitives.h#L426) | 函数 | `directCopySend` |
| [F0155](../../src/collectives/device/primitives.h#L443) | 函数 | `directRecvCopySend` |
| [F0156](../../src/collectives/device/primitives.h#L458) | 函数 | `directRecvReduceCopySend` |
| [T0064](../../src/collectives/device/primitives.h#L37) | 类型 | `ProtoSimple` |
| [T0065](../../src/collectives/device/primitives.h#L112) | 类型 | `ProtoLL` |
| [T0066](../../src/collectives/device/primitives.h#L174) | 类型 | `ProtoLL128` |
| [T0067](../../src/collectives/device/primitives.h#L250) | 类型 | `FanAsymmetric` |
| [T0068](../../src/collectives/device/primitives.h#L306) | 类型 | `FanSymmetric` |
| [T0069](../../src/collectives/device/primitives.h#L366) | 类型 | `PrimitivesWithoutDirect` |

## src/collectives/device/prims_ll.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0157](../../src/collectives/device/prims_ll.h#L113) | 函数 | `recvOffset` |
| [F0158](../../src/collectives/device/prims_ll.h#L126) | 函数 | `sendOffset` |
| [F0159](../../src/collectives/device/prims_ll.h#L139) | 函数 | `recvPtr` |
| [F0160](../../src/collectives/device/prims_ll.h#L152) | 函数 | `sendPtr` |
| [F0161](../../src/collectives/device/prims_ll.h#L165) | 函数 | `recvFlag` |
| [F0162](../../src/collectives/device/prims_ll.h#L178) | 函数 | `sendFlag` |
| [F0163](../../src/collectives/device/prims_ll.h#L192) | 函数 | `barrier` |
| [F0164](../../src/collectives/device/prims_ll.h#L217) | 函数 | `checkAbort` |
| [F0165](../../src/collectives/device/prims_ll.h#L260) | 函数 | `waitSend` |
| [F0166](../../src/collectives/device/prims_ll.h#L299) | 函数 | `incRecv` |
| [F0167](../../src/collectives/device/prims_ll.h#L316) | 函数 | `postRecv` |
| [F0168](../../src/collectives/device/prims_ll.h#L341) | 函数 | `incSend` |
| [F0169](../../src/collectives/device/prims_ll.h#L386) | 函数 | `readLL` |
| [F0170](../../src/collectives/device/prims_ll.h#L448) | 函数 | `readLLBeginAll` |
| [F0171](../../src/collectives/device/prims_ll.h#L489) | 函数 | `readLLFinish` |
| [F0172](../../src/collectives/device/prims_ll.h#L531) | 函数 | `storeLL` |
| [F0173](../../src/collectives/device/prims_ll.h#L566) | 函数 | `load` |
| [F0174](../../src/collectives/device/prims_ll.h#L629) | 函数 | `store` |
| [F0175](../../src/collectives/device/prims_ll.h#L721) | 函数 | `loadBegin` |
| [F0176](../../src/collectives/device/prims_ll.h#L764) | 函数 | `loadFinish` |
| [F0177](../../src/collectives/device/prims_ll.h#L793) | 函数 | `storeData` |
| [F0178](../../src/collectives/device/prims_ll.h#L905) | 函数 | `LLGenericOp` |
| [F0179](../../src/collectives/device/prims_ll.h#L1057) | 函数 | `loadRecvConn` |
| [F0180](../../src/collectives/device/prims_ll.h#L1077) | 函数 | `loadRecvSync` |
| [F0181](../../src/collectives/device/prims_ll.h#L1099) | 函数 | `loadSendConn` |
| [F0182](../../src/collectives/device/prims_ll.h#L1121) | 函数 | `loadSendSync` |
| [F0183](../../src/collectives/device/prims_ll.h#L1155) | 函数 | `Primitives` |
| [F0184](../../src/collectives/device/prims_ll.h#L1214) | 函数 | `~Primitives` |
| [F0185](../../src/collectives/device/prims_ll.h#L1237) | 函数 | `setDataPtrs` |
| [F0186](../../src/collectives/device/prims_ll.h#L1254) | 函数 | `moveDataPtrs` |
| [F0187](../../src/collectives/device/prims_ll.h#L1269) | 函数 | `send` |
| [F0188](../../src/collectives/device/prims_ll.h#L1284) | 函数 | `sendFromOutput` |
| [F0189](../../src/collectives/device/prims_ll.h#L1299) | 函数 | `recv` |
| [F0190](../../src/collectives/device/prims_ll.h#L1315) | 函数 | `recvReduceSend` |
| [F0191](../../src/collectives/device/prims_ll.h#L1330) | 函数 | `recvReduceCopy` |
| [F0192](../../src/collectives/device/prims_ll.h#L1347) | 函数 | `copySend` |
| [F0193](../../src/collectives/device/prims_ll.h#L1364) | 函数 | `recvCopySend` |
| [F0194](../../src/collectives/device/prims_ll.h#L1380) | 函数 | `recvReduceCopySend` |
| [T0070](../../src/collectives/device/prims_ll.h#L7) | 类型 | `Primitives<T, RedOp, Fan, Direct, ProtoLL, P2p>` |
| [T0071](../../src/collectives/device/prims_ll.h#L576) | 类型 | `<匿名union>` |
| [T0072](../../src/collectives/device/prims_ll.h#L639) | 类型 | `<匿名union>` |
| [T0073](../../src/collectives/device/prims_ll.h#L673) | 类型 | `DataLoader` |
| [T0074](../../src/collectives/device/prims_ll.h#L682) | 类型 | `<匿名union>` |
| [T0075](../../src/collectives/device/prims_ll.h#L802) | 类型 | `<匿名union>` |

## src/collectives/device/prims_ll128.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0195](../../src/collectives/device/prims_ll128.h#L146) | 函数 | `recvOffset` |
| [F0196](../../src/collectives/device/prims_ll128.h#L159) | 函数 | `sendOffset` |
| [F0197](../../src/collectives/device/prims_ll128.h#L172) | 函数 | `recvPtr` |
| [F0198](../../src/collectives/device/prims_ll128.h#L185) | 函数 | `sendPtr` |
| [F0199](../../src/collectives/device/prims_ll128.h#L198) | 函数 | `recvFlag` |
| [F0200](../../src/collectives/device/prims_ll128.h#L211) | 函数 | `sendFlag` |
| [F0201](../../src/collectives/device/prims_ll128.h#L225) | 函数 | `barrier` |
| [F0202](../../src/collectives/device/prims_ll128.h#L250) | 函数 | `checkAbort` |
| [F0203](../../src/collectives/device/prims_ll128.h#L286) | 函数 | `waitSend` |
| [F0204](../../src/collectives/device/prims_ll128.h#L319) | 函数 | `postRecv` |
| [F0205](../../src/collectives/device/prims_ll128.h#L337) | 函数 | `postSend` |
| [F0206](../../src/collectives/device/prims_ll128.h#L390) | 函数 | `loadRegsBegin` |
| [F0207](../../src/collectives/device/prims_ll128.h#L504) | 函数 | `loadRegsFinish` |
| [F0208](../../src/collectives/device/prims_ll128.h#L554) | 函数 | `storeRegs` |
| [F0209](../../src/collectives/device/prims_ll128.h#L721) | 函数 | `recvReduceSendCopy` |
| [F0210](../../src/collectives/device/prims_ll128.h#L998) | 函数 | `GenericOp` |
| [F0211](../../src/collectives/device/prims_ll128.h#L1106) | 函数 | `loadRecvConn` |
| [F0212](../../src/collectives/device/prims_ll128.h#L1126) | 函数 | `loadRecvSync` |
| [F0213](../../src/collectives/device/prims_ll128.h#L1148) | 函数 | `loadSendConn` |
| [F0214](../../src/collectives/device/prims_ll128.h#L1176) | 函数 | `loadSendSync` |
| [F0215](../../src/collectives/device/prims_ll128.h#L1216) | 函数 | `Primitives` |
| [F0216](../../src/collectives/device/prims_ll128.h#L1275) | 函数 | `~Primitives` |
| [F0217](../../src/collectives/device/prims_ll128.h#L1298) | 函数 | `setDataPtrs` |
| [F0218](../../src/collectives/device/prims_ll128.h#L1315) | 函数 | `moveDataPtrs` |
| [F0219](../../src/collectives/device/prims_ll128.h#L1330) | 函数 | `send` |
| [F0220](../../src/collectives/device/prims_ll128.h#L1345) | 函数 | `sendFromOutput` |
| [F0221](../../src/collectives/device/prims_ll128.h#L1360) | 函数 | `recv` |
| [F0222](../../src/collectives/device/prims_ll128.h#L1376) | 函数 | `recvReduceSend` |
| [F0223](../../src/collectives/device/prims_ll128.h#L1391) | 函数 | `recvReduceCopy` |
| [F0224](../../src/collectives/device/prims_ll128.h#L1408) | 函数 | `copySend` |
| [F0225](../../src/collectives/device/prims_ll128.h#L1425) | 函数 | `recvCopySend` |
| [F0226](../../src/collectives/device/prims_ll128.h#L1441) | 函数 | `recvReduceCopySend` |
| [T0076](../../src/collectives/device/prims_ll128.h#L21) | 类型 | `Primitives<T, RedOp, Fan, Direct, ProtoLL128, P2p>` |

## src/collectives/device/prims_simple.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0227](../../src/collectives/device/prims_simple.h#L174) | 函数 | `barrier` |
| [F0228](../../src/collectives/device/prims_simple.h#L197) | 函数 | `subBarrier` |
| [F0229](../../src/collectives/device/prims_simple.h#L222) | 函数 | `checkAbort` |
| [F0230](../../src/collectives/device/prims_simple.h#L294) | 函数 | `waitPeer` |
| [F0231](../../src/collectives/device/prims_simple.h#L394) | 函数 | `postPeer` |
| [F0232](../../src/collectives/device/prims_simple.h#L478) | 函数 | `genericOp` |
| [F0233](../../src/collectives/device/prims_simple.h#L740) | 函数 | `ScatterGatherOp` |
| [F0234](../../src/collectives/device/prims_simple.h#L943) | 函数 | `loadRecvConn` |
| [F0235](../../src/collectives/device/prims_simple.h#L1039) | 函数 | `loadSendConn` |
| [F0236](../../src/collectives/device/prims_simple.h#L1166) | 函数 | `Primitives` |
| [F0237](../../src/collectives/device/prims_simple.h#L1272) | 函数 | `~Primitives` |
| [F0238](../../src/collectives/device/prims_simple.h#L1380) | 函数 | `setDataPtrs` |
| [F0239](../../src/collectives/device/prims_simple.h#L1559) | 函数 | `moveDataPtrs` |
| [F0240](../../src/collectives/device/prims_simple.h#L1582) | 函数 | `send` |
| [F0241](../../src/collectives/device/prims_simple.h#L1596) | 函数 | `sendFromOutput` |
| [F0242](../../src/collectives/device/prims_simple.h#L1610) | 函数 | `directSend` |
| [F0243](../../src/collectives/device/prims_simple.h#L1625) | 函数 | `directSendFromOutput` |
| [F0244](../../src/collectives/device/prims_simple.h#L1641) | 函数 | `recv` |
| [F0245](../../src/collectives/device/prims_simple.h#L1656) | 函数 | `directRecv` |
| [F0246](../../src/collectives/device/prims_simple.h#L1671) | 函数 | `copySend` |
| [F0247](../../src/collectives/device/prims_simple.h#L1687) | 函数 | `directCopySend` |
| [F0248](../../src/collectives/device/prims_simple.h#L1705) | 函数 | `recvCopySend` |
| [F0249](../../src/collectives/device/prims_simple.h#L1720) | 函数 | `directRecvCopySend` |
| [F0250](../../src/collectives/device/prims_simple.h#L1735) | 函数 | `recvCopyDirectSend` |
| [F0251](../../src/collectives/device/prims_simple.h#L1752) | 函数 | `recvReduceCopy` |
| [F0252](../../src/collectives/device/prims_simple.h#L1769) | 函数 | `recvReduceSend` |
| [F0253](../../src/collectives/device/prims_simple.h#L1784) | 函数 | `directRecvReduceSend` |
| [F0254](../../src/collectives/device/prims_simple.h#L1801) | 函数 | `recvReduceCopySend` |
| [F0255](../../src/collectives/device/prims_simple.h#L1817) | 函数 | `directRecvReduceCopySend` |
| [F0256](../../src/collectives/device/prims_simple.h#L1836) | 函数 | `scatter` |
| [F0257](../../src/collectives/device/prims_simple.h#L1854) | 函数 | `directScatter` |
| [F0258](../../src/collectives/device/prims_simple.h#L1873) | 函数 | `gather` |
| [F0259](../../src/collectives/device/prims_simple.h#L1892) | 函数 | `directGather` |
| [T0077](../../src/collectives/device/prims_simple.h#L7) | 类型 | `Primitives<     T, RedOp, Fan, Direct, ProtoSimple<SlicePerChunk, StepPerSlice, Unroll>, P2p   >` |
| [T0078](../../src/collectives/device/prims_simple.h#L118) | 类型 | `<匿名union>` |
| [T0079](../../src/collectives/device/prims_simple.h#L135) | 类型 | `<匿名union>` |

## src/collectives/device/reduce.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0260](../../src/collectives/device/reduce.h#L72) | 函数 | `runRing` |
| [F0261](../../src/collectives/device/reduce.h#L274) | 函数 | `run` |
| [F0262](../../src/collectives/device/reduce.h#L296) | 函数 | `run` |
| [F0263](../../src/collectives/device/reduce.h#L317) | 函数 | `run` |
| [T0080](../../src/collectives/device/reduce.h#L263) | 类型 | `RunWorkElement<ncclFuncReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>` |
| [T0081](../../src/collectives/device/reduce.h#L286) | 类型 | `RunWorkElement<ncclFuncReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>` |
| [T0082](../../src/collectives/device/reduce.h#L307) | 类型 | `RunWorkElement<ncclFuncReduce, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>` |

## src/collectives/device/reduce_kernel.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0264](../../src/collectives/device/reduce_kernel.h#L28) | 函数 | `FuncNull` |
| [F0265](../../src/collectives/device/reduce_kernel.h#L39) | 函数 | `operator()` |
| [F0266](../../src/collectives/device/reduce_kernel.h#L66) | 函数 | `FuncSum` |
| [F0267](../../src/collectives/device/reduce_kernel.h#L77) | 函数 | `operator()` |
| [F0268](../../src/collectives/device/reduce_kernel.h#L98) | 函数 | `FuncProd` |
| [F0269](../../src/collectives/device/reduce_kernel.h#L109) | 函数 | `operator()` |
| [F0270](../../src/collectives/device/reduce_kernel.h#L130) | 函数 | `FuncMax` |
| [F0271](../../src/collectives/device/reduce_kernel.h#L141) | 函数 | `operator()` |
| [F0272](../../src/collectives/device/reduce_kernel.h#L162) | 函数 | `FuncMin` |
| [F0273](../../src/collectives/device/reduce_kernel.h#L173) | 函数 | `operator()` |
| [F0274](../../src/collectives/device/reduce_kernel.h#L205) | 函数 | `preOp` |
| [F0275](../../src/collectives/device/reduce_kernel.h#L220) | 函数 | `postOp` |
| [F0276](../../src/collectives/device/reduce_kernel.h#L253) | 函数 | `addChar4` |
| [F0277](../../src/collectives/device/reduce_kernel.h#L305) | 函数 | `FuncSum` |
| [F0278](../../src/collectives/device/reduce_kernel.h#L321) | 函数 | `operator()` |
| [F0279](../../src/collectives/device/reduce_kernel.h#L349) | 函数 | `operator()` |
| [F0280](../../src/collectives/device/reduce_kernel.h#L369) | 函数 | `FuncSum` |
| [F0281](../../src/collectives/device/reduce_kernel.h#L385) | 函数 | `operator()` |
| [F0282](../../src/collectives/device/reduce_kernel.h#L413) | 函数 | `operator()` |
| [F0283](../../src/collectives/device/reduce_kernel.h#L438) | 函数 | `mulChar4` |
| [F0284](../../src/collectives/device/reduce_kernel.h#L485) | 函数 | `FuncProd` |
| [F0285](../../src/collectives/device/reduce_kernel.h#L496) | 函数 | `operator()` |
| [F0286](../../src/collectives/device/reduce_kernel.h#L511) | 函数 | `operator()` |
| [F0287](../../src/collectives/device/reduce_kernel.h#L531) | 函数 | `FuncProd` |
| [F0288](../../src/collectives/device/reduce_kernel.h#L542) | 函数 | `operator()` |
| [F0289](../../src/collectives/device/reduce_kernel.h#L557) | 函数 | `operator()` |
| [F0290](../../src/collectives/device/reduce_kernel.h#L579) | 函数 | `FuncMax` |
| [F0291](../../src/collectives/device/reduce_kernel.h#L612) | 函数 | `operator()` |
| [F0292](../../src/collectives/device/reduce_kernel.h#L655) | 函数 | `operator()` |
| [F0293](../../src/collectives/device/reduce_kernel.h#L676) | 函数 | `FuncMax` |
| [F0294](../../src/collectives/device/reduce_kernel.h#L709) | 函数 | `operator()` |
| [F0295](../../src/collectives/device/reduce_kernel.h#L752) | 函数 | `operator()` |
| [F0296](../../src/collectives/device/reduce_kernel.h#L774) | 函数 | `FuncMin` |
| [F0297](../../src/collectives/device/reduce_kernel.h#L807) | 函数 | `operator()` |
| [F0298](../../src/collectives/device/reduce_kernel.h#L850) | 函数 | `operator()` |
| [F0299](../../src/collectives/device/reduce_kernel.h#L871) | 函数 | `FuncMin` |
| [F0300](../../src/collectives/device/reduce_kernel.h#L904) | 函数 | `operator()` |
| [F0301](../../src/collectives/device/reduce_kernel.h#L947) | 函数 | `operator()` |
| [F0302](../../src/collectives/device/reduce_kernel.h#L968) | 函数 | `FuncSum` |
| [F0303](../../src/collectives/device/reduce_kernel.h#L987) | 函数 | `operator()` |
| [F0304](../../src/collectives/device/reduce_kernel.h#L1022) | 函数 | `operator()` |
| [F0305](../../src/collectives/device/reduce_kernel.h#L1048) | 函数 | `FuncSum` |
| [F0306](../../src/collectives/device/reduce_kernel.h#L1067) | 函数 | `operator()` |
| [F0307](../../src/collectives/device/reduce_kernel.h#L1104) | 函数 | `operator()` |
| [F0308](../../src/collectives/device/reduce_kernel.h#L1131) | 函数 | `FuncProd` |
| [F0309](../../src/collectives/device/reduce_kernel.h#L1150) | 函数 | `operator()` |
| [F0310](../../src/collectives/device/reduce_kernel.h#L1185) | 函数 | `operator()` |
| [F0311](../../src/collectives/device/reduce_kernel.h#L1211) | 函数 | `FuncProd` |
| [F0312](../../src/collectives/device/reduce_kernel.h#L1230) | 函数 | `operator()` |
| [F0313](../../src/collectives/device/reduce_kernel.h#L1267) | 函数 | `operator()` |
| [F0314](../../src/collectives/device/reduce_kernel.h#L1294) | 函数 | `FuncMax` |
| [F0315](../../src/collectives/device/reduce_kernel.h#L1310) | 函数 | `operator()` |
| [F0316](../../src/collectives/device/reduce_kernel.h#L1342) | 函数 | `operator()` |
| [F0317](../../src/collectives/device/reduce_kernel.h#L1376) | 函数 | `FuncMax` |
| [F0318](../../src/collectives/device/reduce_kernel.h#L1395) | 函数 | `operator()` |
| [F0319](../../src/collectives/device/reduce_kernel.h#L1435) | 函数 | `operator()` |
| [F0320](../../src/collectives/device/reduce_kernel.h#L1470) | 函数 | `FuncMin` |
| [F0321](../../src/collectives/device/reduce_kernel.h#L1486) | 函数 | `operator()` |
| [F0322](../../src/collectives/device/reduce_kernel.h#L1518) | 函数 | `operator()` |
| [F0323](../../src/collectives/device/reduce_kernel.h#L1552) | 函数 | `FuncMin` |
| [F0324](../../src/collectives/device/reduce_kernel.h#L1571) | 函数 | `operator()` |
| [F0325](../../src/collectives/device/reduce_kernel.h#L1611) | 函数 | `operator()` |
| [F0326](../../src/collectives/device/reduce_kernel.h#L1646) | 函数 | `FuncMax` |
| [F0327](../../src/collectives/device/reduce_kernel.h#L1657) | 函数 | `operator()` |
| [F0328](../../src/collectives/device/reduce_kernel.h#L1677) | 函数 | `FuncMin` |
| [F0329](../../src/collectives/device/reduce_kernel.h#L1688) | 函数 | `operator()` |
| [F0330](../../src/collectives/device/reduce_kernel.h#L1709) | 函数 | `FuncMax` |
| [F0331](../../src/collectives/device/reduce_kernel.h#L1720) | 函数 | `operator()` |
| [F0332](../../src/collectives/device/reduce_kernel.h#L1740) | 函数 | `FuncMin` |
| [F0333](../../src/collectives/device/reduce_kernel.h#L1751) | 函数 | `operator()` |
| [F0334](../../src/collectives/device/reduce_kernel.h#L1822) | 函数 | `FuncSumPostDiv` |
| [F0335](../../src/collectives/device/reduce_kernel.h#L1834) | 函数 | `preOp` |
| [F0336](../../src/collectives/device/reduce_kernel.h#L1846) | 函数 | `postOp` |
| [F0337](../../src/collectives/device/reduce_kernel.h#L1892) | 函数 | `FuncPreMulSum` |
| [F0338](../../src/collectives/device/reduce_kernel.h#L1904) | 函数 | `preOp` |
| [F0339](../../src/collectives/device/reduce_kernel.h#L1916) | 函数 | `postOp` |
| [F0340](../../src/collectives/device/reduce_kernel.h#L1948) | 函数 | `FuncPreMulSum` |
| [F0341](../../src/collectives/device/reduce_kernel.h#L1962) | 函数 | `preOp` |
| [F0342](../../src/collectives/device/reduce_kernel.h#L1976) | 函数 | `postOp` |
| [F0343](../../src/collectives/device/reduce_kernel.h#L2010) | 函数 | `FuncPreMulSum` |
| [F0344](../../src/collectives/device/reduce_kernel.h#L2024) | 函数 | `preOp` |
| [F0345](../../src/collectives/device/reduce_kernel.h#L2038) | 函数 | `postOp` |
| [F0346](../../src/collectives/device/reduce_kernel.h#L2079) | 函数 | `FuncPreMulSum` |
| [F0347](../../src/collectives/device/reduce_kernel.h#L2094) | 函数 | `preOp` |
| [F0348](../../src/collectives/device/reduce_kernel.h#L2108) | 函数 | `preOp` |
| [F0349](../../src/collectives/device/reduce_kernel.h#L2122) | 函数 | `postOp` |
| [F0350](../../src/collectives/device/reduce_kernel.h#L2136) | 函数 | `postOp` |
| [F0351](../../src/collectives/device/reduce_kernel.h#L2155) | 函数 | `FuncPreMulSum` |
| [F0352](../../src/collectives/device/reduce_kernel.h#L2169) | 函数 | `preOp` |
| [F0353](../../src/collectives/device/reduce_kernel.h#L2190) | 函数 | `preOp` |
| [F0354](../../src/collectives/device/reduce_kernel.h#L2216) | 函数 | `postOp` |
| [F0355](../../src/collectives/device/reduce_kernel.h#L2237) | 函数 | `postOp` |
| [F0356](../../src/collectives/device/reduce_kernel.h#L2292) | 函数 | `FuncPreMulSum` |
| [F0357](../../src/collectives/device/reduce_kernel.h#L2307) | 函数 | `preOp` |
| [F0358](../../src/collectives/device/reduce_kernel.h#L2321) | 函数 | `preOp` |
| [F0359](../../src/collectives/device/reduce_kernel.h#L2335) | 函数 | `postOp` |
| [F0360](../../src/collectives/device/reduce_kernel.h#L2349) | 函数 | `postOp` |
| [F0361](../../src/collectives/device/reduce_kernel.h#L2368) | 函数 | `FuncPreMulSum` |
| [F0362](../../src/collectives/device/reduce_kernel.h#L2382) | 函数 | `preOp` |
| [F0363](../../src/collectives/device/reduce_kernel.h#L2403) | 函数 | `preOp` |
| [F0364](../../src/collectives/device/reduce_kernel.h#L2431) | 函数 | `postOp` |
| [F0365](../../src/collectives/device/reduce_kernel.h#L2452) | 函数 | `postOp` |
| [F0366](../../src/collectives/device/reduce_kernel.h#L2499) | 函数 | `preOp` |
| [F0367](../../src/collectives/device/reduce_kernel.h#L2516) | 函数 | `postOp` |
| [F0368](../../src/collectives/device/reduce_kernel.h#L2549) | 函数 | `preOp` |
| [F0369](../../src/collectives/device/reduce_kernel.h#L2566) | 函数 | `postOp` |
| [T0083](../../src/collectives/device/reduce_kernel.h#L19) | 类型 | `FuncNull` |
| [T0084](../../src/collectives/device/reduce_kernel.h#L51) | 类型 | `FuncSum` |
| [T0085](../../src/collectives/device/reduce_kernel.h#L89) | 类型 | `FuncProd` |
| [T0086](../../src/collectives/device/reduce_kernel.h#L121) | 类型 | `FuncMax` |
| [T0087](../../src/collectives/device/reduce_kernel.h#L153) | 类型 | `FuncMin` |
| [T0088](../../src/collectives/device/reduce_kernel.h#L185) | 类型 | `FuncTraits` |
| [T0089](../../src/collectives/device/reduce_kernel.h#L296) | 类型 | `FuncSum<int8_t>` |
| [T0090](../../src/collectives/device/reduce_kernel.h#L360) | 类型 | `FuncSum<uint8_t>` |
| [T0091](../../src/collectives/device/reduce_kernel.h#L448) | 类型 | `converter` |
| [T0092](../../src/collectives/device/reduce_kernel.h#L476) | 类型 | `FuncProd<int8_t>` |
| [T0093](../../src/collectives/device/reduce_kernel.h#L522) | 类型 | `FuncProd<uint8_t>` |
| [T0094](../../src/collectives/device/reduce_kernel.h#L569) | 类型 | `FuncMax<int8_t>` |
| [T0095](../../src/collectives/device/reduce_kernel.h#L585) | 类型 | `converter` |
| [T0096](../../src/collectives/device/reduce_kernel.h#L666) | 类型 | `FuncMax<uint8_t>` |
| [T0097](../../src/collectives/device/reduce_kernel.h#L682) | 类型 | `converter` |
| [T0098](../../src/collectives/device/reduce_kernel.h#L764) | 类型 | `FuncMin<int8_t>` |
| [T0099](../../src/collectives/device/reduce_kernel.h#L780) | 类型 | `converter` |
| [T0100](../../src/collectives/device/reduce_kernel.h#L861) | 类型 | `FuncMin<uint8_t>` |
| [T0101](../../src/collectives/device/reduce_kernel.h#L877) | 类型 | `converter` |
| [T0102](../../src/collectives/device/reduce_kernel.h#L959) | 类型 | `FuncSum<half>` |
| [T0103](../../src/collectives/device/reduce_kernel.h#L1039) | 类型 | `FuncSum<__nv_bfloat16>` |
| [T0104](../../src/collectives/device/reduce_kernel.h#L1122) | 类型 | `FuncProd<half>` |
| [T0105](../../src/collectives/device/reduce_kernel.h#L1202) | 类型 | `FuncProd<__nv_bfloat16>` |
| [T0106](../../src/collectives/device/reduce_kernel.h#L1285) | 类型 | `FuncMax<half>` |
| [T0107](../../src/collectives/device/reduce_kernel.h#L1367) | 类型 | `FuncMax<__nv_bfloat16>` |
| [T0108](../../src/collectives/device/reduce_kernel.h#L1461) | 类型 | `FuncMin<half>` |
| [T0109](../../src/collectives/device/reduce_kernel.h#L1543) | 类型 | `FuncMin<__nv_bfloat16>` |
| [T0110](../../src/collectives/device/reduce_kernel.h#L1637) | 类型 | `FuncMax<float>` |
| [T0111](../../src/collectives/device/reduce_kernel.h#L1668) | 类型 | `FuncMin<float>` |
| [T0112](../../src/collectives/device/reduce_kernel.h#L1700) | 类型 | `FuncMax<double>` |
| [T0113](../../src/collectives/device/reduce_kernel.h#L1731) | 类型 | `FuncMin<double>` |
| [T0114](../../src/collectives/device/reduce_kernel.h#L1763) | 类型 | `IsFloatingPoint` |
| [T0115](../../src/collectives/device/reduce_kernel.h#L1768) | 类型 | `IsFloatingPoint<half>` |
| [T0116](../../src/collectives/device/reduce_kernel.h#L1774) | 类型 | `IsFloatingPoint<__nv_bfloat16>` |
| [T0117](../../src/collectives/device/reduce_kernel.h#L1780) | 类型 | `IsFloatingPoint<float>` |
| [T0118](../../src/collectives/device/reduce_kernel.h#L1785) | 类型 | `IsFloatingPoint<double>` |
| [T0119](../../src/collectives/device/reduce_kernel.h#L1794) | 类型 | `FuncSumPostDiv<T, /*IsFloating=*/false>` |
| [T0120](../../src/collectives/device/reduce_kernel.h#L1855) | 类型 | `FuncSumPostDiv<T, /*IsFloating=*/true>` |
| [T0121](../../src/collectives/device/reduce_kernel.h#L1863) | 类型 | `FuncPreMulSum` |
| [T0122](../../src/collectives/device/reduce_kernel.h#L1925) | 类型 | `FuncPreMulSum<double>` |
| [T0123](../../src/collectives/device/reduce_kernel.h#L1987) | 类型 | `FuncPreMulSum<float>` |
| [T0124](../../src/collectives/device/reduce_kernel.h#L2049) | 类型 | `FuncPreMulSum<half>` |
| [T0125](../../src/collectives/device/reduce_kernel.h#L2262) | 类型 | `FuncPreMulSum<__nv_bfloat16>` |
| [T0126](../../src/collectives/device/reduce_kernel.h#L2479) | 类型 | `FuncTraits<FuncPreMulSum<T>>` |
| [T0127](../../src/collectives/device/reduce_kernel.h#L2529) | 类型 | `FuncTraits<FuncSumPostDiv<T>>` |

## src/collectives/device/reduce_scatter.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0370](../../src/collectives/device/reduce_scatter.h#L61) | 函数 | `runRing` |
| [F0371](../../src/collectives/device/reduce_scatter.h#L227) | 函数 | `run` |
| [F0372](../../src/collectives/device/reduce_scatter.h#L249) | 函数 | `run` |
| [F0373](../../src/collectives/device/reduce_scatter.h#L270) | 函数 | `run` |
| [T0128](../../src/collectives/device/reduce_scatter.h#L216) | 类型 | `RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>` |
| [T0129](../../src/collectives/device/reduce_scatter.h#L239) | 类型 | `RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL>` |
| [T0130](../../src/collectives/device/reduce_scatter.h#L260) | 类型 | `RunWorkElement<ncclFuncReduceScatter, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_LL128>` |

## src/collectives/device/sendrecv.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0374](../../src/collectives/device/sendrecv.h#L42) | 函数 | `runSend` |
| [F0375](../../src/collectives/device/sendrecv.h#L126) | 函数 | `runRecv` |
| [F0376](../../src/collectives/device/sendrecv.h#L203) | 函数 | `run` |
| [T0131](../../src/collectives/device/sendrecv.h#L11) | 类型 | `RunWork<ncclFuncSendRecv, T, RedOp, NCCL_ALGO_RING, NCCL_PROTO_SIMPLE>` |

## src/collectives/reduce.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0377](../../src/collectives/reduce.cc#L23) | 函数 | `ncclReduce` |

## src/collectives/reduce_scatter.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0378](../../src/collectives/reduce_scatter.cc#L23) | 函数 | `ncclReduceScatter` |

## src/collectives/sendrecv.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0379](../../src/collectives/sendrecv.cc#L35) | 函数 | `ncclSend` |
| [F0380](../../src/collectives/sendrecv.cc#L86) | 函数 | `ncclRecv` |

## src/debug.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0381](../../src/debug.cc#L177) | 函数 | `ncclDebugInit` |
| [F0382](../../src/debug.cc#L390) | 函数 | `ncclDebugLog` |
| [F0383](../../src/debug.cc#L508) | 函数 | `ncclSetThreadName` |

## src/enhcompat.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0384](../../src/enhcompat.cc#L31) | 函数 | `cudaStreamGetCaptureInfo_v2` |
| [F0385](../../src/enhcompat.cc#L51) | 函数 | `cudaUserObjectCreate` |
| [F0386](../../src/enhcompat.cc#L71) | 函数 | `cudaGraphRetainUserObject` |
| [F0387](../../src/enhcompat.cc#L91) | 函数 | `cudaStreamUpdateCaptureDependencies` |
| [F0388](../../src/enhcompat.cc#L111) | 函数 | `cudaGetDriverEntryPoint` |
| [T0132](../../src/enhcompat.cc#L9) | 类型 | `cudaError_t` |

## src/enqueue.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0389](../../src/enqueue.cc#L169) | 函数 | `ncclKernMaxLocalSize` |
| [F0390](../../src/enqueue.cc#L223) | 函数 | `ncclKernSetSharedMemoryCarveout` |
| [F0391](../../src/enqueue.cc#L277) | 函数 | `ncclLaunchCooperativeKernelMultiDevice` |
| [F0392](../../src/enqueue.cc#L354) | 函数 | `getNextOp` |
| [F0393](../../src/enqueue.cc#L447) | 函数 | `setupLaunch` |
| [F0394](../../src/enqueue.cc#L583) | 函数 | `ncclCpuBarrierIn` |
| [F0395](../../src/enqueue.cc#L637) | 函数 | `ncclCpuBarrierLast` |
| [F0396](../../src/enqueue.cc#L673) | 函数 | `ncclCpuBarrierOut` |
| [F0397](../../src/enqueue.cc#L727) | 函数 | `ncclLaunchBarrier` |
| [F0398](../../src/enqueue.cc#L804) | 函数 | `ncclLaunchKernel` |
| [F0399](../../src/enqueue.cc#L864) | 函数 | `ncclLaunchProxy` |
| [F0400](../../src/enqueue.cc#L917) | 函数 | `ncclRecordEvents` |
| [F0401](../../src/enqueue.cc#L967) | 函数 | `ncclLaunchReset` |
| [F0402](../../src/enqueue.cc#L1020) | 函数 | `getCollNetSupport` |
| [F0403](../../src/enqueue.cc#L1125) | 函数 | `getAlgoInfo` |
| [F0404](../../src/enqueue.cc#L1273) | 函数 | `getPatternInfo` |
| [F0405](../../src/enqueue.cc#L1334) | 函数 | `getLoopInfo` |
| [F0406](../../src/enqueue.cc#L1470) | 函数 | `computeColl` |
| [F0407](../../src/enqueue.cc#L1659) | 函数 | `checkSetStream` |
| [F0408](../../src/enqueue.cc#L1744) | 函数 | `ncclRegBuffAndExchange` |
| [F0409](../../src/enqueue.cc#L1872) | 函数 | `ncclSetupCollKernel` |
| [F0410](../../src/enqueue.cc#L1971) | 函数 | `findShortestChannel` |
| [F0411](../../src/enqueue.cc#L2021) | 函数 | `getNextChannel` |
| [F0412](../../src/enqueue.cc#L2118) | 函数 | `ncclSetupAsyncKernels` |
| [F0413](../../src/enqueue.cc#L2252) | 函数 | `ncclSaveAsyncColl` |
| [F0414](../../src/enqueue.cc#L2319) | 函数 | `ncclSaveP2p` |
| [F0415](../../src/enqueue.cc#L2436) | 函数 | `getSegment` |
| [F0416](../../src/enqueue.cc#L2506) | 函数 | `computeP2pWorkElem` |
| [F0417](../../src/enqueue.cc#L2594) | 函数 | `enqueueSegOp` |
| [F0418](../../src/enqueue.cc#L2753) | 函数 | `ncclEnqueueP2pKernel` |
| [F0419](../../src/enqueue.cc#L2838) | 函数 | `ncclSetupP2pKernel` |
| [F0420](../../src/enqueue.cc#L2933) | 函数 | `ncclEnqueueCollKernel` |
| [F0421](../../src/enqueue.cc#L3067) | 函数 | `ncclEnqueueHostSetup` |
| [F0422](../../src/enqueue.cc#L3169) | 函数 | `graphHelperFunc` |
| [F0423](../../src/enqueue.cc#L3285) | 函数 | `ncclGetCudaGraph` |
| [F0424](../../src/enqueue.cc#L3384) | 函数 | `ncclCudaGraphHostSetup` |
| [F0425](../../src/enqueue.cc#L3508) | 函数 | `hostToDevRedOp` |
| [F0426](../../src/enqueue.cc#L3690) | 函数 | `ncclEnqueueCheck` |
| [F0427](../../src/enqueue.cc#L3828) | 函数 | `ncclRedOpCreatePreMulSum` |
| [F0428](../../src/enqueue.cc#L3915) | 函数 | `ncclRedOpDestroy` |
| [T0133](../../src/enqueue.cc#L1679) | 类型 | `ncclBuffRegHandle` |
| [T0134](../../src/enqueue.cc#L3522) | 类型 | `<匿名union>` |

## src/graph/connect.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0429](../../src/graph/connect.cc#L74) | 函数 | `ncclTopoPreset` |
| [F0430](../../src/graph/connect.cc#L239) | 函数 | `connectRings` |
| [F0431](../../src/graph/connect.cc#L348) | 函数 | `getIndexes` |
| [F0432](../../src/graph/connect.cc#L377) | 函数 | `setTreeUp` |
| [F0433](../../src/graph/connect.cc#L410) | 函数 | `setTreeDown` |
| [F0434](../../src/graph/connect.cc#L482) | 函数 | `connectTrees` |
| [F0435](../../src/graph/connect.cc#L652) | 函数 | `connectCollNet` |
| [F0436](../../src/graph/connect.cc#L819) | 函数 | `ncclMinNchannels` |
| [F0437](../../src/graph/connect.cc#L862) | 函数 | `ncclMaxNchannels` |
| [F0438](../../src/graph/connect.cc#L899) | 函数 | `copyChannels` |
| [F0439](../../src/graph/connect.cc#L982) | 函数 | `ncclTopoPostset` |

## src/graph/paths.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0440](../../src/graph/paths.cc#L43) | 函数 | `getPath` |
| [F0441](../../src/graph/paths.cc#L160) | 函数 | `ncclTopoSetPaths` |
| [F0442](../../src/graph/paths.cc#L342) | 函数 | `printNodePaths` |
| [F0443](../../src/graph/paths.cc#L426) | 函数 | `ncclTopoPrintPaths` |
| [F0444](../../src/graph/paths.cc#L472) | 函数 | `getLocalCpu` |
| [F0445](../../src/graph/paths.cc#L542) | 函数 | `addInterStep` |
| [F0446](../../src/graph/paths.cc#L611) | 函数 | `ncclTopoRemovePathType` |
| [F0447](../../src/graph/paths.cc#L702) | 函数 | `ncclGetLevel` |
| [F0448](../../src/graph/paths.cc#L880) | 函数 | `ncclTopoCheckP2p` |
| [F0449](../../src/graph/paths.cc#L1126) | 函数 | `ncclTopoCheckGdr` |
| [F0450](../../src/graph/paths.cc#L1264) | 函数 | `ncclTopoGetIntermediateRank` |
| [F0451](../../src/graph/paths.cc#L1349) | 函数 | `ncclPxnDisable` |
| [F0452](../../src/graph/paths.cc#L1411) | 函数 | `ncclTopoGetPxnRanks` |
| [F0453](../../src/graph/paths.cc#L1563) | 函数 | `ncclTopoComputePaths` |
| [F0454](../../src/graph/paths.cc#L1805) | 函数 | `ncclTopoTrimSystem` |
| [F0455](../../src/graph/paths.cc#L1911) | 函数 | `ncclTopoFree` |
| [F0456](../../src/graph/paths.cc#L1956) | 函数 | `ncclTopoGetNchannels` |
| [F0457](../../src/graph/paths.cc#L2026) | 函数 | `nextPow2` |
| [F0458](../../src/graph/paths.cc#L2081) | 函数 | `ncclTopoComputeP2pChannels` |
| [F0459](../../src/graph/paths.cc#L2181) | 函数 | `ncclTopoGetNvbGpus` |
| [T0135](../../src/graph/paths.cc#L16) | 类型 | `ncclTopoNodeList` |

## src/graph/rings.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0460](../../src/graph/rings.cc#L37) | 函数 | `dumpLine` |
| [F0461](../../src/graph/rings.cc#L108) | 函数 | `ncclBuildRings` |

## src/graph/search.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0462](../../src/graph/search.cc#L29) | 函数 | `getMaxWidth` |
| [F0463](../../src/graph/search.cc#L80) | 函数 | `getTotalWidth` |
| [F0464](../../src/graph/search.cc#L129) | 函数 | `ncclTopoSearchInit` |
| [F0465](../../src/graph/search.cc#L178) | 函数 | `findRevLink` |
| [F0466](../../src/graph/search.cc#L264) | 函数 | `followPath` |
| [F0467](../../src/graph/search.cc#L383) | 函数 | `ncclTopoFollowPath` |
| [F0468](../../src/graph/search.cc#L475) | 函数 | `gpuPciWidth` |
| [F0469](../../src/graph/search.cc#L577) | 函数 | `cmpScore` |
| [F0470](../../src/graph/search.cc#L623) | 函数 | `cmpIntraScores` |
| [F0471](../../src/graph/search.cc#L665) | 函数 | `getGpuIndex` |
| [F0472](../../src/graph/search.cc#L702) | 函数 | `getNetIndex` |
| [F0473](../../src/graph/search.cc#L736) | 函数 | `getNetPaths` |
| [F0474](../../src/graph/search.cc#L807) | 函数 | `ncclTopoSearchNextGpuSort` |
| [F0475](../../src/graph/search.cc#L964) | 函数 | `ncclTopoReplayGetGpu` |
| [F0476](../../src/graph/search.cc#L1022) | 函数 | `ncclTopoSearchTryGpu` |
| [F0477](../../src/graph/search.cc#L1086) | 函数 | `ncclTopoCompareGraphs` |
| [F0478](../../src/graph/search.cc#L1172) | 函数 | `ncclTopoSelectNets` |
| [F0479](../../src/graph/search.cc#L1385) | 函数 | `ncclTopoSearchRecGpu` |
| [F0480](../../src/graph/search.cc#L1646) | 函数 | `ncclTopoSearchRecNet` |
| [F0481](../../src/graph/search.cc#L1855) | 函数 | `ncclTopoSearchParams` |
| [F0482](../../src/graph/search.cc#L1904) | 函数 | `ncclTopoSearchRec` |
| [F0483](../../src/graph/search.cc#L2012) | 函数 | `ncclTopoGetChannelFromXml` |
| [F0484](../../src/graph/search.cc#L2119) | 函数 | `ncclTopoGetGraphFromXmlSub` |
| [F0485](../../src/graph/search.cc#L2182) | 函数 | `ncclTopoGetGraphFromXml` |
| [F0486](../../src/graph/search.cc#L2238) | 函数 | `ncclTopoGetXmlFromChannel` |
| [F0487](../../src/graph/search.cc#L2335) | 函数 | `ncclTopoGetXmlFromGraph` |
| [F0488](../../src/graph/search.cc#L2391) | 函数 | `ncclTopoGetXmlFromGraphs` |
| [F0489](../../src/graph/search.cc#L2615) | 函数 | `ncclTopoCompute` |
| [F0490](../../src/graph/search.cc#L2897) | 函数 | `ncclTopoPrintGraph` |
| [F0491](../../src/graph/search.cc#L2965) | 函数 | `ncclTopoDumpGraphs` |
| [F0492](../../src/graph/search.cc#L3068) | 函数 | `ncclTopoGetNetDev` |
| [T0136](../../src/graph/search.cc#L519) | 类型 | `ncclGpuScore` |

## src/graph/topo.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0493](../../src/graph/topo.cc#L71) | 函数 | `pciPathToInt64` |
| [F0494](../../src/graph/topo.cc#L123) | 函数 | `findLocalCpu` |
| [F0495](../../src/graph/topo.cc#L180) | 函数 | `ncclTopoGetInterCpuWidth` |
| [F0496](../../src/graph/topo.cc#L248) | 函数 | `ncclTopoGetNode` |
| [F0497](../../src/graph/topo.cc#L309) | 函数 | `ncclTopoCreateNode` |
| [F0498](../../src/graph/topo.cc#L382) | 函数 | `ncclTopoRemoveNode` |
| [F0499](../../src/graph/topo.cc#L462) | 函数 | `ncclTopoConnectNodes` |
| [F0500](../../src/graph/topo.cc#L556) | 函数 | `ncclTopoFlattenBcmSwitches` |
| [F0501](../../src/graph/topo.cc#L687) | 函数 | `ncclTopoConnectCpus` |
| [F0502](../../src/graph/topo.cc#L763) | 函数 | `ncclTopoPrintRec` |
| [F0503](../../src/graph/topo.cc#L836) | 函数 | `ncclTopoPrint` |
| [F0504](../../src/graph/topo.cc#L886) | 函数 | `ncclTopoSort` |
| [F0505](../../src/graph/topo.cc#L946) | 函数 | `ncclTopoSortSystem` |
| [F0506](../../src/graph/topo.cc#L996) | 函数 | `ncclTopoAddNet` |
| [F0507](../../src/graph/topo.cc#L1067) | 函数 | `ncclTopoAddNic` |
| [F0508](../../src/graph/topo.cc#L1110) | 函数 | `ncclTopoAddGpu` |
| [F0509](../../src/graph/topo.cc#L1217) | 函数 | `ncclTopoAddPci` |
| [F0510](../../src/graph/topo.cc#L1405) | 函数 | `ncclTopoAddCpu` |
| [F0511](../../src/graph/topo.cc#L1548) | 函数 | `ncclTopoAddNvLinks` |
| [F0512](../../src/graph/topo.cc#L1671) | 函数 | `ncclTopoGetSystemFromXml` |
| [F0513](../../src/graph/topo.cc#L1729) | 函数 | `xmlInitAttrInt` |
| [F0514](../../src/graph/topo.cc#L1765) | 函数 | `xmlInitAttrUint64` |
| [F0515](../../src/graph/topo.cc#L1801) | 函数 | `xmlInitAttrFloat` |
| [F0516](../../src/graph/topo.cc#L1907) | 函数 | `ncclTopoGetSystem` |
| [F0517](../../src/graph/topo.cc#L2083) | 函数 | `ncclTopoGetLocalNet` |
| [F0518](../../src/graph/topo.cc#L2167) | 函数 | `ncclTopoCpuType` |
| [F0519](../../src/graph/topo.cc#L2238) | 函数 | `ncclTopoGetCpuAffinity` |
| [F0520](../../src/graph/topo.cc#L2368) | 函数 | `ncclTopoGetNetCount` |
| [F0521](../../src/graph/topo.cc#L2401) | 函数 | `ncclTopoGetCompCap` |
| [F0522](../../src/graph/topo.cc#L2446) | 函数 | `ncclTopoGetLocalRank` |
| [T0137](../../src/graph/topo.cc#L210) | 类型 | `ncclNvLinkDeviceType` |

## src/graph/topo.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0523](../../src/graph/topo.h#L563) | 函数 | `ncclTopoIdToIndex` |
| [F0524](../../src/graph/topo.h#L601) | 函数 | `ncclTopoRankToIndex` |
| [F0525](../../src/graph/topo.h#L632) | 函数 | `ncclTopoNVLinkSpeed` |
| [T0138](../../src/graph/topo.h#L232) | 类型 | `ncclTopoLink` |
| [T0139](../../src/graph/topo.h#L261) | 类型 | `ncclTopoLinkList` |
| [T0140](../../src/graph/topo.h#L307) | 类型 | `ncclTopoNode` |
| [T0141](../../src/graph/topo.h#L321) | 类型 | `<匿名union>` |
| [T0142](../../src/graph/topo.h#L331) | 类型 | `<匿名struct>` |
| [T0143](../../src/graph/topo.h#L357) | 类型 | `<匿名struct>` |
| [T0144](../../src/graph/topo.h#L395) | 类型 | `<匿名struct>` |
| [T0145](../../src/graph/topo.h#L420) | 类型 | `<匿名struct>` |
| [T0146](../../src/graph/topo.h#L451) | 类型 | `ncclTopoNodeSet` |
| [T0147](../../src/graph/topo.h#L470) | 类型 | `ncclTopoSystem` |

## src/graph/trees.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0526](../../src/graph/trees.cc#L74) | 函数 | `ncclGetBtree` |
| [F0527](../../src/graph/trees.cc#L186) | 函数 | `ncclGetDtree` |

## src/graph/tuning.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0528](../../src/graph/tuning.cc#L49) | 函数 | `getNthreads` |
| [F0529](../../src/graph/tuning.cc#L113) | 函数 | `parseList` |
| [F0530](../../src/graph/tuning.cc#L422) | 函数 | `ncclTopoTuneModel` |
| [F0531](../../src/graph/tuning.cc#L872) | 函数 | `ncclTopoGetAlgoTime` |

## src/graph/xml.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0532](../../src/graph/xml.cc#L35) | 函数 | `xmlGetChar` |
| [F0533](../../src/graph/xml.cc#L80) | 函数 | `xmlGetValue` |
| [F0534](../../src/graph/xml.cc#L158) | 函数 | `xmlGetToken` |
| [F0535](../../src/graph/xml.cc#L232) | 函数 | `xmlSkipComment` |
| [F0536](../../src/graph/xml.cc#L325) | 函数 | `xmlGetNode` |
| [F0537](../../src/graph/xml.cc#L470) | 函数 | `xmlLoadSub` |
| [F0538](../../src/graph/xml.cc#L568) | 函数 | `ncclTopoDumpXmlRec` |
| [F0539](../../src/graph/xml.cc#L626) | 函数 | `ncclTopoDumpXmlToFile` |
| [F0540](../../src/graph/xml.cc#L660) | 函数 | `ncclTopoXmlLoadNvlink` |
| [F0541](../../src/graph/xml.cc#L681) | 函数 | `ncclTopoXmlLoadGpu` |
| [F0542](../../src/graph/xml.cc#L706) | 函数 | `ncclTopoXmlLoadNet` |
| [F0543](../../src/graph/xml.cc#L727) | 函数 | `ncclTopoXmlLoadNic` |
| [F0544](../../src/graph/xml.cc#L753) | 函数 | `ncclTopoXmlLoadPci` |
| [F0545](../../src/graph/xml.cc#L780) | 函数 | `ncclTopoXmlLoadCpu` |
| [F0546](../../src/graph/xml.cc#L820) | 函数 | `ncclTopoXmlLoadSystem` |
| [F0547](../../src/graph/xml.cc#L876) | 函数 | `ncclTopoGetXmlFromFile` |
| [F0548](../../src/graph/xml.cc#L932) | 函数 | `memcpylower` |
| [F0549](../../src/graph/xml.cc#L961) | 函数 | `getPciPath` |
| [F0550](../../src/graph/xml.cc#L1008) | 函数 | `ncclTopoGetStrFromSys` |
| [F0551](../../src/graph/xml.cc#L1067) | 函数 | `ncclTopoSetAttrFromSys` |
| [F0552](../../src/graph/xml.cc#L1136) | 函数 | `ncclTopoGetXmlFromCpu` |
| [F0553](../../src/graph/xml.cc#L1320) | 函数 | `ncclTopoGetPciNode` |
| [F0554](../../src/graph/xml.cc#L1347) | 函数 | `isHex` |
| [F0555](../../src/graph/xml.cc#L1366) | 函数 | `checkBDFFormat` |
| [F0556](../../src/graph/xml.cc#L1517) | 函数 | `ncclTopoGetXmlFromSys` |
| [F0557](../../src/graph/xml.cc#L1835) | 函数 | `ncclTopoGetXmlFromGpu` |
| [F0558](../../src/graph/xml.cc#L2062) | 函数 | `ncclTopoFillGpu` |
| [F0559](../../src/graph/xml.cc#L2111) | 函数 | `ncclTopoGetSubsystem` |
| [F0560](../../src/graph/xml.cc#L2187) | 函数 | `ncclTopoFillNet` |
| [F0561](../../src/graph/xml.cc#L2284) | 函数 | `ncclTopoTrimXmlRec` |
| [F0562](../../src/graph/xml.cc#L2331) | 函数 | `ncclTopoTrimXml` |
| [F0563](../../src/graph/xml.cc#L2353) | 函数 | `ncclTopoXmlGraphLoadGpu` |
| [F0564](../../src/graph/xml.cc#L2373) | 函数 | `ncclTopoXmlGraphLoadNet` |
| [F0565](../../src/graph/xml.cc#L2394) | 函数 | `ncclTopoXmlGraphLoadChannel` |
| [F0566](../../src/graph/xml.cc#L2421) | 函数 | `ncclTopoXmlGraphLoadGraph` |
| [F0567](../../src/graph/xml.cc#L2460) | 函数 | `ncclTopoXmlGraphLoadGraphs` |
| [F0568](../../src/graph/xml.cc#L2513) | 函数 | `ncclTopoGetXmlGraphFromFile` |
| [T0148](../../src/graph/xml.cc#L404) | 类型 | `xmlHandler` |
| [T0149](../../src/graph/xml.cc#L1194) | 类型 | `<匿名union>` |
| [T0150](../../src/graph/xml.cc#L1200) | 类型 | `<匿名struct>` |
| [T0151](../../src/graph/xml.cc#L1243) | 类型 | `<匿名union>` |
| [T0152](../../src/graph/xml.cc#L1249) | 类型 | `<匿名struct>` |

## src/graph/xml.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0569](../../src/graph/xml.h#L200) | 函数 | `xmlGetAttrIndex` |
| [F0570](../../src/graph/xml.h#L238) | 函数 | `xmlGetAttr` |
| [F0571](../../src/graph/xml.h#L268) | 函数 | `xmlGetAttrStr` |
| [F0572](../../src/graph/xml.h#L294) | 函数 | `xmlGetAttrInt` |
| [F0573](../../src/graph/xml.h#L322) | 函数 | `xmlGetAttrIntDefault` |
| [F0574](../../src/graph/xml.h#L352) | 函数 | `xmlGetAttrFloat` |
| [F0575](../../src/graph/xml.h#L385) | 函数 | `xmlFindTag` |
| [F0576](../../src/graph/xml.h#L432) | 函数 | `xmlFindTagKv` |
| [F0577](../../src/graph/xml.h#L486) | 函数 | `xmlSetAttr` |
| [F0578](../../src/graph/xml.h#L528) | 函数 | `xmlSetAttrIfUnset` |
| [F0579](../../src/graph/xml.h#L568) | 函数 | `xmlSetAttrInt` |
| [F0580](../../src/graph/xml.h#L609) | 函数 | `xmlSetAttrFloat` |
| [F0581](../../src/graph/xml.h#L651) | 函数 | `xmlUnsetAttr` |
| [F0582](../../src/graph/xml.h#L692) | 函数 | `xmlGetSub` |
| [F0583](../../src/graph/xml.h#L734) | 函数 | `xmlGetSubKv` |
| [F0584](../../src/graph/xml.h#L782) | 函数 | `xmlGetSubKvInt` |
| [F0585](../../src/graph/xml.h#L823) | 函数 | `xmlAddNode` |
| [F0586](../../src/graph/xml.h#L875) | 函数 | `xmlRemoveNode` |
| [F0587](../../src/graph/xml.h#L940) | 函数 | `kvConvertToInt` |
| [F0588](../../src/graph/xml.h#L981) | 函数 | `kvConvertToStr` |
| [T0153](../../src/graph/xml.h#L68) | 类型 | `ncclXmlNode` |
| [T0154](../../src/graph/xml.h#L81) | 类型 | `<匿名struct>` |
| [T0155](../../src/graph/xml.h#L117) | 类型 | `ncclXml` |
| [T0156](../../src/graph/xml.h#L910) | 类型 | `kvDict` |

## src/group.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0589](../../src/group.cc#L44) | 函数 | `ncclAsyncMode` |
| [F0590](../../src/group.cc#L63) | 函数 | `ncclAsyncErrCheck` |
| [F0591](../../src/group.cc#L181) | 函数 | `ncclAsyncThreadMain` |
| [F0592](../../src/group.cc#L216) | 函数 | `ncclAsyncInit` |
| [F0593](../../src/group.cc#L273) | 函数 | `ncclAsyncColl` |
| [F0594](../../src/group.cc#L320) | 函数 | `ncclGroupStart` |
| [F0595](../../src/group.cc#L347) | 函数 | `scheduleSend` |
| [F0596](../../src/group.cc#L388) | 函数 | `scheduleRecv` |
| [F0597](../../src/group.cc#L432) | 函数 | `ncclAsyncThreadPreconnect` |
| [F0598](../../src/group.cc#L477) | 函数 | `getP2pChunkSize` |
| [F0599](../../src/group.cc#L810) | 函数 | `ncclGroupEnd` |
| [T0157](../../src/group.cc#L76) | 类型 | `ncclInitArgs` |
| [T0158](../../src/group.cc#L106) | 类型 | `ncclCollArgs` |
| [T0159](../../src/group.cc#L117) | 类型 | `ncclAsyncFuncType` |
| [T0160](../../src/group.cc#L138) | 类型 | `ncclAsyncArgs` |
| [T0161](../../src/group.cc#L151) | 类型 | `<匿名union>` |

## src/include/align.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0600](../../src/include/align.h#L49) | 函数 | `divUp` |
| [F0601](../../src/include/align.h#L67) | 函数 | `roundUp` |
| [F0602](../../src/include/align.h#L86) | 函数 | `alignUp` |

## src/include/alloc.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0603](../../src/include/alloc.h#L30) | 函数 | `ncclCudaHostCallocDebug` |
| [F0604](../../src/include/alloc.h#L65) | 函数 | `ncclCudaHostFree` |
| [F0605](../../src/include/alloc.h#L89) | 函数 | `ncclCallocDebug` |
| [F0606](../../src/include/alloc.h#L144) | 函数 | `ncclRealloc` |
| [F0607](../../src/include/alloc.h#L193) | 函数 | `ncclCudaCallocDebug` |
| [F0608](../../src/include/alloc.h#L231) | 函数 | `ncclCudaMemcpy` |
| [F0609](../../src/include/alloc.h#L270) | 函数 | `ncclIbMallocDebug` |

## src/include/channel.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0610](../../src/include/channel.h#L52) | 函数 | `ncclChannelComputeBase` |
| [F0611](../../src/include/channel.h#L119) | 函数 | `ncclChannelComputeFromBase` |
| [F0612](../../src/include/channel.h#L142) | 函数 | `ncclChannelCompute` |

## src/include/coll_net.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0613](../../src/include/coll_net.h#L30) | 函数 | `collNetName` |
| [F0614](../../src/include/coll_net.h#L44) | 函数 | `collNetDevices` |
| [F0615](../../src/include/coll_net.h#L58) | 函数 | `collNetGetProperties` |
| [F0616](../../src/include/coll_net.h#L73) | 函数 | `collNetListen` |
| [F0617](../../src/include/coll_net.h#L89) | 函数 | `collNetConnect` |
| [F0618](../../src/include/coll_net.h#L107) | 函数 | `collNetReduceSupport` |
| [F0619](../../src/include/coll_net.h#L123) | 函数 | `collNetRegMr` |
| [F0620](../../src/include/coll_net.h#L141) | 函数 | `collNetDeregMr` |
| [F0621](../../src/include/coll_net.h#L156) | 函数 | `collNetIallreduce` |
| [F0622](../../src/include/coll_net.h#L179) | 函数 | `collNetIflush` |
| [F0623](../../src/include/coll_net.h#L197) | 函数 | `collNetTest` |
| [F0624](../../src/include/coll_net.h#L213) | 函数 | `collNetCloseColl` |
| [F0625](../../src/include/coll_net.h#L227) | 函数 | `collNetCloseListen` |
| [F0626](../../src/include/coll_net.h#L241) | 函数 | `collNetSupport` |

## src/include/collectives.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0162](../../src/include/collectives.h#L14) | 类型 | `ncclDevRedOp_t` |
| [T0163](../../src/include/collectives.h#L47) | 类型 | `ncclDevRedOpFull` |

## src/include/comm.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0627](../../src/include/comm.h#L714) | 函数 | `ncclUserRedOpMangle` |
| [T0164](../../src/include/comm.h#L19) | 类型 | `cudaLaunchParams` |
| [T0165](../../src/include/comm.h#L88) | 类型 | `ncclSendMem` |
| [T0166](../../src/include/comm.h#L93) | 类型 | `<匿名union>` |
| [T0167](../../src/include/comm.h#L99) | 类型 | `<匿名struct>` |
| [T0168](../../src/include/comm.h#L140) | 类型 | `ncclRecvMem` |
| [T0169](../../src/include/comm.h#L145) | 类型 | `<匿名union>` |
| [T0170](../../src/include/comm.h#L151) | 类型 | `<匿名struct>` |
| [T0171](../../src/include/comm.h#L186) | 类型 | `helperThreadState` |
| [T0172](../../src/include/comm.h#L204) | 类型 | `ncclGraphHelperResources` |
| [T0173](../../src/include/comm.h#L239) | 类型 | `ncclUserRedOp` |
| [T0174](../../src/include/comm.h#L258) | 类型 | `ncclNodeRanks` |
| [T0175](../../src/include/comm.h#L278) | 类型 | `ncclComm` |
| [T0176](../../src/include/comm.h#L377) | 类型 | `<匿名enum>` |
| [T0177](../../src/include/comm.h#L603) | 类型 | `<匿名enum>` |

## src/include/core.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0628](../../src/include/core.h#L72) | 函数 | `ncclTypeSize` |

## src/include/cpuset.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0629](../../src/include/cpuset.h#L31) | 函数 | `hexToInt` |
| [F0630](../../src/include/cpuset.h#L82) | 函数 | `ncclStrToCpuset` |
| [F0631](../../src/include/cpuset.h#L157) | 函数 | `ncclCpusetToStr` |

## src/include/devcomm.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0178](../../src/include/devcomm.h#L23) | 类型 | `ncclFunc_t` |
| [T0179](../../src/include/devcomm.h#L132) | 类型 | `ncclLLFifoLine` |
| [T0180](../../src/include/devcomm.h#L142) | 类型 | `<匿名struct>` |
| [T0181](../../src/include/devcomm.h#L303) | 类型 | `ncclConnInfo` |
| [T0182](../../src/include/devcomm.h#L358) | 类型 | `ncclProxyConnector` |
| [T0183](../../src/include/devcomm.h#L386) | 类型 | `ncclConnector` |
| [T0184](../../src/include/devcomm.h#L423) | 类型 | `ncclRing` |
| [T0185](../../src/include/devcomm.h#L468) | 类型 | `ncclTree` |
| [T0186](../../src/include/devcomm.h#L492) | 类型 | `ncclDirect` |
| [T0187](../../src/include/devcomm.h#L537) | 类型 | `ncclPeer` |
| [T0188](../../src/include/devcomm.h#L563) | 类型 | `ncclWorkElemType` |
| [T0189](../../src/include/devcomm.h#L589) | 类型 | `ncclWorkElemSubType` |
| [T0190](../../src/include/devcomm.h#L617) | 类型 | `ncclWorkElemHeader` |
| [T0191](../../src/include/devcomm.h#L644) | 类型 | `ncclWorkElem` |
| [T0192](../../src/include/devcomm.h#L712) | 类型 | `ncclWorkElemP2p` |
| [T0193](../../src/include/devcomm.h#L756) | 类型 | `ncclWorkElemReg` |
| [T0194](../../src/include/devcomm.h#L807) | 类型 | `ncclWork` |
| [T0195](../../src/include/devcomm.h#L812) | 类型 | `<匿名union>` |
| [T0196](../../src/include/devcomm.h#L848) | 类型 | `ncclChannel` |
| [T0197](../../src/include/devcomm.h#L853) | 类型 | `<匿名union>` |
| [T0198](../../src/include/devcomm.h#L859) | 类型 | `<匿名struct>` |
| [T0199](../../src/include/devcomm.h#L940) | 类型 | `ncclDevComm` |
| [T0200](../../src/include/devcomm.h#L977) | 类型 | `ncclDevCommAndChannels` |

## src/include/enqueue.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0632](../../src/include/enqueue.h#L204) | 函数 | `ncclCreateQueueInfo` |
| [F0633](../../src/include/enqueue.h#L233) | 函数 | `ncclResetQueueInfo` |
| [F0634](../../src/include/enqueue.h#L305) | 函数 | `ncclDestroyQueueInfo` |
| [T0201](../../src/include/enqueue.h#L106) | 类型 | `ncclBuffRegInfo` |
| [T0202](../../src/include/enqueue.h#L140) | 类型 | `ncclQueueElem` |
| [T0203](../../src/include/enqueue.h#L168) | 类型 | `ncclQueueInfo` |

## src/include/gdrwrap.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0635](../../src/include/gdrwrap.h#L68) | 函数 | `wc_store_fence` |
| [F0636](../../src/include/gdrwrap.h#L82) | 函数 | `wc_store_fence` |
| [F0637](../../src/include/gdrwrap.h#L97) | 函数 | `wc_store_fence` |
| [F0638](../../src/include/gdrwrap.h#L111) | 函数 | `wc_store_fence` |
| [F0639](../../src/include/gdrwrap.h#L133) | 函数 | `wrap_gdr_symbols` |
| [F0640](../../src/include/gdrwrap.h#L147) | 函数 | `wrap_gdr_open` |
| [F0641](../../src/include/gdrwrap.h#L164) | 函数 | `wrap_gdr_close` |
| [F0642](../../src/include/gdrwrap.h#L178) | 函数 | `wrap_gdr_pin_buffer` |
| [F0643](../../src/include/gdrwrap.h#L200) | 函数 | `wrap_gdr_unpin_buffer` |
| [F0644](../../src/include/gdrwrap.h#L218) | 函数 | `wrap_gdr_get_info` |
| [F0645](../../src/include/gdrwrap.h#L237) | 函数 | `wrap_gdr_map` |
| [F0646](../../src/include/gdrwrap.h#L260) | 函数 | `wrap_gdr_unmap` |
| [F0647](../../src/include/gdrwrap.h#L283) | 函数 | `wrap_gdr_runtime_get_version` |
| [F0648](../../src/include/gdrwrap.h#L301) | 函数 | `wrap_gdr_driver_get_version` |
| [F0649](../../src/include/gdrwrap.h#L320) | 函数 | `wrap_gdr_copy_to_mapping` |
| [F0650](../../src/include/gdrwrap.h#L340) | 函数 | `wrap_gdr_copy_from_mapping` |
| [F0651](../../src/include/gdrwrap.h#L560) | 函数 | `ncclGdrInit` |
| [F0652](../../src/include/gdrwrap.h#L654) | 函数 | `ncclGdrCudaCalloc` |
| [F0653](../../src/include/gdrwrap.h#L755) | 函数 | `ncclGdrCudaCopy` |
| [F0654](../../src/include/gdrwrap.h#L786) | 函数 | `ncclGdrCudaFree` |
| [T0204](../../src/include/gdrwrap.h#L384) | 类型 | `gdr_mh_s` |
| [T0205](../../src/include/gdrwrap.h#L394) | 类型 | `gdr_info` |
| [T0206](../../src/include/gdrwrap.h#L509) | 类型 | `gdr_mem_desc` |

## src/include/graph.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0207](../../src/include/graph.h#L225) | 类型 | `ncclTopoGraph` |
| [T0208](../../src/include/graph.h#L317) | 类型 | `ncclTopoRanks` |

## src/include/ibvwrap.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0655](../../src/include/ibvwrap.h#L3675) | 函数 | `verbs_get_device` |
| [F0656](../../src/include/ibvwrap.h#L3825) | 函数 | `wrap_ibv_poll_cq` |
| [F0657](../../src/include/ibvwrap.h#L3871) | 函数 | `ibv_post_send` |
| [F0658](../../src/include/ibvwrap.h#L3894) | 函数 | `wrap_ibv_post_send` |
| [F0659](../../src/include/ibvwrap.h#L3927) | 函数 | `wrap_ibv_post_recv` |
| [T0209](../../src/include/ibvwrap.h#L39) | 类型 | `ibv_gid` |
| [T0210](../../src/include/ibvwrap.h#L52) | 类型 | `<匿名struct>` |
| [T0211](../../src/include/ibvwrap.h#L93) | 类型 | `ibv_node_type` |
| [T0212](../../src/include/ibvwrap.h#L139) | 类型 | `ibv_transport_type` |
| [T0213](../../src/include/ibvwrap.h#L175) | 类型 | `ibv_device_cap_flags` |
| [T0214](../../src/include/ibvwrap.h#L267) | 类型 | `ibv_atomic_cap` |
| [T0215](../../src/include/ibvwrap.h#L289) | 类型 | `ibv_device_attr` |
| [T0216](../../src/include/ibvwrap.h#L456) | 类型 | `ibv_mtu` |
| [T0217](../../src/include/ibvwrap.h#L488) | 类型 | `ibv_port_state` |
| [T0218](../../src/include/ibvwrap.h#L525) | 类型 | `<匿名enum>` |
| [T0219](../../src/include/ibvwrap.h#L561) | 类型 | `ibv_port_cap_flags` |
| [T0220](../../src/include/ibvwrap.h#L663) | 类型 | `ibv_port_attr` |
| [T0221](../../src/include/ibvwrap.h#L754) | 类型 | `ibv_event_type` |
| [T0222](../../src/include/ibvwrap.h#L875) | 类型 | `ibv_async_event` |
| [T0223](../../src/include/ibvwrap.h#L884) | 类型 | `<匿名union>` |
| [T0224](../../src/include/ibvwrap.h#L922) | 类型 | `ibv_wc_status` |
| [T0225](../../src/include/ibvwrap.h#L1043) | 类型 | `ibv_wc_opcode` |
| [T0226](../../src/include/ibvwrap.h#L1094) | 类型 | `ibv_wc_flags` |
| [T0227](../../src/include/ibvwrap.h#L1111) | 类型 | `ibv_wc` |
| [T0228](../../src/include/ibvwrap.h#L1170) | 类型 | `ibv_access_flags` |
| [T0229](../../src/include/ibvwrap.h#L1207) | 类型 | `ibv_pd` |
| [T0230](../../src/include/ibvwrap.h#L1222) | 类型 | `ibv_xrcd_init_attr_mask` |
| [T0231](../../src/include/ibvwrap.h#L1244) | 类型 | `ibv_xrcd_init_attr` |
| [T0232](../../src/include/ibvwrap.h#L1263) | 类型 | `ibv_xrcd` |
| [T0233](../../src/include/ibvwrap.h#L1274) | 类型 | `ibv_rereg_mr_flags` |
| [T0234](../../src/include/ibvwrap.h#L1301) | 类型 | `ibv_mr` |
| [T0235](../../src/include/ibvwrap.h#L1336) | 类型 | `ibv_mw_type` |
| [T0236](../../src/include/ibvwrap.h#L1353) | 类型 | `ibv_mw` |
| [T0237](../../src/include/ibvwrap.h#L1372) | 类型 | `ibv_global_route` |
| [T0238](../../src/include/ibvwrap.h#L1399) | 类型 | `ibv_grh` |
| [T0239](../../src/include/ibvwrap.h#L1430) | 类型 | `ibv_rate` |
| [T0240](../../src/include/ibvwrap.h#L1554) | 类型 | `ibv_ah_attr` |
| [T0241](../../src/include/ibvwrap.h#L1589) | 类型 | `ibv_srq_attr_mask` |
| [T0242](../../src/include/ibvwrap.h#L1606) | 类型 | `ibv_srq_attr` |
| [T0243](../../src/include/ibvwrap.h#L1625) | 类型 | `ibv_srq_init_attr` |
| [T0244](../../src/include/ibvwrap.h#L1640) | 类型 | `ibv_srq_type` |
| [T0245](../../src/include/ibvwrap.h#L1657) | 类型 | `ibv_srq_init_attr_mask` |
| [T0246](../../src/include/ibvwrap.h#L1689) | 类型 | `ibv_srq_init_attr_ex` |
| [T0247](../../src/include/ibvwrap.h#L1725) | 类型 | `ibv_qp_type` |
| [T0248](../../src/include/ibvwrap.h#L1787) | 类型 | `ibv_qp_cap` |
| [T0249](../../src/include/ibvwrap.h#L1814) | 类型 | `ibv_qp_init_attr` |
| [T0250](../../src/include/ibvwrap.h#L1854) | 类型 | `ibv_qp_init_attr_mask` |
| [T0251](../../src/include/ibvwrap.h#L1876) | 类型 | `ibv_qp_init_attr_ex` |
| [T0252](../../src/include/ibvwrap.h#L1924) | 类型 | `ibv_qp_open_attr_mask` |
| [T0253](../../src/include/ibvwrap.h#L1956) | 类型 | `ibv_qp_open_attr` |
| [T0254](../../src/include/ibvwrap.h#L1983) | 类型 | `ibv_qp_attr_mask` |
| [T0255](../../src/include/ibvwrap.h#L2095) | 类型 | `ibv_qp_state` |
| [T0256](../../src/include/ibvwrap.h#L2142) | 类型 | `ibv_mig_state` |
| [T0257](../../src/include/ibvwrap.h#L2164) | 类型 | `ibv_qp_attr` |
| [T0258](../../src/include/ibvwrap.h#L2271) | 类型 | `ibv_wr_opcode` |
| [T0259](../../src/include/ibvwrap.h#L2313) | 类型 | `ibv_send_flags` |
| [T0260](../../src/include/ibvwrap.h#L2340) | 类型 | `ibv_sge` |
| [T0261](../../src/include/ibvwrap.h#L2359) | 类型 | `ibv_send_wr` |
| [T0262](../../src/include/ibvwrap.h#L2396) | 类型 | `<匿名union>` |
| [T0263](../../src/include/ibvwrap.h#L2406) | 类型 | `<匿名struct>` |
| [T0264](../../src/include/ibvwrap.h#L2424) | 类型 | `<匿名struct>` |
| [T0265](../../src/include/ibvwrap.h#L2450) | 类型 | `<匿名struct>` |
| [T0266](../../src/include/ibvwrap.h#L2469) | 类型 | `<匿名union>` |
| [T0267](../../src/include/ibvwrap.h#L2478) | 类型 | `<匿名union>` |
| [T0268](../../src/include/ibvwrap.h#L2488) | 类型 | `<匿名struct>` |
| [T0269](../../src/include/ibvwrap.h#L2507) | 类型 | `ibv_recv_wr` |
| [T0270](../../src/include/ibvwrap.h#L2530) | 类型 | `ibv_mw_bind` |
| [T0271](../../src/include/ibvwrap.h#L2561) | 类型 | `ibv_srq` |
| [T0272](../../src/include/ibvwrap.h#L2632) | 类型 | `ibv_event_flags` |
| [T0273](../../src/include/ibvwrap.h#L2646) | 类型 | `ibv_qp` |
| [T0274](../../src/include/ibvwrap.h#L2706) | 类型 | `ibv_comp_channel` |
| [T0275](../../src/include/ibvwrap.h#L2725) | 类型 | `ibv_cq` |
| [T0276](../../src/include/ibvwrap.h#L2769) | 类型 | `ibv_ah` |
| [T0277](../../src/include/ibvwrap.h#L2788) | 类型 | `ibv_flow_flags` |
| [T0278](../../src/include/ibvwrap.h#L2805) | 类型 | `ibv_flow_attr_type` |
| [T0279](../../src/include/ibvwrap.h#L2834) | 类型 | `ibv_flow_spec_type` |
| [T0280](../../src/include/ibvwrap.h#L2861) | 类型 | `ibv_flow_eth_filter` |
| [T0281](../../src/include/ibvwrap.h#L2887) | 类型 | `ibv_flow_spec_eth` |
| [T0282](../../src/include/ibvwrap.h#L2910) | 类型 | `ibv_flow_ipv4_filter` |
| [T0283](../../src/include/ibvwrap.h#L2925) | 类型 | `ibv_flow_spec_ipv4` |
| [T0284](../../src/include/ibvwrap.h#L2948) | 类型 | `ibv_flow_tcp_udp_filter` |
| [T0285](../../src/include/ibvwrap.h#L2963) | 类型 | `ibv_flow_spec_tcp_udp` |
| [T0286](../../src/include/ibvwrap.h#L2986) | 类型 | `ibv_flow_spec` |
| [T0287](../../src/include/ibvwrap.h#L2991) | 类型 | `<匿名union>` |
| [T0288](../../src/include/ibvwrap.h#L3001) | 类型 | `<匿名struct>` |
| [T0289](../../src/include/ibvwrap.h#L3030) | 类型 | `ibv_flow_attr` |
| [T0290](../../src/include/ibvwrap.h#L3069) | 类型 | `ibv_flow` |
| [T0291](../../src/include/ibvwrap.h#L3091) | 类型 | `ibv_device_ops` |
| [T0292](../../src/include/ibvwrap.h#L3112) | 类型 | `<匿名enum>` |
| [T0293](../../src/include/ibvwrap.h#L3129) | 类型 | `ibv_device` |
| [T0294](../../src/include/ibvwrap.h#L3168) | 类型 | `verbs_device` |
| [T0295](../../src/include/ibvwrap.h#L3209) | 类型 | `ibv_context_ops` |
| [T0296](../../src/include/ibvwrap.h#L3490) | 类型 | `ibv_context` |
| [T0297](../../src/include/ibvwrap.h#L3525) | 类型 | `verbs_context_mask` |
| [T0298](../../src/include/ibvwrap.h#L3557) | 类型 | `verbs_context` |
| [T0299](../../src/include/ibvwrap.h#L3688) | 类型 | `ibv_return_enum` |

## src/include/info.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0300](../../src/include/info.h#L18) | 类型 | `ncclPattern_t` |
| [T0301](../../src/include/info.h#L79) | 类型 | `ncclInfo` |

## src/include/nccl_net.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0302](../../src/include/nccl_net.h#L41) | 类型 | `ncclDebugLogLevel` |
| [T0303](../../src/include/nccl_net.h#L63) | 类型 | `ncclDebugLogSubSys` |
| [T0304](../../src/include/nccl_net.h#L100) | 类型 | `ncclNetProperties_v5_t` |
| [T0305](../../src/include/nccl_net.h#L145) | 类型 | `ncclNet_v5_t` |
| [T0306](../../src/include/nccl_net.h#L321) | 类型 | `ncclCollNet_v5_t` |
| [T0307](../../src/include/nccl_net.h#L480) | 类型 | `ncclNetProperties_v4_t` |
| [T0308](../../src/include/nccl_net.h#L515) | 类型 | `ncclNet_v4_t` |
| [T0309](../../src/include/nccl_net.h#L673) | 类型 | `ncclCollNet_v4_t` |

## src/include/net.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0660](../../src/include/net.h#L46) | 函数 | `ncclNetName` |
| [F0661](../../src/include/net.h#L60) | 函数 | `ncclNetDevices` |
| [F0662](../../src/include/net.h#L74) | 函数 | `ncclNetGetProperties` |
| [F0663](../../src/include/net.h#L89) | 函数 | `ncclNetListen` |
| [F0664](../../src/include/net.h#L105) | 函数 | `ncclNetConnect` |
| [F0665](../../src/include/net.h#L121) | 函数 | `ncclNetAccept` |
| [F0666](../../src/include/net.h#L136) | 函数 | `ncclNetRegMr` |
| [F0667](../../src/include/net.h#L154) | 函数 | `ncclNetDeregMr` |
| [F0668](../../src/include/net.h#L169) | 函数 | `ncclNetIsend` |
| [F0669](../../src/include/net.h#L188) | 函数 | `ncclNetIrecv` |
| [F0670](../../src/include/net.h#L208) | 函数 | `ncclNetIflush` |
| [F0671](../../src/include/net.h#L227) | 函数 | `ncclNetTest` |
| [F0672](../../src/include/net.h#L243) | 函数 | `ncclNetCloseSend` |
| [F0673](../../src/include/net.h#L257) | 函数 | `ncclNetCloseRecv` |
| [F0674](../../src/include/net.h#L271) | 函数 | `ncclNetCloseListen` |

## src/include/nvmlwrap.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0310](../../src/include/nvmlwrap.h#L38) | 类型 | `nvmlEnableState_enum` |
| [T0311](../../src/include/nvmlwrap.h#L55) | 类型 | `nvmlNvLinkCapability_enum` |
| [T0312](../../src/include/nvmlwrap.h#L98) | 类型 | `nvmlReturn_enum` |
| [T0313](../../src/include/nvmlwrap.h#L210) | 类型 | `nvmlPciInfo_st` |
| [T0314](../../src/include/nvmlwrap.h#L262) | 类型 | `nvmlGpuP2PStatus_enum` |
| [T0315](../../src/include/nvmlwrap.h#L305) | 类型 | `nvmlGpuP2PCapsIndex_enum` |
| [T0316](../../src/include/nvmlwrap.h#L350) | 类型 | `ncclNvmlDeviceInfo` |
| [T0317](../../src/include/nvmlwrap.h#L365) | 类型 | `ncclNvmlDevicePairInfo` |

## src/include/nvtx.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0318](../../src/include/nvtx.h#L16) | 类型 | `nccl_domain` |

## src/include/nvtx3/nvToolsExt.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0319](../../src/include/nvtx3/nvToolsExt.h#L345) | 类型 | `nvtxColorType_t` |
| [T0320](../../src/include/nvtx3/nvToolsExt.h#L365) | 类型 | `nvtxMessageType_t` |
| [T0321](../../src/include/nvtx3/nvToolsExt.h#L395) | 类型 | `nvtxMessageValue_t` |
| [T0322](../../src/include/nvtx3/nvToolsExt.h#L457) | 类型 | `nvtxPayloadType_t` |
| [T0323](../../src/include/nvtx3/nvToolsExt.h#L577) | 类型 | `nvtxEventAttributes_v2` |
| [T0324](../../src/include/nvtx3/nvToolsExt.h#L670) | 类型 | `payload_t` |
| [T0325](../../src/include/nvtx3/nvToolsExt.h#L1214) | 类型 | `nvtxResourceGenericType_t` |
| [T0326](../../src/include/nvtx3/nvToolsExt.h#L1323) | 类型 | `nvtxResourceAttributes_v0` |
| [T0327](../../src/include/nvtx3/nvToolsExt.h#L1375) | 类型 | `identifier_t` |

## src/include/nvtx3/nvToolsExtCuda.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0328](../../src/include/nvtx3/nvToolsExtCuda.h#L51) | 类型 | `nvtxResourceCUDAType_t` |

## src/include/nvtx3/nvToolsExtCudaRt.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0329](../../src/include/nvtx3/nvToolsExtCudaRt.h#L52) | 类型 | `nvtxResourceCUDARTType_t` |

## src/include/nvtx3/nvToolsExtOpenCL.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0330](../../src/include/nvtx3/nvToolsExtOpenCL.h#L51) | 类型 | `nvtxResourceOpenCLType_t` |

## src/include/nvtx3/nvToolsExtSync.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0331](../../src/include/nvtx3/nvToolsExtSync.h#L117) | 类型 | `nvtxResourceSyncPosixThreadType_t` |
| [T0332](../../src/include/nvtx3/nvToolsExtSync.h#L156) | 类型 | `nvtxResourceSyncWindowsType_t` |
| [T0333](../../src/include/nvtx3/nvToolsExtSync.h#L192) | 类型 | `nvtxResourceSyncLinuxType_t` |
| [T0334](../../src/include/nvtx3/nvToolsExtSync.h#L320) | 类型 | `nvtxSyncUserAttributes_v0` |

## src/include/nvtx3/nvtxDetail/nvtxImpl.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0675](../../src/include/nvtx3/nvtxDetail/nvtxImpl.h#L733) | 函数 | `nvtxEtiGetModuleFunctionTable` |
| [F0676](../../src/include/nvtx3/nvtxDetail/nvtxImpl.h#L807) | 函数 | `nvtxGetExportTable` |
| [F0677](../../src/include/nvtx3/nvtxDetail/nvtxImpl.h#L829) | 函数 | `nvtxEtiSetInjectionNvtxVersion` |
| [T0335](../../src/include/nvtx3/nvtxDetail/nvtxImpl.h#L132) | 类型 | `nvtxGlobals_t` |

## src/include/nvtx3/nvtxDetail/nvtxImplCore.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0678](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L18) | 函数 | `nvtxMarkEx` |
| [F0679](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L46) | 函数 | `nvtxMarkA` |
| [F0680](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L74) | 函数 | `nvtxMarkW` |
| [F0681](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L103) | 函数 | `nvtxRangeStartEx` |
| [F0682](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L135) | 函数 | `nvtxRangeStartA` |
| [F0683](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L167) | 函数 | `nvtxRangeStartW` |
| [F0684](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L198) | 函数 | `nvtxRangeEnd` |
| [F0685](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L227) | 函数 | `nvtxRangePushEx` |
| [F0686](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L259) | 函数 | `nvtxRangePushA` |
| [F0687](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L291) | 函数 | `nvtxRangePushW` |
| [F0688](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L323) | 函数 | `nvtxRangePop` |
| [F0689](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L354) | 函数 | `nvtxNameCategoryA` |
| [F0690](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L383) | 函数 | `nvtxNameCategoryW` |
| [F0691](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L412) | 函数 | `nvtxNameOsThreadA` |
| [F0692](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L441) | 函数 | `nvtxNameOsThreadW` |
| [F0693](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L470) | 函数 | `nvtxDomainMarkEx` |
| [F0694](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L500) | 函数 | `nvtxDomainRangeStartEx` |
| [F0695](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L533) | 函数 | `nvtxDomainRangeEnd` |
| [F0696](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L563) | 函数 | `nvtxDomainRangePushEx` |
| [F0697](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L597) | 函数 | `nvtxDomainRangePop` |
| [F0698](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L629) | 函数 | `nvtxDomainResourceCreate` |
| [F0699](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L662) | 函数 | `nvtxDomainResourceDestroy` |
| [F0700](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L691) | 函数 | `nvtxDomainNameCategoryA` |
| [F0701](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L722) | 函数 | `nvtxDomainNameCategoryW` |
| [F0702](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L754) | 函数 | `nvtxDomainRegisterStringA` |
| [F0703](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L788) | 函数 | `nvtxDomainRegisterStringW` |
| [F0704](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L822) | 函数 | `nvtxDomainCreateA` |
| [F0705](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L854) | 函数 | `nvtxDomainCreateW` |
| [F0706](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L885) | 函数 | `nvtxDomainDestroy` |
| [F0707](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h#L913) | 函数 | `nvtxInitialize` |

## src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0708](../../src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h#L33) | 函数 | `nvtxNameCudaDeviceA` |
| [F0709](../../src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h#L63) | 函数 | `nvtxNameCudaDeviceW` |
| [F0710](../../src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h#L93) | 函数 | `nvtxNameCudaStreamA` |
| [F0711](../../src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h#L123) | 函数 | `nvtxNameCudaStreamW` |
| [F0712](../../src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h#L153) | 函数 | `nvtxNameCudaEventA` |
| [F0713](../../src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h#L183) | 函数 | `nvtxNameCudaEventW` |

## src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0714](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h#L36) | 函数 | `nvtxNameCuDeviceA` |
| [F0715](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h#L66) | 函数 | `nvtxNameCuDeviceW` |
| [F0716](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h#L96) | 函数 | `nvtxNameCuContextA` |
| [F0717](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h#L126) | 函数 | `nvtxNameCuContextW` |
| [F0718](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h#L156) | 函数 | `nvtxNameCuStreamA` |
| [F0719](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h#L186) | 函数 | `nvtxNameCuStreamW` |
| [F0720](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h#L216) | 函数 | `nvtxNameCuEventA` |
| [F0721](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h#L246) | 函数 | `nvtxNameCuEventW` |

## src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0722](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L42) | 函数 | `nvtxNameClDeviceA` |
| [F0723](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L72) | 函数 | `nvtxNameClDeviceW` |
| [F0724](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L102) | 函数 | `nvtxNameClContextA` |
| [F0725](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L132) | 函数 | `nvtxNameClContextW` |
| [F0726](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L162) | 函数 | `nvtxNameClCommandQueueA` |
| [F0727](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L192) | 函数 | `nvtxNameClCommandQueueW` |
| [F0728](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L222) | 函数 | `nvtxNameClMemObjectA` |
| [F0729](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L252) | 函数 | `nvtxNameClMemObjectW` |
| [F0730](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L282) | 函数 | `nvtxNameClSamplerA` |
| [F0731](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L312) | 函数 | `nvtxNameClSamplerW` |
| [F0732](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L342) | 函数 | `nvtxNameClProgramA` |
| [F0733](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L372) | 函数 | `nvtxNameClProgramW` |
| [F0734](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L402) | 函数 | `nvtxNameClEventA` |
| [F0735](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h#L432) | 函数 | `nvtxNameClEventW` |

## src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0736](../../src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h#L35) | 函数 | `nvtxDomainSyncUserCreate` |
| [F0737](../../src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h#L68) | 函数 | `nvtxDomainSyncUserDestroy` |
| [F0738](../../src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h#L97) | 函数 | `nvtxDomainSyncUserAcquireStart` |
| [F0739](../../src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h#L127) | 函数 | `nvtxDomainSyncUserAcquireFailed` |
| [F0740](../../src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h#L157) | 函数 | `nvtxDomainSyncUserAcquireSuccess` |
| [F0741](../../src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h#L187) | 函数 | `nvtxDomainSyncUserReleasing` |

## src/include/nvtx3/nvtxDetail/nvtxInit.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0742](../../src/include/nvtx3/nvtxDetail/nvtxInit.h#L325) | 函数 | `nvtxInitializeInjectionLibrary` |
| [F0743](../../src/include/nvtx3/nvtxDetail/nvtxInit.h#L605) | 函数 | `nvtxInitOnce` |

## src/include/nvtx3/nvtxDetail/nvtxInitDefs.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0744](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L19) | 函数 | `nvtxMarkEx_impl_init` |
| [F0745](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L35) | 函数 | `nvtxMarkA_impl_init` |
| [F0746](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L51) | 函数 | `nvtxMarkW_impl_init` |
| [F0747](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L67) | 函数 | `nvtxRangeStartEx_impl_init` |
| [F0748](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L85) | 函数 | `nvtxRangeStartA_impl_init` |
| [F0749](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L103) | 函数 | `nvtxRangeStartW_impl_init` |
| [F0750](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L121) | 函数 | `nvtxRangeEnd_impl_init` |
| [F0751](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L137) | 函数 | `nvtxRangePushEx_impl_init` |
| [F0752](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L154) | 函数 | `nvtxRangePushA_impl_init` |
| [F0753](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L171) | 函数 | `nvtxRangePushW_impl_init` |
| [F0754](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L188) | 函数 | `nvtxRangePop_impl_init` |
| [F0755](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L205) | 函数 | `nvtxNameCategoryA_impl_init` |
| [F0756](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L223) | 函数 | `nvtxNameCategoryW_impl_init` |
| [F0757](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L241) | 函数 | `nvtxNameOsThreadA_impl_init` |
| [F0758](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L259) | 函数 | `nvtxNameOsThreadW_impl_init` |
| [F0759](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L277) | 函数 | `nvtxDomainMarkEx_impl_init` |
| [F0760](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L294) | 函数 | `nvtxDomainRangeStartEx_impl_init` |
| [F0761](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L313) | 函数 | `nvtxDomainRangeEnd_impl_init` |
| [F0762](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L330) | 函数 | `nvtxDomainRangePushEx_impl_init` |
| [F0763](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L348) | 函数 | `nvtxDomainRangePop_impl_init` |
| [F0764](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L365) | 函数 | `nvtxDomainResourceCreate_impl_init` |
| [F0765](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L383) | 函数 | `nvtxDomainResourceDestroy_impl_init` |
| [F0766](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L399) | 函数 | `nvtxDomainNameCategoryA_impl_init` |
| [F0767](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L418) | 函数 | `nvtxDomainNameCategoryW_impl_init` |
| [F0768](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L437) | 函数 | `nvtxDomainRegisterStringA_impl_init` |
| [F0769](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L455) | 函数 | `nvtxDomainRegisterStringW_impl_init` |
| [F0770](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L473) | 函数 | `nvtxDomainCreateA_impl_init` |
| [F0771](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L490) | 函数 | `nvtxDomainCreateW_impl_init` |
| [F0772](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L507) | 函数 | `nvtxDomainDestroy_impl_init` |
| [F0773](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L523) | 函数 | `nvtxInitialize_impl_init` |
| [F0774](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L543) | 函数 | `nvtxNameCuDeviceA_impl_init` |
| [F0775](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L574) | 函数 | `nvtxNameCuDeviceW_impl_init` |
| [F0776](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L605) | 函数 | `nvtxNameCuContextA_impl_init` |
| [F0777](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L636) | 函数 | `nvtxNameCuContextW_impl_init` |
| [F0778](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L667) | 函数 | `nvtxNameCuStreamA_impl_init` |
| [F0779](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L698) | 函数 | `nvtxNameCuStreamW_impl_init` |
| [F0780](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L729) | 函数 | `nvtxNameCuEventA_impl_init` |
| [F0781](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L760) | 函数 | `nvtxNameCuEventW_impl_init` |
| [F0782](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L791) | 函数 | `nvtxNameCudaDeviceA_impl_init` |
| [F0783](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L822) | 函数 | `nvtxNameCudaDeviceW_impl_init` |
| [F0784](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L853) | 函数 | `nvtxNameCudaStreamA_impl_init` |
| [F0785](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L884) | 函数 | `nvtxNameCudaStreamW_impl_init` |
| [F0786](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L915) | 函数 | `nvtxNameCudaEventA_impl_init` |
| [F0787](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L946) | 函数 | `nvtxNameCudaEventW_impl_init` |
| [F0788](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L977) | 函数 | `nvtxNameClDeviceA_impl_init` |
| [F0789](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1008) | 函数 | `nvtxNameClDeviceW_impl_init` |
| [F0790](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1039) | 函数 | `nvtxNameClContextA_impl_init` |
| [F0791](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1070) | 函数 | `nvtxNameClContextW_impl_init` |
| [F0792](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1101) | 函数 | `nvtxNameClCommandQueueA_impl_init` |
| [F0793](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1133) | 函数 | `nvtxNameClCommandQueueW_impl_init` |
| [F0794](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1165) | 函数 | `nvtxNameClMemObjectA_impl_init` |
| [F0795](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1197) | 函数 | `nvtxNameClMemObjectW_impl_init` |
| [F0796](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1229) | 函数 | `nvtxNameClSamplerA_impl_init` |
| [F0797](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1260) | 函数 | `nvtxNameClSamplerW_impl_init` |
| [F0798](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1291) | 函数 | `nvtxNameClProgramA_impl_init` |
| [F0799](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1322) | 函数 | `nvtxNameClProgramW_impl_init` |
| [F0800](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1353) | 函数 | `nvtxNameClEventA_impl_init` |
| [F0801](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1384) | 函数 | `nvtxNameClEventW_impl_init` |
| [F0802](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1416) | 函数 | `nvtxDomainSyncUserCreate_impl_init` |
| [F0803](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1450) | 函数 | `nvtxDomainSyncUserDestroy_impl_init` |
| [F0804](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1480) | 函数 | `nvtxDomainSyncUserAcquireStart_impl_init` |
| [F0805](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1510) | 函数 | `nvtxDomainSyncUserAcquireFailed_impl_init` |
| [F0806](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1540) | 函数 | `nvtxDomainSyncUserAcquireSuccess_impl_init` |
| [F0807](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1570) | 函数 | `nvtxDomainSyncUserReleasing_impl_init` |
| [F0808](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h#L1787) | 函数 | `nvtxSetInitFunctionsToNoops` |

## src/include/nvtx3/nvtxDetail/nvtxTypes.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0336](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L127) | 类型 | `NvtxCallbackModule` |
| [T0337](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L180) | 类型 | `NvtxCallbackIdCore` |
| [T0338](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L278) | 类型 | `NvtxCallbackIdCore2` |
| [T0339](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L376) | 类型 | `NvtxCallbackIdCuda` |
| [T0340](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L439) | 类型 | `NvtxCallbackIdCudaRt` |
| [T0341](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L492) | 类型 | `NvtxCallbackIdOpenCL` |
| [T0342](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L585) | 类型 | `NvtxCallbackIdSync` |
| [T0343](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L639) | 类型 | `NvtxExportTableID` |
| [T0344](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L680) | 类型 | `NvtxExportTableCallbacks` |
| [T0345](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h#L697) | 类型 | `NvtxExportTableVersionInfo` |

## src/include/nvtx3.hpp

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0809](../../src/include/nvtx3.hpp#L677) | 函数 | `has_name_member` |
| [F0810](../../src/include/nvtx3.hpp#L809) | 函数 | `get` |
| [F0811](../../src/include/nvtx3.hpp#L868) | 函数 | `domain` |
| [F0812](../../src/include/nvtx3.hpp#L888) | 函数 | `domain` |
| [F0813](../../src/include/nvtx3.hpp#L908) | 函数 | `domain` |
| [F0814](../../src/include/nvtx3.hpp#L928) | 函数 | `domain` |
| [F0815](../../src/include/nvtx3.hpp#L988) | 函数 | `domain::get<domain::global>` |
| [F0816](../../src/include/nvtx3.hpp#L1033) | 函数 | `rgb` |
| [F0817](../../src/include/nvtx3.hpp#L1086) | 函数 | `argb` |
| [F0818](../../src/include/nvtx3.hpp#L1148) | 函数 | `color` |
| [F0819](../../src/include/nvtx3.hpp#L1166) | 函数 | `color` |
| [F0820](../../src/include/nvtx3.hpp#L1189) | 函数 | `color` |
| [F0821](../../src/include/nvtx3.hpp#L1209) | 函数 | `get_value` |
| [F0822](../../src/include/nvtx3.hpp#L1227) | 函数 | `get_type` |
| [F0823](../../src/include/nvtx3.hpp#L1254) | 函数 | `from_bytes_msb_to_lsb` |
| [F0824](../../src/include/nvtx3.hpp#L1326) | 函数 | `category` |
| [F0825](../../src/include/nvtx3.hpp#L1343) | 函数 | `get_id` |
| [F0826](../../src/include/nvtx3.hpp#L1462) | 函数 | `get` |
| [F0827](../../src/include/nvtx3.hpp#L1500) | 函数 | `named_category` |
| [F0828](../../src/include/nvtx3.hpp#L1536) | 函数 | `named_category` |
| [F0829](../../src/include/nvtx3.hpp#L1646) | 函数 | `get` |
| [F0830](../../src/include/nvtx3.hpp#L1679) | 函数 | `registered_string` |
| [F0831](../../src/include/nvtx3.hpp#L1705) | 函数 | `registered_string` |
| [F0832](../../src/include/nvtx3.hpp#L1728) | 函数 | `registered_string` |
| [F0833](../../src/include/nvtx3.hpp#L1754) | 函数 | `registered_string` |
| [F0834](../../src/include/nvtx3.hpp#L1771) | 函数 | `get_handle` |
| [F0835](../../src/include/nvtx3.hpp#L1837) | 函数 | `get_value` |
| [F0836](../../src/include/nvtx3.hpp#L1862) | 函数 | `message` |
| [F0837](../../src/include/nvtx3.hpp#L1882) | 函数 | `message` |
| [F0838](../../src/include/nvtx3.hpp#L1910) | 函数 | `message` |
| [F0839](../../src/include/nvtx3.hpp#L1930) | 函数 | `message` |
| [F0840](../../src/include/nvtx3.hpp#L1982) | 函数 | `nvtxMessageType_t get_type` |
| [F0841](../../src/include/nvtx3.hpp#L2042) | 函数 | `payload` |
| [F0842](../../src/include/nvtx3.hpp#L2066) | 函数 | `payload` |
| [F0843](../../src/include/nvtx3.hpp#L2090) | 函数 | `payload` |
| [F0844](../../src/include/nvtx3.hpp#L2114) | 函数 | `payload` |
| [F0845](../../src/include/nvtx3.hpp#L2139) | 函数 | `payload` |
| [F0846](../../src/include/nvtx3.hpp#L2164) | 函数 | `payload` |
| [F0847](../../src/include/nvtx3.hpp#L2184) | 函数 | `get_value` |
| [F0848](../../src/include/nvtx3.hpp#L2205) | 函数 | `get_type` |
| [F0849](../../src/include/nvtx3.hpp#L2301) | 函数 | `event_attributes` |
| [F0850](../../src/include/nvtx3.hpp#L2335) | 函数 | `event_attributes` |
| [F0851](../../src/include/nvtx3.hpp#L2361) | 函数 | `event_attributes` |
| [F0852](../../src/include/nvtx3.hpp#L2388) | 函数 | `event_attributes` |
| [F0853](../../src/include/nvtx3.hpp#L2415) | 函数 | `event_attributes` |
| [F0854](../../src/include/nvtx3.hpp#L2444) | 函数 | `get` |
| [F0855](../../src/include/nvtx3.hpp#L2536) | 函数 | `domain_thread_range` |
| [F0856](../../src/include/nvtx3.hpp#L2582) | 函数 | `domain_thread_range` |
| [F0857](../../src/include/nvtx3.hpp#L2606) | 函数 | `domain_thread_range` |
| [F0858](../../src/include/nvtx3.hpp#L2636) | 函数 | `~domain_thread_range` |
| [F0859](../../src/include/nvtx3.hpp#L2676) | 函数 | `range_handle` |
| [F0860](../../src/include/nvtx3.hpp#L2694) | 函数 | `get_value` |
| [F0861](../../src/include/nvtx3.hpp#L2744) | 函数 | `start_range` |
| [F0862](../../src/include/nvtx3.hpp#L2800) | 函数 | `start_range` |
| [F0863](../../src/include/nvtx3.hpp#L2841) | 函数 | `end_range` |
| [F0864](../../src/include/nvtx3.hpp#L2891) | 函数 | `domain_process_range` |
| [F0865](../../src/include/nvtx3.hpp#L2912) | 函数 | `domain_process_range` |
| [F0866](../../src/include/nvtx3.hpp#L2935) | 函数 | `domain_process_range` |
| [F0867](../../src/include/nvtx3.hpp#L2954) | 函数 | `~domain_process_range` |
| [F0868](../../src/include/nvtx3.hpp#L3034) | 函数 | `mark` |
| [T0346](../../src/include/nvtx3.hpp#L747) | 类型 | `domain` |
| [T0347](../../src/include/nvtx3.hpp#L849) | 类型 | `global` |
| [T0348](../../src/include/nvtx3.hpp#L1011) | 类型 | `rgb` |
| [T0349](../../src/include/nvtx3.hpp#L1065) | 类型 | `argb` |
| [T0350](../../src/include/nvtx3.hpp#L1118) | 类型 | `color` |
| [T0351](../../src/include/nvtx3.hpp#L1304) | 类型 | `category` |
| [T0352](../../src/include/nvtx3.hpp#L1415) | 类型 | `named_category` |
| [T0353](../../src/include/nvtx3.hpp#L1600) | 类型 | `registered_string` |
| [T0354](../../src/include/nvtx3.hpp#L1844) | 类型 | `message` |
| [T0355](../../src/include/nvtx3.hpp#L2021) | 类型 | `payload` |
| [T0356](../../src/include/nvtx3.hpp#L2285) | 类型 | `event_attributes` |
| [T0357](../../src/include/nvtx3.hpp#L2507) | 类型 | `domain_thread_range` |
| [T0358](../../src/include/nvtx3.hpp#L2660) | 类型 | `range_handle` |
| [T0359](../../src/include/nvtx3.hpp#L2875) | 类型 | `domain_process_range` |

## src/include/p2p.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0869](../../src/include/p2p.h#L45) | 函数 | `ncclSaveP2pInfo` |
| [T0360](../../src/include/p2p.h#L16) | 类型 | `ncclP2Pinfo` |

## src/include/profiler.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0361](../../src/include/profiler.h#L16) | 类型 | `ncclProxyProfileState` |

## src/include/proxy.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0362](../../src/include/proxy.h#L19) | 类型 | `ncclProxyOpState` |
| [T0363](../../src/include/proxy.h#L48) | 类型 | `ncclProxyOp` |
| [T0364](../../src/include/proxy.h#L121) | 类型 | `ncclProxySubArgs` |
| [T0365](../../src/include/proxy.h#L195) | 类型 | `ncclProxyArgs` |
| [T0366](../../src/include/proxy.h#L305) | 类型 | `ncclProxyOpsPool` |
| [T0367](../../src/include/proxy.h#L336) | 类型 | `ncclProxyOps` |
| [T0368](../../src/include/proxy.h#L363) | 类型 | `ncclProxySharedP2p` |
| [T0369](../../src/include/proxy.h#L394) | 类型 | `ncclProxySharedCollNet` |
| [T0370](../../src/include/proxy.h#L421) | 类型 | `ncclProxyPeer` |
| [T0371](../../src/include/proxy.h#L440) | 类型 | `ncclSharedNetComms` |
| [T0372](../../src/include/proxy.h#L464) | 类型 | `ncclProxyProgressState` |
| [T0373](../../src/include/proxy.h#L521) | 类型 | `ncclProxyState` |
| [T0374](../../src/include/proxy.h#L565) | 类型 | `ncclProxyConnection` |
| [T0375](../../src/include/proxy.h#L604) | 类型 | `proxyMode` |
| [T0376](../../src/include/proxy.h#L665) | 类型 | `ncclProxyMsgType` |

## src/include/socket.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0377](../../src/include/socket.h#L54) | 类型 | `ncclSocketAddress` |
| [T0378](../../src/include/socket.h#L74) | 类型 | `ncclSocketState` |
| [T0379](../../src/include/socket.h#L101) | 类型 | `ncclSocket` |

## src/include/timer.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0870](../../src/include/timer.h#L38) | 函数 | `calibrate` |
| [F0871](../../src/include/timer.h#L85) | 函数 | `gettime` |

## src/include/transport.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0380](../../src/include/transport.h#L63) | 类型 | `ncclPeerInfo` |
| [T0381](../../src/include/transport.h#L115) | 类型 | `ncclConnect` |
| [T0382](../../src/include/transport.h#L136) | 类型 | `ncclTransportComm` |
| [T0383](../../src/include/transport.h#L229) | 类型 | `ncclTransport` |
| [T0384](../../src/include/transport.h#L272) | 类型 | `<匿名enum>` |

## src/include/utils.h

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0872](../../src/include/utils.h#L99) | 函数 | `log2i` |
| [F0873](../../src/include/utils.h#L171) | 函数 | `ncclRecyclableList` |
| [F0874](../../src/include/utils.h#L187) | 函数 | `count` |
| [F0875](../../src/include/utils.h#L215) | 函数 | `getNewElem` |
| [F0876](../../src/include/utils.h#L249) | 函数 | `begin` |
| [F0877](../../src/include/utils.h#L274) | 函数 | `getNext` |
| [F0878](../../src/include/utils.h#L303) | 函数 | `peakNext` |
| [F0879](../../src/include/utils.h#L323) | 函数 | `recycle` |
| [F0880](../../src/include/utils.h#L343) | 函数 | `~ncclRecyclableList` |
| [T0385](../../src/include/utils.h#L63) | 类型 | `netIf` |
| [T0386](../../src/include/utils.h#L118) | 类型 | `ncclListElem` |
| [T0387](../../src/include/utils.h#L134) | 类型 | `ncclRecyclableList` |

## src/init.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0881](../../src/init.cc#L114) | 函数 | `initGdrCopy` |
| [F0882](../../src/init.cc#L174) | 函数 | `ncclInit` |
| [F0883](../../src/init.cc#L214) | 函数 | `ncclGetVersion` |
| [F0884](../../src/init.cc#L236) | 函数 | `ncclGetUniqueId` |
| [F0885](../../src/init.cc#L265) | 函数 | `commPoison` |
| [F0886](../../src/init.cc#L474) | 函数 | `commAlloc` |
| [F0887](../../src/init.cc#L618) | 函数 | `devCommSetup` |
| [F0888](../../src/init.cc#L678) | 函数 | `showVersion` |
| [F0889](../../src/init.cc#L716) | 函数 | `fillInfo` |
| [F0890](../../src/init.cc#L777) | 函数 | `setupChannel` |
| [F0891](../../src/init.cc#L834) | 函数 | `waitForNonNullPtr` |
| [F0892](../../src/init.cc#L865) | 函数 | `initParams` |
| [F0893](../../src/init.cc#L942) | 函数 | `ncclCommSetIntraProc` |
| [F0894](../../src/init.cc#L1089) | 函数 | `computeBuffSizes` |
| [F0895](../../src/init.cc#L1525) | 函数 | `initTransportsRank` |
| [F0896](../../src/init.cc#L2378) | 函数 | `ncclCommInitRankSync` |
| [F0897](../../src/init.cc#L2455) | 函数 | `ncclCommInitRankDev` |
| [F0898](../../src/init.cc#L2518) | 函数 | `ncclCommInitRank` |
| [F0899](../../src/init.cc#L2565) | 函数 | `ncclCommInitAll` |
| [F0900](../../src/init.cc#L2619) | 函数 | `ncclGraphHelperDestroy` |
| [F0901](../../src/init.cc#L2677) | 函数 | `commDestroy` |
| [F0902](../../src/init.cc#L2742) | 函数 | `ncclCommDestroy` |
| [F0903](../../src/init.cc#L2798) | 函数 | `ncclCommAbort` |
| [F0904](../../src/init.cc#L2855) | 函数 | `ncclGetErrorString` |
| [F0905](../../src/init.cc#L2884) | 函数 | `ncclCommGetAsyncError` |
| [F0906](../../src/init.cc#L2909) | 函数 | `ncclCommCount` |
| [F0907](../../src/init.cc#L2935) | 函数 | `ncclCommCuDevice` |
| [F0908](../../src/init.cc#L2961) | 函数 | `ncclCommUserRank` |
| [T0388](../../src/init.cc#L1690) | 类型 | `ncclGraphInfo` |
| [T0389](../../src/init.cc#L1730) | 类型 | `<匿名struct>` |

## src/misc/argcheck.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0909](../../src/misc/argcheck.cc#L19) | 函数 | `CudaPtrCheck` |
| [F0910](../../src/misc/argcheck.cc#L66) | 函数 | `PtrCheck` |
| [F0911](../../src/misc/argcheck.cc#L133) | 函数 | `ArgsCheck` |

## src/misc/gdrwrap.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0912](../../src/misc/gdrwrap.cc#L262) | 函数 | `wrap_gdr_symbols` |
| [F0913](../../src/misc/gdrwrap.cc#L348) | 函数 | `wrap_gdr_open` |
| [F0914](../../src/misc/gdrwrap.cc#L378) | 函数 | `wrap_gdr_close` |
| [F0915](../../src/misc/gdrwrap.cc#L418) | 函数 | `wrap_gdr_pin_buffer` |
| [F0916](../../src/misc/gdrwrap.cc#L465) | 函数 | `wrap_gdr_unpin_buffer` |
| [F0917](../../src/misc/gdrwrap.cc#L507) | 函数 | `wrap_gdr_get_info` |
| [F0918](../../src/misc/gdrwrap.cc#L550) | 函数 | `wrap_gdr_map` |
| [F0919](../../src/misc/gdrwrap.cc#L594) | 函数 | `wrap_gdr_unmap` |
| [F0920](../../src/misc/gdrwrap.cc#L633) | 函数 | `wrap_gdr_runtime_get_version` |
| [F0921](../../src/misc/gdrwrap.cc#L661) | 函数 | `wrap_gdr_driver_get_version` |
| [F0922](../../src/misc/gdrwrap.cc#L695) | 函数 | `wrap_gdr_copy_to_mapping` |
| [F0923](../../src/misc/gdrwrap.cc#L740) | 函数 | `wrap_gdr_copy_from_mapping` |
| [T0390](../../src/misc/gdrwrap.cc#L17) | 类型 | `<匿名enum>` |

## src/misc/ibvwrap.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0924](../../src/misc/ibvwrap.cc#L353) | 函数 | `wrap_ibv_symbols` |
| [F0925](../../src/misc/ibvwrap.cc#L563) | 函数 | `wrap_ibv_fork_init` |
| [F0926](../../src/misc/ibvwrap.cc#L582) | 函数 | `wrap_ibv_get_device_list` |
| [F0927](../../src/misc/ibvwrap.cc#L602) | 函数 | `wrap_ibv_free_device_list` |
| [F0928](../../src/misc/ibvwrap.cc#L621) | 函数 | `wrap_ibv_get_device_name` |
| [F0929](../../src/misc/ibvwrap.cc#L642) | 函数 | `wrap_ibv_open_device` |
| [F0930](../../src/misc/ibvwrap.cc#L658) | 函数 | `wrap_ibv_close_device` |
| [F0931](../../src/misc/ibvwrap.cc#L673) | 函数 | `wrap_ibv_get_async_event` |
| [F0932](../../src/misc/ibvwrap.cc#L689) | 函数 | `wrap_ibv_ack_async_event` |
| [F0933](../../src/misc/ibvwrap.cc#L704) | 函数 | `wrap_ibv_query_device` |
| [F0934](../../src/misc/ibvwrap.cc#L720) | 函数 | `wrap_ibv_query_port` |
| [F0935](../../src/misc/ibvwrap.cc#L737) | 函数 | `wrap_ibv_query_gid` |
| [F0936](../../src/misc/ibvwrap.cc#L755) | 函数 | `wrap_ibv_query_qp` |
| [F0937](../../src/misc/ibvwrap.cc#L773) | 函数 | `wrap_ibv_alloc_pd` |
| [F0938](../../src/misc/ibvwrap.cc#L789) | 函数 | `wrap_ibv_dealloc_pd` |
| [F0939](../../src/misc/ibvwrap.cc#L804) | 函数 | `wrap_ibv_reg_mr` |
| [F0940](../../src/misc/ibvwrap.cc#L827) | 函数 | `wrap_direct_ibv_reg_mr` |
| [F0941](../../src/misc/ibvwrap.cc#L857) | 函数 | `wrap_ibv_reg_mr_iova2` |
| [F0942](../../src/misc/ibvwrap.cc#L883) | 函数 | `wrap_ibv_dereg_mr` |
| [F0943](../../src/misc/ibvwrap.cc#L898) | 函数 | `wrap_ibv_create_cq` |
| [F0944](../../src/misc/ibvwrap.cc#L918) | 函数 | `wrap_ibv_destroy_cq` |
| [F0945](../../src/misc/ibvwrap.cc#L933) | 函数 | `wrap_ibv_destroy_qp` |
| [F0946](../../src/misc/ibvwrap.cc#L948) | 函数 | `wrap_ibv_create_qp` |
| [F0947](../../src/misc/ibvwrap.cc#L965) | 函数 | `wrap_ibv_modify_qp` |
| [F0948](../../src/misc/ibvwrap.cc#L983) | 函数 | `wrap_ibv_event_type_str` |
| [T0391](../../src/misc/ibvwrap.cc#L19) | 类型 | `<匿名enum>` |

## src/misc/nvmlwrap.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0949](../../src/misc/nvmlwrap.cc#L278) | 函数 | `ncclNvmlDeviceGetHandleByPciBusId` |
| [F0950](../../src/misc/nvmlwrap.cc#L300) | 函数 | `ncclNvmlDeviceGetHandleByIndex` |
| [F0951](../../src/misc/nvmlwrap.cc#L326) | 函数 | `ncclNvmlDeviceGetIndex` |
| [F0952](../../src/misc/nvmlwrap.cc#L358) | 函数 | `ncclNvmlDeviceGetNvLinkState` |
| [F0953](../../src/misc/nvmlwrap.cc#L382) | 函数 | `ncclNvmlDeviceGetNvLinkRemotePciInfo` |
| [F0954](../../src/misc/nvmlwrap.cc#L406) | 函数 | `ncclNvmlDeviceGetNvLinkCapability` |
| [F0955](../../src/misc/nvmlwrap.cc#L439) | 函数 | `ncclNvmlDeviceGetCudaComputeCapability` |
| [F0956](../../src/misc/nvmlwrap.cc#L494) | 函数 | `ncclNvmlDeviceGetP2PStatus` |
| [T0392](../../src/misc/nvmlwrap.cc#L99) | 类型 | `Symbol` |

## src/misc/param.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0957](../../src/misc/param.cc#L26) | 函数 | `userHomeDir` |
| [F0958](../../src/misc/param.cc#L77) | 函数 | `setEnvFile` |
| [F0959](../../src/misc/param.cc#L158) | 函数 | `initEnv` |
| [F0960](../../src/misc/param.cc#L206) | 函数 | `ncclLoadParam` |

## src/misc/profiler.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0961](../../src/misc/profiler.cc#L129) | 函数 | `ncclProfilingRecord` |
| [F0962](../../src/misc/profiler.cc#L221) | 函数 | `ncclProfilingDump` |
| [F0963](../../src/misc/profiler.cc#L329) | 函数 | `ncclProfilingRecord` |
| [F0964](../../src/misc/profiler.cc#L344) | 函数 | `ncclProfilingDump` |
| [T0393](../../src/misc/profiler.cc#L38) | 类型 | `ncclProxyProfileEvent` |

## src/misc/shmutils.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0965](../../src/misc/shmutils.cc#L29) | 函数 | `shm_allocate` |
| [F0966](../../src/misc/shmutils.cc#L53) | 函数 | `shm_map` |
| [F0967](../../src/misc/shmutils.cc#L96) | 函数 | `ncclShmSetup` |
| [F0968](../../src/misc/shmutils.cc#L168) | 函数 | `ncclShmOpen` |
| [F0969](../../src/misc/shmutils.cc#L223) | 函数 | `ncclShmUnlink` |
| [F0970](../../src/misc/shmutils.cc#L248) | 函数 | `ncclShmClose` |

## src/misc/socket.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0971](../../src/misc/socket.cc#L36) | 函数 | `ncclSocketToString` |
| [F0972](../../src/misc/socket.cc#L82) | 函数 | `socketToPort` |
| [F0973](../../src/misc/socket.cc#L117) | 函数 | `envSocketFamily` |
| [F0974](../../src/misc/socket.cc#L204) | 函数 | `findInterfaces` |
| [F0975](../../src/misc/socket.cc#L371) | 函数 | `matchSubnet` |
| [F0976](../../src/misc/socket.cc#L507) | 函数 | `ncclFindInterfaceMatchSubnet` |
| [F0977](../../src/misc/socket.cc#L668) | 函数 | `ncclGetSocketAddrFromString` |
| [F0978](../../src/misc/socket.cc#L835) | 函数 | `ncclFindInterfaces` |
| [F0979](../../src/misc/socket.cc#L936) | 函数 | `ncclSocketListen` |
| [F0980](../../src/misc/socket.cc#L1048) | 函数 | `getFdState` |
| [F0981](../../src/misc/socket.cc#L1104) | 函数 | `ncclGetSocketState` |
| [F0982](../../src/misc/socket.cc#L1170) | 函数 | `ncclSocketConnect` |
| [F0983](../../src/misc/socket.cc#L1305) | 函数 | `ncclSocketAccept` |
| [F0984](../../src/misc/socket.cc#L1359) | 函数 | `ncclSocketInit` |
| [F0985](../../src/misc/socket.cc#L1426) | 函数 | `ncclSocketProgressOpt` |
| [F0986](../../src/misc/socket.cc#L1495) | 函数 | `ncclSocketProgress` |
| [F0987](../../src/misc/socket.cc#L1536) | 函数 | `ncclSocketWait` |
| [F0988](../../src/misc/socket.cc#L1561) | 函数 | `ncclSocketSend` |
| [F0989](../../src/misc/socket.cc#L1587) | 函数 | `ncclSocketRecv` |
| [F0990](../../src/misc/socket.cc#L1620) | 函数 | `ncclSocketTryRecv` |

## src/misc/utils.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F0991](../../src/misc/utils.cc#L29) | 函数 | `ncclCudaCompCap` |
| [F0992](../../src/misc/utils.cc#L66) | 函数 | `int64ToBusId` |
| [F0993](../../src/misc/utils.cc#L99) | 函数 | `busIdToInt64` |
| [F0994](../../src/misc/utils.cc#L151) | 函数 | `getBusId` |
| [F0995](../../src/misc/utils.cc#L187) | 函数 | `getHostName` |
| [F0996](../../src/misc/utils.cc#L222) | 函数 | `getHash` |
| [F0997](../../src/misc/utils.cc#L286) | 函数 | `getHostHash` |
| [F0998](../../src/misc/utils.cc#L365) | 函数 | `getPidHash` |
| [F0999](../../src/misc/utils.cc#L438) | 函数 | `parseStringList` |
| [F1000](../../src/misc/utils.cc#L502) | 函数 | `matchIf` |
| [F1001](../../src/misc/utils.cc#L535) | 函数 | `matchPort` |
| [F1002](../../src/misc/utils.cc#L565) | 函数 | `matchIfList` |

## src/nccl.h.in

| ID | 种类 | 符号 |
| --- | --- | --- |
| [T0394](../../src/nccl.h.in#L64) | 类型 | `ncclUniqueId` |
| [T0395](../../src/nccl.h.in#L72) | 类型 | `ncclResult_t` |
| [T0396](../../src/nccl.h.in#L219) | 类型 | `ncclRedOp_dummy_t` |
| [T0397](../../src/nccl.h.in#L226) | 类型 | `ncclRedOp_t` |
| [T0398](../../src/nccl.h.in#L274) | 类型 | `ncclDataType_t` |
| [T0399](../../src/nccl.h.in#L357) | 类型 | `ncclScalarResidence_t` |

## src/net.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1003](../../src/net.cc#L63) | 函数 | `ncclNet_v4_as_v5_getProperties` |
| [F1004](../../src/net.cc#L101) | 函数 | `ncclNet_v4_as_v5_isend` |
| [F1005](../../src/net.cc#L128) | 函数 | `ncclNet_v4_as_v5_irecv` |
| [F1006](../../src/net.cc#L160) | 函数 | `ncclNet_v4_as_v5_iflush` |
| [F1007](../../src/net.cc#L203) | 函数 | `ncclNet_v4_as_v5_init` |
| [F1008](../../src/net.cc#L249) | 函数 | `ncclCollNet_v4_as_v5_getProperties` |
| [F1009](../../src/net.cc#L303) | 函数 | `ncclCollNet_v4_as_v5_init` |
| [F1010](../../src/net.cc#L381) | 函数 | `initPlugin` |
| [F1011](../../src/net.cc#L508) | 函数 | `ncclNetInit` |
| [F1012](../../src/net.cc#L638) | 函数 | `ncclGpuGdrSupport` |
| [F1013](../../src/net.cc#L759) | 函数 | `ncclNetVersion` |

## src/proxy.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1014](../../src/proxy.cc#L43) | 函数 | `NeedProxy` |
| [F1015](../../src/proxy.cc#L130) | 函数 | `allocateArgs` |
| [F1016](../../src/proxy.cc#L225) | 函数 | `getOpIndex` |
| [F1017](../../src/proxy.cc#L320) | 函数 | `printProxyOp` |
| [F1018](../../src/proxy.cc#L448) | 函数 | `dumpProxyState` |
| [F1019](../../src/proxy.cc#L599) | 函数 | `ncclProxyOpToArgs` |
| [F1020](../../src/proxy.cc#L696) | 函数 | `ProxyAppend` |
| [F1021](../../src/proxy.cc#L784) | 函数 | `ncclProxyPost` |
| [F1022](../../src/proxy.cc#L873) | 函数 | `ncclLocalOpAppend` |
| [F1023](../../src/proxy.cc#L1018) | 函数 | `SaveProxy` |
| [F1024](../../src/proxy.cc#L1090) | 函数 | `ncclProxySaveColl` |
| [F1025](../../src/proxy.cc#L1216) | 函数 | `ncclProxyComputeP2p` |
| [F1026](../../src/proxy.cc#L1309) | 函数 | `ncclProxySaveP2p` |
| [F1027](../../src/proxy.cc#L1370) | 函数 | `removeOp` |
| [F1028](../../src/proxy.cc#L1463) | 函数 | `progressOps` |
| [F1029](../../src/proxy.cc#L1591) | 函数 | `ncclProxyGetPostedOps` |
| [F1030](../../src/proxy.cc#L1755) | 函数 | `ncclDumpProxyState` |
| [F1031](../../src/proxy.cc#L1817) | 函数 | `ncclProxyProgress` |
| [F1032](../../src/proxy.cc#L1925) | 函数 | `ncclProxyStart` |
| [F1033](../../src/proxy.cc#L1971) | 函数 | `ncclProxyProgressCreate` |
| [F1034](../../src/proxy.cc#L2012) | 函数 | `ncclProxyProgressDestroy` |
| [F1035](../../src/proxy.cc#L2148) | 函数 | `ncclProxyNewConnection` |
| [F1036](../../src/proxy.cc#L2180) | 函数 | `ncclProxyGetConnection` |
| [F1037](../../src/proxy.cc#L2216) | 函数 | `proxyFree` |
| [F1038](../../src/proxy.cc#L2247) | 函数 | `ncclProxyFreeConnections` |
| [F1039](../../src/proxy.cc#L2322) | 函数 | `ncclProxyConnect` |
| [F1040](../../src/proxy.cc#L2434) | 函数 | `ncclProxyCall` |
| [F1041](../../src/proxy.cc#L2508) | 函数 | `proxyProgressInit` |
| [F1042](../../src/proxy.cc#L2597) | 函数 | `proxyOpsFree` |
| [F1043](../../src/proxy.cc#L2629) | 函数 | `ncclProxyShmUnlink` |
| [F1044](../../src/proxy.cc#L2680) | 函数 | `proxyConnInit` |
| [F1045](../../src/proxy.cc#L2748) | 函数 | `proxyConnSharedInit` |
| [F1046](../../src/proxy.cc#L2820) | 函数 | `proxyProgressAsync` |
| [F1047](../../src/proxy.cc#L2873) | 函数 | `proxyConnSetupConnect` |
| [F1048](../../src/proxy.cc#L3061) | 函数 | `ncclProxyService` |
| [F1049](../../src/proxy.cc#L3278) | 函数 | `ncclProxyInit` |
| [F1050](../../src/proxy.cc#L3300) | 函数 | `ncclProxyCreate` |
| [F1051](../../src/proxy.cc#L3351) | 函数 | `ncclProxyDestroy` |
| [T0400](../../src/proxy.cc#L20) | 类型 | `<匿名enum>` |
| [T0401](../../src/proxy.cc#L92) | 类型 | `ncclProxyPool` |
| [T0402](../../src/proxy.cc#L2053) | 类型 | `ncclProxyAsyncOp` |
| [T0403](../../src/proxy.cc#L2078) | 类型 | `ncclProxyLocalPeer` |
| [T0404](../../src/proxy.cc#L2112) | 类型 | `ncclProxyConnectionPool` |

## src/transport/coll_net.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1052](../../src/transport/coll_net.cc#L378) | 函数 | `canConnect` |
| [F1053](../../src/transport/coll_net.cc#L426) | 函数 | `sendSetup` |
| [F1054](../../src/transport/coll_net.cc#L482) | 函数 | `recvSetup` |
| [F1055](../../src/transport/coll_net.cc#L548) | 函数 | `collNetDumpMap` |
| [F1056](../../src/transport/coll_net.cc#L635) | 函数 | `sendConnect` |
| [F1057](../../src/transport/coll_net.cc#L724) | 函数 | `recvConnect` |
| [F1058](../../src/transport/coll_net.cc#L793) | 函数 | `sendFree` |
| [F1059](../../src/transport/coll_net.cc#L809) | 函数 | `recvFree` |
| [F1060](../../src/transport/coll_net.cc#L835) | 函数 | `sendProxySetup` |
| [F1061](../../src/transport/coll_net.cc#L903) | 函数 | `sharedListen` |
| [F1062](../../src/transport/coll_net.cc#L960) | 函数 | `sharedConnect` |
| [F1063](../../src/transport/coll_net.cc#L1038) | 函数 | `sharedFree` |
| [F1064](../../src/transport/coll_net.cc#L1085) | 函数 | `sharedBuffersInit` |
| [F1065](../../src/transport/coll_net.cc#L1127) | 函数 | `sharedBuffersGet` |
| [F1066](../../src/transport/coll_net.cc#L1166) | 函数 | `sharedBuffersDestroy` |
| [F1067](../../src/transport/coll_net.cc#L1208) | 函数 | `recvProxySetup` |
| [F1068](../../src/transport/coll_net.cc#L1303) | 函数 | `sendProxyConnect` |
| [F1069](../../src/transport/coll_net.cc#L1476) | 函数 | `recvProxyConnect` |
| [F1070](../../src/transport/coll_net.cc#L1612) | 函数 | `sendProxyFree` |
| [F1071](../../src/transport/coll_net.cc#L1670) | 函数 | `recvProxyFree` |
| [F1072](../../src/transport/coll_net.cc#L1836) | 函数 | `sendProxyProgress` |
| [F1073](../../src/transport/coll_net.cc#L2231) | 函数 | `recvProxyProgress` |
| [T0405](../../src/transport/coll_net.cc#L16) | 类型 | `collNetRecvConnectInfo` |
| [T0406](../../src/transport/coll_net.cc#L35) | 类型 | `collNetSendConnectInfo` |
| [T0407](../../src/transport/coll_net.cc#L161) | 类型 | `connectMapMem` |
| [T0408](../../src/transport/coll_net.cc#L180) | 类型 | `connectMap` |
| [T0409](../../src/transport/coll_net.cc#L199) | 类型 | `<匿名struct>` |
| [T0410](../../src/transport/coll_net.cc#L219) | 类型 | `reqSlot` |
| [T0411](../../src/transport/coll_net.cc#L234) | 类型 | `sendResources` |
| [T0412](../../src/transport/coll_net.cc#L303) | 类型 | `recvResources` |
| [T0413](../../src/transport/coll_net.cc#L394) | 类型 | `setupReq` |
| [T0414](../../src/transport/coll_net.cc#L592) | 类型 | `collNetConnectArgs` |
| [T0415](../../src/transport/coll_net.cc#L871) | 类型 | `sharedResources` |

## src/transport/net.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1074](../../src/transport/net.cc#L407) | 函数 | `canConnect` |
| [F1075](../../src/transport/net.cc#L514) | 函数 | `sendSetup` |
| [F1076](../../src/transport/net.cc#L602) | 函数 | `recvSetup` |
| [F1077](../../src/transport/net.cc#L658) | 函数 | `netMapShm` |
| [F1078](../../src/transport/net.cc#L677) | 函数 | `netCreateShm` |
| [F1079](../../src/transport/net.cc#L710) | 函数 | `netDumpMap` |
| [F1080](../../src/transport/net.cc#L800) | 函数 | `sendConnect` |
| [F1081](../../src/transport/net.cc#L913) | 函数 | `recvConnect` |
| [F1082](../../src/transport/net.cc#L981) | 函数 | `sendFree` |
| [F1083](../../src/transport/net.cc#L1009) | 函数 | `recvFree` |
| [F1084](../../src/transport/net.cc#L1079) | 函数 | `sharedBuffersInit` |
| [F1085](../../src/transport/net.cc#L1169) | 函数 | `sharedBuffersGet` |
| [F1086](../../src/transport/net.cc#L1233) | 函数 | `sharedBuffersDestroy` |
| [F1087](../../src/transport/net.cc#L1287) | 函数 | `proxySharedInit` |
| [F1088](../../src/transport/net.cc#L1339) | 函数 | `sendProxySetup` |
| [F1089](../../src/transport/net.cc#L1419) | 函数 | `recvProxySetup` |
| [F1090](../../src/transport/net.cc#L1580) | 函数 | `sendProxyConnect` |
| [F1091](../../src/transport/net.cc#L1864) | 函数 | `recvProxyConnect` |
| [F1092](../../src/transport/net.cc#L2071) | 函数 | `sendProxyFree` |
| [F1093](../../src/transport/net.cc#L2168) | 函数 | `recvProxyFree` |
| [F1094](../../src/transport/net.cc#L2375) | 函数 | `sendProxyProgress` |
| [F1095](../../src/transport/net.cc#L2934) | 函数 | `recvProxyProgress` |
| [T0416](../../src/transport/net.cc#L118) | 类型 | `connectMapMem` |
| [T0417](../../src/transport/net.cc#L135) | 类型 | `<匿名union>` |
| [T0418](../../src/transport/net.cc#L152) | 类型 | `connectMap` |
| [T0419](../../src/transport/net.cc#L179) | 类型 | `<匿名struct>` |
| [T0420](../../src/transport/net.cc#L199) | 类型 | `sendResources` |
| [T0421](../../src/transport/net.cc#L287) | 类型 | `recvResources` |
| [T0422](../../src/transport/net.cc#L445) | 类型 | `setupReq` |

## src/transport/net_ib.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1096](../../src/transport/net_ib.cc#L294) | 函数 | `ncclIbAsyncThreadMain` |
| [F1097](../../src/transport/net_ib.cc#L359) | 函数 | `ncclIbGetPciPath` |
| [F1098](../../src/transport/net_ib.cc#L423) | 函数 | `firstBitSet` |
| [F1099](../../src/transport/net_ib.cc#L446) | 函数 | `ncclIbWidth` |
| [F1100](../../src/transport/net_ib.cc#L461) | 函数 | `ncclIbSpeed` |
| [F1101](../../src/transport/net_ib.cc#L483) | 函数 | `ncclIbRelaxedOrderingCapable` |
| [F1102](../../src/transport/net_ib.cc#L626) | 函数 | `ncclIbInit` |
| [F1103](../../src/transport/net_ib.cc#L821) | 函数 | `ncclIbDevices` |
| [F1104](../../src/transport/net_ib.cc#L849) | 函数 | `ncclIbGdrSupport` |
| [F1105](../../src/transport/net_ib.cc#L879) | 函数 | `GetSocketAddr` |
| [F1106](../../src/transport/net_ib.cc#L916) | 函数 | `ncclIbGetProperties` |
| [F1107](../../src/transport/net_ib.cc#L1447) | 函数 | `ncclIbInitVerbs` |
| [F1108](../../src/transport/net_ib.cc#L1498) | 函数 | `ncclIbDestroyVerbs` |
| [F1109](../../src/transport/net_ib.cc#L1554) | 函数 | `ncclIbCreateQp` |
| [F1110](../../src/transport/net_ib.cc#L1628) | 函数 | `ncclIbRtrQp` |
| [F1111](../../src/transport/net_ib.cc#L1684) | 函数 | `ncclIbRtsQp` |
| [F1112](../../src/transport/net_ib.cc#L1723) | 函数 | `ncclIbListen` |
| [F1113](../../src/transport/net_ib.cc#L1839) | 函数 | `ncclIbConnect` |
| [F1114](../../src/transport/net_ib.cc#L2112) | 函数 | `ncclIbAccept` |
| [F1115](../../src/transport/net_ib.cc#L2320) | 函数 | `ncclIbGetRequest` |
| [F1116](../../src/transport/net_ib.cc#L2358) | 函数 | `ncclIbFreeRequest` |
| [F1117](../../src/transport/net_ib.cc#L2389) | 函数 | `ncclSendCheck` |
| [F1118](../../src/transport/net_ib.cc#L2446) | 函数 | `ncclRecvCheck` |
| [F1119](../../src/transport/net_ib.cc#L2532) | 函数 | `ncclIbRegMr` |
| [F1120](../../src/transport/net_ib.cc#L2667) | 函数 | `ncclIbDeregMr` |
| [F1121](../../src/transport/net_ib.cc#L2794) | 函数 | `ncclIbMultiSend` |
| [F1122](../../src/transport/net_ib.cc#L3037) | 函数 | `ncclIbIsend` |
| [F1123](../../src/transport/net_ib.cc#L3212) | 函数 | `ncclIbPostFifo` |
| [F1124](../../src/transport/net_ib.cc#L3358) | 函数 | `ncclIbIrecv` |
| [F1125](../../src/transport/net_ib.cc#L3476) | 函数 | `ncclIbIflush` |
| [F1126](../../src/transport/net_ib.cc#L3621) | 函数 | `ncclIbTest` |
| [F1127](../../src/transport/net_ib.cc#L3762) | 函数 | `ncclIbCloseSend` |
| [F1128](../../src/transport/net_ib.cc#L3819) | 函数 | `ncclIbCloseRecv` |
| [F1129](../../src/transport/net_ib.cc#L3862) | 函数 | `ncclIbCloseListen` |
| [T0423](../../src/transport/net_ib.cc#L59) | 类型 | `ncclIbMr` |
| [T0424](../../src/transport/net_ib.cc#L82) | 类型 | `ncclIbMrCache` |
| [T0425](../../src/transport/net_ib.cc#L103) | 类型 | `ncclIbDev` |
| [T0426](../../src/transport/net_ib.cc#L171) | 类型 | `userIbDev` |
| [T0427](../../src/transport/net_ib.cc#L957) | 类型 | `ncclIbQpInfo` |
| [T0428](../../src/transport/net_ib.cc#L1004) | 类型 | `ncclIbCommState` |
| [T0429](../../src/transport/net_ib.cc#L1041) | 类型 | `ncclIbCommStage` |
| [T0430](../../src/transport/net_ib.cc#L1064) | 类型 | `ncclIbHandle` |
| [T0431](../../src/transport/net_ib.cc#L1100) | 类型 | `ncclIbRequest` |
| [T0432](../../src/transport/net_ib.cc#L1125) | 类型 | `<匿名union>` |
| [T0433](../../src/transport/net_ib.cc#L1135) | 类型 | `<匿名struct>` |
| [T0434](../../src/transport/net_ib.cc#L1160) | 类型 | `<匿名struct>` |
| [T0435](../../src/transport/net_ib.cc#L1177) | 类型 | `ncclIbVerbs` |
| [T0436](../../src/transport/net_ib.cc#L1204) | 类型 | `ncclIbListenComm` |
| [T0437](../../src/transport/net_ib.cc#L1223) | 类型 | `ncclIbSendFifo` |
| [T0438](../../src/transport/net_ib.cc#L1258) | 类型 | `ncclIbSendComm` |
| [T0439](../../src/transport/net_ib.cc#L1315) | 类型 | `ncclIbGpuFlush` |
| [T0440](../../src/transport/net_ib.cc#L1342) | 类型 | `ncclIbRemFifo` |
| [T0441](../../src/transport/net_ib.cc#L1381) | 类型 | `ncclIbRecvComm` |

## src/transport/net_socket.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1130](../../src/transport/net_socket.cc#L63) | 函数 | `ncclSocketGetPciPath` |
| [F1131](../../src/transport/net_socket.cc#L115) | 函数 | `ncclSocketInit` |
| [F1132](../../src/transport/net_socket.cc#L188) | 函数 | `ncclSocketDevices` |
| [F1133](../../src/transport/net_socket.cc#L220) | 函数 | `ncclSocketGetSpeed` |
| [F1134](../../src/transport/net_socket.cc#L274) | 函数 | `ncclSocketGetProperties` |
| [F1135](../../src/transport/net_socket.cc#L304) | 函数 | `GetSocketAddr` |
| [F1136](../../src/transport/net_socket.cc#L673) | 函数 | `persistentSocketThread` |
| [F1137](../../src/transport/net_socket.cc#L814) | 函数 | `ncclSocketGetNsockNthread` |
| [F1138](../../src/transport/net_socket.cc#L920) | 函数 | `ncclSocketNewListenComm` |
| [F1139](../../src/transport/net_socket.cc#L944) | 函数 | `ncclSocketNewComm` |
| [F1140](../../src/transport/net_socket.cc#L988) | 函数 | `ncclSocketListen` |
| [F1141](../../src/transport/net_socket.cc#L1079) | 函数 | `ncclSocketConnect` |
| [F1142](../../src/transport/net_socket.cc#L1223) | 函数 | `ncclSocketAccept` |
| [F1143](../../src/transport/net_socket.cc#L1332) | 函数 | `ncclSocketGetRequest` |
| [F1144](../../src/transport/net_socket.cc#L1407) | 函数 | `ncclSocketGetTask` |
| [F1145](../../src/transport/net_socket.cc#L1557) | 函数 | `ncclSocketTest` |
| [F1146](../../src/transport/net_socket.cc#L1699) | 函数 | `ncclSocketRegMr` |
| [F1147](../../src/transport/net_socket.cc#L1718) | 函数 | `ncclSocketDeregMr` |
| [F1148](../../src/transport/net_socket.cc#L1735) | 函数 | `ncclSocketIsend` |
| [F1149](../../src/transport/net_socket.cc#L1767) | 函数 | `ncclSocketIrecv` |
| [F1150](../../src/transport/net_socket.cc#L1797) | 函数 | `ncclSocketIflush` |
| [F1151](../../src/transport/net_socket.cc#L1826) | 函数 | `ncclSocketCloseListen` |
| [F1152](../../src/transport/net_socket.cc#L1875) | 函数 | `ncclSocketClose` |
| [T0442](../../src/transport/net_socket.cc#L25) | 类型 | `ncclSocketDev` |
| [T0443](../../src/transport/net_socket.cc#L355) | 类型 | `ncclSocketCommState` |
| [T0444](../../src/transport/net_socket.cc#L387) | 类型 | `ncclSocketCommStage` |
| [T0445](../../src/transport/net_socket.cc#L410) | 类型 | `ncclSocketHandle` |
| [T0446](../../src/transport/net_socket.cc#L433) | 类型 | `ncclSocketTask` |
| [T0447](../../src/transport/net_socket.cc#L468) | 类型 | `ncclSocketRequest` |
| [T0448](../../src/transport/net_socket.cc#L511) | 类型 | `ncclSocketTaskQueue` |
| [T0449](../../src/transport/net_socket.cc#L530) | 类型 | `ncclSocketThreadResources` |
| [T0450](../../src/transport/net_socket.cc#L557) | 类型 | `ncclSocketListenComm` |
| [T0451](../../src/transport/net_socket.cc#L584) | 类型 | `ncclSocketComm` |

## src/transport/p2p.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1153](../../src/transport/p2p.cc#L113) | 函数 | `busIdToCudaDev` |
| [F1154](../../src/transport/p2p.cc#L214) | 函数 | `p2pCanConnect` |
| [F1155](../../src/transport/p2p.cc#L358) | 函数 | `p2pGetInfo` |
| [F1156](../../src/transport/p2p.cc#L417) | 函数 | `p2pMap` |
| [F1157](../../src/transport/p2p.cc#L497) | 函数 | `p2pSendSetup` |
| [F1158](../../src/transport/p2p.cc#L621) | 函数 | `p2pRecvSetup` |
| [F1159](../../src/transport/p2p.cc#L721) | 函数 | `p2pSendConnect` |
| [F1160](../../src/transport/p2p.cc#L801) | 函数 | `p2pRecvConnect` |
| [F1161](../../src/transport/p2p.cc#L871) | 函数 | `p2pSendFree` |
| [F1162](../../src/transport/p2p.cc#L904) | 函数 | `p2pRecvFree` |
| [F1163](../../src/transport/p2p.cc#L947) | 函数 | `p2pProxySetup` |
| [F1164](../../src/transport/p2p.cc#L997) | 函数 | `p2pProxyFree` |
| [T0452](../../src/transport/p2p.cc#L17) | 类型 | `ncclP2pBuff` |
| [T0453](../../src/transport/p2p.cc#L32) | 类型 | `p2pConnectInfo` |
| [T0454](../../src/transport/p2p.cc#L52) | 类型 | `p2pSendResources` |
| [T0455](../../src/transport/p2p.cc#L71) | 类型 | `p2pRecvResources` |

## src/transport/shm.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1165](../../src/transport/shm.cc#L118) | 函数 | `shmCanConnect` |
| [F1166](../../src/transport/shm.cc#L173) | 函数 | `shmSendSetup` |
| [F1167](../../src/transport/shm.cc#L244) | 函数 | `shmRecvSetup` |
| [F1168](../../src/transport/shm.cc#L321) | 函数 | `shmSendConnect` |
| [F1169](../../src/transport/shm.cc#L398) | 函数 | `shmRecvConnect` |
| [F1170](../../src/transport/shm.cc#L461) | 函数 | `shmSendFree` |
| [F1171](../../src/transport/shm.cc#L489) | 函数 | `shmRecvFree` |
| [T0456](../../src/transport/shm.cc#L10) | 类型 | `shmConnectInfo` |
| [T0457](../../src/transport/shm.cc#L26) | 类型 | `shmSendResources` |
| [T0458](../../src/transport/shm.cc#L57) | 类型 | `shmRecvResources` |

## src/transport.cc

| ID | 种类 | 符号 |
| --- | --- | --- |
| [F1172](../../src/transport.cc#L73) | 函数 | `selectTransport` |
| [F1173](../../src/transport.cc#L173) | 函数 | `ncclTransportP2pConnect` |
| [F1174](../../src/transport.cc#L237) | 函数 | `dumpData` |
| [F1175](../../src/transport.cc#L360) | 函数 | `ncclTransportP2pSetup` |
| [F1176](../../src/transport.cc#L617) | 函数 | `ncclTransportCollNetSetup` |
| [F1177](../../src/transport.cc#L844) | 函数 | `ncclTransportCollNetCheck` |
| [F1178](../../src/transport.cc#L902) | 函数 | `ncclTransportCollNetFree` |
| [T0459](../../src/transport.cc#L668) | 类型 | `<匿名struct>` |
| [T0460](../../src/transport.cc#L749) | 类型 | `<匿名struct>` |
