/* ============================================================================
 *  engine/state.c  ——  各模块的"全局单例"
 *  老师提供，禁止修改。
 *
 *  游戏里每个结构体只有一个实例，所以放在这里。
 *  单独拆一个文件，是为了让测试程序也能拿到同样的实例
 *  （测试不能链接 loop.c，那里有界面代码）。
 * ========================================================================== */
#include "engine.h"

static GrowthList  s_growth;
static LogList     s_log;
static UndoStack   s_undo;
static OrderQueue  s_order;
static SparseMat   s_map;
static HuffTree    s_huff;
static Graph       s_graph;
static Index       s_index;

void *eng_growth(void) { return &s_growth; }
void *eng_log(void)    { return &s_log; }
void *eng_undo(void)   { return &s_undo; }
void *eng_order(void)  { return &s_order; }
void *eng_map(void)    { return &s_map; }
void *eng_huff(void)   { return &s_huff; }
void *eng_graph(void)  { return &s_graph; }
void *eng_index(void)  { return &s_index; }
