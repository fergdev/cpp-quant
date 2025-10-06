# -------- configurable knobs --------
PRESET     ?= conan-release
BUILD_DIR  ?= build
CONF       ?= Release
JOBS       ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# -------- paths to artifacts --------
BIN        ?= $(BUILD_DIR)/quant_engine
CCJSON     ?= $(BUILD_DIR)/compile_commands.json

# -------- phony targets --------
.PHONY: help install configure build all test run clean ccjson

help:
	@echo "Targets:"
	@echo "  install     - Conan profile detect + install deps into $(BUILD_DIR)"
	@echo "  configure   - CMake configure using preset $(PRESET)"
	@echo "  build/all   - Build using preset $(PRESET) with -j$(JOBS)"
	@echo "  test        - Run ctest from $(BUILD_DIR) (config=$(CONF))"
	@echo "  run         - Run built binary $(BIN)"
	@echo "  ccjson      - Symlink compile_commands.json to project root"
	@echo "  clean       - Remove $(BUILD_DIR)"

install:
	conan profile detect --force
	conan install . -s build_type=$(CONF) --build=missing

configure: install
	cmake --preset ${PRESET}

build all: configure
	cmake --build --preset conan-release -j

test: build
	ctest --test-dir $(BUILD_DIR)/$(CONF) --output-on-failure --output-junit ctest-junit.xml

run: build
	$(BIN)

ccjson: build
	@test -f "$(CCJSON)" && ln -sf "$(CCJSON)" ./compile_commands.json || \
	  (echo "compile_commands.json not found at $(CCJSON). Did configure succeed?" && exit 1)

clean:
	rm -rf $(BUILD_DIR) compile_commands.json
