/*
 * Tutorial: Writing Tests with nu/test.h
 *
 * This example demonstrates how to write unit tests using libnu's minimal
 * test framework. The framework dogfoods nu/error.h, showing how Result
 * types provide consistent error handling even in test code.
 *
 * Compile (after installing libnu):
 *   gcc -o test test.c -lnu
 *   ./test
 */

#include <stdint.h>
#include <nu/test.h>
#include <nu/error.h>

/*
 * Example 1: Basic Test Structure
 *
 * Every test is defined with NU_TEST and returns nu_result_t.
 * This creates a function and automatically registers it to run.
 */
NU_TEST(test_basic_math) {
  // Simple assertions
  NU_ASSERT_EQ(2 + 2, 4);
  NU_ASSERT_NE(5, 10);

  // All tests must return nu_ok on success
  return nu_ok(NULL);
}

/*
 * Example 2: Testing Your Own Functions
 *
 * Let's test a discount calculator that uses nu_error patterns.
 */
static nu_result_t
calculate_discount (double price, double discount_percent, double* result)
{
  // Validate inputs using nu_error patterns
  NU_RETURN_IF_ERR(nu_check_null(result, "result"));

  if (price < 0) {
    return ERR(INVALID_ARG, "Price cannot be negative");
  }

  if (discount_percent < 0 || discount_percent > 100) {
    return ERR(OUT_OF_RANGE, "Discount must be between 0 and 100");
  }

  *result = price * (1.0 - discount_percent / 100.0);
  return OK(result);
}

NU_TEST(test_discount_calculator_success) {
  double result;

  // NU_ASSERT_OK verifies a function returned success
  NU_ASSERT_OK(calculate_discount(100.0, 20.0, &result));
  NU_ASSERT_EQ((int)result, 80);

  // Test edge cases
  NU_ASSERT_OK(calculate_discount(50.0, 0.0, &result));
  NU_ASSERT_EQ((int)result, 50);

  NU_ASSERT_OK(calculate_discount(200.0, 100.0, &result));
  NU_ASSERT_EQ((int)result, 0);

  return nu_ok(NULL);
}

NU_TEST(test_discount_calculator_errors) {
  double result;

  // NU_ASSERT_ERR verifies a function returned an error
  // Test NULL result pointer
  {
    nu_result_t res = calculate_discount(100.0, 20.0, NULL);
    NU_ASSERT_ERR(res);
    // Note: We can't use NU_ASSERT_ERR_CODE here due to compound literal limitations
  }

  // Test negative price
  {
    nu_result_t res = calculate_discount(-50.0, 20.0, &result);
    NU_ASSERT_ERR(res);
  }

  // Test discount out of range
  {
    nu_result_t res = calculate_discount(100.0, 150.0, &result);
    NU_ASSERT_ERR(res);
  }

  return nu_ok(NULL);
}

/*
 * Example 3: All Available Assertions
 *
 * This test demonstrates every assertion type provided by nu/test.h
 */
NU_TEST(test_assertion_types) {
  // Boolean assertions
  NU_ASSERT_TRUE(1 == 1);
  NU_ASSERT_FALSE(0 == 1);

  // Generic assertion (any boolean expression)
  int32_t x = 5;
  NU_ASSERT(x > 0 && x < 10);

  // Numeric comparisons
  NU_ASSERT_LT(1, 2);    // less than
  NU_ASSERT_LE(2, 2);    // less or equal
  NU_ASSERT_GT(3, 2);    // greater than
  NU_ASSERT_GE(3, 3);    // greater or equal

  // Pointer checks
  int32_t value = 42;
  NU_ASSERT_NOT_NULL(&value);
  NU_ASSERT_NULL(NULL);

  // String comparison
  NU_ASSERT_STR_EQ("hello", "hello");
  NU_ASSERT_STR_NE("hello", "world");

  // Memory comparison (useful for arrays and structs)
  int32_t arr1[] = {1, 2, 3};
  int32_t arr2[] = {1, 2, 3};
  NU_ASSERT_MEM_EQ(arr1, arr2, sizeof(arr1));

  return nu_ok(NULL);
}

/*
 * Example 4: Custom Test Failures
 *
 * Sometimes you need custom error messages or conditions.
 */
NU_TEST(test_custom_failures) {
  int32_t config_version   = 2;
  int32_t required_version = 3;

  // Use NU_TEST_FAIL_IF for custom conditions
  NU_TEST_FAIL_IF(
    config_version < required_version,
    NU_ERR_NOT_IMPLEMENTED,
    "Config version too old"
    );

  // This line won't be reached if the above condition is true
  return nu_ok(NULL);
}

/*
 * Example 5: Testing Patterns
 *
 * Common patterns when writing tests with nu/test.h
 */
NU_TEST(test_common_patterns) {
  // Pattern 1: Setup state (no fixtures needed, just use locals)
  int32_t test_data[] = {5, 3, 8, 1, 9};

  // Pattern 2: Test operations that verify state
  NU_ASSERT_EQ(test_data[0], 5);
  NU_ASSERT_EQ(test_data[4], 9);

  // Pattern 3: Test cleanup happens automatically
  // No need for teardown - locals are cleaned up on return

  return nu_ok(NULL);
}

/*
 * Main Function
 *
 * NU_TEST_MAIN() generates a main() that:
 * 1. Runs all tests registered with NU_TEST
 * 2. Reports results (PASS/FAIL)
 * 3. Shows file:line for failures
 * 4. Returns 0 on success, 1 if any test failed
 */
NU_TEST_MAIN()

/*
 * When you run this, you'll see output like:
 *
 * Running 6 tests...
 *   test_basic_math... PASS
 *   test_discount_calculator_success... PASS
 *   test_discount_calculator_errors... PASS
 *   test_assertion_types... PASS
 *   test_custom_failures... FAIL
 *     → Config version too old [test.c:137]
 *   test_common_patterns... PASS
 *
 * Tests run: 6
 * Passed: 5
 * Failed: 1
 *
 * Key Features of nu/test.h:
 * - Zero dynamic allocation
 * - Automatic test discovery via __attribute__((constructor))
 * - Tests return nu_result_t for consistent error handling
 * - ~250 lines of readable code you can understand
 * - No external dependencies
 *
 * Limitations:
 * - Maximum 512 tests per executable
 * - Linux/macOS only (uses constructor attributes)
 * - No fixtures (use static/local variables instead)
 * - No mocking (test real code)
 */
