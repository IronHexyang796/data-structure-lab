/* ============================================================================
 *  engine/json.c  ——  把农场状态导出成 JSON（给网页用）
 *  老师提供，禁止修改。
 *
 *  为什么需要这个：浏览器不会算你的算法，它只会"显示"。
 *  所以 C 程序负责算，把结果写成 JSON，网页读走画出来。
 *  两边只通过这段文本打交道，谁都不用懂对方的语言。
 * ========================================================================== */
#include "engine.h"
#include <stdarg.h>

typedef struct {
    char *buf;
    int   cap;
    int   len;
} JBuf;

static void jb_put(JBuf *b, const char *fmt, ...)
{
    if (b->len >= b->cap - 1) return;
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(b->buf + b->len, (size_t)(b->cap - b->len), fmt, ap);
    va_end(ap);
    if (n > 0) b->len += n;
    if (b->len > b->cap - 1) b->len = b->cap - 1;
}

/* JSON 字符串转义：把 " 和 \ 加反斜杠，把换行变成 \n */
static void jb_str(JBuf *b, const char *s)
{
    jb_put(b, "\"");
    for (int i = 0; s && s[i]; i++) {
        unsigned char c = (unsigned char)s[i];
        switch (c) {
            case '"':  jb_put(b, "\\\""); break;
            case '\\': jb_put(b, "\\\\"); break;
            case '\n': jb_put(b, "\\n");  break;
            case '\r': jb_put(b, "\\r");  break;
            case '\t': jb_put(b, "\\t");  break;
            default:
                if (c < 0x20) jb_put(b, "\\u%04x", c);
                else          jb_put(b, "%c", c);
        }
    }
    jb_put(b, "\"");
}

static const char *state_name(PlotState s)
{
    switch (s) {
        case PLOT_WILD:    return "wild";
        case PLOT_TILLED:  return "tilled";
        case PLOT_SEEDED:  return "seeded";
        case PLOT_GROWING: return "growing";
        case PLOT_RIPE:    return "ripe";
        case PLOT_DEAD:    return "dead";
        default:           return "unknown";
    }
}

static int plot_percent(const Plot *p)
{
    if (p->state == PLOT_RIPE) return 100;
    if (p->state != PLOT_SEEDED && p->state != PLOT_GROWING) return 0;
    int total = p->ripe_day - p->planted_day;
    if (total <= 0) return 100;
    int done = g_farm.day - p->planted_day;
    if (done < 0) done = 0;
    int pct = done * 100 / total;
    return pct > 100 ? 100 : pct;
}

static void build(JBuf *b)
{
    jb_put(b, "{\n");
    jb_put(b, "  \"day\":%d,\"hour\":%d,\"minute\":%d,\n",
           g_farm.day, g_farm.hour, g_farm.minute);
    jb_put(b, "  \"gold\":%d,\"stamina\":%d,\n", g_farm.gold / 100, g_farm.stamina);
    jb_put(b, "  \"stats\":{\"cmp\":%ld,\"mov\":%ld,\"cost_min\":%d},\n",
           g_farm.cmp_count, g_farm.move_count,
           (int)(g_farm.cost_seconds / 60.0));

    /* 地块：36 个 */
    jb_put(b, "  \"plots\":[\n");
    for (int y = 0; y < WORLD_H; y++) {
        for (int x = 0; x < WORLD_W; x++) {
            const Plot *p = &g_farm.plots[y][x];
            const Crop *c = crop_by_id(p->crop_id);
            jb_put(b, "    {\"id\":%d,\"x\":%d,\"y\":%d,\"state\":\"%s\",\"crop\":",
                   y * WORLD_W + x, x, y, state_name(p->state));
            jb_str(b, c ? c->name : "");
            jb_put(b, ",\"growth\":%d,\"water\":%d}%s\n",
                   plot_percent(p), p->water,
                   (y == WORLD_H - 1 && x == WORLD_W - 1) ? "" : ",");
        }
    }
    jb_put(b, "  ],\n");

    /* 仓库 */
    jb_put(b, "  \"barn\":[\n");
    for (int i = 0; i < g_farm.barn.count; i++) {
        const Crop *c = crop_by_id(g_farm.barn.data[i].crop_id);
        jb_put(b, "    {\"crop\":");
        jb_str(b, c ? c->name : "?");
        jb_put(b, ",\"count\":%d}%s\n", g_farm.barn.data[i].count,
               (i == g_farm.barn.count - 1) ? "" : ",");
    }
    jb_put(b, "  ],\n");

    /* 行情 */
    jb_put(b, "  \"crops\":[\n");
    for (int i = 0; i < g_crop_n; i++) {
        jb_put(b, "    {\"id\":%d,\"name\":", g_crops[i].id);
        jb_str(b, g_crops[i].name);
        jb_put(b, ",\"price\":%d,\"days\":%d,\"seed\":%d}%s\n",
               g_crops[i].price, g_crops[i].grow_days, g_crops[i].seed_cost,
               (i == g_crop_n - 1) ? "" : ",");
    }
    jb_put(b, "  ]\n");
    jb_put(b, "}\n");
}

/* 给 server.c 用：把状态写进你给的缓冲，返回写进去多少字节 */
int build_state_buf(char *out, int cap)
{
    if (!out || cap <= 0) return 0;
    JBuf b = { out, cap, 0 };
    out[0] = '\0';
    build(&b);
    return b.len;
}

void dump_state_json(const char *path)
{
    static char buf[128 * 1024];
    int n = build_state_buf(buf, (int)sizeof buf);

    FILE *f = fopen(path, "w");
    if (!f) {
        printf(C_BRED "  写不了 %s\n" C_RST, path);
        return;
    }
    fwrite(buf, 1, (size_t)n, f);
    fclose(f);
}

void dump_state_stdout(void)
{
    static char buf[128 * 1024];
    int n = build_state_buf(buf, (int)sizeof buf);
    fwrite(buf, 1, (size_t)n, stdout);
    fflush(stdout);
}
