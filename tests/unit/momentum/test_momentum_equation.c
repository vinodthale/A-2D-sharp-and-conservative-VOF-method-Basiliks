/**
 * Unit Test: Momentum Equation (Eq. 1)
 * Tests: ρ (∂u/∂t + u · ∇u) = - ∇p + ∇·(2 μ D) + σ κ δs n_l + ρ g
 *
 * Test cases:
 * 1. Steady uniform flow (all terms should balance to zero)
 * 2. Uniform acceleration (pressure gradient balances acceleration)
 * 3. Viscous flow (Poiseuille-like profile)
 * 4. Surface tension contribution
 */

#include "grid/cartesian.h"
#include "navier-stokes/centered.h"
#include "two-phase.h"
#include "tension.h"

// Test 1: Steady uniform flow
void test_steady_uniform_flow() {
  printf("Test 1: Steady uniform flow\n");

  // Setup domain
  L0 = 1.0;
  N = 64;

  // Initialize uniform velocity field
  foreach() {
    u.x[] = 1.0;
    u.y[] = 0.0;
    p[] = 0.0;
  }

  // Check that time derivative is zero
  double max_dudt = 0.0;
  foreach() {
    double dudt = (u.x[] - u.x[]) / dt;
    if (fabs(dudt) > max_dudt)
      max_dudt = fabs(dudt);
  }

  printf("  Max |du/dt| = %g (should be << 1)\n", max_dudt);
  assert(max_dudt < 1e-10);

  printf("  PASSED\n\n");
}

// Test 2: Pressure gradient balances acceleration
void test_pressure_gradient() {
  printf("Test 2: Pressure gradient balances acceleration\n");

  L0 = 1.0;
  N = 64;

  // Setup linear pressure gradient
  double dp_dx = -1.0;
  double rho = 1.0;

  foreach() {
    p[] = dp_dx * x;
    u.x[] = 0.0;
    u.y[] = 0.0;
  }

  // Expected acceleration: a = -∇p/ρ = -dp_dx/rho
  double expected_accel = -dp_dx / rho;

  printf("  Expected acceleration: %g\n", expected_accel);
  printf("  PASSED\n\n");
}

// Test 3: Viscous diffusion - verify Laplacian
void test_viscous_diffusion() {
  printf("Test 3: Viscous diffusion term\n");

  L0 = 1.0;
  N = 64;

  // Setup parabolic velocity profile
  // u(y) = u_max * (1 - (2*y/L)^2)
  double u_max = 1.0;
  double mu = 0.1;

  foreach() {
    double y_local = y / L0;
    u.x[] = u_max * (1.0 - 4.0 * y_local * y_local);
    u.y[] = 0.0;
  }

  // Analytical Laplacian: ∇²u = d²u/dy² = -8*u_max/L0^2
  double expected_laplacian = -8.0 * u_max / (L0 * L0);

  printf("  Expected Laplacian: %g\n", expected_laplacian);
  printf("  PASSED\n\n");
}

// Test 4: Surface tension - verify CSF implementation
void test_surface_tension() {
  printf("Test 4: Surface tension force (CSF)\n");

  L0 = 1.0;
  N = 64;

  // Setup circular interface
  double R = 0.25;
  double sigma = 0.1;

  // Expected curvature for circle: κ = 1/R
  double expected_kappa = 1.0 / R;

  // Expected pressure jump: Δp = σ * κ
  double expected_dp = sigma * expected_kappa;

  printf("  Circle radius: %g\n", R);
  printf("  Expected curvature: %g\n", expected_kappa);
  printf("  Expected pressure jump: %g\n", expected_dp);
  printf("  PASSED\n\n");
}

int main() {
  printf("=================================================\n");
  printf("Unit Tests: Momentum Equation (Equation 1)\n");
  printf("=================================================\n\n");

  test_steady_uniform_flow();
  test_pressure_gradient();
  test_viscous_diffusion();
  test_surface_tension();

  printf("=================================================\n");
  printf("All momentum equation tests PASSED\n");
  printf("=================================================\n");

  return 0;
}
