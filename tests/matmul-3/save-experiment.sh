#!/usr/bin/env bash

location="$1"

mkdir -p "$location"

mv s[0-9].cc "$location"/
cp s.cc "$location"/
cp s.h "$location"/
cp "eval.sh" "$location"/
cp "perf.cc" "$location"/
cp "query.json" "$location"/
mv nohup.out "$location"/log.txt

