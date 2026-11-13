/* ============================================================================
 *  第 10 周 · 图鉴检索（哈希表 + 二分查找）      templates/index.c
 *
 *  怎么用：复制到 src/index.c。
 *
 *  两条路：
 *    热数据（常查的 500 条）  -> 哈希表，O(1)
 *    冷数据（其余 4500 条）   -> 排好序的数组 + 二分，O(log n)
 *  index_lookup 先查哈希，没有再二分 —— 这就是"分层"。
 * ========================================================================== */
#include "index.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* ============================================================================
 *  一、哈希表：链地址法（每个桶挂一条链表）
 *
 *  桶下标怎么算？教科书上随便一个都行，比如：
 *      unsigned k = (unsigned)key * 2654435761u;   // 乘个质数打散
 *      b = k % cap;
 * ========================================================================== */

/* TODO —— 初始化：借 cap 个"链表头指针"的内存，用 calloc 一次清零
 *   返回 PF_OK / PF_PARAM / PF_NOMEM
 */
int hash_init(HashMap *h, int cap)
{
    memset(h, 0, sizeof(HashMap));     /* 先把结构体清零；实现完就删掉这行 */
    (void)cap;
    return PF_PARAM;
}

/* TODO —— 销毁：每条链上的节点全 free，再 free 桶数组 */
void hash_destroy(HashMap *h)
{
    (void)h;
}

/* TODO —— 插入/更新：桶里已有的 key 就直接改值，没有才新建节点头插 */
int hash_put(HashMap *h, int key, const Item *val)
{
    (void)h; (void)key; (void)val;
    return PF_PARAM;
}

/* TODO —— 查找：算出桶，顺着链找，找到返回 PF_OK，没有返回 PF_NOTFOUND */
int hash_get(const HashMap *h, int key, Item *out)
{
    (void)h; (void)key; (void)out;
    return PF_PARAM;
}

/* TODO —— 删除：单链表删除节点，注意"删的是链头"要单独处理 */
int hash_remove(HashMap *h, int key)
{
    (void)h; (void)key;
    return PF_PARAM;
}

/* ============================================================================
 *  二、二分查找
 * ========================================================================== */

/* TODO —— 在按 crop_id 升序排好的数组里找 key
 *   返回下标；找不到返回 -1。
 *   边界最容易写错：while (lo <= hi) 还是 (lo < hi)？先想清楚再写。
 */
int bsearch_by_id(const Item *sorted, int n, int key)
{
    (void)sorted; (void)n; (void)key;
    return -1;
}

/* ============================================================================
 *  三、把两者拼起来
 * ========================================================================== */

/* TODO —— 建索引
 *
 *   1) 前 hot_n 条进哈希表（hot）
 *   2) 剩下的 (n - hot_n) 条拷进 cold 数组，并按 crop_id 升序排好
 *      （不排好，二分就没意义）
 *   3) 统计量 hit_hot / hit_cold / miss 清零
 */
int index_build(Index *ix, const Item *all, int n, int hot_n)
{
    (void)ix; (void)all; (void)n; (void)hot_n;
    return PF_PARAM;
}

/* TODO —— 查一条：先哈希，再二分，都没有就记一次 miss */
int index_lookup(Index *ix, int key, Item *out)
{
    (void)ix; (void)key; (void)out;
    return PF_PARAM;
}

/* TODO —— 销毁：哈希表 + cold 数组都要还回去 */
void index_destroy(Index *ix)
{
    (void)ix;
}

/* ----------------------------------------------------------------------------
 *  下面是老师给的，不用改：造一批假数据用来测
 * -------------------------------------------------------------------------- */
void index_demo_data(Item *out, int n)
{
    if (!out) return;
    for (int i = 0; i < n; i++) {
        out[i].crop_id = i;
        out[i].count = (i * 37) % 200;
        out[i].quality = 1 + (i % 5);
    }
}
