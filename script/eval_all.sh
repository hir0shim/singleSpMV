#!/bin/bash -x
script_dir=$(cd $(dirname $BASH_SOURCE); pwd)
$script_dir/eval_cpu.sh
$script_dir/eval_mic.sh
$script_dir/eval_gpu.sh
