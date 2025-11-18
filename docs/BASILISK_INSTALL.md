# Basilisk C Installation Guide

This guide provides step-by-step instructions for installing and configuring Basilisk C, the computational fluid dynamics framework used in this project.

## Table of Contents

- [Overview](#overview)
- [System Requirements](#system-requirements)
- [Installation Methods](#installation-methods)
  - [Method 1: Using Darcs (Recommended)](#method-1-using-darcs-recommended)
  - [Method 2: Tarball Download](#method-2-tarball-download)
- [Configuration](#configuration)
- [Environment Setup](#environment-setup)
- [Verification](#verification)
- [Troubleshooting](#troubleshooting)
- [Platform-Specific Notes](#platform-specific-notes)

## Overview

**Basilisk C** is a powerful framework for solving partial differential equations on adaptive Cartesian meshes. It is particularly well-suited for computational fluid dynamics simulations, including:

- Two-phase flows
- Free surface flows
- Volume-of-Fluid (VOF) methods
- Embedded boundary methods
- Adaptive mesh refinement

**Official Website**: http://basilisk.fr/

**Documentation**: http://basilisk.fr/Tutorial

## System Requirements

### Minimum Requirements

- **Operating System**: Linux, macOS, or Unix-like system
- **Compiler**: C99-compliant C compiler (gcc 4.8+ or clang 3.4+)
- **Make**: GNU Make or compatible
- **Disk Space**: ~500 MB for source code and compiled binaries
- **RAM**: 2 GB minimum (more for large simulations)

### Recommended Tools

- **gcc**: 9.0 or later
- **gawk**: GNU AWK for preprocessing
- **gnuplot**: For visualization (optional)
- **ffmpeg**: For video generation (optional)
- **ImageMagick**: For image processing (optional)
- **MPI**: For parallel execution (optional)

## Installation Methods

### Method 1: Using Darcs (Recommended)

Darcs is the version control system used by Basilisk. This method ensures you get the latest version.

#### Step 1: Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y darcs gcc make gawk
```

**Fedora/RHEL/CentOS:**
```bash
sudo dnf install -y darcs gcc make gawk
```

**macOS (with Homebrew):**
```bash
brew install darcs gcc make gawk
```

#### Step 2: Clone the Repository

```bash
cd ~
darcs clone http://basilisk.fr/basilisk
```

This will create a `~/basilisk` directory with the complete source code.

#### Step 3: Build Basilisk

```bash
cd ~/basilisk/src

# For Linux
ln -s config.gcc config

# For macOS
# ln -s config.osx config

# Compile
make
```

The build process will:
1. Compile the `qcc` preprocessor (Basilisk's compiler wrapper)
2. Build core libraries
3. Create utilities and examples

**Build time**: 5-15 minutes depending on your system.

### Method 2: Tarball Download

If Darcs is not available, you can download a snapshot tarball.

#### Step 1: Install Basic Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y gcc make gawk wget
```

#### Step 2: Download and Extract

```bash
cd ~
wget http://basilisk.fr/basilisk/basilisk.tar.gz
tar xzf basilisk.tar.gz
cd basilisk/src
```

#### Step 3: Build

```bash
# For Linux
ln -s config.gcc config

# Compile
make
```

## Configuration

### Compiler Configuration Files

Basilisk includes several pre-configured compiler options:

- **config.gcc**: Standard GCC configuration (Linux)
- **config.osx**: macOS/Clang configuration
- **config.icc**: Intel compiler configuration
- **config.mpi**: MPI parallel configuration

### Custom Configuration

To create a custom configuration, copy and edit an existing config file:

```bash
cd ~/basilisk/src
cp config.gcc config.custom

# Edit compiler flags
nano config.custom

# Use custom config
ln -sf config.custom config
make clean
make
```

### Compiler Flags

Common flags in the config files:

```makefile
CC = gcc                          # C compiler
CFLAGS = -O2 -Wall               # Optimization and warnings
OPENMPI = 1                      # Enable MPI (optional)
CADNA = 0                        # CADNA library (optional)
```

## Environment Setup

### Required Environment Variables

Add these to your `~/.bashrc` (Linux) or `~/.zshrc` (macOS):

```bash
# Basilisk installation directory
export BASILISK=$HOME/basilisk

# Add qcc to PATH
export PATH=$PATH:$BASILISK

# Optional: Basilisk examples
export BASILISK_EXAMPLES=$BASILISK/src/examples
```

### Apply Changes

```bash
# Reload your shell configuration
source ~/.bashrc  # or ~/.zshrc for macOS
```

### Verify Environment

```bash
echo $BASILISK
# Should output: /home/username/basilisk

which qcc
# Should output: /home/username/basilisk/qcc
```

## Verification

### Test Installation

```bash
# Check qcc version
qcc --version

# Should output something like:
# qcc (GCC) 9.4.0
# ...
```

### Compile a Simple Test

Create a test file:

```bash
cat > test.c << 'EOF'
#include "grid/cartesian.h"

int main() {
  init_grid(64);
  printf("Basilisk works! Grid size: %d x %d\n", N, N);
}
EOF
```

Compile and run:

```bash
qcc -o test test.c
./test

# Expected output:
# Basilisk works! Grid size: 64 x 64
```

### Compile Project Files

Test with the files in this repository:

```bash
cd /path/to/your/project

# Compile the circle droplet simulation
qcc -O2 -Wall -o circle-droplet circle-droplet.c -lm

# Run it
./circle-droplet

# Compile the droplet impact simulation
qcc -O2 -Wall -o droplet-impact-orifice droplet-impact-orifice.c -lm

# Run it
./droplet-impact-orifice 2> log
```

## Troubleshooting

### Issue: `qcc: command not found`

**Solution**: Ensure Basilisk is in your PATH:
```bash
export PATH=$PATH:$HOME/basilisk
```

### Issue: `config: No such file or directory`

**Solution**: Create the symbolic link to a config file:
```bash
cd ~/basilisk/src
ln -s config.gcc config
```

### Issue: Compilation errors with MPI

**Solution**: If you don't need MPI, disable it in the config:
```bash
# Edit config file
nano ~/basilisk/src/config

# Set OPENMPI = 0
# Then rebuild
make clean && make
```

### Issue: `gawk: command not found`

**Solution**: Install GNU AWK:
```bash
# Ubuntu/Debian
sudo apt install gawk

# macOS
brew install gawk
```

### Issue: Math library linking errors

**Solution**: Always link with `-lm`:
```bash
qcc -o program program.c -lm
```

### Issue: X11/OpenGL errors (visualization)

**Solution**: Install development libraries:
```bash
# Ubuntu/Debian
sudo apt install libgl1-mesa-dev libglu1-mesa-dev

# Not required for headless simulations
```

## Platform-Specific Notes

### Linux (Ubuntu/Debian)

**Complete setup command:**
```bash
sudo apt update && sudo apt install -y darcs gcc make gawk
darcs clone http://basilisk.fr/basilisk
cd ~/basilisk/src
ln -s config.gcc config
make
echo "export BASILISK=$HOME/basilisk" >> ~/.bashrc
echo 'export PATH=$PATH:$BASILISK' >> ~/.bashrc
source ~/.bashrc
```

### macOS

**Prerequisites:**
1. Install Xcode Command Line Tools:
   ```bash
   xcode-select --install
   ```

2. Install Homebrew: https://brew.sh/

**Setup:**
```bash
brew install darcs gcc make gawk
darcs clone http://basilisk.fr/basilisk
cd ~/basilisk/src
ln -s config.osx config
make
echo "export BASILISK=$HOME/basilisk" >> ~/.zshrc
echo 'export PATH=$PATH:$BASILISK' >> ~/.zshrc
source ~/.zshrc
```

### Windows (WSL)

Use Windows Subsystem for Linux (WSL2) with Ubuntu:

1. Install WSL2: https://docs.microsoft.com/en-us/windows/wsl/install
2. Open Ubuntu terminal
3. Follow the Linux installation instructions above

### HPC Clusters

For high-performance computing environments:

1. Load appropriate modules:
   ```bash
   module load gcc/9.4.0
   module load openmpi/4.1.0
   ```

2. Use MPI configuration:
   ```bash
   cd ~/basilisk/src
   ln -s config.mpi config
   make
   ```

3. Compile with MPI:
   ```bash
   qcc -Wall -O2 -D_MPI=1 simulation.c -lm
   mpirun -np 8 ./a.out
   ```

## Additional Resources

### Official Documentation

- **Main Tutorial**: http://basilisk.fr/Tutorial
- **Examples**: http://basilisk.fr/src/examples/
- **Reference**: http://basilisk.fr/src/README
- **Sandbox** (user contributions): http://basilisk.fr/sandbox/

### Community Resources

- **GitHub Mirror**: https://github.com/comphy-lab/basilisk-C
- **Basilisk 101**: https://github.com/comphy-lab/Basilisk-101
- **Mailing List**: Check http://basilisk.fr for current contact info

### Learning Materials

1. **Tutorial**: Start with http://basilisk.fr/Tutorial
2. **Test Cases**: Browse http://basilisk.fr/src/test/
3. **Examples**: Study http://basilisk.fr/src/examples/
4. **This Project**: See simulation files and documentation in this repository

## Quick Reference

### Essential Commands

```bash
# Compile a program
qcc -O2 -Wall -o output input.c -lm

# Compile with debugging
qcc -g -Wall -o output input.c -lm

# Compile with MPI
qcc -Wall -D_MPI=1 -o output input.c -lm

# Run with MPI
mpirun -np 4 ./output

# Compile with higher optimization
qcc -O3 -Wall -o output input.c -lm

# Define preprocessor macros
qcc -DMAXLEVEL=10 -o output input.c -lm
```

### Common Preprocessor Flags

- `-DMAXLEVEL=N`: Set maximum refinement level
- `-D_MPI=1`: Enable MPI parallelization
- `-DDEBUG`: Enable debug output
- `-DTRASH=1`: Enable variable trashing (memory debugging)

## Getting Help

If you encounter issues not covered here:

1. Check the official documentation: http://basilisk.fr/
2. Search the examples: http://basilisk.fr/src/examples/
3. Review this project's documentation:
   - [README.md](README.md)
   - [AXISYMMETRIC_GUIDE.md](AXISYMMETRIC_GUIDE.md)
   - [DROPLET_IMPACT_ORIFICE.md](DROPLET_IMPACT_ORIFICE.md)

---

**Last Updated**: 2025
**Basilisk Version**: Latest (darcs repository)
