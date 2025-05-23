/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/05/21 15:59:49
# Desc   :
########################################################################
*/

// clang -Xclang -fdump-record-layouts -fsyntax-only layout.cc

// 空类占用一字节
struct Base {
};  // struct Base
static_assert(sizeof(Base) == 1);

struct Children {
  Base base;
  int other;
};  // struct Children
static_assert(sizeof(Children) == 8);  // 由于对齐原因，Children 占用 8 字节

// 如果使用继承，那么空基类将占用 0 个字节
struct Children2 : Base {
  int other;
};  // struct Children2
static_assert(sizeof(Children2) == 4);

struct Children3 {
  [[no_unique_address]] Base base;
  int other;
};  // struct Children3
static_assert(sizeof(Children3) == 4);
