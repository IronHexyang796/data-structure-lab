/* ============================================================================
 *  engine/clock.c  ——  新的一天、每日刷新
 *  老师提供，禁止修改。
 * ========================================================================== */
#include "engine.h"

/* 价格每天在基准价附近波动 ±25% */
static void refresh_prices(void)
{
    for (int i = 0; i < g_crop_n; i++) {
        int base = g_crops[i].price;
        int delta = base * 25 / 100;
        if (delta < 1) delta = 1;
        int p = base + pf_rand(delta * 2 + 1) - delta;
        if (p < 1) p = 1;
        g_crops[i].price = p;
        g_crops[i].net_profit = p * 5 - g_crops[i].seed_cost;
    }
}

/* 每天自动来 2~4 单 */
static void spawn_orders(void)
{
    OrderQueue *q = (OrderQueue *)eng_order();
    if (!q) return;

    int n = 2 + pf_rand(3);
    for (int i = 0; i < n; i++) {
        Order o;
        order_make(&o, g_farm.day * 100 + i);
        order_enqueue(q, &o);
    }
}

void clock_new_day(void)
{
    g_farm.day++;
    g_farm.day_seconds = 0;

    /* 体力恢复：睡一觉回 60 点，上限 100 */
    g_farm.stamina += 60;
    if (g_farm.stamina > 100) g_farm.stamina = 100;

    /* 作物生长 / 枯萎 */
    field_new_day();

    /* 行情波动 */
    refresh_prices();

    /* 新订单 */
    spawn_orders();

    /* 记一笔日志 */
    LogList *L = (LogList *)eng_log();
    if (L) log_append(L, g_farm.day, 0, "新的一天");

    toast("第 %d 天开始了。体力 %d/100", g_farm.day, g_farm.stamina);
}

/* 真实时间只用来让提示条倒计时，不推进游戏时间。
 * 游戏时间只会被"你的操作"和"你的算法消耗"推走。 */
void clock_tick_real(double dt_sec)
{
    (void)dt_sec;
    if (g_farm.toast_timer > 0) g_farm.toast_timer--;
}
