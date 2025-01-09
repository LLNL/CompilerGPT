#!/usr/bin/env bash

location="$1"

mkdir -p "$location"

mv core[0-9]*.cc "$location"/
cp core.cc "$location"/
cp constants.h "$location"/
cp eval.sh "$location"/
cp perf.cc "$location"/
mv nohup.out "$location"/log.txt
mv query.json "$location"/query.json

