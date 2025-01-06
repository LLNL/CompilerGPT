#!/usr/bin/env bash

src=$1
comp=$2

shift 2

# set optflags according to FAROS
OPTFLAGS="$@"

# find out if we compile with or w/o OpenMP
ompflag=""

for arg in "$@"; do
  if [ "$arg" = "-fopenmp" ]; then
    ompflag="$arg"
  fi  
done

set -e

if [[ $# -eq 0 ]]; then
  echo "Error: No files provided." >&2
  echo "Usage: $0 srcfile" >&2
  exit 1
fi

echo "$comp $OPTFLAGS -c -o bt.o $src"
$comp $OPTFLAGS -c -o bt.o $src

echo "$comp $ompflag -lm -o ../bin/bt.A bt.o ../common/c_print_results.o ../common/c_timers.o ../common/c_wtime.o"
$comp $ompflag -lm -o ../bin/bt.A bt.o ../common/c_print_results.o ../common/c_timers.o ../common/c_wtime.o

export OMP_NUM_THREADS=24 
../bin/bt.A >out.txt

../../../logfilter.bin logfilter.json out.txt
exit 0
