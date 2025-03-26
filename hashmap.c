#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct DictNode {
    uint64_t hashVal;
    struct DictNode *next;
} DictNode;

typedef struct Dict {
    DictNode **table;
    size_t mask;
    size_t size;
} Dict;

static void dict_init(Dict *dict, const size_t n) {
    dict->table = (DictNode **) calloc(n, sizeof(DictNode *));
    dict->size = 0;
    dict->mask = n - 1;
}

static void dict_insert(Dict *dict, DictNode *node) {
    size_t pos = node->hashVal & dict->mask;
    node->next = dict->table[pos];
    dict->table[pos] = node;
    dict->size++;
}

