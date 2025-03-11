#!/usr/bin/env bash

TEST=bt

if [[ $# -lt 2 ]]; then
  echo "Error: Missing provided." >&2
  echo "Usage: $0 srcfile compiler [compilerflags]" >&2
  exit 1
fi

src=$1
comp=$2

shift 2

set -e

if [ $TEST.bin -ot $src ]; then
  (set -x; $comp "$@" -c -o $TEST.o $src)

  success="$?"
  if [ $success -gt 0 ]; then
    exit $success
  fi

  (set -x; $comp -fopenmp -lm -o $TEST.bin $TEST.o ../common/c_print_results.o ../common/c_timers.o ../common/c_randdp.o ../common/c_wtime.o)

  success="$?"
  if [ $success -gt 0 ]; then
    exit $success
  fi
fi

export OMP_NUM_THREADS=24
$TEST.bin >out.txt

../../../logfilter.bin logfilter.json out.txt
exit 0
