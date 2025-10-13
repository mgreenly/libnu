/**
 * nu version API example
 *
 * This example demonstrates how to use the version functions
 * to get information about the library version at runtime.
 */

#include <stdio.h>
#include <nu/version.h>

int
main (void)
{
  printf("libnu Version Information\n");
  printf("=========================\n\n");

  // Get the full version string
  printf("Version string: %s\n", nu_version());

  // Get individual version components
  printf("Major version:  %d\n", nu_version_major());
  printf("Minor version:  %d\n", nu_version_minor());
  printf("Patch version:  %d\n", nu_version_patch());

  printf("\nVersion check at compile time:\n");
  printf("NU_VERSION_STRING: %s\n", NU_VERSION_STRING);
  printf("NU_VERSION_MAJOR:  %d\n", NU_VERSION_MAJOR);
  printf("NU_VERSION_MINOR:  %d\n", NU_VERSION_MINOR);
  printf("NU_VERSION_PATCH:  %d\n", NU_VERSION_PATCH);

  // Example of version checking
  printf("\nVersion checking example:\n");
  if (nu_version_major() >= 1) {
    printf("This is version 1.0 or later\n");
  } else {
    printf("This is a pre-1.0 version (%d.%d.%d)\n",
      nu_version_major(),
      nu_version_minor(),
      nu_version_patch());
  }

  // Using the NU_VERSION macro for compile-time checks
  printf("\nCompiled version code: %d\n", NU_VERSION);
  printf("(calculated as major*10000 + minor*100 + patch)\n");

  return 0;
}
