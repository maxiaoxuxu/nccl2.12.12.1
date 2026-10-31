/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "comm.h"
#include "core.h"
#include "socket.h"
#include "net.h"
#include "param.h"

#include <pthread.h>
#include <stdlib.h>
#include <poll.h>
#include <limits.h>
#include <fcntl.h>

/* Init functions */
/* 中文逐项-变量 V4448：ncclNetIfs
 * ncclNetIfs（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclNetIfs = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
static int ncclNetIfs = -1;
/* 中文逐项-类型 T0442：ncclSocketDev
 * 类型职责：TCP后端接口属性，用于拓扑与网络设备枚举。
 * 声明内可见成员（含嵌套成员）：addr、devName、pciPath。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketDev {
/* 中文逐项-变量 V4449：addr
 * addr（union ncclSocketAddress；成员 ncclSocketDev）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
  union ncclSocketAddress addr;
/* 中文逐项-变量 V4450：devName
 * devName（char；成员 ncclSocketDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  char devName[MAX_IF_NAME_SIZE];
/* 中文逐项-变量 V4451：pciPath
 * pciPath（char；成员 ncclSocketDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  char* pciPath;
};
/* 中文逐项-变量 V4452：ncclSocketDevs
 * ncclSocketDevs（struct ncclSocketDev；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：strcpy(ncclSocketDevs[i].devName, names+i*MAX_IF_NAME_SIZE);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static struct ncclSocketDev ncclSocketDevs[MAX_IFS];

/* 中文逐项-变量 V4453：ncclSocketLock
 * ncclSocketLock（pthread_mutex_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclSocketLock = PTHREAD_MUTEX_INITIALIZER；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
pthread_mutex_t ncclSocketLock = PTHREAD_MUTEX_INITIALIZER;

/* 中文内部逻辑 F1130：ncclSocketGetPciPath
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char devicePath[PATH_MAX];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：snprintf(devicePath, PATH_MAX, "/sys/class/net/%s/device", devName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 3. 调用 / 计算 / 写入：*pciPath = realpath(devicePath, NULL);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1130
 */
/* 中文逐项-函数 F1130：ncclSocketGetPciPath
 * 用途与位置：读取网络接口的 PCI 路径，尽量把 TCP 后端设备纳入统一硬件拓扑。
 * 参数 devName（char* devName）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 参数 pciPath（char** pciPath）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_socket.cc::ncclSocketInit。
 * 本体直接出现的调用/包装：realpath；snprintf。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclSocketGetPciPath(char* devName, char** pciPath) {
/* 中文逐项-变量 V4454：devicePath
 * devicePath（char；函数 ncclSocketGetPciPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：snprintf(devicePath, PATH_MAX, "/sys/class/net/%s/device", devName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char devicePath[PATH_MAX];
  snprintf(devicePath, PATH_MAX, "/sys/class/net/%s/device", devName);
  // May return NULL if the file doesn't exist.
  *pciPath = realpath(devicePath, NULL);
  return ncclSuccess;
}

/* 中文内部逻辑 F1131：ncclSocketInit
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(ncclNetIfs == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：pthread_mutex_lock(&ncclSocketLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 条件判断：(ncclNetIfs == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       5. 真分支：；仅条件为真执行以下子项。
 *         6. 准备局部状态：char names[MAX_IF_NAME_SIZE*MAX_IFS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         7. 准备局部状态：union ncclSocketAddress addrs[MAX_IFS];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         8. 调用 / 计算 / 写入：ncclNetIfs = ncclFindInterfaces(names, addrs, MAX_IF_NAME_SIZE, MAX_IFS);；按显式接口配置或默认优先级选择可用接口，过滤不合适的 loopback/docker 等候选。
 *         9. 条件判断：(ncclNetIfs <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           10. 真分支：；仅条件为真执行以下子项。
 *             11. 调用 / 计算 / 写入：WARN("NET/Socket : no interface found");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *             12. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *           13. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *             14. 编译期分支/指令：#define MAX_LINE_LEN (2047)；由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。
 *             15. 准备局部状态：char line[MAX_LINE_LEN+1];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             16. 准备局部状态：char addrline[SOCKET_NAME_MAXLEN+1];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *             17. 调用 / 计算 / 写入：line[0] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             18. 调用 / 计算 / 写入：addrline[SOCKET_NAME_MAXLEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             19. 循环：初始 int i=0;；继续条件 i<ncclNetIfs；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               20. 调用 / 计算 / 写入：strcpy(ncclSocketDevs[i].devName, names+i*MAX_IF_NAME_SIZE);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *               21. 调用 / 计算 / 写入：memcpy(&ncclSocketDevs[i].addr, addrs+i, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *               22. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketGetPciPath(ncclSocketDevs[i].devName, &ncclSocketDevs[i].pciPath));；失败时可能提前return，成功才继续；读取网络接口的 PCI 路径，尽量把 TCP 后端设备纳入统一硬件拓扑。
 *               23. 调用 / 计算 / 写入：snprintf(line+strlen(line), MAX_LINE_LEN-strlen(line), " [%d]%s:%s", i, names+i*MAX_IF_NAME_SIZE, ncclSocketToString(&addrs[i], addrline));；把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。
 *             24. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *             25. 调用 / 计算 / 写入：line[MAX_LINE_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             26. 调用 / 计算 / 写入：INFO(NCCL_INIT|NCCL_NET,"NET/Socket : Using%s", line);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     27. 调用 / 计算 / 写入：pthread_mutex_unlock(&ncclSocketLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 28. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1131
 */
/* 中文逐项-函数 F1131：ncclSocketInit
 * 用途与位置：本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。
 * 参数 logFunction（ncclDebugLogger_t logFunction）：NCCL提供的日志回调，插件用它按统一级别/子系统报告信息与错误。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbConnect；src/transport/net_socket.cc::ncclSocketConnect；src/transport/net_socket.cc::ncclSocketAccept。
 * 本体直接出现的调用/包装：NCCLCHECK；memcpy；ncclFindInterfaces；ncclSocketGetPciPath；ncclSocketToString；pthread_mutex_lock；pthread_mutex_unlock；snprintf；strcpy；strlen。
 * 需要对照的边界/推进条件：ncclNetIfs == -1；ncclNetIfs == -1；ncclNetIfs <= 0；i<ncclNetIfs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclSocketInit(ncclDebugLogger_t logFunction) {
  if (ncclNetIfs == -1) {
    pthread_mutex_lock(&ncclSocketLock);
    if (ncclNetIfs == -1) {
/* 中文逐项-变量 V4455：names
 * names（char；函数 ncclSocketInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclNetIfs = ncclFindInterfaces(names, addrs, MAX_IF_NAME_SIZE,
 * MAX_IFS);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char names[MAX_IF_NAME_SIZE*MAX_IFS];
/* 中文逐项-变量 V4456：addrs
 * addrs（union ncclSocketAddress；函数 ncclSocketInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclNetIfs = ncclFindInterfaces(names, addrs, MAX_IF_NAME_SIZE,
 * MAX_IFS);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      union ncclSocketAddress addrs[MAX_IFS];
      ncclNetIfs = ncclFindInterfaces(names, addrs, MAX_IF_NAME_SIZE, MAX_IFS);
      if (ncclNetIfs <= 0) {
        WARN("NET/Socket : no interface found");
        return ncclInternalError;
      } else {
/* 中文逐项-常量 X1286：MAX_LINE_LEN
 * 编译期定义：MAX_LINE_LEN = (2047)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
        #define MAX_LINE_LEN (2047)
/* 中文逐项-变量 V4457：line
 * line（char；函数 ncclSocketInit 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：line[0] = '\0';。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        char line[MAX_LINE_LEN+1];
/* 中文逐项-变量 V4458：addrline
 * addrline（char；函数 ncclSocketInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：addrline[SOCKET_NAME_MAXLEN] = '\0';。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        char addrline[SOCKET_NAME_MAXLEN+1];
        line[0] = '\0';
        addrline[SOCKET_NAME_MAXLEN] = '\0';
        for (/* 中文逐项-变量 V4459：i
 * i（int；函数 ncclSocketInit 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<ncclNetIfs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<ncclNetIfs; i++) {
          strcpy(ncclSocketDevs[i].devName, names+i*MAX_IF_NAME_SIZE);
          memcpy(&ncclSocketDevs[i].addr, addrs+i, sizeof(union ncclSocketAddress));
          NCCLCHECK(ncclSocketGetPciPath(ncclSocketDevs[i].devName, &ncclSocketDevs[i].pciPath));
          snprintf(line+strlen(line), MAX_LINE_LEN-strlen(line), " [%d]%s:%s", i, names+i*MAX_IF_NAME_SIZE,
              ncclSocketToString(&addrs[i], addrline));
        }
        line[MAX_LINE_LEN] = '\0';
        INFO(NCCL_INIT|NCCL_NET,"NET/Socket : Using%s", line);
      }
    }
    pthread_mutex_unlock(&ncclSocketLock);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1132：ncclSocketDevices
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*ndev = ncclNetIfs;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1132
 */
/* 中文逐项-函数 F1132：ncclSocketDevices
 * 用途与位置：报告 Socket 后端发现并筛选后的网络接口数量。
 * 参数 ndev（int* ndev）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketDevices(int* ndev) {
  *ndev = ncclNetIfs;
  return ncclSuccess;
}

/* 中文内部逻辑 F1133：ncclSocketGetSpeed
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*speed = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 准备局部状态：char speedPath[PATH_MAX];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：sprintf(speedPath, "/sys/class/net/%s/speed", devName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 准备局部状态：int fd = open(speedPath, O_RDONLY);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 条件判断：(fd != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 准备局部状态：char speedStr[] = " ";；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     8. 条件判断：(read(fd, speedStr, sizeof(speedStr)-1) > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       9. 真分支：；仅条件为真执行以下子项。
 *         10. 调用 / 计算 / 写入：*speed = strtol(speedStr, NULL, 0);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 调用 / 计算 / 写入：close(fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 12. 条件判断：(*speed <= 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   13. 真分支：；仅条件为真执行以下子项。
 *     14. 调用 / 计算 / 写入：INFO(NCCL_NET, "Could not get speed from %s. Defaulting to 10 Gbps.", speedPath);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     15. 调用 / 计算 / 写入：*speed = 10000;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1133
 */
/* 中文逐项-函数 F1133：ncclSocketGetSpeed
 * 用途与位置：从系统接口属性读取链路速率，读取失败时按实现提供保守估计。
 * 参数 devName（char* devName）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 参数 speed（int* speed）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_socket.cc::ncclSocketGetProperties。
 * 本体直接出现的调用/包装：close；open；read；sprintf；strtol。
 * 需要对照的边界/推进条件：fd != -1；read(fd, speedStr, sizeof(speedStr)-1) > 0；*speed <= 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclSocketGetSpeed(char* devName, int* speed) {
  *speed = 0;
/* 中文逐项-变量 V4460：speedPath
 * speedPath（char；函数 ncclSocketGetSpeed 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：sprintf(speedPath, "/sys/class/net/%s/speed", devName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char speedPath[PATH_MAX];
  sprintf(speedPath, "/sys/class/net/%s/speed", devName);
/* 中文逐项-变量 V4461：fd
 * fd（int；函数 ncclSocketGetSpeed 内）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 初值/来源 fd = open(speedPath, O_RDONLY)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int fd = open(speedPath, O_RDONLY);
  if (fd != -1) {
/* 中文逐项-变量 V4462：speedStr
 * speedStr（char；函数 ncclSocketGetSpeed 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 speedStr = " "；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char speedStr[] = "        ";
    if (read(fd, speedStr, sizeof(speedStr)-1) > 0) {
      *speed = strtol(speedStr, NULL, 0);
    }
    close(fd);
  }
  if (*speed <= 0) {
    INFO(NCCL_NET, "Could not get speed from %s. Defaulting to 10 Gbps.", speedPath);
    *speed = 10000;
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1134：ncclSocketGetProperties
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：props->name = ncclSocketDevs[dev].devName;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 调用 / 计算 / 写入：props->pciPath = ncclSocketDevs[dev].pciPath;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 调用 / 计算 / 写入：props->guid = dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 调用 / 计算 / 写入：props->ptrSupport = NCCL_PTR_HOST;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketGetSpeed(props->name, &props->speed));；失败时可能提前return，成功才继续；从系统接口属性读取链路速率，读取失败时按实现提供保守估计。
 * 6. 调用 / 计算 / 写入：props->latency = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：props->port = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：props->maxComms = 65536;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：props->maxRecvs = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1134
 */
/* 中文逐项-函数 F1134：ncclSocketGetProperties
 * 用途与位置：向统一网络层报告 TCP 接口名字/PCI/速度及只接受主机指针等能力。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 props（ncclNetProperties_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketGetSpeed。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketGetProperties(int dev, ncclNetProperties_t* props) {
  props->name = ncclSocketDevs[dev].devName;
  props->pciPath = ncclSocketDevs[dev].pciPath;
  props->guid = dev;
  props->ptrSupport = NCCL_PTR_HOST;
  NCCLCHECK(ncclSocketGetSpeed(props->name, &props->speed));
  props->latency = 0; // Not set
  props->port = 0;
  props->maxComms = 65536;
  props->maxRecvs = 1;
  return ncclSuccess;
}

/* 中文内部逻辑 F1135：GetSocketAddr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(dev >= ncclNetIfs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 调用 / 计算 / 写入：memcpy(addr, &ncclSocketDevs[dev].addr, sizeof(*addr));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1135
 */
/* 中文逐项-函数 F1135：GetSocketAddr
 * 用途与位置：为所选后端设备获得建链控制 socket 地址；控制握手地址与实际数据载荷通路不同。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 addr（union ncclSocketAddress* addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbListen；src/transport/net_socket.cc::ncclSocketListen。
 * 本体直接出现的调用/包装：memcpy。
 * 需要对照的边界/推进条件：dev >= ncclNetIfs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t GetSocketAddr(int dev, union ncclSocketAddress* addr) {
  if (dev >= ncclNetIfs) return ncclInternalError;
  memcpy(addr, &ncclSocketDevs[dev].addr, sizeof(*addr));
  return ncclSuccess;
}

/* Communication functions */

/* 中文逐项-常量 X1287：MAX_SOCKETS
 * 编译期定义：MAX_SOCKETS = 64。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_SOCKETS 64
/* 中文逐项-常量 X1288：MAX_THREADS
 * 编译期定义：MAX_THREADS = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_THREADS 16
/* 中文逐项-常量 X1289：MAX_REQUESTS
 * 编译期定义：MAX_REQUESTS = NCCL_NET_MAX_REQUESTS。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_REQUESTS NCCL_NET_MAX_REQUESTS
/* 中文逐项-常量 X1290：MIN_CHUNKSIZE
 * 编译期定义：MIN_CHUNKSIZE = (64*1024)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MIN_CHUNKSIZE (64*1024)

/* 中文逐项-参数getter P0052：SocketNsocksPerThread
 * 生成函数 ncclParamSocketNsocksPerThread()，读取 NCCL_NSOCKS_PERTHREAD，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(SocketNsocksPerThread, "NSOCKS_PERTHREAD", -2);
/* 中文逐项-参数getter P0053：SocketNthreads
 * 生成函数 ncclParamSocketNthreads()，读取 NCCL_SOCKET_NTHREADS，编译默认值为 -2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(SocketNthreads, "SOCKET_NTHREADS", -2);

/* 中文逐项-类型 T0443：ncclSocketCommState
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclSocketCommState {
/* 中文逐项-枚举项 X1291：ncclSocketCommStateStart
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
  ncclSocketCommStateStart = 0,
/* 中文逐项-枚举项 X1292：ncclSocketCommStateConnect
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
  ncclSocketCommStateConnect = 1,
/* 中文逐项-枚举项 X1293：ncclSocketCommStateAccept
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
  ncclSocketCommStateAccept = 3,
/* 中文逐项-枚举项 X1294：ncclSocketCommStateSend
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
  ncclSocketCommStateSend = 4,
/* 中文逐项-枚举项 X1295：ncclSocketCommStateRecv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
  ncclSocketCommStateRecv = 5,
};

/* 中文逐项-类型 T0444：ncclSocketCommStage
 * 类型职责：TCP多socket建链的可持续阶段状态及可交换监听地址信息。
 * 声明内可见成员（含嵌套成员）：state、iteration、sock、comm。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketCommStage {
/* 中文逐项-变量 V4463：state
 * state（enum ncclSocketCommState；成员 ncclSocketCommStage）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 */
  enum ncclSocketCommState state;
/* 中文逐项-变量 V4464：iteration
 * iteration（uint8_t；成员 ncclSocketCommStage）：请求或握手重复推进次数/序号，用于保持同一状态机阶段的组织。
 */
  uint8_t iteration;
/* 中文逐项-变量 V4465：sock
 * sock（struct ncclSocket；成员 ncclSocketCommStage）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket* sock;
/* 中文逐项-变量 V4466：comm
 * comm（struct ncclSocketComm；成员 ncclSocketCommStage）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  struct ncclSocketComm* comm;
};

/* 中文逐项-类型 T0445：ncclSocketHandle
 * 类型职责：TCP多socket建链的可持续阶段状态及可交换监听地址信息。
 * 声明内可见成员（含嵌套成员）：connectAddr、nSocks、nThreads、stage。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketHandle {
/* 中文逐项-变量 V4467：connectAddr
 * connectAddr（union ncclSocketAddress；成员 ncclSocketHandle）：非阻塞网络建链目标地址，重试仍沿用同一连接状态。
 */
  union ncclSocketAddress connectAddr;
/* 中文逐项-变量 V4468：nSocks
 * nSocks（int；成员 ncclSocketHandle）：TCP后端并行数据socket数组及数量，按线程分片推进消息。
 */
  int nSocks;
/* 中文逐项-变量 V4469：nThreads
 * nThreads（int；成员 ncclSocketHandle）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 */
  int nThreads;
/* 中文逐项-变量 V4470：stage
 * stage（struct ncclSocketCommStage；成员 ncclSocketHandle）：对象状态机当前阶段或保留的未完成握手状态，重复调用时从这里继续。
 */
  struct ncclSocketCommStage stage;
};

/* 中文逐项-类型 T0446：ncclSocketTask
 * 类型职责：TCP子任务和上层异步请求，前者按offset推进，后者等全部子任务完成。
 * 声明内可见成员（含嵌套成员）：op、data、size、sock、offset、used、result。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketTask {
/* 中文逐项-变量 V4471：op
 * op（int；成员 ncclSocketTask）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 */
  int op;
/* 中文逐项-变量 V4472：data
 * data（void；成员 ncclSocketTask）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 */
  void* data;
/* 中文逐项-变量 V4473：size
 * size（int；成员 ncclSocketTask）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  int size;
/* 中文逐项-变量 V4474：sock
 * sock（struct ncclSocket；成员 ncclSocketTask）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket* sock;
/* 中文逐项-变量 V4475：offset
 * offset（int；成员 ncclSocketTask）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 */
  int offset;
/* 中文逐项-变量 V4476：used
 * used（int；成员 ncclSocketTask）：搜索中已使用GPU/节点的位图或状态，回溯后须恢复以尝试其他候选。
 */
  int used;
/* 中文逐项-变量 V4477：result
 * result（ncclResult_t；成员 ncclSocketTask）：异步线程/请求保存的结果码，前台测试函数需要检查它而不只检查长度。
 */
  ncclResult_t result;
};

/* 中文逐项-类型 T0447：ncclSocketRequest
 * 类型职责：TCP子任务和上层异步请求，前者按offset推进，后者等全部子任务完成。
 * 声明内可见成员（含嵌套成员）：op、data、size、ctrlSock、offset、used、comm、tasks、nSubs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketRequest {
/* 中文逐项-变量 V4478：op
 * op（int；成员 ncclSocketRequest）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 */
  int op;
/* 中文逐项-变量 V4479：data
 * data（void；成员 ncclSocketRequest）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 */
  void* data;
/* 中文逐项-变量 V4480：size
 * size（int；成员 ncclSocketRequest）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  int size;
/* 中文逐项-变量 V4481：ctrlSock
 * ctrlSock（struct ncclSocket；成员 ncclSocketRequest）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket* ctrlSock;
/* 中文逐项-变量 V4482：offset
 * offset（int；成员 ncclSocketRequest）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 */
  int offset;
/* 中文逐项-变量 V4483：used
 * used（int；成员 ncclSocketRequest）：搜索中已使用GPU/节点的位图或状态，回溯后须恢复以尝试其他候选。
 */
  int used;
/* 中文逐项-变量 V4484：comm
 * comm（struct ncclSocketComm；成员 ncclSocketRequest）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  struct ncclSocketComm* comm;
/* 中文逐项-变量 V4485：tasks
 * tasks（struct ncclSocketTask；成员 ncclSocketRequest）：TCP后台线程待推进任务或队列，任务offset决定部分完成范围。
 */
  struct ncclSocketTask* tasks[MAX_SOCKETS];
/* 中文逐项-变量 V4486：nSubs
 * nSubs（int；成员 ncclSocketRequest）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 */
  int nSubs;
};

/* 中文逐项-类型 T0448：ncclSocketTaskQueue
 * 类型职责：TCP后台线程任务队列及执行上下文，多个消息复用同一线程。
 * 声明内可见成员（含嵌套成员）：next、len、tasks。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketTaskQueue {
/* 中文逐项-变量 V4487：next
 * next（int；成员 ncclSocketTaskQueue）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
  int next;
/* 中文逐项-变量 V4488：len
 * len（int；成员 ncclSocketTaskQueue）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  int len;
/* 中文逐项-变量 V4489：tasks
 * tasks（struct ncclSocketTask；成员 ncclSocketTaskQueue）：TCP后台线程待推进任务或队列，任务offset决定部分完成范围。
 */
  struct ncclSocketTask* tasks;
};

/* 中文逐项-类型 T0449：ncclSocketThreadResources
 * 类型职责：TCP后台线程任务队列及执行上下文，多个消息复用同一线程。
 * 声明内可见成员（含嵌套成员）：threadTaskQueue、stop、comm、threadLock、threadCond。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketThreadResources {
/* 中文逐项-变量 V4490：threadTaskQueue
 * threadTaskQueue（struct ncclSocketTaskQueue；成员 ncclSocketThreadResources）：TCP后台线程待推进任务或队列，任务offset决定部分完成范围。
 */
  struct ncclSocketTaskQueue threadTaskQueue;
/* 中文逐项-变量 V4491：stop
 * stop（int；成员 ncclSocketThreadResources）：辅助/代理线程生命周期状态，停止时仍须唤醒并 join 才能回收其资源。
 */
  int stop;
/* 中文逐项-变量 V4492：comm
 * comm（struct ncclSocketComm；成员 ncclSocketThreadResources）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  struct ncclSocketComm* comm;
/* 中文逐项-变量 V4493：threadLock
 * threadLock（pthread_mutex_t；成员 ncclSocketThreadResources）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
  pthread_mutex_t threadLock;
/* 中文逐项-变量 V4494：threadCond
 * threadCond（pthread_cond_t；成员 ncclSocketThreadResources）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
  pthread_cond_t  threadCond;
};

/* 中文逐项-类型 T0450：ncclSocketListenComm
 * 类型职责：TCP后端监听对象及已建立的数据/控制socket、请求池和线程资源集合。
 * 声明内可见成员（含嵌套成员）：sock、stage、nSocks、nThreads、dev。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketListenComm {
/* 中文逐项-变量 V4495：sock
 * sock（struct ncclSocket；成员 ncclSocketListenComm）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket sock;
/* 中文逐项-变量 V4496：stage
 * stage（struct ncclSocketCommStage；成员 ncclSocketListenComm）：对象状态机当前阶段或保留的未完成握手状态，重复调用时从这里继续。
 */
  struct ncclSocketCommStage stage;
/* 中文逐项-变量 V4497：nSocks
 * nSocks（int；成员 ncclSocketListenComm）：TCP后端并行数据socket数组及数量，按线程分片推进消息。
 */
  int nSocks;
/* 中文逐项-变量 V4498：nThreads
 * nThreads（int；成员 ncclSocketListenComm）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 */
  int nThreads;
/* 中文逐项-变量 V4499：dev
 * dev（int；成员 ncclSocketListenComm）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 */
  int dev;
};

/* 中文逐项-类型 T0451：ncclSocketComm
 * 类型职责：TCP后端监听对象及已建立的数据/控制socket、请求池和线程资源集合。
 * 声明内可见成员（含嵌套成员）：ctrlSock、socks、dev、cudaDev、nSocks、nThreads、nextSock、requests、helperThread、threadResources。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclSocketComm {
/* 中文逐项-变量 V4500：ctrlSock
 * ctrlSock（struct ncclSocket；成员 ncclSocketComm）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket ctrlSock;
/* 中文逐项-变量 V4501：socks
 * socks（struct ncclSocket；成员 ncclSocketComm）：TCP后端并行数据socket数组及数量，按线程分片推进消息。
 */
  struct ncclSocket socks[MAX_SOCKETS];
/* 中文逐项-变量 V4502：dev
 * dev（int；成员 ncclSocketComm）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 */
  int dev;
/* 中文逐项-变量 V4503：cudaDev
 * cudaDev（int；成员 ncclSocketComm）：本进程 CUDA 可见设备序号，供切换设备上下文使用。
 */
  int cudaDev;
/* 中文逐项-变量 V4504：nSocks
 * nSocks（int；成员 ncclSocketComm）：TCP后端并行数据socket数组及数量，按线程分片推进消息。
 */
  int nSocks;
/* 中文逐项-变量 V4505：nThreads
 * nThreads（int；成员 ncclSocketComm）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 */
  int nThreads;
/* 中文逐项-变量 V4506：nextSock
 * nextSock（int；成员 ncclSocketComm）：TCP任务轮转分派的下一个数据socket索引，用于分散并行负载。
 */
  int nextSock;
/* 中文逐项-变量 V4507：requests
 * requests（struct ncclSocketRequest；成员 ncclSocketComm）：在途网络请求或固定请求池；只有 Test 完成后其槽才能重用。
 */
  struct ncclSocketRequest requests[MAX_REQUESTS];
/* 中文逐项-变量 V4508：helperThread
 * helperThread（pthread_t；成员 ncclSocketComm）：后台线程句柄和其任务/停止状态，资源回收前需要停止并join。
 */
  pthread_t helperThread[MAX_THREADS];
/* 中文逐项-变量 V4509：threadResources
 * threadResources（struct ncclSocketThreadResources；成员 ncclSocketComm）：TCP后台工作线程持有的socket与任务状态，生命周期覆盖多次数据请求。
 */
  struct ncclSocketThreadResources threadResources[MAX_THREADS];
};

/* 中文内部逻辑 F1136：persistentSocketThread
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocketThreadResources* resource = (struct ncclSocketThreadResources*)args_;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclSocketComm* comm = resource->comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct ncclSocketTaskQueue* myQueue = &resource->threadTaskQueue;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：int nSocksPerThread = comm->nSocks / comm->nThreads;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 循环：初始 ；继续条件 (1)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   6. 准备局部状态：int idle = 1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   7. 准备局部状态：int mark = myQueue->next;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   8. 循环：初始 int i=0;；继续条件 i<myQueue->len；每轮更新 i+=nSocksPerThread；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *     9. 准备局部状态：int repeat;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     10. 先执行一次的循环：初始 ；继续条件 (repeat)；每轮更新；do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。
 *       11. 调用 / 计算 / 写入：repeat = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       12. 循环：初始 int j=0;；继续条件 j<nSocksPerThread；每轮更新 j++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         13. 准备局部状态：struct ncclSocketTask* r = myQueue->tasks+i+j;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         14. 条件判断：(r != NULL && r->used == 1 && r->offset < r->size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           15. 真分支：；仅条件为真执行以下子项。
 *             16. 调用 / 计算 / 写入：r->result = ncclSocketProgress(r->op, r->sock, r->data, r->size, &r->offset);；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。
 *             17. 条件判断：(r->result != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               18. 真分支：；仅条件为真执行以下子项。
 *                 19. 调用 / 计算 / 写入：WARN("NET/Socket : socket progress error");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *                 20. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *             21. 调用 / 计算 / 写入：idle = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             22. 条件判断：(r->offset < r->size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               23. 真分支：；仅条件为真执行以下子项。
 *                 24. 调用 / 计算 / 写入：repeat = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       25. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     26. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   27. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *   28. 条件判断：(idle)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     29. 真分支：；仅条件为真执行以下子项。
 *       30. 调用 / 计算 / 写入：pthread_mutex_lock(&resource->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       31. 循环：初始 ；继续条件 (mark == myQueue->next && resource->stop == 0)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *         32. 调用 / 计算 / 写入：pthread_cond_wait(&resource->threadCond, &resource->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       33. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       34. 调用 / 计算 / 写入：pthread_mutex_unlock(&resource->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   35. 条件判断：(resource->stop)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     36. 真分支：；仅条件为真执行以下子项。
 *       37. 函数返回：return NULL;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 38. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1136
 */
/* 中文逐项-函数 F1136：persistentSocketThread
 * 用途与位置：后台交错推进分配到本线程的 TCP 子任务，更新每个任务的传输 offset 直到完成或停止。
 * 参数 args_（void *args_）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclSocketProgress；pthread_cond_wait；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：1；i<myQueue->len；j<nSocksPerThread；r != NULL && r->used == 1 && r->offset < r->size。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
void* persistentSocketThread(void *args_) {
/* 中文逐项-变量 V4510：resource
 * resource（struct ncclSocketThreadResources；函数 persistentSocketThread 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 resource = (struct ncclSocketThreadResources*)args_；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketThreadResources* resource = (struct ncclSocketThreadResources*)args_;
/* 中文逐项-变量 V4511：comm
 * comm（struct ncclSocketComm；函数 persistentSocketThread 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = resource->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocketComm* comm = resource->comm;
/* 中文逐项-变量 V4512：myQueue
 * myQueue（struct ncclSocketTaskQueue；函数 persistentSocketThread 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 myQueue = &resource->threadTaskQueue；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclSocketTaskQueue* myQueue = &resource->threadTaskQueue;
/* 中文逐项-变量 V4513：nSocksPerThread
 * nSocksPerThread（int；函数 persistentSocketThread 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nSocksPerThread = comm->nSocks / comm->nThreads；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int nSocksPerThread = comm->nSocks / comm->nThreads;
  while (1) {
/* 中文逐项-变量 V4514：idle
 * idle（int；函数 persistentSocketThread 内）：本轮推进是否未取得进展，用于 CPU 让出策略，不代表全体通信已经完成。
 * 初值/来源 idle = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int idle = 1;
/* 中文逐项-变量 V4515：mark
 * mark（int；函数 persistentSocketThread 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 mark = myQueue->next；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int mark = myQueue->next; // mark newest task seen
    for (/* 中文逐项-变量 V4516：i
 * i（int；函数 persistentSocketThread 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<myQueue->len；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<myQueue->len; i+=nSocksPerThread) {
/* 中文逐项-变量 V4517：repeat
 * repeat（int；函数 persistentSocketThread 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：repeat = 0;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int repeat;
      do {
        repeat = 0;
        for (/* 中文逐项-变量 V4518：j
 * j（int；函数 persistentSocketThread 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：j<nSocksPerThread；循环体与增量表达式决定每轮如何推进。
 */int j=0; j<nSocksPerThread; j++) {
/* 中文逐项-变量 V4519：r
 * r（struct ncclSocketTask；函数 persistentSocketThread 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 r = myQueue->tasks+i+j；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          struct ncclSocketTask* r = myQueue->tasks+i+j;
          if (r != NULL && r->used == 1 && r->offset < r->size) {
            r->result = ncclSocketProgress(r->op, r->sock, r->data, r->size, &r->offset);
            if (r->result != ncclSuccess) {
              WARN("NET/Socket : socket progress error");
              return NULL;
            }
            idle = 0;
            if (r->offset < r->size) repeat = 1;
          }
        }
      } while (repeat);
    }
    if (idle) {
      pthread_mutex_lock(&resource->threadLock);
      while (mark == myQueue->next && resource->stop == 0) { // no new tasks, wait
        pthread_cond_wait(&resource->threadCond, &resource->threadLock);
      }
      pthread_mutex_unlock(&resource->threadLock);
    }
    if (resource->stop) return NULL;
  }
}

/* 中文内部逻辑 F1137：ncclSocketGetNsockNthread
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int nSocksPerThread = ncclParamSocketNsocksPerThread();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：int nThreads = ncclParamSocketNthreads();；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(nThreads > MAX_THREADS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：WARN("NET/Socket : NCCL_SOCKET_NTHREADS is greater than the maximum allowed, setting to %d", MAX_THREADS);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 调用 / 计算 / 写入：nThreads = MAX_THREADS;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 条件判断：(nThreads == -2 || nSocksPerThread == -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 准备局部状态：int autoNt=0, autoNs=1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     10. 准备局部状态：char vendorPath[PATH_MAX];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     11. 调用 / 计算 / 写入：snprintf(vendorPath, PATH_MAX, "/sys/class/net/%s/device/vendor", ncclSocketDevs[dev].devName);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     12. 准备局部状态：char* rPath = realpath(vendorPath, NULL);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     13. 准备局部状态：int fd = open(rPath, O_RDONLY);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     14. 调用 / 计算 / 写入：free(rPath);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     15. 条件判断：(fd == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       16. 真分支：；仅条件为真执行以下子项。
 *         17. 调用 / 计算 / 写入：TRACE(NCCL_NET, "Open of %s failed : %s", vendorPath, strerror(errno));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         18. 控制转移：goto end;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 *     19. 准备局部状态：char vendor[7];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     20. 调用 / 计算 / 写入：strncpy(vendor, "0x0000", 7);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     21. 准备局部状态：int len;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     22. 调用 / 计算 / 写入：SYSCHECKVAL(read(fd, vendor, 6), "read", len);；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     23. 调用 / 计算 / 写入：SYSCHECK(close(fd), "close");；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     24. 条件判断：(strcmp(vendor, "0x1d0f") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       25. 真分支：；仅条件为真执行以下子项。
 *         26. 调用 / 计算 / 写入：autoNt = 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         27. 调用 / 计算 / 写入：autoNs = 8;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       28. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         29. 条件判断：(strcmp(vendor, "0x1ae0") == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           30. 真分支：；仅条件为真执行以下子项。
 *             31. 调用 / 计算 / 写入：autoNt = 4;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             32. 调用 / 计算 / 写入：autoNs = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     33. 跳转标签：end；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *       34. 条件判断：(nThreads == -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         35. 真分支：；仅条件为真执行以下子项。
 *           36. 调用 / 计算 / 写入：nThreads = autoNt;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     37. 条件判断：(nSocksPerThread == -2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       38. 真分支：；仅条件为真执行以下子项。
 *         39. 调用 / 计算 / 写入：nSocksPerThread = autoNs;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 40. 准备局部状态：int nSocks = nSocksPerThread * nThreads;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 41. 条件判断：(nSocks > MAX_SOCKETS)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   42. 真分支：；仅条件为真执行以下子项。
 *     43. 调用 / 计算 / 写入：nSocksPerThread = MAX_SOCKETS/nThreads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     44. 调用 / 计算 / 写入：WARN("NET/Socket : the total number of sockets is greater than the maximum allowed, setting NCCL_NSOCKS_PERTHREAD to %d", nSocksPerThread);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     45. 调用 / 计算 / 写入：nSocks = nSocksPerThread * nThreads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 46. 调用 / 计算 / 写入：*ns = nSocks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 47. 调用 / 计算 / 写入：*nt = nThreads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 48. 条件判断：(nSocks > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   49. 真分支：；仅条件为真执行以下子项。
 *     50. 调用 / 计算 / 写入：INFO(NCCL_INIT, "NET/Socket: Using %d threads and %d sockets per thread", nThreads, nSocksPerThread);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 51. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1137
 */
/* 中文逐项-函数 F1137：ncclSocketGetNsockNthread
 * 用途与位置：结合参数和平台调优选择每连接 socket 数与后台线程数，平衡并发带宽和 CPU 开销。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 ns（int* ns）：非对称 Fan 的运行时有效接收/发送邻居数。
 * 参数 nt（int* nt）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_socket.cc::ncclSocketListen。
 * 本体直接出现的调用/包装：SYSCHECK；SYSCHECKVAL；close；free；ncclParamSocketNsocksPerThread；ncclParamSocketNthreads；open；read；realpath；snprintf；strcmp；strerror；strncpy。
 * 需要对照的边界/推进条件：nThreads > MAX_THREADS；nThreads == -2 || nSocksPerThread == -2；fd == -1；strcmp(vendor, "0x1d0f") ==
 * 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketGetNsockNthread(int dev, int* ns, int* nt) {
/* 中文逐项-变量 V4520：nSocksPerThread
 * nSocksPerThread（int；函数 ncclSocketGetNsockNthread 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nSocksPerThread = ncclParamSocketNsocksPerThread()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
  int nSocksPerThread = ncclParamSocketNsocksPerThread();
/* 中文逐项-变量 V4521：nThreads
 * nThreads（int；函数 ncclSocketGetNsockNthread 内）：当前原语/任务参与线程数；原语子组可能只占整个 block 的一部分。
 * 初值/来源 nThreads = ncclParamSocketNthreads()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
  int nThreads = ncclParamSocketNthreads();
  if (nThreads > MAX_THREADS) {
    WARN("NET/Socket : NCCL_SOCKET_NTHREADS is greater than the maximum allowed, setting to %d", MAX_THREADS);
    nThreads = MAX_THREADS;
  }
  if (nThreads == -2 || nSocksPerThread == -2) {
    // Auto-detection
/* 中文逐项-变量 V4522：autoNt、autoNs
 * autoNt（int；函数 ncclSocketGetNsockNthread 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 autoNt = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * autoNs（int；函数 ncclSocketGetNsockNthread 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 autoNs = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int autoNt=0, autoNs=1; // By default, we only use the main thread and do not spawn extra threads
/* 中文逐项-变量 V4523：vendorPath
 * vendorPath（char；函数 ncclSocketGetNsockNthread 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：snprintf(vendorPath, PATH_MAX, "/sys/class/net/%s/device/vendor",
 * ncclSocketDevs[dev].devName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char vendorPath[PATH_MAX];
    snprintf(vendorPath, PATH_MAX, "/sys/class/net/%s/device/vendor", ncclSocketDevs[dev].devName);
/* 中文逐项-变量 V4524：rPath
 * rPath（char；函数 ncclSocketGetNsockNthread 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rPath = realpath(vendorPath, NULL)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    char* rPath = realpath(vendorPath, NULL);
/* 中文逐项-变量 V4525：fd
 * fd（int；函数 ncclSocketGetNsockNthread 内）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 初值/来源 fd = open(rPath, O_RDONLY)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int fd = open(rPath, O_RDONLY);
    free(rPath);
    if (fd == -1) {
      // Could not find device vendor. This is handled silently so
      // we don't want to print an INFO error.
      TRACE(NCCL_NET, "Open of %s failed : %s", vendorPath, strerror(errno));
      goto end;
    }
/* 中文逐项-变量 V4526：vendor
 * vendor（char；函数 ncclSocketGetNsockNthread 内）：CPU架构/厂商/型号，决定互连带宽模型与特定策略。
 * 此处未显式给初值；后续首个文本使用点：strncpy(vendor, "0x0000", 7);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char vendor[7];
    strncpy(vendor, "0x0000", 7);
/* 中文逐项-变量 V4527：len
 * len（int；函数 ncclSocketGetNsockNthread 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 此处未显式给初值；后续首个文本使用点：SYSCHECKVAL(read(fd, vendor, 6), "read", len);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int len;
    SYSCHECKVAL(read(fd, vendor, 6), "read", len);
    SYSCHECK(close(fd), "close");
    if (strcmp(vendor, "0x1d0f") == 0) { // AWS
      autoNt = 2;
      autoNs = 8;
    } else if (strcmp(vendor, "0x1ae0") == 0) { // GCP
      autoNt = 4;
      autoNs = 1;
    }
end:
    if (nThreads == -2) nThreads = autoNt;
    if (nSocksPerThread == -2) nSocksPerThread = autoNs;
  }
/* 中文逐项-变量 V4528：nSocks
 * nSocks（int；函数 ncclSocketGetNsockNthread 内）：TCP后端并行数据socket数组及数量，按线程分片推进消息。
 * 初值/来源 nSocks = nSocksPerThread * nThreads；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int nSocks = nSocksPerThread * nThreads;
  if (nSocks > MAX_SOCKETS) {
    nSocksPerThread = MAX_SOCKETS/nThreads;
    WARN("NET/Socket : the total number of sockets is greater than the maximum allowed, setting NCCL_NSOCKS_PERTHREAD to %d", nSocksPerThread);
    nSocks = nSocksPerThread * nThreads;
  }
  *ns = nSocks;
  *nt = nThreads;
  if (nSocks > 0) INFO(NCCL_INIT, "NET/Socket: Using %d threads and %d sockets per thread", nThreads, nSocksPerThread);
  return ncclSuccess;
}

/* 中文内部逻辑 F1138：ncclSocketNewListenComm
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(comm, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 2. 调用 / 计算 / 写入：(*comm)->sock.fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1138
 */
/* 中文逐项-函数 F1138：ncclSocketNewListenComm
 * 用途与位置：分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。
 * 参数 comm（struct ncclSocketListenComm** comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_socket.cc::ncclSocketListen。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketNewListenComm(struct ncclSocketListenComm** comm) {
  NCCLCHECK(ncclCalloc(comm, 1));
  (*comm)->sock.fd = -1;
  return ncclSuccess;
}

/* 中文内部逻辑 F1139：ncclSocketNewComm
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(comm, 1));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 2. 调用 / 计算 / 写入：(*comm)->ctrlSock.fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 循环：初始 int i=0;；继续条件 i < MAX_SOCKETS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 调用 / 计算 / 写入：(*comm)->socks[i].fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 5. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 6. 调用 / 计算 / 写入：(*comm)->nextSock = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1139
 */
/* 中文逐项-函数 F1139：ncclSocketNewComm
 * 用途与位置：分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。
 * 参数 comm（struct ncclSocketComm** comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_socket.cc::ncclSocketConnect；src/transport/net_socket.cc::ncclSocketAccept。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclCalloc。
 * 需要对照的边界/推进条件：i < MAX_SOCKETS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketNewComm(struct ncclSocketComm** comm) {
  NCCLCHECK(ncclCalloc(comm, 1));
  (*comm)->ctrlSock.fd = -1;
  for (/* 中文逐项-变量 V4529：i
 * i（int；函数 ncclSocketNewComm 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < MAX_SOCKETS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < MAX_SOCKETS; i++) {
    (*comm)->socks[i].fd = -1;
  }
  (*comm)->nextSock = 0;
  return ncclSuccess;
}

/* 中文内部逻辑 F1140：ncclSocketListen
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(dev < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：struct ncclSocketHandle* handle = (struct ncclSocketHandle*) opaqueHandle;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 调用 / 计算 / 写入：memset(handle, 0, sizeof(struct ncclSocketHandle));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 6. 源码语法片段：static_assert(sizeof(struct ncclSocketHandle) <= NCCL_NET_HANDLE_MAXSIZE, "ncclSocketHandle size too large");；按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。
 * 7. 准备局部状态：struct ncclSocketListenComm* comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketNewListenComm(&comm));；失败时可能提前return，成功才继续；分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。
 * 9. 调用 / 计算 / 写入：NCCLCHECK(GetSocketAddr(dev, &comm->sock.addr));；失败时可能提前return，成功才继续；为所选后端设备获得建链控制 socket 地址；控制握手地址与实际数据载荷通路不同。
 * 10. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketListen(&comm->sock));；失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。
 * 11. 调用 / 计算 / 写入：memcpy(&handle->connectAddr, &comm->sock.addr, sizeof(union ncclSocketAddress));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 12. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketGetNsockNthread(dev, &comm->nSocks, &comm->nThreads));；失败时可能提前return，成功才继续；结合参数和平台调优选择每连接 socket 数与后台线程数，平衡并发带宽和 CPU 开销。
 * 13. 调用 / 计算 / 写入：handle->nSocks = comm->nSocks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 14. 调用 / 计算 / 写入：handle->nThreads = comm->nThreads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：comm->sock.asyncFlag = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 调用 / 计算 / 写入：comm->dev = dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：*listenComm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1140
 */
/* 中文逐项-函数 F1140：ncclSocketListen
 * 用途与位置：按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 opaqueHandle（void* opaqueHandle）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapCreateRoot；src/bootstrap.cc::bootstrapInit；src/transport/net_ib.cc::ncclIbListen；src/transport/net_socket.cc::ncclSocketListen。
 * 本体直接出现的调用/包装：GetSocketAddr；NCCLCHECK；memcpy；memset；ncclSocketGetNsockNthread；ncclSocketListen；ncclSocketNewListenComm。
 * 需要对照的边界/推进条件：dev < 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketListen(int dev, void* opaqueHandle, void** listenComm) {
  if (dev < 0) { // data transfer socket is based on specified dev
    return ncclInternalError;
  }
/* 中文逐项-变量 V4530：handle
 * handle（struct ncclSocketHandle；函数 ncclSocketListen 内）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 初值/来源 handle = (struct ncclSocketHandle*) opaqueHandle；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketHandle* handle = (struct ncclSocketHandle*) opaqueHandle;
  memset(handle, 0, sizeof(struct ncclSocketHandle));
  static_assert(sizeof(struct ncclSocketHandle) <= NCCL_NET_HANDLE_MAXSIZE, "ncclSocketHandle size too large");
/* 中文逐项-变量 V4531：comm
 * comm（struct ncclSocketListenComm；函数 ncclSocketListen 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketNewListenComm(&comm));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclSocketListenComm* comm;
  NCCLCHECK(ncclSocketNewListenComm(&comm));
  NCCLCHECK(GetSocketAddr(dev, &comm->sock.addr));
  NCCLCHECK(ncclSocketListen(&comm->sock));
  memcpy(&handle->connectAddr, &comm->sock.addr, sizeof(union ncclSocketAddress));
  NCCLCHECK(ncclSocketGetNsockNthread(dev, &comm->nSocks, &comm->nThreads));
  handle->nSocks = comm->nSocks;
  handle->nThreads = comm->nThreads;
  comm->sock.asyncFlag = 1;
  comm->dev = dev;
  *listenComm = comm;
  return ncclSuccess;
}

/* 中文内部逻辑 F1141：ncclSocketConnect
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(dev < 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 4. 准备局部状态：enum ncclSocketState conState;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：struct ncclSocketHandle* handle = (struct ncclSocketHandle*) opaqueHandle;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 准备局部状态：struct ncclSocketCommStage* stage = &handle->stage;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 准备局部状态：struct ncclSocketComm* comm = stage->comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 8. 准备局部状态：uint8_t i = stage->iteration;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 9. 准备局部状态：struct ncclSocket* sock = stage->sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 10. 调用 / 计算 / 写入：*sendComm = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 条件判断：(stage->state == ncclSocketCommStateConnect)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   12. 真分支：；仅条件为真执行以下子项。
 *     13. 控制转移：goto socket_connect_check;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 14. 条件判断：(stage->state == ncclSocketCommStateSend)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   15. 真分支：；仅条件为真执行以下子项。
 *     16. 控制转移：goto socket_send;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 17. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketNewComm(&comm));；失败时可能提前return，成功才继续；分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。
 * 18. 调用 / 计算 / 写入：stage->comm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 19. 调用 / 计算 / 写入：comm->nSocks = handle->nSocks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 调用 / 计算 / 写入：comm->nThreads = handle->nThreads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 21. 调用 / 计算 / 写入：comm->dev = dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 22. 调用 / 计算 / 写入：CUDACHECK(cudaGetDevice(&comm->cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 23. 循环：初始 ；继续条件 i<comm->nSocks+1；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   24. 调用 / 计算 / 写入：sock = i == comm->nSocks ? &comm->ctrlSock : comm->socks+i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   25. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketInit(sock, &handle->connectAddr, NULL, 1));；失败时可能提前return，成功才继续；本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。
 *   26. 调用 / 计算 / 写入：stage->sock = sock;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   27. 调用 / 计算 / 写入：stage->state = ncclSocketCommStateConnect;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   28. 调用 / 计算 / 写入：stage->iteration = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   29. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketConnect(sock));；失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 *   30. 跳转标签：socket_connect_check；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     31. 调用 / 计算 / 写入：NCCLCHECK(ncclGetSocketState(sock, &conState));；失败时可能提前return，成功才继续；检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。
 *   32. 条件判断：(conState == ncclSocketConnecting)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     33. 真分支：；仅条件为真执行以下子项。
 *       34. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     35. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       36. 条件判断：(conState == ncclSocketError)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         37. 真分支：；仅条件为真执行以下子项。
 *           38. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   39. 调用 / 计算 / 写入：stage->state = ncclSocketCommStateSend;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   40. 跳转标签：socket_send；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     41. 准备局部状态：int done = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *   42. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, sock, &i, sizeof(uint8_t), &done));；失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。
 *   43. 条件判断：(done == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     44. 真分支：；仅条件为真执行以下子项。
 *       45. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 46. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 47. 调用 / 计算 / 写入：*sendComm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 48. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1141
 */
/* 中文逐项-函数 F1141：ncclSocketConnect
 * 用途与位置：按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 opaqueHandle（void* opaqueHandle）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 sendComm（void** sendComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRoot；src/bootstrap.cc::bootstrapInit；src/bootstrap.cc::bootstrapSend；src/proxy.cc::ncclProxyConnect；src/proxy.cc::ncclProxyDestroy；src/transport/net_ib.cc::ncclIbConnect；src/transport/net_socket.cc::ncclSocketConnect。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaGetDevice；ncclGetSocketState；ncclSocketConnect；ncclSocketInit；ncclSocketNewComm；ncclSocketProgress。
 * 需要对照的边界/推进条件：dev < 0；stage->state == ncclSocketCommStateConnect；stage->state ==
 * ncclSocketCommStateSend；i<comm->nSocks+1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;；return ncclSystemError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketConnect(int dev, void* opaqueHandle, void** sendComm) {
  if (dev < 0) { // data transfer socket is based on specified dev
    return ncclInternalError;
  }

/* 中文逐项-变量 V4532：conState
 * conState（enum ncclSocketState；函数 ncclSocketConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetSocketState(sock, &conState));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  enum ncclSocketState conState;
/* 中文逐项-变量 V4533：handle
 * handle（struct ncclSocketHandle；函数 ncclSocketConnect 内）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 初值/来源 handle = (struct ncclSocketHandle*) opaqueHandle；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketHandle* handle = (struct ncclSocketHandle*) opaqueHandle;
/* 中文逐项-变量 V4534：stage
 * stage（struct ncclSocketCommStage；函数 ncclSocketConnect 内）：对象状态机当前阶段或保留的未完成握手状态，重复调用时从这里继续。
 * 初值/来源 stage = &handle->stage；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclSocketCommStage* stage = &handle->stage;
/* 中文逐项-变量 V4535：comm
 * comm（struct ncclSocketComm；函数 ncclSocketConnect 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = stage->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocketComm* comm = stage->comm;
/* 中文逐项-变量 V4536：i
 * i（uint8_t；函数 ncclSocketConnect 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = stage->iteration；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  uint8_t i = stage->iteration;
/* 中文逐项-变量 V4537：sock
 * sock（struct ncclSocket；函数 ncclSocketConnect 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = stage->sock；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocket* sock = stage->sock;
  *sendComm = NULL;

  if (stage->state == ncclSocketCommStateConnect) goto socket_connect_check;
  if (stage->state == ncclSocketCommStateSend) goto socket_send;

  NCCLCHECK(ncclSocketNewComm(&comm));
  stage->comm = comm;
  comm->nSocks = handle->nSocks;
  comm->nThreads = handle->nThreads;
  comm->dev = dev;
  CUDACHECK(cudaGetDevice(&comm->cudaDev));
  for (; i<comm->nSocks+1; i++) {
    sock = i == comm->nSocks ? &comm->ctrlSock : comm->socks+i;
    NCCLCHECK(ncclSocketInit(sock, &handle->connectAddr, NULL, 1));

    stage->sock = sock;
    stage->state = ncclSocketCommStateConnect;
    stage->iteration = i;
    NCCLCHECK(ncclSocketConnect(sock));

socket_connect_check:
    NCCLCHECK(ncclGetSocketState(sock, &conState));
    if (conState == ncclSocketConnecting) {
      /* expect user to call again */
      return ncclSuccess;
    } else if (conState == ncclSocketError) {
      return ncclSystemError;
    }
    stage->state = ncclSocketCommStateSend;

socket_send:
/* 中文逐项-变量 V4538：done
 * done（int；函数 ncclSocketConnect 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 初值/来源 done = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int done = 0;
    NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, sock, &i, sizeof(uint8_t), &done));
    if (done == 0) return ncclSuccess;
  }
  *sendComm = comm;
  return ncclSuccess;
}

/* 中文内部逻辑 F1142：ncclSocketAccept
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocketListenComm* lComm = (struct ncclSocketListenComm*)listenComm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclSocketCommStage* stage = &lComm->stage;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct ncclSocketComm* rComm = stage->comm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 准备局部状态：uint8_t i = stage->iteration;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 5. 准备局部状态：struct ncclSocket* sock = stage->sock;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：*recvComm = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 条件判断：(stage->state == ncclSocketCommStateAccept)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 控制转移：goto socket_accept;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 10. 条件判断：(stage->state == ncclSocketCommStateRecv)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 控制转移：goto socket_recv;；跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。
 * 13. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketNewComm(&rComm));；失败时可能提前return，成功才继续；分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。
 * 14. 调用 / 计算 / 写入：stage->comm = rComm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 调用 / 计算 / 写入：rComm->nSocks = lComm->nSocks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 调用 / 计算 / 写入：rComm->nThreads = lComm->nThreads;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 调用 / 计算 / 写入：rComm->dev = lComm->dev;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 18. 调用 / 计算 / 写入：CUDACHECK(cudaGetDevice(&rComm->cudaDev));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 19. 调用 / 计算 / 写入：lComm->sock.asyncFlag = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 20. 循环：初始 ；继续条件 i<rComm->nSocks+1；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   21. 准备局部状态：uint8_t sendSockIdx;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   22. 调用 / 计算 / 写入：ncclCalloc(&sock, 1);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   23. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketInit(sock, NULL, NULL, 1));；失败时可能提前return，成功才继续；本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。
 *   24. 调用 / 计算 / 写入：stage->sock = sock;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   25. 调用 / 计算 / 写入：stage->state = ncclSocketCommStateAccept;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   26. 调用 / 计算 / 写入：stage->iteration = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   27. 跳转标签：socket_accept；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     28. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketAccept(sock, &lComm->sock));；失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。
 *   29. 条件判断：(sock->fd == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     30. 真分支：；仅条件为真执行以下子项。
 *       31. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   32. 调用 / 计算 / 写入：stage->state = ncclSocketCommStateRecv;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   33. 跳转标签：socket_recv；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *     34. 准备局部状态：int done = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *   35. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, sock, &sendSockIdx, sizeof(uint8_t), &done));；失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。
 *   36. 条件判断：(done == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     37. 真分支：；仅条件为真执行以下子项。
 *       38. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   39. 条件判断：(sendSockIdx == rComm->nSocks)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     40. 真分支：；仅条件为真执行以下子项。
 *       41. 调用 / 计算 / 写入：memcpy(&rComm->ctrlSock, sock, sizeof(struct ncclSocket));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     42. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       43. 调用 / 计算 / 写入：memcpy(rComm->socks+sendSockIdx, sock, sizeof(struct ncclSocket));；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   44. 调用 / 计算 / 写入：free(sock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 45. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 46. 调用 / 计算 / 写入：*recvComm = rComm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 47. 调用 / 计算 / 写入：stage->state = ncclSocketCommStateStart;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 48. 调用 / 计算 / 写入：stage->iteration = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 49. 调用 / 计算 / 写入：stage->sock = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 50. 调用 / 计算 / 写入：stage->comm = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 51. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1142
 */
/* 中文逐项-函数 F1142：ncclSocketAccept
 * 用途与位置：按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 recvComm（void** recvComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRoot；src/bootstrap.cc::bootstrapInit；src/bootstrap.cc::bootstrapRecv；src/proxy.cc::ncclProxyService；src/transport/net_ib.cc::ncclIbAccept；src/transport/net_socket.cc::ncclSocketAccept。
 * 本体直接出现的调用/包装：CUDACHECK；NCCLCHECK；cudaGetDevice；free；memcpy；ncclCalloc；ncclSocketAccept；ncclSocketInit；ncclSocketNewComm；ncclSocketProgress。
 * 需要对照的边界/推进条件：stage->state == ncclSocketCommStateAccept；stage->state ==
 * ncclSocketCommStateRecv；i<rComm->nSocks+1；sock->fd == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketAccept(void* listenComm, void** recvComm) {
/* 中文逐项-变量 V4539：lComm
 * lComm（struct ncclSocketListenComm；函数 ncclSocketAccept 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 lComm = (struct ncclSocketListenComm*)listenComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketListenComm* lComm = (struct ncclSocketListenComm*)listenComm;
/* 中文逐项-变量 V4540：stage
 * stage（struct ncclSocketCommStage；函数 ncclSocketAccept 内）：对象状态机当前阶段或保留的未完成握手状态，重复调用时从这里继续。
 * 初值/来源 stage = &lComm->stage；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclSocketCommStage* stage = &lComm->stage;
/* 中文逐项-变量 V4541：rComm
 * rComm（struct ncclSocketComm；函数 ncclSocketAccept 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 rComm = stage->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocketComm* rComm = stage->comm;
/* 中文逐项-变量 V4542：i
 * i（uint8_t；函数 ncclSocketAccept 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = stage->iteration；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  uint8_t i = stage->iteration;
/* 中文逐项-变量 V4543：sock
 * sock（struct ncclSocket；函数 ncclSocketAccept 内）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 初值/来源 sock = stage->sock；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocket* sock = stage->sock;

  *recvComm = NULL;
  if (stage->state == ncclSocketCommStateAccept) goto socket_accept;
  if (stage->state == ncclSocketCommStateRecv) goto socket_recv;

  NCCLCHECK(ncclSocketNewComm(&rComm));
  stage->comm = rComm;
  rComm->nSocks = lComm->nSocks;
  rComm->nThreads = lComm->nThreads;
  rComm->dev = lComm->dev;
  CUDACHECK(cudaGetDevice(&rComm->cudaDev));
  lComm->sock.asyncFlag = 1;
  for (; i<rComm->nSocks+1; i++) {
/* 中文逐项-变量 V4544：sendSockIdx
 * sendSockIdx（uint8_t；函数 ncclSocketAccept 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, sock, &sendSockIdx, sizeof(uint8_t),
 * &done));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    uint8_t sendSockIdx;
    ncclCalloc(&sock, 1);
    NCCLCHECK(ncclSocketInit(sock, NULL, NULL, 1));
    stage->sock = sock;
    stage->state = ncclSocketCommStateAccept;
    stage->iteration = i;
socket_accept:
    NCCLCHECK(ncclSocketAccept(sock, &lComm->sock));
    if (sock->fd == -1) return ncclSuccess;

    stage->state = ncclSocketCommStateRecv;
socket_recv:
/* 中文逐项-变量 V4545：done
 * done（int；函数 ncclSocketAccept 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 初值/来源 done = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int done = 0;
    NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, sock, &sendSockIdx, sizeof(uint8_t), &done));
    if (done == 0) return ncclSuccess;

    if (sendSockIdx == rComm->nSocks) memcpy(&rComm->ctrlSock, sock, sizeof(struct ncclSocket));
    else memcpy(rComm->socks+sendSockIdx, sock, sizeof(struct ncclSocket));

    free(sock);
  }
  *recvComm = rComm;

  /* reset lComm state */
  stage->state = ncclSocketCommStateStart;
  stage->iteration = 0;
  stage->sock = NULL;
  stage->comm = NULL;
  return ncclSuccess;
}

/* 中文内部逻辑 F1143：ncclSocketGetRequest
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 循环：初始 int i=0;；继续条件 i<MAX_REQUESTS；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   2. 准备局部状态：struct ncclSocketRequest* r = comm->requests+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   3. 条件判断：(r->used == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：r->op = op;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 调用 / 计算 / 写入：r->data = data;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       7. 调用 / 计算 / 写入：r->size = size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       8. 调用 / 计算 / 写入：r->ctrlSock = &comm->ctrlSock;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       9. 调用 / 计算 / 写入：r->used = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       10. 调用 / 计算 / 写入：r->comm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       11. 调用 / 计算 / 写入：r->nSubs = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       12. 调用 / 计算 / 写入：*req = r;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 15. 调用 / 计算 / 写入：WARN("NET/Socket : unable to allocate requests");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 16. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1143
 */
/* 中文逐项-函数 F1143：ncclSocketGetRequest
 * 用途与位置：从 TCP 后端固定请求表申请一项记录消息方向、地址与总长度的状态。
 * 参数 comm（struct ncclSocketComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 op（int op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 req（struct ncclSocketRequest** req）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_socket.cc::ncclSocketIsend；src/transport/net_socket.cc::ncclSocketIrecv。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<MAX_REQUESTS；r->used == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketGetRequest(struct ncclSocketComm* comm, int op, void* data, int size, struct ncclSocketRequest** req) {
  for (/* 中文逐项-变量 V4546：i
 * i（int；函数 ncclSocketGetRequest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAX_REQUESTS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAX_REQUESTS; i++) {
/* 中文逐项-变量 V4547：r
 * r（struct ncclSocketRequest；函数 ncclSocketGetRequest 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 r = comm->requests+i；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclSocketRequest* r = comm->requests+i;
    if (r->used == 0) {
      r->op = op;
      r->data = data;
      r->size = size;
      r->ctrlSock = &comm->ctrlSock;
      r->used = 1;
      r->comm = comm;
      r->nSubs = 0;
      *req = r;
      return ncclSuccess;
    }
  }
  WARN("NET/Socket : unable to allocate requests");
  return ncclInternalError;
}

/* 中文内部逻辑 F1144：ncclSocketGetTask
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int tid = comm->nextSock % comm->nThreads;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：struct ncclSocketThreadResources* res = comm->threadResources+tid;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：struct ncclSocketTaskQueue* queue = &res->threadTaskQueue;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 条件判断：(queue->tasks == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：queue->len = MAX_REQUESTS * DIVUP(comm->nSocks, comm->nThreads);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     7. 调用 / 计算 / 写入：NCCLCHECK(ncclCalloc(&queue->tasks, queue->len));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     8. 调用 / 计算 / 写入：queue->next = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     9. 调用 / 计算 / 写入：res->comm = comm;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     10. 调用 / 计算 / 写入：pthread_mutex_init(&res->threadLock, NULL);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     11. 调用 / 计算 / 写入：pthread_cond_init(&res->threadCond, NULL);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     12. 调用 / 计算 / 写入：pthread_create(comm->helperThread+tid, NULL, persistentSocketThread, res);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     13. 调用 / 计算 / 写入：ncclSetThreadName(comm->helperThread[tid], "NCCL Sock%c%1u%2u%2u", op == NCCL_SOCKET_SEND ? 'S' : 'R', comm->dev, tid, comm->cudaDev);；为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。
 * 14. 准备局部状态：struct ncclSocketTask* r = queue->tasks+queue->next;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 15. 条件判断：(r->used == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   16. 真分支：；仅条件为真执行以下子项。
 *     17. 调用 / 计算 / 写入：r->op = op;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     18. 调用 / 计算 / 写入：r->data = data;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     19. 调用 / 计算 / 写入：r->size = size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     20. 调用 / 计算 / 写入：r->sock = comm->socks+comm->nextSock;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     21. 调用 / 计算 / 写入：r->offset = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     22. 调用 / 计算 / 写入：r->result = ncclSuccess;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     23. 调用 / 计算 / 写入：comm->nextSock = (comm->nextSock + 1) % comm->nSocks;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     24. 调用 / 计算 / 写入：r->used = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     25. 调用 / 计算 / 写入：*req = r;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     26. 调用 / 计算 / 写入：pthread_mutex_lock(&res->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     27. 调用 / 计算 / 写入：queue->next = (queue->next+1)%queue->len;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     28. 调用 / 计算 / 写入：pthread_cond_signal(&res->threadCond);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     29. 调用 / 计算 / 写入：pthread_mutex_unlock(&res->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     30. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 31. 调用 / 计算 / 写入：WARN("NET/Socket : unable to allocate subtasks");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 32. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1144
 */
/* 中文逐项-函数 F1144：ncclSocketGetTask
 * 用途与位置：把请求划为 TCP 子任务并交给相应 socket/线程，复用任务队列与后台线程。
 * 参数 comm（struct ncclSocketComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 op（int op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 req（struct ncclSocketTask** req）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_socket.cc::ncclSocketTest。
 * 本体直接出现的调用/包装：DIVUP；NCCLCHECK；ncclCalloc；ncclSetThreadName；pthread_cond_init；pthread_cond_signal；pthread_create；pthread_mutex_init；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：queue->tasks == NULL；r->used == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclSocketGetTask(struct ncclSocketComm* comm, int op, void* data, int size, struct ncclSocketTask** req) {
/* 中文逐项-变量 V4548：tid
 * tid（int；函数 ncclSocketGetTask 内）：当前执行范围内线程号，设备代码可由 threadIdx.x 派生或减去组起点。
 * 初值/来源 tid = comm->nextSock % comm->nThreads；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int tid = comm->nextSock % comm->nThreads;
/* 中文逐项-变量 V4549：res
 * res（struct ncclSocketThreadResources；函数 ncclSocketGetTask 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = comm->threadResources+tid；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocketThreadResources* res = comm->threadResources+tid;
/* 中文逐项-变量 V4550：queue
 * queue（struct ncclSocketTaskQueue；函数 ncclSocketGetTask 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 queue = &res->threadTaskQueue；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclSocketTaskQueue* queue = &res->threadTaskQueue;
  // create helper threads and prepare per-thread task queue
  if (queue->tasks == NULL) {
    // each request can be divided up to nSocks tasks, and
    // these tasks are distributed to nThreads threads,
    // we need to make sure each thread queue has enough slots for MAX_REQUESTS
    queue->len = MAX_REQUESTS * DIVUP(comm->nSocks, comm->nThreads);
    NCCLCHECK(ncclCalloc(&queue->tasks, queue->len));
    queue->next = 0;
    res->comm = comm;
    pthread_mutex_init(&res->threadLock, NULL);
    pthread_cond_init(&res->threadCond, NULL);
    pthread_create(comm->helperThread+tid, NULL, persistentSocketThread, res);
    ncclSetThreadName(comm->helperThread[tid], "NCCL Sock%c%1u%2u%2u", op == NCCL_SOCKET_SEND ? 'S' : 'R', comm->dev, tid, comm->cudaDev);
  }
/* 中文逐项-变量 V4551：r
 * r（struct ncclSocketTask；函数 ncclSocketGetTask 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 r = queue->tasks+queue->next；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclSocketTask* r = queue->tasks+queue->next;
  if (r->used == 0) {
    r->op = op;
    r->data = data;
    r->size = size;
    r->sock = comm->socks+comm->nextSock;
    r->offset = 0;
    r->result = ncclSuccess;
    comm->nextSock = (comm->nextSock + 1) % comm->nSocks;
    r->used = 1;
    *req = r;
    pthread_mutex_lock(&res->threadLock);
    queue->next = (queue->next+1)%queue->len;
    pthread_cond_signal(&res->threadCond);
    pthread_mutex_unlock(&res->threadLock);
    return ncclSuccess;
  }
  WARN("NET/Socket : unable to allocate subtasks");
  return ncclInternalError;
}

/* 中文精读：Socket 后端也实现异步 request/test 契约
 * Test 先推进控制信息与任务分片，再检查各 socket 子任务的 offset 是否达到目标长度。
 * read/write 可能只完成一部分，必须保留 offset 继续，不能把一次系统调用当成整条消息完成。
 * 多 socket/线程用于并行推进 TCP 数据，最终 done 仍回到上层 Proxy 的相同接口。
 * 该后端仅接受主机指针注册，不创建 verbs QP/MR，网络路径会使用可访问的主机通信缓冲。
 */
/* 中文内部逻辑 F1145：ncclSocketTest
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*done = 0;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 * 2. 准备局部状态：struct ncclSocketRequest *r = (struct ncclSocketRequest*)request;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(r == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：WARN("NET/Socket : test called with NULL request");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     6. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 条件判断：(r->used == 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   8. 真分支：；仅条件为真执行以下子项。
 *     9. 准备局部状态：int data = r->size;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     10. 准备局部状态：int offset = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     11. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketProgress(r->op, r->ctrlSock, &data, sizeof(int), &offset));；失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。
 *     12. 条件判断：(offset == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       13. 真分支：；仅条件为真执行以下子项。
 *         14. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     15. 条件判断：(offset < sizeof(int))；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       16. 真分支：；仅条件为真执行以下子项。
 *         17. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketWait(r->op, r->ctrlSock, &data, sizeof(int), &offset));；失败时可能提前return，成功才继续；重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。
 *     18. 条件判断：(r->op == NCCL_SOCKET_RECV && data > r->size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       19. 真分支：；仅条件为真执行以下子项。
 *         20. 准备局部状态：char line[SOCKET_NAME_MAXLEN+1];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         21. 调用 / 计算 / 写入：WARN("NET/Socket : peer %s message truncated : receiving %d bytes instead of %d. If you believe your socket network is in healthy state, \ there may be a mismatch in collective sizes or environment se …（完整代码见函数体）；把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。
 *         22. 函数返回：return ncclInvalidUsage;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     23. 调用 / 计算 / 写入：r->size = data;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     24. 调用 / 计算 / 写入：r->offset = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     25. 调用 / 计算 / 写入：r->used = 2;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     26. 准备局部状态：int chunkOffset = 0, i = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     27. 条件判断：(r->comm->nSocks > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       28. 真分支：；仅条件为真执行以下子项。
 *         29. 准备局部状态：int taskSize = std::max(MIN_CHUNKSIZE, DIVUP(r->size, r->comm->nSocks));；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         30. 循环：初始 ；继续条件 (chunkOffset < r->size)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           31. 准备局部状态：int chunkSize = std::min(taskSize, r->size-chunkOffset);；对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。
 *           32. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketGetTask(r->comm, r->op, (char*)(r->data)+chunkOffset, chunkSize, r->tasks+i++));；失败时可能提前return，成功才继续；把请求划为 TCP 子任务并交给相应 socket/线程，复用任务队列与后台线程。
 *           33. 调用 / 计算 / 写入：chunkOffset += chunkSize;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         34. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     35. 调用 / 计算 / 写入：r->nSubs = i;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 36. 条件判断：(r->used == 2)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   37. 真分支：；仅条件为真执行以下子项。
 *     38. 条件判断：(r->nSubs > 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       39. 真分支：；仅条件为真执行以下子项。
 *         40. 准备局部状态：int nCompleted = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *         41. 循环：初始 int i=0;；继续条件 i<r->nSubs；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *           42. 准备局部状态：struct ncclSocketTask* sub = r->tasks[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *           43. 条件判断：(sub->result != ncclSuccess)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             44. 真分支：；仅条件为真执行以下子项。
 *               45. 函数返回：return sub->result;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *           46. 条件判断：(sub->offset == sub->size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *             47. 真分支：；仅条件为真执行以下子项。
 *               48. 调用 / 计算 / 写入：nCompleted++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *         49. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *         50. 条件判断：(nCompleted == r->nSubs)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           51. 真分支：；仅条件为真执行以下子项。
 *             52. 条件判断：(size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               53. 真分支：；仅条件为真执行以下子项。
 *                 54. 调用 / 计算 / 写入：*size = r->size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             55. 调用 / 计算 / 写入：*done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *             56. 调用 / 计算 / 写入：r->used = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             57. 循环：初始 int i=0;；继续条件 i<r->nSubs；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *               58. 准备局部状态：struct ncclSocketTask* sub = r->tasks[i];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *               59. 调用 / 计算 / 写入：sub->used = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             60. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *       61. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         62. 条件判断：(r->offset < r->size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           63. 真分支：；仅条件为真执行以下子项。
 *             64. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketProgress(r->op, r->ctrlSock, r->data, r->size, &r->offset));；失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。
 *         65. 条件判断：(r->offset == r->size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *           66. 真分支：；仅条件为真执行以下子项。
 *             67. 条件判断：(size)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *               68. 真分支：；仅条件为真执行以下子项。
 *                 69. 调用 / 计算 / 写入：*size = r->size;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *             70. 调用 / 计算 / 写入：*done = 1;；读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。
 *             71. 调用 / 计算 / 写入：r->used = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 72. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1145
 */
/* 中文逐项-函数 F1145：ncclSocketTest
 * 用途与位置：推进控制长度交换并检查所有子任务 offset，直到一条上层异步请求完成。
 * 参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 参数 size（int* size）：网络请求完成后的有效字节数输出地址；可为空时调用者不要求返回长度。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：DIVUP；NCCLCHECK；ncclSocketGetTask；ncclSocketProgress；ncclSocketToString；ncclSocketWait；std::max；std::min。
 * 需要对照的边界/推进条件：r == NULL；r->used == 1；offset == 0；offset < sizeof(int。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;；return ncclInvalidUsage;；return
 * sub->result;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketTest(void* request, int* done, int* size) {
  *done = 0;
/* 中文逐项-变量 V4552：r
 * r（struct ncclSocketRequest；函数 ncclSocketTest 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 r = (struct ncclSocketRequest*)request；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketRequest *r = (struct ncclSocketRequest*)request;
  if (r == NULL) {
    WARN("NET/Socket : test called with NULL request");
    return ncclInternalError;
  }
  if (r->used == 1) { /* try to send/recv size */
/* 中文逐项-变量 V4553：data
 * data（int；函数 ncclSocketTest 内）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 初值/来源 data = r->size；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int data = r->size;
/* 中文逐项-变量 V4554：offset
 * offset（int；函数 ncclSocketTest 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int offset = 0;
    NCCLCHECK(ncclSocketProgress(r->op, r->ctrlSock, &data, sizeof(int), &offset));

    if (offset == 0) return ncclSuccess; /* Not ready -- retry later */

    // Not sure we could ever receive less than 4 bytes, but just in case ...
    if (offset < sizeof(int)) NCCLCHECK(ncclSocketWait(r->op, r->ctrlSock, &data, sizeof(int), &offset));

    // Check size is less or equal to the size provided by the user
    if (r->op == NCCL_SOCKET_RECV && data > r->size) {
/* 中文逐项-变量 V4555：line
 * line（char；函数 ncclSocketTest 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：ncclSocketToString(&r->ctrlSock->addr, line), data, r->size);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char line[SOCKET_NAME_MAXLEN+1];
      WARN("NET/Socket : peer %s message truncated : receiving %d bytes instead of %d. If you believe your socket network is in healthy state, \
          there may be a mismatch in collective sizes or environment settings (e.g. NCCL_PROTO, NCCL_ALGO) between ranks",
          ncclSocketToString(&r->ctrlSock->addr, line), data, r->size);
      return ncclInvalidUsage;
    }
    r->size = data;
    r->offset = 0;
    r->used = 2; // done exchanging size
    // divide into subtasks
/* 中文逐项-变量 V4556：chunkOffset、i
 * chunkOffset（int；函数 ncclSocketTest 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 chunkOffset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * i（int；函数 ncclSocketTest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int chunkOffset = 0, i = 0;
    if (r->comm->nSocks > 0) {
      // each request can be divided up to nSocks tasks
/* 中文逐项-变量 V4557：taskSize
 * taskSize（int；函数 ncclSocketTest 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 taskSize = std::max(MIN_CHUNKSIZE, DIVUP(r->size, r->comm->nSocks))；保留下限或较大候选，避免并行度/尺寸低于需要的最小值。
 */
      int taskSize = std::max(MIN_CHUNKSIZE, DIVUP(r->size, r->comm->nSocks));
      while (chunkOffset < r->size) {
/* 中文逐项-变量 V4558：chunkSize
 * chunkSize（int；函数 ncclSocketTest 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = std::min(taskSize, r->size-chunkOffset)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
        int chunkSize = std::min(taskSize, r->size-chunkOffset);
        NCCLCHECK(ncclSocketGetTask(r->comm, r->op, (char*)(r->data)+chunkOffset, chunkSize, r->tasks+i++));
        chunkOffset += chunkSize;
      }
    }
    r->nSubs = i;
  }
  if (r->used == 2) { // already exchanged size
    if (r->nSubs > 0) {
/* 中文逐项-变量 V4559：nCompleted
 * nCompleted（int；函数 ncclSocketTest 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nCompleted = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
      int nCompleted = 0;
      for (/* 中文逐项-变量 V4560：i
 * i（int；函数 ncclSocketTest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<r->nSubs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<r->nSubs; i++) {
/* 中文逐项-变量 V4561：sub
 * sub（struct ncclSocketTask；函数 ncclSocketTest 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = r->tasks[i]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
        struct ncclSocketTask* sub = r->tasks[i];
        if (sub->result != ncclSuccess) return sub->result;
        if (sub->offset == sub->size) nCompleted++;
      }
      if (nCompleted == r->nSubs) {
        if (size) *size = r->size;
        *done = 1;
        r->used = 0;
        for (/* 中文逐项-变量 V4562：i
 * i（int；函数 ncclSocketTest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<r->nSubs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<r->nSubs; i++) {
/* 中文逐项-变量 V4563：sub
 * sub（struct ncclSocketTask；函数 ncclSocketTest 内）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 初值/来源 sub = r->tasks[i]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          struct ncclSocketTask* sub = r->tasks[i];
          sub->used = 0;
        }
      }
    } else { // progress request using main thread
      if (r->offset < r->size) {
        NCCLCHECK(ncclSocketProgress(r->op, r->ctrlSock, r->data, r->size, &r->offset));
      }
      if (r->offset == r->size) {
        if (size) *size = r->size;
        *done = 1;
        r->used = 0;
      }
    }
  }
  return ncclSuccess;
}

/* 中文导读：
 * Socket 后端为适配统一接口提供的注册回调，不执行 ibv_reg_mr。
 * 因此走 Socket 的同一上层任务不会按内置 IB 的公式消耗 RDMA QP/MR，但有 socket/线程/缓冲开销。
 */
/* 中文内部逻辑 F1146：ncclSocketRegMr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return (type != NCCL_PTR_HOST) ? ncclInternalError : ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1146
 */
/* 中文逐项-函数 F1146：ncclSocketRegMr
 * 用途与位置：仅验证主机指针类型，TCP 后端无需创建 verbs MR；不支持直接传 GPU 指针。
 * 参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return (type != NCCL_PTR_HOST) ? ncclInternalError : ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketRegMr(void* comm, void* data, int size, int type, void** mhandle) {
  return (type != NCCL_PTR_HOST) ? ncclInternalError : ncclSuccess;
}
/* 中文内部逻辑 F1147：ncclSocketDeregMr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1147
 */
/* 中文逐项-函数 F1147：ncclSocketDeregMr
 * 用途与位置：TCP 后端没有真实 verbs MR，可直接成功完成接口对应的注销动作。
 * 参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketDeregMr(void* comm, void* mhandle) { return ncclSuccess; }

/* 中文内部逻辑 F1148：ncclSocketIsend
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocketComm* comm = (struct ncclSocketComm*)sendComm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketGetRequest(comm, NCCL_SOCKET_SEND, data, size, (struct ncclSocketRequest**)request));；失败时可能提前return，成功才继续；从 TCP 后端固定请求表申请一项记录消息方向、地址与总长度的状态。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1148
 */
/* 中文逐项-函数 F1148：ncclSocketIsend
 * 用途与位置：为 TCP 消息分配异步请求，数据在后续 Test/线程推进中完成，不保证本调用返回时已传完。
 * 参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketGetRequest。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketIsend(void* sendComm, void* data, int size, int tag, void* mhandle, void** request) {
/* 中文逐项-变量 V4564：comm
 * comm（struct ncclSocketComm；函数 ncclSocketIsend 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclSocketComm*)sendComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketComm* comm = (struct ncclSocketComm*)sendComm;
  NCCLCHECK(ncclSocketGetRequest(comm, NCCL_SOCKET_SEND, data, size, (struct ncclSocketRequest**)request));
  return ncclSuccess;
}

/* 中文内部逻辑 F1149：ncclSocketIrecv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocketComm* comm = (struct ncclSocketComm*)recvComm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(n != 1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 调用 / 计算 / 写入：NCCLCHECK(ncclSocketGetRequest(comm, NCCL_SOCKET_RECV, data[0], sizes[0], (struct ncclSocketRequest**)request));；失败时可能提前return，成功才继续；从 TCP 后端固定请求表申请一项记录消息方向、地址与总长度的状态。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1149
 */
/* 中文逐项-函数 F1149：ncclSocketIrecv
 * 用途与位置：为 TCP 消息分配异步请求，数据在后续 Test/线程推进中完成，不保证本调用返回时已传完。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 tags（int* tags）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketGetRequest。
 * 需要对照的边界/推进条件：n != 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketIrecv(void* recvComm, int n, void** data, int* sizes, int* tags, void** mhandles, void** request) {
/* 中文逐项-变量 V4565：comm
 * comm（struct ncclSocketComm；函数 ncclSocketIrecv 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclSocketComm*)recvComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketComm* comm = (struct ncclSocketComm*)recvComm;
  if (n != 1) return ncclInternalError;
  NCCLCHECK(ncclSocketGetRequest(comm, NCCL_SOCKET_RECV, data[0], sizes[0], (struct ncclSocketRequest**)request));
  return ncclSuccess;
}

/* 中文内部逻辑 F1150：ncclSocketIflush
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1150
 */
/* 中文逐项-函数 F1150：ncclSocketIflush
 * 用途与位置：TCP 主机缓冲路径不需 GPU RDMA flush，对应接口立即完成而不创建额外硬件请求。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketIflush(void* recvComm, int n, void** data, int* sizes, void** mhandles, void** request) {
  // We don't support CUDA pointers, so we don't need a flush operation
  return ncclInternalError;
}

/* 中文内部逻辑 F1151：ncclSocketCloseListen
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocketListenComm* comm = (struct ncclSocketListenComm*)opaqueComm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(comm)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 条件判断：(comm->sock.fd != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       5. 真分支：；仅条件为真执行以下子项。
 *         6. 调用 / 计算 / 写入：close(comm->sock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     7. 调用 / 计算 / 写入：free(comm);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1151
 */
/* 中文逐项-函数 F1151：ncclSocketCloseListen
 * 用途与位置：关闭 TCP 后端监听 socket 并释放其监听对象。
 * 参数 opaqueComm（void* opaqueComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：close；free。
 * 需要对照的边界/推进条件：comm；comm->sock.fd != -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketCloseListen(void* opaqueComm) {
/* 中文逐项-变量 V4566：comm
 * comm（struct ncclSocketListenComm；函数 ncclSocketCloseListen 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclSocketListenComm*)opaqueComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketListenComm* comm = (struct ncclSocketListenComm*)opaqueComm;
  if (comm) {
    if (comm->sock.fd != -1) close(comm->sock.fd);
    free(comm);
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F1152：ncclSocketClose
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct ncclSocketComm* comm = (struct ncclSocketComm*)opaqueComm;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(comm)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 循环：初始 int i=0;；继续条件 i<comm->nThreads；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       5. 准备局部状态：struct ncclSocketThreadResources* res = comm->threadResources+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       6. 条件判断：(comm->helperThread[i])；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         7. 真分支：；仅条件为真执行以下子项。
 *           8. 调用 / 计算 / 写入：pthread_mutex_lock(&res->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           9. 调用 / 计算 / 写入：res->stop = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           10. 调用 / 计算 / 写入：pthread_cond_signal(&res->threadCond);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           11. 调用 / 计算 / 写入：pthread_mutex_unlock(&res->threadLock);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *           12. 调用 / 计算 / 写入：pthread_join(comm->helperThread[i], NULL);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *       13. 调用 / 计算 / 写入：free(res->threadTaskQueue.tasks);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     14. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     15. 条件判断：(comm->ctrlSock.fd != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       16. 真分支：；仅条件为真执行以下子项。
 *         17. 调用 / 计算 / 写入：close(comm->ctrlSock.fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     18. 循环：初始 int i=0;；继续条件 i<comm->nSocks；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       19. 条件判断：(comm->socks[i].fd != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         20. 真分支：；仅条件为真执行以下子项。
 *           21. 调用 / 计算 / 写入：close(comm->socks[i].fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     22. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     23. 调用 / 计算 / 写入：free(comm);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 24. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__transport__net_socket_cc.md#f1152
 */
/* 中文逐项-函数 F1152：ncclSocketClose
 * 用途与位置：通知 TCP 通信后台线程停止，关闭各数据/控制 socket 并释放任务、请求及线程资源。
 * 参数 opaqueComm（void* opaqueComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：close；free；pthread_cond_signal；pthread_join；pthread_mutex_lock；pthread_mutex_unlock。
 * 需要对照的边界/推进条件：comm；i<comm->nThreads；comm->helperThread[i]；comm->ctrlSock.fd != -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclSocketClose(void* opaqueComm) {
/* 中文逐项-变量 V4567：comm
 * comm（struct ncclSocketComm；函数 ncclSocketClose 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclSocketComm*)opaqueComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclSocketComm* comm = (struct ncclSocketComm*)opaqueComm;
  if (comm) {
    for (/* 中文逐项-变量 V4568：i
 * i（int；函数 ncclSocketClose 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->nThreads；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->nThreads; i++) {
/* 中文逐项-变量 V4569：res
 * res（struct ncclSocketThreadResources；函数 ncclSocketClose 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = comm->threadResources+i；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclSocketThreadResources* res = comm->threadResources+i;
      if (comm->helperThread[i]) {
        pthread_mutex_lock(&res->threadLock);
        res->stop = 1;
        pthread_cond_signal(&res->threadCond);
        pthread_mutex_unlock(&res->threadLock);
        pthread_join(comm->helperThread[i], NULL);
      }
      free(res->threadTaskQueue.tasks);
    }
    if (comm->ctrlSock.fd != -1) close(comm->ctrlSock.fd);
    for (/* 中文逐项-变量 V4570：i
 * i（int；函数 ncclSocketClose 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<comm->nSocks；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<comm->nSocks; i++) {
      if (comm->socks[i].fd != -1) close(comm->socks[i].fd);
    }
    free(comm);
  }
  return ncclSuccess;
}

/* 中文逐项-变量 V4571：ncclNetSocket
 * ncclNetSocket（ncclNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclNetSocket = { "Socket", ncclSocketInit, ncclSocketDevices, ncclSocketGetProperties, ncclSocketListen,
 * ncclSocketConnect, ncclSocketAccept, ncclSocketRegMr, ncclSocketDeregMr, ncclSocketIsend, ncclSocketIrecv,
 * ncclSocketIflush, ncclSocketTest, …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
ncclNet_t ncclNetSocket = {
  "Socket",
  ncclSocketInit,
  ncclSocketDevices,
  ncclSocketGetProperties,
  ncclSocketListen,
  ncclSocketConnect,
  ncclSocketAccept,
  ncclSocketRegMr,
  ncclSocketDeregMr,
  ncclSocketIsend,
  ncclSocketIrecv,
  ncclSocketIflush,
  ncclSocketTest,
  ncclSocketClose,
  ncclSocketClose,
  ncclSocketCloseListen
};
