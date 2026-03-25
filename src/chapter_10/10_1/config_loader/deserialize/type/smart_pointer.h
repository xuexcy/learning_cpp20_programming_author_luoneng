/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/23 15:42:49
# Desc   :
########################################################################
*/
#pragma once

#include "../traits/compound_deserialize.h"
#include "../../result.h"
#include <memory>

namespace detail {
template <typename SP>
struct SmartPointerDeserialize {
  static Result deserialize(SP& sp, concepts::ParserElem auto node) {
    if (!node.isValid()) {
      return Result::SUCCESS;
    }
    using ElementType = typename SP::element_type;
    ElementType value;
    CHECK_SUCCESS_OR_RETURN(CompoundDeserializeTraits<ElementType>::deserialize(value, node));
    sp.reset(new ElementType(std::move(value)));
    return Result::SUCCESS;
  }
};  // struct SmartPointerDeserialize

template <typename T>
struct CompoundDeserializeTraits<std::unique_ptr<T>> : SmartPointerDeserialize<std::unique_ptr<T>>{
};  // struct CompoundDeserializeTraits<std::unique_ptr<T>>
template <typename T>
struct CompoundDeserializeTraits<std::shared_ptr<T>> : SmartPointerDeserialize<std::shared_ptr<T>>{
};  // struct CompoundDeserializeTraits<std::shared_ptr<T>>
}  // namespace detail
