/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/12/28 15:50:54
# Desc   :
########################################################################
*/
#pragma once

#include <cstddef>

template <typename T>
class Collection {
  T* array{nullptr};
  std::size_t size;
 public:
  Collection(std::size_t sz) : size(sz) {}
};  // class Collection<T>
