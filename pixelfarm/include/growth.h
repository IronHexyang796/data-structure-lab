/* ============================================================================
 *  include/growth.h  ——  第 2 周：温室生长队列 + 操作日志（单链表）
 *  老师提供，只读。
 * ========================================================================== */
#ifndef GROWTH_H
#define GROWTH_H
#include "farm.h"

/* 温室节点 */
typedef struct GNode {
    int crop_id;
    int ripe_day;          /* 预计成熟日，排序依据 */
    int x, y;              /* 种在哪块地 */
    struct GNode *next;
} GNode;

typedef struct {
    GNode *head;           /* 哨兵 */
    int    size;           /* 真节点个数（不含哨兵） */
} GrowthList;

/* 日志节点 */
typedef struct LNode {
    int day;
    int action;            /* 见 ACT_* 常量 */
    char detail[64];
    struct LNode *next;
} LNode;

typedef struct {
    LNode *head;           /* 哨兵 */
    LNode *tail;           /* 尾指针，加速追加 */
    int    size;
} LogList;

/* ---------- 要在 src/growth.c 里实现的 ---------- */
int  growth_init(GrowthList *g);
void growth_destroy(GrowthList *g);
int  growth_insert(GrowthList *g, int crop_id, int ripe_day, int x, int y);
int  growth_pop_earliest(GrowthList *g, GNode *out);
int  growth_remove(GrowthList *g, int crop_id);

int  log_init(LogList *L);
void log_destroy(LogList *L);
int  log_append(LogList *L, int day, int action, const char *detail);
void log_reverse(LogList *L);

/* ---------- 引擎用 ---------- */
const char *action_name(int action);

#endif /* GROWTH_H */
