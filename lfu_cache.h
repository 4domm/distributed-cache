#pragma once
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <functional>

#include "cache.h"

template<typename Key, typename Value>
class LFUCache : public Cache<Key, Value> {
public:
    explicit LFUCache(size_t capacity)
        : capacity(capacity), size_(0) {
    }

    void put(const Key &key, const Value &value) override {
        auto it = byKey.find(key);
        if (it != byKey.end()) {
            it->second->value = value;
            increment(it->second);
        } else {
            auto *item = new CacheItem{key, value, freqs.end()};
            byKey[key] = item;
            ++size_;
            increment(item);
        }
    }

    std::optional<std::reference_wrapper<Value> > get(const Key &key) override {
        auto it = byKey.find(key);
        if (it == byKey.end()) return std::nullopt;
        increment(it->second);
        return it->second->value;
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
    };

    std::unordered_map<Key, CacheItem *> byKey;
    std::list<FrequencyItem> freqs;
    size_t capacity;
    size_t size_;

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
