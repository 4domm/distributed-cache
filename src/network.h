#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include "storage.h"
#include <Poco/Net/HTTPRequestHandlerFactory.h>

class GetHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit GetHandler(KVstorage *storage);

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage *storage{};
};


class PutHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit PutHandler(KVstorage *storage);

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage *storage;
};


class DeleteHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit DeleteHandler(KVstorage *storage);

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage *storage;
};


class StatHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit StatHandler(KVstorage *storage);

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;

private:
    KVstorage *storage;
};

class HandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    explicit HandlerFactory(KVstorage *storage);

    Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

private:
    KVstorage *storage;
};
