#pragma once
#include <vector>
#include <functional>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <utility>

template <class K, class V,
          class Hasher = std::hash<K>,
          class KeyEqual = std::equal_to<K>>
class HashMap {
public:
    explicit HashMap(std::size_t initial_bucket_count = 4,
                              double max_load = 1.0,
                              std::size_t move_per_op = 1)
        : hasher_(), eq_(),
          max_load_factor_(max_load),
          move_per_op_(move_per_op),
          size_(0),
          rehash_idx_(-1)
    {
        if (initial_bucket_count < 1) initial_bucket_count = 1;
        ht_[0].init(next_pow2_(initial_bucket_count));
    }

    ~HashMap() { destroy_all_nodes_(); }

    HashMap(const HashMap&) = delete;
    HashMap& operator=(const HashMap&) = delete;
    HashMap(HashMap&&) = delete;
    HashMap& operator=(HashMap&&) = delete;

    void insert_or_assign(const K& key, const V& value) {
        rehash_step(move_per_op_);
        const std::size_t h = hasher_(key);

        if (try_update_(ht_[0], h, key, value)) return;
        if (is_rehashing_() && try_update_(ht_[1], h, key, value)) return;

        Table& t = is_rehashing_() ? ht_[1] : ht_[0];
        const std::size_t idx = h & t.mask;
        Node* n = new Node{h, key, value, t.buckets[idx]};
        t.buckets[idx] = n;
        ++size_;

        if (!is_rehashing_()) maybe_expand_();
    }

    bool insert(const K& key, const V& value) {
        rehash_step(move_per_op_);
        const std::size_t h = hasher_(key);

        if (find_node_(ht_[0], h, key) || (is_rehashing_() && find_node_(ht_[1], h, key)))
            return false;

        Table& t = is_rehashing_() ? ht_[1] : ht_[0];
        const std::size_t idx = h & t.mask;
        Node* n = new Node{h, key, value, t.buckets[idx]};
        t.buckets[idx] = n;
        ++size_;

        if (!is_rehashing_()) maybe_expand_();
        return true;
    }

    std::optional<V> get(const K& key) {
        rehash_step(move_per_op_);
        const std::size_t h = hasher_(key);

        if (Node* n = find_node_(ht_[0], h, key)) return n->value;
        if (is_rehashing_()) if (Node* n = find_node_(ht_[1], h, key)) return n->value;
        return std::nullopt;
    }

    bool find(const K& key, V& out) {
        rehash_step(move_per_op_);
        const std::size_t h = hasher_(key);

        if (Node* n = find_node_(ht_[0], h, key)) { out = n->value; return true; }
        if (is_rehashing_()) if (Node* n = find_node_(ht_[1], h, key)) { out = n->value; return true; }
        return false;
    }

    bool contains(const K& key) {
        rehash_step(move_per_op_);
        const std::size_t h = hasher_(key);
        return find_node_(ht_[0], h, key) || (is_rehashing_() && find_node_(ht_[1], h, key));
    }

    bool erase(const K& key) {
        rehash_step(move_per_op_);
        const std::size_t h = hasher_(key);

        if (erase_from_(ht_[0], h, key)) { --size_; return true; }
        if (is_rehashing_() && erase_from_(ht_[1], h, key)) { --size_; return true; }
        return false;
    }

    void rehash_step(std::size_t steps = 1) {
        if (!is_rehashing_()) return;
        if (steps == 0) steps = 1;

        std::size_t moved_non_empty = 0;
        const std::size_t old_cap = ht_[0].capacity();

        while (rehash_idx_ < static_cast<long long>(old_cap) && moved_non_empty < steps) {
            Node* node = ht_[0].buckets[rehash_idx_];
            if (!node) { ++rehash_idx_; continue; }

            ht_[0].buckets[rehash_idx_] = nullptr;
            while (node) {
                Node* next = node->next;
                const std::size_t idx = node->hash & ht_[1].mask;
                node->next = ht_[1].buckets[idx];
                ht_[1].buckets[idx] = node;
                node = next;
            }
            ++rehash_idx_;
            ++moved_non_empty;
        }

        if (rehash_idx_ >= static_cast<long long>(old_cap)) {
            ht_[0] = std::move(ht_[1]);
            ht_[1].reset();
            rehash_idx_ = -1;
        }
    }

    bool rehash_in_progress() const   { return is_rehashing_(); }
    double load_factor() const   {
        return ht_[0].capacity() ? double(size_) / double(capacity()) : 0.0;
    }

    std::size_t size() const   { return size_; }
    std::size_t capacity() const   {
        return ht_[0].capacity() + (is_rehashing_() ? ht_[1].capacity() : 0);
    }

    void clear() {
        destroy_all_nodes_();
        size_ = 0;
        ht_[1].reset();
        rehash_idx_ = -1;
        ht_[0].init(4);
    }

    void reserve(std::size_t n_elems) {
        const double target_lf = max_load_factor_;
        const std::size_t need = std::max<std::size_t>(
            4, next_pow2_(std::size_t(double(n_elems) / target_lf + 0.999)));
        if (!is_rehashing_() && need > ht_[0].capacity()) start_rehash_to_(need);
    }

    void set_max_load_factor(double f) { max_load_factor_ = (f <= 0.0 ? 1.0 : f); }
    void set_move_per_op(std::size_t n) { move_per_op_ = (n == 0 ? 1 : n); }

private:
    struct Node {
        std::size_t hash;
        K key;
        V value;
        Node* next;
    };

    struct Table {
        std::vector<Node*> buckets;
        std::size_t mask = 0;

        void init(std::size_t cap_pow2) {
            buckets.assign(cap_pow2, nullptr);
            mask = cap_pow2 - 1;
        }
        void reset() {
            buckets.clear();
            buckets.shrink_to_fit();
            mask = 0;
        }
        std::size_t capacity() const   { return buckets.size(); }
    };

    Hasher hasher_;
    KeyEqual eq_;
    double max_load_factor_;
    std::size_t move_per_op_;
    std::size_t size_;
    Table ht_[2];
    long long rehash_idx_; 

    static std::size_t next_pow2_(std::size_t x) {
        if (x <= 1) return 1;
        --x;
        for (std::size_t i = 1; i < sizeof(std::size_t) * 8; i <<= 1) x |= x >> i;
        return x + 1;
    }

    bool is_rehashing_() const   { return rehash_idx_ != -1; }

    Node* find_node_(Table& t, std::size_t h, const K& key) const {
        if (t.capacity() == 0) return nullptr;
        Node* n = t.buckets[h & t.mask];
        while (n) {
            if (n->hash == h && eq_(n->key, key)) return n;
            n = n->next;
        }
        return nullptr;
    }

    bool try_update_(Table& t, std::size_t h, const K& key, const V& value) {
        Node* n = find_node_(t, h, key);
        if (n) { n->value = value; return true; }
        return false;
    }

    bool erase_from_(Table& t, std::size_t h, const K& key) {
        if (t.capacity() == 0) return false;
        const std::size_t idx = h & t.mask;
        Node** pp = &t.buckets[idx];
        while (*pp) {
            Node* cur = *pp;
            if (cur->hash == h && eq_(cur->key, key)) {
                *pp = cur->next;
                delete cur;
                return true;
            }
            pp = &cur->next;
        }
        return false;
    }

    void start_rehash_to_(std::size_t new_cap_pow2) {
        if (is_rehashing_()) return;
        if (new_cap_pow2 == ht_[0].capacity()) return;
        ht_[1].init(new_cap_pow2);
        rehash_idx_ = 0;
    }

    void maybe_expand_() {
        const double lf = double(size_) / double(ht_[0].capacity());
        if (lf > max_load_factor_) {
            const std::size_t new_cap = ht_[0].capacity() ? ht_[0].capacity() * 2 : 4;
            start_rehash_to_(new_cap);
        }
    }

    void destroy_table_nodes_(Table& t) {
        for (Node*& head : t.buckets) {
            while (head) {
                Node* nxt = head->next;
                delete head;
                head = nxt;
            }
            head = nullptr;
        }
    }

    void destroy_all_nodes_() {
        destroy_table_nodes_(ht_[0]);
        destroy_table_nodes_(ht_[1]);
    }
};

