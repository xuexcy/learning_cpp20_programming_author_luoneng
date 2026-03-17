/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2026/03/15 00:57:29
# Desc   : 第 9 章模块
########################################################################
*/

#include "cpp_utils/util.h"

import std;
import math;
import shape;

void run_import() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::cout << "hello world" << std::endl;
}

// 9.3 模块分区
void run_shape() {
  PRINT_CURRENT_FUNCTION_NAME;
  Rectangle r{{1, 2}, {3, 4}};
  std::println("area: {}", r.area());
  std::println("width: {}", r.width());
  std::println("height: {}", r.height());
}

// 9.4 私有片段
// 实现可以写在 .cpp 中，这样就不会暴露在模块接口中
// 如果不想单开一个文件，可以在.cppm 中先使用 module :private 再实现
void run_private() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::println("triple(3): {}", triple(3));
}

// 9.5 模块样板文件
int main() {
  run_import();
  run_shape();
  run_private();
  return 0;
}


