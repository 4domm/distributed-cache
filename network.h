#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include "storage.h"
#include <Poco/Net/HTTPRequestHandlerFactory.h>

#include "cache.h"

class GetHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit GetHandler(KVstorage<std::string,std::string> *storage);

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage<std::string,std::string> *storage;
};


class PutHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit PutHandler(KVstorage<std::string,std::string> *storage);

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage<std::string,std::string> *storage;
};


class DeleteHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit DeleteHandler(KVstorage<std::string,std::string> *storage);

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage<std::string,std::string> *storage;
};


class StatHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit StatHandler(KVstorage<std::string,std::string> *storage);

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage<std::string,std::string> *storage;
};

class HandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    explicit HandlerFactory(KVstorage<std::string,std::string> *storage);

    Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

private:
    KVstorage<std::string,std::string> *storage;
};
