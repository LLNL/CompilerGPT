#!/usr/bin/env bash

location="$1"

mkdir -p "$location"

mv simplematrix[0-9].cc "$location"/
cp simplematrix.cc "$location"/
cp simplematrix.h "$location"/
cp "eval.sh" "$location"/
cp "perf.cc" "$location"/
mv nohup.out "$location"/log.txt

