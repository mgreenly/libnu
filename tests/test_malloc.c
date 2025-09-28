/* Test allocator for simulating malloc failures
 * This is a test utility, not a test file itself
 */

#include "test_malloc.h"
#include <stdlib.h>
#include <stdbool.h>

static int malloc_call_count  = 0;
static int fail_after_n_calls = -1;  // -1 means never fail

/* Configure allocator to fail after n calls
 * Pass -1 to disable failures (default behavior)
 * Pass 0 to fail on first call, 1 to fail on second, etc.
 */
void
test_malloc_set_fail_after
(
  int n
) {
  fail_after_n_calls = n;
  malloc_call_count  = 0;   // Reset counter when configuring
}

/* Reset the allocator to normal behavior */
void
test_malloc_reset
(
  void
) {
  fail_after_n_calls = -1;
  malloc_call_count  = 0;
}

/* Custom malloc that can be configured to fail */
void*
test_malloc
(
  size_t size
) {
  if (fail_after_n_calls >= 0 && malloc_call_count++ >= fail_after_n_calls) {
    return NULL;      // Simulate allocation failure
  }
  return malloc(size);
}

/* Get current call count (useful for debugging) */
int
test_malloc_get_call_count
(
  void
) {
  return malloc_call_count;
}
