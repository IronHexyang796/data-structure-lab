/* ============================================================================
 *  engine/loop.c  ——  初始化、玩家动作、主循环
 *  老师提供，禁止修改。
 * ========================================================================== */
#include "engine.h"

#ifdef _WIN32
  #include <windows.h>
#else
  #include <time.h>
#endif

/* 各模块的单例现在住在 engine/state.c 里。
 * 这里做成宏，是为了让下面的代码还能用 s_xxx 这种短名字，读起来清爽些。 */
#define s_growth  (*(GrowthList  *)eng_growth())
#define s_log     (*(LogList     *)eng_log())
#define s_undo    (*(UndoStack   *)eng_undo())
#define s_order   (*(OrderQueue  *)eng_order())
#define s_map     (*(SparseMat   *)eng_map())
#define s_huff    (*(HuffTree    *)eng_huff())
#define s_graph   (*(Graph       *)eng_graph())
#define s_index   (*(Index       *)eng_index())

/* ---------------------------------------------------------------- 读作物表 */
static int load_crops(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        printf(C_BRED "  打不开 %s —— 你现在是在 pixelfarm/ 目录下运行吗？\n" C_RST, path);
        return PF_NOTFOUND;
    }

    char line[512];
    if (!fgets(line, sizeof line, f)) { fclose(f); return PF_PARAM; }

    g_crop_n = 0;
    while (fgets(line, sizeof line, f) && g_crop_n < MAX_CROPS) {
        int id, days, seedc, price, demand;
        char name[20] = {0};
        /* 格式：id,name,grow_days,seed_cost,price,demand */
        if (sscanf(line, "%d,%19[^,],%d,%d,%d,%d",
                   &id, name, &days, &seedc, &price, &demand) == 6) {
            Crop *c = &g_crops[g_crop_n];
            c->id = id;
            snprintf(c->name, sizeof c->name, "%s", name);
            c->grow_days = days;
            c->seed_cost = seedc;
            c->price = price;
            c->demand = demand;
            c->net_profit = price * 5 - seedc;
            g_crop_n++;
        }
    }
    fclose(f);
    return g_crop_n > 0 ? PF_OK : PF_PARAM;
}

/* ---------------------------------------------------------------- 初始化 */
static void world_init(unsigned seed)
{
    memset(&g_farm, 0, sizeof g_farm);

    g_farm.day = 1;
    g_farm.day_seconds = 0;
    g_farm.hour = 6;              /* 早上 6 点开局 */
    g_farm.minute = 0;
    g_farm.gold = 50000;          /* 500 元启动资金 */
    g_farm.stamina = 100;
    pf_srand(seed);

    for (int y = 0; y < WORLD_H; y++)
        for (int x = 0; x < WORLD_W; x++) {
            Plot *p = &g_farm.plots[y][x];
            p->x = x;
            p->y = y;
            p->crop_id = -1;
            p->state = PLOT_WILD;
            p->water = 20;
        }

    barn_init(&g_farm.barn, BARN_CAP_INIT);

    if (load_crops("data/crops.csv") != PF_OK) {
        printf(C_BYEL "  作物表读不到，用一份内置的应急数据先顶着。\n" C_RST);
        const char *fallback[5][2] = {
            {"小麦","4"},{"玉米","7"},{"胡萝卜","3"},{"番茄","6"},{"南瓜","12"}
        };
        g_crop_n = 5;
        for (int i = 0; i < 5; i++) {
            g_crops[i].id = i;
            snprintf(g_crops[i].name, sizeof g_crops[i].name, "%s", fallback[i][0]);
            g_crops[i].grow_days = atoi(fallback[i][1]);
            g_crops[i].seed_cost = 100 + i * 50;
            g_crops[i].price = 250 + i * 80;
            g_crops[i].demand = 50;
            g_crops[i].net_profit = g_crops[i].price * 5 - g_crops[i].seed_cost;
        }
    }

    growth_init(&s_growth);
    log_init(&s_log);
    undo_init(&s_undo, 64);
    order_init(&s_order, 32);
    sm_init(&s_map, 100, 100);
    map_generate(&s_map, seed ? seed : 1u);
    graph_load_default(&s_graph);
    memset(&s_huff, 0, sizeof s_huff);
    memset(&s_index, 0, sizeof s_index);

    g_ui.cx = 0;
    g_ui.cy = 0;
    g_ui.sel_crop = 0;
    g_ui.running = 1;

    /* 开局送几单 */
    for (int i = 0; i < 3; i++) {
        Order o;
        order_make(&o, 1);
        order_enqueue(&s_order, &o);
    }

    toast("欢迎。按 ? 看帮助，先去把几块地开垦出来（T）。");
}

int game_boot(unsigned seed)
{
    world_init(seed);
    return PF_OK;
}

/* ---------------------------------------------------------------- 撤销记录 */
static void push_undo(int action, int x, int y, int crop_id, int count, int gold)
{
    UndoRec r;
    r.action = action;
    r.x = x; r.y = y;
    r.crop_id = crop_id;
    r.count = count;
    r.gold = gold;
    r.day = g_farm.day;
    undo_push(&s_undo, &r);
}

static void push_log(int action, const char *detail)
{
    log_append(&s_log, g_farm.day, action, detail);
}

/* ---------------------------------------------------------------- 玩家动作 */

int ui_do_till(void)
{
    int gold0 = g_farm.gold;
    int rc = field_till(g_ui.cx, g_ui.cy);

    if (rc == PF_OK) {
        push_undo(ACT_TILL, g_ui.cx, g_ui.cy, -1, 1, g_farm.gold - gold0);
        char buf[48];
        snprintf(buf, sizeof buf, "开垦 (%d,%d)", g_ui.cx, g_ui.cy);
        push_log(ACT_TILL, buf);
        toast("开垦好了。按 S 播种");
    } else if (rc == PF_EMPTY) {
        toast("体力不够了，按 N 睡觉");
    } else {
        toast("这块地不能开垦");
    }
    return rc;
}

int ui_do_sow(void)
{
    int gold0 = g_farm.gold;
    int rc = field_sow(g_ui.cx, g_ui.cy, g_ui.sel_crop);

    if (rc == PF_OK) {
        Plot *p = plot_at(g_ui.cx, g_ui.cy);

        push_undo(ACT_SOW, g_ui.cx, g_ui.cy, g_ui.sel_crop, 1, g_farm.gold - gold0);

        /* 把这块地排进"温室队列"（第 2 周那个链表），
         * 成熟日就在 Plot 里，直接拿。按 G 能看到队列里有什么。 */
        if (p) growth_insert(&s_growth, g_ui.sel_crop, p->ripe_day, g_ui.cx, g_ui.cy);

        const Crop *c = crop_by_id(g_ui.sel_crop);
        char buf[64];
        snprintf(buf, sizeof buf, "播种 %s", c ? c->name : "?");
        push_log(ACT_SOW, buf);
        toast("种下了。记得按 W 浇水，不然会干死");
    } else if (rc == PF_EMPTY) {
        toast("金币不够买种子");
    } else {
        toast("要先开垦（T），才能播种");
    }
    return rc;
}

int ui_do_water(void)
{
    int rc = field_water(g_ui.cx, g_ui.cy);
    if (rc == PF_OK) {
        push_undo(ACT_WATER, g_ui.cx, g_ui.cy, -1, 1, 0);
        toast("浇透了");
    } else {
        toast("这块地不用浇水");
    }
    return rc;
}

int ui_do_harvest(void)
{
    Plot *p = plot_at(g_ui.cx, g_ui.cy);
    int crop_id = p ? p->crop_id : -1;      /* 收完之后 crop_id 会变成 -1，先记下来 */
    int n = field_harvest(g_ui.cx, g_ui.cy);

    if (n > 0) {
        /* 收完了，从温室队列里摘掉这一条 */
        if (crop_id >= 0) growth_remove(&s_growth, crop_id);
        push_undo(ACT_HARVEST, g_ui.cx, g_ui.cy, -1, n, 0);
        push_log(ACT_HARVEST, "收获");
        toast("收了 %d 斤，进仓库了", n);
    } else if (n == PF_FULL) {
        toast("仓库满了！先去卖点东西（V 键出货）");
    } else {
        toast("还没熟，或者这块地没种东西");
    }
    return n > 0 ? PF_OK : PF_PARAM;
}

int ui_do_undo(void)
{
    UndoRec r;
    if (undo_top(&s_undo, &r) != PF_OK) { toast("没有可以撤销的操作"); return PF_EMPTY; }

    char what[32];
    snprintf(what, sizeof what, "%s", action_name(r.action));
    int rc = undo_apply(&s_undo);
    if (rc == PF_OK) toast("撤销了：%s", what);
    return rc;
}

int ui_do_order(void)
{
    Order o;
    if (order_peek(&s_order, &o) != PF_OK) { toast("现在没有订单"); return PF_EMPTY; }

    const Crop *c = crop_by_id(o.crop_id);
    if (!c) { toast("这条订单的作物不存在"); return PF_PARAM; }

    int pos = barn_find(&g_farm.barn, o.crop_id);
    if (pos < 0) {
        toast("仓库里没有 %s，先种点", c->name);
        return PF_EMPTY;
    }
    if (g_farm.barn.data[pos].count < o.count) {
        toast("%s 只有 %d 斤，订单要 %d 斤",
              c->name, g_farm.barn.data[pos].count, o.count);
        return PF_EMPTY;
    }

    Item got;
    barn_remove_at(&g_farm.barn, pos, &got);
    if (got.count > o.count) {
        barn_add(&g_farm.barn, got.crop_id, got.count - o.count);
    }

    g_farm.gold += o.price;
    order_dequeue(&s_order, NULL);
    push_undo(ACT_SELL, -1, -1, o.crop_id, o.count, o.price);
    push_log(ACT_SELL, "完成订单");

    advance_time(60);
    toast("交付成功，+%d.%02d 元", o.price / 100, o.price % 100);
    return PF_OK;
}

/* 把仓库里的东西按今日市价卖掉。
 * 订单是"高价收"，市价是"保底卖" —— 急着用钱就先卖。 */
int ui_do_sell(void)
{
    if (g_farm.barn.count == 0) { toast("仓库是空的"); return PF_EMPTY; }

    int total = 0, n = 0;
    while (g_farm.barn.count > 0) {
        Item it;
        if (barn_remove_at(&g_farm.barn, 0, &it) != PF_OK) break;
        const Crop *c = crop_by_id(it.crop_id);
        int unit = c ? c->price : 100;
        total += unit * it.count;
        n += it.count;
    }
    g_farm.gold += total;
    push_log(ACT_SELL, "出货");
    advance_time(60);
    toast("卖了 %d 斤，+%d.%02d 元", n, total / 100, total % 100);
    return PF_OK;
}

int ui_do_next_day(void)
{
    clock_new_day();
    save_game("save/farm.sav");
    return PF_OK;
}

/* ---------------------------------------------------------------- 面板类 */
void ui_do_market(void)
{
    static char body[4096];
    int n = 0, i;

    Crop tmp[MAX_CROPS];
    for (i = 0; i < g_crop_n; i++) tmp[i] = g_crops[i];

    g_stat.cmp = g_stat.mov = 0;
    market_rank(tmp, g_crop_n);      /* 这一行会调用你的排序 */

    n += snprintf(body + n, sizeof(body) - n,
                  "  今日行情：按净收益从高到低（一共 %d 种）\n\n", g_crop_n);

    for (i = 0; i < g_crop_n && i < 20 && n < 3300; i++) {
        int profit = tmp[i].price * 5 - tmp[i].seed_cost;
        n += snprintf(body + n, sizeof(body) - n,
                      "  %2d. %s   收购 %d 分/斤   %d 天成熟   种子 %d 分   净赚 %d 分\n",
                      i + 1, tmp[i].name, tmp[i].price,
                      tmp[i].grow_days, tmp[i].seed_cost, profit);
    }
    if (g_crop_n > 20)
        n += snprintf(body + n, sizeof(body) - n, "  ……（还有 %d 种，屏幕放不下）\n", g_crop_n - 20);

    n += snprintf(body + n, sizeof(body) - n,
                  "\n  这次排序的代价：比较 %ld 次，移动 %ld 次\n"
                  "  折算成游戏时间是 %.1f 分钟 —— 这就是排序慢的代价\n",
                  g_stat.cmp, g_stat.mov,
                  (g_stat.cmp * SEC_PER_COMPARE + g_stat.mov * SEC_PER_MOVE) / 60.0);

    ui_pause_panel("市场行情", body);
}

void ui_do_growth_list(void)
{
    static char body[4096];
    static LNode *keep[5];
    int n = 0;

    GNode *p;
    LNode *q;
    int cnt = 0, keepn = 0;

    /* 这一屏显示的就是第 2 周那个链表里的东西（不是去扫描地块） */
    n += snprintf(body + n, sizeof(body) - n,
                  "  温室队列：%d 个（存在链表里）\n\n", s_growth.size);

    for (p = s_growth.head ? s_growth.head->next : NULL; p; p = p->next) {
        const Crop *c = crop_by_id(p->crop_id);
        n += snprintf(body + n, sizeof(body) - n,
                      "  (%d,%d)  %s  第 %d 天成熟\n",
                      p->x, p->y, c ? c->name : "?", p->ripe_day);
        cnt++;
        if (n > 3300) {
            n += snprintf(body + n, sizeof(body) - n, "  ……（还有，屏幕放不下了）\n");
            break;
        }
    }
    if (cnt == 0)
        n += snprintf(body + n, sizeof(body) - n, "  （队列是空的：去种点东西，再回来看）\n");

    /* 日志：链表是尾插的，走一遍记住最后 5 条 */
    for (q = s_log.head ? s_log.head->next : NULL; q; q = q->next) {
        keep[keepn % 5] = q;
        keepn++;
    }
    int shown = keepn < 5 ? keepn : 5;
    n += snprintf(body + n, sizeof(body) - n,
                  "\n  操作日志：共 %d 条，最近 %d 条\n", s_log.size, shown);
    for (int i = 0; i < shown; i++) {
        LNode *e = keep[(keepn - shown + i) % 5];
        n += snprintf(body + n, sizeof(body) - n,
                      "    第 %d 天  %s  %s\n", e->day, action_name(e->action), e->detail);
    }

    ui_pause_panel("温室与日志", body);
}

void ui_do_tech(void)
{
    static char body[4096];
    int n = 0;

    TNode *root = tech_default_tree();
    int order_buf[128], pos = 0;

    preorder(root, order_buf, &pos);
    n += snprintf(body + n, sizeof(body) - n, "  科技树：%d 个节点，高度 %d，叶子 %d 个\n\n",
                  tree_count(root), tree_height(root), tree_leaves(root));

    n += snprintf(body + n, sizeof(body) - n, "  前序：");
    for (int i = 0; i < pos && n < 3700; i++)
        n += snprintf(body + n, sizeof(body) - n, "%d ", order_buf[i]);
    n += snprintf(body + n, sizeof(body) - n,
        "\n  遍历序列就是解锁顺序：前序是从上往下一层层看，\n"
        "  中序在二叉搜索树里会得到从小到大排好的顺序。\n");

    tree_free(root);
    ui_pause_panel("科技树", body);
}

void ui_do_graph(void)
{
    static char body[4096];
    int n = 0;

    n += snprintf(body + n, sizeof(body) - n, "  路网：%d 个设施，%d 条路\n\n",
                  s_graph.n, s_graph.edge_n);

    for (int v = 0; v < s_graph.n && n < 3400; v++) {
        n += snprintf(body + n, sizeof(body) - n, "  [%2d] %s ->", v, s_graph.name[v]);
        int k = 0;
        for (ArcNode *p = s_graph.head[v]; p && k < 6; p = p->next, k++)
            n += snprintf(body + n, sizeof(body) - n, " %d(%dm)", p->to, p->w);
        n += snprintf(body + n, sizeof(body) - n, "\n");
    }

    int path[MAXV], plen = 0;
    int cost = harvester_plan(&s_graph, 0, 11, path, &plen);
    n += snprintf(body + n, sizeof(body) - n, "\n  收割机：农场(0) → 仓库(11)\n  路径：");
    for (int i = 0; i < plen; i++)
        n += snprintf(body + n, sizeof(body) - n, "%s%s",
                      i ? " → " : "", s_graph.name[path[i]]);
    n += snprintf(body + n, sizeof(body) - n, "\n  总代价：%d（约 %d 分钟）\n", cost, cost / 60);

    /* 第 7 周的 BFS：不按"米数"算，只按"中间经过几个设施"算 */
    {
        int hop_path[MAXV], hop_len = 0;
        int hops = graph_min_hops(&s_graph, 0, 11, hop_path, &hop_len);
        if (hops >= 0)
            n += snprintf(body + n, sizeof(body) - n,
                          "  最少中转：%d 次（BFS，只数经过几个设施，不管路多远）\n", hops);
        else
            n += snprintf(body + n, sizeof(body) - n,
                          "  最少中转：（还没算出来 —— graph_min_hops 还没实现）\n");
    }

    ui_pause_panel("路网与最短路径", body);
}

void ui_do_index(void)
{
    static char body[4096];
    int n = 0;

    enum { N = 5000 };
    static Item all[N];
    index_demo_data(all, N);

    {
    long c0 = g_farm.cmp_count;
    index_build(&s_index, all, N, 500);

    for (int i = 0; i < 1000; i++)
        index_lookup(&s_index, all[i * 5 % N].crop_id, &all[0]);

    n += snprintf(body + n, sizeof(body) - n,
                  "  图鉴检索：%d 条记录，热数据 500 条放哈希表，冷数据 4500 条放有序数组\n\n"
                  "  查 1000 次的结果：\n"
                  "    命中热数据（哈希 O(1)）：%ld 次\n"
                  "    命中冷数据（二分 O(log n)）：%ld 次\n"
                  "    没找到：%ld 次\n"
                  "    总共比较 %ld 次\n\n"
                  "  如果全部用顺序查找，要比较约 %d 次。\n",
                  N, s_index.hit_hot, s_index.hit_cold, s_index.miss,
                  g_farm.cmp_count - c0, N * 1000 / 2);
    }

    index_destroy(&s_index);

    /* 重建，别把状态弄没了 */
    memset(&s_index, 0, sizeof s_index);
    index_build(&s_index, all, N, 500);

    ui_pause_panel("图鉴检索", body);
}

/* ---------------------------------------------------------------- 大地图 */
/* 按 B 打开：把 100×100 的稀疏矩阵画成 25×25 的缩略图。
 * 注意这里不是逐格去查（那要 1 万次），而是直接遍历"非零三元组" ——
 * 这正是稀疏矩阵存在的意义：只处理有内容的那几百个格子。 */
void ui_do_map(void)
{
    static char body[4096];
    char grid[25][25];
    int n = 0;
    int i, r, c;

    for (r = 0; r < 25; r++)
        for (c = 0; c < 25; c++) grid[r][c] = '.';

    for (i = 0; s_map.data && i < s_map.count; i++) {
        r = s_map.data[i].row / 4;      /* 每 4×4 压成一个字符 */
        c = s_map.data[i].col / 4;
        if (r < 0 || r >= 25 || c < 0 || c >= 25) continue;

        switch (s_map.data[i].val.type) {
            case TILE_GRASS:  grid[r][c] = ','; break;
            case TILE_WATER:  grid[r][c] = '~'; break;
            case TILE_ROCK:   grid[r][c] = '#'; break;
            case TILE_ROAD:   grid[r][c] = '='; break;
            case TILE_TILLED: grid[r][c] = 'T'; break;
            default: break;
        }
    }

    n += snprintf(body + n, sizeof(body) - n,
                  "  大地图 100×100 = 10000 格，实际只存了 %d 个非空格子（%.1f%%）\n\n",
                  s_map.count,
                  s_map.count > 0 ? s_map.count * 100.0 / 10000.0 : 0.0);

    for (r = 0; r < 25; r++) {
        n += snprintf(body + n, sizeof(body) - n, "  ");
        for (c = 0; c < 25; c++)
            n += snprintf(body + n, sizeof(body) - n, "%c ", grid[r][c]);
        n += snprintf(body + n, sizeof(body) - n, "\n");
        if (n > 3400) break;
    }

    n += snprintf(body + n, sizeof(body) - n,
                  "\n  . 空地   , 草地   ~ 水   # 石头   = 路\n"
                  "  图上一格 = 实际 4×4 的地块\n\n"
                  "  内存对比：\n"
                  "    稠密存法  10000 格 × %d 字节 = %d 字节\n"
                  "    三元组    %d 条 × %d 字节 = %d 字节\n",
                  (int)sizeof(Tile), (int)(10000 * sizeof(Tile)),
                  s_map.cap, (int)sizeof(Triple), (int)(s_map.cap * sizeof(Triple)));

    ui_pause_panel("大地图（稀疏矩阵）", body);
}

/* ---------------------------------------------------------------- 装车台 */
/* 按 C 打开：把仓库里每种作物的数量当成"重量"，算合并装车的最小代价。
 * 每次只能把两堆并成一大堆，代价 = 两堆重量之和。合得越早的越吃亏，
 * 所以每次挑最小的两堆合 —— 这就是哈夫曼。 */
void ui_do_truck(void)
{
    static char body[4096];
    int w[MAX_CROPS];
    int k = 0, total = 0, n = 0, i;

    for (i = 0; g_farm.barn.data && i < g_farm.barn.count && k < MAX_CROPS; i++) {
        if (g_farm.barn.data[i].count > 0) {
            w[k] = g_farm.barn.data[i].count;
            total += w[k];
            k++;
        }
    }

    n += snprintf(body + n, sizeof(body) - n,
                  "  装车台：仓库里 %d 种货，共 %d 斤\n\n", k, total);

    if (k < 2) {
        n += snprintf(body + n, sizeof(body) - n,
                      "  至少要有两种货才谈得上合并，先去收点东西。\n");
        ui_pause_panel("装车台（哈夫曼）", body);
        return;
    }

    n += snprintf(body + n, sizeof(body) - n, "  待装车的重量：");
    for (i = 0; i < k && i < 12; i++)
        n += snprintf(body + n, sizeof(body) - n, "%d ", w[i]);
    if (k > 12) n += snprintf(body + n, sizeof(body) - n, "… (共 %d 堆)", k);
    n += snprintf(body + n, sizeof(body) - n, "\n\n");

    {
        int best = batch_min_cost(w, k);
        int seq = 0, acc = w[0];

        for (i = 1; i < k; i++) {
            acc += w[i];
            seq += acc;
        }

        n += snprintf(body + n, sizeof(body) - n,
                      "  每次合最小的两堆（哈夫曼）  总代价 %d\n"
                      "  按仓库顺序一压一压地合      总代价 %d\n",
                      best, seq);

        if (best > 0 && seq > best)
            n += snprintf(body + n, sizeof(body) - n,
                          "  省下 %d（%.0f%%）—— 重的堆放浅一点，这就是哈夫曼的道理\n",
                          seq - best, (seq - best) * 100.0 / seq);
        else if (best == 0)
            n += snprintf(body + n, sizeof(body) - n,
                          "  （哈夫曼还没算出来：huffman_build 还没实现）\n");
    }

    ui_pause_panel("装车台（哈夫曼）", body);
}

void ui_do_help(void)
{
    static char body[4096];
    snprintf(body, sizeof body,
        "  【怎么玩】\n"
        "    把荒地开垦成田（T），播种（S），浇水（W），等它长大，收获（H），\n"
        "    然后按 O 交付订单换钱。每块地每天都要浇水，不然会干死。\n\n"
        "  【按键】\n"
        "    方向键             移动光标（字母键都留给功能了，别用 WASD）\n"
        "    1-9                换手上的种子\n"
        "    T                  开垦（10 分钟，掉 10 点体力）\n"
        "    S                  播种（花种子钱）\n"
        "    W                  浇水（1 分钟，让水分回到 100）\n"
        "    H                  收获（5 分钟，产物进仓库）\n"
        "    O                  交付队头订单\n"
        "    V                  出货：仓库里的东西按今日市价全卖掉\n"
        "    U                  撤销最近一步\n"
        "    N                  睡觉，进入下一天（自动存档）\n"
        "    M                  看今日行情（会调用排序）\n"
        "    G                  温室与日志\n"
        "    K                  科技树\n"
        "    R                  路网：最短路径 + 最少中转\n"
        "    B                  大地图（稀疏矩阵，100×100）\n"
        "    C                  装车台（哈夫曼，算合并代价）\n"
        "    I                  图鉴检索\n"
        "    Q                  退出\n\n"
        "  【最重要的一件事】\n"
        "    屏幕右上角那个“计算已耗”，是你自己代码的代价。\n"
        "    1 次比较 = 0.05 秒游戏时间，1 次移动 = 0.10 秒。\n"
        "    你写的算法越慢，这一天就越短，能收的菜就越少。\n"
        "    这不是比喻 —— 按 M 看行情时，排序差的同学会明显感觉到时间在掉。\n");
    ui_pause_panel("帮助", body);
}

/* ---------------------------------------------------------------- 主循环 */
void game_loop(void)
{
    render_frame();

    for (;;) {
        int k = term_key();

        switch (k) {
            /* 移动：只用方向键。字母键全部分给功能，免得 w 既是"上"又是"浇水" */
            case KEY_UP:    if (g_ui.cy > 0) g_ui.cy--; break;
            case KEY_DOWN:  if (g_ui.cy < WORLD_H - 1) g_ui.cy++; break;
            case KEY_LEFT:  if (g_ui.cx > 0) g_ui.cx--; break;
            case KEY_RIGHT: if (g_ui.cx < WORLD_W - 1) g_ui.cx++; break;

            /* 农活 */
            case 't': case 'T': ui_do_till();    break;
            case 's': case 'S': ui_do_sow();     break;
            case 'w': case 'W': ui_do_water();   break;
            case 'h': case 'H': ui_do_harvest(); break;
            case 'o': case 'O': ui_do_order();   break;
            case 'u': case 'U': ui_do_undo();    break;
            case 'v': case 'V': ui_do_sell();     break;
            case 'n': case 'N': ui_do_next_day(); break;

            /* 面板 */
            case 'm': case 'M': ui_do_market();      break;
            case 'g': case 'G': ui_do_growth_list(); break;
            case 'k': case 'K': ui_do_tech();        break;
            case 'r': case 'R': ui_do_graph();       break;
            case 'b': case 'B': ui_do_map();         break;
            case 'c': case 'C': ui_do_truck();       break;
            case 'i': case 'I': ui_do_index();       break;
            case '?':           ui_do_help();        break;

            case 'q': case 'Q': g_farm.quit = 1;     break;

            default:
                if (k >= '1' && k <= '9') {
                    int idx = k - '1';
                    if (idx < g_crop_n) {
                        g_ui.sel_crop = g_crops[idx].id;
                        toast("换成 %s", g_crops[idx].name);
                    }
                }
                break;
        }

        if (g_farm.quit) break;

        clock_tick_real(0.05);
        render_frame();

#ifdef _WIN32
        Sleep(30);
#else
        struct timespec ts = {0, 30000000L};
        nanosleep(&ts, NULL);
#endif
    }

    save_game("save/farm.sav");
}
