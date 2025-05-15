#!/bin/bash
########################################################################
#
# Copyright (c) 2024 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author  :   xuechengyun
# E-mail  :   xuechengyunxue@gmail.com
# Date    :   2024/10/30 15:02:15
# Desc    :   执行一个目标文件，主要用于自己写代码时运行测试用，程序输出结果到标准输出
########################################################################

# set -x
CUR_DIR=$(cd `dirname $0`; pwd)
cd ${CUR_DIR}

source ./env_variable.sh

cd $build_output_bin_dir

if [ $# -gt 0 ]; then
    ./$1
else
    ./$default_execute_file
fi









cd -


