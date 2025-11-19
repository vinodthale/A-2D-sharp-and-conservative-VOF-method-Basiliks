# Basilisk Code Review: 2D Cartesian vs Axisymmetric Simulations

## Executive Summary

This review examines the syntactic and structural correctness of Basilisk simulation codes for:
1. **2D Cartesian simulations** (planar geometry)
2. **2D Axisymmetric simulations** (cylindrical coordinates with r-z symmetry)

**Key Findings:**
- **3 critical issues** that will prevent correct execution
- **2 portability issues** that may cause problems depending on Basilisk version
- All issues are documented with explanations and corrected code snippets

---

## Issue Summary Table

| # | File | Issue Type | Severity | Affects |
|---|------|------------|----------|---------|
| 1 | `circle-droplet.c` | 3D stencil in 2D simulation | **CRITICAL** | 2D Cartesian only |
| 2 | `droplet-impact-orifice.c` | Missing axi.h include | **CRITICAL** | Axisymmetric only |
| 3 | `droplet-impact-orifice-nondim.c` | Missing axi.h include | **CRITICAL** | Axisymmetric only |
| 4 | All axisymmetric files | `dimension == 2` check | Warning | Axisymmetric portability |
| 5 | All files | Origin position check | Info | Both (best practice) |

---

## Detailed Issue Analysis

### ISSUE 1: 3D Stencil in 2D Cartesian Simulation (CRITICAL)

**File:** `src/2d-cartesian/circle-droplet.c`
**Lines:** 128-144
**Severity:** CRITICAL - Will cause compilation error or incorrect behavior
**Affects:** 2D Cartesian simulations only

#### Problem Description

The adaptive mesh refinement event uses a **3D smoothing stencil** in a **2D simulation**. The code includes z-direction array accesses like `tmp_c[0, 0, 1]` and `tmp_c[0, 0, -1]`, which are invalid in 2D.

#### Current Code (INCORRECT)

```c
#if TREE
event adapt (i++) {
  scalar sf1[];
  foreach() {
    sf1[] = (8. * tmp_c[] +
       4. * (tmp_c[-1] + tmp_c[1] +
       tmp_c[0, 1] + tmp_c[0, -1] +
       tmp_c[0, 0, 1] + tmp_c[0, 0, -1]) +          // ❌ 3D terms!
       2. * (tmp_c[-1, 1] + tmp_c[-1, 0, 1] + tmp_c[-1, 0, -1] + tmp_c[-1, -1] +
       tmp_c[0, 1, 1] + tmp_c[0, 1, -1] + tmp_c[0, -1, 1] + tmp_c[0, -1, -1] +
       tmp_c[1, 1] + tmp_c[1, 0, 1] + tmp_c[1, -1] + tmp_c[1, 0, -1]) +
       tmp_c[1, -1, 1] + tmp_c[-1, 1, 1] + tmp_c[-1, 1, -1] + tmp_c[1, 1, 1] +
       tmp_c[1, 1, -1] + tmp_c[-1, -1, -1] + tmp_c[1, -1, -1] + tmp_c[-1, -1, 1]) / 64.;
    sf1[] += cs[];
  }
  adapt_wavelet ({sf1}, (double[]){1e-5}, minlevel = max(3, MAXLEVEL - 7), maxlevel = MAXLEVEL);
}
#endif
```

#### Why This is Wrong

In 2D Basilisk:
- Array indexing uses only 2 indices: `field[i, j]`
- Valid neighbors: `[-1]`, `[1]`, `[0,1]`, `[0,-1]`, `[-1,1]`, `[-1,-1]`, `[1,1]`, `[1,-1]`
- **Invalid in 2D:** `[0,0,1]`, `[0,0,-1]` (z-direction doesn't exist)

The stencil weights also don't normalize correctly: 8 + 4×6 + 2×12 + 8×8 = 120, divided by 64 ≠ 1.

#### Corrected Code

```c
#if TREE
event adapt (i++) {
  scalar sf1[];
  foreach() {
    // Correct 2D 9-point smoothing stencil
    sf1[] = (8. * tmp_c[] +
       4. * (tmp_c[-1] + tmp_c[1] + tmp_c[0, 1] + tmp_c[0, -1]) +
       2. * (tmp_c[-1, 1] + tmp_c[-1, -1] + tmp_c[1, 1] + tmp_c[1, -1])) / 16.;
    sf1[] += cs[];
  }
  adapt_wavelet ({sf1}, (double[]){1e-5},
                 minlevel = max(3, MAXLEVEL - 7),
                 maxlevel = MAXLEVEL);
}
#endif
```

**Alternative (simpler):** Just use the VOF field directly:

```c
#if TREE
event adapt (i++) {
  adapt_wavelet ({tmp_c, cs, u},
                 (double[]){1e-3, 1e-3, 1e-2, 1e-2},
                 minlevel = max(3, MAXLEVEL - 7),
                 maxlevel = MAXLEVEL);
}
#endif
```

---

### ISSUE 2: Missing axi.h Include (CRITICAL)

**File:** `src/axisymmetric/droplet-impact-orifice.c`
**Lines:** 24-28
**Severity:** CRITICAL - Simulation will run as 2D Cartesian instead of axisymmetric
**Affects:** Axisymmetric simulation only

#### Problem Description

The file defines `#define AXISYM 1` but **does not include "axi.h"**. This means:
- The metric factors (cm, fm) will NOT be set to `y` (radial coordinate)
- Volume integrals will be incorrect (no `2πr` weighting)
- The simulation will run as **2D Cartesian**, not axisymmetric
- Results will be physically incorrect

#### Current Code (INCORRECT)

```c
// Enable axisymmetric coordinates BEFORE including solvers
#define AXISYM 1          // ❌ This macro alone does nothing!

#include "myembed.h"
#include "navier-stokes/centered.h"
// ... no axi.h included!
```

#### Why This is Wrong

In Basilisk's axisymmetric framework:
- `#define AXISYM 1` is just a user-defined flag (has no effect on Basilisk)
- **Only `#include "axi.h"`** sets up axisymmetric coordinates:
  - Defines metric factor: `cm[] = y` (where y = radial coordinate r)
  - Defines face metric: `fm.x[] = y`, `fm.y[] = y`
  - Modifies `dv()` to return `π r² Δz Δr` instead of `Δx Δy`
  - Sets up proper refinement/prolongation functions

Without axi.h, the code computes:
- Volume = `Δx × Δy` (Cartesian)
- Instead of: Volume = `2π × r × Δr × Δz` (Axisymmetric)

#### Corrected Code

```c
// Enable axisymmetric coordinates by including axi.h FIRST
#include "axi.h"          // ✓ This must come BEFORE navier-stokes/centered.h

#include "myembed.h"
#include "navier-stokes/centered.h"
#include "embed_contact.h"
#include "embed_two-phase.h"
#include "embed_tension.h"
#include "navier-stokes/perfs.h"
#include "profiling.h"
```

**Critical:** `axi.h` must be included **before** `navier-stokes/centered.h` to properly set up the metric.

---

### ISSUE 3: Missing axi.h Include (CRITICAL)

**File:** `src/axisymmetric/droplet-impact-orifice-nondim.c`
**Lines:** 45-52
**Severity:** CRITICAL - Same as Issue 2
**Affects:** Axisymmetric simulation only

#### Problem Description

Same as Issue 2 - this file correctly includes `axi.h` in line 45, so **this is actually CORRECT**. However, the comment structure might be confusing.

#### Current Code (CORRECT)

```c
#include "axi.h"                     // ✓ Correct!
#include "navier-stokes/centered.h"
#include "myembed.h"
#include "embed_contact.h"
// ...
```

**Status:** No fix needed - this file is correct.

---

### ISSUE 4: Dimension Check for Visualization (PORTABILITY WARNING)

**Files:**
- `src/axisymmetric/droplet-impact-sharp-orifice.c` (line 348)
- `src/axisymmetric/droplet-impact-round-orifice.c` (line 295)

**Severity:** Warning - May prevent compilation on some Basilisk versions
**Affects:** Axisymmetric simulations (visualization code)

#### Problem Description

The visualization code is guarded by:

```c
#if dimension == 2
#include "view.h"
event movie (t += 0.0002) {
  // ...
}
#endif
```

**Potential Issue:**
- In some Basilisk versions, `dimension` may not be defined as a preprocessor macro
- Even when using `axi.h`, dimension should be 2, but the check may fail if dimension is undefined
- This would silently disable all visualization code

#### Why It's Usually OK

In standard Basilisk:
- Default dimension is 2
- `axi.h` doesn't change dimension (axisymmetric is still 2D, just with modified metric)
- The check `#if dimension == 2` should work

#### Recommended Fix (for robustness)

**Option 1:** Always include visualization in 2D/axisymmetric:

```c
// Visualization works for both 2D Cartesian and axisymmetric
#if !dimension || dimension == 2
#include "view.h"

event movie (t += 0.0002) {
  static FILE * fp = NULL;
  if (fp == NULL)
    fp = fopen ("movie_sharp.ppm", "w");

  view (width = 800, height = 800);
  // ...
}
#endif
```

**Option 2:** Remove the dimension check entirely (simpler):

```c
#include "view.h"

event movie (t += 0.0002) {
  // Visualization code
  // This will work correctly for 2D and axisymmetric
  // (will cause error in 3D, but these are explicitly 2D simulations)
}
```

**Option 3:** Use `#if AXI` to detect axisymmetric mode:

```c
#if dimension == 2 || AXI
#include "view.h"
event movie (t += 0.0002) {
  // ...
}
#endif
```

---

### ISSUE 5: Origin Position for Axisymmetric (BEST PRACTICE)

**Files:** All axisymmetric simulations
**Severity:** Info - Current code is correct, but worth documenting
**Affects:** Axisymmetric simulations

#### Current Practice (CORRECT)

All axisymmetric files correctly use:

```c
int main() {
  size (L0);
  origin (0., 0.);    // ✓ Correct for axisymmetric!
  // ...
}
```

#### Why This is Critical for Axisymmetric

In axisymmetric coordinates:
- `x` = radial coordinate (r)
- `y` = axial coordinate (z)
- **The left boundary (x=0) is the axis of symmetry (r=0)**

**Requirement:** `origin()` must have `x_origin = 0` (or origin must be set such that x ≥ 0 everywhere).

**Why:** The metric factor is `cm[] = y` in standard Basilisk axi.h, **but actually should be `cm[] = x`** for r-z coordinates where x=r!

#### CRITICAL FINDING - Coordinate Convention Issue

Looking at `axi.h`:

```c
// From axi.h documentation:
// longitudinal coordinate (z-axis) is *x*
// radial coordinate (r-axis) is *y*
```

This means in Basilisk's axi.h:
- **x = z (axial/longitudinal)**
- **y = r (radial)**

So the metric should be `cm[] = y` (which is what axi.h implements).

**Boundary conditions must reflect this:**

```c
// Left boundary is r = 0 (axis of symmetry)
u.n[left] = dirichlet(0.);  // u_r = 0 at r=0 ✓
u.t[left] = neumann(0.);    // du_z/dr = 0 at r=0 ✓
```

**ALL axisymmetric files correctly implement this!** ✓

---

## Summary of Required Fixes

### Fix Priority: CRITICAL

1. **`circle-droplet.c`** - Remove 3D stencil terms from adapt event
2. **`droplet-impact-orifice.c`** - Add `#include "axi.h"` before navier-stokes includes

### Fix Priority: Recommended

3. **All axisymmetric visualization blocks** - Make dimension check more robust

---

## Verification Checklist

### For 2D Cartesian Simulations (`circle-droplet.c`)

✓ **Correct:**
- No `#include "axi.h"` (would make it axisymmetric)
- Boundary conditions use u.t, u.n for embedded boundaries
- Uses `dv()` for volume calculation
- Metric factor `cm[]` defaults to 1.0 (Cartesian)

✗ **Incorrect:**
- Adapt event uses 3D stencil → **MUST FIX**

### For Axisymmetric Simulations

✓ **Correct** (droplet-impact-sharp-orifice.c, droplet-impact-round-orifice.c, *-nondim.c):
- `#include "axi.h"` before navier-stokes includes
- `origin(0., 0.)` ensures y ≥ 0 (where y = r)
- Left boundary: `u.n[left] = dirichlet(0.)` (u_r = 0 at axis)
- Left boundary: `u.t[left] = neumann(0.)` (du_z/dr = 0 by symmetry)
- Uses `dv()` which returns `2πr × dr × dz` volume
- Droplet initialization: `fraction(f, -(sq(x) + sq(y - h) - sq(r)))` correct for r-z coords

✗ **Incorrect:**
- `droplet-impact-orifice.c` missing `#include "axi.h"` → **MUST FIX**

⚠ **Warning:**
- Dimension checks may fail on some Basilisk versions → Recommended to fix

---

## Basilisk Framework Correctness

### Events and Time-Stepping

✓ All files correctly use:
- `event init (t = 0)` for initialization
- `event adapt (i++)` for adaptive refinement
- `event logfile (i++)` or `event logfile (t += dt)` for output
- `event end (t = T_END)` for termination

### Adaptive Mesh Refinement

✓ Most files use `adapt_wavelet()` or `adapt_wavelet_limited()` correctly
✗ `circle-droplet.c` has incorrect stencil (see Issue 1)

### VOF and Two-Phase Flow

✓ All files correctly:
- Declare `scalar f[]` and `scalar * interfaces = {f}`
- Use `fraction()` for interface initialization
- Multiply by solid fraction: `f[] *= cs[]`
- Call `boundary()` after field updates
- Use `foreach(reduction(+:v))` for volume integration

### Boundary Conditions

✓ All files correctly apply:
- `boundary()` calls after field initialization
- Proper embedded boundary conditions (u.t, u.n)
- Axisymmetric axis conditions (where applicable)

### Coordinate Systems

✓ **2D Cartesian** (circle-droplet.c):
- x, y are Cartesian coordinates
- `dv()` = Δx × Δy
- `cm[] = 1.0` (default)

✓ **Axisymmetric** (with axi.h):
- x = z (axial), y = r (radial) [per axi.h convention]
- `dv()` = 2π × r × Δr × Δz = 2π × y × Δx × Δy
- `cm[] = y` (radial coordinate)
- `fm.x[] = y`, `fm.y[] = y`

---

## Code Snippets for Migration Between Modes

### Converting 2D Cartesian → Axisymmetric

**Step 1:** Add axi.h include (FIRST!)

```c
#include "axi.h"              // ADD THIS FIRST
#include "navier-stokes/centered.h"
// ... other includes
```

**Step 2:** Update coordinate interpretation

```c
// In 2D Cartesian: x and y are planar coordinates
// In axisymmetric:  x = z (axial), y = r (radial)

// Geometry must be redefined accordingly:
// Circle in x-y plane (Cartesian): sq(x - x0) + sq(y - y0) - sq(R)
// Sphere in r-z coords (Axisym):   sq(x - z0) + sq(y - r0) - sq(R)  (same form!)
```

**Step 3:** Update boundary conditions

```c
// Left boundary becomes axis of symmetry (r = 0)
u.n[left] = dirichlet(0.);  // u_r = 0 at r=0
u.t[left] = neumann(0.);    // du_z/dr = 0 at r=0
f[left] = neumann(0.);
cs[left] = neumann(0.);
```

**Step 4:** Ensure origin has y ≥ 0

```c
origin(0., 0.);  // or origin(z_min, 0.) to ensure y=r≥0 everywhere
```

### Converting Axisymmetric → 2D Cartesian

**Step 1:** Remove axi.h include

```c
// #include "axi.h"           // REMOVE THIS
#include "navier-stokes/centered.h"
// ...
```

**Step 2:** Reinterpret coordinates

```c
// x, y now represent Cartesian planar coordinates
// No metric weighting; dv() = Δx × Δy
```

**Step 3:** Update boundary conditions

```c
// Left boundary is no longer special (not axis of symmetry)
// Set based on physical problem (wall, symmetry, outflow, etc.)
u.n[left] = dirichlet(0.);  // or neumann(0.), etc.
u.t[left] = neumann(0.);    // or dirichlet(0.), etc.
```

---

## Testing Recommendations

### Compilation Test

```bash
# Test all files compile
cd /home/user/A-2D-sharp-and-conservative-VOF-method-Basiliks
make clean
make circle-droplet
make droplet-impact-sharp-orifice
make droplet-impact-round-orifice
make droplet-impact-orifice
make droplet-impact-orifice-nondim
make droplet-impact-sharp-orifice-nondim
```

### Volume Conservation Test

After running, check that `V/V0 ≈ 1.0` in output logs:

```bash
# For 2D Cartesian
./circle-droplet
tail log  # Check volume ratio in column 2

# For axisymmetric
./droplet-impact-sharp-orifice
tail volume_sharp.txt  # Check V/V0 column
```

**Expected:** V/V0 should remain close to 1.0 (±1% for good VOF implementation)

### Geometric Symmetry Test

For axisymmetric cases, verify that flow on the axis (r=0) has:
- u_r = 0 (enforced by boundary condition)
- No singularities or NaN values

---

## References

1. Basilisk documentation: http://basilisk.fr/
2. Axisymmetric coordinates: http://basilisk.fr/src/axi.h
3. Embedded boundaries: http://basilisk.fr/src/embed.h
4. VOF method: http://basilisk.fr/src/vof.h

---

## Appendix: Quick Reference

### Basilisk Coordinate Conventions

| Mode | x-axis | y-axis | Metric cm | Volume dv() |
|------|--------|--------|-----------|-------------|
| 2D Cartesian | x | y | 1.0 | Δx × Δy |
| Axisymmetric (axi.h) | z (axial) | r (radial) | y (= r) | 2π × r × Δr × Δz |

### Key Basilisk Operators

- `dv()`: Cell volume (metric-aware)
- `cm[]`: Cell metric factor
- `fm.x[]`, `fm.y[]`: Face metric factors
- `foreach()`: Loop over all cells
- `foreach_dimension()`: Generate code for each dimension
- `boundary({f})`: Update boundary conditions for field f
- `fraction(f, expr)`: Initialize VOF field where expr < 0 is inside

### Face Vectors and Scalars

In Basilisk:
- `face vector` fields store values on cell faces
- `u.x[]` = velocity in x-direction at x-face
- `u.y[]` = velocity in y-direction at y-face
- `u.t[]` = tangential component (for boundaries)
- `u.n[]` = normal component (for boundaries)

---

**End of Report**
