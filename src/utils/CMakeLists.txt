add_library(main_decorator main_decorator.h main_decorator.cpp)
target_link_libraries(main_decorator boost_dll)
#target_include_directories(common_header INTERFACE main_decorator.h)
#set_target_properties(common_header PROPERTIES LINKER_LANGUAGE CXX)

