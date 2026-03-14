/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2026/01/06 15:55:41
# Desc   : 第 6 章 constexpr 元编程
########################################################################
*/

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <mutex>
#include <numbers>
#include <print>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "cpp_utils/util.h"

// 6.1 constexpr 变量
constexpr double PI = 3.1415;
constexpr double AREA = PI * 2.0 * 2.0;
constexpr size_t MAX_LEN = 32;
int8_t buffer[MAX_LEN];

template <typename T>
inline constexpr bool is_integral_v = std::is_integral<T>::value;
// inline 修饰表明 is_integral_v 是个内联变量，这是C++17的小特性, 可以避免重复定义的问题
static_assert(is_integral_v<int>);
static_assert(!is_integral_v<void>);

template <char c>
constexpr bool is_digit = (c >= '0' && c <= '9');
static_assert(!is_digit<'x'>);
static_assert(is_digit<'0'>);

template <size_t N>
constexpr size_t fibonacci = fibonacci<N - 1> + fibonacci<N-2>;
// 变量模板特化
template <>
constexpr size_t fibonacci<0> = 0;
template <>
constexpr size_t fibonacci<1> = 1;
static_assert(fibonacci<10> == 55);

// 6.2 constinit 初始化
// c++20 中的 constinit 定义的变量和constexpr一样要求能在编译时进行求值
// 但是保留了可变属性

// c++各个编译单元的全局变量在运行时的初始化顺序是不确定的，constinit要求其在编译时就进行初始化
// 节省运行时初始化开销，同时避免依赖问题
constinit std::mutex g_mtx;

// 6.3 折叠表达式
template <size_t ...Is>
constexpr int r_sum = (Is + ... + 0);  // 右折叠
template <size_t ...Is>
constexpr int l_sum = (0 + ... + Is);  // 左折叠
// (1 + (2 + (3 + 0)))
static_assert(6 == r_sum<1, 2, 3>);
// (((0 + 1) + 2) + 3)
static_assert(6 == l_sum<1, 2, 3>);

template <size_t ...Is>
constexpr int r_sub = (Is - ... - 0);
template <size_t ...Is>
constexpr int l_sub = (0 - ... - Is);

// (1 - (2 - (3 - 0)))
static_assert(2 == r_sub<1, 2, 3>);
// (((0 - 1) - 2) - 3)
static_assert(-6 == l_sub<1, 2, 3>);

// 6.4 constexpr 函数
constexpr int min(std::initializer_list<int> xs) {
  int low = std::numeric_limits<int>::max();
  for (int x : xs) {
    if (x < low) { low = x; }
  }
  return low;
}
static_assert(min({1, 2, 3, 4}) == 1);

// 6.4.1 consteval
// constexpr 表达函数是否可能被编译时求值
// consteval 表达函数必须能够被编译时求值

consteval int min2(std::initializer_list<int> xs) {
  int low = std::numeric_limits<int>::max();
  for (int x : xs) {
    if (x < low) { low = x; }
  }
  return low;
}
static_assert(min2({1, 2, 3}) == 1 );
void run_consteval_function() {
  PRINT_CURRENT_FUNCTION_NAME;
  int v = 1;
  // constexpr 可能在编译时求值，也可能在运行时求值
  assert(1 == min({v, 2, 3}));
  // consteval 必须在编译时求值
  // assert(1 == min2({v, 2, 3}));
}

// 6.4.2 编译时内存分配
consteval std::vector<int> sieve_prime(int n) {
  std::vector<bool> marked(n+1, true);
  for (int p = 2; p * p <= n; ++p) {
    if (marked[p]) {
      for (int i = p * p; i <=n; i += p) {
        marked[i] = false;
      }
    }
  }
  std::vector<int> result;
  for (int p = 2; p <= n; ++p) {
    if (marked[p]) { result.push_back(p); }
  }
  return result;
}

consteval size_t prime_count(int n) {
  return sieve_prime(n).size();
}

// 编译时能够存储不定长数据到 result，但是result没办法在运行时进行使用
// 编译时分配的内存必须在编译时释放
// 所以不能将 sieve_prime(50) 的结果赋值给变量 primes50 并在运行使用
// constexpr auto primes50 = sieve_prime(50);
static_assert(25 == prime_count(100));

// 书上说这种方式也不行，但 cppreference 上写到 size() 函数是 constexpr
// 因为这里并没有在运行时使用 sieve_prime 的返回值，而是在运行时直接调用了同为 constexpr 的 size() 函数
static_assert(25 == sieve_prime(100).size());

// 编译时唯一能够分配并被运行时使用的静态容器就是数组，所以需要想办法把动态容器中的数据复制到数组中
template <int n>
consteval auto save_prime_to_array() {
  auto primes = sieve_prime(n);
  // primes 不是 constexpr，不能通过 primes.size() 在编译时获取数组长度
  // 只能通过 consteval 函数 prime_count 来获取数组长度
  std::array<int, prime_count(n)> result;
  std::copy(primes.begin(), primes.end(), result.data());
  return result;
}
constexpr auto primes100 = save_prime_to_array<100>();

void run_dynamic_container_to_static_container() {
  PRINT_CURRENT_FUNCTION_NAME;
  for (auto e : primes100) {
    std::println("{}", e);
  }
}

// constexpr 的编译速度远远快于传统模板元编程
// 书中提到, MSVC 使用 constexpr 与 GCC 使用模板元编程生成小于 10000 的素数表
// 编译前者10s左右，后者1min左右
// constexpr 在编译时被编译器当成函数直接计算，模板元编程需要生成大量类型


// 6.4.3 编译时虚函数
struct Shape {
  virtual ~Shape() = default;
  virtual double GetArea() const = 0;
};  // struct Shape
struct Circle : Shape {
  constexpr Circle(double r) : r_(r) {
  }
  constexpr double GetArea() const override {
    return std::numbers::pi * r_ * r_;
  }
private:
  double r_;
};  // struct Circle : Shape
struct Rectangle : Shape {
  constexpr Rectangle(double width, double length) : w_(width), l_(length) {
  }
  constexpr double GetArea() const override {
    return w_ * l_;
  }
private:
  double w_;
  double l_;
};  // struct Rectangle : Shape

consteval std::tuple<double, double, double> test_subtype() {
  std::array<Shape*, 4> shapes {
    new Circle(10), new Rectangle(3, 5),
    new Circle(5), new Rectangle(2, 3)
  };
  double sum = 0.0;
  double sum_rectangle = 0.0;
  // 2026-01-08: g++ 15.2.0 还不支持编译期 dynamic_cast
  // double sum_rectangle2 = dynamic_cast<Rectangle*>(shapes[1])->GetArea() + dynamic_cast<Rectangle*>(shapes[3])->GetArea();

  double sum_rectangle2 = static_cast<Rectangle*>(shapes[1])->GetArea() + static_cast<Rectangle*>(shapes[3])->GetArea();
  for (auto s : shapes) {
    sum += s->GetArea();
    if (auto rectangle = dynamic_cast<Rectangle*>(s)) {
      sum_rectangle += s->GetArea();
    }
    delete s;
  }
  return {sum, sum_rectangle, sum_rectangle2};
}

constexpr auto to_int = [] <typename Tuple>(Tuple&& t) {
  constexpr size_t len = std::tuple_size_v<std::remove_reference_t<Tuple>>;
  auto convert = []<size_t... Is>(Tuple&& t, std::index_sequence<Is...>) {
    return std::make_tuple(static_cast<int>(std::get<Is>(std::forward<Tuple>(t)))...);
  };
  return convert(std::forward<Tuple>(t), std::make_index_sequence<len>());
};
constexpr auto test_result = to_int(test_subtype());
static_assert(413 == std::get<0>(test_result));
// 编译器还不支持编译期 dynamic_cast, 这里 0 == std::get<1>(test_result);
// static_assert(21 == std::get<1>(test_result));
static_assert(21 == std::get<2>(test_result));

// 6.4.4 is_constant_evaluated
// c++20 <type_traits> 提供, 帮助判断表达式是否在编译期执行, 可以根据编译时和运行时的不同情况选择不同实现
constexpr double power(double b, int x) {
  if (std::is_constant_evaluated()) {
    return 1.0;
  } else {
    return 2.0;
  }
}

// 编译时，power函数进入 std::is_constant_evaluated() 为 true 的分支
constexpr double kilo = power(10.0, 3);
static_assert(1.0 == kilo);

int n = 3;
// 运行时，power函数进入 std::is_constant_evaluated() 为 false 的分支
double value = power(10.0, n);

// static_assert 总是要求表达式为编译期求值，所以此时 std::is_constant_evaluated() 是 true
static_assert(std::is_constant_evaluated());

int y = 0;
const int a = std::is_constant_evaluated() ? y : 1;
const int b = std::is_constant_evaluated() ? 2 : y;

constexpr int square(int x) {
  // 使用 constexpr 和 static_assert 类似，要求表达式在编译期求值，所以该函数总是走进 if 为 true 的分支
  // 所以注意 constexpr 不要和 std::is_constant_evaluated() 一起使用
  if constexpr (std::is_constant_evaluated()) {
    return 2;
  } else {
    return 4;
  }
}
constexpr int c = 0;
static_assert(2 == square(c));
const int d = 1;

void run_is_constant_evaluated() {
  PRINT_CURRENT_FUNCTION_NAME;
  assert(2.0 == value);
  assert(1 == a);  // a 在编译期求值时，对 y 求值失败，导致 std::is_constant_evaluated 为 false, 得到 a = 1
  assert(2 == b);  // b 在编译期求值时，对 2 求值成功，导致 std::is_constant_evaluated 为 true, 得到 b = 2

  assert(2 == square(d));  // 总是进入 square 中 constexpr 为 true 的分支
}

// 6.4.5 停机问题
/*
是否存在一个算法 f，能够判断程序是否会执行结束或无限循环
bool f(Function function);
  - true: function 会执行结束
  - false: function 会无限循环

假设这样的算法 f 存在，则定义程序 g
void g() {
  if (f(g)) {
    while(true);
  } else {
    return;
  }
}
如果算法 f 判断 g 会执行结束，即 f(g) == true, 则程序 g 会无限循环 while(true)
如果算法 f 判断 g 会无限循环, 即 f(g) == false, 则程序 g 会结束 return
所以算法 f 对 g 的判断和 g 的实际执行结果相矛盾，假设不成立
*/

// 根据停机问题，编译器无法知道对于给定的 constexpr 表达式和输入，其是否都能在编译期求值
// 编译器一般会限制计算深度，超过深度则放到运行时进行计算

// 考拉慈猜想 collatz conjecture: 对于正整数 n, 一直执行 n = f(n), 最终都能得到 n = 1
constexpr int f(size_t n) {
  return (n % 2 == 0) ? (n / 2) : (3 * n + 1);
}
constexpr size_t collatz_time(int n) {
  size_t step{0};
  for (; n > 1; ++step) {
    n = f(n);
  }
  return step;
}
// 对步骤进行求和
constexpr size_t all_collatz_time(int n) {
  size_t sum_step{0};
  for (size_t i = 1; i <= n; ++i) {
    sum_step += collatz_time(i);
  }
  return sum_step;
}
constexpr size_t sum_step1 = all_collatz_time(10000);

// 当 n 过大时，编译期求值就会超过限制，编译器会报错, 因为编译器并不知道这个算法是否最终会停止
// error: 'constexpr' evaluation operation count exceeds limit of 33554432 (use '-fconstexpr-ops-limit=' to increase the limit)
// constexpr size_t sum_step2 = all_collatz_time(10000000);

// 6.4.6 检测未定义行为

// 除0
// error: division by zero is not a constant expression
// constexpr auto v = 1/0;

// overflow
// error: overflow in constant expression
// constexpr int v1 = 2147483647 * 2;

// 对空指针解引用
// error: dereferencing a null pointer
constexpr int f() {
  int* p = nullptr;
  return *p;
}
// constexpr auto v2 = f();

// 内存越界访问
// error: array subscript value '12' is outside the bounds of array 'arr' of type 'int [10]'
constexpr int f(const int* p) {
  return *(p + 12);
}
constexpr int g() {
  int arr[10]{};
  return f(arr);
}
// constexpr int v3 = g();

// 悬挂引用
// error: cannot bind non-const lvalue reference of type 'int&' to an rvalue of type 'int'
// constexpr int& f2() {
//   int x = 23;
//   return x;
// }
// constexpr auto v4 = f2();

// vector resize 导致内存重新分配，之前的只分和迭代器将失效，对齐访问则产生未定义行为
// error: use of allocated storage after deallocation in a constant expression
constexpr int f3() {
  std::vector<int> v(700);
  int* q = &v[7];
  v.resize(900);
  return *q;
}
// constexpr int x = f3();

// 6.5 非类型模板参数
template <size_t N>
constexpr void f() {}

enum class Color {
};  // enum class Color
template <Color c>
struct C {
};  // struct C
// c++20 支持以浮点数作为非类型参数，使用 auto 占位符表达由编译器类型推导的非类型参数
struct Foo {
};  // struct Foo
template <auto...>
struct ValueList {
};  // struct ValueList
ValueList<'C', 0, 2L, nullptr, Foo{}> x;

template <const char* p>
struct CC {
};  // struc CC

/* "hello" 是 const char[6], 退化成 const char*
const char* 不是编译期常量，"hello" 在不同的编译单元中可能有不同的地址
你以为 a.h 中使用的 CC<"hello"> 和 b.h 中使用的 CC<"hello"> 是同一个类型
但其实两个 "hello" 的地址 const char* 可能是不同的值，也就是看起来是相同的非类型模板参数，但实际上可能不同
所以编译器不允许字符串字面量直接做非类型模板参数
*/

// Pointer to subobject of string literal is not allowed in a template argument
// clang(constexpr_invalid_template_arg)
// CC<"hello"> cc;

// 解决办法: 为字符串常量分配确定的地址
static const char hello[] = "hello";  // 确定的地址
SAME_TYPE(CC<hello>, CC<hello>);

// 每次都为字符串常量分配地址很繁琐
// C++20 允许用户自定义类型作为非类型模板参数，我们可以将字符串封装到自定义类型来为字符串提供确定的地址
template <size_t N>
struct FixedString {
  char str[N];
  constexpr FixedString(const char(&s)[N]) {
    std::copy_n(s, N, str);
  }
};  // struct FixedString
template <FixedString str>
struct CCC {
};  // struct CCC
SAME_TYPE(CCC<"hello">, CCC<"hello">);
NOT_SAME_TYPE(CCC<"hello">, CCC<"world">);

// 对于使用 auto 表达非类型参数，类型由编译器推导，字符串常量被推导为 const char*
// 同样, "hello" 也不能作为 ValueList 的模板参数
// ValueList<"hello"> vl;

// 自定义后缀操作符
template <FixedString str>
constexpr decltype(str) operator""_fs() {
  return str;
}
ValueList<"hello"_fs> vl;

// 通过多个参数定制类的行为, 模板参数必须按照先后顺序给出
template <bool move_constructable = true, bool copy_constructable = true,
    bool move_assignable = true, bool copy_assignable = true>
struct Counted {
};  // struct Counted
// 即使前两个使用默认的 true, 还是需要写满 4 个模板参数
// 这样很麻烦
using ConstructOnly = Counted<true, true, false, false>;

// 自定义一个参数类，然后将这个类作为非类型模板参数的类型
struct CountedPolicy {
  bool move_constructable = true;
  bool copy_constructable = true;
  bool move_assignable = true;
  bool copy_assignable = true;
};  // struct CountedPolicy
inline constexpr CountedPolicy default_counted_policy;
template <CountedPolicy policy = default_counted_policy>
struct Counted2 {
};  // struct Counted2
using ConstructOnly2 = Counted2<{
  .move_assignable = false,
  .copy_assignable = false
}>;

void run_nonetype_template_parameter() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::println("decltype(x)={}", cpp_utils::type_name<decltype(x)>);
}


// 6.6 constexpr 与 TypeList
// constexpr auto result = type_list<int, char, long, char, short, float, double>
//     | filter([]<typename T>(TypeConst<T>) { return _v<(sizeof(T) < 4); })
//     | transform([]<typename T>(TypeConst<T>) { return _t<add_pointer_t<T>>; })
//     | unique()
//     | convert_to<variant>();
// static_assert(result == _t<variant<char*, short*>>);

// 使用模板元编程实现上面的 result
#include <variant>
#include "chapter_05/algorithm.h"
#include "chapter_05/higher_order_functions.h"
using In = TypeList<int, char, long, char, short, float, double>;
template <typename E>
using TypeSizeLess4 = std::bool_constant<(sizeof(E) < 4)>;
using Res = Unique_t<Map_t<Filter_t<In, TypeSizeLess4>, std::add_pointer>>::to<std::variant>;
SAME_TYPE(Res, (std::variant<char*, short*>));

// 6.6.1 类型、值的包裹类
template <typename T>
struct TypeConst {
  using type = T;
};  // struct TypeConst
template <typename T>
inline constexpr TypeConst<T> _t;
template <auto v>
struct ValueConst {
  constexpr static auto value = v;
};  // struct ValueConst
template <auto v>
inline constexpr ValueConst<v> _v;

template <typename... Ts>
inline constexpr auto type_list = TypeList<TypeConst<Ts>...>{};

template <typename... LHs, typename... RHs>
consteval bool operator==(TypeList<LHs...>, TypeList<RHs...>) {
  if constexpr (sizeof...(LHs) != sizeof...(RHs)) {
    return false;
  } else {
    return ((std::is_same_v<LHs, RHs>) && ...);
  }
}

using F = decltype([]<typename T>(TypeConst<T>) {
  return _t<std::add_pointer_t<T>>;
});
using Res2 = std::invoke_result_t<F, TypeConst<int>>;
SAME_TYPE(Res2, TypeConst<int*>);
auto transform_impl = []<typename F, typename... Ts>(TypeList<Ts...>, F)
    -> TypeList<std::invoke_result_t<F, Ts>...> {
  return {};
};
template <typename P, typename Result, typename... Ts>
struct FilterImpl : std::type_identity<Result> {
};  // struct FilterImpl
template <typename P, typename... Rs, typename H, typename... Ts>
struct FilterImpl<P, TypeList<Rs...>, H, Ts...> :
    std::conditional_t<std::invoke_result_t<P, H>::value,
        FilterImpl<P, TypeList<Rs..., H>, Ts...>,
        FilterImpl<P, TypeList<Rs...>, Ts...>> {
};  // struct FilterImpl<P, TypeList<Rs...>, H, Ts...>

auto filter_impl = []<typename P, typename... Ts>(TypeList<Ts...>, P)
    -> FilterImpl<P, TypeList<>, Ts...>::type {
  return {};
};
auto unique_impl = []<typename... Ts>(TypeList<Ts...>)
    -> Unique_t<TypeList<Ts...>> {
  return {};
};

auto tl = type_list<int, char, double, int>;
auto res = transform_impl(tl, []<typename T>(TypeConst<T>) {
  return _t<std::add_pointer_t<T>>;
});
static_assert(res == type_list<int*, char*, double*, int*>);
auto res2 = filter_impl(tl, []<typename T>(TypeConst<T>) {
  return _v<TypeSizeLess4<T>::value>;
});
static_assert(res2 == type_list<char>);
auto res3 = unique_impl(tl);
static_assert(res3 == type_list<int, char, double>);

// 6.6.3 管道操作符
#include "concepts.h"
template <typename Fn>
struct PipeAdapter : private Fn {
  consteval PipeAdapter(Fn) {}
  template <typename... Args>
  requires(std::invocable<Fn, TypeList<>, Args...>)
  consteval auto operator()(Args... args) const {
    return [=, this](auto vl) consteval {
      return static_cast<const Fn&>(*this)(vl, args...);
    };
  }
  using Fn::operator();
};  // struct PipeAdapter

template <typename VL, typename Adapter>
consteval auto operator|(VL vl, Adapter adapter) { return adapter(vl); }
inline constexpr auto transform = PipeAdapter(transform_impl);
inline constexpr auto filter = PipeAdapter(filter_impl);
inline constexpr auto unique = PipeAdapter(unique_impl);

SAME_TYPE(In, (TypeList<int, char, long, char, short, float, double>));
auto res4 = In{}
    | filter([]<typename T>(T) { return TypeSizeLess4<T>{}; })
    | transform([]<typename T>(T) { return std::add_pointer_t<T>(); })
    | unique();
SAME_TYPE(decltype(res4), (TypeList<char*, short*>));
static_assert(res4 == TypeList<char*, short*>{});



// 6.6.4 重构 KV 数据表
template <typename P, typename Satisfied, typename Unsatisfied, typename... Ts>
struct PartitionImpl : std::type_identity<std::pair<Satisfied, Unsatisfied>> {
};  // struct PartitionImpl
template <typename P, typename... Ss, typename... Us, typename H, typename... Ts>
struct PartitionImpl<P, TypeList<Ss...>, TypeList<Us...>, H, Ts...> :
    std::conditional_t<std::invoke_result_t<P, H>::value,
        PartitionImpl<P, TypeList<Ss..., H>, TypeList<Us...>, Ts...>,
        PartitionImpl<P, TypeList<Ss...>, TypeList<Us..., H>, Ts...>> {
};  // PartitionImpl<P, TypeList<Ss...>, TypeList<Us...>, H, Ts...>
auto partition_impl = []<typename P, typename... Ts>(TypeList<Ts...>, P)
    -> PartitionImpl<P, TypeList<>, TypeList<>, Ts...>::type {
  return {};
};
inline constexpr auto partition = PipeAdapter(partition_impl);
auto append_impl = []<typename T, typename... Ts>(TypeList<Ts...>, T)
    -> TypeList<Ts..., T> {
  return {};
};
inline constexpr auto append = PipeAdapter(append_impl);
auto prepend_impl = []<typename T, typename... Ts>(TypeList<Ts...>, T)
    -> TypeList<T, Ts...> {
  return {};
};
inline constexpr auto prepend = PipeAdapter(prepend_impl);

template <size_t N, typename T>
struct NthType;

template <size_t N, template <typename...> typename Input, typename... Ts>
struct NthType<N,  Input<Ts...>> {
  using type = std::tuple_element_t<N, std::tuple<Ts...>>;
};  // struct NthType<N, Input<Ts...>>

#include "chapter_05/kv_table.h"
template <KVEntry E1, KVEntry E2>
consteval auto is_same_group() {
  return E1::dim == E2::dim &&
      sizeof(typename E1::type) == sizeof(typename E2::type) &&
      alignof(typename E1::type) == alignof(typename E2::type);
}

consteval static auto group_entries(auto es) {
  if constexpr (0 == es.size) {
    return TypeList<>{};
  } else {
    using HeadType = NthType<0, decltype(es)>::type;
    constexpr auto group_result = es | partition([]<typename Entry>(Entry) {
      return _v<is_same_group<HeadType, Entry>()>;
    });
    constexpr auto group_rest = group_entries(group_result.second);
    return group_entries(group_result.second) | prepend(group_result.first);
  }
}

template <template <typename...> typename Target>
struct ConvertToType{
};  // struct ConvertToType

template <template <typename...> typename Target>
consteval auto convert_to() {
  return ConvertToType<Target>{};
}
template <template<typename...> typename Target, typename... Sources>
consteval auto convert_to(TypeList<Sources...>) {
  return Target<Sources...>{};
}

template <template<typename...> typename Target, typename... Sources>
consteval auto operator|(TypeList<Sources...> source, ConvertToType<Target> c) {
  return convert_to<Target>(source);
}
using AllEntries = TypeList<
    Entry<0, int>, Entry<1, char>, Entry<2, char>,
    Entry<3, short>, Entry<4, char[10]>, Entry<5, char[10]>,
    Entry<6, int>
>;
constexpr static auto entry_groups = group_entries(AllEntries{});
constexpr static auto regions = entry_groups
    | transform([](auto group) {
      return group | convert_to<GenericRegion>();
    })
    | convert_to<Regions>();
// 这里只通过constexpr计算了Regions, 主要是如何利用 constexpr 进行group

// template <typename> struct dump;
// using T1 = decltype(regions);
// dump<decltype(regions)> t1{};
SAME_TYPE(decltype(regions), (
  const Regions<
      GenericRegion<Entry<0, int, 1>, Entry<6, int, 1>>,
      GenericRegion<Entry<1, char, 1>, Entry<2, char, 1>>,
      GenericRegion<Entry<3, short int, 1>>,
      GenericRegion<Entry<4, char [10], 1>, Entry<5, char [10], 1>>
  >
));
void run_constexpr_group() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::println("Regions type_name: {}", cpp_utils::type_name<decltype(regions)>);
}

// 6.7 综合运用之编译时字符串操作
template <typename T>
constexpr auto strLength = strLength<std::remove_cvref_t<T>>;
template <size_t N>
constexpr size_t strLength<char[N]> = N - 1;
template <size_t N>
constexpr size_t strLength<std::array<char, N>> = N - 1;
static_assert(strLength<decltype("hello")> == 5);

template <typename DelimType, size_t N>
struct JoinStringFold {
  consteval JoinStringFold(DelimType delimiter) : delimiter_(delimiter) {}
  template <typename STR>
  friend decltype(auto) consteval operator+(JoinStringFold&& self, STR&& str) {
    self.p_str = std::copy_n(std::begin(str), strLength<STR>, self.p_str);
    if (self.joined_str.end() - self.p_str > strLength<DelimType>) {
      self.p_str = std::copy_n(self.delimiter_, strLength<DelimType>, self.p_str);
    }
    return std::forward<JoinStringFold>(self);
  }
  std::array<char, N + 1> joined_str{};
  DelimType delimiter_;
  decltype(joined_str.begin()) p_str = joined_str.begin();
};  // struct JoinStringFold

template <typename DelimType, typename... STRs>
consteval auto join(DelimType&& delimiter, STRs&&... strs) {
  constexpr size_t str_num = sizeof...(STRs);
  constexpr size_t len = (strLength<STRs> + ... + 0) +
      (str_num >= 1 ? str_num - 1 : 0) * strLength<DelimType>;
  return (JoinStringFold<DelimType, len>{std::forward<DelimType>(delimiter)}
      + ... + std::forward<STRs>(strs)).joined_str;
}

template <typename... STRs>
consteval auto concat(STRs&&... strs) {
  return join("", std::forward<STRs>(strs)...);
}
template <typename STR1, typename STR2>
constexpr bool equal(STR1&& str1, STR2&& str2) {
  constexpr size_t len1 = strLength<STR1>;
  constexpr size_t len2 = strLength<STR2>;
  if (len1 != len2) {
    return false;
  }
  for (size_t i = 0; i < len1; ++i) {
    if (str1[i] != str2[i]) { return false; }
  }
  return true;
}
constexpr auto one_two = concat("one", "two");
static_assert(equal("onetwo", one_two));
constexpr auto one_two_three = concat(one_two, "three");
static_assert(equal("onetwothree", one_two_three));

int main() {
  run_consteval_function();
  run_dynamic_container_to_static_container();
  run_is_constant_evaluated();
  run_nonetype_template_parameter();
  run_constexpr_group();
  return 0;
}
