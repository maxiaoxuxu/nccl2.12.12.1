/*************************************************************************
 * Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

/* 中文精读：内置 Proxy profiling 的观察范围
 * 本模块记录 Proxy 子任务在 GPU 等待、网络等待、flush 等阶段的时间，用于定位进度瓶颈。
 * 编译条件关闭时函数退化为空实现，不应把没有记录理解成没有网络活动。
 * 这些时间属于主机 Proxy 观测点，不能直接当作 GPU 每个归约指令的执行时间。
 * 实际对比性能还需统一张量大小、rank/channel 配置与同步测量边界。
 */
#include "profiler.h"

//#define PROFILE_PROXY 1
#ifdef PROFILE_PROXY
#include "timer.h"
#include "alloc.h"

/* 中文逐项-变量 V3430：profilingStateSendStr
 * profilingStateSendStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 profilingStateSendStr = { "BufferWait", "GPUWait", "SendWait", "", "End"
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static const char* profilingStateSendStr[] = { "BufferWait", "GPUWait", "SendWait", "", "End" };
/* 中文逐项-变量 V3431：profilingStateRecvStr
 * profilingStateRecvStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 profilingStateRecvStr = { "BufferWait", "RecvWait", "FlushWait", "GPUWait", "End"
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static const char* profilingStateRecvStr[] = { "BufferWait", "RecvWait", "FlushWait", "GPUWait", "End" };
/* 中文逐项-变量 V3432：profilingEventStr
 * profilingEventStr（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 profilingEventStr = { "SendRecv", "Sleep", "Idle", "Append"
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static const char* profilingEventStr[] = { "SendRecv", "Sleep", "Idle", "Append" };
/* 中文逐项-类型 T0393：ncclProxyProfileEvent
 * 类型职责：可选代理阶段profiling时间点及操作/子项索引，关闭编译开关时不参与通信。
 * 声明内可见成员（含嵌套成员）：timestamp、opCount、peer、step、channel、type、opIndex。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclProxyProfileEvent {
/* 中文逐项-变量 V3433：timestamp
 * timestamp（double；成员 ncclProxyProfileEvent）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 */
  double timestamp[6];
/* 中文逐项-变量 V3434：opCount
 * opCount（uint64_t；成员 ncclProxyProfileEvent）：提交批次/集合操作的单调编号，维持聚合和同连接先后关系，不是数据偏移。
 */
  uint64_t opCount;
/* 中文逐项-变量 V3435：peer
 * peer（int；成员 ncclProxyProfileEvent）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 */
  int peer;
/* 中文逐项-变量 V3436：step
 * step（int；成员 ncclProxyProfileEvent）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 */
  int step;
/* 中文逐项-变量 V3437：channel
 * channel（uint16_t；成员 ncclProxyProfileEvent）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 */
  uint16_t channel;
/* 中文逐项-变量 V3438：type
 * type（uint8_t；成员 ncclProxyProfileEvent）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  uint8_t type; // send / recv
/* 中文逐项-变量 V3439：opIndex
 * opIndex（uint8_t；成员 ncclProxyProfileEvent）：profiling记录的时间点和对应操作槽编号，仅供调试观测。
 */
  uint8_t opIndex;
};

/* 中文逐项-变量 V3440：profilingEvents
 * profilingEvents（struct ncclProxyProfileEvent；文件/静态存储作用域）：可选代理阶段测量记录，关闭profiling时不参与正常数据传输语义。
 * 初值/来源 profilingEvents = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
struct ncclProxyProfileEvent* profilingEvents = NULL;
/* 中文逐项-变量 V3441：profilingIndex
 * profilingIndex（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 profilingIndex = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
int profilingIndex = 0;
/* 中文逐项-变量 V3442：profilingStart
 * profilingStart（double；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 profilingStart = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
double profilingStart = 0;
/* 中文逐项-常量 X1170：MAX_EVENTS
 * 编译期定义：MAX_EVENTS = 200000。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_EVENTS 200000

/* 中文内部逻辑 F0961：ncclProfilingRecord
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(profilingEvents == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&profilingEvents, MAX_EVENTS));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     4. 调用 / 计算 / 写入：profilingStart = gettime();；读取本平台的轻量时间源/计数器并转成 profiling 需要的时间表示。
 * 5. 准备局部状态：struct ncclProxyProfileEvent* event = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 条件判断：(state%8 == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   7. 真分支：；仅条件为真执行以下子项。
 *     8. 条件判断：(profilingIndex == MAX_EVENTS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       9. 真分支：；仅条件为真执行以下子项。
 *         10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     11. 调用 / 计算 / 写入：args->subs[sub].profilingEvents[step%NCCL_STEPS] = event = profilingEvents+profilingIndex++;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     12. 条件判断：(state == ncclProxyProfileBegin)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       13. 真分支：；仅条件为真执行以下子项。
 *         14. 调用 / 计算 / 写入：event->opCount = args->opCount;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         15. 调用 / 计算 / 写入：event->channel = args->subs[sub].channelId;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         16. 调用 / 计算 / 写入：event->peer = args->subs[sub].peer;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         17. 调用 / 计算 / 写入：event->type = args->pattern;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         18. 调用 / 计算 / 写入：event->step = step;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *         19. 调用 / 计算 / 写入：event->opIndex = (((uint64_t)args)/sizeof(struct ncclProxyArgs))%256;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       20. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         21. 调用 / 计算 / 写入：event->peer = -state;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   22. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     23. 调用 / 计算 / 写入：event = (struct ncclProxyProfileEvent*)args->subs[sub].profilingEvents[step%NCCL_STEPS];；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     24. 条件判断：(state == ncclProxyProfileEnd)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       25. 真分支：；仅条件为真执行以下子项。
 *         26. 调用 / 计算 / 写入：args->subs[sub].profilingEvents[step%NCCL_STEPS] = NULL;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     27. 条件判断：(state == ncclProxyProfileAppendEnd)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       28. 真分支：；仅条件为真执行以下子项。
 *         29. 调用 / 计算 / 写入：event->opCount = args->opCount;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 30. 调用 / 计算 / 写入：event->timestamp[state%8] = gettime()-profilingStart;；读取本平台的轻量时间源/计数器并转成 profiling 需要的时间表示。
 * 31. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__profiler_cc.md#f0961
 */
/* 中文逐项-函数 F0961：ncclProfilingRecord
 * 用途与位置：若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 * 参数 args（struct ncclProxyArgs* args）：代理聚合运行态，保存各连接子任务的posted/received/transmitted/done进度和回调。
 * 参数 sub（int sub）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 参数 step（int step）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 参数 state（int state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyGetPostedOps；src/proxy.cc::ncclProxyProgress；src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体直接出现的调用/包装：NCCLCHECK；gettime；ncclCalloc。
 * 需要对照的边界/推进条件：profilingEvents == NULL；state%8 == 0；profilingIndex == MAX_EVENTS；state ==
 * ncclProxyProfileBegin。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProfilingRecord(struct ncclProxyArgs* args, int sub, int step, int state) {
  if (profilingEvents == NULL) {
    NCCLCHECK(ncclCalloc(&profilingEvents, MAX_EVENTS));
    profilingStart = gettime();
  }
/* 中文逐项-变量 V3443：event
 * event（struct ncclProxyProfileEvent；函数 ncclProfilingRecord 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 event = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct ncclProxyProfileEvent* event = NULL;
  if (state%8 == 0) {
    if (profilingIndex == MAX_EVENTS) return ncclSuccess;
    args->subs[sub].profilingEvents[step%NCCL_STEPS] = event = profilingEvents+profilingIndex++;
    if (state == ncclProxyProfileBegin) {
      // Proxy operation information
      event->opCount = args->opCount;
      event->channel = args->subs[sub].channelId;
      event->peer = args->subs[sub].peer;
      event->type = args->pattern;
      event->step = step;
      event->opIndex = (((uint64_t)args)/sizeof(struct ncclProxyArgs))%256;
    } else event->peer = -state;
  } else {
    event = (struct ncclProxyProfileEvent*)args->subs[sub].profilingEvents[step%NCCL_STEPS];
    if (state == ncclProxyProfileEnd) args->subs[sub].profilingEvents[step%NCCL_STEPS] = NULL;
    if (state == ncclProxyProfileAppendEnd) event->opCount = args->opCount;
  }
  // Timestamp
  event->timestamp[state%8] = gettime()-profilingStart;
  return ncclSuccess;
}

/* 中文内部逻辑 F0962：ncclProfilingDump
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：static int dumpDone = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(dumpDone)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 调用 / 计算 / 写入：dumpDone = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 准备局部状态：const char* str = getenv("NCCL_PROXY_PROFILE");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 条件判断：(!str)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：free(profilingEvents);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     10. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 11. 准备局部状态：FILE* f = fopen(str, "w");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 12. 调用 / 计算 / 写入：fprintf(f, "[\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 循环：初始 int i=0;；继续条件 i<profilingIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   14. 准备局部状态：struct ncclProxyProfileEvent* e = profilingEvents+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   15. 准备局部状态：const int sendrecv = e->peer >= 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   16. 准备局部状态：const char* typeStr = sendrecv ? (e->type == ncclPatternSend ? "Send" : "Recv") : profilingEventStr[-(e->peer/8)];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   17. 条件判断：(sendrecv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     18. 真分支：；仅条件为真执行以下子项。
 *       19. 准备局部状态：int state = ncclProxyProfileBegin;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       20. 准备局部状态：const char** stateStr = e->type == ncclPatternSend ? profilingStateSendStr : profilingStateRecvStr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       21. 调用 / 计算 / 写入：fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f, \"args\": { \"opCount\": %ld, \"proxyOpIndex\":%d } },\n", typeStr, e->peer, e->s …（完整代码见函数体）；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *       22. 循环：初始 ；继续条件 (state<ncclProxyProfileEnd)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         23. 条件判断：(e->timestamp[state])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           24. 真分支：；仅条件为真执行以下子项。
 *             25. 准备局部状态：const char* name = stateStr[state];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             26. 调用 / 计算 / 写入：fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n", name, i, e->channel, e->timestamp[state]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             27. 调用 / 计算 / 写入：state++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             28. 循环：初始 ；继续条件 (e->timestamp[state] == 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               29. 调用 / 计算 / 写入：state++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             30. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *             31. 调用 / 计算 / 写入：fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n", name, i, e->channel, e->timestamp[state]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       32. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       33. 调用 / 计算 / 写入：fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n", typeStr, e->peer, e->step, i, e->channel, e->timestamp[state]);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     34. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       35. 条件判断：(e->peer == -ncclProxyProfileAppend)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         36. 真分支：；仅条件为真执行以下子项。
 *           37. 调用 / 计算 / 写入：fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f, \"args\": { \"added\": %ld } },\n", typeStr, i, e->timestamp[0], e->opCount);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         38. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           39. 调用 / 计算 / 写入：fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f },\n", typeStr, i, e->timestamp[0]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       40. 调用 / 计算 / 写入：fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f },\n", typeStr, i, e->timestamp[1]);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 41. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 42. 调用 / 计算 / 写入：fprintf(f, "{} ]\n");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 43. 调用 / 计算 / 写入：fclose(f);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 44. 调用 / 计算 / 写入：free(profilingEvents);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__profiler_cc.md#f0962
 */
/* 中文逐项-函数 F0962：ncclProfilingDump
 * 用途与位置：导出代理阶段耗时记录，帮助区分 GPU 等待、网络等待和 flush；禁用分支无操作。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyProgressDestroy。
 * 本体直接出现的调用/包装：fclose；fopen；fprintf；free；getenv。
 * 需要对照的边界/推进条件：dumpDone；!str；i<profilingIndex；sendrecv。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 */
void ncclProfilingDump() {
/* 中文逐项-变量 V3444：dumpDone
 * dumpDone（int；函数 ncclProfilingDump 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 dumpDone = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  static int dumpDone = 0;
  if (dumpDone) return;
  dumpDone = 1;
/* 中文逐项-变量 V3445：str
 * str（char；函数 ncclProfilingDump 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 初值/来源 str = getenv("NCCL_PROXY_PROFILE")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const char* str = getenv("NCCL_PROXY_PROFILE");
  if (!str) { free(profilingEvents); return; }
/* 中文逐项-变量 V3446：f
 * f（FILE；函数 ncclProfilingDump 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 f = fopen(str, "w")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  FILE* f = fopen(str, "w");
  fprintf(f, "[\n");

  for (/* 中文逐项-变量 V3447：i
 * i（int；函数 ncclProfilingDump 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<profilingIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<profilingIndex; i++) {
/* 中文逐项-变量 V3448：e
 * e（struct ncclProxyProfileEvent；函数 ncclProfilingDump 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 e = profilingEvents+i；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ncclProxyProfileEvent* e = profilingEvents+i;
/* 中文逐项-变量 V3449：sendrecv
 * sendrecv（int；函数 ncclProfilingDump 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendrecv = e->peer >= 0；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const int sendrecv = e->peer >= 0;
/* 中文逐项-变量 V3450：typeStr
 * typeStr（char；函数 ncclProfilingDump 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 typeStr = sendrecv ? (e->type == ncclPatternSend ? "Send" : "Recv") :
 * profilingEventStr[-(e->peer/8)]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    const char* typeStr = sendrecv ? (e->type == ncclPatternSend ? "Send" : "Recv") :
      profilingEventStr[-(e->peer/8)];


    if (sendrecv) {
/* 中文逐项-变量 V3451：state
 * state（int；函数 ncclProfilingDump 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = ncclProxyProfileBegin；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int state = ncclProxyProfileBegin;
/* 中文逐项-变量 V3452：stateStr
 * stateStr（char；函数 ncclProfilingDump 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 stateStr = e->type == ncclPatternSend ? profilingStateSendStr :
 * profilingStateRecvStr；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      const char** stateStr = e->type == ncclPatternSend ? profilingStateSendStr : profilingStateRecvStr;
      fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f, \"args\": { \"opCount\": %ld, \"proxyOpIndex\":%d } },\n",
          typeStr, e->peer, e->step, i, e->channel, e->timestamp[state], e->opCount, e->opIndex);

      while (state<ncclProxyProfileEnd) {
        if (e->timestamp[state]) {
/* 中文逐项-变量 V3453：name
 * name（char；函数 ncclProfilingDump 内）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 * 初值/来源 name = stateStr[state]；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
          const char* name = stateStr[state];
          fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n",
              name, i, e->channel, e->timestamp[state]);
          state++;
          while (e->timestamp[state] == 0) state++;
          fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n",
              name, i, e->channel, e->timestamp[state]);
        }
      }

      fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n",
          typeStr, e->peer, e->step, i, e->channel, e->timestamp[state]);
    } else {
      if (e->peer == -ncclProxyProfileAppend) {
      fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f, \"args\": { \"added\": %ld } },\n",
          typeStr, i, e->timestamp[0], e->opCount);
      } else {
        fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f },\n",
          typeStr, i, e->timestamp[0]);
      }
      fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f },\n",
          typeStr, i, e->timestamp[1]);
    }
  }
  fprintf(f, "{} ]\n");
  fclose(f);
  free(profilingEvents);
}
#else
/* 中文内部逻辑 F0963：ncclProfilingRecord
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__profiler_cc.md#f0963
 */
/* 中文逐项-函数 F0963：ncclProfilingRecord
 * 用途与位置：若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 * 参数 args（struct ncclProxyArgs* args）：代理聚合运行态，保存各连接子任务的posted/received/transmitted/done进度和回调。
 * 参数 sub（int sub）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 参数 step（int step）：单调增加的连接协议进度，对 NCCL_STEPS 取模得到实际循环槽位。
 * 参数 state（int state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyGetPostedOps；src/proxy.cc::ncclProxyProgress；src/transport/net.cc::sendProxyProgress；src/transport/net.cc::recvProxyProgress。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclProfilingRecord(struct ncclProxyArgs* args, int sub, int step, int state) { return ncclSuccess; }
/* 中文内部逻辑 F0964：ncclProfilingDump
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__profiler_cc.md#f0964
 */
/* 中文逐项-函数 F0964：ncclProfilingDump
 * 用途与位置：导出代理阶段耗时记录，帮助区分 GPU 等待、网络等待和 flush；禁用分支无操作。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyProgressDestroy。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 */
void ncclProfilingDump() {}
#endif
