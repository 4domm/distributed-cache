#pragma once
#include <list>
#include <unordered_map>
#include <optional>
#include <functional>
#include <chrono>
#include "cache.h"

template<typename Key, typename Value>
class TwoQCache : public Cache<Key, Value> {
public:
    explicit TwoQCache(size_t capacity, int ttl_seconds)
        : capacity(capacity),
          kin(std::max<size_t>(1, capacity / 4)),
          AmCap(capacity - kin),
          ttl(ttl_seconds)
    {}

    std::optional<std::reference_wrapper<Value>> get(const Key &key) override {
        auto now_time = now();
        if (mapAm.count(key)) {
            auto it = mapAm[key];
            if (expired(it->second)) {
                remove(key);
                return std::nullopt;
            }
            auto kv = *it;
            Am.erase(it);
            kv.second.expiration = now_time + std::chrono::seconds(ttl);
            Am.push_front(kv);
            mapAm[key] = Am.begin();
            return kv.second.value;
        }
        if (mapA1.count(key)) {
            auto it = mapA1[key];
            if (expired(it->second)) {
                remove(key);
                return std::nullopt;
            }
            auto kv = *it;
            A1.erase(it);
            mapA1.erase(key);
            kv.second.expiration = now_time + std::chrono::seconds(ttl);
            Am.push_front(kv);
            mapAm[key] = Am.begin();
            if (Am.size() > AmCap) {
                auto last = Am.back();
                Am.pop_back();
                mapAm.erase(last.first);
            }
            return kv.second.value;
        }
        return std::nullopt;
    }

    void put(const Key &key, const Value &value) override {
        auto now_time = now();
        auto expiration = now_time + std::chrono::seconds(ttl);

        if (mapAm.count(key) || mapA1.count(key)) {
            if (mapAm.count(key)) {
                mapAm[key]->second.value = value;
                mapAm[key]->second.expiration = expiration;
            } else {
                mapA1[key]->second.value = value;
                mapA1[key]->second.expiration = expiration;
            }
            get(key);
            return;
        }

        if (mapA1out.count(key)) {
            replace(true);
            A1out.erase(mapA1out[key]);
            mapA1out.erase(key);
            Am.push_front({key, Item{value, expiration}});
            mapAm[key] = Am.begin();
            return;
        }

        if (A1.size() + Am.size() >= capacity) {
            replace(false);
        }

        A1.push_front({key, Item{value, expiration}});
        mapA1[key] = A1.begin();
    }

    size_t remove(const Key &key) override {
        if (mapAm.count(key)) {
            Am.erase(mapAm[key]);
            mapAm.erase(key);
            return 1;
        }
        if (mapA1.count(key)) {
            A1.erase(mapA1[key]);
            mapA1.erase(key);
            return 1;
        }
        return 0;
    }

    bool needEvict() override {
        return (A1.size() + Am.size()) > capacity;
    }

    void evict() override {
        replace(false);
    }

    size_t size() override {
        return A1.size() + Am.size();
    }

private:
    struct Item {
        Value value;
        std::chrono::steady_clock::time_point expiration;
    };

    size_t capacity;
    size_t kin;
    size_t AmCap;
    int ttl;

    std::list<std::pair<Key, Item>> A1, Am;
    std::list<Key> A1out;
    std::unordered_map<Key, typename std::list<std::pair<Key, Item>>::iterator> mapA1, mapAm;
    std::unordered_map<Key, typename std::list<Key>::iterator> mapA1out;

    static std::chrono::steady_clock::time_point now() {
        return std::chrono::steady_clock::now();
    }

    bool expired(const Item &item) {
        return now() > item.expiration;
    }

    void replace(bool isGhost) {
        if (A1.size() >= kin && (!isGhost || A1.size() > kin)) {
            auto last = A1.back();
            A1.pop_back();
            mapA1.erase(last.first);
            A1out.push_front(last.first);
            mapA1out[last.first] = A1out.begin();
            if (A1out.size() > capacity) {
                auto glast = A1out.back();
                A1out.pop_back();
                mapA1out.erase(glast);
            }
        } else {
            auto last = Am.back();
            Am.pop_back();
            mapAm.erase(last.first);
        }
    }
};