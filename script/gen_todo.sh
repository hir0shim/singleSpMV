#!/bin/bash

function f () {
    echo "mic,$1"
    #echo "cpu,$1"
}

echo "# CRS"
f "crs,-DOPT_CRS"

echo "# MKL"
f "mkl,-DOPT_MKL"

echo "# SS-SIMPLE"
for ((i=1; i<=256; i*=2))
do
    f "ss-simple-width=$i,-DOPT_SS -DSIMPLE -DSEGMENT_WIDTH=$i -DPROFILING"
done

echo "# SS-OPT"
for ((i=1; i<=256; i*=2))
do
    f "ss-opt-width=$i,-DOPT_SS -DOPTIMIZED -DSEGMENT_WIDTH=$i -DMEASURE_STEP_TIME -DPROFILING"
done

#echo "# SS-SIMPLE-PADDING"
#for ((i=1; i<=256; i*=2))
#do
#    for j in 1 2 4 8
#    do
#        f "ss-simple-width=$i-pad=$j,-DOPT_SS -DSIMPLE -DPADDING -DPADDING_SIZE=$j -DSEGMENT_WIDTH=$i"
#    done
#done
#echo "# SS-OPT-PADDING"
#for ((i=1; i<=256; i*=2))
#do
#    for j in 1 2 4 8
#    do
#        f "ss-opt-width=$i-pad=$j,-DOPT_SS -DOPTIMIZED -DPADDING -DPADDING_SIZE=$j -DSEGMENT_WIDTH=$i -DMEASURE_STEP_TIME"
#    done
#done

echo "# CSS"
for ((i=1; i<=256; i*=2))
do
    f "css-opt-nblock=$i,-DOPT_CSS -DOPTIMIZED -DN_BLOCK=$i -DPROFILING"
done
