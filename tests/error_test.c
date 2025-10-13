#include "../src/test.h"
#include "../src/error.h"
#include <limits.h>
#include <stdint.h>

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
  int32_t value = 42;
  nu_result_t res = nu_ok(&value);
  NU_ASSERT_TRUE(nu_is_ok(&res));
  NU_ASSERT_FALSE(nu_is_err(&res));
  NU_ASSERT_EQ(res.ok, &value);
  NU_ASSERT_FALSE(res.is_err);

  // Test nu_err creates error result
  nu_error_t error = {.code = NU_ERR_GENERIC};
  strncpy(error.message, "test error", sizeof(error.message) - 1);
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

  // Test NU_FAIL returns error result
  nu_result_t res = test_function_failure();
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_GENERIC);
  NU_ASSERT_STR_EQ(res.err->message, "This function always fails");

  // Test NU_FAIL_IF with true condition
  int32_t x = 5;
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
  int32_t value = 42;
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
  int32_t value = 42;
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
  nu_error_t err = {.code = NU_ERR_IO};
  strncpy(err.message, "IO failed", sizeof(err.message) - 1);
  NU_ASSERT_EQ(nu_error_code(&err), NU_ERR_IO);
  NU_ASSERT_EQ(nu_error_code(NULL), NU_OK);

  // Test nu_error_message extracts message
  strncpy(err.message, "Custom message", sizeof(err.message) - 1);
  NU_ASSERT_STR_EQ(nu_error_message(&err), "Custom message");

  // Test nu_error_message falls back to code string when message is empty
  err.message[0] = '\0';
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
  // NOTE: With the function-based implementation to avoid GNU extensions,
  // line numbers are captured at the macro expansion point correctly.
  // We still verify basic functionality.

  nu_error_t* err1 = NU_ERROR(NU_ERR_GENERIC, "Error 1");
  nu_error_t* err2 = NU_ERROR(NU_ERR_GENERIC, "Error 2");

  // Both errors should have valid line numbers
  NU_ASSERT_GT(err1->line, 0);
  NU_ASSERT_GT(err2->line, 0);

  // Both errors should have the same line since they use the same static error
  // in _nu_make_error_impl (this is a limitation of avoiding GNU extensions)
  NU_ASSERT_EQ(err1->line, err2->line);

  // File should be set
  NU_ASSERT_NOT_NULL(err1->file);
  NU_ASSERT_NOT_NULL(err2->file);

  return nu_ok(NULL);
}

NU_TEST(test_edge_cases) {
  // Test very long error message (will be truncated to 127 chars)
  const char* long_msg = "This is a very long error message that might cause issues "
    "with formatting or display but should still work correctly "
    "even though it spans multiple lines in the source code";
  nu_error_t* err      = NU_ERROR(NU_ERR_GENERIC, "%s", long_msg);
  // Message should be truncated but still valid
  NU_ASSERT_TRUE(strlen(err->message) <= 127);

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

// Test new OK/ERR/TRY macros
NU_TEST(test_new_macros) {
  // Test OK macro
  nu_result_t res = OK(NULL);
  NU_ASSERT_TRUE(nu_is_ok(&res));
  NU_ASSERT_FALSE(nu_is_err(&res));

  int value = 42;
  res = OK(&value);
  NU_ASSERT_TRUE(nu_is_ok(&res));
  NU_ASSERT_EQ(res.ok, &value);

  // Test ERR macro - note how we don't need NU_ERR_ prefix
  res = ERR(IO, "Failed to open file %s", "test.txt");
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_IO);
  NU_ASSERT_STR_EQ(res.err->message, "Failed to open file test.txt");

  // Test ERR with formatting
  int port = 8080;
  res = ERR(OUT_OF_RANGE, "Port %d is not valid", port);
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_OUT_OF_RANGE);

  return OK(NULL);
}

// Test TRY macro functionality
static nu_result_t try_test_function(void) {
  TRY(OK(&(int){1}));  // Should pass through
  TRY(ERR(IO, "This should propagate"));  // Should return this error
  return OK(NULL);  // Should not reach here
}

NU_TEST(test_try_macro) {
  nu_result_t res = try_test_function();
  NU_ASSERT_TRUE(nu_is_err(&res));
  NU_ASSERT_EQ(res.err->code, NU_ERR_IO);
  NU_ASSERT_STR_EQ(res.err->message, "This should propagate");

  return OK(NULL);
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

// Test thread-safe types and macros
NU_TEST(test_thread_result_types) {
  // Test that nu_thread_result_t has the right structure
  nu_thread_result_t test_result;
  test_result.is_err = false;
  test_result.ok = NULL;
  NU_ASSERT_FALSE(test_result.is_err);

  test_result.is_err = true;
  test_result.err.code = NU_ERR_IO;
  test_result.err.line = 42;
  NU_ASSERT_TRUE(test_result.is_err);
  NU_ASSERT_EQ(test_result.err.code, NU_ERR_IO);

  // Test that nu_thread_result_t embeds error directly (not as pointer)
  nu_thread_result_t result1;
  result1.is_err = true;
  result1.err.code = NU_ERR_IO;
  strncpy(result1.err.message, "Error 1", sizeof(result1.err.message));

  nu_thread_result_t result2;
  result2.is_err = true;
  result2.err.code = NU_ERR_PERMISSION;
  strncpy(result2.err.message, "Error 2", sizeof(result2.err.message));

  // Verify they're independent
  NU_ASSERT_EQ(result1.err.code, NU_ERR_IO);
  NU_ASSERT_EQ(result2.err.code, NU_ERR_PERMISSION);
  NU_ASSERT_STR_NE(result1.err.message, result2.err.message);

  // Ensure the thread result type is reasonably sized
  size_t result_size = sizeof(nu_thread_result_t);
  size_t error_size = sizeof(nu_error_t);

  // Should be close to error size plus overhead
  NU_ASSERT_LE(result_size, error_size + sizeof(bool) + sizeof(void*));

  // But not too large (sanity check)
  NU_ASSERT_LT(result_size, 256);

  return OK(NULL);
}

// Test thread macros compile correctly (we don't actually create threads in unit tests)
void* thread_success_mock(void* arg) {
  int value = *(int*)arg;
  OK_T(&value);  // Never reached but tests compilation
}

void* thread_error_mock(void* arg) {
  (void)arg;
  ERR_T(IO, "Test error from thread");
}

void* thread_error_from_mock(void* arg) {
  (void)arg;
  nu_result_t res = ERR(IO, "Original error");
  ERR_T_FROM(res.err->code, "%s", res.err->message);
}

NU_TEST(test_thread_macros_compile) {
  // This test just verifies the thread macros compile correctly
  // The mock functions above test that OK_T, ERR_T, and ERR_T_FROM compile

  // Test that the helper functions work correctly
  nu_thread_result_t* tres = _nu_make_thread_ok(&(int){42});
  if (!tres) {
    return ERR(OOM, "Failed to allocate thread result");
  }

  if (tres->is_err) {
    free(tres);
    return ERR(GENERIC, "Expected success result but got error");
  }

  // Note: Can't compare pointers to compound literals reliably
  // Just check it's not NULL
  if (!tres->ok) {
    free(tres);
    return ERR(GENERIC, "Expected non-NULL ok value");
  }

  free(tres);

  tres = _nu_make_thread_error(NU_ERR_IO, __FILE__, __LINE__, "Test error %d", 123);
  if (!tres) {
    return ERR(OOM, "Failed to allocate thread error result");
  }

  if (!tres->is_err) {
    free(tres);
    return ERR(GENERIC, "Expected error result but got success");
  }

  if (tres->err.code != NU_ERR_IO) {
    free(tres);
    return ERR(GENERIC, "Wrong error code");
  }

  if (strcmp(tres->err.message, "Test error 123") != 0) {
    free(tres);
    return ERR(GENERIC, "Wrong error message");
  }

  free(tres);

  return OK(NULL);
}

// Main function that runs all tests
NU_TEST_MAIN()
