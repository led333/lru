#include <dns_cache.h>
#include <cstddef>
#include <mutex>
#include <string>

namespace dns_cache {
namespace {
size_t defaultMaxSize{1024};
}  // namespace

bool TrySetMaxSize(size_t maxSize) {
    if (maxSize > 128 && maxSize < 1 << 20) {
        defaultMaxSize = maxSize;
        return true;
    }
    return false;
}
DNSCache& Cache() {
    static DNSCache cache{defaultMaxSize};
    return cache;
}

DNSCache::DNSCache(size_t max_size) : maxSize(max_size) {
}

void DNSCache::update(const std::string& name, const std::string& ip) {
    ++countUpdater;
    std::lock_guard lk{mut};

    if (auto it = umap.find(name); it != std::end(umap)) {
        auto itList = it->second;
        itList->ip = ip;
        listDataBlock.splice(std::begin(listDataBlock), listDataBlock, itList);
    } else {
        if (umap.size() == maxSize) {
            if (auto it = umap.find(*listDataBlock.back().pname); it != std::end(umap)) {
                listDataBlock.pop_back();
                umap.erase(it);
            } else {
                //
            }
        }

        auto [insertIt, insertOk] = umap.emplace(name, std::end(listDataBlock));
        if (!insertOk) {
            //
        }
        listDataBlock.emplace_front(ip, &insertIt->first);
        insertIt->second = std::begin(listDataBlock);
    }

    --countUpdater;
    cv.notify_one();
}

std::string DNSCache::resolve(const std::string& name) {
    std::unique_lock lk{mut};
    cv.wait(lk, [this] { return countUpdater == 0; });

    std::string result;
    if (auto it = umap.find(name); it != std::end(umap)) {
        auto itList = it->second;
        listDataBlock.splice(std::begin(listDataBlock), listDataBlock, it->second);
        result = itList->ip;
    }

    cv.notify_one();
    return result;
}
}  // namespace dns_cache
