#pragma once


#include <cassert>
#include <cstdint>
#include <vector>

struct Node {
    char *key;
    char *data;
    size_t hashVal;
    Node *next;
};

struct HashTable {
    std::vector<Node *> tab;
    size_t mask;
    size_t size;

    explicit HashTable(const int n) {
        assert(n>0 && (n-1&n)==0);
        tab.resize(n, nullptr);
        mask = n - 1;
        size = 0;
    }

    static int getHash(char *key) {
        return 0;
    }

    void insert(char *key, char *value) {

    }
};


class HashMap {
    HashTable newer;
    HashTable older;
    size_t migrate_pos = 0;
};
