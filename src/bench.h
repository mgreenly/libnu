/*
 * nu/bench.h - Minimal benchmarking framework for libnu
 *
 * This framework provides simple performance benchmarking for libnu modules.
 * It measures execution time, handles multiple iterations, and reports statistics.
 *
 * Features:
 * - Automatic benchmark registration via __attribute__((constructor))
 * - Warmup runs to stabilize cache/CPU state
 * - Statistical reporting (min/max/mean)
 * - No dynamic allocation in the framework
 * - ~250 lines of focused benchmarking code
 *
 * Usage:
 *   #include <nu/bench.h>
 *
 *   NU_BENCH(sort_random) {
 *     int arr[1000];
 *     // setup...
 *     NU_BENCH_START();
 *     sort(arr, 1000);
 *     NU_BENCH_END();
 *   }
 *
 *   NU_BENCH_MAIN()
 */

#ifndef NU_BENCH_H
#define NU_BENCH_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>

/* For internal library builds, use NU_MALLOC/NU_FREE macros if defined */
/* For end users, use standard malloc/free */
#ifdef NU_MALLOC
extern void* NU_MALLOC(size_t size);
extern void NU_FREE(void* ptr);

#else
#define NU_MALLOC malloc
#define NU_FREE free
#endif

// Benchmark function signature
typedef void (* nu_bench_fn)(void);

// Benchmark registration entry
typedef struct {
  const char* name;
  nu_bench_fn fn;
  double last_time;  // Last recorded time for current benchmark
} nu_bench_entry_t;

// Global benchmark state
static struct {
  nu_bench_entry_t benches[128];  // Max 128 benchmarks per file
  int count;

  // Current benchmark being run
  int current_bench;
  size_t current_iteration;
  size_t total_iterations;
  size_t warmup_runs;

  // Timing data for current benchmark
  double times[1000];  // Support up to 1000 iterations
  size_t time_count;
  clock_t start_time;

  // Configuration
  bool verbose;
  const char* filter;  // Run only benchmarks matching this
} nu_bench_state = {
  .total_iterations = 100,
  .warmup_runs      = 5,
  .verbose          = false
};

// Register a benchmark
static inline void
nu_bench_register_impl (
  const char* name,
  nu_bench_fn fn)
{
  if (nu_bench_state.count >= 128) {
    fprintf(stderr, "ERROR: Too many benchmarks registered (max 128)\n");
    exit(1);
  }
  nu_bench_state.benches[nu_bench_state.count].name = name;
  nu_bench_state.benches[nu_bench_state.count].fn   = fn;
  nu_bench_state.count++;
}

// Define and register a benchmark
#define NU_BENCH(name) \
        static void nu_bench_ ## name(void); \
        __attribute__((constructor(300))) \
        static void nu_bench_register_ ## name(void) { \
          nu_bench_register_impl(#name, nu_bench_ ## name); \
        } \
        static void nu_bench_ ## name(void)

// Start timing
#define NU_BENCH_START() \
        nu_bench_state.start_time = clock()

// End timing and record
#define NU_BENCH_END() \
        do { \
          clock_t end    = clock(); \
          double elapsed = ((double)(end - nu_bench_state.start_time)) / CLOCKS_PER_SEC; \
          if (nu_bench_state.current_iteration >= nu_bench_state.warmup_runs) { \
            nu_bench_state.times[nu_bench_state.time_count++] = elapsed; \
          } \
        } while (0)

// Helper for array setup
#define NU_BENCH_ARRAY_SETUP(type, arr, size, init_expr) \
        type* arr = NU_MALLOC((size) * sizeof(type)); \
        if (!arr) { \
          fprintf(stderr, "Benchmark allocation failed\n"); \
          exit(1); \
        } \
        for (size_t i = 0; i < (size); i++) { \
          arr[i] = (init_expr); \
        }

#define NU_BENCH_ARRAY_CLEANUP(arr) \
        NU_FREE(arr)

// Calculate statistics for current benchmark
static inline void
nu_bench_calculate_stats (
  double* min,
  double* max,
  double* mean,
  double* median)
{
  if (nu_bench_state.time_count == 0) {
    *min = *max = *mean = *median = 0.0;
    return;
  }

  *min = DBL_MAX;
  *max = 0.0;
  double sum = 0.0;

  // Calculate min, max, sum
  for (size_t i = 0; i < nu_bench_state.time_count; i++) {
    double t = nu_bench_state.times[i];
    if (t < *min)*min = t;
    if (t > *max)*max = t;
    sum += t;
  }

  *mean = sum / (double)nu_bench_state.time_count;

  // Simple median (would need to sort for true median)
  // For now, just use mean as approximation
  *median = *mean;
}

// Run a single benchmark
static inline void
nu_bench_run_one (int idx)
{
  nu_bench_entry_t* bench = &nu_bench_state.benches[idx];

  // Reset timing data
  nu_bench_state.current_bench = idx;
  nu_bench_state.time_count    = 0;

  // Run warmup + actual iterations
  size_t total_runs = nu_bench_state.warmup_runs + nu_bench_state.total_iterations;

  for (size_t i = 0; i < total_runs; i++) {
    nu_bench_state.current_iteration = i;
    bench->fn();
  }

  // Calculate and display stats
  double min, max, mean, median;
  nu_bench_calculate_stats(&min, &max, &mean, &median);

  // Format time with proper padding (12 chars total for time + unit)
  char time_buf[32];
  if (mean < 0.001) {
    snprintf(time_buf, sizeof(time_buf), "%9.3f μs", mean * 1000000);
  } else if (mean < 1.0) {
    snprintf(time_buf, sizeof(time_buf), "%9.3f ms", mean * 1000);
  } else {
    snprintf(time_buf, sizeof(time_buf), "%9.3f s", mean);
  }

  printf("  %s  %s", time_buf, bench->name);

  if (nu_bench_state.verbose) {
    printf(" (min: %.3fs, max: %.3fs, %zu iterations)",
      min, max, nu_bench_state.total_iterations);
  }

  printf("\n");
}

// Run all benchmarks
static inline int
nu_bench_run_all (
  int argc,
  char** argv)
{
  // Parse simple command line args
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
      nu_bench_state.verbose = true;
    } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      nu_bench_state.total_iterations = (size_t)atoi(argv[++i]);
    } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
      nu_bench_state.warmup_runs = (size_t)atoi(argv[++i]);
    } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
      nu_bench_state.filter = argv[++i];
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printf("Usage: %s [options]\n", argv[0]);
      printf("Options:\n");
      printf("  -v, --verbose    Show detailed statistics\n");
      printf("  -n <iterations>  Number of iterations (default: 100)\n");
      printf("  -w <warmups>     Number of warmup runs (default: 5)\n");
      printf("  -f <filter>      Run only benchmarks containing this string\n");
      printf("  -h, --help       Show this help\n");
      return 0;
    }
  }

  printf("Running benchmarks");
  if (nu_bench_state.verbose) {
    printf(" (%zu iterations, %zu warmups)",
      nu_bench_state.total_iterations,
      nu_bench_state.warmup_runs);
  }
  printf("...\n");

  int run_count = 0;
  for (int i = 0; i < nu_bench_state.count; i++) {
    // Apply filter if specified
    if (nu_bench_state.filter) {
      if (!strstr(nu_bench_state.benches[i].name, nu_bench_state.filter)) {
        continue;
      }
    }

    nu_bench_run_one(i);
    run_count++;
  }

  if (run_count == 0) {
    printf("  No benchmarks matched filter.\n");
  }

  printf("\nBenchmarks completed.\n");
  return 0;
}

// Main macro
#define NU_BENCH_MAIN() \
        int main(int argc, char** argv) { \
          return nu_bench_run_all(argc, argv); \
        }

#endif // NU_BENCH_H
