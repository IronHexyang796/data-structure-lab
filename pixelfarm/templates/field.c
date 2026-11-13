/* ============================================================================
 *  第 1 周 · 农田与仓库（顺序表）      templates/field.c
 *
 *  怎么用：
 *    1. 把本文件复制一份，放到 src/ 下面，名字必须是 field.c
 *       （VSCode 左侧：右键 src → 粘贴；或者另存为 src/field.c）
 *    2. 按任务书 T2 ~ T6 的顺序，把每个 TODO 换成你自己的代码
 *    3. Ctrl+Shift+B 编译，pf test 1 跑本周测试
 *
 *  说明：
 *    - 这个文件里 14 个函数一个都不能少。少一个，链接就会报
 *      undefined reference to 'xxx' —— 因为 src/field.c 一出现，
 *      src/stub.c 里第 1 周的临时实现就自动失效了。
 *    - 现在这些函数体是"空壳"：能编译，但什么也不做。
 *      所以刚复制过来时测试是红的，这是正常的。
 *    - 函数签名不要改，include/field.h 是怎么写的，你就得怎么写。
 * ========================================================================== */
#include "field.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* ============================================================================
 *  一、地图：6×6 的地块
 *
 *  g_farm.plots 是二维数组：plots[行][列]，也就是 plots[y][x]。
 *  6×6 的格子在同一块内存里挨着放，编译器帮你算 y*6+x。
 * ========================================================================== */

/* TODO T2 —— 取 (x,y) 这块地的指针；越界返回 NULL
 *
 *   下面这行"能跑"，但还差一件事：越界判断。
 *   （不判越界的话，输入 -1 或者 99 会读到数组外面去，程序直接崩。）
 *
 *   补成：
 *       if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return NULL;
 *       return &g_farm.plots[y][x];
 *
 *   顺便想清楚：为什么是 plots[y][x]，不是 plots[x][y]？
 */
Plot *plot_at(int x, int y)
{
    return &g_farm.plots[y][x];
}

/* TODO T3 —— 开垦：荒地/枯萎地 -> 已开垦
 *
 *   条件：状态是 PLOT_WILD 或 PLOT_DEAD；体力 >= 10
 *   动作：state = PLOT_TILLED；crop_id = -1；water = 30；
 *         体力 -= 10；g_farm.till_count++；advance_time(600)
 *   返回：PF_OK / PF_PARAM（地块不对）/ PF_EMPTY（体力不够）
 */
int field_till(int x, int y)
{
    (void)x; (void)y;
    return PF_PARAM;
}

/* TODO T3 —— 播种：已开垦 -> 已播种
 *
 *   条件：状态是 PLOT_TILLED；crop_by_id(crop_id) 找得到；金币够
 *   动作：扣种子钱；记 crop_id、planted_day、ripe_day = 今天 + grow_days
 *   返回：PF_OK / PF_PARAM / PF_EMPTY（钱不够）
 */
int field_sow(int x, int y, int crop_id)
{
    (void)x; (void)y; (void)crop_id;
    return PF_PARAM;
}

/* TODO T3 —— 浇水：水分回满，荒地不能浇
 *
 *   返回：PF_OK / PF_PARAM
 */
int field_water(int x, int y)
{
    (void)x; (void)y;
    return PF_PARAM;
}

/* TODO T3 —— 收获：成熟 -> 产物进仓库，地块回到已开垦
 *
 *   产量 3~6 斤（3 + pf_rand(4)）
 *   返回：实际斤数（>0）/ PF_PARAM（没熟）/ PF_FULL（仓库满）
 *   注意：仓库满了不能把地块改回已开垦，否则菜就白丢了
 */
int field_harvest(int x, int y)
{
    (void)x; (void)y;
    return PF_PARAM;
}

/* TODO T4 —— 换天：所有种着东西的地块长一天
 *
 *   对每个 SEEDED / GROWING / RIPE 的地块：
 *     water -= 25；water <= 0 时 40% 概率变 PLOT_DEAD
 *     到了 ripe_day -> PLOT_RIPE，否则 -> PLOT_GROWING
 */
void field_new_day(void)
{
}

/* ============================================================================
 *  二、仓库：这是本周的主角，一个会自己长大的顺序表
 * ========================================================================== */

/* 初始化：向系统借 cap 个 Item 大小的内存
 *   返回：PF_OK / PF_PARAM / PF_NOMEM
 *   这段任务书里给了完整代码，可以直接抄（T3）
 */
int barn_init(Barn *b, int cap)
{
    if (!b) return PF_PARAM;

    /* TODO T3：malloc + 判空 + 设 count/cap */

    memset(b, 0, sizeof(Barn));   /* 先把结构体清零，免得 free 到野指针；实现完就删掉这行 */
    (void)cap;
    return PF_PARAM;
}

/* 销毁：把借的内存还回去，指针置 NULL
 *   这段任务书里给了完整代码（T3）
 */
void barn_destroy(Barn *b)
{
    /* TODO T3：free + 置 NULL + count/cap 归零 */
    (void)b;
}

/* TODO T4（本实验核心）—— 扩容：容量翻倍
 *
 *   1) int newcap = b->cap * 2;
 *   2) Item *p = realloc(b->data, newcap * sizeof(Item));
 *      —— 必须先用临时指针接住，不能直接赋给 b->data
 *   3) p == NULL -> return PF_NOMEM（原来的数据还在，不能丢）
 *   4) b->data = p; b->cap = newcap;
 *   5) cost_malloc(1);   // 记一次分配，引擎会折算成游戏时间
 */
int barn_grow(Barn *b)
{
    (void)b;
    return PF_PARAM;
}

/* TODO T5 —— 入库：同一种作物合并数量，没有就追加
 *
 *   1) 先 barn_find 找有没有，有就 data[i].count += count
 *   2) 没有：count >= cap 就先 barn_grow
 *   3) 写到 data[count]，count++
 *   每一步比较都要调 cost_compare(1)
 */
int barn_add(Barn *b, int crop_id, int count)
{
    (void)b; (void)crop_id; (void)count;
    return PF_PARAM;
}

/* TODO T5 —— 按下标删除：后面的元素全部往前挪一格
 *
 *   挪动时调 cost_move(1)，这就是 O(n) 的代价
 *   返回：PF_OK / PF_PARAM（下标越界）/ PF_EMPTY（仓库空）
 */
int barn_remove_at(Barn *b, int pos, Item *out)
{
    (void)b; (void)pos; (void)out;
    return PF_PARAM;
}

/* TODO T5 —— 查找：找到返回下标，找不到返回 -1 */
int barn_find(const Barn *b, int crop_id)
{
    (void)b; (void)crop_id;
    return -1;
}

/* TODO T6 —— 文件读写：把仓库存成 CSV / 从 CSV 读回来
 *   格式：crop_id,count,quality   第一行是表头
 *   任务书 §5 给了参考写法，可以抄
 */
int barn_load_csv(Barn *b, const char *path)
{
    (void)b; (void)path;
    return PF_PARAM;
}

int barn_save_csv(const Barn *b, const char *path)
{
    (void)b; (void)path;
    return PF_PARAM;
}
