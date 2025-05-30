#pragma once
#include <mutex>
#include <optional>
#include <string>
#include <atomic>
#include "cache.h"
#include <future>
#include <fstream>
#include <iostream>
#include <shared_mutex>
#include <thread>

template<typename Key, typename Value>
class KVstorage {
public:
    explicit KVstorage(Cache<Key, Value> *cache, unsigned long capacity): cache(cache), capacity(capacity) {
    }

    void put(const std::string &key, const std::string &value) {
        std::unique_lock lock(mutex);
        cache->put(key, value);
    }

    size_t remove(const std::string &key) {
        std::unique_lock lock(mutex);
        return cache->remove(key);
    }

    std::optional<std::reference_wrapper<std::string> > get(const std::string &key) {
        std::shared_lock lock(mutex);
        return cache->get(key);
    }

    std::pair<double, int> getLoad() {
        std::ifstream statusFile("/proc/self/status");
        std::string line;

        while (std::getline(statusFile, line)) {
            if (line.compare(0, 6, "VmRSS:") == 0) {
                if (const size_t pos = line.find_first_of("0123456789"); pos != std::string::npos) {
                    return std::pair{static_cast<double>(std::stoi(line.substr(pos))) / 1024.0, cache->size()};
                }
            }
        }
        return {0, 0};
    }

    void startEviction() {
        if (runningEviction.load()) {
            return;
        }
        runningEviction.store(true);

        evictionTask = std::async(std::launch::async, [this] {
            while (runningEviction.load()) {
                std::this_thread::sleep_for(std::chrono::seconds(3));

                int attempts = 0;
                while (cache->needEvict() && attempts++ < 4000) {
                    {
                        std::unique_lock lock(mutex);
                        cache->evict();
                    }
                }
            }
        });
    }

    ~KVstorage() {
        runningEviction.store(false);
        if (evictionTask.valid()) {
            evictionTask.wait();
        }
        delete cache;
    }

private:
    Cache<Key, Value> *cache;
    std::shared_mutex mutex;
    unsigned long capacity;
    std::atomic<bool> runningEviction{false};
    std::future<void> evictionTask;
};
