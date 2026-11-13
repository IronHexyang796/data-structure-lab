/* ============================================================================
 *  第 8 周 · 自动收割机（最短路径）      templates/path.c
 *
 *  怎么用：复制到 src/path.c。
 *
 *  依赖第 7 周的图：路网建好了，这一周算"从哪走到哪最省时间"。
 *  先用朴素的 Dijkstra 把功能跑通（O(V²) 也行），再想堆优化。
 * ========================================================================== */
#include "path.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* TODO —— 一条路的代价（秒）
 *   每秒走 4 米，所以 sec = len_m / 4；
 *   石板路（TILE_ROAD）打 8 折；最短算 1 秒。
 */
int edge_cost(int len_m, int road_type)
{
    (void)len_m; (void)road_type;
    return 0;
}

/* TODO —— Dijkstra：单源最短路
 *
 *   1) dist[] 全设 INF，prev[] 全设 -1，dist[src] = 0
 *   2) 循环 n 次：在没有确定过的点里，挑 dist 最小的那个 u
 *   3) 把 u 标记为已确定，然后看它的每条边：
 *         if (dist[u] + p->w < dist[p->to]) 更新 dist 和 prev
 *   返回 PF_OK / PF_PARAM
 *
 *   优化（可选）：把"挑最小"换成第 6 周的堆，就是 O((V+E)logV)。
 */
int dijkstra(Graph *g, int src, int *dist, int *prev)
{
    (void)g; (void)src; (void)dist; (void)prev;
    return PF_PARAM;
}

/* TODO —— 回溯路径：从 dst 顺着 prev 一路往回走，走到 src 为止，
 *   因为是从后往前收集的，最后要倒过来。
 *   返回 PF_OK / PF_NOTFOUND
 */
int path_backtrack(const int *prev, int src, int dst, int *out, int *outn)
{
    (void)prev; (void)src; (void)dst; (void)out; (void)outn;
    return PF_PARAM;
}

/* TODO —— Floyd：三重循环，中间点 k 写在最外层
 *   if (dist[i][k] + dist[k][j] < dist[i][j]) 就更新。
 *   写错顺序（把 k 放最里层）是最常见的错误。
 */
void floyd(int n, int dist[MAXV][MAXV])
{
    (void)n; (void)dist;
}

/* TODO —— 图转邻接矩阵：自己到自己 0，没边 INF，有边取最小的那条 */
void graph_to_matrix(Graph *g, int m[MAXV][MAXV])
{
    (void)g; (void)m;
}

/* TODO —— 给游戏用：规划一条从 from 到 to 的路线
 *   返回总代价；不可达返回 PF_NOTFOUND。
 */
int harvester_plan(Graph *g, int from, int to, int *path, int *plen)
{
    (void)g; (void)from; (void)to; (void)path; (void)plen;
    return PF_PARAM;
}

/* TODO —— 从 start 出发跑遍所有点，把代价加起来
 *   想想：这样算是"每个点单独跑一次 Dijkstra"，能不能一次算完？
 */
int harvester_run(Graph *g, int start)
{
    (void)g; (void)start;
    return 0;
}
