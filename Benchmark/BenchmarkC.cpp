#include "../LOB/Book.hpp"

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

enum class OperationType {
    Add,
    Cancel,
    Modify
};

struct BenchmarkOperation {

    OperationType type;

    int orderId;
    bool buyOrSell;
    int shares;
    int limitPrice;
};

int main() {

    constexpr int NUM_OPERATIONS = 1'000'000;

    // =========================================================
    // 1. GENERATE WORKLOAD
    // =========================================================

    std::mt19937 rng(42);

    std::uniform_int_distribution<int> priceDist(90, 110);
    std::uniform_int_distribution<int> sharesDist(1, 100);
    std::bernoulli_distribution sideDist(0.5);

    std::vector<BenchmarkOperation> operations;
    operations.reserve(NUM_OPERATIONS);

    // Orders currently alive in our simulated book.
    std::vector<int> activeOrders;
    activeOrders.reserve(NUM_OPERATIONS);

    int nextOrderId = 1;

    // =========================================================
    // 2. BUILD 1M-OPERATION WORKLOAD
    // =========================================================

    for (int i = 0; i < NUM_OPERATIONS; ++i) {

        // -----------------------------------------------------
        // First 10,000 operations are ADDs.
        // This gives the book initial liquidity.
        // -----------------------------------------------------

        if (i < 10'000) {

            BenchmarkOperation operation;

            operation.type = OperationType::Add;
            operation.orderId = nextOrderId++;
            operation.buyOrSell = sideDist(rng);
            operation.shares = sharesDist(rng);
            operation.limitPrice = priceDist(rng);

            operations.push_back(operation);

            activeOrders.push_back(operation.orderId);

            continue;
        }

        // -----------------------------------------------------
        // Select operation:
        //
        // 50% Add
        // 25% Cancel
        // 25% Modify
        // -----------------------------------------------------

        std::uniform_int_distribution<int> operationDist(1, 100);

        int choice = operationDist(rng);

        // =====================================================
        // ADD
        // =====================================================

        if (choice <= 50 || activeOrders.empty()) {

            BenchmarkOperation operation;

            operation.type = OperationType::Add;
            operation.orderId = nextOrderId++;
            operation.buyOrSell = sideDist(rng);
            operation.shares = sharesDist(rng);
            operation.limitPrice = priceDist(rng);

            operations.push_back(operation);

            activeOrders.push_back(operation.orderId);
        }

        // =====================================================
        // CANCEL
        // =====================================================

        else if (choice <= 75) {

            std::uniform_int_distribution<size_t> orderDist(
                0,
                activeOrders.size() - 1
            );

            size_t index = orderDist(rng);

            BenchmarkOperation operation;

            operation.type = OperationType::Cancel;
            operation.orderId = activeOrders[index];
            operation.buyOrSell = false;
            operation.shares = 0;
            operation.limitPrice = 0;

            operations.push_back(operation);

            // Remove from active order list.
            activeOrders[index] = activeOrders.back();
            activeOrders.pop_back();
        }

        // =====================================================
        // MODIFY
        // =====================================================

        else {

            std::uniform_int_distribution<size_t> orderDist(
                0,
                activeOrders.size() - 1
            );

            size_t index = orderDist(rng);

            BenchmarkOperation operation;

            operation.type = OperationType::Modify;
            operation.orderId = activeOrders[index];
            operation.buyOrSell = false;
            operation.shares = sharesDist(rng);
            operation.limitPrice = priceDist(rng);

            operations.push_back(operation);
        }
    }

    // =========================================================
    // 3. PRINT WORKLOAD SIZE
    // =========================================================

    std::cout << "Generated "
              << operations.size()
              << " operations.\n";

    // =========================================================
    // 4. CREATE FRESH ORDER BOOK
    // =========================================================

    book* bookPtr = new book();

    // =========================================================
    // 5. START TIMER
    // =========================================================

    auto start = std::chrono::steady_clock::now();

    // =========================================================
    // 6. PROCESS WORKLOAD
    // =========================================================

    for (const auto& operation : operations) {

        switch (operation.type) {

            case OperationType::Add:

                bookPtr->AddLimitOrder(
                    operation.orderId,
                    operation.buyOrSell,
                    operation.shares,
                    operation.limitPrice
                );

                break;

            case OperationType::Cancel:

                bookPtr->CancelLimitOrder(
                    operation.orderId
                );

                break;

            case OperationType::Modify:

                bookPtr->ModifyLimitOrder(
                    operation.orderId,
                    operation.shares,
                    operation.limitPrice
                );

                break;
        }
    }

    // =========================================================
    // 7. STOP TIMER
    // =========================================================

    auto stop = std::chrono::steady_clock::now();

    // =========================================================
    // 8. CALCULATE RESULTS
    // =========================================================

    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            stop - start
        );

    double seconds =
        duration.count() / 1'000'000'000.0;

    double operationsPerSecond =
        static_cast<double>(NUM_OPERATIONS) / seconds;

    double nanosecondsPerOperation =
        static_cast<double>(duration.count())
        / NUM_OPERATIONS;

    // =========================================================
    // 9. PRINT RESULTS
    // =========================================================

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "           BENCHMARK C\n";
    std::cout << "========================================\n";

    std::cout << "Operations processed: "
              << NUM_OPERATIONS
              << "\n";

    std::cout << "Time: "
              << seconds
              << " seconds\n";

    std::cout << "Throughput: "
              << operationsPerSecond
              << " operations/sec\n";

    std::cout << "Average: "
              << nanosecondsPerOperation
              << " ns/operation\n";

    std::cout << "========================================\n";

    delete bookPtr;

    return 0;
}

