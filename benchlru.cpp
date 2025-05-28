#include <iostream>
#include <chrono>
#include "lru_cache.h"
#include "storage.h"

void benchLRU() {
    constexpr int total = 1000000;

    auto *storage = new KVstorage(new LRUCache<std::string, std::string>);

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

    std::cout << "total latency for "<<total<<" op "<< total_latency << " milliseconds" << std::endl;

    delete storage;
}

int main() {
    benchLRU();
}
