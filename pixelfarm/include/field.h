/* ============================================================================
 *  include/field.h  ——  第 1 周：农田 + 仓库（顺序表）
 *  老师提供，只读。函数名、参数、返回值都不能改。
 * ========================================================================== */
#ifndef FIELD_H
#define FIELD_H
#include "farm.h"

/* ---------- 地图 ---------- */

/* 取 (x,y) 格子的指针；越界返回 NULL */
Plot *plot_at(int x, int y);

/* 开垦一块荒地。成功 PF_OK，已开垦 PF_PARAM，体力不够 PF_EMPTY */
int field_till(int x, int y);

/* 播种。种子不够 PF_EMPTY，地块没开垦 PF_PARAM */
int field_sow(int x, int y, int crop_id);

/* 浇水。水分加满。越界或荒地 PF_PARAM */
int field_water(int x, int y);

/* 收获。没成熟 PF_PARAM。产出入仓库，返回实际收获件数 */
int field_harvest(int x, int y);

/* 每天开始时调一次：让所有地块长大一天，处理成熟 / 缺水枯萎 */
void field_new_day(void);

/* ---------- 仓库 ---------- */

int  barn_init(Barn *b, int cap);
void barn_destroy(Barn *b);
int  barn_grow(Barn *b);
int  barn_add(Barn *b, int crop_id, int count);
int  barn_remove_at(Barn *b, int pos, Item *out);
int  barn_find(const Barn *b, int crop_id);

/* ---------- 文件 ---------- */

int barn_load_csv(Barn *b, const char *path);
int barn_save_csv(const Barn *b, const char *path);

#endif /* FIELD_H */
