#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../src/nulib.h"

/* Always include test malloc for all tests */
#include "test_malloc.h"

/* Test utilities */
static int
compare_ints
(
  const void* a,
  const void* b
) {
  int ia = *(const int*) a;
  int ib = *(const int*) b;
  return (ia > ib) - (ia < ib);
}

static int
compare_strings
(
  const void* a,
  const void* b
) {
  const char* const* pa = (const char* const*) a;
  const char* const* pb = (const char* const*) b;
  return strcmp(*pa, *pb);
}

static int
arrays_equal_int
(
  int* a,
  int* b,
  size_t n
) {
  for (size_t i = 0; i < n; i++) {
    if (a[i] != b[i])return 0;
  }
  return 1;
}

static int
is_sorted_int
(
  int* arr,
  size_t n
) {
  for (size_t i = 1; i < n; i++) {
    if (arr[i] < arr[i-1])return 0;
  }
  return 1;
}

/* Basic functionality tests */
static void
test_empty_array
(
  void
) {
  printf("  test_empty_array... ");
  int* arr = NULL;
  nu_sort(arr, 0, sizeof(int), compare_ints);
  printf("PASS\n");
}

static void
test_single_element
(
  void
) {
  printf("  test_single_element... ");
  int arr[] = {42};
  nu_sort(arr, 1, sizeof(int), compare_ints);
  assert(arr[0] == 42);
  printf("PASS\n");
}

static void
test_already_sorted
(
  void
) {
  printf("  test_already_sorted... ");
  int arr[]      = {1, 2, 3, 4, 5};
  int expected[] = {1, 2, 3, 4, 5};
  nu_sort(arr, 5, sizeof(int), compare_ints);
  assert(arrays_equal_int(arr, expected, 5));
  printf("PASS\n");
}

static void
test_reverse_sorted
(
  void
) {
  printf("  test_reverse_sorted... ");
  int arr[]      = {5, 4, 3, 2, 1};
  int expected[] = {1, 2, 3, 4, 5};
  nu_sort(arr, 5, sizeof(int), compare_ints);
  assert(arrays_equal_int(arr, expected, 5));
  printf("PASS\n");
}

static void
test_duplicates
(
  void
) {
  printf("  test_duplicates... ");
  int arr[]      = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
  int expected[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};
  nu_sort(arr, 10, sizeof(int), compare_ints);
  assert(arrays_equal_int(arr, expected, 10));
  printf("PASS\n");
}

static void
test_mixed_signs
(
  void
) {
  printf("  test_mixed_signs... ");
  int arr[]      = {-5, 3, -1, 0, 7, -2};
  int expected[] = {-5, -2, -1, 0, 3, 7};
  nu_sort(arr, 6, sizeof(int), compare_ints);
  assert(arrays_equal_int(arr, expected, 6));
  printf("PASS\n");
}

static void
test_strings
(
  void
) {
  printf("  test_strings... ");
  const char* arr[]      = {"zebra", "apple", "banana", "cherry"};
  const char* expected[] = {"apple", "banana", "cherry", "zebra"};

  nu_sort(arr, 4, sizeof(char*), compare_strings);

  for (int i = 0; i < 4; i++) {
    assert(strcmp(arr[i], expected[i]) == 0);
  }
  printf("PASS\n");
}

static void
test_invalid_params
(
  void
) {
  printf("  test_invalid_params... ");
  int arr[] = {1, 2, 3};

  nu_sort(NULL, 3, sizeof(int), compare_ints);
  nu_sort(arr, 3, sizeof(int), NULL);
  nu_sort(arr, 3, 0, compare_ints);

  assert(arr[0] == 1 && arr[1] == 2 && arr[2] == 3);
  printf("PASS\n");
}

/* Performance tests */
static void
test_large_array
(
  void
) {
  printf("  test_large_array (1000 elements)... ");
  const size_t n = 1000;
  int* arr       = malloc(n * sizeof(int));

  for (size_t i = 0; i < n; i++) {
    arr[i] = (int) (n - i);
  }

  nu_sort(arr, n, sizeof(int), compare_ints);
  assert(is_sorted_int(arr, n));

  for (size_t i = 0; i < n; i++) {
    assert(arr[i] == (int) (i + 1));
  }

  free(arr);
  printf("PASS\n");
}

static void
test_pathological_all_same
(
  void
) {
  printf("  test_pathological_all_same (1000 elements)... ");
  const size_t n = 1000;
  int* arr       = malloc(n * sizeof(int));

  for (size_t i = 0; i < n; i++) {
    arr[i] = 42;
  }

  nu_sort(arr, n, sizeof(int), compare_ints);

  for (size_t i = 0; i < n; i++) {
    assert(arr[i] == 42);
  }

  free(arr);
  printf("PASS\n");
}

/* Stress tests */
static void
stress_test_very_large_array
(
  void
) {
  printf("  stress_test_very_large (100k elements)... ");
  const size_t n = 100000;
  int* arr       = malloc(n * sizeof(int));

  srand(42);
  for (size_t i = 0; i < n; i++) {
    arr[i] = rand() % 10000;
  }

  clock_t start = clock();
  nu_sort(arr, n, sizeof(int), compare_ints);
  clock_t end   = clock();

  assert(is_sorted_int(arr, n));

  double cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("PASS (%.3f seconds)\n", cpu_time);

  free(arr);
}

static void
stress_test_worst_case_sorted
(
  void
) {
  printf("  stress_test_worst_case_sorted (50k elements)... ");
  const size_t n = 50000;
  int* arr       = malloc(n * sizeof(int));

  for (size_t i = 0; i < n; i++) {
    arr[i] = (int) i;
  }

  clock_t start = clock();
  nu_sort(arr, n, sizeof(int), compare_ints);
  clock_t end   = clock();

  assert(is_sorted_int(arr, n));

  double cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("PASS (%.3f seconds)\n", cpu_time);

  free(arr);
}

static void
stress_test_worst_case_reverse
(
  void
) {
  printf("  stress_test_worst_case_reverse (50k elements)... ");
  const size_t n = 50000;
  int* arr       = malloc(n * sizeof(int));

  for (size_t i = 0; i < n; i++) {
    arr[i] = (int) (n - i);
  }

  clock_t start = clock();
  nu_sort(arr, n, sizeof(int), compare_ints);
  clock_t end   = clock();

  assert(is_sorted_int(arr, n));

  double cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("PASS (%.3f seconds)\n", cpu_time);

  free(arr);
}

static void
stress_test_many_duplicates
(
  void
) {
  printf("  stress_test_many_duplicates (50k elements, 10 unique)... ");
  const size_t n = 50000;
  int* arr       = malloc(n * sizeof(int));

  srand(42);
  for (size_t i = 0; i < n; i++) {
    arr[i] = rand() % 10;
  }

  clock_t start = clock();
  nu_sort(arr, n, sizeof(int), compare_ints);
  clock_t end   = clock();

  assert(is_sorted_int(arr, n));

  double cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("PASS (%.3f seconds)\n", cpu_time);

  free(arr);
}

// Test that triggers stack overflow protection (with QUICKSORT_STACK_SIZE=8)
static void
test_stack_overflow_protection
(
  void
) {
  printf("  test_stack_overflow_protection... ");

  // With stack size 8, this will trigger overflow protection
  const size_t n = 100;
  int* arr       = malloc(n * sizeof(int));
  for (size_t i = 0; i < n; i++) {
    arr[i] = (int) (n - i);     // Reverse sorted
  }
  nu_sort(arr, n, sizeof(int), compare_ints);
  assert(is_sorted_int(arr, n));
  free(arr);
  printf("PASS\n");
}

// Test depth limit protection
static void
test_depth_limit_protection
(
  void
) {
  printf("  test_depth_limit_protection... ");

  // Create array that will hit depth limit
  // For n=65536, depth_limit = 2 * 16 = 32
  const size_t n = 65536;
  int* arr       = malloc(n * sizeof(int));

  // Create pattern that causes deep recursion
  for (size_t i = 0; i < n; i++) {
    arr[i] = (int) i;
  }
  // Shuffle to create worst-case partitioning
  for (size_t i = 0; i < n - 1; i += 2) {
    int temp = arr[i];
    arr[i]     = arr[i + 1];
    arr[i + 1] = temp;
  }

  nu_sort(arr, n, sizeof(int), compare_ints);
  assert(is_sorted_int(arr, n));

  printf("PASS\n");
  free(arr);
}

// Test invalid parameters for coverage
static void
test_invalid_params_coverage
(
  void
) {
  printf("  test_invalid_params_coverage... ");

  int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  // Test null base
  nu_sort(NULL, 10, sizeof(int), compare_ints);

  // Test null comparator
  nu_sort(arr, 10, sizeof(int), NULL);

  // Test zero size
  nu_sort(arr, 10, 0, compare_ints);

  // Test size 1 array
  nu_sort(arr, 1, sizeof(int), compare_ints);

  // Test size 0 array
  nu_sort(arr, 0, sizeof(int), compare_ints);

  // Verify array unchanged (since all calls were invalid or no-ops)
  for (int i = 0; i < 10; i++) {
    assert(arr[i] == i + 1);
  }

  printf("PASS\n");
}

// Test malloc failure in insertion_sort
static void
test_malloc_failure
(
  void
) {
  printf("  test_malloc_failure... ");

  // Create array that will trigger insertion_sort (needs < 16 elements)
  int arr[] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
  int original[15];
  memcpy(original, arr, sizeof(arr));

  // Configure allocator to fail on first malloc call
  test_malloc_set_fail_after(0);

  // This should trigger insertion_sort which will fail at malloc
  nu_sort(arr, 15, sizeof(int), compare_ints);

  // Reset allocator to normal
  test_malloc_reset();

  // Array should be unchanged since insertion_sort returned early
  assert(arrays_equal_int(arr, original, 15));

  printf("PASS\n");
}

int
main
(
  void
) {
  printf("Running nu_sort tests...\n");

  /* Also test version functions to ensure coverage */
  printf("Library version: %s (%d.%d.%d)\n",
         nu_version(),
         nu_version_major(),
         nu_version_minor(),
         nu_version_patch());

  printf("\nBasic tests:\n");
  test_empty_array();
  test_single_element();
  test_already_sorted();
  test_reverse_sorted();
  test_duplicates();
  test_mixed_signs();
  test_strings();
  test_invalid_params();

  printf("\nPerformance tests:\n");
  test_large_array();
  test_pathological_all_same();

  printf("\nStress tests:\n");
  stress_test_very_large_array();
  stress_test_worst_case_sorted();
  stress_test_worst_case_reverse();
  stress_test_many_duplicates();

  printf("\nEdge case tests:\n");
  test_stack_overflow_protection();
  test_depth_limit_protection();
  test_invalid_params_coverage();
  test_malloc_failure();

  printf("\nAll sort tests passed!\n");
  return 0;
}
