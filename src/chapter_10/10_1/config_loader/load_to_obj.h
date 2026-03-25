/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/20 14:51:29
# Desc   :
########################################################################
*/
#pragma once

#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>

#include "deserialize/all_types.h"
#include "result.h"

namespace detail {

inline std::string get_file_content(std::string_view path) {
  std::ifstream file{std::filesystem::path(path)};
  if (!file.is_open()) {
    throw std::runtime_error(std::format("Cannot open file: {}", path));
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  return buffer.str();
}
template <concepts::Parser P>
struct LoadToObj {
  template <typename T, std::invocable GET_CONTENT>
  static Result operator()(T& obj, GET_CONTENT&& loader) {
    std::string content = loader();
    if (content.empty()) {
      return Result::ERR_EMPTY_CONTENT;
    }

    P parser;
    CHECK_SUCCESS_OR_RETURN(parser.parse(content.data()));
    auto root_elem = parser.toRootElemType();
    if (!root_elem.isValid()) {
      return Result::ERR_MISSING_FIELD;
    }
    return CompoundDeserializeTraits<T>::deserialize(obj, root_elem);
  }

  template <typename T>
  static Result operator()(T& obj, std::string_view path) {
    return operator()(obj, [&path] {
      return get_file_content(path);
    });
  }
};  // struct LoadToObj

template <concepts::Parser P>
inline constexpr LoadToObj<P> load_to_obj;

template <>
struct LoadToObj<UnsupportedParser> {
  template <typename T, typename Loader>
  Result operator()(T&, Loader&&) const {
    return Result::ERR_UNSUPPORTED_PARSER;
  }
};  // struct LoadToObj<concepts::UnsupportedParser>

}  // namespace detail
