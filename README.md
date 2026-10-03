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
- AVL Tree for price levels with cached node heights
- $O(\log N)$ insertion and deletion of price levels
- $O(1)$ access to best bid and ask
- Doubly linked list of orders at each price level
- Custom memory pool allocator for orders
- Market Orders
- Limit Orders
- Stop Orders
- Order Modification
- Order Cancellation
- Automatic removal of empty price levels
- Fast order lookup using hash tables

---

## Data Structures

### Price Levels

Price levels are stored inside a height-balanced AVL tree.

Each node contains:

- Price
- Total volume
- FIFO queue of orders
- Cached node height (`int height`)
- Parent pointer
- Left child
- Right child

Maintaining an AVL tree guarantees:

- $O(\log N)$ insertion
- $O(\log N)$ deletion
- $O(\log N)$ lookup

while keeping the tree balanced after every update.

---

### Orders

Orders are stored inside a doubly linked list at each price level.