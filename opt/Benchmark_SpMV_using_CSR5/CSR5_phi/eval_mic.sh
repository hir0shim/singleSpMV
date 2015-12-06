#!/bin/bash 
MATRIX_DIR=../../../matrix/artificial/
LOG_DIR=.
BINARY_DIR=.

matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`
logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile

export OMP_NUM_THREADS=240
for matrix in $matrices
do
    echo "CPU $matrix"
    srun -p KAREN $BINARY_DIR/spmv $MATRIX_DIR/$matrix >> $logfile
done



