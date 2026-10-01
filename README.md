# My-Limit-Order-Book

# Architecture (HUGE CREDIT TO BRPROJECTS FOR INSPIRATION)
<br>
<p align="center">
  <img src="https://raw.githubusercontent.com/brprojects/Limit-Order-Book/main/figures/architecture.png" alt="Limit Order Book Architecture (credit to brprojects)" width="900"/>
</p>


# Limit Order Book

A high-performance C++20 Limit Order Book implementing price-time priority (FIFO) matching using an AVL tree for price levels and a custom memory pool for order allocation.

This project was built to explore the data structures and algorithms used in modern electronic exchanges and high-frequency trading systems while emphasizing performance, memory efficiency, and clean object-oriented design.

---

## Features

- Price-Time Priority (FIFO)
- AVL Tree for price levels
- O(log N) insertion and deletion of price levels
- O(1) access to best bid and ask
- Doubly linked list of orders at each price level
- Custom memory pool allocator for orders
- Market Orders
- Limit Orders
- Stop Orders
- Order Modification
- Order Cancellation
- Automatic removal of empty price levels
- Order lookup using hash tables

---

## Data Structures

### Price Levels

Price levels are stored inside an AVL tree.

Each node contains:

- Price
- Total volume
- FIFO queue of orders
- Parent pointer
- Left child
- Right child

Maintaining an AVL tree guarantees

- O(log N) insertion
- O(log N) deletion
- O(log N) lookup

while keeping the tree balanced after every update.

---

### Orders

Orders are stored inside a doubly linked list at each price level.

```
Head <-> Order <-> Order <-> Tail
```

This allows

- O(1) insertion
- O(1) cancellation
- O(1) modification

while preserving FIFO execution.

---

### Memory Pool

Instead of allocating every order with `new`, the project uses a custom memory pool.

Benefits include

- Reduced heap allocations
- Better cache locality
- Lower allocation overhead
- More deterministic performance

Orders are recycled rather than repeatedly allocated and freed.

---

## Complexity

| Operation | Complexity |
|----------|------------|
| Add Limit Order | O(log N) |
| Cancel Order | O(1) + O(log N) if price level removed |
| Modify Order | O(1) / O(log N) depending on modification |
| Market Order | O(log N) |
| Find Order | O(1) |
| Best Bid | O(1) |
| Best Ask | O(1) |

---

## Project Structure

```
LOB/
│
├── book.cpp
├── book.hpp
├── limit.cpp
├── limit.hpp
├── order.cpp
├── order.hpp
├── order_pool.cpp
├── order_pool.hpp
│
Process_Orders/
│
├── OrderPipeline.cpp
└── OrderPipeline.hpp
│
Generate_Orders/
│
└── GenerateOrders.cpp
```

---

## Build

Using GCC

```bash
g++ -std=c++20 -I. -ILOB -IProcess_Orders LOB/*.cpp Process_Orders/*.cpp -o main.exe
```

Run

```bash
./main.exe
```

---

## Current Performance

Current implementation includes

- AVL balanced price tree
- FIFO matching engine
- Custom order allocator

## Performance Benchmarks

The order book is benchmarked using three different workloads to separate raw order-book performance from file-processing overhead and mixed-operation performance.

### Test Environment

* Language: C++20
* Compiler: `g++`
* Optimization: `-O3`
* Architecture optimization: `-march=native`
* Assertions disabled: `-DNDEBUG`
* Orders/operations per benchmark: **1,000,000**

> **Note:** Benchmark results are hardware-dependent. These numbers should be used as a baseline for this implementation and workload rather than as universal performance figures.

---

### Benchmark A — File Pipeline

Benchmark A measures the complete order-processing pipeline using a pre-generated file containing **1,000,000 orders**.

This benchmark includes:

* File I/O
* Order parsing
* Order processing
* Limit order book operations

Random order generation occurs before the timer and is therefore **not included** in the measured time.

| Metric             |                  Result |
| ------------------ | ----------------------: |
| Orders processed   |               1,000,000 |
| Total time         |             **8.420 s** |
| Throughput         | **~118,765 orders/sec** |
| Average time/order |            **~8.42 µs** |

This benchmark represents the end-to-end cost of processing orders through the file-based pipeline.

---

### Benchmark B — Add Limit Orders

Benchmark B measures the raw performance of `AddLimitOrder()` without file I/O, parsing, or random workload generation.

A pre-generated workload of **1,000,000 limit orders** is stored in memory before timing begins.

Five runs were performed:

| Run |       Time |        Throughput |          Latency |
| --: | ---------: | ----------------: | ---------------: |
|   1 | 279.017 ms | 3.584M orders/sec | 279.017 ns/order |
|   2 | 273.857 ms | 3.652M orders/sec | 273.857 ns/order |
|   3 | 283.682 ms | 3.525M orders/sec | 283.682 ns/order |
|   4 | 268.939 ms | 3.718M orders/sec | 268.939 ns/order |
|   5 | 290.376 ms | 3.444M orders/sec | 290.376 ns/order |

**Average:**

| Metric             |                Result |
| ------------------ | --------------------: |
| Average time       |         **279.17 ms** |
| Average throughput | **~3.58M orders/sec** |
| Average latency    |  **~279.17 ns/order** |
| Fastest run        | **3.718M orders/sec** |
| Slowest run        | **3.444M orders/sec** |

This benchmark is intended to measure the performance of the core limit-order insertion path.

---

### Benchmark C — Mixed Order Book Operations

Benchmark C measures a mixed workload consisting of:

* **50% `AddLimitOrder`**
* **25% `CancelLimitOrder`**
* **25% `ModifyLimitOrder`**

The workload is generated completely before the timer starts, so random-number generation and workload construction are excluded from the measured time.

Each run processes **1,000,000 operations**.

#### Results

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

#### Aggregate Results

| Metric             |             Result |
| ------------------ | -----------------: |
| Operations/run     |      **1,000,000** |
| Number of runs     |             **16** |
| Average latency    |  **~592.36 ns/op** |
| Average throughput | **~1.69M ops/sec** |
| Median latency     |  **~588.64 ns/op** |
| Fastest run        |  **544.933 ns/op** |
| Slowest run        |  **642.219 ns/op** |
| Fastest throughput | **1.835M ops/sec** |
| Slowest throughput | **1.557M ops/sec** |
| Standard deviation |      **~26.18 ns** |

---

### Benchmark Summary

| Benchmark | Workload                          | Operations |     Throughput | Avg. Latency |
| --------- | --------------------------------- | ---------: | -------------: | -----------: |
| **A**     | File pipeline                     |  1M orders |    ~118.8K/sec |     ~8.42 µs |
| **B**     | 100% Add                          |  1M orders | **~3.58M/sec** |  **~279 ns** |
| **C**     | 50% Add / 25% Cancel / 25% Modify |     1M ops | **~1.69M/sec** |  **~592 ns** |

### Interpretation

Benchmark A measures the complete file-based processing pipeline, while Benchmarks B and C isolate the in-memory order-book operations.

Benchmark B provides a baseline for the raw limit-order insertion path, achieving approximately **3.58 million orders/sec**.

Benchmark C provides a more representative mixed-operation workload and achieves approximately **1.69 million operations/sec**, with an average latency of approximately **592 ns per operation**.

These benchmarks serve as the baseline for future optimization work. Future optimizations will be evaluated by rerunning the same workloads and comparing throughput and latency against these results.


---

## Planned Optimizations

- SIMD optimizations
- Cache-aware data layout
- Branch prediction improvements
- False sharing reduction
- Multi-threaded order ingestion
- Lock-free queues
- NUMA-aware memory layout
- Benchmark suite
- Latency profiling
- Exchange protocol parser (ITCH/OUCH)

---

## Future Features

- Iceberg Orders
- Pegged Orders
- IOC/FOK Orders
- Good Till Cancelled Orders
- Snapshot generation
- Market data feed
- Replay engine
- Historical backtesting integration

---

## Motivation

This project was created to better understand the internal architecture of electronic exchanges and the software engineering techniques used in low-latency trading systems.

Rather than relying on external libraries, nearly every core data structure—including the AVL tree, memory pool, and matching engine—has been implemented from scratch.

---

## License

MIT License