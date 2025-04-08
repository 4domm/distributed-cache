#pragma once


#include <cstdint>


struct DictNode {
    uint64_t hashVal;
    DictNode *next;
};

struct Dict {
    DictNode **table;
    size_t mask;
    size_t size; 
};


struct HashMap {
    Dict *oldDict;
    Dict *newDict;
    size_t curr_migrate;
};
