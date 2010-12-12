#!/bin/sh

#
# Just a simple script that iterates over the outputs and turns
# each one on and off. This can be useful to test what the U401 
# is actually controlling, like relays.
#

while true
do

    for x in 0 1 2 3 4 5 6 7
    do
	./u401ctl $x=on || exit 1
	usleep 100000
	./u401ctl $x=off || exit 1
    done
    
done

exit 0
