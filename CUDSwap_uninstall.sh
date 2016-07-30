#!/bin/sh

# The script has to be run as root
if [ ! $(id -u) = 0 ]; then
	echo "You must be superuser to run CUDSwap" 1>&2
	exit 1
fi

pid=`ps -e | grep swap_creator | cut -d\  -f2`

/bin/kill -SIGUSR2 $pid

rmmod mod_hack_brk
