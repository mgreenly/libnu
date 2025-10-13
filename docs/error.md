# Error Handling in libnu

## Quick Start

### Single-Threaded Error Handling

```c
#include <nu/error.h>

nu_result_t divide(int a, int b, int* result) {
    if (b == 0) {
        return ERR(INVALID_ARG, "Division by zero");  // Return error
    }
    *result = a / b;
    return OK(result);  // Return success
}

nu_result_t calculate() {
    int result;
    TRY(divide(10, 2, &result));     // If error, returns immediately
    TRY(divide(result, 5, &result)); // Chains operations cleanly
    return OK(&result);
}

int main() {
    nu_result_t res = calculate();
    if (res.is_err) {                            // Check for error
        printf("Error: %s\n", res.err->message); // Access error details
        return 1;
    }
    printf("Result: %d\n", *(int*)res.ok);       // Access success value
    return 0;
}
```

### Multi-Threaded Error Handling

```c
#include <pthread.h>
#include <nu/error.h>

void* worker(void* arg) {
    ERR_T(IO, "Disk is full");
}

int main() {
    pthread_t thread;

    pthread_create(&thread, NULL, worker, NULL);

    nu_result_t res = COLLECT_THREAD(thread);
    if (res.is_err) {
        printf("Thread error: %s\n", res.err->message);
        return 1;
    }

    printf("Thread succeeded\n");   // Never gets here
    return 0;
}
```

## Core Concepts

### The Result Type

```c
typedef struct {
    union {
        void* ok;        // Success value
        nu_error_t* err; // Error details
    };
    bool is_err;         // Discriminator
} nu_result_t;
```

A Result is **always** one of:
- Success with a value (can be NULL)
- Error with details (code, message, location)

### Error Structure

```c
typedef struct nu_error {
    nu_error_code_t code;    // Enum for programmatic handling
    const char* file;        // Source file (__FILE__)
    int32_t line;            // Line number (__LINE__)
    char message[128];       // Human-readable message with printf formatting
} nu_error_t;
```

### Key Macros

**Basic:**
- `OK(value)` - Create a success result
- `ERR(CODE, fmt, ...)` - Create an error with printf-style message
- `TRY(expr)` - Propagate errors (like Rust's `?` operator)

**Resource cleanup:**
- `DEFER { }` - Start a block with deferred cleanup
- `FINALLY { }` - Cleanup code that always runs
- `SET_ERROR(CODE, fmt, ...)` - Set error and jump to cleanup
- `RETURN` - Return the accumulated result

## Design Philosophy

### Zero Heap Allocation
Errors use static storage, requiring no malloc/free in single-threaded code.

### Explicit Error Handling
Functions that can fail return `nu_result_t`, requiring explicit error checks.

### Error Context
Errors include:
- Error code (enum)
- Source location (file:line)
- Formatted message (printf-style)

### Simple API
Three main macros handle most error cases: `OK`, `ERR`, and `TRY`.

## Common Patterns

### Input Validation
```c
nu_result_t process_data(const char* input, size_t len, Buffer* out) {
    if (!input || !out) {
        return ERR(INVALID_ARG, "NULL parameter");
    }

    if (len > MAX_SIZE) {
        return ERR(OUT_OF_RANGE, "Length %zu exceeds maximum %d", len, MAX_SIZE);
    }

    // Process...
    return OK(out);
}
```

### Resource Acquisition
```c
nu_result_t read_file(const char* path, char** content) {
    FILE* f = fopen(path, "r");
    if (!f) {
        return ERR(IO, "Cannot open '%s': %s", path, strerror(errno));
    }

    // Read file...

    fclose(f);
    return OK(*content);
}
```

### Error Propagation Chain
```c
nu_result_t high_level_operation() {
    TRY(setup_environment());
    TRY(load_configuration());
    TRY(connect_services());
    TRY(run_main_logic());
    TRY(cleanup());

    return OK(NULL);
}
```

### Deferred Cleanup Pattern
```c
nu_result_t process_files(const char* in_path, const char* out_path) {
    FILE* in = NULL;
    FILE* out = NULL;
    char* buffer = NULL;

    DEFER {
        in = fopen(in_path, "r");
        if (!in) SET_ERROR(IO, "Cannot open %s", in_path);

        out = fopen(out_path, "w");
        if (!out) SET_ERROR(IO, "Cannot open %s", out_path);

        buffer = malloc(4096);
        if (!buffer) SET_ERROR(OOM, "Out of memory");

        // Process files...

        SET_OK(NULL);  // Success
    }
    FINALLY {
        if (buffer) free(buffer);
        if (out) fclose(out);
        if (in) fclose(in);
    }
    RETURN;  // Returns accumulated error or success
}
```

### Handling Specific Errors
```c
nu_result_t res = try_operation();
if (res.is_err) {
    switch (res.err->code) {
        case NU_ERR_TIMEOUT:
            // Retry
            return try_operation();

        case NU_ERR_PERMISSION:
            // Cannot recover
            return res;

        default:
            // Log and continue
            fprintf(stderr, "Warning: %s\n", res.err->message);
            break;
    }
}
```

## Trade-offs and Limitations

### Trade-offs Made

1. **No Error Chaining**: We don't support linked error causes. Each error stands alone. This simplifies memory management and avoids heap allocation.

2. **Fixed Message Size**: Error messages are limited to 127 characters. Longer messages are truncated. This enables stack allocation but may lose information.

3. **Compound Literal Scope**: Errors are temporary objects that only live for the current statement. You cannot store error pointers - errors must be used immediately or copied.

4. **Static Storage**: Each error creation site uses static storage. This means errors from the same location share storage (thread-unsafe for the error creation, though the Result passing is safe).

### Design Decisions

**Why not errno?**
- `errno` provides no context about where the error occurred
- Easy to forget to check
- Global state causes threading issues
- No type safety

**Why not exceptions?**
- C doesn't have exceptions
- Hidden control flow is hard to reason about
- Unpredictable performance characteristics
- Not suitable for embedded systems

**Why not error codes only?**
- Loses valuable debugging context
- Can't include runtime values in error description
- Harder to debug production issues

**Why 128-byte messages?**
- Large enough for most error messages with context
- Small enough to keep stack usage reasonable
- Fits in two cache lines on 64-bit systems
- Good balance between information and performance

### Limitations

1. **Thread Safety**: Error creation uses static storage shared across threads. Simultaneous error creation at the same source location may cause data races.

2. **No Custom Error Types**: All errors use the same structure. Domain-specific error types require wrapper functions.

3. **Message Truncation**: Messages longer than 127 characters are truncated.

4. **Static Storage**: Errors from the same source location share storage, which is not thread-safe but avoids heap allocation.

## Best Practices

1. **Always use Result types** for fallible functions
2. **Use error codes** for programmatic handling
3. **Use messages** for debugging and logging
4. **Include context** in error messages (what failed, what values caused it)
5. **Fail fast** - return errors immediately rather than continuing in a bad state
6. **Use TRY** for clean error propagation
7. **Document error conditions** in function comments
8. **Test error paths** - errors are part of your API contract

## Migration from Traditional C Error Handling

### From errno
```c
// Old style
int result = some_function();
if (result < 0) {
    perror("some_function failed");
    return -1;
}

// New style
nu_result_t res = some_function();
if (res.is_err) {
    fprintf(stderr, "Failed: %s at %s:%d\n",
            res.err->message, res.err->file, res.err->line);
    return res;
}
```

### From boolean returns
```c
// Old style
if (!do_something(arg)) {
    fprintf(stderr, "do_something failed\n");
    return false;
}

// New style
TRY(do_something(arg));  // Automatically propagates detailed error
```

### From custom error structs
```c
// Old style
MyError* err = NULL;
int result = function(&err);
if (err) {
    print_my_error(err);
    free_my_error(err);
    return -1;
}

// New style
nu_result_t res = function();
if (res.is_err) {
    // No cleanup needed - automatic lifetime
    return res;
}
```

## Thread-Safe Error Handling

### Thread-Safe Macros

**Sending side (in thread):**
- `OK_T(value)` - Return success from a thread
- `ERR_T(CODE, fmt, ...)` - Return a new error from a thread
- `ERR_T_FROM(errcode, fmt, ...)` - Convert existing error to thread return
- `RETURN_THREAD_RESULT(res)` - Auto-convert any `nu_result_t` to thread return

**Receiving side (in parent):**
- `COLLECT_THREAD(thread)` - Join thread and get back a regular `nu_result_t`

### Design Philosophy for Threading

1. **Within a thread**: Use regular `OK/ERR/TRY` macros (fast, zero-allocation)
2. **At thread boundaries**: Use `OK_T/ERR_T` to heap-allocate results for `pthread_join`
3. **Embedded errors**: The `nu_thread_result_t` type embeds the error directly (not as a pointer) to avoid lifetime issues
4. **Explicit allocation**: Only allocate at thread boundaries, not for every error

## Summary

The libnu error system provides:
- Type safety through Result types
- Zero heap allocation in single-threaded code
- File, line, and formatted message context
- Simple OK/ERR/TRY macros
- Thread boundary support with _T variants
- Header-only implementation
