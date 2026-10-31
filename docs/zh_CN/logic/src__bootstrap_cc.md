# src/bootstrap.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0031"></a>

## F0031：bootstrapNetInit

用途：选择初始化控制面使用的本机网络接口和地址；其选择不等于数据面必然使用 TCP。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(bootstrapNetInitDone == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&bootstrapNetLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 条件判断 | <code>(bootstrapNetInitDone == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 准备局部状态 | <code>char* env = getenv("NCCL_COMM_ID");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 4 / 条件判断 | <code>(env)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 6 / 准备局部状态 | <code>union ncclSocketAddress remoteAddr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 6 / 条件判断 | <code>(ncclGetSocketAddrFromString(&remoteAddr, env) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 8 / 调用 / 计算 / 写入 | <code>WARN("Invalid NCCL_COMM_ID, please use format: &lt;ipv4&gt;:&lt;port&gt; or [&lt;ipv6&gt;]:&lt;port&gt; or &lt;hostname&gt;:&lt;port&gt;");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 8 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 6 / 条件判断 | <code>(ncclFindInterfaceMatchSubnet(bootstrapNetIfName, &bootstrapNetIfAddr, &remoteAddr, MAX_IF_NAME_SIZE, 1) &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 8 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : No usable listening interface found");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 8 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 6 / 准备局部状态 | <code>int nIfs = ncclFindInterfaces(bootstrapNetIfName, &bootstrapNetIfAddr, MAX_IF_NAME_SIZE, 1);</code> | 按显式接口配置或默认优先级选择可用接口，过滤不合适的 loopback/docker 等候选。 |
| 20 | 6 / 条件判断 | <code>(nIfs &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 8 / 调用 / 计算 / 写入 | <code>WARN("Bootstrap : no socket interface found");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 8 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 24 | 4 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+MAX_IF_NAME_SIZE+2];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 4 / 调用 / 计算 / 写入 | <code>sprintf(line, " %s:", bootstrapNetIfName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>ncclSocketToString(&bootstrapNetIfAddr, line+strlen(line));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 27 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "Bootstrap : Using%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 4 / 调用 / 计算 / 写入 | <code>bootstrapNetInitDone = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&bootstrapNetLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (bootstrapNetInitDone == 0) {
    pthread_mutex_lock(&bootstrapNetLock);
    if (bootstrapNetInitDone == 0) {
      char* env = getenv("NCCL_COMM_ID");
      if (env) {
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
        int nIfs = ncclFindInterfaces(bootstrapNetIfName, &bootstrapNetIfAddr, MAX_IF_NAME_SIZE, 1);
        if (nIfs <= 0) {
          WARN("Bootstrap : no socket interface found");
          return ncclInternalError;
        }
      }
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
```

</details>

<a id="f0032"></a>

## F0032：bootstrapNetSend

用途：先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(sock, &size, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketSend(sock, data, size));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclSocketSend(sock, &size, sizeof(int)));
  NCCLCHECK(ncclSocketSend(sock, data, size));
  return ncclSuccess;
}
```

</details>

<a id="f0033"></a>

## F0033：bootstrapNetRecv

用途：读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int recvSize;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, &recvSize, sizeof(int)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 3 | 0 / 条件判断 | <code>(recvSize &gt; size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>WARN("Message truncated : received %d bytes instead of %d", recvSize, size);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 6 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketRecv(sock, data, std::min(recvSize, size)));</code> | 失败时可能提前return，成功才继续；从 offset=0 开始发送/接收指定字节数，调用等待型基础逻辑确保该段完整。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int recvSize;
  NCCLCHECK(ncclSocketRecv(sock, &recvSize, sizeof(int)));
  if (recvSize > size) {
    WARN("Message truncated : received %d bytes instead of %d", recvSize, size);
    return ncclInternalError;
  }
  NCCLCHECK(ncclSocketRecv(sock, data, std::min(recvSize, size)));
  return ncclSuccess;
}
```

</details>

<a id="f0034"></a>

## F0034：setFilesLimit

用途：尝试提高本进程可打开文件描述符上限，以容纳初始化与多连接使用的 socket。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct rlimit filesLimit;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(getrlimit(RLIMIT_NOFILE, &filesLimit), "getrlimit");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>filesLimit.rlim_cur = filesLimit.rlim_max;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>SYSCHECK(setrlimit(RLIMIT_NOFILE, &filesLimit), "setrlimit");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct rlimit filesLimit;
  SYSCHECK(getrlimit(RLIMIT_NOFILE, &filesLimit), "getrlimit");
  filesLimit.rlim_cur = filesLimit.rlim_max;
  SYSCHECK(setrlimit(RLIMIT_NOFILE, &filesLimit), "setrlimit");
  return ncclSuccess;
}
```

</details>

<a id="f0035"></a>

## F0035：bootstrapRoot

用途：收集所有 rank 的会合信息并分发环邻居地址；这是 CPU 控制服务，不处理张量归约。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocket* listenSock = (struct ncclSocket*)args;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>ncclResult_t res = ncclSuccess;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int nranks = 0, c = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>struct extInfo info;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>union ncclSocketAddress *rankAddresses = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>union ncclSocketAddress *rankAddressesRoot = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>union ncclSocketAddress *zero = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclCalloc(&zero, 1), res, out);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>setFilesLimit();</code> | 尝试提高本进程可打开文件描述符上限，以容纳初始化与多连接使用的 socket。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "BEGIN");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 0 / 先执行一次的循环 | <code>初始 ；继续条件 (c &lt; nranks)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 12 | 1 / 准备局部状态 | <code>struct ncclSocket sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>sock.abortFlag = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSocketAccept(&sock, listenSock), res, out);</code> | 失败时保存错误并跳到清理标签，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(bootstrapNetRecv(&sock, &info, sizeof(info)), res, out);</code> | 失败时保存错误并跳到清理标签，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。 |
| 16 | 1 / 调用 / 计算 / 写入 | <code>close(sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 1 / 条件判断 | <code>(c == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 3 / 调用 / 计算 / 写入 | <code>nranks = info.nranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclCalloc(&rankAddresses, nranks), res, out);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclCalloc(&rankAddressesRoot, nranks), res, out);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 22 | 1 / 条件判断 | <code>(nranks != info.nranks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 3 / 调用 / 计算 / 写入 | <code>WARN("Bootstrap Root : mismatch in rank count from procs %d : %d", nranks, info.nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 3 / 控制转移 | <code>goto out;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 26 | 1 / 条件判断 | <code>(memcmp(zero, &rankAddressesRoot[info.rank], sizeof(union ncclSocketAddress)) != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 27 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>WARN("Bootstrap Root : rank %d of %d ranks has already checked in", info.rank, nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 3 / 控制转移 | <code>goto out;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 30 | 1 / 调用 / 计算 / 写入 | <code>memcpy(rankAddressesRoot+info.rank, &info.extAddressListenRoot, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 1 / 调用 / 计算 / 写入 | <code>memcpy(rankAddresses+info.rank, &info.extAddressListen, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 1 / 调用 / 计算 / 写入 | <code>++c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 1 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "Received connect from rank %d total %d/%d", info.rank, c, nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 34 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "COLLECTED ALL %d HANDLES", nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 36 | 0 / 循环 | <code>初始 int r=0;；继续条件 r&lt;nranks；每轮更新 ++r</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 37 | 1 / 准备局部状态 | <code>int next = (r+1) % nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 38 | 1 / 准备局部状态 | <code>struct ncclSocket sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 39 | 1 / 调用 / 计算 / 写入 | <code>sock.abortFlag = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 1 / 调用 / 计算 / 写入 | <code>memcpy(&sock.addr, rankAddressesRoot+r, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclSocketConnect(&sock), res, out);</code> | 失败时保存错误并跳到清理标签，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。 |
| 42 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(bootstrapNetSend(&sock, rankAddresses+next, sizeof(union ncclSocketAddress)), res, out);</code> | 失败时保存错误并跳到清理标签，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。 |
| 43 | 1 / 调用 / 计算 / 写入 | <code>close(sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "SENT OUT ALL %d HANDLES", nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 0 / 跳转标签 | <code>out</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 47 | 1 / 调用 / 计算 / 写入 | <code>close(listenSock-&gt;fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>free(listenSock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 0 / 条件判断 | <code>(rankAddresses)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 50 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 51 | 2 / 调用 / 计算 / 写入 | <code>free(rankAddresses);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 52 | 0 / 条件判断 | <code>(rankAddressesRoot)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 2 / 调用 / 计算 / 写入 | <code>free(rankAddressesRoot);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 55 | 0 / 条件判断 | <code>(zero)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 56 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 57 | 2 / 调用 / 计算 / 写入 | <code>free(zero);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 58 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "DONE");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 59 | 0 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocket* listenSock = (struct ncclSocket*)args;
  ncclResult_t res = ncclSuccess;
  int nranks = 0, c = 0;
  struct extInfo info;
  union ncclSocketAddress *rankAddresses = NULL;
  union ncclSocketAddress *rankAddressesRoot = NULL; // for initial rank <-> root information exchange
  union ncclSocketAddress *zero = NULL;
  NCCLCHECKGOTO(ncclCalloc(&zero, 1), res, out);
  setFilesLimit();

  TRACE(NCCL_INIT, "BEGIN");
  /* Receive addresses from all ranks */
  do {
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
  for (int r=0; r<nranks; ++r) {
    int next = (r+1) % nranks;
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
```

</details>

<a id="f0036"></a>

## F0036：bootstrapCreateRoot

用途：创建 bootstrap root 监听对象和后台线程，为后续 rank 会合提供入口。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocket* listenSock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&listenSock, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&listenSock-&gt;addr, id, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketListen(listenSock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>memcpy(id, &listenSock-&gt;addr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 准备局部状态 | <code>pthread_t thread;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>pthread_create(&thread, NULL, bootstrapRoot, (void*)listenSock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>ncclSetThreadName(thread, "NCCL BootstrapR");</code> | 为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>pthread_detach(thread);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocket* listenSock;
  NCCLCHECK(ncclCalloc(&listenSock, 1));
  memcpy(&listenSock->addr, id, sizeof(union ncclSocketAddress));
  NCCLCHECK(ncclSocketListen(listenSock));
  memcpy(id, &listenSock->addr, sizeof(union ncclSocketAddress));
  pthread_t thread;
  pthread_create(&thread, NULL, bootstrapRoot, (void*)listenSock);
  ncclSetThreadName(thread, "NCCL BootstrapR");
  pthread_detach(thread); // will not be pthread_join()'d
  return ncclSuccess;
}
```

</details>

<a id="f0037"></a>

## F0037：bootstrapGetUniqueId

用途：生成/读取引导会合标识供应用分发，必要时建立 root 服务。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 源码语法片段 | <code>static_assert(sizeof(union ncclSocketAddress) &lt; sizeof(ncclUniqueId), "NetId does not fit inside ncclUniqueId");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>memset(id, 0, sizeof(ncclUniqueId));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 准备局部状态 | <code>union ncclSocketAddress* connectAddr = (union ncclSocketAddress*) id;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>char* env = getenv("NCCL_COMM_ID");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(env)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ENV, "NCCL_COMM_ID set by environment to %s", env);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 条件判断 | <code>(ncclGetSocketAddrFromString(connectAddr, env) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 4 / 调用 / 计算 / 写入 | <code>WARN("Invalid NCCL_COMM_ID, please use format: &lt;ipv4&gt;:&lt;port&gt; or [&lt;ipv6&gt;]:&lt;port&gt; or &lt;hostname&gt;:&lt;port&gt;");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 4 / 函数返回 | <code>return ncclInvalidArgument;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>memcpy(id, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapCreateRoot(id, false));</code> | 失败时可能提前return，成功才继续；创建 bootstrap root 监听对象和后台线程，为后续 rank 会合提供入口。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static_assert(sizeof(union ncclSocketAddress) < sizeof(ncclUniqueId), "NetId does not fit inside ncclUniqueId");
  memset(id, 0, sizeof(ncclUniqueId));
  union ncclSocketAddress* connectAddr = (union ncclSocketAddress*) id;

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
```

</details>

<a id="f0038"></a>

## F0038：bootstrapInit

用途：让当前 rank 加入控制网络并交换 peer/proxy 地址，为拓扑发现和传输建链准备控制通道。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int rank = comm-&gt;rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nranks = comm-&gt;nRanks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct bootstrapState* state;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&state, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;rank = rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;nranks = nranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>state-&gt;abortFlag = comm-&gt;abortFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;bootstrap = state;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d", rank, nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 准备局部状态 | <code>struct extInfo info = { 0 };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>info.rank = rank;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>info.nranks = nranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 准备局部状态 | <code>struct ncclSocket sock, listenSockRoot;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>sock.abortFlag = listenSockRoot.abortFlag = comm-&gt;abortFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>sock.asyncFlag = listenSockRoot.asyncFlag = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&state-&gt;listenSock.addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketListen(&state-&gt;listenSock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&info.extAddressListen, &state-&gt;listenSock.addr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&listenSockRoot.addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketListen(&listenSockRoot));</code> | 失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&info.extAddressListenRoot, &listenSockRoot.addr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 0 / 条件判断 | <code>(nranks &gt; 128)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 准备局部状态 | <code>long msec = rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 25 | 2 / 准备局部状态 | <code>struct timespec tv;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>tv.tv_sec = msec / 1000;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>tv.tv_nsec = 1000000 * (msec % 1000);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d delaying connection to root by %ld msec", rank, msec);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>(void) nanosleep(&tv, NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&sock.addr, id, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketConnect(&sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。 |
| 32 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetSend(&sock, &info, sizeof(info)));</code> | 失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>close(sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketAccept(&sock, &listenSockRoot));</code> | 失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。 |
| 35 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetRecv(&sock, &state-&gt;ringSendSocket.addr, sizeof(union ncclSocketAddress)));</code> | 失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>close(sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>close(listenSockRoot.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketConnect(&state-&gt;ringSendSocket));</code> | 失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。 |
| 39 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketAccept(&state-&gt;ringRecvSocket, &state-&gt;listenSock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&state-&gt;peerCommAddresses, nranks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 41 | 0 / 调用 / 计算 / 写入 | <code>memcpy(state-&gt;peerCommAddresses+rank, &state-&gt;listenSock.addr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapAllGather(state, state-&gt;peerCommAddresses, sizeof(union ncclSocketAddress)));</code> | 失败时可能提前return，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&state-&gt;peerProxyAddresses, nranks));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 44 | 0 / 准备局部状态 | <code>struct ncclSocket* proxySocket;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 45 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&proxySocket, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>proxySocket-&gt;abortFlag = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&proxySocket-&gt;addr, &bootstrapNetIfAddr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketListen(proxySocket));</code> | 失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。 |
| 49 | 0 / 调用 / 计算 / 写入 | <code>memcpy(state-&gt;peerProxyAddresses+rank, &proxySocket-&gt;addr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 50 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapAllGather(state, state-&gt;peerProxyAddresses, sizeof(union ncclSocketAddress)));</code> | 失败时可能提前return，成功才继续；沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。 |
| 51 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclProxyInit(comm, proxySocket, state-&gt;peerProxyAddresses));</code> | 失败时可能提前return，成功才继续；将 bootstrap 建立的监听 socket 和全组代理地址表接入当前 communicator 的 Proxy 状态。 |
| 52 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d - DONE", rank, nranks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 53 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int rank = comm->rank;
  int nranks = comm->nRanks;
  struct bootstrapState* state;
  NCCLCHECK(ncclCalloc(&state, 1));
  state->rank = rank;
  state->nranks = nranks;
  state->abortFlag = comm->abortFlag;
  comm->bootstrap = state;

  TRACE(NCCL_INIT, "rank %d nranks %d", rank, nranks);

  struct extInfo info = { 0 };
  info.rank = rank;
  info.nranks = nranks;
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
    long msec = rank;
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
```

</details>

<a id="f0039"></a>

## F0039：bootstrapAllGather

用途：沿控制 socket 环逐轮交换每个 rank 的固定尺寸元数据，最终每方拥有全组摘要。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct bootstrapState* state = (struct bootstrapState*)commState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>char* data = (char*)allData;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int rank = state-&gt;rank;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int nranks = state-&gt;nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d size %d", rank, nranks, size);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;nranks-1；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 准备局部状态 | <code>size_t rslice = (rank - i - 1 + nranks) % nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 准备局部状态 | <code>size_t sslice = (rank - i + nranks) % nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetSend(&state-&gt;ringSendSocket, data+sslice*size, size));</code> | 失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetRecv(&state-&gt;ringRecvSocket, data+rslice*size, size));</code> | 失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。 |
| 11 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d size %d - DONE", rank, nranks, size);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct bootstrapState* state = (struct bootstrapState*)commState;
  char* data = (char*)allData;
  int rank = state->rank;
  int nranks = state->nranks;

  TRACE(NCCL_INIT, "rank %d nranks %d size %d", rank, nranks, size);

  /* Simple ring based AllGather
   * At each step i receive data from (rank-i-1) from left
   * and send previous step's data from (rank-i) to right
   */
  for (int i=0; i<nranks-1; i++) {
    size_t rslice = (rank - i - 1 + nranks) % nranks;
    size_t sslice = (rank - i + nranks) % nranks;

    // Send slice to the right
    NCCLCHECK(bootstrapNetSend(&state->ringSendSocket, data+sslice*size, size));
    // Recv slice from the left
    NCCLCHECK(bootstrapNetRecv(&state->ringRecvSocket, data+rslice*size, size));
  }

  TRACE(NCCL_INIT, "rank %d nranks %d size %d - DONE", rank, nranks, size);
  return ncclSuccess;
}
```

</details>

<a id="f0040"></a>

## F0040：bootstrapSend

用途：向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct bootstrapState* state = (struct bootstrapState*)commState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclSocket sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>sock.abortFlag = state-&gt;abortFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&sock.addr, state-&gt;peerCommAddresses+peer, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketConnect(&sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetSend(&sock, &state-&gt;rank, sizeof(int)));</code> | 失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetSend(&sock, &tag, sizeof(int)));</code> | 失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetSend(&sock, data, size));</code> | 失败时可能提前return，成功才继续；先发送控制消息长度再发送内容，使 TCP 字节流上能恢复一条完整初始化消息。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>close(sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct bootstrapState* state = (struct bootstrapState*)commState;
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
```

</details>

<a id="f0041"></a>

## F0041：bootstrapBarrier

用途：使用倍增距离的控制消息交换，使所选 rank 子集合完成初始化会合。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(nranks == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d tag %x - ENTER", rank, nranks, tag);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 准备局部状态 | <code>int data[1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 循环 | <code>初始 int mask=1;；继续条件 mask&lt;nranks；每轮更新 mask&lt;&lt;=1</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 准备局部状态 | <code>int src = (rank - mask + nranks) % nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 准备局部状态 | <code>int dst = (rank + mask) % nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapSend(commState, ranks[dst], tag, data, sizeof(data)));</code> | 失败时可能提前return，成功才继续；向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapRecv(commState, ranks[src], tag, data, sizeof(data)));</code> | 失败时可能提前return，成功才继续；优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。 |
| 11 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d tag %x - DONE", rank, nranks, tag);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (nranks == 1) return ncclSuccess;
  TRACE(NCCL_INIT, "rank %d nranks %d tag %x - ENTER", rank, nranks, tag);

  /* Simple intra process barrier
   *
   * Based on the dissemination algorithm by Debra Hensgen, Raphael Finkel, and Udi Manbet,
   * "Two Algorithms for Barrier Synchronization," International Journal of Parallel Programming, 17(1):1-17, 1988"
   */
  int data[1];
  for (int mask=1; mask<nranks; mask<<=1) {
    int src = (rank - mask + nranks) % nranks;
    int dst = (rank + mask) % nranks;
    NCCLCHECK(bootstrapSend(commState, ranks[dst], tag, data, sizeof(data)));
    NCCLCHECK(bootstrapRecv(commState, ranks[src], tag, data, sizeof(data)));
  }

  TRACE(NCCL_INIT, "rank %d nranks %d tag %x - DONE", rank, nranks, tag);
  return ncclSuccess;
}
```

</details>

<a id="f0042"></a>

## F0042：bootstrapIntraNodeAllGather

用途：只在所选节点内 rank 集合交换固定大小控制信息，按局部索引映射到全局 rank。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(nranks == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>char* data = (char*)allData;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d size %d - ENTER", rank, nranks, size);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 循环 | <code>初始 int i=1;；继续条件 i&lt;nranks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 准备局部状态 | <code>int src = (rank - i + nranks) % nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 准备局部状态 | <code>int dst = (rank + i) % nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapSend(commState, ranks[dst], i, data+rank*size, size));</code> | 失败时可能提前return，成功才继续；向指定 rank 的控制监听地址建立连接，发送本 rank、tag 及消息内容以便对端匹配。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapRecv(commState, ranks[src], i, data+src*size, size));</code> | 失败时可能提前return，成功才继续；优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。 |
| 11 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_INIT, "rank %d nranks %d size %d - DONE", rank, nranks, size);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (nranks == 1) return ncclSuccess;
  char* data = (char*)allData;
  TRACE(NCCL_INIT, "rank %d nranks %d size %d - ENTER", rank, nranks, size);

  for (int i=1; i<nranks; i++) {
    int src = (rank - i + nranks) % nranks;
    int dst = (rank + i) % nranks;
    NCCLCHECK(bootstrapSend(commState, ranks[dst], /*tag=*/i, data+rank*size, size));
    NCCLCHECK(bootstrapRecv(commState, ranks[src], /*tag=*/i, data+src*size, size));
  }

  TRACE(NCCL_INIT, "rank %d nranks %d size %d - DONE", rank, nranks, size);
  return ncclSuccess;
}
```

</details>

<a id="f0043"></a>

## F0043：unexpectedEnqueue

用途：保存暂时不匹配当前 peer/tag 的控制连接，避免先到消息被错误接收。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct unexConn* unex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&unex, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>unex-&gt;peer = peer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>unex-&gt;tag = tag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&unex-&gt;sock, sock, sizeof(struct ncclSocket));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 准备局部状态 | <code>struct unexConn* list = state-&gt;unexpectedConnections;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 条件判断 | <code>(list == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>state-&gt;unexpectedConnections = unex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 循环 | <code>初始 ；继续条件 (list-&gt;next)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>list = list-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>list-&gt;next = unex;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // New unex
  struct unexConn* unex;
  NCCLCHECK(ncclCalloc(&unex, 1));
  unex->peer = peer;
  unex->tag = tag;
  memcpy(&unex->sock, sock, sizeof(struct ncclSocket));

  // Enqueue
  struct unexConn* list = state->unexpectedConnections;
  if (list == NULL) {
    state->unexpectedConnections = unex;
    return ncclSuccess;
  }
  while (list->next) list = list->next;
  list->next = unex;
  return ncclSuccess;
}
```

</details>

<a id="f0044"></a>

## F0044：unexpectedDequeue

用途：从暂存连接链中找出匹配 peer/tag 的项并摘除，供当前接收继续使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct unexConn* elem = state-&gt;unexpectedConnections;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct unexConn* prev = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 ；继续条件 (elem)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(elem-&gt;peer == peer && elem-&gt;tag == tag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 条件判断 | <code>(prev == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 调用 / 计算 / 写入 | <code>state-&gt;unexpectedConnections = elem-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 10 | 5 / 调用 / 计算 / 写入 | <code>prev-&gt;next = elem-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>memcpy(sock, &elem-&gt;sock, sizeof(struct ncclSocket));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>free(elem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>prev = elem;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 1 / 调用 / 计算 / 写入 | <code>elem = elem-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>sock-&gt;fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct unexConn* elem = state->unexpectedConnections;
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
```

</details>

<a id="f0045"></a>

## F0045：bootstrapRecv

用途：优先匹配暂存连接，否则接收新控制连接并按 peer/tag 分类后读取正确消息。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct bootstrapState* state = (struct bootstrapState*)commState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclSocket sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>sock.abortFlag = state-&gt;abortFlag;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(unexpectedDequeue(state, peer, tag, &sock));</code> | 失败时可能提前return，成功才继续；从暂存连接链中找出匹配 peer/tag 的项并摘除，供当前接收继续使用。 |
| 5 | 0 / 条件判断 | <code>(sock.fd != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetRecv(&sock, ((char*)data), size));</code> | 失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>close(sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 10 | 0 / 循环 | <code>初始 ；继续条件 (1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketAccept(&sock, &state-&gt;listenSock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。 |
| 12 | 1 / 准备局部状态 | <code>int newPeer, newTag;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetRecv(&sock, &newPeer, sizeof(int)));</code> | 失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetRecv(&sock, &newTag, sizeof(int)));</code> | 失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。 |
| 15 | 1 / 条件判断 | <code>(newPeer == peer && newTag == tag)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(bootstrapNetRecv(&sock, ((char*)data), size));</code> | 失败时可能提前return，成功才继续；读取控制消息长度并检查接收容量，再取出完整消息，防止按错误尺寸解释元数据。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>close(sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 20 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(unexpectedEnqueue(state, newPeer, newTag, &sock));</code> | 失败时可能提前return，成功才继续；保存暂时不匹配当前 peer/tag 的控制连接，避免先到消息被错误接收。 |
| 21 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct bootstrapState* state = (struct bootstrapState*)commState;

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
```

</details>

<a id="f0046"></a>

## F0046：bootstrapClose

用途：在正常生命周期结束时关闭 bootstrap socket 并回收地址表、暂存项和状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct bootstrapState* state = (struct bootstrapState*)commState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(state-&gt;unexpectedConnections != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("Unexpected connections are not empty");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>close(state-&gt;listenSock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>close(state-&gt;ringSendSocket.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>close(state-&gt;ringRecvSocket.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>free(state-&gt;peerCommAddresses);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>free(state);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f0047"></a>

## F0047：bootstrapAbort

用途：异常退出时关闭控制网络并回收状态，配合中止流程打破初始化等待。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct bootstrapState* state = (struct bootstrapState*)commState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(commState == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(state-&gt;listenSock.fd)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>close(state-&gt;listenSock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 条件判断 | <code>(state-&gt;ringSendSocket.fd)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>close(state-&gt;ringSendSocket.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 0 / 条件判断 | <code>(state-&gt;ringRecvSocket.fd)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>close(state-&gt;ringRecvSocket.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>free(state-&gt;peerCommAddresses);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>free(state-&gt;peerProxyAddresses);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>free(state);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

