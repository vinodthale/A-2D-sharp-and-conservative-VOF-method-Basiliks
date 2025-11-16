#!/bin/bash
#
# Run script for droplet impact on sharp orifice
#

echo "========================================="
echo "Droplet Impact: Sharp Orifice Case"
echo "========================================="
echo ""
echo "Parameters:"
echo "  Droplet diameter: D = 10.307 mm"
echo "  Orifice diameter: d = 6 mm (d/D = 0.58)"
echo "  Bond number: Bo = 6.0"
echo "  Contact angles: θ_r = 42°, θ_a = 68° (150° at edge)"
echo "  Mesh resolution: ~120 cells per radius"
echo ""
echo "Starting simulation..."
echo "========================================="
echo ""

# Create output directory if it doesn't exist
mkdir -p output_sharp

# Run the simulation
./droplet-impact-sharp-orifice

# Move output files to output directory
echo ""
echo "Moving output files to output_sharp/"
mv -f volume_sharp.txt output_sharp/ 2>/dev/null
mv -f interface_position_sharp.txt output_sharp/ 2>/dev/null
mv -f snapshot_sharp_*.dat output_sharp/ 2>/dev/null
mv -f field_sharp_*.dat output_sharp/ 2>/dev/null
mv -f movie_sharp.ppm output_sharp/ 2>/dev/null

echo ""
echo "========================================="
echo "Simulation completed!"
echo "========================================="
echo ""
echo "Output files in output_sharp/:"
echo "  - volume_sharp.txt"
echo "  - interface_position_sharp.txt"
echo "  - snapshot_sharp_*.dat"
echo "  - field_sharp_*.dat"
echo "  - movie_sharp.ppm"
echo ""
