/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/18 13:48:04
# Desc   :
########################################################################
*/
#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "concepts.h"
#include "macro.h"
#include "result.h"


#define DEFINE_SCHEMA(st, ...) \
  struct st { \
    template <typename, size_t> struct FIELD; \
    static constexpr size_t _field_count_ = GET_ARG_COUNT(__VA_ARGS__); \
    PASTE(REPEAT_, GET_ARG_COUNT(__VA_ARGS__)) (FIELD_EACH, 0, __VA_ARGS__) \
  };
/*
DEFINE_SCHEMA(Point, (double)x, (double)y)
  =>
    struct Point {
      template <typename, size_t> struct FIELD;
      static constexpr size_t _field_count = 2;
      REPEAT_2(FIELD_EACH, 0, (double)x, (double)y)
    };
*/

/*
REPEAT_2(FIELD_EACH, 0, (double)x, (double)y)
  =>
    FIELD_EACH(0, (double)x) FIELD_EACH(1, (double)y)
*/

#define FIELD_EACH(i, arg) \
  PAIR(arg); \
  template <typename T> \
  struct FIELD<T, i> { \
    T& obj; \
    auto value() -> decltype(auto) { return (obj.STRIP(arg)); } \
    static constexpr auto name() { return STRING(STRIP(arg)); } \
  };

/*
FIELD_EACH(1, (double)y)
  =>
    double y;
    template <typename T>
    struct FIELD<T, 1> {
      T& obj;
      auto value() -> decltype(auto) { return (obj.y); }
      static constexpr auto name() { return "y"; }
    };
*/

namespace detail {
struct DummyFieldInfo {
  int& value();
  const char* name();
};  // struct DummyFieldInfo

template <concepts::Reflected T, std::invocable<detail::DummyFieldInfo> F, size_t... Is>
constexpr auto forEachFieldImpl(T&& obj, F&& f, std::index_sequence<Is...>) {
  using TYPE = std::decay_t<T>;
  using detail::DummyFieldInfo;

  if constexpr (std::same_as<decltype(f(std::declval<DummyFieldInfo>())), Result>) {
    Result res{Result::SUCCESS};
    (... && [&] {
      res = f(typename TYPE::template FIELD<T, Is>(std::forward<T>(obj)));
      return res == Result::SUCCESS;
    }());
    return res;
  } else {
    ([&] {
      f(typename TYPE::template FIELD<T, Is>(std::forward<T>(obj)));
    }(), ...);
  }
}
}  // namespace detail

template <concepts::Reflected T, std::invocable<detail::DummyFieldInfo> F>
constexpr auto forEachField(T&& obj, F&& f) {
  return detail::forEachFieldImpl(
    std::forward<T>(obj),
    std::forward<F>(f),
    std::make_index_sequence<std::decay_t<T>::_field_count_>{});
}

