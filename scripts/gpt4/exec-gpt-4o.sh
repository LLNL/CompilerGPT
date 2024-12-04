#!/usr/bin/env bash

set -e

if [[ -n "${OPENAI_ENV_DIR}" ]]; then
  source "$OPENAI_ENV_DIR/bin/activate"
fi

dir=`dirname $0`

# On Linux system use readlink
query=`readlink -e $dir/query-gpt-4o.py`

# On BSD systems use realpath
# query=`realpath $dir/query-gpt-4o.py`

echo "$query"
python3 "$query"

