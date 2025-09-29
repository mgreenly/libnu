/*
 * Tests for src/version.c
 * Tests version API functions
 */

#include <nu/test.h>
#include <string.h>
#include "../src/version.h"

NU_TEST(test_version_string) {
  const char* version = nu_version();
  NU_ASSERT_NOT_NULL(version);
  NU_ASSERT_STR_EQ(version, "0.1.0");
  // Note: Can't output version string in nu/test framework
  return nu_ok(NULL);
}

NU_TEST(test_version_numbers) {
  NU_ASSERT_EQ(nu_version_major(), 0);
  NU_ASSERT_EQ(nu_version_minor(), 1);
  NU_ASSERT_EQ(nu_version_patch(), 0);
  // Note: Can't output version numbers in nu/test framework
  return nu_ok(NULL);
}

NU_TEST(test_version_macros) {
  NU_ASSERT_EQ(NULIB_VERSION_MAJOR, 0);
  NU_ASSERT_EQ(NULIB_VERSION_MINOR, 1);
  NU_ASSERT_EQ(NULIB_VERSION_PATCH, 0);
  NU_ASSERT_STR_EQ(NULIB_VERSION_STRING, "0.1.0");
  return nu_ok(NULL);
}

// Main test runner
NU_TEST_MAIN()
