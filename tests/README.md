# Test Suite for 2D Sharp and Conservative VOF Method

This comprehensive test suite validates the implementation of the **2D sharp and conservative VOF method for modeling contact-line dynamics with hysteresis on complex boundaries**.

## Overview

The test suite covers all major components of the VOF method described in the reference paper:

- **Governing Equations** (Equations 1-5)
- **VOF Advection** with directional splitting (Equations 8-12)
- **Interface Reconstruction** using PLIC (Equations 17-18)
- **Height Function Curvature** (Equations 13-16)
- **Contact Line Dynamics** (Algorithms 1-6)
- **Time Integration** and pressure projection (Equations 31-37)

## Directory Structure

```
tests/
├── README.md                          # This file
├── vof_method_specification.yaml      # Complete specification of all equations
│
├── unit/                              # Unit tests for individual components
│   ├── momentum/
│   │   └── test_momentum_equation.c
│   ├── vof_advection/
│   │   └── test_vof_advection.c
│   ├── interface_reconstruction/
│   │   └── test_plic_reconstruction.c
│   ├── curvature/
│   │   └── test_height_function_curvature.c
│   └── contact_line/
│       └── test_contact_line_algorithms.c
│
├── integration/                       # Integration tests
│   └── vof_advection/
│       └── test_full_vof_pipeline.c
│
├── validation/                        # Validation against reference data
│   └── vof_advection/
│       └── test_droplet_impact_validation.c
│
├── benchmarks/                        # Performance benchmarks
│   └── vof_advection/
│       └── benchmark_reynolds_sweep.c
│
├── data/                              # Reference data and results
└── utils/                             # Testing utilities
```

## Test Categories

### 1. Unit Tests

Unit tests verify individual components in isolation:

#### Momentum Equation (`unit/momentum/`)
- **test_momentum_equation.c**: Tests Equation 1
  - Steady uniform flow
  - Pressure gradient balance
  - Viscous diffusion
  - Surface tension force (CSF)

#### VOF Advection (`unit/vof_advection/`)
- **test_vof_advection.c**: Tests Equations 3, 8, 10, 11
  - Circle translation (mass conservation)
  - Rotation test (Zalesak's disk)
  - Shear flow (reversibility)
  - Volume conservation
  - Boundedness (0 ≤ c ≤ 1)

#### Interface Reconstruction (`unit/interface_reconstruction/`)
- **test_plic_reconstruction.c**: Tests Equations 17, 18
  - Horizontal interface
  - Vertical interface
  - Diagonal interface (45°)
  - Circle reconstruction accuracy
  - Volume accuracy

#### Curvature Calculation (`unit/curvature/`)
- **test_height_function_curvature.c**: Tests Equations 13-16
  - Circle curvature (κ = 1/R)
  - Ellipse curvature
  - Flat interface (κ = 0)
  - Contact angle enforcement (Eq. 15)
  - Convergence with mesh refinement

#### Contact Line Algorithms (`unit/contact_line/`)
- **test_contact_line_algorithms.c**: Tests Algorithms 1-6
  - Contact angle identification
  - Contact angle enforcement (Algorithm 3)
  - Contact line cell identification (Algorithm 4)
  - Hysteresis window
  - Mixed cell reconstruction (Algorithm 1)
  - Small cell clearance (Algorithm 2)

### 2. Integration Tests

Integration tests verify the complete pipeline:

#### Full VOF Pipeline (`integration/vof_advection/`)
- **test_full_vof_pipeline.c**
  - Dam break with obstacle
  - Droplet oscillation (Rayleigh frequency)
  - Rising bubble
  - Droplet coalescence
  - Capillary wave

### 3. Validation Tests

Validation tests compare against reference solutions:

#### Droplet Impact (`validation/vof_advection/`)
- **test_droplet_impact_validation.c**
  - Droplet spreading dynamics
  - Contact angle hysteresis
  - Mass conservation
  - Spurious currents

### 4. Benchmark Tests

Performance benchmarks for parameter sweeps:

#### Reynolds Number Sweep (`benchmarks/vof_advection/`)
- **benchmark_reynolds_sweep.c**
  - Re sweep: 22 to 200 (25 values)
  - Metrics: D_max/D0, contact time, rebound height
  - CPU time and mass conservation

## Method Specification

The complete method specification is provided in `vof_method_specification.yaml`, which includes:

### Nondimensionalization
- Length scale: D = 1.0 (droplet diameter)
- Velocity scale: U0 = 1.0
- Time scale: t_ref = D/U0
- Temperature scale: T* = (T - T_sat)/(T_inf - T_sat)

### Physical Properties (Water-Steam)
```yaml
rho_l: 958.4    rho_g: 0.597
mu_l: 2.8e-4    mu_g: 1.26e-5
sigma: 0.0728   h_lg: 2.26e6
```

### Nondimensional Parameters
- **Re**: 22 to 200 (Reynolds number)
- **We**: 1.5 (Weber number)
- **Density ratio η**: 0.000623
- **Viscosity ratio**: 0.045

### Numerical Parameters
- **CFL**: 0.20
- **Domain**: L0 = 8.0 (axisymmetric)
- **Max level**: Lmax = 12
- **Min spacing**: dx_min = 0.001953125

## Running the Tests

### Prerequisites

1. **Basilisk C** installed and configured
2. Required headers in the include path:
   - `grid/cartesian.h`
   - `axi.h` (for axisymmetric)
   - `navier-stokes/centered.h`
   - `two-phase.h`
   - `vof.h`
   - `tension.h`
   - `embed.h`

### Compilation

For unit tests:
```bash
# Example: compile VOF advection test
qcc -O2 -Wall unit/vof_advection/test_vof_advection.c -o test_vof_advection -lm

# Run
./test_vof_advection
```

For axisymmetric tests:
```bash
# Include axisymmetric coordinate system
qcc -O2 -Wall -grid=multigrid validation/vof_advection/test_droplet_impact_validation.c \
    -o test_droplet_impact -lm
```

### Automated Testing

Use the provided Makefile (to be created):
```bash
# Run all unit tests
make unit-tests

# Run integration tests
make integration-tests

# Run validation tests
make validation-tests

# Run benchmarks
make benchmarks

# Run everything
make test-all
```

## Test Validation Criteria

### Unit Tests
- ✓ Pass if all assertions pass
- ✓ Numerical errors < specified tolerances
- ✓ Physical constraints satisfied

### Integration Tests
- ✓ Mass conservation error < 1%
- ✓ Monotonic progression where expected
- ✓ Physical behavior matches theory

### Validation Tests
- ✓ Results within expected ranges from literature
- ✓ Spreading diameter: 1.5 < D_max/D0 < 2.0
- ✓ Contact angle: θ_rec ≤ θ ≤ θ_adv
- ✓ Spurious currents: u_max < 10⁻⁶

### Benchmarks
- ✓ CPU time scaling
- ✓ Mass error < 2% for all Re
- ✓ Physical bounds on all metrics

## Equations Tested

### Section 2: Governing Equations
- **Eq. 1**: Momentum equation with surface tension
- **Eq. 2**: Continuity (incompressibility)
- **Eq. 3**: VOF transport
- **Eq. 4**: Density mixture rule
- **Eq. 5**: Viscosity mixture rule

### Section 2.3: VOF Advection
- **Eq. 8**: Reformulated VOF with compression
- **Eq. 9**: Integral form
- **Eq. 10**: Directional split (x-direction)
- **Eq. 11**: Directional split (y-direction)
- **Eq. 12**: Geometric flux

### Section 3: Interface Reconstruction
- **Eq. 17**: PLIC plane equation
- **Eq. 18**: Polygon area (shoelace formula)
- **Eq. 20**: Alpha computation
- **Eq. 21**: Adjusted flux for mixed cells
- **Eq. 22**: Dilation term c_c

### Section 3: Height Function Curvature
- **Eq. 13**: Height sum
- **Eq. 14**: Curvature from height function
- **Eq. 15**: Ghost height for contact angle
- **Eq. 16**: Normal from height function

### Section 3: Time Integration
- **Eq. 24**: CFL with small cells
- **Eq. 31**: Staggered VOF advection
- **Eq. 32**: Momentum predictor
- **Eq. 35**: Momentum diffusion
- **Eq. 36**: Pressure Poisson equation
- **Eq. 37**: Velocity projection

### Algorithms
- **Algorithm 1**: Mixed-cell interface reconstruction
- **Algorithm 2**: Clear small mixed cells
- **Algorithm 3**: Compute n_{l,θ}
- **Algorithm 4**: Identify contact-line cell
- **Algorithm 5**: HF contact-angle enforcement
- **Algorithm 6**: Bisection hysteresis

## Expected Results

### Unit Tests
All unit tests should pass with:
- Normal reconstruction errors < 5%
- Curvature errors < 5% for circles
- Mass conservation errors < 1%
- Boundedness: 0 ≤ c ≤ 1

### Validation Tests
Droplet impact should show:
- **D_max/D0**: 1.5 - 2.0 (for We=1.5, Re=100)
- **Contact angle**: Within hysteresis window [60°, 120°]
- **Mass error**: < 1%

### Benchmarks
Reynolds sweep should demonstrate:
- Increasing spreading with Re
- Consistent mass conservation
- Reasonable CPU times

## Reference

**Title**: "A 2D sharp and conservative VOF method for modeling the contact line dynamics with hysteresis on complex boundaries"

**Key Features**:
- Sharp interface reconstruction (PLIC)
- Conservative VOF advection with directional splitting
- Height function curvature with contact angle enforcement
- Hysteresis model for advancing/receding angles
- Small cell treatment for embedded boundaries
- 2D axisymmetric capability

## Contributing

When adding new tests:

1. **Follow naming convention**: `test_<component>_<feature>.c`
2. **Include documentation**: Specify which equations are tested
3. **Add assertions**: Clear pass/fail criteria
4. **Update README**: Document new tests
5. **Add to Makefile**: Include in automated testing

## Troubleshooting

### Common Issues

1. **Compilation errors**
   - Check Basilisk installation
   - Verify include paths
   - Ensure all required headers are available

2. **Test failures**
   - Check grid resolution (may need refinement)
   - Verify numerical parameters (CFL, tolerances)
   - Compare against reference data

3. **Performance issues**
   - Reduce Lmax for faster testing
   - Use coarser grids for unit tests
   - Enable optimization flags (-O2 or -O3)

## Contact

For issues, questions, or contributions, please refer to the main repository documentation.

---

**Test Suite Version**: 1.0
**Last Updated**: 2025-11-18
**Basilisk Version**: Latest stable
