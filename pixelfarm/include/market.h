/* ============================================================================
 *  include/market.h  ——  第 9 周：市场行情（七种排序）
 *  老师提供，只读。
 *
 *  注意：Crop 定义在 farm.h 里（第 1 周就有了），这里不再重复定义。
 *        下面是它的字段，方便你写排序时对照：
 *          int  id;  char name[20];  int price;      // 今日收购价（分/斤）
 *          int  grow_days;  int seed_cost;  int demand;  int net_profit;
 * ========================================================================== */
#ifndef MARKET_H
#define MARKET_H
#include "farm.h"

typedef struct { long cmp, mov; double ms; } SortStat;
extern SortStat g_stat;

/* 比较函数类型：a 排在 b 前面就返回负数 */
typedef int (*CmpFn)(const Crop *a, const Crop *b);

/* 比较函数（已给，直接用） */
int cmp_price (const Crop *a, const Crop *b);   /* 价格降序 */
int cmp_profit(const Crop *a, const Crop *b);   /* 净收益降序 */

/* 七个排序，全部要自己写 */
void sort_bubble(Crop *a, int n, CmpFn cmp);
void sort_select(Crop *a, int n, CmpFn cmp);
void sort_insert(Crop *a, int n, CmpFn cmp);
void sort_shell (Crop *a, int n, CmpFn cmp);
void sort_merge (Crop *a, int n, CmpFn cmp);
void sort_quick (Crop *a, int n, CmpFn cmp);
void sort_heap  (Crop *a, int n, CmpFn cmp);

/* 多关键字：价格降序 → 生长天数升序 → 名字字典序 */
void market_rank(Crop *a, int n);

/* 给游戏用：算出今天这块地该种什么，返回作物 id */
int market_best_crop(const Crop *a, int n, int soil_quality);

/* 引擎用：按 id 找排序算法名 */
const char *sort_name(int algo);

#endif /* MARKET_H */
