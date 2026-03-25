/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/18 13:25:31
# Desc   :
########################################################################
*/
#pragma once

#define EXPAND(x) x

#define GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, n, ...) n
#define GET_ARG_COUNT(...) GET_NTH_ARG(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1)

#define REPEAT_1(f, i, arg) f(i, arg)
#define REPEAT_2(f, i, arg, ...) f(i, arg) EXPAND(REPEAT_1(f, i + 1, __VA_ARGS__))
#define REPEAT_3(f, i, arg, ...) f(i, arg) EXPAND(REPEAT_2(f, i + 1, __VA_ARGS__))
#define REPEAT_4(f, i, arg, ...) f(i, arg) EXPAND(REPEAT_3(f, i + 1, __VA_ARGS__))
#define REPEAT_5(f, i, arg, ...) f(i, arg) EXPAND(REPEAT_4(f, i + 1, __VA_ARGS__))
#define REPEAT_6(f, i, arg, ...) f(i, arg) EXPAND(REPEAT_5(f, i + 1, __VA_ARGS__))
#define REPEAT_7(f, i, arg, ...) f(i, arg) EXPAND(REPEAT_6(f, i + 1, __VA_ARGS__))
#define REPEAT_8(f, i, arg, ...) f(i, arg) EXPAND(REPEAT_7(f, i + 1, __VA_ARGS__))

#define PARE(...) __VA_ARGS__
// PAIR((double)x) -> PARE (double) x -> double x
#define PAIR(x) PARE x

#define EAT(...)
// STRIP((double) x) -> EAT(double) x -> x
#define STRIP(x) EAT x

#define STR(x) #x
// STRING(x) -> "x"
#define STRING(x) EXPAND(STR(x))

#define CONCATENATE(x, y) x ## y
#define PASTE(x, y) CONCATENATE(x, y)

