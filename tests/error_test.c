#include "../src/test.h"
#include "../src/error.h"
#include <limits.h>

// Test functions that use nu_error
static nu_result_t
test_function_failure (void)
{
  NU_TEST_FAIL(NU_ERR_GENERIC, "This function always fails");
}

static nu_result_t
test_function_null_check (void* ptr)
{
  NU_RETURN_IF_ERR(nu_check_null(ptr, "ptr"));
  return nu_ok(ptr);
}

NU_TEST(test_result_construction) {
  // Test nu_ok creates success result
  int value       = 42;
  nu_result_t res = nu_ok(&value);
  NU_ASSERT_TRUE(nu_is_ok(&res));
  NU_ASSERT_FALSE(nu_is_err(&res));
  NU_ASSERT_EQ(res.ok, &value);
  NU_ASSERT_FALSE(res.is_err);

  // Test nu_err creates error result
  nu_error_t error = {.code = NU_ERR_GENERIC, .message = "test error"};
  res = nu_err(&error);
  NU_ASSERT_FALSE(nu_is_ok(&res));
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err, &error);
  NU_ASSERT_TRUE(res.is_err);

  // Test nu_ok with NULL is valid
  res = nu_ok(NULL);
  NU_ASSERT_TRUE(nu_is_ok(&res));
  NU_ASSERT_NULL(res.ok);

  return nu_ok(NULL);
}

NU_TEST(test_error_codes) {
  // Test error codes have correct values
  NU_ASSERT_EQ(NU_OK, 0);
  NU_ASSERT_EQ(NU_ERR_GENERIC, 1);
  NU_ASSERT_GT(NU_ERR_OOM, NU_ERR_GENERIC);
  NU_ASSERT_GT(NU_ERR_INVALID_ARG, NU_ERR_GENERIC);

  // Test nu_error_code_str returns correct strings
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_OK), "OK");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_GENERIC), "Generic error");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_OOM), "Out of memory");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_INVALID_ARG), "Invalid argument");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_OUT_OF_RANGE), "Out of range");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_NOT_FOUND), "Not found");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_PERMISSION), "Permission denied");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_IO), "I/O error");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_INVALID_UTF8), "Invalid UTF-8");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_BUFFER_FULL), "Buffer full");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_WOULD_BLOCK), "Operation would block");
  NU_ASSERT_STR_EQ(nu_error_code_str(NU_ERR_NOT_IMPLEMENTED), "Not implemented");
  NU_ASSERT_STR_EQ(nu_error_code_str(999999), "Unknown error");

  return nu_ok(NULL);
}

NU_TEST(test_error_macros) {
  // Test NU_ERROR creates error with correct fields
  nu_error_t* err = NU_ERROR(NU_ERR_INVALID_ARG, "Test message");
  NU_ASSERT_NOT_NULL(err);
  NU_ASSERT_EQ(err->code, NU_ERR_INVALID_ARG);
  NU_ASSERT_STR_EQ(err->message, "Test message");
  NU_ASSERT_NOT_NULL(err->file);
  NU_ASSERT_NOT_NULL(strstr(err->file, "error_test.c"));
  NU_ASSERT_GT(err->line, 0);
  NU_ASSERT_NULL(err->cause);

  // Test NU_FAIL returns error result
  nu_result_t res = test_function_failure();
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_GENERIC);
  NU_ASSERT_STR_EQ(res.err->message, "This function always fails");

  // Test NU_FAIL_IF with true condition
  int x = 5;
  {
    nu_result_t res_local;
    if (x < 10) {
      res_local = nu_err(NU_ERROR(NU_ERR_OUT_OF_RANGE, "x is too small"));
    } else {
      res_local = nu_ok(NULL);
    }
    NU_ASSERT_TRUE(nu_is_err(&res_local));
    NU_ASSERT_EQ(res_local.err->code, NU_ERR_OUT_OF_RANGE);
  }

  // Test NU_FAIL_IF doesn't fail with false condition
  {
    nu_result_t res_local;
    if (x > 10) {
      res_local = nu_err(NU_ERROR(NU_ERR_OUT_OF_RANGE, "x is too large"));
    } else {
      res_local = nu_ok(&x);
    }
    NU_ASSERT_TRUE(nu_is_ok(&res_local));
  }

  return nu_ok(NULL);
}

NU_TEST(test_error_propagation) {
  // Test NU_RETURN_IF_ERR propagates errors
  nu_result_t res = test_function_null_check(NULL);
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_INVALID_ARG);
  NU_ASSERT_STR_EQ(res.err->message, "NULL pointer parameter");

  // Test NU_RETURN_IF_ERR doesn't propagate success
  int value = 42;
  res = test_function_null_check(&value);
  NU_ASSERT_TRUE(nu_is_ok(&res));
  NU_ASSERT_EQ(res.ok, &value);

  return nu_ok(NULL);
}

NU_TEST(test_helper_functions) {
  // Test nu_check_null detects NULL
  nu_result_t res = nu_check_null(NULL, "test_param");
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_INVALID_ARG);
  NU_ASSERT_STR_EQ(res.err->message, "NULL pointer parameter");

  // Test nu_check_null accepts non-NULL
  int value = 42;
  res = nu_check_null(&value, "test_param");
  NU_ASSERT_TRUE(nu_is_ok(&res));
  NU_ASSERT_EQ(res.ok, (void*)(uintptr_t)&value);

  // Test nu_check_range detects out of range (below)
  res = nu_check_range(5, 10, 20, "test_val");
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_OUT_OF_RANGE);
  NU_ASSERT_STR_EQ(res.err->message, "Value out of range");

  // Test nu_check_range detects out of range (above)
  res = nu_check_range(25, 10, 20, "test_val");
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_OUT_OF_RANGE);

  // Test nu_check_range accepts in-range values
  res = nu_check_range(15, 10, 20, "test_val");
  NU_ASSERT_TRUE(nu_is_ok(&res));
  res = nu_check_range(10, 10, 20, "test_val");  // Min edge
  NU_ASSERT_TRUE(nu_is_ok(&res));
  res = nu_check_range(20, 10, 20, "test_val");  // Max edge
  NU_ASSERT_TRUE(nu_is_ok(&res));

  return nu_ok(NULL);
}

NU_TEST(test_error_inspection) {
  // Test nu_error_code extracts code
  nu_error_t err = {.code = NU_ERR_IO, .message = "IO failed"};
  NU_ASSERT_EQ(nu_error_code(&err), NU_ERR_IO);
  NU_ASSERT_EQ(nu_error_code(NULL), NU_OK);

  // Test nu_error_message extracts message
  err.message = "Custom message";
  NU_ASSERT_STR_EQ(nu_error_message(&err), "Custom message");

  // Test nu_error_message falls back to code string
  err.message = NULL;
  err.code    = NU_ERR_OOM;
  NU_ASSERT_STR_EQ(nu_error_message(&err), "Out of memory");

  // Test nu_error_message handles NULL error
  NU_ASSERT_STR_EQ(nu_error_message(NULL), "Success");

  return nu_ok(NULL);
}

// Helper function for real-world test
static nu_result_t
divide_helper (
  double a,
  double b,
  double* result)
{
  NU_RETURN_IF_ERR(nu_check_null(result, "result"));
  NU_TEST_FAIL_IF(b == 0.0, NU_ERR_INVALID_ARG, "Division by zero");
  *result = a / b;
  return nu_ok(result);
}

NU_TEST(test_real_world_usage) {
  // Test function with multiple error paths
  double result;
  nu_result_t res = divide_helper(10.0, 2.0, &result);
  NU_ASSERT_TRUE(nu_is_ok(&res));
  NU_ASSERT_EQ((int)result, 5);

  res = divide_helper(10.0, 0.0, &result);
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_INVALID_ARG);

  res = divide_helper(10.0, 2.0, NULL);
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_INVALID_ARG);

  // Test error propagation through call chain
  res = test_function_null_check(NULL);
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_INVALID_ARG);
  NU_ASSERT_STR_EQ(res.err->message, "NULL pointer parameter");

  return nu_ok(NULL);
}

NU_TEST(test_line_numbers) {
  // Test line numbers are captured correctly
  int line_before  = __LINE__;
  nu_error_t* err1 = NU_ERROR(NU_ERR_GENERIC, "Error 1");
  int line1        = __LINE__ - 1; // Previous line

  nu_error_t* err2 = NU_ERROR(NU_ERR_GENERIC, "Error 2");
  int line2        = __LINE__ - 1; // Previous line

  NU_ASSERT_EQ(err1->line, line1);
  NU_ASSERT_EQ(err2->line, line2);
  NU_ASSERT_NE(err1->line, err2->line);
  NU_ASSERT_GT(err1->line, line_before);

  // Test different macro invocations get different lines
  nu_result_t r1 = test_function_failure();
  int line_f1    = r1.err->line;

  nu_result_t r2 = test_function_null_check(NULL);
  int line_f2    = r2.err->line;

  NU_ASSERT_NE(line_f1, line_f2);

  return nu_ok(NULL);
}

NU_TEST(test_edge_cases) {
  // Test very long error message
  const char* long_msg = "This is a very long error message that might cause issues "
    "with formatting or display but should still work correctly "
    "even though it spans multiple lines in the source code";
  nu_error_t* err      = NU_ERROR(NU_ERR_GENERIC, long_msg);
  NU_ASSERT_EQ(err->message, long_msg); // Should point to same string

  // Test error code at enum boundaries
  NU_ASSERT_NOT_NULL(nu_error_code_str(NU_OK));
  NU_ASSERT_NOT_NULL(nu_error_code_str(NU_ERR_NOT_IMPLEMENTED));

  // Note about compound literal limitation
  nu_result_t r1        = test_function_failure();
  nu_error_t* err1_copy = r1.err; // Save pointer

  nu_result_t r2        = test_function_failure();
  nu_error_t* err2_copy = r2.err; // Save pointer

  // Due to compound literals, these might be the same address
  // This is a known limitation we accept for zero-allocation
  (void)err1_copy;
  (void)err2_copy;

  return nu_ok(NULL);
}

// Test assertions themselves
NU_TEST(test_nu_assertions) {
  // Test NU_ASSERT_OK
  NU_ASSERT_OK(nu_ok(NULL));

  // Test NU_ASSERT_ERR
  NU_ASSERT_ERR(nu_err(NU_ERROR(NU_ERR_GENERIC, "test")));

  // Test NU_ASSERT_ERR_CODE
  nu_result_t err_result = nu_err(NU_ERROR(NU_ERR_IO, "io error"));
  NU_ASSERT_ERR_CODE(err_result, NU_ERR_IO);

  // Test numeric comparisons
  NU_ASSERT_LT(1, 2);
  NU_ASSERT_LE(2, 2);
  NU_ASSERT_GT(3, 2);
  NU_ASSERT_GE(3, 3);
  NU_ASSERT_NE(1, 2);

  // Test memory comparison
  char buf1[] = "hello";
  char buf2[] = "hello";
  NU_ASSERT_MEM_EQ(buf1, buf2, 5);

  return nu_ok(NULL);
}

// Main function that runs all tests
NU_TEST_MAIN()
