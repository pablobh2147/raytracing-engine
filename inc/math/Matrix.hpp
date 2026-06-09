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

template <int rows, int columns>
concept matrix_dimensions = (rows > 0) && (columns > 0);

template <typename T>
concept matrix_type = std::floating_point<T> || std::integral<T>;

template <int rows, int columns, typename T>
    requires matrix_dimensions<rows, columns> && matrix_type<T>
using Matrix = glm::mat<rows, columns, T>;

template <typename T>
using Matrix2 = Matrix<2, 2, T>;
using Matrix2i = Matrix2<int>;
using Matrix2f = Matrix2<float>;
using Matrix2d = Matrix2<double>;

template <typename T>
using Matrix3 = Matrix<3, 3, T>;
using Matrix3i = Matrix3<int>;
using Matrix3f = Matrix3<float>;
using Matrix3d = Matrix3<double>;

template <typename T>
using Matrix4 = Matrix<4, 4, T>;
using Matrix4i = Matrix4<int>;
using Matrix4f = Matrix4<float>;
using Matrix4d = Matrix4<double>;

}  // namespace hzr

template <int rows, int columns, typename T>
    requires hzr::matrix_dimensions<rows, columns> && hzr::matrix_type<T>
struct std::formatter<hzr::Matrix<rows, columns, T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const hzr::Matrix<rows, columns, T>& mat, std::format_context& ctx) const {
        auto out = std::format_to(ctx.out(), "[");
        for (int r = 0; r < rows; ++r) {
            if (r > 0) {
                out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "(");
            for (int c = 0; c < columns; ++c) {
                if (c > 0) {
                    out = std::format_to(out, ", ");
                }
                out = std::format_to(out, "{}", mat[c][r]);
            }
            out = std::format_to(out, ")");
        }
        return std::format_to(out, "]");
    }
};
