/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/12/25 15:12:43
# Desc   : 第 5 章 模板元编程
########################################################################
*/

#include "cpp_utils/util.h"
#include <cassert>
#include <concepts>
#include <type_traits>
#include <variant>

#include "type_list.h"

// 5.1 模板 vs 宏
// 5.1.1 泛型函数
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
template <typename T>
T max(T a, T b) { return a > b ? a : b; }

template <typename T>
concept Comparable = requires(T a, T b) {
  {a > b} -> std::same_as<bool>;
};

template <Comparable T>
T max_with_concept(T a, T b) { return a > b ? a : b; }

// 5.1.2 泛型容器
// C语言的 X 宏
#define INSTANCE
#define TYPE int
#include "collection.def"
#undef TYPE

#define TYPE double
#include "collection.def"
#undef TYPE

#undef INSTANCE

#include "collection.h"

void run_X_macro() {
  PRINT_CURRENT_FUNCTION_NAME;
  Collection_int col = make_Collection_int(10);
  Collection<int> col2(20);
}

// 5.2 模板类元函数
// 5.2.1 数值计算
template <std::size_t N>
struct Fibonacci {
  constexpr static std::size_t value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};  // struct Fibonacci
template <>
struct Fibonacci<0> {
  constexpr static std::size_t value = 0;
};  // struct Fibonacci<0>
template <>
struct Fibonacci<1> {
  constexpr static std::size_t value = 1;
};  // struct Fibonacci<1>
static_assert(Fibonacci<10>::value == 55);

// 5.2.2 类型计算
using Array5x4x3 = std::array<std::array<std::array<int, 3>, 4>, 5>;
using CArray5x4x3 = int[5][4][3];

template <typename T, std::size_t I, std::size_t... Is>
struct Array {
  using type = std::array<typename Array<T, Is...>::type, I>;
};  // struct Array
template <typename T, std::size_t I>
struct Array<T, I> {
  using type = std::array<T, I>;
};  // struct Array<T, I>
SAME_TYPE((Array<int, 5, 4, 3>::type), Array5x4x3);

// 5.3 TypeList
namespace demo {
template <typename... Ts>
struct TypeList {
};  // struct TypeList
using List = TypeList<int, double>;
using One = std::integral_constant<int, 1>;
constexpr int one = One::value;
using Two = std::integral_constant<int, 2>;
constexpr int two = Two::value;

using OneTwo = TypeList<One, Two>;
using OneTwoV2 = std::integer_sequence<int, 1, 2>;

using TrueType = std::integral_constant<bool, true>;
using FalseType = std::integral_constant<bool, false>;
static_assert(TrueType::value == true);
static_assert(FalseType::value == false);

}

// 5.3.1 基本方法

// type_list.h
using AList = TypeList<int, char>;
static_assert(TL<AList>);
static_assert(AList::size == 2);
SAME_TYPE(AList::prepend<double>, (TypeList<double, int, char>));
SAME_TYPE(AList::append<double>, (TypeList<int, char, double>));
SAME_TYPE(AList::to<std::tuple>, (std::tuple<int, char>));
SAME_TYPE(AList::to<std::variant>, (std::variant<int, char>));

// 5.3.2 高阶函数
// 函数的输入或输出也是函数，那这个函数就是高阶函数
// transform, filter, fold
// #include "higher_order_functions.h"

// 5.3.3 常用算法
// #include "algorithm.h"

// 5.4 综合运用
// 5.4.1 全局最短路径
#include "graph_edsl.h"


using A = Node<'A'>;
using B = Node<'B'>;
using C = Node<'C'>;
using D = Node<'D'>;
using E = Node<'E'>;

// 运行时
using g = Graph<
    link(node(A) -> node(B) -> node(C) -> node(D)),
    link(node(A) -> node(C)),  // 最短路
    link(node(B) -> node(A)),  // 环
    link(node(A) -> node(E))>;
static_assert(1 == sizeof(g));
static_assert(g::get_path('A', 'D').sz == 3);  // A -> C -> D
static_assert(g::get_path('A', 'C').sz == 2);

void run_graph_edsl() {
  PRINT_CURRENT_FUNCTION_NAME;
  auto path = g::get_path('A', 'D');
  std::println("path size from A to D: {}", path.sz);
}


// 5.4.2 KV 数据表
#include "kv_table.h"

using AllEntries = TypeList<
    Entry<0, int>, Entry<1, char>, Entry<2, char>,
    Entry<3, short>, Entry<4, char[10]>, Entry<5, char[10]>,
    Entry<6, int>
>;
/*
按类型大小与对齐方式对键进行排序，形成四个组
Regions {
  GenericRegion<Entry<0, int>, Entry<6, int>>
  GenericRegion<Entry<1, int>, Entry<2, int>>
  GenericRegion<Entry<3, short>>
  GenericRegion<Entry<4, char[10]>, Entry<5, char[10]>>
}

*/
Datatable<AllEntries> datatbl;


void run_kv_table() {
  PRINT_CURRENT_FUNCTION_NAME;
  datatbl.dump_group_info();
  std::string_view expected_value = "hello";
  char value[10]{};
  assert(!datatbl.get_data(4, value));
  assert(datatbl.set_data(4, expected_value.data(), expected_value.length()));
  assert(datatbl.get_data(4, value));
  assert(expected_value == value);
}


int main() {
  run_X_macro();
  run_graph_edsl();
  run_kv_table();
  return 0;
}



