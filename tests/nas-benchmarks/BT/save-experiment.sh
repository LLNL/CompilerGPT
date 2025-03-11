#!/usr/bin/env bash

location="$1"

mkdir -p "$location"

cp bt.c "$location"/
cp bt[0-9]*.c "$location"/
cp eval.sh "$location"/
cp log.txt "$location"/
cp query.json "$location"/


