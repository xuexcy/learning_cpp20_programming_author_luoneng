/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyun@gmail.com
# Date   : 2026/03/18 00:18:57
# Desc   : 10.1 配置文件反序列化框架
########################################################################
*/

#include <print>
#include <cassert>

#include "config_loader/loader.h"
#include "config_loader/result.h"
#include "schema.h"

void run_point() {
  Point point;
  static_assert(4 == Point::_field_count_);
  forEachField(point, [](auto&& field_info) {
    std::println("field: {}", field_info.name());
  });
  {
    Point point;
    assert(Result::SUCCESS == loadJSON2Obj(point, "../conf/point.json"));
    assert(1 == point.x);
    assert(2 == point.y);
    assert(std::nullopt == point.z);

    /*
    根据书中的实现，这里肯定是有问题的, 实现逻辑是: 将 elem 转换成 std::string, 然后由折叠表达式去匹配
    比如 std::variant<std::string, int, double> 和 {"x": 2} 匹配, 那么 JsonParser 先加载数据，然后
    会将 value 2 通过 getValueText() 接口转换成 std::string("2"), 然后通过折叠表达式使用 primitive_deserialize
    进行匹配，只是 "2" 会优先和 std::variant 中的第一个类型 std::string 匹配成功
    另外 std::variant<int, double> 和 {"x": 2.4} 也会匹配成 int, 因为 2.4 -> "2.4", 然后浮点数可以转换成 int
    std::variant<double, int> 和 {"x": 1} 会匹配成 double, 因为整型可以转换为double
    错误的两个主要原因是:
    1. schema 中定义的类型一定是由 Primitive 类型组成的，而 concept_primitive.h 中调用了 getValueText() 导致
    原文件中primitive数据都变成了std::string, 将类型信息丢弃
    2. 对于 std::variant中的类型没有考虑优先级，只要转换成功就算
    */
    assert(0 == point.other.index());
    std::println("std::variant value: {}", std::get<std::string>(point.other));
  }
  {
    Point point;
    assert(Result::ERR_EMPTY_CONTENT == loadJSON2Obj(point, "../conf/point2.json"));
  }
  {
    Point point;
    assert(Result::ERR_ILL_FORMED == loadJSON2Obj(point, "../conf/point3.json"));
  }
  {
    Point point;
    assert(Result::ERR_MISSING_FIELD == loadJSON2Obj(point, "../conf/point4.json"));
  }
  {
    Point point;
    assert(Result::ERR_EXTRACTING_FIELD == loadJSON2Obj(point, "../conf/point5.json"));
  }
  {
    TestTree test_tree;
    assert(Result::SUCCESS == loadJSON2Obj(test_tree, "../conf/test_tree.json"));
    assert("root" == test_tree.name);
    assert(3 == test_tree.children.size());
    assert("mid_right" == test_tree.children[1]->children[1]->name);
  }
  {
    TestTree test_tree;
    assert(Result::ERR_TYPE == loadJSON2Obj(test_tree, "../conf/test_tree2.json"));
  }
  {
    TestTree test_tree;
    assert(Result::ERR_UNSUPPORTED_PARSER == loadYAML2Obj(test_tree, "../conf/test_tree2.json"));
  }
  //assert(Result::SUCCESS == 1);
  //assert(Result::ERR_ILL_FORMED == 2);
  //assert(Result::ERR_ILL_FORMED == 2);
}

int main() {
  run_point();
  return 0;
}

