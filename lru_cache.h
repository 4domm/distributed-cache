#pragma once
#include <list>
#include <unordered_map>

#include "cache.h"

template<typename Key, typename Value>
class LRUCache : public Cache<Key, Value> {
public:
    explicit LRUCache(unsigned long capacity): capacity(capacity) {
        map.reserve(capacity);
    }

    void put(const Key &key, const Value &value) override {
        auto it = map.find(key);
        if (it != map.end()) {
            it->second->second = value;
            touch(it);
        } else {
            lruList.emplace_front(key, value);
            map[key] = lruList.begin();
        }
    }

    size_t remove(const Key &key) override {
        auto it = map.find(key);
        if (it == map.end()) return 0;
        lruList.erase(it->second);
        map.erase(it);
        return 1;
    }

    std::optional<std::reference_wrapper<Value> > get(const Key &key) override {
        auto it = map.find(key);
        if (it == map.end()) return std::nullopt;
        touch(it);
        return it->second->second;
    }

    void evict() override {
        if (lruList.empty()) return;
        auto &last = lruList.back();
        map.erase(last.first);
        lruList.pop_back();
    }

    bool needEvict() override {
        return map.size() > capacity;
    }

    size_t size() override {
        return map.size();
    }

private:
    std::list<std::pair<Key, Value> > lruList;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value> >::iterator> map;
    unsigned long capacity;

    void touch(typename std::unordered_map<Key, typename std::list<std::pair<Key, Value> >::iterator>::iterator it) {
        lruList.splice(lruList.begin(), lruList, it->second);
    }
};
