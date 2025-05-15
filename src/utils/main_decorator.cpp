/**
########################################################################
#
# Copyright (c) 2024 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author  :   xuechengyun
# E-mail  :   xuechengyun@gmail.com
# Date    :   2024/11/04 19:53:11
# Desc    :
########################################################################
*/

#include "main_decorator.h"

#include "boost/dll.hpp"

namespace utils {
const std::string MainDecorator::S_PROGRAM_LOCATION = boost::dll::program_location().string();
void MainDecorator::Access() {}
}  // namespace utils

//utils::MainDecorator g_main_decorator;
utils::MainDecorator g_utils_main_decorator;


