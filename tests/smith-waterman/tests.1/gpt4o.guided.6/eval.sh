#!/usr/bin/env bash

set -e

if [[ $# -eq 0 ]]; then
  echo "Error: No files provided." >&2
  echo "Usage: $0 srcfile" >&2
  exit 1
fi

src=$1

echo "/usr/bin/clang++ -O3 -march=native -DNDEBUG=1 perf.cc $src -o perf.bin"

/usr/bin/clang++ -O3 -march=native -DNDEBUG=1 ./perf.cc "$src" -o perf.bin

./perf.bin

exit 0
