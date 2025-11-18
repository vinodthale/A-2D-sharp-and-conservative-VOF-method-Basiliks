#!/bin/bash
# Validation script for 2D axisymmetric simulation setup
# Checks current repository against best practices

echo "========================================="
echo "2D Axisymmetric Simulation Validation"
echo "========================================="
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0
WARNINGS=0

# Function to check files
check_file() {
    local file=$1
    local description=$2

    if [ -f "$file" ]; then
        echo -e "${GREEN}✓${NC} $description: $file"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}✗${NC} $description: $file NOT FOUND"
        ((FAILED++))
        return 1
    fi
}

# Function to check pattern in files
check_pattern() {
    local file=$1
    local pattern=$2
    local description=$3

    if grep -q "$pattern" "$file" 2>/dev/null; then
        echo -e "${GREEN}✓${NC} $description"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}✗${NC} $description NOT FOUND in $file"
        ((FAILED++))
        return 1
    fi
}

# Function for warnings
warn_pattern() {
    local file=$1
    local pattern=$2
    local description=$3

    if grep -q "$pattern" "$file" 2>/dev/null; then
        echo -e "${YELLOW}⚠${NC} WARNING: $description"
        ((WARNINGS++))
        return 1
    fi
    return 0
}

echo "1. COORDINATE SYSTEM CHECKS"
echo "----------------------------"

# Check for axi.h implementation
check_file "axi.h" "Axisymmetric coordinate file"

# Check for axisymmetric includes in main files
echo ""
echo "Checking simulation files for axisymmetric setup..."
for file in droplet-impact*.c; do
    if [ -f "$file" ]; then
        echo ""
        echo "File: $file"
        if grep -q "#include \"axi.h\"" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Uses axi.h"
            ((PASSED++))
        elif grep -q "#define AXISYM 1" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Uses AXISYM macro"
            ((PASSED++))
        else
            echo -e "${YELLOW}⚠${NC} No axisymmetric setup found"
            ((WARNINGS++))
        fi

        # Check origin
        if grep -q "origin(0[.]*[ ]*,[ ]*0[.]*)" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Correct origin(0., 0.)"
            ((PASSED++))
        else
            echo -e "${RED}✗${NC} origin(0., 0.) not found (CRITICAL for y ≥ 0)"
            ((FAILED++))
        fi

        # Check boundary conditions
        if grep -q "u.n\[left\].*=.*dirichlet(0" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Axis boundary: u.n[left] = dirichlet(0)"
            ((PASSED++))
        else
            echo -e "${RED}✗${NC} Missing axis boundary condition"
            ((FAILED++))
        fi

        # Check for volume calculation using dv()
        if grep -q "dv()" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Uses dv() for volume (includes 2πr)"
            ((PASSED++))
        fi

        # Warn about potential issues
        warn_pattern "$file" "origin(-" "Negative origin detected (may cause y < 0)"
        warn_pattern "$file" "sq(Delta)" "Manual volume calculation (should use dv())"
    fi
done

echo ""
echo "2. EMBEDDED GEOMETRY CHECKS"
echo "----------------------------"

for file in droplet-impact*.c; do
    if [ -f "$file" ]; then
        if grep -q "fractions(phi" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} $file: Uses fractions() for embedded geometry"
            ((PASSED++))
        fi

        if grep -q "vertex scalar phi" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} $file: Uses vertex scalar for level-set"
            ((PASSED++))
        fi
    fi
done

echo ""
echo "3. PHYSICAL SETUP CHECKS"
echo "------------------------"

for file in droplet-impact*.c; do
    if [ -f "$file" ]; then
        echo "File: $file"

        # Check fluid properties
        if grep -q "rho1.*=" "$file" 2>/dev/null && grep -q "rho2.*=" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Density properties defined"
            ((PASSED++))
        fi

        if grep -q "mu1.*=" "$file" 2>/dev/null && grep -q "mu2.*=" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Viscosity properties defined"
            ((PASSED++))
        fi

        if grep -q "f.sigma.*=" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Surface tension defined"
            ((PASSED++))
        fi

        # Check gravity
        if grep -q "face vector g\[\]" "$file" 2>/dev/null || grep -q "const face vector g" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} Gravity defined"
            ((PASSED++))
        fi

        echo ""
    fi
done

echo "4. TEST SUITE CHECKS"
echo "--------------------"

check_file "tests/validation/vof_advection/test_droplet_impact_validation.c" "Validation test file"
check_file "tests/run_all_tests.sh" "Test runner script"

echo ""
echo "5. DOCUMENTATION CHECKS"
echo "-----------------------"

check_file "AXISYMMETRIC_GUIDE.md" "Axisymmetric guide"
check_file "README.md" "README"
check_file "AXISYMMETRIC_COMPARISON_CHECKLIST.md" "Comparison checklist"

echo ""
echo "6. MESH AND REFINEMENT CHECKS"
echo "------------------------------"

for file in droplet-impact*.c; do
    if [ -f "$file" ]; then
        if grep -q "MAXLEVEL" "$file" 2>/dev/null; then
            maxlevel=$(grep "#define MAXLEVEL" "$file" | head -1 | awk '{print $3}')
            echo -e "${GREEN}✓${NC} $file: MAXLEVEL = $maxlevel"
            ((PASSED++))

            if [ "$maxlevel" -ge 10 ]; then
                echo -e "${GREEN}✓${NC} Good resolution (MAXLEVEL ≥ 10)"
                ((PASSED++))
            else
                echo -e "${YELLOW}⚠${NC} Low resolution (MAXLEVEL < 10)"
                ((WARNINGS++))
            fi
        fi

        if grep -q "adapt_wavelet" "$file" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} $file: Adaptive mesh refinement enabled"
            ((PASSED++))
        fi
    fi
done

echo ""
echo "========================================="
echo "VALIDATION SUMMARY"
echo "========================================="
echo -e "${GREEN}Passed:${NC} $PASSED"
echo -e "${RED}Failed:${NC} $FAILED"
echo -e "${YELLOW}Warnings:${NC} $WARNINGS"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All critical checks passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Some critical checks failed. Please review.${NC}"
    exit 1
fi
