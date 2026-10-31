# src/misc/socket.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0971"></a>

## F0971：ncclSocketToString

用途：把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(buf == NULL &#124;&#124; addr == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>struct sockaddr *saddr = &addr-&gt;sa;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(saddr-&gt;sa_family != AF_INET && saddr-&gt;sa_family != AF_INET6)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>buf[0]='\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 函数返回 | <code>return buf;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 准备局部状态 | <code>char host[NI_MAXHOST], service[NI_MAXSERV];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 准备局部状态 | <code>int flag = NI_NUMERICSERV &#124; (numericHostForm ? NI_NUMERICHOST : 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>(void) getnameinfo(saddr, sizeof(union ncclSocketAddress), host, NI_MAXHOST, service, NI_MAXSERV, flag);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>sprintf(buf, "%s&lt;%s&gt;", host, service);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 函数返回 | <code>return buf;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (buf == NULL || addr == NULL) return NULL;
  struct sockaddr *saddr = &addr->sa;
  if (saddr->sa_family != AF_INET && saddr->sa_family != AF_INET6) { buf[0]='\0'; return buf; }
  char host[NI_MAXHOST], service[NI_MAXSERV];
  /* NI_NUMERICHOST: If set, then the numeric form of the hostname is returned.
   * (When not set, this will still happen in case the node's name cannot be determined.)
   */
  int flag = NI_NUMERICSERV | (numericHostForm ? NI_NUMERICHOST : 0);
  (void) getnameinfo(saddr, sizeof(union ncclSocketAddress), host, NI_MAXHOST, service, NI_MAXSERV, flag);
  sprintf(buf, "%s<%s>", host, service);
  return buf;
}
```

</details>

<a id="f0972"></a>

## F0972：socketToPort

用途：从通用 socket 地址的 IPv4/IPv6 具体分支取出端口号。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct sockaddr *saddr = &addr-&gt;sa;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 函数返回 | <code>return ntohs(saddr-&gt;sa_family == AF_INET ? addr-&gt;sin.sin_port : addr-&gt;sin6.sin6_port);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct sockaddr *saddr = &addr->sa;
  return ntohs(saddr->sa_family == AF_INET ? addr->sin.sin_port : addr->sin6.sin6_port);
}
```

</details>

<a id="f0973"></a>

## F0973：envSocketFamily

用途：解析环境选择的 IPv4/IPv6 地址族，影响接口筛选与连接建立。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int family = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>char* env = getenv("NCCL_SOCKET_FAMILY");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(env == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return family;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_SOCKET_FAMILY set by environment to %s", env);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 条件判断 | <code>(strcmp(env, "AF_INET") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>family = AF_INET;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 2 / 条件判断 | <code>(strcmp(env, "AF_INET6") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>family = AF_INET6;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 函数返回 | <code>return family;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int family = -1; // Family selection is not forced, will use first one found
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
```

</details>

<a id="f0974"></a>

## F0974：findInterfaces

用途：遍历系统接口，按地址族/名字前缀等筛选，保存有用接口名与地址。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct netIf userIfs[MAX_IFS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>bool searchNot = prefixList && prefixList[0] == '^';</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(searchNot)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>prefixList++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 准备局部状态 | <code>bool searchExact = prefixList && prefixList[0] == '=';</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 条件判断 | <code>(searchExact)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>prefixList++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 准备局部状态 | <code>int nUserIfs = parseStringList(prefixList, userIfs, MAX_IFS);</code> | 把逗号分隔的接口名前缀及可选端口解析成过滤表。 |
| 13 | 0 / 准备局部状态 | <code>int found = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 准备局部状态 | <code>struct ifaddrs *interfaces, *interface;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>getifaddrs(&interfaces);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 循环 | <code>初始 interface = interfaces；继续条件 interface && found &lt; maxIfs；每轮更新 interface = interface-&gt;ifa_next</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 1 / 条件判断 | <code>(interface-&gt;ifa_addr == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 20 | 1 / 准备局部状态 | <code>int family = interface-&gt;ifa_addr-&gt;sa_family;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 1 / 条件判断 | <code>(family != AF_INET && family != AF_INET6)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT&#124;NCCL_NET,"Found interface %s:%s", interface-&gt;ifa_name, ncclSocketToString((union ncclSocketAddress *) interface-&gt;ifa_addr, line));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 25 | 1 / 条件判断 | <code>(sock_family != -1 && family != sock_family)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 28 | 1 / 条件判断 | <code>(family == AF_INET6)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 3 / 准备局部状态 | <code>struct sockaddr_in6* sa = (struct sockaddr_in6*)(interface-&gt;ifa_addr);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 31 | 3 / 条件判断 | <code>(IN6_IS_ADDR_LOOPBACK(&sa-&gt;sin6_addr))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 34 | 1 / 条件判断 | <code>(!(matchIfList(interface-&gt;ifa_name, -1, userIfs, nUserIfs, searchExact) ^ searchNot))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 37 | 1 / 准备局部状态 | <code>bool duplicate = false;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 1 / 循环 | <code>初始 int i = 0;；继续条件 i &lt; found；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 39 | 2 / 条件判断 | <code>(strcmp(interface-&gt;ifa_name, names+i*maxIfNameSize) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 4 / 调用 / 计算 / 写入 | <code>duplicate = true;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 42 | 4 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 43 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 44 | 1 / 条件判断 | <code>(!duplicate)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 3 / 调用 / 计算 / 写入 | <code>strncpy(names+found*maxIfNameSize, interface-&gt;ifa_name, maxIfNameSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 47 | 3 / 准备局部状态 | <code>int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 48 | 3 / 调用 / 计算 / 写入 | <code>memcpy(addrs+found, interface-&gt;ifa_addr, salen);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 3 / 调用 / 计算 / 写入 | <code>found++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 51 | 0 / 调用 / 计算 / 写入 | <code>freeifaddrs(interfaces);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 52 | 0 / 函数返回 | <code>return found;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#ifdef ENABLE_TRACE
  char line[SOCKET_NAME_MAXLEN+1];
#endif
  struct netIf userIfs[MAX_IFS];
  bool searchNot = prefixList && prefixList[0] == '^';
  if (searchNot) prefixList++;
  bool searchExact = prefixList && prefixList[0] == '=';
  if (searchExact) prefixList++;
  int nUserIfs = parseStringList(prefixList, userIfs, MAX_IFS);

  int found = 0;
  struct ifaddrs *interfaces, *interface;
  getifaddrs(&interfaces);
  for (interface = interfaces; interface && found < maxIfs; interface = interface->ifa_next) {
    if (interface->ifa_addr == NULL) continue;

    /* We only support IPv4 & IPv6 */
    int family = interface->ifa_addr->sa_family;
    if (family != AF_INET && family != AF_INET6)
      continue;

    TRACE(NCCL_INIT|NCCL_NET,"Found interface %s:%s", interface->ifa_name, ncclSocketToString((union ncclSocketAddress *) interface->ifa_addr, line));

    /* Allow the caller to force the socket family type */
    if (sock_family != -1 && family != sock_family)
      continue;

    /* We also need to skip IPv6 loopback interfaces */
    if (family == AF_INET6) {
      struct sockaddr_in6* sa = (struct sockaddr_in6*)(interface->ifa_addr);
      if (IN6_IS_ADDR_LOOPBACK(&sa->sin6_addr)) continue;
    }

    // check against user specified interfaces
    if (!(matchIfList(interface->ifa_name, -1, userIfs, nUserIfs, searchExact) ^ searchNot)) {
      continue;
    }

    // Check that this interface has not already been saved
    // getifaddrs() normal order appears to be; IPv4, IPv6 Global, IPv6 Link
    bool duplicate = false;
    for (int i = 0; i < found; i++) {
      if (strcmp(interface->ifa_name, names+i*maxIfNameSize) == 0) { duplicate = true; break; }
    }

    if (!duplicate) {
      // Store the interface name
      strncpy(names+found*maxIfNameSize, interface->ifa_name, maxIfNameSize);
      // Store the IP address
      int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
      memcpy(addrs+found, interface->ifa_addr, salen);
      found++;
    }
  }

  freeifaddrs(interfaces);
  return found;
}
```

</details>

<a id="f0975"></a>

## F0975：matchSubnet

用途：比较本地接口与远端地址在相同地址族/掩码下是否属于同一子网。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int family = local_if.ifa_addr-&gt;sa_family;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(family != remote-&gt;sa.sa_family)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return false;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(family == AF_INET)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 准备局部状态 | <code>struct sockaddr_in* local_addr = (struct sockaddr_in*)(local_if.ifa_addr);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 准备局部状态 | <code>struct sockaddr_in* mask = (struct sockaddr_in*)(local_if.ifa_netmask);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 准备局部状态 | <code>struct sockaddr_in& remote_addr = remote-&gt;sin;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 准备局部状态 | <code>struct in_addr local_subnet, remote_subnet;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>local_subnet.s_addr = local_addr-&gt;sin_addr.s_addr & mask-&gt;sin_addr.s_addr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>remote_subnet.s_addr = remote_addr.sin_addr.s_addr & mask-&gt;sin_addr.s_addr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 函数返回 | <code>return (local_subnet.s_addr ^ remote_subnet.s_addr) ? false : true;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 15 | 2 / 条件判断 | <code>(family == AF_INET6)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 准备局部状态 | <code>struct sockaddr_in6* local_addr = (struct sockaddr_in6*)(local_if.ifa_addr);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 4 / 准备局部状态 | <code>struct sockaddr_in6* mask = (struct sockaddr_in6*)(local_if.ifa_netmask);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 4 / 准备局部状态 | <code>struct sockaddr_in6& remote_addr = remote-&gt;sin6;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 4 / 准备局部状态 | <code>struct in6_addr& local_in6 = local_addr-&gt;sin6_addr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 4 / 准备局部状态 | <code>struct in6_addr& mask_in6 = mask-&gt;sin6_addr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 4 / 准备局部状态 | <code>struct in6_addr& remote_in6 = remote_addr.sin6_addr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 4 / 准备局部状态 | <code>bool same = true;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 24 | 4 / 准备局部状态 | <code>int len = 16;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 4 / 循环 | <code>初始 int c = 0;；继续条件 c &lt; len；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 26 | 5 / 准备局部状态 | <code>char c1 = local_in6.s6_addr[c] & mask_in6.s6_addr[c];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 5 / 准备局部状态 | <code>char c2 = remote_in6.s6_addr[c] & mask_in6.s6_addr[c];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 5 / 条件判断 | <code>(c1 ^ c2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 7 / 调用 / 计算 / 写入 | <code>same = false;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 31 | 7 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 32 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 33 | 4 / 调用 / 计算 / 写入 | <code>same &= (local_addr-&gt;sin6_scope_id == remote_addr.sin6_scope_id);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 4 / 函数返回 | <code>return same;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 35 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 36 | 4 / 调用 / 计算 / 写入 | <code>WARN("Net : Unsupported address family type");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 37 | 4 / 函数返回 | <code>return false;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  /* Check family first */
  int family = local_if.ifa_addr->sa_family;
  if (family != remote->sa.sa_family) {
    return false;
  }

  if (family == AF_INET) {
    struct sockaddr_in* local_addr = (struct sockaddr_in*)(local_if.ifa_addr);
    struct sockaddr_in* mask = (struct sockaddr_in*)(local_if.ifa_netmask);
    struct sockaddr_in& remote_addr = remote->sin;
    struct in_addr local_subnet, remote_subnet;
    local_subnet.s_addr = local_addr->sin_addr.s_addr & mask->sin_addr.s_addr;
    remote_subnet.s_addr = remote_addr.sin_addr.s_addr & mask->sin_addr.s_addr;
    return (local_subnet.s_addr ^ remote_subnet.s_addr) ? false : true;
  } else if (family == AF_INET6) {
    struct sockaddr_in6* local_addr = (struct sockaddr_in6*)(local_if.ifa_addr);
    struct sockaddr_in6* mask = (struct sockaddr_in6*)(local_if.ifa_netmask);
    struct sockaddr_in6& remote_addr = remote->sin6;
    struct in6_addr& local_in6 = local_addr->sin6_addr;
    struct in6_addr& mask_in6 = mask->sin6_addr;
    struct in6_addr& remote_in6 = remote_addr.sin6_addr;
    bool same = true;
    int len = 16;  //IPv6 address is 16 unsigned char
    for (int c = 0; c < len; c++) {  //Network byte order is big-endian
      char c1 = local_in6.s6_addr[c] & mask_in6.s6_addr[c];
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
```

</details>

<a id="f0976"></a>

## F0976：ncclFindInterfaceMatchSubnet

用途：按远端地址找同子网本地接口，帮助 bootstrap 选择能够到达会合端的地址。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 2 | 1 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>char line_a[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int found = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ifaddrs *interfaces, *interface;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>getifaddrs(&interfaces);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 循环 | <code>初始 interface = interfaces；继续条件 interface && !found；每轮更新 interface = interface-&gt;ifa_next</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 条件判断 | <code>(interface-&gt;ifa_addr == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 11 | 1 / 准备局部状态 | <code>int family = interface-&gt;ifa_addr-&gt;sa_family;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 1 / 条件判断 | <code>(family != AF_INET && family != AF_INET6)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 15 | 1 / 条件判断 | <code>(!matchSubnet(*interface, remoteAddr))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 18 | 1 / 准备局部状态 | <code>int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 19 | 1 / 调用 / 计算 / 写入 | <code>memcpy(localAddrs+found, interface-&gt;ifa_addr, salen);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 1 / 调用 / 计算 / 写入 | <code>strncpy(ifNames+found*ifNameMaxSize, interface-&gt;ifa_name, ifNameMaxSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT&#124;NCCL_NET,"NET : Found interface %s:%s in the same subnet as remote address %s", interface-&gt;ifa_name, ncclSocketToString(localAddrs+found, line), ncclSocketToString(remoteAddr, line_a));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 22 | 1 / 调用 / 计算 / 写入 | <code>found++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 1 / 条件判断 | <code>(found == maxIfs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 26 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 0 / 条件判断 | <code>(found == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>WARN("Net : No interface found in the same subnet as remote address %s", ncclSocketToString(remoteAddr, line_a));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>freeifaddrs(interfaces);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 0 / 函数返回 | <code>return found;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
#ifdef ENABLE_TRACE
  char line[SOCKET_NAME_MAXLEN+1];
#endif
  char line_a[SOCKET_NAME_MAXLEN+1];
  int found = 0;
  struct ifaddrs *interfaces, *interface;
  getifaddrs(&interfaces);
  for (interface = interfaces; interface && !found; interface = interface->ifa_next) {
    if (interface->ifa_addr == NULL) continue;

    /* We only support IPv4 & IPv6 */
    int family = interface->ifa_addr->sa_family;
    if (family != AF_INET && family != AF_INET6)
      continue;

    // check against user specified interfaces
    if (!matchSubnet(*interface, remoteAddr)) {
      continue;
    }

    // Store the local IP address
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
```

</details>

<a id="f0977"></a>

## F0977：ncclGetSocketAddrFromString

用途：解析用户提供的 IP/主机及端口，构造统一 IPv4/IPv6 地址对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(!(ip_port_pair && strlen(ip_port_pair) &gt; 1))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("Net : string is null");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>bool ipv6 = ip_port_pair[0] == '[';</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(!ipv6)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 准备局部状态 | <code>struct netIf ni;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 条件判断 | <code>(parseStringList(ip_port_pair, &ni, 1) != 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 调用 / 计算 / 写入 | <code>WARN("Net : No valid &lt;IPv4_or_hostname&gt;:&lt;port&gt; pair found");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 4 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 2 / 准备局部状态 | <code>struct addrinfo hints, *p;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 2 / 准备局部状态 | <code>int rv;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>memset(&hints, 0, sizeof(hints));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>hints.ai_family = AF_UNSPEC;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>hints.ai_socktype = SOCK_STREAM;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 2 / 条件判断 | <code>( (rv = getaddrinfo(ni.prefix, NULL, &hints, &p)) != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>WARN("Net : error encountered when getting address info : %s", gai_strerror(rv));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 4 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 2 / 条件判断 | <code>(p-&gt;ai_family == AF_INET)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 4 / 准备局部状态 | <code>struct sockaddr_in& sin = ua-&gt;sin;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 4 / 调用 / 计算 / 写入 | <code>memcpy(&sin, p-&gt;ai_addr, sizeof(struct sockaddr_in));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>sin.sin_family = AF_INET;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 4 / 调用 / 计算 / 写入 | <code>sin.sin_port = htons(ni.port);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 29 | 4 / 条件判断 | <code>(p-&gt;ai_family == AF_INET6)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 6 / 准备局部状态 | <code>struct sockaddr_in6& sin6 = ua-&gt;sin6;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 32 | 6 / 调用 / 计算 / 写入 | <code>memcpy(&sin6, p-&gt;ai_addr, sizeof(struct sockaddr_in6));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 6 / 调用 / 计算 / 写入 | <code>sin6.sin6_family = AF_INET6;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 6 / 调用 / 计算 / 写入 | <code>sin6.sin6_port = htons(ni.port);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 6 / 调用 / 计算 / 写入 | <code>sin6.sin6_flowinfo = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 6 / 调用 / 计算 / 写入 | <code>sin6.sin6_scope_id = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 38 | 6 / 调用 / 计算 / 写入 | <code>WARN("Net : unsupported IP family");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 6 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 40 | 2 / 调用 / 计算 / 写入 | <code>freeaddrinfo(p);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 42 | 2 / 准备局部状态 | <code>int i, j = -1, len = strlen(ip_port_pair);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 2 / 循环 | <code>初始 i = 1；继续条件 i &lt; len；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 44 | 3 / 条件判断 | <code>(ip_port_pair[i] == '%')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 45 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 46 | 5 / 调用 / 计算 / 写入 | <code>j = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 3 / 条件判断 | <code>(ip_port_pair[i] == ']')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 48 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 49 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 50 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 51 | 2 / 条件判断 | <code>(i == len)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 52 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 53 | 4 / 调用 / 计算 / 写入 | <code>WARN("Net : No valid [IPv6]:port pair found");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 54 | 4 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 55 | 2 / 准备局部状态 | <code>bool global_scope = (j == -1 ? true : false);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 2 / 准备局部状态 | <code>char ip_str[NI_MAXHOST], port_str[NI_MAXSERV], if_name[IFNAMSIZ];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 57 | 2 / 调用 / 计算 / 写入 | <code>memset(ip_str, '\0', sizeof(ip_str));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 58 | 2 / 调用 / 计算 / 写入 | <code>memset(port_str, '\0', sizeof(port_str));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 59 | 2 / 调用 / 计算 / 写入 | <code>memset(if_name, '\0', sizeof(if_name));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 60 | 2 / 调用 / 计算 / 写入 | <code>strncpy(ip_str, ip_port_pair+1, global_scope ? i-1 : j-1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 61 | 2 / 调用 / 计算 / 写入 | <code>strncpy(port_str, ip_port_pair+i+2, len-i-1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 62 | 2 / 准备局部状态 | <code>int port = atoi(port_str);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 63 | 2 / 条件判断 | <code>(!global_scope)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 64 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 65 | 4 / 调用 / 计算 / 写入 | <code>strncpy(if_name, ip_port_pair+j+1, i-j-1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 66 | 2 / 准备局部状态 | <code>struct sockaddr_in6& sin6 = ua-&gt;sin6;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 67 | 2 / 调用 / 计算 / 写入 | <code>sin6.sin6_family = AF_INET6;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 68 | 2 / 调用 / 计算 / 写入 | <code>inet_pton(AF_INET6, ip_str, &(sin6.sin6_addr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 69 | 2 / 调用 / 计算 / 写入 | <code>sin6.sin6_port = htons(port);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 2 / 调用 / 计算 / 写入 | <code>sin6.sin6_flowinfo = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 71 | 2 / 调用 / 计算 / 写入 | <code>sin6.sin6_scope_id = global_scope ? 0 : if_nametoindex(if_name);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (!(ip_port_pair && strlen(ip_port_pair) > 1)) {
    WARN("Net : string is null");
    return ncclInvalidArgument;
  }

  bool ipv6 = ip_port_pair[0] == '[';
  /* Construct the sockaddress structure */
  if (!ipv6) {
    struct netIf ni;
    // parse <ip_or_hostname>:<port> string, expect one pair
    if (parseStringList(ip_port_pair, &ni, 1) != 1) {
      WARN("Net : No valid <IPv4_or_hostname>:<port> pair found");
      return ncclInvalidArgument;
    }

    struct addrinfo hints, *p;
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
    int i, j = -1, len = strlen(ip_port_pair);
    for (i = 1; i < len; i++) {
      if (ip_port_pair[i] == '%') j = i;
      if (ip_port_pair[i] == ']') break;
    }
    if (i == len) {
      WARN("Net : No valid [IPv6]:port pair found");
      return ncclInvalidArgument;
    }
    bool global_scope = (j == -1 ? true : false);     // If no % found, global scope; otherwise, link scope

    char ip_str[NI_MAXHOST], port_str[NI_MAXSERV], if_name[IFNAMSIZ];
    memset(ip_str, '\0', sizeof(ip_str));
    memset(port_str, '\0', sizeof(port_str));
    memset(if_name, '\0', sizeof(if_name));
    strncpy(ip_str, ip_port_pair+1, global_scope ? i-1 : j-1);
    strncpy(port_str, ip_port_pair+i+2, len-i-1);
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
```

</details>

<a id="f0978"></a>

## F0978：ncclFindInterfaces

用途：按显式接口配置或默认优先级选择可用接口，过滤不合适的 loopback/docker 等候选。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>static int shownIfName = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nIfs = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int sock_family = envSocketFamily();</code> | 解析环境选择的 IPv4/IPv6 地址族，影响接口筛选与连接建立。 |
| 4 | 0 / 准备局部状态 | <code>char* env = getenv("NCCL_SOCKET_IFNAME");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(env && strlen(env) &gt; 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_SOCKET_IFNAME set by environment to %s", env);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 条件判断 | <code>(shownIfName++ == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET, "NCCL_SOCKET_IFNAME set to %s", env);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>nIfs = findInterfaces(env, ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);</code> | 遍历系统接口，按地址族/名字前缀等筛选，保存有用接口名与地址。 |
| 12 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>nIfs = findInterfaces("ib", ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);</code> | 遍历系统接口，按地址族/名字前缀等筛选，保存有用接口名与地址。 |
| 14 | 2 / 条件判断 | <code>(nIfs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 4 / 准备局部状态 | <code>char* commId = getenv("NCCL_COMM_ID");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 4 / 条件判断 | <code>(commId && strlen(commId) &gt; 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 6 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", commId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 6 / 准备局部状态 | <code>union ncclSocketAddress idAddr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 6 / 调用 / 计算 / 写入 | <code>ncclGetSocketAddrFromString(&idAddr, commId);</code> | 解析用户提供的 IP/主机及端口，构造统一 IPv4/IPv6 地址对象。 |
| 22 | 6 / 调用 / 计算 / 写入 | <code>nIfs = ncclFindInterfaceMatchSubnet(ifNames, ifAddrs, &idAddr, ifNameMaxSize, maxIfs);</code> | 按远端地址找同子网本地接口，帮助 bootstrap 选择能够到达会合端的地址。 |
| 23 | 2 / 条件判断 | <code>(nIfs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 4 / 调用 / 计算 / 写入 | <code>nIfs = findInterfaces("^docker,lo", ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);</code> | 遍历系统接口，按地址族/名字前缀等筛选，保存有用接口名与地址。 |
| 26 | 2 / 条件判断 | <code>(nIfs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 4 / 调用 / 计算 / 写入 | <code>nIfs = findInterfaces("docker", ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);</code> | 遍历系统接口，按地址族/名字前缀等筛选，保存有用接口名与地址。 |
| 29 | 2 / 条件判断 | <code>(nIfs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 4 / 调用 / 计算 / 写入 | <code>nIfs = findInterfaces("lo", ifNames, ifAddrs, sock_family, ifNameMaxSize, maxIfs);</code> | 遍历系统接口，按地址族/名字前缀等筛选，保存有用接口名与地址。 |
| 32 | 0 / 函数返回 | <code>return nIfs;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static int shownIfName = 0;
  int nIfs = 0;
  // Allow user to force the INET socket family selection
  int sock_family = envSocketFamily();
  // User specified interface
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
      char* commId = getenv("NCCL_COMM_ID");
      if (commId && strlen(commId) > 1) {
	INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", commId);
	// Try to find interface that is in the same subnet as the IP in comm id
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
```

</details>

<a id="f0979"></a>

## F0979：ncclSocketListen

用途：按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int family = sock-&gt;addr.sa.sa_family;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int flags;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int fd = socket(family, SOCK_STREAM, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(fd == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>WARN("Net : Socket creation failed : %s", strerror(errno));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 条件判断 | <code>(socketToPort(&sock-&gt;addr))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 准备局部状态 | <code>int opt = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 2 / 编译期分支/指令 | <code>#if defined(SO_REUSEPORT)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 13 | 3 / 调用 / 计算 / 写入 | <code>SYSCHECK(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR &#124; SO_REUSEPORT, &opt, sizeof(opt)), "setsockopt");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 3 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>SYSCHECK(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), "setsockopt");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>EQCHECK(flags = fcntl(fd, F_GETFL), -1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(fcntl(fd, F_SETFL, flags &#124; O_NONBLOCK), "fcntl");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(bind(fd, &sock-&gt;addr.sa, salen), "bind");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 19 | 0 / 准备局部状态 | <code>socklen_t size = salen;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(getsockname(fd, &sock-&gt;addr.sa, &size), "getsockname");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 21 | 0 / 编译期分支/指令 | <code>#ifdef ENABLE_TRACE</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 22 | 1 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT&#124;NCCL_NET,"Listening on socket %s", ncclSocketToString(&sock-&gt;addr, line));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(listen(fd, 16384), "listen");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 25 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;fd = fd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  /* IPv4/IPv6 support */
  int family = sock->addr.sa.sa_family;
  int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
  int flags;

  /* Create socket and bind it to a port */
  int fd = socket(family, SOCK_STREAM, 0);
  if (fd == -1) {
    WARN("Net : Socket creation failed : %s", strerror(errno));
    return ncclSystemError;
  }

  if (socketToPort(&sock->addr)) {
    // Port is forced by env. Make sure we get the port.
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
  socklen_t size = salen;
  SYSCHECK(getsockname(fd, &sock->addr.sa, &size), "getsockname");

#ifdef ENABLE_TRACE
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
```

</details>

<a id="f0980"></a>

## F0980：getFdState

用途：检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct pollfd pfd;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int timeout = 1, ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>socklen_t rlen = sizeof(int);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>memset(&pfd, 0, sizeof(struct pollfd));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>pfd.fd = fd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>pfd.events = POLLOUT;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(ret = poll(&pfd, 1, timeout), "poll");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 0 / 条件判断 | <code>(ret == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>ret = EINPROGRESS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>EQCHECK(ret == 1 && (pfd.revents & POLLOUT), 0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>SYSCHECK(getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&ret, &rlen), "getsockopt");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 14 | 0 / 条件判断 | <code>(ret == EINPROGRESS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>*state = ncclSocketConnecting;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 18 | 2 / 条件判断 | <code>(ret == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>*state = ncclSocketConnected;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 22 | 4 / 调用 / 计算 / 写入 | <code>*state = ncclSocketError;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    struct pollfd pfd;
    int timeout = 1, ret;
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
```

</details>

<a id="f0981"></a>

## F0981：ncclGetSocketState

用途：检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getFdState(sock-&gt;fd, state));</code> | 失败时可能提前return，成功才继续；检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;state = *state;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    NCCLCHECK(getFdState(sock->fd, state));
    sock->state = *state;
    return ncclSuccess;
}
```

</details>

<a id="f0982"></a>

## F0982：ncclSocketConnect

用途：按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int family = sock-&gt;addr.sa.sa_family;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(family != AF_INET && family != AF_INET6)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>WARN("Net : connecting to address %s with family %d is neither AF_INET(%d) nor AF_INET6(%d)", ncclSocketToString(&sock-&gt;addr, line), family, AF_INET, AF_INET6);</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 6 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>int flags;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>int fd = socket(family, SOCK_STREAM, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 条件判断 | <code>(fd == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>WARN("Net : Socket creation failed : %s", strerror(errno));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 0 / 准备局部状态 | <code>const int one = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int)), "setsockopt");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>EQCHECK(flags = fcntl(fd, F_GETFL), -1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(fcntl(fd, F_SETFL, flags &#124; O_NONBLOCK), "fcntl");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT&#124;NCCL_NET,"Connecting to socket %s", ncclSocketToString(&sock-&gt;addr, line));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 19 | 0 / 准备局部状态 | <code>int ret;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 0 / 准备局部状态 | <code>int timedout_retries = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 0 / 准备局部状态 | <code>int refused_retries = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 0 / 跳转标签 | <code>retry</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>ret = connect(fd, &sock-&gt;addr.sa, salen);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 条件判断 | <code>(errno == EAGAIN &#124;&#124; (errno == ECONNREFUSED && ++refused_retries &lt; RETRY_REFUSED_TIMES) &#124;&#124; (errno == ETIMEDOUT && ++timedout_retries &lt; RETRY_TIMEDOUT_TIMES))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 2 / 条件判断 | <code>(refused_retries % 1000 == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ALL, "Call to connect returned %s, retrying", strerror(errno));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>usleep(SLEEP_INT);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 2 / 控制转移 | <code>goto retry;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 31 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 32 | 2 / 条件判断 | <code>(errno == EINPROGRESS && !sock-&gt;asyncFlag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 4 / 准备局部状态 | <code>enum ncclSocketState state;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 35 | 4 / 先执行一次的循环 | <code>初始 ；继续条件 (state == ncclSocketConnecting)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 36 | 5 / 条件判断 | <code>(sock-&gt;abortFlag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 7 / 调用 / 计算 / 写入 | <code>NEQCHECK(*sock-&gt;abortFlag, 0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 39 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(getFdState(fd, &state));</code> | 失败时可能提前return，成功才继续；检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。 |
| 40 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 41 | 4 / 调用 / 计算 / 写入 | <code>EQCHECK(state, ncclSocketError);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 4 / 调用 / 计算 / 写入 | <code>ret = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 43 | 0 / 条件判断 | <code>(ret == 0 &#124;&#124; (errno == EINPROGRESS && sock-&gt;asyncFlag))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 2 / 调用 / 计算 / 写入 | <code>sock-&gt;fd = fd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>WARN("Net : Connect to %s failed : %s", ncclSocketToString(&sock-&gt;addr, line), strerror(errno));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 48 | 0 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char line[SOCKET_NAME_MAXLEN+1];
  /* IPv4/IPv6 support */
  int family = sock->addr.sa.sa_family;
  if (family != AF_INET && family != AF_INET6) {
    WARN("Net : connecting to address %s with family %d is neither AF_INET(%d) nor AF_INET6(%d)",
         ncclSocketToString(&sock->addr, line), family, AF_INET, AF_INET6);
    return ncclInternalError;
  }
  int salen = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
  int flags;

  /* Connect to a hostname / port */
  int fd = socket(family, SOCK_STREAM, 0);
  if (fd == -1) {
    WARN("Net : Socket creation failed : %s", strerror(errno));
    return ncclSystemError;
  }

  const int one = 1;
  SYSCHECK(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int)), "setsockopt");

  /* support non-blocking socket; by default, the socket is non-blocking */
  EQCHECK(flags = fcntl(fd, F_GETFL), -1);
  SYSCHECK(fcntl(fd, F_SETFL, flags | O_NONBLOCK), "fcntl");

  /*  const int bufsize = 128*1024;
    SYSCHECK(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&bufsize, sizeof(int)), "setsockopt");
    SYSCHECK(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&bufsize, sizeof(int)), "setsockopt");*/

  TRACE(NCCL_INIT|NCCL_NET,"Connecting to socket %s", ncclSocketToString(&sock->addr, line));

  int ret;
  int timedout_retries = 0;
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
```

</details>

<a id="f0983"></a>

## F0983：ncclSocketAccept

用途：按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>socklen_t socklen = sizeof(union ncclSocketAddress);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int tmpFd = sock-&gt;fd = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 先执行一次的循环 | <code>初始 ；继续条件 ((errno == EAGAIN &#124;&#124; errno == EWOULDBLOCK) && tmpFd == -1 && !listenSocket-&gt;asyncFlag)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 4 | 1 / 条件判断 | <code>(listenSocket-&gt;abortFlag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>NEQCHECK(*listenSocket-&gt;abortFlag, 0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>tmpFd = accept(listenSocket-&gt;fd, &sock-&gt;addr.sa, &socklen);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 9 | 0 / 条件判断 | <code>(!listenSocket-&gt;asyncFlag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>EQCHECK(tmpFd, -1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 2 / 条件判断 | <code>(tmpFd == -1 && errno != EAGAIN && errno != EWOULDBLOCK)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;fd = tmpFd;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  socklen_t socklen = sizeof(union ncclSocketAddress);
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
```

</details>

<a id="f0984"></a>

## F0984：ncclSocketInit

用途：本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(sock == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(addr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>memcpy(&sock-&gt;addr, addr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>memset(&sock-&gt;addr, 0, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;abortFlag = abortFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;asyncFlag = asyncFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;state = ncclSocketStateNum;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f0985"></a>

## F0985：ncclSocketProgressOpt

用途：按保存 offset 尽力推进 TCP send/recv，可选持续等待或检测关闭，并响应 abort。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int bytes = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>*closed = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 准备局部状态 | <code>char* data = (char*)ptr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 先执行一次的循环 | <code>初始 ；继续条件 (bytes &gt; 0 && (*offset) &lt; size)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 6 | 1 / 条件判断 | <code>(op == NCCL_SOCKET_RECV)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>bytes = recv(sock-&gt;fd, data+(*offset), size-(*offset), block ? 0 : MSG_DONTWAIT);</code> | 接收上游数据并写本地输出，本动作不加上本地输入。 |
| 9 | 1 / 条件判断 | <code>(op == NCCL_SOCKET_SEND)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>bytes = send(sock-&gt;fd, data+(*offset), size-(*offset), block ? 0 : MSG_DONTWAIT);</code> | 从本地输入读取并发送到原语的发送邻居，不要求写本地输出。 |
| 12 | 1 / 条件判断 | <code>(op == NCCL_SOCKET_RECV && bytes == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 3 / 调用 / 计算 / 写入 | <code>*closed = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 16 | 1 / 条件判断 | <code>(bytes == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 3 / 条件判断 | <code>(errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 5 / 调用 / 计算 / 写入 | <code>WARN("Net : Call to recv from %s failed : %s", ncclSocketToString(&sock-&gt;addr, line), strerror(errno));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 21 | 5 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 22 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 5 / 调用 / 计算 / 写入 | <code>bytes = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>(*offset) += bytes;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 条件判断 | <code>(sock-&gt;abortFlag && *sock-&gt;abortFlag != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET, "Socket progress: abort called");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 3 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 29 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 30 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int bytes = 0;
  *closed = 0;
  char* data = (char*)ptr;
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
```

</details>

<a id="f0986"></a>

## F0986：ncclSocketProgress

用途：调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int closed;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgressOpt(op, sock, ptr, size, offset, 0, &closed));</code> | 失败时可能提前return，成功才继续；按保存 offset 尽力推进 TCP send/recv，可选持续等待或检测关闭，并响应 abort。 |
| 3 | 0 / 条件判断 | <code>(closed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>WARN("Net : Connection closed by remote peer %s", ncclSocketToString(&sock-&gt;addr, line, 0));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 7 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int closed;
  NCCLCHECK(ncclSocketProgressOpt(op, sock, ptr, size, offset, 0, &closed));
  if (closed) {
    char line[SOCKET_NAME_MAXLEN+1];
    WARN("Net : Connection closed by remote peer %s", ncclSocketToString(&sock->addr, line, 0));
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0987"></a>

## F0987：ncclSocketWait

用途：重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 ；继续条件 (*offset &lt; size)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(op, sock, ptr, size, offset));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  while (*offset < size)
    NCCLCHECK(ncclSocketProgress(op, sock, ptr, size, offset));
  return ncclSuccess;
}
```

</details>

<a id="f0988"></a>

## F0988：ncclSocketSend

用途：从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketWait(NCCL_SOCKET_SEND, sock, ptr, size, &offset));</code> | 失败时可能提前return，成功才继续；重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int offset = 0;
  NCCLCHECK(ncclSocketWait(NCCL_SOCKET_SEND, sock, ptr, size, &offset));
  return ncclSuccess;
}
```

</details>

<a id="f0989"></a>

## F0989：ncclSocketRecv

用途：从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, sock, ptr, size, &offset));</code> | 失败时可能提前return，成功才继续；重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int offset = 0;
  NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, sock, ptr, size, &offset));
  return ncclSuccess;
}
```

</details>

<a id="f0990"></a>

## F0990：ncclSocketTryRecv

用途：尝试接收并可区分对端关闭，供代理服务循环避免无限阻塞在已断开连接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>*closed = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 循环 | <code>初始 ；继续条件 (offset &lt; size)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgressOpt(NCCL_SOCKET_RECV, sock, ptr, size, &offset, 0, closed));</code> | 失败时可能提前return，成功才继续；按保存 offset 尽力推进 TCP send/recv，可选持续等待或检测关闭，并响应 abort。 |
| 5 | 1 / 条件判断 | <code>(*closed)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int offset = 0;
  *closed = 0;
  while (offset < size) {
    NCCLCHECK(ncclSocketProgressOpt(NCCL_SOCKET_RECV, sock, ptr, size, &offset, 0, closed));
    if (*closed) return ncclSuccess;
  }
  return ncclSuccess;
}
```

</details>

