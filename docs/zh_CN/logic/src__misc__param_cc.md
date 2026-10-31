# src/misc/param.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0957"></a>

## F0957：userHomeDir

用途：按当前用户 id 查询用户目录，给用户级 NCCL 配置文件构造路径。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>struct passwd *pwUser = getpwuid(getuid());</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 函数返回 | <code>return pwUser == NULL ? NULL : pwUser-&gt;pw_dir;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  struct passwd *pwUser = getpwuid(getuid());
  return pwUser == NULL ? NULL : pwUser->pw_dir;
}
```

</details>

<a id="f0958"></a>

## F0958：setEnvFile

用途：读取 KEY=VALUE 配置行并只填入尚未存在的环境键，保留启动环境的优先级。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>FILE * file = fopen(fileName, "r");</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(file == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 函数返回 | <code>return;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 5 | 0 / 准备局部状态 | <code>char *line = NULL;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 0 / 准备局部状态 | <code>char envVar[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 0 / 准备局部状态 | <code>char envValue[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 8 | 0 / 准备局部状态 | <code>size_t n = 0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 9 | 0 / 准备局部状态 | <code>ssize_t read;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 10 | 0 / 循环 | <code>初始 ；继续条件 ((read = getline(&line, &n, file)) != -1)；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 11 | 1 / 条件判断 | <code>(line[read-1] == '\n')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 3 / 调用 / 计算 / 写入 | <code>line[read-1] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 1 / 准备局部状态 | <code>int s=0;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 15 | 1 / 循环 | <code>初始 ；继续条件 (line[s] != '\0' && line[s] != '=')；每轮更新</code> | 每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。 |
| 16 | 2 / 调用 / 计算 / 写入 | <code>s++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 17 | 1 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 18 | 1 / 条件判断 | <code>(line[s] == '\0')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 19 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 20 | 3 / 控制转移 | <code>continue;</code> | 跳到当前最内层循环的下一轮，for还会执行其增量步骤。 |
| 21 | 1 / 调用 / 计算 / 写入 | <code>strncpy(envVar, line, std::min(1023,s));</code> | 对不同整型宽度取较小有效范围，避免最后一块读取超过剩余元素。 |
| 22 | 1 / 调用 / 计算 / 写入 | <code>envVar[s] = '\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 23 | 1 / 调用 / 计算 / 写入 | <code>s++;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 1 / 调用 / 计算 / 写入 | <code>strncpy(envValue, line+s, 1023);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 1 / 调用 / 计算 / 写入 | <code>envValue[1023]='\0';</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 26 | 1 / 调用 / 计算 / 写入 | <code>setenv(envVar, envValue, 0);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 27 | 0 / 循环出口 | <code></code> | 条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。 |
| 28 | 0 / 条件判断 | <code>(line)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 29 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 30 | 2 / 调用 / 计算 / 写入 | <code>free(line);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 31 | 0 / 调用 / 计算 / 写入 | <code>fclose(file);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  FILE * file = fopen(fileName, "r");
  if (file == NULL) return;

  char *line = NULL;
  char envVar[1024];
  char envValue[1024];
  size_t n = 0;
  ssize_t read;
  while ((read = getline(&line, &n, file)) != -1) {
    if (line[read-1] == '\n') line[read-1] = '\0';
    int s=0; // Env Var Size
    while (line[s] != '\0' && line[s] != '=') s++;
    if (line[s] == '\0') continue;
    strncpy(envVar, line, std::min(1023,s));
    envVar[s] = '\0';
    s++;
    strncpy(envValue, line+s, 1023);
    envValue[1023]='\0';
    setenv(envVar, envValue, 0);
    //printf("%s : %s->%s\n", fileName, envVar, envValue);
  }
  if (line) free(line);
  fclose(file);
}
```

</details>

<a id="f0959"></a>

## F0959：initEnv

用途：按用户配置、系统配置顺序加载配置文件，已有键不会被后读文件覆盖。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>char confFilePath[1024];</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>const char * userDir = userHomeDir();</code> | 按当前用户 id 查询用户目录，给用户级 NCCL 配置文件构造路径。 |
| 3 | 0 / 条件判断 | <code>(userDir)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 调用 / 计算 / 写入 | <code>sprintf(confFilePath, "%s/.nccl.conf", userDir);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>setEnvFile(confFilePath);</code> | 读取 KEY=VALUE 配置行并只填入尚未存在的环境键，保留启动环境的优先级。 |
| 7 | 0 / 调用 / 计算 / 写入 | <code>sprintf(confFilePath, "/etc/nccl.conf");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 8 | 0 / 调用 / 计算 / 写入 | <code>setEnvFile(confFilePath);</code> | 读取 KEY=VALUE 配置行并只填入尚未存在的环境键，保留启动环境的优先级。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  char confFilePath[1024];
  const char * userDir = userHomeDir();
  if (userDir) {
    sprintf(confFilePath, "%s/.nccl.conf", userDir);
    setEnvFile(confFilePath);
  }
  sprintf(confFilePath, "/etc/nccl.conf");
  setEnvFile(confFilePath);
}
```

</details>

<a id="f0960"></a>

## F0960：ncclLoadParam

用途：在互斥锁内首次读取/解析数值参数并写原子缓存，后续 getter 直接复用缓存值。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_lock(&mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 3 | 0 / 条件判断 | <code>(__atomic_load_n(cache, __ATOMIC_RELAXED) == uninitialized)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 2 / 准备局部状态 | <code>char* str = getenv(env);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 6 | 2 / 准备局部状态 | <code>int64_t value = deftVal;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 7 | 2 / 条件判断 | <code>(str && strlen(str) &gt; 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 8 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 9 | 4 / 调用 / 计算 / 写入 | <code>errno = 0;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 4 / 调用 / 计算 / 写入 | <code>value = strtoll(str, nullptr, 0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 11 | 4 / 条件判断 | <code>(errno)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 12 | 5 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 13 | 6 / 调用 / 计算 / 写入 | <code>value = deftVal;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 14 | 6 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ALL,"Invalid value %s for %s, using default %lld.", str, env, (long long)deftVal);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 15 | 5 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 16 | 6 / 调用 / 计算 / 写入 | <code>INFO(NCCL_ALL,"%s set by environment to %lld.", env, (long long)value);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 2 / 调用 / 计算 / 写入 | <code>__atomic_store_n(cache, value, __ATOMIC_RELAXED);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 18 | 0 / 调用 / 计算 / 写入 | <code>pthread_mutex_unlock(&mutex);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mutex);
  if (__atomic_load_n(cache, __ATOMIC_RELAXED) == uninitialized) {
    char* str = getenv(env);
    int64_t value = deftVal;
    if (str && strlen(str) > 0) {
      errno = 0;
      value = strtoll(str, nullptr, 0);
      if (errno) {
        value = deftVal;
        INFO(NCCL_ALL,"Invalid value %s for %s, using default %lld.", str, env, (long long)deftVal);
      } else {
        INFO(NCCL_ALL,"%s set by environment to %lld.", env, (long long)value);
      }
    }
    __atomic_store_n(cache, value, __ATOMIC_RELAXED);
  }
  pthread_mutex_unlock(&mutex);
}
```

</details>

