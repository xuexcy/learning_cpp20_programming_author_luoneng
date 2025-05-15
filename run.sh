#!/bin/bash
########################################################################
#
# Copyright (c) 2024 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author  :   xuechengyun
# E-mail  :   xuechengyunxue@gmail.com
# Date    :   2024/10/29 22:22:06
# Desc    :   执行所有目标文件，执行输出结果到 ./stdout 目录下
########################################################################

# set -x
CUR_DIR=$(cd `dirname $0`; pwd)
cd ${CUR_DIR}
source ./env_variable.sh

cd $build_output_bin_dir

stdout_dir=${CUR_DIR}/stdout
echo 'stdout_dir:' $stdout_dir
mkdir -p $stdout_dir
for file in *
do
    output=$stdout_dir/$file.txt
    echo 'start' $file
    ./$file > $output
    if [ $? -ne 0 ]; then
        echo $file "execute failed"
        exit 1
    fi
    echo 'end ' $file
done

cd -


