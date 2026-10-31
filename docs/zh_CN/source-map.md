# 按问题进入源码

源码中的详细说明以 `中文逐项-函数/类型/变量`、`中文精读` 和 `中文导读` 标记。若默认master中看不到，请打开 [根目录阅读入口](../../START_HERE_ZH.md)确认分支和下载方式。

| 问题 | 主要入口 |
| --- | --- |
| 各API输入输出与调用流程 | [collectives](../../src/collectives)、[argcheck.cc](../../src/misc/argcheck.cc)、[enqueue.cc](../../src/enqueue.cc) |
| channel如何生成、复制和限量 | [init.cc](../../src/init.cc)、[search.cc](../../src/graph/search.cc)、[connect.cc](../../src/graph/connect.cc) |
| 分片公式、地址和尾块 | [enqueue.cc](../../src/enqueue.cc)、[all_reduce.h](../../src/collectives/device/all_reduce.h)、[all_gather.h](../../src/collectives/device/all_gather.h)、[reduce_scatter.h](../../src/collectives/device/reduce_scatter.h) |
| Group和P2P调度 | [group.cc](../../src/group.cc)、[channel.h](../../src/include/channel.h)、[sendrecv.h](../../src/collectives/device/sendrecv.h) |
| kernel怎样触发并解释FIFO | [enqueue.cc](../../src/enqueue.cc)、[common.h](../../src/collectives/device/common.h)、[functions.cu](../../src/collectives/device/functions.cu) |
| 原语怎样等待、复制、归约与发布 | [prims_simple.h](../../src/collectives/device/prims_simple.h)、[primitives.h](../../src/collectives/device/primitives.h)、[common_kernel.h](../../src/collectives/device/common_kernel.h) |
| LL/LL128行格式和流控 | [devcomm.h](../../src/include/devcomm.h)、[prims_ll.h](../../src/collectives/device/prims_ll.h)、[prims_ll128.h](../../src/collectives/device/prims_ll128.h) |
| Proxy由谁发布、如何推进 | [proxy.cc](../../src/proxy.cc)、[net.cc](../../src/transport/net.cc) |
| QP/MR和网络完成 | [net_ib.cc](../../src/transport/net_ib.cc)、[ibvwrap.h](../../src/include/ibvwrap.h) |
| 全部结构体和变量 | [comm.h](../../src/include/comm.h)、[devcomm.h](../../src/include/devcomm.h)、[proxy.h](../../src/include/proxy.h)、[完整索引](symbol-index.md) |
| 构建怎样实例化所有设备函数 | [gen_rules.sh](../../src/collectives/device/gen_rules.sh)、[设备Makefile](../../src/collectives/device/Makefile) |
| 附带SDK/兼容ABI的用途 | [nvtx3.hpp](../../src/include/nvtx3.hpp)、[nvmlwrap.h](../../src/include/nvmlwrap.h)、[gdrwrap.h](../../src/include/gdrwrap.h)、[enhcompat.cc](../../src/enhcompat.cc) |

## 逐文件覆盖

| 文件 | 函数定义 | 类型定义 | 变量项 |
| --- | ---: | ---: | ---: |
| [ext-net/dummy/plugin.c](../../ext-net/dummy/plugin.c) | 30 | 0 | 2 |
| [src/bootstrap.cc](../../src/bootstrap.cc) | 17 | 4 | 80 |
| [src/channel.cc](../../src/channel.cc) | 2 | 0 | 9 |
| [src/collectives/all_gather.cc](../../src/collectives/all_gather.cc) | 1 | 0 | 1 |
| [src/collectives/all_reduce.cc](../../src/collectives/all_reduce.cc) | 1 | 0 | 1 |
| [src/collectives/broadcast.cc](../../src/collectives/broadcast.cc) | 2 | 0 | 1 |
| [src/collectives/device/all_gather.cu](../../src/collectives/device/all_gather.cu) | 0 | 0 | 0 |
| [src/collectives/device/all_gather.h](../../src/collectives/device/all_gather.h) | 4 | 3 | 21 |
| [src/collectives/device/all_reduce.cu](../../src/collectives/device/all_reduce.cu) | 0 | 0 | 0 |
| [src/collectives/device/all_reduce.h](../../src/collectives/device/all_reduce.h) | 10 | 7 | 123 |
| [src/collectives/device/broadcast.cu](../../src/collectives/device/broadcast.cu) | 0 | 0 | 0 |
| [src/collectives/device/broadcast.h](../../src/collectives/device/broadcast.h) | 4 | 3 | 19 |
| [src/collectives/device/common.h](../../src/collectives/device/common.h) | 7 | 5 | 39 |
| [src/collectives/device/common_kernel.h](../../src/collectives/device/common_kernel.h) | 49 | 40 | 168 |
| [src/collectives/device/functions.cu](../../src/collectives/device/functions.cu) | 1 | 0 | 2 |
| [src/collectives/device/onerank_reduce.cu](../../src/collectives/device/onerank_reduce.cu) | 1 | 0 | 14 |
| [src/collectives/device/op128.h](../../src/collectives/device/op128.h) | 6 | 1 | 9 |
| [src/collectives/device/primitives.h](../../src/collectives/device/primitives.h) | 21 | 6 | 16 |
| [src/collectives/device/prims_ll.h](../../src/collectives/device/prims_ll.h) | 38 | 6 | 78 |
| [src/collectives/device/prims_ll128.h](../../src/collectives/device/prims_ll128.h) | 32 | 1 | 91 |
| [src/collectives/device/prims_simple.h](../../src/collectives/device/prims_simple.h) | 33 | 3 | 94 |
| [src/collectives/device/reduce.cu](../../src/collectives/device/reduce.cu) | 0 | 0 | 0 |
| [src/collectives/device/reduce.h](../../src/collectives/device/reduce.h) | 4 | 3 | 28 |
| [src/collectives/device/reduce_kernel.h](../../src/collectives/device/reduce_kernel.h) | 106 | 45 | 113 |
| [src/collectives/device/reduce_scatter.cu](../../src/collectives/device/reduce_scatter.cu) | 0 | 0 | 0 |
| [src/collectives/device/reduce_scatter.h](../../src/collectives/device/reduce_scatter.h) | 4 | 3 | 19 |
| [src/collectives/device/sendrecv.cu](../../src/collectives/device/sendrecv.cu) | 0 | 0 | 0 |
| [src/collectives/device/sendrecv.h](../../src/collectives/device/sendrecv.h) | 3 | 1 | 19 |
| [src/collectives/reduce.cc](../../src/collectives/reduce.cc) | 1 | 0 | 1 |
| [src/collectives/reduce_scatter.cc](../../src/collectives/reduce_scatter.cc) | 1 | 0 | 1 |
| [src/collectives/sendrecv.cc](../../src/collectives/sendrecv.cc) | 2 | 0 | 4 |
| [src/debug.cc](../../src/debug.cc) | 3 | 0 | 29 |
| [src/enhcompat.cc](../../src/enhcompat.cc) | 5 | 1 | 0 |
| [src/enqueue.cc](../../src/enqueue.cc) | 40 | 2 | 188 |
| [src/graph/connect.cc](../../src/graph/connect.cc) | 11 | 0 | 81 |
| [src/graph/paths.cc](../../src/graph/paths.cc) | 20 | 1 | 149 |
| [src/graph/rings.cc](../../src/graph/rings.cc) | 2 | 0 | 10 |
| [src/graph/rings.h](../../src/graph/rings.h) | 0 | 0 | 0 |
| [src/graph/search.cc](../../src/graph/search.cc) | 31 | 1 | 190 |
| [src/graph/topo.cc](../../src/graph/topo.cc) | 30 | 1 | 132 |
| [src/graph/topo.h](../../src/graph/topo.h) | 3 | 10 | 43 |
| [src/graph/trees.cc](../../src/graph/trees.cc) | 2 | 0 | 12 |
| [src/graph/tuning.cc](../../src/graph/tuning.cc) | 4 | 0 | 67 |
| [src/graph/xml.cc](../../src/graph/xml.cc) | 37 | 5 | 122 |
| [src/graph/xml.h](../../src/graph/xml.h) | 20 | 4 | 41 |
| [src/group.cc](../../src/group.cc) | 11 | 5 | 98 |
| [src/include/align.h](../../src/include/align.h) | 3 | 0 | 0 |
| [src/include/alloc.h](../../src/include/alloc.h) | 7 | 0 | 8 |
| [src/include/argcheck.h](../../src/include/argcheck.h) | 0 | 0 | 0 |
| [src/include/bootstrap.h](../../src/include/bootstrap.h) | 0 | 0 | 0 |
| [src/include/channel.h](../../src/include/channel.h) | 3 | 0 | 8 |
| [src/include/checks.h](../../src/include/checks.h) | 0 | 0 | 0 |
| [src/include/coll_net.h](../../src/include/coll_net.h) | 14 | 0 | 1 |
| [src/include/collectives.h](../../src/include/collectives.h) | 0 | 2 | 3 |
| [src/include/comm.h](../../src/include/comm.h) | 1 | 14 | 117 |
| [src/include/core.h](../../src/include/core.h) | 1 | 0 | 0 |
| [src/include/cpuset.h](../../src/include/cpuset.h) | 3 | 0 | 10 |
| [src/include/debug.h](../../src/include/debug.h) | 0 | 0 | 6 |
| [src/include/devcomm.h](../../src/include/devcomm.h) | 0 | 23 | 104 |
| [src/include/enqueue.h](../../src/include/enqueue.h) | 3 | 3 | 20 |
| [src/include/gdrwrap.h](../../src/include/gdrwrap.h) | 20 | 3 | 35 |
| [src/include/graph.h](../../src/include/graph.h) | 0 | 2 | 23 |
| [src/include/group.h](../../src/include/group.h) | 0 | 0 | 0 |
| [src/include/ibvwrap.h](../../src/include/ibvwrap.h) | 5 | 91 | 372 |
| [src/include/info.h](../../src/include/info.h) | 0 | 2 | 23 |
| [src/include/nccl_net.h](../../src/include/nccl_net.h) | 0 | 8 | 76 |
| [src/include/net.h](../../src/include/net.h) | 15 | 0 | 3 |
| [src/include/nvmlwrap.h](../../src/include/nvmlwrap.h) | 0 | 8 | 19 |
| [src/include/nvtx.h](../../src/include/nvtx.h) | 0 | 1 | 1 |
| [src/include/nvtx3/nvToolsExt.h](../../src/include/nvtx3/nvToolsExt.h) | 0 | 9 | 27 |
| [src/include/nvtx3/nvToolsExtCuda.h](../../src/include/nvtx3/nvToolsExtCuda.h) | 0 | 1 | 0 |
| [src/include/nvtx3/nvToolsExtCudaRt.h](../../src/include/nvtx3/nvToolsExtCudaRt.h) | 0 | 1 | 0 |
| [src/include/nvtx3/nvToolsExtOpenCL.h](../../src/include/nvtx3/nvToolsExtOpenCL.h) | 0 | 1 | 0 |
| [src/include/nvtx3/nvToolsExtSync.h](../../src/include/nvtx3/nvToolsExtSync.h) | 0 | 4 | 4 |
| [src/include/nvtx3/nvtxDetail/nvtxImpl.h](../../src/include/nvtx3/nvtxDetail/nvtxImpl.h) | 3 | 1 | 77 |
| [src/include/nvtx3/nvtxDetail/nvtxImplCore.h](../../src/include/nvtx3/nvtxDetail/nvtxImplCore.h) | 30 | 0 | 30 |
| [src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h](../../src/include/nvtx3/nvtxDetail/nvtxImplCudaRt_v3.h) | 6 | 0 | 6 |
| [src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h](../../src/include/nvtx3/nvtxDetail/nvtxImplCuda_v3.h) | 8 | 0 | 8 |
| [src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h](../../src/include/nvtx3/nvtxDetail/nvtxImplOpenCL_v3.h) | 14 | 0 | 14 |
| [src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h](../../src/include/nvtx3/nvtxDetail/nvtxImplSync_v3.h) | 6 | 0 | 6 |
| [src/include/nvtx3/nvtxDetail/nvtxInit.h](../../src/include/nvtx3/nvtxDetail/nvtxInit.h) | 2 | 0 | 21 |
| [src/include/nvtx3/nvtxDetail/nvtxInitDecls.h](../../src/include/nvtx3/nvtxDetail/nvtxInitDecls.h) | 0 | 0 | 0 |
| [src/include/nvtx3/nvtxDetail/nvtxInitDefs.h](../../src/include/nvtx3/nvtxDetail/nvtxInitDefs.h) | 65 | 0 | 34 |
| [src/include/nvtx3/nvtxDetail/nvtxLinkOnce.h](../../src/include/nvtx3/nvtxDetail/nvtxLinkOnce.h) | 0 | 0 | 0 |
| [src/include/nvtx3/nvtxDetail/nvtxTypes.h](../../src/include/nvtx3/nvtxDetail/nvtxTypes.h) | 0 | 10 | 4 |
| [src/include/nvtx3.hpp](../../src/include/nvtx3.hpp) | 60 | 14 | 28 |
| [src/include/p2p.h](../../src/include/p2p.h) | 1 | 1 | 3 |
| [src/include/param.h](../../src/include/param.h) | 0 | 0 | 1 |
| [src/include/profiler.h](../../src/include/profiler.h) | 0 | 1 | 0 |
| [src/include/proxy.h](../../src/include/proxy.h) | 0 | 15 | 105 |
| [src/include/shm.h](../../src/include/shm.h) | 0 | 0 | 0 |
| [src/include/socket.h](../../src/include/socket.h) | 0 | 3 | 9 |
| [src/include/timer.h](../../src/include/timer.h) | 2 | 0 | 9 |
| [src/include/transport.h](../../src/include/transport.h) | 0 | 5 | 24 |
| [src/include/trees.h](../../src/include/trees.h) | 0 | 0 | 0 |
| [src/include/utils.h](../../src/include/utils.h) | 9 | 3 | 11 |
| [src/init.cc](../../src/init.cc) | 28 | 2 | 144 |
| [src/misc/argcheck.cc](../../src/misc/argcheck.cc) | 3 | 0 | 3 |
| [src/misc/gdrwrap.cc](../../src/misc/gdrwrap.cc) | 12 | 1 | 24 |
| [src/misc/ibvwrap.cc](../../src/misc/ibvwrap.cc) | 25 | 1 | 27 |
| [src/misc/nvmlwrap.cc](../../src/misc/nvmlwrap.cc) | 8 | 1 | 21 |
| [src/misc/param.cc](../../src/misc/param.cc) | 4 | 0 | 13 |
| [src/misc/profiler.cc](../../src/misc/profiler.cc) | 4 | 1 | 24 |
| [src/misc/shmutils.cc](../../src/misc/shmutils.cc) | 6 | 0 | 4 |
| [src/misc/socket.cc](../../src/misc/socket.cc) | 20 | 0 | 89 |
| [src/misc/utils.cc](../../src/misc/utils.cc) | 12 | 0 | 25 |
| [src/nccl.h.in](../../src/nccl.h.in) | 0 | 6 | 3 |
| [src/net.cc](../../src/net.cc) | 11 | 0 | 33 |
| [src/proxy.cc](../../src/proxy.cc) | 38 | 5 | 167 |
| [src/transport/coll_net.cc](../../src/transport/coll_net.cc) | 22 | 11 | 192 |
| [src/transport/net.cc](../../src/transport/net.cc) | 22 | 7 | 251 |
| [src/transport/net_ib.cc](../../src/transport/net_ib.cc) | 34 | 19 | 256 |
| [src/transport/net_socket.cc](../../src/transport/net_socket.cc) | 23 | 10 | 126 |
| [src/transport/p2p.cc](../../src/transport/p2p.cc) | 12 | 4 | 53 |
| [src/transport/shm.cc](../../src/transport/shm.cc) | 7 | 3 | 35 |
| [src/transport.cc](../../src/transport.cc) | 7 | 2 | 75 |

## V3图解与函数内部步骤

[数据分片、原语与不同通信方式的详细图解](visual-guide.md) · [全部函数内部步骤](function-logic-index.md) · [离线函数搜索](function-logic.html)。完整内部步骤也在各函数前的“中文内部逻辑”注释中。
