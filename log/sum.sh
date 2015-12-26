
#!/bin/bash

logs=`ls log/*.tsv`
for log in $logs
do
    sum=`cat $log | grep "Performance(GFLOPS)" | awk 'BEGIN{sum=0}{sum+=$2}END{print sum}'`
    echo $sum $log
done
