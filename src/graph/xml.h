/*************************************************************************
 * Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#ifndef XML_H_
/* 中文逐项-常量 X0100：XML_H_
 * 编译期定义：XML_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define XML_H_

#include "nccl.h"
#include "debug.h"
#include "checks.h"
#include <stdlib.h>

// A few constraints to make the implementation easy
/* 中文逐项-常量 X0101：MAX_STR_LEN
 * 编译期定义：MAX_STR_LEN = 255。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_STR_LEN 255
/* 中文逐项-常量 X0102：MAX_ATTR_COUNT
 * 编译期定义：MAX_ATTR_COUNT = 16。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_ATTR_COUNT 16
/* 中文逐项-常量 X0103：MAX_SUBS
 * 编译期定义：MAX_SUBS = 32。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_SUBS 32
/* 中文逐项-常量 X0104：MAX_NODES
 * 编译期定义：MAX_NODES = 1024。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define MAX_NODES 1024

/* 中文逐项-常量 X0105：NODE_TYPE_NONE
 * 编译期定义：NODE_TYPE_NONE = 0。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NODE_TYPE_NONE 0
/* 中文逐项-常量 X0106：NODE_TYPE_OPEN
 * 编译期定义：NODE_TYPE_OPEN = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NODE_TYPE_OPEN 1
/* 中文逐项-常量 X0107：NODE_TYPE_CLOSE
 * 编译期定义：NODE_TYPE_CLOSE = 2。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NODE_TYPE_CLOSE 2
/* 中文逐项-常量 X0108：NODE_TYPE_SINGLE
 * 编译期定义：NODE_TYPE_SINGLE = 3。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NODE_TYPE_SINGLE 3

/* 中文精读：硬件 XML 与通信图 XML 的区别
 * 硬件拓扑描述 CPU/PCI/GPU/NIC 等节点及属性，之后转成 ncclTopoSystem。
 * 通信图 XML 描述搜索所得 channel 的 GPU 顺序和 NIC 端点，用于导入/导出图搜索结果。
 * 本结构用固定节点池、属性数组和父子指针表达 XML，避免引入通用解析器依赖。
 * XML 节点不是 GPU 上的 work 或用户张量；修改拓扑输入也不会改变真实硬件连线。
 */
/* 中文逐项-类型 T0153：ncclXmlNode
 * 类型职责：轻量XML的节点、属性和固定节点池，统一表示发现/配置的硬件图和搜索图。
 * 声明内可见成员（含嵌套成员）：name、attrs、key、value、nAttrs、type、parent、subs、nSubs。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclXmlNode {
/* 中文逐项-变量 V1833：name
 * name（char；成员 ncclXmlNode）：设备/属性/操作/参数的诊断名字，用于筛选、查找或报错。
 */
  char name[MAX_STR_LEN+1];
/* 中文逐项-变量 V1834：attrs
 * attrs（struct { char key[MAX_STR_LEN+1]; char value[MAX_STR_LEN+1]; }；成员
 * ncclXmlNode）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 */
/* 中文逐项-类型 T0154：<匿名struct>
 * 类型职责：内嵌匿名结构：把紧邻的一组属性组织到外层对象中，布局和拥有关系继承所在对象。
 * 声明内可见成员（含嵌套成员）：attrs、key、value。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
  struct {
/* 中文逐项-变量 V1835：key
 * key（char；成员 ncclXmlNode::<匿名>）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 */
    char key[MAX_STR_LEN+1];
/* 中文逐项-变量 V1836：value
 * value（char；成员 ncclXmlNode::<匿名>）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 */
    char value[MAX_STR_LEN+1];
  } attrs[MAX_ATTR_COUNT+1]; // Need an extra one to consume extra params
/* 中文逐项-变量 V1837：nAttrs
 * nAttrs（int；成员 ncclXmlNode）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 */
  int nAttrs;
/* 中文逐项-变量 V1838：type
 * type（int；成员 ncclXmlNode）：对象身份和类别编号，硬件节点id、通信图id、工作类型等作用域不同。
 */
  int type;
/* 中文逐项-变量 V1839：parent
 * parent（struct ncclXmlNode；成员 ncclXmlNode）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 */
  struct ncclXmlNode* parent;
/* 中文逐项-变量 V1840：subs
 * subs（struct ncclXmlNode；成员 ncclXmlNode）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 */
  struct ncclXmlNode* subs[MAX_SUBS];
/* 中文逐项-变量 V1841：nSubs
 * nSubs（int；成员 ncclXmlNode）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 */
  int nSubs;
};

/* 中文逐项-类型 T0155：ncclXml
 * 类型职责：轻量XML的节点、属性和固定节点池，统一表示发现/配置的硬件图和搜索图。
 * 声明内可见成员（含嵌套成员）：nodes、maxIndex。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclXml {
/* 中文逐项-变量 V1842：nodes
 * nodes（struct ncclXmlNode；成员 ncclXml）：硬件节点表、直接链路数/链路数组和预计算多跳路径缓存。
 */
  struct ncclXmlNode nodes[MAX_NODES];
/* 中文逐项-变量 V1843：maxIndex
 * maxIndex（int；成员 ncclXml）：XML固定节点池已用位置，分配新节点前须检查MAX_NODES。
 */
  int maxIndex;
};

/* File functions */
/* 中文逐项-常量 X0109：NCCL_TOPO_XML_VERSION
 * 编译期定义：NCCL_TOPO_XML_VERSION = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_TOPO_XML_VERSION 1
/* 中文逐项-接口 X0110：ncclTopoGetXmlFromFile
 * 接口职责：打开拓扑文件并解析指定版本的系统 XML，按 warn 决定缺失文件的处理方式。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetXmlFromFile(const char* xmlTopoFile, struct ncclXml* xml,
 * int warn);
 */
ncclResult_t ncclTopoGetXmlFromFile(const char* xmlTopoFile, struct ncclXml* xml, int warn);
/* 中文逐项-接口 X0111：ncclTopoDumpXmlToFile
 * 接口职责：递归把内存 XML 节点序列化并写出文件，供拓扑/搜索图调试和复用。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoDumpXmlToFile(const char* xmlTopoFile, struct ncclXml* xml);
 */
ncclResult_t ncclTopoDumpXmlToFile(const char* xmlTopoFile, struct ncclXml* xml);
/* 中文逐项-常量 X0112：NCCL_GRAPH_XML_VERSION
 * 编译期定义：NCCL_GRAPH_XML_VERSION = 1。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_GRAPH_XML_VERSION 1
/* 中文逐项-接口 X0113：ncclTopoGetXmlGraphFromFile
 * 接口职责：打开并解析通信图 XML 文件，校验图版本以恢复预先保存的搜索结果。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoGetXmlGraphFromFile(const char* xmlGraphFile, struct ncclXml*
 * xml);
 */
ncclResult_t ncclTopoGetXmlGraphFromFile(const char* xmlGraphFile, struct ncclXml* xml);

/* Auto-detect functions */
/* 中文逐项-接口 X0114：ncclTopoFillGpu
 * 接口职责：在 XML 找到或添加指定 GPU，触发系统/NVML 自动发现补全其属性。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoFillGpu(struct ncclXml* xml, const char* busId, struct
 * ncclXmlNode** gpuNode);
 */
ncclResult_t ncclTopoFillGpu(struct ncclXml* xml, const char* busId, struct ncclXmlNode** gpuNode);
/* 中文逐项-接口 X0115：ncclTopoFillNet
 * 接口职责：将后端提供的网络设备属性补入 XML，关联其 PCI/NIC 层次与 GDR/CollNet 能力。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoFillNet(struct ncclXml* xml, const char* pciPath, const char*
 * netName, struct ncclXmlNode** netNode);
 */
ncclResult_t ncclTopoFillNet(struct ncclXml* xml, const char* pciPath, const char* netName, struct ncclXmlNode** netNode);

/* Remove unneeded parts */
/* 中文逐项-接口 X0116：ncclTopoTrimXml
 * 接口职责：裁剪与当前通信设备无关的 XML 子树，保留当前运行所需硬件路径。
 * 此处是声明，真正实现/使用调用链在函数定义与函数表处；原始签名：ncclResult_t ncclTopoTrimXml(struct ncclXml* xml);
 */
ncclResult_t ncclTopoTrimXml(struct ncclXml* xml);

/**************/
/* XML Struct */
/* Functions  */
/**************/

/* 中文内部逻辑 F0569：xmlGetAttrIndex
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*index = -1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 准备局部状态：const int nAttrs = node->nAttrs;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 循环：初始 int a=0;；继续条件 a<nAttrs；每轮更新 a++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   4. 条件判断：(strncmp(node->attrs[a].key, attrName, MAX_STR_LEN) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 调用 / 计算 / 写入：*index = a;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0569
 */
/* 中文逐项-函数 F0569：xmlGetAttrIndex
 * 用途与位置：按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 index（int* index）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddNic；src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::xmlInitAttrInt；src/graph/topo.cc::xmlInitAttrUint64；src/graph/topo.cc::xmlInitAttrFloat；src/graph/xml.cc::ncclTopoGetXmlFromCpu；src/graph/xml.cc::ncclTopoGetXmlFromSys；其余见调用清单。
 * 本体直接出现的调用/包装：strncmp。
 * 需要对照的边界/推进条件：a<nAttrs；strncmp(node->attrs[a].key, attrName, MAX_STR_LEN) == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetAttrIndex(struct ncclXmlNode* node, const char* attrName, int* index) {
  *index = -1;
/* 中文逐项-变量 V1844：nAttrs
 * nAttrs（int；函数 xmlGetAttrIndex 内）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 初值/来源 nAttrs = node->nAttrs；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  const int nAttrs = node->nAttrs;
  for (/* 中文逐项-变量 V1845：a
 * a（int；函数 xmlGetAttrIndex 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 a = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：a<nAttrs；循环体与增量表达式决定每轮如何推进。
 */int a=0; a<nAttrs; a++) {
    if (strncmp(node->attrs[a].key, attrName, MAX_STR_LEN) == 0) {
      *index = a;
      return ncclSuccess;
    }
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0570：xmlGetAttr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 3. 调用 / 计算 / 写入：*value = index == -1 ? NULL : node->attrs[index].value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0570
 */
/* 中文逐项-函数 F0570：xmlGetAttr
 * 用途与位置：读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const char** value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetGraphFromXmlSub；src/graph/topo.cc::ncclTopoAddNet；src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu；src/graph/topo.cc::ncclTopoAddNvLinks；src/graph/xml.cc::ncclTopoXmlLoadSystem；src/graph/xml.cc::ncclTopoGetXmlFromCpu；其余见调用清单。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlGetAttrIndex。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetAttr(struct ncclXmlNode* node, const char* attrName, const char** value) {
/* 中文逐项-变量 V1846：index
 * index（int；函数 xmlGetAttr 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  *value = index == -1 ? NULL : node->attrs[index].value;
  return ncclSuccess;
}

/* 中文内部逻辑 F0571：xmlGetAttrStr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(node, attrName, value));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 2. 条件判断：(*value == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   3. 真分支：；仅条件为真执行以下子项。
 *     4. 调用 / 计算 / 写入：WARN("Attribute %s of node %s not found", attrName, node->name);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     5. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0571
 */
/* 中文逐项-函数 F0571：xmlGetAttrStr
 * 用途与位置：读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const char** value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu；src/graph/topo.cc::ncclTopoAddNvLinks；src/graph/xml.h::xmlGetAttrInt；src/graph/xml.h::xmlGetAttrFloat。
 * 本体直接出现的调用/包装：NCCLCHECK；xmlGetAttr。
 * 需要对照的边界/推进条件：*value == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetAttrStr(struct ncclXmlNode* node, const char* attrName, const char** value) {
  NCCLCHECK(xmlGetAttr(node, attrName, value));
  if (*value == NULL) {
    WARN("Attribute %s of node %s not found", attrName, node->name);
    return ncclInternalError;
  }
  return ncclSuccess;
}
/* 中文内部逻辑 F0572：xmlGetAttrInt
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const char* str;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrStr(node, attrName, &str));；失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。
 * 3. 调用 / 计算 / 写入：*value = strtol(str, NULL, 0);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0572
 */
/* 中文逐项-函数 F0572：xmlGetAttrInt
 * 用途与位置：读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（int* value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetChannelFromXml；src/graph/search.cc::ncclTopoGetGraphFromXmlSub；src/graph/topo.cc::ncclTopoAddNet；src/graph/topo.cc::ncclTopoAddGpu；src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu；src/graph/topo.cc::ncclTopoAddNvLinks；其余见调用清单。
 * 本体直接出现的调用/包装：NCCLCHECK；strtol；xmlGetAttrStr。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetAttrInt(struct ncclXmlNode* node, const char* attrName, int* value) {
/* 中文逐项-变量 V1847：str
 * str（char；函数 xmlGetAttrInt 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrStr(node, attrName, &str));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;
  NCCLCHECK(xmlGetAttrStr(node, attrName, &str));
  *value = strtol(str, NULL, 0);
  return ncclSuccess;
}

/* 中文内部逻辑 F0573：xmlGetAttrIntDefault
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const char* str;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(node, attrName, &str));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 * 3. 调用 / 计算 / 写入：*value = str ? strtol(str, NULL, 0) : defaultValue;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0573
 */
/* 中文逐项-函数 F0573：xmlGetAttrIntDefault
 * 用途与位置：读取可选整数属性，缺失时写调用者指定的默认值。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（int* value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 参数 defaultValue（int defaultValue）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddNet。
 * 本体直接出现的调用/包装：NCCLCHECK；strtol；xmlGetAttr。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetAttrIntDefault(struct ncclXmlNode* node, const char* attrName, int* value, int defaultValue) {
/* 中文逐项-变量 V1848：str
 * str（char；函数 xmlGetAttrIntDefault 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(node, attrName, &str));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;
  NCCLCHECK(xmlGetAttr(node, attrName, &str));
  *value = str ? strtol(str, NULL, 0) : defaultValue;
  return ncclSuccess;
}


/* 中文内部逻辑 F0574：xmlGetAttrFloat
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：const char* str;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrStr(node, attrName, &str));；失败时可能提前return，成功才继续；读取必需字符串属性，缺失时报错，适用于不能省略的拓扑字段。
 * 3. 调用 / 计算 / 写入：*value = strtof(str, NULL);；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0574
 */
/* 中文逐项-函数 F0574：xmlGetAttrFloat
 * 用途与位置：读取必需属性字符串并转成整数/浮点，供硬件宽度、rank、版本等数值属性使用。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（float* value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetGraphFromXmlSub；src/graph/topo.cc::ncclTopoAddNet。
 * 本体直接出现的调用/包装：NCCLCHECK；strtof；xmlGetAttrStr。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetAttrFloat(struct ncclXmlNode* node, const char* attrName, float* value) {
/* 中文逐项-变量 V1849：str
 * str（char；函数 xmlGetAttrFloat 内）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrStr(node, attrName, &str));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  const char* str;
  NCCLCHECK(xmlGetAttrStr(node, attrName, &str));
  *value = strtof(str, NULL);
  return ncclSuccess;
}

/* 中文内部逻辑 F0575：xmlFindTag
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*node = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 循环：初始 int i=0;；继续条件 i<xml->maxIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 准备局部状态：struct ncclXmlNode* n = xml->nodes+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   4. 条件判断：(strcmp(n->name, tagName) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 调用 / 计算 / 写入：*node = n;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       7. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 9. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0575
 */
/* 中文逐项-函数 F0575：xmlFindTag
 * 用途与位置：搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 tagName（const char* tagName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 node（struct ncclXmlNode** node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoGetSystemFromXml；src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoFillNet。
 * 本体直接出现的调用/包装：strcmp。
 * 需要对照的边界/推进条件：i<xml->maxIndex；strcmp(n->name, tagName) == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlFindTag(struct ncclXml* xml, const char* tagName, struct ncclXmlNode** node) {
  *node = NULL;
  for (/* 中文逐项-变量 V1850：i
 * i（int；函数 xmlFindTag 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<xml->maxIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<xml->maxIndex; i++) {
/* 中文逐项-变量 V1851：n
 * n（struct ncclXmlNode；函数 xmlFindTag 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = xml->nodes+i；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* n = xml->nodes+i;
    if (strcmp(n->name, tagName) == 0) {
      *node = n;
      return ncclSuccess;
    }
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0576：xmlFindTagKv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*node = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 循环：初始 int i=0;；继续条件 i<xml->maxIndex；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 准备局部状态：struct ncclXmlNode* n = xml->nodes+i;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   4. 条件判断：(strcmp(n->name, tagName) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 准备局部状态：const char* value;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       7. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(n, attrName, &value));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 *       8. 条件判断：(value && strcmp(value, attrValue) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         9. 真分支：；仅条件为真执行以下子项。
 *           10. 调用 / 计算 / 写入：*node = n;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0576
 */
/* 中文逐项-函数 F0576：xmlFindTagKv
 * 用途与位置：搜索满足标签名及可选键值匹配的 XML 节点，用于复用已有硬件对象。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 tagName（const char* tagName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 node（struct ncclXmlNode** node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 attrValue（const char* attrValue）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetPciNode；src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoFillNet。
 * 本体直接出现的调用/包装：NCCLCHECK；strcmp；xmlGetAttr。
 * 需要对照的边界/推进条件：i<xml->maxIndex；strcmp(n->name, tagName) == 0；value && strcmp(value, attrValue) == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlFindTagKv(struct ncclXml* xml, const char* tagName, struct ncclXmlNode** node, const char* attrName, const char* attrValue) {
  *node = NULL;
  for (/* 中文逐项-变量 V1852：i
 * i（int；函数 xmlFindTagKv 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：i<xml->maxIndex；循环体与增量表达式决定每轮如何推进。
 */int i=0; i<xml->maxIndex; i++) {
/* 中文逐项-变量 V1853：n
 * n（struct ncclXmlNode；函数 xmlFindTagKv 内）：当前对象的有效数量或归约除数，按所属Fan/FuncSumPostDiv/列表以及实际赋值确认。
 * 初值/来源 n = xml->nodes+i；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* n = xml->nodes+i;
    if (strcmp(n->name, tagName) == 0) {
/* 中文逐项-变量 V1854：value
 * value（char；函数 xmlFindTagKv 内）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(n, attrName, &value));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      const char* value;
      NCCLCHECK(xmlGetAttr(n, attrName, &value));
      if (value && strcmp(value, attrValue) == 0) {
        *node = n;
        return ncclSuccess;
      }
    }
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0577：xmlSetAttr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 3. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：index = node->nAttrs++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     7. 调用 / 计算 / 写入：node->attrs[index].key[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：strncpy(node->attrs[index].value, value, MAX_STR_LEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 调用 / 计算 / 写入：node->attrs[index].value[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0577
 */
/* 中文逐项-函数 F0577：xmlSetAttr
 * 用途与位置：在节点中更新已有属性或新增一项键值，维护属性计数和固定容量边界。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const char* value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetXmlFromGraph；src/graph/xml.cc::ncclTopoSetAttrFromSys；src/graph/xml.cc::ncclTopoGetXmlFromCpu；src/graph/xml.cc::ncclTopoGetPciNode；src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoGetXmlFromGpu；src/graph/xml.cc::ncclTopoFillNet。
 * 本体直接出现的调用/包装：NCCLCHECK；strncpy；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlSetAttr(struct ncclXmlNode* node, const char* attrName, const char* value) {
/* 中文逐项-变量 V1855：index
 * index（int；函数 xmlSetAttr 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    node->attrs[index].key[MAX_STR_LEN] = '\0';
  }
  strncpy(node->attrs[index].value, value, MAX_STR_LEN);
  node->attrs[index].value[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}

/* 中文内部逻辑 F0578：xmlSetAttrIfUnset
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 3. 条件判断：(index != -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 调用 / 计算 / 写入：index = node->nAttrs++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 8. 调用 / 计算 / 写入：node->attrs[index].key[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：strncpy(node->attrs[index].value, value, MAX_STR_LEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 调用 / 计算 / 写入：node->attrs[index].value[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0578
 */
/* 中文逐项-函数 F0578：xmlSetAttrIfUnset
 * 用途与位置：只给缺失属性设置默认值，避免自动发现结果覆盖用户明确配置。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const char* value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoFillGpu；src/graph/xml.cc::ncclTopoFillNet。
 * 本体直接出现的调用/包装：NCCLCHECK；strncpy；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：index != -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlSetAttrIfUnset(struct ncclXmlNode* node, const char* attrName, const char* value) {
/* 中文逐项-变量 V1856：index
 * index（int；函数 xmlSetAttrIfUnset 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index != -1) return ncclSuccess;
  index = node->nAttrs++;
  strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
  node->attrs[index].key[MAX_STR_LEN] = '\0';
  strncpy(node->attrs[index].value, value, MAX_STR_LEN);
  node->attrs[index].value[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}

/* 中文内部逻辑 F0579：xmlSetAttrInt
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 3. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：index = node->nAttrs++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     7. 调用 / 计算 / 写入：node->attrs[index].key[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：snprintf(node->attrs[index].value, MAX_STR_LEN, "%d", value);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 调用 / 计算 / 写入：node->attrs[index].value[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0579
 */
/* 中文逐项-函数 F0579：xmlSetAttrInt
 * 用途与位置：把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const int value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetXmlFromChannel；src/graph/search.cc::ncclTopoGetXmlFromGraph；src/graph/search.cc::ncclTopoGetXmlFromGraphs；src/graph/topo.cc::ncclTopoGetSystem；src/graph/xml.cc::ncclTopoGetXmlFromCpu；src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoGetXmlFromGpu。
 * 本体直接出现的调用/包装：NCCLCHECK；snprintf；strncpy；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlSetAttrInt(struct ncclXmlNode* node, const char* attrName, const int value) {
/* 中文逐项-变量 V1857：index
 * index（int；函数 xmlSetAttrInt 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    node->attrs[index].key[MAX_STR_LEN] = '\0';
  }
  snprintf(node->attrs[index].value, MAX_STR_LEN, "%d", value);
  node->attrs[index].value[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}

/* 中文内部逻辑 F0580：xmlSetAttrFloat
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 3. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 调用 / 计算 / 写入：index = node->nAttrs++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     6. 调用 / 计算 / 写入：strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     7. 调用 / 计算 / 写入：node->attrs[index].key[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：snprintf(node->attrs[index].value, MAX_STR_LEN, "%g", value);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 调用 / 计算 / 写入：node->attrs[index].value[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0580
 */
/* 中文逐项-函数 F0580：xmlSetAttrFloat
 * 用途与位置：把数值格式化成字符串后写 XML 属性，统一与文本 XML 表示对接。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 value（const float value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetXmlFromGraph。
 * 本体直接出现的调用/包装：NCCLCHECK；snprintf；strncpy；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：index == -1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlSetAttrFloat(struct ncclXmlNode* node, const char* attrName, const float value) {
/* 中文逐项-变量 V1858：index
 * index（int；函数 xmlSetAttrFloat 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) {
    index = node->nAttrs++;
    strncpy(node->attrs[index].key, attrName, MAX_STR_LEN);
    node->attrs[index].key[MAX_STR_LEN] = '\0';
  }
  snprintf(node->attrs[index].value, MAX_STR_LEN, "%g", value);
  node->attrs[index].value[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}

/* 中文内部逻辑 F0581：xmlUnsetAttr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：int index;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));；失败时可能提前return，成功才继续；按属性名在节点固定属性数组中找下标，缺失时写 -1 让上层决定默认值或错误。
 * 3. 条件判断：(index == -1)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 循环：初始 int i=index+1;；继续条件 i<node->nAttrs；每轮更新 i++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   7. 调用 / 计算 / 写入：strcpy(node->attrs[i-1].key, node->attrs[i].key);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *   8. 调用 / 计算 / 写入：strcpy(node->attrs[i-1].value, node->attrs[i].value);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 9. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 10. 调用 / 计算 / 写入：node->nAttrs--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0581
 */
/* 中文逐项-函数 F0581：xmlUnsetAttr
 * 用途与位置：删除指定属性并整理数组，防止遗留键值参与之后的匹配/导出。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoTrimXmlRec。
 * 本体直接出现的调用/包装：NCCLCHECK；strcpy；xmlGetAttrIndex。
 * 需要对照的边界/推进条件：index == -1；i<node->nAttrs。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlUnsetAttr(struct ncclXmlNode* node, const char* attrName) {
/* 中文逐项-变量 V1859：index
 * index（int；函数 xmlUnsetAttr 内）：当前对象的数组/角色下标；channel.index 专指 GPU 工作 FIFO 的持久读取下标。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int index;
  NCCLCHECK(xmlGetAttrIndex(node, attrName, &index));
  if (index == -1) return ncclSuccess;
  for (/* 中文逐项-变量 V1860：i
 * i（int；函数 xmlUnsetAttr 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 i = index+1；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 * 所在for循环的继续条件：i<node->nAttrs；循环体与增量表达式决定每轮如何推进。
 */int i=index+1; i<node->nAttrs; i++) {
    strcpy(node->attrs[i-1].key, node->attrs[i].key);
    strcpy(node->attrs[i-1].value, node->attrs[i].value);
  }
  node->nAttrs--;
  return ncclSuccess;
}

/* 中文内部逻辑 F0582：xmlGetSub
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*sub = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 循环：初始 int s=0;；继续条件 s<node->nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 条件判断：(strcmp(node->subs[s]->name, subName) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：*sub = node->subs[s];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 7. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 8. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0582
 */
/* 中文逐项-函数 F0582：xmlGetSub
 * 用途与位置：仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 subName（const char* subName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 sub（struct ncclXmlNode** sub）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/topo.cc::ncclTopoAddPci；src/graph/xml.cc::ncclTopoGetXmlFromGpu；src/graph/xml.cc::ncclTopoFillNet。
 * 本体直接出现的调用/包装：strcmp。
 * 需要对照的边界/推进条件：s<node->nSubs；strcmp(node->subs[s]->name, subName) == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetSub(struct ncclXmlNode* node, const char* subName, struct ncclXmlNode** sub) {
  *sub = NULL;
  for (/* 中文逐项-变量 V1861：s
 * s（int；函数 xmlGetSub 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<node->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<node->nSubs; s++) {
    if (strcmp(node->subs[s]->name, subName) == 0) {
      *sub = node->subs[s];
      return ncclSuccess;
    }
  }
  return ncclSuccess;
}

/* 中文内部逻辑 F0583：xmlGetSubKv
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：*sub = NULL;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 循环：初始 int s=0;；继续条件 s<node->nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 准备局部状态：struct ncclXmlNode* subNode = node->subs[s];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *   4. 条件判断：(strcmp(subNode->name, subName) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     5. 真分支：；仅条件为真执行以下子项。
 *       6. 准备局部状态：const char* value;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 *       7. 调用 / 计算 / 写入：NCCLCHECK(xmlGetAttr(subNode, attrName, &value));；失败时可能提前return，成功才继续；读取可选字符串属性，缺失时返回空指针而不自动填默认值。
 *       8. 条件判断：(value && strcmp(value, attrValue) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         9. 真分支：；仅条件为真执行以下子项。
 *           10. 调用 / 计算 / 写入：*sub = node->subs[s];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *           11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 12. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 13. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0583
 */
/* 中文逐项-函数 F0583：xmlGetSubKv
 * 用途与位置：仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 subName（const char* subName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 sub（struct ncclXmlNode** sub）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 attrValue（const char* attrValue）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoGetXmlFromGpu；src/graph/xml.h::xmlGetSubKvInt。
 * 本体直接出现的调用/包装：NCCLCHECK；strcmp；xmlGetAttr。
 * 需要对照的边界/推进条件：s<node->nSubs；strcmp(subNode->name, subName) == 0；value && strcmp(value, attrValue) ==
 * 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetSubKv(struct ncclXmlNode* node, const char* subName, struct ncclXmlNode** sub, const char* attrName, const char* attrValue) {
  *sub = NULL;
  for (/* 中文逐项-变量 V1862：s
 * s（int；函数 xmlGetSubKv 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<node->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<node->nSubs; s++) {
/* 中文逐项-变量 V1863：subNode
 * subNode（struct ncclXmlNode；函数 xmlGetSubKv 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 subNode = node->subs[s]；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
    struct ncclXmlNode* subNode = node->subs[s];
    if (strcmp(subNode->name, subName) == 0) {
/* 中文逐项-变量 V1864：value
 * value（char；函数 xmlGetSubKv 内）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(xmlGetAttr(subNode, attrName, &value));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
      const char* value;
      NCCLCHECK(xmlGetAttr(subNode, attrName, &value));
      if (value && strcmp(value, attrValue) == 0) {
        *sub = node->subs[s];
        return ncclSuccess;
      }
    }
  }
  return ncclSuccess;
}
/* 中文内部逻辑 F0584：xmlGetSubKvInt
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：char strValue[10];；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 调用 / 计算 / 写入：snprintf(strValue, 10, "%d", attrValue);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 3. 调用 / 计算 / 写入：NCCLCHECK(xmlGetSubKv(node, subName, sub, attrName, strValue));；失败时可能提前return，成功才继续；仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 * 4. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0584
 */
/* 中文逐项-函数 F0584：xmlGetSubKvInt
 * 用途与位置：仅在当前节点直接子项中按标签及属性匹配目标，整数版本负责文本格式转换。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 参数 subName（const char* subName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 sub（struct ncclXmlNode** sub）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 参数 attrName（const char* attrName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 attrValue（const int attrValue）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 直接调用检索未命中；公开API、线程入口、模板及函数表可能间接进入，不能因此认定不可达。
 * 本体直接出现的调用/包装：NCCLCHECK；snprintf；xmlGetSubKv。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlGetSubKvInt(struct ncclXmlNode* node, const char* subName, struct ncclXmlNode** sub, const char* attrName, const int attrValue) {
/* 中文逐项-变量 V1865：strValue
 * strValue（char；函数 xmlGetSubKvInt 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：snprintf(strValue, 10, "%d", attrValue);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  char strValue[10];
  snprintf(strValue, 10, "%d", attrValue);
  NCCLCHECK(xmlGetSubKv(node, subName, sub, attrName, strValue));
  return ncclSuccess;
}

/* 中文内部逻辑 F0585：xmlAddNode
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 条件判断：(xml->maxIndex == MAX_NODES)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   2. 真分支：；仅条件为真执行以下子项。
 *     3. 调用 / 计算 / 写入：WARN("Error : too many XML nodes (max %d)", MAX_NODES);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 *     4. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 5. 准备局部状态：struct ncclXmlNode* s = xml->nodes+xml->maxIndex++;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 6. 调用 / 计算 / 写入：s->nSubs = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 7. 调用 / 计算 / 写入：s->nAttrs = 0;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 调用 / 计算 / 写入：*sub = s;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 9. 调用 / 计算 / 写入：s->parent = parent;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 10. 条件判断：(parent)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   11. 真分支：；仅条件为真执行以下子项。
 *     12. 调用 / 计算 / 写入：parent->subs[parent->nSubs++] = s;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 13. 调用 / 计算 / 写入：strncpy(s->name, subName, MAX_STR_LEN);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 14. 调用 / 计算 / 写入：s->name[MAX_STR_LEN] = '\0';；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0585
 */
/* 中文逐项-函数 F0585：xmlAddNode
 * 用途与位置：从 XML 固定节点池分配对象并关联父子关系，检查最大节点/子项限制。
 * 参数 xml（struct ncclXml* xml）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 parent（struct ncclXmlNode* parent）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 * 参数 subName（const char* subName）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 sub（struct ncclXmlNode** sub）：Proxy 聚合任务的子项数、子项数组或当前子项；各项仍保留自己的连接和进度。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetXmlFromChannel；src/graph/search.cc::ncclTopoGetXmlFromGraph；src/graph/search.cc::ncclTopoGetXmlFromGraphs；src/graph/topo.cc::ncclTopoGetSystem；src/graph/xml.cc::ncclTopoGetPciNode；src/graph/xml.cc::ncclTopoGetXmlFromSys；src/graph/xml.cc::ncclTopoGetXmlFromGpu；其余见调用清单。
 * 本体直接出现的调用/包装：strncpy。
 * 需要对照的边界/推进条件：xml->maxIndex == MAX_NODES；parent。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclInternalError;；return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlAddNode(struct ncclXml* xml, struct ncclXmlNode* parent, const char* subName, struct ncclXmlNode** sub) {
  if (xml->maxIndex == MAX_NODES) {
    WARN("Error : too many XML nodes (max %d)", MAX_NODES);
    return ncclInternalError;
  }
/* 中文逐项-变量 V1866：s
 * s（struct ncclXmlNode；函数 xmlAddNode 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 s = xml->nodes+xml->maxIndex++；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclXmlNode* s = xml->nodes+xml->maxIndex++;
  s->nSubs = 0;
  s->nAttrs = 0;
  *sub = s;
  s->parent = parent;
  if (parent) parent->subs[parent->nSubs++] = s;
  strncpy(s->name, subName, MAX_STR_LEN);
  s->name[MAX_STR_LEN] = '\0';
  return ncclSuccess;
}

/* 中文内部逻辑 F0586：xmlRemoveNode
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 调用 / 计算 / 写入：node->type = NODE_TYPE_NONE;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 2. 准备局部状态：struct ncclXmlNode* parent = node->parent;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 3. 条件判断：(parent == NULL)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *   4. 真分支：；仅条件为真执行以下子项。
 *     5. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 6. 准备局部状态：int shift = 0;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 7. 循环：初始 int s=0;；继续条件 s<parent->nSubs；每轮更新 s++；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   8. 条件判断：(parent->subs[s] == node)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     9. 真分支：；仅条件为真执行以下子项。
 *       10. 调用 / 计算 / 写入：shift = 1;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *     11. 假分支：；仅前面条件为假时执行；else-if继续判断。
 *       12. 条件判断：(shift)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *         13. 真分支：；仅条件为真执行以下子项。
 *           14. 调用 / 计算 / 写入：parent->subs[s-1] = parent->subs[s];；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 15. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 16. 调用 / 计算 / 写入：parent->nSubs--;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 17. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0586
 */
/* 中文逐项-函数 F0586：xmlRemoveNode
 * 用途与位置：从父节点子项表摘除目标节点并整理关系，裁剪不需要的拓扑分支。
 * 参数 node（struct ncclXmlNode* node）：硬件节点或当前计算节点编号，具体由声明类型及赋值来源决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/xml.cc::ncclTopoTrimXmlRec。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：parent == NULL；s<parent->nSubs；parent->subs[s] == node；shift。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t xmlRemoveNode(struct ncclXmlNode* node) {
  node->type = NODE_TYPE_NONE;
/* 中文逐项-变量 V1867：parent
 * parent（struct ncclXmlNode；函数 xmlRemoveNode 内）：max层级父节点引用和直接子节点数，XML递归增删必须保持一致。
 * 初值/来源 parent = node->parent；由已有上下文/数组成员读取或定位本轮对象，复用的是该字段引用/值。
 */
  struct ncclXmlNode* parent = node->parent;
  if (parent == NULL) return ncclSuccess;
/* 中文逐项-变量 V1868：shift
 * shift（int；函数 xmlRemoveNode 内）：在多 head/peer 间轮转分片次序的偏移，平衡并发访问。
 * 初值/来源 shift = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 */
  int shift = 0;
  for (/* 中文逐项-变量 V1869：s
 * s（int；函数 xmlRemoveNode 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 初值/来源 s = 0；初始清零；有效数量/进度从零开始，指针类型时表示空地址。
 * 所在for循环的继续条件：s<parent->nSubs；循环体与增量表达式决定每轮如何推进。
 */int s=0; s<parent->nSubs; s++) {
    if (parent->subs[s] == node) shift = 1;
    else if (shift) parent->subs[s-1] = parent->subs[s];
  }
  parent->nSubs--;
  return ncclSuccess;
}

// Dictionary for STR -> INT conversions. No dictionary size information,
// there needs to be a last element with str == NULL.
/* 中文逐项-类型 T0156：kvDict
 * 类型职责：字符串与内部枚举整数的对应项，使XML/配置可读值转换成内部常量。
 * 声明内可见成员（含嵌套成员）：str、value。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct kvDict {
/* 中文逐项-变量 V1870：str
 * str（char；成员 kvDict）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 */
  const char* str;
/* 中文逐项-变量 V1871：value
 * value（int；成员 kvDict）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 */
  int value;
};

/* 中文内部逻辑 F0587：kvConvertToInt
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct kvDict* d = dict;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 ；继续条件 (d->str)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 条件判断：(strncmp(str, d->str, strlen(d->str)) == 0)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：*value = d->value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   7. 调用 / 计算 / 写入：d++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 9. 调用 / 计算 / 写入：INFO(NCCL_GRAPH, "KV Convert to int : could not find value of '%s' in dictionary, falling back to %d", str, d->value);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 调用 / 计算 / 写入：*value = d->value;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 11. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0587
 */
/* 中文逐项-函数 F0587：kvConvertToInt
 * 用途与位置：在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。
 * 参数 str（const char* str）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 参数 value（int* value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 参数 dict（struct kvDict* dict）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetGraphFromXmlSub；src/graph/topo.cc::ncclTopoAddPci；src/graph/topo.cc::ncclTopoAddCpu；src/graph/topo.cc::ncclTopoAddNvLinks。
 * 本体直接出现的调用/包装：strlen；strncmp。
 * 需要对照的边界/推进条件：d->str；strncmp(str, d->str, strlen(d->str)) == 0。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t kvConvertToInt(const char* str, int* value, struct kvDict* dict) {
/* 中文逐项-变量 V1872：d
 * d（struct kvDict；函数 kvConvertToInt 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 d = dict；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  struct kvDict* d = dict;
  while (d->str) {
    if (strncmp(str, d->str, strlen(d->str)) == 0) {
      *value = d->value;
      return ncclSuccess;
    }
    d++;
  }
  INFO(NCCL_GRAPH, "KV Convert to int : could not find value of '%s' in dictionary, falling back to %d", str, d->value);
  *value = d->value;
  return ncclSuccess;
}
/* 中文内部逻辑 F0588：kvConvertToStr
 * 以下编号是静态阅读顺序，缩进表示分支/循环；并非并行线程时间线。
 * 1. 准备局部状态：struct kvDict* d = dict;；声明本作用域临时值/引用并按右侧初始化；此阶段准备后续分支或数据地址，不自动创建新的通信连接。
 * 2. 循环：初始 ；继续条件 (d->str)；每轮更新；每轮按条件决定是否继续；循环体结束后执行增量/再检查，break和continue另有控制转移。
 *   3. 条件判断：(value == d->value)；为真只进入真分支；为假才考虑else，之后在分支出口继续。
 *     4. 真分支：；仅条件为真执行以下子项。
 *       5. 调用 / 计算 / 写入：*str = d->str;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 *       6. 函数返回：return ncclSuccess;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 *   7. 调用 / 计算 / 写入：d++;；按此表达式更新本地值或目标字段，后续条件/地址计算使用更新后的值。
 * 8. 循环出口：；条件不再满足或遇break后继续循环后代码；遇return/goto则转到对应出口。
 * 9. 调用 / 计算 / 写入：WARN("KV Convert to str : could not find value %d in dictionary", value);；调用上述接口/对象方法；返回值和副作用按实际函数/函数表契约解释，静态名字不足以解析全部重载。
 * 10. 函数返回：return ncclInternalError;；立即结束本函数当前路径；在组合原语包装中仍须看被返回调用的内部同步职责。
 * 配套图解：docs/zh_CN/visual-guide.md；完整内部步骤：docs/zh_CN/logic/src__graph__xml_h.md#f0588
 */
/* 中文逐项-函数 F0588：kvConvertToStr
 * 用途与位置：在名称/数值字典中双向转换枚举属性，使 XML 可读名称对应内部路径/模式常量。
 * 参数 value（int value）：XML节点属性键值数组、有效条数与单个键/值，字段容量有固定上限。
 * 参数 str（const char** str）：string或接口前缀，用于环境配置、XML或接口过滤，比较时注意是否允许前缀匹配。
 * 参数 dict（struct kvDict* dict）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/search.cc::ncclTopoGetXmlFromGraph。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：d->str；value == d->value。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;；return ncclInternalError;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t kvConvertToStr(int value, const char** str, struct kvDict* dict) {
/* 中文逐项-变量 V1873：d
 * d（struct kvDict；函数 kvConvertToStr 内）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 初值/来源 d = dict；从该常量/已有表达式取值，后续使用保持本作用域的类型和编号空间。
 */
  struct kvDict* d = dict;
  while (d->str) {
    if (value == d->value) {
      *str = d->str;
      return ncclSuccess;
    }
    d++;
  }
  WARN("KV Convert to str : could not find value %d in dictionary", value);
  return ncclInternalError;
}

#endif
