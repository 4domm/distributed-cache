#pragma once
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <functional>
#include <chrono>

#include "cache.h"

template<typename Key, typename Value>
class LFUCache : public Cache<Key, Value> {
public:
    explicit LFUCache(size_t capacity, int ttl_seconds)
        : capacity(capacity), size_(0), ttl(ttl_seconds) {
        byKey.reserve(capacity);
    }

    void put(const Key &key, const Value &value) override {
        auto now_time = now();
        auto expiration_time = now_time + std::chrono::seconds(ttl);

        auto it = byKey.find(key);
        if (it != byKey.end()) {
            it->second->value = value;
            it->second->expiration = expiration_time;
            increment(it->second);
        } else {
            auto *item = new CacheItem{key, value, freqs.end(), expiration_time};
            byKey[key] = item;
            ++size_;
            increment(item);
        }
    }

    std::optional<std::reference_wrapper<Value>> get(const Key &key) override {
        auto it = byKey.find(key);
        if (it == byKey.end()) return std::nullopt;

        auto *item = it->second;
        if (expired(item)) {
            remove(key);
            return std::nullopt;
        }

        increment(item);
        return item->value;
    }

    size_t remove(const Key &key) override {
        auto it = byKey.find(key);
        if (it == byKey.end()) return 0;
        auto *item = it->second;
        auto freqIt = item->freqIter;
        removeEntry(freqIt, item);
        byKey.erase(it);
        delete item;
        --size_;
        return 1;
    }

    bool needEvict() override {
        return size_ > capacity;
    }

    void evict() override {
        evict(1);
    }

    size_t size() override {
        return size_;
    }

    ~LFUCache() override {
        for (auto &pair: byKey) {
            delete pair.second;
        }
    }

private:
    struct CacheItem;

    struct FrequencyItem {
        int freq;
        std::unordered_set<CacheItem *> entries;
    };

    struct CacheItem {
        Key key;
        Value value;
        typename std::list<FrequencyItem>::iterator freqIter;
        std::chrono::steady_clock::time_point expiration;
    };

    std::unordered_map<Key, CacheItem *> byKey;
    std::list<FrequencyItem> freqs;
    size_t capacity;
    size_t size_;
    int ttl;

    static std::chrono::steady_clock::time_point now() {
        return std::chrono::steady_clock::now();
    }

    bool expired(CacheItem *item) {
        return now() > item->expiration;
    }

    void increment(CacheItem *item) {
        auto curIt = item->freqIter;
        int nextFreq = 1;
        typename std::list<FrequencyItem>::iterator nextIt;

        if (curIt == freqs.end()) {
            nextIt = freqs.begin();
        } else {
            nextFreq = curIt->freq + 1;
            nextIt = std::next(curIt);
        }

        if (nextIt == freqs.end() || nextIt->freq != nextFreq) {
            FrequencyItem node{nextFreq, {}};
            if (curIt == freqs.end()) {
                freqs.push_front(std::move(node));
                nextIt = freqs.begin();
            } else {
                nextIt = freqs.insert(nextIt, std::move(node));
            }
        }

        nextIt->entries.insert(item);
        item->freqIter = nextIt;

        if (curIt != freqs.end()) {
            removeEntry(curIt, item);
        }
    }

    void removeEntry(typename std::list<FrequencyItem>::iterator freqIt, CacheItem *item) {
        auto &entries = freqIt->entries;
        entries.erase(item);
        if (entries.empty()) {
            freqs.erase(freqIt);
        }
    }

    void evict(size_t count) {
        size_t removed = 0;
        while (removed < count && !freqs.empty()) {
            auto freqIt = freqs.begin();
            for (auto it = freqIt->entries.begin(); it != freqIt->entries.end() && removed < count;) {
                CacheItem *ci = *it;
                it = freqIt->entries.erase(it);
                byKey.erase(ci->key);
                delete ci;
                ++removed;
                --size_;
            }
            if (freqIt->entries.empty()) {
                freqs.erase(freqIt);
            }
        }
    }
};