/* 第 3 周 · 撤销栈 + 订单队列 */
#include "test_util.h"

int main(void)
{
    t_setup();

    /* ---------------- 栈 ---------------- */
    t_begin("1. 栈：初始化");
    UndoStack s;
    CHECK(undo_init(&s, 4) == PF_OK, "undo_init 返回 PF_OK");
    CHECK(s.top == 0, "空栈 top = 0");
    CHECK(s.cap == 4, "容量 4");

    t_begin("2. 栈：压入 / 弹出");
    UndoRec r;
    memset(&r, 0, sizeof r);
    r.action = ACT_TILL; r.x = 1; r.y = 2;
    CHECK(undo_push(&s, &r) == PF_OK, "压入第 1 条");
    r.action = ACT_SOW;
    CHECK(undo_push(&s, &r) == PF_OK, "压入第 2 条");
    CHECK(undo_depth(&s) == 2, "深度 2（实际 %d）", undo_depth(&s));

    UndoRec out;
    CHECK(undo_top(&s, &out) == PF_OK, "看一眼栈顶");
    CHECK(out.action == ACT_SOW, "栈顶是后压的（后进先出）");
    CHECK(undo_depth(&s) == 2, "看一眼不影响深度");

    CHECK(undo_pop(&s, &out) == PF_OK, "弹出");
    CHECK(out.action == ACT_SOW, "弹出的是 SOW");
    CHECK(undo_depth(&s) == 1, "深度变 1");

    t_begin("3. 栈：满了");
    undo_push(&s, &r);
    undo_push(&s, &r);
    undo_push(&s, &r);
    CHECK(undo_depth(&s) == 4, "压满 4 个");
    CHECK(undo_push(&s, &r) == PF_FULL, "第 5 个返回 PF_FULL");

    t_begin("4. 栈：空了");
    while (undo_depth(&s) > 0) undo_pop(&s, &out);
    CHECK(undo_pop(&s, &out) == PF_EMPTY, "空栈弹出返回 PF_EMPTY");
    CHECK(undo_top(&s, &out) == PF_EMPTY, "空栈看栈顶也返回 PF_EMPTY");

    t_begin("5. 撤销真的能撤回操作");
    undo_destroy(&s);
    undo_init(&s, 8);
    g_farm.barn.data = NULL; g_farm.barn.count = 0; g_farm.barn.cap = 0;
    barn_init(&g_farm.barn, 8);

    CHECK(field_till(2, 2) == PF_OK, "先开垦一块地");
    CHECK(plot_at(2, 2)->state == PLOT_TILLED, "现在是已开垦");

    UndoRec ur;
    ur.action = ACT_TILL; ur.x = 2; ur.y = 2; ur.crop_id = -1;
    ur.count = 1; ur.gold = 0; ur.day = 1;
    undo_push(&s, &ur);
    CHECK(undo_apply(&s) == PF_OK, "执行撤销");
    CHECK(plot_at(2, 2)->state == PLOT_WILD, "撤销后变回荒地");
    CHECK(undo_depth(&s) == 0, "撤销后栈空了");
    CHECK(undo_apply(&s) == PF_EMPTY, "没东西可撤了");

    undo_destroy(&s);
    barn_destroy(&g_farm.barn);

    /* ---------------- 队列 ---------------- */
    t_begin("6. 队列：初始化");
    OrderQueue q;
    CHECK(order_init(&q, 4) == PF_OK, "order_init 返回 PF_OK");
    CHECK(order_size(&q) == 0, "空队列 size = 0");

    t_begin("7. 队列：先进先出");
    Order o;
    for (int i = 1; i <= 3; i++) {
        memset(&o, 0, sizeof o);
        o.id = i;
        o.crop_id = i;
        CHECK(order_enqueue(&q, &o) == PF_OK, "入队订单 %d", i);
    }
    CHECK(order_size(&q) == 3, "size = 3");

    Order got;
    CHECK(order_peek(&q, &got) == PF_OK, "看队头");
    CHECK(got.id == 1, "队头是先进去的 1");
    CHECK(order_size(&q) == 3, "peek 不影响 size");

    CHECK(order_dequeue(&q, &got) == PF_OK, "出队");
    CHECK(got.id == 1, "出来的是 1（先进先出）");
    CHECK(order_size(&q) == 2, "size = 2");

    t_begin("8. 队列：循环");
    /* 容量 4，实际能装 3 个（留一格）。已经出了 1 个，再进 2 个应该刚好满 */
    o.id = 4; CHECK(order_enqueue(&q, &o) == PF_OK, "再入队 4");
    o.id = 5; CHECK(order_enqueue(&q, &o) == PF_OK, "再入队 5");
    CHECK(order_size(&q) == 4, "装满了 4 个（实际 %d）", order_size(&q));
    o.id = 6;
    CHECK(order_enqueue(&q, &o) == PF_FULL, "满了返回 PF_FULL");

    CHECK(order_dequeue(&q, &got) == PF_OK, "出一个");
    CHECK(got.id == 2, "出的是 2");
    o.id = 7;
    CHECK(order_enqueue(&q, &o) == PF_OK, "腾出位置后能进 7（说明 front/rear 绕回来了）");

    t_begin("9. 队列：清空");
    int n = order_size(&q), cnt = 0;
    while (order_dequeue(&q, &got) == PF_OK) cnt++;
    CHECK(cnt == n, "全部出队 %d 个", cnt);
    CHECK(order_dequeue(&q, &got) == PF_EMPTY, "空了返回 PF_EMPTY");

    order_destroy(&q);
    CHECK(q.data == NULL, "destroy 之后指针置空");

    return t_end("t03");
}
