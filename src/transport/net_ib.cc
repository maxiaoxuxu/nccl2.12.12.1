/*************************************************************************
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "nccl.h"
#include "core.h"
#include "socket.h"
#include "net.h"
#include "graph.h"
#include "utils.h"
#include "param.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <unistd.h>
/* 中文逐项-常量 X1267：ENABLE_TIMER
 * 编译期定义：ENABLE_TIMER = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define ENABLE_TIMER 0
#include "timer.h"

#include "ibvwrap.h"

/* 中文逐项-常量 X1268：USE_RDMA_WRITE
 * 编译期定义：USE_RDMA_WRITE = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define USE_RDMA_WRITE 1
/* 中文逐项-常量 X1269：MAXNAMESIZE
 * 编译期定义：MAXNAMESIZE = 64。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAXNAMESIZE 64
/* 中文逐项-变量 V4193：ncclIbIfName
 * ncclIbIfName（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclFindInterfaces(ncclIbIfName, &ncclIbIfAddr, MAX_IF_NAME_SIZE, 1) != 1)
 * {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static char ncclIbIfName[MAX_IF_NAME_SIZE+1];
/* 中文逐项-变量 V4194：ncclIbIfAddr
 * ncclIbIfAddr（union ncclSocketAddress；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclFindInterfaces(ncclIbIfName, &ncclIbIfAddr, MAX_IF_NAME_SIZE, 1) != 1)
 * {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
static union ncclSocketAddress ncclIbIfAddr;

/* 中文导读：
 * MR 缓存条目：页对齐起始地址、页数、引用计数及实际 ibv_mr。
 * 同一 IB 设备缓存内，只有起始地址与页数都相同才命中；不是任意重叠/包含区间都复用。
 */
/* 中文逐项-类型 T0423：ncclIbMr
 * 类型职责：真实MR缓存项及设备缓存容器，以页范围和引用计数避免相同缓冲重复注册。
 * 声明内可见成员（含嵌套成员）：addr、pages、refs、mr。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbMr {
/* 中文逐项-变量 V4195：addr
 * addr（uintptr_t；成员 ncclIbMr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
  uintptr_t addr;
/* 中文逐项-变量 V4196：pages
 * pages（int；成员 ncclIbMr）：注册覆盖的页数，与页对齐起点共同组成缓存匹配范围。
 */
  int pages;
/* 中文逐项-变量 V4197：refs
 * refs（int；成员 ncclIbMr）：共享对象引用计数；只有最后引用释放才允许回收真实资源。
 */
  int refs;
/* 中文逐项-变量 V4198：mr
 * mr（ibv_mr；成员 ncclIbMr）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 */
  ibv_mr *mr;
};

/* 中文逐项-类型 T0424：ncclIbMrCache
 * 类型职责：真实MR缓存项及设备缓存容器，以页范围和引用计数避免相同缓冲重复注册。
 * 声明内可见成员（含嵌套成员）：slots、capacity、population。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbMrCache {
/* 中文逐项-变量 V4199：slots
 * slots（struct ncclIbMr；成员 ncclIbMrCache）：缓存/池的存储数组、总容量和已占用条目统计，扩容/删除须保持一致。
 */
  struct ncclIbMr *slots;
/* 中文逐项-变量 V4200：capacity、population
 * capacity（int；成员 ncclIbMrCache）：缓存/池的存储数组、总容量和已占用条目统计，扩容/删除须保持一致。
 * population（int；成员 ncclIbMrCache）：缓存/池的存储数组、总容量和已占用条目统计，扩容/删除须保持一致。
 */
  int capacity, population;
};

/* 中文逐项-变量 V4201：ncclNIbDevs
 * ncclNIbDevs（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclNIbDevs = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
static int ncclNIbDevs = -1;
/* 中文逐项-类型 T0425：ncclIbDev
 * 类型职责：已发现可用verbs端口的能力/PD/MR缓存信息及用户接口筛选配置。
 * 声明内可见成员（含嵌套成员）：lock、device、guid、port、link、speed、context、pdRefs、pd、devName、pciPath、realPort、maxQp、mrCache。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct alignas(64) ncclIbDev {
/* 中文逐项-变量 V4202：lock
 * lock（pthread_mutex_t；成员 ncclIbDev）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
  pthread_mutex_t lock;
/* 中文逐项-变量 V4203：device
 * device（int；成员 ncclIbDev）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 */
  int device;
/* 中文逐项-变量 V4204：guid
 * guid（uint64_t；成员 ncclIbDev）：网络设备GUID，用于识别相关设备/端口与稳定硬件身份。
 */
  uint64_t guid;
/* 中文逐项-变量 V4205：port
 * port（uint8_t；成员 ncclIbDev）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
  uint8_t port;
/* 中文逐项-变量 V4206：link
 * link（uint8_t；成员 ncclIbDev）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 */
  uint8_t link;
/* 中文逐项-变量 V4207：speed
 * speed（int；成员 ncclIbDev）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 */
  int speed;
/* 中文逐项-变量 V4208：context
 * context（ibv_context；成员 ncclIbDev）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
  ibv_context* context;
/* 中文逐项-变量 V4209：pdRefs
 * pdRefs（int；成员 ncclIbDev）：共享对象引用计数；只有最后引用释放才允许回收真实资源。
 */
  int pdRefs;
/* 中文逐项-变量 V4210：pd
 * pd（ibv_pd；成员 ncclIbDev）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
  ibv_pd* pd;
/* 中文逐项-变量 V4211：devName
 * devName（char；成员 ncclIbDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  char devName[MAXNAMESIZE];
/* 中文逐项-变量 V4212：pciPath
 * pciPath（char；成员 ncclIbDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  char* pciPath;
/* 中文逐项-变量 V4213：realPort
 * realPort（int；成员 ncclIbDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int realPort;
/* 中文逐项-变量 V4214：maxQp
 * maxQp（int；成员 ncclIbDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  int maxQp;
/* 中文逐项-变量 V4215：mrCache
 * mrCache（struct ncclIbMrCache；成员 ncclIbDev）：设备级MR缓存表，按页范围查找并以引用计数管理真实注册。
 */
  struct ncclIbMrCache mrCache;
};

/* 中文逐项-常量 X1270：MAX_IB_PORT
 * 编译期定义：MAX_IB_PORT = 15。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_IB_PORT 15
/* 中文逐项-类型 T0426：userIbDev
 * 类型职责：已发现可用verbs端口的能力/PD/MR缓存信息及用户接口筛选配置。
 * 声明内可见成员（含嵌套成员）：devName、port_en。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct userIbDev {
/* 中文逐项-变量 V4216：devName
 * devName（char；成员 userIbDev）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 */
  char devName[MAXNAMESIZE];
/* 中文逐项-变量 V4217：port_en
 * port_en（uint16_t；成员 userIbDev）：当前RDMA设备链路类型/端口启用过滤位，接口枚举和筛选时使用。
 */
  uint16_t port_en;
};

/* 中文逐项-常量 X1271：MAX_IB_DEVS
 * 编译期定义：MAX_IB_DEVS = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_IB_DEVS 16
/* 中文逐项-变量 V4218：ncclIbDevs
 * ncclIbDevs（struct ncclIbDev；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (strcmp(p, ncclIbDevs[d].pciPath) == 0) (*realPort)++;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
struct ncclIbDev ncclIbDevs[MAX_IB_DEVS];
/* 中文逐项-变量 V4219：userIbDevs
 * userIbDevs（struct userIbDev；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
struct userIbDev userIbDevs[MAX_IB_DEVS];
/* 中文逐项-变量 V4220：ncclIbLock
 * ncclIbLock（pthread_mutex_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclIbLock = PTHREAD_MUTEX_INITIALIZER；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
pthread_mutex_t ncclIbLock = PTHREAD_MUTEX_INITIALIZER;
/* 中文逐项-变量 V4221：ncclIbRelaxedOrderingEnabled
 * ncclIbRelaxedOrderingEnabled（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclIbRelaxedOrderingEnabled = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
static int ncclIbRelaxedOrderingEnabled = 0;

/* 中文逐项-参数getter P0040：IbGidIndex
 * 生成函数 ncclParamIbGidIndex()，读取 NCCL_IB_GID_INDEX，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbGidIndex, "IB_GID_INDEX", 0);
/* 中文逐项-参数getter P0041：IbTimeout
 * 生成函数 ncclParamIbTimeout()，读取 NCCL_IB_TIMEOUT，编译默认值为 14。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbTimeout, "IB_TIMEOUT", 14);
/* 中文逐项-参数getter P0042：IbRetryCnt
 * 生成函数 ncclParamIbRetryCnt()，读取 NCCL_IB_RETRY_CNT，编译默认值为 7。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbRetryCnt, "IB_RETRY_CNT", 7);
/* 中文逐项-参数getter P0043：IbPkey
 * 生成函数 ncclParamIbPkey()，读取 NCCL_IB_PKEY，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbPkey, "IB_PKEY", 0);
/* 中文逐项-参数getter P0044：IbUseInline
 * 生成函数 ncclParamIbUseInline()，读取 NCCL_IB_USE_INLINE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbUseInline, "IB_USE_INLINE", 0);
/* 中文逐项-参数getter P0045：IbSl
 * 生成函数 ncclParamIbSl()，读取 NCCL_IB_SL，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbSl, "IB_SL", 0);
/* 中文逐项-参数getter P0046：IbTc
 * 生成函数 ncclParamIbTc()，读取 NCCL_IB_TC，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbTc, "IB_TC", 0);
/* 中文逐项-参数getter P0047：IbArThreshold
 * 生成函数 ncclParamIbArThreshold()，读取 NCCL_IB_AR_THRESHOLD，编译默认值为 8192。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbArThreshold, "IB_AR_THRESHOLD", 8192);
/* 中文逐项-参数getter P0048：IbPciRelaxedOrdering
 * 生成函数 ncclParamIbPciRelaxedOrdering()，读取 NCCL_IB_PCI_RELAXED_ORDERING，编译默认值为 2。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbPciRelaxedOrdering, "IB_PCI_RELAXED_ORDERING", 2);

/* 中文逐项-变量 V4222：ncclIbAsyncThread
 * ncclIbAsyncThread（pthread_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：pthread_create(&ncclIbAsyncThread, NULL, ncclIbAsyncThreadMain,
 * context);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
pthread_t ncclIbAsyncThread;
/* 中文逐项-函数 F1096：ncclIbAsyncThreadMain
 * 用途与位置：持续读取并确认 IB 设备异步事件，发现端口/设备异常时记录诊断信息。
 * 参数 args（void* args）：当前执行入口收到的参数对象，主机线程、Proxy运行态与设备work参数的类型不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：wrap_ibv_ack_async_event；wrap_ibv_event_type_str；wrap_ibv_get_async_event。
 * 需要对照的边界/推进条件：1；ncclSuccess != wrap_ibv_get_async_event(context, &event；ncclSuccess != wrap_ibv_event_type_str(&str,
 * event.event_type；event.event_type != IBV_EVENT_COMM_EST。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;。CHECK类宏还可能提前返回/跳转。
 */
static void* ncclIbAsyncThreadMain(void* args) {
/* 中文逐项-变量 V4223：context
 * context（struct ibv_context；函数 ncclIbAsyncThreadMain 内）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 初值/来源 context = (struct ibv_context*)args；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ibv_context* context = (struct ibv_context*)args;
  while (1) {
/* 中文逐项-变量 V4224：event
 * event（struct ibv_async_event；函数 ncclIbAsyncThreadMain 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclSuccess != wrap_ibv_get_async_event(context, &event)) { break;
 * }。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ibv_async_event event;
    if (ncclSuccess != wrap_ibv_get_async_event(context, &event)) { break; }
/* 中文逐项-变量 V4225：str
 * str（char；函数 ncclIbAsyncThreadMain 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：if (ncclSuccess != wrap_ibv_event_type_str(&str, event.event_type)) { break;
 * }。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    char *str;
    if (ncclSuccess != wrap_ibv_event_type_str(&str, event.event_type)) { break; }
    if (event.event_type != IBV_EVENT_COMM_EST)
      WARN("NET/IB : Got async event : %s", str);
    if (ncclSuccess != wrap_ibv_ack_async_event(&event)) { break; }
  }
  return NULL;
}

/* 中文逐项-参数getter P0049：IbDisable
 * 生成函数 ncclParamIbDisable()，读取 NCCL_IB_DISABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbDisable, "IB_DISABLE", 0);

/* 中文逐项-函数 F1097：ncclIbGetPciPath
 * 用途与位置：获取 verbs 设备在 sysfs 的 PCI 路径和实际端口信息，使拓扑可关联该 NIC。
 * 参数 devName（char* devName）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 参数 path（char** path）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 realPort（int* realPort）：实际端口、QP能力上限、设备名、PCI路径和所选后端设备编号。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：realpath；snprintf；strcmp；strlen。
 * 需要对照的边界/推进条件：p == NULL；d<ncclNIbDevs；strcmp(p, ncclIbDevs[d].pciPath) == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclIbGetPciPath(char* devName, char** path, int* realPort) {
/* 中文逐项-变量 V4226：devicePath
 * devicePath（char；函数 ncclIbGetPciPath 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：snprintf(devicePath, PATH_MAX, "/sys/class/infiniband/%s/device",
 * devName);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char devicePath[PATH_MAX];
  snprintf(devicePath, PATH_MAX, "/sys/class/infiniband/%s/device", devName);
/* 中文逐项-变量 V4227：p
 * p（char；函数 ncclIbGetPciPath 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 p = realpath(devicePath, NULL)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  char* p = realpath(devicePath, NULL);
  if (p == NULL) {
    WARN("Could not find real path of %s (%s)", devName, devicePath);
  } else {
    // Merge multi-port NICs into the same PCI device
    p[strlen(p)-1] = '0';
    // Also merge virtual functions (VF) into the same device
    p[strlen(p)-3] = '0';
    // And keep the real port aside (the ibv port is always 1 on recent cards)
    *realPort = 0;
    for (/* 中文逐项-变量 V4228：d
 * d（int；函数 ncclIbGetPciPath 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：d<ncclNIbDevs；循环体与增量表达式决定每轮如何推进。
 */int d=0; d<ncclNIbDevs; d++) {
      if (strcmp(p, ncclIbDevs[d].pciPath) == 0) (*realPort)++;
    }
  }
  *path = p;
  return ncclSuccess;
}

/* 中文逐项-变量 V4229：ibvWidths
 * ibvWidths（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ibvWidths = { 1, 4, 8, 12 }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static int ibvWidths[] = { 1, 4, 8, 12 };
/* 中文逐项-变量 V4230：ibvSpeeds
 * ibvSpeeds（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ibvSpeeds = { 2500, 5000, 10000, 10000, 14000, 25000, 50000
 * }；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
static int ibvSpeeds[] = { 2500, 5000, 10000, 10000, 14000, 25000, 50000 };
/* 中文逐项-函数 F1098：firstBitSet
 * 用途与位置：定位位掩码最低有效位，把 IB 能力字段转换为对应宽度/速度条目。
 * 参数 val（int val）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 * 参数 max（int max）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbWidth；src/transport/net_ib.cc::ncclIbSpeed。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<max && ((val & (1<<i)) == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return i;。CHECK类宏还可能提前返回/跳转。
 */
static int firstBitSet(int val, int max) {
/* 中文逐项-变量 V4231：i
 * i（int；函数 firstBitSet 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int i = 0;
  while (i<max && ((val & (1<<i)) == 0)) i++;
  return i;
}
/* 中文逐项-函数 F1099：ncclIbWidth
 * 用途与位置：把 verbs 端口 active_width/active_speed 编码转成数值，用于报告链路带宽。
 * 参数 width（int width）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：firstBitSet。
 * 源码返回表达式：return ibvWidths[firstBitSet(width, sizeof(ibvWidths)/sizeof(int)-1)];。CHECK类宏还可能提前返回/跳转。
 */
static int ncclIbWidth(int width) {
  return ibvWidths[firstBitSet(width, sizeof(ibvWidths)/sizeof(int)-1)];
}
/* 中文逐项-函数 F1100：ncclIbSpeed
 * 用途与位置：把 verbs 端口 active_width/active_speed 编码转成数值，用于报告链路带宽。
 * 参数 speed（int speed）：链路/路径速度或延迟的估计值，单位和层次由设备属性/图模型字段约定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：firstBitSet。
 * 源码返回表达式：return ibvSpeeds[firstBitSet(speed, sizeof(ibvSpeeds)/sizeof(int)-1)];。CHECK类宏还可能提前返回/跳转。
 */
static int ncclIbSpeed(int speed) {
  return ibvSpeeds[firstBitSet(speed, sizeof(ibvSpeeds)/sizeof(int)-1)];
}

// Determine whether RELAXED_ORDERING is enabled and possible
/* 中文逐项-函数 F1101：ncclIbRelaxedOrderingCapable
 * 用途与位置：检查编译/运行支持和参数策略，判断 MR 注册能否启用 PCIe relaxed ordering。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：ncclParamIbPciRelaxedOrdering；wrap_ibv_reg_mr_iova2。
 * 需要对照的边界/推进条件：roMode == 1 || roMode == 2。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return r == ncclInternalError ? 0 : 1;。CHECK类宏还可能提前返回/跳转。
 */
static int ncclIbRelaxedOrderingCapable(void) {
/* 中文逐项-变量 V4232：roMode
 * roMode（int；函数 ncclIbRelaxedOrderingCapable 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 roMode = ncclParamIbPciRelaxedOrdering()；调用对应接口取得结果；成功、容量与生命周期要按该接口返回契约判断。
 */
  int roMode = ncclParamIbPciRelaxedOrdering();
/* 中文逐项-变量 V4233：r
 * r（ncclResult_t；函数 ncclIbRelaxedOrderingCapable 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 r = ncclInternalError；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  ncclResult_t r = ncclInternalError;
  if (roMode == 1 || roMode == 2) {
    // Query IBVERBS_1.8 API - needed for IBV_ACCESS_RELAXED_ORDERING support
    r = wrap_ibv_reg_mr_iova2(NULL, NULL, NULL, 0, 0, 0);
  }
  return r == ncclInternalError ? 0 : 1;
}

/* 中文逐项-函数 F1102：ncclIbInit
 * 用途与位置：初始化 libibverbs、枚举活动设备/端口、筛选接口、获取属性并启动设备异步事件监控。
 * 参数 logFunction（ncclDebugLogger_t logFunction）：NCCL提供的日志回调，插件用它按统一级别/子系统报告信息与错误。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；getenv；matchIfList；memset；ncclFindInterfaces；ncclIbGetPciPath；ncclIbRelaxedOrderingCapable；ncclIbSpeed；ncclIbWidth；ncclParamIbDisable；ncclSetThreadName；ncclSocketToString；parseStringList；pthread_create；pthread_detach；pthread_mutex_init；pthread_mutex_lock；pthread_mutex_unlock；更多调用保留在函数体。
 * 需要对照的边界/推进条件：ncclParamIbDisable；wrap_ibv_symbols() != ncclSuccess；ncclNIbDevs == -1；ncclNIbDevs ==
 * -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclIbInit(ncclDebugLogger_t logFunction) {
  if (ncclParamIbDisable()) return ncclInternalError;
/* 中文逐项-变量 V4234：shownIbHcaEnv
 * shownIbHcaEnv（int；函数 ncclIbInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shownIbHcaEnv = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  static int shownIbHcaEnv = 0;
  if(wrap_ibv_symbols() != ncclSuccess) { return ncclInternalError; }

  if (ncclNIbDevs == -1) {
    pthread_mutex_lock(&ncclIbLock);
    wrap_ibv_fork_init();
    if (ncclNIbDevs == -1) {
      ncclNIbDevs = 0;
      if (ncclFindInterfaces(ncclIbIfName, &ncclIbIfAddr, MAX_IF_NAME_SIZE, 1) != 1) {
        WARN("NET/IB : No IP interface found.");
        return ncclInternalError;
      }

      // Detect IB cards
/* 中文逐项-变量 V4235：nIbDevs
 * nIbDevs（int；函数 ncclIbInit 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 此处未显式给初值；后续首个文本使用点：if (ncclSuccess != wrap_ibv_get_device_list(&devices, &nIbDevs)) return
 * ncclInternalError;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      int nIbDevs;
/* 中文逐项-变量 V4236：devices
 * devices（struct ibv_device；函数 ncclIbInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclSuccess != wrap_ibv_get_device_list(&devices, &nIbDevs)) return
 * ncclInternalError;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      struct ibv_device** devices;

      // Check if user defined which IB device:port to use
/* 中文逐项-变量 V4237：userIbEnv
 * userIbEnv（char；函数 ncclIbInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 userIbEnv = getenv("NCCL_IB_HCA")；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      char* userIbEnv = getenv("NCCL_IB_HCA");
      if (userIbEnv != NULL && shownIbHcaEnv++ == 0) INFO(NCCL_NET|NCCL_ENV, "NCCL_IB_HCA set to %s", userIbEnv);
/* 中文逐项-变量 V4238：userIfs
 * userIfs（struct netIf；函数 ncclIbInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：int nUserIfs = parseStringList(userIbEnv, userIfs, MAX_IB_DEVS);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      struct netIf userIfs[MAX_IB_DEVS];
/* 中文逐项-变量 V4239：searchNot
 * searchNot（bool；函数 ncclIbInit 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 searchNot = userIbEnv && userIbEnv[0] == '^'；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      bool searchNot = userIbEnv && userIbEnv[0] == '^';
      if (searchNot) userIbEnv++;
/* 中文逐项-变量 V4240：searchExact
 * searchExact（bool；函数 ncclIbInit 内）：本作用域条件状态；为真时启用下面受它保护的分支，不独立创建硬件资源。
 * 初值/来源 searchExact = userIbEnv && userIbEnv[0] == '='；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      bool searchExact = userIbEnv && userIbEnv[0] == '=';
      if (searchExact) userIbEnv++;
/* 中文逐项-变量 V4241：nUserIfs
 * nUserIfs（int；函数 ncclIbInit 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nUserIfs = parseStringList(userIbEnv, userIfs, MAX_IB_DEVS)；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      int nUserIfs = parseStringList(userIbEnv, userIfs, MAX_IB_DEVS);

      if (ncclSuccess != wrap_ibv_get_device_list(&devices, &nIbDevs)) return ncclInternalError;

      for (/* 中文逐项-变量 V4242：d
 * d（int；函数 ncclIbInit 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：d<nIbDevs && ncclNIbDevs<MAX_IB_DEVS；循环体与增量表达式决定每轮如何推进。
 */int d=0; d<nIbDevs && ncclNIbDevs<MAX_IB_DEVS; d++) {
/* 中文逐项-变量 V4243：context
 * context（struct ibv_context；函数 ncclIbInit 内）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 此处未显式给初值；后续首个文本使用点：if (ncclSuccess != wrap_ibv_open_device(&context, devices[d]) || context == NULL)
 * {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        struct ibv_context * context;
        if (ncclSuccess != wrap_ibv_open_device(&context, devices[d]) || context == NULL) {
          WARN("NET/IB : Unable to open device %s", devices[d]->name);
          continue;
        }
/* 中文逐项-变量 V4244：nPorts
 * nPorts（int；函数 ncclIbInit 内）：对应对象/条目的数量，用于限制后续循环、数组容量或并行度。
 * 初值/来源 nPorts = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
        int nPorts = 0;
/* 中文逐项-变量 V4245：devAttr
 * devAttr（struct ibv_device_attr；函数 ncclIbInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(&devAttr, 0, sizeof(devAttr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        struct ibv_device_attr devAttr;
        memset(&devAttr, 0, sizeof(devAttr));
        if (ncclSuccess != wrap_ibv_query_device(context, &devAttr)) {
          WARN("NET/IB : Unable to query device %s", devices[d]->name);
          if (ncclSuccess != wrap_ibv_close_device(context)) { return ncclInternalError; }
          continue;
        }
        for (/* 中文逐项-变量 V4246：port
 * port（int；函数 ncclIbInit 内）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 初值/来源 port = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：port <= devAttr.phys_port_cnt；循环体与增量表达式决定每轮如何推进。
 */int port = 1; port <= devAttr.phys_port_cnt; port++) {
/* 中文逐项-变量 V4247：portAttr
 * portAttr（struct ibv_port_attr；函数 ncclIbInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：if (ncclSuccess != wrap_ibv_query_port(context, port, &portAttr)) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
          struct ibv_port_attr portAttr;
          if (ncclSuccess != wrap_ibv_query_port(context, port, &portAttr)) {
            WARN("NET/IB : Unable to query port %d", port);
            continue;
          }
          if (portAttr.state != IBV_PORT_ACTIVE) continue;
          if (portAttr.link_layer != IBV_LINK_LAYER_INFINIBAND
              && portAttr.link_layer != IBV_LINK_LAYER_ETHERNET) continue;

          // check against user specified HCAs/ports
          if (! (matchIfList(devices[d]->name, port, userIfs, nUserIfs, searchExact) ^ searchNot)) {
            continue;
          }
          TRACE(NCCL_INIT|NCCL_NET,"NET/IB: [%d] %s:%d/%s ", d, devices[d]->name, port,
              portAttr.link_layer == IBV_LINK_LAYER_INFINIBAND ? "IB" : "RoCE");
          pthread_mutex_init(&ncclIbDevs[ncclNIbDevs].lock, NULL);
          ncclIbDevs[ncclNIbDevs].device = d;
          ncclIbDevs[ncclNIbDevs].guid = devAttr.sys_image_guid;
          ncclIbDevs[ncclNIbDevs].port = port;
          ncclIbDevs[ncclNIbDevs].link = portAttr.link_layer;
          ncclIbDevs[ncclNIbDevs].speed = ncclIbSpeed(portAttr.active_speed) * ncclIbWidth(portAttr.active_width);
          ncclIbDevs[ncclNIbDevs].context = context;
          ncclIbDevs[ncclNIbDevs].pdRefs = 0;
          ncclIbDevs[ncclNIbDevs].pd = NULL;
          strncpy(ncclIbDevs[ncclNIbDevs].devName, devices[d]->name, MAXNAMESIZE);
          NCCLCHECK(ncclIbGetPciPath(ncclIbDevs[ncclNIbDevs].devName, &ncclIbDevs[ncclNIbDevs].pciPath, &ncclIbDevs[ncclNIbDevs].realPort));
          ncclIbDevs[ncclNIbDevs].maxQp = devAttr.max_qp;
          ncclIbDevs[ncclNIbDevs].mrCache.capacity = 0;
          ncclIbDevs[ncclNIbDevs].mrCache.population = 0;
          ncclIbDevs[ncclNIbDevs].mrCache.slots = NULL;

          pthread_create(&ncclIbAsyncThread, NULL, ncclIbAsyncThreadMain, context);
          ncclSetThreadName(ncclIbAsyncThread, "NCCL IbAsync %2d", ncclNIbDevs);
          pthread_detach(ncclIbAsyncThread); // will not be pthread_join()'d
          ncclNIbDevs++;
          nPorts++;
        }
        if (nPorts == 0 && ncclSuccess != wrap_ibv_close_device(context)) { return ncclInternalError; }
      }
      if (nIbDevs && (ncclSuccess != wrap_ibv_free_device_list(devices))) { return ncclInternalError; };
    }
    if (ncclNIbDevs == 0) {
      INFO(NCCL_INIT|NCCL_NET, "NET/IB : No device found.");
    } else {
/* 中文逐项-变量 V4248：line
 * line（char；函数 ncclIbInit 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：line[0] = '\0';。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char line[1024];
      line[0] = '\0';
      // Determine whether RELAXED_ORDERING is enabled and possible
      ncclIbRelaxedOrderingEnabled = ncclIbRelaxedOrderingCapable();
      for (/* 中文逐项-变量 V4249：d
 * d（int；函数 ncclIbInit 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 d = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：d<ncclNIbDevs；循环体与增量表达式决定每轮如何推进。
 */int d=0; d<ncclNIbDevs; d++) {
        snprintf(line+strlen(line), 1023-strlen(line), " [%d]%s:%d/%s", d, ncclIbDevs[d].devName,
            ncclIbDevs[d].port, ncclIbDevs[d].link == IBV_LINK_LAYER_INFINIBAND ? "IB" : "RoCE");
      }
      line[1023] = '\0';
/* 中文逐项-变量 V4250：addrline
 * addrline（char；函数 ncclIbInit 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclIbIfName, ncclSocketToString(&ncclIbIfAddr, addrline));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char addrline[SOCKET_NAME_MAXLEN+1];
      INFO(NCCL_INIT|NCCL_NET, "NET/IB : Using%s %s; OOB %s:%s", line, ncclIbRelaxedOrderingEnabled ? "[RO]" : "",
           ncclIbIfName, ncclSocketToString(&ncclIbIfAddr, addrline));
    }
    pthread_mutex_unlock(&ncclIbLock);
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F1103：ncclIbDevices
 * 用途与位置：返回当前已初始化并通过筛选的 IB/RoCE 数据端口数量。
 * 参数 ndev（int* ndev）：当前初始化/枚举范围内的设备数，单进程InitAll中等于该组参与GPU数。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbDevices(int* ndev) {
  *ndev = ncclNIbDevs;
  return ncclSuccess;
}

// Detect whether GDR can work on a given NIC with the current CUDA device
// Returns :
// ncclSuccess : GDR works
// ncclSystemError : no module or module loaded but not supported by GPU
/* 中文逐项-函数 F1104：ncclIbGdrSupport
 * 用途与位置：按设备能力检测能否注册 GPU 缓冲，给上层报告 GDR 支持状态。
 * 参数 ibDev（int ibDev）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbGetProperties；src/transport/net_ib.cc::ncclIbAccept。
 * 本体直接出现的调用/包装：access。
 * 需要对照的边界/推进条件：moduleLoaded == -1；moduleLoaded == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbGdrSupport(int ibDev) {
/* 中文逐项-变量 V4251：moduleLoaded
 * moduleLoaded（int；函数 ncclIbGdrSupport 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 moduleLoaded = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  static int moduleLoaded = -1;
  if (moduleLoaded == -1) {
    // Check for the nv_peer_mem module being loaded
    moduleLoaded = ((access("/sys/kernel/mm/memory_peers/nv_mem/version", F_OK) == -1) &&
                    // Also support the new nvidia-peermem module
                    (access("/sys/kernel/mm/memory_peers/nvidia-peermem/version", F_OK) == -1)) ? 0 : 1;
  }
  if (moduleLoaded == 0) return ncclSystemError;
  return ncclSuccess;
}

/* 中文逐项-函数 F1105：GetSocketAddr
 * 用途与位置：为所选后端设备获得建链控制 socket 地址；控制握手地址与实际数据载荷通路不同。
 * 参数 addr（union ncclSocketAddress* addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbListen；src/transport/net_socket.cc::ncclSocketListen。
 * 本体直接出现的调用/包装：memcpy。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t GetSocketAddr(union ncclSocketAddress* addr) {
  memcpy(addr, &ncclIbIfAddr, sizeof(*addr));
  return ncclSuccess;
}

/* 中文逐项-常量 X1272：NCCL_NET_IB_MAX_RECVS
 * 编译期定义：NCCL_NET_IB_MAX_RECVS = 8。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_IB_MAX_RECVS 8

/* 中文逐项-函数 F1106：ncclIbGetProperties
 * 用途与位置：把端口名、PCI 路径、速度、GDR 支持及接口接收能力报告给 NCCL 拓扑/NET 层。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 props（ncclNetProperties_t* props）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ncclIbGdrSupport。
 * 需要对照的边界/推进条件：ncclIbGdrSupport(dev) != ncclSuccess。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbGetProperties(int dev, ncclNetProperties_t* props) {
  props->name = ncclIbDevs[dev].devName;
  props->pciPath = ncclIbDevs[dev].pciPath;
  props->guid = ncclIbDevs[dev].guid;
  props->ptrSupport = NCCL_PTR_HOST;
  if (ncclIbGdrSupport(dev) != ncclSuccess) {
    INFO(NCCL_NET,"NET/IB : GPU Direct RDMA Disabled for HCA %d '%s' (no module)", dev, ncclIbDevs[dev].devName);
  } else {
    props->ptrSupport |= NCCL_PTR_CUDA;
  }
  props->speed = ncclIbDevs[dev].speed;
  props->latency = 0; // Not set
  props->port = ncclIbDevs[dev].port + ncclIbDevs[dev].realPort;
  props->maxComms = ncclIbDevs[dev].maxQp;
  props->maxRecvs = NCCL_NET_IB_MAX_RECVS;
  return ncclSuccess;
}

// We need to support NCCL_NET_MAX_REQUESTS for each concurrent receive
/* 中文逐项-常量 X1273：MAX_REQUESTS
 * 编译期定义：MAX_REQUESTS = (NCCL_NET_MAX_REQUESTS*NCCL_NET_IB_MAX_RECVS)。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_REQUESTS (NCCL_NET_MAX_REQUESTS*NCCL_NET_IB_MAX_RECVS)
static_assert(MAX_REQUESTS <= 256, "request id are encoded in wr_id and we need up to 8 requests ids per completion");

/* 中文逐项-常量 X1274：NCCL_IB_MAX_QPS
 * 编译期定义：NCCL_IB_MAX_QPS = 128。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_IB_MAX_QPS 128

/* 中文逐项-类型 T0427：ncclIbQpInfo
 * 类型职责：通过控制socket交换的QPN/LID/GID/MTU/控制FIFO地址key摘要，使两端RC QP可进入RTR/RTS。
 * 声明内可见成员（含嵌套成员）：lid、ib_port、link_layer、qpn、spn、iid、mtu、fifoRkey、fifoAddr。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbQpInfo {
/* 中文逐项-变量 V4252：lid
 * lid（uint32_t；成员 ncclIbQpInfo）：IB本地/目标/子网管理器/源LID地址信息，RoCE全球路由通常改用GID路径。
 */
  uint32_t lid;
/* 中文逐项-变量 V4253：ib_port
 * ib_port（uint8_t；成员 ncclIbQpInfo）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
  uint8_t ib_port;
/* 中文逐项-变量 V4254：link_layer
 * link_layer（uint8_t；成员 ncclIbQpInfo）：端口底层链路类别，如InfiniBand或Ethernet，影响LID/GID和RoCE配置。
 */
  uint8_t link_layer;
/* 中文逐项-变量 V4255：qpn
 * qpn（uint32_t；成员 ncclIbQpInfo）：本地/远端QP编号，用于控制握手及连接目标匹配。
 */
  uint32_t qpn[NCCL_IB_MAX_QPS];

  // For RoCE
/* 中文逐项-变量 V4256：spn
 * spn（uint64_t；成员 ncclIbQpInfo）：128位GID的子网前缀和接口标识两个64位组成部分。
 */
  uint64_t spn;
/* 中文逐项-变量 V4257：iid
 * iid（uint64_t；成员 ncclIbQpInfo）：128位GID的子网前缀和接口标识两个64位组成部分。
 */
  uint64_t iid;
/* 中文逐项-变量 V4258：mtu
 * mtu（enum ibv_mtu；成员 ncclIbQpInfo）：路径最大传输单元选择，影响NIC拆包但不直接等于NCCL算法chunk尺寸。
 */
  enum ibv_mtu mtu;

  // FIFO RDMA info
/* 中文逐项-变量 V4259：fifoRkey
 * fifoRkey（uint32_t；成员 ncclIbQpInfo）：握手交换的远端控制FIFO授权key与地址，允许接收方发布下一轮目标信息。
 */
  uint32_t fifoRkey;
/* 中文逐项-变量 V4260：fifoAddr
 * fifoAddr（uint64_t；成员 ncclIbQpInfo）：握手交换的远端控制FIFO授权key与地址，允许接收方发布下一轮目标信息。
 */
  uint64_t fifoAddr;
};

/* 中文逐项-类型 T0428：ncclIbCommState
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclIbCommState {
/* 中文逐项-枚举项 X1275：ncclIbCommStateStart
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
  ncclIbCommStateStart = 0,
/* 中文逐项-枚举项 X1276：ncclIbCommStateConnect
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
  ncclIbCommStateConnect = 1,
/* 中文逐项-枚举项 X1277：ncclIbCommStateAccept
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
  ncclIbCommStateAccept = 3,
/* 中文逐项-枚举项 X1278：ncclIbCommStateSend
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
  ncclIbCommStateSend = 4,
/* 中文逐项-枚举项 X1279：ncclIbCommStateRecv
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
  ncclIbCommStateRecv = 5,
/* 中文逐项-枚举项 X1280：ncclIbCommStateConnected
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
  ncclIbCommStateConnected = 6,
};

/* 中文逐项-类型 T0429：ncclIbCommStage
 * 类型职责：非阻塞建链的阶段/暂存及可交换监听句柄，下一次调用继续先前握手。
 * 声明内可见成员（含嵌套成员）：state、offset、buffer、comm。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbCommStage {
/* 中文逐项-变量 V4261：state
 * state（enum ncclIbCommState；成员 ncclIbCommStage）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 */
  enum ncclIbCommState state;
/* 中文逐项-变量 V4262：offset
 * offset（int；成员 ncclIbCommStage）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 */
  int offset;
/* 中文逐项-变量 V4263：buffer
 * buffer（void；成员 ncclIbCommStage）：握手暂存或数据区，须按所在状态机阶段解释其内容和长度。
 */
  void* buffer;
/* 中文逐项-变量 V4264：comm
 * comm（void；成员 ncclIbCommStage）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 */
  void* comm;
};

/* 中文逐项-类型 T0430：ncclIbHandle
 * 类型职责：非阻塞建链的阶段/暂存及可交换监听句柄，下一次调用继续先前握手。
 * 声明内可见成员（含嵌套成员）：connectAddr、stage。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbHandle {
/* 中文逐项-变量 V4265：connectAddr
 * connectAddr（union ncclSocketAddress；成员 ncclIbHandle）：非阻塞网络建链目标地址，重试仍沿用同一连接状态。
 */
  union ncclSocketAddress connectAddr; // Filled by the target
/* 中文逐项-变量 V4266：stage
 * stage（struct ncclIbCommStage；成员 ncclIbHandle）：对象状态机当前阶段或保留的未完成握手状态，重复调用时从这里继续。
 */
  struct ncclIbCommStage stage; // Used by the other side when connecting
};

/* 中文逐项-常量 X1281：NCCL_NET_IB_REQ_UNUSED
 * 编译期定义：NCCL_NET_IB_REQ_UNUSED = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_IB_REQ_UNUSED 0
/* 中文逐项-常量 X1282：NCCL_NET_IB_REQ_SEND
 * 编译期定义：NCCL_NET_IB_REQ_SEND = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_IB_REQ_SEND 1
/* 中文逐项-常量 X1283：NCCL_NET_IB_REQ_RECV
 * 编译期定义：NCCL_NET_IB_REQ_RECV = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_IB_REQ_RECV 2
/* 中文逐项-常量 X1284：NCCL_NET_IB_REQ_FLUSH
 * 编译期定义：NCCL_NET_IB_REQ_FLUSH = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NET_IB_REQ_FLUSH 3

/* 中文逐项-类型 T0431：ncclIbRequest
 * 类型职责：单个异步IB请求状态，记录类型、待完成事件数、聚合子项与接收长度。
 * 声明内可见成员（含嵌套成员）：verbs、type、events、addr、nreqs、send、size、data、lkey、offset、recv、sizes。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbRequest {
/* 中文逐项-变量 V4267：verbs
 * verbs（struct ncclIbVerbs；成员 ncclIbRequest）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 */
  struct ncclIbVerbs* verbs;
/* 中文逐项-变量 V4268：type
 * type（int；成员 ncclIbRequest）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  int type;
/* 中文逐项-变量 V4269：events
 * events（int；成员 ncclIbRequest）：本请求尚未消耗的CQ完成事件数，减到0才真正done并归还池槽。
 */
  int events;
/* 中文逐项-变量 V4270：addr
 * addr（union ncclSocketAddress；成员 ncclIbRequest）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
  union ncclSocketAddress *addr;
/* 中文逐项-变量 V4271：nreqs
 * nreqs（int；成员 ncclIbRequest）：一条后端请求聚合的子请求数，用于完成计数和长度结果解码。
 */
  int nreqs;
/* 中文逐项-类型 T0432：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：send、size、data、lkey、offset、recv、sizes。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
  union {
/* 中文逐项-变量 V4272：send
 * send（struct { int size; void* data; uint32_t lkey; int offset; }；成员
 * ncclIbRequest::<匿名>）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 */
/* 中文逐项-类型 T0433：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：send、size、data、lkey、offset。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V4273：size
 * size（int；成员 ncclIbRequest::<匿名>::<匿名>）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
      int size;
/* 中文逐项-变量 V4274：data
 * data（void；成员 ncclIbRequest::<匿名>::<匿名>）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 */
      void* data;
/* 中文逐项-变量 V4275：lkey
 * lkey（uint32_t；成员 ncclIbRequest::<匿名>::<匿名>）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
      uint32_t lkey;
/* 中文逐项-变量 V4276：offset
 * offset（int；成员 ncclIbRequest::<匿名>::<匿名>）：当前缓冲/数组范围内的位移；看指针类型和赋值公式确认字节还是元素。
 */
      int offset;
    } send;
/* 中文逐项-变量 V4277：recv
 * recv（struct { int sizes[NCCL_NET_IB_MAX_RECVS]; }；成员 ncclIbRequest::<匿名>）：按当前类型表示发送/接收方向资源或连接数组；两个方向独立保存进度。
 */
/* 中文逐项-类型 T0434：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：recv、sizes。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
    struct {
/* 中文逐项-变量 V4278：sizes
 * sizes（int；成员 ncclIbRequest::<匿名>::<匿名>）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 */
      int sizes[NCCL_NET_IB_MAX_RECVS];
    } recv;
  };
};

/* 中文导读：
 * 底层通信对象的公共前缀，保存设备索引、共享 PD、该对象的 CQ 和请求数组。
 * sendComm/recvComm 把它放在相同偏移，regMr 等公共回调因此可从不透明指针找到设备与 PD。
 */
/* 中文逐项-类型 T0435：ncclIbVerbs
 * 类型职责：后端连接的保护域引用、完成队列和固定请求池，供多个数据QP使用。
 * 声明内可见成员（含嵌套成员）：dev、pd、cq、pad、reqs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbVerbs {
/* 中文逐项-变量 V4279：dev
 * dev（int；成员 ncclIbVerbs）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 */
  int dev;
/* 中文逐项-变量 V4280：pd
 * pd（struct ibv_pd；成员 ncclIbVerbs）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
  struct ibv_pd* pd; // duplicate of ncclIbDevs[dev].pd
/* 中文逐项-变量 V4281：cq
 * cq（struct ibv_cq；成员 ncclIbVerbs）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
  struct ibv_cq* cq;
/* 中文逐项-变量 V4282：pad
 * pad（uint64_t；成员 ncclIbVerbs）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
  uint64_t pad[1];
/* 中文逐项-变量 V4283：reqs
 * reqs（struct ncclIbRequest；成员 ncclIbVerbs）：在途网络请求或固定请求池；只有 Test 完成后其槽才能重用。
 */
  struct ncclIbRequest reqs[MAX_REQUESTS];
};

/* 中文逐项-类型 T0436：ncclIbListenComm
 * 类型职责：IB后端控制监听对象，Accept从这里建立独立接收连接。
 * 声明内可见成员（含嵌套成员）：dev、sock、stage。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbListenComm {
/* 中文逐项-变量 V4284：dev
 * dev（int；成员 ncclIbListenComm）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 */
  int dev;
/* 中文逐项-变量 V4285：sock
 * sock（struct ncclSocket；成员 ncclIbListenComm）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket sock;
/* 中文逐项-变量 V4286：stage
 * stage（struct ncclIbCommStage；成员 ncclIbListenComm）：对象状态机当前阶段或保留的未完成握手状态，重复调用时从这里继续。
 */
  struct ncclIbCommStage stage;
};

/* 中文逐项-类型 T0437：ncclIbSendFifo
 * 类型职责：接收方授予发送方的目标地址/rkey/尺寸/tag/序号条目，载荷按此授权写入。
 * 声明内可见成员（含嵌套成员）：addr、size、rkey、nreqs、tag、idx。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbSendFifo {
/* 中文逐项-变量 V4287：addr
 * addr（uint64_t；成员 ncclIbSendFifo）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
  uint64_t addr;
/* 中文逐项-变量 V4288：size
 * size（int；成员 ncclIbSendFifo）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
  int      size;
/* 中文逐项-变量 V4289：rkey
 * rkey（uint32_t；成员 ncclIbSendFifo）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
  uint32_t rkey;
/* 中文逐项-变量 V4290：nreqs
 * nreqs（uint32_t；成员 ncclIbSendFifo）：一条后端请求聚合的子请求数，用于完成计数和长度结果解码。
 */
  uint32_t nreqs;
/* 中文逐项-变量 V4291：tag
 * tag（uint32_t；成员 ncclIbSendFifo）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 */
  uint32_t tag;
/* 中文逐项-变量 V4292：idx
 * idx（uint64_t；成员 ncclIbSendFifo）：网络控制FIFO条目的轮次序号，避免环形槽复用时把旧地址授权当作新请求。
 */
  uint64_t idx;
};

/* 中文导读：
 * 一个底层发送连接对象：持有 nqps 个 RC 数据 QP、对端接收地址发布 FIFO 及其本地 MR。
 * 高层多个逻辑连接能否共用此对象由 transport/net.cc 决定；不能把一个 SendComm 当成一个 communicator。
 */
/* 中文逐项-类型 T0438：ncclIbSendComm
 * 类型职责：IB后端真正发送/接收连接，持有控制socket、数据QP、FIFO和verbs请求资源。
 * 声明内可见成员（含嵌套成员）：verbs、fifo、fifoHead、fifoReqs、wrs、sges、sock、ready、qps、nqps、fifoMr。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbSendComm {
/* 中文逐项-变量 V4293：verbs
 * verbs（struct ncclIbVerbs；成员 ncclIbSendComm）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 */
  struct ncclIbVerbs verbs;
/* 中文逐项-变量 V4294：fifo
 * fifo（struct ncclIbSendFifo；成员 ncclIbSendComm）：网络控制队列及读写序号，发布/消费接收地址和tag；与GPU协议head/tail区分。
 */
  struct ncclIbSendFifo fifo[MAX_REQUESTS][NCCL_NET_IB_MAX_RECVS];
/* 中文逐项-变量 V4295：fifoHead
 * fifoHead（uint64_t；成员 ncclIbSendComm）：网络控制队列及读写序号，发布/消费接收地址和tag；与GPU协议head/tail区分。
 */
  uint64_t fifoHead;
/* 中文逐项-变量 V4296：fifoReqs
 * fifoReqs（struct ncclIbRequest；成员 ncclIbSendComm）：等待远端接收地址匹配的发送请求表，按FIFO槽聚合后交给MultiSend。
 */
  struct ncclIbRequest* fifoReqs[MAX_REQUESTS][NCCL_NET_IB_MAX_RECVS];
/* 中文逐项-变量 V4297：wrs
 * wrs（struct ibv_send_wr；成员 ncclIbSendComm）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 */
  struct ibv_send_wr wrs[NCCL_NET_IB_MAX_RECVS+1];
/* 中文逐项-变量 V4298：sges
 * sges（struct ibv_sge；成员 ncclIbSendComm）：scatter/gather 地址段列表或单段，指定本地地址、长度和lkey。
 */
  struct ibv_sge sges[NCCL_NET_IB_MAX_RECVS];
/* 中文逐项-变量 V4299：sock
 * sock（struct ncclSocket；成员 ncclIbSendComm）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket sock;

/* 中文逐项-变量 V4300：ready
 * ready（int；成员 ncclIbSendComm）：握手/连接是否完整就绪的状态，未就绪时须继续推进控制握手。
 */
  int ready;
/* 中文逐项-变量 V4301：qps
 * qps（struct ibv_qp；成员 ncclIbSendComm）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 */
  struct ibv_qp* qps[NCCL_IB_MAX_QPS];
/* 中文逐项-变量 V4302：nqps
 * nqps（int；成员 ncclIbSendComm）：此后端连接创建的数据 QP 数，受 IB_QPS_PER_CONNECTION 影响。
 */
  int nqps;
/* 中文逐项-变量 V4303：fifoMr
 * fifoMr（struct ibv_mr；成员 ncclIbSendComm）：控制FIFO或flush主机小缓冲的真实MR，和大协议数据缓冲MR独立。
 */
  struct ibv_mr* fifoMr;
};
// The SendFifo needs to be 32-byte aligned and each element needs
// to be a 32-byte multiple, so that an entry does not get split and
// written out of order when IB Relaxed Ordering is enabled
static_assert((offsetof(struct ncclIbSendComm, fifo) % 32) == 0, "ncclIbSendComm fifo must be 32-byte aligned");
static_assert((sizeof(struct ncclIbSendFifo) % 32) == 0, "ncclIbSendFifo element size must be 32-byte multiples");

/* 中文逐项-类型 T0439：ncclIbGpuFlush
 * 类型职责：GDR接收的可见性刷新上下文，启用时使用自连接读和小主机注册缓冲。
 * 声明内可见成员（含嵌套成员）：enabled、hostMem、hostMr、sge、qp。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbGpuFlush {
/* 中文逐项-变量 V4304：enabled
 * enabled（int；成员 ncclIbGpuFlush）：当前可选功能/资源已建立可用的标志，例如GDR flush，使用前须检查。
 */
  int enabled;
/* 中文逐项-变量 V4305：hostMem
 * hostMem（int；成员 ncclIbGpuFlush）：当前方向拥有的主机或设备分配，地址空间由字段名及类型共同确认。
 */
  int hostMem;
/* 中文逐项-变量 V4306：hostMr
 * hostMr（struct ibv_mr；成员 ncclIbGpuFlush）：控制FIFO或flush主机小缓冲的真实MR，和大协议数据缓冲MR独立。
 */
  struct ibv_mr* hostMr;
/* 中文逐项-变量 V4307：sge
 * sge（struct ibv_sge；成员 ncclIbGpuFlush）：scatter/gather 地址段列表或单段，指定本地地址、长度和lkey。
 */
  struct ibv_sge sge;
/* 中文逐项-变量 V4308：qp
 * qp（struct ibv_qp；成员 ncclIbGpuFlush）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 */
  struct ibv_qp* qp;
};

/* 中文逐项-类型 T0440：ncclIbRemFifo
 * 类型职责：接收方持有的远端发送FIFO地址/key与本地待发布条目暂存。
 * 声明内可见成员（含嵌套成员）：elems、fifoTail、addr、rkey、flags、mr、sge。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbRemFifo {
/* 中文逐项-变量 V4309：elems
 * elems（struct ncclIbSendFifo；成员 ncclIbRemFifo）：固定大小 work union 的不同元素布局，必须先看 header.type 再解释内存。
 */
  struct ncclIbSendFifo elems[MAX_REQUESTS][NCCL_NET_IB_MAX_RECVS];
/* 中文逐项-变量 V4310：fifoTail
 * fifoTail（uint64_t；成员 ncclIbRemFifo）：网络控制队列及读写序号，发布/消费接收地址和tag；与GPU协议head/tail区分。
 */
  uint64_t fifoTail;
/* 中文逐项-变量 V4311：addr
 * addr（uint64_t；成员 ncclIbRemFifo）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
  uint64_t addr;
/* 中文逐项-变量 V4312：rkey
 * rkey（uint32_t；成员 ncclIbRemFifo）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
  uint32_t rkey;
/* 中文逐项-变量 V4313：flags
 * flags（uint32_t；成员 ncclIbRemFifo）：按位组合的当前线程角色/直接访问/共享偏移/中止等状态，按位测试而非单一枚举。
 */
  uint32_t flags;
/* 中文逐项-变量 V4314：mr
 * mr（struct ibv_mr；成员 ncclIbRemFifo）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 */
  struct ibv_mr* mr;
/* 中文逐项-变量 V4315：sge
 * sge（struct ibv_sge；成员 ncclIbRemFifo）：scatter/gather 地址段列表或单段，指定本地地址、长度和lkey。
 */
  struct ibv_sge sge;
};

/* 中文导读：
 * 一个底层接收连接对象：数据 QP、向发送方发布接收地址的 remFifo，以及可选 GDR flush 资源。
 * 用于写 payload 的 rkey 来自接收缓冲区 MR；RDMA_WRITE_WITH_IMM 的完成通知仍需要预挂接收 WQE。
 */
/* 中文逐项-类型 T0441：ncclIbRecvComm
 * 类型职责：IB后端真正发送/接收连接，持有控制socket、数据QP、FIFO和verbs请求资源。
 * 声明内可见成员（含嵌套成员）：verbs、remFifo、sock、ready、qps、nqps、gpuFlush。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclIbRecvComm {
/* 中文逐项-变量 V4316：verbs
 * verbs（struct ncclIbVerbs；成员 ncclIbRecvComm）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 */
  struct ncclIbVerbs verbs;
/* 中文逐项-变量 V4317：remFifo
 * remFifo（struct ncclIbRemFifo；成员 ncclIbRecvComm）：接收端持有的远端发送控制FIFO地址/key及本地发布暂存区。
 */
  struct ncclIbRemFifo remFifo;
/* 中文逐项-变量 V4318：sock
 * sock（struct ncclSocket；成员 ncclIbRecvComm）：当前socket对象、监听socket或后端控制socket，与数据QP作用不同。
 */
  struct ncclSocket sock;
/* 中文逐项-变量 V4319：ready
 * ready（int；成员 ncclIbRecvComm）：握手/连接是否完整就绪的状态，未就绪时须继续推进控制握手。
 */
  int ready;
/* 中文逐项-变量 V4320：qps
 * qps（struct ibv_qp；成员 ncclIbRecvComm）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 */
  struct ibv_qp* qps[NCCL_IB_MAX_QPS];
/* 中文逐项-变量 V4321：nqps
 * nqps（int；成员 ncclIbRecvComm）：此后端连接创建的数据 QP 数，受 IB_QPS_PER_CONNECTION 影响。
 */
  int nqps;
/* 中文逐项-变量 V4322：gpuFlush
 * gpuFlush（struct ncclIbGpuFlush；成员 ncclIbRecvComm）：接收GDR可见性刷新对象，满足条件时包含自连接QP和小主机MR。
 */
  struct ncclIbGpuFlush gpuFlush;
};
static_assert((offsetof(struct ncclIbRecvComm, remFifo) % 32) == 0, "ncclIbSendComm fifo must be 32-byte aligned");

/* 中文逐项-参数getter P0050：IbQpsPerConn
 * 生成函数 ncclParamIbQpsPerConn()，读取 NCCL_IB_QPS_PER_CONNECTION，编译默认值为 1。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbQpsPerConn, "IB_QPS_PER_CONNECTION", 1);

/* 中文导读：
 * 为通信对象取得 IB 设备的引用计数共享 PD，并创建自己的 CQ。
 * 共享 PD 允许同设备/同进程内的不同 QP 使用合适的已注册 MR；共享 PD 不代表 CQ 或 QP 也全局只有一个。
 */
/* 中文逐项-函数 F1107：ncclIbInitVerbs
 * 用途与位置：为一个 backend 连接准备共享设备 PD 引用、CQ 与请求池，供其多个数据 QP 使用。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 ctx（struct ibv_context* ctx）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 verbs（struct ncclIbVerbs* verbs）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbConnect；src/transport/net_ib.cc::ncclIbAccept。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCLCHECKGOTO；ncclParamIbQpsPerConn；pthread_mutex_lock；pthread_mutex_unlock；wrap_ibv_alloc_pd；wrap_ibv_create_cq。
 * 需要对照的边界/推进条件：0 == ncclIbDevs[dev].pdRefs++；0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return res;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclIbInitVerbs(int dev, struct ibv_context* ctx, struct ncclIbVerbs* verbs) {
  verbs->dev = dev;

  pthread_mutex_lock(&ncclIbDevs[dev].lock);
  if (0 == ncclIbDevs[dev].pdRefs++) {
/* 中文逐项-变量 V4323：res
 * res（ncclResult_t；函数 ncclIbInitVerbs 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_ibv_alloc_pd(&ncclIbDevs[dev].pd, ctx), res,
 * failure);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    ncclResult_t res;
    NCCLCHECKGOTO(wrap_ibv_alloc_pd(&ncclIbDevs[dev].pd, ctx), res, failure);
    if (0) {
    failure:
      pthread_mutex_unlock(&ncclIbDevs[dev].lock);
      return res;
    }
  }
  verbs->pd = ncclIbDevs[dev].pd;
  pthread_mutex_unlock(&ncclIbDevs[dev].lock);

  // Recv requests can generate 2 completions (one for the post FIFO, one for the Recv).
  NCCLCHECK(wrap_ibv_create_cq(&verbs->cq, ctx, 2*MAX_REQUESTS*ncclParamIbQpsPerConn(), NULL, NULL, 0));
  return ncclSuccess;
}

/* 中文逐项-函数 F1108：ncclIbDestroyVerbs
 * 用途与位置：销毁连接 CQ 等私有对象，并按引用计数释放设备共享的保护域。
 * 参数 verbs（struct ncclIbVerbs* verbs）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbCloseSend；src/transport/net_ib.cc::ncclIbCloseRecv。
 * 本体直接出现的调用/包装：NCCLCHECK；NCCLCHECKGOTO；pthread_mutex_lock；pthread_mutex_unlock；wrap_ibv_dealloc_pd；wrap_ibv_destroy_cq。
 * 需要对照的边界/推进条件：0 == --ncclIbDevs[verbs->dev].pdRefs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return res;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclIbDestroyVerbs(struct ncclIbVerbs* verbs) {
/* 中文逐项-变量 V4324：res
 * res（ncclResult_t；函数 ncclIbDestroyVerbs 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_ibv_dealloc_pd(ncclIbDevs[verbs->dev].pd), res,
 * returning);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t res;
  NCCLCHECK(wrap_ibv_destroy_cq(verbs->cq));

  pthread_mutex_lock(&ncclIbDevs[verbs->dev].lock);
  if (0 == --ncclIbDevs[verbs->dev].pdRefs) {
    NCCLCHECKGOTO(wrap_ibv_dealloc_pd(ncclIbDevs[verbs->dev].pd), res, returning);
  }
  res = ncclSuccess;
returning:
  pthread_mutex_unlock(&ncclIbDevs[verbs->dev].lock);
  return res;
}

/* 中文导读：
 * 创建 RC QP，配置 SQ/RQ 深度及单个 SGE，然后转到 INIT。
 * qpInitAttr 清零后没有绑定 srq，因此本内置后端走每 QP 的接收队列；后续 Irecv 调用 post_recv。
 * 改变为 SRQ 只会改变接收队列组织，不会自然消除各 peer 的 RC QP 或 payload MR。
 */
/* 中文逐项-函数 F1109：ncclIbCreateQp
 * 用途与位置：创建绑定本连接 CQ/PD 的 RC QP 并设到 INIT，配置队列容量和访问权限。
 * 参数 ib_port（uint8_t ib_port）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 参数 verbs（struct ncclIbVerbs* verbs）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 * 参数 access_flags（int access_flags）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 qp（struct ibv_qp** qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbConnect；src/transport/net_ib.cc::ncclIbAccept。
 * 本体直接出现的调用/包装：NCCLCHECK；memset；ncclParamIbPkey；ncclParamIbUseInline；wrap_ibv_create_qp；wrap_ibv_modify_qp。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbCreateQp(uint8_t ib_port, struct ncclIbVerbs* verbs, int access_flags, struct ibv_qp** qp) {
/* 中文逐项-变量 V4325：qpInitAttr
 * qpInitAttr（struct ibv_qp_init_attr；函数 ncclIbCreateQp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(&qpInitAttr, 0, sizeof(struct ibv_qp_init_attr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_qp_init_attr qpInitAttr;
  memset(&qpInitAttr, 0, sizeof(struct ibv_qp_init_attr));
  qpInitAttr.send_cq = verbs->cq;
  qpInitAttr.recv_cq = verbs->cq;
  qpInitAttr.qp_type = IBV_QPT_RC;
  // We might send 2 messages per send (RDMA and RDMA_WITH_IMM)
  qpInitAttr.cap.max_send_wr = 2*MAX_REQUESTS;
  qpInitAttr.cap.max_recv_wr = MAX_REQUESTS;
  qpInitAttr.cap.max_send_sge = 1;
  qpInitAttr.cap.max_recv_sge = 1;
  qpInitAttr.cap.max_inline_data = ncclParamIbUseInline() ? sizeof(struct ncclIbSendFifo) : 0;
  NCCLCHECK(wrap_ibv_create_qp(qp, verbs->pd, &qpInitAttr));
/* 中文逐项-变量 V4326：qpAttr
 * qpAttr（struct ibv_qp_attr；函数 ncclIbCreateQp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_qp_attr qpAttr;
  memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));
  qpAttr.qp_state = IBV_QPS_INIT;
  qpAttr.pkey_index = ncclParamIbPkey();
  qpAttr.port_num = ib_port;
  qpAttr.qp_access_flags = access_flags;
  NCCLCHECK(wrap_ibv_modify_qp(*qp, &qpAttr, IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS));
  return ncclSuccess;
}

/* 中文导读：
 * 把 QP 转为 RTR，装入对端 QPN、路由地址、端口与协商 MTU 等信息。
 * IB 与 RoCE 的地址字段处理不同；到达 RTR 后仍需 RTS 才能按本实现正常推进发送。
 */
/* 中文逐项-函数 F1110：ncclIbRtrQp
 * 用途与位置：根据远端 QPN/LID/GID 等握手参数把 QP 切到 RTR，使接收方向就绪。
 * 参数 qp（struct ibv_qp* qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 参数 qpn（uint32_t qpn）：本地/远端QP编号，用于控制握手及连接目标匹配。
 * 参数 info（struct ncclIbQpInfo* info）：当前函数处理的任务/属性描述对象，按ncclInfo、PeerInfo、QP信息或XML类型区分其所属阶段。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbAccept；src/transport/net_ib.cc::ncclSendCheck。
 * 本体直接出现的调用/包装：NCCLCHECK；memset；ncclParamIbGidIndex；ncclParamIbSl；ncclParamIbTc；wrap_ibv_modify_qp。
 * 需要对照的边界/推进条件：info->link_layer == IBV_LINK_LAYER_ETHERNET。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbRtrQp(struct ibv_qp* qp, uint32_t qpn, struct ncclIbQpInfo* info) {
/* 中文逐项-变量 V4327：qpAttr
 * qpAttr（struct ibv_qp_attr；函数 ncclIbRtrQp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_qp_attr qpAttr;
  memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));
  qpAttr.qp_state = IBV_QPS_RTR;
  qpAttr.path_mtu = info->mtu;
  qpAttr.dest_qp_num = qpn;
  qpAttr.rq_psn = 0;
  qpAttr.max_dest_rd_atomic = 1;
  qpAttr.min_rnr_timer = 12;
  if (info->link_layer == IBV_LINK_LAYER_ETHERNET) {
    qpAttr.ah_attr.is_global = 1;
    qpAttr.ah_attr.grh.dgid.global.subnet_prefix = info->spn;
    qpAttr.ah_attr.grh.dgid.global.interface_id = info->iid;
    qpAttr.ah_attr.grh.flow_label = 0;
    qpAttr.ah_attr.grh.sgid_index = ncclParamIbGidIndex();
    qpAttr.ah_attr.grh.hop_limit = 255;
    qpAttr.ah_attr.grh.traffic_class = ncclParamIbTc();
  } else {
    qpAttr.ah_attr.is_global = 0;
    qpAttr.ah_attr.dlid = info->lid;
  }
  qpAttr.ah_attr.sl = ncclParamIbSl();
  qpAttr.ah_attr.src_path_bits = 0;
  qpAttr.ah_attr.port_num = info->ib_port;
  NCCLCHECK(wrap_ibv_modify_qp(qp, &qpAttr, IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN | IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER));
  return ncclSuccess;
}

/* 中文逐项-函数 F1111：ncclIbRtsQp
 * 用途与位置：配置超时、重试和发送序号等，把 QP 切到 RTS 允许发送工作请求。
 * 参数 qp（struct ibv_qp* qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbAccept；src/transport/net_ib.cc::ncclSendCheck。
 * 本体直接出现的调用/包装：NCCLCHECK；memset；ncclParamIbRetryCnt；ncclParamIbTimeout；wrap_ibv_modify_qp。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbRtsQp(struct ibv_qp* qp) {
/* 中文逐项-变量 V4328：qpAttr
 * qpAttr（struct ibv_qp_attr；函数 ncclIbRtsQp 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_qp_attr qpAttr;
  memset(&qpAttr, 0, sizeof(struct ibv_qp_attr));
  qpAttr.qp_state = IBV_QPS_RTS;
  qpAttr.timeout = ncclParamIbTimeout();
  qpAttr.retry_cnt = ncclParamIbRetryCnt();
  qpAttr.rnr_retry = 7;
  qpAttr.sq_psn = 0;
  qpAttr.max_rd_atomic = 1;
  NCCLCHECK(wrap_ibv_modify_qp(qp, &qpAttr, IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC));
  return ncclSuccess;
}

/* 中文逐项-函数 F1112：ncclIbListen
 * 用途与位置：建立接收方向的控制监听对象，将设备/地址编码到可交换句柄。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 opaqueHandle（void* opaqueHandle）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 listenComm（void** listenComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：GetSocketAddr；NCCLCHECK；memcpy；memset；ncclCalloc；ncclSocketListen。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbListen(int dev, void* opaqueHandle, void** listenComm) {
/* 中文逐项-变量 V4329：comm
 * comm（struct ncclIbListenComm；函数 ncclIbListen 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclCalloc(&comm, 1));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclIbListenComm* comm;
  NCCLCHECK(ncclCalloc(&comm, 1));
/* 中文逐项-变量 V4330：handle
 * handle（struct ncclIbHandle；函数 ncclIbListen 内）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 初值/来源 handle = (struct ncclIbHandle*) opaqueHandle；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbHandle* handle = (struct ncclIbHandle*) opaqueHandle;
  static_assert(sizeof(struct ncclIbHandle) < NCCL_NET_HANDLE_MAXSIZE, "ncclIbHandle size too large");
  memset(handle, 0, sizeof(struct ncclIbHandle));
  comm->dev = dev;
  NCCLCHECK(GetSocketAddr(&comm->sock.addr));
  NCCLCHECK(ncclSocketListen(&comm->sock));
  memcpy(&handle->connectAddr, &comm->sock.addr, sizeof(union ncclSocketAddress));
  *listenComm = comm;
  return ncclSuccess;
}

/* 中文导读：
 * 发起底层发送连接：socket 握手交换 QP 信息，建立 nqps 个数据 QP，并注册发送方接收控制 FIFO。
 * 连接是分阶段非阻塞推进的，成功返回但 sendComm 仍为空表示尚未完成，调用方必须重试。
 */
/* 中文逐项-函数 F1113：ncclIbConnect
 * 用途与位置：非阻塞推进发送端控制握手，创建数据 QP 和控制 FIFO MR，并交换远端连接信息。
 * 参数 dev（int dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 参数 opaqueHandle（void* opaqueHandle）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 sendComm（void** sendComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；free；memcpy；ncclGetSocketState；ncclIbCreateQp；ncclIbInitVerbs；ncclIbMalloc；ncclParamIbGidIndex；ncclParamIbQpsPerConn；ncclSocketConnect；ncclSocketInit；ncclSocketProgress；wrap_ibv_query_gid；wrap_ibv_query_port；wrap_ibv_reg_mr。
 * 需要对照的边界/推进条件：stage->state == ncclIbCommStateConnect；stage->state == ncclIbCommStateSend；stage->state !=
 * ncclIbCommStateStart；conState == ncclSocketConnecting。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;；return ncclSystemError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbConnect(int dev, void* opaqueHandle, void** sendComm) {
/* 中文逐项-变量 V4331：handle
 * handle（struct ncclIbHandle；函数 ncclIbConnect 内）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 * 初值/来源 handle = (struct ncclIbHandle*) opaqueHandle；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbHandle* handle = (struct ncclIbHandle*) opaqueHandle;
/* 中文逐项-变量 V4332：conState
 * conState（enum ncclSocketState；函数 ncclIbConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclGetSocketState(&comm->sock, &conState));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  enum ncclSocketState conState;
/* 中文逐项-变量 V4333：stage
 * stage（struct ncclIbCommStage；函数 ncclIbConnect 内）：对象状态机当前阶段或保留的未完成握手状态，重复调用时从这里继续。
 * 初值/来源 stage = &handle->stage；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclIbCommStage* stage = &handle->stage;
/* 中文逐项-变量 V4334：comm
 * comm（struct ncclIbSendComm；函数 ncclIbConnect 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclIbSendComm*)stage->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclIbSendComm* comm = (struct ncclIbSendComm*)stage->comm;
  *sendComm = NULL;

  if (stage->state == ncclIbCommStateConnect) goto ib_connect_check;
  if (stage->state == ncclIbCommStateSend) goto ib_send;
  if (stage->state != ncclIbCommStateStart) {
    WARN("Error: trying to connect already connected sendComm");
    return ncclInternalError;
  }

  NCCLCHECK(ncclIbMalloc((void**)&comm, sizeof(struct ncclIbSendComm)));
  NCCLCHECK(ncclSocketInit(&comm->sock, &handle->connectAddr, NULL, 1));
  stage->comm = comm;
  stage->state = ncclIbCommStateConnect;
  NCCLCHECK(ncclSocketConnect(&comm->sock));

ib_connect_check:
  /* since ncclSocketConnect is async, we must check if connection is complete */
  NCCLCHECK(ncclGetSocketState(&comm->sock, &conState));
  if (conState == ncclSocketConnecting) {
    /* expect user to call again */
    return ncclSuccess;
  } else if (conState == ncclSocketError) {
    return ncclSystemError;
  }

  // IB Setup
/* 中文逐项-变量 V4335：ctx
 * ctx（struct ibv_context；函数 ncclIbConnect 内）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 此处未显式给初值；后续首个文本使用点：ctx = ncclIbDevs[dev].context;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_context* ctx;
  ctx = ncclIbDevs[dev].context;
  NCCLCHECK(ncclIbInitVerbs(dev, ctx, &comm->verbs));
/* 中文逐项-变量 V4336：ib_port
 * ib_port（uint8_t；函数 ncclIbConnect 内）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 此处未显式给初值；后续首个文本使用点：ib_port = ncclIbDevs[dev].port;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  uint8_t ib_port;
  ib_port = ncclIbDevs[dev].port;
  comm->nqps = ncclParamIbQpsPerConn();
  for (/* 中文逐项-变量 V4337：q
 * q（int；函数 ncclIbConnect 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++) {
    NCCLCHECK(ncclIbCreateQp(ib_port, &comm->verbs, IBV_ACCESS_REMOTE_WRITE, comm->qps+q));
  }

  // Send my QP Info to receiver through the socket. Hope this won't block.
/* 中文逐项-变量 V4338：portAttr
 * portAttr（struct ibv_port_attr；函数 ncclIbConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_query_port(ctx, ib_port, &portAttr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_port_attr portAttr;
  NCCLCHECK(wrap_ibv_query_port(ctx, ib_port, &portAttr));
/* 中文逐项-变量 V4339：qpInfo
 * qpInfo（struct ncclIbQpInfo；函数 ncclIbConnect 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：qpInfo.ib_port = ib_port;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclIbQpInfo qpInfo;
  qpInfo.ib_port = ib_port;
  for (/* 中文逐项-变量 V4340：q
 * q（int；函数 ncclIbConnect 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++) qpInfo.qpn[q] = comm->qps[q]->qp_num;
  qpInfo.mtu = portAttr.active_mtu;

  // Prepare my fifo
  NCCLCHECK(wrap_ibv_reg_mr(&comm->fifoMr, comm->verbs.pd, comm->fifo, sizeof(struct ncclIbSendFifo)*MAX_REQUESTS*NCCL_NET_IB_MAX_RECVS, IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ));
  qpInfo.fifoRkey = comm->fifoMr->rkey;
  qpInfo.fifoAddr = (uint64_t)comm->fifo;

  // RoCE support
  qpInfo.lid = portAttr.lid;
  qpInfo.link_layer = portAttr.link_layer;
  if (qpInfo.link_layer == IBV_LINK_LAYER_INFINIBAND) { // IB
    for (/* 中文逐项-变量 V4341：q
 * q（int；函数 ncclIbConnect 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++)
      INFO(NCCL_NET,"NET/IB: Dev %d Port %d qpn %d mtu %d LID %d", dev, ib_port, qpInfo.qpn[q], qpInfo.mtu, qpInfo.lid);
  } else { // RoCE
/* 中文逐项-变量 V4342：gid
 * gid（union ibv_gid；函数 ncclIbConnect 内）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_query_gid(ctx, ib_port, ncclParamIbGidIndex(),
 * &gid));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    union ibv_gid gid;
    NCCLCHECK(wrap_ibv_query_gid(ctx, ib_port, ncclParamIbGidIndex(), &gid));
    qpInfo.spn = gid.global.subnet_prefix;
    qpInfo.iid = gid.global.interface_id;
    for (/* 中文逐项-变量 V4343：q
 * q（int；函数 ncclIbConnect 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++)
      INFO(NCCL_NET,"NET/IB: Dev %d Port %d qpn %d mtu %d GID %ld (%lX/%lX)", dev, ib_port, qpInfo.qpn[q], qpInfo.mtu, ncclParamIbGidIndex(), qpInfo.spn, qpInfo.iid);
  }

  stage->state = ncclIbCommStateSend;
  stage->offset = 0;
  NCCLCHECK(ncclIbMalloc((void**)&stage->buffer, sizeof(qpInfo)));
  memcpy(stage->buffer, &qpInfo, sizeof(qpInfo));

ib_send:
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, &comm->sock, stage->buffer, sizeof(qpInfo), &stage->offset));
  if (stage->offset != sizeof(qpInfo))
    return ncclSuccess;

  free(stage->buffer);
  stage->state = ncclIbCommStateConnected;
  *sendComm = comm;
  return ncclSuccess;
}

/* 中文逐项-参数getter P0051：IbGdrFlushDisable
 * 生成函数 ncclParamIbGdrFlushDisable()，读取 NCCL_GDR_FLUSH_DISABLE，编译默认值为 0。
 * 首次调用解析并缓存，后续读取静态缓存；应在初始化/首次调用前设置环境，不保证进程中途修改后重新读取。
 * 这个宏生成getter，不会仅因声明参数就在这里分配QP、MR或数据缓冲；作用取决于getter的调用位置。
 */
NCCL_PARAM(IbGdrFlushDisable, "GDR_FLUSH_DISABLE", 0);

/* 中文导读：
 * 接收侧分阶段建链并配对数据 QP，注册 remFifo；满足条件时再创建一个 flush QP 和 hostMr。
 * 这些控制/flush MR 直接调用 verbs 注册，不进入下方 ncclIbRegMr 的协议缓冲缓存。
 */
/* 中文逐项-函数 F1114：ncclIbAccept
 * 用途与位置：非阻塞推进接收端握手，建立数据 QP、控制区及满足条件时的 GDR flush 资源。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 参数 recvComm（void** recvComm）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；free；memcpy；ncclIbCreateQp；ncclIbGdrSupport；ncclIbInitVerbs；ncclIbMalloc；ncclIbRtrQp；ncclIbRtsQp；ncclParamIbGdrFlushDisable；ncclParamIbGidIndex；ncclParamIbQpsPerConn；ncclParamIbUseInline；ncclSocketAccept；ncclSocketProgress；std::min；wrap_ibv_query_gid；wrap_ibv_query_port；更多调用保留在函数体。
 * 需要对照的边界/推进条件：stage->state == ncclIbCommStateAccept；stage->state == ncclIbCommStateRecv；stage->state ==
 * ncclIbCommStateSend；stage->state != ncclIbCommStateStart。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbAccept(void* listenComm, void** recvComm) {
/* 中文逐项-变量 V4344：lComm
 * lComm（struct ncclIbListenComm；函数 ncclIbAccept 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 lComm = (struct ncclIbListenComm*)listenComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbListenComm* lComm = (struct ncclIbListenComm*)listenComm;
/* 中文逐项-变量 V4345：stage
 * stage（struct ncclIbCommStage；函数 ncclIbAccept 内）：对象状态机当前阶段或保留的未完成握手状态，重复调用时从这里继续。
 * 初值/来源 stage = &lComm->stage；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclIbCommStage* stage = &lComm->stage;
/* 中文逐项-变量 V4346：rComm
 * rComm（struct ncclIbRecvComm；函数 ncclIbAccept 内）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 初值/来源 rComm = (struct ncclIbRecvComm*)stage->comm；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclIbRecvComm* rComm = (struct ncclIbRecvComm*)stage->comm;
  *recvComm = NULL;

  if (stage->state == ncclIbCommStateAccept) goto ib_accept;
  if (stage->state == ncclIbCommStateRecv) goto ib_recv;
  if (stage->state == ncclIbCommStateSend) goto ib_send;
  if (stage->state != ncclIbCommStateStart) {
    WARN("Listencomm in unknown state %d\n", stage->state);
    return ncclInternalError;
  }

  NCCLCHECK(ncclIbMalloc((void**)&rComm, sizeof(struct ncclIbRecvComm)));
  stage->comm = rComm;
  stage->state = ncclIbCommStateAccept;
  lComm->sock.asyncFlag = 1;
  rComm->sock.asyncFlag = 1;

ib_accept:
  NCCLCHECK(ncclSocketAccept(&rComm->sock, &lComm->sock));
  if (rComm->sock.fd == -1)
    return ncclSuccess;

/* 中文逐项-变量 V4347：remQpInfo
 * remQpInfo（struct ncclIbQpInfo；函数 ncclIbAccept 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclIbMalloc((void**)&stage->buffer, sizeof(remQpInfo)));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclIbQpInfo remQpInfo;
  stage->state = ncclIbCommStateRecv;
  stage->offset = 0;
  NCCLCHECK(ncclIbMalloc((void**)&stage->buffer, sizeof(remQpInfo)));
ib_recv:
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &rComm->sock, stage->buffer, sizeof(remQpInfo), &stage->offset));
  if (stage->offset != sizeof(remQpInfo))
    return ncclSuccess;

  /* copy back the received info */
  memcpy(&remQpInfo, stage->buffer, sizeof(struct ncclIbQpInfo));

  // IB setup
/* 中文逐项-变量 V4348：ctx
 * ctx（struct ibv_context；函数 ncclIbAccept 内）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 此处未显式给初值；后续首个文本使用点：ctx = ncclIbDevs[lComm->dev].context;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_context* ctx;
/* 中文逐项-变量 V4349：ib_port
 * ib_port（uint8_t；函数 ncclIbAccept 内）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 此处未显式给初值；后续首个文本使用点：ib_port = ncclIbDevs[lComm->dev].port;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  uint8_t ib_port;
  ctx = ncclIbDevs[lComm->dev].context;
  ib_port = ncclIbDevs[lComm->dev].port;
/* 中文逐项-变量 V4350：portAttr
 * portAttr（struct ibv_port_attr；函数 ncclIbAccept 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_query_port(ctx, ib_port, &portAttr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_port_attr portAttr;
  NCCLCHECK(wrap_ibv_query_port(ctx, ib_port, &portAttr));
/* 中文逐项-变量 V4351：gid
 * gid（union ibv_gid；函数 ncclIbAccept 内）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_query_gid(ctx, ib_port, ncclParamIbGidIndex(),
 * &gid));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  union ibv_gid gid;
  NCCLCHECK(wrap_ibv_query_gid(ctx, ib_port, ncclParamIbGidIndex(), &gid));

  // QP Creation
  NCCLCHECK(ncclIbInitVerbs(lComm->dev, ctx, &rComm->verbs));
  rComm->nqps = ncclParamIbQpsPerConn();
  for (/* 中文逐项-变量 V4352：q
 * q（int；函数 ncclIbAccept 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<rComm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<rComm->nqps; q++) {
    NCCLCHECK(ncclIbCreateQp(ib_port, &rComm->verbs, IBV_ACCESS_REMOTE_WRITE, rComm->qps+q));
  }

  // Adjust the MTU
  remQpInfo.mtu = (enum ibv_mtu)std::min(remQpInfo.mtu, portAttr.active_mtu);

  // Setup QP
  for (/* 中文逐项-变量 V4353：q
 * q（int；函数 ncclIbAccept 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<rComm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<rComm->nqps; q++) {
/* 中文逐项-变量 V4354：qp
 * qp（struct ibv_qp；函数 ncclIbAccept 内）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 初值/来源 qp = rComm->qps[q]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ibv_qp* qp = rComm->qps[q];
    NCCLCHECK(ncclIbRtrQp(qp, remQpInfo.qpn[q], &remQpInfo));
    NCCLCHECK(ncclIbRtsQp(qp));
  }

  // Retain remote fifo info and prepare my RDMA ops
  rComm->remFifo.rkey = remQpInfo.fifoRkey;
  rComm->remFifo.addr = remQpInfo.fifoAddr;
  NCCLCHECK(wrap_ibv_reg_mr(&rComm->remFifo.mr, rComm->verbs.pd, &rComm->remFifo.elems, sizeof(struct ncclIbSendFifo)*MAX_REQUESTS*NCCL_NET_IB_MAX_RECVS, IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_READ));
  rComm->remFifo.sge.lkey = rComm->remFifo.mr->lkey;
  if (ncclParamIbUseInline()) rComm->remFifo.flags = IBV_SEND_INLINE;

/* 中文导读：
 * 条件启用的额外硬件资源：每个这样的 RecvComm 增加 1 个自连接 RC QP 和 1 个小 host MR。
 * 该 QP 用 RDMA_READ 协助 GPU 内存可见性，不是又一条跨机数据连接，数量也不随 nqps 倍增。
 */
  // Allocate Flush dummy buffer for GPU Direct RDMA
  rComm->gpuFlush.enabled = (ncclIbGdrSupport(lComm->dev) == 0) && (ncclParamIbGdrFlushDisable() == 0) ? 1 : 0;
  if (rComm->gpuFlush.enabled) {
    NCCLCHECK(wrap_ibv_reg_mr(&rComm->gpuFlush.hostMr, rComm->verbs.pd, &rComm->gpuFlush.hostMem, sizeof(int), IBV_ACCESS_LOCAL_WRITE));
    rComm->gpuFlush.sge.addr = (uint64_t)&rComm->gpuFlush.hostMem;
    rComm->gpuFlush.sge.length = 1;
    rComm->gpuFlush.sge.lkey = rComm->gpuFlush.hostMr->lkey;
    NCCLCHECK(ncclIbCreateQp(ib_port, &rComm->verbs, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ, &rComm->gpuFlush.qp));
/* 中文逐项-变量 V4355：localQpInfo
 * localQpInfo（struct ncclIbQpInfo；函数 ncclIbAccept 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：localQpInfo.lid=portAttr.lid;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclIbQpInfo localQpInfo;
    localQpInfo.lid=portAttr.lid;
    localQpInfo.link_layer=portAttr.link_layer;
    localQpInfo.ib_port=ib_port;
    localQpInfo.spn=gid.global.subnet_prefix;
    localQpInfo.iid=gid.global.interface_id;
    localQpInfo.mtu=portAttr.active_mtu;
    NCCLCHECK(ncclIbRtrQp(rComm->gpuFlush.qp, rComm->gpuFlush.qp->qp_num, &localQpInfo));
    NCCLCHECK(ncclIbRtsQp(rComm->gpuFlush.qp));
  }

  // Fill Handle
/* 中文逐项-变量 V4356：qpInfo
 * qpInfo（struct ncclIbQpInfo；函数 ncclIbAccept 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：qpInfo.lid=portAttr.lid;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclIbQpInfo qpInfo;
  qpInfo.lid=portAttr.lid;
  qpInfo.link_layer=portAttr.link_layer;
  qpInfo.ib_port=ib_port;
  for (/* 中文逐项-变量 V4357：q
 * q（int；函数 ncclIbAccept 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<rComm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<rComm->nqps; q++) qpInfo.qpn[q]=rComm->qps[q]->qp_num;
  qpInfo.spn=gid.global.subnet_prefix;
  qpInfo.iid=gid.global.interface_id;
  qpInfo.mtu=remQpInfo.mtu;

  stage->state = ncclIbCommStateSend;
  stage->offset = 0;
  if (stage->buffer) free(stage->buffer);
  NCCLCHECK(ncclIbMalloc((void**)&stage->buffer, sizeof(struct ncclIbQpInfo)));
  memcpy(stage->buffer, &qpInfo, sizeof(struct ncclIbQpInfo));
ib_send:
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_SEND, &rComm->sock, stage->buffer, sizeof(struct ncclIbQpInfo), &stage->offset));
  if (stage->offset < sizeof(struct ncclIbQpInfo)) return ncclSuccess;

  free(stage->buffer);
  *recvComm = rComm;

  /* reset lComm stage */
  stage->state = ncclIbCommStateStart;
  stage->offset = 0;
  stage->comm = NULL;
  stage->buffer = NULL;
  return ncclSuccess;
}

/* 中文逐项-函数 F1115：ncclIbGetRequest
 * 用途与位置：从固定请求池取空闲项并初始化类型/事件计数，避免每次网络请求动态 malloc。
 * 参数 verbs（struct ncclIbVerbs* verbs）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 * 参数 req（struct ncclIbRequest** req）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbIsend；src/transport/net_ib.cc::ncclIbIrecv；src/transport/net_ib.cc::ncclIbIflush。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：i<MAX_REQUESTS；r->type == NCCL_NET_IB_REQ_UNUSED。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbGetRequest(struct ncclIbVerbs* verbs, struct ncclIbRequest** req) {
  for (/* 中文逐项-变量 V4358：i
 * i（int；函数 ncclIbGetRequest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<MAX_REQUESTS；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<MAX_REQUESTS; i++) {
/* 中文逐项-变量 V4359：r
 * r（struct ncclIbRequest；函数 ncclIbGetRequest 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 r = verbs->reqs+i；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclIbRequest* r = verbs->reqs+i;
    if (r->type == NCCL_NET_IB_REQ_UNUSED) {
      r->verbs = verbs;
      r->events = 1;
      r->addr = NULL;
      *req = r;
      return ncclSuccess;
    }
  }
  WARN("NET/IB : unable to allocate requests");
  *req = NULL;
  return ncclInternalError;
}
/* 中文逐项-函数 F1116：ncclIbFreeRequest
 * 用途与位置：把已完成请求槽标记为可复用，只有其关联完成事件耗尽后才能调用。
 * 参数 r（struct ncclIbRequest* r）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbTest。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbFreeRequest(struct ncclIbRequest* r) {
  r->type = NCCL_NET_IB_REQ_UNUSED;
  return ncclSuccess;
}

/* 中文逐项-函数 F1117：ncclSendCheck
 * 用途与位置：推进/验证握手后的 ready 状态，确保后续网络请求不会使用尚未完整连接的资源。
 * 参数 comm（struct ncclIbSendComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbIsend。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclIbRtrQp；ncclIbRtsQp；ncclSocketProgress；ncclSocketSend；ncclSocketWait。
 * 需要对照的边界/推进条件：bytes == 0；q<comm->nqps。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclSendCheck(struct ncclIbSendComm* comm) {
/* 中文逐项-变量 V4360：remQpInfo
 * remQpInfo（struct ncclIbQpInfo；函数 ncclSendCheck 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &comm->sock, &remQpInfo, sizeof(remQpInfo),
 * &bytes));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclIbQpInfo remQpInfo;

  // Do not block on this receive, return if not ready.
/* 中文逐项-变量 V4361：bytes
 * bytes（int；函数 ncclSendCheck 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bytes = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int bytes = 0;
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &comm->sock, &remQpInfo, sizeof(remQpInfo), &bytes));
  if (bytes == 0) return ncclSuccess; // Try again later
  NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, &comm->sock, &remQpInfo, sizeof(remQpInfo), &bytes));

  for (/* 中文逐项-变量 V4362：q
 * q（int；函数 ncclSendCheck 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++) {
/* 中文逐项-变量 V4363：qp
 * qp（struct ibv_qp；函数 ncclSendCheck 内）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 初值/来源 qp = comm->qps[q]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ibv_qp* qp = comm->qps[q];
    NCCLCHECK(ncclIbRtrQp(qp, remQpInfo.qpn[q], &remQpInfo));
    NCCLCHECK(ncclIbRtsQp(qp));
  }
  comm->ready = 1;
  // Block until this is done. It *should* not block indefinitely.
  NCCLCHECK(ncclSocketSend(&comm->sock, &comm->ready, sizeof(int)));

  return ncclSuccess;
}

/* 中文逐项-函数 F1118：ncclRecvCheck
 * 用途与位置：推进/验证握手后的 ready 状态，确保后续网络请求不会使用尚未完整连接的资源。
 * 参数 comm（struct ncclIbRecvComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbIrecv。
 * 本体直接出现的调用/包装：NCCLCHECK；ncclSocketProgress；ncclSocketWait。
 * 需要对照的边界/推进条件：bytes == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclRecvCheck(struct ncclIbRecvComm* comm) {
  // Do not block on this receive, return if not ready.
/* 中文逐项-变量 V4364：bytes
 * bytes（int；函数 ncclRecvCheck 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 bytes = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int bytes = 0;
  NCCLCHECK(ncclSocketProgress(NCCL_SOCKET_RECV, &comm->sock, &comm->ready, sizeof(int), &bytes));
  if (bytes == 0) return ncclSuccess; // Try again later
  NCCLCHECK(ncclSocketWait(NCCL_SOCKET_RECV, &comm->sock, &comm->ready, sizeof(int), &bytes));
  return ncclSuccess;
}

/* 中文导读：
 * 非阻塞轮询 CQ，检查 WC 状态并按 wr_id 找到请求，递减相应事件计数。
 * 一个请求可跨多个 QP/聚合多个子请求，只有要求的完成事件全部收齐才报告 done。
 */
/* 中文逐项-接口 X1285：ncclIbTest
 * 接口职责：轮询 CQ 并按 wr_id 找回请求递减完成事件，只有全部事件完成才返回 done 和接收大小。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclIbTest(void* request, int* done, int* size);
 */
ncclResult_t ncclIbTest(void* request, int* done, int* size);

/* 中文导读：
 * 对协议数据缓冲做页对齐后查设备级 MR 缓存；精确匹配 addr/pages 时增引用，否则调用 verbs 注册。
 * 此函数没有遍历 QP，mhandle 本质上返回 ibv_mr，发送使用 lkey，远端写入使用接收方发布的 rkey。
 * 缓存局限于本进程对应设备条目及共享 PD；不同进程不能直接共享这里的软件缓存。
 */
/* 中文逐项-函数 F1119：ncclIbRegMr
 * 用途与位置：按页对齐范围查 MR 缓存，命中增加引用，否则向 verbs 注册缓冲并保存键值。
 * 参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 type（int type）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 * 参数 mhandle（void** mhandle）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECKGOTO；assert；ncclRealloc；pthread_mutex_lock；pthread_mutex_unlock；sysconf；wrap_ibv_reg_mr；wrap_ibv_reg_mr_iova2。
 * 需要对照的边界/推进条件：pageSize == 0；slot == cache->population；cache->population ==
 * cache->capacity；ncclIbRelaxedOrderingEnabled。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return res;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclIbRegMr(void* comm, void* data, int size, int type, void** mhandle) {
  static_assert(offsetof(struct ncclIbSendComm, verbs) == offsetof(struct ncclIbRecvComm, verbs), "Send and recv comms must have verbs at the same offset");
  assert(size > 0);

/* 中文逐项-变量 V4365：pageSize
 * pageSize（uintptr_t；函数 ncclIbRegMr 内）：当前对象或片段的容量，初始化式中的sizeof/乘除关系确定字节或元素单位。
 * 初值/来源 pageSize = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  static __thread uintptr_t pageSize = 0;
  if (pageSize == 0) pageSize = sysconf(_SC_PAGESIZE);

/* 中文逐项-变量 V4366：verbs
 * verbs（struct ncclIbVerbs；函数 ncclIbRegMr 内）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 * 初值/来源 verbs = (struct ncclIbVerbs*)comm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbVerbs* verbs = (struct ncclIbVerbs*)comm;
/* 中文逐项-变量 V4367：cache
 * cache（struct ncclIbMrCache；函数 ncclIbRegMr 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cache = &ncclIbDevs[verbs->dev].mrCache；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclIbMrCache* cache = &ncclIbDevs[verbs->dev].mrCache;
/* 中文逐项-变量 V4368：addr
 * addr（uintptr_t；函数 ncclIbRegMr 内）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 初值/来源 addr = (uintptr_t)data & -pageSize；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  uintptr_t addr = (uintptr_t)data & -pageSize;
/* 中文逐项-变量 V4369：pages
 * pages（int；函数 ncclIbRegMr 内）：注册覆盖的页数，与页对齐起点共同组成缓存匹配范围。
 * 初值/来源 pages = ((uintptr_t)data + size - addr + pageSize-1)/pageSize；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  int pages = ((uintptr_t)data + size - addr + pageSize-1)/pageSize;
/* 中文逐项-变量 V4370：res
 * res（ncclResult_t；函数 ncclIbRegMr 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(ncclRealloc(&cache->slots, cache->population, cache->capacity), res,
 * returning);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t res;
  pthread_mutex_lock(&ncclIbDevs[verbs->dev].lock);
  for (/* 中文逐项-变量 V4371：slot
 * slot（int；函数 ncclIbRegMr 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 slot = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */int slot=0; /*true*/; slot++) {
    if (slot == cache->population) { // didn't find in cache
      if (cache->population == cache->capacity) { // must grow cache
        cache->capacity = cache->capacity < 32 ? 32 : 2*cache->capacity;
        NCCLCHECKGOTO(ncclRealloc(&cache->slots, cache->population, cache->capacity), res, returning);
      }
      // Deregister / register
/* 中文逐项-变量 V4372：mr
 * mr（struct ibv_mr；函数 ncclIbRegMr 内）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_ibv_reg_mr_iova2(&mr, verbs->pd, (void*)addr, pages*pageSize,
 * (uintptr_t)addr, flags|IBV_ACCESS_RELAXED_ORDERING), res, returning);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      struct ibv_mr* mr;
/* 中文逐项-变量 V4373：flags
 * flags（unsigned int；函数 ncclIbRegMr 内）：按位组合的当前线程角色/直接访问/共享偏移/中止等状态，按位测试而非单一枚举。
 * 初值/来源 flags =
 * IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
      unsigned int flags = IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_WRITE|IBV_ACCESS_REMOTE_READ;
      if (ncclIbRelaxedOrderingEnabled) {
        // Use IBVERBS_1.8 API - needed for IBV_ACCESS_RELAXED_ORDERING support
        NCCLCHECKGOTO(wrap_ibv_reg_mr_iova2(&mr, verbs->pd, (void*)addr, pages*pageSize, (uintptr_t)addr, flags|IBV_ACCESS_RELAXED_ORDERING), res, returning);
      }
      else {
        NCCLCHECKGOTO(wrap_ibv_reg_mr(&mr, verbs->pd, (void*)addr, pages*pageSize, flags), res, returning);
      }
      TRACE(NCCL_INIT,"regAddr %llx size %lld rkey %x", (unsigned long long)addr, (long long)pages*pageSize, mr->rkey);
      cache->population += 1;
      cache->slots[slot].addr = addr;
      cache->slots[slot].pages = pages;
      cache->slots[slot].refs = 1;
      cache->slots[slot].mr = mr;
      *mhandle = (void*)mr;
      res = ncclSuccess;
      goto returning;
    }
    else if (cache->slots[slot].addr == addr && cache->slots[slot].pages == pages) {
      cache->slots[slot].refs += 1;
      *mhandle = (void*)cache->slots[slot].mr;
      res = ncclSuccess;
      goto returning;
    }
  }
returning:
  pthread_mutex_unlock(&ncclIbDevs[verbs->dev].lock);
  return res;
}

/* 中文导读：
 * 释放一次注册引用；只有引用数归零才从缓存删除并真正 ibv_dereg_mr。
 * 单次通信结束不调用这里释放持久缓冲 MR，实际释放由 transport/连接的生命周期控制。
 */
/* 中文逐项-函数 F1120：ncclIbDeregMr
 * 用途与位置：按缓存引用计数释放注册，最后一个引用消失时真正 dereg_mr 并移除缓存项。
 * 参数 comm（void* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECKGOTO；free；memmove；pthread_mutex_lock；pthread_mutex_unlock；wrap_ibv_dereg_mr。
 * 需要对照的边界/推进条件：i < cache->population；mhandle == cache->slots[i].mr；0 == --cache->slots[i].refs；cache->population ==
 * 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return res;。CHECK类宏还可能提前返回/跳转。
 * 同步阅读点：本函数含屏障、fence或CPU锁/条件变量；它们分别约束参与线程、可见性或元数据互斥，不能互相替代。
 */
ncclResult_t ncclIbDeregMr(void* comm, void* mhandle) {
/* 中文逐项-变量 V4374：verbs
 * verbs（struct ncclIbVerbs；函数 ncclIbDeregMr 内）：backend连接的PD/CQ/请求池上下文，一个逻辑NET连接可包含多个数据QP。
 * 初值/来源 verbs = (struct ncclIbVerbs*)comm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbVerbs* verbs = (struct ncclIbVerbs*)comm;
/* 中文逐项-变量 V4375：cache
 * cache（struct ncclIbMrCache；函数 ncclIbDeregMr 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 cache = &ncclIbDevs[verbs->dev].mrCache；取得已有对象/数组元素地址，不在这一表达式中创建新分配。
 */
  struct ncclIbMrCache* cache = &ncclIbDevs[verbs->dev].mrCache;
/* 中文逐项-变量 V4376：res
 * res（ncclResult_t；函数 ncclIbDeregMr 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECKGOTO(wrap_ibv_dereg_mr((struct ibv_mr*)mhandle), res,
 * returning);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  ncclResult_t res;
  pthread_mutex_lock(&ncclIbDevs[verbs->dev].lock);
  for (/* 中文逐项-变量 V4377：i
 * i（int；函数 ncclIbDeregMr 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i < cache->population；循环体与增量表达式决定每轮如何推进。
 */int i=0; i < cache->population; i++) {
    if (mhandle == cache->slots[i].mr) {
      if (0 == --cache->slots[i].refs) {
        memmove(&cache->slots[i], &cache->slots[--cache->population], sizeof(struct ncclIbMr));
        if (cache->population == 0) {
          free(cache->slots);
          cache->slots = NULL;
          cache->capacity = 0;
        }
        NCCLCHECKGOTO(wrap_ibv_dereg_mr((struct ibv_mr*)mhandle), res, returning);
      }
      res = ncclSuccess;
      goto returning;
    }
  }
  WARN("NET/IB: could not find mr %p inside cache of %d entries", mhandle, cache->population);
  res = ncclInternalError;
returning:
  pthread_mutex_unlock(&ncclIbDevs[verbs->dev].lock);
  return res;
}

/* 中文导读：
 * 从接收方公布的 addr/rkey 生成 RDMA_WRITE 链，末端使用 WRITE_WITH_IMM 通知接收完成。
 * 消息按 nqps 切分并以 128B 对齐后投递，各 QP 使用同一个已注册缓冲的 key，不为每个分片另建 MR。
 * 多接收聚合或超过自适应路由阈值时，数据 WRITE 与末尾零字节通知可以分开。
 */
/* 中文逐项-函数 F1121：ncclIbMultiSend
 * 用途与位置：把匹配接收的一个或多个发送请求分到数据 QP，组织 RDMA_WRITE/WRITE_WITH_IMM 链和完成标识。
 * 参数 comm（struct ncclIbSendComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 slot（int slot）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbIsend。
 * 本体直接出现的调用/包装：DIVUP；NCCLCHECK；memset；ncclParamIbArThreshold；std::min；wrap_ibv_post_send。
 * 需要对照的边界/推进条件：nreqs > NCCL_NET_IB_MAX_RECVS；r<nreqs；nreqs == 1；r<nreqs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbMultiSend(struct ncclIbSendComm* comm, int slot) {
/* 中文逐项-变量 V4378：reqs
 * reqs（struct ncclIbRequest；函数 ncclIbMultiSend 内）：在途网络请求或固定请求池；只有 Test 完成后其槽才能重用。
 * 初值/来源 reqs = comm->fifoReqs[slot]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclIbRequest** reqs = comm->fifoReqs[slot];
/* 中文逐项-变量 V4379：slots
 * slots（struct ncclIbSendFifo；函数 ncclIbMultiSend 内）：缓存/池的存储数组、总容量和已占用条目统计，扩容/删除须保持一致。
 * 初值/来源 slots = comm->fifo[slot]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  volatile struct ncclIbSendFifo* slots = comm->fifo[slot];
/* 中文逐项-变量 V4380：nreqs
 * nreqs（int；函数 ncclIbMultiSend 内）：一条后端请求聚合的子请求数，用于完成计数和长度结果解码。
 * 初值/来源 nreqs = slots[0].nreqs；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  int nreqs = slots[0].nreqs;
  if (nreqs > NCCL_NET_IB_MAX_RECVS) return ncclInternalError;

/* 中文逐项-变量 V4381：wr_id
 * wr_id（uint64_t；函数 ncclIbMultiSend 内）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 初值/来源 wr_id = 0ULL；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  uint64_t wr_id = 0ULL;

  for (/* 中文逐项-变量 V4382：r
 * r（int；函数 ncclIbMultiSend 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nreqs；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nreqs; r++) {
/* 中文逐项-变量 V4383：wr
 * wr（struct ibv_send_wr；函数 ncclIbMultiSend 内）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 初值/来源 wr = comm->wrs+r；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ibv_send_wr* wr = comm->wrs+r;
    memset(wr, 0, sizeof(struct ibv_send_wr));

/* 中文逐项-变量 V4384：sge
 * sge（struct ibv_sge；函数 ncclIbMultiSend 内）：scatter/gather 地址段列表或单段，指定本地地址、长度和lkey。
 * 初值/来源 sge = comm->sges+r；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ibv_sge* sge = comm->sges+r;
    sge->addr=(uintptr_t)reqs[r]->send.data;
    sge->lkey=reqs[r]->send.lkey;

    wr->opcode = IBV_WR_RDMA_WRITE;
    wr->send_flags = 0;
    wr->wr.rdma.remote_addr = slots[r].addr;
    wr->wr.rdma.rkey = slots[r].rkey;
    wr->next = wr+1;
    wr_id += (reqs[r] - comm->verbs.reqs) << (r*8);
  }

  // Write size as immediate data. In the case of multi-send, only write
  // 0 or 1 as size to indicate whether there was data sent or received.
/* 中文逐项-变量 V4385：immData
 * immData（uint64_t；函数 ncclIbMultiSend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 immData = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  uint64_t immData = 0;
  if (nreqs == 1) {
    immData = reqs[0]->send.size;
  } else {
/* 中文逐项-变量 V4386：multiImmData
 * multiImmData（uint8_t；函数 ncclIbMultiSend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 multiImmData = (uint8_t*)&immData；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    uint8_t* multiImmData = (uint8_t*)&immData;
    for (/* 中文逐项-变量 V4387：r
 * r（int；函数 ncclIbMultiSend 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nreqs；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nreqs; r++) {
      multiImmData[r] = reqs[r]->send.size ? 1 : 0;
    }
  }

/* 中文逐项-变量 V4388：lastWr
 * lastWr（struct ibv_send_wr；函数 ncclIbMultiSend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lastWr = comm->wrs+nreqs-1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ibv_send_wr* lastWr = comm->wrs+nreqs-1;
  if (nreqs > 1 || reqs[0]->send.size > ncclParamIbArThreshold()) {
    // When using adaptive routing, send the bulk of the data first as an
    // RDMA_WRITE, then a 0-byte RDMA_WRITE_WITH_IMM to trigger a remote
    // completion.
    lastWr++;
    memset(lastWr, 0, sizeof(struct ibv_send_wr));
  }
  lastWr->wr_id = wr_id;
  lastWr->opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
  lastWr->imm_data = immData;
  lastWr->next = NULL;
  lastWr->send_flags = IBV_SEND_SIGNALED;

  // Multi-QP: make sure IB writes are multiples of 128B so that LL and LL128 protocols still work
/* 中文逐项-变量 V4389：align
 * align（int；函数 ncclIbMultiSend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 align = 128；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  const int align = 128;
  for (/* 中文逐项-变量 V4390：q
 * q（int；函数 ncclIbMultiSend 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++) {
    for (/* 中文逐项-变量 V4391：r
 * r（int；函数 ncclIbMultiSend 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nreqs；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nreqs; r++) {
/* 中文逐项-变量 V4392：chunkSize
 * chunkSize（int；函数 ncclIbMultiSend 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = DIVUP(DIVUP(reqs[r]->send.size, comm->nqps), align) * align；向上取整分配，确保不能整除时也覆盖最后不足一块的数据。
 */
      int chunkSize = DIVUP(DIVUP(reqs[r]->send.size, comm->nqps), align) * align;
/* 中文逐项-变量 V4393：length
 * length（int；函数 ncclIbMultiSend 内）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 初值/来源 length = std::min(reqs[r]->send.size-reqs[r]->send.offset, chunkSize)；取候选与剩余/上限中的较小值，避免本轮范围超过可用容量。
 */
      int length = std::min(reqs[r]->send.size-reqs[r]->send.offset, chunkSize);
      if (length <= 0) {
        comm->wrs[r].sg_list = NULL;
        comm->wrs[r].num_sge = 0;
      } else {
        comm->sges[r].length = length;
        comm->wrs[r].sg_list = comm->sges+r;
        comm->wrs[r].num_sge = 1;
      }
    }
/* 中文逐项-变量 V4394：bad_wr
 * bad_wr（struct ibv_send_wr；函数 ncclIbMultiSend 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_post_send(comm->qps[q], comm->wrs, &bad_wr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ibv_send_wr* bad_wr;
    NCCLCHECK(wrap_ibv_post_send(comm->qps[q], comm->wrs, &bad_wr));

    for (/* 中文逐项-变量 V4395：r
 * r（int；函数 ncclIbMultiSend 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nreqs；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nreqs; r++) {
/* 中文逐项-变量 V4396：chunkSize
 * chunkSize（int；函数 ncclIbMultiSend 内）：算法/协议块容量；主机 NET 调度多用字节，设备 T 算法通常已除 sizeof(T) 变成元素数。
 * 初值/来源 chunkSize = DIVUP(DIVUP(reqs[r]->send.size, comm->nqps), align) * align；向上取整分配，确保不能整除时也覆盖最后不足一块的数据。
 */
      int chunkSize = DIVUP(DIVUP(reqs[r]->send.size, comm->nqps), align) * align;
      reqs[r]->send.offset += chunkSize;
      comm->sges[r].addr += chunkSize;
      comm->wrs[r].wr.rdma.remote_addr += chunkSize;
    }
  }

  return ncclSuccess;
}

/* 中文导读：
 * 先检查连接就绪和接收方 FIFO 发布信息，以 tag 匹配发送请求，收齐同一聚合槽后调用 MultiSend。
 * request 为空时上层应继续推进重试；API 返回成功不等于网卡已经完成传输。
 */
/* 中文逐项-函数 F1122：ncclIbIsend
 * 用途与位置：先等待/匹配远端 FIFO 中的地址、rkey、tag 与容量，再把可发送项交给 MultiSend。
 * 参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 data（void* data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 size（int size）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 tag（int tag）：控制消息或NET请求的匹配标签；公开ncclSend/Recv没有用户tag参数，此值由内部组织。
 * 参数 mhandle（void* mhandle）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；TIME_START；TIME_STOP；__sync_synchronize；memset；ncclIbGetRequest；ncclIbMultiSend；ncclSendCheck；ncclSocketToString。
 * 需要对照的边界/推进条件：comm->ready == 0；comm->ready == 0；slots[0].idx != idx；r<nreqs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInvalidUsage;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbIsend(void* sendComm, void* data, int size, int tag, void* mhandle, void** request) {
/* 中文逐项-变量 V4397：comm
 * comm（struct ncclIbSendComm；函数 ncclIbIsend 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclIbSendComm*)sendComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbSendComm* comm = (struct ncclIbSendComm*)sendComm;
  if (comm->ready == 0) NCCLCHECK(ncclSendCheck(comm));
  if (comm->ready == 0) { *request = NULL; return ncclSuccess; }

/* 中文逐项-变量 V4398：mr
 * mr（struct ibv_mr；函数 ncclIbIsend 内）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 * 初值/来源 mr = (struct ibv_mr*)mhandle；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ibv_mr* mr = (struct ibv_mr*)mhandle;

  // Wait for the receiver to have posted the corresponding receive
/* 中文逐项-变量 V4399：nreqs
 * nreqs（int；函数 ncclIbIsend 内）：一条后端请求聚合的子请求数，用于完成计数和长度结果解码。
 * 初值/来源 nreqs = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int nreqs = 0;
/* 中文逐项-变量 V4400：slots
 * slots（struct ncclIbSendFifo；函数 ncclIbIsend 内）：缓存/池的存储数组、总容量和已占用条目统计，扩容/删除须保持一致。
 * 此处未显式给初值；后续首个文本使用点：slots = comm->fifo[slot];。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  volatile struct ncclIbSendFifo* slots;

/* 中文逐项-变量 V4401：slot
 * slot（int；函数 ncclIbIsend 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 slot = (comm->fifoHead)%MAX_REQUESTS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
  int slot = (comm->fifoHead)%MAX_REQUESTS;
/* 中文逐项-变量 V4402：reqs
 * reqs（struct ncclIbRequest；函数 ncclIbIsend 内）：在途网络请求或固定请求池；只有 Test 完成后其槽才能重用。
 * 初值/来源 reqs = comm->fifoReqs[slot]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclIbRequest** reqs = comm->fifoReqs[slot];
  slots = comm->fifo[slot];
/* 中文逐项-变量 V4403：idx
 * idx（int；函数 ncclIbIsend 内）：网络控制FIFO条目的轮次序号，避免环形槽复用时把旧地址授权当作新请求。
 * 初值/来源 idx = comm->fifoHead+1；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int idx = comm->fifoHead+1;
  if (slots[0].idx != idx) { *request = NULL; return ncclSuccess; }
  nreqs = slots[0].nreqs;
  // Wait until all data has arrived
  for (/* 中文逐项-变量 V4404：r
 * r（int；函数 ncclIbIsend 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 1；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 * 所在for循环的继续条件：r<nreqs；循环体与增量表达式决定每轮如何推进。
 */int r=1; r<nreqs; r++) while(slots[r].idx != idx);
  __sync_synchronize(); // order the nreqsPtr load against tag/rkey/addr loads below
  for (/* 中文逐项-变量 V4405：r
 * r（int；函数 ncclIbIsend 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nreqs；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nreqs; r++) {
    if (reqs[r] != NULL || slots[r].tag != tag) continue;

    // Sanity checks to catch user collective call count/size mismatches
    if (size > slots[r].size) {
/* 中文逐项-变量 V4406：line
 * line（char；函数 ncclIbIsend 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：r, nreqs, tag, ncclSocketToString(&comm->sock.addr, line), size,
 * slots[r].size);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      char line[SOCKET_NAME_MAXLEN+1];
      WARN("NET/IB : req %d/%d tag %x peer %s collective mismatch error, local size %d remote size %d",
           r, nreqs, tag, ncclSocketToString(&comm->sock.addr, line), size, slots[r].size);
      return ncclInvalidUsage;
    } // plus any potential programming errors
    else if (slots[r].size < 0 || slots[r].addr == 0 || slots[r].rkey == 0) {
/* 中文逐项-变量 V4407：line
 * line（char；函数 ncclIbIsend 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：r, nreqs, tag, ncclSocketToString(&comm->sock.addr, line), slots[r].size, slots[r].addr,
 * slots[r].rkey);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
     char line[SOCKET_NAME_MAXLEN+1];
     WARN("NET/IB : req %d/%d tag %x peer %s posted incorrect receive info: size %d addr %lx rkey %x",
          r, nreqs, tag, ncclSocketToString(&comm->sock.addr, line), slots[r].size, slots[r].addr, slots[r].rkey);
      return ncclInternalError;
    }
/* 中文逐项-变量 V4408：req
 * req（struct ncclIbRequest；函数 ncclIbIsend 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ncclIbRequest* req;
    NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));
    req->type = NCCL_NET_IB_REQ_SEND;
    req->addr = &comm->sock.addr;
    req->verbs = &comm->verbs;
    req->nreqs = nreqs;
    req->send.size = size;
    req->send.data = data;
    req->send.lkey = mr->lkey;
    req->send.offset = 0;
    req->addr = &comm->sock.addr;
    req->events = comm->nqps;
    *request = reqs[r] = req;

    // If this is a multi-recv, send only when all requests have matched.
    for (/* 中文逐项-变量 V4409：r
 * r（int；函数 ncclIbIsend 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 r = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：r<nreqs；循环体与增量表达式决定每轮如何推进。
 */int r=0; r<nreqs; r++) {
      if (reqs[r] == NULL) return ncclSuccess;
    }

    TIME_START(0);
    NCCLCHECK(ncclIbMultiSend(comm, slot));

    // Clear slots[0]->nreqs, as well as other fields to help debugging and sanity checks
    memset((void*)slots, 0, sizeof(struct ncclIbSendFifo));
    memset(reqs, 0, NCCL_NET_IB_MAX_RECVS*sizeof(struct ncclIbRequest*));
    comm->fifoHead++;
    TIME_STOP(0);
    return ncclSuccess;
  }

  *request = NULL;
  return ncclSuccess;
}

/* 中文导读：
 * 接收方把目标 addr、rkey、tag、size 和序号写回发送方控制 FIFO，授予向该缓冲写入的条件。
 * 控制消息也用 RDMA_WRITE；周期性请求 signaled completion，避免一直 unsignaled 导致 SQ 无法回收。
 */
/* 中文逐项-函数 F1123：ncclIbPostFifo
 * 用途与位置：把接收缓冲地址/rkey/尺寸/tag/序号 RDMA 写回发送端，授予本轮写入目标并维持周期 signaled 完成。
 * 参数 comm（struct ncclIbRecvComm* comm）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 tags（int* tags）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 req（struct ncclIbRequest* req）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbIrecv。
 * 本体直接出现的调用/包装：NCCLCHECK；memset；wrap_ibv_post_send。
 * 需要对照的边界/推进条件：i<n；slot == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbPostFifo(struct ncclIbRecvComm* comm, int n, void** data, int* sizes, int* tags, void** mhandles, struct ncclIbRequest* req) {
/* 中文逐项-变量 V4410：wr
 * wr（struct ibv_send_wr；函数 ncclIbPostFifo 内）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 此处未显式给初值；后续首个文本使用点：memset(&wr, 0, sizeof(wr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_send_wr wr;
  memset(&wr, 0, sizeof(wr));

/* 中文逐项-变量 V4411：slot
 * slot（int；函数 ncclIbPostFifo 内）：对有界环形队列取模后的实际槽位下标；须等使用方完成后才能覆盖。
 * 初值/来源 slot = comm->remFifo.fifoTail%MAX_REQUESTS；把单调序号折回有界环形数组槽；取模不代表旧槽一定已空闲，仍须配套流控。
 */
  int slot = comm->remFifo.fifoTail%MAX_REQUESTS;
/* 中文逐项-变量 V4412：localElem
 * localElem（struct ncclIbSendFifo；函数 ncclIbPostFifo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 localElem = comm->remFifo.elems[slot]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclIbSendFifo* localElem = comm->remFifo.elems[slot];

  for (/* 中文逐项-变量 V4413：i
 * i（int；函数 ncclIbPostFifo 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<n；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<n; i++) {
    localElem[i].addr = (uint64_t)data[i];
/* 中文逐项-变量 V4414：mr
 * mr（struct ibv_mr；函数 ncclIbPostFifo 内）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 * 初值/来源 mr = (struct ibv_mr*)mhandles[i]；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    struct ibv_mr* mr = (struct ibv_mr*)mhandles[i];
    localElem[i].rkey = mr->rkey;
    localElem[i].nreqs = n;
    localElem[i].size = sizes[i]; // Sanity/Debugging
    localElem[i].tag = tags[i];
    localElem[i].idx = comm->remFifo.fifoTail+1;
  }

  wr.wr.rdma.remote_addr = comm->remFifo.addr + slot*NCCL_NET_IB_MAX_RECVS*sizeof(struct ncclIbSendFifo);
  wr.wr.rdma.rkey = comm->remFifo.rkey;
  comm->remFifo.sge.addr = (uint64_t)localElem;
  comm->remFifo.sge.length = n*sizeof(struct ncclIbSendFifo);
  wr.sg_list = &comm->remFifo.sge;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_RDMA_WRITE;
  wr.send_flags = comm->remFifo.flags; // IBV_SEND_INLINE

  // We need to occasionally post a request with the IBV_SEND_SIGNALED flag, otherwise
  // the send queue will never empty.
  //
  // From https://www.rdmamojo.com/2014/06/30/working-unsignaled-completions/
  // "How to use Unsignaled Completion?" / "Gotchas and Pitfalls"
  // All posted Send Requested, Signaled and Unsignaled, are considered outstanding until
  // a Work Completion that they, or Send Requests that were posted after them, was polled
  // from the Completion Queue associated with the Send Queue. This means if one works with
  // a Queue Pair that was configured to work with Unsignaled Completions, he must make
  // sure that occasionally (before the Send Queue is full with outstanding Send Requests)
  // a Send Request that generate Work Completion will be posted.
  //
  // Not following this rule may lead to a case that the Send Queue is full with Send
  // Requests that won't generate Work Completion:
  //
  //  - The Send Queue is full, so no new Send Requests can be posted to it
  //  - The Send Queue can't be emptied, since no Work Completion can be generated anymore
  //    (the reason is that no Work Completion, that can generate Work Completion that
  //    polling it will empty the Send Queue, can be posted)
  //  - The status of all posted Send Request is considered unknown
  //
  if (slot == 0) {
    wr.send_flags |= IBV_SEND_SIGNALED;
    wr.wr_id = req - comm->verbs.reqs;
    req->events++;
  }

/* 中文逐项-变量 V4415：bad_wr
 * bad_wr（struct ibv_send_wr；函数 ncclIbPostFifo 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_post_send(comm->qps[0], &wr, &bad_wr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_send_wr* bad_wr;
  NCCLCHECK(wrap_ibv_post_send(comm->qps[0], &wr, &bad_wr));
  comm->remFifo.fifoTail++;

  return ncclSuccess;
}

/* 中文导读：
 * 先在每个数据 QP 上 post_recv 一个零 SGE 接收 WQE，再公布接收缓冲地址/rkey。
 * 零 SGE 因 payload 直接 RDMA 写入目标 MR，RQ WQE 用于接收 WRITE_WITH_IMM 完成通知。
 * 这不是 SRQ：代码直接向各个 qp 投递，完成事件数先设为 nqps。
 */
/* 中文逐项-函数 F1124：ncclIbIrecv
 * 用途与位置：在每个数据 QP 挂零 SGE 接收 WQE以接收立即数通知，再发布本轮接收目标地址。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 tags（int* tags）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；TIME_START；TIME_STOP；memset；ncclIbGetRequest；ncclIbPostFifo；ncclRecvCheck；wrap_ibv_post_recv。
 * 需要对照的边界/推进条件：comm->ready == 0；comm->ready == 0；n > NCCL_NET_IB_MAX_RECVS；i<n。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbIrecv(void* recvComm, int n, void** data, int* sizes, int* tags, void** mhandles, void** request) {
/* 中文逐项-变量 V4416：comm
 * comm（struct ncclIbRecvComm；函数 ncclIbIrecv 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclIbRecvComm*)recvComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;
  if (comm->ready == 0) NCCLCHECK(ncclRecvCheck(comm));
  if (comm->ready == 0) { *request = NULL; return ncclSuccess; }
  if (n > NCCL_NET_IB_MAX_RECVS) return ncclInternalError;

/* 中文逐项-变量 V4417：req
 * req（struct ncclIbRequest；函数 ncclIbIrecv 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclIbRequest* req;
  NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));
  req->type = NCCL_NET_IB_REQ_RECV;
  req->addr = &comm->sock.addr;
  req->nreqs = n;
  for (/* 中文逐项-变量 V4418：i
 * i（int；函数 ncclIbIrecv 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<n；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<n; i++) req->recv.sizes[i] = 0;

/* 中文逐项-变量 V4419：wr
 * wr（struct ibv_recv_wr；函数 ncclIbIrecv 内）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 此处未显式给初值；后续首个文本使用点：memset(&wr, 0, sizeof(wr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_recv_wr wr;
  memset(&wr, 0, sizeof(wr));
  wr.wr_id = req - comm->verbs.reqs;

  wr.sg_list = NULL;
  wr.num_sge = 0;

  TIME_START(1);
  for (/* 中文逐项-变量 V4420：q
 * q（int；函数 ncclIbIrecv 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++) {
/* 中文逐项-变量 V4421：qp
 * qp（struct ibv_qp；函数 ncclIbIrecv 内）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 初值/来源 qp = comm->qps[q]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ibv_qp* qp = comm->qps[q];
/* 中文逐项-变量 V4422：bad_wr
 * bad_wr（struct ibv_recv_wr；函数 ncclIbIrecv 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_post_recv(qp, &wr, &bad_wr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ibv_recv_wr* bad_wr;
    NCCLCHECK(wrap_ibv_post_recv(qp, &wr, &bad_wr));
  }
  TIME_STOP(1);
  req->events = comm->nqps;

  *request = req;

  // Post to FIFO to notify sender
  TIME_START(2);
  NCCLCHECK(ncclIbPostFifo(comm, n, data, sizes, tags, mhandles, req));
  TIME_STOP(2);
  return ncclSuccess;
}

/* 中文导读：
 * 必要时利用自连接 flush QP 从收到的 GPU 数据区域执行一个小 RDMA_READ。
 * 目标是建立所需可见性，不是再次搬运整个消息；是否调用还受上层协议/GDR/平台分支控制。
 */
/* 中文逐项-函数 F1125：ncclIbIflush
 * 用途与位置：需要 GDR 可见性保障时，对目标 GPU 缓冲发小型自连接 RDMA read 并返回可 test 的请求。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 参数 n（int n）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 参数 data（void** data）：当前对象的数据区或固定布局填充；看宿主类型区分用户条目、控制句柄和对齐空间。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 参数 mhandles（void** mhandles）：网络接口返回的注册句柄，本版内置IB通常可转成 ibv_mr；插件内部表示可不同。
 * 参数 request（void** request）：输出句柄地址，成功/已接受后由被调用实现写入有效对象；调用成功但输出仍为空时须按异步建链/请求契约继续推进。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；TIME_START；TIME_STOP；memset；ncclIbGetRequest；wrap_ibv_post_send。
 * 需要对照的边界/推进条件：i<n；sizes[i]；comm->gpuFlush.enabled == 0 || last == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbIflush(void* recvComm, int n, void** data, int* sizes, void** mhandles, void** request) {
/* 中文逐项-变量 V4423：comm
 * comm（struct ncclIbRecvComm；函数 ncclIbIflush 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclIbRecvComm*)recvComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;
/* 中文逐项-变量 V4424：last
 * last（int；函数 ncclIbIflush 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 last = -1；以该实现约定的无效/未选择值起步，后续检查后才可作为合法索引使用。
 */
  int last = -1;
  for (/* 中文逐项-变量 V4425：i
 * i（int；函数 ncclIbIflush 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<n；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<n; i++) if (sizes[i]) last = i;
  if (comm->gpuFlush.enabled == 0 || last == -1) return ncclSuccess;

  // Only flush once using the last non-zero receive
/* 中文逐项-变量 V4426：req
 * req（struct ncclIbRequest；函数 ncclIbIflush 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclIbRequest* req;
  NCCLCHECK(ncclIbGetRequest(&comm->verbs, &req));
  req->type = NCCL_NET_IB_REQ_FLUSH;
  req->addr = &comm->sock.addr;
/* 中文逐项-变量 V4427：mr
 * mr（struct ibv_mr；函数 ncclIbIflush 内）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 * 初值/来源 mr = (struct ibv_mr*)mhandles[last]；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ibv_mr* mr = (struct ibv_mr*)mhandles[last];

/* 中文逐项-变量 V4428：wr
 * wr（struct ibv_send_wr；函数 ncclIbIflush 内）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 此处未显式给初值；后续首个文本使用点：memset(&wr, 0, sizeof(wr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_send_wr wr;
  memset(&wr, 0, sizeof(wr));
  wr.wr_id = req - comm->verbs.reqs;

  wr.wr.rdma.remote_addr = (uint64_t)data[last];
  wr.wr.rdma.rkey = mr->rkey;
  wr.sg_list = &comm->gpuFlush.sge;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_RDMA_READ;
  wr.send_flags = IBV_SEND_SIGNALED;

  TIME_START(4);
/* 中文逐项-变量 V4429：bad_wr
 * bad_wr（struct ibv_send_wr；函数 ncclIbIflush 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_post_send(comm->gpuFlush.qp, &wr, &bad_wr));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ibv_send_wr* bad_wr;
  NCCLCHECK(wrap_ibv_post_send(comm->gpuFlush.qp, &wr, &bad_wr));
  TIME_STOP(4);

  *request = req;
  return ncclSuccess;
}

/* 中文精读：一个 request 可能对应多个 CQ completion
 * r->events 记录此请求尚待完成的事件数，只有减到 0 才 done 并回收 request 槽。
 * 一次 poll_cq 也可能取出同一 CQ 上其他 request 的完成，按 wr_id 解码找到真正所属对象。
 * 多个 QP 拆分发送或聚合接收时，不能把一个完成事件直接等同于整个用户消息完成。
 * wc.status 非成功会打印对端地址、opcode、vendor_err，定位是网络执行错误而非 GPU 归约错误。
 */
/* 中文逐项-函数 F1126：ncclIbTest
 * 用途与位置：轮询 CQ 并按 wr_id 找回请求递减完成事件，只有全部事件完成才返回 done 和接收大小。
 * 参数 request（void* request）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 参数 done（int* done）：完成状态输出地址；实现写1表示当前异步请求已完成，写0表示还要继续推进，不能把它当成已传输字节数。
 * 参数 sizes（int* sizes）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；TIME_CANCEL；TIME_START；TIME_STOP；ncclIbFreeRequest；ncclSocketToString；wrap_ibv_poll_cq。
 * 需要对照的边界/推进条件：1；r->events == 0；sizes && r->type == NCCL_NET_IB_REQ_RECV；i<r->nreqs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclSystemError;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbTest(void* request, int* done, int* sizes) {
/* 中文逐项-变量 V4430：r
 * r（struct ncclIbRequest；函数 ncclIbTest 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 r = (struct ncclIbRequest*)request；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbRequest *r = (struct ncclIbRequest*)request;
  *done = 0;

  while (1) {
    if (r->events == 0) {
      *done = 1;
      if (sizes && r->type == NCCL_NET_IB_REQ_RECV) {
        for (/* 中文逐项-变量 V4431：i
 * i（int；函数 ncclIbTest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<r->nreqs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<r->nreqs; i++) sizes[i] = r->recv.sizes[i];
      }
      NCCLCHECK(ncclIbFreeRequest(r));
      return ncclSuccess;
    }

/* 中文逐项-变量 V4432：wrDone
 * wrDone（int；函数 ncclIbTest 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 wrDone = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
    int wrDone = 0;
/* 中文逐项-变量 V4433：wcs
 * wcs（struct ibv_wc；函数 ncclIbTest 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(wrap_ibv_poll_cq(r->verbs->cq, 4, wcs, &wrDone));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    struct ibv_wc wcs[4];
    TIME_START(3);
    NCCLCHECK(wrap_ibv_poll_cq(r->verbs->cq, 4, wcs, &wrDone));
    if (wrDone == 0) { TIME_CANCEL(3); } else { TIME_STOP(3); }
    if (wrDone == 0) return ncclSuccess;

    for (/* 中文逐项-变量 V4434：w
 * w（int；函数 ncclIbTest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 w = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：w<wrDone；循环体与增量表达式决定每轮如何推进。
 */int w=0; w<wrDone; w++) {
/* 中文逐项-变量 V4435：wc
 * wc（struct ibv_wc；函数 ncclIbTest 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 wc = wcs+w；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
      struct ibv_wc *wc = wcs+w;
      if (wc->status != IBV_WC_SUCCESS) {
/* 中文逐项-变量 V4436：line
 * line（char；函数 ncclIbTest 内）：源文件/行号或文本行缓冲，按类型判断是日志定位信息还是输入文本。
 * 此处未显式给初值；后续首个文本使用点：ncclSocketToString(r->addr, line), wc->status, wc->opcode, wc->byte_len,
 * wc->vendor_err);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
        char line[SOCKET_NAME_MAXLEN+1];
        WARN("NET/IB : Got completion from peer %s with error %d, opcode %d, len %d, vendor err %d",
             ncclSocketToString(r->addr, line), wc->status, wc->opcode, wc->byte_len, wc->vendor_err);
        return ncclSystemError;
      }

/* 中文逐项-变量 V4437：req
 * req（struct ncclIbRequest；函数 ncclIbTest 内）：本次异步网络请求的句柄/状态，成功提交不等于请求已经完成。
 * 初值/来源 req = r->verbs->reqs+(wc->wr_id & 0xff)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
      struct ncclIbRequest* req = r->verbs->reqs+(wc->wr_id & 0xff);
      if (req->type == NCCL_NET_IB_REQ_SEND) {
        for (/* 中文逐项-变量 V4438：i
 * i（int；函数 ncclIbTest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<req->nreqs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<req->nreqs; i++) {
/* 中文逐项-变量 V4439：sendReq
 * sendReq（struct ncclIbRequest；函数 ncclIbTest 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 sendReq = r->verbs->reqs+((wc->wr_id >> (i*8)) & 0xff)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
          struct ncclIbRequest* sendReq = r->verbs->reqs+((wc->wr_id >> (i*8)) & 0xff);
          if ((sendReq->events <= 0)) return ncclInternalError;
          sendReq->events--;
        }
      } else {
        if (req && wc->opcode == IBV_WC_RECV_RDMA_WITH_IMM) {
          if (req->type != NCCL_NET_IB_REQ_RECV) return ncclInternalError;
          if (req->nreqs > 1) {
            // In the case of a multi recv, we only set sizes to 0 or 1.
/* 中文逐项-变量 V4440：sizes
 * sizes（uint8_t；函数 ncclIbTest 内）：聚合接收各项的有效字节长度结果，网络Test和立即数解码后更新。
 * 初值/来源 sizes = (uint8_t*)&wc->imm_data；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
            uint8_t* sizes = (uint8_t*)&wc->imm_data;
            for (/* 中文逐项-变量 V4441：i
 * i（int；函数 ncclIbTest 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<req->nreqs；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<req->nreqs; i++) {
              req->recv.sizes[i] |= sizes[i];
            }
          } else {
            req->recv.sizes[0] += wc->imm_data;
          }
        }
        req->events--;
      }
    }
  }
}

/* 中文精读：网络资源释放要跟随真实 backend 连接
 * 先关闭建链控制 socket，逐个销毁数据 QP，注销该连接的控制 FIFO MR，再回收 verbs 资源。
 * NET 上层通常先注销各协议数据缓冲的 MR，再关闭 backend comm；共享连接还受引用计数约束。
 * 逻辑 channel 销毁与底层连接实际销毁不是永远一一对应，最后引用释放才可回收共享对象。
 * 这里不是每次 AllReduce 结束都执行；后续任务通常复用已建好的连接和注册缓冲。
 */
/* 中文逐项-函数 F1127：ncclIbCloseSend
 * 用途与位置：关闭该方向的 IB 控制 socket、数据 QP、控制 MR及相关 verbs/flush 对象，按持有资源清理。
 * 参数 sendComm（void* sendComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；TIME_PRINT；close；free；ncclIbDestroyVerbs；wrap_ibv_dereg_mr；wrap_ibv_destroy_qp。
 * 需要对照的边界/推进条件：comm；q<comm->nqps；comm->qps[q] != NULL；comm->fifoMr != NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbCloseSend(void* sendComm) {
/* 中文逐项-变量 V4442：comm
 * comm（struct ncclIbSendComm；函数 ncclIbCloseSend 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclIbSendComm*)sendComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbSendComm* comm = (struct ncclIbSendComm*)sendComm;
  if (comm) {
    close(comm->sock.fd);
    for (/* 中文逐项-变量 V4443：q
 * q（int；函数 ncclIbCloseSend 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++)
      if (comm->qps[q] != NULL) NCCLCHECK(wrap_ibv_destroy_qp(comm->qps[q]));
    if (comm->fifoMr != NULL) NCCLCHECK(wrap_ibv_dereg_mr(comm->fifoMr));
    NCCLCHECK(ncclIbDestroyVerbs(&comm->verbs));
    free(comm);
  }
  TIME_PRINT("IB");
  return ncclSuccess;
}

/* 中文逐项-函数 F1128：ncclIbCloseRecv
 * 用途与位置：关闭该方向的 IB 控制 socket、数据 QP、控制 MR及相关 verbs/flush 对象，按持有资源清理。
 * 参数 recvComm（void* recvComm）：后端实际发送/接收连接句柄；可被多个满足共享条件的逻辑 connector 引用。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；close；free；ncclIbDestroyVerbs；wrap_ibv_dereg_mr；wrap_ibv_destroy_qp。
 * 需要对照的边界/推进条件：comm；q<comm->nqps；comm->qps[q] != NULL；comm->gpuFlush.enabled。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbCloseRecv(void* recvComm) {
/* 中文逐项-变量 V4444：comm
 * comm（struct ncclIbRecvComm；函数 ncclIbCloseRecv 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclIbRecvComm*)recvComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbRecvComm* comm = (struct ncclIbRecvComm*)recvComm;
  if (comm) {
    close(comm->sock.fd);
    for (/* 中文逐项-变量 V4445：q
 * q（int；函数 ncclIbCloseRecv 内）：当前数据QP编号，按nqps遍历；每条连接可创建多个QP。
 * 初值/来源 q = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：q<comm->nqps；循环体与增量表达式决定每轮如何推进。
 */int q=0; q<comm->nqps; q++)
      if (comm->qps[q] != NULL) NCCLCHECK(wrap_ibv_destroy_qp(comm->qps[q]));
    if (comm->gpuFlush.enabled) {
      if (comm->gpuFlush.qp != NULL) NCCLCHECK(wrap_ibv_destroy_qp(comm->gpuFlush.qp));
      if (comm->gpuFlush.hostMr != NULL) NCCLCHECK(wrap_ibv_dereg_mr(comm->gpuFlush.hostMr));
    }
    if (comm->remFifo.mr != NULL) NCCLCHECK(wrap_ibv_dereg_mr(comm->remFifo.mr));
    NCCLCHECK(ncclIbDestroyVerbs(&comm->verbs));
    free(comm);
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F1129：ncclIbCloseListen
 * 用途与位置：关闭监听 socket 并释放 IB listen 对象，不负责释放独立的已建立连接。
 * 参数 listenComm（void* listenComm）：对应库返回的不透明对象/连接句柄，必须使用匹配的关闭或释放回调。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：close；free。
 * 需要对照的边界/推进条件：comm。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclIbCloseListen(void* listenComm) {
/* 中文逐项-变量 V4446：comm
 * comm（struct ncclIbListenComm；函数 ncclIbCloseListen 内）：当前 rank 的通信上下文；保存拓扑、channel、排队和代理状态，不是整个集群共用的单个对象。
 * 初值/来源 comm = (struct ncclIbListenComm*)listenComm；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
  struct ncclIbListenComm* comm = (struct ncclIbListenComm*)listenComm;
  if (comm) {
    close(comm->sock.fd);
    free(comm);
  }
  return ncclSuccess;
}

/* 中文逐项-变量 V4447：ncclNetIb
 * ncclNetIb（ncclNet_t；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclNetIb = { "IB", ncclIbInit, ncclIbDevices, ncclIbGetProperties, ncclIbListen, ncclIbConnect,
 * ncclIbAccept, ncclIbRegMr, ncclIbDeregMr, ncclIbIsend, ncclIbIrecv, ncclIbIflush, ncclIbTest, ncclIbCloseSend,
 * ncclIbCloseRecv, ncclIbCloseListen …（完整表达式见紧邻源码）；聚合初始化按结构/数组成员顺序填入；对照该类型的字段声明理解每一项，未显式项遵守语言的初始化规则。
 */
ncclNet_t ncclNetIb = {
  "IB",
  ncclIbInit,
  ncclIbDevices,
  ncclIbGetProperties,
  ncclIbListen,
  ncclIbConnect,
  ncclIbAccept,
  ncclIbRegMr,
  ncclIbDeregMr,
  ncclIbIsend,
  ncclIbIrecv,
  ncclIbIflush,
  ncclIbTest,
  ncclIbCloseSend,
  ncclIbCloseRecv,
  ncclIbCloseListen
};

