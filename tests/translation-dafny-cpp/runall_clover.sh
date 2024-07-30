#!/usr/bin/env bash

for testfile in ./Clover*dfy; do
  echo "$testfile"
  ../../optai.bin --config=dfy-cpp.json $testfile
  mv query.json "$testfile.json"
done

