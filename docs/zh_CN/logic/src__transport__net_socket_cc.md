# src/transport/net_socket.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f1130"></a>

## F1130：ncclSocketGetPciPath

用途：读取网络接口的 PCI 路径，尽量把 TCP 后端设备纳入统一硬件拓扑。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char devicePath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>snprintf(devicePath, PATH_MAX, "/sys/class/net/%s/device", devName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*pciPath = realpath(devicePath, NULL);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char devicePath[PATH_MAX];
  snprintf(devicePath, PATH_MAX, "/sys/class/net/%s/device", devName);
  // May return NULL if the file doesn't exist.
  *pciPath = realpath(devicePath, NULL);
  return ncclSuccess;
}
```

</details>

<a id="f1131"></a>

## F1131：ncclSocketInit

用途：本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(ncclNetIfs == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&ncclSocketLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 条件判断 | <code>(ncclNetIfs == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 准备局部状态 | <code>char names[MAX_IF_NAME_SIZE*MAX_IFS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 4 / 准备局部状态 | <code>union ncclSocketAddress addrs[MAX_IFS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>ncclNetIfs = ncclFindInterfaces(names, addrs, MAX_IF_NAME_SIZE, MAX_IFS);</code> | 按显式接口配置或默认优先级选择可用接口，过滤不合适的 loopback/docker 等候选。 |
| 9 | 4 / 条件判断 | <code>(ncclNetIfs &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 6 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : no interface found");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 6 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 14 | 6 / 编译期分支/指令 | <code>#define MAX_LINE_LEN (2047)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 15 | 6 / 准备局部状态 | <code>char line[MAX_LINE_LEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 6 / 准备局部状态 | <code>char addrline[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 6 / 调用 / 计算 / 写入 | <code>line[0] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 6 / 调用 / 计算 / 写入 | <code>addrline[SOCKET_NAME_MAXLEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 6 / 循环 | <code>初始 int i=0;；继续条件 i&lt;ncclNetIfs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 20 | 7 / 调用 / 计算 / 写入 | <code>strcpy(ncclSocketDevs[i].devName, names+i*MAX_IF_NAME_SIZE);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 7 / 调用 / 计算 / 写入 | <code>memcpy(&ncclSocketDevs[i].addr, addrs+i, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 7 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketGetPciPath(ncclSocketDevs[i].devName, &ncclSocketDevs[i].pciPath));</code> | 失败时可能提前return，成功才继续；读取网络接口的 PCI 路径，尽量把 TCP 后端设备纳入统一硬件拓扑。 |
| 23 | 7 / 调用 / 计算 / 写入 | <code>snprintf(line+strlen(line), MAX_LINE_LEN-strlen(line), " [%d]%s:%s", i, names+i*MAX_IF_NAME_SIZE, ncclSocketToString(&addrs[i], addrline));</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 24 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 25 | 6 / 调用 / 计算 / 写入 | <code>line[MAX_LINE_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 6 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT&#124;NCCL_NET,"NET/Socket : Using%s", line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&ncclSocketLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 28 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (ncclNetIfs == -1) {
    pthread_mutex_lock(&ncclSocketLock);
    if (ncclNetIfs == -1) {
      char names[MAX_IF_NAME_SIZE*MAX_IFS];
      union ncclSocketAddress addrs[MAX_IFS];
      ncclNetIfs = ncclFindInterfaces(names, addrs, MAX_IF_NAME_SIZE, MAX_IFS);
      if (ncclNetIfs <= 0) {
        WARN("NET/Socket : no interface found");
        return ncclInternalError;
      } else {
        #define MAX_LINE_LEN (2047)
        char line[MAX_LINE_LEN+1];
        char addrline[SOCKET_NAME_MAXLEN+1];
        line[0] = '\0';
        addrline[SOCKET_NAME_MAXLEN] = '\0';
        for (int i=0; i<ncclNetIfs; i++) {
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
```

</details>

<a id="f1132"></a>

## F1132：ncclSocketDevices

用途：报告 Socket 后端发现并筛选后的网络接口数量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*ndev = ncclNetIfs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *ndev = ncclNetIfs;
  return ncclSuccess;
}
```

</details>

<a id="f1133"></a>

## F1133：ncclSocketGetSpeed

用途：从系统接口属性读取链路速率，读取失败时按实现提供保守估计。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*speed = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 准备局部状态 | <code>char speedPath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>sprintf(speedPath, "/sys/class/net/%s/speed", devName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 准备局部状态 | <code>int fd = open(speedPath, O_RDONLY);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>(fd != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 准备局部状态 | <code>char speedStr[] = " ";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 条件判断 | <code>(read(fd, speedStr, sizeof(speedStr)-1) &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 4 / 调用 / 计算 / 写入 | <code>*speed = strtol(speedStr, NULL, 0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>close(fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 条件判断 | <code>(*speed &lt;= 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_NET, "Could not get speed from %s. Defaulting to 10 Gbps.", speedPath);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>*speed = 10000;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *speed = 0;
  char speedPath[PATH_MAX];
  sprintf(speedPath, "/sys/class/net/%s/speed", devName);
  int fd = open(speedPath, O_RDONLY);
  if (fd != -1) {
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
```

</details>

<a id="f1134"></a>

## F1134：ncclSocketGetProperties

用途：向统一网络层报告 TCP 接口名字/PCI/速度及只接受主机指针等能力。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;name = ncclSocketDevs[dev].devName;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;pciPath = ncclSocketDevs[dev].pciPath;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;guid = dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;ptrSupport = NCCL_PTR_HOST;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketGetSpeed(props-&gt;name, &props-&gt;speed));</code> | 失败时可能提前return，成功才继续；从系统接口属性读取链路速率，读取失败时按实现提供保守估计。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;latency = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;port = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;maxComms = 65536;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>props-&gt;maxRecvs = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
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
```

</details>

<a id="f1135"></a>

## F1135：GetSocketAddr

用途：为所选后端设备获得建链控制 socket 地址；控制握手地址与实际数据载荷通路不同。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(dev &gt;= ncclNetIfs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>memcpy(addr, &ncclSocketDevs[dev].addr, sizeof(*addr));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (dev >= ncclNetIfs) return ncclInternalError;
  memcpy(addr, &ncclSocketDevs[dev].addr, sizeof(*addr));
  return ncclSuccess;
}
```

</details>

<a id="f1136"></a>

## F1136：persistentSocketThread

用途：后台交错推进分配到本线程的 TCP 子任务，更新每个任务的传输 offset 直到完成或停止。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocketThreadResources* resource = (struct ncclSocketThreadResources*)args_;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclSocketComm* comm = resource-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclSocketTaskQueue* myQueue = &resource-&gt;threadTaskQueue;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>int nSocksPerThread = comm-&gt;nSocks / comm-&gt;nThreads;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 循环 | <code>初始 ；继续条件 (1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 准备局部状态 | <code>int idle = 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 1 / 准备局部状态 | <code>int mark = myQueue-&gt;next;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 循环 | <code>初始 int i=0;；继续条件 i&lt;myQueue-&gt;len；每轮更新 i+=nSocksPerThread</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 9 | 2 / 准备局部状态 | <code>int repeat;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 先执行一次的循环 | <code>初始 ；继续条件 (repeat)；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>repeat = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 循环 | <code>初始 int j=0;；继续条件 j&lt;nSocksPerThread；每轮更新 j++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 13 | 4 / 准备局部状态 | <code>struct ncclSocketTask* r = myQueue-&gt;tasks+i+j;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 4 / 条件判断 | <code>(r != NULL && r-&gt;used == 1 && r-&gt;offset &lt; r-&gt;size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 6 / 调用 / 计算 / 写入 | <code>r-&gt;result = ncclSocketProgress(r-&gt;op, r-&gt;sock, r-&gt;data, r-&gt;size, &r-&gt;offset);</code> | 调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 17 | 6 / 条件判断 | <code>(r-&gt;result != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 8 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : socket progress error");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 8 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 21 | 6 / 调用 / 计算 / 写入 | <code>idle = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 6 / 条件判断 | <code>(r-&gt;offset &lt; r-&gt;size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 8 / 调用 / 计算 / 写入 | <code>repeat = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 26 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 28 | 1 / 条件判断 | <code>(idle)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&resource-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 3 / 循环 | <code>初始 ；继续条件 (mark == myQueue-&gt;next && resource-&gt;stop == 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 32 | 4 / 调用 / 计算 / 写入 | <code>pthread_cond_wait(&resource-&gt;threadCond, &resource-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 34 | 3 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&resource-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 35 | 1 / 条件判断 | <code>(resource-&gt;stop)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 3 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 38 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocketThreadResources* resource = (struct ncclSocketThreadResources*)args_;
  struct ncclSocketComm* comm = resource->comm;
  struct ncclSocketTaskQueue* myQueue = &resource->threadTaskQueue;
  int nSocksPerThread = comm->nSocks / comm->nThreads;
  while (1) {
    int idle = 1;
    int mark = myQueue->next; // mark newest task seen
    for (int i=0; i<myQueue->len; i+=nSocksPerThread) {
      int repeat;
      do {
        repeat = 0;
        for (int j=0; j<nSocksPerThread; j++) {
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
```

</details>

<a id="f1137"></a>

## F1137：ncclSocketGetNsockNthread

用途：结合参数和平台调优选择每连接 socket 数与后台线程数，平衡并发带宽和 CPU 开销。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int nSocksPerThread = ncclParamSocketNsocksPerThread();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int nThreads = ncclParamSocketNthreads();</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(nThreads &gt; MAX_THREADS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : NCCL_SOCKET_NTHREADS is greater than the maximum allowed, setting to %d", MAX_THREADS);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>nThreads = MAX_THREADS;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 条件判断 | <code>(nThreads == -2 &#124;&#124; nSocksPerThread == -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 准备局部状态 | <code>int autoNt=0, autoNs=1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 准备局部状态 | <code>char vendorPath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>snprintf(vendorPath, PATH_MAX, "/sys/class/net/%s/device/vendor", ncclSocketDevs[dev].devName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 2 / 准备局部状态 | <code>char* rPath = realpath(vendorPath, NULL);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 13 | 2 / 准备局部状态 | <code>int fd = open(rPath, O_RDONLY);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>free(rPath);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 2 / 条件判断 | <code>(fd == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_NET, "Open of %s failed : %s", vendorPath, strerror(errno));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 4 / 控制转移 | <code>goto end;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 19 | 2 / 准备局部状态 | <code>char vendor[7];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>strncpy(vendor, "0x0000", 7);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 2 / 准备局部状态 | <code>int len;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>SYSCHECKVAL(read(fd, vendor, 6), "read", len);</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>SYSCHECK(close(fd), "close");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 24 | 2 / 条件判断 | <code>(strcmp(vendor, "0x1d0f") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>autoNt = 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 4 / 调用 / 计算 / 写入 | <code>autoNs = 8;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 29 | 4 / 条件判断 | <code>(strcmp(vendor, "0x1ae0") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 6 / 调用 / 计算 / 写入 | <code>autoNt = 4;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 6 / 调用 / 计算 / 写入 | <code>autoNs = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 2 / 跳转标签 | <code>end</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 34 | 3 / 条件判断 | <code>(nThreads == -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 5 / 调用 / 计算 / 写入 | <code>nThreads = autoNt;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 2 / 条件判断 | <code>(nSocksPerThread == -2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 4 / 调用 / 计算 / 写入 | <code>nSocksPerThread = autoNs;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 0 / 准备局部状态 | <code>int nSocks = nSocksPerThread * nThreads;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 0 / 条件判断 | <code>(nSocks &gt; MAX_SOCKETS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 2 / 调用 / 计算 / 写入 | <code>nSocksPerThread = MAX_SOCKETS/nThreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : the total number of sockets is greater than the maximum allowed, setting NCCL_NSOCKS_PERTHREAD to %d", nSocksPerThread);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 2 / 调用 / 计算 / 写入 | <code>nSocks = nSocksPerThread * nThreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>*ns = nSocks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>*nt = nThreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 条件判断 | <code>(nSocks &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 49 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 50 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_INIT, "NET/Socket: Using %d threads and %d sockets per thread", nThreads, nSocksPerThread);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 51 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int nSocksPerThread = ncclParamSocketNsocksPerThread();
  int nThreads = ncclParamSocketNthreads();
  if (nThreads > MAX_THREADS) {
    WARN("NET/Socket : NCCL_SOCKET_NTHREADS is greater than the maximum allowed, setting to %d", MAX_THREADS);
    nThreads = MAX_THREADS;
  }
  if (nThreads == -2 || nSocksPerThread == -2) {
    // Auto-detection
    int autoNt=0, autoNs=1; // By default, we only use the main thread and do not spawn extra threads
    char vendorPath[PATH_MAX];
    snprintf(vendorPath, PATH_MAX, "/sys/class/net/%s/device/vendor", ncclSocketDevs[dev].devName);
    char* rPath = realpath(vendorPath, NULL);
    int fd = open(rPath, O_RDONLY);
    free(rPath);
    if (fd == -1) {
      // Could not find device vendor. This is handled silently so
      // we don't want to print an INFO error.
      TRACE(NCCL_NET, "Open of %s failed : %s", vendorPath, strerror(errno));
      goto end;
    }
    char vendor[7];
    strncpy(vendor, "0x0000", 7);
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
```

</details>

<a id="f1138"></a>

## F1138：ncclSocketNewListenComm

用途：分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(comm, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>(*comm)-&gt;sock.fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclCalloc(comm, 1));
  (*comm)->sock.fd = -1;
  return ncclSuccess;
}
```

</details>

<a id="f1139"></a>

## F1139：ncclSocketNewComm

用途：分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(comm, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>(*comm)-&gt;ctrlSock.fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 循环 | <code>初始 int i=0;；继续条件 i &lt; MAX_SOCKETS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>(*comm)-&gt;socks[i].fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>(*comm)-&gt;nextSock = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclCalloc(comm, 1));
  (*comm)->ctrlSock.fd = -1;
  for (int i=0; i < MAX_SOCKETS; i++) {
    (*comm)->socks[i].fd = -1;
  }
  (*comm)->nextSock = 0;
  return ncclSuccess;
}
```

</details>

<a id="f1140"></a>

## F1140：ncclSocketListen

用途：按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(dev &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>struct ncclSocketHandle* handle = (struct ncclSocketHandle*) opaqueHandle;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>memset(handle, 0, sizeof(struct ncclSocketHandle));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 源码语法片段 | <code>static_assert(sizeof(struct ncclSocketHandle) &lt;= NCCL_NET_HANDLE_MAXSIZE, "ncclSocketHandle size too large");</code> | 按紧邻完整函数体解释此片段；不能仅凭片段推断跨线程执行次序。 |
| 7 | 0 / 准备局部状态 | <code>struct ncclSocketListenComm* comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketNewListenComm(&comm));</code> | 失败时可能提前return，成功才继续；分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(GetSocketAddr(dev, &comm-&gt;sock.addr));</code> | 失败时可能提前return，成功才继续；为所选后端设备获得建链控制 socket 地址；控制握手地址与实际数据载荷通路不同。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketListen(&comm-&gt;sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件创建基础监听 socket 或 TCP 后端 listenComm，生成对端可连接句柄。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>memcpy(&handle-&gt;connectAddr, &comm-&gt;sock.addr, sizeof(union ncclSocketAddress));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketGetNsockNthread(dev, &comm-&gt;nSocks, &comm-&gt;nThreads));</code> | 失败时可能提前return，成功才继续；结合参数和平台调优选择每连接 socket 数与后台线程数，平衡并发带宽和 CPU 开销。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>handle-&gt;nSocks = comm-&gt;nSocks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>handle-&gt;nThreads = comm-&gt;nThreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;sock.asyncFlag = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;dev = dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>*listenComm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (dev < 0) { // data transfer socket is based on specified dev
    return ncclInternalError;
  }
  struct ncclSocketHandle* handle = (struct ncclSocketHandle*) opaqueHandle;
  memset(handle, 0, sizeof(struct ncclSocketHandle));
  static_assert(sizeof(struct ncclSocketHandle) <= NCCL_NET_HANDLE_MAXSIZE, "ncclSocketHandle size too large");
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
```

</details>

<a id="f1141"></a>

## F1141：ncclSocketConnect

用途：按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(dev &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>enum ncclSocketState conState;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclSocketHandle* handle = (struct ncclSocketHandle*) opaqueHandle;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>struct ncclSocketCommStage* stage = &handle-&gt;stage;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>struct ncclSocketComm* comm = stage-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>uint8_t i = stage-&gt;iteration;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>struct ncclSocket* sock = stage-&gt;sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>*sendComm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 条件判断 | <code>(stage-&gt;state == ncclSocketCommStateConnect)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 2 / 控制转移 | <code>goto socket_connect_check;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 14 | 0 / 条件判断 | <code>(stage-&gt;state == ncclSocketCommStateSend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 2 / 控制转移 | <code>goto socket_send;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketNewComm(&comm));</code> | 失败时可能提前return，成功才继续；分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;nSocks = handle-&gt;nSocks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;nThreads = handle-&gt;nThreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>comm-&gt;dev = dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&comm-&gt;cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 23 | 0 / 循环 | <code>初始 ；继续条件 i&lt;comm-&gt;nSocks+1；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>sock = i == comm-&gt;nSocks ? &comm-&gt;ctrlSock : comm-&gt;socks+i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketInit(sock, &handle-&gt;connectAddr, NULL, 1));</code> | 失败时可能提前return，成功才继续；本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>stage-&gt;sock = sock;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 1 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclSocketCommStateConnect;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 1 / 调用 / 计算 / 写入 | <code>stage-&gt;iteration = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketConnect(sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件推进基础 socket 连接或 TCP 多 socket 后端握手，支持异步未就绪返回。 |
| 30 | 1 / 跳转标签 | <code>socket_connect_check</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 31 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclGetSocketState(sock, &conState));</code> | 失败时可能提前return，成功才继续；检查非阻塞 socket 的连接状态及系统错误，区分仍在连接、已连接或失败。 |
| 32 | 1 / 条件判断 | <code>(conState == ncclSocketConnecting)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 33 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 34 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 35 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 36 | 3 / 条件判断 | <code>(conState == ncclSocketError)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 5 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 39 | 1 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclSocketCommStateSend;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 1 / 跳转标签 | <code>socket_send</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 41 | 2 / 准备局部状态 | <code>int done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 42 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, sock, &i, sizeof(uint8_t), &done));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 43 | 1 / 条件判断 | <code>(done == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 46 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>*sendComm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (dev < 0) { // data transfer socket is based on specified dev
    return ncclInternalError;
  }

  enum ncclSocketState conState;
  struct ncclSocketHandle* handle = (struct ncclSocketHandle*) opaqueHandle;
  struct ncclSocketCommStage* stage = &handle->stage;
  struct ncclSocketComm* comm = stage->comm;
  uint8_t i = stage->iteration;
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
    int done = 0;
    NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, sock, &i, sizeof(uint8_t), &done));
    if (done == 0) return ncclSuccess;
  }
  *sendComm = comm;
  return ncclSuccess;
}
```

</details>

<a id="f1142"></a>

## F1142：ncclSocketAccept

用途：按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocketListenComm* lComm = (struct ncclSocketListenComm*)listenComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclSocketCommStage* stage = &lComm-&gt;stage;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclSocketComm* rComm = stage-&gt;comm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>uint8_t i = stage-&gt;iteration;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclSocket* sock = stage-&gt;sock;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>*recvComm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 条件判断 | <code>(stage-&gt;state == ncclSocketCommStateAccept)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 控制转移 | <code>goto socket_accept;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 10 | 0 / 条件判断 | <code>(stage-&gt;state == ncclSocketCommStateRecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 控制转移 | <code>goto socket_recv;</code> | 跳到明确标签，不再顺序执行当前位置后面的项；通常用于统一清理或复用计算段。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketNewComm(&rComm));</code> | 失败时可能提前return，成功才继续；分配并清零 TCP 后端监听/通信对象，设置请求池及状态初值。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;comm = rComm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;nSocks = lComm-&gt;nSocks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;nThreads = lComm-&gt;nThreads;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 调用 / 计算 / 写入 | <code>rComm-&gt;dev = lComm-&gt;dev;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDevice(&rComm-&gt;cudaDev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>lComm-&gt;sock.asyncFlag = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 循环 | <code>初始 ；继续条件 i&lt;rComm-&gt;nSocks+1；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 21 | 1 / 准备局部状态 | <code>uint8_t sendSockIdx;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 1 / 调用 / 计算 / 写入 | <code>ncclCalloc(&sock, 1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketInit(sock, NULL, NULL, 1));</code> | 失败时可能提前return，成功才继续；本文件定义决定是基础 socket 对象初始化还是 TCP 后端接口枚举，请结合参数和所属文件阅读。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>stage-&gt;sock = sock;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclSocketCommStateAccept;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>stage-&gt;iteration = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 1 / 跳转标签 | <code>socket_accept</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketAccept(sock, &lComm-&gt;sock));</code> | 失败时可能提前return，成功才继续；按该函数所属文件接收基础连接或组成 TCP 后端多 socket 通信对象，保留握手阶段状态。 |
| 29 | 1 / 条件判断 | <code>(sock-&gt;fd == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 32 | 1 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclSocketCommStateRecv;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 33 | 1 / 跳转标签 | <code>socket_recv</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 34 | 2 / 准备局部状态 | <code>int done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 35 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, sock, &sendSockIdx, sizeof(uint8_t), &done));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 36 | 1 / 条件判断 | <code>(done == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 39 | 1 / 条件判断 | <code>(sendSockIdx == rComm-&gt;nSocks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 40 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 41 | 3 / 调用 / 计算 / 写入 | <code>memcpy(&rComm-&gt;ctrlSock, sock, sizeof(struct ncclSocket));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 42 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 43 | 3 / 调用 / 计算 / 写入 | <code>memcpy(rComm-&gt;socks+sendSockIdx, sock, sizeof(struct ncclSocket));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 1 / 调用 / 计算 / 写入 | <code>free(sock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 46 | 0 / 调用 / 计算 / 写入 | <code>*recvComm = rComm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 47 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;state = ncclSocketCommStateStart;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 48 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;iteration = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;sock = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 0 / 调用 / 计算 / 写入 | <code>stage-&gt;comm = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 51 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocketListenComm* lComm = (struct ncclSocketListenComm*)listenComm;
  struct ncclSocketCommStage* stage = &lComm->stage;
  struct ncclSocketComm* rComm = stage->comm;
  uint8_t i = stage->iteration;
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
```

</details>

<a id="f1143"></a>

## F1143：ncclSocketGetRequest

用途：从 TCP 后端固定请求表申请一项记录消息方向、地址与总长度的状态。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;MAX_REQUESTS；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclSocketRequest* r = comm-&gt;requests+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 1 / 条件判断 | <code>(r-&gt;used == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;op = op;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;data = data;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;size = size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;ctrlSock = &comm-&gt;ctrlSock;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;used = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>r-&gt;nSubs = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>*req = r;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : unable to allocate requests");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int i=0; i<MAX_REQUESTS; i++) {
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
```

</details>

<a id="f1144"></a>

## F1144：ncclSocketGetTask

用途：把请求划为 TCP 子任务并交给相应 socket/线程，复用任务队列与后台线程。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int tid = comm-&gt;nextSock % comm-&gt;nThreads;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclSocketThreadResources* res = comm-&gt;threadResources+tid;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>struct ncclSocketTaskQueue* queue = &res-&gt;threadTaskQueue;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(queue-&gt;tasks == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>queue-&gt;len = MAX_REQUESTS * DIVUP(comm-&gt;nSocks, comm-&gt;nThreads);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&queue-&gt;tasks, queue-&gt;len));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>queue-&gt;next = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>res-&gt;comm = comm;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_init(&res-&gt;threadLock, NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>pthread_cond_init(&res-&gt;threadCond, NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>pthread_create(comm-&gt;helperThread+tid, NULL, persistentSocketThread, res);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>ncclSetThreadName(comm-&gt;helperThread[tid], "NCCL Sock%c%1u%2u%2u", op == NCCL_SOCKET_SEND ? 'S' : 'R', comm-&gt;dev, tid, comm-&gt;cudaDev);</code> | 为后台线程设置便于系统工具识别的名称，方便区分 Proxy 服务与数据推进线程。 |
| 14 | 0 / 准备局部状态 | <code>struct ncclSocketTask* r = queue-&gt;tasks+queue-&gt;next;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 条件判断 | <code>(r-&gt;used == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;op = op;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;data = data;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;size = size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;sock = comm-&gt;socks+comm-&gt;nextSock;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;offset = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;result = ncclSuccess;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>comm-&gt;nextSock = (comm-&gt;nextSock + 1) % comm-&gt;nSocks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;used = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>*req = r;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>queue-&gt;next = (queue-&gt;next+1)%queue-&gt;len;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 2 / 调用 / 计算 / 写入 | <code>pthread_cond_signal(&res-&gt;threadCond);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 29 | 2 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 30 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : unable to allocate subtasks");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int tid = comm->nextSock % comm->nThreads;
  struct ncclSocketThreadResources* res = comm->threadResources+tid;
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
```

</details>

<a id="f1145"></a>

## F1145：ncclSocketTest

用途：推进控制长度交换并检查所有子任务 offset，直到一条上层异步请求完成。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*done = 0;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclSocketRequest *r = (struct ncclSocketRequest*)request;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(r == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : test called with NULL request");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 条件判断 | <code>(r-&gt;used == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 准备局部状态 | <code>int data = r-&gt;size;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 2 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(r-&gt;op, r-&gt;ctrlSock, &data, sizeof(int), &offset));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 12 | 2 / 条件判断 | <code>(offset == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 15 | 2 / 条件判断 | <code>(offset &lt; sizeof(int))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketWait(r-&gt;op, r-&gt;ctrlSock, &data, sizeof(int), &offset));</code> | 失败时可能提前return，成功才继续；重复推进直到指定长度完成，等待过程中继续处理临时错误和中止。 |
| 18 | 2 / 条件判断 | <code>(r-&gt;op == NCCL_SOCKET_RECV && data &gt; r-&gt;size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 4 / 准备局部状态 | <code>char line[SOCKET_NAME_MAXLEN+1];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>WARN("NET/Socket : peer %s message truncated : receiving %d bytes instead of %d. If you believe your socket network is in healthy state, \ there may be a mismatch in collective sizes or environment settings (e.g. NCCL_PROTO, NCCL_ALGO) between ranks", ncclSocketToString(&r-&gt;ctrlSock-&gt;addr, line), data, r-&gt;size);</code> | 把 IPv4/IPv6 socket 地址和端口格式化为可读文本，用于日志与错误定位。 |
| 22 | 4 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;size = data;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;offset = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;used = 2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 2 / 准备局部状态 | <code>int chunkOffset = 0, i = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 27 | 2 / 条件判断 | <code>(r-&gt;comm-&gt;nSocks &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 4 / 准备局部状态 | <code>int taskSize = std::max(MIN_CHUNKSIZE, DIVUP(r-&gt;size, r-&gt;comm-&gt;nSocks));</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 4 / 循环 | <code>初始 ；继续条件 (chunkOffset &lt; r-&gt;size)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 31 | 5 / 准备局部状态 | <code>int chunkSize = std::min(taskSize, r-&gt;size-chunkOffset);</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 32 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketGetTask(r-&gt;comm, r-&gt;op, (char*)(r-&gt;data)+chunkOffset, chunkSize, r-&gt;tasks+i++));</code> | 失败时可能提前return，成功才继续；把请求划为 TCP 子任务并交给相应 socket/线程，复用任务队列与后台线程。 |
| 33 | 5 / 调用 / 计算 / 写入 | <code>chunkOffset += chunkSize;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 34 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 35 | 2 / 调用 / 计算 / 写入 | <code>r-&gt;nSubs = i;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 0 / 条件判断 | <code>(r-&gt;used == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 37 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 38 | 2 / 条件判断 | <code>(r-&gt;nSubs &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 4 / 准备局部状态 | <code>int nCompleted = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 4 / 循环 | <code>初始 int i=0;；继续条件 i&lt;r-&gt;nSubs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 42 | 5 / 准备局部状态 | <code>struct ncclSocketTask* sub = r-&gt;tasks[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 5 / 条件判断 | <code>(sub-&gt;result != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 7 / 函数返回 | <code>return sub-&gt;result;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 46 | 5 / 条件判断 | <code>(sub-&gt;offset == sub-&gt;size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 7 / 调用 / 计算 / 写入 | <code>nCompleted++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 49 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 50 | 4 / 条件判断 | <code>(nCompleted == r-&gt;nSubs)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 51 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 52 | 6 / 条件判断 | <code>(size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 8 / 调用 / 计算 / 写入 | <code>*size = r-&gt;size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 55 | 6 / 调用 / 计算 / 写入 | <code>*done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 56 | 6 / 调用 / 计算 / 写入 | <code>r-&gt;used = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 57 | 6 / 循环 | <code>初始 int i=0;；继续条件 i&lt;r-&gt;nSubs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 58 | 7 / 准备局部状态 | <code>struct ncclSocketTask* sub = r-&gt;tasks[i];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 7 / 调用 / 计算 / 写入 | <code>sub-&gt;used = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 60 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 61 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 62 | 4 / 条件判断 | <code>(r-&gt;offset &lt; r-&gt;size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 63 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 64 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketProgress(r-&gt;op, r-&gt;ctrlSock, r-&gt;data, r-&gt;size, &r-&gt;offset));</code> | 失败时可能提前return，成功才继续；调用非阻塞进度基础逻辑推进一段字节流，保留 offset 供后续调用接着传。 |
| 65 | 4 / 条件判断 | <code>(r-&gt;offset == r-&gt;size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 66 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 67 | 6 / 条件判断 | <code>(size)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 7 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 8 / 调用 / 计算 / 写入 | <code>*size = r-&gt;size;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 70 | 6 / 调用 / 计算 / 写入 | <code>*done = 1;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 71 | 6 / 调用 / 计算 / 写入 | <code>r-&gt;used = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 72 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *done = 0;
  struct ncclSocketRequest *r = (struct ncclSocketRequest*)request;
  if (r == NULL) {
    WARN("NET/Socket : test called with NULL request");
    return ncclInternalError;
  }
  if (r->used == 1) { /* try to send/recv size */
    int data = r->size;
    int offset = 0;
    NCCLCHECK(ncclSocketProgress(r->op, r->ctrlSock, &data, sizeof(int), &offset));

    if (offset == 0) return ncclSuccess; /* Not ready -- retry later */

    // Not sure we could ever receive less than 4 bytes, but just in case ...
    if (offset < sizeof(int)) NCCLCHECK(ncclSocketWait(r->op, r->ctrlSock, &data, sizeof(int), &offset));

    // Check size is less or equal to the size provided by the user
    if (r->op == NCCL_SOCKET_RECV && data > r->size) {
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
    int chunkOffset = 0, i = 0;
    if (r->comm->nSocks > 0) {
      // each request can be divided up to nSocks tasks
      int taskSize = std::max(MIN_CHUNKSIZE, DIVUP(r->size, r->comm->nSocks));
      while (chunkOffset < r->size) {
        int chunkSize = std::min(taskSize, r->size-chunkOffset);
        NCCLCHECK(ncclSocketGetTask(r->comm, r->op, (char*)(r->data)+chunkOffset, chunkSize, r->tasks+i++));
        chunkOffset += chunkSize;
      }
    }
    r->nSubs = i;
  }
  if (r->used == 2) { // already exchanged size
    if (r->nSubs > 0) {
      int nCompleted = 0;
      for (int i=0; i<r->nSubs; i++) {
        struct ncclSocketTask* sub = r->tasks[i];
        if (sub->result != ncclSuccess) return sub->result;
        if (sub->offset == sub->size) nCompleted++;
      }
      if (nCompleted == r->nSubs) {
        if (size) *size = r->size;
        *done = 1;
        r->used = 0;
        for (int i=0; i<r->nSubs; i++) {
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
```

</details>

<a id="f1146"></a>

## F1146：ncclSocketRegMr

用途：仅验证主机指针类型，TCP 后端无需创建 verbs MR；不支持直接传 GPU 指针。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return (type != NCCL_PTR_HOST) ? ncclInternalError : ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  return (type != NCCL_PTR_HOST) ? ncclInternalError : ncclSuccess;
}
```

</details>

<a id="f1147"></a>

## F1147：ncclSocketDeregMr

用途：TCP 后端没有真实 verbs MR，可直接成功完成接口对应的注销动作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return ncclSuccess; }
```

</details>

<a id="f1148"></a>

## F1148：ncclSocketIsend

用途：为 TCP 消息分配异步请求，数据在后续 Test/线程推进中完成，不保证本调用返回时已传完。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocketComm* comm = (struct ncclSocketComm*)sendComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketGetRequest(comm, NCCL_SOCKET_SEND, data, size, (struct ncclSocketRequest**)request));</code> | 失败时可能提前return，成功才继续；从 TCP 后端固定请求表申请一项记录消息方向、地址与总长度的状态。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocketComm* comm = (struct ncclSocketComm*)sendComm;
  NCCLCHECK(ncclSocketGetRequest(comm, NCCL_SOCKET_SEND, data, size, (struct ncclSocketRequest**)request));
  return ncclSuccess;
}
```

</details>

<a id="f1149"></a>

## F1149：ncclSocketIrecv

用途：为 TCP 消息分配异步请求，数据在后续 Test/线程推进中完成，不保证本调用返回时已传完。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocketComm* comm = (struct ncclSocketComm*)recvComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(n != 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclSocketGetRequest(comm, NCCL_SOCKET_RECV, data[0], sizes[0], (struct ncclSocketRequest**)request));</code> | 失败时可能提前return，成功才继续；从 TCP 后端固定请求表申请一项记录消息方向、地址与总长度的状态。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocketComm* comm = (struct ncclSocketComm*)recvComm;
  if (n != 1) return ncclInternalError;
  NCCLCHECK(ncclSocketGetRequest(comm, NCCL_SOCKET_RECV, data[0], sizes[0], (struct ncclSocketRequest**)request));
  return ncclSuccess;
}
```

</details>

<a id="f1150"></a>

## F1150：ncclSocketIflush

用途：TCP 主机缓冲路径不需 GPU RDMA flush，对应接口立即完成而不创建额外硬件请求。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // We don't support CUDA pointers, so we don't need a flush operation
  return ncclInternalError;
}
```

</details>

<a id="f1151"></a>

## F1151：ncclSocketCloseListen

用途：关闭 TCP 后端监听 socket 并释放其监听对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocketListenComm* comm = (struct ncclSocketListenComm*)opaqueComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 条件判断 | <code>(comm-&gt;sock.fd != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 调用 / 计算 / 写入 | <code>close(comm-&gt;sock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>free(comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocketListenComm* comm = (struct ncclSocketListenComm*)opaqueComm;
  if (comm) {
    if (comm->sock.fd != -1) close(comm->sock.fd);
    free(comm);
  }
  return ncclSuccess;
}
```

</details>

<a id="f1152"></a>

## F1152：ncclSocketClose

用途：通知 TCP 通信后台线程停止，关闭各数据/控制 socket 并释放任务、请求及线程资源。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclSocketComm* comm = (struct ncclSocketComm*)opaqueComm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(comm)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;nThreads；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 3 / 准备局部状态 | <code>struct ncclSocketThreadResources* res = comm-&gt;threadResources+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 3 / 条件判断 | <code>(comm-&gt;helperThread[i])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 5 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 5 / 调用 / 计算 / 写入 | <code>res-&gt;stop = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 5 / 调用 / 计算 / 写入 | <code>pthread_cond_signal(&res-&gt;threadCond);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 5 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&res-&gt;threadLock);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 5 / 调用 / 计算 / 写入 | <code>pthread_join(comm-&gt;helperThread[i], NULL);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 3 / 调用 / 计算 / 写入 | <code>free(res-&gt;threadTaskQueue.tasks);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 15 | 2 / 条件判断 | <code>(comm-&gt;ctrlSock.fd != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>close(comm-&gt;ctrlSock.fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;comm-&gt;nSocks；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 3 / 条件判断 | <code>(comm-&gt;socks[i].fd != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 5 / 调用 / 计算 / 写入 | <code>close(comm-&gt;socks[i].fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 22 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>free(comm);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclSocketComm* comm = (struct ncclSocketComm*)opaqueComm;
  if (comm) {
    for (int i=0; i<comm->nThreads; i++) {
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
    for (int i=0; i<comm->nSocks; i++) {
      if (comm->socks[i].fd != -1) close(comm->socks[i].fd);
    }
    free(comm);
  }
  return ncclSuccess;
}
```

</details>

