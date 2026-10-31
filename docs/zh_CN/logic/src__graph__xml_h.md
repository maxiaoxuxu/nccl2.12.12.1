# src/graph/xml.h：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0569"></a>

## F0569：xmlGetAttrIndex

用途：按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*index = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 准备局部状态 | <code>const int nAttrs = node-&gt;nAttrs;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 int a=0;；继续条件 a&lt;nAttrs；每轮更新 a++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(strncmp(node-&gt;attrs[a].key, attrName, MAX_STR_LEN) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>*index = a;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *index = -1;
  const int nAttrs = node->nAttrs;
  for (int a=0; a<nAttrs; a++) {
    if (strncmp(node->attrs[a].key, attrName, MAX_STR_LEN) == 0) {
      *index = a;
      return ncclSuccess;
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0570"></a>

## F0570：xmlGetAttr

用途：读取可选字符串属性，缺失时返回空指针而不自动填默认值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*value = index == -1 ? NULL : node-&gt;attrs[index].value;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  *value = index == -1 ? NULL : node->attrs[index].value;
  return ncclSuccess;
}
```

</details>

<a id="f0571"></a>

## F0571：xmlGetAttrStr

用途：读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(node, attrName, value));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 2 | 0 / 条件判断 | <code>(*value == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>WARN("Attribute %s of node %s not found", attrName, node-&gt;name);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 5 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  NCCLCHECK(xmlGetAttr(node, attrName, value));
  if (*value == NULL) {
    WARN("Attribute %s of node %s not found", attrName, node->name);
    return ncclInternalError;
  }
  return ncclSuccess;
}
```

</details>

<a id="f0572"></a>

## F0572：xmlGetAttrInt

用途：读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(node, attrName, &str));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*value = strtol(str, NULL, 0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const char* str;
  NCCLCHECK(xmlGetAttrStr(node, attrName, &str));
  *value = strtol(str, NULL, 0);
  return ncclSuccess;
}
```

</details>

<a id="f0573"></a>

## F0573：xmlGetAttrIntDefault

用途：读取可选整数属性，缺失时写调用者指定的默认值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(node, attrName, &str));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*value = str ? strtol(str, NULL, 0) : defaultValue;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const char* str;
  NCCLCHECK(xmlGetAttr(node, attrName, &str));
  *value = str ? strtol(str, NULL, 0) : defaultValue;
  return ncclSuccess;
}
```

</details>

<a id="f0574"></a>

## F0574：xmlGetAttrFloat

用途：读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>const char* str;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrStr(node, attrName, &str));</code> | 失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>*value = strtof(str, NULL);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  const char* str;
  NCCLCHECK(xmlGetAttrStr(node, attrName, &str));
  *value = strtof(str, NULL);
  return ncclSuccess;
}
```

</details>

<a id="f0575"></a>

## F0575：xmlFindTag

用途：搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*node = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;xml-&gt;maxIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 准备局部状态 | <code>struct ncclXmlNode* n = xml-&gt;nodes+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 条件判断 | <code>(strcmp(n-&gt;name, tagName) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 调用 / 计算 / 写入 | <code>*node = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 9 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *node = NULL;
  for (int i=0; i<xml->maxIndex; i++) {
    struct ncclXmlNode* n = xml->nodes+i;
    if (strcmp(n->name, tagName) == 0) {
      *node = n;
      return ncclSuccess;
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0576"></a>

## F0576：xmlFindTagKv

用途：搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*node = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 循环 | <code>初始 int i=0;；继续条件 i&lt;xml-&gt;maxIndex；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 准备局部状态 | <code>struct ncclXmlNode* n = xml-&gt;nodes+i;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 条件判断 | <code>(strcmp(n-&gt;name, tagName) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 准备局部状态 | <code>const char* value;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(n, attrName, &value));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 8 | 3 / 条件判断 | <code>(value && strcmp(value, attrValue) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 5 / 调用 / 计算 / 写入 | <code>*node = n;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 5 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *node = NULL;
  for (int i=0; i<xml->maxIndex; i++) {
    struct ncclXmlNode* n = xml->nodes+i;
    if (strcmp(n->name, tagName) == 0) {
      const char* value;
      NCCLCHECK(xmlGetAttr(n, attrName, &value));
      if (value && strcmp(value, attrValue) == 0) {
        *node = n;
        return ncclSuccess;
      }
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0577"></a>

## F0577：xmlSetAttr

用途：在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>index = node-&gt;nAttrs++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].key, attrName, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>node-&gt;attrs[index].key[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].value, value, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;attrs[index].value[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    node->attrs[index].key[MAX_STR_LEN] = '\0';
  }
  strncpy(node->attrs[index].value, value, MAX_STR_LEN);
  node->attrs[index].value[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}
```

</details>

<a id="f0578"></a>

## F0578：xmlSetAttrIfUnset

用途：只给缺失属性设置默认值，避免自动发现结果覆盖用户明确配置。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>index = node-&gt;nAttrs++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].key, attrName, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;attrs[index].key[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].value, value, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;attrs[index].value[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index != -1) return ncclSuccess;
  index = node->nAttrs++;
  strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
  node->attrs[index].key[MAX_STR_LEN] = '\0';
  strncpy(node->attrs[index].value, value, MAX_STR_LEN);
  node->attrs[index].value[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}
```

</details>

<a id="f0579"></a>

## F0579：xmlSetAttrInt

用途：把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>index = node-&gt;nAttrs++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].key, attrName, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>node-&gt;attrs[index].key[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>snprintf(node-&gt;attrs[index].value, MAX_STR_LEN, "%d", value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;attrs[index].value[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    node->attrs[index].key[MAX_STR_LEN] = '\0';
  }
  snprintf(node->attrs[index].value, MAX_STR_LEN, "%d", value);
  node->attrs[index].value[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}
```

</details>

<a id="f0580"></a>

## F0580：xmlSetAttrFloat

用途：把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>index = node-&gt;nAttrs++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>strncpy(node-&gt;attrs[index].key, attrName, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>node-&gt;attrs[index].key[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>snprintf(node-&gt;attrs[index].value, MAX_STR_LEN, "%g", value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;attrs[index].value[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    node->attrs[index].key[MAX_STR_LEN] = '\0';
  }
  snprintf(node->attrs[index].value, MAX_STR_LEN, "%g", value);
  node->attrs[index].value[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}
```

</details>

<a id="f0581"></a>

## F0581：xmlUnsetAttr

用途：删除指定属性并整理数组，防止遗留键值参与之后的匹配/导出。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int index;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));</code> | 失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。 |
| 3 | 0 / 条件判断 | <code>(index == -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 循环 | <code>初始 int i=index+1;；继续条件 i&lt;node-&gt;nAttrs；每轮更新 i++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>strcpy(node-&gt;attrs[i-1].key, node-&gt;attrs[i].key);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 1 / 调用 / 计算 / 写入 | <code>strcpy(node-&gt;attrs[i-1].value, node-&gt;attrs[i].value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 9 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;nAttrs--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) return ncclSuccess;
  for (int i=index+1; i<node->nAttrs; i++) {
    strcpy(node->attrs[i-1].key, node->attrs[i].key);
    strcpy(node->attrs[i-1].value, node->attrs[i].value);
  }
  node->nAttrs--;
  return ncclSuccess;
}
```

</details>

<a id="f0582"></a>

## F0582：xmlGetSub

用途：仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*sub = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;node-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(strcmp(node-&gt;subs[s]-&gt;name, subName) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>*sub = node-&gt;subs[s];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *sub = NULL;
  for (int s=0; s<node->nSubs; s++) {
    if (strcmp(node->subs[s]->name, subName) == 0) {
      *sub = node->subs[s];
      return ncclSuccess;
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0583"></a>

## F0583：xmlGetSubKv

用途：仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*sub = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;node-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 准备局部状态 | <code>struct ncclXmlNode* subNode = node-&gt;subs[s];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 1 / 条件判断 | <code>(strcmp(subNode-&gt;name, subName) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 准备局部状态 | <code>const char* value;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 3 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetAttr(subNode, attrName, &value));</code> | 失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。 |
| 8 | 3 / 条件判断 | <code>(value && strcmp(value, attrValue) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 5 / 调用 / 计算 / 写入 | <code>*sub = node-&gt;subs[s];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 5 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 12 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 13 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *sub = NULL;
  for (int s=0; s<node->nSubs; s++) {
    struct ncclXmlNode* subNode = node->subs[s];
    if (strcmp(subNode->name, subName) == 0) {
      const char* value;
      NCCLCHECK(xmlGetAttr(subNode, attrName, &value));
      if (value && strcmp(value, attrValue) == 0) {
        *sub = node->subs[s];
        return ncclSuccess;
      }
    }
  }
  return ncclSuccess;
}
```

</details>

<a id="f0584"></a>

## F0584：xmlGetSubKvInt

用途：仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char strValue[10];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>snprintf(strValue, 10, "%d", attrValue);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECK(xmlGetSubKv(node, subName, sub, attrName, strValue));</code> | 失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char strValue[10];
  snprintf(strValue, 10, "%d", attrValue);
  NCCLCHECK(xmlGetSubKv(node, subName, sub, attrName, strValue));
  return ncclSuccess;
}
```

</details>

<a id="f0585"></a>

## F0585：xmlAddNode

用途：从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(xml-&gt;maxIndex == MAX_NODES)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>WARN("Error : too many XML nodes (max %d)", MAX_NODES);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 4 | 2 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>struct ncclXmlNode* s = xml-&gt;nodes+xml-&gt;maxIndex++;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 调用 / 计算 / 写入 | <code>s-&gt;nSubs = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>s-&gt;nAttrs = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>*sub = s;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>s-&gt;parent = parent;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 条件判断 | <code>(parent)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 11 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>parent-&gt;subs[parent-&gt;nSubs++] = s;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 0 / 调用 / 计算 / 写入 | <code>strncpy(s-&gt;name, subName, MAX_STR_LEN);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>s-&gt;name[MAX_STR_LEN] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (xml->maxIndex == MAX_NODES) {
    WARN("Error : too many XML nodes (max %d)", MAX_NODES);
    return ncclInternalError;
  }
  struct ncclXmlNode* s = xml->nodes+xml->maxIndex++;
  s->nSubs = 0;
  s->nAttrs = 0;
  *sub = s;
  s->parent = parent;
  if (parent) parent->subs[parent->nSubs++] = s;
  strncpy(s->name, subName, MAX_STR_LEN);
  s->name[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}
```

</details>

<a id="f0586"></a>

## F0586：xmlRemoveNode

用途：从父节点子项表摘除目标节点并整理关系，裁剪不需要的拓扑分支。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>node-&gt;type = NODE_TYPE_NONE;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 准备局部状态 | <code>struct ncclXmlNode* parent = node-&gt;parent;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 条件判断 | <code>(parent == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 准备局部状态 | <code>int shift = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 循环 | <code>初始 int s=0;；继续条件 s&lt;parent-&gt;nSubs；每轮更新 s++</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 8 | 1 / 条件判断 | <code>(parent-&gt;subs[s] == node)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 3 / 调用 / 计算 / 写入 | <code>shift = 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 12 | 3 / 条件判断 | <code>(shift)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 13 | 4 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 14 | 5 / 调用 / 计算 / 写入 | <code>parent-&gt;subs[s-1] = parent-&gt;subs[s];</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 16 | 0 / 调用 / 计算 / 写入 | <code>parent-&gt;nSubs--;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  node->type = NODE_TYPE_NONE;
  struct ncclXmlNode* parent = node->parent;
  if (parent == NULL) return ncclSuccess;
  int shift = 0;
  for (int s=0; s<parent->nSubs; s++) {
    if (parent->subs[s] == node) shift = 1;
    else if (shift) parent->subs[s-1] = parent->subs[s];
  }
  parent->nSubs--;
  return ncclSuccess;
}
```

</details>

<a id="f0587"></a>

## F0587：kvConvertToInt

用途：在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct kvDict* d = dict;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 ；继续条件 (d-&gt;str)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(strncmp(str, d-&gt;str, strlen(d-&gt;str)) == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>*value = d-&gt;value;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>d++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>INFO(NCCL_GRAPH, "KV Convert to int : could not find value of '%s' in dictionary, falling back to %d", str, d-&gt;value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 调用 / 计算 / 写入 | <code>*value = d-&gt;value;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct kvDict* d = dict;
  while (d->str) {
    if (strncmp(str, d->str, strlen(d->str)) == 0) {
      *value = d->value;
      return ncclSuccess;
    }
    d++;
  }
  INFO(NCCL_GRAPH, "KV Convert to int : could not find value of '%s' in dictionary, falling back to %d", str, d->value);
  *value = d->value;
  return ncclSuccess;
}
```

</details>

<a id="f0588"></a>

## F0588：kvConvertToStr

用途：在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct kvDict* d = dict;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 循环 | <code>初始 ；继续条件 (d-&gt;str)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 3 | 1 / 条件判断 | <code>(value == d-&gt;value)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 3 / 调用 / 计算 / 写入 | <code>*str = d-&gt;str;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 6 | 3 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 7 | 1 / 调用 / 计算 / 写入 | <code>d++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>WARN("KV Convert to str : could not find value %d in dictionary", value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 10 | 0 / 函数返回 | <code>return ncclInternalError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct kvDict* d = dict;
  while (d->str) {
    if (value == d->value) {
      *str = d->str;
      return ncclSuccess;
    }
    d++;
  }
  WARN("KV Convert to str : could not find value %d in dictionary", value);
  return ncclInternalError;
}
```

</details>

