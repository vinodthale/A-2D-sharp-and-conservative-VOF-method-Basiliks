# Basilisk C Configuration Guide

This guide covers configuration options, compiler flags, environment variables, and customization for Basilisk C.

## Table of Contents

- [Environment Variables](#environment-variables)
- [Compiler Configuration](#compiler-configuration)
- [QCC Compiler Wrapper](#qcc-compiler-wrapper)
- [Compilation Flags](#compilation-flags)
- [MPI Configuration](#mpi-configuration)
- [Performance Tuning](#performance-tuning)
- [Project-Specific Configuration](#project-specific-configuration)

## Environment Variables

### Required Variables

#### BASILISK

Points to the Basilisk installation directory.

```bash
export BASILISK=$HOME/basilisk
```

**Usage**:
- Required for `qcc` to find Basilisk headers and libraries
- Used by scripts to locate Basilisk tools

**Verification**:
```bash
echo $BASILISK
# Should output: /home/username/basilisk
```

#### PATH

Add Basilisk to your PATH to use `qcc` from anywhere.

```bash
export PATH=$PATH:$BASILISK
```

**Verification**:
```bash
which qcc
# Should output: /home/username/basilisk/qcc
```

### Optional Variables

#### BASILISK_EXAMPLES

Path to Basilisk example files (useful for learning).

```bash
export BASILISK_EXAMPLES=$BASILISK/src/examples
```

#### CC

Override the default C compiler.

```bash
export CC=gcc-11  # Use specific GCC version
export CC=clang   # Use Clang instead of GCC
```

#### CFLAGS

Additional compiler flags for all compilations.

```bash
export CFLAGS="-O3 -march=native"  # Aggressive optimization
```

### Shell Configuration

Add to `~/.bashrc` (Linux) or `~/.zshrc` (macOS):

```bash
# Basilisk C Configuration
export BASILISK=$HOME/basilisk
export PATH=$PATH:$BASILISK
export BASILISK_EXAMPLES=$BASILISK/src/examples

# Optional: Compiler preferences
export CC=gcc
export CFLAGS="-O2 -Wall"
```

Apply changes:
```bash
source ~/.bashrc  # or ~/.zshrc
```

## Compiler Configuration

### Configuration Files

Basilisk uses configuration files in `$BASILISK/src/`:

| File | Purpose |
|------|---------|
| `config.gcc` | Standard GCC (Linux default) |
| `config.osx` | macOS with Clang |
| `config.icc` | Intel C Compiler |
| `config.mpi` | MPI parallelization |
| `config.nvcc` | NVIDIA CUDA (experimental) |

### Active Configuration

The active config is a symbolic link:

```bash
cd $BASILISK/src
ls -l config
# config -> config.gcc
```

### Switching Configurations

```bash
cd $BASILISK/src

# Use MPI configuration
ln -sf config.mpi config

# Rebuild Basilisk
make clean
make
```

### Config File Structure

Example `config.gcc`:

```makefile
# Compiler
CC = gcc

# Compiler flags
CFLAGS = -O2 -Wall

# Linker flags
LDFLAGS = -lm

# Enable MPI (0 or 1)
OPENMPI = 0

# Enable CADNA (numerical debugging)
CADNA = 0

# OpenGL visualization
FB_LIBS = -lfb_osmesa -lOSMesa -lGLU
```

### Custom Configuration

Create a custom config:

```bash
cd $BASILISK/src
cp config.gcc config.custom
nano config.custom
```

Modify as needed:

```makefile
CC = gcc-11
CFLAGS = -O3 -march=native -Wall -fopenmp
LDFLAGS = -lm -fopenmp
OPENMPI = 0
```

Activate:

```bash
ln -sf config.custom config
make clean
make
```

## QCC Compiler Wrapper

### What is QCC?

`qcc` is Basilisk's compiler wrapper that:
1. Preprocesses `.c` files using AWK
2. Expands Basilisk-specific syntax
3. Handles `#include` for Basilisk headers
4. Calls the underlying C compiler

### Basic Usage

```bash
qcc [OPTIONS] -o output input.c [LIBRARIES]
```

### Common QCC Options

```bash
# Standard compilation
qcc -o program program.c -lm

# With optimization
qcc -O2 -o program program.c -lm

# With debugging symbols
qcc -g -o program program.c -lm

# Enable all warnings
qcc -Wall -o program program.c -lm

# Define preprocessor macro
qcc -DMAXLEVEL=10 -o program program.c -lm

# Multiple macros
qcc -DMAXLEVEL=10 -DDEBUG=1 -o program program.c -lm
```

### QCC-Specific Flags

```bash
# Show preprocessed output (debugging)
qcc -E program.c > program.i

# Keep intermediate files
qcc -save-temps -o program program.c -lm

# Verbose output
qcc -v -o program program.c -lm

# Show command that would run
qcc -### -o program program.c -lm
```

## Compilation Flags

### Optimization Levels

| Flag | Description | Use Case |
|------|-------------|----------|
| `-O0` | No optimization | Debugging |
| `-O1` | Basic optimization | Development |
| `-O2` | Recommended optimization | Production (default) |
| `-O3` | Aggressive optimization | Performance-critical |
| `-Ofast` | `-O3` + fast-math | Maximum speed (less precise) |

**Recommendation**: Use `-O2` for most cases, `-O3` for production runs.

### Architecture-Specific Optimization

```bash
# Optimize for current CPU
qcc -O3 -march=native -o program program.c -lm

# Specific architecture
qcc -O3 -march=haswell -o program program.c -lm
qcc -O3 -march=skylake -o program program.c -lm
```

### Warning Flags

```bash
# All standard warnings
qcc -Wall -o program program.c -lm

# Extra warnings
qcc -Wall -Wextra -o program program.c -lm

# Treat warnings as errors
qcc -Wall -Werror -o program program.c -lm
```

### Debugging Flags

```bash
# Debug symbols
qcc -g -o program program.c -lm

# Debug symbols + no optimization
qcc -g -O0 -o program program.c -lm

# Enable Basilisk debug output
qcc -DDEBUG -o program program.c -lm

# Memory debugging (variable trashing)
qcc -DTRASH=1 -o program program.c -lm
```

### Math Library

Always link with `-lm`:

```bash
qcc -o program program.c -lm
```

## MPI Configuration

### Enabling MPI

**Method 1: MPI config file**

```bash
cd $BASILISK/src
ln -sf config.mpi config
make clean
make
```

**Method 2: Define at compile time**

```bash
qcc -D_MPI=1 -o program program.c -lm
```

### MPI Compilation

```bash
# Compile with MPI
qcc -Wall -D_MPI=1 -o program program.c -lm

# Run with MPI (4 processes)
mpirun -np 4 ./program

# Run with MPI (8 processes)
mpirun -np 8 ./program 2> log
```

### MPI-Specific Flags

```bash
# MPI with debugging
qcc -g -D_MPI=1 -o program program.c -lm

# MPI with optimization
qcc -O3 -D_MPI=1 -o program program.c -lm
```

### MPI Environment Variables

```bash
# Limit threads per process
export OMP_NUM_THREADS=1

# MPI debugging
export OMPI_MCA_mpi_show_mca_params=all
```

## Performance Tuning

### Adaptive Mesh Refinement

Set maximum refinement level at compile time:

```bash
qcc -DMAXLEVEL=10 -o program program.c -lm
```

Higher `MAXLEVEL` = finer resolution but more memory/CPU:

| MAXLEVEL | Grid Cells | Memory | Use Case |
|----------|-----------|--------|----------|
| 7 | 128 x 128 | ~10 MB | Quick tests |
| 8 | 256 x 256 | ~40 MB | Development |
| 9 | 512 x 512 | ~150 MB | Production |
| 10 | 1024 x 1024 | ~600 MB | High resolution |
| 11 | 2048 x 2048 | ~2.4 GB | Very high resolution |

### Compiler Optimization Strategy

**Development:**
```bash
qcc -O0 -g -Wall -o program program.c -lm
```

**Testing:**
```bash
qcc -O2 -Wall -o program program.c -lm
```

**Production:**
```bash
qcc -O3 -march=native -DNDEBUG -o program program.c -lm
```

**Production with MPI:**
```bash
qcc -O3 -march=native -D_MPI=1 -DNDEBUG -o program program.c -lm
mpirun -np 8 ./program
```

### Memory Optimization

```bash
# Reduce memory usage (trade speed for memory)
qcc -DMAXLEVEL=9 -Os -o program program.c -lm

# Enable variable trashing to catch bugs
qcc -DTRASH=1 -o program program.c -lm
```

## Project-Specific Configuration

### This Project's Standard Compilation

For the VOF simulations in this repository:

```bash
# Standard compilation
qcc -O2 -Wall -o circle-droplet circle-droplet.c -lm
qcc -O2 -Wall -o droplet-impact-orifice droplet-impact-orifice.c -lm

# High resolution
qcc -O3 -DMAXLEVEL=10 -o droplet-impact-orifice droplet-impact-orifice.c -lm

# With debugging
qcc -g -Wall -o circle-droplet circle-droplet.c -lm
```

### Convenience Scripts

The project includes compilation scripts:

#### compile-droplet-impact.sh

```bash
#!/bin/bash
qcc -O2 -Wall -o droplet-impact-orifice droplet-impact-orifice.c -lm
qcc -O2 -Wall -o droplet-impact-orifice-nondim droplet-impact-orifice-nondim.c -lm
```

Usage:
```bash
./compile-droplet-impact.sh
```

### Makefile for This Project

Create a `Makefile` for easier compilation:

```makefile
# Compiler
QCC = qcc

# Flags
CFLAGS = -O2 -Wall
LDFLAGS = -lm

# Targets
TARGETS = circle-droplet \
          droplet-impact-orifice \
          droplet-impact-orifice-nondim \
          droplet-impact-sharp-orifice \
          droplet-impact-sharp-orifice-nondim \
          droplet-impact-round-orifice

# Default target
all: $(TARGETS)

# Generic rule
%: %.c
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean
clean:
	rm -f $(TARGETS) *.o *~ log

# High resolution build
high-res:
	$(QCC) $(CFLAGS) -DMAXLEVEL=10 -o droplet-impact-orifice droplet-impact-orifice.c $(LDFLAGS)

# Debug build
debug:
	$(QCC) -g -Wall -o droplet-impact-orifice droplet-impact-orifice.c $(LDFLAGS)

.PHONY: all clean high-res debug
```

Usage:
```bash
make              # Build all
make clean        # Clean up
make high-res     # High resolution build
make debug        # Debug build
```

### Recommended Workflow

**1. Development/Testing:**
```bash
# Compile with moderate optimization
qcc -O2 -Wall -o program program.c -lm

# Run with lower resolution
./program 2> log
```

**2. Production Runs:**
```bash
# Compile with full optimization
qcc -O3 -march=native -DMAXLEVEL=10 -o program program.c -lm

# Run
./program 2> log &
```

**3. Parallel Production:**
```bash
# Compile with MPI
qcc -O3 -march=native -D_MPI=1 -DMAXLEVEL=10 -o program program.c -lm

# Run on 8 cores
mpirun -np 8 ./program 2> log &
```

## Environment Setup Script

Create `basilisk-env.sh` for easy environment loading:

```bash
#!/bin/bash
# Basilisk C environment setup

export BASILISK=$HOME/basilisk
export PATH=$PATH:$BASILISK
export BASILISK_EXAMPLES=$BASILISK/src/examples

# Compiler settings
export CC=gcc
export CFLAGS="-O2 -Wall"

# MPI settings (if needed)
# export OMP_NUM_THREADS=1

echo "Basilisk environment loaded"
echo "BASILISK: $BASILISK"
echo "qcc: $(which qcc)"
```

Usage:
```bash
source basilisk-env.sh
```

## Troubleshooting

### Issue: Wrong compiler used

**Check:**
```bash
qcc -v program.c
```

**Fix:**
```bash
export CC=gcc
# or edit $BASILISK/src/config
```

### Issue: Optimization too aggressive

**Symptoms**: Wrong results, crashes

**Fix**: Reduce optimization
```bash
qcc -O2 -o program program.c -lm  # Instead of -O3
```

### Issue: Out of memory

**Fix**: Reduce MAXLEVEL or use MPI
```bash
qcc -DMAXLEVEL=8 -o program program.c -lm  # Reduce from 10 to 8
```

### Issue: Slow compilation

**Fix**: Check if debug flags are enabled
```bash
# Remove -g if not debugging
qcc -O2 -o program program.c -lm
```

## Summary

### Essential Commands

```bash
# Standard compilation
qcc -O2 -Wall -o program program.c -lm

# Production compilation
qcc -O3 -march=native -DMAXLEVEL=10 -o program program.c -lm

# MPI compilation
qcc -O3 -D_MPI=1 -o program program.c -lm

# Debug compilation
qcc -g -Wall -o program program.c -lm
```

### Key Environment Variables

```bash
export BASILISK=$HOME/basilisk
export PATH=$PATH:$BASILISK
```

### Configuration Files

- `$BASILISK/src/config` - Active configuration
- `~/.bashrc` or `~/.zshrc` - Environment variables

---

**See Also:**
- [BASILISK_INSTALL.md](BASILISK_INSTALL.md) - Installation guide
- [README.md](README.md) - Project overview
- Official Basilisk documentation: http://basilisk.fr/
