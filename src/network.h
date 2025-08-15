#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include "storage.h"
#include <vector>
#include <string>

class GetHandler : public Poco::Net::HTTPRequestHandler {
public:
    GetHandler(KVstorage<std::string, std::string> *storage,
               const std::vector<std::string> &shards,
               int curr)
        : storage(storage), shards(shards), curr(curr) {
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage<std::string, std::string> *storage;
    std::vector<std::string> shards;
    int curr;
};

class PutHandler : public Poco::Net::HTTPRequestHandler {
public:
    PutHandler(KVstorage<std::string, std::string> *storage,
               const std::vector<std::string> &shards,
               int curr)
        : storage(storage), shards(shards), curr(curr) {
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage<std::string, std::string> *storage;
    std::vector<std::string> shards;
    int curr;
};

class DeleteHandler : public Poco::Net::HTTPRequestHandler {
public:
    DeleteHandler(KVstorage<std::string, std::string> *storage,
                  const std::vector<std::string> &shards,
                  int curr)
        : storage(storage), shards(shards), curr(curr) {
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request,
                       Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage<std::string, std::string> *storage;
    std::vector<std::string> shards;
    int curr;
};


class HandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    HandlerFactory(KVstorage<std::string, std::string> *storage,
                   const std::vector<std::string> &shards,
                   int curr)
        : storage(storage), shards(shards), curr(curr) {
    }

    Poco::Net::HTTPRequestHandler *createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) override;

private:
    KVstorage<std::string, std::string> *storage;
    std::vector<std::string> shards;
    int curr;
};
