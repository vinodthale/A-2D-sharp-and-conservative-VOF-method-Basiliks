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

### 2. Axisymmetric Droplet Impact on Plate with Orifice (Section 5.4)

**Files**: `droplet-impact-round-orifice.c`, `droplet-impact-sharp-orifice.c`

**Complete implementation of Section 5.4** from the research paper, reproducing experimental validations of droplet impact on plates with round and sharp orifices.

#### 2a. Round Orifice Case
**File**: `droplet-impact-round-orifice.c`

Simulates a droplet falling and impacting a plate with a **semicircular-edged orifice**. The simulation uses a contact angle of 180° to prevent wetting, matching experimental conditions.

**Key Parameters**:
- Droplet diameter: **D = 9.315 mm** (Bond number Bo = 4.9)
- Orifice diameter: **d = 6 mm** (d/D = 0.644)
- Semicircular edge radius: 1 mm
- Contact angle: **θ = 180°** (no wetting)
- Resolution: ~120 cells per droplet radius

**Output times**: t* = 0, 0.54, 1.27, 1.81, 2.44, 2.96 (in gravitational time scale)

#### 2b. Sharp Orifice Case
**File**: `droplet-impact-sharp-orifice.c`

Simulates a droplet impacting a plate with a **sharp-edged orifice**. Includes contact angle hysteresis and contact line pinning at the sharp edge.

**Key Parameters**:
- Droplet diameter: **D = 10.307 mm** (Bond number Bo = 6.0)
- Orifice diameter: **d = 6 mm** (d/D = 0.58)
- Sharp 90° edges (no rounding)
- Contact angle hysteresis: **θᵣ = 42°, θₐ = 68°**
- Pinning angle at edge: **θₐ = 150°**
- Resolution: ~120 cells per droplet radius

**Output times**: t* = 0.49, 0.73, 0.90, 1.04 (in gravitational time scale)

**Documentation**:
- See [README_DROPLET_IMPACT.md](README_DROPLET_IMPACT.md) for comprehensive guide
- See [section_5_4_axisymmetric_orifice.yaml](section_5_4_axisymmetric_orifice.yaml) for complete YAML specification
- See [DROPLET_IMPACT_SPECS.md](DROPLET_IMPACT_SPECS.md) for detailed parameters

**Quick Start**:
```bash
# Compile both simulations
./compile-droplet-impact.sh

# Run round orifice case
./run-round-orifice.sh

# Run sharp orifice case
./run-sharp-orifice.sh
```

### 3. General Droplet Impact Simulations

**File**: `droplet-impact-orifice.c`

General-purpose axisymmetric droplet impact simulation with adjustable parameters.

**Features**:
- Adjustable droplet size and impact velocity
- Variable orifice size and geometry
- Customizable contact angle
- Flexible fluid properties
- Detailed diagnostics and visualization

**Documentation**: See [DROPLET_IMPACT_ORIFICE.md](DROPLET_IMPACT_ORIFICE.md) for detailed instructions.

**Key Parameters** (all adjustable):
- Droplet radius: 1 mm
- Impact velocity: 1 m/s
- Orifice radius: 0.4 mm
- Contact angle: 90°
- Fluid properties: Water/air

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

### Step 4: Verify Examples

**Automated verification** (recommended):
```bash
# Quick test all examples (0.001s simulation time)
./verify-examples.sh --quick-test

# Compile-only check (fastest)
./verify-examples.sh --compile-only

# Test specific example
./verify-examples.sh --example circle-droplet --quick-test
```

**See**: [EXAMPLES_VERIFICATION.md](EXAMPLES_VERIFICATION.md) for complete verification guide

## File Structure

```
.
├── README.md                          # This file - Project overview
│
├── Documentation/
│   ├── BASILISK_INSTALL.md            # Basilisk installation guide
│   ├── BASILISK_CONFIG.md             # Basilisk configuration and compiler flags
│   ├── BASILISK_FEATURES.md           # Basilisk features used in this project
│   ├── EXAMPLES_VERIFICATION.md       # Example verification and testing guide
│   ├── README_DROPLET_IMPACT.md       # Comprehensive droplet impact guide
│   ├── DROPLET_IMPACT_ORIFICE.md      # Droplet impact simulation details
│   ├── DROPLET_IMPACT_SPECS.md        # Simulation specifications
│   ├── section_5_4_axisymmetric_orifice.yaml  # Complete YAML spec for Section 5.4
│   ├── NON-DIMENSIONALIZATION.md      # Non-dimensional formulation
│   └── AXISYMMETRIC_GUIDE.md          # Axisymmetric coordinates guide
│
├── Build System/
│   ├── Makefile                       # Build all simulations
│   ├── setup-basilisk.sh              # Automated Basilisk installation script
│   ├── verify-examples.sh             # Automated example verification script
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

### Example Verification
- **[EXAMPLES_VERIFICATION.md](EXAMPLES_VERIFICATION.md)** - Complete verification guide
  - Inventory of all 6 example simulations
  - Automated verification script usage
  - Manual testing procedures
  - Expected results and validation criteria
  - Troubleshooting guide

### Simulation Documentation
- **[DROPLET_IMPACT_ORIFICE.md](DROPLET_IMPACT_ORIFICE.md)** - Droplet impact simulations
- **[README_DROPLET_IMPACT.md](README_DROPLET_IMPACT.md)** - Comprehensive droplet impact guide
- **[DROPLET_IMPACT_SPECS.md](DROPLET_IMPACT_SPECS.md)** - Simulation specifications
- **[section_5_4_axisymmetric_orifice.yaml](section_5_4_axisymmetric_orifice.yaml)** - Complete YAML specification for Section 5.4 orifice simulations
- **[AXISYMMETRIC_GUIDE.md](AXISYMMETRIC_GUIDE.md)** - Axisymmetric coordinates
- **[NON-DIMENSIONALIZATION.md](NON-DIMENSIONALIZATION.md)** - Non-dimensional formulation

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
