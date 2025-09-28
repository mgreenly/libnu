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

# Test-specific flags for malloc testing and other test configurations
TEST_FLAGS = -DQUICKSORT_STACK_SIZE=8 -DMALLOC=test_malloc

CFLAGS = $(CFLAGS_BASE) $(DISTRO_CFLAGS)

LDFLAGS = $(DISTRO_LDFLAGS)

# Library source files
LIB_SOURCES := $(SRCDIR)/nulib.c $(SRCDIR)/sort.c
LIB_OBJECTS := $(LIB_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
LIB_DEPS := $(LIB_OBJECTS:.o=.d)

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
	@echo "#ifndef NULIB_VERSION_H" > $@
	@echo "#define NULIB_VERSION_H" >> $@
	@echo "" >> $@
	@echo "#define NULIB_VERSION_MAJOR $(VERSION_MAJOR)" >> $@
	@echo "#define NULIB_VERSION_MINOR $(VERSION_MINOR)" >> $@
	@echo "#define NULIB_VERSION_PATCH $(VERSION_PATCH)" >> $@
	@echo "" >> $@
	@echo "#define NULIB_VERSION_STRING \"$(VERSION)\"" >> $@
	@echo "" >> $@
	@echo "#define NULIB_MAKE_VERSION(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))" >> $@
	@echo "" >> $@
	@echo "#define NULIB_VERSION NULIB_MAKE_VERSION(NULIB_VERSION_MAJOR, NULIB_VERSION_MINOR, NULIB_VERSION_PATCH)" >> $@
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


# Benchmark sources and binaries
BENCH_TEST_SRCS := $(filter-out bench/bench_utils.c, $(wildcard bench/bench_*.c))
BENCH_UTILS := $(TMPDIR)/bench_utils.o
BENCH_PROGS := $(patsubst bench/bench_%.c,$(TMPDIR)/bench_%,$(BENCH_TEST_SRCS))

# Build and run all benchmarks
bench: $(TMPDIR) $(BENCH_PROGS)
	@echo "Running benchmarks with nice -n -20 (may require sudo)..."
	@for prog in $(BENCH_PROGS); do \
		echo ""; \
		echo "=== $$(basename $$prog) ==="; \
		nice -n -20 $$prog 2>/dev/null || $$prog; \
	done

# Pattern rule for building benchmark binaries
$(TMPDIR)/bench_%: $(TMPDIR)/bench_%.o $(BENCH_UTILS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile benchmark object files
$(TMPDIR)/bench_%.o: bench/bench_%.c | $(TMPDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

# Compile benchmark utils
$(TMPDIR)/bench_utils.o: bench/bench_utils.c | $(TMPDIR)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

$(TMPDIR):
	mkdir -p $(TMPDIR)

# Example sources and binaries
EXAMPLE_SRCS := $(wildcard examples/*.c)
EXAMPLE_PROGS := $(patsubst examples/%.c,examples/%,$(EXAMPLE_SRCS))

# Build all examples
examples: $(STATIC_LIB) $(EXAMPLE_PROGS)
	@echo "All examples built successfully"

# Pattern rule for building example binaries
examples/%: examples/%.c $(STATIC_LIB)
	$(CC) $(CFLAGS_EXAMPLES) $< -I$(SRCDIR) $(STATIC_LIB) -o $@

clean:
	rm -rf $(OBJDIR) $(LIBDIR) $(REPORTSDIR) $(TMPDIR) tags $(SRCDIR)/version.h $(EXAMPLE_PROGS)

-include $(DEPS)

.PHONY: all release clean check install uninstall tags fmt help deps check-deps coverage sanitize analyze check-all bench examples

check:
	@mkdir -p $(TMPDIR)
	@echo "Running tests..."
	@echo ""
	@echo "Testing nulib module:"
	$(CC) $(CFLAGS_BASE) -I. tests/nulib_test.c $(LIB_SOURCES) -o $(TMPDIR)/nulib_test
	$(TMPDIR)/nulib_test
	@echo ""
	@echo "Testing sort module:"
	$(CC) $(CFLAGS_BASE) $(TEST_FLAGS) -I. tests/sort_test.c tests/test_malloc.c $(LIB_SOURCES) -o $(TMPDIR)/sort_test
	$(TMPDIR)/sort_test
	@echo ""

# Generate pkg-config file
$(TMPDIR)/$(LIBNAME).pc: pkgconfig/$(LIBNAME).pc.in | $(TMPDIR)
	PREFIX="$(PREFIX)" LIBDIR_INSTALL="$(LIBDIR_INSTALL)" VERSION="$(VERSION)" PC_REQUIRES="$(PC_REQUIRES)" \
	envsubst '$$PREFIX $$LIBDIR_INSTALL $$VERSION $$PC_REQUIRES' < $< > $@

install: $(STATIC_LIB) $(DYNAMIC_LIB) $(TMPDIR)/$(LIBNAME).pc
	@mkdir -p $(LIBDIR_INSTALL)
	@mkdir -p $(PREFIX)/include/$(LIBNAME)
	@mkdir -p $(PREFIX)/lib/pkgconfig
	install -m 644 $(STATIC_LIB) $(LIBDIR_INSTALL)/
	install -m 755 $(DYNAMIC_LIB) $(LIBDIR_INSTALL)/
	cd $(LIBDIR_INSTALL) && ln -sf lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so.$(SOVERSION)
	cd $(LIBDIR_INSTALL) && ln -sf lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so
	install -m 644 src/$(LIBNAME).h $(PREFIX)/include/$(LIBNAME)/
	install -m 644 src/version.h $(PREFIX)/include/$(LIBNAME)/
	install -m 644 $(TMPDIR)/$(LIBNAME).pc $(PREFIX)/lib/pkgconfig/

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
	@find $(SRCDIR) tests \( -name "*.c" -o -name "*.h" \) -type f \
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
	@echo "  install    - Install the library (use PREFIX=/path to specify location)"
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

require-glib:
	@pkg-config --exists glib-2.0 || { \
		echo "Error: GLib 2.0 not found. Please install libglib2.0-dev"; \
		echo "Run: $(INSTALL_DEPS_CMD)"; \
		exit 1; \
	}

coverage:
	@mkdir -p $(REPORTSDIR) $(TMPDIR)
	@echo "Running tests with coverage..."
	@echo ""
	@echo "  Testing nulib module with coverage..."
	$(CC) $(CFLAGS_BASE) -I. tests/nulib_test.c $(LIB_SOURCES) --coverage -o $(TMPDIR)/nulib_test_cov
	@if ! $(TMPDIR)/nulib_test_cov; then \
		echo "  ERROR: nulib_test_cov failed to execute"; \
		echo "  Check that the executable can run on your system"; \
		exit 1; \
	fi
	@echo ""
	@echo "  Testing sort module with coverage..."
	$(CC) $(CFLAGS_BASE) $(TEST_FLAGS) -I. tests/sort_test.c tests/test_malloc.c $(LIB_SOURCES) --coverage -o $(TMPDIR)/sort_test_cov
	@if ! $(TMPDIR)/sort_test_cov; then \
		echo "  ERROR: sort_test_cov failed to execute"; \
		echo "  Check that the executable can run on your system"; \
		exit 1; \
	fi
	@echo ""
	@echo "Generating coverage report..."
	@rm -f $(TMPDIR)/*.gcov *.gcov
	@if ! ls $(TMPDIR)/*.gcda >/dev/null 2>&1; then \
		echo "  ERROR: No coverage data files (.gcda) were generated"; \
		echo "  This means the test executables did not run properly"; \
		exit 1; \
	fi
	@echo ""
	@echo "Coverage results:"
	@result=$$(gcov -o $(TMPDIR) $(TMPDIR)/nulib_test_cov-nulib.gcda 2>/dev/null | grep "^Lines executed:" | head -1); \
	percent=$$(echo "$$result" | sed -n 's/Lines executed:\([0-9.]*\)%.*/\1/p'); \
	lines=$$(echo "$$result" | sed -n 's/.*of \([0-9]*\).*/\1/p'); \
	printf "  %6s%% of %2s - nulib.c (from nulib_test)\n" "$$percent" "$$lines"
	@if [ -f nulib.c.gcov ]; then \
		mv nulib.c.gcov $(REPORTSDIR)/ 2>/dev/null || true; \
	fi
	@rm -f *.gcov
	@result=$$(gcov -o $(TMPDIR) $(TMPDIR)/sort_test_cov-sort.gcda 2>/dev/null | grep "^Lines executed:" | tail -1); \
	percent=$$(echo "$$result" | sed -n 's/Lines executed:\([0-9.]*\)%.*/\1/p'); \
	lines=$$(echo "$$result" | sed -n 's/.*of \([0-9]*\).*/\1/p'); \
	printf "  %6s%% of %2s - sort.c (from sort_test)\n" "$$percent" "$$lines"
	@if [ -f sort.c.gcov ]; then \
		mv sort.c.gcov $(REPORTSDIR)/ 2>/dev/null || true; \
	fi
	@rm -f *.gcov

sanitize: CFLAGS = $(filter-out -D_FORTIFY_SOURCE=2,$(CFLAGS_COMMON) $(CFLAGS_DEBUG)) $(DISTRO_CFLAGS) -fsanitize=address,undefined
sanitize: LDFLAGS = $(DISTRO_LDFLAGS) -fsanitize=address,undefined
sanitize: clean
	@mkdir -p $(TMPDIR)
	@echo "Running tests with sanitizers..."
	@echo ""
	@echo "  Testing nulib module with sanitizers..."
	$(CC) $(filter-out -D_FORTIFY_SOURCE=2,$(CFLAGS_BASE)) -I. tests/nulib_test.c $(LIB_SOURCES) -fsanitize=address,undefined -o $(TMPDIR)/nulib_test_san
	$(TMPDIR)/nulib_test_san
	@echo ""
	@echo "  Testing sort module with sanitizers..."
	$(CC) $(filter-out -D_FORTIFY_SOURCE=2,$(CFLAGS_BASE)) $(TEST_FLAGS) -I. tests/sort_test.c tests/test_malloc.c $(LIB_SOURCES) -fsanitize=address,undefined -o $(TMPDIR)/sort_test_san
	$(TMPDIR)/sort_test_san
	@echo ""
	@echo "All sanitizer tests passed!"

analyze:
	@echo "Running static analysis..."
	@mkdir -p $(REPORTSDIR) $(TMPDIR)
	@command -v clang >/dev/null 2>&1 && { \
		echo "Using clang static analyzer..."; \
		if clang --analyze $(filter-out -MMD -MP -fanalyzer,$(CFLAGS)) -I./src src/nulib.c src/sort.c; then \
			echo "Static analysis completed - no issues found!"; \
		fi; \
		mv *.plist $(REPORTSDIR)/ 2>/dev/null || true; \
	} || { \
		echo "clang not found, trying cppcheck..."; \
		command -v cppcheck >/dev/null 2>&1 && { \
			cppcheck --enable=all --std=c17 --suppress=missingIncludeSystem --suppress=missingInclude src/*.c --quiet && \
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
