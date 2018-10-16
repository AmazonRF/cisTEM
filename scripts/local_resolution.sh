#!/bin/bash 	
#
# Parallelize local resolution estimation so it can run in a reasonable time.
# Make sure the program "local_resolution" is in your PATH before running
#
# Alexis Rohou, May-June 2018
#
# Last modification: 27-Aug-2018
#

#
# User-supplied parameters
#
input_volume_fsc_one=$1
input_volume_fsc_two=$2
input_volume_mask=$3
pixel_size=$4
symmetry=$5
box_size=$6
use_fixed=$7
fixed_fsc_thresh=$8
snr_target=$9
snr_confidence=${10}
sampling_step=${11}
slices_per_processor=${12}
total_number_of_slices=${13}
phase_randomize=${14}
randomization_resolution=${15}
output_volume=${16}

if [ "$#" -ne 16 ]; then
	echo " "
	echo "16 arguments are expected, you only supplied $#"
	echo "Usage: $0 input_fsc_vol_one input_fsc_vol_two input_mask_vol pixel_size symmetry box_size use_fixed fixed_fsc_thresh snr_target snr_confidence sampling_step slices_per_processor total_number_of_slices phase_randomize randomization_resolution output_volume"
	echo "Suggested defaults:"
	echo "box_size = 20"
	echo "use_fixed = n [alternative:y]"
	echo "fixed_fsc_thresh = 0.5"
	echo "snr_target = 0.334"
	echo "snr_confidence = 5.0"
	echo "sampling_step = 2 or 4"
	echo "phase_randomize = n [alternative:y]"
	echo "randomization_resolution = 20.0"
	echo " "
	exit
fi

#
# Loop over processors
#
first_slice=1
processor_counter=1
job_pid=()
while [ $first_slice -le $total_number_of_slices ]
do
	last_slice=$( echo "$first_slice + $slices_per_processor - 1" | bc)
	if [ $last_slice -gt $total_number_of_slices ]; then last_slice=$total_number_of_slices; fi
	log_fn="local_resolution_${processor_counter}.log"

	echo "$first_slice $last_slice $log_fn"

#
if [ x"$use_fixed" = x"y" ]; then
local_resolution<<eof > $log_fn 2>&1 &
$input_volume_fsc_one
$input_volume_fsc_two
$input_volume_mask
tmp_${processor_counter}.mrc
$pixel_size
$symmetry
$first_slice
$last_slice
$sampling_step
$box_size
$use_fixed
$fixed_fsc_thresh
y
$phase_randomize
$randomization_resolution
eof
else
local_resolution<<eof > $log_fn 2>&1 &
$input_volume_fsc_one
$input_volume_fsc_two
$input_volume_mask
tmp_${processor_counter}.mrc
$pixel_size
$symmetry
$first_slice
$last_slice
$sampling_step
$box_size
$use_fixed
$snr_target
$snr_confidence
y
$phase_randomize
$randomization_resolution
eof
fi
	
	# Remember the PID for this job
	job_pid+=($!)
	
	# Get ready for next increment
	(( first_slice = $last_slice + 1 ))
	(( processor_counter = $processor_counter + 1 ))
done

#
# Wait for the local resolution jobs to complete
#
echo "Waiting for local resolution estimation jobs to complete..."
for pid in ${job_pid[*]}; do
	wait $pid
	echo "Job $pid finished..."
done

#
# Merge the intermediate volumes
# 
log_fn="local_resolution_finalize.log"
local_resolution_finalize<<eof > $log_fn 2>&1
tmp_1.mrc
$slices_per_processor
$sampling_step
$output_volume
eof