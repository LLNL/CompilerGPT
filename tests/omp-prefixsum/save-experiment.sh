#!/usr/bin/env bash

location="$1"

mkdir -p "$location"

mv utility[0-9].cc "$location"/
cp utility.cc "$location"/
cp utility.h "$location"/
cp "eval.sh" "$location"/
cp "perf.cc" "$location"/
cp "query.json" "$location"/
mv nohup.out "$location"/log.txt

