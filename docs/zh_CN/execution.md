# 初始化、调度与 GPU 执行

[返回总览](README.md) · [Proxy 与 RDMA](rdma.md)

分析基线：`04832308d0885226169b9376c1ba4f17b0cb6484`。以下调用链区分普通 collective、group、P2P 和 CUDA Graph 分支；“API 返回”“kernel 已提交”“网络完成”“GPU 可消费数据”是不同事件。

## 1. communicator 初始化的完整主线

主要入口在 [init.cc](../../src/init.cc)。普通单 rank 初始化可按以下层次阅读：

```text
ncclCommInitRank
  ncclCommInitRankDev
    ncclInit
    初始化 CUDA runtime / 参数检查
    ncclCommInitRankSync
      commAlloc
      initTransportsRank
      devCommSetup
```

处于 group 时，`ncclCommInitRankDev` 调用 `ncclAsyncInit` 保存初始化任务；最外层 `ncclGroupEnd` 启动初始化线程并等待它们结束。这里的“异步”是在 group 内推迟执行，不是现代 nonblocking communicator API 的替代名称。

`ncclCommInitAll` 自己获取 unique ID、开始 group、遍历设备调用初始化、再结束 group。因此同进程多 GPU 可以一起参与握手，不会串行卡在第一个 GPU 等待其余参与者。

## 2. unique ID 与 bootstrap 为什么先于 RDMA

见 [bootstrap.cc](../../src/bootstrap.cc)。

RDMA 建链也需要先知道对端 QPN、地址与相关参数；在这些连接尚未建立前，必须先有一条能交换元数据的基础通路。

1. `ncclGetUniqueId` 获取用于 bootstrap 的 ID。
2. 应用把同一个 ID 分发给通信组中的 rank。
3. `bootstrapInit` 通过 root 联系各 rank，建立初始化 socket 通道。
4. 后续通过 bootstrap AllGather 和点对点 send/recv 交换设备属性、图结果与 transport 连接句柄。
5. 数据 transport 建好后，用户 collective 走对应 GPU/网络数据通路。

这里的 bootstrap AllGather 是 CPU 初始化元数据交换，与公开 `ncclAllGather` 的 GPU 数据操作不同。不能从初始化时出现 AllGather 推断模型正在执行一次专家通信。

unique ID 也不是硬件 QP 的标识；一组 rank 使用同一个 unique ID 后，可以建立许多 channel、peer 及底层连接。

## 3. commAlloc 分配什么

`commAlloc` 建立 communicator 基本状态，包括：

- rank、GPU index、PCI busId。
- CUDA events 和设备可见 abortFlag。
- group 暂存操作、P2P 队列和 QueueInfo。
- connectSend/connectRecv 位图。
- stream、错误与图辅助状态。
- 将 channel 标记为未初始化。

此时还没有完成硬件图搜索或为所有可能 peer 建链。静态数组有 32 个 channel 槽位，不意味着全部创建并使用。

## 4. initTransportsRank 的阶段分解

| 阶段 | 关键调用 | 产生的结果 |
| --- | --- | --- |
| 初始联络 | bootstrapInit | bootstrap 控制连接 |
| peer 元数据 | fillInfo、bootstrapAllGather | 各 rank 的主机/进程/GPU 属性 |
| 硬件建图 | ncclTopoGetSystem | GPU、CPU、PCI、NVS、NIC/NET 图 |
| 路径分析 | ncclTopoComputePaths、TrimSystem、再次 ComputePaths | 有效设备子图和可用路径 |
| 本地代理 | ncclProxyCreate | service 线程 |
| 通信图搜索 | ncclTopoCompute | Ring、Balanced Tree、CollNet 候选图 |
| 全局协调 | bootstrap 元数据交换、TopoPreset/Postset | 全局 rank 映射与邻居关系 |
| collective 建链 | ncclTransportP2pConnect/Setup | Ring/Tree 对应连接 |
| 性能模型 | ncclTopoTuneModel | 算法/协议延迟、带宽及阈值 |
| P2P 布局 | ncclTopoComputeP2pChannels | 总通道数、每 peer 通道数和偏移 |
| 可选预连接 | NVB/PXN 相关分支 | 部分本地中间路径/共享池准备 |
| 设备视图 | 外层 devCommSetup | GPU 可执行对象 |

函数开头旧注释写了“AllGathers”的概括，实际阅读应沿该提交中的所有 bootstrap 交换逐段看，不能只按注释数量认定握手次数。

peerInfo 中的 hostHash 与 pidHash 帮助区分同主机、同进程。相同主机且相同 busId 的不同 rank 会触发重复 GPU 检查。

## 5. 硬件路径怎样影响选择

[paths.cc](../../src/graph/paths.cc) 把路径归类为 LOC、NVL、NVB、PIX、PXB、PXN、PHB、SYS 等：

| 路径 | 阅读含义 |
| --- | --- |
| LOC | 本地自身 |
| NVL | NVLink 路径 |
| NVB | 经过中间 GPU 的 NVLink 路径 |
| PIX/PXB | 单个/多个 PCIe bridge 层次 |
| PXN | GPU 经中间 GPU 使用网络设备 |
| PHB | 经 PCIe host bridge |
| SYS | 经 CPU/NUMA 互联 |

这些类别既影响可达性也影响策略。即使 GPU 和 NIC 物理上能通信，NCCL 还会考虑 GDR 支持、读写方向、环境限制和拓扑距离。

PXN 不能只理解为“关闭一些 GPU 网口”。它涉及实际出口 GPU、代理 rank、缓冲映射及共享连接条件。对你设计的集成 GPU/RDMA 芯片，还需让 provider 与拓扑发现正确表达设备关系；源码不会仅根据“集成在同一芯片”的口头设定自动生成这种关系。

## 6. Ring/Tree 搜索与全局拼接

[search.cc](../../src/graph/search.cc) 在本机硬件图上搜索 channel 内 GPU 次序及网络端点，使用路径带宽、跳数、跨 NIC 等约束，并限制搜索预算。可从 XML 载入图。

[connect.cc](../../src/graph/connect.cc) 的两步值得单独理解：

- `ncclTopoPreset`：把本地搜索结果转成待交换的 rank 邻居信息。
- `ncclTopoPostset`：结合其他节点信息连成全局 Ring/Tree，应用 channel 数复制和上下限等规则。

初始化使用 `NCCL_TOPO_PATTERN_BALANCED_TREE` 搜索 Tree。在本版本中，BALANCED_TREE/SPLIT_TREE/TREE 描述跨节点树流量如何落在节点内 GPU 上；设备端 `runTreeSplit` 则描述线程组拆分，两者不应仅凭“Split”字样混同。

Ring 中 `prev/next` 是实际邻居，`userRanks/devUserRanks` 帮助把内部环顺序映射回公开 API 所要求的 rank 布局。拓扑优化可以改变内部次序，不能改变 AllGather 输出按用户 rank 排列的语义。

## 7. transport 如何选，以及何时真正建链

见 [transport.cc](../../src/transport.cc)。

`ncclTransportP2pConnect` 的名字容易误导：它主要把待建连接写入位图。它并不承诺使用 GPU P2P，也不当场为每条边创建 QP。

后续 `ncclTransportP2pSetup` 按位图处理：

1. 对每个 channel/peer/方向调用 `selectTransport`。
2. 按 P2P、SHM、NET 等候选顺序执行 canConnect。
3. 对选中 transport 执行 setup，产生连接句柄。
4. 通过 bootstrap 交换句柄。
5. 调用 connect 完成本地映射或网络连接。
6. 标记 connected，将 connector 更新到设备 peer 表。

同机 P2P 映射在 [p2p.cc](../../src/transport/p2p.cc) 中处理。跨进程用 CUDA IPC handle 打开本地映射，不能直接把对端进程的裸虚拟地址当成自己的地址使用。SHM 则使用共享主机内存机制。

Ring 和 Tree 使用同一个 channel/peer/方向/索引 0 槽位时，已连接对象会被跳过，因此并非每搜索出一种算法就必然再复制完全独立的一套相同边。

## 8. 普通 AllReduce 从 API 到 kernel 的调用链

见 [all_reduce.cc](../../src/collectives/all_reduce.cc) 和 [enqueue.cc](../../src/enqueue.cc)。

```text
ncclAllReduce
  构造 ncclInfo
  ncclEnqueueCheck
    ArgsCheck
    hostToDevRedOp
    checkSetStream
    ncclGetCudaGraph
    ncclSetupCollKernel
      computeColl
        getAlgoInfo
        getPatternInfo / getLoopInfo
        填 ncclWorkElem / ncclProxyOp
    ncclEnqueueHostSetup<0>       普通非 Graph 分支
      ncclEnqueueCollKernel
        选 channel，写工作 FIFO
        ncclProxySaveColl
      setupLaunch
      ncclLaunchProxy
    ncclLaunchBarrier
    ncclLaunchKernel
    ncclRecordEvents
    ncclLaunchReset
```

这里是普通非 group、非 CUDA Graph 的主线。group 保存操作后从 GroupEnd 统一提交；CUDA Graph 把相应 host setup 节点纳入图，不能把所有分支描述为立即调用同一段代码。

`ncclLaunchBarrier` 及 launch mode 还涉及同进程多个 GPU 的协调与 cooperative launch。并非所有配置都会在 `ncclLaunchKernel` 内走完全相同的一次 CUDA 调用。

## 9. 算法和协议怎样选

`ncclTopoTuneModel` 在初始化时建立模型，`getAlgoInfo` 在操作时枚举候选组合，调用 `ncclTopoGetAlgoTime` 估算耗时并选最小值。

核心估算形式为：

```text
time = latency * latencyCount + nBytes / (1000 * bandwidth)
```

实际还有 Tree 的消息大小修正、Ring 多节点大包延迟修正、操作聚合与已有 channel 数等因素。bandwidth=0 的组合被视为不可用。

选择算法/协议后，getAlgoInfo 继续根据消息大小缩减 channel 和线程数。因此：

- communicator 有 16 个 collective channel，不代表所有消息都启用 16 个。
- 模型能枚举 Tree/Ring/CollNet，不代表每个 collective 都实现所有组合。
- 可以用 NCCL_ALGO/NCCL_PROTO 筛选，但不能把不可用组合凭环境变量变成有效实现。
- 小包倾向低延迟、大包倾向高吞吐只是理解趋势，不是此版本代码里唯一固定的分界公式。

本版本设备 AllGather 与 ReduceScatter 的实现重点是 Ring；AllReduce 提供 Ring、Tree 和相应 CollNet 路径。

## 10. step、slice、chunk、loop 怎样联系

见 `computeColl` 和 `prims_simple.h`。

| 单位 | 作用 |
| --- | --- |
| step | 协议循环缓冲区的进度单位 |
| slice | 一次原语推进的数据段，可跨 sliceSteps 个 step |
| chunk | 算法层分块，可跨 chunkSteps 个 step |
| loop | 覆盖一批 channel/rank 分块的一轮算法处理 |

本版本 `NCCL_STEPS=8`。在主机侧，基础 stepSize 为协议缓冲大小除以 8。LL/LL128 因为携带 flag，有效数据大小还要打折；不能直接按原始 buffer 字节数估算有效 payload。

对 Ring：

- AllGather/ReduceScatter：一轮的网络传递次数模型为 nRanks−1。
- AllReduce：一轮的网络传递次数模型为 2×(nRanks−1)。
- 总 Proxy nsteps 还结合消息需要多少轮、chunkSteps 等计算。

设备代码中会有额外的本地读取/最终复制动作，因此不能机械地把源文件里原语调用行数当成网络传输轮数。

## 11. FIFO 怎样把主机任务交给 GPU

`ncclEnqueueCollKernel` 为本次操作选定若干 channel，把 bid 和任务元素写入各 channel FIFO，同时保存 ProxyOp。

聚合时，如果前一条 work 的 funcIndex、线程配置和剩余容量允许，可以把更多元素装在同一条 512 字节记录中。这样减少描述和发射开销，但不改变应用操作语义。

`setupLaunch` 设置发射参数并标记本批最后记录。设备端 [common.h](../../src/collectives/device/common.h) 的 `ncclKernel`：

1. 根据 blockIdx.x 选 channel。
2. 把 communicator/channel 信息载入 shared memory。
3. 特殊情况下直接使用作为参数传入的首工作元素，减少首项延迟。
4. 从 work FIFO 取记录并释放相应主机 FIFO 槽位。
5. 按 funcIndex 执行模板 RunWork 或设备函数表。
6. 碰到 isLast 或中止条件退出。

FIFO 记录已被读入 GPU 与用户通信已经完成是不同概念。前者允许 CPU 复用工作描述槽，后者还需要实际数据计算和网络操作结束。

## 12. Ring AllReduce 的设备实现逐段解释

见 [all_reduce.h](../../src/collectives/device/all_reduce.h) 的 `runRing`。

以一个 channel、4 个 rank 和输入分成 A/B/C/D 四块为例。每个 rank 都有 A/B/C/D 的一份本地输入。不同 rank 从不同块开始沿环发送，使各条边形成流水线。

| 设备阶段 | 原语 | 对数据做什么 |
| --- | --- | --- |
| 起始发送 | send | 把一个本地输入块传给 next |
| 向前归约 | recvReduceSend | 收到前驱部分和，加上本地该块，再传给 next |
| 完成某块归约 | directRecvReduceCopySend | 加入最后贡献，把完整结果写本地输出并继续发送 |
| 传播完整结果 | directRecvCopySend | 收到已完成的块，保存后继续转发 |
| 收尾 | directRecv | 收到最后缺少的结果块 |

对 4 rank，每块获得完整归约需要沿环传递 3 轮，随后完整结果再传播 3 轮。实现把阶段交界处的归约、保存及向前发送融合，不能理解为先调用独立 ncclReduceScatter API，再调用独立 ncclAllGather API。

若各 rank 输入为：

| rank | A | B | C | D |
| --- | ---: | ---: | ---: | ---: |
| 0 | 1 | 2 | 3 | 4 |
| 1 | 10 | 20 | 30 | 40 |
| 2 | 100 | 200 | 300 | 400 |
| 3 | 1000 | 2000 | 3000 | 4000 |

求和后的每个 rank 输出都是 [1111, 2222, 3333, 4444]。中途各 rank 负责完成哪一个块由内部 ring index 决定，最终公开结果不依赖这项安排。

每 rank 的理想环发送量约为 `2*(N-1)/N * inputBytes`，接收量同阶。这是算法 payload 模型，不包含协议 flag、对齐和控制流量，也不直接给出连接数。

## 13. Tree AllReduce 的两种设备执行组织

`runTreeUpDown` 分成向上归约和向下广播。叶子发送本地输入；中间节点合并子节点与自己数据后继续上送；根形成结果，再向下传播。

`runTreeSplit` 把部分线程分给上行归约、部分线程分给下行传播，让同一个 block 可以更好地流水推进。根和非根使用不同原语组合。

两者使用初始化准备的 tree.up/down，不会在 GPU 运行时重新查询系统拓扑或搜索父节点。设备端融合归约能避免把数据搬回 CPU 逐元素求和。

## 14. Simple 协议怎样完成一次收发/归约

见 [prims_simple.h](../../src/collectives/device/prims_simple.h)。

`send`、`recvReduceSend` 等友好命名最终落入模板化 `genericOp`。模板参数控制是否 Recv/Send、是否访问本地 Input/Output、是否允许 direct 以及 fan-in/fan-out。

一次 slice 的主要工作是：

1. waitPeer：等待输入可用或发送位置有空位。
2. 为本次 slice 组装源和目标指针。
3. 多线程执行复制或融合归约/复制。
4. 按路径要求执行线程同步和内存 fence。
5. postPeer：发布新进度，使消费者读取或生产者复用空间。

direct 只是原语能够选择直接访问映射、输入或输出的一类能力。跨机路径中，directSend 名字不表示 GPU 在写 NIC doorbell；检查它使用的 connector、direct 标记与 Proxy 才能确定真实执行路径。

## 15. LL 与 LL128 为什么适合不同场景

见 [devcomm.h](../../src/include/devcomm.h) 和 [prims_ll.h](../../src/collectives/device/prims_ll.h)、[prims_ll128.h](../../src/collectives/device/prims_ll128.h)。

| 协议 | 数据/同步组织 | 有效载荷比例 | 主要取舍 |
| --- | --- | --- | --- |
| Simple | 较大数据段，独立进度和控制字段 | 不使用 LL 这类每行 flag 编码 | 面向吞吐与较大分块 |
| LL | 16B 行中两组 32bit 数据与 32bit flag | 8/16=50% | 细粒度可用性轮询，额外带宽开销较大 |
| LL128 | 128B 行中 15 个 64bit 数据与 1 个 flag | 120/128=93.75% | 减少 flag 开销，仍有平台和可见性要求 |

接收方必须观察到正确的 flag 才认为该粒度有效。flag 放置、序号推进、清理和 RDMA 写入粒度都属于正确性设计，不能只把它看成“多加几个状态字”。

网络 Proxy 对这些协议也有专门就绪判断。例如 LL 需要检查 flag；LL128 主机缓冲情形不能简单地在 tail 变化后立即发送所有内容。

## 16. ncclSend/ncclRecv 和 group 的真实流程

见 [sendrecv.cc](../../src/collectives/sendrecv.cc)、[group.cc](../../src/group.cc)、[enqueue.cc](../../src/enqueue.cc)。

Send/Recv 内部各包一层 GroupStart/End，外部 group 可以继续嵌套。只有最外层 End 提交，适合把彼此依赖的多对收发一起排队。

```text
ncclSend / ncclRecv
  ncclEnqueueCheck
    ncclSaveP2p
      保存每 peer 的任务
      对未连接 channel 设置索引 1 建链位图
最外层 ncclGroupEnd
  ncclAsyncThreadPreconnect
    ncclTransportP2pSetup(..., connIndex=1)
  按节点距离与本地 rank 调度
  getP2pChunkSize
  scheduleRecv / scheduleSend
  ncclSetupP2pKernel
  HostSetup / launch
```

group 的节点调度和分块使互相匹配的收发能够推进，减少某些 channel 过载。单 peer 本次实际分配的 channel 数可因大小和整体负载低于上限。

[device/sendrecv.h](../../src/collectives/device/sendrecv.h) 把 P2P 工作分给不同 warp group，使用 Simple 协议和连接索引 1。向 self 的匹配通信可以执行本地复制。

显式 0 字节发送/接收被保留为同步操作，因此不能一律认为 count=0 就没有协议或连接行为。另一方面，没有调用对应 peer 的操作也不等于框架会自动补一个空发送；要看应用生成了哪些 API 调用。

## 17. “8 p2p channels，2 per peer”的精确读法

见 [paths.cc](../../src/graph/paths.cc) 和 [channel.h](../../src/include/channel.h)。

- `p2pnChannels=8`：P2P 调度可以使用编号 0…7 的 channel 空间。
- `p2pnChannelsPerPeer=2`：每个 peer 的 P2P 拆分/预连接通道预算。
- `p2pChannels`：经过位反转得到的偏移序列。
- `ncclChannelComputeBase`：由节点与 localRank 相对关系算基准。
- `ncclChannelComputeFromBase`：加偏移并对总数取模。

例如 8 通道时前两个偏移是 0、4。若某 peer 的 base=1，就会映射到 channel 1 和 5；另一个 peer 可映射到其他通道，也可以部分重叠。

因此不是“8 个全局通道之外，每个 peer 又额外创建 2 个全局通道”，也不是“每个 peer 必定创建 8×2 个连接”。但同一个 channel 面向不同 peer 的 connector 仍可分别建链，资源总数会随曾经访问的 peer 增长。

## 18. AllGather、ReduceScatter、Broadcast、Reduce 的差别

| 操作 | 输入/输出语义 | 主要执行特征 |
| --- | --- | --- |
| AllGather | 每 rank 提供一份贡献，每 rank 得到所有贡献 | 沿 Ring 传播，输出保持用户 rank 顺序 |
| ReduceScatter | 每 rank 提供所有分块，每 rank 只保留自己分块的归约结果 | Ring 归约并分发，不再传播所有结果 |
| AllReduce | 每 rank 输入相同形状，所有 rank 得到归约结果 | Ring 的 RS+AG 语义或 Tree/CollNet 路径 |
| Broadcast | root 提供数据，各 rank 得到同一内容 | 数据传播，无跨 rank 归约 |
| Reduce | 各 rank 提供输入，root 得到归约结果 | 结果只要求 root 有效 |
| Send/Recv | 两个 peer 之间传递数据 | group 调度、Simple、连接索引 1 |

不能把 “AllGather+ReduceScatter” 在任意模型后端的出现，直接等价为一次 NCCL AllReduce 的同一资源行为。API 调用顺序、communicator、驻留连接和算法可能不同。

## 19. CUDA Graph 与内存注册术语

`ncclGetCudaGraph` 检查捕获状态，`ncclCudaGraphHostSetup` 把相应 host 工作纳入 CUDA Graph，QueueInfo 等对象需要活到重放所需时刻。

`NCCL_GRAPH_REGISTER`、IPC buffer exchange 以及工作类型 RegColl 涉及图和特定直接缓冲路径。看到“Register”不能自动认定它等于 IB `ibv_reg_mr`。是否创建网络 MR，要沿 NET 后端的 regMr 调用继续查。

本导读描述普通协议缓冲路径；CUDA Graph、CollNet 或外部插件的特殊资源需要逐分支另计。

## 20. 返回、完成和销毁的时间关系

| 时刻 | 可以说明什么 | 不能直接说明什么 |
| --- | --- | --- |
| NCCL API 返回成功 | 本次主机路径已按 API 语义处理/提交 | GPU 和远端结果已经全部完成 |
| 最外层 GroupEnd 返回 | group 已按当前模式完成主机提交阶段 | 所有 CUDA 工作已经完成 |
| 某个网络 request done | 该后端请求的要求完成事件已收齐 | 所有 peer 的 collective 已结束 |
| GPU 通信 stream 完成 | 该流中对应已提交工作完成 | communicator 的持久 QP/MR 已释放 |
| communicator 销毁完成 | 其连接、缓冲和代理等走完相应释放路径 | 其他 communicator/进程的资源也释放 |

跨机 deadlock 调试时，应定位具体等待阶段：初始化 bootstrap、transport connect、GPU waitPeer、Proxy 等 GPU 就绪、IB 等接收方 FIFO、CQ 完成，还是 GPU 等消费/flush 发布。只看到 CPU 停在 API 或 GPU 利用率很高，无法区分这些情况。
