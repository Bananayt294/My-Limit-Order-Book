# My-Limit-Order-Book

# Architecture
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

Performance testing is ongoing as additional optimizations are implemented.

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