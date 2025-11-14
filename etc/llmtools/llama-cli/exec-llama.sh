#!/usr/bin/env bash

# NOTE: llama-cli needs to be in the path.

set -e

# set huggingface' gemma-3.1b as the default model
model="-hf ggml-org/gemma-3-1b-it-GGUF"

if [ -n "$1" ]; then
  model="$@"
fi

# ideally we would write the llama-cli response to a file that can be further processed directly.
# Since this does not seem to be supported as of Aug 2025, we write all stdio output to a file, then
# extract the response from the log in the driver.

llama-cli $model --simple-io -no-cnv --chat-template command-r -sysf system.txt -f query.txt >response.log
