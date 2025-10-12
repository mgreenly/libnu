# libnu

A collection of essential "C" utilities with zero abstraction overhead.

I'm just getting started, someday there will be hundreds of modules in this collection!

## Why?

This project is my ikigai! In the quiet moments of crafting these utilities, I find flow and peace. There are no deadlines here, no external pressures, no features demanded by work or circumstance. This is code for the sake of code, where the act of creation itself is the purpose. Each function, each optimization, each test is a meditation on the craft. In a world of constant urgency, this library exists as a sanctuary where programming returns to being the simple joy of making something work well.

## Why C?

I genuinely enjoy writing C - there's something deeply satisfying about its directness and the mechanical connection between code and machine. While C has a reputation for being "unsafe," modern practices like aggressive testing, static/dynamic analysis, arena allocators, and clear ownership rules significantly mitigate these concerns. Employing these disciplines and crafting robust code is part of the joy. Plus, C remains the native language of Linux with unmatched portability, predictable performance, and stable ABI that every other language can consume.

## Principles

  - Pure C17, portable code
  - Direct implementations without cleverness
  - Fully tested with static and dynamic analysis
  - Performance validated through benchmarking

## Installation

```bash
make check && make install PREFIX=$HOME/.local
```

## Modules

  - **nu/error** - A header-only error handling system inspired by Rust's Result type, providing explicit error handling with zero overhead for the success path. It uses compound literals to avoid heap allocation and captures file/line information automatically for debugging. The module provides Result types that can hold either a success value or an error, forcing explicit error handling and making it impossible to accidentally ignore errors. Error propagation is simplified through convenience macros like `NU_RETURN_IF_ERR` and `NU_FAIL`. ([example](examples/error.c))

  - **nu/test** - A minimal header-only unit testing framework that dogfoods nu/error patterns by having tests return `nu_result_t` for consistent error handling. The framework uses `__attribute__((constructor))` for automatic test registration, eliminating the need for explicit test lists or main functions. It provides essential assertions (equality, comparisons, null checks, string/memory comparison) with colored output showing PASS/FAIL status and file:line information for failures. The entire framework is ~250 lines of readable code with zero dynamic allocation, making it easy to understand and modify. Tests are defined with `NU_TEST(name)` and the framework automatically discovers and runs all tests when `NU_TEST_MAIN()` is used. ([example](examples/test.c))

  - **nu/bench** - A header-only benchmarking framework designed for measuring and comparing performance of C code. The framework provides automatic benchmark registration via `__attribute__((constructor))`, eliminating manual benchmark lists. It features warmup runs to stabilize CPU/cache state, multiple iterations for statistical accuracy, and reports timing in appropriate units (μs/ms/s). Benchmarks are defined with `NU_BENCH(name)` and the framework provides helper macros for common patterns like array setup/cleanup. The timing mechanism uses clock-based measurements with automatic calculation of mean, min, and max times. Command-line options support verbose output, custom iteration counts, warmup configuration, and filtering specific benchmarks. The entire framework is ~250 lines of focused code with zero dynamic allocation in the core framework. ([example](examples/bench.c))

  - **nu/sort** - Introsort (introspective sort) is a hybrid sorting algorithm that provides both the fast average performance of quicksort and the optimal worst-case performance of heapsort. The algorithm begins with quicksort and monitors recursion depth, switching to heapsort if the depth exceeds 2×log₂(n) to prevent quicksort's O(n²) worst-case behavior. For small subarrays (< 16 elements), it uses insertion sort where its lower overhead provides better performance. This three-way hybrid guarantees O(n log n) worst-case time complexity while maintaining excellent real-world performance through median-of-three pivot selection and other optimizations. ([example](examples/sort.c))
