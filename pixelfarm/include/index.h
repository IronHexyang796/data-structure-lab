/* ============================================================================
 *  include/index.h  ——  第 10 周：图鉴检索（哈希 + 二分）
 *  老师提供，只读。
 *
 *  注意：节点类型叫 HashNode，不叫 HNode ——
 *        因为第 6 周的 huffman.h 里已经有一个 HNode 了，同名会编译不过。
 * ========================================================================== */
#ifndef INDEX_H
#define INDEX_H
#include "farm.h"

typedef struct HashNode {
    int    key;
    Item   val;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode **bucket;
    int        cap;        /* 桶数 */
    int        size;       /* 元素数 */
} HashMap;

typedef struct {
    HashMap hot;           /* 热数据 */
    Item   *cold;          /* 冷数据，按 id 升序 */
    int     cold_n;
    long    hit_hot, hit_cold, miss;
} Index;

int  hash_init(HashMap *h, int cap);
void hash_destroy(HashMap *h);
int  hash_put(HashMap *h, int key, const Item *val);
int  hash_get(const HashMap *h, int key, Item *out);
int  hash_remove(HashMap *h, int key);

int  bsearch_by_id(const Item *sorted, int n, int key);

int  index_build(Index *ix, const Item *all, int n, int hot_n);
int  index_lookup(Index *ix, int key, Item *out);
void index_destroy(Index *ix);

/* 引擎用：造一批图鉴数据做测试 */
void index_demo_data(Item *out, int n);

#endif /* INDEX_H */
