/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/01/05 16:33:57
# Desc   : 在编译时能对用户定义的KV类型进行重排，将那些同样大小和
#          对齐方式的值类型排到一起，这样就能够生成最为紧凑、缓存占用最小的数据表
########################################################################
*/
#pragma once

#include <algorithm>
#include <bitset>
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "algorithm.h"
#include "type_list.h"

// 1. 对键值信息进行分组
template <auto Key, typename ValueType, std::size_t Dim = 1>
struct Entry {
  constexpr static auto key = Key;
  constexpr static std::size_t dim = Dim;
  constexpr static bool is_array = (Dim > 1);
  using type = ValueType;
};  // struct Entry

// 特化数组
template <auto Key, typename ValueType, std::size_t Dim>
struct Entry<Key, ValueType[Dim]> : Entry<Key, ValueType, Dim> {
};  // struct Entry<Key, ValueType[Dim]> : Entry<Key, ValueType, Dim>

template <typename E>
concept KVEntry = requires {
  typename E::type;
  requires std::is_standard_layout_v<typename E::type>;
  requires std::is_trivial_v<typename E::type>;
  { E::key } -> std::convertible_to<std::size_t>;
  { E::dim } -> std::convertible_to<std::size_t>;
};

static_assert(KVEntry<Entry<0, char[10]>>);


// 对键值信息分组
template <TL Es = TypeList<>, TL Gs = TypeList<>>
struct GroupEntriesTrait : Gs {
};  // struct GroupEntriesTrait

template <KVEntry H, KVEntry... Ts, TL Gs>
class GroupEntriesTrait<TypeList<H, Ts...>, Gs> {
  template <KVEntry E>
  using GroupPrediction = std::bool_constant<
      H::dim == E::dim &&
      sizeof(typename H::type) == sizeof(typename E::type) &&
      alignof(typename H::type) == alignof(typename E::type)>;

      using Group = Partition_t<TypeList<H, Ts...>, GroupPrediction>;
      using Satisfied = typename Group::Satisfied;
      using Rest = typename Group::Rest;
public:
  using type = typename GroupEntriesTrait<Rest, typename Gs::template append<Satisfied>>::type;
};  // class GroupEntriesTrait<TypeList<H, Ts...>, Gs>

template <TL Es = TypeList<>, TL Gs = TypeList<>>
using GroupEntriesTrait_t = GroupEntriesTrait<Es, Gs>::type;

template <TL Es>
using Gs = GroupEntriesTrait_t<Es>;

// 生成 Regions 类
template <KVEntry EH, KVEntry... ET>
class GenericRegion {
  constexpr static std::size_t number_of_entries = sizeof...(ET) + 1;
  // 使用 max 是因为定义类时可以手动指定 alignas
  constexpr static std::size_t max_size = std::max(
      alignof(typename EH::type),
      sizeof(typename EH::type)
  ) * EH::dim;
  char data[number_of_entries][max_size];
public:
  bool get_data(std::size_t nth_data, void* out, std::size_t len) {
    if (nth_data >= number_of_entries) [[unlikely]] { return false; }
    std::copy_n(data[nth_data], std::min(len, max_size), reinterpret_cast<char*>(out));
    return true;
  }
  bool set_data(std::size_t nth_data, const void* value, std::size_t len) {
    if (nth_data >= number_of_entries) [[unlikely]] { return false; }
    std::copy_n(reinterpret_cast<const char*>(value), std::min(len, max_size), data[nth_data]);
    return true;
  }
};  // class GenericRegion

// 将 GenericRegion 装到 Regions 中
namespace use_virtual {
// 方案1: 使用虚函数机制, Regions 中存放继承了 Region 的 GenericRegion
// 缺点:
//   a. 这种方案使用了虚函数表，即使 GenericRegion 中只有一个键值
//   b. 对 Regions 中 Region 的访问使用数组，产生计算指针的开销
struct Region {
  virtual bool get_data(std::size_t index, void* out, std::size_t len) = 0;
  virtual bool get_data(std::size_t index, const void* value, std::size_t len) = 0;
  virtual ~Region() = default;
};  // struct Region

template <KVEntry EH, KVEntry... ET>
class GenericRegion : public Region {
};  // class GenericRegion : public Region

template <typename... R>
class Regions {
  Region* regions[sizeof...(R)];
public:
  constexpr Regions() {
    std::size_t i = 0;
    ((regions[i++] = static_cast<R*>(this)), ...);
  }
};  // class Regions

}  // namespace use_virtual

template <typename... R>
class Regions {
  std::tuple<R...> regions;
public:
  bool get_data(std::size_t index, void* out, std::size_t len) {
    auto op = [&](auto& region, std::size_t nth_data) {
      return region.get_data(nth_data, out, len);
    };
    return for_data(std::make_index_sequence<sizeof...(R)>{}, op, index);
  }
  bool set_data(std::size_t index, const void* value, std::size_t len) {
    auto op = [&](auto& region, std::size_t nth_data) {
      return region.set_data(nth_data, value, len);
    };
    return for_data(std::make_index_sequence<sizeof...(R)>{}, op, index);
  }
  template <std::size_t I, typename OP>
  bool for_data(OP&& op, std::size_t index) {
    std::size_t region_index = (index >> 16);
    if (I == region_index) {
      std::size_t nth_data = (index & 0xFFFF);
      return op(std::get<I>(regions), nth_data);
    }
    return false;
  }
  template <typename OP, std::size_t... Is>
  bool for_data(std::index_sequence<Is...>, OP&& op, std::size_t index) {
    return (for_data<Is>(std::forward<OP>(op), index) || ...);
  }
};  // class Regions

template <TL Gs>
class GenericRegionTrait {
  template <TL G>
  using ToRegion = Return<typename G::template to<GenericRegion>>;
public:
  using type = Map_t<Gs, ToRegion>;
};  // class GenericRegionTrait
template <TL Gs>
using GenericRegionTrait_t = GenericRegionTrait<Gs>::type;

// Gs< <E1, E2>, <E3>, <E4, E5>> 通过 Map_t<Gs, ToRegion> 放到 TypeList<GenericRegion, ...> 中
// 再通过 to<Regions> 将 GenericRegion 放到 Regions 中
template <TL Gs>
using RegionsClass = typename GenericRegionTrait_t<Gs>::template to<Regions>;


// 3. 生成 Indexer 类
template <typename... Indexes>
struct Indexer {
  // key 是 Entry::key, id 是 Group 后的 ((GroupIndex << 16) | InnerIndex)
  std::size_t key_to_id[sizeof...(Indexes)];
  std::bitset<sizeof...(Indexes)> mask;
  constexpr Indexer() {
    constexpr std::size_t index_size = sizeof...(Indexes);
    static_assert(((Indexes::key < index_size) && ...), "key is out of size");
    (void(key_to_id[Indexes::key] = Indexes::id), ...);
  }
};  // struct Indexer

// 输入 GroupEntities, 输出 TypeList<Index1, Index2, ...>
/*
TypeList tl;
std::size_t group_idx = 0;
for (auto& group : GroupEntities) {
  std::size_t inner_idx = 0;
  for (auto& entry : group) {
    tl.append(((group_idx) << 16) | inner_idx);
    ++inner_idx;
  }
  ++group_idx;
}
*/
// 使用元编程实现两层for循环
template <TL Gs>
class GroupIndexTrait {
  template <std::size_t GroupIdx = 0, std::size_t InnerIdx = 0, TL Res = TypeList<>>
  struct Index {
    constexpr static std::size_t GroupIndex = GroupIdx;
    constexpr static std::size_t InnerIndex = InnerIdx;
    using Result = Res;
  };  // struct Index

  template <typename Acc, TL G>
  class AddGroup {
    constexpr static std::size_t GroupIndex = Acc::GroupIndex;
    template <typename Acc_, KVEntry E>
    struct AddKey {
      constexpr static std::size_t InnerIndex = Acc_::InnerIndex;
      struct KeyWithIndex {
        constexpr static auto key = E::key;
        constexpr static auto id = (GroupIndex << 16) | InnerIndex;
      };  // struct KeyWithIndex
      using Result = typename Acc_::Result::template append<KeyWithIndex>;
     public:
      // Index 的第1、2个参数是 next_group_index 和 next_inner_index, 所以需要 +1
      // group_index 和 inner_index 在 KeyWithIndex 中使用了
      using type = Index<GroupIndex + 1, InnerIndex + 1, Result>;
    };  // struct AddKey
  public:
    using type = Fold_t<G, Index<GroupIndex + 1, 0, typename Acc::Result>, AddKey>;
  };  // class AddGroup
public:
  using type = Fold_t<Gs, Index<>, AddGroup>::Result;
};  // class GroupIndexTrait

template <TL Gs>
using GroupIndexTrait_t = typename GroupIndexTrait<Gs>::type;

// 将 TypeList<KeyWithIndex> 中的 KeyWithIndex 放到 class Indexer
template <TL Gs>
using IndexerClass = typename GroupIndexTrait_t<Gs>::template to<Indexer>;

template <TL Es>
class Datatable {
  using InnerRegionsClass = RegionsClass<GroupEntriesTrait_t<Es>>;
  using InnerIndexerClass = IndexerClass<GroupEntriesTrait_t<Es>>;
public:
  bool get_data(std::size_t key, void* out, std::size_t len = -1) {  // size_t 是无符号的，-1 相当于最大值
    if (key >= Es::size || !indexer_.mask[key]) {
      return false;
    }
    return regions_.get_data(indexer_.key_to_id[key], out, len);
  }

  bool set_data(std::size_t key, const void* in, std::size_t len = -1) {
    if (key >= Es::size) {
      return false;
    }
    return indexer_.mask[key] = regions_.set_data(indexer_.key_to_id[key], in, len);
  }

  void dump_group_info() {
    std::println("sizeof Datatable = {}", sizeof(Datatable));
    std::println("sizeof Region = {}", sizeof(InnerRegionsClass));
    std::println("sizeof Indexer = {}", sizeof(InnerIndexerClass));
    for (size_t k = 0; k < Es::size; ++k) {
      auto id = indexer_.key_to_id[k];
      std::println("key = {} id = {:#07x} group = {} subgroup = {}",
          k, id, (id >> 16), (id & 0xFFFF));
    }
  }
private:
  InnerRegionsClass regions_;
  InnerIndexerClass indexer_;
};  // class Datatable
