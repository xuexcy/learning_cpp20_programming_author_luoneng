/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/23 23:50:52
# Desc   :
########################################################################
*/
#pragma once

#include <optional>
#include <string>
#include "../result.h"

#include "json/json.h"

namespace parser {

class JsonElementType {
public:
  JsonElementType() = default;
  explicit JsonElementType(const Json::Value& elem, const char* key_name = nullptr)
      : key_name_(key_name), elem_(elem) {}
  bool isValid() const {
    return !elem_.isNull();
  }
  std::optional<std::string> getValueText() const {
    if (elem_.isObject() || elem_.isArray()) {
      return std::nullopt;
    }
    return elem_.asString(); // 2和"2" 调用 asString() 后都是 "2", 无法通过该结果来判断原始类型是数字还是字符串
  }
  const char* getKeyName() const {
    return key_name_;
  }
  JsonElementType toChildElem(std::string_view key) const {
    if (!elem_.isObject()) {
      return JsonElementType{Json::Value::nullSingleton()};
    }
    return JsonElementType{elem_[key.data()], key.data()};
  }
  template <typename F>
  Result forEachElement(F&& f) const {
    switch (elem_.type()) {
      case Json::ValueType::nullValue:
        return Result::SUCCESS;
      case Json::ValueType::arrayValue:
        for (auto&& e: elem_) {
          CHECK_SUCCESS_OR_RETURN(f(JsonElementType{e}));
        }
        return Result::SUCCESS;
      case Json::ValueType::objectValue: {
        auto keys = elem_.getMemberNames();
        for (auto &&key: keys) {
          CHECK_SUCCESS_OR_RETURN(f(JsonElementType{elem_[key], key.c_str()}));
        }
        return Result::SUCCESS;
      }
      default:
        return Result::ERR_TYPE;
  }
  }
  std::string serializeToString() const {
    Json::FastWriter fastWriter;
    return fastWriter.write(elem_);
  }
private:
  const char* key_name_{nullptr};
  const Json::Value elem_;
};  // class JsonElementType

class JsonCppParser {
public:
  using ElemType = JsonElementType;
  Result parse(std::string_view content);
  ElemType toRootElemType() const;
private:
  Json::Value root;
};  // class JsonCppParser

}  // namespace parser
