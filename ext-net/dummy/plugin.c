/*************************************************************************
 * Copyright (c) 2015-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include <nccl.h>
#include <nccl_net.h>

/* 中文逐项-常量 X0001：__attribute__
 * 编译期定义：__attribute__ = ((visibility("hidden")))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define __hidden __attribute__ ((visibility("hidden")))

__hidden /* 中文内部逻辑 F0001：pluginInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0001
 */
/* 中文逐项-函数 F0001：pluginInit
 * 用途与位置：Dummy插件初始化占位，返回成功但不建立真实网络资源；该文件只演示ABI函数表。
 * 参数 logFunction（ncclDebugLogger_t logFunction）：NCCL提供的日志回调，插件用它按统一级别/子系统报告信息与错误。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginInit(ncclDebugLogger_t logFunction) { return ncclSuccess; }
__hidden /* 中文内部逻辑 F0002：pluginDevices
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*ndev = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0002
 */
/* 中文逐项-函数 F0002：pluginDevices
 * 用途与位置：Dummy插件报告零可用设备，网络选择层因此不会把它当作可传输数据的后端。
 * 参数 ndev（int* ndev）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginDevices(int* ndev) { *ndev = 0; return ncclSuccess; }
__hidden /* 中文内部逻辑 F0003：pluginPciPath
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0003
 */
/* 中文逐项-函数 F0003：pluginPciPath
 * 用途与位置：Dummy插件的 pluginPciPath 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 path（char** path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginPciPath(int dev, char** path) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0004：pluginPtrSupport
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0004
 */
/* 中文逐项-函数 F0004：pluginPtrSupport
 * 用途与位置：Dummy插件的 pluginPtrSupport 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 supportedTypes（int* supportedTypes）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginPtrSupport(int dev, int* supportedTypes) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0005：pluginListen
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0005
 */
/* 中文逐项-函数 F0005：pluginListen
 * 用途与位置：Dummy插件的 pluginListen 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginListen(int dev, void* handle, void** listenComm) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0006：pluginConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0006
 */
/* 中文逐项-函数 F0006：pluginConnect
 * 用途与位置：Dummy插件的 pluginConnect 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 sendComm（void** sendComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginConnect(int dev, void* handle, void** sendComm) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0007：pluginAccept
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0007
 */
/* 中文逐项-函数 F0007：pluginAccept
 * 用途与位置：Dummy插件的 pluginAccept 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 recvComm（void** recvComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginAccept(void* listenComm, void** recvComm) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0008：pluginRegMr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0008
 */
/* 中文逐项-函数 F0008：pluginRegMr
 * 用途与位置：Dummy插件的 pluginRegMr 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginRegMr(void* collComm, void* data, int size, int type, void** mhandle) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0009：pluginDeregMr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0009
 */
/* 中文逐项-函数 F0009：pluginDeregMr
 * 用途与位置：Dummy插件的 pluginDeregMr 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginDeregMr(void* collComm, void* mhandle) { return ncclInternalError;}
__hidden /* 中文内部逻辑 F0010：pluginIsend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0010
 */
/* 中文逐项-函数 F0010：pluginIsend
 * 用途与位置：Dummy插件的 pluginIsend 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginIsend(void* sendComm, void* data, int size, void* mhandle, void** request) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0011：pluginIrecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0011
 */
/* 中文逐项-函数 F0011：pluginIrecv
 * 用途与位置：Dummy插件的 pluginIrecv 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginIrecv(void* recvComm, void* data, int size, void* mhandle, void** request) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0012：pluginFlush
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0012
 */
/* 中文逐项-函数 F0012：pluginFlush
 * 用途与位置：Dummy插件的 pluginFlush 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginFlush(void* recvComm, void* data, int size, void* mhandle) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0013：pluginTest
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0013
 */
/* 中文逐项-函数 F0013：pluginTest
 * 用途与位置：Dummy插件的 pluginTest 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 参数 size（int* size）：网络请求完成后的有效字节数输出地址；可为空时调用者不要求返回长度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginTest(void* request, int* done, int* size) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0014：pluginCloseSend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0014
 */
/* 中文逐项-函数 F0014：pluginCloseSend
 * 用途与位置：Dummy插件的 pluginCloseSend 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCloseSend(void* sendComm) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0015：pluginCloseRecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0015
 */
/* 中文逐项-函数 F0015：pluginCloseRecv
 * 用途与位置：Dummy插件的 pluginCloseRecv 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCloseRecv(void* recvComm) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0016：pluginCloseListen
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0016
 */
/* 中文逐项-函数 F0016：pluginCloseListen
 * 用途与位置：Dummy插件的 pluginCloseListen 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCloseListen(void* listenComm) { return ncclInternalError; }

/* 中文逐项-变量 V0001：NCCL_PLUGIN_SYMBOL
 * NCCL_PLUGIN_SYMBOL（ncclNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 NCCL_PLUGIN_SYMBOL = { "Dummy", pluginInit, pluginDevices, pluginPciPath, pluginPtrSupport, pluginListen,
 * pluginConnect, pluginAccept, pluginRegMr, pluginDeregMr, pluginIsend, pluginIrecv, pluginFlush, pluginTest,
 * pluginCloseSend, pluginCloseRecv, plu …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
ncclNet_t NCCL_PLUGIN_SYMBOL = {
  "Dummy",
  pluginInit,
  pluginDevices,
  pluginPciPath,
  pluginPtrSupport,
  pluginListen,
  pluginConnect,
  pluginAccept,
  pluginRegMr,
  pluginDeregMr,
  pluginIsend,
  pluginIrecv,
  pluginFlush,
  pluginTest,
  pluginCloseSend,
  pluginCloseRecv,
  pluginCloseListen
};

__hidden /* 中文内部逻辑 F0017：pluginCollNetInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0017
 */
/* 中文逐项-函数 F0017：pluginCollNetInit
 * 用途与位置：Dummy插件初始化占位，返回成功但不建立真实网络资源；该文件只演示ABI函数表。
 * 参数 logFunction（ncclDebugLogger_t logFunction）：NCCL提供的日志回调，插件用它按统一级别/子系统报告信息与错误。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetInit(ncclDebugLogger_t logFunction) { return ncclSuccess; }
__hidden /* 中文内部逻辑 F0018：pluginCollNetDevices
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*ndev = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0018
 */
/* 中文逐项-函数 F0018：pluginCollNetDevices
 * 用途与位置：Dummy插件报告零可用设备，网络选择层因此不会把它当作可传输数据的后端。
 * 参数 ndev（int* ndev）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetDevices(int* ndev) { *ndev = 0; return ncclSuccess; }
__hidden /* 中文内部逻辑 F0019：pluginCollNetPciPath
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0019
 */
/* 中文逐项-函数 F0019：pluginCollNetPciPath
 * 用途与位置：Dummy插件的 pluginCollNetPciPath 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 path（char** path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetPciPath(int dev, char** path) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0020：pluginCollNetPtrSupport
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0020
 */
/* 中文逐项-函数 F0020：pluginCollNetPtrSupport
 * 用途与位置：Dummy插件的 pluginCollNetPtrSupport 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 supportedTypes（int* supportedTypes）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetPtrSupport(int dev, int* supportedTypes) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0021：pluginCollNetListen
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0021
 */
/* 中文逐项-函数 F0021：pluginCollNetListen
 * 用途与位置：Dummy插件的 pluginCollNetListen 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 handle（void* handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetListen(int dev, void* handle, void** listenComm) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0022：pluginCollNetConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0022
 */
/* 中文逐项-函数 F0022：pluginCollNetConnect
 * 用途与位置：Dummy插件的 pluginCollNetConnect 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 handles（void* handles[]）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 collComm（void** collComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetConnect(void* handles[], int nranks, int rank, void* listenComm, void** collComm) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0023：pluginCollNetReduceSupport
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0023
 */
/* 中文逐项-函数 F0023：pluginCollNetReduceSupport
 * 用途与位置：Dummy插件的 pluginCollNetReduceSupport 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 dataType（ncclDataType_t dataType）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 redOp（ncclRedOp_t redOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 supported（int* supported）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetReduceSupport(ncclDataType_t dataType, ncclRedOp_t redOp, int* supported) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0024：pluginCollNetRegMr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0024
 */
/* 中文逐项-函数 F0024：pluginCollNetRegMr
 * 用途与位置：Dummy插件的 pluginCollNetRegMr 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetRegMr(void* collComm, void* data, int size, int type, void** mhandle) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0025：pluginCollNetDeregMr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0025
 */
/* 中文逐项-函数 F0025：pluginCollNetDeregMr
 * 用途与位置：Dummy插件的 pluginCollNetDeregMr 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetDeregMr(void* collComm, void* mhandle) { return ncclInternalError;}
__hidden /* 中文内部逻辑 F0026：pluginCollNetIallreduce
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0026
 */
/* 中文逐项-函数 F0026：pluginCollNetIallreduce
 * 用途与位置：Dummy插件的 pluginCollNetIallreduce 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 sendData（void* sendData）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 参数 recvData（void* recvData）：本次网络/设备操作的输入或输出缓冲地址，配套count/size限定有效范围。
 * 参数 count（int count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 参数 dataType（ncclDataType_t dataType）：实际元素类型枚举，决定每元素字节数与网络/设备归约支持组合。
 * 参数 redOp（ncclRedOp_t redOp）：数值归约操作枚举/句柄，决定求和、乘积、最值等数学结合；不是数据类型编号。
 * 参数 sendMhandle（void* sendMhandle）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 参数 recvMhandle（void* recvMhandle）：对应输入/输出缓冲已经注册到该后端的内存句柄，须与data地址和有效范围匹配。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetIallreduce(void* collComm, void* sendData, void* recvData, int count, ncclDataType_t dataType, ncclRedOp_t redOp, void* sendMhandle, void* recvMhandle, void** request) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0027：pluginCollNetFlush
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0027
 */
/* 中文逐项-函数 F0027：pluginCollNetFlush
 * 用途与位置：Dummy插件的 pluginCollNetFlush 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetFlush(void* collComm, void* data, int size, void* mhandle) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0028：pluginCollNetTest
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0028
 */
/* 中文逐项-函数 F0028：pluginCollNetTest
 * 用途与位置：Dummy插件的 pluginCollNetTest 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 参数 size（int* size）：网络请求完成后的有效字节数输出地址；可为空时调用者不要求返回长度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetTest(void* request, int* done, int* size) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0029：pluginCollNetCloseColl
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0029
 */
/* 中文逐项-函数 F0029：pluginCollNetCloseColl
 * 用途与位置：Dummy插件的 pluginCollNetCloseColl 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 collComm（void* collComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetCloseColl(void* collComm) { return ncclInternalError; }
__hidden /* 中文内部逻辑 F0030：pluginCollNetCloseListen
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/ext-net__dummy__plugin_c.md#f0030
 */
/* 中文逐项-函数 F0030：pluginCollNetCloseListen
 * 用途与位置：Dummy插件的 pluginCollNetCloseListen 接口占位，直接返回ncclInternalError；此示例没有真实传输/注册实现，不能当可用后端。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */ncclResult_t pluginCollNetCloseListen(void* listenComm) { return ncclInternalError; }

/* 中文逐项-变量 V0002：NCCL_COLLNET_PLUGIN_SYMBOL
 * NCCL_COLLNET_PLUGIN_SYMBOL（ncclCollNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 NCCL_COLLNET_PLUGIN_SYMBOL = { "Dummy", pluginCollNetInit, pluginCollNetDevices, pluginCollNetPciPath,
 * pluginCollNetPtrSupport, pluginCollNetListen, pluginCollNetConnect, pluginCollNetReduceSupport, pluginCollNetRegMr,
 * pluginCollNetDeregMr, pluginCollNetIallr …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
ncclCollNet_t NCCL_COLLNET_PLUGIN_SYMBOL = {
  "Dummy",
  pluginCollNetInit,
  pluginCollNetDevices,
  pluginCollNetPciPath,
  pluginCollNetPtrSupport,
  pluginCollNetListen,
  pluginCollNetConnect,
  pluginCollNetReduceSupport,
  pluginCollNetRegMr,
  pluginCollNetDeregMr,
  pluginCollNetIallreduce,
  pluginCollNetFlush,
  pluginCollNetTest,
  pluginCollNetCloseColl,
  pluginCollNetCloseListen
};
