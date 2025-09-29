#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bench_utils.h"
#include "sort.h"

static int compare_ints(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

static void shuffle_array(int *arr, size_t n) {
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = (size_t)rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

int main(void) {
    bench_timer_t timer;
    srand(42); // Fixed seed for reproducibility

    printf("Benchmarking nu_sort with different array sizes:\n");
    bench_print_header();

    size_t sizes[] = {100, 1000, 10000, 100000, 1000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    for (int s = 0; s < num_sizes; s++) {
        size_t n = sizes[s];
        int *arr = malloc(n * sizeof(int));
        if (!arr) {
            fprintf(stderr, "Failed to allocate array of size %zu\n", n);
            continue;
        }

        // Initialize with sorted data then shuffle
        for (size_t i = 0; i < n; i++) {
            arr[i] = (int)i;
        }
        shuffle_array(arr, n);

        // Benchmark the sort
        bench_start(&timer);
        nu_sort(arr, n, sizeof(int), compare_ints);
        bench_end(&timer);

        // Verify it's sorted
        for (size_t i = 1; i < n; i++) {
            if (arr[i] < arr[i-1]) {
                fprintf(stderr, "Array not sorted correctly!\n");
                break;
            }
        }

        bench_print_result("nu_sort", n, 1, bench_elapsed_ns(&timer));
        free(arr);
    }

    printf("\nBenchmarking different input patterns (n=10000):\n");
    bench_print_header();

    size_t n = 10000;
    int *arr = malloc(n * sizeof(int));
    if (arr) {
        const char *patterns[] = {"random", "sorted", "reverse", "many_dups"};

        for (int p = 0; p < 4; p++) {
            // Initialize based on pattern
            switch(p) {
                case 0: // Random
                    for (size_t i = 0; i < n; i++) arr[i] = rand() % (int)n;
                    break;
                case 1: // Already sorted
                    for (size_t i = 0; i < n; i++) arr[i] = (int)i;
                    break;
                case 2: // Reverse sorted
                    for (size_t i = 0; i < n; i++) arr[i] = (int)(n - i);
                    break;
                case 3: // Many duplicates
                    for (size_t i = 0; i < n; i++) arr[i] = rand() % 10;
                    break;
            }

            bench_start(&timer);
            nu_sort(arr, n, sizeof(int), compare_ints);
            bench_end(&timer);

            char label[32];
            snprintf(label, sizeof(label), "sort_%s", patterns[p]);
            bench_print_result(label, n, 1, bench_elapsed_ns(&timer));
        }

        free(arr);
    }

    return 0;
}