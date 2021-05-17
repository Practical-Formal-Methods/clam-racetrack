#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(cd .. && pwd)"
TMP_DIR="tmp_files"
RESULT_DIR="results"

pushd "$SRC_DIR"
	mkdir $TMP_DIR
	mkdir $RESULT_DIR
	for px in {0..24}
	do
		for py in {0..24}
		do
			if [[ $px -ge 0 && $px -le 13 && $py -ge 0 && $py -le 13 ]]; then
				echo "0" > "$RESULT_DIR/main_${px}_${py}.log"
			elif [[ $px -eq 24 ]]; then
				echo "0" > "$RESULT_DIR/main_${px}_${py}.log"
			elif [[ $py -eq 24 ]]; then
				echo "0" > "$RESULT_DIR/main_${px}_${py}.log"
			elif [[ $px -ge 14 && $px -le 23 && $py -eq 0 ]]; then
				echo "1" > "$RESULT_DIR/main_${px}_${py}.log"
			else
				sed "s/Position start_position = {0, 14};/Position start_position = {$px, $py};/g" final.c > "$TMP_DIR/main_${px}_${py}.c"
			fi

		done
	done

	#Now we parallely run all these files and record SAFE/WARNING output only
	pushd $TMP_DIR
		echo "Starting Parallel Jobs"
		parallel -j $(nproc) "clam.py {} --inline --crab-check=assert --crab-dom=boxes --crab-add-invariants=all --crab-track=arr --crab-do-not-print-invariants --crab-widening-delay=20 > $SRC_DIR/$RESULT_DIR/\$(basename {} .c).log " ::: *.c
	popd

	#Club all output together
	pushd $RESULT_DIR
		for px in {0..24}
		do
			for py in {0..24}
			do
				safe=$(cat main_${px}_${py}.log  | grep "Reached goal" | wc -l)
				warn=$(cat main_${px}_${py}.log  | grep "Crashed" | wc -l)
				lines=$(wc -l main_${px}_${py}.log | cut -d' ' -f1)

				if [[ $warn != 0 ]]; then
					echo "Unsafe"
					sed -i "s/[0-9]*  Number of total warning check/$warn  Number of total warning check/g" main_${px}_${py}.log 
					sed -i "s/[0-9]*  Number of total safe check/$safe  Number of total safe check/g" main_${px}_${py}.log 
					echo "0" >> results.txt
				elif [[ $safe != 0 ]]; then
					echo "Safe"
					sed -i "s/[0-9]*  Number of total safe check/$safe  Number of total safe check/g" main_${px}_${py}.log 
					echo "1" >> results.txt
				elif [[ $safe -eq 0 && $warn -eq 0 && $lines -eq 1 ]]; then
					cat main_${px}_${py}.log >> results.txt
				else
					echo "2" >> results.txt
			fi
			done
		done

		python3 ${SCRIPT_DIR}/gen_heatmap.py
		#rm *.log

	popd 
popd
