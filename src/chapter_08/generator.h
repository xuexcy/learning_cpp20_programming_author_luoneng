/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/07 15:34:14
# Desc   :
########################################################################
*/
#pragma once

#include <coroutine>
#include <exception>

template <typename T>
struct Generator {
  struct promise_type;
  using handle = std::coroutine_handle<promise_type>;

  struct promise_type {
    Generator<T> get_return_object() {
      return {handle::from_promise(*this)};
    }
    auto initial_suspend() {
      return std::suspend_never{};
    }
    auto final_suspend() {
      return std::suspend_always{};
    }
    void unhandled_exception() {
      std::terminate();
    }
    void return_void() {}
    auto yield_value(T value) {
      current_value_ = value;
      return std::suspend_always{};
    }
    T current_value_;
  };  // struct promise_type

  void next() {
    return coro_handle_.resume();
  }
  bool done() {
    return coro_handle_.done();
  }
  int current_value() {
    return coro_handle_.promise().current_value_;
  }
  Generator(Generator&& rhs) noexcept: coro_handle_(std::exchange(rhs.coro_handle_, {})) {}
  ~Generator() {
    if (coro_handle_) {
      coro_handle_.destroy();
    }
  }
private:
  Generator(handle h): coro_handle_(h) {}
  handle coro_handle_;
};  // struct Generator
