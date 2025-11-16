/**
 * Droplet Impact on Plate with Sharp Orifice
 *
 * 2D axisymmetric simulation of a droplet falling and impacting a plate with a sharp orifice.
 * This case includes contact angle hysteresis and contact line pinning at sharp edges.
 *
 * Physical properties (dimensional):
 * - Droplet: ρ_d = 1130 kg/m³, μ_d = 0.007 kg/(m·s)
 * - Ambient fluid: ρ_f = 960 kg/m³, μ_f = 0.048 kg/(m·s)
 * - Surface tension: σ = 0.0295 N/m
 * - Gravity: g = 9.8 m/s²
 *
 * Geometry:
 * - Droplet diameter: D = 10.307 mm
 * - Orifice diameter: d = 6 mm (d/D = 0.58)
 * - Plate thickness: s = 2 mm
 * - Sharp orifice (no rounding)
 * - Liquid pool top: 120 mm from plate
 * - Droplet release: 110 mm above orifice
 * - Plate: 12.5 mm from right boundary
 *
 * Mesh:
 * - Δ = 3.91×10⁻² mm (≈120 cells per radius)
 *
 * Contact angles:
 * - Receding: θ_r = 42°
 * - Advancing: θ_a = 68° (general), 150° at sharp edge (for pinning)
 * Bond number: Bo = 6.0
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
#define D_DROPLET      0.010307    // m (10.307 mm)
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

// Contact angles with hysteresis
#define THETA_RECEDING   42.0      // degrees
#define THETA_ADVANCING  68.0      // degrees
#define THETA_PINNING    150.0     // degrees (at sharp edge for contact line pinning)
#define EDGE_REGION      0.0005    // m (0.5 mm) - region near sharp edge for pinning

// Time parameters
#define T_GRAVITY      (sqrt(RHO_DROPLET * D_DROPLET / ((RHO_DROPLET - RHO_AMBIENT) * GRAVITY)))
#define T_END          (1.2 * T_GRAVITY)  // Simulate up to t = 1.2 in dimensionless time

// Passage time scale (defined based on impact velocity)
// Will be calculated after determining U_i (velocity 10 ms before impact)

// VOF tracer
scalar f[];
scalar * interfaces = {f};

// Face vector fields
face vector muv[];
face vector av[];

// Contact angle (with hysteresis and pinning)
vector contact_angle[];
scalar edge_marker[];  // Marker for sharp edge region

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
 * Embedded boundary: Plate with sharp orifice
 * The plate is located at y = PLATE_POS_Y with thickness PLATE_THICK
 * Sharp orifice has diameter D_ORIFICE with sharp (90-degree) edges
 */
event init (t = 0) {
  // Define embedded boundary for plate with sharp orifice
  vertex scalar phi[];

  foreach_vertex() {
    double r = x;  // radial coordinate in axisymmetric
    double y_coord = y;

    // Plate upper surface position
    double plate_top = PLATE_POS_Y;
    double plate_bottom = PLATE_POS_Y - PLATE_THICK;

    // Orifice radius
    double r_orifice = R_ORIFICE;

    // Solid region definition (sharp edges - no rounding)
    double phi_val;

    if (r < r_orifice) {
      // Inside orifice region - fluid
      phi_val = 1.0;
    } else {
      // Outside orifice
      if (y_coord <= plate_top && y_coord >= plate_bottom) {
        // Solid plate region
        phi_val = -(r - r_orifice);
      } else {
        // Fluid region above/below plate
        phi_val = 1.0;
      }
    }

    phi[] = phi_val;
  }

  boundary({phi});
  fractions (phi, cs, fs);

  // Initialize droplet
  fraction (f, -sq(x) + sq(y - DROPLET_POS_Y) + sq(R_DROPLET));

  // Initialize edge marker for sharp edge regions
  foreach() {
    double r = x;
    double y_coord = y;
    double plate_top = PLATE_POS_Y;
    double plate_bottom = PLATE_POS_Y - PLATE_THICK;

    // Mark cells near upper and lower sharp edges
    double dist_upper_edge = sqrt(sq(r - R_ORIFICE) + sq(y_coord - plate_top));
    double dist_lower_edge = sqrt(sq(r - R_ORIFICE) + sq(y_coord - plate_bottom));

    if (dist_upper_edge < EDGE_REGION || dist_lower_edge < EDGE_REGION) {
      edge_marker[] = 1.0;  // Near sharp edge
    } else {
      edge_marker[] = 0.0;  // Away from edge
    }
  }

  // Initialize contact angle with hysteresis
  // Default to advancing angle, will be adjusted based on contact line motion
  foreach() {
    if (edge_marker[] > 0.5) {
      // At sharp edge - use pinning angle
      contact_angle.x[] = THETA_PINNING;
      contact_angle.y[] = THETA_PINNING;
    } else {
      // Away from edge - use advancing angle initially
      contact_angle.x[] = THETA_ADVANCING;
      contact_angle.y[] = THETA_ADVANCING;
    }
  }
  boundary({contact_angle, edge_marker});
}

/**
 * Update contact angle based on hysteresis and contact line motion
 */
event contact_angle_update (i++) {
  foreach() {
    if (cs[] > 0 && cs[] < 1) {  // At solid boundary
      if (edge_marker[] > 0.5) {
        // At sharp edge - enforce pinning angle
        contact_angle.x[] = THETA_PINNING;
        contact_angle.y[] = THETA_PINNING;
      } else {
        // Away from edge - apply hysteresis
        // This is simplified; proper hysteresis requires tracking contact line velocity
        // For now, we use a range between receding and advancing angles

        // Check if interface is advancing or receding
        // (This would require velocity information; simplified here)
        double current_angle = contact_angle.x[];

        // Keep angle within hysteresis window
        if (current_angle < THETA_RECEDING) {
          contact_angle.x[] = THETA_RECEDING;
          contact_angle.y[] = THETA_RECEDING;
        } else if (current_angle > THETA_ADVANCING) {
          contact_angle.x[] = THETA_ADVANCING;
          contact_angle.y[] = THETA_ADVANCING;
        }
      }
    }
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
    fp = fopen("volume_sharp.txt", "w");
    fprintf (fp, "# Time(s)\tTime/T_g\tVolume\tV/V0\n");
  }

  fprintf (fp, "%g\t%g\t%g\t%g\n", t, t/T_GRAVITY, volume, volume/volume_initial);
  fflush(fp);
}

/**
 * Output: Interface position tracking (points M and N)
 * M: Leading interface (minimum y on axis)
 * N: Trailing interface (maximum y on axis)
 */
event interface_tracking (t += 0.0001) {
  static FILE * fp = NULL;
  if (t == 0) {
    fp = fopen("interface_position_sharp.txt", "w");
    fprintf (fp, "# Time(s)\tTime/T_g\tTime/T_i\tLeading_Y(M)\tTrailing_Y(N)\n");
  }

  // Calculate impact velocity U_i (velocity 10 ms before impact)
  // For simplicity, use free fall: v = sqrt(2*g*h) where h = DROP_RELEASE
  double U_i = sqrt(2.0 * GRAVITY * DROP_RELEASE);
  double T_passage = cube(D_DROPLET) / (U_i * sq(D_ORIFICE));

  // Find leading (minimum y on axis) and trailing (maximum y on axis) interface positions
  double y_min = L0, y_max = 0.;

  foreach() {
    if (x < 0.001 && f[] > 0.01 && f[] < 0.99) {  // Near axis, at interface
      if (y < y_min) y_min = y;
      if (y > y_max) y_max = y;
    }
  }

  fprintf (fp, "%g\t%g\t%g\t%g\t%g\n", t, t/T_GRAVITY, t/T_passage, y_min, y_max);
  fflush(fp);
}

/**
 * Output: Snapshots at specific times
 */
event snapshots (t = {0.49*T_GRAVITY, 0.73*T_GRAVITY, 0.90*T_GRAVITY, 1.04*T_GRAVITY}) {
  char name[80];
  sprintf (name, "snapshot_sharp_%g.dat", t/T_GRAVITY);

  FILE * fp = fopen (name, "w");
  output_facets (f, fp);
  fclose (fp);

  // Also output the full field
  sprintf (name, "field_sharp_%g.dat", t/T_GRAVITY);
  fp = fopen (name, "w");
  foreach() {
    if (f[] > 1e-6 && f[] < 1. - 1e-6)
      fprintf (fp, "%g %g %g\n", x, y, f[]);
  }
  fclose (fp);
}

/**
 * Visualization
 */
#if dimension == 2
#include "view.h"

event movie (t += 0.0002) {
  static FILE * fp = fopen ("movie_sharp.ppm", "w");

  view (width = 800, height = 800);
  clear();

  // Draw the interface
  draw_vof ("f", lw = 2);

  // Draw the embedded boundary
  draw_vof ("cs", "fs", lw = 2);

  // Highlight sharp edge regions
  squares ("edge_marker", min = 0, max = 1);

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
