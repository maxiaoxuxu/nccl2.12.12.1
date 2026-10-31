/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "ibvwrap.h"
#include <sys/types.h>
#include <unistd.h>

#include <dlfcn.h>
#include "core.h"

/* 中文逐项-变量 V3370：ibvState
 * ibvState（enum { ibvUninitialized, ibvInitializing, ibvInitialized, ibvError
 * }；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ibvState = ibvUninitialized；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
static /* 中文逐项-类型 T0391：<匿名enum>
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum { /* 中文逐项-枚举项 X1164：ibvUninitialized
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ibvUninitialized, /* 中文逐项-枚举项 X1165：ibvInitializing
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ibvInitializing, /* 中文逐项-枚举项 X1166：ibvInitialized
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ibvInitialized, /* 中文逐项-枚举项 X1167：ibvError
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */ibvError } ibvState = ibvUninitialized;

/*Function Pointers*/
/* 中文逐项-变量 V3371：ibv_internal_fork_init
 * ibv_internal_fork_init
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_fork_init操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_fork_init)(void)。
 * 回调参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * ibv_internal_fork_init（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_fork_init", ibv_internal_fork_init);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_fork_init)(void);
/* 中文逐项-变量 V3372：ibv_internal_get_device_list
 * ibv_internal_get_device_list
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_get_device_list操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：**
 * (*ibv_internal_get_device_list)(int *num_devices)。
 * ibv_internal_get_device_list（struct ibv_device；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_get_device_list",
 * ibv_internal_get_device_list);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
struct ibv_device** (*ibv_internal_get_device_list)(int *num_devices);
/* 中文逐项-变量 V3373：ibv_internal_free_device_list
 * ibv_internal_free_device_list
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_free_device_list操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_free_device_list)(struct
 * ibv_device **list)。
 * 回调参数 list（struct ibv_device **list）：节点或链路指针列表，count给出有效条目数，遍历不得越过当前有效范围。
 * ibv_internal_free_device_list（void；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_free_device_list",
 * ibv_internal_free_device_list);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
void (*ibv_internal_free_device_list)(struct ibv_device **list);
/* 中文逐项-变量 V3374：ibv_internal_get_device_name
 * ibv_internal_get_device_name
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_get_device_name操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*ibv_internal_get_device_name)(struct ibv_device *device)。
 * ibv_internal_get_device_name（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_get_device_name",
 * ibv_internal_get_device_name);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
const char * (*ibv_internal_get_device_name)(struct ibv_device *device);
/* 中文逐项-变量 V3375：ibv_internal_open_device
 * ibv_internal_open_device
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_open_device操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*ibv_internal_open_device)(struct ibv_device* device)。
 * ibv_internal_open_device（struct ibv_context；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_open_device", ibv_internal_open_device);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
struct ibv_context* (*ibv_internal_open_device)(struct ibv_device* device);
/* 中文逐项-变量 V3376：ibv_internal_close_device
 * ibv_internal_close_device
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_close_device操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_close_device)(struct
 * ibv_context *context)。
 * 回调参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * ibv_internal_close_device（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_close_device", ibv_internal_close_device);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_close_device)(struct ibv_context *context);
/* 中文逐项-变量 V3377：ibv_internal_get_async_event
 * ibv_internal_get_async_event
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_get_async_event操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_get_async_event)(struct
 * ibv_context *context, struct ibv_async_event *event)。
 * 回调参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 回调参数 event（struct ibv_async_event *event）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * ibv_internal_get_async_event（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_get_async_event",
 * ibv_internal_get_async_event);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_get_async_event)(struct ibv_context *context, struct ibv_async_event *event);
/* 中文逐项-变量 V3378：ibv_internal_ack_async_event
 * ibv_internal_ack_async_event
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_ack_async_event操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_ack_async_event)(struct
 * ibv_async_event *event)。
 * 回调参数 event（struct ibv_async_event *event）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * ibv_internal_ack_async_event（void；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_ack_async_event",
 * ibv_internal_ack_async_event);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
void (*ibv_internal_ack_async_event)(struct ibv_async_event *event);
/* 中文逐项-变量 V3379：ibv_internal_query_device
 * ibv_internal_query_device
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_query_device操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_query_device)(struct
 * ibv_context *context, struct ibv_device_attr *device_attr)。
 * 回调参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 回调参数 device_attr（struct ibv_device_attr *device_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * ibv_internal_query_device（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_query_device", ibv_internal_query_device);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_query_device)(struct ibv_context *context, struct ibv_device_attr *device_attr);
/* 中文逐项-变量 V3380：ibv_internal_query_port
 * ibv_internal_query_port
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_query_port操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_query_port)(struct
 * ibv_context *context, uint8_t port_num, struct ibv_port_attr *port_attr)。
 * 回调参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 回调参数 port_num（uint8_t port_num）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 回调参数 port_attr（struct ibv_port_attr *port_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * ibv_internal_query_port（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_query_port", ibv_internal_query_port);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_query_port)(struct ibv_context *context, uint8_t port_num, struct ibv_port_attr *port_attr);
/* 中文逐项-变量 V3381：ibv_internal_query_gid
 * ibv_internal_query_gid
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_query_gid操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_query_gid)(struct
 * ibv_context *context, uint8_t port_num, int index, union ibv_gid *gid)。
 * 回调参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 回调参数 port_num（uint8_t port_num）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 回调参数 index（int index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 回调参数 gid（union ibv_gid *gid）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 * ibv_internal_query_gid（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_query_gid", ibv_internal_query_gid);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_query_gid)(struct ibv_context *context, uint8_t port_num, int index, union ibv_gid *gid);
/* 中文逐项-变量 V3382：ibv_internal_query_qp
 * ibv_internal_query_qp
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_query_qp操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_query_qp)(struct
 * ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask, struct ibv_qp_init_attr *init_attr)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 attr（struct ibv_qp_attr *attr）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 回调参数 attr_mask（int attr_mask）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 回调参数 init_attr（struct ibv_qp_init_attr *init_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * ibv_internal_query_qp（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_query_qp", ibv_internal_query_qp);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_query_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask, struct ibv_qp_init_attr *init_attr);
/* 中文逐项-变量 V3383：ibv_internal_alloc_pd
 * ibv_internal_alloc_pd
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_alloc_pd操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*ibv_internal_alloc_pd)(struct ibv_context *context)。
 * ibv_internal_alloc_pd（struct ibv_pd；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_alloc_pd", ibv_internal_alloc_pd);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
struct ibv_pd * (*ibv_internal_alloc_pd)(struct ibv_context *context);
/* 中文逐项-变量 V3384：ibv_internal_dealloc_pd
 * ibv_internal_dealloc_pd
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_dealloc_pd操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_dealloc_pd)(struct
 * ibv_pd *pd)。
 * 回调参数 pd（struct ibv_pd *pd）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 * ibv_internal_dealloc_pd（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_dealloc_pd", ibv_internal_dealloc_pd);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_dealloc_pd)(struct ibv_pd *pd);
/* 中文逐项-变量 V3385：ibv_internal_reg_mr
 * ibv_internal_reg_mr 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_reg_mr操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*ibv_internal_reg_mr)(struct ibv_pd *pd, void *addr, size_t length, int access)。
 * ibv_internal_reg_mr（struct ibv_mr；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_reg_mr", ibv_internal_reg_mr);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
struct ibv_mr * (*ibv_internal_reg_mr)(struct ibv_pd *pd, void *addr, size_t length, int access);
/* 中文逐项-变量 V3386：ibv_internal_reg_mr_iova2
 * ibv_internal_reg_mr_iova2
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_reg_mr_iova2操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*ibv_internal_reg_mr_iova2)(struct ibv_pd *pd, void *addr, size_t length, uint64_t iova, int access)。
 * ibv_internal_reg_mr_iova2（struct ibv_mr；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM_VERSION(ibvhandle, "ibv_reg_mr_iova2", ibv_internal_reg_mr_iova2,
 * "IBVERBS_1.8");。若由输出参数填入，必须先检查填充调用成功再读值。
 */
struct ibv_mr * (*ibv_internal_reg_mr_iova2)(struct ibv_pd *pd, void *addr, size_t length, uint64_t iova, int access);
/* 中文逐项-变量 V3387：ibv_internal_dereg_mr
 * ibv_internal_dereg_mr
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_dereg_mr操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_dereg_mr)(struct
 * ibv_mr *mr)。
 * 回调参数 mr（struct ibv_mr *mr）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 * ibv_internal_dereg_mr（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_dereg_mr", ibv_internal_dereg_mr);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_dereg_mr)(struct ibv_mr *mr);
/* 中文逐项-变量 V3388：ibv_internal_create_cq
 * ibv_internal_create_cq
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_create_cq操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*ibv_internal_create_cq)(struct ibv_context *context, int cqe, void *cq_context, struct ibv_comp_channel *channel,
 * int comp_vector)。
 * ibv_internal_create_cq（struct ibv_cq；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_create_cq", ibv_internal_create_cq);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
struct ibv_cq * (*ibv_internal_create_cq)(struct ibv_context *context, int cqe, void *cq_context, struct ibv_comp_channel *channel, int comp_vector);
/* 中文逐项-变量 V3389：ibv_internal_destroy_cq
 * ibv_internal_destroy_cq
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_destroy_cq操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_destroy_cq)(struct
 * ibv_cq *cq)。
 * 回调参数 cq（struct ibv_cq *cq）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 * ibv_internal_destroy_cq（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_destroy_cq", ibv_internal_destroy_cq);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_destroy_cq)(struct ibv_cq *cq);
/* 中文逐项-变量 V3390：ibv_internal_create_qp
 * ibv_internal_create_qp
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_create_qp操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*ibv_internal_create_qp)(struct ibv_pd *pd, struct ibv_qp_init_attr *qp_init_attr)。
 * ibv_internal_create_qp（struct ibv_qp；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_create_qp", ibv_internal_create_qp);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
struct ibv_qp * (*ibv_internal_create_qp)(struct ibv_pd *pd, struct ibv_qp_init_attr *qp_init_attr);
/* 中文逐项-变量 V3391：ibv_internal_modify_qp
 * ibv_internal_modify_qp
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_modify_qp操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_modify_qp)(struct
 * ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 attr（struct ibv_qp_attr *attr）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 回调参数 attr_mask（int attr_mask）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * ibv_internal_modify_qp（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_modify_qp", ibv_internal_modify_qp);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_modify_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask);
/* 中文逐项-变量 V3392：ibv_internal_destroy_qp
 * ibv_internal_destroy_qp
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_destroy_qp操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*ibv_internal_destroy_qp)(struct
 * ibv_qp *qp)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * ibv_internal_destroy_qp（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_destroy_qp", ibv_internal_destroy_qp);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
int (*ibv_internal_destroy_qp)(struct ibv_qp *qp);
/* 中文逐项-变量 V3393：ibv_internal_event_type_str
 * ibv_internal_event_type_str
 * 是函数指针字段/变量：由该资源库/provider提供的ibv_internal_event_type_str操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*ibv_internal_event_type_str)(enum ibv_event_type event)。
 * ibv_internal_event_type_str（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：LOAD_SYM(ibvhandle, "ibv_event_type_str", ibv_internal_event_type_str);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
const char * (*ibv_internal_event_type_str)(enum ibv_event_type event);

// IBVERBS Library versioning
/* 中文逐项-常量 X1168：IBVERBS_VERSION
 * 编译期定义：IBVERBS_VERSION = "IBVERBS_1.1"。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define IBVERBS_VERSION "IBVERBS_1.1"

/* 中文导读：
 * 动态装载 libibverbs 及所需符号，隔离网络后端对具体 verbs 库可用性的依赖。
 * NCCL 的内置 IB 后端经此进入 verbs/provider；这个包装层本身不会自动引入 UCX 或 SRQ。
 */
/* 中文精读：verbs 包装与厂商 provider 的层次
 * 此处动态解析 libibverbs 符号并统一错误返回；post_send/poll_cq 等热路径还可走 verbs 对象的 ops。
 * 厂商 provider 负责把 verbs 操作落实到自己的设备队列、doorbell 与硬件完成机制。
 * NCCL net_ib 是 verbs 的使用者，通常不直接编码某个厂商的寄存器格式。
 * 添加自研 RDMA 设备时，应区分 rdma-core provider 接入与 NCCL 网络插件接入这两个接口层。
 */
/* 中文逐项-函数 F0924：wrap_ibv_symbols
 * 用途与位置：动态装载并解析 libibverbs 符号表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：LOAD_SYM；LOAD_SYM_VERSION；__sync_bool_compare_and_swap；dlclose；dlopen；pthread_yield。
 * 需要对照的边界/推进条件：ibvState == ibvInitialized；ibvState == ibvError；__sync_bool_compare_and_swap(&ibvState,
 * ibvUninitialized, ibvInitializing) == false；ibvState == ibvInitializing。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclSystemError;；return (ibvState == ibvInitialized) ? ncclSuccess :
 * ncclSystemError;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_ibv_symbols(void) {
  if (ibvState == ibvInitialized)
    return ncclSuccess;
  if (ibvState == ibvError)
    return ncclSystemError;

  if (__sync_bool_compare_and_swap(&ibvState, ibvUninitialized, ibvInitializing) == false) {
    // Another thread raced in front of us. Wait for it to be done.
    while (ibvState == ibvInitializing) pthread_yield();
    return (ibvState == ibvInitialized) ? ncclSuccess : ncclSystemError;
  }

/* 中文逐项-变量 V3394：ibvhandle
 * ibvhandle（void；函数 wrap_ibv_symbols 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ibvhandle = NULL；初始为空地址，尚未持有对象，后续分配/映射成功后再更新。
 */
  static void* ibvhandle = NULL;
/* 中文逐项-变量 V3395：tmp
 * tmp（void；函数 wrap_ibv_symbols 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：tmp = dlvsym(handle, symbol, IBVERBS_VERSION); \。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  void* tmp;
/* 中文逐项-变量 V3396：cast
 * cast（void；函数 wrap_ibv_symbols 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：cast = (void**)&funcptr; \。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  void** cast;

  ibvhandle=dlopen("libibverbs.so", RTLD_NOW);
  if (!ibvhandle) {
    ibvhandle=dlopen("libibverbs.so.1", RTLD_NOW);
    if (!ibvhandle) {
      INFO(NCCL_INIT, "Failed to open libibverbs.so[.1]");
      goto teardown;
    }
  }

/* 中文逐项-宏 M0136：LOAD_SYM
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 LOAD_SYM；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define LOAD_SYM(handle, symbol, funcptr) do {           \
    cast = (void**)&funcptr;                             \
    tmp = dlvsym(handle, symbol, IBVERBS_VERSION);       \
    if (tmp == NULL) {                                   \
      WARN("dlvsym failed on %s - %s version %s", symbol, dlerror(), IBVERBS_VERSION);  \
      goto teardown;                                     \
    }                                                    \
    *cast = tmp;                                         \
  } while (0)

// Attempt to load a specific symbol version - fail silently
/* 中文逐项-宏 M0137：LOAD_SYM_VERSION
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 LOAD_SYM_VERSION；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define LOAD_SYM_VERSION(handle, symbol, funcptr, version) do {  \
    cast = (void**)&funcptr;                                     \
    *cast = dlvsym(handle, symbol, version);                     \
  } while (0)

  LOAD_SYM(ibvhandle, "ibv_get_device_list", ibv_internal_get_device_list);
  LOAD_SYM(ibvhandle, "ibv_free_device_list", ibv_internal_free_device_list);
  LOAD_SYM(ibvhandle, "ibv_get_device_name", ibv_internal_get_device_name);
  LOAD_SYM(ibvhandle, "ibv_open_device", ibv_internal_open_device);
  LOAD_SYM(ibvhandle, "ibv_close_device", ibv_internal_close_device);
  LOAD_SYM(ibvhandle, "ibv_get_async_event", ibv_internal_get_async_event);
  LOAD_SYM(ibvhandle, "ibv_ack_async_event", ibv_internal_ack_async_event);
  LOAD_SYM(ibvhandle, "ibv_query_device", ibv_internal_query_device);
  LOAD_SYM(ibvhandle, "ibv_query_port", ibv_internal_query_port);
  LOAD_SYM(ibvhandle, "ibv_query_gid", ibv_internal_query_gid);
  LOAD_SYM(ibvhandle, "ibv_query_qp", ibv_internal_query_qp);
  LOAD_SYM(ibvhandle, "ibv_alloc_pd", ibv_internal_alloc_pd);
  LOAD_SYM(ibvhandle, "ibv_dealloc_pd", ibv_internal_dealloc_pd);
  LOAD_SYM(ibvhandle, "ibv_reg_mr", ibv_internal_reg_mr);
  // Cherry-pick the ibv_reg_mr_iova2 API from IBVERBS 1.8
  LOAD_SYM_VERSION(ibvhandle, "ibv_reg_mr_iova2", ibv_internal_reg_mr_iova2, "IBVERBS_1.8");
  LOAD_SYM(ibvhandle, "ibv_dereg_mr", ibv_internal_dereg_mr);
  LOAD_SYM(ibvhandle, "ibv_create_cq", ibv_internal_create_cq);
  LOAD_SYM(ibvhandle, "ibv_destroy_cq", ibv_internal_destroy_cq);
  LOAD_SYM(ibvhandle, "ibv_create_qp", ibv_internal_create_qp);
  LOAD_SYM(ibvhandle, "ibv_modify_qp", ibv_internal_modify_qp);
  LOAD_SYM(ibvhandle, "ibv_destroy_qp", ibv_internal_destroy_qp);
  LOAD_SYM(ibvhandle, "ibv_fork_init", ibv_internal_fork_init);
  LOAD_SYM(ibvhandle, "ibv_event_type_str", ibv_internal_event_type_str);

  ibvState = ibvInitialized;
  return ncclSuccess;

teardown:
  ibv_internal_get_device_list = NULL;
  ibv_internal_free_device_list = NULL;
  ibv_internal_get_device_name = NULL;
  ibv_internal_open_device = NULL;
  ibv_internal_close_device = NULL;
  ibv_internal_get_async_event = NULL;
  ibv_internal_ack_async_event = NULL;
  ibv_internal_query_device = NULL;
  ibv_internal_query_port = NULL;
  ibv_internal_query_gid = NULL;
  ibv_internal_query_qp = NULL;
  ibv_internal_alloc_pd = NULL;
  ibv_internal_dealloc_pd = NULL;
  ibv_internal_reg_mr = NULL;
  ibv_internal_reg_mr_iova2 = NULL;
  ibv_internal_dereg_mr = NULL;
  ibv_internal_create_cq = NULL;
  ibv_internal_destroy_cq = NULL;
  ibv_internal_create_qp = NULL;
  ibv_internal_modify_qp = NULL;
  ibv_internal_destroy_qp = NULL;
  ibv_internal_fork_init = NULL;
  ibv_internal_event_type_str = NULL;

  if (ibvhandle != NULL) dlclose(ibvhandle);
  ibvState = ibvError;
  return ncclSystemError;
}

/* 中文逐项-宏 M0138：IBV_PTR_CHECK_ERRNO
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 IBV_PTR_CHECK_ERRNO；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IBV_PTR_CHECK_ERRNO(name_internal, call, retval, error_retval, name) \
  if (name_internal == NULL) { \
     WARN("lib wrapper not initialized."); \
     return ncclInternalError; \
  } \
  retval = call; \
  if (retval == error_retval) { \
    WARN("Call to " name " failed with error %s", strerror(errno)); \
    return ncclSystemError; \
  } \
  return ncclSuccess;

/* 中文逐项-宏 M0139：IBV_PTR_CHECK
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 IBV_PTR_CHECK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IBV_PTR_CHECK(name_internal, call, retval, error_retval, name) \
  if (name_internal == NULL) { \
     WARN("lib wrapper not initialized."); \
     return ncclInternalError; \
  } \
  retval = call; \
  if (retval == error_retval) { \
    WARN("Call to " name " failed"); \
    return ncclSystemError; \
  } \
  return ncclSuccess;

/* 中文逐项-宏 M0140：IBV_INT_CHECK_RET_ERRNO
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 IBV_INT_CHECK_RET_ERRNO；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IBV_INT_CHECK_RET_ERRNO(name_internal, call, success_retval, name) \
  if (name_internal == NULL) { \
     WARN("lib wrapper not initialized."); \
     return ncclInternalError; \
  } \
  int ret = call; \
  if (ret != success_retval) { \
    WARN("Call to " name " failed with error %s", strerror(ret)); \
    return ncclSystemError; \
  } \
  return ncclSuccess;

/* 中文逐项-宏 M0141：IBV_INT_CHECK
 * 宏职责：执行调用后检查返回；可能return、goto或写线程结果。把它看作控制流展开，失败后不保证继续执行下一源码行。
 * 原始宏名 IBV_INT_CHECK；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IBV_INT_CHECK(name_internal, call, error_retval, name) \
  if (name_internal == NULL) { \
     WARN("lib wrapper not initialized."); \
     return ncclInternalError; \
  } \
  int ret = call; \
  if (ret == error_retval) { \
    WARN("Call to " name " failed"); \
    return ncclSystemError; \
  } \
  return ncclSuccess;

/* 中文逐项-宏 M0142：IBV_PASSTHRU
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 IBV_PASSTHRU；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define IBV_PASSTHRU(name_internal, call) \
  if (name_internal == NULL) { \
     WARN("lib wrapper not initialized."); \
     return ncclInternalError; \
  } \
  call; \
  return ncclSuccess;

/* 中文逐项-函数 F0925：wrap_ibv_fork_init
 * 用途与位置：初始化 verbs 注册页的 fork 处理；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 形参：没有显式形参；成员函数可使用this对象，设备代码还可读取线程编号/共享上下文。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：IBV_INT_CHECK；ibv_internal_fork_init。
 */
ncclResult_t wrap_ibv_fork_init() {
  IBV_INT_CHECK(ibv_internal_fork_init, ibv_internal_fork_init(), -1, "ibv_fork_init");
}

/* 中文逐项-函数 F0926：wrap_ibv_get_device_list
 * 用途与位置：获取可用 RDMA 设备列表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 ret（struct ibv_device ***ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 num_devices（int *num_devices）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：ibv_internal_get_device_list。
 * 需要对照的边界/推进条件：*ret == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_ibv_get_device_list(struct ibv_device ***ret, int *num_devices) {
  *ret = ibv_internal_get_device_list(num_devices);
  if (*ret == NULL) *num_devices = 0;
  return ncclSuccess;
}

/* 中文逐项-函数 F0927：wrap_ibv_free_device_list
 * 用途与位置：归还设备枚举列表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 list（struct ibv_device **list）：节点或链路指针列表，count给出有效条目数，遍历不得越过当前有效范围。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：IBV_PASSTHRU；ibv_internal_free_device_list。
 */
ncclResult_t wrap_ibv_free_device_list(struct ibv_device **list) {
  IBV_PASSTHRU(ibv_internal_free_device_list, ibv_internal_free_device_list(list));
}

/* 中文逐项-函数 F0928：wrap_ibv_get_device_name
 * 用途与位置：读取设备名字；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 device（struct ibv_device *device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：exit；ibv_internal_get_device_name。
 * 需要对照的边界/推进条件：ibv_internal_get_device_name == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ibv_internal_get_device_name(device);。CHECK类宏还可能提前返回/跳转。
 */
const char *wrap_ibv_get_device_name(struct ibv_device *device) {
  if (ibv_internal_get_device_name == NULL) {
    WARN("lib wrapper not initialized.");
    exit(-1);
  }
  return ibv_internal_get_device_name(device);
}

/* 中文逐项-函数 F0929：wrap_ibv_open_device
 * 用途与位置：打开设备得到 verbs context；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 ret（struct ibv_context **ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 device（struct ibv_device *device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：IBV_PTR_CHECK；ibv_internal_open_device。
 */
ncclResult_t wrap_ibv_open_device(struct ibv_context **ret, struct ibv_device *device) { /*returns 0 on success, -1 on failure*/
  IBV_PTR_CHECK(ibv_internal_open_device, ibv_internal_open_device(device), *ret, NULL, "ibv_open_device");
}

/* 中文逐项-函数 F0930：wrap_ibv_close_device
 * 用途与位置：关闭设备上下文；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：IBV_INT_CHECK；ibv_internal_close_device。
 */
ncclResult_t wrap_ibv_close_device(struct ibv_context *context) { /*returns 0 on success, -1 on failure*/
  IBV_INT_CHECK(ibv_internal_close_device, ibv_internal_close_device(context), -1, "ibv_close_device");
}

/* 中文逐项-函数 F0931：wrap_ibv_get_async_event
 * 用途与位置：等待读取设备异步事件；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 event（struct ibv_async_event *event）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbAsyncThreadMain。
 * 本体直接出现的调用/包装：IBV_INT_CHECK；ibv_internal_get_async_event。
 */
ncclResult_t wrap_ibv_get_async_event(struct ibv_context *context, struct ibv_async_event *event) { /*returns 0 on success, and -1 on error*/
  IBV_INT_CHECK(ibv_internal_get_async_event, ibv_internal_get_async_event(context, event), -1, "ibv_get_async_event");
}

/* 中文逐项-函数 F0932：wrap_ibv_ack_async_event
 * 用途与位置：确认并释放已处理异步事件；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 event（struct ibv_async_event *event）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbAsyncThreadMain。
 * 本体直接出现的调用/包装：IBV_PASSTHRU；ibv_internal_ack_async_event。
 */
ncclResult_t wrap_ibv_ack_async_event(struct ibv_async_event *event) {
  IBV_PASSTHRU(ibv_internal_ack_async_event, ibv_internal_ack_async_event(event));
}

/* 中文逐项-函数 F0933：wrap_ibv_query_device
 * 用途与位置：读取设备能力限制；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 device_attr（struct ibv_device_attr *device_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_query_device。
 */
ncclResult_t wrap_ibv_query_device(struct ibv_context *context, struct ibv_device_attr *device_attr) { /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_device, ibv_internal_query_device(context, device_attr), 0, "ibv_query_device");
}

/* 中文逐项-函数 F0934：wrap_ibv_query_port
 * 用途与位置：读取端口状态/地址与速率能力；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 port_num（uint8_t port_num）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 参数 port_attr（struct ibv_port_attr *port_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInit；src/transport/net_ib.cc::ncclIbConnect；src/transport/net_ib.cc::ncclIbAccept。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_query_port。
 */
ncclResult_t wrap_ibv_query_port(struct ibv_context *context, uint8_t port_num, struct ibv_port_attr *port_attr) { /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_port, ibv_internal_query_port(context, port_num, port_attr), 0, "ibv_query_port");
}

/* 中文逐项-函数 F0935：wrap_ibv_query_gid
 * 用途与位置：读取指定端口/GID索引的地址；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 port_num（uint8_t port_num）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 参数 index（int index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 参数 gid（union ibv_gid *gid）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbConnect；src/transport/net_ib.cc::ncclIbAccept。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_query_gid。
 */
ncclResult_t wrap_ibv_query_gid(struct ibv_context *context, uint8_t port_num, int index, union ibv_gid *gid) {
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_gid, ibv_internal_query_gid(context, port_num, index, gid), 0, "ibv_query_gid");
}

/* 中文逐项-函数 F0936：wrap_ibv_query_qp
 * 用途与位置：查询 QP 当前状态和属性；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 参数 attr（struct ibv_qp_attr *attr）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 参数 attr_mask（int attr_mask）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 init_attr（struct ibv_qp_init_attr *init_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_query_qp。
 */
ncclResult_t wrap_ibv_query_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask, struct ibv_qp_init_attr *init_attr) {
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_query_qp, ibv_internal_query_qp(qp, attr, attr_mask, init_attr), 0, "ibv_query_qp");
}

/* 中文逐项-函数 F0937：wrap_ibv_alloc_pd
 * 用途与位置：分配保护域，隔离允许访问的 QP/MR 集合；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 ret（struct ibv_pd **ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInitVerbs。
 * 本体直接出现的调用/包装：IBV_PTR_CHECK；ibv_internal_alloc_pd。
 */
ncclResult_t wrap_ibv_alloc_pd(struct ibv_pd **ret, struct ibv_context *context) {
  IBV_PTR_CHECK(ibv_internal_alloc_pd, ibv_internal_alloc_pd(context), *ret, NULL, "ibv_alloc_pd");
}

/* 中文逐项-函数 F0938：wrap_ibv_dealloc_pd
 * 用途与位置：释放保护域；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 pd（struct ibv_pd *pd）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbDestroyVerbs。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_dealloc_pd。
 */
ncclResult_t wrap_ibv_dealloc_pd(struct ibv_pd *pd) { /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_dealloc_pd, ibv_internal_dealloc_pd(pd), 0, "ibv_dealloc_pd");
}

/* 中文逐项-函数 F0939：wrap_ibv_reg_mr
 * 用途与位置：把指定虚拟地址范围注册到 PD 并取得 lkey/rkey；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 ret（struct ibv_mr **ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 pd（struct ibv_pd *pd）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 * 参数 addr（void *addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 参数 length（size_t length）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 access（int access）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbConnect；src/transport/net_ib.cc::ncclIbAccept；src/transport/net_ib.cc::ncclIbRegMr。
 * 本体直接出现的调用/包装：IBV_PTR_CHECK；ibv_internal_reg_mr。
 */
ncclResult_t wrap_ibv_reg_mr(struct ibv_mr **ret, struct ibv_pd *pd, void *addr, size_t length, int access) {
  IBV_PTR_CHECK(ibv_internal_reg_mr, ibv_internal_reg_mr(pd, addr, length, access), *ret, NULL, "ibv_reg_mr");
}

/* 中文逐项-函数 F0940：wrap_direct_ibv_reg_mr
 * 用途与位置：直接调用选定 verbs 注册入口并返回 MR 指针，供需要原生指针/错误约定的能力探测使用。
 * 参数 pd（struct ibv_pd *pd）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 * 参数 addr（void *addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 参数 length（size_t length）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 access（int access）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：ibv_internal_reg_mr。
 * 需要对照的边界/推进条件：ibv_internal_reg_mr == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return NULL;；return ibv_internal_reg_mr(pd, addr, length, access);。CHECK类宏还可能提前返回/跳转。
 */
struct ibv_mr * wrap_direct_ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int access) {
  if (ibv_internal_reg_mr == NULL) {
    WARN("lib wrapper not initialized.");
    return NULL;
  }
  return ibv_internal_reg_mr(pd, addr, length, access);
}

/* 中文逐项-函数 F0941：wrap_ibv_reg_mr_iova2
 * 用途与位置：注册内存并指定 HCA 使用的 IOVA 地址及访问权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 ret（struct ibv_mr **ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 pd（struct ibv_pd *pd）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 * 参数 addr（void *addr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 * 参数 length（size_t length）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 * 参数 iova（uint64_t iova）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 access（int access）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbRelaxedOrderingCapable；src/transport/net_ib.cc::ncclIbRegMr。
 * 本体直接出现的调用/包装：IBV_PTR_CHECK；ibv_internal_reg_mr_iova2。
 * 需要对照的边界/推进条件：ibv_internal_reg_mr_iova2 == NULL；ret == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_ibv_reg_mr_iova2(struct ibv_mr **ret, struct ibv_pd *pd, void *addr, size_t length, uint64_t iova, int access) {
  if (ibv_internal_reg_mr_iova2 == NULL) {
    return ncclInternalError;
  }
  if (ret == NULL) { return ncclSuccess; } // Assume dummy call
  IBV_PTR_CHECK(ibv_internal_reg_mr_iova2, ibv_internal_reg_mr_iova2(pd, addr, length, iova, access), *ret, NULL, "ibv_reg_mr_iova2");
}

/* 中文逐项-函数 F0942：wrap_ibv_dereg_mr
 * 用途与位置：注销一个真实 MR；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 mr（struct ibv_mr *mr）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbDeregMr；src/transport/net_ib.cc::ncclIbCloseSend；src/transport/net_ib.cc::ncclIbCloseRecv。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_dereg_mr。
 */
ncclResult_t wrap_ibv_dereg_mr(struct ibv_mr *mr) { /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_dereg_mr, ibv_internal_dereg_mr(mr), 0, "ibv_dereg_mr");
}

/* 中文逐项-函数 F0943：wrap_ibv_create_cq
 * 用途与位置：创建完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 ret（struct ibv_cq **ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 参数 cqe（int cqe）：CQ最大条目能力或当前CQ容量，需容纳未被poll消耗的完成项。
 * 参数 cq_context（void *cq_context）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 * 参数 channel（struct ibv_comp_channel *channel）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 * 参数 comp_vector（int comp_vector）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbInitVerbs。
 * 本体直接出现的调用/包装：IBV_PTR_CHECK；ibv_internal_create_cq。
 */
ncclResult_t wrap_ibv_create_cq(struct ibv_cq **ret, struct ibv_context *context, int cqe, void *cq_context, struct ibv_comp_channel *channel, int comp_vector) {
  IBV_PTR_CHECK(ibv_internal_create_cq, ibv_internal_create_cq(context, cqe, cq_context, channel, comp_vector), *ret, NULL, "ibv_create_cq");
}

/* 中文逐项-函数 F0944：wrap_ibv_destroy_cq
 * 用途与位置：销毁完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 cq（struct ibv_cq *cq）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbDestroyVerbs。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_destroy_cq。
 */
ncclResult_t wrap_ibv_destroy_cq(struct ibv_cq *cq) {
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_destroy_cq, ibv_internal_destroy_cq(cq), 0, "ibv_destroy_cq");
}

/* 中文逐项-函数 F0945：wrap_ibv_destroy_qp
 * 用途与位置：销毁队列对；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbCloseSend；src/transport/net_ib.cc::ncclIbCloseRecv。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_destroy_qp。
 */
ncclResult_t wrap_ibv_destroy_qp(struct ibv_qp *qp) {
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_destroy_qp, ibv_internal_destroy_qp(qp), 0, "ibv_destroy_qp");
}

/* 中文逐项-函数 F0946：wrap_ibv_create_qp
 * 用途与位置：创建指定类型和队列容量的 QP；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 ret（struct ibv_qp **ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 pd（struct ibv_pd *pd）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 * 参数 qp_init_attr（struct ibv_qp_init_attr *qp_init_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbCreateQp。
 * 本体直接出现的调用/包装：IBV_PTR_CHECK；ibv_internal_create_qp。
 */
ncclResult_t wrap_ibv_create_qp(struct ibv_qp **ret, struct ibv_pd *pd, struct ibv_qp_init_attr *qp_init_attr) {
  IBV_PTR_CHECK(ibv_internal_create_qp, ibv_internal_create_qp(pd, qp_init_attr), *ret, NULL, "ibv_create_qp");
}

/* 中文逐项-函数 F0947：wrap_ibv_modify_qp
 * 用途与位置：按属性掩码修改 QP 状态/路由/权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 参数 attr（struct ibv_qp_attr *attr）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 参数 attr_mask（int attr_mask）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbCreateQp；src/transport/net_ib.cc::ncclIbRtrQp；src/transport/net_ib.cc::ncclIbRtsQp。
 * 本体直接出现的调用/包装：IBV_INT_CHECK_RET_ERRNO；ibv_internal_modify_qp。
 */
ncclResult_t wrap_ibv_modify_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask) { /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  IBV_INT_CHECK_RET_ERRNO(ibv_internal_modify_qp, ibv_internal_modify_qp(qp, attr, attr_mask), 0, "ibv_modify_qp");
}

/* 中文逐项-函数 F0948：wrap_ibv_event_type_str
 * 用途与位置：把设备事件枚举转换成诊断字符串；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 参数 ret（char **ret）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 参数 event（enum ibv_event_type event）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbAsyncThreadMain。
 * 本体直接出现的调用/包装：ibv_internal_event_type_str。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t wrap_ibv_event_type_str(char **ret, enum ibv_event_type event) {
  *ret = (char *) ibv_internal_event_type_str(event);
  return ncclSuccess;
}
