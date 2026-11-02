
CMAKE ?= cmake
BUILD_DIR ?= build
CMAKE_ARGS ?=

.PHONY: all check clean test_tables_remote_class

all: test_tables_remote_class

test_tables_remote_class:
	$(CMAKE) -S . -B "$(BUILD_DIR)" -DVSCP_BUILD_TESTS=OFF -DVSCP_INSTALL=OFF $(CMAKE_ARGS)
	$(CMAKE) --build "$(BUILD_DIR)" --target test_tables_remote_class

check: test_tables_remote_class
	$(CMAKE) --build "$(BUILD_DIR)" --target check_tables_remote_class

clean:
	@if test -f "$(BUILD_DIR)/CMakeCache.txt"; then \
		$(CMAKE) --build "$(BUILD_DIR)" --target clean; \
	fi
