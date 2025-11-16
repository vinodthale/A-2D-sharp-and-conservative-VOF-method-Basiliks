# A 2D Sharp and Conservative VOF Method

This repository contains Basilisk C implementations of the sharp and conservative Volume-of-Fluid (VOF) method for modeling contact line dynamics with hysteresis on complex boundaries.

## Reference

Based on the paper:
> Huang, C.-S., Han, T.-Y., Zhang, J., & Ni, M.-J. (2025). "A 2D sharp and conservative VOF method for modeling the contact line dynamics with hysteresis on complex boundary." *Journal of Computational Physics*. https://doi.org/10.1016/j.jcp.2025.113975

Original code: https://basilisk.dalembert.upmc.fr/sandbox/Chongsen/

## Simulations

### 1. Droplet Spreading on Cylinder (Original)

**File**: `circle-droplet.c`

Simulates a droplet spreading on a cylindrical surface of similar size. Demonstrates the contact line dynamics with contact angle hysteresis on curved boundaries.

**Features**:
- 2D Cartesian coordinates with mirroring
- Embedded boundary method for cylinder
- Contact angle: 120° (adjustable)
- Adaptive mesh refinement

### 2. Axisymmetric Droplet Impact on Plate with Orifice (New)

**File**: `droplet-impact-orifice.c`

Simulates the axisymmetric impact of a droplet on a flat plate containing a circular orifice (hole). Models realistic droplet impact scenarios with penetration through the orifice.

**Features**:
- Axisymmetric coordinates (computational efficiency)
- Flat plate with circular orifice geometry
- Impact dynamics with adjustable velocity
- Contact angle control (wetting behavior)
- Detailed diagnostics and visualization

**Documentation**: See [DROPLET_IMPACT_ORIFICE.md](DROPLET_IMPACT_ORIFICE.md) for detailed instructions.

**Key Parameters**:
- Droplet radius: 1 mm (adjustable)
- Impact velocity: 1 m/s (adjustable)
- Orifice radius: 0.4 mm (adjustable)
- Contact angle: 90° (adjustable)
- Fluid properties: Water/air (adjustable)

## Quick Start

### Prerequisites

- Basilisk C (http://basilisk.fr/)
- C compiler (gcc recommended)
- MPI (optional, for parallel execution)

### Compilation

```bash
# Original cylinder simulation
qcc -O2 -Wall -o circle-droplet circle-droplet.c -lm

# New droplet impact simulation
qcc -O2 -Wall -o droplet-impact-orifice droplet-impact-orifice.c -lm
```

### Execution

```bash
# Run simulation and save log
./droplet-impact-orifice 2> log

# View results
tail -f log  # Monitor progress
ls *.mp4     # Check for video output
```

## File Structure

```
.
├── README.md                      # This file
├── DROPLET_IMPACT_ORIFICE.md     # Detailed documentation for droplet impact
├── circle-droplet.c               # Original: droplet on cylinder
├── droplet-impact-orifice.c       # New: droplet impact on plate with orifice
├── myembed.h                      # Embedded boundary utilities
├── embed_contact.h                # Contact line dynamics
├── embed_two-phase.h              # Two-phase flow solver
├── embed_tension.h                # Surface tension
├── embed_vof.h                    # VOF advection
├── embed_curvature.h              # Interface curvature
├── embed_heights.h                # Height function method
├── embed_height_normal.h          # Normal calculation
├── embed_correct_height.h         # Height correction
├── embed_iforce.h                 # Interfacial forces
├── TPR2D.h                        # Two-phase reconstruction
└── tmp_fraction_field.h           # Temporary field storage
```

## Method Overview

The sharp and conservative VOF method combines:

1. **Volume-of-Fluid (VOF)**: Interface tracking using volume fraction
2. **Embedded Boundary Method**: Complex solid geometry representation
3. **Contact Line Dynamics**: Realistic contact angle implementation with hysteresis
4. **Height Function Method**: Accurate curvature calculation
5. **Conservative Advection**: Mass-conserving interface transport

### Key Features

- **Sharp interface**: Maintains interface sharpness without artificial smearing
- **Conservative**: Preserves liquid volume to machine precision
- **Contact angle**: Implements static and dynamic contact angles
- **Adaptive mesh**: Automatic refinement near interfaces and boundaries
- **Complex geometry**: Handles arbitrary embedded boundaries

## Modifying Simulations

### Changing Contact Angle

```c
double thetac = 90.;  // Change to desired angle (0-180°)
```

### Adjusting Impact Velocity

```c
double impact_velocity = -2.0;  // Change magnitude (negative = downward)
```

### Resizing the Orifice

```c
#define r_orifice 0.3*r_drop  // Change multiplier (0.1-0.9)
```

### Increasing Resolution

```c
#define MAXLEVEL  9  // Increase for finer mesh (7-11 typical)
```

## Output and Visualization

### Log File Format

The standard error output (redirected to `log`) contains:
```
time  volume_ratio  center_x  center_y  max_u_radial  max_u_axial
```

### Visualization Files

- `movie.mp4`: Animated visualization
- `out-[time]`: Interface facets (for post-processing)
- `field-[time]`: Complete field data (velocity, pressure, VOF, solid fraction)

### Example Analysis

```bash
# Plot volume conservation
gnuplot -e "plot 'log' using 1:2 with lines; pause -1"

# Extract interface at t=0.005
awk '$7>0.5 && $7<1.0 {print $1,$2}' field-0.005 > interface.dat
```

## Contributing

This code is based on research work. For modifications or extensions, please:

1. Cite the original paper (Huang et al., 2025)
2. Reference the Basilisk C framework
3. Document your changes clearly

## License

Please refer to the Basilisk C license and cite the original paper when using this code.

## References

1. Huang, C.-S., Han, T.-Y., Zhang, J., & Ni, M.-J. (2025). "A 2D sharp and conservative VOF method for modeling the contact line dynamics with hysteresis on complex boundary." *Journal of Computational Physics*. https://doi.org/10.1016/j.jcp.2025.113975

2. Basilisk C: http://basilisk.fr/

3. Original sandbox: https://basilisk.dalembert.upmc.fr/sandbox/Chongsen/ 
