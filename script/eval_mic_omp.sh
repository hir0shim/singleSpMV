#!/bin/bash 
script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
source $script_dir/env.sh 
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`

### MIC (KAREN) ###
cd $script_dir/../
srun -p KAREN make clean
srun -p KAREN make bin/spmv.mic
export MIC_OMP_NUM_THREADS=1
logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
echo "thread $MIC_OMP_NUM_THREADS" >> $logfile
for matrix in $matrices
do
    echo "MIC $matrix"
    srun -p KAREN mpirun-mic -m "$BINARY_DIR/spmv.mic $MATRIX_DIR/$matrix" >> $logfile
done




export MIC_OMP_NUM_THREADS=60
logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
echo "thread $MIC_OMP_NUM_THREADS" >> $logfile
for matrix in $matrices
do
    echo "MIC $matrix"
    srun -p KAREN mpirun-mic -m "$BINARY_DIR/spmv.mic $MATRIX_DIR/$matrix" >> $logfile
done



export MIC_OMP_NUM_THREADS=120
logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
echo "thread $MIC_OMP_NUM_THREADS" >> $logfile
for matrix in $matrices
do
    echo "MIC $matrix"
    srun -p KAREN mpirun-mic -m "$BINARY_DIR/spmv.mic $MATRIX_DIR/$matrix" >> $logfile
done



export MIC_OMP_NUM_THREADS=240
echo "thread $MIC_OMP_NUM_THREADS" >> $logfile
logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
for matrix in $matrices
do
    echo "MIC $matrix"
    srun -p KAREN mpirun-mic -m "$BINARY_DIR/spmv.mic $MATRIX_DIR/$matrix" >> $logfile
done
