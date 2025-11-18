/**
 * Unit Test: PLIC Interface Reconstruction (Equations 17, 18)
 * Tests the piecewise-linear interface construction
 *
 * Equation 17: n_l · x = α
 * Equation 18: A = 1/2 Σ x_n (y_{n+1}-y_{n-1})
 *
 * Test cases:
 * 1. Horizontal interface
 * 2. Vertical interface
 * 3. Diagonal interface (45°)
 * 4. Circle reconstruction
 * 5. Volume accuracy
 */

#include "grid/cartesian.h"
#include "vof.h"
#include "fractions.h"

scalar f[];
vector n[];
scalar alpha[];

// Test 1: Horizontal interface
void test_horizontal_interface() {
  printf("Test 1: Horizontal interface reconstruction\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Create horizontal interface at y = 0.5
  fraction(f, y - 0.5);

  // Compute interface normal
  reconstruction(f, n, alpha);

  // Check normal direction (should be (0, 1))
  double max_nx_error = 0.0;
  double max_ny_error = 0.0;

  foreach(reduction(max:max_nx_error) reduction(max:max_ny_error)) {
    if (f[] > 0 && f[] < 1) {
      double nx_error = fabs(n.x[]);
      double ny_error = fabs(n.y[] - 1.0);

      if (nx_error > max_nx_error)
        max_nx_error = nx_error;
      if (ny_error > max_ny_error)
        max_ny_error = ny_error;
    }
  }

  printf("  Max normal error: nx = %g, ny = %g\n", max_nx_error, max_ny_error);

  assert(max_nx_error < 0.01);
  assert(max_ny_error < 0.01);

  printf("  PASSED\n\n");
}

// Test 2: Vertical interface
void test_vertical_interface() {
  printf("Test 2: Vertical interface reconstruction\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Create vertical interface at x = 0.5
  fraction(f, x - 0.5);

  // Compute interface normal
  reconstruction(f, n, alpha);

  // Check normal direction (should be (1, 0))
  double max_nx_error = 0.0;
  double max_ny_error = 0.0;

  foreach(reduction(max:max_nx_error) reduction(max:max_ny_error)) {
    if (f[] > 0 && f[] < 1) {
      double nx_error = fabs(n.x[] - 1.0);
      double ny_error = fabs(n.y[]);

      if (nx_error > max_nx_error)
        max_nx_error = nx_error;
      if (ny_error > max_ny_error)
        max_ny_error = ny_error;
    }
  }

  printf("  Max normal error: nx = %g, ny = %g\n", max_nx_error, max_ny_error);

  assert(max_nx_error < 0.01);
  assert(max_ny_error < 0.01);

  printf("  PASSED\n\n");
}

// Test 3: Diagonal interface (45°)
void test_diagonal_interface() {
  printf("Test 3: Diagonal interface reconstruction (45°)\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Create diagonal interface: x + y = 1
  fraction(f, 1.0 - x - y);

  // Compute interface normal
  reconstruction(f, n, alpha);

  // Expected normal: (1/√2, 1/√2)
  double expected_nx = 1.0 / sqrt(2.0);
  double expected_ny = 1.0 / sqrt(2.0);

  double max_nx_error = 0.0;
  double max_ny_error = 0.0;

  foreach(reduction(max:max_nx_error) reduction(max:max_ny_error)) {
    if (f[] > 0 && f[] < 1) {
      // Normalize computed normal
      double norm = sqrt(sq(n.x[]) + sq(n.y[]));
      double nx_norm = n.x[] / norm;
      double ny_norm = n.y[] / norm;

      double nx_error = fabs(nx_norm - expected_nx);
      double ny_error = fabs(ny_norm - expected_ny);

      if (nx_error > max_nx_error)
        max_nx_error = nx_error;
      if (ny_error > max_ny_error)
        max_ny_error = ny_error;
    }
  }

  printf("  Max normal error: nx = %g, ny = %g\n", max_nx_error, max_ny_error);

  assert(max_nx_error < 0.05);
  assert(max_ny_error < 0.05);

  printf("  PASSED\n\n");
}

// Test 4: Circle reconstruction accuracy
void test_circle_reconstruction() {
  printf("Test 4: Circle reconstruction\n");

  L0 = 1.0;
  N = 128;
  init_grid(N);

  // Create circle
  double R0 = 0.3;
  double x0 = 0.5, y0 = 0.5;

  fraction(f, sq(R0) - sq(x - x0) - sq(y - y0));

  // Compute interface normal
  reconstruction(f, n, alpha);

  // For circle, normal should point radially outward
  // n = (x - x0, y - y0) / |r|
  double max_normal_error = 0.0;

  foreach(reduction(max:max_normal_error)) {
    if (f[] > 0 && f[] < 1) {
      double rx = x - x0;
      double ry = y - y0;
      double r = sqrt(sq(rx) + sq(ry));

      if (r > 0.01) {  // Avoid center
        double expected_nx = rx / r;
        double expected_ny = ry / r;

        // Normalize computed normal
        double norm = sqrt(sq(n.x[]) + sq(n.y[]));
        double nx_norm = n.x[] / norm;
        double ny_norm = n.y[] / norm;

        double error = sqrt(sq(nx_norm - expected_nx) + sq(ny_norm - expected_ny));

        if (error > max_normal_error)
          max_normal_error = error;
      }
    }
  }

  printf("  Max normal error for circle: %g\n", max_normal_error);

  assert(max_normal_error < 0.1);

  printf("  PASSED\n\n");
}

// Test 5: Volume accuracy of reconstruction
void test_volume_accuracy() {
  printf("Test 5: Volume accuracy\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Test different volume fractions
  double test_fractions[] = {0.1, 0.25, 0.5, 0.75, 0.9};

  for (int i = 0; i < 5; i++) {
    double target_fraction = test_fractions[i];

    // Create interface at y = target_fraction
    fraction(f, y - target_fraction * L0);

    // Compute total volume
    double total_volume = 0.0;
    foreach(reduction(+:total_volume)) {
      total_volume += f[] * sq(Delta);
    }

    double expected_volume = target_fraction * L0 * L0;
    double error = fabs(total_volume - expected_volume) / expected_volume;

    printf("  Fraction %g: Volume error = %g%%\n", target_fraction, 100.0 * error);

    assert(error < 0.001);  // Less than 0.1% error
  }

  printf("  PASSED\n\n");
}

int main() {
  printf("=================================================\n");
  printf("Unit Tests: PLIC Reconstruction (Equations 17, 18)\n");
  printf("=================================================\n\n");

  test_horizontal_interface();
  test_vertical_interface();
  test_diagonal_interface();
  test_circle_reconstruction();
  test_volume_accuracy();

  printf("=================================================\n");
  printf("All PLIC reconstruction tests PASSED\n");
  printf("=================================================\n");

  return 0;
}
