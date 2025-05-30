
#pragma once
#include <list>
#include <unordered_map>
#include <optional>
#include <functional>
#include "cache.h"

template<typename Key, typename Value>
class ARCCache : public Cache<Key, Value> {
public:
    explicit ARCCache(size_t capacity)
        : capacity(capacity), p(0)
    {}

    std::optional<std::reference_wrapper<Value>> get(const Key &key) override {
        if (mapT1.count(key)) {
            auto it = mapT1[key];
            auto kv = *it;
            T1.erase(it);
            mapT1.erase(key);
            T2.push_front(kv);
            mapT2[key] = T2.begin();
            return kv.second;
        }
        if (mapT2.count(key)) {
            auto it = mapT2[key];
            auto kv = *it;
            T2.erase(it);
            T2.push_front(kv);
            mapT2[key] = T2.begin();
            return kv.second;
        }
        return std::nullopt;
    }

    void put(const Key &key, const Value &value) override {
        if (mapT1.count(key) || mapT2.count(key)) {
            if (mapT1.count(key)) {
                mapT1[key]->second = value;
                get(key);
            } else {
                mapT2[key]->second = value;
                get(key);
            }
            return;
        }
        bool inB1 = mapB1.count(key);
        bool inB2 = mapB2.count(key);
        size_t sz = T1.size() + T2.size();
        if (inB1) {
            p = std::min(capacity, p + std::max<size_t>(1, B2.size() / B1.size()));
            replace(key);
            B1.erase(mapB1[key]); mapB1.erase(key);
            T2.emplace_front(key, value);
            mapT2[key] = T2.begin();
        } else if (inB2) {
            p = (p >= std::max<size_t>(1, B1.size() / B2.size()))
                ? p - std::max<size_t>(1, B1.size() / B2.size()) : 0;
            replace(key);
            B2.erase(mapB2[key]); mapB2.erase(key);
            T2.emplace_front(key, value);
            mapT2[key] = T2.begin();
        } else {
            if (sz >= capacity) {
                replace(key);
            }
            T1.emplace_front(key, value);
            mapT1[key] = T1.begin();
        }
        if (B1.size() > capacity) {
            auto last = B1.back();
            B1.pop_back(); mapB1.erase(last);
        }
        if (B2.size() > capacity) {
            auto last = B2.back();
            B2.pop_back(); mapB2.erase(last);
        }
    }

    size_t remove(const Key &key) override {
        if (mapT1.count(key)) {
            T1.erase(mapT1[key]); mapT1.erase(key);
            return 1;
        }
        if (mapT2.count(key)) {
            T2.erase(mapT2[key]); mapT2.erase(key);
            return 1;
        }
        return 0;
    }

    bool needEvict() override {
        return (T1.size() + T2.size()) > capacity;
    }

    void evict() override {
        if (!T1.empty()) {
            auto kv = T1.back();
            mapT1.erase(kv.first);
            T1.pop_back();
            B1.push_front(kv.first);
            mapB1[kv.first] = B1.begin();
        } else if (!T2.empty()) {
            auto kv = T2.back();
            mapT2.erase(kv.first);
            T2.pop_back();
            B2.push_front(kv.first);
            mapB2[kv.first] = B2.begin();
        }
    }

    size_t size() override {
        return T1.size() + T2.size();
    }

private:
    void replace(const Key &key) {
        if (!T1.empty() && (T1.size() > p || (mapB2.count(key) && T1.size() == p))) {
            auto kv = T1.back();
            mapT1.erase(kv.first);
            T1.pop_back();
            B1.push_front(kv.first);
            mapB1[kv.first] = B1.begin();
        } else {
            auto kv = T2.back();
            mapT2.erase(kv.first);
            T2.pop_back();
            B2.push_front(kv.first);
            mapB2[kv.first] = B2.begin();
        }
    }

    size_t capacity;
    size_t p;
    std::list<std::pair<Key, Value>> T1, T2;
    std::list<Key> B1, B2;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> mapT1, mapT2;
    std::unordered_map<Key, typename std::list<Key>::iterator> mapB1, mapB2;
};
