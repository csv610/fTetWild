#!/bin/bash
set -e
BIN=$1
INPUT=$2
OUTPUT="test_output.msh"
$BIN -i $INPUT -o $OUTPUT --is-quiet
if [ -s "$OUTPUT" ]; then
    echo "Functional test PASSED"
    rm "$OUTPUT"
else
    echo "Functional test FAILED"
    exit 1
fi
