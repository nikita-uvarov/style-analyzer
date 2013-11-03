#!/bin/bash

# Invokes ./check-source-file-consistency.sh on every source file in project

pushd $(dirname $(readlink -f $0)) >/dev/null

ERRORS=0

for file in ../src/*; do
    output=$(./check-source-file-consistency.sh "$file")
    if ! [[ $? -eq 0 ]]; then
        echo -e "\e[31mIn file $file:\e[0m"
        echo "$output"
        echo
        ERRORS=1
    fi
done;

popd >/dev/null
exit $ERRORS
