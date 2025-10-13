# Compiler selection (macOS default is clang)
CC := clang

# macOS doesn't use multiarch tuples - libraries go directly in lib/
MULTIARCH_TUPLE :=

# Library configurations (Homebrew paths)
ifneq (,$(wildcard /opt/homebrew/lib))
    # Apple Silicon Macs
    PNG_NAME := png
    PNG_LIBS := -L/opt/homebrew/lib -l$(PNG_NAME)
    PNG_CFLAGS := -I/opt/homebrew/include
    JPEG_NAME := jpeg
    JPEG_LIBS := -L/opt/homebrew/lib -l$(JPEG_NAME)
    JPEG_CFLAGS := -I/opt/homebrew/include
else
    # Intel Macs
    PNG_NAME := png
    PNG_LIBS := -L/usr/local/lib -l$(PNG_NAME)
    PNG_CFLAGS := -I/usr/local/include
    JPEG_NAME := jpeg
    JPEG_LIBS := -L/usr/local/lib -l$(JPEG_NAME)
    JPEG_CFLAGS := -I/usr/local/include
endif

# Aggregated runtime dependencies
LIB_DEPS_LIBS := $(PNG_LIBS) $(JPEG_LIBS)
LIB_DEPS_CFLAGS := $(PNG_CFLAGS) $(JPEG_CFLAGS)
LIB_DEPS_PC := lib$(PNG_NAME) lib$(JPEG_NAME)

# This is what goes into pkg-config
PC_REQUIRES := $(LIB_DEPS_PC)

# Test-only dependencies (not linked into the library)
# GLib no longer required
TEST_PC_REQUIRES :=

# All pkg-config packages we need to check for
CHECK_PC_PACKAGES := $(LIB_DEPS_PC)

# Optional tools to check for
CHECK_OPTIONAL_TOOLS := ctags uncrustify

# Compile flags for the library
DISTRO_CFLAGS := -D_DARWIN_C_SOURCE \
  -fcolor-diagnostics \
  -Wno-gnu-zero-variadic-macro-arguments \
  $(LIB_DEPS_CFLAGS)

# Linker flags for the library
DISTRO_LDFLAGS := -Wl,-dead_strip -Wl,-pie \
  $(LIB_DEPS_LIBS)

# Test-specific compile flags (includes test framework headers)
TEST_CFLAGS := $(DISTRO_CFLAGS)

# Test-specific linker flags (includes all libraries needed for tests)
TEST_LDFLAGS := $(DISTRO_LDFLAGS)

DEV_PACKAGES := libpng jpeg pkg-config uncrustify ctags
INSTALL_DEPS_CMD := xcode-select --install; brew install $(DEV_PACKAGES)
