/*************************************************************************
 * Copyright (c) 2015-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include <stdlib.h>

#ifndef NCCL_P2P_H_
/* 中文逐项-常量 X1043：NCCL_P2P_H_
 * 编译期定义：NCCL_P2P_H_ = <空替换体>。
 * 下方预处理条件决定该定义何时有效；空替换体常作头文件保护/能力开关，数值/字符串/类型别名需按实际替换内容解释。
 */
#define NCCL_P2P_H_

/* 中文逐项-类型 T0360：ncclP2Pinfo
 * 类型职责：Group保存的一条P2P消息的缓冲指针和字节数，尚未决定设备端每块的执行。
 * 声明内可见成员（含嵌套成员）：buff、nbytes。各字段在声明旁逐项解释，不假设同名字段在别的结构中含义相同。
 */
struct ncclP2Pinfo {
/* 中文逐项-变量 V3051：buff
 * buff（void；成员 ncclP2Pinfo）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 */
  void* buff;
/* 中文逐项-变量 V3052：nbytes
 * nbytes（ssize_t；成员 ncclP2Pinfo）：当前消息或片段的字节规模；info.nBytes 对 AG/RS 还计入 nRanks 段总规模。
 */
  ssize_t nbytes;
};

typedef ncclRecyclableList<struct ncclP2Pinfo> ncclP2Plist;

/* 中文逐项-函数 F0869：ncclSaveP2pInfo
 * 用途与位置：按 peer 的可复用列表保存缓冲指针和字节数，供最外层 GroupEnd 统一分块和配对调度。
 * 参数 <未命名参数>（ncclP2Plist* &p2p）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 buff（void* buff）：当前工作范围的缓冲起点；P2P 任务通常已经按 Int8 把 count 归一为字节。
 * 参数 nBytes（ssize_t nBytes）：当前消息或片段的字节规模；info.nBytes 对 AG/RS 还计入 nRanks 段总规模。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/enqueue.cc::ncclSaveP2p。
 * 本体直接出现的调用/包装：NCCLCHECK；p2p->getNewElem。
 * 需要对照的边界/推进条件：p2p == NULL。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
static ncclResult_t ncclSaveP2pInfo(ncclP2Plist* &p2p, void* buff, ssize_t nBytes) {
  if (p2p == NULL) p2p = new ncclP2Plist();
/* 中文逐项-变量 V3053：next
 * next（struct ncclP2Pinfo；函数 ncclSaveP2pInfo 内）：队列/请求链的后继地址；环算法结构中则表示后继rank，按所属对象区分。
 * 此处未显式给初值；后续首个文本使用点：NCCLCHECK(p2p->getNewElem(&next));。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  struct ncclP2Pinfo* next;
  NCCLCHECK(p2p->getNewElem(&next));
  next->buff = buff;
  next->nbytes = nBytes;
  return ncclSuccess;
}
#endif
