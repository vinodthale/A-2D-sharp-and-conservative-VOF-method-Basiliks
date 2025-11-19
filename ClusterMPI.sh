#!/bin/bash
#
# ClusterMPI.sh - SLURM job submission script for Basilisk droplet impact simulation
#
# This script compiles and runs the Bdropimpact.c simulation on a SLURM cluster
# with MPI parallelization.
#
# Usage:
#   sbatch ClusterMPI.sh [simulation arguments]
#
# Example:
#   sbatch ClusterMPI.sh R2448 W873 F180 x12
#
# Author: Basilisk MPI execution framework
# Date: 2025
#

#SBATCH --job-name=basilisk-drop-impact
#SBATCH --output=slurm.out
#SBATCH --error=slurm.err

# =============================================================================
# SLURM RESOURCE ALLOCATION
# =============================================================================

#SBATCH --nodes=1                    # Number of nodes
#SBATCH --ntasks-per-node=32         # MPI tasks per node (adjust based on cluster)
#SBATCH --cpus-per-task=1            # CPU cores per task
#SBATCH --threads-per-core=1         # Threads per core (disable hyperthreading)
#SBATCH --time=48:00:00              # Maximum walltime (48 hours)
#SBATCH --partition=compute          # Partition/queue name (adjust for your cluster)
#SBATCH --mem=64G                    # Total memory per node

# Optional: Constraint for specific CPU architecture
##SBATCH --constraint=haswell

# Optional: Email notifications
##SBATCH --mail-type=BEGIN,END,FAIL
##SBATCH --mail-user=your.email@domain.com

# =============================================================================
# ENVIRONMENT SETUP
# =============================================================================

echo "======================================================="
echo "  Basilisk Droplet Impact Simulation - MPI Execution"
echo "======================================================="
echo ""
echo "Job started at: $(date)"
echo "Job ID: $SLURM_JOB_ID"
echo "Running on node(s): $SLURM_JOB_NODELIST"
echo "Number of nodes: $SLURM_JOB_NUM_NODES"
echo "Number of tasks: $SLURM_NTASKS"
echo "Working directory: $(pwd)"
echo ""

# Load required modules (adjust based on your cluster)
# Uncomment and modify as needed:
# module purge
# module load gcc/11.2.0
# module load openmpi/4.1.2
# module load basilisk/latest

# Set environment variables for Basilisk
# Adjust BASILISK path to your installation
export BASILISK=${BASILISK:-$HOME/basilisk}
export PATH=$BASILISK:$PATH

# Check if Basilisk is available
if ! command -v qcc &> /dev/null; then
    echo "ERROR: Basilisk qcc compiler not found!"
    echo "Please set BASILISK environment variable or load Basilisk module"
    exit 1
fi

echo "Basilisk installation: $BASILISK"
echo "qcc location: $(which qcc)"
echo ""

# MPI environment settings
export OMPI_MCA_btl_openib_allow_ib=1     # Enable InfiniBand (if available)
export OMPI_MCA_mpi_warn_on_fork=0        # Suppress fork warnings

# =============================================================================
# SIMULATION PARAMETERS
# =============================================================================

# Parse command-line arguments or use defaults
ARGS="${@:-R2448 W873 F180 x12 n4 te10}"

echo "Simulation arguments: $ARGS"
echo ""

# =============================================================================
# COMPILATION
# =============================================================================

echo "======================================================="
echo "  COMPILATION PHASE"
echo "======================================================="
echo ""

# Clean previous builds
if [ -f Bdropimpact ]; then
    echo "Removing previous executable..."
    rm -f Bdropimpact
fi

if [ -f _Bdropimpact.c ]; then
    echo "Removing previous intermediate files..."
    rm -f _Bdropimpact.c
fi

# Step 1: Generate intermediate C source with MPI support
echo "Step 1: Generating MPI source code with qcc..."
qcc -source -D_MPI=1 Bdropimpact.c

if [ $? -ne 0 ]; then
    echo "ERROR: qcc source generation failed!"
    exit 1
fi

if [ ! -f _Bdropimpact.c ]; then
    echo "ERROR: Intermediate file _Bdropimpact.c not created!"
    exit 1
fi

echo "  -> Generated _Bdropimpact.c"
echo ""

# Step 2: Compile with mpicc
echo "Step 2: Compiling with mpicc..."

# Compiler flags:
#   -O2                : Optimization level 2
#   -Wall              : Enable all warnings
#   -std=c99           : Use C99 standard
#   -D_MPI=1           : Enable MPI parallelization
#   -D_FORTIFY_SOURCE=0: Disable fortify source (some clusters require this)
#   -lm                : Link math library

mpicc -O2 -Wall -std=c99 -D_MPI=1 -D_FORTIFY_SOURCE=0 \
      _Bdropimpact.c -o Bdropimpact -lm

if [ $? -ne 0 ]; then
    echo "ERROR: mpicc compilation failed!"
    exit 1
fi

if [ ! -f Bdropimpact ]; then
    echo "ERROR: Executable Bdropimpact not created!"
    exit 1
fi

echo "  -> Compilation successful: Bdropimpact"
echo ""

# Display executable info
ls -lh Bdropimpact
echo ""

# =============================================================================
# SETUP OUTPUT DIRECTORIES
# =============================================================================

echo "======================================================="
echo "  SETUP"
echo "======================================================="
echo ""

# Create output directory
if [ ! -d intermediate ]; then
    echo "Creating output directory: intermediate/"
    mkdir -p intermediate
fi

# Clean old output files (optional - comment out to preserve)
# echo "Cleaning old output files..."
# rm -f intermediate/*
# rm -f *.txt *.dat *.plt *.gz *.ppm

echo "Output directory ready"
echo ""

# =============================================================================
# EXECUTION
# =============================================================================

echo "======================================================="
echo "  EXECUTION PHASE"
echo "======================================================="
echo ""
echo "Starting simulation at: $(date)"
echo "Number of MPI tasks: $SLURM_NTASKS"
echo "Arguments: $ARGS"
echo ""

# Run the simulation with srun
# MPI launch flags:
#   --mpi=pmi2         : Use PMI2 for process management
#   -K1                : Enable job step signal forwarding
#   --resv-ports       : Reserve communication ports
#   -n $SLURM_NTASKS   : Number of MPI tasks

time srun --mpi=pmi2 -K1 --resv-ports -n $SLURM_NTASKS \
     ./Bdropimpact $ARGS

# Capture exit status
EXIT_CODE=$?

echo ""
echo "Simulation completed at: $(date)"
echo "Exit code: $EXIT_CODE"
echo ""

# =============================================================================
# POST-PROCESSING AND CLEANUP
# =============================================================================

echo "======================================================="
echo "  POST-PROCESSING"
echo "======================================================="
echo ""

if [ $EXIT_CODE -eq 0 ]; then
    echo "Simulation completed successfully!"
    echo ""
    echo "Output files generated:"
    echo "  - volume_conservation.txt   : Volume conservation log"
    echo "  - interface_position.txt    : Interface tracking"
    echo "  - duration-CPU*.plt         : Performance data"
    echo "  - endofrun-CPU*.txt         : Final summary"
    echo "  - lastfile-CPU*.gz          : Final state"
    echo "  - intermediate/snapshot-*.gz: Intermediate snapshots"
    echo "  - intermediate/facets-*.dat : Interface facets"
    echo ""

    # Count output files
    NUM_SNAPSHOTS=$(ls intermediate/snapshot-*.gz 2>/dev/null | wc -l)
    NUM_FACETS=$(ls intermediate/facets-*.dat 2>/dev/null | wc -l)

    echo "Generated $NUM_SNAPSHOTS snapshots and $NUM_FACETS facet files"
    echo ""

    # Display file sizes
    echo "Disk usage:"
    du -sh intermediate/ 2>/dev/null || echo "  intermediate/ directory empty or not found"
    du -sh *.txt *.dat *.plt *.gz 2>/dev/null | head -10
    echo ""

else
    echo "WARNING: Simulation exited with error code $EXIT_CODE"
    echo "Check slurm.err for error messages"
    echo ""
fi

# =============================================================================
# OPTIONAL: ARCHIVE RESULTS
# =============================================================================

# Uncomment to automatically archive results
# ARCHIVE_NAME="droplet_impact_${SLURM_JOB_ID}_$(date +%Y%m%d_%H%M%S).tar.gz"
# echo "Archiving results to $ARCHIVE_NAME..."
# tar czf $ARCHIVE_NAME intermediate/ *.txt *.dat *.plt endofrun-*.txt lastfile-*.gz
# echo "Archive created: $(ls -lh $ARCHIVE_NAME)"
# echo ""

# =============================================================================
# SUMMARY
# =============================================================================

echo "======================================================="
echo "  JOB SUMMARY"
echo "======================================================="
echo "Job ID:          $SLURM_JOB_ID"
echo "Exit code:       $EXIT_CODE"
echo "Nodes used:      $SLURM_JOB_NUM_NODES"
echo "Tasks:           $SLURM_NTASKS"
echo "Started at:      $(head -1 slurm.out | grep 'Job started' || echo 'N/A')"
echo "Completed at:    $(date)"
echo "======================================================="
echo ""

# Return the simulation exit code
exit $EXIT_CODE
