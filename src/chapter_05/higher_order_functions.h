/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/01/05 15:44:18
# Desc   : 高阶函数
########################################################################
*/
#pragma once

#include <type_traits>

#include "cpp_utils/util.h"

#include "type_list.h"

// map 高阶函数
template <TL In, template <typename> typename F>
struct Map;  // 元函数声明
// 通过偏特化的方式，从 TL In 获取 TypeList 中的各个参数 Ts...
template <template <typename> typename F, typename ...Ts>  // 元函数实现
struct Map<TypeList<Ts...>, F> : TypeList<typename F<Ts>::type...> {
};  // struct Map
template <TL In, template <typename> typename F>
using Map_t = typename Map<In, F>::type;

using LongList = TypeList<char, float, double, int, char>;
SAME_TYPE((Map_t<LongList, std::add_pointer>), (TypeList<char*, float*, double*, int*, char*>));

// filter 高阶函数
template <TL In, template <typename> typename Pred, TL Out=TypeList<>>
struct Filter : Out {};

template <template <typename> typename P, TL Out, typename H, typename ...Ts>
struct Filter<TypeList<H, Ts...>, P, Out> :
    std::conditional_t<
        P<H>::value,
        Filter<TypeList<Ts...>, P, typename Out::template append<H>>,
        Filter<TypeList<Ts...>, P, Out>
    > {
};  // struct Filter<TypeList<H, Ts...>, P, Out>
template <TL In, template <typename> typename Pred, TL Out=TypeList<>>
using Filter_t = typename Filter<In, Pred, Out>::type;

template <typename T>
using SizeLess4 = std::bool_constant<(sizeof(T) < 4)>;
SAME_TYPE((Filter_t<LongList, SizeLess4>), (TypeList<char, char>));

// fold 高阶函数
template <typename T>
struct Return {
  using type = T;
};  // struct Return

template <TL In, typename Init, template <typename, typename> typename Op>
struct Fold : Return<Init> {
};  // struct Fold : Return<Init>
template <typename Acc, template <typename, typename> typename Op, typename H, typename ...Ts>
struct Fold<TypeList<H, Ts...>, Acc, Op> : Fold<TypeList<Ts...>, typename Op<Acc, H>::type, Op> {
};  // struct Fold<TypeList<H, Ts...>, Acc, Op>

template <TL In, typename Init, template <typename, typename> typename Op>
using Fold_t = typename Fold<In, Init, Op>::type;

template <typename Acc, typename E>
using TypeSizeAcc = std::integral_constant<std::size_t, Acc::value + sizeof(E)>;
static_assert(Fold_t<LongList, std::integral_constant<std::size_t, 0>, TypeSizeAcc>::value == 18);

