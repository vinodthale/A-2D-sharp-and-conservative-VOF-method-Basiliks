# Makefile for 2D Sharp and Conservative VOF Method - Basilisk C
#
# Usage:
#   make              # Build all simulations
#   make clean        # Remove compiled files
#   make test         # Test compilation
#   make high-res     # Build with higher resolution
#   make debug        # Build with debug symbols
#   make help         # Show this help

# Compiler
QCC = qcc

# Source directories
SRC_AXI = src/axisymmetric
SRC_2D = src/2d-cartesian
INCLUDE = include/basilisk

# Default flags (include custom headers)
CFLAGS = -O2 -Wall -I$(INCLUDE)/core -I$(INCLUDE)/methods
LDFLAGS = -lm

# Maximum refinement level (override with: make MAXLEVEL=10)
MAXLEVEL = 9

# Simulation targets
TARGETS = circle-droplet \
          droplet-impact-orifice \
          droplet-impact-orifice-nondim \
          droplet-impact-sharp-orifice \
          droplet-impact-sharp-orifice-nondim \
          droplet-impact-round-orifice

# Default target
.PHONY: all
all: $(TARGETS)
	@echo ""
	@echo "All simulations compiled successfully!"
	@echo "Run with: ./circle-droplet 2> log"
	@echo "         or ./droplet-impact-orifice 2> log"

# Generic compilation rule
%: %.c
	@echo "Compiling $@..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Circle droplet simulation
circle-droplet: $(SRC_2D)/circle-droplet.c
	@echo "Compiling circle-droplet..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Droplet impact simulations
droplet-impact-orifice: $(SRC_AXI)/droplet-impact-orifice.c
	@echo "Compiling droplet-impact-orifice..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

droplet-impact-orifice-nondim: $(SRC_AXI)/droplet-impact-orifice-nondim.c
	@echo "Compiling droplet-impact-orifice-nondim..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

droplet-impact-sharp-orifice: $(SRC_AXI)/droplet-impact-sharp-orifice.c
	@echo "Compiling droplet-impact-sharp-orifice..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

droplet-impact-sharp-orifice-nondim: $(SRC_AXI)/droplet-impact-sharp-orifice-nondim.c
	@echo "Compiling droplet-impact-sharp-orifice-nondim..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

droplet-impact-round-orifice: $(SRC_AXI)/droplet-impact-round-orifice.c
	@echo "Compiling droplet-impact-round-orifice..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# High resolution build (MAXLEVEL=10)
.PHONY: high-res
high-res:
	@echo "Building with high resolution (MAXLEVEL=$(MAXLEVEL))..."
	$(MAKE) all CFLAGS="-O3 -march=native -DMAXLEVEL=$(MAXLEVEL)"

# Production build (optimized)
.PHONY: production
production:
	@echo "Building production version with full optimization..."
	$(MAKE) all CFLAGS="-O3 -march=native -DNDEBUG"

# Debug build
.PHONY: debug
debug:
	@echo "Building with debug symbols..."
	$(MAKE) all CFLAGS="-g -O0 -Wall -DDEBUG"

# MPI build
.PHONY: mpi
mpi:
	@echo "Building with MPI support..."
	$(MAKE) all CFLAGS="-O3 -D_MPI=1"
	@echo ""
	@echo "Run with: mpirun -np 4 ./droplet-impact-orifice 2> log"

# Test compilation (compile just one simulation quickly)
.PHONY: test
test: circle-droplet
	@echo ""
	@echo "Test compilation successful!"
	@echo "Testing execution..."
	@./circle-droplet --help || echo "Basilisk simulation compiled."

# Clean compiled files
.PHONY: clean
clean:
	@echo "Cleaning compiled files..."
	rm -f $(TARGETS)
	rm -f *.o *~
	rm -f a.out

# Clean all output files
.PHONY: clean-all
clean-all: clean
	@echo "Cleaning all output files..."
	rm -f log log-*
	rm -f *.mp4 *.ppm *.png
	rm -f out-* field-*
	rm -f dump-*
	rm -f *.dat *.txt

# Check Basilisk installation
.PHONY: check-basilisk
check-basilisk:
	@echo "Checking Basilisk installation..."
	@if command -v qcc >/dev/null 2>&1; then \
		echo "✓ qcc found: $$(which qcc)"; \
		qcc --version 2>&1 | head -1 || echo "qcc is available"; \
	else \
		echo "✗ qcc not found in PATH"; \
		echo "  Please install Basilisk or add it to PATH"; \
		echo "  See BASILISK_INSTALL.md for installation instructions"; \
		exit 1; \
	fi
	@if [ -n "$$BASILISK" ]; then \
		echo "✓ BASILISK environment variable set: $$BASILISK"; \
	else \
		echo "⚠ BASILISK environment variable not set"; \
		echo "  Consider adding: export BASILISK=~/basilisk"; \
	fi

# Show available targets
.PHONY: help
help:
	@echo "Makefile for 2D Sharp and Conservative VOF Method"
	@echo ""
	@echo "Usage:"
	@echo "  make              Build all simulations with default settings"
	@echo "  make test         Quick test compilation"
	@echo "  make high-res     Build with high resolution (MAXLEVEL=10)"
	@echo "  make production   Build with full optimization"
	@echo "  make debug        Build with debug symbols"
	@echo "  make mpi          Build with MPI support"
	@echo "  make clean        Remove compiled binaries"
	@echo "  make clean-all    Remove all compiled and output files"
	@echo "  make check-basilisk  Verify Basilisk installation"
	@echo "  make help         Show this help message"
	@echo ""
	@echo "Individual targets:"
	@echo "  circle-droplet                    Original cylinder simulation"
	@echo "  droplet-impact-orifice            Droplet impact (dimensional)"
	@echo "  droplet-impact-orifice-nondim     Droplet impact (non-dimensional)"
	@echo "  droplet-impact-sharp-orifice      Sharp orifice (dimensional)"
	@echo "  droplet-impact-sharp-orifice-nondim  Sharp orifice (non-dimensional)"
	@echo "  droplet-impact-round-orifice      Round orifice"
	@echo ""
	@echo "Variables:"
	@echo "  MAXLEVEL=N        Set maximum refinement level (default: 9)"
	@echo "  CFLAGS='...'      Override compiler flags"
	@echo ""
	@echo "Examples:"
	@echo "  make MAXLEVEL=10                  Build with higher resolution"
	@echo "  make high-res MAXLEVEL=11         Build with very high resolution"
	@echo "  make CFLAGS='-O3 -march=native'   Custom optimization"
	@echo ""
	@echo "For more information, see:"
	@echo "  README.md                      Project overview"
	@echo "  docs/BASILISK_INSTALL.md       Installation guide"
	@echo "  docs/BASILISK_CONFIG.md        Configuration guide"

# Phony targets that don't correspond to files
.PHONY: all clean clean-all test high-res production debug mpi check-basilisk help

# Header file dependencies
HEADERS_CORE = $(INCLUDE)/core/axi.h $(INCLUDE)/core/myembed.h
HEADERS_METHODS = $(INCLUDE)/methods/embed_contact.h $(INCLUDE)/methods/embed_two-phase.h \
                  $(INCLUDE)/methods/embed_tension.h $(INCLUDE)/methods/embed_vof.h \
                  $(INCLUDE)/methods/embed_curvature.h $(INCLUDE)/methods/embed_heights.h \
                  $(INCLUDE)/methods/embed_height_normal.h $(INCLUDE)/methods/embed_correct_height.h \
                  $(INCLUDE)/methods/embed_iforce.h $(INCLUDE)/methods/TPR2D.h \
                  $(INCLUDE)/methods/tmp_fraction_field.h

# Dependencies for simulations
circle-droplet: $(SRC_2D)/circle-droplet.c $(HEADERS_CORE) $(HEADERS_METHODS)
droplet-impact-orifice: $(SRC_AXI)/droplet-impact-orifice.c $(HEADERS_CORE) $(HEADERS_METHODS)
droplet-impact-orifice-nondim: $(SRC_AXI)/droplet-impact-orifice-nondim.c $(HEADERS_CORE) $(HEADERS_METHODS)
droplet-impact-sharp-orifice: $(SRC_AXI)/droplet-impact-sharp-orifice.c $(HEADERS_CORE) $(HEADERS_METHODS)
droplet-impact-sharp-orifice-nondim: $(SRC_AXI)/droplet-impact-sharp-orifice-nondim.c $(HEADERS_CORE) $(HEADERS_METHODS)
droplet-impact-round-orifice: $(SRC_AXI)/droplet-impact-round-orifice.c $(HEADERS_CORE) $(HEADERS_METHODS)
