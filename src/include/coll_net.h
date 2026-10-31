/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef COLL_NET_H_
/* 中文逐项-常量 X0147：COLL_NET_H_
 * 编译期定义：COLL_NET_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define COLL_NET_H_

#include "nccl.h"
#include "nccl_net.h"

/* 中文逐项-变量 V1985：ncclCollNet
 * ncclCollNet（ncclCollNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：static const char* collNetName() { return ncclCollNet->name; }。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern ncclCollNet_t* ncclCollNet;
typedef char collNetHandle_t[NCCL_NET_HANDLE_MAXSIZE];

// Translation to external API
/* 中文逐项-函数 F0613：collNetName
 * 用途与位置：读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendSetup；src/transport/coll_net.cc::recvSetup。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclCollNet->name;。CHECK类宏还可能提前返回/跳转。
 */
static const char* collNetName() { return ncclCollNet->name; }
/* 中文逐项-函数 F0614：collNetDevices
 * 用途与位置：查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 ndev（int* ndev）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->devices。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetDevices(int* ndev) { NCCLCHECK(ncclCollNet->devices(ndev)); return ncclSuccess; }
/* 中文逐项-函数 F0615：collNetGetProperties
 * 用途与位置：读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 props（ncclNetProperties_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->getProperties。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetGetProperties(int dev, ncclNetProperties_t* props) { NCCLCHECK(ncclCollNet->getProperties(dev, props)); return ncclSuccess; }
/* 中文逐项-函数 F0616：collNetListen
 * 用途与位置：创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sharedListen。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->listen。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetListen(int dev, void* handle, void** listenComm) { NCCLCHECK(ncclCollNet->listen(dev, handle, listenComm)); return ncclSuccess; }
/* 中文逐项-函数 F0617：collNetConnect
 * 用途与位置：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 handles（void* handles[]）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 collComm（void** collComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sharedConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->connect。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetConnect(void* handles[], int nranks, int rank, void* listenComm, void** collComm) { NCCLCHECK(ncclCollNet->connect(handles, nranks, rank, listenComm, collComm)); return ncclSuccess; }
/* 中文逐项-函数 F0618：collNetReduceSupport
 * 用途与位置：查询某数据类型和归约操作能否在网络内完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 dataType（ncclDataType_t dataType）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 redOp（ncclRedOp_t redOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 supported（int* supported）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::getCollNetSupport。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->reduceSupport。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetReduceSupport(ncclDataType_t dataType, ncclRedOp_t redOp, int* supported) { NCCLCHECK(ncclCollNet->reduceSupport(dataType, redOp, supported)); return ncclSuccess; }
/* 中文逐项-函数 F0619：collNetRegMr
 * 用途与位置：注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyConnect；src/transport/coll_net.cc::recvProxyConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->regMr。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetRegMr(void* comm, void* data, int size, int type, void** mhandle) { NCCLCHECK(ncclCollNet->regMr(comm, data, size, type, mhandle)); return ncclSuccess; }
/* 中文逐项-函数 F0620：collNetDeregMr
 * 用途与位置：解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyFree；src/transport/coll_net.cc::recvProxyFree。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->deregMr。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetDeregMr(void* comm, void* mhandle) { NCCLCHECK(ncclCollNet->deregMr(comm, mhandle)); return ncclSuccess; }
/* 中文逐项-函数 F0621：collNetIallreduce
 * 用途与位置：提交网络集合归约请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 sendData（void* sendData）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 参数 recvData（void* recvData）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 参数 count（int count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 参数 dataType（ncclDataType_t dataType）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 redOp（ncclRedOp_t redOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 sendMhandle（void* sendMhandle）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 参数 recvMhandle（void* recvMhandle）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->iallreduce。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetIallreduce(void* collComm, void* sendData, void* recvData, int count, ncclDataType_t dataType, ncclRedOp_t redOp, void* sendMhandle, void* recvMhandle,  void** request) {
  NCCLCHECK(ncclCollNet->iallreduce(collComm, sendData, recvData, count, dataType, redOp, sendMhandle, recvMhandle, request)); return ncclSuccess; }
/* 中文逐项-函数 F0622：collNetIflush
 * 用途与位置：提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->iflush。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetIflush(void* collComm, void* data, int size, void* mhandle, void** request) { NCCLCHECK(ncclCollNet->iflush(collComm, data, size, mhandle, request)); return ncclSuccess; }
/* 中文逐项-函数 F0623：collNetTest
 * 用途与位置：查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 参数 size（int* size）：网络请求完成后的有效字节数输出地址；可为空时调用者不要求返回长度。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sendProxyProgress；src/transport/coll_net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->test。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetTest(void* request, int* done, int* size) { NCCLCHECK(ncclCollNet->test(request, done, size)); return ncclSuccess; }
/* 中文逐项-函数 F0624：collNetCloseColl
 * 用途与位置：关闭集合网络连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sharedFree。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->closeColl。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetCloseColl(void* collComm) { NCCLCHECK(ncclCollNet->closeColl(collComm)); return ncclSuccess; }
/* 中文逐项-函数 F0625：collNetCloseListen
 * 用途与位置：关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/coll_net.cc::sharedConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet->closeListen。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t collNetCloseListen(void* listenComm) { NCCLCHECK(ncclCollNet->closeListen(listenComm)); return ncclSuccess; }

/* 中文逐项-函数 F0626：collNetSupport
 * 用途与位置：判断已选 CollNet 接口是否存在且能使用；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem；src/init.cc::initTransportsRank。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclCollNet != nullptr ? 1 : 0;。CHECK类宏还可能提前返回/跳转。
 */
static int collNetSupport() { return ncclCollNet != nullptr ? 1 : 0; }

#endif
