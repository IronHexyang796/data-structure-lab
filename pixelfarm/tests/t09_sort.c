/* 第 9 周 · 市场行情（七种排序） */
#include "test_util.h"

/* 造 n 个作物，价格随机 */
static void make(Crop *a, int n, unsigned seed)
{
    unsigned s = seed;
    for (int i = 0; i < n; i++) {
        s = s * 1103515245u + 12345u;
        memset(&a[i], 0, sizeof(Crop));
        a[i].id = i;
        snprintf(a[i].name, sizeof a[i].name, "c%02d", i);
        a[i].price = 100 + (int)(s % 900);
        a[i].grow_days = 3 + (int)(s % 15);
        a[i].seed_cost = 50 + (int)(s % 200);
        a[i].net_profit = a[i].price * 5 - a[i].seed_cost;
    }
}

/* 检查是否按价格降序 */
static int sorted_ok(const Crop *a, int n)
{
    for (int i = 1; i < n; i++)
        if (a[i].price > a[i - 1].price) return 0;
    return 1;
}

/* 检查这 n 个元素的 id 恰好是 0..n-1 各一个（没丢、没重复、没多） */
static int same_set(const Crop *a, int n)
{
    static unsigned char seen[512];
    memset(seen, 0, sizeof seen);
    for (int i = 0; i < n; i++) {
        if (a[i].id < 0 || a[i].id >= 512) return 0;
        if (seen[a[i].id]) return 0;      /* 重复了 */
        seen[a[i].id] = 1;
    }
    for (int i = 0; i < n; i++)
        if (!seen[i]) return 0;           /* 少了 */
    return 1;
}

int main(void)
{
    t_setup();

    enum { N = 60 };
    static Crop base[N], work[N], ref[N];

    make(base, N, 7);
    memcpy(ref, base, sizeof base);

    /* 每种排序都跑一遍：结果必须正确 */
    const char *names[7] = {"冒泡", "选择", "插入", "希尔", "归并", "快排", "堆排序"};
    void (*fns[7])(Crop *, int, CmpFn) = {
        sort_bubble, sort_select, sort_insert, sort_shell,
        sort_merge, sort_quick, sort_heap
    };

    for (int k = 0; k < 7; k++) {
        memcpy(work, base, sizeof base);
        g_stat.cmp = g_stat.mov = 0;

        fns[k](work, N, cmp_price);

        t_begin(names[k] /* 用算法名当标题 */);
        CHECK(sorted_ok(work, N), "结果按价格降序");
        CHECK(same_set(work, N), "元素没丢也没多");
        if (k == 4 || k == 5) {
            /* 归并和快排应该比冒泡快很多 */
            SLOWER(g_stat.cmp < N * N / 4,
                   "比较 %ld 次（冒泡要 %d 次左右）", g_stat.cmp, N * N / 2);
        } else {
            printf("  [info] 比较 %ld 次，移动 %ld 次\n", g_stat.cmp, g_stat.mov);
        }
    }

    t_begin("1. 边界情况");
    memcpy(work, base, sizeof base);
    sort_quick(work, 0, cmp_price);
    CHECK(1, "0 个元素不崩");
    sort_quick(work, 1, cmp_price);
    CHECK(work[0].id == base[0].id, "1 个元素不变");

    memset(work, 0, sizeof work);
    for (int i = 0; i < 5; i++) {
        work[i].id = i;
        work[i].price = 500;      /* 全一样 */
        snprintf(work[i].name, sizeof work[i].name, "s%d", i);
    }
    sort_quick(work, 5, cmp_price);
    CHECK(sorted_ok(work, 5), "全部相同也不出错");

    t_begin("2. 已经有序的输入");
    memcpy(work, ref, sizeof base);
    sort_insert(work, N, cmp_price);   /* 先排一次 */
    g_stat.cmp = 0;
    sort_insert(work, N, cmp_price);   /* 再排一次，应该很快 */
    CHECK(sorted_ok(work, N), "结果仍然有序");
    SLOWER(g_stat.cmp < N * 2,
           "对已排好的数组，插入排序只比较了 %ld 次（接近 O(n)）", g_stat.cmp);

    t_begin("3. 两种比较函数");
    memcpy(work, base, sizeof base);
    sort_quick(work, N, cmp_profit);
    int profit_ok = 1;
    for (int i = 1; i < N; i++)
        if (work[i].net_profit > work[i - 1].net_profit) profit_ok = 0;
    CHECK(profit_ok, "按净收益排序也正确");

    t_begin("4. 多关键字");
    memcpy(work, base, sizeof base);
    /* 故意让几个价格一样，考验第二关键字 */
    for (int i = 0; i < 6; i++) work[i].price = 777;
    market_rank(work, N);
    int rank_ok = 1;
    for (int i = 1; i < N; i++) {
        if (work[i].price > work[i - 1].price) rank_ok = 0;
        if (work[i].price == work[i - 1].price &&
            work[i].grow_days < work[i - 1].grow_days) rank_ok = 0;
    }
    CHECK(rank_ok, "价格降序 → 同价按天数升序");

    t_begin("5. 七种算法结果必须完全一致");
    static Crop r0[N];
    memcpy(r0, base, sizeof base);
    sort_merge(r0, N, cmp_price);       /* 用归并当标准答案 */
    int all_same = 1;
    for (int k = 0; k < 7; k++) {
        memcpy(work, base, sizeof base);
        fns[k](work, N, cmp_price);
        for (int i = 0; i < N; i++)
            if (work[i].id != r0[i].id) all_same = 0;
    }
    CHECK(all_same, "七种排序排出来的顺序完全一样");

    t_begin("6. 今天该种什么");
    int best = market_best_crop(base, N, 3);
    CHECK(best >= 0 && best < N, "算出推荐作物 id = %d", best);

    return t_end("t09");
}
