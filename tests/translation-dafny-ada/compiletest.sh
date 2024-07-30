#!/usr/bin/env bash

set -e

if [[ $# -eq 0 ]]; then
  echo "Error: No files provided." >&2
  echo "Usage: $0 srcfile" >&2
  exit 1
fi

src=$1

echo "gcc -c -w $src"

gcc -c -w "$src"

echo "1"
exit 0
