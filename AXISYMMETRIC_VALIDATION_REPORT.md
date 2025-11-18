# 2D Axisymmetric Simulation Validation Report

**Date**: 2025-11-18
**Repository**: A-2D-sharp-and-conservative-VOF-method-Basiliks
**Status**: ⚠️ Issues Found - Action Required

---

## Executive Summary

The repository contains **5 axisymmetric droplet impact simulations** using Basilisk. A validation check revealed that **3 out of 5 files are missing critical boundary condition definitions** for proper axisymmetric simulation. While the simulations may run, the lack of explicit boundary conditions could lead to:

1. Non-physical flow across the axis of symmetry
2. Incorrect symmetry enforcement
3. Numerical instabilities

---

## Validation Results

### ✅ PASSING (2/5 files)

1. **droplet-impact-orifice.c**
   - Uses `#define AXISYM 1`
   - Correct `origin(0., 0.)`
   - ✅ Complete boundary conditions defined (lines 84-107)
   - Uses `dv()` for volume calculations

2. **droplet-impact-orifice-nondim.c**
   - Uses `#include "axi.h"`
   - Correct `origin(0., 0.)`
   - ✅ Complete boundary conditions defined
   - Uses `dv()` for volume calculations

---

### ⚠️ **FAILING (3/5 files)** - Missing Boundary Conditions

3. **droplet-impact-sharp-orifice.c** ❌
   - Uses `#include "axi.h"` ✓
   - Has `origin(0., 0.)` at line 95 ✓
   - **MISSING**: Explicit boundary condition definitions
   - Uses `dv()` ✓

4. **droplet-impact-sharp-orifice-nondim.c** ❌
   - Uses `#include "axi.h"` ✓
   - Has `origin(0., 0.)` at line 111 ✓
   - **MISSING**: Explicit boundary condition definitions
   - Uses `dv()` ✓

5. **droplet-impact-round-orifice.c** ❌
   - Uses `#include "axi.h"` ✓
   - Has `origin(0., 0.)` at line 85 ✓
   - **MISSING**: Explicit boundary condition definitions
   - Uses `dv()` ✓

---

## Detailed Analysis

### What's Missing

All three failing files lack these critical boundary condition definitions:

```c
// Left boundary (axis of symmetry, r=0)
u.n[left] = dirichlet(0.);    // radial velocity = 0
u.t[left] = neumann(0.);      // axial velocity gradient = 0
f[left] = neumann(0.);        // VOF symmetry
cs[left] = neumann(0.);       // solid fraction symmetry

// Right boundary (far field)
u.n[right] = neumann(0.);
u.t[right] = neumann(0.);
p[right] = dirichlet(0.);

// Bottom boundary
u.n[bottom] = dirichlet(0.);  // or neumann depending on case
u.t[bottom] = dirichlet(0.);  // or neumann

// Top boundary (outflow)
u.n[top] = neumann(0.);
u.t[top] = neumann(0.);
p[top] = dirichlet(0.);

// Embedded boundary (solid surfaces)
u.t[embed] = dirichlet(0.);   // no-slip
u.n[embed] = dirichlet(0.);   // no penetration
```

---

## Impact Assessment

### Why This Matters for Axisymmetric Simulations

In axisymmetric coordinates:
- The **left boundary** is the axis of symmetry (r=0)
- Without `u.n[left] = dirichlet(0.)`, fluid can flow across the axis (non-physical)
- Without `u.t[left] = neumann(0.)`, symmetry is not properly enforced

### Potential Consequences

| Issue | Without Boundary Conditions | With Correct Boundary Conditions |
|-------|----------------------------|----------------------------------|
| Flow across axis | May occur | Prevented |
| Symmetry enforcement | Default (may be insufficient) | Explicitly enforced |
| Volume conservation | May be affected | Properly maintained |
| Numerical stability | Potential issues at r=0 | Stable at axis |
| Physical accuracy | Questionable | Guaranteed |

---

## Recommended Fixes

### Fix for all 3 files

Add the following **before the `main()` function** (typically after `#include` statements and before field declarations):

```c
/**
 * Boundary conditions for 2D axisymmetric simulation
 */

// Left boundary: axis of symmetry (r = 0)
u.n[left] = dirichlet(0.);    // radial velocity must be zero
u.t[left] = neumann(0.);      // axial velocity gradient = 0 (symmetry)
f[left] = neumann(0.);        // VOF field symmetry
cs[left] = neumann(0.);       // solid fraction symmetry

// Right boundary: far field (r = L0)
u.n[right] = neumann(0.);     // outflow
u.t[right] = neumann(0.);
p[right] = dirichlet(0.);     // reference pressure

// Bottom boundary (z = 0)
// For wall:
u.n[bottom] = dirichlet(0.);  // no penetration
u.t[bottom] = dirichlet(0.);  // no slip

// Top boundary (z = L0)
u.n[top] = neumann(0.);       // outflow
u.t[top] = neumann(0.);
p[top] = dirichlet(0.);       // reference pressure

// Embedded geometry: no-slip walls
u.t[embed] = dirichlet(0.);
u.n[embed] = dirichlet(0.);
```

---

## Comparison with Reference Implementation

### Working Example: droplet-impact-orifice.c (lines 84-107)

```c
// Embedded geometry (solid plate): no-slip walls
u.t[embed]  = dirichlet(0.);
u.n[embed]  = dirichlet(0.);

// Bottom boundary (vertical)
u.n[bottom] = dirichlet(0.);
u.t[bottom] = dirichlet(0.);

// Top boundary (vertical) - outflow
u.n[top]    = neumann(0.);
u.t[top]    = neumann(0.);
p[top]      = dirichlet(0.);

// Right boundary (far field radial)
u.n[right]  = neumann(0.);
u.t[right]  = neumann(0.);
p[right]    = dirichlet(0.);

// Left boundary (axis of symmetry r=0)
// Axisymmetric conditions: u_r = 0, du_z/dr = 0
u.n[left]   = dirichlet(0.);  // radial velocity = 0
u.t[left]   = neumann(0.);    // axial velocity gradient = 0
f[left]     = neumann(0.);
cs[left]    = neumann(0.);
tmp_c[left] = neumann(0.);    // Note: tmp_c specific to this implementation
```

---

## Additional Findings

### ✅ Strengths of Current Implementation

1. **Coordinate System**:
   - All files correctly use either `axi.h` or `AXISYM` macro
   - Proper origin setting: `origin(0., 0.)`
   - Correct radial coordinate mapping

2. **Volume Calculations**:
   - All files use `dv()` which includes 2πr factor
   - Proper axisymmetric volume integration

3. **Mesh Resolution**:
   - Files with MAXLEVEL=12 have excellent resolution (~132 cells/diameter)
   - Adaptive mesh refinement enabled

4. **Physical Setup**:
   - Complete fluid property definitions
   - Proper gravity implementation
   - Surface tension correctly specified

5. **Documentation**:
   - Excellent AXISYMMETRIC_GUIDE.md
   - Comprehensive test suite
   - Good code comments

### ⚠️ Warnings

1. **Low Resolution** (2 files):
   - `droplet-impact-orifice.c`: MAXLEVEL=8 (may be too coarse)
   - `droplet-impact-orifice-nondim.c`: MAXLEVEL=8
   - Recommendation: Increase to at least MAXLEVEL=10 for production runs

---

## Validation Test Suite Status

### Current Test Coverage

✅ **tests/validation/vof_advection/test_droplet_impact_validation.c**

Tests included:
1. Droplet spreading dynamics
2. Contact angle hysteresis
3. Mass conservation
4. Spurious currents

### Recommended Additional Tests

For axisymmetric-specific validation:
1. **Axis symmetry check**: Verify u_r = 0 at r=0
2. **Volume conservation in 3D**: Verify V_3D = ∫∫ 2πr·f·dr·dz
3. **Metric verification**: Check cm[] = y and fm.x[] = y
4. **Comparison with 3D**: Run same case in 3D and compare

---

## Action Items

### Priority 1: Critical Fixes

- [ ] Add boundary conditions to `droplet-impact-sharp-orifice.c`
- [ ] Add boundary conditions to `droplet-impact-sharp-orifice-nondim.c`
- [ ] Add boundary conditions to `droplet-impact-round-orifice.c`
- [ ] Verify all simulations run correctly after fixes
- [ ] Run validation tests

### Priority 2: Improvements

- [ ] Increase MAXLEVEL in low-resolution files (orifice.c, orifice-nondim.c)
- [ ] Add axis symmetry validation test
- [ ] Document boundary condition choices in each file

### Priority 3: ImpactForce Comparison

- [ ] Access ImpactForce repository files
- [ ] Compare axisymmetric implementations
- [ ] Compare physical parameters
- [ ] Identify portable test cases
- [ ] Document differences

---

## Comparison Framework for ImpactForce

Once ImpactForce repository is accessible, check:

### 1. Coordinate System
- [ ] Same r-z orientation as current repo?
- [ ] Uses axi.h or AXISYM?
- [ ] origin(0., 0.) enforced?

### 2. Boundary Conditions
- [ ] Axis boundary: u.n[left] = dirichlet(0.)?
- [ ] Complete BC set defined?
- [ ] Compatible with current implementation?

### 3. Volume Calculations
- [ ] Uses dv() or manual 2πr integration?
- [ ] Volume conservation checks?
- [ ] Same numerical accuracy?

### 4. Physical Setup
- [ ] Compound drop vs single drop differences
- [ ] Compatible fluid properties?
- [ ] Gravity treatment?

### 5. Geometry
- [ ] Same orifice modeling approach?
- [ ] Compatible level-set/fractions method?
- [ ] Embedded boundary approach?

---

## Checklist for Complete Validation

Use this before finalizing any simulation:

```
2D Axisymmetric Simulation Checklist:
□ Include axi.h or define AXISYM
□ Set origin(0., 0.)
□ Define ALL boundary conditions explicitly:
  □ Left (axis): u.n=dirichlet(0), u.t=neumann(0)
  □ Right (far field): appropriate outflow
  □ Top: outflow or appropriate BC
  □ Bottom: wall or appropriate BC
  □ Embed: no-slip (if applicable)
□ Use dv() for volume calculations
□ Verify volume conservation (|V/V₀ - 1| < 10⁻⁶)
□ Check metric: cm[] = y
□ Ensure y ≥ 0 throughout domain
□ Test for spurious currents
□ Verify axis symmetry: u_r ≈ 0 at r=0
□ Grid convergence study
```

---

## References

- **Basilisk axi.h documentation**: http://basilisk.fr/src/axi.h
- **Current repo guide**: AXISYMMETRIC_GUIDE.md
- **Comparison checklist**: AXISYMMETRIC_COMPARISON_CHECKLIST.md
- **Validation script**: validate_axisymmetric.sh

---

## Contact & Support

For questions about this validation report:
1. Review AXISYMMETRIC_GUIDE.md for detailed implementation guidance
2. Check Basilisk documentation at http://basilisk.fr
3. Run `./validate_axisymmetric.sh` to re-check after fixes

---

**Report Generated**: 2025-11-18
**Next Review**: After boundary condition fixes are implemented
**Status**: ⚠️ Requires immediate attention for 3 files

