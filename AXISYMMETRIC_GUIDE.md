# Axisymmetric Coordinates Integration Guide

## Table of Contents

1. [Overview](#overview)
2. [Coordinate System](#coordinate-system)
3. [When to Use Axisymmetric Coordinates](#when-to-use-axisymmetric-coordinates)
4. [Implementation in Basilisk](#implementation-in-basilisk)
5. [Using `axi.h`](#using-axih)
6. [Integration with Embedded Boundaries](#integration-with-embedded-boundaries)
7. [Volume and Flux Calculations](#volume-and-flux-calculations)
8. [Boundary Conditions](#boundary-conditions)
9. [Common Pitfalls](#common-pitfalls)
10. [Examples from This Repository](#examples-from-this-repository)
11. [Validation and Testing](#validation-and-testing)
12. [Reference](#reference)

---

## Overview

Axisymmetric coordinates allow 3D problems with rotational symmetry to be solved as 2D problems, dramatically reducing computational cost. For droplet impact simulations, this is ideal when the geometry and physics are symmetric around a central axis.

### Key Benefits

- **Computational Efficiency**: Solve 3D problems in 2D (100-1000x faster)
- **Memory Savings**: Reduced memory footprint
- **Higher Resolution**: More grid points for the same computational cost
- **Physical Accuracy**: Exact representation of 3D physics with axial symmetry

---

## Coordinate System

### Cylindrical Coordinates

Axisymmetric problems use cylindrical coordinates $(r, \theta, z)$ where:
- **$r$**: Radial distance from axis of symmetry
- **$\theta$**: Azimuthal angle (ignored due to symmetry)
- **$z$**: Longitudinal/axial coordinate

### Mapping to Basilisk Coordinates

In Basilisk's `axi.h` implementation:

| Physical Coordinate | Basilisk Variable | Description |
|---------------------|-------------------|-------------|
| $z$ (longitudinal) | `x` | Axial direction |
| $r$ (radial) | `y` | Radial direction |
| $\theta$ (azimuthal) | — | Not represented (symmetry) |

```
         z-axis (axial)
         ↑
         |
    y    |     3D representation
    ↑    |     of axisymmetric domain
    |    |
    |    |_____ r-axis (radial)
    |   /
    |  /
    | /
    |/________→ x (in Basilisk)

Basilisk 2D grid
(x=z, y=r)
```

### Important Constraint

⚠️ **The radial coordinate `y` must always be non-negative**: $y \geq 0$

This is enforced by the physics:
- `y = 0` represents the axis of symmetry
- `y > 0` represents points away from the axis

---

## When to Use Axisymmetric Coordinates

### Suitable Problems

✅ **Use axisymmetric coordinates when**:
- Geometry has rotational symmetry around a vertical/horizontal axis
- Initial conditions are axisymmetric
- Boundary conditions are axisymmetric
- No azimuthal flow (no swirl)

**Examples**:
- Droplet impact on flat plate
- Droplet impact on plate with circular orifice
- Bubble rising in liquid
- Jet breakup
- Capillary rise in circular tube

### Unsuitable Problems

❌ **Do NOT use axisymmetric coordinates when**:
- Geometry lacks rotational symmetry (e.g., rectangular channel)
- Flow has azimuthal component (swirl, rotation)
- Instabilities break symmetry (use 3D instead)

---

## Implementation in Basilisk

### Two Approaches

There are two ways to enable axisymmetric coordinates in Basilisk:

#### Approach 1: Using `axi.h` (Recommended for This Repository)

```c
#include "axi.h"
#include "navier-stokes/centered.h"
#include "myembed.h"
#include "embed_two-phase.h"
// ... other includes
```

**Advantages**:
- Explicit metric control
- Full integration with embedded boundaries via `axi_factor()`
- Custom refinement functions for adaptive mesh
- Works with this repository's VOF framework

**Used in**:
- `droplet-impact-sharp-orifice.c`
- `droplet-impact-sharp-orifice-nondim.c`
- `droplet-impact-round-orifice.c`
- `droplet-impact-orifice-nondim.c`

#### Approach 2: Using Built-in `AXISYM` Macro

```c
#define AXISYM 1
#include "myembed.h"
#include "navier-stokes/centered.h"
// ... other includes
```

**Advantages**:
- Simpler syntax
- Built into Basilisk

**Disadvantages**:
- Less control over metric factors
- May not integrate as seamlessly with custom embedded boundary code

**Used in**:
- `droplet-impact-orifice.c`

---

## Using `axi.h`

### Basic Usage

#### 1. Include Order

```c
#include "axi.h"                    // FIRST: Define coordinate system
#include "navier-stokes/centered.h" // SECOND: Solver
#include "myembed.h"                // THIRD: Embedded boundaries
#include "embed_two-phase.h"        // Other modules
#include "embed_vof.h"
#include "embed_tension.h"
#include "embed_contact.h"
```

#### 2. Domain Setup

```c
int main() {
  size(L0);           // Domain size
  origin(0., 0.);     // IMPORTANT: Set origin so y ≥ 0

  init_grid(1 << MINLEVEL);

  // ... physical properties
  run();
}
```

⚠️ **Critical**: Set `origin(0., 0.)` to ensure `y ≥ 0` throughout domain.

#### 3. Geometry Definition

For axisymmetric geometry, use radial coordinate `x` (which is `r`):

```c
event init (t = 0) {
  // Spherical droplet centered at (r=0, z=H_DROP)
  fraction(f, -(sq(x) + sq(y - H_DROP) - sq(R_DROP)));

  // Circular orifice at radius R_ORIFICE
  vertex scalar phi[];
  foreach_vertex() {
    double r = x;  // Radial coordinate
    double z = y;  // Axial coordinate

    if (r < R_ORIFICE) {
      phi[] = 1.0;  // Inside orifice = fluid
    } else {
      // Outside orifice = solid or fluid based on z
      // ... define your geometry
    }
  }
  fractions(phi, cs, fs);
}
```

### What `axi.h` Does Automatically

When you include `axi.h`, it automatically:

1. **Defines `#define AXI 1`** - Enables axisymmetric-specific code
2. **Sets up metric tensor `cm`** - Cell volumes: $V_{cell} \propto r$
3. **Sets up face metrics `fm`** - Face areas: $A_{face} \propto r$
4. **Handles axis singularity** - Prevents division by zero at $r=0$
5. **Provides refinement functions** - Proper metric interpolation during AMR
6. **Integrates with embedded boundaries** - Via `metric_embed_factor`

### Key Variables

| Variable | Type | Description |
|----------|------|-------------|
| `cm` | `scalar` | Cell metric (volume factor = $r$) |
| `fm` | `face vector` | Face metric (area factor = $r$) |
| `axi_factor` | `function` | Metric factor for embedded fluxes |

---

## Integration with Embedded Boundaries

### How It Works

The `axi.h` file integrates with `myembed.h` through the `metric_embed_factor` function pointer:

**In `axi.h` (lines 86-88, 159-160)**:
```c
double axi_factor (Point point, coord p) {
  return (y + p.y*Delta);  // Returns r coordinate
}

// In metric event:
#if EMBED
  metric_embed_factor = axi_factor;
#endif
```

**In `myembed.h` (line 22)**:
```c
double (* metric_embed_factor) (Point, coord) = NULL;
```

This allows embedded boundary fluxes to be correctly weighted by the radial coordinate.

### Embedded Geometry Best Practices

#### 1. Level-Set Function

Define your solid geometry using a level-set (signed distance function):
- **Negative** inside solid
- **Positive** in fluid
- Zero at interface

```c
vertex scalar phi[];
foreach_vertex() {
  double r = x;
  double z = y;

  // Example: Plate with orifice
  double plate_top = PLATE_POS_Y;
  double plate_bottom = PLATE_POS_Y - PLATE_THICK;

  if (r < R_ORIFICE) {
    phi[] = 1.0;  // Inside orifice = fluid
  } else {
    if (z >= plate_bottom && z <= plate_top) {
      phi[] = -(r - R_ORIFICE);  // Solid plate
    } else {
      phi[] = 1.0;  // Fluid above/below plate
    }
  }
}

fractions(phi, cs, fs);
```

#### 2. Volume Fraction Fields

After `fractions(phi, cs, fs)`:
- `cs[]`: Cell solid fraction (0 = full fluid, 1 = full solid, 0-1 = cut cell)
- `fs.x[]`, `fs.y[]`: Face area fractions

### Metric Updates for Cut Cells

For embedded boundaries, `axi.h` provides two update functions:

#### `cm_update(scalar cm, scalar cs, face vector fs)`

Updates cell metric for cut cells:
```c
cm[] = (y + Delta*p.y) * cs[]
```
where `p.y` is the offset to the centroid of the fluid portion.

#### `fm_update(face vector fm, scalar cs, face vector fs)`

Updates face metrics for cut faces:
```c
fm.x[] = (y - sig*(1. - fs.x[])) * fs.x[]
fm.y[] = max(y, 1e-20) * fs.y[]
```

**Typically you don't need to call these manually** - the `metric` event handles it.

---

## Volume and Flux Calculations

### Volume Integration

In axisymmetric coordinates, the volume element is:

$$dV = 2\pi r \, dr \, dz$$

In Basilisk with `axi.h`:
```c
double volume = 0.;
foreach(reduction(+:volume)) {
  volume += f[] * dv();  // dv() already includes 2πr factor via cm
}
```

⚠️ **Important**: `dv()` returns the **full 3D volume** element, already including the $2\pi r$ factor through the metric `cm`.

### Detailed Volume Calculation

When you need to account for solid fractions:
```c
foreach(reduction(+:volume)) {
  if (dv() > 0) {
    double dvolume = dv() / cm[] * f[];  // Divide by cm for safety
    volume += dvolume;
  }
}
```

From `droplet-impact-orifice.c:214`:
```c
double dvolume = dv()/cm[] * f[];
```

### Center of Mass Calculation

```c
double volume = 0., xc = 0., yc = 0.;

foreach(reduction(+:volume) reduction(+:xc) reduction(+:yc)) {
  if (dv() > 0) {
    double dvolume = dv()/cm[] * f[];
    volume += dvolume;
    xc += x * dvolume;  // x is r (radial)
    yc += y * dvolume;  // y is z (axial)
  }
}

xc /= volume;  // Centroid r-coordinate
yc /= volume;  // Centroid z-coordinate
```

### Conservation Checking

Always verify volume conservation:
```c
static double v0;
if (i == 0) v0 = volume;

double conservation_error = fabs(volume/v0 - 1.0);
if (conservation_error > 1e-6) {
  fprintf(stderr, "WARNING: Volume conservation error = %g\n",
          conservation_error);
}
```

Expected: $V/V_0 \approx 1.0$ to machine precision (< $10^{-10}$ for well-resolved cases).

---

## Boundary Conditions

### Standard Boundaries

In axisymmetric coordinates with standard Basilisk orientation:

```
        top (z = L0)
         |
    left | right (r = L0)
    (r=0)|
         |
       bottom (z = 0)
```

### Left Boundary (Axis of Symmetry, r=0)

**Physics**: No flow across axis, symmetry in tangential components.

```c
// Radial velocity must be zero
u.n[left] = dirichlet(0.);

// Axial velocity gradient = 0 (symmetry)
u.t[left] = neumann(0.);

// Volume fraction (symmetry)
f[left] = neumann(0.);

// Solid fraction (symmetry)
cs[left] = neumann(0.);
```

### Right Boundary (Far Field, r=L0)

**Physics**: Usually far from action, use outflow conditions.

```c
u.n[right] = neumann(0.);   // Zero gradient
u.t[right] = neumann(0.);
p[right] = dirichlet(0.);   // Reference pressure
```

### Bottom Boundary (z=0)

**Physics**: Depends on problem (wall, outflow, etc.).

For solid wall:
```c
u.n[bottom] = dirichlet(0.);  // No penetration
u.t[bottom] = dirichlet(0.);  // No slip
```

### Top Boundary (z=L0)

**Physics**: Usually outflow.

```c
u.n[top] = neumann(0.);   // Outflow
u.t[top] = neumann(0.);
p[top] = dirichlet(0.);   // Reference pressure
```

### Embedded Boundary (Solid Surfaces)

For solid surfaces (plates, obstacles):

```c
// No-slip condition
u.t[embed] = dirichlet(0.);
u.n[embed] = dirichlet(0.);

// Contact angle (for two-phase flows)
f.contact_angle = contact_angle;  // Vector field
```

Example from `droplet-impact-orifice.c:84-86`:
```c
u.t[embed] = dirichlet(0.);
u.n[embed] = dirichlet(0.);
```

---

## Common Pitfalls

### 1. Negative Radial Coordinate

❌ **Wrong**:
```c
origin(-L0/2, -L0/2);  // y can be negative!
```

✅ **Correct**:
```c
origin(0., 0.);  // Ensures y ≥ 0
```

**Consequence**: Negative `y` causes:
- Negative volumes (non-physical)
- Division by zero issues
- Incorrect metric calculations

---

### 2. Incorrect Volume Calculation

❌ **Wrong**:
```c
double volume = 0.;
foreach(reduction(+:volume)) {
  volume += f[] * Delta * Delta;  // 2D Cartesian volume!
}
```

✅ **Correct**:
```c
double volume = 0.;
foreach(reduction(+:volume)) {
  volume += f[] * dv();  // Uses proper axisymmetric volume
}
```

**Explanation**: `dv()` automatically includes the $2\pi r$ factor via `cm`.

---

### 3. Forgetting Axis Boundary Conditions

❌ **Wrong**:
```c
// No boundary conditions at left (axis)
```

✅ **Correct**:
```c
u.n[left] = dirichlet(0.);  // Radial velocity = 0
u.t[left] = neumann(0.);    // Axial symmetry
f[left] = neumann(0.);
cs[left] = neumann(0.);
```

**Consequence**: Unphysical flow across axis of symmetry.

---

### 4. Include Order

❌ **Wrong**:
```c
#include "navier-stokes/centered.h"
#include "axi.h"  // Too late!
```

✅ **Correct**:
```c
#include "axi.h"  // BEFORE solver
#include "navier-stokes/centered.h"
```

**Explanation**: `axi.h` must define the coordinate system before the solver is loaded.

---

### 5. Coordinate Confusion

❌ **Wrong**:
```c
// Thinking x is radial, y is axial
fraction(f, -(sq(y) + sq(x - H_DROP) - sq(R_DROP)));
```

✅ **Correct**:
```c
// x is AXIAL (z), y is RADIAL (r)
fraction(f, -(sq(x) + sq(y - H_DROP) - sq(R_DROP)));
```

**Remember**: In `axi.h`:
- `x` in Basilisk = $z$ (axial, longitudinal)
- `y` in Basilisk = $r$ (radial)

---

### 6. Division by Zero at Axis

The `axi.h` implementation handles this automatically:
```c
fm.x[] = max(y, 1./HUGE);  // Prevents division by zero
```

But if you write custom code:

❌ **Wrong**:
```c
double curvature = some_function / y;  // Division by zero at y=0!
```

✅ **Correct**:
```c
double curvature = y > 1e-10 ? some_function / y : 0.;
```

---

## Examples from This Repository

### Example 1: Droplet Impact on Plate with Orifice (Dimensional)

**File**: `droplet-impact-sharp-orifice.c`

**Key Features**:
```c
#include "axi.h"  // Axisymmetric coordinates

// Physical parameters
#define D_DROPLET  0.010307  // m
#define R_DROPLET  (D_DROPLET/2.0)
#define R_ORIFICE  0.003     // m

// Domain setup
int main() {
  size(L0);
  origin(0., 0.);  // Ensure y ≥ 0
  // ...
}

// Geometry: Plate with sharp orifice
event init (t = 0) {
  vertex scalar phi[];
  foreach_vertex() {
    double r = x;  // Radial
    double z = y;  // Axial

    // Define plate with orifice
    // (see file for details)
  }
  fractions(phi, cs, fs);

  // Spherical droplet
  fraction(f, -sq(x) + sq(y - DROPLET_POS_Y) + sq(R_DROPLET));
}
```

**Volume conservation check** (line 246):
```c
foreach(reduction(+:volume)) {
  volume += f[] * dv();
}
fprintf(fp, "%g\t%g\n", t, volume/volume_initial);
```

---

### Example 2: Non-Dimensional Formulation

**File**: `droplet-impact-orifice-nondim.c`

**Dimensionless Numbers**:
```c
#define RE    2000.0   // Reynolds: Re = ρUD/μ
#define WE    27.8     // Weber: We = ρU²D/σ
#define BO    0.544    // Bond: Bo = ρgD²/σ
```

**Non-dimensional properties**:
```c
rho1 = 1.0;           // Reference density
rho2 = RHO_RATIO;     // Density ratio
mu1 = 1.0/RE;         // Viscosity from Re
mu2 = MU_RATIO/RE;
f.sigma = 1.0/WE;     // Surface tension from We

// Gravity from Bo and We
const face vector g[] = {0., -BO/WE};
a = g;
```

**Advantages**:
- Parameter space exploration
- Universal scaling laws
- Direct comparison with theory/experiments

---

### Example 3: Contact Angle Hysteresis

**File**: `droplet-impact-sharp-orifice.c`

**Contact angle with pinning at sharp edge**:
```c
#define THETA_RECEDING   42.0   // Receding angle
#define THETA_ADVANCING  68.0   // Advancing angle
#define THETA_PINNING    150.0  // Pinning at sharp edge

vector contact_angle[];
scalar edge_marker[];

// Initialize edge marker
foreach() {
  double dist_to_edge = sqrt(sq(x - R_ORIFICE) + sq(y - PLATE_TOP));
  if (dist_to_edge < EDGE_REGION) {
    edge_marker[] = 1.0;  // Near sharp edge
  } else {
    edge_marker[] = 0.0;
  }
}

// Update contact angle based on position
event contact_angle_update (i++) {
  foreach() {
    if (cs[] > 0 && cs[] < 1) {  // At boundary
      if (edge_marker[] > 0.5) {
        contact_angle.x[] = THETA_PINNING;  // Pin at edge
      } else {
        // Apply hysteresis (receding-advancing window)
        // ...
      }
    }
  }
}
```

---

## Validation and Testing

### 1. Volume Conservation

**Test**: Total liquid volume should remain constant.

```c
event logfile (i++) {
  double volume = 0.;
  foreach(reduction(+:volume)) {
    volume += f[] * dv();
  }

  static double v0;
  if (i == 0) v0 = volume;

  fprintf(stderr, "t=%g  V/V0=%g\n", t, volume/v0);
}
```

**Expected**: $|V/V_0 - 1| < 10^{-6}$ for well-resolved simulations.

**Common issues if conservation fails**:
- Outflow boundaries removing liquid
- Small cut cells (use `cleansmallcell()`)
- Insufficient resolution
- Time step too large

---

### 2. Metric Verification

**Test**: Check that metric fields are correctly initialized.

```c
event metric_check (i = 0) {
  foreach() {
    // Cell metric should equal y (radial coordinate)
    if (fabs(cm[] - y) > 1e-10 && cs[] > 0.99) {
      fprintf(stderr, "ERROR: cm[] = %g, y = %g at (%g,%g)\n",
              cm[], y, x, y);
    }
  }

  foreach_face(x) {
    // Face metric should equal y
    if (fabs(fm.x[] - y) > 1e-10 && fs.x[] > 0.99) {
      fprintf(stderr, "ERROR: fm.x[] = %g, y = %g at face (%g,%g)\n",
              fm.x[], y, x, y);
    }
  }
}
```

---

### 3. Axis Symmetry Check

**Test**: Flow should be symmetric about axis (r=0).

```c
event symmetry_check (t += 0.1) {
  // Radial velocity at axis should be zero
  foreach() {
    if (x < 0.01*L0) {  // Near axis
      if (fabs(u.x[]) > 1e-6) {
        fprintf(stderr, "WARNING: u_r = %g at r=%g (should be ~0)\n",
                u.x[], x);
      }
    }
  }
}
```

---

### 4. Grid Convergence Study

**Test**: Results should converge as resolution increases.

```bash
# Run with increasing MAXLEVEL
for LEVEL in 7 8 9 10; do
  qcc -DMAXLEVEL=$LEVEL -o sim sim.c
  ./sim
done

# Compare results (e.g., final droplet shape, impact force, etc.)
```

**Expected**: Solution should converge at some resolution level.

---

### 5. Comparison with 3D Simulation

**Gold standard test**: Compare axisymmetric result with full 3D simulation.

For a simple case (e.g., spherical droplet impact on flat plate):
1. Run axisymmetric simulation (2D)
2. Run full 3D simulation with same parameters
3. Compare:
   - Droplet spreading diameter vs time
   - Maximum spreading radius
   - Volume conservation
   - Contact line position

**Expected**: Axisymmetric and 3D results should match within numerical error.

---

## Troubleshooting

### Issue: Compilation Error "axi.h not found"

**Solution**: Ensure `axi.h` is in the same directory as your `.c` file, or in Basilisk's include path.

---

### Issue: Volume Conservation Failure

**Symptoms**: $V/V_0$ drifts from 1.0.

**Possible Causes**:
1. **Outflow boundaries removing fluid**
   - Check boundary conditions
   - Ensure domain is large enough

2. **Small cut cells**
   - Use `cleansmallcell(cs, fs, 1e-2);` after `fractions()`

3. **Insufficient resolution**
   - Increase `MAXLEVEL`
   - Check adaptive mesh criteria

4. **Time step issues**
   - Reduce time step (lower CFL number)
   - Check `dtmax` setting

---

### Issue: Crash at Axis (r=0)

**Symptoms**: Division by zero, NaN values near left boundary.

**Solutions**:
1. Check boundary conditions:
   ```c
   u.n[left] = dirichlet(0.);
   u.t[left] = neumann(0.);
   ```

2. Ensure `axi.h` metric handles axis properly (it does automatically)

3. Check custom code for division by `y`:
   ```c
   // Bad
   double value = something / y;

   // Good
   double value = (y > 1e-10) ? something / y : 0.;
   ```

---

### Issue: Non-Physical Flow Across Axis

**Symptoms**: Radial velocity $u_r \neq 0$ at $r=0$.

**Solution**:
```c
u.n[left] = dirichlet(0.);  // Must be Dirichlet, not Neumann
```

---

### Issue: Negative Volumes

**Symptoms**: Volume becomes negative, `dv() < 0`.

**Cause**: Domain extends to $y < 0$ (negative radial coordinate).

**Solution**:
```c
origin(0., 0.);  // Not origin(-L0/2, -L0/2)
```

---

## Performance Tips

### 1. Adaptive Mesh Refinement

Use AMR to focus resolution where needed:

```c
event adapt (i++) {
  adapt_wavelet({f, u.x, u.y},
                (double[]){0.01, 0.01, 0.01},
                maxlevel = MAXLEVEL,
                minlevel = MINLEVEL);
}
```

**Typical refinement**:
- `MAXLEVEL = 9-12` (512-4096 cells per diameter)
- `MINLEVEL = 4-6` (16-64 cells in coarse regions)

---

### 2. Domain Size

**Rule of thumb**: Domain should be 3-5× droplet diameter to avoid boundary effects.

```c
#define L0  (5.0 * D_DROPLET)
```

---

### 3. Time Step Control

```c
// In main()
DT = 1e-5;     // Initial time step
dtmax = 1e-4;  // Maximum time step
```

Basilisk will adaptively reduce time step based on CFL condition.

---

## Summary Checklist

When setting up an axisymmetric simulation:

- [ ] Include `axi.h` before Navier-Stokes solver
- [ ] Set `origin(0., 0.)` to ensure $y \geq 0$
- [ ] Define geometry using `x` (radial) and `y` (axial) correctly
- [ ] Set axis boundary conditions: `u.n[left] = dirichlet(0.)`
- [ ] Use `dv()` for volume integration
- [ ] Verify volume conservation: $V/V_0 \approx 1.0$
- [ ] Check metric initialization: `cm[] = y`, `fm.x[] = y`
- [ ] Handle embedded boundaries via `fractions()` and `metric_embed_factor`
- [ ] Perform grid convergence study

---

## Reference

### Original Implementation

The `axi.h` file in this repository is sourced from:
- **Author**: Stephane Popinet and contributors
- **Source**: http://basilisk.fr/src/axi.h
- **Documentation**: http://basilisk.fr/src/examples/README#axisymmetric-coordinates

### Related Basilisk Documentation

- [Cylindrical coordinates](http://basilisk.fr/src/README#cylindrical-coordinates)
- [Embedded boundaries](http://basilisk.fr/src/embed.h)
- [Volume of Fluid method](http://basilisk.fr/src/vof.h)
- [Adaptive mesh refinement](http://basilisk.fr/src/grid/tree.h)

### This Repository

- **Main paper**: Huang, C.-S., Han, T.-Y., Zhang, J., & Ni, M.-J. (2025). "A 2D sharp and conservative VOF method for modeling the contact line dynamics with hysteresis on complex boundary." *Journal of Computational Physics*. https://doi.org/10.1016/j.jcp.2025.113975

- **Original code**: https://basilisk.dalembert.upmc.fr/sandbox/Chongsen/

### Further Reading

1. **Cylindrical coordinates in CFD**:
   - Ferziger, J. H., & Perić, M. (2002). *Computational Methods for Fluid Dynamics*. Springer.

2. **Embedded boundaries**:
   - Johansen, H., & Colella, P. (1998). "A Cartesian grid embedded boundary method for Poisson's equation on irregular domains." *Journal of Computational Physics*, 147(1), 60-85.

3. **VOF method**:
   - Popinet, S. (2009). "An accurate adaptive solver for surface-tension-driven interfacial flows." *Journal of Computational Physics*, 228(16), 5838-5866.

4. **Contact line dynamics**:
   - Cox, R. G. (1986). "The dynamics of the spreading of liquids on a solid surface." *Journal of Fluid Mechanics*, 168, 169-194.

---

## Questions and Support

If you encounter issues:

1. **Check this guide** for common pitfalls
2. **Verify examples** in this repository work correctly
3. **Consult Basilisk documentation** at http://basilisk.fr
4. **Review test cases** in Basilisk source code
5. **Check Basilisk forum** for similar questions

---

## License and Attribution

This guide is part of the repository:
**A-2D-sharp-and-conservative-VOF-method-Basiliks**

When using axisymmetric coordinates in your work, please cite:
- Huang et al. (2025) paper (for the VOF method)
- Basilisk documentation (for the coordinate system)
- Original Popinet papers (for the numerical methods)

---

*Last updated: November 2024*
