/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/01/26 15:36:49
# Desc   :
########################################################################
*/
#pragma once

#include <type_traits>

template <typename T>
struct has_member_operator_co_await : std::false_type {
};  // struct has_member_operator_co_await

template <typename T>
requires requires(T t) {
  t.operator co_await();
}
struct has_member_operator_co_await<T> : std::true_type {
};  // struct has_member_operator_co_await<T>

template <typename T>
inline constexpr bool has_member_operator_co_await_v = has_member_operator_co_await<T>::value;


template <typename T>
struct has_non_member_operator_co_await : std::false_type {
};  // struct has_non_member_operator_co_await

template <typename T>
requires requires(T t) {
  operator co_await(t);
}
struct has_non_member_operator_co_await<T> : std::true_type {
};  // struct has_member_operator_co_await<T>

template <typename T>
inline constexpr bool has_non_member_operator_co_await_v = has_non_member_operator_co_await<T>::value;
