#!/bin/bash 

script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
source $script_dir/env.sh 
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`

### CPU (NOEL) ###
export OMP_NUM_THREADS=24
logfile=$LOG_DIR/cpu-`date +%y-%m-%d`.tsv && echo "" > $logfile
for matrix in $matrices
do
    echo "CPU $matrix"
    srun -p NOEL $BINARY_DIR/spmv.cpu $MATRIX_DIR/$matrix >> $logfile
done

### MIC (KAREN) ###
export OMP_NUM_THREADS=240
logfile=$LOG_DIR/mic-`date +%y-%m-%d`.tsv && echo "" > $logfile
for matrix in $matrices
do
    echo "MIC $matrix"
    srun -p KAREN mpirun-mic -m "$BINARY_DIR/spmv.mic $MATRIX_DIR/$matrix" >> $logfile
done

