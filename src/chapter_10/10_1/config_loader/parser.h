/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/23 23:47:19
# Desc   :
########################################################################
*/
#pragma once

#include "enable_parser.h"
#include "parser/json_parser.h"
// #include "parser/yaml_parser.h"

namespace detail {
using TinyXML2Parser = detail::UnsupportedParser;
using JsonCppParser = parser::JsonCppParser;
using YamlCppParser = detail::UnsupportedParser;
}  // namespace detail
