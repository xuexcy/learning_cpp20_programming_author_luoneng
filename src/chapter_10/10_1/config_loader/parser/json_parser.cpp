/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/03/24 00:06:32
# Desc   :
########################################################################
*/

#include "json_parser.h"

#include "json/json.h"
#include <memory>

namespace parser {

Result JsonCppParser::parse(std::string_view content) {
  Json::CharReaderBuilder builder;
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

  return reader->parse(content.data(), content.data() + content.size(), &root , nullptr)
      ? Result::SUCCESS : Result::ERR_ILL_FORMED;

}
JsonCppParser::ElemType JsonCppParser::toRootElemType() const {
  return ElemType{root};
}


}  // namespace parser
