# Makefile for VOF Method Test Suite
# 2D Sharp and Conservative VOF Method with Contact Line Dynamics

# Compiler
QCC = qcc
CFLAGS = -O2 -Wall
LDFLAGS = -lm

# Grid options
GRID_CART = -grid=cartesian
GRID_MULTI = -grid=multigrid

# Directories
UNIT_DIR = unit
INTEGRATION_DIR = integration
VALIDATION_DIR = validation
BENCHMARK_DIR = benchmarks
BUILD_DIR = build

# Create build directory
$(shell mkdir -p $(BUILD_DIR))

# Unit test sources
UNIT_MOMENTUM = $(UNIT_DIR)/momentum/test_momentum_equation.c
UNIT_VOF = $(UNIT_DIR)/vof_advection/test_vof_advection.c
UNIT_PLIC = $(UNIT_DIR)/interface_reconstruction/test_plic_reconstruction.c
UNIT_CURV = $(UNIT_DIR)/curvature/test_height_function_curvature.c
UNIT_CONTACT = $(UNIT_DIR)/contact_line/test_contact_line_algorithms.c

# Integration test sources
INT_PIPELINE = $(INTEGRATION_DIR)/vof_advection/test_full_vof_pipeline.c

# Validation test sources
VAL_DROPLET = $(VALIDATION_DIR)/vof_advection/test_droplet_impact_validation.c

# Benchmark sources
BENCH_RE = $(BENCHMARK_DIR)/vof_advection/benchmark_reynolds_sweep.c

# Executables
UNIT_EXECS = $(BUILD_DIR)/test_momentum \
             $(BUILD_DIR)/test_vof_advection \
             $(BUILD_DIR)/test_plic \
             $(BUILD_DIR)/test_curvature \
             $(BUILD_DIR)/test_contact_line

INT_EXECS = $(BUILD_DIR)/test_pipeline

VAL_EXECS = $(BUILD_DIR)/test_droplet_validation

BENCH_EXECS = $(BUILD_DIR)/benchmark_re_sweep

ALL_EXECS = $(UNIT_EXECS) $(INT_EXECS) $(VAL_EXECS) $(BENCH_EXECS)

# Default target
.PHONY: all
all: unit-tests integration-tests validation-tests benchmarks

# Build all tests
.PHONY: build-all
build-all: $(ALL_EXECS)

# Unit tests
.PHONY: unit-tests
unit-tests: $(UNIT_EXECS)
	@echo "========================================="
	@echo "Running Unit Tests"
	@echo "========================================="
	@for test in $(UNIT_EXECS); do \
		echo ""; \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo ""
	@echo "========================================="
	@echo "All Unit Tests PASSED"
	@echo "========================================="

# Integration tests
.PHONY: integration-tests
integration-tests: $(INT_EXECS)
	@echo "========================================="
	@echo "Running Integration Tests"
	@echo "========================================="
	@for test in $(INT_EXECS); do \
		echo ""; \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo ""
	@echo "========================================="
	@echo "All Integration Tests PASSED"
	@echo "========================================="

# Validation tests
.PHONY: validation-tests
validation-tests: $(VAL_EXECS)
	@echo "========================================="
	@echo "Running Validation Tests"
	@echo "========================================="
	@for test in $(VAL_EXECS); do \
		echo ""; \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo ""
	@echo "========================================="
	@echo "All Validation Tests PASSED"
	@echo "========================================="

# Benchmarks
.PHONY: benchmarks
benchmarks: $(BENCH_EXECS)
	@echo "========================================="
	@echo "Running Benchmarks"
	@echo "========================================="
	@for test in $(BENCH_EXECS); do \
		echo ""; \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo ""
	@echo "========================================="
	@echo "All Benchmarks Completed"
	@echo "========================================="

# Run all tests
.PHONY: test-all
test-all: unit-tests integration-tests validation-tests
	@echo ""
	@echo "========================================="
	@echo "ALL TESTS PASSED ✓"
	@echo "========================================="

# Build individual unit tests
$(BUILD_DIR)/test_momentum: $(UNIT_MOMENTUM)
	$(QCC) $(CFLAGS) $(GRID_CART) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_vof_advection: $(UNIT_VOF)
	$(QCC) $(CFLAGS) $(GRID_CART) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_plic: $(UNIT_PLIC)
	$(QCC) $(CFLAGS) $(GRID_CART) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_curvature: $(UNIT_CURV)
	$(QCC) $(CFLAGS) $(GRID_CART) $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_contact_line: $(UNIT_CONTACT)
	$(QCC) $(CFLAGS) $(GRID_CART) $< -o $@ $(LDFLAGS)

# Build integration tests
$(BUILD_DIR)/test_pipeline: $(INT_PIPELINE)
	$(QCC) $(CFLAGS) $(GRID_MULTI) $< -o $@ $(LDFLAGS)

# Build validation tests
$(BUILD_DIR)/test_droplet_validation: $(VAL_DROPLET)
	$(QCC) $(CFLAGS) $(GRID_MULTI) $< -o $@ $(LDFLAGS)

# Build benchmarks
$(BUILD_DIR)/benchmark_re_sweep: $(BENCH_RE)
	$(QCC) $(CFLAGS) $(GRID_MULTI) $< -o $@ $(LDFLAGS)

# Quick test (fast subset)
.PHONY: quick-test
quick-test:
	@echo "Running quick test suite..."
	$(MAKE) $(BUILD_DIR)/test_vof_advection
	$(BUILD_DIR)/test_vof_advection
	$(MAKE) $(BUILD_DIR)/test_plic
	$(BUILD_DIR)/test_plic
	@echo "Quick tests PASSED ✓"

# Clean
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f *.o *.s *.tst
	rm -f *~ core
	find . -name "*.pyc" -delete
	find . -name "__pycache__" -delete

# Help
.PHONY: help
help:
	@echo "VOF Method Test Suite"
	@echo "====================="
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build and run all tests (default)"
	@echo "  build-all        - Build all test executables"
	@echo "  unit-tests       - Run unit tests"
	@echo "  integration-tests- Run integration tests"
	@echo "  validation-tests - Run validation tests"
	@echo "  benchmarks       - Run performance benchmarks"
	@echo "  test-all         - Run all tests (unit + integration + validation)"
	@echo "  quick-test       - Run quick subset of tests"
	@echo "  clean            - Remove build files"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make unit-tests       # Run only unit tests"
	@echo "  make test-all         # Run all tests"
	@echo "  make quick-test       # Quick sanity check"
	@echo "  make benchmarks       # Run performance benchmarks"

# Specify YAML file as dependency
.PHONY: check-spec
check-spec:
	@if [ ! -f vof_method_specification.yaml ]; then \
		echo "ERROR: vof_method_specification.yaml not found"; \
		exit 1; \
	fi
	@echo "Specification file found ✓"

# Documentation
.PHONY: doc
doc:
	@echo "Test Suite Documentation"
	@echo "========================"
	@cat README.md

.PHONY: list-tests
list-tests:
	@echo "Available Tests:"
	@echo ""
	@echo "Unit Tests:"
	@echo "  - test_momentum_equation"
	@echo "  - test_vof_advection"
	@echo "  - test_plic_reconstruction"
	@echo "  - test_height_function_curvature"
	@echo "  - test_contact_line_algorithms"
	@echo ""
	@echo "Integration Tests:"
	@echo "  - test_full_vof_pipeline"
	@echo ""
	@echo "Validation Tests:"
	@echo "  - test_droplet_impact_validation"
	@echo ""
	@echo "Benchmarks:"
	@echo "  - benchmark_reynolds_sweep"
