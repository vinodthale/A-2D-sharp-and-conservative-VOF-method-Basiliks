# Droplet Impact Simulation

2D axisymmetric simulation of droplet falling and impacting a plate with an orifice using Basilisk.

## Overview

This simulation reproduces the experimental setup from the research paper where a droplet falls through an ambient fluid and impacts a plate with an orifice. Two scenarios are considered:

1. **Round Orifice Case** - The orifice has rounded edges (semicircular), and the droplet does not make contact with the plate (contact angle = 180°)

2. **Sharp Orifice Case** - The orifice has sharp 90° edges, contact line development occurs with hysteresis (advancing angle = 68°, receding angle = 42°), and contact line pinning at sharp edges

## Quick Start

### 1. Compile the simulations

```bash
./compile-droplet-impact.sh
```

This will compile both the round and sharp orifice simulations using the Basilisk `qcc` compiler.

### 2. Run simulations

**Round orifice:**
```bash
./run-round-orifice.sh
```

**Sharp orifice:**
```bash
./run-sharp-orifice.sh
```

**Or run directly:**
```bash
./droplet-impact-round-orifice
./droplet-impact-sharp-orifice
```

## Files

### Source Code
- `droplet-impact-round-orifice.c` - Round orifice simulation
- `droplet-impact-sharp-orifice.c` - Sharp orifice simulation
- `compile-droplet-impact.sh` - Compilation script
- `run-round-orifice.sh` - Run script for round orifice
- `run-sharp-orifice.sh` - Run script for sharp orifice

### Documentation
- `DROPLET_IMPACT_SPECS.md` - Detailed parameter specifications and verification
- `README_DROPLET_IMPACT.md` - This file

### Header Files (Used by simulations)
- `myembed.h` - Embedded boundary module
- `embed_two-phase.h` - Two-phase flow properties
- `embed_vof.h` - VOF methods
- `embed_tension.h` - Surface tension
- `embed_contact.h` - Contact angle handling
- `adapt_wavelet_limited.h` - Adaptive mesh refinement

## Physical Parameters

All parameters match the experimental specifications exactly:

| Property | Value | Units |
|----------|-------|-------|
| Droplet density | 1130 | kg/m³ |
| Ambient fluid density | 960 | kg/m³ |
| Droplet viscosity | 0.007 | kg/(m·s) |
| Ambient fluid viscosity | 0.048 | kg/(m·s) |
| Surface tension | 0.0295 | N/m |
| Gravity | 9.8 | m/s² |

## Geometry Parameters

### Round Orifice Case
- Droplet diameter: **D = 9.315 mm**
- Orifice diameter: **d = 6 mm** (d/D = 0.644)
- Bond number: **Bo = 4.9**
- Contact angle: **θ = 180°**
- Edge type: Semicircular (radius = 1 mm)

### Sharp Orifice Case
- Droplet diameter: **D = 10.307 mm**
- Orifice diameter: **d = 6 mm** (d/D = 0.58)
- Bond number: **Bo = 6.0**
- Receding angle: **θ_r = 42°**
- Advancing angle: **θ_a = 68°**
- Advancing angle at edge: **θ_a = 150°** (for pinning)
- Edge type: Sharp 90° corners

### Common Parameters
- Plate thickness: **s = 2 mm**
- Liquid pool top: **120 mm** from plate
- Droplet release: **110 mm** above orifice
- Plate distance from right boundary: **12.5 mm**

## Computational Domain

- **Domain size:** 160 × 160 mm² (0.16 × 0.16 m²)
- **Coordinate system:** 2D axisymmetric
- **Maximum refinement level:** 12
- **Mesh size:** Δ = 3.91×10⁻⁵ m = 0.0391 mm
- **Grid resolution:** ~120 cells per droplet radius
- **Adaptive mesh refinement:** Wavelet-based

## Output Files

### Round Orifice Case (in `output_round/`)

| File | Description |
|------|-------------|
| `volume_round.txt` | Volume conservation: Time, Time/T_g, Volume, V/V₀ |
| `interface_position_round.txt` | Leading and trailing interface positions vs time |
| `snapshot_round_*.dat` | Interface facets at t/T_g = 0, 0.54, 1.27, 1.81, 2.44, 2.96 |
| `field_round_*.dat` | Full VOF field data at snapshot times |
| `movie_round.ppm` | Animation of the simulation |

### Sharp Orifice Case (in `output_sharp/`)

| File | Description |
|------|-------------|
| `volume_sharp.txt` | Volume conservation: Time, Time/T_g, Volume, V/V₀ |
| `interface_position_sharp.txt` | Points M and N positions: Time, Time/T_g, Time/T_i, Leading_Y, Trailing_Y |
| `snapshot_sharp_*.dat` | Interface facets at t/T_g = 0.49, 0.73, 0.90, 1.04 |
| `field_sharp_*.dat` | Full VOF field data at snapshot times |
| `movie_sharp.ppm` | Animation of the simulation |

## Time Scales

### Round Orifice
**Gravitational time scale:**
```
t_g = √(ρ_l × D / ((ρ_l - ρ_g) × g))
    = 0.0795 s = 79.5 ms
```

**Simulation runs up to:** t = 3.0 × t_g ≈ 239 ms

**Snapshot times:** t/t_g = 0, 0.54, 1.27, 1.81, 2.44, 2.96

### Sharp Orifice
**Gravitational time scale:**
```
t_g = √(ρ_l × D / ((ρ_l - ρ_g) × g))
    = 0.0836 s = 83.6 ms
```

**Passage time scale:**
```
U_i = √(2 × g × h) = 1.47 m/s  (impact velocity)
t_i = D³ / (U_i × d²)
    = 0.0207 s = 20.7 ms
```

**Simulation runs up to:** t = 1.2 × t_g ≈ 100 ms

**Snapshot times:** t/t_g = 0.49, 0.73, 0.90, 1.04

## Numerical Methods

### VOF (Volume of Fluid)
- Sharp interface tracking
- Conservative advection
- Height function for curvature calculation

### Embedded Boundaries
- Plate geometry represented by level set function
- Volume and area fractions for cut cells
- Boundary conditions enforced at solid surface

### Surface Tension
- Continuum surface force (CSF) method
- Curvature from height functions
- Contact angle boundary conditions

### Contact Angle Modeling

**Round Orifice:**
- Static contact angle: θ = 180° (perfect non-wetting)
- No contact with plate surface

**Sharp Orifice:**
- Contact angle hysteresis: θ_r = 42°, θ_a = 68°
- Enhanced advancing angle at sharp edge: θ_a = 150° (for pinning effect)
- Dynamic contact line motion

### Adaptive Mesh Refinement
- Wavelet-based error estimation
- Refinement on interface and velocity gradients
- 4 ≤ level ≤ 12

## Expected Results

### Round Orifice
- Droplet deforms as it approaches the plate
- No contact with plate surface (θ = 180°)
- Droplet passes through orifice without wetting
- Interface follows arc-shaped outline of round orifice
- Good agreement with experimental droplet shapes

### Sharp Orifice
- Contact line appears on upper surface at t ≈ t₂
- Contact line pinning at upper sharp edge
- Trailing surface forms crescent shape at t ≈ t₃
- Trailing surface rebounds above upper surface
- Contact line pinned at lower sharp edge at t ≈ t₄
- Nearly disconnected droplet below and in orifice

## Visualization

Both simulations generate PPM movies showing:
- Droplet interface (VOF field)
- Plate geometry (embedded boundary)
- Adaptive mesh grid
- Evolution over time

To convert PPM to video format:
```bash
ffmpeg -i output_round/movie_round.ppm output_round/movie_round.mp4
ffmpeg -i output_sharp/movie_sharp.ppm output_sharp/movie_sharp.mp4
```

## Post-Processing

### Plot Volume Conservation
```python
import numpy as np
import matplotlib.pyplot as plt

# Round orifice
data = np.loadtxt('output_round/volume_round.txt')
plt.plot(data[:,1], data[:,3])
plt.xlabel('t / t_g')
plt.ylabel('V / V_0')
plt.title('Volume Conservation - Round Orifice')
plt.grid()
plt.show()
```

### Plot Interface Position
```python
import numpy as np
import matplotlib.pyplot as plt

# Sharp orifice
data = np.loadtxt('output_sharp/interface_position_sharp.txt')
plt.plot(data[:,2], data[:,3], label='Leading (M)')
plt.plot(data[:,2], data[:,4], label='Trailing (N)')
plt.xlabel('t / t_i')
plt.ylabel('Position (m)')
plt.title('Interface Position - Sharp Orifice')
plt.legend()
plt.grid()
plt.show()
```

## Validation

The simulations are designed to reproduce experimental results from:
> Research paper reference [44] - Droplet impact on plate with orifice

**Validation criteria:**
1. ✅ Bond numbers: Bo = 4.9 (round), Bo = 6.0 (sharp)
2. ✅ Diameter ratios: d/D = 0.644 (round), d/D = 0.58 (sharp)
3. ✅ Mesh resolution: ~120 cells per radius
4. ✅ Physical properties match experimental conditions
5. ✅ Contact angles configured correctly
6. ✅ Time scales calculated accurately

See `DROPLET_IMPACT_SPECS.md` for detailed verification.

## Troubleshooting

### Compilation Errors

**Error:** `qcc: command not found`
- **Solution:** Ensure Basilisk is installed and `$BASILISK` environment variable is set
  ```bash
  export BASILISK=/path/to/basilisk
  export PATH=$PATH:$BASILISK/bin
  ```

**Error:** Missing header files
- **Solution:** Verify all header files are in the same directory as the source files

### Runtime Errors

**Error:** Simulation crashes or diverges
- **Solution:** Check CFL condition, reduce time step if needed
- Try increasing `MINLEVEL` for more stable initial grid

**Error:** Large volume loss
- **Solution:** Increase `MAXLEVEL` for better resolution
- Check that interface is well-resolved

### Performance

**Slow simulation:**
- Reduce `MAXLEVEL` (but keep resolution > 80 cpr)
- Increase `MINLEVEL` to reduce number of cells
- Run on multiple cores if available

## Requirements

- **Basilisk C** - http://basilisk.fr/
- **GCC** - C compiler
- **OpenGL libraries** (optional, for visualization)

## Contact & References

For questions about this simulation:
1. Check `DROPLET_IMPACT_SPECS.md` for parameter details
2. Refer to Basilisk documentation: http://basilisk.fr/
3. See research paper for experimental comparison

## License

These simulations use the Basilisk framework which is licensed under GPL.

---

**Last updated:** 2025-11-16
**Basilisk version:** Latest
**Simulation framework:** Sharp and Conservative VOF Method
