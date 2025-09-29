# libnu

A collection of essential "C" utilities with zero abstraction overhead.

I'm just getting started, someday there will be hundreds of modules in this collection!

## Why?

This project is my ikigai! In the quiet moments of crafting these utilities, I find flow and peace. There are no deadlines here, no external pressures, no features demanded by work or circumstance. This is code for the sake of code, where the act of creation itself is the purpose. Each function, each optimization, each test is a meditation on the craft. In a world of constant urgency, this library exists as a sanctuary where programming returns to being the simple joy of making something work well.

## Principles

  - Pure C17, portable code
  - Direct implementations without cleverness
  - Fully tested with static and dynamic analysis
  - Performance validated through benchmarking

## Installation

```bash
git clone https://github.com/yourusername/libnu.git
cd libnu
make install-deps; make check; make install PREFIX=/usr/local
```

## Modules

  - [**nu_error**](src/error.h) ([example](examples/error.c)) - A header-only error handling system inspired by Rust's Result type, providing explicit error handling with zero overhead for the success path. It uses compound literals to avoid heap allocation and captures file/line information automatically for debugging. The module provides Result types that can hold either a success value or an error, forcing explicit error handling and making it impossible to accidentally ignore errors. Error propagation is simplified through convenience macros like `NU_RETURN_IF_ERR` and `NU_FAIL`.

  - [**nu_sort**](src/sort.h) ([example](examples/sort.c)) - Introsort (introspective sort) is a hybrid sorting algorithm that provides both the fast average performance of quicksort and the optimal worst-case performance of heapsort. The algorithm begins with quicksort and monitors recursion depth, switching to heapsort if the depth exceeds 2×log₂(n) to prevent quicksort's O(n²) worst-case behavior. For small subarrays (< 16 elements), it uses insertion sort where its lower overhead provides better performance. This three-way hybrid guarantees O(n log n) worst-case time complexity while maintaining excellent real-world performance through median-of-three pivot selection and other optimizations.
