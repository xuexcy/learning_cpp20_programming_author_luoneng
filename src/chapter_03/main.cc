/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2025/06/01 15:39:59
# Desc   : 第 3 章 概念约束
########################################################################
*/

#include <algorithm>
#include <concepts>
#include <print>
#include <type_traits>
#include <vector>

#include "cpp_utils/util.h"

// 长期以来，模板参数没有任何约束，仅在实例化时才能发现类型上的错误

// 3.1 定义概念
// 编译期对类型约束的谓词

/*
1. 定义概念时的约束表达式为 bool 类型
2. 逻辑操作符: 合取 conjunction (c++ 中的 &&), 析取 disjunction (c++中的 ||), 是一个短路操作符
*/
template <typename T>
concept C = std::is_integral_v<typename T::type> || (sizeof(T) > 1);
static_assert(C<double>);

// 对于可变参数模板形成的约束，不是合取也不是析取, 整个约束为一个表达式，首先要检查表达式是否合法，因此没有短路操作
// 只要有一个 Ts 没有 type 类型成员，则表达式不合法, 也就是括号内不是一个合法表达式
// 当表达式合法后才会对约束进行合取、析取计算
// C2 表达式的意思是: 所有 Ts 都有成员类型 type, 且至少有一个是整型
template <typename... Ts>
concept C2 = (std::is_integral_v<typename Ts::type> || ...);

// 如果想要表达至少有一个 Ts 存在类型成员 type 且为整型, 则需要将 Ts 的约束单独定义为一个 concept
template <typename T>
concept integral_with_nest_type = std::is_integral_v<typename T::type>;
// integral_with_nest_type<Ts> 是一个合法表达式，所以括号内是一个合法表达式
// 然后编译器才会对表达式进行合取、析取操作
template <typename... Ts>
concept at_least_one_with_integral_nest_type = (integral_with_nest_type<Ts> || ...);

// 1. 存在一个模板参数拥有类型成员type且为整型
template <typename T, typename U>
concept A1 = std::is_integral_v<typename T::type> || std::is_integral_v<typename U::type>;
// 使用 bool() 来代表一个表达式
// 2. 要求两个模板参数都有类型成员type且至少有一个是整型
template <typename T, typename U>
concept A2 = bool(std::is_integral_v<typename T::type> || std::is_integral_v<typename U::type>);

// 要求类型 T 存在成员类型 type, 且为整型
template <typename T> concept B1 = std::is_integral_v<typename T::type>;
// 这不是 B1 的否定类型，它要求类型 T 存在成员类型 type, 且不为整型
// 也就是要先检测表达式 std::is_integral_v<typename T::type> 的合法性(T有成员类型type才合法)
template <typename T> concept B2 = !std::is_integral_v<typename T::type>;
static_assert(!B1<int>);  // int 没有成员类型 type, B1<int> 为 false
static_assert(!B2<int>);  // int 没有成员类型 type, B2<int> 为 false
struct Foo { using type = float; };  // struct Foo
static_assert(!B1<Foo>);  // Foo 有成员类型但不是整数, B1<Foo> 为 false
static_assert(B2<Foo>);
// 表达 B1 的否定
template <typename T> concept B3 = !B1<T>;  // B1<T> 是合法表达式
static_assert(B3<int>);  // int 没有成员类型 type, B1<T> 为 false, B3<T> 为 true

// 3.2 requires 表达式
/*
concept 用来定义约束(相当于一个constexpr bool名字)
requires 用来表达约束
*/
// 3.2.1 简单要求
template <typename M>
concept Machine = requires(M m) {
  m.power_up();  // M 拥有成员
  m.power_down();
};
template <typename T>
concept Animal = requires(T animal) {
  play(animal);  // 存在函数 play 接受 T 的实例
  T::count;  // T 存在静态成员 count
  animal.age;  // T 存在成员变量 俺哥
};
struct AnAnimal {
  static int count;
  int age;
};  // struct AnAnimal
void play(AnAnimal);
static_assert(Animal<AnAnimal>);

template <typename T>
concept Number = requires(T a, T b, T c) {
  a == a;  // 可判等
  a + b * c;  // 可进行 + * 操作
};

// 3.2.2 类型要求
template <typename T>
concept C3 = requires {
  typename T::type;
  typename std::vector<T>;
};

// 3.2.3 复合要求
// { 表达式 } [noexcept] [返回类型]
// {} 里面的表达式没有分号，有分号就叫语句了
template <typename T>
concept Movable = requires(T a, T b) {
  { a = std::move(b) } noexcept;  // 移动时禁止抛出异常
};
/*
当对表达式类型提出要求时，有两种
1. 确定类型 std::same_as
2. c++ 运行类型转换，对表达式要求可以稍微放宽，只要求能隐式转换到要求的类型即可: std::convertible_to
注意: std::is_same 和 std::is_convertible_to 是 struct, 上面这俩是 concept
*/
int f(int);
int g(int);
Foo f(Foo);
Foo g(Foo);
template <typename T>
concept C4 = requires(T x) {
  // std::same_as 接受两个参数
  // -> 语法会将表达式的类型补充到 std::same_as 的第一个参数
  { f(x) } -> std::same_as<T>;
  { g(x) } -> std::convertible_to<double>;

  // -> 等价于如下两句，其中第 1 句 f(x) 用于检查表达式的合法性，第 2 句用于检查类型约束是否符合要求
  // 使用两句时，编译器能给出更精确的错误信息: 是表达式不合法还是类型不符合要求
  f(x);
  // 注意 f(x) 外面有一个 (), 用于获取表达式的类型，参见 chapter_01 1.4.2 decltype 类型推导
  requires std::same_as<decltype((f(x))), T>;
  g(x);
  requires std::convertible_to<decltype((g(x))), double>;
};
static_assert(C4<int>);
static_assert(!C4<Foo>);  // g(Foo) 的返回类型是 Foo, 不能转换为 double

// 3.2.4 嵌套要求
// requires 表达式体通常只检查表达式的合法性
template <typename T>
concept C5 = requires {
  sizeof(T) > sizeof(void*);  // 外层的 requires 只检查这句表达式的合法性，不检查其计算结果是否为 true
};
template <typename T>
concept C6 = requires {
  requires sizeof(T) > sizeof(void*);  // 通过添加 requires 来要求表达结果为真
};
static_assert(C5<int>);  // 只要求 sizeof(int) > sizeof(void*) 这个表达式合法
static_assert(!C6<int>);

// 3.2.5 注意事项
// requires 表达式是编译时谓词，不一定要和 concept 一起出现
template <typename T>
constexpr bool has_member_swap = requires(T a, T b) {
  a.swap(b);
};
// requires 表达式服务于模板参数约束，不能作用域具体类型
// 下面不是得到 false，而是出现编译错误
// constexpr bool has_int_member_swap = requires(int a, int b) {
//   a.swap(b);
// }

// 约束非类型参数
template <size_t N>
concept Even = requires {
  requires (N % 2 == 0);
};

// 出现在 if constexpr 中
template <typename T>
void clever_swap(T& a, T& b) {
  if constexpr (requires(T a, T b) { a.swap(b); }) {
    a.swap(b);
  } else {
    using std::swap;
    swap(a, b);
  }
}

/*
requires {
  布尔表达式;  // 只检查表达式合法性
  requires 布尔表达式;  // 在合法性基础上求值
  // 两个都写的话，编译器报错时就会更容易看出是合法性问题还是求值问题
}
*/
template <typename T>
concept always_true = requires {  // 永远满足，因为布尔表达式永远合法
  sizeof(T) <= sizeof(int);
};
template <typename T>
concept size_lte_int = requires {
  sizeof(T) <= sizeof(int);
};

// 3.3 requires 子句
template <typename T>
requires std::is_integral_v<T>
T gcd(T a, T b);  // greatest common division

// concept 优先级高于普通函数
// 使用 enable_if 需要条件互斥; 使用 decltype 取决于重载决议
template <typename T>
requires std::is_trivial_v<T>
void f(T) { std::println("1"); }
template <typename T>
void f(T) { std::println("2"); }

template <typename T>
std::enable_if_t<std::is_trivial_v<T>> f(T) { std::println("1"); }
template <typename T>
std::enable_if_t<!std::is_trivial_v<T>> f(T) { std::println("1"); }

template <typename T>
auto init(T& obj) -> decltype(obj.OnInit()) {
  std::println("1");
  return obj.OnInit();
}
// 重载决议，不定参数的优先级较低
void init(...) { std::println("2"); }

// 相比于 decltype, requires 更合理
template <typename T>
requires requires(T obj) { obj.OnInit(); }
void f(T& obj) {
  std::println("1");
  return obj.OnInit();
}
template <typename T>
void f(T&) { std::println("2"); }

// 模板参数前使用
template <std::integral T, std::integral U>
void f(T, U);
// 使用 requires
template <typename T, typename U>
requires (std::integral<T> && std::integral<U>)
void f(T, U);
// 函数参数前使用
void f(std::integral auto a, std::integral auto b);
// lambda 参数使用
auto ff = [](std::integral auto lhs, std::integral auto rhs) { return lhs + rhs; };

// 添加到模板类与其特化版本
template <typename T>
class Optional {
  union { T val; char dummy; } storage_;
  bool init{};
};  // class Optional
template <typename T>
requires std::is_trivial_v<T>
class Optional<T> {
  T storage_;
  bool init{};
};  // class Optional<T>

template <typename T>
struct Wrapper {
  T value_;
  // 放到函数后面
  void operator()() requires std::is_invocable_v<T> {
    value_();
  }
  void reset(T v) { value_ = v; }
};  // struct Wrapper
// 显示实例化, 由于 int 不满足约束，不生成 operator()()
template struct Wrapper<int>;

// 3.4 约束的偏序规则
// 3.4.1 约束表达式归一化
// 3.4.2 简单的约束包含关系
// 3.4.3 一般约束的包含关系
template <typename T>
concept Scalar = std::is_arithmetic_v<T>;
template <typename T>
struct MathematicalTraits {
  constexpr static bool customized = false;
};  // struct MathematicalTraits
struct BigInt {
};  // struct BigInt
template <>
struct MathematicalTraits<BigInt> {
  constexpr static bool customized = true;
};  // struct MathematicalTraits<BigInt>
template <typename T>
concept CustomMath = MathematicalTraits<T>::customized;
template <typename T>
concept Mathematical = Scalar<T> || CustomMath<T>;

template <Mathematical T, Mathematical U>
void calculate(const T&, const U&) {
  std::println("Q");
}
// 重载
template <typename T, typename U>
requires (Scalar<T> && Scalar<U>) || (CustomMath<T> && CustomMath<U>)
void calculate(const T&, const U&) {
  std::println("P");
}

// 3.4.4 using 类型别名与 concept 表达式别名

// 3.5 概念标准库 <concepts>
// 3.5.1 same_as (与某类相同)
template <typename T, typename U>
concept same_as_impl = std::is_same_v<T, U>;
template <typename T, typename U>
concept same_as = same_as_impl<T, U> && same_as_impl<U, T>;

// 3.5.2 derived_from (派生自某类)
// 忽略cv修饰符也满足派生关系，所以在 convertible 中加上 cv 修饰符
template <typename Derived, typename Base>
concept derived_from = std::is_base_of_v<Base, Derived> &&
    std::is_convertible_v<const volatile Derived*, const volatile Base*>;

// 3.5.3 convertible_to (可转换为某类)
// 可隐式或显示转换成目标类型
template <typename From, typename To>
concept convertible_to = std::is_convertible_v<From, To> &&  // 隐式转换
    requires (std::add_rvalue_reference_t<From>(&f)()) {
      static_cast<To>(f());  // 显示转换
    };

// 3.5.4 算术概念
// 3.5.5 值概念
// 3.5.6 invocable (可调用的)
template <typename F, typename... Args>
concept invocable = requires (F&& f, Args&&... args) {
  std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
};
// 谓词，返回类型为 bool
template <typename F, typename... Args>
concept predicate = std::regular_invocable<F, Args...> &&
    std::is_same_v<bool, std::invoke_result_t<F, Args...>>;

// 只接受两个参数的谓词，叫做关系 relation
template <typename R, typename T, typename U>
concept relation = predicate<R, T, T> && predicate<R, U, U> &&
    predicate<R, T, U> && predicate<R, U, T>;

// 3.6 综合运用之扩展 transform 变换算法
template <std::input_iterator... InputIt, std::invocable<std::iter_reference_t<InputIt>...> Op,
    std::output_iterator<std::invoke_result_t<Op, std::iter_reference_t<InputIt>...>> OutputIt>
OutputIt zip_transform(OutputIt out, Op op, std::pair<InputIt, InputIt>... inputs) {
  while (((inputs.first != inputs.second) && ...)) {
    // *out++ = op(*inputs.first++...);
    *out = op(*inputs.first ...);
    ++out;
    ((++inputs.first), ...);
  }
  return out;
}
void run_transform() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::string s("hello");
  std::transform(s.begin(), s.end(), s.begin(), [](auto ch) {
    return std::toupper(ch);
  });

  std::vector<int> v1{1, 2, 3, 4};
  std::vector<int> v2{1, 2, 3, 4};
  std::vector<int> v3{1, 2, 3};
  std::vector<int> result(3);
  zip_transform(result.begin(), [](int a, int b, int c) { return a + b + c; },
      std::make_pair(v1.begin(), v1.end()),
      std::make_pair(v2.begin(), v2.end()),
      std::make_pair(v3.begin(), v3.end())
  );
  std::println("{}", result);
  std::println();
}

// 3.7 注意事项
int main() {
  run_transform();
  return 0;
}
