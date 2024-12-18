#!/usr/bin/env bash

if [[ $# -eq 0 ]]; then
  echo "Error: No files provided." >&2
  echo "Usage: $0 srcfile" >&2
  exit 1
fi

rm -f perf.bin

src=$1

if grep -q "pragma omp" "$src"; then
  echo "Do not use OpenMP!"
  exit 1
fi

# echo "/usr/bin/clang++ -O3 -march=native -DNDEBUG=1 perf.cc $src -o perf.bin"

/usr/bin/clang++ -O3 -march=native -DNDEBUG=1 ./perf.cc "$src" -o perf.bin

success="$?"
if [ $success -gt 0 ]; then
  exit $success
fi

./perf.bin

exit "$?"
