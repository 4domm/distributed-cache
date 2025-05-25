#pragma once
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

class KVstorage {
public:
    void put(const std::string &key, const std::string &value);

    size_t remove(const std::string &key);

    std::optional<std::reference_wrapper<std::string> > get(const std::string &key);

    static std::pair<int, int> getLoad();

private:
    std::unordered_map<std::string, std::string> cache;
    std::mutex mutex;
};
