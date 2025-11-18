# Non-Dimensional Formulation for Droplet Impact Simulations

## Overview

This document explains the non-dimensional formulation used in the axisymmetric droplet impact simulations. Non-dimensionalization reduces the number of independent parameters and makes results more general and comparable across different physical scales.

## Key Principle: Drop Diameter D = 1

In the non-dimensional formulation, **all lengths are scaled by the drop diameter D**, so **D* = 1**.

## Non-Dimensionalization Scheme

### Characteristic Scales

We use the following characteristic scales:

| Quantity | Symbol | Characteristic Scale |
|----------|--------|---------------------|
| Length | L | Drop diameter D |
| Velocity | U | Impact velocity U |
| Time | T | D/U |
| Pressure | P | ρ₁U² |
| Density | ρ | Liquid density ρ₁ |

### Non-Dimensional Variables

All variables are non-dimensionalized as follows:

```
x* = x/D           (spatial coordinates)
y* = y/D
r* = r/D

u* = u/U           (velocity)
v* = v/U

t* = tU/D          (time)

p* = p/(ρ₁U²)      (pressure)

ρ* = ρ/ρ₁          (density)
```

### Non-Dimensional Fluid Properties

In Basilisk, we set:

```c
// Reference fluid (liquid droplet)
rho1 = 1.0;              // ρ₁* = 1
mu1  = 1.0/RE;           // μ₁* = 1/Re

// Ambient fluid (gas)
rho2 = RHO_RATIO;        // ρ₂* = ρ₂/ρ₁
mu2  = MU_RATIO/RE;      // μ₂* = (μ₂/μ₁)/Re

// Interfacial properties
f.sigma = 1.0/WE;        // σ* = 1/We

// Gravity
g* = BO/WE;              // g* = gD/U² = (ρgD²/σ)/(ρU²D/σ) = Bo/We
```

## Dimensionless Numbers

The physics is characterized by the following dimensionless groups:

### Reynolds Number (Re)
```
Re = ρ₁UD/μ₁
```
- **Physical meaning**: Ratio of inertial to viscous forces
- **Typical range**:
  - Re < 1: Viscous dominated (Stokes flow)
  - Re > 1000: Inertia dominated
  - Water droplet (D=2mm, U=1m/s): Re ≈ 2000

### Weber Number (We)
```
We = ρ₁U²D/σ
```
- **Physical meaning**: Ratio of inertial to surface tension forces
- **Typical range**:
  - We < 1: Surface tension dominated
  - We > 100: Inertia dominated
  - Water droplet (D=2mm, U=1m/s): We ≈ 28

### Ohnesorge Number (Oh)
```
Oh = μ₁/√(ρ₁σD) = √We/Re
```
- **Physical meaning**: Ratio of viscous to surface tension forces
- **Useful property**: Independent of velocity
- **Typical values**:
  - Water: Oh ≈ 0.001-0.01
  - Viscous liquids: Oh > 0.1

### Bond Number (Bo)
```
Bo = ρ₁gD²/σ
```
- **Physical meaning**: Ratio of gravitational to surface tension forces
- **Typical range**:
  - Bo < 1: Surface tension dominated (spherical drops)
  - Bo > 10: Gravity dominated (flattened drops)
  - Water droplet (D=2mm): Bo ≈ 0.5

### Density Ratio (ρ*)
```
ρ* = ρ₂/ρ₁
```
- **Physical meaning**: Ratio of ambient to droplet density
- **Typical values**:
  - Air/water: ρ* ≈ 0.001
  - Liquid/liquid: ρ* ≈ 0.5-2.0

### Viscosity Ratio (μ*)
```
μ* = μ₂/μ₁
```
- **Physical meaning**: Ratio of ambient to droplet viscosity
- **Typical values**:
  - Air/water: μ* ≈ 0.02
  - Oil/water: μ* ≈ 10-100

## Axisymmetric Coordinates and the axi.h Header

### Including axi.h

In Basilisk, axisymmetric simulations require the `axi.h` header:

```c
#include "axi.h"
```

This header file defines the metric terms for axisymmetric coordinates:
- Coordinate system: (x, y) → (r, z) where r is radial, z is axial
- Volume element: dV = 2πr·dr·dz·cm[]
- Metric factors: `cm[]` (cell metric), `fm[]` (face metric)

### Important Notes on axi.h

1. **Convention**: In Basilisk, `x` corresponds to the radial coordinate r, and `y` corresponds to the axial coordinate z
2. **Origin**: The axis of symmetry is at x = 0 (r = 0)
3. **Volume calculations**: Always use `dv()` which includes the metric: `dv() = 2πr·Δr·Δz·cm[]`
4. **Volume integrals**: For correct volume, use `dv()/cm[]` in reductions:
   ```c
   foreach(reduction(+:volume)) {
     volume += f[] * dv();  // Correct: includes 2πr
   }
   ```

### Boundary Conditions at Axis of Symmetry

At r = 0 (left boundary), enforce axisymmetric conditions:

```c
u.n[left] = dirichlet(0.);  // u_r = 0 (no radial flow through axis)
u.t[left] = neumann(0.);    // ∂u_z/∂r = 0 (symmetry)
f[left]   = neumann(0.);    // ∂f/∂r = 0 (symmetry)
```

## Converting from Dimensional to Non-Dimensional

### Example: Water Droplet Impact

**Dimensional parameters:**
- Drop diameter: D = 2 mm = 0.002 m
- Impact velocity: U = 1 m/s
- Liquid density: ρ₁ = 1000 kg/m³
- Gas density: ρ₂ = 1.2 kg/m³
- Liquid viscosity: μ₁ = 0.001 Pa·s
- Gas viscosity: μ₂ = 1.8×10⁻⁵ Pa·s
- Surface tension: σ = 0.072 N/m
- Gravity: g = 9.8 m/s²

**Calculate dimensionless numbers:**

```
Re = ρ₁UD/μ₁ = 1000 × 1.0 × 0.002 / 0.001 = 2000

We = ρ₁U²D/σ = 1000 × 1.0² × 0.002 / 0.072 = 27.8

Bo = ρ₁gD²/σ = 1000 × 9.8 × 0.002² / 0.072 = 0.544

ρ* = ρ₂/ρ₁ = 1.2 / 1000 = 0.0012

μ* = μ₂/μ₁ = 1.8×10⁻⁵ / 0.001 = 0.018
```

**Non-dimensional code:**

```c
#define RE        2000.0
#define WE        27.8
#define BO        0.544
#define RHO_RATIO 0.0012
#define MU_RATIO  0.018

// Geometry (non-dimensional)
#define D_DROP    1.0      // Drop diameter = 1
#define R_DROP    0.5      // Drop radius = 0.5

// Set fluid properties
rho1 = 1.0;
rho2 = RHO_RATIO;
mu1 = 1.0/RE;
mu2 = MU_RATIO/RE;
f.sigma = 1.0/WE;

// Gravity
const face vector g[] = {0., -BO/WE};
```

### Converting Geometric Dimensions

If your plate has:
- Orifice diameter: d = 0.8 mm (dimensional)
- Plate thickness: t = 0.4 mm (dimensional)

Non-dimensional versions:
```
d* = d/D = 0.8/2.0 = 0.4
t* = t/D = 0.4/2.0 = 0.2
```

### Converting Time

If simulation runs for t_dim = 0.01 s (dimensional):

```
t* = t_dim × U/D = 0.01 × 1.0 / 0.002 = 5.0
```

So set `TEND = 5.0` in your code.

### Converting Results Back

To convert non-dimensional results back to physical units:

```
x_dim = x* × D
t_dim = t* × D/U
u_dim = u* × U
p_dim = p* × ρ₁U²
```

## Time Scales

### Impact Time Scale
```
T_impact = D/U
```
This is the natural time scale used in the non-dimensionalization.

### Gravity Time Scale
```
T_gravity = √(ρ₁D/((ρ₁-ρ₂)g))
```
In non-dimensional form:
```
T_g* = √(We/(Bo(1-ρ*)))
```

### Capillary Time Scale
```
T_cap = √(ρ₁D³/σ)
```
In non-dimensional form:
```
T_c* = √We
```

## Advantages of Non-Dimensionalization

1. **Fewer parameters**: Instead of 9 dimensional parameters, only 5-6 dimensionless numbers
2. **Generality**: Results apply to any system with the same Re, We, Bo
3. **Numerical stability**: Order-of-magnitude unity values
4. **Physical insight**: Dimensionless numbers reveal dominant physics
5. **Validation**: Easy comparison with experiments at different scales

## Files in This Repository

- `droplet-impact-orifice-nondim.c`: Non-dimensional droplet impact simulation
- `droplet-impact-sharp-orifice-nondim.c`: Non-dimensional sharp orifice simulation
- `droplet-impact-orifice.c`: Original dimensional version (for reference)
- `droplet-impact-sharp-orifice.c`: Original dimensional sharp orifice version

## References

1. Popinet, S. (2009). An accurate adaptive solver for surface-tension-driven interfacial flows. Journal of Computational Physics, 228(16), 5838-5866.
2. Basilisk documentation: http://basilisk.fr/
3. Basilisk axi.h: http://basilisk.fr/src/axi.h

## Summary

**Key Point**: In non-dimensional simulations, **D = 1** (drop diameter equals one), and all physics is controlled by the dimensionless numbers Re, We, Bo, ρ*, and μ*.

The `axi.h` header automatically handles the axisymmetric metric terms - you don't need to manually include it if you use `#include "axi.h"` at the start of your code.
