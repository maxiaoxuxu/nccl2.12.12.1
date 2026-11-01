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

/* 中文内部逻辑 F0589：ncclAsyncMode
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclGroupMode > 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0589
 */
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

/* 中文内部逻辑 F0590：ncclAsyncErrCheck
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(ncclGroupError == ncclSuccess || ret != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：ncclGroupError = ret;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return ret;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0590
 */
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
 * ndev（int；成员 ncclInitArgs）：待初始化communicator的总rank数，在异步任务中保存并传给初始化工作函数。
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

/* 中文内部逻辑 F0591：ncclAsyncThreadMain
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECKTHREAD(args->init.func(args->init.newcomm, args->init.ndev, args->init.commId, args->init.myrank, args->init.cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 函数返回：return args;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0591
 */
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

/* 中文内部逻辑 F0592：ncclAsyncInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(ncclGroupIndex >= MAX_ASYNC_OPS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclAsyncErrCheck(ncclInvalidUsage);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：int index = ncclGroupIndex++;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：args->funcType = ASYNC_FUNC_INIT;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：args->init.func = func;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：args->init.cudaDev = cudaDev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：args->init.newcomm = newcomm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：args->init.ndev = ndev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：memcpy(&args->init.commId, &commId, sizeof(commId));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 调用 / 计算 / 写入：args->init.myrank = myrank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0592
 */
/* 中文逐项-函数 F0592：ncclAsyncInit
 * 用途与位置：把初始化函数、rank、设备和 uniqueId 复制到本线程 Group 任务数组。
 * 参数 func（ncclInitFunc_t func）：函数指针或 CUDA kernel 入口地址，由所选算法/函数表/解析器回调决定。
 * 参数 newcomm（ncclComm_t* newcomm）：公开初始化结果句柄的输出地址，只有初始化成功后才能交给后续通信调用。
 * 参数 ndev（int ndev）：待初始化communicator的总rank数，保存到异步初始化任务。
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

/* 中文内部逻辑 F0593：ncclAsyncColl
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 条件判断：(args->coll.comm == comm)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   6. 调用 / 计算 / 写入：args++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 条件判断：(ncclGroupIndex >= MAX_ASYNC_OPS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：WARN("Too many async operations in progress, max is %d", MAX_ASYNC_OPS);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     11. 函数返回：return ncclAsyncErrCheck(ncclInvalidUsage);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 调用 / 计算 / 写入：ncclGroupIndex++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：args->funcType = ASYNC_FUNC_COLL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 调用 / 计算 / 写入：args->coll.comm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0593
 */
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
/* 中文内部逻辑 F0594：ncclGroupStart
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 条件判断：(ncclGroupMode == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：memset(ncclGroupArgs, 0, sizeof(struct ncclAsyncArgs)*MAX_ASYNC_OPS);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 调用 / 计算 / 写入：ncclGroupMode++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0594
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

/* 中文精读：图示绑定 V3.3 F0595 scheduleSend
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0595
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0075] scheduleSend：Group先预建缺失连接，再按一致的peer次序和channel预算拆分任务；不是按每条消息重新建立全部硬件连接（直接对应）
 *   图：25-p2p
 *   定位：#symbol=F0595&binding=B0075
 */
/* 中文内部逻辑 F0595：scheduleSend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclInfo info = { ncclFuncSend, "Send", NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, 1, 1 };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int channelId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncSend, &channelId));；失败时可能提前return，成功才继续；组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。
 * 4. 调用 / 计算 / 写入：info.channelId = channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclSetupP2pKernel(&info));；失败时可能提前return，成功才继续；为 P2P 块生成设备/Proxy 计划并更新发射所需的线程和 channel 范围。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0595
 */
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
/* 中文精读：图示绑定 V3.3 F0596 scheduleRecv
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0596
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0076] scheduleRecv：Group先预建缺失连接，再按一致的peer次序和channel预算拆分任务；不是按每条消息重新建立全部硬件连接（直接对应）
 *   图：25-p2p
 *   定位：#symbol=F0596&binding=B0076
 */
/* 中文内部逻辑 F0596：scheduleRecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclInfo info = { ncclFuncRecv, "Recv", NULL, buff, count, ncclInt8, ncclSum, peer, comm, comm->userStream, 1, 1 };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int channelId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclChannelCompute(comm, peer, chunk%comm->p2pnChannelsPerPeer, ncclFuncRecv, &channelId));；失败时可能提前return，成功才继续；组合基准计算和分散偏移，得到指定 peer/块号/方向的最终 P2P channelId。
 * 4. 调用 / 计算 / 写入：info.channelId = channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclSetupP2pKernel(&info));；失败时可能提前return，成功才继续；为 P2P 块生成设备/Proxy 计划并更新发射所需的线程和 channel 范围。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0596
 */
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

/* 中文内部逻辑 F0597：ncclAsyncThreadPreconnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclAsyncArgs* args = (struct ncclAsyncArgs*)args_;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclComm* comm = args->coll.comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：CUDACHECKTHREAD(cudaSetDevice(comm->cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 条件判断：(CPU_COUNT(&comm->cpuAffinity))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 7. 调用 / 计算 / 写入：NCCLCHECKTHREAD(ncclTransportP2pSetup(comm, NULL, 1));；失败时可能提前return，成功才继续；按需求位批量选择 transport、交换建链句柄并 connect，将完成的连接复制给设备 peer 表。
 * 8. 函数返回：return args;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0597
 */
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
/* 中文精读：图示绑定 V3.3 F0598 getP2pChunkSize
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0598
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0073] getP2pChunkSize：Group先预建缺失连接，再按一致的peer次序和channel预算拆分任务；不是按每条消息重新建立全部硬件连接（直接对应）
 *   图：25-p2p
 *   定位：#symbol=F0598&binding=B0073
 */
/* 中文内部逻辑 F0598：getP2pChunkSize
 * 精读：先按最小通道数估每块尺寸，超过最大块阈值时倍增通道数直到预算上限。
 * 精读：最后按minSize对齐，所以小消息不一定用满per-peer预算；粒度/并发和调度开销在这里折中。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：size_t size = std::max(minSize, DIVUP(totalSize, minChannels));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int nChannels = minChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 ；继续条件 (size > maxSize && nChannels <= maxChannels/2)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 调用 / 计算 / 写入：nChannels *= 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   5. 调用 / 计算 / 写入：size = DIVUP(totalSize, nChannels);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 7. 调用 / 计算 / 写入：ALIGN_SIZE(size, minSize);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 函数返回：return size;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0598
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
/* 中文精读：图示绑定 V3.3 F0599 ncclGroupEnd
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F0599
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0074] ncclGroupEnd：Group先预建缺失连接，再按一致的peer次序和channel预算拆分任务；不是按每条消息重新建立全部硬件连接（直接对应）
 *   图：25-p2p
 *   定位：#symbol=F0599&binding=B0074
 */
/* 中文内部逻辑 F0599：ncclGroupEnd
 * 精读：只有最外层GroupEnd真正提交；先汇总前序错误，再执行初始化线程与P2P预建链。
 * 精读：按双方一致的节点距离和局部rank步号取peer消息，再切块调度，0字节同步与无消息分开。
 * 精读：准备所有comm后分轮执行HostSetup/发射屏障、kernel发射、事件和reset；避免本地部分GPU永远等尚未启动的伙伴。
 * 精读：错误清理标签回收本批状态，不能把goto后的代码当正常顺序继续执行。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NVTX3_FUNC_RANGE_IN(nccl_domain);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 条件判断：(ncclGroupMode == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("ncclGroupEnd: not in a group call.");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 函数返回：return ncclInvalidUsage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：ncclGroupMode--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 条件判断：(ncclGroupMode > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 准备局部状态：int savedDev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 调用 / 计算 / 写入：CUDACHECK(cudaGetDevice(&savedDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 12. 准备局部状态：int activeThreads = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：int doneArray[MAX_ASYNC_OPS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   15. 调用 / 计算 / 写入：doneArray[i] = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 17. 准备局部状态：ncclResult_t ret = ncclGroupError;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 18. 准备局部状态：int usingCudaGraphAll = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 19. 准备局部状态：cudaGraph_t* graphs = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 20. 条件判断：(ret != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   21. 真分支：；仅条件为真执行以下子项。
 *     22. 控制转移：goto group_cleanup;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 23. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   24. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   25. 条件判断：(args->funcType == ASYNC_FUNC_INIT)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     26. 真分支：；仅条件为真执行以下子项。
 *       27. 调用 / 计算 / 写入：pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadMain, args);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       28. 调用 / 计算 / 写入：activeThreads++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       29. 调用 / 计算 / 写入：doneArray[i] = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 31. 循环：初始 ；继续条件 (activeThreads)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   32. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     33. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     34. 条件判断：(args->funcType == ASYNC_FUNC_INIT && doneArray[i] == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       35. 真分支：；仅条件为真执行以下子项。
 *         36. 准备局部状态：int err = pthread_tryjoin_np(ncclGroupThreads[i], NULL);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         37. 条件判断：(err == EBUSY)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           38. 真分支：；仅条件为真执行以下子项。
 *             39. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *         40. 条件判断：(err != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           41. 真分支：；仅条件为真执行以下子项。
 *             42. 调用 / 计算 / 写入：ret = ncclSystemError;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         43. 条件判断：(args->ret != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           44. 真分支：；仅条件为真执行以下子项。
 *             45. 调用 / 计算 / 写入：ret = args->ret;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         46. 调用 / 计算 / 写入：doneArray[i] = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         47. 调用 / 计算 / 写入：activeThreads--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   48. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 49. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 50. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   51. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   52. 条件判断：(args->funcType == ASYNC_FUNC_COLL && args->coll.comm->connect)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     53. 真分支：；仅条件为真执行以下子项。
 *       54. 调用 / 计算 / 写入：pthread_create(ncclGroupThreads+i, NULL, ncclAsyncThreadPreconnect, args);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 55. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 56. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   57. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   58. 条件判断：(args->funcType == ASYNC_FUNC_COLL && args->coll.comm->connect)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     59. 真分支：；仅条件为真执行以下子项。
 *       60. 准备局部状态：int err = pthread_join(ncclGroupThreads[i], NULL);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       61. 条件判断：(err != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         62. 真分支：；仅条件为真执行以下子项。
 *           63. 调用 / 计算 / 写入：WARN("Error waiting for pthread_join : %s", strerror(errno));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           64. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *       65. 调用 / 计算 / 写入：NCCLCHECKGOTO(args->ret, ret, end);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 *       66. 调用 / 计算 / 写入：args->coll.comm->connect = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 67. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 68. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   69. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   70. 条件判断：(args->funcType == ASYNC_FUNC_COLL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     71. 真分支：；仅条件为真执行以下子项。
 *       72. 准备局部状态：struct ncclComm* comm = args->coll.comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       73. 准备局部状态：int node = comm->node;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       74. 准备局部状态：int nNodes = comm->nNodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       75. 准备局部状态：int localRank = comm->localRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       76. 准备局部状态：ssize_t stepSize = comm->buffSizes[NCCL_PROTO_SIMPLE] / NCCL_STEPS;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       77. 准备局部状态：int nChannelsMax = comm->p2pnChannelsPerPeer;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       78. 准备局部状态：int nChannelsMin = nChannelsMax;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       79. 循环：初始 ；继续条件 (nChannelsMin*comm->nRanks > comm->p2pnChannels && nChannelsMin > 1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         80. 调用 / 计算 / 写入：nChannelsMin /= 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       81. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       82. 循环：初始 ；继续条件 (nChannelsMax*comm->nRanks > comm->p2pnChannels*4 && nChannelsMax > 1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         83. 调用 / 计算 / 写入：nChannelsMax /= 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       84. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       85. 循环：初始 ；继续条件 (comm->p2pSendCount > 0 || comm->p2pRecvCount > 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         86. 循环：初始 int d=0;；继续条件 d<=nNodes/4；每轮更新 d++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           87. 准备局部状态：int deltas[4] = { d, (nNodes-d)%nNodes, nNodes/2-d, (nNodes-(nNodes/2-d))%nNodes };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           88. 准备局部状态：int index = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           89. 准备局部状态：int delta = deltas[index];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           90. 跳转标签：sched_delta；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *             91. 准备局部状态：uint32_t recvNode = (node+nNodes-delta)%nNodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           92. 准备局部状态：uint32_t sendNode = (node+delta)%nNodes;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           93. 准备局部状态：int steps = comm->maxLocalRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           94. 循环：初始 int s=0;；继续条件 s<steps；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *             95. 准备局部状态：int recvIndex = (localRank-s+steps)%steps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             96. 准备局部状态：int recvPeer = recvIndex<comm->nodeRanks[recvNode].localRanks ? comm->nodeRanks[recvNode].localRankToRank[recvIndex] : -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             97. 准备局部状态：int sendIndex = (localRank+s)%steps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             98. 准备局部状态：int sendPeer = sendIndex<comm->nodeRanks[sendNode].localRanks ? comm->nodeRanks[sendNode].localRankToRank[sendIndex] : -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             99. 准备局部状态：struct ncclP2Pinfo* recv = recvPeer != -1 && comm->p2pRecvs[recvPeer] ? comm->p2pRecvs[recvPeer]->getNext() : NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             100. 准备局部状态：struct ncclP2Pinfo* send = sendPeer != -1 && comm->p2pSends[sendPeer] ? comm->p2pSends[sendPeer]->getNext() : NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             101. 条件判断：(recv != NULL || send != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               102. 真分支：；仅条件为真执行以下子项。
 *                 103. 准备局部状态：ssize_t totRecvBytes = -1, totSendBytes = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 104. 条件判断：(recv != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   105. 真分支：；仅条件为真执行以下子项。
 *                     106. 调用 / 计算 / 写入：totRecvBytes = recv->nbytes;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 107. 条件判断：(send != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   108. 真分支：；仅条件为真执行以下子项。
 *                     109. 调用 / 计算 / 写入：totSendBytes = send->nbytes;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 110. 条件判断：(recv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   111. 真分支：；仅条件为真执行以下子项。
 *                     112. 调用 / 计算 / 写入：comm->p2pRecvCount--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 113. 条件判断：(send)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   114. 真分支：；仅条件为真执行以下子项。
 *                     115. 调用 / 计算 / 写入：comm->p2pSendCount--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 116. 条件判断：(recvPeer == comm->rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   117. 真分支：；仅条件为真执行以下子项。
 *                     118. 条件判断：(sendPeer != comm->rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                       119. 真分支：；仅条件为真执行以下子项。
 *                         120. 调用 / 计算 / 写入：WARN("Sendrecv schedule not aligned for self");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                         121. 调用 / 计算 / 写入：ret = ncclInternalError;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                         122. 控制转移：goto group_cleanup;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *                     123. 条件判断：(send && recv == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                       124. 真分支：；仅条件为真执行以下子项。
 *                         125. 调用 / 计算 / 写入：WARN("Trying to send to self without a matching recv");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                         126. 调用 / 计算 / 写入：ret = ncclInvalidUsage;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                         127. 控制转移：goto group_cleanup;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *                     128. 条件判断：(send == NULL && recv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                       129. 真分支：；仅条件为真执行以下子项。
 *                         130. 调用 / 计算 / 写入：WARN("Trying to recv to self without a matching send");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                         131. 调用 / 计算 / 写入：ret = ncclInvalidUsage;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                         132. 控制转移：goto group_cleanup;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *                 133. 准备局部状态：void* recvBuff = recv ? recv->buff : NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 134. 准备局部状态：void* sendBuff = send ? send->buff : NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 135. 条件判断：(recv && comm->p2pRecvs[recvPeer]->peakNext() == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   136. 真分支：；仅条件为真执行以下子项。
 *                     137. 调用 / 计算 / 写入：comm->p2pRecvs[recvPeer]->recycle();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 138. 条件判断：(send && comm->p2pSends[sendPeer]->peakNext() == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   139. 真分支：；仅条件为真执行以下子项。
 *                     140. 调用 / 计算 / 写入：comm->p2pSends[sendPeer]->recycle();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 141. 准备局部状态：ssize_t recvChunkSize = getP2pChunkSize(totRecvBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);；在最小/最大通道预算内倍增并行度，将每 peer 消息划成按 step 对齐的块。
 *                 142. 准备局部状态：ssize_t sendChunkSize = getP2pChunkSize(totSendBytes, nChannelsMin, nChannelsMax, stepSize, SENDRECV_SLICEFACTOR*stepSize);；在最小/最大通道预算内倍增并行度，将每 peer 消息划成按 step 对齐的块。
 *                 143. 准备局部状态：ssize_t sendOffset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 144. 准备局部状态：ssize_t recvOffset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 145. 准备局部状态：int sendRemaining = 1, recvRemaining = 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 146. 准备局部状态：int chunk = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                 147. 先执行一次的循环：初始 ；继续条件 (sendRemaining || recvRemaining)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *                   148. 准备局部状态：ssize_t recvbytes = totRecvBytes-recvOffset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                   149. 准备局部状态：ssize_t sendbytes = totSendBytes-sendOffset;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *                   150. 条件判断：(recvbytes > recvChunkSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     151. 真分支：；仅条件为真执行以下子项。
 *                       152. 调用 / 计算 / 写入：recvbytes = recvChunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                     153. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                       154. 调用 / 计算 / 写入：recvRemaining = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   155. 条件判断：(sendbytes > sendChunkSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     156. 真分支：；仅条件为真执行以下子项。
 *                       157. 调用 / 计算 / 写入：sendbytes = sendChunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                     158. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                       159. 调用 / 计算 / 写入：sendRemaining = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   160. 条件判断：(sendbytes < 0 || (sendbytes == 0 && totSendBytes != 0))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     161. 真分支：；仅条件为真执行以下子项。
 *                       162. 调用 / 计算 / 写入：send = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   163. 条件判断：(recvbytes < 0 || (recvbytes == 0 && totRecvBytes != 0))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     164. 真分支：；仅条件为真执行以下子项。
 *                       165. 调用 / 计算 / 写入：recv = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   166. 条件判断：(recv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     167. 真分支：；仅条件为真执行以下子项。
 *                       168. 调用 / 计算 / 写入：NCCLCHECKGOTO(scheduleRecv(comm, recvPeer, chunk, recvbytes, ((char*)recvBuff)+recvOffset), ret, group_cleanup);；失败时保存错误并跳到清理标签，成功才继续；把 Group 切出的字节块封装为 P2P 任务，按 peer/块号映射 channel 后调用 SetupP2pKernel。
 *                   169. 条件判断：(send)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     170. 真分支：；仅条件为真执行以下子项。
 *                       171. 调用 / 计算 / 写入：NCCLCHECKGOTO(scheduleSend(comm, sendPeer, chunk, sendbytes, ((char*)sendBuff)+sendOffset), ret, group_cleanup);；失败时保存错误并跳到清理标签，成功才继续；把 Group 切出的字节块封装为 P2P 任务，按 peer/块号映射 channel 后调用 SetupP2pKernel。
 *                   172. 调用 / 计算 / 写入：recvOffset += recvChunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   173. 调用 / 计算 / 写入：sendOffset += sendChunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   174. 调用 / 计算 / 写入：chunk++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 175. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           176. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *           177. 调用 / 计算 / 写入：index++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           178. 条件判断：(index == 1 && deltas[1] == deltas[0])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             179. 真分支：；仅条件为真执行以下子项。
 *               180. 调用 / 计算 / 写入：index++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           181. 条件判断：(index == 2 && deltas[2] == deltas[0])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             182. 真分支：；仅条件为真执行以下子项。
 *               183. 调用 / 计算 / 写入：index++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           184. 条件判断：(index == 3 && deltas[3] == deltas[2])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             185. 真分支：；仅条件为真执行以下子项。
 *               186. 调用 / 计算 / 写入：index++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           187. 条件判断：(index == 3 && deltas[3] == deltas[1])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             188. 真分支：；仅条件为真执行以下子项。
 *               189. 调用 / 计算 / 写入：index++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           190. 条件判断：(index < 4)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             191. 真分支：；仅条件为真执行以下子项。
 *               192. 调用 / 计算 / 写入：delta = deltas[index];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               193. 控制转移：goto sched_delta;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *         194. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       195. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 196. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 197. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&graphs, ncclGroupIndex));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 198. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   199. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   200. 条件判断：(args->funcType == ASYNC_FUNC_COLL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     201. 真分支：；仅条件为真执行以下子项。
 *       202. 准备局部状态：ncclComm_t comm = args->coll.comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       203. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclGetCudaGraph(comm, graphs+i), ret, group_cleanup);；失败时保存错误并跳到清理标签，成功才继续；检查当前 stream 是否捕获、图标识和驱动能力，维护图模式队列及生命周期状态。
 *       204. 条件判断：(usingCudaGraphAll == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         205. 真分支：；仅条件为真执行以下子项。
 *           206. 调用 / 计算 / 写入：usingCudaGraphAll = comm->usingCudaGraph;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         207. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           208. 条件判断：(usingCudaGraphAll != comm->usingCudaGraph)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             209. 真分支：；仅条件为真执行以下子项。
 *               210. 调用 / 计算 / 写入：WARN("Illegal to have some communicators in graph mode while others not");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *               211. 调用 / 计算 / 写入：ret = ncclInvalidUsage;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               212. 控制转移：goto group_cleanup;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 213. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 214. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   215. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   216. 条件判断：(args->funcType == ASYNC_FUNC_COLL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     217. 真分支：；仅条件为真执行以下子项。
 *       218. 准备局部状态：ncclComm_t comm = args->coll.comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       219. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSetupAsyncKernels(comm), ret, group_cleanup);；失败时保存错误并跳到清理标签，成功才继续；把 Group 保存的一组 collective 转成发射任务，对同构批次可统一选型并聚合工作元素。
 * 220. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 221. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   222. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   223. 条件判断：(args->funcType == ASYNC_FUNC_COLL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     224. 真分支：；仅条件为真执行以下子项。
 *       225. 条件判断：(args->coll.comm->userStream == cudaStreamDefault || args->coll.comm->userStream == cudaStreamPerThread || args->coll.comm->userStream == cudaStreamLegacy)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         226. 真分支：；仅条件为真执行以下子项。
 *           227. 调用 / 计算 / 写入：CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 *       228. 条件判断：(usingCudaGraphAll == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         229. 真分支：；仅条件为真执行以下子项。
 *           230. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclCudaGraphHostSetup(args->coll.comm, graphs[i]), ret, end);；失败时保存错误并跳到清理标签，成功才继续；给 CUDA Graph 添加/关联 HostSetup 节点及持有对象，确保 replay 时重新发布 GPU/Proxy 工作。
 *         231. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           232. 调用 / 计算 / 写入：ncclEnqueueHostSetup<0>(args->coll.comm->enqueueInfo);；遍历提交批次并执行真正的 FIFO 装填、launch 整理和 Proxy 发布；图 replay 也复用此入口。
 *       233. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclLaunchBarrier(args->coll.comm), ret, end);；失败时保存错误并跳到清理标签，成功才继续；建立用户 stream 与内部 stream 依赖，并在 GROUP 发射模式协调同进程 GPU 的启动。
 * 234. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 235. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   236. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   237. 条件判断：(args->funcType == ASYNC_FUNC_COLL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     238. 真分支：；仅条件为真执行以下子项。
 *       239. 调用 / 计算 / 写入：CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 *       240. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclLaunchKernel(args->coll.comm), ret, end);；失败时保存错误并跳到清理标签，成功才继续；PARALLEL 模式真正调用 CUDA 发射，GROUP 模式等待已由最后线程执行的统一发射完成。
 * 241. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 242. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   243. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   244. 条件判断：(args->funcType == ASYNC_FUNC_COLL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     245. 真分支：；仅条件为真执行以下子项。
 *       246. 条件判断：(args->coll.comm->userStream == cudaStreamDefault || args->coll.comm->userStream == cudaStreamPerThread || args->coll.comm->userStream == cudaStreamLegacy)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         247. 真分支：；仅条件为真执行以下子项。
 *           248. 调用 / 计算 / 写入：CUDACHECKGOTO(cudaSetDevice(args->coll.comm->cudaDev), ret, end);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 *       249. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclRecordEvents(args->coll.comm), ret, end);；失败时保存错误并跳到清理标签，成功才继续；在执行 stream 记录完成事件并建立必要的用户 stream 依赖；record 本身不代表事件已经完成。
 *       250. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclLaunchReset(args->coll.comm), ret, end);；失败时保存错误并跳到清理标签，成功才继续；清理本批发射参数和入队计数，为后续提交复用；图路径按持久 QueueInfo 生命周期处理。
 * 251. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 252. 控制转移：goto end;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 253. 跳转标签：group_cleanup；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   254. 条件判断：(ret != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     255. 真分支：；仅条件为真执行以下子项。
 *       256. 循环：初始 int i=0;；继续条件 i<ncclGroupIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         257. 准备局部状态：struct ncclAsyncArgs* args = ncclGroupArgs+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         258. 条件判断：(args->funcType == ASYNC_FUNC_INIT)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           259. 真分支：；仅条件为真执行以下子项。
 *             260. 条件判断：(args->init.newcomm)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               261. 真分支：；仅条件为真执行以下子项。
 *                 262. 调用 / 计算 / 写入：ncclCommDestroy(*args->init.newcomm);；正常销毁公开 communicator 句柄，按本版生命周期要求回收长期通信对象。
 *             263. 调用 / 计算 / 写入：*args->init.newcomm = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           264. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             265. 准备局部状态：struct ncclComm* comm = args->coll.comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             266. 调用 / 计算 / 写入：comm->asyncOpCount = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             267. 调用 / 计算 / 写入：comm->asyncTotalSize = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             268. 条件判断：(comm->p2pSendCount > 0 || comm->p2pRecvCount > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               269. 真分支：；仅条件为真执行以下子项。
 *                 270. 循环：初始 int peer=0;；继续条件 peer<comm->nRanks；每轮更新 peer++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *                   271. 条件判断：(comm->p2pSends[peer])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     272. 真分支：；仅条件为真执行以下子项。
 *                       273. 调用 / 计算 / 写入：comm->p2pSends[peer]->recycle();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                   274. 条件判断：(comm->p2pRecvs[peer])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     275. 真分支：；仅条件为真执行以下子项。
 *                       276. 调用 / 计算 / 写入：comm->p2pRecvs[peer]->recycle();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 277. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *                 278. 调用 / 计算 / 写入：comm->p2pSendCount = comm->p2pRecvCount = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             279. 调用 / 计算 / 写入：ncclLaunchReset(comm);；清理本批发射参数和入队计数，为后续提交复用；图路径按持久 QueueInfo 生命周期处理。
 *       280. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 281. 跳转标签：end；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   282. 调用 / 计算 / 写入：ncclGroupError = ncclSuccess;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 283. 调用 / 计算 / 写入：ncclGroupIndex = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 284. 调用 / 计算 / 写入：CUDACHECK(cudaSetDevice(savedDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 285. 条件判断：(graphs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   286. 真分支：；仅条件为真执行以下子项。
 *     287. 调用 / 计算 / 写入：free(graphs);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 288. 函数返回：return ret;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__group_cc.md#f0599
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
