/**
# Axisymmetric droplet impact on a flat plate with an orifice

This simulation models the impact of a droplet on a flat plate containing
a circular orifice (hole). The simulation uses axisymmetric coordinates
with the sharp and conservative VOF method for contact line dynamics.

![Animation of the droplet impact](droplet-impact-orifice/movie.mp4)

## Parameters:
- Droplet radius: 1 mm
- Impact velocity: adjustable
- Orifice radius: adjustable
- Contact angle: adjustable
- Surface tension and viscosity: water properties

~~~gnuplot Time evolution of the normalised total volume
set xlabel 'Time'
set ylabel 'V/V_0'
plot 'log' u 1:2 w l t ''
~~~
*/

// Enable axisymmetric coordinates BEFORE including solvers
#define AXISYM 1

#include "myembed.h"
#include "navier-stokes/centered.h"
#include "embed_contact.h"
#include "embed_two-phase.h"
#include "embed_tension.h"
#include "navier-stokes/perfs.h"
#include "profiling.h"

vector tmp_h[], o_interface[], ncc[], hnew1[];

/**
## Physical parameters

Adjustable parameters for the simulation:
- thetac: contact angle (degrees) - controls wetting behavior
- impact_velocity: droplet impact speed (m/s, negative = downward)
- r_orifice: orifice radius - adjust to change hole size
*/

double thetac = 90.;          // Contact angle (degrees): 0=perfect wetting, 180=no wetting
double impact_velocity = -1.0; // Impact velocity (m/s, negative = downward)

// Simulation parameters
#define tend 0.01             // End time (s) - increase for longer simulation
double csTL = max(1e-2, VFTL);

// Domain and mesh parameters
#define MAXLEVEL  8           // Maximum refinement level (increase for finer mesh)
#define r_drop    1e-3        // Droplet radius (m) - 1 mm typical
#define l0        6*r_drop    // Domain size (radial and vertical)
#define ed        l0/pow(2,MAXLEVEL)

// Geometry parameters
#define h_plate   1.0*r_drop  // Height of plate bottom from origin
#define t_plate   0.2*r_drop  // Plate thickness
#define r_orifice 0.4*r_drop  // Orifice radius (adjust hole size here)
#define h_drop0   (h_plate + t_plate + 2.0*r_drop) // Initial droplet height

// Fluid properties (water and air at room temperature)
#define rho01     1000.       // Liquid density (kg/m^3)
#define rho02     1.2         // Gas density (kg/m^3)
#define mu01      1e-3        // Liquid dynamic viscosity (Pa·s)
#define mu02      1.8e-5      // Gas dynamic viscosity (Pa·s)
#define sigma0    0.072       // Surface tension (N/m)

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
  size(l0);
  origin(0, 0);
  rho1 = rho01;
  rho2 = rho02;
  mu1 = mu01;
  mu2 = mu02;
  f.sigma = sigma0;
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

  We define the solid geometry as a flat plate with an orifice.
  The plate is a horizontal slab with a circular hole in the center.

  In axisymmetric coordinates (r,z):
  - r (x-axis): radial direction from axis of symmetry
  - z (y-axis): vertical direction

  The level-set function is negative in solid, positive in fluid.
  */

  // Define the flat plate with orifice using vertex scalar
  // We need to set cs[] such that it's 0 in solid, 1 in fluid
  vertex scalar phi[];

  foreach_vertex() {
    // Plate boundaries in vertical direction
    double plate_bottom = y - h_plate;           // positive above plate bottom
    double plate_top = (h_plate + t_plate) - y;  // positive below plate top

    // Inside plate slab when both are positive
    double in_plate = -min(plate_bottom, plate_top); // negative inside plate

    // Orifice boundary in radial direction
    double outside_orifice = x - r_orifice;  // positive outside orifice, negative inside

    // Solid region: inside plate AND outside orifice
    // We want negative value in solid region
    // If in_plate < 0 (inside plate) and outside_orifice > 0 (outside hole): SOLID
    // Otherwise: FLUID

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
  */

  // Spherical droplet centered at (0, h_drop0)
  fraction(f, -(sq(x) + sq(y - h_drop0) - sq(r_drop)));

  foreach() {
    f[] *= cs[];  // Mask droplet with solid geometry
    contact_angle[] = thetac;

    // Initialize velocity field (impact velocity)
    u.y[] = impact_velocity;
    u.x[] = 0.;
  }
}

event logfile (i = 0; t <= tend; i += 10)
{
  /**
  ## Diagnostics and logging

  Calculate and log:
  - Total liquid volume (to verify conservation)
  - Droplet center of mass position
  - Maximum and minimum velocities
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

  // Output: time, volume_ratio, center_x, center_y, max_u, max_v
  fprintf(stderr, "%.8e %.8e %.8e %.8e %.8e %.8e\n",
          t, v/v0, xc, yc, umax, vmax);
}

/**
## Output and visualization
*/

event output (t += tend/100.)
{
  /**
  Output interface and velocity data for post-processing.
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
event movie (t += tend/300.)
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
