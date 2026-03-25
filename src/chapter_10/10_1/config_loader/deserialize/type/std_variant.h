/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/20 14:32:17
# Desc   :
########################################################################
*/
#pragma once

#include "../traits/compound_deserialize.h"

namespace detail {
template <typename... Ts>
struct CompoundDeserializeTraits<std::variant<Ts...>> {
  static Result deserialize(std::variant<Ts...>& obj, concepts::ParserElem auto node) {
    if (!node.isValid()) {
      return Result::ERR_MISSING_FIELD;
    }
    auto build_variant = [&obj, &node]<typename T>(T&& value) {
      auto res = CompoundDeserializeTraits<T>::deserialize(value, node);
      if (res == Result::SUCCESS) {
        obj.template emplace<T>(std::move(value));
      }
      return res;
    };
    bool success{false};
    ((success = (build_variant(Ts{}) == Result::SUCCESS)) || ...);
    return success ? Result::SUCCESS : Result::ERR_TYPE;
  }
};  // struct CompoundDeserializeTraits<std::variant<Ts...>>


}  // namespace detail
