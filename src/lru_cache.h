#pragma once
#include <chrono>
#include <cstddef>
#include <list>
#include <optional>
#include <utility>

#include "cache.h"
#include "dict.h"

template <typename Key, typename Value>
class LRUCache : public Cache<Key, Value> {
   public:
    explicit LRUCache(std::size_t capacity, int ttl_seconds)
        : capacity(capacity), ttl(ttl_seconds) {
        index.reserve(this->capacity);
    }

    void put(const Key& key, const Value& value) override {
        auto t = now();
        auto exp = t + std::chrono::seconds(ttl);
        if (auto it = index.get(key)) {
            auto li = *it;
            li->second.value = value;
            li->second.expiration = exp;
            touch(li);
        } else {
            lru.emplace_front(key, Item{value, exp});
            index.insert_or_assign(key, lru.begin());
        }
    }

    std::size_t remove(const Key& key) override {
        if (auto it = index.get(key)) {
            auto li = *it;
            lru.erase(li);
            index.erase(key);
            return 1;
        }
        return 0;
    }

    std::optional<Value> get(const Key& key) override {
        auto it = index.get(key);
        if (!it) return std::nullopt;
        auto li = *it;
        auto& item = li->second;
        if (expired(item)) {
            lru.erase(li);
            index.erase(key);
            return std::nullopt;
        }
        touch(li);
        return item.value;
    }

    void evict() override {
        while (!lru.empty()) {
            auto& last = lru.back();
            if (expired(last.second)) {
                index.erase(last.first);
                lru.pop_back();
            } else {
                break;
            }
        }
    }

    bool needEvict() override {
        return index.size() > capacity;
    }

    size_t size() override {
        return index.size();
    }

   private:
    struct Item {
        Value value;
        std::chrono::steady_clock::time_point expiration;
    };

    using ListNode = std::pair<Key, Item>;
    using ListIt = typename std::list<ListNode>::iterator;

    std::list<ListNode> lru;
    HashMap<Key, ListIt> index;
    std::size_t capacity;
    int ttl;

    static std::chrono::steady_clock::time_point now() {
        return std::chrono::steady_clock::now();
    }

    bool expired(const Item& item) const {
        return now() > item.expiration;
    }

    void touch(ListIt li) {
        lru.splice(lru.begin(), lru, li);
    }
};