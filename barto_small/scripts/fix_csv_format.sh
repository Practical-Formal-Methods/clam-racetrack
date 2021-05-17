#!/bin/bash

#Usage : bash fix_csv_format.sh <results.csv>
if [ $# != 1 ]; then
  echo "incorrect usage"
  exit
fi

OUT_DIR="$(cd "$(dirname "${1}")" && pwd)"
awk -F, 'NR<33{$1="x";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
awk -F, 'NR<33{$2="x";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
awk -F, 'NR<33{$3="x";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
awk -F, 'NR<33{$4="x";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
awk -F, 'NR<33{$5="x";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
awk -F, 'NR<5{$10="x";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
awk -F, 'NR<9{$11="x";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
awk -F, 'NR<13{$12="x";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
awk -F, 'NR>32 && NR<36{$1="g";}1' OFS=, ${1} > ${OUT_DIR}/r.csv
mv ${OUT_DIR}/r.csv ${1}
