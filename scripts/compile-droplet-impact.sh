#!/bin/bash
#
# Compilation script for droplet impact simulations
# Compiles both round and sharp orifice cases
#

echo "========================================="
echo "Compiling Droplet Impact Simulations"
echo "========================================="

# Check if BASILISK environment variable is set
if [ -z "$BASILISK" ]; then
    echo "Warning: BASILISK environment variable not set"
    echo "Trying to use default Basilisk installation"
    export BASILISK=/usr/local/basilisk
fi

# Compile round orifice case
echo ""
echo "Compiling round orifice simulation..."
qcc -Wall -O2 droplet-impact-round-orifice.c -o droplet-impact-round-orifice \
    -L$BASILISK/gl -lglutils -lfb_tiny -lm

if [ $? -eq 0 ]; then
    echo "✓ Round orifice simulation compiled successfully"
else
    echo "✗ Round orifice compilation failed"
    exit 1
fi

# Compile sharp orifice case
echo ""
echo "Compiling sharp orifice simulation..."
qcc -Wall -O2 droplet-impact-sharp-orifice.c -o droplet-impact-sharp-orifice \
    -L$BASILISK/gl -lglutils -lfb_tiny -lm

if [ $? -eq 0 ]; then
    echo "✓ Sharp orifice simulation compiled successfully"
else
    echo "✗ Sharp orifice compilation failed"
    exit 1
fi

echo ""
echo "========================================="
echo "Compilation completed successfully!"
echo "========================================="
echo ""
echo "To run the simulations:"
echo "  Round orifice:  ./droplet-impact-round-orifice"
echo "  Sharp orifice:  ./droplet-impact-sharp-orifice"
echo ""
