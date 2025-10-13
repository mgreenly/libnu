#include "version.h"

const char*
nu_version (void)
{
  return NU_VERSION_STRING;
}

int
nu_version_major (void)
{
  return NU_VERSION_MAJOR;
}

int
nu_version_minor (void)
{
  return NU_VERSION_MINOR;
}

int
nu_version_patch (void)
{
  return NU_VERSION_PATCH;
}
