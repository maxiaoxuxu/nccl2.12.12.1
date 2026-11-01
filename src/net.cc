#include "net.h"
#include "bootstrap.h"
#include "checks.h"

#include <string.h>
#include <errno.h>
#include <dlfcn.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>

/* 中文逐项-变量 V3564：ncclNet
 * ncclNet（ncclNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclNet = nets[i];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
ncclNet_t *ncclNet;
/* 中文逐项-变量 V3565：ncclCollNet
 * ncclCollNet（ncclCollNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclCollNet = collNets[i];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
ncclCollNet_t *ncclCollNet;

/* 中文逐项-变量 V3566：ncclNet_v4_as_v5
 * ncclNet_v4_as_v5（ncclNet_v5_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclNet_v4_as_v5.name = ncclNet_v4->name;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static ncclNet_v5_t ncclNet_v4_as_v5;
/* 中文逐项-变量 V3567：ncclNet_v4
 * ncclNet_v4（ncclNet_v4_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclResult_t ans = ncclNet_v4->getProperties(dev, &p4);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static ncclNet_v4_t *ncclNet_v4;
/* 中文逐项-变量 V3568：ncclCollNet_v4_as_v5
 * ncclCollNet_v4_as_v5（ncclCollNet_v5_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclCollNet_v4_as_v5.name = ncclCollNet_v4->name;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static ncclCollNet_v5_t ncclCollNet_v4_as_v5;
/* 中文逐项-变量 V3569：ncclCollNet_v4
 * ncclCollNet_v4（ncclCollNet_v4_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclResult_t ans = ncclCollNet_v4->getProperties(dev, &p4);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static ncclCollNet_v4_t *ncclCollNet_v4;

/* 中文内部逻辑 F1003：ncclNet_v4_as_v5_getProperties
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：ncclNetProperties_v4_t p4;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：ncclResult_t ans = ncclNet_v4->getProperties(dev, &p4);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(ans != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 函数返回：return ans;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：props->name = p4.name;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：props->pciPath = p4.pciPath;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：props->guid = p4.guid;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：props->ptrSupport = p4.ptrSupport;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：props->speed = p4.speed;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：props->port = p4.port;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：props->maxComms = p4.maxComms;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：props->maxRecvs = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 调用 / 计算 / 写入：props->latency = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1003
 */
/* 中文逐项-函数 F1003：ncclNet_v4_as_v5_getProperties
 * 用途与位置：调用旧版插件属性接口并补上 v5 新增字段的兼容默认值，如 maxRecvs=1。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 props（ncclNetProperties_v5_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclNet_v4->getProperties。
 * 需要对照的边界/推进条件：ans != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ans;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNet_v4_as_v5_getProperties(int dev, ncclNetProperties_v5_t* props) {
/* 中文逐项-变量 V3570：p4
 * p4（ncclNetProperties_v4_t；函数 ncclNet_v4_as_v5_getProperties 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclResult_t ans = ncclNet_v4->getProperties(dev, &p4);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclNetProperties_v4_t p4;
/* 中文逐项-变量 V3571：ans
 * ans（ncclResult_t；函数 ncclNet_v4_as_v5_getProperties 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ans = ncclNet_v4->getProperties(dev, &p4)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclResult_t ans = ncclNet_v4->getProperties(dev, &p4);
  if (ans != ncclSuccess) return ans;
  props->name = p4.name;
  props->pciPath = p4.pciPath;
  props->guid = p4.guid;
  props->ptrSupport = p4.ptrSupport;
  props->speed = p4.speed;
  props->port = p4.port;
  props->maxComms = p4.maxComms;
  props->maxRecvs = 1;
  props->latency = 0;
  return ncclSuccess;
}

/* 中文内部逻辑 F1004：ncclNet_v4_as_v5_isend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclNet_v4->isend(sendComm, data, size, mhandle, request);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1004
 */
/* 中文逐项-函数 F1004：ncclNet_v4_as_v5_isend
 * 用途与位置：把 v5 发送调用转成 v4 签名，旧接口没有 v5 tag 参数。
 * 参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclNet_v4->isend。
 * 源码返回表达式：return ncclNet_v4->isend(sendComm, data, size, mhandle, request);。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNet_v4_as_v5_isend(void* sendComm, void* data, int size, int tag, void* mhandle, void** request) {
  return ncclNet_v4->isend(sendComm, data, size, mhandle, request);
}

/* 中文内部逻辑 F1005：ncclNet_v4_as_v5_irecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(n == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 条件判断：(n != 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 函数返回：return ncclNet_v4->irecv(recvComm, data[0], sizes[0], mhandles[0], request);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1005
 */
/* 中文逐项-函数 F1005：ncclNet_v4_as_v5_irecv
 * 用途与位置：校验 v5 批量请求只有一个元素后转调旧 v4 单请求接口，避免错误模拟多接收能力。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 tags（int* tags）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclNet_v4->irecv。
 * 需要对照的边界/推进条件：n == 0；n != 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInvalidArgument;；return ncclNet_v4->irecv(recvComm, data[0], sizes[0],
 * mhandles[0], request);。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNet_v4_as_v5_irecv(void* recvComm, int n, void** data, int* sizes, int* tags, void** mhandles, void** request) {
  if (n == 0) return ncclSuccess;
  if (n != 1) return ncclInvalidArgument;
  return ncclNet_v4->irecv(recvComm, data[0], sizes[0], mhandles[0], request);
}

/* 中文内部逻辑 F1006：ncclNet_v4_as_v5_iflush
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(n == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 条件判断：(n != 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 函数返回：return ncclNet_v4->iflush(recvComm, data[0], sizes[0], mhandles[0], request);；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1006
 */
/* 中文逐项-函数 F1006：ncclNet_v4_as_v5_iflush
 * 用途与位置：校验 v5 批量请求只有一个元素后转调旧 v4 单请求接口，避免错误模拟多接收能力。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclNet_v4->iflush。
 * 需要对照的边界/推进条件：n == 0；n != 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInvalidArgument;；return ncclNet_v4->iflush(recvComm, data[0], sizes[0],
 * mhandles[0], request);。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNet_v4_as_v5_iflush(void* recvComm, int n, void** data, int* sizes, void** mhandles, void** request) {
  if (n == 0) return ncclSuccess;
  if (n != 1) return ncclInvalidArgument;
  return ncclNet_v4->iflush(recvComm, data[0], sizes[0], mhandles[0], request);
}

// We use a wrapper around the v4 init to copy over the struct contents
// post-init since they may not be initialized before hand.
/* 中文内部逻辑 F1007：ncclNet_v4_as_v5_init
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNet_v4->init(logfn));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 2. 调用 / 计算 / 写入：ncclNet_v4_as_v5.name = ncclNet_v4->name;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：ncclNet_v4_as_v5.devices = ncclNet_v4->devices;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：ncclNet_v4_as_v5.getProperties = ncclNet_v4_as_v5_getProperties;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：ncclNet_v4_as_v5.listen = ncclNet_v4->listen;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 调用 / 计算 / 写入：ncclNet_v4_as_v5.connect = ncclNet_v4->connect;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：ncclNet_v4_as_v5.accept = ncclNet_v4->accept;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：ncclNet_v4_as_v5.regMr = ncclNet_v4->regMr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：ncclNet_v4_as_v5.deregMr = ncclNet_v4->deregMr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：ncclNet_v4_as_v5.isend = ncclNet_v4_as_v5_isend;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：ncclNet_v4_as_v5.irecv = ncclNet_v4_as_v5_irecv;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：ncclNet_v4_as_v5.iflush = ncclNet_v4_as_v5_iflush;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：ncclNet_v4_as_v5.test = ncclNet_v4->test;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 调用 / 计算 / 写入：ncclNet_v4_as_v5.closeSend = ncclNet_v4->closeSend;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：ncclNet_v4_as_v5.closeRecv = ncclNet_v4->closeRecv;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 调用 / 计算 / 写入：ncclNet_v4_as_v5.closeListen = ncclNet_v4->closeListen;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1007
 */
/* 中文逐项-函数 F1007：ncclNet_v4_as_v5_init
 * 用途与位置：初始化旧插件后构造 v5 适配函数表，保持旧插件可在本版本网络层使用。
 * 参数 logfn（ncclDebugLogger_t logfn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNet_v4->init。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclNet_v4_as_v5_init(ncclDebugLogger_t logfn) {
  NCCLCHECK(ncclNet_v4->init(logfn));
  ncclNet_v4_as_v5.name = ncclNet_v4->name;
  ncclNet_v4_as_v5.devices = ncclNet_v4->devices;
  ncclNet_v4_as_v5.getProperties = ncclNet_v4_as_v5_getProperties;
  ncclNet_v4_as_v5.listen = ncclNet_v4->listen;
  ncclNet_v4_as_v5.connect = ncclNet_v4->connect;
  ncclNet_v4_as_v5.accept = ncclNet_v4->accept;
  ncclNet_v4_as_v5.regMr = ncclNet_v4->regMr;
  ncclNet_v4_as_v5.deregMr = ncclNet_v4->deregMr;
  ncclNet_v4_as_v5.isend = ncclNet_v4_as_v5_isend;
  ncclNet_v4_as_v5.irecv = ncclNet_v4_as_v5_irecv;
  ncclNet_v4_as_v5.iflush = ncclNet_v4_as_v5_iflush;
  ncclNet_v4_as_v5.test = ncclNet_v4->test;
  ncclNet_v4_as_v5.closeSend = ncclNet_v4->closeSend;
  ncclNet_v4_as_v5.closeRecv = ncclNet_v4->closeRecv;
  ncclNet_v4_as_v5.closeListen = ncclNet_v4->closeListen;
  return ncclSuccess;
}

/* 中文内部逻辑 F1008：ncclCollNet_v4_as_v5_getProperties
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：ncclNetProperties_v4_t p4;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：ncclResult_t ans = ncclCollNet_v4->getProperties(dev, &p4);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(ans != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 函数返回：return ans;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：props->name = p4.name;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：props->pciPath = p4.pciPath;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：props->guid = p4.guid;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：props->ptrSupport = p4.ptrSupport;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：props->speed = p4.speed;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：props->port = p4.port;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：props->maxComms = p4.maxComms;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：props->maxRecvs = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 调用 / 计算 / 写入：props->latency = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1008
 */
/* 中文逐项-函数 F1008：ncclCollNet_v4_as_v5_getProperties
 * 用途与位置：调用旧版插件属性接口并补上 v5 新增字段的兼容默认值，如 maxRecvs=1。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 props（ncclNetProperties_v5_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclCollNet_v4->getProperties。
 * 需要对照的边界/推进条件：ans != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ans;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclCollNet_v4_as_v5_getProperties(int dev, ncclNetProperties_v5_t* props) {
/* 中文逐项-变量 V3572：p4
 * p4（ncclNetProperties_v4_t；函数 ncclCollNet_v4_as_v5_getProperties 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclResult_t ans = ncclCollNet_v4->getProperties(dev, &p4);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclNetProperties_v4_t p4;
/* 中文逐项-变量 V3573：ans
 * ans（ncclResult_t；函数 ncclCollNet_v4_as_v5_getProperties 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ans = ncclCollNet_v4->getProperties(dev, &p4)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  ncclResult_t ans = ncclCollNet_v4->getProperties(dev, &p4);
  if (ans != ncclSuccess) return ans;
  props->name = p4.name;
  props->pciPath = p4.pciPath;
  props->guid = p4.guid;
  props->ptrSupport = p4.ptrSupport;
  props->speed = p4.speed;
  props->port = p4.port;
  props->maxComms = p4.maxComms;
  props->maxRecvs = 1;
  props->latency = 0;
  return ncclSuccess;
}

// We use a wrapper around the v4 init to copy over the struct contents
// post-init since they may not be initialized before hand.
/* 中文内部逻辑 F1009：ncclCollNet_v4_as_v5_init
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclCollNet_v4->init(logfn));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 2. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.name = ncclCollNet_v4->name;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.devices = ncclCollNet_v4->devices;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.getProperties = ncclCollNet_v4_as_v5_getProperties;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.listen = ncclCollNet_v4->listen;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.connect = ncclCollNet_v4->connect;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.reduceSupport = ncclCollNet_v4->reduceSupport;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.regMr = ncclCollNet_v4->regMr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.deregMr = ncclCollNet_v4->deregMr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.iallreduce = ncclCollNet_v4->iallreduce;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.iflush = ncclCollNet_v4->iflush;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.test = ncclCollNet_v4->test;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.closeColl = ncclCollNet_v4->closeColl;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.closeListen = ncclCollNet_v4->closeListen;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1009
 */
/* 中文逐项-函数 F1009：ncclCollNet_v4_as_v5_init
 * 用途与位置：初始化旧插件后构造 v5 适配函数表，保持旧插件可在本版本网络层使用。
 * 参数 logfn（ncclDebugLogger_t logfn）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCollNet_v4->init。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclCollNet_v4_as_v5_init(ncclDebugLogger_t logfn) {
  NCCLCHECK(ncclCollNet_v4->init(logfn));
  ncclCollNet_v4_as_v5.name = ncclCollNet_v4->name;
  ncclCollNet_v4_as_v5.devices = ncclCollNet_v4->devices;
  ncclCollNet_v4_as_v5.getProperties = ncclCollNet_v4_as_v5_getProperties;
  ncclCollNet_v4_as_v5.listen = ncclCollNet_v4->listen;
  ncclCollNet_v4_as_v5.connect = ncclCollNet_v4->connect;
  ncclCollNet_v4_as_v5.reduceSupport = ncclCollNet_v4->reduceSupport;
  ncclCollNet_v4_as_v5.regMr = ncclCollNet_v4->regMr;
  ncclCollNet_v4_as_v5.deregMr = ncclCollNet_v4->deregMr;
  ncclCollNet_v4_as_v5.iallreduce = ncclCollNet_v4->iallreduce;
  ncclCollNet_v4_as_v5.iflush = ncclCollNet_v4->iflush;
  ncclCollNet_v4_as_v5.test = ncclCollNet_v4->test;
  ncclCollNet_v4_as_v5.closeColl = ncclCollNet_v4->closeColl;
  ncclCollNet_v4_as_v5.closeListen = ncclCollNet_v4->closeListen;
  return ncclSuccess;
}

/* 中文精读：插件是网络函数表的动态替换
 * 按 NCCL_NET_PLUGIN 拼出 libnccl-net-名字.so，未设置则尝试 libnccl-net.so。
 * dlopen/dlsym 查找 v5 符号，找不到时尝试 v4 并用适配函数补齐 v5 接口。
 * 这允许外部网络后端实现不同传输协议；本文件不强制插件必须是 UCX 或某种 RDMA 库。
 * 函数指针多一层调用不等于多复制一遍载荷，主要传输成本仍取决于后端实现。
 */
/* 中文内部逻辑 F1010：initPlugin
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char ncclNetPluginName[128];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：const char* envPluginName = getenv("NCCL_NET_PLUGIN");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(envPluginName && strlen(envPluginName))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：snprintf(ncclNetPluginName, 128, "libnccl-net-%s.so", envPluginName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 调用 / 计算 / 写入：INFO(NCCL_INIT, "Plugin name set by env to %s", ncclNetPluginName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   7. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     8. 调用 / 计算 / 写入：sprintf(ncclNetPluginName, "libnccl-net.so");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 准备局部状态：void* netPluginLib = dlopen(ncclNetPluginName, RTLD_NOW | RTLD_LOCAL);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 条件判断：(netPluginLib == nullptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 条件判断：(errno == ENOENT)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       13. 真分支：；仅条件为真执行以下子项。
 *         14. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET, "NET/Plugin : No plugin found (%s), using internal implementation", ncclNetPluginName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       15. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         16. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET, "NET/Plugin : Plugin load returned %d : %s.", errno, dlerror());；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     17. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 18. 调用 / 计算 / 写入：*net = (ncclNet_v5_t*)dlsym(netPluginLib, "ncclNetPlugin_v5");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 条件判断：(*net == nullptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   20. 真分支：；仅条件为真执行以下子项。
 *     21. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclNetPlugin_v5 symbol.");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     22. 调用 / 计算 / 写入：ncclNet_v4 = (ncclNet_v4_t*)dlsym(netPluginLib, "ncclNetPlugin_v4");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     23. 条件判断：(ncclNet_v4 == nullptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       24. 真分支：；仅条件为真执行以下子项。
 *         25. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclNetPlugin_v4 symbol.");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         26. 条件判断：(netPluginLib != nullptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           27. 真分支：；仅条件为真执行以下子项。
 *             28. 调用 / 计算 / 写入：dlclose(netPluginLib);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         29. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     30. 调用 / 计算 / 写入：*net = &ncclNet_v4_as_v5;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     31. 调用 / 计算 / 写入：ncclNet_v4_as_v5.init = ncclNet_v4_as_v5_init;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 32. 调用 / 计算 / 写入：*collnet = (ncclCollNet_v5_t*)dlsym(netPluginLib, "ncclCollNetPlugin_v5");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 33. 条件判断：(*collnet == nullptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   34. 真分支：；仅条件为真执行以下子项。
 *     35. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclCollNetPlugin_v5 symbol.");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     36. 调用 / 计算 / 写入：ncclCollNet_v4 = (ncclCollNet_v4_t*)dlsym(netPluginLib, "ncclCollNetPlugin_v4");；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     37. 条件判断：(ncclCollNet_v4 == nullptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       38. 真分支：；仅条件为真执行以下子项。
 *         39. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclCollNetPlugin_v4 symbol.");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       40. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         41. 调用 / 计算 / 写入：*collnet = &ncclCollNet_v4_as_v5;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         42. 调用 / 计算 / 写入：ncclCollNet_v4_as_v5.init = ncclCollNet_v4_as_v5_init;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 43. 函数返回：return;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1010
 */
/* 中文逐项-函数 F1010：initPlugin
 * 用途与位置：动态装载命名网络插件，优先找 v5 符号，必要时适配 v4，并独立发现 CollNet 接口。
 * 参数 net（ncclNet_v5_t** net）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 参数 collnet（ncclCollNet_v5_t** collnet）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclNetInit。
 * 本体直接出现的调用/包装：dlclose；dlerror；dlopen；dlsym；getenv；snprintf；sprintf；strlen。
 * 需要对照的边界/推进条件：envPluginName && strlen(envPluginName；netPluginLib == nullptr；errno == ENOENT；*net ==
 * nullptr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return;。CHECK类宏还可能提前返回/跳转。
 */
static void initPlugin(ncclNet_v5_t** net, ncclCollNet_v5_t** collnet) {
/* 中文逐项-变量 V3574：ncclNetPluginName
 * ncclNetPluginName（char；函数 initPlugin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：snprintf(ncclNetPluginName, 128, "libnccl-net-%s.so", envPluginName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char ncclNetPluginName[128];
/* 中文逐项-变量 V3575：envPluginName
 * envPluginName（char；函数 initPlugin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 envPluginName = getenv("NCCL_NET_PLUGIN")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const char* envPluginName = getenv("NCCL_NET_PLUGIN");
  if (envPluginName && strlen(envPluginName)) {
    snprintf(ncclNetPluginName, 128, "libnccl-net-%s.so", envPluginName);
    INFO(NCCL_INIT, "Plugin name set by env to %s", ncclNetPluginName);
  } else {
    sprintf(ncclNetPluginName, "libnccl-net.so");
  }
/* 中文逐项-变量 V3576：netPluginLib
 * netPluginLib（void；函数 initPlugin 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 netPluginLib = dlopen(ncclNetPluginName, RTLD_NOW | RTLD_LOCAL)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  void* netPluginLib = dlopen(ncclNetPluginName, RTLD_NOW | RTLD_LOCAL);
  if (netPluginLib == nullptr) {
    // dlopen does not guarantee to set errno, but dlerror only gives us a
    // string, so checking errno doesn't hurt to try to provide a better
    // error message
    if (errno == ENOENT) {
      INFO(NCCL_INIT|NCCL_NET, "NET/Plugin : No plugin found (%s), using internal implementation", ncclNetPluginName);
    } else {
      INFO(NCCL_INIT|NCCL_NET, "NET/Plugin : Plugin load returned %d : %s.", errno, dlerror());
    }
    return;
  }

  *net = (ncclNet_v5_t*)dlsym(netPluginLib, "ncclNetPlugin_v5");
  if (*net == nullptr) {
    INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclNetPlugin_v5 symbol.");
    ncclNet_v4 = (ncclNet_v4_t*)dlsym(netPluginLib, "ncclNetPlugin_v4");
    if (ncclNet_v4 == nullptr) {
      INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclNetPlugin_v4 symbol.");
      if (netPluginLib != nullptr) dlclose(netPluginLib);
      return;
    }
    *net = &ncclNet_v4_as_v5;
    ncclNet_v4_as_v5.init = ncclNet_v4_as_v5_init;
  }

  // Check for CollNet
  *collnet = (ncclCollNet_v5_t*)dlsym(netPluginLib, "ncclCollNetPlugin_v5");
  if (*collnet == nullptr) {
    INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclCollNetPlugin_v5 symbol.");
    ncclCollNet_v4 = (ncclCollNet_v4_t*)dlsym(netPluginLib, "ncclCollNetPlugin_v4");
    if (ncclCollNet_v4 == nullptr) {
      INFO(NCCL_INIT|NCCL_NET, "NET/Plugin: Failed to find ncclCollNetPlugin_v4 symbol.");
    } else {
      *collnet = &ncclCollNet_v4_as_v5;
      ncclCollNet_v4_as_v5.init = ncclCollNet_v4_as_v5_init;
    }
  }
  return;
}

/* 中文导读：
 * 网络后端选择入口：初始化 bootstrap 后，依次尝试外部插件、内置 IB、内置 Socket。
 * NCCL_NET 可按后端名筛选。外部插件通过 libnccl-net*.so 的 v5/v4 函数表接入；不是必须使用 UCX。
 * 真正的 GPU/Proxy 网络传输通过统一 ncclNet 接口调用已选后端。
 */
/* 中文内部逻辑 F1011：ncclNetInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetInit());；失败时可能提前return，成功才继续；选择初始化控制面使用的本机网络接口和地址；其选择不等于数据面必然使用 TCP。
 * 2. 准备局部状态：ncclNet_t* nets[3] = { nullptr, &ncclNetIb, &ncclNetSocket };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：ncclCollNet_t* collNets[3] = { nullptr, nullptr, nullptr };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：initPlugin(&nets[0], &collNets[0]);；动态装载命名网络插件，优先找 v5 符号，必要时适配 v4，并独立发现 CollNet 接口。
 * 5. 准备局部状态：char* netName = getenv("NCCL_NET");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：bool ok = false;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 循环：初始 int i=0;；继续条件 i<3；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   8. 条件判断：(nets[i] == nullptr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     9. 真分支：；仅条件为真执行以下子项。
 *       10. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   11. 条件判断：(netName && strcmp(netName, nets[i]->name) != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     12. 真分支：；仅条件为真执行以下子项。
 *       13. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   14. 准备局部状态：int ndev;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   15. 条件判断：(nets[i]->init(ncclDebugLog) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     16. 真分支：；仅条件为真执行以下子项。
 *       17. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   18. 条件判断：(nets[i]->devices(&ndev) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     19. 真分支：；仅条件为真执行以下子项。
 *       20. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   21. 条件判断：(ndev <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     22. 真分支：；仅条件为真执行以下子项。
 *       23. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   24. 调用 / 计算 / 写入：ncclNet = nets[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   25. 调用 / 计算 / 写入：ok = true;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   26. 条件判断：(collNets[i])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     27. 真分支：；仅条件为真执行以下子项。
 *       28. 先执行一次的循环：初始 ；继续条件 (0)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *         29. 条件判断：(collNets[i]->init(ncclDebugLog) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           30. 真分支：；仅条件为真执行以下子项。
 *             31. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         32. 条件判断：(collNets[i]->devices(&ndev) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           33. 真分支：；仅条件为真执行以下子项。
 *             34. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         35. 条件判断：(ndev <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           36. 真分支：；仅条件为真执行以下子项。
 *             37. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 *         38. 调用 / 计算 / 写入：ncclCollNet = collNets[i];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       39. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   40. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 * 41. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 42. 条件判断：(!ok)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   43. 真分支：；仅条件为真执行以下子项。
 *     44. 调用 / 计算 / 写入：WARN("Error: network %s not found.", netName ? netName : "");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     45. 函数返回：return ncclInvalidUsage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 46. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1011
 */
/* 中文逐项-函数 F1011：ncclNetInit
 * 用途与位置：总是初始化 bootstrap，再按外部插件、内置 IB、内置 Socket 顺序选可用数据后端，支持显式名字筛选。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclInit。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapNetInit；collNets[i]->devices；collNets[i]->init；getenv；initPlugin；nets[i]->devices；nets[i]->init；strcmp。
 * 需要对照的边界/推进条件：i<3；nets[i] == nullptr；netName && strcmp(netName, nets[i]->name) != 0；nets[i]->init(ncclDebugLog) !=
 * ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidUsage;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNetInit() {
  // Always initialize bootstrap network
  NCCLCHECK(bootstrapNetInit());

  // Initialize main communication network
/* 中文逐项-变量 V3577：nets
 * nets（ncclNet_t；函数 ncclNetInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 nets = { nullptr, &ncclNetIb, &ncclNetSocket }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  ncclNet_t* nets[3] = { nullptr, &ncclNetIb, &ncclNetSocket };
/* 中文逐项-变量 V3578：collNets
 * collNets（ncclCollNet_t；函数 ncclNetInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 collNets = { nullptr, nullptr, nullptr }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  ncclCollNet_t* collNets[3] = { nullptr, nullptr, nullptr };
  initPlugin(&nets[0], &collNets[0]);
/* 中文逐项-变量 V3579：netName
 * netName（char；函数 ncclNetInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 netName = getenv("NCCL_NET")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* netName = getenv("NCCL_NET");
/* 中文逐项-变量 V3580：ok
 * ok（bool；函数 ncclNetInit 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 ok = false；布尔初态为false，后续条件决定是否切换。
 */
  bool ok = false;

  for (/* 中文逐项-变量 V3581：i
 * i（int；函数 ncclNetInit 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<3；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<3; i++) {
    if (nets[i] == nullptr) continue;
    if (netName && strcmp(netName, nets[i]->name) != 0) continue;

    // net plugin is already initialized
/* 中文逐项-变量 V3582：ndev
 * ndev（int；函数 ncclNetInit 内）：当前网络后端枚举的设备/端口数，或devices回调的输出地址；它不是CUDA设备数或communicator的rank数。
 * 此处未显式给初值；后续首个文本使用点：if (nets[i]->devices(&ndev) != ncclSuccess) continue;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int ndev;
    if (nets[i]->init(ncclDebugLog) != ncclSuccess) continue;
    if (nets[i]->devices(&ndev) != ncclSuccess) continue;
    if (ndev <= 0) continue;
    ncclNet = nets[i];
    ok = true;

    if (collNets[i]) {
      do {
        if (collNets[i]->init(ncclDebugLog) != ncclSuccess) break;
        if (collNets[i]->devices(&ndev) != ncclSuccess) break;
        if (ndev <= 0) break;
        ncclCollNet = collNets[i];
      } while(0);
    }
    break;
  }

  if (!ok) {
    WARN("Error: network %s not found.", netName ? netName : "");
    return ncclInvalidUsage;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1012：ncclGpuGdrSupport
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：constexpr int GPU_BUF_SIZE = 2*1024*1024;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 编译期分支/指令：#if CUDART_VERSION >= 11030；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *   3. 准备局部状态：int driverVersion;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   4. 调用 / 计算 / 写入：CUDACHECK(cudaDriverGetVersion(&driverVersion));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *   5. 条件判断：(driverVersion >= 11030)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     6. 真分支：；仅条件为真执行以下子项。
 *       7. 准备局部状态：int cudaDev, attr = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       8. 调用 / 计算 / 写入：CUDACHECK(cudaGetDevice(&cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *       9. 调用 / 计算 / 写入：CUDACHECK(cudaDeviceGetAttribute(&attr, cudaDevAttrGPUDirectRDMASupported, cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *       10. 调用 / 计算 / 写入：*gdrSupport = attr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 准备局部状态：int netDevs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 13. 调用 / 计算 / 写入：NCCLCHECK(ncclNetDevices(&netDevs));；失败时可能提前return，成功才继续；查询该后端可用设备数量；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 * 14. 调用 / 计算 / 写入：*gdrSupport = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 循环：初始 int dev=0;；继续条件 dev<netDevs；每轮更新 dev++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   16. 准备局部状态：ncclNetProperties_t props;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   17. 调用 / 计算 / 写入：NCCLCHECK(ncclNetGetProperties(dev, &props));；失败时可能提前return，成功才继续；读取设备拓扑/速度/指针/接收能力属性；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   18. 条件判断：((props.ptrSupport & NCCL_PTR_CUDA) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     19. 真分支：；仅条件为真执行以下子项。
 *       20. 控制转移：continue;；跳到当前最内层循环的下一轮，for还会执行其增量步骤。
 *   21. 准备局部状态：void *lComm = NULL, *sComm = NULL, *rComm = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   22. 准备局部状态：ncclNetHandle_t handle;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   23. 准备局部状态：void* gpuPtr = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   24. 准备局部状态：void* mHandle = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   25. 准备局部状态：ncclResult_t ret;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   26. 调用 / 计算 / 写入：ncclDebugNoWarn = NCCL_NET;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   27. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclNetListen(dev, &handle, &lComm), ret, cleanup1);；失败时保存错误并跳到清理标签，成功才继续；创建接收监听并导出连接句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   28. 循环：初始 ；继续条件 (sComm == NULL)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     29. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclNetConnect(dev, &handle, &sComm), ret, cleanup2);；失败时保存错误并跳到清理标签，成功才继续；推进发送端连接建立；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   30. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   31. 循环：初始 ；继续条件 (rComm == NULL)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     32. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclNetAccept(lComm, &rComm), ret, cleanup3);；失败时保存错误并跳到清理标签，成功才继续；推进接收端接受连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   33. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   34. 调用 / 计算 / 写入：CUDACHECKGOTO(cudaMalloc(&gpuPtr, GPU_BUF_SIZE), ret, cleanup4);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 *   35. 条件判断：(ncclNetRegMr(sComm, gpuPtr, GPU_BUF_SIZE, NCCL_PTR_CUDA, &mHandle) == ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     36. 真分支：；仅条件为真执行以下子项。
 *       37. 调用 / 计算 / 写入：NCCLCHECK(ncclNetDeregMr(sComm, mHandle));；失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *       38. 调用 / 计算 / 写入：NCCLCHECK(ncclNetRegMr(rComm, gpuPtr, GPU_BUF_SIZE, NCCL_PTR_CUDA, &mHandle));；失败时可能提前return，成功才继续；注册后续网络要访问的内存并返回后端句柄；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *       39. 调用 / 计算 / 写入：NCCLCHECK(ncclNetDeregMr(rComm, mHandle));；失败时可能提前return，成功才继续；解除对应后端内存注册引用；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *       40. 调用 / 计算 / 写入：*gdrSupport = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   41. 调用 / 计算 / 写入：ncclDebugNoWarn = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   42. 调用 / 计算 / 写入：CUDACHECK(cudaFree(gpuPtr));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *   43. 跳转标签：cleanup4；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     44. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseRecv(rComm));；失败时可能提前return，成功才继续；关闭接收连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   45. 跳转标签：cleanup3；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     46. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseSend(sComm));；失败时可能提前return，成功才继续；关闭发送连接；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   47. 跳转标签：cleanup2；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     48. 调用 / 计算 / 写入：NCCLCHECK(ncclNetCloseListen(lComm));；失败时可能提前return，成功才继续；关闭监听对象；本层通过函数指针转交已选后端，不额外复制整个用户张量。
 *   49. 跳转标签：cleanup1；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     50. 控制转移：break;；退出当前最内层循环或switch，继续其后；并非结束整个函数。
 * 51. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 52. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1012
 */
/* 中文逐项-函数 F1012：ncclGpuGdrSupport
 * 用途与位置：优先查询 CUDA GDR 属性；旧运行时通过临时连接和 GPU 缓冲注册探测，探测资源用后回收。
 * 参数 gdrSupport（int* gdrSupport）：设备能否注册GPU内存、进行网络集合归约及接受哪种指针类型的能力描述。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::fillInfo。
 * 本体直接出现的调用/包装：CUDACHECK；CUDACHECKGOTO；NCCLCHECK；NCCLCHECKGOTO；cudaDeviceGetAttribute；cudaDriverGetVersion；cudaFree；cudaGetDevice；cudaMalloc；ncclNetAccept；ncclNetCloseListen；ncclNetCloseRecv；ncclNetCloseSend；ncclNetConnect；ncclNetDeregMr；ncclNetDevices；ncclNetGetProperties；ncclNetListen；更多调用保留在函数体。
 * 需要对照的边界/推进条件：driverVersion >= 11030；dev<netDevs；props.ptrSupport & NCCL_PTR_CUDA) == 0；sComm == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGpuGdrSupport(int* gdrSupport) {
/* 中文逐项-变量 V3583：GPU_BUF_SIZE
 * GPU_BUF_SIZE（int；函数 ncclGpuGdrSupport 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 GPU_BUF_SIZE = 2*1024*1024；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  constexpr int GPU_BUF_SIZE = 2*1024*1024;
#if CUDART_VERSION >= 11030
  // In CUDA 11.3 and later we can now query the cudaDevAttrGPUDirectRDMASupported attribute
/* 中文逐项-变量 V3584：driverVersion
 * driverVersion（int；函数 ncclGpuGdrSupport 内）：CUDA 驱动版本与分配范围查询入口，用于判断图/IPC注册优化能力。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaDriverGetVersion(&driverVersion));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int driverVersion;
  CUDACHECK(cudaDriverGetVersion(&driverVersion));
  if (driverVersion >= 11030) {
/* 中文逐项-变量 V3585：cudaDev、attr
 * cudaDev（int；函数 ncclGpuGdrSupport 内）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 * 此处未显式给初值；后续首个文本使用点：CUDACHECK(cudaGetDevice(&cudaDev));。若由输出参数填入，必须先检查填充调用成功再读值。
 * attr（int；函数 ncclGpuGdrSupport 内）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 初值/来源 attr = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int cudaDev, attr = 0;
    CUDACHECK(cudaGetDevice(&cudaDev));
    CUDACHECK(cudaDeviceGetAttribute(&attr, cudaDevAttrGPUDirectRDMASupported, cudaDev));
    *gdrSupport = attr;
    return ncclSuccess;
  }
#endif
/* 中文逐项-变量 V3586：netDevs
 * netDevs（int；函数 ncclGpuGdrSupport 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclNetDevices(&netDevs));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int netDevs;
  NCCLCHECK(ncclNetDevices(&netDevs));
  *gdrSupport = 0;
  for (/* 中文逐项-变量 V3587：dev
 * dev（int；函数 ncclGpuGdrSupport 内）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 初值/来源 dev = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：dev<netDevs；循环体与增量表达式决定每轮如何推进。
 */int dev=0; dev<netDevs; dev++) {
    // Find a net device which is GDR-capable
/* 中文逐项-变量 V3588：props
 * props（ncclNetProperties_t；函数 ncclGpuGdrSupport 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclNetGetProperties(dev, &props));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    ncclNetProperties_t props;
    NCCLCHECK(ncclNetGetProperties(dev, &props));
    if ((props.ptrSupport & NCCL_PTR_CUDA) == 0) continue;

    // Allocate memory on the GPU and try to register it on the NIC.
/* 中文逐项-变量 V3589：lComm、sComm、rComm
 * lComm（void；函数 ncclGpuGdrSupport 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 lComm = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 * sComm（void；函数 ncclGpuGdrSupport 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 sComm = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 * rComm（void；函数 ncclGpuGdrSupport 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 rComm = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    void *lComm = NULL, *sComm = NULL, *rComm = NULL;
/* 中文逐项-变量 V3590：handle
 * handle（ncclNetHandle_t；函数 ncclGpuGdrSupport 内）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(ncclNetListen(dev, &handle, &lComm), ret, cleanup1);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    ncclNetHandle_t handle;
/* 中文逐项-变量 V3591：gpuPtr
 * gpuPtr（void；函数 ncclGpuGdrSupport 内）：同一后端缓冲的CPU/GPU地址视图，是否能直接使用取决于映射和GDR配置。
 * 初值/来源 gpuPtr = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    void* gpuPtr = NULL;
/* 中文逐项-变量 V3592：mHandle
 * mHandle（void；函数 ncclGpuGdrSupport 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 mHandle = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
    void* mHandle = NULL;
/* 中文逐项-变量 V3593：ret
 * ret（ncclResult_t；函数 ncclGpuGdrSupport 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(ncclNetListen(dev, &handle, &lComm), ret, cleanup1);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    ncclResult_t ret;
    ncclDebugNoWarn = NCCL_NET;
    NCCLCHECKGOTO(ncclNetListen(dev, &handle, &lComm), ret, cleanup1);
    while (sComm == NULL) {
      NCCLCHECKGOTO(ncclNetConnect(dev, &handle, &sComm), ret, cleanup2);
    }
    while (rComm == NULL) {
      NCCLCHECKGOTO(ncclNetAccept(lComm, &rComm), ret, cleanup3);
    }
    CUDACHECKGOTO(cudaMalloc(&gpuPtr, GPU_BUF_SIZE), ret, cleanup4);
    if (ncclNetRegMr(sComm, gpuPtr, GPU_BUF_SIZE, NCCL_PTR_CUDA, &mHandle) == ncclSuccess) {
      NCCLCHECK(ncclNetDeregMr(sComm, mHandle));
      NCCLCHECK(ncclNetRegMr(rComm, gpuPtr, GPU_BUF_SIZE, NCCL_PTR_CUDA, &mHandle));
      NCCLCHECK(ncclNetDeregMr(rComm, mHandle));
      *gdrSupport = 1;
    }
    ncclDebugNoWarn = 0;
    CUDACHECK(cudaFree(gpuPtr));
cleanup4:
    NCCLCHECK(ncclNetCloseRecv(rComm));
cleanup3:
    NCCLCHECK(ncclNetCloseSend(sComm));
cleanup2:
    NCCLCHECK(ncclNetCloseListen(lComm));
cleanup1:
    break;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1013：ncclNetVersion
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (ncclNet == &ncclNet_v4_as_v5) ? 4 : 5;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__net_cc.md#f1013
 */
/* 中文逐项-函数 F1013：ncclNetVersion
 * 用途与位置：报告当前选中网络函数表是 v4 兼容还是原生 v5，供依赖接口能力的策略判断。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/paths.cc::ncclPxnDisable。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (ncclNet == &ncclNet_v4_as_v5) ? 4 : 5;。CHECK类宏还可能提前返回/跳转。
 */
int ncclNetVersion() {
  return (ncclNet == &ncclNet_v4_as_v5) ? 4 : 5;
}
