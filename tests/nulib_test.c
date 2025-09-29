/*
 * Tests for src/nulib.c
 * Tests core library functions including version API
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/version.h"

static void
test_version_string
  ( void
  ) {
  printf("  test_version_string... ");
  const char* version = nu_version();
  assert(version != NULL);
  assert(strcmp(version, "0.1.0") == 0);
  printf("PASS (version: %s)\n", version);
}

static void
test_version_numbers
  ( void
  ) {
  printf("  test_version_numbers... ");
  assert(nu_version_major() == 0);
  assert(nu_version_minor() == 1);
  assert(nu_version_patch() == 0);
  printf("PASS (major=%d, minor=%d, patch=%d)\n",
         nu_version_major(),
         nu_version_minor(),
         nu_version_patch());
}

static void
test_version_macros
  ( void
  ) {
  printf("  test_version_macros... ");
  assert(NULIB_VERSION_MAJOR == 0);
  assert(NULIB_VERSION_MINOR == 1);
  assert(NULIB_VERSION_PATCH == 0);
  assert(strcmp(NULIB_VERSION_STRING, "0.1.0") == 0);
  printf("PASS\n");
}

int
main
  ( void
  ) {
  printf("Running nulib version tests...\n");

  test_version_string();
  test_version_numbers();
  test_version_macros();

  printf("\nAll version tests passed!\n");
  return 0;
}
