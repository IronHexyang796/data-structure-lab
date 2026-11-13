/* ============================================================================
 *  engine/main.c  ——  程序入口，解析命令行参数
 *  老师提供，禁止修改。
 *
 *  ./farm                 玩游戏
 *  ./farm --json          把当前状态输出成 JSON（打印到屏幕）
 *  ./farm --json out.json 写到文件
 *  ./farm --serve         启动网页服务器，浏览器打开 localhost:8080
 *  ./farm --serve 8081    换个端口
 *  ./farm --seed 42       固定随机种子（同样的种子 → 同样的开局）
 *  ./farm --demo          自动玩 30 天，出一份报告（不进入交互）
 *  ./farm --version
 *  ./farm --help
 * ========================================================================== */
#include "engine.h"

#ifdef _WIN32
  #include <windows.h>
#endif

/* 让 Windows 控制台按 UTF-8 显示中文。
 * 不设这一步，--demo、--json 这些不进终端的模式会打出一堆乱码
 * （因为控制台默认是 GBK 936，而源文件里存的是 UTF-8 字节）。 */
static void console_utf8(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
}

static void print_help(void)
{
    printf(
"像素农场 PIXEL FARM  v%s\n"
"\n"
"用法（Windows 上把 farm 写成 farm.exe）：\n"
"  farm                   进游戏（方向键移动，? 看帮助，q 退出）\n"
"  farm --demo            自动玩 30 天，出报告\n"
"  farm --json [文件]     把状态输出成 JSON（不给文件名就打到屏幕）\n"
"  farm --serve [端口]    启动网页服务器（默认 8080）\n"
"  farm --seed N          固定随机种子，方便复现\n"
"  farm --path [图文件]   算全源最短路（对拍用）\n"
"  farm --version\n"
"  farm --help\n"
"\n"
"平时不用手敲这些：在工程根目录敲  pf run / pf demo / pf serve 就行。\n"
"\n", PF_VERSION);
}

/* ---------------------------------------------------------------- 自动跑 30 天 */
static int run_demo(void)
{
    printf("自动演示：跑 30 天\n");
    printf("--------------------------------------------------\n");

    for (int d = 0; d < 30; d++) {
        /* 每天：能种的都种上，都浇水 */
        for (int y = 0; y < WORLD_H; y++) {
            for (int x = 0; x < WORLD_W; x++) {
                Plot *p = &g_farm.plots[y][x];
                if (p->state == PLOT_WILD) {
                    if (field_till(x, y) == PF_OK) {
                        field_sow(x, y, g_crops[0].id);
                        field_water(x, y);
                    }
                } else if (p->state == PLOT_RIPE) {
                    field_harvest(x, y);
                } else if (p->state == PLOT_TILLED) {
                    field_sow(x, y, g_crops[0].id);
                    field_water(x, y);
                }
            }
        }
        /* 有订单就交，交不了就按市价卖掉 */
        for (int i = 0; i < 3; i++) ui_do_order();
        ui_do_sell();

        /* 每天看一次行情：这一步会调用排序，是"计算代价"的主要来源 */
        {
            Crop tmp[MAX_CROPS];
            memcpy(tmp, g_crops, sizeof(Crop) * (size_t)g_crop_n);
            market_rank(tmp, g_crop_n);
        }

        clock_new_day();
    }

    printf("第 %d 天结束\n", g_farm.day);
    printf("  金币：      %d.%02d 元\n", g_farm.gold / 100, g_farm.gold % 100);
    printf("  开垦：      %d / %d 块\n", g_farm.till_count, WORLD_W * WORLD_H);
    printf("  仓库：      %d 种\n", g_farm.barn.count);
    printf("  比较次数：  %ld\n", g_farm.cmp_count);
    printf("  移动次数：  %ld\n", g_farm.move_count);
    printf("  计算代价：  %.1f 分钟游戏时间\n", g_farm.cost_seconds / 60.0);
    printf("--------------------------------------------------\n");
    return 0;
}

int main(int argc, char **argv)
{
    unsigned seed = 20250101u;

    console_utf8();

    /* 先扫一遍 --seed，因为初始化时就要用 */
    for (int i = 1; i < argc - 1; i++)
        if (!strcmp(argv[i], "--seed")) seed = (unsigned)atoi(argv[i + 1]);

    if (argc >= 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
        print_help();
        return 0;
    }
    if (argc >= 2 && !strcmp(argv[1], "--version")) {
        printf("pixelfarm %s\n", PF_VERSION);
        return 0;
    }

    game_boot(seed);

    /* ---------------- --json ---------------- */
    if (argc >= 2 && !strcmp(argv[1], "--json")) {
        if (argc >= 3) {
            dump_state_json(argv[2]);
            fprintf(stderr, "已写入 %s\n", argv[2]);
        } else {
            dump_state_stdout();
        }
        return 0;
    }

    /* ---------------- --serve ---------------- */
    if (argc >= 2 && !strcmp(argv[1], "--serve")) {
        int port = (argc >= 3) ? atoi(argv[2]) : 8080;
        return serve_forever(port);
    }

    /* ---------------- --path：最短路对拍 ---------------- */
    if (argc >= 2 && !strcmp(argv[1], "--path")) {
        Graph g;
        const char *file = (argc >= 3) ? argv[2] : NULL;

        if (file) {
            FILE *f = fopen(file, "r");
            if (!f) { printf("打不开 %s\n", file); return 1; }
            int n = 0;
            if (fscanf(f, "%d", &n) != 1 || n <= 1 || n > MAXV) {
                printf("图文件第一行应该是点数（2..%d）\n", MAXV);
                fclose(f);
                return 1;
            }
            graph_init(&g, n);
            int u, v, w;
            while (fscanf(f, "%d %d %d", &u, &v, &w) == 3)
                graph_add_edge(&g, u, v, w);
            fclose(f);
        } else {
            graph_load_default(&g);
        }

        int m[MAXV][MAXV];
        graph_to_matrix(&g, m);
        floyd(g.n, m);

        /* 输出格式固定，tools/brute_path.py 按这个格式对拍 */
        printf("%d\n", g.n);
        for (int i = 0; i < g.n; i++)
            for (int j = 0; j < g.n; j++) {
                if (i == j) continue;
                if (m[i][j] >= INF) printf("dist %d %d INF\n", i, j);
                else                printf("dist %d %d %d\n", i, j, m[i][j]);
            }
        graph_destroy(&g);
        return 0;
    }

    /* ---------------- --demo ---------------- */
    if (argc >= 2 && !strcmp(argv[1], "--demo")) {
        return run_demo();
    }

    /* ---------------- 正常进游戏 ---------------- */
    if (argc >= 2 && argv[1][0] != '-') {
        printf("不认识的参数：%s\n\n", argv[1]);
        print_help();
        return 1;
    }

    term_init();
    game_loop();
    term_restore();

    printf("\n  存档已保存到 save/farm.sav\n");
    printf("  第 %d 天，金币 %d.%02d 元\n\n",
           g_farm.day, g_farm.gold / 100, g_farm.gold % 100);
    return 0;
}
