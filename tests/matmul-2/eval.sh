#!/usr/bin/env bash

if [[ $# -lt 2 ]]; then
  echo "Error: Missing provided." >&2
  echo "Usage: $0 srcfile compiler [compilerflags]" >&2
  exit 1
fi

rm -f perf.bin

src=$1
comp=$2

shift 2

# test for OpenMP code
if grep -q "pragma omp" "$src"; then
  echo "Do not use OpenMP!"
  exit 1
fi

echo "$comp $@ perf.cc $src -o perf.bin"

$comp "$@" ./perf.cc "$src" -o perf.bin

success="$?"
if [ $success -gt 0 ]; then
  exit $success
fi

./perf.bin

exit "$?"

