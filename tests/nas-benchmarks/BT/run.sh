src=bt.c
kernel="1778-2199"

./clean.sh

for i in $(seq 6 7); do 
  testbase="clang-openai-default"
  echo "$testbase.$i"
  time ../../../compgpt.bin --config="$testbase.json" --kernel=$kernel --csvsummary="$testbase.csv" $src >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  testbase="clang-claude-default"
  echo "$testbase.$i"
  time ../../../compgpt.bin --config="$testbase.json" --kernel=$kernel --csvsummary="$testbase.csv" $src >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  testbase="gcc-openai-default"
  echo "$testbase.$i"
  time ../../../compgpt.bin --config="$testbase.json" --kernel=$kernel --csvsummary="$testbase.csv" $src >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  testbase="gcc-claude-default"
  echo "$testbase.$i"
  time ../../../compgpt.bin --config="$testbase.json" --kernel=$kernel --csvsummary="$testbase.csv" $src >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh
done
