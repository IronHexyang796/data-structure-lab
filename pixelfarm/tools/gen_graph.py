#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_graph.py  ——  生成随机图，给对拍用

用法：
    python tools/gen_graph.py 12 out/graph.txt      # 12 个点的随机图
    python tools/gen_graph.py                       # 默认 10 个点，写到 out/graph.txt

输出格式（这个格式 farm --path 能读）：
    第一行：点数 n
    之后每行：u v w   （一条无向边，w 是米数）

为什么要有这个脚本：
    你手写的 5 个测试用例，覆盖不到"图不连通"、"有重边"、"最短路径要绕 3 跳"
    这些情况。随机跑 100 组，能跑出来的 bug 比手写用例多得多。
"""

import random
import sys
import os


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 10
    out_path = sys.argv[2] if len(sys.argv) > 2 else "out/graph.txt"

    if n < 2 or n > 60:
        print("点数要在 2..60 之间")
        return 1

    rng = random.Random()          # 不加种子，每次都不一样（这才是随机测试）

    edges = {}
    # 先连成一棵随机生成树，保证连通（大约 70% 的情况我们想要连通图）
    nodes = list(range(n))
    rng.shuffle(nodes)
    for i in range(1, n):
        u = nodes[i]
        v = nodes[rng.randrange(i)]
        w = rng.randint(1, 100)
        edges[(min(u, v), max(u, v))] = w

    # 再随机加一些边（这样就可能有"绕着走更近"的情况）
    extra = rng.randint(0, n * 2)
    for _ in range(extra):
        u = rng.randrange(n)
        v = rng.randrange(n)
        if u == v:
            continue
        key = (min(u, v), max(u, v))
        if key in edges:
            continue
        edges[key] = rng.randint(1, 100)

    # 有 20% 的概率故意删掉一条边，做成不连通图（考验你的 INF 处理）
    if rng.random() < 0.2 and len(edges) > n:
        drop = rng.choice(list(edges.keys()))
        del edges[drop]

    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(f"{n}\n")
        for (u, v), w in sorted(edges.items()):
            f.write(f"{u} {v} {w}\n")
        # 顺手写一行注释，说明这是怎么生成的
    print(f"生成 {n} 个点、{len(edges)} 条边 -> {out_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
