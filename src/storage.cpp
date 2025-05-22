#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

class KVstorage {
public:
    void put(const std::string &key, const std::string &value) {
        std::lock_guard lock(mutex);
        cache[key] = value;
    }

    size_t remove(const std::string &key) {
        std::lock_guard lock(mutex);
        return cache.erase(key);
    }


    std::optional<std::reference_wrapper<std::string> > get(const std::string &key) {
        std::lock_guard lock(mutex);
        if (cache.count(key) > 0) {
            return cache.at(key);
        }
        return std::nullopt;
    }

private:
    std::unordered_map<std::string, std::string> cache;
    std::mutex mutex;
};
