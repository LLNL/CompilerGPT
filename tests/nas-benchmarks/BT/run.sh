

for i in $(seq 1 6); do 
  nohup ../../../compgpt.bin --config=gpt4o_guided.json --kernel=1775-2195 --csvsummary=gpt4o_guided.csv bt.c
  ./save-experiment.sh gpt4o.guided.$i
  ./clean.sh

  nohup ../../../compgpt.bin --config=gpt4o_unguided.json --kernel=1775-2195 --csvsummary=gpt4o_unguided.csv bt.c
  ./save-experiment.sh gpt4o.unguided.$i
  ./clean.sh

  nohup ../../../compgpt.bin --config=claude_guided.json --kernel=1775-2195 --csvsummary=claude_guided.csv bt.c
  ./save-experiment.sh claude.guided.$i
  ./clean.sh

  nohup ../../../compgpt.bin --config=claude_unguided.json --kernel=1775-2195 --csvsummary=claude_unguided.csv bt.c
  ./save-experiment.sh claude.unguided.$i
  ./clean.sh
done
