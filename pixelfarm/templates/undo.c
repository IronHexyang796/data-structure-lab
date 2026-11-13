/* ============================================================================
 *  第 3 周 · 撤销栈（顺序栈）      templates/undo.c
 *
 *  怎么用：复制到 src/undo.c。
 *          注意另有一个 src/order.c（订单队列），两个文件都建了，
 *          stub.c 里第 3 周那段才会整体失效。
 *
 *  栈顶在 top-1，top 同时等于"元素个数"和"下一个空位的下标"。
 * ========================================================================== */
#include "undo.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* TODO —— 初始化：借 cap 个 UndoRec 的内存
 *   返回 PF_OK / PF_PARAM / PF_NOMEM
 */
int undo_init(UndoStack *s, int cap)
{
    memset(s, 0, sizeof(UndoStack));   /* 先把结构体清零；实现完就删掉这行 */
    (void)cap;
    return PF_PARAM;
}

/* TODO —— 销毁：还内存，指针置 NULL，top/cap 归零 */
void undo_destroy(UndoStack *s)
{
    (void)s;
}

/* TODO —— 入栈：能不能先判断"满没满"？
 *   top >= cap 就是满了，返回 PF_FULL（不要覆盖老记录）
 */
int undo_push(UndoStack *s, const UndoRec *r)
{
    (void)s; (void)r;
    return PF_PARAM;
}

/* TODO —— 出栈：top-- 之后再取，或者先取再 top--
 *   顺序搞反了会取到没用的那个格子。空栈返回 PF_EMPTY
 */
int undo_pop(UndoStack *s, UndoRec *out)
{
    (void)s; (void)out;
    return PF_PARAM;
}

/* TODO —— 只看看栈顶，不弹出（讲课时用） */
int undo_top(const UndoStack *s, UndoRec *out)
{
    (void)s; (void)out;
    return PF_PARAM;
}

/* TODO —— 还能撤几步 = 栈里几个元素 */
int undo_depth(const UndoStack *s)
{
    (void)s;
    return 0;
}

/* TODO —— 撤销一步：弹出栈顶，然后把那一步"反着做一遍"
 *
 *   对照 farm.h 里的 ACT_* 常量：
 *     ACT_TILL    把地块改回荒地，金币加回来
 *     ACT_SOW     把地块改回已开垦，金币加回来
 *     ACT_WATER   水分改回 30
 *     ACT_HARVEST 金币减掉
 *     ACT_SELL    东西放回仓库，金币减掉
 *   撤销本身也要花游戏时间（advance_time），想想为什么。
 */
int undo_apply(UndoStack *s)
{
    (void)s;
    return PF_PARAM;
}
