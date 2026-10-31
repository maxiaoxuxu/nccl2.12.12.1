/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "nccl.h"
#include "core.h"
#include "utils.h"
#include "bootstrap.h"
#include "net.h"
#include <unistd.h>
#include <sys/types.h>
#include "proxy.h"

/* Init functions */
/* 中文逐项-变量 V0003：bootstrapNetIfName
 * bootstrapNetIfName（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclFindInterfaceMatchSubnet(bootstrapNetIfName, &bootstrapNetIfAddr, &remoteAddr,
 * MAX_IF_NAME_SIZE, 1) <= 0) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static char bootstrapNetIfName[MAX_IF_NAME_SIZE+1];
/* 中文逐项-变量 V0004：bootstrapNetIfAddr
 * bootstrapNetIfAddr（union ncclSocketAddress；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclFindInterfaceMatchSubnet(bootstrapNetIfName, &bootstrapNetIfAddr, &remoteAddr,
 * MAX_IF_NAME_SIZE, 1) <= 0) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static union ncclSocketAddress bootstrapNetIfAddr;
/* 中文逐项-变量 V0005：bootstrapNetInitDone
 * bootstrapNetInitDone（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bootstrapNetInitDone = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
static int bootstrapNetInitDone = 0;
/* 中文逐项-变量 V0006：bootstrapNetLock
 * bootstrapNetLock（pthread_mutex_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bootstrapNetLock = PTHREAD_MUTEX_INITIALIZER；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
pthread_mutex_t bootstrapNetLock = PTHREAD_MUTEX_INITIALIZER;

/* 中文内部逻辑 F0031：bootstrapNetInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(bootstrapNetInitDone == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：pthread_mutex_lock(&bootstrapNetLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 条件判断：(bootstrapNetInitDone == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       5. 真分支：；仅条件为真执行以下子项。
 *         6. 准备局部状态：char* env = getenv("NCCL_COMM_ID");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         7. 条件判断：(env)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           8. 真分支：；仅条件为真执行以下子项。
 *             9. 准备局部状态：union ncclSocketAddress remoteAddr;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             10. 条件判断：(ncclGetSocketAddrFromString(&remoteAddr, env) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               11. 真分支：；仅条件为真执行以下子项。
 *                 12. 调用 / 计算 / 写入：WARN("Invalid NCCL_COMM_ID, please use format: <ipv4>:<port> or [<ipv6>]:<port> or <hostname>:<port>");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 13. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *             14. 条件判断：(ncclFindInterfaceMatchSubnet(bootstrapNetIfName, &bootstrapNetIfAddr, &remoteAddr, MAX_IF_NAME_SIZE, 1) <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               15. 真分支：；仅条件为真执行以下子项。
 *                 16. 调用 / 计算 / 写入：WARN("NET/Socket : No usable listening interface found");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 17. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *           18. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             19. 准备局部状态：int nIfs = ncclFindInterfaces(bootstrapNetIfName, &bootstrapNetIfAddr, MAX_IF_NAME_SIZE, 1);；按显式接口配置或默认优先级选择可用接口，过滤不合适的 loopback/docker 等候选。
 *             20. 条件判断：(nIfs <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               21. 真分支：；仅条件为真执行以下子项。
 *                 22. 调用 / 计算 / 写入：WARN("Bootstrap : no socket interface found");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 23. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *         24. 准备局部状态：char line[SOCKET_NAME_MAXLEN+MAX_IF_NAME_SIZE+2];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         25. 调用 / 计算 / 写入：sprintf(line, " %s:", bootstrapNetIfName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         26. 调用 / 计算 / 写入：ncclSocketToString(&bootstrapNetIfAddr, line+strlen(line));；把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。
 *         27. 调用 / 计算 / 写入：INFO(NCCL_INIT, "Bootstrap : Using%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         28. 调用 / 计算 / 写入：bootstrapNetInitDone = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     29. 调用 / 计算 / 写入：pthread_mutex_unlock(&bootstrapNetLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 30. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0031
 */
/* 中文逐项-函数 F0031：bootstrapNetInit
 * 用途与位置：选择初始化控制面使用的本机网络接口和地址；其选择不等于数据面必然使用 TCP。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/net.cc::ncclNetInit。
 * 本体直接出现的调用/包装：getenv；ncclFindInterfaceMatchSubnet；ncclFindInterfaces；ncclGetSocketAddrFromString；ncclSocketToString；pthread_mutex_lock；pthread_mutex_unlock；sprintf；strlen。
 * 需要对照的边界/推进条件：bootstrapNetInitDone == 0；bootstrapNetInitDone == 0；env；ncclGetSocketAddrFromString(&remoteAddr, env)
 * != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSystemError;；return ncclInternalError;；return
 * ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t bootstrapNetInit() {
  if (bootstrapNetInitDone == 0) {
    pthread_mutex_lock(&bootstrapNetLock);
    if (bootstrapNetInitDone == 0) {
/* 中文逐项-变量 V0007：env
 * env（char；函数 bootstrapNetInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 env = getenv("NCCL_COMM_ID")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      char* env = getenv("NCCL_COMM_ID");
      if (env) {
/* 中文逐项-变量 V0008：remoteAddr
 * remoteAddr（union ncclSocketAddress；函数 bootstrapNetInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclGetSocketAddrFromString(&remoteAddr, env) != ncclSuccess) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        union ncclSocketAddress remoteAddr;
        if (ncclGetSocketAddrFromString(&remoteAddr, env) != ncclSuccess) {
          WARN("Invalid NCCL_COMM_ID, please use format: <ipv4>:<port> or [<ipv6>]:<port> or <hostname>:<port>");
          return ncclInvalidArgument;
        }
        if (ncclFindInterfaceMatchSubnet(bootstrapNetIfName, &bootstrapNetIfAddr, &remoteAddr, MAX_IF_NAME_SIZE, 1) <= 0) {
          WARN("NET/Socket : No usable listening interface found");
          return ncclSystemError;
        }
      } else {
/* 中文逐项-变量 V0009：nIfs
 * nIfs（int；函数 bootstrapNetInit 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nIfs = ncclFindInterfaces(bootstrapNetIfName, &bootstrapNetIfAddr, MAX_IF_NAME_SIZE,
 * 1)；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
        int nIfs = ncclFindInterfaces(bootstrapNetIfName, &bootstrapNetIfAddr, MAX_IF_NAME_SIZE, 1);
        if (nIfs <= 0) {
          WARN("Bootstrap : no socket interface found");
          return ncclInternalError;
        }
      }
/* 中文逐项-变量 V0010：line
 * line（char；函数 bootstrapNetInit 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：sprintf(line, " %s:", bootstrapNetIfName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char line[SOCKET_NAME_MAXLEN+MAX_IF_NAME_SIZE+2];
      sprintf(line, " %s:", bootstrapNetIfName);
      ncclSocketToString(&bootstrapNetIfAddr, line+strlen(line));
      INFO(NCCL_INIT, "Bootstrap : Using%s", line);
      bootstrapNetInitDone = 1;
    }
    pthread_mutex_unlock(&bootstrapNetLock);
  }
  return ncclSuccess;
}

/* Socket Interface Selection type */
/* 中文逐项-类型 T0001：bootstrapInterface_t
 * 类型职责：bootstrap接口/会合摘要/未匹配控制连接及当前rank控制网络状态。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum bootstrapInterface_t { /* 中文逐项-枚举项 X0002：findSubnetIf
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：-1
 */findSubnetIf = -1, /* 中文逐项-枚举项 X0003：dontCareIf
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：-2
 */dontCareIf = -2 };

// Additional sync functions
/* 中文内部逻辑 F0032：bootstrapNetSend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(sock, &size, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketSend(sock, data, size));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0032
 */
/* 中文逐项-函数 F0032：bootstrapNetSend
 * 用途与位置：先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRoot；src/bootstrap.cc::bootstrapInit；src/bootstrap.cc::bootstrapAllGather；src/bootstrap.cc::bootstrapSend。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketSend。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t bootstrapNetSend(struct ncclSocket* sock, void* data, int size) {
  NCCLCHECK(ncclSocketSend(sock, &size, sizeof(int)));
  NCCLCHECK(ncclSocketSend(sock, data, size));
  return ncclSuccess;
}
/* 中文内部逻辑 F0033：bootstrapNetRecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int recvSize;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, &recvSize, sizeof(int)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 3. 条件判断：(recvSize > size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：WARN("Message truncated : received %d bytes instead of %d", recvSize, size);；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *     6. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketRecv(sock, data, std::min(recvSize, size)));；失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0033
 */
/* 中文逐项-函数 F0033：bootstrapNetRecv
 * 用途与位置：读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRoot；src/bootstrap.cc::bootstrapInit；src/bootstrap.cc::bootstrapAllGather；src/bootstrap.cc::bootstrapRecv。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketRecv；std::min。
 * 需要对照的边界/推进条件：recvSize > size。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t bootstrapNetRecv(struct ncclSocket* sock, void* data, int size) {
/* 中文逐项-变量 V0011：recvSize
 * recvSize（int；函数 bootstrapNetRecv 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketRecv(sock, &recvSize, sizeof(int)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int recvSize;
  NCCLCHECK(ncclSocketRecv(sock, &recvSize, sizeof(int)));
  if (recvSize > size) {
    WARN("Message truncated : received %d bytes instead of %d", recvSize, size);
    return ncclInternalError;
  }
  NCCLCHECK(ncclSocketRecv(sock, data, std::min(recvSize, size)));
  return ncclSuccess;
}

/* 中文逐项-类型 T0002：extInfo
 * 类型职责：bootstrap接口/会合摘要/未匹配控制连接及当前rank控制网络状态。
 * 声明内可见成员（含嵌套成员）：rank、nranks、extAddressListenRoot、extAddressListen。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct extInfo {
/* 中文逐项-变量 V0012：rank
 * rank（int；成员 extInfo）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V0013：nranks
 * nranks（int；成员 extInfo）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 */
  int nranks;
/* 中文逐项-变量 V0014：extAddressListenRoot
 * extAddressListenRoot（union ncclSocketAddress；成员 extInfo）：当前rank报告给bootstrap root的监听地址，用于root会合回复或环邻居连接建立。
 */
  union ncclSocketAddress extAddressListenRoot;
/* 中文逐项-变量 V0015：extAddressListen
 * extAddressListen（union ncclSocketAddress；成员 extInfo）：当前rank报告给bootstrap root的监听地址，用于root会合回复或环邻居连接建立。
 */
  union ncclSocketAddress extAddressListen;
};

#include <sys/resource.h>

/* 中文内部逻辑 F0034：setFilesLimit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct rlimit filesLimit;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：SYSCHECK(getrlimit(RLIMIT_NOFILE, &filesLimit), "getrlimit");；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：filesLimit.rlim_cur = filesLimit.rlim_max;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：SYSCHECK(setrlimit(RLIMIT_NOFILE, &filesLimit), "setrlimit");；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0034
 */
/* 中文逐项-函数 F0034：setFilesLimit
 * 用途与位置：尝试提高本进程可打开文件描述符上限，以容纳初始化与多连接使用的 socket。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRoot。
 * 本体直接出现的调用/包装：SYSCHECK；getrlimit；setrlimit。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t setFilesLimit() {
/* 中文逐项-变量 V0016：filesLimit
 * filesLimit（struct rlimit；函数 setFilesLimit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：SYSCHECK(getrlimit(RLIMIT_NOFILE, &filesLimit), "getrlimit");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct rlimit filesLimit;
  SYSCHECK(getrlimit(RLIMIT_NOFILE, &filesLimit), "getrlimit");
  filesLimit.rlim_cur = filesLimit.rlim_max;
  SYSCHECK(setrlimit(RLIMIT_NOFILE, &filesLimit), "setrlimit");
  return ncclSuccess;
}

/* 中文内部逻辑 F0035：bootstrapRoot
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocket* listenSock = (struct ncclSocket*)args;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：ncclResult_t res = ncclSuccess;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int nranks = 0, c = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：struct extInfo info;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：union ncclSocketAddress *rankAddresses = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：union ncclSocketAddress *rankAddressesRoot = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 准备局部状态：union ncclSocketAddress *zero = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclCalloc(&zero, 1), res, out);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 * 9. 调用 / 计算 / 写入：setFilesLimit();；尝试提高本进程可打开文件描述符上限，以容纳初始化与多连接使用的 socket。
 * 10. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "BEGIN");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 11. 先执行一次的循环：初始 ；继续条件 (c < nranks)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *   12. 准备局部状态：struct ncclSocket sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   13. 调用 / 计算 / 写入：sock.abortFlag = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   14. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSocketAccept(&sock, listenSock), res, out);；失败时保存错误并跳到清理标签，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。
 *   15. 调用 / 计算 / 写入：NCCLCHECKGOTO(bootstrapNetRecv(&sock, &info, sizeof(info)), res, out);；失败时保存错误并跳到清理标签，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。
 *   16. 调用 / 计算 / 写入：close(sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   17. 条件判断：(c == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     18. 真分支：；仅条件为真执行以下子项。
 *       19. 调用 / 计算 / 写入：nranks = info.nranks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       20. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclCalloc(&rankAddresses, nranks), res, out);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 *       21. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclCalloc(&rankAddressesRoot, nranks), res, out);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 *   22. 条件判断：(nranks != info.nranks)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     23. 真分支：；仅条件为真执行以下子项。
 *       24. 调用 / 计算 / 写入：WARN("Bootstrap Root : mismatch in rank count from procs %d : %d", nranks, info.nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       25. 控制转移：goto out;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *   26. 条件判断：(memcmp(zero, &rankAddressesRoot[info.rank], sizeof(union ncclSocketAddress)) != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     27. 真分支：；仅条件为真执行以下子项。
 *       28. 调用 / 计算 / 写入：WARN("Bootstrap Root : rank %d of %d ranks has already checked in", info.rank, nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       29. 控制转移：goto out;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *   30. 调用 / 计算 / 写入：memcpy(rankAddressesRoot+info.rank, &info.extAddressListenRoot, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   31. 调用 / 计算 / 写入：memcpy(rankAddresses+info.rank, &info.extAddressListen, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   32. 调用 / 计算 / 写入：++c;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   33. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "Received connect from rank %d total %d/%d", info.rank, c, nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 34. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 35. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "COLLECTED ALL %d HANDLES", nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 36. 循环：初始 int r=0;；继续条件 r<nranks；每轮更新 ++r；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   37. 准备局部状态：int next = (r+1) % nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   38. 准备局部状态：struct ncclSocket sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   39. 调用 / 计算 / 写入：sock.abortFlag = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   40. 调用 / 计算 / 写入：memcpy(&sock.addr, rankAddressesRoot+r, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   41. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclSocketConnect(&sock), res, out);；失败时保存错误并跳到清理标签，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 *   42. 调用 / 计算 / 写入：NCCLCHECKGOTO(bootstrapNetSend(&sock, rankAddresses+next, sizeof(union ncclSocketAddress)), res, out);；失败时保存错误并跳到清理标签，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。
 *   43. 调用 / 计算 / 写入：close(sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 44. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 45. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "SENT OUT ALL %d HANDLES", nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 46. 跳转标签：out；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   47. 调用 / 计算 / 写入：close(listenSock->fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 48. 调用 / 计算 / 写入：free(listenSock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 49. 条件判断：(rankAddresses)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   50. 真分支：；仅条件为真执行以下子项。
 *     51. 调用 / 计算 / 写入：free(rankAddresses);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 52. 条件判断：(rankAddressesRoot)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   53. 真分支：；仅条件为真执行以下子项。
 *     54. 调用 / 计算 / 写入：free(rankAddressesRoot);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 55. 条件判断：(zero)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   56. 真分支：；仅条件为真执行以下子项。
 *     57. 调用 / 计算 / 写入：free(zero);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 58. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "DONE");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 59. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0035
 */
/* 中文逐项-函数 F0035：bootstrapRoot
 * 用途与位置：收集所有 rank 的会合信息并分发环邻居地址；这是 CPU 控制服务，不处理张量归约。
 * 参数 args（void* args）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECKGOTO；bootstrapNetRecv；bootstrapNetSend；close；free；memcmp；memcpy；ncclCalloc；ncclSocketAccept；ncclSocketConnect；setFilesLimit。
 * 需要对照的边界/推进条件：c == 0；nranks != info.nranks；memcmp(zero, &rankAddressesRoot[info.rank], sizeof(union
 * ncclSocketAddress)) != 0；r<nranks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;。CHECK类宏还可能提前返回/跳转。
 */
static void *bootstrapRoot(void* args) {
/* 中文逐项-变量 V0017：listenSock
 * listenSock（struct ncclSocket；函数 bootstrapRoot 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 listenSock = (struct ncclSocket*)args；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocket* listenSock = (struct ncclSocket*)args;
/* 中文逐项-变量 V0018：res
 * res（ncclResult_t；函数 bootstrapRoot 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  ncclResult_t res = ncclSuccess;
/* 中文逐项-变量 V0019：nranks、c
 * nranks（int；函数 bootstrapRoot 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * c（int；函数 bootstrapRoot 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int nranks = 0, c = 0;
/* 中文逐项-变量 V0020：info
 * info（struct extInfo；函数 bootstrapRoot 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(bootstrapNetRecv(&sock, &info, sizeof(info)), res, out);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct extInfo info;
/* 中文逐项-变量 V0021：rankAddresses
 * rankAddresses（union ncclSocketAddress；函数 bootstrapRoot 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rankAddresses = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  union ncclSocketAddress *rankAddresses = NULL;
/* 中文逐项-变量 V0022：rankAddressesRoot
 * rankAddressesRoot（union ncclSocketAddress；函数 bootstrapRoot 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rankAddressesRoot = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  union ncclSocketAddress *rankAddressesRoot = NULL; // for initial rank <-> root information exchange
/* 中文逐项-变量 V0023：zero
 * zero（union ncclSocketAddress；函数 bootstrapRoot 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 zero = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  union ncclSocketAddress *zero = NULL;
  NCCLCHECKGOTO(ncclCalloc(&zero, 1), res, out);
  setFilesLimit();

  TRACE(NCCL_INIT, "BEGIN");
  /* Receive addresses from all ranks */
  do {
/* 中文逐项-变量 V0024：sock
 * sock（struct ncclSocket；函数 bootstrapRoot 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 此处未显式给初值；后续首个文本使用点：sock.abortFlag = NULL;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclSocket sock;
    sock.abortFlag = NULL;
    NCCLCHECKGOTO(ncclSocketAccept(&sock, listenSock), res, out);
    NCCLCHECKGOTO(bootstrapNetRecv(&sock, &info, sizeof(info)), res, out);
    close(sock.fd);

    if (c == 0) {
      nranks = info.nranks;
      NCCLCHECKGOTO(ncclCalloc(&rankAddresses, nranks), res, out);
      NCCLCHECKGOTO(ncclCalloc(&rankAddressesRoot, nranks), res, out);
    }

    if (nranks != info.nranks) {
      WARN("Bootstrap Root : mismatch in rank count from procs %d : %d", nranks, info.nranks);
      goto out;
    }

    if (memcmp(zero, &rankAddressesRoot[info.rank], sizeof(union ncclSocketAddress)) != 0) {
      WARN("Bootstrap Root : rank %d of %d ranks has already checked in", info.rank, nranks);
      goto out;
    }

    // Save the connection handle for that rank
    memcpy(rankAddressesRoot+info.rank, &info.extAddressListenRoot, sizeof(union ncclSocketAddress));
    memcpy(rankAddresses+info.rank, &info.extAddressListen, sizeof(union ncclSocketAddress));

    ++c;
    TRACE(NCCL_INIT, "Received connect from rank %d total %d/%d",  info.rank, c, nranks);
  } while (c < nranks);
  TRACE(NCCL_INIT, "COLLECTED ALL %d HANDLES", nranks);

  // Send the connect handle for the next rank in the AllGather ring
  for (/* 中文逐项-变量 V0025：r
 * r（int；函数 bootstrapRoot 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nranks；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nranks; ++r) {
/* 中文逐项-变量 V0026：next
 * next（int；函数 bootstrapRoot 内）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 初值/来源 next = (r+1) % nranks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int next = (r+1) % nranks;
/* 中文逐项-变量 V0027：sock
 * sock（struct ncclSocket；函数 bootstrapRoot 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 此处未显式给初值；后续首个文本使用点：sock.abortFlag = NULL;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclSocket sock;
    sock.abortFlag = NULL;
    memcpy(&sock.addr, rankAddressesRoot+r, sizeof(union ncclSocketAddress));
    NCCLCHECKGOTO(ncclSocketConnect(&sock), res, out);
    NCCLCHECKGOTO(bootstrapNetSend(&sock, rankAddresses+next, sizeof(union ncclSocketAddress)), res, out);
    close(sock.fd);
  }
  TRACE(NCCL_INIT, "SENT OUT ALL %d HANDLES", nranks);

out:
  close(listenSock->fd);
  free(listenSock);
  if (rankAddresses) free(rankAddresses);
  if (rankAddressesRoot) free(rankAddressesRoot);
  if (zero) free(zero);

  TRACE(NCCL_INIT, "DONE");
  return NULL;
}

/* 中文内部逻辑 F0036：bootstrapCreateRoot
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocket* listenSock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&listenSock, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：memcpy(&listenSock->addr, id, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketListen(listenSock));；失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。
 * 5. 调用 / 计算 / 写入：memcpy(id, &listenSock->addr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 准备局部状态：pthread_t thread;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 调用 / 计算 / 写入：pthread_create(&thread, NULL, bootstrapRoot, (void*)listenSock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 调用 / 计算 / 写入：ncclSetThreadName(thread, "NCCL BootstrapR");；为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。
 * 9. 调用 / 计算 / 写入：pthread_detach(thread);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0036
 */
/* 中文逐项-函数 F0036：bootstrapCreateRoot
 * 用途与位置：创建 bootstrap root 监听对象和后台线程，为后续 rank 会合提供入口。
 * 参数 id（ncclUniqueId* id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 idFromEnv（bool idFromEnv）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapGetUniqueId；src/init.cc::ncclCommInitRankDev。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclCalloc；ncclSetThreadName；ncclSocketListen；pthread_create；pthread_detach。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapCreateRoot(ncclUniqueId* id, bool idFromEnv) {
/* 中文逐项-变量 V0028：listenSock
 * listenSock（struct ncclSocket；函数 bootstrapCreateRoot 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&listenSock, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclSocket* listenSock;
  NCCLCHECK(ncclCalloc(&listenSock, 1));
  memcpy(&listenSock->addr, id, sizeof(union ncclSocketAddress));
  NCCLCHECK(ncclSocketListen(listenSock));
  memcpy(id, &listenSock->addr, sizeof(union ncclSocketAddress));
/* 中文逐项-变量 V0029：thread
 * thread（pthread_t；函数 bootstrapCreateRoot 内）：后台线程句柄和其任务/停止状态，资源回收前需要停止并join。
 * 此处未显式给初值；后续首个文本使用点：pthread_create(&thread, NULL, bootstrapRoot, (void*)listenSock);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  pthread_t thread;
  pthread_create(&thread, NULL, bootstrapRoot, (void*)listenSock);
  ncclSetThreadName(thread, "NCCL BootstrapR");
  pthread_detach(thread); // will not be pthread_join()'d
  return ncclSuccess;
}

/* 中文导读：
 * 生成供所有 rank 共同使用的 bootstrap 标识，包含建立初始联络所需的信息。
 * 应用仍需通过 MPI、socket 或其他机制分发此 ID；它不是 RDMA QPN，也不直接包含所有 GPU 数据连接。
 */
/* 中文精读：uniqueId 是引导会合信息
 * 为初始化控制服务准备可分发的标识/地址信息；应用负责把同一份 id 发给所有参与 rank。
 * 它不是全体数据 QP 的连接信息，也不装有每个 GPU 的用户张量地址。
 * 各 rank 先联系 root 获取环邻居信息，再用 bootstrap 环交换设备、拓扑与 transport 建链摘要。
 * 后续数据面可走 GPU P2P、SHM、IB 或 Socket，不受“bootstrap 使用 TCP”这一点限制。
 */
/* 中文内部逻辑 F0037：bootstrapGetUniqueId
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 源码语法片段：static_assert(sizeof(union ncclSocketAddress) < sizeof(ncclUniqueId), "NetId does not fit inside ncclUniqueId");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 2. 调用 / 计算 / 写入：memset(id, 0, sizeof(ncclUniqueId));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 3. 准备局部状态：union ncclSocketAddress* connectAddr = (union ncclSocketAddress*) id;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：char* env = getenv("NCCL_COMM_ID");；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(env)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", env);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     8. 条件判断：(ncclGetSocketAddrFromString(connectAddr, env) != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       9. 真分支：；仅条件为真执行以下子项。
 *         10. 调用 / 计算 / 写入：WARN("Invalid NCCL_COMM_ID, please use format: <ipv4>:<port> or [<ipv6>]:<port> or <hostname>:<port>");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         11. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   12. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     13. 调用 / 计算 / 写入：memcpy(id, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     14. 调用 / 计算 / 写入：NCCLCHECK(bootstrapCreateRoot(id, false));；失败时可能提前return，成功才继续；创建 bootstrap root 监听对象和后台线程，为后续 rank 会合提供入口。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0037
 */
/* 中文逐项-函数 F0037：bootstrapGetUniqueId
 * 用途与位置：生成/读取引导会合标识供应用分发，必要时建立 root 服务。
 * 参数 id（ncclUniqueId* id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclGetUniqueId。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapCreateRoot；getenv；memcpy；memset；ncclGetSocketAddrFromString。
 * 需要对照的边界/推进条件：env；ncclGetSocketAddrFromString(connectAddr, env) != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapGetUniqueId(ncclUniqueId* id) {
  static_assert(sizeof(union ncclSocketAddress) < sizeof(ncclUniqueId), "NetId does not fit inside ncclUniqueId");
  memset(id, 0, sizeof(ncclUniqueId));
/* 中文逐项-变量 V0030：connectAddr
 * connectAddr（union ncclSocketAddress；函数 bootstrapGetUniqueId 内）：非阻塞网络建链目标地址，重试仍沿用同一连接状态。
 * 初值/来源 connectAddr = (union ncclSocketAddress*) id；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  union ncclSocketAddress* connectAddr = (union ncclSocketAddress*) id;

/* 中文逐项-变量 V0031：env
 * env（char；函数 bootstrapGetUniqueId 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 env = getenv("NCCL_COMM_ID")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* env = getenv("NCCL_COMM_ID");
  if (env) {
    INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", env);
    if (ncclGetSocketAddrFromString(connectAddr, env) != ncclSuccess) {
      WARN("Invalid NCCL_COMM_ID, please use format: <ipv4>:<port> or [<ipv6>]:<port> or <hostname>:<port>");
      return ncclInvalidArgument;
    }
  } else {
    memcpy(id, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));
    NCCLCHECK(bootstrapCreateRoot(id, false));
  }

  return ncclSuccess;
}

/* 中文逐项-类型 T0003：unexConn
 * 类型职责：bootstrap接口/会合摘要/未匹配控制连接及当前rank控制网络状态。
 * 声明内可见成员（含嵌套成员）：peer、tag、sock、next。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct unexConn {
/* 中文逐项-变量 V0032：peer
 * peer（int；成员 unexConn）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 */
  int peer;
/* 中文逐项-变量 V0033：tag
 * tag（int；成员 unexConn）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 */
  int tag;
/* 中文逐项-变量 V0034：sock
 * sock（struct ncclSocket；成员 unexConn）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket sock;
/* 中文逐项-变量 V0035：next
 * next（struct unexConn；成员 unexConn）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
  struct unexConn* next;
};

/* 中文逐项-类型 T0004：bootstrapState
 * 类型职责：bootstrap接口/会合摘要/未匹配控制连接及当前rank控制网络状态。
 * 声明内可见成员（含嵌套成员）：listenSock、ringRecvSocket、ringSendSocket、peerCommAddresses、peerProxyAddresses、unexpectedConnections、cudaDev、rank、nranks、abortFlag。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct bootstrapState {
/* 中文逐项-变量 V0036：listenSock
 * listenSock（struct ncclSocket；成员 bootstrapState）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket listenSock;
/* 中文逐项-变量 V0037：ringRecvSocket
 * ringRecvSocket（struct ncclSocket；成员 bootstrapState）：bootstrap控制环的上游接收/下游发送socket，固定元数据AllGather沿它们轮转。
 */
  struct ncclSocket ringRecvSocket;
/* 中文逐项-变量 V0038：ringSendSocket
 * ringSendSocket（struct ncclSocket；成员 bootstrapState）：bootstrap控制环的上游接收/下游发送socket，固定元数据AllGather沿它们轮转。
 */
  struct ncclSocket ringSendSocket;
/* 中文逐项-变量 V0039：peerCommAddresses
 * peerCommAddresses（union ncclSocketAddress；成员 bootstrapState）：全组rank的控制通信/代理服务监听地址表，bootstrap汇总后可直接联系目标。
 */
  union ncclSocketAddress* peerCommAddresses;
/* 中文逐项-变量 V0040：peerProxyAddresses
 * peerProxyAddresses（union ncclSocketAddress；成员 bootstrapState）：全组rank的控制通信/代理服务监听地址表，bootstrap汇总后可直接联系目标。
 */
  union ncclSocketAddress* peerProxyAddresses;
/* 中文逐项-变量 V0041：unexpectedConnections
 * unexpectedConnections（struct unexConn；成员 bootstrapState）：暂时不匹配当前peer/tag的已到控制连接链，后续匹配接收时再取出。
 */
  struct unexConn* unexpectedConnections;
/* 中文逐项-变量 V0042：cudaDev
 * cudaDev（int；成员 bootstrapState）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 */
  int cudaDev;
/* 中文逐项-变量 V0043：rank
 * rank（int；成员 bootstrapState）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 */
  int rank;
/* 中文逐项-变量 V0044：nranks
 * nranks（int；成员 bootstrapState）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 */
  int nranks;
/* 中文逐项-变量 V0045：abortFlag
 * abortFlag（uint32_t；成员 bootstrapState）：主机/设备可观察的异常中止共享标志，用于打破协议或 socket 等待。
 */
  volatile uint32_t *abortFlag;
};

/* 中文导读：
 * 借助 root 联络各 rank，建立 bootstrap 的 socket 通道，并准备后续点对点元数据交换。
 * 它解决如何先认识其他 rank；实际 P2P/SHM/NET 数据通路在后面另建。
 */
/* 中文内部逻辑 F0038：bootstrapInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int rank = comm->rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int nranks = comm->nRanks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct bootstrapState* state;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&state, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 5. 调用 / 计算 / 写入：state->rank = rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 6. 调用 / 计算 / 写入：state->nranks = nranks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：state->abortFlag = comm->abortFlag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：comm->bootstrap = state;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d", rank, nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 准备局部状态：struct extInfo info = { 0 };；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 11. 调用 / 计算 / 写入：info.rank = rank;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 12. 调用 / 计算 / 写入：info.nranks = nranks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 准备局部状态：struct ncclSocket sock, listenSockRoot;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 14. 调用 / 计算 / 写入：sock.abortFlag = listenSockRoot.abortFlag = comm->abortFlag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：sock.asyncFlag = listenSockRoot.asyncFlag = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 调用 / 计算 / 写入：memcpy(&state->listenSock.addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 17. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketListen(&state->listenSock));；失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。
 * 18. 调用 / 计算 / 写入：memcpy(&info.extAddressListen, &state->listenSock.addr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 19. 调用 / 计算 / 写入：memcpy(&listenSockRoot.addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 20. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketListen(&listenSockRoot));；失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。
 * 21. 调用 / 计算 / 写入：memcpy(&info.extAddressListenRoot, &listenSockRoot.addr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 22. 条件判断：(nranks > 128)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   23. 真分支：；仅条件为真执行以下子项。
 *     24. 准备局部状态：long msec = rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     25. 准备局部状态：struct timespec tv;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     26. 调用 / 计算 / 写入：tv.tv_sec = msec / 1000;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     27. 调用 / 计算 / 写入：tv.tv_nsec = 1000000 * (msec % 1000);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     28. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d delaying connection to root by %ld msec", rank, msec);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     29. 调用 / 计算 / 写入：(void) nanosleep(&tv, NULL);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 30. 调用 / 计算 / 写入：memcpy(&sock.addr, id, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 31. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketConnect(&sock));；失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 * 32. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetSend(&sock, &info, sizeof(info)));；失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。
 * 33. 调用 / 计算 / 写入：close(sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 34. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketAccept(&sock, &listenSockRoot));；失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。
 * 35. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetRecv(&sock, &state->ringSendSocket.addr, sizeof(union ncclSocketAddress)));；失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。
 * 36. 调用 / 计算 / 写入：close(sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 37. 调用 / 计算 / 写入：close(listenSockRoot.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 38. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketConnect(&state->ringSendSocket));；失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 * 39. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketAccept(&state->ringRecvSocket, &state->listenSock));；失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。
 * 40. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&state->peerCommAddresses, nranks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 41. 调用 / 计算 / 写入：memcpy(state->peerCommAddresses+rank, &state->listenSock.addr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 42. 调用 / 计算 / 写入：NCCLCHECK(bootstrapAllGather(state, state->peerCommAddresses, sizeof(union ncclSocketAddress)));；失败时可能提前return，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。
 * 43. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&state->peerProxyAddresses, nranks));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 44. 准备局部状态：struct ncclSocket* proxySocket;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 45. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&proxySocket, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 46. 调用 / 计算 / 写入：proxySocket->abortFlag = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 47. 调用 / 计算 / 写入：memcpy(&proxySocket->addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 48. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketListen(proxySocket));；失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。
 * 49. 调用 / 计算 / 写入：memcpy(state->peerProxyAddresses+rank, &proxySocket->addr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 50. 调用 / 计算 / 写入：NCCLCHECK(bootstrapAllGather(state, state->peerProxyAddresses, sizeof(union ncclSocketAddress)));；失败时可能提前return，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。
 * 51. 调用 / 计算 / 写入：NCCLCHECK(ncclProxyInit(comm, proxySocket, state->peerProxyAddresses));；失败时可能提前return，成功才继续；将 bootstrap 建立的监听 socket 和全组代理地址表接入当前 communicator 的 Proxy 状态。
 * 52. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d - DONE", rank, nranks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 53. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0038
 */
/* 中文逐项-函数 F0038：bootstrapInit
 * 用途与位置：让当前 rank 加入控制网络并交换 peer/proxy 地址，为拓扑发现和传输建链准备控制通道。
 * 参数 id（ncclUniqueId * id）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 comm（struct ncclComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapAllGather；bootstrapNetRecv；bootstrapNetSend；close；memcpy；nanosleep；ncclCalloc；ncclProxyInit；ncclSocketAccept；ncclSocketConnect；ncclSocketListen。
 * 需要对照的边界/推进条件：nranks > 128。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapInit(ncclUniqueId * id, struct ncclComm* comm) {
/* 中文逐项-变量 V0046：rank
 * rank（int；函数 bootstrapInit 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = comm->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = comm->rank;
/* 中文逐项-变量 V0047：nranks
 * nranks（int；函数 bootstrapInit 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = comm->nRanks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nranks = comm->nRanks;
/* 中文逐项-变量 V0048：state
 * state（struct bootstrapState；函数 bootstrapInit 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&state, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct bootstrapState* state;
  NCCLCHECK(ncclCalloc(&state, 1));
  state->rank = rank;
  state->nranks = nranks;
  state->abortFlag = comm->abortFlag;
  comm->bootstrap = state;

  TRACE(NCCL_INIT, "rank %d nranks %d", rank, nranks);

/* 中文逐项-变量 V0049：info
 * info（struct extInfo；函数 bootstrapInit 内）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 初值/来源 info = { 0 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
  struct extInfo info = { 0 };
  info.rank = rank;
  info.nranks = nranks;
/* 中文逐项-变量 V0050：sock、listenSockRoot
 * sock（struct ncclSocket；函数 bootstrapInit 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 此处未显式给初值；后续首个文本使用点：sock.abortFlag = listenSockRoot.abortFlag = comm->abortFlag;。若由输出参数填入，必须先检查填充调用成功再读值。
 * listenSockRoot（struct ncclSocket；函数 bootstrapInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sock.abortFlag = listenSockRoot.abortFlag = comm->abortFlag;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclSocket sock, listenSockRoot;
  sock.abortFlag = listenSockRoot.abortFlag = comm->abortFlag;
  sock.asyncFlag = listenSockRoot.asyncFlag = 0;

  // Create socket for other ranks to contact me
  memcpy(&state->listenSock.addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));
  NCCLCHECK(ncclSocketListen(&state->listenSock));
  memcpy(&info.extAddressListen, &state->listenSock.addr, sizeof(union ncclSocketAddress));

  // Create socket for root to contact me
  memcpy(&listenSockRoot.addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));
  NCCLCHECK(ncclSocketListen(&listenSockRoot));
  memcpy(&info.extAddressListenRoot, &listenSockRoot.addr, sizeof(union ncclSocketAddress));

  // stagger connection times to avoid an overload of the root
  if (nranks > 128) {
/* 中文逐项-变量 V0051：msec
 * msec（long；函数 bootstrapInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 msec = rank；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    long msec = rank;
/* 中文逐项-变量 V0052：tv
 * tv（struct timespec；函数 bootstrapInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：tv.tv_sec = msec / 1000;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct timespec tv;
    tv.tv_sec = msec / 1000;
    tv.tv_nsec = 1000000 * (msec % 1000);
    TRACE(NCCL_INIT, "rank %d delaying connection to root by %ld msec", rank, msec);
    (void) nanosleep(&tv, NULL);
  }

  // send info on my listening socket to root
  memcpy(&sock.addr, id, sizeof(union ncclSocketAddress));
  NCCLCHECK(ncclSocketConnect(&sock));
  NCCLCHECK(bootstrapNetSend(&sock, &info, sizeof(info)));
  close(sock.fd);

  // get info on my "next" rank in the bootstrap ring from root
  NCCLCHECK(ncclSocketAccept(&sock, &listenSockRoot));
  NCCLCHECK(bootstrapNetRecv(&sock, &state->ringSendSocket.addr, sizeof(union ncclSocketAddress)));
  close(sock.fd);
  close(listenSockRoot.fd);

  NCCLCHECK(ncclSocketConnect(&state->ringSendSocket));
  // Accept the connect request from the previous rank in the AllGather ring
  NCCLCHECK(ncclSocketAccept(&state->ringRecvSocket, &state->listenSock));

  // AllGather all listen handlers
  NCCLCHECK(ncclCalloc(&state->peerCommAddresses, nranks));
  memcpy(state->peerCommAddresses+rank, &state->listenSock.addr, sizeof(union ncclSocketAddress));
  NCCLCHECK(bootstrapAllGather(state, state->peerCommAddresses, sizeof(union ncclSocketAddress)));

  // Create the service proxy
  NCCLCHECK(ncclCalloc(&state->peerProxyAddresses, nranks));
/* 中文逐项-变量 V0053：proxySocket
 * proxySocket（struct ncclSocket；函数 bootstrapInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&proxySocket, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclSocket* proxySocket;
  NCCLCHECK(ncclCalloc(&proxySocket, 1));
  proxySocket->abortFlag = NULL; // proxy is aborted through a message
  memcpy(&proxySocket->addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));
  NCCLCHECK(ncclSocketListen(proxySocket));
  memcpy(state->peerProxyAddresses+rank, &proxySocket->addr, sizeof(union ncclSocketAddress));
  NCCLCHECK(bootstrapAllGather(state, state->peerProxyAddresses, sizeof(union ncclSocketAddress)));
  NCCLCHECK(ncclProxyInit(comm, proxySocket, state->peerProxyAddresses));

  TRACE(NCCL_INIT, "rank %d nranks %d - DONE", rank, nranks);

  return ncclSuccess;
}

/* 中文导读：
 * 在 bootstrap 环上交换每个 rank 的固定大小元数据，供初始化阶段汇总信息。
 * 此函数与 src/collectives/all_gather.cc 的用户数据集合通信是两套用途不同的实现。
 */
/* 中文内部逻辑 F0039：bootstrapAllGather
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct bootstrapState* state = (struct bootstrapState*)commState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：char* data = (char*)allData;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：int rank = state->rank;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int nranks = state->nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d size %d", rank, nranks, size);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 循环：初始 int i=0;；继续条件 i<nranks-1；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 准备局部状态：size_t rslice = (rank - i - 1 + nranks) % nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 准备局部状态：size_t sslice = (rank - i + nranks) % nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   9. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetSend(&state->ringSendSocket, data+sslice*size, size));；失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。
 *   10. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetRecv(&state->ringRecvSocket, data+rslice*size, size));；失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。
 * 11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 12. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d size %d - DONE", rank, nranks, size);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0039
 */
/* 中文逐项-函数 F0039：bootstrapAllGather
 * 用途与位置：沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。
 * 参数 commState（void* commState）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 allData（void* allData）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapInit；src/init.cc::initTransportsRank；src/transport.cc::ncclTransportCollNetSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapNetRecv；bootstrapNetSend。
 * 需要对照的边界/推进条件：i<nranks-1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapAllGather(void* commState, void* allData, int size) {
/* 中文逐项-变量 V0054：state
 * state（struct bootstrapState；函数 bootstrapAllGather 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = (struct bootstrapState*)commState；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct bootstrapState* state = (struct bootstrapState*)commState;
/* 中文逐项-变量 V0055：data
 * data（char；函数 bootstrapAllGather 内）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 初值/来源 data = (char*)allData；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  char* data = (char*)allData;
/* 中文逐项-变量 V0056：rank
 * rank（int；函数 bootstrapAllGather 内）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 初值/来源 rank = state->rank；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int rank = state->rank;
/* 中文逐项-变量 V0057：nranks
 * nranks（int；函数 bootstrapAllGather 内）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 初值/来源 nranks = state->nranks；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nranks = state->nranks;

  TRACE(NCCL_INIT, "rank %d nranks %d size %d", rank, nranks, size);

  /* Simple ring based AllGather
   * At each step i receive data from (rank-i-1) from left
   * and send previous step's data from (rank-i) to right
   */
  for (/* 中文逐项-变量 V0058：i
 * i（int；函数 bootstrapAllGather 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<nranks-1；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<nranks-1; i++) {
/* 中文逐项-变量 V0059：rslice
 * rslice（size_t；函数 bootstrapAllGather 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rslice = (rank - i - 1 + nranks) % nranks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    size_t rslice = (rank - i - 1 + nranks) % nranks;
/* 中文逐项-变量 V0060：sslice
 * sslice（size_t；函数 bootstrapAllGather 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sslice = (rank - i + nranks) % nranks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    size_t sslice = (rank - i + nranks) % nranks;

    // Send slice to the right
    NCCLCHECK(bootstrapNetSend(&state->ringSendSocket, data+sslice*size, size));
    // Recv slice from the left
    NCCLCHECK(bootstrapNetRecv(&state->ringRecvSocket, data+rslice*size, size));
  }

  TRACE(NCCL_INIT, "rank %d nranks %d size %d - DONE", rank, nranks, size);
  return ncclSuccess;
}

/* 中文内部逻辑 F0040：bootstrapSend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct bootstrapState* state = (struct bootstrapState*)commState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclSocket sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：sock.abortFlag = state->abortFlag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：memcpy(&sock.addr, state->peerCommAddresses+peer, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketConnect(&sock));；失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 * 6. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetSend(&sock, &state->rank, sizeof(int)));；失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。
 * 7. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetSend(&sock, &tag, sizeof(int)));；失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetSend(&sock, data, size));；失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。
 * 9. 调用 / 计算 / 写入：close(sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0040
 */
/* 中文逐项-函数 F0040：bootstrapSend
 * 用途与位置：向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。
 * 参数 commState（void* commState）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapBarrier；src/bootstrap.cc::bootstrapIntraNodeAllGather；src/transport.cc::ncclTransportP2pSetup；src/transport.cc::ncclTransportCollNetSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapNetSend；close；memcpy；ncclSocketConnect。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapSend(void* commState, int peer, int tag, void* data, int size) {
/* 中文逐项-变量 V0061：state
 * state（struct bootstrapState；函数 bootstrapSend 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = (struct bootstrapState*)commState；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct bootstrapState* state = (struct bootstrapState*)commState;
/* 中文逐项-变量 V0062：sock
 * sock（struct ncclSocket；函数 bootstrapSend 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 此处未显式给初值；后续首个文本使用点：sock.abortFlag = state->abortFlag;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclSocket sock;
  sock.abortFlag = state->abortFlag;
  memcpy(&sock.addr, state->peerCommAddresses+peer, sizeof(union ncclSocketAddress));
  NCCLCHECK(ncclSocketConnect(&sock));
  NCCLCHECK(bootstrapNetSend(&sock, &state->rank, sizeof(int)));
  NCCLCHECK(bootstrapNetSend(&sock, &tag, sizeof(int)));
  NCCLCHECK(bootstrapNetSend(&sock, data, size));
  close(sock.fd);
  return ncclSuccess;
}

/* 中文精读：初始化屏障的范围和代价
 * 用距离按 2 的幂增加的点对点控制交换完成所选 ranks 集合的会合。
 * 参数 rank 是该集合中的局部下标，ranks 数组再把它映射到 communicator 的全局编号。
 * 它用于初始化/资源协调，不是 CUDA stream 上用户计算的同步工具。
 * 即使此函数返回，也不能推出之后排队的 GPU collective 已经执行完成。
 */
/* 中文内部逻辑 F0041：bootstrapBarrier
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(nranks == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d tag %x - ENTER", rank, nranks, tag);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 准备局部状态：int data[1];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 循环：初始 int mask=1;；继续条件 mask<nranks；每轮更新 mask<<=1；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 准备局部状态：int src = (rank - mask + nranks) % nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 准备局部状态：int dst = (rank + mask) % nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   9. 调用 / 计算 / 写入：NCCLCHECK(bootstrapSend(commState, ranks[dst], tag, data, sizeof(data)));；失败时可能提前return，成功才继续；向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。
 *   10. 调用 / 计算 / 写入：NCCLCHECK(bootstrapRecv(commState, ranks[src], tag, data, sizeof(data)));；失败时可能提前return，成功才继续；优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。
 * 11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 12. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d tag %x - DONE", rank, nranks, tag);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0041
 */
/* 中文逐项-函数 F0041：bootstrapBarrier
 * 用途与位置：使用倍增距离的控制消息交换，使所选 rank 子集合完成初始化会合。
 * 参数 commState（void* commState）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ranks（int *ranks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::initTransportsRank。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapRecv；bootstrapSend。
 * 需要对照的边界/推进条件：nranks == 1；mask<nranks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapBarrier(void* commState, int *ranks, int rank, int nranks, int tag) {
  if (nranks == 1) return ncclSuccess;
  TRACE(NCCL_INIT, "rank %d nranks %d tag %x - ENTER", rank, nranks, tag);

  /* Simple intra process barrier
   *
   * Based on the dissemination algorithm by Debra Hensgen, Raphael Finkel, and Udi Manbet,
   * "Two Algorithms for Barrier Synchronization," International Journal of Parallel Programming, 17(1):1-17, 1988"
   */
/* 中文逐项-变量 V0063：data
 * data（int；函数 bootstrapBarrier 内）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(bootstrapSend(commState, ranks[dst], tag, data,
 * sizeof(data)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int data[1];
  for (/* 中文逐项-变量 V0064：mask
 * mask（int；函数 bootstrapBarrier 内）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 * 初值/来源 mask = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：mask<nranks；循环体与增量表达式决定每轮如何推进。
 */int mask=1; mask<nranks; mask<<=1) {
/* 中文逐项-变量 V0065：src
 * src（int；函数 bootstrapBarrier 内）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 初值/来源 src = (rank - mask + nranks) % nranks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int src = (rank - mask + nranks) % nranks;
/* 中文逐项-变量 V0066：dst
 * dst（int；函数 bootstrapBarrier 内）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 初值/来源 dst = (rank + mask) % nranks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int dst = (rank + mask) % nranks;
    NCCLCHECK(bootstrapSend(commState, ranks[dst], tag, data, sizeof(data)));
    NCCLCHECK(bootstrapRecv(commState, ranks[src], tag, data, sizeof(data)));
  }

  TRACE(NCCL_INIT, "rank %d nranks %d tag %x - DONE", rank, nranks, tag);
  return ncclSuccess;
}

/* 中文内部逻辑 F0042：bootstrapIntraNodeAllGather
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(nranks == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：char* data = (char*)allData;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d size %d - ENTER", rank, nranks, size);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 循环：初始 int i=1;；继续条件 i<nranks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 准备局部状态：int src = (rank - i + nranks) % nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 准备局部状态：int dst = (rank + i) % nranks;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   9. 调用 / 计算 / 写入：NCCLCHECK(bootstrapSend(commState, ranks[dst], i, data+rank*size, size));；失败时可能提前return，成功才继续；向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。
 *   10. 调用 / 计算 / 写入：NCCLCHECK(bootstrapRecv(commState, ranks[src], i, data+src*size, size));；失败时可能提前return，成功才继续；优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。
 * 11. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 12. 调用 / 计算 / 写入：TRACE(NCCL_INIT, "rank %d nranks %d size %d - DONE", rank, nranks, size);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0042
 */
/* 中文逐项-函数 F0042：bootstrapIntraNodeAllGather
 * 用途与位置：只在所选节点内 rank 集合交换固定大小控制信息，按局部索引映射到全局 rank。
 * 参数 commState（void* commState）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ranks（int *ranks）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 allData（void* allData）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclRegBuffAndExchange；src/init.cc::initTransportsRank；src/transport.cc::ncclTransportCollNetCheck。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapRecv；bootstrapSend。
 * 需要对照的边界/推进条件：nranks == 1；i<nranks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapIntraNodeAllGather(void* commState, int *ranks, int rank, int nranks, void* allData, int size) {
  if (nranks == 1) return ncclSuccess;
/* 中文逐项-变量 V0067：data
 * data（char；函数 bootstrapIntraNodeAllGather 内）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 初值/来源 data = (char*)allData；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  char* data = (char*)allData;
  TRACE(NCCL_INIT, "rank %d nranks %d size %d - ENTER", rank, nranks, size);

  for (/* 中文逐项-变量 V0068：i
 * i（int；函数 bootstrapIntraNodeAllGather 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：i<nranks；循环体与增量表达式决定每轮如何推进。
 */int i=1; i<nranks; i++) {
/* 中文逐项-变量 V0069：src
 * src（int；函数 bootstrapIntraNodeAllGather 内）：输入数据地址，偏移单位由实际指针元素类型和当前 API 的归一化结果决定。
 * 初值/来源 src = (rank - i + nranks) % nranks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int src = (rank - i + nranks) % nranks;
/* 中文逐项-变量 V0070：dst
 * dst（int；函数 bootstrapIntraNodeAllGather 内）：输出数据地址，必须容纳当前 API 语义所需结果；AG 是多 rank 拼接，RS 只保留本段。
 * 初值/来源 dst = (rank + i) % nranks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int dst = (rank + i) % nranks;
    NCCLCHECK(bootstrapSend(commState, ranks[dst], /*tag=*/i, data+rank*size, size));
    NCCLCHECK(bootstrapRecv(commState, ranks[src], /*tag=*/i, data+src*size, size));
  }

  TRACE(NCCL_INIT, "rank %d nranks %d size %d - DONE", rank, nranks, size);
  return ncclSuccess;
}

/* 中文内部逻辑 F0043：unexpectedEnqueue
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct unexConn* unex;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&unex, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 3. 调用 / 计算 / 写入：unex->peer = peer;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：unex->tag = tag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：memcpy(&unex->sock, sock, sizeof(struct ncclSocket));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 准备局部状态：struct unexConn* list = state->unexpectedConnections;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 条件判断：(list == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 调用 / 计算 / 写入：state->unexpectedConnections = unex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 11. 循环：初始 ；继续条件 (list->next)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   12. 调用 / 计算 / 写入：list = list->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 14. 调用 / 计算 / 写入：list->next = unex;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0043
 */
/* 中文逐项-函数 F0043：unexpectedEnqueue
 * 用途与位置：保存暂时不匹配当前 peer/tag 的控制连接，避免先到消息被错误接收。
 * 参数 state（struct bootstrapState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRecv。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclCalloc。
 * 需要对照的边界/推进条件：list == NULL；list->next。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t unexpectedEnqueue(struct bootstrapState* state, int peer, int tag, struct ncclSocket* sock) {
  // New unex
/* 中文逐项-变量 V0071：unex
 * unex（struct unexConn；函数 unexpectedEnqueue 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&unex, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct unexConn* unex;
  NCCLCHECK(ncclCalloc(&unex, 1));
  unex->peer = peer;
  unex->tag = tag;
  memcpy(&unex->sock, sock, sizeof(struct ncclSocket));

  // Enqueue
/* 中文逐项-变量 V0072：list
 * list（struct unexConn；函数 unexpectedEnqueue 内）：节点或链路指针列表，count给出有效条目数，遍历不得越过当前有效范围。
 * 初值/来源 list = state->unexpectedConnections；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct unexConn* list = state->unexpectedConnections;
  if (list == NULL) {
    state->unexpectedConnections = unex;
    return ncclSuccess;
  }
  while (list->next) list = list->next;
  list->next = unex;
  return ncclSuccess;
}

/* 中文内部逻辑 F0044：unexpectedDequeue
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct unexConn* elem = state->unexpectedConnections;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct unexConn* prev = NULL;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 ；继续条件 (elem)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 条件判断：(elem->peer == peer && elem->tag == tag)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 条件判断：(prev == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         7. 真分支：；仅条件为真执行以下子项。
 *           8. 调用 / 计算 / 写入：state->unexpectedConnections = elem->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         9. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *           10. 调用 / 计算 / 写入：prev->next = elem->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       11. 调用 / 计算 / 写入：memcpy(sock, &elem->sock, sizeof(struct ncclSocket));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       12. 调用 / 计算 / 写入：free(elem);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   14. 调用 / 计算 / 写入：prev = elem;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   15. 调用 / 计算 / 写入：elem = elem->next;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 17. 调用 / 计算 / 写入：sock->fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0044
 */
/* 中文逐项-函数 F0044：unexpectedDequeue
 * 用途与位置：从暂存连接链中找出匹配 peer/tag 的项并摘除，供当前接收继续使用。
 * 参数 state（struct bootstrapState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRecv。
 * 本体直接出现的调用/包装：free；memcpy。
 * 需要对照的边界/推进条件：elem；elem->peer == peer && elem->tag == tag；prev == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t unexpectedDequeue(struct bootstrapState* state, int peer, int tag, struct ncclSocket* sock) {
/* 中文逐项-变量 V0073：elem
 * elem（struct unexConn；函数 unexpectedDequeue 内）：注册work扩展或队列容器内嵌的普通工作元素，扩展信息在它后面保存。
 * 初值/来源 elem = state->unexpectedConnections；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct unexConn* elem = state->unexpectedConnections;
/* 中文逐项-变量 V0074：prev
 * prev（struct unexConn；函数 unexpectedDequeue 内）：环前后继或链表前后项，准确含义由所属 ring/队列类型决定。
 * 初值/来源 prev = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  struct unexConn* prev = NULL;
  while (elem) {
    if (elem->peer == peer && elem->tag == tag) {
      if (prev == NULL) {
        state->unexpectedConnections = elem->next;
      } else {
        prev->next = elem->next;
      }
      memcpy(sock, &elem->sock, sizeof(struct ncclSocket));
      free(elem);
      return ncclSuccess;
    }
    prev = elem;
    elem = elem->next;
  }
  sock->fd = -1;
  return ncclSuccess;
}

// We can't know who we'll receive from, so we need to receive everything at once
/* 中文精读：控制信息为什么也需要 peer/tag 匹配
 * 监听 socket 到来的连接可能属于另一个 peer 或 tag，不能把先到的数据直接交给当前调用。
 * 匹配的消息立即读取，不匹配的先放 unexpected 队列，留给之后对应的接收。
 * 这是初始化控制消息的匹配机制；不要把此处 tag 当成公开 ncclSend/Recv 提供的用户 tag 参数。
 * 公开 P2P API 没有显式用户 tag，应用应保证同 peer 的收发次序和长度一致。
 */
/* 中文内部逻辑 F0045：bootstrapRecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct bootstrapState* state = (struct bootstrapState*)commState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclSocket sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：sock.abortFlag = state->abortFlag;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：NCCLCHECK(unexpectedDequeue(state, peer, tag, &sock));；失败时可能提前return，成功才继续；从暂存连接链中找出匹配 peer/tag 的项并摘除，供当前接收继续使用。
 * 5. 条件判断：(sock.fd != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetRecv(&sock, ((char*)data), size));；失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。
 *     8. 调用 / 计算 / 写入：close(sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 10. 循环：初始 ；继续条件 (1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   11. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketAccept(&sock, &state->listenSock));；失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。
 *   12. 准备局部状态：int newPeer, newTag;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   13. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetRecv(&sock, &newPeer, sizeof(int)));；失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。
 *   14. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetRecv(&sock, &newTag, sizeof(int)));；失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。
 *   15. 条件判断：(newPeer == peer && newTag == tag)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     16. 真分支：；仅条件为真执行以下子项。
 *       17. 调用 / 计算 / 写入：NCCLCHECK(bootstrapNetRecv(&sock, ((char*)data), size));；失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。
 *       18. 调用 / 计算 / 写入：close(sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       19. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   20. 调用 / 计算 / 写入：NCCLCHECK(unexpectedEnqueue(state, newPeer, newTag, &sock));；失败时可能提前return，成功才继续；保存暂时不匹配当前 peer/tag 的控制连接，避免先到消息被错误接收。
 * 21. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0045
 */
/* 中文逐项-函数 F0045：bootstrapRecv
 * 用途与位置：优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。
 * 参数 commState（void* commState）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 peer（int peer）：通信对端的用户 rank，决定在 channel.peers 中选择哪个连接槽。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapBarrier；src/bootstrap.cc::bootstrapIntraNodeAllGather；src/transport.cc::ncclTransportP2pSetup；src/transport.cc::ncclTransportCollNetSetup。
 * 本体直接出现的调用/包装：NCCLCHECK；bootstrapNetRecv；close；ncclSocketAccept；unexpectedDequeue；unexpectedEnqueue。
 * 需要对照的边界/推进条件：sock.fd != -1；1；newPeer == peer && newTag == tag。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapRecv(void* commState, int peer, int tag, void* data, int size) {
/* 中文逐项-变量 V0075：state
 * state（struct bootstrapState；函数 bootstrapRecv 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = (struct bootstrapState*)commState；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct bootstrapState* state = (struct bootstrapState*)commState;

/* 中文逐项-变量 V0076：sock
 * sock（struct ncclSocket；函数 bootstrapRecv 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 此处未显式给初值；后续首个文本使用点：sock.abortFlag = state->abortFlag;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclSocket sock;
  sock.abortFlag = state->abortFlag;

  // Search unexpected connections first
  NCCLCHECK(unexpectedDequeue(state, peer, tag, &sock));
  if (sock.fd != -1) {
    NCCLCHECK(bootstrapNetRecv(&sock, ((char*)data), size));
    close(sock.fd);
    return ncclSuccess;
  }

  // Then look for new connections
  while (1) {
    NCCLCHECK(ncclSocketAccept(&sock, &state->listenSock));
/* 中文逐项-变量 V0077：newPeer、newTag
 * newPeer（int；函数 bootstrapRecv 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(bootstrapNetRecv(&sock, &newPeer, sizeof(int)));。若由输出参数填入，必须先检查填充调用成功再读值。
 * newTag（int；函数 bootstrapRecv 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(bootstrapNetRecv(&sock, &newTag, sizeof(int)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int newPeer, newTag;
    NCCLCHECK(bootstrapNetRecv(&sock, &newPeer, sizeof(int)));
    NCCLCHECK(bootstrapNetRecv(&sock, &newTag, sizeof(int)));
    if (newPeer == peer && newTag == tag) {
      NCCLCHECK(bootstrapNetRecv(&sock, ((char*)data), size));
      close(sock.fd);
      return ncclSuccess;
    }
    // Unexpected connection. Save for later.
    NCCLCHECK(unexpectedEnqueue(state, newPeer, newTag, &sock));
  }
}

/* 中文内部逻辑 F0046：bootstrapClose
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct bootstrapState* state = (struct bootstrapState*)commState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(state->unexpectedConnections != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("Unexpected connections are not empty");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：close(state->listenSock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 7. 调用 / 计算 / 写入：close(state->ringSendSocket.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 调用 / 计算 / 写入：close(state->ringRecvSocket.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 调用 / 计算 / 写入：free(state->peerCommAddresses);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 调用 / 计算 / 写入：free(state);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0046
 */
/* 中文逐项-函数 F0046：bootstrapClose
 * 用途与位置：在正常生命周期结束时关闭 bootstrap socket 并回收地址表、暂存项和状态。
 * 参数 commState（void* commState）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：close；free。
 * 需要对照的边界/推进条件：state->unexpectedConnections != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapClose(void* commState) {
/* 中文逐项-变量 V0078：state
 * state（struct bootstrapState；函数 bootstrapClose 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = (struct bootstrapState*)commState；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct bootstrapState* state = (struct bootstrapState*)commState;
  if (state->unexpectedConnections != NULL) {
    WARN("Unexpected connections are not empty");
    return ncclInternalError;
  }
  close(state->listenSock.fd);
  close(state->ringSendSocket.fd);
  close(state->ringRecvSocket.fd);

  free(state->peerCommAddresses);
  free(state);

  return ncclSuccess;
}

/* 中文内部逻辑 F0047：bootstrapAbort
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct bootstrapState* state = (struct bootstrapState*)commState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(commState == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 条件判断：(state->listenSock.fd)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：close(state->listenSock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 条件判断：(state->ringSendSocket.fd)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   9. 真分支：；仅条件为真执行以下子项。
 *     10. 调用 / 计算 / 写入：close(state->ringSendSocket.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 11. 条件判断：(state->ringRecvSocket.fd)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 调用 / 计算 / 写入：close(state->ringRecvSocket.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 14. 调用 / 计算 / 写入：free(state->peerCommAddresses);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 15. 调用 / 计算 / 写入：free(state->peerProxyAddresses);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 16. 调用 / 计算 / 写入：free(state);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 17. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__bootstrap_cc.md#f0047
 */
/* 中文逐项-函数 F0047：bootstrapAbort
 * 用途与位置：异常退出时关闭控制网络并回收状态，配合中止流程打破初始化等待。
 * 参数 commState（void* commState）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/init.cc::ncclCommInitRankSync。
 * 本体直接出现的调用/包装：close；free。
 * 需要对照的边界/推进条件：commState ==
 * NULL；state->listenSock.fd；state->ringSendSocket.fd；state->ringRecvSocket.fd。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t bootstrapAbort(void* commState) {
/* 中文逐项-变量 V0079：state
 * state（struct bootstrapState；函数 bootstrapAbort 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 初值/来源 state = (struct bootstrapState*)commState；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct bootstrapState* state = (struct bootstrapState*)commState;
  if (commState == NULL) return ncclSuccess;
  if (state->listenSock.fd) close(state->listenSock.fd);
  if (state->ringSendSocket.fd) close(state->ringSendSocket.fd);
  if (state->ringRecvSocket.fd) close(state->ringRecvSocket.fd);
  free(state->peerCommAddresses);
  free(state->peerProxyAddresses);
  free(state);
  return ncclSuccess;
}
