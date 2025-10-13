#ifndef NU_SORT_H
#define NU_SORT_H

/**
 * @file sort.h
 * @brief Introsort - A hybrid sorting algorithm with O(n log n) worst-case performance
 *
 * Introsort (introspective sort) is a hybrid sorting algorithm that provides both
 * the fast average performance of quicksort and the optimal worst-case performance
 * of heapsort. The algorithm:
 *
 * 1. Begins with quicksort and monitors the recursion depth
 * 2. Switches to heapsort if the recursion depth exceeds 2*log₂(n) to prevent
 *    quicksort's O(n²) worst-case behavior
 * 3. Uses insertion sort for small subarrays (typically < 16 elements) where
 *    it outperforms both quicksort and heapsort due to lower overhead
 *
 * This three-way hybrid approach guarantees:
 * - O(n log n) worst-case time complexity (from heapsort fallback)
 * - O(log n) space complexity (from limited recursion depth)
 * - Excellent performance on real-world data (from quicksort's average case)
 * - Efficient handling of small datasets (from insertion sort)
 *
 * The implementation is optimized with median-of-three pivot selection to
 * improve performance on already-sorted or reverse-sorted inputs.
 */

#include <stddef.h>
#include <stdlib.h>

/* For internal library builds, NU_MALLOC/NU_FREE are defined by compiler */
/* These are not exposed to end users - sort.c handles allocation internally */
#ifdef NU_MALLOC
extern void* NU_MALLOC(size_t size);
extern void NU_FREE(void* ptr);

#endif

#ifndef NU_QUICKSORT_STACK_SIZE
#define NU_QUICKSORT_STACK_SIZE 64
#endif

/**
 * @brief Sort an array of elements using an optimized introsort algorithm
 *
 * This function sorts an array of nmemb elements of size bytes each.
 * The array is sorted in place using a hybrid introsort algorithm that
 * combines quicksort, heapsort, and insertion sort for optimal performance.
 *
 * @param base Pointer to the first element of the array to sort
 * @param nmemb Number of elements in the array
 * @param size Size of each element in bytes
 * @param compar Comparison function which returns:
 *               - negative if first argument is less than second
 *               - zero if arguments are equal
 *               - positive if first argument is greater than second
 */
void nu_sort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*));

#endif /* NU_SORT_H */
