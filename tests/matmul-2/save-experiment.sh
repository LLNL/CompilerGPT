#!/usr/bin/env bash

location="$1"

mkdir -p "$location"

cp simplematrix[0-9]*.cc "$location"/
cp simplematrix.cc "$location"/
cp simplematrix.h "$location"/
cp "eval.sh" "$location"/
cp "perf.cc" "$location"/
cp "query.json" "$location"/
cp log.txt "$location"/

