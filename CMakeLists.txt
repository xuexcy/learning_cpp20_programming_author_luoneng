cmake_minimum_required(VERSION 3.28)
project(cpp_cmake_project_template)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message ("compiler: ${CMAKE_CXX_COMPILER}")
# set(CMAKE_BUILD_TYPE Debug)

set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)
message("thirdparty dir: " ${THIRDPARTY_DIR})
# set(LIB_PROFILER_DIR $ENV{lib_profiler_dir})

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# import cpp_utils
FetchContent_Declare(
    cpp_utils
    GIT_REPOSITORY https://github.com/xuexcy/cpp_utils.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    SOURCE_DIR ${THIRDPARTY_DIR}/cpp_utils
)
FetchContent_MakeAvailable(cpp_utils)
# TODO
include_directories(${THIRDPARTY_DIR}/cpp_utils)
set(EXTRA_LIBS ${EXTRA_LIBS} cpp_utils)

FetchContent_Declare(GSL
    GIT_REPOSITORY "https://github.com/microsoft/GSL"
    GIT_TAG "v4.2.0"
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(GSL)

# import google test
# FetchContent_Declare(
#     googletest
#     GIT_REPOSITORY https://github.com/google/googletest.git
#     GIT_TAG main
#     GIT_SHALLOW TRUE
#     SOURCE_DIR ${THIRDPARTY_DIR}/googletest
# )
# FetchContent_MakeAvailable(googletest)

# import google benchmark
# FetchContent_Declare(
#     google_benchmark
#     GIT_REPOSITORY https://github.com/google/benchmark.git
#     GIT_TAG main
#     GIT_SHALLOW TRUE
#     SOURCE_DIR ${THIRDPARTY_DIR}/google_benchmark
# )
# FetchContent_MakeAvailable(google_benchmark)

# import boost
set(BOOST_INCLUDE_LIBRARIES bimap dll)
set(BOOST_ENABLE_CMAKE ON)
FetchContent_Declare(
    boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.tar.gz
    URL_HASH SHA256=c62ce6e64d34414864fef946363db91cea89c1b90360eabed0515f0eda74c75c
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    SOURCE_DIR ${THIRDPARTY_DIR}/boost
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(boost)
# #add_subdirectory(${THIRDPARTY_DIR}/boost EXCLUDE_FROM_ALL)
# # set(libs bimap)
# # foreach(lib IN LISTS libs)
# #     include_directories(${THIRDPARTY_DIR}/boost/libs/${lib}/include)
# # endforeach()
#
# #FetchContent_Declare(
# #    eigen
# #    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
# #    DOWNLOAD_EXTRACT_TIMESTAMP ON
# #    #URL_HASH SHA128=80a43aa9b64030639d1b1bd0c01fae801ab722f537ed
# #    SOURCE_DIR ${THIRDPARTY_DIR}/eigen
# #    EXCLUDE_FROM_ALL
# #)
# #FetchContent_MakeAvailable(eigen)
#
# # FetchContent_Declare(
# #     folly
# #     # GIT_REPOSITORY https://github.com/facebook/folly.git
# #     # GIT_TAG main
# #     URL https://github.com/facebook/folly/releases/download/v2024.11.11.00/folly-v2024.11.11.00.tar.gz
# #     SOURCE_DIR ${THIRDPARTY_DIR}/folly
# # )
# # FetchContent_MakeAvailable(folly)
#
# #find_package(folly REQUIRED)
# #include_directories(${FOLLY_INCLUDE_DIRS})

# set(executable_names
# address_of
# algebraic_hierarchy
# attach_by_initialization
# attorney_client
# barton_nackman_trick
# base_from_member
# curiously_recurring_template_pattern
# boost_mutant
# )

# https://stackoverflow.com/questions/69517191/cmake-create-executable-for-all-cpp-file-in-folder
# foreach (target_name IN LISTS executable_names)
#     add_executable(${target_name} src/${target_name}.cc)
#     target_link_libraries(${target_name} main_decorator)
#     target_link_libraries(${target_name} ${EXTRA_LIBS})
# endforeach ()

add_subdirectory(src)
