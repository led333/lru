#pragma once

#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace dns_cache {
class DNSCache {
    friend DNSCache& Cache();

public:
    void update(const std::string& name, const std::string& ip);
    std::string resolve(const std::string& name);

protected:
    explicit DNSCache(size_t max_size);

    struct DataBlock {
        DataBlock(std::string ip, const std::string* pname) : ip(std::move(ip)), pname(pname) {
        }
        std::string ip;
        const std::string* pname;
    };

    size_t maxSize;
    std::unordered_map<std::string, std::list<DataBlock>::iterator> umap;
    std::list<DataBlock> listDataBlock;

    std::mutex mut;
    std::condition_variable cv;
    std::atomic<size_t> countUpdater{};
};

bool TrySetMaxSize(size_t maxSize);
DNSCache& Cache();

}  // namespace dns_cache

#define SET_SIZE_DNS_CACHE(x) dns_cache::TrySetMaxSize(x)
#define DNS_CACHE dns_cache::Cache()
