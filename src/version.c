#include "version.h"
#include <stdint.h>

const char*
nu_version (void)
{
  return NU_VERSION_STRING;
}

uint32_t
nu_version_major (void)
{
  return NU_VERSION_MAJOR;
}

uint32_t
nu_version_minor (void)
{
  return NU_VERSION_MINOR;
}

uint32_t
nu_version_patch (void)
{
  return NU_VERSION_PATCH;
}
