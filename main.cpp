#include "dns_cache.h"

#include <iostream>
#include <iostream>

int main() {
    std::cout << "start" << '\n';

    SET_SIZE_DNS_CACHE(2048);
    DNS_CACHE.update("name1", "ip1");
    DNS_CACHE.update("name2", "ip2");

    std::cout << "name1: " << DNS_CACHE.resolve("name1") << '\n';
    return 0;
}