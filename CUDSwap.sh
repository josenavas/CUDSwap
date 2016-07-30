#!/bin/sh

# The script has to be run as root
if [ ! $(id -u) = 0 ]; then
	echo "You must be superuser to run CUDSwap" 1>&2
	exit 1
fi

# Get the system threshold to be passed to CUDSwap module
threshold=`cat /proc/zoneinfo | grep high | awk '{sum += $2} END {print sum}'`
# Run swap creator process, niced and not attached to any terminal
nohup nice -20 ./swap_creator/swap_creator > /dev/null 2>&1 &
# Get the pid of the swap creator process to be passed to CUDSwap module
pid=$!
# Sleep 2 seconds allowing to the swap creator process to be set up
sleep 2
# Insert the CUDSwap module
insmod ./jprobe/mod_hack_brk.ko swap_creator_pid=$pid threshold=$threshold
