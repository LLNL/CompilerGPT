
srcfile=simplematrix.cc

for i in $(seq 1 6); do 
  nohup ../../compgpt.bin --config=gpt4o_guided.json --csvsummary=got4o_guided.csv $srcfile
  ./save-experiment.sh gpt4o.guided.$i
  ./clean.sh

  nohup ../../compgpt.bin --config=gpt4o_unguided.json --csvsummary=got4o_unguided.csv $srcfile
  ./save-experiment.sh gpt4o.unguided.$i
  ./clean.sh

  nohup ../../compgpt.bin --config=claude_guided.json --csvsummary=claude_guided.csv $srcfile
  ./save-experiment.sh claude.guided.$i
  ./clean.sh

  nohup ../../compgpt.bin --config=claude_unguided.json --csvsummary=claude_unguided.csv $srcfile
  ./save-experiment.sh claude.unguided.$i
  ./clean.sh
done
