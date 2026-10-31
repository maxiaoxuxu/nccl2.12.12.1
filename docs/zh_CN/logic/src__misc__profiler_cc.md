# src/misc/profiler.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0961"></a>

## F0961：ncclProfilingRecord

用途：若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(profilingEvents == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&profilingEvents, MAX_EVENTS));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>profilingStart = gettime();</code> | 读取本平台的轻量时间源/计数器并转成 profiling 需要的时间表示。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclProxyProfileEvent* event = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(state%8 == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 条件判断 | <code>(profilingIndex == MAX_EVENTS)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 4 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>args-&gt;subs[sub].profilingEvents[step%NCCL_STEPS] = event = profilingEvents+profilingIndex++;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 12 | 2 / 条件判断 | <code>(state == ncclProxyProfileBegin)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>event-&gt;opCount = args-&gt;opCount;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>event-&gt;channel = args-&gt;subs[sub].channelId;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 4 / 调用 / 计算 / 写入 | <code>event-&gt;peer = args-&gt;subs[sub].peer;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>event-&gt;type = args-&gt;pattern;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 4 / 调用 / 计算 / 写入 | <code>event-&gt;step = step;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 19 | 4 / 调用 / 计算 / 写入 | <code>event-&gt;opIndex = (((uint64_t)args)/sizeof(struct ncclProxyArgs))%256;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 21 | 4 / 调用 / 计算 / 写入 | <code>event-&gt;peer = -state;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>event = (struct ncclProxyProfileEvent*)args-&gt;subs[sub].profilingEvents[step%NCCL_STEPS];</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 24 | 2 / 条件判断 | <code>(state == ncclProxyProfileEnd)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 25 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 26 | 4 / 调用 / 计算 / 写入 | <code>args-&gt;subs[sub].profilingEvents[step%NCCL_STEPS] = NULL;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 27 | 2 / 条件判断 | <code>(state == ncclProxyProfileAppendEnd)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>event-&gt;opCount = args-&gt;opCount;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>event-&gt;timestamp[state%8] = gettime()-profilingStart;</code> | 读取本平台的轻量时间源/计数器并转成 profiling 需要的时间表示。 |
| 31 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (profilingEvents == NULL) {
    NCCLCHECK(ncclCalloc(&profilingEvents, MAX_EVENTS));
    profilingStart = gettime();
  }
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
```

</details>

<a id="f0962"></a>

## F0962：ncclProfilingDump

用途：导出代理阶段耗时记录，帮助区分 GPU 等待、网络等待和 flush；禁用分支无操作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>static int dumpDone = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(dumpDone)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>dumpDone = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 准备局部状态 | <code>const char* str = getenv("NCCL_PROXY_PROFILE");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 条件判断 | <code>(!str)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>free(profilingEvents);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 准备局部状态 | <code>FILE* f = fopen(str, "w");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>fprintf(f, "[\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;profilingIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 1 / 准备局部状态 | <code>struct ncclProxyProfileEvent* e = profilingEvents+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 1 / 准备局部状态 | <code>const int sendrecv = e-&gt;peer &gt;= 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 16 | 1 / 准备局部状态 | <code>const char* typeStr = sendrecv ? (e-&gt;type == ncclPatternSend ? "Send" : "Recv") : profilingEventStr[-(e-&gt;peer/8)];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 17 | 1 / 条件判断 | <code>(sendrecv)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 3 / 准备局部状态 | <code>int state = ncclProxyProfileBegin;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 20 | 3 / 准备局部状态 | <code>const char** stateStr = e-&gt;type == ncclPatternSend ? profilingStateSendStr : profilingStateRecvStr;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 3 / 调用 / 计算 / 写入 | <code>fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f, \"args\": { \"opCount\": %ld, \"proxyOpIndex\":%d } },\n", typeStr, e-&gt;peer, e-&gt;step, i, e-&gt;channel, e-&gt;timestamp[state], e-&gt;opCount, e-&gt;opIndex);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 22 | 3 / 循环 | <code>初始 ；继续条件 (state&lt;ncclProxyProfileEnd)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 23 | 4 / 条件判断 | <code>(e-&gt;timestamp[state])</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 24 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 25 | 6 / 准备局部状态 | <code>const char* name = stateStr[state];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 26 | 6 / 调用 / 计算 / 写入 | <code>fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n", name, i, e-&gt;channel, e-&gt;timestamp[state]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 6 / 调用 / 计算 / 写入 | <code>state++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 6 / 循环 | <code>初始 ；继续条件 (e-&gt;timestamp[state] == 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 29 | 7 / 调用 / 计算 / 写入 | <code>state++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 30 | 6 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 31 | 6 / 调用 / 计算 / 写入 | <code>fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n", name, i, e-&gt;channel, e-&gt;timestamp[state]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 32 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 33 | 3 / 调用 / 计算 / 写入 | <code>fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f },\n", typeStr, e-&gt;peer, e-&gt;step, i, e-&gt;channel, e-&gt;timestamp[state]);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 34 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 35 | 3 / 条件判断 | <code>(e-&gt;peer == -ncclProxyProfileAppend)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 36 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 37 | 5 / 调用 / 计算 / 写入 | <code>fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f, \"args\": { \"added\": %ld } },\n", typeStr, i, e-&gt;timestamp[0], e-&gt;opCount);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 38 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 39 | 5 / 调用 / 计算 / 写入 | <code>fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f },\n", typeStr, i, e-&gt;timestamp[0]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 40 | 3 / 调用 / 计算 / 写入 | <code>fprintf(f, "{\"name\": \"%s\", \"cat\": \"NET\", \"ph\": \"e\", \"id\": %d, \"pid\": -1, \"tid\": 1, \"ts\": %f },\n", typeStr, i, e-&gt;timestamp[1]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 41 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>fprintf(f, "{} ]\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 43 | 0 / 调用 / 计算 / 写入 | <code>fclose(f);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 44 | 0 / 调用 / 计算 / 写入 | <code>free(profilingEvents);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static int dumpDone = 0;
  if (dumpDone) return;
  dumpDone = 1;
  const char* str = getenv("NCCL_PROXY_PROFILE");
  if (!str) { free(profilingEvents); return; }
  FILE* f = fopen(str, "w");
  fprintf(f, "[\n");

  for (int i=0; i<profilingIndex; i++) {
    struct ncclProxyProfileEvent* e = profilingEvents+i;
    const int sendrecv = e->peer >= 0;
    const char* typeStr = sendrecv ? (e->type == ncclPatternSend ? "Send" : "Recv") :
      profilingEventStr[-(e->peer/8)];


    if (sendrecv) {
      int state = ncclProxyProfileBegin;
      const char** stateStr = e->type == ncclPatternSend ? profilingStateSendStr : profilingStateRecvStr;
      fprintf(f, "{\"name\": \"%s-%d-%d\", \"cat\": \"NET\", \"ph\": \"b\", \"id\": %d, \"pid\": %d, \"tid\": 1, \"ts\": %f, \"args\": { \"opCount\": %ld, \"proxyOpIndex\":%d } },\n",
          typeStr, e->peer, e->step, i, e->channel, e->timestamp[state], e->opCount, e->opIndex);

      while (state<ncclProxyProfileEnd) {
        if (e->timestamp[state]) {
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
```

</details>

<a id="f0963"></a>

## F0963：ncclProfilingRecord

用途：若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return ncclSuccess; }
```

</details>

<a id="f0964"></a>

## F0964：ncclProfilingDump

用途：导出代理阶段耗时记录，帮助区分 GPU 等待、网络等待和 flush；禁用分支无操作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{}
```

</details>

