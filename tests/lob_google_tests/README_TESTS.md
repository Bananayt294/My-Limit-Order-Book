# GoogleTest Suite for My-Limit-Order-Book

This package adds a GoogleTest-based correctness suite for the current Limit Order Book implementation.

## Test coverage

The suite is organized around the actual LOB components:

- `test_order.cpp` — order construction, side, modification, partial fills, full execution
- `test_limit.cpp` — price level construction, empty state, order append, FIFO head/tail, aggregate quantity
- `test_avl.cpp` — LL/RR/LR/RL rotations, parent pointers, in-order ordering, AVL balance, randomized insertion, independent buy/sell trees
- `test_book.cpp` — best bid/ask, price-level aggregation, FIFO matching, cancellation, modification, crossing limit orders, market orders, multi-level fills, empty-book safety, traversal, structural invariants
- `test_stop_orders.cpp` — stop orders, stop-limit orders, cancel/modify, and interaction with normal book state
- `test_pipeline.cpp` — replay-file parsing for add/cancel/modify operations and ordered processing
- `test_randomized.cpp` — larger randomized invariant checks and repeated-price aggregation

There are 56 GoogleTest test cases in the suite.

## Important implementation detail

`test_support.hpp` does not make your private members public. It finds the AVL root by following the public `get_parent()` links from the current best-price edge, then validates:

- BST ordering
- parent pointers
- AVL balance
- best bid / best ask edge correctness

Stop-order tests use the public `getRandomOrder(type, rng)` API used by the existing order generator rather than reaching into private stop maps.

## Setup

GoogleTest 1.18.0 is fetched by CMake during configuration. GoogleTest 1.18.0 is the current release at the time this package was created, and its current CMake quickstart uses `GTest::gtest_main` together with CTest discovery.

You need:

- C++20-capable `g++`
- CMake 3.20+
- Git (CMake uses it to fetch GoogleTest)

Your existing project layout should look like:

```text
My-Limit-Order-Book/
├── LOB/
├── Process_Orders/
├── Generate_Orders/
├── data/
├── tests/
├── main.cpp
├── CMakeLists.txt
└── run_tests.ps1
```

Copy the `tests/` directory and the root-level `CMakeLists.txt` / `run_tests.ps1` from this package into the project root.

## Run with PowerShell

From the project root:

```powershell
.\run_tests.ps1
```

Or manually:

```powershell
cmake -S . -B build-tests -G "MinGW Makefiles"
cmake --build build-tests --parallel
ctest --test-dir build-tests --output-on-failure
```

The first CMake configure downloads GoogleTest, so the first run requires internet access. Later builds can reuse the fetched dependency.

## If your compiler is not using MinGW Makefiles

Use your preferred CMake generator instead of `-G "MinGW Makefiles"`. The important part is that CMake uses the same C++ toolchain as the rest of the project.

## Current validation status

The test source files were generated against the API patterns in the saved project code. The actual repository source tree was not available in the execution environment, so the full suite was not compiled here. Run `run_tests.ps1` in the project root to validate the exact current headers and implementations.
