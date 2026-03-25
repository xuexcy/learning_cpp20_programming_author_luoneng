/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/23 14:26:50
# Desc   :
########################################################################
*/
#pragma once

namespace detail {

struct UnsupportedParser {
};  // struct UnsupportedParser

template <typename P>
inline constexpr bool enable_parser = false;
template <>
inline constexpr bool enable_parser<detail::UnsupportedParser> = true;
}  // namespace detail
