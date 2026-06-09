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

#include <cstddef>
#include <cstdint>

#include "math/Vector.hpp"

namespace hzr {

constexpr size_t STD430_ALIGNMENT = 16;

struct alignas(STD430_ALIGNMENT) Sphere {
    Vector3f position;
    float radius;
    uint32_t material_index;

    Sphere() = default;
    Sphere(const Vector3f& position, float radius, uint32_t material_index) : position(position), radius(radius), material_index(material_index) {}
};

struct alignas(STD430_ALIGNMENT) Plane {
    Vector3f normal;
    float distance;
    uint32_t material_index;

    Plane() = default;
    Plane(const Vector3f& normal, float distance, uint32_t material_index) : normal(normal), distance(distance), material_index(material_index) {}
};

struct alignas(STD430_ALIGNMENT) Vertex {
    Vector3f position;
    uint32_t material_index;

    Vertex() = default;
    Vertex(const Vector3f& position, uint32_t material_index) : position(position), material_index(material_index) {}
};

}  // namespace hzr