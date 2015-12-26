#!/bin/bash

for (( i=1; i <= 4096; i*=4 ))
do
    echo "SIMPLE 32 WIDTH=$i"
    log/format log/mic-ss-simple-width$i.tsv
    echo ""
done
