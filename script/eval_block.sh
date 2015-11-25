#!/bin/bash 
script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
source $script_dir/env.sh 
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`

### MIC (KAREN) ###
export MIC_OMP_NUM_THREADS=240
cd $script_dir/../
logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
for ((i=1; i <= 100000; i*=4)) do
    echo "MIC $i"
    srun -p KAREN make clean
    srun -p KAREN make bin/spmv.mic OPTION=-DN_BLOCK=$i
    srun -p KAREN mpirun-mic -m "$BINARY_DIR/spmv.mic $MATRIX_DIR/cage15.mtx" >> $logfile
done
