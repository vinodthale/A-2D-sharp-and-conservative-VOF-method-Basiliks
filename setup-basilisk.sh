#!/bin/bash
#
# Basilisk C Setup Script
# Automated installation of Basilisk C and dependencies
#
# Usage:
#   ./setup-basilisk.sh [OPTIONS]
#
# Options:
#   --help          Show this help message
#   --method darcs  Install using darcs (default)
#   --method tar    Install using tarball download
#   --prefix DIR    Installation directory (default: $HOME/basilisk)
#   --skip-deps     Skip dependency installation
#   --with-mpi      Enable MPI support
#

set -e  # Exit on error

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default options
INSTALL_METHOD="darcs"
INSTALL_PREFIX="$HOME/basilisk"
SKIP_DEPS=false
WITH_MPI=false

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Help message
show_help() {
    cat << EOF
Basilisk C Setup Script

This script automates the installation of Basilisk C and its dependencies.

Usage:
    ./setup-basilisk.sh [OPTIONS]

Options:
    --help          Show this help message
    --method darcs  Install using darcs version control (default)
    --method tar    Install using tarball download
    --prefix DIR    Installation directory (default: \$HOME/basilisk)
    --skip-deps     Skip dependency installation (use if already installed)
    --with-mpi      Enable MPI support for parallel simulations

Examples:
    # Standard installation
    ./setup-basilisk.sh

    # Install to custom location
    ./setup-basilisk.sh --prefix /opt/basilisk

    # Install with MPI support
    ./setup-basilisk.sh --with-mpi

    # Install using tarball (no darcs)
    ./setup-basilisk.sh --method tar

For more information, see BASILISK_INSTALL.md
EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            show_help
            exit 0
            ;;
        --method)
            INSTALL_METHOD="$2"
            shift 2
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --skip-deps)
            SKIP_DEPS=true
            shift
            ;;
        --with-mpi)
            WITH_MPI=true
            shift
            ;;
        *)
            log_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Detect platform
detect_platform() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

PLATFORM=$(detect_platform)

# Detect Linux distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "$ID"
    else
        echo "unknown"
    fi
}

# Install dependencies
install_dependencies() {
    log_info "Installing dependencies for $PLATFORM..."

    case $PLATFORM in
        linux)
            DISTRO=$(detect_distro)
            case $DISTRO in
                ubuntu|debian)
                    log_info "Detected Ubuntu/Debian"
                    sudo apt update
                    if [ "$INSTALL_METHOD" = "darcs" ]; then
                        sudo apt install -y darcs gcc make gawk wget
                    else
                        sudo apt install -y gcc make gawk wget
                    fi

                    if [ "$WITH_MPI" = true ]; then
                        log_info "Installing MPI support..."
                        sudo apt install -y libopenmpi-dev openmpi-bin
                    fi
                    ;;
                fedora|rhel|centos)
                    log_info "Detected Fedora/RHEL/CentOS"
                    if [ "$INSTALL_METHOD" = "darcs" ]; then
                        sudo dnf install -y darcs gcc make gawk wget
                    else
                        sudo dnf install -y gcc make gawk wget
                    fi

                    if [ "$WITH_MPI" = true ]; then
                        log_info "Installing MPI support..."
                        sudo dnf install -y openmpi-devel
                    fi
                    ;;
                *)
                    log_warning "Unknown Linux distribution: $DISTRO"
                    log_warning "Please install manually: gcc, make, gawk, wget"
                    if [ "$INSTALL_METHOD" = "darcs" ]; then
                        log_warning "Also install: darcs"
                    fi
                    ;;
            esac
            ;;
        macos)
            log_info "Detected macOS"
            if ! command -v brew &> /dev/null; then
                log_error "Homebrew not found. Please install from https://brew.sh/"
                exit 1
            fi

            if [ "$INSTALL_METHOD" = "darcs" ]; then
                brew install darcs gcc make gawk wget
            else
                brew install gcc make gawk wget
            fi

            if [ "$WITH_MPI" = true ]; then
                log_info "Installing MPI support..."
                brew install open-mpi
            fi
            ;;
        *)
            log_error "Unsupported platform: $OSTYPE"
            exit 1
            ;;
    esac

    log_success "Dependencies installed successfully"
}

# Install Basilisk using darcs
install_with_darcs() {
    log_info "Installing Basilisk using darcs..."

    if [ -d "$INSTALL_PREFIX" ]; then
        log_warning "Directory $INSTALL_PREFIX already exists"
        read -p "Remove and reinstall? [y/N] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            rm -rf "$INSTALL_PREFIX"
        else
            log_error "Installation cancelled"
            exit 1
        fi
    fi

    # Clone repository
    log_info "Cloning Basilisk repository (this may take a few minutes)..."
    darcs clone http://basilisk.fr/basilisk "$INSTALL_PREFIX"

    log_success "Repository cloned to $INSTALL_PREFIX"
}

# Install Basilisk using tarball
install_with_tarball() {
    log_info "Installing Basilisk using tarball..."

    local temp_dir=$(mktemp -d)
    cd "$temp_dir"

    log_info "Downloading Basilisk tarball..."
    wget http://basilisk.fr/basilisk/basilisk.tar.gz

    log_info "Extracting tarball..."
    tar xzf basilisk.tar.gz

    if [ -d "$INSTALL_PREFIX" ]; then
        log_warning "Directory $INSTALL_PREFIX already exists"
        read -p "Remove and reinstall? [y/N] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            rm -rf "$INSTALL_PREFIX"
        else
            log_error "Installation cancelled"
            cd - > /dev/null
            rm -rf "$temp_dir"
            exit 1
        fi
    fi

    mv basilisk "$INSTALL_PREFIX"
    cd - > /dev/null
    rm -rf "$temp_dir"

    log_success "Basilisk extracted to $INSTALL_PREFIX"
}

# Build Basilisk
build_basilisk() {
    log_info "Building Basilisk..."

    cd "$INSTALL_PREFIX/src"

    # Select configuration file
    if [ "$WITH_MPI" = true ]; then
        log_info "Configuring for MPI support..."
        ln -sf config.mpi config
    elif [ "$PLATFORM" = "macos" ]; then
        log_info "Using macOS configuration..."
        ln -sf config.osx config
    else
        log_info "Using GCC configuration..."
        ln -sf config.gcc config
    fi

    # Build
    log_info "Compiling (this may take 5-15 minutes)..."
    make

    log_success "Basilisk built successfully"
}

# Setup environment
setup_environment() {
    log_info "Setting up environment variables..."

    # Determine shell config file
    if [ "$PLATFORM" = "macos" ]; then
        SHELL_CONFIG="$HOME/.zshrc"
    else
        SHELL_CONFIG="$HOME/.bashrc"
    fi

    # Check if already configured
    if grep -q "BASILISK=" "$SHELL_CONFIG" 2>/dev/null; then
        log_warning "Basilisk environment variables already in $SHELL_CONFIG"
    else
        log_info "Adding environment variables to $SHELL_CONFIG"
        echo "" >> "$SHELL_CONFIG"
        echo "# Basilisk C environment" >> "$SHELL_CONFIG"
        echo "export BASILISK=$INSTALL_PREFIX" >> "$SHELL_CONFIG"
        echo 'export PATH=$PATH:$BASILISK' >> "$SHELL_CONFIG"
        log_success "Environment variables added to $SHELL_CONFIG"
    fi

    # Set for current session
    export BASILISK="$INSTALL_PREFIX"
    export PATH="$PATH:$BASILISK"
}

# Verify installation
verify_installation() {
    log_info "Verifying installation..."

    # Set environment for this session
    export BASILISK="$INSTALL_PREFIX"
    export PATH="$PATH:$BASILISK"

    # Check if qcc exists
    if [ ! -f "$INSTALL_PREFIX/qcc" ]; then
        log_error "qcc not found in $INSTALL_PREFIX"
        exit 1
    fi

    # Test compilation
    local test_file=$(mktemp --suffix=.c)
    cat > "$test_file" << 'EOF'
#include "grid/cartesian.h"

int main() {
  init_grid(64);
  printf("Basilisk test successful! Grid: %d x %d\n", N, N);
}
EOF

    log_info "Testing compilation with qcc..."
    local test_output=$(mktemp)

    if "$INSTALL_PREFIX/qcc" -o "$test_output" "$test_file" 2>&1; then
        if "$test_output" 2>&1 | grep -q "Basilisk test successful"; then
            log_success "Test compilation and execution successful!"
        else
            log_error "Test execution failed"
            rm -f "$test_file" "$test_output"
            exit 1
        fi
    else
        log_error "Test compilation failed"
        rm -f "$test_file" "$test_output"
        exit 1
    fi

    rm -f "$test_file" "$test_output"
}

# Main installation process
main() {
    echo ""
    log_info "========================================="
    log_info "Basilisk C Installation Script"
    log_info "========================================="
    echo ""
    log_info "Platform: $PLATFORM"
    log_info "Installation method: $INSTALL_METHOD"
    log_info "Installation prefix: $INSTALL_PREFIX"
    log_info "MPI support: $WITH_MPI"
    echo ""

    # Install dependencies
    if [ "$SKIP_DEPS" = false ]; then
        install_dependencies
    else
        log_info "Skipping dependency installation"
    fi

    # Install Basilisk
    if [ "$INSTALL_METHOD" = "darcs" ]; then
        install_with_darcs
    elif [ "$INSTALL_METHOD" = "tar" ]; then
        install_with_tarball
    else
        log_error "Unknown installation method: $INSTALL_METHOD"
        exit 1
    fi

    # Build
    build_basilisk

    # Setup environment
    setup_environment

    # Verify
    verify_installation

    # Success message
    echo ""
    log_success "========================================="
    log_success "Basilisk C installation complete!"
    log_success "========================================="
    echo ""
    log_info "To use Basilisk, either:"
    log_info "  1. Open a new terminal, or"
    log_info "  2. Run: source ~/.bashrc  (or ~/.zshrc on macOS)"
    echo ""
    log_info "Test your installation:"
    log_info "  qcc --version"
    echo ""
    log_info "For more information, see BASILISK_INSTALL.md"
    echo ""
}

# Run main installation
main
