#include "arena.h"
#include <stdint.h>
#include <string.h>

extern void* NU_MALLOC(size_t size);
extern void NU_FREE(void* ptr);

static size_t
align_forward (size_t ptr, size_t alignment)
{
  size_t mask = alignment - 1;
  return (ptr + mask) & ~mask;
}

bool
nu_arena_init (nu_arena* arena, void* buffer, size_t capacity)
{
  if (!arena || !buffer || capacity == 0) {
    return false;
  }

  arena->buffer   = (char* )buffer;
  arena->capacity = capacity;
  arena->used     = 0;
  return true;
}

void*
nu_arena_alloc (nu_arena* arena, size_t size)
{
  if (!arena || size == 0) {
    return NULL;
  }

  if (arena->used + size > arena->capacity) {
    return NULL;
  }

  void* ptr = arena->buffer + arena->used;
  arena->used += size;
  return ptr;
}

void*
nu_arena_alloc_aligned (nu_arena* arena, size_t size, size_t alignment)
{
  if (!arena || size == 0 || alignment == 0) {
    return NULL;
  }

  // Check if alignment is power of 2
  if ((alignment & (alignment - 1)) != 0) {
    return NULL;
  }

  size_t current = (size_t)(arena->buffer + arena->used);
  size_t aligned = align_forward(current, alignment);
  size_t offset  = aligned - (size_t)arena->buffer;

  if (offset + size > arena->capacity) {
    return NULL;
  }

  arena->used = offset + size;
  return (void* )aligned;
}

nu_arena_mark
nu_arena_get_mark (const nu_arena* arena)
{
  nu_arena_mark mark = {0};
  if (arena) {
    mark.position = arena->used;
  }
  return mark;
}

void
nu_arena_restore (nu_arena* arena, nu_arena_mark mark)
{
  if (arena && mark.position <= arena->capacity) {
    arena->used = mark.position;
  }
}

void
nu_arena_reset (nu_arena* arena)
{
  if (arena) {
    arena->used = 0;
  }
}

size_t
nu_arena_used (const nu_arena* arena)
{
  return arena ? arena->used : 0;
}

size_t
nu_arena_available (const nu_arena* arena)
{
  return arena ? (arena->capacity - arena->used) : 0;
}
