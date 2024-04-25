#!/bin/bash
CFILES=$(find . -name "*.c" -o -name "*.h" -not -path "./cpputest/*")
PYFILES=$(find . -name "*.py" -not -path "./cpputest/*")

for file in $CFILES; do
    clang-format -i "$file"
done

for file in $PYFILES; do
    pylint --rcfile=.pylintrc "$file"
done