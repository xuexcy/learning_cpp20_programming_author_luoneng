/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/23 15:50:46
# Desc   :
########################################################################
*/
#pragma once

#include <list>
#include <vector>

#include "../traits/compound_deserialize.h"
#include "../../result.h"

namespace detail {

template <typename Container, typename ValueType = Container::value_type>
struct SeqContainerDeserialize {
  static Result deserialize(Container& container, concepts::ParserElem auto node) {
    if (!node.isValid()) {
      return Result::SUCCESS;
    }
    return node.forEachElement([&container](concepts::ParserElem auto item) {
      ValueType value;
      CHECK_SUCCESS_OR_RETURN(CompoundDeserializeTraits<ValueType>::deserialize(value, item));
      container.emplace_back(std::move(value));
      return Result::SUCCESS;
    });
  }
};  // struct SeqContainerDeserialize

template <typename T>
struct CompoundDeserializeTraits<std::vector<T>> : SeqContainerDeserialize<std::vector<T>>{
};  // struct CompoundDeserializeTraits<std::vector<T>>

template <typename T>
struct CompoundDeserializeTraits<std::list<T>> : SeqContainerDeserialize<std::list<T>>{
};  // struct CompoundDeserializeTraits<std::list<T>>
}  // namespace detail
