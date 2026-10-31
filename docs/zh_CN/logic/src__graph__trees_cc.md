# src/graph/trees.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0526"></a>

## F0526：ncclGetBtree

用途：根据 rank 最低置位构造二叉树父/子编号，并处理非二次幂规模的越界孩子。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int up, down0, down1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>int bit;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 循环 | <code>初始 bit=1；继续条件 bit&lt;nranks；每轮更新 bit&lt;&lt;=1</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 4 | 1 / 条件判断 | <code>(bit & rank)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 3 / 控制转移 | <code>break;</code> | 退出当前最内层循环或switch，继续其后；并非结束整个函数。 |
| 7 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 8 | 0 / 条件判断 | <code>(rank == 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 9 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 10 | 2 / 调用 / 计算 / 写入 | <code>*u = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 2 / 调用 / 计算 / 写入 | <code>*d0 = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>*d1 = nranks &gt; 1 ? bit &gt;&gt; 1 : -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 13 | 2 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 14 | 0 / 调用 / 计算 / 写入 | <code>up = (rank ^ bit) &#124; (bit &lt;&lt; 1);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 0 / 条件判断 | <code>(up &gt;= nranks)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 16 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>up = (rank ^ bit);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>*parentChildType = (rank &lt; up) ? 0 : 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 19 | 0 / 调用 / 计算 / 写入 | <code>*u = up;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 20 | 0 / 准备局部状态 | <code>int lowbit = bit &gt;&gt; 1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>down0 = lowbit == 0 ? -1 : rank-lowbit;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 调用 / 计算 / 写入 | <code>down1 = lowbit == 0 ? -1 : rank+lowbit;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 0 / 循环 | <code>初始 ；继续条件 (down1 &gt;= nranks)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>down1 = lowbit == 0 ? -1 : rank+lowbit;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>lowbit &gt;&gt;= 1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 27 | 0 / 调用 / 计算 / 写入 | <code>*d0 = down0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 28 | 0 / 调用 / 计算 / 写入 | <code>*d1 = down1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 29 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int up, down0, down1;
  int bit;
  for (bit=1; bit<nranks; bit<<=1) {
    if (bit & rank) break;
  }

  if (rank == 0) {
    *u = -1;
    *d0 = -1;
    // Child rank is > 0 so it has to be our child 1, not 0.
    *d1 = nranks > 1 ? bit >> 1 : -1;
    return ncclSuccess;
  }

  up = (rank ^ bit) | (bit << 1);
  // if smaller than the parent, we are his first child, otherwise we're his second
  if (up >= nranks) up = (rank ^ bit);
  *parentChildType = (rank < up) ? 0 : 1;
  *u = up;

  int lowbit = bit >> 1;
  // down0 is always within bounds
  down0 = lowbit == 0 ? -1 : rank-lowbit;

  down1 = lowbit == 0 ? -1 : rank+lowbit;
  // Make sure down1 is within bounds
  while (down1 >= nranks) {
    down1 = lowbit == 0 ? -1 : rank+lowbit;
    lowbit >>= 1;
  }
  *d0 = down0; *d1 = down1;

  return ncclSuccess;
}
```

</details>

<a id="f0527"></a>

## F0527：ncclGetDtree

用途：在第一棵二叉树基础上用镜像或平移构造互补树，分摊中间节点负载。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>ncclGetBtree(nranks, rank, s0, d0_0, d0_1, parentChildType0);</code> | 根据 rank 最低置位构造二叉树父/子编号，并处理非二次幂规模的越界孩子。 |
| 2 | 0 / 条件判断 | <code>(nranks % 2 == 1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 准备局部状态 | <code>int shiftrank = (rank-1+nranks) % nranks;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 5 | 2 / 准备局部状态 | <code>int u, d0, d1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>ncclGetBtree(nranks, shiftrank, &u, &d0, &d1, parentChildType1);</code> | 根据 rank 最低置位构造二叉树父/子编号，并处理非二次幂规模的越界孩子。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>*s1 = u == -1 ? -1 : (u+1) % nranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>*d1_0 = d0 == -1 ? -1 : (d0+1) % nranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 9 | 2 / 调用 / 计算 / 写入 | <code>*d1_1 = d1 == -1 ? -1 : (d1+1) % nranks;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 11 | 2 / 准备局部状态 | <code>int u, d0, d1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 12 | 2 / 调用 / 计算 / 写入 | <code>ncclGetBtree(nranks, nranks-1-rank, &u, &d0, &d1, parentChildType1);</code> | 根据 rank 最低置位构造二叉树父/子编号，并处理非二次幂规模的越界孩子。 |
| 13 | 2 / 调用 / 计算 / 写入 | <code>*s1 = u == -1 ? -1 : nranks-1-u;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>*d1_0 = d0 == -1 ? -1 : nranks-1-d0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 15 | 2 / 调用 / 计算 / 写入 | <code>*d1_1 = d1 == -1 ? -1 : nranks-1-d1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  // First tree ... use a btree
  ncclGetBtree(nranks, rank, s0, d0_0, d0_1, parentChildType0);
  // Second tree ... mirror or shift
  if (nranks % 2 == 1) {
    // shift
    int shiftrank = (rank-1+nranks) % nranks;
    int u, d0, d1;
    ncclGetBtree(nranks, shiftrank, &u, &d0, &d1, parentChildType1);
    *s1 = u == -1 ? -1 : (u+1) % nranks;
    *d1_0 = d0 == -1 ? -1 : (d0+1) % nranks;
    *d1_1 = d1 == -1 ? -1 : (d1+1) % nranks;
  } else {
    // mirror
    int u, d0, d1;
    ncclGetBtree(nranks, nranks-1-rank, &u, &d0, &d1, parentChildType1);
    *s1 = u == -1 ? -1 : nranks-1-u;
    *d1_0 = d0 == -1 ? -1 : nranks-1-d0;
    *d1_1 = d1 == -1 ? -1 : nranks-1-d1;
  }
  return ncclSuccess;
}
```

</details>

