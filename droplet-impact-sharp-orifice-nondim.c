/**
 * Droplet Impact on Plate with Sharp Orifice (Non-dimensional)
 *
 * 2D axisymmetric simulation of a droplet falling and impacting a plate with a sharp orifice.
 * This case includes contact angle hysteresis and contact line pinning at sharp edges.
 *
 * ## Non-dimensional formulation:
 * All variables are non-dimensionalized as follows:
 * - Length scale: Drop diameter D → D* = 1
 * - Velocity scale: Impact velocity U → U* = 1
 * - Time scale: T = D/U → t* = tU/D
 * - Pressure scale: p* = p/(ρ₁U²)
 *
 * ## Dimensionless parameters (based on reference case):
 * Reference dimensional values:
 * - Droplet: ρ_d = 1130 kg/m³, μ_d = 0.007 kg/(m·s)
 * - Ambient fluid: ρ_f = 960 kg/m³, μ_f = 0.048 kg/(m·s)
 * - Surface tension: σ = 0.0295 N/m
 * - Gravity: g = 9.8 m/s²
 * - Droplet diameter: D = 10.307 mm
 * - Impact velocity: U ≈ 1.47 m/s (from free fall h = 110 mm)
 * - Orifice diameter: d = 6 mm (d/D = 0.58)
 *
 * Calculated dimensionless numbers:
 * - Re = ρ₁UD/μ₁ ≈ 2448
 * - We = ρ₁U²D/σ ≈ 873
 * - Bo = ρ₁gD²/σ ≈ 38.7
 * - Oh = μ₁/√(ρ₁σD) ≈ 0.0143
 * - ρ* = ρ₂/ρ₁ = 0.850
 * - μ* = μ₂/μ₁ = 6.86
 *
 * Geometry (non-dimensional):
 * - Drop diameter: D = 1
 * - Orifice diameter: d/D = 0.58
 * - Plate thickness: s/D ≈ 0.19
 * - Sharp orifice (no rounding)
 *
 * Contact angles:
 * - Receding: θ_r = 42°
 * - Advancing: θ_a = 68° (general), 150° at sharp edge (for pinning)
 */

#include "axi.h"
#include "navier-stokes/centered.h"
#include "myembed.h"
#include "embed_two-phase.h"
#include "embed_vof.h"
#include "embed_tension.h"
#include "embed_contact.h"
#include "adapt_wavelet_limited.h"

/**
## Non-dimensional parameters

Adjust these for your specific case:
*/

// Dimensionless numbers
#define RE             2448.0      // Reynolds number: Re = ρ₁UD/μ₁
#define WE             873.0       // Weber number: We = ρ₁U²D/σ
#define BO             38.7        // Bond number: Bo = ρ₁gD²/σ
#define RHO_RATIO      0.850       // Density ratio: ρ₂/ρ₁
#define MU_RATIO       6.86        // Viscosity ratio: μ₂/μ₁

// Geometry (non-dimensional, in units of D=1)
#define D_DROPLET      1.0         // Drop diameter (non-dimensional)
#define R_DROPLET      0.5         // Drop radius
#define D_ORIFICE      0.582       // Orifice diameter (d/D = 6/10.307)
#define R_ORIFICE      (D_ORIFICE/2.0)
#define PLATE_THICK    0.194       // Plate thickness (s/D = 2/10.307)
#define POOL_HEIGHT    11.64       // Pool height (120 mm / 10.307 mm)
#define DROP_RELEASE   10.67       // Drop release height above orifice (110 mm / 10.307 mm)
#define PLATE_FROM_RB  1.213       // Plate from right boundary (12.5 mm / 10.307 mm)

// Domain and mesh parameters (non-dimensional)
#define L0             15.52       // Domain size (160 mm / 10.307 mm)
#define MAXLEVEL       12          // Maximum refinement level
#define MINLEVEL       4

// Derived parameters
#define PLATE_POS_Y    (L0 - POOL_HEIGHT)           // Plate upper surface position
#define DROPLET_POS_Y  (PLATE_POS_Y + DROP_RELEASE) // Initial droplet center position
#define PLATE_POS_X    (L0 - PLATE_FROM_RB)         // Plate radial position

// Contact angles with hysteresis
#define THETA_RECEDING   42.0      // degrees
#define THETA_ADVANCING  68.0      // degrees
#define THETA_PINNING    150.0     // degrees (at sharp edge for contact line pinning)
#define EDGE_REGION      0.0485    // Region near sharp edge (0.5 mm / 10.307 mm)

// Time parameters (non-dimensional)
// T_gravity* = T_g × U/D where T_g = sqrt(ρ₁D/((ρ₁-ρ₂)g))
// For non-dim: g* = Bo/We, so T_g* = sqrt(We/(Bo(1-ρ*)))
#define T_GRAVITY_ND   (sqrt(WE/(BO*(1.0-RHO_RATIO))))
#define T_END          (1.2 * T_GRAVITY_ND)  // Simulate up to t* = 1.2 T_g*

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

  // Set non-dimensional fluid properties
  rho1 = 1.0;                    // Reference density (droplet)
  rho2 = RHO_RATIO;              // Ambient fluid density ratio
  mu1 = 1.0/RE;                  // Droplet viscosity (non-dimensional)
  mu2 = MU_RATIO/RE;             // Ambient viscosity (non-dimensional)
  f.sigma = 1.0/WE;              // Surface tension (non-dimensional)

  // Gravity (non-dimensional)
  const face vector g[] = {0., -BO/WE};
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

    // Plate upper and lower surfaces
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

  // Initialize droplet (spherical, falling under gravity)
  // In non-dim: D=1, starting at rest (gravity will accelerate it)
  // Or we can initialize with impact velocity U*=1
  fraction (f, -sq(x) - sq(y - DROPLET_POS_Y) + sq(R_DROPLET));

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
    fp = fopen("volume_sharp_nondim.txt", "w");
    fprintf (fp, "# Time* (non-dim)\tTime*/T_g*\tVolume\tV/V0\n");
  }

  fprintf (fp, "%g\t%g\t%g\t%g\n", t, t/T_GRAVITY_ND, volume, volume/volume_initial);
  fflush(fp);
}

/**
 * Output: Interface position tracking (points M and N)
 * M: Leading interface (minimum y on axis)
 * N: Trailing interface (maximum y on axis)
 */
event interface_tracking (t += 0.01) {
  static FILE * fp = NULL;
  if (t == 0) {
    fp = fopen("interface_position_sharp_nondim.txt", "w");
    fprintf (fp, "# Time* (non-dim)\tTime*/T_g*\tLeading_Y*(M)\tTrailing_Y*(N)\n");
  }

  // Find leading (minimum y on axis) and trailing (maximum y on axis) interface positions
  double y_min = L0, y_max = 0.;

  foreach() {
    if (x < 0.01 && f[] > 0.01 && f[] < 0.99) {  // Near axis, at interface
      if (y < y_min) y_min = y;
      if (y > y_max) y_max = y;
    }
  }

  fprintf (fp, "%g\t%g\t%g\t%g\n", t, t/T_GRAVITY_ND, y_min, y_max);
  fflush(fp);
}

/**
 * Output: Snapshots at specific times
 */
event snapshots (t = {0.49*T_GRAVITY_ND, 0.73*T_GRAVITY_ND,
                      0.90*T_GRAVITY_ND, 1.04*T_GRAVITY_ND}) {
  char name[80];
  sprintf (name, "snapshot_sharp_nondim_%g.dat", t/T_GRAVITY_ND);

  FILE * fp = fopen (name, "w");
  output_facets (f, fp);
  fclose (fp);

  // Also output the full field
  sprintf (name, "field_sharp_nondim_%g.dat", t/T_GRAVITY_ND);
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

event movie (t += T_END/500.) {
  static FILE * fp = fopen ("movie_sharp_nondim.ppm", "w");

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
  printf ("Simulation completed at t* = %g (%g T_g*)\n", t, t/T_GRAVITY_ND);
}
