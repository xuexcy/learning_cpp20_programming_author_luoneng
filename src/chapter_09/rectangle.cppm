/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/17 23:03:38
# Desc   :
########################################################################
*/

export module shape:rectangle;

import :point;

export struct Rectangle {
  Point top_left;
  Point bottom_right;
  int width();
  int height();
  int area();
};  // struct Rectangle

