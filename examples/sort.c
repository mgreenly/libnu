/**
 * nu_sort Tutorial Example
 *
 * This example demonstrates how to use nu_sort with custom data types
 * and comparison functions. We'll sort an array of Person structs in
 * different ways to show the flexibility of the sorting interface.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <nu/sort.h>

/*
 * Step 1: Define your data structure
 *
 * nu_sort can sort any type of data. Here we'll use a Person struct
 * with multiple fields that we might want to sort by.
 */
typedef struct {
  char first_name[32];
  char last_name[32];
  int age;
  float salary;
} Person;

/*
 * Step 2: Create comparison functions
 *
 * The comparison function must follow the standard C convention:
 * - Return negative if a < b
 * - Return 0 if a == b
 * - Return positive if a > b
 */

// Simple comparison: sort by age (ascending)
static int
compare_by_age (const void* a, const void* b)
{
  const Person* p1 = (const Person*)a;
  const Person* p2 = (const Person*)b;
  return p1->age - p2->age;
}

// Reverse comparison: sort by salary (descending)
static int
compare_by_salary_desc (const void* a, const void* b)
{
  const Person* p1 = (const Person*)a;
  const Person* p2 = (const Person*)b;

  // Note: reversed order for descending sort
  if (p1->salary < p2->salary)return 1;
  if (p1->salary > p2->salary)return -1;
  return 0;
}

// Multi-field comparison: sort by last name, then first name
static int
compare_by_full_name (const void* a, const void* b)
{
  const Person* p1 = (const Person*)a;
  const Person* p2 = (const Person*)b;

  // First compare last names
  int last_cmp     = strcmp(p1->last_name, p2->last_name);
  if (last_cmp != 0) {
    return last_cmp;
  }

  // If last names are equal, compare first names
  return strcmp(p1->first_name, p2->first_name);
}

// Complex comparison: sort by age group, then by name within each group
static int
compare_by_age_group_and_name (const void* a, const void* b)
{
  const Person* p1 = (const Person*)a;
  const Person* p2 = (const Person*)b;

  // Define age groups: <30, 30-50, >50
  int group1       = (p1->age < 30) ? 0 : (p1->age <= 50) ? 1 : 2;
  int group2       = (p2->age < 30) ? 0 : (p2->age <= 50) ? 1 : 2;

  // First sort by age group
  if (group1 != group2) {
    return group1 - group2;
  }

  // Within same age group, sort by name
  return compare_by_full_name(a, b);
}

/*
 * Step 3: Helper function to display the data
 */
static void
print_people (const char* title, Person people[], size_t count)
{
  printf("\n%s:\n", title);
  printf("  %-20s %-15s %3s  %9s\n", "Name", "First", "Age", "Salary");
  printf("  %-20s %-15s %3s  %9s\n", "----", "-----", "---", "------");

  for (size_t i = 0; i < count; i++) {
    printf("  %-20s %-15s %3d  $%8.2f\n",
      people[i].last_name,
      people[i].first_name,
      people[i].age,
      (double)people[i].salary);
  }
}

int
main (void)
{
  printf("==============================================================\n");
  printf("                      nu_sort Tutorial\n");
  printf("==============================================================\n");

  /*
   * Step 4: Create sample data
   *
   * In real applications, this might come from a file, database, or user input
   */
  Person people[] = {
    {"Alice",   "Johnson",  28, 65000.00},
    {"Bob",     "Smith",    45, 85000.00},
    {"Charlie", "Brown",    32, 72000.00},
    {"Diana",   "Miller",   29, 68000.00},
    {"Edward",  "Davis",    51, 95000.00},
    {"Fiona",   "Wilson",   22, 55000.00},
    {"George",  "Anderson", 38, 78000.00},
    {"Helen",   "Taylor",   41, 82000.00},
    {"Ivan",    "Thomas",   35, 75000.00},
    {"Julia",   "Moore",    26, 62000.00},
    {"Kevin",   "Jackson",  47, 88000.00},
    {"Laura",   "White",    30, 70000.00},
    {"Michael", "Brown",    28, 66000.00},      // Note: same last name as Charlie
    {"Nancy",   "Davis",    33, 71000.00}       // Note: same last name as Edward
  };

  size_t count = sizeof(people) / sizeof(people[0]);

  // Show original data
  printf("\nORIGINAL DATA (unsorted):");
  print_people("Initial Order", people, count);

  /*
   * Example 1: Simple sort by age
   */
  printf("\n\nEXAMPLE 1: Sort by age (youngest to oldest)");
  printf("\n   Using: compare_by_age");

  nu_sort(people, count, sizeof(Person), compare_by_age);

  print_people("After sorting by age", people, count);
  printf("\n   Notice how people are now ordered by age ascending");

  /*
   * Example 2: Sort by salary (descending)
   */
  printf("\n\nEXAMPLE 2: Sort by salary (highest to lowest)");
  printf("\n   Using: compare_by_salary_desc");

  nu_sort(people, count, sizeof(Person), compare_by_salary_desc);

  print_people("After sorting by salary", people, count);
  printf("\n   Notice the descending order - highest salaries first");

  /*
   * Example 3: Multi-field sort (last name, then first name)
   */
  printf("\n\nEXAMPLE 3: Sort by full name (last, then first)");
  printf("\n   Using: compare_by_full_name");

  nu_sort(people, count, sizeof(Person), compare_by_full_name);

  print_people("After sorting by name", people, count);
  printf("\n   Notice how Browns and Davises are sub-sorted by first name");

  /*
   * Example 4: Complex sort with grouping
   */
  printf("\n\nEXAMPLE 4: Group by age bracket, then sort by name");
  printf("\n   Using: compare_by_age_group_and_name");
  printf("\n   Groups: Under 30 | 30-50 | Over 50");

  nu_sort(people, count, sizeof(Person), compare_by_age_group_and_name);

  print_people("After grouping and sorting", people, count);
  printf("\n   Notice three distinct age groups, each sorted by name");

  /*
   * Performance note
   */
  printf("\n\nPERFORMANCE NOTE:");
  printf("\n   nu_sort uses introsort, which guarantees:");
  printf("\n   - O(n log n) worst-case time complexity");
  printf("\n   - Excellent real-world performance");
  printf("\n   - Efficient handling of already-sorted data");

  /*
   * Key takeaways
   */
  printf("\n\nKEY TAKEAWAYS:");
  printf("\n   1. nu_sort works with any data type");
  printf("\n   2. You control sort order via comparison functions");
  printf("\n   3. Complex multi-field sorts are easy to implement");
  printf("\n   4. The same interface works for all sorting needs");

  printf("\n\nTutorial complete!\n\n");

  return 0;
}
