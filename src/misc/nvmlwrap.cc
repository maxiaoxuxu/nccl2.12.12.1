/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "nvmlwrap.h"
#include "checks.h"
#include "debug.h"

#include <initializer_list>
#include <memory>
#include <mutex>

/* 中文逐项-变量 V3397：ncclNvmlDeviceCount
 * ncclNvmlDeviceCount（int；文件/静态存储作用域）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 * 初值/来源 ncclNvmlDeviceCount = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
int ncclNvmlDeviceCount = 0;
/* 中文逐项-变量 V3398：ncclNvmlDevices
 * ncclNvmlDevices（ncclNvmlDeviceInfo；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：res1 = pfn_nvmlDeviceGetHandleByIndex(a, &ncclNvmlDevices[a].handle);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
ncclNvmlDeviceInfo ncclNvmlDevices[ncclNvmlMaxDevices];
/* 中文逐项-变量 V3399：ncclNvmlDevicePairs
 * ncclNvmlDevicePairs（ncclNvmlDevicePairInfo；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：res1 = pfn_nvmlDeviceGetP2PStatus(da, db, NVML_P2P_CAPS_INDEX_READ,
 * &ncclNvmlDevicePairs[a][b].p2pStatusRead);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
ncclNvmlDevicePairInfo ncclNvmlDevicePairs[ncclNvmlMaxDevices][ncclNvmlMaxDevices];

#if NCCL_NVML_DIRECT
/* 中文逐项-宏 M0143：NCCL_NVML_FN
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCL_NVML_FN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
  #define NCCL_NVML_FN(name, rettype, arglist) constexpr rettype(*pfn_##name)arglist = name;
#else
  #include <dlfcn.h>
/* 中文逐项-宏 M0144：NCCL_NVML_FN
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 NCCL_NVML_FN；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
  #define NCCL_NVML_FN(name, rettype, arglist) rettype(*pfn_##name)arglist = nullptr;
#endif

namespace {
  NCCL_NVML_FN(nvmlInit, nvmlReturn_t, ())
  NCCL_NVML_FN(nvmlInit_v2, nvmlReturn_t, ())
  NCCL_NVML_FN(nvmlShutdown, nvmlReturn_t, ())
  NCCL_NVML_FN(nvmlDeviceGetCount, nvmlReturn_t, (unsigned int*))
  NCCL_NVML_FN(nvmlDeviceGetCount_v2, nvmlReturn_t, (unsigned int*))
  NCCL_NVML_FN(nvmlDeviceGetHandleByPciBusId, nvmlReturn_t, (const char* pciBusId, nvmlDevice_t* device))
  NCCL_NVML_FN(nvmlDeviceGetHandleByIndex, nvmlReturn_t, (unsigned int index, nvmlDevice_t *device))
  NCCL_NVML_FN(nvmlDeviceGetIndex, nvmlReturn_t, (nvmlDevice_t device, unsigned* index))
  NCCL_NVML_FN(nvmlErrorString, char const*, (nvmlReturn_t r))
  NCCL_NVML_FN(nvmlDeviceGetNvLinkState, nvmlReturn_t, (nvmlDevice_t device, unsigned int link, nvmlEnableState_t *isActive))
  NCCL_NVML_FN(nvmlDeviceGetNvLinkRemotePciInfo, nvmlReturn_t, (nvmlDevice_t device, unsigned int link, nvmlPciInfo_t *pci))
  NCCL_NVML_FN(nvmlDeviceGetNvLinkCapability, nvmlReturn_t, (nvmlDevice_t device, unsigned int link, nvmlNvLinkCapability_t capability, unsigned int *capResult))
  NCCL_NVML_FN(nvmlDeviceGetCudaComputeCapability, nvmlReturn_t, (nvmlDevice_t device, int* major, int* minor))
  NCCL_NVML_FN(nvmlDeviceGetP2PStatus, nvmlReturn_t, (nvmlDevice_t device1, nvmlDevice_t device2, nvmlGpuP2PCapsIndex_t p2pIndex, nvmlGpuP2PStatus_t* p2pStatus))

  std::mutex lock; // NVML has had some thread safety bugs
  bool initialized = false;
  thread_local bool threadInitialized = false;
  ncclResult_t initResult;
}

/* 中文精读：NVML 在 NCCL 中负责发现硬件
 * 动态装载 NVML 并查找所需符号，读取 GPU、NVLink、PCI 地址及 P2P 能力等拓扑信息。
 * NVML 的设备编号与 CUDA_VISIBLE_DEVICES 过滤后的 CUDA ordinal 不保证相等，通常按 PCI busId 关联。
 * 它帮助图搜索了解硬件，不负责每个 chunk 的搬运，也不是 CUDA kernel 发射接口。
 * 初始化使用锁/缓存，避免同进程多个 communicator 重复无序初始化管理库。
 */
ncclResult_t ncclNvmlEnsureInitialized() {
  // Optimization to avoid repeatedly grabbing the lock when we only want to
  // read from the global tables.
  if (threadInitialized) return initResult;
  threadInitialized = true;

  std::lock_guard<std::mutex> locked(lock);

  if (initialized) return initResult;
  initialized = true;

  #if !NCCL_NVML_DIRECT
  if (pfn_nvmlInit == nullptr) {
/* 中文逐项-变量 V3400：libhandle
 * libhandle（void；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 libhandle = dlopen("libnvidia-ml.so.1", RTLD_NOW)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    void *libhandle = dlopen("libnvidia-ml.so.1", RTLD_NOW);
    if (libhandle == nullptr) {
      WARN("Failed to open libnvidia-ml.so.1");
      initResult = ncclSystemError;
      return initResult;
    }

/* 中文逐项-类型 T0392：Symbol
 * 类型职责：NVML动态符号名与目标函数指针槽位的对应表项，初始化时逐项解析。
 * 声明内可见成员（含嵌套成员）：ppfn、name。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct Symbol { /* 中文逐项-变量 V3401：ppfn
 * ppfn（void；成员 Symbol）：动态符号目标函数指针槽的地址，初始化解析成功后写入可调用入口。
 */void **ppfn; /* 中文逐项-变量 V3402：name
 * name（char；成员 Symbol）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */char const *name; };
/* 中文逐项-变量 V3403：symbols
 * symbols（std::initializer_list<Symbol>；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 symbols = { {(void**)&pfn_nvmlInit, "nvmlInit"}, {(void**)&pfn_nvmlInit_v2, "nvmlInit_v2"},
 * {(void**)&pfn_nvmlShutdown, "nvmlShutdown"}, {(void**)&pfn_nvmlDeviceGetCount, "nvmlDeviceGetCount"},
 * {(void**)&pfn_nvmlDeviceGetCount_v2, "nvmlDevi …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
    std::initializer_list<Symbol> symbols = {
      {(void**)&pfn_nvmlInit, "nvmlInit"},
      {(void**)&pfn_nvmlInit_v2, "nvmlInit_v2"},
      {(void**)&pfn_nvmlShutdown, "nvmlShutdown"},
      {(void**)&pfn_nvmlDeviceGetCount, "nvmlDeviceGetCount"},
      {(void**)&pfn_nvmlDeviceGetCount_v2, "nvmlDeviceGetCount_v2"},
      {(void**)&pfn_nvmlDeviceGetHandleByPciBusId, "nvmlDeviceGetHandleByPciBusId"},
      {(void**)&pfn_nvmlDeviceGetHandleByIndex, "nvmlDeviceGetHandleByIndex"},
      {(void**)&pfn_nvmlDeviceGetIndex, "nvmlDeviceGetIndex"},
      {(void**)&pfn_nvmlErrorString, "nvmlErrorString"},
      {(void**)&pfn_nvmlDeviceGetNvLinkState, "nvmlDeviceGetNvLinkState"},
      {(void**)&pfn_nvmlDeviceGetNvLinkRemotePciInfo, "nvmlDeviceGetNvLinkRemotePciInfo"},
      {(void**)&pfn_nvmlDeviceGetNvLinkCapability, "nvmlDeviceGetNvLinkCapability"},
      {(void**)&pfn_nvmlDeviceGetCudaComputeCapability, "nvmlDeviceGetCudaComputeCapability"},
      {(void**)&pfn_nvmlDeviceGetP2PStatus, "nvmlDeviceGetP2PStatus"}
    };
    for(Symbol sym: symbols) {
      *sym.ppfn = dlsym(libhandle, sym.name);
    }
  }
  #endif

  #if NCCL_NVML_DIRECT
/* 中文逐项-变量 V3404：have_v2
 * have_v2（bool；文件/静态存储作用域）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 have_v2 = true；布尔初态为true，后续条件决定是否切换。
 */
    bool have_v2 = true;
  #else
/* 中文逐项-变量 V3405：have_v2
 * have_v2（bool；文件/静态存储作用域）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 have_v2 = pfn_nvmlInit_v2 != nullptr；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
    bool have_v2 = pfn_nvmlInit_v2 != nullptr; // if this compare is done in the NCCL_NVML_DIRECT=1 case then GCC warns about it never being null
  #endif
/* 中文逐项-变量 V3406：res1
 * res1（nvmlReturn_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 res1 = (have_v2 ? pfn_nvmlInit_v2 : pfn_nvmlInit)()；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  nvmlReturn_t res1 = (have_v2 ? pfn_nvmlInit_v2 : pfn_nvmlInit)();
  if (res1 != NVML_SUCCESS) {
    WARN("nvmlInit%s() failed: %s", have_v2 ? "_v2" : "", pfn_nvmlErrorString(res1));
    initResult = ncclSystemError;
    return initResult;
  }

/* 中文逐项-变量 V3407：ndev
 * ndev（unsigned int；文件/静态存储作用域）：NVML管理接口枚举的GPU设备总数，供设备句柄和PCI标识缓存分配使用。
 * 此处未显式给初值；后续首个文本使用点：res1 = (have_v2 ? pfn_nvmlDeviceGetCount_v2 :
 * pfn_nvmlDeviceGetCount)(&ndev);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  unsigned int ndev;
  res1 = (have_v2 ? pfn_nvmlDeviceGetCount_v2 : pfn_nvmlDeviceGetCount)(&ndev);
  if (res1 != NVML_SUCCESS) {
    WARN("nvmlDeviceGetCount%s() failed: %s", have_v2 ? "_v2" :"", pfn_nvmlErrorString(res1));
    initResult = ncclSystemError;
    return initResult;
  }

  ncclNvmlDeviceCount = int(ndev);
  if (ncclNvmlMaxDevices < ncclNvmlDeviceCount) {
    WARN("nvmlDeviceGetCount() reported more devices (%d) than the internal maximum (ncclNvmlMaxDevices=%d)", ncclNvmlDeviceCount, ncclNvmlMaxDevices);
    initResult = ncclInternalError;
    return initResult;
  }

  for(/* 中文逐项-变量 V3408：a
 * a（int；文件/静态存储作用域）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a < ncclNvmlDeviceCount；循环体与增量表达式决定每轮如何推进。
 */int a=0; a < ncclNvmlDeviceCount; a++) {
    res1 = pfn_nvmlDeviceGetHandleByIndex(a, &ncclNvmlDevices[a].handle);
    if (res1 != NVML_SUCCESS) {
      WARN("nvmlDeviceGetHandleByIndex(%d) failed: %s", int(a), pfn_nvmlErrorString(res1));
      initResult = ncclSystemError;
      return initResult;
    }

    res1 = pfn_nvmlDeviceGetCudaComputeCapability(ncclNvmlDevices[a].handle, &ncclNvmlDevices[a].computeCapabilityMajor, &ncclNvmlDevices[a].computeCapabilityMinor);
    if (res1 != NVML_SUCCESS) {
      WARN("nvmlDeviceGetCudaComputeCapability(%d) failed: %s", int(a), pfn_nvmlErrorString(res1));
      initResult = ncclSystemError;
      return initResult;
    }
  }

  for(/* 中文逐项-变量 V3409：a
 * a（int；文件/静态存储作用域）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a < ncclNvmlDeviceCount；循环体与增量表达式决定每轮如何推进。
 */int a=0; a < ncclNvmlDeviceCount; a++) {
    for(/* 中文逐项-变量 V3410：b
 * b（int；文件/静态存储作用域）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：b < ncclNvmlDeviceCount；循环体与增量表达式决定每轮如何推进。
 */int b=0; b < ncclNvmlDeviceCount; b++) {
/* 中文逐项-变量 V3411：da
 * da（nvmlDevice_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 da = ncclNvmlDevices[a].handle；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      nvmlDevice_t da = ncclNvmlDevices[a].handle;
/* 中文逐项-变量 V3412：db
 * db（nvmlDevice_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 db = ncclNvmlDevices[b].handle；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      nvmlDevice_t db = ncclNvmlDevices[b].handle;

      res1 = pfn_nvmlDeviceGetP2PStatus(da, db, NVML_P2P_CAPS_INDEX_READ, &ncclNvmlDevicePairs[a][b].p2pStatusRead);
      if (res1 != NVML_SUCCESS) {
        WARN("nvmlDeviceGetP2PStatus(%d,%d,NVML_P2P_CAPS_INDEX_READ) failed: %s", a, b, pfn_nvmlErrorString(res1));
        initResult = ncclSystemError;
        return initResult;
      }

      res1 = pfn_nvmlDeviceGetP2PStatus(da, db, NVML_P2P_CAPS_INDEX_WRITE, &ncclNvmlDevicePairs[a][b].p2pStatusWrite);
      if (res1 != NVML_SUCCESS) {
        WARN("nvmlDeviceGetP2PStatus(%d,%d,NVML_P2P_CAPS_INDEX_READ) failed: %s", a, b, pfn_nvmlErrorString(res1));
        initResult = ncclSystemError;
        return initResult;
      }
    }
  }

  initResult = ncclSuccess;
  return initResult;
}

/* 中文逐项-宏 M0145：NVMLCHECK
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 NVMLCHECK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define NVMLCHECK(name, ...) do { \
  nvmlReturn_t e44241808 = pfn_##name(__VA_ARGS__); \
  if (e44241808 != NVML_SUCCESS) { \
    WARN(#name "() failed: %s", pfn_nvmlErrorString(e44241808)); \
    return ncclSystemError; \
  } \
} while(0)

/* 中文逐项-常量 X1169：nvmlReturn_t
 * 编译期定义：nvmlReturn_t = e44241808 = pfn_##name(__VA_ARGS__); \ if (e44241808 != NVML_SUCCESS) { \ if (e44241808 !=
 * NVML_ERROR_NOT_SUPPORTED) \ INFO(NCCL_INIT, #name "() failed: %s", pfn_nvmlErrorString(e44241808)); \ return
 * ncclSystemError; \ } \ } while(0)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVMLTRY(name, ...) do { \
  if (!NCCL_NVML_DIRECT && pfn_##name == nullptr) \
    return ncclInternalError; /* missing symbol is not a warned error */ \
  nvmlReturn_t e44241808 = pfn_##name(__VA_ARGS__); \
  if (e44241808 != NVML_SUCCESS) { \
    if (e44241808 != NVML_ERROR_NOT_SUPPORTED) \
      INFO(NCCL_INIT, #name "() failed: %s", pfn_nvmlErrorString(e44241808)); \
    return ncclSystemError; \
  } \
} while(0)

/* 中文内部逻辑 F0949：ncclNvmlDeviceGetHandleByPciBusId
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 2. 准备局部状态：std::lock_guard<std::mutex> locked(lock);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NVMLCHECK(nvmlDeviceGetHandleByPciBusId, pciBusId, device);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__nvmlwrap_cc.md#f0949
 */
/* 中文逐项-函数 F0949：ncclNvmlDeviceGetHandleByPciBusId
 * 用途与位置：按 PCI 地址定位 NVML 设备；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 参数 pciBusId（const char* pciBusId）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 device（nvmlDevice_t* device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoFillGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；NVMLCHECK；ncclNvmlEnsureInitialized。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNvmlDeviceGetHandleByPciBusId(const char* pciBusId, nvmlDevice_t* device) {
  NCCLCHECK(ncclNvmlEnsureInitialized());
  std::lock_guard<std::mutex> locked(lock);
  NVMLCHECK(nvmlDeviceGetHandleByPciBusId, pciBusId, device);
  return ncclSuccess;
}

/* 中文内部逻辑 F0950：ncclNvmlDeviceGetHandleByIndex
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 2. 调用 / 计算 / 写入：*device = ncclNvmlDevices[index].handle;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 3. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__nvmlwrap_cc.md#f0950
 */
/* 中文逐项-函数 F0950：ncclNvmlDeviceGetHandleByIndex
 * 用途与位置：按 NVML 序号取得设备句柄；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 参数 index（unsigned int index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 参数 device（nvmlDevice_t *device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNvmlEnsureInitialized。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNvmlDeviceGetHandleByIndex(unsigned int index, nvmlDevice_t *device) {
  NCCLCHECK(ncclNvmlEnsureInitialized());
  *device = ncclNvmlDevices[index].handle;
  return ncclSuccess;
}

/* 中文内部逻辑 F0951：ncclNvmlDeviceGetIndex
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 2. 循环：初始 int d=0;；继续条件 d < ncclNvmlDeviceCount；每轮更新 d++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 条件判断：(ncclNvmlDevices[d].handle == device)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：*index = d;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__nvmlwrap_cc.md#f0951
 */
/* 中文逐项-函数 F0951：ncclNvmlDeviceGetIndex
 * 用途与位置：由句柄查询 NVML 序号；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 参数 device（nvmlDevice_t device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 index（unsigned* index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNvmlEnsureInitialized。
 * 需要对照的边界/推进条件：d < ncclNvmlDeviceCount；ncclNvmlDevices[d].handle == device。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInvalidArgument;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNvmlDeviceGetIndex(nvmlDevice_t device, unsigned* index) {
  NCCLCHECK(ncclNvmlEnsureInitialized());
  for (/* 中文逐项-变量 V3413：d
 * d（int；函数 ncclNvmlDeviceGetIndex 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：d < ncclNvmlDeviceCount；循环体与增量表达式决定每轮如何推进。
 */int d=0; d < ncclNvmlDeviceCount; d++) {
    if (ncclNvmlDevices[d].handle == device) {
      *index = d;
      return ncclSuccess;
    }
  }
  return ncclInvalidArgument;
}

/* 中文内部逻辑 F0952：ncclNvmlDeviceGetNvLinkState
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 2. 准备局部状态：std::lock_guard<std::mutex> locked(lock);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NVMLTRY(nvmlDeviceGetNvLinkState, device, link, isActive);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__nvmlwrap_cc.md#f0952
 */
/* 中文逐项-函数 F0952：ncclNvmlDeviceGetNvLinkState
 * 用途与位置：查询指定 NVLink 是否激活；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 参数 device（nvmlDevice_t device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 link（unsigned int link）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 参数 isActive（nvmlEnableState_t *isActive）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；NVMLTRY；ncclNvmlEnsureInitialized。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNvmlDeviceGetNvLinkState(nvmlDevice_t device, unsigned int link, nvmlEnableState_t *isActive) {
  NCCLCHECK(ncclNvmlEnsureInitialized());
  std::lock_guard<std::mutex> locked(lock);
  NVMLTRY(nvmlDeviceGetNvLinkState, device, link, isActive);
  return ncclSuccess;
}

/* 中文内部逻辑 F0953：ncclNvmlDeviceGetNvLinkRemotePciInfo
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 2. 准备局部状态：std::lock_guard<std::mutex> locked(lock);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NVMLTRY(nvmlDeviceGetNvLinkRemotePciInfo, device, link, pci);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__nvmlwrap_cc.md#f0953
 */
/* 中文逐项-函数 F0953：ncclNvmlDeviceGetNvLinkRemotePciInfo
 * 用途与位置：查询 NVLink 远端 PCI 标识；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 参数 device（nvmlDevice_t device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 link（unsigned int link）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 参数 pci（nvmlPciInfo_t *pci）：硬件节点union中对应GPU、网络、CPU或PCI的专有属性；先看node.type再读取。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；NVMLTRY；ncclNvmlEnsureInitialized。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNvmlDeviceGetNvLinkRemotePciInfo(nvmlDevice_t device, unsigned int link, nvmlPciInfo_t *pci) {
  NCCLCHECK(ncclNvmlEnsureInitialized());
  std::lock_guard<std::mutex> locked(lock);
  NVMLTRY(nvmlDeviceGetNvLinkRemotePciInfo, device, link, pci);
  return ncclSuccess;
}

/* 中文内部逻辑 F0954：ncclNvmlDeviceGetNvLinkCapability
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 2. 准备局部状态：std::lock_guard<std::mutex> locked(lock);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 调用 / 计算 / 写入：NVMLTRY(nvmlDeviceGetNvLinkCapability, device, link, capability, capResult);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__nvmlwrap_cc.md#f0954
 */
/* 中文逐项-函数 F0954：ncclNvmlDeviceGetNvLinkCapability
 * 用途与位置：查询 NVLink 能力；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 参数 device（nvmlDevice_t device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 link（unsigned int link）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 * 参数 capability（nvmlNvLinkCapability_t capability）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 capResult（unsigned int *capResult）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；NVMLTRY；ncclNvmlEnsureInitialized。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNvmlDeviceGetNvLinkCapability(
    nvmlDevice_t device, unsigned int link, nvmlNvLinkCapability_t capability,
    unsigned int *capResult
  ) {
  NCCLCHECK(ncclNvmlEnsureInitialized());
  std::lock_guard<std::mutex> locked(lock);
  NVMLTRY(nvmlDeviceGetNvLinkCapability, device, link, capability, capResult);
  return ncclSuccess;
}

/* 中文内部逻辑 F0955：ncclNvmlDeviceGetCudaComputeCapability
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 2. 循环：初始 int d=0;；继续条件 d < ncclNvmlDeviceCount；每轮更新 d++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 条件判断：(device == ncclNvmlDevices[d].handle)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：*major = ncclNvmlDevices[d].computeCapabilityMajor;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 调用 / 计算 / 写入：*minor = ncclNvmlDevices[d].computeCapabilityMinor;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 9. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__nvmlwrap_cc.md#f0955
 */
/* 中文逐项-函数 F0955：ncclNvmlDeviceGetCudaComputeCapability
 * 用途与位置：查询 GPU 计算能力；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 参数 device（nvmlDevice_t device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 参数 major（int* major）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 minor（int* minor）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclNvmlEnsureInitialized。
 * 需要对照的边界/推进条件：d < ncclNvmlDeviceCount；device == ncclNvmlDevices[d].handle。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInvalidArgument;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNvmlDeviceGetCudaComputeCapability(nvmlDevice_t device, int* major, int* minor) {
  NCCLCHECK(ncclNvmlEnsureInitialized());

  for(/* 中文逐项-变量 V3414：d
 * d（int；函数 ncclNvmlDeviceGetCudaComputeCapability 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：d < ncclNvmlDeviceCount；循环体与增量表达式决定每轮如何推进。
 */int d=0; d < ncclNvmlDeviceCount; d++) {
    if(device == ncclNvmlDevices[d].handle) {
      *major = ncclNvmlDevices[d].computeCapabilityMajor;
      *minor = ncclNvmlDevices[d].computeCapabilityMinor;
      return ncclSuccess;
    }
  }
  return ncclInvalidArgument;
}

/* 中文内部逻辑 F0956：ncclNvmlDeviceGetP2PStatus
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(ncclNvmlEnsureInitialized());；失败时可能提前return，成功才继续；线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 2. 条件判断：(p2pIndex == NVML_P2P_CAPS_INDEX_READ || p2pIndex == NVML_P2P_CAPS_INDEX_WRITE)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 准备局部状态：int a = -1, b = -1;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     5. 循环：初始 int d=0;；继续条件 d < ncclNvmlDeviceCount；每轮更新 d++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *       6. 条件判断：(device1 == ncclNvmlDevices[d].handle)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         7. 真分支：；仅条件为真执行以下子项。
 *           8. 调用 / 计算 / 写入：a = d;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       9. 条件判断：(device2 == ncclNvmlDevices[d].handle)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         10. 真分支：；仅条件为真执行以下子项。
 *           11. 调用 / 计算 / 写入：b = d;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 *     13. 条件判断：(a == -1 || b == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       14. 真分支：；仅条件为真执行以下子项。
 *         15. 函数返回：return ncclInvalidArgument;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *     16. 条件判断：(p2pIndex == NVML_P2P_CAPS_INDEX_READ)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *       17. 真分支：；仅条件为真执行以下子项。
 *         18. 调用 / 计算 / 写入：*p2pStatus = ncclNvmlDevicePairs[a][b].p2pStatusRead;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       19. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *         20. 调用 / 计算 / 写入：*p2pStatus = ncclNvmlDevicePairs[a][b].p2pStatusWrite;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *   21. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *     22. 准备局部状态：std::lock_guard<std::mutex> locked(lock);；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *     23. 调用 / 计算 / 写入：NVMLCHECK(nvmlDeviceGetP2PStatus, device1, device2, p2pIndex, p2pStatus);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 24. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__misc__nvmlwrap_cc.md#f0956
 */
/* 中文逐项-函数 F0956：ncclNvmlDeviceGetP2PStatus
 * 用途与位置：查询 GPU 对之间的特定 P2P 能力状态；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 参数 device1（nvmlDevice_t device1）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 device2（nvmlDevice_t device2）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 p2pIndex（nvmlGpuP2PCapsIndex_t p2pIndex）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 p2pStatus（nvmlGpuP2PStatus_t* p2pStatus）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；NVMLCHECK；ncclNvmlEnsureInitialized。
 * 需要对照的边界/推进条件：p2pIndex == NVML_P2P_CAPS_INDEX_READ || p2pIndex == NVML_P2P_CAPS_INDEX_WRITE；d <
 * ncclNvmlDeviceCount；device1 == ncclNvmlDevices[d].handle；device2 == ncclNvmlDevices[d].handle。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInvalidArgument;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclNvmlDeviceGetP2PStatus(
    nvmlDevice_t device1, nvmlDevice_t device2, nvmlGpuP2PCapsIndex_t p2pIndex,
    nvmlGpuP2PStatus_t* p2pStatus
  ) {
  NCCLCHECK(ncclNvmlEnsureInitialized());

  if (p2pIndex == NVML_P2P_CAPS_INDEX_READ || p2pIndex == NVML_P2P_CAPS_INDEX_WRITE) {
/* 中文逐项-变量 V3415：a、b
 * a（int；函数 ncclNvmlDeviceGetP2PStatus 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 * b（int；函数 ncclNvmlDeviceGetP2PStatus 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 b = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
    int a = -1, b = -1;
    for(/* 中文逐项-变量 V3416：d
 * d（int；函数 ncclNvmlDeviceGetP2PStatus 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：d < ncclNvmlDeviceCount；循环体与增量表达式决定每轮如何推进。
 */int d=0; d < ncclNvmlDeviceCount; d++) {
      if(device1 == ncclNvmlDevices[d].handle) a = d;
      if(device2 == ncclNvmlDevices[d].handle) b = d;
    }
    if (a == -1 || b == -1) return ncclInvalidArgument;
    if (p2pIndex == NVML_P2P_CAPS_INDEX_READ)
      *p2pStatus = ncclNvmlDevicePairs[a][b].p2pStatusRead;
    else
      *p2pStatus = ncclNvmlDevicePairs[a][b].p2pStatusWrite;
  }
  else {
    std::lock_guard<std::mutex> locked(lock);
    NVMLCHECK(nvmlDeviceGetP2PStatus, device1, device2, p2pIndex, p2pStatus);
  }
  return ncclSuccess;
}
