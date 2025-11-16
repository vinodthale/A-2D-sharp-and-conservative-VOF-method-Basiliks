/**
 * Droplet Impact on Plate with Round Orifice
 *
 * 2D axisymmetric simulation of a droplet falling and impacting a plate with a round orifice.
 *
 * Physical properties (dimensional):
 * - Droplet: ρ_d = 1130 kg/m³, μ_d = 0.007 kg/(m·s)
 * - Ambient fluid: ρ_f = 960 kg/m³, μ_f = 0.048 kg/(m·s)
 * - Surface tension: σ = 0.0295 N/m
 * - Gravity: g = 9.8 m/s²
 *
 * Geometry:
 * - Droplet diameter: D = 9.315 mm
 * - Orifice diameter: d = 6 mm (d/D = 0.644)
 * - Plate thickness: s = 2 mm
 * - Round orifice modeled as semicircle with diameter s = 2 mm
 * - Liquid pool top: 120 mm from plate
 * - Droplet release: 110 mm above orifice
 * - Plate: 12.5 mm from right boundary
 *
 * Mesh:
 * - Δ = 3.91×10⁻² mm (≈120 cells per radius)
 *
 * Contact angle: θ_s = 180° (no contact with plate)
 * Bond number: Bo = 4.9
 */

#include "axi.h"
#include "navier-stokes/centered.h"
#include "myembed.h"
#include "embed_two-phase.h"
#include "embed_vof.h"
#include "embed_tension.h"
#include "embed_contact.h"
#include "adapt_wavelet_limited.h"

// Physical properties (SI units)
#define RHO_DROPLET    1130.0      // kg/m³
#define RHO_AMBIENT    960.0       // kg/m³
#define MU_DROPLET     0.007       // kg/(m·s)
#define MU_AMBIENT     0.048       // kg/(m·s)
#define SIGMA          0.0295      // N/m
#define GRAVITY        9.8         // m/s²

// Geometry (SI units - meters)
#define D_DROPLET      0.009315    // m (9.315 mm)
#define R_DROPLET      (D_DROPLET/2.0)
#define D_ORIFICE      0.006       // m (6 mm)
#define R_ORIFICE      (D_ORIFICE/2.0)
#define PLATE_THICK    0.002       // m (2 mm)
#define POOL_HEIGHT    0.120       // m (120 mm) - distance from plate to pool top
#define DROP_RELEASE   0.110       // m (110 mm) - droplet release height above orifice
#define PLATE_FROM_RB  0.0125      // m (12.5 mm) - plate distance from right boundary

// Domain and mesh parameters
#define L0             0.160       // m (160 mm) - domain size
#define MAXLEVEL       12          // Gives Δ = 0.160/4096 = 3.91e-5 m = 0.0391 mm
#define MINLEVEL       4

// Derived parameters
#define PLATE_POS_Y    (L0 - POOL_HEIGHT)  // Plate upper surface position
#define DROPLET_POS_Y  (PLATE_POS_Y + DROP_RELEASE)  // Initial droplet center position
#define PLATE_POS_X    (L0 - PLATE_FROM_RB)  // Plate radial position (right edge)

// Contact angle
#define THETA_CONTACT  180.0       // degrees

// Time parameters
#define T_GRAVITY      (sqrt(RHO_DROPLET * D_DROPLET / ((RHO_DROPLET - RHO_AMBIENT) * GRAVITY)))
#define T_END          (3.0 * T_GRAVITY)  // Simulate up to t = 3.0 in dimensionless time

// VOF tracer
scalar f[];
scalar * interfaces = {f};

// Face vector fields
face vector muv[];
face vector av[];

// Contact angle
vector contact_angle[];

int main() {
  size (L0);
  origin (0., 0.);

  // Set initial grid
  init_grid (1 << MINLEVEL);

  // Physical properties
  rho1 = RHO_DROPLET;
  rho2 = RHO_AMBIENT;
  mu1 = MU_DROPLET;
  mu2 = MU_AMBIENT;
  f.sigma = SIGMA;

  // Gravity
  const face vector g[] = {0., -GRAVITY};
  a = g;
  av = g;

  // Set viscosity
  muv = mu;

  // Contact angle
  f.contact_angle = contact_angle;

  run();
}

/**
 * Embedded boundary: Plate with round orifice
 * The plate is located at y = PLATE_POS_Y with thickness PLATE_THICK
 * Round orifice has diameter D_ORIFICE, modeled with semicircular edges
 */
event init (t = 0) {
  // Define embedded boundary for plate with round orifice
  vertex scalar phi[];

  foreach_vertex() {
    double r = x;  // radial coordinate in axisymmetric
    double y_coord = y;

    // Plate upper surface position
    double plate_top = PLATE_POS_Y;
    double plate_bottom = PLATE_POS_Y - PLATE_THICK;

    // Orifice radius
    double r_orifice = R_ORIFICE;

    // Round edge radius (semicircle with diameter = plate thickness)
    double round_radius = PLATE_THICK / 2.0;

    // Center of upper semicircle
    double upper_circle_r = r_orifice;
    double upper_circle_y = plate_top;

    // Center of lower semicircle
    double lower_circle_r = r_orifice;
    double lower_circle_y = plate_bottom;

    // Distance from upper and lower semicircle centers
    double dist_upper = sqrt(sq(r - upper_circle_r) + sq(y_coord - upper_circle_y));
    double dist_lower = sqrt(sq(r - lower_circle_r) + sq(y_coord - lower_circle_y));

    // Solid region definition
    double phi_val;

    if (r < r_orifice) {
      // Inside orifice region
      if (y_coord > plate_top && dist_upper < round_radius) {
        // Upper rounded edge
        phi_val = -(dist_upper - round_radius);
      } else if (y_coord < plate_bottom && dist_lower < round_radius) {
        // Lower rounded edge
        phi_val = -(dist_lower - round_radius);
      } else {
        // Fluid region inside orifice
        phi_val = 1.0;
      }
    } else {
      // Outside orifice - solid plate region
      if (y_coord <= plate_top && y_coord >= plate_bottom) {
        phi_val = -(r - r_orifice);  // Solid
      } else {
        phi_val = 1.0;  // Fluid
      }
    }

    phi[] = phi_val;
  }

  boundary({phi});
  fractions (phi, cs, fs);

  // Initialize droplet
  fraction (f, -sq(x) - sq(y - DROPLET_POS_Y) + sq(R_DROPLET));

  // Initialize contact angle (180 degrees = no wetting)
  foreach() {
    contact_angle.x[] = THETA_CONTACT;
    contact_angle.y[] = THETA_CONTACT;
  }
  boundary({contact_angle});
}

/**
 * Adaptive mesh refinement
 */
event adapt (i++) {
  adapt_wavelet_limited ((scalar *){f, u.x, u.y},
                         (double[]){0.01, 0.01, 0.01},
                         MAXLEVEL, MINLEVEL);
}

/**
 * Output: Volume conservation monitoring
 */
event logfile (i++) {
  double volume = 0.;
  double volume_initial = 4./3. * pi * cube(R_DROPLET);

  foreach(reduction(+:volume)) {
    volume += f[] * dv();
  }

  static FILE * fp = NULL;
  if (i == 0) {
    fp = fopen("volume_round.txt", "w");
    fprintf (fp, "# Time(s)\tTime/T_g\tVolume\tV/V0\n");
  }

  fprintf (fp, "%g\t%g\t%g\t%g\n", t, t/T_GRAVITY, volume, volume/volume_initial);
  fflush(fp);
}

/**
 * Output: Snapshots at specific times
 */
event snapshots (t = {0, 0.54*T_GRAVITY, 1.27*T_GRAVITY, 1.81*T_GRAVITY,
                      2.44*T_GRAVITY, 2.96*T_GRAVITY}) {
  char name[80];
  sprintf (name, "snapshot_round_%g.dat", t/T_GRAVITY);

  FILE * fp = fopen (name, "w");
  output_facets (f, fp);
  fclose (fp);

  // Also output the full field
  sprintf (name, "field_round_%g.dat", t/T_GRAVITY);
  fp = fopen (name, "w");
  foreach() {
    if (f[] > 1e-6 && f[] < 1. - 1e-6)
      fprintf (fp, "%g %g %g\n", x, y, f[]);
  }
  fclose (fp);
}

/**
 * Output: Regular field outputs
 */
event output (t += 0.001) {
  static FILE * fp = NULL;
  if (t == 0) {
    fp = fopen("interface_position_round.txt", "w");
    fprintf (fp, "# Time(s)\tTime/T_g\tLeading_Y\tTrailing_Y\n");
  }

  // Find leading (minimum y on axis) and trailing (maximum y on axis) interface positions
  double y_min = L0, y_max = 0.;

  foreach() {
    if (x < 0.001 && f[] > 0.01 && f[] < 0.99) {  // Near axis, at interface
      if (y < y_min) y_min = y;
      if (y > y_max) y_max = y;
    }
  }

  fprintf (fp, "%g\t%g\t%g\t%g\n", t, t/T_GRAVITY, y_min, y_max);
  fflush(fp);
}

/**
 * Visualization
 */
#if dimension == 2
#include "view.h"

event movie (t += 0.0002) {
  static FILE * fp = fopen ("movie_round.ppm", "w");

  view (width = 800, height = 800);
  clear();

  // Draw the interface
  draw_vof ("f", lw = 2);

  // Draw the embedded boundary
  draw_vof ("cs", "fs", lw = 2);

  // Draw cells
  cells();

  save (fp = fp);
}

event end_movie (t = end) {
  fclose (fp);
}
#endif

/**
 * End condition
 */
event end (t = T_END) {
  printf ("Simulation completed at t = %g s (%g T_g)\n", t, t/T_GRAVITY);
}
