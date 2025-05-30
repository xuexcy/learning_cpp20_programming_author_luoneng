/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/05/30 14:03:02
# Desc   :
########################################################################
*/
#pragma once

#include <__ranges/range_adaptor.h>
#include <cstddef>
#include <iterator>
#include <ranges>

namespace ranges = std::ranges;
namespace views = std::views;

template <ranges::input_range Rng>
requires std::ranges::view<Rng>
struct chunk_view : ranges::view_interface<chunk_view<Rng>>{
  chunk_view() = default;
  chunk_view(Rng r, std::size_t n) : rng(std::move(r)), n(n) {}
  struct chunk_iterator {
    using difference_type = std::ptrdiff_t;
    using value_type = ranges::subrange<ranges::iterator_t<Rng>>;
    chunk_iterator& operator++() {
      cur = next;
      if (cur != last) {
        next = ranges::next(cur, n, last);
      }
      return *this;
    }
    chunk_iterator operator++(int) {
      auto tmp(*this);
      ++(*this);
      return tmp;
    }
    value_type operator*() const {
      return {cur, next};
    }
    bool operator==(std::default_sentinel_t) const { return cur == last; }
    bool operator==(const chunk_iterator&) const = default;

    std::size_t n;
    ranges::iterator_t<Rng> cur{};
    ranges::iterator_t<Rng> next{};
    ranges::sentinel_t<Rng> last{};
  };  // struct chunk_iterator
  chunk_iterator begin() {
    auto begin = ranges::begin(rng);
    auto end = ranges::end(rng);
    return {n, begin, ranges::next(begin, n, end), end};
  }
  std::default_sentinel_t end() { return {}; }
  Rng rng;
  std::size_t n;
};  // struct chunk_view

struct chunk_adapter : ranges::range_adaptor_closure<chunk_adapter> {
  std::size_t n;
  template <std::ranges::viewable_range Rng>
  constexpr auto operator()(Rng&& r) const {
    return chunk_view(std::forward<Rng>(r), n);
  }
};  // struct chunk_adapter

inline auto chunk(std::size_t n) {
  return chunk_adapter{.n = n};
}
