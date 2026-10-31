/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_SOCKET_H_
/* 中文逐项-常量 X1097：NCCL_SOCKET_H_
 * 编译期定义：NCCL_SOCKET_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SOCKET_H_

#include "nccl.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>

/* 中文逐项-常量 X1098：MAX_IFS
 * 编译期定义：MAX_IFS = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_IFS 16
/* 中文逐项-常量 X1099：MAX_IF_NAME_SIZE
 * 编译期定义：MAX_IF_NAME_SIZE = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_IF_NAME_SIZE 16
/* 中文逐项-常量 X1100：SLEEP_INT
 * 编译期定义：SLEEP_INT = 1000 // connection retry sleep interval in usec。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define SLEEP_INT            1000 // connection retry sleep interval in usec
/* 中文逐项-常量 X1101：RETRY_REFUSED_TIMES
 * 编译期定义：RETRY_REFUSED_TIMES = 2e4 // connection refused retry times before reporting a timeout (20 sec)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define RETRY_REFUSED_TIMES   2e4 // connection refused retry times before reporting a timeout (20 sec)
/* 中文逐项-常量 X1102：RETRY_TIMEDOUT_TIMES
 * 编译期定义：RETRY_TIMEDOUT_TIMES = 3 // connection timed out retry times (each one can take 20s)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define RETRY_TIMEDOUT_TIMES    3 // connection timed out retry times (each one can take 20s)
/* 中文逐项-常量 X1103：SOCKET_NAME_MAXLEN
 * 编译期定义：SOCKET_NAME_MAXLEN = (NI_MAXHOST+NI_MAXSERV)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define SOCKET_NAME_MAXLEN (NI_MAXHOST+NI_MAXSERV)

/* Common socket address storage structure for IPv4/IPv6 */
/* 中文逐项-类型 T0377：ncclSocketAddress
 * 类型职责：IPv4/IPv6地址联合表示及基础socket状态，服务bootstrap、代理控制和TCP后端。
 * 声明内可见成员（含嵌套成员）：sa、sin、sin6。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
union ncclSocketAddress {
/* 中文逐项-变量 V3158：sa
 * sa（struct sockaddr；成员 ncclSocketAddress）：通用sockaddr、IPv4与IPv6地址视图，先看地址族再选择对应union成员。
 */
  struct sockaddr sa;
/* 中文逐项-变量 V3159：sin
 * sin（struct sockaddr_in；成员 ncclSocketAddress）：通用sockaddr、IPv4与IPv6地址视图，先看地址族再选择对应union成员。
 */
  struct sockaddr_in sin;
/* 中文逐项-变量 V3160：sin6
 * sin6（struct sockaddr_in6；成员 ncclSocketAddress）：通用sockaddr、IPv4与IPv6地址视图，先看地址族再选择对应union成员。
 */
  struct sockaddr_in6 sin6;
};

/* 中文逐项-类型 T0378：ncclSocketState
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclSocketState {
/* 中文逐项-枚举项 X1104：ncclSocketConnecting
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
  ncclSocketConnecting = 0,
/* 中文逐项-枚举项 X1105：ncclSocketConnected
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
  ncclSocketConnected = 1,
/* 中文逐项-枚举项 X1106：ncclSocketError
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
  ncclSocketError = 2,
/* 中文逐项-枚举项 X1107：ncclSocketStateNum
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
  ncclSocketStateNum = 3
} ;

/* 中文逐项-类型 T0379：ncclSocket
 * 类型职责：IPv4/IPv6地址联合表示及基础socket状态，服务bootstrap、代理控制和TCP后端。
 * 声明内可见成员（含嵌套成员）：fd、addr、abortFlag、asyncFlag、state。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocket {
/* 中文逐项-变量 V3161：fd
 * fd（int；成员 ncclSocket）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 */
  int fd;
/* 中文逐项-变量 V3162：addr
 * addr（union ncclSocketAddress；成员 ncclSocket）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
  union ncclSocketAddress addr;
/* 中文逐项-变量 V3163：abortFlag
 * abortFlag（uint32_t；成员 ncclSocket）：主机/设备可观察的异常中止共享标志，用于打破协议或 socket 等待。
 */
  volatile uint32_t* abortFlag;
/* 中文逐项-变量 V3164：asyncFlag
 * asyncFlag（int；成员 ncclSocket）：socket是否使用异步连接/进度推进方式，决定未就绪时返回还是持续等待。
 */
  int asyncFlag;
/* 中文逐项-变量 V3165：state
 * state（enum ncclSocketState；成员 ncclSocket）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 */
  enum ncclSocketState state;
};

/* 中文逐项-变量 V3166：ncclSocketToString
 * ncclSocketToString（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
const char *ncclSocketToString(union ncclSocketAddress *addr, char *buf, const int numericHostForm = 1);
/* 中文逐项-接口 X1108：ncclGetSocketAddrFromString
 * 接口职责：解析用户提供的 IP/主机及端口，构造统一 IPv4/IPv6 地址对象。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclGetSocketAddrFromString(union ncclSocketAddress* ua, const char*
 * ip_port_pair);
 */
ncclResult_t ncclGetSocketAddrFromString(union ncclSocketAddress* ua, const char* ip_port_pair);
/* 中文逐项-接口 X1109：ncclFindInterfaceMatchSubnet
 * 接口职责：按远端地址找同子网本地接口，帮助 bootstrap 选择能够到达会合端的地址。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int ncclFindInterfaceMatchSubnet(char* ifNames, union ncclSocketAddress*
 * localAddrs, union ncclSocketAddress* remoteAddr, int ifNameMaxSize, int maxIfs);
 */
int ncclFindInterfaceMatchSubnet(char* ifNames, union ncclSocketAddress* localAddrs, union ncclSocketAddress* remoteAddr, int ifNameMaxSize, int maxIfs);
/* 中文逐项-接口 X1110：ncclFindInterfaces
 * 接口职责：按显式接口配置或默认优先级选择可用接口，过滤不合适的 loopback/docker 等候选。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int ncclFindInterfaces(char* ifNames, union ncclSocketAddress *ifAddrs, int
 * ifNameMaxSize, int maxIfs);
 */
int ncclFindInterfaces(char* ifNames, union ncclSocketAddress *ifAddrs, int ifNameMaxSize, int maxIfs);
// Create a listening socket. sock->addr can be pre-filled with IP & port info. sock->fd is set after a successful call
/* 中文逐项-接口 X1111：ncclSocketListen
 * 接口职责：按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketListen(struct ncclSocket* sock);
 */
ncclResult_t ncclSocketListen(struct ncclSocket* sock);
// Connect to sock->addr. sock->fd is set after a successful call.
/* 中文逐项-接口 X1112：ncclSocketConnect
 * 接口职责：按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketConnect(struct ncclSocket* sock);
 */
ncclResult_t ncclSocketConnect(struct ncclSocket* sock);
// Return socket connection state.
/* 中文逐项-接口 X1113：ncclGetSocketState
 * 接口职责：检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclGetSocketState(struct ncclSocket* sock, enum ncclSocketState*
 * state);
 */
ncclResult_t ncclGetSocketState(struct ncclSocket* sock, enum ncclSocketState* state);
// Accept an incoming connection from listenSocket->fd and keep the file descriptor in sock->fd, with the remote side IP/port in sock->addr.
/* 中文逐项-接口 X1114：ncclSocketAccept
 * 接口职责：按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketAccept(struct ncclSocket* sock, struct ncclSocket*
 * listenSocket);
 */
ncclResult_t ncclSocketAccept(struct ncclSocket* sock, struct ncclSocket* listenSocket);

/* 中文逐项-常量 X1115：NCCL_SOCKET_SEND
 * 编译期定义：NCCL_SOCKET_SEND = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SOCKET_SEND 0
/* 中文逐项-常量 X1116：NCCL_SOCKET_RECV
 * 编译期定义：NCCL_SOCKET_RECV = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_SOCKET_RECV 1

/* 中文逐项-接口 X1117：ncclSocketProgress
 * 接口职责：调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketProgress(int op, struct ncclSocket* sock, void* ptr, int
 * size, int* offset);
 */
ncclResult_t ncclSocketProgress(int op, struct ncclSocket* sock, void* ptr, int size, int* offset);
/* 中文逐项-接口 X1118：ncclSocketWait
 * 接口职责：重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketWait(int op, struct ncclSocket* sock, void* ptr, int size,
 * int* offset);
 */
ncclResult_t ncclSocketWait(int op, struct ncclSocket* sock, void* ptr, int size, int* offset);
/* 中文逐项-接口 X1119：ncclSocketSend
 * 接口职责：从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketSend(struct ncclSocket* sock, void* ptr, int size);
 */
ncclResult_t ncclSocketSend(struct ncclSocket* sock, void* ptr, int size);
/* 中文逐项-接口 X1120：ncclSocketRecv
 * 接口职责：从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketRecv(struct ncclSocket* sock, void* ptr, int size);
 */
ncclResult_t ncclSocketRecv(struct ncclSocket* sock, void* ptr, int size);
/* 中文逐项-接口 X1121：ncclSocketTryRecv
 * 接口职责：尝试接收并可区分对端关闭，供代理服务循环避免无限阻塞在已断开连接。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketTryRecv(struct ncclSocket* sock, void* ptr, int size, int*
 * closed);
 */
ncclResult_t ncclSocketTryRecv(struct ncclSocket* sock, void* ptr, int size, int* closed);
/* initialize a socket. */
/* 中文逐项-接口 X1122：ncclSocketInit
 * 接口职责：本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclSocketInit(struct ncclSocket* sock, union ncclSocketAddress* addr
 * = NULL, volatile uint32_t* abortFlag = NULL, int asyncFlag = 0);
 */
ncclResult_t ncclSocketInit(struct ncclSocket* sock, union ncclSocketAddress* addr = NULL, volatile uint32_t* abortFlag = NULL, int asyncFlag = 0);
#endif
