# templates/ —— 每周的起始文件

这里放着 10 个"骨架文件"。**用法只有一句话：复制到 `src/` 下面。**

```
templates/field.c    →  src/field.c     第 1 周
templates/growth.c   →  src/growth.c    第 2 周
templates/undo.c     →  src/undo.c      第 3 周（和 order.c 一起）
templates/order.c    →  src/order.c
templates/map.c      →  src/map.c       第 4 周
templates/tech.c     →  src/tech.c      第 5 周
templates/huffman.c  →  src/huffman.c   第 6 周
templates/road.c     →  src/road.c      第 7 周
templates/path.c     →  src/path.c      第 8 周
templates/market.c   →  src/market.c    第 9 周
templates/index.c    →  src/index.c     第 10 周
```

## 为什么要复制，而不是自己新建空文件

因为编译脚本是按"文件在不在"来决定用谁实现的：

- `src/` 里**没有** `field.c` → 用 `src/stub.c` 里第 1 周的临时实现（能跑，但很笨）
- `src/` 里**有** `field.c` → 临时实现立刻失效，第 1 周的 14 个函数**全部**由你的 `field.c` 提供

所以如果你新建一个空文件、只写了 3 个函数，链接时会报：

```
undefined reference to 'barn_grow'
```

这不是你写错了，是"另外 11 个还没写"。从模板复制过来就不会有这个问题——模板里函数是齐的，
只是函数体是空的（先 `return PF_PARAM;`），能编译、能运行，测试一路 `[FAIL]`，
你填一个就少红一片。

## 复制过去之后

1. 先 `Ctrl+Shift+B` 编译一次，确认能过（应该是 0 error、0 warning）
2. 再跑当周测试：`pf test 1`（第几周就写几）
3. 看到一堆 `[FAIL]` 是**正常的**——那正是你要做的事
4. 按任务书的顺序，一个 TODO 一个 TODO 地填

## 注意

- 文件名必须**全小写**、扩展名必须是 `.c`：`Field.c`、`field.C`、`field.c.txt` 都不行
- 函数名、参数、返回值**不能改**，`include/` 里的头文件怎么写，你就得怎么写
- 模板里标了"老师给的，不用改"的那几段（比如 `market.c` 里的 `cmp_price`、`g_stat`），
  **不要删**，删了会链接不上
