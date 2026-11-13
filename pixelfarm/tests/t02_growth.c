/* 第 2 周 · 温室队列 + 操作日志（单链表） */
#include "test_util.h"

int main(void)
{
    t_setup();

    t_begin("1. 初始化");
    GrowthList g;
    CHECK(growth_init(&g) == PF_OK, "growth_init 返回 PF_OK");
    CHECK(g.size == 0, "空链表 size = 0");
    CHECK(g.head != NULL, "有哨兵，head 不是 NULL");

    t_begin("2. 插入");
    growth_insert(&g, 0, 5, 1, 1);
    growth_insert(&g, 1, 3, 2, 2);
    growth_insert(&g, 2, 7, 3, 3);
    CHECK(g.size == 3, "插了 3 个，size = 3（实际 %d）", g.size);

    t_begin("3. 取最早熟的");
    GNode out;
    int rc = growth_pop_earliest(&g, &out);
    CHECK(rc == PF_OK, "取出来了");
    CHECK(out.ripe_day == 3, "最早熟的是第 3 天（实际 %d）", out.ripe_day);
    CHECK(out.crop_id == 1, "是 crop 1");
    CHECK(g.size == 2, "取走一个，size = 2");

    CHECK(growth_pop_earliest(&g, &out) == PF_OK, "再取一个");
    CHECK(out.ripe_day == 5, "这次是第 5 天");
    CHECK(growth_pop_earliest(&g, &out) == PF_OK, "再取一个");
    CHECK(out.ripe_day == 7, "最后是第 7 天");

    t_begin("4. 空链表取");
    CHECK(growth_pop_earliest(&g, &out) == PF_EMPTY, "空了返回 PF_EMPTY");

    t_begin("5. 按作物删除");
    growth_insert(&g, 5, 4, 0, 0);
    growth_insert(&g, 6, 6, 0, 0);
    CHECK(growth_remove(&g, 5) == PF_OK, "删得掉 crop 5");
    CHECK(g.size == 1, "size 变 1");
    CHECK(growth_remove(&g, 5) == PF_NOTFOUND, "再删返回 PF_NOTFOUND");

    growth_destroy(&g);
    CHECK(g.head == NULL || g.size == 0, "destroy 之后是空的");

    t_begin("6. 操作日志");
    LogList L;
    CHECK(log_init(&L) == PF_OK, "log_init 返回 PF_OK");
    for (int i = 1; i <= 5; i++) {
        char buf[32];
        snprintf(buf, sizeof buf, "第 %d 天", i);
        log_append(&L, i, ACT_TILL, buf);
    }
    CHECK(L.size == 5, "写了 5 条（实际 %d）", L.size);

    t_begin("7. 日志倒序");
    log_reverse(&L);
    CHECK(L.size == 5, "倒序之后条数不变");
    /* 倒序后第一条应该是第 5 天（如果你的实现维护了链表，这里会检查首节点） */
    if (L.head && L.head->next) {
        CHECK(L.head->next->day == 5, "倒序后第一条是第 5 天（实际 %d）", L.head->next->day);
    } else {
        printf("  [note] 你的实现没用到 head->next，跳过首节点检查\n");
    }

    log_destroy(&L);

    return t_end("t02");
}
