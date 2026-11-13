/* ============================================================================
 *  include/huffman.h  ——  第 6 周：哈夫曼（手写堆 + 树）
 *  老师提供，只读。
 * ========================================================================== */
#ifndef HUFFMAN_H
#define HUFFMAN_H
#include "farm.h"

#define MAX_HUFF 512

typedef struct {
    int  weight;
    int  parent;
    int  left, right;      /* -1 表示没有 */
    char ch;               /* 对应的字符（编码时用） */
} HNode;

typedef struct {
    HNode nodes[MAX_HUFF * 2];
    int   root;
    int   size;            /* 总共几个节点（含内部节点） */
    int   leaf_n;          /* 叶子个数 */
} HuffTree;

/* 堆 */
typedef struct {
    int w[MAX_HUFF * 2];
    int n[MAX_HUFF * 2];   /* 对应的节点编号 */
    int size;
} Heap;

void heap_init(Heap *h, int cap);
void heap_push(Heap *h, int w, int node);
int  heap_pop(Heap *h);

int  huffman_build(HuffTree *t, const int *weights, int n);
int  huffman_wpl(const HuffTree *t);
void huffman_code(const HuffTree *t, char code[][32]);
int  huffman_decode(const HuffTree *t, const char *bits, int *out, int *outn);

/* 给游戏用的高层接口（内部调上面的） */
int  batch_min_cost(const int *weights, int n);

#endif /* HUFFMAN_H */
