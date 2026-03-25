/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/20 14:34:40
# Desc   :
########################################################################
*/
#pragma once

#include "../traits/compound_deserialize.h"
#include "../traits/primitive_deserialize.h"
#include "../../concepts.h"

namespace detail {

template <concepts::Primitive T>
struct CompoundDeserializeTraits<T> {
  static Result deserialize(T& obj, concepts::ParserElem auto node) {
    if (!node.isValid()) {
      return Result::ERR_MISSING_FIELD;
    } else {
      return PrimitiveDeserializeTraits<T>::deserialize(obj, node.getValueText());
    }
  }
};  // struct CompoundDeserializeTraits<T>


}  // namespace detail
