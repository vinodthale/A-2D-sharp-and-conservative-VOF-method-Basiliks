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

# Default flags
CFLAGS = -O2 -Wall
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
circle-droplet: circle-droplet.c myembed.h embed_*.h TPR2D.h tmp_fraction_field.h
	@echo "Compiling circle-droplet..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Droplet impact simulations
droplet-impact-orifice: droplet-impact-orifice.c axi.h myembed.h embed_*.h
	@echo "Compiling droplet-impact-orifice..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

droplet-impact-orifice-nondim: droplet-impact-orifice-nondim.c axi.h myembed.h embed_*.h
	@echo "Compiling droplet-impact-orifice-nondim..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

droplet-impact-sharp-orifice: droplet-impact-sharp-orifice.c axi.h myembed.h embed_*.h
	@echo "Compiling droplet-impact-sharp-orifice..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

droplet-impact-sharp-orifice-nondim: droplet-impact-sharp-orifice-nondim.c axi.h myembed.h embed_*.h
	@echo "Compiling droplet-impact-sharp-orifice-nondim..."
	$(QCC) $(CFLAGS) -o $@ $< $(LDFLAGS)

droplet-impact-round-orifice: droplet-impact-round-orifice.c axi.h myembed.h embed_*.h
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
	@echo "  README.md              Project overview"
	@echo "  BASILISK_INSTALL.md    Installation guide"
	@echo "  BASILISK_CONFIG.md     Configuration guide"

# Phony targets that don't correspond to files
.PHONY: all clean clean-all test high-res production debug mpi check-basilisk help

# Dependencies for header files
circle-droplet: myembed.h embed_contact.h embed_two-phase.h embed_tension.h \
                embed_vof.h embed_curvature.h embed_heights.h embed_height_normal.h \
                embed_correct_height.h embed_iforce.h TPR2D.h tmp_fraction_field.h

droplet-impact-orifice: axi.h myembed.h embed_contact.h embed_two-phase.h \
                        embed_tension.h embed_vof.h embed_curvature.h embed_heights.h \
                        embed_height_normal.h embed_correct_height.h embed_iforce.h

droplet-impact-orifice-nondim: axi.h myembed.h embed_contact.h embed_two-phase.h \
                               embed_tension.h embed_vof.h embed_curvature.h embed_heights.h \
                               embed_height_normal.h embed_correct_height.h embed_iforce.h

droplet-impact-sharp-orifice: axi.h myembed.h embed_contact.h embed_two-phase.h \
                              embed_tension.h embed_vof.h embed_curvature.h embed_heights.h \
                              embed_height_normal.h embed_correct_height.h embed_iforce.h

droplet-impact-sharp-orifice-nondim: axi.h myembed.h embed_contact.h embed_two-phase.h \
                                     embed_tension.h embed_vof.h embed_curvature.h embed_heights.h \
                                     embed_height_normal.h embed_correct_height.h embed_iforce.h

droplet-impact-round-orifice: axi.h myembed.h embed_contact.h embed_two-phase.h \
                              embed_tension.h embed_vof.h embed_curvature.h embed_heights.h \
                              embed_height_normal.h embed_correct_height.h embed_iforce.h
