script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
todo=`cat $script_dir/todo.csv | grep -v -e '^\s*#' -e '^\s*$'`
source $script_dir/env.sh
cd $script_dir/../
IFS=$'\n'
for i in $todo
do
    arch=`echo $i | cut -d, -f1`
    prefix=`echo $i | cut -d, -f2`
    option=`echo $i | cut -d, -f3`

    logfile=$LOG_DIR/$arch-$prefix.tsv
    params="arch=$arch prefix=$prefix option=$option logfile=$logfile"
    ln -s $logfile $logfile.lock
    if [ $? -eq 1 ]; then
        echo -e "Skip : \n\t $params";
        continue
    fi
    touch $logfile
    echo -e "Do : \n\t $params"
    batch_script=$script_dir/batch/$arch-$prefix.sh
    ### CPU ### 
    if [ $arch = "cpu" ]; then
        make cpu PREFIX=$prefix OPTION=$option
        echo "#!/bin/sh
#SBATCH -p mixed
#SBATCH -N 1
#SBATCH -o $logfile.out
#SBATCH -e $logfile.err
cd $script_dir/../
matrices=\`ls $MATRIX_DIR/*.mtx | xargs -i basename {}\`
export OMP_NUM_THREADS=24
for matrix in \$matrices
do
    $BINARY_DIR/$prefix-spmv.cpu $MATRIX_DIR/\$matrix >> $logfile
done " > $batch_script
        sbatch $batch_script
        
    ### MIC ### 
    elif [ $arch = "mic" ]; then
        make mic PREFIX=$prefix OPTION=$option
    echo "#!/bin/sh
#SBATCH -p mixed
#SBATCH -N 1
#SBATCH -o $logfile.out
#SBATCH -e $logfile.err
cd $script_dir/../
matrices=\`ls $MATRIX_DIR/*.mtx | xargs -i basename {}\`
export MIC_PPN=1
export I_MPI_MIC=enable
export OMP_NUM_THREADS=240
#export SINK_LD_LIBRARY_PATH=/app/intel/composerxe/lib/mic
for matrix in \$matrices
do
    #micnativeloadex $BINARY_DIR/$prefix-spmv.mic -a $MATRIX_DIR/\$matrix >> $logfile
    /opt/slurm/default/local/bin/mpirun-mic -m \"$BINARY_DIR/$prefix-spmv.mic $MATRIX_DIR/\$matrix\" >> $logfile

done
    " > $batch_script
        sbatch $batch_script
    else 
        echo "Invalid arch : $arch"
        exit 1
    fi
done


