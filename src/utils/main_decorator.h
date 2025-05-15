#pragma once

#include <print>

namespace utils {

class MainDecorator {
private:
    static const std::string S_PROGRAM_LOCATION;
public:
    static void Access();
public:
    MainDecorator() {
        std::println("Start main: {}", S_PROGRAM_LOCATION);
    }
    ~MainDecorator() {
        std::println("Finished main: {}\n", S_PROGRAM_LOCATION);
    }
};

}  // namespace utils
///utils::MainDecorator g_main_decorator;
extern utils::MainDecorator g_utils_main_decorator;
