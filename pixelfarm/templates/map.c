/* ============================================================================
 *  第 4 周 · 大地图（稀疏矩阵 / 三元组）      templates/map.c
 *
 *  怎么用：复制到 src/map.c。
 *
 *  100×100 = 10000 个格子，实际有内容的只有三百来个。
 *  所以不存 10000 个 Tile，只存"有内容的那几个"： (行, 列, 值) —— 这就是三元组。
 *  data 里的三元组按 (row, col) 升序排，后面查找、转置都靠这个顺序。
 * ========================================================================== */
#include "map.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* TODO —— 初始化：这次不分配 data，等真要存东西时再按需分配
 *   rows / cols 记下来，count = cap = 0，data = NULL
 */
int sm_init(SparseMat *m, int rows, int cols)
{
    memset(m, 0, sizeof(SparseMat));   /* 先把结构体清零；实现完就删掉这行 */
    (void)rows; (void)cols;
    return PF_PARAM;
}

/* TODO —— 销毁：free(data) 并清零 */
void sm_destroy(SparseMat *m)
{
    (void)m;
}

/* TODO —— 从稠密矩阵压成三元组
 *
 *   dense 是 rows*cols 的一维数组，dense[r*cols+c] 就是第 r 行第 c 列。
 *   先数一遍有多少个非零（type != TILE_EMPTY），再按这个数量 malloc，
 *   然后扫第二遍填进去 —— 这样一点内存都不浪费。
 */
int sm_from_dense(SparseMat *m, const Tile *dense, int rows, int cols)
{
    (void)m; (void)dense; (void)rows; (void)cols;
    return PF_PARAM;
}

/* TODO —— 慢转置：按列扫，O(cols × count)
 *   外层 for c in 0..cols-1，内层扫一遍所有三元组，把 col == c 的挑出来。
 *   顺便体会一下它为什么慢。
 */
int sm_transpose_slow(const SparseMat *src, SparseMat *dst)
{
    (void)src; (void)dst;
    return PF_PARAM;
}

/* TODO —— 快转置：先数每列有几个，算出每列第一个元素应该放在哪，
 *   再扫一遍直接放到位。O(count)。这就是"用空间换时间"。
 */
int sm_transpose_fast(const SparseMat *src, SparseMat *dst)
{
    (void)src; (void)dst;
    return PF_PARAM;
}

/* TODO —— 两个稀疏矩阵相加：像归并排序那样，两个指针一起往前走
 *   行号小的先输出；行号相同的比列号；行列都相同的才相加。
 *   相加后 type 为 TILE_EMPTY 的格子不要存（那就不稀疏了）。
 */
int sm_add(const SparseMat *a, const SparseMat *b, SparseMat *out)
{
    (void)a; (void)b; (void)out;
    return PF_PARAM;
}

/* TODO —— 取 (r,c) 的值：按顺序扫，找到就返回，找不到说明是空格子
 *   记得每比较一次调 cost_compare(1)
 */
int sm_get(const SparseMat *m, int r, int c, Tile *out)
{
    (void)m; (void)r; (void)c; (void)out;
    return PF_PARAM;
}

/* ----------------------------------------------------------------------------
 *  下面是老师给的，不用改：按种子生成一张 100×100 的地图
 * -------------------------------------------------------------------------- */
void map_generate(SparseMat *m, unsigned seed)
{
    static Tile s_dense[128][128];
    if (!m) return;
    unsigned st = seed ? seed : 1u;

    /* 先把 m 初始化干净：外面可能传进来一个没初始化的结构体，
     * 里面 sm_from_dense 会 free(m->data)，野指针会直接把堆写坏。 */
    sm_init(m, 100, 100);

    for (int r = 0; r < 128; r++)
        for (int c = 0; c < 128; c++) {
            s_dense[r][c].type = TILE_EMPTY;
            s_dense[r][c].crop_id = -1;
        }

    for (int r = 0; r < 100; r++) {
        for (int c = 0; c < 100; c++) {
            st ^= st << 13; st ^= st >> 17; st ^= st << 5;
            unsigned v = st % 1000;    /* 0~999，只有 3% 的格子有内容 */
            int t = TILE_EMPTY;
            if (v < 8)       t = TILE_WATER;   /* 0.8% */
            else if (v < 14) t = TILE_ROCK;    /* 0.6% */
            else if (v < 20) t = TILE_GRASS;   /* 0.6% */
            else if (v < 30) t = TILE_ROAD;    /* 1.0% */
            s_dense[r][c].type = t;
            s_dense[r][c].crop_id = -1;
        }
    }
    sm_from_dense(m, &s_dense[0][0], 100, 100);
}
