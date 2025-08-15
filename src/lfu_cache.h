#pragma once
#include <chrono>
#include <list>
#include <optional>
#include <unordered_set>

#include "cache.h"
#include "dict.h"

template <typename Key, typename Value>
class LFUCache : public Cache<Key, Value> {
   public:
    explicit LFUCache(size_t capacity, int ttl_seconds)
        : capacity(capacity), count(0), ttl(ttl_seconds) {
        byKey.reserve(capacity);
    }

    void put(const Key& key, const Value& value) override {
        auto t = now();
        auto exp = t + std::chrono::seconds(ttl);
        if (auto it = byKey.get(key)) {
            auto* item = *it;
            item->value = value;
            item->expiration = exp;
            increment(item);
        } else {
            auto* item = new CacheItem{key, value, freqs.end(), exp};
            byKey.insert_or_assign(key, item);
            ++count;
            increment(item);
        }
    }

    std::optional<Value> get(const Key& key) override {
        auto it = byKey.get(key);
        if (!it) return std::nullopt;
        auto* item = *it;
        if (expired(item)) {
            remove(key);
            return std::nullopt;
        }
        increment(item);
        return item->value;
    }

    size_t remove(const Key& key) override {
        auto it = byKey.get(key);
        if (!it) return 0;
        auto* item = *it;
        auto freqIt = item->freqIter;
        removeEntry(freqIt, item);
        byKey.erase(key);
        delete item;
        --count;
        return 1;
    }

    bool needEvict() override {
        return count > capacity;
    }

    void evict() override {
        if (count == 0 || freqs.empty()) return;

        auto freqIt = freqs.begin();
        if (freqIt->entries.empty()) {
            freqs.erase(freqIt);
            return;
        }

        auto it = freqIt->entries.begin();
        CacheItem* ci = *it;

        freqIt->entries.erase(it);
        byKey.erase(ci->key);
        delete ci;
        --count;

        if (freqIt->entries.empty()) {
            freqs.erase(freqIt);
        }
    }

    size_t size() override {
        return count;
    }

    ~LFUCache() override {
        for (auto& f : freqs) {
            for (auto* p : f.entries) delete p;
        }
    }

   private:
    struct CacheItem;

    struct FrequencyItem {
        int freq;
        std::unordered_set<CacheItem*> entries;
    };

    struct CacheItem {
        Key key;
        Value value;
        typename std::list<FrequencyItem>::iterator freqIter;
        std::chrono::steady_clock::time_point expiration;
    };

    HashMap<Key, CacheItem*> byKey;
    std::list<FrequencyItem> freqs;
    size_t capacity;
    size_t count;
    int ttl;

    static std::chrono::steady_clock::time_point now() {
        return std::chrono::steady_clock::now();
    }

    bool expired(CacheItem* item) const {
        return now() > item->expiration;
    }

    void increment(CacheItem* item) {
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

    void removeEntry(typename std::list<FrequencyItem>::iterator freqIt, CacheItem* item) {
        auto& entries = freqIt->entries;
        entries.erase(item);
        if (entries.empty()) {
            freqs.erase(freqIt);
        }
    }
};