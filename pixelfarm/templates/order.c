/* ============================================================================
 *  第 3 周 · 订单队列（循环队列）      templates/order.c
 *
 *  怎么用：复制到 src/order.c。
 *
 *  循环队列的关键就一句话：留一个空格子，用 (rear+1)%cap == front 判满。
 *  front == rear 表示空。
 * ========================================================================== */
#include "order.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* TODO —— 初始化：借 cap+1 个 Order 的内存（多出来的那一格用来区分空和满）
 *   front = rear = 0;  cap = cap + 1;
 *   返回 PF_OK / PF_PARAM / PF_NOMEM
 */
int order_init(OrderQueue *q, int cap)
{
    memset(q, 0, sizeof(OrderQueue));  /* 先把结构体清零；实现完就删掉这行 */
    (void)cap;
    return PF_PARAM;
}

/* TODO —— 销毁：还内存，三个字段归零 */
void order_destroy(OrderQueue *q)
{
    (void)q;
}

/* TODO —— 入队：先判满，再写 data[rear]，然后 rear = (rear+1)%cap
 *   队列长度 = (rear - front + cap) % cap
 */
int order_enqueue(OrderQueue *q, const Order *o)
{
    (void)q; (void)o;
    return PF_PARAM;
}

/* TODO —— 出队：先判空，取 data[front]，然后 front = (front+1)%cap */
int order_dequeue(OrderQueue *q, Order *out)
{
    (void)q; (void)out;
    return PF_PARAM;
}

/* TODO —— 队列里有几单 */
int order_size(const OrderQueue *q)
{
    (void)q;
    return 0;
}

/* TODO —— 看一眼队头，不出队（交付订单时先看这单要什么） */
int order_peek(const OrderQueue *q, Order *out)
{
    (void)q; (void)out;
    return PF_PARAM;
}

/* ----------------------------------------------------------------------------
 *  下面是老师给的，不用改：随机生成一条订单
 * -------------------------------------------------------------------------- */
void order_make(Order *o, int id)
{
    if (!o) return;
    int cid = g_crop_n > 0 ? pf_rand(g_crop_n) : 0;
    o->id = id;
    o->crop_id = cid;
    o->count = 5 + pf_rand(16);
    o->price = g_crops[cid].price * o->count;
    o->status = ST_PENDING;
    o->day = g_farm.day;
}
