/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/19 13:52:21
# Desc   :
########################################################################
*/
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "config_loader/define_schema.h"

DEFINE_SCHEMA(Point,
  (double)x, (double)y, (std::optional<double>)z, (std::variant<std::string, int, double>)other);

DEFINE_SCHEMA(TestTree,
  (std::string)name,
  (std::vector<std::unique_ptr<TestTree>>)children);
