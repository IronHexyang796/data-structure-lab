/* ============================================================================
 *  第 7 周 · 路网（图 / 邻接表）      templates/road.c
 *
 *  怎么用：复制到 src/road.c。
 *
 *  邻接表：head[v] 是一条链表，链表里每个节点表示"从 v 出发能到的下一个点"。
 *  加一条无向边 (u,v,w)，其实是往两条链表里各插一个节点。
 * ========================================================================== */
#include "road.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* TODO —— 初始化：n 个顶点，所有 head 置 NULL，边数归零
 *   返回 PF_OK / PF_PARAM（n 越界）
 */
int graph_init(Graph *g, int n)
{
    memset(g, 0, sizeof(Graph));       /* 先把结构体清零；实现完就删掉这行 */
    (void)n;
    return PF_PARAM;
}

/* TODO —— 销毁：把每条链表上的节点全部 free
 *   注意：先存下 next 再 free 当前节点，不然就断链了。
 */
void graph_destroy(Graph *g)
{
    (void)g;
}

/* TODO —— 加无向边
 *
 *   两次 malloc，两次头插：
 *       a->to = v; a->next = g->head[u]; g->head[u] = a;
 *       b->to = u; b->next = g->head[v]; g->head[v] = b;
 *   万一第二次 malloc 失败，记得把第一次的 free 掉。
 */
int graph_add_edge(Graph *g, int u, int v, int w)
{
    (void)g; (void)u; (void)v; (void)w;
    return PF_PARAM;
}

/* TODO —— DFS：一条路走到黑，走不动了再退回来
 *   vis[] 记"来过没有"，out[] 记访问顺序，(*pos)++ 往后放。
 */
void graph_dfs(Graph *g, int s, int *out, int *pos)
{
    (void)g; (void)s; (void)out; (void)pos;
}

/* TODO —— BFS：一层一层往外扩，用队列
 *   第一次访问到某个点时的层数，就是它到 s 的最少中转次数。
 */
void graph_bfs(Graph *g, int s, int *out, int *pos)
{
    (void)g; (void)s; (void)out; (void)pos;
}

/* TODO —— 最少中转：BFS 的同时记下 prev[]，最后从 t 往回倒
 *   返回中转次数（dist[t]）；不可达返回 PF_NOTFOUND。
 */
int graph_min_hops(Graph *g, int s, int t, int *path, int *plen)
{
    (void)g; (void)s; (void)t; (void)path; (void)plen;
    return PF_PARAM;
}

/* TODO —— 连通分量：对每个没访问过的点做一次 DFS，能扫到的都是同一块
 *   返回一共有几块；comp_of[v] 记 v 属于第几块。
 */
int graph_components(Graph *g, int *comp_of)
{
    (void)g; (void)comp_of;
    return 0;
}

/* TODO —— 找孤岛：一条边都没有的点
 *   返回孤岛个数，下标放进 out[]。
 */
int graph_find_isolated(Graph *g, int *out)
{
    (void)g; (void)out;
    return 0;
}

/* TODO —— 打印整张图：每个点后面跟一串 " 目标(权重)" */
void graph_print(const Graph *g)
{
    (void)g;
}

/* ----------------------------------------------------------------------------
 *  下面是老师给的，不用改：12 个设施 + 15 条路的默认路网
 * -------------------------------------------------------------------------- */
void graph_load_default(Graph *g)
{
    static const char *names[12] = {
        "农场", "谷仓", "水井", "集市", "码头", "磨坊",
        "风车", "牧场", "果园", "蜂房", "工坊", "仓库"
    };
    graph_init(g, 12);
    for (int i = 0; i < 12; i++)
        snprintf(g->name[i], sizeof g->name[i], "%s", names[i]);

    static const int E[15][3] = {
        {0,1,120},{0,2,90},{0,3,300},{1,4,180},{1,5,240},
        {2,5,150},{3,6,210},{3,7,160},{4,8,270},{5,8,190},
        {6,9,130},{7,9,220},{8,10,170},{9,10,140},{10,11,260}
    };
    for (int i = 0; i < 15; i++) graph_add_edge(g, E[i][0], E[i][1], E[i][2]);
}
