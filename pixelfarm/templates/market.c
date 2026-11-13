/* ============================================================================
 *  第 9 周 · 市场行情（七种排序）      templates/market.c
 *
 *  怎么用：复制到 src/market.c，按任务书一个一个填 TODO。
 *
 *  ── 先说清楚三件事 ──────────────────────────────────────────────
 *
 *  1) 这里的 cmp 是一个"比较函数"的名字。
 *     你不用管它怎么实现的，只要知道：
 *         cmp(&x, &y) < 0   表示 x 应该排在 y 前面
 *         cmp(&x, &y) > 0   表示 x 应该排在 y 后面
 *     老师已经写好两个：cmp_price（价格高的在前）、cmp_profit（净收益高的在前）。
 *     你在排序里直接写 cmp(&a[i], &a[j]) 就行，跟调用普通函数一模一样。
 *
 *  2) 每比较一次、每交换一次，都要"记账" —— 这是本课程的核心机制：
 *         比较一次：  g_stat.cmp++;   cost_compare(1);
 *         交换一次：  g_stat.mov += 3; cost_move(3);
 *     不记账，屏幕上"计算已耗"就不会动，你就看不出快排比冒泡快多少。
 *
 *  3) 交换两个结构体，就三行，不用 memcpy：
 *         Crop t = a[i];  a[i] = a[j];  a[j] = t;
 *
 *  ── 一个完整的样板（冒泡的第一轮），照这个写 ────────────────────
 *
 *      for (int j = 0; j < n - 1 - i; j++) {
 *          g_stat.cmp++;
 *          cost_compare(1);                 // 记一次比较
 *          if (cmp(&a[j], &a[j + 1]) > 0) { // 前一个比后一个大，就该换
 *              Crop t = a[j];
 *              a[j] = a[j + 1];
 *              a[j + 1] = t;
 *              g_stat.mov += 3;             // 一次交换 = 3 次移动
 *              cost_move(3);
 *          }
 *      }
 * ========================================================================== */
#include "market.h"

/* ── 读代码前的两个小约定 ──────────────────────────────────────────────
 *   (void)x;                  这个参数暂时还没用上。写它只是为了让编译器
 *                             不报"参数没用到"的警告，你把函数写完就可以删掉。
 *   memset(p, 0, sizeof(X));  把结构体全部清成 0 的标准写法。sizeof 括号里
 *                             写类型名（比如 Barn、GrowthList），不要写变量名。
 * ──────────────────────────────────────────────────────────────────── */

/* 全局统计量：引擎要读它来显示"本次排序代价"。
 * 这个定义只能有一份，所以必须留在这个文件里，别删。 */
SortStat g_stat = {0, 0, 0};

/* ----------------------------------------------------------------------------
 *  老师给的，不用改
 * -------------------------------------------------------------------------- */

/* 价格高的排前面；价格一样按名字字典序 */
int cmp_price(const Crop *a, const Crop *b)
{
    if (a->price != b->price) return b->price - a->price;
    return strcmp(a->name, b->name);
}

/* 净收益高的排前面 */
int cmp_profit(const Crop *a, const Crop *b)
{
    if (a->net_profit != b->net_profit) return b->net_profit - a->net_profit;
    return strcmp(a->name, b->name);
}

/* ============================================================================
 *  你要写的 7 个排序 + 2 个业务函数
 * ========================================================================== */

/* TODO —— 冒泡：相邻两个比，大的往后冒
 *   外层 i 表示"已经排好了几个"，内层 j 只需要走到 n-1-i。
 */
void sort_bubble(Crop *a, int n, CmpFn cmp)
{
    (void)a; (void)n; (void)cmp;
}

/* TODO —— 选择：每轮在剩下的里面挑最小的，和本轮第一个交换
 *   比较次数永远是 n*(n-1)/2，但交换最多 n-1 次。
 */
void sort_select(Crop *a, int n, CmpFn cmp)
{
    (void)a; (void)n; (void)cmp;
}

/* TODO —— 插入：把 a[i] 插到前面已经排好的序列里
 *   像整理扑克牌：先把它存到临时变量 t，再往前逐个后移，最后把 t 放进空位。
 */
void sort_insert(Crop *a, int n, CmpFn cmp)
{
    (void)a; (void)n; (void)cmp;
}

/* TODO —— 希尔：插入排序的加强版
 *   先按大间隔 gap 跳着做插入排序，gap 每次减半，最后 gap=1 就是普通插入排序。
 */
void sort_shell(Crop *a, int n, CmpFn cmp)
{
    (void)a; (void)n; (void)cmp;
}

/* TODO —— 归并：分成两半各自排好，再合并
 *   需要一块临时数组：Crop *tmp = (Crop *)malloc(sizeof(Crop) * n);
 *   用完 free(tmp)。别忘了判 malloc 是否成功。
 */
void sort_merge(Crop *a, int n, CmpFn cmp)
{
    (void)a; (void)n; (void)cmp;
}

/* TODO —— 快排：选一个基准，比它小的放左边、大的放右边，然后左右两边递归
 *   出口是 n <= 1（一个元素或没有元素，本来就有序）。
 *   别总取第一个当基准 —— 数据已经有序时会退化成 O(n²)。
 */
void sort_quick(Crop *a, int n, CmpFn cmp)
{
    (void)a; (void)n; (void)cmp;
}

/* TODO —— 堆排序：先建大顶堆，再把堆顶一个个换到末尾
 *   下标从 0 开始：节点 i 的孩子是 2*i+1 和 2*i+2。
 *   第 6 周写过小根堆，这里方向反过来。
 */
void sort_heap(Crop *a, int n, CmpFn cmp)
{
    (void)a; (void)n; (void)cmp;
}

/* TODO —— 多关键字排序：价格降序 → 生长天数升序 → 名字字典序
 *
 *   三个条件有优先级，所以不能"三个都比较一遍"，要写成 if / else if 链：
 *       如果价格不同       → 按价格比
 *       否则如果生长天数不同 → 按生长天数比
 *       否则              → 用 strcmp 比名字
 *   用插入排序实现就行（比较次数少，讲多关键字最直观）。
 */
void market_rank(Crop *a, int n)
{
    (void)a; (void)n;
}

/* TODO —— 今天这块地该种什么：在能种的里面挑"净收益"最高的
 *   收益 = 价格 * 土壤质量 - 种子成本
 *   返回作物 id；一个都没有返回 -1。
 */
int market_best_crop(const Crop *a, int n, int soil_quality)
{
    (void)a; (void)n; (void)soil_quality;
    return -1;
}

/* ----------------------------------------------------------------------------
 *  老师给的，不用改：排序算法的名字（引擎显示用）
 * -------------------------------------------------------------------------- */
const char *sort_name(int algo)
{
    switch (algo) {
        case 0: return "bubble";
        case 1: return "select";
        case 2: return "insert";
        case 3: return "shell";
        case 4: return "merge";
        case 5: return "quick";
        case 6: return "heap";
        default: return "?";
    }
}
