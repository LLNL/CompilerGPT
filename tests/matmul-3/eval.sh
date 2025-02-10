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
  echo "Do not use OpenMP!" >&2
  exit 1
fi

echo "$comp $@ perf.cc $src -o perf.bin"

$comp "$@" ./perf.cc "$src" -o perf.bin >perfout.txt 2>perferr.txt 

cat perfout.txt
cat perferr.txt >&2

if grep -q "no member named" perferr.txt; then
  echo "SimpleMatrix has the following member functions:" >&2
  echo "SimpleMatrix::value_type SimpleMatrix::operator()(int row, int col) const" >&2
  echo "SimpleMatrix::value_type& SimpleMatrix::operator()(int row, int col)" >&2
  echo "int SimpleMatrix::rows() const" >&2
  echo "int SimpleMatrix::columns() const" >&2
  exit 1
fi

success="$?"
if [ $success -gt 0 ]; then
  exit $success
fi

./perf.bin

exit "$?"

