# src/graph/xml.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0532"></a>

## F0532：xmlGetChar

用途：从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(fread(c, 1, 1, file) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("XML Parse : Unexpected EOF");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (fread(c, 1, 1, file) == 0) {
    WARN("XML Parse : Unexpected EOF");
    return ncclInternalError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0533"></a>

## F0533：xmlGetValue

用途：解析 XML 的引号属性值或对应值片段，限制目标容量避免读取越界。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetChar(file, &c));</code> | 失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。 |
| 3 | 0 / 条件判断 | <code>(c != '"' && c != '\'')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 编译期分支/指令 | <code>#if INT_OK</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 6 | 3 / 准备局部状态 | <code>int o = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 3 / 先执行一次的循环 | <code>初始 ；继续条件 (c &gt;= '0' && c &lt;= '9')；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>value[o++] = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetChar(file, &c));</code> | 失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。 |
| 10 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>value[o] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 3 / 调用 / 计算 / 写入 | <code>*last = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 3 / 编译期分支/指令 | <code>#else</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>WARN("XML Parse : Expected (double) quote.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 17 | 0 / 准备局部状态 | <code>int o = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 0 / 先执行一次的循环 | <code>初始 ；继续条件 (c != '"')；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 19 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetChar(file, &c));</code> | 失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。 |
| 20 | 1 / 调用 / 计算 / 写入 | <code>value[o++] = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>value[o-1] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetChar(file, last));</code> | 失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。 |
| 24 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char c;
  NCCLCHECK(xmlGetChar(file, &c));
  if (c != '"' && c != '\'') {
#if INT_OK
    int o = 0;
    do {
      value[o++] = c;
      NCCLCHECK(xmlGetChar(file, &c));
    } while (c >= '0' && c <= '9');
    value[o] = '\0';
    *last = c;
    return ncclSuccess;
#else
    WARN("XML Parse : Expected (double) quote.");
    return ncclInternalError;
#endif
  }
  int o = 0;
  do {
    NCCLCHECK(xmlGetChar(file, &c));
    value[o++] = c;
  } while (c != '"');
  value[o-1] = '\0';
  NCCLCHECK(xmlGetChar(file, last));
  return ncclSuccess;
}
```

</details>

<a id="f0534"></a>

## F0534：xmlGetToken

用途：扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>char* ptr = name;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>int o = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 先执行一次的循环 | <code>初始 ；继续条件 (c != ' ' && c != '&gt;' && c != '/' && c != '\n' && c != '\r')；每轮更新</code> | do-while先执行body再检查；其他循环先按继续条件决定是否进入。break/return/abort可改变出口。 |
| 5 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetChar(file, &c));</code> | 失败时可能提前return，成功才继续；从 XML 输入流读取字符并检测结束状态，作为轻量解析器的基础读操作。 |
| 6 | 1 / 条件判断 | <code>(c == '=')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 3 / 调用 / 计算 / 写入 | <code>ptr[o] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 3 / 条件判断 | <code>(value == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 5 / 调用 / 计算 / 写入 | <code>WARN("XML Parse : Unexpected value with name %s", ptr);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 5 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 3 / 函数返回 | <code>return xmlGetValue(file, value, last);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 1 / 调用 / 计算 / 写入 | <code>ptr[o] = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 1 / 条件判断 | <code>(o == MAX_STR_LEN-1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>ptr[o] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>WARN("Error : name %s too long (max %d)", ptr, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 20 | 1 / 调用 / 计算 / 写入 | <code>o++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 21 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>ptr[o-1] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>*last = c;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char c;
  char* ptr = name;
  int o = 0;
  do {
    NCCLCHECK(xmlGetChar(file, &c));
    if (c == '=') {
      ptr[o] = '\0';
      if (value == NULL) {
        WARN("XML Parse : Unexpected value with name %s", ptr);
        return ncclInternalError;
      }
      return xmlGetValue(file, value, last);
    }
    ptr[o] = c;
    if (o == MAX_STR_LEN-1) {
      ptr[o] = '\0';
      WARN("Error : name %s too long (max %d)", ptr, MAX_STR_LEN);
      return ncclInternalError;
    }
    o++;
  } while (c != ' ' && c != '>' && c != '/' && c != '\n' && c != '\r');
  ptr[o-1] = '\0';
  *last = c;
  return ncclSuccess;
}
```

</details>

<a id="f0535"></a>

## F0535：xmlSkipComment

用途：跳过 XML 注释直到终止符，不把注释内容误识别成拓扑节点。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char end[4] = "...";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;strlen(start)；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 调用 / 计算 / 写入 | <code>SHIFT_APPEND(end, start[i]);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>SHIFT_APPEND(end, next);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 0 / 循环 | <code>初始 ；继续条件 (strcmp(end, "--&gt;") != 0)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 准备局部状态 | <code>int c;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 1 / 条件判断 | <code>(fread(&c, 1, 1, file) != 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>WARN("XML Parse error : unterminated comment");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>SHIFT_APPEND(end, c);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Start from something neutral with \0 at the end.
  char end[4] = "...";

  // Inject all trailing chars from previous reads. We don't need
  // to check for --> here because there cannot be a > in the name.
  for (int i=0; i<strlen(start); i++) SHIFT_APPEND(end, start[i]);
  SHIFT_APPEND(end, next);

  // Stop when we find "-->"
  while (strcmp(end, "-->") != 0) {
    int c;
    if (fread(&c, 1, 1, file) != 1) {
      WARN("XML Parse error : unterminated comment");
      return ncclInternalError;
    }
    SHIFT_APPEND(end, c);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0536"></a>

## F0536：xmlGetNode

用途：读取一个 XML 标签的名字、开闭类型和属性，填入节点表示。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;type = NODE_TYPE_NONE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 准备局部状态 | <code>char c = ' ';</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 ；继续条件 (c == ' ' &#124;&#124; c == '\n' &#124;&#124; c == '\r')；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(fread(&c, 1, 1, file) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 条件判断 | <code>(c != '&lt;')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>WARN("XML Parse error : expecting '&lt;', got '%c'", c);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetToken(file, node-&gt;name, NULL, &c));</code> | 失败时可能提前return，成功才继续；扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。 |
| 13 | 0 / 条件判断 | <code>(strncmp(node-&gt;name, "!--", 3) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSkipComment(file, node-&gt;name+3, c));</code> | 失败时可能提前return，成功才继续；跳过 XML 注释直到终止符，不把注释内容误识别成拓扑节点。 |
| 16 | 2 / 函数返回 | <code>return xmlGetNode(file, node);</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 17 | 0 / 条件判断 | <code>(node-&gt;name[0] == '\0' && c == '/')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>node-&gt;type = NODE_TYPE_CLOSE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetToken(file, node-&gt;name, NULL, &c));</code> | 失败时可能提前return，成功才继续；扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。 |
| 21 | 2 / 条件判断 | <code>(c != '&gt;')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>WARN("XML Parse error : unexpected trailing %c in closing tag %s", c, node-&gt;name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 24 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 25 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;type = NODE_TYPE_OPEN;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 27 | 0 / 准备局部状态 | <code>int a = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 0 / 循环 | <code>初始 ；继续条件 (c == ' ')；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 29 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetToken(file, node-&gt;attrs[a].key, node-&gt;attrs[a].value, &c));</code> | 失败时可能提前return，成功才继续；扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。 |
| 30 | 1 / 条件判断 | <code>(a == MAX_ATTR_COUNT)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 3 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "XML Parse : Ignoring extra attributes (max %d)", MAX_ATTR_COUNT);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 33 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 34 | 3 / 调用 / 计算 / 写入 | <code>a++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;nAttrs = a;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 0 / 条件判断 | <code>(c == '/')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 38 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 39 | 2 / 调用 / 计算 / 写入 | <code>node-&gt;type = NODE_TYPE_SINGLE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 40 | 2 / 准备局部状态 | <code>char str[MAX_STR_LEN];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 41 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetToken(file, str, NULL, &c));</code> | 失败时可能提前return，成功才继续；扫描 XML 分隔符之间的 token，供节点名/属性名解析使用。 |
| 42 | 0 / 条件判断 | <code>(c != '&gt;')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 43 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 44 | 2 / 调用 / 计算 / 写入 | <code>WARN("XML Parse : expected &gt;, got '%c'", c);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 45 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 46 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  node->type = NODE_TYPE_NONE;
  char c = ' ';
  while (c == ' ' || c == '\n' || c == '\r') {
    if (fread(&c, 1, 1, file) == 0) return ncclSuccess;
  }
  if (c != '<') {
    WARN("XML Parse error : expecting '<', got '%c'", c);
    return ncclInternalError;
  }
  // Read XML element name
  NCCLCHECK(xmlGetToken(file, node->name, NULL, &c));

  // Check for comments
  if (strncmp(node->name, "!--", 3) == 0) {
    NCCLCHECK(xmlSkipComment(file, node->name+3, c));
    return xmlGetNode(file, node);
  }

  // Check for closing tag
  if (node->name[0] == '\0' && c == '/') {
    node->type = NODE_TYPE_CLOSE;
    // Re-read the name, we got '/' in the first call
    NCCLCHECK(xmlGetToken(file, node->name, NULL, &c));
    if (c != '>') {
      WARN("XML Parse error : unexpected trailing %c in closing tag %s", c, node->name);
      return ncclInternalError;
    }
    return ncclSuccess;
  }

  node->type = NODE_TYPE_OPEN;

  // Get Attributes
  int a = 0;
  while (c == ' ') {
    NCCLCHECK(xmlGetToken(file, node->attrs[a].key, node->attrs[a].value, &c));
    if (a == MAX_ATTR_COUNT) {
      INFO(NCCL_GRAPH, "XML Parse : Ignoring extra attributes (max %d)", MAX_ATTR_COUNT);
      // Actually we need to still consume the extra attributes so we have an extra one.
    } else a++;
  }
  node->nAttrs = a;
  if (c == '/') {
    node->type = NODE_TYPE_SINGLE;
    char str[MAX_STR_LEN];
    NCCLCHECK(xmlGetToken(file, str, NULL, &c));
  }
  if (c != '>') {
    WARN("XML Parse : expected >, got '%c'", c);
    return ncclInternalError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0537"></a>

## F0537：xmlLoadSub

用途：递归读取节点子树并检查父子闭合关系，限制节点/子项容量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(head && head-&gt;type == NODE_TYPE_SINGLE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 循环 | <code>初始 ；继续条件 (1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 5 | 1 / 条件判断 | <code>(xml-&gt;maxIndex == MAX_NODES)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>WARN("Error : XML parser is limited to 1024 nodes");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 3 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 1 / 准备局部状态 | <code>struct ncclXmlNode* node = xml-&gt;nodes+xml-&gt;maxIndex;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 1 / 调用 / 计算 / 写入 | <code>memset(node, 0, sizeof(struct ncclXmlNode));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetNode(file, node));</code> | 失败时可能提前return，成功才继续；读取一个 XML 标签的名字、开闭类型和属性，填入节点表示。 |
| 12 | 1 / 条件判断 | <code>(node-&gt;type == NODE_TYPE_NONE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 3 / 条件判断 | <code>(head)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 5 / 调用 / 计算 / 写入 | <code>WARN("XML Parse : unterminated %s", head-&gt;name);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 17 | 5 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 18 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 5 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 20 | 1 / 条件判断 | <code>(head && node-&gt;type == NODE_TYPE_CLOSE)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 3 / 条件判断 | <code>(strcmp(node-&gt;name, head-&gt;name) != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 5 / 调用 / 计算 / 写入 | <code>WARN("XML Mismatch : %s / %s", head-&gt;name, node-&gt;name);</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 25 | 5 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 27 | 1 / 准备局部状态 | <code>int found = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 1 / 循环 | <code>初始 int h=0;；继续条件 h&lt;nHandlers；每轮更新 h++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 29 | 2 / 条件判断 | <code>(strcmp(node-&gt;name, handlers[h].name) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 30 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 31 | 4 / 条件判断 | <code>(head)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 6 / 调用 / 计算 / 写入 | <code>head-&gt;subs[head-&gt;nSubs++] = node;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 34 | 4 / 调用 / 计算 / 写入 | <code>node-&gt;parent = head;</code> | 读取或更新当前协议阶段的进度；结合字段所属对象区分寄存器计数、共享发布值和后端完成。 |
| 35 | 4 / 调用 / 计算 / 写入 | <code>node-&gt;nSubs = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 4 / 调用 / 计算 / 写入 | <code>xml-&gt;maxIndex++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(handlers[h].func(file, xml, node));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 38 | 4 / 调用 / 计算 / 写入 | <code>found = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 39 | 4 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 40 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 41 | 1 / 条件判断 | <code>(!found)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 42 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 43 | 3 / 条件判断 | <code>(nHandlers)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 5 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Ignoring element %s", node-&gt;name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, node, NULL, 0));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 47 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (head && head->type == NODE_TYPE_SINGLE) return ncclSuccess;
  while (1) {
    if (xml->maxIndex == MAX_NODES) {
      WARN("Error : XML parser is limited to 1024 nodes");
      return ncclInternalError;
    }
    struct ncclXmlNode* node = xml->nodes+xml->maxIndex;
    memset(node, 0, sizeof(struct ncclXmlNode));
    NCCLCHECK(xmlGetNode(file, node));
    if (node->type == NODE_TYPE_NONE) {
      if (head) {
        WARN("XML Parse : unterminated %s", head->name);
        return ncclInternalError;
      } else {
        // All done
        return ncclSuccess;
      }
    }
    if (head && node->type == NODE_TYPE_CLOSE) {
      if (strcmp(node->name, head->name) != 0) {
        WARN("XML Mismatch : %s / %s", head->name, node->name);
        return ncclInternalError;
      }
      return ncclSuccess;
    }
    int found = 0;
    for (int h=0; h<nHandlers; h++) {
      if (strcmp(node->name, handlers[h].name) == 0) {
        if (head) head->subs[head->nSubs++] = node;
        node->parent = head;
        node->nSubs = 0;
        xml->maxIndex++;
        NCCLCHECK(handlers[h].func(file, xml, node));
        found = 1;
        break;
      }
    }
    if (!found) {
      if (nHandlers) INFO(NCCL_GRAPH, "Ignoring element %s", node->name);
      NCCLCHECK(xmlLoadSub(file, xml, node, NULL, 0));
    }
  }
}
```

</details>

<a id="f0538"></a>

## F0538：ncclTopoDumpXmlRec

用途：递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;indent；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>fprintf(file, " ");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>fprintf(file, "&lt;%s", node-&gt;name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 0 / 循环 | <code>初始 int a=0;；继续条件 a&lt;node-&gt;nAttrs；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 6 | 1 / 调用 / 计算 / 写入 | <code>fprintf(file, " %s=\"%s\"", node-&gt;attrs[a].key, node-&gt;attrs[a].value);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 条件判断 | <code>(node-&gt;nSubs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>fprintf(file, "/&gt;\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 11 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>fprintf(file, "&gt;\n");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;node-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 14 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoDumpXmlRec(indent+2, file, node-&gt;subs[s]));</code> | 失败时可能提前return，成功才继续；递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。 |
| 15 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 2 / 循环 | <code>初始 int i=0;；继续条件 i&lt;indent；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 17 | 3 / 调用 / 计算 / 写入 | <code>fprintf(file, " ");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>fprintf(file, "&lt;/%s&gt;\n", node-&gt;name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int i=0; i<indent; i++) fprintf(file, " ");
  fprintf(file, "<%s", node->name);

  for (int a=0; a<node->nAttrs; a++) {
    fprintf(file, " %s=\"%s\"", node->attrs[a].key, node->attrs[a].value);
  }
  if (node->nSubs == 0) {
    fprintf(file, "/>\n");
  } else {
    fprintf(file, ">\n");
    for (int s=0; s<node->nSubs; s++) {
      NCCLCHECK(ncclTopoDumpXmlRec(indent+2, file, node->subs[s]));
    }
    for (int i=0; i<indent; i++) fprintf(file, " ");
    fprintf(file, "</%s>\n", node->name);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0539"></a>

## F0539：ncclTopoDumpXmlToFile

用途：递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>FILE* file = fopen(xmlTopoFile, "w");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(file == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("Unable to open %s, not dumping topology.", xmlTopoFile);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoDumpXmlRec(0, file, xml-&gt;nodes));</code> | 失败时可能提前return，成功才继续；递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>fclose(file);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  FILE* file = fopen(xmlTopoFile, "w");
  if (file == NULL) {
    WARN("Unable to open %s, not dumping topology.", xmlTopoFile);
    return ncclSuccess;
  }
  NCCLCHECK(ncclTopoDumpXmlRec(0, file, xml->nodes));
  fclose(file);
  return ncclSuccess;
}
```

</details>

<a id="f0540"></a>

## F0540：ncclTopoXmlLoadNvlink

用途：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));
  return ncclSuccess;
}
```

</details>

<a id="f0541"></a>

## F0541：ncclTopoXmlLoadGpu

用途：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "nvlink", ncclTopoXmlLoadNvlink } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct xmlHandler handlers[] = { { "nvlink", ncclTopoXmlLoadNvlink } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));
  return ncclSuccess;
}
```

</details>

<a id="f0542"></a>

## F0542：ncclTopoXmlLoadNet

用途：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));
  return ncclSuccess;
}
```

</details>

<a id="f0543"></a>

## F0543：ncclTopoXmlLoadNic

用途：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "net", ncclTopoXmlLoadNet } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct xmlHandler handlers[] = { { "net", ncclTopoXmlLoadNet } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));
  return ncclSuccess;
}
```

</details>

<a id="f0544"></a>

## F0544：ncclTopoXmlLoadPci

用途：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "pci", ncclTopoXmlLoadPci }, { "gpu", ncclTopoXmlLoadGpu }, { "nic", ncclTopoXmlLoadNic} };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 3));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct xmlHandler handlers[] = { { "pci", ncclTopoXmlLoadPci }, { "gpu", ncclTopoXmlLoadGpu }, { "nic", ncclTopoXmlLoadNic} };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 3));
  return ncclSuccess;
}
```

</details>

<a id="f0545"></a>

## F0545：ncclTopoXmlLoadCpu

用途：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "pci", ncclTopoXmlLoadPci }, { "nic", ncclTopoXmlLoadNic } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 2));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct xmlHandler handlers[] = { { "pci", ncclTopoXmlLoadPci }, { "nic", ncclTopoXmlLoadNic } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 2));
  return ncclSuccess;
}
```

</details>

<a id="f0546"></a>

## F0546：ncclTopoXmlLoadSystem

用途：按对应标签种类递归加载硬件 XML，消费子节点并校验该层允许的结构。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int version;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(head, "version", &version));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 3 | 0 / 条件判断 | <code>(version != NCCL_TOPO_XML_VERSION)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>WARN("XML Topology has wrong version %d, %d needed", version, NCCL_TOPO_XML_VERSION);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>const char* name;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(head, "name", &name));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 9 | 0 / 条件判断 | <code>(name != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Loading topology %s", name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Loading unnamed topology");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "cpu", ncclTopoXmlLoadCpu } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int version;
  NCCLCHECK(xmlGetAttrInt(head, "version", &version));
  if (version != NCCL_TOPO_XML_VERSION) {
    WARN("XML Topology has wrong version %d, %d needed", version, NCCL_TOPO_XML_VERSION);
    return ncclInvalidUsage;
  }
  const char* name;
  NCCLCHECK(xmlGetAttr(head, "name", &name));
  if (name != NULL) INFO(NCCL_GRAPH, "Loading topology %s", name);
  else INFO(NCCL_GRAPH, "Loading unnamed topology");

  struct xmlHandler handlers[] = { { "cpu", ncclTopoXmlLoadCpu } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));
  return ncclSuccess;
}
```

</details>

<a id="f0547"></a>

## F0547：ncclTopoGetXmlFromFile

用途：打开拓扑文件并解析指定版本的系统 XML，按 warn 决定缺失文件的处理方式。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>FILE* file = fopen(xmlTopoFile, "r");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(file == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 条件判断 | <code>(warn)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 4 / 调用 / 计算 / 写入 | <code>WARN("Could not open XML topology file %s : %s", xmlTopoFile, strerror(errno));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Loading topology file %s", xmlTopoFile);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "system", ncclTopoXmlLoadSystem } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>xml-&gt;maxIndex = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, NULL, handlers, 1));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>fclose(file);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  FILE* file = fopen(xmlTopoFile, "r");
  if (file == NULL) {
    if (warn) {
      WARN("Could not open XML topology file %s : %s", xmlTopoFile, strerror(errno));
    }
    return ncclSuccess;
  }
  INFO(NCCL_GRAPH, "Loading topology file %s", xmlTopoFile);
  struct xmlHandler handlers[] = { { "system", ncclTopoXmlLoadSystem } };
  xml->maxIndex = 0;
  NCCLCHECK(xmlLoadSub(file, xml, NULL, handlers, 1));
  fclose(file);
  return ncclSuccess;
}
```

</details>

<a id="f0548"></a>

## F0548：memcpylower

用途：复制字符同时转为小写，规范 sysfs 路径/属性中的文本比较。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;size；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 2 | 1 / 调用 / 计算 / 写入 | <code>dst[i] = tolower(src[i]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 3 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  for (int i=0; i<size; i++) dst[i] = tolower(src[i]);
}
```

</details>

<a id="f0549"></a>

## F0549：getPciPath

用途：解析设备对应的系统 PCI 实际路径，供网卡/GPU 归属层级匹配。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char busPath[] = "/sys/class/pci_bus/0000:00/../../0000:00:00.0";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>memcpylower(busPath+sizeof("/sys/class/pci_bus/")-1, busId, BUSID_REDUCED_SIZE-1);</code> | 复制字符同时转为小写，规范 sysfs 路径/属性中的文本比较。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>memcpylower(busPath+sizeof("/sys/class/pci_bus/0000:00/../../")-1, busId, BUSID_SIZE-1);</code> | 复制字符同时转为小写，规范 sysfs 路径/属性中的文本比较。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>*path = realpath(busPath, NULL);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 条件判断 | <code>(*path == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>WARN("Could not find real path of %s", busPath);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char busPath[] = "/sys/class/pci_bus/0000:00/../../0000:00:00.0";
  memcpylower(busPath+sizeof("/sys/class/pci_bus/")-1, busId, BUSID_REDUCED_SIZE-1);
  memcpylower(busPath+sizeof("/sys/class/pci_bus/0000:00/../../")-1, busId, BUSID_SIZE-1);
  *path = realpath(busPath, NULL);
  if (*path == NULL) {
    WARN("Could not find real path of %s", busPath);
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0550"></a>

## F0550：ncclTopoGetStrFromSys

用途：从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char filePath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>sprintf(filePath, "%s/%s", path, fileName);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 准备局部状态 | <code>int offset = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 准备局部状态 | <code>FILE* file;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 0 / 条件判断 | <code>((file = fopen(filePath, "r")) != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 循环 | <code>初始 ；继续条件 (feof(file) == 0 && ferror(file) == 0 && offset &lt; MAX_STR_LEN)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 3 / 准备局部状态 | <code>int len = fread(strValue+offset, 1, MAX_STR_LEN-offset, file);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 3 / 调用 / 计算 / 写入 | <code>offset += len;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>fclose(file);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 0 / 条件判断 | <code>(offset == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>strValue[0] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Topology detection : could not read %s, ignoring", filePath);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 16 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>strValue[offset-1] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char filePath[PATH_MAX];
  sprintf(filePath, "%s/%s", path, fileName);
  int offset = 0;
  FILE* file;
  if ((file = fopen(filePath, "r")) != NULL) {
    while (feof(file) == 0 && ferror(file) == 0 && offset < MAX_STR_LEN) {
      int len = fread(strValue+offset, 1, MAX_STR_LEN-offset, file);
      offset += len;
    }
    fclose(file);
  }
  if (offset == 0) {
    strValue[0] = '\0';
    INFO(NCCL_GRAPH, "Topology detection : could not read %s, ignoring", filePath);
  } else {
    strValue[offset-1] = '\0';
  }
  return ncclSuccess;
}
```

</details>

<a id="f0551"></a>

## F0551：ncclTopoSetAttrFromSys

用途：把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char strValue[MAX_STR_LEN];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetStrFromSys(path, fileName, strValue));</code> | 失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。 |
| 3 | 0 / 条件判断 | <code>(strValue[0] != '\0')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(pciNode, attrName, strValue));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>TRACE(NCCL_GRAPH, "Read from sys %s/%s -&gt; %s=%s", path, fileName, attrName, strValue);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char strValue[MAX_STR_LEN];
  NCCLCHECK(ncclTopoGetStrFromSys(path, fileName, strValue));
  if (strValue[0] != '\0') { NCCLCHECK(xmlSetAttr(pciNode, attrName, strValue)); }
  TRACE(NCCL_GRAPH, "Read from sys %s/%s -> %s=%s", path, fileName, attrName, strValue);
  return ncclSuccess;
}
```

</details>

<a id="f0552"></a>

## F0552：ncclTopoGetXmlFromCpu

用途：读取 CPU/NUMA 属性及亲和性，建立/补齐硬件 XML 的 CPU 层。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(cpuNode, "affinity", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 准备局部状态 | <code>const char* numaId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(cpuNode, "numaid", &numaId));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 7 | 2 / 条件判断 | <code>(numaId == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>WARN("GetXmlFromCpu : could not find CPU numa ID.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 4 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 2 / 准备局部状态 | <code>char cpumaskPath[] = "/sys/devices/system/node/node0000";</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>sprintf(cpumaskPath, "/sys/devices/system/node/node%s", numaId);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSetAttrFromSys(cpuNode, cpumaskPath, "cpumap", "affinity"));</code> | 失败时可能提前return，成功才继续；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(cpuNode, "arch", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 15 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 编译期分支/指令 | <code>#if defined(__PPC__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 18 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(cpuNode, "arch", "ppc64"));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 19 | 3 / 编译期分支/指令 | <code>#elif defined(__aarch64__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 20 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(cpuNode, "arch", "arm64"));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 21 | 4 / 编译期分支/指令 | <code>#elif defined(__x86_64__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 22 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(cpuNode, "arch", "x86_64"));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 23 | 0 / 编译期分支/指令 | <code>#if defined(__x86_64__)</code> | 由宏配置决定哪些片段进入编译；下面各编译分支不代表运行时都顺序执行。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(cpuNode, "vendor", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 25 | 1 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 3 / 准备局部状态 | <code>union { struct { uint32_t ebx; uint32_t edx; uint32_t ecx; }; char vendor[12]; } cpuid0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 28 | 3 / 调用 / 计算 / 写入 | <code>asm volatile("cpuid" : "=b" (cpuid0.ebx), "=c" (cpuid0.ecx), "=d" (cpuid0.edx) : "a" (0) : "memory");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 3 / 准备局部状态 | <code>char vendor[13];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 3 / 调用 / 计算 / 写入 | <code>strncpy(vendor, cpuid0.vendor, 12);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 3 / 调用 / 计算 / 写入 | <code>vendor[12] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 32 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(cpuNode, "vendor", vendor));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 33 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(cpuNode, "familyid", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 34 | 1 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 35 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 36 | 3 / 准备局部状态 | <code>union { struct { unsigned steppingId:4; unsigned modelId:4; unsigned familyId:4; unsigned processorType:2; unsigned resv0:2; unsigned extModelId:4; unsigned extFamilyId:8; unsigned resv1:4; }; uint32_t val; } cpuid1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 37 | 3 / 调用 / 计算 / 写入 | <code>asm volatile("cpuid" : "=a" (cpuid1.val) : "a" (1) : "memory");</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 38 | 3 / 准备局部状态 | <code>int familyId = cpuid1.familyId + (cpuid1.extFamilyId &lt;&lt; 4);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 39 | 3 / 准备局部状态 | <code>int modelId = cpuid1.modelId + (cpuid1.extModelId &lt;&lt; 4);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(cpuNode, "familyid", familyId));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 41 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(cpuNode, "modelid", modelId));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 42 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(cpuNode, "affinity", &index));
  if (index == -1) {
    const char* numaId;
    NCCLCHECK(xmlGetAttr(cpuNode, "numaid", &numaId));
    if (numaId == NULL) {
      WARN("GetXmlFromCpu : could not find CPU numa ID.");
      return ncclInternalError;
    }
    // Set affinity
    char cpumaskPath[] = "/sys/devices/system/node/node0000";
    sprintf(cpumaskPath, "/sys/devices/system/node/node%s", numaId);
    NCCLCHECK(ncclTopoSetAttrFromSys(cpuNode, cpumaskPath, "cpumap", "affinity"));
  }

  NCCLCHECK(xmlGetAttrIndex(cpuNode, "arch", &index));
  if (index == -1) {
    // Fill CPU type / vendor / model
#if defined(__PPC__)
    NCCLCHECK(xmlSetAttr(cpuNode, "arch", "ppc64"));
#elif defined(__aarch64__)
    NCCLCHECK(xmlSetAttr(cpuNode, "arch", "arm64"));
#elif defined(__x86_64__)
    NCCLCHECK(xmlSetAttr(cpuNode, "arch", "x86_64"));
#endif
  }

#if defined(__x86_64__)
  NCCLCHECK(xmlGetAttrIndex(cpuNode, "vendor", &index));
  if (index == -1) {
    union {
      struct {
        // CPUID 0 String register order
        uint32_t ebx;
        uint32_t edx;
        uint32_t ecx;
      };
      char vendor[12];
    } cpuid0;

    asm volatile("cpuid" : "=b" (cpuid0.ebx), "=c" (cpuid0.ecx), "=d" (cpuid0.edx) : "a" (0) : "memory");
    char vendor[13];
    strncpy(vendor, cpuid0.vendor, 12);
    vendor[12] = '\0';
    NCCLCHECK(xmlSetAttr(cpuNode, "vendor", vendor));
  }

  NCCLCHECK(xmlGetAttrIndex(cpuNode, "familyid", &index));
  if (index == -1) {
    union {
      struct {
        unsigned steppingId:4;
        unsigned modelId:4;
        unsigned familyId:4;
        unsigned processorType:2;
        unsigned resv0:2;
        unsigned extModelId:4;
        unsigned extFamilyId:8;
        unsigned resv1:4;
      };
      uint32_t val;
    } cpuid1;
    asm volatile("cpuid" : "=a" (cpuid1.val) : "a" (1) : "memory");
    int familyId = cpuid1.familyId + (cpuid1.extFamilyId << 4);
    int modelId = cpuid1.modelId + (cpuid1.extModelId << 4);
    NCCLCHECK(xmlSetAttrInt(cpuNode, "familyid", familyId));
    NCCLCHECK(xmlSetAttrInt(cpuNode, "modelid", modelId));
  }
#endif
  return ncclSuccess;
}
```

</details>

<a id="f0553"></a>

## F0553：ncclTopoGetPciNode

用途：按 PCI 地址在 XML 中查找已有节点，避免同一设备被重复创建。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlFindTagKv(xml, "pci", pciNode, "busid", busId));</code> | 失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。 |
| 2 | 0 / 条件判断 | <code>(*pciNode == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, NULL, "pci", pciNode));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(*pciNode, "busid", busId));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(xmlFindTagKv(xml, "pci", pciNode, "busid", busId));
  if (*pciNode == NULL) {
    NCCLCHECK(xmlAddNode(xml, NULL, "pci", pciNode));
    NCCLCHECK(xmlSetAttr(*pciNode, "busid", busId));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0554"></a>

## F0554：isHex

用途：检查字符是否为十六进制数字，辅助验证 PCI BDF 路径格式。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 函数返回 | <code>return ((c &gt;= '0' && c &lt;= '9') &#124;&#124; (c &gt;= 'a' && c &lt;= 'f') &#124;&#124; (c &gt;= 'A' && c &lt;= 'F'));</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{ return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')); }
```

</details>

<a id="f0555"></a>

## F0555：checkBDFFormat

用途：验证 PCI domain:bus:device.function 文本格式，防止非设备目录被当作 PCI 节点。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(bdf[4] != ':' &#124;&#124; bdf[7] != ':' &#124;&#124; bdf[10] != '.')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 函数返回 | <code>return 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 4 | 0 / 条件判断 | <code>(isHex(bdf[0]) == 0 &#124;&#124; isHex(bdf[1] == 0) &#124;&#124; isHex(bdf[2] == 0) &#124;&#124; isHex(bdf[3] == 0) &#124;&#124; isHex(bdf[5] == 0) &#124;&#124; isHex(bdf[6] == 0) &#124;&#124; isHex(bdf[8] == 0) &#124;&#124; isHex(bdf[9] == 0) &#124;&#124; isHex(bdf[11] == 0))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 函数返回 | <code>return 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 函数返回 | <code>return 1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (bdf[4] != ':' || bdf[7] != ':' || bdf[10] != '.') return 0;
  if (isHex(bdf[0]) == 0 || isHex(bdf[1] == 0) || isHex(bdf[2] == 0) || isHex(bdf[3] == 0) ||
      isHex(bdf[5] == 0) || isHex(bdf[6] == 0) || isHex(bdf[8] == 0) || isHex(bdf[9] == 0) ||
      isHex(bdf[11] == 0)) return 0;
  return 1;
}
```

</details>

<a id="f0556"></a>

## F0556：ncclTopoGetXmlFromSys

用途：沿 sysfs 真实设备路径建立 PCI 父层级，并读取链路速度/宽度等属性。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const char* busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 3 | 0 / 准备局部状态 | <code>char* path = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = NCCL_GRAPH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 0 / 调用 / 计算 / 写入 | <code>getPciPath(busId, &path);</code> | 解析设备对应的系统 PCI 实际路径，供网卡/GPU 归属层级匹配。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 条件判断 | <code>(path)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSetAttrFromSys(pciNode, path, "class", "class"));</code> | 失败时可能提前return，成功才继续；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。 |
| 10 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 11 | 0 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = NCCL_GRAPH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(pciNode, "vendor", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 13 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 2 / 条件判断 | <code>(path)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 4 / 调用 / 计算 / 写入 | <code>ncclTopoSetAttrFromSys(pciNode, path, "vendor", "vendor");</code> | 把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(pciNode, "device", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 19 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 20 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 21 | 2 / 条件判断 | <code>(path)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 22 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 23 | 4 / 调用 / 计算 / 写入 | <code>ncclTopoSetAttrFromSys(pciNode, path, "device", "device");</code> | 把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。 |
| 24 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(pciNode, "subsystem_vendor", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 25 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 26 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 27 | 2 / 条件判断 | <code>(path)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 4 / 调用 / 计算 / 写入 | <code>ncclTopoSetAttrFromSys(pciNode, path, "subsystem_vendor", "subsystem_vendor");</code> | 把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。 |
| 30 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(pciNode, "subsystem_device", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 31 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 32 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 33 | 2 / 条件判断 | <code>(path)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 34 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 35 | 4 / 调用 / 计算 / 写入 | <code>ncclTopoSetAttrFromSys(pciNode, path, "subsystem_device", "subsystem_device");</code> | 把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。 |
| 36 | 0 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 37 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(pciNode, "link_speed", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 38 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 39 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 40 | 2 / 条件判断 | <code>(path)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 41 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 42 | 4 / 准备局部状态 | <code>char deviceSpeedStr[MAX_STR_LEN];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 43 | 4 / 准备局部状态 | <code>float deviceSpeed;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 44 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_speed", deviceSpeedStr));</code> | 失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。 |
| 45 | 4 / 调用 / 计算 / 写入 | <code>sscanf(deviceSpeedStr, "%f GT/s", &deviceSpeed);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 46 | 4 / 准备局部状态 | <code>char portSpeedStr[MAX_STR_LEN];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 47 | 4 / 准备局部状态 | <code>float portSpeed;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 48 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_speed", portSpeedStr));</code> | 失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。 |
| 49 | 4 / 调用 / 计算 / 写入 | <code>sscanf(portSpeedStr, "%f GT/s", &portSpeed);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 50 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(pciNode, "link_speed", portSpeed &lt; deviceSpeed ? portSpeedStr : deviceSpeedStr));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 51 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 52 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(pciNode, "link_speed", ""));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 53 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(pciNode, "link_width", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 54 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 55 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 56 | 2 / 条件判断 | <code>(path)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 4 / 准备局部状态 | <code>char strValue[MAX_STR_LEN];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 59 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_width", strValue));</code> | 失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。 |
| 60 | 4 / 准备局部状态 | <code>int deviceWidth = strtol(strValue, NULL, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 61 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_width", strValue));</code> | 失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。 |
| 62 | 4 / 准备局部状态 | <code>int portWidth = strtol(strValue, NULL, 0);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 63 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(pciNode, "link_width", std::min(deviceWidth,portWidth)));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 64 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 65 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(pciNode, "link_width", ""));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 66 | 0 / 准备局部状态 | <code>struct ncclXmlNode* parent = pciNode-&gt;parent;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 67 | 0 / 条件判断 | <code>(parent == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 2 / 条件判断 | <code>(path)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 70 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 71 | 4 / 准备局部状态 | <code>char numaIdStr[MAX_STR_LEN];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 72 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetStrFromSys(path, "numa_node", numaIdStr));</code> | 失败时可能提前return，成功才继续；从 sysfs 属性文件读取并整理文本值，失败时按本函数的返回约定报告。 |
| 73 | 4 / 准备局部状态 | <code>int slashCount = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 74 | 4 / 准备局部状态 | <code>int parentOffset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 75 | 4 / 循环 | <code>初始 parentOffset = strlen(path)-1；继续条件 parentOffset&gt;0；每轮更新 parentOffset--</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 76 | 5 / 条件判断 | <code>(path[parentOffset] == '/')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 77 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 78 | 7 / 调用 / 计算 / 写入 | <code>slashCount++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 79 | 7 / 调用 / 计算 / 写入 | <code>path[parentOffset] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 80 | 7 / 准备局部状态 | <code>int start = parentOffset - 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 81 | 7 / 循环 | <code>初始 ；继续条件 (start&gt;0 && path[start] != '/')；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 82 | 8 / 调用 / 计算 / 写入 | <code>start--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 83 | 7 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 84 | 7 / 条件判断 | <code>(checkBDFFormat(path+start+1) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 85 | 8 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 86 | 9 / 准备局部状态 | <code>struct ncclXmlNode* topNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 87 | 9 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlFindTag(xml, "system", &topNode));</code> | 失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。 |
| 88 | 9 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetSubKv(topNode, "cpu", &parent, "numaid", numaIdStr));</code> | 失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。 |
| 89 | 9 / 条件判断 | <code>(parent == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 90 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 91 | 11 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, topNode, "cpu", &parent));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 92 | 11 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(parent, "numaid", numaIdStr));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 93 | 8 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 94 | 9 / 条件判断 | <code>(slashCount == 2)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 95 | 10 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 96 | 11 / 循环 | <code>初始 int i = strlen(path)-1;；继续条件 i&gt;0；每轮更新 i--</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 97 | 12 / 条件判断 | <code>(path[i] == '/')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 98 | 13 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 99 | 14 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlFindTagKv(xml, "pci", &parent, "busid", path+i+1));</code> | 失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。 |
| 100 | 14 / 条件判断 | <code>(parent == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 101 | 15 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 102 | 16 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, NULL, "pci", &parent));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 103 | 16 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(parent, "busid", path+i+1));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 104 | 14 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 105 | 11 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 106 | 5 / 条件判断 | <code>(parent)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 107 | 6 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 108 | 7 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 109 | 4 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 110 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 111 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlFindTagKv(xml, "cpu", &parent, "numaid", "-1"));</code> | 失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。 |
| 112 | 4 / 条件判断 | <code>(parent == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 113 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 114 | 6 / 准备局部状态 | <code>struct ncclXmlNode* topNode;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 115 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlFindTag(xml, "system", &topNode));</code> | 失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。 |
| 116 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, topNode, "cpu", &parent));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 117 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(parent, "numaid", "-1"));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 118 | 6 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromCpu(parent, xml));</code> | 失败时可能提前return，成功才继续；读取 CPU/NUMA 属性及亲和性，建立/补齐硬件 XML 的 CPU 层。 |
| 119 | 2 / 调用 / 计算 / 写入 | <code>pciNode-&gt;parent = parent;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 120 | 2 / 调用 / 计算 / 写入 | <code>parent-&gt;subs[parent-&gt;nSubs++] = pciNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 121 | 0 / 条件判断 | <code>(strcmp(parent-&gt;name, "pci") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 122 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 123 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromSys(parent, xml));</code> | 失败时可能提前return，成功才继续；沿 sysfs 真实设备路径建立 PCI 父层级，并读取链路速度/宽度等属性。 |
| 124 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 125 | 2 / 条件判断 | <code>(strcmp(parent-&gt;name, "cpu") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 126 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 127 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromCpu(parent, xml));</code> | 失败时可能提前return，成功才继续；读取 CPU/NUMA 属性及亲和性，建立/补齐硬件 XML 的 CPU 层。 |
| 128 | 0 / 调用 / 计算 / 写入 | <code>free(path);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 129 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // Fill info, then parent
  const char* busId;
  NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));
  char* path = NULL;
  ncclDebugNoWarn = NCCL_GRAPH;
  getPciPath(busId, &path);
  ncclDebugNoWarn = 0;

  if (path) {
    NCCLCHECK(ncclTopoSetAttrFromSys(pciNode, path, "class", "class"));
  }
  int index;
  ncclDebugNoWarn = NCCL_GRAPH;
  NCCLCHECK(xmlGetAttrIndex(pciNode, "vendor", &index));
  if (index == -1) {
    if (path) ncclTopoSetAttrFromSys(pciNode, path, "vendor", "vendor");
  }
  NCCLCHECK(xmlGetAttrIndex(pciNode, "device", &index));
  if (index == -1) {
    if (path) ncclTopoSetAttrFromSys(pciNode, path, "device", "device");
  }
  NCCLCHECK(xmlGetAttrIndex(pciNode, "subsystem_vendor", &index));
  if (index == -1) {
    if (path) ncclTopoSetAttrFromSys(pciNode, path, "subsystem_vendor", "subsystem_vendor");
  }
  NCCLCHECK(xmlGetAttrIndex(pciNode, "subsystem_device", &index));
  if (index == -1) {
    if (path) ncclTopoSetAttrFromSys(pciNode, path, "subsystem_device", "subsystem_device");
  }
  ncclDebugNoWarn = 0;
  NCCLCHECK(xmlGetAttrIndex(pciNode, "link_speed", &index));
  if (index == -1) {
    if (path) {
      char deviceSpeedStr[MAX_STR_LEN];
      float deviceSpeed;
      NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_speed", deviceSpeedStr));
      sscanf(deviceSpeedStr, "%f GT/s", &deviceSpeed);
      char portSpeedStr[MAX_STR_LEN];
      float portSpeed;
      NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_speed", portSpeedStr));
      sscanf(portSpeedStr, "%f GT/s", &portSpeed);
      NCCLCHECK(xmlSetAttr(pciNode, "link_speed", portSpeed < deviceSpeed ? portSpeedStr : deviceSpeedStr));
    } else {
      NCCLCHECK(xmlSetAttr(pciNode, "link_speed", ""));
    }
  }
  NCCLCHECK(xmlGetAttrIndex(pciNode, "link_width", &index));
  if (index == -1) {
    if (path) {
      char strValue[MAX_STR_LEN];
      NCCLCHECK(ncclTopoGetStrFromSys(path, "max_link_width", strValue));
      int deviceWidth = strtol(strValue, NULL, 0);
      NCCLCHECK(ncclTopoGetStrFromSys(path, "../max_link_width", strValue));
      int portWidth = strtol(strValue, NULL, 0);
      NCCLCHECK(xmlSetAttrInt(pciNode, "link_width", std::min(deviceWidth,portWidth)));
    } else {
      NCCLCHECK(xmlSetAttr(pciNode, "link_width", ""));
    }
  }
  struct ncclXmlNode* parent = pciNode->parent;
  if (parent == NULL) {
    if (path) {
      // Save that for later in case next step is a CPU
      char numaIdStr[MAX_STR_LEN];
      NCCLCHECK(ncclTopoGetStrFromSys(path, "numa_node", numaIdStr));

      // Go up one level in the PCI tree. Rewind two "/" and follow the upper PCI
      // switch, or stop if we reach a CPU root complex.
      int slashCount = 0;
      int parentOffset;
      for (parentOffset = strlen(path)-1; parentOffset>0; parentOffset--) {
        if (path[parentOffset] == '/') {
          slashCount++;
          path[parentOffset] = '\0';
          int start = parentOffset - 1;
          while (start>0 && path[start] != '/') start--;
          // Check whether the parent path looks like "BBBB:BB:DD.F" or not.
          if (checkBDFFormat(path+start+1) == 0) {
            // This a CPU root complex. Create a CPU tag and stop there.
            struct ncclXmlNode* topNode;
            NCCLCHECK(xmlFindTag(xml, "system", &topNode));
            NCCLCHECK(xmlGetSubKv(topNode, "cpu", &parent, "numaid", numaIdStr));
            if (parent == NULL) {
              NCCLCHECK(xmlAddNode(xml, topNode, "cpu", &parent));
              NCCLCHECK(xmlSetAttr(parent, "numaid", numaIdStr));
            }
          } else if (slashCount == 2) {
            // Continue on the upper PCI switch
            for (int i = strlen(path)-1; i>0; i--) {
              if (path[i] == '/') {
                NCCLCHECK(xmlFindTagKv(xml, "pci", &parent, "busid", path+i+1));
                if (parent == NULL) {
                  NCCLCHECK(xmlAddNode(xml, NULL, "pci", &parent));
                  NCCLCHECK(xmlSetAttr(parent, "busid", path+i+1));
                }
                break;
              }
            }
          }
        }
        if (parent) break;
      }
    } else {
      // No information on /sys, attach GPU to unknown CPU
      NCCLCHECK(xmlFindTagKv(xml, "cpu", &parent, "numaid", "-1"));
      if (parent == NULL) {
        struct ncclXmlNode* topNode;
        NCCLCHECK(xmlFindTag(xml, "system", &topNode));
        NCCLCHECK(xmlAddNode(xml, topNode, "cpu", &parent));
        NCCLCHECK(xmlSetAttr(parent, "numaid", "-1"));
        NCCLCHECK(ncclTopoGetXmlFromCpu(parent, xml));
      }
    }
    pciNode->parent = parent;
    parent->subs[parent->nSubs++] = pciNode;
  }
  if (strcmp(parent->name, "pci") == 0) {
    NCCLCHECK(ncclTopoGetXmlFromSys(parent, xml));
  } else if (strcmp(parent->name, "cpu") == 0) {
    NCCLCHECK(ncclTopoGetXmlFromCpu(parent, xml));
  }
  free(path);
  return ncclSuccess;
}
```

</details>

<a id="f0557"></a>

## F0557：ncclTopoGetXmlFromGpu

用途：利用 CUDA/NVML 信息补齐 GPU 的 PCI、计算能力和 NVLink 属性。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclXmlNode* gpuNode = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetSub(pciNode, "gpu", &gpuNode));</code> | 失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。 |
| 3 | 0 / 条件判断 | <code>(gpuNode == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, pciNode, "gpu", &gpuNode));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 6 | 0 / 准备局部状态 | <code>int index = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>int dev = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(gpuNode, "dev", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 9 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 条件判断 | <code>(nvmlDev == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 4 / 准备局部状态 | <code>const char* busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 14 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 15 | 4 / 条件判断 | <code>(busId == NULL &#124;&#124; cudaDeviceGetByPCIBusId(&dev, busId) != cudaSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 6 / 调用 / 计算 / 写入 | <code>dev = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 19 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlDeviceGetIndex(nvmlDev, (unsigned int*)&dev));</code> | 失败时可能提前return，成功才继续；由句柄查询 NVML 序号；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。 |
| 20 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(gpuNode, "dev", dev));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(gpuNode, "dev", &dev));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 22 | 0 / 条件判断 | <code>(dev == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>*gpuNodeRet = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 26 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(gpuNode, "sm", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 27 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 28 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 29 | 2 / 准备局部状态 | <code>int cudaMajor, cudaMinor;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 30 | 2 / 条件判断 | <code>(nvmlDev == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 4 / 准备局部状态 | <code>cudaDeviceProp devProp;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 33 | 4 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaGetDeviceProperties(&devProp, dev));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 34 | 4 / 调用 / 计算 / 写入 | <code>cudaMajor = devProp.major;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 35 | 4 / 调用 / 计算 / 写入 | <code>cudaMinor = devProp.minor;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 36 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 37 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclNvmlDeviceGetCudaComputeCapability(nvmlDev, &cudaMajor, &cudaMinor));</code> | 失败时可能提前return，成功才继续；查询 GPU 计算能力；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。 |
| 38 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(gpuNode, "sm", cudaMajor*10+cudaMinor));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 39 | 0 / 准备局部状态 | <code>int sm;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 40 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(gpuNode, "sm", &sm));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 41 | 0 / 准备局部状态 | <code>struct ncclXmlNode* nvlNode = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 42 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetSub(gpuNode, "nvlink", &nvlNode));</code> | 失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。 |
| 43 | 0 / 条件判断 | <code>(nvlNode == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 44 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 45 | 2 / 准备局部状态 | <code>int maxNvLinks = (sm &lt; 60) ? 0 : (sm &lt; 70) ? 4 : (sm &lt; 80) ? 6 : 12;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 46 | 2 / 条件判断 | <code>(maxNvLinks &gt; 0 && nvmlDev == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 47 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 48 | 4 / 调用 / 计算 / 写入 | <code>WARN("No NVML device handle. Skipping nvlink detection.");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 49 | 4 / 调用 / 计算 / 写入 | <code>maxNvLinks = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 50 | 2 / 循环 | <code>初始 int l=0;；继续条件 l&lt;maxNvLinks；每轮更新 ++l</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 51 | 3 / 准备局部状态 | <code>unsigned canP2P;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 52 | 3 / 条件判断 | <code>((ncclNvmlDeviceGetNvLinkCapability(nvmlDev, l, NVML_NVLINK_CAP_P2P_SUPPORTED, &canP2P) != ncclSuccess) &#124;&#124; !canP2P)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 53 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 54 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 55 | 3 / 准备局部状态 | <code>nvmlEnableState_t isActive;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 56 | 3 / 条件判断 | <code>((ncclNvmlDeviceGetNvLinkState(nvmlDev, l, &isActive) != ncclSuccess) &#124;&#124; (isActive != NVML_FEATURE_ENABLED))</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 57 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 58 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 59 | 3 / 准备局部状态 | <code>nvmlPciInfo_t remoteProc;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 60 | 3 / 条件判断 | <code>(ncclNvmlDeviceGetNvLinkRemotePciInfo(nvmlDev, l, &remoteProc) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 61 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 62 | 5 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 63 | 3 / 准备局部状态 | <code>char* p = remoteProc.busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 64 | 3 / 准备局部状态 | <code>char lowerId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 65 | 3 / 循环 | <code>初始 int c=0;；继续条件 c&lt;NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE；每轮更新 c++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 66 | 4 / 调用 / 计算 / 写入 | <code>lowerId[c] = tolower(p[c]);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 67 | 4 / 条件判断 | <code>(p[c] == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 68 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 69 | 6 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 70 | 3 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 71 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetSubKv(gpuNode, "nvlink", &nvlNode, "target", lowerId));</code> | 失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。 |
| 72 | 3 / 条件判断 | <code>(nvlNode == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 73 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 74 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, gpuNode, "nvlink", &nvlNode));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 75 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(nvlNode, "target", lowerId));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 76 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(nvlNode, "count", 1));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 77 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 78 | 5 / 准备局部状态 | <code>int count;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 79 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(nvlNode, "count", &count));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 80 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrInt(nvlNode, "count", count+1));</code> | 失败时可能提前return，成功才继续；把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。 |
| 81 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 82 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;gpuNode-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 83 | 1 / 准备局部状态 | <code>struct ncclXmlNode* sub = gpuNode-&gt;subs[s];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 84 | 1 / 条件判断 | <code>(strcmp(sub-&gt;name, "nvlink") != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 85 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 86 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 87 | 1 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 88 | 1 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(sub, "tclass", &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 89 | 1 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 90 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 91 | 3 / 准备局部状态 | <code>const char* busId;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 92 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(sub, "target", &busId));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 93 | 3 / 准备局部状态 | <code>char* path;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 94 | 3 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = NCCL_GRAPH;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 95 | 3 / 调用 / 计算 / 写入 | <code>getPciPath(busId, &path);</code> | 解析设备对应的系统 PCI 实际路径，供网卡/GPU 归属层级匹配。 |
| 96 | 3 / 调用 / 计算 / 写入 | <code>ncclDebugNoWarn = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 97 | 3 / 条件判断 | <code>(path == NULL &#124;&#124; strcmp(busId, "fffffff:ffff:ff") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 98 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 99 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(sub, "tclass", "0x068000"));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 100 | 4 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 101 | 5 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoSetAttrFromSys(sub, path, "class", "tclass"));</code> | 失败时可能提前return，成功才继续；把 sysfs 文件读取的值填入指定 XML 属性，连接自动发现与统一 XML 表示。 |
| 102 | 5 / 调用 / 计算 / 写入 | <code>free(path);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 103 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 104 | 0 / 调用 / 计算 / 写入 | <code>*gpuNodeRet = gpuNode;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 105 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclXmlNode* gpuNode = NULL;
  NCCLCHECK(xmlGetSub(pciNode, "gpu", &gpuNode));
  if (gpuNode == NULL) NCCLCHECK(xmlAddNode(xml, pciNode, "gpu", &gpuNode));

  int index = -1;

  int dev = -1;
  NCCLCHECK(xmlGetAttrIndex(gpuNode, "dev", &index));
  if (index == -1) {
    if (nvmlDev == NULL) {
      const char* busId;
      NCCLCHECK(xmlGetAttr(pciNode, "busid", &busId));
      if (busId == NULL || cudaDeviceGetByPCIBusId(&dev, busId) != cudaSuccess) dev = -1;
    } else {
      NCCLCHECK(ncclNvmlDeviceGetIndex(nvmlDev, (unsigned int*)&dev));
    }
    NCCLCHECK(xmlSetAttrInt(gpuNode, "dev", dev));
  }
  NCCLCHECK(xmlGetAttrInt(gpuNode, "dev", &dev));
  if (dev == -1) { *gpuNodeRet = NULL; return ncclSuccess; }

  NCCLCHECK(xmlGetAttrIndex(gpuNode, "sm", &index));
  if (index == -1) {
    int cudaMajor, cudaMinor;
    if (nvmlDev == NULL) {
      cudaDeviceProp devProp;
      CUDACHECK(cudaGetDeviceProperties(&devProp, dev));
      cudaMajor = devProp.major; cudaMinor = devProp.minor;
    } else {
      NCCLCHECK(ncclNvmlDeviceGetCudaComputeCapability(nvmlDev, &cudaMajor, &cudaMinor));
    }
    NCCLCHECK(xmlSetAttrInt(gpuNode, "sm", cudaMajor*10+cudaMinor));
  }
  int sm;
  NCCLCHECK(xmlGetAttrInt(gpuNode, "sm", &sm));

  struct ncclXmlNode* nvlNode = NULL;
  NCCLCHECK(xmlGetSub(gpuNode, "nvlink", &nvlNode));
  if (nvlNode == NULL) {
    // NVML NVLink detection
    int maxNvLinks = (sm < 60) ? 0 : (sm < 70) ? 4 : (sm < 80) ? 6 : 12;

    if (maxNvLinks > 0 && nvmlDev == NULL) {
      WARN("No NVML device handle. Skipping nvlink detection.");
      maxNvLinks = 0;
    }

    for (int l=0; l<maxNvLinks; ++l) {
      // Check whether we can use this NVLink for P2P
      unsigned canP2P;
      if ((ncclNvmlDeviceGetNvLinkCapability(nvmlDev, l, NVML_NVLINK_CAP_P2P_SUPPORTED, &canP2P) != ncclSuccess) || !canP2P) continue;

      // Make sure the Nvlink is up. The previous call should have trained the link.
      nvmlEnableState_t isActive;
      if ((ncclNvmlDeviceGetNvLinkState(nvmlDev, l, &isActive) != ncclSuccess) || (isActive != NVML_FEATURE_ENABLED)) continue;

      // Try to figure out what's on the other side of the NVLink
      nvmlPciInfo_t remoteProc;
      if (ncclNvmlDeviceGetNvLinkRemotePciInfo(nvmlDev, l, &remoteProc) != ncclSuccess) continue;

      // Make a lower case copy of the bus ID for calling ncclDeviceType
      // PCI system path is in lower case
      char* p = remoteProc.busId;
      char lowerId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
      for (int c=0; c<NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE; c++) {
        lowerId[c] = tolower(p[c]);
        if (p[c] == 0) break;
      }

      NCCLCHECK(xmlGetSubKv(gpuNode, "nvlink", &nvlNode, "target", lowerId));
      if (nvlNode == NULL) {
        NCCLCHECK(xmlAddNode(xml, gpuNode, "nvlink", &nvlNode));
        NCCLCHECK(xmlSetAttr(nvlNode, "target", lowerId));
        NCCLCHECK(xmlSetAttrInt(nvlNode, "count", 1));
      } else {
        int count;
        NCCLCHECK(xmlGetAttrInt(nvlNode, "count", &count));
        NCCLCHECK(xmlSetAttrInt(nvlNode, "count", count+1));
      }
    }
  }
  // Fill target classes
  for (int s=0; s<gpuNode->nSubs; s++) {
    struct ncclXmlNode* sub = gpuNode->subs[s];
    if (strcmp(sub->name, "nvlink") != 0) continue;
    int index;
    NCCLCHECK(xmlGetAttrIndex(sub, "tclass", &index));
    if (index == -1) {
      const char* busId;
      NCCLCHECK(xmlGetAttr(sub, "target", &busId));
      char* path;
      ncclDebugNoWarn = NCCL_GRAPH;
      getPciPath(busId, &path);
      ncclDebugNoWarn = 0;
      if (path == NULL || strcmp(busId, "fffffff:ffff:ff") == 0) {
        // Remote NVLink device is not visible inside this VM. Assume NVSwitch.
        NCCLCHECK(xmlSetAttr(sub, "tclass", "0x068000"));
      } else {
        NCCLCHECK(ncclTopoSetAttrFromSys(sub, path, "class", "tclass"));
        free(path);
      }
    }
  }
  *gpuNodeRet = gpuNode;
  return ncclSuccess;
}
```

</details>

<a id="f0558"></a>

## F0558：ncclTopoFillGpu

用途：在 XML 找到或添加指定 GPU，触发系统/NVML 自动发现补全其属性。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct ncclXmlNode* node;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetPciNode(xml, busId, &node));</code> | 失败时可能提前return，成功才继续；按 PCI 地址在 XML 中查找已有节点，避免同一设备被重复创建。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrIfUnset(node, "class", "0x03"));</code> | 失败时可能提前return，成功才继续；只给缺失属性设置默认值，避免自动发现结果覆盖用户明确配置。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromSys(node, xml));</code> | 失败时可能提前return，成功才继续；沿 sysfs 真实设备路径建立 PCI 父层级，并读取链路速度/宽度等属性。 |
| 5 | 0 / 准备局部状态 | <code>nvmlDevice_t nvmlDev = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(ncclNvmlDeviceGetHandleByPciBusId(busId, &nvmlDev) != ncclSuccess)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>nvmlDev = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromGpu(node, nvmlDev, xml, gpuNode));</code> | 失败时可能提前return，成功才继续；利用 CUDA/NVML 信息补齐 GPU 的 PCI、计算能力和 NVLink 属性。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct ncclXmlNode* node;
  NCCLCHECK(ncclTopoGetPciNode(xml, busId, &node));
  NCCLCHECK(xmlSetAttrIfUnset(node, "class", "0x03"));
  NCCLCHECK(ncclTopoGetXmlFromSys(node, xml));
  nvmlDevice_t nvmlDev = NULL;
  if (ncclNvmlDeviceGetHandleByPciBusId(busId, &nvmlDev) != ncclSuccess) nvmlDev = NULL;
  NCCLCHECK(ncclTopoGetXmlFromGpu(node, nvmlDev, xml, gpuNode));
  return ncclSuccess;
}
```

</details>

<a id="f0559"></a>

## F0559：ncclTopoGetSubsystem

用途：读取 PCI 子系统标识，辅助识别设备类别与硬件拓扑特殊处理。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char subSysPath[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>sprintf(subSysPath, "%s/subsystem", sysPath);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 准备局部状态 | <code>char* path = realpath(subSysPath, NULL);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 条件判断 | <code>(path == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>subSys[0] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 8 | 2 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 循环 | <code>初始 offset = strlen(path)；继续条件 offset &gt; 0 && path[offset] != '/'；每轮更新 offset--</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 11 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>strcpy(subSys, path+offset+1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>free(path);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char subSysPath[PATH_MAX];
  sprintf(subSysPath, "%s/subsystem", sysPath);
  char* path = realpath(subSysPath, NULL);
  if (path == NULL) {
    subSys[0] = '\0';
  } else {
    int offset;
    for (offset = strlen(path); offset > 0 && path[offset] != '/'; offset--);
    strcpy(subSys, path+offset+1);
    free(path);
  }
  return ncclSuccess;
}
```

</details>

<a id="f0560"></a>

## F0560：ncclTopoFillNet

用途：将后端提供的网络设备属性补入 XML，关联其 PCI/NIC 层次与 GDR/CollNet 能力。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlFindTagKv(xml, "net", netNode, "name", netName));</code> | 失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。 |
| 2 | 0 / 条件判断 | <code>(*netNode != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>const char* pciSysPath = pciPath;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 条件判断 | <code>(pciSysPath)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 7 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 8 | 2 / 准备局部状态 | <code>char subSystem[PATH_MAX];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetSubsystem(pciSysPath, subSystem));</code> | 失败时可能提前return，成功才继续；读取 PCI 子系统标识，辅助识别设备类别与硬件拓扑特殊处理。 |
| 10 | 2 / 条件判断 | <code>(strcmp(subSystem, "pci") != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 4 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Topology detection: network path %s is not a PCI device (%s). Attaching to first CPU", pciSysPath, subSystem);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 4 / 调用 / 计算 / 写入 | <code>pciSysPath = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 0 / 准备局部状态 | <code>struct ncclXmlNode* parent = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 条件判断 | <code>(pciSysPath)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 准备局部状态 | <code>int offset;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 18 | 2 / 循环 | <code>初始 offset=strlen(pciSysPath)-1；继续条件 pciSysPath[offset] != '/'；每轮更新 offset--</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 19 | 3 / 调用 / 计算 / 写入 | <code>;</code> | 执行这段本地计算/控制片段；保持源码表达式的求值顺序与类型/单位。 |
| 20 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 21 | 2 / 准备局部状态 | <code>char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>strcpy(busId, pciSysPath+offset+1);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetPciNode(xml, busId, &parent));</code> | 失败时可能提前return，成功才继续；按 PCI 地址在 XML 中查找已有节点，避免同一设备被重复创建。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttrIfUnset(parent, "class", "0x02"));</code> | 失败时可能提前return，成功才继续；只给缺失属性设置默认值，避免自动发现结果覆盖用户明确配置。 |
| 25 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoGetXmlFromSys(parent, xml));</code> | 失败时可能提前return，成功才继续；沿 sysfs 真实设备路径建立 PCI 父层级，并读取链路速度/宽度等属性。 |
| 26 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 27 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlFindTag(xml, "cpu", &parent));</code> | 失败时可能提前return，成功才继续；搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。 |
| 28 | 0 / 准备局部状态 | <code>struct ncclXmlNode* nicNode = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 29 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetSub(parent, "nic", &nicNode));</code> | 失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。 |
| 30 | 0 / 条件判断 | <code>(nicNode == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 31 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 32 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, parent, "nic", &nicNode));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 33 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlAddNode(xml, nicNode, "net", netNode));</code> | 失败时可能提前return，成功才继续；从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。 |
| 34 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlSetAttr(*netNode, "name", netName));</code> | 失败时可能提前return，成功才继续；在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。 |
| 35 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(xmlFindTagKv(xml, "net", netNode, "name", netName));
  if (*netNode != NULL) return ncclSuccess;

  const char* pciSysPath = pciPath;
  if (pciSysPath) {
    char subSystem[PATH_MAX];
    NCCLCHECK(ncclTopoGetSubsystem(pciSysPath, subSystem));
    // This is not a PCI device (virtual, usb, ...).
    if (strcmp(subSystem, "pci") != 0) {
      INFO(NCCL_GRAPH, "Topology detection: network path %s is not a PCI device (%s). Attaching to first CPU", pciSysPath, subSystem);
      pciSysPath = NULL;
    }
  }

  struct ncclXmlNode* parent = NULL;
  if (pciSysPath) {
    int offset;
    for (offset=strlen(pciSysPath)-1; pciSysPath[offset] != '/'; offset--);
    char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
    strcpy(busId, pciSysPath+offset+1);
    NCCLCHECK(ncclTopoGetPciNode(xml, busId, &parent));
    NCCLCHECK(xmlSetAttrIfUnset(parent, "class", "0x02"));
    NCCLCHECK(ncclTopoGetXmlFromSys(parent, xml));
  } else {
    // Virtual NIC, no PCI device, attach to first CPU
    NCCLCHECK(xmlFindTag(xml, "cpu", &parent));
  }

  struct ncclXmlNode* nicNode = NULL;
  NCCLCHECK(xmlGetSub(parent, "nic", &nicNode));
  if (nicNode == NULL) {
    NCCLCHECK(xmlAddNode(xml, parent, "nic", &nicNode));
  }

  // We know that this net does not exist yet (we searched for it at the
  // beginning of this function), so we can add it.
  NCCLCHECK(xmlAddNode(xml, nicNode, "net", netNode));
  NCCLCHECK(xmlSetAttr(*netNode, "name", netName));
  return ncclSuccess;
}
```

</details>

<a id="f0561"></a>

## F0561：ncclTopoTrimXmlRec

用途：裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(node, "keep", &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 3 | 0 / 条件判断 | <code>(str && strcmp(str, "1") == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlUnsetAttr(node, "keep"));</code> | 失败时可能提前return，成功才继续；删除指定属性并整理数组，防止遗留键值参与之后的匹配/导出。 |
| 6 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 7 | 2 / 准备局部状态 | <code>struct ncclXmlNode* subs[MAX_SUBS];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 2 / 准备局部状态 | <code>int nSubs = node-&gt;nSubs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>memcpy(subs, node-&gt;subs, node-&gt;nSubs*sizeof(struct ncclXmlNode*));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 2 / 循环 | <code>初始 int s=0;；继续条件 s&lt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoTrimXmlRec(subs[s]));</code> | 失败时可能提前return，成功才继续；裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。 |
| 12 | 2 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 2 / 条件判断 | <code>(node-&gt;nSubs == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 14 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 15 | 4 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlRemoveNode(node));</code> | 失败时可能提前return，成功才继续；从父节点子项表摘除目标节点并整理关系，裁剪不需要的拓扑分支。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const char* str;
  NCCLCHECK(xmlGetAttr(node, "keep", &str));
  if (str && strcmp(str, "1") == 0) {
    NCCLCHECK(xmlUnsetAttr(node, "keep"));
  } else {
    // Copy nSubs and subs as they could change as we trim recursively.
    struct ncclXmlNode* subs[MAX_SUBS];
    int nSubs = node->nSubs;
    memcpy(subs, node->subs, node->nSubs*sizeof(struct ncclXmlNode*));
    for (int s=0; s<nSubs; s++) {
      NCCLCHECK(ncclTopoTrimXmlRec(subs[s]));
    }
    if (node->nSubs == 0) NCCLCHECK(xmlRemoveNode(node));
  }
  return ncclSuccess;
}
```

</details>

<a id="f0562"></a>

## F0562：ncclTopoTrimXml

用途：裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(ncclTopoTrimXmlRec(xml-&gt;nodes));</code> | 失败时可能提前return，成功才继续；裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(ncclTopoTrimXmlRec(xml->nodes));
  return ncclSuccess;
}
```

</details>

<a id="f0563"></a>

## F0563：ncclTopoXmlGraphLoadGpu

用途：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));
  return ncclSuccess;
}
```

</details>

<a id="f0564"></a>

## F0564：ncclTopoXmlGraphLoadNet

用途：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 2 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(xmlLoadSub(file, xml, head, NULL, 0));
  return ncclSuccess;
}
```

</details>

<a id="f0565"></a>

## F0565：ncclTopoXmlGraphLoadChannel

用途：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "net", ncclTopoXmlGraphLoadNet }, { "gpu", ncclTopoXmlGraphLoadGpu } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 2));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct xmlHandler handlers[] = { { "net", ncclTopoXmlGraphLoadNet }, { "gpu", ncclTopoXmlGraphLoadGpu } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 2));
  return ncclSuccess;
}
```

</details>

<a id="f0566"></a>

## F0566：ncclTopoXmlGraphLoadGraph

用途：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "channel", ncclTopoXmlGraphLoadChannel } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 3 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct xmlHandler handlers[] = { { "channel", ncclTopoXmlGraphLoadChannel } };
  NCCLCHECK(xmlLoadSub(file, xml, head, handlers, 1));
  return ncclSuccess;
}
```

</details>

<a id="f0567"></a>

## F0567：ncclTopoXmlGraphLoadGraphs

用途：解析对应层级的通信图 XML 标签，与硬件系统 XML 加载路径分开。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int version;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrInt(head, "version", &version));</code> | 失败时可能提前return，成功才继续；读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。 |
| 3 | 0 / 条件判断 | <code>(version != NCCL_GRAPH_XML_VERSION)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>WARN("XML Graph has wrong version %d, %d needed", version, NCCL_GRAPH_XML_VERSION);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 函数返回 | <code>return ncclInvalidUsage;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 准备局部状态 | <code>const char* name;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(head, "name", &name));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 9 | 0 / 条件判断 | <code>(name != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Loading graphs for topology %s", name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "Loading graphs");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "graph", ncclTopoXmlGraphLoadGraph } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xmlGraph, head, handlers, 1));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int version;
  NCCLCHECK(xmlGetAttrInt(head, "version", &version));
  if (version != NCCL_GRAPH_XML_VERSION) {
    WARN("XML Graph has wrong version %d, %d needed", version, NCCL_GRAPH_XML_VERSION);
    return ncclInvalidUsage;
  }
  const char* name;
  NCCLCHECK(xmlGetAttr(head, "name", &name));
  if (name != NULL) INFO(NCCL_GRAPH, "Loading graphs for topology %s", name);
  else INFO(NCCL_GRAPH, "Loading graphs");

  struct xmlHandler handlers[] = { { "graph", ncclTopoXmlGraphLoadGraph } };
  NCCLCHECK(xmlLoadSub(file, xmlGraph, head, handlers, 1));
  return ncclSuccess;
}
```

</details>

<a id="f0568"></a>

## F0568：ncclTopoGetXmlGraphFromFile

用途：打开并解析通信图 XML 文件，校验图版本以恢复预先保存的搜索结果。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>FILE* file = fopen(xmlGraphFile, "r");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(file == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("Could not open XML graph file %s : %s", xmlGraphFile, strerror(errno));</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 准备局部状态 | <code>struct xmlHandler handlers[] = { { "graphs", ncclTopoXmlGraphLoadGraphs } };</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>xml-&gt;maxIndex = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlLoadSub(file, xml, NULL, handlers, 1));</code> | 失败时可能提前return，成功才继续；递归读取节点子树并检查父子闭合关系，限制节点/子项容量。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>fclose(file);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  FILE* file = fopen(xmlGraphFile, "r");
  if (file == NULL) {
    WARN("Could not open XML graph file %s : %s", xmlGraphFile, strerror(errno));
    return ncclSystemError;
  }
  struct xmlHandler handlers[] = { { "graphs", ncclTopoXmlGraphLoadGraphs } };
  xml->maxIndex = 0;
  NCCLCHECK(xmlLoadSub(file, xml, NULL, handlers, 1));
  fclose(file);
  return ncclSuccess;
}
```

</details>

