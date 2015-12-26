#!/bin/bash
#SBATCH -p mixed
#SBATCH -N 1
#SBATCH -t 05:00:00
#SBATCH -o slurm.out
#SBATCH -e slurm.err
MATRIX_DIR=../../../matrix/boundary/
LOG_DIR=.
BINARY_DIR=.
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`
logfile=$LOG_DIR/mic-`date +%y-%m-%d-%H-%M`.tsv && echo "" > $logfile
export MIC_PPN=1
export I_MPI_MIC=enable
export OMP_NUM_THREADS=240
export KMP_AFFINITY=compact
module load intelmpi intel mkl
cd $SLURM_SUBMIT_DIR
for matrix in $matrices
do
    #micnativeloadex $BINARY_DIR/$prefix-spmv.mic -a $MATRIX_DIR/$matrix >> $logfile
    ./spmv $MATRIX_DIR/$matrix >> $logfile
done
