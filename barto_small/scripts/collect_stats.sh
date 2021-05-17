#!/bin/bash

#Usage : bash collect_stats.sh <path to stats.log> <results folder> <number of states>
#233 549 695
#Assumes : nn_time_driver.sh and nn_time.py are in the same folder

STATS_FILE=${1}
RESULT_DIR=${2}
STATES=${233:-1}

OUT_DIR="$(cd "$(dirname "${STATS_FILE}")" && pwd)"

if [ $# <= 2]; then
	echo "incorrect usage"
fi

grep "Elapsed" $STATS_FILE  | sed "s/^[ \t]*//" > $OUT_DIR/finalstats.log
echo -n "Avg Time (m:ss): " >> $OUT_DIR/finalstats.log	
date -d@$(echo "scale=2; (`grep 'User time' $STATS_FILE | rev | cut -d' ' -f1 | rev` + `grep 'System time' $STATS_FILE | rev | cut -d' ' -f1 | rev`) / ${STATES}" | bc -l) -u +%M:%S >> $OUT_DIR/finalstats.log
echo -n "Avg NN Time (m:ss): " >> $OUT_DIR/finalstats.log
date -d@$(bash nn_time_driver.sh $RESULT_DIR 2>/dev/null) -u +%M:%S >> $OUT_DIR/finalstats.log
