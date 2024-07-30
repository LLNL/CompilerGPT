#!/usr/bin/env bash

for testfile in ./Clover*dfy; do
  echo "$testfile"
  ../../optai.bin --config=dfy-ada.json $testfile
  mv query.json "$testfile.json"
done

