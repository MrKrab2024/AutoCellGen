#!/bin/bash 
mode=build/width_only

##########################################
###### Change input parameters here ######
##########################################
input=../../DATA/input 

#cdl
######### asap7sc7p5t netlist ############
#cdl=$input/asap7sc6t.sp
########## asap7sc6t netlist #############
#cdl=$input/asap7sc6t.sp
cdl=$1

#Placement configuration
dr=$input/placement_file.style  
#########################################

out=./output/width_only  
mkdir -p $out 

PlacementDir=./csyn_fp  

echo "Running width-only analysis..."
echo "Input CDL: $cdl"
echo "Output directory: $out"

$PlacementDir/$mode -i $cdl -d $dr -o $out 

echo "Width analysis completed!"
echo "Results saved to: $out/widths_summary.tsv"
echo "Detailed log saved to: $out/summary.txt"

# Move summary to parent directory for easy access
mv $out/summary.txt $out/../width_only_summary.txt
mv $out/widths_summary.tsv $out/../widths_summary.tsv
