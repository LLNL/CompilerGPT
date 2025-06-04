src=ft.c
kernel="472-804"

./clean.sh

for i in $(seq 6 6); do 
  testbase="clang-openai-default"
  echo "$testbase.$i"
  time ../../../compgpt.bin --config="$testbase.json" --kernel=$kernel --csvsummary="$testbase.csv" $src >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  echo "$testbase.$i"
  testbase="clang-claude-default"
  time ../../../compgpt.bin --config="$testbase.json" --kernel=$kernel --csvsummary="$testbase.csv" $src >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  echo "$testbase.$i"
  testbase="gcc-openai-default"
  time ../../../compgpt.bin --config="$testbase.json" --kernel=$kernel --csvsummary="$testbase.csv" $src >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh

  echo "$testbase.$i"
  testbase="gcc-claude-default"
  time ../../../compgpt.bin --config="$testbase.json" --kernel=$kernel --csvsummary="$testbase.csv" $src >log.txt 2>&1
  ./save-experiment.sh "$testbase.$i"
  ./clean.sh
done
