/*************************************************************************
 * Copyright (c) 2017-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_NET_H_
/* 中文逐项-常量 X0611：NCCL_NET_H_
 * 编译期定义：NCCL_NET_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_H_

#include "nccl.h"
#include <stdint.h>

/* 中文逐项-常量 X0612：NCCL_NET_HANDLE_MAXSIZE
 * 编译期定义：NCCL_NET_HANDLE_MAXSIZE = 128。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_HANDLE_MAXSIZE 128

/* 中文逐项-常量 X0613：NCCL_PTR_HOST
 * 编译期定义：NCCL_PTR_HOST = 0x1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PTR_HOST 0x1
/* 中文逐项-常量 X0614：NCCL_PTR_CUDA
 * 编译期定义：NCCL_PTR_CUDA = 0x2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PTR_CUDA 0x2

// Maximum number of requests per comm object
/* 中文逐项-常量 X0615：NCCL_NET_MAX_REQUESTS
 * 编译期定义：NCCL_NET_MAX_REQUESTS = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_MAX_REQUESTS 8

typedef /* 中文逐项-类型 T0302：ncclDebugLogLevel
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum {/* 中文逐项-枚举项 X0616：NCCL_LOG_NONE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */NCCL_LOG_NONE=0, /* 中文逐项-枚举项 X0617：NCCL_LOG_VERSION
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */NCCL_LOG_VERSION=1, /* 中文逐项-枚举项 X0618：NCCL_LOG_WARN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */NCCL_LOG_WARN=2, /* 中文逐项-枚举项 X0619：NCCL_LOG_INFO
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */NCCL_LOG_INFO=3, /* 中文逐项-枚举项 X0620：NCCL_LOG_ABORT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */NCCL_LOG_ABORT=4, /* 中文逐项-枚举项 X0621：NCCL_LOG_TRACE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */NCCL_LOG_TRACE=5} ncclDebugLogLevel;
typedef /* 中文逐项-类型 T0303：ncclDebugLogSubSys
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum {/* 中文逐项-枚举项 X0622：NCCL_INIT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */NCCL_INIT=1, /* 中文逐项-枚举项 X0623：NCCL_COLL
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */NCCL_COLL=2, /* 中文逐项-枚举项 X0624：NCCL_P2P
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */NCCL_P2P=4, /* 中文逐项-枚举项 X0625：NCCL_SHM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */NCCL_SHM=8, /* 中文逐项-枚举项 X0626：NCCL_NET
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：16
 */NCCL_NET=16, /* 中文逐项-枚举项 X0627：NCCL_GRAPH
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：32
 */NCCL_GRAPH=32, /* 中文逐项-枚举项 X0628：NCCL_TUNING
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：64
 */NCCL_TUNING=64, /* 中文逐项-枚举项 X0629：NCCL_ENV
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：128
 */NCCL_ENV=128, /* 中文逐项-枚举项 X0630：NCCL_ALLOC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：256
 */NCCL_ALLOC=256, /* 中文逐项-枚举项 X0631：NCCL_ALL
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：~0
 */NCCL_ALL=~0} ncclDebugLogSubSys;

typedef void (*ncclDebugLogger_t)(ncclDebugLogLevel level, unsigned long flags, const char *file, int line, const char *fmt, ...);

typedef /* 中文逐项-类型 T0304：ncclNetProperties_v5_t
 * 类型职责：网络设备能力报告，描述PCI关联、速率、指针支持和连接/接收能力，v5增加批量接收等字段。
 * 声明内可见成员（含嵌套成员）：name、pciPath、guid、ptrSupport、speed、port、latency、maxComms、maxRecvs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct {
/* 中文逐项-变量 V2695：name
 * name（char；成员 ncclNetProperties_v5_t）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  char* name;     // Used mostly for logging.
/* 中文逐项-变量 V2696：pciPath
 * pciPath（char；成员 ncclNetProperties_v5_t）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  char* pciPath;  // Path to the PCI device in /sys.
/* 中文逐项-变量 V2697：guid
 * guid（uint64_t；成员 ncclNetProperties_v5_t）：网络设备GUID，用于识别相关设备/端口与稳定硬件身份。
 */
  uint64_t guid;  // Unique identifier for the NIC chip. Important for
                  // cards with multiple PCI functions (Physical or virtual).
/* 中文逐项-变量 V2698：ptrSupport
 * ptrSupport（int；成员 ncclNetProperties_v5_t）：设备能否注册GPU内存、进行网络集合归约及接受哪种指针类型的能力描述。
 */
  int ptrSupport; // NCCL_PTR_HOST or NCCL_PTR_HOST|NCCL_PTR_CUDA
/* 中文逐项-变量 V2699：speed
 * speed（int；成员 ncclNetProperties_v5_t）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 */
  int speed;      // Port speed in Mbps.
/* 中文逐项-变量 V2700：port
 * port（int；成员 ncclNetProperties_v5_t）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
  int port;       // Port number.
/* 中文逐项-变量 V2701：latency
 * latency（float；成员 ncclNetProperties_v5_t）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 */
  float latency;  // Network latency
/* 中文逐项-变量 V2702：maxComms
 * maxComms（int；成员 ncclNetProperties_v5_t）：网络后端支持的并发连接数量及一次Irecv可聚合的接收项数，不等同于QP数量或SRQ。
 */
  int maxComms;   // Maximum number of comms we can create
/* 中文逐项-变量 V2703：maxRecvs
 * maxRecvs（int；成员 ncclNetProperties_v5_t）：网络后端支持的并发连接数量及一次Irecv可聚合的接收项数，不等同于QP数量或SRQ。
 */
  int maxRecvs;   // Maximum number of grouped receives.
}ncclNetProperties_v5_t;

typedef ncclNetProperties_v5_t ncclNetProperties_t;

typedef /* 中文逐项-类型 T0305：ncclNet_v5_t
 * 类型职责：网络点对点后端ABI函数表，NCCL通过listen/connect/regMr/isend/irecv/test调用实现。
 * 声明内可见成员（含嵌套成员）：name、init、devices、getProperties、listen、connect、accept、regMr、deregMr、isend、irecv、iflush、test、closeSend、closeRecv、closeListen。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct {
  // Name of the network (mainly for logs)
/* 中文逐项-变量 V2704：name
 * name（char；成员 ncclNet_v5_t）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  const char* name;
  // Initialize the network.
/* 中文逐项-变量 V2705：init
 * init 是函数指针字段/变量：初始化当前选定的网络插件/后端状态并接收日志回调，设备枚举在devices中单独完成；函数表保存可调用入口，不是调用返回的数据。完整声明：(*init)(ncclDebugLogger_t
 * logFunction)。
 * 回调参数 logFunction（ncclDebugLogger_t logFunction）：NCCL提供的日志回调，插件用它按统一级别/子系统报告信息与错误。
 * init（ncclResult_t；成员 ncclNet_v5_t）：初始化任务参数或后端初始化回调，按其声明为对象还是函数指针分别解释。
 */
  ncclResult_t (*init)(ncclDebugLogger_t logFunction);
  // Return the number of adapters.
/* 中文逐项-变量 V2706：devices
 * devices 是函数指针字段/变量：查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*devices)(int* ndev)。
 * 回调参数 ndev（int* ndev）：当前网络后端枚举的设备/端口数，或devices回调的输出地址；它不是CUDA设备数或communicator的rank数。
 * devices（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*devices)(int* ndev);
  // Get various device properties.
/* 中文逐项-变量 V2707：getProperties
 * getProperties
 * 是函数指针字段/变量：读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*getProperties)(int dev,
 * ncclNetProperties_v5_t* props)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 props（ncclNetProperties_v5_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * getProperties（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*getProperties)(int dev, ncclNetProperties_v5_t* props);
  // Create a receiving object and provide a handle to connect to it. The
  // handle can be up to NCCL_NET_HANDLE_MAXSIZE bytes and will be exchanged
  // between ranks to create a connection.
/* 中文逐项-变量 V2708：listen
 * listen 是函数指针字段/变量：创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*listen)(int dev, void*
 * handle, void** listenComm)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * listen（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*listen)(int dev, void* handle, void** listenComm);
  // Connect to a handle and return a sending comm object for that peer.
  // This call must not block for the connection to be established, and instead
  // should return successfully with sendComm == NULL with the expectation that
  // it will be called again until sendComm != NULL.
/* 中文逐项-变量 V2709：connect
 * connect 是函数指针字段/变量：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*connect)(int dev, void* handle,
 * void** sendComm)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 sendComm（void** sendComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * connect（ncclResult_t；成员 ncclNet_v5_t）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
  ncclResult_t (*connect)(int dev, void* handle, void** sendComm);
  // Finalize connection establishment after remote peer has called connect.
  // This call must not block for the connection to be established, and instead
  // should return successfully with recvComm == NULL with the expectation that
  // it will be called again until recvComm != NULL.
/* 中文逐项-变量 V2710：accept
 * accept 是函数指针字段/变量：推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*accept)(void* listenComm, void**
 * recvComm)。
 * 回调参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 recvComm（void** recvComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * accept（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*accept)(void* listenComm, void** recvComm);
  // Register/Deregister memory. Comm can be either a sendComm or a recvComm.
  // Type is either NCCL_PTR_HOST or NCCL_PTR_CUDA.
/* 中文逐项-变量 V2711：regMr
 * regMr 是函数指针字段/变量：注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*regMr)(void* comm,
 * void* data, int size, int type, void** mhandle)。
 * 回调参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 回调参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * regMr（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*regMr)(void* comm, void* data, int size, int type, void** mhandle);
/* 中文逐项-变量 V2712：deregMr
 * deregMr 是函数指针字段/变量：解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*deregMr)(void* comm, void*
 * mhandle)。
 * 回调参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * deregMr（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*deregMr)(void* comm, void* mhandle);
  // Asynchronous send to a peer.
  // May return request == NULL if the call cannot be performed (or would block)
/* 中文逐项-变量 V2713：isend
 * isend 是函数指针字段/变量：提交异步发送请求；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*isend)(void* sendComm, void* data,
 * int size, int tag, void* mhandle, void** request)。
 * 回调参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * isend（ncclResult_t；成员 ncclNet_v5_t）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 */
  ncclResult_t (*isend)(void* sendComm, void* data, int size, int tag, void* mhandle, void** request);
  // Asynchronous recv from a peer.
  // May return request == NULL if the call cannot be performed (or would block)
/* 中文逐项-变量 V2714：irecv
 * irecv 是函数指针字段/变量：提交一组带地址/长度/tag的异步接收；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*irecv)(void* recvComm,
 * int n, void** data, int* sizes, int* tags, void** mhandles, void** request)。
 * 回调参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 回调参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 回调参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 回调参数 tags（int* tags）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * irecv（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*irecv)(void* recvComm, int n, void** data, int* sizes, int* tags, void** mhandles, void** request);
  // Perform a flush/fence to make sure all data received with NCCL_PTR_CUDA is
  // visible to the GPU
/* 中文逐项-变量 V2715：iflush
 * iflush 是函数指针字段/变量：提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*iflush)(void* recvComm, int
 * n, void** data, int* sizes, void** mhandles, void** request)。
 * 回调参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 回调参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 回调参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 回调参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * iflush（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*iflush)(void* recvComm, int n, void** data, int* sizes, void** mhandles, void** request);
  // Test whether a request is complete. If size is not NULL, it returns the
  // number of bytes sent/received.
/* 中文逐项-变量 V2716：test
 * test 是函数指针字段/变量：查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*test)(void* request, int* done,
 * int* sizes)。
 * 回调参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 回调参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 回调参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * test（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*test)(void* request, int* done, int* sizes);
  // Close and free send/recv comm objects
/* 中文逐项-变量 V2717：closeSend
 * closeSend 是函数指针字段/变量：关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeSend)(void* sendComm)。
 * 回调参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * closeSend（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeSend)(void* sendComm);
/* 中文逐项-变量 V2718：closeRecv
 * closeRecv 是函数指针字段/变量：关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeRecv)(void* recvComm)。
 * 回调参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * closeRecv（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeRecv)(void* recvComm);
/* 中文逐项-变量 V2719：closeListen
 * closeListen 是函数指针字段/变量：关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeListen)(void*
 * listenComm)。
 * 回调参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * closeListen（ncclResult_t；成员 ncclNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeListen)(void* listenComm);
} ncclNet_v5_t;

typedef ncclNet_v5_t ncclNet_t;

/* 中文逐项-常量 X0632：NCCL_PLUGIN_SYMBOL
 * 编译期定义：NCCL_PLUGIN_SYMBOL = ncclNetPlugin_v5。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PLUGIN_SYMBOL ncclNetPlugin_v5

typedef /* 中文逐项-类型 T0306：ncclCollNet_v5_t
 * 类型职责：网络内集合归约后端ABI函数表，单独报告reduceSupport并提交Iallreduce。
 * 声明内可见成员（含嵌套成员）：name、init、devices、getProperties、listen、connect、reduceSupport、regMr、deregMr、iallreduce、iflush、test、closeColl、closeListen。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct {
  // Name of the collective network (mainly for logs)
/* 中文逐项-变量 V2720：name
 * name（char；成员 ncclCollNet_v5_t）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  const char* name;
  // Initialize the collective network.
/* 中文逐项-变量 V2721：init
 * init 是函数指针字段/变量：初始化当前选定的网络插件/后端状态并接收日志回调，设备枚举在devices中单独完成；函数表保存可调用入口，不是调用返回的数据。完整声明：(*init)(ncclDebugLogger_t
 * logFunction)。
 * 回调参数 logFunction（ncclDebugLogger_t logFunction）：NCCL提供的日志回调，插件用它按统一级别/子系统报告信息与错误。
 * init（ncclResult_t；成员 ncclCollNet_v5_t）：初始化任务参数或后端初始化回调，按其声明为对象还是函数指针分别解释。
 */
  ncclResult_t (*init)(ncclDebugLogger_t logFunction);
  // Return the number of adapters capable of doing collective operations.
  // If ndev returns 0, all other functions might be set to NULL.
/* 中文逐项-变量 V2722：devices
 * devices 是函数指针字段/变量：查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*devices)(int* ndev)。
 * 回调参数 ndev（int* ndev）：当前网络后端枚举的设备/端口数，或devices回调的输出地址；它不是CUDA设备数或communicator的rank数。
 * devices（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*devices)(int* ndev);
  // Get various device properties.
/* 中文逐项-变量 V2723：getProperties
 * getProperties
 * 是函数指针字段/变量：读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*getProperties)(int dev,
 * ncclNetProperties_v5_t* props)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 props（ncclNetProperties_v5_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * getProperties（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*getProperties)(int dev, ncclNetProperties_v5_t* props);
  // Create a receiving object and provide a handle to connect to it. The
  // handle can be up to NCCL_NET_HANDLE_MAXSIZE bytes and will be exchanged
  // between ranks to create connections.
/* 中文逐项-变量 V2724：listen
 * listen 是函数指针字段/变量：创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*listen)(int dev, void*
 * handle, void** listenComm)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * listen（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*listen)(int dev, void* handle, void** listenComm);
  // Create a group for collective operations. handles have been created
  // using listen() above. rank indicates caller's rank in the collective network.
/* 中文逐项-变量 V2725：connect
 * connect 是函数指针字段/变量：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*connect)(void* handles[], int
 * nranks, int rank, void* listenComm, void** collComm)。
 * 回调参数 handles（void* handles[]）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 回调参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 回调参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 collComm（void** collComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * connect（ncclResult_t；成员 ncclCollNet_v5_t）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
  ncclResult_t (*connect)(void* handles[], int nranks, int rank, void* listenComm, void** collComm);
  // Returns whether a reduction operation on a data type is supported.
  // 1 for supported, 0 otherwise.
/* 中文逐项-变量 V2726：reduceSupport
 * reduceSupport
 * 是函数指针字段/变量：查询某数据类型和归约操作能否在网络内完成；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*reduceSupport)(ncclDataType_t
 * dataType, ncclRedOp_t redOp, int* supported)。
 * 回调参数 dataType（ncclDataType_t dataType）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 回调参数 redOp（ncclRedOp_t redOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 回调参数 supported（int* supported）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * reduceSupport（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*reduceSupport)(ncclDataType_t dataType, ncclRedOp_t redOp, int* supported);
  // Register/Deregister memory. Type is either NCCL_PTR_HOST or NCCL_PTR_CUDA.
/* 中文逐项-变量 V2727：regMr
 * regMr 是函数指针字段/变量：注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*regMr)(void* collComm,
 * void* data, int size, int type, void** mhandle)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 回调参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * regMr（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*regMr)(void* collComm, void* data, int size, int type, void** mhandle);
/* 中文逐项-变量 V2728：deregMr
 * deregMr 是函数指针字段/变量：解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*deregMr)(void* collComm,
 * void* mhandle)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * deregMr（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*deregMr)(void* collComm, void* mhandle);
  // Performs an asynchronous allreduce operation on the collective group.
  // May return request == NULL if the call cannot be performed (or would block).
/* 中文逐项-变量 V2729：iallreduce
 * iallreduce 是函数指针字段/变量：提交网络集合归约请求；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*iallreduce)(void* collComm,
 * void* sendData, void* recvData, int count, ncclDataType_t dataType, ncclRedOp_t redOp, void* sendMhandle, void*
 * recvMhandle, void** request)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 sendData（void* sendData）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 回调参数 recvData（void* recvData）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 回调参数 count（int count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 回调参数 dataType（ncclDataType_t dataType）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 回调参数 redOp（ncclRedOp_t redOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 回调参数 sendMhandle（void* sendMhandle）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 回调参数 recvMhandle（void* recvMhandle）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * iallreduce（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*iallreduce)(void* collComm, void* sendData, void* recvData, int count,
      ncclDataType_t dataType, ncclRedOp_t redOp, void* sendMhandle, void* recvMhandle, void** request);
  // Perform a flush/fence to make sure all data received with NCCL_PTR_CUDA is
  // visible to the GPU
/* 中文逐项-变量 V2730：iflush
 * iflush 是函数指针字段/变量：提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*iflush)(void* collComm,
 * void* data, int size, void* mhandle, void** request)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * iflush（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*iflush)(void* collComm, void* data, int size, void* mhandle, void** request);
  // Test whether a request is complete. If size is not NULL, it returns the
  // number of bytes sent/received.
/* 中文逐项-变量 V2731：test
 * test 是函数指针字段/变量：查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*test)(void* request, int* done,
 * int* size)。
 * 回调参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 回调参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 回调参数 size（int* size）：网络请求完成后的有效字节数输出地址；可为空时调用者不要求返回长度。
 * test（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*test)(void* request, int* done, int* size);
  // Close and free collective comm objects
/* 中文逐项-变量 V2732：closeColl
 * closeColl 是函数指针字段/变量：关闭集合网络连接；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeColl)(void* collComm)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * closeColl（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeColl)(void* collComm);
/* 中文逐项-变量 V2733：closeListen
 * closeListen 是函数指针字段/变量：关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeListen)(void*
 * listenComm)。
 * 回调参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * closeListen（ncclResult_t；成员 ncclCollNet_v5_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeListen)(void* listenComm);
} ncclCollNet_v5_t;

typedef ncclCollNet_v5_t ncclCollNet_t;

/* 中文逐项-常量 X0633：NCCL_COLLNET_PLUGIN_SYMBOL
 * 编译期定义：NCCL_COLLNET_PLUGIN_SYMBOL = ncclCollNetPlugin_v5。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_COLLNET_PLUGIN_SYMBOL ncclCollNetPlugin_v5

typedef /* 中文逐项-类型 T0307：ncclNetProperties_v4_t
 * 类型职责：网络设备能力报告，描述PCI关联、速率、指针支持和连接/接收能力，v5增加批量接收等字段。
 * 声明内可见成员（含嵌套成员）：name、pciPath、guid、ptrSupport、speed、port、maxComms。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct {
/* 中文逐项-变量 V2734：name
 * name（char；成员 ncclNetProperties_v4_t）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  char* name;     // Used mostly for logging.
/* 中文逐项-变量 V2735：pciPath
 * pciPath（char；成员 ncclNetProperties_v4_t）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  char* pciPath;  // Path to the PCI device in /sys.
/* 中文逐项-变量 V2736：guid
 * guid（uint64_t；成员 ncclNetProperties_v4_t）：网络设备GUID，用于识别相关设备/端口与稳定硬件身份。
 */
  uint64_t guid;  // Unique identifier for the NIC chip. Important for
                  // cards with multiple PCI functions (Physical or virtual).
/* 中文逐项-变量 V2737：ptrSupport
 * ptrSupport（int；成员 ncclNetProperties_v4_t）：设备能否注册GPU内存、进行网络集合归约及接受哪种指针类型的能力描述。
 */
  int ptrSupport; // NCCL_PTR_HOST or NCCL_PTR_HOST|NCCL_PTR_CUDA
/* 中文逐项-变量 V2738：speed
 * speed（int；成员 ncclNetProperties_v4_t）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 */
  int speed;      // Port speed in Mbps.
/* 中文逐项-变量 V2739：port
 * port（int；成员 ncclNetProperties_v4_t）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
  int port;       // Port number.
/* 中文逐项-变量 V2740：maxComms
 * maxComms（int；成员 ncclNetProperties_v4_t）：网络后端支持的并发连接数量及一次Irecv可聚合的接收项数，不等同于QP数量或SRQ。
 */
  int maxComms;   // Maximum number of comms we can create
} ncclNetProperties_v4_t;

typedef /* 中文逐项-类型 T0308：ncclNet_v4_t
 * 类型职责：网络点对点后端ABI函数表，NCCL通过listen/connect/regMr/isend/irecv/test调用实现。
 * 声明内可见成员（含嵌套成员）：name、init、devices、getProperties、listen、connect、accept、regMr、deregMr、isend、irecv、iflush、test、closeSend、closeRecv、closeListen。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct {
  // Name of the network (mainly for logs)
/* 中文逐项-变量 V2741：name
 * name（char；成员 ncclNet_v4_t）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  const char* name;
  // Initialize the network.
/* 中文逐项-变量 V2742：init
 * init 是函数指针字段/变量：初始化当前选定的网络插件/后端状态并接收日志回调，设备枚举在devices中单独完成；函数表保存可调用入口，不是调用返回的数据。完整声明：(*init)(ncclDebugLogger_t
 * logFunction)。
 * 回调参数 logFunction（ncclDebugLogger_t logFunction）：NCCL提供的日志回调，插件用它按统一级别/子系统报告信息与错误。
 * init（ncclResult_t；成员 ncclNet_v4_t）：初始化任务参数或后端初始化回调，按其声明为对象还是函数指针分别解释。
 */
  ncclResult_t (*init)(ncclDebugLogger_t logFunction);
  // Return the number of adapters.
/* 中文逐项-变量 V2743：devices
 * devices 是函数指针字段/变量：查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*devices)(int* ndev)。
 * 回调参数 ndev（int* ndev）：当前网络后端枚举的设备/端口数，或devices回调的输出地址；它不是CUDA设备数或communicator的rank数。
 * devices（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*devices)(int* ndev);
  // Get various device properties.
/* 中文逐项-变量 V2744：getProperties
 * getProperties
 * 是函数指针字段/变量：读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*getProperties)(int dev,
 * ncclNetProperties_v4_t* props)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 props（ncclNetProperties_v4_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * getProperties（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*getProperties)(int dev, ncclNetProperties_v4_t* props);
  // Create a receiving object and provide a handle to connect to it. The
  // handle can be up to NCCL_NET_HANDLE_MAXSIZE bytes and will be exchanged
  // between ranks to create a connection.
/* 中文逐项-变量 V2745：listen
 * listen 是函数指针字段/变量：创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*listen)(int dev, void*
 * handle, void** listenComm)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * listen（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*listen)(int dev, void* handle, void** listenComm);
  // Connect to a handle and return a sending comm object for that peer.
/* 中文逐项-变量 V2746：connect
 * connect 是函数指针字段/变量：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*connect)(int dev, void* handle,
 * void** sendComm)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 sendComm（void** sendComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * connect（ncclResult_t；成员 ncclNet_v4_t）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
  ncclResult_t (*connect)(int dev, void* handle, void** sendComm);
  // Finalize connection establishment after remote peer has called connectHandle
/* 中文逐项-变量 V2747：accept
 * accept 是函数指针字段/变量：推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*accept)(void* listenComm, void**
 * recvComm)。
 * 回调参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 recvComm（void** recvComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * accept（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*accept)(void* listenComm, void** recvComm);
  // Register/Deregister memory. Comm can be either a sendComm or a recvComm.
  // Type is either NCCL_PTR_HOST or NCCL_PTR_CUDA.
/* 中文逐项-变量 V2748：regMr
 * regMr 是函数指针字段/变量：注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*regMr)(void* comm,
 * void* data, int size, int type, void** mhandle)。
 * 回调参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 回调参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * regMr（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*regMr)(void* comm, void* data, int size, int type, void** mhandle);
/* 中文逐项-变量 V2749：deregMr
 * deregMr 是函数指针字段/变量：解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*deregMr)(void* comm, void*
 * mhandle)。
 * 回调参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * deregMr（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*deregMr)(void* comm, void* mhandle);
  // Asynchronous send to a peer.
  // May return request == NULL if the call cannot be performed (or would block)
/* 中文逐项-变量 V2750：isend
 * isend 是函数指针字段/变量：提交异步发送请求；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*isend)(void* sendComm, void* data,
 * int size, void* mhandle, void** request)。
 * 回调参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * isend（ncclResult_t；成员 ncclNet_v4_t）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 */
  ncclResult_t (*isend)(void* sendComm, void* data, int size, void* mhandle, void** request);
  // Asynchronous recv from a peer.
  // May return request == NULL if the call cannot be performed (or would block)
/* 中文逐项-变量 V2751：irecv
 * irecv 是函数指针字段/变量：提交一组带地址/长度/tag的异步接收；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*irecv)(void* recvComm,
 * void* data, int size, void* mhandle, void** request)。
 * 回调参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * irecv（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*irecv)(void* recvComm, void* data, int size, void* mhandle, void** request);
  // Perform a flush/fence to make sure all data received with NCCL_PTR_CUDA is
  // visible to the GPU
/* 中文逐项-变量 V2752：iflush
 * iflush 是函数指针字段/变量：提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*iflush)(void* recvComm,
 * void* data, int size, void* mhandle, void** request)。
 * 回调参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * iflush（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*iflush)(void* recvComm, void* data, int size, void* mhandle, void** request);
  // Test whether a request is complete. If size is not NULL, it returns the
  // number of bytes sent/received.
/* 中文逐项-变量 V2753：test
 * test 是函数指针字段/变量：查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*test)(void* request, int* done,
 * int* size)。
 * 回调参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 回调参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 回调参数 size（int* size）：网络请求完成后的有效字节数输出地址；可为空时调用者不要求返回长度。
 * test（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*test)(void* request, int* done, int* size);
  // Close and free send/recv comm objects
/* 中文逐项-变量 V2754：closeSend
 * closeSend 是函数指针字段/变量：关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeSend)(void* sendComm)。
 * 回调参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * closeSend（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeSend)(void* sendComm);
/* 中文逐项-变量 V2755：closeRecv
 * closeRecv 是函数指针字段/变量：关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeRecv)(void* recvComm)。
 * 回调参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * closeRecv（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeRecv)(void* recvComm);
/* 中文逐项-变量 V2756：closeListen
 * closeListen 是函数指针字段/变量：关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeListen)(void*
 * listenComm)。
 * 回调参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * closeListen（ncclResult_t；成员 ncclNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeListen)(void* listenComm);
} ncclNet_v4_t;

typedef /* 中文逐项-类型 T0309：ncclCollNet_v4_t
 * 类型职责：网络内集合归约后端ABI函数表，单独报告reduceSupport并提交Iallreduce。
 * 声明内可见成员（含嵌套成员）：name、init、devices、getProperties、listen、connect、reduceSupport、regMr、deregMr、iallreduce、iflush、test、closeColl、closeListen。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct {
  // Name of the collective network (mainly for logs)
/* 中文逐项-变量 V2757：name
 * name（char；成员 ncclCollNet_v4_t）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  const char* name;
  // Initialize the collective network.
/* 中文逐项-变量 V2758：init
 * init 是函数指针字段/变量：初始化当前选定的网络插件/后端状态并接收日志回调，设备枚举在devices中单独完成；函数表保存可调用入口，不是调用返回的数据。完整声明：(*init)(ncclDebugLogger_t
 * logFunction)。
 * 回调参数 logFunction（ncclDebugLogger_t logFunction）：NCCL提供的日志回调，插件用它按统一级别/子系统报告信息与错误。
 * init（ncclResult_t；成员 ncclCollNet_v4_t）：初始化任务参数或后端初始化回调，按其声明为对象还是函数指针分别解释。
 */
  ncclResult_t (*init)(ncclDebugLogger_t logFunction);
  // Return the number of adapters capable of doing collective operations.
  // If ndev returns 0, all other functions might be set to NULL.
/* 中文逐项-变量 V2759：devices
 * devices 是函数指针字段/变量：查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*devices)(int* ndev)。
 * 回调参数 ndev（int* ndev）：当前网络后端枚举的设备/端口数，或devices回调的输出地址；它不是CUDA设备数或communicator的rank数。
 * devices（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*devices)(int* ndev);
  // Get various device properties.
/* 中文逐项-变量 V2760：getProperties
 * getProperties
 * 是函数指针字段/变量：读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*getProperties)(int dev,
 * ncclNetProperties_v4_t* props)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 props（ncclNetProperties_v4_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * getProperties（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*getProperties)(int dev, ncclNetProperties_v4_t* props);
  // Create a receiving object and provide a handle to connect to it. The
  // handle can be up to NCCL_NET_HANDLE_MAXSIZE bytes and will be exchanged
  // between ranks to create connections.
/* 中文逐项-变量 V2761：listen
 * listen 是函数指针字段/变量：创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*listen)(int dev, void*
 * handle, void** listenComm)。
 * 回调参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 回调参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * listen（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*listen)(int dev, void* handle, void** listenComm);
  // Create a group for collective operations. handles have been created
  // using listen() above. rank indicates caller's rank in the collective network.
/* 中文逐项-变量 V2762：connect
 * connect 是函数指针字段/变量：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*connect)(void* handles[], int
 * nranks, int rank, void* listenComm, void** collComm)。
 * 回调参数 handles（void* handles[]）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 回调参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 回调参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 collComm（void** collComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * connect（ncclResult_t；成员 ncclCollNet_v4_t）：各 peer 的待建 channel 位图及是否需要预建链的状态。
 */
  ncclResult_t (*connect)(void* handles[], int nranks, int rank, void* listenComm, void** collComm);
  // Returns whether a reduction operation on a data type is supported.
  // 1 for supported, 0 otherwise.
/* 中文逐项-变量 V2763：reduceSupport
 * reduceSupport
 * 是函数指针字段/变量：查询某数据类型和归约操作能否在网络内完成；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*reduceSupport)(ncclDataType_t
 * dataType, ncclRedOp_t redOp, int* supported)。
 * 回调参数 dataType（ncclDataType_t dataType）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 回调参数 redOp（ncclRedOp_t redOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 回调参数 supported（int* supported）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * reduceSupport（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*reduceSupport)(ncclDataType_t dataType, ncclRedOp_t redOp, int* supported);
  // Register/Deregister memory. Type is either NCCL_PTR_HOST or NCCL_PTR_CUDA.
/* 中文逐项-变量 V2764：regMr
 * regMr 是函数指针字段/变量：注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*regMr)(void* collComm,
 * void* data, int size, int type, void** mhandle)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 回调参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * regMr（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*regMr)(void* collComm, void* data, int size, int type, void** mhandle);
/* 中文逐项-变量 V2765：deregMr
 * deregMr 是函数指针字段/变量：解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*deregMr)(void* collComm,
 * void* mhandle)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * deregMr（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*deregMr)(void* collComm, void* mhandle);
  // Performs an asynchronous allreduce operation on the collective group.
  // May return request == NULL if the call cannot be performed (or would block).
/* 中文逐项-变量 V2766：iallreduce
 * iallreduce 是函数指针字段/变量：提交网络集合归约请求；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*iallreduce)(void* collComm,
 * void* sendData, void* recvData, int count, ncclDataType_t dataType, ncclRedOp_t redOp, void* sendMhandle, void*
 * recvMhandle, void** request)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 sendData（void* sendData）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 回调参数 recvData（void* recvData）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 回调参数 count（int count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 回调参数 dataType（ncclDataType_t dataType）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 回调参数 redOp（ncclRedOp_t redOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 回调参数 sendMhandle（void* sendMhandle）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 回调参数 recvMhandle（void* recvMhandle）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * iallreduce（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*iallreduce)(void* collComm, void* sendData, void* recvData, int count,
      ncclDataType_t dataType, ncclRedOp_t redOp, void* sendMhandle, void* recvMhandle, void** request);
  // Perform a flush/fence to make sure all data received with NCCL_PTR_CUDA is
  // visible to the GPU
/* 中文逐项-变量 V2767：iflush
 * iflush 是函数指针字段/变量：提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*iflush)(void* collComm,
 * void* data, int size, void* mhandle, void** request)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 回调参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 回调参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 回调参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * iflush（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*iflush)(void* collComm, void* data, int size, void* mhandle, void** request);
  // Test whether a request is complete. If size is not NULL, it returns the
  // number of bytes sent/received.
/* 中文逐项-变量 V2768：test
 * test 是函数指针字段/变量：查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*test)(void* request, int* done,
 * int* size)。
 * 回调参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 回调参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 回调参数 size（int* size）：网络请求完成后的有效字节数输出地址；可为空时调用者不要求返回长度。
 * test（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*test)(void* request, int* done, int* size);
  // Close and free collective comm objects
/* 中文逐项-变量 V2769：closeColl
 * closeColl 是函数指针字段/变量：关闭集合网络连接；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeColl)(void* collComm)。
 * 回调参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * closeColl（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeColl)(void* collComm);
/* 中文逐项-变量 V2770：closeListen
 * closeListen 是函数指针字段/变量：关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量；函数表保存可调用入口，不是调用返回的数据。完整声明：(*closeListen)(void*
 * listenComm)。
 * 回调参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * closeListen（ncclResult_t；成员 ncclCollNet_v4_t）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  ncclResult_t (*closeListen)(void* listenComm);
} ncclCollNet_v4_t;

#endif // end include guard
