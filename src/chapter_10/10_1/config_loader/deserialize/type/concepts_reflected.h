/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/20 14:33:47
# Desc   :
########################################################################
*/
#pragma once

#include "../traits/compound_deserialize.h"
#include "../../concepts.h"

namespace detail {

template <concepts::Reflected T>
struct CompoundDeserializeTraits<T> {
  static Result deserialize(T& obj, concepts::ParserElem auto node) {
    if (!node.isValid()) {
      return Result::ERR_MISSING_FIELD;
    }
    return forEachField(obj, [&node](auto&& field_info) {
      decltype(auto) field_name = field_info.name();
      decltype(auto) value = field_info.value();
      return CompoundDeserializeTraits<std::remove_cvref_t<decltype(value)>>
        ::deserialize(value, node.toChildElem(field_name));
    });
  }
};  // struct CompoundDeserializeTraits<T>

}  // namespace detail

