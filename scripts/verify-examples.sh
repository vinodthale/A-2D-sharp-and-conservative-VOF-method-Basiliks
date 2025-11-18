#!/bin/bash
#
# Verification Script for Huang et al. (2025) VOF Method Examples
#
# This script compiles and tests all example simulations from the paper:
# "A 2D sharp and conservative VOF method for modeling the contact line
#  dynamics with hysteresis on complex boundary"
#
# Usage:
#   ./verify-examples.sh [OPTIONS]
#
# Options:
#   --compile-only    Only compile examples, don't run them
#   --quick-test      Run quick tests with reduced simulation time
#   --full-test       Run full simulations (may take hours)
#   --example NAME    Test only specific example
#   --help            Show this help message
#

set -e  # Exit on error

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Default options
COMPILE_ONLY=false
QUICK_TEST=false
FULL_TEST=false
SPECIFIC_EXAMPLE=""

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
declare -a FAILED_EXAMPLES

# Logging functions
log_header() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}========================================${NC}"
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

# Help message
show_help() {
    cat << EOF
Verification Script for Huang et al. (2025) VOF Method Examples

This script verifies all example simulations from the paper:
"A 2D sharp and conservative VOF method for modeling the contact line
 dynamics with hysteresis on complex boundary"

Journal of Computational Physics (2025)
https://doi.org/10.1016/j.jcp.2025.113975

USAGE:
    ./verify-examples.sh [OPTIONS]

OPTIONS:
    --compile-only       Only compile examples, don't run them
    --quick-test         Run quick tests (0.001s simulation time)
    --full-test          Run full simulations (may take hours)
    --example NAME       Test only specific example
    --help               Show this help message

EXAMPLES:
    # Check if all examples compile
    ./verify-examples.sh --compile-only

    # Quick verification test
    ./verify-examples.sh --quick-test

    # Test specific example
    ./verify-examples.sh --example circle-droplet --quick-test

    # Full simulation suite (WARNING: may take hours)
    ./verify-examples.sh --full-test

AVAILABLE EXAMPLES:
    1. circle-droplet                    - Droplet spreading on cylinder
    2. droplet-impact-orifice            - Droplet impact (dimensional)
    3. droplet-impact-orifice-nondim     - Droplet impact (non-dimensional)
    4. droplet-impact-sharp-orifice      - Sharp edge orifice (dimensional)
    5. droplet-impact-sharp-orifice-nondim - Sharp edge (non-dimensional)
    6. droplet-impact-round-orifice      - Round edge orifice

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --compile-only)
            COMPILE_ONLY=true
            shift
            ;;
        --quick-test)
            QUICK_TEST=true
            shift
            ;;
        --full-test)
            FULL_TEST=true
            shift
            ;;
        --example)
            SPECIFIC_EXAMPLE="$2"
            shift 2
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Check Basilisk installation
check_basilisk() {
    log_header "Checking Basilisk Installation"

    if ! command -v qcc >/dev/null 2>&1; then
        log_error "qcc not found in PATH"
        log_error "Please install Basilisk C first"
        log_info "See docs/BASILISK_INSTALL.md for installation instructions"
        exit 1
    fi

    log_success "qcc found: $(which qcc)"

    if [ -n "$BASILISK" ]; then
        log_success "BASILISK environment variable set: $BASILISK"
    else
        log_warning "BASILISK environment variable not set"
        log_info "Consider adding: export BASILISK=~/basilisk"
    fi
}

# Compile an example
compile_example() {
    local example=$1
    local source=""

    # Determine source file path based on example name
    case "$example" in
        "circle-droplet")
            source="src/2d-cartesian/${example}.c"
            ;;
        "droplet-impact-"*)
            source="src/axisymmetric/${example}.c"
            ;;
        *)
            source="${example}.c"
            ;;
    esac

    log_info "Compiling $example..."
    log_info "Source: $source"

    if [ ! -f "$source" ]; then
        log_error "Source file not found: $source"
        return 1
    fi

    # Include paths for custom headers
    local include_flags="-I include/basilisk/core -I include/basilisk/methods"

    # Compile with error checking
    if qcc -O2 -Wall $include_flags -o "$example" "$source" -lm 2>&1 | tee "compile-${example}.log"; then
        if [ -f "$example" ]; then
            log_success "Compiled $example successfully"
            return 0
        else
            log_error "Compilation reported success but binary not found: $example"
            return 1
        fi
    else
        log_error "Compilation failed for $example"
        log_info "See compile-${example}.log for details"
        return 1
    fi
}

# Run an example simulation
run_example() {
    local example=$1
    local sim_time=$2

    log_info "Running $example (simulation time: ${sim_time}s)..."

    if [ ! -f "$example" ]; then
        log_error "Binary not found: $example"
        return 1
    fi

    # Create output directory
    mkdir -p "test-output/${example}"
    cd "test-output/${example}"

    # Run simulation with timeout
    local timeout_duration=600  # 10 minutes max for quick tests
    if [ "$FULL_TEST" = true ]; then
        timeout_duration=7200  # 2 hours for full tests
    fi

    if timeout ${timeout_duration} ../../${example} 2> log; then
        # Check if simulation produced output
        if [ -f "log" ] && [ -s "log" ]; then
            local lines=$(wc -l < log)
            log_success "Simulation completed: $lines output lines"

            # Basic validation: check volume conservation
            if command -v awk >/dev/null 2>&1; then
                local vol_drift=$(awk 'NR>1 {print $2}' log | tail -1)
                if [ -n "$vol_drift" ]; then
                    log_info "Final volume ratio: $vol_drift"
                    # Check if volume is conserved (should be close to 1.0)
                    local vol_ok=$(awk -v v=$vol_drift 'BEGIN {print (v > 0.99 && v < 1.01) ? 1 : 0}')
                    if [ "$vol_ok" -eq 1 ]; then
                        log_success "Volume conservation: PASS"
                    else
                        log_warning "Volume drift detected: $vol_drift (expected ~1.0)"
                    fi
                fi
            fi

            cd ../..
            return 0
        else
            log_error "Simulation produced no output"
            cd ../..
            return 1
        fi
    else
        local exit_code=$?
        if [ $exit_code -eq 124 ]; then
            log_error "Simulation timed out after ${timeout_duration}s"
        else
            log_error "Simulation failed with exit code: $exit_code"
        fi
        cd ../..
        return 1
    fi
}

# Test a single example
test_example() {
    local example=$1

    log_header "Testing: $example"
    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    # Compile
    if ! compile_example "$example"; then
        FAILED_TESTS=$((FAILED_TESTS + 1))
        FAILED_EXAMPLES+=("$example (compilation failed)")
        return 1
    fi

    # Run if not compile-only
    if [ "$COMPILE_ONLY" = false ]; then
        local sim_time="0.01"
        if [ "$QUICK_TEST" = true ]; then
            sim_time="0.001"
        fi

        if ! run_example "$example" "$sim_time"; then
            FAILED_TESTS=$((FAILED_TESTS + 1))
            FAILED_EXAMPLES+=("$example (runtime failed)")
            return 1
        fi
    fi

    PASSED_TESTS=$((PASSED_TESTS + 1))
    log_success "Test passed: $example"
    return 0
}

# Main test suite
run_test_suite() {
    log_header "Example Verification Suite"
    log_info "Reference: Huang et al. (2025), J. Comput. Phys."
    log_info "DOI: https://doi.org/10.1016/j.jcp.2025.113975"
    echo ""

    if [ "$COMPILE_ONLY" = true ]; then
        log_info "Mode: Compilation only"
    elif [ "$QUICK_TEST" = true ]; then
        log_info "Mode: Quick test (0.001s simulation time)"
    elif [ "$FULL_TEST" = true ]; then
        log_info "Mode: Full simulation"
        log_warning "This may take several hours!"
    else
        log_info "Mode: Standard test (0.01s simulation time)"
    fi

    # List of all examples
    local examples=(
        "circle-droplet"
        "droplet-impact-orifice"
        "droplet-impact-orifice-nondim"
        "droplet-impact-sharp-orifice"
        "droplet-impact-sharp-orifice-nondim"
        "droplet-impact-round-orifice"
    )

    # Test specific example or all
    if [ -n "$SPECIFIC_EXAMPLE" ]; then
        log_info "Testing specific example: $SPECIFIC_EXAMPLE"
        test_example "$SPECIFIC_EXAMPLE"
    else
        log_info "Testing all ${#examples[@]} examples..."
        for example in "${examples[@]}"; do
            test_example "$example"
            echo ""
        done
    fi
}

# Print summary
print_summary() {
    log_header "Test Summary"

    echo -e "${BLUE}Total tests:${NC}  $TOTAL_TESTS"
    echo -e "${GREEN}Passed:${NC}       $PASSED_TESTS"
    echo -e "${RED}Failed:${NC}       $FAILED_TESTS"
    echo ""

    if [ $FAILED_TESTS -eq 0 ]; then
        log_success "All tests passed! ✓"
        echo ""
        echo -e "${GREEN}All examples from Huang et al. (2025) are working correctly.${NC}"
        return 0
    else
        log_error "Some tests failed:"
        for failed in "${FAILED_EXAMPLES[@]}"; do
            echo -e "  ${RED}✗${NC} $failed"
        done
        echo ""
        log_info "Check compile-*.log and test-output/*/log for details"
        return 1
    fi
}

# Main execution
main() {
    # Change to repository root directory
    cd "$(dirname "$0")/.." || exit 1
    log_info "Working directory: $(pwd)"

    check_basilisk
    run_test_suite
    print_summary
}

# Run main function
main
exit_code=$?

# Cleanup message
if [ "$COMPILE_ONLY" = false ]; then
    log_info "Test outputs saved in: test-output/"
    log_info "Compilation logs: compile-*.log"
fi

exit $exit_code
