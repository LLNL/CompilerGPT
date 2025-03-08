
srcfile="utility.cc"

for i in $(seq 2 5); do
  testbase="clang-openai-default"
  time ../../compgpt.bin --config="$testbase.json" --csvsummary="$testbase.csv" $srcfile >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  testbase="clang-claude-default"
  time ../../compgpt.bin --config="$testbase.json" --csvsummary="$testbase.csv" $srcfile >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  testbase="gcc-openai-default"
  time ../../compgpt.bin --config="$testbase.json" --csvsummary="$testbase.csv" $srcfile >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  testbase="gcc-claude-default"
  time ../../compgpt.bin --config="$testbase.json" --csvsummary="$testbase.csv" $srcfile >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh
done
