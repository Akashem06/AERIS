#!/bin/bash
FILES=$(find . -name "*.c" -o -name "*.h")

for file in $FILES; do
    clang-format -i "$file"
done