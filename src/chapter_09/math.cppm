/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/17 13:47:47
# Desc   :
########################################################################
*/

// 全局模块声明
module;

// 全局模块片段: global module fragment
// 在全局模块声明与正式模块声明之间，可以放置 #include 和 #define
// 该片段内容不归模块所有，不会导出
#include <concepts>


// 正式模块声明
export module math;
// 模块名允许带 ".", 比如 std.core

// 正式模块声明后的内容就是模块内的内容
// 除了在全局模块片段 #include，也可以在模块内部进行 import
// import <concepts>

export template <std::integral T>
T square(T x) {
  return x * x;
}

export int square(int x);

export template <std::integral T>
T triple(T x);

// gcc 15.2 还不支持 private:  sorry, unimplemented: private module fragment
// module :private;
template <std::integral T>
T triple(T x) {
  return x * 3;
}
