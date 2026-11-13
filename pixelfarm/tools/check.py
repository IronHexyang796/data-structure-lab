#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
check.py  ——  对拍脚本（第 8 周）

用法：
    python tools/check.py           # 跑 100 组随机图
    python tools/check.py 500       # 跑 500 组

它干的事：
    1. 生成一张随机图
    2. 让 ./farm --path 算一遍（你的 C 代码）
    3. 让 brute_path.py 算一遍（Python 标准答案）
    4. 逐行对比

只要有一组不一样，就把那张图存下来，方便你复现。

为什么比 pf test 强：
    pf test 只有 5 个我写的用例，覆盖不了"图不连通""重边""绕 4 跳更近"。
    随机跑 100 组，撞到边界条件的概率大得多。

前置条件：
    - 已经编译过（有 farm 或 farm.exe）
    - 有 python（Windows 上可能是 python 而不是 python3）
"""

import os
import random
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
OUT = os.path.join(ROOT, "out")

# Windows 上叫 farm.exe，其它平台叫 farm
FARM = os.path.join(ROOT, "farm.exe")
if not os.path.exists(FARM):
    FARM = os.path.join(ROOT, "farm")
if not os.path.exists(FARM):
    print("找不到 farm 可执行文件，先敲一次 pf")
    sys.exit(1)

PY = sys.executable or "python3"


def run(cmd):
    r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    if r.returncode != 0:
        return None, r.stderr.strip() or r.stdout.strip()
    return r.stdout, None


def main():
    rounds = int(sys.argv[1]) if len(sys.argv) > 1 else 100
    os.makedirs(OUT, exist_ok=True)

    gpath = os.path.join(OUT, "graph.txt")
    apath = os.path.join(OUT, "answer.txt")

    bad = 0

    for i in range(1, rounds + 1):
        n = random.randint(4, 20)

        # 1) 生成随机图
        out, err = run([PY, "tools/gen_graph.py", str(n), gpath])
        if err:
            print(f"[{i}] 生成图失败：{err}")
            return 1

        # 2) 你的 C 代码算一遍
        got, err = run([FARM, "--path", gpath])
        if err:
            print(f"[{i}] farm --path 出错：{err}")
            print(f"     这张图留在 {gpath}，可以手动复现")
            return 1

        # 3) Python 标准答案算一遍
        out, err = run([PY, "tools/brute_path.py", gpath, apath])
        if err:
            print(f"[{i}] 标准答案出错：{err}")
            return 1

        with open(apath, encoding="utf-8") as f:
            want = f.read()

        # 4) 对比
        if got.strip() != want.strip():
            bad += 1
            keep = os.path.join(OUT, f"bad_{i}.txt")
            with open(keep, "w", encoding="utf-8") as f:
                f.write(f"# 第 {i} 组，{n} 个点\n")
                with open(gpath, encoding="utf-8") as g:
                    f.write(g.read())
            print(f"[{i}] ✗ 不一致！这张图存到了 {keep}")

            # 打印第一处不同的地方
            gl = got.strip().splitlines()
            wl = want.strip().splitlines()
            for k in range(min(len(gl), len(wl))):
                if gl[k] != wl[k]:
                    print(f"        第 {k+1} 行：")
                    print(f"          你的：{gl[k]}")
                    print(f"          正确答案：{wl[k]}")
                    break
            else:
                print(f"        行数不一样：你的 {len(gl)} 行，答案 {len(wl)} 行")

            if bad >= 3:
                print("\n连续 3 组不一致，先停下来修代码吧。")
                return 1
        else:
            if i % 20 == 0 or i == rounds:
                print(f"[{i}/{rounds}] 到目前为止全部一致")

    print()
    if bad == 0:
        print(f"✓ {rounds} 组随机数据全部通过")
        print("  这比 pf test 有说服力得多 —— 说明你不是刚好碰对了 5 个用例。")
        return 0
    else:
        print(f"✗ {bad} 组不一致，看看 out/bad_*.txt")
        return 1


if __name__ == "__main__":
    sys.exit(main())
