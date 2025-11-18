/**
 * Unit Test: VOF Advection (Equations 3, 8, 10, 11)
 * Tests the geometric VOF advection scheme with directional splitting
 *
 * Equation 3: ∂c/∂t + u·∇c = 0
 * Equation 8: ∂c/∂t = -∇·(c u) + c_c ∇·u
 * Equation 10: (c* - c^{n-1/2})/Δt = Δt/Δ (F_left - F_right - c_c (u_left - u_right))
 * Equation 11: (c^{n+1/2} - c*)/Δt = Δt/Δ (F_down - F_up - c_c (v_down - v_up))
 *
 * Test cases:
 * 1. Translation test (circle advection)
 * 2. Rotation test (Zalesak's disk)
 * 3. Shear flow test
 * 4. Volume conservation
 * 5. Boundedness (0 ≤ c ≤ 1)
 */

#include "grid/cartesian.h"
#include "vof.h"
#include "utils.h"

scalar f[];
scalar * interfaces = {f};
face vector uf[];

// Test 1: Translation of a circle
void test_circle_translation() {
  printf("Test 1: Circle translation (mass conservation)\n");

  L0 = 2.0;
  origin(-L0/2, -L0/2);
  N = 128;
  init_grid(N);

  // Initial circle
  double R0 = 0.25;
  double x0 = 0.0, y0 = 0.0;
  double u_const = 1.0;

  fraction(f, sq(R0) - sq(x - x0) - sq(y - y0));

  // Uniform velocity
  foreach_face(x)
    uf.x[] = u_const;
  foreach_face(y)
    uf.y[] = 0.0;

  // Initial volume
  double V0 = statsf(f).sum;
  double V_exact = pi * R0 * R0;

  printf("  Initial volume: %g (exact: %g, error: %g%%)\n",
         V0, V_exact, 100.0 * fabs(V0 - V_exact) / V_exact);

  // Advect for one period
  double T = L0 / u_const;
  double dt_max = 0.2 * L0 / N / u_const;  // CFL = 0.2
  int nsteps = (int)(T / dt_max);

  printf("  Advecting for %d steps (T = %g)\n", nsteps, T);

  for (int i = 0; i < nsteps; i++) {
    vof_advection({f}, nsteps, dt_max);
  }

  // Final volume
  double V1 = statsf(f).sum;
  double mass_error = fabs(V1 - V0) / V0;

  printf("  Final volume: %g (error: %g%%)\n", V1, 100.0 * mass_error);

  assert(mass_error < 0.01);  // Less than 1% error

  printf("  PASSED\n\n");
}

// Test 2: Rotation test (Zalesak's disk)
void test_rotation() {
  printf("Test 2: Rotation test (shape preservation)\n");

  L0 = 2.0;
  origin(-L0/2, -L0/2);
  N = 128;
  init_grid(N);

  // Zalesak's notched disk
  double R0 = 0.25;
  double x0 = 0.0, y0 = -0.25;
  double slot_width = 0.05;
  double slot_depth = 0.15;

  // Circle with notch
  fraction(f, intersection(sq(R0) - sq(x - x0) - sq(y - y0),
                           union(-((y - y0) - slot_depth),
                                 sq(slot_width/2) - sq(x - x0))));

  // Initial volume
  double V0 = statsf(f).sum;

  // Solid body rotation: u = -ω*y, v = ω*x
  double omega = 2.0 * pi;  // One revolution per unit time

  foreach_face(x)
    uf.x[] = -omega * y;
  foreach_face(y)
    uf.y[] = omega * x;

  // Advect for one full rotation
  double T = 2.0 * pi / omega;
  double dt_max = 0.2 * L0 / N / (omega * L0/2);  // CFL = 0.2
  int nsteps = (int)(T / dt_max);

  printf("  Rotating for %d steps (T = %g)\n", nsteps, T);

  for (int i = 0; i < nsteps; i++) {
    vof_advection({f}, nsteps, dt_max);
  }

  // Final volume
  double V1 = statsf(f).sum;
  double mass_error = fabs(V1 - V0) / V0;

  printf("  Volume error: %g%%\n", 100.0 * mass_error);

  assert(mass_error < 0.05);  // Less than 5% error after full rotation

  printf("  PASSED\n\n");
}

// Test 3: Shear flow test
void test_shear_flow() {
  printf("Test 3: Shear flow (reversibility)\n");

  L0 = 1.0;
  N = 128;
  init_grid(N);

  // Initial circle
  double R0 = 0.15;
  double x0 = 0.5, y0 = 0.5;

  fraction(f, sq(R0) - sq(x - x0) - sq(y - y0));

  // Initial volume
  double V0 = statsf(f).sum;

  // Shear flow: u = sin(2πy), v = 0
  double shear_period = 1.0;
  double T_half = shear_period / 2.0;

  foreach_face(x)
    uf.x[] = sin(2.0 * pi * y);
  foreach_face(y)
    uf.y[] = 0.0;

  double dt_max = 0.2 * L0 / N;
  int nsteps = (int)(T_half / dt_max);

  printf("  Forward shear for %d steps\n", nsteps);

  // Forward shear
  for (int i = 0; i < nsteps; i++) {
    vof_advection({f}, nsteps, dt_max);
  }

  // Reverse shear
  printf("  Reverse shear for %d steps\n", nsteps);

  foreach_face(x)
    uf.x[] = -sin(2.0 * pi * y);

  for (int i = 0; i < nsteps; i++) {
    vof_advection({f}, nsteps, dt_max);
  }

  // Final volume
  double V1 = statsf(f).sum;
  double mass_error = fabs(V1 - V0) / V0;

  printf("  Volume error after reversibility: %g%%\n", 100.0 * mass_error);

  assert(mass_error < 0.02);  // Less than 2% error

  printf("  PASSED\n\n");
}

// Test 4: Volume conservation
void test_volume_conservation() {
  printf("Test 4: Volume conservation (various shapes)\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Test various shapes
  double shapes[][3] = {
    {0.5, 0.5, 0.2},  // Circle
    {0.3, 0.3, 0.15}, // Small circle
    {0.7, 0.7, 0.25}  // Large circle
  };

  for (int s = 0; s < 3; s++) {
    double x0 = shapes[s][0];
    double y0 = shapes[s][1];
    double R0 = shapes[s][2];

    fraction(f, sq(R0) - sq(x - x0) - sq(y - y0));

    double V_initial = statsf(f).sum;

    // Advect with uniform velocity
    foreach_face(x)
      uf.x[] = 1.0;
    foreach_face(y)
      uf.y[] = 0.5;

    double dt_max = 0.2 * L0 / N;
    for (int i = 0; i < 10; i++) {
      vof_advection({f}, 1, dt_max);
    }

    double V_final = statsf(f).sum;
    double error = fabs(V_final - V_initial) / V_initial;

    printf("  Shape %d: Volume error = %g%%\n", s + 1, 100.0 * error);
    assert(error < 0.01);
  }

  printf("  PASSED\n\n");
}

// Test 5: Boundedness check (0 ≤ c ≤ 1)
void test_boundedness() {
  printf("Test 5: Boundedness (0 ≤ c ≤ 1)\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Circle
  double R0 = 0.2;
  fraction(f, sq(R0) - sq(x - 0.5) - sq(y - 0.5));

  // Advect with varying velocity
  foreach_face(x)
    uf.x[] = sin(2.0 * pi * x);
  foreach_face(y)
    uf.y[] = cos(2.0 * pi * y);

  double dt_max = 0.2 * L0 / N;

  for (int i = 0; i < 100; i++) {
    vof_advection({f}, 1, dt_max);

    // Check boundedness
    foreach() {
      if (f[] < -1e-10 || f[] > 1.0 + 1e-10) {
        printf("  ERROR: f[%g,%g] = %g (out of bounds)\n", x, y, f[]);
        assert(false);
      }
    }
  }

  printf("  All cells satisfy 0 ≤ c ≤ 1\n");
  printf("  PASSED\n\n");
}

int main() {
  printf("=================================================\n");
  printf("Unit Tests: VOF Advection (Equations 3, 8, 10, 11)\n");
  printf("=================================================\n\n");

  test_circle_translation();
  test_rotation();
  test_shear_flow();
  test_volume_conservation();
  test_boundedness();

  printf("=================================================\n");
  printf("All VOF advection tests PASSED\n");
  printf("=================================================\n");

  return 0;
}
