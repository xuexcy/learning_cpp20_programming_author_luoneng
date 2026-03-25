add_library(json_parser json_parser.h json_parser.cpp)
target_link_libraries(json_parser jsoncpp_lib)
add_library(yaml_parser yaml_parser.h yaml_parser.cpp)
