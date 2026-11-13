#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
brute_path.py  ——  标准答案（用 Python 暴力算全源最短路）

用法：
    python tools/brute_path.py out/graph.txt out/answer.txt

它读 gen_graph.py 生成的图，用 Floyd 算一遍，输出和 `farm --path` 完全一样的格式：
    n
    dist u v w

为什么用 Python 写标准答案：
    同一个算法用两种语言各写一遍，如果两边都错成一样，那概率极低。
    这就是"对拍"的原理 —— 不是相信某一份代码，而是相信两份独立的实现。
"""

import sys

INF = 0x3f3f3f3f


def main():
    if len(sys.argv) < 2:
        print("用法: python tools/brute_path.py <图文件> [输出文件]")
        return 1

    src = sys.argv[1]
    dst = sys.argv[2] if len(sys.argv) > 2 else None

    with open(src, encoding="utf-8") as f:
        first = f.readline().split()
        if not first:
            print("图文件是空的")
            return 1
        n = int(first[0])
        edges = []
        for line in f:
            parts = line.split()
            if len(parts) != 3:
                continue
            u, v, w = int(parts[0]), int(parts[1]), int(parts[2])
            if 0 <= u < n and 0 <= v < n:
                edges.append((u, v, w))

    # 邻接矩阵
    m = [[0 if i == j else INF for j in range(n)] for i in range(n)]
    for u, v, w in edges:
        if w < m[u][v]:
            m[u][v] = w
            m[v][u] = w

    # Floyd：三重循环，中间点 k 放最外层（放错就错）
    for k in range(n):
        for i in range(n):
            dik = m[i][k]
            if dik == INF:
                continue
            for j in range(n):
                nd = dik + m[k][j]
                if nd < m[i][j]:
                    m[i][j] = nd

    lines = [str(n)]
    for i in range(n):
        for j in range(n):
            if i == j:
                continue
            lines.append(f"dist {i} {j} {'INF' if m[i][j] >= INF else m[i][j]}")

    text = "\n".join(lines) + "\n"
    if dst:
        with open(dst, "w", encoding="utf-8") as f:
            f.write(text)
        print(f"标准答案写入 {dst}（{n} 个点，{len(edges)} 条边）")
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
