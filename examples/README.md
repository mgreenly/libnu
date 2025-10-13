# nulib Examples

This directory contains example programs demonstrating the usage of different nulib modules. Each example focuses on a specific module and shows its typical usage patterns.

## Available Examples

- `arena.c` - Demonstrates the arena allocator with mark/restore functionality
- `bench.c` - Shows how to use the nu/bench.h benchmarking utilities
- `error.c` - Demonstrates error handling with the nu/error.h module
- `sort.c` - Demonstrates the introsort implementation from the sort module
- `test.c` - Shows how to use the nu/test.h testing framework
- `version.c` - Simple example showing how to query the library version

## Building Examples

All examples can be compiled the same way. Replace `<example>` with the name of the example file (without the .c extension).

### Using Installed Library

First install nulib:
```bash
sudo make install PREFIX=/usr/local
```

Then compile with dynamic linking:
```bash
gcc -std=c17 -Wall -Wextra -g <example>.c $(pkg-config --cflags --libs nu) -o <example>
./<example>
```

Or with static linking:
```bash
gcc -std=c17 -Wall -Wextra -g -static <example>.c $(pkg-config --cflags --libs --static nu) -o <example>
./<example>
```

### Using Source Directly (Without Installation)

From the project root directory:
```bash
gcc -std=c17 -Wall -Wextra -g examples/<example>.c -Isrc src/*.c -o examples/<example>
./examples/<example>
```

## Troubleshooting

Verify that pkg-config is installed and can find the installed library.

```bash
pkg-config --exists nu && echo "Found" || echo "Not found"
pkg-config --modversion nu
pkg-config --cflags --libs nu
```
