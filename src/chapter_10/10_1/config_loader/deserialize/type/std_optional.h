/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/20 14:33:04
# Desc   :
########################################################################
*/
#pragma once

#include "../traits/compound_deserialize.h"

namespace detail {
template <typename T>
struct CompoundDeserializeTraits<std::optional<T>> {
  static Result deserialize(std::optional<T>& obj, concepts::ParserElem auto node) {
    if (!node.isValid()) {
      return Result::SUCCESS;
    }
    T value;
    CHECK_SUCCESS_OR_RETURN(CompoundDeserializeTraits<T>::deserialize(value, node));
    obj.emplace(std::move(value));
    return Result::SUCCESS;
  }
};  // struct CompoundDeserializeTraits<std::optional<T>>


}  // namespace detail
