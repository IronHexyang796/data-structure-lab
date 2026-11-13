/* 第 4 周 · 大地图（稀疏矩阵） */
#include "test_util.h"

int main(void)
{
    t_setup();

    t_begin("1. 初始化");
    SparseMat m;
    memset(&m, 0, sizeof m);
    CHECK(sm_init(&m, 10, 10) == PF_OK, "sm_init 返回 PF_OK");
    CHECK(m.rows == 10 && m.cols == 10, "尺寸 10×10");
    CHECK(m.count == 0, "一开始没有非零元素");

    t_begin("2. 从稠密矩阵压缩");
    /* 10×10 里只有 4 个非空格子 */
    static Tile dense[100];
    memset(dense, 0, sizeof dense);
    dense[0 * 10 + 0].type = TILE_GRASS;
    dense[2 * 10 + 3].type = TILE_WATER;
    dense[5 * 10 + 7].type = TILE_ROCK;
    dense[9 * 10 + 9].type = TILE_ROAD;

    CHECK(sm_from_dense(&m, dense, 10, 10) == PF_OK, "压缩成功");
    CHECK(m.count == 4, "只存了 4 个非零（实际 %d）", m.count);
    CHECK(m.data && m.data[0].row == 0 && m.data[0].col == 0, "按行优先排序：第一个是 (0,0)");
    CHECK(m.data && m.data[3].row == 9 && m.data[3].col == 9, "最后一个是 (9,9)");

    t_begin("3. 取值");
    Tile t;
    CHECK(sm_get(&m, 2, 3, &t) == PF_OK, "取 (2,3)");
    CHECK(t.type == TILE_WATER, "取到的是水塘");
    CHECK(sm_get(&m, 1, 1, &t) == PF_OK, "取一个空格子");
    CHECK(t.type == TILE_EMPTY, "空格子返回 TILE_EMPTY");
    CHECK(sm_get(&m, 99, 0, &t) == PF_PARAM, "越界返回 PF_PARAM");

    t_begin("4. 转置");
    SparseMat tr;
    memset(&tr, 0, sizeof tr);
    CHECK(sm_transpose_slow(&m, &tr) == PF_OK, "转置成功");
    CHECK(tr.rows == 10 && tr.cols == 10, "转置后尺寸不变（方阵）");
    CHECK(tr.count == m.count, "非零个数不变");
    CHECK(tr.data && tr.data[0].col == 0 && tr.data[0].row == 0, "原来 (0,0) 转置后还是 (0,0)");

    /* (2,3) 转置后应该在 (3,2) */
    int found = 0;
    for (int i = 0; i < tr.count && tr.data; i++)
        if (tr.data[i].row == 3 && tr.data[i].col == 2) found = 1;
    CHECK(found, "原来 (2,3) 的水塘跑到了 (3,2)");

    t_begin("5. 快速转置结果一致");
    SparseMat tf;
    memset(&tf, 0, sizeof tf);
    if (sm_transpose_fast(&m, &tf) == PF_OK) {
        int same = (tf.count == tr.count);
        CHECK(same, "快转置和慢转置元素个数一致");
        int ok = 1;
        for (int i = 0; i < m.count; i++) {
            /* 原矩阵每一个 (r,c) 都应该在转置结果里以 (c,r) 出现 */
            int hit = 0;
            for (int j = 0; j < tf.count; j++)
                if (tf.data[j].row == m.data[i].col &&
                    tf.data[j].col == m.data[i].row) hit = 1;
            if (!hit) ok = 0;
        }
        CHECK(ok, "每个元素都正确搬到了转置位置");
        sm_destroy(&tf);
    } else {
        printf("  [note] 快转置还没实现，跳过\n");
    }
    sm_destroy(&tr);

    t_begin("6. 稀疏的意义：省内存");
    long dense_bytes = (long)m.rows * m.cols * (long)sizeof(Tile);
    long sparse_bytes = (long)m.cap * (long)sizeof(Triple);
    /* 除零保护：sm_init 还没实现时 dense_bytes 会是 0 */
    long save_pct = dense_bytes > 0 ? (dense_bytes - sparse_bytes) * 100 / dense_bytes : 0;
    CHECK(dense_bytes > 0 && sparse_bytes < dense_bytes,
          "三元组 %ld 字节 < 稠密 %ld 字节（省了 %ld%%）",
          sparse_bytes, dense_bytes, save_pct);

    sm_destroy(&m);
    CHECK(m.data == NULL && m.count == 0, "destroy 之后清空");

    t_begin("7. 引擎生成的大地图");
    SparseMat big;
    memset(&big, 0, sizeof big);
    map_generate(&big, 42);
    CHECK(big.rows == 100 && big.cols == 100, "100×100");
    CHECK(big.count > 0, "有 %d 个非零格子", big.count);
    SLOWER(big.count < 100 * 100 / 2,
           "非零只占 %.0f%%（确实稀疏）",
           big.count * 100.0 / (100.0 * 100.0));
    sm_destroy(&big);

    return t_end("t04");
}
