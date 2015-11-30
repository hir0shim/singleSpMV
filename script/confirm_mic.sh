#!/bin/bash 
script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
source $script_dir/env.sh 
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`

### MIC (KAREN) ###
cd $script_dir/../
srun -p KAREN make clean
srun -p KAREN make mic
export MIC_OMP_NUM_THREADS=240
logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
for matrix in $matrices
do
    echo "MIC $matrix"
    srun -p KAREN mpirun-mic -m "$BINARY_DIR/test-spmv.mic $MATRIX_DIR/$matrix" >> $logfile
done
