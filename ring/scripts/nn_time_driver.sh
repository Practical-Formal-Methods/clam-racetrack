#!/bin/bash

# Usage : bash nn_time_driver.sh <folder> -- where <folder> is "results"
# Assumes : nn_time.py script present in current dir

# XXX: Set this based on map
STATES="695"

FOLDER="$(cd $(dirname $1); pwd)/$(basename $1)"
cp nn_time.py ${FOLDER}/.
pushd "${FOLDER}" 1>&2
    rm -f outtime
    for f in *.log; do
        sed -n "s/^\(Reached crab intrinsic.*\)/\1/p" ${f} > logfile
        python3 nn_time.py ${f} outtime
    done
    rm nn_time.py
    rm logfile
    echo "scale=2; (`perl -nle '$sum += $_ } END { print $sum' outtime` / ${STATES})" | bc -l
    rm outtime
popd 1>&2


