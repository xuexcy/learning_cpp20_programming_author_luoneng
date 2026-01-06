/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/01/05 15:46:44
# Desc   :
########################################################################
*/
#pragma once

#include <type_traits>

#include "cpp_utils/util.h"

#include "higher_order_functions.h"
#include "type_list.h"

// concat 算法
template <TL... In> struct Concat;
template <TL... In> using Concat_t = typename Concat<In...>::type;

template <>
struct Concat<> : TypeList<> {
};  // struct Concat<> : TypeList<>
template <TL In>
struct Concat<In> : In {
};  // struct Concat<In> : In
template <TL In, TL In2, TL ...Rest>
struct Concat<In, In2, Rest...> : Concat_t<Concat_t<In, In2>, Rest...> {
};  // struct Concat<In, In2, Rest...>

/*
1. 使用 fold 将 In2 append 到 In 后面
但 fold 是将 In2 中的元素一个个 append 到 In 后面，效率比较低

template <TL In1, TL In2>
struct Concat<In1, In2> {
private:
  template <TL Acc, typename E>
  using Append = typename Acc::template append<E>;
public:
  using type = Fold_t<In2, In1, Append>;
};  // struct Concat<In1, In2>

2. 使用 to 方法，将 In2 的模板参数 Ts2... 给到 In1 的 append 方法
append 会 Ts2... 一次性添加到 In2 的模板参数 Ts1... 后面

template <TL In1, TL In2>
struct Concat<In1, In2> : In2::template to<In1::template append> {
};  // struct Concat<In1, In2>

*/

// 3. 直接偏特化获取 In1 和 In2 的模板参数列表
template <typename ...Ts, typename ... Ts2>
struct Concat<TypeList<Ts...>, TypeList<Ts2...>>  : TypeList<Ts..., Ts2...> {
};  // struct Concat<TypeList<Ts...>, TypeList<Ts2...>

SAME_TYPE(
    (Concat_t<TypeList<int, double>, TypeList<char, float>>),
    (TypeList<int, double, char, float>));


// elem 算法: 查找元素是否存在
// 1. 使用 fold 实现
template <TL In, typename E>
class Elem {
  template <typename Acc, typename T>
  using FindE = std::conditional_t<Acc::value, Acc, std::is_same<T, E>>;
  using Found = Fold_t<In, std::false_type, FindE>;
public:
  constexpr static bool value = Found::value;
};  // class Elem
// 2. 使用 c++17 折叠表达式实现
template <TL In, typename E>
struct Elem2 : std::false_type {
  constexpr static int s = 1;
};  // struct Elem2 : std::false_type
template <typename E, typename ...Ts>
struct Elem2<TypeList<Ts...>, E> : std::bool_constant<(false  || ... || std::is_same_v<Ts, E>)> {
  constexpr static int s = 2;
};  // struct Elem2<TypeList<Ts...>, E>

template <TL In, typename E>
constexpr bool Elem2_v = Elem2<In, E>::value;
static_assert(false == Elem2<TypeList<>, int>::value);
static_assert(2 == Elem2<TypeList<>, int>::s);
static_assert(Elem2_v<LongList, char>);
static_assert(!Elem2_v<LongList, long long>);

// unique 算法：去重，删除重复
template <TL In>
class Unique {
  template <TL Acc, typename E>
  using Append = std::conditional_t<Elem2_v<Acc, E>, Acc, typename Acc::template append<E>>;
public:
  using type = Fold_t<In, TypeList<>, Append>;
};  // class Unique
template <TL In>
using Unique_t = Unique<In>::type;

SAME_TYPE(Unique_t<LongList>, (TypeList<char, float, double, int>));

// partition 算法
template <TL In, template<typename> typename Pred>
class Partition {
  template <typename Arg>
  using NotPred = std::bool_constant<!Pred<Arg>::value>;
public:
  struct type {
    // 这种方法不够高效，需要 filter 两次
    using Satisfied = Filter_t<In, Pred>;
    using Rest = Filter_t<In, NotPred>;
  };  // struct type
};  // class Partition
template <TL In, template<typename> typename Pred>
using Partition_t = Partition<In, Pred>::type;

using SplitBySize4 = Partition_t<LongList, SizeLess4>;
SAME_TYPE(SplitBySize4::Satisfied, (TypeList<char, char>));
SAME_TYPE(SplitBySize4::Rest, (TypeList<float, double, int>));

// sort 算法
// 快排: 使用
template <TL In, template <typename, typename> typename Cmp>
struct Sort : TypeList<> {
};  // struct Sort : TypeList<>
template <TL In, template <typename, typename> typename Cmp>
using Sort_t = Sort<In, Cmp>::type;

template <template<typename, typename> typename Cmp, typename Pivot, typename... Ts>
class Sort<TypeList<Pivot, Ts...>, Cmp> {
  template <typename E>
  using LT = Cmp<E, Pivot>;
  using P = Partition_t<TypeList<Ts...>, LT>;
  using SmallerSorted = Sort_t<typename P::Satisfied, Cmp>;
  using BiggerSorted = Sort_t<typename P::Rest, Cmp>;

public:
  using type = Concat_t<
      typename SmallerSorted::template append<Pivot>,
      BiggerSorted>;
};  // class Sort<TypeList<Pivot, Ts...>, Cmp>


template <typename L, typename R>
using SizeCmp = std::bool_constant<sizeof(L) < sizeof(R)>;
SAME_TYPE((Sort_t<LongList, SizeCmp>), (TypeList<char, char, float, int, double>));
