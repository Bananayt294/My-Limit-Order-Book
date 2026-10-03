#include "../Generate_Orders/GenerateOrders.hpp"
#include "../Process_Orders/OrderPipeline.hpp"
#include "../LOB/Book.hpp"
#include "../LOB/Limit.hpp"
#include "../LOB/Order.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>

int main() {
    Book* book = new Book();

    OrderPipeline orderPipeline(book);

    const long long expectedOrders = 400391; // Change this for your dataset

    std::cout << "========================================\n";
    std::cout << "        LIMIT ORDER BOOK BENCHMARK\n";
    std::cout << "========================================\n";

    std::cout << "Input file: AAPL_replay.txt\n";
    std::cout << "Orders:     " << expectedOrders << "\n";
    std::cout << "----------------------------------------\n";

    // Start measuring
    auto start = std::chrono::high_resolution_clock::now();

    orderPipeline.processOrdersFromFile(
        "c:\\LOB\\My-Limit-Order-Book\\data\\AAPL_replay.txt"
    );

    // Stop measuring
    auto stop = std::chrono::high_resolution_clock::now();

    // Measure in nanoseconds for maximum precision
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        stop - start
    );

    const double seconds = duration.count() / 1'000'000'000.0;

    const double ordersPerSecond =
        expectedOrders / seconds;

    const double nanosecondsPerOrder =
        static_cast<double>(duration.count()) / expectedOrders;

    const double microsecondsPerOrder =
        nanosecondsPerOrder / 1'000.0;

    const double millisecondsPerOrder =
        nanosecondsPerOrder / 1'000'000.0;

    const double timePerMillionOrders =
        seconds * (1'000'000.0 / expectedOrders);

    std::cout << std::fixed << std::setprecision(3);

    std::cout << "Processing time:       "
              << seconds << " seconds\n";

    std::cout << "Processing time:       "
              << duration.count() / 1'000'000.0
              << " milliseconds\n";

    std::cout << "Throughput:             "
              << ordersPerSecond
              << " orders/sec\n";

    std::cout << "Average time/order:     "
              << nanosecondsPerOrder
              << " ns/order\n";

    std::cout << "Average time/order:     "
              << microsecondsPerOrder
              << " us/order\n";

    std::cout << "Average time/order:     "
              << millisecondsPerOrder
              << " ms/order\n";

    std::cout << "Time per 1M orders:     "
              << timePerMillionOrders
              << " seconds\n";

    std::cout << "========================================\n";

    delete book;

    return 0;
}