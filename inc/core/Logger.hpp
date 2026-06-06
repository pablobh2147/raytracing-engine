#pragma once

#include <format>
#include <iostream>
#include <string_view>

namespace hzr {

class Logger {
   public:
    template <typename... Args>
    static void Info(std::string_view prefix, std::format_string<Args...> format, Args&&... args) {
        std::cout << "[info] [" << prefix << "] " << std::format(format, std::forward<Args>(args)...) << std::endl;
    }

    template <typename... Args>
    static void Warning(std::string_view prefix, std::format_string<Args...> format, Args&&... args) {
        std::cerr << "[warning] [" << prefix << "] " << std::format(format, std::forward<Args>(args)...) << std::endl;
    }

    template <typename... Args>
    static void Error(std::string_view prefix, std::format_string<Args...> format, Args&&... args) {
        std::cerr << "[error] [" << prefix << "] " << std::format(format, std::forward<Args>(args)...) << std::endl;
    }
};

}  // namespace hzr