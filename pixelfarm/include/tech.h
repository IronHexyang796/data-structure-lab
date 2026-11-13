/* ============================================================================
 *  include/tech.h  ——  第 5 周：科技树（二叉树）
 *  老师提供，只读。
 * ========================================================================== */
#ifndef TECH_H
#define TECH_H
#include "farm.h"

typedef struct TNode {
    int id;                 /* 科技编号 / 作物编号 */
    int unlocked;           /* 1 = 已解锁 */
    struct TNode *left, *right;
} TNode;

TNode *node_new(int id);        /* 已实现，直接用 */
void   tree_free(TNode *t);     /* 已实现 */

/* 要在 src/tech.c 里实现的 */
int    tree_height(const TNode *t);
int    tree_count(const TNode *t);
int    tree_leaves(const TNode *t);

void   preorder(TNode *t, int *out, int *pos);
void   inorder(TNode *t, int *out, int *pos);
void   postorder(TNode *t, int *out, int *pos);
void   levelorder(TNode *t, int *out, int *pos);

TNode *tree_build(const int *pre, const int *in, int n);
TNode *bst_insert(TNode *root, int id);
TNode *bst_find(TNode *root, int id);
void   tree_mirror(TNode *t);
void   tree_print(const TNode *t, int depth);

/* 引擎用 */
TNode *tech_default_tree(void);

#endif /* TECH_H */
