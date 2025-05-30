#include "storage.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <iostream>

#include "arc_cache.h"
#include "lru_cache.h"
#include "network.h"
#include "lfu_cache.h"
#include "twoq_cache.h"
#include "random_cache.h"

std::unordered_map<std::string, std::string> parseConfig(const std::string &filename) {
    std::unordered_map<std::string, std::string> result;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "cant open config" << std::endl;
        return result;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        result[key] = value;
    }

    return result;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <shard_number>" << std::endl;
        return 1;
    }
    int instance = std::stoi(argv[1]);
    auto cfg = parseConfig("config.cfg");
    std::vector<std::string> shards;
    for (int i = 0;; ++i) {
        std::string key = "shard" + std::to_string(i);
        if (auto it = cfg.find(key); it != cfg.end()) {
            shards.push_back(it->second);
        } else break;
    }
    std::cout << shards.size() << "\n";
    if (instance < 0 || instance >= static_cast<int>(shards.size())) {
        std::cerr << "Invalid shard number: " << instance << std::endl;
        return 1;
    }
    if (shards.size() < 0) {
        std::cerr << "Invalid shard number: " << instance << std::endl;
        return 1;
    }
    std::string selfAddr = shards[instance];
    auto pos = selfAddr.rfind(':');
    std::string host = selfAddr.substr(0, pos);
    int port = std::stoi(selfAddr.substr(pos + 1));
    size_t capacity = 1000;
    if (auto it = cfg.find("capacity"); it != cfg.end()) {
        capacity = std::stoul(it->second);
    }
    std::string algo = "lru";
    if (auto it = cfg.find("algo"); it != cfg.end()) {
        algo = it->second;
    }
    Poco::Net::ServerSocket socket(port);
    auto *params = new Poco::Net::HTTPServerParams;
    params->setMaxThreads(24);
    params->setKeepAlive(true);
    int ttl = 36000;
    if (auto it = cfg.find("ttl"); it != cfg.end()) {
        ttl = std::atoi(it->second.c_str());
    }
    Cache<std::string, std::string> *cache = nullptr;
    if (algo == "lru") cache = new LRUCache<std::string, std::string>(capacity, ttl);
    else if (algo == "lfu") cache = new LFUCache<std::string, std::string>(capacity, ttl);
    else if (algo == "2q") cache = new TwoQCache<std::string, std::string>(capacity, ttl);
    else if (algo == "arc") cache = new ARCCache<std::string, std::string>(capacity, ttl);
    else if (algo == "rand") cache = new RandomCache<std::string, std::string>(capacity,ttl);
    else {
        std::cerr << "Unknown algorithm: " << algo << std::endl;
        cache = new LRUCache<std::string, std::string>(capacity, ttl);
    }
    auto *storage = new KVstorage(cache, capacity);
    Poco::Net::HTTPServer server(new HandlerFactory(storage, shards, instance), socket, params);
    server.start();
    storage->startEviction();
    std::cout << "Shard " << instance << " serving at " << host << ":" << port
            << ", cache=" << algo << ", cap=" << capacity << std::endl;
    std::cin.get();
    server.stop();
    delete storage;
}
