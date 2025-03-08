#!/usr/bin/env bash

if [[ $# -lt 2 ]]; then
  echo "Error: Missing provided." >&2
  echo "Usage: $0 srcfile compiler [compilerflags]" >&2
  exit 1
fi

src=$1
comp=$2

shift 2

# only compiler if the binary is older than the source file
if [ "perf.bin" -ot $src ]; then
  (set -x; $comp "$@" ./perf.cc "$src" -o perf.bin)

  success="$?"
  if [ $success -gt 0 ]; then
    exit $success
  fi
fi

export OMP_NUM_THREADS=24
./perf.bin

exit "$?"

