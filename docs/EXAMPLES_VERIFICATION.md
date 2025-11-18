# Examples Verification Guide

## Overview

This document describes the example simulations from the paper by Huang et al. (2025) and provides instructions for verifying that all examples compile and run correctly.

**Reference:**
> Huang, C.-S., Han, T.-Y., Zhang, J., & Ni, M.-J. (2025). "A 2D sharp and conservative VOF method for modeling the contact line dynamics with hysteresis on complex boundary." *Journal of Computational Physics*. https://doi.org/10.1016/j.jcp.2025.113975

## Example Simulations Inventory

The repository contains **6 example simulations** demonstrating different aspects of the sharp and conservative VOF method:

### 1. Circle Droplet (`circle-droplet.c`)

**Description:** Droplet spreading on a cylindrical surface of similar size

**Key Features:**
- 2D Cartesian coordinates with mirror symmetry
- Embedded boundary method for cylinder representation
- Contact line dynamics with adjustable contact angle
- Demonstrates contact angle hysteresis on curved boundaries

**Parameters:**
- Contact angle: 120° (adjustable via `thetac`)
- Droplet radius: 0.2/200 length units
- Cylinder radius: 0.201191/200 length units
- Maximum refinement level: 6
- Simulation time: 0.001 (short test simulation)

**What it validates:**
- VOF method on curved embedded boundaries
- Contact angle implementation
- Volume conservation
- Adaptive mesh refinement near interfaces

---

### 2. Droplet Impact on Orifice - Dimensional (`droplet-impact-orifice.c`)

**Description:** Axisymmetric droplet impact on a flat plate with circular orifice (hole)

**Key Features:**
- Axisymmetric coordinates (cylindrical symmetry)
- Flat plate with circular orifice geometry
- Adjustable impact velocity and contact angle
- Full physical units (SI)

**Parameters:**
- Droplet radius: 1 mm
- Impact velocity: -1.0 m/s (downward)
- Orifice radius: 0.4 mm (40% of droplet radius)
- Contact angle: 90°
- Fluid properties: water/air
- Simulation time: 0.01 s

**What it validates:**
- Axisymmetric VOF method
- Droplet impact dynamics
- Penetration through orifice
- Contact line dynamics on flat surfaces

---

### 3. Droplet Impact on Orifice - Non-dimensional (`droplet-impact-orifice-nondim.c`)

**Description:** Same as #2 but with non-dimensional formulation

**Key Features:**
- Non-dimensional scaling (length, time, velocity)
- Characterized by dimensionless numbers (Re, We, Bo)
- More general formulation for parameter studies

**What it validates:**
- Non-dimensional VOF implementation
- Scaling laws for droplet impact
- Comparison with dimensional version

---

### 4. Droplet Impact on Sharp Orifice (`droplet-impact-sharp-orifice.c`)

**Description:** Droplet impact on plate with sharp-edged orifice

**Key Features:**
- Sharp edge geometry (no rounding)
- Contact line pinning at sharp edges
- Contact angle hysteresis (advancing/receding angles)
- Higher resolution mesh

**Parameters:**
- Droplet diameter: 10.307 mm
- Orifice diameter: 6 mm (d/D = 0.58)
- Plate thickness: 2 mm
- Receding angle: θ_r = 42°
- Advancing angle: θ_a = 68°
- Sharp edge pinning angle: 150°
- Bond number: Bo = 6.0
- Mesh resolution: Δ = 3.91×10⁻² mm (~120 cells per radius)

**What it validates:**
- Contact line pinning at sharp edges
- Contact angle hysteresis
- Detailed contact line dynamics
- High-resolution simulations

---

### 5. Droplet Impact on Sharp Orifice - Non-dimensional (`droplet-impact-sharp-orifice-nondim.c`)

**Description:** Non-dimensional version of #4

**What it validates:**
- Non-dimensional formulation with contact angle hysteresis
- Sharp edge pinning in non-dimensional framework

---

### 6. Droplet Impact on Round Orifice (`droplet-impact-round-orifice.c`)

**Description:** Droplet impact on plate with rounded orifice edge

**Key Features:**
- Rounded edge modeled as semicircle
- No contact with plate (θ_s = 180°)
- Different contact line behavior vs sharp edge

**Parameters:**
- Droplet diameter: 9.315 mm
- Orifice diameter: 6 mm (d/D = 0.644)
- Plate thickness: 2 mm
- Round edge radius: 2 mm (same as plate thickness)
- Contact angle: θ_s = 180° (no wetting)
- Bond number: Bo = 4.9

**What it validates:**
- Contact line dynamics on rounded geometries
- Comparison with sharp edge case
- Non-wetting boundary conditions

---

## Verification Prerequisites

### 1. Basilisk C Installation

All examples require Basilisk C to be installed and configured:

```bash
# Check if Basilisk is installed
which qcc

# Check environment variable
echo $BASILISK

# If not installed, see BASILISK_INSTALL.md
```

### 2. System Requirements

**Minimum:**
- GCC or Clang compiler
- GNU Make
- 4 GB RAM
- 1 GB disk space for outputs

**Recommended:**
- 16 GB RAM for high-resolution simulations
- Multi-core CPU for faster compilation
- FFmpeg for video generation (optional)

---

## Verification Methods

### Method 1: Automated Verification Script (Recommended)

Use the provided `verify-examples.sh` script:

```bash
# Quick verification (all examples, 0.001s simulation time)
./verify-examples.sh --quick-test

# Compile-only check (fastest)
./verify-examples.sh --compile-only

# Test specific example
./verify-examples.sh --example circle-droplet --quick-test

# Full simulation suite (WARNING: may take hours)
./verify-examples.sh --full-test
```

**Output:**
- Compilation logs: `compile-*.log`
- Simulation outputs: `test-output/*/log`
- Summary of passed/failed tests

---

### Method 2: Manual Verification Using Makefile

```bash
# Compile all examples
make all

# Compile specific example
make circle-droplet

# High-resolution build
make high-res MAXLEVEL=10

# Check Basilisk installation
make check-basilisk

# Clean and rebuild
make clean
make all
```

---

### Method 3: Individual Manual Testing

#### Step 1: Compile

```bash
qcc -O2 -Wall -o circle-droplet circle-droplet.c -lm
```

#### Step 2: Run

```bash
./circle-droplet 2> log
```

#### Step 3: Verify Output

Check the log file for volume conservation:

```bash
# View log file
cat log

# Expected format:
# time  volume_ratio  [other data...]

# Volume ratio should stay close to 1.0 (conserved)
awk '{print $1, $2}' log
```

#### Step 4: Validate Results

**Success criteria:**
1. Simulation completes without errors
2. Volume ratio stays within 0.99-1.01 (1% tolerance)
3. Output files generated (log, movie.mp4, field-*)

---

## Expected Test Results

### Compilation Test

All 6 examples should compile successfully:

```
✓ circle-droplet
✓ droplet-impact-orifice
✓ droplet-impact-orifice-nondim
✓ droplet-impact-sharp-orifice
✓ droplet-impact-sharp-orifice-nondim
✓ droplet-impact-round-orifice
```

### Runtime Test (Quick)

With 0.001s simulation time, each example should:
- Complete in < 1 minute
- Produce log file with volume data
- Maintain volume conservation (V/V₀ ≈ 1.0)

### Volume Conservation Check

The second column in the log file (`V/V₀`) should remain close to 1.0:

```
# Example output from circle-droplet
0.0000000000000000 1.0000000000000000
0.0000033333333333 0.9999998234567890
0.0000066666666667 0.9999997123456789
...
```

**Acceptable range:** 0.99 - 1.01 (±1% for short tests)

For longer simulations, stricter conservation is expected (< 0.1% drift).

---

## Troubleshooting

### Issue: `qcc: command not found`

**Solution:** Basilisk is not installed or not in PATH

```bash
# Install Basilisk
./setup-basilisk.sh

# Or add to PATH
export BASILISK=$HOME/basilisk
export PATH=$PATH:$BASILISK
```

---

### Issue: Compilation errors with missing headers

**Problem:** Standard Basilisk headers not found (e.g., `navier-stokes/centered.h`)

**Solution:** Ensure BASILISK environment variable is set:

```bash
export BASILISK=$HOME/basilisk
```

The custom headers (axi.h, myembed.h, embed_*.h) are included in this repository and should be found automatically.

---

### Issue: Simulation hangs or takes too long

**For quick tests:**
- Use `--quick-test` mode (0.001s simulation time)
- Reduce MAXLEVEL in source file
- Use simpler example (circle-droplet is fastest)

**For production runs:**
- Normal behavior - full simulations can take hours
- Use background execution: `./example 2> log &`
- Monitor progress: `tail -f log`

---

### Issue: Volume conservation poor (V/V₀ far from 1.0)

**Possible causes:**
1. Simulation time too short (transients)
2. Mesh resolution too low (increase MAXLEVEL)
3. Numerical instability (reduce time step)

**Check:**
```bash
# Plot volume evolution
gnuplot -e "plot 'log' using 1:2 with lines; pause -1"
```

---

### Issue: No movie.mp4 generated

**Cause:** FFmpeg not installed or view.h not available

**Solution:**
- Install FFmpeg: `sudo apt install ffmpeg`
- Not critical for validation (log file is sufficient)
- Comment out movie generation in source if needed

---

## Example Test Output

### Successful Quick Test

```
========================================
Example Verification Suite
========================================
[INFO] Reference: Huang et al. (2025), J. Comput. Phys.
[INFO] Mode: Quick test (0.001s simulation time)

========================================
Testing: circle-droplet
========================================
[INFO] Compiling circle-droplet...
[PASS] Compiled circle-droplet successfully
[INFO] Running circle-droplet (simulation time: 0.001s)...
[PASS] Simulation completed: 45 output lines
[INFO] Final volume ratio: 0.9999998234
[PASS] Volume conservation: PASS
[PASS] Test passed: circle-droplet

[... similar output for other examples ...]

========================================
Test Summary
========================================
Total tests:  6
Passed:       6
Failed:       0

[PASS] All tests passed! ✓

All examples from Huang et al. (2025) are working correctly.
```

---

## Current Status

**As of this verification:**

⚠️ **Basilisk C is not currently installed in this environment** due to network restrictions preventing external downloads.

**To verify examples:**

1. **Install Basilisk C** (see [BASILISK_INSTALL.md](BASILISK_INSTALL.md)):
   ```bash
   ./setup-basilisk.sh
   ```

2. **Run verification script:**
   ```bash
   ./verify-examples.sh --quick-test
   ```

**All example source files are present and ready:**
- ✓ circle-droplet.c
- ✓ droplet-impact-orifice.c
- ✓ droplet-impact-orifice-nondim.c
- ✓ droplet-impact-sharp-orifice.c
- ✓ droplet-impact-sharp-orifice-nondim.c
- ✓ droplet-impact-round-orifice.c

**All custom headers are present:**
- ✓ axi.h (axisymmetric coordinates)
- ✓ myembed.h (embedded boundaries)
- ✓ embed_contact.h (contact line dynamics)
- ✓ embed_two-phase.h (two-phase flow)
- ✓ embed_tension.h (surface tension)
- ✓ embed_vof.h (VOF advection)
- ✓ embed_curvature.h (curvature calculation)
- ✓ embed_heights.h (height function method)
- ✓ embed_height_normal.h (normal calculation)
- ✓ embed_correct_height.h (height correction)
- ✓ embed_iforce.h (interfacial forces)
- ✓ TPR2D.h (two-phase reconstruction)
- ✓ tmp_fraction_field.h (temporary field storage)

---

## Paper Example Coverage

The 6 simulations in this repository cover the main test cases presented in Huang et al. (2025):

### Covered Cases:

1. **Droplet spreading on curved boundary** (circle-droplet)
   - Validates: curved geometry, contact angles, VOF method

2. **Droplet impact through orifice** (droplet-impact-orifice variants)
   - Validates: axisymmetric formulation, impact dynamics, penetration

3. **Contact line pinning at sharp edges** (droplet-impact-sharp-orifice)
   - Validates: sharp geometry, contact angle hysteresis, pinning

4. **Rounded vs sharp edge comparison** (droplet-impact-round-orifice)
   - Validates: edge curvature effects, different boundary conditions

5. **Non-dimensional formulation** (nondim variants)
   - Validates: scaling laws, dimensionless parameters

### Typical Paper Validation Metrics:

✓ Volume conservation (V/V₀ ≈ 1.0)
✓ Contact angle accuracy
✓ Interface sharpness
✓ Convergence with mesh refinement
✓ Comparison with experiments/benchmarks

---

## References

1. Huang, C.-S., Han, T.-Y., Zhang, J., & Ni, M.-J. (2025). "A 2D sharp and conservative VOF method for modeling the contact line dynamics with hysteresis on complex boundary." *Journal of Computational Physics*. https://doi.org/10.1016/j.jcp.2025.113975

2. Basilisk C: http://basilisk.fr/

3. Original sandbox: https://basilisk.dalembert.upmc.fr/sandbox/Chongsen/

---

## Contact and Issues

For issues with:
- **Basilisk installation:** See [BASILISK_INSTALL.md](BASILISK_INSTALL.md)
- **Compilation:** See [BASILISK_CONFIG.md](BASILISK_CONFIG.md)
- **Example simulations:** Check [README.md](README.md) and simulation source files

For questions about the VOF method itself, refer to the original paper.
