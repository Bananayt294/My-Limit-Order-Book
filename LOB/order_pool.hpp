#ifndef ORDER_POOL_HPP
#define ORDER_POOL_HPP

#include "order.hpp"
#include <vector>
class order_pool {
    private:
    constexpr static int MAX_ORDERS = 1000000;
    alignas(order) char memory[MAX_ORDERS * sizeof(order)];
    std::vector<size_t> free_indices;
    std::vector<unsigned char> slot_in_use;

    public:
    order_pool();
    order* allocate(int idnumber, bool buyorsell, int shares, int limit);
    void release (order* order_ptr);
    size_t available() const;
};

#endif
