#!/usr/bin/env bash

if [[ -n "${OPENAI_ENV_DIR}" ]]; then
  source "$OPENAI_ENV_DIR/bin/activate"
fi

dir=`dirname $0`

query=`realpath $dir/openaiquery.py`

echo "$query"
python3 "$query"

