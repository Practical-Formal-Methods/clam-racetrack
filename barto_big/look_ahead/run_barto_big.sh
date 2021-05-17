#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(cd .. && pwd)"
TMP_DIR="tmp_files"
RESULT_DIR="results"

# CHCEK if CN Noise is active
if grep -q "int noise = 1" /clam/crab/include/crab/analysis/abs_transformer.hpp; then
	if [ -z "$1" ]
	then
		echo "parameter noise needs to be passed"
		exit 0
	fi
	if [[ "$1" != "noise" ]]; then
		echo "paramter needs to be noise"
		exit 0
	fi
fi

if grep -q "int NN_Noise = 1" /deepsymbol/middleware.cpp; then
	if [[ "$1" == "noise" ]]; then
		echo "Cannot pass noise when NN noise is on"
		exit 0
	fi
fi

pushd "$SRC_DIR"
	mkdir $TMP_DIR
	mkdir $RESULT_DIR
	for px in {0..29}
	do
		for py in {0..32}
		do
			x=`expr $px + 1`
			y=`expr $py + 1`
			map_pos="$(cat ./scripts/barto-big.csv | head -n $x | tail -n 1 | cut -d',' -f$y)"
			if [[ $map_pos == "x" ]]; then
				echo "x" > "$RESULT_DIR/main_${px}_${py}.log"
			elif [[ $map_pos == "g" ]]; then
				echo "g" > "$RESULT_DIR/main_${px}_${py}.log"
			else
				sed "s/Position start_position = {0, 14};/Position start_position = {$px, $py};/g" ./look_ahead/barto_big.c > "$TMP_DIR/main_${px}_${py}.c"
				if [[ $1 == "noise" ]]; then
					sed -i "s/Velocity start_velocity = {0, 0};/Velocity start_velocity = {`python3 scripts/gen_ini_velocity.py barto_big ${px} ${py}`};/g" "$TMP_DIR/main_${px}_${py}.c"
				fi

			fi

		done
	done

	#Now we parallely run all these files and record SAFE/WARNING output only
	pushd $TMP_DIR
		echo "Starting Parallel Jobs"
		/usr/bin/time -v parallel -j $(nproc) "(/usr/bin/time -v clam.py {} --crab-disable-warnings --inline --crab-check=assert --crab-dom=boxes --crab-add-invariants=all --crab-track=arr --crab-do-not-print-invariants --crab-widening-delay=20) &> $SRC_DIR/$RESULT_DIR/\$(basename {} .c).log " ::: *.c 2>stats.log
		sed -i '1d' stats.log
	popd

	#Club all output together
	pushd $RESULT_DIR
		for px in {0..29}
		do
			for py in {0..32}
			do
				safe=$(cat main_${px}_${py}.log  | grep "safe" | cut -d' ' -f1)
				warn=$(cat main_${px}_${py}.log  | grep "warning checks" | cut -d' ' -f1)
				lines=$(wc -l main_${px}_${py}.log | cut -d' ' -f1)

				if [[ $lines == 1 ]]; then
					cat main_${px}_${py}.log >> results.txt
				elif [[ $safe == 2 && $warn == 0 ]]; then
					echo "1" >> results.txt
				elif [[ $safe == 1 && $warn == 1 ]]; then
					echo "0" >> results.txt
				elif [[ $safe == 0 && $warn == 2 ]]; then
					echo "2" >> results.txt
				else
					echo "2" >> results.txt
				fi

			done
		done

		rm -f /deepsymbol/info_*.json
		rm -f /deepsymbol/summary.txt 
		rm -f /deepsymbol/bound.log
		awk -f $SCRIPT_DIR/makeCSV.awk results.txt > results.csv
	popd 

	# Store total time
	pushd $SCRIPT_DIR
		grep "Elapsed" $SRC_DIR/$TMP_DIR/stats.log  | sed "s/^[ \t]*//" > $SRC_DIR/$RESULT_DIR/stats.log
		echo -n "Avg Time (m:ss): " >> $SRC_DIR/$RESULT_DIR/stats.log	
		date -d@$(echo "scale=2; (`grep 'User time' $SRC_DIR/$TMP_DIR/stats.log | rev | cut -d' ' -f1 | rev` + `grep 'System time' $SRC_DIR/$TMP_DIR/stats.log | rev | cut -d' ' -f1 | rev`) / 549" | bc -l) -u +%M:%S >> $SRC_DIR/$RESULT_DIR/stats.log
		echo -n "Avg NN Time (m:ss): " >> $SRC_DIR/$RESULT_DIR/stats.log
		date -d@$(bash nn_time_driver.sh $SRC_DIR/$RESULT_DIR 2>/dev/null) -u +%M:%S >> $SRC_DIR/$RESULT_DIR/stats.log
	popd

	rm -rf $TMP_DIR
popd
