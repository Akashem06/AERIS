#!/bin/bash

FILES=$(find . -name "*.c" -o -name "*.h")

format_check=0

for file in $FILES; do
  if ! clang-format "$file" | diff "$file" - >/dev/null
  then
    format_check=1
    echo "Error: $file needs formatting"
  fi
done

if [ "$format_check" -eq 1 ]
then
  exit 1
fi