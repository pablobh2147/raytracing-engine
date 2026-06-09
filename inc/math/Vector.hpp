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

#include <glm/glm.hpp>

#include <concepts>
#include <format>

namespace hzr {

template <int lenght>
concept vector_lenght = (lenght >= 2);

template <typename T>
concept vector_type = std::floating_point<T> || std::integral<T>;

template <int lenght, typename T>
    requires vector_lenght<lenght> && vector_type<T>
using Vector = glm::vec<lenght, T>;

template <typename T>
using Vector2 = Vector<2, T>;
using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;

template <typename T>
using Vector3 = Vector<3, T>;
using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;

template <typename T>
using Vector4 = Vector<4, T>;
using Vector4i = Vector4<int>;
using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;

}  // namespace hzr

template <int length, typename T>
    requires hzr::vector_lenght<length> && hzr::vector_type<T>
struct std::formatter<hzr::Vector<length, T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const hzr::Vector<length, T>& vec, std::format_context& ctx) const {
        auto out = std::format_to(ctx.out(), "(");
        for (int i = 0; i < length; ++i) {
            if (i > 0) {
                out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "{}", vec[i]);
        }
        return std::format_to(out, ")");
    }
};
