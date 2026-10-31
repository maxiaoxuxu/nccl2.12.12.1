/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "group.h"
#include "debug.h"
#include "enqueue.h"
#include "transport.h"
#include "channel.h"

/* 中文逐项-常量 X0117：MAX_ASYNC_OPS
 * 编译期定义：MAX_ASYNC_OPS = 128。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_ASYNC_OPS 128
/* 中文逐项-变量 V1874：ncclGroupThreads
 * ncclGroupThreads（pthread_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadMain, args);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
thread_local pthread_t ncclGroupThreads[MAX_ASYNC_OPS];
/* 中文逐项-变量 V1875：ncclGroupIndex
 * ncclGroupIndex（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclGroupIndex = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
thread_local int ncclGroupIndex = 0;
/* 中文逐项-变量 V1876：ncclGroupMode
 * ncclGroupMode（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclGroupMode = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
thread_local int ncclGroupMode = 0;
/* 中文逐项-变量 V1877：ncclGroupError
 * ncclGroupError（ncclResult_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclGroupError = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
thread_local ncclResult_t ncclGroupError = ncclSuccess;

/* 中文逐项-函数 F0589：ncclAsyncMode
 * 用途与位置：查询本线程是否处于 Group 嵌套内部，决定 API 立即准备还是延迟到最外层 GroupEnd。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/init.cc::ncclCommInitRankDev。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclGroupMode > 0;。CHECK类宏还可能提前返回/跳转。
 */
bool ncclAsyncMode() {
  return ncclGroupMode > 0;
}

/* 中文逐项-函数 F0590：ncclAsyncErrCheck
 * 用途与位置：把组内调用错误记录到线程局部 Group 状态，供统一清理和返回。
 * 参数 ret（ncclResult_t ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck；src/group.cc::ncclAsyncInit；src/group.cc::ncclAsyncColl；src/init.cc::ncclCommInitRankDev。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：ncclGroupError == ncclSuccess || ret != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ret;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclAsyncErrCheck(ncclResult_t ret) {
  if (ncclGroupError == ncclSuccess || ret != ncclSuccess) ncclGroupError = ret;
  return ret;
}

/* 中文逐项-类型 T0157：ncclInitArgs
 * 类型职责：Group保存的初始化或通信提交参数，外层GroupEnd按funcType执行正确路径。
 * 声明内可见成员（含嵌套成员）：func、cudaDev、newcomm、ndev、commId、myrank。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclInitArgs {
/* 中文逐项-变量 V1878：func
 * func（ncclInitFunc_t；成员 ncclInitArgs）：函数指针或 CUDA kernel 入口地址，由所选算法/函数表/解析器回调决定。
 */
  ncclInitFunc_t func;
/* 中文逐项-变量 V1879：cudaDev
 * cudaDev（int；成员 ncclInitArgs）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 */
  int cudaDev;
/* 中文逐项-变量 V1880：newcomm
 * newcomm（ncclComm_t；成员 ncclInitArgs）：公开初始化结果句柄的输出地址，只有初始化成功后才能交给后续通信调用。
 */
  ncclComm_t* newcomm;
/* 中文逐项-变量 V1881：ndev
 * ndev（int；成员 ncclInitArgs）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 */
  int ndev;
/* 中文逐项-变量 V1882：commId
 * commId（ncclUniqueId；成员 ncclInitArgs）：应用分发的引导会合标识，同一通信组所有rank需要使用同一份。
 */
  ncclUniqueId commId;
/* 中文逐项-变量 V1883：myrank
 * myrank（int；成员 ncclInitArgs）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int myrank;
};
/* 中文逐项-类型 T0158：ncclCollArgs
 * 类型职责：Group保存的初始化或通信提交参数，外层GroupEnd按funcType执行正确路径。
 * 声明内可见成员（含嵌套成员）：comm。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclCollArgs {
/* 中文逐项-变量 V1884：comm
 * comm（ncclComm_t；成员 ncclCollArgs）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  ncclComm_t comm;
};

/* 中文逐项-类型 T0159：ncclAsyncFuncType
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclAsyncFuncType {
/* 中文逐项-枚举项 X0118：ASYNC_FUNC_INVALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
  ASYNC_FUNC_INVALID = 0,
/* 中文逐项-枚举项 X0119：ASYNC_FUNC_INIT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
  ASYNC_FUNC_INIT = 1,
/* 中文逐项-枚举项 X0120：ASYNC_FUNC_COLL
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
  ASYNC_FUNC_COLL = 2,
};
/* 中文逐项-类型 T0160：ncclAsyncArgs
 * 类型职责：Group保存的初始化或通信提交参数，外层GroupEnd按funcType执行正确路径。
 * 声明内可见成员（含嵌套成员）：ret、funcType、coll、init。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclAsyncArgs {
/* 中文逐项-变量 V1885：ret
 * ret（ncclResult_t；成员 ncclAsyncArgs）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 */
  ncclResult_t ret;
/* 中文逐项-变量 V1886：funcType
 * funcType（enum ncclAsyncFuncType；成员 ncclAsyncArgs）：Group任务种类，决定union读取初始化参数还是通信提交参数。
 */
  enum ncclAsyncFuncType funcType;
/* 中文逐项-类型 T0161：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：coll、init。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V1887：coll
 * coll（ncclCollArgs；成员 ncclAsyncArgs::<匿名>）：本任务的公开操作类别，如 AllReduce、Broadcast、Send 或 Recv。
 */
    ncclCollArgs coll;
/* 中文逐项-变量 V1888：init
 * init（ncclInitArgs；成员 ncclAsyncArgs::<匿名>）：初始化任务参数或后端初始化回调，按其声明为对象还是函数指针分别解释。
 */
    ncclInitArgs init;
  };
};

/* 中文逐项-变量 V1889：ncclGroupArgs
 * ncclGroupArgs（struct ncclAsyncArgs；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：struct ncclAsyncArgs* args = ncclGroupArgs+index;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
thread_local struct ncclAsyncArgs ncclGroupArgs[MAX_ASYNC_OPS];

/* 中文逐项-函数 F0591：ncclAsyncThreadMain
 * 用途与位置：后台执行一项延迟 communicator 初始化，并把结果存回组任务参数。
 * 参数 args_（void* args_）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECKTHREAD；args->init.func。
 * 源码返回表达式：return args;。CHECK类宏还可能提前返回/跳转。
 */
void* ncclAsyncThreadMain(void* args_) {
/* 中文逐项-变量 V1890：args
 * args（struct ncclAsyncArgs；函数 ncclAsyncThreadMain 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = (struct ncclAsyncArgs*)args_；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;
  NCCLCHECKTHREAD(args->init.func(args->init.newcomm, args->init.ndev, args->init.commId, args->init.myrank, args->init.cudaDev));
  return args;
}

/* 中文逐项-函数 F0592：ncclAsyncInit
 * 用途与位置：把初始化函数、rank、设备和 uniqueId 复制到本线程 Group 任务数组。
 * 参数 func（ncclInitFunc_t func）：函数指针或 CUDA kernel 入口地址，由所选算法/函数表/解析器回调决定。
 * 参数 newcomm（ncclComm_t* newcomm）：公开初始化结果句柄的输出地址，只有初始化成功后才能交给后续通信调用。
 * 参数 ndev（int ndev）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 * 参数 commId（ncclUniqueId commId）：应用分发的引导会合标识，同一通信组所有rank需要使用同一份。
 * 参数 myrank（int myrank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 cudaDev（int cudaDev）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitRankDev。
 * 本体直接出现的调用/包装：memcpy；ncclAsyncErrCheck。
 * 需要对照的边界/推进条件：ncclGroupIndex >= MAX_ASYNC_OPS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclAsyncErrCheck(ncclInvalidUsage);；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclAsyncInit(ncclInitFunc_t func, ncclComm_t* newcomm, int ndev, ncclUniqueId commId, int myrank, int cudaDev) {
  if (ncclGroupIndex >= MAX_ASYNC_OPS) {
    WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);
    return ncclAsyncErrCheck(ncclInvalidUsage);
  }
/* 中文逐项-变量 V1891：index
 * index（int；函数 ncclAsyncInit 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 初值/来源 index = ncclGroupIndex++；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int index = ncclGroupIndex++;
/* 中文逐项-变量 V1892：args
 * args（struct ncclAsyncArgs；函数 ncclAsyncInit 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+index；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclAsyncArgs* args = ncclGroupArgs+index;
  args->funcType = ASYNC_FUNC_INIT;
  args->init.func = func;
  args->init.cudaDev = cudaDev;
  args->init.newcomm = newcomm;
  args->init.ndev = ndev;
  memcpy(&args->init.commId, &commId, sizeof(commId));
  args->init.myrank = myrank;
  return ncclSuccess;
}

/* 中文逐项-函数 F0593：ncclAsyncColl
 * 用途与位置：把 communicator 加入本批 Group 提交集合，重复遇到相同 comm 时避免多次登记。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCheck。
 * 本体直接出现的调用/包装：ncclAsyncErrCheck。
 * 需要对照的边界/推进条件：i<ncclGroupIndex；args->coll.comm == comm；ncclGroupIndex >= MAX_ASYNC_OPS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclAsyncErrCheck(ncclInvalidUsage);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclAsyncColl(ncclComm_t comm) {
/* 中文逐项-变量 V1893：args
 * args（struct ncclAsyncArgs；函数 ncclAsyncColl 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  struct ncclAsyncArgs* args = ncclGroupArgs;
  for (/* 中文逐项-变量 V1894：i
 * i（int；函数 ncclAsyncColl 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
    if (args->coll.comm == comm) return ncclSuccess;
    args++;
  }
  if (ncclGroupIndex >= MAX_ASYNC_OPS) {
    WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);
    return ncclAsyncErrCheck(ncclInvalidUsage);
  }
  ncclGroupIndex++;
  args->funcType = ASYNC_FUNC_COLL;
  args->coll.comm = comm;
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclGroupStart);
/* 中文导读：
 * 增加线程局部的 group 嵌套深度；组内通信调用先记录，不立即按单次 API 路径完整启动。
 * 用于多 GPU 发射协调及批量 Send/Recv。嵌套 group 只有最外层 GroupEnd 才真正处理。
 */
/* 中文逐项-函数 F0594：ncclGroupStart
 * 用途与位置：进入线程局部 Group 嵌套，最外层首次进入时清理组任务状态。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/sendrecv.cc::ncclSend；src/collectives/sendrecv.cc::ncclRecv；src/init.cc::ncclCommInitAll。
 * 本体直接出现的调用/包装：memset。
 * 需要对照的边界/推进条件：ncclGroupMode == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGroupStart() {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (ncclGroupMode == 0) {
    memset(ncclGroupArgs, 0, sizeof(struct ncclAsyncArgs)*MAX_ASYNC_OPS);
  }
  ncclGroupMode++;
  return ncclSuccess;
}

/* 中文逐项-函数 F0595：scheduleSend
 * 用途与位置：把 Group 切出的字节块封装为 P2P 任务，按 peer/块号映射 channel 后调用 SetupP2pKernel。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 chunk（int chunk）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 count（size_t count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 参数 buff（void* buff）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclChannelCompute；ncclSetupP2pKernel。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t scheduleSend(struct ncclComm* comm, int peer, int chunk, size_t count, void* buff) {
/* 中文逐项-变量 V1895：info
 * info（struct ncclInfo；函数 scheduleSend 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncSend, "Send", NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, / * Args *
 * / 1, 1 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncSend, "Send",
    NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, /* Args */
    1, 1 };
/* 中文逐项-变量 V1896：channelId
 * channelId（int；函数 scheduleSend 内）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncSend,
 * &channelId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int channelId;
  NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncSend, &channelId));
  info.channelId = channelId;
  NCCLCHECK(ncclSetupP2pKernel(&info));
  return ncclSuccess;
}
/* 中文逐项-函数 F0596：scheduleRecv
 * 用途与位置：把 Group 切出的字节块封装为 P2P 任务，按 peer/块号映射 channel 后调用 SetupP2pKernel。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 chunk（int chunk）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 count（size_t count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 参数 buff（void* buff）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclChannelCompute；ncclSetupP2pKernel。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t scheduleRecv(struct ncclComm* comm, int peer, int chunk, size_t count, void* buff) {
/* 中文逐项-变量 V1897：info
 * info（struct ncclInfo；函数 scheduleRecv 内）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 初值/来源 info = { ncclFuncRecv, "Recv", NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, / * Args *
 * / 1, 1 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct ncclInfo info = { ncclFuncRecv, "Recv",
    NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, /* Args */
    1, 1 };
/* 中文逐项-变量 V1898：channelId
 * channelId（int；函数 scheduleRecv 内）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncRecv,
 * &channelId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int channelId;
  NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncRecv, &channelId));
  info.channelId = channelId;
  NCCLCHECK(ncclSetupP2pKernel(&info));
  return ncclSuccess;
}

/* 中文逐项-函数 F0597：ncclAsyncThreadPreconnect
 * 用途与位置：在正确 CUDA 设备和 CPU 亲和性下完成本批 P2P 缺失连接的预建链。
 * 参数 args_（void* args_）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CPU_COUNT；CUDACHECKTHREAD；NCCLCHECKTHREAD；cudaSetDevice；ncclTransportP2pSetup；sched_setaffinity。
 * 需要对照的边界/推进条件：CPU_COUNT(&comm->cpuAffinity。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return args;。CHECK类宏还可能提前返回/跳转。
 */
void* ncclAsyncThreadPreconnect(void* args_) {
/* 中文逐项-变量 V1899：args
 * args（struct ncclAsyncArgs；函数 ncclAsyncThreadPreconnect 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = (struct ncclAsyncArgs*)args_；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;
/* 中文逐项-变量 V1900：comm
 * comm（struct ncclComm；函数 ncclAsyncThreadPreconnect 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = args->coll.comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclComm* comm = args->coll.comm;
  CUDACHECKTHREAD(cudaSetDevice(comm->cudaDev));
  if (CPU_COUNT(&comm->cpuAffinity)) sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);
  NCCLCHECKTHREAD(ncclTransportP2pSetup(comm, NULL, 1));
  return args;
}

/* 中文精读：P2P 的第一层切块：把一条消息铺到若干 channel
 * 从 totalSize/minChannels 得到每块候选尺寸；若太大，就在 maxChannels 范围内倍增通道数。
 * 最后按 minSize 向上对齐；调用处 minSize 是 Simple 的一个 step，避免产生过碎的小块。
 * 大块降低调度开销，小块增加并行度；实际折中还考虑同一 work 内同时活跃的 peer 数。
 * 这层 chunk 是 Group 给 channel 的消息块，设备 runSend/runRecv 内还会继续分成协议块。
 */
/* 中文逐项-函数 F0598：getP2pChunkSize
 * 用途与位置：在最小/最大通道预算内倍增并行度，将每 peer 消息划成按 step 对齐的块。
 * 参数 totalSize（size_t totalSize）：排队有效数据规模统计，供负载平衡或聚合选型使用。
 * 参数 minChannels（int minChannels）：图/后端允许的逻辑通道上限和下限，受容量与配置共同约束。
 * 参数 maxChannels（int maxChannels）：图/后端允许的逻辑通道上限和下限，受容量与配置共同约束。
 * 参数 minSize（size_t minSize）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 参数 maxSize（size_t maxSize）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：ALIGN_SIZE；DIVUP；std::max。
 * 需要对照的边界/推进条件：size > maxSize && nChannels <= maxChannels/2。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return size;。CHECK类宏还可能提前返回/跳转。
 */
static size_t getP2pChunkSize(size_t totalSize, int minChannels, int maxChannels, size_t minSize, size_t maxSize) {
/* 中文逐项-变量 V1901：size
 * size（size_t；函数 getP2pChunkSize 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = std::max(minSize, DIVUP(totalSize, minChannels))；保留下限或较大候选，避免并行度/尺寸低于需要的最小值。
 */
  size_t size = std::max(minSize, DIVUP(totalSize, minChannels));
/* 中文逐项-变量 V1902：nChannels
 * nChannels（int；函数 getP2pChunkSize 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = minChannels；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int nChannels = minChannels;
  while (size > maxSize && nChannels <= maxChannels/2) {
    nChannels *= 2;
    size = DIVUP(totalSize, nChannels);
  }
  ALIGN_SIZE(size, minSize);
  return size;
}

NCCL_API(ncclResult_t, ncclGroupEnd);
/* 中文导读：
 * group 的实际提交点：执行初始化任务、补建 P2P 连接、按节点/peer 分块调度通信、准备并启动 kernels。
 * 将同进程多个 communicator 的准备与发射阶段分开，避免一部分 GPU 还未启动而其他路径先阻塞。
 * 调用完成仍不代表 CUDA stream 上的全部通信已经完成；应用需按 CUDA 流/事件规则等待。
 */
/* 中文逐项-函数 F0599：ncclGroupEnd
 * 用途与位置：最外层出口统一执行初始化、P2P 预建链、消息分块、collective 聚合、Proxy 发布和 kernel 发射。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/collectives/sendrecv.cc::ncclSend；src/collectives/sendrecv.cc::ncclRecv；src/init.cc::ncclCommInitAll。
 * 本体直接出现的调用/包装：CUDACHECK；CUDACHECKGOTO；NCCLCHECK；NCCLCHECKGOTO；comm->p2pRecvs[peer]->recycle；comm->p2pRecvs[recvPeer]->getNext；comm->p2pRecvs[recvPeer]->peakNext；comm->p2pRecvs[recvPeer]->recycle；comm->p2pSends[peer]->recycle；comm->p2pSends[sendPeer]->getNext；comm->p2pSends[sendPeer]->peakNext；comm->p2pSends[sendPeer]->recycle；cudaGetDevice；cudaSetDevice；free；getP2pChunkSize；ncclCalloc；ncclCommDestroy；更多调用保留在函数体。
 * 需要对照的边界/推进条件：ncclGroupMode == 0；ncclGroupMode > 0；i<ncclGroupIndex；ret != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclSuccess;；return ncclSystemError;；return ret;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclGroupEnd() {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (ncclGroupMode == 0) {
    WARN("ncclGroupEnd: not in a group call.");
    return ncclInvalidUsage;
  }
  ncclGroupMode--;
  if (ncclGroupMode > 0) return ncclSuccess;
/* 中文逐项-变量 V1903：savedDev
 * savedDev（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaGetDevice(&savedDev));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int savedDev;
  CUDACHECK(cudaGetDevice(&savedDev));
/* 中文逐项-变量 V1904：activeThreads
 * activeThreads（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 activeThreads = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int activeThreads = 0;
/* 中文逐项-变量 V1905：doneArray
 * doneArray（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (int i=0; i<ncclGroupIndex; i++) doneArray[i] = 1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int doneArray[MAX_ASYNC_OPS];
  for (/* 中文逐项-变量 V1906：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) doneArray[i] = 1;
/* 中文逐项-变量 V1907：ret
 * ret（ncclResult_t；函数 ncclGroupEnd 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = ncclGroupError；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  ncclResult_t ret = ncclGroupError;
/* 中文逐项-变量 V1908：usingCudaGraphAll
 * usingCudaGraphAll（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 usingCudaGraphAll = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int usingCudaGraphAll = -1;
/* 中文逐项-变量 V1909：graphs
 * graphs（cudaGraph_t；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 graphs = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  cudaGraph_t* graphs = NULL;
  if (ret != ncclSuccess) goto group_cleanup;

  /* Launch async ncclCommInitRank */
  for (/* 中文逐项-变量 V1910：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1911：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_INIT) {
      pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadMain, args);
      activeThreads++;
      doneArray[i] = 0;
    }
  }
  /* For init, since we use threads, we just wait for threads to complete */
  while (activeThreads) {
    for (/* 中文逐项-变量 V1912：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1913：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      struct ncclAsyncArgs* args = ncclGroupArgs+i;
      if (args->funcType == ASYNC_FUNC_INIT && doneArray[i] == 0) {
/* 中文逐项-变量 V1914：err
 * err（int；函数 ncclGroupEnd 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 err = pthread_tryjoin_np(ncclGroupThreads[i], NULL)；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
        int err = pthread_tryjoin_np(ncclGroupThreads[i], NULL);
        if (err == EBUSY) continue;
        if (err != 0) ret = ncclSystemError;
        if (args->ret != ncclSuccess) ret = args->ret;
        doneArray[i] = 1;
        activeThreads--;
      }
    }
  }

  for (/* 中文逐项-变量 V1915：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1916：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL && args->coll.comm->connect) {
      pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadPreconnect, args);
    }
  }

  for (/* 中文逐项-变量 V1917：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1918：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL && args->coll.comm->connect) {
/* 中文逐项-变量 V1919：err
 * err（int；函数 ncclGroupEnd 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 err = pthread_join(ncclGroupThreads[i], NULL)；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
      int err = pthread_join(ncclGroupThreads[i], NULL);
      if (err != 0) {
        WARN("Error waiting for pthread_join : %s", strerror(errno));
        return ncclSystemError;
      }
      NCCLCHECKGOTO(args->ret, ret, end);
      args->coll.comm->connect = 0;
    }
  }

  for (/* 中文逐项-变量 V1920：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1921：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
/* 中文逐项-变量 V1922：comm
 * comm（struct ncclComm；函数 ncclGroupEnd 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = args->coll.comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclComm* comm = args->coll.comm;
/* 中文逐项-变量 V1923：node
 * node（int；函数 ncclGroupEnd 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = comm->node；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int node = comm->node;
/* 中文逐项-变量 V1924：nNodes
 * nNodes（int；函数 ncclGroupEnd 内）：当前 communicator 跨越的节点数量；节点编号不等于 GPU rank。
 * 初值/来源 nNodes = comm->nNodes；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int nNodes = comm->nNodes;
/* 中文逐项-变量 V1925：localRank
 * localRank（int；函数 ncclGroupEnd 内）：本节点内 rank 下标，主要索引本地共享资源。
 * 初值/来源 localRank = comm->localRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int localRank = comm->localRank;

/* 中文精读：P2P 并行度为什么不是永远开到最大
 * 每 peer 的 channel 预算是上限；很多 peer 同时收发会争用全局 p2pnChannels。
 * 这里按 rank 数缩小 nChannelsMin/Max，减少一个 block 被塞入过多收发项而损失同步 warp 的情况。
 * 因此初始化日志中的 per peer 数不等于每条消息实际占用数；消息尺寸与同批工作量都会影响拆分。
 */
      // Compute how much to split operations
      // Natural step size matching buffer steps.
/* 中文逐项-变量 V1926：stepSize
 * stepSize（ssize_t；函数 ncclGroupEnd 内）：一 FIFO step 的容量；设备 T 指针路径以元素计，主机缓冲路径以字节计。
 * 初值/来源 stepSize = comm->buffSizes[NCCL_PROTO_SIMPLE] / NCCL_STEPS；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      ssize_t stepSize = comm->buffSizes[NCCL_PROTO_SIMPLE] / NCCL_STEPS;
      // Try to use all channels
/* 中文逐项-变量 V1927：nChannelsMax
 * nChannelsMax（int；函数 ncclGroupEnd 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nChannelsMax = comm->p2pnChannelsPerPeer；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int nChannelsMax = comm->p2pnChannelsPerPeer;
/* 中文逐项-变量 V1928：nChannelsMin
 * nChannelsMin（int；函数 ncclGroupEnd 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nChannelsMin = nChannelsMax；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int nChannelsMin = nChannelsMax;
      // Try to use all channels, but one channel per operation.
      while (nChannelsMin*comm->nRanks > comm->p2pnChannels && nChannelsMin > 1) nChannelsMin /= 2;
      // Avoid overloading channels with 8+ operations as we loose the sync warp, hence a bit of bandwidth.
      while (nChannelsMax*comm->nRanks > comm->p2pnChannels*4 && nChannelsMax > 1) nChannelsMax /= 2;

      while (comm->p2pSendCount > 0 || comm->p2pRecvCount > 0) {
/* 中文精读：为什么收发必须采用双方一致的顺序
 * 用节点相对距离 delta 和节点内步号 s 选 sendPeer/recvPeer，再按统一规则跳过重复距离。
 * 这是显式排列 peer 对，而不是让所有 rank 按各自任意顺序串行等待；匹配收发才能持续前进。
 * 接着将每 peer 消息按 chunk 分发，Send 的偏移递增，Recv 的偏移独立递增。
 * 0 字节是有效同步任务，-1 表示这一方向没有任务，二者不能合并成同一种空操作。
 */
        // schedule delta 0, +1, -1, +2, -2, ...
        // also make sure we don't do 0 twice, nor +n/2 and -n/2 if n is even.
        for (/* 中文逐项-变量 V1929：d
 * d（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：d<=nNodes/4；循环体与增量表达式决定每轮如何推进。
 */int d=0; d<=nNodes/4; d++) {
/* 中文逐项-变量 V1930：deltas
 * deltas（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 deltas = { d, (nNodes-d)%nNodes, nNodes/2-d, (nNodes-(nNodes/2-d))%nNodes
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
          int deltas[4] = { d, (nNodes-d)%nNodes, nNodes/2-d, (nNodes-(nNodes/2-d))%nNodes };
/* 中文逐项-变量 V1931：index
 * index（int；函数 ncclGroupEnd 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 初值/来源 index = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
          int index = 0;
/* 中文逐项-变量 V1932：delta
 * delta（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 delta = deltas[index]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          int delta = deltas[index];
sched_delta:
/* 中文逐项-变量 V1933：recvNode
 * recvNode（uint32_t；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvNode = (node+nNodes-delta)%nNodes；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          uint32_t recvNode = (node+nNodes-delta)%nNodes;
/* 中文逐项-变量 V1934：sendNode
 * sendNode（uint32_t；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendNode = (node+delta)%nNodes；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
          uint32_t sendNode = (node+delta)%nNodes;
/* 中文逐项-变量 V1935：steps
 * steps（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 steps = comm->maxLocalRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          int steps = comm->maxLocalRanks;
          for (/* 中文逐项-变量 V1936：s
 * s（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<steps；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<steps; s++) {
/* 中文逐项-变量 V1937：recvIndex
 * recvIndex（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvIndex = (localRank-s+steps)%steps；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
            int recvIndex = (localRank-s+steps)%steps;
/* 中文逐项-变量 V1938：recvPeer
 * recvPeer（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvPeer = recvIndex<comm->nodeRanks[recvNode].localRanks ?
 * comm->nodeRanks[recvNode].localRankToRank[recvIndex] : -1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            int recvPeer = recvIndex<comm->nodeRanks[recvNode].localRanks ? comm->nodeRanks[recvNode].localRankToRank[recvIndex] : -1;
/* 中文逐项-变量 V1939：sendIndex
 * sendIndex（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendIndex = (localRank+s)%steps；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
            int sendIndex = (localRank+s)%steps;
/* 中文逐项-变量 V1940：sendPeer
 * sendPeer（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendPeer = sendIndex<comm->nodeRanks[sendNode].localRanks ?
 * comm->nodeRanks[sendNode].localRankToRank[sendIndex] : -1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            int sendPeer = sendIndex<comm->nodeRanks[sendNode].localRanks ? comm->nodeRanks[sendNode].localRankToRank[sendIndex] : -1;
/* 中文逐项-变量 V1941：recv
 * recv（struct ncclP2Pinfo；函数 ncclGroupEnd 内）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 初值/来源 recv = recvPeer != -1 && comm->p2pRecvs[recvPeer] ? comm->p2pRecvs[recvPeer]->getNext() :
 * NULL；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            struct ncclP2Pinfo* recv = recvPeer != -1 && comm->p2pRecvs[recvPeer] ? comm->p2pRecvs[recvPeer]->getNext() : NULL;
/* 中文逐项-变量 V1942：send
 * send（struct ncclP2Pinfo；函数 ncclGroupEnd 内）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 初值/来源 send = sendPeer != -1 && comm->p2pSends[sendPeer] ? comm->p2pSends[sendPeer]->getNext() :
 * NULL；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            struct ncclP2Pinfo* send = sendPeer != -1 && comm->p2pSends[sendPeer] ? comm->p2pSends[sendPeer]->getNext() : NULL;
            if (recv != NULL || send != NULL) {
/* 中文逐项-变量 V1943：totRecvBytes、totSendBytes
 * totRecvBytes（ssize_t；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 totRecvBytes = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 * totSendBytes（ssize_t；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 totSendBytes = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
              ssize_t totRecvBytes = -1, totSendBytes = -1;
              if (recv != NULL) totRecvBytes = recv->nbytes;
              if (send != NULL) totSendBytes = send->nbytes;
              if (recv) comm->p2pRecvCount--;
              if (send) comm->p2pSendCount--;
              if (recvPeer == comm->rank) { // Check self send/recv
                if (sendPeer != comm->rank) { WARN("Sendrecv schedule not aligned for self"); ret = ncclInternalError; goto group_cleanup; }
                if (send && recv == NULL) { WARN("Trying to send to self without a matching recv"); ret = ncclInvalidUsage; goto group_cleanup; }
                if (send == NULL && recv) { WARN("Trying to recv to self without a matching send"); ret = ncclInvalidUsage; goto group_cleanup; }
              }
/* 中文逐项-变量 V1944：recvBuff
 * recvBuff（void；函数 ncclGroupEnd 内）：各邻居的协议缓冲指针，按当前 step 计算实际槽位置。
 * 初值/来源 recvBuff = recv ? recv->buff : NULL；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              void* recvBuff = recv ? recv->buff : NULL;
/* 中文逐项-变量 V1945：sendBuff
 * sendBuff（void；函数 ncclGroupEnd 内）：各邻居的协议缓冲指针，按当前 step 计算实际槽位置。
 * 初值/来源 sendBuff = send ? send->buff : NULL；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
              void* sendBuff = send ? send->buff : NULL;
              // After we recycle p2pSend/Recv, we're no longer allowed to dereference send or recv, only use them as boolean NULL/not NULL.
              if (recv && comm->p2pRecvs[recvPeer]->peakNext() == NULL) comm->p2pRecvs[recvPeer]->recycle();
              if (send && comm->p2pSends[sendPeer]->peakNext() == NULL) comm->p2pSends[sendPeer]->recycle();

/* 中文逐项-变量 V1946：recvChunkSize
 * recvChunkSize（ssize_t；函数 ncclGroupEnd 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 recvChunkSize = getP2pChunkSize(totRecvBytes, nChannelsMin, nChannelsMax, stepSize,
 * SENDRECV_SLICEFACTOR*stepSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
              ssize_t recvChunkSize = getP2pChunkSize(totRecvBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);
/* 中文逐项-变量 V1947：sendChunkSize
 * sendChunkSize（ssize_t；函数 ncclGroupEnd 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 sendChunkSize = getP2pChunkSize(totSendBytes, nChannelsMin, nChannelsMax, stepSize,
 * SENDRECV_SLICEFACTOR*stepSize)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
              ssize_t sendChunkSize = getP2pChunkSize(totSendBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);

/* 中文逐项-变量 V1948：sendOffset
 * sendOffset（ssize_t；函数 ncclGroupEnd 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 sendOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
              ssize_t sendOffset = 0;
/* 中文逐项-变量 V1949：recvOffset
 * recvOffset（ssize_t；函数 ncclGroupEnd 内）：当前分段或表项的相对位置，精确单位由下面赋值式和指针元素类型确定。
 * 初值/来源 recvOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
              ssize_t recvOffset = 0;
/* 中文逐项-变量 V1950：sendRemaining、recvRemaining
 * sendRemaining（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendRemaining = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * recvRemaining（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvRemaining = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
              int sendRemaining = 1, recvRemaining = 1;
/* 中文逐项-变量 V1951：chunk
 * chunk（int；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 chunk = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
              int chunk = 0;
              do {
                // Shuffle channels with s intra-node, and delta inter-node. Inter-node, make sure
                // to use multiple channels to guarantee progress on all ranks from the same node.
/* 中文逐项-变量 V1952：recvbytes
 * recvbytes（ssize_t；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 recvbytes = totRecvBytes-recvOffset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
                ssize_t recvbytes = totRecvBytes-recvOffset;
/* 中文逐项-变量 V1953：sendbytes
 * sendbytes（ssize_t；函数 ncclGroupEnd 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendbytes = totSendBytes-sendOffset；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
                ssize_t sendbytes = totSendBytes-sendOffset;
                if (recvbytes > recvChunkSize) { recvbytes = recvChunkSize; } else { recvRemaining = 0; }
                if (sendbytes > sendChunkSize) { sendbytes = sendChunkSize; } else { sendRemaining = 0; }
                // 0-bytes send/recv are considered as syncs. Make sure we only add syncs when requested
                // (total size == 0), otherwise set size to -1.
                if (sendbytes < 0 || (sendbytes == 0 && totSendBytes != 0)) send = NULL;
                if (recvbytes < 0 || (recvbytes == 0 && totRecvBytes != 0)) recv = NULL;
                if (recv) {
                  NCCLCHECKGOTO(scheduleRecv(comm, recvPeer, chunk, recvbytes, ((char*)recvBuff)+recvOffset), ret, group_cleanup);
                }
                if (send) {
                  NCCLCHECKGOTO(scheduleSend(comm, sendPeer, chunk, sendbytes, ((char*)sendBuff)+sendOffset), ret, group_cleanup);
                }
                recvOffset += recvChunkSize;
                sendOffset += sendChunkSize;
                chunk++;
              } while (sendRemaining || recvRemaining);
            }
          }
          index++;
          if (index == 1 && deltas[1] == deltas[0]) index++;
          if (index == 2 && deltas[2] == deltas[0]) index++;
          if (index == 3 && deltas[3] == deltas[2]) index++;
          if (index == 3 && deltas[3] == deltas[1]) index++;
          if (index < 4) {
            delta = deltas[index];
            goto sched_delta;
          }
        }
      }
    }
  }

  /* Collectives are done in three steps :
   * 0. Save kernels previously enqueued. Compute channel, algo, proto, etc.
   * 1. Barrier Check In. Only the last call may call cudaLaunchKernel[cooperative]
   * 2. Barrier Wait. No CUDA call is permitted
   * 3. Enqueue Events. CUDA event wait/enqueue.
   * This is needed because step 2 cannot call any CUDA primitive, otherwise if
   * cudaFree happens between 1 and 3, it could block that CUDA call and
   * prevent some ranks from launching their network threads, which would
   * prevent the NCCL call from completing, blocking the cudaFree call.
   */

/* 中文精读：Group 最后阶段的调用顺序
 * 先统一检查各 communicator 是否都处于图捕获模式，再 SetupAsyncKernels 生成各自计划。
 * 普通模式对每个 comm 执行 HostSetup（其中发布 Proxy 任务）和 LaunchBarrier，
 * 然后再统一调用 LaunchKernel，最后 RecordEvents/LaunchReset。
 * 把准备、屏障、发射、事件拆为多轮，是为了同进程多 GPU 能互相等到匹配的工作。
 */
  // Check whether we are in cuda graph mode
  NCCLCHECK(ncclCalloc(&graphs, ncclGroupIndex));
  for (/* 中文逐项-变量 V1954：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1955：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
/* 中文逐项-变量 V1956：comm
 * comm（ncclComm_t；函数 ncclGroupEnd 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = args->coll.comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      ncclComm_t comm = args->coll.comm;
      NCCLCHECKGOTO(ncclGetCudaGraph(comm, graphs+i), ret, group_cleanup);
      if (usingCudaGraphAll == -1) {
        usingCudaGraphAll = comm->usingCudaGraph;
      } else if (usingCudaGraphAll != comm->usingCudaGraph) {
        WARN("Illegal to have some communicators in graph mode while others not");
        ret = ncclInvalidUsage;
        goto group_cleanup;
      }
    }
  }
  for (/* 中文逐项-变量 V1957：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1958：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
/* 中文逐项-变量 V1959：comm
 * comm（ncclComm_t；函数 ncclGroupEnd 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = args->coll.comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      ncclComm_t comm = args->coll.comm;
      NCCLCHECKGOTO(ncclSetupAsyncKernels(comm), ret, group_cleanup);
    }
  }
  for (/* 中文逐项-变量 V1960：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1961：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      if (args->coll.comm->userStream == cudaStreamDefault ||
          args->coll.comm->userStream == cudaStreamPerThread ||
          args->coll.comm->userStream == cudaStreamLegacy)
        CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      if (usingCudaGraphAll == 1) {
        NCCLCHECKGOTO(ncclCudaGraphHostSetup(args->coll.comm, graphs[i]), ret, end);
      } else {
        ncclEnqueueHostSetup<0>(args->coll.comm->enqueueInfo);
      }
      NCCLCHECKGOTO(ncclLaunchBarrier(args->coll.comm), ret, end);
    }
  }
  for (/* 中文逐项-变量 V1962：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1963：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      NCCLCHECKGOTO(ncclLaunchKernel(args->coll.comm), ret, end);
    }
  }
  for (/* 中文逐项-变量 V1964：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1965：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclAsyncArgs* args = ncclGroupArgs+i;
    if (args->funcType == ASYNC_FUNC_COLL) {
      if (args->coll.comm->userStream == cudaStreamDefault ||
          args->coll.comm->userStream == cudaStreamPerThread ||
          args->coll.comm->userStream == cudaStreamLegacy)
        CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);
      NCCLCHECKGOTO(ncclRecordEvents(args->coll.comm), ret, end);
      NCCLCHECKGOTO(ncclLaunchReset(args->coll.comm), ret, end);
    }
  }

  goto end;
group_cleanup:
  if (ret != ncclSuccess) {
    // At least one call in the group failed. Since we want to make that group
    // an atomic operation, we need to cancel all operations.
    for (/* 中文逐项-变量 V1966：i
 * i（int；函数 ncclGroupEnd 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclGroupIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclGroupIndex; i++) {
/* 中文逐项-变量 V1967：args
 * args（struct ncclAsyncArgs；函数 ncclGroupEnd 内）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 初值/来源 args = ncclGroupArgs+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      struct ncclAsyncArgs* args = ncclGroupArgs+i;
      if (args->funcType == ASYNC_FUNC_INIT) {
        if (args->init.newcomm) ncclCommDestroy(*args->init.newcomm);
        *args->init.newcomm = NULL;
      } else {
/* 中文逐项-变量 V1968：comm
 * comm（struct ncclComm；函数 ncclGroupEnd 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = args->coll.comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclComm* comm = args->coll.comm;
        // Reset aggregation counters
        comm->asyncOpCount = 0;
        comm->asyncTotalSize = 0;
        // Dequeue p2p lists
        if (comm->p2pSendCount > 0 || comm->p2pRecvCount > 0) {
          for (/* 中文逐项-变量 V1969：peer
 * peer（int；函数 ncclGroupEnd 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：peer<comm->nRanks；循环体与增量表达式决定每轮如何推进。
 */int peer=0; peer<comm->nRanks; peer++) {
            if (comm->p2pSends[peer]) comm->p2pSends[peer]->recycle();
            if (comm->p2pRecvs[peer]) comm->p2pRecvs[peer]->recycle();
          }
          comm->p2pSendCount = comm->p2pRecvCount = 0;
        }
        ncclLaunchReset(comm);
      }
    }
  }
end:
  ncclGroupError = ncclSuccess;
  ncclGroupIndex = 0;
  CUDACHECK(cudaSetDevice(savedDev)); // do other clean-ups first before calling cudaSetDevice, because this call can fail too
  if (graphs) free(graphs);
  return ret;
}
