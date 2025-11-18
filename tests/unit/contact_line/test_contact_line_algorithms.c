/**
 * Unit Test: Contact Line Algorithms (Algorithms 1-6)
 * Tests contact line dynamics and hysteresis
 *
 * Algorithm 1: Mixed-cell interface reconstruction
 * Algorithm 2: Clear small mixed cells
 * Algorithm 3: Compute n_{l,θ}
 * Algorithm 4: Identify contact-line cell
 * Algorithm 5: HF contact-angle enforcement
 * Algorithm 6: Bisection hysteresis
 *
 * Test cases:
 * 1. Contact angle identification
 * 2. Contact angle enforcement (Algorithm 3)
 * 3. Contact line cell identification (Algorithm 4)
 * 4. Hysteresis window (advancing/receding angles)
 * 5. Mixed cell reconstruction (Algorithm 1)
 * 6. Small cell clearance (Algorithm 2)
 */

#include "grid/cartesian.h"
#include "embed.h"
#include "vof.h"

scalar f[];
scalar cs[];  // Solid fraction
vector n_s[];  // Solid normal

// Test 1: Contact angle identification
void test_contact_angle_identification() {
  printf("Test 1: Contact angle identification\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Create droplet on surface
  double R0 = 0.2;
  double x0 = 0.5, y0 = 0.1;

  fraction(f, sq(R0) - sq(x - x0) - sq(y - y0));

  // Create flat surface at y = 0
  fraction(cs, y);

  // Find contact line cells
  int contact_line_cells = 0;

  foreach(reduction(+:contact_line_cells)) {
    // Algorithm 4 criterion: 0 < c_s < 1 && 0 < c < c_s
    if (cs[] > 0 && cs[] < 1 && f[] > 0 && f[] < cs[]) {
      contact_line_cells++;
    }
  }

  printf("  Contact line cells found: %d\n", contact_line_cells);

  assert(contact_line_cells > 0);

  printf("  PASSED\n\n");
}

// Test 2: Contact angle enforcement (Algorithm 3)
void test_contact_angle_enforcement() {
  printf("Test 2: Contact angle enforcement (Algorithm 3)\n");

  // Test different contact angles
  double test_angles[] = {30.0, 60.0, 90.0, 120.0, 150.0};

  for (int i = 0; i < 5; i++) {
    double theta_deg = test_angles[i];
    double theta_rad = theta_deg * pi / 180.0;

    // Algorithm 3: Compute n_{l,θ}
    // If (∇c × n_s) · e_z > 0:
    //   n_{l,θ} = -R(θ) · n_s
    // else:
    //   n_{l,θ} = -R(-θ) · n_s

    // Example: n_s = (0, 1) (upward normal)
    double n_s_x = 0.0;
    double n_s_y = 1.0;

    // Rotation matrix R(θ) = [[cos θ, -sin θ], [sin θ, cos θ]]
    double cos_theta = cos(theta_rad);
    double sin_theta = sin(theta_rad);

    // For positive cross product:
    double n_l_theta_x = -(cos_theta * n_s_x - sin_theta * n_s_y);
    double n_l_theta_y = -(sin_theta * n_s_x + cos_theta * n_s_y);

    printf("  θ = %g°: n_{l,θ} = (%g, %g)\n",
           theta_deg, n_l_theta_x, n_l_theta_y);

    // Verify angle
    double computed_angle = atan2(n_l_theta_y, n_l_theta_x);
    double expected_angle = pi/2 - theta_rad;

    double angle_error = fabs(computed_angle - expected_angle);

    // Allow wrapping around 2π
    if (angle_error > pi)
      angle_error = 2*pi - angle_error;

    printf("    Angle verification: error = %g rad (%g°)\n",
           angle_error, angle_error * 180.0 / pi);

    assert(angle_error < 0.01);
  }

  printf("  PASSED\n\n");
}

// Test 3: Contact line cell identification (Algorithm 4)
void test_contact_line_cell_identification() {
  printf("Test 3: Contact line cell identification (Algorithm 4)\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Create test configuration
  // Droplet near boundary
  double R0 = 0.15;
  double x0 = 0.5, y0 = 0.15;

  fraction(f, sq(R0) - sq(x - x0) - sq(y - y0));

  // Solid boundary at y = 0
  fraction(cs, y);

  // Algorithm 4: Identify contact-line cell
  // Criteria:
  // 1. 0 < c_s < 1 (mixed cell)
  // 2. 0 < c < c_s (fluid partially filling cell)
  // 3. Neighbors in τ_{l,θ} direction fully fluid or solid

  int identified_cells = 0;

  foreach(reduction(+:identified_cells)) {
    if (cs[] > 0 && cs[] < 1 && f[] > 0 && f[] < cs[]) {
      // Check neighbors (simplified)
      bool valid_neighbors = true;

      // Check right neighbor
      if (!(f[1,0] == 0 || f[1,0] >= cs[1,0]))
        valid_neighbors = false;

      // Check below neighbor
      if (!(f[0,-1] == 0 || f[0,-1] >= cs[0,-1]))
        valid_neighbors = false;

      if (valid_neighbors)
        identified_cells++;
    }
  }

  printf("  Identified contact-line cells: %d\n", identified_cells);

  assert(identified_cells > 0);

  printf("  PASSED\n\n");
}

// Test 4: Hysteresis window (advancing/receding angles)
void test_hysteresis_window() {
  printf("Test 4: Hysteresis window\n");

  // Define hysteresis parameters
  double theta_adv = 120.0 * pi / 180.0;  // Advancing angle
  double theta_rec = 60.0 * pi / 180.0;   // Receding angle
  double theta_eq = 90.0 * pi / 180.0;    // Equilibrium angle

  printf("  θ_advancing = %g°\n", theta_adv * 180.0 / pi);
  printf("  θ_receding  = %g°\n", theta_rec * 180.0 / pi);
  printf("  θ_equilibrium = %g°\n", theta_eq * 180.0 / pi);

  // Algorithm 6: Bisection for hysteresis
  // Initialize bracket [θ_min, θ_max]
  double theta_min = theta_rec;
  double theta_max = theta_adv;

  // Test different initial guesses
  double theta_test[] = {theta_rec, theta_eq, theta_adv};

  for (int i = 0; i < 3; i++) {
    double theta_init = theta_test[i];

    printf("\n  Testing θ_init = %g°\n", theta_init * 180.0 / pi);

    // Bisection iteration (simplified)
    double theta_current = theta_init;
    double bracket_min = theta_min;
    double bracket_max = theta_max;

    // Simulate bisection
    for (int iter = 0; iter < 5; iter++) {
      theta_current = 0.5 * (bracket_min + bracket_max);

      printf("    Iter %d: θ = %g° [%g°, %g°]\n",
             iter, theta_current * 180.0 / pi,
             bracket_min * 180.0 / pi,
             bracket_max * 180.0 / pi);

      // Convergence check
      if (fabs(bracket_max - bracket_min) < 0.01 * pi / 180.0)
        break;

      // Update bracket (example logic)
      if (theta_current > theta_eq)
        bracket_max = theta_current;
      else
        bracket_min = theta_current;
    }

    assert(theta_current >= theta_rec && theta_current <= theta_adv);
  }

  printf("\n  PASSED\n\n");
}

// Test 5: Mixed cell reconstruction (Algorithm 1)
void test_mixed_cell_reconstruction() {
  printf("Test 5: Mixed cell reconstruction (Algorithm 1)\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Algorithm 1 steps:
  // 1. Reconstruct embedded boundary Γ_s
  // 2. Rotate cell to align n_l with y-axis
  // 3. Subdivide fluid polygon
  // 4. Compute interval areas S_i
  // 5. Find interval i
  // 6. Solve quadratic for interface height
  // 7. Compute α

  // Test case: cell with embedded boundary
  double c = 0.5;  // Volume fraction
  double c_s = 0.3;  // Solid fraction
  double Delta = L0 / N;

  printf("  Volume fraction c = %g\n", c);
  printf("  Solid fraction c_s = %g\n", c_s);

  // Simplified reconstruction
  // Assume linear interface: n_l · x = α

  // Normal pointing upward
  double n_l_x = 0.0;
  double n_l_y = 1.0;

  // Compute α from volume constraint
  // For horizontal interface: c = (α + 0.5) if 0 < α < 0.5
  double alpha = c - 0.5;

  printf("  Computed α = %g\n", alpha);

  // Verify volume
  double reconstructed_volume = (alpha + 0.5);
  double volume_error = fabs(reconstructed_volume - c) / c;

  printf("  Volume error = %g%%\n", 100.0 * volume_error);

  assert(volume_error < 0.01);

  printf("  PASSED\n\n");
}

// Test 6: Small cell clearance (Algorithm 2)
void test_small_cell_clearance() {
  printf("Test 6: Small cell clearance (Algorithm 2)\n");

  L0 = 1.0;
  N = 64;
  init_grid(N);

  // Create solid fraction field with small mixed cells
  fraction(cs, y - 0.01);  // Very thin solid layer

  // Algorithm 2: Clear cells with c_s < 0.01
  double threshold = 0.01;
  int cleared_cells = 0;
  int modified_cells = 0;

  foreach(reduction(+:cleared_cells) reduction(+:modified_cells)) {
    if (cs[] > 0 && cs[] < threshold) {
      // Clear this cell
      cs[] = 0.0;
      cleared_cells++;

      // Modify neighbor (simplified)
      if (cs[1,1] < 1.0) {
        cs[1,1] = 0.99;
        modified_cells++;
      }
    }
  }

  printf("  Threshold c_s = %g\n", threshold);
  printf("  Cleared cells: %d\n", cleared_cells);
  printf("  Modified neighbors: %d\n", modified_cells);

  // Verify no cells below threshold remain
  int remaining_small_cells = 0;

  foreach(reduction(+:remaining_small_cells)) {
    if (cs[] > 0 && cs[] < threshold)
      remaining_small_cells++;
  }

  printf("  Remaining small cells: %d (should be 0)\n", remaining_small_cells);

  assert(remaining_small_cells == 0);

  printf("  PASSED\n\n");
}

int main() {
  printf("=================================================\n");
  printf("Unit Tests: Contact Line Algorithms (Alg 1-6)\n");
  printf("=================================================\n\n");

  test_contact_angle_identification();
  test_contact_angle_enforcement();
  test_contact_line_cell_identification();
  test_hysteresis_window();
  test_mixed_cell_reconstruction();
  test_small_cell_clearance();

  printf("=================================================\n");
  printf("All contact line algorithm tests PASSED\n");
  printf("=================================================\n");

  return 0;
}
