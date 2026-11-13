/* ============================================================================
 *  include/map.h  ——  第 4 周：大地图（稀疏矩阵，三元组）
 *  老师提供，只读。
 * ========================================================================== */
#ifndef MAP_H
#define MAP_H
#include "farm.h"

#define TILE_EMPTY  0
#define TILE_GRASS  1
#define TILE_TILLED 2
#define TILE_WATER  3
#define TILE_ROCK   4
#define TILE_ROAD   5

typedef struct {
    int type;
    int crop_id;      /* -1 表示没种东西 */
} Tile;

/* 三元组的一条 */
typedef struct {
    int  row;
    int  col;
    Tile val;
} Triple;

typedef struct {
    Triple *data;       /* 按 (row, col) 升序排列 */
    int count;
    int cap;
    int rows, cols;     /* 原矩阵的尺寸 */
} SparseMat;

int  sm_init(SparseMat *m, int rows, int cols);
void sm_destroy(SparseMat *m);
int  sm_from_dense(SparseMat *m, const Tile *dense, int rows, int cols);
int  sm_transpose_slow(const SparseMat *src, SparseMat *dst);
int  sm_transpose_fast(const SparseMat *src, SparseMat *dst);
int  sm_add(const SparseMat *a, const SparseMat *b, SparseMat *out);
int  sm_get(const SparseMat *m, int r, int c, Tile *out);

/* 引擎用：按种子生成一张 100×100 地图（里面有农场地块对应的 type） */
void map_generate(SparseMat *m, unsigned seed);

#endif /* MAP_H */
