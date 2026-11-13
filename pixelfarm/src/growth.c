/* ============================================================================
 *  第 2 周 · 温室队列 + 操作日志（单链表）      templates/growth.c
 *
 *  怎么用：复制到 src/growth.c，按任务书一个一个填 TODO。
 *          src/growth.c 一出现，src/stub.c 里第 2 周那段就自动失效。
 *
 *  这一周的关键词是"只改指针，不挪数据"。
 *  growth_* 是温室队列（按成熟日有序），log_* 是操作日志（尾插单链表）。
 * ========================================================================== */
#include "growth.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* ============================================================================
 *  一、温室队列：一条按 ripe_day 从小到大排好序的链表
 *
 *  GrowthList.head 是哨兵节点（dummy），它自己不存数据，
 *  这样"删第一个"和"删中间"就能用同一套代码，不用特判。
 * ========================================================================== */

/* TODO —— 初始化：建哨兵，size 归零
 *   返回 PF_OK / PF_NOMEM
 */
int growth_init(GrowthList *g)
{
    memset(g, 0, sizeof(GrowthList));  /* 先把结构体清零；实现完就删掉这行 */
    return PF_PARAM;
}

/* TODO —— 销毁：从头到尾 free 掉所有节点（哨兵也要 free） */
void growth_destroy(GrowthList *g)
{
    (void)g;
}

/* TODO —— 有序插入：找到第一个 ripe_day 比它大的节点，插在它前面
 *
 *   遍历时用两个指针：pre 指向"前一个"，cur 指向"当前"。
 *   找到位置后：
 *       node->next = cur;
 *       pre->next  = node;
 *   注意：先接 node->next，再接 pre->next，顺序反了会丢链。
 */
int growth_insert(GrowthList *g, int crop_id, int ripe_day, int x, int y)
{
    (void)g; (void)crop_id; (void)ripe_day; (void)x; (void)y;
    return PF_PARAM;
}

/* TODO —— 取最早熟的那一个：因为链表有序，就是哨兵后面的第一个
 *
 *   O(1)，不用扫描。取出来后要把这个节点摘掉并 free。
 *   返回 PF_OK / PF_EMPTY
 */
int growth_pop_earliest(GrowthList *g, GNode *out)
{
    (void)g; (void)out;
    return PF_PARAM;
}

/* TODO —— 按 crop_id 删除第一个匹配的节点
 *   返回 PF_OK / PF_NOTFOUND
 */
int growth_remove(GrowthList *g, int crop_id)
{
    (void)g; (void)crop_id;
    return PF_PARAM;
}

/* ============================================================================
 *  二、操作日志：尾插的单链表（因为有 tail 指针，追加是 O(1)）
 * ========================================================================== */

/* TODO —— 初始化：建哨兵，head = tail = 哨兵，size = 0 */
int log_init(LogList *L)
{
    memset(L, 0, sizeof(LogList));     /* 先把结构体清零；实现完就删掉这行 */
    return PF_PARAM;
}

/* TODO —— 销毁：全部 free */
void log_destroy(LogList *L)
{
    (void)L;
}

/* TODO —— 追加一条：新节点接在 tail 后面，然后 tail 前移
 *   detail 是字符串，用 snprintf 复制进节点里，别直接赋指针。
 *   返回 PF_OK / PF_NOMEM
 */
int log_append(LogList *L, int day, int action, const char *detail)
{
    (void)L; (void)day; (void)action; (void)detail;
    return PF_PARAM;
}

/* TODO —— 原地反转整条链表（不许新建数组、不许递归）
 *
 *   标准三指针：pre = NULL; cur = head->next;
 *   while (cur) { next = cur->next; cur->next = pre; pre = cur; cur = next; }
 *   最后 head->next = pre;  并把 tail 重新指到原来的第一个节点。
 */
void log_reverse(LogList *L)
{
    (void)L;
}
