#!/usr/bin/env bash

location="$1"

mkdir -p "$location"

cp utility[0-9]*.cc "$location"/
cp utility.cc "$location"/
cp utility.h "$location"/
cp "eval.sh" "$location"/
cp "perf.cc" "$location"/
cp "query.json" "$location"/
cp log.txt "$location"/log.txt

