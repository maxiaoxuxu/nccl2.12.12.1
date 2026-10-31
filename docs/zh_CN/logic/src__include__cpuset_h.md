# src/include/cpuset.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0629"></a>

## F0629：hexToInt

用途：把十六进制字符转换成位掩码需要的数值，辅助 CPU affinity 字符串解析。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int v = c - '0';</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(v &lt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 条件判断 | <code>(v &gt; 9)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>v = 10 + c - 'a';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 条件判断 | <code>((v &lt; 0) &#124;&#124; (v &gt; 15))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 函数返回 | <code>return v;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int v = c - '0';
  if (v < 0) return -1;
  if (v > 9) v = 10 + c - 'a';
  if ((v < 0) || (v > 15)) return -1;
  return v;
}
```

</details>

<a id="f0630"></a>

## F0630：ncclStrToCpuset

用途：把系统/配置的十六进制 CPU 位图解析成 cpu_set_t，支持线程亲和性设置。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>uint32_t cpumasks[CPU_SET_N_U32];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int m = CPU_SET_N_U32-1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>cpumasks[m] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 循环 | <code>初始 int o=0;；继续条件 o&lt;strlen(str)；每轮更新 o++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 准备局部状态 | <code>char c = str[o];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 1 / 条件判断 | <code>(c == ',')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>m--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>cpumasks[m] = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 3 / 准备局部状态 | <code>int v = hexToInt(c);</code> | 把十六进制字符转换成位掩码需要的数值，辅助 CPU affinity 字符串解析。 |
| 12 | 3 / 条件判断 | <code>(v == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 5 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 15 | 3 / 调用 / 计算 / 写入 | <code>cpumasks[m] &lt;&lt;= 4;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>cpumasks[m] += v;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 0 / 循环 | <code>初始 int a=0;；继续条件 m&lt;CPU_SET_N_U32；每轮更新 a++,m++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 1 / 调用 / 计算 / 写入 | <code>memcpy(((uint32_t*)mask)+a, cpumasks+m, sizeof(uint32_t));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  uint32_t cpumasks[CPU_SET_N_U32];
  int m = CPU_SET_N_U32-1;
  cpumasks[m] = 0;
  for (int o=0; o<strlen(str); o++) {
    char c = str[o];
    if (c == ',') {
      m--;
      cpumasks[m] = 0;
    } else {
      int v = hexToInt(c);
      if (v == -1) break;
      cpumasks[m] <<= 4;
      cpumasks[m] += v;
    }
  }
  // Copy cpumasks to mask
  for (int a=0; m<CPU_SET_N_U32; a++,m++) {
    memcpy(((uint32_t*)mask)+a, cpumasks+m, sizeof(uint32_t));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0631"></a>

## F0631：ncclCpusetToStr

用途：把 cpu_set_t 格式化成十六进制文本，方便导出/诊断拓扑 CPU affinity。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int c = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>uint8_t* m8 = (uint8_t*)mask;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int o=sizeof(cpu_set_t)-1;；继续条件 o&gt;=0；每轮更新 o--</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(c == 0 && m8[o] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>sprintf(str+c, "%02x", m8[o]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>c+=2;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 1 / 条件判断 | <code>(o && o%4 == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>sprintf(str+c, ",");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>c++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>str[c] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int c = 0;
  uint8_t* m8 = (uint8_t*)mask;
  for (int o=sizeof(cpu_set_t)-1; o>=0; o--) {
    if (c == 0 && m8[o] == 0) continue;
    sprintf(str+c, "%02x", m8[o]);
    c+=2;
    if (o && o%4 == 0) {
      sprintf(str+c, ",");
      c++;
    }
  }
  str[c] = '\0';
  return ncclSuccess;
}
```

</details>

