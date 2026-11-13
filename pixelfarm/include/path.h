/* ============================================================================
 *  include/path.h  ——  第 8 周：最短路（Dijkstra / Floyd）
 *  老师提供，只读。
 * ========================================================================== */
#ifndef PATH_H
#define PATH_H
#include "farm.h"
#include "road.h"

#define INF 0x3f3f3f3f

/* 单源最短路。dist[i] 存 src 到 i 的最短时间，prev[i] 存前驱 */
int dijkstra(Graph *g, int src, int *dist, int *prev);

/* 回溯路径。out 存顶点序列（从 src 到 dst），outn 存长度 */
int path_backtrack(const int *prev, int src, int dst, int *out, int *outn);

/* 全源最短路。dist 是 n×n 的二维数组，进来时是邻接矩阵 */
void floyd(int n, int dist[MAXV][MAXV]);

/* 把图转成邻接矩阵（Floyd 要用） */
void graph_to_matrix(Graph *g, int m[MAXV][MAXV]);

/* 边权换算：土路打折，石板路更快 */
int edge_cost(int len_m, int road_type);

/* 给游戏用 */
int harvester_plan(Graph *g, int from, int to, int *path, int *plen);
int harvester_run(Graph *g, int start);

#endif /* PATH_H */
