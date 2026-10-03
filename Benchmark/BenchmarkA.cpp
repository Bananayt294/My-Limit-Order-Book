#include "../Generate_Orders/GenerateOrders.hpp"
#include "../Process_Orders/OrderPipeline.hpp"
#include "../LOB/Book.hpp"
#include "../LOB/Limit.hpp"
#include "../LOB/Order.hpp"

#include <iostream>
#include <chrono>

int main() {

    book* bookPtr = new book();

    OrderPipeline orderPipeline(bookPtr);

    GenerateOrders generateOrders(bookPtr);

    // Generate the workload BEFORE the timer.
    generateOrders.createInitialOrders(1'000'000, 300);

    // ---------------------------------------------------------
    // START BENCHMARK
    // ---------------------------------------------------------

    auto start = std::chrono::steady_clock::now();

    orderPipeline.processOrdersFromFile(
        "C:\\LOB\\My-Limit-Order-Book\\Generate_Orders\\initialOrders.txt"
    );

    auto stop = std::chrono::steady_clock::now();

    // ---------------------------------------------------------
    // RESULTS
    // ---------------------------------------------------------

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            stop - start
        );

    double seconds = duration.count() / 1000.0;

    double ordersPerSecond =
        1'000'000.0 / seconds;

    double nanosecondsPerOrder =
        (seconds * 1'000'000'000.0) / 1'000'000.0;

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "           BENCHMARK A\n";
    std::cout << "========================================\n";

    std::cout << "Orders processed: "
              << 1'000'000
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
