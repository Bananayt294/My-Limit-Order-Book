Limit Order Book

A high-performance C++20 Limit Order Book implementing price-time priority (FIFO) matching using an AVL tree for price levels and a custom memory pool for order allocation.

This project was built to explore the data structures and algorithms used in modern electronic exchanges and high-frequency trading systems while emphasizing performance, memory efficiency, and clean object-oriented design.

Features

Price-Time Priority (FIFO)

AVL Tree for price levels with cached node heights

$O(\log N)$ insertion and deletion of price levels

$O(1)$ access to best bid and ask

Doubly linked list of orders at each price level

Custom memory pool allocator for orders

Market Orders

Limit Orders

Stop Orders

Order Modification

Order Cancellation

Automatic removal of empty price levels

Fast order lookup using hash tables

Data Structures

Price Levels

Price levels are stored inside a height-balanced AVL tree.

Each node contains:

Price

Total volume

FIFO queue of orders

Cached node height (int height)

Parent pointer

Left child

Right child

Maintaining an AVL tree guarantees:

$O(\log N)$ insertion

$O(\log N)$ deletion

$O(\log N)$ lookup

while keeping the tree balanced after every update.

Orders

Orders are stored inside a doubly linked list at each price level.

Head <-> Order <-> Order <-> Tail


This allows:

$O(1)$ insertion

$O(1)$ cancellation

$O(1)$ modification

while preserving FIFO execution.

Memory Pool

Instead of allocating every order with new, the project uses a custom memory pool.

Benefits include:

Reduced heap allocations

Better cache locality

Lower allocation overhead

More deterministic performance

Orders are recycled rather than repeatedly allocated and freed.

Performance Optimization — Cached AVL Tree Heights

The primary optimization made to the Limit Order Book was the addition of a height cache to the AVL tree nodes used by the price-level structure.

The Problem

The AVL tree requires node heights when calculating balance factors:

$$\text{balance factor} = \text{height}(\text{left subtree}) - \text{height}(\text{right subtree})$$

Without cached heights, obtaining the height of a subtree can require recursively traversing that subtree. This becomes particularly expensive because AVL balancing is performed during structural modifications to the tree (insertions, deletions, and rotations) which occur frequently in an active order book.

The Optimization

Each AVL node now stores its current height directly:

struct Node {
    // ...
    int height;
};


Instead of recursively calculating height on demand, the height is retrieved in constant time:

node->height


When structural changes occur, the affected node's height is updated in $O(1)$ using cached child heights:

node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));


(where getHeight() returns 0 for null nodes or the cached node->height value).

Why This Matters

This optimization does not alter the asymptotic time complexity of the AVL tree ($O(\log N)$ search, insertion, deletion, and modification). Instead, it dramatically reduces the constant factor cost during structural modifications, rebalancing, and rotations.

Complexity

Operation

Complexity

Add Limit Order

$O(\log N)$

Cancel Order

$O(1) + O(\log N)$ if price level removed

Modify Order

$O(1) / O(\log N)$ depending on modification

Market Order

$O(\log N)$

Find Order

$O(1)$

Best Bid

$O(1)$

Best Ask

$O(1)$

Project Structure

LOB/
│
├── book.cpp
├── book.hpp
├── limit.cpp
├── limit.hpp
├── order.cpp
├── order.hpp
├── order_pool.cpp
└── order_pool.hpp
│
Process_Orders/
│
├── OrderPipeline.cpp
└── OrderPipeline.hpp
│
Generate_Orders/
│
└── GenerateOrders.cpp


Build & Run

Compilation

Build with GCC (C++20, -O3, -march=native, -DNDEBUG):

g++ -std=c++20 -O3 -march=native -DNDEBUG -I. -ILOB -IProcess_Orders LOB/*.cpp Process_Orders/*.cpp Generate_Orders/*.cpp -o main.exe


Execution

./main.exe


Performance Benchmarks

The order book was benchmarked before and after adding the cached AVL tree height optimization.

Test Methodology & Environment

To ensure strict, reproducible comparison, both benchmark sets were executed under identical conditions:

Environment: Same machine, OS, compiler (g++), standard (C++20), and optimization flags (-O3 -march=native -DNDEBUG).

Workload Parameters: Same random seed (42), prices (90–110), volumes (1–100), side distribution (50/50), and number of operations (1,000,000).

Isolation: For direct in-memory benchmarks (B & C), workloads were pre-generated before starting the timer to isolate LOB execution from I/O or RNG overhead.

Note: Benchmark results are hardware-dependent. These numbers serve as relative baseline performance figures for this implementation.

Benchmark A — Full Order Pipeline

Measures the end-to-end order processing path using a pre-generated file of 1,000,000 orders:

$$\text{Input File} \longrightarrow \text{File I/O} \longrightarrow \text{Order Parsing} \longrightarrow \text{Order Pipeline} \longrightarrow \text{Limit Order Book}$$

Metric

Before Optimization

After Optimization

Change

Orders processed

1,000,000

1,000,000

—

Total time

8.420 s

2.335 s

3.61× faster

Throughput

118,765 orders/sec

428,266 orders/sec

+260.6%

Average latency

8,420 ns/order

2,335 ns/order

72.27% lower

Benchmark B — Add Limit Orders

Isolates raw AddLimitOrder() performance across 5 consecutive runs of 1,000,000 limit order insertions in memory.

Detailed Run Data (After Optimization)

Run 1: 251.231 ms

Run 2: 302.998 ms

Run 3: 289.677 ms

Run 4: 209.951 ms

Run 5: 250.688 ms

Summary Comparison

Metric

Before Optimization

After Optimization

Change

Average time

279.174 ms

260.909 ms

1.07× faster

Average throughput

~3.58M orders/sec

~3.833M orders/sec

+~7.1%

Average latency

279.17 ns/order

260.91 ns/order

6.54% lower

The modest improvement here is expected as pure insertions exercise rebalancing less frequently than mixed deletion/modification workloads.

Benchmark C — Mixed Order Book Operations

Measures a dynamic workload of 1,000,000 operations simulating active trading:

50% AddLimitOrder

25% CancelLimitOrder

25% ModifyLimitOrder

(Note: The first 10,000 operations are guaranteed additions to build initial depth before cancels/modifications begin).

Detailed Run Data (After Optimization — 16 Runs in ns/op)

146.867 | 167.506 | 163.665 | 144.840 | 147.413 | 139.413 | 196.712 | 149.138
160.244 | 160.682 | 159.232 | 181.986 | 151.299 | 176.320 | 143.133 | 159.824


Summary Comparison

Metric

Before Optimization

After Optimization

Change

Average latency

592.36 ns/op

159.27 ns/op

73.11% lower

Median latency

588.64 ns/op

159.53 ns/op

72.90% lower

Fastest run

544.93 ns/op

139.41 ns/op

~7.17M ops/sec

Slowest run

642.22 ns/op

196.71 ns/op

~5.08M ops/sec

Average throughput

~1.69M ops/sec

~6.279M ops/sec

3.72× higher (+271.5%)

Benchmark Performance Summary

Benchmark

Workload

Before (Latency / Throughput)

After (Latency / Throughput)

Overall Speedup

A

Full File Pipeline

8,420 ns / 118.8K ops/s

2,335 ns / 428.3K ops/s

3.61×

B

100% Add Limit Order

279.17 ns / 3.58M ops/s

260.91 ns / 3.83M ops/s

1.07×

C

Mixed (50% Add / 25% Cancel / 25% Modify)

592.36 ns / 1.69M ops/s

159.27 ns / 6.28M ops/s

3.72×

Performance Analysis

Impact on Tree Maintenance: The height-cache optimization had a massive impact on mixed operations (3.72× speedup in Benchmark C). Mixed operations trigger frequent additions, modifications, and deletions that create and destroy price levels, causing repeated tree rebalancing and rotations.

Pipeline Scaling: The 3.61× gain in Benchmark A demonstrates that optimizing core data structures yields substantial end-to-end benefits across the entire processing pipeline.

Current System Capability:

~428K orders/sec end-to-end file pipeline.

~3.83M orders/sec pure limit order insertion.

~6.28M ops/sec mixed trading workload.

Planned Optimizations

SIMD optimizations

Cache-aware data layout

Branch prediction improvements

False sharing reduction

Multi-threaded order ingestion

Lock-free queues

NUMA-aware memory layout

Latency profiling

Exchange protocol parser (ITCH/OUCH)

Future Features

Iceberg Orders

Pegged Orders

IOC/FOK Orders

Good Till Cancelled Orders

Snapshot generation

Market data feed

Replay engine

Historical backtesting integration

Motivation

This project was created to better understand the internal architecture of electronic exchanges and the software engineering techniques used in low-latency trading systems.

Rather than relying on external libraries, nearly every core data structure—including the AVL tree, memory pool, and matching engine—has been implemented from scratch.

License

MIT License