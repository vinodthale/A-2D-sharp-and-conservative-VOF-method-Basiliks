# Droplet Impact Simulation Specifications

This document verifies that all simulation parameters match the exact specifications from the research paper.

## Overview

We simulate a droplet falling in an ambient fluid and impacting a plate with an orifice, where inertial and viscous effects are significant. Two scenarios are considered:
1. **Round orifice** - No contact between droplet and plate
2. **Sharp orifice** - Contact line development and hysteresis

## Physical Properties

All values are identical for both cases and match the experimental conditions:

| Property | Symbol | Value | Units | Implementation |
|----------|--------|-------|-------|----------------|
| Droplet density | ρ_d | 1130 | kg/m³ | `RHO_DROPLET = 1130.0` |
| Ambient fluid density | ρ_f | 960 | kg/m³ | `RHO_AMBIENT = 960.0` |
| Droplet viscosity | μ_d | 0.007 | kg/(m·s) | `MU_DROPLET = 0.007` |
| Ambient fluid viscosity | μ_f | 0.048 | kg/(m·s) | `MU_AMBIENT = 0.048` |
| Surface tension | σ | 0.0295 | N/m | `SIGMA = 0.0295` |
| Gravitational acceleration | g | 9.8 | m/s² | `GRAVITY = 9.8` |

✅ **All physical properties match specifications exactly**

## Common Geometry Parameters

| Parameter | Value | Units | Implementation |
|-----------|-------|-------|----------------|
| Liquid pool top distance from plate | 120 | mm | `POOL_HEIGHT = 0.120` m |
| Droplet release height above orifice | 110 | mm | `DROP_RELEASE = 0.110` m |
| Plate distance from right boundary | 12.5 | mm | `PLATE_FROM_RB = 0.0125` m |
| Plate thickness | s = 2 | mm | `PLATE_THICK = 0.002` m |
| Orifice diameter | d = 6 | mm | `D_ORIFICE = 0.006` m |

✅ **All common geometry parameters match specifications exactly**

## Round Orifice Case

### Specific Parameters

| Parameter | Symbol | Value | Units | Implementation |
|-----------|--------|-------|-------|----------------|
| Droplet diameter | D | 9.315 | mm | `D_DROPLET = 0.009315` m |
| Droplet radius | R | 4.6575 | mm | `R_DROPLET = 0.0046575` m |
| Bond number | Bo | 4.9 | - | Calculated from properties |
| d/D ratio | - | 0.644 | - | 6/9.315 = 0.644 ✓ |
| Contact angle | θ_s | 180 | degrees | `THETA_CONTACT = 180.0` |
| Orifice edge type | - | Round | - | Semicircle with diameter s = 2 mm |

### Bond Number Verification
Bo = (ρ_l - ρ_g) * g * D² / σ
- = (1130 - 960) * 9.8 * (0.009315)² / 0.0295
- = 170 * 9.8 * 0.00008677 / 0.0295
- = 0.1445 / 0.0295
- = **4.9** ✅

### Time Scale
t_g = √(ρ_l * D / ((ρ_l - ρ_g) * g))
- = √(1130 * 0.009315 / (170 * 9.8))
- = √(10.5260 / 1666)
- = √0.006317
- = **0.0795 s** = **79.5 ms**

### Dimensionless Time Points

| Label | t/t_g | Time (ms) |
|-------|-------|-----------|
| t₀ | 0 | 0 |
| t₁ | 0.54 | 42.9 |
| t₂ | 1.27 | 101.0 |
| t₃ | 1.81 | 143.9 |
| t₄ | 2.44 | 194.0 |
| t₅ | 2.96 | 235.3 |

✅ **Round orifice parameters match specifications exactly**

## Sharp Orifice Case

### Specific Parameters

| Parameter | Symbol | Value | Units | Implementation |
|-----------|--------|-------|-------|----------------|
| Droplet diameter | D | 10.307 | mm | `D_DROPLET = 0.010307` m |
| Droplet radius | R | 5.1535 | mm | `R_DROPLET = 0.0051535` m |
| Bond number | Bo | 6.0 | - | Calculated from properties |
| d/D ratio | - | 0.58 | - | 6/10.307 = 0.582 ≈ 0.58 ✓ |
| Receding contact angle | θ_r | 42 | degrees | `THETA_RECEDING = 42.0` |
| Advancing contact angle | θ_a | 68 | degrees | `THETA_ADVANCING = 68.0` |
| Advancing angle at edge | θ_a | 150 | degrees | `THETA_PINNING = 150.0` |
| Orifice edge type | - | Sharp | - | 90-degree corners (no rounding) |

**Note:** Experimental values were θ_a = 68° and θ_r = 46° ± 4°. We use θ_r = 42° as specified.

### Bond Number Verification
Bo = (ρ_l - ρ_g) * g * D² / σ
- = (1130 - 960) * 9.8 * (0.010307)² / 0.0295
- = 170 * 9.8 * 0.00010623 / 0.0295
- = 0.1768 / 0.0295
- = **5.99** ≈ **6.0** ✅

### Time Scale
t_g = √(ρ_l * D / ((ρ_l - ρ_g) * g))
- = √(1130 * 0.010307 / (170 * 9.8))
- = √(11.6469 / 1666)
- = √0.006990
- = **0.0836 s** = **83.6 ms**

### Passage Time Scale
t_i = D³ / (U_i * d²)

Where U_i is the falling velocity 10 ms before impact. Using free fall from height h = 110 mm:
- U_i = √(2 * g * h) = √(2 * 9.8 * 0.110) = **1.47 m/s**
- t_i = (0.010307)³ / (1.47 * 0.006²)
- t_i = 1.095×10⁻⁶ / (1.47 * 3.6×10⁻⁵)
- t_i = 1.095×10⁻⁶ / 5.292×10⁻⁵
- t_i = **0.0207 s** = **20.7 ms**

### Dimensionless Time Points

| Label | t/t_g | t/t_i | Time (ms) |
|-------|-------|-------|-----------|
| t₁ | 0.49 | 1.98 | 41.0 |
| t₂ | 0.73 | 2.95 | 61.0 |
| t₃ | 0.90 | 3.64 | 75.2 |
| t₄ | 1.04 | 4.21 | 86.9 |

✅ **Sharp orifice parameters match specifications exactly**

## Mesh Resolution

| Parameter | Value | Implementation |
|-----------|-------|----------------|
| Mesh size | Δ = 3.91×10⁻² mm | `MAXLEVEL = 12` |
| Grid resolution | ≈ 120 cells per radius | Verified below |

### Mesh Verification (Round Orifice)
- Droplet radius: R = 4.6575 mm = 4.6575×10⁻³ m
- Mesh size: Δ = L₀/2^MAXLEVEL = 0.160/4096 = **3.906×10⁻⁵ m** = **0.03906 mm**
- Cells per radius: R/Δ = 4.6575×10⁻³ / 3.906×10⁻⁵ = **119.2** ≈ **120 cpr** ✅

### Mesh Verification (Sharp Orifice)
- Droplet radius: R = 5.1535 mm = 5.1535×10⁻³ m
- Mesh size: Δ = 3.906×10⁻⁵ m = 0.03906 mm
- Cells per radius: R/Δ = 5.1535×10⁻³ / 3.906×10⁻⁵ = **131.9** ≈ **132 cpr** ✅

**Note:** Both cases achieve the target resolution of ~120 cells per radius.

## Domain Setup

| Parameter | Value | Implementation |
|-----------|-------|----------------|
| Domain size | 160 × 160 mm² | `L0 = 0.160` m |
| Coordinate system | 2D axisymmetric | `#include "axi.h"` |
| Origin | (0, 0) | Bottom-left corner |
| Minimum refinement level | 4 | `MINLEVEL = 4` |
| Maximum refinement level | 12 | `MAXLEVEL = 12` |

## Boundary Conditions

### Round Orifice
- Plate surface: No-slip boundary condition
- Contact angle: θ = 180° (enforced to avoid contact)
- Orifice edge: Semicircular with radius = s/2 = 1 mm

### Sharp Orifice
- Plate surface: No-slip boundary condition
- Contact angle hysteresis: θ_r = 42°, θ_a = 68° (general)
- Sharp edge: θ_a = 150° (for contact line pinning)
- Orifice edge: Sharp 90° corners

## Output Files

### Round Orifice
- `volume_round.txt`: Volume conservation data (Time, Time/T_g, Volume, V/V₀)
- `interface_position_round.txt`: Leading and trailing interface positions
- `snapshot_round_*.dat`: Interface snapshots at specific times
- `field_round_*.dat`: Full field data at specific times
- `movie_round.ppm`: Animation of simulation

### Sharp Orifice
- `volume_sharp.txt`: Volume conservation data
- `interface_position_sharp.txt`: Points M and N tracking (Time, Time/T_g, Time/T_i, Leading_Y, Trailing_Y)
- `snapshot_sharp_*.dat`: Interface snapshots at specific times
- `field_sharp_*.dat`: Full field data at specific times
- `movie_sharp.ppm`: Animation of simulation

## Numerical Methods

Both simulations use:
- **VOF method**: Sharp and conservative interface tracking
- **Embedded boundaries**: For plate geometry
- **Surface tension**: Continuum surface force method
- **Adaptive mesh refinement**: Wavelet-based adaptation
- **Contact angle modeling**:
  - Round: Static angle (180°)
  - Sharp: Hysteresis model with pinning
- **Two-phase flow**: Navier-Stokes solver with variable density/viscosity

## Compilation and Execution

```bash
# Compile both simulations
./compile-droplet-impact.sh

# Run round orifice simulation
./droplet-impact-round-orifice

# Run sharp orifice simulation
./droplet-impact-sharp-orifice
```

## Summary

✅ **All physical properties match specifications exactly**
✅ **All geometric parameters match specifications exactly**
✅ **Bond numbers verified: Bo = 4.9 (round), Bo = 6.0 (sharp)**
✅ **Mesh resolution verified: ~120 cells per radius**
✅ **Contact angles configured correctly for both cases**
✅ **Time scales calculated and verified**
✅ **Output files configured for comparison with experiments**

**Both simulations are ready to run with exact specifications from the research paper.**
