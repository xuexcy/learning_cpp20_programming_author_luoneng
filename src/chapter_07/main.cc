/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/05/14 15:00:33
# Desc   : 第 7 章 Ranges 标准库
########################################################################
*/

#include "cpp_utils/util.h"
#include <concepts>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <ostream>
#include <print>
#include <ranges>
#include <type_traits>
#include <vector>

/*
range 是对一系列数据的抽象
1. 只要有头有尾（对于无穷列表，尾部也是无穷的）, 既可以是有界容器，也可以是无界列表
2. 能够被管道操作符进行组合
3. 延迟计算: 只有在被迭代时才会「按需」计算
*/

void run_range_demo() {
  PRINT_CURRENT_FUNCTION_NAME;
  auto res = std::views::iota(1) | // 从 1 开始生成无穷序列
      std::views::transform([](auto n) { std::print("transform "); return n * n; }) |
      std::views::filter([](auto n) { std::print("filter "); return n % 2 == 1; }) |
      std::views::take_while([](auto n) { std::println(" take_while"); return n < 10000; });
  // 根据输出结果可以看到, 先输出了 before, 然后是 res 的计算过程，然后是 res 的结果
  // 也就是上面的 auto res = xxxx 并没有对 res 进行求值，只有在 res 要使用时才求值，也就是延迟计算
  std::println("before");
  std::println("{}", res);
  std::println("after");
  std::println();
}

// 7.1 range 访问操作
/*
range:
1. 标准库中的容器
2. 普通数组
3. 用户自定义类型
需要一个统一的方式去访问这些 range, 相比成员函数， 更通用的是非成员函数访问(标准库的函数对象)
*/

// 7.1.1 ranges::begin
/*
考虑 r 是一个 range, ranges::begin(r) 返回一个起始的迭代器
1. 若 r 为右值的临时对象，则编译错误
2. 若 r 为数组，返回 r + 0
3. 若 r 存在成员函数 begin 且返回类型满足迭代器要求，则返回 r.begin()
4. 若 r 通过参数依赖查找(ADL) 找到自由函数 begin(r) 且返回类型满足迭代器要求，同时 begin 不是泛函数，则返回 begin(r)
其中第 4 步，必须排除泛函数 begin
ranges 标准库通过函数对象来实现，函数对象也被称为定制点对象(customization point object)
*/

namespace ns {
  struct Foo {
  };  // struct Foo
  void swap(Foo&, Foo&) noexcept {
    PRINT_CURRENT_FUNCTION_NAME;
    std::println("custom swap");
  }
}  // namespace ns

// ranges::swap 是一个函数对象，简要实现如下
namespace my_ranges {
namespace _swap {
  // 这里将泛型的 swap 声明为 delete，即使 adl 在别处找到了泛型的 swap 定义，编译器也会排除它
  template <typename T> void swap(T&, T&) = delete;

  template <typename T, typename U>
  concept adl_swap = true;  // 这里我们先写成 true，表明 adl 永远查找成功

  struct Swap {
    template <typename T, typename U>
    constexpr void operator()(T&& t, U&& u) const noexcept {
      PRINT_CURRENT_FUNCTION_NAME;
      if constexpr (adl_swap<T, U>) {
        // 前面我们假定了 adl 一定会查找成功，所以一定有一个 swap 函数
        swap(static_cast<T&&>(t), static_cast<U&&>(t));
      } else {
        // 在 adl 查找不成功的情况下没有 swap 函数， 我们通过 move 的方式来达到 swap 的目的
        // static_cast<std::remove_reference_t<T>&&>(t) 相当于 std::move(t)
        // 参见 chapter_01/main.cc 中 move 函数
        auto tmp = static_cast<std::remove_reference_t<T>&&>(t);
        t = static_cast<std::remove_reference_t<T>&&>(u);
        u = static_cast<std::remove_reference_t<U>&&>(tmp);
      }
    }
  };  // struct swap
}  // namespace _swap
inline constexpr _swap::Swap swap{};
/*
这是设计模式模板方法模式的等价版本，在一个被称为"模板方法"的方法中定义算法的流程框架, 将算法实现推迟到子类.
这里说的模板和C++模板没有关系
在 swap 中我们设计了算法，也就是:
1. 禁用泛型 swap
2. 通过 ADL 来确定 swap
这样，在不改变整体结构的情况下，我们能够通过子类重新定义算法的某些步骤(我们优先使用用户自定义的swap，然后才是默认的 std::swap)

std::ranges::begin 正如 my_ranges::swap 一样，也是一个函数对象，按照先前说的四步设计调用 begin 的算法
*/
}  // namespace my_ranges

ns::Foo a, b;
void run_foo() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    std::println("call std::swap");
    std::swap(a, b);  // 总是使用标准库中的 swap
  }
  {
    std::println("using std::swap, call ns::swap");
    using std::swap;
    // 先在对象所在空间查找，若未找到，则使用 std::swap
    // 这里会使用 ns::swap
    swap(a, b);
  }
  {
    std::println("std::ranges::swap, call ns::swap");
    // ranges 库可以直接达到上述先在对象空间查找的目的
    std::ranges::swap(a, b);
  }
  {
    std::println("my_ranges::swap, call ns::swap");
    my_ranges::swap(a, b);
  }
  std::println();
}


// 7.1.2 ranges::end
/*
终止迭代器也被称为哨兵 sentinel, 但是不要求起始迭代器和终止迭代器类型一致，这个容器有所不同
c++17 前都要求容器两端的迭代器类型一致
可以看到 run_for_loop 中的循环，使用了 begin != end, 也就是要求两者可以比较
*/
void run_for_loop() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> v{1, 2};
  // 1
  {
    for (auto x : v) { std::cout << x; }
    std::cout << std::endl;
  }
  // 上面这种 for (auto x : v) 在cpp14中会被编译器展开成如下形式(可以通过 https://cppinsights.io/ 将上述代码展开)
  // 2
  {
    // 此时 begin 和 end 是同一个类型
    for (auto begin = v.begin(), end = v.end(); begin != end; ++begin) {
      auto x = *begin;
      std::cout << x << std::endl;
    }
    std::cout << std::endl;
  }
  // 在cpp17中会被编译器展开成如下形式
  {
    // 此时，并不要求 begin 和 end 是同种类型，只要可以比较就行了
    auto begin = v.begin();
    auto end = v.end();
    for ( ; begin != end; ) {
      auto x = *begin;
      std::cout << x << std::endl;
    }
    std::cout << std::endl;
  }
  // 起始迭代器已经有足够的信息用于判断循环是否终止，在多要求一个终止迭代器变量就有些多余
  // 标准库中提供了空类型 std::default_sentinel_t
  // 当我们自定义迭代器时，end() 返回 std::default_sentinel_t 的实例即可
}

// 7.1.3 ranges::size
/*
1. 若为数组类型则返回数组长度
2. 否则,使用成员函数 r.size()
3. 否则,通过 ADL 调用非泛型函数 size(r)
4. 否则, ranges::end(r) - ranges::begin(r)
*/

// 7.1.3 ranges::empty
/*
1. bool(r.empty())
2. ranges::size(r) == 0
3. ranges::begin(r) == ranges::end(r)
*/

// 7.1.5 ranges::data
/*
返回 range 的数据地址
1. 若 r 为右值的临时对象，则编译错误
2. auto ptr = r.data()
3. r.begin()
*/

// 7.2 range 相关概念
// 7.2.1 range
template <typename R>
concept range = requires(R& r) {
  std::ranges::begin(r);
  std::ranges::end(r);
  // begin 和 end 已经约束了迭代器类型
};
static_assert(std::ranges::range<std::vector<int>>);
static_assert(std::ranges::range<std::string>);
static_assert(std::ranges::range<int[10]>);

// 7.2.2 borrowed_range
// 对 range 的引用
template <typename R>
inline constexpr bool enable_borrowed_range = false;
template <typename R>
concept borrowed_range = range<R> && (
    std::is_lvalue_reference_v<R> ||
    enable_borrowed_range<std::remove_cvref_t<R>>);
// enable_borrowed_range 的作用是让用户自己去特化，比如某个类型是一个右值，std::is_lvalue_reference_v<R> 就是
// false, 这是用户就可以自己将 enable_borrowed_range 特化成 true，比如
// template <> inline constexpr bool enable_borrowed_range<R> = true;
template <borrowed_range R>
void f(R&& r) {}
template <>
inline constexpr bool enable_borrowed_range<std::vector<double>> = true;
template <std::ranges::borrowed_range R>
void g(R&& r) {}

void run_borrowed_range() {
  PRINT_CURRENT_FUNCTION_NAME;
  // 编译错误，参数是一个右值
  //  f(std::vector<int>{1, 2, 3, 4});
  // 编译错误，int 不是一个 range
  // f(1);

  // 编译成功，特化了 enable_borrowed_range
  f(std::vector<double>{1.0, 2.0});
  std::vector<int> v{1, 2, 3, 4};
  f(v);

  //  g(std::vector<int>{1, 2, 3});
  // std::string_view 对 std::ranges::enable_borrowed_range 进行了特化
  g(std::string_view{"1234"});
  std::string* s = new std::string("abcd");
  g(std::string_view(s->data(), s->length()));
  // std::string_view 和它所借用的字符串的声明周期之间没有关系
  // 因为字符串存在于静态存储区
  // 即使在另一个线程中将 s 销毁(delete s), 这个 borrowed_range 依然有效

  std::println();
}

// 7.2.3 sized_range
// 要求能通过接口 ranges::size 获取元素数量，并且是常量时间的复杂度
template <typename R>
concept sized_range = range<R> &&
    requires (R r) { std::ranges::size(r); };
// 7.2.4 view
// 在 range 基础上，要求常量时间复杂度内进行移动构造、移动赋值与析构
// template <typename R>
// concept view = range<R> && movable<R> && default_initializable<R> && enable_view<R>;

// 容器都是 range 而不是 view，因为他们的拷贝是 O(n) 的，不是常量时间
// 但 std::string_view 是 view, 因为它可以在常量时间进行拷贝(拷贝数据地址和长度即可)
static_assert(!std::ranges::view<std::vector<int>>);
static_assert(!std::ranges::view<std::string>);
static_assert(std::ranges::view<std::string_view>);
// 在 chapter_01 中使用的 std::span 也是一个 view，它也是只需要拷贝指针和长度信息即可
static_assert(std::ranges::view<std::span<int>>);
static_assert(std::ranges::view<std::span<double>>);
// 延时计算需要使用 range，因为它们按需进行计算并生成迭代值，移动和拷贝的开销都很小

// 7.2.5 其他概念
// common_range 要求起始迭代器和哨兵迭代器类型一致，容器都是 common_range
template <typename R>
concept common_range = range<R> && std::same_as<std::ranges::iterator_t<R>, std::ranges::sentinel_t<R>>;
static_assert(common_range<std::vector<int>>);
static_assert(common_range<std::list<int>>);

// viewable_range 要求一个 range 是一个 view 或者 range 的引用是一个 view
template <typename R>
concept viewable_range = range<R> && (borrowed_range<R> || std::ranges::view<std::remove_cvref_t<R>>);

// 7.3 range 实用组件
// 7.3.1 view_interface
/*
通过 view_interface 实现标准接口，创建自己的 view
实现 begin 和 end 接口，基类的 view_interface 将自动提供像同期一样的接口
*/
// 7.3.2 subrange
// 7.3.3 ref_view

// 7.4 range 工厂
// 7.4.1 empty_view 没有任何元素
auto v1 = std::ranges::empty_view<int>{};
auto v2 = std::views::empty<int>;
auto v3 = std::ranges::views::empty<int>;
static_assert(v1.empty() && v2.empty() && v3.empty());
// 7.4.2 single_view 只有一个元素
auto s_v1 = std::ranges::single_view<int>{6};
auto s_v2 = std::views::single(6);
void run_single_view() {
  PRINT_CURRENT_FUNCTION_NAME;
  for (auto e : s_v1) {
    std::println("single_view s_v1 elem: {}", e);
  }
  for (auto e : s_v2) {
    std::println("single_view s_v2 elem: {}", e);
  }
  std::println();
}

// 7.4.3 iota_view
// 延时计算从初始值开始的连续增加的序列

void run_iota_view() {
  PRINT_CURRENT_FUNCTION_NAME;
  for (auto e : std::ranges::iota_view{0, 5}) {
    std::print("{} ", e);
  }
  std::println();
  for (auto e : std::views::iota(0, 5)) {
    std::print("{} ", e);
  }
  std::println();
  std::println();
}


// 7.4.4 istream_view
void run_istream_view() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    // 相当于 int e; while (cin >> e)
    auto v = std::ranges::istream_view<int>(std::cin);
    for (auto e : v) {
      std::print("{} ", e);
    }
    std::println();
  }

  std::println();
}

// 7.5 range 适配器
/*
1. 适配器将 range 转换成 view 并包含了自定义的计算
2. 适配器通过管道操作符串联起来，最终进行延时计算
*/
// 7.5.1 适配器
void run_adapter() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> v_ints{0, 1, 2, 3, 5};\
  auto even = [] (int i) { return i % 2 == 0; };
  auto square = [](int i) { return i * i; };
  {
    for (auto i : v_ints | std::views::filter(even) | std::views::transform(square)) {
      std::print("{} ", i);
    }
    std::println();
  }
  {
    for (auto i : std::views::filter(v_ints, even) | std::views::transform(square)) {
      std::println("{} ", i);
    }
    std::println();
  }
  std::println();
}

// 7.5.2 all 将 range 转换为 view
// 7.5.3 filter
// 7.5.4 transform
// 7.5.5 take 返回前 N 个元素
void run_take() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> v_ints{0, 1, 2, 3, 4, 5, 6, 7};
  auto first5 /*take_view*/= v_ints | std::views::take(5);
  for (auto i : first5) {
    std::print("{} ", i);
  }
  std::println();
  std::println();
}

// 7.5.6 take_while 直到不满足条件为止
void run_take_while() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> v_ints{0, 1, 2, 3, 4, 5, 6, 7};
  auto take = v_ints | std::views::take_while([](auto n) {return n < 3;});
  for (auto i : take) {
    std::print("{} ", i);
  }
  std::println();
  std::println();
}

// 7.5.7 drop 扔掉前 N 个元素，返回剩余的
void run_drop() {
  PRINT_CURRENT_FUNCTION_NAME;
  auto ints = std::views::iota(0) | std::views::take(10);
  auto latter_half/*drop_view*/ = std::ranges::drop_view{ints, 5};  // 扔掉前面5个
  for (auto i : latter_half) {
    std::print("{} ", i);  // 5 6 7 8 9
  }
  std::println();
  std::println();
}
// 7.5.8 drop_while 扔掉元素知道不满足条件为止
void run_drop_while() {
  PRINT_CURRENT_FUNCTION_NAME;
  constexpr auto source = std::string_view(" \t \t \t hello there");
  auto is_invisible = [](auto x) { return x == ' ' || x == '\t';};
  auto skip_ws = source | std::views::drop_while(is_invisible);
  for (auto i : skip_ws) {
    std::print("{}", i);  // "hello there"
  }
  std::println();
  std::println();

}

// 7.5.9 join 接受一个range 的 range，将它们平铺
void run_join() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<std::string_view> ss{"hello", " ", "world", "!"};
  auto greeting = ss | std::views::join;
  for (auto ch : greeting/*join_view*/) {
    std::print("{}", ch);
  }
  std::println();
  std::println();
}
// 7.5.10 split 使用分隔符得到一系列 view 的 split_view
void run_split() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::string_view str{"the quick brown fox"};
  auto sentence = str | std::views::split(' ');
  for (auto word : sentence/*split_view*/) {
    for (auto ch : word) {
      std::print("{}", ch);
    }
    std::print("* ");
  }
  std::println();
  std::println();
}
// 7.5.11 common 返回同类型的起始和哨兵迭代器 common_view. 用于适配传统标准库中的算法
// 7.5.12 reverse 返回 reverse_view
void run_reverse() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector<int> v_ints{0, 1, 2};
  auto rv = v_ints | std::views::reverse;
  for (auto i : rv/* reverse_view */) {
    std::print("{} ", i);
  }
  std::println();
  std::println();
}

// 7.5.13 elements 接受数 N, 返回第 N 个元素形成的 element_view
void run_elements() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::map<std::string_view, long> historical_figures {
    {"Lovelace", 1815}, {"Turing", 1912},
    {"Babbage", 1791}, {"Hamilton", 1936}
  };
  auto names = historical_figures | std::views::elements<0>;
  for (auto&& name : names /* element_view */) {
    std::print("{} ", name);
  }
  std::println();

  auto birth_years = historical_figures | std::views::elements<1>;
  for (auto&& birth_year : birth_years /* element_view */) {
    std::print("{} ", birth_year);
  }
  std::println();
  // 标准库定义了 keys 和 values 来代表 elements<0> 和 element<1>
  // 因为这两个比较常见
  static_assert(std::is_same_v<decltype(std::views::keys), decltype(std::views::elements<0>)>);
  static_assert(std::is_same_v<decltype(std::views::value), decltype(std::views::elements<1>)>);

  std::println();
}

// 7.6 其他改善
// 7.6.1 迭代器概念
int main() {
  run_range_demo();
  run_foo();
  run_borrowed_range();
  run_iota_view();
  // run_istream_view();
  run_take();
  run_take_while();
  run_drop();
  run_drop_while();
  run_join();
  run_split();
  run_reverse();
  run_elements();
  return 0;

}
