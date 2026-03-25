/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/19 21:58:38
# Desc   :
########################################################################
*/
#pragma once


enum class Result {
  SUCCESS,              // 解析成功
  ERR_EMPTY_CONTENT,    // 解析文件为空
  ERR_ILL_FORMED,       // 解析文件非法
  ERR_MISSING_FIELD,    // 丢失字段
  ERR_EXTRACTING_FIELD, // 解析值失败
  ERR_TYPE,             // 类型错误
  ERR_UNSUPPORTED_PARSER, // 不支持的解析器
};

#define CHECK_SUCCESS_OR_RETURN(call) \
  do { \
    if (auto res = call; res != Result::SUCCESS) { return res; } \
  } while (0)
