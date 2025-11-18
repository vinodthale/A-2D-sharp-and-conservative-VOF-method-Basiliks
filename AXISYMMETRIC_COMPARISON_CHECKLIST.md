# 2D Axisymmetric Simulation Comparison Checklist

This checklist helps compare 2D axisymmetric droplet impact simulations between different codebases.

## 1. Coordinate System Implementation

### Current Repository (A-2D-sharp-and-conservative-VOF-method-Basiliks)

- [x] **Coordinate System**: Uses `axi.h` or `#define AXISYM 1`
- [x] **Mapping**: x = z (axial), y = r (radial)
- [x] **Origin**: `origin(0., 0.)` ensures y ≥ 0
- [x] **Metric factors**:
  - Cell volume: `cm[] = y` (radial coordinate)
  - Face area: `fm.x[] = y`, `fm.y[] = y`
- [x] **Singularity handling**: Prevents division by zero at r=0

### ImpactForce Repository

- [ ] **Coordinate System**: ___ (to be checked)
- [ ] **Mapping**: ___
- [ ] **Origin**: ___
- [ ] **Metric factors**: ___
- [ ] **Singularity handling**: ___

---

## 2. Boundary Conditions

### Current Repository

| Boundary | Position | Type | Conditions |
|----------|----------|------|------------|
| **Left** | r = 0 (axis) | Symmetry | `u.n[left] = dirichlet(0)`, `u.t[left] = neumann(0)` |
| **Right** | r = L0 | Far-field | `u.n[right] = neumann(0)`, `p[right] = dirichlet(0)` |
| **Bottom** | z = 0 | Wall | `u.n[bottom] = dirichlet(0)`, `u.t[bottom] = dirichlet(0)` |
| **Top** | z = L0 | Outflow | `u.n[top] = neumann(0)`, `p[top] = dirichlet(0)` |
| **Embed** | Solid surfaces | No-slip | `u.t[embed] = dirichlet(0)`, `u.n[embed] = dirichlet(0)` |

### ImpactForce Repository

| Boundary | Position | Type | Conditions |
|----------|----------|------|------------|
| **Left** | ___ | ___ | ___ |
| **Right** | ___ | ___ | ___ |
| **Bottom** | ___ | ___ | ___ |
| **Top** | ___ | ___ | ___ |
| **Embed** | ___ | ___ | ___ |

---

## 3. Physical Parameters

### Current Repository - Example Case (droplet-impact-sharp-orifice.c)

**Fluids**:
- Droplet: ρ₁ = 1130 kg/m³, μ₁ = 0.007 kg/(m·s)
- Ambient: ρ₂ = 960 kg/m³, μ₂ = 0.048 kg/(m·s)
- Surface tension: σ = 0.0295 N/m
- Gravity: g = 9.8 m/s²

**Geometry**:
- Droplet diameter: D = 10.307 mm
- Orifice diameter: d = 6 mm (d/D = 0.58)
- Plate thickness: s = 2 mm
- Domain size: L0 = 160 mm

**Mesh**:
- Max level: 12 (Δ = 0.0391 mm ≈ 132 cells/diameter)
- Min level: 4

**Contact Angles**:
- Receding: θᵣ = 42°
- Advancing: θₐ = 68°
- Pinning (sharp edge): θₚ = 150°

**Dimensionless Numbers**:
- Bond number: Bo = ρgD²/σ = 6.0
- Weber number: We = ρU²D/σ (velocity-dependent)
- Reynolds number: Re = ρUD/μ (velocity-dependent)

### ImpactForce Repository

**Fluids**:
- Droplet: ρ₁ = ___, μ₁ = ___
- Ambient: ρ₂ = ___, μ₂ = ___
- Surface tension: σ = ___
- Gravity: g = ___

**Geometry**:
- Droplet diameter: D = ___
- Orifice diameter: d = ___
- Plate thickness: s = ___
- Domain size: L0 = ___

**Mesh**:
- Max level: ___
- Min level: ___

**Contact Angles**:
- Contact angle model: ___

**Dimensionless Numbers**:
- Bond number: Bo = ___
- Weber number: We = ___
- Reynolds number: Re = ___

---

## 4. Numerical Methods

### Current Repository

**VOF Method**:
- [x] Sharp VOF interface reconstruction (PLIC)
- [x] Conservative advection scheme
- [x] Height function for curvature (Eqs 13-16)
- [x] Contact line dynamics with hysteresis (Algorithms 1-6)
- [x] Embedded boundary method for complex geometries

**Time Integration**:
- [x] Adaptive time stepping
- [x] CFL condition monitoring
- [x] Time scale: T = √(ρD/(Δρ·g))

**Adaptive Mesh Refinement**:
- [x] Wavelet-based refinement
- [x] Refinement criteria: VOF field, velocity gradients
- [x] Dynamic adaptation during simulation

### ImpactForce Repository

**VOF Method**:
- [ ] Interface reconstruction: ___
- [ ] Advection scheme: ___
- [ ] Curvature calculation: ___
- [ ] Contact line model: ___
- [ ] Embedded boundaries: ___

**Time Integration**:
- [ ] Time stepping: ___
- [ ] CFL condition: ___
- [ ] Time scale: ___

**Adaptive Mesh Refinement**:
- [ ] AMR enabled: ___
- [ ] Refinement criteria: ___

---

## 5. Embedded Geometry Definition

### Current Repository

```c
// Sharp orifice geometry
vertex scalar phi[];
foreach_vertex() {
  double r = x;  // radial coordinate
  double z = y;  // axial coordinate

  if (r < R_ORIFICE) {
    phi[] = 1.0;  // fluid (inside orifice)
  } else {
    if (z >= plate_bottom && z <= plate_top) {
      phi[] = -1.0;  // solid (plate)
    } else {
      phi[] = 1.0;  // fluid (outside plate region)
    }
  }
}
fractions(phi, cs, fs);
```

**Features**:
- Level-set based geometry
- Sharp edges (no rounding)
- Contact line pinning at sharp edges

### ImpactForce Repository

```c
// Geometry definition (to be filled)
```

---

## 6. Volume Conservation

### Current Repository

**Volume Calculation**:
```c
double volume = 0.;
foreach(reduction(+:volume)) {
  volume += f[] * dv();  // dv() includes 2πr factor
}
```

**Conservation Check**:
- [x] Volume tracking enabled
- [x] Expected: |V/V₀ - 1| < 10⁻⁶
- [x] Output: Time series of V/V₀

### ImpactForce Repository

**Volume Calculation**:
```c
// (to be filled)
```

**Conservation Check**:
- [ ] Volume tracking: ___
- [ ] Expected accuracy: ___

---

## 7. Initial Conditions

### Current Repository

**Droplet**:
```c
// Spherical droplet
fraction(f, -(sq(x) + sq(y - DROPLET_POS_Y) - sq(R_DROPLET)));

// Initial velocity (falling under gravity)
foreach() {
  u.x[] = 0.0;  // radial velocity = 0
  u.y[] = 0.0;  // axial velocity (will accelerate under gravity)
}
```

**Release Height**: 110 mm above orifice
**Initial condition**: Droplet at rest, falls under gravity

### ImpactForce Repository

**Droplet**:
```c
// (to be filled)
```

**Release conditions**: ___

---

## 8. Output and Diagnostics

### Current Repository

**Standard Outputs**:
- Volume conservation (V/V₀ vs time)
- Droplet spreading diameter
- Contact line position
- Impact force on plate
- Velocity fields
- Pressure fields
- VOF field visualization

**Data Files**:
- `log` file: time, volume, center of mass, etc.
- VTK/GFS output for visualization
- Timestep snapshots

### ImpactForce Repository

**Standard Outputs**:
- ___

**Data Files**:
- ___

---

## 9. Validation Tests

### Current Repository

**Test Suite** (`tests/validation/vof_advection/test_droplet_impact_validation.c`):
1. Droplet spreading dynamics
2. Contact angle hysteresis
3. Mass conservation
4. Spurious currents

**Expected Results**:
- Maximum spreading ratio: Dₘₐₓ/D₀ ≈ 1.5-2.0 (for We=1.5, Re=100)
- Mass conservation error: < 1%
- Spurious currents: < 10⁻⁶

### ImpactForce Repository

**Test Suite**:
- ___

**Expected Results**:
- ___

---

## 10. Key Differences Summary

| Aspect | Current Repo | ImpactForce | Notes |
|--------|--------------|-------------|-------|
| Droplet type | Single-phase droplet | Compound drop (air-in-liquid) | Major difference |
| Geometry | Plate with orifice | ___ | To compare |
| Contact angles | Hysteresis + pinning | ___ | To compare |
| Axisymmetric method | axi.h | ___ | To compare |
| VOF method | Sharp conservative VOF | ___ | To compare |
| Mesh resolution | ~132 cells/D | ___ | To compare |
| Validation | Full test suite | ___ | To compare |

---

## 11. Compatibility Assessment

### Can ImpactForce tests be adapted to this repository?

**Geometric compatibility**:
- [ ] Same coordinate system orientation
- [ ] Compatible boundary conditions
- [ ] Similar geometry definition approach

**Physical compatibility**:
- [ ] Same fluid property definitions
- [ ] Compatible gravity treatment
- [ ] Same non-dimensionalization

**Numerical compatibility**:
- [ ] Compatible VOF methods
- [ ] Similar time integration
- [ ] Compatible AMR approach

---

## 12. Action Items

Based on comparison results:

- [ ] Identify test cases to port from ImpactForce
- [ ] Adapt geometry definitions if needed
- [ ] Verify physical parameter equivalence
- [ ] Run comparison simulations
- [ ] Document differences and adaptations
- [ ] Validate results against reference data

---

## Notes

_Fill in details from ImpactForce repository once accessed._

**Key questions to answer**:
1. Is ImpactForce using the same axisymmetric coordinate convention?
2. Are the volume calculations consistent (2πr factor)?
3. Can compound drop simulations be adapted to this VOF framework?
4. What validation metrics should match between implementations?

---

Last updated: 2025-11-18
