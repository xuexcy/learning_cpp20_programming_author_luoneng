/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/20 14:37:54
# Desc   :
########################################################################
*/
#pragma once

#include <algorithm>
#include <optional>
#include <sstream>
#include <string>

#include "../../concepts.h"
#include "../../result.h"

namespace detail {

constexpr auto is_hex_char = [](char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
};
inline bool is_hex(std::string_view str) noexcept {
  if (str.empty()) return false;
  auto view = str;
  if (str.size() >= 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
    view = str.substr(2);
    if (view.empty()) return false;
  }
  return std::ranges::all_of(view, is_hex_char);
}

template <typename T>
struct PrimitiveDeserializeTraits;

template <concepts::Arithmetic Number>
struct PrimitiveDeserializeTraits<Number> {
  static Result deserialize(Number& num, std::optional<std::string> value_text) {
    if (!value_text.has_value()) {
      return Result::ERR_EXTRACTING_FIELD;
    }
    // 对 int8_t和 uint8_t 特殊处理，免得被当成 char 类型
    if constexpr(std::is_same_v<Number, int8_t> || std::is_same_v<Number, uint8_t>) {
      num = std::stol(*value_text, nullptr, is_hex(*value_text) ? 16 : 10);
      return Result::SUCCESS;
    } else {
      std::stringstream ss;
      ss << *value_text;
      if (is_hex(*value_text)) {
        ss << std::hex;
      }
      ss >> num;
      return ss.fail() ? Result::ERR_EXTRACTING_FIELD : Result::SUCCESS;
    }
  }
};  // struct PrimitiveDeserializeTraits<Number>

template <>
struct PrimitiveDeserializeTraits<bool> {
  static Result deserialize(bool& value, std::optional<std::string> value_text) {
    if (!value_text.has_value()) {
      return Result::ERR_EXTRACTING_FIELD;
    }
    const auto& text = value_text.value();
    if (text == "true" || text == "True") {
      value = true;
      return Result::SUCCESS;
    } else if (text == "false" || text == "False") {
      value = false;
      return Result::SUCCESS;
    } else {
      return Result::ERR_EXTRACTING_FIELD;
    }
  }
};  // struct PrimitiveDeserializeTraits<bool>

template <>
struct PrimitiveDeserializeTraits<std::string> {
  static Result deserialize(std::string& value, std::optional<std::string> value_text) {
    if (!value_text.has_value()) {
      return Result::ERR_EXTRACTING_FIELD;
    }
    value = std::move(*value_text);
    return Result::SUCCESS;
  }
};  // struct PrimitiveDeserializeTraits<std::string>

}  // namespace detail

namespace concepts {

template <typename T>
concept Primitive = requires {
  sizeof(detail::PrimitiveDeserializeTraits<T>);  // 有定义/特化
};

}
