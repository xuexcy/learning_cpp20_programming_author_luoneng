/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/01/14 14:56:02
# Desc   :
########################################################################
*/
#pragma once

#include <concepts>
#include <cstddef>

namespace concepts {
template <typename List>
concept list = requires(List l) {
  { l.size() } -> std::same_as<size_t>;
  l.is_type_list;
  requires l.type_list;
};

}  // namespace concepts
