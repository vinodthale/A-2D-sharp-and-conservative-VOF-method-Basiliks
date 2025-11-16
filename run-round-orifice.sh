#!/bin/bash
#
# Run script for droplet impact on round orifice
#

echo "========================================="
echo "Droplet Impact: Round Orifice Case"
echo "========================================="
echo ""
echo "Parameters:"
echo "  Droplet diameter: D = 9.315 mm"
echo "  Orifice diameter: d = 6 mm (d/D = 0.644)"
echo "  Bond number: Bo = 4.9"
echo "  Contact angle: θ = 180°"
echo "  Mesh resolution: ~120 cells per radius"
echo ""
echo "Starting simulation..."
echo "========================================="
echo ""

# Create output directory if it doesn't exist
mkdir -p output_round

# Run the simulation
./droplet-impact-round-orifice

# Move output files to output directory
echo ""
echo "Moving output files to output_round/"
mv -f volume_round.txt output_round/ 2>/dev/null
mv -f interface_position_round.txt output_round/ 2>/dev/null
mv -f snapshot_round_*.dat output_round/ 2>/dev/null
mv -f field_round_*.dat output_round/ 2>/dev/null
mv -f movie_round.ppm output_round/ 2>/dev/null

echo ""
echo "========================================="
echo "Simulation completed!"
echo "========================================="
echo ""
echo "Output files in output_round/:"
echo "  - volume_round.txt"
echo "  - interface_position_round.txt"
echo "  - snapshot_round_*.dat"
echo "  - field_round_*.dat"
echo "  - movie_round.ppm"
echo ""
