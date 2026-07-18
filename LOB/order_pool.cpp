#include "order_pool.hpp"
#include <cstdint>
#include <new>

order_pool::order_pool() : slot_in_use(MAX_ORDERS, 0) {
    free_indices.reserve(MAX_ORDERS);
    for (size_t i = 0; i < MAX_ORDERS; i++) {
        free_indices.push_back(i);
    }
}

order* order_pool::allocate(int idnumber, bool buyorsell, int shares, int limit) {
    if (free_indices.empty()) {
        return nullptr; // No more orders available
    }

    size_t index = free_indices.back();
    free_indices.pop_back();
    slot_in_use[index] = 1;
    return new (memory + index * sizeof(order)) order(idnumber, buyorsell, shares, limit);
}

void order_pool::release(order* order_ptr) {
    if (order_ptr == nullptr) {
        return;
    }

    const auto* byte_ptr = reinterpret_cast<const char*>(order_ptr);
    const auto* memory_begin = reinterpret_cast<const char*>(memory);
    const auto* memory_end = memory_begin + sizeof(memory);

    if (byte_ptr < memory_begin || byte_ptr >= memory_end) {
        return;
    }

    const auto offset = static_cast<size_t>(byte_ptr - memory_begin);
    if (offset % sizeof(order) != 0) {
        return;
    }

    const size_t index = offset / sizeof(order);
    if (index >= MAX_ORDERS || slot_in_use[index] == 0) {
        return;
    }

    order_ptr->~order();
    slot_in_use[index] = 0;
    free_indices.push_back(index);
}

size_t order_pool::available() const {
    return free_indices.size();
};