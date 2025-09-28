#ifndef NULIB_H
#define NULIB_H

#include <stddef.h>

/* Include all module headers */
#include "sort.h"

/* Version information */
#define NULIB_VERSION_MAJOR 0
#define NULIB_VERSION_MINOR 1
#define NULIB_VERSION_PATCH 0

#define NULIB_VERSION_STRING "0.1.0"

/**
 * @brief Get the version string of the nulib library
 * @return Version string in the format "major.minor.patch"
 */
const char* nu_version(void);

/**
 * @brief Get the major version number
 * @return Major version number
 */
int nu_version_major(void);

/**
 * @brief Get the minor version number
 * @return Minor version number
 */
int nu_version_minor(void);

/**
 * @brief Get the patch version number
 * @return Patch version number
 */
int nu_version_patch(void);

#endif
