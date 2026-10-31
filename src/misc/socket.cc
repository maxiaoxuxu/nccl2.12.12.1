/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "socket.h"
#include "utils.h"
#include <stdlib.h>

#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>

/* Format a string representation of a (union ncclSocketAddress *) socket address using getnameinfo()
 *
 * Output: "IPv4/IPv6 address<port>"
 */
/* 中文逐项-函数 F0971：ncclSocketToString
 * 用途与位置：把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。
 * 参数 addr（union ncclSocketAddress *addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 参数 buf（char *buf）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 numericHostForm（const int numericHostForm）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapNetInit；src/misc/socket.cc::findInterfaces；src/misc/socket.cc::ncclFindInterfaceMatchSubnet；src/misc/socket.cc::ncclSocketListen；src/misc/socket.cc::ncclSocketConnect；src/misc/socket.cc::ncclSocketProgressOpt；src/misc/socket.cc::ncclSocketProgress；其余见调用清单。
 * 本体直接出现的调用/包装：getnameinfo；sprintf。
 * 需要对照的边界/推进条件：buf == NULL || addr == NULL；saddr->sa_family != AF_INET && saddr->sa_family != AF_INET6。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;；return buf;。CHECK类宏还可能提前返回/跳转。
 */
const char *ncclSocketToString(union ncclSocketAddress *addr, char *buf, const int numericHostForm /*= 1*/) {
  if (buf == NULL || addr == NULL) return NULL;
/* 中文逐项-变量 V3458：saddr
 * saddr（struct sockaddr；函数 ncclSocketToString 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 saddr = &addr->sa；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct sockaddr *saddr = &addr->sa;
  if (saddr->sa_family != AF_INET && saddr->sa_family != AF_INET6) { buf[0]='\0'; return buf; }
/* 中文逐项-变量 V3459：host、service
 * host（char；函数 ncclSocketToString 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：(void) getnameinfo(saddr, sizeof(union ncclSocketAddress), host, NI_MAXHOST, service,
 * NI_MAXSERV, flag);。若由输出参数填入，必须先检查填充调用成功再读值。
 * service（char；函数 ncclSocketToString 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：(void) getnameinfo(saddr, sizeof(union ncclSocketAddress), host, NI_MAXHOST, service,
 * NI_MAXSERV, flag);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char host[NI_MAXHOST], service[NI_MAXSERV];
  /* NI_NUMERICHOST: If set, then the numeric form of the hostname is returned.
   * (When not set, this will still happen in case the node's name cannot be determined.)
   */
/* 中文逐项-变量 V3460：flag
 * flag（int；函数 ncclSocketToString 内）：当前协议轮次的就绪序号，匹配 step 才表示对应行的新数据完整。
 * 初值/来源 flag = NI_NUMERICSERV | (numericHostForm ? NI_NUMERICHOST : 0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int flag = NI_NUMERICSERV | (numericHostForm ? NI_NUMERICHOST : 0);
  (void) getnameinfo(saddr, sizeof(union ncclSocketAddress), host, NI_MAXHOST, service, NI_MAXSERV, flag);
  sprintf(buf, "%s<%s>", host, service);
  return buf;
}

/* 中文逐项-函数 F0972：socketToPort
 * 用途与位置：从通用 socket 地址的 IPv4/IPv6 具体分支取出端口号。
 * 参数 addr（union ncclSocketAddress *addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketListen。
 * 本体直接出现的调用/包装：ntohs。
 * 源码返回表达式：return ntohs(saddr->sa_family == AF_INET ? addr->sin.sin_port : addr->sin6.sin6_port);。CHECK类宏还可能提前返回/跳转。
 */
static uint16_t socketToPort(union ncclSocketAddress *addr) {
/* 中文逐项-变量 V3461：saddr
 * saddr（struct sockaddr；函数 socketToPort 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 saddr = &addr->sa；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct sockaddr *saddr = &addr->sa;
  return ntohs(saddr->sa_family == AF_INET ? addr->sin.sin_port : addr->sin6.sin6_port);
}

/* Allow the user to force the IPv4/IPv6 interface selection */
/* 中文逐项-函数 F0973：envSocketFamily
 * 用途与位置：解析环境选择的 IPv4/IPv6 地址族，影响接口筛选与连接建立。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclFindInterfaces。
 * 本体直接出现的调用/包装：getenv；strcmp。
 * 需要对照的边界/推进条件：env == NULL；strcmp(env, "AF_INET") == 0；strcmp(env, "AF_INET6") == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return family;。CHECK类宏还可能提前返回/跳转。
 */
static int envSocketFamily(void) {
/* 中文逐项-变量 V3462：family
 * family（int；函数 envSocketFamily 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 family = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int family = -1; // Family selection is not forced, will use first one found
/* 中文逐项-变量 V3463：env
 * env（char；函数 envSocketFamily 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 env = getenv("NCCL_SOCKET_FAMILY")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* env = getenv("NCCL_SOCKET_FAMILY");
  if (env == NULL)
    return family;

  INFO(NCCL_ENV, "NCCL_SOCKET_FAMILY set by environment to %s", env);

  if (strcmp(env, "AF_INET") == 0)
    family = AF_INET;  // IPv4
  else if (strcmp(env, "AF_INET6") == 0)
    family = AF_INET6; // IPv6
  return family;
}

/* 中文逐项-函数 F0974：findInterfaces
 * 用途与位置：遍历系统接口，按地址族/名字前缀等筛选，保存有用接口名与地址。
 * 参数 prefixList（const char* prefixList）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 names（char* names）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 addrs（union ncclSocketAddress *addrs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 sock_family（int sock_family）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 maxIfNameSize（int maxIfNameSize）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 参数 maxIfs（int maxIfs）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclFindInterfaces。
 * 本体直接出现的调用/包装：IN6_IS_ADDR_LOOPBACK；freeifaddrs；getifaddrs；matchIfList；memcpy；ncclSocketToString；parseStringList；strcmp；strncpy。
 * 需要对照的边界/推进条件：searchNot；searchExact；interface && found < maxIfs；interface->ifa_addr == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return found;。CHECK类宏还可能提前返回/跳转。
 */
static int findInterfaces(const char* prefixList, char* names, union ncclSocketAddress *addrs, int sock_family, int maxIfNameSize, int maxIfs) {
#ifdef ENABLE_TRACE
/* 中文逐项-变量 V3464：line
 * line（char；函数 findInterfaces 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：TRACE(NCCL_INIT|NCCL_NET,"Found interface %s:%s", interface->ifa_name, ncclSocketToString((union
 * ncclSocketAddress *) interface->ifa_addr, line));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[SOCKET_NAME_MAXLEN+1];
#endif
/* 中文逐项-变量 V3465：userIfs
 * userIfs（struct netIf；函数 findInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：int nUserIfs = parseStringList(prefixList, userIfs, MAX_IFS);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct netIf userIfs[MAX_IFS];
/* 中文逐项-变量 V3466：searchNot
 * searchNot（bool；函数 findInterfaces 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 searchNot = prefixList && prefixList[0] == '^'；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  bool searchNot = prefixList && prefixList[0] == '^';
  if (searchNot) prefixList++;
/* 中文逐项-变量 V3467：searchExact
 * searchExact（bool；函数 findInterfaces 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 searchExact = prefixList && prefixList[0] == '='；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  bool searchExact = prefixList && prefixList[0] == '=';
  if (searchExact) prefixList++;
/* 中文逐项-变量 V3468：nUserIfs
 * nUserIfs（int；函数 findInterfaces 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nUserIfs = parseStringList(prefixList, userIfs, MAX_IFS)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int nUserIfs = parseStringList(prefixList, userIfs, MAX_IFS);

/* 中文逐项-变量 V3469：found
 * found（int；函数 findInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 found = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int found = 0;
/* 中文逐项-变量 V3470：interfaces、interface
 * interfaces（struct ifaddrs；函数 findInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：getifaddrs(&interfaces);。若由输出参数填入，必须先检查填充调用成功再读值。
 * interface（struct ifaddrs；函数 findInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (interface = interfaces; interface && found < maxIfs; interface = interface->ifa_next)
 * {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ifaddrs *interfaces, *interface;
  getifaddrs(&interfaces);
  for (interface = interfaces; interface && found < maxIfs; interface = interface->ifa_next) {
    if (interface->ifa_addr == NULL) continue;

    /* We only support IPv4 & IPv6 */
/* 中文逐项-变量 V3471：family
 * family（int；函数 findInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 family = interface->ifa_addr->sa_family；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int family = interface->ifa_addr->sa_family;
    if (family != AF_INET && family != AF_INET6)
      continue;

    TRACE(NCCL_INIT|NCCL_NET,"Found interface %s:%s", interface->ifa_name, ncclSocketToString((union ncclSocketAddress *) interface->ifa_addr, line));

    /* Allow the caller to force the socket family type */
    if (sock_family != -1 && family != sock_family)
      continue;

    /* We also need to skip IPv6 loopback interfaces */
    if (family == AF_INET6) {
/* 中文逐项-变量 V3472：sa
 * sa（struct sockaddr_in6；函数 findInterfaces 内）：通用sockaddr、IPv4与IPv6地址视图，先看地址族再选择对应union成员。
 * 初值/来源 sa = (struct sockaddr_in6*)(interface->ifa_addr)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct sockaddr_in6* sa = (struct sockaddr_in6*)(interface->ifa_addr);
      if (IN6_IS_ADDR_LOOPBACK(&sa->sin6_addr)) continue;
    }

    // check against user specified interfaces
    if (!(matchIfList(interface->ifa_name, -1, userIfs, nUserIfs, searchExact) ^ searchNot)) {
      continue;
    }

    // Check that this interface has not already been saved
    // getifaddrs() normal order appears to be; IPv4, IPv6 Global, IPv6 Link
/* 中文逐项-变量 V3473：duplicate
 * duplicate（bool；函数 findInterfaces 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 duplicate = false；布尔初态为false，后续条件决定是否切换。
 */
    bool duplicate = false;
    for (/* 中文逐项-变量 V3474：i
 * i（int；函数 findInterfaces 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < found；循环体与增量表达式决定每轮如何推进。
 */int i = 0; i < found; i++) {
      if (strcmp(interface->ifa_name, names+i*maxIfNameSize) == 0) { duplicate = true; break; }
    }

    if (!duplicate) {
      // Store the interface name
      strncpy(names+found*maxIfNameSize, interface->ifa_name, maxIfNameSize);
      // Store the IP address
/* 中文逐项-变量 V3475：salen
 * salen（int；函数 findInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct
 * sockaddr_in6)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
      int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
      memcpy(addrs+found, interface->ifa_addr, salen);
      found++;
    }
  }

  freeifaddrs(interfaces);
  return found;
}

/* 中文逐项-函数 F0975：matchSubnet
 * 用途与位置：比较本地接口与远端地址在相同地址族/掩码下是否属于同一子网。
 * 参数 local_if（struct ifaddrs local_if）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 remote（union ncclSocketAddress* remote）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclFindInterfaceMatchSubnet。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：family != remote->sa.sa_family；family == AF_INET；family == AF_INET6；c < len。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return false;；return (local_subnet.s_addr ^ remote_subnet.s_addr) ? false : true;；return
 * same;。CHECK类宏还可能提前返回/跳转。
 */
static bool matchSubnet(struct ifaddrs local_if, union ncclSocketAddress* remote) {
  /* Check family first */
/* 中文逐项-变量 V3476：family
 * family（int；函数 matchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 family = local_if.ifa_addr->sa_family；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int family = local_if.ifa_addr->sa_family;
  if (family != remote->sa.sa_family) {
    return false;
  }

  if (family == AF_INET) {
/* 中文逐项-变量 V3477：local_addr
 * local_addr（struct sockaddr_in；函数 matchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local_addr = (struct sockaddr_in*)(local_if.ifa_addr)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct sockaddr_in* local_addr = (struct sockaddr_in*)(local_if.ifa_addr);
/* 中文逐项-变量 V3478：mask
 * mask（struct sockaddr_in；函数 matchSubnet 内）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 * 初值/来源 mask = (struct sockaddr_in*)(local_if.ifa_netmask)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct sockaddr_in* mask = (struct sockaddr_in*)(local_if.ifa_netmask);
    struct sockaddr_in& remote_addr = remote->sin;
/* 中文逐项-变量 V3479：local_subnet、remote_subnet
 * local_subnet（struct in_addr；函数 matchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：local_subnet.s_addr = local_addr->sin_addr.s_addr &
 * mask->sin_addr.s_addr;。若由输出参数填入，必须先检查填充调用成功再读值。
 * remote_subnet（struct in_addr；函数 matchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：remote_subnet.s_addr = remote_addr.sin_addr.s_addr &
 * mask->sin_addr.s_addr;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct in_addr local_subnet, remote_subnet;
    local_subnet.s_addr = local_addr->sin_addr.s_addr & mask->sin_addr.s_addr;
    remote_subnet.s_addr = remote_addr.sin_addr.s_addr & mask->sin_addr.s_addr;
    return (local_subnet.s_addr ^ remote_subnet.s_addr) ? false : true;
  } else if (family == AF_INET6) {
/* 中文逐项-变量 V3480：local_addr
 * local_addr（struct sockaddr_in6；函数 matchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 local_addr = (struct sockaddr_in6*)(local_if.ifa_addr)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct sockaddr_in6* local_addr = (struct sockaddr_in6*)(local_if.ifa_addr);
/* 中文逐项-变量 V3481：mask
 * mask（struct sockaddr_in6；函数 matchSubnet 内）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 * 初值/来源 mask = (struct sockaddr_in6*)(local_if.ifa_netmask)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct sockaddr_in6* mask = (struct sockaddr_in6*)(local_if.ifa_netmask);
    struct sockaddr_in6& remote_addr = remote->sin6;
    struct in6_addr& local_in6 = local_addr->sin6_addr;
    struct in6_addr& mask_in6 = mask->sin6_addr;
    struct in6_addr& remote_in6 = remote_addr.sin6_addr;
/* 中文逐项-变量 V3482：same
 * same（bool；函数 matchSubnet 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 same = true；布尔初态为true，后续条件决定是否切换。
 */
    bool same = true;
/* 中文逐项-变量 V3483：len
 * len（int；函数 matchSubnet 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 len = 16；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int len = 16;  //IPv6 address is 16 unsigned char
    for (/* 中文逐项-变量 V3484：c
 * c（int；函数 matchSubnet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 c = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：c < len；循环体与增量表达式决定每轮如何推进。
 */int c = 0; c < len; c++) {  //Network byte order is big-endian
/* 中文逐项-变量 V3485：c1
 * c1（char；函数 matchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 c1 = local_in6.s6_addr[c] & mask_in6.s6_addr[c]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      char c1 = local_in6.s6_addr[c] & mask_in6.s6_addr[c];
/* 中文逐项-变量 V3486：c2
 * c2（char；函数 matchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 c2 = remote_in6.s6_addr[c] & mask_in6.s6_addr[c]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      char c2 = remote_in6.s6_addr[c] & mask_in6.s6_addr[c];
      if (c1 ^ c2) {
        same = false;
        break;
      }
    }
    // At last, we need to compare scope id
    // Two Link-type addresses can have the same subnet address even though they are not in the same scope
    // For Global type, this field is 0, so a comparison wouldn't matter
    same &= (local_addr->sin6_scope_id == remote_addr.sin6_scope_id);
    return same;
  } else {
    WARN("Net : Unsupported address family type");
    return false;
  }
}

/* 中文逐项-函数 F0976：ncclFindInterfaceMatchSubnet
 * 用途与位置：按远端地址找同子网本地接口，帮助 bootstrap 选择能够到达会合端的地址。
 * 参数 ifNames（char* ifNames）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 localAddrs（union ncclSocketAddress* localAddrs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 remoteAddr（union ncclSocketAddress* remoteAddr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ifNameMaxSize（int ifNameMaxSize）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 参数 maxIfs（int maxIfs）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapNetInit；src/misc/socket.cc::ncclFindInterfaces。
 * 本体直接出现的调用/包装：freeifaddrs；getifaddrs；matchSubnet；memcpy；ncclSocketToString；strncpy。
 * 需要对照的边界/推进条件：interface && !found；interface->ifa_addr == NULL；family != AF_INET && family !=
 * AF_INET6；!matchSubnet(*interface, remoteAddr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return found;。CHECK类宏还可能提前返回/跳转。
 */
int ncclFindInterfaceMatchSubnet(char* ifNames, union ncclSocketAddress* localAddrs, union ncclSocketAddress* remoteAddr, int ifNameMaxSize, int maxIfs) {
#ifdef ENABLE_TRACE
/* 中文逐项-变量 V3487：line
 * line（char；函数 ncclFindInterfaceMatchSubnet 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：TRACE(NCCL_INIT|NCCL_NET,"NET : Found interface %s:%s in the same subnet as remote address %s",
 * interface->ifa_name, ncclSocketToString(localAddrs+found, line), ncclSocketToString(remoteAddr,
 * line_a));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[SOCKET_NAME_MAXLEN+1];
#endif
/* 中文逐项-变量 V3488：line_a
 * line_a（char；函数 ncclFindInterfaceMatchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：TRACE(NCCL_INIT|NCCL_NET,"NET : Found interface %s:%s in the same subnet as remote address %s",
 * interface->ifa_name, ncclSocketToString(localAddrs+found, line), ncclSocketToString(remoteAddr,
 * line_a));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line_a[SOCKET_NAME_MAXLEN+1];
/* 中文逐项-变量 V3489：found
 * found（int；函数 ncclFindInterfaceMatchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 found = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int found = 0;
/* 中文逐项-变量 V3490：interfaces、interface
 * interfaces（struct ifaddrs；函数 ncclFindInterfaceMatchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：getifaddrs(&interfaces);。若由输出参数填入，必须先检查填充调用成功再读值。
 * interface（struct ifaddrs；函数 ncclFindInterfaceMatchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (interface = interfaces; interface && !found; interface = interface->ifa_next)
 * {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ifaddrs *interfaces, *interface;
  getifaddrs(&interfaces);
  for (interface = interfaces; interface && !found; interface = interface->ifa_next) {
    if (interface->ifa_addr == NULL) continue;

    /* We only support IPv4 & IPv6 */
/* 中文逐项-变量 V3491：family
 * family（int；函数 ncclFindInterfaceMatchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 family = interface->ifa_addr->sa_family；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    int family = interface->ifa_addr->sa_family;
    if (family != AF_INET && family != AF_INET6)
      continue;

    // check against user specified interfaces
    if (!matchSubnet(*interface, remoteAddr)) {
      continue;
    }

    // Store the local IP address
/* 中文逐项-变量 V3492：salen
 * salen（int；函数 ncclFindInterfaceMatchSubnet 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct
 * sockaddr_in6)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    memcpy(localAddrs+found, interface->ifa_addr, salen);

    // Store the interface name
    strncpy(ifNames+found*ifNameMaxSize, interface->ifa_name, ifNameMaxSize);

    TRACE(NCCL_INIT|NCCL_NET,"NET : Found interface %s:%s in the same subnet as remote address %s", interface->ifa_name, ncclSocketToString(localAddrs+found, line), ncclSocketToString(remoteAddr, line_a));
    found++;
    if (found == maxIfs) break;
  }

  if (found == 0) {
    WARN("Net : No interface found in the same subnet as remote address %s", ncclSocketToString(remoteAddr, line_a));
  }
  freeifaddrs(interfaces);
  return found;
}

/* 中文逐项-函数 F0977：ncclGetSocketAddrFromString
 * 用途与位置：解析用户提供的 IP/主机及端口，构造统一 IPv4/IPv6 地址对象。
 * 参数 ua（union ncclSocketAddress* ua）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ip_port_pair（const char* ip_port_pair）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapNetInit；src/bootstrap.cc::bootstrapGetUniqueId；src/misc/socket.cc::ncclFindInterfaces。
 * 本体直接出现的调用/包装：atoi；freeaddrinfo；gai_strerror；getaddrinfo；htons；if_nametoindex；inet_pton；memcpy；memset；parseStringList；strlen；strncpy。
 * 需要对照的边界/推进条件：!(ip_port_pair && strlen(ip_port_pair) > 1；!ipv6；parseStringList(ip_port_pair, &ni, 1) != 1；(rv =
 * getaddrinfo(ni.prefix, NULL, &hints, &p)) != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGetSocketAddrFromString(union ncclSocketAddress* ua, const char* ip_port_pair) {
  if (!(ip_port_pair && strlen(ip_port_pair) > 1)) {
    WARN("Net : string is null");
    return ncclInvalidArgument;
  }

/* 中文逐项-变量 V3493：ipv6
 * ipv6（bool；函数 ncclGetSocketAddrFromString 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 ipv6 = ip_port_pair[0] == '['；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  bool ipv6 = ip_port_pair[0] == '[';
  /* Construct the sockaddress structure */
  if (!ipv6) {
/* 中文逐项-变量 V3494：ni
 * ni（struct netIf；函数 ncclGetSocketAddrFromString 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (parseStringList(ip_port_pair, &ni, 1) != 1) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct netIf ni;
    // parse <ip_or_hostname>:<port> string, expect one pair
    if (parseStringList(ip_port_pair, &ni, 1) != 1) {
      WARN("Net : No valid <IPv4_or_hostname>:<port> pair found");
      return ncclInvalidArgument;
    }

/* 中文逐项-变量 V3495：hints、p
 * hints（struct addrinfo；函数 ncclGetSocketAddrFromString 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(&hints, 0, sizeof(hints));。若由输出参数填入，必须先检查填充调用成功再读值。
 * p（struct addrinfo；函数 ncclGetSocketAddrFromString 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 此处未显式给初值；后续首个文本使用点：if ( (rv = getaddrinfo(ni.prefix, NULL, &hints, &p)) != 0) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct addrinfo hints, *p;
/* 中文逐项-变量 V3496：rv
 * rv（int；函数 ncclGetSocketAddrFromString 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：if ( (rv = getaddrinfo(ni.prefix, NULL, &hints, &p)) != 0) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int rv;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ( (rv = getaddrinfo(ni.prefix, NULL, &hints, &p)) != 0) {
      WARN("Net : error encountered when getting address info : %s", gai_strerror(rv));
      return ncclInvalidArgument;
    }

    // use the first
    if (p->ai_family == AF_INET) {
      struct sockaddr_in& sin = ua->sin;
      memcpy(&sin, p->ai_addr, sizeof(struct sockaddr_in));
      sin.sin_family = AF_INET;                        // IPv4
      //inet_pton(AF_INET, ni.prefix, &(sin.sin_addr));  // IP address
      sin.sin_port = htons(ni.port);                   // port
    } else if (p->ai_family == AF_INET6) {
      struct sockaddr_in6& sin6 = ua->sin6;
      memcpy(&sin6, p->ai_addr, sizeof(struct sockaddr_in6));
      sin6.sin6_family = AF_INET6;                     // IPv6
      sin6.sin6_port = htons(ni.port);                 // port
      sin6.sin6_flowinfo = 0;                          // needed by IPv6, but possibly obsolete
      sin6.sin6_scope_id = 0;                          // should be global scope, set to 0
    } else {
      WARN("Net : unsupported IP family");
      return ncclInvalidArgument;
    }

    freeaddrinfo(p); // all done with this structure

  } else {
/* 中文逐项-变量 V3497：i、j、len
 * i（int；函数 ncclGetSocketAddrFromString 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：for (i = 1; i < len; i++) {。若由输出参数填入，必须先检查填充调用成功再读值。
 * j（int；函数 ncclGetSocketAddrFromString 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 j = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 * len（int；函数 ncclGetSocketAddrFromString 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 len = strlen(ip_port_pair)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int i, j = -1, len = strlen(ip_port_pair);
    for (i = 1; i < len; i++) {
      if (ip_port_pair[i] == '%') j = i;
      if (ip_port_pair[i] == ']') break;
    }
    if (i == len) {
      WARN("Net : No valid [IPv6]:port pair found");
      return ncclInvalidArgument;
    }
/* 中文逐项-变量 V3498：global_scope
 * global_scope（bool；函数 ncclGetSocketAddrFromString 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 global_scope = (j == -1 ? true : false)；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    bool global_scope = (j == -1 ? true : false);     // If no % found, global scope; otherwise, link scope

/* 中文逐项-变量 V3499：ip_str、port_str、if_name
 * ip_str（char；函数 ncclGetSocketAddrFromString 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(ip_str, '\0', sizeof(ip_str));。若由输出参数填入，必须先检查填充调用成功再读值。
 * port_str（char；函数 ncclGetSocketAddrFromString 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(port_str, '\0', sizeof(port_str));。若由输出参数填入，必须先检查填充调用成功再读值。
 * if_name（char；函数 ncclGetSocketAddrFromString 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(if_name, '\0', sizeof(if_name));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char ip_str[NI_MAXHOST], port_str[NI_MAXSERV], if_name[IFNAMSIZ];
    memset(ip_str, '\0', sizeof(ip_str));
    memset(port_str, '\0', sizeof(port_str));
    memset(if_name, '\0', sizeof(if_name));
    strncpy(ip_str, ip_port_pair+1, global_scope ? i-1 : j-1);
    strncpy(port_str, ip_port_pair+i+2, len-i-1);
/* 中文逐项-变量 V3500：port
 * port（int；函数 ncclGetSocketAddrFromString 内）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 初值/来源 port = atoi(port_str)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int port = atoi(port_str);
    if (!global_scope) strncpy(if_name, ip_port_pair+j+1, i-j-1); // If not global scope, we need the intf name

    struct sockaddr_in6& sin6 = ua->sin6;
    sin6.sin6_family = AF_INET6;                       // IPv6
    inet_pton(AF_INET6, ip_str, &(sin6.sin6_addr));    // IP address
    sin6.sin6_port = htons(port);                      // port
    sin6.sin6_flowinfo = 0;                            // needed by IPv6, but possibly obsolete
    sin6.sin6_scope_id = global_scope ? 0 : if_nametoindex(if_name);  // 0 if global scope; intf index if link scope
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0978：ncclFindInterfaces
 * 用途与位置：按显式接口配置或默认优先级选择可用接口，过滤不合适的 loopback/docker 等候选。
 * 参数 ifNames（char* ifNames）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ifAddrs（union ncclSocketAddress *ifAddrs）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 ifNameMaxSize（int ifNameMaxSize）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 参数 maxIfs（int maxIfs）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapNetInit；src/transport/net_ib.cc::ncclIbInit；src/transport/net_socket.cc::ncclSocketInit。
 * 本体直接出现的调用/包装：envSocketFamily；findInterfaces；getenv；ncclFindInterfaceMatchSubnet；ncclGetSocketAddrFromString；strlen。
 * 需要对照的边界/推进条件：env && strlen(env) > 1；shownIfName++ == 0；nIfs == 0；commId && strlen(commId) > 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return nIfs;。CHECK类宏还可能提前返回/跳转。
 */
int ncclFindInterfaces(char* ifNames, union ncclSocketAddress *ifAddrs, int ifNameMaxSize, int maxIfs) {
/* 中文逐项-变量 V3501：shownIfName
 * shownIfName（int；函数 ncclFindInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shownIfName = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  static int shownIfName = 0;
/* 中文逐项-变量 V3502：nIfs
 * nIfs（int；函数 ncclFindInterfaces 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nIfs = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int nIfs = 0;
  // Allow user to force the INET socket family selection
/* 中文逐项-变量 V3503：sock_family
 * sock_family（int；函数 ncclFindInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sock_family = envSocketFamily()；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int sock_family = envSocketFamily();
  // User specified interface
/* 中文逐项-变量 V3504：env
 * env（char；函数 ncclFindInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 env = getenv("NCCL_SOCKET_IFNAME")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* env = getenv("NCCL_SOCKET_IFNAME");
  if (env && strlen(env) > 1) {
    INFO(NCCL_ENV, "NCCL_SOCKET_IFNAME set by environment to %s", env);
    // Specified by user : find or fail
    if (shownIfName++ == 0) INFO(NCCL_NET, "NCCL_SOCKET_IFNAME set to %s", env);
    nIfs = findInterfaces(env, ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);
  } else {
    // Try to automatically pick the right one
    // Start with IB
    nIfs = findInterfaces("ib", ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);
    // else see if we can get some hint from COMM ID
    if (nIfs == 0) {
/* 中文逐项-变量 V3505：commId
 * commId（char；函数 ncclFindInterfaces 内）：应用分发的引导会合标识，同一通信组所有rank需要使用同一份。
 * 初值/来源 commId = getenv("NCCL_COMM_ID")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      char* commId = getenv("NCCL_COMM_ID");
      if (commId && strlen(commId) > 1) {
	INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", commId);
	// Try to find interface that is in the same subnet as the IP in comm id
/* 中文逐项-变量 V3506：idAddr
 * idAddr（union ncclSocketAddress；函数 ncclFindInterfaces 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclGetSocketAddrFromString(&idAddr, commId);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        union ncclSocketAddress idAddr;
        ncclGetSocketAddrFromString(&idAddr, commId);
        nIfs = ncclFindInterfaceMatchSubnet(ifNames, ifAddrs, &idAddr, ifNameMaxSize, maxIfs);
      }
    }
    // Then look for anything else (but not docker or lo)
    if (nIfs == 0) nIfs = findInterfaces("^docker,lo", ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);
    // Finally look for docker, then lo.
    if (nIfs == 0) nIfs = findInterfaces("docker", ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);
    if (nIfs == 0) nIfs = findInterfaces("lo", ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);
  }
  return nIfs;
}

/* 中文逐项-函数 F0979：ncclSocketListen
 * 用途与位置：按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapCreateRoot；src/bootstrap.cc::bootstrapInit；src/transport/net_ib.cc::ncclIbListen；src/transport/net_socket.cc::ncclSocketListen。
 * 本体直接出现的调用/包装：EQCHECK；SYSCHECK；bind；fcntl；getsockname；listen；ncclSocketToString；setsockopt；socket；socketToPort；strerror。
 * 需要对照的边界/推进条件：fd == -1；socketToPort(&sock->addr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketListen(struct ncclSocket* sock) {
  /* IPv4/IPv6 support */
/* 中文逐项-变量 V3507：family
 * family（int；函数 ncclSocketListen 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 family = sock->addr.sa.sa_family；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int family = sock->addr.sa.sa_family;
/* 中文逐项-变量 V3508：salen
 * salen（int；函数 ncclSocketListen 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct
 * sockaddr_in6)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
/* 中文逐项-变量 V3509：flags
 * flags（int；函数 ncclSocketListen 内）：按位组合的当前线程角色/直接访问/共享偏移/中止等状态，按位测试而非单一枚举。
 * 此处未显式给初值；后续首个文本使用点：EQCHECK(flags = fcntl(fd, F_GETFL), -1);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int flags;

  /* Create socket and bind it to a port */
/* 中文逐项-变量 V3510：fd
 * fd（int；函数 ncclSocketListen 内）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 初值/来源 fd = socket(family, SOCK_STREAM, 0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int fd = socket(family, SOCK_STREAM, 0);
  if (fd == -1) {
    WARN("Net : Socket creation failed : %s", strerror(errno));
    return ncclSystemError;
  }

  if (socketToPort(&sock->addr)) {
    // Port is forced by env. Make sure we get the port.
/* 中文逐项-变量 V3511：opt
 * opt（int；函数 ncclSocketListen 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 opt = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    int opt = 1;
#if defined(SO_REUSEPORT)
    SYSCHECK(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt");
#else
    SYSCHECK(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), "setsockopt");
#endif
  }

  /* make all new sockets non-blocking */
  EQCHECK(flags = fcntl(fd, F_GETFL), -1);
  SYSCHECK(fcntl(fd, F_SETFL, flags | O_NONBLOCK), "fcntl");

  // addr port should be 0 (Any port)
  SYSCHECK(bind(fd, &sock->addr.sa, salen), "bind");

  /* Get the assigned Port */
/* 中文逐项-变量 V3512：size
 * size（socklen_t；函数 ncclSocketListen 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 size = salen；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  socklen_t size = salen;
  SYSCHECK(getsockname(fd, &sock->addr.sa, &size), "getsockname");

#ifdef ENABLE_TRACE
/* 中文逐项-变量 V3513：line
 * line（char；函数 ncclSocketListen 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：TRACE(NCCL_INIT|NCCL_NET,"Listening on socket %s", ncclSocketToString(&sock->addr,
 * line));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[SOCKET_NAME_MAXLEN+1];
  TRACE(NCCL_INIT|NCCL_NET,"Listening on socket %s", ncclSocketToString(&sock->addr, line));
#endif

  /* Put the socket in listen mode
   * NB: The backlog will be silently truncated to the value in /proc/sys/net/core/somaxconn
   */
  SYSCHECK(listen(fd, 16384), "listen");
  sock->fd = fd;
  return ncclSuccess;
}

/* 中文逐项-函数 F0980：getFdState
 * 用途与位置：检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。
 * 参数 fd（int fd）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 参数 state（enum ncclSocketState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclGetSocketState；src/misc/socket.cc::ncclSocketConnect。
 * 本体直接出现的调用/包装：EQCHECK；SYSCHECK；getsockopt；memset；poll。
 * 需要对照的边界/推进条件：ret == 0；ret == EINPROGRESS；ret == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t getFdState(int fd, enum ncclSocketState* state) {
/* 中文逐项-变量 V3514：pfd
 * pfd（struct pollfd；函数 getFdState 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(&pfd, 0, sizeof(struct pollfd));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct pollfd pfd;
/* 中文逐项-变量 V3515：timeout、ret
 * timeout（int；函数 getFdState 内）：RC重试、接收未就绪重试与超时配置；属于可靠传输，不是用户API超时。
 * 初值/来源 timeout = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * ret（int；函数 getFdState 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：SYSCHECK(ret = poll(&pfd, 1, timeout), "poll");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int timeout = 1, ret;
/* 中文逐项-变量 V3516：rlen
 * rlen（socklen_t；函数 getFdState 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 rlen = sizeof(int)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
    socklen_t rlen = sizeof(int);

    memset(&pfd, 0, sizeof(struct pollfd));
    pfd.fd = fd;
    pfd.events = POLLOUT;
    SYSCHECK(ret = poll(&pfd, 1, timeout), "poll");
    if (ret == 0) {
      ret = EINPROGRESS;
    } else {
      /* check socket status */
      EQCHECK(ret == 1 && (pfd.revents & POLLOUT), 0);
      SYSCHECK(getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&ret, &rlen), "getsockopt");
    }

    if (ret == EINPROGRESS)
      *state = ncclSocketConnecting;
    else if (ret == 0)
      *state = ncclSocketConnected;
    else
      *state = ncclSocketError;
    return ncclSuccess;
}

/* 中文逐项-函数 F0981：ncclGetSocketState
 * 用途与位置：检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 state（enum ncclSocketState* state）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbConnect；src/transport/net_socket.cc::ncclSocketConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；getFdState。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGetSocketState(struct ncclSocket* sock, enum ncclSocketState* state) {
    NCCLCHECK(getFdState(sock->fd, state));
    sock->state = *state;
    return ncclSuccess;
}

/* 中文逐项-函数 F0982：ncclSocketConnect
 * 用途与位置：按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRoot；src/bootstrap.cc::bootstrapInit；src/bootstrap.cc::bootstrapSend；src/proxy.cc::ncclProxyConnect；src/proxy.cc::ncclProxyDestroy；src/transport/net_ib.cc::ncclIbConnect；src/transport/net_socket.cc::ncclSocketConnect。
 * 本体直接出现的调用/包装：EQCHECK；NCCLCHECK；NEQCHECK；SYSCHECK；connect；fcntl；getFdState；ncclSocketToString；setsockopt；socket；strerror；usleep。
 * 需要对照的边界/推进条件：family != AF_INET && family != AF_INET6；fd == -1；errno == EAGAIN || (errno == ECONNREFUSED &&
 * ++refused_retries < RETRY_REFUSED_TIMES) || (errno == ETIMEDOUT && ++timedout_retries < RETRY_TIMEDO
 * …（完整表达式见紧邻源码）；refused_retries % 1000 == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketConnect(struct ncclSocket* sock) {
/* 中文逐项-变量 V3517：line
 * line（char；函数 ncclSocketConnect 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：ncclSocketToString(&sock->addr, line), family, AF_INET, AF_INET6);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[SOCKET_NAME_MAXLEN+1];
  /* IPv4/IPv6 support */
/* 中文逐项-变量 V3518：family
 * family（int；函数 ncclSocketConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 family = sock->addr.sa.sa_family；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int family = sock->addr.sa.sa_family;
  if (family != AF_INET && family != AF_INET6) {
    WARN("Net : connecting to address %s with family %d is neither AF_INET(%d) nor AF_INET6(%d)",
         ncclSocketToString(&sock->addr, line), family, AF_INET, AF_INET6);
    return ncclInternalError;
  }
/* 中文逐项-变量 V3519：salen
 * salen（int；函数 ncclSocketConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct
 * sockaddr_in6)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
/* 中文逐项-变量 V3520：flags
 * flags（int；函数 ncclSocketConnect 内）：按位组合的当前线程角色/直接访问/共享偏移/中止等状态，按位测试而非单一枚举。
 * 此处未显式给初值；后续首个文本使用点：EQCHECK(flags = fcntl(fd, F_GETFL), -1);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int flags;

  /* Connect to a hostname / port */
/* 中文逐项-变量 V3521：fd
 * fd（int；函数 ncclSocketConnect 内）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 初值/来源 fd = socket(family, SOCK_STREAM, 0)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int fd = socket(family, SOCK_STREAM, 0);
  if (fd == -1) {
    WARN("Net : Socket creation failed : %s", strerror(errno));
    return ncclSystemError;
  }

/* 中文逐项-变量 V3522：one
 * one（int；函数 ncclSocketConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 one = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const int one = 1;
  SYSCHECK(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int)), "setsockopt");

  /* support non-blocking socket; by default, the socket is non-blocking */
  EQCHECK(flags = fcntl(fd, F_GETFL), -1);
  SYSCHECK(fcntl(fd, F_SETFL, flags | O_NONBLOCK), "fcntl");

  /*  const int bufsize = 128*1024;
    SYSCHECK(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, sizeof(int)), "setsockopt");
    SYSCHECK(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(int)), "setsockopt");*/

  TRACE(NCCL_INIT|NCCL_NET,"Connecting to socket %s", ncclSocketToString(&sock->addr, line));

/* 中文逐项-变量 V3523：ret
 * ret（int；函数 ncclSocketConnect 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：ret = connect(fd, &sock->addr.sa, salen);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ret;
/* 中文逐项-变量 V3524：timedout_retries
 * timedout_retries（int；函数 ncclSocketConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 timedout_retries = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int timedout_retries = 0;
/* 中文逐项-变量 V3525：refused_retries
 * refused_retries（int；函数 ncclSocketConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 refused_retries = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int refused_retries = 0;
retry:
  /* async connect; abort when error happens and abortFlag is present. */
  ret = connect(fd, &sock->addr.sa, salen);

  if (errno == EAGAIN || (errno == ECONNREFUSED && ++refused_retries < RETRY_REFUSED_TIMES) ||
    (errno == ETIMEDOUT && ++timedout_retries < RETRY_TIMEDOUT_TIMES)) {
    if (refused_retries % 1000 == 0) INFO(NCCL_ALL, "Call to connect returned %s, retrying", strerror(errno));
    usleep(SLEEP_INT);
    goto retry;
  } else if (errno == EINPROGRESS && !sock->asyncFlag) {
/* 中文逐项-变量 V3526：state
 * state（enum ncclSocketState；函数 ncclSocketConnect 内）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(getFdState(fd, &state));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    enum ncclSocketState state;
    do {
      if (sock->abortFlag) NEQCHECK(*sock->abortFlag, 0);
      NCCLCHECK(getFdState(fd, &state));
    } while (state == ncclSocketConnecting);
    EQCHECK(state, ncclSocketError);
    ret = 0;
  }

  if (ret == 0 || (errno == EINPROGRESS && sock->asyncFlag)) {
    sock->fd = fd;
    return ncclSuccess;
  }

  WARN("Net : Connect to %s failed : %s", ncclSocketToString(&sock->addr, line), strerror(errno));
  return ncclSystemError;
}

/* 中文逐项-函数 F0983：ncclSocketAccept
 * 用途与位置：按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 listenSocket（struct ncclSocket* listenSocket）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapRoot；src/bootstrap.cc::bootstrapInit；src/bootstrap.cc::bootstrapRecv；src/proxy.cc::ncclProxyService；src/transport/net_ib.cc::ncclIbAccept；src/transport/net_socket.cc::ncclSocketAccept。
 * 本体直接出现的调用/包装：EQCHECK；NEQCHECK；accept。
 * 需要对照的边界/推进条件：listenSocket->abortFlag；!listenSocket->asyncFlag；tmpFd == -1 && errno != EAGAIN && errno !=
 * EWOULDBLOCK。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketAccept(struct ncclSocket* sock, struct ncclSocket* listenSocket) {
/* 中文逐项-变量 V3527：socklen
 * socklen（socklen_t；函数 ncclSocketAccept 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 socklen = sizeof(union ncclSocketAddress)；通过类型字节宽度换算元素、物理字节或对齐粒度，单位按完整乘除表达式确认。
 */
  socklen_t socklen = sizeof(union ncclSocketAddress);
/* 中文逐项-变量 V3528：tmpFd
 * tmpFd（int；函数 ncclSocketAccept 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 tmpFd = sock->fd = -1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int tmpFd = sock->fd = -1;

  do {
    if (listenSocket->abortFlag) NEQCHECK(*listenSocket->abortFlag, 0);
    tmpFd = accept(listenSocket->fd, &sock->addr.sa, &socklen);
  } while ((errno == EAGAIN || errno == EWOULDBLOCK) && tmpFd == -1 && !listenSocket->asyncFlag);

  if (!listenSocket->asyncFlag) {
    EQCHECK(tmpFd, -1);
  } else if (tmpFd == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
    return ncclSystemError;
  }

  sock->fd = tmpFd;
  return ncclSuccess;
}

/* 中文逐项-函数 F0984：ncclSocketInit
 * 用途与位置：本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 addr（union ncclSocketAddress* addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 参数 abortFlag（volatile uint32_t* abortFlag）：主机/设备可观察的异常中止共享标志，用于打破协议或 socket 等待。
 * 参数 asyncFlag（int asyncFlag）：socket是否使用异步连接/进度推进方式，决定未就绪时返回还是持续等待。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbConnect；src/transport/net_socket.cc::ncclSocketConnect；src/transport/net_socket.cc::ncclSocketAccept。
 * 本体直接出现的调用/包装：memcpy；memset。
 * 需要对照的边界/推进条件：sock == NULL；addr。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketInit(struct ncclSocket* sock, union ncclSocketAddress* addr, volatile uint32_t* abortFlag, int asyncFlag) {
  if (sock == NULL)
    return ncclSuccess;

  sock->fd = -1;
  if (addr) {
    memcpy(&sock->addr, addr, sizeof(union ncclSocketAddress));
  } else {
    memset(&sock->addr, 0, sizeof(union ncclSocketAddress));
  }
  sock->abortFlag = abortFlag;
  sock->asyncFlag = asyncFlag;
  sock->state = ncclSocketStateNum;
  return ncclSuccess;
}

/* 中文精读：TCP 是字节流，必须保存部分完成的 offset
 * 一次 send/recv 可能只推进一部分数据，非阻塞还可能返回 EAGAIN/EWOULDBLOCK。
 * 这里按 offset 累积进度，block 参数区分尽力推进与持续等待，并检查连接关闭/中止状态。
 * bootstrap 控制消息和 Socket 数据后端都复用这些基础函数，但二者上层消息语义不同。
 * 读网络日志时，应先分清是控制 socket 建链失败，还是数据 socket 在传输中失败。
 */
/* 中文逐项-函数 F0985：ncclSocketProgressOpt
 * 用途与位置：按保存 offset 尽力推进 TCP send/recv，可选持续等待或检测关闭，并响应 abort。
 * 参数 op（int op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 offset（int* offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 参数 block（int block）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 closed（int* closed）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketProgress；src/misc/socket.cc::ncclSocketTryRecv。
 * 本体直接出现的调用/包装：ncclSocketToString；recv；send；strerror。
 * 需要对照的边界/推进条件：op == NCCL_SOCKET_RECV；op == NCCL_SOCKET_SEND；op == NCCL_SOCKET_RECV && bytes == 0；bytes ==
 * -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclSystemError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclSocketProgressOpt(int op, struct ncclSocket* sock, void* ptr, int size, int* offset, int block, int* closed) {
/* 中文逐项-变量 V3529：bytes
 * bytes（int；函数 ncclSocketProgressOpt 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bytes = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int bytes = 0;
  *closed = 0;
/* 中文逐项-变量 V3530：data
 * data（char；函数 ncclSocketProgressOpt 内）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 初值/来源 data = (char*)ptr；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  char* data = (char*)ptr;
/* 中文逐项-变量 V3531：line
 * line（char；函数 ncclSocketProgressOpt 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：WARN("Net : Call to recv from %s failed : %s", ncclSocketToString(&sock->addr, line),
 * strerror(errno));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char line[SOCKET_NAME_MAXLEN+1];
  do {
    if (op == NCCL_SOCKET_RECV) bytes = recv(sock->fd, data+(*offset), size-(*offset), block ? 0 : MSG_DONTWAIT);
    if (op == NCCL_SOCKET_SEND) bytes = send(sock->fd, data+(*offset), size-(*offset), block ? 0 : MSG_DONTWAIT);
    if (op == NCCL_SOCKET_RECV && bytes == 0) {
      *closed = 1;
      return ncclSuccess;
    }
    if (bytes == -1) {
      if (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN) {
        WARN("Net : Call to recv from %s failed : %s", ncclSocketToString(&sock->addr, line), strerror(errno));
        return ncclSystemError;
      } else {
        bytes = 0;
      }
    }
    (*offset) += bytes;
    if (sock->abortFlag && *sock->abortFlag != 0) {
      INFO(NCCL_NET, "Socket progress: abort called");
      return ncclSystemError;
    }
  } while (bytes > 0 && (*offset) < size);
  return ncclSuccess;
}

/* 中文逐项-函数 F0986：ncclSocketProgress
 * 用途与位置：调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。
 * 参数 op（int op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 offset（int* offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketWait；src/transport/net_ib.cc::ncclIbConnect；src/transport/net_ib.cc::ncclIbAccept；src/transport/net_ib.cc::ncclSendCheck；src/transport/net_ib.cc::ncclRecvCheck；src/transport/net_socket.cc::persistentSocketThread；src/transport/net_socket.cc::ncclSocketConnect；其余见调用清单。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketProgressOpt；ncclSocketToString。
 * 需要对照的边界/推进条件：closed。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketProgress(int op, struct ncclSocket* sock, void* ptr, int size, int* offset) {
/* 中文逐项-变量 V3532：closed
 * closed（int；函数 ncclSocketProgress 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketProgressOpt(op, sock, ptr, size, offset, 0,
 * &closed));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int closed;
  NCCLCHECK(ncclSocketProgressOpt(op, sock, ptr, size, offset, 0, &closed));
  if (closed) {
/* 中文逐项-变量 V3533：line
 * line（char；函数 ncclSocketProgress 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：WARN("Net : Connection closed by remote peer %s", ncclSocketToString(&sock->addr, line,
 * 0));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char line[SOCKET_NAME_MAXLEN+1];
    WARN("Net : Connection closed by remote peer %s", ncclSocketToString(&sock->addr, line, 0));
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0987：ncclSocketWait
 * 用途与位置：重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。
 * 参数 op（int op）：用户归约操作句柄或代理/系统调用方向枚举；查看类型和所属函数辨认。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 offset（int* offset）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/socket.cc::ncclSocketSend；src/misc/socket.cc::ncclSocketRecv；src/transport/net_ib.cc::ncclSendCheck；src/transport/net_ib.cc::ncclRecvCheck；src/transport/net_socket.cc::ncclSocketTest。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketProgress。
 * 需要对照的边界/推进条件：*offset < size。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketWait(int op, struct ncclSocket* sock, void* ptr, int size, int* offset) {
  while (*offset < size)
    NCCLCHECK(ncclSocketProgress(op, sock, ptr, size, offset));
  return ncclSuccess;
}

/* 中文逐项-函数 F0988：ncclSocketSend
 * 用途与位置：从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapNetSend；src/proxy.cc::ncclProxyConnect；src/proxy.cc::ncclProxyCall；src/proxy.cc::proxyConnInit；src/proxy.cc::proxyProgressAsync；src/proxy.cc::ncclProxyDestroy；src/transport/net_ib.cc::ncclSendCheck。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketWait。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketSend(struct ncclSocket* sock, void* ptr, int size) {
/* 中文逐项-变量 V3534：offset
 * offset（int；函数 ncclSocketSend 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int offset = 0;
  NCCLCHECK(ncclSocketWait(NCCL_SOCKET_SEND, sock, ptr, size, &offset));
  return ncclSuccess;
}

/* 中文逐项-函数 F0989：ncclSocketRecv
 * 用途与位置：从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/bootstrap.cc::bootstrapNetRecv；src/proxy.cc::ncclProxyConnect；src/proxy.cc::ncclProxyCall；src/proxy.cc::proxyConnInit；src/proxy.cc::proxyConnSharedInit；src/proxy.cc::proxyConnSetupConnect。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketWait。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketRecv(struct ncclSocket* sock, void* ptr, int size) {
/* 中文逐项-变量 V3535：offset
 * offset（int；函数 ncclSocketRecv 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int offset = 0;
  NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, sock, ptr, size, &offset));
  return ncclSuccess;
}

// Receive or detect connection closed
/* 中文逐项-函数 F0990：ncclSocketTryRecv
 * 用途与位置：尝试接收并可区分对端关闭，供代理服务循环避免无限阻塞在已断开连接。
 * 参数 sock（struct ncclSocket* sock）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 * 参数 ptr（void* ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 closed（int* closed）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyService。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketProgressOpt。
 * 需要对照的边界/推进条件：offset < size；*closed。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSocketTryRecv(struct ncclSocket* sock, void* ptr, int size, int* closed) {
/* 中文逐项-变量 V3536：offset
 * offset（int；函数 ncclSocketTryRecv 内）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 * 初值/来源 offset = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int offset = 0;
  *closed = 0;
  while (offset < size) {
    NCCLCHECK(ncclSocketProgressOpt(NCCL_SOCKET_RECV, sock, ptr, size, &offset, 0, closed));
    if (*closed) return ncclSuccess;
  }
  return ncclSuccess;
}
