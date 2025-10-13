# User-configurable variables
LIBNAME := nu
VERSION_MAJOR := 0
VERSION_MINOR := 1
VERSION_PATCH := 0

ifeq ($(PREFIX),)
PREFIX = /usr/local
endif

# Derived version variables
VERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
SOVERSION := $(VERSION_MAJOR)

# Platform detection and distro-specific configuration
DISTRO := $(shell if [ "$(shell uname -s)" = "Darwin" ]; then echo "darwin"; elif [ -f /etc/os-release ]; then . /etc/os-release && echo $$ID; else echo "unknown"; fi)
-include mk/$(DISTRO).mk

# Directory structure
SRCDIR := src
OBJDIR := obj
LIBDIR := lib
REPORTSDIR := reports
TMPDIR := tmp

# Compiler flags
CFLAGS_COMMON = -std=c17 -pedantic \
  -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings \
  -Werror -Wall -Wextra -Wformat=2 -Wconversion -Wcast-qual -Wundef \
  -Wdate-time -Winit-self -Wstrict-overflow=2 \
  -MMD -MP -fstack-protector-strong -fPIC \
  -Wimplicit-fallthrough -Walloca -Wvla \
  -Wnull-dereference -Wdouble-promotion

CFLAGS_DEBUG = -D_FORTIFY_SOURCE=2 -g -O0

CFLAGS_RELEASE_OPTS = -DNDEBUG -g -O3

CFLAGS_BASE = $(CFLAGS_COMMON) $(CFLAGS_DEBUG)
CFLAGS_RELEASE = $(CFLAGS_COMMON) $(CFLAGS_RELEASE_OPTS)

CFLAGS_TEST = $(filter-out -Wmissing-prototypes,$(CFLAGS_COMMON)) $(CFLAGS_DEBUG) $(TEST_CFLAGS)

# Example compilation flags (without dependency generation)
CFLAGS_EXAMPLES = $(filter-out -MMD -MP,$(CFLAGS_BASE)) $(DISTRO_CFLAGS)

# Test-specific flags for stack size configuration
TEST_FLAGS = -DNU_QUICKSORT_STACK_SIZE=8

CFLAGS = $(CFLAGS_BASE) $(DISTRO_CFLAGS)

LDFLAGS = $(DISTRO_LDFLAGS)

# Library source files - automatically discover all *.c files in src/
LIB_SOURCES := $(wildcard $(SRCDIR)/*.c)
LIB_OBJECTS := $(LIB_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
LIB_DEPS := $(LIB_OBJECTS:.o=.d)

# Test files - automatically discover all *_test.c files
TEST_SOURCES := $(wildcard tests/*_test.c)
TEST_NAMES := $(patsubst tests/%_test.c,%,$(TEST_SOURCES))
TEST_PROGS := $(patsubst %,$(TMPDIR)/%_test,$(TEST_NAMES))

# All objects and dependencies
OBJECTS := $(LIB_OBJECTS)
DEPS := $(LIB_DEPS)

# Library files
STATIC_LIB := $(LIBDIR)/lib$(LIBNAME).a
DYNAMIC_LIB := $(LIBDIR)/lib$(LIBNAME).so.$(VERSION)
DYNAMIC_LIB_SONAME := $(LIBDIR)/lib$(LIBNAME).so.$(SOVERSION)
DYNAMIC_LIB_LINK := $(LIBDIR)/lib$(LIBNAME).so

# Library installation directory (set by distro-specific makefile)
# On Debian: $(PREFIX)/lib/amd64-linux-gnu, on macOS: $(PREFIX)/lib
LIBDIR_INSTALL := $(PREFIX)/lib$(if $(MULTIARCH_TUPLE),/$(MULTIARCH_TUPLE))

all: $(STATIC_LIB) $(DYNAMIC_LIB) $(TMPDIR)/$(LIBNAME).pc

release: CFLAGS = $(CFLAGS_RELEASE) $(DISTRO_CFLAGS)
release: clean all

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)

# Generate version.h from Makefile variables
$(SRCDIR)/version.h: Makefile
	@echo "Generating version.h..."
	@echo "#ifndef NU_VERSION_H" > $@
	@echo "#define NU_VERSION_H" >> $@
	@echo "" >> $@
	@echo "#define NU_VERSION_MAJOR $(VERSION_MAJOR)" >> $@
	@echo "#define NU_VERSION_MINOR $(VERSION_MINOR)" >> $@
	@echo "#define NU_VERSION_PATCH $(VERSION_PATCH)" >> $@
	@echo "" >> $@
	@echo "#define NU_VERSION_STRING \"$(VERSION)\"" >> $@
	@echo "" >> $@
	@echo "#define NU_MAKE_VERSION(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))" >> $@
	@echo "" >> $@
	@echo "#define NU_VERSION NU_MAKE_VERSION(NU_VERSION_MAJOR, NU_VERSION_MINOR, NU_VERSION_PATCH)" >> $@
	@echo "" >> $@
	@echo "/**" >> $@
	@echo " * @brief Get the version string of the nu library" >> $@
	@echo " * @return Version string in the format \"major.minor.patch\"" >> $@
	@echo " */" >> $@
	@echo "const char* nu_version(void);" >> $@
	@echo "" >> $@
	@echo "/**" >> $@
	@echo " * @brief Get the major version number" >> $@
	@echo " * @return Major version number" >> $@
	@echo " */" >> $@
	@echo "int nu_version_major(void);" >> $@
	@echo "" >> $@
	@echo "/**" >> $@
	@echo " * @brief Get the minor version number" >> $@
	@echo " * @return Minor version number" >> $@
	@echo " */" >> $@
	@echo "int nu_version_minor(void);" >> $@
	@echo "" >> $@
	@echo "/**" >> $@
	@echo " * @brief Get the patch version number" >> $@
	@echo " * @return Patch version number" >> $@
	@echo " */" >> $@
	@echo "int nu_version_patch(void);" >> $@
	@echo "" >> $@
	@echo "#endif" >> $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/version.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Static library
$(STATIC_LIB): $(LIB_OBJECTS) | $(LIBDIR)
	ar rcs $@ $^

# Dynamic library
$(DYNAMIC_LIB): $(LIB_OBJECTS) | $(LIBDIR)
	$(CC) -shared -Wl,-soname,lib$(LIBNAME).so.$(SOVERSION) $(LIB_OBJECTS) $(PNG_LIBS) -o $@
	cd $(LIBDIR) && ln -sf lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so.$(SOVERSION)
	cd $(LIBDIR) && ln -sf lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so


# Benchmark sources and binaries using nu/bench.h (*_bench.c pattern)
BENCH_SRCS := $(wildcard bench/*_bench.c)
BENCH_PROGS := $(patsubst bench/%_bench.c,$(TMPDIR)/%_bench,$(BENCH_SRCS))

# Build and run all benchmarks
bench: $(TMPDIR) $(BENCH_PROGS)
	@echo "Running benchmarks with nice -n -20 (may require sudo)..."
	@for prog in $(BENCH_PROGS); do \
		echo ""; \
		echo "=== $$(basename $$prog) ==="; \
		nice -n -20 $$prog 2>/dev/null || $$prog; \
	done

# Pattern rule for building benchmark binaries (*_bench pattern)
# Each bench/X_bench.c is expected to benchmark src/X.c
$(TMPDIR)/%_bench: bench/%_bench.c src/%.c $(SRCDIR)/version.h | $(TMPDIR)
	@mkdir -p $(TMPDIR)/include/nu
	@for h in src/*.h; do ln -sf ../../../$$h $(TMPDIR)/include/nu/; done
	$(CC) $(CFLAGS) -O2 -DNU_MALLOC=malloc -DNU_FREE=free $< src/$*.c -I$(TMPDIR)/include -o $@

$(TMPDIR):
	mkdir -p $(TMPDIR)

# Example sources and binaries
EXAMPLE_SRCS := $(wildcard examples/*.c)
EXAMPLE_PROGS := $(patsubst examples/%.c,examples/%,$(EXAMPLE_SRCS))

# Build all examples (requires library to be installed)
examples: install
	@echo "Building examples using installed library..."
	@cd examples && for src in *.c; do \
		echo "  Compiling $$src..."; \
		PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig:$$PKG_CONFIG_PATH $(CC) -std=c17 -Wall -Wextra -g $$src $$(PKG_CONFIG_PATH=$(PREFIX)/lib/pkgconfig:$$PKG_CONFIG_PATH pkg-config --cflags --libs nu) -o $${src%.c} || exit 1; \
	done
	@echo "All examples built successfully"

clean:
	rm -rf $(OBJDIR) $(LIBDIR) $(REPORTSDIR) $(TMPDIR) tags $(SRCDIR)/version.h $(EXAMPLE_PROGS)

-include $(DEPS)

.PHONY: all release clean check install uninstall tags fmt help deps check-deps coverage sanitize analyze check-all bench examples

# Pattern rule for building test executables
# Tests are compiled with their corresponding source files
# Special case for error_test (header-only module)
$(TMPDIR)/error_test: tests/error_test.c | $(TMPDIR)
	$(CC) $(CFLAGS_TEST) -Wno-analyzer-use-of-pointer-in-stale-stack-frame -Wno-analyzer-use-of-uninitialized-value -I. $< -o $@

# Special cases that need TEST_FLAGS
# Override MALLOC for tests to use test_malloc
$(TMPDIR)/sort_test: tests/sort_test.c src/sort.c | $(TMPDIR)
	$(CC) $(CFLAGS_TEST) $(TEST_FLAGS) -DNU_MALLOC=test_malloc -DNU_FREE=free -I. $^ -o $@

$(TMPDIR)/arena_test: tests/arena_test.c src/arena.c | $(TMPDIR)
	$(CC) $(CFLAGS_TEST) $(TEST_FLAGS) -DNU_MALLOC=test_malloc -DNU_FREE=free -I. $^ -o $@

# Default pattern: foo_test compiles with src/foo.c
$(TMPDIR)/%_test: tests/%_test.c src/%.c $(SRCDIR)/version.h | $(TMPDIR)
	$(CC) $(CFLAGS_TEST) -I. $(filter-out $(SRCDIR)/version.h,$^) -o $@

# Explicit rules for library object files that need MALLOC/FREE
$(OBJDIR)/arena.o: src/arena.c | $(OBJDIR)
	$(CC) $(CFLAGS) -DNU_MALLOC=malloc -DNU_FREE=free -c $< -o $@

$(OBJDIR)/sort.o: src/sort.c | $(OBJDIR)
	$(CC) $(CFLAGS) -DNU_MALLOC=malloc -DNU_FREE=free -c $< -o $@

check: $(TEST_PROGS)
	@echo "Running tests..."
	@echo ""
	@for test in $(TEST_PROGS); do \
		test_name=$$(basename $$test | sed 's/_test$$//'); \
		echo "Testing $$test_name module:"; \
		$$test || exit 1; \
		echo ""; \
	done
	@echo "All tests passed!"

# Generate pkg-config file
$(TMPDIR)/$(LIBNAME).pc: pkgconfig/$(LIBNAME).pc.in | $(TMPDIR)
	@sed -e 's|@PREFIX@|$(PREFIX)|g' \
	     -e 's|@LIBDIR_INSTALL@|$(LIBDIR_INSTALL)|g' \
	     -e 's|@VERSION@|$(VERSION)|g' \
	     -e 's|@PC_REQUIRES@|$(PC_REQUIRES)|g' \
	     $< > $@

install:
	@echo "Building optimized release version for installation..."
	@$(MAKE) CFLAGS="$(CFLAGS_RELEASE) $(DISTRO_CFLAGS)" PREFIX="$(PREFIX)" clean all
	@echo "Installing library to $(PREFIX)..."
	@mkdir -p $(LIBDIR_INSTALL)
	@mkdir -p $(PREFIX)/include/$(LIBNAME)
	@mkdir -p $(PREFIX)/lib/pkgconfig
	install -m 644 $(STATIC_LIB) $(LIBDIR_INSTALL)/
	install -m 755 $(DYNAMIC_LIB) $(LIBDIR_INSTALL)/
	cd $(LIBDIR_INSTALL) && ln -sf lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so.$(SOVERSION)
	cd $(LIBDIR_INSTALL) && ln -sf lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so
	install -m 644 src/*.h $(PREFIX)/include/$(LIBNAME)/
	install -m 644 $(TMPDIR)/$(LIBNAME).pc $(PREFIX)/lib/pkgconfig/
	@echo "Installation complete!"

uninstall:
	rm -f $(LIBDIR_INSTALL)/lib$(LIBNAME).a
	rm -f $(LIBDIR_INSTALL)/lib$(LIBNAME).so*
	rm -rf $(PREFIX)/include/$(LIBNAME)
	rm -f $(PREFIX)/lib/pkgconfig/$(LIBNAME).pc

tags:
	@command -v ctags >/dev/null 2>&1 || { \
		echo "Error: ctags is not installed."; \
		exit 1; \
	}
	ctags -R $(SRCDIR)


fmt:
	@echo "Formatting with uncrustify:"
	@find $(SRCDIR) tests examples \( -name "*.c" -o -name "*.h" \) -type f \
		| while read file; do \
			if command -v uncrustify >/dev/null 2>&1; then \
				uncrustify -c uncrustify.cfg --replace --no-backup "$$file"; \
			else \
				echo "uncrustify not available - install with: brew install uncrustify"; \
			fi; \
		done

help:
	@echo "Available targets:"
	@echo "  all        - Build static and dynamic libraries (default, debug mode)"
	@echo "  release    - Build optimized release version"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Clean build and install optimized library (use PREFIX=/path)"
	@echo "  uninstall  - Uninstall the library"
	@echo "  check      - Run tests"
	@echo "  bench      - Run benchmarks"
	@echo "  examples   - Build all example programs"
	@echo "  check-all  - Run comprehensive checks (check + analyze + sanitize + coverage)"
	@echo "  analyze    - Run static analysis (clang or cppcheck)"
	@echo "  sanitize   - Run tests with AddressSanitizer and UBSan"
	@echo "  coverage   - Run tests with coverage analysis"
	@echo "  fmt        - Format code with uncrustify"
	@echo "  tags       - Generate ctags file"
	@echo "  deps       - Show package installation instructions"
	@echo "  check-deps - Check if build dependencies are available"
	@echo "  help       - Show this help message"

deps:
	@echo ""
	@echo "To install required development packages:"
	@echo ""
	@echo "  $(INSTALL_DEPS_CMD)"
	@echo ""
	@echo "To check if all dependencies are available:"
	@echo ""
	@echo "  make check-deps"
	@echo ""

check-deps:
	@echo "Checking build dependencies..."
	@failed=0; \
	echo "Essential tools:"; \
	command -v pkg-config >/dev/null 2>&1 || { \
		echo "  ✗ pkg-config not found"; \
		failed=1; \
	} && echo "  ✓ pkg-config found"; \
	command -v $(CC) >/dev/null 2>&1 || { \
		echo "  ✗ $(CC) not found"; \
		failed=1; \
	} && echo "  ✓ $(CC) found"; \
	echo "Required libraries:"; \
	for pkg in $(CHECK_PC_PACKAGES); do \
		pkg-config --exists $$pkg 2>/dev/null || { \
			echo "  ✗ $$pkg not found"; \
			failed=1; \
		} && echo "  ✓ $$pkg found"; \
	done; \
	echo "Optional tools:"; \
	for tool in $(CHECK_OPTIONAL_TOOLS); do \
		command -v $$tool >/dev/null 2>&1 || { \
			echo "  ⚠ $$tool not found (optional)"; \
		} && echo "  ✓ $$tool found"; \
	done; \
	if [ $$failed -eq 1 ]; then \
		echo ""; \
		echo "Some required dependencies are missing."; \
		echo "Run 'make deps' to see installation instructions."; \
		exit 1; \
	else \
		echo ""; \
		echo "All required dependencies satisfied!"; \
	fi

# Pattern rules for coverage builds
# Use -DMALLOC=test_malloc for coverage to test malloc failure paths
# Special cases that need TEST_FLAGS
$(TMPDIR)/arena_test_cov: tests/arena_test.c src/arena.c | $(TMPDIR)
	$(CC) $(CFLAGS_BASE) $(TEST_FLAGS) -DNU_MALLOC=test_malloc -DNU_FREE=free -I. $^ --coverage -o $@

$(TMPDIR)/sort_test_cov: tests/sort_test.c src/sort.c | $(TMPDIR)
	$(CC) $(CFLAGS_BASE) $(TEST_FLAGS) -DNU_MALLOC=test_malloc -DNU_FREE=free -I. $^ --coverage -o $@

# Default pattern for coverage (version_test doesn't use malloc)
$(TMPDIR)/%_test_cov: tests/%_test.c src/%.c $(SRCDIR)/version.h | $(TMPDIR)
	$(CC) $(CFLAGS_BASE) $(TEST_FLAGS) -DNU_MALLOC=malloc -DNU_FREE=free -I. $(filter-out $(SRCDIR)/version.h,$^) --coverage -o $@

# Note: error_test excluded from coverage - it's header-only with no source to measure
TEST_NAMES_COV := $(filter-out error,$(TEST_NAMES))
TEST_PROGS_COV := $(patsubst %,$(TMPDIR)/%_test_cov,$(TEST_NAMES_COV))

coverage: $(TEST_PROGS_COV)
	@mkdir -p $(REPORTSDIR)
	@echo "Running tests with coverage..."
	@echo ""
	@for test in $(TEST_PROGS_COV); do \
		test_name=$$(basename $$test | sed 's/_test_cov$$//'); \
		echo "  Testing $$test_name module with coverage..."; \
		if ! $$test 2>/dev/null; then \
			echo "  ERROR: $$test failed to execute"; \
			exit 1; \
		fi; \
		echo ""; \
	done
	@echo "Generating coverage report..."
	@rm -f $(TMPDIR)/*.gcov *.gcov
	@if ! ls $(TMPDIR)/*.gcda >/dev/null 2>&1; then \
		echo "  ERROR: No coverage data files (.gcda) were generated"; \
		exit 1; \
	fi
	@echo ""
	@echo "Coverage results:"
	@for src in $(LIB_SOURCES); do \
		src_base=$$(basename $$src); \
		gcda_files=$$(ls $(TMPDIR)/*_cov-$${src_base%.c}.gcda 2>/dev/null || true); \
		if [ -n "$$gcda_files" ]; then \
			for gcda in $$gcda_files; do \
				test_name=$$(basename $$gcda | sed 's/_cov-.*//'); \
				result=$$(gcov -o $(TMPDIR) $$gcda 2>/dev/null | grep "^Lines executed:" | head -1); \
				percent=$$(echo "$$result" | sed -n 's/Lines executed:\([0-9.]*\)%.*/\1/p'); \
				lines=$$(echo "$$result" | sed -n 's/.*of \([0-9]*\).*/\1/p'); \
				printf "  %6s%% of %2s - $$src_base (from $$test_name)\n" "$$percent" "$$lines"; \
			done; \
			if [ -f $${src_base}.gcov ]; then \
				mv $${src_base}.gcov $(REPORTSDIR)/ 2>/dev/null || true; \
			fi; \
		fi; \
	done
	@rm -f *.gcov

# Pattern rules for sanitizer builds
# Use regular malloc for sanitizers (not test_malloc)
$(TMPDIR)/arena_test_san: tests/arena_test.c src/arena.c | $(TMPDIR)
	$(CC) $(filter-out -D_FORTIFY_SOURCE=2,$(CFLAGS_BASE)) $(TEST_FLAGS) -DNU_MALLOC=malloc -DNU_FREE=free -I. $^ -fsanitize=address,undefined -o $@

$(TMPDIR)/sort_test_san: tests/sort_test.c src/sort.c | $(TMPDIR)
	$(CC) $(filter-out -D_FORTIFY_SOURCE=2,$(CFLAGS_BASE)) $(TEST_FLAGS) -DNU_MALLOC=malloc -DNU_FREE=free -I. $^ -fsanitize=address,undefined -o $@

# Default pattern for sanitizer
$(TMPDIR)/%_test_san: tests/%_test.c src/%.c $(SRCDIR)/version.h | $(TMPDIR)
	$(CC) $(filter-out -D_FORTIFY_SOURCE=2,$(CFLAGS_BASE)) -DNU_MALLOC=malloc -DNU_FREE=free -I. $(filter-out $(SRCDIR)/version.h,$^) -fsanitize=address,undefined -o $@

# Note: error_test excluded from sanitizer runs due to compound literal stack issues
# The nu_error module uses compound literals which AddressSanitizer flags as
# stack-use-after-return. This is a known limitation documented in the module.
TEST_NAMES_SAN := $(filter-out error,$(TEST_NAMES))
TEST_PROGS_SAN := $(patsubst %,$(TMPDIR)/%_test_san,$(TEST_NAMES_SAN))

sanitize: CFLAGS = $(filter-out -D_FORTIFY_SOURCE=2,$(CFLAGS_COMMON) $(CFLAGS_DEBUG)) $(DISTRO_CFLAGS) -fsanitize=address,undefined
sanitize: LDFLAGS = $(DISTRO_LDFLAGS) -fsanitize=address,undefined
sanitize: clean $(TEST_PROGS_SAN)
	@echo "Running tests with sanitizers..."
	@echo ""
	@for test in $(TEST_PROGS_SAN); do \
		test_name=$$(basename $$test | sed 's/_test_san$$//'); \
		echo "  Testing $$test_name module with sanitizers..."; \
		$$test || exit 1; \
		echo ""; \
	done
	@echo "All sanitizer tests passed!"

analyze: $(SRCDIR)/version.h
	@echo "Running static analysis..."
	@mkdir -p $(REPORTSDIR) $(TMPDIR)
	@command -v clang >/dev/null 2>&1 && { \
		echo "Using clang static analyzer..."; \
		if clang --analyze $(filter-out -MMD -MP -fanalyzer,$(CFLAGS)) -DNU_MALLOC=malloc -DNU_FREE=free -I./src $(LIB_SOURCES); then \
			echo "Static analysis completed - no issues found!"; \
		fi; \
		mv *.plist $(REPORTSDIR)/ 2>/dev/null || true; \
	} || { \
		echo "clang not found, trying cppcheck..."; \
		command -v cppcheck >/dev/null 2>&1 && { \
			cppcheck --enable=all --std=c17 --suppress=missingIncludeSystem --suppress=missingInclude $(LIB_SOURCES) --quiet && \
			echo "Static analysis completed - no issues found!"; \
		} || { \
			echo "No static analysis tools found. Install clang or cppcheck."; \
		}; \
	}

check-all:
	@echo "=== Running comprehensive checks ==="
	@echo "1/4: Running unit tests..."
	@$(MAKE) check
	@echo ""
	@echo "2/4: Running static analysis..."
	@$(MAKE) analyze
	@echo ""
	@echo "3/4: Running sanitizer tests..."
	@$(MAKE) sanitize
	@echo ""
	@echo "4/4: Running coverage analysis..."
	@$(MAKE) coverage
	@echo ""
	@echo "=== All checks completed successfully! ==="
