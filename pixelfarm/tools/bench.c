/* ============================================================================
 *  tools/bench.c  ——  七种排序的性能对比（第 9 周）
 *
 *  编译运行：
 *      pf bench
 *
 *  它干的事：
 *      造 4 组不同规模的数据（100 / 1000 / 5000 / 20000）
 *      每种排序各跑一次，统计比较次数、移动次数、真实耗时
 *      结果写到 out/bench.csv
 *
 *  为什么还要统计"比较次数"和"移动次数"：
 *      因为游戏里的时间就是这么算的。真实耗时受电脑性能影响，
 *      但比较次数只跟算法有关 —— 换台电脑，这个数字不变。
 * ========================================================================== */
#include "engine.h"

#ifdef _WIN32
  #include <windows.h>
  static double now_ms(void)
{
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000.0 / (double)f.QuadPart;
}
#else
  #include <time.h>
  static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}
#endif

typedef void (*SortFn)(Crop *, int, CmpFn);

static const char *NAMES[7] = {
    "bubble", "select", "insert", "shell", "merge", "quick", "heap"
};
static const char *CN[7] = {
    "冒泡", "选择", "插入", "希尔", "归并", "快排", "堆排序"
};
static SortFn FNS[7] = {
    sort_bubble, sort_select, sort_insert, sort_shell,
    sort_merge, sort_quick, sort_heap
};

/* 造 n 条随机作物数据 */
static void make_data(Crop *a, int n, unsigned seed)
{
    unsigned s = seed;
    for (int i = 0; i < n; i++) {
        s = s * 1103515245u + 12345u;
        memset(&a[i], 0, sizeof(Crop));
        a[i].id = i % 1000;
        snprintf(a[i].name, sizeof a[i].name, "crop%05d", i % 1000);
        a[i].price = 100 + (int)(s % 900);
        a[i].grow_days = 3 + (int)(s % 20);
        a[i].seed_cost = 50 + (int)(s % 300);
        a[i].net_profit = a[i].price * 5 - a[i].seed_cost;
    }
}

int main(void)
{
    const int sizes[4] = {100, 1000, 5000, 20000};

#ifdef _WIN32
    system("if not exist out mkdir out");
#else
    system("mkdir -p out");
#endif

    FILE *csv = fopen("out/bench.csv", "w");
    if (!csv) { printf("写不了 out/bench.csv\n"); return 1; }
    fprintf(csv, "algo,algo_cn,n,cmp,mov,ms\n");

    printf("\n");
    printf("  七种排序性能对比\n");
    printf("  ================================================================\n");
    printf("  %-10s %8s %12s %12s %10s\n", "算法", "规模", "比较次数", "移动次数", "耗时(ms)");
    printf("  ----------------------------------------------------------------\n");

    for (int si = 0; si < 4; si++) {
        int n = sizes[si];

        Crop *base = (Crop *)malloc((size_t)n * sizeof(Crop));
        Crop *work = (Crop *)malloc((size_t)n * sizeof(Crop));
        if (!base || !work) { printf("内存不够\n"); return 1; }

        make_data(base, n, 20250101u + (unsigned)si);

        for (int k = 0; k < 7; k++) {
            /* 冒泡在 20000 那组太慢，跳过（不然要等半小时） */
            if (n >= 20000 && (k == 0 || k == 1)) {
                printf("  %-10s %8d %12s %12s %10s\n", CN[k], n, "—", "—", "跳过");
                continue;
            }

            memcpy(work, base, (size_t)n * sizeof(Crop));

            /* 统计前先清零 */
            memset(&g_farm, 0, sizeof g_farm);
            g_farm.day = 1;
            g_stat.cmp = g_stat.mov = 0;

            double t0 = now_ms();
            FNS[k](work, n, cmp_price);
            double dt = now_ms() - t0;

            printf("  %-10s %8d %12ld %12ld %10.1f\n",
                   CN[k], n, g_stat.cmp, g_stat.mov, dt);

            fprintf(csv, "%s,%s,%d,%ld,%ld,%.2f\n",
                    NAMES[k], CN[k], n, g_stat.cmp, g_stat.mov, dt);

            /* 顺手检查一下结果对不对 */
            int ok = 1;
            for (int i = 1; i < n; i++)
                if (work[i].price > work[i - 1].price) { ok = 0; break; }
            if (!ok) printf("             !! 排序结果不正确\n");
        }

        free(base);
        free(work);
        printf("  ----------------------------------------------------------------\n");
    }

    fclose(csv);

    printf("\n  结果已写入 out/bench.csv\n");
    printf("  画图：python tools/plot.py\n");
    printf("  （比较次数只跟算法有关，换台电脑这个数字不变 ——\n");
    printf("    所以游戏里的计算代价用比较次数来算，是公平的）\n\n");
    return 0;
}
