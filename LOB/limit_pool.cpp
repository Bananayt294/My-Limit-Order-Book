#include "limit_pool.hpp"
#include <cstdint>
#include <new>

limit_pool::limit_pool() : slot_in_use(MAX_LIMITS, 0) {
    free_indices.reserve(MAX_LIMITS);
    for (size_t i = 0; i < MAX_LIMITS; i++) {
        free_indices.push_back(i);
    }
}

limit* limit_pool::allocate(int limit_price, int size, bool buyorsell, int totalshares) {
    if (free_indices.empty()) {
        return nullptr;
    }

    size_t index = free_indices.back();
    free_indices.pop_back();
    slot_in_use[index] = 1;
    return new (memory + index * sizeof(limit)) limit(limit_price, size, buyorsell, totalshares);
}

void limit_pool::release(limit* limit_ptr) {
    if (limit_ptr == nullptr) {
        return;
    }

    const auto* byte_ptr = reinterpret_cast<const char*>(limit_ptr);
    const auto* memory_begin = reinterpret_cast<const char*>(memory);
    const auto* memory_end = memory_begin + sizeof(memory);

    if (byte_ptr < memory_begin || byte_ptr >= memory_end) {
        return;
    }

    const auto offset = static_cast<size_t>(byte_ptr - memory_begin);
    if (offset % sizeof(limit) != 0) {
        return;
    }

    const size_t index = offset / sizeof(limit);
    if (index >= MAX_LIMITS || slot_in_use[index] == 0) {
        return;
    }

    limit_ptr->~limit();
    slot_in_use[index] = 0;
    free_indices.push_back(index);
}

size_t limit_pool::available() const {
    return free_indices.size();
}
