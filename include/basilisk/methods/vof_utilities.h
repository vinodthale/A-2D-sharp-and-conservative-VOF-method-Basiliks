/**
 * VOF Utility Functions
 *
 * This file provides utility functions for Volume-of-Fluid (VOF) methods
 * that are used throughout the Basilisk VOF implementation.
 */

#ifndef VOF_UTILITIES_H
#define VOF_UTILITIES_H

/**
 * swap macro - Generic swap for any type
 */
#define swap(type, a, b) { type tmp = (a); (a) = (b); (b) = tmp; }

/**
 * mycs() - Mixed Youngs-Centered Scheme for computing interface normal
 *
 * Computes the interface normal vector from the volume fraction field
 * using a Mixed Youngs-Centered (MYC) scheme. This is a standard method
 * in VOF for estimating the interface orientation.
 *
 * @param point Current point in the grid
 * @param c Volume fraction scalar field
 * @return coord Normal vector to the interface (not normalized)
 */
static inline coord mycs (Point point, scalar c)
{
  coord n;

#if dimension == 2
  /**
   * 2D implementation: Use central differences with neighbor averaging
   * This implements the Mixed Youngs-Centered scheme
   */
  double dx = 0., dy = 0.;

  // x-direction gradient estimation
  for (int i = -1; i <= 1; i++) {
    double cx_m = c[-1,i];
    double cx_p = c[+1,i];
    dx += (cx_p - cx_m) * (i == 0 ? 2.0 : 1.0);
  }

  // y-direction gradient estimation
  for (int i = -1; i <= 1; i++) {
    double cy_m = c[i,-1];
    double cy_p = c[i,+1];
    dy += (cy_p - cy_m) * (i == 0 ? 2.0 : 1.0);
  }

  n.x = -dx;
  n.y = -dy;

#else // dimension == 3
  /**
   * 3D implementation: Use central differences with neighbor averaging
   */
  double dx = 0., dy = 0., dz = 0.;

  // x-direction gradient
  for (int i = -1; i <= 1; i++)
    for (int j = -1; j <= 1; j++) {
      double weight = (i == 0 ? 2.0 : 1.0) * (j == 0 ? 2.0 : 1.0);
      dx += (c[+1,i,j] - c[-1,i,j]) * weight;
    }

  // y-direction gradient
  for (int i = -1; i <= 1; i++)
    for (int j = -1; j <= 1; j++) {
      double weight = (i == 0 ? 2.0 : 1.0) * (j == 0 ? 2.0 : 1.0);
      dy += (c[i,+1,j] - c[i,-1,j]) * weight;
    }

  // z-direction gradient
  for (int i = -1; i <= 1; i++)
    for (int j = -1; j <= 1; j++) {
      double weight = (i == 0 ? 2.0 : 1.0) * (j == 0 ? 2.0 : 1.0);
      dz += (c[i,j,+1] - c[i,j,-1]) * weight;
    }

  n.x = -dx;
  n.y = -dy;
  n.z = -dz;
#endif

  return n;
}

/**
 * line_area() - Compute the area below a line in a unit square
 *
 * Computes the area of the region {(x,y) : nx*x + ny*y ≤ alpha, 0 ≤ x ≤ 1, 0 ≤ y ≤ 1}
 * This is the standard PLIC (Piecewise Linear Interface Calculation) area computation.
 *
 * The algorithm assumes the inputs are already normalized such that |nx| + |ny| = 1.
 * If not normalized, the function will normalize them.
 *
 * @param nx x-component of the normal vector (will be made positive)
 * @param ny y-component of the normal vector (will be made positive)
 * @param alpha plane constant (normalized distance)
 * @return The area below the line in the unit square [0, 1]
 */
static inline double line_area (double nx, double ny, double alpha)
{
  // Work with absolute values (by symmetry)
  nx = fabs(nx);
  ny = fabs(ny);

  // Normalize to make nx + ny = 1
  double n = nx + ny;

  if (n == 0.)
    return alpha > 0. ? 1. : 0.;

  alpha /= n;
  nx /= n;
  ny /= n;

  // Now nx + ny = 1, nx ≥ 0, ny ≥ 0

  // Boundary cases
  if (alpha <= 0.)
    return 0.;

  if (alpha >= 1.)
    return 1.;

  /**
   * Standard PLIC area formula for 0 < alpha < 1:
   *
   * The line nx*x + ny*y = alpha divides the unit square.
   * We compute the area where nx*x + ny*y ≤ alpha.
   *
   * Three cases based on the value of alpha:
   */

  // Case 1: Small alpha - triangle in the corner
  // The line cuts through bottom and left edges only
  if (alpha <= nx * ny) {
    return alpha * alpha / (2. * nx * ny);
  }

  // Case 2: Middle range - trapezoid
  // The line cuts through different edge pairs
  else if (alpha <= 1. - nx * ny) {
    return alpha - nx * ny / 2.;
  }

  // Case 3: Large alpha - almost full square minus a corner triangle
  // Complement of the small triangle case
  else {
    double beta = 1. - alpha;
    return 1. - beta * beta / (2. * nx * ny);
  }
}

#endif // VOF_UTILITIES_H
