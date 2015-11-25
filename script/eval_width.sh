#!/bin/bash 
script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
source $script_dir/env.sh 
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`
cd $script_dir/../
for ((i=1; i <= 4096; i*=4)) do
    make clean
    # --- CPU ---
    srun -p NOEL make bin/spmv.cpu OPTION="-DCOEF_SEGMENT_WIDTH=$i"
    #srun -p NOEL make bin/spmv.cpu OPTION="-DCOEF_SEGMENT_WIDTH=$i -DPADDING -DCOEF_PADDING_SIZE=1"
    logfile=$LOG_DIR/cpu-`date +%y-%m-%d-%H-%M`-wcoef-$i.tsv && echo "" > $logfile
    export OMP_NUM_THREADS=24
    for matrix in $matrices
    do
        echo "CPU $matrix"
        srun -p NOEL $BINARY_DIR/spmv.cpu $MATRIX_DIR/$matrix >> $logfile
    done
    # --- MIC ---
    srun -p KAREN make bin/spmv.mic OPTION="-DCOEF_SEGMENT_WIDTH=$i"
    #srun -p KAREN make bin/spmv.mic OPTION="-DCOEF_SEGMENT_WIDTH=$i -DPADDING -DCOEF_PADDING_SIZE=1"
    logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`-wcoef-$i.tsv && echo "" > $logfile
    export MIC_OMP_NUM_THREADS=240
    for matrix in $matrices
    do
        echo "MIC $matrix"
        srun -p KAREN mpirun-mic -m "$BINARY_DIR/spmv.mic $MATRIX_DIR/$matrix" >> $logfile
    done
done
