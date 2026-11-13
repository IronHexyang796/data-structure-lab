/* ============================================================================
 *  engine/render.c  ——  把农场画到终端上
 *  老师提供，禁止修改。
 *
 *  画法：每帧把整屏拼成一个字符串，一次性输出，光标回到左上角覆盖。
 *  这样不闪屏，老机房电脑也扛得住。
 * ========================================================================== */
#include "engine.h"
#include <stdarg.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <time.h>
#endif

#define SCR_W 78

static char  s_buf[16384];
static int   s_len;

/* ---------------------------------------------------------------- 拼字符串 */
static void put(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(s_buf + s_len, sizeof(s_buf) - s_len, fmt, ap);
    va_end(ap);
    if (n > 0) s_len += n;
    if (s_len > (int)sizeof(s_buf) - 2) s_len = (int)sizeof(s_buf) - 2;
}

static void putln(const char *s)
{
    put("%s\033[K\n", s);
}

/* 画一条横线 */
static void hrule(char c)
{
    put(C_DIM);
    for (int i = 0; i < SCR_W; i++) put("%c", c);
    put(C_RST "\033[K\n");
}

/* ---------------------------------------------------------------- 地块显示 */
static int growth_percent(const Plot *p)
{
    if (p->state == PLOT_RIPE)  return 100;
    if (p->state != PLOT_GROWING && p->state != PLOT_SEEDED) return 0;
    int total = p->ripe_day - p->planted_day;
    if (total <= 0) return 100;
    int done = g_farm.day - p->planted_day;
    if (done < 0) done = 0;
    int pct = done * 100 / total;
    if (pct > 100) pct = 100;
    return pct;
}

static void plot_cell(const Plot *p, char out[16], const char **col)
{
    switch (p->state) {
        case PLOT_WILD:    strcpy(out, "[  .  ]"); *col = C_DIM;  break;
        case PLOT_TILLED:  strcpy(out, "[  ,  ]"); *col = C_YEL;  break;
        case PLOT_SEEDED:  strcpy(out, "[  o  ]"); *col = C_CYN;  break;
        case PLOT_GROWING: snprintf(out, 16, "[%3d%% ]", growth_percent(p));
                           *col = C_GRN; break;
        case PLOT_RIPE:    strcpy(out, "[ *** ]"); *col = C_BGRN; break;
        case PLOT_DEAD:    strcpy(out, "[  x  ]"); *col = C_RED;  break;
        default:           strcpy(out, "[  ?  ]"); *col = C_RST;  break;
    }
}

/* ---------------------------------------------------------------- 顶部 */
static void draw_header(void)
{
    int h = g_farm.hour, m = g_farm.minute;

    put(C_BG_D C_BOLD C_BGRN);
    put(" 像素农场 ");
    put(C_RST C_BG_D C_WHT);
    put(" PIXEL FARM ");
    put(C_RST C_BG_D);
    put("  第 %d 天  %02d:%02d ", g_farm.day, h, m);
    put(C_RST "\033[K\n");

    /* 第二行：金币 / 体力 / 已耗时间 */
    put(C_BG_D);
    put(" 金币 " C_BYEL "%d" C_RST C_BG_D, g_farm.gold / 100);

    put("   体力 " C_BCYN "%d" C_RST C_BG_D "/100", g_farm.stamina);

    int cost_min = (int)(g_farm.cost_seconds / 60.0);
    if (cost_min < 60)
        put("   计算已耗 " C_BYEL "%d 分钟" C_RST C_BG_D, cost_min);
    else
        put("   计算已耗 " C_BRED "%d 时 %d 分" C_RST C_BG_D,
            cost_min / 60, cost_min % 60);

    put(C_RST "\033[K\n");

    /* 第三行：累计次数 */
    put(C_BG_D " 比较 " C_DIM "%ld" C_RST C_BG_D " 次   移动 " C_DIM "%ld"
        C_RST C_BG_D " 次   malloc " C_DIM "%ld" C_RST C_BG_D " 次",
        g_farm.cmp_count, g_farm.move_count, g_farm.malloc_count);
    put(C_RST "\033[K\n");
    put("\033[K\n");
}

/* ---------------------------------------------------------------- 地图 */
static void draw_map(void)
{
    put("       ");
    for (int x = 0; x < WORLD_W; x++) put(C_DIM "  %d    " C_RST, x);
    putln("");

    for (int y = 0; y < WORLD_H; y++) {
        put("   " C_DIM "%d" C_RST "  ", y);
        for (int x = 0; x < WORLD_W; x++) {
            const Plot *p = &g_farm.plots[y][x];
            char cell[16];
            const char *col;
            plot_cell(p, cell, &col);

            int is_cur = (x == g_ui.cx && y == g_ui.cy);
            if (is_cur) put(C_BOLD C_BYEL "%s" C_RST, cell);
            else        put("%s%s%s", col, cell, C_RST);
        }
        putln("");
    }
    putln("");
}

/* ---------------------------------------------------------------- 光标信息 */
static void draw_cursor_info(void)
{
    const Plot *p = &g_farm.plots[g_ui.cy][g_ui.cx];

    put("  光标 " C_BOLD "(%d,%d)" C_RST "  ", g_ui.cx, g_ui.cy);

    switch (p->state) {
        case PLOT_WILD:   put(C_DIM "荒地，可以开垦" C_RST); break;
        case PLOT_TILLED: put(C_YEL "已开垦，可以播种" C_RST); break;
        case PLOT_SEEDED: put(C_CYN "已播种，等待发芽" C_RST); break;
        case PLOT_GROWING: {
            const Crop *c = crop_by_id(p->crop_id);
            put(C_GRN "生长中" C_RST "  %s  " C_BGRN "%d%%" C_RST
                "  水分 %d  第 %d 天成熟",
                c ? c->name : "?", growth_percent(p), p->water, p->ripe_day);
            break;
        }
        case PLOT_RIPE: {
            const Crop *c = crop_by_id(p->crop_id);
            put(C_BGRN "成熟了！" C_RST "  %s  按 H 收获", c ? c->name : "?");
            break;
        }
        case PLOT_DEAD: put(C_RED "枯萎了。按 T 重新开垦" C_RST); break;
        default: break;
    }
    putln("");
}

static void draw_selected_crop(void)
{
    const Crop *c = crop_by_id(g_ui.sel_crop);
    if (!c) { putln("  " C_DIM "（没选作物）" C_RST); return; }

    put("  种子 " C_BOLD "%s" C_RST "  %d 天成熟  种子 " C_BYEL "%d 分" C_RST
        "  收购 " C_BGRN "%d 分/斤" C_RST "   [1-9] 换",
        c->name, c->grow_days, c->seed_cost, c->price);
    putln("");
}

/* ---------------------------------------------------------------- 底部 */
static void draw_help(void)
{
    hrule('-');
    /* 这里故意不用 C_DIM：某些终端（包括 VSCode 的浅色主题）把"暗色"渲染得几乎看不见，
     * 学生会以为"屏幕上没有操作提示"。用默认前景色加粗，什么主题都看得清。 */
    putln(C_BOLD "  [方向键] 移动    [T] 开垦    [S] 播种    [W] 浇水    [H] 收获" C_RST);
    putln(C_BOLD "  [O] 交单   [V] 出货   [N] 睡觉   [U] 撤销   [M] 行情   [Q] 退出" C_RST);
    putln(C_DIM  "  面板：[G] 温室  [K] 科技树  [R] 路网  [B] 大地图  [C] 装车台  [I] 图鉴  [?] 帮助" C_RST);

    if (g_farm.toast_timer > 0 && g_farm.toast_msg[0]) {
        put("  " C_BYEL ">> " C_BOLD "%s" C_RST "\033[K\n", g_farm.toast_msg);
    } else {
        put("\033[K\n");
    }
}

/* ---------------------------------------------------------------- 主渲染 */
void render_frame(void)
{
    s_len = 0;
    s_buf[0] = '\0';

    put("\033[H");     /* 光标回左上角，不清屏，避免闪 */
    draw_header();
    draw_map();
    draw_cursor_info();
    draw_selected_crop();
    draw_help();

    fwrite(s_buf, 1, (size_t)s_len, stdout);
    fflush(stdout);
}

/* ---------------------------------------------------------------- 详情面板 */
void ui_pause_panel(const char *title, const char *body)
{
    term_clear();
    printf(C_BOLD C_BGRN "  %s" C_RST "\n", title);
    printf(C_DIM);
    for (int i = 0; i < SCR_W; i++) putchar('=');
    printf(C_RST "\n\n");
    printf("%s", body);
    fflush(stdout);

    /* 输出被重定向到文件（不是真终端）时，不等按键，否则会卡死。
     * 正常在 VSCode 终端或 F5 黑窗口里玩，走的还是下面"等一个键"。 */
    if (pf_stdin_is_tty()) {
        printf("\n" C_DIM "  （按任意键返回）" C_RST);
        fflush(stdout);
        for (;;) {
            int k = term_key();
            if (k >= 0) break;
#ifdef _WIN32
            Sleep(20);
#else
            struct timespec ts = {0, 20000000L};
            nanosleep(&ts, NULL);
#endif
        }
    }
    term_clear();
}
