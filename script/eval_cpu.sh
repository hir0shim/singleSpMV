#!/bin/bash 
script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
source $script_dir/env.sh 
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`

### CPU (NOEL) ###
cd $script_dir/../
srun -p NOEL make clean 
srun -p NOEL make bin/spmv.cpu
export OMP_NUM_THREADS=24
logfile=$LOG_DIR/cpu-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
for matrix in $matrices
do
    echo "CPU $matrix"
    srun -p NOEL $BINARY_DIR/spmv.cpu $MATRIX_DIR/$matrix >> $logfile
done
