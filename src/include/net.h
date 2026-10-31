/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_INT_NET_H_
/* 中文逐项-常量 X0634：NCCL_INT_NET_H_
 * 编译期定义：NCCL_INT_NET_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_INT_NET_H_

#include "nccl.h"
#include "nccl_net.h"
#include "checks.h"

/* 中文逐项-变量 V2771：ncclNet
 * ncclNet（ncclNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：static const char* ncclNetName() { return ncclNet->name; }。若由输出参数填入，必须先检查填充调用成功再读值。
 */
extern ncclNet_t* ncclNet;
typedef char ncclNetHandle_t[NCCL_NET_HANDLE_MAXSIZE];

/* 中文逐项-接口 X0635：ncclNetInit
 * 接口职责：总是初始化 bootstrap，再按外部插件、内置 IB、内置 Socket 顺序选可用数据后端，支持显式名字筛选。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNetInit();
 */
ncclResult_t ncclNetInit();
/* 中文逐项-接口 X0636：ncclNetVersion
 * 接口职责：报告当前选中网络函数表是 v4 兼容还是原生 v5，供依赖接口能力的策略判断。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：int ncclNetVersion();
 */
int ncclNetVersion();

/* 中文导读：
 * 统一 NET 回调层：把 transport 的操作转发给 ncclNet 当前指向的后端函数表。
 * 相同 regMr/isend 接口在 IB、Socket 或外部插件中的实现不同；不能只凭接口名判断有多少硬件 MR/QP。
 */
// Translation to external API
/* 中文逐项-函数 F0660：ncclNetName
 * 用途与位置：读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclInit；src/transport/net.cc::sendSetup；src/transport/net.cc::recvSetup。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclNet->name;。CHECK类宏还可能提前返回/跳转。
 */
static const char* ncclNetName() { return ncclNet->name; }
/* 中文逐项-函数 F0661：ncclNetDevices
 * 用途与位置：查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 ndev（int* ndev）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem；src/net.cc::ncclGpuGdrSupport。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->devices。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetDevices(int* ndev) { NCCLCHECK(ncclNet->devices(ndev)); return ncclSuccess; }
/* 中文逐项-函数 F0662：ncclNetGetProperties
 * 用途与位置：读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 props（ncclNetProperties_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystem；src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::sendProxySetup；src/transport/net.cc::recvProxySetup。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->getProperties。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetGetProperties(int dev, ncclNetProperties_t* props) { NCCLCHECK(ncclNet->getProperties(dev, props)); return ncclSuccess; }
/* 中文逐项-函数 F0663：ncclNetListen
 * 用途与位置：创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::recvProxySetup。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->listen。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetListen(int dev, void* handle, void** listenComm) { NCCLCHECK(ncclNet->listen(dev, handle, listenComm)); return ncclSuccess; }
/* 中文逐项-函数 F0664：ncclNetConnect
 * 用途与位置：推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 sendComm（void** sendComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::sendProxyConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->connect。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetConnect(int dev, void* handle, void** sendComm) { NCCLCHECK(ncclNet->connect(dev, handle, sendComm)); return ncclSuccess; }
/* 中文逐项-函数 F0665：ncclNetAccept
 * 用途与位置：推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 recvComm（void** recvComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::recvProxyConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->accept。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetAccept(void* listenComm, void** recvComm) { NCCLCHECK(ncclNet->accept(listenComm, recvComm)); return ncclSuccess; }
/* 中文逐项-函数 F0666：ncclNetRegMr
 * 用途与位置：注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::sendProxyConnect；src/transport/net.cc::recvProxyConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->regMr。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetRegMr(void* comm, void* data, int size, int type, void** mhandle) { NCCLCHECK(ncclNet->regMr(comm, data, size, type, mhandle)); return ncclSuccess; }
/* 中文逐项-函数 F0667：ncclNetDeregMr
 * 用途与位置：解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::sendProxyFree；src/transport/net.cc::recvProxyFree。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->deregMr。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetDeregMr(void* comm, void* mhandle) { NCCLCHECK(ncclNet->deregMr(comm, mhandle)); return ncclSuccess; }
/* 中文逐项-函数 F0668：ncclNetIsend
 * 用途与位置：提交异步发送请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net.cc::sendProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->isend。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetIsend(void* sendComm, void* data, int size, int tag, void* mhandle, void** request) { NCCLCHECK(ncclNet->isend(sendComm, data, size, tag, mhandle, request)); return ncclSuccess; }
/* 中文逐项-函数 F0669：ncclNetIrecv
 * 用途与位置：提交一组带地址/长度/tag的异步接收；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 tags（int* tags）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->irecv。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetIrecv(void* recvComm, int n, void** data, int* sizes, int* tags, void** mhandles, void** request) { NCCLCHECK(ncclNet->irecv(recvComm, n, data, sizes, tags, mhandles, request)); return ncclSuccess; }
/* 中文逐项-函数 F0670：ncclNetIflush
 * 用途与位置：提交接收缓冲可见性刷新请求；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->iflush。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetIflush(void* recvComm, int n, void** data, int* sizes, void** mhandles, void** request) { NCCLCHECK(ncclNet->iflush(recvComm, n, data, sizes, mhandles, request)); return ncclSuccess; }
/* 中文逐项-函数 F0671：ncclNetTest
 * 用途与位置：查询并推进异步请求完成；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->test。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetTest(void* request, int* done, int* sizes) { NCCLCHECK(ncclNet->test(request, done, sizes)); return ncclSuccess; }
/* 中文逐项-函数 F0672：ncclNetCloseSend
 * 用途与位置：关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::sendProxyFree。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->closeSend。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetCloseSend(void* sendComm) { NCCLCHECK(ncclNet->closeSend(sendComm)); return ncclSuccess; }
/* 中文逐项-函数 F0673：ncclNetCloseRecv
 * 用途与位置：关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::recvProxyFree。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->closeRecv。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetCloseRecv(void* recvComm) { NCCLCHECK(ncclNet->closeRecv(recvComm)); return ncclSuccess; }
/* 中文逐项-函数 F0674：ncclNetCloseListen
 * 用途与位置：关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclGpuGdrSupport；src/transport/net.cc::recvProxyConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet->closeListen。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNetCloseListen(void* listenComm) { NCCLCHECK(ncclNet->closeListen(listenComm)); return ncclSuccess; }

// Test whether the current GPU support GPU Direct RDMA.
/* 中文逐项-接口 X0637：ncclGpuGdrSupport
 * 接口职责：优先查询 CUDA GDR 属性；旧运行时通过临时连接和 GPU 缓冲注册探测，探测资源用后回收。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclGpuGdrSupport(int* gdrSupport);
 */
ncclResult_t ncclGpuGdrSupport(int* gdrSupport);

/* 中文逐项-变量 V2772：ncclNetIb
 * ncclNetIb（ncclNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern ncclNet_t ncclNetIb;
/* 中文逐项-变量 V2773：ncclNetSocket
 * ncclNetSocket（ncclNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern ncclNet_t ncclNetSocket;

#endif
