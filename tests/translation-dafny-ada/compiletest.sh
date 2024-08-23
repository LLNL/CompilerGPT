#!/usr/bin/env bash

set -e

if [[ $# -eq 0 ]]; then
  echo "Error: No files provided." >&2
  echo "Usage: $0 srcfile" >&2
  exit 1
fi

src=$1


if grep -iq "with arrays" "$src"; then
  echo ""
else
  >&2 echo "Import package arrays using the following code: \"with arrays; use arrays;\""
  exit 1
fi

if grep -iq "package " "$src"; then
  >&2 echo "Do not generate a package. Generate a subprogram unit."
  exit 1
fi

if grep -iq "Ada.Containers" "$src"; then
  >&2 echo "Do not use Ada.Containers. Use package arrays and type Integer_Array."
  exit 1
fi

if grep -iq "text_io " "$src"; then
  >&2 echo "Do not include Text_IO."
  exit 1
fi


#~ sed -i.old '1s/^/with arrays; use arrays;/' "$src"

echo "gcc -c -w $src"

gcc -c -w "$src"

echo "1"
exit 0
