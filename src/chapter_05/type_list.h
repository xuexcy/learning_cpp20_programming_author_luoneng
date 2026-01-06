/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/01/05 15:41:20
# Desc   :
########################################################################
*/
#pragma once

#include <cstddef>

template <typename... Ts>
struct TypeList {
  struct IsTypeList {};
  using type = TypeList;
  constexpr static std::size_t size = sizeof...(Ts);
  template <typename ...T>
  using append = TypeList<Ts..., T...>;
  template <typename ...T>
  using prepend = TypeList<T..., Ts...>;
  template <template <typename...> typename T>
  using to = T<Ts...>;
};  // struct TypeList

template <typename TypeList>
concept TL = requires {
  typename TypeList::IsTypeList;
  typename TypeList::type;
};

static_assert(TypeList<>::size == 0);
