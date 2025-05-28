#include "storage.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <iostream>

#include "lru_cache.h"
#include "network.h"

void setup() {
    const Poco::Net::ServerSocket serverSocket(8080);
    auto *storage = new KVstorage(new LRUCache<std::string, std::string>);
    auto *params = new Poco::Net::HTTPServerParams();
    params->setMaxThreads(24);
    params->setKeepAlive(true);
    Poco::Net::HTTPServer server(new HandlerFactory(storage), serverSocket, params);
    server.start();
    storage->startEviction();
    std::cout << "started" << std::endl;
    std::cin.get();
    server.stop();
    delete storage;
}

int main() {
    setup();
    return 0;
}
