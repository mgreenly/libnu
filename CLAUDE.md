# Claude Code Project Guidelines

## Memory Allocation Pattern

The project uses a configurable memory allocation pattern for testing:

- **All source code** expects `NU_MALLOC` and `NU_FREE` to be defined by the compiler
- **The Makefile** always defines these macros:
  - For library builds: `-DNU_MALLOC=malloc -DNU_FREE=free`
  - For test builds: `-DNU_MALLOC=test_malloc -DNU_FREE=free`
- **Source files** should use `NU_MALLOC()` and `NU_FREE()` instead of direct `malloc()`/`free()` calls
- **Source files** must declare: `extern void* NU_MALLOC(size_t size); extern void NU_FREE(void* ptr);`
- This allows testing malloc failure paths without modifying source code
- The `NU_` prefix prevents namespace conflicts with user code

## Code Style

### Pointer Formatting
The project uses "star next to type" style for pointer declarations with a space after the star:
- Use `char* ptr` not `char *ptr` or `char * ptr`
- Use `void* base` not `void *base`
- In casts, use `(char*) base` not `(char *)base`
- For function pointers: `int (*compar)(const void*, const void* )`

Note: The uncrustify configuration has been set to `ignore` for pointer spacing options to preserve manual formatting. The formatter will not change existing pointer spacing, allowing us to maintain our preferred `type* name` style with a space after the star.

Exception: In function pointer parameters, uncrustify may still adjust spacing around commas, resulting in `(const void*, const void* )` instead of `(const void* , const void* )`.

### Code Formatting
Run `make fmt` to format code using uncrustify with the project's configuration file. The formatter preserves most manual pointer formatting choices.
