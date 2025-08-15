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

    std::optional<std::string> get(const std::string &key) {
        std::shared_lock lock(mutex);
        return cache->get(key);
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
