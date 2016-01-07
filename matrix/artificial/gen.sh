#!/bin/bash -x
pref=""
sizes=(512 1024 2048 4096 8192 16384 32768)
for i in ${sizes[@]}
do
    ./generator band $i > ${pref}band-$i.mtx
done
for i in ${sizes[@]}
do
    ./generator unbalance $i > ${pref}unbalance-$i.mtx
done
for i in ${sizes[@]}
do
    ./generator random $i > ${pref}random-$i.mtx
done
sizes=(512 1024 2048 4096)
for i in ${sizes[@]}
do
    ./generator dense $i > dense-$i.mtx
done
