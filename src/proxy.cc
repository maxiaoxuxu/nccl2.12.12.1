/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "info.h"
#include "collectives.h"
#include "socket.h"
#include "shm.h"
#include "profiler.h"
/* 中文逐项-常量 X1235：ENABLE_TIMER
 * 编译期定义：ENABLE_TIMER = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ENABLE_TIMER 0
#include "timer.h"

/* 中文逐项-类型 T0400：<匿名enum>
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum { /* 中文逐项-枚举项 X1236：proxyRecv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */proxyRecv=0, /* 中文逐项-枚举项 X1237：proxySend
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */proxySend=1 };

/* 中文内部逻辑 F1014：NeedProxy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(pattern == ncclPatternRing || pattern == ncclPatternRingTwice)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return true;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：const int myrank = 0, nextrank = 1, prevrank = nranks-1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：int index = pattern == ncclPatternPipelineFrom ? (type == proxyRecv ? myrank : nextrank ): (type == proxyRecv ? prevrank : myrank );；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：int rank = ring->userRanks[index];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 函数返回：return (root != rank);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1014
 */
/* 中文逐项-函数 F1014：NeedProxy
 * 用途与位置：按通信模式、root 与环邻居判断此方向是否真的需要代理，root 流水线端点可省掉无效边。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 pattern（int pattern）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 * 参数 root（int root）：集合操作的根用户 rank；在 P2P 的 ncclInfo 中复用该字段表示 peer。
 * 参数 ring（struct ncclRing* ring）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxySaveColl。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：pattern == ncclPatternRing || pattern == ncclPatternRingTwice。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return true;；return (root != rank);。CHECK类宏还可能提前返回/跳转。
 */
static bool NeedProxy(int type, int pattern, int root, struct ncclRing* ring, int nranks) {
  if (pattern == ncclPatternRing || pattern == ncclPatternRingTwice) return true;

  /* In chains, one rank does not need a proxy. Let's figure out which one it is */
  /* Which index in the reorganized rings should we compare root against */
/* 中文逐项-变量 V3594：myrank、nextrank、prevrank
 * myrank（int；函数 NeedProxy 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 myrank = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * nextrank（int；函数 NeedProxy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nextrank = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * prevrank（int；函数 NeedProxy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 prevrank = nranks-1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  const int myrank = 0, nextrank = 1, prevrank = nranks-1;
/* 中文逐项-变量 V3595：index
 * index（int；函数 NeedProxy 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 初值/来源 index = pattern == ncclPatternPipelineFrom ? / * no recv / no send if root = * / / * bcast * / (type ==
 * proxyRecv ? myrank : nextrank ): / * reduce * / (type == proxyRecv ? prevrank : myrank
 * )；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int index = pattern == ncclPatternPipelineFrom ?
      /*                            no recv /  no send    if root = */
      /* bcast  */ (type == proxyRecv ?   myrank : nextrank ):
      /* reduce */ (type == proxyRecv ? prevrank :   myrank );
/* 中文逐项-变量 V3596：rank
 * rank（int；函数 NeedProxy 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = ring->userRanks[index]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = ring->userRanks[index];
  return (root != rank);
}

/* 中文逐项-常量 X1238：PROXYARGS_ALLOCATE_SIZE
 * 编译期定义：PROXYARGS_ALLOCATE_SIZE = NCCL_MAX_OPS。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define PROXYARGS_ALLOCATE_SIZE NCCL_MAX_OPS
/* 中文逐项-类型 T0401：ncclProxyPool
 * 类型职责：共享提交池、本地提交链视图和可复用运行参数池，降低每操作分配/进程通信开销。
 * 声明内可见成员（含嵌套成员）：next、elems。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyPool {
/* 中文逐项-变量 V3597：next
 * next（struct ncclProxyPool；成员 ncclProxyPool）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
  struct ncclProxyPool *next;
/* 中文逐项-变量 V3598：elems
 * elems（struct ncclProxyArgs；成员 ncclProxyPool）：固定大小 work union 的不同元素布局，必须先看 header.type 再解释内存。
 */
  struct ncclProxyArgs elems[PROXYARGS_ALLOCATE_SIZE];
};

/* 中文内部逻辑 F1015：allocateArgs
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyArgs* elem;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(state->pool == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 准备局部状态：struct ncclProxyPool* newPool;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&newPool, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     6. 准备局部状态：struct ncclProxyArgs* newElems = newPool->elems;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     7. 循环：初始 int i=0;；继续条件 i<PROXYARGS_ALLOCATE_SIZE；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       8. 条件判断：(i+1 < PROXYARGS_ALLOCATE_SIZE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         9. 真分支：；仅条件为真执行以下子项。
 *           10. 调用 / 计算 / 写入：newElems[i].next = newElems+i+1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     12. 调用 / 计算 / 写入：state->pool = newElems;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 调用 / 计算 / 写入：newPool->next = state->pools;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     14. 调用 / 计算 / 写入：state->pools = newPool;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：elem = state->pool;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 调用 / 计算 / 写入：state->pool = state->pool->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：elem->next = elem->nextPeer = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 调用 / 计算 / 写入：*argsptr = elem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1015
 */
/* 中文逐项-函数 F1015：allocateArgs
 * 用途与位置：从可复用参数池分配运行态 ProxyArgs，容量不足时扩展池块而不逐请求零散分配。
 * 参数 state（struct ncclProxyProgressState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 参数 argsptr（struct ncclProxyArgs** argsptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ProxyAppend。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc。
 * 需要对照的边界/推进条件：state->pool == NULL；i<PROXYARGS_ALLOCATE_SIZE；i+1 < PROXYARGS_ALLOCATE_SIZE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t allocateArgs(struct ncclProxyProgressState* state, struct ncclProxyArgs** argsptr) {
/* 中文逐项-变量 V3599：elem
 * elem（struct ncclProxyArgs；函数 allocateArgs 内）：注册work扩展或队列容器内嵌的普通工作元素，扩展信息在它后面保存。
 * 此处未显式给初值；后续首个文本使用点：elem = state->pool;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyArgs* elem;
  if (state->pool == NULL) {
    // Allocate a new pool of elements. Make sure we allocate the memory close
    // to the network thread
/* 中文逐项-变量 V3600：newPool
 * newPool（struct ncclProxyPool；函数 allocateArgs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&newPool, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclProxyPool* newPool;
    NCCLCHECK(ncclCalloc(&newPool, 1));

/* 中文逐项-变量 V3601：newElems
 * newElems（struct ncclProxyArgs；函数 allocateArgs 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 newElems = newPool->elems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyArgs* newElems = newPool->elems;
    // Chain newly allocated elements
    for (/* 中文逐项-变量 V3602：i
 * i（int；函数 allocateArgs 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<PROXYARGS_ALLOCATE_SIZE；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<PROXYARGS_ALLOCATE_SIZE; i++) {
      if (i+1 < PROXYARGS_ALLOCATE_SIZE) newElems[i].next = newElems+i+1;
    }
    // Add them all to the pool list
    state->pool = newElems;
    // Save the pool memory block for later resource release
    newPool->next = state->pools;
    state->pools = newPool;
  }
  elem = state->pool;
  state->pool = state->pool->next;
  elem->next = elem->nextPeer = NULL;
  *argsptr = elem;
  return ncclSuccess;
}

//#define DEBUG_PROXY 1
#ifdef DEBUG_PROXY
/* 中文逐项-常量 X1239：DEBUG_PROXY_PRINT
 * 编译期定义：DEBUG_PROXY_PRINT = printf。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define DEBUG_PROXY_PRINT printf
#else
/* 中文逐项-宏 M0147：DEBUG_PROXY_PRINT
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 DEBUG_PROXY_PRINT；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define DEBUG_PROXY_PRINT(...)
#endif

/* 中文逐项-宏 M0148：OP_INDEX
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 OP_INDEX；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define OP_INDEX(op) ((op) ? (op)-state->pools->elems : -1)
/* 中文逐项-常量 X1240：OP_SEEN
 * 编译期定义：OP_SEEN = 0x100000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define OP_SEEN 0x100000

/* 中文内部逻辑 F1016：getOpIndex
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyPool* pool = state->pools;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int p = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 ；继续条件 (pool)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 准备局部状态：uint64_t o = op-pool->elems;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   5. 条件判断：(o < PROXYARGS_ALLOCATE_SIZE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     6. 真分支：；仅条件为真执行以下子项。
 *       7. 调用 / 计算 / 写入：*opIndex = o;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       8. 调用 / 计算 / 写入：*poolIndex = p;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   10. 调用 / 计算 / 写入：pool = pool->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   11. 调用 / 计算 / 写入：p++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 调用 / 计算 / 写入：WARN("Could not find pool of op %p\n", op);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 14. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1016
 */
/* 中文逐项-函数 F1016：getOpIndex
 * 用途与位置：计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。
 * 参数 op（struct ncclProxyArgs* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 state（struct ncclProxyProgressState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 参数 poolIndex（int* poolIndex）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 opIndex（int* opIndex）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::dumpProxyState。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：pool；o < PROXYARGS_ALLOCATE_SIZE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t getOpIndex(struct ncclProxyArgs* op, struct ncclProxyProgressState* state, int* poolIndex, int* opIndex) {
/* 中文逐项-变量 V3603：pool
 * pool（struct ncclProxyPool；函数 getOpIndex 内）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 初值/来源 pool = state->pools；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyPool* pool = state->pools;
/* 中文逐项-变量 V3604：p
 * p（int；函数 getOpIndex 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int p = 0;
  while (pool) {
/* 中文逐项-变量 V3605：o
 * o（uint64_t；函数 getOpIndex 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 o = op-pool->elems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    uint64_t o = op-pool->elems;
    if (o < PROXYARGS_ALLOCATE_SIZE) {
      *opIndex = o;
      *poolIndex = p;
      return ncclSuccess;
    }
    pool = pool->next;
    p++;
  }
  WARN("Could not find pool of op %p\n", op);
  return ncclInternalError;
}

/* 中文内部逻辑 F1017：printProxyOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：printf("[%d-%d|%ld| %s", poolIndex, opIndex, op->opCount, op->pattern == ncclPatternSend ? "Send" : op->pattern == ncclPatternRecv ? "Recv" : "Coll");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 循环：初始 int s=0;；继续条件 s<op->nsubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 准备局部状态：struct ncclProxySubArgs* sub = op->subs+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   4. 条件判断：(op->state == ncclProxyOpProgress)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 准备局部状态：char status = ' ';；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       7. 条件判断：(op->pattern == ncclPatternRecv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         8. 真分支：；仅条件为真执行以下子项。
 *           9. 条件判断：(sub->posted < sub->nsteps && sub->posted < sub->done + NCCL_STEPS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             10. 真分支：；仅条件为真执行以下子项。
 *               11. 调用 / 计算 / 写入：status = 'I';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             12. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *               13. 条件判断：(sub->received < sub->posted)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 14. 真分支：；仅条件为真执行以下子项。
 *                   15. 调用 / 计算 / 写入：status = 'R';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 16. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   17. 条件判断：(sub->received < sub->transmitted)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     18. 真分支：；仅条件为真执行以下子项。
 *                       19. 调用 / 计算 / 写入：status = 'R';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                     20. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                       21. 条件判断：(sub->transmitted < sub->received)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                         22. 真分支：；仅条件为真执行以下子项。
 *                           23. 调用 / 计算 / 写入：status = 'F';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                         24. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                           25. 条件判断：(sub->done < sub->transmitted)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                             26. 真分支：；仅条件为真执行以下子项。
 *                               27. 调用 / 计算 / 写入：status = 'G';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                             28. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                               29. 调用 / 计算 / 写入：status = 'D';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         30. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           31. 条件判断：(op->pattern == ncclPatternSend)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             32. 真分支：；仅条件为真执行以下子项。
 *               33. 条件判断：(sub->posted < sub->nsteps && sub->posted < sub->done + NCCL_STEPS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                 34. 真分支：；仅条件为真执行以下子项。
 *                   35. 调用 / 计算 / 写入：status = 'I';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                 36. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                   37. 条件判断：(sub->transmitted < sub->posted)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                     38. 真分支：；仅条件为真执行以下子项。
 *                       39. 调用 / 计算 / 写入：status = 'G';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                     40. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                       41. 条件判断：(sub->done < sub->transmitted)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                         42. 真分支：；仅条件为真执行以下子项。
 *                           43. 调用 / 计算 / 写入：status = 'S';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                         44. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                           45. 调用 / 计算 / 写入：status = 'D';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       46. 调用 / 计算 / 写入：printf(" %d%c/%d", sub->peer, status, sub->channelId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     47. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       48. 调用 / 计算 / 写入：printf(" %d/%d", sub->peer, sub->channelId);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 49. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 50. 调用 / 计算 / 写入：printf("]");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 51. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1017
 */
/* 中文逐项-函数 F1017：printProxyOp
 * 用途与位置：打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 * 参数 op（struct ncclProxyArgs* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 poolIndex（int poolIndex）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 opIndex（int opIndex）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::dumpProxyState。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：s<op->nsubs；op->state == ncclProxyOpProgress；op->pattern == ncclPatternRecv；sub->posted < sub->nsteps
 * && sub->posted < sub->done + NCCL_STEPS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t printProxyOp(struct ncclProxyArgs* op, int poolIndex, int opIndex) {
  printf("[%d-%d|%ld| %s", poolIndex, opIndex, op->opCount, op->pattern == ncclPatternSend ? "Send" : op->pattern == ncclPatternRecv ? "Recv" : "Coll");
  for (/* 中文逐项-变量 V3606：s
 * s（int；函数 printProxyOp 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<op->nsubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<op->nsubs; s++) {
/* 中文逐项-变量 V3607：sub
 * sub（struct ncclProxySubArgs；函数 printProxyOp 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = op->subs+s；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxySubArgs* sub = op->subs+s;
    if (op->state == ncclProxyOpProgress) {
/* 中文逐项-变量 V3608：status
 * status（char；函数 printProxyOp 内）：完成/调用的通用状态和厂商错误细节，非成功不能把目标数据当作有效结果。
 * 初值/来源 status = ' '；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      char status = ' ';
      if (op->pattern == ncclPatternRecv) {
        if (sub->posted < sub->nsteps && sub->posted < sub->done + NCCL_STEPS) status = 'I'; // Init
        else if (sub->received < sub->posted) status = 'R'; // Receiving
        else if (sub->received < sub->transmitted) status = 'R'; // Receiving
        else if (sub->transmitted < sub->received) status = 'F'; // Flushing
        else if (sub->done < sub->transmitted) status = 'G'; // Waiting on GPU
        else status = 'D'; // Done
      } else if (op->pattern == ncclPatternSend) {
        if (sub->posted < sub->nsteps && sub->posted < sub->done + NCCL_STEPS) status = 'I'; // Init
        else if (sub->transmitted < sub->posted) status = 'G'; // Waiting on GPU
        else if (sub->done < sub->transmitted) status = 'S'; // Sending
        else status = 'D'; // Done
      }
      printf(" %d%c/%d", sub->peer, status, sub->channelId);
    } else {
      printf(" %d/%d", sub->peer, sub->channelId);
    }
  }
  printf("]");
  return ncclSuccess;
}
/* 中文内部逻辑 F1018：dumpProxyState
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyArgs* op = state->active;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int poolIndex, opIndex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：printf("ACTIVE OPS\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 循环：初始 ；继续条件 (op)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   5. 调用 / 计算 / 写入：NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));；失败时可能提前return，成功才继续；计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。
 *   6. 条件判断：(op->state & OP_SEEN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     7. 真分支：；仅条件为真执行以下子项。
 *       8. 调用 / 计算 / 写入：WARN("List loop at element %d-%d", poolIndex, opIndex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   9. 调用 / 计算 / 写入：NCCLCHECK(printProxyOp(op, poolIndex, opIndex));；失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 *   10. 调用 / 计算 / 写入：op->state |= OP_SEEN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   11. 调用 / 计算 / 写入：printf("\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   12. 准备局部状态：struct ncclProxyArgs* nextOp = op->nextPeer;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   13. 循环：初始 ；继续条件 (nextOp)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     14. 调用 / 计算 / 写入：NCCLCHECK(getOpIndex(nextOp, state, &poolIndex, &opIndex));；失败时可能提前return，成功才继续；计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。
 *     15. 条件判断：(nextOp->state & OP_SEEN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       16. 真分支：；仅条件为真执行以下子项。
 *         17. 调用 / 计算 / 写入：WARN("List loop at element %d-%d", poolIndex, opIndex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     18. 调用 / 计算 / 写入：printf("| `-> ");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     19. 调用 / 计算 / 写入：NCCLCHECK(printProxyOp(nextOp, poolIndex, opIndex));；失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 *     20. 调用 / 计算 / 写入：nextOp->state |= OP_SEEN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     21. 调用 / 计算 / 写入：printf("\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     22. 条件判断：(nextOp->next)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       23. 真分支：；仅条件为真执行以下子项。
 *         24. 调用 / 计算 / 写入：WARN("Inactive op has next set!\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     25. 调用 / 计算 / 写入：nextOp = nextOp->nextPeer;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   26. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   27. 条件判断：(op->nextPeer == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     28. 真分支：；仅条件为真执行以下子项。
 *       29. 调用 / 计算 / 写入：printf("|\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   30. 调用 / 计算 / 写入：op = op->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   31. 调用 / 计算 / 写入：printf("v\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 32. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 33. 调用 / 计算 / 写入：printf("[X]\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 34. 编译期分支/指令：# if 0；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   35. 调用 / 计算 / 写入：printf("FREE OPS\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   36. 调用 / 计算 / 写入：op = state->pool;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   37. 循环：初始 ；继续条件 (op)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     38. 调用 / 计算 / 写入：NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));；失败时可能提前return，成功才继续；计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。
 *     39. 条件判断：(op->state & OP_SEEN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       40. 真分支：；仅条件为真执行以下子项。
 *         41. 调用 / 计算 / 写入：WARN("List loop at element %d-%d", poolIndex, opIndex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     42. 调用 / 计算 / 写入：NCCLCHECK(printProxyOp(op, poolIndex, opIndex));；失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 *     43. 调用 / 计算 / 写入：op->state |= OP_SEEN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     44. 调用 / 计算 / 写入：printf("->");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     45. 调用 / 计算 / 写入：op = op->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   46. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   47. 调用 / 计算 / 写入：printf("[X]\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   48. 编译期分支/指令：#else；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *     49. 调用 / 计算 / 写入：op = state->pool;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     50. 循环：初始 ；继续条件 (op)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       51. 调用 / 计算 / 写入：NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));；失败时可能提前return，成功才继续；计算一个 ProxyArgs 位于哪个参数池与槽位，主要供诊断日志定位对象。
 *       52. 条件判断：(op->state & OP_SEEN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         53. 真分支：；仅条件为真执行以下子项。
 *           54. 调用 / 计算 / 写入：WARN("List loop at element %d-%d", poolIndex, opIndex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       55. 调用 / 计算 / 写入：op->state |= OP_SEEN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       56. 调用 / 计算 / 写入：op = op->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     57. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 58. 准备局部状态：struct ncclProxyPool* pool = state->pools;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 59. 调用 / 计算 / 写入：poolIndex = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 60. 循环：初始 ；继续条件 (pool)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   61. 准备局部状态：struct ncclProxyArgs* elem = pool->elems;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   62. 循环：初始 int e=0;；继续条件 e<PROXYARGS_ALLOCATE_SIZE；每轮更新 e++, elem++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     63. 条件判断：((elem->state & OP_SEEN) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       64. 真分支：；仅条件为真执行以下子项。
 *         65. 调用 / 计算 / 写入：printf("Elem %d-%d is not in any list:\n", poolIndex, e);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         66. 调用 / 计算 / 写入：NCCLCHECK(printProxyOp(elem, poolIndex, e));；失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 *         67. 调用 / 计算 / 写入：printf("\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       68. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         69. 调用 / 计算 / 写入：elem->state -= OP_SEEN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   70. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   71. 调用 / 计算 / 写入：pool = pool->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   72. 调用 / 计算 / 写入：poolIndex++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 73. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 74. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1018
 */
/* 中文逐项-函数 F1018：dumpProxyState
 * 用途与位置：打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 * 参数 state（struct ncclProxyProgressState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::removeOp；src/proxy.cc::ncclDumpProxyState。
 * 本体直接出现的调用/包装：NCCLCHECK；getOpIndex；printProxyOp。
 * 需要对照的边界/推进条件：op；op->state & OP_SEEN；nextOp；nextOp->state & OP_SEEN。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t dumpProxyState(struct ncclProxyProgressState* state) {
/* 中文逐项-变量 V3609：op
 * op（struct ncclProxyArgs；函数 dumpProxyState 内）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 初值/来源 op = state->active；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyArgs* op = state->active;
/* 中文逐项-变量 V3610：poolIndex、opIndex
 * poolIndex（int；函数 dumpProxyState 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));。若由输出参数填入，必须先检查填充调用成功再读值。
 * opIndex（int；函数 dumpProxyState 内）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int poolIndex, opIndex;
  printf("ACTIVE OPS\n");
  while (op) {
    NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));
    if (op->state & OP_SEEN) {
      WARN("List loop at element %d-%d", poolIndex, opIndex);
    }
    NCCLCHECK(printProxyOp(op, poolIndex, opIndex));
    op->state |= OP_SEEN;
    printf("\n");
/* 中文逐项-变量 V3611：nextOp
 * nextOp（struct ncclProxyArgs；函数 dumpProxyState 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nextOp = op->nextPeer；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyArgs* nextOp = op->nextPeer;
    while (nextOp) {
      NCCLCHECK(getOpIndex(nextOp, state, &poolIndex, &opIndex));
      if (nextOp->state & OP_SEEN) {
        WARN("List loop at element %d-%d", poolIndex, opIndex);
      }
      printf("| `-> ");
      NCCLCHECK(printProxyOp(nextOp, poolIndex, opIndex));
      nextOp->state |= OP_SEEN;
      printf("\n");
      if (nextOp->next) {
        WARN("Inactive op has next set!\n");
      }
      nextOp = nextOp->nextPeer;
    }
    if (op->nextPeer == NULL) printf("|\n");
    op = op->next;
    printf("v\n");
  }
  printf("[X]\n");

# if 0
  printf("FREE OPS\n");
  op = state->pool;
  while (op) {
    NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));
    if (op->state & OP_SEEN) {
      WARN("List loop at element %d-%d", poolIndex, opIndex);
    }
    NCCLCHECK(printProxyOp(op, poolIndex, opIndex));
    op->state |= OP_SEEN;
    printf("->");
    op = op->next;
  }
  printf("[X]\n");
#else
  op = state->pool;
  while (op) {
    NCCLCHECK(getOpIndex(op, state, &poolIndex, &opIndex));
    if (op->state & OP_SEEN) {
      WARN("List loop at element %d-%d", poolIndex, opIndex);
    }
    op->state |= OP_SEEN;
    op = op->next;
  }
#endif

/* 中文逐项-变量 V3612：pool
 * pool（struct ncclProxyPool；函数 dumpProxyState 内）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 初值/来源 pool = state->pools；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyPool* pool = state->pools;
  poolIndex = 0;
  while (pool) {
/* 中文逐项-变量 V3613：elem
 * elem（struct ncclProxyArgs；函数 dumpProxyState 内）：注册work扩展或队列容器内嵌的普通工作元素，扩展信息在它后面保存。
 * 初值/来源 elem = pool->elems；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyArgs* elem = pool->elems;
    for (/* 中文逐项-变量 V3614：e
 * e（int；函数 dumpProxyState 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 e = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：e<PROXYARGS_ALLOCATE_SIZE；循环体与增量表达式决定每轮如何推进。
 */int e=0; e<PROXYARGS_ALLOCATE_SIZE; e++, elem++) {
      if ((elem->state & OP_SEEN) == 0) {
        printf("Elem %d-%d is not in any list:\n", poolIndex, e);
        NCCLCHECK(printProxyOp(elem, poolIndex, e));
        printf("\n");
      } else {
        elem->state -= OP_SEEN;
      }
    }
    pool = pool->next;
    poolIndex++;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1019：ncclProxyOpToArgs
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxySubArgs* sub = args->subs+subIndex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(subIndex >= NCCL_PROXY_MAX_SUBS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("Proxy append out of bounds");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：sub->connection = op->connection;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：sub->channelId = op->channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：sub->nsteps = op->nsteps;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：sub->nbytes = op->nbytes;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：sub->peer = op->root;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：args->nsubs = subIndex+1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 条件判断：(subIndex)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 条件判断：((args->sliceSteps != op->sliceSteps) || (args->chunkSteps != op->chunkSteps) || (args->protocol != op->protocol) || (args->dtype != op->dtype) || (args->redOp != op->redOp))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       15. 真分支：；仅条件为真执行以下子项。
 *         16. 调用 / 计算 / 写入：WARN("Proxy append mismatch");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         17. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     18. 条件判断：(args->state != ncclProxyOpReady)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       19. 真分支：；仅条件为真执行以下子项。
 *         20. 调用 / 计算 / 写入：WARN("Proxy append on running operation");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         21. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     22. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 23. 调用 / 计算 / 写入：args->done = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 24. 调用 / 计算 / 写入：args->opCount = op->opCount;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 25. 调用 / 计算 / 写入：args->sliceSteps = op->sliceSteps;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 26. 调用 / 计算 / 写入：args->chunkSteps = op->chunkSteps;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 27. 调用 / 计算 / 写入：args->chunkSize = op->chunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 28. 调用 / 计算 / 写入：args->dtype = op->dtype;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 29. 调用 / 计算 / 写入：args->redOp = op->redOp;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 调用 / 计算 / 写入：args->pattern = op->pattern;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 31. 调用 / 计算 / 写入：args->protocol = op->protocol;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 调用 / 计算 / 写入：args->state = ncclProxyOpReady;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 33. 调用 / 计算 / 写入：args->progress = op->connection->tcomm->proxyProgress;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 34. 调用 / 计算 / 写入：args->proxyAppendPtr = op->connection->proxyAppendPtr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 35. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1019
 */
/* 中文逐项-函数 F1019：ncclProxyOpToArgs
 * 用途与位置：把紧凑提交描述展开到运行态 args/sub，保留协议、步数、连接和操作编号。
 * 参数 op（struct ncclProxyOp* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 args（struct ncclProxyArgs* args）：代理聚合运行态，保存各连接子任务的posted/received/transmitted/done进度和回调。
 * 参数 subIndex（int subIndex）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ProxyAppend。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：subIndex >= NCCL_PROXY_MAX_SUBS；subIndex；args->sliceSteps != op->sliceSteps) || (args->chunkSteps !=
 * op->chunkSteps) || (args->protocol != op->protocol) || (args->dtype != op->dtype) || …（完整表达式见紧邻源码）；args->state !=
 * ncclProxyOpReady。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclProxyOpToArgs(struct ncclProxyOp* op, struct ncclProxyArgs* args, int subIndex) {
/* 中文逐项-变量 V3615：sub
 * sub（struct ncclProxySubArgs；函数 ncclProxyOpToArgs 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = args->subs+subIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxySubArgs* sub = args->subs+subIndex;
  if (subIndex >= NCCL_PROXY_MAX_SUBS) {
    WARN("Proxy append out of bounds");
    return ncclInternalError;
  }

  //memset(sub, 0, sizeof(struct ncclProxySubArgs));
  sub->connection = op->connection;
  sub->channelId = op->channelId;
  sub->nsteps = op->nsteps;
  sub->nbytes = op->nbytes;
  sub->peer = op->root;
  args->nsubs = subIndex+1;
  if (subIndex) {
    if ((args->sliceSteps != op->sliceSteps) ||
        (args->chunkSteps != op->chunkSteps) ||
        (args->protocol != op->protocol) ||
        (args->dtype != op->dtype) ||
        (args->redOp != op->redOp)) {
      WARN("Proxy append mismatch");
      return ncclInternalError;
    }
    if (args->state != ncclProxyOpReady) {
      WARN("Proxy append on running operation");
      return ncclInternalError;
    }
    return ncclSuccess;
  }
  //memset(&args->progress, 0, sizeof(struct ncclProxyArgs)-offsetof(struct ncclProxyArgs, progress));
  args->done = 0;
  args->opCount = op->opCount;
  args->sliceSteps = op->sliceSteps;
  args->chunkSteps = op->chunkSteps;
  args->chunkSize = op->chunkSize;
  args->dtype = op->dtype;
  args->redOp = op->redOp;
  args->pattern = op->pattern;
  args->protocol = op->protocol;
  args->state = ncclProxyOpReady;
  args->progress = op->connection->tcomm->proxyProgress;
  args->proxyAppendPtr = op->connection->proxyAppendPtr;
  return ncclSuccess;
}

/* 中文内部逻辑 F1020：ProxyAppend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyConnection* connection = op->connection;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int shared = connection->shared;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct ncclProxyArgs* args = *connection->proxyAppendPtr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 条件判断：(args)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 条件判断：(shared && args->opCount == op->opCount)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       7. 真分支：；仅条件为真执行以下子项。
 *         8. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyOpToArgs(op, args, args->nsubs));；失败时可能提前return，成功才继续；把紧凑提交描述展开到运行态 args/sub，保留协议、步数、连接和操作编号。
 *         9. 调用 / 计算 / 写入：DEBUG_PROXY_PRINT("Insert (%d/%5ld/%5ld) as group with %5ld\n", shared, args->opCount, op->opCount, OP_INDEX(args));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       10. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         11. 准备局部状态：struct ncclProxyArgs* prevArgs = args;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         12. 调用 / 计算 / 写入：NCCLCHECK(allocateArgs(state, &args));；失败时可能提前return，成功才继续；从可复用参数池分配运行态 ProxyArgs，容量不足时扩展池块而不逐请求零散分配。
 *         13. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyOpToArgs(op, args, 0));；失败时可能提前return，成功才继续；把紧凑提交描述展开到运行态 args/sub，保留协议、步数、连接和操作编号。
 *         14. 调用 / 计算 / 写入：prevArgs->nextPeer = args;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         15. 调用 / 计算 / 写入：DEBUG_PROXY_PRINT("Insert %5ld (%d/%5ld/%5ld) as nextPeer of %5ld\n", OP_INDEX(args), shared, prevArgs->opCount, args->opCount, OP_INDEX(prevArgs));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         16. 调用 / 计算 / 写入：*(args->proxyAppendPtr) = args;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   17. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     18. 调用 / 计算 / 写入：NCCLCHECK(allocateArgs(state, &args));；失败时可能提前return，成功才继续；从可复用参数池分配运行态 ProxyArgs，容量不足时扩展池块而不逐请求零散分配。
 *     19. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyOpToArgs(op, args, 0));；失败时可能提前return，成功才继续；把紧凑提交描述展开到运行态 args/sub，保留协议、步数、连接和操作编号。
 *     20. 条件判断：(state->active == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       21. 真分支：；仅条件为真执行以下子项。
 *         22. 调用 / 计算 / 写入：DEBUG_PROXY_PRINT("Insert %5ld (%d/%5ld) as first element\n", OP_INDEX(args), shared, args->opCount);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         23. 调用 / 计算 / 写入：state->active = args;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       24. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         25. 准备局部状态：struct ncclProxyArgs* last = state->active;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         26. 循环：初始 ；继续条件 (last->next)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           27. 调用 / 计算 / 写入：last = last->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         28. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         29. 调用 / 计算 / 写入：last->next = args;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         30. 调用 / 计算 / 写入：DEBUG_PROXY_PRINT("Insert %5ld (%d/%5ld) as last element\n", OP_INDEX(args), shared, args->opCount);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     31. 调用 / 计算 / 写入：*(args->proxyAppendPtr) = args;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1020
 */
/* 中文逐项-函数 F1020：ProxyAppend
 * 用途与位置：把提交操作追加/聚合到活跃链或同连接后继链，维持共享连接上的先后顺序。
 * 参数 state（struct ncclProxyProgressState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 参数 op（struct ncclProxyOp* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyGetPostedOps。
 * 本体直接出现的调用/包装：DEBUG_PROXY_PRINT；NCCLCHECK；OP_INDEX；allocateArgs；ncclProxyOpToArgs。
 * 需要对照的边界/推进条件：args；shared && args->opCount == op->opCount；state->active == NULL；last->next。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ProxyAppend(struct ncclProxyProgressState* state, struct ncclProxyOp* op) {
/* 中文逐项-变量 V3616：connection
 * connection（struct ncclProxyConnection；函数 ProxyAppend 内）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 初值/来源 connection = op->connection；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyConnection* connection = op->connection;
/* 中文逐项-变量 V3617：shared
 * shared（int；函数 ProxyAppend 内）：是否使用共享缓冲资源的标志；不自动表示所有 channel 或全部 backend 连接合并。
 * 初值/来源 shared = connection->shared；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int shared = connection->shared;
/* 中文逐项-变量 V3618：args
 * args（struct ncclProxyArgs；函数 ProxyAppend 内）：代理聚合运行态，保存各连接子任务的posted/received/transmitted/done进度和回调。
 * 初值/来源 args = *connection->proxyAppendPtr；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyArgs* args = *connection->proxyAppendPtr;

  if (args) {
    if (shared && args->opCount == op->opCount) {
      NCCLCHECK(ncclProxyOpToArgs(op, args, args->nsubs));
      DEBUG_PROXY_PRINT("Insert (%d/%5ld/%5ld) as group with %5ld\n", shared, args->opCount, op->opCount, OP_INDEX(args));
    } else {
/* 中文逐项-变量 V3619：prevArgs
 * prevArgs（struct ncclProxyArgs；函数 ProxyAppend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 prevArgs = args；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      struct ncclProxyArgs* prevArgs = args;
      NCCLCHECK(allocateArgs(state, &args));
      NCCLCHECK(ncclProxyOpToArgs(op, args, 0));
      prevArgs->nextPeer = args;
      DEBUG_PROXY_PRINT("Insert  %5ld (%d/%5ld/%5ld) as nextPeer of %5ld\n", OP_INDEX(args), shared, prevArgs->opCount, args->opCount, OP_INDEX(prevArgs));
      *(args->proxyAppendPtr) = args;
    }
  } else {
    // Nothing running for that peer. Add to the list
    NCCLCHECK(allocateArgs(state, &args));
    NCCLCHECK(ncclProxyOpToArgs(op, args, 0));
    if (state->active == NULL) {
      // Create the list
      DEBUG_PROXY_PRINT("Insert  %5ld (%d/%5ld) as first element\n", OP_INDEX(args), shared, args->opCount);
      state->active = args;
    } else {
      // Append element at the end of the list
/* 中文逐项-变量 V3620：last
 * last（struct ncclProxyArgs；函数 ProxyAppend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 last = state->active；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclProxyArgs* last = state->active;
      while (last->next) last = last->next;
      last->next = args;
      DEBUG_PROXY_PRINT("Insert  %5ld (%d/%5ld) as last element\n", OP_INDEX(args), shared, args->opCount);
    }
    *(args->proxyAppendPtr) = args;
  }
  return ncclSuccess;
}

/* 中文精读：CPU 怎样唤醒数据面代理线程
 * 生产者在互斥锁内把链表接到 opsPool；队列先前为空时用 pthread_cond_signal 唤醒等待线程。
 * 不是每个 slice 都新建线程，也没有要求 GPU 主动发 CPU 中断。
 * progress 线程活跃时会继续轮询网络请求与 GPU 进度，无任务时才在条件变量上睡眠。
 * 任务发布的锁保护与数据缓冲的 head/tail/flag 协议不同，不能只保留其中一种同步。
 */
/* 中文内部逻辑 F1021：ncclProxyPost
 * 精读：加锁后追加任务索引链，只有由空转为非空时需要signal唤醒等待线程。
 * 精读：锁保护的是CPU任务链元数据；真实数据仍按head/tail/flag协议与GPU协作。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：pthread_mutex_lock(&pool->mutex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 条件判断：(pool->nextOps == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：pool->nextOps = nextOps;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     5. 调用 / 计算 / 写入：pthread_cond_signal(&pool->cond);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   6. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     7. 调用 / 计算 / 写入：pool->ops[pool->nextOpsEnd].next = nextOps;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：pool->nextOpsEnd = nextOpsEnd;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：pthread_mutex_unlock(&pool->mutex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1021
 */
/* 中文逐项-函数 F1021：ncclProxyPost
 * 用途与位置：在锁内发布任务索引链，队列从空转为非空时通过条件变量唤醒 Progress 线程。
 * 参数 pool（struct ncclProxyOpsPool* pool）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 参数 nextOps（int nextOps）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 * 参数 nextOpsEnd（int nextOpsEnd）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclLocalOpAppend；src/proxy.cc::ncclProxyStart。
 * 本体直接出现的调用/包装：pthread_cond_signal；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：pool->nextOps == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclProxyPost(struct ncclProxyOpsPool* pool, int nextOps, int nextOpsEnd) {
  pthread_mutex_lock(&pool->mutex);
  if (pool->nextOps == -1) {
    pool->nextOps = nextOps;
    pthread_cond_signal(&pool->cond);
  } else {
    pool->ops[pool->nextOpsEnd].next = nextOps;
  }
  pool->nextOpsEnd = nextOpsEnd;
  pthread_mutex_unlock(&pool->mutex);
  return ncclSuccess;
}

/* 中文内部逻辑 F1022：ncclLocalOpAppend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyOps* proxyOps = proxyConn->comm->proxyState.proxyOps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(proxyOps == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 调用 / 计算 / 写入：proxyOps += proxyConn->localRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 准备局部状态：struct ncclProxyOpsPool* pool = proxyOps->pool;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：TIME_START(0);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 准备局部状态：int opIndex = proxyOps->freeOp;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 准备局部状态：struct ncclProxyOp* op;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 条件判断：(opIndex != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 调用 / 计算 / 写入：op = pool->ops+opIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 调用 / 计算 / 写入：proxyOps->freeOp = op->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   14. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     15. 准备局部状态：int freeOp;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     16. 循环：初始 ；继续条件 ((freeOp = pool->freeOps[comm->localRank]) == -1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       17. 调用 / 计算 / 写入：sched_yield();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     18. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     19. 准备局部状态：int freeOpNew;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     20. 循环：初始 ；继续条件 ((freeOpNew = __sync_val_compare_and_swap(pool->freeOps+comm->localRank, freeOp, -1)) != freeOp)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       21. 调用 / 计算 / 写入：freeOp = freeOpNew;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     22. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     23. 调用 / 计算 / 写入：opIndex = freeOp;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     24. 调用 / 计算 / 写入：op = pool->ops+opIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     25. 调用 / 计算 / 写入：proxyOps->freeOp = op->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 26. 条件判断：(op->next != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   27. 真分支：；仅条件为真执行以下子项。
 *     28. 调用 / 计算 / 写入：__builtin_prefetch(pool->ops+op->next);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 29. 调用 / 计算 / 写入：memcpy(op, proxyOp, sizeof(struct ncclProxyOp));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 30. 调用 / 计算 / 写入：op->next = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 31. 调用 / 计算 / 写入：op->connection = proxyConn->connection;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 条件判断：(proxyOps->nextOps == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   33. 真分支：；仅条件为真执行以下子项。
 *     34. 调用 / 计算 / 写入：proxyOps->nextOps = proxyOps->nextOpsEnd = opIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   35. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     36. 调用 / 计算 / 写入：pool->ops[proxyOps->nextOpsEnd].next = opIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     37. 调用 / 计算 / 写入：proxyOps->nextOpsEnd = opIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 38. 条件判断：(++proxyOps->count == MAX_OPS_PER_PEER)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   39. 真分支：；仅条件为真执行以下子项。
 *     40. 准备局部状态：uint64_t lastOpCount = pool->ops[proxyOps->nextOpsEnd].opCount;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     41. 准备局部状态：int lastOp = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     42. 准备局部状态：int toSend = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     43. 准备局部状态：int ops = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     44. 循环：初始 int op= proxyOps->nextOps;；继续条件 op != proxyOps->nextOpsEnd；每轮更新 op=pool->ops[op].next；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       45. 调用 / 计算 / 写入：ops++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       46. 条件判断：(pool->ops[op].opCount != lastOpCount)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         47. 真分支：；仅条件为真执行以下子项。
 *           48. 调用 / 计算 / 写入：lastOp = op;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           49. 调用 / 计算 / 写入：toSend = ops;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     50. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     51. 条件判断：(lastOp == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       52. 真分支：；仅条件为真执行以下子项。
 *         53. 调用 / 计算 / 写入：WARN("Unable to post incomplete proxy op chain %d..%d (opCount %ld)\n", proxyOps->nextOps, proxyOps->nextOpsEnd, lastOpCount);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         54. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     55. 准备局部状态：int nextOps = proxyOps->nextOps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     56. 调用 / 计算 / 写入：proxyOps->nextOps = pool->ops[lastOp].next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     57. 调用 / 计算 / 写入：pool->ops[lastOp].next = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     58. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyPost(proxyOps->pool, nextOps, lastOp));；失败时可能提前return，成功才继续；在锁内发布任务索引链，队列从空转为非空时通过条件变量唤醒 Progress 线程。
 *     59. 调用 / 计算 / 写入：proxyOps->count -= toSend;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 60. 调用 / 计算 / 写入：TIME_STOP(0);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 61. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1022
 */
/* 中文逐项-函数 F1022：ncclLocalOpAppend
 * 用途与位置：从本地代理共享池取空闲项复制 ProxyOp，追加待发布链并在容量紧张时发布完整批次。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 proxyConn（struct ncclProxyConnector* proxyConn）：逻辑连接到代理服务的引用，记录代理 rank 和实际后台连接。
 * 参数 proxyOp（struct ncclProxyOp* proxyOp）：交给 CPU Proxy 的紧凑计划，与 GPU work 的协议和步数必须一致。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::SaveProxy。
 * 本体直接出现的调用/包装：NCCLCHECK；TIME_START；TIME_STOP；__builtin_prefetch；__sync_val_compare_and_swap；memcpy；ncclProxyPost；sched_yield。
 * 需要对照的边界/推进条件：proxyOps == NULL；opIndex != -1；freeOp = pool->freeOps[comm->localRank]) == -1；freeOpNew =
 * __sync_val_compare_and_swap(pool->freeOps+comm->localRank, freeOp, -1)) != freeOp。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclLocalOpAppend(struct ncclComm* comm, struct ncclProxyConnector* proxyConn, struct ncclProxyOp* proxyOp) {
/* 中文逐项-变量 V3621：proxyOps
 * proxyOps（struct ncclProxyOps；函数 ncclLocalOpAppend 内）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 初值/来源 proxyOps = proxyConn->comm->proxyState.proxyOps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyOps* proxyOps = proxyConn->comm->proxyState.proxyOps;
  if (proxyOps == NULL) return ncclInternalError;
  proxyOps += proxyConn->localRank;
/* 中文逐项-变量 V3622：pool
 * pool（struct ncclProxyOpsPool；函数 ncclLocalOpAppend 内）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 初值/来源 pool = proxyOps->pool；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyOpsPool* pool = proxyOps->pool;

  TIME_START(0);
/* 中文逐项-变量 V3623：opIndex
 * opIndex（int；函数 ncclLocalOpAppend 内）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 初值/来源 opIndex = proxyOps->freeOp；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int opIndex = proxyOps->freeOp;
/* 中文逐项-变量 V3624：op
 * op（struct ncclProxyOp；函数 ncclLocalOpAppend 内）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 此处未显式给初值；后续首个文本使用点：op = pool->ops+opIndex;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyOp* op;
  if (opIndex != -1) {
    op = pool->ops+opIndex;
    proxyOps->freeOp = op->next;
  } else {
/* 中文逐项-变量 V3625：freeOp
 * freeOp（int；函数 ncclLocalOpAppend 内）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 * 此处未显式给初值；后续首个文本使用点：while ((freeOp = pool->freeOps[comm->localRank]) == -1) sched_yield();。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int freeOp;
    while ((freeOp = pool->freeOps[comm->localRank]) == -1) sched_yield();
/* 中文逐项-变量 V3626：freeOpNew
 * freeOpNew（int；函数 ncclLocalOpAppend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：while ((freeOpNew = __sync_val_compare_and_swap(pool->freeOps+comm->localRank, freeOp, -1)) !=
 * freeOp) freeOp = freeOpNew;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int freeOpNew;
    while ((freeOpNew = __sync_val_compare_and_swap(pool->freeOps+comm->localRank, freeOp, -1)) != freeOp) freeOp = freeOpNew;
    opIndex = freeOp;
    op = pool->ops+opIndex;
    proxyOps->freeOp = op->next;
  }
  if (op->next != -1) __builtin_prefetch(pool->ops+op->next); // Prefetch next free op
  memcpy(op, proxyOp, sizeof(struct ncclProxyOp));
  op->next = -1;
  op->connection = proxyConn->connection;
  if (proxyOps->nextOps == -1) {
    proxyOps->nextOps = proxyOps->nextOpsEnd = opIndex;
  } else {
    pool->ops[proxyOps->nextOpsEnd].next = opIndex;
    proxyOps->nextOpsEnd = opIndex;
  }
  if (++proxyOps->count == MAX_OPS_PER_PEER) {
    // Post what we have so far to free some ops in the pool
    // Do not post last operations as we could have more coming with the same opCount, and posting
    // them in different batches would break proxyArgs aggregation with subs.
/* 中文逐项-变量 V3627：lastOpCount
 * lastOpCount（uint64_t；函数 ncclLocalOpAppend 内）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 lastOpCount = pool->ops[proxyOps->nextOpsEnd].opCount；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    uint64_t lastOpCount = pool->ops[proxyOps->nextOpsEnd].opCount;
/* 中文逐项-变量 V3628：lastOp
 * lastOp（int；函数 ncclLocalOpAppend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lastOp = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
    int lastOp = -1;
/* 中文逐项-变量 V3629：toSend
 * toSend（int；函数 ncclLocalOpAppend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 toSend = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int toSend = 0;
/* 中文逐项-变量 V3630：ops
 * ops（int；函数 ncclLocalOpAppend 内）：所处类型的provider函数表、代理待办数组或任务链，按声明类型读取，不能跨类型强转解释。
 * 初值/来源 ops = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int ops = 0;
    for (/* 中文逐项-变量 V3631：op
 * op（int；函数 ncclLocalOpAppend 内）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 初值/来源 op = proxyOps->nextOps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：op != proxyOps->nextOpsEnd；循环体与增量表达式决定每轮如何推进。
 */int op= proxyOps->nextOps; op != proxyOps->nextOpsEnd; op=pool->ops[op].next) {
      ops++;
      if (pool->ops[op].opCount != lastOpCount) {
        lastOp = op;
        toSend = ops;
      }
    }
    if (lastOp == -1) {
      WARN("Unable to post incomplete proxy op chain %d..%d (opCount %ld)\n", proxyOps->nextOps, proxyOps->nextOpsEnd, lastOpCount);
      return ncclInternalError;
    }
    // Cut chain at lastOp
/* 中文逐项-变量 V3632：nextOps
 * nextOps（int；函数 ncclLocalOpAppend 内）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 * 初值/来源 nextOps = proxyOps->nextOps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int nextOps = proxyOps->nextOps;
    proxyOps->nextOps = pool->ops[lastOp].next;
    pool->ops[lastOp].next = -1;
    NCCLCHECK(ncclProxyPost(proxyOps->pool, nextOps, lastOp));
    proxyOps->count -= toSend;
  }
  TIME_STOP(0);
  return ncclSuccess;
}

/* 中文精读：只有需要数据面 Proxy 的边才产生工作
 * 先按 channel/peer/方向/connIndex 找 connector；peer=-1 是缺失邻居，可直接跳过。
 * 若 transportComm->proxyProgress 为空，也直接返回：本地 GPU P2P/SHM 不必复制一份网络任务。
 * 因此 API 即使调用 ProxySaveColl，也不意味着每个 channel 的每条边都启动 IB 传输。
 * 实际追加的是连接上的元数据，数据仍由 GPU/协议缓冲/网络后端共同搬运。
 */
/* 中文内部逻辑 F1023：SaveProxy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(peer < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：struct ncclPeer* peerComm = channel->peers+peer;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：struct ncclConnector* connector = type == proxyRecv ? peerComm->recv+connIndex : peerComm->send+connIndex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 条件判断：(connector->transportComm == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 调用 / 计算 / 写入：WARN("Rank %d has no transport for %s peer %d on channel %d/%d", connector->comm->rank, type == proxyRecv ? "recv" : "send", peer, channel->id, connIndex);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 条件判断：(connector->transportComm->proxyProgress == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 13. 调用 / 计算 / 写入：NCCLCHECK(ncclLocalOpAppend(connector->comm, &connector->proxyConn, op));；失败时可能提前return，成功才继续；从本地代理共享池取空闲项复制 ProxyOp，追加待发布链并在容量紧张时发布完整批次。
 * 14. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1023
 */
/* 中文逐项-函数 F1023：SaveProxy
 * 用途与位置：找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 * 参数 channel（struct ncclChannel* channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 op（struct ncclProxyOp* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 connIndex（int connIndex）：同一 peer/方向内的连接槽编号；本版普通集合用0，常规 P2P 用1。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxySaveColl；src/proxy.cc::ncclProxySaveP2p。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclLocalOpAppend。
 * 需要对照的边界/推进条件：peer < 0；connector->transportComm == NULL；connector->transportComm->proxyProgress ==
 * NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t SaveProxy(struct ncclChannel* channel, int type, int peer, struct ncclProxyOp* op, int connIndex) {
  if (peer < 0) return ncclSuccess;

/* 中文逐项-变量 V3633：peerComm
 * peerComm（struct ncclPeer；函数 SaveProxy 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 peerComm = channel->peers+peer；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclPeer* peerComm = channel->peers+peer;
/* 中文逐项-变量 V3634：connector
 * connector（struct ncclConnector；函数 SaveProxy 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 connector = type == proxyRecv ? peerComm->recv+connIndex :
 * peerComm->send+connIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclConnector* connector = type == proxyRecv ? peerComm->recv+connIndex : peerComm->send+connIndex;
  if (connector->transportComm == NULL) {
    WARN("Rank %d has no transport for %s peer %d on channel %d/%d", connector->comm->rank,
        type == proxyRecv ? "recv" : "send", peer, channel->id, connIndex);
    return ncclInternalError;
  }
  if (connector->transportComm->proxyProgress == NULL) return ncclSuccess;

  NCCLCHECK(ncclLocalOpAppend(connector->comm, &connector->proxyConn, op));
  return ncclSuccess;
}

/* 中文精读：图示绑定 V3.3 F1024 ncclProxySaveColl
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F1024
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0097] ncclProxySaveColl：ProxyStart发布已有队列，progress循环调用具体transport回调；收发图对应NET transport数据面，不代表每个本地P2P数据块也走网络代理（调度上下文）
 *   图：01-submit, 21-proxy-send, 22-proxy-recv
 *   定位：#symbol=F1024&binding=B0097
 */
/* 中文内部逻辑 F1024：ncclProxySaveColl
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclChannel* channel = comm->channels+op->channelId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int pattern = op->pattern;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(pattern == ncclPatternRing || pattern == ncclPatternRingTwice || pattern == ncclPatternPipelineFrom || pattern == ncclPatternPipelineTo)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 准备局部状态：struct ncclRing* ring = &channel->ring;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 条件判断：(NeedProxy(proxyRecv, pattern, op->root, ring, nranks))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       7. 真分支：；仅条件为真执行以下子项。
 *         8. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxyRecv, ring->prev, op, 0));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 *     9. 条件判断：(NeedProxy(proxySend, pattern, op->root, ring, nranks))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       10. 真分支：；仅条件为真执行以下子项。
 *         11. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxySend, ring->next, op, 0));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 * 12. 条件判断：(pattern == ncclPatternTreeUp || pattern == ncclPatternTreeUpDown)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 准备局部状态：struct ncclTree* tree = &channel->tree;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     15. 循环：初始 int i=0;；继续条件 i<NCCL_MAX_TREE_ARITY；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       16. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxyRecv, tree->down[i], op, 0));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 *     17. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     18. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxySend, tree->up, op, 0));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 * 19. 条件判断：(pattern == ncclPatternTreeDown || pattern == ncclPatternTreeUpDown)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 准备局部状态：struct ncclTree* tree = &channel->tree;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     22. 循环：初始 int i=0;；继续条件 i< NCCL_MAX_TREE_ARITY；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       23. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxySend, tree->down[i], op, 0));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 *     24. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     25. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxyRecv, tree->up, op, 0));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 * 26. 条件判断：(pattern == ncclPatternCollTreeUpDown)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   27. 真分支：；仅条件为真执行以下子项。
 *     28. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxySend, channel->collTree.out, op, 1));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 *     29. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxyRecv, channel->collTree.out, op, 0));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 * 30. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1024
 */
/* 中文逐项-函数 F1024：ncclProxySaveColl
 * 用途与位置：按 Ring/Tree/Pipeline/CollNet 模式展开必要邻居与方向，给相应连接保存代理工作。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 op（struct ncclProxyOp* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueCollKernel。
 * 本体直接出现的调用/包装：NCCLCHECK；NeedProxy；SaveProxy。
 * 需要对照的边界/推进条件：pattern == ncclPatternRing || pattern == ncclPatternRingTwice || pattern == ncclPatternPipelineFrom ||
 * pattern == ncclPatternPipelineTo；NeedProxy(proxyRecv, pattern, op->root, ring, nranks；NeedProxy(proxySend, pattern,
 * op->root, ring, nranks；pattern == ncclPatternTreeUp || pattern == ncclPatternTreeUpDown。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxySaveColl(struct ncclComm* comm, struct ncclProxyOp* op, int nranks) {
/* 中文逐项-变量 V3635：channel
 * channel（struct ncclChannel；函数 ncclProxySaveColl 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+op->channelId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclChannel* channel = comm->channels+op->channelId;
/* 中文逐项-变量 V3636：pattern
 * pattern（int；函数 ncclProxySaveColl 内）：图通信模式与允许的机内/机间路径类别，决定搜索可接受候选。
 * 初值/来源 pattern = op->pattern；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int pattern = op->pattern;
  if (pattern == ncclPatternRing || pattern == ncclPatternRingTwice || pattern == ncclPatternPipelineFrom || pattern == ncclPatternPipelineTo) {
/* 中文逐项-变量 V3637：ring
 * ring（struct ncclRing；函数 ncclProxySaveColl 内）：本 channel 的环邻居和内部顺序，决定前驱接收与后继发送。
 * 初值/来源 ring = &channel->ring；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    struct ncclRing* ring = &channel->ring;
    if (NeedProxy(proxyRecv, pattern, op->root, ring, nranks)) NCCLCHECK(SaveProxy(channel, proxyRecv, ring->prev, op, 0));
    if (NeedProxy(proxySend, pattern, op->root, ring, nranks)) NCCLCHECK(SaveProxy(channel, proxySend, ring->next, op, 0));
  }
  if (pattern == ncclPatternTreeUp || pattern == ncclPatternTreeUpDown) {
    // Tree up
/* 中文逐项-变量 V3638：tree
 * tree（struct ncclTree；函数 ncclProxySaveColl 内）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 * 初值/来源 tree = &channel->tree；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    struct ncclTree* tree = &channel->tree;
    for (/* 中文逐项-变量 V3639：i
 * i（int；函数 ncclProxySaveColl 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<NCCL_MAX_TREE_ARITY；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<NCCL_MAX_TREE_ARITY; i++) NCCLCHECK(SaveProxy(channel, proxyRecv, tree->down[i], op, 0));
    NCCLCHECK(SaveProxy(channel, proxySend, tree->up, op, 0));
  }
  if (pattern == ncclPatternTreeDown || pattern == ncclPatternTreeUpDown) {
    // Tree down
/* 中文逐项-变量 V3640：tree
 * tree（struct ncclTree；函数 ncclProxySaveColl 内）：本 channel 的树邻接关系，up/down 决定向上归约和向下广播。
 * 初值/来源 tree = &channel->tree；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    struct ncclTree* tree = &channel->tree;
    for (/* 中文逐项-变量 V3641：i
 * i（int；函数 ncclProxySaveColl 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i< NCCL_MAX_TREE_ARITY；循环体与增量表达式决定每轮如何推进。
 */int i=0; i< NCCL_MAX_TREE_ARITY; i++) NCCLCHECK(SaveProxy(channel, proxySend, tree->down[i], op, 0));
    NCCLCHECK(SaveProxy(channel, proxyRecv, tree->up, op, 0));
  }
  if (pattern == ncclPatternCollTreeUpDown) {
    // CollTree up
    NCCLCHECK(SaveProxy(channel, proxySend, channel->collTree.out, op, 1));  // For CollTree up, we are using push
    // CollTree down
    NCCLCHECK(SaveProxy(channel, proxyRecv, channel->collTree.out, op, 0));
  }
  return ncclSuccess;
}

/* 中文逐项-参数getter P0034：ChunkSize
 * 生成函数 ncclParamChunkSize()，读取 NCCL_CHUNK_SIZE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(ChunkSize, "CHUNK_SIZE", 0);

/* 中文内部逻辑 F1025：ncclProxyComputeP2p
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：memset(op, 0, sizeof(struct ncclProxyOp));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 准备局部状态：int channelId = info->channelId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct ncclChannel* channel = info->comm->channels+channelId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：op->channelId = channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：op->sliceSteps = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 调用 / 计算 / 写入：op->chunkSteps = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：op->protocol = NCCL_PROTO_SIMPLE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：op->dtype = info->datatype;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 准备局部状态：int stepSize = info->comm->buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS/SENDRECV_SLICEFACTOR;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 调用 / 计算 / 写入：info->chunkSize = stepSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：op->root = info->root;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：op->nbytes = info->count;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 准备局部状态：struct ncclPeer* peer = channel->peers + op->root;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 条件判断：(info->coll == ncclFuncSend)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 调用 / 计算 / 写入：op->pattern = ncclPatternSend;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     17. 条件判断：(op->root != info->comm->rank && peer->send[1].transportComm && peer->send[1].transportComm->proxyProgress)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       18. 真分支：；仅条件为真执行以下子项。
 *         19. 条件判断：(info->count < stepSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           20. 真分支：；仅条件为真执行以下子项。
 *             21. 调用 / 计算 / 写入：info->chunkSize /= 4;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           22. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             23. 条件判断：(info->count < 8*stepSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               24. 真分支：；仅条件为真执行以下子项。
 *                 25. 调用 / 计算 / 写入：info->chunkSize /= 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   26. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     27. 条件判断：(info->coll == ncclFuncRecv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       28. 真分支：；仅条件为真执行以下子项。
 *         29. 调用 / 计算 / 写入：op->pattern = ncclPatternRecv;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         30. 条件判断：(op->root != info->comm->rank && peer->recv[1].transportComm && peer->recv[1].transportComm->proxyProgress)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           31. 真分支：；仅条件为真执行以下子项。
 *             32. 条件判断：(info->count < stepSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               33. 真分支：；仅条件为真执行以下子项。
 *                 34. 调用 / 计算 / 写入：info->chunkSize /= 4;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               35. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                 36. 条件判断：(info->count < 8*stepSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   37. 真分支：；仅条件为真执行以下子项。
 *                     38. 调用 / 计算 / 写入：info->chunkSize /= 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       39. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         40. 调用 / 计算 / 写入：WARN("P2p operation is neither send or recv");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         41. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 42. 条件判断：(ncclParamChunkSize() != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   43. 真分支：；仅条件为真执行以下子项。
 *     44. 调用 / 计算 / 写入：info->chunkSize = ncclParamChunkSize();；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 45. 调用 / 计算 / 写入：op->chunkSize = info->chunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 46. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1025
 */
/* 中文逐项-函数 F1025：ncclProxyComputeP2p
 * 用途与位置：根据已切好的 P2P 块计算协议尺寸、step 数和 channel/peer 标签，使 CPU/GPU 计划一致。
 * 参数 info（struct ncclInfo* info）：本次API的主机中间表示；先保存原始用户参数，再由检查/选型/分片填充为GPU与Proxy双计划。
 * 参数 op（struct ncclProxyOp* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclSetupP2pKernel。
 * 本体直接出现的调用/包装：memset；ncclParamChunkSize。
 * 需要对照的边界/推进条件：info->coll == ncclFuncSend；op->root != info->comm->rank && peer->send[1].transportComm &&
 * peer->send[1].transportComm->proxyProgress；info->count < stepSize；info->count < 8*stepSize。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyComputeP2p(struct ncclInfo* info, struct ncclProxyOp* op) {
  memset(op, 0, sizeof(struct ncclProxyOp));
/* 中文逐项-变量 V3642：channelId
 * channelId（int；函数 ncclProxyComputeP2p 内）：实际 channels 数组下标；与一次操作内的分片编号 bid 区分。
 * 初值/来源 channelId = info->channelId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int channelId = info->channelId;
/* 中文逐项-变量 V3643：channel
 * channel（struct ncclChannel；函数 ncclProxyComputeP2p 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = info->comm->channels+channelId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclChannel* channel = info->comm->channels+channelId;
  op->channelId = channelId;
  op->sliceSteps = 1;
  op->chunkSteps = 1;
  op->protocol = NCCL_PROTO_SIMPLE;
  op->dtype = info->datatype;

/* 中文逐项-变量 V3644：stepSize
 * stepSize（int；函数 ncclProxyComputeP2p 内）：一 FIFO step 的容量；设备 T 指针路径以元素计，主机缓冲路径以字节计。
 * 初值/来源 stepSize =
 * info->comm->buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS/SENDRECV_SLICEFACTOR；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int stepSize = info->comm->buffSizes[NCCL_PROTO_SIMPLE]/NCCL_STEPS/SENDRECV_SLICEFACTOR;
  info->chunkSize = stepSize;
  op->root = info->root;
  op->nbytes = info->count;
/* 中文逐项-变量 V3645：peer
 * peer（struct ncclPeer；函数 ncclProxyComputeP2p 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = channel->peers + op->root；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclPeer* peer = channel->peers + op->root;

  if (info->coll == ncclFuncSend) {
    op->pattern = ncclPatternSend;
    if (op->root != info->comm->rank && peer->send[1].transportComm && peer->send[1].transportComm->proxyProgress) {
      // Tune chunk size for the network
      if (info->count < stepSize) info->chunkSize /= 4;
      else if (info->count < 8*stepSize) info->chunkSize /= 2;
    }
  } else if (info->coll == ncclFuncRecv) {
    op->pattern = ncclPatternRecv;
    if (op->root != info->comm->rank && peer->recv[1].transportComm && peer->recv[1].transportComm->proxyProgress) {
      // Tune chunk size for the network
      if (info->count < stepSize) info->chunkSize /= 4;
      else if (info->count < 8*stepSize) info->chunkSize /= 2;
    }
  } else {
    WARN("P2p operation is neither send or recv");
    return ncclInternalError;
  }
  if (ncclParamChunkSize() != 0) {
    info->chunkSize = ncclParamChunkSize();
  }
  op->chunkSize = info->chunkSize;
  return ncclSuccess;
}

/* 中文内部逻辑 F1026：ncclProxySaveP2p
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclChannel* channel = comm->channels+op->channelId;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：op->opCount = channel->workFifoTail-1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 条件判断：(op->root == comm->rank)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 条件判断：(op->pattern == ncclPatternRecv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 调用 / 计算 / 写入：op->nsteps = DIVUP(op->nbytes, op->chunkSize);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 条件判断：(op->nsteps == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       10. 真分支：；仅条件为真执行以下子项。
 *         11. 调用 / 计算 / 写入：op->nsteps = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     12. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxyRecv, op->root, op, 1));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 *   13. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     14. 条件判断：(op->pattern == ncclPatternSend)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       15. 真分支：；仅条件为真执行以下子项。
 *         16. 调用 / 计算 / 写入：op->nsteps = DIVUP(op->nbytes, op->chunkSize);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         17. 条件判断：(op->nsteps == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           18. 真分支：；仅条件为真执行以下子项。
 *             19. 调用 / 计算 / 写入：op->nsteps = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         20. 调用 / 计算 / 写入：NCCLCHECK(SaveProxy(channel, proxySend, op->root, op, 1));；失败时可能提前return，成功才继续；找到 channel/peer/方向/索引的连接，仅当 transport 有 proxyProgress 回调时记录代理工作。
 * 21. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1026
 */
/* 中文逐项-函数 F1026：ncclProxySaveP2p
 * 用途与位置：按 P2P 任务方向和连接索引1保存代理需求，跳过 self 或无代理的本地路径。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 op（struct ncclProxyOp* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclEnqueueP2pKernel。
 * 本体直接出现的调用/包装：DIVUP；NCCLCHECK；SaveProxy。
 * 需要对照的边界/推进条件：op->root == comm->rank；op->pattern == ncclPatternRecv；op->nsteps == 0；op->pattern ==
 * ncclPatternSend。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxySaveP2p(struct ncclComm* comm, struct ncclProxyOp* op) {
/* 中文逐项-变量 V3646：channel
 * channel（struct ncclChannel；函数 ncclProxySaveP2p 内）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 初值/来源 channel = comm->channels+op->channelId；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclChannel* channel = comm->channels+op->channelId;
  op->opCount = channel->workFifoTail-1;
  if (op->root == comm->rank) return ncclSuccess;
  if (op->pattern == ncclPatternRecv) {
    op->nsteps = DIVUP(op->nbytes, op->chunkSize);
    if (op->nsteps == 0) op->nsteps = 1;
    NCCLCHECK(SaveProxy(channel, proxyRecv, op->root, op, 1));
  } else if (op->pattern == ncclPatternSend) {
    op->nsteps = DIVUP(op->nbytes, op->chunkSize);
    if (op->nsteps == 0) op->nsteps = 1;
    NCCLCHECK(SaveProxy(channel, proxySend, op->root, op, 1));
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1027：removeOp
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyArgs* freeOp = *opPtr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclProxyArgs* next = freeOp->next;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：DEBUG_PROXY_PRINT("Remove %ld -> %ld -> %ld\n", OP_INDEX(*prevOpPtr), OP_INDEX(freeOp), OP_INDEX(next));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 调用 / 计算 / 写入：*opPtr = next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 条件判断：(freeOp->nextPeer)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 准备局部状态：struct ncclProxyArgs* nextPeer = freeOp->nextPeer;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     8. 条件判断：(*prevOpPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       9. 真分支：；仅条件为真执行以下子项。
 *         10. 调用 / 计算 / 写入：(*prevOpPtr)->next = nextPeer;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       11. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         12. 调用 / 计算 / 写入：state->active = nextPeer;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     13. 调用 / 计算 / 写入：nextPeer->next = next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     14. 调用 / 计算 / 写入：*(prevOpPtr) = nextPeer;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   15. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     16. 调用 / 计算 / 写入：*(freeOp->proxyAppendPtr) = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     17. 条件判断：(*prevOpPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       18. 真分支：；仅条件为真执行以下子项。
 *         19. 调用 / 计算 / 写入：(*prevOpPtr)->next = next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       20. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         21. 调用 / 计算 / 写入：state->active = next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 22. 调用 / 计算 / 写入：freeOp->next = state->pool;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 23. 调用 / 计算 / 写入：state->pool = freeOp;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 24. 调用 / 计算 / 写入：DEBUG_PROXY_PRINT("Removed %5ld (%5ld) : ", OP_INDEX(freeOp), OP_INDEX(*freeOp->proxyAppendPtr));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 25. 编译期分支/指令：#ifdef DEBUG_PROXY；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   26. 调用 / 计算 / 写入：NCCLCHECK(dumpProxyState(state));；失败时可能提前return，成功才继续；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 * 27. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1027
 */
/* 中文逐项-函数 F1027：removeOp
 * 用途与位置：从活跃 Proxy 链移除完成项，提升同连接后继项并归还可复用参数对象。
 * 参数 state（struct ncclProxyProgressState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 参数 opPtr（struct ncclProxyArgs** opPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 prevOpPtr（struct ncclProxyArgs** prevOpPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::progressOps。
 * 本体直接出现的调用/包装：DEBUG_PROXY_PRINT；NCCLCHECK；OP_INDEX；dumpProxyState。
 * 需要对照的边界/推进条件：freeOp->nextPeer；*prevOpPtr；*prevOpPtr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t removeOp(struct ncclProxyProgressState* state, struct ncclProxyArgs** opPtr, struct ncclProxyArgs** prevOpPtr) {
/* 中文逐项-变量 V3647：freeOp
 * freeOp（struct ncclProxyArgs；函数 removeOp 内）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 * 初值/来源 freeOp = *opPtr；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclProxyArgs* freeOp = *opPtr;
/* 中文逐项-变量 V3648：next
 * next（struct ncclProxyArgs；函数 removeOp 内）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 初值/来源 next = freeOp->next；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyArgs* next = freeOp->next;
  DEBUG_PROXY_PRINT("Remove %ld -> %ld -> %ld\n", OP_INDEX(*prevOpPtr), OP_INDEX(freeOp), OP_INDEX(next));
  *opPtr = next;
  if (freeOp->nextPeer) {
    // replace op by nextPeer
/* 中文逐项-变量 V3649：nextPeer
 * nextPeer（struct ncclProxyArgs；函数 removeOp 内）：同连接操作的后继链/尾部引用，维持共享连接上的执行顺序。
 * 初值/来源 nextPeer = freeOp->nextPeer；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyArgs* nextPeer = freeOp->nextPeer;
    if (*prevOpPtr) {
      (*prevOpPtr)->next = nextPeer;
    } else {
      state->active = nextPeer;
    }
    nextPeer->next = next;
    *(prevOpPtr) = nextPeer;
  } else {
    *(freeOp->proxyAppendPtr) = NULL;
    if (*prevOpPtr) {
      (*prevOpPtr)->next = next;
    } else {
      state->active = next;
    }
  }
  freeOp->next = state->pool;
  state->pool = freeOp;
  DEBUG_PROXY_PRINT("Removed %5ld (%5ld) : ", OP_INDEX(freeOp), OP_INDEX(*freeOp->proxyAppendPtr));
#ifdef DEBUG_PROXY
  NCCLCHECK(dumpProxyState(state));
#endif
  return ncclSuccess;
}

/* 中文精读：Proxy 是反复调用的非阻塞状态机
 * 遍历 active 操作，调用 args->progress（例如 NET 的 sendProxyProgress/recvProxyProgress）。
 * 每次尽量推进若干阶段后返回；如果请求尚未完成，就保留 args 等下一轮，而非阻塞等待整个张量。
 * 操作进入 None 后从活跃链移除，并推进同连接后继任务，维护连接上的顺序。
 * 这种结构让一个线程交错服务多个 channel/peer，隐藏网络与 GPU 的等待时间。
 */
/* 中文内部逻辑 F1028：progressOps
 * 精读：遍历活跃链，调用每个transport提供的非阻塞progress回调。
 * 精读：未完成项保留，完成项从链中摘除并按同连接先后规则提升下一项；一次回调不要求等整个张量完成。
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyArgs* prevOp = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclProxyArgs* op = opStart;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 ；继续条件 (op)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 条件判断：(op->state == ncclProxyOpNone)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   7. 调用 / 计算 / 写入：TIME_START(0);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   8. 调用 / 计算 / 写入：TIME_START(1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   9. 调用 / 计算 / 写入：NCCLCHECK(op->progress(comm, op));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *   10. 条件判断：(op->idle)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     11. 真分支：；仅条件为真执行以下子项。
 *       12. 调用 / 计算 / 写入：TIME_STOP(1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       13. 调用 / 计算 / 写入：TIME_CANCEL(0);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     14. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       15. 调用 / 计算 / 写入：TIME_CANCEL(1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       16. 调用 / 计算 / 写入：TIME_STOP(0);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   17. 调用 / 计算 / 写入：*idle &= op->idle;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   18. 条件判断：(op->state == ncclProxyOpNone)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     19. 真分支：；仅条件为真执行以下子项。
 *       20. 调用 / 计算 / 写入：TIME_START(2);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       21. 调用 / 计算 / 写入：NCCLCHECK(removeOp(state, &op, &prevOp));；失败时可能提前return，成功才继续；从活跃 Proxy 链移除完成项，提升同连接后继项并归还可复用参数对象。
 *       22. 调用 / 计算 / 写入：TIME_STOP(2);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     23. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       24. 调用 / 计算 / 写入：prevOp = op;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       25. 调用 / 计算 / 写入：op = op->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 26. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 27. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1028
 */
/* 中文逐项-函数 F1028：progressOps
 * 用途与位置：遍历活跃操作并调用各 transport 的非阻塞进度函数，完成后摘链，未完成保留下一轮。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 state（struct ncclProxyProgressState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 参数 opStart（struct ncclProxyArgs* opStart）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 idle（int* idle）：本轮推进是否未取得进展，用于 CPU 让出策略，不代表全体通信已经完成。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；TIME_CANCEL；TIME_START；TIME_STOP；op->progress；removeOp。
 * 需要对照的边界/推进条件：op；op->state == ncclProxyOpNone；op->idle；op->state == ncclProxyOpNone。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t progressOps(struct ncclComm* comm, struct ncclProxyProgressState* state, struct ncclProxyArgs* opStart, int* idle) {
/* 中文逐项-变量 V3650：prevOp
 * prevOp（struct ncclProxyArgs；函数 progressOps 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 prevOp = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclProxyArgs* prevOp = NULL;
/* 中文逐项-变量 V3651：op
 * op（struct ncclProxyArgs；函数 progressOps 内）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 初值/来源 op = opStart；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  struct ncclProxyArgs* op = opStart;
  while (op) {
    if (op->state == ncclProxyOpNone) return ncclInternalError;
    TIME_START(0); TIME_START(1);
    NCCLCHECK(op->progress(comm, op));
    if (op->idle) { TIME_STOP(1); TIME_CANCEL(0); } else { TIME_CANCEL(1); TIME_STOP(0); }
    *idle &= op->idle;
    if (op->state == ncclProxyOpNone) {
      TIME_START(2);
      NCCLCHECK(removeOp(state, &op, &prevOp));
      TIME_STOP(2);
    } else {
      prevOp = op;
      op = op->next;
    }
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1029：ncclProxyGetPostedOps
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyProgressState* state = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(state->opsPool == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：struct ncclProxyOpsPool* pool = state->opsPool;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：struct ncclProxyArgs profArgs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 条件判断：(state->nextOps != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 控制转移：goto process_nextops;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 10. 条件判断：(state->active != NULL && (pool->nextOps == -1 || pthread_mutex_trylock(&pool->mutex) != 0))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 13. 条件判断：(state->active == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 调用 / 计算 / 写入：pthread_mutex_lock(&pool->mutex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     16. 循环：初始 ；继续条件 (pool->nextOps == -1 && !state->stop)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       17. 准备局部状态：struct ncclProxyArgs profArgs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       18. 调用 / 计算 / 写入：ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileSleep);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *       19. 调用 / 计算 / 写入：pthread_cond_wait(&pool->cond, &pool->mutex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       20. 调用 / 计算 / 写入：ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileWakeup);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *     21. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     22. 条件判断：(state->stop)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       23. 真分支：；仅条件为真执行以下子项。
 *         24. 调用 / 计算 / 写入：pthread_mutex_unlock(&pool->mutex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         25. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 26. 调用 / 计算 / 写入：state->nextOps = pool->nextOps;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 27. 调用 / 计算 / 写入：pool->nextOps = pool->nextOpsEnd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 28. 调用 / 计算 / 写入：pthread_mutex_unlock(&pool->mutex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 29. 条件判断：(state->nextOps == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   30. 真分支：；仅条件为真执行以下子项。
 *     31. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 32. 跳转标签：process_nextops；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   33. 调用 / 计算 / 写入：ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileAppend);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 * 34. 调用 / 计算 / 写入：TIME_START(2);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 35. 准备局部状态：int freeOp[NCCL_MAX_LOCAL_RANKS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 36. 准备局部状态：int freeOpEnd[NCCL_MAX_LOCAL_RANKS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 37. 循环：初始 int i=0;；继续条件 i<comm->localRanks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   38. 调用 / 计算 / 写入：freeOp[i] = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 39. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 40. 循环：初始 int opIndex = state->nextOps;；继续条件 opIndex != -1；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   41. 准备局部状态：struct ncclProxyOp* peerOp = pool->ops+opIndex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   42. 准备局部状态：int peer = opIndex / MAX_OPS_PER_PEER;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   43. 条件判断：(peerOp->connection == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     44. 真分支：；仅条件为真执行以下子项。
 *       45. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   46. 条件判断：(peerOp->next != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     47. 真分支：；仅条件为真执行以下子项。
 *       48. 调用 / 计算 / 写入：__builtin_prefetch(pool->ops+peerOp->next);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   49. 调用 / 计算 / 写入：NCCLCHECK(ProxyAppend(state, peerOp));；失败时可能提前return，成功才继续；把提交操作追加/聚合到活跃链或同连接后继链，维持共享连接上的先后顺序。
 *   50. 调用 / 计算 / 写入：(*added)++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   51. 准备局部状态：int lastOpIndex = opIndex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   52. 调用 / 计算 / 写入：opIndex = peerOp->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   53. 条件判断：(freeOp[peer] == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     54. 真分支：；仅条件为真执行以下子项。
 *       55. 调用 / 计算 / 写入：freeOpEnd[peer] = lastOpIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     56. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       57. 调用 / 计算 / 写入：peerOp->next = freeOp[peer];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   58. 调用 / 计算 / 写入：freeOp[peer] = lastOpIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   59. 调用 / 计算 / 写入：state->nextOps = opIndex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 60. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 61. 循环：初始 int i=0;；继续条件 i<comm->localRanks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   62. 条件判断：(freeOp[i] == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     63. 真分支：；仅条件为真执行以下子项。
 *       64. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   65. 准备局部状态：int newFree = freeOp[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   66. 准备局部状态：int oldFree = pool->freeOps[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   67. 调用 / 计算 / 写入：pool->ops[freeOpEnd[i]].next = oldFree;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   68. 条件判断：(oldFree == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     69. 真分支：；仅条件为真执行以下子项。
 *       70. 调用 / 计算 / 写入：pool->freeOps[i] = newFree;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     71. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       72. 准备局部状态：int swap = __sync_val_compare_and_swap(pool->freeOps+i, oldFree, newFree);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       73. 条件判断：(swap != oldFree)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         74. 真分支：；仅条件为真执行以下子项。
 *           75. 条件判断：(swap != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             76. 真分支：；仅条件为真执行以下子项。
 *               77. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *           78. 调用 / 计算 / 写入：pool->ops[freeOpEnd[i]].next = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           79. 调用 / 计算 / 写入：pool->freeOps[i] = newFree;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 80. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 81. 调用 / 计算 / 写入：profArgs.opCount = *added;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 82. 调用 / 计算 / 写入：ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileAppendEnd);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 * 83. 调用 / 计算 / 写入：TIME_STOP(2);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 84. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1029
 */
/* 中文逐项-函数 F1029：ncclProxyGetPostedOps
 * 用途与位置：从共享池取走已发布工作，转换/聚合成运行态队列；无任务时可在条件变量上等待。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 added（int* added）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；ProxyAppend；TIME_START；TIME_STOP；__builtin_prefetch；__sync_val_compare_and_swap；ncclProfilingRecord；pthread_cond_wait；pthread_mutex_lock；pthread_mutex_trylock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：state->opsPool == NULL；state->nextOps != -1；state->active != NULL && (pool->nextOps == -1 ||
 * pthread_mutex_trylock(&pool->mutex) != 0；state->active == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
static ncclResult_t ncclProxyGetPostedOps(struct ncclComm* comm, int* added) {
/* 中文逐项-变量 V3652：state
 * state（struct ncclProxyProgressState；函数 ncclProxyGetPostedOps 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (state->opsPool == NULL) return ncclInternalError;
/* 中文逐项-变量 V3653：pool
 * pool（struct ncclProxyOpsPool；函数 ncclProxyGetPostedOps 内）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 初值/来源 pool = state->opsPool；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyOpsPool* pool = state->opsPool;

/* 中文逐项-变量 V3654：profArgs
 * profArgs（struct ncclProxyArgs；函数 ncclProxyGetPostedOps 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：struct ncclProxyArgs profArgs;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyArgs profArgs; // Only used for profiling purposes
  if (state->nextOps != -1) goto process_nextops;

  // If we have ops to progress, no need to block waiting for something to arrive or even wait for the lock
  // to be available. Exit, continue progress, and come back later.
  if (state->active != NULL && (pool->nextOps == -1 || pthread_mutex_trylock(&pool->mutex) != 0)) return ncclSuccess;

  if (state->active == NULL) {
    pthread_mutex_lock(&pool->mutex);
    while (pool->nextOps == -1 && !state->stop) {
/* 中文逐项-变量 V3655：profArgs
 * profArgs（struct ncclProxyArgs；函数 ncclProxyGetPostedOps 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileSleep);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      struct ncclProxyArgs profArgs; // Only used for profiling purposes
      ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileSleep);
      pthread_cond_wait(&pool->cond, &pool->mutex);
      ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileWakeup);
    }
    if (state->stop) { // We might have been woken up to stop.
      pthread_mutex_unlock(&pool->mutex);
      return ncclSuccess;
    }
  }

  state->nextOps = pool->nextOps;
  pool->nextOps = pool->nextOpsEnd = -1;
  pthread_mutex_unlock(&pool->mutex);
  if (state->nextOps == -1) return ncclInternalError;

process_nextops:
  ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileAppend);
  TIME_START(2);
/* 中文逐项-变量 V3656：freeOp
 * freeOp（int；函数 ncclProxyGetPostedOps 内）：共享任务池已发布/待发布链的头尾和空闲索引，靠锁/原子维护。
 * 此处未显式给初值；后续首个文本使用点：for (int i=0; i<comm->localRanks; i++) freeOp[i] = -1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int freeOp[NCCL_MAX_LOCAL_RANKS];
/* 中文逐项-变量 V3657：freeOpEnd
 * freeOpEnd（int；函数 ncclProxyGetPostedOps 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：freeOpEnd[peer] = lastOpIndex;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int freeOpEnd[NCCL_MAX_LOCAL_RANKS];
  for (/* 中文逐项-变量 V3658：i
 * i（int；函数 ncclProxyGetPostedOps 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->localRanks; i++) freeOp[i] = -1;

  for (/* 中文逐项-变量 V3659：opIndex
 * opIndex（int；函数 ncclProxyGetPostedOps 内）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 * 初值/来源 opIndex = state->nextOps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 * 所在for循环的继续条件：opIndex != -1；循环体与增量表达式决定每轮如何推进。
 */int opIndex = state->nextOps; opIndex != -1;) {
/* 中文逐项-变量 V3660：peerOp
 * peerOp（struct ncclProxyOp；函数 ncclProxyGetPostedOps 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 peerOp = pool->ops+opIndex；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyOp* peerOp = pool->ops+opIndex;
/* 中文逐项-变量 V3661：peer
 * peer（int；函数 ncclProxyGetPostedOps 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = opIndex / MAX_OPS_PER_PEER；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int peer = opIndex / MAX_OPS_PER_PEER;
    if (peerOp->connection == NULL) return ncclInternalError;
    if (peerOp->next != -1) __builtin_prefetch(pool->ops+peerOp->next);
    NCCLCHECK(ProxyAppend(state, peerOp));
    (*added)++;
/* 中文逐项-变量 V3662：lastOpIndex
 * lastOpIndex（int；函数 ncclProxyGetPostedOps 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lastOpIndex = opIndex；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int lastOpIndex = opIndex;
    opIndex = peerOp->next;
    // Return op to peer pool
    if (freeOp[peer] == -1) {
      freeOpEnd[peer] = lastOpIndex;
    } else {
      peerOp->next = freeOp[peer];
    }
    freeOp[peer] = lastOpIndex;
    state->nextOps = opIndex;
  }

  for (/* 中文逐项-变量 V3663：i
 * i（int；函数 ncclProxyGetPostedOps 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->localRanks; i++) {
    if (freeOp[i] == -1) continue;
/* 中文逐项-变量 V3664：newFree
 * newFree（int；函数 ncclProxyGetPostedOps 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 newFree = freeOp[i]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int newFree = freeOp[i];
/* 中文逐项-变量 V3665：oldFree
 * oldFree（int；函数 ncclProxyGetPostedOps 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 oldFree = pool->freeOps[i]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int oldFree = pool->freeOps[i];
    pool->ops[freeOpEnd[i]].next = oldFree;
    if (oldFree == -1) {
      // Nothing for the main thread to consume, we can set it.
      pool->freeOps[i] = newFree;
    } else {
      // The main thread may recycle free ops at any time, replace the freeOps value atomically and check it worked.
/* 中文逐项-变量 V3666：swap
 * swap（int；函数 ncclProxyGetPostedOps 内）：RDMA原子比较交换/加法请求的操作数，本版IB普通数据传输不据此自动执行张量归约。
 * 初值/来源 swap = __sync_val_compare_and_swap(pool->freeOps+i, oldFree, newFree)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      int swap = __sync_val_compare_and_swap(pool->freeOps+i, oldFree, newFree);
      if (swap != oldFree) {
        if (swap != -1) return ncclInternalError;
        // Ops were recycled while we were trying to swap, just set the value directly now.
        pool->ops[freeOpEnd[i]].next = -1;
        pool->freeOps[i] = newFree;
      }
    }
  }
  profArgs.opCount = *added;
  ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileAppendEnd);
  TIME_STOP(2);
  return ncclSuccess;
}

#include <signal.h>
/* 中文逐项-变量 V3667：ncclLastProxyState
 * ncclLastProxyState（ncclProxyProgressState；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：dumpProxyState(ncclLastProxyState);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static ncclProxyProgressState* ncclLastProxyState;
/* 中文内部逻辑 F1030：ncclDumpProxyState
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：dumpProxyState(ncclLastProxyState);；打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1030
 */
/* 中文逐项-函数 F1030：ncclDumpProxyState
 * 用途与位置：打印代理活跃队列和各 sub 的 posted/transmitted/done 等进度，辅助定位等待阶段。
 * 参数 signal（int signal）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：dumpProxyState。
 */
void ncclDumpProxyState(int signal) {
  dumpProxyState(ncclLastProxyState);
}

/* 中文导读：
 * 数据面后台推进循环：从共享任务池取得 ProxyOp，转成运行参数并轮询 active 操作的 progress 回调。
 * 网络传输尚未完成时继续推进；空闲时让出 CPU。它不负责执行 GPU 上的逐元素归约。
 */
/* 中文精读：图示绑定 V3.3 F1031 ncclProxyProgress
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F1031
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0096] ncclProxyProgress：ProxyStart发布已有队列，progress循环调用具体transport回调；收发图对应NET transport数据面，不代表每个本地P2P数据块也走网络代理（调度上下文）
 *   图：01-submit, 21-proxy-send, 22-proxy-recv
 *   定位：#symbol=F1031&binding=B0096
 */
/* 中文内部逻辑 F1031：ncclProxyProgress
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclComm* comm = (struct ncclComm*)comm_;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclProxyProgressState* state = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：state->nextOps = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：signal(SIGUSR1, ncclDumpProxyState);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 调用 / 计算 / 写入：ncclLastProxyState = state;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 准备局部状态：char threadName[NCCL_THREAD_NAMELEN];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：snprintf(threadName, NCCL_THREAD_NAMELEN, "NCCL Progress%2d", comm->cudaDev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 调用 / 计算 / 写入：nvtxNameOsThreadA(syscall(SYS_gettid), threadName);；为该API名字指定的类别、线程、CUDA/OpenCL资源登记可读名字，A/W分别接收窄/宽字符串。这些调用只提供观测标记，不构成CUDA stream同步，也不负责NCCL通信进度。
 * 9. 准备局部状态：int lastIdle = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 准备局部状态：struct ncclProxyArgs profArgs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 循环：初始 ；继续条件 (state->stop == 0 && *comm->abortFlag == 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   12. 准备局部状态：int idle = 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   13. 准备局部状态：ncclResult_t ret = progressOps(comm, state, state->active, &idle);；遍历活跃操作并调用各 transport 的非阻塞进度函数，完成后摘链，未完成保留下一轮。
 *   14. 条件判断：(ret != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     15. 真分支：；仅条件为真执行以下子项。
 *       16. 调用 / 计算 / 写入：comm->fatalError = ret;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       17. 调用 / 计算 / 写入：INFO(NCCL_ALL,"%s:%d -> %d [Proxy Thread]", __FILE__, __LINE__, ret);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       18. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   19. 条件判断：(lastIdle == 0 && idle == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     20. 真分支：；仅条件为真执行以下子项。
 *       21. 调用 / 计算 / 写入：ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileIdle);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *   22. 条件判断：(lastIdle == 1 && idle == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     23. 真分支：；仅条件为真执行以下子项。
 *       24. 调用 / 计算 / 写入：ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileActive);；若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 *   25. 条件判断：(idle)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     26. 真分支：；仅条件为真执行以下子项。
 *       27. 准备局部状态：int added = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       28. 调用 / 计算 / 写入：TIME_START(3);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       29. 调用 / 计算 / 写入：ret = ncclProxyGetPostedOps(comm, &added);；从共享池取走已发布工作，转换/聚合成运行态队列；无任务时可在条件变量上等待。
 *       30. 条件判断：(added)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         31. 真分支：；仅条件为真执行以下子项。
 *           32. 调用 / 计算 / 写入：TIME_STOP(3);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         33. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           34. 调用 / 计算 / 写入：TIME_CANCEL(3);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       35. 条件判断：(ret != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         36. 真分支：；仅条件为真执行以下子项。
 *           37. 调用 / 计算 / 写入：comm->fatalError = ret;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           38. 调用 / 计算 / 写入：INFO(NCCL_ALL,"%s:%d -> %d [Proxy Thread]", __FILE__, __LINE__, ret);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       39. 条件判断：(added == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         40. 真分支：；仅条件为真执行以下子项。
 *           41. 调用 / 计算 / 写入：sched_yield();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   42. 调用 / 计算 / 写入：lastIdle = idle;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 43. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 44. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1031
 */
/* 中文逐项-函数 F1031：ncclProxyProgress
 * 用途与位置：代理数据面后台循环，交错推进多个 channel/peer 的网络请求并处理退出/异步错误。
 * 参数 comm_（void *comm_）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：TIME_CANCEL；TIME_START；TIME_STOP；ncclProfilingRecord；ncclProxyGetPostedOps；nvtxNameOsThreadA；progressOps；sched_yield；signal；snprintf；syscall。
 * 需要对照的边界/推进条件：state->stop == 0 && *comm->abortFlag == 0；ret != ncclSuccess；lastIdle == 0 && idle == 1；lastIdle == 1
 * && idle == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;。CHECK类宏还可能提前返回/跳转。
 */
void* ncclProxyProgress(void *comm_) {
/* 中文逐项-变量 V3668：comm
 * comm（struct ncclComm；函数 ncclProxyProgress 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclComm*)comm_；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclComm* comm = (struct ncclComm*)comm_;
/* 中文逐项-变量 V3669：state
 * state（struct ncclProxyProgressState；函数 ncclProxyProgress 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  state->nextOps = -1;
  signal(SIGUSR1, ncclDumpProxyState);
  ncclLastProxyState = state;
/* 中文逐项-变量 V3670：threadName
 * threadName（char；函数 ncclProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：snprintf(threadName, NCCL_THREAD_NAMELEN, "NCCL Progress%2d",
 * comm->cudaDev);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char threadName[NCCL_THREAD_NAMELEN];
  snprintf(threadName, NCCL_THREAD_NAMELEN, "NCCL Progress%2d", comm->cudaDev);
  nvtxNameOsThreadA(syscall(SYS_gettid), threadName);

/* 中文逐项-变量 V3671：lastIdle
 * lastIdle（int；函数 ncclProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lastIdle = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int lastIdle = 0;
/* 中文逐项-变量 V3672：profArgs
 * profArgs（struct ncclProxyArgs；函数 ncclProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (lastIdle == 0 && idle == 1) ncclProfilingRecord(&profArgs, 0, 0,
 * ncclProxyProfileIdle);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyArgs profArgs; // Only used for profiling purposes
  while (state->stop == 0 && *comm->abortFlag == 0) {
/* 中文逐项-变量 V3673：idle
 * idle（int；函数 ncclProxyProgress 内）：本轮推进是否未取得进展，用于 CPU 让出策略，不代表全体通信已经完成。
 * 初值/来源 idle = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int idle = 1;
/* 中文逐项-变量 V3674：ret
 * ret（ncclResult_t；函数 ncclProxyProgress 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = progressOps(comm, state, state->active, &idle)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    ncclResult_t ret = progressOps(comm, state, state->active, &idle);
    if (ret != ncclSuccess) {
      comm->fatalError = ret;
      INFO(NCCL_ALL,"%s:%d -> %d [Proxy Thread]", __FILE__, __LINE__, ret);
      return NULL;
    }
    if (lastIdle == 0 && idle == 1) ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileIdle);
    if (lastIdle == 1 && idle == 0) ncclProfilingRecord(&profArgs, 0, 0, ncclProxyProfileActive);
    if (idle) {
/* 中文逐项-变量 V3675：added
 * added（int；函数 ncclProxyProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 added = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int added = 0;
      TIME_START(3);
      ret = ncclProxyGetPostedOps(comm, &added);
      if (added) { TIME_STOP(3); } else { TIME_CANCEL(3); }
      if (ret != ncclSuccess) {
        comm->fatalError = ret;
        INFO(NCCL_ALL,"%s:%d -> %d [Proxy Thread]", __FILE__, __LINE__, ret);
      }
      if (added == 0) {
        sched_yield(); // No request progressed. Let others run.
      }
    }
    lastIdle = idle;
  }
  return NULL;
}

/* 中文导读：
 * 将本批已记录的 Proxy 工作发布到对应代理的任务池，让 progress 线程能够开始推进。
 * 它与 GPU FIFO 发布配合；Proxy 工作必须与 GPU 工作使用相同连接、协议和步数。
 */
/* 中文精读：图示绑定 V3.3 F1032 ncclProxyStart
 * 源码旁看图：docs/zh_CN/11_源码注释与图示对照_离线版.html#symbol=F1032
 * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。
 * [B0095] ncclProxyStart：ProxyStart发布已有队列，progress循环调用具体transport回调；收发图对应NET transport数据面，不代表每个本地P2P数据块也走网络代理（调度上下文）
 *   图：01-submit, 21-proxy-send, 22-proxy-recv
 *   定位：#symbol=F1032&binding=B0095
 */
/* 中文内部逻辑 F1032：ncclProxyStart
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyOps* proxyOps = comm->proxyState.proxyOps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(proxyOps == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 调用 / 计算 / 写入：TIME_START(1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 循环：初始 int r=0;；继续条件 r<comm->localRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 准备局部状态：struct ncclProxyOps* ops = proxyOps+r;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 条件判断：(ops->pool == NULL || ops->nextOps == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     9. 真分支：；仅条件为真执行以下子项。
 *       10. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   11. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyPost(ops->pool, ops->nextOps, ops->nextOpsEnd));；失败时可能提前return，成功才继续；在锁内发布任务索引链，队列从空转为非空时通过条件变量唤醒 Progress 线程。
 *   12. 调用 / 计算 / 写入：ops->nextOps = ops->nextOpsEnd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   13. 调用 / 计算 / 写入：ops->count = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 15. 调用 / 计算 / 写入：comm->opCount++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 调用 / 计算 / 写入：TIME_STOP(1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 17. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1032
 */
/* 中文逐项-函数 F1032：ncclProxyStart
 * 用途与位置：发布本批累积代理工作到相关本地代理的池，递增操作编号而不新建每消息线程。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclLaunchProxy。
 * 本体直接出现的调用/包装：NCCLCHECK；TIME_START；TIME_STOP；ncclProxyPost。
 * 需要对照的边界/推进条件：proxyOps == NULL；r<comm->localRanks；ops->pool == NULL || ops->nextOps == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyStart(struct ncclComm* comm) {
/* 中文逐项-变量 V3676：proxyOps
 * proxyOps（struct ncclProxyOps；函数 ncclProxyStart 内）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 初值/来源 proxyOps = comm->proxyState.proxyOps；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclProxyOps* proxyOps = comm->proxyState.proxyOps;
  if (proxyOps == NULL) return ncclSuccess;
  TIME_START(1);
  for (/* 中文逐项-变量 V3677：r
 * r（int；函数 ncclProxyStart 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->localRanks; r++) {
/* 中文逐项-变量 V3678：ops
 * ops（struct ncclProxyOps；函数 ncclProxyStart 内）：所处类型的provider函数表、代理待办数组或任务链，按声明类型读取，不能跨类型强转解释。
 * 初值/来源 ops = proxyOps+r；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclProxyOps* ops = proxyOps+r;
    if (ops->pool == NULL || ops->nextOps == -1) continue;
    NCCLCHECK(ncclProxyPost(ops->pool, ops->nextOps, ops->nextOpsEnd));
    ops->nextOps = ops->nextOpsEnd = -1;
    ops->count = 0;
  }
  comm->opCount++;
  TIME_STOP(1);
  return ncclSuccess;
}

/* 中文内部逻辑 F1033：ncclProxyProgressCreate
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyProgressState* state = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(!state->thread)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：pthread_create(&state->thread, NULL, ncclProxyProgress, comm);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 调用 / 计算 / 写入：ncclSetThreadName(state->thread, "NCCL Progress%2d", comm->cudaDev);；为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1033
 */
/* 中文逐项-函数 F1033：ncclProxyProgressCreate
 * 用途与位置：按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::proxyProgressInit。
 * 本体直接出现的调用/包装：ncclSetThreadName；pthread_create。
 * 需要对照的边界/推进条件：!state->thread。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyProgressCreate(struct ncclComm* comm) {
/* 中文逐项-变量 V3679：state
 * state（struct ncclProxyProgressState；函数 ncclProxyProgressCreate 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (!state->thread) {
    pthread_create(&state->thread, NULL, ncclProxyProgress, comm);
    ncclSetThreadName(state->thread, "NCCL Progress%2d", comm->cudaDev);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1034：ncclProxyProgressDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyProgressState* state = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(state->opsPool)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：pthread_mutex_lock(&state->opsPool->mutex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 调用 / 计算 / 写入：state->stop = true;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：pthread_cond_signal(&state->opsPool->cond);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     7. 调用 / 计算 / 写入：pthread_mutex_unlock(&state->opsPool->mutex);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     8. 调用 / 计算 / 写入：pthread_join(state->thread, NULL);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 循环：初始 ；继续条件 (state->pools != NULL)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   10. 准备局部状态：struct ncclProxyPool *next = state->pools->next;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   11. 调用 / 计算 / 写入：free(state->pools);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   12. 调用 / 计算 / 写入：state->pools = next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 14. 调用 / 计算 / 写入：ncclProfilingDump();；导出代理阶段耗时记录，帮助区分 GPU 等待、网络等待和 flush；禁用分支无操作。
 * 15. 调用 / 计算 / 写入：TIME_PRINT("Proxy");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1034
 */
/* 中文逐项-函数 F1034：ncclProxyProgressDestroy
 * 用途与位置：通知数据面线程停止并唤醒等待者，join 后回收共享任务和运行状态。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyService。
 * 本体直接出现的调用/包装：TIME_PRINT；free；ncclProfilingDump；pthread_cond_signal；pthread_join；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：state->opsPool；state->pools != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclProxyProgressDestroy(struct ncclComm* comm) {
/* 中文逐项-变量 V3680：state
 * state（struct ncclProxyProgressState；函数 ncclProxyProgressDestroy 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;

  // Request the proxy to stop and then wake it
  if (state->opsPool) {
    pthread_mutex_lock(&state->opsPool->mutex);
    state->stop = true;
    pthread_cond_signal(&state->opsPool->cond);
    pthread_mutex_unlock(&state->opsPool->mutex);
    pthread_join(state->thread, NULL);
  }

  // Free off any memory allocated for the proxy arg pools
  while (state->pools != NULL) {
/* 中文逐项-变量 V3681：next
 * next（struct ncclProxyPool；函数 ncclProxyProgressDestroy 内）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 初值/来源 next = state->pools->next；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyPool *next = state->pools->next;
    free(state->pools);
    state->pools = next;
  }

  ncclProfilingDump();
  TIME_PRINT("Proxy");
  return ncclSuccess;
}

/* 中文逐项-类型 T0402：ncclProxyAsyncOp
 * 类型职责：未完成代理控制请求及其发起peer状态，支持setup/connect回调多轮异步完成。
 * 声明内可见成员（含嵌套成员）：type、connection、reqSize、respSize、reqBuff、respBuff。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyAsyncOp {
/* 中文逐项-变量 V3682：type
 * type（int；成员 ncclProxyAsyncOp）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  int type;
/* 中文逐项-变量 V3683：connection
 * connection（struct ncclProxyConnection；成员 ncclProxyAsyncOp）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 */
  struct ncclProxyConnection* connection;
/* 中文逐项-变量 V3684：reqSize、respSize
 * reqSize（int；成员 ncclProxyAsyncOp）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * respSize（int；成员 ncclProxyAsyncOp）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 */
  int reqSize, respSize;
/* 中文逐项-变量 V3685：reqBuff、respBuff
 * reqBuff（char；成员 ncclProxyAsyncOp）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * respBuff（char；成员 ncclProxyAsyncOp）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 */
  char *reqBuff, *respBuff;
};

/* 中文逐项-类型 T0403：ncclProxyLocalPeer
 * 类型职责：未完成代理控制请求及其发起peer状态，支持setup/connect回调多轮异步完成。
 * 声明内可见成员（含嵌套成员）：sock、localRank、asyncOps。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyLocalPeer {
/* 中文逐项-变量 V3686：sock
 * sock（struct ncclSocket；成员 ncclProxyLocalPeer）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket sock;
/* 中文逐项-变量 V3687：localRank
 * localRank（int；成员 ncclProxyLocalPeer）：本节点内 rank 下标，主要索引本地共享资源。
 */
  int localRank;
/* 中文逐项-变量 V3688：asyncOps
 * asyncOps（struct ncclProxyAsyncOp；成员 ncclProxyLocalPeer）：组内延迟 collective 描述数组及有效条数，GroupEnd 统一选型/生成 work。
 */
  struct ncclProxyAsyncOp asyncOps;
};

/* 中文逐项-常量 X1241：NCCL_PROXY_CONN_POOL_SIZE_POW2
 * 编译期定义：NCCL_PROXY_CONN_POOL_SIZE_POW2 = 7。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROXY_CONN_POOL_SIZE_POW2 7
/* 中文逐项-常量 X1242：NCCL_PROXY_CONN_POOL_SIZE
 * 编译期定义：NCCL_PROXY_CONN_POOL_SIZE = (1<<(NCCL_PROXY_CONN_POOL_SIZE_POW2))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROXY_CONN_POOL_SIZE (1<<(NCCL_PROXY_CONN_POOL_SIZE_POW2))
/* 中文逐项-常量 X1243：NCCL_PROXY_CONN_POOL_MASK
 * 编译期定义：NCCL_PROXY_CONN_POOL_MASK = ((NCCL_PROXY_CONN_POOL_SIZE)-1)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROXY_CONN_POOL_MASK ((NCCL_PROXY_CONN_POOL_SIZE)-1)
/* 中文逐项-类型 T0404：ncclProxyConnectionPool
 * 类型职责：代理实际持有的传输连接资源及其分块存储池，由主线程控制请求索引访问。
 * 声明内可见成员（含嵌套成员）：pools、banks、offset、ops。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyConnectionPool {
/* 中文逐项-变量 V3689：pools
 * pools（struct ncclProxyConnection；成员 ncclProxyConnectionPool）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 */
  struct ncclProxyConnection** pools;
/* 中文逐项-变量 V3690：banks
 * banks（int；成员 ncclProxyConnectionPool）：网络传输内存映射描述，按不同主机/GPU/共享bank定位各控制区和协议缓冲。
 */
  int banks;
/* 中文逐项-变量 V3691：offset
 * offset（int；成员 ncclProxyConnectionPool）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 */
  int offset;
/* 中文逐项-变量 V3692：ops
 * ops（struct ncclProxyAsyncOp；成员 ncclProxyConnectionPool）：所处类型的provider函数表、代理待办数组或任务链，按声明类型读取，不能跨类型强转解释。
 */
  struct ncclProxyAsyncOp* ops;
};

/* 中文内部逻辑 F1035：ncclProxyNewConnection
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(pool->offset == NCCL_PROXY_CONN_POOL_SIZE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：NCCLCHECK(ncclRealloc(&pool->pools, pool->banks, pool->banks+1));；失败时可能提前return，成功才继续；分配更大且尾部清零的 CPU 数组，复制旧元素后释放旧内存，不支持缩小。
 *     4. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(pool->pools+pool->banks, NCCL_PROXY_CONN_POOL_SIZE));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     5. 调用 / 计算 / 写入：pool->banks++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：pool->offset = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：*id = ((pool->banks-1) << NCCL_PROXY_CONN_POOL_SIZE_POW2) + pool->offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：pool->offset++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1035
 */
/* 中文逐项-函数 F1035：ncclProxyNewConnection
 * 用途与位置：在分块连接池分配一个空连接槽，返回可用于控制消息索引的连接编号。
 * 参数 pool（struct ncclProxyConnectionPool* pool）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 参数 id（int* id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::proxyConnInit。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc；ncclRealloc。
 * 需要对照的边界/推进条件：pool->offset == NCCL_PROXY_CONN_POOL_SIZE。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclProxyNewConnection(struct ncclProxyConnectionPool* pool, int* id) {
  if (pool->offset == NCCL_PROXY_CONN_POOL_SIZE) {
    NCCLCHECK(ncclRealloc(&pool->pools, pool->banks, pool->banks+1));
    NCCLCHECK(ncclCalloc(pool->pools+pool->banks, NCCL_PROXY_CONN_POOL_SIZE));
    pool->banks++;
    pool->offset = 0;
  }
  *id = ((pool->banks-1) << NCCL_PROXY_CONN_POOL_SIZE_POW2) + pool->offset;
  pool->offset++;
  return ncclSuccess;
}

/* 中文内部逻辑 F1036：ncclProxyGetConnection
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int bank = id>>NCCL_PROXY_CONN_POOL_SIZE_POW2;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int offset = id&NCCL_PROXY_CONN_POOL_MASK;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：((pool->pools == NULL) || (bank > pool->banks) || (pool->pools[bank] == NULL))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：*conn = pool->pools[bank]+offset;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1036
 */
/* 中文逐项-函数 F1036：ncclProxyGetConnection
 * 用途与位置：按池编号定位已存在代理连接对象，校验池/索引后返回其地址。
 * 参数 pool（struct ncclProxyConnectionPool* pool）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 参数 id（int id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 conn（struct ncclProxyConnection** conn）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::proxyConnInit。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：pool->pools == NULL) || (bank > pool->banks) || (pool->pools[bank] == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclProxyGetConnection(struct ncclProxyConnectionPool* pool, int id, struct ncclProxyConnection** conn) {
/* 中文逐项-变量 V3693：bank
 * bank（int；函数 ncclProxyGetConnection 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bank = id>>NCCL_PROXY_CONN_POOL_SIZE_POW2；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
  int bank = id>>NCCL_PROXY_CONN_POOL_SIZE_POW2;
/* 中文逐项-变量 V3694：offset
 * offset（int；函数 ncclProxyGetConnection 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = id&NCCL_PROXY_CONN_POOL_MASK；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int offset = id&NCCL_PROXY_CONN_POOL_MASK;
  if ((pool->pools == NULL) || (bank > pool->banks) || (pool->pools[bank] == NULL)) return ncclInternalError;
  *conn = pool->pools[bank]+offset;
  return ncclSuccess;
}

/* 中文内部逻辑 F1037：proxyFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(connection->send)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：NCCLCHECK(ncclTransports[connection->transport].send.proxyFree(connection, comm));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *   4. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     5. 调用 / 计算 / 写入：NCCLCHECK(ncclTransports[connection->transport].recv.proxyFree(connection, comm));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1037
 */
/* 中文逐项-函数 F1037：proxyFree
 * 用途与位置：调用连接 transport 的代理资源释放回调，按具体后端回收连接持有的对象。
 * 参数 connection（struct ncclProxyConnection* connection）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyFreeConnections。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclTransports[connection->transport].recv.proxyFree；ncclTransports[connection->transport].send.proxyFree。
 * 需要对照的边界/推进条件：connection->send。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t proxyFree(struct ncclProxyConnection* connection, struct ncclComm* comm) {
  if (connection->send) {
    NCCLCHECK(ncclTransports[connection->transport].send.proxyFree(connection, comm));
  } else {
    NCCLCHECK(ncclTransports[connection->transport].recv.proxyFree(connection, comm));
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1038：ncclProxyFreeConnections
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int b=0;；继续条件 b<pool->banks；每轮更新 b++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 准备局部状态：int max = b == pool->banks-1 ? pool->offset : NCCL_PROXY_CONN_POOL_SIZE;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 循环：初始 int i=0;；继续条件 i<max；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     4. 调用 / 计算 / 写入：NCCLCHECK(proxyFree(pool->pools[b]+i, comm));；失败时可能提前return，成功才继续；调用连接 transport 的代理资源释放回调，按具体后端回收连接持有的对象。
 *   5. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   6. 调用 / 计算 / 写入：free(pool->pools[b]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 调用 / 计算 / 写入：free(pool->pools);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1038
 */
/* 中文逐项-函数 F1038：ncclProxyFreeConnections
 * 用途与位置：遍历代理连接池释放全部已分配连接及池块，避免退出后残留后台资源。
 * 参数 pool（struct ncclProxyConnectionPool* pool）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyService。
 * 本体直接出现的调用/包装：NCCLCHECK；free；proxyFree。
 * 需要对照的边界/推进条件：b<pool->banks；i<max。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclProxyFreeConnections(struct ncclProxyConnectionPool* pool, struct ncclComm* comm) {
  for (/* 中文逐项-变量 V3695：b
 * b（int；函数 ncclProxyFreeConnections 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：b<pool->banks；循环体与增量表达式决定每轮如何推进。
 */int b=0; b<pool->banks; b++) {
/* 中文逐项-变量 V3696：max
 * max（int；函数 ncclProxyFreeConnections 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 max = b == pool->banks-1 ? pool->offset : NCCL_PROXY_CONN_POOL_SIZE；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int max = b == pool->banks-1 ? pool->offset : NCCL_PROXY_CONN_POOL_SIZE;
    for (/* 中文逐项-变量 V3697：i
 * i（int；函数 ncclProxyFreeConnections 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<max；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<max; i++) {
      NCCLCHECK(proxyFree(pool->pools[b]+i, comm));
    }
    free(pool->pools[b]);
  }
  free(pool->pools);
  return ncclSuccess;
}

#include "transport.h"

/* 中文内部逻辑 F1039：ncclProxyConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：proxyConn->connection = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：proxyConn->rank = rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 条件判断：(comm->proxyState.peerSocks == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->proxyState.peerSocks, comm->localRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     6. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->proxyState.proxyOps, comm->localRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&comm->proxyState.sharedDevMems, comm->localRanks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     8. 循环：初始 int r=0;；继续条件 r<comm->localRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       9. 调用 / 计算 / 写入：comm->proxyState.peerSocks[r].fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       10. 调用 / 计算 / 写入：comm->proxyState.peerSocks[r].abortFlag = comm->abortFlag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(ncclTopoGetLocalRank(comm->topo, rank, &proxyConn->localRank));；失败时可能提前return，成功才继续；把全局用户 rank 映射到当前硬件系统内的 GPU 下标/局部位置。
 * 13. 准备局部状态：struct ncclSocket* sock = comm->proxyState.peerSocks+proxyConn->localRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 条件判断：(sock->fd == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 调用 / 计算 / 写入：memcpy(&sock->addr, comm->proxyState.peerAddresses+rank, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     17. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketConnect(sock));；失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 * 18. 准备局部状态：int type = ncclProxyMsgInit;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 19. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(sock, &type, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 20. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(sock, &transport, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 21. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(sock, &send, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 22. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(sock, &comm->localRank, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 23. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &proxyConn->connection, sizeof(void*)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 24. 准备局部状态：struct ncclTransportComm* tcomm = send ? &ncclTransports[transport].send : &ncclTransports[transport].recv;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 25. 条件判断：(tcomm->proxyProgress)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   26. 真分支：；仅条件为真执行以下子项。
 *     27. 准备局部状态：char poolPath[] = "/dev/shm/nccl-XXXXXX";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     28. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, poolPath+sizeof("/dev/shm/nccl-")-1, sizeof("XXXXXX")-1));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 *     29. 准备局部状态：struct ncclProxyOps* proxyOps = comm->proxyState.proxyOps+proxyConn->localRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     30. 条件判断：(proxyOps->pool == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       31. 真分支：；仅条件为真执行以下子项。
 *         32. 调用 / 计算 / 写入：NCCLCHECK(ncclShmOpen(poolPath, sizeof(struct ncclProxyOpsPool), (void**)(&proxyOps->pool), NULL, 0));；失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。
 *         33. 调用 / 计算 / 写入：proxyOps->nextOps = proxyOps->nextOpsEnd = proxyOps->freeOp = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 34. 调用 / 计算 / 写入：INFO(NCCL_NET, "Connection to proxy localRank %d -> connection %p", proxyConn->localRank, proxyConn->connection);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 35. 调用 / 计算 / 写入：proxyConn->comm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 36. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1039
 */
/* 中文逐项-函数 F1039：ncclProxyConnect
 * 用途与位置：联系目标代理服务并申请 transport/方向连接，建立主线程到该代理连接的控制引用。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 transport（int transport）：控制/连接的内部描述或transport类型编号，用于选择正确后台回调。
 * 参数 send（int send）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 proxyConn（struct ncclProxyConnector* proxyConn）：逻辑连接到代理服务的引用，记录代理 rank 和实际后台连接。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank；src/transport/coll_net.cc::sendSetup；src/transport/coll_net.cc::recvSetup；src/transport/net.cc::sendSetup；src/transport/net.cc::recvSetup；src/transport/p2p.cc::p2pSendSetup；src/transport/p2p.cc::p2pRecvSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclCalloc；ncclShmOpen；ncclSocketConnect；ncclSocketRecv；ncclSocketSend；ncclTopoGetLocalRank。
 * 需要对照的边界/推进条件：comm->proxyState.peerSocks == NULL；r<comm->localRanks；sock->fd ==
 * -1；tcomm->proxyProgress。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyConnect(struct ncclComm* comm, int transport, int send, int rank, struct ncclProxyConnector* proxyConn) {
  // Keep one connection per mlocal rank
  proxyConn->connection = NULL;
  proxyConn->rank = rank;
  if (comm->proxyState.peerSocks == NULL) {
    NCCLCHECK(ncclCalloc(&comm->proxyState.peerSocks, comm->localRanks));
    NCCLCHECK(ncclCalloc(&comm->proxyState.proxyOps, comm->localRanks));
    NCCLCHECK(ncclCalloc(&comm->proxyState.sharedDevMems, comm->localRanks));
    for (/* 中文逐项-变量 V3698：r
 * r（int；函数 ncclProxyConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->localRanks; r++) {
      comm->proxyState.peerSocks[r].fd = -1;
      comm->proxyState.peerSocks[r].abortFlag = comm->abortFlag;
    }
  }
  NCCLCHECK(ncclTopoGetLocalRank(comm->topo, rank, &proxyConn->localRank));
/* 中文逐项-变量 V3699：sock
 * sock（struct ncclSocket；函数 ncclProxyConnect 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = comm->proxyState.peerSocks+proxyConn->localRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocket* sock = comm->proxyState.peerSocks+proxyConn->localRank;
  if (sock->fd == -1) {
    memcpy(&sock->addr, comm->proxyState.peerAddresses+rank, sizeof(union ncclSocketAddress));
    NCCLCHECK(ncclSocketConnect(sock));
  }
/* 中文逐项-变量 V3700：type
 * type（int；函数 ncclProxyConnect 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 初值/来源 type = ncclProxyMsgInit；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int type = ncclProxyMsgInit;
  NCCLCHECK(ncclSocketSend(sock, &type, sizeof(int)));
  NCCLCHECK(ncclSocketSend(sock, &transport, sizeof(int)));
  NCCLCHECK(ncclSocketSend(sock, &send, sizeof(int)));
  NCCLCHECK(ncclSocketSend(sock, &comm->localRank, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &proxyConn->connection, sizeof(void*)));
/* 中文逐项-变量 V3701：tcomm
 * tcomm（struct ncclTransportComm；函数 ncclProxyConnect 内）：所选 transport 的回调表，规定建链、释放与代理推进方式。
 * 初值/来源 tcomm = send ? &ncclTransports[transport].send :
 * &ncclTransports[transport].recv；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  struct ncclTransportComm* tcomm = send ? &ncclTransports[transport].send : &ncclTransports[transport].recv;
  // If we need proxy progress, map progress ops
  if (tcomm->proxyProgress) {
/* 中文逐项-变量 V3702：poolPath
 * poolPath（char；函数 ncclProxyConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 poolPath = "/dev/shm/nccl-XXXXXX"；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    char poolPath[] = "/dev/shm/nccl-XXXXXX";
    NCCLCHECK(ncclSocketRecv(sock, poolPath+sizeof("/dev/shm/nccl-")-1, sizeof("XXXXXX")-1));
/* 中文逐项-变量 V3703：proxyOps
 * proxyOps（struct ncclProxyOps；函数 ncclProxyConnect 内）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 初值/来源 proxyOps = comm->proxyState.proxyOps+proxyConn->localRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclProxyOps* proxyOps = comm->proxyState.proxyOps+proxyConn->localRank;
    if (proxyOps->pool == NULL) {
      NCCLCHECK(ncclShmOpen(poolPath, sizeof(struct ncclProxyOpsPool), (void**)(&proxyOps->pool), NULL, 0));
      proxyOps->nextOps = proxyOps->nextOpsEnd = proxyOps->freeOp = -1;
    }
  }
  INFO(NCCL_NET, "Connection to proxy localRank %d -> connection %p", proxyConn->localRank, proxyConn->connection);
  proxyConn->comm = comm;
  return ncclSuccess;
}

/* 中文逐项-变量 V3704：ncclProxyMsgTypeStr
 * ncclProxyMsgTypeStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclProxyMsgTypeStr = { "Unknown", "Init", "SharedInit", "Setup", "Connect", "Start", "Close", "Abort",
 * "Stop" }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
const char* ncclProxyMsgTypeStr[] = { "Unknown", "Init", "SharedInit", "Setup", "Connect", "Start", "Close", "Abort", "Stop" };
/* 中文内部逻辑 F1040：ncclProxyCall
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(proxyConn->comm->proxyState.peerSocks == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：struct ncclSocket* sock = proxyConn->comm->proxyState.peerSocks+proxyConn->localRank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(sock->fd == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 准备局部状态：ncclResult_t ret;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSocketSend(sock, &type, sizeof(int)), ret, error);；失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 10. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSocketSend(sock, &proxyConn->connection, sizeof(void*)), ret, error);；失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 11. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSocketSend(sock, &reqSize, sizeof(int)), ret, error);；失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 12. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSocketSend(sock, &respSize, sizeof(int)), ret, error);；失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 13. 条件判断：(reqSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSocketSend(sock, reqBuff, reqSize), ret, error);；失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 16. 条件判断：(respSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   17. 真分支：；仅条件为真执行以下子项。
 *     18. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSocketRecv(sock, respBuff, respSize), ret, error);；失败时保存错误并跳到清理标签，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 20. 跳转标签：error；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   21. 调用 / 计算 / 写入：WARN("Proxy Call to rank %d failed (%s)", proxyConn->comm->localRankToRank[proxyConn->localRank], ncclProxyMsgTypeStr[type]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 22. 调用 / 计算 / 写入：sock->fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 23. 函数返回：return ret;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1040
 */
/* 中文逐项-函数 F1040：ncclProxyCall
 * 用途与位置：通过代理控制 socket 发送指定请求及参数并接收回复，供 setup/connect/free 等控制操作使用。
 * 参数 proxyConn（struct ncclProxyConnector* proxyConn）：逻辑连接到代理服务的引用，记录代理 rank 和实际后台连接。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 reqBuff（void* reqBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 reqSize（int reqSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respBuff（void* respBuff）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 参数 respSize（int respSize）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank；src/transport/coll_net.cc::sendSetup；src/transport/coll_net.cc::recvSetup；src/transport/coll_net.cc::sendConnect；src/transport/coll_net.cc::recvConnect；src/transport/net.cc::sendSetup；src/transport/net.cc::recvSetup；其余见调用清单。
 * 本体直接出现的调用/包装：NCCLCHECKGOTO；ncclSocketRecv；ncclSocketSend。
 * 需要对照的边界/推进条件：proxyConn->comm->proxyState.peerSocks == NULL；sock->fd == -1；reqSize；respSize。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;；return ret;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyCall(struct ncclProxyConnector* proxyConn, int type, void* reqBuff, int reqSize, void* respBuff, int respSize) {
  if (proxyConn->comm->proxyState.peerSocks == NULL) return ncclInternalError;
/* 中文逐项-变量 V3705：sock
 * sock（struct ncclSocket；函数 ncclProxyCall 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = proxyConn->comm->proxyState.peerSocks+proxyConn->localRank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocket* sock = proxyConn->comm->proxyState.peerSocks+proxyConn->localRank;
  if (sock->fd == -1) return ncclInternalError;
/* 中文逐项-变量 V3706：ret
 * ret（ncclResult_t；函数 ncclProxyCall 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(ncclSocketSend(sock, &type, sizeof(int)), ret, error);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t ret;

  NCCLCHECKGOTO(ncclSocketSend(sock, &type, sizeof(int)), ret, error);
  NCCLCHECKGOTO(ncclSocketSend(sock, &proxyConn->connection, sizeof(void*)), ret, error);
  NCCLCHECKGOTO(ncclSocketSend(sock, &reqSize, sizeof(int)), ret, error);
  NCCLCHECKGOTO(ncclSocketSend(sock, &respSize, sizeof(int)), ret, error);
  if (reqSize) NCCLCHECKGOTO(ncclSocketSend(sock, reqBuff, reqSize), ret, error);
  if (respSize) NCCLCHECKGOTO(ncclSocketRecv(sock, respBuff, respSize), ret, error);
  return ncclSuccess;
error:
  WARN("Proxy Call to rank %d failed (%s)", proxyConn->comm->localRankToRank[proxyConn->localRank], ncclProxyMsgTypeStr[type]);
  sock->fd = -1;
  return ret;
}

/* 中文内部逻辑 F1041：proxyProgressInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyProgressState* state = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(state->opsPool == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 准备局部状态：int size = sizeof(struct ncclProxyOpsPool);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 准备局部状态：struct ncclProxyOpsPool* pool = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     6. 准备局部状态：char shmPath[sizeof("/dev/shm/nccl-XXXXXX")];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     7. 调用 / 计算 / 写入：shmPath[0] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     8. 调用 / 计算 / 写入：NCCLCHECK(ncclShmOpen(shmPath, size, (void**)&pool, NULL, 1));；失败时可能提前return，成功才继续；在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。
 *     9. 调用 / 计算 / 写入：pool->nextOps = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 循环：初始 ；继续条件 (comm->localRanks == 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       11. 调用 / 计算 / 写入：sched_yield();；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     13. 循环：初始 int r=0;；继续条件 r<comm->localRanks；每轮更新 r++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       14. 调用 / 计算 / 写入：pool->freeOps[r] = r*MAX_OPS_PER_PEER;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       15. 循环：初始 int i=0;；继续条件 i<MAX_OPS_PER_PEER-1；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         16. 调用 / 计算 / 写入：pool->ops[r*MAX_OPS_PER_PEER+i].next = r*MAX_OPS_PER_PEER+i+1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       17. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       18. 调用 / 计算 / 写入：pool->ops[(r+1)*MAX_OPS_PER_PEER-1].next = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     19. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     20. 准备局部状态：pthread_mutexattr_t mutexAttr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     21. 调用 / 计算 / 写入：pthread_mutexattr_init(&mutexAttr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     22. 调用 / 计算 / 写入：pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     23. 调用 / 计算 / 写入：pthread_mutex_init(&pool->mutex, &mutexAttr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     24. 准备局部状态：pthread_condattr_t condAttr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     25. 调用 / 计算 / 写入：pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     26. 调用 / 计算 / 写入：pthread_cond_init(&pool->cond, &condAttr);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     27. 调用 / 计算 / 写入：state->opsPool = pool;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     28. 调用 / 计算 / 写入：memcpy(state->opsPoolShmSuffix, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof("XXXXXX")-1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     29. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyProgressCreate(comm));；失败时可能提前return，成功才继续；按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。
 * 30. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1041
 */
/* 中文逐项-函数 F1041：proxyProgressInit
 * 用途与位置：创建/映射共享代理任务池并初始化跨进程锁、条件变量和空闲链，为数据提交准备接口。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::proxyConnInit。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclProxyProgressCreate；ncclShmOpen；pthread_cond_init；pthread_condattr_setpshared；pthread_mutex_init；pthread_mutexattr_init；pthread_mutexattr_setpshared；sched_yield。
 * 需要对照的边界/推进条件：state->opsPool == NULL；comm->localRanks == 0；r<comm->localRanks；i<MAX_OPS_PER_PEER-1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
static ncclResult_t proxyProgressInit(struct ncclComm* comm) {
/* 中文逐项-变量 V3707：state
 * state（struct ncclProxyProgressState；函数 proxyProgressInit 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (state->opsPool == NULL) {
/* 中文逐项-变量 V3708：size
 * size（int；函数 proxyProgressInit 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = sizeof(struct ncclProxyOpsPool)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    int size = sizeof(struct ncclProxyOpsPool);
/* 中文逐项-变量 V3709：pool
 * pool（struct ncclProxyOpsPool；函数 proxyProgressInit 内）：代理待发布任务或可复用参数池，存元数据和索引，不装整个用户张量。
 * 初值/来源 pool = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    struct ncclProxyOpsPool* pool = NULL;

/* 中文逐项-变量 V3710：shmPath
 * shmPath（char；函数 proxyProgressInit 内）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 此处未显式给初值；后续首个文本使用点：shmPath[0] = '\0';。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char shmPath[sizeof("/dev/shm/nccl-XXXXXX")];
    shmPath[0] = '\0';
    NCCLCHECK(ncclShmOpen(shmPath, size, (void**)&pool, NULL, 1));

    // Init pool
    pool->nextOps = -1;

    // The service thread may be launched already but localRanks may not be set yet.
    while (comm->localRanks == 0) sched_yield();

    for (/* 中文逐项-变量 V3711：r
 * r（int；函数 proxyProgressInit 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<comm->localRanks; r++) {
      pool->freeOps[r] = r*MAX_OPS_PER_PEER;
      for (/* 中文逐项-变量 V3712：i
 * i（int；函数 proxyProgressInit 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAX_OPS_PER_PEER-1；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAX_OPS_PER_PEER-1; i++) pool->ops[r*MAX_OPS_PER_PEER+i].next = r*MAX_OPS_PER_PEER+i+1;
      pool->ops[(r+1)*MAX_OPS_PER_PEER-1].next = -1;
    }

    // Setup mutex/cond to work inter-process
/* 中文逐项-变量 V3713：mutexAttr
 * mutexAttr（pthread_mutexattr_t；函数 proxyProgressInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：pthread_mutexattr_init(&mutexAttr);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&pool->mutex, &mutexAttr);
/* 中文逐项-变量 V3714：condAttr
 * condAttr（pthread_condattr_t；函数 proxyProgressInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    pthread_condattr_t condAttr;
    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&pool->cond, &condAttr);
    state->opsPool = pool;

    memcpy(state->opsPoolShmSuffix, shmPath+sizeof("/dev/shm/nccl-")-1, sizeof("XXXXXX")-1);

    // All ops structures are created, we can start the progress thread
    NCCLCHECK(ncclProxyProgressCreate(comm));
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1042：proxyOpsFree
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyProgressState* state = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(ncclShmClose(state->opsPool, NULL, sizeof(struct ncclProxyOpsPool)) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("[Service thread] shm close failed");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1042
 */
/* 中文逐项-函数 F1042：proxyOpsFree
 * 用途与位置：回收当前 communicator 映射/分配的代理任务池视图，不混同于单条数据请求完成。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyService。
 * 本体直接出现的调用/包装：ncclShmClose。
 * 需要对照的边界/推进条件：ncclShmClose(state->opsPool, NULL, sizeof(struct ncclProxyOpsPool)) != ncclSuccess。各条件所属分支见紧邻函数体。
 */
static void proxyOpsFree(struct ncclComm* comm) {
/* 中文逐项-变量 V3715：state
 * state（struct ncclProxyProgressState；函数 proxyOpsFree 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (ncclShmClose(state->opsPool, NULL, sizeof(struct ncclProxyOpsPool)) != ncclSuccess) {
    WARN("[Service thread] shm close failed");
  }
}

/* 中文内部逻辑 F1043：ncclProxyShmUnlink
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyProgressState* state = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(state->opsPool == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：char shmPath[] = "/dev/shm/nccl-XXXXXX";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：memcpy(shmPath+sizeof("/dev/shm/nccl-")-1, state->opsPoolShmSuffix, sizeof("XXXXXX")-1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 7. 条件判断：(ncclShmUnlink(shmPath) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：WARN("[Service thread] shm unlink failed");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1043
 */
/* 中文逐项-函数 F1043：ncclProxyShmUnlink
 * 用途与位置：移除代理共享内存对象名字，已有映射仍按各自引用和退出流程释放。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：memcpy；ncclShmUnlink。
 * 需要对照的边界/推进条件：state->opsPool == NULL；ncclShmUnlink(shmPath) != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyShmUnlink(struct ncclComm* comm) {
/* 中文逐项-变量 V3716：state
 * state（struct ncclProxyProgressState；函数 ncclProxyShmUnlink 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyProgressState* state = &comm->proxyState.progressState;
  if (state->opsPool == NULL) return ncclSuccess;

/* 中文逐项-变量 V3717：shmPath
 * shmPath（char；函数 ncclProxyShmUnlink 内）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 初值/来源 shmPath = "/dev/shm/nccl-XXXXXX"；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  char shmPath[] = "/dev/shm/nccl-XXXXXX";
  memcpy(shmPath+sizeof("/dev/shm/nccl-")-1, state->opsPoolShmSuffix, sizeof("XXXXXX")-1);
  if (ncclShmUnlink(shmPath) != ncclSuccess) {
    WARN("[Service thread] shm unlink failed");
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1044：proxyConnInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocket* sock = &peer->sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int id;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct ncclProxyConnection* connection;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyNewConnection(connectionPool, &id));；失败时可能提前return，成功才继续；在分块连接池分配一个空连接槽，返回可用于控制消息索引的连接编号。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyGetConnection(connectionPool, id, &connection));；失败时可能提前return，成功才继续；按池编号定位已存在代理连接对象，校验池/索引后返回其地址。
 * 6. 调用 / 计算 / 写入：connection->sock = sock;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &connection->transport, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &connection->send, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &peer->localRank, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 10. 调用 / 计算 / 写入：connection->localRank = peer->localRank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(sock, &connection, sizeof(void*)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 12. 调用 / 计算 / 写入：connection->tcomm = connection->send ? &ncclTransports[connection->transport].send : &ncclTransports[connection->transport].recv;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 条件判断：(connection->tcomm->proxyProgress)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 调用 / 计算 / 写入：NCCLCHECK(proxyProgressInit(comm));；失败时可能提前return，成功才继续；创建/映射共享代理任务池并初始化跨进程锁、条件变量和空闲链，为数据提交准备接口。
 *     16. 准备局部状态：struct ncclProxyProgressState* state = &comm->proxyState.progressState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     17. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(sock, state->opsPoolShmSuffix, sizeof("XXXXXX")-1));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 18. 调用 / 计算 / 写入：INFO(NCCL_NET, "New proxy %s connection %d from local rank %d, transport %d", connection->send ? "send":"recv", id, connection->localRank, connection->transport);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1044
 */
/* 中文逐项-函数 F1044：proxyConnInit
 * 用途与位置：处理服务端初始化消息，建立 peer 到连接池中 transport 对象的关联。
 * 参数 peer（struct ncclProxyLocalPeer* peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 connectionPool（struct ncclProxyConnectionPool* connectionPool）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyService。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclProxyGetConnection；ncclProxyNewConnection；ncclSocketRecv；ncclSocketSend；proxyProgressInit。
 * 需要对照的边界/推进条件：connection->tcomm->proxyProgress。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t proxyConnInit(struct ncclProxyLocalPeer* peer, struct ncclProxyConnectionPool* connectionPool, struct ncclComm* comm) {
/* 中文逐项-变量 V3718：sock
 * sock（struct ncclSocket；函数 proxyConnInit 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = &peer->sock；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclSocket* sock = &peer->sock;
/* 中文逐项-变量 V3719：id
 * id（int；函数 proxyConnInit 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclProxyNewConnection(connectionPool, &id));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int id;
/* 中文逐项-变量 V3720：connection
 * connection（struct ncclProxyConnection；函数 proxyConnInit 内）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclProxyGetConnection(connectionPool, id, &connection));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyConnection* connection;
  NCCLCHECK(ncclProxyNewConnection(connectionPool, &id));
  NCCLCHECK(ncclProxyGetConnection(connectionPool, id, &connection));
  connection->sock = sock;
  NCCLCHECK(ncclSocketRecv(sock, &connection->transport, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &connection->send, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &peer->localRank, sizeof(int)));
  connection->localRank = peer->localRank;
  NCCLCHECK(ncclSocketSend(sock, &connection, sizeof(void*)));
  connection->tcomm = connection->send ? &ncclTransports[connection->transport].send : &ncclTransports[connection->transport].recv;
  // If we need proxy progress, let's allocate ops and start the thread
  if (connection->tcomm->proxyProgress) {
    NCCLCHECK(proxyProgressInit(comm));
/* 中文逐项-变量 V3721：state
 * state（struct ncclProxyProgressState；函数 proxyConnInit 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState.progressState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
    struct ncclProxyProgressState* state = &comm->proxyState.progressState;
    NCCLCHECK(ncclSocketSend(sock, state->opsPoolShmSuffix, sizeof("XXXXXX")-1));
  }
  INFO(NCCL_NET, "New proxy %s connection %d from local rank %d, transport %d", connection->send ? "send":"recv", id, connection->localRank, connection->transport);
  return ncclSuccess;
}

/* 中文内部逻辑 F1045：proxyConnSharedInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocket* sock = &peer->sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclProxyConnection* connection;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &connection, sizeof(void*)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 4. 准备局部状态：int reqSize, respSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &reqSize, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &respSize, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 7. 条件判断：(reqSize != sizeof(int) || respSize != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 准备局部状态：int nChannels;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &nChannels, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 12. 条件判断：(connection->tcomm->proxySharedInit)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 调用 / 计算 / 写入：NCCLCHECK(connection->tcomm->proxySharedInit(connection, comm, nChannels));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1045
 */
/* 中文逐项-函数 F1045：proxyConnSharedInit
 * 用途与位置：处理共享资源初始化请求，为该代理和 peer 的 P2P 共享缓冲/队列建立基础容量。
 * 参数 peer（struct ncclProxyLocalPeer* peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 connectionPool（struct ncclProxyConnectionPool* connectionPool）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyService。
 * 本体直接出现的调用/包装：NCCLCHECK；connection->tcomm->proxySharedInit；ncclSocketRecv。
 * 需要对照的边界/推进条件：reqSize != sizeof(int) || respSize != 0；connection->tcomm->proxySharedInit。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t proxyConnSharedInit(struct ncclProxyLocalPeer* peer, struct ncclProxyConnectionPool* connectionPool, struct ncclComm* comm) {
/* 中文逐项-变量 V3722：sock
 * sock（struct ncclSocket；函数 proxyConnSharedInit 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = &peer->sock；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclSocket* sock = &peer->sock;
/* 中文逐项-变量 V3723：connection
 * connection（struct ncclProxyConnection；函数 proxyConnSharedInit 内）：当前传输或代理连接对象；查看其声明类型辨认设备视图与主机后台对象。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketRecv(sock, &connection, sizeof(void*)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyConnection* connection;
  NCCLCHECK(ncclSocketRecv(sock, &connection, sizeof(void*)));
/* 中文逐项-变量 V3724：reqSize、respSize
 * reqSize（int；函数 proxyConnSharedInit 内）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketRecv(sock, &reqSize, sizeof(int)));。若由输出参数填入，必须先检查填充调用成功再读值。
 * respSize（int；函数 proxyConnSharedInit 内）：代理控制请求/回复的字节长度和缓冲；属于建链等控制RPC，不是完整张量。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketRecv(sock, &respSize, sizeof(int)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int reqSize, respSize;
  NCCLCHECK(ncclSocketRecv(sock, &reqSize, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &respSize, sizeof(int)));
  if (reqSize != sizeof(int) || respSize != 0) return ncclInternalError;
/* 中文逐项-变量 V3725：nChannels
 * nChannels（int；函数 proxyConnSharedInit 内）：当前作用域的通道数量；comm 表示配置结果，info/work 表示本次任务使用数，graph 表示搜索结果。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketRecv(sock, &nChannels, sizeof(int)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int nChannels;
  NCCLCHECK(ncclSocketRecv(sock, &nChannels, sizeof(int)));
  if (connection->tcomm->proxySharedInit) NCCLCHECK(connection->tcomm->proxySharedInit(connection, comm, nChannels));
  return ncclSuccess;
}

/* 中文内部逻辑 F1046：proxyProgressAsync
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 2. 条件判断：(op->type == ncclProxyMsgSetup)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：NCCLCHECK(op->connection->tcomm->proxySetup(op->connection, comm, op->reqBuff, op->reqSize, op->respBuff, op->respSize, &done));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *   5. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     6. 条件判断：(op->type == ncclProxyMsgConnect)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       7. 真分支：；仅条件为真执行以下子项。
 *         8. 调用 / 计算 / 写入：NCCLCHECK(op->connection->tcomm->proxyConnect(op->connection, comm, op->reqBuff, op->reqSize, op->respBuff, op->respSize, &done));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *       9. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         10. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 11. 条件判断：(done)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 条件判断：(op->respSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       14. 真分支：；仅条件为真执行以下子项。
 *         15. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(op->connection->sock, op->respBuff, op->respSize));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 *     16. 条件判断：(op->reqBuff)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       17. 真分支：；仅条件为真执行以下子项。
 *         18. 调用 / 计算 / 写入：free(op->reqBuff);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     19. 条件判断：(op->respBuff)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       20. 真分支：；仅条件为真执行以下子项。
 *         21. 调用 / 计算 / 写入：free(op->respBuff);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     22. 调用 / 计算 / 写入：op->reqBuff = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     23. 调用 / 计算 / 写入：op->respBuff = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     24. 调用 / 计算 / 写入：op->type = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     25. 调用 / 计算 / 写入：(*asyncOpCount)--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 26. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1046
 */
/* 中文逐项-函数 F1046：proxyProgressAsync
 * 用途与位置：推进一个尚未完成的代理控制请求，transport 声明 done 后才组织回复并移出待完成集合。
 * 参数 op（struct ncclProxyAsyncOp* op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 asyncOpCount（int* asyncOpCount）：组内延迟 collective 描述数组及有效条数，GroupEnd 统一选型/生成 work。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::proxyConnSetupConnect；src/proxy.cc::ncclProxyService。
 * 本体直接出现的调用/包装：NCCLCHECK；free；ncclSocketSend；op->connection->tcomm->proxyConnect；op->connection->tcomm->proxySetup。
 * 需要对照的边界/推进条件：op->type == ncclProxyMsgSetup；op->type == ncclProxyMsgConnect；done；op->respSize。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t proxyProgressAsync(struct ncclProxyAsyncOp* op, struct ncclComm* comm, int* asyncOpCount) {
/* 中文逐项-变量 V3726：done
 * done（int；函数 proxyProgressAsync 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 初值/来源 done = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int done = 1;
  if (op->type == ncclProxyMsgSetup) {
    NCCLCHECK(op->connection->tcomm->proxySetup(op->connection, comm, op->reqBuff, op->reqSize, op->respBuff, op->respSize, &done));
  } else if (op->type == ncclProxyMsgConnect) {
    NCCLCHECK(op->connection->tcomm->proxyConnect(op->connection, comm, op->reqBuff, op->reqSize, op->respBuff, op->respSize, &done));
  } else return ncclInternalError;
  if (done) {
    if (op->respSize) NCCLCHECK(ncclSocketSend(op->connection->sock, op->respBuff, op->respSize));
    if (op->reqBuff) free(op->reqBuff);
    if (op->respBuff) free(op->respBuff);
    op->reqBuff = NULL;
    op->respBuff = NULL;
    op->type = 0;
    (*asyncOpCount)--;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1047：proxyConnSetupConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocket* sock = &peer->sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclProxyAsyncOp* asyncOp = &peer->asyncOps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：asyncOp->type = type;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &asyncOp->connection, sizeof(void*)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &asyncOp->reqSize, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &asyncOp->respSize, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 7. 条件判断：(asyncOp->reqSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&asyncOp->reqBuff, asyncOp->reqSize));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     10. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, asyncOp->reqBuff, asyncOp->reqSize));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 11. 条件判断：(asyncOp->respSize)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&asyncOp->respBuff, asyncOp->respSize));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 14. 调用 / 计算 / 写入：(*asyncOpCount)++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：NCCLCHECK(proxyProgressAsync(asyncOp, comm, asyncOpCount));；失败时可能提前return，成功才继续；推进一个尚未完成的代理控制请求，transport 声明 done 后才组织回复并移出待完成集合。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1047
 */
/* 中文逐项-函数 F1047：proxyConnSetupConnect
 * 用途与位置：接收 setup/connect 请求参数并开始执行对应 transport 回调，支持回调异步多轮完成。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 peer（struct ncclProxyLocalPeer* peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 connectionPool（struct ncclProxyConnectionPool* connectionPool）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 asyncOpCount（int* asyncOpCount）：组内延迟 collective 描述数组及有效条数，GroupEnd 统一选型/生成 work。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyService。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc；ncclSocketRecv；proxyProgressAsync。
 * 需要对照的边界/推进条件：asyncOp->reqSize；asyncOp->respSize。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t proxyConnSetupConnect(int type, struct ncclProxyLocalPeer* peer, struct ncclProxyConnectionPool* connectionPool, struct ncclComm* comm, int* asyncOpCount) {
/* 中文逐项-变量 V3727：sock
 * sock（struct ncclSocket；函数 proxyConnSetupConnect 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = &peer->sock；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclSocket* sock = &peer->sock;
/* 中文逐项-变量 V3728：asyncOp
 * asyncOp（struct ncclProxyAsyncOp；函数 proxyConnSetupConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 asyncOp = &peer->asyncOps；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyAsyncOp* asyncOp = &peer->asyncOps;
  asyncOp->type = type;
  NCCLCHECK(ncclSocketRecv(sock, &asyncOp->connection, sizeof(void*)));

  NCCLCHECK(ncclSocketRecv(sock, &asyncOp->reqSize, sizeof(int)));
  NCCLCHECK(ncclSocketRecv(sock, &asyncOp->respSize, sizeof(int)));
  if (asyncOp->reqSize) {
    NCCLCHECK(ncclCalloc(&asyncOp->reqBuff, asyncOp->reqSize));
    NCCLCHECK(ncclSocketRecv(sock, asyncOp->reqBuff, asyncOp->reqSize));
  }
  if (asyncOp->respSize) NCCLCHECK(ncclCalloc(&asyncOp->respBuff, asyncOp->respSize));
  (*asyncOpCount)++;
  NCCLCHECK(proxyProgressAsync(asyncOp, comm, asyncOpCount));
  return ncclSuccess;
}

#include <poll.h>

/* 中文导读：
 * 控制面服务循环：处理 Init/Setup/Connect/Close 等请求，并异步重试未完成的建链回调。
 * 与 ncclProxyProgress 的数据面工作分开，因此看到 service 线程存在不代表当前一直有数据传输。
 */
/* 中文精读：服务线程与 Progress 线程是两类工作
 * Service 监听控制 socket，处理连接初始化、共享初始化、setup/connect/free 等请求。
 * Progress 轮询已建立连接上的数据传输；它在需要时创建，不是由每次 API 创建。
 * PXN 等路径可让 GPU 使用同机另一 rank 的代理，因此 ProxyConnector 记录代理 rank/localRank。
 * 排查卡住应分清是在建链控制请求阶段，还是已建链后的数据进度阶段。
 */
/* 中文内部逻辑 F1048：ncclProxyService
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclComm* comm = (struct ncclComm *) _args;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(cudaSetDevice(comm->cudaDev) != cudaSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("[Proxy Service] Failed to set CUDA device %d", comm->cudaDev);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 条件判断：(CPU_COUNT(&comm->cpuAffinity))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 准备局部状态：struct ncclProxyConnectionPool connectionPool;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 调用 / 计算 / 写入：connectionPool.pools = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：connectionPool.banks = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：connectionPool.offset = NCCL_PROXY_CONN_POOL_SIZE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 准备局部状态：struct pollfd pollfds[NCCL_MAX_LOCAL_RANKS+1];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 准备局部状态：struct ncclProxyLocalPeer peers[NCCL_MAX_LOCAL_RANKS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 调用 / 计算 / 写入：memset(&peers, 0, sizeof(struct ncclProxyLocalPeer)*NCCL_MAX_LOCAL_RANKS);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 15. 循环：初始 int s=0;；继续条件 s<NCCL_MAX_LOCAL_RANKS；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   16. 调用 / 计算 / 写入：peers[s].sock.fd = pollfds[s].fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   17. 调用 / 计算 / 写入：pollfds[s].events = POLLHUP|POLLIN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 19. 调用 / 计算 / 写入：pollfds[NCCL_MAX_LOCAL_RANKS].fd = comm->proxyState.listenSock->fd;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 调用 / 计算 / 写入：pollfds[NCCL_MAX_LOCAL_RANKS].events = POLLIN;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 21. 准备局部状态：int maxnpeers = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 22. 准备局部状态：int npeers = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 23. 准备局部状态：int stop = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 24. 准备局部状态：int asyncOpCount = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 25. 循环：初始 ；继续条件 (stop == 0 || (stop == 1 && npeers > 0))；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   26. 条件判断：(int error = poll(pollfds, NCCL_MAX_LOCAL_RANKS+1, asyncOpCount ? 0 : -1) < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     27. 真分支：；仅条件为真执行以下子项。
 *       28. 调用 / 计算 / 写入：WARN("[Proxy Service] Poll failed with error %d", error);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       29. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   30. 条件判断：(pollfds[NCCL_MAX_LOCAL_RANKS].revents)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     31. 真分支：；仅条件为真执行以下子项。
 *       32. 准备局部状态：int s = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       33. 循环：初始 ；继续条件 (s < NCCL_MAX_LOCAL_RANKS && peers[s].sock.fd != -1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         34. 调用 / 计算 / 写入：s++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       35. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       36. 条件判断：(s == NCCL_MAX_LOCAL_RANKS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         37. 真分支：；仅条件为真执行以下子项。
 *           38. 调用 / 计算 / 写入：WARN("[Proxy service] Too many connections (%d max)", NCCL_MAX_LOCAL_RANKS);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           39. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *       40. 条件判断：(maxnpeers < s+1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         41. 真分支：；仅条件为真执行以下子项。
 *           42. 调用 / 计算 / 写入：maxnpeers = s+1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       43. 准备局部状态：struct ncclSocket* sock = &peers[s].sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       44. 条件判断：(ncclSocketAccept(sock, comm->proxyState.listenSock) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         45. 真分支：；仅条件为真执行以下子项。
 *           46. 调用 / 计算 / 写入：WARN("[Service thread] Accept failed %s", strerror(errno));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         47. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           48. 调用 / 计算 / 写入：pollfds[s].fd = sock->fd;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           49. 调用 / 计算 / 写入：npeers++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           50. 调用 / 计算 / 写入：peers[s].localRank = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   51. 循环：初始 int s=0;；继续条件 s<maxnpeers；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     52. 准备局部状态：struct ncclProxyLocalPeer* peer = peers+s;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     53. 准备局部状态：struct ncclSocket* sock = &peer->sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     54. 准备局部状态：struct ncclProxyAsyncOp* op = &peer->asyncOps;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     55. 准备局部状态：int closeConn = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     56. 准备局部状态：int type = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     57. 准备局部状态：ncclResult_t res = ncclSuccess;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     58. 条件判断：(op->type != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       59. 真分支：；仅条件为真执行以下子项。
 *         60. 调用 / 计算 / 写入：res = proxyProgressAsync(op, comm, &asyncOpCount);；推进一个尚未完成的代理控制请求，transport 声明 done 后才组织回复并移出待完成集合。
 *         61. 调用 / 计算 / 写入：type = op->type;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         62. 条件判断：(res != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           63. 真分支：；仅条件为真执行以下子项。
 *             64. 调用 / 计算 / 写入：op->type = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       65. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         66. 条件判断：(pollfds[s].revents & POLLIN)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           67. 真分支：；仅条件为真执行以下子项。
 *             68. 准备局部状态：int closed;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             69. 条件判断：(ncclSocketTryRecv(sock, &type, sizeof(int), &closed) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               70. 真分支：；仅条件为真执行以下子项。
 *                 71. 调用 / 计算 / 写入：WARN("[Service thread] Could not receive type from localRank %d", peer->localRank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 72. 调用 / 计算 / 写入：closeConn = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *               73. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                 74. 条件判断：(closed)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                   75. 真分支：；仅条件为真执行以下子项。
 *                     76. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET, "[Service thread] Connection closed by localRank %d", peer->localRank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                     77. 调用 / 计算 / 写入：closeConn = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                   78. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                     79. 条件判断：(type == ncclProxyMsgAbort)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                       80. 真分支：；仅条件为真执行以下子项。
 *                         81. 调用 / 计算 / 写入：stop = 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                         82. 调用 / 计算 / 写入：closeConn = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                       83. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                         84. 条件判断：(type == ncclProxyMsgStop)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                           85. 真分支：；仅条件为真执行以下子项。
 *                             86. 调用 / 计算 / 写入：stop = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                             87. 调用 / 计算 / 写入：closeConn = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                           88. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                             89. 条件判断：(type == ncclProxyMsgClose)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                               90. 真分支：；仅条件为真执行以下子项。
 *                                 91. 调用 / 计算 / 写入：closeConn = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *                               92. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                                 93. 条件判断：(type == ncclProxyMsgInit)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                                   94. 真分支：；仅条件为真执行以下子项。
 *                                     95. 调用 / 计算 / 写入：res = proxyConnInit(peers+s, &connectionPool, comm);；处理服务端初始化消息，建立 peer 到连接池中 transport 对象的关联。
 *                                   96. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                                     97. 条件判断：(type == ncclProxyMsgSharedInit)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                                       98. 真分支：；仅条件为真执行以下子项。
 *                                         99. 调用 / 计算 / 写入：res = proxyConnSharedInit(peers+s, &connectionPool, comm);；处理共享资源初始化请求，为该代理和 peer 的 P2P 共享缓冲/队列建立基础容量。
 *                                       100. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                                         101. 条件判断：(type == ncclProxyMsgSetup || type == ncclProxyMsgConnect)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *                                           102. 真分支：；仅条件为真执行以下子项。
 *                                             103. 调用 / 计算 / 写入：res = proxyConnSetupConnect(type, peers+s, &connectionPool, comm, &asyncOpCount);；接收 setup/connect 请求参数并开始执行对应 transport 回调，支持回调异步多轮完成。
 *                                           104. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *                                             105. 调用 / 计算 / 写入：WARN("[Service thread] Unknown command %d from localRank %d\n", type, peer->localRank);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                                             106. 调用 / 计算 / 写入：closeConn = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           107. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             108. 条件判断：(pollfds[s].revents & POLLHUP)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               109. 真分支：；仅条件为真执行以下子项。
 *                 110. 调用 / 计算 / 写入：closeConn = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     111. 条件判断：(res != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       112. 真分支：；仅条件为真执行以下子项。
 *         113. 调用 / 计算 / 写入：WARN("[Proxy Service %d] Failed to execute operation %s from rank %d, retcode %d", comm->rank, ncclProxyMsgTypeStr[type], comm->localRankToRank[peer->localRank], res);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         114. 调用 / 计算 / 写入：closeConn = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     115. 条件判断：(closeConn)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       116. 真分支：；仅条件为真执行以下子项。
 *         117. 调用 / 计算 / 写入：close(sock->fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         118. 调用 / 计算 / 写入：sock->fd = pollfds[s].fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         119. 调用 / 计算 / 写入：npeers--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   120. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 121. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 122. 条件判断：(ncclProxyProgressDestroy(comm) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   123. 真分支：；仅条件为真执行以下子项。
 *     124. 调用 / 计算 / 写入：WARN("[Proxy Service] proxyDestroy failed");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 125. 循环：初始 int s=0;；继续条件 s<maxnpeers；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   126. 条件判断：(peers[s].sock.fd != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     127. 真分支：；仅条件为真执行以下子项。
 *       128. 调用 / 计算 / 写入：close(peers[s].sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 129. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 130. 调用 / 计算 / 写入：ncclProxyFreeConnections(&connectionPool, comm);；遍历代理连接池释放全部已分配连接及池块，避免退出后残留后台资源。
 * 131. 调用 / 计算 / 写入：close(comm->proxyState.listenSock->fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 132. 调用 / 计算 / 写入：free(comm->proxyState.listenSock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 133. 调用 / 计算 / 写入：proxyOpsFree(comm);；回收当前 communicator 映射/分配的代理任务池视图，不混同于单条数据请求完成。
 * 134. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1048
 */
/* 中文逐项-函数 F1048：ncclProxyService
 * 用途与位置：监听并处理代理控制 socket 的新连接、初始化、setup/connect/free/停止请求，与数据面循环分工。
 * 参数 _args（void* _args）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CPU_COUNT；close；cudaSetDevice；free；memset；ncclProxyFreeConnections；ncclProxyProgressDestroy；ncclSocketAccept；ncclSocketTryRecv；poll；proxyConnInit；proxyConnSetupConnect；proxyConnSharedInit；proxyOpsFree；proxyProgressAsync；sched_setaffinity；strerror。
 * 需要对照的边界/推进条件：cudaSetDevice(comm->cudaDev) != cudaSuccess；CPU_COUNT(&comm->cpuAffinity；s<NCCL_MAX_LOCAL_RANKS；stop
 * == 0 || (stop == 1 && npeers > 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;。CHECK类宏还可能提前返回/跳转。
 */
void* ncclProxyService(void* _args) {
/* 中文逐项-变量 V3729：comm
 * comm（struct ncclComm；函数 ncclProxyService 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclComm *) _args；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclComm* comm =  (struct ncclComm *) _args;
  if (cudaSetDevice(comm->cudaDev) != cudaSuccess) {
    WARN("[Proxy Service] Failed to set CUDA device %d", comm->cudaDev);
  }
  if (CPU_COUNT(&comm->cpuAffinity)) sched_setaffinity(0, sizeof(cpu_set_t), &comm->cpuAffinity);

  // Prepare poll descriptor
/* 中文逐项-变量 V3730：connectionPool
 * connectionPool（struct ncclProxyConnectionPool；函数 ncclProxyService 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：connectionPool.pools = NULL;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyConnectionPool connectionPool;
  connectionPool.pools = NULL;
  connectionPool.banks = 0;
  connectionPool.offset = NCCL_PROXY_CONN_POOL_SIZE;

/* 中文逐项-变量 V3731：pollfds
 * pollfds（struct pollfd；函数 ncclProxyService 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：peers[s].sock.fd = pollfds[s].fd = -1;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct pollfd pollfds[NCCL_MAX_LOCAL_RANKS+1];
/* 中文逐项-变量 V3732：peers
 * peers（struct ncclProxyLocalPeer；函数 ncclProxyService 内）：按对端 rank 索引的主机/设备连接表；预留槽位不表示已建链。
 * 此处未显式给初值；后续首个文本使用点：memset(&peers, 0, sizeof(struct
 * ncclProxyLocalPeer)*NCCL_MAX_LOCAL_RANKS);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclProxyLocalPeer peers[NCCL_MAX_LOCAL_RANKS];
  memset(&peers, 0, sizeof(struct ncclProxyLocalPeer)*NCCL_MAX_LOCAL_RANKS);
  for (/* 中文逐项-变量 V3733：s
 * s（int；函数 ncclProxyService 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<NCCL_MAX_LOCAL_RANKS；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<NCCL_MAX_LOCAL_RANKS; s++) {
    peers[s].sock.fd = pollfds[s].fd = -1;
    pollfds[s].events = POLLHUP|POLLIN;
  }
  pollfds[NCCL_MAX_LOCAL_RANKS].fd = comm->proxyState.listenSock->fd;
  pollfds[NCCL_MAX_LOCAL_RANKS].events = POLLIN;

/* 中文逐项-变量 V3734：maxnpeers
 * maxnpeers（int；函数 ncclProxyService 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 maxnpeers = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int maxnpeers = 0;
/* 中文逐项-变量 V3735：npeers
 * npeers（int；函数 ncclProxyService 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 npeers = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int npeers = 0;
/* 中文逐项-变量 V3736：stop
 * stop（int；函数 ncclProxyService 内）：辅助/代理线程生命周期状态，停止时仍须唤醒并 join 才能回收其资源。
 * 初值/来源 stop = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int stop = 0;
/* 中文逐项-变量 V3737：asyncOpCount
 * asyncOpCount（int；函数 ncclProxyService 内）：组内延迟 collective 描述数组及有效条数，GroupEnd 统一选型/生成 work。
 * 初值/来源 asyncOpCount = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int asyncOpCount = 0;
  while (stop == 0 || (stop == 1 && npeers > 0)) {
    if (/* 中文逐项-变量 V3738：error
 * error（int；函数 ncclProxyService 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：WARN("[Proxy Service] Poll failed with error %d", error);。若由输出参数填入，必须先检查填充调用成功再读值。
 */int error = poll(pollfds, NCCL_MAX_LOCAL_RANKS+1, asyncOpCount ? 0 : -1) < 0) {
      WARN("[Proxy Service] Poll failed with error %d", error);
      return NULL;
    }
    if (pollfds[NCCL_MAX_LOCAL_RANKS].revents) {
/* 中文逐项-变量 V3739：s
 * s（int；函数 ncclProxyService 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int s = 0;
      while (s < NCCL_MAX_LOCAL_RANKS && peers[s].sock.fd != -1) s++;
      if (s == NCCL_MAX_LOCAL_RANKS) {
        WARN("[Proxy service] Too many connections (%d max)", NCCL_MAX_LOCAL_RANKS);
        return NULL;
      }
      if (maxnpeers < s+1) maxnpeers = s+1;
/* 中文逐项-变量 V3740：sock
 * sock（struct ncclSocket；函数 ncclProxyService 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = &peers[s].sock；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
      struct ncclSocket* sock = &peers[s].sock;
      if (ncclSocketAccept(sock, comm->proxyState.listenSock) != ncclSuccess) {
        WARN("[Service thread] Accept failed %s", strerror(errno));
      } else {
        pollfds[s].fd = sock->fd;
        npeers++;
        peers[s].localRank = -1;
      }
    }
    for (/* 中文逐项-变量 V3741：s
 * s（int；函数 ncclProxyService 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<maxnpeers；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<maxnpeers; s++) {
/* 中文逐项-变量 V3742：peer
 * peer（struct ncclProxyLocalPeer；函数 ncclProxyService 内）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 初值/来源 peer = peers+s；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      struct ncclProxyLocalPeer* peer = peers+s;
/* 中文逐项-变量 V3743：sock
 * sock（struct ncclSocket；函数 ncclProxyService 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = &peer->sock；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
      struct ncclSocket* sock = &peer->sock;
/* 中文逐项-变量 V3744：op
 * op（struct ncclProxyAsyncOp；函数 ncclProxyService 内）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 初值/来源 op = &peer->asyncOps；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
      struct ncclProxyAsyncOp* op = &peer->asyncOps;
/* 中文逐项-变量 V3745：closeConn
 * closeConn（int；函数 ncclProxyService 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 closeConn = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int closeConn = 0;
/* 中文逐项-变量 V3746：type
 * type（int；函数 ncclProxyService 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 初值/来源 type = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int type = 0;
/* 中文逐项-变量 V3747：res
 * res（ncclResult_t；函数 ncclProxyService 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      ncclResult_t res = ncclSuccess;
      if (op->type != 0) {
        res = proxyProgressAsync(op, comm, &asyncOpCount);
        type = op->type;
        if (res != ncclSuccess) op->type = 0;
      } else if (pollfds[s].revents & POLLIN) {
/* 中文逐项-变量 V3748：closed
 * closed（int；函数 ncclProxyService 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclSocketTryRecv(sock, &type, sizeof(int), &closed) != ncclSuccess)
 * {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        int closed;
        if (ncclSocketTryRecv(sock, &type, sizeof(int), &closed) != ncclSuccess) {
          WARN("[Service thread] Could not receive type from localRank %d", peer->localRank);
          closeConn = 1;
        } else if (closed) {
          INFO(NCCL_INIT|NCCL_NET, "[Service thread] Connection closed by localRank %d", peer->localRank);
          closeConn = 1;
        } else {
          if (type == ncclProxyMsgAbort) {
            stop = 2;
            closeConn = 1;
          } else if (type == ncclProxyMsgStop) {
            stop = 1;
            closeConn = 1;
          } else if (type == ncclProxyMsgClose) {
            closeConn = 1;
          } else if (type == ncclProxyMsgInit) {
            res = proxyConnInit(peers+s, &connectionPool, comm);
          } else if (type == ncclProxyMsgSharedInit) {
            res = proxyConnSharedInit(peers+s, &connectionPool, comm);
          } else if (type == ncclProxyMsgSetup || type == ncclProxyMsgConnect) {
            res = proxyConnSetupConnect(type, peers+s, &connectionPool, comm, &asyncOpCount);
          } else {
            WARN("[Service thread] Unknown command %d from localRank %d\n", type, peer->localRank);
            closeConn = 1;
          }
        }
      } else if (pollfds[s].revents & POLLHUP) {
        closeConn = 1;
      } 
      if (res != ncclSuccess) {
        WARN("[Proxy Service %d] Failed to execute operation %s from rank %d, retcode %d", comm->rank, ncclProxyMsgTypeStr[type], comm->localRankToRank[peer->localRank], res);
        closeConn = 1;
      }
      if (closeConn) {
        close(sock->fd);
        sock->fd = pollfds[s].fd = -1;
        npeers--;
      }
    }
  }
  // Wait for all operations to complete and stop progress thread before freeing any resource
  if (ncclProxyProgressDestroy(comm) != ncclSuccess) {
    WARN("[Proxy Service] proxyDestroy failed");
  }
  for (/* 中文逐项-变量 V3749：s
 * s（int；函数 ncclProxyService 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<maxnpeers；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<maxnpeers; s++) {
    if (peers[s].sock.fd != -1) close(peers[s].sock.fd);
  }
  ncclProxyFreeConnections(&connectionPool, comm);
  close(comm->proxyState.listenSock->fd);
  free(comm->proxyState.listenSock);
  proxyOpsFree(comm);
  return NULL;
}

/* 中文内部逻辑 F1049：ncclProxyInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：comm->proxyState.listenSock = sock;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：comm->proxyState.peerAddresses = peerAddresses;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1049
 */
/* 中文逐项-函数 F1049：ncclProxyInit
 * 用途与位置：将 bootstrap 建立的监听 socket 和全组代理地址表接入当前 communicator 的 Proxy 状态。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 peerAddresses（union ncclSocketAddress* peerAddresses）：代理本地peer控制对象、控制socket及引导交换的地址表。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapInit。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyInit(struct ncclComm* comm, struct ncclSocket* sock, union ncclSocketAddress* peerAddresses) {
  comm->proxyState.listenSock = sock;
  comm->proxyState.peerAddresses = peerAddresses;
  return ncclSuccess;
}

/* 中文内部逻辑 F1050：ncclProxyCreate
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：pthread_create(&comm->proxyState.thread, NULL, ncclProxyService, comm);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 2. 调用 / 计算 / 写入：ncclSetThreadName(comm->proxyState.thread, "NCCL Service %2d", comm->cudaDev);；为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1050
 */
/* 中文逐项-函数 F1050：ncclProxyCreate
 * 用途与位置：按需要创建并命名代理后台线程，线程生命周期覆盖多次通信调用。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：ncclSetThreadName；pthread_create。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyCreate(struct ncclComm* comm) {
  // comm->proxyState.thread is pthread_join()'d by commFree() in init.cc
  pthread_create(&comm->proxyState.thread, NULL, ncclProxyService, comm);
  ncclSetThreadName(comm->proxyState.thread, "NCCL Service %2d", comm->cudaDev);
  return ncclSuccess;
}

/* 中文内部逻辑 F1051：ncclProxyDestroy
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclProxyState* state = &comm->proxyState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(state->peerAddresses)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 准备局部状态：struct ncclSocket sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 调用 / 计算 / 写入：sock.abortFlag = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：sock.asyncFlag = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：memcpy(&sock.addr, comm->proxyState.peerAddresses+comm->rank, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     8. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketConnect(&sock));；失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 *     9. 准备局部状态：int type = (*comm->abortFlag) ? ncclProxyMsgAbort : ncclProxyMsgStop;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     10. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(&sock, &type, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 *     11. 调用 / 计算 / 写入：close(sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     12. 调用 / 计算 / 写入：free(state->peerAddresses);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 条件判断：(state->peerSocks)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   14. 真分支：；仅条件为真执行以下子项。
 *     15. 循环：初始 int i=0;；继续条件 i<comm->localRanks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       16. 条件判断：(state->peerSocks[i].fd != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         17. 真分支：；仅条件为真执行以下子项。
 *           18. 条件判断：(state->proxyOps[i].pool)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             19. 真分支：；仅条件为真执行以下子项。
 *               20. 调用 / 计算 / 写入：NCCLCHECK(ncclShmClose(state->proxyOps[i].pool, NULL, sizeof(struct ncclProxyOpsPool)));；失败时可能提前return，成功才继续；先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。
 *           21. 条件判断：(state->sharedDevMems[i])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             22. 真分支：；仅条件为真执行以下子项。
 *               23. 调用 / 计算 / 写入：CUDACHECK(cudaIpcCloseMemHandle(state->sharedDevMems[i]));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *           24. 准备局部状态：int type = ncclProxyMsgClose;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           25. 条件判断：(*comm->abortFlag == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             26. 真分支：；仅条件为真执行以下子项。
 *               27. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(state->peerSocks+i, &type, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 *           28. 调用 / 计算 / 写入：close(state->peerSocks[i].fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     29. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     30. 调用 / 计算 / 写入：free(state->peerSocks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     31. 调用 / 计算 / 写入：free(state->proxyOps);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     32. 调用 / 计算 / 写入：free(state->sharedDevMems);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 33. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__proxy_cc.md#f1051
 */
/* 中文逐项-函数 F1051：ncclProxyDestroy
 * 用途与位置：向代理服务发送停止/关闭信号并等待服务线程结束，完成代理总生命周期清理。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；close；cudaIpcCloseMemHandle；free；memcpy；ncclShmClose；ncclSocketConnect；ncclSocketSend。
 * 需要对照的边界/推进条件：state->peerAddresses；state->peerSocks；i<comm->localRanks；state->peerSocks[i].fd != -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProxyDestroy(struct ncclComm* comm) {
/* 中文逐项-变量 V3750：state
 * state（struct ncclProxyState；函数 ncclProxyDestroy 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = &comm->proxyState；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclProxyState* state = &comm->proxyState;
  if (state->peerAddresses) {
/* 中文逐项-变量 V3751：sock
 * sock（struct ncclSocket；函数 ncclProxyDestroy 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 此处未显式给初值；后续首个文本使用点：sock.abortFlag = NULL;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclSocket sock;
    sock.abortFlag = NULL;
    sock.asyncFlag = 0;
    memcpy(&sock.addr, comm->proxyState.peerAddresses+comm->rank, sizeof(union ncclSocketAddress));
    NCCLCHECK(ncclSocketConnect(&sock));
/* 中文逐项-变量 V3752：type
 * type 是函数指针字段/变量：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同；函数表保存可调用入口，不是调用返回的数据。完整声明：type = (*comm->abortFlag) ?
 * ncclProxyMsgAbort : ncclProxyMsgStop。
 * type（int；函数 ncclProxyDestroy 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 初值/来源 type = (*comm->abortFlag) ? ncclProxyMsgAbort : ncclProxyMsgStop；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int type = (*comm->abortFlag) ? ncclProxyMsgAbort : ncclProxyMsgStop;
    NCCLCHECK(ncclSocketSend(&sock, &type, sizeof(int)));
    close(sock.fd);
    free(state->peerAddresses);
  }
  if (state->peerSocks) {
    for (/* 中文逐项-变量 V3753：i
 * i（int；函数 ncclProxyDestroy 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->localRanks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->localRanks; i++) {
      if (state->peerSocks[i].fd != -1) {
        if (state->proxyOps[i].pool) {
          NCCLCHECK(ncclShmClose(state->proxyOps[i].pool, NULL, sizeof(struct ncclProxyOpsPool)));
        }
        if (state->sharedDevMems[i]) {
          CUDACHECK(cudaIpcCloseMemHandle(state->sharedDevMems[i]));
        }
/* 中文逐项-变量 V3754：type
 * type（int；函数 ncclProxyDestroy 内）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 初值/来源 type = ncclProxyMsgClose；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
        int type = ncclProxyMsgClose;
        if (*comm->abortFlag == 0) NCCLCHECK(ncclSocketSend(state->peerSocks+i, &type, sizeof(int)));
        close(state->peerSocks[i].fd);
      }
    }
    free(state->peerSocks);
    free(state->proxyOps);
    free(state->sharedDevMems);
  }
  return ncclSuccess;
}
