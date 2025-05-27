#pragma once
#include <mutex>
#include <optional>
#include <string>

#include "cache.h"

#include <fstream>

template<typename Key, typename Value>
class KVstorage {
public:
    explicit KVstorage(Cache<Key, Value> *cache): cache(cache) {
    }

    void put(const std::string &key, const std::string &value) {
        std::lock_guard lock(mutex);
        cache->put(key, value);
    }

    size_t remove(const std::string &key) {
        std::lock_guard lock(mutex);
        return cache->remove(key);
    }

    std::optional<std::reference_wrapper<std::string> > get(const std::string &key) {
        std::lock_guard lock(mutex);
        return cache->get(key);
    }

    static int getLoad() {
        std::ifstream statusFile("/proc/self/status");
        std::string line;

        while (std::getline(statusFile, line)) {
            if (line.compare(0, 6, "VmRSS:") == 0) {
                if (const size_t pos = line.find_first_of("0123456789"); pos != std::string::npos) {
                    return std::stoi(line.substr(pos));
                }
            }
        }
        return 0;
    }

    ~KVstorage() {
        delete cache;
    }

private:
    Cache<Key, Value> *cache;
    std::mutex mutex;
};
