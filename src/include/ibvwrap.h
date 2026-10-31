/*************************************************************************
 * Copyright (c) 2004, 2005 Topspin Communications.  All rights reserved.
 * Copyright (c) 2004, 2011-2012 Intel Corporation.  All rights reserved.
 * Copyright (c) 2005, 2006, 2007 Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2005 PathScale, Inc.  All rights reserved.
 *
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_IBVWRAP_H_
/* 中文逐项-常量 X0332：NCCL_IBVWRAP_H_
 * 编译期定义：NCCL_IBVWRAP_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_IBVWRAP_H_

#include "core.h"
#include <sys/types.h>
#include <unistd.h>

// Dynamically handle dependencies on IB verbs

#if __GNUC__ >= 3
/* 中文逐项-常量 X0333：__attribute_const
 * 编译期定义：__attribute_const = __attribute__((const))。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#  define __attribute_const __attribute__((const))
#else
/* 中文逐项-常量 X0334：__attribute_const
 * 编译期定义：__attribute_const = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#  define __attribute_const
#endif

/* 中文逐项-类型 T0209：ibv_gid
 * 类型职责：128位GID地址，支持raw字节及subnet_prefix/interface_id双视图；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：raw、global、subnet_prefix、interface_id。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
union ibv_gid {
/* 中文逐项-变量 V2300：raw
 * raw（uint8_t；成员 ibv_gid）：协议地址/union的原始字节视图，GID时为完整128位内容。
 */
	uint8_t			raw[16];
/* 中文逐项-变量 V2301：global
 * global（struct { uint64_t subnet_prefix; uint64_t interface_id; }；成员 ibv_gid）：GID的全局字段视图或全局路由头/属性，和裸16字节表示复用同一数据。
 */
/* 中文逐项-类型 T0210：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：global、subnet_prefix、interface_id。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
	struct {
/* 中文逐项-变量 V2302：subnet_prefix
 * subnet_prefix（uint64_t；成员 ibv_gid::<匿名>）：128位GID的子网前缀和接口标识两个64位组成部分。
 */
		uint64_t	subnet_prefix;
/* 中文逐项-变量 V2303：interface_id
 * interface_id（uint64_t；成员 ibv_gid::<匿名>）：128位GID的子网前缀和接口标识两个64位组成部分。
 */
		uint64_t	interface_id;
	} global;
};

#ifndef container_of
/**
  * container_of - cast a member of a structure out to the containing structure
  * @ptr:        the pointer to the member.
  * @type:       the type of the container struct this is embedded in.
  * @member:     the name of the member within the struct.
  *
 */
/* 中文逐项-宏 M0092：container_of
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 container_of；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define container_of(ptr, type, member) \
	((type *) ((uint8_t *)(ptr) - offsetof(type, member)))
#endif

/* 中文逐项-宏 M0093：vext_field_avail
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 vext_field_avail；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define vext_field_avail(type, fld, sz) (offsetof(type, fld) < (sz))

/*XXX:__VERBS_ABI_IS_EXTENDED produces warning "integer operation result is out of range" with g++ 4.8.2*/
//static void *__VERBS_ABI_IS_EXTENDED = ((uint8_t *)NULL) - 1;

/* 中文逐项-类型 T0211：ibv_node_type
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_node_type {
/* 中文逐项-枚举项 X0335：IBV_NODE_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：-1
 */
	IBV_NODE_UNKNOWN	= -1,
/* 中文逐项-枚举项 X0336：IBV_NODE_CA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
	IBV_NODE_CA 		= 1,
/* 中文逐项-枚举项 X0337：IBV_NODE_SWITCH
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_NODE_SWITCH,
/* 中文逐项-枚举项 X0338：IBV_NODE_ROUTER
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_NODE_ROUTER,
/* 中文逐项-枚举项 X0339：IBV_NODE_RNIC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_NODE_RNIC,

	/* Leave a gap for future node types before starting with
	 * experimental node types.
	 */
/* 中文逐项-枚举项 X0340：IBV_EXP_NODE_TYPE_START
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：32
 */
	IBV_EXP_NODE_TYPE_START	= 32,
/* 中文逐项-枚举项 X0341：IBV_EXP_NODE_MIC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：IBV_EXP_NODE_TYPE_START
 */
	IBV_EXP_NODE_MIC	= IBV_EXP_NODE_TYPE_START
};

/* 中文逐项-类型 T0212：ibv_transport_type
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_transport_type {
/* 中文逐项-枚举项 X0342：IBV_TRANSPORT_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：-1
 */
	IBV_TRANSPORT_UNKNOWN	= -1,
/* 中文逐项-枚举项 X0343：IBV_TRANSPORT_IB
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
	IBV_TRANSPORT_IB	= 0,
/* 中文逐项-枚举项 X0344：IBV_TRANSPORT_IWARP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_TRANSPORT_IWARP,

	/* Leave a gap for future transport types before starting with
	 * experimental transport types.
	 */
/* 中文逐项-枚举项 X0345：IBV_EXP_TRANSPORT_TYPE_START
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：32
 */
	IBV_EXP_TRANSPORT_TYPE_START	= 32,
/* 中文逐项-枚举项 X0346：IBV_EXP_TRANSPORT_SCIF
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：IBV_EXP_TRANSPORT_TYPE_START
 */
	IBV_EXP_TRANSPORT_SCIF		= IBV_EXP_TRANSPORT_TYPE_START
};

/* 中文逐项-类型 T0213：ibv_device_cap_flags
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_device_cap_flags {
/* 中文逐项-枚举项 X0347：IBV_DEVICE_RESIZE_MAX_WR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
	IBV_DEVICE_RESIZE_MAX_WR	= 1,
/* 中文逐项-枚举项 X0348：IBV_DEVICE_BAD_PKEY_CNTR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_DEVICE_BAD_PKEY_CNTR	= 1 <<  1,
/* 中文逐项-枚举项 X0349：IBV_DEVICE_BAD_QKEY_CNTR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 2
 */
	IBV_DEVICE_BAD_QKEY_CNTR	= 1 <<  2,
/* 中文逐项-枚举项 X0350：IBV_DEVICE_RAW_MULTI
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 3
 */
	IBV_DEVICE_RAW_MULTI		= 1 <<  3,
/* 中文逐项-枚举项 X0351：IBV_DEVICE_AUTO_PATH_MIG
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 4
 */
	IBV_DEVICE_AUTO_PATH_MIG	= 1 <<  4,
/* 中文逐项-枚举项 X0352：IBV_DEVICE_CHANGE_PHY_PORT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 5
 */
	IBV_DEVICE_CHANGE_PHY_PORT	= 1 <<  5,
/* 中文逐项-枚举项 X0353：IBV_DEVICE_UD_AV_PORT_ENFORCE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 6
 */
	IBV_DEVICE_UD_AV_PORT_ENFORCE	= 1 <<  6,
/* 中文逐项-枚举项 X0354：IBV_DEVICE_CURR_QP_STATE_MOD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 7
 */
	IBV_DEVICE_CURR_QP_STATE_MOD	= 1 <<  7,
/* 中文逐项-枚举项 X0355：IBV_DEVICE_SHUTDOWN_PORT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 8
 */
	IBV_DEVICE_SHUTDOWN_PORT	= 1 <<  8,
/* 中文逐项-枚举项 X0356：IBV_DEVICE_INIT_TYPE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 9
 */
	IBV_DEVICE_INIT_TYPE		= 1 <<  9,
/* 中文逐项-枚举项 X0357：IBV_DEVICE_PORT_ACTIVE_EVENT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 10
 */
	IBV_DEVICE_PORT_ACTIVE_EVENT	= 1 << 10,
/* 中文逐项-枚举项 X0358：IBV_DEVICE_SYS_IMAGE_GUID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 11
 */
	IBV_DEVICE_SYS_IMAGE_GUID	= 1 << 11,
/* 中文逐项-枚举项 X0359：IBV_DEVICE_RC_RNR_NAK_GEN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 12
 */
	IBV_DEVICE_RC_RNR_NAK_GEN	= 1 << 12,
/* 中文逐项-枚举项 X0360：IBV_DEVICE_SRQ_RESIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 13
 */
	IBV_DEVICE_SRQ_RESIZE		= 1 << 13,
/* 中文逐项-枚举项 X0361：IBV_DEVICE_N_NOTIFY_CQ
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 14
 */
	IBV_DEVICE_N_NOTIFY_CQ		= 1 << 14,
/* 中文逐项-枚举项 X0362：IBV_DEVICE_XRC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 20
 */
	IBV_DEVICE_XRC			= 1 << 20,
/* 中文逐项-枚举项 X0363：IBV_DEVICE_MANAGED_FLOW_STEERING
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 29
 */
	IBV_DEVICE_MANAGED_FLOW_STEERING = 1 << 29
};

/* 中文逐项-类型 T0214：ibv_atomic_cap
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_atomic_cap {
/* 中文逐项-枚举项 X0364：IBV_ATOMIC_NONE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_ATOMIC_NONE,
/* 中文逐项-枚举项 X0365：IBV_ATOMIC_HCA
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_ATOMIC_HCA,
/* 中文逐项-枚举项 X0366：IBV_ATOMIC_GLOB
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_ATOMIC_GLOB
};

/* 中文逐项-类型 T0215：ibv_device_attr
 * 类型职责：HCA能力上限报告，包括QP/MR/CQ/SRQ容量与原子操作支持，不是已分配资源计数；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：fw_ver、node_guid、sys_image_guid、max_mr_size、page_size_cap、vendor_id、vendor_part_id、hw_ver、max_qp、max_qp_wr、device_cap_flags、max_sge、max_sge_rd、max_cq、max_cqe、max_mr、max_pd、max_qp_rd_atom、max_ee_rd_atom、max_res_rd_atom、max_qp_init_rd_atom、max_ee_init_rd_atom、atomic_cap、max_ee、max_rdd、max_mw、max_raw_ipv6_qp、max_raw_ethy_qp、max_mcast_grp、max_mcast_qp_attach、max_total_mcast_qp_attach、max_ah、max_fmr、max_map_per_fmr、max_srq、max_srq_wr、max_srq_sge、max_pkeys、local_ca_ack_delay、phys_port_cnt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_device_attr {
/* 中文逐项-变量 V2304：fw_ver
 * fw_ver（char；成员 ibv_device_attr）：HCA固件/节点/系统GUID与厂商硬件标识，属于能力枚举元信息。
 */
	char			fw_ver[64];
/* 中文逐项-变量 V2305：node_guid
 * node_guid（uint64_t；成员 ibv_device_attr）：HCA固件/节点/系统GUID与厂商硬件标识，属于能力枚举元信息。
 */
	uint64_t		node_guid;
/* 中文逐项-变量 V2306：sys_image_guid
 * sys_image_guid（uint64_t；成员 ibv_device_attr）：HCA固件/节点/系统GUID与厂商硬件标识，属于能力枚举元信息。
 */
	uint64_t		sys_image_guid;
/* 中文逐项-变量 V2307：max_mr_size
 * max_mr_size（uint64_t；成员 ibv_device_attr）：单MR最大可注册字节范围及支持的页尺寸位图，不等于可同时存在MR的条目数。
 */
	uint64_t		max_mr_size;
/* 中文逐项-变量 V2308：page_size_cap
 * page_size_cap（uint64_t；成员 ibv_device_attr）：单MR最大可注册字节范围及支持的页尺寸位图，不等于可同时存在MR的条目数。
 */
	uint64_t		page_size_cap;
/* 中文逐项-变量 V2309：vendor_id
 * vendor_id（uint32_t；成员 ibv_device_attr）：HCA固件/节点/系统GUID与厂商硬件标识，属于能力枚举元信息。
 */
	uint32_t		vendor_id;
/* 中文逐项-变量 V2310：vendor_part_id
 * vendor_part_id（uint32_t；成员 ibv_device_attr）：HCA固件/节点/系统GUID与厂商硬件标识，属于能力枚举元信息。
 */
	uint32_t		vendor_part_id;
/* 中文逐项-变量 V2311：hw_ver
 * hw_ver（uint32_t；成员 ibv_device_attr）：HCA固件/节点/系统GUID与厂商硬件标识，属于能力枚举元信息。
 */
	uint32_t		hw_ver;
/* 中文逐项-变量 V2312：max_qp
 * max_qp（int；成员 ibv_device_attr）：max并发QP/MR/CQ/PD/SRQ/MW/AH/FMR对象数量能力上限，区别于当前已分配数。
 */
	int			max_qp;
/* 中文逐项-变量 V2313：max_qp_wr
 * max_qp_wr（int；成员 ibv_device_attr）：单QP队列或SRQ所支持的最大工作请求条目容量，单位是WQE条目。
 */
	int			max_qp_wr;
/* 中文逐项-变量 V2314：device_cap_flags
 * device_cap_flags（int；成员 ibv_device_attr）：端口/设备能力位掩码，用于检测支持的verbs功能。
 */
	int			device_cap_flags;
/* 中文逐项-变量 V2315：max_sge
 * max_sge（int；成员 ibv_device_attr）：max发送/接收单请求允许的scatter/gather条目数。
 */
	int			max_sge;
/* 中文逐项-变量 V2316：max_sge_rd
 * max_sge_rd（int；成员 ibv_device_attr）：RDMA读或SRQ单条工作请求支持的SGE最大数量。
 */
	int			max_sge_rd;
/* 中文逐项-变量 V2317：max_cq
 * max_cq（int；成员 ibv_device_attr）：max并发QP/MR/CQ/PD/SRQ/MW/AH/FMR对象数量能力上限，区别于当前已分配数。
 */
	int			max_cq;
/* 中文逐项-变量 V2318：max_cqe
 * max_cqe（int；成员 ibv_device_attr）：CQ最大条目能力或当前CQ容量，需容纳未被poll消耗的完成项。
 */
	int			max_cqe;
/* 中文逐项-变量 V2319：max_mr
 * max_mr（int；成员 ibv_device_attr）：max并发QP/MR/CQ/PD/SRQ/MW/AH/FMR对象数量能力上限，区别于当前已分配数。
 */
	int			max_mr;
/* 中文逐项-变量 V2320：max_pd
 * max_pd（int；成员 ibv_device_attr）：max并发QP/MR/CQ/PD/SRQ/MW/AH/FMR对象数量能力上限，区别于当前已分配数。
 */
	int			max_pd;
/* 中文逐项-变量 V2321：max_qp_rd_atom
 * max_qp_rd_atom（int；成员 ibv_device_attr）：HCA可支持的RDMA读/原子未完成请求容量，分别对应QP/EE/总资源和发起方向能力。
 */
	int			max_qp_rd_atom;
/* 中文逐项-变量 V2322：max_ee_rd_atom
 * max_ee_rd_atom（int；成员 ibv_device_attr）：HCA可支持的RDMA读/原子未完成请求容量，分别对应QP/EE/总资源和发起方向能力。
 */
	int			max_ee_rd_atom;
/* 中文逐项-变量 V2323：max_res_rd_atom
 * max_res_rd_atom（int；成员 ibv_device_attr）：HCA可支持的RDMA读/原子未完成请求容量，分别对应QP/EE/总资源和发起方向能力。
 */
	int			max_res_rd_atom;
/* 中文逐项-变量 V2324：max_qp_init_rd_atom
 * max_qp_init_rd_atom（int；成员 ibv_device_attr）：HCA可支持的RDMA读/原子未完成请求容量，分别对应QP/EE/总资源和发起方向能力。
 */
	int			max_qp_init_rd_atom;
/* 中文逐项-变量 V2325：max_ee_init_rd_atom
 * max_ee_init_rd_atom（int；成员 ibv_device_attr）：HCA可支持的RDMA读/原子未完成请求容量，分别对应QP/EE/总资源和发起方向能力。
 */
	int			max_ee_init_rd_atom;
/* 中文逐项-变量 V2326：atomic_cap
 * atomic_cap（enum ibv_atomic_cap；成员 ibv_device_attr）：设备原子操作能力级别，不表示支持NCCL张量级硬件AllReduce。
 */
	enum ibv_atomic_cap	atomic_cap;
/* 中文逐项-变量 V2327：max_ee
 * max_ee（int；成员 ibv_device_attr）：扩展端到端上下文与可靠数据报域容量，普通RC路径不据此创建这些对象。
 */
	int			max_ee;
/* 中文逐项-变量 V2328：max_rdd
 * max_rdd（int；成员 ibv_device_attr）：扩展端到端上下文与可靠数据报域容量，普通RC路径不据此创建这些对象。
 */
	int			max_rdd;
/* 中文逐项-变量 V2329：max_mw
 * max_mw（int；成员 ibv_device_attr）：max并发QP/MR/CQ/PD/SRQ/MW/AH/FMR对象数量能力上限，区别于当前已分配数。
 */
	int			max_mw;
/* 中文逐项-变量 V2330：max_raw_ipv6_qp
 * max_raw_ipv6_qp（int；成员 ibv_device_attr）：原始IPv6/以太网类型QP能力上限，不是普通RC数据QP配额。
 */
	int			max_raw_ipv6_qp;
/* 中文逐项-变量 V2331：max_raw_ethy_qp
 * max_raw_ethy_qp（int；成员 ibv_device_attr）：原始IPv6/以太网类型QP能力上限，不是普通RC数据QP配额。
 */
	int			max_raw_ethy_qp;
/* 中文逐项-变量 V2332：max_mcast_grp
 * max_mcast_grp（int；成员 ibv_device_attr）：多播组数量、每组QP附着数和总附着数能力。
 */
	int			max_mcast_grp;
/* 中文逐项-变量 V2333：max_mcast_qp_attach
 * max_mcast_qp_attach（int；成员 ibv_device_attr）：多播组数量、每组QP附着数和总附着数能力。
 */
	int			max_mcast_qp_attach;
/* 中文逐项-变量 V2334：max_total_mcast_qp_attach
 * max_total_mcast_qp_attach（int；成员 ibv_device_attr）：多播组数量、每组QP附着数和总附着数能力。
 */
	int			max_total_mcast_qp_attach;
/* 中文逐项-变量 V2335：max_ah
 * max_ah（int；成员 ibv_device_attr）：max并发QP/MR/CQ/PD/SRQ/MW/AH/FMR对象数量能力上限，区别于当前已分配数。
 */
	int			max_ah;
/* 中文逐项-变量 V2336：max_fmr
 * max_fmr（int；成员 ibv_device_attr）：max并发QP/MR/CQ/PD/SRQ/MW/AH/FMR对象数量能力上限，区别于当前已分配数。
 */
	int			max_fmr;
/* 中文逐项-变量 V2337：max_map_per_fmr
 * max_map_per_fmr（int；成员 ibv_device_attr）：每个快速内存注册FMR支持的最大映射次数/容量限制，区别于普通MR缓存引用。
 */
	int			max_map_per_fmr;
/* 中文逐项-变量 V2338：max_srq
 * max_srq（int；成员 ibv_device_attr）：max并发QP/MR/CQ/PD/SRQ/MW/AH/FMR对象数量能力上限，区别于当前已分配数。
 */
	int			max_srq;
/* 中文逐项-变量 V2339：max_srq_wr
 * max_srq_wr（int；成员 ibv_device_attr）：单QP队列或SRQ所支持的最大工作请求条目容量，单位是WQE条目。
 */
	int			max_srq_wr;
/* 中文逐项-变量 V2340：max_srq_sge
 * max_srq_sge（int；成员 ibv_device_attr）：RDMA读或SRQ单条工作请求支持的SGE最大数量。
 */
	int			max_srq_sge;
/* 中文逐项-变量 V2341：max_pkeys
 * max_pkeys（uint16_t；成员 ibv_device_attr）：设备最大PKey数量及当前端口PKey表长度，参与IB分区隔离。
 */
	uint16_t		max_pkeys;
/* 中文逐项-变量 V2342：local_ca_ack_delay
 * local_ca_ack_delay（uint8_t；成员 ibv_device_attr）：设备本地确认延迟编码，RC超时建模的一部分。
 */
	uint8_t			local_ca_ack_delay;
/* 中文逐项-变量 V2343：phys_port_cnt
 * phys_port_cnt（uint8_t；成员 ibv_device_attr）：设备物理端口数量，不等于NCCL可见且筛选后的NET设备数。
 */
	uint8_t			phys_port_cnt;
};

/* 中文逐项-类型 T0216：ibv_mtu
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_mtu {
/* 中文逐项-枚举项 X0367：IBV_MTU_256
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
	IBV_MTU_256  = 1,
/* 中文逐项-枚举项 X0368：IBV_MTU_512
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
	IBV_MTU_512  = 2,
/* 中文逐项-枚举项 X0369：IBV_MTU_1024
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
	IBV_MTU_1024 = 3,
/* 中文逐项-枚举项 X0370：IBV_MTU_2048
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
	IBV_MTU_2048 = 4,
/* 中文逐项-枚举项 X0371：IBV_MTU_4096
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
	IBV_MTU_4096 = 5
};

/* 中文逐项-类型 T0217：ibv_port_state
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_port_state {
/* 中文逐项-枚举项 X0372：IBV_PORT_NOP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
	IBV_PORT_NOP		= 0,
/* 中文逐项-枚举项 X0373：IBV_PORT_DOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
	IBV_PORT_DOWN		= 1,
/* 中文逐项-枚举项 X0374：IBV_PORT_INIT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
	IBV_PORT_INIT		= 2,
/* 中文逐项-枚举项 X0375：IBV_PORT_ARMED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
	IBV_PORT_ARMED		= 3,
/* 中文逐项-枚举项 X0376：IBV_PORT_ACTIVE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
	IBV_PORT_ACTIVE		= 4,
/* 中文逐项-枚举项 X0377：IBV_PORT_ACTIVE_DEFER
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
	IBV_PORT_ACTIVE_DEFER	= 5
};

/* 中文逐项-类型 T0218：<匿名enum>
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum {
/* 中文逐项-枚举项 X0378：IBV_LINK_LAYER_UNSPECIFIED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_LINK_LAYER_UNSPECIFIED,
/* 中文逐项-枚举项 X0379：IBV_LINK_LAYER_INFINIBAND
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_LINK_LAYER_INFINIBAND,
/* 中文逐项-枚举项 X0380：IBV_LINK_LAYER_ETHERNET
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_LINK_LAYER_ETHERNET,

	/* Leave a gap for future link layer types before starting with
	 * experimental link layer.
	 */
/* 中文逐项-枚举项 X0381：IBV_EXP_LINK_LAYER_START
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：32
 */
	IBV_EXP_LINK_LAYER_START	= 32,
/* 中文逐项-枚举项 X0382：IBV_EXP_LINK_LAYER_SCIF
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：IBV_EXP_LINK_LAYER_START
 */
	IBV_EXP_LINK_LAYER_SCIF		= IBV_EXP_LINK_LAYER_START
};

/* 中文逐项-类型 T0219：ibv_port_cap_flags
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_port_cap_flags {
/* 中文逐项-枚举项 X0383：IBV_PORT_SM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_PORT_SM				= 1 <<  1,
/* 中文逐项-枚举项 X0384：IBV_PORT_NOTICE_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 2
 */
	IBV_PORT_NOTICE_SUP			= 1 <<  2,
/* 中文逐项-枚举项 X0385：IBV_PORT_TRAP_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 3
 */
	IBV_PORT_TRAP_SUP			= 1 <<  3,
/* 中文逐项-枚举项 X0386：IBV_PORT_OPT_IPD_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 4
 */
	IBV_PORT_OPT_IPD_SUP			= 1 <<  4,
/* 中文逐项-枚举项 X0387：IBV_PORT_AUTO_MIGR_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 5
 */
	IBV_PORT_AUTO_MIGR_SUP			= 1 <<  5,
/* 中文逐项-枚举项 X0388：IBV_PORT_SL_MAP_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 6
 */
	IBV_PORT_SL_MAP_SUP			= 1 <<  6,
/* 中文逐项-枚举项 X0389：IBV_PORT_MKEY_NVRAM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 7
 */
	IBV_PORT_MKEY_NVRAM			= 1 <<  7,
/* 中文逐项-枚举项 X0390：IBV_PORT_PKEY_NVRAM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 8
 */
	IBV_PORT_PKEY_NVRAM			= 1 <<  8,
/* 中文逐项-枚举项 X0391：IBV_PORT_LED_INFO_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 9
 */
	IBV_PORT_LED_INFO_SUP			= 1 <<  9,
/* 中文逐项-枚举项 X0392：IBV_PORT_SYS_IMAGE_GUID_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 11
 */
	IBV_PORT_SYS_IMAGE_GUID_SUP		= 1 << 11,
/* 中文逐项-枚举项 X0393：IBV_PORT_PKEY_SW_EXT_PORT_TRAP_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 12
 */
	IBV_PORT_PKEY_SW_EXT_PORT_TRAP_SUP	= 1 << 12,
/* 中文逐项-枚举项 X0394：IBV_PORT_EXTENDED_SPEEDS_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 14
 */
	IBV_PORT_EXTENDED_SPEEDS_SUP		= 1 << 14,
/* 中文逐项-枚举项 X0395：IBV_PORT_CM_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 16
 */
	IBV_PORT_CM_SUP				= 1 << 16,
/* 中文逐项-枚举项 X0396：IBV_PORT_SNMP_TUNNEL_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 17
 */
	IBV_PORT_SNMP_TUNNEL_SUP		= 1 << 17,
/* 中文逐项-枚举项 X0397：IBV_PORT_REINIT_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 18
 */
	IBV_PORT_REINIT_SUP			= 1 << 18,
/* 中文逐项-枚举项 X0398：IBV_PORT_DEVICE_MGMT_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 19
 */
	IBV_PORT_DEVICE_MGMT_SUP		= 1 << 19,
/* 中文逐项-枚举项 X0399：IBV_PORT_VENDOR_CLASS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 24
 */
	IBV_PORT_VENDOR_CLASS			= 1 << 24,
/* 中文逐项-枚举项 X0400：IBV_PORT_CLIENT_REG_SUP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 25
 */
	IBV_PORT_CLIENT_REG_SUP			= 1 << 25,
/* 中文逐项-枚举项 X0401：IBV_PORT_IP_BASED_GIDS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 26
 */
	IBV_PORT_IP_BASED_GIDS			= 1 << 26,
};

/* 中文逐项-类型 T0220：ibv_port_attr
 * 类型职责：端口状态、LID、MTU、速率、GID表等能力报告，供建链参数选择；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：state、max_mtu、active_mtu、gid_tbl_len、port_cap_flags、max_msg_sz、bad_pkey_cntr、qkey_viol_cntr、pkey_tbl_len、lid、sm_lid、lmc、max_vl_num、sm_sl、subnet_timeout、init_type_reply、active_width、active_speed、phys_state、link_layer、reserved。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_port_attr {
/* 中文逐项-变量 V2344：state
 * state（enum ibv_port_state；成员 ibv_port_attr）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 */
	enum ibv_port_state	state;
/* 中文逐项-变量 V2345：max_mtu
 * max_mtu（enum ibv_mtu；成员 ibv_port_attr）：端口最大支持和当前激活的MTU枚举，连接路径MTU不得超过可用值。
 */
	enum ibv_mtu		max_mtu;
/* 中文逐项-变量 V2346：active_mtu
 * active_mtu（enum ibv_mtu；成员 ibv_port_attr）：端口最大支持和当前激活的MTU枚举，连接路径MTU不得超过可用值。
 */
	enum ibv_mtu		active_mtu;
/* 中文逐项-变量 V2347：gid_tbl_len
 * gid_tbl_len（int；成员 ibv_port_attr）：端口GID表条目数，选择GID索引前需确保在有效范围。
 */
	int			gid_tbl_len;
/* 中文逐项-变量 V2348：port_cap_flags
 * port_cap_flags（uint32_t；成员 ibv_port_attr）：端口/设备能力位掩码，用于检测支持的verbs功能。
 */
	uint32_t		port_cap_flags;
/* 中文逐项-变量 V2349：max_msg_sz
 * max_msg_sz（uint32_t；成员 ibv_port_attr）：端口支持的最大消息字节数，NCCL大张量仍会经过算法和协议分片。
 */
	uint32_t		max_msg_sz;
/* 中文逐项-变量 V2350：bad_pkey_cntr
 * bad_pkey_cntr（uint32_t；成员 ibv_port_attr）：端口分区key或队列key违规计数，用于诊断访问/配置错误。
 */
	uint32_t		bad_pkey_cntr;
/* 中文逐项-变量 V2351：qkey_viol_cntr
 * qkey_viol_cntr（uint32_t；成员 ibv_port_attr）：端口分区key或队列key违规计数，用于诊断访问/配置错误。
 */
	uint32_t		qkey_viol_cntr;
/* 中文逐项-变量 V2352：pkey_tbl_len
 * pkey_tbl_len（uint16_t；成员 ibv_port_attr）：设备最大PKey数量及当前端口PKey表长度，参与IB分区隔离。
 */
	uint16_t		pkey_tbl_len;
/* 中文逐项-变量 V2353：lid
 * lid（uint16_t；成员 ibv_port_attr）：IB本地/目标/子网管理器/源LID地址信息，RoCE全球路由通常改用GID路径。
 */
	uint16_t		lid;
/* 中文逐项-变量 V2354：sm_lid
 * sm_lid（uint16_t；成员 ibv_port_attr）：IB本地/目标/子网管理器/源LID地址信息，RoCE全球路由通常改用GID路径。
 */
	uint16_t		sm_lid;
/* 中文逐项-变量 V2355：lmc
 * lmc（uint8_t；成员 ibv_port_attr）：IB LID掩码控制与支持虚拟通道数量的端口属性。
 */
	uint8_t			lmc;
/* 中文逐项-变量 V2356：max_vl_num
 * max_vl_num（uint8_t；成员 ibv_port_attr）：IB LID掩码控制与支持虚拟通道数量的端口属性。
 */
	uint8_t			max_vl_num;
/* 中文逐项-变量 V2357：sm_sl
 * sm_sl（uint8_t；成员 ibv_port_attr）：IB服务等级及路径位，参与数据包路径/优先级选择。
 */
	uint8_t			sm_sl;
/* 中文逐项-变量 V2358：subnet_timeout
 * subnet_timeout（uint8_t；成员 ibv_port_attr）：IB子网超时编码与初始化类型回复信息，属于端口管理属性。
 */
	uint8_t			subnet_timeout;
/* 中文逐项-变量 V2359：init_type_reply
 * init_type_reply（uint8_t；成员 ibv_port_attr）：IB子网超时编码与初始化类型回复信息，属于端口管理属性。
 */
	uint8_t			init_type_reply;
/* 中文逐项-变量 V2360：active_width
 * active_width（uint8_t；成员 ibv_port_attr）：当前端口实际激活链路宽度/速率的verbs编码，需经ncclIbWidth/Speed换成数值。
 */
	uint8_t			active_width;
/* 中文逐项-变量 V2361：active_speed
 * active_speed（uint8_t；成员 ibv_port_attr）：当前端口实际激活链路宽度/速率的verbs编码，需经ncclIbWidth/Speed换成数值。
 */
	uint8_t			active_speed;
/* 中文逐项-变量 V2362：phys_state
 * phys_state（uint8_t；成员 ibv_port_attr）：端口物理状态，与逻辑port_state共同判断链路是否可用。
 */
	uint8_t			phys_state;
/* 中文逐项-变量 V2363：link_layer
 * link_layer（uint8_t；成员 ibv_port_attr）：端口底层链路类别，如InfiniBand或Ethernet，影响LID/GID和RoCE配置。
 */
	uint8_t			link_layer;
/* 中文逐项-变量 V2364：reserved
 * reserved（uint8_t；成员 ibv_port_attr）：布局保留/填充空间，保证缓存行或固定大小ABI；不承载算法有效数据。
 */
	uint8_t			reserved;
};

/* 中文逐项-类型 T0221：ibv_event_type
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_event_type {
/* 中文逐项-枚举项 X0402：IBV_EVENT_CQ_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_CQ_ERR,
/* 中文逐项-枚举项 X0403：IBV_EVENT_QP_FATAL
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_QP_FATAL,
/* 中文逐项-枚举项 X0404：IBV_EVENT_QP_REQ_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_QP_REQ_ERR,
/* 中文逐项-枚举项 X0405：IBV_EVENT_QP_ACCESS_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_QP_ACCESS_ERR,
/* 中文逐项-枚举项 X0406：IBV_EVENT_COMM_EST
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_COMM_EST,
/* 中文逐项-枚举项 X0407：IBV_EVENT_SQ_DRAINED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_SQ_DRAINED,
/* 中文逐项-枚举项 X0408：IBV_EVENT_PATH_MIG
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_PATH_MIG,
/* 中文逐项-枚举项 X0409：IBV_EVENT_PATH_MIG_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_PATH_MIG_ERR,
/* 中文逐项-枚举项 X0410：IBV_EVENT_DEVICE_FATAL
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_DEVICE_FATAL,
/* 中文逐项-枚举项 X0411：IBV_EVENT_PORT_ACTIVE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_PORT_ACTIVE,
/* 中文逐项-枚举项 X0412：IBV_EVENT_PORT_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_PORT_ERR,
/* 中文逐项-枚举项 X0413：IBV_EVENT_LID_CHANGE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_LID_CHANGE,
/* 中文逐项-枚举项 X0414：IBV_EVENT_PKEY_CHANGE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_PKEY_CHANGE,
/* 中文逐项-枚举项 X0415：IBV_EVENT_SM_CHANGE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_SM_CHANGE,
/* 中文逐项-枚举项 X0416：IBV_EVENT_SRQ_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_SRQ_ERR,
/* 中文逐项-枚举项 X0417：IBV_EVENT_SRQ_LIMIT_REACHED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_SRQ_LIMIT_REACHED,
/* 中文逐项-枚举项 X0418：IBV_EVENT_QP_LAST_WQE_REACHED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_QP_LAST_WQE_REACHED,
/* 中文逐项-枚举项 X0419：IBV_EVENT_CLIENT_REREGISTER
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_CLIENT_REREGISTER,
/* 中文逐项-枚举项 X0420：IBV_EVENT_GID_CHANGE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EVENT_GID_CHANGE,

	/* new experimental events start here leaving enough
	 * room for 14 events which should be enough
	 */
/* 中文逐项-枚举项 X0421：IBV_EXP_EVENT_DCT_KEY_VIOLATION
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：32
 */
	IBV_EXP_EVENT_DCT_KEY_VIOLATION = 32,
/* 中文逐项-枚举项 X0422：IBV_EXP_EVENT_DCT_ACCESS_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EXP_EVENT_DCT_ACCESS_ERR,
/* 中文逐项-枚举项 X0423：IBV_EXP_EVENT_DCT_REQ_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_EXP_EVENT_DCT_REQ_ERR,
};

/* 中文逐项-类型 T0222：ibv_async_event
 * 类型职责：异步设备事件，union按event_type解释关联的CQ/QP/SRQ/端口；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：element、cq、qp、srq、dct、port_num、xrc_qp_num、event_type。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_async_event {
/* 中文逐项-变量 V2365：element
 * element（union { struct ibv_cq *cq; struct ibv_qp *qp; struct ibv_srq *srq; struct ibv_exp_dct *dct; int port_num; /
 * * …（完整表达式见紧邻源码）；成员 ibv_async_event）：异步事件关联对象union与事件类别；必须先按event_type确定哪个union字段有效。
 */
/* 中文逐项-类型 T0223：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：element、cq、qp、srq、dct、port_num、xrc_qp_num。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
	union {
/* 中文逐项-变量 V2366：cq
 * cq（struct ibv_cq；成员 ibv_async_event::<匿名>）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
		struct ibv_cq  *cq;
/* 中文逐项-变量 V2367：qp
 * qp（struct ibv_qp；成员 ibv_async_event::<匿名>）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 */
		struct ibv_qp  *qp;
/* 中文逐项-变量 V2368：srq
 * srq（struct ibv_srq；成员 ibv_async_event::<匿名>）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 */
		struct ibv_srq *srq;
/* 中文逐项-变量 V2369：dct
 * dct（struct ibv_exp_dct；成员 ibv_async_event::<匿名>）：特定DCT/XRC扩展机制关联的对象/编号，仅保留ABI定义不表示本版普通数据路径采用。
 */
		struct ibv_exp_dct *dct;
/* 中文逐项-变量 V2370：port_num
 * port_num（int；成员 ibv_async_event::<匿名>）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
		int		port_num;
		/* For source compatible with Legacy API */
/* 中文逐项-变量 V2371：xrc_qp_num
 * xrc_qp_num（uint32_t；成员 ibv_async_event::<匿名>）：特定DCT/XRC扩展机制关联的对象/编号，仅保留ABI定义不表示本版普通数据路径采用。
 */
		uint32_t	xrc_qp_num;
	} element;
/* 中文逐项-变量 V2372：event_type
 * event_type（enum ibv_event_type；成员 ibv_async_event）：异步事件关联对象union与事件类别；必须先按event_type确定哪个union字段有效。
 */
	enum ibv_event_type	event_type;
};

/* 中文逐项-类型 T0224：ibv_wc_status
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_wc_status {
/* 中文逐项-枚举项 X0424：IBV_WC_SUCCESS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_SUCCESS,
/* 中文逐项-枚举项 X0425：IBV_WC_LOC_LEN_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_LOC_LEN_ERR,
/* 中文逐项-枚举项 X0426：IBV_WC_LOC_QP_OP_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_LOC_QP_OP_ERR,
/* 中文逐项-枚举项 X0427：IBV_WC_LOC_EEC_OP_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_LOC_EEC_OP_ERR,
/* 中文逐项-枚举项 X0428：IBV_WC_LOC_PROT_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_LOC_PROT_ERR,
/* 中文逐项-枚举项 X0429：IBV_WC_WR_FLUSH_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_WR_FLUSH_ERR,
/* 中文逐项-枚举项 X0430：IBV_WC_MW_BIND_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_MW_BIND_ERR,
/* 中文逐项-枚举项 X0431：IBV_WC_BAD_RESP_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_BAD_RESP_ERR,
/* 中文逐项-枚举项 X0432：IBV_WC_LOC_ACCESS_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_LOC_ACCESS_ERR,
/* 中文逐项-枚举项 X0433：IBV_WC_REM_INV_REQ_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_REM_INV_REQ_ERR,
/* 中文逐项-枚举项 X0434：IBV_WC_REM_ACCESS_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_REM_ACCESS_ERR,
/* 中文逐项-枚举项 X0435：IBV_WC_REM_OP_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_REM_OP_ERR,
/* 中文逐项-枚举项 X0436：IBV_WC_RETRY_EXC_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_RETRY_EXC_ERR,
/* 中文逐项-枚举项 X0437：IBV_WC_RNR_RETRY_EXC_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_RNR_RETRY_EXC_ERR,
/* 中文逐项-枚举项 X0438：IBV_WC_LOC_RDD_VIOL_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_LOC_RDD_VIOL_ERR,
/* 中文逐项-枚举项 X0439：IBV_WC_REM_INV_RD_REQ_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_REM_INV_RD_REQ_ERR,
/* 中文逐项-枚举项 X0440：IBV_WC_REM_ABORT_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_REM_ABORT_ERR,
/* 中文逐项-枚举项 X0441：IBV_WC_INV_EECN_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_INV_EECN_ERR,
/* 中文逐项-枚举项 X0442：IBV_WC_INV_EEC_STATE_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_INV_EEC_STATE_ERR,
/* 中文逐项-枚举项 X0443：IBV_WC_FATAL_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_FATAL_ERR,
/* 中文逐项-枚举项 X0444：IBV_WC_RESP_TIMEOUT_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_RESP_TIMEOUT_ERR,
/* 中文逐项-枚举项 X0445：IBV_WC_GENERAL_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_GENERAL_ERR
};
/* 中文逐项-变量 V2373：ibv_wc_status_str
 * ibv_wc_status_str（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
const char *ibv_wc_status_str(enum ibv_wc_status status);

/* 中文逐项-类型 T0225：ibv_wc_opcode
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_wc_opcode {
/* 中文逐项-枚举项 X0446：IBV_WC_SEND
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_SEND,
/* 中文逐项-枚举项 X0447：IBV_WC_RDMA_WRITE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_RDMA_WRITE,
/* 中文逐项-枚举项 X0448：IBV_WC_RDMA_READ
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_RDMA_READ,
/* 中文逐项-枚举项 X0449：IBV_WC_COMP_SWAP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_COMP_SWAP,
/* 中文逐项-枚举项 X0450：IBV_WC_FETCH_ADD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_FETCH_ADD,
/* 中文逐项-枚举项 X0451：IBV_WC_BIND_MW
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_BIND_MW,
/*
 * Set value of IBV_WC_RECV so consumers can test if a completion is a
 * receive by testing (opcode & IBV_WC_RECV).
 */
/* 中文逐项-枚举项 X0452：IBV_WC_RECV
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 7
 */
	IBV_WC_RECV			= 1 << 7,
/* 中文逐项-枚举项 X0453：IBV_WC_RECV_RDMA_WITH_IMM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WC_RECV_RDMA_WITH_IMM
};

/* 中文逐项-类型 T0226：ibv_wc_flags
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_wc_flags {
/* 中文逐项-枚举项 X0454：IBV_WC_GRH
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 0
 */
	IBV_WC_GRH		= 1 << 0,
/* 中文逐项-枚举项 X0455：IBV_WC_WITH_IMM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_WC_WITH_IMM		= 1 << 1
};

/* 中文逐项-类型 T0227：ibv_wc
 * 类型职责：硬件完成项，按status/opcode/wr_id解释长度/立即数并找回所属请求；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：wr_id、status、opcode、vendor_err、byte_len、imm_data、qp_num、src_qp、wc_flags、pkey_index、slid、sl、dlid_path_bits。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_wc {
/* 中文逐项-变量 V2374：wr_id
 * wr_id（uint64_t；成员 ibv_wc）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 */
	uint64_t		wr_id;
/* 中文逐项-变量 V2375：status
 * status（enum ibv_wc_status；成员 ibv_wc）：完成/调用的通用状态和厂商错误细节，非成功不能把目标数据当作有效结果。
 */
	enum ibv_wc_status	status;
/* 中文逐项-变量 V2376：opcode
 * opcode（enum ibv_wc_opcode；成员 ibv_wc）：verbs 请求/完成的操作码，区分 RDMA_WRITE、WRITE_WITH_IMM、READ 等。
 */
	enum ibv_wc_opcode	opcode;
/* 中文逐项-变量 V2377：vendor_err
 * vendor_err（uint32_t；成员 ibv_wc）：完成/调用的通用状态和厂商错误细节，非成功不能把目标数据当作有效结果。
 */
	uint32_t		vendor_err;
/* 中文逐项-变量 V2378：byte_len
 * byte_len（uint32_t；成员 ibv_wc）：完成项报告的长度或32位立即数；WRITE_WITH_IMM立即数用于控制通知，具体编码由后端约定。
 */
	uint32_t		byte_len;
/* 中文逐项-变量 V2379：imm_data
 * imm_data（uint32_t；成员 ibv_wc）：完成项报告的长度或32位立即数；WRITE_WITH_IMM立即数用于控制通知，具体编码由后端约定。
 */
	uint32_t		imm_data;	/* in network byte order */
/* 中文逐项-变量 V2380：qp_num
 * qp_num（uint32_t；成员 ibv_wc）：本地/远端QP编号，用于控制握手及连接目标匹配。
 */
	uint32_t		qp_num;
/* 中文逐项-变量 V2381：src_qp
 * src_qp（uint32_t；成员 ibv_wc）：完成项中的来源QP编号，主要供UD等接收识别来源。
 */
	uint32_t		src_qp;
/* 中文逐项-变量 V2382：wc_flags
 * wc_flags（int；成员 ibv_wc）：完成项附加标志，如是否含立即数、全局路由头等，结合opcode解释。
 */
	int			wc_flags;
/* 中文逐项-变量 V2383：pkey_index
 * pkey_index（uint16_t；成员 ibv_wc）：主/备用路径使用的分区key表索引，参与IB分区隔离。
 */
	uint16_t		pkey_index;
/* 中文逐项-变量 V2384：slid
 * slid（uint16_t；成员 ibv_wc）：IB本地/目标/子网管理器/源LID地址信息，RoCE全球路由通常改用GID路径。
 */
	uint16_t		slid;
/* 中文逐项-变量 V2385：sl
 * sl（uint8_t；成员 ibv_wc）：IB服务等级及路径位，参与数据包路径/优先级选择。
 */
	uint8_t			sl;
/* 中文逐项-变量 V2386：dlid_path_bits
 * dlid_path_bits（uint8_t；成员 ibv_wc）：IB服务等级及路径位，参与数据包路径/优先级选择。
 */
	uint8_t			dlid_path_bits;
};

/* 中文逐项-类型 T0228：ibv_access_flags
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_access_flags {
/* 中文逐项-枚举项 X0456：IBV_ACCESS_LOCAL_WRITE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
	IBV_ACCESS_LOCAL_WRITE		= 1,
/* 中文逐项-枚举项 X0457：IBV_ACCESS_REMOTE_WRITE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1<<1)
 */
	IBV_ACCESS_REMOTE_WRITE		= (1<<1),
/* 中文逐项-枚举项 X0458：IBV_ACCESS_REMOTE_READ
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1<<2)
 */
	IBV_ACCESS_REMOTE_READ		= (1<<2),
/* 中文逐项-枚举项 X0459：IBV_ACCESS_REMOTE_ATOMIC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1<<3)
 */
	IBV_ACCESS_REMOTE_ATOMIC	= (1<<3),
/* 中文逐项-枚举项 X0460：IBV_ACCESS_MW_BIND
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1<<4)
 */
	IBV_ACCESS_MW_BIND		= (1<<4),
/* 中文逐项-枚举项 X0461：IBV_ACCESS_RELAXED_ORDERING
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1<<20)
 */
	IBV_ACCESS_RELAXED_ORDERING     = (1<<20),
};

/* 中文逐项-类型 T0229：ibv_pd
 * 类型职责：保护域句柄，隔离其关联的QP、MR等资源；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：context、handle。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_pd {
/* 中文逐项-变量 V2387：context
 * context（struct ibv_context；成员 ibv_pd）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
/* 中文逐项-变量 V2388：handle
 * handle（uint32_t；成员 ibv_pd）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 */
	uint32_t		handle;
};

/* 中文逐项-类型 T0230：ibv_xrcd_init_attr_mask
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_xrcd_init_attr_mask {
/* 中文逐项-枚举项 X0462：IBV_XRCD_INIT_ATTR_FD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 0
 */
	IBV_XRCD_INIT_ATTR_FD	    = 1 << 0,
/* 中文逐项-枚举项 X0463：IBV_XRCD_INIT_ATTR_OFLAGS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_XRCD_INIT_ATTR_OFLAGS   = 1 << 1,
/* 中文逐项-枚举项 X0464：IBV_XRCD_INIT_ATTR_RESERVED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 2
 */
	IBV_XRCD_INIT_ATTR_RESERVED = 1 << 2
};

/* 中文逐项-类型 T0231：ibv_xrcd_init_attr
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：comp_mask、fd、oflags。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_xrcd_init_attr {
/* 中文逐项-变量 V2389：comp_mask
 * comp_mask（uint32_t；成员 ibv_xrcd_init_attr）：结构体可选扩展字段有效性掩码，调用者只允许使用当前ABI支持的字段。
 */
	uint32_t comp_mask;
/* 中文逐项-变量 V2390：fd
 * fd（int；成员 ibv_xrcd_init_attr）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 */
	int	 fd;
/* 中文逐项-变量 V2391：oflags
 * oflags（int；成员 ibv_xrcd_init_attr）：打开/创建扩展资源的系统标志，按对应verbs接口解释。
 */
	int	 oflags;
};

/* 中文逐项-类型 T0232：ibv_xrcd
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：context。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_xrcd {
/* 中文逐项-变量 V2392：context
 * context（struct ibv_context；成员 ibv_xrcd）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
};

/* 中文逐项-类型 T0233：ibv_rereg_mr_flags
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_rereg_mr_flags {
/* 中文逐项-枚举项 X0465：IBV_REREG_MR_CHANGE_TRANSLATION
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1 << 0)
 */
	IBV_REREG_MR_CHANGE_TRANSLATION	= (1 << 0),
/* 中文逐项-枚举项 X0466：IBV_REREG_MR_CHANGE_PD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1 << 1)
 */
	IBV_REREG_MR_CHANGE_PD		= (1 << 1),
/* 中文逐项-枚举项 X0467：IBV_REREG_MR_CHANGE_ACCESS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1 << 2)
 */
	IBV_REREG_MR_CHANGE_ACCESS	= (1 << 2),
/* 中文逐项-枚举项 X0468：IBV_REREG_MR_KEEP_VALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(1 << 3)
 */
	IBV_REREG_MR_KEEP_VALID		= (1 << 3)
};

/* 中文逐项-类型 T0234：ibv_mr
 * 类型职责：内存注册描述，绑定PD、地址范围及lkey/rkey，使用结束须注销；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：context、pd、addr、length、handle、lkey、rkey。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_mr {
/* 中文逐项-变量 V2393：context
 * context（struct ibv_context；成员 ibv_mr）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
/* 中文逐项-变量 V2394：pd
 * pd（struct ibv_pd；成员 ibv_mr）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
	struct ibv_pd	       *pd;
/* 中文逐项-变量 V2395：addr
 * addr（void；成员 ibv_mr）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
	void		       *addr;
/* 中文逐项-变量 V2396：length
 * length（size_t；成员 ibv_mr）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
	size_t			length;
/* 中文逐项-变量 V2397：handle
 * handle（uint32_t；成员 ibv_mr）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 */
	uint32_t		handle;
/* 中文逐项-变量 V2398：lkey
 * lkey（uint32_t；成员 ibv_mr）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
	uint32_t		lkey;
/* 中文逐项-变量 V2399：rkey
 * rkey（uint32_t；成员 ibv_mr）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
	uint32_t		rkey;
};

/* 中文逐项-类型 T0235：ibv_mw_type
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_mw_type {
/* 中文逐项-枚举项 X0469：IBV_MW_TYPE_1
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
	IBV_MW_TYPE_1			= 1,
/* 中文逐项-枚举项 X0470：IBV_MW_TYPE_2
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
	IBV_MW_TYPE_2			= 2
};

/* 中文逐项-类型 T0236：ibv_mw
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：context、pd、rkey。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_mw {
/* 中文逐项-变量 V2400：context
 * context（struct ibv_context；成员 ibv_mw）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
/* 中文逐项-变量 V2401：pd
 * pd（struct ibv_pd；成员 ibv_mw）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
	struct ibv_pd	       *pd;
/* 中文逐项-变量 V2402：rkey
 * rkey（uint32_t；成员 ibv_mw）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
	uint32_t		rkey;
};

/* 中文逐项-类型 T0237：ibv_global_route
 * 类型职责：全局路由/地址句柄属性/路由头，结合LID或GID指定目标路径；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：dgid、flow_label、sgid_index、hop_limit、traffic_class。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_global_route {
/* 中文逐项-变量 V2403：dgid
 * dgid（union ibv_gid；成员 ibv_global_route）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 */
	union ibv_gid		dgid;
/* 中文逐项-变量 V2404：flow_label
 * flow_label（uint32_t；成员 ibv_global_route）：全局路由/IPv6的流标签、业务类别和跳数上限。
 */
	uint32_t		flow_label;
/* 中文逐项-变量 V2405：sgid_index
 * sgid_index（uint8_t；成员 ibv_global_route）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 */
	uint8_t			sgid_index;
/* 中文逐项-变量 V2406：hop_limit
 * hop_limit（uint8_t；成员 ibv_global_route）：全局路由/IPv6的流标签、业务类别和跳数上限。
 */
	uint8_t			hop_limit;
/* 中文逐项-变量 V2407：traffic_class
 * traffic_class（uint8_t；成员 ibv_global_route）：全局路由/IPv6的流标签、业务类别和跳数上限。
 */
	uint8_t			traffic_class;
};

/* 中文逐项-类型 T0238：ibv_grh
 * 类型职责：全局路由/地址句柄属性/路由头，结合LID或GID指定目标路径；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：version_tclass_flow、paylen、next_hdr、hop_limit、sgid、dgid。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_grh {
/* 中文逐项-变量 V2408：version_tclass_flow
 * version_tclass_flow（uint32_t；成员 ibv_grh）：全局路由头的版本/业务类/流标签组合、载荷长度与下一层协议号。
 */
	uint32_t		version_tclass_flow;
/* 中文逐项-变量 V2409：paylen
 * paylen（uint16_t；成员 ibv_grh）：全局路由头的版本/业务类/流标签组合、载荷长度与下一层协议号。
 */
	uint16_t		paylen;
/* 中文逐项-变量 V2410：next_hdr
 * next_hdr（uint8_t；成员 ibv_grh）：全局路由头的版本/业务类/流标签组合、载荷长度与下一层协议号。
 */
	uint8_t			next_hdr;
/* 中文逐项-变量 V2411：hop_limit
 * hop_limit（uint8_t；成员 ibv_grh）：全局路由/IPv6的流标签、业务类别和跳数上限。
 */
	uint8_t			hop_limit;
/* 中文逐项-变量 V2412：sgid
 * sgid（union ibv_gid；成员 ibv_grh）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 */
	union ibv_gid		sgid;
/* 中文逐项-变量 V2413：dgid
 * dgid（union ibv_gid；成员 ibv_grh）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 */
	union ibv_gid		dgid;
};

/* 中文逐项-类型 T0239：ibv_rate
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_rate {
/* 中文逐项-枚举项 X0471：IBV_RATE_MAX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
	IBV_RATE_MAX      = 0,
/* 中文逐项-枚举项 X0472：IBV_RATE_2_5_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
	IBV_RATE_2_5_GBPS = 2,
/* 中文逐项-枚举项 X0473：IBV_RATE_5_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
	IBV_RATE_5_GBPS   = 5,
/* 中文逐项-枚举项 X0474：IBV_RATE_10_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
	IBV_RATE_10_GBPS  = 3,
/* 中文逐项-枚举项 X0475：IBV_RATE_20_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
	IBV_RATE_20_GBPS  = 6,
/* 中文逐项-枚举项 X0476：IBV_RATE_30_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
	IBV_RATE_30_GBPS  = 4,
/* 中文逐项-枚举项 X0477：IBV_RATE_40_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：7
 */
	IBV_RATE_40_GBPS  = 7,
/* 中文逐项-枚举项 X0478：IBV_RATE_60_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
	IBV_RATE_60_GBPS  = 8,
/* 中文逐项-枚举项 X0479：IBV_RATE_80_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：9
 */
	IBV_RATE_80_GBPS  = 9,
/* 中文逐项-枚举项 X0480：IBV_RATE_120_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：10
 */
	IBV_RATE_120_GBPS = 10,
/* 中文逐项-枚举项 X0481：IBV_RATE_14_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：11
 */
	IBV_RATE_14_GBPS  = 11,
/* 中文逐项-枚举项 X0482：IBV_RATE_56_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：12
 */
	IBV_RATE_56_GBPS  = 12,
/* 中文逐项-枚举项 X0483：IBV_RATE_112_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：13
 */
	IBV_RATE_112_GBPS = 13,
/* 中文逐项-枚举项 X0484：IBV_RATE_168_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：14
 */
	IBV_RATE_168_GBPS = 14,
/* 中文逐项-枚举项 X0485：IBV_RATE_25_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：15
 */
	IBV_RATE_25_GBPS  = 15,
/* 中文逐项-枚举项 X0486：IBV_RATE_100_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：16
 */
	IBV_RATE_100_GBPS = 16,
/* 中文逐项-枚举项 X0487：IBV_RATE_200_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：17
 */
	IBV_RATE_200_GBPS = 17,
/* 中文逐项-枚举项 X0488：IBV_RATE_300_GBPS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：18
 */
	IBV_RATE_300_GBPS = 18
};

/**
 * ibv_rate_to_mult - Convert the IB rate enum to a multiple of the
 * base rate of 2.5 Gbit/sec.  For example, IBV_RATE_5_GBPS will be
 * converted to 2, since 5 Gbit/sec is 2 * 2.5 Gbit/sec.
 * @rate: rate to convert.
 */
int ibv_rate_to_mult(enum ibv_rate rate) __attribute_const;

/**
 * mult_to_ibv_rate - Convert a multiple of 2.5 Gbit/sec to an IB rate enum.
 * @mult: multiple to convert.
 */
enum ibv_rate mult_to_ibv_rate(int mult) __attribute_const;

/**
 * ibv_rate_to_mbps - Convert the IB rate enum to Mbit/sec.
 * For example, IBV_RATE_5_GBPS will return the value 5000.
 * @rate: rate to convert.
 */
int ibv_rate_to_mbps(enum ibv_rate rate) __attribute_const;

/**
 * mbps_to_ibv_rate - Convert a Mbit/sec value to an IB rate enum.
 * @mbps: value to convert.
 */
enum ibv_rate mbps_to_ibv_rate(int mbps) __attribute_const;

/* 中文逐项-类型 T0240：ibv_ah_attr
 * 类型职责：全局路由/地址句柄属性/路由头，结合LID或GID指定目标路径；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：grh、dlid、sl、src_path_bits、static_rate、is_global、port_num。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_ah_attr {
/* 中文逐项-变量 V2414：grh
 * grh（struct ibv_global_route；成员 ibv_ah_attr）：GID的全局字段视图或全局路由头/属性，和裸16字节表示复用同一数据。
 */
	struct ibv_global_route	grh;
/* 中文逐项-变量 V2415：dlid
 * dlid（uint16_t；成员 ibv_ah_attr）：IB本地/目标/子网管理器/源LID地址信息，RoCE全球路由通常改用GID路径。
 */
	uint16_t		dlid;
/* 中文逐项-变量 V2416：sl
 * sl（uint8_t；成员 ibv_ah_attr）：IB服务等级及路径位，参与数据包路径/优先级选择。
 */
	uint8_t			sl;
/* 中文逐项-变量 V2417：src_path_bits
 * src_path_bits（uint8_t；成员 ibv_ah_attr）：IB服务等级及路径位，参与数据包路径/优先级选择。
 */
	uint8_t			src_path_bits;
/* 中文逐项-变量 V2418：static_rate
 * static_rate（uint8_t；成员 ibv_ah_attr）：IB路径的静态速率选择值，属于路由配置字段。
 */
	uint8_t			static_rate;
/* 中文逐项-变量 V2419：is_global
 * is_global（uint8_t；成员 ibv_ah_attr）：地址句柄是否使用全局路由属性，决定GID等字段是否生效。
 */
	uint8_t			is_global;
/* 中文逐项-变量 V2420：port_num
 * port_num（uint8_t；成员 ibv_ah_attr）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
	uint8_t			port_num;
};

/* 中文逐项-类型 T0241：ibv_srq_attr_mask
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_srq_attr_mask {
/* 中文逐项-枚举项 X0489：IBV_SRQ_MAX_WR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 0
 */
	IBV_SRQ_MAX_WR	= 1 << 0,
/* 中文逐项-枚举项 X0490：IBV_SRQ_LIMIT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_SRQ_LIMIT	= 1 << 1
};

/* 中文逐项-类型 T0242：ibv_srq_attr
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：max_wr、max_sge、srq_limit。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_srq_attr {
/* 中文逐项-变量 V2421：max_wr
 * max_wr（uint32_t；成员 ibv_srq_attr）：单QP队列或SRQ所支持的最大工作请求条目容量，单位是WQE条目。
 */
	uint32_t		max_wr;
/* 中文逐项-变量 V2422：max_sge
 * max_sge（uint32_t；成员 ibv_srq_attr）：max发送/接收单请求允许的scatter/gather条目数。
 */
	uint32_t		max_sge;
/* 中文逐项-变量 V2423：srq_limit
 * srq_limit（uint32_t；成员 ibv_srq_attr）：SRQ剩余接收项阈值，用于触发低水位事件；本版内置数据QP未绑定SRQ。
 */
	uint32_t		srq_limit;
};

/* 中文逐项-类型 T0243：ibv_srq_init_attr
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：srq_context、attr。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_srq_init_attr {
/* 中文逐项-变量 V2424：srq_context
 * srq_context（void；成员 ibv_srq_init_attr）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 */
	void		       *srq_context;
/* 中文逐项-变量 V2425：attr
 * attr（struct ibv_srq_attr；成员 ibv_srq_init_attr）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 */
	struct ibv_srq_attr	attr;
};

/* 中文逐项-类型 T0244：ibv_srq_type
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_srq_type {
/* 中文逐项-枚举项 X0491：IBV_SRQT_BASIC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_SRQT_BASIC,
/* 中文逐项-枚举项 X0492：IBV_SRQT_XRC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_SRQT_XRC
};

/* 中文逐项-类型 T0245：ibv_srq_init_attr_mask
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_srq_init_attr_mask {
/* 中文逐项-枚举项 X0493：IBV_SRQ_INIT_ATTR_TYPE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 0
 */
	IBV_SRQ_INIT_ATTR_TYPE		= 1 << 0,
/* 中文逐项-枚举项 X0494：IBV_SRQ_INIT_ATTR_PD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_SRQ_INIT_ATTR_PD		= 1 << 1,
/* 中文逐项-枚举项 X0495：IBV_SRQ_INIT_ATTR_XRCD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 2
 */
	IBV_SRQ_INIT_ATTR_XRCD		= 1 << 2,
/* 中文逐项-枚举项 X0496：IBV_SRQ_INIT_ATTR_CQ
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 3
 */
	IBV_SRQ_INIT_ATTR_CQ		= 1 << 3,
/* 中文逐项-枚举项 X0497：IBV_SRQ_INIT_ATTR_RESERVED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 4
 */
	IBV_SRQ_INIT_ATTR_RESERVED	= 1 << 4
};

/* 中文逐项-类型 T0246：ibv_srq_init_attr_ex
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：srq_context、attr、comp_mask、srq_type、pd、xrcd、cq。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_srq_init_attr_ex {
/* 中文逐项-变量 V2426：srq_context
 * srq_context（void；成员 ibv_srq_init_attr_ex）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 */
	void		       *srq_context;
/* 中文逐项-变量 V2427：attr
 * attr（struct ibv_srq_attr；成员 ibv_srq_init_attr_ex）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 */
	struct ibv_srq_attr	attr;

/* 中文逐项-变量 V2428：comp_mask
 * comp_mask（uint32_t；成员 ibv_srq_init_attr_ex）：结构体可选扩展字段有效性掩码，调用者只允许使用当前ABI支持的字段。
 */
	uint32_t		comp_mask;
/* 中文逐项-变量 V2429：srq_type
 * srq_type（enum ibv_srq_type；成员 ibv_srq_init_attr_ex）：共享接收队列类别，区分基本SRQ与XRC等扩展。
 */
	enum ibv_srq_type	srq_type;
/* 中文逐项-变量 V2430：pd
 * pd（struct ibv_pd；成员 ibv_srq_init_attr_ex）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
	struct ibv_pd	       *pd;
/* 中文逐项-变量 V2431：xrcd
 * xrcd（struct ibv_xrcd；成员 ibv_srq_init_attr_ex）：扩展可靠连接域相关对象，用于XRC机制，与普通RC数据QP路径区分。
 */
	struct ibv_xrcd	       *xrcd;
/* 中文逐项-变量 V2432：cq
 * cq（struct ibv_cq；成员 ibv_srq_init_attr_ex）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
	struct ibv_cq	       *cq;
};

/* 中文逐项-类型 T0247：ibv_qp_type
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_qp_type {
/* 中文逐项-枚举项 X0498：IBV_QPT_RC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
	IBV_QPT_RC = 2,
/* 中文逐项-枚举项 X0499：IBV_QPT_UC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPT_UC,
/* 中文逐项-枚举项 X0500：IBV_QPT_UD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPT_UD,
	/* XRC compatible code */
/* 中文逐项-枚举项 X0501：IBV_QPT_XRC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPT_XRC,
/* 中文逐项-枚举项 X0502：IBV_QPT_RAW_PACKET
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
	IBV_QPT_RAW_PACKET = 8,
/* 中文逐项-枚举项 X0503：IBV_QPT_RAW_ETH
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
	IBV_QPT_RAW_ETH = 8,
/* 中文逐项-枚举项 X0504：IBV_QPT_XRC_SEND
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：9
 */
	IBV_QPT_XRC_SEND = 9,
/* 中文逐项-枚举项 X0505：IBV_QPT_XRC_RECV
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPT_XRC_RECV,

	/* Leave a gap for future qp types before starting with
	 * experimental qp types.
	 */
/* 中文逐项-枚举项 X0506：IBV_EXP_QP_TYPE_START
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：32
 */
	IBV_EXP_QP_TYPE_START	= 32,
/* 中文逐项-枚举项 X0507：IBV_EXP_QPT_DC_INI
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：IBV_EXP_QP_TYPE_START
 */
	IBV_EXP_QPT_DC_INI	= IBV_EXP_QP_TYPE_START
};

/* 中文逐项-类型 T0248：ibv_qp_cap
 * 类型职责：QP队列容量参数，包括发送/接收WR、SGE及inline上限；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：max_send_wr、max_recv_wr、max_send_sge、max_recv_sge、max_inline_data。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_qp_cap {
/* 中文逐项-变量 V2433：max_send_wr
 * max_send_wr（uint32_t；成员 ibv_qp_cap）：max QP容量描述及发送/接收队列WQE条目上限，不等于已提交请求数。
 */
	uint32_t		max_send_wr;
/* 中文逐项-变量 V2434：max_recv_wr
 * max_recv_wr（uint32_t；成员 ibv_qp_cap）：max QP容量描述及发送/接收队列WQE条目上限，不等于已提交请求数。
 */
	uint32_t		max_recv_wr;
/* 中文逐项-变量 V2435：max_send_sge
 * max_send_sge（uint32_t；成员 ibv_qp_cap）：max发送/接收单请求允许的scatter/gather条目数。
 */
	uint32_t		max_send_sge;
/* 中文逐项-变量 V2436：max_recv_sge
 * max_recv_sge（uint32_t；成员 ibv_qp_cap）：max发送/接收单请求允许的scatter/gather条目数。
 */
	uint32_t		max_recv_sge;
/* 中文逐项-变量 V2437：max_inline_data
 * max_inline_data（uint32_t；成员 ibv_qp_cap）：允许把多少载荷字节内联到发送WQE，避免NIC再次读取这段小数据源。
 */
	uint32_t		max_inline_data;
};

/* 中文逐项-类型 T0249：ibv_qp_init_attr
 * 类型职责：创建QP所需PD之外的初始属性，指定类型、CQ、可选SRQ和容量；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：qp_context、send_cq、recv_cq、srq、cap、qp_type、sq_sig_all、xrc_domain。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_qp_init_attr {
/* 中文逐项-变量 V2438：qp_context
 * qp_context（void；成员 ibv_qp_init_attr）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 */
	void		       *qp_context;
/* 中文逐项-变量 V2439：send_cq
 * send_cq（struct ibv_cq；成员 ibv_qp_init_attr）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
	struct ibv_cq	       *send_cq;
/* 中文逐项-变量 V2440：recv_cq
 * recv_cq（struct ibv_cq；成员 ibv_qp_init_attr）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
	struct ibv_cq	       *recv_cq;
/* 中文逐项-变量 V2441：srq
 * srq（struct ibv_srq；成员 ibv_qp_init_attr）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 */
	struct ibv_srq	       *srq;
/* 中文逐项-变量 V2442：cap
 * cap（struct ibv_qp_cap；成员 ibv_qp_init_attr）：max QP容量描述及发送/接收队列WQE条目上限，不等于已提交请求数。
 */
	struct ibv_qp_cap	cap;
/* 中文逐项-变量 V2443：qp_type
 * qp_type（enum ibv_qp_type；成员 ibv_qp_init_attr）：QP服务类型，本版数据连接创建RC即可靠连接类型。
 */
	enum ibv_qp_type	qp_type;
/* 中文逐项-变量 V2444：sq_sig_all
 * sq_sig_all（int；成员 ibv_qp_init_attr）：是否每条发送WR默认都产生完成通知，NCCL通常按请求选择性signaled。
 */
	int			sq_sig_all;
	/* Below is needed for backwards compatabile */
/* 中文逐项-变量 V2445：xrc_domain
 * xrc_domain（struct ibv_xrc_domain；成员 ibv_qp_init_attr）：扩展可靠连接域相关对象，用于XRC机制，与普通RC数据QP路径区分。
 */
	struct ibv_xrc_domain  *xrc_domain;
};

/* 中文逐项-类型 T0250：ibv_qp_init_attr_mask
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_qp_init_attr_mask {
/* 中文逐项-枚举项 X0508：IBV_QP_INIT_ATTR_PD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 0
 */
	IBV_QP_INIT_ATTR_PD		= 1 << 0,
/* 中文逐项-枚举项 X0509：IBV_QP_INIT_ATTR_XRCD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_QP_INIT_ATTR_XRCD		= 1 << 1,
/* 中文逐项-枚举项 X0510：IBV_QP_INIT_ATTR_RESERVED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 2
 */
	IBV_QP_INIT_ATTR_RESERVED	= 1 << 2
};

/* 中文逐项-类型 T0251：ibv_qp_init_attr_ex
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：qp_context、send_cq、recv_cq、srq、cap、qp_type、sq_sig_all、comp_mask、pd、xrcd。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_qp_init_attr_ex {
/* 中文逐项-变量 V2446：qp_context
 * qp_context（void；成员 ibv_qp_init_attr_ex）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 */
	void		       *qp_context;
/* 中文逐项-变量 V2447：send_cq
 * send_cq（struct ibv_cq；成员 ibv_qp_init_attr_ex）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
	struct ibv_cq	       *send_cq;
/* 中文逐项-变量 V2448：recv_cq
 * recv_cq（struct ibv_cq；成员 ibv_qp_init_attr_ex）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
	struct ibv_cq	       *recv_cq;
/* 中文逐项-变量 V2449：srq
 * srq（struct ibv_srq；成员 ibv_qp_init_attr_ex）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 */
	struct ibv_srq	       *srq;
/* 中文逐项-变量 V2450：cap
 * cap（struct ibv_qp_cap；成员 ibv_qp_init_attr_ex）：max QP容量描述及发送/接收队列WQE条目上限，不等于已提交请求数。
 */
	struct ibv_qp_cap	cap;
/* 中文逐项-变量 V2451：qp_type
 * qp_type（enum ibv_qp_type；成员 ibv_qp_init_attr_ex）：QP服务类型，本版数据连接创建RC即可靠连接类型。
 */
	enum ibv_qp_type	qp_type;
/* 中文逐项-变量 V2452：sq_sig_all
 * sq_sig_all（int；成员 ibv_qp_init_attr_ex）：是否每条发送WR默认都产生完成通知，NCCL通常按请求选择性signaled。
 */
	int			sq_sig_all;

/* 中文逐项-变量 V2453：comp_mask
 * comp_mask（uint32_t；成员 ibv_qp_init_attr_ex）：结构体可选扩展字段有效性掩码，调用者只允许使用当前ABI支持的字段。
 */
	uint32_t		comp_mask;
/* 中文逐项-变量 V2454：pd
 * pd（struct ibv_pd；成员 ibv_qp_init_attr_ex）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
	struct ibv_pd	       *pd;
/* 中文逐项-变量 V2455：xrcd
 * xrcd（struct ibv_xrcd；成员 ibv_qp_init_attr_ex）：扩展可靠连接域相关对象，用于XRC机制，与普通RC数据QP路径区分。
 */
	struct ibv_xrcd	       *xrcd;
};

/* 中文逐项-类型 T0252：ibv_qp_open_attr_mask
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_qp_open_attr_mask {
/* 中文逐项-枚举项 X0511：IBV_QP_OPEN_ATTR_NUM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 0
 */
	IBV_QP_OPEN_ATTR_NUM		= 1 << 0,
/* 中文逐项-枚举项 X0512：IBV_QP_OPEN_ATTR_XRCD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_QP_OPEN_ATTR_XRCD		= 1 << 1,
/* 中文逐项-枚举项 X0513：IBV_QP_OPEN_ATTR_CONTEXT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 2
 */
	IBV_QP_OPEN_ATTR_CONTEXT	= 1 << 2,
/* 中文逐项-枚举项 X0514：IBV_QP_OPEN_ATTR_TYPE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 3
 */
	IBV_QP_OPEN_ATTR_TYPE		= 1 << 3,
/* 中文逐项-枚举项 X0515：IBV_QP_OPEN_ATTR_RESERVED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 4
 */
	IBV_QP_OPEN_ATTR_RESERVED	= 1 << 4
};

/* 中文逐项-类型 T0253：ibv_qp_open_attr
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：comp_mask、qp_num、xrcd、qp_context、qp_type。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_qp_open_attr {
/* 中文逐项-变量 V2456：comp_mask
 * comp_mask（uint32_t；成员 ibv_qp_open_attr）：结构体可选扩展字段有效性掩码，调用者只允许使用当前ABI支持的字段。
 */
	uint32_t		comp_mask;
/* 中文逐项-变量 V2457：qp_num
 * qp_num（uint32_t；成员 ibv_qp_open_attr）：本地/远端QP编号，用于控制握手及连接目标匹配。
 */
	uint32_t		qp_num;
/* 中文逐项-变量 V2458：xrcd
 * xrcd（struct ibv_xrcd；成员 ibv_qp_open_attr）：扩展可靠连接域相关对象，用于XRC机制，与普通RC数据QP路径区分。
 */
	struct ibv_xrcd        *xrcd;
/* 中文逐项-变量 V2459：qp_context
 * qp_context（void；成员 ibv_qp_open_attr）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 */
	void		       *qp_context;
/* 中文逐项-变量 V2460：qp_type
 * qp_type（enum ibv_qp_type；成员 ibv_qp_open_attr）：QP服务类型，本版数据连接创建RC即可靠连接类型。
 */
	enum ibv_qp_type	qp_type;
};

/* 中文逐项-类型 T0254：ibv_qp_attr_mask
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_qp_attr_mask {
/* 中文逐项-枚举项 X0516：IBV_QP_STATE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 0
 */
	IBV_QP_STATE			= 1 << 	0,
/* 中文逐项-枚举项 X0517：IBV_QP_CUR_STATE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_QP_CUR_STATE		= 1 << 	1,
/* 中文逐项-枚举项 X0518：IBV_QP_EN_SQD_ASYNC_NOTIFY
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 2
 */
	IBV_QP_EN_SQD_ASYNC_NOTIFY	= 1 << 	2,
/* 中文逐项-枚举项 X0519：IBV_QP_ACCESS_FLAGS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 3
 */
	IBV_QP_ACCESS_FLAGS		= 1 << 	3,
/* 中文逐项-枚举项 X0520：IBV_QP_PKEY_INDEX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 4
 */
	IBV_QP_PKEY_INDEX		= 1 << 	4,
/* 中文逐项-枚举项 X0521：IBV_QP_PORT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 5
 */
	IBV_QP_PORT			= 1 << 	5,
/* 中文逐项-枚举项 X0522：IBV_QP_QKEY
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 6
 */
	IBV_QP_QKEY			= 1 << 	6,
/* 中文逐项-枚举项 X0523：IBV_QP_AV
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 7
 */
	IBV_QP_AV			= 1 << 	7,
/* 中文逐项-枚举项 X0524：IBV_QP_PATH_MTU
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 8
 */
	IBV_QP_PATH_MTU			= 1 << 	8,
/* 中文逐项-枚举项 X0525：IBV_QP_TIMEOUT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 9
 */
	IBV_QP_TIMEOUT			= 1 << 	9,
/* 中文逐项-枚举项 X0526：IBV_QP_RETRY_CNT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 10
 */
	IBV_QP_RETRY_CNT		= 1 << 10,
/* 中文逐项-枚举项 X0527：IBV_QP_RNR_RETRY
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 11
 */
	IBV_QP_RNR_RETRY		= 1 << 11,
/* 中文逐项-枚举项 X0528：IBV_QP_RQ_PSN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 12
 */
	IBV_QP_RQ_PSN			= 1 << 12,
/* 中文逐项-枚举项 X0529：IBV_QP_MAX_QP_RD_ATOMIC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 13
 */
	IBV_QP_MAX_QP_RD_ATOMIC		= 1 << 13,
/* 中文逐项-枚举项 X0530：IBV_QP_ALT_PATH
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 14
 */
	IBV_QP_ALT_PATH			= 1 << 14,
/* 中文逐项-枚举项 X0531：IBV_QP_MIN_RNR_TIMER
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 15
 */
	IBV_QP_MIN_RNR_TIMER		= 1 << 15,
/* 中文逐项-枚举项 X0532：IBV_QP_SQ_PSN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 16
 */
	IBV_QP_SQ_PSN			= 1 << 16,
/* 中文逐项-枚举项 X0533：IBV_QP_MAX_DEST_RD_ATOMIC
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 17
 */
	IBV_QP_MAX_DEST_RD_ATOMIC	= 1 << 17,
/* 中文逐项-枚举项 X0534：IBV_QP_PATH_MIG_STATE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 18
 */
	IBV_QP_PATH_MIG_STATE		= 1 << 18,
/* 中文逐项-枚举项 X0535：IBV_QP_CAP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 19
 */
	IBV_QP_CAP			= 1 << 19,
/* 中文逐项-枚举项 X0536：IBV_QP_DEST_QPN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 20
 */
	IBV_QP_DEST_QPN			= 1 << 20
};

/* 中文逐项-类型 T0255：ibv_qp_state
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_qp_state {
/* 中文逐项-枚举项 X0537：IBV_QPS_RESET
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPS_RESET,
/* 中文逐项-枚举项 X0538：IBV_QPS_INIT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPS_INIT,
/* 中文逐项-枚举项 X0539：IBV_QPS_RTR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPS_RTR,
/* 中文逐项-枚举项 X0540：IBV_QPS_RTS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPS_RTS,
/* 中文逐项-枚举项 X0541：IBV_QPS_SQD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPS_SQD,
/* 中文逐项-枚举项 X0542：IBV_QPS_SQE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPS_SQE,
/* 中文逐项-枚举项 X0543：IBV_QPS_ERR
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPS_ERR,
/* 中文逐项-枚举项 X0544：IBV_QPS_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_QPS_UNKNOWN
};

/* 中文逐项-类型 T0256：ibv_mig_state
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_mig_state {
/* 中文逐项-枚举项 X0545：IBV_MIG_MIGRATED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_MIG_MIGRATED,
/* 中文逐项-枚举项 X0546：IBV_MIG_REARM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_MIG_REARM,
/* 中文逐项-枚举项 X0547：IBV_MIG_ARMED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_MIG_ARMED
};

/* 中文逐项-类型 T0257：ibv_qp_attr
 * 类型职责：修改QP所需的状态、路由、序号、访问权限及重试参数，按attr_mask选择有效字段；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：qp_state、cur_qp_state、path_mtu、path_mig_state、qkey、rq_psn、sq_psn、dest_qp_num、qp_access_flags、cap、ah_attr、alt_ah_attr、pkey_index、alt_pkey_index、en_sqd_async_notify、sq_draining、max_rd_atomic、max_dest_rd_atomic、min_rnr_timer、port_num、timeout、retry_cnt、rnr_retry、alt_port_num、alt_timeout。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_qp_attr {
/* 中文逐项-变量 V2461：qp_state
 * qp_state（enum ibv_qp_state；成员 ibv_qp_attr）：队列对目标/当前状态，建链通常按 RESET/INIT/RTR/RTS 配置。
 */
	enum ibv_qp_state	qp_state;
/* 中文逐项-变量 V2462：cur_qp_state
 * cur_qp_state（enum ibv_qp_state；成员 ibv_qp_attr）：队列对目标/当前状态，建链通常按 RESET/INIT/RTR/RTS 配置。
 */
	enum ibv_qp_state	cur_qp_state;
/* 中文逐项-变量 V2463：path_mtu
 * path_mtu（enum ibv_mtu；成员 ibv_qp_attr）：路径最大传输单元选择，影响NIC拆包但不直接等于NCCL算法chunk尺寸。
 */
	enum ibv_mtu		path_mtu;
/* 中文逐项-变量 V2464：path_mig_state
 * path_mig_state（enum ibv_mig_state；成员 ibv_qp_attr）：QP主/备用路径迁移状态，本版普通RC建立通常使用固定主路径。
 */
	enum ibv_mig_state	path_mig_state;
/* 中文逐项-变量 V2465：qkey
 * qkey（uint32_t；成员 ibv_qp_attr）：不可靠数据报等类型使用的授权key，不能与MR的rkey混淆。
 */
	uint32_t		qkey;
/* 中文逐项-变量 V2466：rq_psn
 * rq_psn（uint32_t；成员 ibv_qp_attr）：接收/发送分组序列号，可靠连接状态机用来识别顺序和重传。
 */
	uint32_t		rq_psn;
/* 中文逐项-变量 V2467：sq_psn
 * sq_psn（uint32_t；成员 ibv_qp_attr）：接收/发送分组序列号，可靠连接状态机用来识别顺序和重传。
 */
	uint32_t		sq_psn;
/* 中文逐项-变量 V2468：dest_qp_num
 * dest_qp_num（uint32_t；成员 ibv_qp_attr）：本地/远端QP编号，用于控制握手及连接目标匹配。
 */
	uint32_t		dest_qp_num;
/* 中文逐项-变量 V2469：qp_access_flags
 * qp_access_flags（int；成员 ibv_qp_attr）：允许远端对相关接收方向执行的访问权限位掩码。
 */
	int			qp_access_flags;
/* 中文逐项-变量 V2470：cap
 * cap（struct ibv_qp_cap；成员 ibv_qp_attr）：max QP容量描述及发送/接收队列WQE条目上限，不等于已提交请求数。
 */
	struct ibv_qp_cap	cap;
/* 中文逐项-变量 V2471：ah_attr
 * ah_attr（struct ibv_ah_attr；成员 ibv_qp_attr）：主/备用寻址与路由属性，包含LID/GID、端口及服务等级。
 */
	struct ibv_ah_attr	ah_attr;
/* 中文逐项-变量 V2472：alt_ah_attr
 * alt_ah_attr（struct ibv_ah_attr；成员 ibv_qp_attr）：主/备用寻址与路由属性，包含LID/GID、端口及服务等级。
 */
	struct ibv_ah_attr	alt_ah_attr;
/* 中文逐项-变量 V2473：pkey_index
 * pkey_index（uint16_t；成员 ibv_qp_attr）：主/备用路径使用的分区key表索引，参与IB分区隔离。
 */
	uint16_t		pkey_index;
/* 中文逐项-变量 V2474：alt_pkey_index
 * alt_pkey_index（uint16_t；成员 ibv_qp_attr）：主/备用路径使用的分区key表索引，参与IB分区隔离。
 */
	uint16_t		alt_pkey_index;
/* 中文逐项-变量 V2475：en_sqd_async_notify
 * en_sqd_async_notify（uint8_t；成员 ibv_qp_attr）：是否启用SQ排空异步通知及发送队列是否在排空，属于QP状态变更协议。
 */
	uint8_t			en_sqd_async_notify;
/* 中文逐项-变量 V2476：sq_draining
 * sq_draining（uint8_t；成员 ibv_qp_attr）：是否启用SQ排空异步通知及发送队列是否在排空，属于QP状态变更协议。
 */
	uint8_t			sq_draining;
/* 中文逐项-变量 V2477：max_rd_atomic
 * max_rd_atomic（uint8_t；成员 ibv_qp_attr）：本QP发起或接收侧允许的未完成RDMA读/原子操作并发深度。
 */
	uint8_t			max_rd_atomic;
/* 中文逐项-变量 V2478：max_dest_rd_atomic
 * max_dest_rd_atomic（uint8_t；成员 ibv_qp_attr）：本QP发起或接收侧允许的未完成RDMA读/原子操作并发深度。
 */
	uint8_t			max_dest_rd_atomic;
/* 中文逐项-变量 V2479：min_rnr_timer
 * min_rnr_timer（uint8_t；成员 ibv_qp_attr）：RC重试、接收未就绪重试与超时配置；属于可靠传输，不是用户API超时。
 */
	uint8_t			min_rnr_timer;
/* 中文逐项-变量 V2480：port_num
 * port_num（uint8_t；成员 ibv_qp_attr）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
	uint8_t			port_num;
/* 中文逐项-变量 V2481：timeout
 * timeout（uint8_t；成员 ibv_qp_attr）：RC重试、接收未就绪重试与超时配置；属于可靠传输，不是用户API超时。
 */
	uint8_t			timeout;
/* 中文逐项-变量 V2482：retry_cnt
 * retry_cnt（uint8_t；成员 ibv_qp_attr）：RC重试、接收未就绪重试与超时配置；属于可靠传输，不是用户API超时。
 */
	uint8_t			retry_cnt;
/* 中文逐项-变量 V2483：rnr_retry
 * rnr_retry（uint8_t；成员 ibv_qp_attr）：RC重试、接收未就绪重试与超时配置；属于可靠传输，不是用户API超时。
 */
	uint8_t			rnr_retry;
/* 中文逐项-变量 V2484：alt_port_num
 * alt_port_num（uint8_t；成员 ibv_qp_attr）：QP备用路径所用端口及超时设置，配合alt_ah_attr等字段。
 */
	uint8_t			alt_port_num;
/* 中文逐项-变量 V2485：alt_timeout
 * alt_timeout（uint8_t；成员 ibv_qp_attr）：QP备用路径所用端口及超时设置，配合alt_ah_attr等字段。
 */
	uint8_t			alt_timeout;
};

/* 中文逐项-类型 T0258：ibv_wr_opcode
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_wr_opcode {
/* 中文逐项-枚举项 X0548：IBV_WR_RDMA_WRITE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WR_RDMA_WRITE,
/* 中文逐项-枚举项 X0549：IBV_WR_RDMA_WRITE_WITH_IMM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WR_RDMA_WRITE_WITH_IMM,
/* 中文逐项-枚举项 X0550：IBV_WR_SEND
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WR_SEND,
/* 中文逐项-枚举项 X0551：IBV_WR_SEND_WITH_IMM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WR_SEND_WITH_IMM,
/* 中文逐项-枚举项 X0552：IBV_WR_RDMA_READ
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WR_RDMA_READ,
/* 中文逐项-枚举项 X0553：IBV_WR_ATOMIC_CMP_AND_SWP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WR_ATOMIC_CMP_AND_SWP,
/* 中文逐项-枚举项 X0554：IBV_WR_ATOMIC_FETCH_AND_ADD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
	IBV_WR_ATOMIC_FETCH_AND_ADD
};

/* 中文逐项-类型 T0259：ibv_send_flags
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_send_flags {
/* 中文逐项-枚举项 X0555：IBV_SEND_FENCE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 0
 */
	IBV_SEND_FENCE		= 1 << 0,
/* 中文逐项-枚举项 X0556：IBV_SEND_SIGNALED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_SEND_SIGNALED	= 1 << 1,
/* 中文逐项-枚举项 X0557：IBV_SEND_SOLICITED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 2
 */
	IBV_SEND_SOLICITED	= 1 << 2,
/* 中文逐项-枚举项 X0558：IBV_SEND_INLINE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 3
 */
	IBV_SEND_INLINE		= 1 << 3
};

/* 中文逐项-类型 T0260：ibv_sge
 * 类型职责：本地内存段地址、字节长度和lkey，一个WR可引用若干SGE；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：addr、length、lkey。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_sge {
/* 中文逐项-变量 V2486：addr
 * addr（uint64_t；成员 ibv_sge）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
	uint64_t		addr;
/* 中文逐项-变量 V2487：length
 * length（uint32_t；成员 ibv_sge）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
	uint32_t		length;
/* 中文逐项-变量 V2488：lkey
 * lkey（uint32_t；成员 ibv_sge）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
	uint32_t		lkey;
};

/* 中文逐项-类型 T0261：ibv_send_wr
 * 类型职责：发送工作请求及类型相关union，opcode决定按RDMA/atomic/UD哪种布局解释；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：wr_id、next、sg_list、num_sge、opcode、send_flags、imm_data、wr、rdma、remote_addr、rkey、atomic、remote_addr、compare_add、swap、rkey、ud、ah、remote_qpn、remote_qkey、qp_type、xrc、remote_srqn、xrc_remote_srq_num。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_send_wr {
/* 中文逐项-变量 V2489：wr_id
 * wr_id（uint64_t；成员 ibv_send_wr）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 */
	uint64_t		wr_id;
/* 中文逐项-变量 V2490：next
 * next（struct ibv_send_wr；成员 ibv_send_wr）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
	struct ibv_send_wr     *next;
/* 中文逐项-变量 V2491：sg_list
 * sg_list（struct ibv_sge；成员 ibv_send_wr）：scatter/gather 地址段列表或单段，指定本地地址、长度和lkey。
 */
	struct ibv_sge	       *sg_list;
/* 中文逐项-变量 V2492：num_sge
 * num_sge（int；成员 ibv_send_wr）：当前请求的SGE条目数；接收WRITE_WITH_IMM通知时本版可为0，载荷直接写目标MR。
 */
	int			num_sge;
/* 中文逐项-变量 V2493：opcode
 * opcode（enum ibv_wr_opcode；成员 ibv_send_wr）：verbs 请求/完成的操作码，区分 RDMA_WRITE、WRITE_WITH_IMM、READ 等。
 */
	enum ibv_wr_opcode	opcode;
/* 中文逐项-变量 V2494：send_flags
 * send_flags（int；成员 ibv_send_wr）：发送请求的位标志，如 signaled、inline；是否请求完成会影响CQ事件计数。
 */
	int			send_flags;
/* 中文逐项-变量 V2495：imm_data
 * imm_data（uint32_t；成员 ibv_send_wr）：完成项报告的长度或32位立即数；WRITE_WITH_IMM立即数用于控制通知，具体编码由后端约定。
 */
	uint32_t		imm_data;	/* in network byte order */
/* 中文逐项-变量 V2496：wr
 * wr（union { struct { uint64_t remote_addr; uint32_t rkey; } rdma; struct { uint64_t remote_addr; uint64_t compare_
 * …（完整表达式见紧邻源码）；成员 ibv_send_wr）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 */
/* 中文逐项-类型 T0262：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：wr、rdma、remote_addr、rkey、atomic、remote_addr、compare_add、swap、rkey、ud、ah、remote_qpn、remote_qkey。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
	union {
/* 中文逐项-变量 V2497：rdma
 * rdma（struct { uint64_t remote_addr; uint32_t rkey; }；成员
 * ibv_send_wr::<匿名>）：发送WR的操作相关union视图，由opcode/qp_type决定使用远端地址、原子操作数或UD/XRC寻址。
 */
/* 中文逐项-类型 T0263：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：rdma、remote_addr、rkey。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
		struct {
/* 中文逐项-变量 V2498：remote_addr
 * remote_addr（uint64_t；成员 ibv_send_wr::<匿名>::<匿名>）：远端MR范围内的虚拟/HCA可访问地址，必须与正确rkey匹配。
 */
			uint64_t	remote_addr;
/* 中文逐项-变量 V2499：rkey
 * rkey（uint32_t；成员 ibv_send_wr::<匿名>::<匿名>）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
			uint32_t	rkey;
		} rdma;
/* 中文逐项-变量 V2500：atomic
 * atomic（struct { uint64_t remote_addr; uint64_t compare_add; uint64_t swap; uint32_t rkey; }；成员
 * ibv_send_wr::<匿名>）：发送WR的操作相关union视图，由opcode/qp_type决定使用远端地址、原子操作数或UD/XRC寻址。
 */
/* 中文逐项-类型 T0264：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：atomic、remote_addr、compare_add、swap、rkey。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
		struct {
/* 中文逐项-变量 V2501：remote_addr
 * remote_addr（uint64_t；成员 ibv_send_wr::<匿名>::<匿名>）：远端MR范围内的虚拟/HCA可访问地址，必须与正确rkey匹配。
 */
			uint64_t	remote_addr;
/* 中文逐项-变量 V2502：compare_add
 * compare_add（uint64_t；成员 ibv_send_wr::<匿名>::<匿名>）：RDMA原子比较交换/加法请求的操作数，本版IB普通数据传输不据此自动执行张量归约。
 */
			uint64_t	compare_add;
/* 中文逐项-变量 V2503：swap
 * swap（uint64_t；成员 ibv_send_wr::<匿名>::<匿名>）：RDMA原子比较交换/加法请求的操作数，本版IB普通数据传输不据此自动执行张量归约。
 */
			uint64_t	swap;
/* 中文逐项-变量 V2504：rkey
 * rkey（uint32_t；成员 ibv_send_wr::<匿名>::<匿名>）：本地 SGE/远端 RDMA 访问所需授权key；与地址和PD共同限定有效注册区域。
 */
			uint32_t	rkey;
		} atomic;
/* 中文逐项-变量 V2505：ud
 * ud（struct { struct ibv_ah *ah; uint32_t remote_qpn; uint32_t remote_qkey; }；成员
 * ibv_send_wr::<匿名>）：发送WR的操作相关union视图，由opcode/qp_type决定使用远端地址、原子操作数或UD/XRC寻址。
 */
/* 中文逐项-类型 T0265：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：ud、ah、remote_qpn、remote_qkey。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
		struct {
/* 中文逐项-变量 V2506：ah
 * ah（struct ibv_ah；成员 ibv_send_wr::<匿名>::<匿名>）：地址句柄，描述UD等非连接路径的寻址属性。
 */
			struct ibv_ah  *ah;
/* 中文逐项-变量 V2507：remote_qpn
 * remote_qpn（uint32_t；成员 ibv_send_wr::<匿名>::<匿名>）：本地/远端QP编号，用于控制握手及连接目标匹配。
 */
			uint32_t	remote_qpn;
/* 中文逐项-变量 V2508：remote_qkey
 * remote_qkey（uint32_t；成员 ibv_send_wr::<匿名>::<匿名>）：不可靠数据报等类型使用的授权key，不能与MR的rkey混淆。
 */
			uint32_t	remote_qkey;
		} ud;
	} wr;
/* 中文逐项-类型 T0266：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：qp_type、xrc、remote_srqn、xrc_remote_srq_num。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
	union {
/* 中文逐项-变量 V2509：qp_type
 * qp_type（union { struct { uint32_t remote_srqn; } xrc; }；成员 ibv_send_wr::<匿名>）：QP服务类型，本版数据连接创建RC即可靠连接类型。
 */
/* 中文逐项-类型 T0267：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：qp_type、xrc、remote_srqn。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
		union {
/* 中文逐项-变量 V2510：xrc
 * xrc（struct { uint32_t remote_srqn; }；成员
 * ibv_send_wr::<匿名>::<匿名>）：发送WR的操作相关union视图，由opcode/qp_type决定使用远端地址、原子操作数或UD/XRC寻址。
 */
/* 中文逐项-类型 T0268：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：xrc、remote_srqn。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
			struct {
/* 中文逐项-变量 V2511：remote_srqn
 * remote_srqn（uint32_t；成员 ibv_send_wr::<匿名>::<匿名>::<匿名>）：XRC等机制使用的远端SRQ编号，接口声明不等于NCCL内置路径使用XRC。
 */
				uint32_t    remote_srqn;
			} xrc;
		} qp_type;

/* 中文逐项-变量 V2512：xrc_remote_srq_num
 * xrc_remote_srq_num（uint32_t；成员 ibv_send_wr::<匿名>）：XRC等机制使用的远端SRQ编号，接口声明不等于NCCL内置路径使用XRC。
 */
		uint32_t		xrc_remote_srq_num;
	};
};

/* 中文逐项-类型 T0269：ibv_recv_wr
 * 类型职责：接收工作请求，指定wr_id与SGE；通知型零SGE请求仍可消耗RQ槽；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：wr_id、next、sg_list、num_sge。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_recv_wr {
/* 中文逐项-变量 V2513：wr_id
 * wr_id（uint64_t；成员 ibv_recv_wr）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 */
	uint64_t		wr_id;
/* 中文逐项-变量 V2514：next
 * next（struct ibv_recv_wr；成员 ibv_recv_wr）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 */
	struct ibv_recv_wr     *next;
/* 中文逐项-变量 V2515：sg_list
 * sg_list（struct ibv_sge；成员 ibv_recv_wr）：scatter/gather 地址段列表或单段，指定本地地址、长度和lkey。
 */
	struct ibv_sge	       *sg_list;
/* 中文逐项-变量 V2516：num_sge
 * num_sge（int；成员 ibv_recv_wr）：当前请求的SGE条目数；接收WRITE_WITH_IMM通知时本版可为0，载荷直接写目标MR。
 */
	int			num_sge;
};

/* 中文逐项-类型 T0270：ibv_mw_bind
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：wr_id、mr、addr、length、send_flags、mw_access_flags。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_mw_bind {
/* 中文逐项-变量 V2517：wr_id
 * wr_id（uint64_t；成员 ibv_mw_bind）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 */
	uint64_t		wr_id;
/* 中文逐项-变量 V2518：mr
 * mr（struct ibv_mr；成员 ibv_mw_bind）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 */
	struct ibv_mr	       *mr;
/* 中文逐项-变量 V2519：addr
 * addr（void；成员 ibv_mw_bind）：本地或远端内存/网络地址，确切地址空间以所属类型及key配对为准。
 */
	void		       *addr;
/* 中文逐项-变量 V2520：length
 * length（size_t；成员 ibv_mw_bind）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
	size_t			length;
/* 中文逐项-变量 V2521：send_flags
 * send_flags（int；成员 ibv_mw_bind）：发送请求的位标志，如 signaled、inline；是否请求完成会影响CQ事件计数。
 */
	int			send_flags;
/* 中文逐项-变量 V2522：mw_access_flags
 * mw_access_flags（int；成员 ibv_mw_bind）：内存窗口绑定访问权限位掩码，与普通MR本身权限区分。
 */
	int			mw_access_flags;
};

/* 中文逐项-类型 T0271：ibv_srq
 * 类型职责：共享接收队列句柄；本版声明ABI不代表net_ib实际使用SRQ；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：context、srq_context、pd、handle、mutex、cond、events_completed、xrc_srq_num_bin_compat_padding、xrc_domain_bin_compat_padding、xrc_cq_bin_compat_padding、ibv_srq_padding、xrc_srq_num、xrc_domain、xrc_cq。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_srq {
/* 中文逐项-变量 V2523：context
 * context（struct ibv_context；成员 ibv_srq）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
/* 中文逐项-变量 V2524：srq_context
 * srq_context（void；成员 ibv_srq）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 */
	void		       *srq_context;
/* 中文逐项-变量 V2525：pd
 * pd（struct ibv_pd；成员 ibv_srq）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
	struct ibv_pd	       *pd;
/* 中文逐项-变量 V2526：handle
 * handle（uint32_t；成员 ibv_srq）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 */
	uint32_t		handle;

/* 中文逐项-变量 V2527：mutex
 * mutex（pthread_mutex_t；成员 ibv_srq）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
	pthread_mutex_t		mutex;
/* 中文逐项-变量 V2528：cond
 * cond（pthread_cond_t；成员 ibv_srq）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
	pthread_cond_t		cond;
/* 中文逐项-变量 V2529：events_completed
 * events_completed（uint32_t；成员 ibv_srq）：资源相关事件已确认/完成计数，销毁等待要保证异步事件引用被正确处理。
 */
	uint32_t		events_completed;

	/* below are for source compatabilty with legacy XRC,
	*   padding based on ibv_srq_legacy.
	*/
/* 中文逐项-变量 V2530：xrc_srq_num_bin_compat_padding
 * xrc_srq_num_bin_compat_padding（uint32_t；成员 ibv_srq）：历史verbs二进制兼容保留字段，必须保持布局，不能当作NCCL实际启用的XRC配置。
 */
	uint32_t		xrc_srq_num_bin_compat_padding;
/* 中文逐项-变量 V2531：xrc_domain_bin_compat_padding
 * xrc_domain_bin_compat_padding（struct ibv_xrc_domain；成员 ibv_srq）：历史verbs二进制兼容保留字段，必须保持布局，不能当作NCCL实际启用的XRC配置。
 */
	struct ibv_xrc_domain	*xrc_domain_bin_compat_padding;
/* 中文逐项-变量 V2532：xrc_cq_bin_compat_padding
 * xrc_cq_bin_compat_padding（struct ibv_cq；成员 ibv_srq）：历史verbs二进制兼容保留字段，必须保持布局，不能当作NCCL实际启用的XRC配置。
 */
	struct ibv_cq	*xrc_cq_bin_compat_padding;
/* 中文逐项-变量 V2533：ibv_srq_padding
 * ibv_srq_padding（void；成员 ibv_srq）：历史verbs二进制兼容保留字段，必须保持布局，不能当作NCCL实际启用的XRC配置。
 */
	void		*ibv_srq_padding;

	/* legacy fields */
/* 中文逐项-变量 V2534：xrc_srq_num
 * xrc_srq_num（uint32_t；成员 ibv_srq）：XRC等机制使用的远端SRQ编号，接口声明不等于NCCL内置路径使用XRC。
 */
	uint32_t		xrc_srq_num;
/* 中文逐项-变量 V2535：xrc_domain
 * xrc_domain（struct ibv_xrc_domain；成员 ibv_srq）：扩展可靠连接域相关对象，用于XRC机制，与普通RC数据QP路径区分。
 */
	struct ibv_xrc_domain	*xrc_domain;
/* 中文逐项-变量 V2536：xrc_cq
 * xrc_cq（struct ibv_cq；成员 ibv_srq）：XRC接收使用的完成队列引用，本版普通RC接收路径使用QP绑定的recv_cq。
 */
	struct ibv_cq		*xrc_cq;
};

/* Not in use in new API, needed for compilation as part of source compat layer */
/* 中文逐项-类型 T0272：ibv_event_flags
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_event_flags {
/* 中文逐项-枚举项 X0559：IBV_XRC_QP_EVENT_FLAG
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x80000000
 */
	IBV_XRC_QP_EVENT_FLAG = 0x80000000,
};



/* 中文逐项-类型 T0273：ibv_qp
 * 类型职责：队列对用户态句柄及provider上下文，承载SQ/RQ并绑定CQ；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：context、qp_context、pd、send_cq、recv_cq、srq、handle、qp_num、state、qp_type、mutex、cond、events_completed。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_qp {
/* 中文逐项-变量 V2537：context
 * context（struct ibv_context；成员 ibv_qp）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
/* 中文逐项-变量 V2538：qp_context
 * qp_context（void；成员 ibv_qp）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 */
	void		       *qp_context;
/* 中文逐项-变量 V2539：pd
 * pd（struct ibv_pd；成员 ibv_qp）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
	struct ibv_pd	       *pd;
/* 中文逐项-变量 V2540：send_cq
 * send_cq（struct ibv_cq；成员 ibv_qp）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
	struct ibv_cq	       *send_cq;
/* 中文逐项-变量 V2541：recv_cq
 * recv_cq（struct ibv_cq；成员 ibv_qp）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 */
	struct ibv_cq	       *recv_cq;
/* 中文逐项-变量 V2542：srq
 * srq（struct ibv_srq；成员 ibv_qp）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 */
	struct ibv_srq	       *srq;
/* 中文逐项-变量 V2543：handle
 * handle（uint32_t；成员 ibv_qp）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 */
	uint32_t		handle;
/* 中文逐项-变量 V2544：qp_num
 * qp_num（uint32_t；成员 ibv_qp）：本地/远端QP编号，用于控制握手及连接目标匹配。
 */
	uint32_t		qp_num;
/* 中文逐项-变量 V2545：state
 * state（enum ibv_qp_state；成员 ibv_qp）：当前状态机或资源池的持久上下文，下次调用依赖其已保存阶段继续推进。
 */
	enum ibv_qp_state       state;
/* 中文逐项-变量 V2546：qp_type
 * qp_type（enum ibv_qp_type；成员 ibv_qp）：QP服务类型，本版数据连接创建RC即可靠连接类型。
 */
	enum ibv_qp_type	qp_type;

/* 中文逐项-变量 V2547：mutex
 * mutex（pthread_mutex_t；成员 ibv_qp）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
	pthread_mutex_t		mutex;
/* 中文逐项-变量 V2548：cond
 * cond（pthread_cond_t；成员 ibv_qp）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
	pthread_cond_t		cond;
/* 中文逐项-变量 V2549：events_completed
 * events_completed（uint32_t；成员 ibv_qp）：资源相关事件已确认/完成计数，销毁等待要保证异步事件引用被正确处理。
 */
	uint32_t		events_completed;
};

/* 中文逐项-类型 T0274：ibv_comp_channel
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：context、fd、refcnt。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_comp_channel {
/* 中文逐项-变量 V2550：context
 * context（struct ibv_context；成员 ibv_comp_channel）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
/* 中文逐项-变量 V2551：fd
 * fd（int；成员 ibv_comp_channel）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 */
	int			fd;
/* 中文逐项-变量 V2552：refcnt
 * refcnt（int；成员 ibv_comp_channel）：共享对象引用计数；只有最后引用释放才允许回收真实资源。
 */
	int			refcnt;
};

/* 中文逐项-类型 T0275：ibv_cq
 * 类型职责：完成队列句柄及通知/事件计数，poll得到ibv_wc；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：context、channel、cq_context、handle、cqe、mutex、cond、comp_events_completed、async_events_completed。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_cq {
/* 中文逐项-变量 V2553：context
 * context（struct ibv_context；成员 ibv_cq）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
/* 中文逐项-变量 V2554：channel
 * channel（struct ibv_comp_channel；成员 ibv_cq）：当前逻辑通信流水线对象，包含邻居、连接表和工作 FIFO。
 */
	struct ibv_comp_channel *channel;
/* 中文逐项-变量 V2555：cq_context
 * cq_context（void；成员 ibv_cq）：用户附加到QP/CQ/SRQ对象的上下文指针，verbs本身不解释其内容。
 */
	void		       *cq_context;
/* 中文逐项-变量 V2556：handle
 * handle（uint32_t；成员 ibv_cq）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 */
	uint32_t		handle;
/* 中文逐项-变量 V2557：cqe
 * cqe（int；成员 ibv_cq）：CQ最大条目能力或当前CQ容量，需容纳未被poll消耗的完成项。
 */
	int			cqe;

/* 中文逐项-变量 V2558：mutex
 * mutex（pthread_mutex_t；成员 ibv_cq）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
	pthread_mutex_t		mutex;
/* 中文逐项-变量 V2559：cond
 * cond（pthread_cond_t；成员 ibv_cq）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
	pthread_cond_t		cond;
/* 中文逐项-变量 V2560：comp_events_completed
 * comp_events_completed（uint32_t；成员 ibv_cq）：资源相关事件已确认/完成计数，销毁等待要保证异步事件引用被正确处理。
 */
	uint32_t		comp_events_completed;
/* 中文逐项-变量 V2561：async_events_completed
 * async_events_completed（uint32_t；成员 ibv_cq）：资源相关事件已确认/完成计数，销毁等待要保证异步事件引用被正确处理。
 */
	uint32_t		async_events_completed;
};

/* 中文逐项-类型 T0276：ibv_ah
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：context、pd、handle。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_ah {
/* 中文逐项-变量 V2562：context
 * context（struct ibv_context；成员 ibv_ah）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context     *context;
/* 中文逐项-变量 V2563：pd
 * pd（struct ibv_pd；成员 ibv_ah）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 */
	struct ibv_pd	       *pd;
/* 中文逐项-变量 V2564：handle
 * handle（uint32_t；成员 ibv_ah）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 */
	uint32_t		handle;
};

/* 中文逐项-类型 T0277：ibv_flow_flags
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_flow_flags {
/* 中文逐项-枚举项 X0560：IBV_FLOW_ATTR_FLAGS_ALLOW_LOOP_BACK
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
	IBV_FLOW_ATTR_FLAGS_ALLOW_LOOP_BACK = 1,
/* 中文逐项-枚举项 X0561：IBV_FLOW_ATTR_FLAGS_DONT_TRAP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1 << 1
 */
	IBV_FLOW_ATTR_FLAGS_DONT_TRAP = 1 << 1,
};

/* 中文逐项-类型 T0278：ibv_flow_attr_type
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_flow_attr_type {
	/* steering according to rule specifications */
/* 中文逐项-枚举项 X0562：IBV_FLOW_ATTR_NORMAL
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x0
 */
	IBV_FLOW_ATTR_NORMAL		= 0x0,
	/* default unicast and multicast rule -
	 * receive all Eth traffic which isn't steered to any QP
	 */
/* 中文逐项-枚举项 X0563：IBV_FLOW_ATTR_ALL_DEFAULT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x1
 */
	IBV_FLOW_ATTR_ALL_DEFAULT	= 0x1,
	/* default multicast rule -
	 * receive all Eth multicast traffic which isn't steered to any QP
	 */
/* 中文逐项-枚举项 X0564：IBV_FLOW_ATTR_MC_DEFAULT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x2
 */
	IBV_FLOW_ATTR_MC_DEFAULT	= 0x2,
};

/* 中文逐项-类型 T0279：ibv_flow_spec_type
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ibv_flow_spec_type {
/* 中文逐项-枚举项 X0565：IBV_FLOW_SPEC_ETH
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x20
 */
	IBV_FLOW_SPEC_ETH	= 0x20,
/* 中文逐项-枚举项 X0566：IBV_FLOW_SPEC_IPV4
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x30
 */
	IBV_FLOW_SPEC_IPV4	= 0x30,
/* 中文逐项-枚举项 X0567：IBV_FLOW_SPEC_TCP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x40
 */
	IBV_FLOW_SPEC_TCP	= 0x40,
/* 中文逐项-枚举项 X0568：IBV_FLOW_SPEC_UDP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0x41
 */
	IBV_FLOW_SPEC_UDP	= 0x41,
};

/* 中文逐项-类型 T0280：ibv_flow_eth_filter
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：dst_mac、src_mac、ether_type、vlan_tag。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow_eth_filter {
/* 中文逐项-变量 V2565：dst_mac
 * dst_mac（uint8_t；成员 ibv_flow_eth_filter）：以太网流过滤的源/目标MAC、EtherType及VLAN标签。
 */
	uint8_t		dst_mac[6];
/* 中文逐项-变量 V2566：src_mac
 * src_mac（uint8_t；成员 ibv_flow_eth_filter）：以太网流过滤的源/目标MAC、EtherType及VLAN标签。
 */
	uint8_t		src_mac[6];
/* 中文逐项-变量 V2567：ether_type
 * ether_type（uint16_t；成员 ibv_flow_eth_filter）：以太网流过滤的源/目标MAC、EtherType及VLAN标签。
 */
	uint16_t	ether_type;
	/*
	 * same layout as 802.1q: prio 3, cfi 1, vlan id 12
	 */
/* 中文逐项-变量 V2568：vlan_tag
 * vlan_tag（uint16_t；成员 ibv_flow_eth_filter）：以太网流过滤的源/目标MAC、EtherType及VLAN标签。
 */
	uint16_t	vlan_tag;
};

/* 中文逐项-类型 T0281：ibv_flow_spec_eth
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：type、size、val、mask。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow_spec_eth {
/* 中文逐项-变量 V2569：type
 * type（enum ibv_flow_spec_type；成员 ibv_flow_spec_eth）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
	enum ibv_flow_spec_type  type;
/* 中文逐项-变量 V2570：size
 * size（uint16_t；成员 ibv_flow_spec_eth）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
	uint16_t  size;
/* 中文逐项-变量 V2571：val
 * val（struct ibv_flow_eth_filter；成员 ibv_flow_spec_eth）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 */
	struct ibv_flow_eth_filter val;
/* 中文逐项-变量 V2572：mask
 * mask（struct ibv_flow_eth_filter；成员 ibv_flow_spec_eth）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 */
	struct ibv_flow_eth_filter mask;
};

/* 中文逐项-类型 T0282：ibv_flow_ipv4_filter
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：src_ip、dst_ip。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow_ipv4_filter {
/* 中文逐项-变量 V2573：src_ip
 * src_ip（uint32_t；成员 ibv_flow_ipv4_filter）：IPv4流过滤的源/目标地址位模式，结合过滤mask选择匹配位。
 */
	uint32_t src_ip;
/* 中文逐项-变量 V2574：dst_ip
 * dst_ip（uint32_t；成员 ibv_flow_ipv4_filter）：IPv4流过滤的源/目标地址位模式，结合过滤mask选择匹配位。
 */
	uint32_t dst_ip;
};

/* 中文逐项-类型 T0283：ibv_flow_spec_ipv4
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：type、size、val、mask。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow_spec_ipv4 {
/* 中文逐项-变量 V2575：type
 * type（enum ibv_flow_spec_type；成员 ibv_flow_spec_ipv4）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
	enum ibv_flow_spec_type  type;
/* 中文逐项-变量 V2576：size
 * size（uint16_t；成员 ibv_flow_spec_ipv4）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
	uint16_t  size;
/* 中文逐项-变量 V2577：val
 * val（struct ibv_flow_ipv4_filter；成员 ibv_flow_spec_ipv4）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 */
	struct ibv_flow_ipv4_filter val;
/* 中文逐项-变量 V2578：mask
 * mask（struct ibv_flow_ipv4_filter；成员 ibv_flow_spec_ipv4）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 */
	struct ibv_flow_ipv4_filter mask;
};

/* 中文逐项-类型 T0284：ibv_flow_tcp_udp_filter
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：dst_port、src_port。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow_tcp_udp_filter {
/* 中文逐项-变量 V2579：dst_port
 * dst_port（uint16_t；成员 ibv_flow_tcp_udp_filter）：TCP/UDP流过滤的源/目标端口，注意网络字节序和对应mask。
 */
	uint16_t dst_port;
/* 中文逐项-变量 V2580：src_port
 * src_port（uint16_t；成员 ibv_flow_tcp_udp_filter）：TCP/UDP流过滤的源/目标端口，注意网络字节序和对应mask。
 */
	uint16_t src_port;
};

/* 中文逐项-类型 T0285：ibv_flow_spec_tcp_udp
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：type、size、val、mask。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow_spec_tcp_udp {
/* 中文逐项-变量 V2581：type
 * type（enum ibv_flow_spec_type；成员 ibv_flow_spec_tcp_udp）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
	enum ibv_flow_spec_type  type;
/* 中文逐项-变量 V2582：size
 * size（uint16_t；成员 ibv_flow_spec_tcp_udp）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
	uint16_t  size;
/* 中文逐项-变量 V2583：val
 * val（struct ibv_flow_tcp_udp_filter；成员 ibv_flow_spec_tcp_udp）：属性/位域/流过滤的值视图，按宿主类型解释；若是过滤值则与mask共同选择有效匹配位。
 */
	struct ibv_flow_tcp_udp_filter val;
/* 中文逐项-变量 V2584：mask
 * mask（struct ibv_flow_tcp_udp_filter；成员 ibv_flow_spec_tcp_udp）：位掩码或流过滤选择掩码，决定哪些位/状态参与匹配，不等于被匹配的数据值。
 */
	struct ibv_flow_tcp_udp_filter mask;
};

/* 中文逐项-类型 T0286：ibv_flow_spec
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：hdr、type、size、eth、ipv4、tcp_udp。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow_spec {
/* 中文逐项-类型 T0287：<匿名union>
 * 类型职责：内嵌匿名联合体：各成员复用同一段内存，仅按外层类型/状态选择有效视图。
 * 声明内可见成员（含嵌套成员）：hdr、type、size、eth、ipv4、tcp_udp。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 * 存储规则：成员共享同一段存储；写一种视图后应按本协议约定解释另一视图，不能把所有成员当作独立有效对象。
 */
	union {
/* 中文逐项-变量 V2585：hdr
 * hdr（struct { enum ibv_flow_spec_type type; uint16_t size; }；成员
 * ibv_flow_spec::<匿名>）：流过滤规格union的公共头或具体协议视图，由type决定有效布局。
 */
/* 中文逐项-类型 T0288：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：hdr、type、size。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
		struct {
/* 中文逐项-变量 V2586：type
 * type（enum ibv_flow_spec_type；成员 ibv_flow_spec::<匿名>::<匿名>）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
			enum ibv_flow_spec_type	type;
/* 中文逐项-变量 V2587：size
 * size（uint16_t；成员 ibv_flow_spec::<匿名>::<匿名>）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
			uint16_t		size;
		} hdr;
/* 中文逐项-变量 V2588：eth
 * eth（struct ibv_flow_spec_eth；成员 ibv_flow_spec::<匿名>）：流过滤规格union的公共头或具体协议视图，由type决定有效布局。
 */
		struct ibv_flow_spec_eth eth;
/* 中文逐项-变量 V2589：ipv4
 * ipv4（struct ibv_flow_spec_ipv4；成员 ibv_flow_spec::<匿名>）：流过滤规格union的公共头或具体协议视图，由type决定有效布局。
 */
		struct ibv_flow_spec_ipv4 ipv4;
/* 中文逐项-变量 V2590：tcp_udp
 * tcp_udp（struct ibv_flow_spec_tcp_udp；成员 ibv_flow_spec::<匿名>）：流过滤规格union的公共头或具体协议视图，由type决定有效布局。
 */
		struct ibv_flow_spec_tcp_udp tcp_udp;
	};
};

/* 中文逐项-类型 T0289：ibv_flow_attr
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：comp_mask、type、size、priority、num_of_specs、port、flags。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow_attr {
/* 中文逐项-变量 V2591：comp_mask
 * comp_mask（uint32_t；成员 ibv_flow_attr）：结构体可选扩展字段有效性掩码，调用者只允许使用当前ABI支持的字段。
 */
	uint32_t comp_mask;
/* 中文逐项-变量 V2592：type
 * type（enum ibv_flow_attr_type；成员 ibv_flow_attr）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
	enum ibv_flow_attr_type type;
/* 中文逐项-变量 V2593：size
 * size（uint16_t；成员 ibv_flow_attr）：当前对象/消息范围的尺寸；按声明及初始化式辨认字节数、元素数或条目数。
 */
	uint16_t size;
/* 中文逐项-变量 V2594：priority
 * priority（uint16_t；成员 ibv_flow_attr）：流转发/过滤规则优先级及后随的规格项数量。
 */
	uint16_t priority;
/* 中文逐项-变量 V2595：num_of_specs
 * num_of_specs（uint8_t；成员 ibv_flow_attr）：流转发/过滤规则优先级及后随的规格项数量。
 */
	uint8_t num_of_specs;
/* 中文逐项-变量 V2596：port
 * port（uint8_t；成员 ibv_flow_attr）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 */
	uint8_t port;
/* 中文逐项-变量 V2597：flags
 * flags（uint32_t；成员 ibv_flow_attr）：按位组合的当前线程角色/直接访问/共享偏移/中止等状态，按位测试而非单一枚举。
 */
	uint32_t flags;
	/* Following are the optional layers according to user request
	 * struct ibv_flow_spec_xxx [L2]
	 * struct ibv_flow_spec_yyy [L3/L4]
	 */
};

/* 中文逐项-类型 T0290：ibv_flow
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：comp_mask、context、handle。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_flow {
/* 中文逐项-变量 V2598：comp_mask
 * comp_mask（uint32_t；成员 ibv_flow）：结构体可选扩展字段有效性掩码，调用者只允许使用当前ABI支持的字段。
 */
	uint32_t	   comp_mask;
/* 中文逐项-变量 V2599：context
 * context（struct ibv_context；成员 ibv_flow）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context *context;
/* 中文逐项-变量 V2600：handle
 * handle（uint32_t；成员 ibv_flow）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 */
	uint32_t	   handle;
};

struct ibv_device;
struct ibv_context;

/* 中文逐项-类型 T0291：ibv_device_ops
 * 类型职责：verbs对应资源/属性/能力的ABI表示，字段必须与动态链接的libibverbs二进制布局相容；存在定义不意味着NCCL运行时启用了该功能。
 * 声明内可见成员（含嵌套成员）：alloc_context、free_context。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_device_ops {
/* 中文逐项-变量 V2601：alloc_context
 * alloc_context 是函数指针字段/变量：由该资源库/provider提供的alloc_context操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*alloc_context)(struct ibv_device *device, int cmd_fd)。
 * alloc_context（struct ibv_context；成员 ibv_device_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_context *	(*alloc_context)(struct ibv_device *device, int cmd_fd);
/* 中文逐项-变量 V2602：free_context
 * free_context
 * 是函数指针字段/变量：由该资源库/provider提供的free_context操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*free_context)(struct
 * ibv_context *context)。
 * 回调参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * free_context（void；成员 ibv_device_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	void			(*free_context)(struct ibv_context *context);
};

/* 中文逐项-类型 T0292：<匿名enum>
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum {
/* 中文逐项-枚举项 X0569：IBV_SYSFS_NAME_MAX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：64
 */
	IBV_SYSFS_NAME_MAX	= 64,
/* 中文逐项-枚举项 X0570：IBV_SYSFS_PATH_MAX
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：256
 */
	IBV_SYSFS_PATH_MAX	= 256
};

/* 中文逐项-类型 T0293：ibv_device
 * 类型职责：设备上下文、provider操作表和扩展ABI布局，把通用verbs调用接到厂商实现；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：ops、node_type、transport_type、name、dev_name、dev_path、ibdev_path。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_device {
/* 中文逐项-变量 V2603：ops
 * ops（struct ibv_device_ops；成员 ibv_device）：所处类型的provider函数表、代理待办数组或任务链，按声明类型读取，不能跨类型强转解释。
 */
	struct ibv_device_ops	ops;
/* 中文逐项-变量 V2604：node_type
 * node_type（enum ibv_node_type；成员 ibv_device）：verbs设备节点与底层传输类型枚举，辅助判断IB/RoCE等能力。
 */
	enum ibv_node_type	node_type;
/* 中文逐项-变量 V2605：transport_type
 * transport_type（enum ibv_transport_type；成员 ibv_device）：verbs设备节点与底层传输类型枚举，辅助判断IB/RoCE等能力。
 */
	enum ibv_transport_type	transport_type;
	/* Name of underlying kernel IB device, eg "mthca0" */
/* 中文逐项-变量 V2606：name
 * name（char；成员 ibv_device）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
	char			name[IBV_SYSFS_NAME_MAX];
	/* Name of uverbs device, eg "uverbs0" */
/* 中文逐项-变量 V2607：dev_name
 * dev_name（char；成员 ibv_device）：verbs设备名称及sysfs路径文本，关联物理NIC拓扑。
 */
	char			dev_name[IBV_SYSFS_NAME_MAX];
	/* Path to infiniband_verbs class device in sysfs */
/* 中文逐项-变量 V2608：dev_path
 * dev_path（char；成员 ibv_device）：verbs设备名称及sysfs路径文本，关联物理NIC拓扑。
 */
	char			dev_path[IBV_SYSFS_PATH_MAX];
	/* Path to infiniband class device in sysfs */
/* 中文逐项-变量 V2609：ibdev_path
 * ibdev_path（char；成员 ibv_device）：verbs设备名称及sysfs路径文本，关联物理NIC拓扑。
 */
	char			ibdev_path[IBV_SYSFS_PATH_MAX];
};

/* 中文逐项-类型 T0294：verbs_device
 * 类型职责：设备上下文、provider操作表和扩展ABI布局，把通用verbs调用接到厂商实现；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：device、sz、size_of_context、init_context、uninit_context。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct verbs_device {
/* 中文逐项-变量 V2610：device
 * device（struct ibv_device；成员 verbs_device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 */
	struct ibv_device device; /* Must be first */
/* 中文逐项-变量 V2611：sz
 * sz（size_t；成员 verbs_device）：扩展ABI结构体容量与需要的上下文大小，用于版本兼容判断。
 */
	size_t	sz;
/* 中文逐项-变量 V2612：size_of_context
 * size_of_context（size_t；成员 verbs_device）：扩展ABI结构体容量与需要的上下文大小，用于版本兼容判断。
 */
	size_t	size_of_context;
/* 中文逐项-变量 V2613：init_context
 * init_context
 * 是函数指针字段/变量：由该资源库/provider提供的init_context操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*init_context)(struct
 * verbs_device *device, struct ibv_context *ctx, int cmd_fd)。
 * 回调参数 device（struct verbs_device *device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 回调参数 ctx（struct ibv_context *ctx）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 回调参数 cmd_fd（int cmd_fd）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 * init_context（int；成员 verbs_device）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int	(*init_context)(struct verbs_device *device,
				struct ibv_context *ctx, int cmd_fd);
/* 中文逐项-变量 V2614：uninit_context
 * uninit_context
 * 是函数指针字段/变量：由该资源库/provider提供的uninit_context操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*uninit_context)(struct
 * verbs_device *device, struct ibv_context *ctx)。
 * 回调参数 device（struct verbs_device *device）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 * 回调参数 ctx（struct ibv_context *ctx）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * uninit_context（void；成员 verbs_device）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	void	(*uninit_context)(struct verbs_device *device,
				struct ibv_context *ctx);
	/* future fields added here */
};

/* 中文逐项-类型 T0295：ibv_context_ops
 * 类型职责：设备上下文、provider操作表和扩展ABI布局，把通用verbs调用接到厂商实现；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：query_device、query_port、alloc_pd、dealloc_pd、reg_mr、rereg_mr、dereg_mr、alloc_mw、bind_mw、dealloc_mw、create_cq、poll_cq、req_notify_cq、cq_event、resize_cq、destroy_cq、create_srq、modify_srq、query_srq、destroy_srq、post_srq_recv、create_qp、query_qp、modify_qp、destroy_qp、post_send、post_recv、create_ah、destroy_ah、attach_mcast、detach_mcast、async_event。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_context_ops {
/* 中文逐项-变量 V2615：query_device
 * query_device 是函数指针字段/变量：读取设备能力限制；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：(*query_device)(struct ibv_context *context, struct ibv_device_attr *device_attr)。
 * 回调参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 回调参数 device_attr（struct ibv_device_attr *device_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * query_device（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*query_device)(struct ibv_context *context,
					      struct ibv_device_attr *device_attr);
/* 中文逐项-变量 V2616：query_port
 * query_port 是函数指针字段/变量：读取端口状态/地址与速率能力；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：(*query_port)(struct ibv_context *context, uint8_t port_num, struct ibv_port_attr
 * *port_attr)。
 * 回调参数 context（struct ibv_context *context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 * 回调参数 port_num（uint8_t port_num）：设备物理端口或socket端口编号，按所属结构体区分RDMA端口与TCP/UDP端口。
 * 回调参数 port_attr（struct ibv_port_attr *port_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * query_port（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*query_port)(struct ibv_context *context, uint8_t port_num,
					      struct ibv_port_attr *port_attr);
/* 中文逐项-变量 V2617：alloc_pd
 * alloc_pd 是函数指针字段/变量：分配保护域，隔离允许访问的 QP/MR 集合；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*alloc_pd)(struct ibv_context *context)。
 * alloc_pd（struct ibv_pd；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_pd *		(*alloc_pd)(struct ibv_context *context);
/* 中文逐项-变量 V2618：dealloc_pd
 * dealloc_pd 是函数指针字段/变量：释放保护域；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：(*dealloc_pd)(struct ibv_pd *pd)。
 * 回调参数 pd（struct ibv_pd *pd）：RDMA 保护域，关联允许互相访问的 QP、MR 等对象，不能假设不同PD的key可互用。
 * dealloc_pd（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*dealloc_pd)(struct ibv_pd *pd);
/* 中文逐项-变量 V2619：reg_mr
 * reg_mr 是函数指针字段/变量：把指定虚拟地址范围注册到 PD 并取得 lkey/rkey；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：* (*reg_mr)(struct ibv_pd *pd, void *addr, size_t length, int access)。
 * reg_mr（struct ibv_mr；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_mr *		(*reg_mr)(struct ibv_pd *pd, void *addr, size_t length,
					  int access);
/* 中文逐项-变量 V2620：rereg_mr
 * rereg_mr 是函数指针字段/变量：由该资源库/provider提供的rereg_mr操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：* (*rereg_mr)(struct
 * ibv_mr *mr, int flags, struct ibv_pd *pd, void *addr, size_t length, int access)。
 * rereg_mr（struct ibv_mr；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_mr *		(*rereg_mr)(struct ibv_mr *mr,
					    int flags,
					    struct ibv_pd *pd, void *addr,
					    size_t length,
					    int access);
/* 中文逐项-变量 V2621：dereg_mr
 * dereg_mr 是函数指针字段/变量：注销一个真实 MR；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：(*dereg_mr)(struct ibv_mr *mr)。
 * 回调参数 mr（struct ibv_mr *mr）：真实 verbs 内存注册对象，包含本地/远端访问key，可能由注册缓存共享。
 * dereg_mr（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*dereg_mr)(struct ibv_mr *mr);
/* 中文逐项-变量 V2622：alloc_mw
 * alloc_mw 是函数指针字段/变量：由该资源库/provider提供的alloc_mw操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：* (*alloc_mw)(struct
 * ibv_pd *pd, enum ibv_mw_type type)。
 * alloc_mw（struct ibv_mw；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_mw *		(*alloc_mw)(struct ibv_pd *pd, enum ibv_mw_type type);
/* 中文逐项-变量 V2623：bind_mw
 * bind_mw 是函数指针字段/变量：由该资源库/provider提供的bind_mw操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*bind_mw)(struct ibv_qp
 * *qp, struct ibv_mw *mw, struct ibv_mw_bind *mw_bind)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 mw（struct ibv_mw *mw）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 mw_bind（struct ibv_mw_bind *mw_bind）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * bind_mw（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*bind_mw)(struct ibv_qp *qp, struct ibv_mw *mw,
					   struct ibv_mw_bind *mw_bind);
/* 中文逐项-变量 V2624：dealloc_mw
 * dealloc_mw 是函数指针字段/变量：由该资源库/provider提供的dealloc_mw操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*dealloc_mw)(struct
 * ibv_mw *mw)。
 * 回调参数 mw（struct ibv_mw *mw）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * dealloc_mw（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*dealloc_mw)(struct ibv_mw *mw);
/* 中文逐项-变量 V2625：create_cq
 * create_cq 是函数指针字段/变量：创建完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*create_cq)(struct ibv_context *context, int cqe, struct ibv_comp_channel *channel, int comp_vector)。
 * create_cq（struct ibv_cq；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_cq *		(*create_cq)(struct ibv_context *context, int cqe,
					     struct ibv_comp_channel *channel,
					     int comp_vector);
/* 中文逐项-变量 V2626：poll_cq
 * poll_cq 是函数指针字段/变量：从 CQ 轮询完成项并统一错误处理，返回数量可为零而不代表失败；函数表保存可调用入口，不是调用返回的数据。完整声明：(*poll_cq)(struct ibv_cq *cq, int
 * num_entries, struct ibv_wc *wc)。
 * 回调参数 cq（struct ibv_cq *cq）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 * 回调参数 num_entries（int num_entries）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 回调参数 wc（struct ibv_wc *wc）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * poll_cq（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*poll_cq)(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc);
/* 中文逐项-变量 V2627：req_notify_cq
 * req_notify_cq
 * 是函数指针字段/变量：由该资源库/provider提供的req_notify_cq操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*req_notify_cq)(struct
 * ibv_cq *cq, int solicited_only)。
 * 回调参数 cq（struct ibv_cq *cq）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 * 回调参数 solicited_only（int solicited_only）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * req_notify_cq（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*req_notify_cq)(struct ibv_cq *cq, int solicited_only);
/* 中文逐项-变量 V2628：cq_event
 * cq_event 是函数指针字段/变量：由该资源库/provider提供的cq_event操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*cq_event)(struct
 * ibv_cq *cq)。
 * 回调参数 cq（struct ibv_cq *cq）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 * cq_event（void；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	void			(*cq_event)(struct ibv_cq *cq);
/* 中文逐项-变量 V2629：resize_cq
 * resize_cq 是函数指针字段/变量：由该资源库/provider提供的resize_cq操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*resize_cq)(struct
 * ibv_cq *cq, int cqe)。
 * 回调参数 cq（struct ibv_cq *cq）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 * 回调参数 cqe（int cqe）：CQ最大条目能力或当前CQ容量，需容纳未被poll消耗的完成项。
 * resize_cq（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*resize_cq)(struct ibv_cq *cq, int cqe);
/* 中文逐项-变量 V2630：destroy_cq
 * destroy_cq 是函数指针字段/变量：销毁完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：(*destroy_cq)(struct ibv_cq *cq)。
 * 回调参数 cq（struct ibv_cq *cq）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 * destroy_cq（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*destroy_cq)(struct ibv_cq *cq);
/* 中文逐项-变量 V2631：create_srq
 * create_srq 是函数指针字段/变量：由该资源库/provider提供的create_srq操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*create_srq)(struct ibv_pd *pd, struct ibv_srq_init_attr *srq_init_attr)。
 * create_srq（struct ibv_srq；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_srq *	(*create_srq)(struct ibv_pd *pd,
					      struct ibv_srq_init_attr *srq_init_attr);
/* 中文逐项-变量 V2632：modify_srq
 * modify_srq 是函数指针字段/变量：由该资源库/provider提供的modify_srq操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*modify_srq)(struct
 * ibv_srq *srq, struct ibv_srq_attr *srq_attr, int srq_attr_mask)。
 * 回调参数 srq（struct ibv_srq *srq）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 * 回调参数 srq_attr（struct ibv_srq_attr *srq_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 srq_attr_mask（int srq_attr_mask）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * modify_srq（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*modify_srq)(struct ibv_srq *srq,
					      struct ibv_srq_attr *srq_attr,
					      int srq_attr_mask);
/* 中文逐项-变量 V2633：query_srq
 * query_srq 是函数指针字段/变量：由该资源库/provider提供的query_srq操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*query_srq)(struct
 * ibv_srq *srq, struct ibv_srq_attr *srq_attr)。
 * 回调参数 srq（struct ibv_srq *srq）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 * 回调参数 srq_attr（struct ibv_srq_attr *srq_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * query_srq（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*query_srq)(struct ibv_srq *srq,
					     struct ibv_srq_attr *srq_attr);
/* 中文逐项-变量 V2634：destroy_srq
 * destroy_srq
 * 是函数指针字段/变量：由该资源库/provider提供的destroy_srq操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*destroy_srq)(struct ibv_srq
 * *srq)。
 * 回调参数 srq（struct ibv_srq *srq）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 * destroy_srq（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*destroy_srq)(struct ibv_srq *srq);
/* 中文逐项-变量 V2635：post_srq_recv
 * post_srq_recv
 * 是函数指针字段/变量：由该资源库/provider提供的post_srq_recv操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*post_srq_recv)(struct
 * ibv_srq *srq, struct ibv_recv_wr *recv_wr, struct ibv_recv_wr **bad_recv_wr)。
 * 回调参数 srq（struct ibv_srq *srq）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 * 回调参数 recv_wr（struct ibv_recv_wr *recv_wr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 回调参数 bad_recv_wr（struct ibv_recv_wr **bad_recv_wr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * post_srq_recv（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*post_srq_recv)(struct ibv_srq *srq,
						 struct ibv_recv_wr *recv_wr,
						 struct ibv_recv_wr **bad_recv_wr);
/* 中文逐项-变量 V2636：create_qp
 * create_qp 是函数指针字段/变量：创建指定类型和队列容量的 QP；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*create_qp)(struct ibv_pd *pd, struct ibv_qp_init_attr *attr)。
 * create_qp（struct ibv_qp；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_qp *		(*create_qp)(struct ibv_pd *pd, struct ibv_qp_init_attr *attr);
/* 中文逐项-变量 V2637：query_qp
 * query_qp 是函数指针字段/变量：查询 QP 当前状态和属性；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：(*query_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask, struct
 * ibv_qp_init_attr *init_attr)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 attr（struct ibv_qp_attr *attr）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 回调参数 attr_mask（int attr_mask）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 回调参数 init_attr（struct ibv_qp_init_attr *init_attr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * query_qp（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*query_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr,
					    int attr_mask,
					    struct ibv_qp_init_attr *init_attr);
/* 中文逐项-变量 V2638：modify_qp
 * modify_qp 是函数指针字段/变量：按属性掩码修改 QP 状态/路由/权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：(*modify_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 attr（struct ibv_qp_attr *attr）：创建/修改资源所需属性集合，具体布局由所属srq/qp等对象决定。
 * 回调参数 attr_mask（int attr_mask）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * modify_qp（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*modify_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr,
					     int attr_mask);
/* 中文逐项-变量 V2639：destroy_qp
 * destroy_qp 是函数指针字段/变量：销毁队列对；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider
 * 执行；函数表保存可调用入口，不是调用返回的数据。完整声明：(*destroy_qp)(struct ibv_qp *qp)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * destroy_qp（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*destroy_qp)(struct ibv_qp *qp);
/* 中文逐项-变量 V2640：post_send
 * post_send 是函数指针字段/变量：通过 QP 的 provider 操作表提交发送工作请求链，立即返回仅表示提交，不代表载荷完成；函数表保存可调用入口，不是调用返回的数据。完整声明：(*post_send)(struct
 * ibv_qp *qp, struct ibv_send_wr *wr, struct ibv_send_wr **bad_wr)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 wr（struct ibv_send_wr *wr）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 回调参数 bad_wr（struct ibv_send_wr **bad_wr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * post_send（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*post_send)(struct ibv_qp *qp, struct ibv_send_wr *wr,
					     struct ibv_send_wr **bad_wr);
/* 中文逐项-变量 V2641：post_recv
 * post_recv 是函数指针字段/变量：向指定 QP 的接收队列投递 WQE，用于 IB 接收通知；该接口不自动变成 SRQ；函数表保存可调用入口，不是调用返回的数据。完整声明：(*post_recv)(struct
 * ibv_qp *qp, struct ibv_recv_wr *wr, struct ibv_recv_wr **bad_wr)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 wr（struct ibv_recv_wr *wr）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 回调参数 bad_wr（struct ibv_recv_wr **bad_wr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * post_recv（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*post_recv)(struct ibv_qp *qp, struct ibv_recv_wr *wr,
					     struct ibv_recv_wr **bad_wr);
/* 中文逐项-变量 V2642：create_ah
 * create_ah 是函数指针字段/变量：由该资源库/provider提供的create_ah操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：* (*create_ah)(struct
 * ibv_pd *pd, struct ibv_ah_attr *attr)。
 * create_ah（struct ibv_ah；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_ah *		(*create_ah)(struct ibv_pd *pd, struct ibv_ah_attr *attr);
/* 中文逐项-变量 V2643：destroy_ah
 * destroy_ah 是函数指针字段/变量：由该资源库/provider提供的destroy_ah操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*destroy_ah)(struct
 * ibv_ah *ah)。
 * 回调参数 ah（struct ibv_ah *ah）：地址句柄，描述UD等非连接路径的寻址属性。
 * destroy_ah（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*destroy_ah)(struct ibv_ah *ah);
/* 中文逐项-变量 V2644：attach_mcast
 * attach_mcast
 * 是函数指针字段/变量：由该资源库/provider提供的attach_mcast操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*attach_mcast)(struct ibv_qp
 * *qp, const union ibv_gid *gid, uint16_t lid)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 gid（const union ibv_gid *gid）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 * 回调参数 lid（uint16_t lid）：IB本地/目标/子网管理器/源LID地址信息，RoCE全球路由通常改用GID路径。
 * attach_mcast（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*attach_mcast)(struct ibv_qp *qp, const union ibv_gid *gid,
						uint16_t lid);
/* 中文逐项-变量 V2645：detach_mcast
 * detach_mcast
 * 是函数指针字段/变量：由该资源库/provider提供的detach_mcast操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*detach_mcast)(struct ibv_qp
 * *qp, const union ibv_gid *gid, uint16_t lid)。
 * 回调参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 回调参数 gid（const union ibv_gid *gid）：本地/源/目标GID及源GID表索引，用于RoCE或IB全局路由。
 * 回调参数 lid（uint16_t lid）：IB本地/目标/子网管理器/源LID地址信息，RoCE全球路由通常改用GID路径。
 * detach_mcast（int；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int			(*detach_mcast)(struct ibv_qp *qp, const union ibv_gid *gid,
						uint16_t lid);
/* 中文逐项-变量 V2646：async_event
 * async_event
 * 是函数指针字段/变量：由该资源库/provider提供的async_event操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*async_event)(struct
 * ibv_async_event *event)。
 * 回调参数 event（struct ibv_async_event *event）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * async_event（void；成员 ibv_context_ops）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	void			(*async_event)(struct ibv_async_event *event);
};

/* 中文逐项-类型 T0296：ibv_context
 * 类型职责：设备上下文、provider操作表和扩展ABI布局，把通用verbs调用接到厂商实现；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：device、ops、cmd_fd、async_fd、num_comp_vectors、mutex、abi_compat。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ibv_context {
/* 中文逐项-变量 V2647：device
 * device（struct ibv_device；成员 ibv_context）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 */
	struct ibv_device      *device;
/* 中文逐项-变量 V2648：ops
 * ops（struct ibv_context_ops；成员 ibv_context）：所处类型的provider函数表、代理待办数组或任务链，按声明类型读取，不能跨类型强转解释。
 */
	struct ibv_context_ops	ops;
/* 中文逐项-变量 V2649：cmd_fd
 * cmd_fd（int；成员 ibv_context）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 */
	int			cmd_fd;
/* 中文逐项-变量 V2650：async_fd
 * async_fd（int；成员 ibv_context）：系统文件描述符：socket或verbs命令/异步事件通道，按所属类型辨认。
 */
	int			async_fd;
/* 中文逐项-变量 V2651：num_comp_vectors
 * num_comp_vectors（int；成员 ibv_context）：设备支持的完成中断向量数，决定可选CQ事件向量范围。
 */
	int			num_comp_vectors;
/* 中文逐项-变量 V2652：mutex
 * mutex（pthread_mutex_t；成员 ibv_context）：CPU 线程/进程间同步原语；锁保护元数据，条件变量唤醒等待，与 GPU head/tail 协议不同。
 */
	pthread_mutex_t		mutex;
/* 中文逐项-变量 V2653：abi_compat
 * abi_compat（void；成员 ibv_context）：历史verbs二进制兼容保留字段，必须保持布局，不能当作NCCL实际启用的XRC配置。
 */
	void		       *abi_compat;
};

/* 中文逐项-类型 T0297：verbs_context_mask
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum verbs_context_mask {
/* 中文逐项-枚举项 X0571：VERBS_CONTEXT_XRCD
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(uint64_t)1 << 0
 */
	VERBS_CONTEXT_XRCD         = (uint64_t)1 << 0,
/* 中文逐项-枚举项 X0572：VERBS_CONTEXT_SRQ
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(uint64_t)1 << 1
 */
	VERBS_CONTEXT_SRQ          = (uint64_t)1 << 1,
/* 中文逐项-枚举项 X0573：VERBS_CONTEXT_QP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(uint64_t)1 << 2
 */
	VERBS_CONTEXT_QP           = (uint64_t)1 << 2,
/* 中文逐项-枚举项 X0574：VERBS_CONTEXT_RESERVED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(uint64_t)1 << 3
 */
	VERBS_CONTEXT_RESERVED     = (uint64_t)1 << 3,
/* 中文逐项-枚举项 X0575：VERBS_CONTEXT_EXP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：(uint64_t)1 << 62
 */
	VERBS_CONTEXT_EXP	   = (uint64_t)1 << 62
};

/* 中文逐项-类型 T0298：verbs_context
 * 类型职责：设备上下文、provider操作表和扩展ABI布局，把通用verbs调用接到厂商实现；本文件为动态调用保留必要ABI定义，修改成员布局会破坏二进制兼容。
 * 声明内可见成员（含嵌套成员）：_reserved_2、destroy_flow、_reserved_1、create_flow、open_qp、create_qp_ex、get_srq_num、create_srq_ex、open_xrcd、close_xrcd、has_comp_mask、sz、context。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct verbs_context {
	/*  "grows up" - new fields go here */
/* 中文逐项-变量 V2654：_reserved_2
 * _reserved_2 是函数指针字段/变量：由该资源库/provider提供的_reserved_2操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*_reserved_2)
 * (void)。
 * 回调参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * _reserved_2（int；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int (*_reserved_2) (void);
/* 中文逐项-变量 V2655：destroy_flow
 * destroy_flow 是函数指针字段/变量：由该资源库/provider提供的destroy_flow操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*destroy_flow)
 * (struct ibv_flow *flow)。
 * 回调参数 flow（struct ibv_flow *flow）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * destroy_flow（int；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int (*destroy_flow) (struct ibv_flow *flow);
/* 中文逐项-变量 V2656：_reserved_1
 * _reserved_1 是函数指针字段/变量：由该资源库/provider提供的_reserved_1操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*_reserved_1)
 * (void)。
 * 回调参数 <未命名参数>（void）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * _reserved_1（int；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int (*_reserved_1) (void);
/* 中文逐项-变量 V2657：create_flow
 * create_flow 是函数指针字段/变量：由该资源库/provider提供的create_flow操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：* (*create_flow)
 * (struct ibv_qp *qp, struct ibv_flow_attr *flow_attr)。
 * create_flow（struct ibv_flow；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_flow * (*create_flow) (struct ibv_qp *qp,
					  struct ibv_flow_attr *flow_attr);
/* 中文逐项-变量 V2658：open_qp
 * open_qp 是函数指针字段/变量：由该资源库/provider提供的open_qp操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：* (*open_qp)(struct
 * ibv_context *context, struct ibv_qp_open_attr *attr)。
 * open_qp（struct ibv_qp；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_qp * (*open_qp)(struct ibv_context *context,
			struct ibv_qp_open_attr *attr);
/* 中文逐项-变量 V2659：create_qp_ex
 * create_qp_ex 是函数指针字段/变量：由该资源库/provider提供的create_qp_ex操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*create_qp_ex)(struct ibv_context *context, struct ibv_qp_init_attr_ex *qp_init_attr_ex)。
 * create_qp_ex（struct ibv_qp；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_qp * (*create_qp_ex)(struct ibv_context *context,
			struct ibv_qp_init_attr_ex *qp_init_attr_ex);
/* 中文逐项-变量 V2660：get_srq_num
 * get_srq_num
 * 是函数指针字段/变量：由该资源库/provider提供的get_srq_num操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*get_srq_num)(struct ibv_srq
 * *srq, uint32_t *srq_num)。
 * 回调参数 srq（struct ibv_srq *srq）：共享接收队列引用；本头文件声明此能力不代表内置IB路径实际绑定或使用它。
 * 回调参数 srq_num（uint32_t *srq_num）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * get_srq_num（int；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int (*get_srq_num)(struct ibv_srq *srq, uint32_t *srq_num);
/* 中文逐项-变量 V2661：create_srq_ex
 * create_srq_ex 是函数指针字段/变量：由该资源库/provider提供的create_srq_ex操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：*
 * (*create_srq_ex)(struct ibv_context *context, struct ibv_srq_init_attr_ex *srq_init_attr_ex)。
 * create_srq_ex（struct ibv_srq；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_srq * (*create_srq_ex)(struct ibv_context *context,
			struct ibv_srq_init_attr_ex *srq_init_attr_ex);
/* 中文逐项-变量 V2662：open_xrcd
 * open_xrcd 是函数指针字段/变量：由该资源库/provider提供的open_xrcd操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：* (*open_xrcd)(struct
 * ibv_context *context, struct ibv_xrcd_init_attr *xrcd_init_attr)。
 * open_xrcd（struct ibv_xrcd；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	struct ibv_xrcd * (*open_xrcd)(struct ibv_context *context,
			struct ibv_xrcd_init_attr *xrcd_init_attr);
/* 中文逐项-变量 V2663：close_xrcd
 * close_xrcd 是函数指针字段/变量：由该资源库/provider提供的close_xrcd操作入口，具体参数和返回类型见完整声明；函数表保存可调用入口，不是调用返回的数据。完整声明：(*close_xrcd)(struct
 * ibv_xrcd *xrcd)。
 * 回调参数 xrcd（struct ibv_xrcd *xrcd）：扩展可靠连接域相关对象，用于XRC机制，与普通RC数据QP路径区分。
 * close_xrcd（int；成员 verbs_context）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
	int  (*close_xrcd)(struct ibv_xrcd *xrcd);
/* 中文逐项-变量 V2664：has_comp_mask
 * has_comp_mask（uint64_t；成员 verbs_context）：扩展verbs上下文已具备哪些功能字段的掩码，读取新增操作前须检测。
 */
	uint64_t has_comp_mask;
/* 中文逐项-变量 V2665：sz
 * sz（size_t；成员 verbs_context）：扩展ABI结构体容量与需要的上下文大小，用于版本兼容判断。
 */
	size_t   sz;	/* Must be immediately before struct ibv_context */
/* 中文逐项-变量 V2666：context
 * context（struct ibv_context；成员 verbs_context）：verbs/CUDA/回调的运行上下文句柄，具体对象由声明类型决定，不是用户张量指针。
 */
	struct ibv_context context;/* Must be last field in the struct */
};

/*XXX:__VERBS_ABI_IS_EXTENDED produces warning "integer operation result is out of range" with g++ 4.8.2*/
/*static inline struct verbs_context *verbs_get_ctx(struct ibv_context *ctx)
{
	return (!ctx || (ctx->abi_compat != __VERBS_ABI_IS_EXTENDED)) ?
		NULL : container_of(ctx, struct verbs_context, context);
}

#define verbs_get_ctx_op(ctx, op) ({ \
	struct verbs_context *_vctx = verbs_get_ctx(ctx); \
	(!_vctx || (_vctx->sz < sizeof(*_vctx) - offsetof(struct verbs_context, op)) || \
	!_vctx->op) ? NULL : _vctx; })*/

/* 中文逐项-宏 M0094：verbs_set_ctx_op
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 verbs_set_ctx_op；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define verbs_set_ctx_op(_vctx, op, ptr) ({ \
	struct verbs_context *vctx = _vctx; \
	if (vctx && (vctx->sz >= sizeof(*vctx) - offsetof(struct verbs_context, op))) \
		vctx->op = ptr; })

/* 中文逐项-函数 F0655：verbs_get_device
 * 用途与位置：从上下文得到 verbs 扩展设备对象，支持按 ABI 能力寻找正确的操作函数。
 * 参数 dev（struct ibv_device *dev）：当前设备在相关库/拓扑表内的下标，NVML编号、CUDA ordinal和NET编号不可直接互换。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：container_of。
 * 源码返回表达式：return (dev->ops.alloc_context) ? NULL : container_of(dev, struct verbs_device, device);。CHECK类宏还可能提前返回/跳转。
 */
static inline struct verbs_device *verbs_get_device(struct ibv_device *dev)
{
	return (dev->ops.alloc_context) ?
		NULL : container_of(dev, struct verbs_device, device);
}

typedef /* 中文逐项-类型 T0299：ibv_return_enum
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum ibv_return_enum
{
/* 中文逐项-枚举项 X0576：IBV_SUCCESS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    IBV_SUCCESS = 0,                   //!< The operation was successful
} ibv_return_t;

/* 中文逐项-接口 X0577：wrap_ibv_symbols
 * 接口职责：动态装载并解析 libibverbs 符号表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_symbols(void);
 */
ncclResult_t wrap_ibv_symbols(void);
/* 中文逐项-接口 X0578：wrap_ibv_fork_init
 * 接口职责：初始化 verbs 注册页的 fork 处理；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_fork_init(void);
 */
ncclResult_t wrap_ibv_fork_init(void);
/* 中文逐项-接口 X0579：wrap_ibv_get_device_list
 * 接口职责：获取可用 RDMA 设备列表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_get_device_list(struct ibv_device ***ret, int *num_devices);
 */
ncclResult_t wrap_ibv_get_device_list(struct ibv_device ***ret, int *num_devices);
/* 中文逐项-接口 X0580：wrap_ibv_free_device_list
 * 接口职责：归还设备枚举列表；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_free_device_list(struct ibv_device **list);
 */
ncclResult_t wrap_ibv_free_device_list(struct ibv_device **list);
/* 中文逐项-变量 V2667：wrap_ibv_get_device_name
 * wrap_ibv_get_device_name（char；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
const char *wrap_ibv_get_device_name(struct ibv_device *device);
/* 中文逐项-接口 X0581：wrap_ibv_open_device
 * 接口职责：打开设备得到 verbs context；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_open_device(struct ibv_context **ret, struct ibv_device
 * *device);
 */
ncclResult_t wrap_ibv_open_device(struct ibv_context **ret, struct ibv_device *device);
/* 中文逐项-接口 X0582：wrap_ibv_close_device
 * 接口职责：关闭设备上下文；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_close_device(struct ibv_context *context);
 */
ncclResult_t wrap_ibv_close_device(struct ibv_context *context);
/* 中文逐项-接口 X0583：wrap_ibv_get_async_event
 * 接口职责：等待读取设备异步事件；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_get_async_event(struct ibv_context *context, struct
 * ibv_async_event *event);
 */
ncclResult_t wrap_ibv_get_async_event(struct ibv_context *context, struct ibv_async_event *event);
/* 中文逐项-接口 X0584：wrap_ibv_ack_async_event
 * 接口职责：确认并释放已处理异步事件；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_ack_async_event(struct ibv_async_event *event);
 */
ncclResult_t wrap_ibv_ack_async_event(struct ibv_async_event *event);
/* 中文逐项-接口 X0585：wrap_ibv_query_device
 * 接口职责：读取设备能力限制；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_query_device(struct ibv_context *context, struct
 * ibv_device_attr *device_attr);
 */
ncclResult_t wrap_ibv_query_device(struct ibv_context *context, struct ibv_device_attr *device_attr);
/* 中文逐项-接口 X0586：wrap_ibv_query_port
 * 接口职责：读取端口状态/地址与速率能力；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_query_port(struct ibv_context *context, uint8_t port_num,
 * struct ibv_port_attr *port_attr);
 */
ncclResult_t wrap_ibv_query_port(struct ibv_context *context, uint8_t port_num, struct ibv_port_attr *port_attr);
/* 中文逐项-接口 X0587：wrap_ibv_query_gid
 * 接口职责：读取指定端口/GID索引的地址；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_query_gid(struct ibv_context *context, uint8_t port_num, int
 * index, union ibv_gid *gid);
 */
ncclResult_t wrap_ibv_query_gid(struct ibv_context *context, uint8_t port_num, int index, union ibv_gid *gid);
/* 中文逐项-接口 X0588：wrap_ibv_query_qp
 * 接口职责：查询 QP 当前状态和属性；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_query_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int
 * attr_mask, struct ibv_qp_init_attr *init_attr);
 */
ncclResult_t wrap_ibv_query_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask, struct ibv_qp_init_attr *init_attr);
/* 中文逐项-接口 X0589：wrap_ibv_alloc_pd
 * 接口职责：分配保护域，隔离允许访问的 QP/MR 集合；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_alloc_pd(struct ibv_pd **ret, struct ibv_context *context);
 */
ncclResult_t wrap_ibv_alloc_pd(struct ibv_pd **ret, struct ibv_context *context);
/* 中文逐项-接口 X0590：wrap_ibv_dealloc_pd
 * 接口职责：释放保护域；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_dealloc_pd(struct ibv_pd *pd);
 */
ncclResult_t wrap_ibv_dealloc_pd(struct ibv_pd *pd);
/* 中文逐项-接口 X0591：wrap_ibv_reg_mr
 * 接口职责：把指定虚拟地址范围注册到 PD 并取得 lkey/rkey；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_reg_mr(struct ibv_mr **ret, struct ibv_pd *pd, void *addr,
 * size_t length, int access);
 */
ncclResult_t wrap_ibv_reg_mr(struct ibv_mr **ret, struct ibv_pd *pd, void *addr, size_t length, int access);
/* 中文逐项-变量 V2668：wrap_direct_ibv_reg_mr
 * wrap_direct_ibv_reg_mr（struct ibv_mr；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
struct ibv_mr * wrap_direct_ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int access);
/* 中文逐项-接口 X0592：wrap_ibv_reg_mr_iova2
 * 接口职责：注册内存并指定 HCA 使用的 IOVA 地址及访问权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_reg_mr_iova2(struct ibv_mr **ret, struct ibv_pd *pd, void
 * *addr, size_t length, uint64_t iova, int access);
 */
ncclResult_t wrap_ibv_reg_mr_iova2(struct ibv_mr **ret, struct ibv_pd *pd, void *addr, size_t length, uint64_t iova, int access);
/* 中文逐项-接口 X0593：wrap_ibv_dereg_mr
 * 接口职责：注销一个真实 MR；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_dereg_mr(struct ibv_mr *mr);
 */
ncclResult_t wrap_ibv_dereg_mr(struct ibv_mr *mr);
ncclResult_t wrap_ibv_create_comp_channel(struct ibv_comp_channel **ret, struct ibv_context *context);
ncclResult_t wrap_ibv_destroy_comp_channel(struct ibv_comp_channel *channel);
/* 中文逐项-接口 X0594：wrap_ibv_create_cq
 * 接口职责：创建完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_create_cq(struct ibv_cq **ret, struct ibv_context *context,
 * int cqe, void *cq_context, struct ibv_comp_channel *channel, int comp_vector);
 */
ncclResult_t wrap_ibv_create_cq(struct ibv_cq **ret, struct ibv_context *context, int cqe, void *cq_context, struct ibv_comp_channel *channel, int comp_vector);
/* 中文逐项-接口 X0595：wrap_ibv_destroy_cq
 * 接口职责：销毁完成队列；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_destroy_cq(struct ibv_cq *cq);
 */
ncclResult_t wrap_ibv_destroy_cq(struct ibv_cq *cq);
/* 中文逐项-函数 F0656：wrap_ibv_poll_cq
 * 用途与位置：从 CQ 轮询完成项并统一错误处理，返回数量可为零而不代表失败。
 * 参数 cq（struct ibv_cq *cq）：完成队列或 QP 绑定的发送/接收 CQ，保存硬件工作完成项。
 * 参数 num_entries（int num_entries）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 参数 wc（struct ibv_wc *wc）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 num_done（int* num_done）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbTest。
 * 本体直接出现的调用/包装：cq->context->ops.poll_cq。
 * 需要对照的边界/推进条件：done < 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static inline ncclResult_t wrap_ibv_poll_cq(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc, int* num_done) {
/* 中文逐项-变量 V2669：done
 * done（int；函数 wrap_ibv_poll_cq 内）：本次网络Test/状态检查的局部完成标志，检查为真后才推进done计数或回收请求。
 * 初值/来源 done = cq->context->ops.poll_cq(cq, num_entries, wc)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int done = cq->context->ops.poll_cq(cq, num_entries, wc); /*returns the number of wcs or 0 on success, a negative number otherwise*/
  if (done < 0) {
    WARN("Call to ibv_poll_cq() returned %d", done);
    return ncclSystemError;
  }
  *num_done = done;
  return ncclSuccess;
}
/* 中文逐项-接口 X0596：wrap_ibv_create_qp
 * 接口职责：创建指定类型和队列容量的 QP；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_create_qp(struct ibv_qp **ret, struct ibv_pd *pd, struct
 * ibv_qp_init_attr *qp_init_attr);
 */
ncclResult_t wrap_ibv_create_qp(struct ibv_qp **ret, struct ibv_pd *pd, struct ibv_qp_init_attr *qp_init_attr);
/* 中文逐项-接口 X0597：wrap_ibv_modify_qp
 * 接口职责：按属性掩码修改 QP 状态/路由/权限；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_modify_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int
 * attr_mask);
 */
ncclResult_t wrap_ibv_modify_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int attr_mask);
/* 中文逐项-接口 X0598：wrap_ibv_destroy_qp
 * 接口职责：销毁队列对；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_destroy_qp(struct ibv_qp *qp);
 */
ncclResult_t wrap_ibv_destroy_qp(struct ibv_qp *qp);
/* 中文逐项-函数 F0657：ibv_post_send
 * 用途与位置：通过 QP 的 provider 操作表提交发送工作请求链，立即返回仅表示提交，不代表载荷完成。
 * 参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 参数 wr（struct ibv_send_wr *wr）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 参数 bad_wr（struct ibv_send_wr **bad_wr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：qp->context->ops.post_send。
 * 源码返回表达式：return qp->context->ops.post_send(qp, wr, bad_wr);。CHECK类宏还可能提前返回/跳转。
 */
static inline int ibv_post_send(struct ibv_qp *qp, struct ibv_send_wr *wr, struct ibv_send_wr **bad_wr) {
  return qp->context->ops.post_send(qp, wr, bad_wr);
}

/* 中文逐项-函数 F0658：wrap_ibv_post_send
 * 用途与位置：通过 QP 的 provider 操作表提交发送工作请求链，立即返回仅表示提交，不代表载荷完成。
 * 参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 参数 wr（struct ibv_send_wr *wr）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 参数 bad_wr（struct ibv_send_wr **bad_wr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbMultiSend；src/transport/net_ib.cc::ncclIbPostFifo；src/transport/net_ib.cc::ncclIbIflush。
 * 本体直接出现的调用/包装：qp->context->ops.post_send；strerror。
 * 需要对照的边界/推进条件：ret != IBV_SUCCESS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static inline ncclResult_t wrap_ibv_post_send(struct ibv_qp *qp, struct ibv_send_wr *wr, struct ibv_send_wr **bad_wr) {
/* 中文逐项-变量 V2670：ret
 * ret（int；函数 wrap_ibv_post_send 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = qp->context->ops.post_send(qp, wr, bad_wr)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ret = qp->context->ops.post_send(qp, wr, bad_wr); /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  if (ret != IBV_SUCCESS) {
    WARN("ibv_post_send() failed with error %s, Bad WR %p, First WR %p", strerror(ret), wr, *bad_wr);
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-函数 F0659：wrap_ibv_post_recv
 * 用途与位置：向指定 QP 的接收队列投递 WQE，用于 IB 接收通知；该接口不自动变成 SRQ。
 * 参数 qp（struct ibv_qp *qp）：单个或多个 verbs 队列对，承载实际数据/控制请求，区别于 NCCL 逻辑channel。
 * 参数 wr（struct ibv_recv_wr *wr）：工作请求对象/用于完成解码的用户标识/请求链数组；wr_id不是载荷数据。
 * 参数 bad_wr（struct ibv_recv_wr **bad_wr）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/transport/net_ib.cc::ncclIbIrecv。
 * 本体直接出现的调用/包装：qp->context->ops.post_recv；strerror。
 * 需要对照的边界/推进条件：ret != IBV_SUCCESS。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSystemError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static inline ncclResult_t wrap_ibv_post_recv(struct ibv_qp *qp, struct ibv_recv_wr *wr, struct ibv_recv_wr **bad_wr) {
/* 中文逐项-变量 V2671：ret
 * ret（int；函数 wrap_ibv_post_recv 内）：调用/线程的返回值或错误码；须依据其类型区分NCCL、CUDA、errno或系统状态。
 * 初值/来源 ret = qp->context->ops.post_recv(qp, wr, bad_wr)；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  int ret = qp->context->ops.post_recv(qp, wr, bad_wr); /*returns 0 on success, or the value of errno on failure (which indicates the failure reason)*/
  if (ret != IBV_SUCCESS) {
    WARN("ibv_post_recv() failed with error %s", strerror(ret));
    return ncclSystemError;
  }
  return ncclSuccess;
}

/* 中文逐项-接口 X0599：wrap_ibv_event_type_str
 * 接口职责：把设备事件枚举转换成诊断字符串；包装层统一动态符号检查与 verbs 错误返回，硬件操作由 provider 执行。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t wrap_ibv_event_type_str(char **ret, enum ibv_event_type event);
 */
ncclResult_t wrap_ibv_event_type_str(char **ret, enum ibv_event_type event);

#endif //End include guard
