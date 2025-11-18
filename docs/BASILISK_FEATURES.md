# Basilisk C Features Used in This Project

This document describes the key Basilisk C features and modules used in the 2D Sharp and Conservative VOF method implementation.

## Table of Contents

- [Overview](#overview)
- [Core Basilisk Features](#core-basilisk-features)
- [Grid Systems](#grid-systems)
- [Volume-of-Fluid (VOF) Method](#volume-of-fluid-vof-method)
- [Embedded Boundary Method](#embedded-boundary-method)
- [Two-Phase Flow Solver](#two-phase-flow-solver)
- [Surface Tension](#surface-tension)
- [Contact Line Dynamics](#contact-line-dynamics)
- [Adaptive Mesh Refinement](#adaptive-mesh-refinement)
- [Visualization and Output](#visualization-and-output)

## Overview

Basilisk C is a framework for solving partial differential equations using adaptive mesh refinement. This project leverages several advanced Basilisk capabilities:

- **Adaptive Cartesian grids** with quadtree/octree refinement
- **Volume-of-Fluid (VOF)** interface tracking
- **Embedded boundaries** for complex geometries
- **Two-phase flow** incompressible Navier-Stokes solver
- **Surface tension** with contact angle implementation
- **Axisymmetric coordinates** for cylindrical problems

## Core Basilisk Features

### 1. Automatic Differentiation

Basilisk automatically computes spatial derivatives using centered differences:

```c
// Velocity gradient (automatically computed)
foreach() {
  double dudx = (u.x[1,0] - u.x[-1,0])/(2.*Delta);
  double dudy = (u.x[0,1] - u.x[0,-1])/(2.*Delta);
}
```

### 2. Foreach Loops

Basilisk provides specialized iteration:

```c
// Loop over all cells
foreach() {
  // Process each cell
}

// Loop over leaf cells only (refined grid)
foreach_leaf() {
  // Process only finest level cells
}

// Loop over faces
foreach_face() {
  // Process face-centered quantities
}
```

### 3. Field Variables

Basilisk manages field variables with automatic allocation:

```c
// Scalar field (cell-centered)
scalar f[];

// Vector field (face-centered)
vector u[];

// Access field values
foreach() {
  f[] = 1.0;           // Current cell
  f[1,0] = 0.5;        // Right neighbor
  u.x[] = 1.0;         // x-component
  u.y[] = 0.0;         // y-component
}
```

### 4. Events

Time-based and iteration-based execution:

```c
// Initialize
event init(i = 0) {
  // Setup code
}

// Run every timestep
event update(i++) {
  // Update logic
}

// Run at specific times
event snapshots(t = 0.1; t <= 1.0; t += 0.1) {
  // Save output
}

// Run every N iterations
event logging(i += 10) {
  // Write log
}
```

## Grid Systems

### Cartesian Grid

Used in: `circle-droplet.c`

```c
#include "grid/cartesian.h"

int main() {
  // Initialize grid
  init_grid(64);  // 64x64 base grid

  // Set domain size
  size(4.0);  // Domain: [-2, 2] x [-2, 2]

  // Origin at center
  origin(-2.0, -2.0);

  run();
}
```

**Features**:
- 2D Cartesian coordinates
- Square cells
- Adaptive refinement up to MAXLEVEL

### Axisymmetric Grid

Used in: `droplet-impact-*.c`

```c
#include "axi.h"

int main() {
  init_grid(128);
  size(3.0);
  run();
}
```

**Features**:
- Cylindrical coordinates (r, z)
- Volume weighting for axisymmetry
- Singularity treatment at r=0

**Coordinate mapping**:
- x → z (axial direction)
- y → r (radial direction)

See [AXISYMMETRIC_GUIDE.md](AXISYMMETRIC_GUIDE.md) for details.

## Volume-of-Fluid (VOF) Method

### VOF Interface Tracking

The VOF method tracks interfaces using volume fractions.

```c
#include "vof.h"

scalar f[];  // Volume fraction field
vector u[];  // Velocity field

event advection(i++) {
  // Advect interface
  vof_advection(f, u);
}
```

**Volume fraction meaning**:
- `f = 0`: Gas/air phase
- `f = 1`: Liquid phase
- `0 < f < 1`: Interface cell

### Interface Reconstruction

Height function method for accurate curvature:

```c
#include "curvature.h"

event interface_properties(i++) {
  // Compute interface curvature
  scalar kappa[];
  curvature(f, kappa);

  // Use in surface tension
  foreach() {
    if (f[] > 0 && f[] < 1) {
      // Surface tension force: σ κ n
      double Fx = sigma * kappa[] * normal_x[];
      double Fy = sigma * kappa[] * normal_y[];
    }
  }
}
```

### Conservative VOF Advection

The project uses custom conservative VOF (`embed_vof.h`):

```c
#include "embed_vof.h"

// Conservative advection with embedded boundaries
event vof_advection(i++) {
  vof_advection({f}, uf, dt);
}
```

**Key features**:
- Mass conservation to machine precision
- Sharp interface maintenance
- Compatible with embedded boundaries

## Embedded Boundary Method

Handles complex solid geometries using embedded boundaries.

### Solid Fraction Field

```c
scalar cs[];  // Solid fraction (0 = solid, 1 = fluid)
vector fs[];  // Face solid fraction
```

### Defining Geometry

Example: Cylinder in `circle-droplet.c`

```c
#include "embed.h"

// Define solid geometry
vertex scalar phi[];

event init(i = 0) {
  // Signed distance function
  // Negative inside solid, positive in fluid
  foreach_vertex() {
    double r = sqrt(x*x + y*y);
    phi[] = r - r_cylinder;  // Circle of radius r_cylinder
  }

  // Initialize embedded boundary
  fractions(phi, cs, fs);
}
```

### Orifice Geometry

Example: Plate with circular orifice

```c
event init(i = 0) {
  foreach_vertex() {
    double r = sqrt(sq(y));  // Radial distance

    // Plate with hole
    if (x < 0 || x > thickness) {
      phi[] = 1.0;  // No solid (fluid region)
    } else if (r < r_orifice) {
      phi[] = 1.0;  // Inside orifice (fluid)
    } else {
      phi[] = -1.0; // Plate (solid)
    }
  }

  fractions(phi, cs, fs);
}
```

### Custom Embedded Headers

This project includes modified embedded boundary modules:

| Header | Purpose |
|--------|---------|
| `myembed.h` | Core embedded boundary utilities |
| `embed_vof.h` | VOF advection with embedded boundaries |
| `embed_two-phase.h` | Two-phase flow solver |
| `embed_tension.h` | Surface tension |
| `embed_contact.h` | Contact line dynamics |
| `embed_curvature.h` | Interface curvature calculation |
| `embed_heights.h` | Height function method |
| `embed_iforce.h` | Interfacial forces |

## Two-Phase Flow Solver

### Navier-Stokes Solver

Incompressible two-phase flow:

```c
#include "navier-stokes/centered.h"
#include "two-phase.h"

// Fluid properties
double rho1 = 1000.0;  // Liquid density (kg/m³)
double mu1 = 0.001;    // Liquid viscosity (Pa·s)
double rho2 = 1.0;     // Gas density
double mu2 = 1e-5;     // Gas viscosity

event properties(i++) {
  // Update density and viscosity based on f
  foreach() {
    rho[] = clamp(f[], 0., 1.)*(rho1 - rho2) + rho2;
    mu[]  = clamp(f[], 0., 1.)*(mu1 - mu2) + mu2;
  }
}
```

**Features**:
- Projection method for pressure
- Centered finite differences
- Variable density and viscosity
- Adaptive time stepping (CFL condition)

### Momentum Equation

```
∂u/∂t + u·∇u = -(1/ρ)∇p + (1/ρ)∇·(2μD) + g + F_st
```

Where:
- u: velocity
- ρ: density (from VOF)
- μ: viscosity (from VOF)
- p: pressure
- g: gravity
- F_st: surface tension force

### Velocity Boundary Conditions

```c
// No-slip on embedded boundaries (automatic)

// Free surface (top boundary)
u.n[top] = neumann(0);
p[top] = dirichlet(0);

// Symmetry (for axisymmetric)
u.n[left] = neumann(0);
u.t[left] = dirichlet(0);
```

## Surface Tension

### Continuum Surface Force (CSF)

Surface tension implemented via curvature:

```c
#include "tension.h"

double sigma = 0.072;  // Surface tension (N/m)

event acceleration(i++) {
  // Add surface tension to momentum equation
  // F_st = σ κ δ_s n
  // Where κ is curvature, δ_s is interface delta function
}
```

### Curvature Calculation

Height function method (more accurate than normal-based):

```c
scalar kappa[];

event curvature_calc(i++) {
  curvature(f, kappa);

  // kappa = ∇·n where n is interface normal
  // Positive for convex (liquid side)
  // Negative for concave
}
```

### Marangoni Effects

Can include temperature or concentration gradients:

```c
// Surface tension variation
event marangoni(i++) {
  foreach() {
    if (interfacial(cs, fs)) {
      // dσ/dT · ∇T
      sigma_local = sigma0 - dsigma_dT * (T[] - T0);
    }
  }
}
```

## Contact Line Dynamics

### Contact Angle Implementation

Static contact angle at solid boundaries:

```c
#include "contact.h"

// Contact angle (degrees)
double theta_contact = 90.0;

event contact_angle(i++) {
  contact_angle_static(f, cs, fs, theta_contact);
}
```

### Contact Angle Hysteresis

Dynamic advancing/receding angles:

```c
double theta_advancing = 120.0;
double theta_receding = 60.0;

event contact_hysteresis(i++) {
  foreach() {
    if (interfacial(cs, fs)) {
      // Determine if advancing or receding
      double ux = u.x[];

      if (ux > 0) {
        // Advancing contact line
        apply_contact_angle(f, cs, fs, theta_advancing);
      } else if (ux < 0) {
        // Receding contact line
        apply_contact_angle(f, cs, fs, theta_receding);
      }
    }
  }
}
```

### Sharp Interface Treatment

The custom `embed_contact.h` ensures:
- Sharp interface at contact line
- No artificial smearing
- Conservative volume advection
- Consistent with hysteresis model

## Adaptive Mesh Refinement

### Automatic Refinement

Basilisk automatically refines/coarsens the grid:

```c
#define MAXLEVEL 10  // Maximum refinement level

event adapt(i++) {
  // Refine based on interface
  adapt_wavelet({f}, (double[]){0.01}, MAXLEVEL);

  // Refine based on velocity
  adapt_wavelet({u.x, u.y}, (double[]){0.1, 0.1}, MAXLEVEL);
}
```

**Refinement criteria**:
- Interface location (VOF field)
- Velocity gradients
- Vorticity
- Error estimation

### Wavelet Error Estimation

```c
event adapt(i++) {
  // Wavelet-based error estimation
  // Refine where error > threshold
  adapt_wavelet({f, u.x, u.y},
                (double[]){0.01, 0.1, 0.1},
                maxlevel = MAXLEVEL,
                minlevel = 4);
}
```

**Parameters**:
- First argument: list of fields to monitor
- Second argument: error thresholds for each field
- `maxlevel`: maximum refinement level
- `minlevel`: minimum refinement level

### Grid Levels

Quadtree hierarchy:

```
Level  Cells     Resolution  Cell size (L=1)
-----  -----     ----------  ---------------
0      1×1       1           1.0
1      2×2       2           0.5
2      4×4       4           0.25
3      8×8       8           0.125
...
n      2^n×2^n   2^n         2^(-n)
```

For MAXLEVEL=10: up to 1024×1024 effective resolution.

### Refinement Near Boundaries

```c
event adapt(i++) {
  // Refine interface
  adapt_wavelet({f}, (double[]){0.01}, MAXLEVEL);

  // Also refine near solid boundaries
  foreach() {
    if (cs[] > 0 && cs[] < 1) {
      // Force refinement at embedded boundary
      refine_cell(point, maxlevel = MAXLEVEL);
    }
  }
}
```

## Visualization and Output

### Movie Generation

```c
#include "view.h"

event movies(t += 0.01) {
  view(fov = 10, samples = 2);

  // Draw VOF interface
  draw_vof("f", lw = 2);

  // Draw embedded boundary
  draw_vof("cs", fc = {0.5, 0.5, 0.5});

  // Color by velocity
  squares("u.x", linear = true);

  save("movie.mp4");
}
```

### Field Output

```c
event snapshots(t += 0.1) {
  char name[80];
  sprintf(name, "field-%g", t);

  // Output all fields
  FILE *fp = fopen(name, "w");
  output_field({f, u.x, u.y, p, cs}, fp);
  fclose(fp);
}
```

### Interface Output

```c
event interface_output(t += 0.05) {
  char name[80];
  sprintf(name, "interface-%g.dat", t);

  FILE *fp = fopen(name, "w");
  output_facets(f, fp);
  fclose(fp);
}
```

### Diagnostics

```c
event logfile(i++) {
  // Volume conservation
  double volume = statsf(f).sum;

  // Kinetic energy
  double ke = 0.;
  foreach(reduction(+:ke)) {
    ke += 0.5 * rho[] * (sq(u.x[]) + sq(u.y[])) * dv();
  }

  // Output to stderr
  fprintf(stderr, "%g %g %g\n", t, volume, ke);
}
```

## Performance Optimization

### Parallel Execution (MPI)

```c
// Compile with MPI
// qcc -D_MPI=1 -o program program.c -lm

// Run with MPI
// mpirun -np 8 ./program
```

Basilisk automatically:
- Partitions domain
- Handles communication
- Load balances

### Memory Management

Adaptive grid reduces memory:

```
Base grid (64×64):     ~4K cells
Refined (MAXLEVEL=10): ~50-200K cells (varies)
Uniform (1024×1024):   ~1M cells

Memory savings: 5-20x
```

### Timestep Control

CFL-based adaptive timestepping:

```c
// Automatically adjusted each step
// dt = CFL * min(Δx/|u|)

// Can override
event timestep(i++) {
  dt = min(dt, 0.01);  // Limit maximum timestep
}
```

## Key Algorithms

### 1. VOF Advection

Piecewise-linear interface calculation (PLIC):
1. Reconstruct interface in each cell
2. Advect interface segments
3. Update volume fractions conservatively

### 2. Poisson Solver

Multigrid solver for pressure:
- V-cycle or full multigrid
- Geometric coarsening
- Red-black Gauss-Seidel smoothing

### 3. Height Function

For curvature calculation:
1. Compute height from VOF
2. Fit parabola to heights
3. Curvature from parabola

More accurate than normal-based methods.

## Further Reading

### Official Basilisk Documentation

- **Main site**: http://basilisk.fr/
- **Tutorial**: http://basilisk.fr/Tutorial
- **Examples**: http://basilisk.fr/src/examples/
- **VOF examples**: http://basilisk.fr/src/examples/#volume-of-fluid-vof
- **Navier-Stokes**: http://basilisk.fr/src/navier-stokes/centered.h
- **Two-phase**: http://basilisk.fr/src/two-phase.h

### Project Documentation

- [README.md](README.md) - Project overview
- [BASILISK_INSTALL.md](BASILISK_INSTALL.md) - Installation guide
- [BASILISK_CONFIG.md](BASILISK_CONFIG.md) - Configuration guide
- [AXISYMMETRIC_GUIDE.md](AXISYMMETRIC_GUIDE.md) - Axisymmetric coordinates
- [DROPLET_IMPACT_ORIFICE.md](DROPLET_IMPACT_ORIFICE.md) - Simulation details

### Research Paper

Huang, C.-S., Han, T.-Y., Zhang, J., & Ni, M.-J. (2025). "A 2D sharp and conservative VOF method for modeling the contact line dynamics with hysteresis on complex boundary." *Journal of Computational Physics*. https://doi.org/10.1016/j.jcp.2025.113975

---

**Summary**: This project demonstrates advanced Basilisk C capabilities including VOF interface tracking, embedded boundaries, adaptive mesh refinement, and contact line dynamics. The custom embedded boundary modules extend Basilisk's standard capabilities to handle sharp, conservative VOF with contact angle hysteresis on complex geometries.
