/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "nccl.h"
#include "channel.h"
#include "nvmlwrap.h"
#include "gdrwrap.h"
#include "bootstrap.h"
#include "transport.h"
#include "group.h"
#include "net.h"
#include "coll_net.h"
#include "enqueue.h"
#include "graph.h"
#include "argcheck.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* 中文逐项-宏 M0131：STR2
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 STR2；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define STR2(v) #v
/* 中文逐项-宏 M0132：STR
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 STR；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define STR(v) STR2(v)

#ifdef ENABLE_TRACE
/* 中文逐项-变量 V3211：ncclEpoch
 * ncclEpoch（std::chrono::high_resolution_clock::time_point；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
std::chrono::high_resolution_clock::time_point ncclEpoch;
#endif

#if CUDART_VERSION >= 9020
/* 中文逐项-常量 X1152：NCCL_GROUP_CUDA_STREAM
 * 编译期定义：NCCL_GROUP_CUDA_STREAM = 0 // CGMD: CUDA 9.2,10.X Don't need to use an internal CUDA stream。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_GROUP_CUDA_STREAM 0 // CGMD: CUDA 9.2,10.X Don't need to use an internal CUDA stream
#else
/* 中文逐项-常量 X1153：NCCL_GROUP_CUDA_STREAM
 * 编译期定义：NCCL_GROUP_CUDA_STREAM = 1 // CGMD: CUDA 9.0,9.1 Need to use an internal CUDA stream。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_GROUP_CUDA_STREAM 1 // CGMD: CUDA 9.0,9.1 Need to use an internal CUDA stream
#endif

/* 中文逐项-变量 V3212：ncclFuncStr
 * ncclFuncStr（char；文件/静态存储作用域）：内部操作/算法/协议枚举对应的可读名字表，供日志和配置匹配使用。
 * 初值/来源 ncclFuncStr = { "Broadcast", "Reduce", "AllGather", "ReduceScatter", "AllReduce"
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
const char* ncclFuncStr[NCCL_NUM_FUNCTIONS] = { "Broadcast", "Reduce", "AllGather", "ReduceScatter", "AllReduce" };
/* 中文逐项-变量 V3213：ncclAlgoStr
 * ncclAlgoStr（char；文件/静态存储作用域）：内部操作/算法/协议枚举对应的可读名字表，供日志和配置匹配使用。
 * 初值/来源 ncclAlgoStr = { "Tree", "Ring", "CollNet" }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
const char* ncclAlgoStr[NCCL_NUM_ALGORITHMS] = { "Tree", "Ring", "CollNet" };
/* 中文逐项-变量 V3214：ncclProtoStr
 * ncclProtoStr（char；文件/静态存储作用域）：内部操作/算法/协议枚举对应的可读名字表，供日志和配置匹配使用。
 * 初值/来源 ncclProtoStr = { "LL", "LL128", "Simple" }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
const char* ncclProtoStr[NCCL_NUM_PROTOCOLS] = { "LL", "LL128", "Simple" };

/* 中文逐项-参数getter P0020：GroupCudaStream
 * 生成函数 ncclParamGroupCudaStream()，读取 NCCL_GROUP_CUDA_STREAM，编译默认值为 NCCL_GROUP_CUDA_STREAM。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(GroupCudaStream, "GROUP_CUDA_STREAM", NCCL_GROUP_CUDA_STREAM);

/* 中文逐项-参数getter P0021：CheckPointers
 * 生成函数 ncclParamCheckPointers()，读取 NCCL_CHECK_POINTERS，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(CheckPointers, "CHECK_POINTERS", 0);

// GDRCOPY support: Off by default
/* 中文逐项-参数getter P0022：GdrCopyEnable
 * 生成函数 ncclParamGdrCopyEnable()，读取 NCCL_GDRCOPY_ENABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(GdrCopyEnable, "GDRCOPY_ENABLE", 0);

// GDRCOPY support
/* 中文逐项-变量 V3215：ncclGdrCopy
 * ncclGdrCopy（gdr_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclGdrCopy = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
gdr_t ncclGdrCopy = NULL;

/* 中文内部逻辑 F0881：initGdrCopy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(ncclParamGdrCopyEnable() == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：ncclGdrCopy = ncclGdrInit();；动态初始化 GDRCopy 并取得映射句柄，失败时由调用者决定是否使用普通回退路径。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0881
 */
/* 中文逐项-函数 F0881：initGdrCopy
 * 用途与位置：按编译支持及运行环境尝试初始化 GDRCopy，供小量显存映射访问优化使用。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclInit。
 * 本体直接出现的调用/包装：ncclGdrInit；ncclParamGdrCopyEnable。
 * 需要对照的边界/推进条件：ncclParamGdrCopyEnable() == 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t initGdrCopy() {
  if (ncclParamGdrCopyEnable() == 1) {
    ncclGdrCopy = ncclGdrInit();
  }
  return ncclSuccess;
}


/* 中文逐项-参数getter P0023：L1SharedMemoryCarveout
 * 生成函数 ncclParamL1SharedMemoryCarveout()，读取 NCCL_L1_SHARED_MEMORY_CARVEOUT，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(L1SharedMemoryCarveout, "L1_SHARED_MEMORY_CARVEOUT", 0);

/* 中文逐项-变量 V3216：initLock
 * initLock（pthread_mutex_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 initLock = PTHREAD_MUTEX_INITIALIZER；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
pthread_mutex_t initLock = PTHREAD_MUTEX_INITIALIZER;
/* 中文逐项-变量 V3217：initialized
 * initialized（bool；文件/静态存储作用域）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 initialized = false；布尔初态为false，后续条件决定是否切换。
 */
static bool initialized = false;
/* 中文逐项-变量 V3218：maxLocalSizeBytes
 * maxLocalSizeBytes（size_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxLocalSizeBytes = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
static size_t maxLocalSizeBytes = 0;
/* 中文内部逻辑 F0882：ncclInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(initialized)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 调用 / 计算 / 写入：pthread_mutex_lock(&initLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 条件判断：(!initialized)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：initEnv();；按用户配置、系统配置顺序加载配置文件，已有键不会被后读文件覆盖。
 *     8. 调用 / 计算 / 写入：initGdrCopy();；按编译支持及运行环境尝试初始化 GDRCopy，供小量显存映射访问优化使用。
 *     9. 调用 / 计算 / 写入：maxLocalSizeBytes = ncclKernMaxLocalSize();；遍历入口 kernel 属性以求最大局部内存使用，供初始化设备栈/资源配置参考。
 *     10. 准备局部状态：int carveout = ncclParamL1SharedMemoryCarveout();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     11. 条件判断：(carveout)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       12. 真分支：；仅条件为真执行以下子项。
 *         13. 调用 / 计算 / 写入：ncclKernSetSharedMemoryCarveout(carveout);；给受支持的 CUDA kernel 入口设置共享内存划分偏好，影响 SM 资源分配而不改算法语义。
 *     14. 调用 / 计算 / 写入：NCCLCHECK(ncclNetInit());；失败时可能提前return，成功才继续；总是初始化 bootstrap，再按外部插件、内置 IB、内置 Socket 顺序选可用数据后端，支持显式名字筛选。
 *     15. 调用 / 计算 / 写入：INFO(NCCL_INIT, "Using network %s", ncclNetName());；读取已选网络后端名字；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *     16. 调用 / 计算 / 写入：initialized = true;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：pthread_mutex_unlock(&initLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0882
 */
/* 中文逐项-函数 F0882：ncclInit
 * 用途与位置：进程级单次初始化配置、网络与相关支持库，避免每个 communicator 重复初始化全局组件。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclGetUniqueId；src/init.cc::ncclCommInitRankDev。
 * 本体直接出现的调用/包装：NCCLCHECK；initEnv；initGdrCopy；ncclKernMaxLocalSize；ncclKernSetSharedMemoryCarveout；ncclNetInit；ncclNetName；ncclParamL1SharedMemoryCarveout；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：initialized；!initialized；carveout。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
static ncclResult_t ncclInit() {
  if (initialized) return ncclSuccess;
  pthread_mutex_lock(&initLock);
  if (!initialized) {
    initEnv();
    initGdrCopy();
    maxLocalSizeBytes = ncclKernMaxLocalSize();
/* 中文逐项-变量 V3219：carveout
 * carveout（int；函数 ncclInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 carveout = ncclParamL1SharedMemoryCarveout()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
    int carveout = ncclParamL1SharedMemoryCarveout();
    if (carveout) ncclKernSetSharedMemoryCarveout(carveout);
    NCCLCHECK(ncclNetInit());
    INFO(NCCL_INIT, "Using network %s", ncclNetName());
    initialized = true;
  }
  pthread_mutex_unlock(&initLock);
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclGetVersion, int* version);
/* 中文内部逻辑 F0883：ncclGetVersion
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(version == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 调用 / 计算 / 写入：*version = NCCL_VERSION_CODE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0883
 */
/* 中文逐项-函数 F0883：ncclGetVersion
 * 用途与位置：把本库编译版本写到用户输出参数，帮助应用确认实际链接的 NCCL 版本。
 * 参数 version（int* version）：外部库事件/导出结构的布局大小与版本号，接收方据此判断可安全读取哪些字段。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：version == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGetVersion(int* version) {
  if (version == NULL) return ncclInvalidArgument;
  *version = NCCL_VERSION_CODE;
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclGetUniqueId, ncclUniqueId* out);
/* 中文内部逻辑 F0884：ncclGetUniqueId
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclInit());；失败时可能提前return，成功才继续；进程级单次初始化配置、网络与相关支持库，避免每个 communicator 重复初始化全局组件。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(out, "GetUniqueId", "out"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 3. 函数返回：return bootstrapGetUniqueId(out);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0884
 */
/* 中文逐项-函数 F0884：ncclGetUniqueId
 * 用途与位置：先完成库级初始化，再生成应用需要分发给全组 rank 的 bootstrap 标识。
 * 参数 out（ncclUniqueId* out）：CollNet 连接网络的特殊对端槽位，-1 表示当前 GPU 非对应网络出口。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitAll。
 * 本体直接出现的调用/包装：NCCLCHECK；PtrCheck；bootstrapGetUniqueId；ncclInit。
 * 源码返回表达式：return bootstrapGetUniqueId(out);。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGetUniqueId(ncclUniqueId* out) {
  NCCLCHECK(ncclInit());
  NCCLCHECK(PtrCheck(out, "GetUniqueId", "out"));
  return bootstrapGetUniqueId(out);
}

// Prevent compiler from optimizing out these operations
#ifdef __clang__
/* 中文逐项-宏 M0133：__attribute__
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 __attribute__；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCL_NO_OPTIMIZE __attribute__((optnone))
#else
#define NCCL_NO_OPTIMIZE __attribute__((optimize("O0")))
#endif

/* 中文内部逻辑 F0885：commPoison
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：comm->rank = comm->cudaDev = comm->busId = comm->nRanks = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0885
 */
/* 中文逐项-函数 F0885：commPoison
 * 用途与位置：在释放/销毁流程给 communicator 关键字段写入无效状态，辅助发现后续误用。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
void NCCL_NO_OPTIMIZE commPoison(ncclComm_t comm) {
  comm->rank = comm->cudaDev = comm->busId = comm->nRanks = -1;
}

#undef NCCL_NO_OPTIMIZE

static ncclResult_t commFree(ncclComm_t comm) {
  if (comm == NULL)
    return ncclSuccess;

  // First stop all threads before we free anything.
  NCCLCHECK(ncclProxyDestroy(comm));

  delete[] comm->userRedOps;

  free(comm->connectSend);
  free(comm->connectRecv);
  for (/* 中文逐项-变量 V3220：peer
 * peer（int；文件/静态存储作用域）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：peer<comm->nRanks；循环体与增量表达式决定每轮如何推进。
 */int peer=0; peer<comm->nRanks; peer++) {
    delete comm->p2pSends[peer];
    delete comm->p2pRecvs[peer];
  }
  free(comm->p2pSends);
  free(comm->p2pRecvs);
  free(comm->asyncOps);

  free(comm->peerInfo);
  ncclTopoFree(comm->topo);
  for (/* 中文逐项-变量 V3221：n
 * n（int；文件/静态存储作用域）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<comm->nNodes；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<comm->nNodes; n++) free(comm->nodeRanks[n].localRankToRank);
  free(comm->nodeRanks);
  free(comm->rankToNode);
  free(comm->rankToLocalRank);

  if (comm->bootstrap)
    NCCLCHECK(bootstrapClose(comm->bootstrap));

  CUDACHECK(cudaFree((ncclDevCommAndChannels*)comm->devComm));

  for (/* 中文逐项-变量 V3222：channel
 * channel（int；文件/静态存储作用域）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：channel<MAXCHANNELS；循环体与增量表达式决定每轮如何推进。
 */int channel=0; channel<MAXCHANNELS; channel++)
    NCCLCHECK(freeChannel(comm->channels+channel, comm->nRanks));

  if (comm->doneEvent != NULL)
    CUDACHECK(cudaEventDestroy(comm->doneEvent));

  if (comm->intDoneEvent != NULL)
    CUDACHECK(cudaEventDestroy(comm->intDoneEvent));

  if (comm->launchMode == ncclComm::GROUP) {
    CUDACHECK(cudaStreamDestroy(comm->groupStream));
  }

  // Last rank frees shared resources between threads
/* 中文逐项-变量 V3223：isLast
 * isLast（int；文件/静态存储作用域）：本通道当前批次的最后一条任务标记，GPU 执行完此记录即可退出本次 kernel。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCpuBarrierIn(comm, &isLast));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int isLast;
  NCCLCHECK(ncclCpuBarrierIn(comm, &isLast));
  if (isLast) {
    // Wait for all service threads to be done. We could not
    // do it earlier because it could have blocked and prevented
    // other ranks in the process to call ncclCommDestroy
    for (/* 中文逐项-变量 V3224：i
 * i（int；文件/静态存储作用域）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->intraRanks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->intraRanks; i++) {
/* 中文逐项-变量 V3225：ret
 * ret（void；文件/静态存储作用域）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：if (comm->intraThreads[i]) pthread_join(comm->intraThreads[i], &ret);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      void* ret;
      if (comm->intraThreads[i]) pthread_join(comm->intraThreads[i], &ret);
    }
    free(comm->intraBarrier);
    free(comm->intraParams);
    free(comm->intraThreads);
    free(comm->intraCudaDevs);
    free(comm->intraCGMode);
    free(comm->intraCC);
  }
  NCCLCHECK(ncclCudaHostFree((void *)comm->abortFlag));

  // Poison comm to try and catch a double free
  commPoison(comm);

  free(comm);
  return ncclSuccess;
}

/* 中文逐项-参数getter P0024：AggChannelSize
 * 生成函数 ncclParamAggChannelSize()，读取 NCCL_AGG_CHANNEL_SIZE，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(AggChannelSize, "AGG_CHANNEL_SIZE", -2);
/* 中文逐项-参数getter P0025：DisableGraphHelper
 * 生成函数 ncclParamDisableGraphHelper()，读取 NCCL_GRAPH_HELPER_DISABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(DisableGraphHelper, "GRAPH_HELPER_DISABLE", 0);
/* 中文逐项-参数getter P0026：GraphRegister
 * 生成函数 ncclParamGraphRegister()，读取 NCCL_GRAPH_REGISTER，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(GraphRegister, "GRAPH_REGISTER", 0);

/* 中文导读：
 * 创建一个 rank 的 communicator 基础状态，分配控制字段、队列及同步对象。
 * 此阶段与后面的拓扑搜索/transport 建链分开；不能在此把 channels 数组大小解释为硬件连接数。
 */
/* 中文精读：总对象的分配不等于所有连接已建立
 * 先分配 comm、peer 标记、事件、P2P 排队表与初始状态；通道 id 用 -1 表示尚未 init。
 * 通信组中的每个 rank 各有一份 comm；同进程多 GPU 还会共享本地发射协调所需的某些对象。
 * 后面要经历拓扑搜索、initChannel、transport setup/connect，才能拿到可用的设备连接指针。
 * 读资源生命周期时，应把 comm 分配、channel 分配和 backend 连接分配分成三个阶段。
 */
/* 中文内部逻辑 F0886：commAlloc
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(ndev < 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：WARN("invalid device count (%d) requested", ndev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 条件判断：(rank >= ndev || rank < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：WARN("rank %d exceeds ndev=%d", rank, ndev);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 9. 准备局部状态：cudaEvent_t doneEvent;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 调用 / 计算 / 写入：CUDACHECK(cudaEventCreateWithFlags(&doneEvent, cudaEventDisableTiming));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 11. 准备局部状态：cudaEvent_t intDoneEvent;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 调用 / 计算 / 写入：CUDACHECK(cudaEventCreateWithFlags(&intDoneEvent, cudaEventDisableTiming));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 13. 准备局部状态：struct ncclComm* comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 15. 调用 / 计算 / 写入：comm->rank = comm->hostDevComm.rank = rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 调用 / 计算 / 写入：comm->nRanks = comm->hostDevComm.nRanks = ndev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：cudaGetDevice(&comm->cudaDev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 18. 调用 / 计算 / 写入：NCCLCHECK(getBusId(comm->cudaDev, &comm->busId));；失败时可能提前return，成功才继续；查询本进程 CUDA 设备的 PCI 地址并转为整数标识，不能把设备 ordinal 当作 busId。
 * 19. 调用 / 计算 / 写入：TRACE(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx", comm, rank, ndev, comm->cudaDev, comm->busId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 20. 调用 / 计算 / 写入：comm->doneEvent = doneEvent;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 21. 调用 / 计算 / 写入：comm->intDoneEvent = intDoneEvent;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 22. 调用 / 计算 / 写入：comm->checkPointers = ncclParamCheckPointers() == 1 ? true : false;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 23. 编译期分支/指令：#if CUDART_VERSION >= 9020；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   24. 调用 / 计算 / 写入：comm->groupCudaStream = ncclParamGroupCudaStream();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   25. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     26. 调用 / 计算 / 写入：comm->groupCudaStream = NCCL_GROUP_CUDA_STREAM;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 27. 调用 / 计算 / 写入：comm->fatalError = ncclSuccess;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 28. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaHostCalloc((uint32_t**)&comm->abortFlag, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 29. 调用 / 计算 / 写入：comm->hostDevComm.abortFlag = comm->abortFlag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 调用 / 计算 / 写入：*comm->abortFlag = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 31. 调用 / 计算 / 写入：comm->argsptrs[0] = &comm->devComm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 调用 / 计算 / 写入：comm->argsptrs[1] = &comm->args;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 33. 调用 / 计算 / 写入：comm->collNetSupport = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 34. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->asyncOps, NCCL_MAX_OPS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 35. 调用 / 计算 / 写入：comm->asyncOpCount = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 36. 调用 / 计算 / 写入：comm->asyncTotalSize = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 37. 调用 / 计算 / 写入：comm->channelSize = ncclParamAggChannelSize();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 38. 调用 / 计算 / 写入：comm->asyncAllocMode = ncclComm::SHORTEST_QUEUE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 39. 准备局部状态：char* str = getenv("NCCL_AGG_ALLOC_MODE");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 40. 条件判断：(str)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   41. 真分支：；仅条件为真执行以下子项。
 *     42. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_AGG_ALLOC_MODE set by environment to %s", str);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 43. 条件判断：(str && strcmp(str, "ROUND_ROBIN") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   44. 真分支：；仅条件为真执行以下子项。
 *     45. 调用 / 计算 / 写入：comm->asyncAllocMode = ncclComm::ROUND_ROBIN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 46. 调用 / 计算 / 写入：CUDACHECK(cudaDriverGetVersion(&comm->driverVersion));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 47. 调用 / 计算 / 写入：NCCLCHECK(ncclCreateQueueInfo(&comm->enqueueInfo, comm));；失败时可能提前return，成功才继续；分配一个提交/图持有的 QueueInfo 和可复用元素列表，记录 communicator 队列生命周期计数。
 * 48. 调用 / 计算 / 写入：comm->lastSetupNode = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 49. 调用 / 计算 / 写入：comm->lastCudaGraphId = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 50. 调用 / 计算 / 写入：comm->disableGraphHelper = ncclParamDisableGraphHelper();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 51. 调用 / 计算 / 写入：comm->graphRegister = ncclParamGraphRegister();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 52. 编译期分支/指令：#if CUDART_VERSION >= 11030；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   53. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->graphHelperResources, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *   54. 调用 / 计算 / 写入：comm->graphHelperResources->comm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   55. 条件判断：(comm->driverVersion >= 11030)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     56. 真分支：；仅条件为真执行以下子项。
 *       57. 调用 / 计算 / 写入：CUDACHECK(cudaGetDriverEntryPoint("cuMemGetAddressRange", (void**)&comm->pfnCuMemGetAddressRange, cudaEnableDefault));；失败时可能提前return，成功才继续；提供旧版静态 CUDA runtime 缺少新符号时的 weak stub；调用即返回 cudaErrorStubLibrary，不实现真正 CUDA 功能。
 * 58. 源码语法片段：static_assert(MAXCHANNELS <= sizeof(*comm->connectSend)*8, "comm->connectSend must have enough bits for all channels");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 59. 源码语法片段：static_assert(MAXCHANNELS <= sizeof(*comm->connectRecv)*8, "comm->connectRecv must have enough bits for all channels");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 60. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->connectSend, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 61. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->connectRecv, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 62. 调用 / 计算 / 写入：comm->p2pSendCount = comm->p2pRecvCount = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 63. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->p2pSends, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 64. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->p2pRecvs, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 65. 循环：初始 int c=0;；继续条件 c<MAXCHANNELS；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   66. 调用 / 计算 / 写入：comm->channels[c].id = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 67. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 68. 调用 / 计算 / 写入：*comret = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 69. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0886
 */
/* 中文逐项-函数 F0886：commAlloc
 * 用途与位置：分配并初始化当前 rank 的 communicator 总对象、排队状态、事件与连接需求表。
 * 参数 comret（ncclComm_t* comret）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ndev（int ndev）：该communicator的总rank数，参与后续表/队列分配；不是本机CUDA设备枚举数。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitRankSync。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaDriverGetVersion；cudaEventCreateWithFlags；cudaGetDevice；cudaGetDriverEntryPoint；getBusId；getenv；ncclCalloc；ncclCreateQueueInfo；ncclCudaHostCalloc；ncclParamAggChannelSize；ncclParamCheckPointers；ncclParamDisableGraphHelper；ncclParamGraphRegister；ncclParamGroupCudaStream；strcmp。
 * 需要对照的边界/推进条件：ndev < 1；rank >= ndev || rank < 0；str；str && strcmp(str, "ROUND_ROBIN") == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t commAlloc(ncclComm_t* comret, int ndev, int rank) {
  if (ndev < 1) {
    WARN("invalid device count (%d) requested", ndev);
    return ncclInvalidArgument;
  }
  if (rank >= ndev || rank < 0) {
    WARN("rank %d exceeds ndev=%d", rank, ndev);
    return ncclInvalidArgument;
  }

  // Try to create a CUDA object right away. If there is something wrong with
  // the device we're on (failure cause #1) , better know it early.
/* 中文逐项-变量 V3226：doneEvent
 * doneEvent（cudaEvent_t；函数 commAlloc 内）：通信完成事件及内部 stream 依赖事件；record 只排队，不表示已完成。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaEventCreateWithFlags(&doneEvent,
 * cudaEventDisableTiming));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cudaEvent_t doneEvent;
  CUDACHECK(cudaEventCreateWithFlags(&doneEvent, cudaEventDisableTiming));
/* 中文逐项-变量 V3227：intDoneEvent
 * intDoneEvent（cudaEvent_t；函数 commAlloc 内）：通信完成事件及内部 stream 依赖事件；record 只排队，不表示已完成。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaEventCreateWithFlags(&intDoneEvent,
 * cudaEventDisableTiming));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cudaEvent_t intDoneEvent;
  CUDACHECK(cudaEventCreateWithFlags(&intDoneEvent, cudaEventDisableTiming));

/* 中文逐项-变量 V3228：comm
 * comm（struct ncclComm；函数 commAlloc 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&comm, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclComm* comm;
  NCCLCHECK(ncclCalloc(&comm, 1));

  comm->rank = comm->hostDevComm.rank = rank;
  comm->nRanks = comm->hostDevComm.nRanks = ndev;
  cudaGetDevice(&comm->cudaDev);
  NCCLCHECK(getBusId(comm->cudaDev, &comm->busId));
  TRACE(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx", comm, rank, ndev, comm->cudaDev, comm->busId);

  comm->doneEvent = doneEvent;
  comm->intDoneEvent = intDoneEvent;
  comm->checkPointers = ncclParamCheckPointers() == 1 ? true : false;
#if CUDART_VERSION >= 9020
  comm->groupCudaStream = ncclParamGroupCudaStream();
#else
  // Don't allow the user to overload the default setting in older CUDA builds
  comm->groupCudaStream = NCCL_GROUP_CUDA_STREAM;
#endif
  comm->fatalError = ncclSuccess;

  NCCLCHECK(ncclCudaHostCalloc((uint32_t**)&comm->abortFlag, 1));
  comm->hostDevComm.abortFlag = comm->abortFlag;
  *comm->abortFlag = 0;

  comm->argsptrs[0] = &comm->devComm;
  comm->argsptrs[1] = &comm->args;
  comm->collNetSupport = 0;

  NCCLCHECK(ncclCalloc(&comm->asyncOps, NCCL_MAX_OPS));
  comm->asyncOpCount = 0;
  comm->asyncTotalSize = 0;
  comm->channelSize = ncclParamAggChannelSize();
  comm->asyncAllocMode = ncclComm::SHORTEST_QUEUE;
/* 中文逐项-变量 V3229：str
 * str（char；函数 commAlloc 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv("NCCL_AGG_ALLOC_MODE")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* str = getenv("NCCL_AGG_ALLOC_MODE");
  if (str) INFO(NCCL_ENV, "NCCL_AGG_ALLOC_MODE set by environment to %s", str);
  if (str && strcmp(str, "ROUND_ROBIN") == 0) {
    comm->asyncAllocMode = ncclComm::ROUND_ROBIN;
  }

  CUDACHECK(cudaDriverGetVersion(&comm->driverVersion));

  NCCLCHECK(ncclCreateQueueInfo(&comm->enqueueInfo, comm));
  comm->lastSetupNode = NULL;
  comm->lastCudaGraphId = -1;
  comm->disableGraphHelper = ncclParamDisableGraphHelper();
  comm->graphRegister = ncclParamGraphRegister();
#if CUDART_VERSION >= 11030
  NCCLCHECK(ncclCalloc(&comm->graphHelperResources, 1));
  comm->graphHelperResources->comm = comm;
  if (comm->driverVersion >= 11030)
    // cudaGetDriverEntryPoint requires R465 or above (enhanced compat need)
    CUDACHECK(cudaGetDriverEntryPoint("cuMemGetAddressRange", (void**)&comm->pfnCuMemGetAddressRange, cudaEnableDefault));
#endif

  static_assert(MAXCHANNELS <= sizeof(*comm->connectSend)*8, "comm->connectSend must have enough bits for all channels");
  static_assert(MAXCHANNELS <= sizeof(*comm->connectRecv)*8, "comm->connectRecv must have enough bits for all channels");
  NCCLCHECK(ncclCalloc(&comm->connectSend, comm->nRanks));
  NCCLCHECK(ncclCalloc(&comm->connectRecv, comm->nRanks));

  comm->p2pSendCount = comm->p2pRecvCount = 0;
  NCCLCHECK(ncclCalloc(&comm->p2pSends, comm->nRanks));
  NCCLCHECK(ncclCalloc(&comm->p2pRecvs, comm->nRanks));

  // Mark channels as non initialized.
  for (/* 中文逐项-变量 V3230：c
 * c（int；函数 commAlloc 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<MAXCHANNELS；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<MAXCHANNELS; c++) comm->channels[c].id = -1;

  *comret = comm;
  return ncclSuccess;
}

/* 中文导读：
 * 在连接与 channel 配置完成后，构造 GPU 端 communicator/channel 视图。
 * GPU 后续从这些对象取 peer 连接和工作 FIFO；主机端与设备端对象有不同地址及职责。
 */
/* 中文精读：CPU 总对象怎样交给 GPU
 * ncclComm 中有 pthread/socket/拓扑模型，GPU 无需也不能把它们当作设备对象使用。
 * 这里构造 ncclDevCommAndChannels，复制 rank/nRanks/buffSizes/abortFlag 与 channel 设备视图。
 * peers 要指向 devPeers，workFifoDev 要指向 GPU 可读取的位置；设备 kernel 再拷到共享内存。
 * 因此调试 host 地址与 device 地址时必须辨认当前字段属于哪一侧，不能只看结构体名字相同。
 */
/* 中文精读：图示绑定 V3.3 F0887 devCommSetup
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0887
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0089] devCommSetup：从硬件路径和候选图到全局邻居、逻辑channel与GPU视图；ring.index相对用户rank0，userRanks先以本rank开头（直接对应）
 *   图：28-channel-generation
 *   定位：#symbol=F0887&binding=B0089
 */
/* 中文内部逻辑 F0887：devCommSetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：ncclDevCommAndChannels *devCommAndChans;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaCalloc(&devCommAndChans, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：comm->devComm = &devCommAndChans->comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：comm->hostDevComm.channels = devCommAndChans->channels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 准备局部状态：int nChannels = std::max(comm->nChannels, comm->p2pnChannels);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaMemcpy(comm->hostDevComm.channels, comm->channels, nChannels));；失败时可能提前return，成功才继续；按模板元素数量调用 cudaMemcpyDefault，实际字节数乘 sizeof(T)。
 * 7. 循环：初始 int r=0;；继续条件 r<comm->nChannels；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   8. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaMemcpy(comm->channels[r].ring.devUserRanks, comm->channels[r].ring.userRanks, comm->nRanks));；失败时可能提前return，成功才继续；按模板元素数量调用 cudaMemcpyDefault，实际字节数乘 sizeof(T)。
 * 9. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 10. 调用 / 计算 / 写入：NCCLCHECK(ncclCudaMemcpy(comm->devComm, &comm->hostDevComm, 1));；失败时可能提前return，成功才继续；按模板元素数量调用 cudaMemcpyDefault，实际字节数乘 sizeof(T)。
 * 11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0887
 */
/* 中文逐项-函数 F0887：devCommSetup
 * 用途与位置：构造设备端精简 communicator 和连续 channel 布局，把主机连接视图转换成 GPU 可访问地址。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitRankSync。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCudaCalloc；ncclCudaMemcpy；std::max。
 * 需要对照的边界/推进条件：r<comm->nChannels。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t devCommSetup(ncclComm_t comm) {
/* 中文逐项-变量 V3231：devCommAndChans
 * devCommAndChans（ncclDevCommAndChannels；函数 devCommSetup 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCudaCalloc(&devCommAndChans, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclDevCommAndChannels *devCommAndChans;
  NCCLCHECK(ncclCudaCalloc(&devCommAndChans, 1));
  comm->devComm = &devCommAndChans->comm;
  comm->hostDevComm.channels = devCommAndChans->channels;

  // Duplicate the channels on the device
/* 中文逐项-变量 V3232：nChannels
 * nChannels（int；函数 devCommSetup 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 初值/来源 nChannels = std::max(comm->nChannels, comm->p2pnChannels)；保留下限或较大候选，避免并行度/尺寸低于需要的最小值。
 */
  int nChannels = std::max(comm->nChannels, comm->p2pnChannels);
  NCCLCHECK(ncclCudaMemcpy(comm->hostDevComm.channels, comm->channels, nChannels));

  // Copy userRanks and peers
  for (/* 中文逐项-变量 V3233：r
 * r（int；函数 devCommSetup 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->nChannels; r++) {
    NCCLCHECK(ncclCudaMemcpy(comm->channels[r].ring.devUserRanks, comm->channels[r].ring.userRanks, comm->nRanks));
  }

  // Duplicate the dev comm on the device
  NCCLCHECK(ncclCudaMemcpy(comm->devComm, &comm->hostDevComm, 1));
  return ncclSuccess;
}

// Pre-process the string so that running "strings" on the lib can quickly reveal the version.
/* 中文逐项-常量 X1154：VERSION_STRING
 * 编译期定义：VERSION_STRING = "NCCL version " STR(NCCL_MAJOR) "." STR(NCCL_MINOR) "." STR(NCCL_PATCH) NCCL_SUFFIX "+cuda"
 * STR(CUDA_MAJOR) "." STR(CUDA_MINOR)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define VERSION_STRING "NCCL version " STR(NCCL_MAJOR) "." STR(NCCL_MINOR) "." STR(NCCL_PATCH) NCCL_SUFFIX "+cuda" STR(CUDA_MAJOR) "." STR(CUDA_MINOR)
/* 中文内部逻辑 F0888：showVersion
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：static int shown = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(shown == 0 && ncclDebugLevel >= NCCL_LOG_VERSION)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：printf("%s\n", VERSION_STRING);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 调用 / 计算 / 写入：fflush(stdout);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 条件判断：(ncclDebugFile != stdout)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       7. 真分支：；仅条件为真执行以下子项。
 *         8. 调用 / 计算 / 写入：INFO(NCCL_ALL,"%s", VERSION_STRING);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     9. 调用 / 计算 / 写入：shown = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0888
 */
/* 中文逐项-函数 F0888：showVersion
 * 用途与位置：按日志配置打印库版本和构建相关信息，辅助确认实际加载的库。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitRankDev。
 * 本体直接出现的调用/包装：fflush。
 * 需要对照的边界/推进条件：shown == 0 && ncclDebugLevel >= NCCL_LOG_VERSION；ncclDebugFile != stdout。各条件所属分支见紧邻函数体。
 */
static void showVersion() {
/* 中文逐项-变量 V3234：shown
 * shown（int；函数 showVersion 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shown = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  static int shown = 0;
  if (shown == 0 && ncclDebugLevel >= NCCL_LOG_VERSION) {
    printf("%s\n", VERSION_STRING);
    fflush(stdout);
    if (ncclDebugFile != stdout)
      INFO(NCCL_ALL,"%s", VERSION_STRING); // Also log NCCL version in one of the files
    shown = 1;
  }
}

/* 中文内部逻辑 F0889：fillInfo
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：info->rank = comm->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：CUDACHECK(cudaGetDevice(&info->cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：info->hostHash=getHostHash()+commHash;；由主机/启动环境相关身份生成一致摘要，帮助识别同一主机上的参与者。
 * 4. 调用 / 计算 / 写入：info->pidHash=getPidHash()+commHash;；结合进程命名空间/进程标识形成摘要，用于判断是否同进程共享本地指针与发射状态。
 * 5. 准备局部状态：struct stat statbuf;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：SYSCHECK(stat("/dev/shm", &statbuf), "stat");；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 7. 调用 / 计算 / 写入：info->shmDev = statbuf.st_dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：info->busId = comm->busId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(ncclGpuGdrSupport(&info->gdrSupport));；失败时可能提前return，成功才继续；优先查询 CUDA GDR 属性；旧运行时通过临时连接和 GPU 缓冲注册探测，探测资源用后回收。
 * 10. 调用 / 计算 / 写入：info->comm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：info->cudaCompCap = ncclCudaCompCap();；查询当前 CUDA 设备主次计算能力并编码成整数，供协议和架构策略判断。
 * 12. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0889
 */
/* 中文逐项-函数 F0889：fillInfo
 * 用途与位置：收集当前 rank 的主机、进程、GPU、共享内存和 GDR 能力摘要供 bootstrap 汇总。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 info（struct ncclPeerInfo* info）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 参数 commHash（uint64_t commHash）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；SYSCHECK；cudaGetDevice；getHostHash；getPidHash；ncclCudaCompCap；ncclGpuGdrSupport；stat。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t fillInfo(struct ncclComm* comm, struct ncclPeerInfo* info, uint64_t commHash) {
  info->rank = comm->rank;
  CUDACHECK(cudaGetDevice(&info->cudaDev));
  info->hostHash=getHostHash()+commHash;
  info->pidHash=getPidHash()+commHash;

  // Get the device MAJOR:MINOR of /dev/shm so we can use that
  // information to decide whether we can use SHM for inter-process
  // communication in a container environment
/* 中文逐项-变量 V3235：statbuf
 * statbuf（struct stat；函数 fillInfo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：SYSCHECK(stat("/dev/shm", &statbuf), "stat");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct stat statbuf;
  SYSCHECK(stat("/dev/shm", &statbuf), "stat");
  info->shmDev = statbuf.st_dev;

  info->busId = comm->busId;

  NCCLCHECK(ncclGpuGdrSupport(&info->gdrSupport));
  info->comm = comm;
  info->cudaCompCap = ncclCudaCompCap();
  return ncclSuccess;
}

/* 中文精读：内部环顺序怎样映射回用户 rank
 * 拓扑搜索可以把 GPU 排成任意有利于带宽的顺序，用户的 rank 编号不随之改变。
 * setupChannel 把环顺序旋转到本 rank 开头，写入主机 userRanks；devCommSetup随后复制到devUserRanks。ring.index=(ixRank-ixZero+nranks)%nranks，是相对用户rank0的环距离。
 * AllGather/ReduceScatter 用用户 rank 决定输出/输入段位置；AllReduce 用内部环 index 排算法块。
 * 物理相邻 GPU、用户 rank 相邻、环顺序相邻是三种关系，应分别跟踪。
 */
/* 中文精读：图示绑定 V3.3 F0890 setupChannel
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0890
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0088] setupChannel：从硬件路径和候选图到全局邻居、逻辑channel与GPU视图；ring.index相对用户rank0，userRanks先以本rank开头（直接对应）
 *   图：28-channel-generation
 *   定位：#symbol=F0890&binding=B0088
 */
/* 中文内部逻辑 F0890：setupChannel
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d", rank, nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(initChannel(comm, channelId));；失败时可能提前return，成功才继续；为单条逻辑 channel 分配环顺序表、peer 连接表和 GPU 可读取的工作描述符 FIFO。
 * 3. 准备局部状态：struct ncclRing* ring = &comm->channels[channelId].ring;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int ixZero=0, ixRank=0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 循环：初始 int i=0;；继续条件 i < nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 条件判断：(ringRanks[i] == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     7. 真分支：；仅条件为真执行以下子项。
 *       8. 调用 / 计算 / 写入：ixZero = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   9. 条件判断：(ringRanks[i] == rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     10. 真分支：；仅条件为真执行以下子项。
 *       11. 调用 / 计算 / 写入：ixRank = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 调用 / 计算 / 写入：ring->index = (ixRank-ixZero + nranks)%nranks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 循环：初始 int i=0;；继续条件 i<nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   15. 调用 / 计算 / 写入：ring->userRanks[i] = ringRanks[(i+ixRank)%nranks];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 17. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0890
 */
/* 中文逐项-函数 F0890：setupChannel
 * 用途与位置：把拓扑环顺序旋转到本 rank 开头，建立用户 rank 表及本 rank 内部环索引。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 channelId（int channelId）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 ringRanks（int* ringRanks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；initChannel。
 * 需要对照的边界/推进条件：i < nranks；ringRanks[i] == 0；ringRanks[i] == rank；i<nranks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t setupChannel(struct ncclComm* comm, int channelId, int rank, int nranks, int* ringRanks) {
  TRACE(NCCL_INIT, "rank %d nranks %d", rank, nranks);
  NCCLCHECK(initChannel(comm, channelId));

/* 中文逐项-变量 V3236：ring
 * ring（struct ncclRing；函数 setupChannel 内）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 * 初值/来源 ring = &comm->channels[channelId].ring；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclRing* ring = &comm->channels[channelId].ring;
  // Find our ring-distance from rank zero and reorganize ranks to start with rank.
/* 中文逐项-变量 V3237：ixZero、ixRank
 * ixZero（int；函数 setupChannel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ixZero = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * ixRank（int；函数 setupChannel 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ixRank = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int ixZero=0, ixRank=0;
  for (/* 中文逐项-变量 V3238：i
 * i（int；函数 setupChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < nranks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < nranks; i++) {
    if (ringRanks[i] == 0) ixZero = i;
    if (ringRanks[i] == rank) ixRank = i;
  }
  ring->index = (ixRank-ixZero + nranks)%nranks;
  for (/* 中文逐项-变量 V3239：i
 * i（int；函数 setupChannel 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nranks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nranks; i++) {
    ring->userRanks[i] = ringRanks[(i+ixRank)%nranks];
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0891：waitForNonNullPtr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：volatile void** ptr = (volatile void**) p;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 ；继续条件 (*ptr == NULL)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 调用 / 计算 / 写入：sched_yield();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 5. 函数返回：return (void*)*ptr;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0891
 */
/* 中文逐项-函数 F0891：waitForNonNullPtr
 * 用途与位置：等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。
 * 参数 p（void* p）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommSetIntraProc。
 * 本体直接出现的调用/包装：sched_yield。
 * 需要对照的边界/推进条件：*ptr == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return (void*)*ptr;。CHECK类宏还可能提前返回/跳转。
 */
void* waitForNonNullPtr(void* p) {
/* 中文逐项-变量 V3240：ptr
 * ptr（void；函数 waitForNonNullPtr 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = (volatile void**) p；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  volatile void** ptr = (volatile void**) p;
  while (*ptr == NULL) sched_yield();
  return (void*)*ptr;
}

/* 中文内部逻辑 F0892：initParams
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct cudaLaunchParams* params = comm->myParams = comm->intraParams+comm->intraRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：params->args = comm->argsptrs;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：params->stream = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：params->sharedMem = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：params->blockDim.x = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 调用 / 计算 / 写入：params->blockDim.y = params->blockDim.z = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：params->gridDim.x = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：params->gridDim.y = params->gridDim.z = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0892
 */
/* 中文逐项-函数 F0892：initParams
 * 用途与位置：初始化当前 communicator 的 CUDA 发射参数、参数指针与设备 kernel 所需上下文。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommSetIntraProc。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t initParams(struct ncclComm* comm) {
/* 中文逐项-变量 V3241：params
 * params（struct cudaLaunchParams；函数 initParams 内）：CUDA 发射参数或当前/同进程全组的发射参数表，包含 grid、block、stream 和参数地址。
 * 初值/来源 params = comm->myParams = comm->intraParams+comm->intraRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct cudaLaunchParams* params = comm->myParams = comm->intraParams+comm->intraRank;
  params->args = comm->argsptrs;
  params->stream = NULL;
  params->sharedMem = 0;
  params->blockDim.x = 0; params->blockDim.y = params->blockDim.z = 1;
  params->gridDim.x = 0; params->gridDim.y = params->gridDim.z = 1;
  return ncclSuccess;
}

// Allocate/Set Intra Process Structures and set CG options
/* 中文内部逻辑 F0893：ncclCommSetIntraProc
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：comm->intraRank = rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：comm->intraRanks = ranks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：comm->intraPhase = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 条件判断：(rank == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：assert(comm == comm0);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 准备局部状态：int* bar;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     8. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&bar, 2));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     9. 调用 / 计算 / 写入：bar[0] = bar[1] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 调用 / 计算 / 写入：comm->intraBarrier = bar;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->intraParams, comm->intraRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     12. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->intraThreads, comm->intraRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     13. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->intraCudaDevs, comm->intraRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     14. 准备局部状态：int* CGMode;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     15. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&CGMode, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     16. 调用 / 计算 / 写入：*CGMode = 0x11;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     17. 调用 / 计算 / 写入：comm->intraCGMode = CGMode;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     18. 准备局部状态：int* CC;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     19. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&CC, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     20. 调用 / 计算 / 写入：*CC = ncclCudaCompCap();；查询当前 CUDA 设备主次计算能力并编码成整数，供协议和架构策略判断。
 *     21. 调用 / 计算 / 写入：comm->intraCC = CC;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   22. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     23. 调用 / 计算 / 写入：comm->intraBarrier = (int*)waitForNonNullPtr(&comm0->intraBarrier);；等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。
 *     24. 调用 / 计算 / 写入：comm->intraParams = (struct cudaLaunchParams*)waitForNonNullPtr(&comm0->intraParams);；等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。
 *     25. 调用 / 计算 / 写入：comm->intraThreads = (pthread_t*)waitForNonNullPtr(&comm0->intraThreads);；等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。
 *     26. 调用 / 计算 / 写入：comm->intraCudaDevs = (int*)waitForNonNullPtr(&comm0->intraCudaDevs);；等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。
 *     27. 调用 / 计算 / 写入：comm->intraCGMode = (int*)waitForNonNullPtr(&comm0->intraCGMode);；等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。
 *     28. 调用 / 计算 / 写入：comm->intraCC = (int*)waitForNonNullPtr(&comm0->intraCC);；等待同进程初始化伙伴发布共享指针，作为本地对象会合的一部分。
 * 29. 调用 / 计算 / 写入：comm->intraCudaDevs[comm->intraRank] = comm->cudaDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 调用 / 计算 / 写入：comm->intraThreads[comm->intraRank] = comm->proxyState.thread;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 31. 调用 / 计算 / 写入：NCCLCHECK(initParams(comm));；失败时可能提前return，成功才继续；初始化当前 communicator 的 CUDA 发射参数、参数指针与设备 kernel 所需上下文。
 * 32. 准备局部状态：int cgMdLaunch = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 33. 调用 / 计算 / 写入：comm->launchMode = ncclComm::PARALLEL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 34. 准备局部状态：char* str = getenv("NCCL_LAUNCH_MODE");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 35. 条件判断：(str)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   36. 真分支：；仅条件为真执行以下子项。
 *     37. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_LAUNCH_MODE set by environment to %s", str);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 38. 条件判断：(str && strcmp(str, "GROUP") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   39. 真分支：；仅条件为真执行以下子项。
 *     40. 调用 / 计算 / 写入：comm->launchMode = ncclComm::GROUP;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 41. 条件判断：(comm->launchMode == ncclComm::GROUP)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   42. 真分支：；仅条件为真执行以下子项。
 *     43. 调用 / 计算 / 写入：CUDACHECK(cudaStreamCreateWithFlags(&comm->groupStream, cudaStreamNonBlocking));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     44. 编译期分支/指令：#if CUDART_VERSION >= 9000；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *       45. 条件判断：(*comm->intraCC && (ncclCudaCompCap() == *comm->intraCC))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         46. 真分支：；仅条件为真执行以下子项。
 *           47. 调用 / 计算 / 写入：(void) cudaDeviceGetAttribute(&cgMdLaunch, cudaDevAttrCooperativeMultiDeviceLaunch, comm->cudaDev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 48. 条件判断：(cgMdLaunch == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   49. 真分支：；仅条件为真执行以下子项。
 *     50. 调用 / 计算 / 写入：*comm->intraCGMode = 0x10;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 51. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0893
 */
/* 中文逐项-函数 F0893：ncclCommSetIntraProc
 * 用途与位置：把同进程 rank 加入本地发射协调组，设置屏障、设备列表及协作发射能力。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 ranks（int ranks）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 comm0（struct ncclComm* comm0）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；assert；cudaDeviceGetAttribute；cudaStreamCreateWithFlags；getenv；initParams；ncclCalloc；ncclCudaCompCap；strcmp；waitForNonNullPtr。
 * 需要对照的边界/推进条件：rank == 0；str；str && strcmp(str, "GROUP") == 0；comm->launchMode == ncclComm::GROUP。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommSetIntraProc(struct ncclComm* comm, int rank, int ranks, struct ncclComm* comm0) {
  comm->intraRank = rank;
  comm->intraRanks = ranks;
  comm->intraPhase = 0;

  // Alloc shared structures
  if (rank == 0) {
    assert(comm == comm0);
/* 中文逐项-变量 V3242：bar
 * bar（int；函数 ncclCommSetIntraProc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&bar, 2));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int* bar;
    NCCLCHECK(ncclCalloc(&bar, 2));
    bar[0] = bar[1] = 0;
    comm->intraBarrier = bar;
    NCCLCHECK(ncclCalloc(&comm->intraParams, comm->intraRanks));
    NCCLCHECK(ncclCalloc(&comm->intraThreads, comm->intraRanks));
    NCCLCHECK(ncclCalloc(&comm->intraCudaDevs, comm->intraRanks));
/* 中文逐项-变量 V3243：CGMode
 * CGMode（int；函数 ncclCommSetIntraProc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&CGMode, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int* CGMode;
    NCCLCHECK(ncclCalloc(&CGMode, 1));
    *CGMode = 0x11;
    comm->intraCGMode = CGMode;
/* 中文逐项-变量 V3244：CC
 * CC（int；函数 ncclCommSetIntraProc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&CC, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int* CC;
    NCCLCHECK(ncclCalloc(&CC, 1));
    *CC = ncclCudaCompCap();
    comm->intraCC = CC;
  } else {
    comm->intraBarrier = (int*)waitForNonNullPtr(&comm0->intraBarrier);
    comm->intraParams = (struct cudaLaunchParams*)waitForNonNullPtr(&comm0->intraParams);
    comm->intraThreads = (pthread_t*)waitForNonNullPtr(&comm0->intraThreads);
    comm->intraCudaDevs = (int*)waitForNonNullPtr(&comm0->intraCudaDevs);
    comm->intraCGMode = (int*)waitForNonNullPtr(&comm0->intraCGMode);
    comm->intraCC = (int*)waitForNonNullPtr(&comm0->intraCC);
  }
  comm->intraCudaDevs[comm->intraRank] = comm->cudaDev;
  comm->intraThreads[comm->intraRank] = comm->proxyState.thread;
  NCCLCHECK(initParams(comm));

/* 中文逐项-变量 V3245：cgMdLaunch
 * cgMdLaunch（int；函数 ncclCommSetIntraProc 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cgMdLaunch = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int cgMdLaunch = 0;

  // Set CG Mode
  comm->launchMode = ncclComm::PARALLEL;
/* 中文逐项-变量 V3246：str
 * str（char；函数 ncclCommSetIntraProc 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv("NCCL_LAUNCH_MODE")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* str = getenv("NCCL_LAUNCH_MODE");
  if (str) INFO(NCCL_ENV, "NCCL_LAUNCH_MODE set by environment to %s", str);
  if (str && strcmp(str, "GROUP") == 0) {
    comm->launchMode = ncclComm::GROUP;
  }
  if (comm->launchMode == ncclComm::GROUP) {
    CUDACHECK(cudaStreamCreateWithFlags(&comm->groupStream, cudaStreamNonBlocking));
#if CUDART_VERSION >= 9000
    if (*comm->intraCC && (ncclCudaCompCap() == *comm->intraCC)) {
      // Check whether the GPU supports Cooperative Group Multi Device Launch
      (void) cudaDeviceGetAttribute(&cgMdLaunch, cudaDevAttrCooperativeMultiDeviceLaunch, comm->cudaDev);
    }
#endif
  }

  // Disable cgMdLaunch if any rank does not support it
  if (cgMdLaunch == 0) {
    *comm->intraCGMode = 0x10;
  }
  return ncclSuccess;
}

/* 中文逐项-常量 X1155：DEFAULT_LL_BUFFSIZE
 * 编译期定义：DEFAULT_LL_BUFFSIZE = (NCCL_LL_LINES_PER_THREAD*NCCL_LL_MAX_NTHREADS*NCCL_STEPS*sizeof(union
 * ncclLLFifoLine))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define DEFAULT_LL_BUFFSIZE (NCCL_LL_LINES_PER_THREAD*NCCL_LL_MAX_NTHREADS*NCCL_STEPS*sizeof(union ncclLLFifoLine))
/* 中文逐项-常量 X1156：DEFAULT_LL128_BUFFSIZE
 * 编译期定义：DEFAULT_LL128_BUFFSIZE = (NCCL_LL128_ELEMS_PER_THREAD*NCCL_LL128_MAX_NTHREADS*NCCL_STEPS*sizeof(uint64_t))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define DEFAULT_LL128_BUFFSIZE (NCCL_LL128_ELEMS_PER_THREAD*NCCL_LL128_MAX_NTHREADS*NCCL_STEPS*sizeof(uint64_t))
/* 中文逐项-常量 X1157：DEFAULT_BUFFSIZE
 * 编译期定义：DEFAULT_BUFFSIZE = (1 << 22)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define DEFAULT_BUFFSIZE (1 << 22) /* 4MiB */
/* 中文逐项-常量 X1158：DEFAULT_BUFFSIZE_ARM
 * 编译期定义：DEFAULT_BUFFSIZE_ARM = (1 << 20)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define DEFAULT_BUFFSIZE_ARM (1 << 20) /* 1MiB */
/* 中文逐项-参数getter P0027：BuffSize
 * 生成函数 ncclParamBuffSize()，读取 NCCL_BUFFSIZE，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(BuffSize, "BUFFSIZE", -2);
/* 中文逐项-参数getter P0028：LlBuffSize
 * 生成函数 ncclParamLlBuffSize()，读取 NCCL_LL_BUFFSIZE，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(LlBuffSize, "LL_BUFFSIZE", -2);
/* 中文逐项-参数getter P0029：Ll128BuffSize
 * 生成函数 ncclParamLl128BuffSize()，读取 NCCL_LL128_BUFFSIZE，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(Ll128BuffSize, "LL128_BUFFSIZE", -2);

/* 中文内部逻辑 F0894：computeBuffSizes
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int cpuArch, cpuVendor, cpuModel;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor, &cpuModel));；失败时可能提前return，成功才继续；从硬件图读取 CPU 架构、厂商和型号，供容量建模和策略选择。
 * 3. 准备局部状态：int64_t envs[NCCL_NUM_PROTOCOLS] = { ncclParamLlBuffSize(), ncclParamLl128BuffSize(), ncclParamBuffSize() };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int defaults[NCCL_NUM_PROTOCOLS] = { DEFAULT_LL_BUFFSIZE, DEFAULT_LL128_BUFFSIZE, DEFAULT_BUFFSIZE };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(cpuArch == NCCL_TOPO_CPU_ARCH_ARM)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：defaults[NCCL_PROTO_SIMPLE] = DEFAULT_BUFFSIZE_ARM;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 循环：初始 int p=0;；继续条件 p<NCCL_NUM_PROTOCOLS；每轮更新 p++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   9. 调用 / 计算 / 写入：comm->buffSizes[p] = comm->hostDevComm.buffSizes[p] = envs[p] != -2 ? envs[p] : defaults[p];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0894
 */
/* 中文逐项-函数 F0894：computeBuffSizes
 * 用途与位置：读取/选择 LL、LL128、Simple 的协议缓冲尺寸，为后续 channel 传输与分片提供容量。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclParamBuffSize；ncclParamLl128BuffSize；ncclParamLlBuffSize；ncclTopoCpuType。
 * 需要对照的边界/推进条件：cpuArch == NCCL_TOPO_CPU_ARCH_ARM；p<NCCL_NUM_PROTOCOLS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t computeBuffSizes(struct ncclComm* comm) {
/* 中文逐项-变量 V3247：cpuArch、cpuVendor、cpuModel
 * cpuArch（int；函数 computeBuffSizes 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor,
 * &cpuModel));。若由输出参数填入，必须先检查填充调用成功再读值。
 * cpuVendor（int；函数 computeBuffSizes 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor,
 * &cpuModel));。若由输出参数填入，必须先检查填充调用成功再读值。
 * cpuModel（int；函数 computeBuffSizes 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor,
 * &cpuModel));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int cpuArch, cpuVendor, cpuModel;
  NCCLCHECK(ncclTopoCpuType(comm->topo, &cpuArch, &cpuVendor, &cpuModel));

/* 中文逐项-变量 V3248：envs
 * envs（int64_t；函数 computeBuffSizes 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 envs = { ncclParamLlBuffSize(), ncclParamLl128BuffSize(), ncclParamBuffSize()
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  int64_t envs[NCCL_NUM_PROTOCOLS] = { ncclParamLlBuffSize(), ncclParamLl128BuffSize(), ncclParamBuffSize() };
/* 中文逐项-变量 V3249：defaults
 * defaults（int；函数 computeBuffSizes 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 defaults = { DEFAULT_LL_BUFFSIZE, DEFAULT_LL128_BUFFSIZE, DEFAULT_BUFFSIZE
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  int defaults[NCCL_NUM_PROTOCOLS] = { DEFAULT_LL_BUFFSIZE, DEFAULT_LL128_BUFFSIZE, DEFAULT_BUFFSIZE };

  if (cpuArch == NCCL_TOPO_CPU_ARCH_ARM) defaults[NCCL_PROTO_SIMPLE] = DEFAULT_BUFFSIZE_ARM;

  for (/* 中文逐项-变量 V3250：p
 * p（int；函数 computeBuffSizes 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：p<NCCL_NUM_PROTOCOLS；循环体与增量表达式决定每轮如何推进。
 */int p=0; p<NCCL_NUM_PROTOCOLS; p++) {
    comm->buffSizes[p] = comm->hostDevComm.buffSizes[p] = envs[p] != -2 ? envs[p] : defaults[p];
  }
  return ncclSuccess;
}

/* 中文逐项-参数getter P0030：GraphDumpFileRank
 * 生成函数 ncclParamGraphDumpFileRank()，读取 NCCL_GRAPH_DUMP_FILE_RANK，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(GraphDumpFileRank, "GRAPH_DUMP_FILE_RANK", 0);
/* 中文逐项-参数getter P0031：CollNetNodeThreshold
 * 生成函数 ncclParamCollNetNodeThreshold()，读取 NCCL_COLLNET_NODE_THRESHOLD，编译默认值为 2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(CollNetNodeThreshold, "COLLNET_NODE_THRESHOLD", 2);
/* 中文逐项-参数getter P0032：NvbPreconnect
 * 生成函数 ncclParamNvbPreconnect()，读取 NCCL_NVB_PRECONNECT，编译默认值为 1。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(NvbPreconnect, "NVB_PRECONNECT", 1);

/* 中文导读：
 * 初始化主流程：bootstrap 元数据交换 → 硬件拓扑/路径 → 通信图搜索与全局拼接 → transport 建链。
 * Ring/Tree 使用已选出的邻居连接；随后建立性能模型并计算 P2P channel 布局。
 * 函数名中的 transport 不只代表 IB；同机 GPU 可以选择 P2P/SHM，跨机通常选择 NET。
 */
/* 中文精读：初始化主干，按下面的阶段阅读
 * 1. Bootstrap 建立控制网络，交换 peerInfo，识别 host/process/device 和同机 rank。
 * 2. 构造硬件拓扑，计算 P2P/GDR 路径，裁剪不可用节点，再计算实际通信域内路径。
 * 3. 创建 Proxy 服务，搜索 Ring/Tree/CollNet 候选图；各 rank 交换结果并收敛到共同配置。
 * 4. Preset/Postset 将机内顺序与机间端点拼成完整 ring/tree，分配 channel 并建立邻居连接。
 * 5. 调优模型、P2P channel 映射、共享资源和启动参数就绪后，devCommSetup 复制设备视图。
 * 这里的 bootstrapAllGather 是 CPU 控制信息交换，不是调用公开 ncclAllGather GPU API。
 */
/* 中文精读：图示绑定 V3.3 F0895 initTransportsRank
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0895
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0087] initTransportsRank：从硬件路径和候选图到全局邻居、逻辑channel与GPU视图；ring.index相对用户rank0，userRanks先以本rank开头（直接对应）
 *   图：28-channel-generation
 *   定位：#symbol=F0895&binding=B0087
 */
/* 中文内部逻辑 F0895：initTransportsRank
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int rank = comm->rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int nranks = comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：uint64_t commHash = getHash(commId->internal, NCCL_UNIQUE_ID_BYTES);；对给定字符串字节计算哈希，供主机/进程身份摘要使用，不承担安全认证。
 * 4. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "comm %p, commHash %lx, rank %d nranks %d - BEGIN", comm, commHash, rank, nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(bootstrapInit(commId, comm));；失败时可能提前return，成功才继续；让当前 rank 加入控制网络并交换 peer/proxy 地址，为拓扑发现和传输建链准备控制通道。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->peerInfo, nranks+1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(fillInfo(comm, comm->peerInfo+rank, commHash));；失败时可能提前return，成功才继续；收集当前 rank 的主机、进程、GPU、共享内存和 GDR 能力摘要供 bootstrap 汇总。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(bootstrapAllGather(comm->bootstrap, comm->peerInfo, sizeof(struct ncclPeerInfo)));；失败时可能提前return，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。
 * 9. 循环：初始 int i = 0;；继续条件 i < nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   10. 条件判断：((i != rank) && (comm->peerInfo[i].hostHash == comm->peerInfo[rank].hostHash) && (comm->peerInfo[i].busId == comm->peerInfo[rank].busId))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     11. 真分支：；仅条件为真执行以下子项。
 *       12. 调用 / 计算 / 写入：WARN("Duplicate GPU detected : rank %d and rank %d both on CUDA device %lx", rank, i, comm->peerInfo[rank].busId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       13. 函数返回：return ncclInvalidUsage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 15. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetSystem(comm, &comm->topo));；失败时可能提前return，成功才继续；读入可选拓扑文件并补齐当前 rank 所见 GPU/NIC 的自动发现信息，再生成硬件图。
 * 16. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoComputePaths(comm->topo, comm->peerInfo));；失败时可能提前return，成功才继续；计算 CPU/GPU/NET 路径，并按 P2P、GDR、PXN 策略替换不可直接使用的路径。
 * 17. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoTrimSystem(comm->topo, comm));；失败时可能提前return，成功才继续；删除不属于当前通信可达域的硬件节点，并在完全机内通信时裁剪不需要的网络对象。
 * 18. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoComputePaths(comm->topo, comm->peerInfo));；失败时可能提前return，成功才继续；计算 CPU/GPU/NET 路径，并按 P2P、GDR、PXN 策略替换不可直接使用的路径。
 * 19. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoSearchInit(comm->topo));；失败时可能提前return，成功才继续；初始化拓扑搜索的最大宽度和总宽度，建立速度候选与提前终止判断依据。
 * 20. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoPrint(comm->topo));；失败时可能提前return，成功才继续；递归打印硬件节点、类型和直接链路，验证发现到的硬件树。
 * 21. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetCpuAffinity(comm->topo, comm->rank, &comm->cpuAffinity));；失败时可能提前return，成功才继续；结合 GPU 的近端 CPU/NUMA 亲和范围与当前进程约束，生成线程亲和性掩码。
 * 22. 准备局部状态：cpu_set_t affinitySave;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 23. 条件判断：(CPU_COUNT(&comm->cpuAffinity))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   24. 真分支：；仅条件为真执行以下子项。
 *     25. 调用 / 计算 / 写入：sched_getaffinity(0, sizeof(cpu_set_t), &affinitySave);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     26. 调用 / 计算 / 写入：sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 27. 准备局部状态：ncclResult_t ret;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 28. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCreate(comm));；失败时可能提前return，成功才继续；按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。
 * 29. 准备局部状态：struct ncclTopoGraph ringGraph;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 30. 调用 / 计算 / 写入：ringGraph.id = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 31. 调用 / 计算 / 写入：ringGraph.pattern = NCCL_TOPO_PATTERN_RING;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 调用 / 计算 / 写入：ringGraph.collNet = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 33. 调用 / 计算 / 写入：ringGraph.minChannels = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 34. 调用 / 计算 / 写入：ringGraph.maxChannels = MAXCHANNELS/2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 35. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCompute(comm->topo, &ringGraph));；失败时可能提前return，成功才继续；在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。
 * 36. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoPrintGraph(comm->topo, &ringGraph));；失败时可能提前return，成功才继续；输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。
 * 37. 准备局部状态：struct ncclTopoGraph treeGraph;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 38. 调用 / 计算 / 写入：treeGraph.id = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 39. 调用 / 计算 / 写入：treeGraph.pattern = NCCL_TOPO_PATTERN_BALANCED_TREE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 40. 调用 / 计算 / 写入：treeGraph.collNet = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 41. 调用 / 计算 / 写入：treeGraph.minChannels = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 42. 调用 / 计算 / 写入：treeGraph.maxChannels = ringGraph.nChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 43. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCompute(comm->topo, &treeGraph));；失败时可能提前return，成功才继续；在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。
 * 44. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoPrintGraph(comm->topo, &treeGraph));；失败时可能提前return，成功才继续；输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。
 * 45. 准备局部状态：struct ncclTopoGraph collNetGraph;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 46. 调用 / 计算 / 写入：collNetGraph.id = 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 47. 调用 / 计算 / 写入：collNetGraph.pattern = NCCL_TOPO_PATTERN_TREE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 48. 调用 / 计算 / 写入：collNetGraph.collNet = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 49. 调用 / 计算 / 写入：collNetGraph.minChannels = collNetGraph.maxChannels = ringGraph.nChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 50. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoCompute(comm->topo, &collNetGraph));；失败时可能提前return，成功才继续；在搜索预算内调整速度、路径级别、同序通道和 crossNic，挑选通信图并必要时复制或退回简单顺序。
 * 51. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoPrintGraph(comm->topo, &collNetGraph));；失败时可能提前return，成功才继续；输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。
 * 52. 条件判断：(comm->rank == ncclParamGraphDumpFileRank())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   53. 真分支：；仅条件为真执行以下子项。
 *     54. 准备局部状态：struct ncclTopoGraph* graphs[3] = { &ringGraph, &treeGraph, &collNetGraph };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     55. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoDumpGraphs(comm->topo, 3, graphs));；失败时可能提前return，成功才继续；输出通信图的模式/带宽/端点及 GPU 顺序，或导出到配置指定的图文件。
 * 56. 条件判断：(collNetSupport())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   57. 真分支：；仅条件为真执行以下子项。
 *     58. 准备局部状态：char *collNetEnable = getenv("NCCL_COLLNET_ENABLE");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     59. 条件判断：(collNetEnable != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       60. 真分支：；仅条件为真执行以下子项。
 *         61. 调用 / 计算 / 写入：INFO(NCCL_ALL, "NCCL_COLLNET_ENABLE set by environment to %s.", collNetEnable);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         62. 条件判断：(strcmp(collNetEnable, "1") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           63. 真分支：；仅条件为真执行以下子项。
 *             64. 调用 / 计算 / 写入：comm->collNetSupport = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 65. 条件判断：(comm->collNetSupport == 1 && collNetGraph.nChannels <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   66. 真分支：；仅条件为真执行以下子项。
 *     67. 调用 / 计算 / 写入：comm->collNetSupport = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 68. 源码语法片段：struct ncclGraphInfo { int pattern; int nChannels; int sameChannels; float speedIntra; float speedInter; int typeIntra; int typeInter; }；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 69. 准备局部状态：struct { int netDev; int collNetSupport; struct ncclGraphInfo tree; struct ncclGraphInfo ring; struct ncclGraphInfo collNet; struct ncclTopoRanks topoRanks; } *allGather3Data;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 70. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&allGather3Data, nranks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 71. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetLocalNet(comm->topo, rank, &allGather3Data[rank].netDev));；失败时可能提前return，成功才继续；按 GPU 到 NIC 路径的距离/带宽等条件找出合适本地网络设备。
 * 72. 调用 / 计算 / 写入：allGather3Data[rank].tree.pattern = treeGraph.pattern;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 73. 调用 / 计算 / 写入：allGather3Data[rank].tree.nChannels = treeGraph.nChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 74. 调用 / 计算 / 写入：allGather3Data[rank].tree.sameChannels = treeGraph.sameChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 75. 调用 / 计算 / 写入：allGather3Data[rank].tree.speedIntra = treeGraph.speedIntra;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 76. 调用 / 计算 / 写入：allGather3Data[rank].tree.speedInter = treeGraph.speedInter;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 77. 调用 / 计算 / 写入：allGather3Data[rank].tree.typeIntra = treeGraph.typeIntra;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 78. 调用 / 计算 / 写入：allGather3Data[rank].tree.typeInter = treeGraph.typeInter;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 79. 调用 / 计算 / 写入：allGather3Data[rank].ring.pattern = ringGraph.pattern;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 80. 调用 / 计算 / 写入：allGather3Data[rank].ring.nChannels = ringGraph.nChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 81. 调用 / 计算 / 写入：allGather3Data[rank].ring.sameChannels = ringGraph.sameChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 82. 调用 / 计算 / 写入：allGather3Data[rank].ring.speedIntra = ringGraph.speedIntra;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 83. 调用 / 计算 / 写入：allGather3Data[rank].ring.speedInter = ringGraph.speedInter;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 84. 调用 / 计算 / 写入：allGather3Data[rank].ring.typeIntra = ringGraph.typeIntra;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 85. 调用 / 计算 / 写入：allGather3Data[rank].ring.typeInter = ringGraph.typeInter;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 86. 调用 / 计算 / 写入：allGather3Data[rank].collNet.pattern = collNetGraph.pattern;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 87. 调用 / 计算 / 写入：allGather3Data[rank].collNet.nChannels = collNetGraph.nChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 88. 调用 / 计算 / 写入：allGather3Data[rank].collNet.sameChannels = collNetGraph.sameChannels;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 89. 调用 / 计算 / 写入：allGather3Data[rank].collNet.speedIntra = collNetGraph.speedIntra;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 90. 调用 / 计算 / 写入：allGather3Data[rank].collNet.speedInter = collNetGraph.speedInter;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 91. 调用 / 计算 / 写入：allGather3Data[rank].collNet.typeIntra = collNetGraph.typeIntra;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 92. 调用 / 计算 / 写入：allGather3Data[rank].collNet.typeInter = collNetGraph.typeInter;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 93. 调用 / 计算 / 写入：allGather3Data[rank].collNetSupport = comm->collNetSupport;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 94. 调用 / 计算 / 写入：comm->nChannels = std::min(treeGraph.nChannels, ringGraph.nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 * 95. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoPreset(comm, &treeGraph, &ringGraph, &allGather3Data[rank].topoRanks));；失败时可能提前return，成功才继续；将本机搜索图转成可交换的局部环/树端点摘要，为跨节点拼接准备元数据。
 * 96. 调用 / 计算 / 写入：NCCLCHECK(bootstrapAllGather(comm->bootstrap, allGather3Data, sizeof(*allGather3Data)));；失败时可能提前return，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。
 * 97. 准备局部状态：int *nodesFirstRank, *nodesTreePatterns;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 98. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&nodesFirstRank, nranks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 99. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&nodesTreePatterns, nranks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 100. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->rankToNode, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 101. 循环：初始 int r=0;；继续条件 r<nranks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   102. 准备局部状态：int node;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   103. 准备局部状态：int firstRank = allGather3Data[r].topoRanks.ringRecv[0];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   104. 循环：初始 node=0；继续条件 node<comm->nNodes && nodesFirstRank[node] != firstRank；每轮更新 node++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     105. 调用 / 计算 / 写入：;；执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。
 *   106. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   107. 条件判断：(node == comm->nNodes)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     108. 真分支：；仅条件为真执行以下子项。
 *       109. 调用 / 计算 / 写入：comm->nNodes++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       110. 调用 / 计算 / 写入：nodesFirstRank[node] = firstRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       111. 调用 / 计算 / 写入：nodesTreePatterns[node] = allGather3Data[r].tree.pattern;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   112. 调用 / 计算 / 写入：comm->rankToNode[r] = node;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 113. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 114. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->nodeRanks, comm->nNodes));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 115. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->rankToLocalRank, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 116. 循环：初始 int r=0;；继续条件 r<comm->nRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   117. 准备局部状态：int node = comm->rankToNode[r];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   118. 调用 / 计算 / 写入：comm->rankToLocalRank[r] = comm->nodeRanks[node].localRanks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   119. 调用 / 计算 / 写入：comm->nodeRanks[node].localRanks++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 120. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 121. 循环：初始 int n=0;；继续条件 n<comm->nNodes；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   122. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->nodeRanks[n].localRankToRank, comm->nodeRanks[n].localRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *   123. 调用 / 计算 / 写入：comm->maxLocalRanks = std::max(comm->maxLocalRanks, comm->nodeRanks[n].localRanks);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   124. 调用 / 计算 / 写入：comm->nodeRanks[n].localRanks = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 125. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 126. 循环：初始 int r=0;；继续条件 r<comm->nRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   127. 准备局部状态：int node = comm->rankToNode[r];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   128. 调用 / 计算 / 写入：comm->nodeRanks[node].localRankToRank[comm->nodeRanks[node].localRanks++] = r;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 129. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 130. 调用 / 计算 / 写入：comm->node = comm->rankToNode[rank];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 131. 调用 / 计算 / 写入：comm->localRankToRank = comm->nodeRanks[comm->node].localRankToRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 132. 调用 / 计算 / 写入：comm->localRank = comm->rankToLocalRank[rank];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 133. 调用 / 计算 / 写入：comm->localRanks = comm->nodeRanks[comm->node].localRanks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 134. 调用 / 计算 / 写入：TRACE(NCCL_INIT,"hostHash[%d] %lx localRank %d localRanks %d localRank0 %d", rank, comm->peerInfo[rank].hostHash, comm->localRank, comm->localRanks, comm->localRankToRank[0]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 135. 条件判断：(comm->localRank == -1 || comm->localRankToRank[0] == -1 || comm->localRanks == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   136. 真分支：；仅条件为真执行以下子项。
 *     137. 调用 / 计算 / 写入：WARN("Failed to determine local ranks rank %d hostHash %lx pidHash %lx localRank %d localRanks %d localRank0 %d", rank, comm->peerInfo[rank].hostHash, comm->peerInfo[rank].pidHash, comm->localRank, co …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     138. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 139. 准备局部状态：int nChannelsOrig = comm->nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 140. 准备局部状态：struct ncclTopoRanks** allTopoRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 141. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&allTopoRanks, comm->nRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 142. 循环：初始 int i=0;；继续条件 i<nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   143. 调用 / 计算 / 写入：comm->peerInfo[i].netDev = allGather3Data[i].netDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   144. 调用 / 计算 / 写入：allTopoRanks[i] = &allGather3Data[i].topoRanks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   145. 调用 / 计算 / 写入：treeGraph.nChannels = std::min(allGather3Data[i].tree.nChannels, treeGraph.nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   146. 调用 / 计算 / 写入：treeGraph.sameChannels = std::min(allGather3Data[i].tree.sameChannels, treeGraph.sameChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   147. 调用 / 计算 / 写入：treeGraph.speedIntra = std::min(allGather3Data[i].tree.speedIntra, treeGraph.speedIntra);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   148. 调用 / 计算 / 写入：treeGraph.speedInter = std::min(allGather3Data[i].tree.speedInter, treeGraph.speedInter);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   149. 调用 / 计算 / 写入：treeGraph.typeIntra = std::max(allGather3Data[i].tree.typeIntra, treeGraph.typeIntra);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   150. 调用 / 计算 / 写入：treeGraph.typeInter = std::max(allGather3Data[i].tree.typeInter, treeGraph.typeInter);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   151. 调用 / 计算 / 写入：ringGraph.nChannels = std::min(allGather3Data[i].ring.nChannels, ringGraph.nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   152. 调用 / 计算 / 写入：ringGraph.sameChannels = std::min(allGather3Data[i].ring.sameChannels, ringGraph.sameChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   153. 调用 / 计算 / 写入：ringGraph.speedIntra = std::min(allGather3Data[i].ring.speedIntra, ringGraph.speedIntra);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   154. 调用 / 计算 / 写入：ringGraph.speedInter = std::min(allGather3Data[i].ring.speedInter, ringGraph.speedInter);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   155. 调用 / 计算 / 写入：ringGraph.typeIntra = std::max(allGather3Data[i].ring.typeIntra, ringGraph.typeIntra);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   156. 调用 / 计算 / 写入：ringGraph.typeInter = std::max(allGather3Data[i].ring.typeInter, ringGraph.typeInter);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   157. 调用 / 计算 / 写入：collNetGraph.nChannels = std::min(allGather3Data[i].collNet.nChannels, collNetGraph.nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   158. 调用 / 计算 / 写入：collNetGraph.sameChannels = std::min(allGather3Data[i].collNet.sameChannels, collNetGraph.sameChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   159. 调用 / 计算 / 写入：collNetGraph.speedIntra = std::min(allGather3Data[i].collNet.speedIntra, collNetGraph.speedIntra);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   160. 调用 / 计算 / 写入：collNetGraph.speedInter = std::min(allGather3Data[i].collNet.speedInter, collNetGraph.speedInter);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *   161. 调用 / 计算 / 写入：collNetGraph.typeIntra = std::max(allGather3Data[i].collNet.typeIntra, collNetGraph.typeIntra);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   162. 调用 / 计算 / 写入：collNetGraph.typeInter = std::max(allGather3Data[i].collNet.typeInter, collNetGraph.typeInter);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   163. 调用 / 计算 / 写入：comm->collNetSupport = std::min(allGather3Data[i].collNetSupport, comm->collNetSupport);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 * 164. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 165. 调用 / 计算 / 写入：comm->nChannels = treeGraph.nChannels = ringGraph.nChannels = std::min(treeGraph.nChannels, ringGraph.nChannels);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 * 166. 条件判断：(comm->nChannels < nChannelsOrig)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   167. 真分支：；仅条件为真执行以下子项。
 *     168. 循环：初始 int i=0;；继续条件 i<comm->nChannels；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       169. 调用 / 计算 / 写入：memcpy(comm->channels+comm->nChannels+i, comm->channels+nChannelsOrig+i, sizeof(struct ncclChannel));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     170. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 171. 条件判断：(comm->collNetSupport == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   172. 真分支：；仅条件为真执行以下子项。
 *     173. 准备局部状态：int collNetNodeThreshold = ncclParamCollNetNodeThreshold();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     174. 条件判断：(comm->nNodes < collNetNodeThreshold)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       175. 真分支：；仅条件为真执行以下子项。
 *         176. 调用 / 计算 / 写入：INFO(NCCL_INIT, "Communicator has %d nodes which is less than CollNet node threshold %d, disabling CollNet", comm->nNodes, collNetNodeThreshold);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         177. 调用 / 计算 / 写入：comm->collNetSupport = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     178. 循环：初始 int n=0;；继续条件 n<comm->nNodes；每轮更新 n++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       179. 条件判断：(comm->nodeRanks[n].localRanks > NCCL_MAX_DIRECT_ARITY+1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         180. 真分支：；仅条件为真执行以下子项。
 *           181. 调用 / 计算 / 写入：WARN("CollNet currently only supports up to %d GPUs per node, disabling CollNet", NCCL_MAX_DIRECT_ARITY+1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           182. 调用 / 计算 / 写入：comm->collNetSupport = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           183. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *     184. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 185. 准备局部状态：int *rings;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 186. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&rings, nranks*MAXCHANNELS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 187. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoPostset(comm, nodesFirstRank, nodesTreePatterns, allTopoRanks, rings, &collNetGraph));；失败时可能提前return，成功才继续；汇总全部节点的端点信息并拼接全局 Ring/Tree，复制/裁剪通道后验证环完整性。
 * 188. 调用 / 计算 / 写入：free(allTopoRanks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 189. 调用 / 计算 / 写入：free(nodesTreePatterns);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 190. 调用 / 计算 / 写入：free(nodesFirstRank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 191. 调用 / 计算 / 写入：free(allGather3Data);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 192. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d - BUILT %d TREES/RINGS", rank, nranks, comm->nChannels);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 193. 准备局部状态：char line[1024];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 194. 调用 / 计算 / 写入：line[0]='\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 195. 循环：初始 int c=0;；继续条件 c<comm->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   196. 准备局部状态：struct ncclTree* tree = &comm->channels[c].tree;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   197. 调用 / 计算 / 写入：snprintf(line+strlen(line), 1023-strlen(line), " [%d] %d/%d/%d->%d->%d", c, tree->down[0], tree->down[1], tree->down[2], rank, tree->up);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   198. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "Ring %02d : %d -> %d -> %d", c, comm->channels[c].ring.prev, comm->rank, comm->channels[c].ring.next);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 199. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 200. 调用 / 计算 / 写入：line[1023] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 201. 调用 / 计算 / 写入：INFO(NCCL_INIT, "Trees%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 202. 调用 / 计算 / 写入：NCCLCHECK(computeBuffSizes(comm));；失败时可能提前return，成功才继续；读取/选择 LL、LL128、Simple 的协议缓冲尺寸，为后续 channel 传输与分片提供容量。
 * 203. 循环：初始 int c=0;；继续条件 c<comm->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   204. 准备局部状态：struct ncclChannel* channel = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   205. 调用 / 计算 / 写入：NCCLCHECKGOTO(setupChannel(comm, c, rank, nranks, rings+c*nranks), ret, affinity_restore);；失败时保存错误并跳到清理标签，成功才继续；把拓扑环顺序旋转到本 rank 开头，建立用户 rank 表及本 rank 内部环索引。
 *   206. 条件判断：(comm->nRanks == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     207. 真分支：；仅条件为真执行以下子项。
 *       208. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   209. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, 1, &channel->ring.prev, 1, &channel->ring.next, 0), ret, affinity_restore);；失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。
 * 210. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 211. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &ringGraph, 0), ret, affinity_restore);；失败时保存错误并跳到清理标签，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。
 * 212. 调用 / 计算 / 写入：free(rings);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 213. 调用 / 计算 / 写入：INFO(NCCL_INIT, "Connected all rings");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 214. 循环：初始 int c=0;；继续条件 c<comm->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   215. 准备局部状态：struct ncclChannel* channel = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   216. 条件判断：(comm->nRanks == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     217. 真分支：；仅条件为真执行以下子项。
 *       218. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   219. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, NCCL_MAX_TREE_ARITY, channel->tree.down, 1, &channel->tree.up, 0), ret, affinity_restore);；失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。
 *   220. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, 1, &channel->tree.up, NCCL_MAX_TREE_ARITY, channel->tree.down, 0), ret, affinity_restore);；失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。
 * 221. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 222. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &treeGraph, 0), ret, affinity_restore);；失败时保存错误并跳到清理标签，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。
 * 223. 调用 / 计算 / 写入：INFO(NCCL_INIT, "Connected all trees");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 224. 条件判断：(comm->collNetSupport > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   225. 真分支：；仅条件为真执行以下子项。
 *     226. 准备局部状态：int collNetSetupFail = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     227. 准备局部状态：int highestTypes[NCCL_MAX_LOCAL_RANKS] = {TRANSPORT_P2P};；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     228. 准备局部状态：int nHeads = collNetGraph.nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     229. 准备局部状态：int *heads;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     230. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&heads, nHeads));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     231. 循环：初始 int c=0;；继续条件 c<nHeads；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       232. 调用 / 计算 / 写入：heads[c] = collNetGraph.intra[c*comm->localRanks+0];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     233. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     234. 循环：初始 int c=0;；继续条件 c<comm->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       235. 准备局部状态：struct ncclChannel* channel = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       236. 循环：初始 int h=0;；继续条件 h<nHeads；每轮更新 h++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         237. 准备局部状态：const int head = heads[h];；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *         238. 调用 / 计算 / 写入：collNetSetupFail = ncclTransportCollNetSetup(comm, &collNetGraph, channel, head, head, h, collNetRecv);；为网络归约 head 建立特殊 CollNet 连接并交换网络参与者句柄。
 *         239. 条件判断：(!collNetSetupFail)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           240. 真分支：；仅条件为真执行以下子项。
 *             241. 调用 / 计算 / 写入：collNetSetupFail = ncclTransportCollNetSetup(comm, &collNetGraph, channel, head, head, h, collNetSend);；为网络归约 head 建立特殊 CollNet 连接并交换网络参与者句柄。
 *       242. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       243. 条件判断：(c == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         244. 真分支：；仅条件为真执行以下子项。
 *           245. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportCollNetCheck(comm, collNetSetupFail), ret, collnet_cleanup);；失败时保存错误并跳到清理标签，成功才继续；全组汇总 CollNet 建链失败状态，一处不支持时一致回退，避免各 rank 选择不一致。
 *     246. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     247. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportCollNetCheck(comm, collNetSetupFail), ret, collnet_cleanup);；失败时保存错误并跳到清理标签，成功才继续；全组汇总 CollNet 建链失败状态，一处不支持时一致回退，避免各 rank 选择不一致。
 *     248. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d Connected inter-node CollNet", rank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     249. 准备局部状态：int highestTransportType0, highestTransportType1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     250. 循环：初始 int c=0;；继续条件 c<comm->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       251. 准备局部状态：struct ncclChannel* channelRecv = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       252. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channelRecv, NCCL_MAX_DIRECT_ARITY, channelRecv->collTree.up, NCCL_MAX_DIRECT_ARITY, channelRecv->collTree.down, 0), ret, collnet_cleanup);；失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。
 *     253. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     254. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 0, &highestTransportType0), ret, collnet_cleanup);；失败时保存错误并跳到清理标签，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。
 *     255. 循环：初始 int c=0;；继续条件 c<comm->nChannels；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       256. 准备局部状态：struct ncclChannel* channelSend = comm->channels+c;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       257. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channelSend, NCCL_MAX_DIRECT_ARITY, channelSend->collTree.down, NCCL_MAX_DIRECT_ARITY, channelSend->collTree.up, 1), ret, collnet_cleanup);；失败时保存错误并跳到清理标签，成功才继续；为尚未连通的邻居方向/channel 设置建链需求位，真正资源创建留到 Setup。
 *     258. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     259. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 1, &highestTransportType1), ret, collnet_cleanup);；失败时保存错误并跳到清理标签，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。
 *     260. 调用 / 计算 / 写入：comm->intraHighestTransportType = highestTypes[comm->localRank] = highestTransportType0 > highestTransportType1 ? highestTransportType0 : highestTransportType1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     261. 调用 / 计算 / 写入：NCCLCHECK(bootstrapIntraNodeAllGather(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, highestTypes, sizeof(int)));；失败时可能提前return，成功才继续；只在所选节点内 rank 集合交换固定大小控制信息，按局部索引映射到全局 rank。
 *     262. 循环：初始 int i=0;；继续条件 i<comm->localRanks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       263. 条件判断：(highestTypes[i] > comm->intraHighestTransportType)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         264. 真分支：；仅条件为真执行以下子项。
 *           265. 调用 / 计算 / 写入：comm->intraHighestTransportType = highestTypes[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     266. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     267. 调用 / 计算 / 写入：INFO(NCCL_INIT, "rank %d Connected CollNet", rank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     268. 跳转标签：collnet_cleanup；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *       269. 调用 / 计算 / 写入：free(heads);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     270. 条件判断：(ret != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       271. 真分支：；仅条件为真执行以下子项。
 *         272. 调用 / 计算 / 写入：NCCLCHECK(ncclTransportCollNetFree(comm));；失败时可能提前return，成功才继续；关闭已创建的 CollNet 特殊连接，配合初始化失败回退或整体销毁。
 *         273. 调用 / 计算 / 写入：comm->collNetSupport = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         274. 调用 / 计算 / 写入：ret = ncclSuccess;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 275. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d - CONNECTED %d RINGS AND TREES", rank, nranks, comm->nChannels);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 276. 先执行一次的循环：初始 ；继续条件 (0)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *   277. 准备局部状态：int myCompCap = comm->peerInfo[rank].cudaCompCap;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   278. 准备局部状态：int minCompCap = myCompCap, maxCompCap = myCompCap;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   279. 循环：初始 int i = 0;；继续条件 i < nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     280. 调用 / 计算 / 写入：minCompCap = std::min(comm->peerInfo[i].cudaCompCap, minCompCap);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *     281. 调用 / 计算 / 写入：maxCompCap = std::max(comm->peerInfo[i].cudaCompCap, maxCompCap);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   282. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   283. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoTuneModel(comm, minCompCap, maxCompCap, &treeGraph, &ringGraph, &collNetGraph));；失败时可能提前return，成功才继续；结合拓扑图带宽、节点规模、GPU 架构和配置构建各 API/算法/协议的延迟与带宽表。
 * 284. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 285. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoComputeP2pChannels(comm));；失败时可能提前return，成功才继续；收敛所有本地 GPU 到 peer 的通道预算，计算全局 P2P 空间与每 peer 数量并生成位反转偏移。
 * 286. 条件判断：(ncclParamNvbPreconnect())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   287. 真分支：；仅条件为真执行以下子项。
 *     288. 准备局部状态：int nvbNpeers;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     289. 准备局部状态：int* nvbPeers;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     290. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetNvbGpus(comm->topo, comm->rank, &nvbNpeers, &nvbPeers));；失败时可能提前return，成功才继续；找出可经一个中间 GPU 走 NVLink 桥接的 peer，为特定机内路径预连接提供候选。
 *     291. 循环：初始 int r=0;；继续条件 r<nvbNpeers；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       292. 准备局部状态：int peer = nvbPeers[r];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       293. 准备局部状态：int channelId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       294. 循环：初始 int c=0;；继续条件 c<comm->p2pnChannelsPerPeer；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         295. 调用 / 计算 / 写入：NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncSend, &channelId));；失败时可能提前return，成功才继续；组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。
 *         296. 条件判断：(comm->channels[channelId].peers[peer].send[1].connected == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           297. 真分支：；仅条件为真执行以下子项。
 *             298. 调用 / 计算 / 写入：comm->connectSend[peer] |= (1<<channelId);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       299. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       300. 循环：初始 int c=0;；继续条件 c<comm->p2pnChannelsPerPeer；每轮更新 c++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         301. 调用 / 计算 / 写入：NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncRecv, &channelId));；失败时可能提前return，成功才继续；组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。
 *         302. 条件判断：(comm->channels[channelId].peers[peer].recv[1].connected == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           303. 真分支：；仅条件为真执行以下子项。
 *             304. 调用 / 计算 / 写入：comm->connectRecv[peer] |= (1<<channelId);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       305. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     306. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     307. 调用 / 计算 / 写入：NCCLCHECK(ncclTransportP2pSetup(comm, NULL, 1));；失败时可能提前return，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。
 *     308. 调用 / 计算 / 写入：free(nvbPeers);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 309. 准备局部状态：struct ncclProxyConnector proxyConn;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 310. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, comm->rank, &proxyConn));；失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。
 * 311. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCall(&proxyConn, ncclProxyMsgSharedInit, &comm->p2pnChannels, sizeof(int), NULL, 0));；失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 312. 条件判断：(ncclPxnDisable() == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   313. 真分支：；仅条件为真执行以下子项。
 *     314. 准备局部状态：int nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     315. 准备局部状态：int* pxnPeers;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     316. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetPxnRanks(comm, &pxnPeers, &nranks));；失败时可能提前return，成功才继续；列出本 rank 网络路径可能依赖的中间 GPU rank，为共享代理资源提前准备。
 *     317. 循环：初始 int r=0;；继续条件 r<nranks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       318. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, pxnPeers[r], &proxyConn));；失败时可能提前return，成功才继续；联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。
 *       319. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyCall(&proxyConn, ncclProxyMsgSharedInit, &comm->p2pnChannels, sizeof(int), NULL, 0));；失败时可能提前return，成功才继续；通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 *     320. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     321. 调用 / 计算 / 写入：free(pxnPeers);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 322. 先执行一次的循环：初始 ；继续条件 (0)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *   323. 准备局部状态：int intraProcRank0 = -1, intraProcRank = -1, intraProcRanks = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   324. 循环：初始 int i = 0;；继续条件 i < nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     325. 条件判断：((comm->peerInfo[i].hostHash == comm->peerInfo[rank].hostHash) && (comm->peerInfo[i].pidHash == comm->peerInfo[rank].pidHash))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       326. 真分支：；仅条件为真执行以下子项。
 *         327. 条件判断：(intraProcRanks == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           328. 真分支：；仅条件为真执行以下子项。
 *             329. 调用 / 计算 / 写入：intraProcRank0 = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         330. 条件判断：(i == rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           331. 真分支：；仅条件为真执行以下子项。
 *             332. 调用 / 计算 / 写入：intraProcRank = intraProcRanks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         333. 调用 / 计算 / 写入：intraProcRanks++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   334. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   335. 调用 / 计算 / 写入：TRACE(NCCL_INIT,"pidHash[%d] %lx intraProcRank %d intraProcRanks %d intraProcRank0 %d", rank, comm->peerInfo[rank].pidHash, intraProcRank, intraProcRanks, intraProcRank0);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   336. 条件判断：(intraProcRank == -1 || intraProcRank0 == -1 || comm->peerInfo[intraProcRank0].comm == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     337. 真分支：；仅条件为真执行以下子项。
 *       338. 调用 / 计算 / 写入：WARN("Failed to determine intra proc ranks rank %d hostHash %lx pidHash %lx intraProcRank %d intraProcRanks %d intraProcRank0 %d", rank, comm->peerInfo[rank].hostHash, comm->peerInfo[rank].pidHash, in …（完整代码见函数体）；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       339. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   340. 调用 / 计算 / 写入：NCCLCHECK(ncclCommSetIntraProc(comm, intraProcRank, intraProcRanks, comm->peerInfo[intraProcRank0].comm));；失败时可能提前return，成功才继续；把同进程 rank 加入本地发射协调组，设置屏障、设备列表及协作发射能力。
 * 341. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 342. 调用 / 计算 / 写入：NCCLCHECK(bootstrapBarrier(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, comm->localRankToRank[0]));；失败时可能提前return，成功才继续；使用倍增距离的控制消息交换，使所选 rank 子集合完成初始化会合。
 * 343. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyShmUnlink(comm));；失败时可能提前return，成功才继续；移除代理共享内存对象名字，已有映射仍按各自引用和退出流程释放。
 * 344. 跳转标签：affinity_restore；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   345. 条件判断：(CPU_COUNT(&comm->cpuAffinity))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     346. 真分支：；仅条件为真执行以下子项。
 *       347. 调用 / 计算 / 写入：sched_setaffinity(0, sizeof(cpu_set_t), &affinitySave);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 348. 条件判断：(ret != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   349. 真分支：；仅条件为真执行以下子项。
 *     350. 函数返回：return ret;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 351. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d - DONE", rank, nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 352. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0895
 */
/* 中文逐项-函数 F0895：initTransportsRank
 * 用途与位置：串联 bootstrap、硬件建图、路径搜索、全局图拼接、传输建链和性能模型，建立当前 rank 通信环境。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 commId（ncclUniqueId* commId）：应用分发的引导会合标识，同一通信组所有rank需要使用同一份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitRankSync。
 * 本体直接出现的调用/包装：CPU_COUNT；NCCLCHECK；NCCLCHECKGOTO；bootstrapAllGather；bootstrapBarrier；bootstrapInit；bootstrapIntraNodeAllGather；collNetSupport；computeBuffSizes；fillInfo；free；getHash；getenv；memcpy；ncclCalloc；ncclChannelCompute；ncclCommSetIntraProc；ncclParamCollNetNodeThreshold；更多调用保留在函数体。
 * 需要对照的边界/推进条件：i < nranks；i != rank) && (comm->peerInfo[i].hostHash == comm->peerInfo[rank].hostHash) &&
 * (comm->peerInfo[i].busId == comm->peerInfo[rank].busId；CPU_COUNT(&comm->cpuAffinity；comm->rank ==
 * ncclParamGraphDumpFileRank。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclInternalError;；return ret;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
static ncclResult_t initTransportsRank(struct ncclComm* comm, ncclUniqueId* commId) {
  // We use 2 AllGathers
  // 1. { peerInfo, comm, compCap}
  // 2. { nChannels, graphInfo, topoRanks }

/* 中文逐项-变量 V3251：rank
 * rank（int；函数 initTransportsRank 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = comm->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = comm->rank;
/* 中文逐项-变量 V3252：nranks
 * nranks（int；函数 initTransportsRank 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nranks = comm->nRanks;
/* 中文逐项-变量 V3253：commHash
 * commHash（uint64_t；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 commHash = getHash(commId->internal, NCCL_UNIQUE_ID_BYTES)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  uint64_t commHash = getHash(commId->internal, NCCL_UNIQUE_ID_BYTES);
  TRACE(NCCL_INIT, "comm %p, commHash %lx, rank %d nranks %d - BEGIN", comm, commHash, rank, nranks);
  NCCLCHECK(bootstrapInit(commId, comm));

/* 中文导读：
 * 这一轮 AllGather 是 bootstrap 控制面操作，交换 peerInfo，而非用户调用的 GPU AllGather 集合通信。
 * 使用 hostHash/pidHash/busId 等识别主机、进程和 GPU，为去重、路径选择和 IPC 建立依据。
 */
  // AllGather1 - begin
  NCCLCHECK(ncclCalloc(&comm->peerInfo, nranks+1)); // Extra rank to represent CollNet root
  NCCLCHECK(fillInfo(comm, comm->peerInfo+rank, commHash));
  NCCLCHECK(bootstrapAllGather(comm->bootstrap, comm->peerInfo, sizeof(struct ncclPeerInfo)));

  for (/* 中文逐项-变量 V3254：i
 * i（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < nranks；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < nranks; i++) {
    if ((i != rank) && (comm->peerInfo[i].hostHash == comm->peerInfo[rank].hostHash) && (comm->peerInfo[i].busId == comm->peerInfo[rank].busId)) {
      WARN("Duplicate GPU detected : rank %d and rank %d both on CUDA device %lx", rank, i, comm->peerInfo[rank].busId);
      return ncclInvalidUsage;
    }
  }

  // AllGather1 - end

/* 中文导读：
 * 先算路径、裁剪不可达/无用节点，再重算路径；搜索器只在有效硬件子图上工作。
 * GPU 物理互联能力、网卡亲和性与后端 GDR 支持共同影响最终路径。
 */
  // Topo detection / System graph creation
  NCCLCHECK(ncclTopoGetSystem(comm, &comm->topo));
  // Compute paths between GPUs and NICs
  NCCLCHECK(ncclTopoComputePaths(comm->topo, comm->peerInfo));
  // Remove inaccessible GPUs and unused NICs
  NCCLCHECK(ncclTopoTrimSystem(comm->topo, comm));
  // Recompute paths after trimming
  NCCLCHECK(ncclTopoComputePaths(comm->topo, comm->peerInfo));
  // Init search
  NCCLCHECK(ncclTopoSearchInit(comm->topo));
  // Print final topology
  NCCLCHECK(ncclTopoPrint(comm->topo));

  // Set Affinity to a CPU local the our GPU, so that all memory we allocate
  // on the host is local.
  NCCLCHECK(ncclTopoGetCpuAffinity(comm->topo, comm->rank, &comm->cpuAffinity));
/* 中文逐项-变量 V3255：affinitySave
 * affinitySave（cpu_set_t；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sched_getaffinity(0, sizeof(cpu_set_t), &affinitySave);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  cpu_set_t affinitySave;
  if (CPU_COUNT(&comm->cpuAffinity)) {
    sched_getaffinity(0, sizeof(cpu_set_t), &affinitySave);
    sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);
  }
/* 中文逐项-变量 V3256：ret
 * ret（ncclResult_t；函数 initTransportsRank 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(setupChannel(comm, c, rank, nranks, rings+c*nranks), ret,
 * affinity_restore);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t ret;

  // Launch proxy service thread
  NCCLCHECK(ncclProxyCreate(comm));

/* 中文导读：
 * 分别搜索 Ring、Balanced Tree 和 CollNet 图。本版本 Ring 初始搜索上限是 MAXCHANNELS/2。
 * 后续 Postset 会拼接节点并可能复制 channel；最终 comm->nChannels 不能只看这里的 maxChannels。
 */
  // Get rings and trees
/* 中文逐项-变量 V3257：ringGraph
 * ringGraph（struct ncclTopoGraph；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ringGraph.id = 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoGraph ringGraph;
  ringGraph.id = 0;
  ringGraph.pattern = NCCL_TOPO_PATTERN_RING;
  ringGraph.collNet = 0;
  ringGraph.minChannels = 1;
  ringGraph.maxChannels = MAXCHANNELS/2;
  NCCLCHECK(ncclTopoCompute(comm->topo, &ringGraph));
  NCCLCHECK(ncclTopoPrintGraph(comm->topo, &ringGraph));

/* 中文逐项-变量 V3258：treeGraph
 * treeGraph（struct ncclTopoGraph；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：treeGraph.id = 1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoGraph treeGraph;
  treeGraph.id = 1;
  treeGraph.pattern = NCCL_TOPO_PATTERN_BALANCED_TREE;
  treeGraph.collNet = 0;
  treeGraph.minChannels = 1;
  treeGraph.maxChannels = ringGraph.nChannels;
  NCCLCHECK(ncclTopoCompute(comm->topo, &treeGraph));
  NCCLCHECK(ncclTopoPrintGraph(comm->topo, &treeGraph));

/* 中文逐项-变量 V3259：collNetGraph
 * collNetGraph（struct ncclTopoGraph；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：collNetGraph.id = 2;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoGraph collNetGraph;
  collNetGraph.id = 2;
  collNetGraph.pattern = NCCL_TOPO_PATTERN_TREE;
  collNetGraph.collNet = 1;
  collNetGraph.minChannels = collNetGraph.maxChannels = ringGraph.nChannels;
  NCCLCHECK(ncclTopoCompute(comm->topo, &collNetGraph));
  NCCLCHECK(ncclTopoPrintGraph(comm->topo, &collNetGraph));

  if (comm->rank == ncclParamGraphDumpFileRank()) {
/* 中文逐项-变量 V3260：graphs
 * graphs（struct ncclTopoGraph；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 graphs = { &ringGraph, &treeGraph, &collNetGraph }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
    struct ncclTopoGraph* graphs[3] = { &ringGraph, &treeGraph, &collNetGraph };
    NCCLCHECK(ncclTopoDumpGraphs(comm->topo, 3, graphs));
  }

  // Determine local CollNet support before all-gather
  if (collNetSupport()) {
/* 中文逐项-变量 V3261：collNetEnable
 * collNetEnable（char；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNetEnable = getenv("NCCL_COLLNET_ENABLE")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char *collNetEnable = getenv("NCCL_COLLNET_ENABLE");
    if (collNetEnable != NULL) {
      INFO(NCCL_ALL, "NCCL_COLLNET_ENABLE set by environment to %s.", collNetEnable);
      if (strcmp(collNetEnable, "1") == 0) {
        comm->collNetSupport = 1;
      }
    }
  }
  if (comm->collNetSupport == 1 && collNetGraph.nChannels <= 0) comm->collNetSupport = 0;

  // AllGather3 - begin
/* 中文逐项-类型 T0388：ncclGraphInfo
 * 类型职责：初始化全组交换的图参数摘要，使不同节点的通道数/速度/模式收敛一致。
 * 声明内可见成员（含嵌套成员）：pattern、nChannels、sameChannels、speedIntra、speedInter、typeIntra、typeInter。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct ncclGraphInfo {
/* 中文逐项-变量 V3262：pattern
 * pattern（int；成员 ncclGraphInfo）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
    int pattern;
/* 中文逐项-变量 V3263：nChannels
 * nChannels（int；成员 ncclGraphInfo）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 */
    int nChannels;
/* 中文逐项-变量 V3264：sameChannels
 * sameChannels（int；成员 ncclGraphInfo）：图搜索是否允许进出不同NIC、是否重放相同机内GPU顺序的限制。
 */
    int sameChannels;
/* 中文逐项-变量 V3265：speedIntra
 * speedIntra（float；成员 ncclGraphInfo）：图中每通道机内/机间速度与机间延迟估计，不是运行时逐包测量。
 */
    float speedIntra;
/* 中文逐项-变量 V3266：speedInter
 * speedInter（float；成员 ncclGraphInfo）：图中每通道机内/机间速度与机间延迟估计，不是运行时逐包测量。
 */
    float speedInter;
/* 中文逐项-变量 V3267：typeIntra
 * typeIntra（int；成员 ncclGraphInfo）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
    int typeIntra;
/* 中文逐项-变量 V3268：typeInter
 * typeInter（int；成员 ncclGraphInfo）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 */
    int typeInter;
  };

/* 中文逐项-变量 V3269：allGather3Data
 * allGather3Data（struct { int netDev; int collNetSupport; struct ncclGraphInfo tree; struct ncclGraphInfo ring;
 * struct ncclGrap …（完整表达式见紧邻源码）；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&allGather3Data, nranks));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
/* 中文逐项-类型 T0389：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：netDev、collNetSupport、tree、ring、collNet、topoRanks。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct {
/* 中文逐项-变量 V3270：netDev
 * netDev（int；成员 <匿名>）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
    int netDev;
/* 中文逐项-变量 V3271：collNetSupport
 * collNetSupport（int；成员 <匿名>）：该communicator是否具备一致可用的CollNet路径，算法选型还要检查类型/归约支持。
 */
    int collNetSupport;
/* 中文逐项-变量 V3272：tree
 * tree（struct ncclGraphInfo；成员 <匿名>）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 */
    struct ncclGraphInfo tree;
/* 中文逐项-变量 V3273：ring
 * ring（struct ncclGraphInfo；成员 <匿名>）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 */
    struct ncclGraphInfo ring;
/* 中文逐项-变量 V3274：collNet
 * collNet（struct ncclGraphInfo；成员 <匿名>）：图搜索任务是否服务网络集合归约，影响端点与速度约束。
 */
    struct ncclGraphInfo collNet;
/* 中文逐项-变量 V3275：topoRanks
 * topoRanks（struct ncclTopoRanks；成员 <匿名>）：该rank的局部拓扑端点摘要，初始化AllGather后供全局Ring/Tree拼接。
 */
    struct ncclTopoRanks topoRanks;
  } *allGather3Data;

  NCCLCHECK(ncclCalloc(&allGather3Data, nranks));
  NCCLCHECK(ncclTopoGetLocalNet(comm->topo, rank, &allGather3Data[rank].netDev));
  allGather3Data[rank].tree.pattern = treeGraph.pattern;
  allGather3Data[rank].tree.nChannels = treeGraph.nChannels;
  allGather3Data[rank].tree.sameChannels = treeGraph.sameChannels;
  allGather3Data[rank].tree.speedIntra = treeGraph.speedIntra;
  allGather3Data[rank].tree.speedInter = treeGraph.speedInter;
  allGather3Data[rank].tree.typeIntra = treeGraph.typeIntra;
  allGather3Data[rank].tree.typeInter = treeGraph.typeInter;
  allGather3Data[rank].ring.pattern = ringGraph.pattern;
  allGather3Data[rank].ring.nChannels = ringGraph.nChannels;
  allGather3Data[rank].ring.sameChannels = ringGraph.sameChannels;
  allGather3Data[rank].ring.speedIntra = ringGraph.speedIntra;
  allGather3Data[rank].ring.speedInter = ringGraph.speedInter;
  allGather3Data[rank].ring.typeIntra = ringGraph.typeIntra;
  allGather3Data[rank].ring.typeInter = ringGraph.typeInter;
  allGather3Data[rank].collNet.pattern = collNetGraph.pattern;
  allGather3Data[rank].collNet.nChannels = collNetGraph.nChannels;
  allGather3Data[rank].collNet.sameChannels = collNetGraph.sameChannels;
  allGather3Data[rank].collNet.speedIntra = collNetGraph.speedIntra;
  allGather3Data[rank].collNet.speedInter = collNetGraph.speedInter;
  allGather3Data[rank].collNet.typeIntra = collNetGraph.typeIntra;
  allGather3Data[rank].collNet.typeInter = collNetGraph.typeInter;
  allGather3Data[rank].collNetSupport = comm->collNetSupport;

  comm->nChannels = std::min(treeGraph.nChannels, ringGraph.nChannels);
  NCCLCHECK(ncclTopoPreset(comm, &treeGraph, &ringGraph, &allGather3Data[rank].topoRanks));

  NCCLCHECK(bootstrapAllGather(comm->bootstrap, allGather3Data, sizeof(*allGather3Data)));

  // Determine nNodes, firstRanks, ...
/* 中文逐项-变量 V3276：nodesFirstRank、nodesTreePatterns
 * nodesFirstRank（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&nodesFirstRank, nranks));。若由输出参数填入，必须先检查填充调用成功再读值。
 * nodesTreePatterns（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&nodesTreePatterns, nranks));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int *nodesFirstRank, *nodesTreePatterns;
  NCCLCHECK(ncclCalloc(&nodesFirstRank, nranks));
  NCCLCHECK(ncclCalloc(&nodesTreePatterns, nranks));
  NCCLCHECK(ncclCalloc(&comm->rankToNode, comm->nRanks));
  for (/* 中文逐项-变量 V3277：r
 * r（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nranks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nranks; r++) {
/* 中文逐项-变量 V3278：node
 * node（int；函数 initTransportsRank 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 此处未显式给初值；后续首个文本使用点：for (node=0; node<comm->nNodes && nodesFirstRank[node] != firstRank;
 * node++);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int node;
/* 中文逐项-变量 V3279：firstRank
 * firstRank（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 firstRank = allGather3Data[r].topoRanks.ringRecv[0]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int firstRank = allGather3Data[r].topoRanks.ringRecv[0];
    for (node=0; node<comm->nNodes && nodesFirstRank[node] != firstRank; node++);
    if (node == comm->nNodes) {
      comm->nNodes++;
      nodesFirstRank[node] = firstRank;
      // Record tree pattern of each node as they can be different depending on sm arch
      nodesTreePatterns[node] = allGather3Data[r].tree.pattern;
    }
    comm->rankToNode[r] = node;
  }
  // Now that we know nNodes, alloc nodeRanks and compute localRanks for each node
  NCCLCHECK(ncclCalloc(&comm->nodeRanks, comm->nNodes));
  NCCLCHECK(ncclCalloc(&comm->rankToLocalRank, comm->nRanks));
  for (/* 中文逐项-变量 V3280：r
 * r（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->nRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->nRanks; r++) {
/* 中文逐项-变量 V3281：node
 * node（int；函数 initTransportsRank 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = comm->rankToNode[r]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int node = comm->rankToNode[r];
    comm->rankToLocalRank[r] = comm->nodeRanks[node].localRanks;
    comm->nodeRanks[node].localRanks++;
  }
  // Allocate ranks arrays for each node
  for (/* 中文逐项-变量 V3282：n
 * n（int；函数 initTransportsRank 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<comm->nNodes；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<comm->nNodes; n++) {
    NCCLCHECK(ncclCalloc(&comm->nodeRanks[n].localRankToRank, comm->nodeRanks[n].localRanks));
    comm->maxLocalRanks = std::max(comm->maxLocalRanks, comm->nodeRanks[n].localRanks);
    comm->nodeRanks[n].localRanks = 0;
  }
  // And fill the ranks arrays
  for (/* 中文逐项-变量 V3283：r
 * r（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->nRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->nRanks; r++) {
/* 中文逐项-变量 V3284：node
 * node（int；函数 initTransportsRank 内）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 初值/来源 node = comm->rankToNode[r]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int node = comm->rankToNode[r];
    comm->nodeRanks[node].localRankToRank[comm->nodeRanks[node].localRanks++] = r;
  }
  comm->node = comm->rankToNode[rank];
  comm->localRankToRank = comm->nodeRanks[comm->node].localRankToRank;
  comm->localRank = comm->rankToLocalRank[rank];
  comm->localRanks = comm->nodeRanks[comm->node].localRanks;

  TRACE(NCCL_INIT,"hostHash[%d] %lx localRank %d localRanks %d localRank0 %d",
        rank, comm->peerInfo[rank].hostHash, comm->localRank, comm->localRanks, comm->localRankToRank[0]);
  if (comm->localRank == -1 || comm->localRankToRank[0] == -1 || comm->localRanks == 0) {
    WARN("Failed to determine local ranks rank %d hostHash %lx pidHash %lx localRank %d localRanks %d localRank0 %d",
         rank, comm->peerInfo[rank].hostHash, comm->peerInfo[rank].pidHash,
         comm->localRank, comm->localRanks, comm->localRankToRank[0]);
    return ncclInternalError;
  }

/* 中文逐项-变量 V3285：nChannelsOrig
 * nChannelsOrig（int；函数 initTransportsRank 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nChannelsOrig = comm->nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nChannelsOrig = comm->nChannels;
/* 中文逐项-变量 V3286：allTopoRanks
 * allTopoRanks（struct ncclTopoRanks；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&allTopoRanks, comm->nRanks));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclTopoRanks** allTopoRanks;
  NCCLCHECK(ncclCalloc(&allTopoRanks, comm->nRanks));
  for (/* 中文逐项-变量 V3287：i
 * i（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nranks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nranks; i++) {
    comm->peerInfo[i].netDev = allGather3Data[i].netDev;
    allTopoRanks[i] = &allGather3Data[i].topoRanks;
    // Make sure we align all ranks so that the tuning is consistent across ranks
    treeGraph.nChannels = std::min(allGather3Data[i].tree.nChannels, treeGraph.nChannels);
    treeGraph.sameChannels = std::min(allGather3Data[i].tree.sameChannels, treeGraph.sameChannels);
    treeGraph.speedIntra = std::min(allGather3Data[i].tree.speedIntra, treeGraph.speedIntra);
    treeGraph.speedInter = std::min(allGather3Data[i].tree.speedInter, treeGraph.speedInter);
    treeGraph.typeIntra = std::max(allGather3Data[i].tree.typeIntra, treeGraph.typeIntra);
    treeGraph.typeInter = std::max(allGather3Data[i].tree.typeInter, treeGraph.typeInter);
    ringGraph.nChannels = std::min(allGather3Data[i].ring.nChannels, ringGraph.nChannels);
    ringGraph.sameChannels = std::min(allGather3Data[i].ring.sameChannels, ringGraph.sameChannels);
    ringGraph.speedIntra = std::min(allGather3Data[i].ring.speedIntra, ringGraph.speedIntra);
    ringGraph.speedInter = std::min(allGather3Data[i].ring.speedInter, ringGraph.speedInter);
    ringGraph.typeIntra = std::max(allGather3Data[i].ring.typeIntra, ringGraph.typeIntra);
    ringGraph.typeInter = std::max(allGather3Data[i].ring.typeInter, ringGraph.typeInter);
    collNetGraph.nChannels = std::min(allGather3Data[i].collNet.nChannels, collNetGraph.nChannels);
    collNetGraph.sameChannels = std::min(allGather3Data[i].collNet.sameChannels, collNetGraph.sameChannels);
    collNetGraph.speedIntra = std::min(allGather3Data[i].collNet.speedIntra, collNetGraph.speedIntra);
    collNetGraph.speedInter = std::min(allGather3Data[i].collNet.speedInter, collNetGraph.speedInter);
    collNetGraph.typeIntra = std::max(allGather3Data[i].collNet.typeIntra, collNetGraph.typeIntra);
    collNetGraph.typeInter = std::max(allGather3Data[i].collNet.typeInter, collNetGraph.typeInter);
    comm->collNetSupport = std::min(allGather3Data[i].collNetSupport, comm->collNetSupport);
  }

/* 中文精读：跨 rank 收敛后的 channel 数
 * 各 rank/节点的本地图搜索结果可能不同，前面的 AllGather 后用最小可用通道/速度收敛。
 * 这里把 Ring/Tree 对齐到共同数量；后续 TopoPostset 还会复制通道并应用 MIN/MAX 约束。
 * 所以 graph 搜索日志的 nChannels、最终 comm->nChannels、本次 API 的 info->nChannels
 * 处于三个阶段，不能拿其中一个数代表另外两个。
 */
  comm->nChannels = treeGraph.nChannels = ringGraph.nChannels = std::min(treeGraph.nChannels, ringGraph.nChannels);
  if (comm->nChannels < nChannelsOrig) {
    // We started duplicating channels during Preset(), so we need to move the
    // duplicated channels since we have removed some.
    for (/* 中文逐项-变量 V3288：i
 * i（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->nChannels; i++) memcpy(comm->channels+comm->nChannels+i, comm->channels+nChannelsOrig+i, sizeof(struct ncclChannel));
  }

  // Determine CollNet support after all-gather now that we know nNodes and each node localRanks
  if (comm->collNetSupport == 1) {
/* 中文逐项-变量 V3289：collNetNodeThreshold
 * collNetNodeThreshold（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNetNodeThreshold = ncclParamCollNetNodeThreshold()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
    int collNetNodeThreshold = ncclParamCollNetNodeThreshold();
    if (comm->nNodes < collNetNodeThreshold) {
      INFO(NCCL_INIT, "Communicator has %d nodes which is less than CollNet node threshold %d, disabling CollNet", comm->nNodes, collNetNodeThreshold);
      comm->collNetSupport = 0;
    }
    for (/* 中文逐项-变量 V3290：n
 * n（int；函数 initTransportsRank 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：n<comm->nNodes；循环体与增量表达式决定每轮如何推进。
 */int n=0; n<comm->nNodes; n++) {
      if (comm->nodeRanks[n].localRanks > NCCL_MAX_DIRECT_ARITY+1) {
        WARN("CollNet currently only supports up to %d GPUs per node, disabling CollNet", NCCL_MAX_DIRECT_ARITY+1);
        comm->collNetSupport = 0;
        break;
      }
    }
  }

/* 中文逐项-变量 V3291：rings
 * rings（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&rings, nranks*MAXCHANNELS));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int *rings;
  NCCLCHECK(ncclCalloc(&rings, nranks*MAXCHANNELS));
  NCCLCHECK(ncclTopoPostset(comm, nodesFirstRank, nodesTreePatterns, allTopoRanks, rings, &collNetGraph));

  free(allTopoRanks);
  free(nodesTreePatterns);
  free(nodesFirstRank);
  free(allGather3Data);

  // AllGather3 - end

  TRACE(NCCL_INIT, "rank %d nranks %d - BUILT %d TREES/RINGS", rank, nranks, comm->nChannels);

/* 中文逐项-变量 V3292：line
 * line（char；函数 initTransportsRank 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：line[0]='\0';。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[1024];
  line[0]='\0';
  for (/* 中文逐项-变量 V3293：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->nChannels; c++) {
/* 中文逐项-变量 V3294：tree
 * tree（struct ncclTree；函数 initTransportsRank 内）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 * 初值/来源 tree = &comm->channels[c].tree；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    struct ncclTree* tree = &comm->channels[c].tree;
    snprintf(line+strlen(line), 1023-strlen(line), " [%d] %d/%d/%d->%d->%d",
        c, tree->down[0], tree->down[1], tree->down[2], rank, tree->up);
    INFO(NCCL_GRAPH, "Ring %02d : %d -> %d -> %d", c, comm->channels[c].ring.prev, comm->rank, comm->channels[c].ring.next);
  }
  line[1023] = '\0';
  INFO(NCCL_INIT, "Trees%s", line);

  NCCLCHECK(computeBuffSizes(comm));

/* 中文导读：
 * 每个 channel 先标记 Ring 前驱接收/后继发送，再统一 Setup 交换句柄并建链。
 * connectSend/connectRecv 是待建链位图；已 connected 的同槽连接不重复创建。
 */
  // Connect with prev/next for each ring
  for (/* 中文逐项-变量 V3295：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->nChannels; c++) {
/* 中文逐项-变量 V3296：channel
 * channel（struct ncclChannel；函数 initTransportsRank 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+c;
    NCCLCHECKGOTO(setupChannel(comm, c, rank, nranks, rings+c*nranks), ret, affinity_restore);
    if (comm->nRanks == 1) continue;
    NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, 1, &channel->ring.prev, 1, &channel->ring.next, 0), ret, affinity_restore);
  }
  NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &ringGraph, 0), ret, affinity_restore);
  free(rings);
  INFO(NCCL_INIT, "Connected all rings");

/* 中文导读：
 * 在同一个连接索引 0 上补齐 Tree 的父子双向边。
 * 若 Ring 已经连接了相同 channel/peer/方向，连接标记逻辑会复用该对象，不能简单相加 Ring 与 Tree 边数。
 */
  // Connect Trees
  for (/* 中文逐项-变量 V3297：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->nChannels; c++) {
/* 中文逐项-变量 V3298：channel
 * channel（struct ncclChannel；函数 initTransportsRank 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclChannel* channel = comm->channels+c;
    if (comm->nRanks == 1) continue;
    NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, NCCL_MAX_TREE_ARITY, channel->tree.down, 1, &channel->tree.up, 0), ret, affinity_restore);
    NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channel, 1, &channel->tree.up, NCCL_MAX_TREE_ARITY, channel->tree.down, 0), ret, affinity_restore);
  }
  NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &treeGraph, 0), ret, affinity_restore);
  INFO(NCCL_INIT, "Connected all trees");

  // Check if we can setup CollNet
  if (comm->collNetSupport > 0) {
/* 中文逐项-变量 V3299：collNetSetupFail
 * collNetSetupFail（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNetSetupFail = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int collNetSetupFail = 0;
/* 中文逐项-变量 V3300：highestTypes
 * highestTypes（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 highestTypes = {TRANSPORT_P2P}；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
    int highestTypes[NCCL_MAX_LOCAL_RANKS] = {TRANSPORT_P2P};
    // Find all head ranks
/* 中文逐项-变量 V3301：nHeads
 * nHeads（int；函数 initTransportsRank 内）：当前 CollNet 分工的网络 head 数，额外参与一轮数据块数量计算。
 * 初值/来源 nHeads = collNetGraph.nChannels；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int nHeads = collNetGraph.nChannels;
/* 中文逐项-变量 V3302：heads
 * heads（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&heads, nHeads));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int *heads;
    NCCLCHECK(ncclCalloc(&heads, nHeads));
    // Head GPU index is always 0
    for (/* 中文逐项-变量 V3303：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<nHeads；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<nHeads; c++) {
      heads[c] = collNetGraph.intra[c*comm->localRanks+0];
    }
    for (/* 中文逐项-变量 V3304：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->nChannels; c++) {
/* 中文逐项-变量 V3305：channel
 * channel（struct ncclChannel；函数 initTransportsRank 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclChannel* channel = comm->channels+c;
      for (/* 中文逐项-变量 V3306：h
 * h（int；函数 initTransportsRank 内）：GDRCopy固定缓冲句柄的内部表示，必须用匹配的unpin接口回收。
 * 初值/来源 h = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：h<nHeads；循环体与增量表达式决定每轮如何推进。
 */int h=0; h<nHeads; h++) {
/* 中文逐项-变量 V3307：head
 * head（int；函数 initTransportsRank 内）：消费者已经释放的协议进度，生产者据此判断循环缓冲能否重用。
 * 初值/来源 head = heads[h]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        const int head = heads[h];
        collNetSetupFail = ncclTransportCollNetSetup(comm, &collNetGraph, channel, head, head, h, collNetRecv);
        if (!collNetSetupFail) collNetSetupFail = ncclTransportCollNetSetup(comm, &collNetGraph, channel, head, head, h, collNetSend);
      }
      // Verify CollNet setup across ranks after trying the first channel
      if (c == 0) {
        NCCLCHECKGOTO(ncclTransportCollNetCheck(comm, collNetSetupFail), ret, collnet_cleanup);
      }
    }
    // Verify CollNet setup across ranks after trying all channels
    NCCLCHECKGOTO(ncclTransportCollNetCheck(comm, collNetSetupFail), ret, collnet_cleanup);
    TRACE(NCCL_INIT, "rank %d Connected inter-node CollNet", rank);

    // Connect intra-node CollNet
/* 中文逐项-变量 V3308：highestTransportType0、highestTransportType1
 * highestTransportType0（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 0, &highestTransportType0), ret,
 * collnet_cleanup);。若由输出参数填入，必须先检查填充调用成功再读值。
 * highestTransportType1（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 1, &highestTransportType1), ret,
 * collnet_cleanup);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int highestTransportType0, highestTransportType1;
    for (/* 中文逐项-变量 V3309：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->nChannels; c++) {
/* 中文逐项-变量 V3310：channelRecv
 * channelRecv（struct ncclChannel；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channelRecv = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclChannel* channelRecv = comm->channels+c;
      NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channelRecv, NCCL_MAX_DIRECT_ARITY, channelRecv->collTree.up, NCCL_MAX_DIRECT_ARITY, channelRecv->collTree.down, 0), ret, collnet_cleanup);
    }
    NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 0, &highestTransportType0), ret, collnet_cleanup);
    for (/* 中文逐项-变量 V3311：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->nChannels；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->nChannels; c++) {
/* 中文逐项-变量 V3312：channelSend
 * channelSend（struct ncclChannel；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 channelSend = comm->channels+c；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclChannel* channelSend = comm->channels+c;
      NCCLCHECKGOTO(ncclTransportP2pConnect(comm, channelSend, NCCL_MAX_DIRECT_ARITY, channelSend->collTree.down, NCCL_MAX_DIRECT_ARITY, channelSend->collTree.up, 1), ret, collnet_cleanup);
    }
    NCCLCHECKGOTO(ncclTransportP2pSetup(comm, &collNetGraph, 1, &highestTransportType1), ret, collnet_cleanup);

    // Exchange highest intra-node transport type among ranks
    // because we need to know whether all ranks can p2p each other to determine whether we can directly read/write registered user buffer
    comm->intraHighestTransportType = highestTypes[comm->localRank] = highestTransportType0 > highestTransportType1 ? highestTransportType0 : highestTransportType1;
    NCCLCHECK(bootstrapIntraNodeAllGather(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, highestTypes, sizeof(int)));
    for (/* 中文逐项-变量 V3313：i
 * i（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->localRanks; i++) {
      if (highestTypes[i] > comm->intraHighestTransportType)
        comm->intraHighestTransportType = highestTypes[i];
    }
    INFO(NCCL_INIT, "rank %d Connected CollNet", rank);

collnet_cleanup:
    free(heads);
    if (ret != ncclSuccess) {
      NCCLCHECK(ncclTransportCollNetFree(comm));
      comm->collNetSupport = 0;
      ret = ncclSuccess;
    }
  }
  TRACE(NCCL_INIT, "rank %d nranks %d - CONNECTED %d RINGS AND TREES", rank, nranks, comm->nChannels);

  // Compute time models for algorithm and protocol combinations
  do {
/* 中文逐项-变量 V3314：myCompCap
 * myCompCap（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 myCompCap = comm->peerInfo[rank].cudaCompCap；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int myCompCap = comm->peerInfo[rank].cudaCompCap;
/* 中文逐项-变量 V3315：minCompCap、maxCompCap
 * minCompCap（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 minCompCap = myCompCap；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * maxCompCap（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxCompCap = myCompCap；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int minCompCap = myCompCap, maxCompCap = myCompCap;
    for (/* 中文逐项-变量 V3316：i
 * i（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < nranks；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < nranks; i++) {
      minCompCap = std::min(comm->peerInfo[i].cudaCompCap, minCompCap);
      maxCompCap = std::max(comm->peerInfo[i].cudaCompCap, maxCompCap);
    }
    NCCLCHECK(ncclTopoTuneModel(comm, minCompCap, maxCompCap, &treeGraph, &ringGraph, &collNetGraph));
  } while(0);

/* 中文导读：
 * 计算 P2P 总 channel 空间与每 peer channel 数，生成映射；这一步不等于对所有 rank 全互联。
 * 普通 Send/Recv 还会在任务出现时记录待连接位图，由 GroupEnd 预连接路径处理。
 */
  // Compute nChannels per peer for p2p
  NCCLCHECK(ncclTopoComputeP2pChannels(comm));

  if (ncclParamNvbPreconnect()) {
    // Connect p2p when using NVB path
/* 中文逐项-变量 V3317：nvbNpeers
 * nvbNpeers（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNvbGpus(comm->topo, comm->rank, &nvbNpeers,
 * &nvbPeers));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int nvbNpeers;
/* 中文逐项-变量 V3318：nvbPeers
 * nvbPeers（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetNvbGpus(comm->topo, comm->rank, &nvbNpeers,
 * &nvbPeers));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int* nvbPeers;
    NCCLCHECK(ncclTopoGetNvbGpus(comm->topo, comm->rank, &nvbNpeers, &nvbPeers));
    for (/* 中文逐项-变量 V3319：r
 * r（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nvbNpeers；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nvbNpeers; r++) {
/* 中文逐项-变量 V3320：peer
 * peer（int；函数 initTransportsRank 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = nvbPeers[r]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int peer = nvbPeers[r];
/* 中文逐项-变量 V3321：channelId
 * channelId（int；函数 initTransportsRank 内）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncSend, &channelId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int channelId;
      for (/* 中文逐项-变量 V3322：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->p2pnChannelsPerPeer；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->p2pnChannelsPerPeer; c++) {
        NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncSend, &channelId));
        if (comm->channels[channelId].peers[peer].send[1].connected == 0) {
          comm->connectSend[peer] |= (1<<channelId);
        }
      }
      for (/* 中文逐项-变量 V3323：c
 * c（int；函数 initTransportsRank 内）：本作用域通道/候选遍历位置，实际上界由下面循环条件限定。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c<comm->p2pnChannelsPerPeer；循环体与增量表达式决定每轮如何推进。
 */int c=0; c<comm->p2pnChannelsPerPeer; c++) {
        NCCLCHECK(ncclChannelCompute(comm, peer, c, ncclFuncRecv, &channelId));
        if (comm->channels[channelId].peers[peer].recv[1].connected == 0) {
          comm->connectRecv[peer] |= (1<<channelId);
        }
      }
    }
    NCCLCHECK(ncclTransportP2pSetup(comm, NULL, 1));
    free(nvbPeers);
  }

  // Connect to local net proxy
/* 中文逐项-变量 V3324：proxyConn
 * proxyConn（struct ncclProxyConnector；函数 initTransportsRank 内）：逻辑连接到代理服务的引用，记录代理 rank 和实际后台连接。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, comm->rank,
 * &proxyConn));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyConnector proxyConn;
  NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, comm->rank, &proxyConn));
  NCCLCHECK(ncclProxyCall(&proxyConn, ncclProxyMsgSharedInit, &comm->p2pnChannels, sizeof(int), NULL, 0));

  // Then to remote ones when using PXN
  if (ncclPxnDisable() == 0) {
/* 中文逐项-变量 V3325：nranks
 * nranks（int；函数 initTransportsRank 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetPxnRanks(comm, &pxnPeers, &nranks));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int nranks;
/* 中文逐项-变量 V3326：pxnPeers
 * pxnPeers（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclTopoGetPxnRanks(comm, &pxnPeers, &nranks));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int* pxnPeers;
    NCCLCHECK(ncclTopoGetPxnRanks(comm, &pxnPeers, &nranks));
    for (/* 中文逐项-变量 V3327：r
 * r（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nranks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nranks; r++) {
      NCCLCHECK(ncclProxyConnect(comm, TRANSPORT_NET, 1, pxnPeers[r], &proxyConn));
      NCCLCHECK(ncclProxyCall(&proxyConn, ncclProxyMsgSharedInit, &comm->p2pnChannels, sizeof(int), NULL, 0));
    }
    free(pxnPeers);
  }

  do {
    // Compute intra-process ranks
/* 中文逐项-变量 V3328：intraProcRank0、intraProcRank、intraProcRanks
 * intraProcRank0（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 intraProcRank0 = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 * intraProcRank（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 intraProcRank = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 * intraProcRanks（int；函数 initTransportsRank 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 intraProcRanks = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int intraProcRank0 = -1, intraProcRank = -1, intraProcRanks = 0;
    for (/* 中文逐项-变量 V3329：i
 * i（int；函数 initTransportsRank 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < nranks；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < nranks; i++) {
      if ((comm->peerInfo[i].hostHash == comm->peerInfo[rank].hostHash)
          && (comm->peerInfo[i].pidHash == comm->peerInfo[rank].pidHash)) {
        // Rank is in same process
        if (intraProcRanks == 0) intraProcRank0 = i;
        if (i == rank) intraProcRank = intraProcRanks;
        intraProcRanks++;
      }
    }
    TRACE(NCCL_INIT,"pidHash[%d] %lx intraProcRank %d intraProcRanks %d intraProcRank0 %d",
        rank, comm->peerInfo[rank].pidHash, intraProcRank, intraProcRanks, intraProcRank0);
    if (intraProcRank == -1 || intraProcRank0 == -1 || comm->peerInfo[intraProcRank0].comm == NULL) {
      WARN("Failed to determine intra proc ranks rank %d hostHash %lx pidHash %lx intraProcRank %d intraProcRanks %d intraProcRank0 %d",
          rank, comm->peerInfo[rank].hostHash, comm->peerInfo[rank].pidHash,
          intraProcRank, intraProcRanks, intraProcRank0);
      return ncclInternalError;
    }
    NCCLCHECK(ncclCommSetIntraProc(comm, intraProcRank, intraProcRanks, comm->peerInfo[intraProcRank0].comm));
  } while(0);

  /* Local intra-node barrier */
  NCCLCHECK(bootstrapBarrier(comm->bootstrap, comm->localRankToRank, comm->localRank, comm->localRanks, comm->localRankToRank[0]));

  // Unlink proxy shm to make sure it will be properly cleaned up.
  NCCLCHECK(ncclProxyShmUnlink(comm));

  // We should have allocated all buffers, collective fifos, ... we can
  // restore the affinity.
affinity_restore:
  if (CPU_COUNT(&comm->cpuAffinity)) sched_setaffinity(0, sizeof(cpu_set_t), &affinitySave);
  if (ret != ncclSuccess) return ret;

  TRACE(NCCL_INIT, "rank %d nranks %d - DONE", rank, nranks);
  return ncclSuccess;
}

/* 中文逐项-参数getter P0033：SetStackSize
 * 生成函数 ncclParamSetStackSize()，读取 NCCL_SET_STACK_SIZE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(SetStackSize, "SET_STACK_SIZE", 0);

/* 中文导读：
 * 真正执行单 rank 初始化的工作函数：切换 CUDA 设备，分配 communicator，建立拓扑/连接，配置设备视图。
 * Sync 指这里执行初始化流程，不代表后续集合通信会同步等待 GPU 完成。
 */
/* 中文内部逻辑 F0896：ncclCommInitRankSync
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：ncclResult_t res;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：CUDACHECK(cudaSetDevice(cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 条件判断：(maxLocalSizeBytes > 0 && ncclParamSetStackSize() == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "Setting cudaLimitStackSize to %zi", maxLocalSizeBytes);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 调用 / 计算 / 写入：CUDACHECKIGNORE(cudaDeviceSetLimit(cudaLimitStackSize, maxLocalSizeBytes));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 7. 调用 / 计算 / 写入：*newcomm = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：NCCLCHECKGOTO(commAlloc(newcomm, nranks, myrank), res, cleanup);；失败时保存错误并跳到清理标签，成功才继续；分配并初始化当前 rank 的 communicator 总对象、排队状态、事件与连接需求表。
 * 9. 调用 / 计算 / 写入：NCCLCHECKGOTO(initTransportsRank(*newcomm, &commId), res, cleanup);；失败时保存错误并跳到清理标签，成功才继续；串联 bootstrap、硬件建图、路径搜索、全局图拼接、传输建链和性能模型，建立当前 rank 通信环境。
 * 10. 调用 / 计算 / 写入：NCCLCHECKGOTO(devCommSetup(*newcomm), res, cleanup);；失败时保存错误并跳到清理标签，成功才继续；构造设备端精简 communicator 和连续 channel 布局，把主机连接视图转换成 GPU 可访问地址。
 * 11. 调用 / 计算 / 写入：INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Init COMPLETE", *newcomm, myrank, nranks, (*newcomm)->cudaDev, (*newcomm)->busId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 12. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 13. 跳转标签：cleanup；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   14. 条件判断：((*newcomm) && (*newcomm)->bootstrap)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     15. 真分支：；仅条件为真执行以下子项。
 *       16. 调用 / 计算 / 写入：bootstrapAbort((*newcomm)->bootstrap);；异常退出时关闭控制网络并回收状态，配合中止流程打破初始化等待。
 * 17. 调用 / 计算 / 写入：*newcomm = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 函数返回：return res;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0896
 */
/* 中文逐项-函数 F0896：ncclCommInitRankSync
 * 用途与位置：同步建立一个 rank 的 communicator，完成设备选择、总对象分配、传输初始化和设备视图构造。
 * 参数 newcomm（ncclComm_t* newcomm）：公开初始化结果句柄的输出地址，只有初始化成功后才能交给后续通信调用。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 commId（ncclUniqueId commId）：应用分发的引导会合标识，同一通信组所有rank需要使用同一份。
 * 参数 myrank（int myrank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 cudaDev（int cudaDev）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitRankDev。
 * 本体直接出现的调用/包装：CUDACHECK；CUDACHECKIGNORE；NCCLCHECKGOTO；bootstrapAbort；commAlloc；cudaDeviceSetLimit；cudaSetDevice；devCommSetup；initTransportsRank；ncclParamSetStackSize。
 * 需要对照的边界/推进条件：maxLocalSizeBytes > 0 && ncclParamSetStackSize() == 1；*newcomm) &&
 * (*newcomm)->bootstrap。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return res;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommInitRankSync(ncclComm_t* newcomm, int nranks, ncclUniqueId commId, int myrank, int cudaDev) {
/* 中文逐项-变量 V3330：res
 * res（ncclResult_t；函数 ncclCommInitRankSync 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(commAlloc(newcomm, nranks, myrank), res, cleanup);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t res;

  CUDACHECK(cudaSetDevice(cudaDev));
  // Set the maximum kernel stack size of all kernels to avoid
  // a CUDA memory reconfig on load (c.f. NVSHMEM issue)
  if (maxLocalSizeBytes > 0 && ncclParamSetStackSize() == 1) {
    TRACE(NCCL_INIT, "Setting cudaLimitStackSize to %zi", maxLocalSizeBytes);
    CUDACHECKIGNORE(cudaDeviceSetLimit(cudaLimitStackSize, maxLocalSizeBytes));
  }
  *newcomm = NULL;
  NCCLCHECKGOTO(commAlloc(newcomm, nranks, myrank), res, cleanup);
  NCCLCHECKGOTO(initTransportsRank(*newcomm, &commId), res, cleanup);
  NCCLCHECKGOTO(devCommSetup(*newcomm), res, cleanup);

  INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Init COMPLETE", *newcomm, myrank, nranks, (*newcomm)->cudaDev, (*newcomm)->busId);

  return ncclSuccess;
cleanup:
  if ((*newcomm) && (*newcomm)->bootstrap) bootstrapAbort((*newcomm)->bootstrap);
  *newcomm = NULL;
  return res;
}

/* 中文导读：
 * 公开初始化 API 与实际初始化之间的调度层。
 * 处于 group 时记录异步初始化任务，最外层 GroupEnd 再启动线程，使同进程多 GPU 初始化能够相互握手。
 */
/* 中文内部逻辑 F0897：ncclCommInitRankDev
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：ncclResult_t res;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：char* env = getenv("NCCL_COMM_ID");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(env && myrank == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", env);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 调用 / 计算 / 写入：NCCLCHECKGOTO(bootstrapCreateRoot(&commId, true), res, end);；失败时保存错误并跳到清理标签，成功才继续；创建 bootstrap root 监听对象和后台线程，为后续 rank 会合提供入口。
 * 7. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclInit(), res, end);；失败时保存错误并跳到清理标签，成功才继续；进程级单次初始化配置、网络与相关支持库，避免每个 communicator 重复初始化全局组件。
 * 8. 条件判断：(myrank == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：showVersion();；按日志配置打印库版本和构建相关信息，辅助确认实际加载的库。
 * 11. 调用 / 计算 / 写入：CUDACHECKGOTO(cudaFree(NULL), res, end);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 * 12. 调用 / 计算 / 写入：NCCLCHECKGOTO(PtrCheck(newcomm, "CommInitRank", "newcomm"), res, end);；失败时保存错误并跳到清理标签，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 13. 条件判断：(nranks < 1 || myrank < 0 || myrank >= nranks)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 调用 / 计算 / 写入：WARN("Invalid rank requested : %d/%d", myrank, nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     16. 调用 / 计算 / 写入：res = ncclInvalidArgument;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     17. 控制转移：goto end;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 18. 条件判断：(ncclAsyncMode())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   19. 真分支：；仅条件为真执行以下子项。
 *     20. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclAsyncInit(ncclCommInitRankSync, newcomm, nranks, commId, myrank, cudaDev), res, end);；失败时保存错误并跳到清理标签，成功才继续；把初始化函数、rank、设备和 uniqueId 复制到本线程 Group 任务数组。
 *   21. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     22. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclCommInitRankSync(newcomm, nranks, commId, myrank, cudaDev), res, end);；失败时保存错误并跳到清理标签，成功才继续；同步建立一个 rank 的 communicator，完成设备选择、总对象分配、传输初始化和设备视图构造。
 * 23. 跳转标签：end；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   24. 条件判断：(ncclAsyncMode())；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     25. 真分支：；仅条件为真执行以下子项。
 *       26. 函数返回：return ncclAsyncErrCheck(res);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     27. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       28. 函数返回：return res;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0897
 */
/* 中文逐项-函数 F0897：ncclCommInitRankDev
 * 用途与位置：按是否处于 Group 选择立即初始化或保存异步初始化任务，并处理设备上下文。
 * 参数 newcomm（ncclComm_t* newcomm）：公开初始化结果句柄的输出地址，只有初始化成功后才能交给后续通信调用。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 commId（ncclUniqueId commId）：应用分发的引导会合标识，同一通信组所有rank需要使用同一份。
 * 参数 myrank（int myrank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 cudaDev（int cudaDev）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitRank；src/init.cc::ncclCommInitAll。
 * 本体直接出现的调用/包装：CUDACHECKGOTO；NCCLCHECKGOTO；PtrCheck；bootstrapCreateRoot；cudaFree；getenv；ncclAsyncErrCheck；ncclAsyncInit；ncclAsyncMode；ncclCommInitRankSync；ncclInit；showVersion。
 * 需要对照的边界/推进条件：env && myrank == 0；myrank == 0；nranks < 1 || myrank < 0 || myrank >=
 * nranks；ncclAsyncMode。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclAsyncErrCheck(res);；return res;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclCommInitRankDev(ncclComm_t* newcomm, int nranks, ncclUniqueId commId, int myrank, int cudaDev) {
/* 中文逐项-变量 V3331：res
 * res（ncclResult_t；函数 ncclCommInitRankDev 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(bootstrapCreateRoot(&commId, true), res, end);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t res;
/* 中文逐项-变量 V3332：env
 * env（char；函数 ncclCommInitRankDev 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 env = getenv("NCCL_COMM_ID")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* env = getenv("NCCL_COMM_ID");
  if (env && myrank == 0) {
    INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", env);
    NCCLCHECKGOTO(bootstrapCreateRoot(&commId, true), res, end);
  }

  NCCLCHECKGOTO(ncclInit(), res, end);
  if (myrank == 0) showVersion();

  // Make sure the CUDA runtime is initialized.
  CUDACHECKGOTO(cudaFree(NULL), res, end);

  NCCLCHECKGOTO(PtrCheck(newcomm, "CommInitRank", "newcomm"), res, end);
  if (nranks < 1 || myrank < 0 || myrank >= nranks) {
    WARN("Invalid rank requested : %d/%d", myrank, nranks);
    res = ncclInvalidArgument;
    goto end;
  }

  if (ncclAsyncMode()) {
    NCCLCHECKGOTO(ncclAsyncInit(ncclCommInitRankSync, newcomm, nranks, commId, myrank, cudaDev), res, end);
  } else {
    NCCLCHECKGOTO(ncclCommInitRankSync(newcomm, nranks, commId, myrank, cudaDev), res, end);
  }

end:
  if (ncclAsyncMode()) return ncclAsyncErrCheck(res);
  else return res;
}

NCCL_API(ncclResult_t, ncclCommInitRank, ncclComm_t* newcomm, int nranks, ncclUniqueId commId, int myrank);
/* 中文内部逻辑 F0898：ncclCommInitRank
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 准备局部状态：int cudaDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：CUDACHECK(cudaGetDevice(&cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclCommInitRankDev(newcomm, nranks, commId, myrank, cudaDev));；失败时可能提前return，成功才继续；按是否处于 Group 选择立即初始化或保存异步初始化任务，并处理设备上下文。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0898
 */
/* 中文逐项-函数 F0898：ncclCommInitRank
 * 用途与位置：公开多进程/多机初始化入口，使用当前设备、全组规模和 uniqueId 建立当前 rank。
 * 参数 newcomm（ncclComm_t* newcomm）：公开初始化结果句柄的输出地址，只有初始化成功后才能交给后续通信调用。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 commId（ncclUniqueId commId）：应用分发的引导会合标识，同一通信组所有rank需要使用同一份。
 * 参数 myrank（int myrank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaGetDevice；ncclCommInitRankDev。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommInitRank(ncclComm_t* newcomm, int nranks, ncclUniqueId commId, int myrank) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
/* 中文逐项-变量 V3333：cudaDev
 * cudaDev（int；函数 ncclCommInitRank 内）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaGetDevice(&cudaDev));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int cudaDev;
  CUDACHECK(cudaGetDevice(&cudaDev));
  NCCLCHECK(ncclCommInitRankDev(newcomm, nranks, commId, myrank, cudaDev));
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclCommInitAll, ncclComm_t* comms, int ndev, const int* devlist);
/* 中文精读：单进程多 GPU 初始化的便捷接口
 * 为 devlist 指定的各 GPU 生成同一个 uniqueId，并用 Group 包住多次 InitRankDev。
 * 这样这些 rank 的初始化能够并行推进，避免第一个 rank 等待尚未开始初始化的其他 rank。
 * 它面向单进程本地设备；多进程/多机器通常由应用分发 uniqueId 后各自调用 ncclCommInitRank。
 * 这里 ndev 是 communicator 的 rank 数，不意味着创建一个覆盖整个集群的硬件对象。
 */
/* 中文内部逻辑 F0899：ncclCommInitAll
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(comms, "CommInitAll", "comms"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 3. 条件判断：(ndev < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：WARN("Invalid device count requested : %d", ndev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 准备局部状态：ncclUniqueId uniqueId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(ncclGetUniqueId(&uniqueId));；失败时可能提前return，成功才继续；先完成库级初始化，再生成应用需要分发给全组 rank 的 bootstrap 标识。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(ncclGroupStart());；失败时可能提前return，成功才继续；进入线程局部 Group 嵌套，最外层首次进入时清理组任务状态。
 * 10. 循环：初始 int i=0;；继续条件 i<ndev；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   11. 调用 / 计算 / 写入：ncclCommInitRankDev(comms+i, ndev, uniqueId, i, devlist ? devlist[i] : i);；按是否处于 Group 选择立即初始化或保存异步初始化任务，并处理设备上下文。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 调用 / 计算 / 写入：NCCLCHECK(ncclGroupEnd());；失败时可能提前return，成功才继续；最外层出口统一执行初始化、P2P 预建链、消息分块、collective 聚合、Proxy 发布和 kernel 发射。
 * 14. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0899
 */
/* 中文逐项-函数 F0899：ncclCommInitAll
 * 用途与位置：单进程本地多 GPU 便捷初始化，使用同一 uniqueId 和 Group 协调各设备加入同一通信组。
 * 参数 comms（ncclComm_t* comms）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ndev（int ndev）：本次单进程InitAll参与的GPU/rank数量；devlist为空时使用设备0到ndev-1。
 * 参数 devlist（const int* devlist）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；PtrCheck；ncclCommInitRankDev；ncclGetUniqueId；ncclGroupEnd；ncclGroupStart。
 * 需要对照的边界/推进条件：ndev < 0；i<ndev。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommInitAll(ncclComm_t* comms, int ndev, const int* devlist) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  NCCLCHECK(PtrCheck(comms, "CommInitAll", "comms"));
  if (ndev < 0) {
    WARN("Invalid device count requested : %d", ndev);
    return ncclInvalidArgument;
  }

/* 中文逐项-变量 V3334：uniqueId
 * uniqueId（ncclUniqueId；函数 ncclCommInitAll 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetUniqueId(&uniqueId));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclUniqueId uniqueId;
  NCCLCHECK(ncclGetUniqueId(&uniqueId));
  NCCLCHECK(ncclGroupStart());
  for (/* 中文逐项-变量 V3335：i
 * i（int；函数 ncclCommInitAll 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ndev；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ndev; i++) {
    // Ignore return codes .. we need to call ncclGroupEnd to clean up anyway
    ncclCommInitRankDev(comms+i, ndev, uniqueId, i, devlist ? devlist[i] : i);
  }
  NCCLCHECK(ncclGroupEnd());
  return ncclSuccess;
}

/* 中文内部逻辑 F0900：ncclGraphHelperDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：auto res = comm->graphHelperResources;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(comm->graphHelperThread && res)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：pthread_mutex_lock(&res->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 调用 / 计算 / 写入：res->threadState = ThreadStop;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：pthread_cond_signal(&res->threadCond);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     7. 调用 / 计算 / 写入：pthread_mutex_unlock(&res->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     8. 调用 / 计算 / 写入：pthread_join(comm->graphHelperThread, NULL);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 条件判断：(res)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   10. 真分支：；仅条件为真执行以下子项。
 *     11. 调用 / 计算 / 写入：free(res);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     12. 调用 / 计算 / 写入：res = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0900
 */
/* 中文逐项-函数 F0900：ncclGraphHelperDestroy
 * 用途与位置：停止并等待图 IPC 清理辅助线程，回收其锁、队列与同步对象。
 * 参数 comm（ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::commDestroy。
 * 本体直接出现的调用/包装：free；pthread_cond_signal；pthread_join；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：comm->graphHelperThread && res；res。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
static ncclResult_t ncclGraphHelperDestroy(ncclComm* comm) {
/* 中文逐项-变量 V3336：res
 * res（auto；函数 ncclGraphHelperDestroy 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = comm->graphHelperResources；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  auto res = comm->graphHelperResources;
  if (comm->graphHelperThread && res) {
    pthread_mutex_lock(&res->threadLock);
    res->threadState = ThreadStop;
    pthread_cond_signal(&res->threadCond);
    pthread_mutex_unlock(&res->threadLock);
    pthread_join(comm->graphHelperThread, NULL);
  }
  if (res) {
    free(res);
    res = NULL;
  }
  return ncclSuccess;
}

/* 中文导读：
 * 销毁 communicator 的完整生命周期入口之一：协调后台服务与图辅助资源，再释放通道及主机/设备状态。
 * 做资源统计时要区分单次操作完成、连接仍驻留和 communicator 最终销毁，三者释放范围不同。
 */
/* 中文内部逻辑 F0901：commDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(comm->rank == -1 || comm->nRanks <= 0 || comm->cudaDev == -1 || comm->busId == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：WARN("comm %p has already been destroyed", comm);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int savedDevice;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：CUDACHECK(cudaGetDevice(&savedDevice));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 7. 准备局部状态：int commDevice = comm->cudaDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 条件判断：(savedDevice != commDevice)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：CUDACHECK(cudaSetDevice(commDevice));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 11. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "Destroying comm %p rank %d abortFlag %d fatalError %d", comm, comm->rank, *comm->abortFlag, comm->fatalError);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 12. 调用 / 计算 / 写入：CUDACHECK(cudaStreamSynchronize(comm->groupStream));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 13. 调用 / 计算 / 写入：ncclDestroyQueueInfo(comm->enqueueInfo);；释放图/普通提交持有的队列，把不能在图析构回调直接关闭的 CUDA IPC 句柄转交辅助线程。
 * 14. 编译期分支/指令：#if CUDART_VERSION >= 11030；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   15. 调用 / 计算 / 写入：NCCLCHECK(ncclGraphHelperDestroy(comm));；失败时可能提前return，成功才继续；停止并等待图 IPC 清理辅助线程，回收其锁、队列与同步对象。
 * 16. 调用 / 计算 / 写入：INFO(NCCL_COLL, "Created %d queue info, destroyed %d", comm->nQueueInfoCreated, comm->nQueueInfoDestroyed);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 17. 调用 / 计算 / 写入：NCCLCHECK(commFree(comm));；失败时可能提前return，成功才继续；回收 communicator 分配的 channel、拓扑、事件、队列、映射等对象，按依赖顺序释放。
 * 18. 条件判断：(savedDevice != commDevice)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   19. 真分支：；仅条件为真执行以下子项。
 *     20. 调用 / 计算 / 写入：CUDACHECK(cudaSetDevice(savedDevice));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 21. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0901
 */
/* 中文逐项-函数 F0901：commDestroy
 * 用途与位置：协调 Proxy 和图辅助资源停止，再进入 communicator 总资源释放。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommDestroy；src/init.cc::ncclCommAbort。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；commFree；cudaGetDevice；cudaSetDevice；cudaStreamSynchronize；ncclDestroyQueueInfo；ncclGraphHelperDestroy。
 * 需要对照的边界/推进条件：comm->rank == -1 || comm->nRanks <= 0 || comm->cudaDev == -1 || comm->busId == -1；savedDevice !=
 * commDevice；savedDevice != commDevice。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t commDestroy(ncclComm_t comm) {
  // Try and prevent a double free of the comm struct (user error)
  if (comm->rank == -1 || comm->nRanks <= 0 || comm->cudaDev == -1 || comm->busId == -1) {
    WARN("comm %p has already been destroyed", comm);
    return ncclInvalidArgument;
  }

/* 中文逐项-变量 V3337：savedDevice
 * savedDevice（int；函数 commDestroy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaGetDevice(&savedDevice));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int savedDevice;
  CUDACHECK(cudaGetDevice(&savedDevice));
/* 中文逐项-变量 V3338：commDevice
 * commDevice（int；函数 commDestroy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 commDevice = comm->cudaDev；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int commDevice = comm->cudaDev;

  if (savedDevice != commDevice) {
    CUDACHECK(cudaSetDevice(commDevice));
  }

  TRACE(NCCL_INIT, "Destroying comm %p rank %d abortFlag %d fatalError %d", comm, comm->rank, *comm->abortFlag, comm->fatalError);

  CUDACHECK(cudaStreamSynchronize(comm->groupStream));

  ncclDestroyQueueInfo(comm->enqueueInfo);
#if CUDART_VERSION >= 11030
  NCCLCHECK(ncclGraphHelperDestroy(comm));
#endif
  INFO(NCCL_COLL, "Created %d queue info, destroyed %d", comm->nQueueInfoCreated, comm->nQueueInfoDestroyed);

  NCCLCHECK(commFree(comm));

  if (savedDevice != commDevice)
    CUDACHECK(cudaSetDevice(savedDevice));

  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclCommDestroy, ncclComm_t comm);
/* 中文内部逻辑 F0902：ncclCommDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 条件判断：(comm == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int rank = comm->rank, nranks = comm->nRanks, cudaDev = comm->cudaDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：int64_t busId = comm->busId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "comm %p rank %d nRanks %d cudaDev %d busId %lx", comm, rank, nranks, cudaDev, busId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(commDestroy(comm));；失败时可能提前return，成功才继续；协调 Proxy 和图辅助资源停止，再进入 communicator 总资源释放。
 * 9. 调用 / 计算 / 写入：INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Destroy COMPLETE", comm, rank, nranks, cudaDev, busId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0902
 */
/* 中文逐项-函数 F0902：ncclCommDestroy
 * 用途与位置：正常销毁公开 communicator 句柄，按本版生命周期要求回收长期通信对象。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/group.cc::ncclGroupEnd。
 * 本体直接出现的调用/包装：NCCLCHECK；commDestroy。
 * 需要对照的边界/推进条件：comm == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommDestroy(ncclComm_t comm) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (comm == NULL)
    return ncclSuccess;

/* 中文逐项-变量 V3339：rank、nranks、cudaDev
 * rank（int；函数 ncclCommDestroy 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = comm->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * nranks（int；函数 ncclCommDestroy 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * cudaDev（int；函数 ncclCommDestroy 内）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 初值/来源 cudaDev = comm->cudaDev；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = comm->rank, nranks = comm->nRanks, cudaDev = comm->cudaDev;
/* 中文逐项-变量 V3340：busId
 * busId（int64_t；函数 ncclCommDestroy 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 初值/来源 busId = comm->busId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int64_t busId = comm->busId;
  TRACE(NCCL_INIT, "comm %p rank %d nRanks %d cudaDev %d busId %lx", comm, rank, nranks, cudaDev, busId);

  NCCLCHECK(commDestroy(comm));
  INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Destroy COMPLETE", comm, rank, nranks, cudaDev, busId);
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclCommAbort, ncclComm_t comm);
/* 中文精读：异常中止与正常销毁的区别
 * abortFlag 是映射给 GPU 的中止信号，设备原语在自旋中周期检查它以便退出异常等待。
 * Proxy/socket 路径也有退出协调；Abort 随后进入资源清理。它不是一次正常的 collective 完成通知。
 * 应用正常路径先确保相关 CUDA 工作生命周期正确，再 Destroy；异步错误可用 GetAsyncError 查询。
 * 本版本不包含后来版本的 InitRankConfig/Split 等 API，不应套用新版对象生命周期。
 */
/* 中文内部逻辑 F0903：ncclCommAbort
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 条件判断：(comm == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int rank = comm->rank, nranks = comm->nRanks, cudaDev = comm->cudaDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：int64_t busId = comm->busId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "comm %p rank %d nRanks %d cudaDev %d busId %lx", comm, rank, nranks, cudaDev, busId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 调用 / 计算 / 写入：*comm->abortFlag = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(commDestroy(comm));；失败时可能提前return，成功才继续；协调 Proxy 和图辅助资源停止，再进入 communicator 总资源释放。
 * 10. 调用 / 计算 / 写入：INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Abort COMPLETE", comm, rank, nranks, cudaDev, busId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0903
 */
/* 中文逐项-函数 F0903：ncclCommAbort
 * 用途与位置：设置共享中止标志并执行异常清理，帮助 GPU/Proxy/socket 等待退出。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；commDestroy。
 * 需要对照的边界/推进条件：comm == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommAbort(ncclComm_t comm) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  if (comm == NULL)
    return ncclSuccess;

/* 中文逐项-变量 V3341：rank、nranks、cudaDev
 * rank（int；函数 ncclCommAbort 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = comm->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * nranks（int；函数 ncclCommAbort 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * cudaDev（int；函数 ncclCommAbort 内）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 初值/来源 cudaDev = comm->cudaDev；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = comm->rank, nranks = comm->nRanks, cudaDev = comm->cudaDev;
/* 中文逐项-变量 V3342：busId
 * busId（int64_t；函数 ncclCommAbort 内）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 * 初值/来源 busId = comm->busId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int64_t busId = comm->busId;
  TRACE(NCCL_INIT, "comm %p rank %d nRanks %d cudaDev %d busId %lx", comm, rank, nranks, cudaDev, busId);

  // Ask anything that might still be running on the device to quit
  *comm->abortFlag = 1;

  NCCLCHECK(commDestroy(comm));
  INFO(NCCL_INIT,"comm %p rank %d nranks %d cudaDev %d busId %lx - Abort COMPLETE", comm, rank, nranks, cudaDev, busId);
  return ncclSuccess;
}

NCCL_API(const char*, ncclGetErrorString, ncclResult_t code);
/* 中文内部逻辑 F0904：ncclGetErrorString
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 多路分派：(code)；按case匹配进入；没有break/return/goto时可能贯穿下一个case。
 *   2. case / default：ncclSuccess；从匹配项进入，随后依源码直到break等跳转。
 *     3. 函数返回：return "no error";；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   4. case / default：ncclUnhandledCudaError；从匹配项进入，随后依源码直到break等跳转。
 *     5. 函数返回：return "unhandled cuda error";；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   6. case / default：ncclSystemError；从匹配项进入，随后依源码直到break等跳转。
 *     7. 函数返回：return "unhandled system error";；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   8. case / default：ncclInternalError；从匹配项进入，随后依源码直到break等跳转。
 *     9. 函数返回：return "internal error";；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   10. case / default：ncclInvalidArgument；从匹配项进入，随后依源码直到break等跳转。
 *     11. 函数返回：return "invalid argument";；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   12. case / default：ncclInvalidUsage；从匹配项进入，随后依源码直到break等跳转。
 *     13. 函数返回：return "invalid usage";；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   14. case / default：default；从匹配项进入，随后依源码直到break等跳转。
 *     15. 函数返回：return "unknown result code";；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0904
 */
/* 中文逐项-函数 F0904：ncclGetErrorString
 * 用途与位置：把 NCCL 结果枚举映射成稳定可读字符串，不改变 communicator 的执行状态。
 * 参数 code（ncclResult_t code）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueHostSetup。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return "no error";；return "unhandled cuda error";；return "unhandled system error";；return "internal
 * error";；return "invalid argument";；其余错误/特化返回见函数体。CHECK类宏还可能提前返回/跳转。
 */
const char* ncclGetErrorString(ncclResult_t code) {
  switch (code) {
    case ncclSuccess                : return "no error";
    case ncclUnhandledCudaError     : return "unhandled cuda error";
    case ncclSystemError            : return "unhandled system error";
    case ncclInternalError          : return "internal error";
    case ncclInvalidArgument        : return "invalid argument";
    case ncclInvalidUsage           : return "invalid usage";
    default                         : return "unknown result code";
  }
}

NCCL_API(ncclResult_t, ncclCommGetAsyncError, ncclComm_t comm, ncclResult_t *asyncError);
/* 中文内部逻辑 F0905：ncclCommGetAsyncError
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(comm, "ncclGetAsyncError", "comm"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(asyncError, "ncclGetAsyncError", "asyncError"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 3. 调用 / 计算 / 写入：*asyncError = comm->fatalError;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0905
 */
/* 中文逐项-函数 F0905：ncclCommGetAsyncError
 * 用途与位置：查询 communicator 已记录的异步失败，使应用可在提交成功后检测后台错误。
 * 参数 comm（ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 asyncError（ncclResult_t *asyncError）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；PtrCheck。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommGetAsyncError(ncclComm_t comm, ncclResult_t *asyncError) {
  NCCLCHECK(PtrCheck(comm, "ncclGetAsyncError", "comm"));
  NCCLCHECK(PtrCheck(asyncError, "ncclGetAsyncError", "asyncError"));
  *asyncError = comm->fatalError;
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclCommCount, const ncclComm_t comm, int* count);
/* 中文内部逻辑 F0906：ncclCommCount
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(comm, "CommCount", "comm"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(count, "CommCount", "count"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 4. 调用 / 计算 / 写入：*count = comm->nRanks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0906
 */
/* 中文逐项-函数 F0906：ncclCommCount
 * 用途与位置：读取通信组 rank 总数到用户输出参数，返回的是逻辑参与规模。
 * 参数 comm（const ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 count（int* count）：当前对象的有效元素数量；复制 API 和 P2P 归一到 Int8 后等于字节数，不能脱离 datatype 解释。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；PtrCheck。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommCount(const ncclComm_t comm, int* count) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  NCCLCHECK(PtrCheck(comm, "CommCount", "comm"));
  NCCLCHECK(PtrCheck(count, "CommCount", "count"));
  *count = comm->nRanks;
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclCommCuDevice, const ncclComm_t comm, int* devid);
/* 中文内部逻辑 F0907：ncclCommCuDevice
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(comm, "CommCuDevice", "comm"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(devid, "CommCuDevice", "devid"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 4. 调用 / 计算 / 写入：*devid = comm->cudaDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0907
 */
/* 中文逐项-函数 F0907：ncclCommCuDevice
 * 用途与位置：查询该 communicator 所属本地 CUDA 设备序号，不能把它等同于全局 rank。
 * 参数 comm（const ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 devid（int* devid）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；PtrCheck。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommCuDevice(const ncclComm_t comm, int* devid) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  NCCLCHECK(PtrCheck(comm, "CommCuDevice", "comm"));
  NCCLCHECK(PtrCheck(devid, "CommCuDevice", "devid"));
  *devid = comm->cudaDev;
  return ncclSuccess;
}

NCCL_API(ncclResult_t, ncclCommUserRank, const ncclComm_t comm, int* rank);
/* 中文内部逻辑 F0908：ncclCommUserRank
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(comm, "CommUserRank", "comm"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(PtrCheck(rank, "CommUserRank", "rank"));；失败时可能提前return，成功才继续；检查必要的指针参数非空，失败时报告具体 API/参数名称。
 * 4. 调用 / 计算 / 写入：*rank = comm->rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__init_cc.md#f0908
 */
/* 中文逐项-函数 F0908：ncclCommUserRank
 * 用途与位置：查询本 communicator 当前对象对应的用户 rank 编号。
 * 参数 comm（const ncclComm_t comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 rank（int* rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；PtrCheck。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclCommUserRank(const ncclComm_t comm, int* rank) {
  NVTX3_FUNC_RANGE_IN(nccl_domain);
  NCCLCHECK(PtrCheck(comm, "CommUserRank", "comm"));
  NCCLCHECK(PtrCheck(rank, "CommUserRank", "rank"));
  *rank = comm->rank;
  return ncclSuccess;
}
