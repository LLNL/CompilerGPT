#!/usr/bin/env bash

set -e

# set llama3.1 as the default model
model="llama3.1"

if [ -n "$1" ]; then
  model="$1"
fi

query=$(<query.json)

if [ -f "system.txt" ]; then
  system=$(<system.txt)
fi


# write curl data file data

echo "{" >q.json
echo "  \"model\": \"$model\"," >>q.json
echo "  \"stream\": false," >>q.json

# not sure how to set system in ollama
#   see also:
#   https://www.reddit.com/r/ollama/comments/1czw7mj/how_to_set_system_prompt_in_ollama/?rdt=42436
if [[ 0 -lt ${#system} ]]; then
  echo "  \"system\": \"$system\"," >>q.json
else
  echo "No system message given."
fi

echo "  \"messages\": $query" >>q.json
echo "}" >>q.json

# curl invocation and response gathering
curl http://localhost:11434/api/chat \
     -H "Content-Type: application/json" \
     --data "@q.json" \
     -o response.json

