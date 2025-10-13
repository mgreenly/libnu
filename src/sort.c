/**
 * @file sort.c
 * @brief Implementation of nu_sort using introsort algorithm
 */

#include "sort.h"
#include <string.h>
#include <stdint.h>

static size_t
floor_log2 (size_t n)
{
  size_t log = 0;
  while (n >>= 1) {
    log++;
  }
  return log;
}

static void
swap_bytes (
  void* a,
  void* b,
  size_t size)
{
  char* pa = (char*) a;
  char* pb = (char*) b;
  char temp;

  for (size_t i = 0; i < size; i++) {
    temp  = pa[i];
    pa[i] = pb[i];
    pb[i] = temp;
  }
}

static void*
get_element (
  void* base,
  size_t index,
  size_t size)
{
  return (char*) base + index * size;
}

static void
insertion_sort (
  void* base,
  size_t low,
  size_t high,
  size_t size,
  int (*compar)(const void*, const void* ))
{
  char* key = NU_MALLOC(size);
  if (!key)
    return;

  for (size_t i = low + 1; i <= high; i++) {
    memcpy(key, get_element(base, i, size), size);

    size_t j = i;
    while (j > low && compar(get_element(base, j - 1, size), key) > 0) {
      memcpy(get_element(base, j, size), get_element(base, j - 1, size), size);
      j--;
    }
    memcpy(get_element(base, j, size), key, size);
  }

  NU_FREE(key);
}

static void
heapify (
  void* base,
  size_t start,
  size_t end,
  size_t size,
  int (*compar)(const void*, const void* ))
{
  size_t root = start;

  while (2 * root + 1 <= end) {
    size_t child    = 2 * root + 1;
    size_t swap_idx = root;

    if (compar(get_element(base, swap_idx, size), get_element(base, child, size)) < 0) {
      swap_idx = child;
    }

    if (child + 1 <= end && compar(get_element(base, swap_idx, size), get_element(base, child + 1, size)) < 0) {
      swap_idx = child + 1;
    }

    if (swap_idx == root) {
      return;
    } else {
      swap_bytes(get_element(base, root, size), get_element(base, swap_idx, size), size);
      root = swap_idx;
    }
  }
}

static void
heapsort (
  void* base,
  size_t low,
  size_t high,
  size_t size,
  int (*compar)(const void*, const void* ))
{
  size_t count    = high - low + 1;
  void* heap_base = get_element(base, low, size);

  for (size_t start = (count - 2) / 2; start != SIZE_MAX; start--) {
    heapify(heap_base, start, count - 1, size, compar);
  }

  for (size_t end = count - 1; end > 0; end--) {
    swap_bytes(get_element(heap_base, 0, size), get_element(heap_base, end, size), size);
    heapify(heap_base, 0, end - 1, size, compar);
  }
}

static size_t
partition (
  void* base,
  size_t low,
  size_t high,
  size_t size,
  int (*compar)(const void*, const void* ))
{
  size_t pivot_idx = low + (high - low) / 2;
  swap_bytes(get_element(base, pivot_idx, size), get_element(base, high, size), size);

  void* pivot      = get_element(base, high, size);
  size_t i         = low;

  for (size_t j = low; j < high; j++) {
    if (compar(get_element(base, j, size), pivot) <= 0) {
      swap_bytes(get_element(base, i, size), get_element(base, j, size), size);
      i++;
    }
  }

  swap_bytes(get_element(base, i, size), get_element(base, high, size), size);
  return i;
}

static void
introsort_impl (
  void* base,
  size_t low,
  size_t high,
  size_t depth_limit,
  size_t size,
  int (*compar)(const void*,
  const void* ))
{
  typedef struct {
    size_t low;
    size_t high;
    size_t depth;
  } frame_t;

  frame_t stack[NU_QUICKSORT_STACK_SIZE];
  int32_t top = 0;

  stack[top++] = (frame_t){low, high, 0};

  while (top > 0) {
    frame_t frame = stack[--top];

    if (frame.low >= frame.high) {
      continue;
    }

    size_t len = frame.high - frame.low + 1;

    if (len < 16) {
      insertion_sort(base, frame.low, frame.high, size, compar);
    } else if (frame.depth >= depth_limit) {
      heapsort(base, frame.low, frame.high, size, compar);
    } else {
      size_t pivot           = partition(base, frame.low, frame.high, size, compar);

      int32_t frames_to_push = 0;
      if (pivot > frame.low)
        frames_to_push++;
      if (pivot < frame.high)
        frames_to_push++;

      if (top + frames_to_push >= NU_QUICKSORT_STACK_SIZE) {
        heapsort(base, frame.low, frame.high, size, compar);
      } else {
        if (pivot > frame.low) {
          stack[top++] = (frame_t){frame.low, pivot - 1, frame.depth + 1};
        }
        if (pivot < frame.high) {
          stack[top++] = (frame_t){pivot + 1, frame.high, frame.depth + 1};
        }
      }
    }
  }
}

void
nu_sort (
  void* base,
  size_t nmemb,
  size_t size,
  int (*compar)(const void*, const void* ))
{
  if (!base || !compar || nmemb <= 1 || size == 0) {
    return;
  }

  size_t depth_limit = 2 * floor_log2(nmemb);
  introsort_impl(base, 0, nmemb - 1, depth_limit, size, compar);
}
