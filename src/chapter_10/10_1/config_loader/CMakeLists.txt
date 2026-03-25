add_subdirectory(parser)

add_library(config_loader INTERFACE)
target_link_libraries(config_loader INTERFACE json_parser yaml_parser)

