/* ============================================================================
 *  tests/test_util.h  ——  测试用的小工具
 *  老师提供，禁止修改。
 *
 *  用法：
 *      #include "test_util.h"
 *      int main(void) {
 *          t_setup();
 *          t_begin("第 1 周 · 仓库");
 *          CHECK(b.cap == 8, "初始化容量是 8");
 *          return t_end("t01");
 *      }
 *
 *  输出规则：
 *      [ok]    通过
 *      [FAIL]  失败（最后返回非 0，pf test 会统计）
 *      [slow]  能跑通，但比合格线慢很多（不算失败，但说明还有优化空间）
 * ========================================================================== */
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#include "farm.h"
#include "field.h"
#include "growth.h"
#include "undo.h"
#include "order.h"
#include "map.h"
#include "tech.h"
#include "huffman.h"
#include "road.h"
#include "path.h"
#include "market.h"
#include "index.h"

static int t_pass = 0;
static int t_fail = 0;
static int t_slow = 0;

/* 每个测试程序开头调一次：把全局状态弄干净，造一批假作物 */
static void t_setup(void)
{
#ifdef _WIN32
    /* 控制台按 UTF-8 显示：测试里的中文提示才不会是乱码。
     * 不管你的终端当前是什么代码页，这一句都管用。 */
    SetConsoleOutputCP(65001);
#endif

    /* 关掉输出缓冲：万一哪个用例把程序跑崩了，前面已经打印的 [ok]/[FAIL]
     * 也还能留在屏幕上，不至于一片空白。 */
    setvbuf(stdout, NULL, _IONBF, 0);

    memset(&g_farm, 0, sizeof g_farm);
    g_farm.day = 1;
    g_farm.gold = 1000000;
    g_farm.stamina = 100;
    g_farm.day_seconds = 0;
    g_farm.hour = 6;
    pf_srand(12345);

    g_crop_n = 12;
    for (int i = 0; i < 12; i++) {
        memset(&g_crops[i], 0, sizeof(Crop));
        g_crops[i].id = i;
        snprintf(g_crops[i].name, sizeof g_crops[i].name, "crop%d", i);
        g_crops[i].grow_days = 2 + i;
        g_crops[i].seed_cost = 100 + i * 10;
        g_crops[i].price = 300 + i * 50;
        g_crops[i].demand = 50;
        g_crops[i].net_profit = g_crops[i].price * 5 - g_crops[i].seed_cost;
    }

    /* 地块全部重置成荒地 */
    for (int y = 0; y < WORLD_H; y++)
        for (int x = 0; x < WORLD_W; x++) {
            Plot *p = &g_farm.plots[y][x];
            p->x = x; p->y = y;
            p->crop_id = -1;
            p->state = PLOT_WILD;
            p->water = 20;
            p->planted_day = 0;
            p->ripe_day = 0;
        }

    /* 统计清零，免得上个用例的数字串到下一个 */
    g_farm.cmp_count = 0;
    g_farm.move_count = 0;
    g_farm.cost_seconds = 0;
}

static void t_begin(const char *name)
{
    printf("\n== %s ==\n", name);
}

static int t_end(const char *tag)
{
    if (t_slow)
        printf("  (%d 项偏慢，不算失败，但离合格线还有距离)\n", t_slow);
    if (t_fail == 0)
        printf("  %s: 全部通过（%d 项）\n", tag, t_pass);
    else
        printf("  %s: %d 项通过，%d 项失败\n", tag, t_pass, t_fail);
    return t_fail == 0 ? 0 : 1;
}

#define CHECK(cond, ...) do {                              \
    if (cond) { t_pass++; printf("  [ok]   "); }           \
    else      { t_fail++; printf("  [FAIL] "); }           \
    printf(__VA_ARGS__);                                   \
    printf("\n");                                          \
} while (0)

/* 只提示，不算失败：用来对比"你的实现 vs 合格线" */
#define SLOWER(cond, ...) do {                             \
    if (!(cond)) { t_slow++; printf("  [slow] "); }        \
    else         { printf("  [ok]   "); }                  \
    printf(__VA_ARGS__);                                   \
    printf("\n");                                          \
} while (0)

#endif /* TEST_UTIL_H */
