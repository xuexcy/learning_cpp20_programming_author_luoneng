/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/05/17 16:32:14
# Desc   :
########################################################################
*/
#pragma once

#include <numbers>
#include <variant>

/*
subtype 多态，构造的对象需要常驻内存，可以使用智能指针来管理内存
*/

/*
自由函数 method(obj)
成员函数 obj.method(obj)
*/

/*
扩展性
1. 增加一个类，subtype 可以直接继承基类，不需要修改源代码, 但 ad_hoc 需要修改 Shape 的定义, 并且影响自由函数对心累的扩充
2. 增加一个行为，ad_hoc 方法只需要新增一系列自由函数即可， subtype 需要修改基类，新增虚函数, 所有的子类的定义需要修改

subtype 对类扩展开放，对行为扩展封闭
ad_hoc 对行为扩展开放，对类扩展封闭
如果类的数量比较稳定，能够在编译时枚举出来，就用 ad_hoc ，否则就用 subtype
*/

constexpr auto kPI=  std::numbers::pi;
namespace subtype {
// 继承多态
struct Shape {
  virtual ~Shape() = default;
  virtual double get_area() const = 0;
  virtual double get_perimeter() const = 0;  // 周长
};  // struct Shape
struct Circle : public Shape {
  Circle(double r) : r_(r) {}
  double get_area() const override { return kPI * r_ * r_; }
  double get_perimeter() const override { return 2 * kPI * r_; }
private:
  double r_;
};  // struct Circle
struct Rectangle : public Shape {
  Rectangle(double w, double h) : w_(w), h_(h) {}
  double get_area() const override { return w_ * h_; }
  double get_perimeter() const override { return 2 * (w_ + h_); }
private:
  double w_;
  double h_;
};  // struct Rectangle
}  // namespace subtype

namespace ad_hoc {
struct Circle {
  double r;
};  // struct Circle
inline double get_area(const Circle& c) { return kPI * c.r * c.r; }
struct Rectangle {
  double w;
  double h;
};  // struct Rectangle
inline double get_area(const Rectangle& r) { return kPI * r.w * r.h; }
// std::variant 多态
using Shape = std::variant<Circle, Rectangle>;
inline double get_area(const Shape& s) {
    return std::visit([](const auto& data) { return get_area(data); }, s);
}
}  // namespace ad_hoc
