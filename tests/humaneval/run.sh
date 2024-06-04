#!/usr/bin/env bash

if [ "$#" -ne 3 ]; then
    echo "Need two parameters: kernel.cc harness.cc"
    exit 1
fi

kernel=$1
harness=$2

cat "$kernel" "$harness" >code.cc

clang-15 -O3 -march=native code.cc -o code.bin

./code.bin
res=$?

echo -n "0.0"

exit res

