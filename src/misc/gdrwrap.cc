/*************************************************************************
 * Copyright (c) 2020-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "gdrwrap.h"

#ifndef GDR_DIRECT
#include "core.h"

/* 中文逐项-变量 V3346：gdrState
 * gdrState（enum { gdrUninitialized, gdrInitializing, gdrInitialized, gdrError
 * }；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdrState = gdrUninitialized；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
static /* 中文逐项-类型 T0390：<匿名enum>
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum { /* 中文逐项-枚举项 X1159：gdrUninitialized
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */gdrUninitialized, /* 中文逐项-枚举项 X1160：gdrInitializing
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */gdrInitializing, /* 中文逐项-枚举项 X1161：gdrInitialized
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */gdrInitialized, /* 中文逐项-枚举项 X1162：gdrError
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */gdrError } gdrState = gdrUninitialized;

/* Function pointers assigned from dlopen() */
/* 中文逐项-变量 V3347：gdr_internal_open
 * gdr_internal_open
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_open操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_open)(void)。
 * 回调参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * gdr_internal_open（gdr_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_open", gdr_internal_open);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static gdr_t (*gdr_internal_open)(void);
/* 中文逐项-变量 V3348：gdr_internal_close
 * gdr_internal_close
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_close操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_close)(gdr_t
 * g)。
 * 回调参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * gdr_internal_close（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_close", gdr_internal_close);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static int (*gdr_internal_close)(gdr_t g);
/* 中文逐项-变量 V3349：gdr_internal_pin_buffer
 * gdr_internal_pin_buffer
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_pin_buffer操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_pin_buffer)(gdr_t
 * g, unsigned long addr, size_t size, uint64_t p2p_token, uint32_t va_space, gdr_mh_t *handle)。
 * 回调参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 回调参数 addr（unsigned long addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 回调参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 回调参数 p2p_token（uint64_t p2p_token）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 回调参数 va_space（uint32_t va_space）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 回调参数 handle（gdr_mh_t *handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * gdr_internal_pin_buffer（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_pin_buffer", gdr_internal_pin_buffer);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static int (*gdr_internal_pin_buffer)(gdr_t g, unsigned long addr, size_t size, uint64_t p2p_token, uint32_t va_space, gdr_mh_t *handle);
/* 中文逐项-变量 V3350：gdr_internal_unpin_buffer
 * gdr_internal_unpin_buffer
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_unpin_buffer操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_unpin_buffer)(gdr_t
 * g, gdr_mh_t handle)。
 * 回调参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 回调参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * gdr_internal_unpin_buffer（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_unpin_buffer", gdr_internal_unpin_buffer);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static int (*gdr_internal_unpin_buffer)(gdr_t g, gdr_mh_t handle);
/* 中文逐项-变量 V3351：gdr_internal_get_info
 * gdr_internal_get_info
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_get_info操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_get_info)(gdr_t
 * g, gdr_mh_t handle, gdr_info_t *info)。
 * 回调参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 回调参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 info（gdr_info_t *info）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * gdr_internal_get_info（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_get_info", gdr_internal_get_info);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static int (*gdr_internal_get_info)(gdr_t g, gdr_mh_t handle, gdr_info_t *info);
/* 中文逐项-变量 V3352：gdr_internal_map
 * gdr_internal_map
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_map操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_map)(gdr_t
 * g, gdr_mh_t handle, void **va, size_t size)。
 * 回调参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 回调参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 va（void **va）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 * 回调参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * gdr_internal_map（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_map", gdr_internal_map);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static int (*gdr_internal_map)(gdr_t g, gdr_mh_t handle, void **va, size_t size);
/* 中文逐项-变量 V3353：gdr_internal_unmap
 * gdr_internal_unmap
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_unmap操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_unmap)(gdr_t
 * g, gdr_mh_t handle, void *va, size_t size)。
 * 回调参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 回调参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 va（void *va）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 * 回调参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * gdr_internal_unmap（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_unmap", gdr_internal_unmap);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static int (*gdr_internal_unmap)(gdr_t g, gdr_mh_t handle, void *va, size_t size);
/* 中文逐项-变量 V3354：gdr_internal_runtime_get_version
 * gdr_internal_runtime_get_version
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_runtime_get_version操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_runtime_get_version)(int
 * *major, int *minor)。
 * 回调参数 major（int *major）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 minor（int *minor）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * gdr_internal_runtime_get_version（void；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_runtime_get_version",
 * gdr_internal_runtime_get_version);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static void (*gdr_internal_runtime_get_version)(int *major, int *minor);
/* 中文逐项-变量 V3355：gdr_internal_driver_get_version
 * gdr_internal_driver_get_version
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_driver_get_version操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_driver_get_version)(gdr_t
 * g, int *major, int *minor)。
 * 回调参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 回调参数 major（int *major）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 minor（int *minor）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * gdr_internal_driver_get_version（void；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_driver_get_version",
 * gdr_internal_driver_get_version);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static void (*gdr_internal_driver_get_version)(gdr_t g, int *major, int *minor);
/* 中文逐项-变量 V3356：gdr_internal_copy_to_mapping
 * gdr_internal_copy_to_mapping
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_copy_to_mapping操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_copy_to_mapping)(gdr_mh_t
 * handle, void *map_d_ptr, const void *h_ptr, size_t size)。
 * 回调参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 map_d_ptr（void *map_d_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 h_ptr（const void *h_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * gdr_internal_copy_to_mapping（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_copy_to_mapping",
 * gdr_internal_copy_to_mapping);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static int (*gdr_internal_copy_to_mapping)(gdr_mh_t handle, void *map_d_ptr, const void *h_ptr, size_t size);
/* 中文逐项-变量 V3357：gdr_internal_copy_from_mapping
 * gdr_internal_copy_from_mapping
 * 是函数指针字段/变量：由该资源库/provider提供的gdr_internal_copy_from_mapping操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*gdr_internal_copy_from_mapping)(gdr_mh_t
 * handle, void *h_ptr, const void *map_d_ptr, size_t size)。
 * 回调参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 回调参数 h_ptr（void *h_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 map_d_ptr（const void *map_d_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * gdr_internal_copy_from_mapping（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(gdrhandle, "gdr_copy_from_mapping",
 * gdr_internal_copy_from_mapping);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static int (*gdr_internal_copy_from_mapping)(gdr_mh_t handle, void *h_ptr, const void *map_d_ptr, size_t size);


// Used to make the GDR library calls thread safe
/* 中文逐项-变量 V3358：gdrLock
 * gdrLock（pthread_mutex_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdrLock = PTHREAD_MUTEX_INITIALIZER；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
pthread_mutex_t gdrLock = PTHREAD_MUTEX_INITIALIZER;

/* 中文逐项-常量 X1163：GDRAPI_LIBNAME
 * 编译期定义：GDRAPI_LIBNAME = "libgdrapi.so"。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define GDRAPI_LIBNAME "libgdrapi.so"

/* 中文逐项-宏 M0134：LOAD_SYM
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 LOAD_SYM；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define LOAD_SYM(handle, symbol, funcptr) do {         \
    cast = (void**)&funcptr;                             \
    tmp = dlsym(handle, symbol);                         \
    if (tmp == NULL) {                                   \
      WARN("dlsym failed on %s - %s", symbol, dlerror());\
      goto teardown;                                     \
    }                                                    \
    *cast = tmp;                                         \
  } while (0)

/* 中文逐项-宏 M0135：LOAD_SYM_OPTIONAL
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 LOAD_SYM_OPTIONAL；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define LOAD_SYM_OPTIONAL(handle, symbol, funcptr) do {\
    cast = (void**)&funcptr;                             \
    tmp = dlsym(handle, symbol);                         \
    if (tmp == NULL) {                                   \
      INFO(NCCL_INIT,"dlsym failed on %s, ignoring", symbol); \
    }                                                    \
    *cast = tmp;                                         \
  } while (0)

/* 中文精读：GDRCopy 与 GPUDirect RDMA 不是同一接口
 * 此包装动态加载 libgdrapi，让 CPU 建立 GPU 显存的映射并执行小数据复制/读取。
 * 本仓库用它优化 work FIFO、进度同步或 flush；是否启用还取决于运行环境与参数。
 * GPUDirect RDMA 则指 NIC 直接访问已注册的 GPU 内存，走网络后端的 regMr/verbs 路径。
 * 两者可以配合，但没有 GDRCopy 不自动意味着所有 GDR 网络传输不可用。
 */
/* 中文逐项-函数 F0912：wrap_gdr_symbols
 * 用途与位置：解析 libgdrapi 动态符号；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：LOAD_SYM；__sync_bool_compare_and_swap；dlclose；dlopen；pthread_yield。
 * 需要对照的边界/推进条件：gdrState == gdrInitialized；gdrState == gdrError；__sync_bool_compare_and_swap(&gdrState,
 * gdrUninitialized, gdrInitializing) == false；gdrState == gdrInitializing。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclSystemError;；return (gdrState == gdrInitialized) ? ncclSuccess :
 * ncclSystemError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_symbols(void) {
  if (gdrState == gdrInitialized)
    return ncclSuccess;
  if (gdrState == gdrError)
    return ncclSystemError;

  if (__sync_bool_compare_and_swap(&gdrState, gdrUninitialized, gdrInitializing) == false) {
    // Another thread raced in front of us. Wait for it to be done.
    while (gdrState == gdrInitializing) pthread_yield();
    return (gdrState == gdrInitialized) ? ncclSuccess : ncclSystemError;
  }

/* 中文逐项-变量 V3359：gdrhandle
 * gdrhandle（void；函数 wrap_gdr_symbols 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 gdrhandle = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  static void* gdrhandle = NULL;
/* 中文逐项-变量 V3360：tmp
 * tmp（void；函数 wrap_gdr_symbols 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  void* tmp;
/* 中文逐项-变量 V3361：cast
 * cast（void；函数 wrap_gdr_symbols 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
  void** cast;

  gdrhandle=dlopen(GDRAPI_LIBNAME, RTLD_NOW);
  if (!gdrhandle) {
    WARN("Failed to open %s", GDRAPI_LIBNAME);
    goto teardown;
  }

  /* Load the function pointers from the DL library image */
  LOAD_SYM(gdrhandle, "gdr_open", gdr_internal_open);
  LOAD_SYM(gdrhandle, "gdr_close", gdr_internal_close);
  LOAD_SYM(gdrhandle, "gdr_pin_buffer", gdr_internal_pin_buffer);
  LOAD_SYM(gdrhandle, "gdr_unpin_buffer", gdr_internal_unpin_buffer);
  LOAD_SYM(gdrhandle, "gdr_get_info", gdr_internal_get_info);
  LOAD_SYM(gdrhandle, "gdr_map", gdr_internal_map);
  LOAD_SYM(gdrhandle, "gdr_unmap", gdr_internal_unmap);
  LOAD_SYM(gdrhandle, "gdr_runtime_get_version", gdr_internal_runtime_get_version);
  LOAD_SYM(gdrhandle, "gdr_driver_get_version", gdr_internal_driver_get_version);
  LOAD_SYM(gdrhandle, "gdr_copy_to_mapping", gdr_internal_copy_to_mapping);
  LOAD_SYM(gdrhandle, "gdr_copy_from_mapping", gdr_internal_copy_from_mapping);

  gdrState = gdrInitialized;
  return ncclSuccess;

teardown:
  gdr_internal_open = NULL;
  gdr_internal_close = NULL;
  gdr_internal_pin_buffer = NULL;
  gdr_internal_unpin_buffer = NULL;
  gdr_internal_get_info = NULL;
  gdr_internal_map = NULL;
  gdr_internal_unmap = NULL;
  gdr_internal_runtime_get_version = NULL;
  gdr_internal_driver_get_version = NULL;
  gdr_internal_copy_to_mapping = NULL;
  gdr_internal_copy_from_mapping = NULL;

  if (gdrhandle != NULL) dlclose(gdrhandle);
  gdrState = gdrError;
  return ncclSystemError;
}


/* 中文逐项-函数 F0913：wrap_gdr_open
 * 用途与位置：打开 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：gdr_internal_open。
 * 需要对照的边界/推进条件：gdr_internal_open == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;；return gdr_internal_open();。CHECK类宏还可能提前返回/跳转。
 */
gdr_t wrap_gdr_open(void) {
  if (gdr_internal_open == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return NULL;
  }
  return gdr_internal_open();
}

/* 中文逐项-函数 F0914：wrap_gdr_close
 * 用途与位置：关闭 GDRCopy 实例；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：gdr_internal_close。
 * 需要对照的边界/推进条件：gdr_internal_close == NULL；ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_close(gdr_t g) {
  if (gdr_internal_close == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
/* 中文逐项-变量 V3362：ret
 * ret（int；函数 wrap_gdr_close 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = gdr_internal_close(g)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int ret = gdr_internal_close(g);
  if (ret != 0) {
    WARN("gdr_close() failed: %d", ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0915：wrap_gdr_pin_buffer
 * 用途与位置：固定 GPU 显存以供映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 addr（unsigned long addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 p2p_token（uint64_t p2p_token）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 va_space（uint32_t va_space）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 handle（gdr_mh_t *handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaCalloc。
 * 本体直接出现的调用/包装：GDRLOCKCALL；gdr_internal_pin_buffer。
 * 需要对照的边界/推进条件：gdr_internal_pin_buffer == NULL；ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_pin_buffer(gdr_t g, unsigned long addr, size_t size, uint64_t p2p_token, uint32_t va_space, gdr_mh_t *handle) {
  if (gdr_internal_pin_buffer == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
/* 中文逐项-变量 V3363：ret
 * ret（int；函数 wrap_gdr_pin_buffer 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：GDRLOCKCALL(gdr_internal_pin_buffer(g, addr, size, p2p_token, va_space, handle),
 * ret);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ret;
  GDRLOCKCALL(gdr_internal_pin_buffer(g, addr, size, p2p_token, va_space, handle), ret);
  if (ret != 0) {
    WARN("gdr_pin_buffer(addr %lx, size %zi) failed: %d", addr, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0916：wrap_gdr_unpin_buffer
 * 用途与位置：解除 GPU 显存固定；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaFree。
 * 本体直接出现的调用/包装：GDRLOCKCALL；gdr_internal_unpin_buffer。
 * 需要对照的边界/推进条件：gdr_internal_unpin_buffer == NULL；ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_unpin_buffer(gdr_t g, gdr_mh_t handle) {
  if (gdr_internal_unpin_buffer == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
/* 中文逐项-变量 V3364：ret
 * ret（int；函数 wrap_gdr_unpin_buffer 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：GDRLOCKCALL(gdr_internal_unpin_buffer(g, handle), ret);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ret;
  GDRLOCKCALL(gdr_internal_unpin_buffer(g, handle), ret);
  if (ret != 0) {
    WARN("gdr_unpin_buffer(handle %lx) failed: %d", handle.h, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0917：wrap_gdr_get_info
 * 用途与位置：读取 GPU 显存映射属性及页偏移；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 info（gdr_info_t *info）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaCalloc。
 * 本体直接出现的调用/包装：GDRLOCKCALL；gdr_internal_get_info。
 * 需要对照的边界/推进条件：gdr_internal_get_info == NULL；ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_get_info(gdr_t g, gdr_mh_t handle, gdr_info_t *info) {
  if (gdr_internal_get_info == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
/* 中文逐项-变量 V3365：ret
 * ret（int；函数 wrap_gdr_get_info 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：GDRLOCKCALL(gdr_internal_get_info(g, handle, info), ret);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ret;
  GDRLOCKCALL(gdr_internal_get_info(g, handle, info), ret);
  if (ret != 0) {
    WARN("gdr_get_info(handle %lx) failed: %d", handle.h, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0918：wrap_gdr_map
 * 用途与位置：把已固定 GPU 内存映射到 CPU 地址空间；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 va（void **va）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaCalloc。
 * 本体直接出现的调用/包装：GDRLOCKCALL；gdr_internal_map。
 * 需要对照的边界/推进条件：gdr_internal_map == NULL；ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_map(gdr_t g, gdr_mh_t handle, void **va, size_t size) {
  if (gdr_internal_map == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
/* 中文逐项-变量 V3366：ret
 * ret（int；函数 wrap_gdr_map 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：GDRLOCKCALL(gdr_internal_map(g, handle, va, size), ret);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ret;
  GDRLOCKCALL(gdr_internal_map(g, handle, va, size), ret);
  if (ret != 0) {
    WARN("gdr_map(handle %lx, size %zi) failed: %d", handle.h, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0919：wrap_gdr_unmap
 * 用途与位置：解除 CPU 映射；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 va（void *va）：GDRCopy映射的GPU虚拟地址、映射字节容量、页尺寸及驱动时间计数信息。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaFree。
 * 本体直接出现的调用/包装：GDRLOCKCALL；gdr_internal_unmap。
 * 需要对照的边界/推进条件：gdr_internal_unmap == NULL；ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_unmap(gdr_t g, gdr_mh_t handle, void *va, size_t size) {
  if (gdr_internal_unmap == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
/* 中文逐项-变量 V3367：ret
 * ret（int；函数 wrap_gdr_unmap 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：GDRLOCKCALL(gdr_internal_unmap(g, handle, va, size), ret);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ret;
  GDRLOCKCALL(gdr_internal_unmap(g, handle, va, size), ret);
  if (ret != 0) {
    WARN("gdr_unmap(handle %lx, va %p, size %zi) failed: %d", handle.h, va, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0920：wrap_gdr_runtime_get_version
 * 用途与位置：查询 GDRCopy 用户库版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 major（int *major）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 minor（int *minor）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：gdr_internal_runtime_get_version。
 * 需要对照的边界/推进条件：gdr_internal_runtime_get_version == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_runtime_get_version(int *major, int *minor) {
  if (gdr_internal_runtime_get_version == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  gdr_internal_runtime_get_version(major, minor);
  return ncclSuccess;
}

/* 中文逐项-函数 F0921：wrap_gdr_driver_get_version
 * 用途与位置：查询 GDRCopy 驱动版本；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 g（gdr_t g）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 参数 major（int *major）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 minor（int *minor）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrInit。
 * 本体直接出现的调用/包装：gdr_internal_driver_get_version。
 * 需要对照的边界/推进条件：gdr_internal_driver_get_version == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_driver_get_version(gdr_t g, int *major, int *minor) {
  if (gdr_internal_driver_get_version == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
  gdr_internal_driver_get_version(g, major, minor);
  return ncclSuccess;
}

/* 中文逐项-函数 F0922：wrap_gdr_copy_to_mapping
 * 用途与位置：向 CPU 可见的 GPU 映射复制数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 map_d_ptr（void *map_d_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 h_ptr（const void *h_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/include/gdrwrap.h::ncclGdrCudaCopy。
 * 本体直接出现的调用/包装：GDRLOCKCALL；gdr_internal_copy_to_mapping。
 * 需要对照的边界/推进条件：gdr_internal_copy_to_mapping == NULL；ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_copy_to_mapping(gdr_mh_t handle, void *map_d_ptr, const void *h_ptr, size_t size) {
  if (gdr_internal_copy_to_mapping == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
/* 中文逐项-变量 V3368：ret
 * ret（int；函数 wrap_gdr_copy_to_mapping 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：GDRLOCKCALL(gdr_internal_copy_to_mapping(handle, map_d_ptr, h_ptr, size),
 * ret);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ret;
  GDRLOCKCALL(gdr_internal_copy_to_mapping(handle, map_d_ptr, h_ptr, size), ret);
  if (ret != 0) {
    WARN("gdr_copy_to_mapping(handle %lx, map_d_ptr %p, h_ptr %p, size %zi) failed: %d", handle.h, map_d_ptr, h_ptr, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0923：wrap_gdr_copy_from_mapping
 * 用途与位置：从 CPU 可见的 GPU 映射读回数据；按编译/运行支持提供真实调用或不支持的回退，注意配对释放。
 * 参数 handle（gdr_mh_t handle）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 参数 h_ptr（void *h_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 map_d_ptr（const void *map_d_ptr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 size（size_t size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GDRLOCKCALL；gdr_internal_copy_from_mapping。
 * 需要对照的边界/推进条件：gdr_internal_copy_from_mapping == NULL；ret != 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_gdr_copy_from_mapping(gdr_mh_t handle, void *h_ptr, const void *map_d_ptr, size_t size) {
  if (gdr_internal_copy_from_mapping == NULL) {
    WARN("GDRCOPY lib wrapper not initialized.");
    return ncclInternalError;
  }
/* 中文逐项-变量 V3369：ret
 * ret（int；函数 wrap_gdr_copy_from_mapping 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：GDRLOCKCALL(gdr_internal_copy_from_mapping(handle, h_ptr, map_d_ptr, size),
 * ret);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int ret;
  GDRLOCKCALL(gdr_internal_copy_from_mapping(handle, h_ptr, map_d_ptr, size), ret);
  if (ret != 0) {
    WARN("gdr_copy_from_mapping(handle %lx, h_ptr %p, map_d_ptr %p, size %zi) failed: %d", handle.h, h_ptr, map_d_ptr, size, ret);
    return ncclSystemError;
  }
  return ncclSuccess;
}

#endif /* !GDR_DIRECT */
