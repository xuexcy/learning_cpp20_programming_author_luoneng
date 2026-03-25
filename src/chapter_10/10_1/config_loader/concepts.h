/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/19 14:43:09
# Desc   :
########################################################################
*/
#pragma once

#include <concepts>
#include <optional>
#include <string>

#include "enable_parser.h"
#include "result.h"

namespace concepts {

template <typename T>
concept Reflected = requires (T obj) {
  { obj._field_count_ } -> std::convertible_to<size_t>;
  requires ((obj._field_count_ == 0) ||
    requires {
      typename std::decay_t<T>::template FIELD<T, 0>;
    }
  );
};


template <typename ElemType>
concept ParserElem = requires(const ElemType& elem) {
  { elem.isValid() } -> std::convertible_to<bool>;
  { elem.getValueText() } -> std::same_as<std::optional<std::string>>;
  { elem.getKeyName() } -> std::same_as<const char*>;
  { elem.toChildElem("") } -> std::same_as<ElemType>;
  requires requires(Result(&f)(ElemType)) {
    elem.forEachElement(f);
  };
  { elem.serializeToString() } -> std::same_as<std::string>;
};


template <typename P>
concept Parser = detail::enable_parser<P> || requires(P p, std::string_view content) {
  requires std::default_initializable<P>;
  typename P::ElemType;
  requires ParserElem<typename P::ElemType>;
  { p.parse(content) } -> std::same_as<Result>;
  { p.toRootElemType() } -> std::same_as<typename P::ElemType>;
};
static_assert(Parser<detail::UnsupportedParser>);

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;


}  // namespace concepts

