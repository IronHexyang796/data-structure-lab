/* 第 7 周 · 路网（邻接表） */
#include "test_util.h"

int main(void)
{
    t_setup();

    t_begin("1. 建图");
    Graph g;
    CHECK(graph_init(&g, 6) == PF_OK, "graph_init 返回 PF_OK");
    CHECK(g.n == 6, "6 个顶点");
    CHECK(g.edge_n == 0, "还没有边");

    /*  0 --1-- 1 --1-- 2
     *  |               |
     *  1               1
     *  |               |
     *  3 -----1------ 4      5 是孤立的
     */
    graph_add_edge(&g, 0, 1, 10);
    graph_add_edge(&g, 1, 2, 10);
    graph_add_edge(&g, 2, 4, 10);
    graph_add_edge(&g, 4, 3, 10);
    graph_add_edge(&g, 3, 0, 10);
    CHECK(g.edge_n == 5, "5 条边（实际 %d）", g.edge_n);

    t_begin("2. 度数（无向图要插两次）");
    int deg0 = 0;
    for (ArcNode *p = g.head[0]; p; p = p->next) deg0++;
    CHECK(deg0 == 2, "顶点 0 的度是 2（实际 %d）", deg0);

    t_begin("3. DFS");
    int out[MAXV], pos = 0;
    graph_dfs(&g, 0, out, &pos);
    CHECK(pos == 5, "从 0 出发能走到 5 个点（实际 %d）", pos);
    CHECK(out[0] == 0, "第一个是起点 0");

    t_begin("4. BFS");
    pos = 0;
    graph_bfs(&g, 0, out, &pos);
    CHECK(pos == 5, "BFS 也走 5 个");
    CHECK(out[0] == 0, "从 0 开始");
    /* BFS 第一层是 0 的邻居 */
    CHECK(out[1] == 1 || out[1] == 3, "第二个是 0 的邻居（实际 %d）", out[1]);

    t_begin("5. 连通分量");
    int comp[MAXV];
    int c = graph_components(&g, comp);
    CHECK(c == 2, "2 个连通分量（大圈 + 孤立的 5）（实际 %d）", c);
    CHECK(comp[0] == comp[3], "0 和 3 在同一分量");

    t_begin("6. 孤立点");
    int iso[MAXV];
    int n = graph_find_isolated(&g, iso);
    CHECK(n == 1, "1 个孤立点（实际 %d）", n);
    CHECK(n >= 1 && iso[0] == 5, "孤立的是 5");

    t_begin("7. 最少跳数");
    int path[MAXV], plen = 0;
    int hops = graph_min_hops(&g, 0, 4, path, &plen);
    CHECK(hops == 2, "0 到 4 最少 2 跳（实际 %d）", hops);
    CHECK(plen == 3, "路径上有 3 个点");
    CHECK(path[0] == 0 && path[plen - 1] == 4, "起点终点对得上");

    CHECK(graph_min_hops(&g, 0, 5, path, &plen) < 0, "到不了 5，返回负数");

    graph_destroy(&g);
    CHECK(g.n == 0, "destroy 之后 n = 0");

    t_begin("8. 默认路网（引擎给的那张）");
    Graph d;
    graph_load_default(&d);
    CHECK(d.n == 12, "12 个设施（实际 %d）", d.n);
    CHECK(d.edge_n == 15, "15 条路（实际 %d）", d.edge_n);
    CHECK(graph_components(&d, comp) == 1, "整张网是连通的");

    pos = 0;
    graph_dfs(&d, 0, out, &pos);
    CHECK(pos == 12, "从农场出发能走遍全部 12 个设施（实际 %d）", pos);

    graph_destroy(&d);
    return t_end("t07");
}
