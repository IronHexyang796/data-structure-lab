/* ============================================================================
 *  第 6 周 · 装车台（哈夫曼树 + 小根堆）      templates/huffman.c
 *
 *  怎么用：复制到 src/huffman.c。
 *
 *  两部分：
 *    1. 堆：heap_init / heap_push / heap_pop —— 手写小根堆
 *    2. 树：huffman_build / wpl / code / decode
 *
 *  堆的存储很朴素：就是一个数组，w[i] 是权重，n[i] 是它对应的节点编号。
 *  下标从 0 开始的话：节点 i 的孩子是 2i+1 和 2i+2，父亲是 (i-1)/2。
 * ========================================================================== */
#include "huffman.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* ============================================================================
 *  一、小根堆
 * ========================================================================== */

/* TODO —— 初始化：size = 0 就够了，数组是结构体里自带的 */
void heap_init(Heap *h, int cap)
{
    (void)h; (void)cap;
}

/* TODO —— 入堆（上浮）
 *
 *   放到末尾，然后和父亲比：比父亲小就交换，一路往上，直到不比他小。
 *   注意：交换的是 w 和 n 两个数组的同一位置。
 */
void heap_push(Heap *h, int w, int node)
{
    (void)h; (void)w; (void)node;
}

/* TODO —— 出堆（下沉）
 *
 *   堆顶就是最小的。把最后一个元素搬到堆顶，size--，然后一路往下沉：
 *   和两个孩子里较小的那个比，比它大就交换。
 *   返回被弹出的那个"节点编号"（注意返回的是 n[]，不是 w[]）。
 */
int heap_pop(Heap *h)
{
    (void)h;
    return -1;
}

/* ============================================================================
 *  二、哈夫曼树
 * ========================================================================== */

/* TODO —— 建树
 *
 *   1) 前 n 个节点是叶子，权重是 weights[i]，parent/left/right 都设 -1
 *   2) 把 n 个叶子全部入堆
 *   3) 循环 n-1 次：弹出两个最小的 a、b，合并成新节点 size，
 *      nodes[size].weight = a+b，left = a，right = b，
 *      再把 a、b 的 parent 都指向 size，最后把 size 入堆
 *   4) 循环结束时堆里剩的那一个就是根
 *
 *   下标要小心：新节点从 n 开始往后放，最多到 2n-2。
 */
int huffman_build(HuffTree *t, const int *weights, int n)
{
    (void)t; (void)weights; (void)n;
    return PF_PARAM;
}

/* TODO —— 带权路径长度 WPL = Σ(叶子权重 × 叶子深度)
 *   深度怎么算？从叶子往上数 parent，数到 -1 为止。
 */
int huffman_wpl(const HuffTree *t)
{
    (void)t;
    return 0;
}

/* TODO —— 生成编码：从叶子往上走，是左孩子记 '0'，右孩子记 '1'，
 *   走到根之后把串倒过来。
 *   code[i] 是第 i 个叶子的编码，长度不超过 31。
 */
void huffman_code(const HuffTree *t, char code[][32])
{
    (void)t; (void)code;
}

/* TODO —— 解码：从根出发，读到 '0' 往左、'1' 往右，
 *   走到叶子就输出这个叶子，然后回到根继续。
 *   返回解出来几个字符。
 */
int huffman_decode(const HuffTree *t, const char *bits, int *out, int *outn)
{
    (void)t; (void)bits; (void)out; (void)outn;
    return PF_PARAM;
}

/* ----------------------------------------------------------------------------
 *  下面是老师给的，不用改：装车台算总代价（内部调上面的建树）
 * -------------------------------------------------------------------------- */
int batch_min_cost(const int *weights, int n)
{
    if (!weights || n <= 0) return 0;
    HuffTree t;
    if (huffman_build(&t, weights, n) != PF_OK) return 0;
    return huffman_wpl(&t);
}
