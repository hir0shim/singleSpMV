script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
todo=`cat $script_dir/todo.csv`
source $script_dir/env.sh
matrices=`ls $MATRIX_DIR/*.mtx | xargs -i basename {}`
cd $script_dir/../
IFS=$'\n'
for i in $todo
do
    arch=`echo $i | cut -d, -f1`
    prefix=`echo $i | cut -d, -f2`
    option=`echo $i | cut -d, -f3`

    logfile=$LOG_DIR/$arch-$prefix.tsv
    params="arch=$arch prefix=$prefix option=$option logfile=$logfile"
    if [ -f $logfile ]; then
        echo -e "Skip : \n\t $params";
        continue
    fi
    touch $logfile
    logfile_tmp=$logfile-$RANDOM
    touch $logfile_tmp
    echo -e "Do : \n\t $params"

    ### CPU ### 
    if [ $arch = "cpu" ]; then
        make cpu PREFIX=$prefix OPTION=$option
        export OMP_NUM_THREADS=24
        for matrix in $matrices
        do
            echo "CPU $matrix"
            srun -p NOEL $BINARY_DIR/$prefix-spmv.cpu $MATRIX_DIR/$matrix >> $logfile_tmp
        done
    ### MIC ### 
    elif [ $arch = "mic" ]; then
        make mic PREFIX=$prefix OPTION=$option
        export MIC_OMP_NUM_THREADS=240
        for matrix in $matrices
        do
            echo "MIC $matrix"
            srun -p KAREN mpirun-mic -m "$BINARY_DIR/$prefix-spmv.mic $MATRIX_DIR/$matrix" >> $logfile_tmp
        done
    else 
        echo "Invalid arch : $arch"
        exit 1
    fi

    mv $logfile_tmp $logfile
done


