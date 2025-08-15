#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <pthread.h>

#include <csignal>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "lfu_cache.h"
#include "lru_cache.h"
#include "network.h"
#include "storage.h"

struct Config {
    std::vector<std::string> shards;
    std::size_t capacity = 1000;
    std::string algo = "lru";
    int ttl = 3600;
};

Config parseConfigJson(const std::string& filename) {
    Config cfg;
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::fprintf(stderr, "cant open config: %s\n", filename.c_str());
        return cfg;
    }
    std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    try {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(json);
        Poco::JSON::Object::Ptr obj = result.extract<Poco::JSON::Object::Ptr>();

        if (obj->has("shards")) {
            auto arr = obj->getArray("shards");
            for (std::size_t i = 0; i < arr->size(); ++i)
                cfg.shards.push_back(arr->getElement<std::string>(i));
        }
        cfg.capacity = static_cast<std::size_t>(obj->optValue<int>("capacity", static_cast<int>(cfg.capacity)));
        cfg.algo = obj->optValue<std::string>("algo", cfg.algo);
        cfg.ttl = obj->optValue<int>("ttl", cfg.ttl);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "config parse error: %s\n", e.what());
    }
    return cfg;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <shard_number> [config.json]\n", argv[0]);
        return 1;
    }

    int instance = std::stoi(argv[1]);
    std::string cfgPath = (argc >= 3) ? argv[2] : "config.json";
    Config cfg = parseConfigJson(cfgPath);
    const auto& shards = cfg.shards;

    if (shards.empty()) {
        std::fprintf(stderr, "no shards in config: %s\n", cfgPath.c_str());
        return 1;
    }
    if (instance < 0 || instance >= static_cast<int>(shards.size())) {
        std::fprintf(stderr, "Invalid shard number: %d (0..%zu)\n", instance, shards.size() - 1);
        return 1;
    }

    std::string selfAddr = shards[instance];
    auto pos = selfAddr.rfind(':');
    if (pos == std::string::npos) {
        std::fprintf(stderr, "bad shard address: %s\n", selfAddr.c_str());
        return 1;
    }
    std::string host = selfAddr.substr(0, pos);
    int port = std::stoi(selfAddr.substr(pos + 1));

    std::size_t capacity = cfg.capacity;
    std::string algo = cfg.algo;
    int ttl = cfg.ttl;

    Poco::Net::ServerSocket socket(port);
    auto* params = new Poco::Net::HTTPServerParams;
    params->setMaxThreads(24);
    params->setKeepAlive(true);

    Cache<std::string, std::string>* cache = nullptr;
    if (algo == "lru")
        cache = new LRUCache<std::string, std::string>(capacity, ttl);
    else if (algo == "lfu")
        cache = new LFUCache<std::string, std::string>(capacity, ttl);
    else {
        std::fprintf(stderr, "bad algorithm: %s (fallback to lru)\n", algo.c_str());
        cache = new LRUCache<std::string, std::string>(capacity, ttl);
    }

    auto* storage = new KVstorage(cache, capacity);
    Poco::Net::HTTPServer server(new HandlerFactory(storage, shards, instance), socket, params);
    server.start();
    storage->startEviction();

    std::printf("Shard %d serving at %s:%d, cache=%s, cap=%zu\n",
                instance, host.c_str(), port, algo.c_str(), capacity);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, nullptr);

    int sig = 0;
    sigwait(&mask, &sig);

    std::printf("stopping\n");
    server.stop();
    delete storage;
    return 0;
}
