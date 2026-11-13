/* ============================================================================
 *  include/farm.h  ——  像素农场 · 公共定义
 *
 *  老师提供，禁止修改。所有模块（学生的 10 个 .c、引擎、测试）都包含这个头文件。
 *  如果你改了这里，你的程序和别人的对不上，第 10 周就拼不起来。
 * ========================================================================== */
#ifndef FARM_H
#define FARM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------- 版本 */
#define PF_VERSION "1.1.0"

/* ---------------------------------------------------------------- 错误码 */
#define PF_OK         0     /* 成功 */
#define PF_PARAM     -1     /* 参数不对（越界、状态不符） */
#define PF_EMPTY     -2     /* 空的 / 不够（没东西可拿、体力不足） */
#define PF_FULL      -3     /* 满了（仓库满、栈满） */
#define PF_NOMEM     -4     /* 内存分配失败 */
#define PF_NOTFOUND  -5     /* 没找到 */

/* ---------------------------------------------------------------- 地图 */
#define WORLD_W   6         /* 农场是 6×6，共 36 块地 */
#define WORLD_H   6
#define MAX_CROPS 64        /* 作物表最多 64 种（实际给 40 种） */
#define BARN_CAP_INIT 8     /* 仓库初始容量。故意给小，第 1 周就会撞上扩容 */

/* ---------------------------------------------------------------- 地块状态 */
typedef enum {
    PLOT_WILD   = 0,        /* 荒地，还没开垦 */
    PLOT_TILLED,            /* 已开垦，可以播种 */
    PLOT_SEEDED,            /* 已播种，还没发芽 */
    PLOT_GROWING,           /* 生长中 */
    PLOT_RIPE,              /* 成熟，可以收 */
    PLOT_DEAD               /* 枯萎（缺水太久） */
} PlotState;

/* ---------------------------------------------------------------- 地块 */
typedef struct {
    int       x, y;         /* 坐标：0..5 */
    int       crop_id;      /* 种了什么，-1 表示空 */
    PlotState state;
    int       planted_day;  /* 第几天种的 */
    int       ripe_day;     /* 预计第几天成熟 */
    int       water;        /* 水分 0~100 */
} Plot;

/* ---------------------------------------------------------------- 仓库物品 */
typedef struct {
    int crop_id;
    int count;
    int quality;            /* 1~5 星 */
} Item;

/* ---------------------------------------------------------------- 仓库（第 1 周的顺序表） */
typedef struct {
    Item *data;             /* 指向内存块 */
    int   count;            /* 现在有几件 */
    int   cap;              /* 最多能放几件 */
} Barn;

/* ---------------------------------------------------------------- 作物（价格单位：分） */
typedef struct {
    int  id;
    char name[20];
    int  price;             /* 今日收购价（分/斤） */
    int  grow_days;         /* 生长周期（天） */
    int  seed_cost;         /* 种子成本（分） */
    int  demand;            /* 今日需求（斤） */
    int  net_profit;        /* 净收益，由 market 模块算出来存这里 */
} Crop;

/* ---------------------------------------------------------------- 订单（第 3 周的队列） */
#define ST_PENDING  0       /* 待接单 */
#define ST_DELIVER  1       /* 配送中 */
#define ST_DONE     2       /* 已完成 */
#define ST_CANCEL   3       /* 已取消 */

typedef struct {
    int id;
    int crop_id;
    int count;
    int price;              /* 成交价（分） */
    int status;
    int day;
} Order;

/* ---------------------------------------------------------------- 撤销记录（第 3 周的栈） */
#define ACT_TILL     1      /* 开垦 */
#define ACT_SOW      2      /* 播种 */
#define ACT_WATER    3      /* 浇水 */
#define ACT_HARVEST  4      /* 收获 */
#define ACT_SELL     5      /* 卖出 */

typedef struct {
    int action;             /* ACT_* */
    int x, y;               /* 作用的地块（不是地块操作就填 -1） */
    int crop_id;
    int count;
    int gold;               /* 这一步花了/赚了多少金币（撤销时要反向） */
    int day;
} UndoRec;

/* ---------------------------------------------------------------- 引擎时间折算
 *
 *  这是整门课的核心机制：
 *  你在代码里数的"比较次数"和"移动次数"，会真的变成游戏里流逝的时间。
 *  1 次比较 = 0.05 秒，1 次移动 = 0.10 秒。
 *  所以 O(n²) 的排序会让你在游戏里少收一轮菜。这不是比喻，是真的扣时间。
 */
#define SEC_PER_COMPARE 0.05
#define SEC_PER_MOVE    0.10
#define SEC_PER_MALLOC  2.00

/* ---------------------------------------------------------------- 全局农场状态 */
typedef struct {
    int      day;           /* 第几天，从 1 开始 */
    int      hour;          /* 0..23，由 day_seconds 算出来，只用来显示 */
    int      minute;        /* 0..59 */
    int      day_seconds;   /* 今天已经过了多少游戏秒（0 = 早上 6:00） */
    int      gold;          /* 金币（单位：分） */
    int      stamina;       /* 体力 0..100 */
    unsigned seed;          /* 随机种子，--seed 42 可复现 */

    Plot     plots[WORLD_W][WORLD_H];
    Barn     barn;
    int      till_count;    /* 已开垦的地块数 */

    /* 统计：玩家这一局一共付了多少"计算代价" */
    long     cmp_count;
    long     move_count;
    long     malloc_count;
    double   cost_seconds;  /* 累计折算出的游戏秒数 */

    /* 屏幕底部那行提示 */
    int      toast_timer;
    char     toast_msg[128];

    int      quit;          /* 置 1 就让主循环退出 */
} Farm;

/* ---------------------------------------------------------------- 全局变量 */
extern Farm  g_farm;                /* 整个农场的数据，随时可读 */
extern Crop  g_crops[MAX_CROPS];    /* 作物表，启动时从 data/crops.csv 读 */
extern int   g_crop_n;              /* 实际读进来几种 */

/* ---------------------------------------------------------------- 引擎提供的函数（你不用实现） */

/* 游戏时间前进 sec 秒。开垦、扩容这类"一整件事"调用一次 */
void advance_time(int sec);

/* 记 n 次比较，折算成游戏时间（每次 0.05 秒）。每次比较后调用 */
void cost_compare(int n);

/* 记 n 次移动（挪数据），折算成游戏时间（每次 0.10 秒）。每次 memmove 后调用 */
void cost_move(int n);

/* 记一次 malloc，折算成游戏时间（每次 2 秒） */
void cost_malloc(int n);

/* 屏幕底部弹一行提示。fmt 和 printf 一样 */
void toast(const char *fmt, ...);

/* 按 id 找作物，找不到返回 NULL */
const Crop *crop_by_id(int id);

/* 0..n-1 的随机数（用 g_farm.seed，可复现） */
int  pf_rand(int n);
void pf_srand(unsigned s);

/* 当前游戏时间换算成总分钟数，方便算"这一天还剩多少时间" */
int  game_minutes(void);

/* 把作物名转成安全的文件名/CSV 字段（去掉逗号引号） */
void safe_name(char *dst, const char *src, int max);

/* ---------------------------------------------------------------- 各模块的实例（引擎持有） */
struct GrowthList_tag;
struct LogList_tag;
struct UndoStack_tag;
struct OrderQueue_tag;
struct SparseMat_tag;
struct HuffTree_tag;
struct Graph_tag;
struct Index_tag;

/* 第 2 周起用：引擎里那个唯一的链表实例 */
void *eng_growth(void);
void *eng_log(void);
/* 第 3 周 */
void *eng_undo(void);
void *eng_order(void);
/* 第 4 周 */
void *eng_map(void);
/* 第 6 周 */
void *eng_huff(void);
/* 第 7 周 */
void *eng_graph(void);
/* 第 10 周 */
void *eng_index(void);

#endif /* FARM_H */
