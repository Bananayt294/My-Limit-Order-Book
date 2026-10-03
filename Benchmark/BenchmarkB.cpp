#include "../LOB/Book.hpp"

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

struct TestOrder {
    int orderId;
    bool buyOrSell;
    int shares;
    int limitPrice;
};

int main() {

    constexpr int NUM_ORDERS = 1'000'000;

    // =========================================================
    // 1. GENERATE WORKLOAD
    // =========================================================

    std::mt19937 rng(42);

    std::uniform_int_distribution<int> priceDist(90, 110);
    std::uniform_int_distribution<int> sharesDist(1, 100);
    std::bernoulli_distribution sideDist(0.5);

    std::vector<TestOrder> orders;
    orders.reserve(NUM_ORDERS);

    for (int i = 0; i < NUM_ORDERS; ++i) {

        TestOrder order;

        order.orderId = i + 1;
        order.buyOrSell = sideDist(rng);
        order.shares = sharesDist(rng);
        order.limitPrice = priceDist(rng);

        orders.push_back(order);
    }

    // =========================================================
    // 2. CREATE FRESH BOOK
    // =========================================================

    book* bookPtr = new book();

    // =========================================================
    // 3. START TIMER
    // =========================================================

    auto start = std::chrono::steady_clock::now();

    // =========================================================
    // 4. PROCESS ALL ORDERS
    // =========================================================

    for (const auto& order : orders) {

        bookPtr->AddLimitOrder(
            order.orderId,
            order.buyOrSell,
            order.shares,
            order.limitPrice
        );
    }

    // =========================================================
    // 5. STOP TIMER
    // =========================================================

    auto stop = std::chrono::steady_clock::now();

    // =========================================================
    // 6. CALCULATE RESULTS
    // =========================================================

    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            stop - start
        );

    double seconds =
        duration.count() / 1'000'000'000.0;

    double ordersPerSecond =
        NUM_ORDERS / seconds;

    double nanosecondsPerOrder =
        static_cast<double>(duration.count())
        / NUM_ORDERS;

    // =========================================================
    // 7. PRINT RESULTS
    // =========================================================

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "           BENCHMARK B\n";
    std::cout << "========================================\n";

    std::cout << "Orders processed: "
              << NUM_ORDERS
              << "\n";

    std::cout << "Time: "
              << seconds
              << " seconds\n";

    std::cout << "Throughput: "
              << ordersPerSecond
              << " orders/sec\n";

    std::cout << "Average: "
              << nanosecondsPerOrder
              << " ns/order\n";

    std::cout << "========================================\n";

    delete bookPtr;

    return 0;
}

