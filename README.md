# My-Limit-Order-Book

# Architecture

**HUGE CREDIT TO [BRPROJECTS](https://github.com/brprojects/Limit-Order-Book) FOR INSPIRATION**

<br>

<p align="center">
  <img src="https://raw.githubusercontent.com/brprojects/Limit-Order-Book/main/figures/architecture.png" alt="Limit Order Book Architecture (credit to brprojects)" width="900"/>
</p>

# Limit Order Book

A high-performance C++20 Limit Order Book designed for experimenting with exchange-style order processing, market microstructure, and low-latency trading systems.

The project implements price-time priority matching, AVL-based price levels, custom order allocation, order tracking, cancellations, modifications, market orders, limit orders, and stop orders.

This project was built to explore the data structures and algorithms used in modern electronic exchanges and high-frequency trading systems while emphasizing performance, memory efficiency, and clean object-oriented design.

---

## Features

* Price-Time Priority (FIFO)
* AVL Tree for price levels
* O(log N) insertion and deletion of price levels
* O(1) access to best bid and ask
* Doubly linked list of orders at each price level
* Custom memory pool allocator for orders
* Market Orders
* Limit Orders
* Stop Orders
* Order Modification
* Order Cancellation
* Automatic removal of empty price levels
* Order lookup using hash tables

---

## Data Structures

### Price Levels

Price levels are stored inside an AVL tree.

Each node contains:

* Price
* Total volume
* FIFO queue of orders
* Cached height
* Parent pointer
* Left child
* Right child

The AVL tree maintains balance after structural modifications, providing:

* O(log N) insertion
* O(log N) deletion
* O(log N) lookup

The tree also stores the height of each node directly, allowing balance calculations without recursively traversing subtrees to determine their heights.

---

### Orders

Orders are stored inside a doubly linked list at each price level.

```text
Head <-> Order <-> Order <-> Tail
```

This allows:

* O(1) insertion into an existing price-level queue
* O(1) cancellation using stored order references
* O(1) removal from the linked list
* FIFO execution ordering

When an order is cancelled or modified, the order can be removed directly from its linked-list position without searching through the queue.

---

### Memory Pool

Instead of allocating every order individually with `new`, the project uses a custom memory pool.

Benefits include:

* Reduced heap allocations
* Better cache locality
* Lower allocation overhead
* More deterministic performance
* Order object reuse

Orders are recycled through the pool rather than repeatedly allocating and freeing memory from the general-purpose heap.

---

## Complexity

| Operation       | Complexity                                |
| --------------- | ----------------------------------------- |
| Add Limit Order | O(log N)                                  |
| Cancel Order    | O(1) + O(log N) if price level is removed |
| Modify Order    | O(1) / O(log N) depending on modification |
| Market Order    | O(log N)                                  |
| Find Order      | O(1)                                      |
| Best Bid        | O(1)                                      |
| Best Ask        | O(1)                                      |

The asymptotic complexity remains unchanged by the AVL height-cache optimization described below. The optimization reduces the constant amount of work performed by AVL operations.

---

## Project Structure

```text
LOB/

├── book.cpp
├── book.hpp
├── limit.cpp
├── limit.hpp
├── order.cpp
├── order.hpp
├── order_pool.cpp
├── order_pool.hpp

Process_Orders/

├── OrderPipeline.cpp
└── OrderPipeline.hpp

Generate_Orders/

└── GenerateOrders.cpp
```

---

# Performance

The Limit Order Book was benchmarked before and after a major internal optimization: **caching AVL tree node heights**.

Both benchmark runs used:

* The same source workload
* The same benchmark code
* The same compiler
* The same compilation flags
* The same machine
* The same number of operations
* The same random seed
* The same benchmark methodology

This makes the before/after measurements useful for evaluating the performance change introduced by the optimization.

---

## Performance Optimization — Cached AVL Tree Heights

The primary optimization made to the Limit Order Book was the addition of a **height cache** to the AVL tree nodes used by the order-book price-level structure.

### The Problem

The AVL tree requires node heights when calculating balance factors:

```text
balance factor =
height(left subtree) - height(right subtree)
```

Without cached heights, obtaining the height of a subtree can require recursively traversing that subtree.

Conceptually, an uncached implementation may need to repeatedly perform work equivalent to:

```text
height(node)
    -> height(left subtree)
    -> height(right subtree)
        -> height(their children)
            -> ...
```

This becomes particularly expensive because AVL balancing is performed during structural modifications to the tree.

In an order book, these operations occur frequently when price levels are:

* Added
* Removed
* Rebalanced
* Rotated

Repeatedly calculating subtree heights therefore creates unnecessary work inside the tree's critical path.

---

### The Optimization

Each AVL node now stores its current height directly.

Conceptually:

```cpp
struct Node {
    // ...

    int height;
};
```

Instead of recursively calculating the height whenever it is needed, the implementation can retrieve it directly:

```cpp
node->height
```

When the tree structure changes, the affected node's cached height is updated:

```cpp
node->height =
    1 + std::max(
        getHeight(node->left),
        getHeight(node->right)
    );
```

where `getHeight()` simply returns the cached height of the node.

This changes height retrieval from a potentially recursive operation into a constant-time lookup.

---

### Why This Matters

The optimization **does not change the asymptotic complexity** of the AVL tree.

The tree remains:

```text
Search:    O(log N)
Insertion: O(log N)
Deletion:  O(log N)
```

The improvement instead comes from reducing the amount of repeated work performed inside those operations.

Without cached heights, the implementation can repeatedly traverse subtrees simply to determine their heights.

With cached heights:

```text
height(node)
    ↓
node->height
```

This is a direct memory access.

The optimization is especially relevant to a limit order book because changes to price levels can trigger AVL rebalancing and tree rotations. Cached heights reduce the cost of the balance calculations required during those operations.

The key distinction is therefore:

```text
Asymptotic complexity: unchanged
Constant-factor cost:   reduced
```

---

# Benchmark Methodology

All benchmarks follow the same general principle:

> Generate the workload before starting the performance timer.

This prevents workload generation and random-number generation from contaminating the measured LOB performance.

For the direct LOB benchmarks, the timer measures only the operations performed by the order book.

Benchmark A intentionally measures the complete file-processing pipeline.

The benchmarks were compiled using:

```bash
g++ -std=c++20 -O3 -march=native -DNDEBUG -I. -ILOB -IProcess_Orders LOB/*.cpp Process_Orders/*.cpp Generate_Orders/*.cpp -o main.exe
```

### Test Environment

* Language: C++20
* Compiler: `g++`
* Optimization: `-O3`
* Architecture optimization: `-march=native`
* Assertions disabled: `-DNDEBUG`
* Orders/operations per benchmark: **1,000,000**

> **Note:** Benchmark results are hardware-dependent. These measurements should be treated as a performance baseline for this implementation and workload rather than as universal performance figures.

---

# Benchmark A — Full Order Pipeline

Benchmark A measures the complete processing pipeline:

```text
Input File
    ↓
File I/O
    ↓
Order Parsing
    ↓
Order Pipeline
    ↓
Limit Order Book
```

The workload consists of **1,000,000 orders**.

Random order generation is performed before the benchmark timer starts, so generation time is not included in the measured result.

### Before Optimization

| Metric          |                 Result |
| --------------- | ---------------------: |
| Orders          |              1,000,000 |
| Time            |            **8.420 s** |
| Throughput      | **118,765 orders/sec** |
| Average latency |     **8,420 ns/order** |

### After Optimization

| Metric          |                 Result |
| --------------- | ---------------------: |
| Orders          |              1,000,000 |
| Time            |            **2.335 s** |
| Throughput      | **428,266 orders/sec** |
| Average latency |     **2,335 ns/order** |

### Improvement

| Metric              |      Improvement |
| ------------------- | ---------------: |
| Processing time     | **3.61× faster** |
| Latency             | **72.27% lower** |
| Throughput          | **3.61× higher** |
| Throughput increase |      **+260.6%** |

The optimized implementation processes approximately **428K orders/sec** through the complete file → parser → pipeline → LOB path.

---

# Benchmark B — AddLimitOrder

Benchmark B isolates the `AddLimitOrder()` operation.

The benchmark generates **1,000,000 limit orders in memory before timing begins**, then inserts every order directly into the book.

The workload uses:

* Deterministic random seed: `42`
* Prices: `90–110`
* Shares: `1–100`
* Buy/sell side: `50/50`

### Before Optimization

Five benchmark runs:

```text
279.017 ms
273.857 ms
283.682 ms
268.939 ms
290.376 ms
```

Average:

| Metric          |                 Result |
| --------------- | ---------------------: |
| Average time    |         **279.174 ms** |
| Throughput      | **~3.582M orders/sec** |
| Average latency |   **~279.17 ns/order** |

### After Optimization

Five benchmark runs:

```text
251.231 ms
302.998 ms
289.677 ms
209.951 ms
250.688 ms
```

Average:

| Metric          |                 Result |
| --------------- | ---------------------: |
| Average time    |         **260.909 ms** |
| Throughput      | **~3.833M orders/sec** |
| Average latency |   **~260.91 ns/order** |

### Improvement

| Metric       |      Before |       After |           Change |
| ------------ | ----------: | ----------: | ---------------: |
| Average time |  279.174 ms |  260.909 ms | **1.07× faster** |
| Throughput   | ~3.582M/sec | ~3.833M/sec | **~7.1% higher** |
| Latency      |   279.17 ns |   260.91 ns |  **6.54% lower** |

The insertion path improved modestly compared with the mixed-operation benchmark.

This is expected because Benchmark B isolates the `AddLimitOrder()` path rather than repeatedly exercising the broader combination of additions, cancellations, and modifications.

---

# Benchmark C — Mixed Order Book Operations

Benchmark C measures a mixed workload consisting of:

* **50% `AddLimitOrder`**
* **25% `CancelLimitOrder`**
* **25% `ModifyLimitOrder`**

The workload contains **1,000,000 operations**.

The first **10,000 operations are guaranteed to be additions**, ensuring that the book contains active orders before cancellation and modification operations begin.

The complete workload is generated before timing starts.

---

## Before Optimization

16 benchmark runs:

| Run |       Time |     Throughput |       Latency |
| --: | ---------: | -------------: | ------------: |
|   1 | 544.933 ms | 1.835M ops/sec | 544.933 ns/op |
|   2 | 569.752 ms | 1.755M ops/sec | 569.752 ns/op |
|   3 | 642.219 ms | 1.557M ops/sec | 642.219 ns/op |
|   4 | 573.920 ms | 1.742M ops/sec | 573.920 ns/op |
|   5 | 583.839 ms | 1.713M ops/sec | 583.839 ns/op |
|   6 | 575.459 ms | 1.738M ops/sec | 575.459 ns/op |
|   7 | 604.632 ms | 1.654M ops/sec | 604.632 ns/op |
|   8 | 606.005 ms | 1.650M ops/sec | 606.005 ns/op |
|   9 | 619.348 ms | 1.615M ops/sec | 619.348 ns/op |
|  10 | 608.896 ms | 1.642M ops/sec | 608.896 ns/op |
|  11 | 635.376 ms | 1.574M ops/sec | 635.376 ns/op |
|  12 | 593.441 ms | 1.685M ops/sec | 593.441 ns/op |
|  13 | 574.592 ms | 1.740M ops/sec | 574.592 ns/op |
|  14 | 568.263 ms | 1.760M ops/sec | 568.263 ns/op |
|  15 | 599.064 ms | 1.669M ops/sec | 599.064 ns/op |
|  16 | 578.046 ms | 1.730M ops/sec | 578.046 ns/op |

### Aggregate Results

| Metric             |             Result |
| ------------------ | -----------------: |
| Operations/run     |      **1,000,000** |
| Number of runs     |             **16** |
| Average latency    |  **~592.36 ns/op** |
| Average throughput | **~1.69M ops/sec** |
| Median latency     |  **~588.64 ns/op** |
| Fastest run        |  **544.933 ns/op** |
| Slowest run        |  **642.219 ns/op** |
| Standard deviation |      **~26.18 ns** |
| Fastest throughput | **1.835M ops/sec** |
| Slowest throughput | **1.557M ops/sec** |

---

## After Optimization

16 benchmark runs:

```text
146.867 ns
167.506 ns
163.665 ns
144.840 ns
147.413 ns
139.413 ns
196.712 ns
149.138 ns
160.244 ns
160.682 ns
159.232 ns
181.986 ns
151.299 ns
176.320 ns
143.133 ns
159.824 ns
```

### Aggregate Results

| Metric             |              Result |
| ------------------ | ------------------: |
| Operations/run     |       **1,000,000** |
| Number of runs     |              **16** |
| Average latency    |   **~159.27 ns/op** |
| Average throughput | **~6.279M ops/sec** |
| Median latency     |   **~159.53 ns/op** |
| Fastest run        |   **139.413 ns/op** |
| Slowest run        |   **196.712 ns/op** |
| Standard deviation |       **~15.54 ns** |
| Fastest throughput | **~7.173M ops/sec** |
| Slowest throughput | **~5.084M ops/sec** |

### Improvement

| Metric             |       Before |        After |                 Change |
| ------------------ | -----------: | -----------: | ---------------------: |
| Average latency    | 592.36 ns/op | 159.27 ns/op |       **73.11% lower** |
| Average throughput |   ~1.69M/sec |  ~6.279M/sec |      **~3.72× higher** |
| Processing speed   |           1× |        3.72× | **+271.5% throughput** |

This is the largest performance improvement among the three benchmarks.

---

# Benchmark Summary

| Benchmark | Workload                          |          Before |           After |   Speedup |
| --------- | --------------------------------- | --------------: | --------------: | --------: |
| **A**     | Full file/pipeline                |         8.420 s |         2.335 s | **3.61×** |
| **B**     | 100% `AddLimitOrder`              | 279.17 ns/order | 260.91 ns/order | **1.07×** |
| **C**     | 50% Add / 25% Cancel / 25% Modify |    592.36 ns/op |    159.27 ns/op | **3.72×** |

### Throughput Summary

| Benchmark                |             Before |              After |     Increase |
| ------------------------ | -----------------: | -----------------: | -----------: |
| **A — Full Pipeline**    | 118,765 orders/sec | 428,266 orders/sec |  **+260.6%** |
| **B — AddLimitOrder**    | ~3.582M orders/sec | ~3.833M orders/sec |   **~+7.1%** |
| **C — Mixed Operations** |     ~1.69M ops/sec |    ~6.279M ops/sec | **~+271.5%** |

---

# Performance Analysis

The benchmark results show that the cached AVL height optimization had a substantially larger effect on the mixed order-book workload than on pure limit-order insertion.

Benchmark B improved by approximately **7%**, while Benchmark C improved by approximately **3.72×**.

This difference is consistent with the fact that the mixed workload repeatedly performs additions, cancellations, and modifications, causing more frequent interaction with existing orders, price levels, and AVL-tree maintenance.

Benchmark A also improved by approximately **3.61×**, demonstrating that the internal optimization translated into a significant end-to-end improvement when the complete file-processing pipeline was used.

The current measured performance is approximately:

```text
428K orders/sec — complete file/pipeline benchmark

3.83M orders/sec — direct limit-order insertion

6.28M operations/sec — mixed Add/Cancel/Modify workload
```

These are benchmark measurements on the test system and should not be interpreted as guarantees for other CPUs, workloads, operating systems, or compiler configurations.

---

## Important Interpretation

The benchmark results show a strong correlation between introducing cached AVL heights and the observed performance improvement because the benchmark environment and methodology were held constant.

However, these benchmarks measure the **overall Limit Order Book implementation**, rather than measuring AVL height calculation in isolation.

Therefore, the results should be interpreted as:

> **Performance of the complete LOB after introducing cached AVL heights**

rather than claiming that the height lookup itself is responsible for a specific percentage of the total speedup.

This distinction is important when evaluating low-latency data structures because an optimization can affect multiple parts of the execution path indirectly through reduced tree-maintenance work, memory access patterns, and control flow.

---

## Benchmark Limitations

These benchmarks measure the performance of this implementation under the specified synthetic workloads.

They should not be interpreted as exchange-grade or production-HFT performance measurements.

Real-world performance can differ substantially depending on:

* Order-flow distribution
* Number of active price levels
* Number of active orders
* Cancel/modify frequency
* Memory allocation behavior
* CPU architecture
* Cache behavior
* NUMA configuration
* Compiler version
* Operating-system scheduling
* Input-data characteristics

The purpose of these benchmarks is to measure and track the performance of the implementation consistently as the architecture evolves.

---

# Planned Optimizations

* SIMD optimizations
* Cache-aware data layout
* Branch prediction improvements
* False sharing reduction
* Multi-threaded order ingestion
* Lock-free queues
* NUMA-aware memory layout
* Latency profiling
* Exchange protocol parser (ITCH/OUCH)

---

# Future Features

* Iceberg Orders
* Pegged Orders
* IOC/FOK Orders
* Good Till Cancelled Orders
* Snapshot generation
* Market data feed
* Replay engine
* Historical backtesting integration

---

# Motivation

This project was created to better understand the internal architecture of electronic exchanges and the software engineering techniques used in low-latency trading systems.

Rather than relying on external libraries, nearly every core data structure—including the AVL tree, memory pool, and matching engine—has been implemented from scratch.

The project is also intended to serve as a platform for experimentation with:

* Data structures
* Memory management
* CPU cache behavior
* Low-latency C++
* Market microstructure
* Benchmarking and profiling
* Exchange-style order processing

---

# License

MIT License
