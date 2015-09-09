#!/bin/bash 
script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
source $script_dir/env.sh 
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`

### GPU (FATE) ###
cd $script_dir/../
srun -p FATE make clean 
srun -p FATE make bin/spmv.gpu
export OMP_NUM_THREADS=4
logfile=$LOG_DIR/gpu-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
for matrix in $matrices
do
    echo "GPU $matrix"
    srun -p FATE $BINARY_DIR/spmv.gpu $MATRIX_DIR/$matrix >> $logfile
done
