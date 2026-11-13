/* 第 10 周 · 图鉴检索（哈希 + 二分 + 两级索引） */
#include "test_util.h"

int main(void)
{
    t_setup();

    t_begin("1. 哈希表：基本操作");
    HashMap h;
    CHECK(hash_init(&h, 16) == PF_OK, "hash_init 返回 PF_OK");
    CHECK(h.cap == 16 && h.size == 0, "16 个桶，空的");

    Item v;
    v.crop_id = 42; v.count = 7; v.quality = 5;
    CHECK(hash_put(&h, 42, &v) == PF_OK, "放一个 key=42");
    CHECK(h.size == 1, "size = 1");

    Item got;
    CHECK(hash_get(&h, 42, &got) == PF_OK, "取得到 42");
    CHECK(got.count == 7 && got.quality == 5, "取出来的值对");

    CHECK(hash_get(&h, 999, &got) == PF_NOTFOUND, "没有的 key 返回 PF_NOTFOUND");

    t_begin("2. 哈希表：覆盖同一个 key");
    v.count = 99;
    hash_put(&h, 42, &v);
    CHECK(h.size == 1, "覆盖不会让 size 变大");
    hash_get(&h, 42, &got);
    CHECK(got.count == 99, "值是新的");

    t_begin("3. 哈希表：删除");
    CHECK(hash_remove(&h, 42) == PF_OK, "删掉 42");
    CHECK(h.size == 0, "size 变 0");
    CHECK(hash_remove(&h, 42) == PF_NOTFOUND, "再删返回 PF_NOTFOUND");

    t_begin("4. 哈希表：冲突");
    /* 16 个桶放 50 个 key，一定会有冲突（拉链法要能处理） */
    for (int i = 0; i < 50; i++) {
        v.crop_id = i; v.count = i * 2; v.quality = 3;
        hash_put(&h, i, &v);
    }
    CHECK(h.size == 50, "50 个都放进去了（实际 %d）", h.size);
    int find_all = 1;
    for (int i = 0; i < 50; i++) {
        if (hash_get(&h, i, &got) != PF_OK || got.count != i * 2) find_all = 0;
    }
    CHECK(find_all, "50 个都能正确取回来");
    hash_destroy(&h);
    CHECK(h.bucket == NULL, "destroy 之后指针置空");

    t_begin("5. 二分查找");
    enum { N = 1000 };
    static Item arr[N];
    for (int i = 0; i < N; i++) {
        arr[i].crop_id = i * 2;      /* 0,2,4,... 有序 */
        arr[i].count = i;
        arr[i].quality = 3;
    }
    CHECK(bsearch_by_id(arr, N, 0) == 0, "找第一个");
    CHECK(bsearch_by_id(arr, N, 1998) == N - 1, "找最后一个（1998 在下标 999）");
    CHECK(bsearch_by_id(arr, N, 500) == 250, "找中间：500 在下标 250");
    CHECK(bsearch_by_id(arr, N, 1) == -1, "奇数不存在，返回 -1");
    CHECK(bsearch_by_id(arr, N, 2000) == -1, "超出范围，返回 -1");

    t_begin("6. 二分比顺序快多少");
    g_farm.cmp_count = 0;
    for (int i = 0; i < 100; i++) bsearch_by_id(arr, N, i * 10);
    long bcmp = g_farm.cmp_count;

    g_farm.cmp_count = 0;
    for (int i = 0; i < 100; i++) {
        int k = i * 20;
        for (int j = 0; j < N; j++) {
            cost_compare(1);
            if (arr[j].crop_id == k) break;
        }
    }
    long lcmp = g_farm.cmp_count;

    SLOWER(bcmp * 10 < lcmp,
           "二分 %ld 次比较 vs 顺序 %ld 次（快 %.0f 倍）",
           bcmp, lcmp, (double)lcmp / (bcmp ? bcmp : 1));

    t_begin("7. 两级索引");
    enum { M = 5000 };
    static Item all[M];
    index_demo_data(all, M);

    Index ix;
    CHECK(index_build(&ix, all, M, 500) == PF_OK, "建索引成功");
    CHECK(ix.cold_n == M - 500, "冷数据 %d 条（实际 %d）", M - 500, ix.cold_n);

    /* 查 2000 次：一部分命中热数据，一部分命中冷数据，一部分不存在 */
    int ok = 1;
    for (int i = 0; i < 2000; i++) {
        int key = (i * 3) % (M + 500);     /* 一部分会超出，制造 miss */
        int rc = index_lookup(&ix, key, &got);
        if (key < M && rc != PF_OK) ok = 0;
        if (key >= M && rc != PF_NOTFOUND) ok = 0;
    }
    CHECK(ok, "2000 次查询，结果全部正确");

    long total = ix.hit_hot + ix.hit_cold + ix.miss;
    CHECK(total == 2000, "统计加起来是 2000（实际 %ld）", total);
    CHECK(ix.hit_hot > 0, "热数据命中了 %ld 次", ix.hit_hot);
    CHECK(ix.hit_cold > 0, "冷数据命中了 %ld 次", ix.hit_cold);

    printf("  [info] 热 %ld / 冷 %ld / 未命中 %ld\n",
           ix.hit_hot, ix.hit_cold, ix.miss);

    double rate = (ix.hit_hot + ix.hit_cold) * 100.0 / total;
    SLOWER(rate > 75, "命中率 %.0f%%", rate);

    t_begin("8. 和纯顺序查找比");
    /* 同样的 2000 次，改成每次从头扫 */
    g_farm.cmp_count = 0;
    for (int i = 0; i < 2000; i++) {
        int key = (i * 3) % M;
        for (int j = 0; j < M; j++) {
            cost_compare(1);
            if (all[j].crop_id == key) break;
        }
    }
    long naive = g_farm.cmp_count;

    g_farm.cmp_count = 0;
    for (int i = 0; i < 2000; i++) {
        int key = (i * 3) % M;
        index_lookup(&ix, key, &got);
    }
    long fast = g_farm.cmp_count;

    SLOWER(fast * 20 < naive,
           "索引 %ld 次比较 vs 顺序 %ld 次（快 %.0f 倍）",
           fast, naive, (double)naive / (fast ? fast : 1));

    index_destroy(&ix);
    CHECK(ix.cold == NULL, "destroy 之后清空");

    return t_end("t10");
}
