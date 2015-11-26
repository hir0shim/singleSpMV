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
    ### CPU ### 
    if [ $arch = "cpu" ]; then
        make cpu PREFIX=$prefix OPTION=$option
        
    ### MIC ### 
    elif [ $arch = "mic" ]; then
        make mic PREFIX=$prefix OPTION=$option
    fi
done


