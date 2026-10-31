/*************************************************************************
 * Copyright (c) 2016-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "nccl.h"

/* 中文逐项-宏 M0046：RANK_TO_INDEX
 * 宏职责：用下面的参数与展开式生成编译期表达式/调用包装；调用实参会原样替换，必须注意括号、求值次数及返回/跳转副作用。
 * 原始宏名 RANK_TO_INDEX；完整替换体紧邻下方，续行反斜线属于宏定义的一部分。注释放在宏定义外，未改替换体。
 */
#define RANK_TO_INDEX(r) (rank > root ? rank-1 : rank)

/* Btree which alternates leaves and nodes.
 * Assumes root is 0, which conveniently builds a tree on powers of two,
 * (because we have pow2-1 ranks) which lets us manipulate bits.
 * Find first non-zero bit, then :
 * Find the parent :
 *   xx01[0] -> xx10[0] (1,5,9 below) or xx00[0] if xx10[0] is out of bounds (13 below)
 *   xx11[0] -> xx10[0] (3,7,11 below)
 * Find the children :
 *   xx10[0] -> xx01[0] (2,4,6,8,10,12) or -1 (1,3,5,7,9,11,13)
 *   xx10[0] -> xx11[0] (2,4,6,8,10) or xx101[0] (12) or xx1001[0] ... or -1 (1,3,5,7,9,11,13)
 *
 * Illustration :
 * 0---------------8
 *          ______/ \______
 *         4               12
 *       /   \            /  \
 *     2       6       10     \
 *    / \     / \     /  \     \
 *   1   3   5   7   9   11    13
 */
/* 中文精读：用 rank 位模式生成二叉树
 * 最低置位 bit 决定当前 rank 在树中的层级，翻转/提升该位构造父节点，bit/2 构造孩子。
 * rank=0 单独作为根，-1 表示不存在的父/子；非 2 的幂规模要额外处理越界孩子。
 * 这里处理的 rank 可是节点级别编号，由 connectTrees 再映射到实际 GPU rank。
 * 它负责生成邻接关系，不分配通信缓冲，也不发起 GPU 归约。
 */
/* 中文逐项-函数 F0526：ncclGetBtree
 * 用途与位置：根据 rank 最低置位构造二叉树父/子编号，并处理非二次幂规模的越界孩子。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 u（int* u）：当前遍历或处理对象的局部引用/值，类型和下面初始化来源确定其具体身份。
 * 参数 d0（int* d0）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 d1（int* d1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 parentChildType（int* parentChildType）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/trees.cc::ncclGetDtree。
 * 本体不依赖额外可见函数调用，主要执行当前表达式、字段读写或返回。
 * 需要对照的边界/推进条件：bit<nranks；bit & rank；rank == 0；up >= nranks。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGetBtree(int nranks, int rank, int* u, int* d0, int* d1, int* parentChildType) {
/* 中文逐项-变量 V1643：up、down0、down1
 * up（int；函数 ncclGetBtree 内）：树父/孩子用户 rank；-1 表示对应邻居不存在。
 * 此处未显式给初值；后续首个文本使用点：up = (rank ^ bit) | (bit << 1);。若由输出参数填入，必须先检查填充调用成功再读值。
 * down0（int；函数 ncclGetBtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：down0 = lowbit == 0 ? -1 : rank-lowbit;。若由输出参数填入，必须先检查填充调用成功再读值。
 * down1（int；函数 ncclGetBtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：down1 = lowbit == 0 ? -1 : rank+lowbit;。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int up, down0, down1;
/* 中文逐项-变量 V1644：bit
 * bit（int；函数 ncclGetBtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：for (bit=1; bit<nranks; bit<<=1) {。若由输出参数填入，必须先检查填充调用成功再读值。
 */
  int bit;
  for (bit=1; bit<nranks; bit<<=1) {
    if (bit & rank) break;
  }

  if (rank == 0) {
    *u = -1;
    *d0 = -1;
    // Child rank is > 0 so it has to be our child 1, not 0.
    *d1 = nranks > 1 ? bit >> 1 : -1;
    return ncclSuccess;
  }

  up = (rank ^ bit) | (bit << 1);
  // if smaller than the parent, we are his first child, otherwise we're his second
  if (up >= nranks) up = (rank ^ bit);
  *parentChildType = (rank < up) ? 0 : 1;
  *u = up;

/* 中文逐项-变量 V1645：lowbit
 * lowbit（int；函数 ncclGetBtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 lowbit = bit >> 1；按位移编码/提取标志或二次幂容量，区别于普通字节地址加法。
 */
  int lowbit = bit >> 1;
  // down0 is always within bounds
  down0 = lowbit == 0 ? -1 : rank-lowbit;

  down1 = lowbit == 0 ? -1 : rank+lowbit;
  // Make sure down1 is within bounds
  while (down1 >= nranks) {
    down1 = lowbit == 0 ? -1 : rank+lowbit;
    lowbit >>= 1;
  }
  *d0 = down0; *d1 = down1;

  return ncclSuccess;
}

/* Build a double binary tree. Take the previous tree for the first tree.
 * For the second tree, we use a mirror tree (if nranks is even)
 *
 * 0---------------8                   3----------------11
 *          ______/ \                 / \______
 *         4         \               /         7
 *       /   \        \             /        /   \
 *     2       6       10         1        5      9
 *    / \     / \     /  \       / \      / \    / \
 *   1   3   5   7   9   11     0   2    4   6  8   10
 *
 * or shift it by one rank (if nranks is odd).
 *
 * 0---------------8            1---------------9
 *          ______/ \______              ______/ \______
 *         4               12           5                0
 *       /   \            /           /   \            /
 *     2       6       10           3       7       11
 *    / \     / \     /  \         / \     / \     /  \
 *   1   3   5   7   9   11       2   4   6   8  10   12
 */
/* 中文精读：互补树的生成方法
 * 第一棵直接用 Btree；偶数规模把 rank 做镜像，奇数规模则循环平移，得到第二棵。
 * 把变换后的父/子编号再映射回原 rank，保证两棵树都覆盖同一批节点。
 * 这样第一棵中的叶节点可承担第二棵的中间节点工作，改善节点级负载分布。
 * 第二棵不是把第一棵的数据方向反转；每棵在 AllReduce 中仍有向上归约和向下广播。
 */
/* 中文逐项-函数 F0527：ncclGetDtree
 * 用途与位置：在第一棵二叉树基础上用镜像或平移构造互补树，分摊中间节点负载。
 * 参数 nranks（int nranks）：当前通信组参与 rank 总数；用于环步数、数组分段与全局规模。
 * 参数 rank（int rank）：参与者在当前通信组中的用户编号；与本地 CUDA 设备编号不同。
 * 参数 s0（int* s0）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 d0_0（int* d0_0）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 d0_1（int* d0_1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 parentChildType0（int* parentChildType0）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 s1（int* s1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 d1_0（int* d1_0）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 d1_1（int* d1_1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 参数 parentChildType1（int* parentChildType1）：指向声明类型对象的地址引用；其拥有关系和地址空间由赋值/分配接口决定。
 * 按同名符号检索的直接调用候选（未做重载/类型解析）：src/graph/connect.cc::connectTrees。
 * 本体直接出现的调用/包装：ncclGetBtree。
 * 需要对照的边界/推进条件：nranks % 2 == 1。各条件所属分支见紧邻函数体。
 * 源码返回表达式：return ncclSuccess;。CHECK类宏还可能提前返回/跳转。
 */
ncclResult_t ncclGetDtree(int nranks, int rank, int* s0, int* d0_0, int* d0_1, int* parentChildType0, int* s1, int* d1_0, int* d1_1, int* parentChildType1) {
  // First tree ... use a btree
  ncclGetBtree(nranks, rank, s0, d0_0, d0_1, parentChildType0);
  // Second tree ... mirror or shift
  if (nranks % 2 == 1) {
    // shift
/* 中文逐项-变量 V1646：shiftrank
 * shiftrank（int；函数 ncclGetDtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 初值/来源 shiftrank = (rank-1+nranks) % nranks；按该式计算本轮范围/数量/位置；乘除和取整顺序会影响尾块与数组边界。
 */
    int shiftrank = (rank-1+nranks) % nranks;
/* 中文逐项-变量 V1647：u、d0、d1
 * u（int；函数 ncclGetDtree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：ncclGetBtree(nranks, shiftrank, &u, &d0, &d1, parentChildType1);。若由输出参数填入，必须先检查填充调用成功再读值。
 * d0（int；函数 ncclGetDtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclGetBtree(nranks, shiftrank, &u, &d0, &d1, parentChildType1);。若由输出参数填入，必须先检查填充调用成功再读值。
 * d1（int；函数 ncclGetDtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclGetBtree(nranks, shiftrank, &u, &d0, &d1, parentChildType1);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int u, d0, d1;
    ncclGetBtree(nranks, shiftrank, &u, &d0, &d1, parentChildType1);
    *s1 = u == -1 ? -1 : (u+1) % nranks;
    *d1_0 = d0 == -1 ? -1 : (d0+1) % nranks;
    *d1_1 = d1 == -1 ? -1 : (d1+1) % nranks;
  } else {
    // mirror
/* 中文逐项-变量 V1648：u、d0、d1
 * u（int；函数 ncclGetDtree 内）：当前循环/索引/算术分量的局部值，初始化和下方条件给出实际取值范围，不代表固定全局编号。
 * 此处未显式给初值；后续首个文本使用点：ncclGetBtree(nranks, nranks-1-rank, &u, &d0, &d1, parentChildType1);。若由输出参数填入，必须先检查填充调用成功再读值。
 * d0（int；函数 ncclGetDtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclGetBtree(nranks, nranks-1-rank, &u, &d0, &d1, parentChildType1);。若由输出参数填入，必须先检查填充调用成功再读值。
 * d1（int；函数 ncclGetDtree 内）：当前声明类型的值；下面列出它的确定来源或首个使用点，避免仅凭缩写推断用途。
 * 此处未显式给初值；后续首个文本使用点：ncclGetBtree(nranks, nranks-1-rank, &u, &d0, &d1, parentChildType1);。若由输出参数填入，必须先检查填充调用成功再读值。
 */
    int u, d0, d1;
    ncclGetBtree(nranks, nranks-1-rank, &u, &d0, &d1, parentChildType1);
    *s1 = u == -1 ? -1 : nranks-1-u;
    *d1_0 = d0 == -1 ? -1 : nranks-1-d0;
    *d1_1 = d1 == -1 ? -1 : nranks-1-d1;
  }
  return ncclSuccess;
}
