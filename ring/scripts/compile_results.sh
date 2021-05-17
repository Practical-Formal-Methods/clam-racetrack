#!/bin/bash

# Usage : bash compile_resultssh <results_folder>

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESULT_DIR="$(cd ${1} && pwd)"

pushd "$RESULT_DIR"
	rm results.txt
	for px in {0..49}
	do
		for py in {0..44}
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
popd
