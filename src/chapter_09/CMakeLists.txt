add_library(math)
target_sources(math
  PUBLIC
    FILE_SET CXX_MODULES
    FILES math.cppm
  PRIVATE
    math.cpp)

add_library(shape)
target_sources(shape
  PUBLIC
    FILE_SET CXX_MODULES
    FILES shape.cppm point.cppm rectangle.cppm
  PRIVATE
    rectangle.cpp)
set_property(SOURCE rectangle.cppm PROPERTY CXX_MODULE_NAME "shape:rectangle")
set_property(SOURCE rectangle.cppm PROPERTY CXX_MODULE_IS_PARTITION ON)
set_property(SOURCE point.cppm PROPERTY CXX_MODULE_NAME "shape:point")
set_property(SOURCE point.cppm PROPERTY CXX_MODULE_IS_PARTITION ON)

set(main_name chapter_09)
add_executable(${main_name} main.cc)
target_link_libraries(${main_name} math shape)
