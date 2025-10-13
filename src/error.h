#ifndef NU_ERROR_H
#define NU_ERROR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>

// Error codes - extend as needed
typedef enum {
  NU_OK = 0,
  NU_ERR_GENERIC = 1,
  NU_ERR_OOM,
  NU_ERR_INVALID_ARG,
  NU_ERR_OUT_OF_RANGE,
  NU_ERR_NOT_FOUND,
  NU_ERR_PERMISSION,
  NU_ERR_IO,
  NU_ERR_INVALID_UTF8,
  NU_ERR_BUFFER_FULL,
  NU_ERR_WOULD_BLOCK,
  NU_ERR_NOT_IMPLEMENTED,
} nu_error_code_t;

// Error with context and embedded message buffer
typedef struct nu_error {
  nu_error_code_t code;
  const char* file;
  int32_t line;
  char message[128];             // Embedded message buffer for printf-style formatting
} nu_error_t;

// Result type - can hold either success value or error
// This is stack-allocated, zero overhead for success case
typedef struct {
  union {
    void* ok;
    nu_error_t* err;
  };
  bool is_err;
} nu_result_t;

// Core Result construction - always inline for zero overhead
static inline nu_result_t
nu_ok (void* value)
{
  return (nu_result_t){.ok = value, .is_err = false};
}

static inline nu_result_t
nu_err (nu_error_t* error)
{
  return (nu_result_t){.err = error, .is_err = true};
}

// Result checking
static inline bool
nu_is_ok (const nu_result_t* result)
{
  return !result->is_err;
}

static inline bool
nu_is_err (const nu_result_t* result)
{
  return result->is_err;
}

// Internal macro for creating errors with printf-style formatting
// Use a function-like approach to avoid statement expressions
static inline nu_error_t*
_nu_make_error_impl(nu_error_code_t code, const char* file, int line, const char* fmt, ...)
{
  static nu_error_t _err;
  _err.code = code;
  _err.file = file;
  _err.line = line;

  va_list args;
  va_start(args, fmt);
  vsnprintf(_err.message, sizeof(_err.message), fmt, args);
  va_end(args);

  return &_err;
}

#define _NU_MAKE_ERROR(errcode, ...) \
        _nu_make_error_impl(errcode, __FILE__, __LINE__, __VA_ARGS__)

// Clean public API - Rust-style OK/ERR
#define OK(value) nu_ok(value)

#define ERR(code_suffix, ...) \
        nu_err(_NU_MAKE_ERROR(NU_ERR_##code_suffix, __VA_ARGS__))

// TRY macro - return early if error (like Rust's ? operator)
#define TRY(expr) \
        do { \
          nu_result_t _result = (expr); \
          if (_result.is_err) { \
            static nu_error_t _err_copy; \
            _err_copy = *_result.err; \
            return nu_err(&_err_copy); \
          } \
        } while (0)

// Backward compatibility / alternative names
#define NU_ERROR(errcode, ...) \
        _NU_MAKE_ERROR(errcode, __VA_ARGS__)

#define NU_FAIL(code, ...) \
        return nu_err(NU_ERROR(code, __VA_ARGS__))

#define NU_FAIL_IF(cond, code, ...) \
        do { if (cond) NU_FAIL(code, __VA_ARGS__); } while (0)

#define NU_RETURN_IF_ERR(result) TRY(result)

// Error code to string conversion
static inline const char*
nu_error_code_str (nu_error_code_t code)
{
  switch (code) {
    case NU_OK:                 return "OK";
    case NU_ERR_GENERIC:        return "Generic error";
    case NU_ERR_OOM:            return "Out of memory";
    case NU_ERR_INVALID_ARG:    return "Invalid argument";
    case NU_ERR_OUT_OF_RANGE:   return "Out of range";
    case NU_ERR_NOT_FOUND:      return "Not found";
    case NU_ERR_PERMISSION:     return "Permission denied";
    case NU_ERR_IO:             return "I/O error";
    case NU_ERR_INVALID_UTF8:   return "Invalid UTF-8";
    case NU_ERR_BUFFER_FULL:    return "Buffer full";
    case NU_ERR_WOULD_BLOCK:    return "Operation would block";
    case NU_ERR_NOT_IMPLEMENTED: return "Not implemented";
    default:                    return "Unknown error";
  }
}

// Error inspection
static inline nu_error_code_t
nu_error_code (const nu_error_t* err)
{
  return err ? err->code : NU_OK;
}

static inline const char*
nu_error_message (const nu_error_t* err)
{
  if (!err)return "Success";
  // Check if message buffer has content, otherwise use code string
  return err->message[0] ? err->message : nu_error_code_str(err->code);
}

// Error formatting for debugging
static inline void
nu_error_fprintf (
  FILE* f,
  const nu_error_t* err)
{
  if (!err) {
    fprintf(f, "Success\n");
    return;
  }

  fprintf(f, "Error: %s [%s:%d]\n",
          nu_error_message(err),
          err->file ? err->file : "unknown",
          err->line);
}

// Common error conditions as inline functions
static inline nu_result_t
nu_check_null (
  const void* ptr,
  const char* param_name)
{
  (void)param_name;     // Unused - would need persistent storage for custom message
  if (!ptr) {
    // Use generic message - dynamic formatting would require persistent storage
    NU_FAIL(NU_ERR_INVALID_ARG, "NULL pointer parameter");
  }
  // Cast away const is safe here - we're just passing through the pointer
  return nu_ok((void*)(uintptr_t)ptr);
}

static inline nu_result_t
nu_check_range (
  size_t value,
  size_t min,
  size_t max,
  const char* param_name)
{
  (void)param_name;     // Unused - would need persistent storage for custom message
  if (value < min || value > max) {
    // Use generic message - dynamic formatting would require persistent storage
    NU_FAIL(NU_ERR_OUT_OF_RANGE, "Value out of range");
  }
  return nu_ok(NULL);
}

// Thread result type for passing across thread boundaries
// This type embeds the error data directly (not as a pointer) to avoid lifetime issues
typedef struct {
  union {
    void* ok;
    nu_error_t err;  // Embedded directly, not as pointer
  };
  bool is_err;
} nu_thread_result_t;

// Thread-safe helper functions for thread boundary returns
// These allocate on heap for safe passing via pthread_join

static inline nu_thread_result_t*
_nu_make_thread_ok(void* value)
{
  nu_thread_result_t* res = malloc(sizeof(nu_thread_result_t));
  if (res) {
    res->is_err = false;
    res->ok = value;
  }
  return res;
}

static inline nu_thread_result_t*
_nu_make_thread_error(nu_error_code_t code, const char* file, int line, const char* fmt, ...)
{
  nu_thread_result_t* res = malloc(sizeof(nu_thread_result_t));
  if (res) {
    res->is_err = true;
    res->err.code = code;
    res->err.file = file;
    res->err.line = line;

    va_list args;
    va_start(args, fmt);
    vsnprintf(res->err.message, sizeof(res->err.message), fmt, args);
    va_end(args);
  }
  return res;
}

// Return success from thread - allocates result on heap
#define OK_T(value) \
        return _nu_make_thread_ok(value)

// Return error from thread - allocates result on heap
#define ERR_T(code_suffix, ...) \
        return _nu_make_thread_error(NU_ERR_##code_suffix, __FILE__, __LINE__, __VA_ARGS__)

// Helper to convert existing error to thread-safe return
// This version takes a full error code, not just the suffix
#define ERR_T_FROM(errcode, ...) \
        return _nu_make_thread_error(errcode, __FILE__, __LINE__, __VA_ARGS__)

// Helper to convert regular result to thread result at boundary
#define RETURN_THREAD_RESULT(res) \
        do { \
          if ((res).is_err) { \
            ERR_T_FROM((res).err->code, "%s", (res).err->message); \
          } else { \
            OK_T((res).ok); \
          } \
        } while(0)

// Helper function to collect thread result and convert back to regular nu_result_t
// This handles pthread_join, conversion, and cleanup automatically
static inline nu_result_t
_nu_collect_thread(pthread_t thread)
{
  nu_thread_result_t* tres;
  pthread_join(thread, (void**)&tres);

  nu_result_t res;
  if (tres->is_err) {
    static nu_error_t err;
    err = tres->err;
    res = nu_err(&err);
  } else {
    res = nu_ok(tres->ok);
  }

  free(tres);
  return res;
}

#define COLLECT_THREAD(thread) _nu_collect_thread(thread)

// Deferred cleanup macros for resource management
// These allow goto-based cleanup while still using the error system

// Start a deferred cleanup block
#define DEFER \
        nu_result_t _defer_result = OK(NULL); \
        goto _defer_body; \
        _defer_cleanup: \
        goto _defer_end; \
        _defer_body:

// Start the cleanup section
#define FINALLY \
        goto _defer_cleanup; \
        _defer_end:

// Return the accumulated result
#define RETURN return _defer_result

// Set an error and jump to cleanup
#define SET_ERROR(code_suffix, ...) \
        do { \
          _defer_result = ERR(code_suffix, __VA_ARGS__); \
          goto _defer_end; \
        } while(0)

// Set success and jump to cleanup
#define SET_OK(value) \
        do { \
          _defer_result = OK(value); \
          goto _defer_end; \
        } while(0)

#endif // NU_ERROR_H
