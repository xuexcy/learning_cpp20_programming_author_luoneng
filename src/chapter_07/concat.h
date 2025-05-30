/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/05/29 18:56:17
# Desc   :
########################################################################
*/
#pragma once

#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>



template <std::ranges::input_range... Rngs>
requires (std::ranges::view<Rngs> && ...) && (sizeof...(Rngs) >= 1)
struct concat_view {
  static constexpr std::size_t rng_size{sizeof...(Rngs)};
  using RNGs= std::tuple<Rngs...>;
  RNGs rngs;

  concat_view() = default;
  concat_view(Rngs... rngs) : rngs(std::move(rngs)...) {}
  struct concat_iterator {
    using difference_type = std::ptrdiff_t;
    using value_type = std::common_type_t<std::ranges::range_value_t<Rngs>...>;

    concat_iterator() = default;
    concat_iterator(RNGs* rngs): rngs(rngs) {
      its.template emplace<0>(std::ranges::begin(std::get<0>(*rngs)));
    }
    concat_iterator& operator++() {
      std::visit([&](auto&& iter) {
        constexpr std::size_t idx = std::decay_t<decltype(iter)>::value;
        iter.iterator = std::ranges::next(iter.iterator);
        if (iter.iterator == std::ranges::end(std::get<idx>(*rngs))) {
          if constexpr (idx + 1 < rng_size) {
            its.template emplace<idx+1>(std::ranges::begin(std::get<idx+1>(*rngs)));
          }
        }
      }, its);
      return *this;
    }
    concat_iterator operator++(int) {
      auto tmp(*this);
      ++(*this);
      return tmp;
    }
    bool operator==(std::default_sentinel_t) const {
      return its.index() == rng_size - 1 &&
          (std::get<rng_size-1>(its).iterator == std::ranges::end(std::get<rng_size-1>(*rngs)));
    }
    bool operator==(const concat_iterator&) const = default;
    // common_reference : 类型公共部分的引用
    using reference = std::common_reference_t<std::ranges::range_reference_t<Rngs>...>;
    reference operator*() const {
      return std::visit([&](auto&& iter) -> reference{
        return *iter.iterator;
      }, its);
    }

    // 储存 Rng 的一个 iterator 和 Rng 在 Rngs 中的 index
    template <std::size_t N, typename Rng>
    struct iterator_with_index : std::integral_constant<std::size_t, N> {
      std::ranges::iterator_t<Rng> iterator;

      iterator_with_index() = default;
      iterator_with_index(std::ranges::iterator_t<Rng> it) : iterator(std::move(it)) {}
      bool operator==(const iterator_with_index&) const = default;
    };  // struct iterator_with_index

    // 函数模板声明
    template <std::size_t... Is>
    static constexpr auto iterator_variant_generator(std::index_sequence<Is...>) ->
        std::variant<iterator_with_index<Is, std::tuple_element_t<Is, RNGs>>...>;

    /* 由 <0, 1, 2, 3, 4> 推导出迭代器类型 std::variant<
          iterator_with_index<0, RNGs[0]>,
          iterator_with_index<1, RNGs[1]>,
          iterator_with_index<2, RNGs[2]>,
          iterator_with_index<3, RNGs[3]>,
          iterator_with_index<4, RNGs[4]>
        >
    */
    using IteratorTypes = decltype(iterator_variant_generator(std::make_index_sequence<rng_size>{}));
    IteratorTypes its;
    RNGs* rngs{};
  };  // struct concat_iterator
  concat_iterator begin() {
    return &this->rngs;  // tuple 的第 0 个元素的地址
  }
  std::default_sentinel_t end() { return {}; }

};  // struct concat_view
inline constexpr auto concat = []<std::ranges::viewable_range... Rngs>(Rngs&&... rngs) {
  return concat_view<Rngs...>(std::forward<Rngs>(rngs)...);
};
