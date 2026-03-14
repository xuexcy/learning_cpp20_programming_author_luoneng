/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2026/01/22 14:53:35
# Desc   : 第 8 章 协程
########################################################################
*/

#include <concepts>
#include <coroutine>
#include <exception>
#include <future>
#include <generator>
#include <iterator>
#include <numeric>
#include <optional>
#include <print>
#include <tuple>
#include <utility>
#include <vector>

#include "cpp_utils/util.h"

// 8.2 协程初探
std::generator<int> fibo(int max_num) {
  int a = 1;
  int b = 1;
  while (a < max_num) {
    co_yield  a;  // 让出控制流
    std::tie(a, b) = std::make_tuple(b, a + b);
  }
  co_return;
}

void run_fibo() {
  PRINT_CURRENT_FUNCTION_NAME;
  for (auto f : fibo(1000000)) {
    std::println("{}", f);
  }
}

// 8.3 函数与协程理论
// 有栈协程: https://mthli.xyz/stackful-stackless/
// 无栈协程: https://mthli.xyz/coroutines-in-c/
// TODO(xuechengyun)

// 8.4 揭秘 co_await 表达式
// 编译器将 co_await expr 进行两步转换，得到 Awaiter 类
// 8.4.1 表达式转换过程
/*
两步: 先 transform 再 co_await
1. 将 expr 转换成 Awaitable 对象
    - auto awaitable = p.await_transform(expr);
    - auto awaitable = expr;
2. 调用 Awaitable 对象的 co_wait 操作符
    - auto awaiter = awaitable.operator co_await();
    - auto awaiter = operator co_await(awaitable);
    - auto awaiter = awaitable;
最后有 2 x 3 = 6 中可能性
*/
template <typename P, typename Expr>
decltype(auto) get_awaitable(P& p, Expr&& expr) {
  if constexpr (requires { p.await_transform(std::forward<Expr>(expr)); }) {
    // Promise 对象存在成员函数 await_transform
    return p.await_transform(std::forward<Expr>(expr));
  } else {
    return std::forward<Expr>(expr);
  }
}
#include "chapter_08/has_operator.h"
template <typename Awaitable>
decltype(auto) get_awaiter(Awaitable&& awaitable) {
  if constexpr (has_member_operator_co_await_v<Awaitable&&>) {
    // 成员操作符 co_await
    return std::forward<Awaitable>(awaitable).operator co_await();
  } else if constexpr (has_non_member_operator_co_await_v<Awaitable&&>) {
    // 非成员操作符 co_await
    return operator co_await(std::forward<Awaitable>(awaitable));
  } else {
    return std::forward<Awaitable>(awaitable);
  }
}
template <typename Expr>
using GetAwait_t = decltype(get_awaiter(std::declval<Expr>()));
// 8.4.2 Awaiter 对象
/*
Awaiter 对象需要提供三个接口
1. await_ready: 判断当前协程是否需要在此处挂起
2. await_suspend: 接受协程句柄 coroutine_handle, 用户可以发起异步动作，完成后对 coroutine_handle执行 resume()
    恢复当前协程
  a. 返回类型 void: 挂起(suspend)后 return
  b. 返回 true: 挂起(suspend)后 return
  c. 返回 false: 恢复(resume)后 return
  d. 返回 coroutine_handle: 挂起(suspend)后 return, 然后恢复(resume)这个返回的 coroutine_handle
3. await_resume: 当协程恢复时，将返回值作为 co_await 表达式的值
*/
template <typename Expr>
concept C_Awaitable = requires(GetAwait_t<Expr> awaiter, std::coroutine_handle<> handle) {
  { awaiter.await_ready() } -> std::convertible_to<bool>;
  awaiter.await_suspend(handle);
  awaiter.await_resume();
};

/* 编译器对 co_await 表达式进行处理
比如当编译器遇到 auto result = co_wait <expr>; 时, 就可能处理成
  using Future = decltype(expr);
  using Promise = Future::promise_type;
  std::coroutine_handle<Promise> handle;
  Promise p;
  auto awaiter = get_awaiter(get_awaitable(p, expr));
  ReturnType res = process_co_await(awaiter, handle);
*/

template <C_Awaitable Awaiter, typename Handle>
auto try_await_suspend(Awaiter& awaiter, Handle& handle) {
  try {
    return awaiter.await_suspend(handle);
  } catch(...) {
    resume_coroutine(handle);
    throw;
  }
}
void return_to_caller() {}

template <C_Awaitable Awaiter, typename Handle>
void suspend(Awaiter& awaiter, Handle& handle) {
  // 将协程标记为挂起状态
  // 先标记以防止和恢复产生竞争关系, 如果在 await_suspend 中或之后标记, await_suspend 和 await_resume 的
  // 逻辑就会有冲突, 也就是先标记状态，再进行逻辑处理
  mark_suspended_state(handle);
  // 在协程句柄中记录当前挂起的位置，以便后续能够从这个位置恢复
  save_suspended_point(handle);

  // 执行 await_suspend
  // 可以在 await_suspend 中自定义一些逻辑，有栈协程做不到这一点, 因为有栈协程框架将协程切换打包成了一个上下文切换
  // 的原子动作
  using SuspendResult = decltype(awaiter.await_suspend(handle));
  if constexpr (std::is_same_v<SuspendResult, void>) {
    try_await_suspend(awaiter, handle);
    return_to_caller();
  } else if constexpr (std::is_same_v<SuspendResult, bool>) {
    if(try_await_suspend(awaiter, handle)) {
      return_to_caller();
    } else {
      unmark_suspended_state(handle);
    }
  } else if constexpr (std::is_same_v<SuspendResult, std::coroutine_handle<>>) {
    auto next_handle = try_await_suspend(awaiter, handle);
    if (next_handle == handle) {
      unmark_suspended_state(handle);
    } else if (next_handle) {
      transfer_to_next_coroutine(next_handle);
    } else {
      return_to_caller();
      resume_with_exception(handle, std::current_exception());
    }
  }

  clear_suspended_state(handle);
}
template <C_Awaitable Awaiter>
auto await_resume(Awaiter& awaiter) {
  try {
    return awaiter.await_resume();
  } catch (...) {
    throw;
  }
}

template <C_Awaitable Awaiter, typename Handle>
auto process_co_await(Awaiter& awaiter, Handle& handle) {
  if (!awaiter.await_ready()) {
    // 动作是同步的，或者值已经存在，
    suspend(awaiter, handle);
  }
  await_resume(awaiter, handle);
}

struct suspend_always {
  constexpr bool await_ready() const noexcept {
    return false;
  }
  constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
  constexpr void await_resume() const noexcept {}
};  // struct suspend_always

struct suspend_never {
  constexpr bool await_ready() const noexcept { return true; }
  constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
  constexpr void await_resume() const noexcept {}
};  // struct suspend_never

static_assert(C_Awaitable<std::suspend_always>);
static_assert(C_Awaitable<std::suspend_never>);

// 8.5 揭秘 Promise 概念
// Promise 概念用于描述一个未知值的对象，生产者通过其提供值，消费者通过对应 Future 获取值
// Promise<T>.set_value -> Result<T> -> Future<T>.get_result
// Future 中包含 Promise, Future 用于调用者与协程交互，Promise 用于管理协程状态
//


// 8.5.1 协程句柄
// 协程句柄存储了协程帧
// 协程帧中存储了协程恢复、销毁两个函数指针，协程实参、内部状态，和保存的局部变量

struct frame {  // coroutine_state
  using resume_fn_t = void(frame*);
  using destroy_fn_t = void(frame*);
  resume_fn_t* resume_fn; // void (*resume_fn)(frame_ptr);
  destroy_fn_t* destroy_fn;
  // promise_type
  // 协程实参
  // 协程状态
  // 保存的局部变量
};  // struct frame

template <typename Promise_type=void>
struct coroutine_handle;

std::coroutine_handle<> t;
template <>
struct coroutine_handle<void> {
  constexpr void* address() const noexcept;  // 获取协程帧地址
  // 从协程帧地址构造
  constexpr static coroutine_handle from_address(void*) noexcept;
  constexpr explicit operator bool() const noexcept;  // 判断协程是否结束
  bool done() const noexcept;  // 判断协程是否结束
  void operator()() const;  // 恢复协程
  void resume() const;  // 恢复协程
  void destroy() const;  // 销毁协程
private:
  void* frame_ptr;  // 存放协程帧指针
};  // struct coroutine_handle<void>

template <typename Promise>
struct coroutine_handle {
  static coroutine_handle from_promise(Promise&);
  Promise& promise() const;
private:
  void* frame_ptr;
};  // struct coroutine_handle

/* 编译器对协程的代码进行变换
1. 动态分配协程帧内存 operator new
2. 将调用者传递的实参考到协程帧
    a. 按值传递, 通过移动构造
    b. 按引用传递, 仅传递引用
3. 构造 Promise 对象
*/
/*
frame* f = new frame{
  .resume_fn = &coroutine_resume,
  .destroy_fn = &coroutine_destroy
};
TaskPromise<int> promise = new (&f->promise_storage)TaskPromise<int>();
Task<int> future = promise->get_return_object();
future.handle_ = std::coroutine_handle<TaskPromise<int>>::from_promise(*promise);
auto awaiter = promise->init_suspend();
if (!awaiter.await_ready) {
  awaiter.await_suspend(future.handle_);
  return future;
}
// 开始执行协程体
*/


// 8.5.2 Promise 概念
// 1. coroutine_traits 函数
//    a. 通过使用这个函数将 Future::promise_type 作为 Promise
//    b. 如果没有 Future::promise_type, 可以特化 coroutine_traits 来扩展 Future 的 promise_type
template <typename, typename...>
struct coroutine_traits {
};  // struct coroutine_traits
template <typename R, typename... Args>
requires requires {
  typename R::promise_type;
}
struct coroutine_traits<R, Args...> {
  using promise_type = typename R::promise_type;
};  // struct coroutine_traits<R, Args...>

// int 没有 promise_type, 特化一个玩玩
template <>
struct coroutine_traits<int> {
  using promise_type = uint32_t;
};  // struct coroutine_traits<int>

// 对于 fibo 协程，Promise类下为 std::generator<int>::promise_type


// 2. 构造 Promise 与 Future 对象
struct Future {
  struct promise_type;
  using handle = std::coroutine_handle<promise_type>;
  struct promise_type {
    template <typename Obj>
    promise_type(Obj&&, int v) {
      std::println("member of lambda coroutine, v = {}", v);
    }
    promise_type(int v) {
      std::println("free coroutine, v = {}", v);
    }
    promise_type() {
      std::println("default constructor");
    }
    void return_void() {}
    auto initial_suspend() {
      return suspend_never{};
    }
    auto final_suspend() noexcept {
      return suspend_always{};
    }
    void unhandled_exception() {
      std::terminate();
    }
    // Future 存储协程句柄handle，通过其 from_promise 来获取协程句柄
    Future get_return_object() {
      return {handle::from_promise(*this)};
    }
  };  // struct promise_type
  Future(Future&& rhs) noexcept: coro_handle_(std::exchange(rhs.coro_handle_, {})) {
  }
  // Future 不仅需要作为消费者获取值，还需要管理协程的声明周期，通过析构函数对协程句柄进行释放
  ~Future() {
    if (coro_handle_) {
      coro_handle_.destroy();
    }
  }
private:
  Future(handle h): coro_handle_(h) {}
  handle coro_handle_;
};  // struct Future
// 编译时元函数 coroutine_traits 根据协程的返回类型 Future 得到对应的 Promise 类型，即 Future::promise_type
// 运行时，在构造 Promise 对象后接着构造 Future (promise.get_return_object())
struct Class {
  Future member_coro(int) {
    co_return;
  }
};  // struct Class
Future free_coro(int) {
  co_return;
}
Future free_coro2(std::string) {
  co_return;
}
auto lambda_coro = [](int) -> Future {
  co_return;
};

void run_future() {
  PRINT_CURRENT_FUNCTION_NAME;
  Class obj;
  obj.member_coro(0);
  lambda_coro(1);
  free_coro(2);
  free_coro2("hello");  // default constructor;
}

template <typename F>
concept C_Future = std::move_constructible<F> && requires(F f) {
  { std::move(f).get() };
};
// 使用 concept 表达 Promise 概念
template <typename P>
concept Promise = requires (P p) {
  { p.get_return_object() } -> C_Future;
  { p.initial_suspend() } -> C_Awaitable;
  { p.final_suspend() } noexcept -> C_Awaitable;
  p.unhandled_exception();
  requires (
    requires(int v) {
      p.return_value(v);
    } ||
    requires {
      p.return_void();
    }
  );
};


// 8.6 综合运用
// 8.6.1 生成器
#include "generator.h"

// 8.6.2 为已有类型非侵入式扩展协程接口
/*
1. 特化 coroutine_traits 元函数，以便让编译器找到协程的 Promise
2. c++ 标准要求特化 std空间内的模板时，特化的模板参数至少要有一个自定义类型
    a. 添加一个自定义 AsCoroutine 类型
3. future 需要符合 Awaitable 概念
*/

// c++ 标准要求特化 std空间内的模板时，特化的模板参数至少要有一个自定义类型
// 这样可以避免和未来标准库升级产生冲突
struct AsCoroutine {
};  // struct AsCoroutine
inline constexpr AsCoroutine as_coroutine;


template <typename T, typename... Args>
struct std::coroutine_traits<std::future<T>, AsCoroutine, Args...> {
  struct Awaiter {
    bool await_ready() {
      return fut_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }
    void await_suspend(std::coroutine_handle<> handle) {
      std::thread([=, this] {
        fut_.wait();
        handle.resume();
      }).detach();
    }
    decltype(auto) await_resume() {
      return fut_.get();
    }
    std::future<T> fut_;
  };  // struct Awaiter

  // 这样，std::future<T> 的 promise_type 就是 std::promise<T>
  struct promise_type : std::promise<T> {
    Awaiter await_transform(std::future<T> fut) {
      return {std::move(fut)};
    }
    std::future<T> get_return_object() {
      return this->get_future();
    }
    auto initial_suspend() noexcept {
      return std::suspend_never{};
    }
    auto final_suspend() noexcept {
      return std::suspend_never{};
    }
    void unhandled_exception() {
      this->set_exception(std::current_exception());
    }
    template <typename U>
    void return_value(U&& value) {
      this->set_value(std::forward<U>(value));
    }
  };  // struct promise_type : std::promise<T>
};  // struct std::coroutine_handle<std::future<T>, Args...>

template <typename... Args>
struct std::coroutine_traits<std::future<void>, AsCoroutine, Args...> {
  struct promise_type : std::promise<void> {
    std::future<void> get_return_object() {
      return this->get_future();
    }
    auto initial_suspend() noexcept {
      return std::suspend_never{};
    }
    auto final_suspend() noexcept {
      return std::suspend_never{};
    }
    void unhandled_exception() {
      this->set_exception(std::current_exception());
    }
    void return_void() {
      this->set_value();
    }
  };  // struct promise_type : std::promise<T>
};  // struct std::coroutine_handle<std::future<T>, Args...>

template <std::random_access_iterator RandIt>
std::future<int> parallel_sum(AsCoroutine, RandIt beg, RandIt end) {
  auto len = end - beg;
  if (0 == len) { co_return 0; }
  RandIt mid = beg + len / 2;
  auto rest_task = std::async([](RandIt b, RandIt e) {
    return std::accumulate(b, e, 0);
  }, mid, end);
  auto first_task = parallel_sum(as_coroutine, beg, mid);
  // first_task 和 rest_task 的返回结果都是 std::future<int>
  // 通过特化 coroutine_traits，编译器知道了 std::future<int> 的 promise_type 是 std::promise<int>
  // 这样，std::future 就支持了协程
  auto first = co_await std::move(first_task);
  auto rest = co_await std::move(rest_task);
  co_return first + rest;
}

void run_sum() {
  PRINT_CURRENT_FUNCTION_NAME;
  std::vector v(100000000, 1);
  std::println("sum is: {}", parallel_sum(as_coroutine, v.begin(), v.end()).get());
}

// 8.6.3 利用协程机制简化错误处理
/*
std::optional<int> read_int();
std::optional<int> compute() {
  int x;
  int y;
  if (auto v = read_int()) {
    x = v.value();
  } else {
    return std::nullopt;
  }
  if (auto v = read_int()) {
    y = v.value();
  } else {
    return std::nullopt;
  }
}
*/
#include "maybe.h"
maybe<int> read_int() {
  static int count = 0;
  auto res = count;
  ++count;
  if (res < 2) {
    return std::nullopt;
  } else {
    return res;
  }
}
maybe<int> compute() {
  int x = co_yield read_int();
  int y = co_yield read_int();
  co_return x + y;
}

void maybe_maybe() {
  auto res = compute();
  if (res) {
    std::println("result is: {}", *res);
  } else {
    std::println("compute failed");
  }
}

void run_maybe() {
  PRINT_CURRENT_FUNCTION_NAME;
    maybe_maybe();
    int i = 1;
    maybe_maybe();
    int j = 1;
    maybe_maybe();
    int k = 1;
    maybe_maybe();
    int r = 1;
    maybe_maybe();
}
int main() {
  run_fibo();
  run_future();
  run_sum();
  run_maybe();
  return 0;
}
