#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
plot.py  ——  把 bench.csv 画成图（第 9 周）

用法：
    pf bench                # 先跑出 out/bench.csv
    python tools/plot.py      # 画图，存成 out/bench.png

关于 matplotlib：
    如果你的电脑没装，脚本会退化成"用字符画一个柱状图"，照样能看。
    想装：pip install matplotlib
"""

import csv
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CSV = os.path.join(ROOT, "out", "bench.csv")
PNG = os.path.join(ROOT, "out", "bench.png")


def load():
    if not os.path.exists(CSV):
        print(f"找不到 {CSV}")
        print("先跑 pf bench")
        sys.exit(1)

    rows = []
    with open(CSV, encoding="utf-8") as f:
        for r in csv.DictReader(f):
            rows.append({
                "algo": r["algo"],
                "cn": r["algo_cn"],
                "n": int(r["n"]),
                "cmp": int(r["cmp"]),
                "mov": int(r["mov"]),
                "ms": float(r["ms"]),
            })
    return rows


def ascii_chart(rows, n_want=1000):
    """没装 matplotlib 时的退路：用字符画柱状图"""
    sel = [r for r in rows if r["n"] == n_want]
    if not sel:
        return

    print(f"\n  比较次数（n = {n_want}）")
    print("  " + "-" * 60)
    mx = max(r["cmp"] for r in sel)
    for r in sorted(sel, key=lambda x: x["cmp"]):
        bar = int(r["cmp"] / mx * 40)
        print(f"  {r['cn']:<8} {'#' * bar} {r['cmp']:,}")
    print()

    print(f"  真实耗时 ms（n = {n_want}）")
    print("  " + "-" * 60)
    mx = max(r["ms"] for r in sel)
    for r in sorted(sel, key=lambda x: x["ms"]):
        bar = int(r["ms"] / mx * 40) if mx > 0 else 0
        print(f"  {r['cn']:<8} {'#' * bar} {r['ms']:.1f}")
    print()


def main():
    rows = load()

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        from matplotlib import font_manager
    except ImportError:
        print("没装 matplotlib，用字符画代替（pip install matplotlib 可以装）")
        ascii_chart(rows)
        return 0

    # 找一款能显示中文的字体，找不到就退回英文
    zh_font = None
    for name in ["Microsoft YaHei", "SimHei", "PingFang SC",
                 "Noto Sans CJK SC", "WenQuanYi Micro Hei", "Arial Unicode MS"]:
        try:
            font_manager.findfont(name, fallback_to_default=False)
            zh_font = name
            break
        except Exception:
            continue

    if zh_font:
        plt.rcParams["font.sans-serif"] = [zh_font]
    plt.rcParams["axes.unicode_minus"] = False

    # 没有中文字体就整张图用英文（不然满屏方块）
    T = {
        "n": "数据规模 n" if zh_font else "n",
        "cmp": "比较次数" if zh_font else "comparisons",
        "cmp_axis": "比较次数（对数）" if zh_font else "comparisons (log)",
        "t1": "比较次数：n 越大，差距越夸张" if zh_font else "comparisons vs n",
        "t2": (lambda k: f"同样是 {k} 个数") if zh_font else (lambda k: f"n = {k}"),
            }

    sizes = sorted({r["n"] for r in rows})
    algos = []
    for r in rows:
        if r["algo"] not in algos:
            algos.append(r["algo"])

    label = {r["algo"]: (r["cn"] if zh_font else r["algo"]) for r in rows}

    fig, axes = plt.subplots(1, 2, figsize=(13, 5))

    # 左：比较次数 vs 规模（对数坐标，否则小的被压成一条线）
    ax = axes[0]
    for a in algos:
        xs, ys = [], []
        for n in sizes:
            hit = [r for r in rows if r["algo"] == a and r["n"] == n]
            if hit:
                xs.append(n)
                ys.append(hit[0]["cmp"])
        if xs:
            ax.plot(xs, ys, marker="o", label=label[a])
    ax.set_xlabel(T["n"])
    ax.set_ylabel(T["cmp_axis"])
    ax.set_yscale("log")
    ax.set_title(T["t1"])
    ax.grid(True, alpha=.3)
    ax.legend(fontsize=9)

    # 右：同一规模下各算法比较次数
    ax = axes[1]
    n_show = sizes[min(1, len(sizes) - 1)]
    sel = [r for r in rows if r["n"] == n_show]
    sel.sort(key=lambda r: r["cmp"])
    ax.barh([label[r["algo"]] for r in sel],
            [r["cmp"] for r in sel],
            color="#7fb356")
    ax.set_xlabel(T["cmp"])
    ax.set_title(T["t2"](n_show))
    for i, r in enumerate(sel):
        ax.text(r["cmp"], i, f" {r['cmp']:,}", va="center", fontsize=9)
    ax.grid(True, axis="x", alpha=.3)

    fig.tight_layout()
    fig.savefig(PNG, dpi=130)
    print(f"图已保存到 {PNG}")
    if not zh_font:
        print("（没找到中文字体，图里用英文标签。Windows 上装了微软雅黑就会自动用中文）")
    print("（左边是对数坐标：如果是普通坐标，冒泡会把别的全压平）")
    return 0


if __name__ == "__main__":
    sys.exit(main())
