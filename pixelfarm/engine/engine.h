/* ============================================================================
 *  engine/engine.h  ——  引擎内部头文件（学生不用看）
 * ========================================================================== */
#ifndef ENGINE_H
#define ENGINE_H

#include "farm.h"
#include "field.h"
#include "growth.h"
#include "undo.h"
#include "order.h"
#include "map.h"
#include "tech.h"
#include "huffman.h"
#include "road.h"
#include "path.h"
#include "market.h"
#include "index.h"

/* ---------------------------------------------------------------- 终端 */
void term_init(void);           /* 开 ANSI、关行缓冲、隐藏光标 */
void term_restore(void);        /* 恢复原状，程序退出前必须调 */
void term_clear(void);          /* 清屏，光标归位 */
void term_flush(void);

/* 读一个键。返回 -1 表示没按键（不阻塞）。
 * 特殊键编码：UP=1000 DOWN=1001 LEFT=1002 RIGHT=1003 ESC=27 */
int  term_key(void);
int  pf_stdin_is_tty(void);   /* stdin 是真终端才等按键 */

#define KEY_UP    1000
#define KEY_DOWN  1001
#define KEY_LEFT  1002
#define KEY_RIGHT 1003

/* ---------------------------------------------------------------- 颜色 */
#define C_RST   "\033[0m"
#define C_BOLD  "\033[1m"
#define C_DIM   "\033[2m"
#define C_RED   "\033[31m"
#define C_GRN   "\033[32m"
#define C_YEL   "\033[33m"
#define C_BLU   "\033[34m"
#define C_MAG   "\033[35m"
#define C_CYN   "\033[36m"
#define C_WHT   "\033[37m"
#define C_BRED  "\033[91m"
#define C_BGRN  "\033[92m"
#define C_BYEL  "\033[93m"
#define C_BBLU  "\033[94m"
#define C_BCYN  "\033[96m"
#define C_BG_D  "\033[48;5;236m"

/* ---------------------------------------------------------------- 启动 / 主循环 */
int  game_boot(unsigned seed);   /* 初始化全局状态，返回 PF_OK */
void game_loop(void);            /* 进游戏，按 Q 退出 */

/* ---------------------------------------------------------------- 渲染 */
void render_frame(void);        /* 画整屏 */
void render_flush_toast(void);  /* 提示只活几帧，主循环里递减 */

/* ---------------------------------------------------------------- 存档 */
int  save_game(const char *path);
int  load_game(const char *path);

/* ---------------------------------------------------------------- 时间 */
void clock_new_day(void);
void clock_tick_real(double dt_sec);   /* 真实时间推进（只有 1 秒 = 若干游戏秒） */

/* ---------------------------------------------------------------- JSON */
void dump_state_json(const char *path);
void dump_state_stdout(void);
int  build_state_buf(char *out, int cap);   /* 写进内存缓冲，给 server 用 */

/* ---------------------------------------------------------------- Web 服务器 */
int  serve_forever(int port);               /* 阻塞，Ctrl+C 退出 */

/* ---------------------------------------------------------------- 玩家动作（供 main/loop 调） */
typedef struct {
    int cx, cy;        /* 光标 */
    int sel_crop;      /* 当前选中的作物 id */
    int running;
} UIState;

extern UIState g_ui;

int  ui_do_till(void);
int  ui_do_sow(void);
int  ui_do_water(void);
int  ui_do_harvest(void);
int  ui_do_undo(void);
int  ui_do_order(void);
int  ui_do_sell(void);
void ui_do_market(void);
int  ui_do_next_day(void);
void ui_do_growth_list(void);
void ui_do_tech(void);
void ui_do_graph(void);
void ui_do_map(void);
void ui_do_truck(void);
void ui_do_index(void);
void ui_do_help(void);

/* 弹一层"详情面板"，按任意键返回 */
void ui_pause_panel(const char *title, const char *body);

#endif /* ENGINE_H */
