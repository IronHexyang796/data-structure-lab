/* ============================================================================
 *  include/order.h  ——  第 3 周：订单队列（循环队列）
 *  老师提供，只读。
 * ========================================================================== */
#ifndef ORDER_H
#define ORDER_H
#include "farm.h"

typedef struct {
    Order *data;
    int front, rear;
    int cap;
} OrderQueue;

int order_init(OrderQueue *q, int cap);
void order_destroy(OrderQueue *q);
int order_enqueue(OrderQueue *q, const Order *o);
int order_dequeue(OrderQueue *q, Order *out);
int order_size(const OrderQueue *q);
int order_peek(const OrderQueue *q, Order *out);

/* 引擎用：生成一条随机订单（不负责入队） */
void order_make(Order *o, int id);

#endif /* ORDER_H */
