/* ============================================================================
 *  include/road.h  ——  第 7 周：路网（邻接表）
 *  老师提供，只读。
 * ========================================================================== */
#ifndef ROAD_H
#define ROAD_H
#include "farm.h"

#define MAXV 64

/* 一条边 */
typedef struct ArcNode {
    int to;                    /* 到哪个顶点 */
    int w;                     /* 权重（米数） */
    struct ArcNode *next;
} ArcNode;

/* 图 */
typedef struct {
    ArcNode *head[MAXV];       /* 每个顶点的边链表 */
    char     name[MAXV][24];
    int      x[MAXV], y[MAXV];
    int      n, edge_n;
} Graph;

int  graph_init(Graph *g, int n);
void graph_destroy(Graph *g);
int  graph_add_edge(Graph *g, int u, int v, int w);

void graph_dfs(Graph *g, int s, int *out, int *pos);
void graph_bfs(Graph *g, int s, int *out, int *pos);
int  graph_min_hops(Graph *g, int s, int t, int *path, int *plen);
int  graph_components(Graph *g, int *comp_of);
int  graph_find_isolated(Graph *g, int *out);
void graph_print(const Graph *g);

/* 引擎用：加载默认路网（12 个设施 + 15 条路） */
void graph_load_default(Graph *g);

#endif /* ROAD_H */
