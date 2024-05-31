#!/usr/bin/env bash

if [[ -n "${OPENAI_ENV_DIR}" ]]; then
  openaidir="${OPENAI_ENV_DIR}"

  source $(openaidir)/bin/activate 
fi

python3 ./openaiquery.py

