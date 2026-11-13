/* ============================================================================
 *  include/undo.h  ——  第 3 周：撤销（顺序栈）
 *  老师提供，只读。
 * ========================================================================== */
#ifndef UNDO_H
#define UNDO_H
#include "farm.h"

typedef struct {
    UndoRec *data;
    int top;        /* 下一个空位（也是元素个数） */
    int cap;
} UndoStack;

int  undo_init(UndoStack *s, int cap);
void undo_destroy(UndoStack *s);
int  undo_push(UndoStack *s, const UndoRec *r);
int  undo_pop(UndoStack *s, UndoRec *out);
int  undo_apply(UndoStack *s);
int  undo_depth(const UndoStack *s);   /* 还能撤几步 */

/* 看一眼栈顶但不弹出（讲课时用）。空栈返回 PF_EMPTY */
int  undo_top(const UndoStack *s, UndoRec *out);

#endif /* UNDO_H */
