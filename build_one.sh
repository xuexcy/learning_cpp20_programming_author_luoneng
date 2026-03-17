#!/bin/bash
########################################################################
#
# Copyright (c) 2024 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author  :   xuechengyun
# E-mail  :   xuechengyun@gmail.com
# Date    :   2024/11/06 16:57:53
# Desc    :   编译一个目标文件，主要用于自己写代码时运行测试用
########################################################################

# set -x
CUR_DIR=$(cd `dirname $0`; pwd)
cd ${CUR_DIR}

source ./env_variable.sh


# cmake ..

main=`basename $default_execute_file`
if [ $# -gt 0 ]; then
    main=$1
fi

cd $build_dir
if [[ ${use_cpp_modules} -eq 1 ]]; then
    ninja -j${cpu_core_num} $main
else
    make -j${cpu_core_num} $main #VERBOSE=1
fi
status=$?
if [ $status != 0 ]; then
    exit $status
fi









cd -



