/* Test allocator header for simulating malloc failures */

#ifndef TEST_MALLOC_H
#define TEST_MALLOC_H

#include <stddef.h>

/* Configure allocator to fail after n calls (-1 to disable) */
void test_malloc_set_fail_after(int n);

/* Reset allocator to normal behavior */
void test_malloc_reset(void);

/* Custom malloc implementation */
void* test_malloc(size_t size);

/* Get current call count */
int test_malloc_get_call_count(void);

#endif /* TEST_MALLOC_H */
