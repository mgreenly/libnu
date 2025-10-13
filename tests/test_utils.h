#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stddef.h>
#include <stdint.h>

// Test malloc that can simulate failures
void* test_malloc(size_t size);

// Control test malloc behavior
void test_malloc_set_fail_after(int32_t count);
void test_malloc_reset(void);

#endif // TEST_UTILS_H
