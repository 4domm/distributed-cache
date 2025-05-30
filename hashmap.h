#pragma once
#include <vector>
#include <list>
#include <functional>
#include <optional>
#include <cstddef>
#include <utility>
#include "hashmap.h"

template<typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename KeyEq = std::equal_to<Key> >
class HashMap {
public:
    using KVPair = std::pair<Key, Value>;
    using Bucket = std::list<KVPair>;

    explicit HashMap(std::size_t initial_buckets = 4)
        : newer_(initial_buckets),
          older_(),
          migrate_pos_(0),
          size_(0) {
    }

    std::optional<Value> get(const Key &key) {
        helpRehash();
        std::size_t h = hasher_(key);
        if (auto val = findIn(newer_, h, key).has_value()) return val;
        return findIn(older_, h, key);
    }

    void insert(const Key &key, Value value) {
        helpRehash();
        std::size_t h = hasher_(key);
        auto &bucket = bucketFor(newer_, h);
        for (auto &kv: bucket) {
            if (KeyEq{}(kv.first, key)) {
                kv.second = std::move(value);
                return;
            }
        }
        bucket.emplace_front(key, std::move(value));
        ++size_;
        if (older_.empty()) {
            std::size_t threshold = newer_.size() * kMaxLoadFactor;
            if (size_ > threshold) triggerRehash();
        }
        helpRehash();
    }

    bool erase(const Key &key) {
        helpRehash();
        std::size_t h = hasher_(key);
        if (eraseFrom(newer_, h, key)) {
            --size_;
            return true;
        }
        if (eraseFrom(older_, h, key)) {
            --size_;
            return true;
        }
        return false;
    }

    void clear() {
        newer_.clear();
        older_.clear();
        size_ = 0;
        migrate_pos_ = 0;
    }

    std::size_t size() const noexcept {
        return size_;
    }

private:
    static constexpr std::size_t kMaxLoadFactor = 8;
    static constexpr std::size_t kRehashWork = 128;

    std::vector<Bucket> newer_, older_;
    std::size_t migrate_pos_;
    std::size_t size_;
    Hash hasher_;

    void triggerRehash() {
        older_ = std::move(newer_);
        newer_.assign(older_.size() * 2, Bucket{});
        migrate_pos_ = 0;
    }

    void helpRehash() {
        std::size_t work = 0;
        while (work < kRehashWork && migrate_pos_ < older_.size()) {
            auto &bucket = older_[migrate_pos_];
            if (bucket.empty()) {
                ++migrate_pos_;
                continue;
            }
            auto it = bucket.begin();
            auto kv = std::move(*it);
            bucket.erase(it);
            std::size_t h = hasher_(kv.first);
            bucketFor(newer_, h).emplace_front(std::move(kv));
            ++work;
        }
        if (migrate_pos_ >= older_.size()) {
            older_.clear();
            migrate_pos_ = 0;
        }
    }

    static Bucket &bucketFor(std::vector<Bucket> &table, std::size_t h) {
        return table[h & (table.size() - 1)];
    }

    static bool eraseFrom(std::vector<Bucket> &table, std::size_t h, const Key &key) {
        auto &bucket = bucketFor(table, h);
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (KeyEq{}(it->first, key)) {
                bucket.erase(it);
                return true;
            }
        }
        return false;
    }

    std::optional<Value> findIn(
        std::vector<Bucket> &table,
        std::size_t h,
        const Key &key
    ) {
        if (table.empty()) return std::nullopt;
        auto &bucket = bucketFor(table, h);
        for (auto &kv: bucket) {
            if (KeyEq{}(kv.first, key)) return kv.second;
        }
        return std::nullopt;
    }
};