#!/bin/bash


echo "# CRS"
echo "mic,crs,-DOPT_CRS"
echo "cpu,crs,-DOPT_CRS"

echo "# MKL"
echo "mic,mkl,-DOPT_MKL"
echo "cpu,mkl,-DOPT_MKL"

echo "# SS-SIMPLE"
for ((i=1; i<=256; i*=2))
do
    echo "mic,ss-simple-width=$i,-DOPT_SS -DSIMPLE -DCOEF_SEGMENT_WIDTH=$i"
    echo "cpu,ss-simple-width=$i,-DOPT_SS -DSIMPLE -DCOEF_SEGMENT_WIDTH=$i"
done

echo "# SS-OPT"
for ((i=1; i<=256; i*=2))
do
    echo "mic,ss-opt-width=$i,-DOPT_SS -DOPTIMIZED -DCOEF_SEGMENT_WIDTH=$i"
    echo "cpu,ss-opt-width=$i,-DOPT_SS -DOPTIMIZED -DCOEF_SEGMENT_WIDTH=$i"
done

echo "# SS-OPT-PADDING"
for ((i=1; i<=256; i*=2))
do
    echo "mic,ss-opt-width=$i-pad=1,-DOPT_SS -DOPTIMIZED -DPADDING -DCOEF_PADDING_SIZE=1 -DCOEF_SEGMENT_WIDTH=$i"
    echo "cpu,ss-opt-width=$i-pad=1,-DOPT_SS -DOPTIMIZED -DPADDING -DCOEF_PADDING_SIZE=1 -DCOEF_SEGMENT_WIDTH=$i"
done

echo "# CSS"
for ((i=1; i<=256; i*=2))
do
    echo "mic,css-opt-nblock=$i,-DOPT_CSS -DOPTIMIZED -DN_BLOCK=$i -DPROFILING"
    echo "cpu,css-opt-nblock=$i,-DOPT_CSS -DOPTIMIZED -DN_BLOCK=$i -DPROFILING"
done


OPTION1="-ansi-alias -opt-streaming-cache-evict=3 -opt-prefetch=3 -opt-prefetch-distance=64,8 -opt-streaming-stores always"
echo "# SS-OPT-OPTION1"
for ((i=1; i<=256; i*=2))
do
    echo "mic,ss-opt-width=$i-option1,-DOPT_SS -DOPTIMIZED -DCOEF_SEGMENT_WIDTH=$i $OPTION1"
    echo "cpu,ss-opt-width=$i-option1,-DOPT_SS -DOPTIMIZED -DCOEF_SEGMENT_WIDTH=$i $OPTION1"
done

