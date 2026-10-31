/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "shm.h"
#include "checks.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Change functions behavior to match other SYS functions
/* 中文内部逻辑 F0965：shm_allocate
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int err = posix_fallocate(fd, 0, shmSize);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 条件判断：(err)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：errno = err;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     5. 函数返回：return -1;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__shmutils_cc.md#f0965
 */
/* 中文逐项-函数 F0965：shm_allocate
 * 用途与位置：包装 posix_fallocate，把其错误码返回约定转换成普通系统调用 -1/errno 风格。
 * 参数 fd（int fd）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 参数 shmSize（const int shmSize）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：posix_fallocate。
 * 需要对照的边界/推进条件：err。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return -1;；return 0;。CHECK类宏还可能提前返回/跳转。
 */
static int shm_allocate(int fd, const int shmSize) {
/* 中文逐项-变量 V3454：err
 * err（int；函数 shm_allocate 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 err = posix_fallocate(fd, 0, shmSize)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int err = posix_fallocate(fd, 0, shmSize);
  if (err) { errno = err; return -1; }
  return 0;
}
/* 中文内部逻辑 F0966：shm_map
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*ptr = mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 函数返回：return (*ptr == MAP_FAILED) ? -1 : 0;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__shmutils_cc.md#f0966
 */
/* 中文逐项-函数 F0966：shm_map
 * 用途与位置：用 MAP_SHARED 映射已有文件页到本进程，失败返回系统调用风格错误。
 * 参数 fd（int fd）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 参数 shmSize（const int shmSize）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 * 参数 ptr（void** ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：mmap。
 * 源码返回表达式：return (*ptr == MAP_FAILED) ? -1 : 0;。CHECK类宏还可能提前返回/跳转。
 */
static int shm_map(int fd, const int shmSize, void** ptr) {
  *ptr = mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  return (*ptr == MAP_FAILED) ? -1 : 0;
}

/* 中文内部逻辑 F0967：ncclShmSetup
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(create)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 条件判断：(shmPath[0] == '\0')；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       4. 真分支：；仅条件为真执行以下子项。
 *         5. 调用 / 计算 / 写入：sprintf(shmPath, "/dev/shm/nccl-XXXXXX");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         6. 调用 / 计算 / 写入：*fd = mkstemp(shmPath);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       7. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         8. 调用 / 计算 / 写入：SYSCHECKVAL(open(shmPath, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR), "open", *fd);；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 *     9. 条件判断：(ftruncate(*fd, shmSize) != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       10. 真分支：；仅条件为真执行以下子项。
 *         11. 调用 / 计算 / 写入：WARN("Error: failed to extend %s to %d bytes", shmPath, shmSize);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *         12. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   13. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     14. 调用 / 计算 / 写入：SYSCHECKVAL(open(shmPath, O_RDWR, S_IRUSR | S_IWUSR), "open", *fd);；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 15. 调用 / 计算 / 写入：*ptr = (char*)mmap(NULL, shmSize, PROT_READ|PROT_WRITE, MAP_SHARED, *fd, 0);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 16. 条件判断：(*ptr == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   17. 真分支：；仅条件为真执行以下子项。
 *     18. 调用 / 计算 / 写入：WARN("Could not map %s\n", shmPath);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     19. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 20. 调用 / 计算 / 写入：close(*fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 21. 调用 / 计算 / 写入：*fd = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 22. 条件判断：(create)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   23. 真分支：；仅条件为真执行以下子项。
 *     24. 调用 / 计算 / 写入：memset(*ptr, 0, shmSize);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 25. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__shmutils_cc.md#f0967
 */
/* 中文逐项-函数 F0967：ncclShmSetup
 * 用途与位置：创建/打开共享文件、设定尺寸并 mmap，创建者清零后关闭不再需要的文件描述符。
 * 参数 shmPath（char* shmPath）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 参数 shmSize（const int shmSize）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 * 参数 fd（int* fd）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 参数 ptr（void** ptr）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 参数 create（int create）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/misc/shmutils.cc::ncclShmOpen。
 * 本体直接出现的调用/包装：SYSCHECKVAL；close；ftruncate；memset；mkstemp；mmap；open；sprintf。
 * 需要对照的边界/推进条件：create；shmPath[0] == '\0'；ftruncate(*fd, shmSize) != 0；*ptr == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclShmSetup(char* shmPath, const int shmSize, int* fd, void** ptr, int create) {
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

/* 中文精读：同一共享内存在 CPU/GPU 上的地址视图
 * 先创建/打开共享文件并 mmap 到本进程，必要时 cudaHostRegisterMapped 注册为 GPU 可访问。
 * 再通过 cudaHostGetDevicePointer 获取设备侧地址，分别返回 shmPtr 与 devShmPtr。
 * 跨进程映射同一文件时，CPU 虚拟地址可不同；传递的是共享对象身份，不是假设地址相同。
 * Close 先取消 CUDA 注册再 munmap；Unlink 删除名字，已存在的映射生命周期另行管理。
 */
/* 中文内部逻辑 F0968：ncclShmOpen
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int fd = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 准备局部状态：void* ptr = MAP_FAILED;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 准备局部状态：ncclResult_t res = ncclSuccess;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 4. 调用 / 计算 / 写入：NCCLCHECKGOTO(ncclShmSetup(shmPath, shmSize, &fd, &ptr, create), res, sysError);；失败时保存错误并跳到清理标签，成功才继续；创建/打开共享文件、设定尺寸并 mmap，创建者清零后关闭不再需要的文件描述符。
 * 5. 条件判断：(devShmPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   6. 真分支：；仅条件为真执行以下子项。
 *     7. 调用 / 计算 / 写入：CUDACHECKGOTO(cudaHostRegister(ptr, shmSize, cudaHostRegisterMapped), res, cudaError);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 *     8. 调用 / 计算 / 写入：CUDACHECKGOTO(cudaHostGetDevicePointer(devShmPtr, ptr, 0), res, cudaError);；执行被包装调用；失败时保存错误并跳到指定清理标签，成功才继续下一项。
 * 9. 调用 / 计算 / 写入：*shmPtr = ptr;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 11. 跳转标签：sysError；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   12. 调用 / 计算 / 写入：WARN("Error while %s shared memory segment %s (size %d)", create ? "creating" : "attaching to", shmPath, shmSize);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 13. 跳转标签：cudaError；来自goto或正常贯穿的路径可在此会合，标签本身不执行额外计算。
 *   14. 条件判断：(fd != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     15. 真分支：；仅条件为真执行以下子项。
 *       16. 调用 / 计算 / 写入：close(fd);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 17. 条件判断：(create)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   18. 真分支：；仅条件为真执行以下子项。
 *     19. 调用 / 计算 / 写入：shm_unlink(shmPath);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 20. 条件判断：(ptr != MAP_FAILED)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   21. 真分支：；仅条件为真执行以下子项。
 *     22. 调用 / 计算 / 写入：munmap(ptr, shmSize);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 23. 调用 / 计算 / 写入：*shmPtr = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 24. 函数返回：return res;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__shmutils_cc.md#f0968
 */
/* 中文逐项-函数 F0968：ncclShmOpen
 * 用途与位置：在共享 mmap 基础上可选 cudaHostRegisterMapped 并取得设备侧地址，建立 CPU/GPU 双视图。
 * 参数 shmPath（char* shmPath）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 参数 shmSize（const int shmSize）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 * 参数 shmPtr（void** shmPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 devShmPtr（void** devShmPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 create（int create）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyConnect；src/proxy.cc::proxyProgressInit；src/transport/net.cc::netMapShm；src/transport/net.cc::netCreateShm；src/transport/shm.cc::shmSendSetup；src/transport/shm.cc::shmRecvSetup；src/transport/shm.cc::shmSendConnect；其余见调用清单。
 * 本体直接出现的调用/包装：CUDACHECKGOTO；NCCLCHECKGOTO；close；cudaHostGetDevicePointer；cudaHostRegister；munmap；ncclShmSetup；shm_unlink。
 * 需要对照的边界/推进条件：devShmPtr；fd != -1；create；ptr != MAP_FAILED。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return res;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclShmOpen(char* shmPath, const int shmSize, void** shmPtr, void** devShmPtr, int create) {
/* 中文逐项-变量 V3455：fd
 * fd（int；函数 ncclShmOpen 内）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * 初值/来源 fd = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int fd = -1;
/* 中文逐项-变量 V3456：ptr
 * ptr（void；函数 ncclShmOpen 内）：当前数据/元数据对象地址；请结合类型与赋值区分CPU、设备或输出指针。
 * 初值/来源 ptr = MAP_FAILED；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  void* ptr = MAP_FAILED;
/* 中文逐项-变量 V3457：res
 * res（ncclResult_t；函数 ncclShmOpen 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 res = ncclSuccess；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
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

/* 中文内部逻辑 F0969：ncclShmUnlink
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(shmPath != NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：SYSCHECK(unlink(shmPath), "unlink");；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__shmutils_cc.md#f0969
 */
/* 中文逐项-函数 F0969：ncclShmUnlink
 * 用途与位置：删除共享内存文件名，已打开的映射仍由各进程按生命周期单独解除。
 * 参数 shmPath（const char* shmPath）：共享文件短名或完整路径，跨进程传递共享对象身份，而非假定裸虚拟地址一致。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::ncclProxyShmUnlink；src/transport/net.cc::netMapShm；src/transport/shm.cc::shmSendConnect；src/transport/shm.cc::shmRecvConnect。
 * 本体直接出现的调用/包装：SYSCHECK；unlink。
 * 需要对照的边界/推进条件：shmPath != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclShmUnlink(const char* shmPath) {
  if (shmPath != NULL) SYSCHECK(unlink(shmPath), "unlink");
  return ncclSuccess;
}

/* 中文内部逻辑 F0970：ncclShmClose
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(devShmPtr)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：CUDACHECK(cudaHostUnregister(shmPtr));；执行被包装调用并检查返回；失败可能提前return，不能把后续项当成无条件执行。
 * 4. 条件判断：(munmap(shmPtr, shmSize) != 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   5. 真分支：；仅条件为真执行以下子项。
 *     6. 调用 / 计算 / 写入：WARN("munmap of shared memory failed");；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     7. 函数返回：return ncclSystemError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__shmutils_cc.md#f0970
 */
/* 中文逐项-函数 F0970：ncclShmClose
 * 用途与位置：先撤销可选 CUDA 主机内存注册，再 munmap 当前进程映射。
 * 参数 shmPtr（void* shmPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 devShmPtr（void* devShmPtr）：对应数据或控制字段的缓存地址，先确认主机/设备地址空间，再看后续读写位置。
 * 参数 shmSize（const int shmSize）：本地/对端共享映射字节容量，解除映射时须使用对应大小。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/proxy.cc::proxyOpsFree；src/proxy.cc::ncclProxyDestroy；src/transport/net.cc::sendFree；src/transport/net.cc::sendProxyFree；src/transport/shm.cc::shmSendFree；src/transport/shm.cc::shmRecvFree。
 * 本体直接出现的调用/包装：CUDACHECK；cudaHostUnregister；munmap。
 * 需要对照的边界/推进条件：devShmPtr；munmap(shmPtr, shmSize) != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclShmClose(void* shmPtr, void* devShmPtr, const int shmSize) {
  if (devShmPtr) CUDACHECK(cudaHostUnregister(shmPtr));
  if (munmap(shmPtr, shmSize) != 0) {
    WARN("munmap of shared memory failed");
    return ncclSystemError;
  }
  return ncclSuccess;
}
