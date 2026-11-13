/* ============================================================================
 *  src/stub.c  ——  临时实现（老师给，不用你改）
 *
 *  这不是"什么都不做的空壳"，而是"能跑但很笨"的实现：
 *  全是定长数组 + 线性扫描，复杂度 O(n) 甚至 O(n²)。
 *  游戏能玩，但你会明显感觉到卡 —— 那就是数据结构要解决的问题。
 *
 *  你的任务：用课上学的数据结构，把每一段替换掉。
 *
 *  【不用你删任何东西】
 *  你在 src/ 下新建 field.c，脚本就自动给编译器加上 -DPF_HAVE_FIELD，
 *  下面【第 1 周】那一整段就自动失效了，不会和你写的函数撞车。
 *  换句话说：src/field.c 一出现，stub.c 里第 1 周的代码就"隐身"了。
 *
 *  ┌────────────────────────────────────────────────────────────┐
 *  │ 第 1 周：复制 templates/field.c 到 src/field.c              │
 *  │ 第 2 周：复制 templates/growth.c 到 src/growth.c            │
 *  │ ……以此类推                                                │
 *  └────────────────────────────────────────────────────────────┘
 *
 *  反过来说：你一旦建了 src/field.c，第 1 周那 14 个函数就全得由你提供。
 *  少一个，链接时就会报 undefined reference to 'xxx'。
 *  所以从 templates/ 复制出来再改，不要从空文件开始写。
 * ========================================================================== */
#include "engine.h"

/* ============================================================================
 *  【第 1 周】field.c 的临时实现
 *  新建 src/field.c 之后，这一整段自动失效（怎么做到的见文件开头）
 * ========================================================================== */
#ifndef PF_HAVE_FIELD

/* --- 朴素版：地块直接存在 g_farm.plots 里，仓库用定长数组 + 线性扫描 --- */

Plot *plot_at(int x, int y)
{
    if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return NULL;
    return &g_farm.plots[y][x];
}

int field_till(int x, int y)
{
    Plot *p = plot_at(x, y);
    if (!p) return PF_PARAM;
    if (p->state != PLOT_WILD && p->state != PLOT_DEAD) return PF_PARAM;
    if (g_farm.stamina < 10) return PF_EMPTY;

    p->state = PLOT_TILLED;
    p->crop_id = -1;
    p->water = 30;
    g_farm.stamina -= 10;
    g_farm.till_count++;

    advance_time(600);          /* 开垦很费时间：10 分钟 */
    return PF_OK;
}

int field_sow(int x, int y, int crop_id)
{
    Plot *p = plot_at(x, y);
    if (!p) return PF_PARAM;
    if (p->state != PLOT_TILLED) return PF_PARAM;

    const Crop *c = crop_by_id(crop_id);
    if (!c) return PF_PARAM;
    if (g_farm.gold < c->seed_cost) return PF_EMPTY;

    g_farm.gold -= c->seed_cost;
    p->crop_id = crop_id;
    p->state = PLOT_SEEDED;
    p->planted_day = g_farm.day;
    p->ripe_day = g_farm.day + c->grow_days;

    advance_time(120);
    return PF_OK;
}

int field_water(int x, int y)
{
    Plot *p = plot_at(x, y);
    if (!p) return PF_PARAM;
    if (p->state == PLOT_WILD) return PF_PARAM;

    p->water = 100;
    advance_time(60);
    return PF_OK;
}

int field_harvest(int x, int y)
{
    Plot *p = plot_at(x, y);
    if (!p) return PF_PARAM;
    if (p->state != PLOT_RIPE) return PF_PARAM;

    int crop_id = p->crop_id;
    const Crop *c = crop_by_id(crop_id);
    int qty = c ? (3 + pf_rand(4)) : 1;     /* 一株产 3~6 斤 */

    int rc = barn_add(&g_farm.barn, crop_id, qty);
    if (rc != PF_OK) return rc;             /* 仓库满了 */

    p->state = PLOT_TILLED;
    p->crop_id = -1;
    p->planted_day = 0;
    p->ripe_day = 0;

    advance_time(300);
    return qty;
}

void field_new_day(void)
{
    for (int y = 0; y < WORLD_H; y++) {
        for (int x = 0; x < WORLD_W; x++) {
            Plot *p = &g_farm.plots[y][x];
            if (p->state != PLOT_SEEDED && p->state != PLOT_GROWING &&
                p->state != PLOT_RIPE) continue;

            /* 每天扣水，没浇水会干死 */
            p->water -= 25;
            if (p->water <= 0) {
                p->water = 0;
                if (pf_rand(100) < 40) {    /* 40% 概率枯死 */
                    p->state = PLOT_DEAD;
                    continue;
                }
            }

            /* 到日子就熟 */
            if (g_farm.day >= p->ripe_day) p->state = PLOT_RIPE;
            else                           p->state = PLOT_GROWING;
        }
    }
}

int barn_init(Barn *b, int cap)
{
    if (!b || cap <= 0) return PF_PARAM;
    b->data = (Item *)malloc((size_t)cap * sizeof(Item));
    if (!b->data) return PF_NOMEM;
    b->count = 0;
    b->cap = cap;
    return PF_OK;
}

void barn_destroy(Barn *b)
{
    if (!b) return;
    free(b->data);
    b->data = NULL;
    b->count = b->cap = 0;
}

/* TODO(第 1 周)：这只是"翻倍扩容"，你要检查 realloc 失败时别丢原指针 */
int barn_grow(Barn *b)
{
    if (!b) return PF_PARAM;
    int newcap = b->cap * 2;
    Item *p = (Item *)realloc(b->data, (size_t)newcap * sizeof(Item));
    if (!p) return PF_NOMEM;
    b->data = p;
    b->cap = newcap;
    cost_malloc(1);
    return PF_OK;
}

/* TODO(第 1 周)：朴素版是"线性扫描找有没有，没有就追加"。这很慢，你要重写 */
int barn_add(Barn *b, int crop_id, int count)
{
    if (!b || count <= 0) return PF_PARAM;

    for (int i = 0; i < b->count; i++) {
        cost_compare(1);
        if (b->data[i].crop_id == crop_id) {
            b->data[i].count += count;
            return PF_OK;
        }
    }

    if (b->count >= b->cap) {
        int rc = barn_grow(b);
        if (rc != PF_OK) return rc;
    }
    b->data[b->count].crop_id = crop_id;
    b->data[b->count].count = count;
    b->data[b->count].quality = 3;
    b->count++;
    return PF_OK;
}

/* TODO(第 1 周)：删除中间元素要挪后面所有元素，这才是 O(n) 的代价 */
int barn_remove_at(Barn *b, int pos, Item *out)
{
    if (!b) return PF_PARAM;
    if (b->count == 0) return PF_EMPTY;
    if (pos < 0 || pos >= b->count) return PF_PARAM;

    if (out) *out = b->data[pos];
    for (int i = pos; i < b->count - 1; i++) {
        b->data[i] = b->data[i + 1];
        cost_move(1);
    }
    b->count--;
    return PF_OK;
}

int barn_find(const Barn *b, int crop_id)
{
    if (!b) return -1;
    for (int i = 0; i < b->count; i++) {
        cost_compare(1);
        if (b->data[i].crop_id == crop_id) return i;
    }
    return -1;
}

int barn_load_csv(Barn *b, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return PF_NOTFOUND;

    char line[256];
    /* 跳过表头 */
    if (!fgets(line, sizeof line, f)) { fclose(f); return PF_PARAM; }

    while (fgets(line, sizeof line, f)) {
        int cid = -1, cnt = 0, ql = 3;
        if (sscanf(line, "%d,%d,%d", &cid, &cnt, &ql) >= 2)
            barn_add(b, cid, cnt);
    }
    fclose(f);
    return PF_OK;
}

int barn_save_csv(const Barn *b, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f) return PF_PARAM;

    fprintf(f, "crop_id,count,quality\n");
    for (int i = 0; i < b->count; i++)
        fprintf(f, "%d,%d,%d\n",
                b->data[i].crop_id, b->data[i].count, b->data[i].quality);
    fclose(f);
    return PF_OK;
}
#endif /* PF_HAVE_FIELD */

/* ======================== 第 1 周结束 ==================================== */

/* ============================================================================
 *  【第 2 周】growth.c 的临时实现
 *  新建 src/growth.c 之后，这一整段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_GROWTH

/* 朴素版：确实是链表，但"插入不排序、取最早熟要全扫一遍"。
 * 你要做的改进：插入的时候按 ripe_day 排好序，
 * 这样取最早熟就是 O(1)（直接拿哨兵后面那个）。
 *
 * 顺带体会一下：同样是链表，写法不同，复杂度差一个数量级。 */

int growth_init(GrowthList *g)
{
    if (!g) return PF_PARAM;

    /* 哨兵节点：不存数据，只为了让"删第一个"和"删中间"能用同一套代码 */
    g->head = (GNode *)calloc(1, sizeof(GNode));
    if (!g->head) return PF_NOMEM;
    g->head->next = NULL;
    g->size = 0;
    return PF_OK;
}

void growth_destroy(GrowthList *g)
{
    GNode *p, *q;
    if (!g) return;

    p = g->head;
    while (p) {
        q = p->next;      /* 先记住下一个，再 free 当前 */
        free(p);
        p = q;
    }
    g->head = NULL;
    g->size = 0;
}

/* 尾插：从头一路走到尾，把新节点挂上去。
 * 没有排序，所以 O(n)，而且取的时候还得再扫一遍。 */
int growth_insert(GrowthList *g, int crop_id, int ripe_day, int x, int y)
{
    GNode *p, *node;

    if (!g) return PF_PARAM;

    node = (GNode *)calloc(1, sizeof(GNode));
    if (!node) return PF_NOMEM;
    node->crop_id = crop_id;
    node->ripe_day = ripe_day;
    node->x = x;
    node->y = y;
    node->next = NULL;

    p = g->head;
    while (p->next) {
        p = p->next;
        cost_move(1);     /* 走一步也算挪了一次，记上 */
    }
    p->next = node;
    g->size++;
    return PF_OK;
}

/* 取最早熟：全扫一遍找最小，再从链表上摘掉 */
int growth_pop_earliest(GrowthList *g, GNode *out)
{
    GNode *p, *pre, *best, *bestpre;

    if (!g || !out) return PF_PARAM;
    if (!g->head || !g->head->next) return PF_EMPTY;

    bestpre = g->head;
    best = g->head->next;
    pre = g->head;

    for (p = g->head->next; p; p = p->next) {
        cost_compare(1);
        if (p->ripe_day < best->ripe_day) {
            best = p;
            bestpre = pre;
        }
        pre = p;
    }

    *out = *best;               /* 把数据拷出来 */
    bestpre->next = best->next; /* 从链表里摘掉 */
    free(best);
    g->size--;
    return PF_OK;
}

/* 按作物编号删除第一个匹配的节点 */
int growth_remove(GrowthList *g, int crop_id)
{
    GNode *p, *pre;

    if (!g) return PF_PARAM;

    pre = g->head;
    for (p = g->head->next; p; p = p->next) {
        cost_compare(1);
        if (p->crop_id == crop_id) {
            pre->next = p->next;
            free(p);
            g->size--;
            return PF_OK;
        }
        pre = p;
    }
    return PF_NOTFOUND;
}

/* ---------------------------------------------------------------- 操作日志 */

int log_init(LogList *L)
{
    if (!L) return PF_PARAM;

    L->head = (LNode *)calloc(1, sizeof(LNode));
    if (!L->head) return PF_NOMEM;
    L->head->next = NULL;
    L->tail = L->head;      /* 空表时尾指针指向哨兵 */
    L->size = 0;
    return PF_OK;
}

void log_destroy(LogList *L)
{
    LNode *p, *q;
    if (!L) return;

    p = L->head;
    while (p) {
        q = p->next;
        free(p);
        p = q;
    }
    L->head = L->tail = NULL;
    L->size = 0;
}

/* 尾插：有 tail 指针，所以不用找尾巴，O(1) */
int log_append(LogList *L, int day, int action, const char *detail)
{
    LNode *node;

    if (!L) return PF_PARAM;

    node = (LNode *)calloc(1, sizeof(LNode));
    if (!node) return PF_NOMEM;
    node->day = day;
    node->action = action;
    snprintf(node->detail, sizeof(node->detail), "%s", detail ? detail : "");
    node->next = NULL;

    L->tail->next = node;
    L->tail = node;
    L->size++;
    return PF_OK;
}

/* 原地反转：三根指针一路往右挪。
 * 反转完 head->next 是原来最后一个节点，tail 要重新指到原来第一个节点。 */
void log_reverse(LogList *L)
{
    LNode *pre, *cur, *next;

    if (!L || !L->head) return;

    pre = NULL;
    cur = L->head->next;
    L->tail = cur;          /* 反转后，原来的第一个变成最后一个 */
    while (cur) {
        next = cur->next;
        cur->next = pre;
        pre = cur;
        cur = next;
    }
    L->head->next = pre;
}
#endif /* PF_HAVE_GROWTH */

/* ======================== 第 2 周结束 ==================================== */

/* ============================================================================
 *  【第 3 周】undo.c / order.c 的临时实现
 *  新建 src/undo.c / src/order.c 之后，这两段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_UNDO


int undo_init(UndoStack *s, int cap)
{
    if (!s || cap <= 0) return PF_PARAM;
    s->data = (UndoRec *)calloc((size_t)cap, sizeof(UndoRec));
    if (!s->data) return PF_NOMEM;
    s->top = 0;
    s->cap = cap;
    return PF_OK;
}

void undo_destroy(UndoStack *s)
{
    if (!s) return;
    free(s->data);
    s->data = NULL;
    s->top = s->cap = 0;
}

int undo_push(UndoStack *s, const UndoRec *r)
{
    if (!s || !r) return PF_PARAM;
    if (s->top >= s->cap) return PF_FULL;
    s->data[s->top++] = *r;
    return PF_OK;
}

int undo_pop(UndoStack *s, UndoRec *out)
{
    if (!s) return PF_PARAM;
    if (s->top <= 0) return PF_EMPTY;
    if (out) *out = s->data[--s->top];
    else     s->top--;
    return PF_OK;
}

int undo_top(const UndoStack *s, UndoRec *out)
{
    if (!s) return PF_PARAM;
    if (s->top <= 0) return PF_EMPTY;
    if (out) *out = s->data[s->top - 1];
    return PF_OK;
}

int undo_depth(const UndoStack *s)
{
    return s ? s->top : 0;
}

/* TODO(第 3 周)：这一步要"反向执行"栈顶记录，而不是简单地弹出来 */
int undo_apply(UndoStack *s)
{
    UndoRec r;
    if (undo_pop(s, &r) != PF_OK) return PF_EMPTY;

    switch (r.action) {
        case ACT_TILL: {
            Plot *p = plot_at(r.x, r.y);
            if (p) { p->state = PLOT_WILD; p->crop_id = -1; }
            g_farm.gold -= r.gold;
            advance_time(600);
            break;
        }
        case ACT_SOW: {
            Plot *p = plot_at(r.x, r.y);
            if (p) { p->state = PLOT_TILLED; p->crop_id = -1; }
            g_farm.gold -= r.gold;
            advance_time(120);
            break;
        }
        case ACT_WATER:
            if (plot_at(r.x, r.y)) plot_at(r.x, r.y)->water = 30;
            advance_time(60);
            break;
        case ACT_HARVEST:
            g_farm.gold -= r.gold;
            advance_time(300);
            break;
        case ACT_SELL:
            g_farm.gold -= r.gold;
            barn_add(&g_farm.barn, r.crop_id, r.count);
            advance_time(60);
            break;
        default: break;
    }
    return PF_OK;
}

#endif /* PF_HAVE_UNDO */

#ifndef PF_HAVE_ORDER
/* ---------- 订单队列 ---------- */

/* 循环队列：front/rear 都用取模 */
int order_init(OrderQueue *q, int cap)
{
    if (!q || cap <= 0) return PF_PARAM;
    q->data = (Order *)calloc((size_t)(cap + 1), sizeof(Order));
    if (!q->data) return PF_NOMEM;
    q->front = q->rear = 0;
    q->cap = cap + 1;        /* 留一格，用来区分空和满 */
    return PF_OK;
}

void order_destroy(OrderQueue *q)
{
    if (!q) return;
    free(q->data);
    q->data = NULL;
    q->front = q->rear = q->cap = 0;
}

int order_size(const OrderQueue *q)
{
    if (!q) return 0;
    return (q->rear - q->front + q->cap) % q->cap;
}

int order_enqueue(OrderQueue *q, const Order *o)
{
    if (!q || !o) return PF_PARAM;
    if ((q->rear + 1) % q->cap == q->front) return PF_FULL;
    q->data[q->rear] = *o;
    q->rear = (q->rear + 1) % q->cap;
    return PF_OK;
}

int order_dequeue(OrderQueue *q, Order *out)
{
    if (!q) return PF_PARAM;
    if (q->front == q->rear) return PF_EMPTY;
    if (out) *out = q->data[q->front];
    q->front = (q->front + 1) % q->cap;
    return PF_OK;
}

int order_peek(const OrderQueue *q, Order *out)
{
    if (!q) return PF_PARAM;
    if (q->front == q->rear) return PF_EMPTY;
    if (out) *out = q->data[q->front];
    return PF_OK;
}

void order_make(Order *o, int id)
{
    if (!o) return;
    int cid = g_crop_n > 0 ? pf_rand(g_crop_n) : 0;
    o->id = id;
    o->crop_id = cid;
    o->count = 5 + pf_rand(16);
    o->price = g_crops[cid].price * o->count;
    o->status = ST_PENDING;
    o->day = g_farm.day;
}
#endif /* PF_HAVE_ORDER */

/* ======================== 第 3 周结束 ==================================== */

/* ============================================================================
 *  【第 4 周】map.c 的临时实现
 *  新建 src/map.c 之后，这一整段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_MAP

static Tile s_dense[128][128];      /* 朴素版：直接开一个稠密二维数组 */
static int  s_dense_inited = 0;

int sm_init(SparseMat *m, int rows, int cols)
{
    if (!m || rows <= 0 || cols <= 0) return PF_PARAM;
    m->data = NULL;
    m->count = 0;
    m->cap = 0;
    m->rows = rows;
    m->cols = cols;
    if (!s_dense_inited) {
        memset(s_dense, 0, sizeof s_dense);
        s_dense_inited = 1;
    }
    return PF_OK;
}

void sm_destroy(SparseMat *m)
{
    if (!m) return;
    free(m->data);
    m->data = NULL;
    m->count = m->cap = 0;
}

/* TODO(第 4 周)：这里把稠密矩阵整个扫一遍存成三元组。
 * 100×100 是 10000 个格子，但只有 300 多个非零 —— 这就是"稀疏"的意义。 */
int sm_from_dense(SparseMat *m, const Tile *dense, int rows, int cols)
{
    if (!m || !dense) return PF_PARAM;
    if (rows > 128 || cols > 128) return PF_PARAM;

    m->rows = rows;
    m->cols = cols;
    free(m->data);
    m->count = 0;

    /* 先数一遍有几个非零，再按需分配。
     * 如果直接开 rows×cols 个 Triple，那就退化成稠密矩阵了，白折腾。 */
    int cnt = 0;
    for (int i = 0; i < rows * cols; i++)
        if (dense[i].type != TILE_EMPTY) cnt++;

    m->cap = cnt;
    if (cnt == 0) { m->data = NULL; return PF_OK; }

    m->data = (Triple *)malloc((size_t)cnt * sizeof(Triple));
    if (!m->data) { m->cap = 0; return PF_NOMEM; }

    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++) {
            const Tile *t = &dense[r * cols + c];
            if (t->type == TILE_EMPTY) continue;
            m->data[m->count].row = r;
            m->data[m->count].col = c;
            m->data[m->count].val = *t;
            m->count++;
        }
    return PF_OK;
}

/* TODO(第 4 周)：慢转置 —— 按列扫描，O(cols × count) */
int sm_transpose_slow(const SparseMat *src, SparseMat *dst)
{
    if (!src || !dst) return PF_PARAM;
    if (sm_init(dst, src->cols, src->rows) != PF_OK) return PF_NOMEM;

    dst->data = (Triple *)malloc((size_t)(src->count + 1) * sizeof(Triple));
    if (!dst->data) return PF_NOMEM;
    dst->cap = src->count + 1;
    dst->count = 0;

    for (int c = 0; c < src->cols; c++)
        for (int i = 0; i < src->count; i++) {
            cost_compare(1);
            if (src->data[i].col == c) {
                dst->data[dst->count].row = src->data[i].col;
                dst->data[dst->count].col = src->data[i].row;
                dst->data[dst->count].val = src->data[i].val;
                dst->count++;
            }
        }
    return PF_OK;
}

/* TODO(第 4 周)：快转置 —— 先统计每列个数，再定位，O(count) */
int sm_transpose_fast(const SparseMat *src, SparseMat *dst)
{
    return sm_transpose_slow(src, dst);   /* 朴素版就先这样，你要优化它 */
}

int sm_add(const SparseMat *a, const SparseMat *b, SparseMat *out)
{
    if (!a || !b || !out) return PF_PARAM;
    if (a->rows != b->rows || a->cols != b->cols) return PF_PARAM;
    return sm_init(out, a->rows, a->cols);
}

int sm_get(const SparseMat *m, int r, int c, Tile *out)
{
    if (!m || !out) return PF_PARAM;
    if (r < 0 || r >= m->rows || c < 0 || c >= m->cols) return PF_PARAM;

    for (int i = 0; i < m->count; i++) {
        cost_compare(1);
        if (m->data[i].row == r && m->data[i].col == c) {
            *out = m->data[i].val;
            return PF_OK;
        }
    }
    out->type = TILE_EMPTY;
    out->crop_id = -1;
    return PF_OK;
}

void map_generate(SparseMat *m, unsigned seed)
{
    if (!m) return;
    unsigned st = seed ? seed : 1u;

    /* 先把 m 初始化干净：外面可能传进来一个没初始化的结构体，
     * 里面 sm_from_dense 会 free(m->data)，野指针会直接把堆写坏。 */
    sm_init(m, 100, 100);

    for (int r = 0; r < 128; r++)
        for (int c = 0; c < 128; c++) {
            s_dense[r][c].type = TILE_EMPTY;
            s_dense[r][c].crop_id = -1;
        }

    /* 对角线附近铺一些草地和水塘，做出"稀疏"的效果 */
    for (int r = 0; r < 100; r++) {
        for (int c = 0; c < 100; c++) {
            st ^= st << 13; st ^= st >> 17; st ^= st << 5;
            unsigned v = st % 1000;    /* 0~999，只有 3% 的格子有内容 */
            int t = TILE_EMPTY;
            if (v < 8)       t = TILE_WATER;   /* 0.8% */
            else if (v < 14) t = TILE_ROCK;    /* 0.6% */
            else if (v < 20) t = TILE_GRASS;   /* 0.6% */
            else if (v < 30) t = TILE_ROAD;    /* 1.0% */
            s_dense[r][c].type = t;
            s_dense[r][c].crop_id = -1;
        }
    }
    sm_from_dense(m, &s_dense[0][0], 100, 100);
}
#endif /* PF_HAVE_MAP */

/* ======================== 第 4 周结束 ==================================== */

/* ============================================================================
 *  【第 5 周】tech.c 的临时实现
 *  新建 src/tech.c 之后，这一整段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_TECH

TNode *node_new(int id)
{
    TNode *t = (TNode *)calloc(1, sizeof(TNode));
    if (!t) return NULL;
    t->id = id;
    t->unlocked = 0;
    t->left = t->right = NULL;
    return t;
}

void tree_free(TNode *t)
{
    if (!t) return;
    tree_free(t->left);
    tree_free(t->right);
    free(t);
}

int tree_height(const TNode *t)
{
    if (!t) return 0;
    int l = tree_height(t->left);
    int r = tree_height(t->right);
    return (l > r ? l : r) + 1;
}

int tree_count(const TNode *t)
{
    if (!t) return 0;
    return 1 + tree_count(t->left) + tree_count(t->right);
}

int tree_leaves(const TNode *t)
{
    if (!t) return 0;
    if (!t->left && !t->right) return 1;
    return tree_leaves(t->left) + tree_leaves(t->right);
}

void preorder(TNode *t, int *out, int *pos)
{
    if (!t || !out || !pos) return;
    out[(*pos)++] = t->id;
    preorder(t->left, out, pos);
    preorder(t->right, out, pos);
}

void inorder(TNode *t, int *out, int *pos)
{
    if (!t || !out || !pos) return;
    inorder(t->left, out, pos);
    out[(*pos)++] = t->id;
    inorder(t->right, out, pos);
}

void postorder(TNode *t, int *out, int *pos)
{
    if (!t || !out || !pos) return;
    postorder(t->left, out, pos);
    postorder(t->right, out, pos);
    out[(*pos)++] = t->id;
}

/* TODO(第 5 周)：层序遍历要用队列，不能递归 */
void levelorder(TNode *t, int *out, int *pos)
{
    if (!t || !out || !pos) return;
    TNode *q[256];
    int h = 0, tl = 0;
    q[tl++] = t;
    while (h < tl && tl < 256) {
        TNode *cur = q[h++];
        out[(*pos)++] = cur->id;
        if (cur->left)  q[tl++] = cur->left;
        if (cur->right) q[tl++] = cur->right;
    }
}

/* 前序 + 中序 还原二叉树。
 * 思路：前序的第一个一定是根；在中序里找到它，左边是左子树，右边是右子树。 */
static TNode *build_rec(const int *pre, int pl, const int *in, int il, int n)
{
    if (n <= 0) return NULL;

    TNode *root = node_new(pre[pl]);

    int k = 0;
    while (k < n && in[il + k] != pre[pl]) k++;
    if (k >= n) k = 0;      /* 数据不合法，当左子树空处理 */

    root->left  = build_rec(pre, pl + 1,     in, il,          k);
    root->right = build_rec(pre, pl + 1 + k, in, il + k + 1,  n - 1 - k);
    return root;
}

TNode *tree_build(const int *pre, const int *in, int n)
{
    if (!pre || !in || n <= 0) return NULL;
    return build_rec(pre, 0, in, 0, n);
}

TNode *bst_insert(TNode *root, int id)
{
    if (!root) return node_new(id);
    cost_compare(1);
    if (id < root->id)      root->left  = bst_insert(root->left, id);
    else if (id > root->id) root->right = bst_insert(root->right, id);
    return root;
}

TNode *bst_find(TNode *root, int id)
{
    while (root) {
        cost_compare(1);
        if (id == root->id) return root;
        root = (id < root->id) ? root->left : root->right;
    }
    return NULL;
}

void tree_mirror(TNode *t)
{
    if (!t) return;
    TNode *tmp = t->left;
    t->left = t->right;
    t->right = tmp;
    tree_mirror(t->left);
    tree_mirror(t->right);
}

void tree_print(const TNode *t, int depth)
{
    if (!t) return;
    tree_print(t->right, depth + 1);
    for (int i = 0; i < depth; i++) printf("    ");
    printf("%d%s\n", t->id, t->unlocked ? " *" : "");
    tree_print(t->left, depth + 1);
}

TNode *tech_default_tree(void)
{
    /* 一棵小科技树：1 是根，往下分叉 */
    TNode *r = node_new(1);
    if (!r) return NULL;
    r->unlocked = 1;
    r->left  = node_new(2);
    r->right = node_new(3);
    if (r->left)  r->left->left = node_new(4);
    if (r->right) r->right->right = node_new(5);
    return r;
}
#endif /* PF_HAVE_TECH */

/* ======================== 第 5 周结束 ==================================== */

/* ============================================================================
 *  【第 6 周】huffman.c 的临时实现
 *  新建 src/huffman.c 之后，这一整段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_HUFF

static int s_heap_size = 0;

void heap_init(Heap *h, int cap)
{
    if (!h) return;
    h->size = 0;
    (void)cap;
    s_heap_size = 0;
}

/* TODO(第 6 周)：堆的上浮。朴素版就是"追加到数组末尾"，取的时候再扫最小值 */
void heap_push(Heap *h, int w, int node)
{
    if (!h) return;
    if (h->size >= MAX_HUFF * 2) return;
    h->w[h->size] = w;
    h->n[h->size] = node;
    h->size++;
    s_heap_size = h->size;
}

/* TODO(第 6 周)：应该 O(log n)。朴素版 O(n) 扫最小值，这就是差距 */
int heap_pop(Heap *h)
{
    if (!h || h->size <= 0) return -1;

    int best = 0;
    for (int i = 1; i < h->size; i++) {
        cost_compare(1);
        if (h->w[i] < h->w[best]) best = i;
    }
    int node = h->n[best];
    for (int i = best; i < h->size - 1; i++) {
        h->w[i] = h->w[i + 1];
        h->n[i] = h->n[i + 1];
        cost_move(1);
    }
    h->size--;
    s_heap_size = h->size;
    return node;
}

/* 建哈夫曼树：每次从"还没合并的节点"里挑两个权重最小的，合成一个新节点。
 * 这里用"排序 + 取前两个"，没有用堆 —— 你要改成堆，才叫 O(n log n)。 */
int huffman_build(HuffTree *t, const int *weights, int n)
{
    if (!t || !weights || n <= 0) return PF_PARAM;
    if (n > MAX_HUFF) return PF_PARAM;

    memset(t, 0, sizeof(HuffTree));
    for (int i = 0; i < n; i++) {
        t->nodes[i].weight = weights[i];
        t->nodes[i].parent = -1;
        t->nodes[i].left   = -1;
        t->nodes[i].right  = -1;
        t->nodes[i].ch     = (char)('A' + i % 26);
    }
    t->leaf_n = n;

    if (n == 1) { t->root = 0; t->size = 1; return PF_OK; }

    int alive[MAX_HUFF * 2];
    int alive_n = n;
    for (int i = 0; i < n; i++) alive[i] = i;

    int size = n;
    while (alive_n > 1) {
        /* 按权重把 alive 排好序（插入排序，稳住就行） */
        for (int i = 1; i < alive_n; i++) {
            int key = alive[i], j = i - 1;
            while (j >= 0 && t->nodes[alive[j]].weight > t->nodes[key].weight) {
                alive[j + 1] = alive[j];
                j--;
            }
            alive[j + 1] = key;
        }

        int a = alive[0], b = alive[1];
        if (size >= MAX_HUFF * 2) return PF_PARAM;

        t->nodes[size].weight = t->nodes[a].weight + t->nodes[b].weight;
        t->nodes[size].left   = a;
        t->nodes[size].right  = b;
        t->nodes[size].parent = -1;
        t->nodes[size].ch     = 0;
        t->nodes[a].parent = size;
        t->nodes[b].parent = size;

        /* 新节点回到队列头，把第二小的挪走 */
        alive[0] = size;
        for (int i = 1; i < alive_n - 1; i++) alive[i] = alive[i + 1];
        alive_n--;

        size++;
    }

    t->size = size;
    t->root = alive[0];
    return PF_OK;
}

/* 每个叶子的深度（从叶子往上数到根） */
static int node_depth(const HuffTree *t, int idx)
{
    int d = 0;
    int p = t->nodes[idx].parent;
    while (p >= 0) { d++; p = t->nodes[p].parent; }
    return d;
}

int huffman_wpl(const HuffTree *t)
{
    if (!t) return 0;
    int sum = 0;
    for (int i = 0; i < t->leaf_n; i++)
        sum += t->nodes[i].weight * node_depth(t, i);
    return sum;
}

/* 从叶子往上走，是左孩子记 0，右孩子记 1，最后倒过来 */
void huffman_code(const HuffTree *t, char code[][32])
{
    if (!t || !code) return;

    for (int i = 0; i < t->leaf_n; i++) {
        char tmp[32];
        int k = 0, cur = i;

        while (cur >= 0 && t->nodes[cur].parent >= 0 && k < 31) {
            int p = t->nodes[cur].parent;
            tmp[k++] = (t->nodes[p].left == cur) ? '0' : '1';
            cur = p;
        }
        for (int j = 0; j < k; j++) code[i][j] = tmp[k - 1 - j];
        code[i][k] = '\0';
    }
}

/* 从根开始按位往下走，走到叶子就输出一个字符，然后回到根 */
int huffman_decode(const HuffTree *t, const char *bits, int *out, int *outn)
{
    if (!t || !bits || !out || !outn) return PF_PARAM;

    int maxn = *outn, n = 0;
    int cur = t->root;

    for (int i = 0; bits[i]; i++) {
        if (cur < 0) break;
        cur = (bits[i] == '0') ? t->nodes[cur].left : t->nodes[cur].right;
        if (cur < 0) { cur = t->root; continue; }

        if (t->nodes[cur].left < 0 && t->nodes[cur].right < 0) {
            if (n < maxn) out[n++] = cur;
            cur = t->root;
        }
    }
    *outn = n;
    return n;
}

int batch_min_cost(const int *weights, int n)
{
    if (!weights || n <= 0) return 0;
    HuffTree t;
    if (huffman_build(&t, weights, n) != PF_OK) return 0;
    return huffman_wpl(&t);
}
#endif /* PF_HAVE_HUFF */

/* ======================== 第 6 周结束 ==================================== */

/* ============================================================================
 *  【第 7 周】road.c 的临时实现
 *  新建 src/road.c 之后，这一整段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_ROAD

int graph_init(Graph *g, int n)
{
    if (!g) return PF_PARAM;
    if (n < 0 || n > MAXV) return PF_PARAM;
    memset(g, 0, sizeof(Graph));
    g->n = n;
    return PF_OK;
}

void graph_destroy(Graph *g)
{
    if (!g) return;
    for (int i = 0; i < MAXV; i++) {
        ArcNode *p = g->head[i];
        while (p) {
            ArcNode *nx = p->next;
            free(p);
            p = nx;
        }
        g->head[i] = NULL;
    }
    g->n = g->edge_n = 0;
}

int graph_add_edge(Graph *g, int u, int v, int w)
{
    if (!g || u < 0 || u >= g->n || v < 0 || v >= g->n) return PF_PARAM;

    /* 先把两条边都分配好，都成功了再插进链表。
     * 这样万一第二条分配失败，直接把第一条 free 掉就行，不用回滚链表。 */
    ArcNode *a = (ArcNode *)malloc(sizeof(ArcNode));
    if (!a) return PF_NOMEM;

    ArcNode *b = (ArcNode *)malloc(sizeof(ArcNode));
    if (!b) { free(a); return PF_NOMEM; }

    a->to = v; a->w = w; a->next = g->head[u];
    g->head[u] = a;

    b->to = u; b->w = w; b->next = g->head[v];
    g->head[v] = b;

    g->edge_n++;
    return PF_OK;
}

static void dfs_rec(Graph *g, int v, int *vis, int *out, int *pos)
{
    vis[v] = 1;
    out[(*pos)++] = v;
    for (ArcNode *p = g->head[v]; p; p = p->next) {
        cost_compare(1);
        if (!vis[p->to]) dfs_rec(g, p->to, vis, out, pos);
    }
}

void graph_dfs(Graph *g, int s, int *out, int *pos)
{
    if (!g || !out || !pos) return;
    if (s < 0 || s >= g->n) return;
    int vis[MAXV] = {0};
    dfs_rec(g, s, vis, out, pos);
}

void graph_bfs(Graph *g, int s, int *out, int *pos)
{
    if (!g || !out || !pos) return;
    if (s < 0 || s >= g->n) return;

    int vis[MAXV] = {0};
    int q[MAXV];
    int h = 0, tl = 0;
    vis[s] = 1;
    q[tl++] = s;
    while (h < tl) {
        int v = q[h++];
        out[(*pos)++] = v;
        for (ArcNode *p = g->head[v]; p; p = p->next) {
            cost_compare(1);
            if (!vis[p->to]) { vis[p->to] = 1; q[tl++] = p->to; }
        }
    }
}

int graph_min_hops(Graph *g, int s, int t, int *path, int *plen)
{
    if (!g || !path || !plen) return PF_PARAM;
    if (s < 0 || s >= g->n || t < 0 || t >= g->n) return PF_PARAM;

    int prev[MAXV], dist[MAXV];
    for (int i = 0; i < MAXV; i++) { prev[i] = -1; dist[i] = -1; }

    int q[MAXV], h = 0, tl = 0;
    dist[s] = 0;
    q[tl++] = s;
    while (h < tl) {
        int v = q[h++];
        if (v == t) break;
        for (ArcNode *p = g->head[v]; p; p = p->next) {
            cost_compare(1);
            if (dist[p->to] < 0) {
                dist[p->to] = dist[v] + 1;
                prev[p->to] = v;
                q[tl++] = p->to;
            }
        }
    }
    if (dist[t] < 0) { *plen = 0; return PF_NOTFOUND; }

    int tmp[MAXV], n = 0;
    for (int v = t; v != -1; v = prev[v]) tmp[n++] = v;
    for (int i = 0; i < n; i++) path[i] = tmp[n - 1 - i];
    *plen = n;
    return dist[t];
}

int graph_components(Graph *g, int *comp_of)
{
    if (!g || !comp_of) return PF_PARAM;
    int vis[MAXV] = {0};
    int comp = 0;
    for (int s = 0; s < g->n; s++) {
        if (vis[s]) continue;
        int out[MAXV], pos = 0;
        dfs_rec(g, s, vis, out, &pos);
        for (int i = 0; i < pos; i++) comp_of[out[i]] = comp;
        comp++;
    }
    for (int i = 0; i < MAXV; i++) if (i >= g->n) comp_of[i] = -1;
    return comp;
}

int graph_find_isolated(Graph *g, int *out)
{
    if (!g || !out) return PF_PARAM;
    int n = 0;
    for (int v = 0; v < g->n; v++)
        if (!g->head[v]) out[n++] = v;
    return n;
}

void graph_print(const Graph *g)
{
    if (!g) return;
    printf("图：%d 个顶点，%d 条边\n", g->n, g->edge_n);
    for (int v = 0; v < g->n; v++) {
        printf("  [%2d] %-12s ->", v, g->name[v]);
        for (ArcNode *p = g->head[v]; p; p = p->next)
            printf(" %d(%dm)", p->to, p->w);
        printf("\n");
    }
}

void graph_load_default(Graph *g)
{
    static const char *names[12] = {
        "农场", "谷仓", "水井", "集市", "码头", "磨坊",
        "风车", "牧场", "果园", "蜂房", "工坊", "仓库"
    };
    graph_init(g, 12);
    for (int i = 0; i < 12; i++)
        snprintf(g->name[i], sizeof g->name[i], "%s", names[i]);

    /* 15 条路，权重是米数 */
    static const int E[15][3] = {
        {0,1,120},{0,2,90},{0,3,300},{1,4,180},{1,5,240},
        {2,5,150},{3,6,210},{3,7,160},{4,8,270},{5,8,190},
        {6,9,130},{7,9,220},{8,10,170},{9,10,140},{10,11,260}
    };
    for (int i = 0; i < 15; i++) graph_add_edge(g, E[i][0], E[i][1], E[i][2]);
}
#endif /* PF_HAVE_ROAD */

/* ======================== 第 7 周结束 ==================================== */

/* ============================================================================
 *  【第 8 周】path.c 的临时实现
 *  新建 src/path.c 之后，这一整段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_PATH

int edge_cost(int len_m, int road_type)
{
    /* 石板路打 8 折，土路原价。越短越好，所以这里返回"秒数" */
    int sec = len_m / 4;             /* 每秒走 4 米 */
    if (road_type == TILE_ROAD) sec = sec * 8 / 10;
    if (sec < 1) sec = 1;
    return sec;
}

/* TODO(第 8 周)：Dijkstra 要用"每次取 dist 最小的未访问点"。
 * 朴素版用线性扫描 O(V²)，你要改成用堆 O((V+E)logV) */
int dijkstra(Graph *g, int src, int *dist, int *prev)
{
    if (!g || !dist || !prev) return PF_PARAM;
    if (src < 0 || src >= g->n) return PF_PARAM;

    int vis[MAXV] = {0};
    for (int i = 0; i < g->n; i++) { dist[i] = INF; prev[i] = -1; }
    dist[src] = 0;

    for (int it = 0; it < g->n; it++) {
        int u = -1, best = INF;
        for (int i = 0; i < g->n; i++) {
            cost_compare(1);
            if (!vis[i] && dist[i] < best) { best = dist[i]; u = i; }
        }
        if (u < 0) break;
        vis[u] = 1;

        for (ArcNode *p = g->head[u]; p; p = p->next) {
            cost_compare(1);
            if (dist[u] + p->w < dist[p->to]) {
                dist[p->to] = dist[u] + p->w;
                prev[p->to] = u;
            }
        }
    }
    return PF_OK;
}

int path_backtrack(const int *prev, int src, int dst, int *out, int *outn)
{
    if (!prev || !out || !outn) return PF_PARAM;
    if (dst < 0) { *outn = 0; return PF_NOTFOUND; }

    int tmp[MAXV], n = 0;
    int v = dst;
    while (v != -1 && n < MAXV) {
        tmp[n++] = v;
        if (v == src) break;
        v = prev[v];
    }
    if (tmp[n - 1] != src) { *outn = 0; return PF_NOTFOUND; }

    for (int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    *outn = n;
    return PF_OK;
}

void floyd(int n, int dist[MAXV][MAXV])
{
    if (!dist || n <= 0) return;
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                cost_compare(1);
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];
            }
}

void graph_to_matrix(Graph *g, int m[MAXV][MAXV])
{
    if (!g || !m) return;
    for (int i = 0; i < MAXV; i++)
        for (int j = 0; j < MAXV; j++)
            m[i][j] = (i == j) ? 0 : INF;

    for (int v = 0; v < g->n; v++)
        for (ArcNode *p = g->head[v]; p; p = p->next)
            if (p->w < m[v][p->to]) m[v][p->to] = p->w;
}

int harvester_plan(Graph *g, int from, int to, int *path, int *plen)
{
    if (!g || !path || !plen) return PF_PARAM;
    int dist[MAXV], prev[MAXV];
    if (dijkstra(g, from, dist, prev) != PF_OK) return PF_PARAM;
    if (dist[to] >= INF) { *plen = 0; return PF_NOTFOUND; }
    path_backtrack(prev, from, to, path, plen);
    return dist[to];
}

int harvester_run(Graph *g, int start)
{
    if (!g) return PF_PARAM;
    int total = 0;
    for (int v = 0; v < g->n; v++) {
        if (v == start) continue;
        int path[MAXV], n = 0;
        int c = harvester_plan(g, start, v, path, &n);
        if (c > 0) total += c;
    }
    return total;
}
#endif /* PF_HAVE_PATH */

/* ======================== 第 8 周结束 ==================================== */

/* ============================================================================
 *  【第 9 周】market.c 的临时实现
 *  新建 src/market.c 之后，这一整段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_MARKET

SortStat g_stat = {0, 0, 0};

int cmp_price(const Crop *a, const Crop *b)
{
    if (a->price != b->price) return b->price - a->price;   /* 价格高的在前 */
    return strcmp(a->name, b->name);
}

int cmp_profit(const Crop *a, const Crop *b)
{
    if (a->net_profit != b->net_profit) return b->net_profit - a->net_profit;
    return strcmp(a->name, b->name);
}

/* 每次比较都要记一笔 —— 这就是"游戏时间被吃掉"的来源 */
#define LESS(a, b) (g_stat.cmp++, cost_compare(1), cmp(a, b) < 0)
#define SWAP(a, b) do { Crop _t = *(a); *(a) = *(b); *(b) = _t; \
                        g_stat.mov += 3; cost_move(3); } while (0)

/* TODO(第 9 周)：七个排序，现在的实现全是冒泡 —— 你要一个个换成真的 */
void sort_bubble(Crop *a, int n, CmpFn cmp)
{
    if (!a || n <= 1) return;
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (LESS(&a[j + 1], &a[j])) SWAP(&a[j], &a[j + 1]);
}

/* TODO(第 9 周)：选择排序 —— 每轮选最小的放到前面 */
void sort_select(Crop *a, int n, CmpFn cmp)
{
    if (!a || n <= 1) return;
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (LESS(&a[j], &a[i])) SWAP(&a[i], &a[j]);
}

/* TODO(第 9 周)：插入排序 —— 把 a[i] 插到前面合适位置 */
void sort_insert(Crop *a, int n, CmpFn cmp)
{
    if (!a || n <= 1) return;
    for (int i = 1; i < n; i++)
        for (int j = i; j > 0 && LESS(&a[j], &a[j - 1]); j--)
            SWAP(&a[j], &a[j - 1]);
}

/* TODO(第 9 周)：希尔排序 —— 增量序列 1,4,13,40... */
void sort_shell(Crop *a, int n, CmpFn cmp)
{
    if (!a || n <= 1) return;
    for (int gap = n / 2; gap > 0; gap /= 2)
        for (int i = gap; i < n; i++)
            for (int j = i; j >= gap && LESS(&a[j], &a[j - gap]); j -= gap)
                SWAP(&a[j], &a[j - gap]);
}

/* TODO(第 9 周)：归并排序 —— 要额外 n 空间，但移动次数少 */
void sort_merge(Crop *a, int n, CmpFn cmp)
{
    if (!a || n <= 1) return;
    Crop *tmp = (Crop *)malloc((size_t)n * sizeof(Crop));
    if (!tmp) { sort_bubble(a, n, cmp); return; }
    for (int width = 1; width < n; width *= 2) {
        for (int lo = 0; lo < n - width; lo += width * 2) {
            int mid = lo + width;
            int hi = (lo + width * 2 < n) ? lo + width * 2 : n;
            int i = lo, j = mid, k = lo;
            while (i < mid && j < hi) {
                if (LESS(&a[j], &a[i])) tmp[k++] = a[j++];
                else                    tmp[k++] = a[i++];
            }
            while (i < mid) tmp[k++] = a[i++];
            while (j < hi)  tmp[k++] = a[j++];
            for (int t = lo; t < hi; t++) { a[t] = tmp[t]; g_stat.mov++; cost_move(1); }
        }
    }
    free(tmp);
}

/* TODO(第 9 周)：快排 —— 关键是划分函数，别把 pivot 写死 */
void sort_quick(Crop *a, int n, CmpFn cmp)
{
    if (!a || n <= 1) return;
    int pivot = n / 2;
    Crop pv = a[pivot];
    int i = 0, j = n - 1;
    while (i <= j) {
        while (i < n && LESS(&a[i], &pv)) i++;
        while (j >= 0 && LESS(&pv, &a[j])) j--;
        if (i <= j) { SWAP(&a[i], &a[j]); i++; j--; }
    }
    if (j > 0)   sort_quick(a, j + 1, cmp);
    if (i < n-1) sort_quick(a + i, n - i, cmp);
}

/* TODO(第 9 周)：堆排序 —— 先建大顶堆，再一个个把堆顶换到末尾 */
void sort_heap(Crop *a, int n, CmpFn cmp)
{
    if (!a || n <= 1) return;
    /* 朴素版：先排好序再说（这不是堆排序，你要重写） */
    sort_insert(a, n, cmp);
}

/* 多关键字排序：价格降序 → 生长天数升序 → 名字字典序
 *
 * 返回负数表示 a 排在 b 前面。三个 if 的顺序就是优先级。 */
static int cmp_rank(const Crop *a, const Crop *b)
{
    if (a->price != b->price) return b->price - a->price;              /* 价高的在前 */
    if (a->grow_days != b->grow_days) return a->grow_days - b->grow_days; /* 天数少的在前 */
    return strcmp(a->name, b->name);                                   /* 字典序 */
}

void market_rank(Crop *a, int n)
{
    if (!a || n <= 1) return;

    /* 用插入排序，因为它的"比较"次数少，讲多关键字也最直观 */
    for (int i = 1; i < n; i++) {
        Crop key = a[i];
        int j = i - 1;
        while (j >= 0) {
            g_stat.cmp++;
            cost_compare(1);
            if (cmp_rank(&a[j], &key) <= 0) break;   /* a[j] 本来就该在前面 */
            a[j + 1] = a[j];
            g_stat.mov++; cost_move(1);
            j--;
        }
        a[j + 1] = key;
    }
}

int market_best_crop(const Crop *a, int n, int soil_quality)
{
    if (!a || n <= 0) return -1;
    int best = 0;
    long best_v = -1;
    for (int i = 0; i < n; i++) {
        cost_compare(1);
        long v = (long)a[i].price * soil_quality - (long)a[i].seed_cost;
        if (v > best_v) { best_v = v; best = i; }
    }
    return a[best].id;
}

const char *sort_name(int algo)
{
    switch (algo) {
        case 0: return "bubble";
        case 1: return "select";
        case 2: return "insert";
        case 3: return "shell";
        case 4: return "merge";
        case 5: return "quick";
        case 6: return "heap";
        default: return "?";
    }
}
#endif /* PF_HAVE_MARKET */

/* ======================== 第 9 周结束 ==================================== */

/* ============================================================================
 *  【第 10 周】index.c 的临时实现
 *  新建 src/index.c 之后，这一整段自动失效
 * ========================================================================== */
#ifndef PF_HAVE_INDEX

int hash_init(HashMap *h, int cap)
{
    if (!h || cap <= 0) return PF_PARAM;
    h->bucket = (HashNode **)calloc((size_t)cap, sizeof(HashNode *));
    if (!h->bucket) return PF_NOMEM;
    h->cap = cap;
    h->size = 0;
    return PF_OK;
}

void hash_destroy(HashMap *h)
{
    if (!h) return;
    for (int i = 0; i < h->cap; i++) {
        HashNode *p = h->bucket[i];
        while (p) { HashNode *n = p->next; free(p); p = n; }
    }
    free(h->bucket);
    h->bucket = NULL;
    h->cap = h->size = 0;
}

static unsigned hash_of(int key, int cap)
{
    unsigned k = (unsigned)key;
    k = k * 2654435761u;
    return k % (unsigned)cap;
}

int hash_put(HashMap *h, int key, const Item *val)
{
    if (!h || !val) return PF_PARAM;
    unsigned b = hash_of(key, h->cap);
    for (HashNode *p = h->bucket[b]; p; p = p->next) {
        cost_compare(1);
        if (p->key == key) { p->val = *val; return PF_OK; }
    }
    HashNode *nd = (HashNode *)malloc(sizeof(HashNode));
    if (!nd) return PF_NOMEM;
    nd->key = key;
    nd->val = *val;
    nd->next = h->bucket[b];
    h->bucket[b] = nd;
    h->size++;
    return PF_OK;
}

int hash_get(const HashMap *h, int key, Item *out)
{
    if (!h || !out) return PF_PARAM;
    unsigned b = hash_of(key, h->cap);
    for (HashNode *p = h->bucket[b]; p; p = p->next) {
        cost_compare(1);
        if (p->key == key) { *out = p->val; return PF_OK; }
    }
    return PF_NOTFOUND;
}

int hash_remove(HashMap *h, int key)
{
    if (!h) return PF_PARAM;
    unsigned b = hash_of(key, h->cap);
    HashNode *prev = NULL, *p = h->bucket[b];
    while (p) {
        cost_compare(1);
        if (p->key == key) {
            if (prev) prev->next = p->next;
            else      h->bucket[b] = p->next;
            free(p);
            h->size--;
            return PF_OK;
        }
        prev = p;
        p = p->next;
    }
    return PF_NOTFOUND;
}

/* TODO(第 10 周)：二分查找。注意边界：while (lo <= hi) 还是 (lo < hi) */
int bsearch_by_id(const Item *sorted, int n, int key)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        cost_compare(1);
        if (sorted[mid].crop_id == key) return mid;
        if (sorted[mid].crop_id < key) lo = mid + 1;
        else                           hi = mid - 1;
    }
    return -1;
}

int index_build(Index *ix, const Item *all, int n, int hot_n)
{
    if (!ix || !all || n < 0) return PF_PARAM;
    if (hot_n > n) hot_n = n;

    memset(ix, 0, sizeof(Index));
    if (hash_init(&ix->hot, hot_n * 4 + 8) != PF_OK) return PF_NOMEM;

    for (int i = 0; i < hot_n; i++)
        hash_put(&ix->hot, all[i].crop_id, &all[i]);

    ix->cold_n = n - hot_n;
    if (ix->cold_n > 0) {
        ix->cold = (Item *)malloc((size_t)ix->cold_n * sizeof(Item));
        if (!ix->cold) { hash_destroy(&ix->hot); return PF_NOMEM; }
        for (int i = 0; i < ix->cold_n; i++) ix->cold[i] = all[hot_n + i];
    }
    return PF_OK;
}

int index_lookup(Index *ix, int key, Item *out)
{
    if (!ix || !out) return PF_PARAM;

    if (hash_get(&ix->hot, key, out) == PF_OK) { ix->hit_hot++; return PF_OK; }

    int p = bsearch_by_id(ix->cold, ix->cold_n, key);
    if (p >= 0) { *out = ix->cold[p]; ix->hit_cold++; return PF_OK; }

    ix->miss++;
    return PF_NOTFOUND;
}

void index_destroy(Index *ix)
{
    if (!ix) return;
    hash_destroy(&ix->hot);
    free(ix->cold);
    ix->cold = NULL;
    ix->cold_n = 0;
}

void index_demo_data(Item *out, int n)
{
    if (!out) return;
    for (int i = 0; i < n; i++) {
        out[i].crop_id = i;
        out[i].count = (i * 37) % 200;
        out[i].quality = 1 + (i % 5);
    }
}
#endif /* PF_HAVE_INDEX */

/* ======================== 第 10 周结束 =================================== */
