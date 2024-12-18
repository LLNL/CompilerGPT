#!/usr/bin/env bash

# set optflags according to FAROS
#~ OPTFLAGS="-O3 -march=native -fsave-optimization-record"
OPTFLAGS="-O3 -march=native -fopenmp -fsave-optimization-record -save-stats"

set -e

if [[ $# -eq 0 ]]; then
  echo "Error: No files provided." >&2
  echo "Usage: $0 srcfile" >&2
  exit 1
fi

src=$1

echo "/usr/bin/clang -I../common $OPTFLAGS -c -o bt.o $src"
/usr/bin/clang -I../common $OPTFLAGS -c -o bt.o $src

echo "/usr/bin/clang -fopenmp -lm -o ../bin/bt.A bt.o ../common/c_print_results.o ../common/c_timers.o ../common/c_wtime.o"
/usr/bin/clang -fopenmp -lm -o ../bin/bt.A bt.o ../common/c_print_results.o ../common/c_timers.o ../common/c_wtime.o

export OMP_NUM_THREADS=24 
../bin/bt.A >out.txt

../../../logfilter.bin logfilter.json out.txt
exit 0
