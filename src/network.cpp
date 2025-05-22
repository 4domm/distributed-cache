#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include "storage.h"


class GetHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit GetHandler(KVstorage *storage) : storage(storage) {
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
        response.setContentType("application/json");
        try {
            Poco::JSON::Parser parser;
            Poco::JSON::Object::Ptr reqData = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
            auto key = reqData->getValue<std::string>("key");
            auto res = storage->get(key);
            if (res.has_value()) {
                json->set("status", "ok");
                json->set("key", res->get());
            } else {
                json->set("status", "not found");
            }
        } catch (...) {
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
        }
        std::ostream &out = response.send();
        Poco::JSON::Stringifier::stringify(json, out);
    }

private:
    KVstorage *storage;
};


class PutHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit PutHandler(KVstorage *storage) : storage(storage) {
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
        response.setContentType("application/json");
        try {
            Poco::JSON::Parser parser;
            Poco::JSON::Object::Ptr reqData = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
            storage->put(reqData->getValue<std::string>("key"), reqData->getValue<std::string>("value"));
        } catch (...) {
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
        }
        std::ostream &out = response.send();
        Poco::JSON::Stringifier::stringify(json, out);
    }

private:
    KVstorage *storage;
};


class DeleteHandler : public Poco::Net::HTTPRequestHandler {
public:
    explicit DeleteHandler(KVstorage *storage) : storage(storage) {
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
        response.setContentType("application/json");
        try {
            Poco::JSON::Parser parser;
            Poco::JSON::Object::Ptr reqData = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
            storage->put(reqData->getValue<std::string>("key"), reqData->getValue<std::string>("value"));
        } catch (...) {
            response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
        }
        std::ostream &out = response.send();
        Poco::JSON::Stringifier::stringify(json, out);
    }

private:
    KVstorage *storage;
};
