if(DEFINED ENV{build_output_bin_dir})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{build_output_bin_dir})
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)
endif()
message("CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_subdirectory(utils)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(main_name chapter_01)
add_executable(${main_name} ${main_name}/main.cc)
target_link_libraries(${main_name} Microsoft.GSL::GSL)

set(main_name chapter_01_benchmark_shape)
add_executable(${main_name} chapter_01/benchmark_shape.cc)
target_link_libraries(${main_name} benchmark::benchmark )

set(main_name chapter_02)
add_executable(${main_name} ${main_name}/main.cc)

set(main_name chapter_03)
add_executable(${main_name} ${main_name}/main.cc)

set(main_name chapter_07)
add_executable(${main_name} ${main_name}/main.cc)
