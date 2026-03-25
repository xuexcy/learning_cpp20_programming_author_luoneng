add_subdirectory(config_loader)

set(main_name chapter_10_1)
add_executable(${main_name} main.cc)
target_link_libraries(${main_name} config_loader)

add_custom_target(copy_conf ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/output/conf
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/conf
        ${CMAKE_BINARY_DIR}/output/conf
    COMMENT "Copying configuration files to output/conf"
)
