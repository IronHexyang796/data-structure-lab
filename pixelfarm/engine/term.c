/* ============================================================================
 *  engine/term.c  ——  终端：初始化、清屏、非阻塞读键
 *  老师提供，禁止修改。
 *
 *  这一层专门处理 Windows 和 Linux/macOS 的差异，
 *  你在 src/ 里写的代码完全不用管平台。
 * ========================================================================== */
#include "engine.h"

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
  #include <io.h>
  static HANDLE h_out = INVALID_HANDLE_VALUE;
  static DWORD  old_mode = 0;
#else
  #include <termios.h>
  #include <unistd.h>
  #include <sys/select.h>
  static struct termios old_tio;
  static int tio_saved = 0;
#endif

/* stdin 是不是一个真正的终端窗口？
 * 重定向到文件/管道时返回 0 —— 这时候读键盘永远读不到，不能死等。 */
int pf_stdin_is_tty(void)
{
#ifdef _WIN32
    return _isatty(_fileno(stdin));
#else
    return isatty(STDIN_FILENO);
#endif
}

void term_init(void)
{
#ifdef _WIN32
    h_out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h_out, &mode)) {
        old_mode = mode;
        /* ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004，让 Windows 认 ANSI 转义 */
        SetConsoleMode(h_out, mode | 0x0004);
    }
    /* 让控制台用 UTF-8，减少中文乱码 */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    system("");   /* 什么都不做，只是让某些老的 Windows 刷新一次控制台属性 */
#else
    if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &old_tio) == 0) {
        tio_saved = 1;
        struct termios t = old_tio;
        t.c_lflag &= ~(ICANON | ECHO);   /* 关掉行缓冲和回显 */
        t.c_cc[VMIN]  = 0;
        t.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
    printf("\033[?25l");   /* 隐藏光标 */
#endif
    term_clear();
}

void term_restore(void)
{
#ifdef _WIN32
    if (h_out != INVALID_HANDLE_VALUE && old_mode)
        SetConsoleMode(h_out, old_mode);
#else
    if (tio_saved) tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    printf("\033[?25h");
#endif
    printf(C_RST "\n");
    fflush(stdout);
}

void term_clear(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

void term_flush(void)
{
    fflush(stdout);
}

/* 读一个键，不阻塞。返回 -1 表示没有按键 */
int term_key(void)
{
#ifdef _WIN32
    if (!_kbhit()) return -1;
    int c = _getch();
    if (c == 0 || c == 224) {          /* 方向键是两个字节 */
        int c2 = _getch();
        switch (c2) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            default: return -1;
        }
    }
    return c;
#else
    unsigned char buf[8];
    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(STDIN_FILENO, &fs);
    struct timeval tv = {0, 0};
    if (select(STDIN_FILENO + 1, &fs, NULL, NULL, &tv) <= 0) return -1;

    int n = (int)read(STDIN_FILENO, buf, sizeof buf);
    if (n <= 0) return -1;

    if (buf[0] == 27 && n >= 3 && buf[1] == '[') {
        switch (buf[2]) {
            case 'A': return KEY_UP;
            case 'B': return KEY_DOWN;
            case 'C': return KEY_RIGHT;
            case 'D': return KEY_LEFT;
        }
        return -1;
    }
    if (buf[0] == 27 && n == 1) return 27;
    return buf[0];
#endif
}
