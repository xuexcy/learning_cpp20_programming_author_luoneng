#!/bin/bash
########################################################################
#
# Copyright (c) 2024 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author  :   xuechengyun
# E-mail  :   xuechengyunxue@gmail.com
# Date    :   24/10/20 19:34:50
# Desc    :   编译所有目标文件
########################################################################

# set -x
CUR_DIR=$(cd `dirname $0`; pwd)
cd ${CUR_DIR}

source env_variable.sh
mkdir -p $build_dir
cd $build_dir
cmake ..
make -j$(getconf _NPROCESSORS_ONLN)

