/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/17 23:06:36
# Desc   :
########################################################################
*/

module;

// 全局模块片段: global module fragment
#include <cmath>

module shape;

int Rectangle::width() {
  return abs(bottom_right.x - top_left.x);
}
int Rectangle::height() {
  return abs(bottom_right.y - top_left.y);
}
int Rectangle::area() {
  return width() * height();
}
