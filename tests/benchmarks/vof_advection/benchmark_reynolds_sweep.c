/**
 * Benchmark Test: Reynolds Number Sweep
 * Tests the VOF method across a range of Reynolds numbers
 * as specified in the paper
 *
 * Reference: Paper parameters
 * - Re sweep: 25 values from 22 to 200 (linear spacing)
 * - Weber number: We = 1.5
 * - Density ratio: η = ρ_g/ρ_l = 0.000623
 * - Viscosity ratio: μ_g/μ_l = 0.045
 *
 * Benchmark metrics:
 * 1. Maximum spreading diameter D_max/D0
 * 2. Contact time
 * 3. Rebound height (if applicable)
 * 4. CPU time and iterations
 * 5. Mass conservation error
 */

#include "grid/cartesian.h"
#include "axi.h"
#include "navier-stokes/centered.h"
#include "two-phase.h"
#include "tension.h"

// Nondimensional parameters
double eta = 0.000623;      // Density ratio ρ_g/ρ_l
double mu_ratio = 0.045;    // Viscosity ratio
double We = 1.5;            // Weber number

// Domain
double L0_domain = 8.0;
int Lmax_grid = 10;  // Reduced for benchmark speed

// Initial droplet
double R0 = 0.5;
double U0 = 1.0;

scalar f[];
scalar * interfaces = {f};

// Results structure
typedef struct {
  double Re;
  double D_max_ratio;
  double contact_time;
  double rebound_height;
  double cpu_time;
  double mass_error;
  int iterations;
} BenchmarkResult;

BenchmarkResult run_single_case(double Re) {
  printf("\n  Running Re = %g\n", Re);

  BenchmarkResult result;
  result.Re = Re;

  // Setup grid
  L0 = L0_domain;
  origin(0.0, 0.0);
  N = 1 << Lmax_grid;
  init_grid(N);

  // Set properties based on Re
  rho1 = 1.0;
  rho2 = eta;
  mu1 = eta / (Re * mu_ratio);
  mu2 = eta / Re;
  f.sigma = 1.0 / We;

  // Initial droplet position
  double x0 = L0_domain / 2.0;
  double y0 = 2.0 * R0;

  fraction(f, sq(R0) - sq(x - x0) - sq(y - y0));

  // Initial velocity
  foreach() {
    u.y[] = -U0 * (f[] > 0.5 ? 1.0 : 0.0);
  }

  // Solid surface
  vertex scalar phi[];
  foreach_vertex()
    phi[] = y;

  fractions(phi, cs, fs);

  // Initial mass
  double mass_initial = 0.0;
  foreach(reduction(+:mass_initial))
    mass_initial += f[] * dv();

  // Time integration
  double t_end = 0.16;  // Paper: t* = 0.16
  double dt_max = 0.001;
  DT = dt_max;

  // Monitoring
  double D_max = 0.0;
  double t_contact = 0.0;
  double h_rebound = 0.0;
  bool contact_detected = false;
  int iter_count = 0;

  clock_t start_time = clock();

  // Run simulation
  for (double t = 0; t < t_end; t += dt) {
    run();
    iter_count++;

    // Measure spreading diameter
    double x_min = L0_domain, x_max = 0.0;
    double y_contact = L0_domain;

    foreach() {
      if (f[] > 0.5) {
        x_min = min(x_min, x);
        x_max = max(x_max, x);
        y_contact = min(y_contact, y);
      }
    }

    double D_spread = x_max - x_min;

    if (D_spread > D_max)
      D_max = D_spread;

    // Detect contact
    if (!contact_detected && y_contact < 0.05 * R0) {
      contact_detected = true;
      t_contact = t;
    }

    // Measure rebound (if droplet lifts off)
    if (contact_detected && y_contact > 0.1 * R0) {
      h_rebound = max(h_rebound, y_contact);
    }

    if (iter_count % 50 == 0)
      printf("    t = %g: D = %g\n", t, D_spread);
  }

  clock_t end_time = clock();

  // Final mass
  double mass_final = 0.0;
  foreach(reduction(+:mass_final))
    mass_final += f[] * dv();

  // Store results
  result.D_max_ratio = D_max / (2.0 * R0);
  result.contact_time = t_contact;
  result.rebound_height = h_rebound / R0;
  result.cpu_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  result.mass_error = fabs(mass_final - mass_initial) / mass_initial;
  result.iterations = iter_count;

  printf("    D_max/D0 = %g\n", result.D_max_ratio);
  printf("    Contact time = %g\n", result.contact_time);
  printf("    CPU time = %g s\n", result.cpu_time);
  printf("    Mass error = %g%%\n", 100.0 * result.mass_error);

  return result;
}

int main() {
  printf("=================================================\n");
  printf("Benchmark: Reynolds Number Sweep\n");
  printf("=================================================\n\n");

  printf("Parameters:\n");
  printf("  We = %g\n", We);
  printf("  η = %g\n", eta);
  printf("  μ_ratio = %g\n", mu_ratio);
  printf("  Grid: %d × %d (Lmax = %d)\n\n", N, N, Lmax_grid);

  // Reynolds number sweep (subset for speed)
  double Re_values[] = {22, 50, 100, 150, 200};
  int n_cases = 5;

  BenchmarkResult results[5];

  printf("Running %d cases:\n", n_cases);

  for (int i = 0; i < n_cases; i++) {
    results[i] = run_single_case(Re_values[i]);
  }

  // Summary
  printf("\n=================================================\n");
  printf("Benchmark Summary\n");
  printf("=================================================\n\n");

  printf("%-8s %-12s %-12s %-12s %-12s %-12s\n",
         "Re", "D_max/D0", "t_contact", "h_rebound", "CPU(s)", "Mass_err(%)");
  printf("------------------------------------------------------------------------\n");

  for (int i = 0; i < n_cases; i++) {
    printf("%-8.0f %-12.4f %-12.4f %-12.4f %-12.2f %-12.6f\n",
           results[i].Re,
           results[i].D_max_ratio,
           results[i].contact_time,
           results[i].rebound_height,
           results[i].cpu_time,
           100.0 * results[i].mass_error);
  }

  printf("\n");

  // Validation checks
  bool all_passed = true;

  for (int i = 0; i < n_cases; i++) {
    // Check physical bounds
    if (results[i].D_max_ratio < 1.0 || results[i].D_max_ratio > 3.0) {
      printf("WARNING: Re=%g has D_max/D0 = %g (expected 1.0-3.0)\n",
             results[i].Re, results[i].D_max_ratio);
      all_passed = false;
    }

    // Check mass conservation
    if (results[i].mass_error > 0.02) {
      printf("WARNING: Re=%g has mass error = %g%% (> 2%%)\n",
             results[i].Re, 100.0 * results[i].mass_error);
      all_passed = false;
    }
  }

  if (all_passed) {
    printf("All benchmark cases PASSED ✓\n");
  } else {
    printf("Some benchmark cases FAILED ✗\n");
  }

  printf("\n=================================================\n");

  return all_passed ? 0 : 1;
}
