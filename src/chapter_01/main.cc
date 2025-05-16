/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/05/12 18:45:04
# Desc   : 第 1 章 类型与对象
########################################################################
*/
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <print>
#include <span>
#include <type_traits>
#include <vector>

#include <gsl/gsl>
#include "cpp_utils/util.h"

// std::span: 连续内存引用视图, 不拥有数据，但可修改数据(std::string_view 不可以修改数据)
// 在不使用 std::span 时，参数类型可能需要为 (T* t, size_t len), 然后函数中需要判断 t != nullptr
template <typename T>
void print_span(T* t, size_t len) {
  if (t == nullptr) { return; }
  for (auto i = 0; i < len; ++i) {
    std::print("{}", t[i]);
  }
  std::println();
}

template <typename T>
void print_span(std::span<T> s) {
  for (auto&& elem : s) {
    std::print("{}", elem);
  }
  std::println();
}

void print_char_span(const char* p, size_t len) {
  print_span(p, len);
  std::span<const char> s(p, len);
  print_span(s);
}

void run_span() {
  PRINT_CURRENT_FUNCTION_NAME;
  char c1[] = "abcdef";
  print_char_span(c1, 3);
  const char* c2 = "hijklm";
  print_char_span(c2, 4);

  std::vector<int> v{1, 2, 3, 4, 5};
  std::span<int> s(v.begin(), 2);
  print_span(s);
  print_span<int>({v.begin(), v.end()});  // 1 2 3 4 5
  // 修改数据
  *s.begin() = 2;
  print_span<int>({v.begin(), v.end()});  // 2 2 3 4 5
  std::println("{} {} {} {} {}",
      *(v.begin() + 2),
      s.subspan(2).front(),
      s.last(0).front(),
      s.first(3).back(),
      s[2]);
  int c_arr[] = {1, 2, 3};
  print_span<int>({c_arr, std::size(c_arr)});

  std::println();
}

// gsl::not_null: 在保证函数通过指针传递的参数非空的情况下，可以使用 gsl::not_null<T*>, 这样就不用每次都对指针判空
void print(gsl::not_null<int*> data) {
  std::println("{}", *data);
}
void run_not_null() {
  PRINT_CURRENT_FUNCTION_NAME;
  int i  = 32;
  print(&i);
  std::println();
}

// 1.3 值类别
// 值类别指的是表达式结果的类型，而不是对象、变量或类型的类别
// 1.3.1 理解左值与右值
void foo(int&) { PRINT_CURRENT_FUNCTION_NAME; }
using FooType1 = decltype(foo(std::declval<int&>()));
void foo(int&&) { PRINT_CURRENT_FUNCTION_NAME; }
using FooType2 = decltype(foo(std::declval<int&&>()));
void run_foo() {
  PRINT_CURRENT_FUNCTION_NAME;
  int&& value = 5;
  // 能取址就是左值，比如 value 是个变量，它是有地址的
  foo(value);  // 调用 foo(int&), 表达式 value 是一个右值引用类型
  // static_assert(std::is_same_v<decltype(foo(value)), FooType1>);
  SAME_TYPE(decltype(foo(value)), FooType1);
  foo(5);  // 调用 foo(int&&), 5 是一个右值
  SAME_TYPE(decltype(foo(5)), FooType2);
  foo(static_cast<int&&>(value));  // 调用 foo(int&&)
  SAME_TYPE(decltype(foo(static_cast<int&&>(value))), FooType2);
  std::println();
}

// 1.3.2 函数形参合适使用何种引用
/*引用传参
1. Value&
2. const Value&
3. Value&&: 拥有对 value 对了的控制权
*/
struct GNode {
  using _TCache = std::map<const GNode*, GNode*>;
  GNode* clone(_TCache&& cache = {}) const {
    if (auto iter = cache.find(this); iter != cache.end()) {
      return iter->second;
    }
    auto node = new GNode(*this);
    cache.emplace(this, node);
    for (auto child : children_) {
      node->children_.emplace_back(child->clone(static_cast<_TCache&&>(cache)));
    }
    return node;
  }
  std::vector<GNode*> children_;
};  // struct GNode

// GNode::clone 的参数是一个临时右值，中途可以被修改，相当于建了一个临时变量，一直递归往下传
// 如果不使用临时右值，我们可能需要两个 clone 函数，一个给用户调用，一个内部传递
struct GNode2 {
  using _TCache = std::map<const GNode2*, GNode2*>;
public:
  GNode2* clone() const {
    _TCache cache;
    return clone(&cache);
  }
private:
  GNode2* clone(_TCache* cache) const {
    if (auto iter = cache->find(this); iter != cache->end()) {
      return iter->second;
    }
    auto node = new GNode2(*this);
    cache->emplace(this, node);
    for (auto child : children_) {
      node->children_.emplace_back(child->clone(cache));
    }
    return node;
  }
  std::vector<GNode2*> children_;
};  // struct GNode2

// 1.3.3 转发引用与完美转发
template <typename T, typename Arg>
std::unique_ptr<T> make_unique(Arg& arg) {
  PRINT_CURRENT_FUNCTION_NAME;
  return std::unique_ptr<T>(new T(arg));
}
// 如果 T 的构造只接受左值引用，那么上述 make_unique 将无法使用，因为其参数是常引用
// 那样的话我们可能需要一个如下的重载版本
template <typename T, typename Arg>
std::unique_ptr<T> make_unique(Arg&& arg) {
  PRINT_CURRENT_FUNCTION_NAME;
  // 即使 Arg 是右值类型，变量 arg 在作为表达式传递给 new T 时，表达式 arg 依然是左值(参考前面的 foo 函数调用),
  // 所以需要 std::forward 将 new T(arg) 中的表达式 arg 转换成为 Arg&& 类型,
  return std::unique_ptr<T>(new T(arg));
}
// 假设有 N 个参数，那么我们就需要 2^N 个重载版本

template <class T>
inline constexpr T&& forward(std::remove_reference_t<T>& t) noexcept {
  PRINT_CURRENT_FUNCTION_NAME;
  std::println("hi");
  return static_cast<T&&>(t);
}
// cpp11 引用变参模板特性解决多参数版本问题
// 转发引用解决同一个参数的不同引用类型导致的多个重载问题
template <typename T, typename Arg>
std::unique_ptr<T> make_unique_with_forward(Arg&& arg) {
  PRINT_CURRENT_FUNCTION_NAME;
  return std::unique_ptr<T>(new T(forward<Arg>(arg)));
}

template <class T>
inline constexpr T&& forward2(T& t) noexcept {
  PRINT_CURRENT_FUNCTION_NAME;
  return static_cast<T&&>(t);
}

template <typename T, typename Arg>
std::unique_ptr<T> make_unique_with_forward2(Arg&& arg) {
  PRINT_CURRENT_FUNCTION_NAME;
  return std::unique_ptr<T>(new T(forward2<Arg>(arg)));
}

struct A {
  int value;
  A(int&& i): value(i) {
    PRINT_CURRENT_FUNCTION_NAME;
  }
};  // class A

struct B {
  int value;
  B(int& i): value(i) {
    PRINT_CURRENT_FUNCTION_NAME;
  }
  B(int&& i): value(i + 1) {
    PRINT_CURRENT_FUNCTION_NAME;
  }
  int get() && {
    return value;
  }
  int& get() & {
    return value;
  }
};  // struct B
int get_int() {
  return 1;
}


void run_make_unique() {
  PRINT_CURRENT_FUNCTION_NAME;
  int i{0};
  {
    // No matching constructor for initialization of 'A'
    // make_unique<A>(1);
    // No matching constructor for initialization of 'A'
    // make_unique<A>(i);

    // A 只能接受右值作为构造参数，也就是我们期望 make_unique<A>(1) 是可以编译通过的
    // 但是，不论是参数 (1) 还是参数 (i), 在 make_unique 函数中调用 new T(arg), 表达式 arg 都是一个左值

    auto b1 = make_unique<B>(i);   // 左值构造
    assert(i == b1->value);
    auto b2 = make_unique<B>(1);  // 还是左值构造
    assert(1 == b2->value);
    std::println("------------");
  }
  {
    make_unique_with_forward<A>(1);  // 右值构造，移动构造函数
    make_unique_with_forward<A>(get_int());  // 右值构造，移动构造函数
    int t = 1;
    make_unique_with_forward<A>(std::move(t));
    // make_unique_with_forward<A>(i);
    auto b1 = make_unique_with_forward<B>(i);   // 左值构造
    assert(i == b1->value);
    auto b2 = make_unique_with_forward<B>(1);  // 右值构造
    assert(1 + 1 == b2->value);
    std::println("------------");
  }
  {
    make_unique_with_forward2<A>(1);  // 右值构造，移动构造函数
    int t = 1;
    make_unique_with_forward<A>(std::move(t));
    // make_unique_with_forward2<A>(i);
    auto b1 = make_unique_with_forward2<B>(i);   // 左值构造
    assert(i == b1->value);
    auto b2 = make_unique_with_forward2<B>(1);  // 右值构造
    assert(1 + 1 == b2->value);
    std::println("------------");
  }
  std::println();
}


/*
为什么需要 std::forward ?
参考前面的 foo 函数调用，不论 make_unique_2 的参数 arg 是左值还是右值，在将这个参数 arg 作为传递给 new T(arg) 时，
new T(arg) 中的 arg 这个表达式永远是左值, 也就是 new T 永远接受了左值. 我们需要是，在表达式 arg 永远是左值的情况下，
如果参数 arg 是左值，我们就通过一个函数 std::forward 得到左值并将它传递给 new T,
如果参数 arg 是右值，我们就通过 std::forward 得到右值并将它传递给 new T,
而不是永远将一个左值表达式 arg 传递给 new T

如何实现 std::forward ?
从 make_unique_2 的声明中，我们得到了两个和参数相关的信息，一个是参数类型 Arg, 一个是参数值 arg, 我们已经知道，
将参数 arg 传递给其他函数时，表达式 arg 一定是左值，也就是我们无法实现一个函数仅通过表达式 arg 来还原参数 arg 的左右值情况.
因此，我们只能从参数类型 Arg 下手，这也就是在调用 std::forward 时为什么一定要指定模板参数的原因(如果不指定模板参数，
我们就只能依赖左值表达式 arg 来推导模板参数，但是我们已经知道无法通过一个左值来推导其原始类型是左值还是右值).

从参数类型 Arg 下手，我们就可以知道，只要 std::forward 函数返回类型是 Arg&& 就行了, 而表达式 arg 是一个左值，所以只需要
声明
template <typename T>
T&& forward(std::remove_reference_t<T>& t)
或
template <typename T>
T&& forward2(T& t)
即可

这里的两种写法都接受了左值引用，因为表达式 arg 一直都是左值
*/


/*
但是，在标准库的 std::forward 实现中，我们还可以看到接受 (std::remove_reference_t<T>&& t) 参数的重载, 即
template <typename T>
T&& forward(std::remove_reference_t<T>&& t);

既然表达式 arg 永远是左值，那我们为什么需要一个接受右值类型的重载版本呢? 答案在 cppreference.com 中有也写, 即
  This overload makes it possible to forward a result of an expression (such as function call),
  which may be rvalue or lvalue, as the original value category of a forwarding reference argument.
  这种重载使得可以转发表达式的结果（例如函数调用）, 作为转发引用参数的原始值类别，它可以是rvalue或lvalue。
也就是说，我们并不一定就是只转发一个左值表达式，也有可能转发一个表达式的结果，比如针对上面的 struct B
B(1).get() 就是就是一个右值, 而
int i{0}; B(i).get() 就是一个左值
当 new T(arg) 时， arg 永远时左值，但是，当 new T(arg.get()) 时，arg.get() 可能是左值，也可能是右值，这时候就
可能会使用到接受 std::remove_reference_t<T>&& 的 std::forward 重载版本.
参考:
  https://en.cppreference.com/w/cpp/utility/forward
  https://zhuanlan.zhihu.com/p/705380238
样例如下
*/
template <class T>
inline constexpr T&& forward(std::remove_reference_t<T>&& t) noexcept {
  PRINT_CURRENT_FUNCTION_NAME;
  static_assert(!std::is_lvalue_reference_v<T>, "cannot forward an rvalue as an lvalue");
  return static_cast<T&&>(t);
}

template <class T>
inline constexpr T&& forward2(T&& t) noexcept {
  PRINT_CURRENT_FUNCTION_NAME;
  static_assert(!std::is_lvalue_reference_v<T>, "cannot forward an rvalue as an lvalue");
  return static_cast<T&&>(t);
}

template<typename T, typename Arg>
std::unique_ptr<T> make_unique_with_forward3(Arg&& arg) {
  // 这里我们想要将 arg.get() 进行转发, 所以需要知道 arg.get() 的类型,
  // 宏 EXPRESSION_OF_GET 可以对 arg 进行转发并调用 get()
  // 这样 type_of_get 就可以得到调用结果的类型
  #define EXPRESSION_OF_GET forward<Arg>(arg).get()
  // 注意，这个宏调用的是左值版本，因为表达式 arg 是左值
  using type_of_get = decltype(EXPRESSION_OF_GET);

  // 这里宏 EXPRESSION_OF_GET 依然是调用的接受左值的版本，但是这里的 forward 就可能是接受右值的版本
  // 于是我们可以从输出中看到两种情况：
  //    1. 如果 Arg&& 是左值引用,则先通过宏调用一次接受左值的版本(T = Arg&),再调用一次接受左值的版本(T = type_of_get)
  //    2. 如果 Arg&& 是右值引用,则先通过宏调用一次接受左值的版本(T = Arg),再调用一次接受右值的版本(T = type_of_get)
  return std::unique_ptr<T>(new T(forward<type_of_get>(EXPRESSION_OF_GET)));
  //  return std::make_unique(new T(forward<decltype(forward<T>(arg).get())>(forward<T>(arg).get())));
}

void run_make_unique_with_get() {
  PRINT_CURRENT_FUNCTION_NAME;
  B b(1);
  auto _ = make_unique_with_forward3<int>(b);   // 左值构造一个 int
  auto _ = make_unique_with_forward3<int>(B(1));  // 右值构造一个 int
  std::println("------------");
}

/*
从 run_make_unique 中可以看到, 不论是 make_unique_with_forward 和 make_unique_with_forward2 都可以实现完美转发.
也就是使用 forward 和 forward2 都可以实现完美转发
Q: 那么为什么标准库要使用 std:remove_reference_t<T>& 和 std::remove_reference_t<T>&& 作为参数的版本 ，而不是直接使用 T& 和 && ?
A: 根据前面的解释，我们知道，在使用 std::forward 时, 我们一定要指定模板参数(T = Arg)才能获取表达式 arg 的原有类型,
  也就是形如 std::forwards<Arg>(arg) 的调用, 而不是 std::forward(arg).
  于是,我们需要一个强制用户指定模板参数的实现方法.
  对于参数类型为 T& 和 T&& 的版本(也就是函数 forward2), 当调用 std::forward(arg) 时, 编译器是可以推导类型 T (当然，这里会将 arg 推导成一个
  左值), 所以如果用户忘记指定模板参数，代码依然可以通过，只是不符合我们完美转发的预期. 因此，我们需要使用 std::remove_reference_t<T>,
  这样用户就必须指定 forward 的模板参数

Q: 为什么使用 std::remove_reference_t 后，用户就必须指定模板参数
A: 关键在于"可推导的上下文"和"不可推导的上下文", 对于模板参数来说，出现作用域符 :: 就是不可推导的上下文，因为我们没法反向推导, 所以只能指定模板参数类型
参考:
  1. https://stackoverflow.com/questions/70159423/forward-with-remove-reference-in-template-function-parameter-type
  2. https://stackoverflow.com/questions/25245453/what-is-a-non-deduced-context
*/

struct H {
  using type = double;
};  // struct H
struct I {
  using type = double;
};  // struct I
struct J {
  using type = int;
};  // struct J
template <typename T>
void f(typename T::type t) {
}
void run_f() {
  // 此时需要根据 1.0 这个 double 反向推导 T, 但此时有两种符合要求的类型，一个是 H, 一个是 I
  // 所以这是一个不可推导的上下文，必须显示指定模板参数类型
  // f(1.0);
  f<H>(1.0);
  forward<int>(1);
  // forward(1);  // 必须显示指定类型
  forward2<int>(1);
  forward2(1);  // 可以不用显示指定类型，推导为 T = int
}

// std::move 的实现就是使用 return static_cast<U&&>(t), 其中 U 是 remove_reference_t<T>, 这样，不管 T 是什么类型
// U&& 都是右值类型，最终返回的一个右值类型
template <class T>
inline constexpr std::remove_reference_t<T>&& move(T&& __t) noexcept {
  typedef std::remove_reference_t<T> U;
  return static_cast<U&&>(__t);
}


// 1.4.1 auto 类型推导
// auto 会丢失引用属性，如果表达式为引用类型，那么也会丢失 cv 属性
// 因为在 auto 语义下，其表现为值语义，即通过移动、拷贝构造,自然失去了 cv 属性
struct BB {};
void run_auto() {
  PRINT_CURRENT_FUNCTION_NAME;
  BB b1;
  BB& b2 = b1;
  const BB& b3 = b1;
  auto a1 = b1;
  SAME_TYPE(BB, decltype(a1));
  auto a2 = b2;
  SAME_TYPE(BB, decltype(a2));
  auto a3 = b3;
  SAME_TYPE(BB, decltype(a3));

  // 显示指定 auto& 来保留引用与 cv 属性
  auto& a4 = b1;
  SAME_TYPE(BB&, decltype(a4));
  auto& a5 = b2;
  SAME_TYPE(BB&, decltype(a5));
  auto& a6 = b3;
  SAME_TYPE(const BB&, decltype(a6));
  const auto& a7 = b2;
  SAME_TYPE(const BB&, decltype(a7));


  // 转发引用
  auto&& a8 = b1;  // 左值引用
  SAME_TYPE(BB&, decltype(a8));
  auto&& a9 = BB{};  // 右值引用
  SAME_TYPE(BB&&, decltype(a9));
  std::println();
}

// 1.4.2 decltype 类型推导
/*
使用 decltype 获取表达式的类型
1. 带括号，获取表达式值的类型: 整体作为表达式使用时的值类别
    a. 值为将亡值(xvalue) -> T&&
    b. 值为左值(lvalue)   -> T&
    c. 值为纯右值(plvalue) -> T
2. 不带括号，获取标识符的类型: 标识符被定义时的类型
*/
struct Point {
  int x = 0;
  int y = 0;
};  // struct Point

void run_decltype() {
  PRINT_CURRENT_FUNCTION_NAME;
  Point pt;
  Point* p_pt = &pt;
  const Point* cp_pt = &pt;
  Point& lr_pt = pt;
  Point&& rr_pt{};
  { // 带括号版本 : expression
    { //左值 -> T&
      SAME_TYPE(decltype((pt)), Point&);
      SAME_TYPE(decltype((p_pt)), Point*&);
      SAME_TYPE(decltype((cp_pt)), const Point*&);
      SAME_TYPE(decltype((lr_pt)), Point&);  // 不存在引用的引用
      SAME_TYPE(decltype((rr_pt)), Point&);  // 虽然 rr_ptr 是一个右值的引用，但这个变量作为一个表达式一个左值(它是有地址的, &rr_pt)
      SAME_TYPE(decltype((rr_pt.x)), int&);
      SAME_TYPE(decltype((pt.x)), int&);
      // 前置++(pre increment) 返回的是 pt.x 的引用
      // T& T::operator++();
      SAME_TYPE(decltype((++pt.x)), int&);
    }
    {  // 纯右值 -> T
      // 后置++(pos increment) 返回的是一个纯右值
      // T T::operator++(int);
      SAME_TYPE(decltype((pt.x++)), int);
      SAME_TYPE(decltype((Point(1, 2))), Point);
      SAME_TYPE(decltype((5)), int);
    }
    {  // 将亡值 -> T&&
      SAME_TYPE(decltype((Point(10, 10).x)), int&&);
      SAME_TYPE(decltype((std::move(pt))), Point&&);
      SAME_TYPE(decltype((static_cast<Point&&>(pt))), Point&&);
    }
  }
  {  // 不带括号版本 : identifier
    SAME_TYPE(decltype(pt), Point);
    SAME_TYPE(decltype(p_pt), Point*);
    SAME_TYPE(decltype(cp_pt), const Point*);
    SAME_TYPE(decltype(lr_pt), Point&);
    SAME_TYPE(decltype(rr_pt), Point&&);
    SAME_TYPE(decltype(rr_pt.x), int);
    SAME_TYPE(decltype(Point{10, 10}.x), int);
  }
  std::println();
}

// 1.4.3 decltype(auto) 类型推导 : cpp14
/*
1. auto 表现为值语义，丢失引用性和cv属性, 使用const 和 & 都要手动指定
2. 使用 auto& 和 auto&& 则只能表现引用语义
于是，需要使用 decltype(expression) res = expression; 来获取右侧表达式的类型
为了简化代码，则需要使用 decltype(auto) res = expression; 也就是使用 auto 来代表右侧的表达式
*/
void run_decltype_auto() {
  PRINT_CURRENT_FUNCTION_NAME;
  Point pt{1, 2};
  {  // decltype(expression) res = expression;
    decltype(pt) v1 = pt;
    SAME_TYPE(decltype(v1), Point);
    decltype((pt)) v2 = pt;
    SAME_TYPE(decltype(v2), Point&);
    decltype(1+2+3+4) v3 = 1 + 2 + 3 + 4;
    SAME_TYPE(decltype(v3), int);
  }
  {  // decltype(auto) res = expression;
    decltype(auto) v1 = pt;
    SAME_TYPE(decltype(v1), Point);
    // 在等号右边使用() 来区分decltype的两种能力
    decltype(auto) v2 = (pt);  // (pt) 相当于调用 Point& get_reference() { return pt; }
    SAME_TYPE(decltype(v2), Point&);
    decltype(1+2+3+4) v3 = 1 + 2 + 3 + 4;
  }
  std::println();
}


// decltype(auto) 推导函数返回的表达式类型
std::string lookup1() { return ""; }
std::string& lookup2() { static std::string s(""); return s; }

std::string lookup_A_1() { return lookup1(); }
std::string& lookup_A_2() { return lookup2(); }
SAME_TYPE(decltype(lookup_A_1()), std::string);
SAME_TYPE(decltype(lookup_A_2()), std::string&);

decltype(auto) lookup_B_1() { return lookup1(); }
decltype(auto) lookup_B_2() { return lookup2(); }
SAME_TYPE(decltype(lookup_B_1()), std::string);
SAME_TYPE(decltype(lookup_B_2()), std::string&);

decltype(auto) lookup_C_1() {
  auto s = lookup1();
  return s;
}
decltype(auto) lookup_C_2() {
  auto s = lookup1();
  SAME_TYPE(decltype((s)), std::string&);
  return (s);  // 返回一个 std::string
}
decltype(auto) lookup_C_3() {
  static auto s = lookup1();
  SAME_TYPE(decltype((s)), std::string&);
  return (s);  // 返回一个 std::string&
}
SAME_TYPE(decltype(lookup_C_1()), std::string);
// 书上写的这里是 string&, 根据语法 (s) 确实也是 string&
// 但是 s 是一个函数内的局部变量，如果返回 std::string& 会造成悬垂引用
// 个人理解，编译器将这个类型改成了右值引用，也就是 std::string&&
// 这种函数写法并不安全,不过多纠结
SAME_TYPE(decltype(lookup_C_2()), std::string&&);
SAME_TYPE(decltype(lookup_C_3()), std::string&);

// 1.4.4 std::declval 元函数
template <typename F, typename... Args>
// using InvokeResultOfFunc = decltype(F{}(Args{}...));
using InvokeResultOfFunc = decltype(std::declval<F>()(std::declval<Args>()...));

struct AFunctionObj {
  double operator()(char, int);
  float operator()(int);
};  // struct AFunctionObj
SAME_TYPE((InvokeResultOfFunc<AFunctionObj, char, int>), double);
SAME_TYPE((InvokeResultOfFunc<AFunctionObj, int>), float);

// std::declval 返回一个转发引用，即 T&&
// 因为引用类型可以是非完备类型，其上下文不用求值，也就不需要完整定义
// 源码实现如下
template <typename T>
struct declval_protector {
  static constexpr bool value = false;
};  // struct declval_protector
template <typename T>
typename std::add_rvalue_reference_t<T> declval() noexcept {
  // declval 只能在 decltype/sizeof 等非求值上下文中使用, 也就是说 declval() 这个函数其实只需要声明即可

  // 防呆设计:
  // 在非求值上下文中不会调用这个函数，也就不会实例化 declval_protector<T>, 这样 static_assert 就不会执行
  // 如果在求值的上下文中调用这个函数，那么就会 assert false

  // declval_protector 是模板的原因就是延迟实例化，如果直接设计成 constexpr bool dp_value = false; 就会直接
  // static_assert(false)
  static_assert(declval_protector<T>::value, "防呆设计, 请在 decltype/sizeof 等非求值上下文中使用");
  // 没有返回值，因为不需要在求值上下文中使用
}
template <typename T>
T&& declval_2() noexcept;  // 没有给加防呆设计, 在求值上下文中使用会链接错误，但是报错信息没有防呆设计友好

/*
根据 https://en.cppreference.com/w/cpp/types/add_reference 可以了解到
T&& 和 std::add_rvalue_reference_t<T> 的区别在于对不可引用类型的处理
不可引用类型U + T&&  ->  编译错误
不可引用类型U + std::add_rvalue_reference<T>   -->  U
常见的就是 void 类型
*/

void run_declval() {
  PRINT_CURRENT_FUNCTION_NAME;
  int i{0};
  {
    SAME_TYPE(decltype(std::declval<int>()), int&&);
    SAME_TYPE(decltype(std::declval<decltype(i)>()), int&&);

    SAME_TYPE(decltype(declval<int>()), int&&);
    SAME_TYPE(decltype(declval<decltype(i)>()), int&&);

    SAME_TYPE(decltype(declval_2<int>()), int&&);
    SAME_TYPE(decltype(declval_2<decltype(i)>()), int&&);
  }
  int& j = i;
  {
    SAME_TYPE(decltype(std::declval<int&>()), int&);
    SAME_TYPE(decltype(std::declval<decltype(j)>()), int&);

    SAME_TYPE(decltype(declval<int&>()), int&);
    SAME_TYPE(decltype(declval<decltype(j)>()), int&);

    SAME_TYPE(decltype(declval_2<int&>()), int&);
    SAME_TYPE(decltype(declval_2<decltype(j)>()), int&);
  }
  int&& k = 1;
  {
    // k 的类型(type) 是 int&&
    // k 的值类别(value category) 是左值, 表达式 (左值) 得到左值引用
    SAME_TYPE(decltype(k), int&&);  // 使用类型
    SAME_TYPE(decltype((k)), int&);  // 使用值类别

    SAME_TYPE(decltype(std::declval<int&&>()), int&&);
    SAME_TYPE(decltype(std::declval<decltype(k)>()), int&&);

    SAME_TYPE(decltype(declval<int&&>()), int&&);
    SAME_TYPE(decltype(declval<decltype(k)>()), int&&);

    SAME_TYPE(decltype(declval_2<int&&>()), int&&);
    SAME_TYPE(decltype(declval_2<decltype(k)>()), int&&);
  }
  {  // std::add_rvalue_reference_t<T> 和 T&& 的区别
    SAME_TYPE(decltype(declval<void>()), void);

    // 编译错误
    // SAME_TYPE(decltype(declval_2<void>()), void);
  }
  {  // 防呆设计
    // declval<int>();  // int&&(), 编译的时候就会 static_assert(false)
    // declval_2<int>();  // int&&(), 链接错误, 因为没有 declval_2 的定义, 只是报错信息没有 declval 防呆设计的友好
  }

  std::println();
}

// 1.4.5 类模板参数推导(CTAD) Class Template Argument Deduction
// cpp17
int fffoo = std::max(1, 2);  // 当两个数值类型一致时，编译器会帮我们推导模板参数
double bar = std::max<double>(1, 2.0);  // 当类型不一致时，去要显示指定模板参数
void run_ctad() {
  // 然而，cpp17 以前,模板类必须指定模板参数
  std::vector foofo{1, 2, 3, 4};  // 这个代码在 cpp14 无法编译, missing template arguments before 'v'
  SAME_TYPE(std::vector<int>, decltype(foofo));
  std::vector<int> fofo{1, 2, 3, 4};

  std::pair<int, double> bar{1, 2.0};
  // 在cpp17 以前，为了避免在模板类中显示指定模板参数，可以使用辅助函数
  auto bar_2 = std::make_pair(1, 2.0);   // 辅助函数可以不用指定模板参数，由编译器推导
}
template <typename T, typename U>
struct Pair {
  Pair();
  Pair(T, U);
};  // struct Pair
// 编译器会构造函数推导 Pair 中 T 和 U 的类型
SAME_TYPE(decltype(Pair{1, 2}), (Pair<int, int>));
// 此时会出现编译错误，因为调用 Pair 的无参构造函数无法推导出 T 和 U
// SAME_TYPE(decltype(Pair{}), (Pair<???>));

struct Pair2 {
  template <typename A, typename B>
  Pair2(A&&, B&&);
};  // struct Pair2
// 当构造函数与模板参数无法对应时，需要使用推导规则, 编译器会优先考虑推导规则，之后才通过构造函数来推导
// 此时无法通过 A = int, B = int 推导出 T 和 U 的类型
// SAME_TYPE(decltype(Pair{1, 2}), (Pair<int&&, int&&>));
// 推导规则如下
template <typename A, typename B>
Pair(A, B) -> Pair<A, B>;  // 通过构造函数的模板实参 A, B 推导出模板类的模板实参 T = A, U = B

// 1.5 函数对象
// 一个对象能够像函数一样调用
/*
函数指针和函数对象相比，会产生间接调用的开销，不利于编译器优化
1. 函数指针是一个变量，在运行时生成并调用，调用函数指针指向的函数时存在地址表跳转(虽然函数指针指向的地址也是确定的，但是需要编译器优化出来，
比如将函数指针层层传递给不同的变量，那么这些变量的值是在运行时确定的)
2. 函数对象在编译时就确定了类型，函数对象的调用地址也就确定了(函数对象类中的 operator() 的地址)
*/

// 1.5.1 定义函数对象
// 命令模式: 将行为包裹与函数对象中并传递给用户供后续调用
template <typename T>
struct Plus {
  constexpr T operator() (T x, T y) { return x + y; }
};  // struct Plus
struct PlusN {
  PlusN(int N): N(N) {}
  int operator()(int x) { return x + N; }
private:
  int N;
};  // struct PlusN

constexpr auto a = Plus<int>{}(1, 2);
auto plus5 = PlusN(5);
void run_function_object() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::println("1 + 5 = {}", plus5(1));
  auto plus5_with_bind = std::bind(Plus<int>{}, 5, std::placeholders::_1);
  std::println("1 + 5 = {}", plus5_with_bind(1));

  std::vector<int> nums{5, 3, 2, 5, 6, 1, 7, 4};
  std::copy_if(nums.begin(), nums.end(), std::ostream_iterator<int>(std::cout, ", "),
      std::bind(std::greater<int>{}, std::placeholders::_1, 4));
  std::println();
}

// 1.5.2 lambda 表达式
constexpr auto add_lambda = [](auto a, auto b) { return a + b; };
// cpp17 起 lambda 默认为 constexpr
// 编译器为 lambda 生成了一个匿名类型(可以使用 cppinsights.io 查看编译器生成的匿名类)
struct add_lambda_ {
  template <typename T0, typename T1>
  constexpr auto operator()(T0 a, T1 b) const { return a + b; }
};  // struct add_lambda_
constexpr const add_lambda_ add_lambda2 = add_lambda_{};
// 如果 lambda 存在捕获列表，那么会在匿名类中生成成员变量与构造函数来存储捕获
// 没有捕获列表的 lambda 可以赋值给函数指针
using IntAddFunc = int(*)(int, int);
constexpr IntAddFunc iadd = [](auto a, auto b) { return a + b; };

// 模板函数要求两个形参同类型，而 lambda 将两个形参声明为 auto, 不要去其类型一致
template <typename T> T add(T a, T b) { return a + b; }
// cpp20 中也支持泛型 lambda
constexpr auto add2 = []<typename T>(T a, T b) { return a + b; };

/* 延迟实例化
1. 模板函数要求先实例化，然后才可以传递
2. 泛型 lambda 是对象，可以先按值传递，然后在调用时实例化
比如说传递调用模板函数 invoke(add<int>), 此时已经将 add<int> 实例化, 此时传递的是函数指针
而 invoke(add2) 就不会实例化 add2, 因为 add2 是按值传递的，传递的是对象，add2 的类型 T 是什么，在调用
add2 时实例化的时候才知道
*/

// lambda 替代 std::bind, lambda 可以捕获引用，std::bind 只能捕获值，如果要捕获引用，需要使用 std::ref, 而且类型不直观
// lambda 是语言特性，std::bind 是标准库，编译器优化前者更简单
void run_lambda() {
  PRINT_CURRENT_FUNCTION_NAME;
  // 'add2' cannot be captured because it does not have automatic storage duration
  // 只能捕获局部变量或函数参数
  // constexpr auto plus5_with_lambda1 = [add2](auto x) { return add2(x, 5); };

  // 无需捕获
  constexpr auto plus5_with_lambda2 = [](auto x) { return add2(x, 5); };

  constexpr auto add3 = []<typename T>(T a, T b) { return a + b; };
  constexpr auto plus5_with_lambda3 = [add3](auto x) { return add2(x, 5); };
  std::println();
}

// 1.5.3 函数适配器 std::function
// 适配器可以承载同种类型的可调用对象
enum class Op { ADD, MUL };
std::function<int(int, int)> OperationFactory(Op op) {
  switch (op) {
    case Op::ADD : return [](int a, int b) { return a + b; };
    case Op::MUL : return std::multiplies<int>{};
  }
}
// 上面的函数适配器可以动态绑定两种不同的函数对象

int main() {
  run_span();
  run_not_null();
  std::vector<int> v;

  auto v2 =  std::move(v);
  run_make_unique();
  run_make_unique_with_get();
  run_auto();
  run_decltype();
  run_decltype_auto();
  run_declval();
  run_ctad();
  run_function_object();
  run_lambda();
  return 0;
}
