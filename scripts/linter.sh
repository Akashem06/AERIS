#!/bin/bash

mkdir -p cppcheckbuild

FILES=$(find . -name "*.c" -o -name "*.h" | grep -v "build/")

SUPPRESSIONS="--suppress=missingIncludeSystem --suppress=unusedFunction --suppress=missingInclude --suppress=unmatchedSuppression"

HEADER_FILES=$(echo "$FILES" | grep -E "\.h$")

HEADER_FILES=$(echo "$HEADER_FILES" | xargs -n1 dirname | sort | uniq | sed -e 's/^/-I/')

BASE_ARGS="--enable=all --template=gcc --error-exitcode=1 --cppcheck-build-dir=cppcheckbuild -q --inline-suppr"

cppcheck $FILES $SUPPRESSIONS $BASE_ARGS $HEADER_FILES

if [ $? -eq 0 ]; then
  echo "Cppcheck was successful"
else
  echo "Cppcheck encountered errors or warnings"
  exit 1
fi
