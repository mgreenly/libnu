/* Test suite for arena module using nu test framework */

/* Include test framework directly */
#include "../src/error.h"
#include "../src/test.h"

/* Standard headers */
#include <string.h>
#include <stdint.h>

/* Test utilities - include implementation directly */
#include "test_utils.c"

/* NU_MALLOC will be defined by the compiler for test builds (-DNU_MALLOC=test_malloc)
 * Arena.c will use it if defined
 */
extern void* NU_MALLOC(size_t size);
extern void NU_FREE(void* ptr);

/* Module under test */
#include "../src/arena.h"

NU_TEST(test_arena_init) {
  nu_arena arena;
  char buffer[1024];

  /* Test valid initialization */
  NU_ASSERT(nu_arena_init(&arena, buffer, sizeof(buffer)));
  NU_ASSERT_EQ(nu_arena_used(&arena), 0u);
  NU_ASSERT_EQ(nu_arena_available(&arena), sizeof(buffer));

  /* Test invalid parameters */
  NU_ASSERT(!nu_arena_init(NULL, buffer, sizeof(buffer)));
  NU_ASSERT(!nu_arena_init(&arena, NULL, sizeof(buffer)));
  NU_ASSERT(!nu_arena_init(&arena, buffer, 0));

  return nu_ok(NULL);
}

NU_TEST(test_arena_alloc) {
  nu_arena arena;
  char buffer[1024];

  NU_ASSERT(nu_arena_init(&arena, buffer, sizeof(buffer)));

  /* Test basic allocation */
  void* p1 = nu_arena_alloc(&arena, 100);
  NU_ASSERT(p1 != NULL);
  NU_ASSERT(p1 == buffer);
  NU_ASSERT_EQ(nu_arena_used(&arena), 100u);

  /* Test multiple allocations */
  void* p2 = nu_arena_alloc(&arena, 200);
  NU_ASSERT(p2 != NULL);
  NU_ASSERT(p2 == buffer + 100);
  NU_ASSERT_EQ(nu_arena_used(&arena), 300u);

  /* Test zero size allocation */
  NU_ASSERT(nu_arena_alloc(&arena, 0) == NULL);

  /* Test allocation from NULL arena */
  NU_ASSERT(nu_arena_alloc(NULL, 100) == NULL);

  /* Test allocation that exceeds capacity */
  NU_ASSERT(nu_arena_alloc(&arena, 1000) == NULL);
  NU_ASSERT_EQ(nu_arena_used(&arena), 300u);   /* Should not change */

  return nu_ok(NULL);
}

NU_TEST(test_arena_alloc_aligned) {
  nu_arena arena;
  char buffer[1024];

  NU_ASSERT(nu_arena_init(&arena, buffer, sizeof(buffer)));

  /* Test 8-byte alignment */
  void* p1 = nu_arena_alloc(&arena, 5);
  NU_ASSERT(p1 != NULL);

  void* p2 = nu_arena_alloc_aligned(&arena, 10, 8);
  NU_ASSERT(p2 != NULL);
  NU_ASSERT(((uintptr_t)p2 & 7) == 0);   /* Check 8-byte alignment */

  /* Test 16-byte alignment */
  void* p3 = nu_arena_alloc_aligned(&arena, 20, 16);
  NU_ASSERT(p3 != NULL);
  NU_ASSERT(((uintptr_t)p3 & 15) == 0);   /* Check 16-byte alignment */

  /* Test invalid alignment (not power of 2) */
  NU_ASSERT(nu_arena_alloc_aligned(&arena, 10, 7) == NULL);
  NU_ASSERT(nu_arena_alloc_aligned(&arena, 10, 0) == NULL);

  /* Test NULL arena */
  NU_ASSERT(nu_arena_alloc_aligned(NULL, 10, 8) == NULL);

  /* Test alignment causing capacity overflow */
  nu_arena arena2;
  char small_buffer[32];
  NU_ASSERT(nu_arena_init(&arena2, small_buffer, sizeof(small_buffer)));

  /* Allocate most of the buffer */
  void* p4 = nu_arena_alloc(&arena2, 30);
  NU_ASSERT(p4 != NULL);

  /* Try to allocate aligned memory that would exceed capacity due to alignment padding */
  void* p5 = nu_arena_alloc_aligned(&arena2, 8, 16);
  NU_ASSERT(p5 == NULL);   /* Should fail because alignment + size exceeds capacity */

  return nu_ok(NULL);
}

NU_TEST(test_arena_mark_restore) {
  nu_arena arena;
  char buffer[1024];

  NU_ASSERT(nu_arena_init(&arena, buffer, sizeof(buffer)));

  /* Allocate some memory */
  void* p1 = nu_arena_alloc(&arena, 100);
  NU_ASSERT(p1 != NULL);
  NU_ASSERT_EQ(nu_arena_used(&arena), 100u);

  /* Mark current position */
  nu_arena_mark mark1 = nu_arena_get_mark(&arena);
  NU_ASSERT_EQ(mark1.position, 100u);

  /* Allocate more memory */
  void* p2 = nu_arena_alloc(&arena, 200);
  NU_ASSERT(p2 != NULL);
  NU_ASSERT_EQ(nu_arena_used(&arena), 300u);

  /* Mark new position */
  nu_arena_mark mark2 = nu_arena_get_mark(&arena);
  NU_ASSERT_EQ(mark2.position, 300u);

  /* Allocate even more */
  void* p3 = nu_arena_alloc(&arena, 150);
  NU_ASSERT(p3 != NULL);
  NU_ASSERT_EQ(nu_arena_used(&arena), 450u);

  /* Restore to mark2 */
  nu_arena_restore(&arena, mark2);
  NU_ASSERT_EQ(nu_arena_used(&arena), 300u);

  /* Can allocate again from restored position */
  void* p4 = nu_arena_alloc(&arena, 50);
  NU_ASSERT(p4 != NULL);
  NU_ASSERT(p4 == buffer + 300);
  NU_ASSERT_EQ(nu_arena_used(&arena), 350u);

  /* Restore to mark1 */
  nu_arena_restore(&arena, mark1);
  NU_ASSERT_EQ(nu_arena_used(&arena), 100u);

  /* Test mark with NULL arena */
  nu_arena_mark mark_null = nu_arena_get_mark(NULL);
  NU_ASSERT_EQ(mark_null.position, 0u);

  /* Test restore with NULL arena (should not crash) */
  nu_arena_restore(NULL, mark1);

  /* Test restore with invalid mark (beyond capacity) */
  nu_arena_mark bad_mark = {.position = 2000};
  size_t before          = nu_arena_used(&arena);
  nu_arena_restore(&arena, bad_mark);
  NU_ASSERT_EQ(nu_arena_used(&arena), before);   /* Should not change */

  return nu_ok(NULL);
}

NU_TEST(test_arena_reset) {
  nu_arena arena;
  char buffer[1024];

  NU_ASSERT(nu_arena_init(&arena, buffer, sizeof(buffer)));

  /* Allocate memory */
  void* p1 = nu_arena_alloc(&arena, 100);
  NU_ASSERT(p1 != NULL);
  void* p2 = nu_arena_alloc(&arena, 200);
  NU_ASSERT(p2 != NULL);
  NU_ASSERT_EQ(nu_arena_used(&arena), 300u);

  /* Reset arena */
  nu_arena_reset(&arena);
  NU_ASSERT_EQ(nu_arena_used(&arena), 0u);
  NU_ASSERT_EQ(nu_arena_available(&arena), sizeof(buffer));

  /* Can allocate from beginning again */
  void* p3 = nu_arena_alloc(&arena, 50);
  NU_ASSERT(p3 != NULL);
  NU_ASSERT(p3 == buffer);
  NU_ASSERT_EQ(nu_arena_used(&arena), 50u);

  /* Test reset with NULL (should not crash) */
  nu_arena_reset(NULL);

  return nu_ok(NULL);
}

NU_TEST(test_arena_queries) {
  nu_arena arena;
  char buffer[1024];

  NU_ASSERT(nu_arena_init(&arena, buffer, sizeof(buffer)));

  /* Initial state */
  NU_ASSERT_EQ(nu_arena_used(&arena), 0u);
  NU_ASSERT_EQ(nu_arena_available(&arena), 1024u);

  /* After allocation */
  nu_arena_alloc(&arena, 256);
  NU_ASSERT_EQ(nu_arena_used(&arena), 256u);
  NU_ASSERT_EQ(nu_arena_available(&arena), 768u);

  /* Test with NULL */
  NU_ASSERT_EQ(nu_arena_used(NULL), 0u);
  NU_ASSERT_EQ(nu_arena_available(NULL), 0u);

  return nu_ok(NULL);
}

NU_TEST(test_arena_pattern) {
  nu_arena arena;
  char buffer[1024];

  NU_ASSERT(nu_arena_init(&arena, buffer, sizeof(buffer)));

  /* Simulate a typical usage pattern */
  nu_arena_mark frame_start = nu_arena_get_mark(&arena);

  /* Allocate temporary data for processing */
  int* nums = (int* )nu_arena_alloc_aligned(&arena, sizeof(int) * 10, sizeof(int));
  NU_ASSERT(nums != NULL);

  for (int32_t i = 0; i < 10; i++) {
    nums[i] = i * i;
  }

  char* str = (char* )nu_arena_alloc(&arena, 50);
  NU_ASSERT(str != NULL);
  strcpy(str, "temporary string");

  /* Do some work with allocated memory */
  int32_t sum = 0;
  for (int32_t i = 0; i < 10; i++) {
    sum += nums[i];
  }
  NU_ASSERT_EQ(sum, 285);
  NU_ASSERT_STR_EQ(str, "temporary string");

  /* Clean up frame */
  nu_arena_restore(&arena, frame_start);
  NU_ASSERT_EQ(nu_arena_used(&arena), 0u);

  return nu_ok(NULL);
}

NU_TEST(test_arena_malloc_failure) {
  /* Test that arena doesn't use malloc - it uses provided buffer */
  nu_arena arena;
  char buffer[256];

  /* Arena should work even if malloc would fail - arena uses provided buffer */
  test_malloc_set_fail_after(0);   /* Make malloc fail immediately */

  NU_ASSERT(nu_arena_init(&arena, buffer, sizeof(buffer)));

  void* p = nu_arena_alloc(&arena, 100);
  NU_ASSERT(p != NULL);
  NU_ASSERT(p == buffer);

  test_malloc_reset();

  return nu_ok(NULL);
}

/* Generate main function that runs all tests */
NU_TEST_MAIN()
