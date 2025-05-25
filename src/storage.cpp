#include "storage.h"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>


void KVstorage::put(const std::string &key, const std::string &value) {
    std::lock_guard lock(mutex);
    cache[key] = value;
}

size_t KVstorage::remove(const std::string &key) {
    std::lock_guard lock(mutex);
    return cache.erase(key);
}


std::optional<std::reference_wrapper<std::string> > KVstorage::get(const std::string &key) {
    std::lock_guard lock(mutex);
    if (cache.count(key) > 0) {
        return cache.at(key);
    }
    return std::nullopt;
}


std::pair<int, int> KVstorage::getLoad() {
    return {0, 0}; // cpu, mem
}
