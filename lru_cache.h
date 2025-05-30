#pragma once
#include <list>
#include <unordered_map>
#include <optional>
#include <chrono>

#include "cache.h"

template<typename Key, typename Value>
class LRUCache : public Cache<Key, Value> {
public:
    explicit LRUCache(unsigned long capacity, int ttl_seconds)
        : capacity(capacity), ttl(ttl_seconds) {
        map.reserve(capacity);
    }

    void put(const Key &key, const Value &value) override {
        auto now_time = now();
        auto expiration = now_time + std::chrono::seconds(ttl);

        auto it = map.find(key);
        if (it != map.end()) {
            it->second->second.value = value;
            it->second->second.expiration = expiration;
            touch(it);
        } else {
            lruList.emplace_front(key, Item{value, expiration});
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

    std::optional<std::reference_wrapper<Value>> get(const Key &key) override {
        auto it = map.find(key);
        if (it == map.end()) return std::nullopt;

        auto &item = it->second->second;
        if (expired(item)) {
            remove(key);
            return std::nullopt;
        }

        touch(it);
        return item.value;
    }

    void evict() override {
        while (!lruList.empty()) {
            auto &last = lruList.back();
            if (expired(last.second)) {
                map.erase(last.first);
                lruList.pop_back();
            } else {
                break;
            }
        }
    }

    bool needEvict() override {
        return map.size() > capacity;
    }

    size_t size() override {
        return map.size();
    }

private:
    struct Item {
        Value value;
        std::chrono::steady_clock::time_point expiration;
    };

    std::list<std::pair<Key, Item>> lruList;
    std::unordered_map<Key, typename std::list<std::pair<Key, Item>>::iterator> map;
    unsigned long capacity;
    int ttl;

    static std::chrono::steady_clock::time_point now() {
        return std::chrono::steady_clock::now();
    }

    bool expired(const Item &item) {
        return now() > item.expiration;
    }

    void touch(typename std::unordered_map<Key, typename std::list<std::pair<Key, Item>>::iterator>::iterator it) {
        lruList.splice(lruList.begin(), lruList, it->second);
    }
};