/* 第 5 周 · 科技树（二叉树） */
#include "test_util.h"

int main(void)
{
    t_setup();

    /*        1
     *       / \
     *      2   3
     *     /     \
     *    4       5
     */
    t_begin("1. 建一棵小树");
    TNode *r = node_new(1);
    r->left  = node_new(2);
    r->right = node_new(3);
    r->left->left = node_new(4);
    r->right->right = node_new(5);

    CHECK(tree_count(r) == 5, "节点数 5（实际 %d）", tree_count(r));
    CHECK(tree_height(r) == 3, "高度 3（实际 %d）", tree_height(r));
    CHECK(tree_leaves(r) == 2, "叶子 2 个：4 和 5（实际 %d）", tree_leaves(r));

    t_begin("2. 递归不要写错");
    CHECK(tree_count(NULL) == 0, "空树节点数 0");
    CHECK(tree_height(NULL) == 0, "空树高度 0");
    CHECK(tree_leaves(NULL) == 0, "空树叶子 0");

    t_begin("3. 三种遍历");
    int out[32], pos;

    pos = 0; preorder(r, out, &pos);
    CHECK(pos == 5, "前序输出 5 个");
    CHECK(out[0] == 1, "前序第一个是根 1（实际 %d）", out[0]);
    CHECK(out[1] == 2 && out[2] == 4, "前序是 1 2 4 3 5");

    pos = 0; inorder(r, out, &pos);
    CHECK(pos == 5, "中序输出 5 个");
    CHECK(out[0] == 4, "中序第一个是最左 4（实际 %d）", out[0]);
    CHECK(out[4] == 5, "中序最后是最右 5（实际 %d）", out[4]);

    pos = 0; postorder(r, out, &pos);
    CHECK(pos == 5, "后序输出 5 个");
    CHECK(out[0] == 4, "后序第一个是 4（实际 %d）", out[0]);
    CHECK(out[4] == 1, "后序最后是根 1（实际 %d）", out[4]);

    t_begin("4. 层序（要用队列）");
    pos = 0; levelorder(r, out, &pos);
    CHECK(pos == 5, "层序输出 5 个");
    CHECK(out[0] == 1, "层序第一个是根");
    CHECK(out[1] == 2 && out[2] == 3, "第二层是 2 3");

    tree_free(r);

    t_begin("5. 前序 + 中序 还原树");
    /*       A(1)
     *      /    \
     *    B(2)   C(3)
     *    /  \      \
     *  D(4) E(5)   F(6)
     * 前序：1 2 4 5 3 6   中序：4 2 5 1 3 6
     */
    int pre[6] = {1, 2, 4, 5, 3, 6};
    int in[6]  = {4, 2, 5, 1, 6, 3};
    TNode *t = tree_build(pre, in, 6);
    CHECK(t != NULL, "树建出来了");
    if (t) {
        CHECK(tree_count(t) == 6, "节点数 6（实际 %d）", tree_count(t));
        pos = 0; preorder(t, out, &pos);
        CHECK(pos == 6 && out[0] == 1 && out[1] == 2 && out[2] == 4,
              "还原后前序对得上");
        pos = 0; inorder(t, out, &pos);
        CHECK(pos == 6 && out[0] == 4 && out[2] == 5,
              "还原后中序对得上");
        tree_free(t);
    }

    t_begin("6. 二叉搜索树");
    TNode *bst = NULL;
    int keys[7] = {5, 3, 8, 1, 4, 7, 9};
    for (int i = 0; i < 7; i++) bst = bst_insert(bst, keys[i]);
    CHECK(tree_count(bst) == 7, "插了 7 个");
    CHECK(bst && bst->id == 5, "第一个插入的是根");
    CHECK(bst_find(bst, 7) != NULL, "找得到 7");
    CHECK(bst_find(bst, 6) == NULL, "找不到 6");

    pos = 0; inorder(bst, out, &pos);
    int sorted = 1;
    for (int i = 1; i < pos; i++) if (out[i] < out[i-1]) sorted = 0;
    CHECK(sorted && pos == 7, "中序遍历是有序的（这就是搜索树的定义）");

    t_begin("7. 镜像");
    tree_mirror(bst);
    CHECK(bst && bst->id == 5, "镜像后根不变");
    CHECK(bst && bst->left && bst->left->id == 8, "左子树变成了 8");
    CHECK(bst && bst->right && bst->right->id == 3, "右子树变成了 3");

    pos = 0; inorder(bst, out, &pos);
    int desc = 1;
    for (int i = 1; i < pos; i++) if (out[i] > out[i-1]) desc = 0;
    CHECK(desc, "镜像后中序变成降序");

    tree_free(bst);
    return t_end("t05");
}
