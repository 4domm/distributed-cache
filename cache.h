#pragma once
#include <optional>

template<typename Key, typename Value>
class Cache {
public:
    virtual void put(const Key &key, const Value &value) = 0;

    virtual size_t remove(const Key &key) = 0;

    virtual std::optional<std::reference_wrapper<Value> > get(const Key &key) = 0;

    virtual bool evict() = 0;

    virtual ~Cache() = default;
};
