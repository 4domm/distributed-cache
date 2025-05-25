
#include "storage.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <iostream>

#include "network.h"

void setup() {
    Poco::Net::ServerSocket serverSocket(8080);
    auto *storage = new KVstorage();

    Poco::Net::HTTPServer server(new HandlerFactory(storage), serverSocket, new Poco::Net::HTTPServerParams);
    server.start();

    std::cout << "started" << std::endl;
    std::cin.get();
    server.stop();
    delete storage;
}

int main() {
    setup();
    return 0;
}
