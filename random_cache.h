#pragma once

#include <vector>
#include <unordered_map>
#include <random>
#include <optional>
#include <functional>
#include <chrono>

#include "cache.h"

template<typename Key, typename Value>
class RandomCache : public Cache<Key, Value> {
public:
    explicit RandomCache(size_t capacity, int ttl_seconds)
        : capacity_(capacity),
          ttl_(ttl_seconds),
          engine_(std::random_device{}())
    {
        map_.reserve(capacity_);
        keys_.reserve(capacity_);
    }

    void put(const Key& key, const Value& value) override {
        auto now_time = now();
        auto expiration = now_time + std::chrono::seconds(ttl_);

        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second.value = value;
            it->second.expiration = expiration;
        }
        else {
            keys_.push_back(key);
            map_[key] = { value, keys_.size() - 1, expiration };
        }
    }

    size_t remove(const Key& key) override {
        auto it = map_.find(key);
        if (it == map_.end()) return 0;

        size_t idx = it->second.index;
        Key lastKey = keys_.back();

        if (idx != keys_.size() - 1) {
            keys_[idx] = lastKey;
            map_[lastKey].index = idx;
        }

        keys_.pop_back();
        map_.erase(it);
        return 1;
    }

    std::optional<std::reference_wrapper<Value>> get(const Key& key) override {
        auto it = map_.find(key);
        if (it == map_.end()) return std::nullopt;

        if (expired(it->second)) {
            remove(key);
            return std::nullopt;
        }

        return std::ref(it->second.value);
    }

    void evict() override {
        if (keys_.empty()) return;
        std::uniform_int_distribution<size_t> dist(0, keys_.size() - 1);
        size_t idx = dist(engine_);
        remove(keys_[idx]);
    }

    bool needEvict() override {
        return map_.size() > capacity_;
    }

    size_t size() override {
        return map_.size();
    }

private:
    struct Item {
        Value value;
        size_t index;
        std::chrono::steady_clock::time_point expiration;
    };

    size_t capacity_;
    int ttl_;
    std::vector<Key> keys_;
    std::unordered_map<Key, Item> map_;
    std::mt19937 engine_;

    static std::chrono::steady_clock::time_point now() {
        return std::chrono::steady_clock::now();
    }

    bool expired(const Item& item) const {
        return now() > item.expiration;
    }
};