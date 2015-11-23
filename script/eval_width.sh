#!/bin/bash 
script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
source $script_dir/env.sh 
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`

### MIC (KAREN) ###
export MIC_OMP_NUM_THREADS=240
cd $script_dir/../
for ((i=1; i <= 4096; i*=4)) do
    make clean
    srun -p NOEL make bin/spmv.cpu CMDLINE_OPTION=-DCOEF_SEGMENT_WIDTH=$i
    logfile=$LOG_DIR/cpu-`date +%y-%m-%d-%H-%M`-coef-$i.tsv && echo "" > $logfile
    for matrix in $matrices
    do
        echo "CPU $matrix"
        srun -p NOEL $BINARY_DIR/spmv.cpu $MATRIX_DIR/$matrix >> $logfile
    done
    logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`-coef-$i.tsv && echo "" > $logfile
    srun -p KAREN make bin/spmv.mic CMDLINE_OPTION=-DCOEF_SEGMENT_WIDTH=$i
    for matrix in $matrices
    do
        echo "MIC $matrix"
        srun -p KAREN mpirun-mic -m "$BINARY_DIR/spmv.mic $MATRIX_DIR/$matrix" >> $logfile
    done
done
