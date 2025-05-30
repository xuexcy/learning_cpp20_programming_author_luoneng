/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/05/17 23:35:18
# Desc   : 第 2 章 编译时多态
########################################################################
*/

#include <cmath>
#include <forward_list>
#include <cassert>
#include <iterator>
#include <limits>
#include <list>
#include <span>
#include <type_traits>
#include <vector>

#include "cpp_utils/util.h"
// 2.1 函数重载机制
/*
函数重载机制三个阶段
1. 名称查找
2. 模板函数处理
3. 重载决议
前两个阶段得到函数候选集，最后一个接入选取最合适的版本
*/
namespace animal {
struct Cat {
};  // struct Cat
void feed(Cat* foo, int) {
  PRINT_CURRENT_FUNCTION_NAME;
}
}  // namespace animal
struct CatLike {
  CatLike(animal::Cat*);
};  // struct CatLike
void feed(CatLike* foo) {
  PRINT_CURRENT_FUNCTION_NAME;
}
template <typename T>
void feed(T* obj, double) {
  PRINT_CURRENT_FUNCTION_NAME;
}
template <>  // 全特化
void feed(animal::Cat* obj, double) {
  PRINT_CURRENT_FUNCTION_NAME;
}
void run_feed() {
  PRINT_CURRENT_FUNCTION_NAME;
  animal::Cat cat;
  feed(&cat, 1);
  std::println();
}

// 2.1.1 名称查找
/*
候选函数如下
void animal::feed(Cat* foo, int);
void feed(CatLike);
template <typename T> void feed(T*, double);

在名称查找过程中仅考虑普通函数和主模板函数，不会考虑其他所有的特化版本，只有第三阶段选择最合适版本时才会考虑特化版本
*/

// 2.1.2 模板函数处理
// 实例化模板函数
// void feed<animal::Cat>(animal::Cat*, double);

// 2.1.3 重载决议
/*
1. 参数数量、变长参数、默认参数
2. 参数类型
3. c++20 起，约束
4. 隐式转换
*/

// 2.1.4 注意事项
// 函数重载不考虑返回类型，因为返回类型是可选的，用户使用时可能不会处理返回值

// 2.1.5 再谈 SFINAE


// 2.2 类型特征(Type traits)
// 2.2.1 Type traits 谓词与变量模板
// 2.2.2 类型变换
SAME_TYPE(std::remove_const_t<const int>, int);
SAME_TYPE(std::remove_const_t<int>, int);
SAME_TYPE(std::add_const_t<int>, const int);
SAME_TYPE(std::add_pointer_t<int*>, int**);
SAME_TYPE(std::decay_t<int[5][6]>, int(*)[6]);  // 指向数组的指针，而不是二级指针

// 数组退化为指针会丢失长度信息，cpp20 提供 std::span 来同时传递指针和长度
void pass_array_like(std::span<int> container) {
  std::println("container size: {}", container.size());
}
void run_span() {
  PRINT_CURRENT_FUNCTION_NAME;
  int arr[] = {1, 2, 3, 4, 5};
  std::vector<int> v{1, 2};
  std::array arr2{1, 2, 3};
  pass_array_like(arr);
  pass_array_like(v);
  pass_array_like(arr2);
  std::println();
}

// 2.2.3 辅助类
// 将一个值转换成类，这个类能转回值，实现值和类型的映射
using Two = std::integral_constant<int, 2>;
using Four = std::integral_constant<int, 4>;
static_assert(Two::value * Two::value == Four::value);
template <typename T, T v>
struct IntegralConstant {
  using type = IntegralConstant;
  using value_type = T;
  static constexpr T value = v;
};  // struct IntegralConstant

// 2.2.4 空基类优化
// https://github.com/xuexcy/learning_more_cpp_idioms/blob/main/src/empty_base_optimization.cc

// 空类占用一字节
struct Base {
};  // struct Base
static_assert(sizeof(Base) == 1);

struct Children {
  Base base;
  int other;
};  // struct Children
static_assert(sizeof(Children) == 8);  // 由于对齐原因，Children 占用 8 字节

// 如果使用继承，那么空基类将占用 0 个字节
struct Children2 : Base {
  int other;
};  // struct Children2
static_assert(sizeof(Children2) == 4);
// clang -Xclang -fdump-record-layouts -fsyntax-only main.cc

/*
对于 std::vector 的模板参数 allocator 来说，使用的默认值是无状态的空类，vector 通过继承这个 allocator 的
方式来实现空基类优化.
但是，如果用户提供了一个 allocator
  1. 这个 allocator 可能和 vector 的成员变量产生冲突(相同的变量名)
  2. 这个 allocator 可能是 final 而无法被继承
cpp20 提供 [[ no_unique_address ]] 来避免这个问题，当该成员是空类时，依然可以被优化
*/
struct Children3 {
  [[no_unique_address]] Base base;
  int other;
};  // struct Children3
static_assert(sizeof(Children3) == 4);

// 2.2.5 实现 Type traits
// 定义一个基本的模板类，令其默认返回 false, 然后枚举出有限的类型返回 true

// 实现 is_floating_point
// 步骤 1: 定义一个基本的模板类，令其默认返回 false
template <typename T>
struct is_floating_point : std::false_type {
};  // struct is_floating_point
// 步骤 2: 枚举出有限的类型返回 true
template <> struct is_floating_point<float> : std::true_type {};
template <> struct is_floating_point<double> : std::true_type {};
template <> struct is_floating_point<long double> : std::true_type {};

// 实现 is_same
template <typename T, typename U>
struct is_same : std::false_type {
};  // struct is_same
template <typename T>  // 偏特化
struct is_same<T, T> : std::true_type {
};  // struct is_same

// 实现 remove_const
template <typename T>
struct remove_const {
  using type = T;
};  // struct remove_const
template <typename T>
struct remove_const<const T> {
  using type = T;
};  // struct remove_const<const T>

// 实现 conditional
template <bool v, typename Then, typename Else>
struct conditional{
  using type = Then;
};  // struct conditional
template <typename Then, typename Else>
struct conditional<false, Then, Else>{
  using type = Else;
};  // struct conditional<false, Then, Else>
template <bool v, typename Then, typename Else>
using conditional_t = typename conditional<v, Then, Else>::type;

// 使用 conditional 实现 decay
/*
1. 数组类型 -> 退化成一维指针
2. 函数类型 -> 退化成函数指针
3. 去掉 cv 限定符
*/
template <typename T>
struct decay {
  using U = std::remove_reference<T>;
  using type = conditional_t<std::is_array_v<U>,
      std::remove_extent_t<U>*,  // 数组类型 -> 退化成一维指针
      conditional_t<std::is_function_v<U>,
          std::add_pointer_t<U>,  // 函数类型 -> 退化成函数指针
          std::remove_cv_t<U>
      >
  >;
};  // struct decay

// 标准库中的 type traits 并不全能 C++ 语法实现，有些只能通过编译器开洞来实现
// 比如 is_abstract, 只有编译器知道一个类是否是抽象类

// 2.2.5 类型内省
// 内省是程序运行时检查对象类型或属性的一种能力
// type traits 是在编译时做内省

// 获取 array 的大小
template <typename T> struct array_size;
template <typename E, std::size_t N>  // 偏特化
struct array_size<E[N]> {
  using value_type = E;
  static constexpr std::size_t len = N;
};  // struct array_size<E[N]>

SAME_TYPE(int, array_size<int[5]>::value_type);
static_assert(5 == array_size<int[5]>::len);

template <typename T> struct function_trait;
template <typename R, typename... Args>
struct function_trait<R(Args...)> {
  using result_type = R;
  using args_type = std::tuple<Args...>;
  static constexpr size_t num_of_args = sizeof...(Args);
  template <size_t I>
  using arg = std::tuple_element_t<I, args_type>;
};  // struct function_traits<R(Args...)>
using F = void(int, float, std::vector<char>);
SAME_TYPE(function_trait<F>::result_type, void);
static_assert(function_trait<F>::num_of_args == 3);
SAME_TYPE(function_trait<F>::arg<1>, float);

// 2.2.7 enable_if 函数
template <bool, typename = void> struct enable_if {};
template <typename T> struct enable_if<true, T> {
  using type = T;
};  // struct enable_if<true, T>
template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <typename T, enable_if_t<std::is_integral_v<T>>* = nullptr>
bool num_eq(T lhs, T rhs) {
  return lhs == rhs;
}
template <typename T, typename = enable_if_t<is_floating_point<T>::value>>
bool num_eq(T lhs, T rhs) {
  return std::fabs(lhs - rhs) < std::numeric_limits<T>::epsilon();
}
/*
对于 num_eq 的实现方式
1. 可以重载所有所有类型，但是比较繁琐
2. 只写一个不受任何约束的模板函数，但是，一旦用户传递了错误的类型，就会得到模糊的编译错误信息
3. 使用 enable_if, 这样可以减少需要重载的数量，比如所有的整型都使用了同一个声明，否则按第 1 中方式，
    需要给 int/unsigned int/long/char 等都提供一个重载函数

    不过，enable_if 会让声明看起来很复杂，影响使用者辨别真正的模板参数
*/

// 2.2.8 标签分发 tag dispatching
// 将不同的标签，比如 std::true_type 和 std::false_type, 作为重载函数的最后一个参数
template <typename T>
bool num_eq_impl(T lhs, T rhs, std::true_type) {
  return std::fabs(lhs - rhs) < std::numeric_limits<T>::epsilon();
}
template <typename T>
bool num_eq_impl(T lhs, T rhs, std::false_type) {
  return lhs == rhs;
}
template <typename T>  // 标签分发，在是 arithmetic 前提下，区分 float 和非 float
auto num_eq(T lhs, T rhs) -> enable_if_t<std::is_arithmetic_v<T>, bool> {
  return num_eq_impl(lhs, rhs, std::is_floating_point<T>());
}

template <typename InputIter>
void advance_impl(InputIter& iter,
    typename std::iterator_traits<InputIter>::difference_type n,
    std::input_iterator_tag) {
  PRINT_CURRENT_FUNCTION_NAME;
  for (; n > 0; --n) {
    ++iter;
  }
}
template <typename BidirectionalIter>
void advance_impl(BidirectionalIter& iter,
    typename std::iterator_traits<BidirectionalIter>::difference_type n,
    std::bidirectional_iterator_tag) {
  PRINT_CURRENT_FUNCTION_NAME;
  if (n >= 0) {
    for (; n > 0; --n) {
      ++iter;
    }
  } else {
    for (; n < 0; ++n) {
      --iter;
    }
  }
}
template <typename RandomIter>
void advance_impl(RandomIter& iter,
    typename std::iterator_traits<RandomIter>::difference_type n,
    std::random_access_iterator_tag) {
  PRINT_CURRENT_FUNCTION_NAME;
  iter += n;
}
template <typename InputIter>
void advance(InputIter& iter, typename std::iterator_traits<InputIter>::difference_type n) {
  return advance_impl(iter, n, typename std::iterator_traits<InputIter>::iterator_category());
}

void run_advance() {
  PRINT_CURRENT_FUNCTION_NAME;
  {  // forward_iterator_tag
    std::forward_list<int> fl{1, 2, 3, 4, 5};
    SAME_TYPE(decltype(fl)::iterator::iterator_category, std::forward_iterator_tag);

    auto iter = fl.begin();
    int N = 2;
    ::advance(iter, N);
    std::println("forward_list[{}] : {}", N, *iter);  // 3
    ::advance(iter, -N);
    std::println("forward_list[{}] : {}", 0, *iter);  // 这里还是 3, 因为 forward_iterator 不能倒退
  }
  {  // bidirectional_iterator_tag
    std::list<int> l{1, 2, 3, 4, 5};
    SAME_TYPE(decltype(l)::iterator::iterator_category, std::bidirectional_iterator_tag);

    auto iter = l.begin();
    int N = 2;
    ::advance(iter, N);
    std::println("list[{}] : {}", N, *iter);  // 3
    ::advance(iter, -N);
    std::println("list[{}] : {}", 0, *iter);  // 1
  }
  {  // random_access_iterator_tag
    std::vector<int> v{1, 2, 3, 4, 5};
    SAME_TYPE(decltype(v)::iterator::iterator_category, std::random_access_iterator_tag);

    auto iter = v.begin();
    int N = 2;
    ::advance(iter, N);
    std::println("vector[{}] : {}", N, *iter);  // 3
    ::advance(iter, -N);
    std::println("vector[{}] : {}", 0, *iter);  // 1
  }
  std::println();
}

// 2.2.9 if constexpr
template <typename T>
auto num_eq(T lhs, T rhs) -> std::enable_if_t<std::is_arithmetic_v<T>, bool> {
  if constexpr (std::is_integral_v<T>) {
    return lhs == rhs;
  } else {
    return std::fabs(lhs - rhs) < std::numeric_limits<T>::epsilon();
  }
}
template <typename InputIter>
void advance2(InputIter& iter, typename std::iterator_traits<InputIter>::difference_type n) {
  using Category = typename std::iterator_traits<InputIter>::iterator_category;
  if constexpr (std::is_base_of_v<std::random_access_iterator_tag, Category>) {
    iter += n;
  } else if constexpr (std::is_base_of_v<std::bidirectional_iterator_tag, Category>) {
    if (n >= 0) {
      for (; n > 0; --n) ++iter;
    } else {
      for (; n < 0; ++n) --iter;
    }
  } else {
    for (; n > 0; --n) ++iter;
  }
}

// 2.2.10 void_t 元函数
template <typename...>
using my_void_t = void;

template <typename T, typename = void>  // void = std::void_t<>
struct HasTypeMember : std::false_type {
};  // struct HasTypeMember
template <typename T>
struct HasTypeMember<T, std::void_t<typename T::type>> : std::true_type {
};  // struct HasTypeMember<T, void_t<typename T::type>>
static_assert(!HasTypeMember<int>::value);  // int 没有 type 成员类型
static_assert(HasTypeMember<std::true_type>::value);


template <typename T, typename = void>
struct HasInit : std::false_type {
};  // struct HasInit
template <typename T>
struct HasInit<T, std::void_t<decltype(declval<T>().Init())>> : std::true_type {
};  // struct HashInit

// 2.3 奇异递归模板 CRTP curiously recurring template pattern
// 将 Derived 作为 Base 的模板参数，从而让 Base 可以使用 Derived 提供的方法
/*
主要用途:
1. 代码复用: 继承类复用基类的方法
2. 编译时多态: 基类可以使用继承类的方法, 没有虚函数表开销
template <typename T>
struct Base {
};  // struct Base
struct Derived : Base<Derived> {
};  // struct Derived
*/

// 2.3.1 代码复用
class VideoFile;
class TextFile;
struct Visitor {
  virtual void visit(VideoFile&) = 0;
  virtual void visit(TextFile&) = 0;
  virtual ~Visitor() = default;
};  // struct Visitor
struct Elem {
  virtual void accept(Visitor& visit) = 0;
  virtual ~Elem();
};  // struct Elem
struct VideoFile : Elem {
  void accept(Visitor& visit) override {
    visit.visit(*this);
  }
};  // struct VideoFile
struct TextFile: Elem {
  void accept(Visitor& visit) override {
    visit.visit(*this);
  }
};  // struct TextFile
/*
一个 Visitor 可以对 VideoFile 和 TextFile 进行访问
一个 Elem 使用 accept 接受一个 Visitor 让它访问自己
每个继承了 Elem 并实现的 accept 都是一样的，但是没办法在 Elem 中统一实现，因为 this 在基类中没有多态性
每个继承类的差别就在于 this 的类型
*/
template <typename Derived>
struct AutoDispatchElem : Elem {
  void accept(Visitor& visitor) override {
    visitor.visit(static_cast<Derived&>(*this));
  }
};  // struct AutoDispatchElem
/*
继承关系 Elem -> AutoDispatchElem -> VideoFile2
解决问题的关键在于，AutoDispatchElem 可以通过模板参数 Derived 知道谁继承了它
这样 AutoDispatchElem 就可以在 accept 的定义中将 *this 转换成 Derived&
*/
struct VideoFile2 : AutoDispatchElem<VideoFile2> {
};  // struct VideoFile2
struct TextFile2 : AutoDispatchElem<TextFile2>{
};  // struct TextFile2

template <typename Derived>
struct Comparable {
  friend bool operator==(const Derived& lhs, const Derived& rhs) {
    return lhs.tie() == rhs.tie();
  }
  friend bool operator<(const Derived& lhs, const Derived& rhs) {
    return lhs.tie() < rhs.tie();
  }
};  // struct Compareable
struct Point : Comparable<Point> {
  int x;
  int y;
  Point(int x, int y) : x(x), y(y) {}
  auto tie() const { return std::tie(x, y); }
};  // struct Point

// cpp20 三路比较操作符 three-way comparison operator
struct Point2 {
  int x;
  int y;
  friend auto operator<=>(const Point2& lhs, const Point2& rhs) = default;
};  // struct Point2

// 2.3.2 静态多态
template <typename Derived>
struct Animal {
  void bark() { static_cast<Derived&>(*this).bark_impl(); }
};  // struct Animal
class Cat : public Animal<Cat> {
  friend Animal;  // class Cat 的 bark_impl 为 private, 但对 Animal 可见
  void bark_impl() {
    PRINT_CURRENT_FUNCTION_NAME;
    std::println("Miaowing!");
  }
};  // struct Cat
class Dog : public Animal<Dog> {
  friend Animal;  // class Dog 的 bark_impl 为 private, 但对 Animal 可见
  void bark_impl() {
    PRINT_CURRENT_FUNCTION_NAME;
    std::println("Wang Wang!");
  }
};  // struct Dog

template <typename T>
void play(Animal<T>& animal) {
  PRINT_CURRENT_FUNCTION_NAME;
  std::println("let's go to play");
  animal.bark();
  std::println("let's go home");
}
void run_crtp() {
  PRINT_CURRENT_FUNCTION_NAME;
  Cat c;
  Dog d;
  play(c);
  play(d);
  std::println();
}

// 2.3.3 enable_shared_from_this 模板类

// 2.4 表达式模板
// https://github.com/xuexcy/learning_more_cpp_idioms/tree/main/src/expression_template_deps

// 2.4.1 标量延迟计算
template <typename T, typename U, typename Op>
struct BinaryExpression {
  BinaryExpression(const T& lhs, const U& rhs, Op op): lhs(lhs), rhs(rhs), op(op) {}
  auto operator()() const {
    return op(lhs, rhs);
  }
protected:
  T lhs;
  U rhs;
  Op op;
};  // struct BinaryExpression
// 2.4.2 向量延迟计算
template <typename T, typename U, typename Op>
struct BinaryContainerExpression : private BinaryExpression<T, U, Op> {
  using Base = BinaryExpression<T, U, Op>;
  using Base::Base;
  auto operator[](std::size_t index) const {
    assert(index < size());
    return Base::op(Base::lhs[index], Base::rhs[index]);
  }
  std::size_t size() const {
    // 这个 assert 最好在构造函数中调用, 并在构造函数中记录 size, 避免每次都调用 lhs.size()
    assert(Base::lhs.size() == Base::rhs.size());
    return Base::lhs.size();
  }
};  // struct BinaryContainerExpression

// 类模板参数推导规则, 通过构造时的参数类型推导类模板参数类型
// 继承关系的存在使得编译器无法自动获取这个规则，所以要主动写这个推导规则
template <typename T, typename U, typename Op>
BinaryContainerExpression(T, U, Op) -> BinaryContainerExpression<T, U, Op>;

void run_expression_template() {
  PRINT_CURRENT_FUNCTION_NAME;
  {
    auto plus = [](auto x, auto y) { return x + y; };
    BinaryExpression expr(5, 3.5, plus);
    // 此时只记录了表达式，并没有计算，只有调用 expr 的 operator() 才会进行计算
    std::println("{}", expr() * 2.0);  // (5 + + 3.5) * 2  =17
  }
  {
    std::vector<int> x{1, 2, 3};
    std::vector<int> y{3, 2, 1};
    int alpha = 4;
    auto add_scaled = [alpha](auto lhs, auto rhs) {
      return lhs + alpha * rhs;
    };
    auto expr = BinaryContainerExpression(x, y, add_scaled);
    for (std::size_t i = 0; i < expr.size(); ++i) {
      std::println("{} + {} * {} = {}", x[i], y[i], alpha, expr[i]);
    }
  }

  std::println();
}

// 2.4.3 提高表达力
// 重载 operator: 参见https://github.com/xuexcy/learning_more_cpp_idioms/tree/main/src/expression_template_deps
int main() {
  run_feed();
  run_span();
  run_advance();
  run_crtp();
  run_expression_template();
  return 0;
}



