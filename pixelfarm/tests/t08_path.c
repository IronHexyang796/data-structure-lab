/* 第 8 周 · 最短路（Dijkstra / Floyd） */
#include "test_util.h"

int main(void)
{
    t_setup();

    /* 一张小图：
     *
     *       (10)      (5)
     *   0 -------- 1 ------ 2
     *    \        /         |
     *  (30)\  (2)/       (1)|
     *      \    /           |
     *       3 ------------- 4
     *            (5)
     *
     * 0→2 最短：0-1-2 = 15（不是 0-3-4-2 = 36）
     */
    Graph g;
    graph_init(&g, 5);
    graph_add_edge(&g, 0, 1, 10);
    graph_add_edge(&g, 1, 2, 5);
    graph_add_edge(&g, 0, 3, 30);
    graph_add_edge(&g, 1, 3, 2);
    graph_add_edge(&g, 3, 4, 5);
    graph_add_edge(&g, 2, 4, 1);

    t_begin("1. Dijkstra 求最短路");
    int dist[MAXV], prev[MAXV];
    CHECK(dijkstra(&g, 0, dist, prev) == PF_OK, "dijkstra 跑成功");
    CHECK(dist[0] == 0, "到自己是 0");
    CHECK(dist[1] == 10, "0→1 是 10（实际 %d）", dist[1]);
    CHECK(dist[2] == 15, "0→2 是 15（实际 %d，注意不是走 3-4）", dist[2]);
    CHECK(dist[3] == 12, "0→3 是 12（实际 %d，0-1-3 比 0-3 近）", dist[3]);
    CHECK(dist[4] == 16, "0→4 是 16（实际 %d，0-1-2-4）", dist[4]);

    t_begin("2. 回溯路径");
    int path[MAXV], plen = 0;
    CHECK(path_backtrack(prev, 0, 2, path, &plen) == PF_OK, "回溯 0→2");
    CHECK(plen == 3, "路径长度 3（实际 %d）", plen);
    CHECK(path[0] == 0, "起点 0");
    CHECK(path[1] == 1, "中间是 1");
    CHECK(path[2] == 2, "终点 2");

    t_begin("3. 到不了的点");
    Graph iso;
    graph_init(&iso, 3);
    graph_add_edge(&iso, 0, 1, 5);
    /* 2 是孤立的 */
    dijkstra(&iso, 0, dist, prev);
    CHECK(dist[2] == INF, "到不了的点 dist = INF");
    CHECK(path_backtrack(prev, 0, 2, path, &plen) != PF_OK, "回溯也失败");
    graph_destroy(&iso);

    t_begin("4. 邻接矩阵 + Floyd");
    int m[MAXV][MAXV];
    graph_to_matrix(&g, m);
    CHECK(m[0][1] == 10, "矩阵里 A[0][1] = 10");
    CHECK(m[0][2] == INF, "没有直连就是 INF");
    CHECK(m[0][0] == 0, "自己到自己 0");

    floyd(5, m);
    CHECK(m[0][2] == 15, "Floyd 之后 0→2 = 15（实际 %d）", m[0][2]);
    CHECK(m[0][4] == 16, "Floyd 之后 0→4 = 16（实际 %d）", m[0][4]);
    CHECK(m[0][3] == 12, "Floyd 之后 0→3 = 12（实际 %d）", m[0][3]);

    t_begin("5. 两种算法结果一致");
    /* 上一段用 iso 图覆盖过 dist，这里重新对 g 算一次 */
    dijkstra(&g, 0, dist, prev);
    int same = 1;
    for (int j = 0; j < 5; j++)
        if (m[0][j] != dist[j]) same = 0;
    CHECK(same, "Dijkstra 和 Floyd 算出来的距离一致");

    t_begin("6. 边权换算");
    int c1 = edge_cost(400, TILE_GRASS);
    int c2 = edge_cost(400, TILE_ROAD);
    CHECK(c1 > 0, "400 米土路要 %d 秒", c1);
    CHECK(c2 < c1, "石板路 %d 秒 < 土路 %d 秒（好路更快）", c2, c1);

    graph_destroy(&g);

    t_begin("7. 自动收割机（引擎给的路网）");
    Graph d;
    graph_load_default(&d);
    int route[MAXV], rlen = 0;
    int cost = harvester_plan(&d, 0, 11, route, &rlen);
    CHECK(cost > 0, "农场→仓库 代价 %d", cost);
    CHECK(rlen >= 2, "路径至少 2 个点（实际 %d）", rlen);
    CHECK(route[0] == 0 && route[rlen - 1] == 11, "起点终点对");

    int all = harvester_run(&d, 0);
    CHECK(all > 0, "跑遍全部设施总代价 %d", all);
    graph_destroy(&d);

    return t_end("t08");
}
