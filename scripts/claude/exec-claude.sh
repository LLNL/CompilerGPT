#!/usr/bin/env bash

set -e

if [[ ! -v ANTHROPIC_API_KEY ]]; then
  echo "Environment variable ANTHROPIC_API_KEY is not set." >&2
  echo "Exiting with error.." >&2
  exit 1
fi

# set claude-3-5-sonnet-20241022 as the default model
#   (set by the driver; default supports legacy configurations)
model="claude-3-5-sonnet-20241022"

if [ -n "$1" ]; then
  model="$1"
fi


numtokens=4096
maxtokens=8192

query=$(<query.json)

if [ -f "system.txt" ]; then
  system=$(<system.txt)
fi

if [[ $numtokens -lt ${#query} ]]; then
  numtokens=${#query}
fi

if [[ $maxtokens -lt $numtokens ]]; then
  # just issue warning because input file length does not
  #   translate directly into number of tokens.
  echo "WARNING: approaching token limit"
  numtokens=$maxtokens
fi

# write curl data file data

echo "{" >q.json
echo "  \"model\": \"$model\"," >>q.json
echo "  \"max_tokens\": $numtokens," >>q.json

if [[ 0 -lt ${#system} ]]; then
  echo "  \"system\": \"$system\"," >>q.json
else
  echo "No system message given."
fi

echo "  \"messages\": $query" >>q.json
echo "}" >>q.json

# curl invocation and response gathering

curl https://api.anthropic.com/v1/messages \
     --header "x-api-key: $ANTHROPIC_API_KEY" \
     --header "anthropic-version: 2023-06-01" \
     --header "content-type: application/json" \
     --data "@q.json" \
     -o response.json
