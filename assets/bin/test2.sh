#!/bin/bash

for i in {1..10}; do
    mkdir -p test
    eval echo {1..$i} > test/file_$i.txt
done