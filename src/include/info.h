/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_INFO_H_
/* 中文逐项-常量 X0600：NCCL_INFO_H_
 * 编译期定义：NCCL_INFO_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_INFO_H_

#include "nccl.h"
#include "devcomm.h"
#include "collectives.h"

typedef /* 中文逐项-类型 T0300：ncclPattern_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum : uint8_t {
/* 中文逐项-枚举项 X0601：ncclPatternRing
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternRing,
/* 中文逐项-枚举项 X0602：ncclPatternRingTwice
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternRingTwice,
/* 中文逐项-枚举项 X0603：ncclPatternPipelineFrom
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternPipelineFrom,
/* 中文逐项-枚举项 X0604：ncclPatternPipelineTo
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternPipelineTo,
/* 中文逐项-枚举项 X0605：ncclPatternTreeUp
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternTreeUp,
/* 中文逐项-枚举项 X0606：ncclPatternTreeDown
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternTreeDown,
/* 中文逐项-枚举项 X0607：ncclPatternTreeUpDown
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternTreeUpDown,
/* 中文逐项-枚举项 X0608：ncclPatternCollTreeUpDown
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternCollTreeUpDown,
/* 中文逐项-枚举项 X0609：ncclPatternSend
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternSend,
/* 中文逐项-枚举项 X0610：ncclPatternRecv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclPatternRecv
} ncclPattern_t;

// Used to pass NCCL call information between functions
/* 中文导读：
 * 一次公开 API 调用在主机端的中间表示。前半部保存调用参数，后半部由调度器补齐。
 * 随后转换成 GPU 的 ncclWorkElem 与 CPU 网络推进的 ncclProxyOp，实现同一操作的双侧协同。
 */
/* 中文逐项-类型 T0301：ncclInfo
 * 类型职责：主机一次API调用的中间表示，先保存用户参数再补充算法、协议、通道、线程和分片推导值。
 * 声明内可见成员（含嵌套成员）：coll、opName、sendbuff、recvbuff、count、datatype、op、root、comm、stream、chunkSteps、sliceSteps、opFull、algorithm、protocol、pattern、nChannels、nThreads、nBytes、nstepsPerLoop、nchunksPerLoop、chunkSize、channelId。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclInfo {
/* 中文逐项-变量 V2672：coll
 * coll（ncclFunc_t；成员 ncclInfo）：本任务的公开操作类别，如 AllReduce、Broadcast、Send 或 Recv。
 */
  ncclFunc_t coll;
/* 中文逐项-变量 V2673：opName
 * opName（char；成员 ncclInfo）：日志/错误报告使用的公开操作名字，不参与实际设备函数选择。
 */
  const char* opName;
  // NCCL Coll Args
/* 中文逐项-变量 V2674：sendbuff
 * sendbuff（void；成员 ncclInfo）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 */
  const void* sendbuff;
/* 中文逐项-变量 V2675：recvbuff
 * recvbuff（void；成员 ncclInfo）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 */
  void* recvbuff;
/* 中文逐项-变量 V2676：count
 * count（size_t；成员 ncclInfo）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 */
  size_t count;
/* 中文逐项-变量 V2677：datatype
 * datatype（ncclDataType_t；成员 ncclInfo）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 */
  ncclDataType_t datatype;
/* 中文逐项-变量 V2678：op
 * op（ncclRedOp_t；成员 ncclInfo）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 */
  ncclRedOp_t op;
/* 中文逐项-变量 V2679：root
 * root（int；成员 ncclInfo）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 */
  int root; // peer for p2p operations
/* 中文逐项-变量 V2680：comm
 * comm（ncclComm_t；成员 ncclInfo）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  ncclComm_t comm;
/* 中文逐项-变量 V2681：stream
 * stream（cudaStream_t；成员 ncclInfo）：CUDA 排队顺序上下文，用户流与内部流通过事件建立先后依赖。
 */
  cudaStream_t stream;
  // Algorithm details
/* 中文逐项-变量 V2682：chunkSteps
 * chunkSteps（int；成员 ncclInfo）：一个算法块/原语片段占用的 FIFO step 数，用来对齐 CPU Proxy 与 GPU 进度。
 */
  int chunkSteps;
/* 中文逐项-变量 V2683：sliceSteps
 * sliceSteps（int；成员 ncclInfo）：一个算法块/原语片段占用的 FIFO step 数，用来对齐 CPU Proxy 与 GPU 进度。
 */
  int sliceSteps;
/* 中文精读：输入、推导值与单位
 * count/datatype 可能被 ArgsCheck 归一化；nBytes 是模型用的整体字节规模。
 * algorithm/protocol/pattern 是选定的执行策略，nChannels/nThreads 是本次操作的并行度。
 * nstepsPerLoop/nchunksPerLoop 描述算法一轮，chunkSteps/sliceSteps 描述协议缓冲推进粒度。
 * channelId 主要由 P2P 调度指定；collective 则为每个 bid 在实际入 FIFO 时选择 channelId。
 */
  // Computed later
/* 中文逐项-变量 V2684：opFull
 * opFull（ncclDevRedOpFull；成员 ncclInfo）：解析公开句柄后的设备归约操作、标量位模式和是否为设备指针的完整信息。
 */
  ncclDevRedOpFull opFull;
/* 中文逐项-变量 V2685：algorithm
 * algorithm（int；成员 ncclInfo）：当前集合算法 Ring、Tree 或 CollNet；决定邻居与数据交换次序。
 */
  int algorithm;
/* 中文逐项-变量 V2686：protocol
 * protocol（int；成员 ncclInfo）：当前数据编码/同步协议枚举 LL、LL128 或 Simple；决定有效载荷比例和推进方法。
 */
  int protocol;
/* 中文逐项-变量 V2687：pattern
 * pattern（ncclPattern_t；成员 ncclInfo）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
  ncclPattern_t pattern;
/* 中文逐项-变量 V2688：nChannels
 * nChannels（int；成员 ncclInfo）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 */
  int nChannels;
/* 中文逐项-变量 V2689：nThreads
 * nThreads（int；成员 ncclInfo）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 */
  int nThreads;
/* 中文逐项-变量 V2690：nBytes
 * nBytes（size_t；成员 ncclInfo）：当前消息或片段的字节规模；info.nBytes 对 AG/RS 还计入 nRanks 段总规模。
 */
  size_t nBytes;
/* 中文逐项-变量 V2691：nstepsPerLoop
 * nstepsPerLoop（int；成员 ncclInfo）：一次算法轮的通信步数和不同逻辑块数；还需乘 chunkSteps 才得到 FIFO 进度。
 */
  int nstepsPerLoop;
/* 中文逐项-变量 V2692：nchunksPerLoop
 * nchunksPerLoop（int；成员 ncclInfo）：一次算法轮的通信步数和不同逻辑块数；还需乘 chunkSteps 才得到 FIFO 进度。
 */
  int nchunksPerLoop;
/* 中文逐项-变量 V2693：chunkSize
 * chunkSize（int；成员 ncclInfo）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 */
  int chunkSize;
/* 中文逐项-变量 V2694：channelId
 * channelId（int；成员 ncclInfo）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 */
  int channelId;
};

#endif
