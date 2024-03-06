#!/bin/bash
CFILES=$(find . -name "*.c" -o -name "*.h")
PYFILES=$(find . -name "*.py")

for file in $CFILES; do
    clang-format -i "$file"
done

for file in $PYFILES; do
    pylint --rcfile=.pylintrc "$file"
done