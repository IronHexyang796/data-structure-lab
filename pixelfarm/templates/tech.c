/* ============================================================================
 *  第 5 周 · 科技树（二叉树）      templates/tech.c
 *
 *  怎么用：复制到 src/tech.c。
 *
 *  文件分成两半：
 *    上半部分（node_new / tree_free / tech_default_tree）是老师给的，直接用；
 *    下半部分（12 个函数）是你要写的。
 *  之所以把 node_new 给你，是因为"怎么造一个节点"跟递归思维没关系，
 *  别把时间花在 malloc 写三遍上。
 * ========================================================================== */
#include "tech.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* ============================================================================
 *  老师给的，不用改
 * ========================================================================== */

TNode *node_new(int id)
{
    TNode *t = (TNode *)calloc(1, sizeof(TNode));
    if (!t) return NULL;
    t->id = id;
    t->unlocked = 0;
    t->left = t->right = NULL;
    return t;
}

void tree_free(TNode *t)
{
    if (!t) return;
    tree_free(t->left);
    tree_free(t->right);
    free(t);
}

/* 一棵小科技树：1 是根，往下分叉，方便你直接看效果 */
TNode *tech_default_tree(void)
{
    TNode *r = node_new(1);
    if (!r) return NULL;
    r->unlocked = 1;
    r->left  = node_new(2);
    r->right = node_new(3);
    if (r->left)  r->left->left = node_new(4);
    if (r->right) r->right->right = node_new(5);
    return r;
}

/* ============================================================================
 *  你要写的（12 个）
 *
 *  递归三句话，背下来：
 *    1) 空树怎么办？（这是出口，必须先写）
 *    2) 根节点要做什么？
 *    3) 左右子树交给递归。
 * ========================================================================== */

/* TODO —— 树高：空树 0，否则 max(左, 右) + 1 */
int tree_height(const TNode *t)
{
    (void)t;
    return 0;
}

/* TODO —— 节点总数：空树 0，否则 1 + 左 + 右 */
int tree_count(const TNode *t)
{
    (void)t;
    return 0;
}

/* TODO —— 叶子数：没有左孩子也没有右孩子才算叶子 */
int tree_leaves(const TNode *t)
{
    (void)t;
    return 0;
}

/* TODO —— 前序：根 → 左 → 右
 *   out[(*pos)++] = t->id; 然后递归。这三个遍历只有这一行的位置不同。
 */
void preorder(TNode *t, int *out, int *pos)
{
    (void)t; (void)out; (void)pos;
}

/* TODO —— 中序：左 → 根 → 右 */
void inorder(TNode *t, int *out, int *pos)
{
    (void)t; (void)out; (void)pos;
}

/* TODO —— 后序：左 → 右 → 根 */
void postorder(TNode *t, int *out, int *pos)
{
    (void)t; (void)out; (void)pos;
}

/* TODO —— 层序：一层一层往下，这个不能用递归，要用队列
 *   拿一个数组当队列：head 指向出队位置，tail 指向入队位置。
 */
void levelorder(TNode *t, int *out, int *pos)
{
    (void)t; (void)out; (void)pos;
}

/* TODO —— 前序 + 中序 还原一棵二叉树
 *
 *   pre[0] 一定是根；在中序里找到它，左边几个就是左子树，右边几个是右子树。
 *   然后递归。参数 n 是这一段的节点个数，注意下标别越界。
 */
TNode *tree_build(const int *pre, const int *in, int n)
{
    (void)pre; (void)in; (void)n;
    return NULL;
}

/* TODO —— 二叉搜索树插入：小的往左，大的往右，相等不插
 *   返回新的根（根可能变，所以必须 return）
 */
TNode *bst_insert(TNode *root, int id)
{
    (void)root; (void)id;
    return NULL;
}

/* TODO —— 二叉搜索树查找：while 循环一路往下，比当前节点小就往左 */
TNode *bst_find(TNode *root, int id)
{
    (void)root; (void)id;
    return NULL;
}

/* TODO —— 镜像：交换左右孩子，然后递归两边 */
void tree_mirror(TNode *t)
{
    (void)t;
}

/* TODO —— 打印：把树横过来打（先打右子树，再打自己，再打左子树）
 *   depth 控制缩进，每层 4 个空格
 */
void tree_print(const TNode *t, int depth)
{
    (void)t; (void)depth;
}
