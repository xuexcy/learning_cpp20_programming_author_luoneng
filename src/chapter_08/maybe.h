/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/10 13:55:59
# Desc   :
########################################################################
*/
#pragma once

#include <coroutine>
#include <optional>

template <typename T>
struct maybe : public std::optional<T> {
  using Base = std::optional<T>;
  using Base::Base;

  struct promise_type;
  using Handle = std::coroutine_handle<promise_type>;

  struct promise_type {
    std::suspend_never initial_suspend() noexcept {
      return {};
    }
    std::suspend_always final_suspend() noexcept {
      return {};
    }
    void unhandled_exception() {}
    maybe<T> get_return_object() {
      return {&res_};  // 将 res_ 指向返回的 maybe 对象
    }

    template <typename U>
    void return_value(U&& value) {
      res_->emplace(std::forward<U>(value));
    }
    auto yield_value(maybe<T> opt) {
      struct Awaiter {
        bool await_ready() {
          return opt_.has_value();
        }
        T await_resume() {
          return std::move(opt_.value());
        }
        void await_suspend(std::coroutine_handle<> handle) {
          handle.destroy();
        }
        maybe<T> opt_;
      };  // struct Awaiter
      return Awaiter{std::move(opt)};
    }
    maybe<T>* res_{};
  };  // struct promise_type

  maybe(maybe** p) {
    *p = this;
  }
  maybe(maybe&& rhs) noexcept: Base(std::move(rhs)) {
  }
};  // struct maybe : public std::optional<T>
