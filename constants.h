/**
 * constants.h - Physical parameters, nondimensional numbers, and macros
 *
 * This header defines the simulation parameters for axisymmetric droplet impact
 * simulations. It supports three modes:
 *   - 'd': Dimensional simulation using SI units
 *   - 'n': Nondimensional mode with fixed Re, We, Fr
 *   - 'e': Experimental mode importing lab parameters
 *
 * Usage: Include this file in the main simulation (Bdropimpact.c)
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <math.h>

// =============================================================================
// SIMULATION MODE SELECTION
// =============================================================================
// Define one of these before including this header, or use default
#ifndef SIMMODE
#define SIMMODE 'n'  // Default: nondimensional mode
#endif

// =============================================================================
// DIMENSIONAL PHYSICAL CONSTANTS (SI units) - Used for mode 'd' or 'e'
// =============================================================================
#ifdef DIMENSIONAL_MODE

// Fluid properties
#define RHO_L          1000.0      // Liquid density [kg/m³]
#define RHO_G          1.2         // Gas density [kg/m³]
#define MU_L           0.001       // Liquid dynamic viscosity [Pa·s]
#define MU_G           1.8e-5      // Gas dynamic viscosity [Pa·s]
#define SIGMA          0.072       // Surface tension [N/m]
#define GRAVITY        9.81        // Gravitational acceleration [m/s²]

// Droplet parameters
#define DROP_DIAMETER  0.003       // Droplet diameter [m] (3 mm)
#define DROP_VELOCITY  1.0         // Impact velocity [m/s]

// Bubble parameters
#define BUBBLE_RATIO   0.3         // Bubble diameter ratio (d_bubble/d_drop)

// Geometry
#define POOL_DEPTH     0.01        // Pool depth [m] (10 mm)
#define INITIAL_DIS    0.001       // Initial distance above pool [m]

#endif

// =============================================================================
// NONDIMENSIONAL NUMBERS - Used for mode 'n'
// =============================================================================
#ifndef DIMENSIONAL_MODE

// Default nondimensional parameters (can be overridden by command-line args)
#ifndef REYNOLDS
#define REYNOLDS       2448.0      // Re = ρ_L * U * D / μ_L
#endif

#ifndef WEBER
#define WEBER          873.0       // We = ρ_L * U² * D / σ
#endif

#ifndef FROUDE
#define FROUDE         180.0       // Fr = U / sqrt(g * D)
#endif

#ifndef BOND
#define BOND           38.7        // Bo = ρ_L * g * D² / σ
#endif

// Density and viscosity ratios
#ifndef RHO_RATIO
#define RHO_RATIO      0.0012      // ρ_g / ρ_L (air/water ~ 0.0012)
#endif

#ifndef MU_RATIO
#define MU_RATIO       0.018       // μ_g / μ_L (air/water ~ 0.018)
#endif

// Geometry (nondimensional, normalized by drop diameter D)
#define D_DROP_ND      1.0         // Drop diameter (reference scale)
#define R_DROP_ND      0.5         // Drop radius

#ifndef BUBBLE_RATIO_ND
#define BUBBLE_RATIO_ND 0.3        // Bubble/drop diameter ratio
#endif

#ifndef POOL_DEPTH_ND
#define POOL_DEPTH_ND  3.33        // Pool depth / D
#endif

#ifndef INITIAL_DIS_ND
#define INITIAL_DIS_ND 0.33        // Initial gap / D
#endif

#endif

// =============================================================================
// SIMULATION PARAMETERS
// =============================================================================

// Domain size
#ifndef DOMAIN_SIZE
#define DOMAIN_SIZE    15.52       // Domain size (nondim) or [m] (dimensional)
#endif

// Grid refinement levels
#ifndef MAXLEVEL
#define MAXLEVEL       12          // Maximum AMR level
#endif

#ifndef MINLEVEL
#define MINLEVEL       4           // Minimum AMR level
#endif

// Refinement thresholds for adaptive mesh refinement
#define REFINE_VALUE_0 0.01        // VOF interface refinement threshold
#define REFINE_VALUE_1 0.01        // Velocity refinement threshold
#define REFINE_VALUE_2 0.01        // Secondary threshold

// Refinement gap around interfaces
#ifndef REFINE_GAP
#define REFINE_GAP     0.1         // Distance around interface to refine
#endif

// Time parameters
#ifndef TIMESTEP
#define TIMESTEP       0.0         // Timestep (0 = automatic)
#endif

#ifndef ENDTIME
#define ENDTIME        10.0        // Simulation end time
#endif

// Contact angle (degrees)
#ifndef THETA_CONTACT
#define THETA_CONTACT  90.0        // Contact angle at walls
#endif

// =============================================================================
// RUNTIME PARAMETER STRUCTURE
// =============================================================================
/**
 * Structure to hold computed fluid and boundary values
 * These are calculated from nondimensional numbers or dimensional values
 */
typedef struct {
    double rho1, rho2;     // Fluid densities
    double mu1, mu2;       // Dynamic viscosities
    double sigma;          // Surface tension coefficient
    double vel;            // Impact velocity
    double diameter;       // Drop diameter
    double Bond;           // Bond number
    double Reynolds;       // Reynolds number
    double Weber;          // Weber number
    double Froude;         // Froude number
    int maxlevel;          // Max refinement level
    int minlevel;          // Min refinement level
    double refine_gap;     // Refinement gap
    double pooldepth;      // Pool depth
    double initialdis;     // Initial distance
    double bubblediameter; // Bubble diameter ratio
    double domainsize;     // Domain size
    double timestep;       // Timestep override
    double endtime;        // End time
} SimParams;

// =============================================================================
// COMPUTATIONAL MACROS
// =============================================================================

// Square function
#define sq(x) ((x)*(x))

// Cube function
#define cube(x) ((x)*(x)*(x))

// Minimum/Maximum
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

// Clamp value between min and max
#define clamp(x, xmin, xmax) (min(max((x), (xmin)), (xmax)))

// =============================================================================
// NONDIMENSIONALIZATION FUNCTIONS
// =============================================================================

/**
 * Calculate nondimensional fluid properties from dimensionless numbers
 * This converts Re, We, Fr, Bo into ρ*, μ*, σ* for the solver
 */
static inline void compute_nondim_properties(SimParams *p) {
    // In nondimensional form with U*=1, D*=1:
    // Re = ρ_L * U * D / μ_L = 1 / μ*_L
    // We = ρ_L * U² * D / σ = 1 / σ*
    // Bo = ρ_L * g * D² / σ = g* / σ*
    // Fr = U / sqrt(g * D) = 1 / sqrt(g*)

    p->rho1 = 1.0;                           // Reference density (liquid)
    p->rho2 = RHO_RATIO;                     // Gas density ratio
    p->mu1 = 1.0 / p->Reynolds;              // Liquid viscosity
    p->mu2 = MU_RATIO / p->Reynolds;         // Gas viscosity
    p->sigma = 1.0 / p->Weber;               // Surface tension
    p->vel = 1.0;                            // Reference velocity
    p->diameter = 1.0;                       // Reference length
}

/**
 * Calculate dimensional properties
 * Computes Re, We, Fr, Bo from dimensional values
 */
static inline void compute_dimensional_properties(SimParams *p) {
#ifdef DIMENSIONAL_MODE
    p->Reynolds = RHO_L * DROP_VELOCITY * DROP_DIAMETER / MU_L;
    p->Weber = RHO_L * sq(DROP_VELOCITY) * DROP_DIAMETER / SIGMA;
    p->Froude = DROP_VELOCITY / sqrt(GRAVITY * DROP_DIAMETER);
    p->Bond = RHO_L * GRAVITY * sq(DROP_DIAMETER) / SIGMA;

    p->rho1 = RHO_L;
    p->rho2 = RHO_G;
    p->mu1 = MU_L;
    p->mu2 = MU_G;
    p->sigma = SIGMA;
    p->vel = DROP_VELOCITY;
    p->diameter = DROP_DIAMETER;
#endif
}

/**
 * Parse command-line arguments for runtime parameter override
 * Supports:
 *   R### → Reynolds number
 *   W### → Weber number
 *   F### → Froude number
 *   H### → Pool depth
 *   xN   → Max level
 *   nN   → Min level
 *   ts#  → Timestep
 *   te#  → End time
 */
static inline void parse_runtime_args(int argc, char **argv, SimParams *p) {
    // Set defaults
    p->Reynolds = REYNOLDS;
    p->Weber = WEBER;
    p->Froude = FROUDE;
    p->Bond = BOND;
    p->maxlevel = MAXLEVEL;
    p->minlevel = MINLEVEL;
    p->refine_gap = REFINE_GAP;
    p->pooldepth = POOL_DEPTH_ND;
    p->initialdis = INITIAL_DIS_ND;
    p->bubblediameter = BUBBLE_RATIO_ND;
    p->domainsize = DOMAIN_SIZE;
    p->timestep = TIMESTEP;
    p->endtime = ENDTIME;

    // Parse command-line overrides
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == 'R') {
            p->Reynolds = atof(&argv[i][1]);
        }
        else if (argv[i][0] == 'W') {
            p->Weber = atof(&argv[i][1]);
        }
        else if (argv[i][0] == 'F') {
            p->Froude = atof(&argv[i][1]);
        }
        else if (argv[i][0] == 'H') {
            p->pooldepth = atof(&argv[i][1]);
        }
        else if (argv[i][0] == 'x') {
            p->maxlevel = atoi(&argv[i][1]);
        }
        else if (argv[i][0] == 'n') {
            p->minlevel = atoi(&argv[i][1]);
        }
        else if (strncmp(argv[i], "ts", 2) == 0) {
            p->timestep = atof(&argv[i][2]);
        }
        else if (strncmp(argv[i], "te", 2) == 0) {
            p->endtime = atof(&argv[i][2]);
        }
    }

    // Recompute Bond number from Froude and Weber
    // Bo = We / Fr² (from Bo = ρ g D² / σ and We = ρ U² D / σ, Fr = U / √(gD))
    p->Bond = p->Weber / sq(p->Froude);

    // Compute fluid properties
    compute_nondim_properties(p);
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Print simulation parameters summary
 */
static inline void print_simulation_params(SimParams *p) {
    fprintf(stderr, "\n");
    fprintf(stderr, "=======================================================\n");
    fprintf(stderr, "  Basilisk Droplet Impact Simulation Parameters\n");
    fprintf(stderr, "=======================================================\n");
    fprintf(stderr, "Nondimensional Numbers:\n");
    fprintf(stderr, "  Reynolds (Re)  = %.2f\n", p->Reynolds);
    fprintf(stderr, "  Weber (We)     = %.2f\n", p->Weber);
    fprintf(stderr, "  Froude (Fr)    = %.2f\n", p->Froude);
    fprintf(stderr, "  Bond (Bo)      = %.2f\n", p->Bond);
    fprintf(stderr, "  ρ₂/ρ₁          = %.4f\n", p->rho2/p->rho1);
    fprintf(stderr, "  μ₂/μ₁          = %.4f\n", p->mu2/p->mu1);
    fprintf(stderr, "\nFluid Properties (nondim):\n");
    fprintf(stderr, "  ρ₁ = %.6f,  ρ₂ = %.6f\n", p->rho1, p->rho2);
    fprintf(stderr, "  μ₁ = %.6f,  μ₂ = %.6f\n", p->mu1, p->mu2);
    fprintf(stderr, "  σ  = %.6f\n", p->sigma);
    fprintf(stderr, "\nGeometry (nondim):\n");
    fprintf(stderr, "  Drop diameter     = %.3f\n", p->diameter);
    fprintf(stderr, "  Bubble/drop ratio = %.3f\n", p->bubblediameter);
    fprintf(stderr, "  Pool depth        = %.3f\n", p->pooldepth);
    fprintf(stderr, "  Initial gap       = %.3f\n", p->initialdis);
    fprintf(stderr, "  Domain size       = %.3f\n", p->domainsize);
    fprintf(stderr, "\nGrid Refinement:\n");
    fprintf(stderr, "  Max level  = %d (Δx_min = %.6f)\n",
            p->maxlevel, p->domainsize / (1 << p->maxlevel));
    fprintf(stderr, "  Min level  = %d (Δx_max = %.6f)\n",
            p->minlevel, p->domainsize / (1 << p->minlevel));
    fprintf(stderr, "  Refine gap = %.3f\n", p->refine_gap);
    fprintf(stderr, "\nTime Integration:\n");
    fprintf(stderr, "  Timestep   = %s\n",
            p->timestep > 0 ? "fixed" : "automatic (CFL)");
    if (p->timestep > 0)
        fprintf(stderr, "  dt         = %.6f\n", p->timestep);
    fprintf(stderr, "  End time   = %.3f\n", p->endtime);
    fprintf(stderr, "=======================================================\n");
    fprintf(stderr, "\n");
}

/**
 * Calculate contact time estimate
 * Based on dimensional analysis: t_c ~ D/U
 * In nondimensional form: t_c* ~ 1
 */
static inline double estimate_contact_time(SimParams *p) {
    // Simple estimate: contact time ~ drop diameter / impact velocity
    // In nondimensional units: t_c* ~ D* / U* = 1 / 1 = 1
    return p->diameter / p->vel;
}

#endif // CONSTANTS_H
