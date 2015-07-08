#!/bin/bash -x

sizes=(512 1024 2048 4096 8192 16384)
for i in ${sizes[@]}
do
    ./generator band $i $(($i/100)) > band-$i.mtx
done
sizes=(512 1024 2048 4096 8192 16384)
for i in ${sizes[@]}
do
    ./generator unbalance $i > unbalance-$i.mtx
done
sizes=(512 1024 2048 4096)
for i in ${sizes[@]}
do
    ./generator dense $i > dense-$i.mtx
done
sizes=(512 1024 2048 4096 8192 16384)
for i in ${sizes[@]}
do
    ./generator random $i $i > random-$i.mtx
done
