/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/19 23:07:16
# Desc   :
########################################################################
*/
#pragma once

#include "load_to_obj.h"
#include "parser.h"

template <typename T, typename Content>
Result loadXML2Obj(T& obj, Content&& content) {
  return detail::load_to_obj<detail::TinyXML2Parser>(obj, content);
}

template <typename T, typename Content>
Result loadJSON2Obj(T& obj, Content&& content) {
  return detail::load_to_obj<detail::JsonCppParser>(obj, content);
}

template <typename T, typename Content>
Result loadYAML2Obj(T& obj, Content&& content) {
  return detail::load_to_obj<detail::YamlCppParser>(obj, content);
}
