/* ============================================================================
 *  engine/util.c  ——  全局变量、时间折算、随机数、提示
 *  老师提供，禁止修改。
 *
 *  这个文件里的 cost_compare / cost_move 是整门课的核心机制：
 *  你在代码里数的比较和移动次数，会真的变成游戏里流逝的时间。
 * ========================================================================== */
#include "engine.h"
#include <stdarg.h>
#include <ctype.h>

/* ---------------------------------------------------------------- 全局 */
Farm g_farm;
Crop g_crops[MAX_CROPS];
int  g_crop_n = 0;
UIState g_ui;

/* ---------------------------------------------------------------- 随机数（可复现） */
void pf_srand(unsigned s)
{
    g_farm.seed = s ? s : 1u;
}

static unsigned xorshift(unsigned *st)
{
    unsigned x = *st;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *st = x;
    return x;
}

int pf_rand(int n)
{
    if (n <= 0) return 0;
    return (int)(xorshift(&g_farm.seed) % (unsigned)n);
}

/* ---------------------------------------------------------------- 提示 */
void toast(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_farm.toast_msg, sizeof g_farm.toast_msg, fmt, ap);
    va_end(ap);
    g_farm.toast_timer = 40;      /* 大约 2 秒（每帧 50ms） */
}

/* ---------------------------------------------------------------- 作物 */
const Crop *crop_by_id(int id)
{
    for (int i = 0; i < g_crop_n; i++)
        if (g_crops[i].id == id) return &g_crops[i];
    return NULL;
}

void safe_name(char *dst, const char *src, int max)
{
    int j = 0;
    for (int i = 0; src[i] && j < max - 1; i++) {
        char c = src[i];
        if (c == ',' || c == '"' || c == '\n' || c == '\r') c = '_';
        dst[j++] = c;
    }
    dst[j] = '\0';
}

/* ---------------------------------------------------------------- 时间 */
int game_minutes(void)
{
    return g_farm.day_seconds / 60;
}

/* 把 day_seconds 换算成 hour / minute 供显示 */
static void sync_clock(void)
{
    int total = 6 * 3600 + g_farm.day_seconds;   /* 从早上 6 点算 */
    int h = (total / 3600) % 24;
    int m = (total % 3600) / 60;
    g_farm.hour = h;
    g_farm.minute = m;
}

/* 游戏时间前进 sec 秒 */
void advance_time(int sec)
{
    if (sec <= 0) return;
    g_farm.day_seconds += sec;

    /* 一天最多 24 小时（86400 秒）。超了就自动翻页 */
    while (g_farm.day_seconds >= 86400) {
        g_farm.day_seconds -= 86400;
        clock_new_day();
    }
    sync_clock();
}

void cost_compare(int n)
{
    if (n <= 0) return;
    g_farm.cmp_count += n;
    double sec = n * SEC_PER_COMPARE;
    g_farm.cost_seconds += sec;
    advance_time((int)(sec + 0.5));
}

void cost_move(int n)
{
    if (n <= 0) return;
    g_farm.move_count += n;
    double sec = n * SEC_PER_MOVE;
    g_farm.cost_seconds += sec;
    advance_time((int)(sec + 0.5));
}

void cost_malloc(int n)
{
    if (n <= 0) return;
    g_farm.malloc_count += n;
    double sec = n * SEC_PER_MALLOC;
    g_farm.cost_seconds += sec;
    advance_time((int)(sec + 0.5));
}

/* ---------------------------------------------------------------- 日志动作名 */
const char *action_name(int action)
{
    switch (action) {
        case ACT_TILL:    return "开垦";
        case ACT_SOW:     return "播种";
        case ACT_WATER:   return "浇水";
        case ACT_HARVEST: return "收获";
        case ACT_SELL:    return "卖出";
        default:          return "其它";
    }
}
