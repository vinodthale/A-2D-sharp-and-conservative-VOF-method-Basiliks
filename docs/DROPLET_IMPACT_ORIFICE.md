# Axisymmetric Droplet Impact on Flat Plate with Orifice

## Overview

This simulation models the axisymmetric impact of a droplet on a flat plate containing a circular orifice (hole). The code uses the sharp and conservative Volume-of-Fluid (VOF) method from the paper:

> Huang, C.-S., Han, T.-Y., Zhang, J., & Ni, M.-J. (2025). "A 2D sharp and conservative VOF method for modeling the contact line dynamics with hysteresis on complex boundary." Journal of Computational Physics. https://doi.org/10.1016/j.jcp.2025.113975

## File

- **Main simulation**: `droplet-impact-orifice.c`

## Compilation and Execution

### Prerequisites

- Basilisk C (http://basilisk.fr/)
- Required headers from this repository

### Compilation

```bash
# Using qcc (Basilisk compiler)
qcc -O2 -Wall -o droplet-impact-orifice droplet-impact-orifice.c -lm

# For parallel execution with MPI
qcc -O2 -Wall -D_MPI=1 -o droplet-impact-orifice droplet-impact-orifice.c -lm
```

### Execution

```bash
# Serial execution
./droplet-impact-orifice 2> log

# Parallel execution (e.g., 4 processors)
mpirun -np 4 ./droplet-impact-orifice 2> log
```

### Output

The simulation produces several output files:

1. **log**: Contains time-series data with columns:
   - Time (s)
   - Volume ratio (V/V₀) - should remain ~1.0 for volume conservation
   - Droplet center of mass x-coordinate (m)
   - Droplet center of mass y-coordinate (m)
   - Maximum radial velocity (m/s)
   - Maximum axial velocity (m/s)

2. **out-[time]**: Interface facet data at different time steps
3. **field-[time]**: Complete field data (position, velocity, pressure, VOF fraction)
4. **movie.mp4**: Visualization of the droplet impact process

## Adjustable Parameters

### Physical Parameters (in code header)

| Parameter | Default | Description | How to adjust |
|-----------|---------|-------------|---------------|
| `thetac` | 90° | Contact angle | 0° = perfect wetting, 180° = no wetting |
| `impact_velocity` | -1.0 m/s | Droplet impact speed | Negative = downward |
| `r_orifice` | 0.4×r_drop | Orifice radius | Change to adjust hole size |
| `r_drop` | 1 mm | Droplet radius | Reference length scale |
| `tend` | 0.01 s | Simulation end time | Increase for longer runs |
| `MAXLEVEL` | 8 | Maximum mesh refinement | Increase for finer resolution |

### Geometry Parameters

```c
#define h_plate   1.0*r_drop   // Vertical position of plate bottom
#define t_plate   0.2*r_drop   // Plate thickness
#define r_orifice 0.4*r_drop   // Orifice radius (KEY PARAMETER)
#define h_drop0   (h_plate + t_plate + 2.0*r_drop) // Initial droplet height
```

### Fluid Properties

Default values are for water droplet impacting into air:

```c
#define rho01     1000.    // Liquid density (kg/m³)
#define rho02     1.2      // Gas density (kg/m³)
#define mu01      1e-3     // Liquid viscosity (Pa·s)
#define mu02      1.8e-5   // Gas viscosity (Pa·s)
#define sigma0    0.072    // Surface tension (N/m)
```

## Example Modifications

### 1. Smaller orifice (hole radius = 20% of droplet radius)

```c
#define r_orifice 0.2*r_drop
```

### 2. Larger orifice (hole radius = 60% of droplet radius)

```c
#define r_orifice 0.6*r_drop
```

### 3. Higher impact velocity

```c
double impact_velocity = -2.0;  // 2 m/s downward
```

### 4. Hydrophobic surface (contact angle 120°)

```c
double thetac = 120.;
```

### 5. Hydrophilic surface (contact angle 60°)

```c
double thetac = 60.;
```

### 6. Finer mesh resolution

```c
#define MAXLEVEL  9  // or 10 for very fine mesh (slower)
```

## Physical Interpretation

### Axisymmetric Coordinates

The simulation uses axisymmetric coordinates where:
- **x-axis**: radial direction (r) from axis of symmetry
- **y-axis**: vertical/axial direction (z)
- The domain represents one half-plane of the full 3D problem
- The visualization mirrors the solution to show the full 3D appearance

### Geometry

```
                    ← Droplet (radius r_drop)
    |
    |               ↓ impact_velocity
    |            ___
    |           /   \
    |          |  ●  |
    |           \___/
    |
    |
    |  ═════════════════  ← Top of plate (y = h_plate + t_plate)
    |  ║           ║
    |  ║  Orifice  ║      ← Plate (thickness t_plate)
    |  ║           ║
    |  ═════════════════  ← Bottom of plate (y = h_plate)
    |
    └─────────────────────
    r=0 (axis)      r →
```

### Non-dimensional Numbers

For typical parameters, the simulation captures:

**Reynolds number**: Re = ρ V D / μ
- With default values: Re ≈ 2000 (inertia-dominated)

**Weber number**: We = ρ V² D / σ
- With default values: We ≈ 28 (moderate inertia vs. surface tension)

**Capillary number**: Ca = μ V / σ
- With default values: Ca ≈ 0.014 (surface tension dominated)

## Troubleshooting

### Volume not conserved (V/V₀ deviates significantly from 1.0)

- Increase mesh resolution: `MAXLEVEL 9` or `10`
- Decrease time step if using fixed time stepping
- Check that `csTL` parameter is appropriate for small cut cells

### Simulation runs very slowly

- Decrease `MAXLEVEL` (8 is reasonable, 7 for faster tests)
- Reduce domain size `l0` if possible
- Use MPI parallel execution
- Decrease `tend` for shorter simulation time

### Droplet doesn't interact with plate

- Check initial height `h_drop0` is above plate
- Verify impact velocity is negative (downward)
- Ensure simulation time `tend` is long enough

### Contact line behavior looks incorrect

- Verify contact angle `thetac` is set correctly
- Check that contact angle implementation is working (see embed_contact.h)
- Ensure mesh is fine enough near contact line

## Visualization

The code generates `movie.mp4` showing:
- **Gray**: Solid plate
- **Blue**: Liquid droplet
- **Grid lines**: Adaptive mesh (finer near interfaces)
- **Mirrored**: Shows full axisymmetric result

Additional visualization can be created using the field data:

```bash
# Plot interface position
awk '{if ($7 > 0.5 && $7 < 1.0) print $1, $2}' field-0.005 > interface.dat

# Plot velocity field
awk '{print $1, $2, $3, $4}' field-0.005 > velocity.dat
```

## References

1. Huang, C.-S., Han, T.-Y., Zhang, J., & Ni, M.-J. (2025). Journal of Computational Physics. https://doi.org/10.1016/j.jcp.2025.113975
2. Basilisk C: http://basilisk.fr/
3. Original sandbox: https://basilisk.dalembert.upmc.fr/sandbox/Chongsen/

## Contact and Support

For issues specific to this implementation, please refer to the original paper or the Basilisk documentation.
