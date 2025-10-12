#include "test_utils.h"
#include <stdlib.h>
#include <stdbool.h>

static int malloc_fail_after = -1;
static int malloc_call_count = 0;
static bool malloc_enabled   = true;

void*
test_malloc (size_t size)
{
  if (!malloc_enabled) {
    return NULL;
  }

  if (malloc_fail_after >= 0) {
    if (malloc_call_count >= malloc_fail_after) {
      malloc_call_count++;
      return NULL;
    }
    malloc_call_count++;
  }

  return malloc(size);
}

void
test_malloc_set_fail_after (int count)
{
  malloc_fail_after = count;
  malloc_call_count = 0;
}

void
test_malloc_reset (void)
{
  malloc_fail_after = -1;
  malloc_call_count = 0;
  malloc_enabled    = true;
}
