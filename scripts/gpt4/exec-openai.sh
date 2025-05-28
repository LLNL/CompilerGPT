#!/usr/bin/env bash

set -e

# set gpt4o as the default model 
#   (should be set from the model anyways)
model="gpt-4o"

if [ -n "$1" ]; then
  model="$1"
fi

query=$(<query.json)

#if [ -f "system.txt" ]; then
#  system=$(<system.txt)
#fi

# write curl data file data

echo "{" >q.json
echo "  \"model\": \"$model\"," >>q.json
echo "  \"stream\": false," >>q.json

# not sure how to set system with OpenAI
#   see also:
#   https://www.reddit.com/r/ollama/comments/1czw7mj/how_to_set_system_prompt_in_ollama/?rdt=42436
#if [[ 0 -lt ${#system} ]]; then
#  echo "  \"system\": \"$system\"," >>q.json
#else
#  echo "No system message given."
#fi

echo "  \"messages\": $query" >>q.json
echo "}" >>q.json

# curl invocation and response gathering
curl https://api.openai.com/v1/chat/completions \
     -H "Content-Type: application/json" \
     -H "Authorization: Bearer $OPENAI_API_KEY" \
     --data "@q.json" \
     -o response.json

