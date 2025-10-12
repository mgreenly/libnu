/* Test suite for sort module using nu test framework */

/* Include test framework directly */
#include "../src/error.h"
#include "../src/test.h"

/* Standard headers */
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Test utilities - include implementation directly */
#include "test_utils.c"

/* NU_MALLOC will be defined by the compiler for coverage tests (-DNU_MALLOC=test_malloc)
 * Sort.c will use it if defined, otherwise defaults to malloc
 */
extern void* NU_MALLOC(size_t size);
extern void NU_FREE(void* ptr);

/* Module under test */
#include "../src/sort.h"

/* Test utilities */
static int
compare_ints (
  const void* a,
  const void* b)
{
  int ia = *(const int*)a;
  int ib = *(const int*)b;
  return (ia > ib) - (ia < ib);
}

static int
compare_strings (
  const void* a,
  const void* b)
{
  const char* const* pa = (const char* const*)a;
  const char* const* pb = (const char* const*)b;
  return strcmp(*pa, *pb);
}

static int
arrays_equal_int (
  int* a,
  int* b,
  size_t n)
{
  for (size_t i = 0; i < n; i++) {
    if (a[i] != b[i])return 0;
  }
  return 1;
}

static int
is_sorted_int (
  int* arr,
  size_t n)
{
  for (size_t i = 1; i < n; i++) {
    if (arr[i] < arr[i-1])return 0;
  }
  return 1;
}

/* Basic functionality tests */
NU_TEST(test_empty_array) {
  int* arr = NULL;
  nu_sort(arr, 0, sizeof(int), compare_ints);
  return nu_ok(NULL);
}

NU_TEST(test_single_element) {
  int arr[] = {42};
  nu_sort(arr, 1, sizeof(int), compare_ints);
  NU_ASSERT_EQ(arr[0], 42);
  return nu_ok(NULL);
}

NU_TEST(test_already_sorted) {
  int arr[]      = {1, 2, 3, 4, 5};
  int expected[] = {1, 2, 3, 4, 5};
  nu_sort(arr, 5, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(arrays_equal_int(arr, expected, 5));
  return nu_ok(NULL);
}

NU_TEST(test_reverse_sorted) {
  int arr[]      = {5, 4, 3, 2, 1};
  int expected[] = {1, 2, 3, 4, 5};
  nu_sort(arr, 5, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(arrays_equal_int(arr, expected, 5));
  return nu_ok(NULL);
}

NU_TEST(test_duplicates) {
  int arr[]      = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
  int expected[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};
  nu_sort(arr, 10, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(arrays_equal_int(arr, expected, 10));
  return nu_ok(NULL);
}

NU_TEST(test_mixed_signs) {
  int arr[]      = {-5, 3, -1, 0, 7, -2};
  int expected[] = {-5, -2, -1, 0, 3, 7};
  nu_sort(arr, 6, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(arrays_equal_int(arr, expected, 6));
  return nu_ok(NULL);
}

NU_TEST(test_strings) {
  const char* arr[]      = {"zebra", "apple", "banana", "cherry"};
  const char* expected[] = {"apple", "banana", "cherry", "zebra"};

  nu_sort(arr, 4, sizeof(char*), compare_strings);

  for (int i = 0; i < 4; i++) {
    NU_ASSERT_STR_EQ(arr[i], expected[i]);
  }
  return nu_ok(NULL);
}

NU_TEST(test_invalid_params) {
  int arr[] = {1, 2, 3};

  nu_sort(NULL, 3, sizeof(int), compare_ints);
  nu_sort(arr, 3, sizeof(int), NULL);
  nu_sort(arr, 3, 0, compare_ints);

  NU_ASSERT_TRUE(arr[0] == 1 && arr[1] == 2 && arr[2] == 3);
  return nu_ok(NULL);
}

/* Performance tests */
NU_TEST(test_large_array) {
  const size_t n = 1000;
  int* arr       = NU_MALLOC(n * sizeof(int));
  NU_ASSERT_NOT_NULL(arr);

  for (size_t i = 0; i < n; i++) {
    arr[i] = (int)(n - i);
  }

  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(is_sorted_int(arr, n));

  for (size_t i = 0; i < n; i++) {
    NU_ASSERT_EQ(arr[i], (int)(i + 1));
  }

  NU_FREE(arr);
  return nu_ok(NULL);
}

NU_TEST(test_pathological_all_same) {
  const size_t n = 1000;
  int* arr       = NU_MALLOC(n * sizeof(int));
  NU_ASSERT_NOT_NULL(arr);

  for (size_t i = 0; i < n; i++) {
    arr[i] = 42;
  }

  nu_sort(arr, n, sizeof(int), compare_ints);

  for (size_t i = 0; i < n; i++) {
    NU_ASSERT_EQ(arr[i], 42);
  }

  NU_FREE(arr);
  return nu_ok(NULL);
}

/* Edge case tests */
NU_TEST(test_very_large_array) {
  const size_t n = 100000;
  int* arr       = NU_MALLOC(n * sizeof(int));
  NU_ASSERT_NOT_NULL(arr);

  srand(42);
  for (size_t i = 0; i < n; i++) {
    arr[i] = rand() % 10000;
  }

  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(is_sorted_int(arr, n));

  NU_FREE(arr);
  return nu_ok(NULL);
}

NU_TEST(test_already_sorted_large) {
  const size_t n = 50000;
  int* arr       = NU_MALLOC(n * sizeof(int));
  NU_ASSERT_NOT_NULL(arr);

  for (size_t i = 0; i < n; i++) {
    arr[i] = (int)i;
  }

  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(is_sorted_int(arr, n));

  NU_FREE(arr);
  return nu_ok(NULL);
}

NU_TEST(test_reverse_sorted_large) {
  const size_t n = 50000;
  int* arr       = NU_MALLOC(n * sizeof(int));
  NU_ASSERT_NOT_NULL(arr);

  for (size_t i = 0; i < n; i++) {
    arr[i] = (int)(n - i);
  }

  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(is_sorted_int(arr, n));

  NU_FREE(arr);
  return nu_ok(NULL);
}

NU_TEST(test_many_duplicates_large) {
  const size_t n = 50000;
  int* arr       = NU_MALLOC(n * sizeof(int));
  NU_ASSERT_NOT_NULL(arr);

  srand(42);
  for (size_t i = 0; i < n; i++) {
    arr[i] = rand() % 10;  // Only 10 unique values
  }

  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(is_sorted_int(arr, n));

  NU_FREE(arr);
  return nu_ok(NULL);
}

// Test that triggers stack overflow protection (with QUICKSORT_STACK_SIZE=8)
NU_TEST(test_stack_overflow_protection) {
  // With stack size 8, this will trigger overflow protection
  const size_t n = 100;
  int* arr       = NU_MALLOC(n * sizeof(int));
  NU_ASSERT_NOT_NULL(arr);
  for (size_t i = 0; i < n; i++) {
    arr[i] = (int)(n - i);       // Reverse sorted
  }
  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(is_sorted_int(arr, n));
  NU_FREE(arr);
  return nu_ok(NULL);
}

// Test depth limit protection
NU_TEST(test_depth_limit_protection) {
  // Create array that will hit depth limit
  // For n=65536, depth_limit = 2 * 16 = 32
  const size_t n = 65536;
  int* arr       = NU_MALLOC(n * sizeof(int));
  NU_ASSERT_NOT_NULL(arr);

  // Create pattern that causes deep recursion
  for (size_t i = 0; i < n; i++) {
    arr[i] = (int)i;
  }
  // Shuffle to create worst-case partitioning
  for (size_t i = 0; i < n - 1; i += 2) {
    int temp = arr[i];
    arr[i]     = arr[i + 1];
    arr[i + 1] = temp;
  }

  nu_sort(arr, n, sizeof(int), compare_ints);
  NU_ASSERT_TRUE(is_sorted_int(arr, n));

  NU_FREE(arr);
  return nu_ok(NULL);
}

// Test invalid parameters for coverage
NU_TEST(test_invalid_params_coverage) {
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
    NU_ASSERT_EQ(arr[i], i + 1);
  }

  return nu_ok(NULL);
}

// Test malloc failure in insertion_sort
NU_TEST(test_malloc_failure) {
#if !defined(__SANITIZE_ADDRESS__) && !defined(__has_feature)
  // Only run this test when NOT using AddressSanitizer
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
  NU_ASSERT_TRUE(arrays_equal_int(arr, original, 15));
#elif defined(__has_feature)
#if !__has_feature(address_sanitizer)
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
  NU_ASSERT_TRUE(arrays_equal_int(arr, original, 15));
#endif
#endif

  return nu_ok(NULL);
}

// Main test runner
NU_TEST_MAIN()
