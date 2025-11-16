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

### Step 1: Install Basilisk C

**New to Basilisk?** See our comprehensive guides:

- **[BASILISK_INSTALL.md](BASILISK_INSTALL.md)** - Complete installation guide for Linux, macOS, and Windows (WSL)
- **[setup-basilisk.sh](setup-basilisk.sh)** - Automated installation script

**Quick install** (Linux/Ubuntu):
```bash
# Automated installation
./setup-basilisk.sh

# Or manual installation
sudo apt install darcs gcc make gawk
darcs clone http://basilisk.fr/basilisk ~/basilisk
cd ~/basilisk/src && ln -s config.gcc config && make
export BASILISK=$HOME/basilisk
export PATH=$PATH:$BASILISK
```

**Verify installation**:
```bash
make check-basilisk
```

### Step 2: Compile Simulations

**Using Makefile** (recommended):
```bash
# Build all simulations
make

# Build specific simulation
make circle-droplet
make droplet-impact-orifice

# High resolution build
make high-res MAXLEVEL=10

# With MPI support
make mpi
```

**Manual compilation**:
```bash
# Original cylinder simulation
qcc -O2 -Wall -o circle-droplet circle-droplet.c -lm

# Droplet impact simulation
qcc -O2 -Wall -o droplet-impact-orifice droplet-impact-orifice.c -lm

# High resolution
qcc -O3 -DMAXLEVEL=10 -o droplet-impact-orifice droplet-impact-orifice.c -lm
```

**See**: [BASILISK_CONFIG.md](BASILISK_CONFIG.md) for compiler flags and optimization options

### Step 3: Run Simulations

```bash
# Run simulation and save log
./droplet-impact-orifice 2> log

# Run in background
./droplet-impact-orifice 2> log &

# Monitor progress
tail -f log

# View results
ls *.mp4     # Check for video output
ls field-*   # Check for field data
```

## File Structure

```
.
├── README.md                          # This file - Project overview
│
├── Documentation/
│   ├── BASILISK_INSTALL.md            # Basilisk installation guide
│   ├── BASILISK_CONFIG.md             # Basilisk configuration and compiler flags
│   ├── BASILISK_FEATURES.md           # Basilisk features used in this project
│   ├── DROPLET_IMPACT_ORIFICE.md      # Droplet impact simulation details
│   ├── DROPLET_IMPACT_SPECS.md        # Simulation specifications
│   ├── README_DROPLET_IMPACT.md       # Additional droplet impact documentation
│   ├── NON-DIMENSIONALIZATION.md      # Non-dimensional formulation
│   └── AXISYMMETRIC_GUIDE.md          # Axisymmetric coordinates guide
│
├── Build System/
│   ├── Makefile                       # Build all simulations
│   ├── setup-basilisk.sh              # Automated Basilisk installation script
│   ├── compile-droplet-impact.sh      # Compile droplet impact simulations
│   ├── run-sharp-orifice.sh           # Run sharp orifice simulation
│   └── run-round-orifice.sh           # Run round orifice simulation
│
├── Simulations/
│   ├── circle-droplet.c               # Original: droplet spreading on cylinder
│   ├── droplet-impact-orifice.c       # Droplet impact (dimensional)
│   ├── droplet-impact-orifice-nondim.c    # Droplet impact (non-dimensional)
│   ├── droplet-impact-sharp-orifice.c     # Sharp edge orifice (dimensional)
│   ├── droplet-impact-sharp-orifice-nondim.c  # Sharp edge (non-dimensional)
│   └── droplet-impact-round-orifice.c     # Round edge orifice
│
└── Custom Basilisk Headers/
    ├── axi.h                          # Axisymmetric coordinates
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

## Documentation

This repository includes comprehensive documentation:

### Basilisk Setup
- **[BASILISK_INSTALL.md](BASILISK_INSTALL.md)** - Complete installation guide
  - System requirements and dependencies
  - Installation methods (darcs and tarball)
  - Platform-specific instructions (Linux, macOS, Windows/WSL)
  - Troubleshooting and verification

- **[BASILISK_CONFIG.md](BASILISK_CONFIG.md)** - Configuration and optimization
  - Environment variables setup
  - Compiler flags and options
  - MPI configuration for parallel execution
  - Performance tuning recommendations

- **[BASILISK_FEATURES.md](BASILISK_FEATURES.md)** - Basilisk features guide
  - VOF method implementation
  - Embedded boundary method
  - Adaptive mesh refinement
  - Two-phase flow solver
  - Surface tension and contact line dynamics

### Simulation Documentation
- **[DROPLET_IMPACT_ORIFICE.md](DROPLET_IMPACT_ORIFICE.md)** - Droplet impact simulations
- **[AXISYMMETRIC_GUIDE.md](AXISYMMETRIC_GUIDE.md)** - Axisymmetric coordinates
- **[NON-DIMENSIONALIZATION.md](NON-DIMENSIONALIZATION.md)** - Non-dimensional formulation
- **[DROPLET_IMPACT_SPECS.md](DROPLET_IMPACT_SPECS.md)** - Simulation specifications

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
