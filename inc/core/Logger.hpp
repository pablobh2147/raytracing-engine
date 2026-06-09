/*  ==============================================================================
 *  Hoshizora (星空) — Physically Based Renderer
 *  ------------------------------------------------------------------------------
 *  Copyright (c) 2026 Pablo Bermejo Hernández. All rights reserved.
 *
 *  This software is licensed under the MIT License.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 *  See the LICENSE file in the project root for full license information.
 *  ==============================================================================
 */

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