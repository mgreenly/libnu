/*
 * nu/test.h - Minimal header-only test framework for libnu
 *
 * This test framework is designed specifically for libnu, dogfooding the
 * nu_error Result types for consistent error handling throughout tests.
 *
 * Features:
 * - Tests return nu_result_t for consistent error handling
 * - Automatic test registration via __attribute__((constructor))
 * - Zero dynamic allocation - all state is static
 * - ~250 lines of simple, readable code
 * - Header-only for easy inclusion
 *
 * Usage:
 *   #include <nu/test.h>  // Includes nu/error.h automatically
 *
 *   NU_TEST(test_something) {
 *     NU_ASSERT_EQ(2 + 2, 4);
 *     return nu_ok(NULL);
 *   }
 *
 *   NU_TEST_MAIN()  // Generates main() that runs all tests
 *
 * Known limitations:
 * - Maximum 512 tests per executable
 * - No parallel execution
 * - No fixtures (use static variables)
 * - Linux/macOS only (uses constructor attribute)
 */

#ifndef NU_TEST_H
#define NU_TEST_H

#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// Test function signature - returns nu_result_t for consistency
typedef nu_result_t (* nu_test_fn)(void);

// Test registration entry
typedef struct {
  const char* name;
  nu_test_fn fn;
  const char* file;
  int32_t line;
} nu_test_entry_t;

// Global test state - no dynamic allocation needed
static struct {
  nu_test_entry_t tests[512];  // Reasonable limit for single test file
  int32_t count;
  int32_t passed;
  int32_t failed;
  bool verbose;
  bool stop_on_fail;
  // Static storage for test errors to avoid compound literal scope issues
  nu_error_t last_error;
} nu_test_state = {0};

// Test registration - called by constructor attribute
static inline void
nu_test_register_impl (
  const char* name,
  nu_test_fn fn,
  const char* file,
  int32_t line)
{
  if (nu_test_state.count >= 512) {
    fprintf(stderr, "ERROR: Too many tests registered (max 512)\n");
    exit(1);
  }
  nu_test_state.tests[nu_test_state.count].name = name;
  nu_test_state.tests[nu_test_state.count].fn   = fn;
  nu_test_state.tests[nu_test_state.count].file = file;
  nu_test_state.tests[nu_test_state.count].line = line;
  nu_test_state.count++;
}

// Test definition macro - creates function and registers it
#define NU_TEST(name) \
        static nu_result_t name(void); \
        __attribute__((constructor(200))) \
        static void nu_test_register_ ## name(void) { \
          nu_test_register_impl(#name, name, __FILE__, __LINE__); \
        } \
        static nu_result_t name(void)

// Helper macro to create persistent test errors
#define NU_TEST_ERROR(errcode, errmsg) \
        (nu_test_state.last_error = (nu_error_t){ \
    .code = (errcode), \
    .message = (errmsg), \
    .file = __FILE__, \
    .line = __LINE__, \
    .cause = NULL \
  }, &nu_test_state.last_error)

// Test-specific failure macro that uses persistent storage
#define NU_TEST_FAIL(code, msg) \
        return nu_err(NU_TEST_ERROR(code, msg))

#define NU_TEST_FAIL_IF(cond, code, msg) \
        do { if (cond) NU_TEST_FAIL(code, msg); } while (0)

// Assertion macros that return nu_result_t
#define NU_ASSERT(cond) \
        NU_TEST_FAIL_IF(!(cond), NU_ERR_GENERIC, "Assertion failed: " #cond)

#define NU_ASSERT_TRUE(cond) \
        NU_TEST_FAIL_IF(!(cond), NU_ERR_GENERIC, "Expected true: " #cond)

#define NU_ASSERT_FALSE(cond) \
        NU_TEST_FAIL_IF((cond), NU_ERR_GENERIC, "Expected false: " #cond)

#define NU_ASSERT_NULL(ptr) \
        NU_TEST_FAIL_IF((ptr) != NULL, NU_ERR_GENERIC, "Expected NULL")

#define NU_ASSERT_NOT_NULL(ptr) \
        NU_TEST_FAIL_IF((ptr) == NULL, NU_ERR_GENERIC, "Expected non-NULL")

// Numeric comparisons - simple version without formatted messages
#define NU_ASSERT_EQ(a, b) \
        NU_TEST_FAIL_IF((a) != (b), NU_ERR_GENERIC, "Values not equal")

#define NU_ASSERT_NE(a, b) \
        NU_TEST_FAIL_IF((a) == (b), NU_ERR_GENERIC, "Values should not be equal")

#define NU_ASSERT_LT(a, b) \
        NU_TEST_FAIL_IF((a) >= (b), NU_ERR_GENERIC, "Expected a < b")

#define NU_ASSERT_LE(a, b) \
        NU_TEST_FAIL_IF((a) > (b), NU_ERR_GENERIC, "Expected a <= b")

#define NU_ASSERT_GT(a, b) \
        NU_TEST_FAIL_IF((a) <= (b), NU_ERR_GENERIC, "Expected a > b")

#define NU_ASSERT_GE(a, b) \
        NU_TEST_FAIL_IF((a) < (b), NU_ERR_GENERIC, "Expected a >= b")

// String comparison
#define NU_ASSERT_STR_EQ(a, b) \
        NU_TEST_FAIL_IF(strcmp((a), (b)) != 0, NU_ERR_GENERIC, "Strings not equal")

#define NU_ASSERT_STR_NE(a, b) \
        NU_TEST_FAIL_IF(strcmp((a), (b)) == 0, NU_ERR_GENERIC, "Strings should not be equal")

// Memory comparison
#define NU_ASSERT_MEM_EQ(a, b, size) \
        NU_TEST_FAIL_IF(memcmp((a), (b), (size)) != 0, NU_ERR_GENERIC, "Memory regions not equal")

// Result checking helpers
#define NU_ASSERT_OK(result) \
        do { \
          nu_result_t _r = (result); \
          if (nu_is_err(&_r)) { \
            return _r; \
          } \
        } while (0)

#define NU_ASSERT_ERR(result) \
        do { \
          nu_result_t _r = (result); \
          NU_TEST_FAIL_IF(nu_is_ok(&_r), NU_ERR_GENERIC, "Expected error but got success"); \
        } while (0)

#define NU_ASSERT_ERR_CODE(result, expected_code) \
        do { \
          nu_result_t _r = (result); \
          NU_TEST_FAIL_IF(nu_is_ok(&_r), NU_ERR_GENERIC, "Expected error but got success"); \
          NU_TEST_FAIL_IF(_r.err->code != (expected_code), NU_ERR_GENERIC, "Wrong error code"); \
        } while (0)

// ANSI color codes for terminal output
#define NU_TEST_GREEN "\033[32m"
#define NU_TEST_RED   "\033[31m"
#define NU_TEST_RESET "\033[0m"

// Test runner - returns exit code
static inline int
nu_test_run_all (void)
{
  printf("Running %d test%s...\n",
    nu_test_state.count,
    nu_test_state.count == 1 ? "" : "s");

  for (int32_t i = 0; i < nu_test_state.count; i++) {
    nu_test_entry_t* test = &nu_test_state.tests[i];
    nu_result_t result    = test->fn();

    if (nu_is_ok(&result)) {
      nu_test_state.passed++;
      printf("  %sPASS%s %s\n", NU_TEST_GREEN, NU_TEST_RESET, test->name);
    } else {
      nu_test_state.failed++;
      printf("  %sFAIL%s %s", NU_TEST_RED, NU_TEST_RESET, test->name);

      // Print error details on same line
      if (result.err) {
        printf(" → %s", nu_error_message(result.err));
        if (result.err->file) {
          printf(" [%s:%d]", result.err->file, result.err->line);
        }

        // Print cause chain if present (indented on next lines)
        nu_error_t* cause = result.err->cause;
        while (cause) {
          printf("\n         → %s", nu_error_message(cause));
          if (cause->file) {
            printf(" [%s:%d]", cause->file, cause->line);
          }
          cause = cause->cause;
        }
      }
      printf("\n");

      if (nu_test_state.stop_on_fail) {
        printf("\nStopping on first failure.\n");
        return 1;
      }
    }
  }

  printf("\n");
  printf("%s%d Passed%s, %s%d Failed%s, %d Total\n",
    NU_TEST_GREEN, nu_test_state.passed, NU_TEST_RESET,
    nu_test_state.failed > 0 ? NU_TEST_RED : NU_TEST_GREEN,
    nu_test_state.failed,
    NU_TEST_RESET,
    nu_test_state.count);
  printf("\n");

  return nu_test_state.failed > 0 ? 1 : 0;
}

// Configuration functions
static inline void
nu_test_set_verbose (bool verbose)
{
  nu_test_state.verbose = verbose;
}

static inline void
nu_test_set_stop_on_fail (bool stop)
{
  nu_test_state.stop_on_fail = stop;
}

// Main macro for test programs
#define NU_TEST_MAIN() \
        int main(int argc, char** argv) { \
          (void)argc; (void)argv; \
          return nu_test_run_all(); \
        }

// Manual test registration for when constructors aren't available
#define NU_TEST_MANUAL(name) \
        static nu_result_t name(void)

#define NU_TEST_REGISTER(name) \
        nu_test_register_impl(#name, name, __FILE__, __LINE__)

// Test suite macro for grouping (optional)
#define NU_TEST_SUITE(suite_name) \
        static const char* nu_test_current_suite = #suite_name; \
        static void __attribute__((constructor(199))) \
        nu_test_suite_ ## suite_name(void) { \
          (void)nu_test_current_suite; \
        }

#endif // NU_TEST_H
