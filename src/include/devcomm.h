/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_DEVICE_H_
/* 中文逐项-常量 X0193：NCCL_DEVICE_H_
 * 编译期定义：NCCL_DEVICE_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_DEVICE_H_

#include "nccl.h"
#include "align.h"
#include <stdint.h>

/* 中文逐项-常量 X0194：NCCL_NUM_FUNCTIONS
 * 编译期定义：NCCL_NUM_FUNCTIONS = 5 // Send/Recv not included for now。
 * 设备函数表的API/算法/协议维度数量，改变时必须同步检查索引与表排列。
 */
#define NCCL_NUM_FUNCTIONS 5 // Send/Recv not included for now
typedef /* 中文逐项-类型 T0178：ncclFunc_t
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum { /* 中文逐项-枚举项 X0195：ncclFuncBroadcast
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclFuncBroadcast, /* 中文逐项-枚举项 X0196：ncclFuncReduce
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclFuncReduce, /* 中文逐项-枚举项 X0197：ncclFuncAllGather
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclFuncAllGather, /* 中文逐项-枚举项 X0198：ncclFuncReduceScatter
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclFuncReduceScatter, /* 中文逐项-枚举项 X0199：ncclFuncAllReduce
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclFuncAllReduce, /* 中文逐项-枚举项 X0200：ncclFuncSendRecv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclFuncSendRecv, /* 中文逐项-枚举项 X0201：ncclFuncSend
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclFuncSend, /* 中文逐项-枚举项 X0202：ncclFuncRecv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclFuncRecv, /* 中文逐项-枚举项 X0203：ncclNumFuncs
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ncclNumFuncs} ncclFunc_t;
/* 中文逐项-变量 V2121：ncclFuncStr
 * ncclFuncStr（char；文件/静态存储作用域）：内部操作/算法/协议枚举对应的可读名字表，供日志和配置匹配使用。
 */
extern const char* ncclFuncStr[NCCL_NUM_FUNCTIONS];

/* 中文逐项-常量 X0204：NCCL_NUM_ALGORITHMS
 * 编译期定义：NCCL_NUM_ALGORITHMS = 3 // Tree/Ring/CollNet。
 * 设备函数表的API/算法/协议维度数量，改变时必须同步检查索引与表排列。
 */
#define NCCL_NUM_ALGORITHMS 3 // Tree/Ring/CollNet
/* 中文逐项-常量 X0205：NCCL_ALGO_TREE
 * 编译期定义：NCCL_ALGO_TREE = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_ALGO_TREE 0
/* 中文逐项-常量 X0206：NCCL_ALGO_RING
 * 编译期定义：NCCL_ALGO_RING = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_ALGO_RING 1
/* 中文逐项-常量 X0207：NCCL_ALGO_COLLNET
 * 编译期定义：NCCL_ALGO_COLLNET = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_ALGO_COLLNET 2
/* 中文逐项-变量 V2122：ncclAlgoStr
 * ncclAlgoStr（char；文件/静态存储作用域）：内部操作/算法/协议枚举对应的可读名字表，供日志和配置匹配使用。
 */
extern const char* ncclAlgoStr[NCCL_NUM_ALGORITHMS];

/* 中文逐项-常量 X0208：NCCL_NUM_PROTOCOLS
 * 编译期定义：NCCL_NUM_PROTOCOLS = 3 // Simple/LL/LL128。
 * 设备函数表的API/算法/协议维度数量，改变时必须同步检查索引与表排列。
 */
#define NCCL_NUM_PROTOCOLS 3 // Simple/LL/LL128
/* 中文逐项-常量 X0209：NCCL_PROTO_LL
 * 编译期定义：NCCL_PROTO_LL = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROTO_LL 0
/* 中文逐项-常量 X0210：NCCL_PROTO_LL128
 * 编译期定义：NCCL_PROTO_LL128 = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROTO_LL128 1
/* 中文逐项-常量 X0211：NCCL_PROTO_SIMPLE
 * 编译期定义：NCCL_PROTO_SIMPLE = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROTO_SIMPLE 2
/* 中文逐项-变量 V2123：ncclProtoStr
 * ncclProtoStr（char；文件/静态存储作用域）：内部操作/算法/协议枚举对应的可读名字表，供日志和配置匹配使用。
 */
extern const char* ncclProtoStr[NCCL_NUM_PROTOCOLS];

/* 中文精读：两种循环队列，不要混淆
 * NCCL_MAX_OPS=2048 限制每 channel 的工作描述符 FIFO，存的是 ncclWork。
 * NCCL_STEPS=8 则切分每条连接的协议数据缓冲，承载真实通信数据并由 head/tail 控制复用。
 * 一个 work 可以触发许多数据 step；许多次操作也会继续复用同一批 8 个数据槽。
 * MAXCHANNELS=32 是本版编译期数组上限，实际值由拓扑与调度决定。
 */
/* 中文逐项-常量 X0212：NCCL_MAX_OPS
 * 编译期定义：NCCL_MAX_OPS = 2048。
 * 每channel工作描述FIFO最大条目数2048，CPU写尾和GPU读索引对它取模。
 */
#define NCCL_MAX_OPS 2048
/* 中文逐项-常量 X0213：NCCL_STEPS
 * 编译期定义：NCCL_STEPS = 8。
 * 每连接协议数据缓冲的固定循环槽数8；不是工作FIFO的2048条描述符容量。
 */
#define NCCL_STEPS 8

/* 中文精读：LL 行的数据与完成标记
 * 一行 16 字节，data1/data2 合计 8 字节有效载荷，flag1/flag2 合计 8 字节序号。
 * 接收者必须看到两个 flag 都匹配当前 step 才读取这一行，防止新旧数据混合。
 * flag 不是一个全局布尔 ready：槽位会复用，必须用进度序号区分不同轮次。
 * 因此 LL 的有效载荷效率是 1/2，主机 Proxy 计算 nLoops 时要相应扣除标记开销。
 */
/* 中文逐项-类型 T0179：ncclLLFifoLine
 * 类型职责：16字节LL协议行，8字节用户载荷加两个32位就绪标记，支持行级流水接收。
 * 声明内可见成员（含嵌套成员）：data1、flag1、data2、flag2、v、i4。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
union ncclLLFifoLine {
  /* Flags have to be *after* data, because otherwise, an incomplete receive
     from the network may receive the flag but not the data.
     Note this is assuming that either we receive contiguous chunks of data
     (sockets) or data is written with an atomicity of 8 bytes (IB/RDMA). */
/* 中文逐项-类型 T0180：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：data1、flag1、data2、flag2。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct {
/* 中文逐项-变量 V2124：data1
 * data1（uint32_t；成员 ncclLLFifoLine::<匿名>）：一个LL物理行中的两个32位用户载荷分量，配套flag1/flag2才表明数据完整。
 */
    uint32_t data1;
/* 中文逐项-变量 V2125：flag1
 * flag1（uint32_t；成员 ncclLLFifoLine::<匿名>）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 */
    uint32_t flag1;
/* 中文逐项-变量 V2126：data2
 * data2（uint32_t；成员 ncclLLFifoLine::<匿名>）：一个LL物理行中的两个32位用户载荷分量，配套flag1/flag2才表明数据完整。
 */
    uint32_t data2;
/* 中文逐项-变量 V2127：flag2
 * flag2（uint32_t；成员 ncclLLFifoLine::<匿名>）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 */
    uint32_t flag2;
  };
/* 中文逐项-变量 V2128：v
 * v（uint64_t；成员 ncclLLFifoLine）：LL行的两个64位或int4整行视图，用向量汇编同时读写数据和标记。
 */
  uint64_t v[2];
/* 中文逐项-变量 V2129：i4
 * i4（int4；成员 ncclLLFifoLine）：LL行的两个64位或int4整行视图，用向量汇编同时读写数据和标记。
 */
  int4 i4;
};

/* 中文逐项-常量 X0214：WARP_SIZE
 * 编译期定义：WARP_SIZE = 32。
 * 本版设备线程warp宽度32，用于lane/warp分组、同步和对齐公式。
 */
#define WARP_SIZE 32
/* 中文逐项-常量 X0215：MAXCHANNELS
 * 编译期定义：MAXCHANNELS = 32。
 * 本版communicator channel数组编译期上限32，实际使用数量仍由搜索和每次调度决定。
 */
#define MAXCHANNELS 32
/* 中文逐项-常量 X0216：NCCL_MAX_NTHREADS
 * 编译期定义：NCCL_MAX_NTHREADS = 640。
 * kernel/协议编译期最大线程预算，实际线程数可按消息规模缩小。
 */
#define NCCL_MAX_NTHREADS 640
/* 中文逐项-常量 X0217：NCCL_SIMPLE_MAX_NTHREADS
 * 编译期定义：NCCL_SIMPLE_MAX_NTHREADS = 512。
 * kernel/协议编译期最大线程预算，实际线程数可按消息规模缩小。
 */
#define NCCL_SIMPLE_MAX_NTHREADS 512
/* 中文逐项-常量 X0218：NCCL_LL_MAX_NTHREADS
 * 编译期定义：NCCL_LL_MAX_NTHREADS = 512。
 * kernel/协议编译期最大线程预算，实际线程数可按消息规模缩小。
 */
#define NCCL_LL_MAX_NTHREADS 512
/* 中文逐项-常量 X0219：NCCL_LL_LINES_PER_THREAD
 * 编译期定义：NCCL_LL_LINES_PER_THREAD = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_LL_LINES_PER_THREAD 8
#ifdef TEST_LL_CLEANUP
/* 中文逐项-常量 X0220：NCCL_LL_CLEAN_MASK
 * 编译期定义：NCCL_LL_CLEAN_MASK = 0x078 // Set to 0x100 to disable cleanup。
 * LL标记回绕和清理触发的位域/测试阈值，避免重复序号误识别旧数据。
 */
#define NCCL_LL_CLEAN_MASK 0x078 // Set to 0x100 to disable cleanup
/* 中文逐项-常量 X0221：NCCL_LL_FLAG_MAX
 * 编译期定义：NCCL_LL_FLAG_MAX = 0x100。
 * LL标记回绕和清理触发的位域/测试阈值，避免重复序号误识别旧数据。
 */
#define NCCL_LL_FLAG_MAX   0x100
/* 中文逐项-宏 M0087：NCCL_LL_FLAG
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCL_LL_FLAG；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_LL_FLAG(a) ((uint32_t)((a) % NCCL_LL_FLAG_MAX))
#else
/* 中文逐项-常量 X0222：NCCL_LL_CLEAN_MASK
 * 编译期定义：NCCL_LL_CLEAN_MASK = 0x7ffffff8。
 * LL标记回绕和清理触发的位域/测试阈值，避免重复序号误识别旧数据。
 */
#define NCCL_LL_CLEAN_MASK 0x7ffffff8
/* 中文逐项-宏 M0088：NCCL_LL_FLAG
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCL_LL_FLAG；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_LL_FLAG(a) ((uint32_t)(a))
#endif
// Make sure the clean mask will last for at least NCCL_NSTEPS
static_assert(NCCL_LL_CLEAN_MASK % NCCL_STEPS == 0, "Invalid NCCL_LL_CLEAN_MASK value");

/* 中文逐项-常量 X0223：NCCL_LL128_LINESIZE
 * 编译期定义：NCCL_LL128_LINESIZE = 128。
 * LL128物理行128字节，其中120字节为有效载荷、8字节为flag。
 */
#define NCCL_LL128_LINESIZE 128
/* 中文逐项-常量 X0224：NCCL_LL128_LINEELEMS
 * 编译期定义：NCCL_LL128_LINEELEMS = (NCCL_LL128_LINESIZE/sizeof(uint64_t))。
 * LL128行的64位字总数和有效数据字数，分别16与15，用于扣除flag开销。
 */
#define NCCL_LL128_LINEELEMS (NCCL_LL128_LINESIZE/sizeof(uint64_t))
/* 中文逐项-常量 X0225：NCCL_LL128_DATAELEMS
 * 编译期定义：NCCL_LL128_DATAELEMS = (NCCL_LL128_LINEELEMS-1)。
 * LL128行的64位字总数和有效数据字数，分别16与15，用于扣除flag开销。
 */
#define NCCL_LL128_DATAELEMS (NCCL_LL128_LINEELEMS-1)

/* 中文逐项-常量 X0226：NCCL_LL128_MAX_NTHREADS
 * 编译期定义：NCCL_LL128_MAX_NTHREADS = 640。
 * kernel/协议编译期最大线程预算，实际线程数可按消息规模缩小。
 */
#define NCCL_LL128_MAX_NTHREADS 640
/* 中文逐项-常量 X0227：NCCL_LL128_ELEMS_PER_THREAD
 * 编译期定义：NCCL_LL128_ELEMS_PER_THREAD = 120。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_LL128_ELEMS_PER_THREAD 120

/* 中文逐项-常量 X0228：NCCL_LL128_SHMEM_ELEMS_PER_THREAD
 * 编译期定义：NCCL_LL128_SHMEM_ELEMS_PER_THREAD = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_LL128_SHMEM_ELEMS_PER_THREAD 8
/* 中文逐项-常量 X0229：NCCL_LL128_SHMEM_SIZE
 * 编译期定义：NCCL_LL128_SHMEM_SIZE = (NCCL_LL128_SHMEM_ELEMS_PER_THREAD*NCCL_LL128_MAX_NTHREADS)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_LL128_SHMEM_SIZE (NCCL_LL128_SHMEM_ELEMS_PER_THREAD*NCCL_LL128_MAX_NTHREADS)

/* 中文逐项-常量 X0230：NCCL_DIRECT_WRITE
 * 编译期定义：NCCL_DIRECT_WRITE = 0x01。
 * 直接写、直接读、NIC或IPC访问的能力位，按连接和注册条件组合，不是公开算法枚举。
 */
#define NCCL_DIRECT_WRITE 0x01
/* 中文逐项-常量 X0231：NCCL_DIRECT_READ
 * 编译期定义：NCCL_DIRECT_READ = 0x02。
 * 直接写、直接读、NIC或IPC访问的能力位，按连接和注册条件组合，不是公开算法枚举。
 */
#define NCCL_DIRECT_READ  0x02
/* 中文逐项-常量 X0232：NCCL_DIRECT_NIC
 * 编译期定义：NCCL_DIRECT_NIC = 0x04。
 * 直接写、直接读、NIC或IPC访问的能力位，按连接和注册条件组合，不是公开算法枚举。
 */
#define NCCL_DIRECT_NIC   0x04
/* 中文逐项-常量 X0233：NCCL_IPC_WRITE
 * 编译期定义：NCCL_IPC_WRITE = 0x08。
 * 直接写、直接读、NIC或IPC访问的能力位，按连接和注册条件组合，不是公开算法枚举。
 */
#define NCCL_IPC_WRITE    0x08
/* 中文逐项-常量 X0234：NCCL_IPC_READ
 * 编译期定义：NCCL_IPC_READ = 0x10。
 * 直接写、直接读、NIC或IPC访问的能力位，按连接和注册条件组合，不是公开算法枚举。
 */
#define NCCL_IPC_READ     0x10

/* 中文导读：
 * GPU 原语实际使用的连接视图。buffs 按 LL/LL128/Simple 区分，head/tail 和 step 驱动槽位流控。
 * 此处没有 ibv_qp；网络 QP/MR 由主机端 NET/IB 后端持有，GPU 经这些共享字段与 Proxy 协作。
 */
/* 中文精读：图示绑定 V3.3 T0181 ncclConnInfo
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=T0181
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0117] ncclConnInfo：字段协作图（结构字段上下文）
 *   图：14-fifo, 19-direct
 *   定位：#symbol=T0181&binding=B0117
 */
/* 中文逐项-类型 T0181：ncclConnInfo
 * 类型职责：GPU原语使用的连接视图，记录协议缓冲、进度、共享偏移和直接访问能力。
 * 声明内可见成员（含嵌套成员）：buffs、tail、head、direct、shared、ptrExchange、redOpArgExchange、sizesFifo、offsFifo、step、llLastCleaning。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclConnInfo {
  // Regular comm mechanism
/* 中文逐项-变量 V2130：buffs
 * buffs（char；成员 ncclConnInfo）：按 LL/LL128/Simple 区分的协议缓冲基址集合，承载分片流水数据。
 */
  char *buffs[NCCL_NUM_PROTOCOLS]; // Local for recv, remote for send
/* 中文逐项-变量 V2131：tail
 * tail（uint64_t；成员 ncclConnInfo）：生产者已发布的数据进度，消费者据此判断相应片段是否可读。
 */
  uint64_t *tail;     // Local for recv, remote for send
/* 中文逐项-变量 V2132：head
 * head（uint64_t；成员 ncclConnInfo）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 */
  uint64_t *head;     // Local for send, remote for recv

/* 中文逐项-变量 V2133：direct
 * direct（int；成员 ncclConnInfo）：当前连接支持的直接用户缓冲/GPU-NIC访问能力位掩码，不能仅凭非零就认定GPU发起RDMA。
 */
  int direct;         // Direct communication
/* 中文逐项-变量 V2134：shared
 * shared（int；成员 ncclConnInfo）：是否使用共享缓冲资源的标志；不自动表示所有 channel 或全部 backend 连接合并。
 */
  int shared;         // Buffers are shared
/* 中文逐项-变量 V2135：ptrExchange
 * ptrExchange（void；成员 ncclConnInfo）：直接用户缓冲访问的指针交换区，不是所有 rank 的全局共享地址表。
 */
  void **ptrExchange; // Pointer exchange for direct communication
/* 中文逐项-变量 V2136：redOpArgExchange
 * redOpArgExchange（uint64_t；成员 ncclConnInfo）：直接拉取路径中交换预处理标量的控制区，保证远端输入使用正确缩放语义。
 */
  uint64_t* redOpArgExchange; // PreOp scaler exchange for direct pull case

/* 中文逐项-变量 V2137：sizesFifo
 * sizesFifo（int；成员 ncclConnInfo）：GPU 向代理报告各数据槽有效物理字节数的控制数组，空/未就绪值需按后端约定处理。
 */
  int *sizesFifo;     // Sizes fifo from GPU to proxy
/* 中文逐项-变量 V2138：offsFifo
 * offsFifo（int；成员 ncclConnInfo）：共享数据池每槽字节偏移，代理先提供地址，GPU 再按偏移读写本次槽位。
 */
  int *offsFifo;      // Buffer fifo from proxy to GPU

/* 中文逐项-变量 V2139：step
 * step（uint64_t；成员 ncclConnInfo）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 */
  uint64_t step;      // Keep where we are
/* 中文逐项-变量 V2140：llLastCleaning
 * llLastCleaning（uint64_t；成员 ncclConnInfo）：记录 LL 标记回绕清理进度，避免旧行标记被误认为新数据。
 */
  uint64_t llLastCleaning;
};

/* 中文逐项-类型 T0182：ncclProxyConnector
 * 类型职责：主线程引用实际代理连接所需的代理rank、本地编号和后台连接对象。
 * 声明内可见成员（含嵌套成员）：rank、localRank、connection、comm。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyConnector {
/* 中文逐项-变量 V2141：rank
 * rank（int；成员 ncclProxyConnector）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V2142：localRank
 * localRank（int；成员 ncclProxyConnector）：本节点内 rank 下标，主要索引本地共享资源。
 */
  int localRank;
/* 中文逐项-变量 V2143：connection
 * connection（struct ncclProxyConnection；成员 ncclProxyConnector）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 */
  struct ncclProxyConnection* connection;
/* 中文逐项-变量 V2144：comm
 * comm（struct ncclComm；成员 ncclProxyConnector）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  struct ncclComm* comm;
};

/* 中文导读：
 * 一个 channel 到一个 peer、一个方向、一个连接索引的逻辑连接。
 * transportComm 是主机端回调表，transportResources 是具体 transport 的私有对象，conn 是 GPU 使用的数据。
 * connected 标记建链完成，使相同逻辑连接可被后续操作复用。逻辑连接数不直接等于 QP 数。
 */
/* 中文逐项-类型 T0183：ncclConnector
 * 类型职责：一个channel到一个peer、一个方向、一个连接索引的逻辑连接，连接视图和主机后端资源在此关联。
 * 声明内可见成员（含嵌套成员）：connected、proxyConn、transportComm、transportResources、conn、comm。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclConnector {
/* 中文逐项-变量 V2145：connected
 * connected（int；成员 ncclConnector）：逻辑连接已完成建链的标记，后续操作可复用已建立资源。
 */
  int connected;
/* 中文逐项-变量 V2146：proxyConn
 * proxyConn（struct ncclProxyConnector；成员 ncclConnector）：逻辑连接到代理服务的引用，记录代理 rank 和实际后台连接。
 */
  struct ncclProxyConnector proxyConn;
/* 中文逐项-变量 V2147：transportComm
 * transportComm（struct ncclTransportComm；成员 ncclConnector）：所选 transport 的回调表，规定建链、释放与代理推进方式。
 */
  struct ncclTransportComm* transportComm;
/* 中文逐项-变量 V2148：transportResources
 * transportResources（void；成员 ncclConnector）：具体 transport 私有资源；先依据实际后端类型再读取其中成员。
 */
  void* transportResources;
/* 中文逐项-变量 V2149：conn
 * conn（struct ncclConnInfo；成员 ncclConnector）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 */
  struct ncclConnInfo conn;
/* 中文逐项-变量 V2150：comm
 * comm（struct ncclComm；成员 ncclConnector）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  struct ncclComm *comm;
};

/* 中文精读：设备算法中的两个 rank 坐标系
 * prev/next 是 communicator 的用户 rank，用于在 devPeers 中索引连接。
 * userRanks[0] 是自己，[1] 是下游，[nRanks-1] 是上游；数组从当前 rank 开始旋转。
 * index 是自己在共同环顺序中的内部位置，用来安排 AllReduce 各块的起始归约顺序。
 * AllGather/ReduceScatter 的输出段仍按用户 rank 排列，不会被拓扑优化打乱。
 */
/* 中文精读：图示绑定 V3.3 T0184 ncclRing
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=T0184
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0116] ncclRing：字段协作图（结构字段上下文）
 *   图：03-layout, 28-channel-generation
 *   定位：#symbol=T0184&binding=B0116
 */
/* 中文逐项-类型 T0184：ncclRing
 * 类型职责：环算法的直接邻居与内部到用户rank排列映射，兼顾物理路径和公开输出顺序。
 * 声明内可见成员（含嵌套成员）：prev、next、userRanks、devUserRanks、index。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclRing {
  // Shortcuts for userRanks[1] and userRanks[n-1]
/* 中文逐项-变量 V2151：prev
 * prev（int；成员 ncclRing）：环前后继或链表前后项，准确含义由所属 ring/队列类型决定。
 */
  int prev;
/* 中文逐项-变量 V2152：next
 * next（int；成员 ncclRing）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
  int next;

  // Maps an internal nccl index to user-specified rank order. This is necessary
  // since we need to know how the user expects data to be ordered across
  // devices. Ordered from current device.
/* 中文逐项-变量 V2153：userRanks
 * userRanks（int；成员 ncclRing）：从当前 rank 开头排列的环顺序表；保存用户 rank 以恢复公开 API 的分段顺序。
 */
  int* userRanks;
/* 中文逐项-变量 V2154：devUserRanks
 * devUserRanks（int；成员 ncclRing）：从当前 rank 开头排列的环顺序表；保存用户 rank 以恢复公开 API 的分段顺序。
 */
  int* devUserRanks;

/* 中文逐项-变量 V2155：index
 * index（int；成员 ncclRing）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 */
  int index; // This rank's index in the ring
};


/* 中文逐项-常量 X0235：NCCL_MAX_TREE_ARITY
 * 编译期定义：NCCL_MAX_TREE_ARITY = 3。
 * 树/CollNet直接原语的编译期邻居度容量，决定连接数组与展开上限。
 */
#define NCCL_MAX_TREE_ARITY 3
/* 中文精读：一条 channel 的树邻接关系
 * up 为父 rank，down 为孩子 rank，缺失位置用 -1；depth 为流水/分片调优提供深度信息。
 * 单 GPU rank 最多 3 个 down 槽，包含节点内部连接与节点间树拼接的需要。
 * 构造原语时 FanAsymmetric 限定编译期最大收发数，实际有效邻居数由 -1 终止的列表确定。
 * 树数组本身不代表已分配 QP；对应 connector 的 transport 才决定走本地还是网络。
 */
/* 中文逐项-类型 T0185：ncclTree
 * 类型职责：当前GPU的树父/孩子邻接关系与深度，-1表示没有该邻居。
 * 声明内可见成员（含嵌套成员）：depth、up、down。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclTree {
/* 中文逐项-变量 V2156：depth
 * depth（int；成员 ncclTree）：通信树估计深度，用于分片和流水调优。
 */
  int depth;
/* 中文逐项-变量 V2157：up
 * up（int；成员 ncclTree）：树父/孩子用户 rank；-1 表示对应邻居不存在。
 */
  int up;
/* 中文逐项-变量 V2158：down
 * down（int；成员 ncclTree）：树父/孩子用户 rank；-1 表示对应邻居不存在。
 */
  int down[NCCL_MAX_TREE_ARITY];
};

/* 中文逐项-常量 X0236：NCCL_MAX_DIRECT_ARITY
 * 编译期定义：NCCL_MAX_DIRECT_ARITY = 7。
 * 树/CollNet直接原语的编译期邻居度容量，决定连接数组与展开上限。
 */
#define NCCL_MAX_DIRECT_ARITY 7
/* 中文逐项-类型 T0186：ncclDirect
 * 类型职责：CollNet多head的机内分散/归约/广播/收集邻接关系，包含特殊网络端点。
 * 声明内可见成员（含嵌套成员）：depth、out、nHeads、headRank、shift、up、down。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclDirect {
/* 中文逐项-变量 V2159：depth
 * depth（int；成员 ncclDirect）：通信树估计深度，用于分片和流水调优。
 */
  int depth;
/* 中文逐项-变量 V2160：out
 * out（int；成员 ncclDirect）：CollNet 连接网络的特殊对端槽位，-1 表示当前 GPU 非对应网络出口。
 */
  int out;
/* 中文逐项-变量 V2161：nHeads
 * nHeads（int；成员 ncclDirect）：当前 CollNet 分工的网络 head 数，额外参与一轮数据块数量计算。
 */
  int nHeads;
/* 中文逐项-变量 V2162：headRank
 * headRank（int；成员 ncclDirect）：本 GPU 在 CollNet 多 head 分工中的编号，决定负责哪一片数据。
 */
  int headRank;
/* 中文逐项-变量 V2163：shift
 * shift（int；成员 ncclDirect）：在多 head/peer 间轮转分片次序的偏移，平衡并发访问。
 */
  int shift;
/* 中文逐项-变量 V2164：up
 * up（int；成员 ncclDirect）：树父/孩子用户 rank；-1 表示对应邻居不存在。
 */
  int up[NCCL_MAX_DIRECT_ARITY];
/* 中文逐项-变量 V2165：down
 * down（int；成员 ncclDirect）：树父/孩子用户 rank；-1 表示对应邻居不存在。
 */
  int down[NCCL_MAX_DIRECT_ARITY];
};

/* 中文逐项-常量 X0237：NCCL_MAX_CONNS
 * 编译期定义：NCCL_MAX_CONNS = 2。
 * 每peer每方向的连接槽数量2，用于区分集合路径与常规P2P路径。
 */
#define NCCL_MAX_CONNS 2
/* 中文导读：
 * channel 内按 rank 索引的对端对象；send/recv 分开，每个方向有 NCCL_MAX_CONNS 个槽位。
 * 本版本初始化 Ring/Tree 使用索引 0，普通 Send/Recv 路径使用索引 1；CollNet 也有专门用法。
 * 数组被分配不意味着每个槽位都已建立连接，更不意味着都使用 RDMA。
 */
/* 中文逐项-类型 T0187：ncclPeer
 * 类型职责：单个channel内某个对端rank的发送/接收连接槽集合，预留槽不是已创建QP。
 * 声明内可见成员（含嵌套成员）：send、recv。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclPeer {
/* 中文逐项-变量 V2166：send
 * send（struct ncclConnector；成员 ncclPeer）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 */
  struct ncclConnector send[NCCL_MAX_CONNS];
/* 中文逐项-变量 V2167：recv
 * recv（struct ncclConnector；成员 ncclPeer）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 */
  struct ncclConnector recv[NCCL_MAX_CONNS];
};

struct ncclDevComm;

/* ncclWork is to be a power of two, currently 8x64 bytes, */
/* to make sure reads to host from the CUDA kernel are aligned. */
/* Make sure to adjust padding at the end of ncclWorkElem. */
/* 中文逐项-常量 X0238：NCCL_WORK_SIZE
 * 编译期定义：NCCL_WORK_SIZE = 512。
 * 一条work描述的固定512字节布局，union中多个任务共用该存储容量。
 */
#define NCCL_WORK_SIZE 512

/* 中文逐项-类型 T0188：ncclWorkElemType
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclWorkElemType : uint8_t {
/* 中文逐项-枚举项 X0239：ncclWorkTypeUnused
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
   ncclWorkTypeUnused=0,
/* 中文逐项-枚举项 X0240：ncclWorkTypeColl
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
   ncclWorkTypeColl=1,
/* 中文逐项-枚举项 X0241：ncclWorkTypeP2p
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
   ncclWorkTypeP2p=2,
/* 中文逐项-枚举项 X0242：ncclWorkTypeRegColl
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
   ncclWorkTypeRegColl=3
};
/* 中文逐项-类型 T0189：ncclWorkElemSubType
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclWorkElemSubType : uint8_t {
/* 中文逐项-枚举项 X0243：ncclWorkSubTypeUnused
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
  ncclWorkSubTypeUnused =0,
/* 中文逐项-枚举项 X0244：ncclWorkSubTypeSend
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclWorkSubTypeSend,
/* 中文逐项-枚举项 X0245：ncclWorkSubTypeRecv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
  ncclWorkSubTypeRecv
};

/* 中文精读：GPU 如何知道这条任务要执行什么
 * type 区分 Unused/Collective/P2P/RegisteredCollective；funcIndex 编码具体设备函数。
 * nWarps 指定参与执行的线程数，isLast 表示本批任务到此结束。
 * header 与 union 中第一个元素重叠，使 kernel 可先看公共头，再按具体类型读取正确布局。
 * 此头部属于工作描述，不是发到网络上的报文头。
 */
/* 中文逐项-类型 T0190：ncclWorkElemHeader
 * 类型职责：设备工作公共头，决定类型、设备函数、参与warp数和本批结束位置。
 * 声明内可见成员（含嵌套成员）：funcIndex、type、nWarps、isLast。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclWorkElemHeader {
/* 中文逐项-变量 V2168：funcIndex
 * funcIndex（uint16_t；成员 ncclWorkElemHeader）：操作/归约/类型/算法/协议组合在线性设备函数表中的下标，主机与设备排列必须一致。
 */
  uint16_t funcIndex;
/* 中文逐项-变量 V2169：type
 * type（enum ncclWorkElemType；成员 ncclWorkElemHeader）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  enum ncclWorkElemType type;
/* 中文逐项-变量 V2170：nWarps
 * nWarps（unsigned；成员 ncclWorkElemHeader）：本工作元素分到的起始 warp 和 warp 数，决定哪些线程执行本项。
 */
  unsigned nWarps:5;
/* 中文逐项-变量 V2171：isLast
 * isLast（unsigned；成员 ncclWorkElemHeader）：本通道当前批次的最后一条任务标记，GPU 执行完此记录即可退出本次 kernel。
 */
  unsigned isLast:1;
};

/* 中文导读：
 * 主机调度器生成的集合通信工作描述：缓冲区、元素数量、channel 分片编号及执行函数索引。
 * count 是数据类型元素数，不能始终当成字节数；funcIndex 编码操作/归约/类型/算法/协议组合。
 */
/* 中文精读：图示绑定 V3.3 T0191 ncclWorkElem
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=T0191
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0118] ncclWorkElem：字段协作图（结构字段上下文）
 *   图：01-submit, 02-split-layers
 *   定位：#symbol=T0191&binding=B0118
 */
/* 中文逐项-类型 T0191：ncclWorkElem
 * 类型职责：一次collective在单channel上的设备任务描述，保存用户指针、元素数、分片号及归约参数。
 * 声明内可见成员（含嵌套成员）：header、regUsed、direct、redOpArgIsPtr、sendbuff、recvbuff、count、lastChunkSize、root、bid、nChannels、redOpArg、pad。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclWorkElem {
/* 中文逐项-变量 V2172：header
 * header（struct ncclWorkElemHeader；成员 ncclWorkElem）：work 公共头，包含任务类型、函数下标、线程数和本批终止标记。
 */
  struct ncclWorkElemHeader header;
/* 中文逐项-变量 V2173：regUsed
 * regUsed（uint8_t；成员 ncclWorkElem）：是否采用已注册用户缓冲优化，决定使用普通协议缓冲还是直接地址路径。
 */
  uint8_t regUsed;
/* 中文逐项-变量 V2174：direct
 * direct（uint8_t；成员 ncclWorkElem）：当前连接支持的直接用户缓冲/GPU-NIC访问能力位掩码，不能仅凭非零就认定GPU发起RDMA。
 */
  uint8_t direct;
/* 中文逐项-变量 V2175：redOpArgIsPtr
 * redOpArgIsPtr（uint8_t；成员 ncclWorkElem）：该标量是否保存设备地址；设备执行前须解引用才能用于预处理。
 */
  uint8_t redOpArgIsPtr;

/* 中文逐项-变量 V2176：sendbuff
 * sendbuff（void；成员 ncclWorkElem）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 */
  const void * sendbuff;
/* 中文逐项-变量 V2177：recvbuff
 * recvbuff（void；成员 ncclWorkElem）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 */
  void * recvbuff;

/* 中文逐项-变量 V2178：count
 * count（size_t；成员 ncclWorkElem）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 */
  size_t count;
/* 中文逐项-变量 V2179：lastChunkSize
 * lastChunkSize（size_t；成员 ncclWorkElem）：由主机计算交给设备的尾块/调优块尺寸，含义依算法协议分支而定。
 */
  size_t lastChunkSize;
/* 中文逐项-变量 V2180：root
 * root（uint32_t；成员 ncclWorkElem）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 */
  uint32_t root;
/* 中文逐项-变量 V2181：bid
 * bid（uint8_t；成员 ncclWorkElem）：当前任务内部的 channel 分片编号，参与张量地址计算；不是网卡或全局 rank 编号。
 */
  uint8_t bid;
/* 中文逐项-变量 V2182：nChannels
 * nChannels（uint8_t；成员 ncclWorkElem）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 */
  uint8_t nChannels;
/* 中文逐项-变量 V2183：redOpArg
 * redOpArg（uint64_t；成员 ncclWorkElem）：归约标量的位模式容器或设备指针，需结合对应 IsPtr 标志理解。
 */
  uint64_t redOpArg;
/* 中文逐项-变量 V2184：pad
 * pad（uint64_t；成员 ncclWorkElem）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
  uint64_t pad;
};
static_assert(NCCL_WORK_SIZE % sizeof(struct ncclWorkElem) == 0, "ncclWorkElem size must be a multiple of ncclWork size");

/* 中文精读：P2P 元素逐字段阅读
 * peer/buff/count 指定通信对端、本次块起点、块长度；本版本 P2P 设备任务按 Int8 即字节处理。
 * chunkSize 是设备端继续切片的字节上限，ngroups 是同条 work 的线程组数量。
 * warpStart/nWarps 指定本项的线程范围；subType 用于主机装填时区分 Send 和 Recv。
 * 一条 work 可能有多个 peer 的收发，因此一条 channel/block 不固定只服务一个 peer。
 */
/* 中文逐项-类型 T0192：ncclWorkElemP2p
 * 类型职责：一次P2P块的设备任务描述，含peer、字节范围和分配给它的线程组。
 * 声明内可见成员（含嵌套成员）：header、peer、buff、count、chunkSize、ngroups、warpStart、nWarps、subType。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclWorkElemP2p {
/* 中文逐项-变量 V2185：header
 * header（struct ncclWorkElemHeader；成员 ncclWorkElemP2p）：work 公共头，包含任务类型、函数下标、线程数和本批终止标记。
 */
  struct ncclWorkElemHeader header;
/* 中文逐项-变量 V2186：peer
 * peer（int32_t；成员 ncclWorkElemP2p）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 */
  int32_t peer;
/* 中文逐项-变量 V2187：buff
 * buff（void；成员 ncclWorkElemP2p）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 */
  void* buff;
/* 中文逐项-变量 V2188：count
 * count（size_t；成员 ncclWorkElemP2p）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 */
  size_t count;
/* 中文逐项-变量 V2189：chunkSize
 * chunkSize（int；成员 ncclWorkElemP2p）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 */
  int chunkSize;
/* 中文逐项-变量 V2190：ngroups
 * ngroups（uint8_t；成员 ncclWorkElemP2p）：同一 P2P work 内同时划分的收发线程组数，向上按二次幂组织。
 */
  uint8_t ngroups;
/* 中文逐项-变量 V2191：warpStart
 * warpStart（uint8_t；成员 ncclWorkElemP2p）：本工作元素分到的起始 warp 和 warp 数，决定哪些线程执行本项。
 */
  uint8_t warpStart;
/* 中文逐项-变量 V2192：nWarps
 * nWarps（uint8_t；成员 ncclWorkElemP2p）：本工作元素分到的起始 warp 和 warp 数，决定哪些线程执行本项。
 */
  uint8_t nWarps;
/* 中文逐项-变量 V2193：subType
 * subType（enum ncclWorkElemSubType；成员 ncclWorkElemP2p）：P2P 元素发送/接收方向类别，影响装填时的偶数接收/奇数发送布局。
 */
  enum ncclWorkElemSubType subType;
};
static_assert(NCCL_WORK_SIZE % sizeof(struct ncclWorkElemP2p) == 0, "ncclWorkElemP2p size must be a multiple of ncclWork size");

/* 中文逐项-类型 T0193：ncclWorkElemReg
 * 类型职责：注册用户缓冲的collective扩展任务，额外保存CollNet直接访问需要的上下游地址。
 * 声明内可见成员（含嵌套成员）：elem、dnInputs、dnOutputs、upOutputs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclWorkElemReg {
/* 中文逐项-变量 V2194：elem
 * elem（struct ncclWorkElem；成员 ncclWorkElemReg）：注册work扩展或队列容器内嵌的普通工作元素，扩展信息在它后面保存。
 */
  struct ncclWorkElem elem;
/* 中文逐项-变量 V2195：dnInputs
 * dnInputs（void；成员 ncclWorkElemReg）：CollNet 直接缓冲优化保存的下行输入/输出及上行输出映射地址。
 */
  void* dnInputs[NCCL_MAX_DIRECT_ARITY+1];
/* 中文逐项-变量 V2196：dnOutputs
 * dnOutputs（void；成员 ncclWorkElemReg）：CollNet 直接缓冲优化保存的下行输入/输出及上行输出映射地址。
 */
  void* dnOutputs[NCCL_MAX_DIRECT_ARITY+1];
/* 中文逐项-变量 V2197：upOutputs
 * upOutputs（void；成员 ncclWorkElemReg）：CollNet 直接缓冲优化保存的下行输入/输出及上行输出映射地址。
 */
  void* upOutputs[NCCL_MAX_DIRECT_ARITY+1];
};
static_assert(NCCL_WORK_SIZE % sizeof(struct ncclWorkElemReg) == 0, "ncclWork size must be a multiple of ncclWorkElemReg size");
static_assert(sizeof(struct ncclWorkElemReg) % sizeof(struct ncclWorkElem) == 0, "ncclWorkElemReg size must be a multiple of ncclWorkElem size");

/* 中文逐项-常量 X0246：NCCL_MAX_WORK_ELEMENTS
 * 编译期定义：NCCL_MAX_WORK_ELEMENTS = (NCCL_WORK_SIZE/sizeof(struct ncclWorkElem))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MAX_WORK_ELEMENTS (NCCL_WORK_SIZE/sizeof(struct ncclWorkElem))
/* 中文逐项-常量 X0247：NCCL_MAX_WORK_ELEMENTS_P2P
 * 编译期定义：NCCL_MAX_WORK_ELEMENTS_P2P = (NCCL_WORK_SIZE/sizeof(struct ncclWorkElemP2p))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MAX_WORK_ELEMENTS_P2P (NCCL_WORK_SIZE/sizeof(struct ncclWorkElemP2p))
/* 中文逐项-常量 X0248：NCCL_MAX_WORK_ELEMENTS_REG
 * 编译期定义：NCCL_MAX_WORK_ELEMENTS_REG = (NCCL_WORK_SIZE/sizeof(struct ncclWorkElemReg))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MAX_WORK_ELEMENTS_REG (NCCL_WORK_SIZE/sizeof(struct ncclWorkElemReg))
// Number of named barriers supported by CUDA
/* 中文逐项-常量 X0249：NCCL_MAX_GROUPS
 * 编译期定义：NCCL_MAX_GROUPS = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_MAX_GROUPS 16

/* 中文导读：
 * 固定 512 字节的工作记录，可装多个 collective 元素或 P2P 元素。
 * CPU 填入 channel 工作 FIFO，GPU 取出到共享内存后分派执行；isLast 标记本批最后一个记录。
 */
/* 中文逐项-类型 T0194：ncclWork
 * 类型职责：固定512字节任务记录，union允许容纳多项collective/P2P/注册元素，是FIFO元数据单位。
 * 声明内可见成员（含嵌套成员）：pad、header、elems、p2pElems、regElems。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclWork {
/* 中文逐项-类型 T0195：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：pad、header、elems、p2pElems、regElems。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V2198：pad
 * pad（char；成员 ncclWork::<匿名>）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
    char pad[NCCL_WORK_SIZE];
/* 中文逐项-变量 V2199：header
 * header（struct ncclWorkElemHeader；成员 ncclWork::<匿名>）：work 公共头，包含任务类型、函数下标、线程数和本批终止标记。
 */
    struct ncclWorkElemHeader header;
/* 中文逐项-变量 V2200：elems
 * elems（struct ncclWorkElem；成员 ncclWork::<匿名>）：固定大小 work union 的不同元素布局，必须先看 header.type 再解释内存。
 */
    struct ncclWorkElem elems[NCCL_MAX_WORK_ELEMENTS];
/* 中文逐项-变量 V2201：p2pElems
 * p2pElems（struct ncclWorkElemP2p；成员 ncclWork::<匿名>）：固定大小 work union 的不同元素布局，必须先看 header.type 再解释内存。
 */
    struct ncclWorkElemP2p p2pElems[NCCL_MAX_WORK_ELEMENTS_P2P];
/* 中文逐项-变量 V2202：regElems
 * regElems（struct ncclWorkElemReg；成员 ncclWork::<匿名>）：固定大小 work union 的不同元素布局，必须先看 header.type 再解释内存。
 */
    struct ncclWorkElemReg regElems[NCCL_MAX_WORK_ELEMENTS_REG];
  };
};

static_assert(sizeof(struct ncclWork) == NCCL_WORK_SIZE, "ncclWork size needs to be well aligned");

/* 中文导读：
 * 独立的逻辑通信流水线：拥有 Ring/Tree 拓扑、peer 连接表和工作 FIFO。
 * 常规 kernel 以 blockIdx.x 选 channel；同一 channel 可执行一批工作，多个 channel 并行分担数据。
 * channel 不是网口、QP 或 CUDA stream；collective 与 P2P 使用同一 channels 数组中的对象。
 */
/* 中文精读：图示绑定 V3.3 T0196 ncclChannel
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=T0196
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0115] ncclChannel：字段协作图（结构字段上下文）
 *   图：28-channel-generation, 01-submit
 *   定位：#symbol=T0196&binding=B0115
 */
/* 中文逐项-类型 T0196：ncclChannel
 * 类型职责：独立逻辑执行流水线，持有Ring/Tree、peer表和工作描述符FIFO。
 * 声明内可见成员（含嵌套成员）：ring、tree、collTree、id、peers、devPeers、workFifo、workCount、totalSize、workFifoTail、index、workFifoGdr、workFifoDev、gdrMemDesc、data。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclChannel {
/* 中文逐项-类型 T0197：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：ring、tree、collTree、id、peers、devPeers、workFifo、workCount、totalSize、workFifoTail、index、workFifoGdr、workFifoDev、gdrMemDesc、data。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-类型 T0198：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：ring、tree、collTree、id、peers、devPeers、workFifo、workCount、totalSize、workFifoTail、index、workFifoGdr、workFifoDev、gdrMemDesc。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V2203：ring
 * ring（struct ncclRing；成员 ncclChannel::<匿名>::<匿名>）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 */
      struct ncclRing ring;
/* 中文逐项-变量 V2204：tree
 * tree（struct ncclTree；成员 ncclChannel::<匿名>::<匿名>）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 */
      struct ncclTree tree;
/* 中文逐项-变量 V2205：collTree
 * collTree（struct ncclDirect；成员 ncclChannel::<匿名>::<匿名>）：CollNet 的机内 head/分散/收集关系，包含虚拟网络端点。
 */
      struct ncclDirect collTree;

/* 中文逐项-变量 V2206：id
 * id（int；成员 ncclChannel::<匿名>::<匿名>）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
      int id;

      // Communication structures
/* 中文逐项-变量 V2207：peers
 * peers（struct ncclPeer；成员 ncclChannel::<匿名>::<匿名>）：按对端 rank 索引的主机/设备连接表；预留槽位不表示已建链。
 */
      struct ncclPeer* peers;
/* 中文逐项-变量 V2208：devPeers
 * devPeers（struct ncclPeer；成员 ncclChannel::<匿名>::<匿名>）：按对端 rank 索引的主机/设备连接表；预留槽位不表示已建链。
 */
      struct ncclPeer* devPeers;

      // Operation list for aggregation
/* 中文逐项-变量 V2209：workFifo
 * workFifo（struct ncclWork；成员 ncclChannel::<匿名>::<匿名>）：一条任务描述或 channel 的描述符 FIFO，存元数据而非整个用户张量。
 */
      struct ncclWork* workFifo;
/* 中文逐项-变量 V2210：workCount
 * workCount（int；成员 ncclChannel::<匿名>::<匿名>）：当前批次此 channel 排入的描述符条数；不代表长期累计操作总数。
 */
      int workCount;
/* 中文逐项-变量 V2211：totalSize
 * totalSize（size_t；成员 ncclChannel::<匿名>::<匿名>）：排队有效数据规模统计，供负载平衡或聚合选型使用。
 */
      size_t totalSize;
/* 中文逐项-变量 V2212：workFifoTail
 * workFifoTail（uint64_t；成员 ncclChannel::<匿名>::<匿名>）：CPU 工作 FIFO 单调写序号，对 NCCL_MAX_OPS 取模才得到槽下标。
 */
      uint64_t workFifoTail; // Only used by CPU
/* 中文逐项-变量 V2213：index
 * index（uint16_t；成员 ncclChannel::<匿名>::<匿名>）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 */
      uint16_t index;        // Only used by GPU

      // GDRCOPY support
/* 中文逐项-变量 V2214：workFifoGdr
 * workFifoGdr（struct ncclWork；成员 ncclChannel::<匿名>::<匿名>）：GPU 读取的工作队列视图或 GDRCopy 主机映射地址，可能与普通主机 FIFO 分开。
 */
      struct ncclWork* workFifoGdr;
/* 中文逐项-变量 V2215：workFifoDev
 * workFifoDev（struct ncclWork；成员 ncclChannel::<匿名>::<匿名>）：GPU 读取的工作队列视图或 GDRCopy 主机映射地址，可能与普通主机 FIFO 分开。
 */
      struct ncclWork* workFifoDev;
/* 中文逐项-变量 V2216：gdrMemDesc
 * gdrMemDesc（void；成员 ncclChannel::<匿名>::<匿名>）：撤销 GDRCopy pin/map 及显存分配所需的描述对象。
 */
      void* gdrMemDesc;
    };
/* 中文逐项-变量 V2217：data
 * data（int；成员 ncclChannel::<匿名>）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 */
    int data[0x80];
  };
};
static_assert(sizeof(struct ncclChannel) == 0x80*sizeof(int), "ncclChannel must have a pow2 size");

/* 中文导读：
 * communicator 的 GPU 端精简视图，只保留执行需要的 rank、缓冲区大小、中止标记和 channel 指针。
 * 主机端 ncclComm 中的 socket、pthread 和 verbs 资源不会直接由 GPU kernel 操作。
 */
/* 中文逐项-类型 T0199：ncclDevComm
 * 类型职责：GPU执行用的精简通信上下文及连续channel布局，排除socket/pthread等主机对象。
 * 声明内可见成员（含嵌套成员）：rank、nRanks、buffSizes、abortFlag、channels。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclDevComm {
/* 中文逐项-变量 V2218：rank
 * rank（int；成员 ncclDevComm）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V2219：nRanks
 * nRanks（int；成员 ncclDevComm）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 */
  int nRanks;
/* 中文逐项-变量 V2220：buffSizes
 * buffSizes（int；成员 ncclDevComm）：各协议物理缓冲字节容量，先除8得到每step物理容量。
 */
  int buffSizes[NCCL_NUM_PROTOCOLS];

  // Flag to ask NCCL kernels to abort
/* 中文逐项-变量 V2221：abortFlag
 * abortFlag（uint32_t；成员 ncclDevComm）：主机/设备可观察的异常中止共享标志，用于打破协议或 socket 等待。
 */
  volatile uint32_t *abortFlag;

  // Channels, device side
/* 中文逐项-变量 V2222：channels
 * channels（struct ncclChannel；成员 ncclDevComm）：communicator 的逻辑通道数组，GPU 常以 blockIdx.x 选择一项。
 */
  struct ncclChannel* channels;
};

/* 中文精读：为何 comm 与 channels 连续布局
 * 在设备端可以从 comm 指针强转得到紧随其后的 channels 数组，少一次间接全局内存读取。
 * 主机 devCommSetup 分配并构造这个完整对象，kernel 的 common.h 按相同布局定位 channel。
 * 这是固定 ABI/布局契约；即使只为阅读增加字段，也可能破坏偏移和大小约束。
 * 本注释分支只增加注释，不改变结构体成员、对齐或任何编译期常量。
 */
/* 中文逐项-类型 T0200：ncclDevCommAndChannels
 * 类型职责：GPU执行用的精简通信上下文及连续channel布局，排除socket/pthread等主机对象。
 * 声明内可见成员（含嵌套成员）：comm、channels。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclDevCommAndChannels {
/* 中文逐项-变量 V2223：comm
 * comm（ncclDevComm；成员 ncclDevCommAndChannels）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  ncclDevComm comm;
/* 中文逐项-变量 V2224：channels
 * channels（ncclChannel；成员 ncclDevCommAndChannels）：communicator 的逻辑通道数组，GPU 常以 blockIdx.x 选择一项。
 */
  ncclChannel channels[MAXCHANNELS];
};

#endif
