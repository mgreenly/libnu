/*
 * Benchmarks for nu_sort
 *
 * Tests performance across different input patterns that affect
 * introsort's behavior:
 * - Random data (typical case)
 * - Already sorted (best case for many algorithms)
 * - Reverse sorted (worst case for naive quicksort)
 * - Many duplicates (tests pivot selection effectiveness)
 */

#include <nu/bench.h>
#include <stdlib.h>
#include <string.h>
#include "../src/sort.h"

/* Comparator for integers */
static int
compare_ints(const void* a, const void* b) {
  int ia = *(const int*)a;
  int ib = *(const int*)b;
  return (ia > ib) - (ia < ib);
}

/* Benchmark: 100k random elements */
NU_BENCH(sort_random_100k) {
  const size_t n = 100000;

  // Setup: Create random array
  NU_BENCH_ARRAY_SETUP(int, arr, n, rand() % 10000);

  // Benchmark the sort
  NU_BENCH_START();
  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_BENCH_END();

  // Cleanup
  NU_BENCH_ARRAY_CLEANUP(arr);
}

/* Benchmark: 50k already sorted elements */
NU_BENCH(sort_already_sorted_50k) {
  const size_t n = 50000;

  // Setup: Create already sorted array
  NU_BENCH_ARRAY_SETUP(int, arr, n, (int)i);

  // Benchmark the sort
  NU_BENCH_START();
  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_BENCH_END();

  // Cleanup
  NU_BENCH_ARRAY_CLEANUP(arr);
}

/* Benchmark: 50k reverse sorted elements */
NU_BENCH(sort_reverse_sorted_50k) {
  const size_t n = 50000;

  // Setup: Create reverse sorted array
  NU_BENCH_ARRAY_SETUP(int, arr, n, (int)(n - i));

  // Benchmark the sort
  NU_BENCH_START();
  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_BENCH_END();

  // Cleanup
  NU_BENCH_ARRAY_CLEANUP(arr);
}

/* Benchmark: 50k elements with many duplicates (only 10 unique values) */
NU_BENCH(sort_many_duplicates_50k) {
  const size_t n = 50000;

  // Setup: Create array with only 10 unique values
  srand(42);  // Fixed seed for reproducibility
  NU_BENCH_ARRAY_SETUP(int, arr, n, rand() % 10);

  // Benchmark the sort
  NU_BENCH_START();
  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_BENCH_END();

  // Cleanup
  NU_BENCH_ARRAY_CLEANUP(arr);
}

/* Benchmark: Small array (16 elements) - triggers insertion sort */
NU_BENCH(sort_small_16) {
  const size_t n = 16;

  // Setup: Small random array
  NU_BENCH_ARRAY_SETUP(int, arr, n, rand() % 100);

  // Benchmark the sort
  NU_BENCH_START();
  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_BENCH_END();

  // Cleanup
  NU_BENCH_ARRAY_CLEANUP(arr);
}

/* Benchmark: Medium array (1000 elements) */
NU_BENCH(sort_medium_1k) {
  const size_t n = 1000;

  // Setup: Medium random array
  NU_BENCH_ARRAY_SETUP(int, arr, n, rand() % 1000);

  // Benchmark the sort
  NU_BENCH_START();
  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_BENCH_END();

  // Cleanup
  NU_BENCH_ARRAY_CLEANUP(arr);
}

/* Benchmark: Sawtooth pattern (partially sorted sequences) */
NU_BENCH(sort_sawtooth_10k) {
  const size_t n = 10000;

  // Setup: Create sawtooth pattern (0,1,2,3,4, 0,1,2,3,4, ...)
  NU_BENCH_ARRAY_SETUP(int, arr, n, (int)(i % 5));

  // Benchmark the sort
  NU_BENCH_START();
  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_BENCH_END();

  // Cleanup
  NU_BENCH_ARRAY_CLEANUP(arr);
}

/* Main function - runs all benchmarks */
NU_BENCH_MAIN()