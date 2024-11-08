#!/usr/bin/env bash

set -e

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
echo "  \"model\": \"claude-3-5-sonnet-20241022\"," >>q.json
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
