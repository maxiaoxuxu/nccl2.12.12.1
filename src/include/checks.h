/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_CHECKS_H_
/* 中文逐项-常量 X0141：NCCL_CHECKS_H_
 * 编译期定义：NCCL_CHECKS_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_CHECKS_H_

#include "debug.h"

/* 中文精读：阅读错误处理宏时要展开控制流
 * CUDACHECK/NCCLCHECK 常在失败时立即 return；带 GOTO 的版本把错误保存在 res 并跳转清理标签。
 * 因此同一函数里的 cleanup/end/cudaError 并非普通顺序执行，每条资源分配路径要配套分析。
 * IGNORE 宏用于释放阶段尽力清理并继续，THREAD 宏用于把线程失败传回调用者。
 * 错误处理宏本身不会替应用重试整个 collective；系统调用包装的临时 EINTR/EAGAIN 重试另有规则。
 */
// Check CUDA calls
/* 中文逐项-宏 M0055：CUDACHECK
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 CUDACHECK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define CUDACHECK(cmd) do {                                 \
    cudaError_t err = cmd;                                  \
    if( err != cudaSuccess ) {                              \
        WARN("Cuda failure '%s'", cudaGetErrorString(err)); \
        return ncclUnhandledCudaError;                      \
    }                                                       \
} while(false)

/* 中文逐项-宏 M0056：CUDACHECKGOTO
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 CUDACHECKGOTO；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define CUDACHECKGOTO(cmd, res, label) do {                 \
    cudaError_t err = cmd;                                  \
    if( err != cudaSuccess ) {                              \
        WARN("Cuda failure '%s'", cudaGetErrorString(err)); \
        res = ncclUnhandledCudaError;                       \
        goto label;                                         \
    }                                                       \
} while(false)

// Report failure but clear error and continue
/* 中文逐项-宏 M0057：CUDACHECKIGNORE
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 CUDACHECKIGNORE；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define CUDACHECKIGNORE(cmd) do {  \
    cudaError_t err = cmd;         \
    if( err != cudaSuccess ) {     \
        INFO(NCCL_ALL,"%s:%d Cuda failure '%s'", __FILE__, __LINE__, cudaGetErrorString(err)); \
        (void) cudaGetLastError(); \
    }                              \
} while(false)

#include <errno.h>
// Check system calls
/* 中文逐项-宏 M0058：SYSCHECK
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 SYSCHECK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define SYSCHECK(call, name) do { \
  int retval; \
  SYSCHECKVAL(call, name, retval); \
} while (false)

/* 中文逐项-宏 M0059：SYSCHECKVAL
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 SYSCHECKVAL；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define SYSCHECKVAL(call, name, retval) do { \
  SYSCHECKSYNC(call, name, retval); \
  if (retval == -1) { \
    WARN("Call to " name " failed : %s", strerror(errno)); \
    return ncclSystemError; \
  } \
} while (false)

/* 中文逐项-宏 M0060：SYSCHECKSYNC
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 SYSCHECKSYNC；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define SYSCHECKSYNC(call, name, retval) do { \
  retval = call; \
  if (retval == -1 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) { \
    INFO(NCCL_ALL,"Call to " name " returned %s, retrying", strerror(errno)); \
  } else { \
    break; \
  } \
} while(true)

/* 中文逐项-常量 X0142：res
 * 编译期定义：res = = ncclSystemError; \ INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res); \ goto label; \ } \ } while
 * (0);。
 * 调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 */
#define SYSCHECKGOTO(statement, res, label) do { \
  if ((statement) == -1) {    \
    /* Print the back trace*/ \
    res = ncclSystemError;    \
    INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res);    \
    goto label; \
  } \
} while (0);

/* 中文逐项-宏 M0061：NEQCHECK
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 NEQCHECK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NEQCHECK(statement, value) do {   \
  if ((statement) != value) {             \
    /* Print the back trace*/             \
    INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, ncclSystemError);    \
    return ncclSystemError;     \
  }                             \
} while (0);

/* 中文逐项-常量 X0143：res
 * 编译期定义：res = = ncclSystemError; \ INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res); \ goto label; \ } \ } while
 * (0);。
 * 调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 */
#define NEQCHECKGOTO(statement, value, res, label) do { \
  if ((statement) != value) { \
    /* Print the back trace*/ \
    res = ncclSystemError;    \
    INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res);    \
    goto label; \
  } \
} while (0);

/* 中文逐项-宏 M0062：EQCHECK
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 EQCHECK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define EQCHECK(statement, value) do {    \
  if ((statement) == value) {             \
    /* Print the back trace*/             \
    INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, ncclSystemError);    \
    return ncclSystemError;     \
  }                             \
} while (0);

/* 中文逐项-常量 X0144：res
 * 编译期定义：res = = ncclSystemError; \ INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res); \ goto label; \ } \ } while
 * (0);。
 * 调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 */
#define EQCHECKGOTO(statement, value, res, label) do { \
  if ((statement) == value) { \
    /* Print the back trace*/ \
    res = ncclSystemError;    \
    INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res);    \
    goto label; \
  } \
} while (0);

// Propagate errors up
/* 中文逐项-常量 X0145：if
 * 编译期定义：if = (ncclDebugNoWarn == 0) INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res); \ return res; \ } \ }
 * while (0);。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCLCHECK(call) do { \
  ncclResult_t res = call; \
  if (res != ncclSuccess) { \
    /* Print the back trace*/ \
    if (ncclDebugNoWarn == 0) INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res);    \
    return res; \
  } \
} while (0);

/* 中文逐项-常量 X0146：if
 * 编译期定义：if = (ncclDebugNoWarn == 0) INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res); \ goto label; \ } \ }
 * while (0);。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCLCHECKGOTO(call, res, label) do { \
  res = call; \
  if (res != ncclSuccess) { \
    /* Print the back trace*/ \
    if (ncclDebugNoWarn == 0) INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res);    \
    goto label; \
  } \
} while (0);

/* 中文逐项-宏 M0063：NCCLWAIT
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCLWAIT；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCLWAIT(call, cond, abortFlagPtr) do {         \
  volatile uint32_t* tmpAbortFlag = (abortFlagPtr);     \
  ncclResult_t res = call;                \
  if (res != ncclSuccess) {               \
    if (ncclDebugNoWarn == 0) INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res);    \
    return ncclInternalError;             \
  }                                       \
  if (tmpAbortFlag) NEQCHECK(*tmpAbortFlag, 0); \
} while (!(cond));

/* 中文逐项-宏 M0064：NCCLWAITGOTO
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCLWAITGOTO；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCLWAITGOTO(call, cond, abortFlagPtr, res, label) do { \
  volatile uint32_t* tmpAbortFlag = (abortFlagPtr);             \
  res = call;                             \
  if (res != ncclSuccess) {               \
    if (ncclDebugNoWarn == 0) INFO(NCCL_ALL,"%s:%d -> %d", __FILE__, __LINE__, res);    \
    goto label;                           \
  }                                       \
  if (tmpAbortFlag) NEQCHECKGOTO(*tmpAbortFlag, 0, res, label); \
} while (!(cond));

/* 中文逐项-宏 M0065：NCCLCHECKTHREAD
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 NCCLCHECKTHREAD；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NCCLCHECKTHREAD(a) do { \
  if ((args->ret = (a)) != ncclSuccess) { \
    INFO(NCCL_INIT,"%s:%d -> %d [Async thread]", __FILE__, __LINE__, args->ret); \
    return args; \
  } \
} while(0)

/* 中文逐项-宏 M0066：CUDACHECKTHREAD
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 CUDACHECKTHREAD；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define CUDACHECKTHREAD(a) do { \
  if ((a) != cudaSuccess) { \
    INFO(NCCL_INIT,"%s:%d -> %d [Async thread]", __FILE__, __LINE__, args->ret); \
    args->ret = ncclUnhandledCudaError; \
    return args; \
  } \
} while(0)

#endif
