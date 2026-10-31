# 中文注释源码阅读入口

基线：`04832308d0885226169b9376c1ba4f17b0cb6484`，NCCL 2.12.12、打包修订 1。

讲解直接写在代码的 `中文精读` / `中文导读` 块注释中。本页只做导航。注释说明函数在调用链中的位置、输入输出及单位、关键分支、数据和对象生命周期，以及为什么使用这种组织方式。

## 按问题进入源码

| 要理解的问题 | 推荐阅读顺序 |
| --- | --- |
| 一次调用到底做了什么 | `collectives/*.cc` → `misc/argcheck.cc` → `enqueue.cc::ncclEnqueueCheck` |
| 每种 API 的数据布局 | 各 API 的 `.cc` 入口示例 → `collectives/device/` 同名 `.h` |
| channel 怎样生成 | `init.cc::initTransportsRank` → `graph/search.cc` → `graph/connect.cc` → `channel.cc` |
| 为什么这次没用满所有 channel | `graph/tuning.cc` → `enqueue.cc::getAlgoInfo / ncclSetupAsyncKernels` |
| chunk、slice、step 如何对应 | `enqueue.cc::computeColl` → `device/primitives.h` → `device/prims_simple.h::genericOp` |
| 每个块对应哪段地址 | `device/all_reduce.h::calcOffset`，再比较 `all_gather.h` 和 `reduce_scatter.h` |
| Group 如何拆 P2P 消息 | `group.cc::getP2pChunkSize / ncclGroupEnd` → `include/channel.h` → `device/sendrecv.h` |
| 怎样发射 kernel | `enqueue.cc::ncclSetupCollKernel / setupLaunch / ncclLaunchKernel` → `device/common.h` |
| 为什么 kernel 名称不等于实际算法 | `include/collectives.h::FUNC_INDEX` → `device/functions.cu` → `device/common.h::ncclKernel` |
| 谁触发 Proxy | `enqueue.cc::ncclEnqueueHostSetup / ncclLaunchProxy` → `proxy.cc::ncclProxyStart / ncclProxyPost` |
| Proxy 如何等 GPU 再发网络 | `transport/net.cc::sendProxyProgress` → `transport/net_ib.cc::ncclIbIsend` |
| 网络收完 GPU 为什么还在等 | `transport/net.cc::recvProxyProgress` → `net_ib.cc::ncclIbIflush` → `prims_simple.h::waitPeer` |
| 原语怎样组合收、发、归约 | `device/primitives.h` → `prims_simple.h` → `common_kernel.h::ReduceOrCopyMulti` |
| LL/LL128 与 Simple 有什么不同 | `include/devcomm.h` 的 FIFO 格式 → `device/prims_ll.h / prims_ll128.h` |
| Tree/CollNet 的线程怎样分工 | `device/all_reduce.h::runTreeSplit` 与 CollNet 特化 → `transport/coll_net.cc` |
| 结构体怎样串起来 | `include/comm.h` → `devcomm.h` → `enqueue.h` → `proxy.h` → `transport.h` |
| 谁分配、共享和释放内存 | `include/alloc.h` → `transport/*::setup/connect/free` → `channel.cc::freeChannel` → `init.cc::commFree` |
| 为什么会创建 QP、MR | `transport/net.cc` → `transport/net_ib.cc`；先分清逻辑 connector、后端 comm 和 verbs 对象 |

表中的路径省略了共同前缀 `src/`。下面可直接点击具体文件。

## API 范围与理解边界

本版公开数据通信包括 AllReduce、AllGather、ReduceScatter、Broadcast/Bcast、Reduce、Send、Recv；另外有 communicator 初始化/查询/销毁/中止、Group 和 PreMulSum 操作管理。源码注释区分这些路径。

本版没有公开 `ncclAllToAll`、`ncclGather`、`ncclScatter`：这类模式可由上层组合 Send/Recv，不能把较新版 API 的调用链安到本版本。NVLS、较新的 nonblocking communicator 和专用 EP 后端同样不属于这里的实现。

普通非图路径的顺序以实际调用点为准：HostSetup 装 FIFO、发布 Proxy 操作，然后发射 GPU kernel。`ncclLaunchProxy` 附近原有英文注释提到的先后顺序与此调用点有出入，已在旁边加入中文说明；图路径则按 host node 与 kernel 的依赖执行。

本分支重点覆盖 NCCL 自身的主干和辅助模块；不逐行翻译第三方 NVTX SDK，也不把函数级注释等同于每条指令已完成硬件验证。

## 源码注释目录

数字是本分支相对基线新增的中文注释块数，不包含原有英文注释。链接指向该文件第一段中文注释；同文件其余注释可搜索“中文精读”。

| 源码文件 | 注释块数 | 首个阅读点 |
| --- | ---: | --- |
| [src/bootstrap.cc](../../src/bootstrap.cc#L173) | 6 | 中文导读： |
| [src/channel.cc](../../src/channel.cc#L14) | 2 | 中文导读： |
| [src/collectives/all_gather.cc](../../src/collectives/all_gather.cc#L12) | 2 | 中文导读： |
| [src/collectives/all_reduce.cc](../../src/collectives/all_reduce.cc#L11) | 2 | 中文导读： |
| [src/collectives/broadcast.cc](../../src/collectives/broadcast.cc#L12) | 2 | 中文导读： |
| [src/collectives/device/all_gather.cu](../../src/collectives/device/all_gather.cu#L11) | 1 | 中文精读：这个短 .cu 文件是模板实例化入口 |
| [src/collectives/device/all_gather.h](../../src/collectives/device/all_gather.h#L13) | 2 | 中文导读： |
| [src/collectives/device/all_reduce.cu](../../src/collectives/device/all_reduce.cu#L11) | 1 | 中文精读：这个短 .cu 文件是模板实例化入口 |
| [src/collectives/device/all_reduce.h](../../src/collectives/device/all_reduce.h#L13) | 9 | 中文导读： |
| [src/collectives/device/broadcast.cu](../../src/collectives/device/broadcast.cu#L11) | 1 | 中文精读：这个短 .cu 文件是模板实例化入口 |
| [src/collectives/device/broadcast.h](../../src/collectives/device/broadcast.h#L47) | 1 | 中文精读：Broadcast 如何把环变成从 root 开始的流水线 |
| [src/collectives/device/common.h](../../src/collectives/device/common.h#L117) | 4 | 中文精读：共享内存装的是元数据和小块暂存 |
| [src/collectives/device/common_kernel.h](../../src/collectives/device/common_kernel.h#L630) | 1 | 中文精读：最终负责读、归约、写的线程搬运层 |
| [src/collectives/device/functions.cu](../../src/collectives/device/functions.cu#L89) | 1 | 中文精读：设备函数表怎样被填满 |
| [src/collectives/device/onerank_reduce.cu](../../src/collectives/device/onerank_reduce.cu#L14) | 1 | 中文精读：只有一个 rank 时仍可能需要 GPU 运算 |
| [src/collectives/device/op128.h](../../src/collectives/device/op128.h#L10) | 1 | 中文精读：读 PTX 内联汇编的方法 |
| [src/collectives/device/primitives.h](../../src/collectives/device/primitives.h#L23) | 3 | 中文精读：算法与协议通过模板解耦 |
| [src/collectives/device/prims_ll.h](../../src/collectives/device/prims_ll.h#L58) | 3 | 中文精读：LL 仍然需要空间流控 |
| [src/collectives/device/prims_ll128.h](../../src/collectives/device/prims_ll128.h#L9) | 2 | 中文精读：LL128 如何提高有效载荷比例 |
| [src/collectives/device/prims_simple.h](../../src/collectives/device/prims_simple.h#L72) | 11 | 中文导读： |
| [src/collectives/device/reduce.cu](../../src/collectives/device/reduce.cu#L11) | 1 | 中文精读：这个短 .cu 文件是模板实例化入口 |
| [src/collectives/device/reduce.h](../../src/collectives/device/reduce.h#L44) | 1 | 中文精读：Reduce 的流水线起点在 root 的后继 |
| [src/collectives/device/reduce_kernel.h](../../src/collectives/device/reduce_kernel.h#L24) | 3 | 中文精读：数值操作层不负责通信 |
| [src/collectives/device/reduce_scatter.cu](../../src/collectives/device/reduce_scatter.cu#L11) | 1 | 中文精读：这个短 .cu 文件是模板实例化入口 |
| [src/collectives/device/reduce_scatter.h](../../src/collectives/device/reduce_scatter.h#L13) | 2 | 中文导读： |
| [src/collectives/device/sendrecv.cu](../../src/collectives/device/sendrecv.cu#L11) | 1 | 中文精读：这个短 .cu 文件是模板实例化入口 |
| [src/collectives/device/sendrecv.h](../../src/collectives/device/sendrecv.h#L12) | 3 | 中文导读： |
| [src/collectives/reduce.cc](../../src/collectives/reduce.cc#L12) | 2 | 中文导读： |
| [src/collectives/reduce_scatter.cc](../../src/collectives/reduce_scatter.cc#L12) | 2 | 中文导读： |
| [src/collectives/sendrecv.cc](../../src/collectives/sendrecv.cc#L13) | 3 | 中文导读： |
| [src/debug.cc](../../src/debug.cc#L18) | 1 | 中文精读：日志开关如何帮助对应源码阶段 |
| [src/enqueue.cc](../../src/enqueue.cc#L166) | 24 | 中文精读：工作 FIFO 的生产者与槽位复用 |
| [src/graph/connect.cc](../../src/graph/connect.cc#L17) | 5 | 中文精读：先构造机内片段，再交换跨机拼接信息 |
| [src/graph/paths.cc](../../src/graph/paths.cc#L34) | 5 | 中文精读：硬件图上的路径预计算 |
| [src/graph/rings.cc](../../src/graph/rings.cc#L22) | 1 | 中文精读：最终环的完整性检查 |
| [src/graph/search.cc](../../src/graph/search.cc#L65) | 5 | 中文精读：搜索阶段如何扣减链路带宽 |
| [src/graph/topo.cc](../../src/graph/topo.cc#L595) | 1 | 中文导读： |
| [src/graph/topo.h](../../src/graph/topo.h#L99) | 2 | 中文精读：硬件节点、边与缓存路径 |
| [src/graph/trees.cc](../../src/graph/trees.cc#L31) | 2 | 中文精读：用 rank 位模式生成二叉树 |
| [src/graph/tuning.cc](../../src/graph/tuning.cc#L77) | 4 | 中文导读： |
| [src/graph/xml.cc](../../src/graph/xml.cc#L14) | 1 | 中文精读：从系统发现信息到图搜索输入 |
| [src/graph/xml.h](../../src/graph/xml.h#L26) | 1 | 中文精读：硬件 XML 与通信图 XML 的区别 |
| [src/group.cc](../../src/group.cc#L96) | 6 | 中文导读： |
| [src/include/alloc.h](../../src/include/alloc.h#L19) | 2 | 中文精读：三类常见内存的区别 |
| [src/include/channel.h](../../src/include/channel.h#L13) | 2 | 中文导读： |
| [src/include/checks.h](../../src/include/checks.h#L12) | 1 | 中文精读：阅读错误处理宏时要展开控制流 |
| [src/include/collectives.h](../../src/include/collectives.h#L21) | 1 | 中文精读：主机和设备必须一致的函数表下标 |
| [src/include/comm.h](../../src/include/comm.h#L34) | 5 | 中文导读： |
| [src/include/core.h](../../src/include/core.h#L17) | 1 | 中文精读：公开符号与实现函数的关系 |
| [src/include/devcomm.h](../../src/include/devcomm.h#L30) | 14 | 中文精读：两种循环队列，不要混淆 |
| [src/include/enqueue.h](../../src/include/enqueue.h#L43) | 2 | 中文精读：一项操作为什么保存两份执行描述 |
| [src/include/graph.h](../../src/include/graph.h#L65) | 2 | 中文导读： |
| [src/include/info.h](../../src/include/info.h#L28) | 2 | 中文导读： |
| [src/include/net.h](../../src/include/net.h#L20) | 1 | 中文导读： |
| [src/include/param.h](../../src/include/param.h#L18) | 1 | 中文精读：NCCL_ 环境参数怎样变成 C++ 函数 |
| [src/include/proxy.h](../../src/include/proxy.h#L23) | 6 | 中文导读： |
| [src/include/transport.h](../../src/include/transport.h#L30) | 3 | 中文精读：初始化时交换的参与者摘要 |
| [src/include/utils.h](../../src/include/utils.h#L49) | 1 | 中文精读：排队链表为何 recycle 而非每轮 free |
| [src/init.cc](../../src/init.cc#L187) | 19 | 中文导读： |
| [src/misc/argcheck.cc](../../src/misc/argcheck.cc#L36) | 2 | 中文导读： |
| [src/misc/gdrwrap.cc](../../src/misc/gdrwrap.cc#L52) | 1 | 中文精读：GDRCopy 与 GPUDirect RDMA 不是同一接口 |
| [src/misc/ibvwrap.cc](../../src/misc/ibvwrap.cc#L44) | 2 | 中文导读： |
| [src/misc/nvmlwrap.cc](../../src/misc/nvmlwrap.cc#L48) | 1 | 中文精读：NVML 在 NCCL 中负责发现硬件 |
| [src/misc/param.cc](../../src/misc/param.cc#L51) | 1 | 中文精读：配置文件与环境变量的优先级 |
| [src/misc/profiler.cc](../../src/misc/profiler.cc#L7) | 1 | 中文精读：内置 Proxy profiling 的观察范围 |
| [src/misc/shmutils.cc](../../src/misc/shmutils.cc#L55) | 1 | 中文精读：同一共享内存在 CPU/GPU 上的地址视图 |
| [src/misc/socket.cc](../../src/misc/socket.cc#L489) | 1 | 中文精读：TCP 是字节流，必须保存部分完成的 offset |
| [src/net.cc](../../src/net.cc#L110) | 2 | 中文精读：插件是网络函数表的动态替换 |
| [src/proxy.cc](../../src/proxy.cc#L278) | 7 | 中文精读：CPU 怎样唤醒数据面代理线程 |
| [src/transport.cc](../../src/transport.cc#L26) | 4 | 中文导读： |
| [src/transport/coll_net.cc](../../src/transport/coll_net.cc#L304) | 3 | 中文精读：CollNet 连接共享的作用域 |
| [src/transport/net.cc](../../src/transport/net.cc#L165) | 12 | 中文导读： |
| [src/transport/net_ib.cc](../../src/transport/net_ib.cc#L33) | 20 | 中文导读： |
| [src/transport/net_socket.cc](../../src/transport/net_socket.cc#L483) | 2 | 中文精读：Socket 后端也实现异步 request/test 契约 |
| [src/transport/p2p.cc](../../src/transport/p2p.cc#L11) | 3 | 中文精读：GPU 间共享内存地址如何跨进程表达 |
| [src/transport/shm.cc](../../src/transport/shm.cc#L37) | 2 | 中文导读： |

## 验证范围

- 76 个源码文件，261 个中文注释块，新增 1384 行源码注释，共 54135 字符（含注释标记）。
- 逐文件删除本分支新增的中文块注释后，与本地 Git 基线逐字节相同。原语、宏、结构体布局与执行语句没有改动。
- 补丁还需通过干净基线重放、文件完整性与相对链接检查；结果见 [验证记录](verification.md)。
- 当前环境没有 nvcc，未执行 CUDA 编译、GPU 正确性或多机性能测试。示例仅用于解释下标与调用流程。

配套长文：[整体架构](README.md)、[初始化与执行](execution.md)、[Proxy 与 RDMA](rdma.md)。
