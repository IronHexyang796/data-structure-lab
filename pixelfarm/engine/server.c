/* ============================================================================
 *  engine/server.c  ——  让 C 程序自己当 Web 服务器
 *  老师提供，禁止修改。
 *
 *  跑法： ./farm --serve
 *        然后用浏览器打开 http://localhost:8080
 *
 *  原理：HTTP 就是"一问一答的文本"，不需要 nginx，不需要框架。
 *        浏览器发  GET /api/state HTTP/1.1
 *        我们回    HTTP/1.1 200 OK + 一段 JSON
 *  就这么简单。
 * ========================================================================== */
#include "engine.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  /* 链接时要加 -lws2_32，pf.bat / pf 里已经写了。
   * 这里不要用 #pragma comment(lib,...) —— 那是 MSVC 的写法，
   * gcc 会报一条 "ignoring #pragma comment" 的警告，破坏"0 warning"。 */
  typedef SOCKET sock_t;
  #define BAD_SOCK   INVALID_SOCKET
  #define sock_close closesocket
  typedef int sock_len_t;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int sock_t;
  #define BAD_SOCK   (-1)
  #define sock_close close
  typedef socklen_t sock_len_t;
#endif

static int   s_stop = 0;

/* ---------------------------------------------------------------- 静态文件 */
static int read_file(const char *path, char *buf, int cap)
{
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    int n = (int)fread(buf, 1, (size_t)(cap - 1), f);
    buf[n] = '\0';
    fclose(f);
    return n;
}

static void send_all(sock_t c, const char *data, int len)
{
    int sent = 0;
    while (sent < len) {
        int n = (int)send(c, data + sent, len - sent, 0);
        if (n <= 0) break;
        sent += n;
    }
}

static void send_response(sock_t c, const char *ctype,
                          const char *body, int blen, int code)
{
    char head[512];
    const char *reason = (code == 200) ? "OK" : "Not Found";
    int n = snprintf(head, sizeof head,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n\r\n",
        code, reason, ctype, blen);
    send_all(c, head, n);
    send_all(c, body, blen);
}

/* ---------------------------------------------------------------- 处理一个请求 */
static void handle(sock_t c)
{
    static char req[4096];
    static char body[256 * 1024];

    int n = (int)recv(c, req, sizeof req - 1, 0);
    if (n <= 0) return;
    req[n] = '\0';

    /* 第一行形如： GET /api/state HTTP/1.1 */
    char method[8] = {0}, path[512] = {0};
    if (sscanf(req, "%7s %511s", method, path) != 2) return;

    /* ---------- 1) 数据接口 ---------- */
    if (!strcmp(path, "/api/state")) {
        int len = build_state_buf(body, (int)sizeof body);
        send_response(c, "application/json", body, len, 200);
        return;
    }

    /* ---------- 2) 动作接口：/api/water?x=1&y=2 ---------- */
    if (!strncmp(path, "/api/", 5)) {
        char qx[32] = {0}, qy[32] = {0};
        char *q = strchr(path, '?');
        int x = -1, y = -1;

        if (q) {
            char *px = strstr(q, "x=");
            char *py = strstr(q, "y=");
            if (px) { sscanf(px + 2, "%31[0-9-]", qx); x = atoi(qx); }
            if (py) { sscanf(py + 2, "%31[0-9-]", qy); y = atoi(qy); }
        }

        /* 网页和终端走同一套动作函数，撤销记录、日志、提示都一样 */
        int ok = 0;

        if (!strncmp(path, "/api/sow", 8)) {
            char *pc = q ? strstr(q, "c=") : NULL;
            if (pc) g_ui.sel_crop = atoi(pc + 2);
        }
        g_ui.cx = x;
        g_ui.cy = y;

        if (!strncmp(path, "/api/water", 10)) {
            ok = (ui_do_water() == PF_OK);
        } else if (!strncmp(path, "/api/till", 9)) {
            ok = (ui_do_till() == PF_OK);
        } else if (!strncmp(path, "/api/sow", 8)) {
            ok = (ui_do_sow() == PF_OK);
        } else if (!strncmp(path, "/api/harvest", 12)) {
            ok = (ui_do_harvest() == PF_OK);
        } else if (!strncmp(path, "/api/nextday", 12)) {
            ok = (ui_do_next_day() == PF_OK);
        } else if (!strncmp(path, "/api/order", 10)) {
            ok = (ui_do_order() == PF_OK);
        } else if (!strncmp(path, "/api/sell", 9)) {
            ok = (ui_do_sell() == PF_OK);
        }

        int len = snprintf(body, sizeof body,
                           "{\"ok\":%s,\"toast\":", ok ? "true" : "false");
        /* 把提示信息也带回去，网页可以弹出来 */
        {
            char esc[300];
            int k = 0;
            for (int i = 0; g_farm.toast_msg[i] && k < 280; i++) {
                char ch = g_farm.toast_msg[i];
                if (ch == '"' || ch == '\\') esc[k++] = '\\';
                esc[k++] = ch;
            }
            esc[k] = '\0';
            len += snprintf(body + len, sizeof body - len, "\"%s\"}", esc);
        }
        send_response(c, "application/json", body, len, 200);
        return;
    }

    /* ---------- 3) 静态文件 ---------- */
    const char *file = "web/index.html";
    const char *ctype = "text/html";

    if (!strcmp(path, "/") || !strcmp(path, "/index.html")) {
        file = "web/index.html"; ctype = "text/html";
    } else if (!strcmp(path, "/style.css")) {
        file = "web/style.css"; ctype = "text/css";
    } else if (!strcmp(path, "/app.js")) {
        file = "web/app.js"; ctype = "text/javascript";
    } else if (!strcmp(path, "/favicon.ico")) {
        send_response(c, "text/plain", "", 0, 200);
        return;
    } else {
        const char *msg = "404 not found";
        send_response(c, "text/plain", msg, (int)strlen(msg), 404);
        return;
    }

    int len = read_file(file, body, (int)sizeof body);
    if (len < 0) {
        char msg[256];
        int m = snprintf(msg, sizeof msg,
                         "找不到 %s\n\n你现在是在 pixelfarm/ 目录下运行 farm 吗？\n"
                         "web/ 文件夹要和 farm 可执行文件在同一层。", file);
        send_response(c, "text/plain", msg, m, 404);
        return;
    }
    send_response(c, ctype, body, len, 200);
}

/* ---------------------------------------------------------------- 启动 */
int serve_forever(int port)
{
    if (port <= 0) port = 8080;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup 失败\n");
        return PF_PARAM;
    }
#endif

    sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == BAD_SOCK) { perror("socket"); return PF_PARAM; }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof opt);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);

    if (bind(srv, (struct sockaddr *)&addr, sizeof addr) < 0) {
        printf(C_BRED "\n  端口 %d 被占用了。换个端口试试： ./farm --serve %d\n" C_RST,
               port, port + 1);
        sock_close(srv);
        return PF_PARAM;
    }
    listen(srv, 8);

    printf(C_BGRN "\n  农场服务器已启动\n" C_RST);
    printf("  请用浏览器打开：  " C_BCYN "http://localhost:%d" C_RST "\n", port);
    printf(C_DIM "  按 Ctrl+C 停止\n\n" C_RST);
    fflush(stdout);

    while (!s_stop) {
        struct sockaddr_in cli;
        sock_len_t cl = sizeof cli;
        sock_t c = accept(srv, (struct sockaddr *)&cli, &cl);
        if (c == BAD_SOCK) continue;
        handle(c);
        sock_close(c);
    }

    sock_close(srv);
#ifdef _WIN32
    WSACleanup();
#endif
    return PF_OK;
}
