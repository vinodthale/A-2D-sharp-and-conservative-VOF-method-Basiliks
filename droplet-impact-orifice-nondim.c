/**
# Axisymmetric droplet impact on a flat plate with an orifice (Non-dimensional)

This simulation models the impact of a droplet on a flat plate containing
a circular orifice (hole). The simulation uses axisymmetric coordinates
with the sharp and conservative VOF method for contact line dynamics.

## Non-dimensional formulation:
All variables are non-dimensionalized as follows:
- Length scale: Drop diameter D → D* = 1
- Velocity scale: Impact velocity U → U* = 1
- Time scale: T = D/U → t* = tU/D
- Pressure scale: p* = p/(ρ₁U²)
- Density: ρ* = ρ/ρ₁ = 1 (liquid), ρ₂/ρ₁ (gas)
- Viscosity: μ* = μRe/(ρ₁UD) = 1/Re (liquid)
- Surface tension: σ* = σ/(ρ₁U²D) = 1/We

## Dimensionless parameters (typical water droplet, D=2mm, U=1m/s):
- Reynolds number: Re = ρ₁UD/μ₁ = 2000
- Weber number: We = ρ₁U²D/σ = 27.8
- Ohnesorge number: Oh = μ₁/√(ρ₁σD) = √We/Re = 0.0027
- Bond number: Bo = ρ₁gD²/σ = 0.544
- Density ratio: ρ* = ρ₂/ρ₁ = 0.0012 (air/water)
- Viscosity ratio: μ* = μ₂/μ₁ = 0.018 (air/water)

## Physical interpretation:
- Re: ratio of inertial to viscous forces
- We: ratio of inertial to surface tension forces
- Oh: ratio of viscous to surface tension forces (Oh = √We/Re)
- Bo: ratio of gravitational to surface tension forces

## Adjustable parameters below:
- Contact angle
- Orifice diameter ratio d/D
- Plate thickness ratio t/D
- Dimensionless numbers (Re, We, Bo)

~~~gnuplot Time evolution of the normalised total volume
set xlabel 'Time* (non-dimensional)'
set ylabel 'V/V_0'
plot 'log' u 1:2 w l t ''
~~~
*/

#include "axi.h"
#include "navier-stokes/centered.h"
#include "myembed.h"
#include "embed_contact.h"
#include "embed_two-phase.h"
#include "embed_tension.h"
#include "navier-stokes/perfs.h"
#include "profiling.h"

vector tmp_h[], o_interface[], ncc[], hnew1[];

/**
## Non-dimensional parameters

Set the dimensionless numbers for your simulation:
*/

// Dimensionless numbers (adjust for your case)
#define RE    2000.0          // Reynolds number: Re = ρ₁UD/μ₁
#define WE    27.8            // Weber number: We = ρ₁U²D/σ
#define BO    0.544           // Bond number: Bo = ρ₁gD²/σ
#define RHO_RATIO  0.0012     // Density ratio: ρ₂/ρ₁ (air/water ≈ 0.0012)
#define MU_RATIO   0.018      // Viscosity ratio: μ₂/μ₁ (air/water ≈ 0.018)

// Contact angle (degrees)
double thetac = 90.;          // Contact angle: 0=perfect wetting, 180=no wetting

// Geometry (non-dimensional, in units of drop diameter D=1)
#define D_DROP    1.0         // Drop diameter (non-dimensional)
#define R_DROP    0.5         // Drop radius
#define D_ORIFICE 0.4         // Orifice diameter (adjust: 0.4 means d/D = 0.4)
#define R_ORIFICE (D_ORIFICE/2.0)
#define T_PLATE   0.2         // Plate thickness (in units of D)
#define H_PLATE   1.0         // Height of plate bottom from origin
#define H_DROP0   (H_PLATE + T_PLATE + 2.0*R_DROP) // Initial droplet center height

// Simulation parameters
#define TEND      10.0        // End time (non-dimensional: t* = tU/D)
double csTL = max(1e-2, VFTL);

// Domain and mesh parameters
#define MAXLEVEL  8           // Maximum refinement level
#define L0        6.0         // Domain size (in units of D)
#define ED        (L0/pow(2,MAXLEVEL))

/**
## Non-dimensional fluid properties

In non-dimensional form:
- ρ₁* = 1 (reference density)
- ρ₂* = ρ₂/ρ₁ = RHO_RATIO
- μ₁* = 1/Re
- μ₂* = μ₂/μ₁ × 1/Re = MU_RATIO/Re
- σ* = 1/We
- g* = Bo/We (from Bo = ρgD²/σ and We = ρU²D/σ)
*/

/**
## Boundary conditions

In axisymmetric coordinates:
- x-axis (horizontal): radial direction (r)
- y-axis (vertical): axial direction (z)
- left boundary: axis of symmetry (r = 0)
- right boundary: far-field radial boundary
- bottom/top: vertical boundaries
*/

// Embedded geometry (solid plate): no-slip walls
u.t[embed]  = dirichlet(0.);
u.n[embed]  = dirichlet(0.);

// Bottom boundary (vertical)
u.n[bottom] = dirichlet(0.);
u.t[bottom] = dirichlet(0.);

// Top boundary (vertical) - outflow
u.n[top]    = neumann(0.);
u.t[top]    = neumann(0.);
p[top]      = dirichlet(0.);

// Right boundary (far field radial)
u.n[right]  = neumann(0.);
u.t[right]  = neumann(0.);
p[right]    = dirichlet(0.);

// Left boundary (axis of symmetry r=0)
// Axisymmetric conditions: u_r = 0, du_z/dr = 0
u.n[left]   = dirichlet(0.);  // radial velocity = 0
u.t[left]   = neumann(0.);    // axial velocity gradient = 0
f[left]     = neumann(0.);
cs[left]    = neumann(0.);
tmp_c[left] = neumann(0.);

int main()
{
  size(L0);
  origin(0, 0);

  // Set non-dimensional fluid properties
  rho1 = 1.0;                    // Reference density (liquid)
  rho2 = RHO_RATIO;              // Gas density ratio
  mu1 = 1.0/RE;                  // Liquid viscosity (non-dimensional)
  mu2 = MU_RATIO/RE;             // Gas viscosity (non-dimensional)
  f.sigma = 1.0/WE;              // Surface tension (non-dimensional)

  // Gravity (non-dimensional)
  const face vector g[] = {0., -BO/WE};
  a = g;

  // Contact angle and VOF helper fields
  tmp_c.height = tmp_h;
  tmp_c.hnew1 = hnew1;
  tmp_c.oxyi = o_interface;
  tmp_c.nc = ncc;

  N = 1 << MAXLEVEL;
  run();
}

event init (t = 0)
{
  /**
  ## Geometry definition

  Define the solid geometry as a flat plate with a circular orifice.
  The plate is a horizontal slab with a circular hole in the center.

  In axisymmetric coordinates (r,z):
  - r (x-axis): radial direction from axis of symmetry
  - z (y-axis): vertical direction

  The level-set function is negative in solid, positive in fluid.
  */

  vertex scalar phi[];

  foreach_vertex() {
    // Plate boundaries in vertical direction
    double plate_bottom = y - H_PLATE;           // positive above plate bottom
    double plate_top = (H_PLATE + T_PLATE) - y;  // positive below plate top

    // Inside plate slab when both are positive
    double in_plate = -min(plate_bottom, plate_top); // negative inside plate

    // Orifice boundary in radial direction
    double outside_orifice = x - R_ORIFICE;  // positive outside orifice

    // Solid region: inside plate AND outside orifice
    if (in_plate < 0 && outside_orifice > 0) {
      // Inside plate body and outside orifice = solid
      phi[] = -min(-in_plate, outside_orifice); // negative value
    } else if (in_plate < 0 && outside_orifice <= 0) {
      // Inside plate but inside orifice = fluid (hole)
      phi[] = -outside_orifice; // positive value
    } else {
      // Outside plate = fluid
      phi[] = -in_plate; // positive value
    }
  }

  // Use the level-set function to define solid fraction
  fractions(phi, cs, fs);

  // Clean up small cut cells
  cleansmallcell(cs, fs, csTL);

  /**
  ## Droplet initialization

  Initialize a spherical droplet above the plate with downward velocity.
  In non-dimensional form: D=1, U=1 (downward)
  */

  // Spherical droplet centered at (0, H_DROP0)
  fraction(f, -(sq(x) + sq(y - H_DROP0) - sq(R_DROP)));

  foreach() {
    f[] *= cs[];  // Mask droplet with solid geometry
    contact_angle[] = thetac;

    // Initialize velocity field (impact velocity = -1 in non-dimensional units)
    u.y[] = -1.0;  // Downward velocity
    u.x[] = 0.;
  }
}

event logfile (i = 0; t <= TEND; i += 10)
{
  /**
  ## Diagnostics and logging

  Calculate and log (in non-dimensional units):
  - Total liquid volume (to verify conservation)
  - Droplet center of mass position
  - Maximum velocities
  */
  double v = 0, xc = 0, yc = 0;
  double umax = 0, vmax = 0;

  foreach(reduction(+:v) reduction(+:xc) reduction(+:yc)
          reduction(max:umax) reduction(max:vmax)) {
    if (dv() > 0) {
      double dvolume = dv()/cm[] * f[];
      v += dvolume;
      xc += x * dvolume;
      yc += y * dvolume;
    }
    umax = max(umax, fabs(u.x[]));
    vmax = max(vmax, fabs(u.y[]));
  }

  static double v0;
  if (i == 0) v0 = v;

  if (v > 0) {
    xc /= v;
    yc /= v;
  }

  // Output: time*, volume_ratio, center_x*, center_y*, max_u*, max_v*
  fprintf(stderr, "%.8e %.8e %.8e %.8e %.8e %.8e\n",
          t, v/v0, xc, yc, umax, vmax);
}

/**
## Output and visualization
*/

event output (t += TEND/100.)
{
  /**
  Output interface and velocity data for post-processing.
  All quantities are in non-dimensional units.
  */
  char name[80];
  sprintf(name, "out-%g", t);
  FILE * fp = fopen(name, "w");
  output_facets(tmp_c, fp);
  fclose(fp);

  // Output velocity and pressure fields
  sprintf(name, "field-%g", t);
  fp = fopen(name, "w");
  foreach() {
    fprintf(fp, "%g %g %g %g %g %g %g\n",
            x, y, u.x[], u.y[], p[], tmp_c[], cs[]);
  }
  fclose(fp);
}

#include "view.h"
event movie (t += TEND/300.)
{
  /**
  Create visualization of the droplet impact.
  - Grey: solid plate
  - Blue: liquid droplet
  - Grid: computational mesh
  */

  view(quat = {0.000, 0.000, 0.000, 1.000},
       fov = 30, near = 0.01, far = 1000,
       tx = -0.25, ty = -0.25, tz = -2.5,
       width = 800, height = 800);

  // Draw solid geometry
  draw_vof(c = "cs", s = "fs", filled = -1, fc = {0.5, 0.5, 0.5});

  // Draw liquid droplet
  draw_vof(c = "tmp_c", fc = {0.447, 0.717, 0.972}, filled = 1);

  // Draw computational mesh
  cells(lw = 0.5);

  // In axisymmetric case, mirror to show 3D appearance
  mirror(n = {1, 0}) {
    draw_vof(c = "cs", s = "fs", filled = -1, fc = {0.5, 0.5, 0.5});
    draw_vof(c = "tmp_c", fc = {0.447, 0.717, 0.972}, filled = 1);
  }

  save("movie.mp4");
}

#if TREE
event adapt (i++) {
  /**
  ## Adaptive mesh refinement

  Refine the mesh near the interface and solid boundaries.
  */
  scalar sf1[];
  foreach() {
    sf1[] = (8. * tmp_c[] +
       4. * (tmp_c[-1] + tmp_c[1] +
       tmp_c[0, 1] + tmp_c[0, -1]) +
       2. * (tmp_c[-1, 1] + tmp_c[-1, -1] +
       tmp_c[1, 1] + tmp_c[1, -1])) / 16.;
    sf1[] += cs[];
  }
  adapt_wavelet({sf1, u.x, u.y},
                (double[]){1e-3, 1e-2, 1e-2},
                minlevel = max(3, MAXLEVEL - 6),
                maxlevel = MAXLEVEL);
}
#endif
