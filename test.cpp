#include <gtest/gtest.h>

#include <dns_cache.h>
#include <atomic>
#include <cstddef>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename... T>
std::string MakeString(T&&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return ss.str();
}

using namespace dns_cache;

class FixtureDNSCache : public DNSCache {
public:
    FixtureDNSCache(size_t max_size) : DNSCache(max_size) {
    }

    size_t MapSize() const noexcept {
        return umap.size();
    }

    size_t ListSize() const noexcept {
        return listDataBlock.size();
    }
};

TEST(simple, test1) {
    constexpr size_t maxSize{10};
    FixtureDNSCache cache(maxSize);
    ASSERT_EQ(cache.resolve(""), "");
    ASSERT_EQ(cache.resolve("aaa"), "");

    cache.update("aaa", "bbb");
    ASSERT_EQ(cache.resolve("aaa"), "bbb");
}

TEST(simple, test2) {
    constexpr size_t maxSize{10000};
    FixtureDNSCache cache(maxSize);
    ASSERT_EQ("123_abc", MakeString(1, 2, 3, "_abc"));

    for (auto i = 0; i < maxSize; ++i) {
        cache.update(MakeString("key", i), MakeString("value", i));
        ASSERT_EQ(cache.resolve(MakeString("key", i)), MakeString("value", i));
    }
    ASSERT_EQ(cache.ListSize(), maxSize);
    ASSERT_EQ(cache.ListSize(), cache.MapSize());
    for (auto i = 0; i < maxSize; ++i) {
        ASSERT_EQ(cache.resolve(MakeString("key", i)), MakeString("value", i));
    }
    ASSERT_EQ(cache.ListSize(), cache.MapSize());

    cache.update(MakeString("key", maxSize), MakeString("value", maxSize));
    ASSERT_EQ(cache.ListSize(), maxSize);
    ASSERT_EQ(cache.ListSize(), cache.MapSize());
    ASSERT_EQ(cache.resolve(MakeString("key", maxSize)), MakeString("value", maxSize));
    ASSERT_EQ(cache.resolve(MakeString("key", 0)), "");
}

TEST(simple, test3) {
    constexpr size_t maxSize{10000};
    FixtureDNSCache cache(maxSize);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, std::numeric_limits<int>::max());

    std::unordered_map<std::string, std::string> data;
    for (auto i = 0; i < maxSize; ++i) {
        auto name = MakeString("key", distrib(gen));
        auto addr = MakeString("value", distrib(gen));
        data[name] = addr;
        cache.update(name, addr);
    }

    for (const auto& [name, addr] : data) {
        ASSERT_EQ(cache.resolve(name), addr);
    }

    data.clear();
    for (auto i = 0; i < maxSize; ++i) {
        auto name = MakeString("key", distrib(gen));
        auto addr = MakeString("value", distrib(gen));
        data[name] = addr;
        cache.update(name, addr);
    }

    for (const auto& [name, addr] : data) {
        ASSERT_EQ(cache.resolve(name), addr);
    }
}

TEST(simple, test4) {
    using namespace std::chrono_literals;
    constexpr size_t maxSize{512};
    FixtureDNSCache cache(maxSize);

    std::vector<std::pair<std::string, std::string>> data;
    for (auto i = 0; i < maxSize; ++i) {
        data.push_back(std::make_pair(MakeString("key", i), MakeString("value", i)));
        cache.update(MakeString("key", i), MakeString("value", i));
    }

    std::atomic<bool> stopFlag{false};
    std::vector<std::jthread> updateThreads;
    std::vector<std::jthread> resolveThreads;
    std::atomic<size_t> countUpdate{0};
    std::atomic<size_t> countResolve{0};

    for (int i = 0; i < 16; ++i) {

        if (i < 2) {
            updateThreads.emplace_back([&cache, data, &stopFlag, &countUpdate] {
                while (!stopFlag) {
                    for (const auto& [name, addr] : data) {
                        cache.update(name, addr);
                        ++countUpdate;
                        std::this_thread::sleep_for(1000us);
                    }
                }
            });
        }

        resolveThreads.emplace_back([&cache, data, &stopFlag, &countResolve] {
            while (!stopFlag) {
                for (const auto& [name, addr] : data) {
                    ++countResolve;
                    ASSERT_EQ(addr, cache.resolve(name));
                }
            }
        });
    }

    std::this_thread::sleep_for(1000ms);
    stopFlag = true;
    updateThreads.clear();
    resolveThreads.clear();
    std::cout << "[ INFO     ] "
              << "countUpdate: " << countUpdate << " countResolve: " << countResolve << '\n';
}