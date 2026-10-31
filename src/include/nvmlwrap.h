/*************************************************************************
 * Copyright (c) 2015-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_NVMLWRAP_H_
/* 中文逐项-常量 X0638：NCCL_NVMLWRAP_H_
 * 编译期定义：NCCL_NVMLWRAP_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NVMLWRAP_H_

#include "nccl.h"

//#define NCCL_NVML_DIRECT 1
#ifndef NCCL_NVML_DIRECT
/* 中文逐项-常量 X0639：NCCL_NVML_DIRECT
 * 编译期定义：NCCL_NVML_DIRECT = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_NVML_DIRECT 0
#endif

#if NCCL_NVML_DIRECT
#include "nvml.h"
#else
// Dynamically handle dependencies on NVML

/* Extracted from nvml.h */
typedef struct nvmlDevice_st* nvmlDevice_t;
/* 中文逐项-常量 X0640：NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE
 * 编译期定义：NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE   16

typedef /* 中文逐项-类型 T0310：nvmlEnableState_enum
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvmlEnableState_enum
{
/* 中文逐项-枚举项 X0641：NVML_FEATURE_DISABLED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVML_FEATURE_DISABLED    = 0,     //!< Feature disabled
/* 中文逐项-枚举项 X0642：NVML_FEATURE_ENABLED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVML_FEATURE_ENABLED     = 1      //!< Feature enabled
} nvmlEnableState_t;

typedef /* 中文逐项-类型 T0311：nvmlNvLinkCapability_enum
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvmlNvLinkCapability_enum
{
/* 中文逐项-枚举项 X0643：NVML_NVLINK_CAP_P2P_SUPPORTED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVML_NVLINK_CAP_P2P_SUPPORTED = 0,     // P2P over NVLink is supported
/* 中文逐项-枚举项 X0644：NVML_NVLINK_CAP_SYSMEM_ACCESS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVML_NVLINK_CAP_SYSMEM_ACCESS = 1,     // Access to system memory is supported
/* 中文逐项-枚举项 X0645：NVML_NVLINK_CAP_P2P_ATOMICS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVML_NVLINK_CAP_P2P_ATOMICS   = 2,     // P2P atomics are supported
/* 中文逐项-枚举项 X0646：NVML_NVLINK_CAP_SYSMEM_ATOMICS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVML_NVLINK_CAP_SYSMEM_ATOMICS= 3,     // System memory atomics are supported
/* 中文逐项-枚举项 X0647：NVML_NVLINK_CAP_SLI_BRIDGE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVML_NVLINK_CAP_SLI_BRIDGE    = 4,     // SLI is supported over this link
/* 中文逐项-枚举项 X0648：NVML_NVLINK_CAP_VALID
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVML_NVLINK_CAP_VALID         = 5,     // Link is supported on this device
    // should be last
/* 中文逐项-枚举项 X0649：NVML_NVLINK_CAP_COUNT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_NVLINK_CAP_COUNT
} nvmlNvLinkCapability_t;

typedef /* 中文逐项-类型 T0312：nvmlReturn_enum
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvmlReturn_enum
{
/* 中文逐项-枚举项 X0650：NVML_SUCCESS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVML_SUCCESS = 0,                   //!< The operation was successful
/* 中文逐项-枚举项 X0651：NVML_ERROR_UNINITIALIZED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
    NVML_ERROR_UNINITIALIZED = 1,       //!< NVML was not first initialized with nvmlInit()
/* 中文逐项-枚举项 X0652：NVML_ERROR_INVALID_ARGUMENT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
    NVML_ERROR_INVALID_ARGUMENT = 2,    //!< A supplied argument is invalid
/* 中文逐项-枚举项 X0653：NVML_ERROR_NOT_SUPPORTED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
    NVML_ERROR_NOT_SUPPORTED = 3,       //!< The requested operation is not available on target device
/* 中文逐项-枚举项 X0654：NVML_ERROR_NO_PERMISSION
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
    NVML_ERROR_NO_PERMISSION = 4,       //!< The current user does not have permission for operation
/* 中文逐项-枚举项 X0655：NVML_ERROR_ALREADY_INITIALIZED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：5
 */
    NVML_ERROR_ALREADY_INITIALIZED = 5, //!< Deprecated: Multiple initializations are now allowed through ref counting
/* 中文逐项-枚举项 X0656：NVML_ERROR_NOT_FOUND
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：6
 */
    NVML_ERROR_NOT_FOUND = 6,           //!< A query to find an object was unsuccessful
/* 中文逐项-枚举项 X0657：NVML_ERROR_INSUFFICIENT_SIZE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：7
 */
    NVML_ERROR_INSUFFICIENT_SIZE = 7,   //!< An input argument is not large enough
/* 中文逐项-枚举项 X0658：NVML_ERROR_INSUFFICIENT_POWER
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
    NVML_ERROR_INSUFFICIENT_POWER = 8,  //!< A device's external power cables are not properly attached
/* 中文逐项-枚举项 X0659：NVML_ERROR_DRIVER_NOT_LOADED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：9
 */
    NVML_ERROR_DRIVER_NOT_LOADED = 9,   //!< NVIDIA driver is not loaded
/* 中文逐项-枚举项 X0660：NVML_ERROR_TIMEOUT
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：10
 */
    NVML_ERROR_TIMEOUT = 10,            //!< User provided timeout passed
/* 中文逐项-枚举项 X0661：NVML_ERROR_IRQ_ISSUE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：11
 */
    NVML_ERROR_IRQ_ISSUE = 11,          //!< NVIDIA Kernel detected an interrupt issue with a GPU
/* 中文逐项-枚举项 X0662：NVML_ERROR_LIBRARY_NOT_FOUND
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：12
 */
    NVML_ERROR_LIBRARY_NOT_FOUND = 12,  //!< NVML Shared Library couldn't be found or loaded
/* 中文逐项-枚举项 X0663：NVML_ERROR_FUNCTION_NOT_FOUND
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：13
 */
    NVML_ERROR_FUNCTION_NOT_FOUND = 13, //!< Local version of NVML doesn't implement this function
/* 中文逐项-枚举项 X0664：NVML_ERROR_CORRUPTED_INFOROM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：14
 */
    NVML_ERROR_CORRUPTED_INFOROM = 14,  //!< infoROM is corrupted
/* 中文逐项-枚举项 X0665：NVML_ERROR_GPU_IS_LOST
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：15
 */
    NVML_ERROR_GPU_IS_LOST = 15,        //!< The GPU has fallen off the bus or has otherwise become inaccessible
/* 中文逐项-枚举项 X0666：NVML_ERROR_RESET_REQUIRED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：16
 */
    NVML_ERROR_RESET_REQUIRED = 16,     //!< The GPU requires a reset before it can be used again
/* 中文逐项-枚举项 X0667：NVML_ERROR_OPERATING_SYSTEM
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：17
 */
    NVML_ERROR_OPERATING_SYSTEM = 17,   //!< The GPU control device has been blocked by the operating system/cgroups
/* 中文逐项-枚举项 X0668：NVML_ERROR_LIB_RM_VERSION_MISMATCH
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：18
 */
    NVML_ERROR_LIB_RM_VERSION_MISMATCH = 18,   //!< RM detects a driver/library version mismatch
/* 中文逐项-枚举项 X0669：NVML_ERROR_IN_USE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：19
 */
    NVML_ERROR_IN_USE = 19,             //!< An operation cannot be performed because the GPU is currently in use
/* 中文逐项-枚举项 X0670：NVML_ERROR_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：999
 */
    NVML_ERROR_UNKNOWN = 999            //!< An internal driver error occurred
} nvmlReturn_t;

typedef /* 中文逐项-类型 T0313：nvmlPciInfo_st
 * 类型职责：动态管理/映射库所需的ABI类型，保存该接口返回的属性/句柄；先看外层调用确定设备身份和生命周期。
 * 声明内可见成员（含嵌套成员）：busId、domain、bus、device、pciDeviceId、pciSubSystemId、reserved0、reserved1、reserved2、reserved3。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */struct nvmlPciInfo_st
{
/* 中文逐项-变量 V2774：busId
 * busId（char；成员 nvmlPciInfo_st）：PCI BDF 的整数标识，用于将 CUDA、NVML 与拓扑中的同一设备关联。
 */
    char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE]; //!< The tuple domain:bus:device.function PCI identifier (&amp; NULL terminator)
/* 中文逐项-变量 V2775：domain
 * domain（unsigned int；成员 nvmlPciInfo_st）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 */
    unsigned int domain;             //!< The PCI domain on which the device's bus resides, 0 to 0xffff
/* 中文逐项-变量 V2776：bus
 * bus（unsigned int；成员 nvmlPciInfo_st）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 */
    unsigned int bus;                //!< The bus on which the device resides, 0 to 0xff
/* 中文逐项-变量 V2777：device
 * device（unsigned int；成员 nvmlPciInfo_st）：当前设备对象/PCI设备号/网络设备编号；其声明类型与宿主结构决定编号空间。
 */
    unsigned int device;             //!< The device's id on the bus, 0 to 31
/* 中文逐项-变量 V2778：pciDeviceId
 * pciDeviceId（unsigned int；成员 nvmlPciInfo_st）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 */
    unsigned int pciDeviceId;        //!< The combined 16-bit device id and 16-bit vendor id

    // Added in NVML 2.285 API
/* 中文逐项-变量 V2779：pciSubSystemId
 * pciSubSystemId（unsigned int；成员 nvmlPciInfo_st）：NVML报告的PCI总线/域及设备/子系统标识，结合busId匹配CUDA设备。
 */
    unsigned int pciSubSystemId;     //!< The 32-bit Sub System Device ID

    // NVIDIA reserved for internal use only
/* 中文逐项-变量 V2780：reserved0
 * reserved0（unsigned int；成员 nvmlPciInfo_st）：ABI/硬件位域保留空间，保留大小和对齐，不能擅自解释为有效配置。
 */
    unsigned int reserved0;
/* 中文逐项-变量 V2781：reserved1
 * reserved1（unsigned int；成员 nvmlPciInfo_st）：ABI/硬件位域保留空间，保留大小和对齐，不能擅自解释为有效配置。
 */
    unsigned int reserved1;
/* 中文逐项-变量 V2782：reserved2
 * reserved2（unsigned int；成员 nvmlPciInfo_st）：ABI/硬件位域保留空间，保留大小和对齐，不能擅自解释为有效配置。
 */
    unsigned int reserved2;
/* 中文逐项-变量 V2783：reserved3
 * reserved3（unsigned int；成员 nvmlPciInfo_st）：ABI/硬件位域保留空间，保留大小和对齐，不能擅自解释为有效配置。
 */
    unsigned int reserved3;
} nvmlPciInfo_t;

/* P2P Capability Index Status*/
typedef /* 中文逐项-类型 T0314：nvmlGpuP2PStatus_enum
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvmlGpuP2PStatus_enum
{
/* 中文逐项-枚举项 X0671：NVML_P2P_STATUS_OK
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVML_P2P_STATUS_OK     = 0,
/* 中文逐项-枚举项 X0672：NVML_P2P_STATUS_CHIPSET_NOT_SUPPORED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_STATUS_CHIPSET_NOT_SUPPORED,
/* 中文逐项-枚举项 X0673：NVML_P2P_STATUS_GPU_NOT_SUPPORTED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_STATUS_GPU_NOT_SUPPORTED,
/* 中文逐项-枚举项 X0674：NVML_P2P_STATUS_IOH_TOPOLOGY_NOT_SUPPORTED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_STATUS_IOH_TOPOLOGY_NOT_SUPPORTED,
/* 中文逐项-枚举项 X0675：NVML_P2P_STATUS_DISABLED_BY_REGKEY
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_STATUS_DISABLED_BY_REGKEY,
/* 中文逐项-枚举项 X0676：NVML_P2P_STATUS_NOT_SUPPORTED
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_STATUS_NOT_SUPPORTED,
/* 中文逐项-枚举项 X0677：NVML_P2P_STATUS_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_STATUS_UNKNOWN
} nvmlGpuP2PStatus_t;

/* P2P Capability Index*/
typedef /* 中文逐项-类型 T0315：nvmlGpuP2PCapsIndex_enum
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */enum nvmlGpuP2PCapsIndex_enum
{
/* 中文逐项-枚举项 X0678：NVML_P2P_CAPS_INDEX_READ
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
    NVML_P2P_CAPS_INDEX_READ = 0,
/* 中文逐项-枚举项 X0679：NVML_P2P_CAPS_INDEX_WRITE
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_CAPS_INDEX_WRITE,
/* 中文逐项-枚举项 X0680：NVML_P2P_CAPS_INDEX_NVLINK
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_CAPS_INDEX_NVLINK,
/* 中文逐项-枚举项 X0681：NVML_P2P_CAPS_INDEX_ATOMICS
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_CAPS_INDEX_ATOMICS,
/* 中文逐项-枚举项 X0682：NVML_P2P_CAPS_INDEX_PROP
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_CAPS_INDEX_PROP,
/* 中文逐项-枚举项 X0683：NVML_P2P_CAPS_INDEX_UNKNOWN
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 未写显式值，按前一枚举项加1计算；首项默认0。位掩码类请对照同组显式移位值。
 */
    NVML_P2P_CAPS_INDEX_UNKNOWN
} nvmlGpuP2PCapsIndex_t;

/* End of nvml.h */
#endif // NCCL_NVML_DIRECT

/* 中文逐项-变量 V2784：ncclNvmlMaxDevices
 * ncclNvmlMaxDevices（int；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 ncclNvmlMaxDevices = 32；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
constexpr int ncclNvmlMaxDevices = 32;
/* 中文逐项-类型 T0316：ncclNvmlDeviceInfo
 * 类型职责：NVML发现的设备句柄/能力和GPU对P2P状态缓存，供拓扑策略使用。
 * 声明内可见成员（含嵌套成员）：handle、computeCapabilityMajor、computeCapabilityMinor。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclNvmlDeviceInfo {
/* 中文逐项-变量 V2785：handle
 * handle（nvmlDevice_t；成员 ncclNvmlDeviceInfo）：库/内核资源句柄或可交换连接句柄，不能当作通用可直接解引用指针。
 */
  nvmlDevice_t handle;
/* 中文逐项-变量 V2786：computeCapabilityMajor、computeCapabilityMinor
 * computeCapabilityMajor（int；成员 ncclNvmlDeviceInfo）：NVML缓存的GPU计算能力主/次版本，用于模型与协议策略。
 * computeCapabilityMinor（int；成员 ncclNvmlDeviceInfo）：NVML缓存的GPU计算能力主/次版本，用于模型与协议策略。
 */
  int computeCapabilityMajor, computeCapabilityMinor;
};
/* 中文逐项-类型 T0317：ncclNvmlDevicePairInfo
 * 类型职责：NVML发现的设备句柄/能力和GPU对P2P状态缓存，供拓扑策略使用。
 * 声明内可见成员（含嵌套成员）：p2pStatusRead、p2pStatusWrite。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclNvmlDevicePairInfo {
/* 中文逐项-变量 V2787：p2pStatusRead、p2pStatusWrite
 * p2pStatusRead（nvmlGpuP2PStatus_t；成员 ncclNvmlDevicePairInfo）：GPU设备对在读/写方向的P2P能力状态，两方向可能分别检查。
 * p2pStatusWrite（nvmlGpuP2PStatus_t；成员 ncclNvmlDevicePairInfo）：GPU设备对在读/写方向的P2P能力状态，两方向可能分别检查。
 */
  nvmlGpuP2PStatus_t p2pStatusRead, p2pStatusWrite;
};
/* 中文逐项-变量 V2788：ncclNvmlDeviceCount
 * ncclNvmlDeviceCount（int；文件/静态存储作用域）：当前对象累计或有效条目计数，递增/复位位置决定统计时间范围。
 */
extern int ncclNvmlDeviceCount;
/* 中文逐项-变量 V2789：ncclNvmlDevices
 * ncclNvmlDevices（ncclNvmlDeviceInfo；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern ncclNvmlDeviceInfo ncclNvmlDevices[ncclNvmlMaxDevices];
/* 中文逐项-变量 V2790：ncclNvmlDevicePairs
 * ncclNvmlDevicePairs（ncclNvmlDevicePairInfo；文件/静态存储作用域）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 */
extern ncclNvmlDevicePairInfo ncclNvmlDevicePairs[ncclNvmlMaxDevices][ncclNvmlMaxDevices];

// All ncclNvmlFoo() functions call ncclNvmlEnsureInitialized() implicitly.
// Outsiders need only call it if they want to inspect the ncclNvml global
// tables above.
/* 中文逐项-接口 X0684：ncclNvmlEnsureInitialized
 * 接口职责：线程安全初始化动态 NVML 接口、设备句柄和支持能力缓存，供硬件发现复用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlEnsureInitialized();
 */
ncclResult_t ncclNvmlEnsureInitialized();

/* 中文逐项-接口 X0685：ncclNvmlDeviceGetHandleByPciBusId
 * 接口职责：按 PCI 地址定位 NVML 设备；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlDeviceGetHandleByPciBusId(const char* pciBusId, nvmlDevice_t*
 * device);
 */
ncclResult_t ncclNvmlDeviceGetHandleByPciBusId(const char* pciBusId, nvmlDevice_t* device);
/* 中文逐项-接口 X0686：ncclNvmlDeviceGetIndex
 * 接口职责：由句柄查询 NVML 序号；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlDeviceGetIndex(nvmlDevice_t device, unsigned* index);
 */
ncclResult_t ncclNvmlDeviceGetIndex(nvmlDevice_t device, unsigned* index);
/* 中文逐项-接口 X0687：ncclNvmlDeviceGetHandleByIndex
 * 接口职责：按 NVML 序号取得设备句柄；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlDeviceGetHandleByIndex(unsigned int index, nvmlDevice_t
 * *device);
 */
ncclResult_t ncclNvmlDeviceGetHandleByIndex(unsigned int index, nvmlDevice_t *device);
/* 中文逐项-接口 X0688：ncclNvmlDeviceGetNvLinkState
 * 接口职责：查询指定 NVLink 是否激活；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlDeviceGetNvLinkState(nvmlDevice_t device, unsigned int link,
 * nvmlEnableState_t *isActive);
 */
ncclResult_t ncclNvmlDeviceGetNvLinkState(nvmlDevice_t device, unsigned int link, nvmlEnableState_t *isActive);
/* 中文逐项-接口 X0689：ncclNvmlDeviceGetNvLinkRemotePciInfo
 * 接口职责：查询 NVLink 远端 PCI 标识；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlDeviceGetNvLinkRemotePciInfo(nvmlDevice_t device, unsigned int
 * link, nvmlPciInfo_t *pci);
 */
ncclResult_t ncclNvmlDeviceGetNvLinkRemotePciInfo(nvmlDevice_t device, unsigned int link, nvmlPciInfo_t *pci);
/* 中文逐项-接口 X0690：ncclNvmlDeviceGetNvLinkCapability
 * 接口职责：查询 NVLink 能力；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlDeviceGetNvLinkCapability(nvmlDevice_t device, unsigned int
 * link, nvmlNvLinkCapability_t capability, unsigned int *capResult);
 */
ncclResult_t ncclNvmlDeviceGetNvLinkCapability(nvmlDevice_t device, unsigned int link, nvmlNvLinkCapability_t capability, unsigned int *capResult);
/* 中文逐项-接口 X0691：ncclNvmlDeviceGetCudaComputeCapability
 * 接口职责：查询 GPU 计算能力；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlDeviceGetCudaComputeCapability(nvmlDevice_t device, int*
 * major, int* minor);
 */
ncclResult_t ncclNvmlDeviceGetCudaComputeCapability(nvmlDevice_t device, int* major, int* minor);
/* 中文逐项-接口 X0692：ncclNvmlDeviceGetP2PStatus
 * 接口职责：查询 GPU 对之间的特定 P2P 能力状态；先保证管理库初始化，再在锁保护下调用动态符号，结果供拓扑发现使用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclNvmlDeviceGetP2PStatus(nvmlDevice_t device1, nvmlDevice_t device2,
 * nvmlGpuP2PCapsIndex_t p2pIndex, nvmlGpuP2PStatus_t* p2pStatus);
 */
ncclResult_t ncclNvmlDeviceGetP2PStatus(nvmlDevice_t device1, nvmlDevice_t device2, nvmlGpuP2PCapsIndex_t p2pIndex, nvmlGpuP2PStatus_t* p2pStatus);
#endif // End include guard
