#!/usr/bin/env bash

location="$1"

mkdir -p "$location"

mv bt[0-9].c "$location"/
cp "perftest.sh" "$location"/
mv nohup.out "$location"/log.txt
mv query.json "$location"/query.json


