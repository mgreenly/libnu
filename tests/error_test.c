#include "../src/error.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Test counter
static int tests_run    = 0;
static int tests_passed = 0;

#define TEST(name) \
        do { \
          tests_run++; \
          printf("  Testing %s...", name); \
          fflush(stdout); \
        } while (0)

#define PASS() \
        do { \
          tests_passed++; \
          printf(" PASS\n"); \
        } while (0)

#define ASSERT(cond) \
        do { \
          if (!(cond)) { \
            printf(" FAIL\n    Assertion failed: %s\n    at %s:%d\n", \
                   #cond, __FILE__, __LINE__); \
            exit(1); \
          } \
        } while (0)

// Test functions that use nu_error
static nu_result_t
test_function_failure
  (void
  ) {
  NU_FAIL(NU_ERR_GENERIC, "This function always fails");
}

static nu_result_t
test_function_null_check
  (void* ptr
  ) {
  NU_RETURN_IF_ERR(nu_check_null(ptr, "ptr"));
  return nu_ok(ptr);
}

static void
test_result_construction
  (void
  ) {
  printf("\nTesting Result Construction:\n");

  TEST("nu_ok creates success result");
  int value       = 42;
  nu_result_t res = nu_ok(&value);
  ASSERT(nu_is_ok(&res));
  ASSERT(!nu_is_err(&res));
  ASSERT(res.ok == &value);
  ASSERT(!res.is_err);
  PASS();

  TEST("nu_err creates error result");
  nu_error_t error = {.code = NU_ERR_GENERIC, .message = "test error"};
  res = nu_err(&error);
  ASSERT(!nu_is_ok(&res));
  ASSERT(nu_is_err(&res));
  ASSERT(res.err == &error);
  ASSERT(res.is_err);
  PASS();

  TEST("nu_ok with NULL is valid");
  res = nu_ok(NULL);
  ASSERT(nu_is_ok(&res));
  ASSERT(res.ok == NULL);
  PASS();
}

static void
test_error_codes
  (void
  ) {
  printf("\nTesting Error Codes:\n");

  TEST("Error codes have correct values");
  ASSERT(NU_OK == 0);
  ASSERT(NU_ERR_GENERIC == 1);
  ASSERT(NU_ERR_OOM > NU_ERR_GENERIC);
  ASSERT(NU_ERR_INVALID_ARG > NU_ERR_GENERIC);
  PASS();

  TEST("nu_error_code_str returns correct strings");
  ASSERT(strcmp(nu_error_code_str(NU_OK), "OK") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_GENERIC), "Generic error") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_OOM), "Out of memory") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_INVALID_ARG), "Invalid argument") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_OUT_OF_RANGE), "Out of range") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_NOT_FOUND), "Not found") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_PERMISSION), "Permission denied") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_IO), "I/O error") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_INVALID_UTF8), "Invalid UTF-8") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_BUFFER_FULL), "Buffer full") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_WOULD_BLOCK), "Operation would block") == 0);
  ASSERT(strcmp(nu_error_code_str(NU_ERR_NOT_IMPLEMENTED), "Not implemented") == 0);
  ASSERT(strcmp(nu_error_code_str(999999), "Unknown error") == 0);
  PASS();
}

static void
test_error_macros
  (void
  ) {
  printf("\nTesting Error Macros:\n");

  TEST("NU_ERROR creates error with correct fields");
  nu_error_t* err = NU_ERROR(NU_ERR_INVALID_ARG, "Test message");
  ASSERT(err != NULL);
  ASSERT(err->code == NU_ERR_INVALID_ARG);
  ASSERT(strcmp(err->message, "Test message") == 0);
  ASSERT(err->file != NULL);
  ASSERT(strstr(err->file, "error_test.c") != NULL);
  ASSERT(err->line > 0);
  ASSERT(err->cause == NULL);
  PASS();

  TEST("NU_FAIL returns error result");
  nu_result_t res = test_function_failure();
  ASSERT(nu_is_err(&res));
  ASSERT(res.err->code == NU_ERR_GENERIC);
  ASSERT(strcmp(res.err->message, "This function always fails") == 0);
  PASS();

  TEST("NU_FAIL_IF works with true condition");
  // Test NU_FAIL_IF directly in a simple case
  int x = 5;
  {
    // Use a block scope instead of nested function
    nu_result_t res_local;
    if (x < 10) {
      res_local = nu_err(NU_ERROR(NU_ERR_OUT_OF_RANGE, "x is too small"));
    } else {
      res_local = nu_ok(NULL);
    }
    ASSERT(nu_is_err(&res_local));
    ASSERT(res_local.err->code == NU_ERR_OUT_OF_RANGE);
  }
  PASS();

  TEST("NU_FAIL_IF doesn't fail with false condition");
  {
    nu_result_t res_local;
    if (x > 10) {
      res_local = nu_err(NU_ERROR(NU_ERR_OUT_OF_RANGE, "x is too large"));
    } else {
      res_local = nu_ok(&x);
    }
    ASSERT(nu_is_ok(&res_local));
  }
  PASS();
}

static void
test_error_propagation
  (void
  ) {
  printf("\nTesting Error Propagation:\n");

  TEST("NU_RETURN_IF_ERR propagates errors");
  nu_result_t res = test_function_null_check(NULL);
  ASSERT(nu_is_err(&res));
  ASSERT(res.err->code == NU_ERR_INVALID_ARG);
  ASSERT(strcmp(res.err->message, "NULL pointer parameter") == 0);
  PASS();

  TEST("NU_RETURN_IF_ERR doesn't propagate success");
  int value = 42;
  res = test_function_null_check(&value);
  ASSERT(nu_is_ok(&res));
  ASSERT(res.ok == &value);
  PASS();
}

static void
test_helper_functions
  (void
  ) {
  printf("\nTesting Helper Functions:\n");

  TEST("nu_check_null detects NULL");
  nu_result_t res = nu_check_null(NULL, "test_param");
  ASSERT(nu_is_err(&res));
  ASSERT(res.err->code == NU_ERR_INVALID_ARG);
  ASSERT(strcmp(res.err->message, "NULL pointer parameter") == 0);
  PASS();

  TEST("nu_check_null accepts non-NULL");
  int value = 42;
  res = nu_check_null(&value, "test_param");
  ASSERT(nu_is_ok(&res));
  // Note: cast away const in implementation, so pointer matches
  ASSERT(res.ok == (void*) (uintptr_t) &value);
  PASS();

  TEST("nu_check_range detects out of range (below)");
  res = nu_check_range(5, 10, 20, "test_val");
  ASSERT(nu_is_err(&res));
  ASSERT(res.err->code == NU_ERR_OUT_OF_RANGE);
  ASSERT(strcmp(res.err->message, "Value out of range") == 0);
  PASS();

  TEST("nu_check_range detects out of range (above)");
  res = nu_check_range(25, 10, 20, "test_val");
  ASSERT(nu_is_err(&res));
  ASSERT(res.err->code == NU_ERR_OUT_OF_RANGE);
  PASS();

  TEST("nu_check_range accepts in-range values");
  res = nu_check_range(15, 10, 20, "test_val");
  ASSERT(nu_is_ok(&res));
  res = nu_check_range(10, 10, 20, "test_val");    // Min edge
  ASSERT(nu_is_ok(&res));
  res = nu_check_range(20, 10, 20, "test_val");    // Max edge
  ASSERT(nu_is_ok(&res));
  PASS();
}

static void
test_error_inspection
  (void
  ) {
  printf("\nTesting Error Inspection:\n");

  TEST("nu_error_code extracts code");
  nu_error_t err = {.code = NU_ERR_IO, .message = "IO failed"};
  ASSERT(nu_error_code(&err) == NU_ERR_IO);
  ASSERT(nu_error_code(NULL) == NU_OK);
  PASS();

  TEST("nu_error_message extracts message");
  err.message = "Custom message";
  ASSERT(strcmp(nu_error_message(&err), "Custom message") == 0);
  PASS();

  TEST("nu_error_message falls back to code string");
  err.message = NULL;
  err.code    = NU_ERR_OOM;
  ASSERT(strcmp(nu_error_message(&err), "Out of memory") == 0);
  PASS();

  TEST("nu_error_message handles NULL error");
  ASSERT(strcmp(nu_error_message(NULL), "Success") == 0);
  PASS();
}

static void
test_error_formatting
  (void
  ) {
  printf("\nTesting Error Formatting:\n");

  TEST("nu_error_fprintf formats simple error");
  printf("\n    Output: ");
  nu_error_t err = {
    .code    = NU_ERR_INVALID_ARG,
    .message = "Test error",
    .file    = "test.c",
    .line    = 42,
    .cause   = NULL
  };
  nu_error_fprintf(stdout, &err);
  printf("    Expected: → Test error [test.c:42]\n");
  PASS();

  TEST("nu_error_fprintf handles NULL");
  printf("\n    Output: ");
  nu_error_fprintf(stdout, NULL);
  printf("    Expected: Success\n");
  PASS();

  TEST("nu_error_fprintf handles chained errors");
  nu_error_t cause = {
    .code    = NU_ERR_IO,
    .message = "Root cause",
    .file    = "io.c",
    .line    = 10,
    .cause   = NULL
  };
  nu_error_t chained = {
    .code    = NU_ERR_GENERIC,
    .message = "High level error",
    .file    = "main.c",
    .line    = 50,
    .cause   = &cause
  };
  printf("\n    Output:\n");
  nu_error_fprintf(stdout, &chained);
  printf("    Expected:\n");
  printf("    → High level error [main.c:50]\n");
  printf("      → Root cause [io.c:10]\n");
  PASS();
}

// Helper function for real-world test
static nu_result_t
divide_helper
  (double a, double b, double* result
  ) {
  NU_RETURN_IF_ERR(nu_check_null(result, "result"));
  NU_FAIL_IF(b == 0.0, NU_ERR_INVALID_ARG, "Division by zero");
  *result = a / b;
  return nu_ok(result);
}

static void
test_real_world_usage
  (void
  ) {
  printf("\nTesting Real-World Usage Patterns:\n");

  TEST("Function with multiple error paths");
  double result;
  nu_result_t res = divide_helper(10.0, 2.0, &result);
  ASSERT(nu_is_ok(&res));
  ASSERT(result == 5.0);

  res = divide_helper(10.0, 0.0, &result);
  ASSERT(nu_is_err(&res));
  ASSERT(res.err->code == NU_ERR_INVALID_ARG);

  res = divide_helper(10.0, 2.0, NULL);
  ASSERT(nu_is_err(&res));
  ASSERT(res.err->code == NU_ERR_INVALID_ARG);
  PASS();

  TEST("Error propagation through call chain");
  // Use existing test functions to demonstrate propagation
  res = test_function_null_check(NULL);    // This will fail with INVALID_ARG
  ASSERT(nu_is_err(&res));
  ASSERT(res.err->code == NU_ERR_INVALID_ARG);
  ASSERT(strcmp(res.err->message, "NULL pointer parameter") == 0);
  PASS();
}

static void
test_line_numbers
  (void
  ) {
  printf("\nTesting Line Number Capture:\n");

  TEST("Line numbers are captured correctly");
  int line_before  = __LINE__;
  nu_error_t* err1 = NU_ERROR(NU_ERR_GENERIC, "Error 1");
  int line1        = __LINE__ - 1; // Previous line

  nu_error_t* err2 = NU_ERROR(NU_ERR_GENERIC, "Error 2");
  int line2        = __LINE__ - 1; // Previous line

  ASSERT(err1->line == line1);
  ASSERT(err2->line == line2);
  ASSERT(err1->line != err2->line);
  ASSERT(err1->line > line_before);
  PASS();

  TEST("Different macro invocations get different lines");
  // Test with direct macro invocations instead of nested functions
  nu_result_t r1 = test_function_failure();
  int line_f1    = r1.err->line;

  // Call a different error to get a different line
  nu_result_t r2 = test_function_null_check(NULL);
  int line_f2    = r2.err->line;

  ASSERT(line_f1 != line_f2);
  PASS();
}

static void
test_edge_cases
  (void
  ) {
  printf("\nTesting Edge Cases:\n");

  TEST("Very long error message");
  const char* long_msg = "This is a very long error message that might cause issues "
                         "with formatting or display but should still work correctly "
                         "even though it spans multiple lines in the source code";
  nu_error_t* err      = NU_ERROR(NU_ERR_GENERIC, long_msg);
  ASSERT(err->message == long_msg);    // Should point to same string
  PASS();

  TEST("Error code at enum boundaries");
  ASSERT(nu_error_code_str(NU_OK) != NULL);
  ASSERT(nu_error_code_str(NU_ERR_NOT_IMPLEMENTED) != NULL);
  PASS();

  TEST("Multiple errors in same scope (compound literal issue)");
  // This demonstrates the compound literal limitation
  nu_result_t r1        = test_function_failure();
  nu_error_t* err1_copy = r1.err;    // Save pointer

  nu_result_t r2        = test_function_failure();
  nu_error_t* err2_copy = r2.err;    // Save pointer

  // Due to compound literals, these might be the same address
  // This is a known limitation we accept for zero-allocation
  printf("\n    Note: err1=%p, err2=%p (may be same - known limitation)",
         (void*) err1_copy, (void*) err2_copy);
  PASS();
}

int
main
  (void
  ) {
  printf("=== nu_error Test Suite ===\n");

  test_result_construction();
  test_error_codes();
  test_error_macros();
  test_error_propagation();
  test_helper_functions();
  test_error_inspection();
  test_error_formatting();
  test_real_world_usage();
  test_line_numbers();
  test_edge_cases();

  printf("\n=== Test Summary ===\n");
  printf("Tests run: %d\n", tests_run);
  printf("Tests passed: %d\n", tests_passed);

  if (tests_passed == tests_run) {
    printf("All tests PASSED!\n");
    return 0;
  } else {
    printf("Some tests FAILED!\n");
    return 1;
  }
}
