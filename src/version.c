#include "version.h"

const char*
nu_version (void)
{
  return NULIB_VERSION_STRING;
}

int
nu_version_major (void)
{
  return NULIB_VERSION_MAJOR;
}

int
nu_version_minor (void)
{
  return NULIB_VERSION_MINOR;
}

int
nu_version_patch (void)
{
  return NULIB_VERSION_PATCH;
}
