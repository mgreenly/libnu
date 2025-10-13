/**
 * nu_arena Tutorial Example
 *
 * This example demonstrates the nu_arena allocator, a stack-based allocator
 * with mark/restore functionality. Arena allocators are perfect for temporary
 * allocations that have a clear lifetime and can be freed all at once.
 */

#include <stdio.h>
#include <string.h>
#include <nu/arena.h>

/*
 * Example 1: Basic Arena Usage
 *
 * Shows how to initialize an arena and make simple allocations.
 */
static void
basic_usage_example (void)
{
  printf("=== Basic Arena Usage ===\n");

  // Arena allocators need a backing buffer
  char buffer[1024];
  nu_arena arena;

  // Initialize the arena with the buffer
  if (!nu_arena_init(&arena, buffer, sizeof(buffer))) {
    printf("Failed to initialize arena\n");
    return;
  }

  printf("Arena initialized with %zu bytes\n", nu_arena_available(&arena));

  // Allocate some memory
  int* numbers = (int* )nu_arena_alloc(&arena, sizeof(int) * 10);
  if (!numbers) {
    printf("Allocation failed\n");
    return;
  }

  // Use the allocated memory
  for (int i = 0; i < 10; i++) {
    numbers[i] = i * i;
  }

  printf("Allocated array of 10 integers\n");
  printf("Arena now has %zu bytes used, %zu bytes available\n",
    nu_arena_used(&arena), nu_arena_available(&arena));

  // Allocate a string
  char* message = (char* )nu_arena_alloc(&arena, 50);
  if (message) {
    strcpy(message, "Hello from the arena!");
    printf("Message: %s\n", message);
  }

  // No need to free individual allocations!
  // When done, just reset the arena or let it go out of scope
  nu_arena_reset(&arena);
  printf("Arena reset - all memory reclaimed\n\n");
}

/*
 * Example 2: Mark and Restore Pattern
 *
 * Shows how to use marks to create temporary allocation scopes.
 * This is useful for functions that need temporary memory.
 */
static void
mark_restore_example (void)
{
  printf("=== Mark and Restore Pattern ===\n");

  char buffer[2048];
  nu_arena arena;
  nu_arena_init(&arena, buffer, sizeof(buffer));

  // Simulate processing multiple items with temporary allocations
  for (int item = 1; item <= 3; item++) {
    // Mark the current position
    nu_arena_mark mark = nu_arena_get_mark(&arena);
    printf("Processing item %d (arena at %zu bytes)\n", item, nu_arena_used(&arena));

    // Make temporary allocations for this item
    char* temp_buffer = (char* )nu_arena_alloc(&arena, 256);
    sprintf(temp_buffer, "Temporary data for item %d", item);

    int* temp_array   = (int* )nu_arena_alloc(&arena, sizeof(int) * 50);
    for (int i = 0; i < 50; i++) {
      temp_array[i] = item * i;
    }

    printf("  Created temporary data: %s\n", temp_buffer);
    printf("  Arena now using %zu bytes\n", nu_arena_used(&arena));

    // Process the item (actual work would happen here)

    // Restore to mark - frees all temporary allocations for this item
    nu_arena_restore(&arena, mark);
    printf("  Restored arena to %zu bytes\n\n", nu_arena_used(&arena));
  }
}

/*
 * Example 3: Aligned Allocations
 *
 * Shows how to allocate memory with specific alignment requirements.
 * This is useful for SIMD operations or hardware requirements.
 */
static void
aligned_allocation_example (void)
{
  printf("=== Aligned Allocations ===\n");

  char buffer[1024];
  nu_arena arena;
  nu_arena_init(&arena, buffer, sizeof(buffer));

  // Allocate with different alignments
  void* ptr8  = nu_arena_alloc_aligned(&arena, 32, 8);
  void* ptr16 = nu_arena_alloc_aligned(&arena, 32, 16);
  void* ptr32 = nu_arena_alloc_aligned(&arena, 32, 32);

  printf("8-byte aligned:  %p (address %% 8 = %lu)\n",
    ptr8, (unsigned long)ptr8 % 8);
  printf("16-byte aligned: %p (address %% 16 = %lu)\n",
    ptr16, (unsigned long)ptr16 % 16);
  printf("32-byte aligned: %p (address %% 32 = %lu)\n\n",
    ptr32, (unsigned long)ptr32 % 32);
}

/*
 * Example 4: Building a Tree Structure
 *
 * Shows how arena allocation simplifies complex data structures.
 * No need to free individual nodes - just reset the arena when done!
 */
typedef struct TreeNode {
  int value;
  struct TreeNode* left;
  struct TreeNode* right;
} TreeNode;

static TreeNode*
create_node (nu_arena* arena, int value)
{
  TreeNode* node = (TreeNode* )nu_arena_alloc(arena, sizeof(TreeNode));
  if (node) {
    node->value = value;
    node->left  = NULL;
    node->right = NULL;
  }
  return node;
}

static void
print_tree (TreeNode* node, int depth)
{
  if (!node)return;

  for (int i = 0; i < depth; i++)printf("  ");
  printf("%d\n", node->value);

  print_tree(node->left, depth + 1);
  print_tree(node->right, depth + 1);
}

static void
tree_example (void)
{
  printf("=== Building a Tree with Arena ===\n");

  char buffer[4096];
  nu_arena arena;
  nu_arena_init(&arena, buffer, sizeof(buffer));

  // Build a simple binary tree
  TreeNode* root = create_node(&arena, 10);
  root->left         = create_node(&arena, 5);
  root->right        = create_node(&arena, 15);
  root->left->left   = create_node(&arena, 3);
  root->left->right  = create_node(&arena, 7);
  root->right->left  = create_node(&arena, 12);
  root->right->right = create_node(&arena, 20);

  printf("Created binary tree:\n");
  print_tree(root, 0);

  printf("\nTotal arena memory used: %zu bytes\n", nu_arena_used(&arena));
  printf("No need to free individual nodes!\n\n");

  // Reset frees the entire tree at once
  nu_arena_reset(&arena);
}

/*
 * Example 5: Performance Comparison
 *
 * Compare arena allocation speed with malloc/free.
 * Arena allocation is typically much faster for temporary allocations.
 */
static void
performance_example (void)
{
  printf("=== Performance Benefits ===\n");

  const int iterations           = 1000;
  const int allocs_per_iteration = 10;

  // Arena version
  char buffer[8192];
  nu_arena arena;
  nu_arena_init(&arena, buffer, sizeof(buffer));

  printf("Arena allocation: %d iterations, %d allocations each\n",
    iterations, allocs_per_iteration);

  for (int i = 0; i < iterations; i++) {
    nu_arena_mark mark = nu_arena_get_mark(&arena);

    for (int j = 0; j < allocs_per_iteration; j++) {
      void* ptr = nu_arena_alloc(&arena, 64);
      if (!ptr)break;
      // Use the memory
      memset(ptr, i + j, 64);
    }

    nu_arena_restore(&arena, mark); // Fast bulk deallocation
  }

  printf("Completed - arena allocation is typically 10-100x faster\n");
  printf("than malloc/free for this pattern!\n\n");
}

int
main (void)
{
  printf("nu_arena Examples\n");
  printf("==================\n\n");

  basic_usage_example();
  mark_restore_example();
  aligned_allocation_example();
  tree_example();
  performance_example();

  printf("Key Benefits of Arena Allocation:\n");
  printf("- No memory fragmentation\n");
  printf("- Very fast allocation (just pointer bump)\n");
  printf("- Bulk deallocation with reset or restore\n");
  printf("- No need to track individual allocations\n");
  printf("- Perfect for temporary/scoped allocations\n");

  return 0;
}
