/* 第 6 周 · 哈夫曼（手写堆 + 树） */
#include "test_util.h"

int main(void)
{
    t_setup();

    t_begin("1. 堆：基本操作");
    Heap h;
    heap_init(&h, 64);
    CHECK(h.size == 0, "空堆 size = 0");

    heap_push(&h, 5, 0);
    heap_push(&h, 2, 1);
    heap_push(&h, 9, 2);
    heap_push(&h, 1, 3);
    CHECK(h.size == 4, "压了 4 个");

    int n1 = heap_pop(&h);
    CHECK(n1 == 3, "最小的先出来，是节点 3（权重 1）（实际 %d）", n1);
    int n2 = heap_pop(&h);
    CHECK(n2 == 1, "第二个是节点 1（权重 2）（实际 %d）", n2);
    CHECK(h.size == 2, "还剩 2 个");

    t_begin("2. 堆：空堆弹出");
    heap_init(&h, 8);
    CHECK(heap_pop(&h) < 0, "空堆弹出返回负数");

    t_begin("3. 哈夫曼：权重 [5 2 9 1]");
    HuffTree t;
    int w[4] = {5, 2, 9, 1};
    CHECK(huffman_build(&t, w, 4) == PF_OK, "建树成功");
    CHECK(t.leaf_n == 4, "4 个叶子");
    CHECK(t.size == 7, "总节点 4 + 3 = 7（实际 %d）", t.size);

    t_begin("4. 加权路径长度");
    /* 手工算：
     *   排序：1 2 5 9
     *   合并 1+2=3 → {3,5,9}
     *   合并 3+5=8 → {8,9}
     *   合并 8+9=17
     * WPL = 17 + 8 + 3 = 28
     */
    int wpl = huffman_wpl(&t);
    CHECK(wpl == 28, "WPL = 28（实际 %d）", wpl);

    t_begin("5. 编码：高频用短码");
    char code[MAX_HUFF][32];
    huffman_code(&t, code);
    int ok = 1;
    for (int i = 0; i < t.leaf_n; i++)
        if (code[i][0] == '\0') ok = 0;
    CHECK(ok, "每个叶子都有编码");

    /* 权重最大的（9）应该拿到最短的码之一 */
    int max_i = 0;
    for (int i = 1; i < t.leaf_n; i++)
        if (w[i] > w[max_i]) max_i = i;
    CHECK(strlen(code[max_i]) <= 2,
          "权重最大的字符码长 %zu（应该 ≤ 2）", strlen(code[max_i]));

    int min_i = 0;
    for (int i = 1; i < t.leaf_n; i++)
        if (w[i] < w[min_i]) min_i = i;
    CHECK(strlen(code[min_i]) >= strlen(code[max_i]),
          "权重最小（%d）的码长 %zu 不短于最大的",
          w[min_i], strlen(code[min_i]));

    t_begin("6. 前缀性质");
    int prefix_ok = 1;
    for (int i = 0; i < t.leaf_n; i++)
        for (int j = 0; j < t.leaf_n; j++) {
            if (i == j) continue;
            if (strncmp(code[i], code[j], strlen(code[i])) == 0) prefix_ok = 0;
        }
    CHECK(prefix_ok, "没有任何一个码是别人的前缀（能唯一解码）");

    t_begin("7. 对比：按顺序装 vs 哈夫曼装");
    /* 权重相差很大时，哈夫曼应该明显更省 */
    int skew[6] = {100, 50, 20, 10, 5, 1};
    int huff_cost = batch_min_cost(skew, 6);
    int naive = 0, acc = 0;
    for (int i = 0; i < 6; i++) { acc += skew[i]; naive += acc; }
    CHECK(huff_cost <= naive,
          "哈夫曼代价 %d ≤ 顺序装 %d（省 %d）",
          huff_cost, naive, naive - huff_cost);

    SLOWER(huff_cost < naive * 85 / 100,
           "省下 %.0f%%（权重差越大省得越多）",
           (naive - huff_cost) * 100.0 / naive);

    return t_end("t06");
}
