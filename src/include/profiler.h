/*************************************************************************
 * Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef NCCL_PROFILER_H_
/* 中文逐项-常量 X1048：NCCL_PROFILER_H_
 * 编译期定义：NCCL_PROFILER_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_PROFILER_H_

#include "proxy.h"

/* 中文逐项-类型 T0361：ncclProxyProfileState
 * 类型职责：本作用域的状态/类别/能力枚举。命名常量固定表示法，下面逐项标明值；按使用字段判断是互斥状态还是可组合位掩码。
 * 取值规则：显式赋值以右侧常量/位表达式为准，未显式赋值的项按C/C++枚举顺序递增；使用是否按位组合取决于消费者。
 */
enum ncclProxyProfileState {
/* 中文逐项-枚举项 X1049：ncclProxyProfileBegin
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：0
 */
  ncclProxyProfileBegin = 0,

/* 中文逐项-枚举项 X1050：ncclProxyProfileSendGPUWait
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
  ncclProxyProfileSendGPUWait = 1,
/* 中文逐项-枚举项 X1051：ncclProxyProfileSendWait
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
  ncclProxyProfileSendWait = 2,

/* 中文逐项-枚举项 X1052：ncclProxyProfileRecvWait
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：1
 */
  ncclProxyProfileRecvWait = 1,
/* 中文逐项-枚举项 X1053：ncclProxyProfileRecvFlushWait
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：2
 */
  ncclProxyProfileRecvFlushWait = 2,
/* 中文逐项-枚举项 X1054：ncclProxyProfileRecvGPUWait
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：3
 */
  ncclProxyProfileRecvGPUWait = 3,

/* 中文逐项-枚举项 X1055：ncclProxyProfileEnd
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：4
 */
  ncclProxyProfileEnd = 4,

/* 中文逐项-枚举项 X1056：ncclProxyProfileSleep
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：8
 */
  ncclProxyProfileSleep = 8,
/* 中文逐项-枚举项 X1057：ncclProxyProfileWakeup
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：9
 */
  ncclProxyProfileWakeup = 9,

/* 中文逐项-枚举项 X1058：ncclProxyProfileIdle
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：16
 */
  ncclProxyProfileIdle = 16,
/* 中文逐项-枚举项 X1059：ncclProxyProfileActive
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：17
 */
  ncclProxyProfileActive = 17,

/* 中文逐项-枚举项 X1060：ncclProxyProfileAppend
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：24
 */
  ncclProxyProfileAppend = 24,
/* 中文逐项-枚举项 X1061：ncclProxyProfileAppendEnd
 * 所属枚举中的命名取值；名称表达对应状态/能力/类别，使用字段和下方值决定协议编码。
 * 显式值：25
 */
  ncclProxyProfileAppendEnd = 25
};

/* 中文逐项-接口 X1062：ncclProfilingRecord
 * 接口职责：若编译启用 profiling，记录 Proxy 子步骤在当前阶段的时间点；禁用分支为空实现。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclProfilingRecord(struct ncclProxyArgs* args, int sub, int step, int
 * state);
 */
ncclResult_t ncclProfilingRecord(struct ncclProxyArgs* args, int sub, int step, int state);
/* 中文逐项-接口 X1063：ncclProfilingDump
 * 接口职责：导出代理阶段耗时记录，帮助区分 GPU 等待、网络等待和 flush；禁用分支无操作。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：void ncclProfilingDump();
 */
void ncclProfilingDump();

#endif
