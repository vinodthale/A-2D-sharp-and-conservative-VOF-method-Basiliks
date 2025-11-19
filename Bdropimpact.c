/**
 * Bdropimpact.c - Axisymmetric droplet impact simulation
 *
 * Main simulation driver for 2D axisymmetric droplet impact on liquid pool
 * using Basilisk's VOF method with adaptive mesh refinement.
 *
 * Features:
 *   - Axisymmetric geometry (physical z → Basilisk x, physical r → Basilisk y)
 *   - Two-phase flow with surface tension
 *   - Optional bubble entrapment
 *   - Adaptive mesh refinement (AMR)
 *   - MPI parallelization support
 *   - Flexible parameter input (dimensional/nondimensional)
 *
 * Coordinate mapping:
 *   Physical: (r, z) with r ≥ 0
 *   Basilisk: (y, x) where y = r (radial), x = z (axial)
 *   Gravity acts in negative x direction (downward)
 *
 * Compilation:
 *   Serial:  qcc -O2 -Wall Bdropimpact.c -o Bdropimpact -lm
 *   MPI:     qcc -source -D_MPI=1 Bdropimpact.c
 *            mpicc -O2 -Wall -std=c99 -D_MPI=1 _Bdropimpact.c -o Bdropimpact -lm
 *
 * Execution:
 *   ./Bdropimpact [R2448] [W873] [F180] [H3.33] [x12] [n4] [ts0.001] [te10]
 *
 * Author: Basilisk simulation framework
 * Date: 2025
 */

// =============================================================================
// BASILISK INCLUDES
// =============================================================================

#include "axi.h"                          // Axisymmetric geometry
#include "navier-stokes/centered.h"       // Centered Navier-Stokes solver
#include "two-phase.h"                    // VOF two-phase interface tracking
#include "tension.h"                      // Surface tension forces
#include "tag.h"                          // Droplet/bubble counting
#include "curvature.h"                    // Interface curvature calculation

// Include physical parameters and utilities
#include "constants.h"

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

// Simulation parameters structure
SimParams cfdbv;

// VOF tracers
scalar f[];         // Primary phase (droplet)
scalar fb[];        // Secondary phase (bubble, if present)
scalar * tracers = {f, fb};

// Drop and bubble positions
double x0;          // Drop center (axial direction)
double y0;          // Drop center (radial direction) - always 0 for axisymmetry
double Bubtx0;      // Bubble center (axial)
double Bubty0;      // Bubble center (radial) - always 0

// Time tracking
double timecontact; // Contact time

// Output files
FILE * fp_volume;   // Volume conservation log
FILE * fp_interface;// Interface position log
FILE * fp_duration; // Simulation duration log

// CPU info for output naming
int cpu_rank = 0;

// =============================================================================
// BOUNDARY CONDITIONS
// =============================================================================

/**
 * Left boundary (axis of symmetry, r = 0)
 * Axisymmetric boundary conditions
 */
u.n[left] = dirichlet(0.);     // u_r = 0 (no radial flow)
u.t[left] = neumann(0.);       // ∂u_z/∂r = 0 (symmetry)
f[left] = neumann(0.);         // ∂f/∂r = 0 (symmetry)
p[left] = neumann(0.);         // ∂p/∂r = 0 (symmetry)

/**
 * Right boundary (far field, r = L0)
 * Outflow conditions
 */
u.n[right] = neumann(0.);      // Zero gradient
u.t[right] = neumann(0.);      // Zero gradient
p[right] = dirichlet(0.);      // Zero pressure (reference)
f[right] = neumann(0.);        // Zero gradient

/**
 * Top boundary (z = L0)
 * Outflow with zero pressure
 */
u.n[top] = neumann(0.);        // Zero gradient
u.t[top] = neumann(0.);        // Zero gradient
p[top] = dirichlet(0.);        // Zero pressure (atmospheric)
f[top] = neumann(0.);          // Zero gradient

/**
 * Bottom boundary (z = 0)
 * Wall boundary (pool bottom) - optional, usually far from action
 */
u.n[bottom] = dirichlet(0.);   // No-slip wall
u.t[bottom] = dirichlet(0.);   // No-slip wall
f[bottom] = neumann(0.);       // Zero gradient

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(int argc, char **argv) {

#if _MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &cpu_rank);
#endif

    // Parse runtime arguments and initialize parameters
    parse_runtime_args(argc, argv, &cfdbv);

    // Print simulation parameters (only from rank 0)
    if (cpu_rank == 0) {
        print_simulation_params(&cfdbv);
    }

    // Set domain size and origin
    size(cfdbv.domainsize);
    origin(0., 0.);  // y=0 (axis), x=0 (bottom)

    // Initialize grid with minimum refinement level
    init_grid(1 << cfdbv.minlevel);

    // Set fluid properties
    rho1 = cfdbv.rho1;  // Liquid (droplet) density
    rho2 = cfdbv.rho2;  // Gas (ambient) density
    mu1 = cfdbv.mu1;    // Liquid viscosity
    mu2 = cfdbv.mu2;    // Gas viscosity
    f.sigma = cfdbv.sigma;  // Surface tension

    // Set gravity in axial (x) direction
    // Physical gravity points downward (negative z)
    // In Basilisk coordinates: z → x, so gravity is -x direction
    G.x = -cfdbv.Bond;  // Gravity in axial direction
    G.y = 0.0;          // No radial gravity

    // Set timestep if specified
    if (cfdbv.timestep > 0) {
        DT = cfdbv.timestep;
    }

    // Run simulation
    run();

    return 0;
}

// =============================================================================
// EVENT: DEFAULTS - INTERFACE AND SOLVER SETUP
// =============================================================================

event defaults(i = 0) {
    // Set interface tracers
    tracers = {f, fb};

    if (cpu_rank == 0) {
        fprintf(stderr, "# Initialization: Setting up interface trackers\n");
        fprintf(stderr, "# Primary phase: f (droplet)\n");
        fprintf(stderr, "# Secondary phase: fb (bubble)\n");
    }
}

// =============================================================================
// EVENT: INIT - INITIAL CONDITIONS
// =============================================================================

event init(t = 0) {

    if (cpu_rank == 0) {
        fprintf(stderr, "\n# Event: init (t = 0)\n");
        fprintf(stderr, "# Initializing drop and bubble positions...\n");
    }

    // Calculate initial positions
    // Drop center: above pool surface by initial distance
    x0 = cfdbv.pooldepth + cfdbv.initialdis + 0.5 * cfdbv.diameter;
    y0 = 0.0;  // On axis (axisymmetric)

    // Bubble center (same as drop center initially)
    Bubtx0 = x0;
    Bubty0 = 0.0;

    if (cpu_rank == 0) {
        fprintf(stderr, "# Drop center: (r=%.3f, z=%.3f)\n", y0, x0);
        fprintf(stderr, "# Drop radius: %.3f\n", 0.5 * cfdbv.diameter);
        fprintf(stderr, "# Bubble diameter ratio: %.3f\n", cfdbv.bubblediameter);
        fprintf(stderr, "# Pool depth: %.3f\n", cfdbv.pooldepth);
    }

    // Refine mesh in regions of interest before initialization
    // This ensures proper resolution of initial interface
    refine(sq(x - x0) + sq(y - y0) < sq(0.5 * cfdbv.diameter + cfdbv.refine_gap) &&
           level < cfdbv.maxlevel);

    // Initialize droplet shape (spherical)
    // VOF fraction: inside droplet = 1, outside = 0
    // Level set: φ = -(r² - R²) where r is distance from center
    fraction(f, -(sq(x - x0) + sq(y - y0) - sq(0.5 * cfdbv.diameter)));

    // Initialize bubble if present (entrapped air bubble)
    if (cfdbv.bubblediameter > 0) {
        fraction(fb, -(sq(x - Bubtx0) + sq(y - Bubty0) -
                       sq(0.5 * cfdbv.bubblediameter * cfdbv.diameter)));
    } else {
        // No bubble
        foreach() {
            fb[] = 0.;
        }
    }

    // Set initial velocity field
    // Drop moving downward with impact velocity
    foreach() {
        if (f[] > 0.5) {
            u.x[] = -cfdbv.vel;  // Downward (negative x)
            u.y[] = 0.;          // No radial motion
        } else {
            u.x[] = 0.;
            u.y[] = 0.;
        }
    }

    boundary({f, fb, u.x, u.y});

    if (cpu_rank == 0) {
        fprintf(stderr, "# Initial conditions set successfully\n");
    }
}

// =============================================================================
// EVENT: INITFRACTION - ADDITIONAL INTERFACE INITIALIZATION
// =============================================================================

event initfraction(t = 0) {
    // Additional refinement around interfaces after initialization
    foreach() {
        if (f[] > 0.01 && f[] < 0.99) {
            refine(level < cfdbv.maxlevel);
        }
        if (fb[] > 0.01 && fb[] < 0.99) {
            refine(level < cfdbv.maxlevel);
        }
    }

    boundary({f, fb});
}

// =============================================================================
// EVENT: ADAPT - ADAPTIVE MESH REFINEMENT
// =============================================================================

event adapt(i++) {
    // Adapt based on VOF interface and velocity field
    // Refine where:
    //   - Interface is present (f or fb between 0 and 1)
    //   - Velocity gradients are large
    //   - Within specified gap around interface

    adapt_wavelet({f, fb, u.x, u.y},
                  (double[]){REFINE_VALUE_0, REFINE_VALUE_0,
                            REFINE_VALUE_1, REFINE_VALUE_1},
                  maxlevel = cfdbv.maxlevel,
                  minlevel = cfdbv.minlevel);
}

// =============================================================================
// EVENT: SHOWITERATION - PRINT ITERATION INFO
// =============================================================================

event showiteration(i += 10) {
    if (cpu_rank == 0) {
        fprintf(stderr, "# i=%d  t=%.4f  dt=%.6f  cells=%ld\n",
                i, t, dt, grid->tn);
    }
}

// =============================================================================
// EVENT: LOGFILES - VOLUME CONSERVATION AND INTERFACE TRACKING
// =============================================================================

event logfiles(i++) {

    // Calculate droplet volume
    double volume_drop = 0.;
    foreach(reduction(+:volume_drop)) {
        volume_drop += f[] * dv();
    }

    // Calculate bubble volume
    double volume_bubble = 0.;
    foreach(reduction(+:volume_bubble)) {
        volume_bubble += fb[] * dv();
    }

    // Theoretical initial volumes
    double V0_drop = (4./3.) * pi * cube(0.5 * cfdbv.diameter);
    double V0_bubble = (4./3.) * pi * cube(0.5 * cfdbv.bubblediameter * cfdbv.diameter);

    // Write volume conservation data (only rank 0)
    if (cpu_rank == 0) {
        if (i == 0) {
            fp_volume = fopen("volume_conservation.txt", "w");
            fprintf(fp_volume, "# Droplet Impact - Volume Conservation\n");
            fprintf(fp_volume, "# time  iter  V_drop  V_drop/V0  V_bubble  V_bubble/V0\n");
        }
        fprintf(fp_volume, "%.6e %d %.6e %.6f %.6e %.6f\n",
                t, i, volume_drop, volume_drop/V0_drop,
                volume_bubble, V0_bubble > 0 ? volume_bubble/V0_bubble : 0.);
        fflush(fp_volume);
    }

    // Track interface position on axis (leading and trailing)
    double z_min = HUGE, z_max = -HUGE;
    foreach() {
        if (y < 0.1 * cfdbv.diameter && f[] > 0.5) {  // Near axis
            if (x < z_min) z_min = x;
            if (x > z_max) z_max = x;
        }
    }

#if _MPI
    // Reduce across MPI ranks
    double z_min_global = z_min, z_max_global = z_max;
    MPI_Allreduce(&z_min, &z_min_global, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&z_max, &z_max_global, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    z_min = z_min_global;
    z_max = z_max_global;
#endif

    if (cpu_rank == 0) {
        if (i == 0) {
            fp_interface = fopen("interface_position.txt", "w");
            fprintf(fp_interface, "# Droplet Impact - Interface Position\n");
            fprintf(fp_interface, "# time  iter  z_leading  z_trailing  spread\n");
        }
        fprintf(fp_interface, "%.6e %d %.6e %.6e %.6e\n",
                t, i, z_min, z_max, z_max - z_min);
        fflush(fp_interface);
    }
}

// =============================================================================
// EVENT: OUTPUTFILES - PERIODIC DATA DUMP
// =============================================================================

event outputfiles(t += 0.1) {

    if (cpu_rank == 0) {
        fprintf(stderr, "# Writing snapshot at t = %.4f\n", t);
    }

    // Create output directory if needed
    static int first = 1;
    if (first && cpu_rank == 0) {
        system("mkdir -p intermediate");
        first = 0;
    }

    // Write snapshot
    char filename[256];
    sprintf(filename, "intermediate/snapshot-%04d-CPU%02d.gz", (int)(t*100), cpu_rank);

    FILE * fp = fopen(filename, "w");
    if (fp) {
        output_gfs(fp);
        fclose(fp);
    }

    // Write interface facets for visualization
    sprintf(filename, "intermediate/facets-%04d-CPU%02d.dat", (int)(t*100), cpu_rank);
    fp = fopen(filename, "w");
    if (fp) {
        output_facets(f, fp);
        fclose(fp);
    }
}

// =============================================================================
// EVENT: MOVIES - OPTIONAL VISUALIZATION
// =============================================================================

#if 0  // Enable by changing to #if 1
#include "view.h"

event movies(t += 0.01) {
    static FILE * fp = NULL;
    if (cpu_rank == 0) {
        if (fp == NULL) {
            fp = fopen("movie.ppm", "w");
        }

        view(width = 800, height = 800);
        clear();

        // Draw VOF interface
        draw_vof("f", lw = 2);

        // Draw bubble if present
        if (cfdbv.bubblediameter > 0) {
            draw_vof("fb", lw = 1.5);
        }

        // Draw cells
        cells();

        save(fp = fp);
    }
}

event end_movie(t = end) {
    if (cpu_rank == 0 && fp != NULL) {
        fclose(fp);
    }
}
#endif

// =============================================================================
// EVENT: END - FINAL OUTPUT AND CLEANUP
// =============================================================================

event end(t = cfdbv.endtime) {

    if (cpu_rank == 0) {
        fprintf(stderr, "\n");
        fprintf(stderr, "=======================================================\n");
        fprintf(stderr, "  Simulation completed at t = %.4f\n", t);
        fprintf(stderr, "=======================================================\n");

        // Write final summary
        char filename[256];
        sprintf(filename, "endofrun-CPU%02d.txt", cpu_rank);
        FILE * fp = fopen(filename, "w");
        if (fp) {
            fprintf(fp, "Simulation End Summary\n");
            fprintf(fp, "======================\n");
            fprintf(fp, "Final time: t = %.6f\n", t);
            fprintf(fp, "Total iterations: %d\n", i);
            fprintf(fp, "Reynolds: %.2f\n", cfdbv.Reynolds);
            fprintf(fp, "Weber: %.2f\n", cfdbv.Weber);
            fprintf(fp, "Froude: %.2f\n", cfdbv.Froude);
            fprintf(fp, "Bond: %.2f\n", cfdbv.Bond);
            fprintf(fp, "Max level: %d\n", cfdbv.maxlevel);
            fprintf(fp, "Min level: %d\n", cfdbv.minlevel);
            fclose(fp);
        }

        // Close log files
        if (fp_volume) fclose(fp_volume);
        if (fp_interface) fclose(fp_interface);

        // Write final snapshot
        sprintf(filename, "lastfile-CPU%02d.gz", cpu_rank);
        fp = fopen(filename, "w");
        if (fp) {
            output_gfs(fp);
            fclose(fp);
        }

        fprintf(stderr, "Output files written to current directory and intermediate/\n");
        fprintf(stderr, "\n");
    }
}

// =============================================================================
// EVENT: DURATION LOG - PERFORMANCE TRACKING
// =============================================================================

event duration_log(i += 100) {
    static double t_start = 0;
    static int first = 1;

    if (first) {
        t_start = clock();
        first = 0;

        if (cpu_rank == 0) {
            char filename[256];
            sprintf(filename, "duration-CPU%02d.plt", cpu_rank);
            fp_duration = fopen(filename, "w");
            fprintf(fp_duration, "# iter  time  wallclock(s)  cells  grids\n");
        }
    }

    if (cpu_rank == 0 && fp_duration) {
        double elapsed = (clock() - t_start) / CLOCKS_PER_SEC;
        fprintf(fp_duration, "%d %.6e %.3f %ld %d\n",
                i, t, elapsed, grid->tn, grid->depth);
        fflush(fp_duration);
    }
}

event cleanup(t = end) {
    if (cpu_rank == 0 && fp_duration) {
        fclose(fp_duration);
    }
}
