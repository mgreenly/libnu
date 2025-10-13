/**
 * nu_error Tutorial Example
 *
 * This example demonstrates how to use nu_error for robust error handling
 * in C programs. We'll build a simple configuration file parser to show
 * real-world error handling patterns.
 *
 * Compile (after installing libnu):
 *   gcc -pthread -o error error.c -lnu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <nu/error.h>

/*
 * Step 1: Understanding the Result Type
 *
 * nu_error provides a Result type that can hold either:
 * - A success value (void* ok)
 * - An error (nu_error_t* err)
 *
 * This forces explicit error handling - you can't accidentally ignore errors!
 */

/*
 * Step 2: Define domain-specific error scenarios
 *
 * While nu_error provides common error codes, you can create
 * specific error messages for your application's needs.
 */

// A simple configuration structure for our example
typedef struct {
  char name[64];
  int32_t port;
  int32_t max_connections;
  bool verbose;
} ServerConfig;

/*
 * Step 3: Create functions that can fail
 *
 * Any function that might fail should return nu_result_t.
 * This makes error handling explicit and consistent.
 */

// Parse a single integer from a string
static nu_result_t
parse_int (const char* str, int32_t* out)
{
  // Validate inputs using nu_check_null
  NU_RETURN_IF_ERR(nu_check_null(str, "str"));
  NU_RETURN_IF_ERR(nu_check_null(out, "out"));

  // Skip whitespace
  while (isspace(*str))str++;

  // Check for empty string
  if (*str == '\0') {
    return ERR(INVALID_ARG, "Empty string cannot be parsed as integer");
  }

  // Use strtol for parsing
  char* endptr;
  long val = strtol(str, &endptr, 10);

  // Check for parsing errors
  if (endptr == str) {
    return ERR(INVALID_ARG, "No valid digits found");
  }

  // Check for trailing garbage
  while (isspace(*endptr))endptr++;
  if (*endptr != '\0') {
    return ERR(INVALID_ARG, "Invalid characters after number");
  }

  // Check range
  if (val < INT_MIN || val > INT_MAX) {
    return ERR(OUT_OF_RANGE, "Integer value out of range");
  }

  *out = (int32_t)val;
  return OK(out);
}

// Parse a configuration line like "key = value"
static nu_result_t
parse_config_line (const char* line, char* key, char* value)
{
  NU_RETURN_IF_ERR(nu_check_null(line, "line"));
  NU_RETURN_IF_ERR(nu_check_null(key, "key"));
  NU_RETURN_IF_ERR(nu_check_null(value, "value"));

  // Find the equals sign
  const char* equals = strchr(line, '=');
  if (!equals) {
    return ERR(INVALID_ARG, "Config line must contain '='");
  }

  // Extract key (trim whitespace)
  const char* key_start = line;
  const char* key_end   = equals - 1;
  while (key_start < equals && isspace(*key_start))key_start++;
  while (key_end > key_start && isspace(*key_end))key_end--;

  if (key_start > key_end) {
    return ERR(INVALID_ARG, "Empty key in config line");
  }

  size_t key_len = (size_t)(key_end - key_start + 1);
  if (key_len >= 64) {
    return ERR(BUFFER_FULL, "Key too long");
  }

  // Extract value (trim whitespace)
  const char* val_start = equals + 1;
  while (*val_start && isspace(*val_start))val_start++;
  const char* val_end   = val_start + strlen(val_start) - 1;
  while (val_end > val_start && isspace(*val_end))val_end--;

  size_t val_len        = (size_t)(val_end - val_start + 1);
  if (val_len >= 256) {
    return ERR(BUFFER_FULL, "Value too long");
  }

  // Copy trimmed strings
  memcpy(key, key_start, key_len);
  key[key_len]   = '\0';
  memcpy(value, val_start, val_len);
  value[val_len] = '\0';

  return OK(NULL);
}

// Parse a complete configuration
static nu_result_t
parse_server_config (const char* config_text, ServerConfig* config)
{
  NU_RETURN_IF_ERR(nu_check_null(config_text, "config_text"));
  NU_RETURN_IF_ERR(nu_check_null(config, "config"));

  // Initialize with defaults
  strcpy(config->name, "default");
  config->port            = 8080;
  config->max_connections = 100;
  config->verbose         = false;

  // Parse line by line
  char buffer[1024];
  strncpy(buffer, config_text, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  char* line       = strtok(buffer, "\n");
  int32_t line_num = 0;

  while (line) {
    line_num++;

    // Skip empty lines and comments
    while (isspace(*line))line++;
    if (*line == '\0' || *line == '#') {
      line = strtok(NULL, "\n");
      continue;
    }

    // Parse the line
    char key[64];
    char value[256];
    nu_result_t parse_result = parse_config_line(line, key, value);

    if (nu_is_err(&parse_result)) {
      printf("   Warning: Line %d: %s (skipping)\n",
        line_num, nu_error_message(parse_result.err));
      line = strtok(NULL, "\n");
      continue;
    }

    // Process known keys
    if (strcmp(key, "name") == 0) {
      strncpy(config->name, value, sizeof(config->name) - 1);
      config->name[sizeof(config->name) - 1] = '\0';
    }else if (strcmp(key, "port") == 0) {
      int32_t port;
      nu_result_t int_result = parse_int(value, &port);
      if (nu_is_err(&int_result)) {
        printf("   Warning: Invalid port value '%s': %s (using default)\n",
          value, nu_error_message(int_result.err));
      } else {
        // Validate port range
        NU_RETURN_IF_ERR(nu_check_range((size_t)port, 1, 65535, "port"));
        config->port = port;
      }
    }else if (strcmp(key, "max_connections") == 0) {
      int32_t max_conn;
      nu_result_t int_result = parse_int(value, &max_conn);
      if (nu_is_err(&int_result)) {
        printf("   Warning: Invalid max_connections '%s': %s (using default)\n",
          value, nu_error_message(int_result.err));
      } else {
        config->max_connections = max_conn;
      }
    }else if (strcmp(key, "verbose") == 0) {
      config->verbose = (strcmp(value, "true") == 0 ||
        strcmp(value, "yes") == 0 ||
        strcmp(value, "1") == 0);
    }else {
      printf("   Info: Unknown configuration key '%s' (ignoring)\n", key);
    }

    line = strtok(NULL, "\n");
  }

  return OK(config);
}

/*
 * Step 4: Demonstrate error handling patterns
 */

static void
print_config (const ServerConfig* cfg)
{
  printf("   Server Configuration:\n");
  printf("     Name: %s\n", cfg->name);
  printf("     Port: %d\n", cfg->port);
  printf("     Max Connections: %d\n", cfg->max_connections);
  printf("     Verbose: %s\n", cfg->verbose ? "yes" : "no");
}

/*
 * Thread-safe error handling example functions
 */

// Simulate work that can fail based on input
static nu_result_t
process_item(int item_id)
{
  if (item_id < 0) {
    return ERR(INVALID_ARG, "Item ID cannot be negative: %d", item_id);
  }
  if (item_id > 100) {
    return ERR(OUT_OF_RANGE, "Item ID too large: %d", item_id);
  }
  // Simulate some processing
  usleep(10000); // 10ms
  return OK(NULL);
}

// Thread function that processes an item
static void*
worker_thread(void* arg)
{
  int item_id = *(int*)arg;

  printf("   [Thread %ld] Processing item %d...\n", (long)pthread_self() % 10000, item_id);

  // Do the actual work
  nu_result_t res = process_item(item_id);

  // Convert result to thread-safe return
  if (nu_is_err(&res)) {
    printf("   [Thread %ld] Error occurred for item %d\n", (long)pthread_self() % 10000, item_id);
    ERR_T_FROM(res.err->code, "%s", res.err->message);
  }

  printf("   [Thread %ld] Successfully processed item %d\n", (long)pthread_self() % 10000, item_id);
  OK_T(NULL);
}

static void
demonstrate_thread_errors(void)
{
  printf("\n\nEXAMPLE 6: Thread-Safe Error Handling\n");
  printf("   Demonstrating passing errors across thread boundaries:\n\n");

  // Test with multiple threads - mix of valid and invalid items
  int test_items[] = {5, -10, 50, 200, 25};
  pthread_t threads[5];

  // Start threads
  printf("   Starting 5 worker threads...\n\n");
  for (int i = 0; i < 5; i++) {
    if (pthread_create(&threads[i], NULL, worker_thread, &test_items[i]) != 0) {
      printf("   Failed to create thread %d\n", i);
    }
  }

  // Collect results
  printf("\n   Collecting thread results:\n");
  for (int i = 0; i < 5; i++) {
    nu_result_t result = COLLECT_THREAD(threads[i]);

    if (nu_is_err(&result)) {
      printf("   ✗ Item %d failed: %s\n", test_items[i], nu_error_message(result.err));
    } else {
      printf("   ✓ Item %d processed successfully\n", test_items[i]);
    }
  }

  printf("\n   Thread example shows:\n");
  printf("   - ERR_T_FROM converts regular errors to thread-safe returns\n");
  printf("   - OK_T returns heap-allocated success values\n");
  printf("   - COLLECT_THREAD safely retrieves results from pthread_join\n");
}

int
main (void)
{
  printf("==============================================================\n");
  printf("           nu_error Tutorial - Error Handling in C\n");
  printf("==============================================================\n");

  /*
   * Example 1: Basic error checking
   */
  printf("\n\nEXAMPLE 1: Basic Error Checking\n");
  printf("   Parsing integers with explicit error handling:\n\n");

  const char* test_numbers[] = {
    "42",               // Valid
    "  -123  ",         // Valid with whitespace
    "not_a_number",     // Invalid
    "123abc",           // Trailing garbage
    "99999999999",      // Out of range
    "",                 // Empty
    NULL                // NULL pointer
  };

  for (int32_t i = 0; i < 7; i++) {
    int32_t value      = 0;
    printf("   Parsing '%s':\n", test_numbers[i] ? test_numbers[i] : "NULL");

    nu_result_t result = parse_int(test_numbers[i], &value);

    if (nu_is_ok(&result)) {
      printf("     ✓ Success: %d\n", value);
    } else {
      printf("     ✗ Error: %s\n", nu_error_message(result.err));
      // In debug builds, you might also print location:
      // nu_error_fprintf(stderr, result.err);
    }
  }

  /*
   * Example 2: Configuration parsing with partial errors
   */
  printf("\n\nEXAMPLE 2: Configuration Parsing\n");
  printf("   Parsing a configuration with some invalid entries:\n\n");

  const char* config1 =
    "# Server configuration file\n"
    "name = MyServer\n"
    "port = 3000\n"
    "max_connections = 50\n"
    "verbose = true\n";

  ServerConfig config;
  nu_result_t cfg_result = parse_server_config(config1, &config);

  if (nu_is_ok(&cfg_result)) {
    printf("   ✓ Configuration parsed successfully:\n\n");
    print_config(&config);
  } else {
    printf("   ✗ Failed to parse: %s\n", nu_error_message(cfg_result.err));
  }

  /*
   * Example 3: Handling invalid configuration
   */
  printf("\n\nEXAMPLE 3: Invalid Configuration\n");
  printf("   Demonstrating graceful error recovery:\n\n");

  const char* config2 =
    "name = TestServer\n"
    "port = not_a_port\n"              // Invalid
    "max_connections = 200\n"
    "invalid_line_without_equals\n"     // Invalid format
    "timeout = 30\n"                    // Unknown key
    "verbose = yes\n";

  nu_result_t cfg_result2 = parse_server_config(config2, &config);

  if (nu_is_ok(&cfg_result2)) {
    printf("\n   ✓ Configuration parsed (with warnings above):\n\n");
    print_config(&config);
    printf("\n   Notice: Invalid values were skipped or used defaults\n");
  }

  /*
   * Example 4: NULL safety
   */
  printf("\n\nEXAMPLE 4: NULL Safety\n");
  printf("   nu_error helps prevent NULL pointer issues:\n\n");

  nu_result_t null_result = parse_server_config(NULL, &config);
  if (nu_is_err(&null_result)) {
    printf("   ✓ NULL input detected: %s\n", nu_error_message(null_result.err));
  }

  null_result = parse_server_config(config1, NULL);
  if (nu_is_err(&null_result)) {
    printf("   ✓ NULL output detected: %s\n", nu_error_message(null_result.err));
  }

  /*
   * Example 5: Error propagation pattern
   */
  printf("\n\nEXAMPLE 5: Error Propagation\n");
  printf("   Using NU_RETURN_IF_ERR for clean error propagation:\n\n");

  printf("   When a function calls multiple operations that can fail,\n");
  printf("   NU_RETURN_IF_ERR automatically propagates errors upward.\n");
  printf("   This eliminates repetitive error checking code.\n");

  /*
   * Example 6: Thread-safe error handling
   */
  demonstrate_thread_errors();

  /*
   * Performance and design notes
   */
  printf("\n\nDESIGN NOTES:\n");
  printf("   • Result types make errors impossible to ignore\n");
  printf("   • Zero overhead for success path (just a bool check)\n");
  printf("   • File/line info captured automatically for debugging\n");
  printf("   • Header-only implementation - no linking required\n");

  /*
   * Key takeaways
   */
  printf("\n\nKEY TAKEAWAYS:\n");
  printf("   1. Always return nu_result_t from fallible functions\n");
  printf("   2. Use TRY() or NU_RETURN_IF_ERR to propagate errors cleanly\n");
  printf("   3. Use ERR() for immediate error returns (cleaner than NU_FAIL)\n");
  printf("   4. Use OK() for success returns\n");
  printf("   5. Check results with nu_is_ok() / nu_is_err()\n");
  printf("   6. NULL checks are built-in with nu_check_null()\n");
  printf("   7. Range validation is easy with nu_check_range()\n");

  printf("\n\nTHREAD-SAFE ERROR HANDLING:\n");
  printf("   For passing errors across thread boundaries, use:\n");
  printf("   • OK_T(value) - Return success from thread (heap-allocated)\n");
  printf("   • ERR_T(code, fmt, ...) - Return error from thread\n");
  printf("   • ERR_T_FROM(code, fmt, ...) - Convert existing error to thread return\n");
  printf("   • RETURN_THREAD_RESULT(res) - Auto-convert nu_result_t to thread result\n");
  printf("   • COLLECT_THREAD(thread) - Join thread and convert result back\n");
  printf("   Thread results (nu_thread_result_t) embed errors directly, not as pointers,\n");
  printf("   making them safe to pass through pthread_join().\n");

  printf("\n\nIMPORTANT LIMITATION:\n");
  printf("   Error objects are temporary (compound literals).\n");
  printf("   Use errors immediately - don't store error pointers!\n");

  printf("\n\nTutorial complete!\n\n");

  return 0;
}
