#ifndef LIMIT_POOL_HPP
#define LIMIT_POOL_HPP

#include "limit.hpp"
#include <vector>

class limit_pool {
private:
    constexpr static int MAX_LIMITS = 2000000;
    alignas(limit) char memory[MAX_LIMITS * sizeof(limit)];
    std::vector<size_t> free_indices;
    std::vector<unsigned char> slot_in_use;

public:
    limit_pool();
    limit* allocate(int limit_price, int size = 0, bool buyorsell = false, int totalshares = 0);
    void release(limit* limit_ptr);
    size_t available() const;
};

#endif
