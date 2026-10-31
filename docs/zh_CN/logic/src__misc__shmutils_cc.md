# src/misc/shmutils.cc：函数内部逻辑

[总索引](../function-logic-index.md) · [关键路径图解](../visual-guide.md)

以下是源码结构展开：编号表示阅读顺序，不是线程执行时间。缩进表示条件/循环嵌套；CHECK宏有隐藏的失败跳转。无法可靠拆分的片段会明确保留原文。

<a id="f0965"></a>

## F0965：shm_allocate

用途：包装 posix_fallocate，把其错误码返回约定转换成普通系统调用 -1/errno 风格。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int err = posix_fallocate(fd, 0, shmSize);</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 条件判断 | <code>(err)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 3 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 4 | 2 / 调用 / 计算 / 写入 | <code>errno = err;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 5 | 2 / 函数返回 | <code>return -1;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 6 | 0 / 函数返回 | <code>return 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int err = posix_fallocate(fd, 0, shmSize);
  if (err) { errno = err; return -1; }
  return 0;
}
```

</details>

<a id="f0966"></a>

## F0966：shm_map

用途：用 MAP_SHARED 映射已有文件页到本进程，失败返回系统调用风格错误。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 调用 / 计算 / 写入 | <code>*ptr = mmap(NULL, shmSize, PROT_READ &#124; PROT_WRITE, MAP_SHARED, fd, 0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 2 | 0 / 函数返回 | <code>return (*ptr == MAP_FAILED) ? -1 : 0;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  *ptr = mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  return (*ptr == MAP_FAILED) ? -1 : 0;
}
```

</details>

<a id="f0967"></a>

## F0967：ncclShmSetup

用途：创建/打开共享文件、设定尺寸并 mmap，创建者清零后关闭不再需要的文件描述符。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(create)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 条件判断 | <code>(shmPath[0] == '\0')</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 4 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 5 | 4 / 调用 / 计算 / 写入 | <code>sprintf(shmPath, "/dev/shm/nccl-XXXXXX");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 6 | 4 / 调用 / 计算 / 写入 | <code>*fd = mkstemp(shmPath);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 7 | 3 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 8 | 4 / 调用 / 计算 / 写入 | <code>SYSCHECKVAL(open(shmPath, O_CREAT &#124; O_RDWR, S_IRUSR &#124; S_IWUSR), "open", *fd);</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 9 | 2 / 条件判断 | <code>(ftruncate(*fd, shmSize) != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 10 | 3 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 11 | 4 / 调用 / 计算 / 写入 | <code>WARN("Error: failed to extend %s to %d bytes", shmPath, shmSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 12 | 4 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 13 | 1 / 假分支 | <code></code> | 仅前面条件为假时执行；else-if继续判断。 |
| 14 | 2 / 调用 / 计算 / 写入 | <code>SYSCHECKVAL(open(shmPath, O_RDWR, S_IRUSR &#124; S_IWUSR), "open", *fd);</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 15 | 0 / 调用 / 计算 / 写入 | <code>*ptr = (char*)mmap(NULL, shmSize, PROT_READ&#124;PROT_WRITE, MAP_SHARED, *fd, 0);</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 16 | 0 / 条件判断 | <code>(*ptr == NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 17 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 18 | 2 / 调用 / 计算 / 写入 | <code>WARN("Could not map %s\n", shmPath);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 19 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 20 | 0 / 调用 / 计算 / 写入 | <code>close(*fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 21 | 0 / 调用 / 计算 / 写入 | <code>*fd = -1;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 22 | 0 / 条件判断 | <code>(create)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 23 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 24 | 2 / 调用 / 计算 / 写入 | <code>memset(*ptr, 0, shmSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 25 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (create) {
    if (shmPath[0] == '\0') {
      sprintf(shmPath, "/dev/shm/nccl-XXXXXX");
      *fd = mkstemp(shmPath);
    } else {
      SYSCHECKVAL(open(shmPath, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR), "open", *fd);
    }
    if (ftruncate(*fd, shmSize) != 0) {
      WARN("Error: failed to extend %s to %d bytes", shmPath, shmSize);
      return ncclSystemError;
    }
  } else {
    SYSCHECKVAL(open(shmPath, O_RDWR, S_IRUSR | S_IWUSR), "open", *fd);
  }
  *ptr = (char*)mmap(NULL, shmSize, PROT_READ|PROT_WRITE, MAP_SHARED, *fd, 0);
  if (*ptr == NULL) {
    WARN("Could not map %s\n", shmPath);
    return ncclSystemError;
  }
  close(*fd);
  *fd = -1;
  if (create) memset(*ptr, 0, shmSize);
  return ncclSuccess;
}
```

</details>

<a id="f0968"></a>

## F0968：ncclShmOpen

用途：在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 准备局部状态 | <code>int fd = -1;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 2 | 0 / 准备局部状态 | <code>void* ptr = MAP_FAILED;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 3 | 0 / 准备局部状态 | <code>ncclResult_t res = ncclSuccess;</code> | 声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。 |
| 4 | 0 / 调用 / 计算 / 写入 | <code>NCCLCHECKGOTO(ncclShmSetup(shmPath, shmSize, &fd, &ptr, create), res, sysError);</code> | 失败时保存错误并跳到清理标签，成功才继续；创建/打开共享文件、设定尺寸并 mmap，创建者清零后关闭不再需要的文件描述符。 |
| 5 | 0 / 条件判断 | <code>(devShmPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 6 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 7 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaHostRegister(ptr, shmSize, cudaHostRegisterMapped), res, cudaError);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 8 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECKGOTO(cudaHostGetDevicePointer(devShmPtr, ptr, 0), res, cudaError);</code> | 执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。 |
| 9 | 0 / 调用 / 计算 / 写入 | <code>*shmPtr = ptr;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 10 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 11 | 0 / 跳转标签 | <code>sysError</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 12 | 1 / 调用 / 计算 / 写入 | <code>WARN("Error while %s shared memory segment %s (size %d)", create ? "creating" : "attaching to", shmPath, shmSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 13 | 0 / 跳转标签 | <code>cudaError</code> | 来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。 |
| 14 | 1 / 条件判断 | <code>(fd != -1)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 15 | 2 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 16 | 3 / 调用 / 计算 / 写入 | <code>close(fd);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 17 | 0 / 条件判断 | <code>(create)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 18 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 19 | 2 / 调用 / 计算 / 写入 | <code>shm_unlink(shmPath);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 20 | 0 / 条件判断 | <code>(ptr != MAP_FAILED)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 21 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 22 | 2 / 调用 / 计算 / 写入 | <code>munmap(ptr, shmSize);</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 23 | 0 / 调用 / 计算 / 写入 | <code>*shmPtr = NULL;</code> | 按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。 |
| 24 | 0 / 函数返回 | <code>return res;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  int fd = -1;
  void* ptr = MAP_FAILED;
  ncclResult_t res = ncclSuccess;

  NCCLCHECKGOTO(ncclShmSetup(shmPath, shmSize, &fd, &ptr, create), res, sysError);
  if (devShmPtr) {
    CUDACHECKGOTO(cudaHostRegister(ptr, shmSize, cudaHostRegisterMapped), res, cudaError);
    CUDACHECKGOTO(cudaHostGetDevicePointer(devShmPtr, ptr, 0), res, cudaError);
  }

  *shmPtr = ptr;
  return ncclSuccess;
sysError:
  WARN("Error while %s shared memory segment %s (size %d)", create ? "creating" : "attaching to", shmPath, shmSize);
cudaError:
  if (fd != -1) close(fd);
  if (create) shm_unlink(shmPath);
  if (ptr != MAP_FAILED) munmap(ptr, shmSize);
  *shmPtr = NULL;
  return res;
}
```

</details>

<a id="f0969"></a>

## F0969：ncclShmUnlink

用途：删除共享内存文件名，已打开的映射仍由各进程按生命周期单独解除。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(shmPath != NULL)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>SYSCHECK(unlink(shmPath), "unlink");</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (shmPath != NULL) SYSCHECK(unlink(shmPath), "unlink");
  return ncclSuccess;
}
```

</details>

<a id="f0970"></a>

## F0970：ncclShmClose

用途：先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。

| 步骤 | 层级/种类 | 代码与条件 | 内部作用 |
| --- | --- | --- | --- |
| 1 | 0 / 条件判断 | <code>(devShmPtr)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 2 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 3 | 2 / 调用 / 计算 / 写入 | <code>CUDACHECK(cudaHostUnregister(shmPtr));</code> | 执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。 |
| 4 | 0 / 条件判断 | <code>(munmap(shmPtr, shmSize) != 0)</code> | 为真只进入真分支；为假才考虑else，之后在分支出口继续。 |
| 5 | 1 / 真分支 | <code></code> | 仅条件为真执行以下子项。 |
| 6 | 2 / 调用 / 计算 / 写入 | <code>WARN("munmap of shared memory failed");</code> | 调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。 |
| 7 | 2 / 函数返回 | <code>return ncclSystemError;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |
| 8 | 0 / 函数返回 | <code>return ncclSuccess;</code> | 立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。 |

<details>
<summary>完整函数体（保持原逻辑，未做宏展开）</summary>

```cpp
{
  if (devShmPtr) CUDACHECK(cudaHostUnregister(shmPtr));
  if (munmap(shmPtr, shmSize) != 0) {
    WARN("munmap of shared memory failed");
    return ncclSystemError;
  }
  return ncclSuccess;
}
```

</details>

