#ifndef NU_ERROR_H
#define NU_ERROR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

// Error with context - only allocated on failure
typedef struct nu_error {
  nu_error_code_t code;
  const char* message;           // Static string preferred
  const char* file;
  int line;
  struct nu_error* cause;        // Chain to previous error (owned)
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

// Error creation - using compound literals
// These create temporary error objects that should be used immediately
// For persistent errors, allocate them separately
#define NU_ERROR(errcode, errmsg) \
        (&(nu_error_t){ \
    .code = (errcode), \
    .message = (errmsg), \
    .file = __FILE__, \
    .line = __LINE__, \
    .cause = NULL \
  })

#define NU_ERROR_CHAIN(errcode, errmsg, errcause) \
        (&(nu_error_t){ \
    .code = (errcode), \
    .message = (errmsg), \
    .file = __FILE__, \
    .line = __LINE__, \
    .cause = (errcause) \
  })

// Quick error returns
#define NU_RETURN_IF_ERR(result) \
        do { \
          nu_result_t _r = (result); \
          if (nu_is_err(&_r)) return _r; \
        } while (0)

#define NU_FAIL(code, msg) \
        return nu_err(NU_ERROR(code, msg))

#define NU_FAIL_IF(cond, code, msg) \
        do { if (cond) NU_FAIL(code, msg); } while (0)

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
  return err->message ? err->message : nu_error_code_str(err->code);
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

  const nu_error_t* e = err;
  int depth           = 0;
  while (e) {
    for (int i = 0; i < depth; i++)fprintf(f, "  ");
    fprintf(f, "→ %s", nu_error_message(e));
    if (e->file) {
      fprintf(f, " [%s:%d]", e->file, e->line);
    }
    fprintf(f, "\n");
    e = e->cause;
    depth++;
  }
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

#endif // NU_ERROR_H
