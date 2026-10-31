/*************************************************************************
 * Copyright (c) 2016-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * See LICENSE.txt for license information
 ************************************************************************/

#include "nccl.h"

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
ncclResult_t ncclGetBtree(int nranks, int rank, int* u, int* d0, int* d1, int* parentChildType) {
  int up, down0, down1;
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
ncclResult_t ncclGetDtree(int nranks, int rank, int* s0, int* d0_0, int* d0_1, int* parentChildType0, int* s1, int* d1_0, int* d1_1, int* parentChildType1) {
  // First tree ... use a btree
  ncclGetBtree(nranks, rank, s0, d0_0, d0_1, parentChildType0);
  // Second tree ... mirror or shift
  if (nranks % 2 == 1) {
    // shift
    int shiftrank = (rank-1+nranks) % nranks;
    int u, d0, d1;
    ncclGetBtree(nranks, shiftrank, &u, &d0, &d1, parentChildType1);
    *s1 = u == -1 ? -1 : (u+1) % nranks;
    *d1_0 = d0 == -1 ? -1 : (d0+1) % nranks;
    *d1_1 = d1 == -1 ? -1 : (d1+1) % nranks;
  } else {
    // mirror
    int u, d0, d1;
    ncclGetBtree(nranks, nranks-1-rank, &u, &d0, &d1, parentChildType1);
    *s1 = u == -1 ? -1 : nranks-1-u;
    *d1_0 = d0 == -1 ? -1 : nranks-1-d0;
    *d1_1 = d1 == -1 ? -1 : nranks-1-d1;
  }
  return ncclSuccess;
}
