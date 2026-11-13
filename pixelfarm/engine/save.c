/* ============================================================================
 *  engine/save.c  ——  存档读写
 *  老师提供，禁止修改。
 *
 *  存档格式故意用最简单的 "键 值" 文本，而不是真 JSON。
 *  因为写一个 JSON 解析器要 300 行，跟数据结构没关系，会分散你的注意力。
 *  要交给网页用的那份 JSON 在 json.c 里，那个只需要"写"不需要"读"。
 * ========================================================================== */
#include "engine.h"

#define SAVE_MAGIC "PFSAVE1"

int save_game(const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f) { toast("存档失败：打不开 %s", path); return PF_PARAM; }

    fprintf(f, "%s\n", SAVE_MAGIC);
    fprintf(f, "day %d\n",         g_farm.day);
    fprintf(f, "day_seconds %d\n", g_farm.day_seconds);
    fprintf(f, "gold %d\n",        g_farm.gold);
    fprintf(f, "stamina %d\n",     g_farm.stamina);
    fprintf(f, "seed %u\n",        g_farm.seed);
    fprintf(f, "till_count %d\n",  g_farm.till_count);
    fprintf(f, "cmp %ld\n",        g_farm.cmp_count);
    fprintf(f, "mov %ld\n",        g_farm.move_count);

    /* 地块：x y crop state planted ripe water */
    fprintf(f, "plots %d\n", WORLD_W * WORLD_H);
    for (int y = 0; y < WORLD_H; y++)
        for (int x = 0; x < WORLD_W; x++) {
            const Plot *p = &g_farm.plots[y][x];
            fprintf(f, "%d %d %d %d %d %d %d\n",
                    x, y, p->crop_id, (int)p->state,
                    p->planted_day, p->ripe_day, p->water);
        }

    /* 仓库 */
    fprintf(f, "barn %d\n", g_farm.barn.count);
    for (int i = 0; i < g_farm.barn.count; i++)
        fprintf(f, "%d %d %d\n",
                g_farm.barn.data[i].crop_id,
                g_farm.barn.data[i].count,
                g_farm.barn.data[i].quality);

    fclose(f);
    return PF_OK;
}

int load_game(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return PF_NOTFOUND;

    char tag[32] = {0};
    if (fscanf(f, "%31s", tag) != 1 || strcmp(tag, SAVE_MAGIC) != 0) {
        fclose(f);
        return PF_PARAM;
    }

    int n = 0;
    while (fscanf(f, "%31s", tag) == 1) {
        if (!strcmp(tag, "day"))         { if (fscanf(f, "%d", &g_farm.day) != 1) break; }
        else if (!strcmp(tag, "day_seconds")) { if (fscanf(f, "%d", &g_farm.day_seconds) != 1) break; }
        else if (!strcmp(tag, "gold"))   { if (fscanf(f, "%d", &g_farm.gold) != 1) break; }
        else if (!strcmp(tag, "stamina")){ if (fscanf(f, "%d", &g_farm.stamina) != 1) break; }
        else if (!strcmp(tag, "seed"))   { if (fscanf(f, "%u", &g_farm.seed) != 1) break; }
        else if (!strcmp(tag, "till_count")) { if (fscanf(f, "%d", &g_farm.till_count) != 1) break; }
        else if (!strcmp(tag, "cmp"))    { if (fscanf(f, "%ld", &g_farm.cmp_count) != 1) break; }
        else if (!strcmp(tag, "mov"))    { if (fscanf(f, "%ld", &g_farm.move_count) != 1) break; }
        else if (!strcmp(tag, "plots")) {
            int cnt = 0;
            if (fscanf(f, "%d", &cnt) != 1) break;
            for (int i = 0; i < cnt; i++) {
                int x, y, cid, st, pd, rd, wt;
                if (fscanf(f, "%d %d %d %d %d %d %d",
                           &x, &y, &cid, &st, &pd, &rd, &wt) != 7) break;
                if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) continue;
                Plot *p = &g_farm.plots[y][x];
                p->x = x; p->y = y;
                p->crop_id = cid;
                p->state = (PlotState)st;
                p->planted_day = pd;
                p->ripe_day = rd;
                p->water = wt;
            }
        }
        else if (!strcmp(tag, "barn")) {
            int cnt = 0;
            if (fscanf(f, "%d", &cnt) != 1) break;
            for (int i = 0; i < cnt; i++) {
                int cid, q, ql;
                if (fscanf(f, "%d %d %d", &cid, &q, &ql) != 3) break;
                barn_add(&g_farm.barn, cid, q);
            }
        }
        else {
            /* 不认识的键：跳过一整行 */
            int ch;
            while ((ch = fgetc(f)) != '\n' && ch != EOF) { }
        }
        n++;
    }

    fclose(f);
    return n > 0 ? PF_OK : PF_PARAM;
}
