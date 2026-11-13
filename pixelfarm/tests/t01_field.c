/* 第 1 周 · 农田与仓库（顺序表） */
#include "test_util.h"

int main(void)
{
    t_setup();

    /* ---------------- 仓库：初始化 / 扩容 ---------------- */
    t_begin("1. 仓库初始化");
    Barn b;
    CHECK(barn_init(&b, 8) == PF_OK, "barn_init 返回 PF_OK");
    CHECK(b.cap == 8, "容量是 8（实际 %d）", b.cap);
    CHECK(b.count == 0, "一开始是空的");
    CHECK(b.data != NULL, "内存真的分配了");

    t_begin("2. 加东西 + 自动扩容");
    for (int i = 0; i < 20; i++)
        barn_add(&b, i % 12, i + 1);

    CHECK(b.count == 12, "12 种作物合并后 count = 12（实际 %d）", b.count);
    CHECK(b.cap >= 12, "容量自动长到 %d（原来 8）", b.cap);
    int sum = 0;
    for (int i = 0; i < b.count; i++) sum += b.data[i].count;
    CHECK(sum == 210, "数量总和 210（实际 %d）", sum);

    t_begin("3. 查找");
    CHECK(barn_find(&b, 3) >= 0, "找得到 crop 3");
    CHECK(barn_find(&b, 99) == -1, "找不到的返回 -1");

    t_begin("4. 按下标删除");
    int cnt_before = b.count;
    Item out;
    int rc = barn_remove_at(&b, 3, &out);
    CHECK(rc == PF_OK, "删除下标 3 成功");
    CHECK(b.count == cnt_before - 1, "count 少了一个");
    CHECK(barn_remove_at(&b, 999, &out) == PF_PARAM, "越界删除返回 PF_PARAM");

    t_begin("5. 空仓库删东西");
    Barn e;
    barn_init(&e, 4);
    CHECK(barn_remove_at(&e, 0, &out) == PF_EMPTY, "空仓库删除返回 PF_EMPTY");
    barn_destroy(&e);
    CHECK(e.data == NULL && e.count == 0, "destroy 之后指针置空");

    barn_destroy(&b);

    /* ---------------- 农田 ---------------- */
    t_begin("6. 地块操作");
    Plot *p = plot_at(0, 0);
    CHECK(p != NULL, "plot_at(0,0) 能拿到");
    CHECK(plot_at(-1, 0) == NULL, "越界返回 NULL");
    CHECK(plot_at(6, 0) == NULL, "越界返回 NULL（右边界）");

    CHECK(field_till(0, 0) == PF_OK, "开垦成功");
    CHECK(plot_at(0, 0)->state == PLOT_TILLED, "状态变成已开垦");
    CHECK(field_till(0, 0) == PF_PARAM, "重复开垦返回 PF_PARAM");

    CHECK(field_sow(0, 0, 0) == PF_OK, "播种成功");
    CHECK(plot_at(0, 0)->state == PLOT_SEEDED, "状态变成已播种");
    CHECK(plot_at(0, 0)->ripe_day == 1 + 2, "成熟日 = 今天 + 生长天数（crop0 是 2 天）");

    CHECK(field_harvest(0, 0) <= 0, "没熟不能收");

    /* 推进到成熟日 */
    g_farm.day = plot_at(0, 0)->ripe_day;
    field_new_day();
    CHECK(plot_at(0, 0)->state == PLOT_RIPE, "到日子就成熟了");

    Barn bk;
    barn_init(&bk, 8);
    /* field_harvest 用的是 g_farm.barn，先接上去 */
    Barn old = g_farm.barn;
    g_farm.barn = bk;
    int got = field_harvest(0, 0);
    CHECK(got > 0, "收获拿到 %d 斤", got);
    CHECK(g_farm.barn.count == 1, "进仓库了");
    CHECK(plot_at(0, 0)->state == PLOT_TILLED, "收完回到已开垦");

    barn_destroy(&g_farm.barn);
    g_farm.barn = old;

    t_begin("7. 浇水");
    CHECK(field_water(1, 1) != PF_OK, "荒地不能浇水");
    field_till(1, 1);
    CHECK(field_water(1, 1) == PF_OK, "开垦后能浇水");
    CHECK(plot_at(1, 1)->water == 100, "浇完水分是 100");

    return t_end("t01");
}
