#ifndef NU_ARENA_H
#define NU_ARENA_H

#include <stddef.h>
#include <stdbool.h>

typedef struct nu_arena nu_arena;
typedef struct nu_arena_mark nu_arena_mark;

struct nu_arena {
  char* buffer;
  size_t capacity;
  size_t used;
};

struct nu_arena_mark {
  size_t position;
};

/**
 * @brief Initialize an arena allocator
 * @param arena Pointer to arena structure to initialize
 * @param buffer Memory buffer to use for allocations
 * @param capacity Size of the buffer in bytes
 * @return true on success, false on failure
 */
bool nu_arena_init(nu_arena* arena, void* buffer, size_t capacity);

/**
 * @brief Allocate memory from the arena
 * @param arena Pointer to the arena
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory or NULL on failure
 */
void* nu_arena_alloc(nu_arena* arena, size_t size);

/**
 * @brief Allocate aligned memory from the arena
 * @param arena Pointer to the arena
 * @param size Number of bytes to allocate
 * @param alignment Required alignment (must be power of 2)
 * @return Pointer to aligned memory or NULL on failure
 */
void* nu_arena_alloc_aligned(nu_arena* arena, size_t size, size_t alignment);

/**
 * @brief Mark current position in arena for later restore
 * @param arena Pointer to the arena
 * @return Mark representing current allocation position
 */
nu_arena_mark nu_arena_get_mark(const nu_arena* arena);

/**
 * @brief Restore arena to previously marked position
 * @param arena Pointer to the arena
 * @param mark Mark to restore to
 */
void nu_arena_restore(nu_arena* arena, nu_arena_mark mark);

/**
 * @brief Reset arena to initial empty state
 * @param arena Pointer to the arena
 */
void nu_arena_reset(nu_arena* arena);

/**
 * @brief Get amount of memory currently used
 * @param arena Pointer to the arena
 * @return Number of bytes used
 */
size_t nu_arena_used(const nu_arena* arena);

/**
 * @brief Get amount of memory still available
 * @param arena Pointer to the arena
 * @return Number of bytes available
 */
size_t nu_arena_available(const nu_arena* arena);

#endif // NU_ARENA_H
