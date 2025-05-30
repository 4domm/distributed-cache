#include <iostream>
#include <chrono>

#include "hashmap.h"
#include "lru_cache.h"
#include "storage.h"
#include <unordered_map>

void benchLRU() {
    constexpr int total = 1000000;

    auto *storage = new KVstorage(new LRUCache<std::string, std::string>(100000000), 100000000);

    double total_latency = 0;

    for (int i = 0; i < total; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);

        auto start = std::chrono::high_resolution_clock::now();
        storage->put(key, value);
        auto end = std::chrono::high_resolution_clock::now();

        double latency = std::chrono::duration<double, std::milli>(end - start).count();
        total_latency += latency;
    }

    std::cout << "total latency for " << total << " op " << total_latency << " milliseconds" << std::endl;

    delete storage;
}

void benchOwnHashMap() {
    constexpr int total = 1000000;
    auto ownHMap = new HashMap<std::string, std::string>();
    double total_latency = 0;
    for (int i = 0; i < total; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        auto start = std::chrono::high_resolution_clock::now();
        ownHMap->insert(key, value);
        auto end = std::chrono::high_resolution_clock::now();
        double latency = std::chrono::duration<double, std::milli>(end - start).count();
        total_latency += latency;
    }
    std::cout << "total latency for own hashmap with total" << total << " op " << total_latency << " milliseconds" <<
            std::endl;
    delete ownHMap;
}

void benchSTLHashMap() {
    constexpr int total = 1000000;
    auto stlHMAP = std::unordered_map<std::string, std::string>(); // test with no reserve
    double total_latency = 0;
    for (int i = 0; i < total; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        auto start = std::chrono::high_resolution_clock::now();
        stlHMAP.insert({key, value});
        auto end = std::chrono::high_resolution_clock::now();
        double latency = std::chrono::duration<double, std::milli>(end - start).count();
        total_latency += latency;
    }
    std::cout << "total latency for stl hashmap with total" << total << " op " << total_latency << " milliseconds" <<
            std::endl;
}

int main() {
    benchLRU();
    benchOwnHashMap();
    benchSTLHashMap();
}
