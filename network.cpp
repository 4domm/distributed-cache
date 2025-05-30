#include "network.h"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <functional>
#include <Poco/Net/HTTPRequestHandler.h>

bool redirectIfNeeded(const std::vector<std::string> &shards,
                      int curr,
                      const std::string &key,
                      Poco::Net::HTTPServerRequest &request,
                      Poco::Net::HTTPServerResponse &response) {
    if (shards.size() == 1 || key.empty()) return false;
    size_t idx = std::hash<std::string>{}(key) % shards.size();
    if (static_cast<int>(idx) != curr) {
        std::string target = "http://" + shards[idx] + request.getURI();
        response.setStatus(Poco::Net::HTTPResponse::HTTP_TEMPORARY_REDIRECT);
        response.set("Location", target);
        response.send();
        return true;
    }
    return false;
}

void GetHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) {
    Poco::JSON::Object::Ptr jsonResp = new Poco::JSON::Object;
    response.setContentType("application/json");
    try {
        Poco::JSON::Parser parser;
        auto reqObj = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
        std::string key = reqObj->getValue<std::string>("key");
        if (redirectIfNeeded(shards, curr, key, request, response)) {
            return;
        }
        auto res = storage->get(key);
        if (res) {
            jsonResp->set("status", "ok");
            jsonResp->set("value", res->get());
        } else {
            jsonResp->set("status", "not found");
        }
    } catch (const std::runtime_error &) {
        return;
    } catch (...) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
    std::ostream &out = response.send();
    Poco::JSON::Stringifier::stringify(jsonResp, out);
}

void PutHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) {
    Poco::JSON::Object::Ptr jsonResp = new Poco::JSON::Object;
    response.setContentType("application/json");
    try {
        Poco::JSON::Parser parser;
        auto reqObj = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
        auto key = reqObj->getValue<std::string>("key");
        auto value = reqObj->getValue<std::string>("value");
        if (redirectIfNeeded(shards, curr, key, request, response)) {
            return;
        }
        storage->put(key, value);
        jsonResp->set("status", "ok");
    } catch (const std::runtime_error &) {
        return;
    } catch (...) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
    std::ostream &out = response.send();
    Poco::JSON::Stringifier::stringify(jsonResp, out);
}

void DeleteHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                  Poco::Net::HTTPServerResponse &response) {
    Poco::JSON::Object::Ptr jsonResp = new Poco::JSON::Object;
    response.setContentType("application/json");
    try {
        Poco::JSON::Parser parser;
        auto reqObj = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();
        std::string key = reqObj->getValue<std::string>("key");
        if (redirectIfNeeded(shards, curr, key, request, response)) {
            return;
        }
        storage->remove(key);
        jsonResp->set("status", "ok");
    } catch (const std::runtime_error &) {
        return;
    } catch (...) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
    std::ostream &out = response.send();
    Poco::JSON::Stringifier::stringify(jsonResp, out);
}

void StatHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
                                Poco::Net::HTTPServerResponse &response) {
    Poco::JSON::Object::Ptr jsonResp = new Poco::JSON::Object;
    response.setContentType("application/json");
    try {
        auto stats = storage->getLoad();
        jsonResp->set("mem", stats.first);
        jsonResp->set("size", stats.second);
    } catch (...) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    }
    std::ostream &out = response.send();
    Poco::JSON::Stringifier::stringify(jsonResp, out);
}

Poco::Net::HTTPRequestHandler *HandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest &request) {
    if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_POST) return nullptr;
    std::string uri = request.getURI();
    if (uri == "/get") return new GetHandler(storage, shards, curr);
    if (uri == "/put") return new PutHandler(storage, shards, curr);
    if (uri == "/delete") return new DeleteHandler(storage, shards, curr);
    if (uri == "/stat") return new StatHandler(storage);
    return nullptr;
}
