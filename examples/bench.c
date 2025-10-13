/**
 * nu/bench.h Tutorial Example
 *
 * This example demonstrates how to write benchmarks using the nu/bench.h
 * framework. We'll benchmark different algorithms to show how to measure
 * and compare performance.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <nu/bench.h>

/*
 * Step 1: Understanding the benchmarking framework
 *
 * The nu/bench.h framework provides:
 * - NU_BENCH(name) to define benchmarks
 * - NU_BENCH_START() to start timing
 * - NU_BENCH_END() to stop timing
 * - Automatic registration and execution
 * - Statistical analysis (mean, min, max)
 * - Warmup runs to stabilize performance
 */

/*
 * Example 1: Basic benchmark
 *
 * This shows the simplest benchmark - measuring a single operation.
 * The framework will automatically run this multiple times and report
 * the average time.
 */
NU_BENCH(string_concat_simple) {
  char buffer[1024];
  const char* parts[] = {"Hello", " ", "World", "!", NULL};

  NU_BENCH_START();

  // Operation to benchmark: concatenate strings
  buffer[0] = '\0';
  for (int32_t i = 0; parts[i] != NULL; i++) {
    strcat(buffer, parts[i]);
  }

  NU_BENCH_END();
}

/*
 * Example 2: Benchmark with dynamic allocation
 *
 * For benchmarks that need allocated memory, use the helper macros:
 * - NU_BENCH_ARRAY_SETUP() to allocate and initialize
 * - NU_BENCH_ARRAY_CLEANUP() to free memory
 */
NU_BENCH(array_sum_1000) {
  const size_t n = 1000;

  // Setup: Create array with random values
  NU_BENCH_ARRAY_SETUP(int, arr, n, rand() % 100);

  // The actual operation we're benchmarking
  NU_BENCH_START();

  long sum = 0;
  for (size_t i = 0; i < n; i++) {
    sum += arr[i];
  }

  NU_BENCH_END();

  // Cleanup
  NU_BENCH_ARRAY_CLEANUP(arr);

  // Prevent compiler from optimizing away the sum
  (void)sum;
}

/*
 * Example 3: Comparing algorithms
 *
 * Define multiple benchmarks to compare different approaches.
 * Here we compare linear search vs binary search.
 */

// Helper: Linear search
static int32_t
linear_search (const int* arr, size_t n, int32_t target)
{
  for (size_t i = 0; i < n; i++) {
    if (arr[i] == target)return (int32_t)i;
  }
  return -1;
}

// Helper: Binary search (requires sorted array)
static int32_t
binary_search (const int* arr, size_t n, int32_t target)
{
  size_t left = 0, right = n;
  while (left < right) {
    size_t mid = left + (right - left) / 2;
    if (arr[mid] == target)return (int32_t)mid;
    if (arr[mid] < target)left = mid + 1;
    else right = mid;
  }
  return -1;
}

// Benchmark linear search
NU_BENCH(search_linear_10k) {
  const size_t n = 10000;

  // Create sorted array (for fair comparison with binary search)
  NU_BENCH_ARRAY_SETUP(int, arr, n, (int)i * 2);

  // Search for multiple values to get stable timing
  NU_BENCH_START();

  // Search for 10 different values
  for (int32_t i = 0; i < 10; i++) {
    int32_t target = (rand() % (int32_t)n) * 2;
    int32_t result = linear_search(arr, n, target);
    (void)result;      // Prevent optimization
  }

  NU_BENCH_END();

  NU_BENCH_ARRAY_CLEANUP(arr);
}

// Benchmark binary search
NU_BENCH(search_binary_10k) {
  const size_t n = 10000;

  // Create same sorted array
  NU_BENCH_ARRAY_SETUP(int, arr, n, (int)i * 2);

  NU_BENCH_START();

  // Search for same 10 values
  for (int32_t i = 0; i < 10; i++) {
    int32_t target = (rand() % (int32_t)n) * 2;
    int32_t result = binary_search(arr, n, target);
    (void)result;      // Prevent optimization
  }

  NU_BENCH_END();

  NU_BENCH_ARRAY_CLEANUP(arr);
}

/*
 * Example 4: Benchmarking different input sizes
 *
 * Create multiple benchmarks with different sizes to understand
 * how performance scales.
 */
NU_BENCH(bubble_sort_100) {
  const size_t n = 100;
  NU_BENCH_ARRAY_SETUP(int, arr, n, rand() % 1000);

  NU_BENCH_START();

  // Simple bubble sort
  for (size_t i = 0; i < n-1; i++) {
    for (size_t j = 0; j < n-i-1; j++) {
      if (arr[j] > arr[j+1]) {
        int32_t temp = arr[j];
        arr[j]   = arr[j+1];
        arr[j+1] = temp;
      }
    }
  }

  NU_BENCH_END();

  NU_BENCH_ARRAY_CLEANUP(arr);
}

NU_BENCH(bubble_sort_500) {
  const size_t n = 500;
  NU_BENCH_ARRAY_SETUP(int, arr, n, rand() % 1000);

  NU_BENCH_START();

  // Same bubble sort, larger input
  for (size_t i = 0; i < n-1; i++) {
    for (size_t j = 0; j < n-i-1; j++) {
      if (arr[j] > arr[j+1]) {
        int32_t temp = arr[j];
        arr[j]   = arr[j+1];
        arr[j+1] = temp;
      }
    }
  }

  NU_BENCH_END();

  NU_BENCH_ARRAY_CLEANUP(arr);
}

/*
 * Example 5: Micro-benchmarks
 *
 * For very fast operations, benchmark a loop of operations
 * to get measurable times.
 */
NU_BENCH(string_length_micro) {
  const char* test_strings[] = {
    "Hello, World!",
    "The quick brown fox jumps over the lazy dog",
    "Benchmarking is important for performance",
    "Short",
    "A much longer string that contains more characters to process",
    NULL
  };

  NU_BENCH_START();

  // Run strlen many times for measurable timing
  size_t total = 0;
  for (int32_t iter = 0; iter < 10000; iter++) {
    for (int32_t i = 0; test_strings[i]; i++) {
      total += strlen(test_strings[i]);
    }
  }

  NU_BENCH_END();

  (void)total;    // Prevent optimization
}

/*
 * Step 2: Define the main function
 *
 * Use NU_BENCH_MAIN() to create a main function that runs all benchmarks.
 * The framework provides command-line options:
 *   -v           Verbose output with detailed statistics
 *   -n <count>   Number of iterations (default: 100)
 *   -w <count>   Number of warmup runs (default: 5)
 *   -f <filter>  Run only benchmarks matching filter
 *   -h           Show help
 */

/*
 * Tutorial notes printed before benchmarks run
 */
static void __attribute__((constructor(200)))
print_tutorial_header (void)
{
  printf("==============================================================\n");
  printf("                    nu/bench.h Tutorial\n");
  printf("==============================================================\n");
  printf("\nThis tutorial demonstrates:\n");
  printf("  1. Basic benchmarking with NU_BENCH\n");
  printf("  2. Memory allocation helpers\n");
  printf("  3. Algorithm comparison\n");
  printf("  4. Performance scaling analysis\n");
  printf("  5. Micro-benchmarking techniques\n");
  printf("\nKey concepts:\n");
  printf("  - Benchmarks run multiple iterations for statistical accuracy\n");
  printf("  - Warmup runs eliminate cold-start effects\n");
  printf("  - Times are reported as mean of all iterations\n");
  printf("  - Smaller times are better (faster execution)\n");
  printf("\nCommand-line options:\n");
  printf("  ./bench           Run all benchmarks\n");
  printf("  ./bench -v        Show detailed statistics\n");
  printf("  ./bench -n 1000   Run 1000 iterations\n");
  printf("  ./bench -f sort   Run only 'sort' benchmarks\n");
  printf("\n");
}

NU_BENCH_MAIN()
