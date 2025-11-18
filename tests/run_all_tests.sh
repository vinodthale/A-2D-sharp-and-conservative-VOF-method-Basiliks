#!/bin/bash
#
# run_all_tests.sh
# Automated test runner for VOF method test suite
#

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=================================================${NC}"
echo -e "${BLUE}VOF Method Test Suite - Automated Runner${NC}"
echo -e "${BLUE}=================================================${NC}"
echo ""

# Check if qcc is available
if ! command -v qcc &> /dev/null; then
    echo -e "${RED}Error: qcc (Basilisk compiler) not found${NC}"
    echo "Please install Basilisk C and ensure qcc is in your PATH"
    exit 1
fi

echo -e "${GREEN}✓ Basilisk compiler found${NC}"

# Check for YAML specification
if [ ! -f "vof_method_specification.yaml" ]; then
    echo -e "${RED}Error: vof_method_specification.yaml not found${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Specification file found${NC}"
echo ""

# Parse command line arguments
RUN_UNIT=1
RUN_INTEGRATION=1
RUN_VALIDATION=1
RUN_BENCHMARKS=0  # Disabled by default (time-consuming)

while [[ $# -gt 0 ]]; do
    case $1 in
        --unit-only)
            RUN_INTEGRATION=0
            RUN_VALIDATION=0
            shift
            ;;
        --quick)
            RUN_INTEGRATION=0
            RUN_VALIDATION=0
            RUN_BENCHMARKS=0
            shift
            ;;
        --with-benchmarks)
            RUN_BENCHMARKS=1
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --unit-only        Run only unit tests"
            echo "  --quick            Quick test (unit tests only)"
            echo "  --with-benchmarks  Include performance benchmarks"
            echo "  --help             Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Initialize counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test category
run_test_category() {
    local category=$1
    echo ""
    echo -e "${BLUE}=================================================${NC}"
    echo -e "${BLUE}Running $category${NC}"
    echo -e "${BLUE}=================================================${NC}"
    echo ""

    if make $category; then
        echo -e "${GREEN}✓ $category PASSED${NC}"
        return 0
    else
        echo -e "${RED}✗ $category FAILED${NC}"
        return 1
    fi
}

# Run unit tests
if [ $RUN_UNIT -eq 1 ]; then
    if run_test_category "unit-tests"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
fi

# Run integration tests
if [ $RUN_INTEGRATION -eq 1 ]; then
    if run_test_category "integration-tests"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
fi

# Run validation tests
if [ $RUN_VALIDATION -eq 1 ]; then
    if run_test_category "validation-tests"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
fi

# Run benchmarks
if [ $RUN_BENCHMARKS -eq 1 ]; then
    if run_test_category "benchmarks"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
fi

# Summary
echo ""
echo -e "${BLUE}=================================================${NC}"
echo -e "${BLUE}Test Suite Summary${NC}"
echo -e "${BLUE}=================================================${NC}"
echo ""
echo "Total test categories: $TOTAL_TESTS"
echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"

if [ $FAILED_TESTS -gt 0 ]; then
    echo -e "${RED}Failed: $FAILED_TESTS${NC}"
else
    echo -e "${GREEN}Failed: $FAILED_TESTS${NC}"
fi

echo ""

# Final result
if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}=================================================${NC}"
    echo -e "${GREEN}ALL TESTS PASSED ✓${NC}"
    echo -e "${GREEN}=================================================${NC}"
    exit 0
else
    echo -e "${RED}=================================================${NC}"
    echo -e "${RED}SOME TESTS FAILED ✗${NC}"
    echo -e "${RED}=================================================${NC}"
    exit 1
fi
