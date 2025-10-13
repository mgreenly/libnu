CC := gcc

MULTIARCH_TUPLE := $(shell dpkg --print-architecture 2>/dev/null || echo "amd64")-linux-gnu

# # libs section
# PNG_NAME := png16
# PNG_LIBS := -l$(PNG_NAME)
# PNG_CFLAGS := $(shell pkg-config --cflags lib$(PNG_NAME) 2>/dev/null)

# JPEG_NAME := jpeg
# JPEG_LIBS := -l$(JPEG_NAME)
# JPEG_CFLAGS := $(shell pkg-config --cflags lib$(JPEG_NAME) 2>/dev/null)

# aggregated libs
LIB_DEPS_LIBS := # $(PNG_LIBS) $(JPEG_LIBS)
LIB_DEPS_CFLAGS := # $(PNG_CFLAGS) $(JPEG_CFLAGS)
LIB_DEPS_PC := #lib$(PNG_NAME) lib$(JPEG_NAME)

# aggregated pkg-config libs
PC_REQUIRES := $(LIB_DEPS_PC)

# test-only deps
# GLib no longer required
TEST_PC_REQUIRES :=

# All pkg-config packages we need to check for
CHECK_PC_PACKAGES := $(LIB_DEPS_PC)

# Optional tools to check for
CHECK_OPTIONAL_TOOLS := ctags cppcheck uncrustify clang

# compile flags
DISTRO_CFLAGS := -D_DEFAULT_SOURCE \
  -fanalyzer -fstack-clash-protection \
  -Wduplicated-cond -Wduplicated-branches \
  -Wformat-signedness -fdiagnostics-color \
  $(LIB_DEPS_CFLAGS)

# linker flags
DISTRO_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,--as-needed -pie \
  $(LIB_DEPS_LIBS)

# test-specific flags
TEST_CFLAGS := $(DISTRO_CFLAGS)
TEST_LDFLAGS := $(DISTRO_LDFLAGS)

DEV_PACKAGES := build-essential libpng-dev libjpeg-dev pkg-config cppcheck uncrustify clang universal-ctags
INSTALL_DEPS_CMD := sudo apt-get install $(DEV_PACKAGES)
