#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include "storage.h"
#include "network.h"


GetHandler::GetHandler(KVstorage<std::string, std::string> *storage) : storage(storage) {
}

void GetHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
    Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
    response.setContentType("application/json");
    try {
        Poco::JSON::Parser parser;
        auto reqData = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
        const auto key = reqData->getValue<std::string>("key");
        const auto res = storage->get(key);
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


PutHandler::PutHandler(KVstorage<std::string, std::string> *storage) : storage(storage) {
}

void PutHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) {
    const Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
    response.setContentType("application/json");
    try {
        Poco::JSON::Parser parser;
        auto reqData = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
        storage->put(reqData->getValue<std::string>("key"), reqData->getValue<std::string>("value"));
    } catch (...) {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
    }
    std::ostream &out = response.send();
    Poco::JSON::Stringifier::stringify(json, out);
}


DeleteHandler::DeleteHandler(KVstorage<std::string, std::string> *storage) : storage(storage) {
}

void DeleteHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                  Poco::Net::HTTPServerResponse &response) {
    const Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
    response.setContentType("application/json");
    try {
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr reqData = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
        storage->remove(reqData->getValue<std::string>("key"));
    } catch (...) {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
    }
    std::ostream &out = response.send();
    Poco::JSON::Stringifier::stringify(json, out);
}


StatHandler::StatHandler(KVstorage<std::string, std::string> *storage) : storage(storage) {
}

void StatHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                Poco::Net::HTTPServerResponse &response) {
    Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
    response.setContentType("application/json");

    try {
        json->set("mem", KVstorage<std::basic_string<char>, std::basic_string<char>>::getLoad());
    } catch (...) {
        response.setStatus(Poco::Net::HTTPServerResponse::HTTP_BAD_REQUEST);
    }
    std::ostream &out = response.send();
    Poco::JSON::Stringifier::stringify(json, out);
}


HandlerFactory::HandlerFactory(KVstorage<std::string, std::string> *storage) : storage(storage) {
}

Poco::Net::HTTPRequestHandler *
HandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &request) {
    if (request.getMethod() != "POST") return nullptr;
    if (request.getURI() == "/put") return new PutHandler(storage);
    if (request.getURI() == "/delete") return new DeleteHandler(storage);
    if (request.getURI() == "/get") return new GetHandler(storage);
    if (request.getURI() == "/stat") return new StatHandler(storage);

    return nullptr;
}
