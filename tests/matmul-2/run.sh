
srcfile=simplematrix.cc

for i in $(seq 1 1); do 
  ../../compgpt.bin --config=clang-openai-default.json --csvsummary=clang-openai-default.csv $srcfile >log.txt 2>&1
  ./save-experiment.sh clang-openai-default.$i
  ./clean.sh

  ../../compgpt.bin --config=clang-claude-default.json --csvsummary=clang-claude-default.csv $srcfile >log.txt 2>&1
  ./save-experiment.sh clang-claude-default.$i
  ./clean.sh

  ../../compgpt.bin --config=gcc-openai-default.json --csvsummary=gcc-openai-default.csv $srcfile >log.txt 2>&1
  ./save-experiment.sh gcc-openai-default.$i
  ./clean.sh

  ../../compgpt.bin --config=gcc-claude-default.json --csvsummary=gcc-claude-default.csv $srcfile >log.txt 2>&1
  ./save-experiment.sh gcc-claude-default.$i
  ./clean.sh
done
