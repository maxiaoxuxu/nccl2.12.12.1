# src/include/utils.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0872"></a>

## F0872：log2i

用途：用移位计算整数以2为底的向下取整对数，供树深、性能分桶等离散决策使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>long l = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 ；继续条件 (n&gt;&gt;=1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>l++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 5 | 0 / 函数返回 | <code>return l;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
 long l = 0;
 while (n>>=1) l++;
 return l;
}
```

</details>

<a id="f0873"></a>

## F0873：ncclRecyclableList

用途：建立空的可复用链表，分离已有容量、已用边界和遍历游标。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>tail = cursor = head = NULL;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>n = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    tail = cursor = head = NULL;
    n = 0;
  }
```

</details>

<a id="f0874"></a>

## F0874：count

用途：返回当前这轮列表已使用的元素数，而不是历史分配容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return n;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return n; }
```

</details>

<a id="f0875"></a>

## F0875：getNewElem

用途：取可复用节点并清零 data，容量不足时扩展节点链，减少高频排队的动态分配。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(tail != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>*dataOut = &tail-&gt;data;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>memset(*dataOut, 0, sizeof(T));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&tail, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>*dataOut = &tail-&gt;data;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>cursor = head = tail;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 9 | 0 / 条件判断 | <code>(tail-&gt;next == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclCalloc(&tail-&gt;next, 1));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>tail = tail-&gt;next;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>n += 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (tail != NULL) {
      *dataOut = &tail->data;
      memset(*dataOut, 0, sizeof(T));
    } else {
      NCCLCHECK(ncclCalloc(&tail, 1));
      *dataOut = &tail->data;
      cursor = head = tail;
    }
    if (tail->next == NULL) {
      NCCLCHECK(ncclCalloc(&tail->next, 1));
    }
    tail = tail->next;
    n += 1;
    return ncclSuccess;
  }
```

</details>

<a id="f0876"></a>

## F0876：begin

用途：定位本轮首个已用节点并初始化后续遍历游标，空列表返回 NULL。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(head == NULL &#124;&#124; head == tail)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>cursor = head-&gt;next;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 5 | 0 / 函数返回 | <code>return &head-&gt;data;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (head == NULL || head == tail) return NULL;
    cursor = head->next;
    return &head->data;
  }
```

</details>

<a id="f0877"></a>

## F0877：getNext

用途：返回游标处元素并向后推进，越过本轮已用边界时返回 NULL。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(cursor == NULL &#124;&#124; cursor == tail)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 准备局部状态 | <code>T* rv = &cursor-&gt;data;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>cursor = cursor-&gt;next;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 0 / 函数返回 | <code>return rv;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    // tail always points to the next element to be enqueued
    // hence does not contain valid data
    if (cursor == NULL || cursor == tail) return NULL;
    T* rv = &cursor->data;
    cursor = cursor->next;
    return rv;
  }
```

</details>

<a id="f0878"></a>

## F0878：peakNext

用途：只查看下一个元素是否存在而不推进游标，供队列耗尽后回收判断。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(cursor == NULL &#124;&#124; cursor == tail)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return NULL;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 函数返回 | <code>return &cursor-&gt;data;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    if (cursor == NULL || cursor == tail) return NULL;
    return &cursor->data;
  }
```

</details>

<a id="f0879"></a>

## F0879：recycle

用途：把本轮已用边界/游标复位到已分配链头，下轮复用现有节点而不释放容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>tail = cursor = head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>n = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    tail = cursor = head;
    n = 0;
  }
```

</details>

<a id="f0880"></a>

## F0880：~ncclRecyclableList

用途：遍历并释放整条已分配链，和仅重置使用状态的 recycle 区分。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 ；继续条件 (head != NULL)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 准备局部状态 | <code>struct ncclListElem&lt;T&gt;* temp = head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>head = head-&gt;next;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 4 | 1 / 调用 / 计算 / 写入 | <code>free(temp);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
    while (head != NULL) {
      struct ncclListElem<T>* temp = head;
      head = head->next;
      free(temp);
    }
  }
```

</details>

